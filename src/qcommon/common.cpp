#include "common.h"

#include <string.h>
#include <universal/mem_userhunk.h>
#include <game_mp/g_main_mp.h>
#include <client/con_channels.h>
#include <win32/win_common.h>
#include <client/cl_console.h>
#include <win32/win_net.h>
#include <win32/win_main.h>
#include <monkey/monkey.h>
#include <universal/com_files.h>
#include <ctime>
#include "threads.h"
#include <universal/com_buildinfo.h>
#include <server_mp/sv_main_mp.h>
#include <server_mp/sv_init_mp.h>
#include <gfx_d3d/r_water_sim.h>
#include <gfx_d3d/r_extracam.h>
#include <gfx_d3d/r_ui3d.h>
#include <gfx_d3d/rb_resource.h>
#include <glass/glass_client.h>
#include <demo/demo_playback.h>
#include <stringed/stringed_hooks.h>
#include <universal/com_tasks.h>
#include <csetjmp>
#include <win32/win_splash.h>
#include <clientscript/cscr_stringlist.h>
#include <universal/com_memory.h>
#include "dvar_cmds.h"
#include <win32/win_shared.h>
#include "com_clients.h"
#include <client/cl_keys.h>
#include <win32/win_local.h>
#include <client/splitscreen.h>
#include <universal/q_parse.h>
#include <clientscript/cscr_vm.h>
#include <game_mp/ui_gameinfo_mp.h>
#include <bgame/bg_fire.h>
#include <EffectsCore/fx_load_obj.h>
#include "com_profilemapload.h"
#include "files.h"
#include <ui/ui_shared.h>
#include <ui_mp/ui_main_mp.h>
#include "com_bsp_load_obj.h"
#include <client/cl_debugdata.h>
#include <ik/ik.h>
#include <universal/physicalmemory.h>
#include <live/live_steam.h>
#include <win32/win_stream.h>
#include <client/cl_gamepad.h>
#include <ddl/ddl_api.h>
#include <DW/dwLogOn_pc.h>
#include <gfx_d3d/r_stream.h>
#include <universal/reliablemsg.h>
#include <client_mp/cl_scrn_mp.h>
#include <win32/win_workercmds.h>
#include <live/live.h>
#include <live/live_win.h>
#include <ui/ui_playlists.h>
#include <bgame/bg_emblems.h>
#include <demo/demo_files.h>
#include <mjpeg/mjpeg.h>
#include <game_mp/pregame.h>
#include <database/db_file_load.h>
#include <universal/com_loadutils.h>
#include <ui/ui_viewer.h>
#include <clientscript/cscr_debugger.h>
#include <clientscript/cscr_memorytree.h>
#include <win32/win_wndproc.h>
#include <tl/gdt_remote.h>
#include <stringed/stringed_remote.h>
#include <client/cl_main.h>
#include <client/client.h>
#include <cgame/cg_compass.h>
#include <win32/win_input.h>
#include "dobj_management.h"
#include "cm_load.h"
#include <ui/ui_screenshot.h>
#include <live/live_fileshare_cache.h>
#include <server/sv_game.h>
#include <cgame_mp/cg_ents_mp.h>
#include <bgame/bg_weapons_def.h>
#include <universal/com_workercmds.h>
#include <gfx_d3d/r_dvars.h>
#include <cgame/cg_sound.h>
#include <win32/win_voice.h>
#include <gfx_d3d/r_cinematic.h>
#include <cgame/cg_clouds.h>
#include <devgui/devgui.h>

cmd_function_s Com_Error_f_VAR;
cmd_function_s Com_Crash_f_VAR;
cmd_function_s Com_Freeze_f_VAR;
cmd_function_s Com_Assert_f_VAR;
cmd_function_s Com_Quit_f_VAR;
cmd_function_s Com_WriteConfig_f_VAR;
cmd_function_s Com_WriteKeyConfig_f_VAR;
cmd_function_s Com_SaveKeys_f_VAR;
cmd_function_s Com_RestoreKeys_f_VAR;
cmd_function_s Com_WriteDefaults_f_VAR;

char asc_CD51B0[3] = { '[', ']', '\0'};

char cl_cdkey[34];
char cl_cdkey_dw[34];
char cl_cdkeychecksum[10] =
{ ' ', ' ', ' ', ' ', '\0', '\0', '\0', '\0', '\0', '\0' };


const char *noticeErrors[14] =
{
  "EXE_SERVER_DISCONNECTED",
  "EXE_DISCONNECTED",
  "EXE_SERVERISFULL",
  "XBOXLIVE_SIGNEDOUTOFLIVE",
  "XBOXLIVE_CANTJOINSESSION",
  "XBOXLIVE_MPNOTALLOWED",
  "XBOXLIVE_MUSTLOGIN",
  "MENU_RESETCUSTOMCLASSES",
  "MP_BETACLOSED",
  "XBOXLIVE_SIGNINCHANGED",
  "XBOXLIVE_SIGNEDOUT",
  "XBOXLIVE_SIGNEDOUT_SPLITSCREEN",
  "XBOXLIVE_NETCONNECTION",
  ""
};

static const int maxDemoMsec = 200;

const dvar_t *collectors;
const dvar_t *primaryWeaponOffset;
const dvar_t *scr_xpcollectorsscale;
const dvar_t *scr_xpscale;
const dvar_t *scr_xpzmscale;
const dvar_t *scr_codpointsxpscale;
const dvar_t *scr_codpointsmatchscale;
const dvar_t *scr_codpointsperchallenge;
const dvar_t *scr_rankXpCap;
const dvar_t *scr_codPointsCap;
const dvar_t *version;
const dvar_t *shortversion;
const dvar_s *com_recommendedSet;

const dvar_s *useFastFile;
const dvar_s *sys_smp_allowed;
const dvar_t *com_maxclients;
const dvar_t *com_freemoveScale;
const dvar_t *disconnected_ctrls;
const dvar_t *com_first_time;
const dvar_t *com_first_time_pc;
const dvar_t *dedicated;
const dvar_t *com_maxfps;
const dvar_t *arcademode;
const dvar_t *zombiemode;
const dvar_t *legacy_zombiemode;
const dvar_t *zombieStopSplitScreen;
const dvar_t *zombietron;
const dvar_t *zombietron_discovered;
const dvar_t *zombiefive_discovered;
const dvar_t *zombiefive_norandomchar;
const dvar_t *blackopsmode;
const dvar_t *spmode;
//const dvar_t *onlinegame;
const dvar_t *xblive_rankedmatch;
const dvar_t *xblive_privatematch;
const dvar_t *sys_lockThreads;
const dvar_t *com_developer;
const dvar_t *com_developer_script;
const dvar_t *com_script_debugger_smoke_test;
const dvar_t *com_logfile;
//const dvar_t *com_statmon;
const dvar_t *com_timescale;
const dvar_t *dev_timescale;
const dvar_t *com_fixedtime;
const dvar_t *com_maxFrameTime;
const dvar_t *long_blocking_call;
const dvar_t *sv_network_warning;
const dvar_t *cl_network_warning;
const dvar_t *sv_paused;
const dvar_t *cl_paused;
const dvar_t *cl_paused_simple;
const dvar_t *com_sv_running;
const dvar_t *com_show_tty_timestamps;
const dvar_t *com_voip_resume_time;
const dvar_t *com_voip_bandwidth_restricted;
const dvar_t *com_voip_disable_threshold;
const dvar_t *com_filter_output;
const dvar_t *com_introPlayed;
const dvar_t *com_startupIntroPlayed;
const dvar_t *com_desiredMenu;
const dvar_t *com_skipMovies;
const dvar_t *com_animCheck;
const dvar_t *com_hiDef;
const dvar_t *com_wideScreen;
const dvar_t *doublesided_raycasts;
const dvar_t *log_append;
const dvar_t *com_waitForStreamer;
const dvar_t *dec20_Enabled;
const dvar_t *band_demosystem;
const dvar_t *band_2players;
const dvar_t *band_4players;
const dvar_t *band_6players;
const dvar_t *band_8players;
const dvar_t *band_12players;
const dvar_t *band_18players;
const dvar_t *band_lotsplayers;
const dvar_t *band_dedicated;

int com_errorEntered;
int com_frameNumber;
int com_expectedHunkUsage;

char *rd_buffer;
unsigned int rd_buffersize;
void(__cdecl *rd_flush)(char *);
int logfile;

int opening_qconsole;
int com_consoleLogOpenFailed;
unsigned int com_errorPrintsCount;

float com_codeTimeScale;

char g_stackTrace[32768];
char com_consoleBuffer[100][256];
int com_consoleBufferCurLine;
int com_numConsoleLines;
char *com_consoleLines[32];

int weaponInfoSource;

int com_safemode;
int com_fixedConsolePosition;

float com_timescaleValue;
int timeClientFrame;

int com_frameTime;
int com_fullyInitialized;
int com_lastFrameTime[1];
int com_lastFrameIndex;

HunkUser *liveMemHunk;
char s_liveAllocatorMem[0x40000];

char com_errorMessage[4096];
errorParm_t errorcode;

const dvar_t *ui_errorMessage;
const dvar_t *ui_errorTitle;

void *__cdecl Com_LiveAllocate(unsigned int size)
{
    if ( !liveMemHunk )
        liveMemHunk = Hunk_UserCreateFromBuffer(s_liveAllocatorMem, 0x40000, HU_SCHEME_FIRSTFIT, 0, 0, "LiveAllocator", 0);
    return Hunk_UserAlloc(liveMemHunk, size, 4, 0);
}

void __cdecl Com_LiveDeallocate(void *data)
{
    Hunk_UserFree(liveMemHunk, data);
}

unsigned __int8 *__cdecl Com_LiveRealloc(unsigned __int8 *ptr, unsigned int size)
{
    unsigned __int8 *result; // [esp+0h] [ebp-4h]

    if ( size || !ptr )
    {
        result = (unsigned __int8 *)Com_LiveAllocate(size);
        if ( result )
        {
            if ( ptr )
            {
                memcpy(result, ptr, size);
                Com_LiveDeallocate(ptr);
            }
        }
        return result;
    }
    else
    {
        Com_LiveDeallocate(ptr);
        return 0;
    }
}

unsigned __int8 *__cdecl Com_LiveCalloc(unsigned int num, unsigned int size)
{
    unsigned __int8 *p; // [esp+0h] [ebp-4h]

    p = (unsigned __int8 *)Com_LiveAllocate(size * num);
    if ( p )
        memset(p, 0, size * num);
    return p;
}

bool __cdecl Com_IsRunningMenuLevel(const char *name)
{
    if ( !name )
    {
        if ( !sv_mapname )
            return 0;
        name = sv_mapname->current.string;
    }
    if ( !com_sv_running->current.enabled )
        return 0;
    if ( I_strnicmp(name, "menu_", 5) )
        return I_strcmp(name, "ui") == 0;
    return 1;
}

bool __cdecl Com_IsMenuLevel(const char *name)
{
    if ( !name )
        name = sv_mapname->current.string;

    return !I_strnicmp(name, "menu_", 5) || !I_strcmp(name, "ui") || !I_strcmp(name, "ui_mp");
}

void __cdecl Com_BeginRedirect(char *buffer, unsigned int buffersize, void (__cdecl *flush)(char *))
{
    if ( buffer && buffersize )
    {
        if ( flush )
        {
            rd_buffer = buffer;
            rd_buffersize = buffersize;
            rd_flush = flush;
            *buffer = 0;
        }
    }
}

void __cdecl Com_EndRedirect()
{
    if ( rd_flush )
        rd_flush(rd_buffer);
    rd_buffer = 0;
    rd_buffersize = 0;
    rd_flush = 0;
}

void __cdecl Com_PrintMessage(int channel, char *msg, int error)
{
    char v3; // [esp+23h] [ebp-25h]
    char *v4; // [esp+28h] [ebp-20h]
    char *v5; // [esp+2Ch] [ebp-1Ch]
    bool v6; // [esp+40h] [ebp-8h]

    v6 = channel >= 31 && !Con_IsChannelVisible(CON_DEST_CONSOLE, channel, 0);
    if ( !v6 || error == 2 || error == 3 )
    {
        //PbCaptureConsoleOutput(msg, 4096);
        Sys_EnterCriticalSection(CRITSECT_CONSOLE);
        if ( (int)(strlen(msg) + 1) <= 256 )
        {
            v5 = msg;
            v4 = com_consoleBuffer[com_consoleBufferCurLine];
            do
            {
                v3 = *v5;
                *v4++ = *v5++;
            }
            while ( v3 );
        }
        else
        {
            strncpy(com_consoleBuffer[com_consoleBufferCurLine], msg, 0xFFu);
            com_consoleBuffer[com_consoleBufferCurLine][255] = 0;
        }
        if ( ++com_consoleBufferCurLine >= 100 )
            com_consoleBufferCurLine = 0;
        Sys_LeaveCriticalSection(CRITSECT_CONSOLE);
        if ( rd_buffer )
        {
            if ( channel != 6 )
            {
                Sys_EnterCriticalSection(CRITSECT_RD_BUFFER);
                if ( strlen(rd_buffer) + strlen(msg) > rd_buffersize - 1 )
                {
                    rd_flush(rd_buffer);
                    *rd_buffer = 0;
                }
                I_strncat(rd_buffer, rd_buffersize, msg);
                Sys_LeaveCriticalSection(CRITSECT_RD_BUFFER);
            }
        }
        else
        {
            if ( channel != 6  )
            {
                if (Sys_IsRemoteDebugServer() && Sys_DebugSocketReady(0))
                {
                    Sys_WriteDebugSocketMessageType(0x2Au);
                    Sys_WriteDebugSocketString(msg);
                    Sys_EndWriteDebugSocket();
                }
                
                if (!IsDedicatedServer())
                {
                    CL_ConsolePrint(0, channel, msg, 0, 0, 32 * error);
                }
            }

            if ( *msg == 94 && msg[1] )
                msg += 2;
            if ( channel != 6
                && (!com_filter_output
                 || !com_filter_output->current.enabled
                 || Con_IsChannelVisible(CON_DEST_CONSOLE, channel, 3)) )
            {
                Sys_Print(msg);
            }
            if ( Monkey_IsRunning() )
            {
                Sys_EnterCriticalSection(CRITSECT_CONSOLE);
                Monkey_ComPrintHook(msg);
                Sys_LeaveCriticalSection(CRITSECT_CONSOLE);
            }
            if ( channel != 7 && com_logfile && com_logfile->current.integer )
                Com_LogPrintMessage(channel, msg);
        }
    }
}

void __cdecl Com_LogPrintMessage(int channel, char *msg)
{
    Sys_EnterCriticalSection(CRITSECT_CONSOLE);
    if ( FS_Initialized() )
    {
        if ( !logfile )
            Com_OpenLogFile();
        if ( logfile )
        {
            FS_WriteToDemo(msg, strlen(msg), logfile);
            if ( com_logfile->current.integer > 1 )
                FS_Flush(logfile);
        }
    }
    Sys_LeaveCriticalSection(CRITSECT_CONSOLE);
}

void Com_OpenLogFile()
{
    int BuildNumber; // eax
    const char *v1; // [esp-4h] [ebp-14h]
    __int64 aclock; // [esp+0h] [ebp-10h] BYREF
    tm *newtime; // [esp+Ch] [ebp-4h]

    if ( Sys_IsMainThread() && !opening_qconsole )
    {
        opening_qconsole = 1;
        _time64(&aclock);
        newtime = _localtime64(&aclock);
        if ( log_append && log_append->current.enabled )
            logfile = FS_FOpenFileAppend((char*)"console_mp.log");
        else
            logfile = FS_FOpenTextFileWrite((char*)"console_mp.log");
        com_consoleLogOpenFailed = logfile == 0;
        v1 = asctime(newtime);
        BuildNumber = Com_GetBuildNumber();
        Com_Printf(16, "Build %d\nlogfile opened on %s\n", BuildNumber, v1);
        opening_qconsole = 0;
    }
}

void Com_Printf(int channel, const char *fmt, ...)
{
    char string[4100]; // [esp+8h] [ebp-1008h] BYREF
    va_list va; // [esp+1020h] [ebp+10h] BYREF

    va_start(va, fmt);
    if ( channel < 31 || Con_IsChannelVisible(CON_DEST_CONSOLE, channel, 0) )
    {
        _vsnprintf(string, 0x1000u, fmt, va);
        string[4095] = 0;
        Com_PrintMessage(channel, string, 0);
    }
}

void Com_DPrintf(int channel, const char *fmt, ...)
{
    char string[4100]; // [esp+8h] [ebp-1008h] BYREF
    va_list va; // [esp+1020h] [ebp+10h] BYREF

    va_start(va, fmt);
    if ( com_developer
        && com_developer->current.integer
        && (channel < 31 || Con_IsChannelVisible(CON_DEST_CONSOLE, channel, 0)) )
    {
        _vsnprintf(string, 0x1000u, fmt, va);
        string[4095] = 0;
        Com_Printf(channel, "%s", string);
    }
}

void Com_PrintError(int channel, const char *fmt, ...)
{
    char dest[4096]; // [esp+14h] [ebp-1008h] BYREF
    int v3; // [esp+1018h] [ebp-4h]
    va_list va; // [esp+102Ch] [ebp+10h] BYREF

    va_start(va, fmt);
    if (I_stristr(fmt, "error"))
        I_strncpyz(dest, "^1", 4096);
    else
        I_strncpyz(dest, "^1Error: ", 4096);
    v3 = &dest[strlen(dest) + 1] - &dest[1];
    _vsnprintf(&dest[v3], 4096 - v3, fmt, va);
    dest[4095] = 0;
    ++com_errorPrintsCount;
    Com_PrintMessage(channel, dest, 3);
}

void Com_PrintWarning(int channel, const char *fmt, ...)
{
    char dest[4096]; // [esp+14h] [ebp-1008h] BYREF
    int v3; // [esp+1018h] [ebp-4h]
    va_list va; // [esp+102Ch] [ebp+10h] BYREF

    va_start(va, fmt);
    I_strncpyz(dest, "^3", 4096);
    v3 = &dest[strlen(dest) + 1] - &dest[1];
    _vsnprintf(&dest[v3], 4096 - v3, fmt, va);
    dest[4095] = 0;
    Com_PrintMessage(channel, dest, 2);
}

void __cdecl Com_Shutdown(const char *finalmsg)
{
    Com_ShutdownInternal(finalmsg);

    if (!IsDedicatedServer())
    {
        CL_InitRenderer();
        Com_AssetLoadUI();
    }
}

void __cdecl Com_ShutdownInternal(const char *finalmsg)
{
    int localClientNum; // [esp+0h] [ebp-4h]

    for ( localClientNum = 0; localClientNum < 1; ++localClientNum )
        CL_Disconnect(localClientNum, 0);
    SV_AllowPackets(0);
    CL_ShutdownAll();
    CL_ShutdownDemo();
    SV_Shutdown(finalmsg);
    Dvar_ResetDvars(8u, DVAR_SOURCE_INTERNAL);
    Dvar_ResetDvars(0x100u, DVAR_SOURCE_INTERNAL);
    Com_Restart();
    Com_UnloadFrontEnd();
    CL_FreePerLocalClientMemory();
}

void __cdecl Com_InitDynamicRender()
{
    bool IsMenuLevel; // al
    bool v1; // al
    bool v2; // al

    IsMenuLevel = Com_IsMenuLevel(0);
    R_InitWaterSimulationBuffers(IsMenuLevel);
    v1 = Com_IsMenuLevel(0);
    R_ExtraCam_Init(v1);
    v2 = Com_IsMenuLevel(0);
    R_UI3D_OnetimeInit(0x80u, 0x80u, 0, v2, 0);
}

void __cdecl Com_InitDynamicMemorySystems()
{
    bool IsMenuLevel; // al

    RB_Resource_Callback(Com_InitDynamicRender);
    RB_Resource_Flush();
    GlassCl_AllocateMemory();
    IsMenuLevel = Com_IsMenuLevel(0);
    Demo_AllocatePlaybackMemory(IsMenuLevel);
    Com_IsMenuLevel(0);
    //BLOPS_NULLSUB();
    if (useFastFile->current.enabled)
    {
        //BLOPS_NULLSUB();
    }
}

void __cdecl Com_ShutdownDynamicMemorySystems()
{
    //BLOPS_NULLSUB();
    Demo_DeallocatePlaybackMemory();
    GlassCl_FreeMemory();
    R_UI3D_Shutdown();
    R_ExtraCam_Shutdown();
    R_FreeWaterSimulationBuffers();
}

double __cdecl Com_GetTimeScale()
{
    return com_codeTimeScale;
}

void __cdecl Com_SetTimeScale(float timescale)
{
    com_codeTimeScale = timescale;
}

void __cdecl Com_SetLocalizedErrorMessage(const char *localizedErrorMessage, const char *titleToken)
{
    char *translation; // [esp+0h] [ebp-4h]

    ui_errorMessage = _Dvar_RegisterString(
                                            "com_errorMessage",
                                            (char *)"",
                                            0x40u,
                                            "Most recent error message");
    ui_errorTitle = _Dvar_RegisterString(
                                        "com_errorTitle",
                                        (char *)"",
                                        0x40u,
                                        "Title of the most recent error message");
    translation = SEH_LocalizeTextMessage(titleToken, "error message", LOCMSG_NOERR);
    if ( translation )
        Dvar_SetString((dvar_s *)ui_errorTitle, translation);
    else
        Dvar_SetString((dvar_s *)ui_errorTitle, "");
    Dvar_SetString((dvar_s *)ui_errorMessage, localizedErrorMessage);
    I_strncpyz(com_errorMessage, localizedErrorMessage, 4096);
}

void __cdecl Com_SetErrorMessage(char *errorMessage)
{
    char *translation; // [esp+0h] [ebp-8h]
    const char *title; // [esp+4h] [ebp-4h]

    if ( errorcode == ERR_SERVERDISCONNECT || Com_ErrorIsNotice(errorMessage) )
        title = "MENU_NOTICE";
    else
        title = "MENU_ERROR";
    translation = SEH_LocalizeTextMessage(errorMessage, "error message", LOCMSG_NOERR);
    if ( !translation )
        translation = errorMessage;
    Com_SetLocalizedErrorMessage(translation, title);
}

char __cdecl Com_ErrorIsNotice(const char *errorMessage)
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; *noticeErrors[i]; ++i )
    {
        if ( !I_stricmp(noticeErrors[i], errorMessage) )
            return 1;
    }
    return 0;
}

void __cdecl Com_PrintStackTrace()
{
    Assert_DoStackTrace(g_stackTrace, 1, 2, 0);
    Com_Printf(16, "STACKBEGIN -------------------------------------------------------------------\n");
    Com_Printf(16, g_stackTrace);
    Com_Printf(16, "STACKEND ---------------------------------------------------------------------\n");
}

void __cdecl    Com_ErrorAbort()
{
    Sys_Error((char*)"%s", com_errorMessage);
}

void Com_Error(errorParm_t code, const char *fmt, ...)
{
    _iobuf *v2; // eax
    int *Value; // eax
    va_list va; // [esp+18h] [ebp+10h] BYREF

    va_start(va, fmt);
    if ( (code == ERR_DROP || code == ERR_SCRIPT_DROP) && G_ExitAfterToolComplete() )
    {
        _vsnprintf(com_errorMessage, 0x1000u, fmt, va);
        com_errorMessage[4095] = 0;
        printf(com_errorMessage);
        Com_Printf(16, com_errorMessage);
        Com_PrintStackTrace();
    }
    else
    {
        Sys_EnterCriticalSection(CRITSECT_COM_ERROR);
        if ( code != ERR_DISCONNECT )
            Monkey_GrabComPrints(1);
        Assert_ResetAddressInfo();
        if ( (unsigned int)code <= ERR_DROP )
            Com_PrintStackTrace();
        if ( !Demo_IsIdle() && code != ERR_SCRIPT )
            Demo_End(1);
        if ( com_errorEntered )
            Sys_Error((char*)"%s", com_errorMessage);
        com_errorEntered = 1;
        _vsnprintf(com_errorMessage, 0x1000u, fmt, va);
        com_errorMessage[4095] = 0;
        if ( code != ERR_DISCONNECT && Monkey_IsRunning() )
        {
            Com_Printf(16, com_errorMessage);
            Monkey_Error(0);
            exit(-1);
        }
        if ( code == ERR_SCRIPT )
        {
            StatMon_Warning(10, 3000, (char *)"code_warning_scripterrors");
        }
        else if ( code != ERR_LOCALIZATION )
        {
            if ( code == ERR_SCRIPT_DROP )
            {
                com_fixedConsolePosition = 1;
                CL_ConsoleFixPosition();
                code = ERR_DROP;
            }
            else
            {
                com_fixedConsolePosition = 0;
            }
            errorcode = code;
            Sys_LeaveCriticalSection(CRITSECT_COM_ERROR);
            Com_Printf(16, "\n====================================================\n");
            Com_Printf(16, "Com_ERROR: %s", com_errorMessage);
            Com_Printf(16, "\n====================================================\n\n");
            if ( G_ExitOnComError(code) )
            {
                printf("Fatal Error: %s\n", com_errorMessage);
                Sys_NormalExit();
                //v2 = __iob_func();
                //fflush(v2 + 1);
                fflush(stdout);
                ExitProcess(0xFFFFFFFF);
            }
            TaskManager2_ComErrorCleanup();
            GlassCl_WaitUpdate();
            Value = (int *)Sys_GetValue(2);
            longjmp(Value, -1);
        }
        if ( !com_fixedConsolePosition )
        {
            com_fixedConsolePosition = 1;
            CL_ConsoleFixPosition();
        }
        if ( cls.uiStarted )
        {
            Com_SetErrorMessage(com_errorMessage);
            _InterlockedExchange(&cls.scriptError, 1);
        }
        com_errorEntered = 0;
        Sys_LeaveCriticalSection(CRITSECT_COM_ERROR);
    }
}

void __cdecl Com_CheckError()
{
    int *Value; // eax
    int errorEntered; // [esp+0h] [ebp-4h]

    Sys_EnterCriticalSection(CRITSECT_COM_ERROR);
    errorEntered = com_errorEntered;
    Sys_LeaveCriticalSection(CRITSECT_COM_ERROR);
    if ( errorEntered )
    {
        Value = (int *)Sys_GetValue(2);
        longjmp(Value, -1);
    }
}

void __cdecl    Com_Quit_f()
{
    int localClientNum; // [esp+0h] [ebp-4h]

    Com_Printf(0, "quitting...\n");
    R_PopRemoteScreenUpdate();
    Com_SyncThreads();
    Sys_EnterCriticalSection(CRITSECT_COM_ERROR);
    GScr_Shutdown();
    if ( !com_errorEntered )
    {
        Com_ClearTempMemory();
        Sys_DestroySplashWindow();
        for ( localClientNum = 0; localClientNum < 1; ++localClientNum )
            CL_Shutdown(localClientNum);
        SV_Shutdown("EXE_SERVERQUIT");
        Sys_SyncDatabase();
        CL_ShutdownRef();
        R_Shutdown(1);
        SL_ShutdownSystem(SCRIPTINSTANCE_SERVER, 1u);
        Com_Close();
        CL_FreePerLocalClientMemory();
        Com_CloseLogfiles();
        FS_Shutdown();
        FS_ShutDownIwdPureCheckReferences();
        FS_ShutdownServerIwdNames();
        FS_ShutdownServerReferencedIwds();
        FS_ShutdownServerReferencedFFs();
        //BLOPS_NULLSUB();
    }
    Sys_Quit();
}

void Com_ClearTempMemory()
{
    Hunk_ClearTempMemory();
    Hunk_ClearTempMemoryHigh();
}

int __cdecl Com_SafeMode()
{
    const char *v0; // eax
    const char *v1; // eax
    bool v3; // [esp+0h] [ebp-1Ch]
    int i; // [esp+18h] [ebp-4h]

    for ( i = 0; i < com_numConsoleLines; ++i )
    {
        Cmd_TokenizeString(com_consoleLines[i]);
        v0 = Cmd_Argv(0);
        v3 = 1;
        if ( I_stricmp(v0, "safe") )
        {
            v1 = Cmd_Argv(0);
            if ( I_stricmp(v1, "dvar_restart") )
                v3 = 0;
        }
        Cmd_EndTokenizedString();
        if ( v3 )
        {
            *com_consoleLines[i] = 0;
            return 1;
        }
    }
    return com_safemode;
}

void __cdecl Com_ForceSafeMode()
{
    com_safemode = 1;
}

void __cdecl Com_StartupVariable(const char *match)
{
    const char *v1; // eax
    const char *v2; // eax
    int lineIndex; // [esp+2Ch] [ebp-4h]

    for ( lineIndex = 0; lineIndex < com_numConsoleLines; ++lineIndex )
    {
        if ( !Com_StartupProcessSetCommand(lineIndex, match) )
        {
            Cmd_TokenizeString(com_consoleLines[lineIndex]);
            if ( !match || !strcmp(Cmd_Argv(1), match) )
            {
                v1 = Cmd_Argv(0);
                if ( I_stricmp(v1, "set") )
                {
                    v2 = Cmd_Argv(0);
                    if ( !I_stricmp(v2, "seta") )
                        Dvar_SetA_f();
                }
                else
                {
                    Dvar_Set_f();
                }
            }
            Cmd_EndTokenizedString();
        }
    }
}

bool __cdecl Com_StartupProcessSetCommand(int lineIndex, const char *match)
{
    const char *v2; // eax
    const char *v3; // eax
    unsigned int v5; // [esp+0h] [ebp-178h]
    int len; // [esp+58h] [ebp-120h]
    int i; // [esp+5Ch] [ebp-11Ch]
    char combined[260]; // [esp+60h] [ebp-118h] BYREF
    const dvar_s *dvar; // [esp+168h] [ebp-10h]
    int c; // [esp+16Ch] [ebp-Ch]
    const char *dvarName; // [esp+170h] [ebp-8h]
    bool result; // [esp+177h] [ebp-1h]

    result = 0;
    Cmd_TokenizeString(com_consoleLines[lineIndex]);
    if ( !match || !strcmp(Cmd_Argv(1), match) )
    {
        v2 = Cmd_Argv(0);
        if ( !I_stricmp(v2, "set") )
        {
            c = Cmd_Argc();
            if ( c >= 3 )
            {
                dvarName = Cmd_Argv(1);
                if ( Dvar_IsValidName(dvarName) )
                {
                    dvar = Dvar_FindVar(dvarName);
                    if ( dvar && (dvar->flags & 0x10) != 0 )
                    {
                        combined[0] = 0;
                        i = 2;
                        len = 0;
                        while ( i < c )
                        {
                            v5 = strlen(Cmd_Argv(i));
                            if ( v5 + 1 + len >= 0xFE )
                                break;
                            v3 = Cmd_Argv(i);
                            I_strncat(combined, 256, v3);
                            if ( i != c - 1 )
                                I_strncat(combined, 256, " ");
                            len += v5 + 1;
                            ++i;
                        }
                        Dvar_SetFromString((dvar_s *)dvar, combined);
                        result = 1;
                    }
                }
                else
                {
                    Com_Printf(0, "invalid variable name: %s\n", dvarName);
                }
            }
            else
            {
                Com_Printf(0, "USAGE: set <variable> <value>\n");
            }
        }
    }
    Cmd_EndTokenizedString();
    return result;
}

void __cdecl Info_Print(const char *s)
{
    unsigned __int8 *o; // [esp+0h] [ebp-410h]
    char *oa; // [esp+0h] [ebp-410h]
    char key[512]; // [esp+8h] [ebp-408h] BYREF
    char value[516]; // [esp+208h] [ebp-208h] BYREF

    if ( *s == 92 )
        ++s;
    while ( *s )
    {
        o = (unsigned __int8 *)key;
        while ( *s && *s != 92 )
            *o++ = *s++;
        if ( o - (unsigned __int8 *)key >= 20 )
        {
            *o = 0;
        }
        else
        {
            memset(o, 0x20u, 20 - (o - (unsigned __int8 *)key));
            key[20] = 0;
        }
        Com_Printf(0, "%s", key);
        if ( !*s )
        {
            Com_Printf(16, "MISSING VALUE\n");
            return;
        }
        oa = value;
        ++s;
        while ( *s && *s != 92 )
            *oa++ = *s++;
        *oa = 0;
        if ( *s )
            ++s;
        Com_Printf(0, "%s\n", value);
    }
}

unsigned int *__cdecl Com_AllocEvent(int size)
{
    return Z_Malloc(size, "Com_AllocEvent", 11);
}

void __cdecl Com_FreeEvent(char *ptr)
{
    Z_Free(ptr, 11);
}

void __cdecl Com_ServerPacketEvent()
{
    msg_t netmsg; // [esp+0h] [ebp-50h] BYREF
    unsigned __int8 (*msgBuf)[65536]; // [esp+30h] [ebp-20h]
    netadr_t adr; // [esp+34h] [ebp-1Ch] BYREF
    LargeLocal msgBuf_large_local(0x10000); // [esp+48h] [ebp-8h] BYREF

    //LargeLocal::LargeLocal(&msgBuf_large_local, 0x10000);
    msgBuf = (unsigned __int8 (*)[65536])msgBuf_large_local.GetBuf(); // LargeLocal::GetBuf(&msgBuf_large_local);
    MSG_Init(&netmsg, (unsigned __int8 *)msgBuf, 0x10000);
    if ( com_sv_running->current.enabled )
    {
        while ( NET_GetClientPacket(&adr, &netmsg) )
            SV_PacketEvent(adr, &netmsg);
    }
    while ( NET_GetLoopPacket(NS_SERVER, &adr, &netmsg) )
    {
        if ( com_sv_running->current.enabled )
            SV_PacketEvent(adr, &netmsg);
    }
    //LargeLocal::~LargeLocal(&msgBuf_large_local);
}

void __cdecl Com_EventLoop()
{
    sysEvent_t result; // [esp+14h] [ebp-50h] BYREF
    sysEvent_t v1; // [esp+30h] [ebp-34h]
    sysEvent_t ev; // [esp+4Ch] [ebp-18h]

    PROF_SCOPED("Com_EventLoop");

    while (1)
    {
        v1 = *Sys_GetEvent(&result);
        ev = v1;
        switch (v1.evType)
        {
        case SE_NONE:
            Com_ClientPacketEvent();
            return;
        case SE_KEY:
            CL_KeyEvent(0, ev.evValue, ev.evValue2, ev.evTime);
            break;
        case SE_CHAR:
            CL_CharEvent(0, ev.evValue);
            break;
        case SE_CONSOLE:
            Con_Restricted_AddBuf((char *)ev.evPtr);
            Com_FreeEvent((char *)ev.evPtr);
            break;
        default:
            Com_Error(ERR_FATAL, "Com_EventLoop: bad event type %i", ev.evType);
            break;
        }
    }
}

void Com_ClientPacketEvent()
{
    msg_t netmsg; // [esp+0h] [ebp-50h] BYREF
    unsigned __int8 (*msgBuf)[0x10000]; // [esp+30h] [ebp-20h]
    netadr_t adr; // [esp+34h] [ebp-1Ch] BYREF
    LargeLocal msgBuf_large_local(0x10000); // [esp+48h] [ebp-8h] BYREF

    //LargeLocal::LargeLocal(&msgBuf_large_local, 0x10000);
    msgBuf = (unsigned __int8 (*)[0x10000])msgBuf_large_local.GetBuf(); // LargeLocal::GetBuf(&msgBuf_large_local);
    MSG_Init(&netmsg, (unsigned __int8 *)msgBuf, 0x10000);

    Com_PacketEventLoop(NS_CLIENT1, &netmsg);

    while ( NET_GetDeferredClientPacket(&adr, &netmsg) )
        Com_DispatchClientPacketEvent(adr, &netmsg);

    if ( !com_sv_running->current.enabled )
    {
        while ( NET_GetClientPacket(&adr, &netmsg) )
            Com_DispatchClientPacketEvent(adr, &netmsg);

        while ( Sys_SocketPool_GetPacket(&adr, &netmsg) )
            Com_DispatchClientPacketEvent(adr, &netmsg);

        Sys_CheckForNATOverflow();
    }

    //LargeLocal::~LargeLocal(&msgBuf_large_local);
}

void __cdecl Com_PacketEventLoop(int localClientNum, msg_t *netmsg)
{
    netadr_t adr; // [esp+0h] [ebp-14h] BYREF

    while (NET_GetLoopPacket((netsrc_t)Com_LocalClient_GetNetworkID(localClientNum), &adr, netmsg))
    {
        CL_PacketEvent(localClientNum, adr, netmsg, Sys_Milliseconds(), (clientConnections == NULL));
    }
}

void __cdecl Com_DispatchClientPacketEvent(netadr_t adr, msg_t *netmsg)
{
    CL_PacketEvent(0, adr, netmsg, Sys_Milliseconds(), 0);
}

void __cdecl Com_ReadCDKey()
{
    unsigned int size; // [esp+0h] [ebp-2Ch]
    _iobuf *f; // [esp+4h] [ebp-28h]
    char regkey[32]; // [esp+8h] [ebp-24h] BYREF

    f = fopen("dwclientkey.txt", "rt");
    if ( f )
    {
        size = fread(regkey, 1u, 0x14u, f);
        fclose(f);
        if ( size == 20
            && (regkey[20] = 0,
                    memcpy((unsigned __int8 *)cl_cdkey, (unsigned __int8 *)regkey, 0x15u),
                    cl_cdkey[22] = 0,
                    *(unsigned int *)cl_cdkeychecksum = *(unsigned int *)&regkey[16],
                    //byte_E0AA3C = 0,
                    CL_LocalClient_GetActiveCount()) )
        {
            CL_ConvertRegKeytoDWKey(cl_cdkey, 0x15u);
        }
        else
        {
            Com_ClearCDKey();
        }
    }
}

int Com_ClearCDKey()
{
    strcpy(cl_cdkey, "                                ");
    return 538976288;
}

void __cdecl CL_ConvertRegKeytoDWKey(char *key, unsigned int size)
{
    int i; // [esp+0h] [ebp-4h]

    memcpy((unsigned __int8 *)cl_cdkey_dw, (unsigned __int8 *)key, size);
    for (i = 4; i < 24; i += 5)
    {
        memcpy((unsigned __int8 *)&cl_cdkey_dw[i + 1], (unsigned __int8 *)&cl_cdkey_dw[i], 25 - i);
        cl_cdkey_dw[i] = '-';
    }
}

void __cdecl Com_SetRecommended(int localClientNum, int restart)
{
    int filesize; // [esp+Ch] [ebp-274h]
    SysInfo info; // [esp+10h] [ebp-270h] BYREF
    int checksum; // [esp+274h] [ebp-Ch]
    char *csv; // [esp+278h] [ebp-8h] BYREF
    const char *text; // [esp+27Ch] [ebp-4h] BYREF

    Com_Printf(16, "========= autoconfigure\n");
    Sys_GetInfo(&info);
    info.configureGHz = info.configureGHz * 1.02;
    if ( info.sysMB >= 128 )
        info.sysMB += 8;
    else
        info.sysMB = 128;
    filesize = FS_ReadFile("configure_mp.csv", (void **)&csv);
    if ( filesize < 0 )
        Com_Error(ERR_FATAL, "EXE_ERR_NOT_FOUND");
    text = csv;
    Com_BeginParseSession("configure_mp.csv");
    Com_SetCSV(1);
    if ( !Com_SetRecommendedCpu(localClientNum, &info, (char **)&text) )
    {
        Sys_GetInfo(&info);
        Com_Error(ERR_FATAL, "configure_mp.csv: EXE_ERR_COULDNT_CONFIGURE %.0f GHZ %i MB", info.configureGHz, info.sysMB);
    }
    if ( !Com_SetRecommendedGpu(&info, (char **)&text) )
        Com_Error(ERR_FATAL, "configure_mp.csv: EXE_ERR_COULDNT_CONFIGURE %s", info.gpuDescription);
    Com_EndParseSession();
    checksum = Com_ConfigureChecksum(csv, filesize);
    FS_FreeFile(csv);
    Sys_ArchiveInfo(checksum);
    if ( restart )
    {
        if ( Dvar_AnyLatchedValues() )
            Cbuf_AddText(localClientNum, "vid_restart\n");
    }
}

int __cdecl Com_ConfigureChecksum(const char *csv, int filesize)
{
    int checksum; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    checksum = 0;
    for ( i = 0; i < filesize; ++i )
        checksum = csv[i] + 31337 * checksum;
    return (checksum & 0xFFFFFFF) + 1;
}

char __cdecl Com_SetRecommendedCpu(int localClientNum, const SysInfo *info, char **text)
{
    char dvarValues[64][32]{ 0 }; // [esp+14h] [ebp-14D8h] BYREF
    char dvarNames[64][32]{ 0 }; // [esp+814h] [ebp-CD8h] BYREF
    int dvarCount; // [esp+1018h] [ebp-4D4h]
    double v7[76]; // [esp+101Ch] [ebp-4D0h] BYREF
    char v8; // [esp+1282h] [ebp-26Ah]
    char v9; // [esp+1283h] [ebp-269h]
    double v10[76]; // [esp+1284h] [ebp-268h] BYREF
    char *s0; // [esp+14E8h] [ebp-4h]

    dvarCount = 0;
    v7[0] = -1.0;
    LODWORD(v7[3]) = 0;
    v8 = 0;
    while ( 1 )
    {
        s0 = (char *)Com_Parse((const char **)text);
        if ( !text )
            break;
        if ( *s0 && *s0 != '#')
        {
            if ( !I_stricmp(s0, "gpu") )
            {
                Com_UngetToken();
                break;
            }
            if ( dvarCount )
            {
                v10[0] = atof(s0);
                if ( v10[0] < 0.0 )
                    Com_Error(ERR_FATAL, "configure_mp.csv: cpu ghz %g not allowed to be less than 0", v10[0]);
                s0 = (char *)Com_ParseOnLine((const char **)text);
                LODWORD(v10[3]) = atoi(s0);
                if ( SLODWORD(v10[3]) < 128 )
                    Com_Error(ERR_FATAL, "configure_mp.csv: sys mb %i not allowed to be less than 128", LODWORD(v10[3]));
                v9 = 0;
                if ( info->configureGHz >= v10[0]
                    && info->sysMB >= SLODWORD(v10[3])
                    && (v10[0] > v7[0] || v7[0] == v10[0] && SLODWORD(v7[3]) < SLODWORD(v10[3])) )
                {
                    v9 = 1;
                    memcpy(v7, v10, sizeof(v7));
                    v8 = 1;
                }
                Com_GetConfigureDvarValues(dvarCount, (const char **)text, v9 != 0 ? (char (*)[32])dvarValues : 0);
            }
            else
            {
                if ( I_stricmp(s0, "cpu ghz") )
                    Com_Error(ERR_FATAL, "configure_mp.csv: \"cpu ghz\" should be the first column");
                s0 = (char *)Com_ParseOnLine((const char **)text);
                if ( I_stricmp(s0, "sys mb") )
                    Com_Error(ERR_FATAL, "configure_mp.csv: \"sys mb\" should be the second column");
                dvarCount = Com_GetConfigureDvarNames((const char **)text, (char (*)[32])dvarNames);
            }
        }
        else
        {
            Com_SkipRestOfLine((const char **)text);
        }
    }
    if ( !v8 )
        return 0;
    Com_Printf(16, "configure_mp.csv: using CPU configuration %.0f GHz %i MB\n", v7[0], LODWORD(v7[3]));
    Cbuf_AddText(localClientNum, "exec configure_mp.cfg");
    Cbuf_Execute(localClientNum, 0);
    Com_SetConfigureDvars(dvarCount, (const char (*)[32])dvarNames, (const char (*)[32])dvarValues);
    return 1;
}

int __cdecl Com_GetConfigureDvarNames(const char **text, char (*dvarNames)[32])
{
    int dvarCount; // [esp+10h] [ebp-8h]
    parseInfo_t *token; // [esp+14h] [ebp-4h]

    for ( dvarCount = 0; ; ++dvarCount )
    {
        token = Com_ParseOnLine(text);
        if ( !*text )
            Com_Error(ERR_FATAL, "configure_mp.csv: unexpected end-of-file");
        if ( !token->token[0] )
            break;
        if ( strlen(token->token) > 31 )
            Com_Error(ERR_FATAL, "configure_mp.csv: dvar name \"%s\" longer than %i", token, 31);
        if ( dvarCount >= 64 )
            Com_Error(ERR_FATAL, "configure_mp.csv: more than %i dvars", 64);
        I_strncpyz(&(*dvarNames)[32 * dvarCount], token->token, 32);
    }
    return dvarCount;
}

void __cdecl Com_GetConfigureDvarValues(int dvarCount, const char **text, char (*dvarValues)[32])
{
    int dvarIndex; // [esp+10h] [ebp-8h]
    parseInfo_t *token; // [esp+14h] [ebp-4h]
    parseInfo_t *tokena; // [esp+14h] [ebp-4h]

    for ( dvarIndex = 0; dvarIndex < dvarCount; ++dvarIndex )
    {
        token = Com_ParseOnLine(text);
        if ( !*text )
            Com_Error(ERR_FATAL, "configure_mp.csv: unexpected end-of-file");
        if ( !token->token[0] )
            Com_Error(ERR_FATAL, "configure_mp.csv: missing entry in dvar value column %i", dvarIndex);
        if ( strlen(token->token) > 0x1F )
            Com_Error(ERR_FATAL, "configure_mp.csv: entry \"%s\" in davr value column %i is loinger than %i", token, dvarIndex, 31);
        if ( dvarValues )
            I_strncpyz(&(*dvarValues)[32 * dvarIndex], token->token, 32);
    }
    tokena = Com_ParseOnLine(text);
    if ( tokena->token[0] )
        Com_Error(ERR_FATAL, "configure_mp.csv: extra dvar value column(s): value = %s", tokena);
}

void __cdecl Com_SetConfigureDvars(int dvarCount, const char (*dvarNames)[32], const char (*dvarValues)[32])
{
    int dvarIndex; // [esp+0h] [ebp-8h]
    const dvar_s *dvar; // [esp+4h] [ebp-4h]

    for ( dvarIndex = 0; dvarIndex < dvarCount; ++dvarIndex )
    {
        Dvar_SetFromStringByNameFromSource(
            &(*dvarNames)[32 * dvarIndex],
            (char *)&(*dvarValues)[32 * dvarIndex],
            DVAR_SOURCE_EXTERNAL,
            0);
        dvar = Dvar_FindVar(&(*dvarNames)[32 * dvarIndex]);
        Dvar_AddFlags(dvar, 1);
    }
}

char __cdecl Com_SetRecommendedGpu(const SysInfo *info, char **text)
{
    char dvarValues[64][32]; // [esp+0h] [ebp-1010h] BYREF
    char dvarNames[64][32]; // [esp+800h] [ebp-810h] BYREF
    int dvarCount; // [esp+1004h] [ebp-Ch]
    char v6; // [esp+100Bh] [ebp-5h]
    char *s0; // [esp+100Ch] [ebp-4h]

    s0 = (char *)Com_Parse((const char **)text);
    if ( I_stricmp(s0, "gpu") )
    {
        Com_UngetToken();
        return 0;
    }
    else
    {
        dvarCount = Com_GetConfigureDvarNames((const char **)text, (char (*)[32])dvarNames);
        v6 = 0;
        while ( 1 )
        {
            s0 = (char *)Com_Parse((const char **)text);
            if ( !*text )
                break;
            if ( *s0 && *s0 != 35 )
            {
                if ( v6 || !Com_DoesGpuStringMatch(s0, info->gpuDescription) )
                {
                    Com_GetConfigureDvarValues(dvarCount, (const char **)text, 0);
                }
                else
                {
                    Com_Printf(16, "configure_mp.csv: using GPU configuration \"%s\"\n", s0);
                    Com_GetConfigureDvarValues(dvarCount, (const char **)text, (char (*)[32])dvarValues);
                    Com_SetConfigureDvars(dvarCount, (const char (*)[32])dvarNames, (const char (*)[32])dvarValues);
                    v6 = 1;
                }
            }
            else
            {
                Com_SkipRestOfLine((const char **)text);
            }
        }
        return v6;
    }
}

bool __cdecl Com_DoesGpuStringMatch(const char *find, const char *ref)
{
    int wildcardLen; // [esp+0h] [ebp-40Ch]
    char wildcardTemplate[1024]; // [esp+4h] [ebp-408h] BYREF
    int findLen; // [esp+408h] [ebp-4h]

    wildcardTemplate[0] = 42;
    wildcardLen = 1;
    for ( findLen = 0; find[findLen]; ++findLen )
    {
        if ( !isspace(find[findLen]) )
        {
            wildcardTemplate[wildcardLen] = find[findLen];
LABEL_11:
            if ( ++wildcardLen == 1023 )
                Com_Error(ERR_FATAL, "configure_mp.csv: \"find\" string is too long");
            continue;
        }
        if ( wildcardLen < 1
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\common.cpp",
                        3841,
                        1,
                        "%s\n\t(wildcardLen) = %i",
                        "(wildcardLen >= 1)",
                        wildcardLen) )
        {
            __debugbreak();
        }
        if ( wildcardTemplate[wildcardLen - 1] != 32 )
        {
            wildcardTemplate[wildcardLen] = 32;
            goto LABEL_11;
        }
    }
    if ( wildcardTemplate[wildcardLen - 1] != 42 )
        wildcardTemplate[wildcardLen++] = 42;
    wildcardTemplate[wildcardLen] = 0;
    return Com_GpuStringCompare(wildcardTemplate, ref) == 0;
}

int __cdecl Com_GpuStringCompare(const char *wild, const char *s)
{
    int v3; // esi
    char charWild; // [esp+7h] [ebp-9h]
    int delta; // [esp+8h] [ebp-8h]
    char charRef; // [esp+Fh] [ebp-1h]

    do
    {
        charWild = *wild++;
        if ( charWild == 42 )
        {
            if ( !*wild )
                return 0;
            if ( *s && !Com_GpuStringCompare(wild - 1, s + 1) )
                return 0;
        }
        else if ( charWild == 32 )
        {
            if ( *s && !isdigit(*s) && !Com_GpuStringCompare(wild - 1, s + 1) )
                return 0;
        }
        else
        {
            charRef = *s++;
            if ( charWild != charRef && charWild != 63 )
            {
                v3 = tolower(charWild);
                delta = v3 - tolower(charRef);
                if ( delta )
                    return 2 * (delta >= 0) - 1;
            }
        }
    }
    while ( charWild );
    return 0;
}

void __cdecl Com_CheckSetRecommended(int localClientNum)
{
    if ( !com_recommendedSet->current.enabled || Com_HasConfigureFileChanged() )
    {
        Com_SetRecommended(localClientNum, 0);
        Dvar_SetBool((dvar_s *)com_recommendedSet, 1);
    }
    if ( Sys_HasInfoChanged() )
        Com_SetRecommended(localClientNum, 0);
}

bool __cdecl Com_HasConfigureFileChanged()
{
    int filesize; // [esp+0h] [ebp-Ch]
    int checksum; // [esp+4h] [ebp-8h]
    char *csv; // [esp+8h] [ebp-4h] BYREF

    filesize = FS_ReadFile("configure_mp.csv", (void **)&csv);
    if ( filesize < 0 )
        Com_Error(ERR_FATAL, "EXE_ERR_NOT_FOUND");
    checksum = Com_ConfigureChecksum(csv, filesize);
    FS_FreeFile(csv);
    return Sys_HasConfigureChecksumChanged(checksum);
}

void __cdecl Com_SetScriptSettings()
{
    bool v0; // [esp+0h] [ebp-Ch]
    int abort_on_error; // [esp+4h] [ebp-8h]

    v0 = com_developer->current.integer || com_logfile->current.integer;
    abort_on_error = com_developer->current.integer;
    Scr_Settings(v0, com_developer_script->current.color[0], abort_on_error, SCRIPTINSTANCE_SERVER);
    Scr_Settings(v0, com_developer_script->current.color[0], abort_on_error, SCRIPTINSTANCE_CLIENT);
}

void __cdecl Com_RunAutoExec(int localClientNum, int controllerIndex)
{
    if ( localClientNum >= 0 )
    {
        Dvar_SetInAutoExec(1);
        Cmd_ExecuteSingleCommand(localClientNum, controllerIndex, (char*)"exec autoexec_dev_mp.cfg");
        Dvar_SetInAutoExec(0);
    }
}

void __cdecl Com_ExecStartupConfigs(int localClientNum, const char *configFile)
{
    Cbuf_AddText(localClientNum, "exec " DEFAULT_CFG "\n");
    Cbuf_AddText(localClientNum, "exec language.cfg\n");

#ifndef KISAK_DEDICATED
    if (IsDedicatedServer())
    {
        Cbuf_AddText(localClientNum, "exec init_dvars_pc_dedicated_mp.cfg\n");
    }
    else
    {
        Cbuf_AddText(localClientNum, "exec init_dvars_pc_mp.cfg\n");
    }
#endif

    if ( configFile )
    {
        Cbuf_AddText(localClientNum, va("exec %s\n", configFile));
    }

    Cbuf_Execute(localClientNum, Com_LocalClient_GetControllerIndex(localClientNum));
    Com_RunAutoExec(localClientNum, Com_LocalClient_GetControllerIndex(localClientNum));

    if ( Com_SafeMode() )
        Cbuf_AddText(localClientNum, "exec safemode_mp.cfg\n");

    Cbuf_Execute(localClientNum, Com_LocalClient_GetControllerIndex(localClientNum));
}

void __cdecl Com_InitUI3DCallback()
{
    R_UI3D_OnetimeInit(vidConfig.displayWidth, vidConfig.displayHeight, 1, 1, 1);
}

void __cdecl Com_InitUIAndCommonXAssets()
{
    if (!IsDedicatedServer())
    {
        Com_UnloadLevelFastFiles();
        CL_AllocatePerLocalClientMemory();
        RB_Resource_Callback(Com_InitUI3DCallback);
        RB_Resource_Flush();

        R_UI3D_SetupTextureWindow(0, 0.0f, 0.0f, 1.0f, 1.0f);
        ScreenPlacement *scrPlace = R_UI3D_ScrPlaceFromTextureWindow(0);
        ScrPlace_SetupUI3DForFullscreen(scrPlace, &scrPlaceFull);
    }

    DB_LoadFastFilesForPC();
}

unsigned int __cdecl Com_CalculateStreamBuffer()
{
    return 402653184;
}

void __cdecl Com_Init(char *commandLine)
{
    if ( _setjmp((int *)Sys_GetValue(2)) )
    {
        Sys_Error(va("Error during initialization:\n%s\n", com_errorMessage));
    }

    Com_Init_Try_Block_Function(commandLine);

    Monkey_Start();

    if ( !_setjmp((int*)Sys_GetValue(2)) )
        Com_AddStartupCommands();

    if ( !I_strcmp(sv_mapname->current.string, "") )
        Com_InitUIAndCommonXAssets();

    if ( com_errorEntered )
        Com_ErrorCleanup();

#ifndef KISAK_DEDICATED
    if (!com_sv_running->current.enabled && !IsDedicatedServer())
    {
        if (_setjmp((int *)Sys_GetValue(2)))
        {
            Sys_Error(va("Error during initialization:\n%s\n", com_errorMessage));
        }

        if (!cls.rendererStarted)
        {
            CL_InitRenderer();
        }

        R_BeginRemoteScreenUpdate();
        CL_StartHunkUsers();
        R_EndRemoteScreenUpdate(NULL);
    }
#endif

    if ( !com_sv_running->current.enabled )
    {
        if ( *Dvar_GetString("com_errorMessage") )
            Com_LoadUiFastFile();
        //BLOPS_NULLSUB();
        Com_LoadFrontEnd();
        // The intro-movie/logo startup sequence is stubbed (COM_PlayIntroMovies is a
        // no-op and Bink is disabled), so the logo-skip path that normally opens the
        // main menu via CL_StopLogoOrCinematic never runs. Open the startup menu
        // directly now that the UI is up, mirroring Com_StartHunkUsers()'s recovery path.
        if ( !IsDedicatedServer() && cls.uiStarted )
            UI_SetActiveMenu(Com_LocalClients_GetPrimary(), (uiMenuCommand_t)UI_GetMenuScreen());
    }

    if (IsDedicatedServer())
    {
        UI_LoadArenas();
        UI_LoadCustomMatchGameTypes();
        UI_LoadMaps();
    }
}

int lastErrorTime;
int errorCount;
void Com_ErrorCleanup()
{
    int Primary; // eax
    uiMenuCommand_t MenuScreenForError; // [esp-4h] [ebp-1028h]
    char v2; // [esp+3h] [ebp-1021h]
    char *v3; // [esp+8h] [ebp-101Ch]
    char *v4; // [esp+Ch] [ebp-1018h]
    const char *src; // [esp+14h] [ebp-1010h]
    unsigned int v6; // [esp+18h] [ebp-100Ch]
    char finalmsg[4100]; // [esp+1Ch] [ebp-1008h] BYREF

    Dvar_RestoreDvars();
    LargeLocalReset();
    R_PopRemoteScreenUpdate();
    Com_SyncThreads();
#ifndef KISAK_DEDICATED
    if (!IsDedicatedServer())
        R_ComErrorCleanup();
#endif
    Cmd_ComErrorCleanup();
    Dvar_SetInAutoExec(0);
    if ( useFastFile->current.enabled )
        DB_Cleanup();
    Com_ClearTempMemory();
    BG_ShutdownFire();
    if ( !useFastFile->current.enabled )
        FX_UnregisterAll();
    if ( ProfLoad_IsActive() )
        ProfLoad_Deactivate();
    Dvar_SetIntByName("cl_paused", 0);
    Dvar_SetBoolByName("long_blocking_call", 0);
    Dvar_SetBoolByName("sv_network_warning", 0);
    Dvar_SetBoolByName("cl_network_warning", 0);
    FS_PureServerSetLoadedIwds((char *)"", (char *)"");
    SEH_UpdateLanguageInfo();
    v4 = com_errorMessage;
    v3 = finalmsg;
    do
    {
        v2 = *v4;
        *v3++ = *v4++;
    }
    while ( v2 );
    if ( errorcode == ERR_DISCONNECT )
    {
        if ( com_errorMessage[0] )
        {
            src = SEH_LocalizeTextMessage(com_errorMessage, "error message", LOCMSG_NOERR);
            if ( src )
                I_strncpyz(com_errorMessage, src, 4096);
        }
    }
    else
    {
        if ( cls.uiStarted && errorcode != ERR_DROP )
        {
            MenuScreenForError = (uiMenuCommand_t)UI_GetMenuScreenForError();
            Primary = Com_LocalClients_GetPrimary();
            UI_SetActiveMenu(Primary, MenuScreenForError);
        }
        Com_SetErrorMessage(com_errorMessage);
    }
    if ( fs_debug && fs_debug->current.integer == 2 )
        Dvar_SetInt((dvar_s *)fs_debug, 0);
    Com_CleanupBsp();
    //BLOPS_NULLSUB();
    Com_ResetParseSessions();
    CL_FlushDebugServerData();
    CL_UpdateDebugServerData();
    FS_ResetFiles();
    if ( errorcode == ERR_DROP )
        Cbuf_Init();
    v6 = Sys_Milliseconds();
    if ( (int)(v6 - lastErrorTime) >= 100 )
    {
        errorCount = 0;
    }
    else if ( ++errorCount > 3 )
    {
        errorcode = ERR_FATAL;
    }
    lastErrorTime = v6;
    if ( errorcode != ERR_SERVERDISCONNECT && errorcode != ERR_DROP && errorcode != ERR_DISCONNECT )
        Sys_Error((char*)"%s", com_errorMessage);
    updateScreenCalled = 0;
    if ( errorcode == ERR_SERVERDISCONNECT )
    {
        Com_ShutdownInternal("EXE_DISCONNECTEDFROMOWNLISTENSERVER");
    }
    else
    {
        if ( errorcode != ERR_DROP
            && errorcode != ERR_DISCONNECT
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\common.cpp",
                        1851,
                        0,
                        "%s\n\t(errorcode) = %i",
                        "(errorcode == ERR_DROP || errorcode == ERR_DISCONNECT)",
                        errorcode) )
        {
            __debugbreak();
        }
        if ( errorcode == ERR_DROP )
        {
            Com_PrintError(16, "********************\nERROR: %s\n********************\n", com_errorMessage);
            if ( cls.uiStarted && !com_fixedConsolePosition )
                CL_ConsoleFixPosition();
        }
        else
        {
            Com_Printf(16, "********************\nDisconnecting: %s\n********************\n", com_errorMessage);
        }
        Com_ShutdownInternal(finalmsg);
        if ( errorcode == ERR_DROP && QuitOnError() )
            Com_Quit_f();
    }
    //*(unsigned int *)(*((unsigned int *)NtCurrentTeb()->ThreadLocalStoragePointer + _tls_index) + 8) = 0;
    bgs = NULL;
    com_fixedConsolePosition = 0;
    NET_RestartDebug();
    com_errorEntered = 0;
    if ( errorcode == ERR_DISCONNECT )
        Monkey_Event("disconnected");
}

void Com_AddStartupCommands()
{
    int ControllerIndex; // eax
    int i; // [esp+0h] [ebp-414h]
    char localBuffer[1036]; // [esp+4h] [ebp-410h] BYREF

    for ( i = 0; i < com_numConsoleLines; ++i )
    {
        if ( *com_consoleLines[i] )
        {
            if ( !Com_StartupProcessSetCommand(i, 0) )
            {
                Com_sprintf(localBuffer, 0x401u, "%s\n", com_consoleLines[i]);
                ControllerIndex = Com_LocalClient_GetControllerIndex(0);
                Cbuf_ExecuteBuffer(0, ControllerIndex, localBuffer);
            }
        }
    }
}

void __cdecl Com_Init_Try_Block_Function(char *commandLine)
{
    char *BuildVersion; // eax
    int ControllerIndex; // eax
    void *v5; // ecx
    unsigned int v6; // eax
    const char *max; // [esp+8h] [ebp-5Ch]
    LARGE_INTEGER PerformanceCount; // [esp+18h] [ebp-4Ch] BYREF
    int localClientNum; // [esp+58h] [ebp-Ch]
    int initStartTime; // [esp+60h] [ebp-4h]

    max = Com_GetBuildName();
    BuildVersion = Com_GetBuildVersion();
    Com_Printf(16, "%s %s build %s %s\n", BuildVersion, max, "win-x86", "Nov    5 2010");
    Com_ParseCommandLine(commandLine);
    SL_Init(SCRIPTINSTANCE_SERVER);
    SL_Init(SCRIPTINSTANCE_CLIENT);
    Swap_Init();
    Cbuf_Init();
    Cmd_Init();
    Com_StartupVariable(0);
    Com_InitDvars();
    IK_InitSystem();
    initStartTime = 0;

    if ( useFastFile->current.enabled )
    {
        PMem_Init();
        LiveSteam_Init();
        DB_SetInitializing(1);
        Com_Printf(7, "begin $init\n");
        initStartTime = Sys_Milliseconds();
        PMem_BeginAlloc("$init", 1u, TRACK_MISC);
    }

    Stream_Init();
    //BLOPS_NULLSUB();

    if ( useFastFile->current.enabled )
        Com_InitCodeXAssets();

    CL_InitKeyCommands();
    CL_InitGamepadCommands();
    CL_InitGamepadAxisBindings();

    FS_InitFilesystem(1);

    Con_InitChannels();
    TaskManager2_Init();

#ifdef KISAK_LIVE
    dwInit();
#endif

    DDL_Init();
    Com_InitClientGameStates();
    Con_Restricted_SecureConfigs();

    for ( localClientNum = 0; localClientNum < 1; ++localClientNum )
    {
        Com_LocalClient_SetControllerIndex(localClientNum, localClientNum);
        Com_StartupConfigs(localClientNum);
    }

    ControllerIndex = Com_LocalClient_GetControllerIndex(0);
    Cbuf_Execute(0, ControllerIndex);
    Con_Restricted_InitLists();

    if ( (dvar_modifiedFlags & 0x20) != 0 )
        Com_InitDvars();

    R_StreamSetInitData(Com_CalculateStreamBuffer());
    com_recommendedSet = _Dvar_RegisterBool("com_recommendedSet", 0, 1u, "Use recommended settings");
    Com_CheckSetRecommended(0);
    Com_StartupVariable(0);

    if ( !useFastFile->current.enabled )
        SEH_UpdateLanguageInfo();

    if (IsDedicatedServer())
    {
        CL_InitDedicated();
    }

    Com_InitHunkMemory();
    Hunk_UserStartup();

    dvar_modifiedFlags &= ~1u;

    com_codeTimeScale = 1.0f;

    collectors = _Dvar_RegisterBool("collectors", 0, 0x40u, "Set to true if the player has the collector's edition");
    primaryWeaponOffset = _Dvar_RegisterInt(
                                                    "primaryWeaponOffset",
                                                    0,
                                                    0,
                                                    7,
                                                    0x40u,
                                                    "Primary Weapon Offset for CE and Presell");
    scr_xpcollectorsscale = _Dvar_RegisterInt("scr_xpcollectorsscale", 1, 0, 9, 0x40u, "");
    scr_xpscale = _Dvar_RegisterInt("scr_xpscale", 1, 0, 9, 0x40u, "");
    scr_xpzmscale = _Dvar_RegisterInt("scr_xpzmscale", 1, 0, 9, 0x40u, "");
    scr_codpointsxpscale = _Dvar_RegisterFloat("scr_codpointsxpscale", 0.1, 0.1, 0.1, 0x40u, "");
    scr_codpointsmatchscale = _Dvar_RegisterFloat("scr_codpointsmatchscale", 0.1, 0.1, 0.1, 0x40u, "");
    scr_codpointsperchallenge = _Dvar_RegisterFloat("scr_codpointsperchallenge", 0.1, 0.1, 0.1, 0x40u, "");
    scr_rankXpCap = _Dvar_RegisterInt("scr_rankXpCap", 0, 0, 0, 0x40u, "");
    scr_codPointsCap = _Dvar_RegisterInt("scr_codPointsCap", 0, 0, 0, 0x40u, "");

    ProfLoad_Init();

    if ( com_developer->current.integer )
    {
        Cmd_AddCommandInternal("error", Com_Error_f, &Com_Error_f_VAR);
        Cmd_AddCommandInternal("crash", Com_Crash_f, &Com_Crash_f_VAR);
        Cmd_AddCommandInternal("freeze", Com_Freeze_f, &Com_Freeze_f_VAR);
        Cmd_AddCommandInternal("assert", Com_Assert_f, &Com_Assert_f_VAR);
    }

    Cmd_AddCommandInternal("quit", (void(__cdecl *)())Com_Quit_f, &Com_Quit_f_VAR);
    Cmd_AddCommandInternal("writeconfig", Com_WriteConfig_f, &Com_WriteConfig_f_VAR);
    Cmd_AddCommandInternal("writekeyconfig", Com_WriteKeyConfig_f, &Com_WriteKeyConfig_f_VAR);
    Cmd_AddCommandInternal("savekeys", Com_SaveKeys_f, &Com_SaveKeys_f_VAR);
    Cmd_AddCommandInternal("restorekeys", Com_RestoreKeys_f, &Com_RestoreKeys_f_VAR);
    Cmd_AddCommandInternal("writedefaults", Com_WriteDefaults_f, &Com_WriteDefaults_f_VAR);

    version = _Dvar_RegisterString("version", (char *)"", 0x40u, "Game version");
    Dvar_SetString((dvar_s *)version, va("%s %s build %s %s", Com_GetBuildDisplayName(), Com_GetBuildName(), Com_GetBuildVersion(), "win-x86"));
    shortversion = _Dvar_RegisterString("shortversion", "7", 0x44u, "Short game version");

    Sys_Init();
    QueryPerformanceCounter(&PerformanceCount);
    Netchan_Init(PerformanceCount.QuadPart);

    Scr_InitVariables(SCRIPTINSTANCE_SERVER);
    Scr_Init(SCRIPTINSTANCE_SERVER);

    Com_SetScriptSettings();
    XAnimInit();
    DObjInit();
    SV_Init();
    NET_Init();
    RMsg_Init();

#ifndef KISAK_DEDICATED
    Dvar_ClearModified((dvar_s *)dedicated);
#endif

    if (!IsDedicatedServer())
    {
        CL_InitOnceForAllClients();
        for (int localClientNum = 0; localClientNum < 1; ++localClientNum)
        {
            CL_Init(localClientNum);
        }
        Com_LocalClient_SetPrimary(0, 1);
    }
    else
    {
        CL_InitOnceForAllClients();
        Com_LocalClient_SetPrimary(0, 1);
    }


    com_frameTime = Sys_Milliseconds();
    Com_StartupVariable(0);

#ifndef KISAK_DEDICATED
    if (IsDedicatedServer())
    {
        R_InitWorkerThreads();
    }
    else
    {
        r_smp_worker_threads = _Dvar_RegisterInt("r_smp_worker_threads",
            Sys_GetDefaultWorkerThreadsCount(),
            2,
            8,
            0,
            "Number of worker threads");

        R_InitThreads();

        //KISAK_NULLSUB();
        CL_InitRenderer();
        Expression_Init();
        //KISAK_NULLSUB();
        iassert(!cls.soundStarted);
        cls.soundStarted = 1;
        SND_Init();
        CG_SndGameReset();
        Voice_Init();
        R_Cinematic_Init();
    }
#else
    R_InitWorkerThreads();
#endif

    Sys_LoadingKeepAlive();
    Live_Init();
    PC_InitSigninState();
    Playlist_Init();
    R_BeginRemoteScreenUpdate();
    BG_EmblemsInit();
    SV_InitServerThread();
    Demo_InitFileHandlerSystem();
    mjpeg_initonce();
    COM_PlayIntroMovies();

    if ( useFastFile->current.enabled )
    {
        PMem_EndAlloc("$init", 1u);
        DB_SetInitializing(0);
        v6 = Sys_Milliseconds();
        Com_Printf(16, "end $init %d ms\n", v6 - initStartTime);
    }

    R_EndRemoteScreenUpdate(0);
    com_fullyInitialized = 1;
    Com_Printf(16, "--- Common Initialization Complete ---\n");
    Com_DvarDump(6, 0);
}

void __cdecl Com_ParseCommandLine(char *commandLine)
{
    com_consoleLines[0] = commandLine;
    com_numConsoleLines = 1;
    while ( *commandLine )
    {
        if ( *commandLine == 43 || *commandLine == 10 )
        {
            if ( com_numConsoleLines == 32 )
                return;
            com_consoleLines[com_numConsoleLines++] = commandLine + 1;
            *commandLine = 0;
        }
        ++commandLine;
    }
}

void __cdecl Com_Error_f()
{
    if ( Cmd_Argc() <= 1 )
        Com_Error(ERR_FATAL, "Testing fatal error");
    else
        Com_Error(ERR_DROP, "Testing drop error");
}

void __cdecl Com_Freeze_f()
{
    const char *v0; // eax
    unsigned int start; // [esp+10h] [ebp-Ch]
    float s; // [esp+18h] [ebp-4h]

    if ( Cmd_Argc() == 2 )
    {
        v0 = Cmd_Argv(1);
        s = atof(v0);
        start = Sys_Milliseconds();
        while ( (double)(int)(Sys_Milliseconds() - start) * 0.001 <= s )
            ;
    }
    else
    {
        Com_Printf(0, "freeze <seconds>\n");
    }
}

void __cdecl Com_Crash_f()
{
    //MEMORY[0] = 305419896;
    int *ptr = NULL;
    *ptr = 0x12345678;
}

void __cdecl Com_Assert_f()
{
    ;
}

void COM_PlayIntroMovies()
{
    if (!IsDedicatedServer())
    {
        // KISAKTODO: intro movies
    }
}

static const char *g_dedicatedEnumNames[4] = { "listen server", "dedicated LAN server", "dedicated internet server", NULL }; // idb

void Com_InitDvars()
{
    unsigned int CpuCount; // eax

    com_maxclients = _Dvar_RegisterInt("com_maxclients", 18, 1, 32, 0x44u, "Maximum amount of clients on the server");
    com_freemoveScale = _Dvar_RegisterFloat(
                                                "com_freemoveScale",
                                                1.0,
                                                0.0,
                                                5.0,
                                                0x80u,
                                                "Scale how fast you move in com_freemove mode");
    disconnected_ctrls = _Dvar_RegisterString(
                                                 "disconnected_ctrls",
                                                 (char *)"",
                                                 0,
                                                 "String representing the disconnected controllers");
    // KISAKTODO: cg_playerState (#1)
    com_first_time = _Dvar_RegisterInt(
                                         "com_first_time",
                                         0,
                                         0,
                                         1,
                                         0x4000u,
                                         "non zero if the profile has never run the game before (only accurate after the iis)");
    com_first_time_pc = _Dvar_RegisterInt(
                                                "com_first_time_pc",
                                                1,
                                                0,
                                                1,
                                                1u,
                                                "non zero if the profile has never run the game before (only accurate after the iis)");
    // KISAKTODO: cg_playerState (#2)
#ifndef KISAK_DEDICATED
    dedicated = _Dvar_RegisterEnum("dedicated", g_dedicatedEnumNames, 0, 32, "Dedicated Server");
    if (dedicated->current.integer)
        _Dvar_RegisterEnum("dedicated", g_dedicatedEnumNames, 0, 64, "Dedicated Server");
#endif
    com_maxfps = _Dvar_RegisterInt("com_maxfps", 85, 0, 1000, 1u, "Cap frames per second");
    arcademode = _Dvar_RegisterBool("arcademode", 0, 0x100u, "Current game is an arcade mode game");
    zombiemode = _Dvar_RegisterBool("zombiemode", 0, 0x40u, "Current game is an zombie game");
    legacy_zombiemode = _Dvar_RegisterBool("legacy_zombiemode", 0, 0x40u, "Current game is a legacy zombie game");
    zombieStopSplitScreen = _Dvar_RegisterBool(
                                                        "zombieStopSplitScreen",
                                                        0,
                                                        0x40u,
                                                        "Force Split Screen to Fullscreen (for HUD)");
    zombietron = _Dvar_RegisterBool("zombietron", 0, 0x40u, "Current game is an zombietron top down game");
    zombietron_discovered = _Dvar_RegisterBool("zombietron_discovered", 0, 0x4001u, "Zombietron mode discovered");
    zombiefive_discovered = _Dvar_RegisterBool("zombiefive_discovered", 0, 0x4001u, "Zombie Five map discovered");

    // KISAKTODO
    //zombiemode_path_minz_bias
    //zombietron_discovered_override
    //zombiefive_discovered_override

    _Dvar_RegisterBool(
        "zombiefive_norandomchar",
        0,
        0x4000u,
        "Forces no random character when following the end game credits");
    blackopsmode = _Dvar_RegisterBool("blackopsmode", 0, 0x100u, "Current game is a blackops game");
    spmode = _Dvar_RegisterBool("spmode", 0, 0x100u, "Current game is a sp game");
#ifdef KISAK_DEDICATED
    onlinegame = _Dvar_RegisterBool(
                                 "onlinegame",
                                 1,
                                 0x40u,
                                 "Current game is an online game with stats, custom classes, unlocks");
#else
    if (IsDedicatedServer())
    {
        onlinegame = _Dvar_RegisterBool(
            "onlinegame",
            true,
            0x40u,
            "Current game is an online game with stats, custom classes, unlocks");
    }
    else
    {
        onlinegame = _Dvar_RegisterBool(
            "onlinegame",
            false,
            0,
            "Current game is an online game with stats, custom classes, unlocks");
    }
#endif
    xblive_rankedmatch = _Dvar_RegisterBool("xblive_rankedmatch", 0, 4u, "Current game is a ranked match");
    xblive_privatematch = _Dvar_RegisterBool("xblive_privatematch", 0, 4u, "Current game is a private match");
    useFastFile = _Dvar_RegisterBool("useFastFile", 1, 0x10u, "Enables loading data from fast files.");
    sys_lockThreads = _Dvar_RegisterBool(
                                            "sys_lockThreads",
                                            0,
                                            0,
                                            "Prevents threads from changing CPUs; improves profiling and may fix some bugs, but can hurt performance");
    CpuCount = Sys_GetCpuCount();
    sys_smp_allowed = _Dvar_RegisterBool("sys_smp_allowed", CpuCount > 1, 0x10u, "Allow multi-threading");
#ifdef _DEBUG
    com_developer = _Dvar_RegisterInt("developer", 1, 0, 2, 0, "Enable development options");
#else 
    com_developer = _Dvar_RegisterInt("developer", 0, 0, 2, 0, "Enable development options");
#endif
    com_developer_script = _Dvar_RegisterBool("developer_script", 0, 0, "Enable developer script comments");
    com_script_debugger_smoke_test = _Dvar_RegisterBool(
                                                                         "script_debugger_smoke_test",
                                                                         0,
                                                                         0,
                                                                         "perform script debugger smoke test and exit");
    com_logfile = _Dvar_RegisterInt(
                                    "logfile",
                                    1,
                                    0,
                                    2,
                                    0,
                                    "Write to log file - 0 = disabled, 1 = async file write, 2 = Sync every write");
    if ( com_logfile && !com_logfile->current.integer )
        Dvar_SetInt((dvar_s *)com_logfile, 1);
    com_statmon = _Dvar_RegisterBool("com_statmon", 0, 0, "Draw stats monitor");
    com_timescale = _Dvar_RegisterFloat("com_timescale", 1.0, 0.001, 10.0, 0x11C0u, "Scale time of each frame");
    dev_timescale = _Dvar_RegisterFloat("timescale", 1.0, 0.001, 10.0, 0x180u, "Scale time of each frame");
    com_fixedtime = _Dvar_RegisterInt("fixedtime", 0, 0, 1000, 0x80u, "Use a fixed time rate for each frame");
    com_maxFrameTime = _Dvar_RegisterInt(
                                             "com_maxFrameTime",
                                             100,
                                             50,
                                             5000,
                                             0,
                                             "Time slows down if a frame takes longer than this many milliseconds");
    long_blocking_call = _Dvar_RegisterBool("long_blocking_call", 0, 0, "Enable SCR_DrawPleaseWait dialog");
    sv_network_warning = _Dvar_RegisterBool("sv_network_warning", 0, 0, "Alternative enable SCR_DrawPleaseWait dialog");
    cl_network_warning = _Dvar_RegisterBool("cl_network_warning", 0, 0, "Alternative enable SCR_DrawPleaseWait dialog");
    sv_paused = _Dvar_RegisterInt("sv_paused", 0, 0, 2, 0x40u, "Pause the server");
    cl_paused = _Dvar_RegisterInt("cl_paused", 0, 0, 2, 0, "Pause the client");
    cl_paused_simple = _Dvar_RegisterBool(
                                             "cl_paused_simple",
                                             0,
                                             0,
                                             "Toggling pause won't do any additional special processing if true.");
    com_sv_running = _Dvar_RegisterBool("sv_running", 0, 0x40u, "Server is running");
    com_show_tty_timestamps = _Dvar_RegisterBool("show_tty_timestamps", 0, 0, "Show timestamps in tty output");
    com_voip_resume_time = _Dvar_RegisterInt("com_voip_resume_time", 0, 0, 0x7FFFFFFF, 0, "Time at which voip can resume");
    com_voip_bandwidth_restricted = _Dvar_RegisterBool(
                                                                        "com_voip_bandwidth_restricted",
                                                                        1,
                                                                        0x40u,
                                                                        "Use VOIP inhibitor during high bandwidth usage");
    com_voip_disable_threshold = _Dvar_RegisterInt(
                                                                 "com_voip_disable_threshold",
                                                                 1200,
                                                                 0,
                                                                 0x7FFFFFFF,
                                                                 0,
                                                                 "Message size at which voip becomes disabled");
    com_filter_output = _Dvar_RegisterBool("com_filter_output", 0, 0, "Use console filters for filtering output.");
    com_introPlayed = _Dvar_RegisterBool("com_introPlayed", 0, 0, "Intro movie has been played");
    com_startupIntroPlayed = _Dvar_RegisterBool(
                                                         "com_startupIntroPlayed",
                                                         0,
                                                         1u,
                                                         "Game startup intro movie(s) has been played");
    com_desiredMenu = _Dvar_RegisterInt(
                                            "com_desiredMenu",
                                            0,
                                            0,
                                            0x7FFFFFFF,
                                            0,
                                            "Target menu to navigate to when possible");
    com_skipMovies = _Dvar_RegisterBool("com_skipMovies", 0, 0, "Skip intro movies");
    com_animCheck = _Dvar_RegisterBool("com_animCheck", 0, 0, "Check anim tree");
    com_hiDef = _Dvar_RegisterBool("hiDef", 1, 0x40u, "True if the game video is running in high-def.");
    com_wideScreen = _Dvar_RegisterBool(
                                         "wideScreen",
                                         1,
                                         0x40u,
                                         "True if the game video is running in 16x9 aspect, false if 4x3.");
    doublesided_raycasts = _Dvar_RegisterBool("doublesided_raycasts", 0, 0x80u, "turn on double sided ray casts");
    log_append = _Dvar_RegisterBool("log_append", 0, 1u, "Open log file in append mode");
    com_waitForStreamer = _Dvar_RegisterInt(
                                                    "waitForStreamer",
                                                    1,
                                                    0,
                                                    2,
                                                    0,
                                                    "1) wait for initial lowmips, 2) wait for full initial texture load.");
    dec20_Enabled = _Dvar_RegisterBool("dec20_Enabled", 0, 0x5000u, "enable dec20 terminal");
    band_demosystem = _Dvar_RegisterInt("band_demosystem", 64000, 0, 0x7FFFFFFF, 0, "demo system bandwidth req'd");
    band_2players = _Dvar_RegisterInt("band_2players", 64000, 0, 0x7FFFFFFF, 0, "2 player bandwidth req'd");
    band_4players = _Dvar_RegisterInt("band_4players", 128000, 0, 0x7FFFFFFF, 0, "4 player bandwidth req'd");
    band_6players = _Dvar_RegisterInt("band_6players", 192000, 0, 0x7FFFFFFF, 0, "8 player bandwidth req'd");
    band_8players = _Dvar_RegisterInt("band_8players", 256000, 0, 0x7FFFFFFF, 0, "8 player bandwidth req'd");
    band_12players = _Dvar_RegisterInt("band_12players", 384000, 0, 0x7FFFFFFF, 0, "12 player bandwidth req'd");
    band_18players = _Dvar_RegisterInt("band_18players", 580000, 0, 0x7FFFFFFF, 0, "18 player bandwidth req'd");
    band_lotsplayers = _Dvar_RegisterInt("band_lotsplayers", 900000, 0, 0x7FFFFFFF, 0, ">18 player bandwidth req'd");
    band_dedicated = _Dvar_RegisterInt("band_dedicated", 2048000, 0, 0x7FFFFFFF, 0, ">18 player bandwidth req'd");
    G_RegisterRegisterToolDvars();
    Pregame_RegisterDvars();

    // KISAKTODO: add cheats
    //dword_355E974 = _Dvar_RegisterBool("sv_EnableDevCheats", 0, 0x4000, &String);
    //dword_35A03DC = _Dvar_RegisterBool("sv_NoClip", 0, 0x4000, &String);
    //dword_359FB44 = _Dvar_RegisterBool("sv_FullAmmo", 0, 0x4000, &String);
    //dword_359EB1C = _Dvar_RegisterBool("sv_InfiniteSprint", 0, 0x4000, &String);
    //dword_359EB04 = _Dvar_RegisterBool("sv_RadarAlwaysOn", 0, 0x4000, &String);
    //dword_355E8F0 = _Dvar_RegisterBool("sv_Invisible", 0, 0x4000, &String);
    //dword_359EB0C = _Dvar_RegisterBool("sv_SuperPenetrate", 0, 0x4000, &String);
    //dword_35A03B8 = _Dvar_RegisterBool("sv_TripleBullet", 0, 0x4000, &String);
    //dword_355E8B0 = _Dvar_RegisterBool("sv_QuickHealthRecharge", 0, 0x4000, &String);
    //dword_355E958 = _Dvar_RegisterBool("sv_InstantReload", 0, 0x4000, &String);
    //dword_35A0364 = _Dvar_RegisterBool("sv_3xEXP", 0, 0x4000, &String);
    //dword_359EA10 = _Dvar_RegisterBool("sv_UnlockAllIntel", 0, 0x4000, &String);
    //dword_35B03F4 = _Dvar_RegisterBool("sv_UnlockAllSlots", 0, 0x4000, &String);
    //dword_35A03A4 = _Dvar_RegisterBool("sv_DoubleCodPoints", 0, 0x4000, &String);
    //dword_355E8C0 = _Dvar_RegisterBool("sv_SetAllFree", 0, 0x4000, &String);
    //dword_35B03FC = _Dvar_RegisterBool("sv_EnableSuperuser", 0, 0x4000, &String);
    //dword_359EA28 = _Dvar_RegisterBool("sv_MakeMeHost", 0, 0x4000, &String);
    //dword_35A035C = _Dvar_RegisterBool("sv_DisableTheatre", 0, 0x4000, &String);
}

void __cdecl Com_StartupConfigs(int localClientNum)
{
    if ( Con_Restricted_ValidateConfig("config_mp.cfg") )
        Com_ExecStartupConfigs(localClientNum, "config_mp.cfg");
    else
        Com_ExecStartupConfigs(localClientNum, 0);
}

int g_loadedPreXAssets = 0;
void __cdecl Com_InitCodeXAssets()
{
    XZoneInfo zoneInfo[4]; // [esp+0h] [ebp-34h] BYREF
    unsigned int zoneCount; // [esp+30h] [ebp-4h]

    DB_InitThread();
    if ( !g_loadedPreXAssets )
    {
        g_loadedPreXAssets = 1;
        zoneInfo[0].name = "code_pre_gfx_mp";
        zoneInfo[0].allocFlags = 1;
        zoneInfo[0].freeFlags = 0;
        zoneCount = 1;
        DB_LoadXAssets(zoneInfo, 1u, 0);
        DB_SyncXAssets();
    }
}

void __cdecl Com_WriteDefaultsToFile(char *filename)
{
    int f; // [esp+0h] [ebp-4h]

    f = FS_FOpenFileWrite(filename);
    if ( f )
    {
        FS_Printf(f, (char*)"// generated by Call of Duty, do not modify\n");
        Dvar_WriteDefaults(f);
        FS_FCloseFile(f);
    }
    else
    {
        Com_Printf(16, "Couldn't write %s.\n", filename);
    }
}

void __cdecl Com_WriteConfig_f()
{
    const char *v0; // eax
    char filename[132]; // [esp+10h] [ebp-88h] BYREF

    if ( Cmd_Argc() == 2 )
    {
        v0 = Cmd_Argv(1);
        I_strncpyz(filename, v0, 128);
        Com_DefaultExtension(filename, 0x80u, ".cfg");
        Com_Printf(0, "Writing %s.\n", filename);
        Com_WriteConfigToFile(0, filename);
    }
    else
    {
        Com_Printf(0, "Usage: writeconfig <filename>\n");
    }
}

void __cdecl Com_WriteConfigToFile(int localClientNum, char *filename)
{
    int f; // [esp+4h] [ebp-4h]

    f = FS_FOpenFileWriteToDir(filename, (char*)"players", fs_homepath->current.string);
    if ( f )
    {
        FS_Printf(f, (char*)"// generated by Call of Duty, do not modify\n");
#ifndef KISAK_DEDICATED
        if (!IsDedicatedServer())
        {
            FS_Printf(f, (char*)"unbindall\n");
            Key_WriteBindings(localClientNum, f);
            Gamepad_WriteBindings(localClientNum, f);
        }
#else
        Gamepad_WriteBindings(localClientNum, f);
#endif
        Dvar_WriteVariables(f);
        Con_WriteFilterConfigString(f);
        FS_FCloseFile(f);
    }
    else
    {
        Com_Printf(16, "Couldn't write %s.\n", filename);
    }
}

void __cdecl Com_WriteKeyConfig_f()
{
    const char *v0; // eax
    char filename[132]; // [esp+10h] [ebp-88h] BYREF

    if ( Cmd_Argc() == 2 )
    {
        v0 = Cmd_Argv(1);
        I_strncpyz(filename, v0, 128);
        Com_DefaultExtension(filename, 0x80u, ".cfg");
        Com_Printf(0, "Writing %s.\n", filename);
        Com_WriteKeyConfigToFile(0, filename);
    }
    else
    {
        Com_Printf(0, "Usage: writekeyconfig <filename>\n");
    }
}

void __cdecl Com_WriteKeyConfigToFile(int localClientNum, char *filename)
{
    const char *v2; // eax
    int f; // [esp+4h] [ebp-24h]
    const char *dvars[6]; // [esp+8h] [ebp-20h]
    int i; // [esp+20h] [ebp-8h]
    const dvar_s *dvar; // [esp+24h] [ebp-4h]

    f = FS_FOpenFileWriteToDir(filename, (char*)"players", fs_homepath->current.string);
    if ( f )
    {
        FS_Printf(f, (char *)"// generated by Call of Duty, do not modify\n");

#ifndef KISAK_DEDICATED
        if (!IsDedicatedServer())
        {
            FS_Printf(f, (char*)"unbindall\n");
            FS_Printf(f, (char*)"unbindallaxis\n");
            Key_WriteBindings(localClientNum, f);
        }
#endif
        dvars[0] = "sensitivity";
        dvars[1] = "cl_freelook";
        dvars[2] = "ui_mousePitch";
        dvars[3] = "m_pitch";
        dvars[4] = "m_filter";
        dvars[5] = "cl_mouseAccel";
        for ( i = 0; (unsigned int)i < 6; ++i )
        {
            dvar = Dvar_FindVar(dvars[i]);
            if ( dvar )
            {
                v2 = Dvar_DisplayableValue(dvar);
                FS_Printf(f, (char *)"set %s \"%s\"\n", dvar->name, v2);
            }
        }
        FS_FCloseFile(f);
    }
    else
    {
        Com_Printf(16, "Couldn't write %s.\n", filename);
    }
}

void __cdecl Com_SaveKeys_f()
{
    Key_WriteBindingsToTempBuf(0);
}

void __cdecl Com_RestoreKeys_f()
{
    Key_RestoreBindingsFromTempBuf(0);
}

void __cdecl Com_WriteDefaults_f()
{
    const char *v0; // eax
    char filename[132]; // [esp+10h] [ebp-88h] BYREF

    if ( Cmd_Argc() == 2 )
    {
        v0 = Cmd_Argv(1);
        I_strncpyz(filename, v0, 128);
        Com_DefaultExtension(filename, 0x80u, ".cfg");
        Com_Printf(0, "Writing %s.\n", filename);
        Com_WriteDefaultsToFile(filename);
    }
    else
    {
        Com_Printf(0, "Usage: writedefaults <filename>\n");
    }
}

double __cdecl Com_GetTimescaleForSnd()
{
    if ( com_fixedtime->current.integer )
        return (double)com_fixedtime->current.integer;
    else
        return com_timescale->current.value * dev_timescale->current.value * com_codeTimeScale;
}

void Com_LoadUiFastFile()
{
    // this doesn't show in IDA, but you can actually find it in the asm here
    // I just byte patch the ASM to 1 instead of 0 in a cmp and it shows it.
    XZoneInfo zoneInfo[4];

    if (!IsDedicatedServer())
    {
        unsigned int zone = 0;

        if (IsFastFileLoad())
        {
            zoneInfo[zone].name = 0;
            zoneInfo[zone].allocFlags = 0;
            zoneInfo[zone].freeFlags = 0x800;
            zone++;
            DB_LoadXAssets(zoneInfo, zone, 0);
        }

        CL_AllocatePerLocalClientMemory();
        RB_Resource_Callback(Com_InitUI3DCallback);
        RB_Resource_Flush();
        R_UI3D_SetupTextureWindow(0, 0.0f, 0.0f, 1.0f, 1.0f);

        ScreenPlacement *scrPlace = R_UI3D_ScrPlaceFromTextureWindow(0);
        ScrPlace_SetupUI3DForFullscreen(scrPlace, &scrPlaceFull);

        zone = 0;
#ifdef KISAK_DEDICATED
        zoneInfo[zone].name = "ui_mp";
        zoneInfo[zone].allocFlags = 0x4000000;
        zoneInfo[zone].freeFlags = 0;
        zone++;
#else
        //zoneInfo[zone].name = "patch_ui_mp";
        //zoneInfo[zone].allocFlags = 0x4000000;
        //zoneInfo[zone].freeFlags = 0;
        //zone++;

        zoneInfo[zone].name = "ui_mp";
        //zoneInfo[zone].allocFlags = 0x2000000;
        zoneInfo[zone].allocFlags = 0x4000000;//  0x2000000; (KISAKTODO: flag fix, this flag doesn't load for some reason)
        zoneInfo[zone].freeFlags = 0;
        zone++;

        DB_LoadXAssets(zoneInfo, zone, 0);
#endif

        zone = 0;

        zoneInfo[zone].name = "ui_viewer_mp";
#ifdef KISAK_DEDICATED
        zoneInfo[zone].allocFlags = 0x2000000;
#else
        zoneInfo[zone].allocFlags = 2048;
#endif
        zoneInfo[zone].freeFlags = 0;
        zone++;

        if (fs_usermapDir && fs_usermapDir->current.string[0])
        {
            Dvar_SetStringByName("fs_usermapdir", (char*)"");
        }

        DB_LoadXAssets(zoneInfo, zone, 0);
    }
}

void __cdecl Com_LoadMapLoadingScreenFastFile(const char *mapName)
{
    if (!IsDedicatedServer())
    {
        //track_set_max_memory_level(mapName);
        DB_ResetZoneSize(0);
        if (useFastFile->current.enabled)
            DB_ReleaseXAssets();
        UI_SetLoadingScreenMaterial(mapName);
    }
}

void __cdecl Com_UnloadLevelFastFiles()
{
    XZoneInfo zoneInfo[1]; // [esp+4h] [ebp-Ch] BYREF

    if ( useFastFile->current.enabled )
    {
        zoneInfo[0].name = 0;
        zoneInfo[0].allocFlags = 0;
        zoneInfo[0].freeFlags = 0x4000;
        DB_LoadXAssets(zoneInfo, 1u, 0);
    }
}

void __cdecl Com_LoadLevelFastFiles(char *mapName)
{
    int ControllerIndex; // eax
    const char *basename; // [esp+44h] [ebp-C4h]
    XZoneInfo zoneInfo[4]; // [esp+48h] [ebp-C0h] BYREF
    char specOpsZoneName[68]; // [esp+78h] [ebp-90h] BYREF
    const char *levelSharedFastFile; // [esp+BCh] [ebp-4Ch]
    char levelPatchZoneName[64]; // [esp+C0h] [ebp-48h] BYREF
    int zoneCount; // [esp+104h] [ebp-4h]

    zoneCount = 0;
    DB_ResetZoneSize(0);
    UI_SetLoadingScreenMaterial(mapName);
    Com_sprintf(levelPatchZoneName, 0x40u, "%s_patch", mapName);
    ControllerIndex = Com_LocalClient_GetControllerIndex(0);
    Cbuf_ExecuteBuffer(0, ControllerIndex, (char *)"ui_animate connect * meet 500 1;\n");
    DB_AddUserMapDir(mapName);
    zoneInfo[zoneCount].name = 0;
    zoneInfo[zoneCount].allocFlags = 0;
    zoneInfo[zoneCount++].freeFlags = 0x4000000;
    if ( I_stristr(mapName, "zombietron") )
    {
        Dvar_SetBool((dvar_s *)zombiemode, 1);
        Dvar_SetBool((dvar_s *)zombietron, 1);
    }
    if ( I_stristr(mapName, "zombie_cod5_") )
    {
        Dvar_SetBool((dvar_s *)zombiemode, 1);
        Dvar_SetBool((dvar_s *)legacy_zombiemode, 1);
    }
    else
    {
        Dvar_SetBool((dvar_s *)legacy_zombiemode, 0);
    }
    if ( !Com_IsMenuLevel(mapName) )
        Com_LoadCommonFastFile();
    levelSharedFastFile = Com_GetLevelSharedFastFile(mapName);
    if ( levelSharedFastFile )
    {
        zoneInfo[zoneCount].name = levelSharedFastFile;
        zoneInfo[zoneCount].allocFlags = 4096;
        zoneInfo[zoneCount++].freeFlags = 0;
    }
    if ( !I_strncmp("so_", mapName, strlen("so_")) )
    {
        for ( basename = &mapName[strlen("so_")]; *basename && *basename != 95; ++basename )
            ;
        if ( !*basename )
            Com_PrintError(1, "Bad specop level name\n");
        Com_sprintf(specOpsZoneName, 0x40u, "%s", basename + 1);
        zoneInfo[zoneCount].name = specOpsZoneName;
        zoneInfo[zoneCount].allocFlags = 0x4000;
        zoneInfo[zoneCount++].freeFlags = 0;
    }
    zoneInfo[zoneCount].name = mapName;
    if ( I_strncmp("so_", mapName, strlen("so_")) )
        zoneInfo[zoneCount].allocFlags = 0x4000;
    else
        zoneInfo[zoneCount].allocFlags = 0x10000;
    zoneInfo[zoneCount++].freeFlags = 0;
    R_BeginRemoteScreenUpdate();
    DB_LoadXAssets(zoneInfo, zoneCount, 0);
    R_EndRemoteScreenUpdate(0);
}

int gLevelDependenciesInited = 0;
int gLevelDependenciesCount;
char gLevelDependencies[16][2][64];
char *__cdecl Com_GetLevelSharedFastFile(char *mapName)
{
    int i; // [esp+0h] [ebp-4010h]
    char *data_p; // [esp+4h] [ebp-400Ch] BYREF
    char loadBuffer[16384]; // [esp+8h] [ebp-4008h] BYREF
    char *src; // [esp+400Ch] [ebp-4h]

    if ( !gLevelDependenciesInited )
    {
        gLevelDependenciesInited = 1;
        data_p = Com_LoadInfoString((char*)"level_dependencies.csv", "level_dependency_info", "", loadBuffer);
        Com_BeginParseSession("level_dependencies.csv");
        Com_SetCSV(1);
        while ( 1 )
        {
            src = (char *)Com_Parse((const char **)&data_p);
            if ( !*src )
                break;
            if ( *src != 47 )
            {
                if ( gLevelDependenciesCount >= 16 )
                {
                    Com_PrintWarning(10, "Failed to load level dependencies: Max is %d\n", 16);
                }
                else
                {
                    I_strncpyz(gLevelDependencies[gLevelDependenciesCount][0], src, 64);
                    src = (char *)Com_Parse((const char **)&data_p);
                    I_strncpyz(gLevelDependencies[gLevelDependenciesCount][1], src, 64);
                    ++gLevelDependenciesCount;
                }
            }
        }
        Com_EndParseSession();
    }
    for ( i = 0; i < gLevelDependenciesCount; ++i )
    {
        if ( !I_strcmp(gLevelDependencies[i][0], mapName) )
            return gLevelDependencies[i][1];
    }
    return 0;
}

void Com_LoadCommonFastFile()
{
    XZoneInfo zoneInfo[1]; // [esp+4h] [ebp-10h] BYREF
    int zoneCount; // [esp+10h] [ebp-4h]

    zoneCount = 0;
    DB_ResetZoneSize(0);
    if ( useFastFile->current.enabled )
        DB_ReleaseXAssets();
    if ( zombietron->current.enabled )
    {
        zoneInfo[zoneCount].name = 0;
        zoneInfo[zoneCount].allocFlags = 0;
        zoneInfo[zoneCount++].freeFlags = 256;
    }
    else if ( zombiemode->current.enabled )
    {
        if ( legacy_zombiemode->current.enabled )
        {
            if ( !DB_IsZoneLoaded("waw_zombie") )
            {
                zoneInfo[zoneCount].name = "waw_zombie";
                zoneInfo[zoneCount].allocFlags = 256;
                zoneInfo[zoneCount++].freeFlags = 0;
            }
        }
        else if ( !DB_IsZoneLoaded("common_zombie") )
        {
            zoneInfo[zoneCount].name = "common_zombie";
            zoneInfo[zoneCount].allocFlags = 256;
            zoneInfo[zoneCount++].freeFlags = 0;
        }
    }
    else if ( !DB_IsZoneLoaded("common_mp") )
    {
        zoneInfo[zoneCount].name = "common_mp";
        zoneInfo[zoneCount].allocFlags = 256;
        zoneInfo[zoneCount++].freeFlags = 0;
    }
    if ( zoneCount )
        DB_LoadXAssets(zoneInfo, zoneCount, 0);
}

void __cdecl Com_LoadFrontEnd()
{
    Dvar_SetBool((dvar_s *)xblive_matchEndingSoon, 0);
    if (!IsDedicatedServer())
    {
        CL_SetupClientsForFrontend();
    }
}

void __cdecl Com_UnloadFrontEnd()
{
    bool shutdown; // [esp+3h] [ebp-11h]
    int localClientNum; // [esp+4h] [ebp-10h]
    XZoneInfo zoneInfo[1]; // [esp+8h] [ebp-Ch] BYREF

    UI_ViewerShutdown();
    for ( localClientNum = 0; localClientNum < 1; ++localClientNum )
        UI_CloseAll(localClientNum);
    if ( useFastFile->current.enabled )
    {
        shutdown = 0;
        DB_ReleaseXAssets();
        if ( DB_IsZoneTypeLoaded(0x1000000) && cls.rendererStarted )
        {
            CL_ShutdownWorld();
            shutdown = 1;
        }
        zoneInfo[0].name = 0;
        zoneInfo[0].allocFlags = 0;
        zoneInfo[0].freeFlags = 0x4000000;
        DB_LoadXAssets(zoneInfo, 1u, 0);
        if ( shutdown )
            CL_InitRenderer();
    }
    R_UI3D_Shutdown();
}

void __cdecl Com_AssetLoadUI()
{
    if ( useFastFile->current.enabled )
    {
        Com_LoadCommonFastFile();
        Com_LoadUiFastFile();
    }
    UI_SetMap((char *)"", "");
    R_BeginRemoteScreenUpdate();
    CL_StartHunkUsers();
    R_EndRemoteScreenUpdate(0);
    Com_LoadFrontEnd();
}

void __cdecl Com_ResetFrametime()
{
    unsigned int timeMsec; // [esp+0h] [ebp-8h]
    unsigned int lastFrameIndex; // [esp+4h] [ebp-4h]

    timeMsec = Sys_Milliseconds();
    for ( lastFrameIndex = 0; !lastFrameIndex; lastFrameIndex = 1 )
        com_lastFrameTime[0] = timeMsec;
}

void __cdecl Com_CheckSyncFrame()
{
    PROF_SCOPED("Com_CheckSyncFrame"); // LWSS ADD

    Scr_UpdateRemoteDebugger(SCRIPTINSTANCE_SERVER);
    DB_Update();
    UI_ViewerCheckStreamer();
}

void __cdecl Com_Frame()
{
    void *Value; // eax

    proftimer_physics_frame_advance.reset();
    sv_flame_proftimer.reset();
    cl_flame_proftimer.reset();
    Value = Sys_GetValue(2);
    //if ( !_setjmp3(Value, 0) )
    if ( !_setjmp((int*)Value) )
    {
        Com_CheckSyncFrame();
        Com_Frame_Try_Block_Function();
        ++com_frameNumber;
    }

    Sys_EnterCriticalSection(CRITSECT_COM_ERROR);
    if (com_errorEntered)
    {
        Com_ErrorCleanup();
        Sys_LeaveCriticalSection(CRITSECT_COM_ERROR);
        if (!IsDedicatedServer())
        {
            CL_InitRenderer();
            Com_StartHunkUsers();
        }
    }
    else
    {
        Sys_LeaveCriticalSection(CRITSECT_COM_ERROR);
    }

    FrameMark;
}

unsigned int Com_Frame_Try_Block_Function()
{
    CmdArgs *v0; // eax
    unsigned int v1; // edx
    unsigned int result; // eax
    int localControllerIndex; // [esp+3Ch] [ebp-20h]
    int i; // [esp+44h] [ebp-18h]
    int lastFrameIndex; // [esp+48h] [ebp-14h]
    int msec; // [esp+4Ch] [ebp-10h]
    int mseca; // [esp+4Ch] [ebp-10h]
    int maxFPS; // [esp+58h] [ebp-4h] BYREF
    int minMsec;

    iassert(Cmd_Args()->nesting == -1);

    PROF_SCOPED("Com_Frame");

    Com_WriteConfiguration(0);
    Sys_UpdateHotkeyBlock();
    SetAnimCheck(com_animCheck->current.color[0], SCRIPTINSTANCE_SERVER);

    minMsec = 1;
    maxFPS = com_maxfps->current.integer;
    Com_AdjustMaxFPS(&maxFPS);

    if (!IsDedicatedServer() && maxFPS > 0)
    {
        minMsec = 1000 / maxFPS;
        iassert(minMsec >= 0);
        if (!minMsec)
            minMsec = 1;
    }

    if ( sys_lockThreads->modified )
    {
        Dvar_ClearModified(sys_lockThreads);
        if ( sys_lockThreads->current.enabled )
            Win_LockThreadAffinity();
        else
            Win_UnlockThreadAffinity();
    }

    v1 = com_lastFrameIndex & 0x80000000;
    if ( com_lastFrameIndex < 0 )
        v1 = 0;
    lastFrameIndex = v1;
    ++com_lastFrameIndex;

    if (IsDedicatedServer())
    {
        for (i = 0; i < 50; ++i)
        {
            Com_EventLoop();
            com_frameTime = Sys_Milliseconds();
            if (com_frameTime - com_lastFrameTime[lastFrameIndex] < 0)
                com_lastFrameTime[lastFrameIndex] = com_frameTime;
            msec = com_frameTime - com_lastFrameTime[lastFrameIndex];
            if (msec >= minMsec)
                break;
            NET_Sleep(1u);
        }
        com_lastFrameTime[lastFrameIndex] = com_frameTime;
    }
    else
    {
        // adapted from kcod4
        for (int i = 0; i < 50; ++i)
        {
            Com_EventLoop();
            com_frameTime = Sys_Milliseconds();
            if (com_frameTime - com_lastFrameTime[lastFrameIndex] < 0)
                com_lastFrameTime[lastFrameIndex] = com_frameTime;

            if (com_frameTime - com_lastFrameTime[lastFrameIndex] > 0)
            {
                break;
            }
            NET_Sleep(1u);
        }

        int v4;
        if (com_frameTime - com_lastFrameTime[lastFrameIndex] < minMsec)
            v4 = minMsec;
        else
            v4 = com_frameTime - com_lastFrameTime[lastFrameIndex];
        com_lastFrameTime[lastFrameIndex] += v4;
        msec = lastFrameIndex + v4;
        if (!(lastFrameIndex + v4))
            msec = 1;
    }

    for ( localControllerIndex = 0; localControllerIndex < 1; ++localControllerIndex )
        TaskManager2_ProcessTasks(localControllerIndex);

    Cbuf_Execute(0, Com_LocalClient_GetControllerIndex(0));
    ProcessStringEdCmds();
    ProcessGDTCmds();

    if ( Demo_IsPlaying() && msec > maxDemoMsec )
        msec = maxDemoMsec;

    cls.inputRealMsec = msec;
    Demo_Frame(msec);
    mseca = Com_ModifyMsec(msec);
    LiveSteam_Frame();

    {
        PROF_SCOPED("SV frame");
        SV_Frame(Com_LocalClient_GetControllerIndex(0), mseca);
    }

    Monkey_Frame();

    //BLOPS_NULLSUB();

    if (IsDedicatedServer())
    {
        Phys_RunToTime(svsHeader.time);
#ifdef KISAK_LIVE
        DWDedicatedLobbyPump();
#endif
    }
    else
    {
        R_SetEndTime(com_lastFrameTime[lastFrameIndex]);
        {
            PROF_SCOPED("pre frame");
            CL_RunOncePerClientFrame(Com_LocalClients_GetPrimary(), mseca);
            Com_EventLoop();
            for (int localClientNum = 0; localClientNum < 1; ++localClientNum)
            {
                if (Demo_IsPlaying())
                    Demo_UpdateDesiredTime(localClientNum);
                Cbuf_Execute(localClientNum, Com_LocalClient_GetControllerIndex(localClientNum));
            }
            //BG_EvalVehicleName(0);
        }
        
        RMsg_SendMessages();

        {
            PROF_SCOPED("CL_Frame");
            for (int localClientNuma = 0; localClientNuma < 1; ++localClientNuma)
                CL_Frame(localClientNuma, mseca);
        }

        dvar_modifiedFlags &= ~2u;
        Com_UpdateMenu();
        CG_UpdateClouds(mseca);
        PhysicsSystem_Update();
        //BG_EvalVehicleName(0);
        SCR_UpdateScreen();
        gjk_collision_epilog(0);
        //BG_EvalVehicleName(v8);
        for (int localClientNumb = 0; localClientNumb < 1; ++localClientNumb)
            DevGui_Update(localClientNumb, (float)cls.frametime * 0.001);
        Com_Statmon();
        R_WaitEndTime();
    }
    
    result = GetCurrentThreadId();
    return result;
}

void __cdecl Com_WriteConfiguration(int localClientNum)
{
    char configFile[132]; // [esp+0h] [ebp-88h] BYREF

    if ( com_fullyInitialized )
    {
        if ( (dvar_modifiedFlags & 1) != 0 )
        {
            dvar_modifiedFlags &= ~1u;
            I_strncpyz(configFile, "config_mp.cfg", 128);
            Com_WriteConfigToFile(localClientNum, configFile);
            //BLOPS_NULLSUB();
        }
    }
}

int __cdecl Com_ModifyMsec(int msec)
{
    float v2; // [esp+0h] [ebp-20h]
    int originalMsec; // [esp+18h] [ebp-8h]
    bool useTimescale; // [esp+1Fh] [ebp-1h]
    int clampTime;

    originalMsec = msec;
    if ( com_fixedtime->current.integer )
    {
        msec = com_fixedtime->current.integer;
        useTimescale = 1;
    }
    else if ( com_timescale->current.value == 1.0 && com_codeTimeScale == 1.0 && dev_timescale->current.value == 1.0 )
    {
        useTimescale = 0;
    }
    else
    {
        msec = (int)((float)((float)((float)((float)msec * com_timescale->current.value) * com_codeTimeScale)
                                             * dev_timescale->current.value)
                             + 9.313225746154785e-10);
        useTimescale = 1;
    }
    if ( msec < 1 )
        msec = 1;
    if (IsDedicatedServer())
    {
        if (msec > 500 && msec < 500000)
            Com_PrintWarning(16, "Hitch warning: %i msec frame time\n", msec);
        clampTime = 5000;
    }
    else if (com_sv_running->current.enabled)
    {
        clampTime = com_maxFrameTime->current.integer;
    }
    else
    {
        clampTime = 5000;
    }

    if ( msec > clampTime)
        msec = clampTime;

    if ( useTimescale && originalMsec )
        v2 = (float)msec / (float)originalMsec;
    else
        v2 = 1.0f;

    com_timescaleValue = v2;

    return msec;
}

void Com_Statmon()
{
    int timePrevFrame; // [esp+0h] [ebp-4h]

    if ( com_statmon->current.enabled )
    {
        if ( com_fileAccessed )
        {
            StatMon_Warning(1, 3000, (char*)"code_warning_file");
            com_fileAccessed = 0;
        }
        timePrevFrame = timeClientFrame;
        timeClientFrame = Sys_Milliseconds();
        if ( com_statmon->current.enabled )
        {
            if ( timeClientFrame - timePrevFrame > 33 && timePrevFrame )
                StatMon_Warning(0, 3000, (char *)"code_warning_fps");
            if (sv.serverFrameTimeMax > 50)
                StatMon_Warning(6, 3000, (char *)"code_warning_serverfps");
        }
    }
}

void __cdecl Com_AdjustMaxFPS(int *maxFPS)
{
    int maxUserCmdsPerSecond; // [esp+0h] [ebp-4h]

    if ( com_timescaleValue < 1.0 )
    {
        maxUserCmdsPerSecond = (int)(float)(320.0 * com_timescaleValue);
        if ( maxUserCmdsPerSecond < 1 )
            maxUserCmdsPerSecond = 1;
        if ( !*maxFPS || *maxFPS > maxUserCmdsPerSecond )
            *maxFPS = maxUserCmdsPerSecond;
    }
}

char Com_UpdateMenu()
{
    PROF_SCOPED("Com_UpdateMenu"); // LWSS ADD

    int IsFullscreen; // eax
    uiMenuCommand_t MenuScreen; // eax
    connstate_t clcState; // [esp+4h] [ebp-4h]

    clcState = CL_GetLocalClientConnectionState(0);
    IsFullscreen = UI_IsFullscreen(0);
    if ( !IsFullscreen && (clcState == CA_DISCONNECTED || clcState == CA_UICINEMATIC) )
    {
        IsFullscreen = CG_IsShowingZombieMap();
        if ( !(_BYTE)IsFullscreen )
        {
            MenuScreen = (uiMenuCommand_t)UI_GetMenuScreen();
            IsFullscreen = UI_SetActiveMenu(0, MenuScreen);
        }
    }
    return IsFullscreen;
}

void Com_StartHunkUsers()
{
    void *Value; // eax
    int Primary; // eax
    uiMenuCommand_t MenuScreen; // [esp-4h] [ebp-4h]

    Value = Sys_GetValue(2);
    //if ( _setjmp3(Value, 0) )
    if (_setjmp((int *)Value))
    {
        Sys_Error((char*)"Error during initialization:\n%s\n", com_errorMessage);

    }
    Com_AssetLoadUI();
    MenuScreen = (uiMenuCommand_t)UI_GetMenuScreen();
    Primary = Com_LocalClients_GetPrimary();
    UI_SetActiveMenu(Primary, MenuScreen);
    IN_Frame();
    Com_EventLoop();
}

void __cdecl Com_CloseLogfiles()
{
    if ( logfile )
    {
        FS_FCloseLogFile(logfile);
        logfile = 0;
    }
}

bool __cdecl Com_LogFileOpen()
{
    return logfile != 0;
}

void __cdecl Com_Close()
{
    Com_ShutdownDObj();
    DObjShutdown();
    XAnimShutdown();
    R_FreeWaterSimulationBuffers();
    Com_ShutdownWorld();
    CM_Shutdown();
    UI_ScreenshotShutdown();
    Live_FileShare_CacheShutdown();
    Hunk_Clear();
    if ( useFastFile->current.enabled )
        DB_ShutdownXAssets();
    Scr_Shutdown(SCRIPTINSTANCE_SERVER);
    Scr_Shutdown(SCRIPTINSTANCE_CLIENT);
    NET_ShutdownDebug();
    Hunk_UserShutdown();
}

void __cdecl Field_Clear(field_t *edit)
{
    memset((unsigned __int8 *)edit->buffer, 0, sizeof(edit->buffer));
    edit->cursor = 0;
    edit->scroll = 0;
    edit->drawWidth = 256;
}

void __cdecl Com_Restart()
{
    XZoneInfo zoneInfo[1]; // [esp+4h] [ebp-Ch] BYREF

    com_codeTimeScale = 1.0f;
    CL_ShutdownHunkUsers();
    SV_ShutdownGameProgs();
    Com_ShutdownDObj();
    DObjShutdown();
    XAnimShutdown();
    //BLOPS_NULLSUB();
    Com_ShutdownDynamicMemorySystems();
    if ( useFastFile->current.enabled )
    {
        zoneInfo[0].name = 0;
        zoneInfo[0].allocFlags = 0;
        zoneInfo[0].freeFlags = 0x400000;
        DB_LoadXAssets(zoneInfo, 1u, 0);
    }
    Com_ShutdownWorld();
    CM_Shutdown();
    UI_ScreenshotShutdown();
    Hunk_Clear();
    Hunk_UserReset(g_DebugHunkUser);
    CL_ShutdownDebugData();
    if ( useFastFile->current.enabled )
        DB_ReleaseXAssets();
    Com_SetScriptSettings();
    com_fixedConsolePosition = 0;
    XAnimInit();
    DObjInit();
    Com_InitDObj();
    Flame_Init();
}

XAnimTree_s *__cdecl Com_XAnimCreateSmallTree(XAnim_s *anims)
{
    return XAnimCreateTree(anims, (void *(__cdecl *)(unsigned int))CG_AllocAnimTree);
}

void __cdecl Com_XAnimFreeSmallTree(XAnimTree_s *animtree)
{
    XAnimFreeTree(animtree, (void (__cdecl *)(void *, int, scriptInstance_t))MT_Free, SCRIPTINSTANCE_SERVER);
}

void __cdecl Com_SetWeaponInfoMemory(int source)
{
    weaponInfoSource = source;
}

void __cdecl Com_FreeWeaponInfoMemory(int source)
{
    if ( source == weaponInfoSource )
    {
        weaponInfoSource = 0;
        BG_ShutdownWeaponDefFiles();
    }
}

int __cdecl Com_AddToString(const char *add, char *msg, int len, int maxlen, int mayAddQuotes)
{
    int addQuotes; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]
    int ia; // [esp+4h] [ebp-4h]

    addQuotes = 0;
    if ( mayAddQuotes )
    {
        if ( *add )
        {
            for ( i = 0; i < maxlen - len && add[i]; ++i )
            {
                if ( add[i] <= 32 )
                {
                    addQuotes = 1;
                    break;
                }
            }
        }
        else
        {
            addQuotes = 1;
        }
    }
    if ( addQuotes && len < maxlen )
        msg[len++] = 34;
    for ( ia = 0; len < maxlen && add[ia]; ++ia )
        msg[len++] = add[ia];
    if ( addQuotes && len < maxlen )
        msg[len++] = 34;
    return len;
}

char __cdecl Com_GetDecimalDelimiter()
{
    int lang; // [esp+0h] [ebp-4h]

    lang = loc_language->current.integer;
    if ( lang == 1 || lang == 2 || lang == 3 || lang == 5 || lang == 6 || lang == 8 || lang == 9 || lang == 12 )
        return 44;
    else
        return 46;
}

void __cdecl Com_LocalizedFloatToString(float f, char *buffer, unsigned int maxlen, unsigned int numDecimalPlaces)
{
    unsigned int charPos; // [esp+8h] [ebp-8h]
    char delimiter; // [esp+Fh] [ebp-1h]

    _snprintf(buffer, maxlen - 1, "%.*f", numDecimalPlaces, f);
    buffer[maxlen - 1] = 0;
    delimiter = Com_GetDecimalDelimiter();
    if ( delimiter != 46 )
    {
        for ( charPos = 0; charPos < maxlen; ++charPos )
        {
            if ( buffer[charPos] == 46 )
            {
                buffer[charPos] = delimiter;
                return;
            }
        }
    }
}

void __cdecl Com_SyncThreads()
{
    R_SyncRenderThread();
    if ( com_sv_running && com_sv_running->current.enabled )
    {
        SV_WaitServer();
        SV_AllowPackets(0);
    }
    R_WaitWorkerCmds();
}

const char *__cdecl Com_DisplayName(const char *name, const char *clanAbbrev, int type)
{
    if ( !*clanAbbrev )
        type &= ~2u;
    switch ( type )
    {
        case 3:
            return va("%c%s%c%s", asc_CD51B0[0], clanAbbrev, asc_CD51B0[1], name);
        case 1:
            return name;
        case 2:
            return va("%c%s%c", asc_CD51B0[0], clanAbbrev, asc_CD51B0[1]);
    }
    return "";
}

const char *__cdecl CS_DisplayName(const clientState_s *cs, int type)
{
    return Com_DisplayName(cs->name, cs->clanAbbrev, type);
}

int __cdecl Com_GetPrivateClients()
{
    if ( !com_maxclients->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\common.cpp",
                    7798,
                    0,
                    "privateClients doesn't index com_maxclients->current.integer\n\t%i not in [0, %i)",
                    0,
                    com_maxclients->current.integer) )
    {
        __debugbreak();
    }
    return 0;
}

void Com_Printf_NoFilter(const char *fmt, ...)
{
    char string[4100]; // [esp+4h] [ebp-1008h] BYREF
    va_list va; // [esp+1018h] [ebp+Ch] BYREF

    va_start(va, fmt);
    _vsnprintf(string, 0x1000u, fmt, va);
    string[4095] = 0;
    Com_PrintMessage(0, string, 0);
}

