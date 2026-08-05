#include "sv_client_mp.h"
#include <server_mp/sv_main_mp.h>
#include <live/live_storage_win.h>
#include <server_mp/sv_init_mp.h>
#include <qcommon/com_gamemodes.h>
#include <live/live_stats.h>
#include <universal/mem_largelocal.h>
#include <live/live_steam_server.h>
#include <universal/com_files.h>
#include <universal/q_parse.h>
#include "g_client_mp.h"
#include <game_mp/g_cmds_mp.h>
#include <universal/com_buildinfo.h>
#include <win32/win_net.h>
#include <server_mp/sv_main_pc_mp.h>
#include <demo/demo_recording.h>
#include <game_mp/pregame.h>
#include <server_mp/sv_bot_mp.h>
#include <server_mp/sv_ccmds_mp.h>
#include <stringed/stringed_hooks.h>
#include <DW/MatchRecorder.h>
#include <qcommon/sv_msg_write_mp.h>
#include <server_mp/sv_net_chan_mp.h>
#include <server_mp/sv_snapshot_mp.h>
#include <universal/com_constantconfigstrings.h>
#include <clientscript/cscr_stringlist.h>
#include <qcommon/files.h>
#include <win32/win_shared.h>
#include <qcommon/dvar_cmds.h>
#include <server_mp/sv_snapshot_profile_mp.h>
#include <qcommon/sv_msg_write.h>
#include <physics/rope_gamestate.h>
#include <ui_mp/ui_gametype_custom_mp.h>
#include <glass/glass_server.h>
#include <universal/com_memory.h>
#include <qcommon/com_clients.h>
#include <win32/win_voice.h>
#include <live/live_sessions_win.h>
#include <universal/com_shared.h>
#include <server/sv_game.h>
#include <bgame/bg_weapons.h>
#include <game_mp/g_active_mp.h>
#include <universal/base64.h>
#include <win32/win_steam.h>

struct ucmd_t // sizeof=0xC
{                                       // XREF: .data:ucmds/r
    const char *name;
    void (__cdecl *func)(client_t *);
    int allowFromOldServer;
};

ucmd_t ucmds[13] =
{
  { "userinfo", &SV_UpdateUserinfo_f, 0 },
  { "disconnect", &SV_Disconnect_f, 1 },
  { "cp", &SV_VerifyIwds_f, 0 },
  { "vdr", &SV_ResetPureClient_f, 0 },
  { "download", &SV_BeginDownload_f, 0 },
  { "nextdl", &SV_NextDownload_f, 0 },
  { "stopdl", &SV_StopDownload_f, 0 },
  { "donedl", &SV_DoneDownload_f, 0 },
  { "retransdl", &SV_RetransmitDownload_f, 0 },
  { "wwwdl", &SV_WWWDownload_f, 0 },
  { "muteplayer", &SV_MutePlayer_f, 0 },
  { "unmuteplayer", &SV_UnmutePlayer_f, 0 },
  { NULL, NULL, 0 }
};

int sv_serverId_value;

unsigned __int16 botport;
unsigned __int64 g_notifyLeave[32];

void __cdecl SV_HandleDWChallengeResponse(netadr_t from, msg_t *msg)
{
    client_t *client; // [esp+0h] [ebp-10h]
    unsigned int serverchallenge; // [esp+4h] [ebp-Ch] BYREF
    int qport; // [esp+8h] [ebp-8h]

    if ( msg )
    {
        qport = MSG_ReadShort(msg);
        MSG_ReadData(msg, (unsigned __int8 *)&serverchallenge, 4);
        Com_DPrintf(15, "DWCHALLENGERESPONSE: Read server challenge %u\n", serverchallenge);
        client = SV_FindClientByAddress(from, qport);
        if ( client )
        {
            if ( serverchallenge == client->dwchallenge )
            {
                Com_DPrintf(15, "DWCHALLENGERESPONSE: Matches\n");
                client->dwchallenge = 0;
                client->guid = client->dw_userID;
                SV_DWReadClientCAC(client);
                SV_DWReadClientStats(client);
            }
            else
            {
                Com_DPrintf(
                    15,
                    "DWCHALLENGERESPONSE: serverchallenge %u doesn't match what we sent client: %u. Ask Ewan.\n",
                    serverchallenge,
                    client->dwchallenge);
                NET_OutOfBandPrint(NS_SERVER, client->header.netchan.remoteAddress, "error\nEXE_BAD_CHALLENGE");
            }
        }
    }
}

void __cdecl SV_GetChallenge(netadr_t from)
{
    int v1; // esi
    int time; // [esp+4h] [ebp-18h]
    challenge_t *challenge; // [esp+8h] [ebp-14h]
    int oldest; // [esp+Ch] [ebp-10h]
    int i; // [esp+10h] [ebp-Ch]
    int oldestTime; // [esp+14h] [ebp-8h]

    if ( sv_authenticating->current.enabled )
    {
        Com_Printf(0, "Dedicated server: Rejecting incoming connection, we're re-authing\n");
        return;
    }
    else if ( sv_dwlsgerror->current.enabled )
    {
        Com_PrintWarning(0, "Dedicated server: Session update failure, rejecting incoming connection\n");
        return;
    }

    oldest = 0;
    oldestTime = 0x7FFFFFFF;
    challenge = svs.challenges;
    for ( i = 0; i < 1024 && (challenge->connected || !NET_CompareAdr(from, challenge->adr)); ++i )
    {
        if ( challenge->time < oldestTime )
        {
            oldestTime = challenge->time;
            oldest = i;
        }
        ++challenge;
    }
    if ( i == 1024 )
    {
        challenge = &svs.challenges[oldest];
        v1 = rand() << 16;
        challenge->challenge = svs.time ^ rand() ^ v1;
        challenge->adr = from;
        challenge->firstTime = svs.time;
        challenge->firstPing = 0;
        if ( svs.time <= 0 )
            time = 1;
        else
            time = svs.time;
        challenge->time = time;
        challenge->connected = 0;
    }


    //challenge->pingTime = svs.time;
    //NET_OutOfBandPrint(NS_SERVER, from, va("challengeResponse %i", challenge->challenge));
    
    // LWSS ADD - all new below here
    char *clientSteamTicketBase64 = (char *)SV_Cmd_Argv(2);
    char *clientSteamID64 = (char *)SV_Cmd_Argv(3);
    unsigned char decodedSteamTicket[1024 + 128]{ 0 };

    if (!clientSteamTicketBase64[0] || !clientSteamID64[0])
    {
        iassert(0); // guy didn't send enough information
        return;
    }

    if (strlen(clientSteamTicketBase64) > sizeof(decodedSteamTicket))
    {
        iassert(0);
        return;
    }

    uint32 decodedLen = b64_decode((unsigned char *)clientSteamTicketBase64, strlen(clientSteamTicketBase64), decodedSteamTicket);

    char *endptr;
    uint64_t steamID64 = strtoull(clientSteamID64, &endptr, 10);

    //if (SV_IsBannedGuid(guid)) 
    //{
    //    Com_Printf(15, "rejected connection from permanently banned GUID \"%s\"\n", clientSteamID64);
    //    NET_OutOfBandPrint(NS_SERVER, svs.challenges[i].adr, "error\xA\x15You are permanently banned from this server");
    //    memset(&svs.challenges[i], 0, sizeof(svs.challenges[i]));
    //    return;
    //}
    //if (SV_IsTempBannedGuid(guid)) 
    //{
    //    Com_Printf(15, "rejected connection from temporarily banned GUID \"%s\"\n", clientSteamID64);
    //    NET_OutOfBandPrint(NS_SERVER, svs.challenges[i].adr, "error\xA\x15You are temporarily banned from this server");
    //    memset(&svs.challenges[i], 0, sizeof(svs.challenges[i]));
    //    return;
    //}

    challenge->steamID64 = steamID64;

    if (Steam_CheckClientTicket(decodedSteamTicket, decodedLen, steamID64))
    {
        challenge->pingTime = svs.time;
        NET_OutOfBandPrint(NS_SERVER, from, va("challengeResponse %i", challenge->challenge));
        return;
    }
    else
    {
        Com_Printf(15, "rejected connection from invalid Steam GUID \"%s\"\n", clientSteamID64);
        NET_OutOfBandPrint(NS_SERVER, challenge->adr, "error\xA\x15Your Steam Client Ticket was Invalid");
        memset(challenge, 0, sizeof(challenge_t));
        return;
    }
}

void __cdecl SV_CacheClientStatChange(unsigned int clientNum, ddlState_t *searchState)
{
    int offset; // [esp+4h] [ebp-10h]
    int byteEndOffset; // [esp+8h] [ebp-Ch]
    int bitEndOffset; // [esp+Ch] [ebp-8h]

    if ( clientNum >= com_maxclients->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp",
                    496,
                    0,
                    "clientNum doesn't index com_maxclients->current.integer\n\t%i not in [0, %i)",
                    clientNum,
                    com_maxclients->current.integer) )
    {
        __debugbreak();
    }
    bitEndOffset = searchState->absoluteOffset + searchState->member->size / searchState->member->arraySize;
    if ( bitEndOffset % 8 )
        byteEndOffset = bitEndOffset / 8 + 1;
    else
        byteEndOffset = bitEndOffset / 8;
    for ( offset = searchState->absoluteOffset / 8; offset < byteEndOffset; ++offset )
        svs.clients[clientNum].modifiedStatBytes[offset >> 3] |= 1 << (offset & 7);
    svs.clients[clientNum].statsModified = 1;
}

void __cdecl SV_SetClientDIntStat(unsigned int clientNum, ddlState_t *searchState, unsigned int value)
{
    ddlDef_t *StatsDDL; // eax
    client_t *cl; // [esp+8h] [ebp-50h]
    char *buffer; // [esp+Ch] [ebp-4Ch]
    ddlResult_t oldValue; // [esp+10h] [ebp-48h] BYREF

    cl = &svs.clients[clientNum];
    if ( clientNum >= com_maxclients->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp",
                    528,
                    0,
                    "clientNum doesn't index com_maxclients->current.integer\n\t%i not in [0, %i)",
                    clientNum,
                    com_maxclients->current.integer) )
    {
        __debugbreak();
    }

    iassert(cl->statPacketsReceived == ALL_STATS_PACKETS_RECEIVED);
    
    if (cl->statPacketsReceived == ALL_STATS_PACKETS_RECEIVED && cl->header.state >= CS_RECONNECTING)
    {
        if ( xblive_basictraining->current.enabled )
            buffer = (char *)svs.clients[clientNum].stats;
        else
            buffer = (char *)svs.clients[clientNum].globalStats;
        if ( !cl->statsValidated )
        {
            StatsDDL = LiveStats_GetStatsDDL();
            if ( !DDL_AssociateBuffer(buffer, 40168, StatsDDL) )
            {
                DDL_PrintError("DDL: Could not get stat. Buffer error.");
                return;
            }
            cl->statsValidated = 1;
        }
        if ( !DDL_GetValue(searchState, &oldValue, buffer) || oldValue.intValue != value )
        {
            if ( !DDL_SetInt(searchState, value, buffer) )
                DDL_PrintError("DDL: Could not set stat. DDL set error.");
            SV_CacheClientStatChange(clientNum, searchState);
        }
    }
}

void __cdecl SV_SetClientDStringStat(unsigned int clientNum, ddlState_t *searchState, const char *value)
{
    ddlDef_t *StatsDDL; // eax
    client_t *cl; // [esp+8h] [ebp-50h]
    char *buffer; // [esp+Ch] [ebp-4Ch]
    ddlResult_t result; // [esp+10h] [ebp-48h] BYREF

    cl = &svs.clients[clientNum];
    if ( clientNum >= com_maxclients->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp",
                    585,
                    0,
                    "clientNum doesn't index com_maxclients->current.integer\n\t%i not in [0, %i)",
                    clientNum,
                    com_maxclients->current.integer) )
    {
        __debugbreak();
    }
    
    iassert(cl->statPacketsReceived == ALL_STATS_PACKETS_RECEIVED);

    if (cl->statPacketsReceived == ALL_STATS_PACKETS_RECEIVED && cl->header.state >= CS_RECONNECTING)
    {
        if ( xblive_basictraining->current.enabled )
            buffer = (char *)svs.clients[clientNum].stats;
        else
            buffer = (char *)svs.clients[clientNum].globalStats;
        if ( !cl->statsValidated )
        {
            StatsDDL = LiveStats_GetStatsDDL();
            if ( !DDL_AssociateBuffer(buffer, 40168, StatsDDL) )
            {
                DDL_PrintError("DDL: Could not get stat. Buffer error.");
                return;
            }
            cl->statsValidated = 1;
        }
        if ( !DDL_GetValue(searchState, &result, buffer) || I_strcmp(value, (const char *)&result) )
        {
            if ( !DDL_SetString(searchState, value, buffer) )
                DDL_PrintError("DDL: Could not set stat. DDL set error.");
            SV_CacheClientStatChange(clientNum, searchState);
        }
    }
}

void __cdecl SV_SetClientDInt64Stat(unsigned int clientNum, ddlState_t *searchState, unsigned __int64 value)
{
    ddlDef_t *StatsDDL; // eax
    client_t *cl; // [esp+8h] [ebp-50h]
    char *buffer; // [esp+Ch] [ebp-4Ch]
    ddlResult_t result; // [esp+10h] [ebp-48h] BYREF

    cl = &svs.clients[clientNum];
    if ( clientNum >= com_maxclients->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp",
                    642,
                    0,
                    "clientNum doesn't index com_maxclients->current.integer\n\t%i not in [0, %i)",
                    clientNum,
                    com_maxclients->current.integer) )
    {
        __debugbreak();
    }

    iassert(cl->statPacketsReceived == ALL_STATS_PACKETS_RECEIVED);
    
    if (cl->statPacketsReceived == ALL_STATS_PACKETS_RECEIVED && cl->header.state >= CS_RECONNECTING)
    {
        if ( xblive_basictraining->current.enabled )
            buffer = (char *)svs.clients[clientNum].stats;
        else
            buffer = (char *)svs.clients[clientNum].globalStats;
        if ( !cl->statsValidated )
        {
            StatsDDL = LiveStats_GetStatsDDL();
            if ( !DDL_AssociateBuffer(buffer, 40168, StatsDDL) )
            {
                DDL_PrintError("DDL: Could not get stat. Buffer error.");
                return;
            }
            cl->statsValidated = 1;
        }
        if ( !DDL_GetValue(searchState, &result, buffer) || value != result.int64Value )
        {
            if ( !DDL_SetInt64(searchState, value, buffer) )
                DDL_PrintError("DDL: Could not set stat. DDL set error.");
            SV_CacheClientStatChange(clientNum, searchState);
        }
    }
}

unsigned int __cdecl SV_GetClientDIntStat(unsigned int clientNum, ddlState_t *searchState)
{
    ddlDef_t *StatsDDL; // eax
    char *buffer; // [esp+8h] [ebp-4Ch]

    if ( clientNum >= com_maxclients->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp",
                    698,
                    0,
                    "clientNum doesn't index com_maxclients->current.integer\n\t%i not in [0, %i)",
                    clientNum,
                    com_maxclients->current.integer) )
    {
        __debugbreak();
    }

#ifndef KISAK_STATS
    iassert(0);
#endif

    if (svs.clients[clientNum].statPacketsReceived != ALL_STATS_PACKETS_RECEIVED)
    {
        return 0;
    }

    if ( svs.clients[clientNum].header.state < CS_RECONNECTING)
        return 0;
    if ( xblive_basictraining->current.enabled || searchState->member->permission != 2 )
        buffer = (char *)svs.clients[clientNum].stats;
    else
        buffer = (char *)svs.clients[clientNum].globalStats;
    if ( !svs.clients[clientNum].statsValidated )
    {
        StatsDDL = LiveStats_GetStatsDDL();
        if ( !DDL_AssociateBuffer(buffer, 40168, StatsDDL) )
        {
            DDL_PrintError("DDL: Could not get stat. Buffer error.");
            return 0;
        }
        svs.clients[clientNum].statsValidated = 1;
    }
    return DDL_GetInt(searchState, buffer);
}

char *__cdecl SV_GetClientDStringStat(unsigned int clientNum, ddlState_t *searchState)
{
    ddlDef_t *StatsDDL; // eax
    char *buffer; // [esp+8h] [ebp-4Ch]

    if ( clientNum >= com_maxclients->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp",
                    742,
                    0,
                    "clientNum doesn't index com_maxclients->current.integer\n\t%i not in [0, %i)",
                    clientNum,
                    com_maxclients->current.integer) )
    {
        __debugbreak();
    }

    iassert(svs.clients[clientNum].statPacketsReceived == ALL_STATS_PACKETS_RECEIVED);
    iassert(svs.clients[clientNum].header.state >= CS_RECONNECTING);

    if ( xblive_basictraining->current.enabled || searchState->member->permission != 2 )
        buffer = (char *)svs.clients[clientNum].stats;
    else
        buffer = (char *)svs.clients[clientNum].globalStats;
    if ( !svs.clients[clientNum].statsValidated )
    {
        StatsDDL = LiveStats_GetStatsDDL();
        if ( !DDL_AssociateBuffer(buffer, 40168, StatsDDL) )
        {
            DDL_PrintError("DDL: Could not get stat. Buffer error.");
            return (char *)"";
        }
        svs.clients[clientNum].statsValidated = 1;
    }
    return DDL_GetString(searchState, buffer);
}

unsigned int __cdecl SV_GetClientDInt64Stat(unsigned int clientNum, ddlState_t *searchState)
{
    ddlDef_t *StatsDDL; // eax
    char *buffer; // [esp+8h] [ebp-4Ch]

    if ( clientNum >= com_maxclients->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp",
                    778,
                    0,
                    "clientNum doesn't index com_maxclients->current.integer\n\t%i not in [0, %i)",
                    clientNum,
                    com_maxclients->current.integer) )
    {
        __debugbreak();
    }

#ifndef KISAK_STATS
    iassert(0);
#endif

    iassert(svs.clients[clientNum].statPacketsReceived == ALL_STATS_PACKETS_RECEIVED);
    iassert(svs.clients[clientNum].header.state >= CS_RECONNECTING);

    if ( xblive_basictraining->current.enabled || searchState->member->permission != 2 )
        buffer = (char *)svs.clients[clientNum].stats;
    else
        buffer = (char *)svs.clients[clientNum].globalStats;
    if ( !svs.clients[clientNum].statsValidated )
    {
        StatsDDL = LiveStats_GetStatsDDL();
        if ( !DDL_AssociateBuffer(buffer, 40168, StatsDDL) )
        {
            DDL_PrintError("DDL: Could not get stat. Buffer error.");
            return 0;
        }
        svs.clients[clientNum].statsValidated = 1;
    }
    return DDL_GetInt64(searchState, buffer);
}

void __cdecl SV_UploadStats(int clientNum)
{
    client_t *client; // [esp+0h] [ebp-4h]

    client = &svs.clients[clientNum];
    if ( client->header.state >= 3 )
    {
        SV_SendServerCommand(client, SV_CMD_RELIABLE, "%c", 81);
        if ( !xblive_wagermatch->current.enabled )
            SV_DWWriteClientStats(client);
    }
}

void __cdecl SV_UploadStats()
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < com_maxclients->current.integer; ++i )
        SV_UploadStats(i);
}

void __cdecl SV_SteamAuthClientRequest(netadr_t to, unsigned __int64 serverSteamID)
{
    unsigned __int8 *msgBuffer; // [esp+0h] [ebp-40h]
    msg_t msg; // [esp+4h] [ebp-3Ch] BYREF
    LargeLocal msgBuffer_large_local(2048); // [esp+34h] [ebp-Ch] BYREF

    //LargeLocal::LargeLocal(&msgBuffer_large_local, 2048);
    msgBuffer = msgBuffer_large_local.GetBuf(); // LargeLocal::GetBuf(&msgBuffer_large_local);
    MSG_Init(&msg, msgBuffer, 2048);
    MSG_WriteString(&msg, "steamauthReq");
    MSG_WriteInt64(&msg, serverSteamID);
    NET_OutOfBandData(NS_SERVER, to, msg.data, msg.cursize);
    //LargeLocal::~LargeLocal(&msgBuffer_large_local);
}

void __cdecl SV_SteamAuthClient(netadr_t from, msg_t *msg)
{
    unsigned __int64 clientID; // [esp+0h] [ebp-818h]
    unsigned int authBlobLen; // [esp+Ch] [ebp-80Ch]
    unsigned __int8 authBlob[0x800]; // [esp+10h] [ebp-808h] BYREF

    clientID = MSG_ReadInt64(msg);
    authBlobLen = MSG_ReadShort(msg);

    iassert(authBlobLen < sizeof(authBlob));

    MSG_ReadData(msg, authBlob, authBlobLen);
    LiveSteam_Server_ClientSteamAuthentication(clientID, from, authBlob, authBlobLen);
}

int __cdecl SV_IsTempBannedGuid(int guid)
{
    unsigned int banSlot; // [esp+0h] [ebp-4h]

    if ( !guid )
        return 0;
    for ( banSlot = 0; banSlot < 0x10; ++banSlot )
    {
        if ( svs.tempBans[banSlot].guid == guid
            && (float)(svs.time - svs.tempBans[banSlot].banTime) <= (float)(sv_kickBanTime->current.value * 1000.0) )
        {
            return 1;
        }
    }
    return 0;
}

int __cdecl SV_IsBannedGuid(int guid)
{
    char *file; // [esp+4h] [ebp-10h] BYREF
    int banned; // [esp+8h] [ebp-Ch]
    const char *token; // [esp+Ch] [ebp-8h]
    const char *text; // [esp+10h] [ebp-4h] BYREF

    if ( !guid )
        return 0;
    if ( FS_ReadFile("ban.txt", (void **)&file) < 0 )
        return 0;
    text = file;
    banned = 0;
    while ( 1 )
    {
        token = Com_Parse(&text)->token;
        if ( !*token )
            break;
        if ( atoi(token) == guid )
        {
            banned = 1;
            break;
        }
        Com_SkipRestOfLine(&text);
    }
    FS_FreeFile(file);
    return banned;
}

void __cdecl SV_BanGuidBriefly(int guid)
{
    unsigned int FreeTempBanSlot; // eax

    FreeTempBanSlot = SV_FindFreeTempBanSlot();
    svs.tempBans[FreeTempBanSlot].guid = guid;
    svs.tempBans[FreeTempBanSlot].banTime = svs.time;
}

unsigned int __cdecl SV_FindFreeTempBanSlot()
{
    unsigned int oldestSlot; // [esp+0h] [ebp-8h]
    unsigned int banSlot; // [esp+4h] [ebp-4h]

    oldestSlot = 0;
    for ( banSlot = 0; banSlot < 0x10; ++banSlot )
    {
        if ( !svs.tempBans[banSlot].guid )
            return banSlot;
        if ( svs.tempBans[banSlot].banTime < svs.tempBans[oldestSlot].banTime )
            oldestSlot = banSlot;
    }
    return oldestSlot;
}

void __cdecl SV_BanClient(client_t *cl)
{
    int file; // [esp+0h] [ebp-4Ch] BYREF
    char cleanName[68]; // [esp+4h] [ebp-48h] BYREF

    if ( !cl && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp", 997, 0, "%s", "cl") )
        __debugbreak();
    if ( cl->header.netchan.remoteAddress.type == NA_LOOPBACK )
    {
        SV_SendServerCommand(0, SV_CMD_CAN_IGNORE, "%c \"EXE_CANNOTKICKHOSTPLAYER\"", 101);
    }
    else if ( cl->guid )
    {
        if ( SV_IsBannedGuid(cl->guid) )
        {
            Com_Printf(15, "This GUID (%i) is already banned\n", cl->guid);
        }
        else if ( (FS_FOpenFileByMode((char*)"ban.txt", &file, FS_APPEND) & 0x80000000) == 0 )
        {
            I_strncpyz(cleanName, cl->name, 64);
            I_CleanStr(cleanName);
            FS_Printf(file, (char *)"%i %s\r\n", cl->guid, cleanName);
            FS_FCloseFile(file);
            SV_DropClient(cl, "EXE_PLAYERKICKED", 1, 1);
            cl->lastPacketTime = svs.time;
        }
    }
    else
    {
        Com_Printf(15, "Can't ban user, GUID is 0\n");
    }
}

void __cdecl SV_UnbanClient(const char *name)
{
    bool v1; // [esp+0h] [ebp-7Ch]
    int nameLen; // [esp+14h] [ebp-68h]
    char *file; // [esp+18h] [ebp-64h] BYREF
    int fileSize; // [esp+1Ch] [ebp-60h]
    int unban; // [esp+20h] [ebp-5Ch]
    char cleanName[68]; // [esp+24h] [ebp-58h] BYREF
    char *line; // [esp+6Ch] [ebp-10h]
    const char *token; // [esp+70h] [ebp-Ch]
    char *text; // [esp+74h] [ebp-8h] BYREF
    int found; // [esp+78h] [ebp-4h]

    fileSize = FS_ReadFile("ban.txt", (void **)&file);
    if ( fileSize >= 0 )
    {
        I_strncpyz(cleanName, name, 64);
        I_CleanStr(cleanName);
        nameLen = &cleanName[strlen(cleanName) + 1] - &cleanName[1];
        found = 0;
        text = file;
        while ( 1 )
        {
            line = text;
            token = (const char *)Com_Parse((const char **)&text);
            if ( !*token )
                break;
            while ( *text && *text <= 32 )
                ++text;
            v1 = !I_strnicmp(text, cleanName, nameLen) && (text[nameLen] == 13 || text[nameLen] == 10);
            unban = v1;
            Com_SkipRestOfLine((const char **)&text);
            if ( unban )
            {
                ++found;
                memmove((unsigned __int8 *)line, (unsigned __int8 *)text, fileSize - (text - file) + 1);
                fileSize -= text - line;
                text = line;
            }
        }
        FS_WriteFile((char *)"ban.txt", file, fileSize);
        FS_FreeFile(file);
        if ( found )
            Com_Printf(15, "unbanned %i user(s) named %s\n", found, cleanName);
        else
            Com_Printf(15, "no banned user has name %s\n", cleanName);
    }
}

void __cdecl SV_UpdateSplitscreenStateForAddr()
{
    ;
}

void __cdecl SV_FreeClient(client_t *cl)
{
    iassert(cl->header.state > CS_ZOMBIE);

    if ( (unsigned int)(cl - svs.clients) >= com_maxclients->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp",
                    1310,
                    0,
                    "cl - svs.clients doesn't index com_maxclients->current.integer\n\t%i not in [0, %i)",
                    cl - svs.clients,
                    com_maxclients->current.integer) )
    {
        __debugbreak();
    }
    //BLOPS_NULLSUB();
    SV_CloseDownload(cl);
    if ( SV_Loaded() )
        ClientDisconnect(cl - svs.clients);
    SV_SetUserinfo(cl - svs.clients, (char *)"");
    SV_FreeClientScriptId(cl);
}

void __cdecl SV_FreeClients()
{
    client_t *clients; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    i = 0;
    clients = svs.clients;
    while ( i < com_maxclients->current.integer )
    {
        if ( clients->header.state >= CS_CONNECTED )
            SV_FreeClient(clients);
        ++i;
        ++clients;
    }
}

void __cdecl SV_DirectConnect(netadr_t from)
{
    const char *BuildName; // eax

    unsigned int v19; // eax
    unsigned __int64 v20; // rax
    BOOL v24; // [esp-8h] [ebp-4E0h]
    const char *string; // [esp-4h] [ebp-4DCh]
    BOOL v26; // [esp-4h] [ebp-4DCh]
    int integer; // [esp+4h] [ebp-4D4h]
    char *fromAddr; // [esp+8h] [ebp-4D0h]
    unsigned int clientChallange; // [esp+18h] [ebp-4C0h] BYREF
    unsigned int nonce; // [esp+1Ch] [ebp-4BCh] BYREF
    int slot; // [esp+20h] [ebp-4B8h]
    int endIndex; // [esp+24h] [ebp-4B4h]
    int loller; // [esp+28h] [ebp-4B0h]
    int ping; // [esp+2Ch] [ebp-4ACh]
    client_t *clients; // [esp+30h] [ebp-4A8h]
    int cl_pb; // [esp+34h] [ebp-4A4h]
    const char *denied; // [esp+38h] [ebp-4A0h]
    bool isDemoClient; // [esp+3Fh] [ebp-499h]
    unsigned __int64 uid; // [esp+40h] [ebp-498h] BYREF
    int version; // [esp+48h] [ebp-490h]
    client_t *newcl; // [esp+4Ch] [ebp-48Ch]
    const char *steamIDStr; // [esp+50h] [ebp-488h]
    int challenge; // [esp+54h] [ebp-484h]
    int startIndex; // [esp+58h] [ebp-480h]
    char authSvPBguid[36]; // [esp+5Ch] [ebp-47Ch] BYREF
    const char *password; // [esp+80h] [ebp-458h]
    entityState_s *p_s; // [esp+84h] [ebp-454h]
    char userinfo[1028]; // [esp+88h] [ebp-450h] BYREF
    gentity_s *ent; // [esp+48Ch] [ebp-4Ch]
    unsigned int scriptId; // [esp+490h] [ebp-48h]
    char *pb_authmsg; // [esp+494h] [ebp-44h]
    int i; // [esp+498h] [ebp-40h]
    int clientNum; // [esp+49Ch] [ebp-3Ch]
    int guid; // [esp+4A0h] [ebp-38h]
    int count; // [esp+4A4h] [ebp-34h]
    int tempslot; // [esp+4A8h] [ebp-30h]
    int qport; // [esp+4ACh] [ebp-2Ch]
    char clientPBguid[36]; // [esp+4B0h] [ebp-28h] BYREF

    uint64_t steamID64 = 0; // lwss add

    memset(authSvPBguid, 0, 33);
    memset(clientPBguid, 0, 33);
    Com_DPrintf(15, "SV_DirectConnect()\n");
    I_strncpyz(userinfo, SV_Cmd_Argv(1), 1024);
    version = atoi(Info_ValueForKey(userinfo, "protocol"));
    if (version != 1044)
    {
        BuildName = Com_GetBuildName();
        NET_OutOfBandPrint(NS_SERVER, from, va("EXE_SERVER_IS_DIFFERENT_VER", BuildName));
        Com_DPrintf(15, "    rejected connect from protocol version %i (should be %i)\n", version, 1044);
        return;
    }
    challenge = atoi(Info_ValueForKey(userinfo, "challenge"));
    qport = atoi(Info_ValueForKey(userinfo, "qport"));
    i = 0;
    clients = svs.clients;
    while (i < com_maxclients->current.integer)
    {
        if (NET_CompareBaseAdr(from, clients->header.netchan.remoteAddress)
            && (clients->header.netchan.qport == qport || from.port == clients->header.netchan.remoteAddress.port))
        {
            if (svs.time - clients->lastConnectTime < 1000 * sv_reconnectlimit->current.integer)
            {
                Com_DPrintf(15, "%s:reconnect rejected : too soon\n", NET_AdrToString(from));
                return;
            }
            break;
        }
        ++i;
        ++clients;
    }

    guid = 0;


    if (!NET_IsLocalAddress(from))
    {
        for (i = 0; i < 1024; ++i)
        {
            if (NET_CompareAdr(from, svs.challenges[i].adr) && challenge == svs.challenges[i].challenge)
            {
                guid = svs.challenges[i].guid;
                steamID64 = svs.challenges[i].steamID64; // LWSS ADD
                I_strncpyz(authSvPBguid, svs.challenges[i].PBguid, 33);
                I_strncpyz(clientPBguid, svs.challenges[i].clientPBguid, 33);
                break;
            }
        }
        if (i == 1024)
        {
            NET_OutOfBandPrint(NS_SERVER, from, "error\nEXE_BAD_CHALLENGE");
            return;
        }
        if (svs.challenges[i].firstPing)
        {
            ping = svs.challenges[i].firstPing;
        }
        else
        {
            ping = svs.time - svs.challenges[i].pingTime;
            svs.challenges[i].firstPing = ping;
        }
        Com_Printf(15, "Client %i connecting with %i challenge ping from %s\n", i, ping, NET_AdrToString(from));
        svs.challenges[i].connected = 1;
        if (g_password)
        {
            if (*(_BYTE *)g_password->current.integer)
            {
                string = g_password->current.string;
                if (I_strcmp(Info_ValueForKey(userinfo, "password"), string))
                {
                    NET_OutOfBandPrint(NS_SERVER, from, "error\nGAME_INVALIDPASSWORD");
                    Com_DPrintf(15, "Rejected connection, invalid password %s\n", Info_ValueForKey(userinfo, "password"));
                    return;
                }
            }
        }

        if (!Sys_IsLANAddress(from))
        {
            if (sv_minPing->current.integer && ping < sv_minPing->current.integer)
            {
                NET_OutOfBandPrint(NS_SERVER, from, "error\nEXE_ERR_HIGH_PING_ONLY");
                Com_DPrintf(15, "Client %i rejected on a too low ping\n", i);
                return;
            }

            if (sv_maxPing->current.integer && ping > sv_maxPing->current.integer)
            {
                NET_OutOfBandPrint(NS_SERVER, from, "error\nEXE_ERR_LOW_PING_ONLY");
                Com_DPrintf(15, "Client %i rejected on a too high ping: %i\n", i, ping);
                return;
            }
        }
    }
    
    cl_pb = atoi(Info_ValueForKey(userinfo, "cl_punkbuster"));
    if (NET_IsLocalAddress(from))
    {
        //v13 = PbAuthClient("localhost", cl_pb, authSvPBguid);
    }
    else
    {
        fromAddr = NET_AdrToString(from);
        //v13 = PbAuthClient(fromAddr, cl_pb, authSvPBguid);
    }
    //pb_authmsg = v13;
    //if (v13)
    //{
    //    if (!I_strnicmp(pb_authmsg, "error\n", 6))
    //        NET_OutOfBandPrint(NS_SERVER, from, pb_authmsg);
    //}
    //else
    {
        i = 0;
        clients = svs.clients;
        while (i < com_maxclients->current.integer)
        {
            if (clients->header.state
                && NET_CompareBaseAdr(from, clients->header.netchan.remoteAddress)
                && (clients->header.netchan.qport == qport || from.port == clients->header.netchan.remoteAddress.port))
            {
                v26 = from.port == clients->header.netchan.remoteAddress.port;
                v24 = clients->header.netchan.qport == qport;
                Com_Printf(15, "%s:reconnect. same qport: %i, same port: %i\n", NET_AdrToString(from), v24, v26);
                if (clients->header.state >= CS_CONNECTED)
                {
                    NET_OutOfBandPrint(NS_SERVER, from, "error\nEXE_ERR_QPORT");
                    return;
                }
                newcl = clients;
                goto gotnewcl;
            }
            ++i;
            ++clients;
        }
        startIndex = Com_GetPrivateClients();
        isDemoClient = 0;
        if (Demo_IsEnabled())
        {
            loller = atoi(Info_ValueForKey(userinfo, "loller"));
            if (from.port == 1773 && loller == qport)
            {
                isDemoClient = 1;
                if (from.type
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp",
                        1871,
                        0,
                        "%s",
                        "from.type == NA_BOT"))
                {
                    __debugbreak();
                }
                newcl = svs.clients;
                goto gotnewcl;
            }
            ++startIndex;
        }
        newcl = 0;
        if (Demo_IsEnabled())
        {
            if (com_maxclients->current.integer < sv_maxclients->current.integer + 1)
                integer = com_maxclients->current.integer;
            else
                integer = sv_maxclients->current.integer + 1;
            endIndex = integer;
        }
        else
        {
            endIndex = sv_maxclients->current.integer;
        }
        for (i = startIndex; i < endIndex; ++i)
        {
            clients = &svs.clients[i];
            if (!clients->header.state)
            {
                newcl = clients;
                password = Info_ValueForKey(userinfo, "password");
                clients->reservedSlot = SV_GetSlotForPasswordIfFree(password);
                break;
            }
        }
        if (!newcl)
        {
            password = Info_ValueForKey(userinfo, "password");
            slot = SV_DropClientForReservedSlot(password);
            if (slot < 0)
            {
                NET_OutOfBandPrint(NS_SERVER, from, "error\nEXE_SERVERISFULL");
                Com_DPrintf(15, "Rejected a connection. com_maxclients = %d\n", com_maxclients->current.integer);
                return;
            }
            Com_DPrintf(15, "Accepted client for reserved slot %d, welcome!.\n", slot);
            newcl = &svs.clients[slot];
            newcl->reservedSlot = slot;
        }
        uid = 0;
        StringToXUID(Info_ValueForKey(userinfo, "bdOnlineUserID"), &uid);
        if (SV_IsBannedGuid(uid) || SV_IsTempBannedGuid(uid))
        {
            NET_OutOfBandPrint(NS_SERVER, from, "error\nPATCH_BANNED_FROM_SERVER");
            return;
        }
        clients->reliableAcknowledge = 0;
        clients->reliableSequence = 0;
        Demo_ClientConnected(clients - svs.clients);
    gotnewcl:
        tempslot = newcl->reservedSlot;
        memset(newcl, 0, sizeof(client_t));
        newcl->reservedSlot = tempslot;
        clientNum = newcl - svs.clients;
        Pregame_ResetDataForClient(clientNum);
        ent = (gentity_s *)((char *)sv.gentities + clientNum * sv.gentitySize);
        newcl->gentity = ent;
        p_s = &ent->s;
        if (ent->s.clientNum != ent->s.number
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp",
                2048,
                0,
                "%s",
                "es->clientNum == es->number"))
        {
            __debugbreak();
        }
        iassert(!newcl->scriptId);
#ifdef KISAK_LIVE_SERVICE
        if (live_service && live_service->current.enabled)
        {
            clientChallange = 0;
            nonce = 0;
            v17 = Info_ValueForKey(userinfo, "challengeNum");
            sscanf(v17, "%u", &clientChallange);
            v18 = Info_ValueForKey(userinfo, "nonce");
            sscanf(v18, "%u", &nonce);
            Com_DPrintf(
                15,
                "CHALLENGE RESPONSE: Read userID %llu, clientChallenge %u, nonce %u\n",
                uid,
                clientChallange,
                nonce);
            if (uid && clientChallange && nonce)
            {
                v19 = SV_SendClientChallenge(nonce, clientChallange, uid);
                newcl->dwchallenge = v19;
                newcl->dw_userID = uid;
            }
            else
            {
                Com_PrintWarning(15, "Failed to parse userId!\n");
            }
        }
#endif
        if (!newcl->bIsTestClient && !newcl->bIsDemoClient)
            newcl->notifyJoin = 1;
        scriptId = Scr_AllocArray(SCRIPTINSTANCE_SERVER);
        newcl->scriptId = scriptId;
        Com_Printf(15, "SV_DirectConnect: %d, 0 -> %d\n", newcl - svs.clients, newcl->scriptId);
        newcl->challenge = challenge;
        if (!clients->guid)
            Com_Printf(15, "Connecting player #%i has a zero GUID\n", clientNum);
        steamIDStr = Info_ValueForKey(userinfo, "steamid");

        // LWSS: removed this because my steam auth re-uses the same steamID field and this zeroes it
        //newcl->steamAuthorized = 0;
        //newcl->steamAuthFailCount = 0;
        //v20 = StringToInt64(steamIDStr);
        //newcl->steamID = v20;
        
        Netchan_Setup(
            NS_SERVER,
            &newcl->header.netchan,
            from,
            qport,
            newcl->netchanOutgoingBuffer,
            0x10000,
            newcl->netchanIncomingBuffer,
            2048);
        newcl->voicePacketCount = 0;
        newcl->sendVoice = 1;
        I_strncpyz(newcl->userinfo, userinfo, 1024);
        if (Demo_IsEnabled())
            newcl->bIsDemoClient = isDemoClient;
        denied = ClientConnect(clientNum, newcl->scriptId);
        if (denied)
        {
            NET_OutOfBandPrint(NS_SERVER, from, va("error\n%s", denied));
            Com_DPrintf(15, "Game rejected a connection: %s.\n", denied);
            SV_FreeClientScriptId(newcl);
        }
        else
        {
            Com_Printf(
                15,
                "Going from CS_FREE to CS_CONNECTED for %s (num %i guid %i)\n",
                newcl->name,
                clientNum,
                newcl->guid);
            newcl->header.state = CS_CONNECTED;
            newcl->nextSnapshotTime = svs.time;
            newcl->lastSnapshotTime = -1;
            newcl->lastPacketTime = svs.time;
            newcl->lastConnectTime = svs.time;
            I_strncpyz(newcl->PBguid, authSvPBguid, 33);
            I_strncpyz(newcl->clientPBguid, clientPBguid, 33);

            // LWSS ADD - for steam auth
            iassert(steamID64 || newcl->bIsDemoClient || !IsDedicatedServer());
            newcl->steamID = steamID64;
            // LWSS END

            SV_UserinfoChanged(newcl);
            svs.challenges[i].firstPing = 0;
            NET_OutOfBandPrint(NS_SERVER, from, va("connectResponse %s", fs_gameDirVar->current.string));
            newcl->gamestateMessageNum = -1;
            SV_BotNameUpdate(Info_ValueForKey(userinfo, "name"));
            SV_UpdateSplitscreenStateForAddr();
            count = 0;
            i = 0;
            clients = svs.clients;
            while (i < com_maxclients->current.integer)
            {
                if (svs.clients[i].header.state >= CS_CONNECTED)
                    ++count;
                ++i;
                ++clients;
            }
            if (count == 1 || count == com_maxclients->current.integer)
                SV_Heartbeat_f();
        }
    }
}

void __cdecl SV_FreeClientScriptPers()
{
    client_t *clients; // [esp+0h] [ebp-Ch]
    int i; // [esp+8h] [ebp-4h]

    i = 0;
    clients = svs.clients;
    while ( i < com_maxclients->current.integer )
    {
        if ( clients->header.state >= CS_CONNECTED )
        {
            SV_FreeClientScriptId(clients);
            clients->scriptId = Scr_AllocArray(SCRIPTINSTANCE_SERVER);
            Com_Printf(15, "SV_FreeClientScriptPers: %d, 0 -> %d\n", clients - svs.clients, clients->scriptId);
        }
        ++i;
        ++clients;
    }
}

void __cdecl SV_SendDisconnect(
                client_t *client,
                int state,
                const char *reason,
                bool translationForReason,
                const char *clientName)
{
    const char *v5; // eax

    if ( !client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp", 2260, 0, "%s", "client") )
    {
        __debugbreak();
    }
    if ( state < 3
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp",
                    2261,
                    0,
                    "%s\n\t(state) = %i",
                    "(state >= CS_CONNECTED)",
                    state) )
    {
        __debugbreak();
    }
    if ( !reason
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp", 2262, 0, "%s", "reason") )
    {
        __debugbreak();
    }
    if ( state == 5 )
    {
        if ( translationForReason )
            SV_SendServerCommand(client, SV_CMD_RELIABLE, "%c \"%s\"", 119, reason);
        else
            SV_SendServerCommand(client, SV_CMD_RELIABLE, "%c \"%s^7 %s\" PB", 119, clientName, reason);
    }
    else
    {
        if ( state != 3
            && state != 4
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp",
                        2287,
                        0,
                        "%s\n\t(state) = %i",
                        "(state == CS_CONNECTED || state == CS_CLIENTLOADING)",
                        state) )
        {
            __debugbreak();
        }
        v5 = va("disconnect %s", reason);
        NET_OutOfBandPrint(NS_SERVER, client->header.netchan.remoteAddress, v5);
    }
}


void __cdecl SV_DropClient(client_t *drop, const char *reason, bool tellThem, bool writeStats)
{
    int j; // [esp+14h] [ebp-3Ch]
    int dropState; // [esp+18h] [ebp-38h]
    char droppedClientName[32]; // [esp+1Ch] [ebp-34h] BYREF
    bool translationForReason; // [esp+43h] [ebp-Dh]
    challenge_t *challenge; // [esp+44h] [ebp-Ch]
    int i; // [esp+48h] [ebp-8h]
    int clientNum; // [esp+4Ch] [ebp-4h]

    iassert(drop->header.state != CS_FREE);

    dropState = drop->header.state;
    if (drop->header.state == CS_ZOMBIE)
    {
        iassert(drop->dropReason == 0);
    }
    else
    {
        drop->dropReason = 0;

        iassert(dropState >= CS_ZOMBIE);

        for (j = 0; j < 32; ++j)
        {
            if (g_notifyLeave[j] == 0)
            {
                g_notifyLeave[j] = drop->dw_userID;
            }
            //if (!(HIDWORD(g_notifyLeave[j]) | LODWORD(g_notifyLeave[j])))
            //{
            //    LODWORD(g_notifyLeave[j]) = drop->dw_userID;
            //    HIDWORD(g_notifyLeave[j]) = HIDWORD(drop->dw_userID);
            //}
        }

#ifdef KISAK_STATS
        if (drop->statPacketsReceived == ALL_STATS_PACKETS_RECEIVED && writeStats)
            SV_DWWriteClientStats(drop);
#endif

        if (drop->reservedSlot > 0)
            SV_FreeReservedSlot(drop->reservedSlot);

        I_strncpyz(droppedClientName, Com_DisplayName(drop->name, drop->clanAbbrev, 3), 32);
        clientNum = drop - svs.clients;
        if ((unsigned int)clientNum >= level.maxclients
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp",
                2360,
                0,
                "clientNum doesn't index level.maxclients\n\t%i not in [0, %i)",
                clientNum,
                level.maxclients))
        {
            __debugbreak();
        }
#ifdef KISAK_LIVE
        if (onlinegame->current.enabled && com_sv_running->current.enabled)
            MatchRecordPlayerDetails(&level.clients[clientNum], reason);
#endif
        // LWSS ADD
        if (IsDedicatedServer() && !drop->bIsDemoClient)
        {
            Steam_OnClientDropped(drop->steamID);
        }
        // LWSS END
        LiveSteam_Server_ClientSteamDisconnect(drop->steamID);
        SV_FreeClient(drop);
        Com_DPrintf(15, "Going to CS_ZOMBIE from %i for %s due to %s\n", dropState, droppedClientName, reason);
        drop->header.state = CS_ZOMBIE;
        if (!drop->gentity)
        {
            challenge = svs.challenges;
            i = 0;
            while (i < 1024)
            {
                if (NET_CompareAdr(drop->header.netchan.remoteAddress, challenge->adr))
                {
                    challenge->connected = 0;
                    break;
                }
                ++i;
                ++challenge;
            }
        }
        translationForReason = SEH_StringEd_GetString((char*)reason) != 0;

        static const char aC_2[17] = "%c \"\x15%s^7 \x14%s%s\"";

        if (I_stricmp(reason, "EXE_DISCONNECTED"))
        {
            if (I_stricmp(reason, "EXE_PLAYERKICKED_BAN"))
            {
                if (translationForReason)
                    SV_SendServerCommand(0, SV_CMD_CAN_IGNORE, aC_2, 101, droppedClientName, "", reason);
                else
                    SV_SendServerCommand(0, SV_CMD_CAN_IGNORE, aC_2, 101, droppedClientName, "", reason);
            }
            else if (translationForReason)
            {
                SV_SendServerCommand(0, SV_CMD_CAN_IGNORE, aC_2, 101, droppedClientName, "", "EXE_PLAYERKICKED"); // KISAKTODO: 1st strings here might not be blank?
            }
            else
            {
                SV_SendServerCommand(0, SV_CMD_CAN_IGNORE, aC_2, 101, droppedClientName, "", "EXE_PLAYERKICKED");
            }
        }
        else if (translationForReason)
        {
            SV_SendServerCommand(0, SV_CMD_CAN_IGNORE, aC_2, 101, droppedClientName, "", "EXE_LEFTGAME");
        }
        else
        {
            SV_SendServerCommand(0, SV_CMD_CAN_IGNORE, aC_2, 101, droppedClientName, "", "EXE_LEFTGAME");
        }
        Com_Printf(15, "%i:%s %s\n", clientNum, droppedClientName, reason);
        SV_SendServerCommand(0, SV_CMD_RELIABLE, "%c %d", 75, clientNum);
        if (tellThem)
            SV_SendDisconnect(drop, dropState, reason, translationForReason, droppedClientName);
        if (drop->bIsTestClient || drop->bIsDemoClient)
        {
            drop->bIsDemoClient = 0;
            drop->bIsTestClient = 0;
            if (drop->header.state == CS_ZOMBIE)
                drop->header.state = CS_FREE;
        }

        for (i = 0; i < com_maxclients->current.integer && svs.clients[i].header.state < CS_CONNECTED; ++i)
            ;

        if (i == com_maxclients->current.integer)
            SV_Heartbeat_f();
    }
}

void __cdecl SV_DelayDropClient(client_t *drop, const char *reason)
{
    iassert(drop);
    iassert(reason);
    iassert(drop->header.state != CS_FREE);

    if ( drop->header.state == CS_ZOMBIE )
    {
        iassert(drop->dropReason == 0); 
    }
    else if ( !drop->dropReason )
    {
        drop->dropReason = reason;
    }
}

void __cdecl SV_SendClientGameState(client_t *client)
{
    const char *ConfigStringValue; // eax
    const char *v8; // eax
    const char *v9; // eax
    const char *v11; // [esp-4h] [ebp-1C8h]
    int entNum; // [esp+40h] [ebp-184h]
    int configStringCount; // [esp+44h] [ebp-180h]
    char *c; // [esp+48h] [ebp-17Ch]
    int configStringNum; // [esp+4Ch] [ebp-178h]
    int configStringNuma; // [esp+4Ch] [ebp-178h]
    int configStringNumb; // [esp+4Ch] [ebp-178h]
    unsigned __int8 *msgBuffer; // [esp+50h] [ebp-174h]
    entityState_s nullstate; // [esp+54h] [ebp-170h] BYREF
    int numWritten; // [esp+138h] [ebp-8Ch]
    int nextConstConfigStringIndex; // [esp+13Ch] [ebp-88h]
    SnapshotInfo_s snapInfo; // [esp+140h] [ebp-84h] BYREF
    int startTime; // [esp+158h] [ebp-6Ch]
    int size; // [esp+15Ch] [ebp-68h]
    msg_t msg; // [esp+160h] [ebp-64h] BYREF
    int oldDebugQuick; // [esp+190h] [ebp-34h]
    int lastStringIndex; // [esp+194h] [ebp-30h]
    int largestString; // [esp+198h] [ebp-2Ch]
    const char *configString; // [esp+19Ch] [ebp-28h]
    int dataStart; // [esp+1A0h] [ebp-24h]
    const char *constValue; // [esp+1A4h] [ebp-20h]
    int f; // [esp+1A8h] [ebp-1Ch]
    int nextConstConfigStringNumber; // [esp+1ACh] [ebp-18h]
    entityState_s *base; // [esp+1B0h] [ebp-14h]
    LargeLocal msgBuffer_large_local(0x10000); // [esp+1B4h] [ebp-10h] BYREF
    int clientNum; // [esp+1BCh] [ebp-8h]
    int totalStringSize; // [esp+1C0h] [ebp-4h]

    //LargeLocal::LargeLocal(&msgBuffer_large_local, 0x10000);
    msgBuffer = msgBuffer_large_local.GetBuf(); // LargeLocal::GetBuf(&msgBuffer_large_local);

    bcassert(client - svs.clients, com_maxclients->current.integer);

    if ( client->header.state != CS_FREE && client->header.netchan.unsentFragments && client->header.netchan.reliable_fragments )
    {
        Com_PrintError(0, va(
            "Would have gone into INFINITE LOOP OF DOOM for client %s, unsentlength %i, state %i\n",
            client->name,
            client->header.netchan.unsentLength,
            client->header.state));
        SV_SysLog_LogMessage(0, va(
            "Would have gone into INFINITE LOOP OF DOOM for client %s, unsentlength %i, state %i\n",
            client->name,
            client->header.netchan.unsentLength,
            client->header.state));
        return;
    }

    while ( client->header.state != CS_FREE && client->header.netchan.unsentFragments && !client->header.netchan.reliable_fragments )
    {
        if ( !SV_Netchan_TransmitNextFragment(client, &client->header.netchan) )
        {
            Com_PrintError(0, va(
                "Dropping client %s, unsentlength %i, state %i\n",
                client->name,
                client->header.netchan.unsentLength,
                client->header.state));
            SV_SysLog_LogMessage(0, va(
                "Dropping client %s, unsentlength %i, state %i\n",
                client->name,
                client->header.netchan.unsentLength,
                client->header.state));
            SV_DropClient(client, "EXE_TRANSMITERROR", 0, 1);
            return;
        }
    }

#ifdef KISAK_STATS
    // to pass the below check if it's just a testclient/democlient 
    if (client->bIsTestClient || client->bIsDemoClient)
    {
        memset(client->stats, 0, sizeof(client->stats));
        client->statPacketsReceived = ALL_STATS_PACKETS_RECEIVED;
    }

    if ( client->statPacketsReceived != ALL_STATS_PACKETS_RECEIVED )
    {
        Com_DPrintf(15, "Not sending state to %s, waiting on stats\n", client->name);
        return;
    }
#else
    memset(client->stats, 0, sizeof(client->stats));
    client->statPacketsReceived = ALL_STATS_PACKETS_RECEIVED;
#endif

    memset(&snapInfo, 0, sizeof(snapInfo));
    SV_SetServerStaticHeader();
    Com_Printf(15, "SV_SendClientGameState() for %s\n", client->name);
    Com_Printf(15, "Going from CS_CONNECTED to CS_CLIENTLOADING for %s\n", client->name);
    client->header.state = CS_CLIENTLOADING;
    client->pureAuthentic = 0;
    client->gamestateMessageNum = client->header.netchan.outgoingSequence;
    MSG_Init(&msg, msgBuffer, 0x10000);
    MSG_ClearLastReferencedEntity(&msg);
    MSG_WriteLong(&msg, client->lastClientCommand);
    dataStart = msg.cursize;
    oldDebugQuick = sv_debugPacketContentsQuick->current.integer;
    if ( oldDebugQuick > 0 )
        Dvar_SetInt((dvar_s *)sv_debugPacketContentsQuick, 0);
    SV_UpdateServerCommandsToClient(client, &msg);
    Com_Printf(15, "Gamestate has %i bytes of server commands\n", msg.cursize - dataStart);

    iassert(sv_mapname->current.string[0]);
    iassert(sv_gametype->current.string[0]);

    MSG_WriteByte(&msg, svc_gamestate);
    MSG_WriteLong(&msg, client->reliableSequence);
    MSG_WriteString(&msg, sv_mapname->current.string);
    MSG_WriteString(&msg, sv_gametype->current.string);
    MSG_WriteLong(&msg, CCS_GetChecksum());
    numWritten = 0;
    largestString = 0;
    totalStringSize = 0;
    dataStart = msg.cursize;

    MSG_WriteByte(&msg, svc_configstring);
    configStringCount = 0;
    nextConstConfigStringIndex = 0;
    nextConstConfigStringNumber = CCS_GetConfigStringNum(0);
    for ( configStringNum = 0; configStringNum < MAX_CONFIGSTRINGS; ++configStringNum )
    {
        if ( configStringNum == 1 )
        {
            iassert(nextConstConfigStringNumber != configStringNum);
            ++configStringCount;
        }
        else if ( nextConstConfigStringNumber == configStringNum )
        {
            configString = SL_ConvertToString(sv.configstrings[configStringNum], SCRIPTINSTANCE_SERVER);
            if ( nextConstConfigStringNumber >= 1547
                && (v11 = configString,
                        ConfigStringValue = CCS_GetConfigStringValue(nextConstConfigStringIndex),
                        I_stricmp(ConfigStringValue, v11)) )
            {
                ++configStringCount;
            }
            else if ( nextConstConfigStringNumber < 1547
                         && strcmp(CCS_GetConfigStringValue(nextConstConfigStringIndex), configString) )
            {
                ++configStringCount;
            }
            nextConstConfigStringNumber = CCS_GetConfigStringNum(++nextConstConfigStringIndex);
        }
        else if ( sv.configstrings[configStringNum] != sv.emptyConfigString )
        {
            ++configStringCount;
        }
    }

    f = 0;
    if ( client->header.netchan.remoteAddress.type == NA_LOOPBACK
        && onlinegame->current.enabled
        && sv_writeConfigStrings->current.enabled )
    {
        f = FS_SV_FOpenFileWrite(va("configStrings_pc_%s_%s.csv", sv_mapname->current.string, g_gametype->current.string), (char*)"devOutput");
    }

    if ( f )
    {
        Com_Printf(15, "Starting writing config strings to file\n");
        startTime = Sys_Milliseconds();
        for ( configStringNuma = 0; configStringNuma < MAX_CONFIGSTRINGS; ++configStringNuma )
        {
            if ( SV_ConfigStringIsConstant(configStringNuma) && sv.configstrings[configStringNuma] != sv.emptyConfigString )
            {
                configString = SL_ConvertToString(sv.configstrings[configStringNuma], SCRIPTINSTANCE_SERVER);
                v8 = strchr(configString, 0x22u);
                if ( v8 || (v9 = strchr(configString, 0x5Cu), v9) )
                {
                    FS_Printf(f, (char*)"%i,\"", configStringNuma);
                    for ( c = (char *)configString; *c; ++c )
                    {
                        if ( *c == 34 )
                        {
                            FS_Printf(f, (char *)"\"\"");
                        }
                        else if ( *c == 92 )
                        {
                            FS_Printf(f, (char *)"\\");
                        }
                        else
                        {
                            FS_Printf(f, (char *)"%c", *c);
                        }
                    }
                    FS_Printf(f, (char *)"\"\n");
                }
                else
                {
                    FS_Printf(f, (char *)"%i,\"%s\"\n", configStringNuma, configString);
                }
            }
        }
        FS_FCloseFile(f);
        Com_Printf(15, "Finished writing config strings to file (%i mec)\n", Sys_Milliseconds() - startTime);
    }
    else if ( sv_writeConfigStrings->current.enabled )
    {
        Com_Printf(15, "Could not write config strings to file\n");
    }

    MSG_WriteShort(&msg, configStringCount);
    nextConstConfigStringIndex = 0;
    nextConstConfigStringNumber = CCS_GetConfigStringNum(0);
    lastStringIndex = -1;
    for ( configStringNumb = 0; configStringNumb < MAX_CONFIGSTRINGS; ++configStringNumb )
    {
        if ( configStringNumb == 1 )
        {
            if ( nextConstConfigStringNumber == 1
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp",
                            2795,
                            0,
                            "%s",
                            "nextConstConfigStringNumber != configStringNum") )
            {
                __debugbreak();
            }
            configString = Dvar_InfoString_Big(8);
process_configString:
            if ( configStringCount < 0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp",
                            2859,
                            0,
                            "%s",
                            "configStringCount >= 0") )
            {
                __debugbreak();
            }
            if ( configStringNumb == lastStringIndex + 1 )
            {
                MSG_WriteBit1(&msg);
            }
            else
            {
                MSG_WriteBit0(&msg);
                MSG_WriteBits(&msg, configStringNumb, 0xCu);
            }
            lastStringIndex = configStringNumb;
            MSG_WriteBigString(&msg, (char *)configString);
            --configStringCount;
            if ( sv_debugConstantConfigStrings->current.enabled )
                Com_Printf(15, "v|CS %4i: %s\n", configStringNumb, configString);
            size = strlen(configString);
            if ( size > largestString )
                largestString = size;
            totalStringSize += size;
            ++numWritten;
            continue;
        }
        configString = SL_ConvertToString(sv.configstrings[configStringNumb], SCRIPTINSTANCE_SERVER);
        if ( nextConstConfigStringNumber && nextConstConfigStringNumber < configStringNumb )
            Com_Error(
                ERR_DROP,
                "Invalid line in the config string csv - config string indexes must only go up, but it went down and is now %i (l"
                "ine %i) - check the bottom of the file to see if it's corrupt",
                nextConstConfigStringNumber,
                nextConstConfigStringIndex);
        if ( nextConstConfigStringNumber == configStringNumb )
        {
            constValue = CCS_GetConfigStringValue(nextConstConfigStringIndex);
            if ( configStringNumb >= 1547 && !I_stricmp(constValue, configString)
                || configStringNumb < 1547 && !strcmp(constValue, configString) )
            {
                nextConstConfigStringNumber = CCS_GetConfigStringNum(++nextConstConfigStringIndex);
                continue;
            }
            if ( sv.configstrings[configStringNumb] == sv.emptyConfigString )
            {
                if ( !G_ExitAfterToolComplete()
                    && configStringCount < 0
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp",
                                2828,
                                0,
                                "%s",
                                "configStringCount >= 0") )
                {
                    __debugbreak();
                }
                MSG_WriteBit0(&msg);
                MSG_WriteBits(&msg, configStringNumb, 0xCu);
                lastStringIndex = configStringNumb;
                MSG_WriteBigString(&msg, (char *)"");
                if ( sv_debugConstantConfigStrings->current.enabled )
                    Com_Printf(
                        15,
                        "v|CS %4i: Sending empty config string - we had '%s' as the const value\n",
                        configStringNumb,
                        constValue);
                ++numWritten;
                --configStringCount;
                nextConstConfigStringNumber = CCS_GetConfigStringNum(++nextConstConfigStringIndex);
            }
            else
            {
                if ( sv_debugConstantConfigStrings->current.enabled )
                    Com_Printf(
                        15,
                        "w|CS %4i: constant config string '%s' doesn't match our value '%s'\n",
                        configStringNumb,
                        constValue,
                        configString);
                nextConstConfigStringNumber = CCS_GetConfigStringNum(++nextConstConfigStringIndex);
            }
        }
        if ( sv.configstrings[configStringNumb] != sv.emptyConfigString )
            goto process_configString;
    }

    iassert(configStringCount == 0);

    if ( msg.cursize - dataStart > 4000 && onlinegame->current.enabled )
        StatMon_Warning(12, 20000, (char *)"code_warning_gamestate");
    Com_Printf(
        15,
        "Gamestate has %i bytes of config strings (%i total config strings)\n",
        msg.cursize - dataStart,
        numWritten);
    Com_Printf(15, "     Largest config string was %i bytes\n", largestString);
    Com_Printf(15, "     Average config string was %i bytes\n", totalStringSize / numWritten);
    dataStart = msg.cursize;
    numWritten = 0;
    clientNum = client - svs.clients;

    bcassert(clientNum, com_maxclients->current.integer);
    memset(&nullstate, 0, sizeof(nullstate));
    for ( entNum = 0; entNum < 1024; ++entNum )
    {
        base = &sv.svEntities[entNum].baseline.s;
        if ( base->number )
        {
            MSG_WriteByte(&msg, svc_baseline);
            snapInfo.clientNum = client - svs.clients;
            snapInfo.snapshotDeltaTime = -1;
            snapInfo.fromBaseline = 1;
            MSG_PacketAnalyze_SetPacketEntityType(&snapInfo, ANALYZE_DATATYPE_ENTITYTYPE_BASELINE, 0);
            MSG_WriteEntity(&snapInfo, &msg, 0, &nullstate, base, DELTA_FLAGS_FORCE);
            ++numWritten;
            snapInfo.fromBaseline = 0;
        }
    }
    if ( oldDebugQuick > 0 )
        Dvar_SetInt((dvar_s *)sv_debugPacketContentsQuick, oldDebugQuick);
    Com_Printf(15, "Gamestate has %i bytes of svc_baselines\n", numWritten);
    Com_Printf(15, "Gamestate has %i bytes of gentity numbers\n", 10 * numWritten / 8);
    Com_Printf(
        15,
        "Gamestate has %i bytes of entity deltas\n",
        msg.cursize - dataStart - numWritten - 10 * numWritten / 8);


    MSG_WriteByte(&msg, svc_gameinformation);
    MSG_WriteLong(
        &msg,
        level.matchState.unarchivedState.matchUIVisibilityFlags | level.matchState.archivedState.matchUIVisibilityFlags);
    MSG_WriteByte(&msg, svc_EOF);
    MSG_WriteLong(&msg, client - svs.clients);

    MSG_WriteLong(&msg, sv.checksumFeed);

    MSG_WriteByte(&msg, svc_dynentstate);
    DynEnt_WriteGameState(&msg);

    MSG_WriteByte(&msg, svc_destructiblestate);
    Destructible_WriteGameState(&msg);

    MSG_WriteByte(&msg, svc_ropestate);
    Rope_WriteGameState(&msg);

    MSG_WriteByte(&msg, svc_glassstate);
    GlassSv_WriteGameState(&msg);

    UI_Gametype_Custom_WriteClientData(&msg);
    MSG_WriteByte(&msg, svc_EOF);

    Com_DPrintf(15, "Sending %i bytes in gamestate to client: %i\n", msg.cursize, client - svs.clients);

    if ( Demo_IsRecording() && clientNum == Demo_GetDemoClientIndex() )
        Demo_WriteGamestateToBuffer(clientNum, 3, &msg, client->header.netchan.outgoingSequence);

    SV_SendMessageToClient(&msg, client, 1);
    SV_GetServerStaticHeader();
    //LargeLocal::~LargeLocal(&msgBuffer_large_local);
}

bool __cdecl SV_ConfigStringIsConstant(int configStringNum)
{
    bool result; // al

    switch ( configStringNum )
    {
        case 0:
        case 5:
        case 6:
        case 12:
            result = 0;
            break;
        default:
            result = (configStringNum < 323 || configStringNum > 354) && (configStringNum < 355 || configStringNum > 386);
            break;
    }
    return result;
}

void __cdecl SV_ClientEnterWorld(client_t *client, usercmd_s *cmd)
{
    gentity_s *ent; // [esp+Ch] [ebp-8h]
    unsigned int clientNum; // [esp+10h] [ebp-4h]

    Com_DPrintf(15, "Going from CS_CLIENTLOADING to CS_ACTIVE for %s\n", client->name);
    client->header.state = CS_ACTIVE;
    clientNum = client - svs.clients;
    if (clientNum >= com_maxclients->current.integer
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp",
            3018,
            0,
            "clientNum doesn't index com_maxclients->current.integer\n\t%i not in [0, %i)",
            clientNum,
            com_maxclients->current.integer))
    {
        __debugbreak();
    }
    client->header.deltaMessage = -1;
    client->nextSnapshotTime = svs.time;
    client->lastSnapshotTime = -1;
    memcpy(&client->lastUsercmd, cmd, sizeof(client->lastUsercmd));
    ent = (gentity_s *)((char *)sv.gentities + clientNum * sv.gentitySize);
    ent->s.number = clientNum;
    client->gentity = ent;
    ClientBegin(client - svs.clients);
}

void __cdecl SV_CloseDownload(client_t *cl)
{
    int i; // [esp+0h] [ebp-4h]

    if ( cl->download )
        FS_FCloseFile(cl->download);
    cl->download = 0;
    cl->downloadName[0] = 0;
    for ( i = 0; i < 8; ++i )
    {
        if ( cl->downloadBlocks[i] )
        {
            Z_Free((char *)cl->downloadBlocks[i], 10);
            cl->downloadBlocks[i] = 0;
        }
    }
}

void __cdecl SV_StopDownload_f(client_t *cl)
{
    if ( (unsigned int)(cl - svs.clients) >= com_maxclients->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp",
                    3130,
                    0,
                    "cl - svs.clients doesn't index com_maxclients->current.integer\n\t%i not in [0, %i)",
                    cl - svs.clients,
                    com_maxclients->current.integer) )
    {
        __debugbreak();
    }
    if ( cl->downloadName[0] )
        Com_DPrintf(0, "clientDownload: %d : file \"%s\" aborted\n", cl - svs.clients, cl->downloadName);
    SV_CloseDownload(cl);
    SV_Download_Clear(cl);
}

void __cdecl SV_Download_Clear(client_t *cl)
{
    if ( !cl && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp", 3075, 0, "%s", "cl") )
        __debugbreak();
    cl->downloading = 0;
    if ( cl->download )
        FS_FCloseFile(cl->download);
    cl->download = 0;
    cl->downloadName[0] = 0;
}

void __cdecl SV_DoneDownload_f(client_t *cl)
{
    Com_DPrintf(0, "clientDownload: %s Done\n", cl->name);
    SV_Download_Clear(cl);
    Com_Printf(
        15,
        "SV_SendClientGameState() for %s file %s line %i\n",
        cl->name,
        "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp",
        3154);
    SV_SendClientGameState(cl);
}

void __cdecl SV_RetransmitDownload_f(client_t *cl)
{
    const char *v1; // eax

    v1 = SV_Cmd_Argv(1);
    if ( atoi(v1) == cl->downloadClientBlock )
        cl->downloadXmitBlock = cl->downloadClientBlock;
}

void __cdecl SV_NextDownload_f(client_t *cl)
{
    const char *v1; // eax
    int block; // [esp+0h] [ebp-4h]

    if ( (unsigned int)(cl - svs.clients) >= com_maxclients->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp",
                    3189,
                    0,
                    "cl - svs.clients doesn't index com_maxclients->current.integer\n\t%i not in [0, %i)",
                    cl - svs.clients,
                    com_maxclients->current.integer) )
    {
        __debugbreak();
    }
    v1 = SV_Cmd_Argv(1);
    block = atoi(v1);
    if ( block == cl->downloadClientBlock )
    {
        Com_DPrintf(0, "clientDownload: %d : client acknowledge of block %d\n", cl - svs.clients, block);
        if ( cl->downloadBlockSize[cl->downloadClientBlock % 8] )
        {
            cl->downloadSendTime = svs.time;
            ++cl->downloadClientBlock;
        }
        else
        {
            Com_Printf(0, "clientDownload: %d : file \"%s\" completed\n", cl - svs.clients, cl->downloadName);
            SV_CloseDownload(cl);
        }
    }
    else
    {
        SV_DropClient(cl, "broken download", 1, 1);
    }
}

void __cdecl SV_BeginDownload_f(client_t *cl)
{
    const char *v1; // eax

    SV_CloseDownload(cl);
    cl->downloading = 1;
    v1 = SV_Cmd_Argv(1);
    I_strncpyz(cl->downloadName, v1, 64);
}

void __cdecl SV_WWWDownload_f(client_t *cl)
{
    const char *subcmd; // [esp+0h] [ebp-4h]

    subcmd = SV_Cmd_Argv(1);
    if ( !cl->downloadingWWW )
        goto LABEL_2;
    if ( I_stricmp(subcmd, "ack") )
    {
        if ( I_stricmp(subcmd, "bbl8r") )
        {
            if ( !cl->clientDownloadingWWW )
            {
LABEL_2:
                Com_Printf(15, "SV_WWWDownload: unexpected wwwdl '%s' for client '%s'\n", subcmd, cl->name);
                SV_DropClient(cl, "Unexpected www download message.", 1, 1);
                return;
            }
            if ( I_stricmp(subcmd, "done") )
            {
                if ( I_stricmp(subcmd, "fail") )
                {
                    if ( I_stricmp(subcmd, "chkfail") )
                    {
                        Com_Printf(15, "SV_WWWDownload: unknown wwwdl subcommand '%s' for client '%s'\n", subcmd, cl->name);
                        SV_DropClient(cl, "Unexpected www download message.", 1, 1);
                    }
                    else
                    {
                        Com_Printf(
                            15,
                            "WARNING: client '%s' reports that the redirect download for '%s' had wrong checksum.\n",
                            cl->name,
                            cl->downloadName);
                        Com_Printf(15, "                 you should check your download redirect configuration.\n");
                        SV_WWWDownLoad_Clear(cl);
                        cl->wwwFallback = 1;
                        Com_Printf(
                            15,
                            "SV_SendClientGameState() for %s file %s line %i\n",
                            cl->name,
                            "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp",
                            3316);
                        SV_SendClientGameState(cl);
                    }
                }
                else
                {
                    SV_WWWDownLoad_Clear(cl);
                    cl->wwwFallback = 1;
                    Com_Printf(
                        15,
                        "Client '%s' reported that the http download of '%s' failed, falling back to a server download\n",
                        cl->name,
                        cl->downloadName);
                    Com_Printf(
                        15,
                        "SV_SendClientGameState() for %s file %s line %i\n",
                        cl->name,
                        "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp",
                        3304);
                    SV_SendClientGameState(cl);
                }
            }
            else
            {
                SV_WWWDownLoad_Clear(cl);
            }
        }
        else
        {
            SV_DropClient(cl, "Client dropped to download files.", 1, 1);
        }
    }
    else
    {
        if ( cl->clientDownloadingWWW )
            Com_Printf(15, "WARNING: dupe wwwdl ack from client '%s'\n", cl->name);
        cl->clientDownloadingWWW = 1;
    }
}

void __cdecl SV_WWWDownLoad_Clear(client_t *cl)
{
    if ( !cl && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp", 3240, 0, "%s", "cl") )
        __debugbreak();
    SV_Download_Clear(cl);
    cl->clientDownloadingWWW = 0;
    cl->downloadingWWW = 0;
}

void __cdecl SV_WWWRedirectClient(client_t *cl, msg_t *msg)
{
    const char *v2; // eax
    int download_flag; // [esp+0h] [ebp-4h]

    download_flag = 0;
    if ( cl->downloadSize < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp",
                    3338,
                    0,
                    "%s",
                    "cl->downloadSize >= 0") )
    {
        __debugbreak();
    }
    v2 = va("%s/%s", sv_wwwBaseURL->current.string, cl->downloadName);
    I_strncpyz(cl->downloadURL, v2, 256);
    Com_Printf(15, "Redirecting client '%s' to %s\n", cl->name, cl->downloadURL);
    cl->downloadingWWW = 1;
    MSG_WriteByte(msg, 0xBu);
    MSG_WriteLong(msg, -1);
    MSG_WriteString(msg, cl->downloadURL);
    MSG_WriteLong(msg, cl->downloadSize);
    if ( sv_wwwDlDisconnected->current.enabled )
        download_flag = 1;
    MSG_WriteLong(msg, download_flag);
    SV_Download_Clear(cl);
}

void __cdecl SV_WriteDownloadErrorMessage(client_t *cl, msg_t *msg, const char *errorMessage)
{
    MSG_WriteByte(msg, 0xBu);
    MSG_WriteLong(msg, 0);
    MSG_WriteLong(msg, -1);
    MSG_WriteString(msg, errorMessage);
    SV_Download_Clear(cl);
}

void __cdecl SV_WriteDownloadToClient(client_t *cl, msg_t *msg)
{
    int v2; // edx
    int rate; // [esp+0h] [ebp-414h]
    char errorMessage[1024]; // [esp+4h] [ebp-410h] BYREF
    int blockspersnap; // [esp+40Ch] [ebp-8h]
    int curindex; // [esp+410h] [ebp-4h]

    if ( (unsigned int)(cl - svs.clients) >= com_maxclients->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp",
                    3397,
                    0,
                    "cl - svs.clients doesn't index com_maxclients->current.integer\n\t%i not in [0, %i)",
                    cl - svs.clients,
                    com_maxclients->current.integer) )
    {
        __debugbreak();
    }
    if ( cl->downloadName[0] && !cl->clientDownloadingWWW )
    {
        if ( cl->download )
        {
LABEL_22:
            while ( cl->downloadCurrentBlock - cl->downloadClientBlock < 8 && cl->downloadSize != cl->downloadCount )
            {
                curindex = cl->downloadCurrentBlock % 8;
                if ( !cl->downloadBlocks[curindex] )
                    cl->downloadBlocks[curindex] = (unsigned __int8 *)Z_Malloc(2048, "SV_WriteDownloadToClient", 10);
                cl->downloadBlockSize[curindex] = FS_Read(cl->downloadBlocks[curindex], 0x800u, cl->download);
                if ( cl->downloadBlockSize[curindex] < 0 )
                {
                    cl->downloadCount = cl->downloadSize;
                    break;
                }
                cl->downloadCount += cl->downloadBlockSize[curindex];
                ++cl->downloadCurrentBlock;
            }
            if ( cl->downloadCount == cl->downloadSize
                && !cl->downloadEOF
                && cl->downloadCurrentBlock - cl->downloadClientBlock < 8 )
            {
                cl->downloadBlockSize[cl->downloadCurrentBlock++ % 8] = 0;
                cl->downloadEOF = 1;
            }
            rate = cl->rate;
            if ( sv_maxRate->current.integer )
            {
                if ( sv_maxRate->current.integer < 1000 )
                    Dvar_SetInt((dvar_s *)sv_maxRate, 1000);
                if ( sv_maxRate->current.integer < rate )
                    rate = sv_maxRate->current.integer;
            }
            if ( rate )
                blockspersnap = (cl->snapshotMsec * rate / 1000 + 2048) / 2048;
            else
                blockspersnap = 1;
            if ( blockspersnap < 0 )
                blockspersnap = 1;
            while ( 1 )
            {
                v2 = blockspersnap--;
                if ( !v2 || cl->downloadClientBlock == cl->downloadCurrentBlock )
                    break;
                if ( cl->downloadXmitBlock == cl->downloadCurrentBlock )
                {
                    if ( svs.time - cl->downloadSendTime <= 1000 )
                        return;
                    cl->downloadXmitBlock = cl->downloadClientBlock;
                }
                curindex = cl->downloadXmitBlock % 8;
                MSG_WriteByte(msg, 0xBu);
                MSG_WriteLong(msg, cl->downloadXmitBlock);
                if ( !cl->downloadXmitBlock )
                    MSG_WriteLong(msg, cl->downloadSize);
                MSG_WriteShort(msg, cl->downloadBlockSize[curindex]);
                if ( cl->downloadBlockSize[curindex] )
                    MSG_WriteData(msg, cl->downloadBlocks[curindex], cl->downloadBlockSize[curindex]);
                Com_DPrintf(15, "clientDownload: %d : writing block %d\n", cl - svs.clients, cl->downloadXmitBlock);
                ++cl->downloadXmitBlock;
                cl->downloadSendTime = svs.time;
            }
        }
        else if ( sv_allowDownload->current.enabled )
        {
            if ( FS_iwIwd(cl->downloadName, (char *)"main") )
            {
                Com_Printf(15, "clientDownload: %d : \"%s\" cannot download IW iwd files\n", cl - svs.clients, cl->downloadName);
                Com_sprintf(errorMessage, 0x400u, "EXE_CANTAUTODLGAMEPAK %s", cl->downloadName);
                SV_WriteDownloadErrorMessage(cl, msg, errorMessage);
            }
            else
            {
                Com_Printf(15, "clientDownload: %d : beginning \"%s\"\n", cl - svs.clients, cl->downloadName);
                cl->downloadSize = FS_SV_FOpenFileRead(cl->downloadName, 0, &cl->download);
                if ( cl->downloadSize > 0 )
                {
                    if ( !sv_wwwDownload->current.enabled || !cl->wwwOk )
                    {
                        cl->downloadingWWW = 0;
                        cl->downloadXmitBlock = 0;
                        cl->downloadClientBlock = 0;
                        cl->downloadCurrentBlock = 0;
                        cl->downloadCount = 0;
                        cl->downloadEOF = 0;
                        goto LABEL_22;
                    }
                    if ( cl->wwwFallback )
                        cl->wwwFallback = 0;
                    else
                        SV_WWWRedirectClient(cl, msg);
                }
                else
                {
                    Com_Printf(15, "clientDownload: %d : \"%s\" file not found on server\n", cl - svs.clients, cl->downloadName);
                    Com_sprintf(errorMessage, 0x400u, "EXE_AUTODL_FILENOTONSERVER %s", cl->downloadName);
                    SV_WriteDownloadErrorMessage(cl, msg, errorMessage);
                }
            }
        }
        else
        {
            Com_Printf(15, "clientDownload: %d : \"%s\" download disabled", cl - svs.clients, cl->downloadName);
            if ( sv_pure->current.enabled )
                Com_sprintf(errorMessage, 0x400u, "EXE_AUTODL_SERVERDISABLED_PURE %s", cl->downloadName);
            else
                Com_sprintf(errorMessage, 0x400u, "EXE_AUTODL_SERVERDISABLED %s", cl->downloadName);
            SV_WriteDownloadErrorMessage(cl, msg, errorMessage);
        }
    }
}

void __cdecl SV_Disconnect_f(client_t *cl)
{
    if ( (unsigned int)(cl - svs.clients) >= com_maxclients->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp",
                    3592,
                    0,
                    "cl - svs.clients doesn't index com_maxclients->current.integer\n\t%i not in [0, %i)",
                    cl - svs.clients,
                    com_maxclients->current.integer) )
    {
        __debugbreak();
    }
    SV_DropClient(cl, "EXE_DISCONNECTED", 1, 1);
}

void __cdecl SV_VerifyIwds_f(client_t *cl)
{
    int v1; // eax
    const char *v2; // eax
    int v3; // eax
    char *nptr; // [esp+0h] [ebp-2038h]
    int j; // [esp+Ch] [ebp-202Ch]
    int k; // [esp+Ch] [ebp-202Ch]
    const char *v7; // [esp+10h] [ebp-2028h]
    int v8; // [esp+14h] [ebp-2024h]
    int v9; // [esp+14h] [ebp-2024h]
    const char *text_in; // [esp+18h] [ebp-2020h]
    int v11; // [esp+20h] [ebp-2018h]
    int checksumFeed; // [esp+24h] [ebp-2014h]
    unsigned int v13[1024]; // [esp+28h] [ebp-2010h]
    int v14; // [esp+1028h] [ebp-1010h]
    int i; // [esp+102Ch] [ebp-100Ch]
    unsigned int v16[1025]; // [esp+1030h] [ebp-1008h]
    int v17; // [esp+2034h] [ebp-4h]

    v11 = 1;
    v8 = SV_Cmd_Argc();
    v17 = 1;
    if ( v8 >= 2 )
    {
        v7 = SV_Cmd_Argv(v17++);
        if ( *v7 == 35 )
        {
            i = 0;
            while ( v17 < v8 )
            {
                nptr = (char *)SV_Cmd_Argv(v17);
                v1 = atoi(nptr);
                v13[i] = v1;
                ++v17;
                ++i;
            }
            v9 = i - 1;
            for ( i = 0; i < v9; ++i )
            {
                for ( j = 0; j < v9; ++j )
                {
                    if ( i != j && v13[i] == v13[j] )
                    {
                        v11 = 0;
                        break;
                    }
                }
                if ( !v11 )
                    break;
            }
            if ( v11 )
            {
                text_in = FS_LoadedIwdPureChecksums();
                SV_Cmd_TokenizeString(text_in);
                v14 = SV_Cmd_Argc();
                if ( v14 > 1024 )
                    v14 = 1024;
                for ( i = 0; i < v14; ++i )
                {
                    v2 = SV_Cmd_Argv(i);
                    v3 = atoi(v2);
                    v16[i] = v3;
                }
                SV_Cmd_EndTokenizedString();
                for ( i = 0; i < v9; ++i )
                {
                    for ( k = 0; k < v14 && v13[i] != v16[k]; ++k )
                        ;
                    if ( k >= v14 )
                    {
                        v11 = 0;
                        break;
                    }
                }
                if ( v11 )
                {
                    checksumFeed = sv.checksumFeed;
                    for ( i = 0; i < v9; ++i )
                        checksumFeed ^= v13[i];
                    if ( (v9 ^ checksumFeed) != v13[v9] )
                        v11 = 0;
                }
            }
        }
        else
        {
            v11 = 0;
        }
    }
    else
    {
        v11 = 0;
    }
    if ( v11 )
        cl->pureAuthentic = 1;
    else
        cl->pureAuthentic = 2;
}

void __cdecl SV_ResetPureClient_f(client_t *cl)
{
    cl->pureAuthentic = 0;
}

void __cdecl SV_StringToLower(const char *source, char *dest, int size)
{
    char c; // [esp+3h] [ebp-5h]
    int i; // [esp+4h] [ebp-4h]

    for ( i = 0; i < size; ++i )
    {
        c = tolower(*source);
        dest[i] = c;
        if ( !c )
            break;
        ++source;
    }
}

char __cdecl SV_ValidateName(client_t *newcl, const char *name)
{
    const char *String; // eax
    char *v4; // eax
    char *v5; // eax
    char *v6; // eax
    int i; // [esp+10h] [ebp-4Ch]
    client_t *v8; // [esp+14h] [ebp-48h]
    char nameLower[32]; // [esp+18h] [ebp-44h] BYREF
    char tempLower[32]; // [esp+38h] [ebp-24h] BYREF

    if ( strlen(name) < 4 )
        return 0;
    SV_StringToLower(name, nameLower, 32);
    String = Dvar_GetString("sv_noname");
    SV_StringToLower(String, tempLower, 32);
    v4 = strstr(nameLower, tempLower);
    if ( v4 )
        return 0;
    v5 = strstr(nameLower, "unknown soldier");
    if ( v5 )
        return 0;
    v6 = strstr(nameLower, "democlient");
    if ( v6 )
        return 0;
    for ( i = 0; i < com_maxclients->current.integer; ++i )
    {
        v8 = &svs.clients[i];
        if ( v8 != newcl && v8->header.state == CS_ACTIVE )
        {
            SV_StringToLower(v8->name, tempLower, 32);
            if ( !I_strcmp(nameLower, tempLower) )
                return 0;
        }
    }
    return 1;
}

bool __cdecl SV_SanitizeName(const char *name, char *sanitizedName, int size)
{
    unsigned __int8 c; // [esp+3h] [ebp-9h]
    int sanitizedLength; // [esp+4h] [ebp-8h]
    int validCharCount; // [esp+8h] [ebp-4h]

    validCharCount = 0;
    sanitizedLength = 0;
    while ( 1 )
    {
        c = *name;
        if ( !*name )
            break;
        if ( (I_isalnum(c) || validCharCount) && (I_isalnum(c) || c == 32) )
        {
            if ( I_isalnum(c) )
                ++validCharCount;
            sanitizedName[sanitizedLength] = c;
            if ( ++sanitizedLength == size - 1 )
                break;
        }
        ++name;
    }
    sanitizedName[sanitizedLength] = 0;
    return validCharCount >= 4;
}

void __cdecl SV_UserinfoChanged(client_t *cl)
{
    char *v1; // eax
    const char *v2; // eax
    const char *String; // eax
    const char *v4; // eax
    char *v5; // eax
    char *v6; // eax
    int v7; // ecx
    const char *v8; // eax
    char *v9; // eax
    char *v10; // eax
    unsigned int *v11; // ecx
    __int64 v12; // [esp-14h] [ebp-CCh]
    unsigned __int64 v13; // [esp-8h] [ebp-C0h]
    int j; // [esp+3Ch] [ebp-7Ch]
    char filteredName[32]; // [esp+44h] [ebp-74h] BYREF
    int clientNumForXuid; // [esp+64h] [ebp-54h]
    netadr_t addr; // [esp+68h] [ebp-50h] BYREF
    unsigned __int64 newXuid; // [esp+78h] [ebp-40h] BYREF
    const char *val; // [esp+84h] [ebp-34h]
    int i; // [esp+88h] [ebp-30h]
    char cleanName[32]; // [esp+8Ch] [ebp-2Ch] BYREF
    int clientNum; // [esp+B0h] [ebp-8h]
    bool validCharCount; // [esp+B7h] [ebp-1h]

    memset(cleanName, 0, sizeof(cleanName));
    v1 = Info_ValueForKey(cl->userinfo, "name");
    validCharCount = SV_SanitizeName(v1, cleanName, 32);
    if ( cl->bIsTestClient || cl->bIsDemoClient || validCharCount && SV_ValidateName(cl, cleanName) )
    {
        v2 = va("%s", cleanName);
        Info_SetValueForKey(cl->userinfo, "name", v2);
    }
    else
    {
        String = Dvar_GetString("sv_noname");
        validCharCount = SV_SanitizeName(String, cleanName, 32);
        if ( validCharCount )
            v4 = va("%s %d", cleanName, cl - svs.clients);
        else
            v4 = va("%s %d", "Unknown Soldier", cl - svs.clients);
        Info_SetValueForKey(cl->userinfo, "name", v4);
        SV_SendServerCommand(cl, SV_CMD_CAN_IGNORE, "%c \"EXE_CANNOTUSENAME\"", 101);
    }
    v5 = Info_ValueForKey(cl->userinfo, "name");
    I_strncpyz(cl->name, v5, 32);
    v6 = Info_ValueForKey(cl->userinfo, "clanAbbrev");
    I_strncpyz(cl->clanAbbrev, v6, 5);

    val = Info_ValueForKey(cl->userinfo, "rate");
    if ( strlen(val) )
    {
        i = atoi(val);
        cl->rate = i;
        if ( cl->rate >= 1000 )
        {
            if ( cl->rate > 90000 )
                cl->rate = 90000;
        }
        else
        {
            cl->rate = 1000;
        }
    }
    else
    {
        cl->rate = 5000;
    }

    val = Info_ValueForKey(cl->userinfo, "snaps");
    if ( strlen(val) )
    {
        i = atoi(val);
        if ( i >= 1 )
        {
            if ( i > 30 )
                i = 30;
        }
        else
        {
            i = 1;
        }
        cl->snapshotMsec = 1000 / i;
    }
    else
    {
        cl->snapshotMsec = 50;
    }

    val = Info_ValueForKey(cl->userinfo, "cl_voice");
    cl->sendVoice = atoi(val) > 0;
    if ( cl->rate < 5000 )
        cl->sendVoice = 0;

    if ( cl->header.netchan.remoteAddress.type )
    {
        clientNum = cl - svs.clients;
        bcassert(clientNum, com_maxclients->current.integer);

        val = Info_ValueForKey(cl->userinfo, "bdOnlineUserID");
        StringToXUID(val, &newXuid);
        if ( newXuid ) // KISAKTODO: prob should ifdef off this branch
        {
            if ( newXuid != __PAIR64__(LODWORD(svs.mapCenter[2 * clientNum - 63]), svs.xuids[clientNum]) )
            {
                cl->xuid = newXuid;
                if ( LODWORD(svs.mapCenter[2 * clientNum - 63]) | LODWORD(svs.xuids[clientNum]) )
                {
                    HIDWORD(v12) = LODWORD(svs.mapCenter[2 * clientNum - 63]);
                    LODWORD(v12) = svs.xuids[clientNum];
                    Com_PrintWarning(
                        15,
                        "Unregistering xuid %llx because %llx has gotten their clientNum (%i)\n",
                        v12,
                        newXuid,
                        clientNum);
                }
                v7 = clientNum;
                LODWORD(svs.xuids[clientNum]) = newXuid;
                svs.mapCenter[2 * v7 - 63] = *((float *)&newXuid + 1);
            }
            if ( cl->header.netchan.remoteAddress.type == NA_LOOPBACK )
                NET_SetNetAdrLoopbackIP(&addr);
            else
                addr = cl->header.netchan.remoteAddress;
            Com_Printf(15, "Setting client %i XUID to %s\n", clientNum, val);
            if ( LODWORD(svs.mapCenter[2 * clientNum - 63]) | LODWORD(svs.xuids[clientNum]) )
            {
                HIDWORD(v13) = LODWORD(svs.mapCenter[2 * clientNum - 63]);
                LODWORD(v13) = svs.xuids[clientNum];
                clientNumForXuid = Live_GetClientNumForXuid(&g_serverSession, v13);
                if ( clientNumForXuid != clientNum )
                {
                    if ( clientNumForXuid >= 0 )
                    {
                        v8 = va("%s went from client num %i to client num %i", cl->name, clientNumForXuid, clientNum);
                        if ( !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp",
                                        4100,
                                        0,
                                        "%s\n\t%s",
                                        "clientNumForXuid < 0",
                                        v8) )
                            __debugbreak();
                    }
                    Com_ControllerIndexes_GetPrimary();
                }
            }
            if ( !*val
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp", 4153, 0, "%s", "val[0]") )
            {
                __debugbreak();
            }
            I_strncpyz(filteredName, cl->name, 32);
            for ( j = 0; filteredName[j] && j < 32; ++j )
            {
                if ( filteredName[j] == 32 )
                    filteredName[j] = 1;
            }
            v9 = va("%s %s", val, filteredName);
            SV_SetConfigstring(clientNum + 323, v9);
            v10 = Info_ValueForKey(cl->userinfo, "emblem");
            SV_SetConfigstring(clientNum + 355, v10);
        }
        else
        {
            svs.xuids[clientNum] = 0;
            SV_SetConfigstring(clientNum + 355, 0);
            Com_Printf(15, "Player '%s' logged out of Xbox Live\n", cl->name);
        }
        val = Info_ValueForKey(cl->userinfo, "cl_wwwDownload");
        cl->wwwOk = 0;
        if ( strlen(val) )
        {
            i = atoi(val);
            if ( i )
                cl->wwwOk = 1;
        }
    }
}

void __cdecl SV_UpdateUserinfo_f(client_t *cl)
{
    const char *v1; // eax

    if ( (unsigned int)(cl - svs.clients) >= com_maxclients->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp",
                    4214,
                    0,
                    "cl - svs.clients doesn't index com_maxclients->current.integer\n\t%i not in [0, %i)",
                    cl - svs.clients,
                    com_maxclients->current.integer) )
    {
        __debugbreak();
    }
    v1 = SV_Cmd_Argv(1);
    I_strncpyz(cl->userinfo, v1, 1024);
    SV_UserinfoChanged(cl);
    ClientUserinfoChanged(cl - svs.clients);
}

void __cdecl SV_MutePlayer_f(client_t *cl)
{
    const char *v1; // eax
    unsigned int otherClient; // [esp+0h] [ebp-4h]

    v1 = SV_Cmd_Argv(1);
    otherClient = atoi(v1);
    if ( otherClient <= 0x20 )
        cl->muteList[otherClient] = 1;
    else
        Com_Printf(0, "Invalid mute client %i\n", otherClient);
}

void __cdecl SV_UnmutePlayer_f(client_t *cl)
{
    const char *v1; // eax
    unsigned int otherClient; // [esp+0h] [ebp-4h]

    v1 = SV_Cmd_Argv(1);
    otherClient = atoi(v1);
    if ( otherClient <= 0x20 )
        cl->muteList[otherClient] = 0;
    else
        Com_Printf(0, "Invalid unmute client %i\n", otherClient);
}

void __cdecl SV_ExecuteClientCommand(client_t *cl, const char *s, int clientOK, int fromOldServer)
{
    const ucmd_t *u; // [esp+14h] [ebp-4h]

    if ( (unsigned int)(cl - svs.clients) >= com_maxclients->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp",
                    4361,
                    0,
                    "cl - svs.clients doesn't index com_maxclients->current.integer\n\t%i not in [0, %i)",
                    cl - svs.clients,
                    com_maxclients->current.integer) )
    {
        __debugbreak();
    }
    SV_Cmd_TokenizeString(s);
    for ( u = ucmds; u->name; ++u )
    {
        if ( !strcmp(SV_Cmd_Argv(0), u->name) )
        {
            if ( !fromOldServer || u->allowFromOldServer )
                u->func(cl);
            break;
        }
    }
    if ( clientOK && !u->name && SV_Loaded() )
        ClientCommand(cl - svs.clients);
    SV_Cmd_EndTokenizedString();
}

void __cdecl SV_ClientThink(client_t *cl, usercmd_s *cmd)
{
    char *v2; // eax
    char *Name; // [esp+20h] [ebp-8h]

    //Name = va("SV_ClientThink '%s'", cl->name);
    PROF_SCOPED("SV_ClientThink");
    ZoneTextF("'%s'", cl->name);
    if ( (unsigned int)(cl - svs.clients) >= com_maxclients->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp",
                    4512,
                    0,
                    "cl - svs.clients doesn't index com_maxclients->current.integer\n\t%i not in [0, %i)",
                    cl - svs.clients,
                    com_maxclients->current.integer) )
    {
        __debugbreak();
    }
    if ( cmd->serverTime - svs.time > 20000 )
    {
        v2 = va("Invalid command time %i from client %s, current server time is %i", cmd->serverTime, cl->name, svs.time);
        Com_PrintError(15, v2);
        return;
    }
    memcpy(&cl->lastUsercmd, cmd, sizeof(cl->lastUsercmd));
    if ( cl->header.state == CS_ACTIVE )
    {
        G_SetLastServerTime(cl - svs.clients, cmd->serverTime);
        ClientThink(cl - svs.clients);
        return;
    }
}

void __cdecl SV_UserMove(client_t *cl, msg_t *msg, int delta)
{
    const char *v3; // eax
    int v4; // eax
    int v5; // eax
    int v6; // eax
    int v7; // esi
    int v8; // eax
    int v9; // [esp-4h] [ebp-720h]
    char *v10; // [esp+0h] [ebp-71Ch]
    unsigned int v11; // [esp+10h] [ebp-70Ch]
    int v12; // [esp+20h] [ebp-6FCh]
    usercmd_s *k; // [esp+24h] [ebp-6F8h]
    int m; // [esp+2Ch] [ebp-6F0h]
    int j; // [esp+34h] [ebp-6E8h]
    int n; // [esp+3Ch] [ebp-6E0h]
    usercmd_s nullcmd; // [esp+44h] [ebp-6D8h] BYREF
    usercmd_s *oldcmd; // [esp+78h] [ebp-6A4h]
    int cmdCount; // [esp+7Ch] [ebp-6A0h]
    int key; // [esp+80h] [ebp-69Ch]
    usercmd_s cmds[32]; // [esp+84h] [ebp-698h] BYREF
    int i; // [esp+70Ch] [ebp-10h]
    playerState_s *ps; // [esp+710h] [ebp-Ch]
    int value; // [esp+714h] [ebp-8h]
    usercmd_s *cmd; // [esp+718h] [ebp-4h]

    for ( j = 0; j < 2; ++j )
        nullcmd.button_bits.array[j] = 0;
    v12 = 32;
    for ( k = cmds; --v12 >= 0; ++k )
    {
        for ( m = 0; m < 2; ++m )
            k->button_bits.array[m] = 0;
    }
    if ( (unsigned int)(cl - svs.clients) >= com_maxclients->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp",
                    4558,
                    0,
                    "cl - svs.clients doesn't index com_maxclients->current.integer\n\t%i not in [0, %i)",
                    cl - svs.clients,
                    com_maxclients->current.integer) )
    {
        __debugbreak();
    }
    if ( delta )
        cl->header.deltaMessage = cl->messageAcknowledge;
    else
        cl->header.deltaMessage = -1;
    if ( cl->reliableSequence - cl->reliableAcknowledge < 128 )
    {
        cmdCount = MSG_ReadByte(msg);
        if ( cmdCount >= 1 )
        {
            if ( cmdCount <= 32 )
            {
                key = sv.checksumFeed;
                key = cl->messageAcknowledge ^ sv.checksumFeed;
                v4 = Com_HashKey(cl->reliableCommandInfo[cl->reliableAcknowledge & 0x7F].cmd, 32);
                key ^= v4;
                if ( sv_debugMessageKey->current.enabled )
                {
                    v11 = strlen(cl->reliableCommandInfo[cl->reliableAcknowledge & 0x7F].cmd);
                    v10 = cl->reliableCommandInfo[cl->reliableAcknowledge & 0x7F].cmd;
                    v9 = cl->reliableAcknowledge & 0x7F;
                    v5 = Com_HashKey(v10, 32);
                    Com_Printf(
                        15,
                        "key:%i, checksumFeed:%i, messageAcknowledge:%i, Com_HashKey:%i, servercommand(%i):'%s', len:%i\n",
                        key,
                        sv.checksumFeed,
                        cl->messageAcknowledge,
                        v5,
                        v9,
                        v10,
                        v11);
                }
                ps = SV_GameClientNum(cl - svs.clients);
                if ( !ps
                    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp", 4608, 0, "%s", "ps") )
                {
                    __debugbreak();
                }
                if ( !BG_ValidateWeaponNumber(ps->weapon)
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp",
                                4609,
                                0,
                                "%s",
                                "BG_ValidateWeaponNumber( ps->weapon )") )
                {
                    __debugbreak();
                }
                MSG_SetDefaultUserCmd(ps, &nullcmd);
                cmd = &nullcmd;
                oldcmd = &nullcmd;
                for ( i = 0; i < cmdCount; ++i )
                {
                    cmd = &cmds[i];
                    MSG_ReadDeltaUsercmdKey(msg, key, oldcmd, cmd);
                    if ( !BG_IsWeaponValid(ps, cmd->weapon) )
                        cmd->weapon = ps->weapon;
                    if ( ps->offHandIndex != LOWORD(ps->offHandIndex)
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp",
                                    4628,
                                    0,
                                    "%s",
                                    "ps->offHandIndex == static_cast<ushort>( ps->offHandIndex )") )
                    {
                        __debugbreak();
                    }
                    if ( !BG_IsWeaponValid(ps, cmd->offHandIndex) )
                        cmd->offHandIndex = ps->offHandIndex;
                    if ( !BG_ValidateWeaponNumber(cmd->weapon) || !BG_ValidateWeaponNumber(cmd->offHandIndex) )
                    {
                        Com_Printf(15, "###!#!#!#!#!#!#!#!#!#!#!#!#!#!#!#!###\n");
                        Com_Printf(
                            15,
                            "Encountered corrupt user command. This means the client's write key and the server read key for the net me"
                            "ssage were different\n");
                        Com_Printf(15, "Problem occured with client #%i:'%s'\n", cl - svs.clients, cl->name);
                        Com_Printf(15, "Server game time: %i\n", svs.time);
                        Com_Printf(15, "---- Corrupt user command data:\n");
                        Com_Printf(15, "Command was %i of %i in the packet.\n", i, cmdCount);
                        Com_Printf(15, "serverTime = %i\n", cmd->serverTime);
                        Com_Printf(15, "angles[0] = %i(%f)\n", cmd->angles[0], (float)((float)cmd->angles[0] * 0.0054931641));
                        Com_Printf(15, "angles[1] = %i(%f)\n", cmd->angles[1], (float)((float)cmd->angles[1] * 0.0054931641));
                        Com_Printf(15, "angles[2] = %i(%f)\n", cmd->angles[2], (float)((float)cmd->angles[2] * 0.0054931641));
                        Com_Printf(15, "forwardmove = %i\n", cmd->forwardmove);
                        Com_Printf(15, "rightmove = %i\n", cmd->rightmove);
                        for ( n = 0; n < 2; ++n )
                            Com_Printf(15, "buttons(field %i) = %i\n", n, cmd->button_bits.array[n]);
                        Com_Printf(15, "weapon = %i\n", cmd->weapon);
                        Com_Printf(15, "---- %i Client Info\n", cl - svs.clients);
                        switch ( cl->header.state )
                        {
                            case CS_FREE:
                                Com_Printf(15, "state: %s\n", "free");
                                break;
                            case CS_ZOMBIE:
                                Com_Printf(15, "state: %s\n", "zombie");
                                break;
                            case CS_CONNECTED:
                                Com_Printf(15, "state: %s\n", "connected");
                                break;
                            case CS_CLIENTLOADING:
                                Com_Printf(15, "state: %s\n", "clientloading");
                                break;
                            case CS_ACTIVE:
                                Com_Printf(15, "state: %s\n", "active");
                                break;
                            default:
                                Com_Printf(15, "state: %s\n", va("unknown(%i)", cl->header.state));
                                break;
                        }
                        Com_Printf(15, "userinfo: '%s'\n", cl->userinfo);
                        Com_Printf(15, "reliableSequence: %i\n", cl->reliableSequence);
                        Com_Printf(15, "reliableAcknowledge: %i\n", cl->reliableAcknowledge);
                        Com_Printf(15, "reliableSent: %i\n", cl->reliableSent);
                        Com_Printf(15, "messageAcknowledge: %i\n", cl->messageAcknowledge);
                        Com_Printf(15, "gamestateMessageNum: %i\n", cl->gamestateMessageNum);
                        Com_Printf(15, "challenge: %i\n", cl->challenge);
                        Com_Printf(15, "lastClientCommand: %i\n", cl->lastClientCommand);
                        Com_Printf(15, "deltaMessage: %i\n", cl->header.deltaMessage);
                        Com_Printf(15, "nextReliableTime: %i\n", cl->nextReliableTime);
                        Com_Printf(15, "lastPacketTime: %i\n", cl->lastPacketTime);
                        Com_Printf(15, "lastConnectTime: %i\n", cl->lastConnectTime);
                        Com_Printf(15, "nextSnapshotTime: %i\n", cl->nextSnapshotTime);
                        Com_Printf(15, "rateDelayed: %i\n", cl->header.rateDelayed);
                        Com_Printf(15, "timeoutCount: %i\n", cl->timeoutCount);
                        Com_Printf(15, "ping: %i\n", cl->ping);
                        Com_Printf(15, "rate: %i\n", cl->rate);
                        Com_Printf(15, "snapshotMsec: %i\n", cl->snapshotMsec);
                        Com_Printf(15, "pureAuthentic: %i\n", cl->pureAuthentic);
                        Com_Printf(15, "---- Misc Messaging Info\n");
                        Com_Printf(15, "sv.checksumFeed: %i\n", sv.checksumFeed);
                        Com_Printf(15, "cl->messageAcknowledge: %i\n", cl->messageAcknowledge);
                        Com_Printf(15, "cl->reliableAcknowledge: %i\n", cl->reliableAcknowledge);
                        Com_Printf(15, "cl->reliableAcknowledge&(MAX_RELIABLE_COMMANDS-1): %i\n", cl->reliableAcknowledge & 0x7F);
                        Com_Printf(
                            15,
                            "cl->reliableCommandInfo[cl->reliableAcknowledge&(MAX_RELIABLE_COMMANDS-1)].cmd: '%s'\n",
                            cl->reliableCommandInfo[cl->reliableAcknowledge & 0x7F].cmd);
                        v6 = Com_HashKey(cl->reliableCommandInfo[cl->reliableAcknowledge & 0x7F].cmd, 32);
                        Com_Printf(
                            15,
                            "Com_HashKey(cl->reliableCommandInfo[cl->reliableAcknowledge&(MAX_RELIABLE_COMMANDS-1)].cmd,32): %i\n",
                            v6);
                        Com_Printf(15, "key = sv.checksumFeed: %i\n", sv.checksumFeed);
                        Com_Printf(15, "key ^= cl->messageAcknowledge: %i\n", cl->messageAcknowledge ^ sv.checksumFeed);
                        v7 = cl->messageAcknowledge ^ sv.checksumFeed;
                        v8 = Com_HashKey(cl->reliableCommandInfo[cl->reliableAcknowledge & 0x7F].cmd, 32);
                        Com_Printf(
                            15,
                            "key ^= Com_HashKey(cl->reliableCommandInfo[cl->reliableAcknowledge&(MAX_RELIABLE_COMMANDS-1)].cmd,32): %i\n",
                            v8 ^ v7);
                        Com_Printf(15, "key: %i\n", key);
                        Com_Printf(15, "key ^= cmd->serverTime: %i\n", cmd->serverTime ^ key);
                        Com_Printf(15, "########################################\n");
                        SV_DropClient(cl, "Corrupted network messaging detected", 1, 1);
                    }
                    oldcmd = cmd;
                }
                if ( cmdCount > 0 )
                {
                    value = MSG_ReadLong(msg);
                    LODWORD(cl->header.predictedOrigin[0]) = value;
                    value = MSG_ReadLong(msg);
                    LODWORD(cl->header.predictedOrigin[1]) = value;
                    value = MSG_ReadLong(msg);
                    LODWORD(cl->header.predictedOrigin[2]) = value;
                    cl->header.predictedOriginServerTime = MSG_ReadLong(msg);
                    if ( MSG_ReadBit(msg) )
                    {
                        LODWORD(cl->header.vehicle.origin[0]) = MSG_ReadLong(msg);
                        LODWORD(cl->header.vehicle.origin[1]) = MSG_ReadLong(msg);
                        LODWORD(cl->header.vehicle.origin[2]) = MSG_ReadLong(msg);
                        LODWORD(cl->header.vehicle.angles[0]) = MSG_ReadLong(msg);
                        LODWORD(cl->header.vehicle.angles[1]) = MSG_ReadLong(msg);
                        LODWORD(cl->header.vehicle.angles[2]) = MSG_ReadLong(msg);
                        LODWORD(cl->header.vehicle.tVel[0]) = MSG_ReadLong(msg);
                        LODWORD(cl->header.vehicle.tVel[1]) = MSG_ReadLong(msg);
                        LODWORD(cl->header.vehicle.tVel[2]) = MSG_ReadLong(msg);
                        LODWORD(cl->header.vehicle.aVel[0]) = MSG_ReadLong(msg);
                        LODWORD(cl->header.vehicle.aVel[1]) = MSG_ReadLong(msg);
                        LODWORD(cl->header.vehicle.aVel[2]) = MSG_ReadLong(msg);
                        cl->header.vehicle.inVehicle = 1;
                    }
                    else
                    {
                        cl->header.vehicle.inVehicle = 0;
                    }
                }
                if ( cl->frames[cl->messageAcknowledge & 0x1F].messageAcked <= 0 )
                    cl->frames[cl->messageAcknowledge & 0x1F].messageAcked = Sys_Milliseconds();
                if ( cl->header.state == CS_CLIENTLOADING )
                    SV_ClientEnterWorld(cl, cmds);
                if ( !sv_pure->current.enabled || cl->pureAuthentic )
                {
                    if ( cl->header.state == CS_ACTIVE )
                    {
                        for ( i = 0; i < cmdCount; ++i )
                        {
                            if ( cmds[i].serverTime <= cmds[cmdCount - 1].serverTime
                                && cmds[i].serverTime > cl->lastUsercmd.serverTime )
                            {
                                SV_ClientThink(cl, &cmds[i]);
                            }
                        }
                    }
                    else
                    {
                        cl->header.deltaMessage = -1;
                    }
                }
                else
                {
                    SV_DropClient(cl, "EXE_CANNOTVALIDATEPURECLIENT", 1, 1);
                }
            }
            else
            {
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp",
                                4587,
                                0,
                                "cmdCount > MAX_PACKET_USERCMDS") )
                    __debugbreak();
                Com_Printf(15, "cmdCount > MAX_PACKET_USERCMDS\n");
            }
        }
        else
        {
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp", 4580, 0, "cmdCount < 1") )
                __debugbreak();
            Com_Printf(15, "cmdCount < 1\n");
        }
    }
    else
    {
        v3 = va(
                     "cl->reliableSequence is %i, cl->reliableAcknowledge is %i for %s",
                     cl->reliableSequence,
                     cl->reliableAcknowledge,
                     cl->name);
        if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp", 4572, 0, v3) )
            __debugbreak();
    }
}

void __cdecl SV_ExecuteClientMessage(client_t *cl, msg_t *msg)
{
    msg_t v2; // [esp+0h] [ebp-70h] BYREF
    int c; // [esp+30h] [ebp-40h] BYREF
    msg_t msgCompressed; // [esp+34h] [ebp-3Ch] BYREF
    unsigned __int8 (*msgCompressed_buf)[65536]; // [esp+64h] [ebp-Ch]
    LargeLocal msgCompressed_buf_large_local(0x10000); // [esp+68h] [ebp-8h] BYREF

    //LargeLocal::LargeLocal(&msgCompressed_buf_large_local, 0x10000);
    msgCompressed_buf = (unsigned __int8 (*)[65536])msgCompressed_buf_large_local.GetBuf(); // LargeLocal::GetBuf(&msgCompressed_buf_large_local);
    if ( (unsigned int)(cl - svs.clients) >= com_maxclients->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp",
                    4860,
                    0,
                    "cl - svs.clients doesn't index com_maxclients->current.integer\n\t%i not in [0, %i)",
                    cl - svs.clients,
                    com_maxclients->current.integer) )
    {
        __debugbreak();
    }
    MSG_Init(&msgCompressed, (unsigned __int8 *)msgCompressed_buf, 0x10000);
    msgCompressed.cursize = MSG_ReadBitsCompress(
                                                        &msg->data[msg->readcount],
                                                        msg->cursize - msg->readcount,
                                                        (unsigned __int8 *)msgCompressed_buf,
                                                        0x10000u);
    if ( !cl->downloading && !cl->downloadingWWW && !cl->clientDownloadingWWW && cl->serverId != sv_serverId_value )
    {
        if ( (cl->serverId & 0xF0) == (sv_serverId_value & 0xF0) )
        {
            if ( cl->header.state == CS_CLIENTLOADING )
                SV_ClientEnterWorld(cl, &cl->lastUsercmd);
            //LargeLocal::~LargeLocal(&msgCompressed_buf_large_local);
            return;
        }
        if ( SV_ProcessClientCommands(cl, &msgCompressed, 1, &c) && cl->messageAcknowledge > cl->gamestateMessageNum )
        {
            Com_DPrintf(15, "%s : dropped gamestate, resending\n", cl->name);
            Com_Printf(
                15,
                "SV_SendClientGameState() for %s file %s line %i\n",
                cl->name,
                "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp",
                4902);
            SV_SendClientGameState(cl);
        }
        //LargeLocal::~LargeLocal(&msgCompressed_buf_large_local);
        return;
    }

    if (!SV_ProcessClientCommands(cl, &msgCompressed, 0, &c))
        return;

    if ( sv_pure->current.enabled && cl->pureAuthentic == 2 && cl->header.state >= CS_ACTIVE )
    {
        cl->nextSnapshotTime = -1;
        cl->lastSnapshotTime = -1;
        SV_DropClient(cl, "EXE_UNPURECLIENTDETECTED", 1, 1);
        cl->header.state = CS_ACTIVE;
        if ( cl->header.state == CS_ACTIVE || cl->header.state == CS_ZOMBIE)
            SV_BuildClientSnapshot(cl);
        SV_SetServerStaticHeader();
        SV_BeginClientSnapshot(cl, &v2);
        if ( cl->header.state == CS_ACTIVE || cl->header.state == CS_ZOMBIE)
            SV_WriteSnapshotToClient(cl, &v2);
        SV_EndClientSnapshot(cl, &v2);
        SV_GetServerStaticHeader();
        cl->header.state = CS_ZOMBIE;
    }

    iassert(bgs == 0);
    
    if ( c )
    {
        if ( c == 1 )
        {
            SV_UserMove(cl, &msgCompressed, 0);
        }
        else if ( c != 3 )
        {
            Com_PrintWarning(15, "WARNING: bad command byte %i for client %i\n", c, cl - svs.clients);
        }
    }
    else
    {
        SV_UserMove(cl, &msgCompressed, 1);
    }
    iassert(bgs == 0);
    
    //LargeLocal::~LargeLocal(&msgCompressed_buf_large_local);
}

int __cdecl SV_ProcessClientCommands(client_t *cl, msg_t *msg, int fromOldServer, int *lastCommand)
{
    do
    {
        *lastCommand = MSG_ReadBits(msg, 3u);
        if ( *lastCommand == 3 )
            return 1;
        if ( *lastCommand != 2 )
            return 1;
        if ( !SV_ClientCommand(cl, msg, fromOldServer) )
            return 0;
    }
    while ( cl->header.state != CS_ZOMBIE );
    return 0;
}

int __cdecl SV_ClientCommand(client_t *cl, msg_t *msg, int fromOldServer)
{
    int v4; // [esp+0h] [ebp-424h]
    bool v5; // [esp+4h] [ebp-420h]
    int seq; // [esp+Ch] [ebp-418h]
    bool floodprotect; // [esp+13h] [ebp-411h]
    char strBuf[1028]; // [esp+14h] [ebp-410h] BYREF
    const char *s; // [esp+41Ch] [ebp-8h]
    int clientOk; // [esp+420h] [ebp-4h]

    clientOk = 1;
    if ( (unsigned int)(cl - svs.clients) >= com_maxclients->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp",
                    4401,
                    0,
                    "cl - svs.clients doesn't index com_maxclients->current.integer\n\t%i not in [0, %i)",
                    cl - svs.clients,
                    com_maxclients->current.integer) )
    {
        __debugbreak();
    }
    seq = MSG_ReadLong(msg);
    s = MSG_ReadString(msg, strBuf, 0x400u);
    v5 = sv_floodProtect->current.integer && cl->header.state >= CS_ACTIVE && cl->header.netchan.remoteAddress.type != NA_LOOPBACK;
    floodprotect = v5;
    if ( cl->lastClientCommand >= seq )
        return 1;
    if ( sv_showCommands->current.enabled )
        Com_Printf(15, "clientCommand: %i : %s\n", seq, s);
    if ( seq <= cl->lastClientCommand + 1 )
    {
        if ( !I_strncmp("team ", s, 5) || !I_strncmp("score", s, 5) || !I_strncmp("mr ", s, 3) )
            floodprotect = 0;
        if ( cl->nextReliableCount < 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp",
                        4443,
                        0,
                        "%s\n\t(cl->nextReliableCount) = %i",
                        "(cl->nextReliableCount >= 0)",
                        cl->nextReliableCount) )
        {
            __debugbreak();
        }
        if ( fromOldServer || floodprotect && !cl->nextReliableCount && svs.time < cl->nextReliableTime )
        {
            clientOk = 0;
            Com_DPrintf(15, "client text ignored for %s: %s\n", cl->name, s);
        }
        if ( floodprotect )
        {
            if ( !sv_floodProtect->current.integer
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp",
                            4459,
                            0,
                            "%s",
                            "sv_floodProtect->current.integer != 0") )
            {
                __debugbreak();
            }
            if ( svs.time >= cl->nextReliableTime )
            {
                cl->nextReliableCount = sv_floodProtect->current.integer - 1;
            }
            else
            {
                if ( cl->nextReliableCount - 1 > 0 )
                    v4 = cl->nextReliableCount - 1;
                else
                    v4 = 0;
                cl->nextReliableCount = v4;
            }
            cl->nextReliableTime = svs.time + 800;
        }
        SV_ExecuteClientCommand(cl, s, clientOk, fromOldServer);
        cl->lastClientCommand = seq;
        Com_sprintf(cl->lastClientCommandString, 0x400u, "%s", s);
        return 1;
    }
    else
    {
        Com_Printf(15, "Client %s lost %i clientCommands\n", cl->name, seq - cl->lastClientCommand + 1);
        SV_DropClient(cl, "EXE_LOSTRELIABLECOMMANDS", 1, 1);
        return 0;
    }
}

gentity_s *__cdecl SV_AddTestClient()
{
    int LicenseType; // eax
    int Primary; // eax
    netadr_t v4; // [esp-10h] [ebp-4B4h]
    int j; // [esp+8h] [ebp-49Ch]
    client_t *drop; // [esp+Ch] [ebp-498h]
    client_t *dropa; // [esp+Ch] [ebp-498h]
    client_t *dropb; // [esp+Ch] [ebp-498h]
    usercmd_s nullcmd; // [esp+10h] [ebp-494h] BYREF
    char xuidStr[20]; // [esp+44h] [ebp-460h] BYREF
    const char *name; // [esp+58h] [ebp-44Ch]
    char file[1028]; // [esp+5Ch] [ebp-448h] BYREF
    int i; // [esp+460h] [ebp-44h]
    netadr_t a; // [esp+464h] [ebp-40h]
    unsigned __int64 xuid; // [esp+474h] [ebp-30h] BYREF
    char friendName[32]; // [esp+480h] [ebp-24h] BYREF

    for (j = 0; j < 2; ++j)
        nullcmd.button_bits.array[j] = 0;
    if (!com_sv_running->current.enabled
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp",
            4998,
            0,
            "%s",
            "com_sv_running->current.enabled"))
    {
        __debugbreak();
    }
    if (!Dvar_GetInt("developer"))
    {
        LicenseType = SV_GetLicenseType();
        if (SV_IsServerRanked(LicenseType))
            return 0;
    }
    i = 0;
    drop = svs.clients;
    while (i < com_maxclients->current.integer)
    {
        if (drop->bIsTestClient && drop->header.state != CS_ACTIVE && drop->header.state >= CS_CONNECTED)
        {
            Com_Printf(15, "TEST client[%d] '%s' removed\n", i, drop->name);
            SV_DropClient(drop, "EXE_PLAYERKICKED", 0, 1);
        }
        ++i;
        ++drop;
    }
    i = 0;
    for (dropa = svs.clients; i < com_maxclients->current.integer && dropa->header.state; ++dropa)
        ++i;
    if (i == com_maxclients->current.integer)
    {
        Com_Printf(15, "Can't add bot, server is full.\n", dropa->name);
        return 0;
    }
    else
    {
        Primary = Com_ControllerIndexes_GetPrimary();
        if (SV_BotNameRandomFriend(Primary, friendName, 32))
            name = friendName;
        else
            name = SV_BotNameRandom();
        xuid = -256;
        XUIDToString(&xuid, xuidStr);
        sprintf(
            file,
            "connect \"\\cg_predictItems\\1\\cl_punkbuster\\0\\cl_anonymous\\0\\color\\4\\head\\default\\model\\multi\\snaps\\2"
            "0\\rate\\5000\\name\\%s\\clanAbbrev\\3arc\\bdOnlineUserID\\%s\\protocol\\%d\\qport\\%d\"",
            name,
            xuidStr,
            1044,
            botport + 1);
        SV_Cmd_TokenizeString(file);
        *(_DWORD *)a.ip = 0;
        a.addrHandleIndex = 0;
        a.type = NA_BOT;
        *(_DWORD *)&a.port = botport++;
        *(_QWORD *)&v4.type = 0;
        *(_QWORD *)&v4.port = *(unsigned int *)&a.port;
        SV_DirectConnect(v4);
        SV_Cmd_EndTokenizedString();
        i = 0;
        for (dropb = svs.clients;
            i < com_maxclients->current.integer
            && (dropb->header.state == CS_FREE || !NET_CompareBaseAdr(a, dropb->header.netchan.remoteAddress));
            ++dropb)
        {
            ++i;
        }
        if (i == com_maxclients->current.integer)
        {
            return 0;
        }
        else
        {
            dropb->bIsTestClient = 1;
            Com_Printf(
                15,
                "SV_SendClientGameState() for %s file %s line %i\n",
                dropb->name,
                "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp",
                5090);
            SV_SendClientGameState(dropb);
            memset(&nullcmd, 0, sizeof(nullcmd));
            SV_ClientEnterWorld(dropb, &nullcmd);
            return (gentity_s *)((char *)sv.gentities + i * sv.gentitySize);
        }
    }
}

bool __cdecl SV_IsTestClient(unsigned int clientNum)
{
    if ( clientNum >= com_maxclients->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp",
                    5102,
                    0,
                    "clientNum doesn't index com_maxclients->current.integer\n\t%i not in [0, %i)",
                    clientNum,
                    com_maxclients->current.integer) )
    {
        __debugbreak();
    }
    return svs.clients[clientNum].bIsTestClient == 1;
}

bool __cdecl SV_IsDemoClient(unsigned int clientNum)
{
    if ( clientNum >= com_maxclients->current.integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp",
                    5113,
                    0,
                    "clientNum doesn't index com_maxclients->current.integer\n\t%i not in [0, %i)",
                    clientNum,
                    com_maxclients->current.integer) )
    {
        __debugbreak();
    }
    return svs.clients[clientNum].bIsDemoClient == 1;
}

char __cdecl SV_AddDemoClient()
{
    netadr_t v1; // [esp-10h] [ebp-468h]
    int i; // [esp+4h] [ebp-454h]
    client_t *client; // [esp+8h] [ebp-450h]
    usercmd_s nullcmd; // [esp+Ch] [ebp-44Ch] BYREF
    char file[1028]; // [esp+40h] [ebp-418h] BYREF
    netadr_t a; // [esp+444h] [ebp-14h]

    for ( i = 0; i < 2; ++i )
        nullcmd.button_bits.array[i] = 0;
    if ( !Demo_IsRecording() )
        return 0;
    if ( !com_sv_running->current.enabled )
        return 0;
    client = svs.clients;
    if ( svs.clients->header.state != CS_FREE )
    {
        Cbuf_AddText(0, "wait 10; demo_stoprecord\n");
        Com_Printf(15, "SV_AddDemoClient: can not add democlient because client[0] is being used. Recording Stopped.\n");
        return 0;
    }
    else
    {
        sprintf(
            file,
            "connect \"\\cg_predictItems\\1\\cl_punkbuster\\0\\cl_anonymous\\0\\color\\4\\head\\default\\model\\multi\\snaps\\2"
            "0\\rate\\5000\\name\\democlient\\clanAbbrev\\3arc\\protocol\\%d\\qport\\%d\\loller\\%d\"",
            1044,
            1773,
            1773);
        SV_Cmd_TokenizeString(file);
        *(_QWORD *)&a.type = 0;
        *(_QWORD *)&a.port = 1773;
        *(_QWORD *)&v1.type = 0;
        *(_QWORD *)&v1.port = 1773;
        SV_DirectConnect(v1);
        SV_Cmd_EndTokenizedString();
        if ( !NET_CompareBaseAdr(a, client->header.netchan.remoteAddress)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp",
                        5162,
                        0,
                        "democlient needs to be client 0") )
        {
            __debugbreak();
        }
        client->bIsDemoClient = 1;
        Com_Printf(
            15,
            "SV_SendClientGameState() for %s file %s line %i\n",
            client->name,
            "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_client_mp.cpp",
            5166);
        SV_SendClientGameState(client);
        memset(&nullcmd, 0, sizeof(nullcmd));
        SV_ClientEnterWorld(client, &nullcmd);
        Com_Printf(15, "SV_AddDemoClient: democlient added.\n");
        return 1;
    }
}

void __cdecl SV_RemoveDemoClient()
{
    client_t *drop; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    if ( svs.clients )
    {
        i = 0;
        drop = svs.clients;
        while ( i < com_maxclients->current.integer )
        {
            if ( drop && drop->header.state != CS_FREE )
            {
                if ( drop->bIsDemoClient )
                {
                    Com_Printf(15, "SV_RemoveDemoClient: democlient removed %d.\n", i);
                    SV_DropClient(drop, "EXE_DISCONNECTED", 0, 1);
                }
            }
            ++i;
            ++drop;
        }
    }
}

