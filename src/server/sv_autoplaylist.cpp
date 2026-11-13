#include "sv_autoplaylist.h"
#include <universal/dvar.h>
#include <qcommon/common.h>
#include <universal/q_parse.h>
#include <qcommon/cmd.h>
#include <universal/offline.h>
#include <client_mp/cl_main_mp.h>

sv_apstate_t s_apstate = AP_SLEEPING;

FileOperationInfo s_finfo;

int s_probabilities[64];

void __cdecl SV_AP_DumpTable()
{
    int i; // [esp+0h] [ebp-4h]

    if ( Dvar_GetBool("sv_ap_debug") )
    {
        Com_Printf(15, "\n**************************************\nPlaylist\t\tProbability\n");
        for ( i = 0; i < 64; ++i )
            Com_Printf(15, "%i\t\t%u\n", i, s_probabilities[i]);
        Com_Printf(15, "\n**************************************\n");
    }
}

bool __cdecl SV_AP_ParseControlFile(unsigned __int8 *controlFile)
{
    parseInfo_t *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    const char *v9; // eax
    const char *v10; // eax
    const char *v11; // eax
    int value; // [esp+0h] [ebp-24h]
    unsigned int probability; // [esp+4h] [ebp-20h]
    int playlist; // [esp+8h] [ebp-1Ch]
    parseInfo_t *token; // [esp+Ch] [ebp-18h]
    int totalProb; // [esp+10h] [ebp-14h]
    int numPlaylists; // [esp+14h] [ebp-10h]
    const char *cFile; // [esp+18h] [ebp-Ch] BYREF
    bool retval; // [esp+1Fh] [ebp-5h]
    int line; // [esp+20h] [ebp-4h]

    cFile = (const char *)controlFile;
    numPlaylists = 0;
    totalProb = 0;
    line = 0;
    retval = 0;
    Com_BeginParseSession("AutoPlaylist");
    Com_SetSpaceDelimited(0);
    Com_SetCSV(1);
    while ( 1 )
    {
        ++line;
        token = Com_Parse(&cFile);
        if ( !token || !token->token[0] )
            break;
        if ( token->token[0] == 35 )
        {
            Com_SkipRestOfLine(&cFile);
        }
        else
        {
            playlist = atoi(token->token);
            if ( playlist <= 0 || playlist >= 64 )
            {
                if ( Dvar_GetBool("sv_ap_fatal") )
                {
                    v6 = va("Failed parsing control file at line %i, playlist %i failed sanity check\n", line, playlist);
                    Com_Error(ERR_DROP, "AP error: %s\n", v6);
                }
                else
                {
                    v7 = va("Failed parsing control file at line %i, playlist %i failed sanity check\n", line, playlist);
                    Com_PrintError(15, "AP Error: %s\n", v7);
                }
                break;
            }
            v1 = Com_Parse(&cFile);
            probability = atoi(v1->token);
            if ( probability > 0x64 )
            {
                if ( Dvar_GetBool("sv_ap_fatal") )
                {
                    v4 = va("Failed parsing control file at line %i, probability %i failed sanity check\n", line, probability);
                    Com_Error(ERR_DROP, "AP error: %s\n", v4);
                }
                else
                {
                    v5 = va("Failed parsing control file at line %i, probability %i failed sanity check\n", line, probability);
                    Com_PrintError(15, "AP Error: %s\n", v5);
                }
                break;
            }
            s_probabilities[playlist] = (unsigned __int8)probability;
            totalProb += probability;
            if ( ++numPlaylists == 64 )
            {
                if ( Dvar_GetBool("sv_ap_fatal") )
                    Com_Error(ERR_DROP, "AP error: %s\n", "Too many playlists, truncating!\n");
                else
                    Com_PrintError(15, "AP Error: %s\n", "Too many playlists, truncating!\n");
                break;
            }
            if ( totalProb > 100 )
            {
                if ( Dvar_GetBool("sv_ap_fatal") )
                {
                    v2 = va("We've exceeded 100%% total probability on line %i\n", line);
                    Com_Error(ERR_DROP, "AP error: %s\n", v2);
                }
                else
                {
                    v3 = va("We've exceeded 100%% total probability on line %i\n", line);
                    Com_PrintError(15, "AP Error: %s\n", v3);
                }
                break;
            }
        }
    }
    Com_EndParseSession();
    if ( totalProb == 100 )
    {
        if ( Dvar_GetBool("sv_ap_debug") )
        {
            v10 = va("Successfully parsed %i playlist probabilities, %i lines\n", numPlaylists, line);
            Com_Printf(15, "AP: %s\n", v10);
        }
        retval = 1;
        SV_AP_DumpTable();
        value = SV_AP_PlaylistFromDistribution();
        if ( Dvar_GetBool("sv_ap_debug") )
        {
            v11 = va("Choosing playlist %i\n", value);
            Com_Printf(15, "AP: %s\n", v11);
        }
        Dvar_SetIntByName("playlist", value);
        Cbuf_AddText(0, "map_rotate\n");
    }
    else if ( Dvar_GetBool("sv_ap_fatal") )
    {
        v8 = va("Finished parsing, total probability is %i, should be 100\n", totalProb);
        Com_Error(ERR_DROP, "AP error: %s\n", v8);
    }
    else
    {
        v9 = va("Finished parsing, total probability is %i, should be 100\n", totalProb);
        Com_PrintError(15, "AP Error: %s\n", v9);
    }
    return retval;
}

int __cdecl SV_AP_PlaylistFromDistribution()
{
    return playlist->current.integer;
}

int __cdecl comparePlaylists(unsigned int *p1, unsigned int *p2)
{
    return *p2 - *p1;
}

void __cdecl SV_AP_GetControlFileComplete()
{
    if ( Dvar_GetBool("sv_ap_debug") )
        Com_Printf(15, "AP: %s\n", "Received control file, attempting to parse\n");
    //operator++(&s_apstate);
    s_apstate++;
}

int __cdecl SV_AP_GetControlFileFailure()
{
    if ( Dvar_GetBool("sv_ap_fatal") )
        Com_Error(ERR_DROP, "AP error: %s\n", "Couldn't get control file.\n");
    else
        Com_PrintError(15, "AP Error: %s\n", "Couldn't get control file.\n");
    return 1;
}

TaskRecord *__cdecl SV_AP_GetControlFile()
{
    return NULL;
}

void __cdecl SV_AP_GetControlFileName(char *buf, int buflen)
{
}

void __cdecl SV_SetGroupCountsComplete()
{
}

void __cdecl SV_GetGroupCountsComplete()
{
}

void __cdecl SV_GroupsFailure(TaskRecord *task)
{
}

void SV_GroupError(const char *fmt, ...)
{
    char msg[132]; // [esp+4h] [ebp-88h] BYREF
    va_list va; // [esp+98h] [ebp+Ch] BYREF

    va_start(va, fmt);
    _vsnprintf(msg, 0x80u, fmt, va);
    Com_PrintError(15, "GROUPS: %s\n", msg);
}

TaskRecord *__cdecl SV_GetGroupCounts()
{
    return NULL;
}

void __cdecl SV_Groups_SetGroupMembership(bool full)
{
}

void __cdecl SV_Groups_ParseGeos(const char *geoblob)
{
}

void __cdecl SV_AP_Start()
{
}

void __cdecl SV_AP_Frame()
{
}

bool __cdecl SV_AP_ServerIsFull()
{
    int i; // [esp+0h] [ebp-8h]
    bool retval; // [esp+7h] [ebp-1h]

    retval = 1;
    if ( com_sv_running && com_sv_running->current.enabled )
    {
        for ( i = 0; i < com_maxclients->current.integer; ++i )
        {
            if ( svs.clients[i].header.state == CS_FREE )
                return 0;
        }
    }
    return retval;
//#else
//    return false;
//#endif
}


