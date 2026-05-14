#include "sv_ccmds_mp.h"
#include <qcommon/files.h>
#include <server/server.h>
#include "sv_main_mp.h"
#include <client_mp/g_client_mp.h>
#include <client_mp/sv_client_mp.h>
#include <qcommon/mem_track.h>
#include <server/sv_game.h>
#include <universal/com_files.h>
#include "sv_init_mp.h"
#include <live/live_storage_win.h>
#include "sv_main_pc_mp.h"
#include <game_mp/g_cmds_mp.h>
#include <monkey/monkey.h>
#include <qcommon/dvar_cmds.h>
#include <clientscript/cscr_memorytree.h>
#include <bgame/bg_perks.h>
#include <game_mp/g_client_script_cmd_mp.h>
#include <ui/ui_main_pc.h>
#include <ui/ui_playlists.h>
#include <live/live_storage_pub.h>
#include <universal/q_parse.h>
#include <game_mp/g_main_mp.h>

int sv_migrate;

char *__cdecl SV_GetMapBaseName(char *mapname)
{
    return FS_GetMapBaseName(mapname);
}

void __cdecl SV_ReconnectClients(int savepersist)
{
    char *v1; // eax
    client_t *client; // [esp+0h] [ebp-Ch]
    const char *denied; // [esp+4h] [ebp-8h]
    signed int i; // [esp+8h] [ebp-4h]

    for ( i = 0; i < com_maxclients->current.integer; ++i )
    {
        client = &svs.clients[i];
        if ( client->header.state >= CS_CONNECTED )
        {
            v1 = va("%c", savepersist != 0 ? 110 : 66);
            SV_AddServerCommand(client, SV_CMD_RELIABLE, v1);
            denied = ClientConnect(i, client->scriptId);
            if ( denied )
            {
                SV_DropClient(client, denied, 1, 1);
                Com_Printf(0, "SV_MapRestart_f: dropped client %i - denied!\n", i);
            }
            else if ( client->header.state == CS_ACTIVE )
            {
                SV_ClientEnterWorld(client, &client->lastUsercmd);
            }
        }
    }
}

void __cdecl SV_MapRestart(int fast_restart)
{
    const char *String; // eax
    client_t *client; // [esp+20h] [ebp-58h]
    int i; // [esp+24h] [ebp-54h]
    int savepersist; // [esp+28h] [ebp-50h]
    char mapname[68]; // [esp+30h] [ebp-48h] BYREF

    PROF_SCOPED("SV_MapRestart");

    Com_SyncThreads();
    track_hunk_ClearToStart();
    if ( com_sv_running->current.enabled )
    {
        if ( !fast_restart && Demo_IsRecording() )
            Demo_End(0);
        SV_SetGametype();
        I_strncpyz(sv.gametype, sv_gametype->current.string, 64);
        savepersist = G_GetSavePersist();
        if (com_maxclients->modified || I_stricmp(sv.gametype, sv_gametype->current.string) || !fast_restart)
        {
            G_SetSavePersist(0);
            String = Dvar_GetString("mapname");
            I_strncpyz(mapname, String, 64);
            FS_ConvertPath(mapname);
            SV_SpawnServer(0, mapname, 0, 0);
        }
        if ( com_frameTime != sv.start_frameTime )
        {
            for ( i = 0; i < com_maxclients->current.integer; ++i )
            {
                client = &svs.clients[i];
                if ( client->header.state >= CS_CONNECTED )
                {
                    if ( savepersist || !client->bIsTestClient )
                        NET_OutOfBandPrint(NS_SERVER, client->header.netchan.remoteAddress, "fastrestart");
                    else
                        SV_DropClient(client, "EXE_PLAYERKICKED", 1, 1);
                }
            }
            SV_InitArchivedSnapshot();
            SV_InitSnapshot();
            svs.snapFlagServerBit ^= 4u;
            sv_serverId_value = (((_BYTE)sv_serverId_value + 1) & 0xF) + (sv_serverId_value & 0xF0);
            sv.start_frameTime = com_frameTime;
            sv.state = SS_LOADING;
            sv.restarting = 1;
            SV_RestartGameProgs(savepersist);
        }
    }
    else
    {
        Com_Printf(0, "Server is not running.\n");
    }
}

void __cdecl SV_RankUpClient(client_t *client)
{
    SV_CACValidate_SetIntStat(client->globalStats, "RANKXP", 0x1343A4u);
    SV_CACValidate_SetIntStat(client->globalStats, "CODPOINTS", 0x186A0u);
    SV_CACValidate_SetIntStat(client->globalStats, "PLEVEL", 0xFu);
}

void __cdecl SV_Heartbeat_f()
{
    svs.nextHeartbeatTime = 0x80000000;
}

cmd_function_s SV_Heartbeat_f_VAR;
cmd_function_s SV_Heartbeat_f_VAR_SERVER;
cmd_function_s SV_Drop_f_VAR;
cmd_function_s SV_Drop_f_VAR_SERVER;
cmd_function_s SV_Ban_f_VAR;
cmd_function_s SV_Ban_f_VAR_SERVER;
cmd_function_s SV_BanNum_f_VAR;
cmd_function_s SV_BanNum_f_VAR_SERVER;
cmd_function_s SV_TempBan_f_VAR_0;
cmd_function_s SV_TempBan_f_VAR_SERVER_0;
cmd_function_s SV_TempBan_f_VAR;
cmd_function_s SV_TempBan_f_VAR_SERVER;
cmd_function_s SV_TempBanNum_f_VAR;
cmd_function_s SV_TempBanNum_f_VAR_SERVER;
cmd_function_s SV_Unban_f_VAR;
cmd_function_s SV_Unban_f_VAR_SERVER;
cmd_function_s SV_DropNum_f_VAR;
cmd_function_s SV_DropNum_f_VAR_SERVER;
cmd_function_s SV_Status_f_VAR;
cmd_function_s SV_Status_f_VAR_SERVER;
cmd_function_s SV_TeamStatus_f_VAR;
cmd_function_s SV_TeamStatus_f_VAR_SERVER;
cmd_function_s SV_Serverinfo_f_VAR;
cmd_function_s SV_Serverinfo_f_VAR_SERVER;
cmd_function_s SV_Systeminfo_f_VAR;
cmd_function_s SV_Systeminfo_f_VAR_SERVER;
cmd_function_s SV_DumpUser_f_VAR;
cmd_function_s SV_DumpUser_f_VAR_SERVER;
cmd_function_s SV_MapRestart_f_VAR;
cmd_function_s SV_MapRestart_f_VAR_SERVER;
cmd_function_s SV_FastRestart_f_VAR;
cmd_function_s SV_FastRestart_f_VAR_SERVER;
cmd_function_s SV_Map_f_VAR_0;
cmd_function_s SV_Map_f_VAR_SERVER_0;
cmd_function_s SV_MapRotate_f_VAR;
cmd_function_s SV_MapRotate_f_VAR_SERVER;
cmd_function_s SV_GameCompleteStatus_f_VAR;
cmd_function_s SV_GameCompleteStatus_f_VAR_SERVER;
cmd_function_s SV_Map_f_VAR;
cmd_function_s SV_Map_f_VAR_SERVER;
cmd_function_s SV_KillServer_f_VAR;
cmd_function_s SV_KillServer_f_VAR_SERVER;
cmd_function_s SV_ScriptUsage_f_VAR;
cmd_function_s SV_ScriptUsage_f_VAR_SERVER;
cmd_function_s SV_StringUsage_f_VAR;
cmd_function_s SV_StringUsage_f_VAR_SERVER;
cmd_function_s SV_SetPerk_f_VAR;
cmd_function_s SV_SetPerk_f_VAR_SERVER;
cmd_function_s SV_SysLog_LogMessage_f_VAR;
cmd_function_s SV_SysLog_LogMessage_f_VAR_SERVER;
cmd_function_s SV_RegisterRconKey_f_VAR;
cmd_function_s SV_RegisterRconKey_f_VAR_SERVER;
cmd_function_s SV_RankUp_f_VAR;
cmd_function_s SV_RankUp_f_VAR_SERVER;

static bool initialized_0 = 0;
void __cdecl SV_AddOperatorCommands()
{
    if ( !initialized_0 )
    {
        initialized_0 = 1;
        Cmd_AddCommandInternal("heartbeat", Cbuf_AddServerText_f, &SV_Heartbeat_f_VAR);
        Cmd_AddServerCommandInternal("heartbeat", SV_Heartbeat_f, &SV_Heartbeat_f_VAR_SERVER);
        Cmd_AddCommandInternal("onlykick", Cbuf_AddServerText_f, &SV_Drop_f_VAR);
        Cmd_AddServerCommandInternal("onlykick", SV_Drop_f, &SV_Drop_f_VAR_SERVER);
        Cmd_AddCommandInternal("banUser", Cbuf_AddServerText_f, &SV_Ban_f_VAR);
        Cmd_AddServerCommandInternal("banUser", SV_Ban_f, &SV_Ban_f_VAR_SERVER);
        Cmd_AddCommandInternal("banClient", Cbuf_AddServerText_f, &SV_BanNum_f_VAR);
        Cmd_AddServerCommandInternal("banClient", SV_BanNum_f, &SV_BanNum_f_VAR_SERVER);
        Cmd_AddCommandInternal("kick", Cbuf_AddServerText_f, &SV_TempBan_f_VAR_0);
        Cmd_AddServerCommandInternal("kick", SV_TempBan_f, &SV_TempBan_f_VAR_SERVER_0);
        Cmd_AddCommandInternal("tempBanUser", Cbuf_AddServerText_f, &SV_TempBan_f_VAR);
        Cmd_AddServerCommandInternal("tempBanUser", SV_TempBan_f, &SV_TempBan_f_VAR_SERVER);
        Cmd_AddCommandInternal("tempBanClient", Cbuf_AddServerText_f, &SV_TempBanNum_f_VAR);
        Cmd_AddServerCommandInternal("tempBanClient", SV_TempBanNum_f, &SV_TempBanNum_f_VAR_SERVER);
        Cmd_AddCommandInternal("unbanUser", Cbuf_AddServerText_f, &SV_Unban_f_VAR);
        Cmd_AddServerCommandInternal("unbanUser", SV_Unban_f, &SV_Unban_f_VAR_SERVER);
        Cmd_AddCommandInternal("clientkick", Cbuf_AddServerText_f, &SV_DropNum_f_VAR);
        Cmd_AddServerCommandInternal("clientkick", SV_DropNum_f, &SV_DropNum_f_VAR_SERVER);
        Cmd_AddCommandInternal("status", Cbuf_AddServerText_f, &SV_Status_f_VAR);
        Cmd_AddServerCommandInternal("status", SV_Status_f, &SV_Status_f_VAR_SERVER);
        Cmd_AddCommandInternal("teamstatus", Cbuf_AddServerText_f, &SV_TeamStatus_f_VAR);
        Cmd_AddServerCommandInternal("teamstatus", SV_TeamStatus_f, &SV_TeamStatus_f_VAR_SERVER);
        Cmd_AddCommandInternal("serverinfo", Cbuf_AddServerText_f, &SV_Serverinfo_f_VAR);
        Cmd_AddServerCommandInternal("serverinfo", SV_Serverinfo_f, &SV_Serverinfo_f_VAR_SERVER);
        Cmd_AddCommandInternal("systeminfo", Cbuf_AddServerText_f, &SV_Systeminfo_f_VAR);
        Cmd_AddServerCommandInternal("systeminfo", SV_Systeminfo_f, &SV_Systeminfo_f_VAR_SERVER);
        Cmd_AddCommandInternal("dumpuser", Cbuf_AddServerText_f, &SV_DumpUser_f_VAR);
        Cmd_AddServerCommandInternal("dumpuser", SV_DumpUser_f, &SV_DumpUser_f_VAR_SERVER);
        Cmd_AddCommandInternal("map_restart", Cbuf_AddServerText_f, &SV_MapRestart_f_VAR);
        Cmd_AddServerCommandInternal("map_restart", SV_MapRestart_f, &SV_MapRestart_f_VAR_SERVER);
        Cmd_AddCommandInternal("fast_restart", Cbuf_AddServerText_f, &SV_FastRestart_f_VAR);
        Cmd_AddServerCommandInternal("fast_restart", SV_FastRestart_f, &SV_FastRestart_f_VAR_SERVER);
        Cmd_AddCommandInternal("map", Cbuf_AddServerText_f, &SV_Map_f_VAR_0);
        Cmd_AddServerCommandInternal("map", SV_Map_f, &SV_Map_f_VAR_SERVER_0);
        Cmd_SetAutoComplete("map", "maps/mp", "d3dbsp");
        Cmd_AddCommandInternal("map_rotate", Cbuf_AddServerText_f, &SV_MapRotate_f_VAR);
        Cmd_AddServerCommandInternal("map_rotate", SV_MapRotate_f, &SV_MapRotate_f_VAR_SERVER);
        Cmd_AddCommandInternal("gameCompleteStatus", Cbuf_AddServerText_f, &SV_GameCompleteStatus_f_VAR);
        Cmd_AddServerCommandInternal("gameCompleteStatus", BLOPS_NULLSUB, &SV_GameCompleteStatus_f_VAR_SERVER);
        Cmd_AddCommandInternal("devmap", Cbuf_AddServerText_f, &SV_Map_f_VAR);
        Cmd_AddServerCommandInternal("devmap", SV_Map_f, &SV_Map_f_VAR_SERVER);
        Cmd_SetAutoComplete("devmap", "maps/mp", "d3dbsp");
        Demo_RegisterCommands();
        Cmd_AddCommandInternal("killserver", Cbuf_AddServerText_f, &SV_KillServer_f_VAR);
        Cmd_AddServerCommandInternal("killserver", SV_KillServer_f, &SV_KillServer_f_VAR_SERVER);
        if (IsDedicatedServer())
        {
            SV_AddDedicatedCommands();
        }
        Cmd_AddCommandInternal("scriptUsage", Cbuf_AddServerText_f, &SV_ScriptUsage_f_VAR);
        Cmd_AddServerCommandInternal("scriptUsage", SV_ScriptUsage_f, &SV_ScriptUsage_f_VAR_SERVER);
        Cmd_AddCommandInternal("stringUsage", Cbuf_AddServerText_f, &SV_StringUsage_f_VAR);
        Cmd_AddServerCommandInternal("stringUsage", SV_StringUsage_f, &SV_StringUsage_f_VAR_SERVER);
        Cmd_AddCommandInternal("setPerk", Cbuf_AddServerText_f, &SV_SetPerk_f_VAR);
        Cmd_AddServerCommandInternal("setPerk", SV_SetPerk_f, &SV_SetPerk_f_VAR_SERVER);
        Cmd_AddCommandInternal("logmessage", Cbuf_AddServerText_f, &SV_SysLog_LogMessage_f_VAR);
        Cmd_AddServerCommandInternal("logmessage", SV_SysLog_LogMessage_f, &SV_SysLog_LogMessage_f_VAR_SERVER);
        Cmd_AddCommandInternal("setrconkey", Cbuf_AddServerText_f, &SV_RegisterRconKey_f_VAR);
        Cmd_AddServerCommandInternal("setrconkey", SV_RegisterRconKey_f, &SV_RegisterRconKey_f_VAR_SERVER);
        Cmd_AddCommandInternal("rankup", Cbuf_AddServerText_f, &SV_RankUp_f_VAR);
        Cmd_AddServerCommandInternal("rankup", SV_RankUp_f, &SV_RankUp_f_VAR_SERVER);
    }
}

void __cdecl SV_Map_f()
{
    bool v2; // [esp+0h] [ebp-60h]
    bool isDevmap; // [esp+7h] [ebp-59h]
    char *map; // [esp+8h] [ebp-58h]
    const dvar_s *cow; // [esp+Ch] [ebp-54h]
    char mapname[64]; // [esp+10h] [ebp-50h] BYREF
    bool mapIsPreloaded; // [esp+56h] [ebp-Ah]
    bool cheat; // [esp+57h] [ebp-9h]
    const char *basename; // [esp+58h] [ebp-8h]
    const char *cmd; // [esp+5Ch] [ebp-4h]

    map = (char *)SV_Cmd_Argv(1);

    iassert(map);

    if (!map[0])
    {
        return;
    }

    if ( SV_Cmd_Argc() <= 2 )
    {
        mapIsPreloaded = 0;
    }
    else
    {
        mapIsPreloaded = atoi(SV_Cmd_Argv(2)) != 0;
    }

    if ( SV_Cmd_Argc() <= 3 )
    {
        sv_migrate = 0;
    }
    else
    {
        sv_migrate = atoi(SV_Cmd_Argv(3));
    }

    com_errorPrintsCount = 0;

    if (!IsDedicatedServer())
    {
        Cbuf_ExecuteBuffer(0, 0, (char*)"selectStringTableEntryInDvar mp/didyouknow.csv 0 didyouknow");
    }

    basename = SV_GetMapBaseName(map);

    I_strncpyz(mapname, basename, 64);
    I_strlwr(mapname);

// LWSS: IDA got this totally wrong. The goto logic is wrong and causes SV_SpawnServer() to be called in an infinite loop
//    if ( !useFastFile->current.enabled )
//    {
//        if ( !SV_CheckMapExists(mapname) )
//        {
//            Com_PrintError(1, "Can't find map \"%s\".\n", mapname);
//            return;
//        }
//LABEL_18:
//        if ( Demo_IsRecording() )
//            Demo_End(0);
//        cmd = SV_Cmd_Argv(0);
//        isDevmap = I_stricmp(cmd, "devmap") == 0;
//        cheat = com_developer->current.integer == 2;
//        cow = Dvar_FindVar("thereisacow");
//        if ( !cow || atoi(cow->current.string) != 1960 )
//            cheat = 0;
//        v2 = isDevmap || cheat;
//        cheat = v2;
//        Dvar_SetBool((dvar_s *)sv_cheats, v2);
//        FS_ConvertPath(mapname);
//        SV_SpawnServer(0, mapname, mapIsPreloaded, sv_migrate);
//    }
//    if ( DB_FileExists(mapname, FFD_DEFAULT) || DB_FileExists(mapname, FFD_USER_MAP) )
//        goto LABEL_18;
//    Com_PrintError(1, "Can't find map \"%s\" in usermaps\\%s folder.\n", mapname, mapname);

    if (!IsFastFileLoad())
    {
        if (!SV_CheckMapExists(mapname))
        {
            Com_PrintError(1, "Can't find map \"%s\".\n", mapname);
            return;
        }
    }
    else
    {
        if (!DB_FileExists(mapname, FFD_DEFAULT) && !DB_FileExists(mapname, FFD_USER_MAP))
        {
            Com_PrintError(1, "Can't find map \"%s\" in usermaps\\%s folder.\n", mapname, mapname);
            return;
        }
    }

    if (Demo_IsRecording())
        Demo_End(0);

    cmd = SV_Cmd_Argv(0);
    isDevmap = I_stricmp(cmd, "devmap") == 0;

    cheat = (com_developer->current.integer == 2);

    cow = Dvar_FindVar("thereisacow");
    if (!cow || atoi(cow->current.string) != 1960)
        cheat = 0;

    cheat = isDevmap || cheat;

    Dvar_SetBool((dvar_s*)sv_cheats, cheat);

    FS_ConvertPath(mapname);
    SV_SpawnServer(0, mapname, mapIsPreloaded, sv_migrate);
}

char __cdecl SV_CheckMapExists(const char *map)
{
    const char *v1; // eax
    char expanded[68]; // [esp+0h] [ebp-48h] BYREF

    Com_GetBspFilename(expanded, 0x40u, map);
    if ( FS_ReadFile(expanded, 0) != -1 )
        return 1;
    Com_PrintError(1, "Can't find map %s\n", expanded);
    if ( Monkey_IsRunning() )
    {
        v1 = va("Can't find map %s\n", expanded);
        if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_ccmds_mp.cpp", 81, 0, v1) )
            __debugbreak();
    }
    return 0;
}

void __cdecl ShowLoadErrorsSummary(const char *mapName, unsigned int count)
{
    if ( !Monkey_IsRunning() && com_errorPrintsCount )
    {
        if ( count == 1 )
            Com_PrintError(16, (char *)"%s - There was %u error when loading this map See the console log for details", mapName, com_errorPrintsCount);
        else
            Com_PrintError(16, (char *)"%s - There were %u errors when loading this map. See the console log for details", mapName, com_errorPrintsCount);
    }
}

void __cdecl SV_MapRestart_f()
{
    SV_MapRestart(0);
}

void __cdecl SV_FastRestart_f()
{
    SV_MapRestart(1);
}

void __cdecl SV_MapRotate_f()
{
    int LicenseType; // eax
    const char *String; // eax
    const char *v2; // eax
    const char *v3; // eax
    char *v4; // eax
    char *v5; // eax
    const char *v6; // eax
    char *v7; // eax
    parseInfo_t *token; // [esp+0h] [ebp-4h]
    parseInfo_t *tokena; // [esp+0h] [ebp-4h]
    parseInfo_t *tokenb; // [esp+0h] [ebp-4h]
    parseInfo_t *tokenc; // [esp+0h] [ebp-4h]

    Com_Printf(0, "\n\nmap_rotate...\n\n");
    LicenseType = SV_GetLicenseType();
    if ( SV_IsServerRanked(LicenseType) )
        Dvar_SetBoolByName("playlist_enabled", 1);
    if ( LiveStorage_FetchingOnlineWAD() )
    {
        Com_PrintWarning(0, "Early out of maprotate, waiting for WAD!\n");
        SV_SetShouldMapRotate(1);
        return;
    }
    if ( Dvar_GetBool("playlist_enabled") )
    {
        if ( !LiveStorage_DoWeHavePlaylists() )
        {
            Com_Printf(0, "Early out of maprotate, waiting for playlist!\n");
            SV_SetShouldMapRotate(1);
            return;
        }
        String = Dvar_GetString("playlist_excludeMap");
        Com_Printf(0, "\"playlist_excludeMap\" is:\"%s\"\n", String);
        v2 = Dvar_GetString("playlist_excludeGametype");
        Com_Printf(0, "\"playlist_excludeGametype\" is:\"%s\"\n", v2);
        v3 = Dvar_GetString("playlist_excludeGametypeMap");
        Com_Printf(0, "\"playlist_excludeGametypeMap\" is:\"%s\"\n\n", v3);
        Playlist_SetSVMapRotation();
        Playlist_SVMapRotate();
    }
    Com_Printf(0, "\"sv_mapRotation\" is:\"%s\"\n\n", sv_mapRotation->current.string);
    if ( !*(_BYTE *)sv_mapRotationCurrent->current.integer )
        Dvar_SetString((dvar_s *)sv_mapRotationCurrent, sv_mapRotation->current.string);
    Com_Printf(0, "\"sv_mapRotationCurrent\" is:\"%s\"\n\n", sv_mapRotationCurrent->current.string);
    token = UI_GetMapRotationToken();
    if ( !token )
    {
        Dvar_SetString((dvar_s *)sv_mapRotationCurrent, sv_mapRotation->current.string);
        token = UI_GetMapRotationToken();
    }
    while ( 1 )
    {
        if ( !token )
        {
            Com_Printf(0, "No map specified in sv_mapRotation - forcing map_restart.\n");
            SV_FastRestart_f();
            return;
        }
        if ( !I_stricmp(token->token, "gametype") )
        {
            tokena = UI_GetMapRotationToken();
            if ( !tokena )
            {
                Com_Printf(0, "No gametype specified after 'gametype' keyword in sv_mapRotation - forcing map_restart.\n");
                SV_FastRestart_f();
                return;
            }
            Com_Printf(0, "Setting g_gametype: %s.\n", tokena->token);
            if ( com_sv_running->current.enabled )
            {
                if ( I_stricmp(sv_gametype->current.string, tokena->token) )
                    G_SetSavePersist(0);
            }
            Dvar_SetString((dvar_s *)sv_gametype, tokena->token);
            goto LABEL_33;
        }
        if ( !I_stricmp(token->token, "map") )
            break;
        if ( I_stricmp(token->token, "arena") )
        {
            if ( I_stricmp(token->token, "nextarena") )
            {
                Com_Printf(0, "Unknown keyword '%s' in sv_mapRotation.\n", token->token);
            }
            else
            {
                Com_Printf(0, "nextarena executing arena: %s.\n", token->token);
                v6 = Dvar_GetString("nextarena");
                v7 = va("exec %s\n", v6);
                Cmd_ExecuteSingleCommand(0, 0, v7);
                Dvar_SetString((dvar_s *)sv_mapRotationCurrent, sv_mapRotation->current.string);
                Com_Printf(0, "\"sv_mapRotationCurrent\" is:\"%s\"\n\n", sv_mapRotationCurrent->current.string);
            }
        }
        else
        {
            tokenc = UI_GetMapRotationToken();
            if ( !tokenc )
            {
                Com_Printf(0, "No arena specified after 'arena' keyword in sv_mapRotation - forcing map_restart.\n");
                SV_FastRestart_f();
                return;
            }
            Com_Printf(0, "Setting arena: %s.\n", tokenc->token);
            v5 = va("exec %s\n", tokenc->token);
            Cmd_ExecuteSingleCommand(0, 0, v5);
        }
LABEL_33:
        token = UI_GetMapRotationToken();
    }
    tokenb = UI_GetMapRotationToken();
    if ( tokenb )
    {
        Com_Printf(0, "Setting map: %s.\n", tokenb->token);
        v4 = va("map %s\n", tokenb->token);
        Cmd_ExecuteSingleCommand(0, 0, v4);
    }
    else
    {
        Com_Printf(0, "No map specified after 'map' keyword in sv_mapRotation - forcing map_restart.\n");
        SV_FastRestart_f();
    }
}

void __cdecl SV_TempBan_f()
{
    char playerName[64]; // [esp+0h] [ebp-48h] BYREF
    int guid; // [esp+44h] [ebp-4h]

    guid = SV_KickUser_f(playerName, 64);
    if ( guid )
    {
        Com_Printf(0, "%s (guid %i) was kicked.\n", playerName, guid);
        SV_BanGuidBriefly(guid);
    }
}

int __cdecl SV_KickUser_f(char *playerName, int maxPlayerNameLen)
{
    const char *v3; // eax
    client_t *PlayerByName; // [esp+0h] [ebp-14h]
    client_t *clients; // [esp+0h] [ebp-14h]
    const char *reason; // [esp+4h] [ebp-10h]
    int clientNum; // [esp+8h] [ebp-Ch]
    const char *cmdName; // [esp+Ch] [ebp-8h]
    int argc; // [esp+10h] [ebp-4h]

    if ( com_sv_running->current.enabled )
    {
        argc = SV_Cmd_Argc();
        if ( argc >= 2 )
        {
            if ( argc == 3 )
                reason = SV_Cmd_Argv(2);
            else
                reason = "EXE_PLAYERKICKED";
            PlayerByName = SV_GetPlayerByName();
            if ( PlayerByName )
            {
                return SV_KickClient(PlayerByName, playerName, maxPlayerNameLen, reason);
            }
            else
            {
                v3 = SV_Cmd_Argv(1);
                if ( !I_stricmp(v3, "all") )
                {
                    clientNum = 0;
                    clients = svs.clients;
                    while ( clientNum < com_maxclients->current.integer )
                    {
                        if ( clients->header.state != CS_FREE )
                        {
                            if ( !clients->bIsDemoClient )
                                SV_KickClient(clients, 0, 0, reason);
                        }
                        ++clientNum;
                        ++clients;
                    }
                }
                return 0;
            }
        }
        else
        {
            cmdName = SV_Cmd_Argv(0);
            Com_Printf(0, "Usage: %s <player name> <optional reason>\n%s all = kick everyone\n", cmdName, cmdName);
            return 0;
        }
    }
    else
    {
        Com_Printf(0, "Server is not running.\n");
        return 0;
    }
}

client_t *__cdecl SV_GetPlayerByName()
{
    client_t *clients; // [esp+0h] [ebp-54h]
    const char *s; // [esp+4h] [ebp-50h]
    int i; // [esp+8h] [ebp-4Ch]
    char cleanName[68]; // [esp+Ch] [ebp-48h] BYREF

    if ( !com_sv_running->current.enabled )
        return 0;
    if ( SV_Cmd_Argc() >= 2 )
    {
        s = SV_Cmd_Argv(1);
        i = 0;
        clients = svs.clients;
        while ( i < com_maxclients->current.integer )
        {
            if ( clients->header.state != CS_FREE )
            {
                if ( !I_stricmp(clients->name, s) )
                    return clients;
                I_strncpyz(cleanName, clients->name, 64);
                I_CleanStr(cleanName);
                if ( !I_stricmp(cleanName, s) )
                    return clients;
            }
            ++i;
            ++clients;
        }
        Com_Printf(0, "Player %s is not on the server\n", s);
        return 0;
    }
    else
    {
        Com_Printf(0, "No player specified.\n");
        return 0;
    }
}

int __cdecl SV_KickClient(client_t *cl, char *playerName, int maxPlayerNameLen, const char *reason)
{
    int guid; // [esp+0h] [ebp-4h]

    if ( !cl && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_ccmds_mp.cpp", 758, 0, "%s", "cl") )
        __debugbreak();
    if ( cl->header.netchan.remoteAddress.type == NA_LOOPBACK )
    {
        SV_SendServerCommand(0, SV_CMD_CAN_IGNORE, "%c \"EXE_CANNOTKICKHOSTPLAYER\"", 101);
        return 0;
    }
    else
    {
        if ( playerName )
        {
            I_strncpyz(playerName, cl->name, maxPlayerNameLen);
            I_CleanStr(playerName);
        }
        guid = cl->guid;
        SV_DropClient(cl, reason, 1, 1);
        cl->lastPacketTime = svs.time;
        return guid;
    }
}

void __cdecl SV_Ban_f()
{
    client_t *PlayerByName; // [esp+0h] [ebp-4h]

    if ( com_sv_running->current.enabled )
    {
        if ( SV_Cmd_Argc() == 2 )
        {
            PlayerByName = SV_GetPlayerByName();
            if ( PlayerByName )
                SV_BanClient(PlayerByName);
        }
        else
        {
            Com_Printf(0, "Usage: banUser <player name>\n");
        }
    }
    else
    {
        Com_Printf(0, "Server is not running.\n");
    }
}

void __cdecl SV_RankUp_f()
{
    const char *v0; // eax
    client_t *client; // [esp+0h] [ebp-4h]

    if ( com_sv_running->current.enabled )
    {
        if ( SV_Cmd_Argc() == 2 )
        {
            client = SV_GetPlayerByName();
            if ( client )
            {
                SV_RankUpClient(client);
                SV_DWWriteClientStats(client);
            }
            else
            {
                v0 = SV_Cmd_Argv(1);
                Com_Printf(0, "Couldn't find user %s\n", v0);
            }
        }
        else
        {
            Com_Printf(0, "Usage: rankup <player name>\n");
        }
    }
    else
    {
        Com_Printf(0, "Server is not running.\n");
    }
}

void __cdecl SV_BanNum_f()
{
    client_t *PlayerByNum; // [esp+0h] [ebp-4h]

    if ( com_sv_running->current.enabled )
    {
        if ( SV_Cmd_Argc() == 2 )
        {
            PlayerByNum = SV_GetPlayerByNum();
            if ( PlayerByNum )
                SV_BanClient(PlayerByNum);
        }
        else
        {
            Com_Printf(0, "Usage: banClient <client number>\n");
        }
    }
    else
    {
        Com_Printf(0, "Server is not running.\n");
    }
}

client_t *__cdecl SV_GetPlayerByNum()
{
    int idnum; // [esp+4h] [ebp-Ch]
    const char *s; // [esp+8h] [ebp-8h]
    int i; // [esp+Ch] [ebp-4h]

    if ( !com_sv_running->current.enabled )
        return 0;
    if ( SV_Cmd_Argc() >= 2 )
    {
        s = SV_Cmd_Argv(1);
        for ( i = 0; s[i]; ++i )
        {
            if ( s[i] < 48 || s[i] > 57 )
            {
                Com_Printf(0, "Bad slot number: %s\n", s);
                return 0;
            }
        }
        idnum = atoi(s);
        if ( idnum >= 0 && idnum < com_maxclients->current.integer )
        {
            if ( svs.clients[idnum].header.state != CS_FREE )
            {
                return &svs.clients[idnum];
            }
            else
            {
                Com_Printf(0, "Client %i is not active\n", idnum);
                return 0;
            }
        }
        else
        {
            Com_Printf(0, "Bad client slot: %i\n", idnum);
            return 0;
        }
    }
    else
    {
        Com_Printf(0, "No player specified.\n");
        return 0;
    }
}

void __cdecl SV_Unban_f()
{
    const char *v0; // eax

    if ( SV_Cmd_Argc() == 2 )
    {
        v0 = SV_Cmd_Argv(1);
        SV_UnbanClient(v0);
    }
    else
    {
        Com_Printf(0, "Usage: unban <client name>\n");
    }
}

void __cdecl SV_Drop_f()
{
    SV_KickUser_f(0, 0);
}

void __cdecl SV_DropNum_f()
{
    SV_KickClient_f(0, 0);
}

int __cdecl SV_KickClient_f(char *playerName, int maxPlayerNameLen)
{
    const char *v3; // eax
    client_t *PlayerByNum; // [esp+0h] [ebp-Ch]
    const char *reason; // [esp+4h] [ebp-8h]
    int argc; // [esp+8h] [ebp-4h]

    if ( com_sv_running->current.enabled )
    {
        argc = SV_Cmd_Argc();
        if ( argc >= 2 )
        {
            if ( argc == 3 )
                reason = SV_Cmd_Argv(2);
            else
                reason = "EXE_PLAYERKICKED";
            PlayerByNum = SV_GetPlayerByNum();
            if ( PlayerByNum )
                return SV_KickClient(PlayerByNum, playerName, maxPlayerNameLen, reason);
            else
                return 0;
        }
        else
        {
            v3 = SV_Cmd_Argv(0);
            Com_Printf(0, "Usage: %s <client number> <optional reason>\n", v3);
            return 0;
        }
    }
    else
    {
        Com_Printf(0, "Server is not running.\n");
        return 0;
    }
}

void __cdecl SV_TempBanNum_f()
{
    char playerName[64]; // [esp+0h] [ebp-48h] BYREF
    int guid; // [esp+44h] [ebp-4h]

    guid = SV_KickClient_f(playerName, 64);
    if ( guid )
    {
        Com_Printf(0, "%s (guid %i) was kicked by the server\n", playerName, guid);
        SV_BanGuidBriefly(guid);
    }
}

void __cdecl SV_Status_f()
{
    int ClientScore; // eax
    unsigned int v1; // kr00_4
    int j; // [esp+14h] [ebp-1Ch]
    int ja; // [esp+14h] [ebp-1Ch]
    client_t *clients; // [esp+18h] [ebp-18h]
    int l; // [esp+1Ch] [ebp-14h]
    char *s; // [esp+24h] [ebp-Ch]
    int i; // [esp+28h] [ebp-8h]

    if ( com_sv_running->current.enabled )
    {
        Com_Printf(0, "map: %s\n", sv_mapname->current.string);
        Com_Printf(0, "num score ping guid     name                        lastmsg address                             qport rate\n");
        Com_Printf(0, "--- ----- ---- ---------- --------------- ------- --------------------- ------ -----\n");
        i = 0;
        clients = svs.clients;
        while ( i < com_maxclients->current.integer )
        {
            if ( clients->header.state != CS_FREE )
            {
                Com_Printf(0, "%3i ", i);
                SV_GameClientNum(i);
                ClientScore = G_GetClientScore(clients - svs.clients);
                Com_Printf(0, "%5i ", ClientScore);
                if ( clients->header.state == CS_CONNECTED )
                {
                    Com_Printf(0, "CNCT ");
                }
                else if ( clients->header.state == CS_ZOMBIE )
                {
                    Com_Printf(0, "ZMBI ");
                }
                else if ( clients->ping >= 9999 )
                {
                    Com_Printf(0, "%4i ", 9999);
                }
                else
                {
                    Com_Printf(0, "%4i ", clients->ping);
                }
                Com_Printf(0, "%6i ", clients->guid);
                Com_Printf(0, "%s^7", clients->name);
                l = 16 - I_DrawStrlen(clients->name);
                for ( j = 0; j < l; ++j )
                    Com_Printf(0, " ");
                Com_Printf(0, "%7i ", svs.time - clients->lastPacketTime);
                s = NET_AdrToString(clients->header.netchan.remoteAddress);
                Com_Printf(0, "%s", s);
                v1 = strlen(s);
                for ( ja = 0; ja < (int)(22 - v1); ++ja )
                    Com_Printf(0, " ");
                Com_Printf(0, "%6i", clients->header.netchan.qport);
                Com_Printf(0, " %5i", clients->rate);
                Com_Printf(0, "\n");
            }
            ++i;
            ++clients;
        }
        Com_Printf(0, "\n");
    }
    else
    {
        Com_Printf(0, "Server is not running.\n");
    }
}

void __cdecl SV_TeamStatus_f()
{
    int ClientScore; // eax
    team_t ClientTeam; // eax
    unsigned int v2; // kr00_4
    int j; // [esp+14h] [ebp-1Ch]
    int ja; // [esp+14h] [ebp-1Ch]
    client_t *clients; // [esp+18h] [ebp-18h]
    int l; // [esp+1Ch] [ebp-14h]
    char *s; // [esp+24h] [ebp-Ch]
    int i; // [esp+28h] [ebp-8h]

    if ( com_sv_running->current.enabled )
    {
        Com_Printf(0, "map: %s\n", sv_mapname->current.string);
        Com_Printf(0, "num score ping guid             name                        team lastmsg address                             qport    rate\n");
        Com_Printf(0, "--- ----- ---- ---------- --------------- ---- ------- --------------------- ------ -----\n");
        i = 0;
        clients = svs.clients;
        while ( i < com_maxclients->current.integer )
        {
            if ( clients->header.state != CS_FREE )
            {
                Com_Printf(0, "%3i ", i);
                SV_GameClientNum(i);
                ClientScore = G_GetClientScore(clients - svs.clients);
                Com_Printf(0, "%5i ", ClientScore);
                if ( clients->header.state == CS_CONNECTED )
                {
                    Com_Printf(0, "CNCT ");
                }
                else if ( clients->header.state == CS_ZOMBIE )
                {
                    Com_Printf(0, "ZMBI ");
                }
                else if ( clients->ping >= 9999 )
                {
                    Com_Printf(0, "%4i ", 9999);
                }
                else
                {
                    Com_Printf(0, "%4i ", clients->ping);
                }
                Com_Printf(0, "%6i ", clients->guid);
                Com_Printf(0, "%s^7", clients->name);
                l = 16 - I_DrawStrlen(clients->name);
                for ( j = 0; j < l; ++j )
                    Com_Printf(0, " ");
                ClientTeam = G_GetClientTeam(clients - svs.clients);
                Com_Printf(0, "%4i ", ClientTeam);
                Com_Printf(0, "%7i ", svs.time - clients->lastPacketTime);
                s = NET_AdrToString(clients->header.netchan.remoteAddress);
                Com_Printf(0, "%s", s);
                v2 = strlen(s);
                for ( ja = 0; ja < (int)(22 - v2); ++ja )
                    Com_Printf(0, " ");
                Com_Printf(0, "%6i", clients->header.netchan.qport);
                Com_Printf(0, " %5i", clients->rate);
                Com_Printf(0, "\n");
            }
            ++i;
            ++clients;
        }
        Com_Printf(0, "\n");
    }
    else
    {
        Com_Printf(0, "Server is not running.\n");
    }
}

void __cdecl SV_Serverinfo_f()
{
    char *v0; // eax

    Com_Printf(0, "Server info settings:\n");
    v0 = Dvar_InfoString(0, 4);
    Info_Print(v0);
}

void __cdecl SV_Systeminfo_f()
{
    char *v0; // eax

    Com_Printf(0, "System info settings:\n");
    v0 = Dvar_InfoString(0, 8);
    Info_Print(v0);
}

void __cdecl SV_DumpUser_f()
{
    client_t *PlayerByName; // [esp+0h] [ebp-4h]

    if ( com_sv_running->current.enabled )
    {
        if ( SV_Cmd_Argc() == 2 )
        {
            PlayerByName = SV_GetPlayerByName();
            if ( PlayerByName )
            {
                Com_Printf(0, "userinfo\n");
                Com_Printf(0, "--------\n");
                Info_Print(PlayerByName->userinfo);
            }
        }
        else
        {
            Com_Printf(0, "Usage: info <userid>\n");
        }
    }
    else
    {
        Com_Printf(0, "Server is not running.\n");
    }
}

void __cdecl SV_KillServer_f()
{
    Com_Shutdown("EXE_SERVERKILLED");
}

void __cdecl SV_ScriptUsage_f()
{
    Scr_DumpScriptThreads(SCRIPTINSTANCE_SERVER);
}

void __cdecl SV_StringUsage_f()
{
    MT_DumpTree(SCRIPTINSTANCE_SERVER);
}

void __cdecl SV_SetPerk_f()
{
    clientState_s *ClientState; // eax
    client_t *PlayerByName; // [esp+8h] [ebp-18h]
    const char *perkName; // [esp+Ch] [ebp-14h]
    unsigned int perkIndex; // [esp+10h] [ebp-10h]
    unsigned int i; // [esp+14h] [ebp-Ch]
    playerState_s *ps; // [esp+18h] [ebp-8h]
    client_t *clIdx; // [esp+1Ch] [ebp-4h]

    PlayerByName = SV_GetPlayerByName();
    if ( PlayerByName )
    {
        perkName = SV_Cmd_Argv(2);
        perkIndex = BG_GetPerkIndexForName(perkName);
        if ( perkIndex < 0x34 )
        {
            i = 0;
            for ( clIdx = svs.clients; (signed int)i < com_maxclients->current.integer && clIdx != PlayerByName; ++clIdx )
                ++i;
            if ( i >= com_maxclients->current.integer
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_ccmds_mp.cpp",
                            1627,
                            0,
                            "i doesn't index com_maxclients->current.integer\n\t%i not in [0, %i)",
                            i,
                            com_maxclients->current.integer) )
            {
                __debugbreak();
            }
            ps = SV_GameClientNum(i);
            BG_SetPerk(ps->perks, perkIndex);
            ClientState = G_GetClientState(i);
            BG_SetPerk(ClientState->perks, perkIndex);
        }
        else
        {
            Com_DPrintf(0, "Unknown perk: %s\n", perkName);
        }
    }
}

cmd_function_s SV_ConSay_f_VAR;
cmd_function_s SV_ConSay_f_VAR_SERVER;
cmd_function_s SV_ConTell_f_VAR;
cmd_function_s SV_ConTell_f_VAR_SERVER;

void __cdecl SV_AddDedicatedCommands()
{
    SV_RemoveDedicatedCommands();
    Cmd_AddCommandInternal("say", Cbuf_AddServerText_f, &SV_ConSay_f_VAR);
    Cmd_AddServerCommandInternal("say", SV_ConSay_f, &SV_ConSay_f_VAR_SERVER);
    Cmd_AddCommandInternal("tell", Cbuf_AddServerText_f, &SV_ConTell_f_VAR);
    Cmd_AddServerCommandInternal("tell", SV_ConTell_f, &SV_ConTell_f_VAR_SERVER);
}

void __cdecl SV_ConSay_f()
{
    char text[1028]; // [esp+0h] [ebp-408h] BYREF

    if ( com_sv_running->current.enabled )
    {
        if ( SV_Cmd_Argc() >= 2 )
        {
            SV_AssembleConSayMessage(1, text, 1024);
            SV_SendServerCommand(0, SV_CMD_CAN_IGNORE, "%c \"\x15%s\"", 104, text);
        }
    }
    else
    {
        Com_Printf(0, "Server is not running.\n");
    }
}

void __cdecl SV_AssembleConSayMessage(int firstArg, char *text, int sizeofText)
{
    unsigned int textLen; // [esp+10h] [ebp-4h]

    strcpy(text, "console: ");
    textLen = 9;
    if ( strlen(text) != 9
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_ccmds_mp.cpp",
                    1141,
                    1,
                    "%s",
                    "textLen == strlen( text )") )
    {
        __debugbreak();
    }
    Cmd_ArgsBuffer(firstArg, text + 9, sizeofText - 9);
    if ( text[9] == 34 )
    {
        while ( text[textLen + 1] )
        {
            text[textLen - 1] = text[textLen];
            ++textLen;
        }
        text[textLen] = 0;
    }
}

void __cdecl SV_ConTell_f()
{
    const char *v0; // eax
    client_t *v1; // [esp+0h] [ebp-410h]
    int clientNum; // [esp+4h] [ebp-40Ch]
    char text[1028]; // [esp+8h] [ebp-408h] BYREF

    if ( com_sv_running->current.enabled )
    {
        if ( SV_Cmd_Argc() >= 3 )
        {
            v0 = SV_Cmd_Argv(1);
            clientNum = atoi(v0);
            if ( clientNum >= 0 && clientNum < com_maxclients->current.integer )
            {
                v1 = &svs.clients[clientNum];
                if ( v1->header.state == CS_ACTIVE )
                {
                    SV_AssembleConSayMessage(2, text, 1024);
                    SV_SendServerCommand(v1, SV_CMD_CAN_IGNORE, "%c \"\x15%s\"", 104, text);
                }
            }
        }
    }
    else
    {
        Com_Printf(0, "Server is not running.\n");
    }
}

void __cdecl SV_RemoveDedicatedCommands()
{
    Cmd_RemoveCommand("say");
    Cmd_RemoveCommand("tell");
}

