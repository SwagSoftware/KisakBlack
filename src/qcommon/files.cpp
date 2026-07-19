#include "files.h"
#include <cstdio>
#include <universal/com_files.h>
#include <universal/com_fileaccess.h>
#include "common.h"

#include <cstring>
#include "cmd.h"
#include <database/db_registry.h>
#include <clientscript/cscr_main.h>
#include <universal/com_memory.h>
#include <sound/snd_public_async.h>

int __cdecl FS_SV_FileExists(char *file, char *dir)
{
    FILE *f; // [esp+4h] [ebp-10Ch]
    char testpath[260]; // [esp+8h] [ebp-108h] BYREF

    FS_BuildOSPath((char *)fs_homepath->current.integer, dir, file, testpath);
    f = FS_FileOpenReadBinary(testpath);
    if ( !f )
        return 0;
    FS_FileClose(f);
    return 1;
}

int __cdecl FS_SV_FOpenFileWrite(char *filename, char *dir)
{
    char ospath[260]; // [esp+0h] [ebp-110h] BYREF
    const char *basepath; // [esp+108h] [ebp-8h]
    int f; // [esp+10Ch] [ebp-4h]

    f = 0;
    FS_CheckFileSystemStarted();
    if ( fs_debug->current.integer )
        Com_Printf(10, "FS_SV_FOpenFileWrite: opening %s\n", filename);
    basepath = fs_homepath->current.string;
    f = FS_FOpenFileWriteCurrentThread(filename, dir, basepath);
    if ( !f && fs_debug->current.integer )
        Com_Printf(10, "FS_SV_FOpenFileWrite: failed to open %s\n", ospath);
    return f;
}

int __cdecl FS_SV_FOpenFileRead(char *filename, char *dir, int *fp)
{
    _iobuf *Binary; // eax
    _iobuf *v5; // eax
    _iobuf *v6; // eax
    char ospath[260]; // [esp+0h] [ebp-110h] BYREF
    const char *basepath; // [esp+108h] [ebp-8h]
    int f; // [esp+10Ch] [ebp-4h]

    f = 0;
    FS_CheckFileSystemStarted();
    if (fs_debug->current.integer)
        Com_Printf(10, "FS_SV_FOpenFileRead: opening %s\n", filename);
    basepath = fs_homepath->current.string;
    FS_BuildOSPath((char *)basepath, dir, filename, ospath);
    f = FS_HandleForFileCurrentThread(filename);
    if (!f)
        return 0;
    fsh[f].zipFile = 0;
    fsh[f].handleSync = 0;
    Binary = FS_FileOpenReadBinary(ospath);
    fsh[f].handleFiles.file.o = Binary;
    if (!fsh[f].handleFiles.file.o && I_stricmp(fs_homepath->current.string, fs_basepath->current.string))
    {
        FS_BuildOSPath((char *)fs_basepath->current.integer, dir, filename, ospath);
        if (fs_debug->current.integer)
            Com_Printf(10, "FS_SV_FOpenFileRead (fs_basepath): %s\n", ospath);
        v5 = FS_FileOpenReadBinary(ospath);
        fsh[f].handleFiles.file.o = v5;
    }
    if (!fsh[f].handleFiles.file.o)
    {
        FS_BuildOSPath((char *)fs_cdpath->current.integer, dir, filename, ospath);
        if (fs_debug->current.integer)
            Com_Printf(10, "FS_SV_FOpenFileRead (fs_cdpath) : %s\n", ospath);
        v6 = FS_FileOpenReadBinary(ospath);
        fsh[f].handleFiles.file.o = v6;
    }
    if (fsh[f].handleFiles.file.o)
    {
        I_strncpyz(fsh[f].name, filename, 256);
        *fp = f;
        return FS_filelength(f);
    }
    else
    {
        if (fs_debug->current.integer)
            Com_Printf(10, "FS_SV_FOpenFileRead: failed to open %s\n", filename);
        *fp = 0;
        return 0;
    }
}

void __cdecl FS_SV_Rename(char *from, char *to, char *dir)
{
    char to_ospath[260]; // [esp+0h] [ebp-210h] BYREF
    const char *basepath; // [esp+104h] [ebp-10Ch]
    char from_ospath[260]; // [esp+108h] [ebp-108h] BYREF

    FS_CheckFileSystemStarted();
    basepath = fs_homepath->current.string;
    FS_BuildOSPath((char *)basepath, dir, from, from_ospath);
    FS_BuildOSPath((char *)basepath, dir, to, to_ospath);
    if ( fs_debug->current.integer )
        Com_Printf(10, "FS_SV_Rename: %s --> %s\n", from_ospath, to_ospath);
    if ( rename(from_ospath, to_ospath) )
    {
        FS_CopyFile(from_ospath, to_ospath);
        FS_Remove(from_ospath);
    }
}

int __cdecl FS_GetModList(char *listbuf, int bufsize)
{
    char v3; // [esp+3h] [ebp-181h]
    char *v4; // [esp+8h] [ebp-17Ch]
    char *v5; // [esp+Ch] [ebp-178h]
    char v6; // [esp+13h] [ebp-171h]
    char *v7; // [esp+18h] [ebp-16Ch]
    char *v8; // [esp+1Ch] [ebp-168h]
    char v9; // [esp+33h] [ebp-151h]
    char *v10; // [esp+38h] [ebp-14Ch]
    char *v11; // [esp+3Ch] [ebp-148h]
    FILE *file; // [esp+50h] [ebp-134h]
    int nMods; // [esp+54h] [ebp-130h]
    int nDescLen; // [esp+58h] [ebp-12Ch]
    int nDescLena; // [esp+58h] [ebp-12Ch]
    int descHandle; // [esp+5Ch] [ebp-128h] BYREF
    const char *basepath; // [esp+60h] [ebp-124h]
    int dummy; // [esp+64h] [ebp-120h] BYREF
    char *name; // [esp+68h] [ebp-11Ch]
    char descPath[256]; // [esp+6Ch] [ebp-118h] BYREF
    int nPotential; // [esp+170h] [ebp-14h]
    int nLen; // [esp+174h] [ebp-10h]
    int nTotal; // [esp+178h] [ebp-Ch]
    int i; // [esp+17Ch] [ebp-8h]
    char **pFiles; // [esp+180h] [ebp-4h]
    char *listbufa; // [esp+18Ch] [ebp+8h]

    pFiles = 0;
    *listbuf = 0;
    nTotal = 0;
    nPotential = 0;
    nMods = 0;
    basepath = fs_homepath->current.string;
    Com_sprintf(descPath, 0x100u, "%s/%s", basepath, "mods");
    pFiles = (char **)Sys_ListFiles(descPath, 0, 0, &dummy, 1);
    nPotential = Sys_CountFileList(pFiles);
    for ( i = 0; i < nPotential; ++i )
    {
        name = pFiles[i];
        nLen = strlen(name) + 1;
        v11 = name;
        v10 = descPath;
        do
        {
            v9 = *v11;
            *v10++ = *v11++;
        }
        while ( v9 );
        I_strncat(descPath, 256, "/description.txt");
        if ( FS_SV_FOpenFileRead(descPath, (char*)"mods", &descHandle) > 0 && descHandle )
        {
            file = FS_FileForHandle(descHandle);
            Com_Memset((unsigned int *)descPath, 0, 256);
            nDescLen = FS_FileRead(descPath, 0x30u, file);
            if ( nDescLen >= 0 )
                descPath[nDescLen] = 0;
            FS_FCloseFile(descHandle);
        }
        else
        {
            Com_Printf(10, "FS_GetModList: failed to open %s\n", descPath);
            descPath[0] = 0;
        }
        nDescLena = &descPath[strlen(descPath) + 1] - &descPath[1] + 1;
        if ( nLen + nTotal + nDescLena + 2 >= bufsize )
            break;
        v8 = name;
        v7 = listbuf;
        do
        {
            v6 = *v8;
            *v7++ = *v8++;
        }
        while ( v6 );
        listbufa = &listbuf[nLen];
        v5 = descPath;
        v4 = listbufa;
        do
        {
            v3 = *v5;
            *v4++ = *v5++;
        }
        while ( v3 );
        listbuf = &listbufa[nDescLena];
        nTotal += nDescLena + nLen;
        ++nMods;
    }
    FS_FreeFileList((const char **)pFiles);
    return nMods;
}

int __cdecl Sys_CountFileList(char **list)
{
    int i; // [esp+0h] [ebp-4h]

    i = 0;
    if ( list )
    {
        while ( *list )
        {
            ++list;
            ++i;
        }
    }
    return i;
}

void __cdecl FS_Dir_f()
{
    const char *path; // [esp+30h] [ebp-14h]
    char *extension; // [esp+34h] [ebp-10h]
    int ndirs; // [esp+38h] [ebp-Ch] BYREF
    int i; // [esp+3Ch] [ebp-8h]
    const char **dirnames; // [esp+40h] [ebp-4h]

    if ( Cmd_Argc() >= 2 && Cmd_Argc() <= 3 )
    {
        if ( Cmd_Argc() == 2 )
        {
            path = Cmd_Argv(1);
            extension = (char *)"";
            Com_Printf(0, "Directory of %s %s\n", path, "");
        }
        else
        {
            path = Cmd_Argv(1);
            extension = (char *)Cmd_Argv(2);
            Com_Printf(0, "Directory of %s %s\n", path, extension);
        }
        Com_Printf(0, "---------------\n");
        dirnames = FS_ListFiles(path, extension, FS_LIST_PURE_ONLY, &ndirs);
        for ( i = 0; i < ndirs; ++i )
            Com_Printf(0, "%s\n", dirnames[i]);
        FS_FreeFileList(dirnames);
    }
    else
    {
        Com_Printf(0, "usage: dir <directory> [extension]\n");
    }
}

void __cdecl FS_NewDir_f()
{
    int ndirs; // [esp+10h] [ebp-10h] BYREF
    int i; // [esp+14h] [ebp-Ch]
    const char *filter; // [esp+18h] [ebp-8h]
    const char **dirnames; // [esp+1Ch] [ebp-4h]

    if ( Cmd_Argc() >= 2 )
    {
        filter = Cmd_Argv(1);
        Com_Printf(0, "---------------\n");
        dirnames = FS_ListFilteredFiles(
                                 fs_searchpaths,
                                 "",
                                 (char *)"",
                                 (char *)filter,
                                 FS_LIST_PURE_ONLY,
                                 &ndirs);
        FS_SortFileList(dirnames, ndirs);
        for ( i = 0; i < ndirs; ++i )
        {
            FS_ConvertPath((char *)dirnames[i]);
            Com_Printf(0, "%s\n", dirnames[i]);
        }
        Com_Printf(0, "%d files listed\n", ndirs);
        FS_FreeFileList(dirnames);
    }
    else
    {
        Com_Printf(0, "usage: fdir <filter>\n");
        Com_Printf(0, "example: fdir *q3dm*.bsp\n");
    }
}

void __cdecl FS_TouchFile_f()
{
    const char *v0; // eax

    if ( Cmd_Argc() == 2 )
    {
        v0 = Cmd_Argv(1);
        FS_TouchFile(v0);
    }
    else
    {
        Com_Printf(0, "Usage: touchFile <file>\n");
    }
}

int __cdecl FS_iwIwd(char *iwd, char *base)
{
    const char *v2; // eax
    const char *v4; // eax
    const char *v5; // eax
    char *v6; // eax
    char v7; // [esp+3h] [ebp-71h]
    char *v8; // [esp+8h] [ebp-6Ch]
    const char *v9; // [esp+Ch] [ebp-68h]
    char v10; // [esp+13h] [ebp-61h]
    char *v11; // [esp+18h] [ebp-5Ch]
    char *v12; // [esp+1Ch] [ebp-58h]
    char *str2; // [esp+20h] [ebp-54h]
    char szFile[68]; // [esp+24h] [ebp-50h] BYREF
    const char *pszLoc; // [esp+6Ch] [ebp-8h]
    int i; // [esp+70h] [ebp-4h]

    for ( i = 0; i < 28; ++i )
    {
        v2 = va("%s/iw_%02d", base, i);
        if ( !FS_FilenameCompare(iwd, v2) )
            return 1;
    }
    v4 = strstr(iwd, "localized_");
    pszLoc = v4;
    if ( v4 )
    {
        v12 = iwd;
        v11 = szFile;
        do
        {
            v10 = *v12;
            *v11++ = *v12++;
        }
        while ( v10 );
        szFile[pszLoc - iwd + 10] = 0;
        v5 = va("%s/localized_", base);
        if ( !FS_FilenameCompare(szFile, v5) )
        {
            v9 = pszLoc + 10;
            v8 = szFile;
            do
            {
                v7 = *v9;
                *v8++ = *v9++;
            }
            while ( v7 );
            I_strlwr(szFile);
            for ( i = 0; i < 28; ++i )
            {
                str2 = va("_iw%02d", i);
                v6 = strstr(szFile, str2);
                if ( v6 )
                    return 1;
            }
        }
    }
    return 0;
}

bool __cdecl FS_serverPak(const char *pak)
{
    char *v1; // eax
    char v3; // [esp+3h] [ebp-55h]
    char *v4; // [esp+8h] [ebp-50h]
    char szFile[68]; // [esp+10h] [ebp-48h] BYREF

    v4 = szFile;
    do
    {
        v3 = *pak;
        *v4++ = *pak++;
    }
    while ( v3 );
    I_strlwr(szFile);
    v1 = strstr(szFile, "_svr_");
    return v1 != 0;
}

FS_SERVER_COMPARE_RESULT __cdecl FS_CompareWithServerFiles(char *neededFiles, int len, int dlstring)
{
    char buffer[1024]; // [esp+10h] [ebp-410h] BYREF
    FS_SERVER_COMPARE_RESULT iwdCompareResult; // [esp+414h] [ebp-Ch]
    int neededIWDStrLen; // [esp+418h] [ebp-8h]
    FS_SERVER_COMPARE_RESULT ffCompareResult; // [esp+41Ch] [ebp-4h]

    *neededFiles = 0;
    iwdCompareResult = FS_CompareIwds(neededFiles, len, dlstring);
    if ( iwdCompareResult == NOT_DOWNLOADABLE )
        return NOT_DOWNLOADABLE;
    neededIWDStrLen = strlen(neededFiles);
    if ( len < neededIWDStrLen
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\files.cpp",
                    865,
                    0,
                    "%s",
                    "len >= neededIWDStrLen") )
    {
        __debugbreak();
    }
    ffCompareResult = FS_CompareFFs(&neededFiles[neededIWDStrLen], len - neededIWDStrLen, dlstring);
    if ( ffCompareResult != NOT_DOWNLOADABLE )
        return (FS_SERVER_COMPARE_RESULT)(iwdCompareResult == NEED_DOWNLOAD || ffCompareResult == NEED_DOWNLOAD);
    if ( neededIWDStrLen > 0 )
    {
        if ( len > 1024
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\files.cpp",
                        873,
                        0,
                        "%s",
                        "MAX_STRING_CHARS >= len") )
        {
            __debugbreak();
        }
        I_strncpyz(buffer, &neededFiles[neededIWDStrLen], len);
        I_strncpyz(neededFiles, buffer, len);
    }
    return NOT_DOWNLOADABLE;
}

FS_SERVER_COMPARE_RESULT __cdecl FS_CompareIwds(char *needediwds, int len, int dlstring)
{
    int v4; // eax
    const char *v5; // eax
    char *v6; // eax
    const char *v7; // [esp+8h] [ebp-130h]
    const char *string; // [esp+Ch] [ebp-12Ch]
    unsigned int v9; // [esp+Ch] [ebp-12Ch]
    const char *dirNameEnd; // [esp+14h] [ebp-124h]
    char dirName[256]; // [esp+18h] [ebp-120h] BYREF
    int gameDirNameLength; // [esp+11Ch] [ebp-1Ch]
    int isBaseDir; // [esp+120h] [ebp-18h]
    int isUserMapIWD; // [esp+124h] [ebp-14h]
    int userMapDirAdded; // [esp+128h] [ebp-10h]
    int haveiwd; // [esp+12Ch] [ebp-Ch]
    searchpath_s *j; // [esp+130h] [ebp-8h]
    int i; // [esp+134h] [ebp-4h]

    if ( !fs_numServerReferencedIwds )
        return FILES_MATCH;
    *needediwds = 0;
    userMapDirAdded = 0;
    string = fs_gameDirVar->current.string;
    v7 = string + 1;
    v9 = (unsigned int)&string[strlen(string) + 1];
    gameDirNameLength = v9 - (unsigned int)v7;
    isBaseDir = v9 == (unsigned int)v7;
    for ( i = 0; i < fs_numServerReferencedIwds; ++i )
    {
        haveiwd = 0;
        if ( isBaseDir || !FS_serverPak(fs_serverReferencedIwdNames[i]) )
        {
            v4 = -(I_strnicmp(fs_serverReferencedIwdNames[i], "usermaps", 8) != 0);
            isUserMapIWD = v4 + 1;
            if ( v4 != -1 && !userMapDirAdded )
            {
                v5 = strchr(fs_serverReferencedIwdNames[i] + 9, '/');
                dirNameEnd = v5;
                if ( !v5 )
                    Com_Error(ERR_DROP, "Invalid IWD info from server: %s", fs_serverReferencedIwdNames[i]);
                I_strncpyz(dirName, fs_serverReferencedIwdNames[i], dirNameEnd - fs_serverReferencedIwdNames[i] + 1);
                FS_AddUserMapDirIWDs(dirName);
                userMapDirAdded = 1;
            }
            for ( j = fs_searchpaths; j; j = j->next )
            {
                if ( j->iwd && j->iwd->checksum == fs_serverReferencedIwds[i] )
                {
                    haveiwd = 1;
                    break;
                }
            }
            if ( !haveiwd && fs_serverReferencedIwdNames[i] && *fs_serverReferencedIwdNames[i] )
            {
                if ( isBaseDir
                    || I_strnicmp(fs_serverReferencedIwdNames[i], fs_gameDirVar->current.string, gameDirNameLength)
                    && !isUserMapIWD
                    || FS_iwIwd((char *)fs_serverReferencedIwdNames[i], (char*)"main") )
                {
                    I_strncpyz(needediwds, fs_serverReferencedIwdNames[i], len);
                    I_strncat(needediwds, len, ".iwd");
                    return NOT_DOWNLOADABLE;
                }
                if ( dlstring )
                {
                    I_strncat(needediwds, len, "@");
                    I_strncat(needediwds, len, fs_serverReferencedIwdNames[i]);
                    I_strncat(needediwds, len, ".iwd");
                    I_strncat(needediwds, len, "@");
                    I_strncat(needediwds, len, fs_serverReferencedIwdNames[i]);
                    I_strncat(needediwds, len, ".iwd");
                }
                else
                {
                    I_strncat(needediwds, len, fs_serverReferencedIwdNames[i]);
                    I_strncat(needediwds, len, ".iwd");
                    v6 = va("%s.iwd", fs_serverReferencedIwdNames[i]);
                    if ( FS_SV_FileExists(v6, (char *)"") )
                        I_strncat(needediwds, len, " (local file exists with wrong checksum)");
                    I_strncat(needediwds, len, "\n");
                }
            }
        }
    }
    if ( !*needediwds )
        return FILES_MATCH;
    Com_Printf(10, "Need iwds: %s\n", needediwds);
    return NEED_DOWNLOAD;
}

FS_SERVER_COMPARE_RESULT __cdecl FS_CompareFFs(char *neededFFs, int len, int dlstring)
{
    bool v4; // [esp+10h] [ebp-16Ch]
    bool v5; // [esp+24h] [ebp-158h]
    unsigned int v6; // [esp+38h] [ebp-144h]
    char ffNameCopy[260]; // [esp+5Ch] [ebp-120h] BYREF
    int usermapDirNameLength; // [esp+164h] [ebp-18h]
    const char *ffName; // [esp+168h] [ebp-14h]
    int fileSize; // [esp+16Ch] [ebp-10h]
    int isSameFile; // [esp+170h] [ebp-Ch]
    int i; // [esp+174h] [ebp-8h]
    FF_DIR ffDir; // [esp+178h] [ebp-4h]

    if ( !fs_numServerReferencedFFs )
        return FILES_MATCH;
    *neededFFs = 0;
    v6 = strlen(fs_gameDirVar->current.string);
    usermapDirNameLength = strlen(fs_usermapDir->current.string);
    for ( i = 0; i < fs_numServerReferencedFFs; ++i )
    {
        ffDir = FFD_DEFAULT;
        ffName = fs_serverReferencedFFNames[i];
        fileSize = DB_FileSize(ffName, FFD_DEFAULT);
        if ( !fileSize )
        {
            v5 = fs_usermapDir && *(_BYTE *)fs_usermapDir->current.integer;
            if ( !v5 || I_strnicmp(fs_serverReferencedFFNames[i], "usermaps", 8) )
            {
                v4 = fs_gameDirVar && *(_BYTE *)fs_gameDirVar->current.integer;
                if ( !v4 || I_strnicmp(fs_serverReferencedFFNames[i], "mods", 4) )
                {
                    ffName = fs_serverReferencedFFNames[i];
                    ffDir = FFD_DEFAULT;
                }
                else if ( strlen(fs_serverReferencedFFNames[i]) > v6 + 1 )
                {
                    ffName = &fs_serverReferencedFFNames[i][v6 + 1];
                    ffDir = FFD_MOD_DIR;
                }
            }
            else if ( strlen(fs_serverReferencedFFNames[i]) > 9 )
            {
                ffName = fs_serverReferencedFFNames[i] + 9;
                ffDir = FFD_USER_MAP;
            }
            fileSize = DB_FileSize(ffName, ffDir);
        }
        isSameFile = fileSize == fs_serverReferencedFFCheckSums[i];
        if ( !isSameFile && fs_serverReferencedFFNames[i] && *fs_serverReferencedFFNames[i] )
        {
            if ( ffDir == FFD_DEFAULT )
            {
                I_strncpyz(neededFFs, fs_serverReferencedFFNames[i], len);
                I_strncat(neededFFs, len, ".ff");
                return NOT_DOWNLOADABLE;
            }
            I_strncpyz(ffNameCopy, fs_serverReferencedFFNames[i], 256);
            if ( dlstring )
            {
                I_strncat(neededFFs, len, "@");
                I_strncat(neededFFs, len, ffNameCopy);
                I_strncat(neededFFs, len, ".ff");
                I_strncat(neededFFs, len, "@");
                I_strncat(neededFFs, len, ffNameCopy);
                I_strncat(neededFFs, len, ".ff");
                if ( ffDir == FFD_USER_MAP )
                {
                    I_strncat(neededFFs, len, "@");
                    I_strncat(neededFFs, len, ffNameCopy);
                    I_strncat(neededFFs, len, "_load");
                    I_strncat(neededFFs, len, ".ff");
                    I_strncat(neededFFs, len, "@");
                    I_strncat(neededFFs, len, ffNameCopy);
                    I_strncat(neededFFs, len, "_load");
                    I_strncat(neededFFs, len, ".ff");
                }
            }
            else
            {
                I_strncat(neededFFs, len, ffNameCopy);
                I_strncat(neededFFs, len, ".ff");
                if ( fileSize )
                    I_strncat(neededFFs, len, " (local file exists with wrong filesize)");
                I_strncat(neededFFs, len, "\n");
            }
        }
    }
    if ( !*neededFFs )
        return FILES_MATCH;
    Com_Printf(10, "Need FFs: %s\n", neededFFs);
    return NEED_DOWNLOAD;
}

void __cdecl FS_RemoveCommands()
{
    Cmd_RemoveCommand("path");
    Cmd_RemoveCommand("fullpath");
    Cmd_RemoveCommand("dir");
    Cmd_RemoveCommand("fdir");
    Cmd_RemoveCommand("touchFile");
}

cmd_function_s FS_Path_f_VAR;
cmd_function_s FS_FullPath_f_VAR;
cmd_function_s FS_Dir_f_VAR;
cmd_function_s FS_NewDir_f_VAR;
cmd_function_s FS_TouchFile_f_VAR;
void __cdecl FS_AddCommands()
{
    Cmd_AddCommandInternal("path", FS_Path_f, &FS_Path_f_VAR);
    Cmd_AddCommandInternal("fullpath", FS_FullPath_f, &FS_FullPath_f_VAR);
    Cmd_AddCommandInternal("dir", FS_Dir_f, &FS_Dir_f_VAR);
    Cmd_AddCommandInternal("fdir", FS_NewDir_f, &FS_NewDir_f_VAR);
    Cmd_AddCommandInternal("touchFile", FS_TouchFile_f, &FS_TouchFile_f_VAR);
}

void __cdecl FS_SetRestrictions()
{
    searchpath_s *path; // [esp+0h] [ebp-4h]

    if ( fs_restrict->current.enabled )
    {
        Dvar_SetBool((dvar_s *)fs_restrict, 1);
        Com_Printf(10, "\nRunning in restricted demo mode.\n\n");
        FS_Shutdown();
        FS_Startup("demomain", 1);
        for ( path = fs_searchpaths; path; path = path->next )
        {
            if (FS_UseSearchPath(path) && path->iwd && (path->iwd->checksum ^ 0x2261994) != 0xB3D38C61) // (KISAKTODO:) kinda interesting magic
            {
                Com_Error(ERR_FATAL, "Corrupted iw0.iwd: %u", path->iwd->checksum);
            }
        }
    }
}

char info2_0[16384];
char info3[16384];
void __cdecl FS_LoadedIwds(const char **checksums, const char **names)
{
    const char *v2; // eax
    searchpath_s *search; // [esp+0h] [ebp-4h]

    info2_0[0] = 0;
    info3[0] = 0;
    for ( search = fs_searchpaths; search; search = search->next )
    {
        if ( search->iwd && !search->bLocalized )
        {
            v2 = va("%i ", search->iwd->checksum);
            I_strncat(info2_0, 0x4000, v2);
            if ( info3[0] )
                I_strncat(info3, 0x4000, " ");
            I_strncat(info3, 0x4000, search->iwd->iwdBasename);
        }
    }
    *checksums = info2_0;
    *names = info3;
}

char info4[16384];
char *__cdecl FS_LoadedIwdPureChecksums()
{
    const char *v0; // eax
    searchpath_s *search; // [esp+0h] [ebp-4h]

    info4[0] = 0;
    for ( search = fs_searchpaths; search; search = search->next )
    {
        if ( search->iwd )
        {
            if ( !search->bLocalized )
            {
                v0 = va("%i ", search->iwd->pure_checksum);
                I_strncat(info4, 0x4000, v0);
            }
        }
    }
    return info4;
}

char info5[16384];
char info8[16384];
void __cdecl FS_ReferencedIwds(const char **checksums, const char **names)
{
    const char *v2; // eax
    const char *v3; // eax
    iwd_pure_check_s *iwd; // [esp+0h] [ebp-8h]
    searchpath_s *search; // [esp+4h] [ebp-4h]

    info5[0] = 0;
    info8[0] = 0;
    for ( iwd = fs_iwdPureChecks; iwd; iwd = iwd->next )
    {
        v2 = va("%i ", iwd->checksum);
        I_strncat(info5, 0x4000, v2);
        if ( info8[0] )
            I_strncat(info8, 0x4000, " ");
        I_strncat(info8, 0x4000, iwd->iwdGamename);
        I_strncat(info8, 0x4000, "/");
        I_strncat(info8, 0x4000, iwd->iwdBasename);
    }
    if ( *(_BYTE *)fs_gameDirVar->current.integer )
    {
        for ( search = fs_searchpaths; search; search = search->next )
        {
            if ( search->iwd
                && !search->bLocalized
                && !search->iwd->referenced
                && (!I_stricmp(search->iwd->iwdGamename, fs_gameDirVar->current.string)
                 || !I_strnicmp(search->iwd->iwdGamename, "usermaps", 8)) )
            {
                v3 = va("%i ", search->iwd->checksum);
                I_strncat(info5, 0x4000, v3);
                if ( info8[0] )
                    I_strncat(info8, 0x4000, " ");
                I_strncat(info8, 0x4000, search->iwd->iwdGamename);
                I_strncat(info8, 0x4000, "/");
                I_strncat(info8, 0x4000, search->iwd->iwdBasename);
            }
        }
    }
    *checksums = info5;
    *names = info8;
}

char info6[16384];
char *__cdecl FS_ReferencedIwdPureChecksums()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    int checksum; // [esp+40h] [ebp-Ch]
    searchpath_s *search; // [esp+44h] [ebp-8h]
    int numIwds; // [esp+48h] [ebp-4h]

    info6[0] = 0;
    checksum = fs_checksumFeed;
    numIwds = 0;
    *(_BYTE *)(strlen(info6) + 67341897) = 0;
    *(_BYTE *)(strlen(info6) + 67341898) = 0;
    info6[strlen(info6)] = 35;
    info6[strlen(info6)] = 32;
    for ( search = fs_searchpaths; search; search = search->next )
    {
        if ( search->iwd && !search->bLocalized )
        {
            if ( search->iwd->referenced )
            {
                v0 = va("%i ", search->iwd->pure_checksum);
                I_strncat(info6, 0x4000, v0);
                checksum ^= search->iwd->pure_checksum;
                ++numIwds;
            }
        }
    }
    if ( fs_fakeChkSum )
    {
        v1 = va("%i ", fs_fakeChkSum);
        I_strncat(info6, 0x4000, v1);
    }
    v2 = va("%i ", numIwds ^ checksum);
    I_strncat(info6, 0x4000, v2);
    return info6;
}

char __cdecl FS_PureServerSetLoadedIwds(char *iwdSums, char *iwdNames)
{
    const char *v2; // eax
    int v3; // eax
    char *v4; // eax
    char *v5; // eax
    int j; // [esp+20h] [ebp-2020h]
    int v8; // [esp+24h] [ebp-201Ch]
    int v9; // [esp+28h] [ebp-2018h]
    char v10; // [esp+2Eh] [ebp-2012h]
    char v11; // [esp+2Fh] [ebp-2011h]
    int src[1025]; // [esp+30h] [ebp-2010h] BYREF
    int argIndex; // [esp+1034h] [ebp-100Ch]
    char *s0[1025]; // [esp+1038h] [ebp-1008h] BYREF
    iwd_pure_check_s *i; // [esp+203Ch] [ebp-4h]

    v10 = 0;
    if ( !iwdSums && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\files.cpp", 1519, 0, "%s", "iwdSums") )
        __debugbreak();
    if ( !iwdNames
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\files.cpp", 1520, 0, "%s", "iwdNames") )
    {
        __debugbreak();
    }
    Cmd_TokenizeString(iwdSums);
    v8 = Cmd_Argc();
    if ( v8 > 1024 )
        v8 = 1024;
    for ( argIndex = 0; argIndex < v8; ++argIndex )
    {
        v2 = Cmd_Argv(argIndex);
        v3 = atoi(v2);
        src[argIndex] = v3;
    }
    Cmd_EndTokenizedString();
    Cmd_TokenizeString(iwdNames);
    v9 = Cmd_Argc();
    if ( v9 > 1024 )
        v9 = 1024;
    for ( argIndex = 0; argIndex < v9; ++argIndex )
    {
        v4 = (char *)Cmd_Argv(argIndex);
        v5 = CopyString(v4, "FS_PureServerSetLoadedIwds", 11, SCRIPTINSTANCE_SERVER);
        s0[argIndex] = v5;
    }
    Cmd_EndTokenizedString();
    if ( v8 != v9 )
        Com_Error(ERR_DROP, "iwd sum/name mismatch");
    for ( i = fs_iwdPureChecks; v8 && i; i = i->next )
    {
        v11 = 0;
        for ( argIndex = 0; argIndex < v8; ++argIndex )
        {
            if ( src[argIndex] == i->checksum && !I_stricmp(s0[argIndex], i->iwdBasename) )
            {
                v11 = 1;
                break;
            }
        }
        if ( !v11 )
        {
            v10 = 1;
            break;
        }
    }
    if ( !v10 && v8 == fs_numServerIwds )
    {
        argIndex = 0;
LABEL_34:
        if ( argIndex >= v8 )
        {
            for ( argIndex = 0; argIndex < v9; ++argIndex )
                FreeString(s0[argIndex], 11, SCRIPTINSTANCE_SERVER);
            return 0;
        }
        for ( j = 0; j < fs_numServerIwds; ++j )
        {
            if ( src[argIndex] == fs_serverIwds[j] && !I_stricmp(s0[argIndex], fs_serverIwdNames[j]) )
            {
                ++argIndex;
                goto LABEL_34;
            }
        }
    }
    SND_StopSounds(SND_STOP_STREAMED);
    FS_ShutdownServerIwdNames();
    fs_numServerIwds = v8;
    if ( v8 )
    {
        Com_DPrintf(10, "Connected to a pure server.\n");
        Com_Memcpy(fs_serverIwds, src, 4 * fs_numServerIwds);
        Com_Memcpy(fs_serverIwdNames, s0, 4 * fs_numServerIwds);
        fs_fakeChkSum = 0;
    }
    return v10;
}

void __cdecl FS_ServerSetReferencedIwds(const char *iwdSums, const char *iwdNames)
{
    FS_ShutdownServerReferencedIwds();
    fs_numServerReferencedIwds = FS_ServerSetReferencedFiles(
                                                                 iwdSums,
                                                                 iwdNames,
                                                                 1024,
                                                                 fs_serverReferencedIwds,
                                                                 fs_serverReferencedIwdNames);
}

int __cdecl FS_ServerSetReferencedFiles(
                const char *fileSums,
                const char *fileNames,
                int maxFiles,
                int *fs_sums,
                const char **fs_names)
{
    const char *v5; // eax
    char *v6; // eax
    int c; // [esp+20h] [ebp-Ch]
    int d; // [esp+24h] [ebp-8h]
    int i; // [esp+28h] [ebp-4h]
    int ia; // [esp+28h] [ebp-4h]

    if ( !fileSums
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\files.cpp", 1618, 0, "%s", "fileSums") )
    {
        __debugbreak();
    }
    if ( !fileNames
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\files.cpp", 1619, 0, "%s", "fileNames") )
    {
        __debugbreak();
    }
    if ( !maxFiles
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\files.cpp", 1620, 0, "%s", "maxFiles") )
    {
        __debugbreak();
    }
    if ( !fs_sums && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\files.cpp", 1621, 0, "%s", "fs_sums") )
        __debugbreak();
    Cmd_TokenizeString(fileSums);
    c = Cmd_Argc();
    if ( c > maxFiles )
        c = maxFiles;
    for ( i = 0; i < c; ++i )
    {
        v5 = Cmd_Argv(i);
        fs_sums[i] = atoi(v5);
    }
    Cmd_EndTokenizedString();
    if ( fileNames && *fileNames )
    {
        Cmd_TokenizeString(fileNames);
        d = Cmd_Argc();
        if ( d > maxFiles )
            d = maxFiles;
        if ( c != d )
            Com_Error(ERR_DROP, "file sum/name mismatch");
        for ( ia = 0; ia < d; ++ia )
        {
            v6 = (char *)Cmd_Argv(ia);
            fs_names[ia] = CopyString(v6, "FS_ServerSetReferencedFiles", 11, SCRIPTINSTANCE_SERVER);
        }
        Cmd_EndTokenizedString();
    }
    else if ( c )
    {
        Com_Error(ERR_DROP, "file sum/name mismatch");
    }
    return c;
}

void __cdecl FS_ServerSetReferencedFFs(const char *FFSums, const char *FFNames)
{
    FS_ShutdownServerReferencedFFs();
    fs_numServerReferencedFFs = FS_ServerSetReferencedFiles(
                                                                FFSums,
                                                                FFNames,
                                                                1024,
                                                                fs_serverReferencedFFCheckSums,
                                                                fs_serverReferencedFFNames);
}

char map_basename[64];
char *__cdecl FS_GetMapBaseName(char *mapname)
{
    unsigned int v2; // [esp+0h] [ebp-18h]
    int c; // [esp+10h] [ebp-8h]
    signed int len; // [esp+14h] [ebp-4h]

    if ( !mapname && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\files.cpp", 1701, 0, "%s", "mapname") )
        __debugbreak();
    if ( !I_strnicmp(mapname, "maps/mp/", 8) )
        mapname += 8;
    v2 = strlen(mapname);
    len = v2;
    if ( !I_stricmp(&mapname[v2 - 3], "bsp") )
        len = v2 - 7;
    memcpy((unsigned __int8 *)map_basename, (unsigned __int8 *)mapname, len);
    map_basename[len] = 0;
    for ( c = 0; c < len; ++c )
    {
        if ( map_basename[c] == 37 )
            map_basename[c] = 95;
    }
    return map_basename;
}

