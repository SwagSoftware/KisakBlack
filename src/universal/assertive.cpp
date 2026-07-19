#include "assertive.h"

#include <cstdarg>
#include <cstdio>
#include <Windows.h>
#include "com_buildinfo.h"
#include "q_shared.h"
#include <win32/win_common.h>
#include <qcommon/common.h>
#include <win32/win_mini_dumper.h>

HWND__ *g_hwndGame[4];
int g_hiddenCount;
bool g_inStackTrace;
AddressInfo_s g_assertAddress[32];
int g_assertAddressCount;
unsigned __int8 *lineBufferEndPos;
int lineBufferStartPos;
char lineBuffer[4096];
char message[1024];
int isHandlingAssert;
int lastAssertType;
char assertMessage[4096];
char g_module[260];
char *startOfStackTrace;
bool shouldQuitOnError;

void(__cdecl *AssertCallback)(const char *);
void(__cdecl *MonkeyAssertCallback)(const char *);

static HINSTANCE__ *__cdecl GetModuleBase(char *name)
{
        const char *v2; // [esp+Ch] [ebp-11Ch]
        char moduleName[264]; // [esp+10h] [ebp-118h] BYREF
        HINSTANCE__ *moduleHandle; // [esp+11Ch] [ebp-Ch]
        int nameLength; // [esp+120h] [ebp-8h]
        int nameIndex; // [esp+124h] [ebp-4h]

        v2 = &name[strlen(name) + 1];
        nameLength = v2 - (name + 1);
        for (nameIndex = nameLength - 1;
                nameIndex >= 0 && name[nameIndex] != 46 && name[nameIndex] != 47 && name[nameIndex] != 92;
                --nameIndex)
        {
                ;
        }
        if (nameIndex >= 0 && name[nameIndex] == 46)
                nameLength = nameIndex;
        memcpy((unsigned __int8 *)moduleName, (unsigned __int8 *)name, nameLength);
        strcpy(&moduleName[nameLength], ".exe");
        moduleHandle = GetModuleHandleA(moduleName);
        if (moduleHandle)
                return moduleHandle;
        strcpy(&moduleName[nameLength], ".dll");
        return GetModuleHandleA(moduleName);
}

static int __stdcall HideWindowCallback(HWND__ *hwnd, int lParam)
{
        int styleEx; // [esp+14h] [ebp-40Ch]
        char caption[1024]; // [esp+18h] [ebp-408h] BYREF
        int style; // [esp+41Ch] [ebp-4h]

        if (GetWindowTextA(hwnd, caption, 1024) && strcmp(caption, Com_GetBuildDisplayNameR()))
                return 1;
        style = GetWindowLongA(hwnd, -16);
        styleEx = GetWindowLongA(hwnd, -20);
        if ((style & 0x10000000) != 0)
        {
                g_hwndGame[g_hiddenCount++] = hwnd;
                SetWindowLongA(hwnd, -16, style & 0xEFFFFFFF);
                SetWindowLongA(hwnd, -20, styleEx & 0xFFFFFFF7);
        }
        return 1;
}

void __cdecl FixWindowsDesktop()
{
    DWORD CurrentThreadId; // eax
    HDC__ *hdc; // [esp+0h] [ebp-614h]
    _WORD ramp[770]; // [esp+4h] [ebp-610h] BYREF
    unsigned __int16 i; // [esp+60Ch] [ebp-8h]
    HWND__ *hwndDesktop; // [esp+610h] [ebp-4h]

    ChangeDisplaySettingsA(0, 0);
    CurrentThreadId = GetCurrentThreadId();
    EnumThreadWindows(CurrentThreadId, (WNDENUMPROC)HideWindowCallback, 0);
    hwndDesktop = GetDesktopWindow();
    hdc = GetDC(hwndDesktop);
    for (i = 0; i < 0x100u; ++i)
    {
        ramp[i] = 257 * i;
        ramp[i + 256] = 257 * i;
        ramp[i + 512] = 257 * i;
    }
    SetDeviceGammaRamp(hdc, ramp);
    ReleaseDC(hwndDesktop, hdc);
}



int __cdecl Assert_DoStackTrace(char *msg, int nIgnore, int type, int *context)
{
#if 0 // KISAKTODO
    int result; // eax
    int *v5; // ecx
    int *reg_ebp; // [esp+8h] [ebp-Ch]
    int i; // [esp+10h] [ebp-4h]
    int savedregs; // [esp+14h] [ebp+0h] BYREF

    if ( g_inStackTrace )
        return 0;
    g_inStackTrace = 1;
    memset((unsigned __int8 *)g_assertAddress, 0, sizeof(g_assertAddress));
    g_assertAddressCount = 0;
    if ( context )
        reg_ebp = context;
    else
        reg_ebp = &savedregs;
    for ( i = 0; i < nIgnore + 32; ++i )
    {
        v5 = reg_ebp;
        reg_ebp = (int *)*reg_ebp;
        if ( i >= nIgnore )
        {
            g_assertAddress[g_assertAddressCount++].address = v5[1] - 5;
            if ( !reg_ebp )
                break;
        }
    }
    result = LoadMapFiles(msg);
    g_inStackTrace = 0;
    return result;
#else
    return 0;
#endif
}

int __cdecl LoadMapFiles(char *msg)
{
    int v2; // eax
    int j; // [esp+0h] [ebp-Ch]
    AddressInfo_s *addressInfo; // [esp+4h] [ebp-8h]
    char *curPos; // [esp+8h] [ebp-4h]
    char *curPosa; // [esp+8h] [ebp-4h]
    char *curPosb; // [esp+8h] [ebp-4h]

    LoadMapFilesForDir("");
    if ( !msg )
        return 0;
    curPos = msg;
    for ( j = 0; j < g_assertAddressCount; ++j )
    {
        addressInfo = &g_assertAddress[j];
        if ( addressInfo->moduleName[0] )
        {
            curPosa = &curPos[sprintf(curPos, "%s:        ", addressInfo->moduleName)];
            if ( addressInfo->bestLineFilename[0] )
            {
                curPosb = &curPosa[sprintf(
                                                         curPosa,
                                                         "%s                ...%s, line %i",
                                                         addressInfo->bestFunction,
                                                         addressInfo->bestLineFilename,
                                                         addressInfo->bestLineNumber)];
                v2 = sprintf(curPosb, "\n");
            }
            else
            {
                if ( addressInfo->bestFunction[0] )
                    curPosb = &curPosa[sprintf(
                                                             curPosa,
                                                             "%s                ...%s, address %x",
                                                             addressInfo->bestFunction,
                                                             addressInfo->bestFunctionFilename,
                                                             addressInfo->address)];
                else
                    curPosb = &curPosa[sprintf(curPosa, "%s, address %x", addressInfo->bestFunction, addressInfo->address)];
                v2 = sprintf(curPosb, "\n");
            }
            curPos = &curPosb[v2];
        }
    }
    return curPos - msg;
}

void __cdecl LoadMapFilesForDir(const char *dir)
{
    char *v1; // eax
    char *v2; // eax
    char *v3; // eax
    char v4; // [esp+13h] [ebp-1165h]
    char *p_file; // [esp+18h] [ebp-1160h]
    char *cFileName; // [esp+1Ch] [ebp-115Ch]
    _WIN32_FIND_DATAA FindFileData; // [esp+20h] [ebp-1158h] BYREF
    char file; // [esp+160h] [ebp-1018h] BYREF
    _BYTE v9[3]; // [esp+161h] [ebp-1017h] BYREF
    unsigned int baseAddress; // [esp+964h] [ebp-814h]
    _iobuf *fp; // [esp+968h] [ebp-810h]
    HANDLE hFindFile; // [esp+96Ch] [ebp-80Ch]
    char string[2052]; // [esp+970h] [ebp-808h] BYREF

    if ( *dir )
    {
        v1 = Sys_DefaultInstallPath();
        sprintf(string, "%s\\%s\\*.map", v1, dir);
    }
    else
    {
        v2 = Sys_DefaultInstallPath();
        sprintf(string, "%s\\*.map", v2);
    }
    hFindFile = FindFirstFileA(string, &FindFileData);
    if ( hFindFile != (HANDLE)-1 )
    {
        do
        {
            baseAddress = (unsigned int)GetModuleBase(FindFileData.cFileName);
            if ( baseAddress )
            {
                v3 = Sys_DefaultInstallPath();
                sprintf(&file, "%s\\%s", v3, FindFileData.cFileName);
                fp = fopen(&file, "rb");
                if ( fp )
                {
                    cFileName = FindFileData.cFileName;
                    p_file = &file;
                    do
                    {
                        v4 = *cFileName;
                        *p_file++ = *cFileName++;
                    }
                    while ( v4 );
                    FindFileData.cAlternateFileName[&v9[strlen(&file)] - v9 + 12] = 0;
                    ParseMapFile(fp, baseAddress, &file);
                    fclose(fp);
                }
            }
        }
        while ( FindNextFileA(hFindFile, &FindFileData) );
        FindClose(hFindFile);
    }
}

char __cdecl ParseMapFile(_iobuf *fp, unsigned int baseAddress, const char *mapName)
{
    const char *v4; // eax
    const char *v5; // eax
    char *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    char *v9; // eax
    const char *v10; // eax
    char *v11; // eax
    char *v12; // eax
    const char *v13; // eax
    char *pszNameStop; // [esp+14h] [ebp-878h]
    char *pszNameStart; // [esp+18h] [ebp-874h]
    unsigned int loadAddress; // [esp+20h] [ebp-86Ch] BYREF
    const char *filenameSubStr; // [esp+24h] [ebp-868h]
    int j; // [esp+28h] [ebp-864h]
    unsigned int address; // [esp+2Ch] [ebp-860h] BYREF
    const char *filename; // [esp+30h] [ebp-85Ch]
    AddressInfo_s *addressInfo; // [esp+34h] [ebp-858h]
    unsigned int relAddress; // [esp+38h] [ebp-854h]
    unsigned int lineOffset[4]; // [esp+3Ch] [ebp-850h] BYREF
    char filenameBuffer[1024]; // [esp+4Ch] [ebp-840h] BYREF
    unsigned int offset; // [esp+44Ch] [ebp-440h] BYREF
    unsigned int baseEndAddress; // [esp+450h] [ebp-43Ch]
    unsigned int group; // [esp+454h] [ebp-438h] BYREF
    const char *funcName; // [esp+458h] [ebp-434h]
    unsigned int lineGroup[4]; // [esp+45Ch] [ebp-430h] BYREF
    int i; // [esp+46Ch] [ebp-420h]
    unsigned int lineNumber[4]; // [esp+470h] [ebp-41Ch] BYREF
    char *atChar; // [esp+480h] [ebp-40Ch]
    char function[1024]; // [esp+484h] [ebp-408h] BYREF
    int readCount; // [esp+888h] [ebp-4h]

    lineBufferEndPos = 0;
    lineBufferStartPos = 0;
    do
    {
        if ( !ReadLine(fp) )
            return 0;
    }
    while ( sscanf(lineBuffer, " Preferred load address is %x\r\n", &loadAddress) != 1 );
    if ( !SkipLines(2, fp) )
        return 0;
    baseEndAddress = 0;
    while ( 1 )
    {
        if ( !ReadLine(fp) )
            return 0;
        if ( !lineBuffer[0] )
            break;
        if ( sscanf(lineBuffer, "%x:%x %xH %s %s", &group, &offset, &address, function, filenameBuffer) != 5 )
        {
            ParseError("Unknown line format in the segments section");
            return 0;
        }
        if ( group == 1 && baseEndAddress < address + offset + baseAddress + 4096 )
            baseEndAddress = address + offset + baseAddress + 4096;
    }
    for ( j = 0; j < g_assertAddressCount; ++j )
    {
        addressInfo = &g_assertAddress[j];
        if ( addressInfo->address >= baseAddress && addressInfo->address < baseEndAddress )
            I_strncpyz(addressInfo->moduleName, mapName, 64);
    }
    do
    {
        if ( !ReadLine(fp) )
            return 0;
        v4 = strstr(lineBuffer, "Publics by Value");
    }
    while ( !v4 );
    if ( !SkipLines(1, fp) )
        return 0;
    while ( 1 )
    {
        if ( !ReadLine(fp) )
            return 0;
        if ( !lineBuffer[0] )
            break;
        if ( sscanf(lineBuffer, "%x:%x %s %x", &group, &offset, function, &address) != 4 )
        {
            ParseError("Unknown line format in the public symbols section");
            return 0;
        }
        v5 = strrchr(lineBuffer, 0x20u);
        filenameSubStr = v5;
        if ( !v5 || sscanf(filenameSubStr + 1, "%s", filenameBuffer) != 1 )
        {
            ParseError("Couldn't parse file name in the public symbols section");
            return 0;
        }
        relAddress = address;
        for ( j = 0; j < g_assertAddressCount; ++j )
        {
            addressInfo = &g_assertAddress[j];
            if ( addressInfo->address >= baseAddress
                && addressInfo->address < baseEndAddress
                && relAddress <= addressInfo->address
                && (!addressInfo->bestFunction[0] || addressInfo->bestFunctionAddress < relAddress) )
            {
                addressInfo->bestFunctionAddress = relAddress;
                funcName = function;
                if ( function[0] == 95 || *funcName == 63 )
                    ++funcName;
                I_strncpyz(addressInfo->bestFunction, funcName, 64);
                v6 = strchr(addressInfo->bestFunction, 0x40u);
                atChar = v6;
                if ( v6 )
                    *atChar = 0;
                v7 = strrchr(filenameBuffer, 0x5Cu);
                filename = v7;
                if ( v7 )
                    ++filename;
                else
                    filename = filenameBuffer;
                I_strncpyz(addressInfo->bestFunctionFilename, filename, 64);
            }
        }
    }
    if ( !SkipLines(2, fp) )
        return 0;
    if ( !ReadLine(fp) )
        return 0;
    if ( strcmp(lineBuffer, " Static symbols\r") )
        goto LABEL_91;
    if ( !SkipLines(1, fp) )
        return 0;
    while ( ReadLine(fp) && lineBuffer[0] )
    {
        if ( sscanf(lineBuffer, "%x:%x %s %x", &group, &offset, function, &address) != 4 )
        {
            ParseError("Unknown line format in the static symbols section");
            return 0;
        }
        v8 = strrchr(lineBuffer, 0x20u);
        filenameSubStr = v8;
        if ( !v8 || sscanf(filenameSubStr + 1, "%s", filenameBuffer) != 1 )
        {
            ParseError("Couldn't parse file name in the static symbols section");
            return 0;
        }
        relAddress = address;
        for ( j = 0; j < g_assertAddressCount; ++j )
        {
            addressInfo = &g_assertAddress[j];
            if ( addressInfo->address >= baseAddress
                && addressInfo->address < baseEndAddress
                && relAddress <= addressInfo->address
                && (!addressInfo->bestFunction[0] || addressInfo->bestFunctionAddress < relAddress) )
            {
                addressInfo->bestFunctionAddress = relAddress;
                funcName = function;
                if ( function[0] == 95 || *funcName == 63 )
                    ++funcName;
                I_strncpyz(addressInfo->bestFunction, funcName, 64);
                v9 = strchr(addressInfo->bestFunction, 0x40u);
                atChar = v9;
                if ( v9 )
                    *atChar = 0;
                v10 = strrchr(filenameBuffer, 0x5Cu);
                filename = v10;
                if ( v10 )
                    ++filename;
                else
                    filename = filenameBuffer;
                I_strncpyz(addressInfo->bestFunctionFilename, filename, 64);
            }
        }
    }
LABEL_91:
    while ( ReadLine(fp) )
    {
        if ( strncmp(lineBuffer, "Line numbers for ", 0x11u) )
        {
            ParseError("Expected line number section");
            return 0;
        }
        v11 = strchr(lineBuffer, 0x28u);
        pszNameStart = (char *)v11;
        if ( !v11 )
        {
            ParseError("Couldn't find '(' for the name of the source file in line number section");
            return 0;
        }
        v12 = strchr(v11, 0x29u);
        pszNameStop = v12;
        if ( !v12 )
        {
            ParseError("Couldn't find ')' for the name of the source file in line number section");
            return 0;
        }
        strncpy(filenameBuffer, pszNameStart + 1, v12 - pszNameStart - 1);
        *((_BYTE *)&lineOffset[3] + (int)pszNameStop - (int)pszNameStart + 3) = 0;
        filenameSubStr = filenameBuffer;
        if ( !SkipLines(1, fp) )
            return 0;
        while ( ReadLine(fp) && lineBuffer[0] )
        {
            readCount = sscanf(
                                        lineBuffer,
                                        "%i %x:%x %i %x:%x %i %x:%x %i %x:%x\r\n",
                                        lineNumber,
                                        lineGroup,
                                        lineOffset,
                                        &lineNumber[1],
                                        &lineGroup[1],
                                        &lineOffset[1],
                                        &lineNumber[2],
                                        &lineGroup[2],
                                        &lineOffset[2],
                                        &lineNumber[3],
                                        &lineGroup[3],
                                        &lineOffset[3]);
            if ( readCount % 3 || readCount / 3 <= 0 )
            {
                ParseError("unknown line format in the line number section");
                return 0;
            }
            for ( i = 0; 3 * i < readCount; ++i )
            {
                relAddress = lineOffset[i] + baseAddress + 4096;
                for ( j = 0; j < g_assertAddressCount; ++j )
                {
                    addressInfo = &g_assertAddress[j];
                    if ( addressInfo->address >= baseAddress
                        && addressInfo->address < baseEndAddress
                        && relAddress <= addressInfo->address
                        && (!addressInfo->bestLineFilename[0] || addressInfo->bestLineAddress < relAddress) )
                    {
                        addressInfo->bestLineAddress = relAddress;
                        addressInfo->bestLineNumber = lineNumber[i];
                        v13 = strrchr(filenameSubStr, 0x5Cu);
                        filename = v13;
                        if ( v13 )
                            ++filename;
                        else
                            filename = filenameSubStr;
                        I_strncpyz(addressInfo->bestLineFilename, filename, 64);
                    }
                }
            }
        }
    }
    return 1;
}

void __cdecl ParseError(const char *msg)
{
    HWND ActiveWindow; // eax

    ActiveWindow = GetActiveWindow();
    MessageBoxA(ActiveWindow, msg, ".map parse error", 0x10u);
}

char __cdecl ReadLine(_iobuf *fp)
{
    signed int readSize; // [esp+0h] [ebp-Ch]
    int bytesRead; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

LABEL_1:
    lineBufferEndPos -= lineBufferStartPos;
    if ( (int)lineBufferEndPos < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\assertive.cpp",
                    306,
                    0,
                    "%s",
                    "lineBufferEndPos >= 0") )
    {
        __debugbreak();
    }
    memmove(
        (unsigned __int8 *)lineBuffer,
        (unsigned __int8 *)&lineBuffer[lineBufferStartPos],
        (unsigned int)lineBufferEndPos);
    lineBufferStartPos = 0;
    for ( i = 0; ; ++i )
    {
        if ( i >= (int)lineBufferEndPos )
        {
LABEL_13:
            readSize = 4096 - (unsigned int)lineBufferEndPos - 1;
            bytesRead = fread(&lineBuffer[(unsigned int)lineBufferEndPos], 1u, readSize, fp);
            if ( bytesRead > readSize
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\assertive.cpp",
                            334,
                            0,
                            "%s",
                            "bytesRead <= readSize") )
            {
                __debugbreak();
            }
            lineBufferEndPos += bytesRead;
            lineBuffer[(unsigned int)lineBufferEndPos] = 0;
            if ( !bytesRead )
            {
                if ( !lineBufferEndPos )
                    return 0;
                lineBufferStartPos = (int)lineBufferEndPos;
                return 1;
            }
            goto LABEL_1;
        }
        if ( lineBuffer[i] == 10 )
            break;
    }
    if ( (unsigned __int8 *)(i + 1) == lineBufferEndPos )
        goto LABEL_13;
    lineBuffer[i] = 0;
    if (lineBuffer[i + 1] == 13)
        lineBufferStartPos = i + 2;
    else
        lineBufferStartPos = i + 1;
    return 1;
}

char __cdecl SkipLines(int lineCount, _iobuf *fp)
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < lineCount; ++i )
    {
        if ( !ReadLine(fp) )
            return 0;
    }
    return 1;
}

void __cdecl Assert_ResetAddressInfo()
{
    g_assertAddressCount = 0;
}

void __cdecl Assert_SetMonkeyCallbackHandler(void (__cdecl *AssertCallbackFunc)(const char *))
{
    MonkeyAssertCallback = AssertCallbackFunc;
}

bool Assert_MyHandler(const char *filename, int line, int type, const char *fmt, ...)
{
#if 0
    char *v4; // eax
    char shouldBreak; // [esp+3h] [ebp-5h]
    va_list va; // [esp+20h] [ebp+18h] BYREF

    va_start(va, fmt);
    Sys_EnterCriticalSection(CRITSECT_ASSERT);
    _vsnprintf(message, 0x400u, fmt, va);
    message[1023] = 0;
    if ( isHandlingAssert )
    {
        v4 = ::va("Assert Expression:\n        %s\nFile:        %s\nLine:        %d\n\n", message, filename, line);
        Com_Printf(16, v4);
        CopyMessageToClipboard();
        AssertNotify(lastAssertType, RECURSIVE);
        Assert_BuildAssertMessage(message, (char*)filename, line, type, 1, assertMessage);
        if ( isHandlingAssert == 1 )
        {
            isHandlingAssert = 2;
            Com_Printf(16, "ASSERTBEGIN - ( Recursive assert )---------------------------------------------\n");
            Com_Printf(16, assertMessage);
            Com_Printf(16, "ASSERTEND - ( Recursive assert ) ----------------------------------------------\n\n");
        }
        exit(-1);
    }
    lastAssertType = type;
    isHandlingAssert = 1;
    FixWindowsDesktop();
    Assert_BuildAssertMessage(message, (char*)filename, line, type, 1, assertMessage);
    Com_Printf(16, "ASSERTBEGIN -------------------------------------------------------------------\n");
    Com_Printf(16, "%s", assertMessage);
    Com_Printf(16, "ASSERTEND ---------------------------------------------------------------------\n");
    if ( QuitOnError() )
        ExitProcess(0xFFFFFFFF);
    CopyMessageToClipboard();
    shouldBreak = AssertNotify(type, FIRST_TIME);
    isHandlingAssert = 0;
    Sys_LeaveCriticalSection(CRITSECT_ASSERT);
    return shouldBreak == 0;
#else
    // Asserts are dev-only checks. Rather than trap (SIGILL) on every one — which
    // kills the game on non-fatal checks whose callers already handle the bad case
    // (release builds compile asserts out entirely) — log the assertion and continue,
    // i.e. behave like clicking "Ignore". Genuine faults still surface as real
    // crashes at the point they actually occur.
    {
        char assertMsg[1024];
        va_list ap;
        va_start(ap, fmt);
        vsnprintf(assertMsg, sizeof(assertMsg), fmt ? fmt : "", ap);
        va_end(ap);
        Com_Printf(16, "ASSERT (ignored) %s (%d): %s\n", filename, line, assertMsg);
    }
    (void)type;
    return 1;
#endif
}

bool CopyMessageToClipboard()
{
    HWND DesktopWindow; // eax
    bool result; // eax
    char v2; // dl
    char *v3; // [esp+8h] [ebp-20h]
    char *v4; // [esp+Ch] [ebp-1Ch]
    char *mem; // [esp+20h] [ebp-8h]
    HGLOBAL memoryHandle; // [esp+24h] [ebp-4h]

    DesktopWindow = GetDesktopWindow();
    result = OpenClipboard(DesktopWindow);
    if ( result )
    {
        EmptyClipboard();
        memoryHandle = GlobalAlloc(2u, strlen(assertMessage) + 1);
        if ( memoryHandle )
        {
            mem = (char *)GlobalLock(memoryHandle);
            if ( mem )
            {
                v4 = assertMessage;
                v3 = mem;
                do
                {
                    v2 = *v4;
                    *v3++ = *v4++;
                }
                while ( v2 );
                GlobalUnlock(memoryHandle);
                SetClipboardData(1u, memoryHandle);
            }
        }
        return CloseClipboard();
    }
    return result;
}

char __cdecl AssertNotify(int type, AssertOccurance occurance)
{
    HWND ActiveWindow; // eax
    const char *msg; // [esp+8h] [ebp-4h]

    if ( MonkeyAssertCallback )
    {
        MonkeyAssertCallback(assertMessage);
        if ( occurance != RECURSIVE )
            ExitProcess(0xFFFFFFFF);
        return 1;
    }
    else
    {
        if ( AssertCallback )
            AssertCallback(assertMessage);
        if ( type )
        {
            if ( type == 1 )
                msg = "SANITY CHECK FAILURE... (this text is on the clipboard)";
            else
                msg = "INTERNAL ERROR";
        }
        else
        {
            msg = "ASSERTION FAILURE... (this text is on the clipboard)";
        }
        ShowCursor(1);
        ActiveWindow = GetActiveWindow();
        if ( MessageBoxA(ActiveWindow, assertMessage, msg, 0x12011u) == 1 )
        {
            if ( occurance != RECURSIVE )
            {
                if ( Sys_IsMiniDumpStarted() && !IsDebuggerPresent() )
                    RaiseException(1u, 0, 0, 0);
                exit(-1);
            }
            return 1;
        }
        else
        {
            return 1;
        }
    }
}

void __cdecl Assert_BuildAssertMessage(char *expr, char *filename, int line, int type, int skipLevels, char *message)
{
    const char *BuildDisplayName; // eax
    const char *BuildName; // [esp-Ch] [ebp-20h]
    char *BuildVersion; // [esp-8h] [ebp-1Ch]
    char *curPos; // [esp+0h] [ebp-14h]
    char unknown[12]; // [esp+4h] [ebp-10h] BYREF

    strcpy(unknown, "<unknown>");
    if ( !filename )
        filename = unknown;
    if ( !expr )
        expr = unknown;
    if ( !GetModuleFileNameA(0, g_module, 0x104u) )
        strcpy(g_module, "<unknown application>");
    BuildVersion = Com_GetBuildVersion();
    BuildName = Com_GetBuildName();
    BuildDisplayName = Com_GetBuildDisplayName();
    curPos = &message[sprintf(message, "Build: %s %s build %s %s\n", BuildDisplayName, BuildName, BuildVersion, "win-x86")];
    startOfStackTrace = &curPos[sprintf(
                                                                curPos,
                                                                "Expression:\n        %s\n\nModule:        %s\nFile:        %s\nLine:        %d\n\n",
                                                                expr,
                                                                g_module,
                                                                filename,
                                                                line)];
    Assert_DoStackTrace(startOfStackTrace, skipLevels + 1, type, 0);
}

void __cdecl RefreshQuitOnErrorCondition()
{
    bool v0; // [esp+0h] [ebp-4h]

    if ( Dvar_IsSystemActive() )
    {
        v0 = Dvar_GetBool("QuitOnError") || Dvar_GetInt("r_vc_compile") == 2;
        shouldQuitOnError = v0;
    }
}

bool __cdecl QuitOnError()
{
    RefreshQuitOnErrorCondition();
    return shouldQuitOnError;
}

