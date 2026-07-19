#include "win_main.h"
#include "win_net.h"
#include <Windows.h>
#include <mmsystem.h>
#include <shellapi.h>
#include <game_mp/g_main_mp.h>
#include <sound/snd_driver_xaudio2.h>
#include "win_localize.h"
#include <monkey/monkey.h>
#include <qcommon/threads.h>
#include "win_syscon.h"
#include <live/live_steam.h>
#include "win_input.h"
#include <client/cl_keys.h>
#include <clientscript/cscr_stringlist.h>
#include <qcommon/mem_track.h>
#include <client/con_channels.h>
#include <universal/com_memory.h>
#include <gfx_d3d/rb_backend.h>
#include "win_shared.h"
#include <gfx_d3d/r_dvars.h>
#include <mjpeg/mjpeg.h>
#include <mjpeg/avi.h>
#include <universal/com_buildinfo.h>
#undef UNICODE // Module32First ascii version in below Tlhelp32.h
#include <TlHelp32.h>
#include "win_mini_dumper.h"
#include <universal/physicalmemory.h>
#include <universal/q_parse.h>
#include <universal/timing.h>
#include "win_wndproc.h"
#include <tl/tl_system.h>
#include <qcommon/tl_support.h>
#include "win_configure.h"
#include <direct.h>
#include "win_steam.h"

const dvar_t *sys_configureGHz;
const dvar_t *sys_sysMB;
const dvar_t *sys_gpu;
const dvar_t *sys_configSum;
const dvar_t *sys_SSE;

bool g_allowMature = true;

int s_nosnd;
SysInfo sys_info;
char sys_exitCmdLine[1024];
sysEvent_t eventQue[256];
int eventHead;
int eventTail;
char sys_processSemaphoreFile[260];

bool __cdecl PC_StartWithNoSounds()
{
    if ( G_ExitAfterToolComplete() )
        s_nosnd = 1;
    if ( !SD_Xaudio2CanInit() )
        s_nosnd = 1;
    return s_nosnd != 0;
}

void __cdecl Sys_GetInfo(SysInfo *info)
{
    memcpy(info, &sys_info, sizeof(SysInfo));
}

bool __cdecl Sys_HasConfigureChecksumChanged(int checksum)
{
    bool changed; // [esp+3h] [ebp-1h]

    Sys_RegisterInfoDvars();
    if ( G_OnlyConnectingPaths() )
        return 0;
    changed = 0;
    if ( sys_configSum->current.integer && sys_configSum->current.integer != checksum )
        changed = Sys_ShouldUpdateForConfigChange();
    if ( !sys_configSum->current.integer || sys_configSum->current.integer != checksum )
        Dvar_SetInt((dvar_s *)sys_configSum, checksum);
    return changed;
}

bool __cdecl Sys_ShouldUpdateForConfigChange()
{
    HWND ActiveWindow; // eax
    char *v2; // [esp-Ch] [ebp-Ch]
    char *v3; // [esp-8h] [ebp-8h]

    v3 = Win_LocalizeRef("WIN_CONFIGURE_UPDATED_TITLE");
    v2 = Win_LocalizeRef("WIN_CONFIGURE_UPDATED_BODY");
    ActiveWindow = GetActiveWindow();
    return MessageBoxA(ActiveWindow, v2, v3, 0x44u) == 6;
}

void Sys_RegisterInfoDvars()
{
    float value; // xmm0_4

    sys_configureGHz = _Dvar_RegisterFloat(
                                             "sys_configureGHz",
                                             0.0,
                                             -3.4028235e38,
                                             3.4028235e38,
                                             0x11u,
                                             "Normalized total CPU power, based on cpu type, count, and speed; used in autoconfigure");
    sys_sysMB = _Dvar_RegisterInt("sys_sysMB", 0, 0x80000000, 0x7FFFFFFF, 0x11u, "Physical memory in the system");
    sys_gpu = _Dvar_RegisterString("sys_gpu", (char *)"", 0x11u, "GPU description");
    sys_configSum = _Dvar_RegisterInt("sys_configSum", 0, 0x80000000, 0x7FFFFFFF, 0x11u, "Configuration checksum");
    sys_SSE = _Dvar_RegisterBool("sys_SSE", sys_info.SSE, 0x40u, "Operating system allows Streaming SIMD Extensions");
    value = sys_info.cpuGHz;
    _Dvar_RegisterFloat("sys_cpuGHz", value, -3.4028235e38, 3.4028235e38, 0x40u, "Measured CPU speed");
    _Dvar_RegisterString("sys_cpuName", sys_info.cpuName, 0x40u, "CPU name description");
}

bool __cdecl Sys_HasInfoChanged()
{
    Sys_RegisterInfoDvars();
    return (sys_configureGHz->current.value > sys_info.configureGHz * 1.100000023841858
             || sys_info.configureGHz * 0.8999999761581421 > sys_configureGHz->current.value
             || sys_sysMB->current.integer > sys_info.sysMB + 32
             || sys_sysMB->current.integer < sys_info.sysMB - 32
             || strcmp(sys_gpu->current.string, sys_info.gpuDescription))
            && Sys_ShouldUpdateForInfoChange();
}

bool __cdecl Sys_ShouldUpdateForInfoChange()
{
    HWND ActiveWindow; // eax
    char *v2; // [esp-Ch] [ebp-Ch]
    char *v3; // [esp-8h] [ebp-8h]

    Sys_ArchiveInfo(0);
    if ( G_OnlyConnectingPaths() )
        return 0;
    v3 = Win_LocalizeRef("WIN_COMPUTER_CHANGE_TITLE");
    v2 = Win_LocalizeRef("WIN_COMPUTER_CHANGE_BODY");
    ActiveWindow = GetActiveWindow();
    return MessageBoxA(ActiveWindow, v2, v3, 0x44u) == 6;
}

void __cdecl Sys_ArchiveInfo(int checksum)
{
    float value; // xmm0_4

    Sys_RegisterInfoDvars();
    value = sys_info.configureGHz;
    Dvar_SetFloat((dvar_s *)sys_configureGHz, value);
    Dvar_SetInt((dvar_s *)sys_sysMB, sys_info.sysMB);
    Dvar_SetString((dvar_s *)sys_gpu, sys_info.gpuDescription);
    Dvar_SetInt((dvar_s *)sys_configSum, checksum);
}

void __cdecl    Sys_DirectXFatalError()
{
    HWND ActiveWindow; // eax
    char *v1; // [esp-Ch] [ebp-Ch]
    char *v2; // [esp-8h] [ebp-8h]

    if ( Monkey_IsRunning() )
    {
        Monkey_Error("Sys_DirectXFatalError");
        exit(-1);
    }
    Sys_EnterCriticalSection(CRITSECT_FATAL_ERROR);
    v2 = Win_LocalizeRef("WIN_DIRECTX_INIT_TITLE");
    v1 = Win_LocalizeRef("WIN_DIRECTX_INIT_BODY");
    ActiveWindow = GetActiveWindow();
    MessageBoxA(ActiveWindow, v1, v2, 0x10u);
    ShellExecuteA(0, "open", "Docs\\TechHelp\\Tech Help\\Information\\DirectX.htm", 0, 0, 3);
    //BLOPS_NULLSUB();
    exit(-1);
}

void __cdecl    Sys_OutOfMemErrorInternal(const char *filename, int line)
{
    const char *v2; // eax
    HWND ActiveWindow; // eax
    char *v4; // [esp-Ch] [ebp-Ch]
    char *v5; // [esp-8h] [ebp-8h]

    if ( Monkey_IsRunning() )
    {
        v2 = va("Sys_OutOfMemErrorInternal(%s, %d)", filename, line);
        Monkey_Error(v2);
        exit(-1);
    }
    Sys_EnterCriticalSection(CRITSECT_FATAL_ERROR);
    Com_Printf(16, "Out of memory: filename '%s', line %d\n", filename, line);
    v5 = Win_LocalizeRef("WIN_OUT_OF_MEM_TITLE");
    v4 = Win_LocalizeRef("WIN_OUT_OF_MEM_BODY");
    ActiveWindow = GetActiveWindow();
    MessageBoxA(ActiveWindow, v4, v5, 0x10u);
    //BLOPS_NULLSUB();
    exit(-1);
}

void __cdecl Sys_QuitAndStartProcess(const char *exeName)
{
    I_strncpyz(sys_exitCmdLine, exeName, 1024);
    Cbuf_AddText(0, "quit\n");
}

void __cdecl Sys_OpenURL(const char *url, int doexit)
{
#if 0 // KISAKTODO: re-enable this later when it's rewritten
    const char *v2; // eax
    HWND__ *wnd; // [esp+0h] [ebp-4h]

    if ( !ShellExecuteA(0, "open", url, 0, 0, 9) )
    {
        v2 = va("EXE_ERR_COULDNT_OPEN_URL %s", url);
        Com_Error(ERR_DROP, v2);
    }
    wnd = GetForegroundWindow();
    if ( wnd )
        ShowWindow(wnd, 3);
    if ( doexit )
        Cbuf_AddText(0, "quit\n");
#endif
}

void    Sys_Error(char *error, ...)
{
    char string[4100]; // [esp+20h] [ebp-1008h] BYREF
    va_list va; // [esp+1034h] [ebp+Ch] BYREF
    MSG Msg;

    va_start(va, error);
    Sys_EnterCriticalSection(CRITSECT_COM_ERROR);
    Com_PrintStackTrace();
    com_errorEntered = 1;
    Sys_SuspendOtherThreads();
    _vsnprintf(string, 0x1000u, error, va);

    if ( Monkey_IsRunning() )
    {
        Monkey_Error(string);
        exit(0);
    }

    FixWindowsDesktop();

    if (IsDedicatedServer())
    {
        Sys_SetErrorText(string);
    }
    else
    {
        if (Sys_IsMainThread())
        {
            Sys_ShowConsole();
            Conbuf_AppendText((char*)"\n\n");
            Conbuf_AppendText(string);
            Conbuf_AppendText((char *)"\n");
        }

        Sys_SetErrorText(string);

        // wait for the user to quit
        while (GetMessage(&Msg, 0, 0, 0))
        {
            TranslateMessage(&Msg);
            DispatchMessage(&Msg);
        }
    }
    //BLOPS_NULLSUB();
    exit(0);
}

void __cdecl    Sys_Quit()
{
    Sys_EnterCriticalSection(CRITSECT_COM_ERROR);
    timeEndPeriod(1u);
    Sys_SpawnQuitProcess();
    LiveSteam_Shutdown();
    CL_ShutdownAll();
    IN_Shutdown();
    Key_Shutdown();
    Sys_DestroyConsole();
    Sys_NormalExit();
    Win_ShutdownLocalization();
    RefreshQuitOnErrorCondition();
    Dvar_Shutdown();
    Cmd_Shutdown();
    //BLOPS_NULLSUB();
    //BLOPS_NULLSUB();
    Sys_ShutdownEvents();
    SL_Shutdown(SCRIPTINSTANCE_SERVER);
    SL_Shutdown(SCRIPTINSTANCE_CLIENT);
    if ( !com_errorEntered )
        track_shutdown(0);
    Con_ShutdownChannels();
    exit(0);
}

void Sys_SpawnQuitProcess()
{
    char *v0; // eax
    void *v1; // [esp-8h] [ebp-14h]
    unsigned int v2; // [esp-4h] [ebp-10h]
    void *msgBuf; // [esp+0h] [ebp-Ch] BYREF
    unsigned int error; // [esp+4h] [ebp-8h]

    if ( sys_exitCmdLine[0] )
    {
        if ( !LiveSteam_LaunchOtherApp(sys_exitCmdLine) )
        {
            error = GetLastError();
            FormatMessageA(0x1300u, 0, error, 0x400u, (LPSTR)&msgBuf, 0, 0);
            v2 = error;
            v1 = msgBuf;
            v0 = Win_LocalizeRef("WIN_COULDNT_START_PROCESS");
            Com_Error(ERR_FATAL, "%s %s %s(0x%08x)", v0, sys_exitCmdLine, v1, v2);
        }
    }
}

int enable_OutputDebugString = 1;

void __cdecl Sys_Print(char *msg)
{
    if ( enable_OutputDebugString )
        OutputDebugStringA(msg);
    Conbuf_AppendTextInMainThread(msg);
}

char *__cdecl Sys_GetClipboardData()
{
    SIZE_T v0; // eax
    SIZE_T v1; // eax
    HANDLE hClipboardData; // [esp+0h] [ebp-Ch]
    char *data; // [esp+4h] [ebp-8h]
    char *cliptext; // [esp+8h] [ebp-4h]

    data = 0;
    if ( OpenClipboard(0) )
    {
        hClipboardData = GetClipboardData(1u);
        if ( hClipboardData )
        {
            cliptext = (char *)GlobalLock(hClipboardData);
            if ( cliptext )
            {
                v0 = GlobalSize(hClipboardData);
                data = (char *)Z_Malloc(v0 + 1, "Sys_GetClipboardData", 11);
                v1 = GlobalSize(hClipboardData);
                I_strncpyz(data, cliptext, v1);
                GlobalUnlock(hClipboardData);
                strtok(data, "\n\r\b");
            }
        }
        CloseClipboard();
    }
    return data;
}

void __cdecl Sys_QueEvent(unsigned int time, sysEventType_t type, int value, int value2, int ptrLength, void *ptr)
{
    sysEvent_t *ev; // [esp+0h] [ebp-4h]

    Sys_EnterCriticalSection(CRITSECT_SYS_EVENT_QUEUE);
    ev = &eventQue[(unsigned __int8)eventHead];
    if ( eventHead - eventTail >= 256 )
    {
        Com_Printf(16, "Sys_QueEvent: overflow\n");
        if ( ev->evPtr )
            Z_Free((char *)ev->evPtr, 11);
        ++eventTail;
    }
    ++eventHead;
    if ( !time )
        time = Sys_Milliseconds();
    ev->evTime = time;
    ev->evType = type;
    ev->evValue = value;
    ev->evValue2 = value2;
    ev->evPtrLength = ptrLength;
    ev->evPtr = ptr;
    Sys_LeaveCriticalSection(CRITSECT_SYS_EVENT_QUEUE);
}

void Sys_ShutdownEvents()
{
    sysEvent_t *ev; // [esp+0h] [ebp-4h]

    Sys_EnterCriticalSection(CRITSECT_SYS_EVENT_QUEUE);
    while ( eventHead > eventTail )
    {
        ev = &eventQue[(unsigned __int8)eventTail++];
        if ( ev->evPtr )
            Z_Free((char *)ev->evPtr, 11);
    }
    Sys_LeaveCriticalSection(CRITSECT_SYS_EVENT_QUEUE);
}

void __cdecl Sys_LoadingKeepAlive()
{
    sysEvent_t result; // [esp+0h] [ebp-48h] BYREF
    sysEvent_t v1; // [esp+18h] [ebp-30h]
    sysEvent_t ev; // [esp+30h] [ebp-18h]

    Monkey_KeepAlive();
    do
    {
        v1 = *Win_GetEvent(&result);
        ev = v1;
    }
    while ( v1.evType );
}

sysEvent_t *__cdecl Win_GetEvent(sysEvent_t *result)
{
    int v2; // [esp+0h] [ebp-50h]
    char *b; // [esp+10h] [ebp-40h]
    tagMSG msg; // [esp+18h] [ebp-38h] BYREF
    char *s; // [esp+34h] [ebp-1Ch]
    sysEvent_t ev; // [esp+38h] [ebp-18h] BYREF

    Sys_EnterCriticalSection(CRITSECT_SYS_EVENT_QUEUE);
    if ( eventHead <= eventTail )
    {
        if ( Sys_QueryWin32QuitEvent() )
            Com_Quit_f();

        if (IsDedicatedServer())
        {
            while (PeekMessageA(&msg, 0, 0, 0, 0))
            {
                if (!GetMessageA(&msg, 0, 0, 0))
                    Com_Quit_f();
                g_msgTime = msg.time;
                TranslateMessage(&msg);
                DispatchMessageA(&msg);
            }
        }
        
        s = Sys_ConsoleInput();
        if ( s )
        {
            v2 = strlen(s);
            b = (char *)Com_AllocEvent(v2 + 1);
            I_strncpyz(b, s, v2);
            Sys_QueEvent(0, SE_CONSOLE, 0, 0, v2 + 1, b);
        }
        if ( eventHead <= eventTail )
        {
            memset(&ev, 0, sizeof(ev));
            ev.evTime = Sys_Milliseconds();
        }
        else
        {
            ev = eventQue[(unsigned __int8)eventTail++];
        }
        Sys_LeaveCriticalSection(CRITSECT_SYS_EVENT_QUEUE);
        *result = ev;
        return result;
    }
    else
    {
        ev = eventQue[(unsigned __int8)eventTail++];
        Sys_LeaveCriticalSection(CRITSECT_SYS_EVENT_QUEUE);
        *result = ev;
        return result;
    }
}

sysEvent_t *__cdecl Sys_GetEvent(sysEvent_t *result)
{
    sysEvent_t v2; // [esp+0h] [ebp-30h] BYREF
    sysEvent_t v3; // [esp+18h] [ebp-18h]

    v3 = *Win_GetEvent(&v2);
    *result = v3;
    return result;
}

void __cdecl Sys_Mjpeg()
{
    const char *v0; // eax
    unsigned int v1; // [esp-Ch] [ebp-38h]
    int v2; // [esp-8h] [ebp-34h]
    unsigned int unsignedInt; // [esp-4h] [ebp-30h]
    int heights[4]; // [esp+1Ch] [ebp-10h]

    heights[0] = 360;
    heights[1] = 480;
    heights[2] = 720;
    heights[3] = 1080;
    if ( Cmd_Argc() == 2 )
    {
        mjpeg_init();
        unsignedInt = r_clipFPS->current.unsignedInt;
        v2 = heights[r_clipSize->current.integer];
        v1 = (vidConfig.displayWidth * v2 / vidConfig.displayHeight) & 0xFFF0;
        v0 = Cmd_Argv(1);
        mjpeg_create(v0, v1, v2, unsignedInt);
    }
}

void __cdecl Sys_MjpegClose()
{
    mjpeg_close();
}

cmd_function_s Sys_In_Restart_f_VAR;
cmd_function_s Sys_Net_Restart_f_VAR;
cmd_function_s Sys_Mjpeg_VAR;
cmd_function_s Sys_MjpegClose_VAR;
cmd_function_s Sys_Listen_f_VAR;
cmd_function_s Sys_Connect_f_VAR;

void __cdecl Sys_Init()
{
    const char *BuildDisplayNameR; // eax
    char *v1; // eax
    const char *v2; // eax
    char *v3; // eax
    _OSVERSIONINFOA osversion; // [esp+14h] [ebp-A0h] BYREF

    timeBeginPeriod(1u);
    Cmd_AddCommandInternal("in_restart", Sys_In_Restart_f, &Sys_In_Restart_f_VAR);
    Cmd_AddCommandInternal("net_restart", Sys_Net_Restart_f, &Sys_Net_Restart_f_VAR);
    Cmd_AddCommandInternal("movie_start", Sys_Mjpeg, &Sys_Mjpeg_VAR);
    Cmd_AddCommandInternal("movie_stop", Sys_MjpegClose, &Sys_MjpegClose_VAR);
    Cmd_AddCommandInternal("net_listen", Sys_Listen_f, &Sys_Listen_f_VAR);
    Cmd_AddCommandInternal("net_connect", Sys_Connect_f, &Sys_Connect_f_VAR);
    osversion.dwOSVersionInfoSize = 148;
#pragma warning(push)
#pragma warning(disable : 4996) // cannot call GetVersionExA nowadays without this warning suppression
    if ( !GetVersionExA(&osversion) )
        Sys_Error((char*)"Couldn't get OS info");
#pragma warning(pop)
    if ( osversion.dwMajorVersion < 4 )
    {
        BuildDisplayNameR = Com_GetBuildDisplayNameR();
        v1 = va("%s requires Windows version 4 or greater", BuildDisplayNameR);
        Sys_Error(v1);
    }
    if ( !osversion.dwPlatformId )
    {
        v2 = Com_GetBuildDisplayNameR();
        v3 = va("%s doesn't run on Win32s", v2);
        Sys_Error(v3);
    }
    Com_Printf(16, "CPU vendor is \"%s\"\n", sys_info.cpuVendor);
    Com_Printf(16, "CPU name is \"%s\"\n", sys_info.cpuName);
    if ( sys_info.logicalCpuCount == 1 )
        Com_Printf(16, "%i logical CPU%s reported\n", 1, "");
    else
        Com_Printf(16, "%i logical CPU%s reported\n", sys_info.logicalCpuCount, "s");
    if ( sys_info.physicalCpuCount == 1 )
        Com_Printf(16, "%i physical CPU%s detected\n", 1, "");
    else
        Com_Printf(16, "%i physical CPU%s detected\n", sys_info.physicalCpuCount, "s");
    Com_Printf(16, "Measured CPU speed is %.2lf GHz\n", sys_info.cpuGHz);
    Com_Printf(16, "Total CPU performance is estimated as %.2lf GHz\n", sys_info.configureGHz);
    Com_Printf(16, "System memory is %i MB (capped at 1 GB)\n", sys_info.sysMB);
    Com_Printf(16, "Video card is \"%s\"\n", sys_info.gpuDescription);
    if ( sys_info.SSE )
        Com_Printf(16, "Streaming SIMD Extensions (SSE) %ssupported\n", "");
    else
        Com_Printf(16, "Streaming SIMD Extensions (SSE) %ssupported\n", "not ");
    Com_Printf(16, "\n");
    IN_Init();
}

void __cdecl Sys_In_Restart_f()
{
    IN_Shutdown();
    IN_Init();
}

void __cdecl Sys_Net_Restart_f()
{
    NET_Restart();
}

int __cdecl Sys_CheckCrashOrRerun()
{
#ifdef KISAK_PURE
    HWND ActiveWindow; // eax
    char *v2; // [esp-Ch] [ebp-20h]
    char *v3; // [esp-8h] [ebp-1Ch]
    unsigned int procID; // [esp+0h] [ebp-14h] BYREF
    int answer; // [esp+4h] [ebp-10h]
    DWORD byteCount; // [esp+8h] [ebp-Ch] BYREF
    void *file; // [esp+Ch] [ebp-8h]
    unsigned int id; // [esp+10h] [ebp-4h] BYREF

    if ( !sys_processSemaphoreFile[0] )
        return 1;
    procID = GetCurrentProcessId();
    file = CreateFileA(sys_processSemaphoreFile, 0x80000000, 0, 0, 3u, 2u, 0);
    if ( file != (void *)-1 )
    {
        if ( ReadFile(file, &id, 4u, &byteCount, 0) && byteCount == 4 )
        {
            CloseHandle(file);
            if ( procID != id && Sys_IsGameProcess(id) )
                return 0;
            v3 = Win_LocalizeRef("WIN_IMPROPER_QUIT_TITLE");
            v2 = Win_LocalizeRef("WIN_IMPROPER_QUIT_BODY");
            ActiveWindow = GetActiveWindow();
            answer = MessageBoxA(ActiveWindow, v2, v3, 0x33u);
            if ( answer == 6 )
            {
                Com_ForceSafeMode();
            }
            else if ( answer == 2 )
            {
                return 0;
            }
        }
        else
        {
            CloseHandle(file);
        }
    }
    file = CreateFileA(sys_processSemaphoreFile, 0x40000000u, 0, 0, 2u, 2u, 0);
    if ( file == (void *)-1 )
        Sys_NoFreeFilesError();
    if ( !WriteFile(file, &procID, 4u, &byteCount, 0) || byteCount != 4 )
    {
        CloseHandle(file);
        Sys_NoFreeFilesError();
    }
    CloseHandle(file);
    return 1;
#else
    return 1; // LWSS: disable "do you wanna restart in safe mode?" prompt
#endif
}

void    Sys_NoFreeFilesError()
{
    HWND ActiveWindow; // eax
    char *v1; // [esp-Ch] [ebp-Ch]
    char *v2; // [esp-8h] [ebp-8h]

    if ( Monkey_IsRunning() )
    {
        Monkey_Error("Sys_NoFreeFilesError");
        exit(-1);
    }
    Sys_EnterCriticalSection(CRITSECT_FATAL_ERROR);
    v2 = Win_LocalizeRef("WIN_DISK_FULL_TITLE");
    v1 = Win_LocalizeRef("WIN_DISK_FULL_BODY");
    ActiveWindow = GetActiveWindow();
    MessageBoxA(ActiveWindow, v1, v2, 0x10u);
    //BLOPS_NULLSUB();
    exit(-1);
}

int __cdecl Sys_IsGameProcess(unsigned int id)
{
    //tagMODULEENTRY32 me; // [esp+0h] [ebp-350h] BYREF
    MODULEENTRY32 me; // [esp+0h] [ebp-350h] BYREF
    int isGame; // [esp+22Ch] [ebp-124h]
    char *i; // [esp+230h] [ebp-120h]
    char *moduleName; // [esp+234h] [ebp-11Ch]
    char modulePath[268]; // [esp+238h] [ebp-118h] BYREF
    void *snapshot; // [esp+348h] [ebp-8h]
    void *process; // [esp+34Ch] [ebp-4h]

    process = OpenProcess(0x1F0FFFu, 0, id);
    if ( !process )
        return 0;
    CloseHandle(process);
    snapshot = CreateToolhelp32Snapshot(8u, id);
    if ( snapshot == (void *)-1 )
        return 0;
    isGame = 0;
    me.dwSize = 548;
    if ( Module32First(snapshot, &me) )
    {
        GetModuleFileNameA(0, modulePath, 0x104u);
        modulePath[259] = 0;
        moduleName = modulePath;
        for ( i = modulePath; *i; ++i )
        {
            if ( *i == 92 || *i == 58 )
                moduleName = i + 1;
        }
        while ( I_stricmp(me.szModule, moduleName) )
        {
            if ( !Module32Next(snapshot, &me) )
                goto LABEL_15;
        }
        isGame = 1;
    }
LABEL_15:
    CloseHandle(snapshot);
    return isGame;
}

void __cdecl Sys_NormalExit()
{
    DeleteFileA(sys_processSemaphoreFile);
}

char g_ExceptionStr[32768];
int __cdecl PrivateUnhandledExceptionFilter(_EXCEPTION_POINTERS *ExceptionInfo)
{
    char *v2; // eax
    unsigned int ExceptionCode; // [esp+148h] [ebp-10h]
    int j; // [esp+14Ch] [ebp-Ch]
    int ja; // [esp+14Ch] [ebp-Ch]
    int i; // [esp+154h] [ebp-4h]

    strcpy(g_ExceptionStr, "Exception: ");
    ExceptionCode = ExceptionInfo->ExceptionRecord->ExceptionCode;
    if (ExceptionCode > 0xC0000005)
    {
        switch (ExceptionCode)
        {
        case 0xC0000006:
            strcat(g_ExceptionStr, "EXCEPTION_IN_PAGE_ERROR\n");
            break;
        case 0xC000001D:
            strcat(g_ExceptionStr, "EXCEPTION_ILLEGAL_INSTRUCTION\n");
            break;
        case 0xC0000025:
            strcat(g_ExceptionStr, "EXCEPTION_NONCONTINUABLE_EXCEPTION\n");
            break;
        case 0xC0000026:
            strcat(g_ExceptionStr, "EXCEPTION_INVALID_DISPOSITION\n");
            break;
        case 0xC000008C:
            strcat(g_ExceptionStr, "EXCEPTION_ARRAY_BOUNDS_EXCEEDED\n");
            break;
        case 0xC000008D:
            strcat(g_ExceptionStr, "EXCEPTION_FLT_DENORMAL_OPERAND\n");
            break;
        case 0xC000008E:
            strcat(g_ExceptionStr, "EXCEPTION_FLT_DIVIDE_BY_ZERO\n");
            break;
        case 0xC000008F:
            strcat(g_ExceptionStr, "EXCEPTION_FLT_INEXACT_RESULT\n");
            break;
        case 0xC0000090:
            strcat(g_ExceptionStr, "EXCEPTION_FLT_INVALID_OPERATION\n");
            break;
        case 0xC0000091:
            strcat(g_ExceptionStr, "EXCEPTION_FLT_OVERFLOW\n");
            break;
        case 0xC0000092:
            strcat(g_ExceptionStr, "EXCEPTION_FLT_STACK_CHECK\n");
            break;
        case 0xC0000093:
            strcat(g_ExceptionStr, "EXCEPTION_FLT_UNDERFLOW\n");
            break;
        case 0xC0000094:
            strcat(g_ExceptionStr, "EXCEPTION_INT_DIVIDE_BY_ZERO\n");
            break;
        case 0xC0000095:
            strcat(g_ExceptionStr, "EXCEPTION_INT_OVERFLOW\n");
            break;
        case 0xC0000096:
            strcat(g_ExceptionStr, "EXCEPTION_PRIV_INSTRUCTION\n");
            break;
        case 0xC00000FD:
            strcat(g_ExceptionStr, "EXCEPTION_STACK_OVERFLOW\n");
            break;
        default:
            goto LABEL_28;
        }
    }
    else
    {
        switch (ExceptionCode)
        {
        case 0xC0000005:
            strcat(g_ExceptionStr, "EXCEPTION_ACCESS_VIOLATION\n");
            break;
        case 0x80000002:
            strcat(g_ExceptionStr, "EXCEPTION_DATATYPE_MISALIGNMENT\n");
            break;
        case 0x80000003:
            strcat(g_ExceptionStr, "EXCEPTION_BREAKPOINT\n");
            break;
        case 0x80000004:
            strcat(g_ExceptionStr, "EXCEPTION_SINGLE_STEP\n");
            break;
        default:
        LABEL_28:
            strcat(g_ExceptionStr, "UNKNOWN\n");
            break;
        }
    }
    sprintf(
        &g_ExceptionStr[strlen(g_ExceptionStr)],
        "Exception Address: %08x\n\n",
        ExceptionInfo->ExceptionRecord->ExceptionAddress);
    if ((ExceptionInfo->ContextRecord->ContextFlags & 0x10001) != 0)
        sprintf(
            &g_ExceptionStr[strlen(g_ExceptionStr)],
            "EBP: %08x\tEIP: %08x\tSEGCS: %08x\tESP: %08x\tSEGSS: %08x\tEFLAGS: %08x\n\n",
            ExceptionInfo->ContextRecord->Ebp,
            ExceptionInfo->ContextRecord->Eip,
            ExceptionInfo->ContextRecord->SegCs,
            ExceptionInfo->ContextRecord->Esp,
            ExceptionInfo->ContextRecord->SegSs,
            ExceptionInfo->ContextRecord->EFlags);
    if ((ExceptionInfo->ContextRecord->ContextFlags & 0x10004) != 0)
        sprintf(
            &g_ExceptionStr[strlen(g_ExceptionStr)],
            "SEGGS: %08x\tSEGFS: %08x\tSEGES: %08x\tSEGDS: %08x\n",
            ExceptionInfo->ContextRecord->SegGs,
            ExceptionInfo->ContextRecord->SegFs,
            ExceptionInfo->ContextRecord->SegEs,
            ExceptionInfo->ContextRecord->SegDs);
    if ((ExceptionInfo->ContextRecord->ContextFlags & 0x10002) != 0)
        sprintf(
            &g_ExceptionStr[strlen(g_ExceptionStr)],
            "EDI: %08x\tESI: %08x\nEAX: %08x\tEBX: %08x\nECX: %08x\tEDX: %08x\n\n",
            ExceptionInfo->ContextRecord->Edi,
            ExceptionInfo->ContextRecord->Esi,
            ExceptionInfo->ContextRecord->Eax,
            ExceptionInfo->ContextRecord->Ebx,
            ExceptionInfo->ContextRecord->Ecx,
            ExceptionInfo->ContextRecord->Edx);
    if ((ExceptionInfo->ContextRecord->ContextFlags & 0x10001) != 0)
    {
        strcat(g_ExceptionStr, "Stack Bytes: \n");
        for (i = 0; i < 128; ++i)
        {
            for (j = 0; j < 8; ++j)
            {
                sprintf(
                    &g_ExceptionStr[strlen(g_ExceptionStr)],
                    "%02x",
                    *(unsigned __int8 *)(ExceptionInfo->ContextRecord->Esp + j + 8 * i));
                strcat(g_ExceptionStr, " ");
            }
            strcat(g_ExceptionStr, "\t[");
            for (ja = 0; ja < 8; ++ja)
            {
                if (*(unsigned __int8 *)(ExceptionInfo->ContextRecord->Esp + ja + 8 * i) >= 0x21u)
                    sprintf(
                        &g_ExceptionStr[strlen(g_ExceptionStr)],
                        "%c",
                        *(unsigned __int8 *)(ExceptionInfo->ContextRecord->Esp + ja + 8 * i));
                else
                    sprintf(&g_ExceptionStr[strlen(g_ExceptionStr)], ".");
            }
            strcat(g_ExceptionStr, "]\n");
        }
        strcat(g_ExceptionStr, "\n");
    }
    v2 = Win_LocalizeRef("WIN_ERROR");
    Com_Error(ERR_FATAL, v2);
    return 1;
}

char sys_cmdline[1024];
char g_open_automate_benchmark[260];
int __stdcall WinMain(HINSTANCE__ *hInstance, HINSTANCE__ *hPrevInstance, char *lpCmdLine, int nCmdShow)
{
    char *v5; // eax
    //jpeg_decompress_struct *SCRIPT_DEBUGGER_SMOKE_TEST_SUCCESS_EXIT_CODE; // [esp+0h] [ebp-4h]

    if ( !StartingDedicatedServer(lpCmdLine) && CheckRemoteSession() )
        return 0;
    g_allowMature = 1;
    if ( I_stristr(lpCmdLine, "minidump") || !I_stristr(lpCmdLine, "nodump") )
        Sys_StartMiniDump(1);
    else
        SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)PrivateUnhandledExceptionFilter);
    Sys_InitializeCriticalSections();
    Sys_InitMainThread();
    PMem_Init();
    track_init();
    Win_InitLocalization();
    if ( !I_strnicmp(lpCmdLine, "allowdupe", 9) && lpCmdLine[9] <= 32
        || (v5 = strstr(lpCmdLine, "g_connectpaths 3"), v5)
        || (Sys_GetSemaphoreFileName(), Sys_CheckCrashOrRerun()) )
    {
        s_nosnd = I_stristr(lpCmdLine, "nosnd") != 0;
        if ( !hPrevInstance )
        {
            Com_InitParse();
            Dvar_Init();
            InitTiming();
            Sys_FindInfo();
            g_wv.hInstance = hInstance;
            I_strncpyz(sys_cmdline, lpCmdLine, 1024);
            if ( g_open_automate_benchmark[0] )
            {
                I_strncat(sys_cmdline, 1024, " +set r_open_automate 1 +set com_introPlayed 1 +set ui_autoContinue 1 +devmap ");
                I_strncat(sys_cmdline, 1024, g_open_automate_benchmark);
            }
            Win_RegisterClass();
            SetErrorMode(1u);
            Sys_Milliseconds();
            //BLOPS_NULLSUB(SCRIPT_DEBUGGER_SMOKE_TEST_SUCCESS_EXIT_CODE);
            tlPrintf("Hello from the wonderful world of TL\n"); // fuck you
            Sys_SetupTLCallbacks(0x900000);
            if ( !Sys_IsMainThread()
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_main.cpp",
                            1684,
                            0,
                            "%s",
                            "Sys_IsMainThread()") )
            {
                __debugbreak();
            }

            // LWSS ADD: Steam Init
            Steam_Init();
            // LWSS END

            Com_Init(sys_cmdline);

            if (!IsDedicatedServer())
            {
                Cbuf_AddText(0, "readStats\n");
            }

            PrintWorkingDir();
            SetFocus(g_wv.hWnd);
            if ( com_script_debugger_smoke_test->current.enabled )
                exit(31415);
            while ( 1 )
            {
                // if not running as a game client, sleep a bit
#ifdef KISAK_MP
                if (g_wv.isMinimized || IsDedicatedServer())
#elif KISAK_SP
                if (g_wv.isMinimized)
#endif
                {
                    Sleep(5);
                }

                // run the game
                Com_Frame();
                
                //while ( !Dvar_GetBool("onlinegame") );
                //PbServerProcessEvents();
            }
        }
    }
    Win_ShutdownLocalization();
    track_shutdown(0);
    return 0;
}

void Sys_FindInfo()
{
    sys_info.logicalCpuCount = Sys_GetCpuCount();
    sys_info.cpuGHz = 1.0 / (((double)1LL - (double)0LL) * msecPerRawTimerTick * 1000000.0);
    sys_info.sysMB = Sys_SystemMemoryMB();
    Sys_DetectVideoCard(512, sys_info.gpuDescription);
    sys_info.SSE = Sys_SupportsSSE();
    Sys_DetectCpuVendorAndName(sys_info.cpuVendor, sys_info.cpuName);
    Sys_SetAutoConfigureGHz(&sys_info);
}

int Sys_GetSemaphoreFileName()
{
    char *i; // [esp+0h] [ebp-118h]
    const char *moduleName; // [esp+4h] [ebp-114h]
    char modulePath[268]; // [esp+8h] [ebp-110h] BYREF

    GetModuleFileNameA(0, modulePath, 0x104u);
    modulePath[259] = 0;
    moduleName = modulePath;
    for ( i = modulePath; *i; ++i )
    {
        if ( *i == 92 || *i == 58 )
        {
            moduleName = i + 1;
        }
        else if ( *i == 46 )
        {
            *i = 0;
        }
    }
    return sprintf(sys_processSemaphoreFile, "__%s", moduleName);
}

void Win_RegisterClass()
{
    tagWNDCLASSEXA wce; // [esp+0h] [ebp-30h] BYREF

    memset((unsigned __int8 *)&wce, 0, sizeof(wce));
    wce.cbSize = 48;
    wce.lpfnWndProc = (WNDPROC)MainWndProc;
    wce.hInstance = g_wv.hInstance;
    wce.hIcon = LoadIconA(g_wv.hInstance, (LPCSTR)1);
    wce.hCursor = LoadCursorA(0, (LPCSTR)0x7F00);
    wce.hbrBackground = CreateSolidBrush(0);
    wce.lpszClassName = "CoDBlackOps";
    if ( !RegisterClassExA(&wce) )
        Com_Error(ERR_FATAL, "EXE_ERR_COULDNT_REGISTER_WINDOW");
}

void PrintWorkingDir()
{
    char cwd[260]; // [esp+0h] [ebp-108h] BYREF

    _getcwd(cwd, 256);
    Com_Printf(16, "Working directory: %s\n", cwd);
}

char __cdecl CheckRemoteSession()
{
    if ( !GetSystemMetrics(4096) )
        return 0;
    MessageBoxA(0, "The game can not be run over a remote desktop connection.", "CoD", 0);
    return 1;
}

bool __cdecl StartingDedicatedServer(char *cmdline)
{
    const char *v1; // eax
    const char *p; // [esp+18h] [ebp-4h]

    v1 = strstr(cmdline, "dedicated");
    if ( !v1 )
        return 0;
    for ( p = (const char *)(strlen("dedicated") + v1); *p && *p == 32; ++p )
        ;
    if ( *p )
    {
        if ( *p == 34 )
            ++p;
    }
    return atoi(p) != 0;
}

