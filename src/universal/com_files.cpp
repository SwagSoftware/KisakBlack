#include "com_files.h"
#include "assertive.h"
#include "q_shared.h"

#include <ShlObj.h>
#include <qcommon/threads.h>
#include <qcommon/common.h>

#include <qcommon/unzip.h>
#include "com_fileaccess.h"

#include <cstring>
#include "com_memory.h"
#include <stringed/stringed_hooks.h>
#include "mem_userhunk.h"
#include "com_shared.h"
#include <qcommon/files.h>
#include <win32/win_localize.h>
#include <qcommon/md4.h>
#include <qcommon/cmd.h>
#include <qcommon/com_profilemapload.h>
#include <database/db_registry.h>
#include <sound/snd_public_async.h>
#include <win32/win_net.h>
#include <sound/snd_stream.h>

const char TEN_SPACE[] = "          ";

char lastValidBase[256];
char lastValidGame[256];
iwd_pure_check_s *fs_iwdPureChecks;
char ospathPersonalDocuments[256];
int fs_checksumFeed;
int fs_iwdFileCount;
int fs_numServerReferencedIwds;
const char *fs_serverIwdNames[1024];
const char *fs_serverReferencedIwdNames[1024];
int fs_serverReferencedIwds[1024];
char szIwdLanguageName[2][64];
int fs_numServerReferencedFFs;
const char *fs_serverReferencedFFNames[32];
int fs_serverReferencedFFCheckSums[32];

searchpath_s *fs_searchpaths;
int fs_numServerIwds;
int fs_serverIwds[1024];

fileHandleData_t fsh[70];

const dvar_t *fs_debug;
const dvar_t *fs_copyfiles;
const dvar_t *fs_cdpath;
const dvar_t *fs_basepath;
const dvar_t *fs_basegame;
const dvar_t *fs_gameDirVar;
const dvar_t *fs_usermapDir;
const dvar_t *fs_ignoreLocalized;
const dvar_t *fs_homepath;
const dvar_t *fs_userDocuments;
const dvar_t *fs_restrict;
const dvar_t *fs_usedevdir;

char fs_gamedir[256];

bool g_disablePureCheck;

int fs_loadStack;

int com_fileAccessed;

int fs_fakeChkSum;

char *__cdecl FS_GetOsFolderPath(int folder, char *ospath)
{
    char buffer[1028]; // [esp+0h] [ebp-408h] BYREF

    if ( folder != 28
        && folder != 5
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_files.cpp",
                    466,
                    0,
                    "%s",
                    "CSIDL_LOCAL_APPDATA == folder || CSIDL_PERSONAL == folder") )
    {
        __debugbreak();
    }
    SHGetFolderPathA(0, folder | 0x8000, 0, 0, buffer);
    I_strncpyz(ospath, buffer, 256);
    I_strncat(ospath, 256, "\\Activision\\CoD");
    return ospath;
}

bool __cdecl FS_Initialized()
{
    return fs_searchpaths != 0;
}

void __cdecl FS_CheckFileSystemStarted()
{
    if ( !fs_searchpaths
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_files.cpp", 931, 0, "%s", "fs_searchpaths") )
    {
        __debugbreak();
    }
}

int __cdecl FS_IwdIsPure(const iwd_t *iwd)
{
    int i; // [esp+0h] [ebp-4h]

    if ( !fs_numServerIwds )
        return 1;
    for ( i = 0; i < fs_numServerIwds; ++i )
    {
        if ( iwd->checksum == fs_serverIwds[i] )
            return 1;
    }
    return 0;
}

void __cdecl FS_DisablePureCheck(bool disable)
{
    if ( fs_gameDirVar && *(_BYTE *)fs_gameDirVar->current.integer )
        g_disablePureCheck = disable;
    if ( disable )
        FS_ShutdownServerIwdNames();
}

int __cdecl FS_LoadStack()
{
    return fs_loadStack;
}

bool __cdecl FS_UseSearchPath(const searchpath_s *pSearch)
{
    if ( pSearch->bLocalized && fs_ignoreLocalized->current.enabled )
        return 0;
    return !pSearch->bLocalized || pSearch->language == SEH_GetCurrentLanguage();
}

int __cdecl FS_LanguageHasAssets(int iLanguage)
{
    searchpath_s *pSearch; // [esp+0h] [ebp-4h]

    for ( pSearch = fs_searchpaths; pSearch; pSearch = pSearch->next )
    {
        if ( pSearch->bLocalized && pSearch->language == iLanguage )
            return 1;
    }
    return 0;
}

int __cdecl FS_HashFileName(const char *fname, int hashSize)
{
    int hash; // [esp+0h] [ebp-Ch]
    int letter; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    hash = 0;
    for ( i = 0; fname[i]; ++i )
    {
        letter = tolower(fname[i]);
        if ( letter == 46 )
            break;
        if ( letter == 92 )
            letter = 47;
        hash += letter * (i + 119);
    }
    return ((hash >> 20) ^ hash ^ (hash >> 10)) & (hashSize - 1);
}

int __cdecl FS_HandleForFile(const char *name, FsThread thread)
{
    int i; // [esp+4h] [ebp-Ch]
    int ia; // [esp+4h] [ebp-Ch]
    int count; // [esp+8h] [ebp-8h]
    int first; // [esp+Ch] [ebp-4h]

    switch (thread)
    {
    case FS_THREAD_MAIN:
        if (!Sys_IsMainThread()
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_files.cpp",
                1136,
                0,
                "%s",
                "Sys_IsMainThread()"))
        {
            __debugbreak();
        }
        first = 1;
        count = 49;
        break;
    case FS_THREAD_STREAM:
        if (!Sys_IsStreamThread()
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_files.cpp",
                1143,
                0,
                "%s",
                "Sys_IsStreamThread()"))
        {
            __debugbreak();
        }
        first = 50;
        count = 11;
        break;
    case FS_THREAD_DATABASE:
        if (!Sys_IsDatabaseThread()
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_files.cpp",
                1151,
                0,
                "%s",
                "Sys_IsDatabaseThread()"))
        {
            __debugbreak();
        }
        first = 61;
        count = 1;
        break;
    case FS_THREAD_BACKEND:
        if (!Sys_IsRenderThread()
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_files.cpp",
                1159,
                0,
                "%s",
                "Sys_IsRenderThread()"))
        {
            __debugbreak();
        }
        first = 62;
        count = 1;
        break;
    case FS_THREAD_SERVER:
        if (!Sys_IsServerThread()
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_files.cpp",
                1167,
                0,
                "%s",
                "Sys_IsServerThread()"))
        {
            __debugbreak();
        }
        first = 63;
        count = 2;
        break;
    default:
        if (!Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_files.cpp",
            1178,
            0,
            "Does the FS need to support a new thread? "))
            __debugbreak();
        first = 0;
        count = 0;
        break;
    }
    for (i = 0; i < count; ++i)
    {
        if (!fsh[i + first].handleFiles.file.o)
            return i + first;
    }
    Com_PrintWarning(10, "FILE %2i: '%s' 0x%x\n", first, fsh[first].name, fsh[first].handleFiles.file.o);
    Com_PrintWarning(10, "FS_HandleForFile: none free (%d)\n", thread);
    for (ia = 1; ia < 70; ++ia)
        Com_Printf(10, "FILE %2i: '%s' 0x%x\n", ia, fsh[ia].name, fsh[ia].handleFiles.file.o);
    if (!Assert_MyHandler(
        "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_files.cpp",
        1197,
        0,
        "FS_HandleForFile: No free handles available"))
        __debugbreak();
    Com_Error(ERR_DROP, "FS_HandleForFile: none free");
    return 0;
}

int __cdecl FS_HandleForFileCurrentThread(const char *filename)
{
    FsThread CurrentThread; // eax

    CurrentThread = (FsThread)FS_GetCurrentThread();
    return FS_HandleForFile(filename, CurrentThread);
}

FsThread __cdecl FS_GetCurrentThread()
{
    if ( Sys_IsMainThread() )
        return FS_THREAD_MAIN;
    if ( Sys_IsDatabaseThread() )
        return FS_THREAD_DATABASE;
    if ( Sys_IsStreamThread() )
        return FS_THREAD_STREAM;
    if ( Sys_IsRenderThread() )
        return FS_THREAD_BACKEND;
    if ( Sys_IsServerThread() )
        return FS_THREAD_SERVER;
    return FS_THREAD_INVALID;
}

FILE *__cdecl FS_FileForHandle(int f)
{
    if ((f <= 0 || f >= 70)
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_files.cpp",
            1251,
            0,
            "%s\n\t(f) = %i",
            "(f > 0 && f < (1 + 49 + 11 + 1 + 1 + 2 + 5 ))",
            f))
    {
        __debugbreak();
    }
    if (fsh[f].zipFile
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_files.cpp",
            1252,
            0,
            "%s",
            "!fsh[f].zipFile"))
    {
        __debugbreak();
    }
    if (!fsh[f].handleFiles.file.o
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_files.cpp",
            1253,
            0,
            "%s",
            "fsh[f].handleFiles.file.o"))
    {
        __debugbreak();
    }
    return fsh[f].handleFiles.file.o;
}

int __cdecl FS_filelength(int f)
{
    FILE *h; // [esp+4h] [ebp-4h]

    if (!f && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_files.cpp", 1267, 0, "%s", "f"))
        __debugbreak();

    FS_CheckFileSystemStarted();

    if (fsh[f].zipFile)
    {
        unz_s *zfi = (unz_s *)fsh[f].handleFiles.file.o;
        return zfi->cur_file_info.uncompressed_size;
    }

    h = FS_FileForHandle(f);
    return FS_FileGetFileSize(h);
}

void __cdecl FS_ReplaceSeparators(char *path)
{
    char *src; // [esp+0h] [ebp-Ch]
    char *dst; // [esp+4h] [ebp-8h]
    bool wasSep; // [esp+Bh] [ebp-1h]

    wasSep = 0;
    src = path;
    dst = path;
    while ( *src )
    {
        if ( *src == 47 || *src == 92 )
        {
            if ( !wasSep )
            {
                wasSep = 1;
                *dst++ = PATH_SEP;   // '\\' on Windows, '/' on Linux (q_shared.h)
            }
        }
        else
        {
            wasSep = 0;
            *dst++ = *src;
        }
        ++src;
    }
    *dst = 0;
}

void __cdecl FS_BuildOSPath(char *base, char *game, char *qpath, char *ospath)
{
    FS_BuildOSPathForThread(base, game, qpath, ospath, FS_THREAD_MAIN);
}

void __cdecl FS_BuildOSPathForThread(char *base, char *game, char *qpath, char *ospath, FsThread thread)
{
    unsigned int v5; // [esp+0h] [ebp-3Ch]
    unsigned int v6; // [esp+10h] [ebp-2Ch]
    unsigned int v7; // [esp+20h] [ebp-1Ch]

    if ( !base && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_files.cpp", 1351, 0, "%s", "base") )
        __debugbreak();
    if ( !qpath
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_files.cpp", 1352, 0, "%s", "qpath") )
    {
        __debugbreak();
    }
    if ( !ospath
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_files.cpp", 1353, 0, "%s", "ospath") )
    {
        __debugbreak();
    }
    if ( game )
    {
        if ( !*game )
            game = fs_gamedir;
    }
    else
    {
        game = (char *)"";
    }
    v7 = strlen(base);
    v6 = strlen(game);
    v5 = strlen(qpath);
    if ( (int)(v7 + v6 + 1 + v5 + 1) >= 256 )
    {
        if ( thread )
        {
            *ospath = 0;
            return;
        }
        Com_Error(ERR_FATAL, "FS_BuildOSPath: os path length exceeded");
    }
    memcpy((unsigned __int8 *)ospath, (unsigned __int8 *)base, v7);
    ospath[v7] = 47;
    memcpy((unsigned __int8 *)&ospath[v7 + 1], (unsigned __int8 *)game, v6);
    ospath[v7 + 1 + v6] = 47;
    memcpy((unsigned __int8 *)&ospath[v7 + 2 + v6], (unsigned __int8 *)qpath, v5 + 1);
    FS_ReplaceSeparators(ospath);
}

int __cdecl FS_CreatePath(char *OSPath)
{
    int v1; // eax
    int v2; // eax
    char *ofs; // [esp+0h] [ebp-4h]

    if (strstr(OSPath, "..") || strstr(OSPath, "::") )
    {
        Com_PrintWarning(10, "WARNING: refusing to create relative path \"%s\"\n", OSPath);
        return 1;
    }
    else
    {
        for ( ofs = OSPath + 1; *ofs; ++ofs )
        {
            // Accept either separator: FS_BuildOSPath produces '/'-separated paths on
            // Linux, so scanning only for '\\' (92) created no directories at all here
            // (e.g. the screenshots/ dir was never made, so file writes silently failed).
            if ( *ofs == 92 || *ofs == 47 )
            {
                char ch = *ofs;
                *ofs = 0;
                Sys_Mkdir(OSPath);
                *ofs = ch;
            }
        }
        return 0;
    }
}

int __cdecl FS_FileExists(char *file)
{
    _iobuf *f; // [esp+4h] [ebp-10Ch]
    char testpath[260]; // [esp+8h] [ebp-108h] BYREF

    FS_BuildOSPath((char *)fs_homepath->current.integer, fs_gamedir, file, testpath);
    f = FS_FileOpenReadBinary(testpath);
    if ( !f )
        return 0;
    FS_FileClose(f);
    return 1;
}

int __cdecl FS_OSFPathExists(const char *file)
{
    _iobuf *f; // [esp+0h] [ebp-4h]

    f = FS_FileOpenReadBinary(file);
    if ( !f )
        return 0;
    FS_FileClose(f);
    return 1;
}

void __cdecl FS_CopyFile(char *fromOSPath, char *toOSPath)
{
    unsigned __int8 *buf; // [esp+0h] [ebp-Ch]
    int len; // [esp+4h] [ebp-8h]
    _iobuf *f; // [esp+8h] [ebp-4h]
    _iobuf *fa; // [esp+8h] [ebp-4h]

    f = FS_FileOpenReadBinary(fromOSPath);
    if ( f )
    {
        len = FS_FileGetFileSize(f);
        buf = (unsigned __int8 *)Z_Malloc(len, "FS_CopyFile", 3);
        if ( FS_FileRead(buf, len, f) != len )
            Com_Error(ERR_FATAL, "Short read in FS_CopyFile()");
        FS_FileClose(f);
        if ( FS_CreatePath(toOSPath) || (fa = FS_FileOpenWriteBinary(toOSPath)) == 0 )
        {
            Z_Free((char *)buf, 3);
        }
        else
        {
            if ( FS_FileWrite(buf, len, fa) != len )
                Com_Error(ERR_FATAL, "Short write in FS_CopyFile()");
            FS_FileClose(fa);
            Z_Free((char *)buf, 3);
        }
    }
}

void __cdecl FS_Remove(const char *osPath)
{
    remove(osPath);
}

void __cdecl FS_FCloseFile(int h)
{
    const char *CurrentThreadName; // eax
    _iobuf *f; // [esp+0h] [ebp-4h]

    FS_CheckFileSystemStarted();
    if (fsh[h].streamed
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_files.cpp",
            1620,
            0,
            "%s",
            "!fsh[h].streamed"))
    {
        __debugbreak();
    }
    if (fs_debug->current.integer)
    {
        CurrentThreadName = Sys_GetCurrentThreadName();
        Com_Printf(10, "FS_FCloseFile from thread '%s', handle '%i', filename '%s'\n", CurrentThreadName, h, fsh[h].name);
    }
    if (fsh[h].zipFile)
    {
        unzCloseCurrentFile(fsh[h].handleFiles.file.z);
        if (fsh[h].handleFiles.iwdIsClone)
        {
            unzClose(fsh[h].handleFiles.file.z);
        }
        else
        {
            if (!fsh[h].zipFile->hasOpenFile
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_files.cpp",
                    1640,
                    0,
                    "%s",
                    "fsh[h].zipFile->hasOpenFile"))
            {
                __debugbreak();
            }
            fsh[h].zipFile->hasOpenFile = 0;
        }
    }
    else if (fsh[h].handleFiles.file.o)
    {
        f = FS_FileForHandle(h);
        FS_FileClose(f);
    }
    Com_Memset((unsigned int *)&fsh[h], 0, 284);
}

void __cdecl FS_FCloseLogFile(int h)
{
    FS_FCloseFile(h);
}

int __cdecl FS_FOpenFileWriteToDir(char *filename, char *dir, const char *osbasepath)
{
    return FS_FOpenFileWriteToDirForThread(filename, dir, osbasepath, FS_THREAD_MAIN);
}

int __cdecl FS_FOpenFileWriteToDirForThread(char *filename, char *dir, const char *osbasepath, FsThread thread)
{
    char ospath[256]; // [esp+0h] [ebp-108h] BYREF
    const char *basepath; // [esp+104h] [ebp-4h]

    FS_CheckFileSystemStarted();
    basepath = fs_homepath->current.string;
    FS_BuildOSPath((char *)basepath, dir, filename, ospath);
    if ( fs_debug->current.integer )
        Com_Printf(10, "FS_FOpenFileWriteToDirForThread: %s\n", ospath);
    if ( FS_CreatePath(ospath) )
        return 0;
    else
        return FS_GetHandleAndOpenFile(filename, ospath, thread);
}

int __cdecl FS_GetHandleAndOpenFile(const char *filename, const char *ospath, FsThread thread)
{
    int f; // [esp+0h] [ebp-8h]
    _iobuf *fp; // [esp+4h] [ebp-4h]

    fp = FS_FileOpenWriteBinary(ospath);
    if (!fp)
        return 0;
    f = FS_HandleForFile(filename, thread);
    fsh[f].zipFile = 0;
    fsh[f].handleFiles.file.o = fp;
    I_strncpyz(fsh[f].name, filename, 256);
    fsh[f].handleSync = 0;
    return f;
}

int __cdecl FS_FOpenFileWrite(char *filename)
{
    return FS_FOpenFileWriteToDirForThread(filename, fs_gamedir, "", FS_THREAD_MAIN);
}

int __cdecl FS_FOpenFileWriteCurrentThread(char *filename, char *dir, const char *osbasepath)
{
    FsThread thread; // [esp+0h] [ebp-4h]

    thread = FS_GetCurrentThread();
    if ( thread != FS_THREAD_INVALID )
        return FS_FOpenFileWriteToDirForThread(filename, dir, osbasepath, thread);
    Com_PrintError(1, "FS_FOpenFileWriteCurrentThread for an unknown thread\n");
    return 0;
}

int __cdecl FS_FOpenTextFileWrite(char *filename)
{
    char ospath[256]; // [esp+0h] [ebp-110h] BYREF
    const char *basepath; // [esp+104h] [ebp-Ch]
    _iobuf *f; // [esp+108h] [ebp-8h]
    int h; // [esp+10Ch] [ebp-4h]

    h = 0;
    FS_CheckFileSystemStarted();
    basepath = fs_homepath->current.string;
    FS_BuildOSPath((char *)basepath, fs_gamedir, filename, ospath);
    if (fs_debug->current.integer)
        Com_Printf(10, "FS_FOpenTextFileWrite: %s\n", ospath);
    if (FS_CreatePath(ospath))
        return 0;
    f = FS_FileOpenWriteText(ospath);
    if (!f)
        return 0;
    h = FS_HandleForFileCurrentThread(filename);
    fsh[h].zipFile = 0;
    fsh[h].handleFiles.file.o = f;
    I_strncpyz(fsh[h].name, filename, 256);
    fsh[h].handleSync = 0;
    if (!fsh[h].handleFiles.file.o)
    {
        FS_FCloseFile(h);
        return 0;
    }
    return h;
}

int __cdecl FS_FOpenFileAppend(char *filename)
{
    char ospath[256]; // [esp+0h] [ebp-110h] BYREF
    const char *basepath; // [esp+104h] [ebp-Ch]
    _iobuf *f; // [esp+108h] [ebp-8h]
    int h; // [esp+10Ch] [ebp-4h]

    h = 0;
    FS_CheckFileSystemStarted();
    basepath = fs_homepath->current.string;
    FS_BuildOSPath((char *)basepath, fs_gamedir, filename, ospath);
    if (fs_debug->current.integer)
        Com_Printf(10, "FS_FOpenFileAppend: %s\n", ospath);
    if (FS_CreatePath(ospath))
        return 0;
    f = FS_FileOpenAppendText(ospath);
    if (!f)
        return 0;
    h = FS_HandleForFileCurrentThread(filename);
    fsh[h].zipFile = 0;
    I_strncpyz(fsh[h].name, filename, 256);
    fsh[h].handleFiles.file.o = f;
    fsh[h].handleSync = 0;
    if (!fsh[h].handleFiles.file.o)
    {
        FS_FCloseFile(h);
        return 0;
    }
    return h;
}

int __cdecl FS_FilenameCompare(const char *s1, const char *s2)
{
    int c2; // [esp+0h] [ebp-8h]
    int c1; // [esp+4h] [ebp-4h]

    do
    {
        c1 = *s1++;
        c2 = *s2++;
        if ( I_islower(c1) )
            c1 -= 32;
        if ( I_islower(c2) )
            c2 -= 32;
        if ( c1 == 92 || c1 == 58 )
            c1 = 47;
        if ( c2 == 92 || c2 == 58 )
            c2 = 47;
        if ( c1 != c2 )
            return -1;
    }
    while ( c1 );
    return 0;
}

bool __cdecl FS_PureIgnoreFiles(const char *filename)
{
    const char *extension; // [esp+0h] [ebp-4h]

    if ( !I_stricmp(filename, "ban.txt") )
        return 1;
    extension = Com_GetExtensionSubString(filename);
    if ( *extension == 46 )
        ++extension;
    return !I_stricmp(extension, "cfg") || I_stricmp(extension, ".dm_NETWORK_PROTOCOL_VERSION") == 0;
}

int __cdecl FS_FOpenFileRead(const char *filename, int *file)
{
    com_fileAccessed = 1;
    return FS_FOpenFileReadCurrentThread(filename, file);
}

unsigned int __cdecl FS_FOpenFileReadCurrentThread(const char *filename, int *file)
{
    FsThread thread; // [esp+0h] [ebp-4h]

    thread = FS_GetCurrentThread();
    if ( thread != FS_THREAD_INVALID )
        return FS_FOpenFileReadForThread(filename, file, thread);
    Com_PrintError(1, "FS_FOpenFileReadCurrentThread for an unknown thread\n");
    if ( file )
        *file = 0;
    return -1;
}

unsigned int __cdecl FS_FOpenFileReadForThread(const char *filename, int *file, FsThread thread)
{
    const char *CurrentThreadName; // eax
    const char *v5; // eax
    const char *v6; // eax
    int v7; // [esp-10h] [ebp-348h]
    int v8; // [esp-Ch] [ebp-344h]
    directory_t *v9; // [esp-8h] [ebp-340h]
    char *gamedir; // [esp-4h] [ebp-33Ch]
    char copypath[256]; // [esp+0h] [ebp-338h] BYREF
    fileInIwd_s *i; // [esp+100h] [ebp-238h]
    int v13; // [esp+104h] [ebp-234h]
    iwd_t *v14; // [esp+108h] [ebp-230h]
    fileInIwd_s *iwdFile; // [esp+10Ch] [ebp-22Ch]
    int hash; // [esp+110h] [ebp-228h]
    const iwd_t *iwd; // [esp+114h] [ebp-224h]
    char sanitizedName[256]; // [esp+118h] [ebp-220h] BYREF
    directory_t *dir; // [esp+218h] [ebp-120h]
    unz_s *zfi; // [esp+21Ch] [ebp-11Ch]
    const char *impureIwd; // [esp+220h] [ebp-118h]
    file_in_zip_read_info_s *ziptemp; // [esp+224h] [ebp-114h]
    char netpath[256]; // [esp+228h] [ebp-110h] BYREF
    bool wasSkipped; // [esp+32Fh] [ebp-9h]
    searchpath_s *search; // [esp+330h] [ebp-8h]
    _iobuf *filetemp; // [esp+334h] [ebp-4h]

    impureIwd = 0;
    wasSkipped = 0;
    if ( !filename
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_files.cpp", 2192, 0, "%s", "filename") )
    {
        __debugbreak();
    }
    FS_CheckFileSystemStarted();
    if ( FS_SanitizeFilename(filename, sanitizedName, 256) )
    {
        if ( file )
        {
            *file = FS_HandleForFile(filename, thread);
            if ( *file )
            {
                for ( search = fs_searchpaths; search; search = search->next )
                {
                    if ( FS_UseSearchPath(search) )
                    {
                        v14 = search->iwd;
                        if ( v14 && v14->numFiles )
                        {
                            if ( (!v14->hashTable || !v14->hashSize)
                                && !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_files.cpp",
                                            2276,
                                            0,
                                            "%s",
                                            "iwd->hashTable && iwd->hashSize") )
                            {
                                __debugbreak();
                            }
                            v13 = FS_HashFileName(sanitizedName, v14->hashSize);
                            for ( i = v14->hashTable[v13]; i; i = i->next )
                            {
                                if ( !FS_FilenameCompare(i->name, sanitizedName) )
                                {
                                    if ( !g_disablePureCheck && !search->bLocalized && !search->ignorePureCheck && !FS_IwdIsPure(v14) )
                                    {
                                        impureIwd = (const char *)v14;
                                        break;
                                    }
                                    if ( !v14->referenced && !FS_FilesAreLoadedGlobally(sanitizedName) )
                                    {
                                        v14->referenced = 1;
                                        FS_AddIwdPureCheckReference(search);
                                    }
                                    if ( _InterlockedCompareExchange(&v14->hasOpenFile, 1, 0) == 1 )
                                    {
                                        fsh[*file].handleFiles.iwdIsClone = 1;
                                        fsh[*file].handleFiles.file.o = (_iobuf *)unzReOpen(v14->iwdFilename, v14->handle);
                                        if (!fsh[*file].handleFiles.file.o)
                                        {
                                            if (thread)
                                            {
                                                FS_FCloseFile(*file);
                                                *file = 0;
                                                return -1;
                                            }
                                            Com_Error(ERR_FATAL, "Couldn't reopen %s", v14);
                                        }
                                    }
                                    else
                                    {
                                        fsh[*file].handleFiles.iwdIsClone = 0;
                                        fsh[*file].handleFiles.file.o = (_iobuf *)v14->handle;
                                    }
                                    I_strncpyz(fsh[*file].name, sanitizedName, 256);
                                    fsh[*file].zipFile = v14;
                                    zfi = (unz_s *)fsh[*file].handleFiles.file.o;
                                    filetemp = zfi->file;
                                    ziptemp = zfi->pfile_in_zip_read;
                                    unzSetCurrentFileInfoPosition(v14->handle, i->pos);
                                    Com_Memcpy(zfi, v14->handle, 128);
                                    zfi->file = filetemp;
                                    zfi->pfile_in_zip_read = ziptemp;
                                    unzOpenCurrentFile(fsh[*file].handleFiles.file.z);
                                    fsh[*file].zipFilePos = i->pos;
                                    if ( fs_debug->current.integer )
                                    {
                                        v8 = *file;
                                        CurrentThreadName = Sys_GetCurrentThreadName();
                                        Com_Printf(
                                            10,
                                            "FS_FOpenFileReadfrom thread '%s', handle '%d', %s (found in '%s')\n",
                                            CurrentThreadName,
                                            v8,
                                            sanitizedName,
                                            v14->iwdFilename);
                                    }
                                    return zfi->cur_file_info.uncompressed_size;
                                }
                            }
                        }
                        else if ( search->dir )
                        {
                            if ( !search->ignore && !fs_restrict->current.enabled && !fs_numServerIwds
                                || search->bLocalized
                                || search->ignorePureCheck
                                || FS_PureIgnoreFiles(sanitizedName) )
                            {
                                dir = search->dir;
                                FS_BuildOSPathForThread(dir->path, dir->gamedir, sanitizedName, netpath, thread);
                                fsh[*file].handleFiles.file.o = FS_FileOpenReadBinary(netpath);
                                if ( fsh[*file].handleFiles.file.o )
                                {
                                    if (!search->bLocalized && !search->ignorePureCheck && !FS_PureIgnoreFiles(sanitizedName))
                                        fs_fakeChkSum = rand() + 1;
                                    I_strncpyz(fsh[*file].name, sanitizedName, 256);
                                    fsh[*file].zipFile = 0;
                                    if ( fs_debug->current.integer )
                                    {
                                        gamedir = dir->gamedir;
                                        v9 = dir;
                                        v7 = *file;
                                        v5 = Sys_GetCurrentThreadName();
                                        Com_Printf(
                                            10,
                                            "FS_FOpenFileRead from thread '%s', handle '%d', %s (found in '%s/%s')\n",
                                            v5,
                                            v7,
                                            sanitizedName,
                                            v9->path,
                                            gamedir);
                                    }
                                    if ( fs_copyfiles->current.enabled && !I_stricmp(dir->path, fs_cdpath->current.string) )
                                    {
                                        FS_BuildOSPathForThread(
                                            (char *)fs_basepath->current.integer,
                                            dir->gamedir,
                                            sanitizedName,
                                            copypath,
                                            thread);
                                        FS_CopyFile(netpath, copypath);
                                    }
                                    return FS_filelength(*file);
                                }
                            }
                            else if ( !wasSkipped )
                            {
                                dir = search->dir;
                                FS_BuildOSPathForThread(dir->path, dir->gamedir, sanitizedName, netpath, thread);
                                filetemp = FS_FileOpenReadBinary(netpath);
                                if ( filetemp )
                                {
                                    wasSkipped = 1;
                                    FS_FileClose(filetemp);
                                }
                            }
                        }
                    }
                }
                if ( fs_debug->current.integer && thread == FS_THREAD_MAIN )
                    Com_Printf(10, "Can't find %s\n", filename);
                FS_FCloseFile(*file);
                *file = 0;
                if ( impureIwd )
                {
                    v6 = va("%s\n%s", "Impure client detected. Invalid .IWD files referenced!", impureIwd);
                    Com_Error(ERR_DROP, v6);
                }
                if ( wasSkipped )
                {
                    if ( fs_numServerIwds || fs_restrict->current.enabled )
                        Com_Printf(10, "Error: %s must be in an IWD\n", filename);
                    else
                        Com_Printf(10, "Error: %s must be in an IWD or not in the main directory\n", filename);
                    return -2;
                }
                else
                {
                    return -1;
                }
            }
            else
            {
                return -1;
            }
        }
        else
        {
            for ( search = fs_searchpaths; search; search = search->next )
            {
                if ( FS_UseSearchPath(search) )
                {
                    iwd = search->iwd;
                    if ( iwd && iwd->numFiles )
                    {
                        if ( (!iwd->hashTable || !iwd->hashSize)
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_files.cpp",
                                        2219,
                                        0,
                                        "%s",
                                        "iwd->hashTable && iwd->hashSize") )
                        {
                            __debugbreak();
                        }
                        hash = FS_HashFileName(sanitizedName, iwd->hashSize);
                        for ( iwdFile = iwd->hashTable[hash]; iwdFile; iwdFile = iwdFile->next )
                        {
                            if ( !FS_FilenameCompare(iwdFile->name, sanitizedName) )
                                return 1;
                        }
                    }
                    else if ( search->dir )
                    {
                        dir = search->dir;
                        FS_BuildOSPathForThread(dir->path, dir->gamedir, sanitizedName, netpath, thread);
                        filetemp = FS_FileOpenReadBinary(netpath);
                        if ( filetemp )
                        {
                            FS_FileClose(filetemp);
                            return 1;
                        }
                    }
                }
            }
            return -1;
        }
    }
    else
    {
        if ( file )
            *file = 0;
        if ( fs_debug->current.integer )
            Com_Printf(10, "fs_debug: %s is invalid or contains and invalid substring\n", filename);
        return -1;
    }
}

char __cdecl FS_SanitizeFilename(const char *filename, char *sanitizedName, int sanitizedNameSize)
{
    const char *v4; // eax
    char v7; // [esp+8h] [ebp-14h]
    char v9; // [esp+10h] [ebp-Ch]
    char v10; // [esp+11h] [ebp-Bh]
    char v11; // [esp+12h] [ebp-Ah]
    char v12; // [esp+13h] [ebp-9h]
    int srcIndex; // [esp+14h] [ebp-8h]
    int dstIndex; // [esp+18h] [ebp-4h]

    if ( !filename
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_files.cpp", 2102, 0, "%s", "filename") )
    {
        __debugbreak();
    }
    if ( !sanitizedName
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_files.cpp", 2103, 0, "%s", "sanitizedName") )
    {
        __debugbreak();
    }
    if ( sanitizedNameSize <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_files.cpp",
                    2104,
                    0,
                    "%s\n\t(sanitizedNameSize) = %i",
                    "(sanitizedNameSize > 0)",
                    sanitizedNameSize) )
    {
        __debugbreak();
    }
    for ( srcIndex = 0; ; ++srcIndex )
    {
        v12 = filename[srcIndex];
        if ( v12 != 47 && v12 != 92 )
            break;
    }
    dstIndex = 0;
    while ( filename[srcIndex] )
    {
        if ( FS_IsBackupSubStr(&filename[srcIndex]) )
            return 0;
        if ( filename[srcIndex] != 46
            || filename[srcIndex + 1] && ((v11 = filename[srcIndex + 1], v11 == 47) || v11 == 92 ? (v7 = 1) : (v7 = 0), !v7) )
        {
            if ( dstIndex + 1 >= sanitizedNameSize )
            {
                v4 = va("%i + 1 > %i", dstIndex, sanitizedNameSize);
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_files.cpp",
                                2120,
                                0,
                                "%s\n\t%s",
                                "dstIndex + 1 < sanitizedNameSize",
                                v4) )
                    __debugbreak();
                return 0;
            }
            v10 = filename[srcIndex];
            if ( v10 == 47 || v10 == 92 )
            {
                sanitizedName[dstIndex] = 47;
                while ( 1 )
                {
                    v9 = filename[srcIndex + 1];
                    if ( v9 != 47 && v9 != 92 )
                        break;
                    ++srcIndex;
                }
            }
            else
            {
                sanitizedName[dstIndex] = filename[srcIndex];
            }
            ++dstIndex;
        }
        ++srcIndex;
    }
    if ( dstIndex > srcIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_files.cpp",
                    2135,
                    1,
                    "dstIndex <= srcIndex\n\t%i, %i",
                    dstIndex,
                    srcIndex) )
    {
        __debugbreak();
    }
    sanitizedName[dstIndex] = 0;
    return 1;
}

bool __cdecl FS_IsBackupSubStr(const char *filenameSubStr)
{
    if ( *filenameSubStr == 46 && filenameSubStr[1] == 46 )
        return 1;
    return *filenameSubStr == 58 && filenameSubStr[1] == 58;
}

char __cdecl FS_FilesAreLoadedGlobally(const char *filename)
{
    const char *extensions[8]; // [esp+20h] [ebp-28h]
    int filenameLen; // [esp+40h] [ebp-8h]
    int extensionNum; // [esp+44h] [ebp-4h]

    extensions[0] = ".hlsl";
    extensions[1] = ".txt";
    extensions[2] = ".cfg";
    extensions[3] = ".levelshots";
    extensions[4] = ".menu";
    extensions[5] = ".arena";
    extensions[6] = ".str";
    extensions[7] = "";
    filenameLen = strlen(filename);
    for ( extensionNum = 0; *extensions[extensionNum]; ++extensionNum )
    {
        if ( !I_stricmp(&filename[filenameLen - strlen(extensions[extensionNum])], extensions[extensionNum]) )
            return 1;
    }
    return 0;
}

bool __cdecl FS_TouchFile(const char *name)
{
    return FS_FOpenFileRead(name, 0) != -1;
}

bool __cdecl FS_Delete(char *filename)
{
    char ospath[256]; // [esp+0h] [ebp-108h] BYREF
    const char *basepath; // [esp+104h] [ebp-4h]

    FS_CheckFileSystemStarted();
    if ( !filename
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_files.cpp", 2606, 0, "%s", "filename") )
    {
        __debugbreak();
    }
    if ( !*filename )
        return 0;
    basepath = fs_homepath->current.string;
    FS_BuildOSPath((char *)basepath, fs_gamedir, filename, ospath);
    return remove(ospath) != -1;
}

unsigned int __cdecl FS_Read(unsigned __int8 *buffer, unsigned int len, int h)
{
    int tries; // [esp+4h] [ebp-14h]
    unsigned int remaining; // [esp+8h] [ebp-10h]
    unsigned __int8 *buf; // [esp+Ch] [ebp-Ch]
    _iobuf *f; // [esp+10h] [ebp-8h]
    int read; // [esp+14h] [ebp-4h]

    FS_CheckFileSystemStarted();
    if (!h)
        return 0;
    if (fsh[h].zipFile)
        return unzReadCurrentFile(fsh[h].handleFiles.file.z, buffer, len);
    f = FS_FileForHandle(h);
    buf = buffer;
    remaining = len;
    tries = 0;
    while (remaining)
    {
        read = FS_FileRead(buf, remaining, f);
        if (!read)
        {
            if (tries)
                return len - remaining;
            tries = 1;
        }
        if (read == -1)
        {
            if (h >= 50 && h < 61)
                return -1;
            Com_Error(ERR_FATAL, "FS_Read: -1 bytes read");
        }
        remaining -= read;
        buf += read;
    }
    return len;
}

unsigned int __cdecl FS_Write(char *buffer, unsigned int len, int h)
{
    int tries; // [esp+4h] [ebp-14h]
    unsigned int remaining; // [esp+8h] [ebp-10h]
    int written; // [esp+10h] [ebp-8h]
    _iobuf *f; // [esp+14h] [ebp-4h]

    FS_CheckFileSystemStarted();
    if (!h)
        return 0;
    f = FS_FileForHandle(h);
    remaining = len;
    tries = 0;
    while (remaining)
    {
        written = FS_FileWrite(buffer, remaining, f);
        if (!written)
        {
            if (tries)
                return 0;
            tries = 1;
        }
        if (written == -1)
            return 0;
        remaining -= written;
        buffer += written;
    }
    if (fsh[h].handleSync)
        fflush(f);
    return len;
}

unsigned int __cdecl FS_WriteToDemo(char *buffer, unsigned int len, int h)
{
    return FS_Write(buffer, len, h);
}

void FS_Printf(int h, char *fmt, ...)
{
    char string[4100]; // [esp+14h] [ebp-1008h] BYREF
    va_list va; // [esp+102Ch] [ebp+10h] BYREF

    va_start(va, fmt);
    _vsnprintf(string, 0x1000u, fmt, va);
    FS_Write(string, &string[strlen(string) + 1] - &string[1], h);
}

int __cdecl FS_Seek(int f, int offset, int origin)
{
    unsigned int CurrentFile; // eax
    const char *v5; // eax
    _iobuf *v6; // eax
    signed int iZipPos; // [esp+8h] [ebp-8h]
    unsigned int iZipOffset; // [esp+Ch] [ebp-4h]

    FS_CheckFileSystemStarted();
    if (fsh[f].streamed
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_files.cpp",
            3326,
            0,
            "%s",
            "!fsh[f].streamed"))
    {
        __debugbreak();
    }
    if (!fsh[f].zipFile)
    {
        v6 = FS_FileForHandle(f);
        return FS_FileSeek(v6, offset, origin);
    }
    if (!offset && origin == 2)
    {
        unzSetCurrentFileInfoPosition(fsh[f].handleFiles.file.z, fsh[f].zipFilePos);
        return unzOpenCurrentFile(fsh[f].handleFiles.file.z);
    }
    if (!offset && !origin)
        return 0;
    iZipPos = unztell(fsh[f].handleFiles.file.z);
    switch (origin)
    {
    case 0:
        if (!offset
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_files.cpp",
                3347,
                0,
                "%s",
                "offset != 0"))
        {
            __debugbreak();
        }
        if (offset >= 0)
        {
            CurrentFile = unzReadCurrentFile(fsh[f].handleFiles.file.z, 0, offset);
        }
        else
        {
            unzSetCurrentFileInfoPosition(fsh[f].handleFiles.file.z, fsh[f].zipFilePos);
            unzOpenCurrentFile(fsh[f].handleFiles.file.z);
            CurrentFile = unzReadCurrentFile(fsh[f].handleFiles.file.z, 0, offset + iZipPos);
        }
        goto LABEL_30;
    case 1:
        if (offset + FS_filelength(f) >= iZipPos)
        {
            iZipOffset = offset + FS_filelength(f) - iZipPos;
        }
        else
        {
            unzSetCurrentFileInfoPosition(fsh[f].handleFiles.file.z, fsh[f].zipFilePos);
            unzOpenCurrentFile(fsh[f].handleFiles.file.z);
            iZipOffset = offset + FS_filelength(f);
        }
        CurrentFile = unzReadCurrentFile(fsh[f].handleFiles.file.z, 0, iZipOffset);
        goto LABEL_30;
    case 2:
        if (offset >= iZipPos)
        {
            CurrentFile = unzReadCurrentFile(fsh[f].handleFiles.file.z, 0, offset - iZipPos);
        }
        else
        {
            unzSetCurrentFileInfoPosition(fsh[f].handleFiles.file.z, fsh[f].zipFilePos);
            unzOpenCurrentFile(fsh[f].handleFiles.file.z);
            CurrentFile = unzReadCurrentFile(fsh[f].handleFiles.file.z, 0, offset);
        }
    LABEL_30:
        if (CurrentFile)
            return 0;
        else
            return -1;
    }
    v5 = va("Bad origin %i in FS_Seek", origin);
    if (!Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_files.cpp", 3391, 0, v5))
        __debugbreak();
    return -1;
}

int __cdecl FS_ReadFile(const char *qpath, void **buffer)
{
    unsigned __int8 *buf; // [esp+0h] [ebp-Ch]
    int len; // [esp+4h] [ebp-8h]
    int h; // [esp+8h] [ebp-4h] BYREF

    FS_CheckFileSystemStarted();
    if ( !qpath || !*qpath )
        Com_Error(ERR_FATAL, "FS_ReadFile with empty name");
    len = FS_FOpenFileReadCurrentThread(qpath, &h);
    if ( h )
    {
        if ( buffer )
        {
            ++fs_loadStack;
            buf = (unsigned __int8 *)FS_AllocMem(len + 1);
            *buffer = buf;
            FS_Read(buf, len, h);
            buf[len] = 0;
        }
        FS_FCloseFile(h);
        return len;
    }
    else
    {
        if ( buffer )
            *buffer = 0;
        return -1;
    }
}

char *__cdecl FS_AllocMem(int bytes)
{
    return Z_VirtualAlloc(bytes, "FS_AllocMem", 0);
}

void __cdecl FS_ResetFiles()
{
    fs_loadStack = 0;
}

void __cdecl FS_FreeFile(void *buffer)
{
    FS_CheckFileSystemStarted();
    if ( !buffer
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_files.cpp", 3588, 0, "%s", "buffer") )
    {
        __debugbreak();
    }
    --fs_loadStack;
    FS_FreeMem(buffer);
}

void __cdecl FS_FreeMem(void *buffer)
{
    Z_VirtualFree(buffer, 0);
}

int __cdecl FS_WriteFile(char *filename, char *buffer, unsigned int size)
{
    int f; // [esp+0h] [ebp-8h]
    unsigned int actualSize; // [esp+4h] [ebp-4h]

    FS_CheckFileSystemStarted();
    if ( !filename
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_files.cpp", 3645, 0, "%s", "filename") )
    {
        __debugbreak();
    }
    if ( !buffer
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_files.cpp", 3646, 0, "%s", "buffer") )
    {
        __debugbreak();
    }
    f = FS_FOpenFileWrite(filename);
    if ( f )
    {
        actualSize = FS_Write(buffer, size, f);
        FS_FCloseFile(f);
        if ( actualSize == size )
        {
            return 1;
        }
        else
        {
            FS_Delete(filename);
            return 0;
        }
    }
    else
    {
        Com_Printf(10, "Failed to open %s\n", filename);
        return 0;
    }
}

int __cdecl FS_GetFileOsPath(const char *filename, char *ospath)
{
    char sanitizedName[256]; // [esp+0h] [ebp-110h] BYREF
    directory_t *dir; // [esp+104h] [ebp-Ch]
    searchpath_s *search; // [esp+108h] [ebp-8h]
    _iobuf *fp; // [esp+10Ch] [ebp-4h]

    if ( !filename
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_files.cpp", 3681, 0, "%s", "filename") )
    {
        __debugbreak();
    }
    if ( !ospath
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_files.cpp", 3682, 0, "%s", "ospath") )
    {
        __debugbreak();
    }
    if ( !FS_SanitizeFilename(filename, sanitizedName, 256) )
        return -1;
    for ( search = fs_searchpaths; search; search = search->next )
    {
        if ( FS_UseSearchPath(search) )
        {
            if ( !search->iwd )
            {
                dir = search->dir;
                FS_BuildOSPathForThread(dir->path, dir->gamedir, sanitizedName, ospath, FS_THREAD_MAIN);
                fp = FS_FileOpenReadBinary(ospath);
                if ( fp )
                {
                    FS_FileClose(fp);
                    return 0;
                }
            }
        }
    }
    return -1;
}

int __cdecl FS_OpenFileOverwrite(const char *qpath)
{
    DWORD FileAttributesA; // eax
    char ospath[256]; // [esp+4h] [ebp-108h] BYREF
    unsigned int attributes; // [esp+108h] [ebp-4h]

    FS_CheckFileSystemStarted();
    if ( !qpath
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_files.cpp", 3720, 0, "%s", "qpath") )
    {
        __debugbreak();
    }
    if ( FS_GetFileOsPath(qpath, ospath) < 0 )
        Com_Error(
            ERR_DROP,
            "FS_FOpenFileOverWrite: Failed to open %s for writing.    It either does not exist or is in a iwd file.",
            qpath);
    if ( fs_debug->current.integer )
        Com_Printf(10, "FS_FOpenFileOverWrite: %s\n", ospath);
    FileAttributesA = GetFileAttributesA(ospath);
    attributes = FileAttributesA & 0xFFFFFFFE;
    if ( (FileAttributesA & 0xFFFFFFFE) != FileAttributesA )
        SetFileAttributesA(ospath, attributes);
    return FS_GetHandleAndOpenFile(qpath, ospath, FS_THREAD_MAIN);
}

const char **__cdecl FS_ListFilteredFiles(
                searchpath_s *searchPath,
                const char *path,
                char *extension,
                char *filter,
                FsListBehavior_e behavior,
                int *numfiles)
{
    char v7; // [esp+13h] [ebp-3E1h]
    char *v8; // [esp+18h] [ebp-3DCh]
    char *v9; // [esp+1Ch] [ebp-3D8h]
    char *v10; // [esp+5Ch] [ebp-398h]
    char netpath[256]; // [esp+64h] [ebp-390h] BYREF
    int numSysFiles; // [esp+164h] [ebp-290h] BYREF
    char **sysFiles; // [esp+168h] [ebp-28Ch]
    char szTrimmedName[68]; // [esp+16Ch] [ebp-288h] BYREF
    int depth; // [esp+1B0h] [ebp-244h] BYREF
    char *name; // [esp+1B4h] [ebp-240h]
    int zpathLen; // [esp+1B8h] [ebp-23Ch]
    int pathDepth; // [esp+1BCh] [ebp-238h] BYREF
    iwd_t *iwd; // [esp+1C0h] [ebp-234h]
    char zpath[259]; // [esp+1C4h] [ebp-230h] BYREF
    bool isDirSearch; // [esp+2C7h] [ebp-12Dh]
    const char **list; // [esp+2C8h] [ebp-12Ch]
    int extensionLength; // [esp+2CCh] [ebp-128h]
    int nfiles; // [esp+2D0h] [ebp-124h]
    int pathLength; // [esp+2D4h] [ebp-120h]
    fileInIwd_s *buildBuffer; // [esp+2D8h] [ebp-11Ch]
    HunkUser *user; // [esp+2DCh] [ebp-118h]
    int temp; // [esp+2E0h] [ebp-114h]
    char sanitizedPath[256]; // [esp+2E4h] [ebp-110h] BYREF
    searchpath_s *search; // [esp+3E8h] [ebp-Ch]
    int i; // [esp+3ECh] [ebp-8h]
    int length; // [esp+3F0h] [ebp-4h]

    FS_CheckFileSystemStarted();
    if ( !path )
    {
        *numfiles = 0;
        return 0;
    }
    if ( !extension )
        extension = (char *)"";
    if ( !FS_SanitizeFilename(path, sanitizedPath, 256) )
    {
        *numfiles = 0;
        return 0;
    }
    isDirSearch = I_stricmp(extension, "/") == 0;
    v10 = &sanitizedPath[strlen(sanitizedPath) + 1];
    pathLength = v10 - &sanitizedPath[1];
    if ( v10 != &sanitizedPath[1] && (sanitizedPath[pathLength - 1] == 92 || sanitizedPath[pathLength - 1] == 47) )
        --pathLength;
    extensionLength = strlen(extension);
    nfiles = 0;
    FS_ReturnPath(sanitizedPath, zpath, &pathDepth);
    if ( sanitizedPath[0] )
        ++pathDepth;
    user = Hunk_UserCreate(0x20000, HU_SCHEME_DEFAULT, 0, 0, "FS_ListFilteredFiles", 3);
    list = (const char **)Hunk_UserAlloc(user, 65540, 4, 0);
    *list++ = (const char *)user;
    for ( search = searchPath; search; search = search->next )
    {
        if ( FS_UseSearchPath(search) )
        {
            if ( search->iwd )
            {
                if ( search->bLocalized || FS_IwdIsPure(search->iwd) )
                {
                    iwd = search->iwd;
                    buildBuffer = iwd->buildBuffer;
                    for ( i = 0; i < iwd->numFiles; ++i )
                    {
                        name = buildBuffer[i].name;
                        if ( filter )
                        {
                            if ( Com_FilterPath(filter, name, 0) )
                                nfiles = FS_AddFileToList(user, name, list, nfiles);
                            continue;
                        }
                        zpathLen = FS_ReturnPath(name, zpath, &depth);
                        if ( depth == pathDepth
                            && pathLength <= zpathLen
                            && (pathLength <= 0 || name[pathLength] == 47)
                            && !I_strnicmp(name, sanitizedPath, pathLength) )
                        {
                            if ( !isDirSearch )
                            {
                                if ( extensionLength )
                                {
                                    length = strlen(name);
                                    if ( length <= extensionLength
                                        || name[length - extensionLength - 1] != 46
                                        || I_stricmp(&name[length - extensionLength], extension) )
                                    {
                                        continue;
                                    }
                                }
LABEL_46:
                                temp = pathLength;
                                if ( pathLength )
                                    ++temp;
                                if ( isDirSearch )
                                {
                                    v9 = &name[temp];
                                    v8 = szTrimmedName;
                                    do
                                    {
                                        v7 = *v9;
                                        *v8++ = *v9++;
                                    }
                                    while ( v7 );
                                    szTrimmedName[&szTrimmedName[strlen(szTrimmedName) + 1] - &szTrimmedName[1] - 1] = 0;
                                    nfiles = FS_AddFileToList(user, szTrimmedName, list, nfiles);
                                }
                                else
                                {
                                    nfiles = FS_AddFileToList(user, &name[temp], list, nfiles);
                                }
                                continue;
                            }
                            if ( extensionLength != 1
                                && !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_files.cpp",
                                            4129,
                                            1,
                                            "%s",
                                            "extensionLength == 1") )
                            {
                                __debugbreak();
                            }
                            if ( (*extension != 47 || extension[1])
                                && !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_files.cpp",
                                            4130,
                                            1,
                                            "%s",
                                            "extension[0] == '/' && extension[1] == '\\0'") )
                            {
                                __debugbreak();
                            }
                            if ( name[strlen(name) - 1] == 47 )
                                goto LABEL_46;
                        }
                    }
                }
            }
            else if ( search->dir && (!fs_restrict->current.enabled && !fs_numServerIwds || behavior) )
            {
                FS_BuildOSPath(search->dir->path, search->dir->gamedir, sanitizedPath, netpath);
                sysFiles = (char **)Sys_ListFiles(netpath, extension, filter, &numSysFiles, isDirSearch);
                for ( i = 0; i < numSysFiles; ++i )
                    nfiles = FS_AddFileToList(user, sysFiles[i], list, nfiles);
                FS_FreeFileList((const char **)sysFiles);
            }
        }
    }
    *numfiles = nfiles;
    if ( nfiles )
    {
        list[nfiles] = 0;
        return list;
    }
    else
    {
        Hunk_UserDestroy(user);
        return 0;
    }
}

int __cdecl FS_ReturnPath(const char *zname, char *zpath, int *depth)
{
    char v3; // cl
    char *v5; // [esp+8h] [ebp-14h]
    const char *v6; // [esp+Ch] [ebp-10h]
    int at; // [esp+10h] [ebp-Ch]
    int len; // [esp+14h] [ebp-8h]
    int newdep; // [esp+18h] [ebp-4h]

    newdep = 0;
    *zpath = 0;
    len = 0;
    for ( at = 0; zname[at]; ++at )
    {
        if ( zname[at] == 47 || zname[at] == 92 )
        {
            len = at;
            ++newdep;
        }
    }
    v6 = zname;
    v5 = zpath;
    do
    {
        v3 = *v6;
        *v5++ = *v6++;
    }
    while ( v3 );
    zpath[len] = 0;
    if ( len + 1 == at )
        --newdep;
    *depth = newdep;
    return len;
}

int __cdecl FS_AddFileToList(HunkUser *user, const char *name, const char **list, int nfiles)
{
    int i; // [esp+0h] [ebp-4h]

    if ( nfiles == 0x3FFF )
        return 0x3FFF;
    for ( i = 0; i < nfiles; ++i )
    {
        if ( !I_stricmp(name, list[i]) )
            return nfiles;
    }
    list[nfiles] = Hunk_CopyString(user, name);
    return nfiles + 1;
}

const char **__cdecl FS_ListFiles(const char *path, char *extension, FsListBehavior_e behavior, int *numfiles)
{
    return FS_ListFilteredFiles(fs_searchpaths, path, extension, 0, behavior, numfiles);
}

const char **__cdecl FS_ListFilesInLocation(
                const char *path,
                char *extension,
                FsListBehavior_e behavior,
                int *numfiles,
                int lookInFlags)
{
    return FS_ListFilteredFilesInLocation(path, extension, 0, behavior, numfiles, lookInFlags);
}

const char **__cdecl FS_ListFilteredFilesInLocation(
                const char *path,
                char *extension,
                char *filter,
                FsListBehavior_e behavior,
                int *numfiles,
                int lookInFlags)
{
    const char **result; // [esp+0h] [ebp-18h]
    searchpath_s *locationSearchPath; // [esp+4h] [ebp-14h]
    HunkUser *user; // [esp+8h] [ebp-10h]
    char *pathDir; // [esp+Ch] [ebp-Ch]
    searchpath_s *search; // [esp+10h] [ebp-8h]
    searchpath_s *locationSearch; // [esp+14h] [ebp-4h]

    user = Hunk_UserCreate(0x20000, HU_SCHEME_DEFAULT, 0, 0, "FS_ListFilteredFilesInLocation", 3);
    locationSearchPath = 0;
    locationSearch = 0;
    for ( search = fs_searchpaths; search; search = search->next )
    {
        if ( search->dir )
        {
            pathDir = search->dir->gamedir;
        }
        else if ( search->iwd )
        {
            pathDir = search->iwd->iwdGamename;
        }
        else
        {
            pathDir = 0;
        }
        if ( !pathDir
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_files.cpp", 4298, 0, "%s", "pathDir") )
        {
            __debugbreak();
        }
        if ( FS_CheckLocation(pathDir, lookInFlags) )
        {
            if ( locationSearchPath )
            {
                locationSearch->next = (searchpath_s *)Hunk_UserAlloc(user, 28, 4, 0);
                locationSearch = locationSearch->next;
            }
            else
            {
                locationSearchPath = (searchpath_s *)Hunk_UserAlloc(user, 28, 4, 0);
                locationSearch = locationSearchPath;
            }
            locationSearch->next = 0;
            locationSearch->dir = search->dir;
            locationSearch->language = search->language;
            locationSearch->bLocalized = search->bLocalized;
            locationSearch->iwd = search->iwd;
        }
    }
    result = FS_ListFilteredFiles(locationSearchPath, path, extension, filter, behavior, numfiles);
    Hunk_UserDestroy(user);
    return result;
}

bool __cdecl FS_CheckLocation(const char *path, int lookInFlags)
{
    if ( lookInFlags == 63 )
        return 1;
    if ( (lookInFlags & 1) != 0 && !I_strncmp(path, "main", 4) )
        return 1;
    if ( (lookInFlags & 2) != 0 && !I_strncmp(path, "dev", 3) )
        return 1;
    if ( (lookInFlags & 4) != 0 && !I_strncmp(path, "temp", 4) )
        return 1;
    if ( (lookInFlags & 8) != 0 && !I_strncmp(path, "raw", 3) )
        return 1;
    if ( (lookInFlags & 0x10) != 0 && !I_strncmp(path, "raw_shared", 10) )
        return 1;
    return (lookInFlags & 0x20) != 0 && !I_strncmp(path, "devraw", 6);
}

void __cdecl FS_FreeFileList(const char **list)
{
    if ( list )
        Hunk_UserDestroy((HunkUser *)*(list - 1));
}

int __cdecl FS_GetFileList(const char *path, char *extension, FsListBehavior_e behavior, char *listbuf, int bufsize)
{
    char v6; // [esp+3h] [ebp-35h]
    char *v7; // [esp+8h] [ebp-30h]
    const char *v8; // [esp+Ch] [ebp-2Ch]
    const char **fileNames; // [esp+24h] [ebp-14h]
    int nLen; // [esp+28h] [ebp-10h]
    int nTotal; // [esp+2Ch] [ebp-Ch]
    int i; // [esp+30h] [ebp-8h]
    int fileCount; // [esp+34h] [ebp-4h] BYREF

    *listbuf = 0;
    fileCount = 0;
    nTotal = 0;
    if ( !I_stricmp(path, "$modlist") )
        return FS_GetModList(listbuf, bufsize);
    fileNames = FS_ListFiles(path, extension, behavior, &fileCount);
    for ( i = 0; i < fileCount; ++i )
    {
        nLen = strlen(fileNames[i]) + 1;
        if ( nTotal + nLen + 1 >= bufsize )
        {
            fileCount = i;
            break;
        }
        v8 = fileNames[i];
        v7 = listbuf;
        do
        {
            v6 = *v8;
            *v7++ = *v8++;
        }
        while ( v6 );
        listbuf += nLen;
        nTotal += nLen;
    }
    FS_FreeFileList(fileNames);
    return fileCount;
}

void __cdecl FS_ConvertPath(char *s)
{
    while ( *s )
    {
        if ( *s == 92 || *s == 58 )
            *s = 47;
        ++s;
    }
}

int __cdecl FS_PathCmp(const char *s1, const char *s2)
{
    int c2; // [esp+0h] [ebp-8h]
    int c1; // [esp+4h] [ebp-4h]

    do
    {
        c1 = *s1++;
        c2 = *s2++;
        if ( I_islower(c1) )
            c1 -= 32;
        if ( I_islower(c2) )
            c2 -= 32;
        if ( c1 == 92 || c1 == 58 )
            c1 = 47;
        if ( c2 == 92 || c2 == 58 )
            c2 = 47;
        if ( c1 < c2 )
            return -1;
        if ( c1 > c2 )
            return 1;
    }
    while ( c1 );
    return 0;
}

void __cdecl FS_SortFileList(const char **filelist, int numfiles)
{
    int j; // [esp+4h] [ebp-14h]
    int k; // [esp+8h] [ebp-10h]
    int numsortedfiles; // [esp+Ch] [ebp-Ch]
    int i; // [esp+10h] [ebp-8h]
    unsigned int *sortedlist; // [esp+14h] [ebp-4h]

    sortedlist = Z_Malloc(4 * numfiles + 4, "FS_SortFileList", 3);
    *sortedlist = 0;
    numsortedfiles = 0;
    for ( i = 0; i < numfiles; ++i )
    {
        for ( j = 0; j < numsortedfiles && FS_PathCmp(filelist[i], (const char *)sortedlist[j]) >= 0; ++j )
            ;
        for ( k = numsortedfiles; k > j; --k )
            sortedlist[k] = sortedlist[k - 1];
        sortedlist[j] = (unsigned int)filelist[i];
        ++numsortedfiles;
    }
    Com_Memcpy(filelist, sortedlist, 4 * numfiles);
    Z_Free((char *)sortedlist, 3);
}

void __cdecl FS_DisplayPath(int bLanguageCull)
{
    const char *LanguageName; // eax
    const char *v2; // eax
    unsigned int iLanguage; // [esp+0h] [ebp-10h]
    searchpath_s *s; // [esp+4h] [ebp-Ch]
    const char *pszLanguageName; // [esp+8h] [ebp-8h]
    int i; // [esp+Ch] [ebp-4h]

    iLanguage = SEH_GetCurrentLanguage();
    pszLanguageName = SEH_GetLanguageName(iLanguage);
    Com_Printf(10, "Current language: %s\n", pszLanguageName);
    if ( fs_ignoreLocalized->current.enabled )
        Com_Printf(10, "        localized assets are being ignored\n");
    Com_Printf(10, "Current search path:\n");
    for ( s = fs_searchpaths; s; s = s->next )
    {
        if ( !bLanguageCull || FS_UseSearchPath(s) )
        {
            if ( s->iwd )
            {
                Com_Printf(10, "%s (%i files)\n", s->iwd->iwdFilename, s->iwd->numFiles);
                if ( s->bLocalized )
                {
                    LanguageName = SEH_GetLanguageName(s->language);
                    Com_Printf(10, "        localized assets iwd file for %s\n", LanguageName);
                }
                if ( fs_numServerIwds )
                {
                    if ( FS_IwdIsPure(s->iwd) )
                        Com_Printf(10, "        on the pure list\n");
                    else
                        Com_Printf(10, "        not on the pure list\n");
                }
            }
            else
            {
                Com_Printf(10, "%s/%s\n", s->dir->path, s->dir->gamedir);
                if ( s->bLocalized )
                {
                    v2 = SEH_GetLanguageName(s->language);
                    Com_Printf(10, "        localized assets game folder for %s\n", v2);
                }
            }
        }
    }
    Com_Printf(10, "\nFile Handles:\n");
    for ( i = 1; i < 70; ++i )
    {
        if ( fsh[i].handleFiles.file.o )
            Com_Printf(10, "handle %i: %s\n", i, fsh[i].name);
    }
}

void __cdecl FS_FullPath_f()
{
    FS_DisplayPath(0);
}

void __cdecl FS_Path_f()
{
    FS_DisplayPath(1);
}

void __cdecl FS_AddUserMapDirIWDs(char *pszGameFolder)
{
    searchpath_s *i; // [esp+4h] [ebp-4h]

    for ( i = fs_searchpaths; i; i = i->next )
    {
        if ( i->iwd && !I_stricmp(i->iwd->iwdGamename, pszGameFolder) )
            return;
    }
    FS_AddIwdFilesForGameDirectory((char *)fs_homepath->current.integer, pszGameFolder);
}

int bLanguagesListed;
void __cdecl FS_AddIwdFilesForGameDirectory(char *path, char *pszGameFolder)
{
    const char *Language; // eax
    const char *v3; // eax
    const char *LanguageName; // [esp-4h] [ebp-115Ch]
    int v5; // [esp+0h] [ebp-1158h]
    char *iwdGamename; // [esp+8h] [ebp-1150h]
    char *v7; // [esp+Ch] [ebp-114Ch]
    BOOL v8; // [esp+10h] [ebp-1148h]
    bool v9; // [esp+27h] [ebp-1131h]
    int iLanguage; // [esp+28h] [ebp-1130h]
    BOOL mainAndBase; // [esp+2Ch] [ebp-112Ch]
    char ospath[256]; // [esp+30h] [ebp-1128h] BYREF
    iwd_t *ZipFile; // [esp+134h] [ebp-1024h]
    int v14; // [esp+138h] [ebp-1020h]
    char **list; // [esp+13Ch] [ebp-101Ch]
    char *pszLanguageName; // [esp+140h] [ebp-1018h]
    searchpath_s *search; // [esp+144h] [ebp-1014h]
    bool v18; // [esp+14Bh] [ebp-100Dh]
    int i; // [esp+14Ch] [ebp-100Ch]
    int piLanguageIndex; // [esp+150h] [ebp-1008h] BYREF
    int numfiles; // [esp+154h] [ebp-1004h] BYREF
    char *s0[1024]; // [esp+158h] [ebp-1000h] BYREF

    v9 = 1;
    if (!loc_language || loc_language->current.integer != 4)
    {
        if (loc_language
            || (LanguageName = SEH_GetLanguageName(4), Language = Win_GetLanguage(), I_stricmp(Language, LanguageName)))
        {
            v9 = 0;
        }
    }
    v18 = v9;
    FS_BuildOSPath(path, pszGameFolder, (char*)"", ospath);
    ospath[&ospath[strlen(ospath) + 1] - &ospath[1] - 1] = 0;
    list = (char**)Sys_ListFiles(ospath, (char*)"iwd", 0, &numfiles, 0);
    if (numfiles > 1024)
    {
        Com_PrintWarning(
            10,
            "WARNING: Exceeded max number of iwd files in %s/%s (%1/%1)\n",
            path,
            pszGameFolder,
            numfiles,
            1024);
        numfiles = 1024;
    }
    v8 = !I_stricmp(pszGameFolder, "main") && !I_stricmp(path, fs_basepath->current.string);
    mainAndBase = v8;
    for (i = 0; i < numfiles; ++i)
    {
        s0[i] = list[i];
        if (!I_strncmp(s0[i], "localized_", 10))
            memcpy(s0[i], "          ", 10);
    }
    qsort(s0, numfiles, 4u, (int(__cdecl *)(const void *, const void *))iwdsort);
    for (i = 0; i < numfiles; ++i)
    {
        if (I_strncmp(s0[i], TEN_SPACE, 10))
        {
            if (mainAndBase && I_strnicmp(s0[i], "iw_", 3))
            {
                Com_PrintWarning(10, "WARNING: Invalid IWD %s in \\main.\n", s0[i]);
                continue;
            }
            v14 = 0;
            piLanguageIndex = 0;
        }
        else
        {
            memcpy(s0[i], "localized_", 10);
            v14 = 1;
            pszLanguageName = (char *)IwdFileLanguage(s0[i]);
            if (!*pszLanguageName)
            {
                Com_PrintWarning(
                    10,
                    "WARNING: Localized assets iwd file %s/%s/%s has invalid name (no language specified). Proper naming convention"
                    " is: localized_[language]_iwd#.iwd\n",
                    path,
                    pszGameFolder,
                    s0[i]);
                continue;
            }
            if (!SEH_GetLanguageIndexForName(pszLanguageName, &piLanguageIndex))
            {
                Com_PrintWarning(
                    10,
                    "WARNING: Localized assets iwd file %s/%s/%s has invalid name (bad language name specified). Proper naming conv"
                    "ention is: localized_[language]_iwd#.iwd\n",
                    path,
                    pszGameFolder,
                    s0[i]);
                if (!bLanguagesListed)
                {
                    Com_Printf(10, "Supported languages are:\n");
                    for (iLanguage = 0; iLanguage < 13; ++iLanguage)
                    {
                        v3 = SEH_GetLanguageName(iLanguage);
                        Com_Printf(10, "    %s\n", v3);
                    }
                    bLanguagesListed = 1;
                }
                continue;
            }
            if (piLanguageIndex == 3 && v18)
                piLanguageIndex = 4;
        }
        FS_BuildOSPath(path, pszGameFolder, s0[i], ospath);
        ZipFile = FS_LoadZipFile(ospath, s0[i]);
        if (!ZipFile)
            ZipFile = FS_LoadFakeIwdFile(ospath, s0[i]);
        if (ZipFile)
        {
            v7 = pszGameFolder;
            iwdGamename = ZipFile->iwdGamename;
            do
            {
                //HIBYTE(v5) = *v7;
                v5 = *v7;
                *iwdGamename++ = *v7++;
                //} while (HIBYTE(v5));
            } while (v5);
            search = (searchpath_s *)Z_Malloc(28, "FS_AddIwdFilesForGameDirectory", 3);
            search->iwd = ZipFile;
            search->bLocalized = v14;
            search->language = piLanguageIndex;
            FS_AddSearchPath(search);
        }
    }
    FS_FreeFileList((const char **)list);
}

iwd_t *__cdecl FS_LoadZipFile(char *zipfile, const char *basename)
{
    char v3; // [esp+17h] [ebp-1DDh]
    char *name; // [esp+1Ch] [ebp-1D8h]
    char *v5; // [esp+20h] [ebp-1D4h]
    unsigned __int8 *uf; // [esp+70h] [ebp-184h]
    char *namePtr; // [esp+74h] [ebp-180h]
    int hash; // [esp+7Ch] [ebp-178h]
    int fs_numHeaderLongs; // [esp+80h] [ebp-174h]
    int *fs_headerLongs; // [esp+84h] [ebp-170h]
    unsigned int *iwd; // [esp+88h] [ebp-16Ch]
    unsigned int len; // [esp+8Ch] [ebp-168h]
    fileInIwd_s *buildBuffer; // [esp+90h] [ebp-164h]
    unz_file_info_s file_info; // [esp+94h] [ebp-160h] BYREF
    char filename_inzip[256]; // [esp+E4h] [ebp-110h] BYREF
    unsigned int i; // [esp+1E8h] [ebp-Ch]
    unz_global_info_s gi; // [esp+1ECh] [ebp-8h] BYREF

    fs_numHeaderLongs = 0;
    uf = (unsigned __int8 *)unzOpen(zipfile);
    if ( unzGetGlobalInfo(uf, &gi) )
        return 0;
    fs_iwdFileCount += gi.number_entry;
    len = 0;
    unzGoToFirstFile(uf);
    for ( i = 0; i < gi.number_entry && !unzGetCurrentFileInfo(uf, &file_info, filename_inzip, 0x100u, 0, 0, 0, 0); ++i )
    {
        len += &filename_inzip[strlen(filename_inzip) + 1] - &filename_inzip[1] + 1;
        unzGoToNextFile(uf);
    }
    buildBuffer = (fileInIwd_s *)Z_Malloc(len + 12 * gi.number_entry, "FS_LoadZipFile1", 3);
    namePtr = (char *)&buildBuffer[gi.number_entry];
    fs_headerLongs = (int *)Z_Malloc(4 * gi.number_entry, "FS_LoadZipFile2", 3);
    for ( i = 1; i <= 0x400 && i <= gi.number_entry; i *= 2 )
        ;
    iwd = Z_Malloc(4 * i + 804, "FS_LoadZipFile3", 3);
    iwd[198] = i;
    iwd[199] = (unsigned int)(iwd + 201);
    for ( i = 0; i < iwd[198]; ++i )
        *(unsigned int *)(iwd[199] + 4 * i) = 0;
    I_strncpyz((char *)iwd, zipfile, 256);
    I_strncpyz((char *)iwd + 256, basename, 256);
    if ( strlen((const char *)iwd + 256) > 4
        && !I_stricmp((const char *)iwd + strlen((const char *)iwd + 256) + 252, ".iwd") )
    {
        *((_BYTE *)iwd + strlen((const char *)iwd + 256) + 252) = 0;
    }
    iwd[192] = (unsigned int)uf;
    iwd[196] = gi.number_entry;
    iwd[195] = 0;
    unzGoToFirstFile(uf);
    for ( i = 0; i < gi.number_entry && !unzGetCurrentFileInfo(uf, &file_info, filename_inzip, 0x100u, 0, 0, 0, 0); ++i )
    {
        if ( file_info.uncompressed_size )
            fs_headerLongs[fs_numHeaderLongs++] = file_info.crc;
        I_strlwr(filename_inzip);
        hash = FS_HashFileName(filename_inzip, iwd[198]);
        buildBuffer[i].name = namePtr;
        v5 = filename_inzip;
        name = buildBuffer[i].name;
        do
        {
            v3 = *v5;
            *name++ = *v5++;
        }
        while ( v3 );
        namePtr += &filename_inzip[strlen(filename_inzip) + 1] - &filename_inzip[1] + 1;
        unzGetCurrentFileInfoPosition(uf, (unsigned long*)&buildBuffer[i].pos);
        buildBuffer[i].next = *(fileInIwd_s **)(iwd[199] + 4 * hash);
        *(unsigned int *)(iwd[199] + 4 * hash) = (unsigned int)&buildBuffer[i];
        unzGoToNextFile(uf);
    }
    iwd[193] = Com_BlockChecksumKey32((const unsigned __int8 *)fs_headerLongs, 4 * fs_numHeaderLongs, 0);
    if ( fs_checksumFeed )
        iwd[194] = Com_BlockChecksumKey32((const unsigned __int8 *)fs_headerLongs, 4 * fs_numHeaderLongs, fs_checksumFeed);
    else
        iwd[194] = iwd[193];
    iwd[193] = iwd[193];
    iwd[194] = iwd[194];
    Z_Free((char *)fs_headerLongs, 3);
    iwd[200] = (unsigned int)buildBuffer;
    return (iwd_t *)iwd;
}

iwd_t *__cdecl FS_LoadFakeIwdFile(char *fakeiwdfile, const char *basename)
{
    unsigned int v3; // [esp+0h] [ebp-3Ch]
    unsigned int len; // [esp+24h] [ebp-18h]
    int *fs_headerLongs; // [esp+2Ch] [ebp-10h]
    unsigned int *buf; // [esp+30h] [ebp-Ch]
    unsigned int *iwd; // [esp+34h] [ebp-8h]
    _iobuf *f; // [esp+38h] [ebp-4h]

    buf = 0;
    f = FS_FileOpenReadBinary(fakeiwdfile);
    iwd = 0;
    len = FS_FileGetFileSize(f);
    if ( len >= 8 )
    {
        buf = Z_Malloc(len, "FS_LoadFakeIwdFile1", 3);
        if ( FS_FileRead(buf, len, f) == len && (int)*buf >= (int)buf[1] )
        {
            fs_headerLongs = (int *)(buf + 2);
            iwd = Z_Malloc(804, "FS_LoadFakeIwdFile2", 3);
            I_strncpyz((char *)iwd, fakeiwdfile, 256);
            I_strncpyz((char *)iwd + 256, basename, 256);
            v3 = strlen(basename);
            if ( v3 > 4 && !I_stricmp((const char *)iwd + v3 + 252, ".iwd") )
                *((_BYTE *)iwd + v3 + 252) = 0;
            iwd[192] = 0;
            iwd[196] = 0;
            iwd[195] = 0;
            *((_BYTE *)iwd + 788) = 0;
            iwd[198] = 0;
            iwd[193] = Com_BlockChecksumKey32((const unsigned __int8 *)fs_headerLongs, 4 * buf[1], 0);
            if ( fs_checksumFeed )
                iwd[194] = Com_BlockChecksumKey32((const unsigned __int8 *)fs_headerLongs, 4 * buf[1], fs_checksumFeed);
            else
                iwd[194] = iwd[193];
            iwd[193] = iwd[193];
            iwd[194] = iwd[194];
        }
    }
    Z_Free((char *)buf, 3);
    if ( f )
        FS_FileClose(f);
    return (iwd_t *)iwd;
}

int iString;
char *__cdecl IwdFileLanguage(const char *pszIwdFileName)
{
    int iCurrChar; // [esp+10h] [ebp-4h]

    iString ^= 1u;
    if ( strlen(pszIwdFileName) >= 0xA )
    {
        iCurrChar = 10;
        memset((unsigned __int8 *)szIwdLanguageName[iString], 0, sizeof(char[64]));
        while ( iCurrChar < 64 && pszIwdFileName[iCurrChar] && isalpha(pszIwdFileName[iCurrChar]) )
        {
            //*(_BYTE *)((iString << 6) + iCurrChar + 161332270) = pszIwdFileName[iCurrChar];
            //*((_BYTE *)&fs_numServerIwds + 64 * iString + iCurrChar + 2) = pszIwdFileName[iCurrChar];
            szIwdLanguageName[iString][iCurrChar - 10] = pszIwdFileName[iCurrChar];
            ++iCurrChar;
        }
    }
    else
    {
        szIwdLanguageName[iString][0] = 0;
    }
    return szIwdLanguageName[iString];
}

int __cdecl iwdsort(const char **a, char **b)
{
    const char *pszLanguageB; // [esp+0h] [ebp-10h]
    const char *pszLanguageA; // [esp+4h] [ebp-Ch]
    char *bb; // [esp+8h] [ebp-8h]
    char *aa; // [esp+Ch] [ebp-4h]

    aa = (char *)*a;
    bb = *b;
    if (!I_strncmp(*a, TEN_SPACE, 10) && !I_strncmp(bb, TEN_SPACE, 10))
    {
        pszLanguageA = IwdFileLanguage(aa);
        pszLanguageB = IwdFileLanguage(bb);
        if (I_stricmp(pszLanguageA, "english"))
        {
            if (!I_stricmp(pszLanguageB, "english"))
                return 1;
        }
        else if (I_stricmp(pszLanguageB, "english"))
        {
            return -1;
        }
    }
    return FS_PathCmp(aa, bb);
}

void __cdecl FS_AddSearchPath(searchpath_s *search)
{
    searchpath_s **pSearch; // [esp+0h] [ebp-4h]

    pSearch = &fs_searchpaths;
    if ( search->bLocalized )
    {
        while ( *pSearch && !(*pSearch)->bLocalized )
            pSearch = (searchpath_s **)*pSearch;
    }
    search->next = *pSearch;
    *pSearch = search;
}

void __cdecl FS_ShutdownServerIwdNames()
{
    FS_ShutdownServerFileReferences(&fs_numServerIwds, fs_serverIwdNames);
}

void __cdecl FS_ShutdownServerFileReferences(int *numFiles, const char **fileNames)
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < *numFiles; ++i )
    {
        if ( fileNames[i] )
        {
            FreeString(fileNames[i], 11, SCRIPTINSTANCE_SERVER);
            fileNames[i] = 0;
        }
    }
    *numFiles = 0;
}

void __cdecl FS_ShutdownServerReferencedIwds()
{
    FS_ShutdownServerFileReferences(&fs_numServerReferencedIwds, fs_serverReferencedIwdNames);
}

void __cdecl FS_ShutdownServerReferencedFFs()
{
    FS_ShutdownServerFileReferences(&fs_numServerReferencedFFs, fs_serverReferencedFFNames);
}

void __cdecl FS_Shutdown()
{
    int i; // [esp+0h] [ebp-4h]

    SEH_Shutdown_StringEd();
    for (i = 1; i < 70; ++i)
    {
        if (fsh[i].fileSize)
            FS_FCloseFile(i);
    }
    FS_ShutdownSearchPaths(fs_searchpaths);
    fs_searchpaths = 0;
    FS_RemoveCommands();
}

void __cdecl FS_ShutdownSearchPaths(searchpath_s *p)
{
    searchpath_s *next; // [esp+0h] [ebp-4h]

    while ( p )
    {
        next = p->next;
        if ( p->iwd )
        {
            unzClose(p->iwd->handle);
            Z_Free((char *)p->iwd->buildBuffer, 3);
            Z_Free(p->iwd->iwdFilename, 3);
        }
        if ( p->dir )
            Z_Free(p->dir->path, 3);
        Z_Free((char *)p, 3);
        p = next;
    }
}

void __cdecl FS_Startup(const char *gameName, bool allow_devraw)
{
    const char *v3; // eax
    const char *v4; // eax

    Com_Printf(10, "----- FS_Startup -----\n");
    FS_RegisterDvars();
    if ( fs_usedevdir->current.enabled )
    {
        if ( *(_BYTE *)fs_basepath->current.integer )
            FS_AddDevGameDirs((char *)fs_basepath->current.integer, allow_devraw);
        if ( *(_BYTE *)fs_homepath->current.integer && I_stricmp(fs_basepath->current.string, fs_homepath->current.string) )
            FS_AddDevGameDirs((char *)fs_homepath->current.integer, allow_devraw);
        if ( *(_BYTE *)fs_cdpath->current.integer && I_stricmp(fs_basepath->current.string, fs_cdpath->current.string) )
            FS_AddDevGameDirs((char *)fs_cdpath->current.integer, allow_devraw);
    }
    if ( *(_BYTE *)fs_cdpath->current.integer && I_stricmp(fs_basepath->current.string, fs_cdpath->current.string) )
        FS_AddLocalizedGameDirectory((char *)fs_cdpath->current.integer, gameName);
    if ( *(_BYTE *)fs_basepath->current.integer )
        FS_AddLocalizedGameDirectory((char *)fs_basepath->current.integer, "players");
    if ( *(_BYTE *)fs_basepath->current.integer )
    {
        v3 = va("%s_shared", gameName);
        FS_AddLocalizedGameDirectory((char *)fs_basepath->current.integer, v3);
        FS_AddLocalizedGameDirectory((char *)fs_basepath->current.integer, gameName);
    }
    if ( *(_BYTE *)fs_basepath->current.integer && I_stricmp(fs_homepath->current.string, fs_basepath->current.string) )
    {
        v4 = va("%s_shared", gameName);
        FS_AddLocalizedGameDirectory((char *)fs_basepath->current.integer, v4);
        FS_AddLocalizedGameDirectory((char *)fs_homepath->current.integer, gameName);
    }
    if ( *(_BYTE *)fs_basegame->current.integer
        && !I_stricmp(gameName, "main")
        && I_stricmp(fs_basegame->current.string, gameName)
        && *(_BYTE *)fs_basepath->current.integer )
    {
        FS_AddGameDirectory((char *)fs_basepath->current.integer, fs_basegame->current.string, 0, 0);
    }
    if ( *(_BYTE *)fs_gameDirVar->current.integer
        && !I_stricmp(gameName, "main")
        && I_stricmp(fs_gameDirVar->current.string, gameName)
        && *(_BYTE *)fs_basepath->current.integer )
    {
        FS_AddGameDirectory((char *)fs_basepath->current.integer, "usermaps", 0, 0);
        FS_AddGameDirectory((char *)fs_basepath->current.integer, fs_gameDirVar->current.string, 0, 0);
    }
    Com_ReadCDKey();
    //BLOPS_NULLSUB();
    FS_AddCommands();
    FS_Path_f();
    Dvar_ClearModified(fs_gameDirVar);
    Com_Printf(10, "----------------------\n");
    Com_Printf(10, "%d files in iwd files\n", fs_iwdFileCount);
}

void __cdecl FS_AddGameDirectory(char *path, const char *dir, int bLanguageDirectory, int iLanguage)
{
    unsigned int *v4; // eax
    int v5; // eax
    bool v6; // [esp+0h] [ebp-170h]
    const char *v7; // [esp+14h] [ebp-15Ch]
    char ospath[260]; // [esp+18h] [ebp-158h] BYREF
    const char *pszLanguage; // [esp+11Ch] [ebp-54h]
    char szGameFolder[68]; // [esp+120h] [ebp-50h] BYREF
    searchpath_s *search; // [esp+168h] [ebp-8h]
    searchpath_s *i; // [esp+16Ch] [ebp-4h]

    if ( bLanguageDirectory )
    {
        pszLanguage = SEH_GetLanguageName(iLanguage);
        Com_sprintf(szGameFolder, 0x40u, "%s/%s", dir, pszLanguage);
    }
    else
    {
        I_strncpyz(szGameFolder, dir, 64);
    }
    for ( i = fs_searchpaths; i; i = i->next )
    {
        if ( i->dir && !I_stricmp(i->dir->path, path) && !I_stricmp(i->dir->gamedir, szGameFolder) )
        {
            if ( i->bLocalized != bLanguageDirectory )
            {
                if ( i->bLocalized )
                    v7 = "localized";
                else
                    v7 = "non-localized";
                Com_PrintWarning(
                    10,
                    "WARNING: game folder %s/%s added as both localized & non-localized. Using folder as %s\n",
                    path,
                    szGameFolder,
                    v7);
            }
            if ( i->bLocalized )
            {
                if ( i->language != iLanguage )
                    Com_PrintWarning(
                        10,
                        "WARNING: game folder %s/%s re-added as localized folder with different language\n",
                        path,
                        szGameFolder);
            }
            return;
        }
    }
    if ( bLanguageDirectory )
    {
        FS_BuildOSPath(path, szGameFolder, (char *)"", ospath);
        ospath[&ospath[strlen(ospath) + 1] - &ospath[1] - 1] = 0;
        if ( !Sys_DirectoryHasContents(ospath) )
            return;
    }
    else
    {
        I_strncpyz(fs_gamedir, szGameFolder, 256);
    }
    search = (searchpath_s *)Z_Malloc(28, "FS_AddGameDirectory", 3);
    v4 = Z_Malloc(512, "FS_AddGameDirectory", 3);
    search->dir = (directory_t *)v4;
    I_strncpyz(search->dir->path, path, 256);
    I_strncpyz(search->dir->gamedir, szGameFolder, 256);
    if ( !bLanguageDirectory
        && iLanguage
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_files.cpp",
                    4992,
                    0,
                    "%s",
                    "bLanguageDirectory || (!bLanguageDirectory && !iLanguage)") )
    {
        __debugbreak();
    }
    search->bLocalized = bLanguageDirectory;
    search->language = iLanguage;
    if ( fs_usedevdir->current.enabled )
    {
        search->ignore = 0;
        search->ignorePureCheck = 1;
    }
    else
    {
        v5 = I_stricmp(dir, "main");
        search->ignore = v5 == 0;
        v6 = !I_stricmp(dir, "players") || !I_stricmp(dir, "demos");
        search->ignorePureCheck = v6;
    }
    FS_AddSearchPath(search);
    FS_AddIwdFilesForGameDirectory(path, szGameFolder);
}

void __cdecl FS_AddLocalizedGameDirectory(char *path, const char *dir)
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 12; i >= 0; --i )
        FS_AddGameDirectory(path, dir, 1, i);
    FS_AddGameDirectory(path, dir, 0, 0);
}

void FS_RegisterDvars()
{
    char *v1; // eax
    char *v2; // eax
    const dvar_s *result; // eax
    char *homePath; // [esp+0h] [ebp-4h]

    fs_debug = _Dvar_RegisterInt("fs_debug", 0, 0, 2, 0, "Enable file system debugging information");
    fs_copyfiles = _Dvar_RegisterBool("fs_copyfiles", 0, 0x10u, "Copy all used files to another location");
    v1 = (char *)Sys_DefaultCDPath();
    fs_cdpath = _Dvar_RegisterString("fs_cdpath", v1, 0x10u, "CD path");
    v2 = Sys_Cwd();
    fs_basepath = _Dvar_RegisterString("fs_b", v2, 0x210u, "Base game path");
    fs_basegame = _Dvar_RegisterString("fs_basegame", (char *)"", 0x10u, "Base game name");
    fs_gameDirVar = _Dvar_RegisterString(
                                        "fs_game",
                                        (char *)"",
                                        0x114u,
                                        "Game data directory. Must be \"\" or a sub directory of 'mods/'.");
    fs_usermapDir = _Dvar_RegisterString("fs_usermapdir", (char *)"", 0x144u, "Usermap data directory.");
    fs_ignoreLocalized = _Dvar_RegisterBool("fs_ignoreLocalized", 0, 0xA0u, "Ignore localized files");
    homePath = (char *)RETURN_ZERO32();
    if ( !homePath || !*homePath )
        homePath = (char *)fs_basepath->reset.integer;
    fs_homepath = _Dvar_RegisterString("fs_h", homePath, 0x210u, "Game home path");
    FS_GetOsFolderPath(5, ospathPersonalDocuments);
    fs_userDocuments = _Dvar_RegisterString(
                                             "fs_userDocuments",
                                             ospathPersonalDocuments,
                                             0x10u,
                                             "user documents path(screenshots).");
    fs_restrict = _Dvar_RegisterBool("fs_restrict", 0, 0x10u, "Restrict file access for demos etc.");
    fs_usedevdir = _Dvar_RegisterBool("fs_usedevdir", 1, 0x10u, "Use development directories.");
}

void __cdecl FS_AddDevGameDirs(char *path, bool allow_devraw)
{
    unsigned int j; // [esp+0h] [ebp-38h]
    unsigned int i; // [esp+4h] [ebp-34h]
    char devDir[32]; // [esp+8h] [ebp-30h] BYREF
    const char *devDirsOrder[3]; // [esp+2Ch] [ebp-Ch]

    devDirsOrder[0] = "raw_shared";
    devDirsOrder[1] = "raw";
    devDirsOrder[2] = "discdata";
    if ( (!fs_usedevdir->current.enabled || !*path)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_files.cpp",
                    5325,
                    0,
                    "%s",
                    "fs_usedevdir->current.enabled && *path") )
    {
        __debugbreak();
    }
    if ( allow_devraw )
    {
        strcpy(devDir, "dev");
        memset(&devDir[4], 0, 28);
        for ( i = 0; i < 3; ++i )
        {
            I_strncpyz(&devDir[3], devDirsOrder[i], 29);
            FS_AddLocalizedGameDirectory(path, devDir);
        }
    }
    for ( j = 0; j < 3; ++j )
        FS_AddLocalizedGameDirectory(path, devDirsOrder[j]);
}

void __cdecl FS_ClearIwdReferences()
{
    searchpath_s *search; // [esp+0h] [ebp-4h]

    for ( search = fs_searchpaths; search; search = search->next )
    {
        if ( search->iwd )
            search->iwd->referenced = 0;
    }
}

void __cdecl FS_AddIwdPureCheckReference(const searchpath_s *search)
{
    iwd_pure_check_s *pureCheckInfo; // [esp+0h] [ebp-8h]
    iwd_pure_check_s *p; // [esp+4h] [ebp-4h]
    iwd_pure_check_s *pa; // [esp+4h] [ebp-4h]

    if ( !search->iwd
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_files.cpp", 5487, 0, "%s", "search->iwd") )
    {
        __debugbreak();
    }
    if ( !search->bLocalized )
    {
        for ( p = fs_iwdPureChecks; p; p = p->next )
        {
            if ( p->checksum == search->iwd->checksum && !I_stricmp(p->iwdBasename, search->iwd->iwdBasename) )
                return;
        }
        pureCheckInfo = (iwd_pure_check_s *)Z_Malloc(520, "FS_AddIwdPureCheckReference", 3);
        pureCheckInfo->next = 0;
        pureCheckInfo->checksum = search->iwd->checksum;
        I_strncpyz(pureCheckInfo->iwdBasename, search->iwd->iwdBasename, 256);
        I_strncpyz(pureCheckInfo->iwdGamename, search->iwd->iwdGamename, 256);
        if ( fs_iwdPureChecks )
        {
            for ( pa = fs_iwdPureChecks; pa; pa = pa->next )
            {
                if ( !pa->next )
                {
                    pa->next = pureCheckInfo;
                    return;
                }
            }
        }
        else
        {
            fs_iwdPureChecks = pureCheckInfo;
        }
    }
}

void __cdecl FS_ShutDownIwdPureCheckReferences()
{
    iwd_pure_check_s *next; // [esp+0h] [ebp-8h]
    iwd_pure_check_s *p; // [esp+4h] [ebp-4h]

    for ( p = fs_iwdPureChecks; p; p = next )
    {
        next = p->next;
        Z_Free((char *)p, 3);
    }
    fs_iwdPureChecks = 0;
}

bool __cdecl FS_IsBasePathValid()
{
    return FS_ReadFile("fileSysCheck.cfg", 0) > 0;
}

void __cdecl FS_InitFilesystem(bool allow_devraw)
{
    Com_StartupVariable("fs_cdpath");
    Com_StartupVariable("fs_b");
    Com_StartupVariable("fs_h");
    Com_StartupVariable("fs_game");
    Com_StartupVariable("fs_copyfiles");
    Com_StartupVariable("fs_restrict");
    Com_StartupVariable("loc_language");
    SEH_InitLanguage();
    FS_Startup("main", allow_devraw);
    SEH_Init_StringEd();
    SEH_UpdateLanguageInfo();
    FS_SetRestrictions();
    if ( !FS_IsBasePathValid() )
        Com_Error(
            ERR_FATAL,
            "Couldn't load %s.    Make sure Call of Duty is run from the correct folder.",
            "fileSysCheck.cfg");
    I_strncpyz(lastValidBase, fs_basepath->current.string, 256);
    I_strncpyz(lastValidGame, fs_gameDirVar->current.string, 256);
}

void __cdecl FS_Restart(int localClientNum, int checksumFeed)
{
    const char *v2; // eax
    bool defaultConfigLoaded; // [esp+Bh] [ebp-1h]

    if (!IsDedicatedServer())
    {
        SND_StopSounds(SND_EVERY_SINGLE_ONE_DONT_ASK_ANY_QUESTIONS);
        NET_Sleep(1000);
        SND_StreamCloseFiles();
    }

    FS_Shutdown();
    fs_checksumFeed = checksumFeed;
    FS_ClearIwdReferences();
    FS_ShutDownIwdPureCheckReferences();
    ProfLoad_Begin("Start file system");
    FS_Startup("main", 1);
    ProfLoad_End();
    ProfLoad_Begin("Init text localization");
    SEH_Init_StringEd();
    SEH_UpdateLanguageInfo();
    ProfLoad_End();
    ProfLoad_Begin("Set restrictions");
    FS_SetRestrictions();
    ProfLoad_End();
    ProfLoad_Begin("Default config");
    if ( useFastFile->current.enabled )
    {
        DB_FindXAssetHeader(ASSET_TYPE_RAWFILE, (char*)DEFAULT_CFG, 1, -1);
        defaultConfigLoaded = !DB_IsXAssetDefault(ASSET_TYPE_RAWFILE, DEFAULT_CFG);
    }
    else
    {
        defaultConfigLoaded = FS_ReadFile(DEFAULT_CFG, 0) > 0;
    }
    if ( !defaultConfigLoaded )
    {
        if ( lastValidBase[0] )
        {
            FS_PureServerSetLoadedIwds((char *)"", (char *)"");
            Dvar_SetString((dvar_s *)fs_basepath, lastValidBase);
            Dvar_SetString((dvar_s *)fs_gameDirVar, lastValidGame);
            lastValidBase[0] = 0;
            lastValidGame[0] = 0;
            Dvar_SetBool((dvar_s *)fs_restrict, 0);
            FS_Restart(localClientNum, checksumFeed);
            Com_Error(ERR_DROP, "Invalid game folder\n");
        }
        Com_Error(
            ERR_FATAL,
            "Couldn't load %s.    Make sure Call of Duty is run from the correct folder.",
            DEFAULT_CFG);
    }
    if ( I_stricmp(fs_gameDirVar->current.string, lastValidGame) && !Com_SafeMode() )
    {
        v2 = va("exec %s\n", "config_mp.cfg");
        Cbuf_AddText(0, v2);
    }
    I_strncpyz(lastValidBase, fs_basepath->current.string, 256);
    I_strncpyz(lastValidGame, fs_gameDirVar->current.string, 256);
    ProfLoad_End();
}

bool __cdecl FS_NeedRestart(int checksumFeed)
{
    if ( com_sv_running->current.enabled )
        return 0;
    if ( fs_gameDirVar->modified )
        return 1;
    return checksumFeed != fs_checksumFeed;
}

unsigned int __cdecl FS_FOpenFileByMode(char *qpath, int *f, fsMode_t mode)
{
    unsigned int r; // [esp+4h] [ebp-8h]
    int sync; // [esp+8h] [ebp-4h]

    r = 6969;
    sync = 0;
    switch (mode)
    {
    case FS_READ:
        r = FS_FOpenFileRead(qpath, f);
        break;
    case FS_WRITE:
        *f = FS_FOpenFileWrite(qpath);
        r = 0;
        if (!*f)
            r = -1;
        break;
    case FS_APPEND:
        goto $LN5_118;
    case FS_APPEND_SYNC:
        sync = 1;
    $LN5_118:
        *f = FS_FOpenFileAppend(qpath);
        r = 0;
        if (!*f)
            r = -1;
        break;
    default:
        Com_Error(ERR_FATAL, "FSH_FOpenFile: bad mode");
        break;
    }
    if (!f)
        return r;
    if (*f)
    {
        fsh[*f].fileSize = r;
        fsh[*f].streamed = 0;
    }
    fsh[*f].handleSync = sync;
    return r;
}

unsigned int __cdecl FS_FTell(int f)
{
    if (fsh[f].zipFile)
        return unztell(fsh[f].handleFiles.file.z);
    return ftell(FS_FileForHandle(f));
}

void __cdecl FS_Flush(int f)
{
    _iobuf *v1; // eax

    v1 = FS_FileForHandle(f);
    fflush(v1);
}

void __cdecl Com_GetBspFilename(char *filename, unsigned int size, const char *mapname)
{
    Com_sprintf(filename, size, "maps/mp/%s.d3dbsp", mapname);
}

