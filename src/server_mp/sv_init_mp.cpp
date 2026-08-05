#include "sv_init_mp.h"
#include <server/server.h>
#include <qcommon/common.h>
#include <clientscript/cscr_stringlist.h>

#include <cstring>
#include "sv_main_mp.h"
#include "sv_main_pc_mp.h"
#include <game_mp/g_main_mp.h>
#include <universal/com_files.h>
#include <universal/com_memory.h>
#include <universal/q_parse.h>
#include <game_mp/g_utils_mp.h>
#include <qcommon/dvar_cmds.h>
#include <qcommon/com_profilemapload.h>
#include <server/sv_game.h>
#include <win32/win_net.h>
#include <live/live_steam_server.h>
#include <win32/win_shared.h>
#include <client_mp/cl_cgame_mp.h>
#include <qcommon/com_clients.h>
#include <qcommon/com_bsp_load_obj.h>
#include <qcommon/cm_load.h>
#include <qcommon/cm_world.h>
#include <universal/com_constantconfigstrings.h>
#include <game_mp/pregame.h>
#include <client_mp/sv_client_mp.h>
#include <ik/ik.h>
#include "sv_ccmds_mp.h"
#include "sv_bot_mp.h"
#include <qcommon/com_gamemodes.h>
#include <qcommon/threads.h>
#include <stringed/stringed_hooks.h>
#include <live/live_sessions_win.h>
#include <live/live_win.h>
#include <DW/dwUtils_pc.h>
#include <client_mp/cl_main_pc_mp.h>
#include <qcommon/files.h>

const dvar_t *sv_gametype;
const dvar_t *sv_privateClients;
const dvar_t *sv_hostname;
const dvar_t *sv_noname;
const dvar_t *sv_geolocation;
const dvar_t *sv_maxgrouperrors;
const dvar_t *sv_ownerid;
const dvar_t *sv_numreservedslots;
const dvar_t *sv_clientSideBullets;
const dvar_t *sv_clientSideVehicles;
const dvar_t *sv_penetrationCount;
const dvar_t *sv_axis_penetrationCount;
const dvar_t *sv_allies_penetrationCount;
const dvar_t *sv_bullet_range;
const dvar_t *sv_hitFXFrustumCutoff;
const dvar_t *sv_punkbuster;
const dvar_t *sv_security;
const dvar_t *sv_ranked;
const dvar_t *ui_ranked;
const dvar_t *sv_dedicatedmaxclients;
const dvar_t *sv_maxclients;
const dvar_t *sv_maxRate;
const dvar_t *sv_minPing;
const dvar_t *sv_maxPing;
const dvar_t *sv_timeout;
const dvar_t *sv_connectTimeout;
const dvar_t *sv_floodProtect;
const dvar_t *sv_showCommands;
const dvar_t *sv_writeConfigStrings;
const dvar_t *scr_writeConfigStrings;
const dvar_t *sv_dwlsgerror;
const dvar_t *sv_allowAnonymous;
const dvar_t *sv_disableClientConsole;
const dvar_t *sv_privatePassword;
const dvar_t *sv_allowDownload;
const dvar_t *sv_iwds;
const dvar_t *sv_iwdNames;
const dvar_t *sv_referencedIwds;
const dvar_t *sv_referencedIwdNames;
const dvar_t *sv_FFCheckSums;
const dvar_t *sv_FFNames;
const dvar_t *sv_referencedFFCheckSums;
const dvar_t *sv_referencedFFNames;
const dvar_t *sv_authenticating;
const dvar_t *sv_voice;
const dvar_t *sv_voiceQuality;
const dvar_t *sv_cheats;
const dvar_t *sv_pure;
const dvar_t *rcon_password;
const dvar_t *sv_fps;
const dvar_t *sv_showPingSpam;
const dvar_t *sv_zombietime;
const dvar_t *sv_reconnectlimit;
const dvar_t *sv_padPackets;
const dvar_t *sv_allowedClan1;
const dvar_t *sv_allowedClan2;
const dvar_t *sv_packet_info;
const dvar_t *sv_showAverageBPS;
const dvar_t *sv_kickBanTime;
const dvar_t *sv_debugMessageKey;
const dvar_t *sv_debugPacketContents;
const dvar_t *sv_debugPacketContentsForClientThisFrame;
const dvar_t *sv_showHuffmanData;
const dvar_t *sv_debugConstantConfigStrings;
const dvar_t *sv_loadMyChanges;
const dvar_t *sv_debugPlayerstate;
const dvar_t *sv_debugPacketContentsQuick;
const dvar_t *sv_printMessageSize;
const dvar_t *sv_mapRotation;
const dvar_t *sv_mapRotationCurrent;
const dvar_t *sv_debugRate;
const dvar_t *sv_debugReliableCmds;
const dvar_t *nextmap;
const dvar_t *com_movieIsPlaying;
const dvar_t *sv_wwwDownload;
const dvar_t *sv_wwwBaseURL;
const dvar_t *sv_wwwDlDisconnected;
const dvar_t *sv_smp;
const dvar_t *sv_network_fps;
const dvar_t *sv_assistWorkers;
const dvar_t *sv_clientArchive;

volatile unsigned int sv_thread_owns_game;

void __cdecl SV_SetConfigstring(int index, char *val)
{
    unsigned __int16 v2; // [esp+20h] [ebp-448h]
    client_t *client; // [esp+38h] [ebp-430h]
    int chunkSize; // [esp+3Ch] [ebp-42Ch]
    int maxChunk; // [esp+40h] [ebp-428h]
    int remaining; // [esp+44h] [ebp-424h]
    char buf[1028]; // [esp+48h] [ebp-420h] BYREF
    int len; // [esp+450h] [ebp-18h]
    int overhead; // [esp+454h] [ebp-14h]
    int sent; // [esp+458h] [ebp-10h]
    int caseSensitive; // [esp+45Ch] [ebp-Ch]
    int i; // [esp+460h] [ebp-8h]
    char cmd; // [esp+467h] [ebp-1h]

    if ( (unsigned int)index >= 0xCBC )
        Com_Error(ERR_DROP, "SV_SetConfigstring: bad index %i", index);
    if ( sv.configstrings[index] )
    {
        if ( !val )
            val = (char *)"";
        if ( strcmp(val, SL_ConvertToString(sv.configstrings[index], SCRIPTINSTANCE_SERVER)) )
        {
            SL_RemoveRefToString(SCRIPTINSTANCE_SERVER, sv.configstrings[index]);
            caseSensitive = index < 1547;
            v2 = index < 1547
                 ? SL_GetString_(SCRIPTINSTANCE_SERVER, val, 0, 19)
                 : SL_GetLowercaseString_(val, 0, 19, SCRIPTINSTANCE_SERVER);
            sv.configstrings[index] = v2;
            if ( SV_Loaded() || sv.restarting )
            {
                len = strlen(val);
                sprintf(buf, "%i", index);
                overhead = &buf[strlen(buf) + 1] - &buf[1] + 4;
                maxChunk = 1024 - overhead;
                i = 0;
                client = svs.clients;
                while ( i < com_maxclients->current.integer )
                {
                    if ( client->header.state >= CS_CLIENTLOADING )
                    {
                        if ( len <= maxChunk )
                        {
                            SV_SendServerCommand(client, SV_CMD_RELIABLE, "%c %i %s", 100, index, val);
                        }
                        else
                        {
                            sent = 0;
                            for ( remaining = len; remaining > 0; remaining -= chunkSize )
                            {
                                if ( sent )
                                {
                                    if ( remaining > maxChunk )
                                        cmd = 121;
                                    else
                                        cmd = 122;
                                }
                                else
                                {
                                    cmd = 120;
                                }
                                chunkSize = maxChunk;
                                while ( remaining > chunkSize && val[chunkSize + sent] == 32 )
                                {
                                    if ( !--chunkSize )
                                        Com_Error(ERR_DROP, "SV_SetConfigstring: big config string with %d empty spaces", maxChunk);
                                }
                                I_strncpyz(buf, &val[sent], chunkSize + 1);
                                SV_SendServerCommand(client, SV_CMD_RELIABLE, "%c %i %s", cmd, index, buf);
                                sent += chunkSize;
                            }
                        }
                    }
                    ++i;
                    ++client;
                }
            }
        }
    }
    else if ( val )
    {
        if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_init_mp.cpp", 156, 0, "%s", "!val") )
            __debugbreak();
    }
}

void __cdecl SV_GetConfigstring(unsigned int index, char *buffer, int bufferSize)
{
    char *v3; // eax

    if ( bufferSize < 1 )
        Com_Error(ERR_DROP, "SV_GetConfigstring: bufferSize == %i", bufferSize);
    if ( index >= MAX_CONFIGSTRINGS )
        Com_Error(ERR_DROP, "SV_GetConfigstring: bad index %i", index);
    if ( !sv.configstrings[index]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_init_mp.cpp",
                    247,
                    0,
                    "%s",
                    "sv.configstrings[index]") )
    {
        __debugbreak();
    }
    v3 = SL_ConvertToString(sv.configstrings[index], SCRIPTINSTANCE_SERVER);
    I_strncpyz(buffer, v3, bufferSize);
}

unsigned int __cdecl SV_GetConfigstringConst(unsigned int index)
{
    iassert((unsigned)index < MAX_CONFIGSTRINGS);
    iassert(sv.configstrings[index]);

    return sv.configstrings[index];
}

void __cdecl SV_SetConfigValueForKey(int start, int max, char *key, char *value)
{
    char *v4; // eax
    unsigned int String; // [esp+0h] [ebp-14h]
    unsigned int name; // [esp+4h] [ebp-10h]
    int i; // [esp+10h] [ebp-4h]

    if ( start < 1547 )
        String = SL_FindString(key, SCRIPTINSTANCE_SERVER);
    else
        String = SL_FindLowercaseString(key, SCRIPTINSTANCE_SERVER);
    for ( i = 0; i < max; ++i )
    {
        name = sv.configstrings[i + start];
        if ( name == sv.emptyConfigString )
        {
            SV_SetConfigstring(i + start, key);
            break;
        }
        if ( String == name )
            break;
    }
    if ( i == max )
    {
        Com_Printf(15, "Overflow at config string start value of %i: key values printed below\n", start);
        for ( i = 0; i < max; ++i )
        {
            v4 = SL_ConvertToString(sv.configstrings[i + start], SCRIPTINSTANCE_SERVER);
            Com_Printf(15, "%i: %i ( %s )\n", i + start, sv.configstrings[i + start], v4);
        }
        Com_Error(ERR_DROP, "SV_SetConfigValueForKey: overflow");
    }
    SV_SetConfigstring(i + max + start, value);
}

void __cdecl SV_SetUserinfo(int index, char *val)
{
    char *v2; // eax

    if ( index < 0 || index >= com_maxclients->current.integer )
        Com_Error(ERR_DROP, "SV_SetUserinfo: bad index %i", index);
    if ( !val )
        val = (char *)"";
    I_strncpyz(svs.clients[index].userinfo, val, 1024);
    v2 = Info_ValueForKey(val, "name");
    I_strncpyz(svs.clients[index].name, v2, 32);
}

void __cdecl SV_GetUserinfo(int index, char *buffer, int bufferSize)
{
    if ( (unsigned int)index >= com_maxclients->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_init_mp.cpp",
                    341,
                    0,
                    "index doesn't index com_maxclients->current.integer\n\t%i not in [0, %i)",
                    index,
                    com_maxclients->current.integer) )
    {
        __debugbreak();
    }
    if ( bufferSize < 1 )
        Com_Error(ERR_DROP, "SV_GetUserinfo: bufferSize == %i", bufferSize);
    if ( index < 0 || index >= com_maxclients->current.integer )
        Com_Error(ERR_DROP, "SV_GetUserinfo: bad index %i", index);
    I_strncpyz(buffer, svs.clients[index].userinfo, bufferSize);
}

void __cdecl SV_CreateBaseline()
{
    float *absmax; // [esp+8h] [ebp-1Ch]
    float *absmin; // [esp+10h] [ebp-14h]
    gentity_s *svent; // [esp+1Ch] [ebp-8h]
    int entnum; // [esp+20h] [ebp-4h]

    for (entnum = 1; entnum < sv.num_entities; ++entnum)
    {
        svent = (gentity_s *)((char *)sv.gentities + entnum * sv.gentitySize);
        if (svent->r.linked && svent->s.eType != 14 && svent->s.eType != 17)
        {
            svent->s.number = entnum;
            memcpy(&sv.svEntities[entnum].baseline, svent, 0xE0u);
            sv.svEntities[entnum].baseline.r.svFlags = svent->r.svFlags;
            sv.svEntities[entnum].baseline.r.clientMask[0] = svent->r.clientMask[0];
            absmin = sv.svEntities[entnum].baseline.r.absmin;
            *absmin = svent->r.absmin[0];
            absmin[1] = svent->r.absmin[1];
            absmin[2] = svent->r.absmin[2];
            absmax = sv.svEntities[entnum].baseline.r.absmax;
            *absmax = svent->r.absmax[0];
            absmax[1] = svent->r.absmax[1];
            absmax[2] = svent->r.absmax[2];
            if (svent->s.clientNum >= 0x20u)
                svent->s.clientNum = 32;
        }
    }
}

void __cdecl SV_SetXUIDConfigStrings()
{
    char *v0; // eax
    int index; // [esp+0h] [ebp-4Ch]
    int j; // [esp+4h] [ebp-48h]
    char filteredName[32]; // [esp+8h] [ebp-44h] BYREF
    client_t *client; // [esp+28h] [ebp-24h]
    int i; // [esp+2Ch] [ebp-20h]
    char xuidStr[20]; // [esp+30h] [ebp-1Ch] BYREF
    int xuidCount; // [esp+48h] [ebp-4h]

    xuidCount = 0;
    for ( i = 0; i < com_maxclients->current.integer; ++i )
    {
        client = &svs.clients[i];
        if ( client->header.state == CS_ACTIVE )
        {
            XUIDToString(&client->dw_userID, xuidStr);
            I_strncpyz(filteredName, client->name, 32);
            for ( j = 0; filteredName[j] && j < 32; ++j )
            {
                if ( filteredName[j] == 32 )
                    filteredName[j] = 1;
            }
            index = xuidCount + 323;
            v0 = va("%s %s", xuidStr, filteredName);
            SV_SetConfigstring(index, v0);
            ++xuidCount;
        }
    }
}

void __cdecl SV_Startup(int controllerIndex)
{
    iassert(!svs.initialized);

    if (IsDedicatedServer())
    {
        SV_ResetDWState();
        Dvar_SetBoolByName("r_gfxopt_water_simulation", 0);

#ifdef KISAK_LIVE
        dwNetStart(1);
        while (g_dwNetStatus == DW_NET_STARTING_ONLINE)
            dwNetPump();

        if (g_svdedicatedauthstate != SV_DWAUTHORIZED)
        {
            DW_DedicatedLogonStart(controllerIndex);
            while (g_svdedicatedauthstate == SV_DWAUTHORIZING)
                DW_DedicatedLogonComplete(0);
            if (g_svdedicatedauthstate != SV_DWAUTHORIZED)
                Com_Error(ERR_DROP, "Dedicated server authentication failure.\n");
            Com_Printf(0, "should be logged in ok\n");
        }
#endif
    }

    //BLOPS_NULLSUB();

    iassert(com_maxclients->current.integer <= 32);
    
    if ( g_entsInSnapshot )
        Dvar_SetInt((dvar_s *)g_entsInSnapshot, 1024);

    svs.initialized = 1;

    Dvar_SetBool((dvar_s *)com_sv_running, 1);
}

void __cdecl SV_SetExpectedHunkUsage(char *mapname)
{
    int handle; // [esp+0h] [ebp-18h] BYREF
    const char *memlistfile; // [esp+4h] [ebp-14h]
    char *buf; // [esp+8h] [ebp-10h]
    int len; // [esp+Ch] [ebp-Ch]
    const char *token; // [esp+10h] [ebp-8h]
    const char *buftrav; // [esp+14h] [ebp-4h] BYREF

    memlistfile = "hunkusage.dat";
    len = FS_FOpenFileByMode((char*)"hunkusage.dat", &handle, FS_READ);
    if ( len >= 0 )
    {
        buf = (char *)Z_Malloc(len + 1, "SV_SetExpectedHunkUsage", 11);
        memset(buf, 0, len + 1);
        FS_Read((unsigned __int8 *)buf, len, handle);
        FS_FCloseFile(handle);
        buftrav = buf;
        while ( 1 )
        {
            token = (const char *)Com_Parse(&buftrav);
            if ( !token
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_init_mp.cpp", 724, 0, "%s", "token") )
            {
                __debugbreak();
            }
            if ( !*token )
                break;
            if ( !I_stricmp(token, mapname) )
            {
                token = (const char *)Com_Parse(&buftrav);
                if ( token )
                {
                    if ( *token )
                    {
                        com_expectedHunkUsage = atoi(token);
                        Z_Free(buf, 11);
                        return;
                    }
                }
            }
        }
        Z_Free(buf, 11);
    }
}

void __cdecl SV_ClearServer()
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < MAX_CONFIGSTRINGS; ++i )
    {
        if ( sv.configstrings[i] )
            SL_RemoveRefToString(SCRIPTINSTANCE_SERVER, sv.configstrings[i]);
    }
    if ( sv.emptyConfigString )
        SL_RemoveRefToString(SCRIPTINSTANCE_SERVER, sv.emptyConfigString);
    G_ClearCachedModels();
    Com_Memset(&sv, 0, sizeof(server_t));
    com_inServerFrame = 0;
}

void __cdecl SV_InitArchivedSnapshot()
{
    svs.nextArchivedSnapshotFrames = 0;
    svs.nextArchivedSnapshotBuffer = 0;
    svs.nextCachedSnapshotEntities = 0;
    svs.nextCachedSnapshotClients = 0;
    svs.nextCachedSnapshotFrames = 0;
    svs.nextCachedSnapshotMatchStates = 0;
}

void __cdecl SV_SetSystemInfoConfig()
{
    dvar_modifiedFlags &= ~8u;
}

void __cdecl SV_SaveSystemInfo()
{
    char *v0; // eax

    SV_SetSystemInfoConfig();
    v0 = Dvar_InfoString(0, 4);
    SV_SetConfigstring(0, v0);
    dvar_modifiedFlags &= ~4u;
    SV_SetConfig(23, 150, 256);
    dvar_modifiedFlags &= ~0x100u;
    sv.state = SS_GAME;
    sv.restarting = 0;
}

void __cdecl SV_SetServerDvarsBeforeScriptsInit()
{
    int LicenseType; // eax
    int v1; // eax

    LicenseType = SV_GetLicenseType();
    Dvar_SetInt((dvar_s *)sv_ranked, LicenseType);
    v1 = SV_GetLicenseType();
    if ( SV_IsServerRanked(v1) )
        Dvar_SetBoolByName("g_allowvote", 0);
}

void __cdecl    SV_SpawnServer(int controllerIndex, char *server, int mapIsPreloaded, int savegame)
{
    //jpeg_decompress_struct *v10; // [esp+0h] [ebp-84h]
    unsigned int bspVersion; // [esp+1Ch] [ebp-68h]
    client_t *client; // [esp+20h] [ebp-64h]
    char filename[68]; // [esp+24h] [ebp-60h] BYREF
    int checksum; // [esp+6Ch] [ebp-18h] BYREF
    int party; // [esp+70h] [ebp-14h]
    int savepersist; // [esp+74h] [ebp-10h]
    int i; // [esp+80h] [ebp-4h]

    Com_SyncThreads();
#ifdef KISAK_LIVE
    MatchRecord_InitMatchData();
#endif
    iassert(SV_GetServerThreadOwnsGame() == 0);

    if ( useFastFile->current.enabled && !mapIsPreloaded )
    {
        DB_AddUserMapDir(server);
        FS_DisablePureCheck(1);
        Com_LoadMapLoadingScreenFastFile(server);
    }

    if (!mapIsPreloaded && !IsDedicatedServer())
    {
        CL_SetupClientsForIngame();
    }

    CL_AllocatePerLocalClientMemory();
    Scr_ParseGameTypeList();
    SV_SetGametype();

    if ( !mapIsPreloaded )
        CL_InitLoad(server, sv_gametype->current.string);

    if ( useFastFile->current.enabled && !mapIsPreloaded )
        DB_SyncXAssets();

    R_BeginRemoteScreenUpdate();

    if ( fs_debug->current.integer == 2 )
        Dvar_SetInt((dvar_s*)fs_debug, 0);

    ProfLoad_Activate();

    iassert(SV_GetServerThreadOwnsGame() == 0);

    if ( com_sv_running->current.enabled )
    {
        savepersist = G_GetSavePersist();
        i = 0;
        client = svs.clients;
        while ( i < com_maxclients->current.integer )
        {
            if ( client->header.state >= CS_CLIENTLOADING )
            {
                Com_sprintf(filename, 64, "loadingnewmap\n%s\n%s", server, sv_gametype->current.string);
                NET_OutOfBandPrint(NS_SERVER, client->header.netchan.remoteAddress, filename);
            }
            ++i;
            ++client;
        }
        NET_Sleep(250);
    }
    else
    {
        savepersist = 0;
    }

    iassert(!strstr(server, "\\"));

    Dvar_SetString((dvar_s*)sv_mapname, server);
    LiveSteam_Server_Init();
    R_EndRemoteScreenUpdate(0);

    if ( !mapIsPreloaded )
    {
        CL_MapLoading(server);
        R_BeginRemoteScreenUpdate();
        R_EndRemoteScreenUpdate(0);
        CL_ShutdownAll();
    }

    SV_ShutdownGameProgs();
    Com_Printf(15, "------ Server Initialization ------\n");
    Com_Printf(15, "Server: %s\n", server);
    SV_ClearServer();

    if ( !useFastFile->current.enabled )
    {
        FS_Shutdown();
        FS_ClearIwdReferences();
    }
    if ( !mapIsPreloaded )
        Com_Restart();

    if (com_sv_running->current.enabled)
    {
        //BLOPS_NULLSUB(v10);
    }
    else
    {
        SV_Startup(controllerIndex);
    }

    Dvar_ClearModified(com_maxclients);
    I_strncpyz(sv.gametype, sv_gametype->current.string, 64);
    G_srand(Sys_MillisecondsRaw());
    sv.checksumFeed = Sys_Milliseconds() ^ (G_rand() ^ (G_rand() << 16));
    FS_Restart(0, sv.checksumFeed);

    if ( !useFastFile->current.enabled )
    {
        Com_GetBspFilename(filename, 64, server);
        SV_SetExpectedHunkUsage(filename);
    }

    if ( !mapIsPreloaded )
    {
        ProfLoad_Begin("start loading client");
        CL_StartLoading();
        ProfLoad_End();
        if ( useFastFile->current.enabled )
        {
            Com_LoadLevelFastFiles(server);

            iassert(sv_loadMyChanges);

            if ( sv_loadMyChanges->current.enabled )
            {
                Cbuf_ExecuteBuffer(0, Com_LocalClient_GetControllerIndex(0), (char*)"loadzone mychanges\n");
            }
        }
    }

    R_BeginRemoteScreenUpdate();
    sv.emptyConfigString = SL_GetString_(SCRIPTINSTANCE_SERVER, "", 0, 19);
    for ( i = 0; i < MAX_CONFIGSTRINGS; ++i )
    {
        sv.configstrings[i] = SL_GetString_(SCRIPTINSTANCE_SERVER, "", 0, 19);
    }
    Dvar_ResetScriptInfo();
    svs.nextSnapshotEntities = 0;
    svs.nextSnapshotClients = 0;
    SV_InitArchivedSnapshot();
    SV_InitSnapshot();
    svs.snapFlagServerBit ^= 4u;
    Dvar_SetString((dvar_s*)nextmap, "map_restart");
    Dvar_SetInt((dvar_s *)cl_paused, 0);
    Com_GetBspFilename(filename, 64, server);
    if ( !useFastFile->current.enabled )
        Com_LoadBsp(filename);
    if ( G_OnlyConnectingPaths() )
    {
        bspVersion = Com_GetBspVersion();
        if ( bspVersion == 45 )
            Material_SetAlwaysUseDefaultMaterial(1);
        else
            Com_Error(
                ERR_DROP,
                "Can only connect pths with bsp version %i, but the bsp is version %i.    You need to recompile the map.",
                45,
                bspVersion);
    }
    CM_LoadMap(filename, &checksum);
    Com_LoadWorld(filename);
    if ( !G_OnlyConnectingPaths() && !useFastFile->current.enabled )
        Com_UnloadBsp();
    CM_LinkWorld();
    sv_serverId_value = (unsigned __int8)(sv_serverId_value + 16);
    if ( (sv_serverId_value & 0xF0) == 0 )
        sv_serverId_value += 16;
    sv.start_frameTime = com_frameTime;
    sv.state = SS_LOADING;
    if ( !G_ExitAfterConnectPaths() && !useFastFile->current.enabled )
        Com_GetBspFilename(filename, 64, server);
    R_EndRemoteScreenUpdate(0);
    party = 1;
    if ( CCS_ShouldLoadConstConfigStrings(1) )
        CCS_LoadConstantConfigStrings(server, sv_gametype->current.string);
    else
        CCS_ClearConstantConfigStrings();
    R_BeginRemoteScreenUpdate();

    if (!IsDedicatedServer() && !G_ExitAfterToolComplete())
    {
        if (!g_serverSession.sessionHandle)
        {
            Session_Init();
            Session_StartHost(&g_serverSession, 1, Com_GetPrivateClients(), 0x20 - Com_GetPrivateClients());
        }
        else
        {
            //I_strncpyz(g_matchmakingInfo->m_membermapname, sv_mapname->current.string, 0x21);
            //g_matchmakingInfo->m_memberGAME_TYPE = Com_GametypeToInt(g_gametype->current.string);
            //Session_Modify(0, &g_serverSession, 1, Com_GetPrivateClients(), 0x20 - Com_GetPrivateClients());
        }
    }

    iassert(SV_GetServerThreadOwnsGame() == 0);

    SV_SetXUIDConfigStrings();
    Pregame_Reset();
    SV_SetServerDvarsBeforeScriptsInit();

    {
        ProfLoad_Begin("Init game");
        SV_InitGameProgs(savepersist);
        ProfLoad_End();
    }


    SV_CreateBaseline();
    Demo_SetDemoClientState(0);

    for (i = 0; i < com_maxclients->current.integer; ++i)
    {
        client_t *cl = &svs.clients[i];

        if (cl->header.state >= CS_CONNECTED)
        {
            if (cl->bIsDemoClient || cl->bIsTestClient)
            {
                SV_DropClient(cl, "EXE_PLAYERKICKED", false, true);
                continue;
            }

            const char *denied = ClientConnect(i, cl->scriptId);

            if (denied)
            {
                SV_DropClient(cl, denied, true, true);
                continue;
            }

            // Restore connected state
            cl->header.state = CS_CONNECTED;
        }
    }

    const char *names;
    const char *checksums;

    if (sv_pure->current.enabled)
    {
        FS_LoadedIwds(&checksums, &names);

        if (!*checksums)
            Com_PrintWarning(15, "WARNING: sv_pure set but no IWD files loaded\n");

        Dvar_SetString((dvar_s*)sv_iwds, checksums);
        Dvar_SetString((dvar_s*)sv_iwdNames, names);
    }
    else
    {
        Dvar_SetString((dvar_s *)sv_iwds, "");
        Dvar_SetString((dvar_s *)sv_iwdNames, "");
    }

    FS_ReferencedIwds(&checksums, &names);

    Dvar_SetString((dvar_s*)sv_referencedIwds, checksums);
    Dvar_SetString((dvar_s*)sv_referencedIwdNames, names);

    Dvar_SetString((dvar_s*)sv_referencedFFCheckSums, DB_ReferencedFFChecksums());
    Dvar_SetString((dvar_s*)sv_referencedFFNames, DB_ReferencedFFNameList());

    SV_SaveSystemInfo();

    if (Dvar_GetBool("playlist_enabled"))
    {
        int maxplayers = Dvar_GetInt("party_maxplayers");
        Dvar_SetIntByName("sv_maxclients", maxplayers);
    }

    SV_Heartbeat_f();

    ProfLoad_Deactivate();

    Com_Printf(15, "-----------------------------------\n");

    if (G_ExitAfterToolComplete())
        Dvar_SetBoolByName("sv_punkbuster", false);

    //DisablePbSv();

    R_EndRemoteScreenUpdate(NULL);

    if (G_OnlyConnectingPaths())
        Path_InitPaths();

}

const int ikStateSize = 3680;
unsigned __int8 *sv_ikBuf;
char *__cdecl SV_AllocateClientMemory_SizeRequired(int maxLocalClients, int maxClients)
{
    int v3; // [esp+0h] [ebp-Ch]
    int v4; // [esp+4h] [ebp-8h]

    if ( maxClients > 2 )
        v4 = maxClients;
    else
        v4 = 2;
    if ( maxClients > 2 )
        v3 = maxClients;
    else
        v3 = 2;

    return (char *)(0x118D00 * maxClients
        + 0x1A00 * maxClients * maxClients
        + 0x4880
        + (maxClients << 7)
        + 0x5000 * maxClients
        + 0x80
        + 0x2780 * v3 * v4
        + 0x1002580
        + 0x20 * ikStateSize);
}

void __cdecl SV_AllocateClientMemory(HunkUser *hunk, int maxLocalClients, int maxClients)
{
    int v3; // [esp+0h] [ebp-8h]
    int v4; // [esp+4h] [ebp-4h]

    svs.clients = (client_t *)Hunk_UserAlloc(hunk, 544000 * maxClients, 4, "svs.clients");
    memset(svs.clients, 0, 544000 * maxClients);
    svs.numSnapshotMatchStates = 32 * maxClients;
    svs.snapshotMatchStates = (MatchState *)Hunk_UserAlloc(hunk, maxClients << 12, 4, "svs.snapshotMatchStates");
    memset(svs.snapshotMatchStates, 0, svs.numSnapshotMatchStates << 7);
    svs.numSnapshotEntities = 2688 * maxClients;
    svs.snapshotEntities = (entityState_s *)Hunk_UserAlloc(hunk, 602112 * maxClients, 4, "svs.snapshotEntities");
    memset(svs.snapshotEntities, 0, 224 * svs.numSnapshotEntities);
    svs.numSnapshotClients = 32 * maxClients * maxClients;
    svs.snapshotClients = (clientState_s *)Hunk_UserAlloc(hunk, 6656 * maxClients * maxClients, 4, "svs.snapshotClients");
    memset(svs.snapshotClients, 0, 208 * svs.numSnapshotClients);
    svs.cachedSnapshotFrames = (cachedSnapshot_t *)Hunk_UserAlloc(hunk, 18432, 128, "svs.cachedSnapshotFrames");
    memset(svs.cachedSnapshotFrames, 0, 0x4800u);
    svs.numCachedSnapshotMatchStates = maxClients;
    svs.cachedSnapshotMatchStates = (MatchState *)Hunk_UserAlloc(
                                                                                                    hunk,
                                                                                                    maxClients << 7,
                                                                                                    4,
                                                                                                    "svs.cachedSnapshotMatchStates");
    memset(svs.cachedSnapshotMatchStates, 0, svs.numCachedSnapshotMatchStates << 7);
    svs.numCachedSnapshotEntities = 80 * maxClients;
    svs.cachedSnapshotEntities = (archivedEntity_s *)Hunk_UserAlloc(
                                                                                                         hunk,
                                                                                                         20480 * maxClients,
                                                                                                         128,
                                                                                                         "svs.cachedSnapshotEntities");
    memset(svs.cachedSnapshotEntities, 0, svs.numCachedSnapshotEntities << 8);
    if ( maxClients > 2 )
        v4 = maxClients;
    else
        v4 = 2;
    if ( maxClients > 2 )
        v3 = maxClients;
    else
        v3 = 2;
    svs.numCachedSnapshotClients = v3 * v4;
    svs.cachedSnapshotClients = (cachedClient_s *)Hunk_UserAlloc(hunk, 10112 * v3 * v4, 4, "svs.cachedSnapshotClients");
    memset(svs.cachedSnapshotClients, 0, 10112 * svs.numCachedSnapshotClients);
    svs.archivedSnapshotFrames = (archivedSnapshot_s *)Hunk_UserAlloc(hunk, 9600, 4, "svs.archivedSnapshotFrames");
    memset(svs.archivedSnapshotFrames, 0, 0x2580u);
    svs.archivedSnapshotBuffer = (unsigned __int8 *)Hunk_UserAlloc(hunk, 0x1000000, 4, "svs.archivedSnapshotBuffer");
    memset(svs.archivedSnapshotBuffer, 0, 0x1000000);
    sv_ikBuf = (unsigned __int8 *)Hunk_UserAlloc(hunk, 32 * ikStateSize, 16, "sv_ikStatesArray");
    memset(sv_ikBuf, 0, 32 * ikStateSize);
    IK_AllocateLocalClientMemory(sv_ikBuf, -1);
}

void __cdecl SV_FreeClientMemory(HunkUser *hunk)
{
    if ( sv_ikBuf )
    {
        Hunk_UserFree(hunk, sv_ikBuf);
        IK_AllocateLocalClientMemory(0, -1);
    }
    sv_ikBuf = 0;
    if ( svs.clients )
        Hunk_UserFree(hunk, svs.clients);
    svs.clients = 0;
    if ( svs.snapshotMatchStates )
        Hunk_UserFree(hunk, svs.snapshotMatchStates);
    svs.snapshotMatchStates = 0;
    if ( svs.snapshotEntities )
        Hunk_UserFree(hunk, svs.snapshotEntities);
    svs.snapshotEntities = 0;
    if ( svs.snapshotClients )
        Hunk_UserFree(hunk, svs.snapshotClients);
    svs.snapshotClients = 0;
    if ( svs.cachedSnapshotFrames )
        Hunk_UserFree(hunk, svs.cachedSnapshotFrames);
    svs.cachedSnapshotFrames = 0;
    if ( svs.cachedSnapshotMatchStates )
        Hunk_UserFree(hunk, svs.cachedSnapshotMatchStates);
    svs.cachedSnapshotMatchStates = 0;
    if ( svs.cachedSnapshotEntities )
        Hunk_UserFree(hunk, svs.cachedSnapshotEntities);
    svs.cachedSnapshotEntities = 0;
    if ( svs.cachedSnapshotClients )
        Hunk_UserFree(hunk, svs.cachedSnapshotClients);
    svs.cachedSnapshotClients = 0;
    if ( svs.archivedSnapshotFrames )
        Hunk_UserFree(hunk, svs.archivedSnapshotFrames);
    svs.archivedSnapshotFrames = 0;
    if ( svs.archivedSnapshotBuffer )
        Hunk_UserFree(hunk, svs.archivedSnapshotBuffer);
    svs.archivedSnapshotBuffer = 0;
}

bool __cdecl SV_Loaded()
{
    return sv.state == SS_GAME;
}

void __cdecl SV_Init()
{
    SV_AddOperatorCommands();
    Demo_RegisterDvars();
    SV_BotRegisterDvars();
    sv_gametype = _Dvar_RegisterString("g_gametype", "tdm", 0x24u, "Current game type");
    _Dvar_RegisterInt("protocol", 1044, 1044, 1044, 0x44u, "Protocol version");
    sv_mapname = _Dvar_RegisterString("mapname", (char *)"", 0x44u, "Current map name");
    sv_privateClients = _Dvar_RegisterInt(
                                                "sv_privateClients",
                                                0,
                                                0,
                                                32,
                                                0,
                                                "Maximum number of private clients allowed on the server");
    sv_hostname = _Dvar_RegisterString("sv_hostname", IsDedicatedServer() ? "BlackOpsPublic" : "BlackOpsPrivate", 5u, "Host name of the server");
    sv_noname = _Dvar_RegisterString(
                                "sv_noname",
                                "Unknown Soldier",
                                0,
                                "Player name assigned to players that fail name validation");
    sv_geolocation = _Dvar_RegisterString("sv_geolocation", (char *)"", 0x10u, "geolocation");
    sv_maxgrouperrors = _Dvar_RegisterInt(
                                                "sv_maxgrouperrors",
                                                5,
                                                0,
                                                0x7FFFFFFF,
                                                0,
                                                "Number of group errors before a sys_error");
    sv_ownerid = _Dvar_RegisterString("sv_ownerid", (char *)"", 0, "SteamID of server owner");
    sv_numreservedslots = _Dvar_RegisterInt("maxreservedslots", 6, 0, 6, 0x40u, "");
    sv_clientSideBullets = _Dvar_RegisterBool(
                                                     "sv_clientSideBullets",
                                                     1,
                                                     0x100u,
                                                     "If true, clients will synthesize tracers and bullet impacts");
    sv_clientSideVehicles = _Dvar_RegisterBool(
                                                        "sv_clientSideVehicles",
                                                        1,
                                                        0x100u,
                                                        "If true, vehicles will be predicted on the client reducing response time");
    sv_penetrationCount = _Dvar_RegisterInt(
                                                    "penetrationCount",
                                                    5,
                                                    0,
                                                    5,
                                                    0x100u,
                                                    "Maximum number of private clients allowed on the server");
    sv_axis_penetrationCount = _Dvar_RegisterInt("penetrationCount_axis", 5, 0, 5, 0x100u, "Maximum number for TEAM_AXIS");
    sv_allies_penetrationCount = _Dvar_RegisterInt(
                                                                 "penetrationCount_allies",
                                                                 5,
                                                                 0,
                                                                 5,
                                                                 0x100u,
                                                                 "Maximum number for TEAM_ALLIES");
    sv_bullet_range = _Dvar_RegisterFloat(
                                            "bulletrange",
                                            8192.0,
                                            0.0,
                                            65536.0,
                                            0x100u,
                                            "Defines the how far the bulllets will go.");
    sv_hitFXFrustumCutoff = _Dvar_RegisterFloat(
                                                        "fxfrustumCutoff",
                                                        1000.0,
                                                        0.0,
                                                        5000.0,
                                                        0x100u,
                                                        "Hit effects that are more than <this value> outside of the frustum will be culled.");
    sv_punkbuster = _Dvar_RegisterBool("sv_punkbuster", 0, 0x10u, "Enable PunkBuster on this server");
    sv_security = _Dvar_RegisterInt("sv_security", 1, 0, 2, 0x14u, "Enable security on this server");
    sv_ranked = _Dvar_RegisterInt("sv_ranked", 0, 0, 5, 0x44u, "Server license type.");
    ui_ranked = _Dvar_RegisterBool("ui_ranked", 0, 0x80u, "True if playing in a ranked server");
    sv_dedicatedmaxclients = _Dvar_RegisterInt(
                                                         "sv_dedicatedmaxclients",
                                                         32,
                                                         1,
                                                         32,
                                                         0x10u,
                                                         "The dedicated server max clients");
    sv_maxclients = _Dvar_RegisterInt(
                                        "sv_maxclients",
                                        18,
                                        1,
                                        IsDedicatedServer() ? sv_dedicatedmaxclients->current.integer : 30,
                                        5u,
                                        "The maximum number of clients that can connect to a server");
    sv_maxRate = _Dvar_RegisterInt("sv_maxRate", 5000, 0, 25000, 5u, "Maximum bit rate");
    sv_minPing = _Dvar_RegisterInt("sv_minPing", 0, 0, 999, 5u, "Minimum ping allowed on the server");
    sv_maxPing = _Dvar_RegisterInt("sv_maxPing", 0, 0, 999, 5u, "Maximum ping allowed on the server");
    sv_timeout = _Dvar_RegisterInt("sv_timeout", 240, 0, 1800, 0, "seconds without any message");
    sv_connectTimeout = _Dvar_RegisterInt(
                                                "sv_connectTimeout",
                                                80,
                                                0,
                                                1800,
                                                0,
                                                "seconds without any message when a client is loading");
    sv_floodProtect = _Dvar_RegisterInt(
                                            "sv_floodprotect",
                                            4,
                                            0,
                                            0x7FFFFFFF,
                                            5u,
                                            "Prevent malicious lagging by flooding the server with commands.    Is the number of client commands "
                                            "the server will process per 800ms.    0 means no flood protection.");
    sv_showCommands = _Dvar_RegisterBool("sv_showCommands", 0, 0, "Print client commands in the log file");
    sv_writeConfigStrings = _Dvar_RegisterBool("sv_writeConfigStrings", 0, 0, "Write out the config string file");
    scr_writeConfigStrings = _Dvar_RegisterBool(
                                                         "scr_writeConfigStrings",
                                                         0,
                                                         0,
                                                         "Special script mode for writing config string files");
    _Dvar_RegisterString("sv_keywords", (char *)"", 0, "Server keywords");
    sv_dwlsgerror = _Dvar_RegisterBool("sv_dwlsgerror", 0, 0, "Demonware LSG error");
    sv_allowAnonymous = _Dvar_RegisterBool("sv_allowAnonymous", 0, 0, "Allow anonymous access");
    sv_disableClientConsole = _Dvar_RegisterBool(
                                                            "sv_disableClientConsole",
                                                            0,
                                                            4u,
                                                            "Disallow remote clients from accessing the console");
    sv_privatePassword = _Dvar_RegisterString(
                                                 "sv_privatePassword",
                                                 (char *)"",
                                                 0,
                                                 "password for the privateClient slots");
    sv_allowDownload = _Dvar_RegisterBool("sv_allowDownload", 1, 1u, "Allow auto download of files");
    sv_iwds = _Dvar_RegisterString("sv_iwds", (char *)"", 0x48u, "IWD server checksums");
    sv_iwdNames = _Dvar_RegisterString(
                                    "sv_iwdNames",
                                    (char *)"",
                                    0x48u,
                                    "Names of IWD files used by the server");
    sv_referencedIwds = _Dvar_RegisterString(
                                                "sv_referencedIwds",
                                                (char *)"",
                                                0x48u,
                                                "Checksum of all referenced IWD files");
    sv_referencedIwdNames = _Dvar_RegisterString(
                                                        "sv_referencedIwdNames",
                                                        (char *)"",
                                                        0x48u,
                                                        "Names of all referenced IWD files");
    sv_FFCheckSums = _Dvar_RegisterString("sv_FFCheckSums", (char *)"", 0x48u, "Fast File server checksums");
    sv_FFNames = _Dvar_RegisterString(
                                 "sv_FFNames",
                                 (char *)"",
                                 0x48u,
                                 "Names of Fast Files used by the server");
    sv_referencedFFCheckSums = _Dvar_RegisterString(
                                                             "sv_referencedFFCheckSums",
                                                             (char *)"",
                                                             0x48u,
                                                             "Checksum of all referenced Fast Files");
    sv_referencedFFNames = _Dvar_RegisterString(
                                                     "sv_referencedFFNames",
                                                     (char *)"",
                                                     0x48u,
                                                     "Names of all referenced Fast Files");
    sv_authenticating = _Dvar_RegisterBool("sv_authenticating", 0, 0x40u, "");
    sv_voice = _Dvar_RegisterBool("sv_voice", 0, 0x105u, "Use server side voice communications");
    sv_voiceQuality = _Dvar_RegisterInt("sv_voiceQuality", 3, 0, 9, 0x100u, "Voice quality");
    sv_cheats = _Dvar_RegisterBool("sv_cheats", 1, 0x18u, "Enable cheats on the server");
    sv_pure = _Dvar_RegisterBool("sv_pure", 0, 0x104u, "Cannot use modified IWD files");
    rcon_password = _Dvar_RegisterString("rcon_password", (char *)"", 0, "Password for the rcon command");
    sv_fps = _Dvar_RegisterInt("sv_fps", 20, 10, 1000, 0, "Server frames per second");
    sv_showPingSpam = _Dvar_RegisterInt("sv_showPingSpam", 0, 0, 1, 0, "Turns on ping info spam.");
    sv_zombietime = _Dvar_RegisterInt("sv_zombietime", 2, 0, 1800, 0, "seconds to sync messages after disconnect");
    sv_reconnectlimit = _Dvar_RegisterInt("sv_reconnectlimit", 3, 0, 1800, 1u, "minimum seconds between connect messages");
    sv_padPackets = _Dvar_RegisterInt("sv_padPackets", 0, 0, 0x7FFFFFFF, 0, "add nop bytes to messages");
    sv_allowedClan1 = _Dvar_RegisterString(
                                            "sv_allowedClan1",
                                            (char *)"",
                                            0,
                                            "Allow this clan to join the server");
    sv_allowedClan2 = _Dvar_RegisterString(
                                            "sv_allowedClan2",
                                            (char *)"",
                                            0,
                                            "Allow this clan to join the server");
    sv_packet_info = _Dvar_RegisterBool("sv_packet_info", 0, 0, "Enable packet info debugging information");
    sv_showAverageBPS = _Dvar_RegisterBool("sv_showAverageBPS", 0, 0, "Show average bytes per second for net debugging");
    sv_kickBanTime = _Dvar_RegisterFloat(
                                         "sv_kickBanTime",
                                         300.0,
                                         0.0,
                                         3600.0,
                                         0,
                                         "Time in seconds for a player to be banned from the server after being kicked");
    sv_debugMessageKey = _Dvar_RegisterBool("sv_debugMessageKey", 0, 0, "net message key generation debugging");
    sv_debugPacketContents = _Dvar_RegisterBool(
                                                         "sv_debugPacketContents",
                                                         0,
                                                         0,
                                                         "print out the contents of every snapshot (VERY SLOW)");
    sv_debugPacketContentsForClientThisFrame = _Dvar_RegisterBool(
                                                                                             "sv_debugPacketContentsForClientThisFrame",
                                                                                             0,
                                                                                             0,
                                                                                             "set to true to get the next snapshot for this client");
    sv_showHuffmanData = _Dvar_RegisterBool(
                                                 "sv_showHuffmanData",
                                                 0,
                                                 0,
                                                 "To enable or disable the printing of the huffman data byte count");
    sv_debugConstantConfigStrings = _Dvar_RegisterBool(
                                                                        "sv_debugConstantConfigStrings",
                                                                        0,
                                                                        0,
                                                                        "const config strings debugging");
    sv_loadMyChanges = _Dvar_RegisterBool("sv_loadMyChanges", 0, 0, "Load my changes fast file on devmap.");
    sv_debugPlayerstate = _Dvar_RegisterBool(
                                                    "sv_debugPlayerstate",
                                                    0,
                                                    0,
                                                    "Print out what fields are changing in the playerstate");
    sv_debugPacketContentsQuick = _Dvar_RegisterInt(
                                                                    "sv_debugPacketContentsQuick",
                                                                    0,
                                                                    0,
                                                                    2,
                                                                    0,
                                                                    "print out snapshot entity changed fields");
    sv_printMessageSize = _Dvar_RegisterBool("sv_printMessageSize", 0, 0, "print out size of client messages");
    sv_mapRotation = _Dvar_RegisterString(
                                         "sv_mapRotation",
                                         (char *)"",
                                         0,
                                         "List of maps for the server to play");
    sv_mapRotationCurrent = _Dvar_RegisterString(
                                                        "sv_mapRotationCurrent",
                                                        (char *)"",
                                                        0,
                                                        "Current map in the map rotation");
    sv_debugRate = _Dvar_RegisterBool("sv_debugRate", 0, 0, "Enable snapshot rate debugging info");
    sv_debugReliableCmds = _Dvar_RegisterBool(
                                                     "sv_debugReliableCmds",
                                                     0,
                                                     0,
                                                     "Enable debugging information for 'reliable' commands");
    nextmap = _Dvar_RegisterString("nextmap", (char *)"", 0, "Next map to play");
    com_movieIsPlaying = _Dvar_RegisterBool("com_movieIsPlaying", 0, 0, "Is a movie playiner.");
    sv_wwwDownload = _Dvar_RegisterBool("sv_wwwDownload", 0, 1u, "Enable http downloads");
    sv_wwwBaseURL = _Dvar_RegisterString(
                                        "sv_wwwBaseURL",
                                        (char *)"",
                                        1u,
                                        "The base url for files downloaded via http");
    sv_wwwDlDisconnected = _Dvar_RegisterBool(
                                                     "sv_wwwDlDisconnected",
                                                     0,
                                                     1u,
                                                     "Should clients stay connected while downloading?");
    sv_smp = _Dvar_RegisterBool("sv_smp", 1, 0x80u, "Enable server multithreading");
    sv_network_fps = _Dvar_RegisterInt(
                                         "sv_network_fps",
                                         100,
                                         20,
                                         200,
                                         0x80u,
                                         "Number of times per second the server checks for net messages");
    sv_assistWorkers = _Dvar_RegisterBool("sv_assistWorkers", 0, 0x80u, "Enable server worker thread assist when idle");
    sv_clientArchive = _Dvar_RegisterBool(
                                             "sv_clientArchive",
                                             1,
                                             0,
                                             "Have the clients archive data to save bandwidth on the server");
}

void __cdecl SV_DropAllClients()
{
    client_t *drop; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    i = 0;
    drop = svs.clients;
    while ( i < com_maxclients->current.integer )
    {
        if ( drop->header.state >= CS_CONNECTED )
            SV_DropClient(drop, "EXE_DISCONNECTED", 1, !xblive_wagermatch->current.enabled);
        ++i;
        ++drop;
    }
}

void __cdecl SV_Shutdown(const char *finalmsg)
{
    if ( !Sys_IsMainThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_init_mp.cpp",
                    2039,
                    0,
                    "%s",
                    "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    if ( com_sv_running && com_sv_running->current.enabled )
    {
        Com_SyncThreads();
        if ( Demo_IsRecording() )
            Demo_End(0);
        Com_Printf(15, "----- Server Shutdown -----\n");
        SV_FinalMessage(finalmsg);
        if (IsDedicatedServer())
        {
            SV_SysLog_LogMessage(5, va("shutting down: %s", finalmsg));
        }
        //BLOPS_NULLSUB();
        SV_ShutdownGameProgs();
        SV_DropAllClients();
        LiveSteam_Server_Shutdown();
        SV_FreeClients();
        SV_ClearServer();
        Dvar_SetBool((dvar_s *)com_sv_running, 0);
        CL_FreePerLocalClientMemory();
        memset(&svs, 0, sizeof(svs));
        if (!IsDedicatedServer())
        {
            Session_DeleteSession(&g_serverSession);
        }
        //*(unsigned int *)(*((unsigned int *)NtCurrentTeb()->ThreadLocalStoragePointer + _tls_index) + 8) = 0;
        bgs = 0;
        Com_Printf(15, "---------------------------\n");
    }
}

void __cdecl SV_FinalMessage(const char *message)
{
    int j; // [esp+0h] [ebp-40h]
    client_t *client; // [esp+4h] [ebp-3Ch]
    bool translationForReason; // [esp+Bh] [ebp-35h]
    msg_t msg; // [esp+Ch] [ebp-34h] BYREF
    int i; // [esp+3Ch] [ebp-4h]

    translationForReason = SEH_StringEd_GetString((char*)message) != 0;
    for ( j = 0; j < 2; ++j )
    {
        i = 0;
        client = svs.clients;
        while ( i < com_maxclients->current.integer )
        {
            if ( client->header.state >= CS_CONNECTED )
            {
                if ( client->header.netchan.remoteAddress.type != NA_LOOPBACK )
                    SV_SendDisconnect(client, client->header.state, message, translationForReason, client->name);
                client->nextSnapshotTime = -1;
                client->lastSnapshotTime = -1;
                SV_SetServerStaticHeader();
                SV_BeginClientSnapshot(client, &msg);
                if ( client->header.state == CS_ACTIVE || client->header.state == CS_ZOMBIE )
                    SV_WriteSnapshotToClient(client, &msg);
                SV_EndClientSnapshot(client, &msg);
                SV_GetServerStaticHeader();
            }
            ++i;
            ++client;
        }
    }
}

void __cdecl SV_ClearServerThreadOwnsGame()
{
    _InterlockedExchange(&sv_thread_owns_game, 0);
}

void __cdecl SV_IncServerThreadOwnsGame()
{
    _InterlockedExchangeAdd(&sv_thread_owns_game, 1u);
}

void __cdecl SV_DecServerThreadOwnsGame()
{
    if ( sv_thread_owns_game <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_init_mp.cpp",
                    2163,
                    0,
                    "%s",
                    "sv_thread_owns_game > 0") )
    {
        __debugbreak();
    }
    _InterlockedExchangeAdd(&sv_thread_owns_game, 0xFFFFFFFF);
}

int __cdecl SV_GetServerThreadOwnsGame()
{
    return sv_thread_owns_game;
}

void __cdecl SV_CheckThread()
{
    if ( SV_GetServerThreadOwnsGame() <= 0 )
    {
        if ( !Sys_IsMainThread()
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_init_mp.cpp",
                        2190,
                        0,
                        "%s",
                        "Sys_IsMainThread()") )
        {
            __debugbreak();
        }
    }
    else if ( !Sys_IsServerThread()
                 && !Assert_MyHandler(
                             "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_init_mp.cpp",
                             2188,
                             0,
                             "%s",
                             "Sys_IsServerThread()") )
    {
        __debugbreak();
    }
}

