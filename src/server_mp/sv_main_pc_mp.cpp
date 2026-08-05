#include "sv_main_pc_mp.h"
#include <qcommon/common.h>
#include "sv_main_mp.h"
#include <client_mp/sv_client_mp.h>
#include "sv_init_mp.h"
#include <game_mp/g_main_mp.h>
#include <live/live_win.h>
#include <game_mp/pregame.h>
#include <DW/dwUtils_pc.h>
#include <live/live_storage_pub.h>
#include <ui/ui_playlists.h>
#include <win32/win_main.h>
#include <cstring>
#include <win32/win_shared.h>
#include <game_mp/g_cmds_mp.h>
#include <client/cl_console.h>
#include <live/live_storage_win.h>
#include <client_mp/cl_main_mp.h>

int s_region = 1;
int s_licensetype = -1;


int s_LSGTime;
bool sessionCreated;
bool sessionCreateFinished;
int s_numslotpasses;
reservedslot_t s_reservedSlots[6];

void __cdecl SV_ResetDWState()
{
    Com_Printf(0, "resetting state..\n");
    sessionCreated = 0;
    sessionCreateFinished = 0;
}

int __cdecl SV_GetRegion()
{
    return s_region;
}

void __cdecl SV_SetRegion(int region)
{
    s_region = region;
}

void __cdecl SV_SetTime(int time)
{
    s_LSGTime = time;
}

int __cdecl SV_GetTime()
{
    if ( !s_LSGTime
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_main_pc_mp.cpp",
                    199,
                    0,
                    "%s",
                    "0 != s_LSGTime") )
    {
        __debugbreak();
    }
    return s_LSGTime;
}

void __cdecl SV_DWUserDisconnected(unsigned __int64 userID)
{
    int i; // [esp+8h] [ebp-4h]

    if ( svs.clients )
    {
        for ( i = 0; i < com_maxclients->current.integer; ++i )
        {
            if ( svs.clients[i].header.state >= CS_CONNECTED
                && !svs.clients[i].bIsTestClient
                && !svs.clients[i].bIsDemoClient
                && svs.clients[i].dw_userID == userID )
            {
                Com_Printf(
                    0,
                    "Treason uncloaked! User %s is still connected, but the LSG says they're gone!\n",
                    svs.clients[i].name);
                SV_DropClient(&svs.clients[i], "EXE_DISCONNECTED", 1, 1);
                return;
            }
        }
    }
    else
    {
        Com_DPrintf(15, "Got user disconnected notification, but svs.clients is NULL.\n");
    }
}

int __cdecl SV_GetSlotForPasswordIfFree(const char *password)
{
    char *v1; // eax
    int i; // [esp+0h] [ebp-8h]
    int slot; // [esp+4h] [ebp-4h]

    slot = -1;
    for ( i = 0; i < s_numslotpasses; ++i )
    {
        if ( !I_strcmp(password, s_reservedSlots[i].pass) )
        {
            if ( !s_reservedSlots[i].inUse
                || (v1 = Info_ValueForKey(svs.clients[i].userinfo, "password"), I_strcmp(v1, s_reservedSlots[i].pass)) )
            {
                s_reservedSlots[i].inUse = 1;
                return i;
            }
            else
            {
                Com_Printf(15, "Reserved slot %d in use by %s.\n", i, svs.clients[i].name);
            }
            return slot;
        }
    }
    return slot;
}

void __cdecl SV_FreeReservedSlot(int slot)
{
    if ( (slot < 0 || slot > s_numslotpasses)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_main_pc_mp.cpp",
                    284,
                    0,
                    "%s",
                    "slot >= 0 && slot <= s_numslotpasses") )
    {
        __debugbreak();
    }
    if ( !s_reservedSlots[slot].inUse
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_main_pc_mp.cpp",
                    285,
                    0,
                    "%s",
                    "s_reservedSlots[slot].inUse") )
    {
        __debugbreak();
    }
    if ( slot >= 0 && slot <= s_numslotpasses )
        s_reservedSlots[slot].inUse = 0;
}

int __cdecl SV_DropClientForReservedSlot(const char *password)
{
    int slot; // [esp+0h] [ebp-4h]

    slot = SV_GetSlotForPasswordIfFree(password);
    if ( !Demo_IsEnabled() || slot )
    {
        if ( slot >= 0 )
        {
            if ( (unsigned int)slot >= com_maxclients->current.integer
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_main_pc_mp.cpp",
                            309,
                            0,
                            "slot doesn't index com_maxclients->current.integer\n\t%i not in [0, %i)",
                            slot,
                            com_maxclients->current.integer) )
            {
                __debugbreak();
            }
            SV_DropClient(&svs.clients[slot], "EXE_SERVERISFULL", 1, 1);
        }
        return slot;
    }
    else
    {
        Com_DPrintf(15, "Reserved slot 0 can not be used when film recording is enabled.\n");
        return -1;
    }
}

void __cdecl SV_SetLicenseType(int licensetype)
{
    if ( licensetype == 5 )
        Com_Error(ERR_FATAL, "Incorrect licensetype for server!\n");
    else
        s_licensetype = licensetype;
}

int __cdecl SV_GetLicenseType()
{
    if ( Dvar_GetInt("sv_forcelicensetype") )
        return Dvar_GetInt("sv_forcelicensetype");
    else
        return s_licensetype;
}

bool __cdecl SV_IsWagerServer()
{
    return xblive_wagermatch && xblive_wagermatch->current.enabled;
}

const char *__cdecl SV_GetMapName()
{
    return sv_mapname->current.string;
}

const char *__cdecl SV_GetHostName()
{
    return sv_hostname->current.string;
}

unsigned int __cdecl SV_GetClientCount()
{
    int i; // [esp+0h] [ebp-8h]
    unsigned int clientCount; // [esp+4h] [ebp-4h]

    clientCount = 0;
    if ( com_sv_running && com_sv_running->current.enabled )
    {
        if ( xblive_wagermatch && xblive_wagermatch->current.enabled && Pregame_GetState() == PREGAME_GAMESTARTED )
        {
            return sv_maxclients->current.unsignedInt;
        }
        else
        {
            for ( i = 0; i < com_maxclients->current.integer; ++i )
            {
                if ( svs.clients[i].header.state >= CS_CONNECTED )
                    ++clientCount;
            }
        }
    }
    return clientCount;
}

bool __cdecl SV_HasInfoChanged()
{
#ifdef KISAK_LIVE
    int LicenseType; // eax
    int v1; // eax
    bool retval; // [esp+7h] [ebp-1h]

    retval = 0;
    if ( I_strcmp(g_matchmakingInfo->m_membermapname, sv_mapname->current.string) )
    {
        I_strncpyz(g_matchmakingInfo->m_membermapname, sv_mapname->current.string, 32);
        retval = 1;
    }
    if ( I_strcmp(g_matchmakingInfo->m_memberservername, sv_hostname->current.string) )
    {
        I_strncpyz(g_matchmakingInfo->m_memberservername, sv_hostname->current.string, 64);
        retval = 1;
    }
    if ( sv_maxclients->current.integer != g_matchmakingInfo->m_maxPlayers )
    {
        g_matchmakingInfo->m_maxPlayers = sv_maxclients->current.unsignedInt;
        retval = 1;
    }
#ifdef KISAK_LIVE
    if ( Com_GametypeToInt(g_gametype->current.string) != g_matchmakingInfo->m_memberGAME_TYPE )
    {
        g_matchmakingInfo->m_memberGAME_TYPE = Com_GametypeToInt(g_gametype->current.string);
        retval = 1;
    }
#endif
    if ( LiveStorage_DoWeHavePlaylists() )
    {
        if ( g_matchmakingInfo->m_memberPLAYLIST_VERSION != Playlist_GetVersionNumber() )
        {
            g_matchmakingInfo->m_memberPLAYLIST_VERSION = Playlist_GetVersionNumber();
            retval = 1;
        }
        if ( playlist && g_matchmakingInfo->m_memberPLAYLIST != playlist->current.integer )
        {
            g_matchmakingInfo->m_memberPLAYLIST = playlist->current.integer;
            retval = 1;
        }
    }
    switch ( g_matchmakingInfo->m_memberserverType )
    {
        case 0:
        case 1:
            if ( SV_IsWagerServer() )
            {
                g_matchmakingInfo->m_memberserverType = 2;
                retval = 1;
            }
            break;
        case 2:
            if ( !SV_IsWagerServer() )
            {
                LicenseType = SV_GetLicenseType();
                g_matchmakingInfo->m_memberserverType = SV_IsServerRanked(LicenseType);
                retval = 1;
            }
            break;
        case 0x63:
            if ( SV_IsWagerServer() )
            {
                g_matchmakingInfo->m_memberserverType = 2;
            }
            else
            {
                v1 = SV_GetLicenseType();
                g_matchmakingInfo->m_memberserverType = SV_IsServerRanked(v1);
            }
            retval = 1;
            break;
        default:
            if ( !"Unhandled server type %i\n"
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_main_pc_mp.cpp",
                            503,
                            0,
                            "%s\n\t%s",
                            "\"Unhandled server type %i\\n\"",
                            (const char *)g_matchmakingInfo->m_memberserverType) )
            {
                __debugbreak();
            }
            break;
    }
    return retval;
#else
    return false;
#endif
}

bool __cdecl SV_IsServerRanked(int licensetype)
{
    if ( Dvar_GetBool("sv_forceunranked") )
        return 0;
    return licensetype == 4 || licensetype == 2;
}

void __cdecl SV_MasterHeartbeat(int controllerIndex)
{
#ifdef KISAK_LIVE
    int LicenseType; // eax
    bdTrulyRandomImpl *Instance; // eax
    const struct bdSecurityID *SessionSecID; // eax
    const struct bdSecurityID *v4; // eax
    unsigned intv5; // esi
    unsigned intv6; // esi
    int PlaylistFetchedTime; // eax
    int v8; // eax
    unsigned int v9; // ecx
    client_t *clients; // edx
    const bdSessionID *SessionID; // eax
    const bdSessionID *v12; // eax
    unsigned int v13; // [esp-4h] [ebp-1FCh]
    bdReference<bdCommonAddr> v14; // [esp+60h] [ebp-198h] BYREF
    bdReference<bdCommonAddr> v15; // [esp+64h] [ebp-194h] BYREF
    client_t *drop; // [esp+68h] [ebp-190h]
    int j; // [esp+6Ch] [ebp-18Ch]
    bdMatchMaking *v18; // [esp+70h] [ebp-188h]
    bdMatchMaking *matchmaking; // [esp+74h] [ebp-184h]
    int i; // [esp+78h] [ebp-180h]
    unsigned int notifyCount; // [esp+7Ch] [ebp-17Ch]
    unsigned __int64 toNotify[32]; // [esp+80h] [ebp-178h] BYREF
    char tmp[68]; // [esp+180h] [ebp-78h] BYREF
    bdQoSProbe *qosProbe; // [esp+1C4h] [ebp-34h]
    bdSecurityKey seckey; // [esp+1C8h] [ebp-30h] BYREF
    taskCompleteResults result; // [esp+1D8h] [ebp-20h]
    bdSecurityKey secKey; // [esp+1DCh] [ebp-1Ch] BYREF
    bdQoSProbe *qosprobe; // [esp+1F0h] [ebp-8h]
    bool forceSessionUpdate; // [esp+1F7h] [ebp-1h]

    if ( !G_ExitAfterToolComplete() )
    {
        forceSessionUpdate = 0;
        if ( !sv_authenticating )
            sv_authenticating = _Dvar_RegisterBool("sv_authenticating", 0, 0x40u, "");
        if ( sessionCreateFinished )
        {
            qosprobe = dwGetQoSProbe();
            bdQoSProbe::pump(qosprobe);
        }
        if ( (_S1_9 & 1) == 0 )
        {
            _S1_9 |= 1u;
            sessionupdatetimer = svs.g_msgTime;
        }
        if ( g_svdedicatedauthstate == SV_DWAUTHORIZED )
        {
            LiveStorage_CheckOngoingSessionTasks();
            if ( sessionCreated || (int)(Sys_Milliseconds() - lastdwattempt) <= s_dwBackOff )
            {
                if ( !sessionCreateFinished )
                {
                    result = dwCreateSessionComplete(createSessionTask);
                    if ( result == TASK_COMPLETE )
                    {
                        Com_Printf(0, "Session creation complete!\n");
                        sessionCreateFinished = 1;
                        Dvar_SetBool((dvar_s *)sv_dwlsgerror, 0);
                        lastdwattempt = 0;
                        s_dwBackOff = 2000;
                        qosProbe = dwGetQoSProbe();
                        memset(tmp, 0, 64);
                        bdSecurityKey::bdSecurityKey(&seckey);
                        SessionSecID = dwGetSessionSecID();
                        bdSecurityInfo::toString(SessionSecID, tmp, 0x24u);
                        Com_Printf(0, "listening for secid %s\n", tmp);
                        MatchMakingInfo::getSessionKey(g_matchmakingInfo, &seckey);
                        bdSecurityInfo::toString(&seckey, tmp, 0x24u);
                        Com_Printf(0, "listening for seckey %s\n", tmp);
                        if ( qosProbe && (v4 = dwGetSessionSecID(), bdQoSProbe::listen(qosProbe, v4, 0, 0)) )
                            Com_Printf(0, "Qos listener started\n");
                        else
                            SV_QuitIfNeeded();
                        LiveCounter_Init();
                        LiveCounter_SetupCounters();
                        s_lastDwGoodUpdate = Sys_Milliseconds();
                    }
                    else if ( result == TASK_ERROR )
                    {
                        SV_QuitIfNeeded();
                        sessionCreated = 0;
                        sessionCreateFinished = 1;
                        s_dwBackOff *= 2;
                    }
                }
            }
            else
            {
                lastdwattempt = Sys_Milliseconds();
                createSessionTask = TaskManager_GetOpenTaskSlot(overlappedTasks_1, 0, 2);
                g_matchmakingInfo->m_memberNETCODE_VERSION = 1044;
                if ( SV_IsWagerServer() )
                {
                    g_matchmakingInfo->m_memberserverType = 2;
                }
                else
                {
                    LicenseType = SV_GetLicenseType();
                    g_matchmakingInfo->m_memberserverType = SV_IsServerRanked(LicenseType);
                }
                I_strncpyz(g_matchmakingInfo->m_membermapname, sv_mapname->current.string, 32);
                I_strncpyz(g_matchmakingInfo->m_memberservername, sv_hostname->current.string, 64);
                g_matchmakingInfo->m_memberGAME_TYPE = Com_GametypeToInt(g_gametype->current.string);
                g_matchmakingInfo->m_maxPlayers = sv_maxclients->current.unsignedInt;
                g_matchmakingInfo->m_memberLICENSE = SV_GetLicenseType();
                if ( Dvar_GetBool("sv_nocacvalidate") )
                    g_matchmakingInfo->m_memberDDL_VERSION = LiveStats_GetStatsDDL()->version + 100;
                else
                    g_matchmakingInfo->m_memberDDL_VERSION = LiveStats_GetStatsDDL()->version;
                bdSecurityKey::bdSecurityKey(&secKey);
                Instance = bdSingleton<bdTrulyRandomImpl>::getInstance();
                bdTrulyRandomImpl::getRandomUByte8(Instance, secKey.ab, 0x10u);
                MatchMakingInfo::setSessionKey(g_matchmakingInfo, &secKey);
                dwGetOnlineUserID(0, &g_matchmakingInfo->m_memberdemonwareID);
                g_matchmakingInfo->m_memberGEOGRAPHICAL_LOCATION = SV_GetRegion();
                dwCreateSession((bdReference<bdCommonAddr> *)createSessionTask, g_matchmakingInfo);
                sessionCreated = 1;
                sessionCreateFinished = 0;
                SV_DWInitStats();
                SV_ReadWhiteListfile();
            }
            if ( outstandingnotifyjoin && bdRemoteTask::getStatus(notifyJoinTask) != BD_PENDING )
            {
                Com_Printf(0, "SERVER: notifyjoin complete!\n");
                outstandingnotifyjoin = 0;
            }
            if ( outstandingnotifyleave && bdRemoteTask::getStatus(notifyLeaveTask) != BD_PENDING )
            {
                Com_Printf(0, "SERVER: notifyleave complete!\n");
                outstandingnotifyleave = 0;
            }
            if ( sessionCreateFinished )
                forceSessionUpdate = SV_HasInfoChanged();
            if ( !updateSessionTask && (svs.g_msgTime - sessionupdatetimer > 180000 || forceSessionUpdate && sessionCreateFinished) )
            {
                updateSessionTask = TaskManager_GetOpenTaskSlot(overlappedTasks_1, 0, 4);
                dwUpdateSession(updateSessionTask, g_matchmakingInfo);
                if ( forceSessionUpdate )
                    Com_Printf(0, "Server: Updating session - serverinfo change\n");
                else
                    Com_Printf(0, "Server: Updating session (%ims)\n", svs.g_msgTime - sessionupdatetimer);
                sessionupdatetimer = svs.g_msgTime;
                sessionupdatehangtimer = svs.g_msgTime;
                forceSessionUpdate = 0;
                lastdwattempt = Sys_Milliseconds();
            }
            if ( updateSessionTask )
            {
                Com_Printf(0, ".");
                if ( svs.g_msgTime - sessionupdatehangtimer <= 10000 )
                {
                    if ( dwUpdateSessionComplete(updateSessionTask) )
                    {
                        if ( dwUpdateSessionComplete(updateSessionTask) == TASK_COMPLETE )
                        {
                            Com_Printf(0, "Server: Session updated - %ims\n", svs.g_msgTime - sessionupdatehangtimer);
                            Dvar_SetBool((dvar_s *)sv_dwlsgerror, 0);
                            s_lastDwGoodUpdate = Sys_Milliseconds();
                            s_dwBackOff = 2000;
                        }
                        else
                        {
                            Com_PrintWarning(0, "Session update error!\n");
                            Dvar_SetBool((dvar_s *)sv_dwlsgerror, 1);
                            sessionCreated = 0;
                            s_dwBackOff *= 2;
                        }
                        TaskManager_ClearTask(updateSessionTask);
                        updateSessionTask = 0;
                    }
                }
                else
                {
                    Com_Printf(0, "Server: Giving up on session update after %ims :(\n", svs.g_msgTime - sessionupdatehangtimer);
                    TaskManager_ClearTask(updateSessionTask);
                    updateSessionTask = 0;
                    Dvar_SetBool((dvar_s *)sv_dwlsgerror, 1);
                    s_dwBackOff *= 2;
                    sessionCreated = 0;
                }
            }
        }
        if ( sv_dwlsgerror->current.enabled
            && !sv_authenticating->current.enabled
            && (int)(Sys_Milliseconds() - s_lastConnectAttempt) > 1200000 )
        {
            Com_Printf(
                0,
                "\n************************\n************************\nRECREATING SESSION\n************************\n");
            dwNetStart(1);
            while ( g_dwNetStatus == DW_NET_STARTING_ONLINE )
                dwNetPump();
            Dvar_SetBool((dvar_s *)sv_authenticating, 1);
            Com_Printf(0, "reauthenticating dedicated server!\n");
            g_svdedicatedauthstate = SV_DWNOTAUTHORIZED;
            DW_DedicatedLogonStart(0);
        }
        if ( sv_authenticating->current.enabled )
        {
            if ( g_svdedicatedauthstate == SV_DWAUTHORIZING )
                DW_DedicatedLogonComplete(controllerIndex);
            if ( g_svdedicatedauthstate == SV_DWAUTHORIZED )
            {
                Dvar_SetBool((dvar_s *)sv_authenticating, 0);
                Dvar_SetBool((dvar_s *)sv_dwlsgerror, 0);
                sessionCreateFinished = 0;
                sessionCreated = 0;
                lastdwattempt = 0;
                s_dwBackOff = 2000;
            }
            if ( g_svdedicatedauthstate == SV_DWAUTHORIZEFAIL )
            {
                if ( sv_dwlsgerror->current.enabled )
                {
                    Dvar_SetBool((dvar_s *)sv_authenticating, 0);
                    s_lastConnectAttempt = Sys_Milliseconds() + 1200000;
                }
                else
                {
                    Com_Error(ERR_DROP, "Dedicated server authentication failure\n");
                }
            }
        }
        if ( playlist_enabled )
        {
            if ( playlist_enabled->current.enabled && LiveStorage_DoWeHavePlaylists() )
            {
                if ( sv_playlistFetchInterval )
                {
                    v5 = Sys_Milliseconds();
                    if ( (int)(v5 - SV_GetPlaylistFetchedTime()) > 1000 * sv_playlistFetchInterval->current.integer )
                    {
                        v6 = Sys_Milliseconds();
                        PlaylistFetchedTime = SV_GetPlaylistFetchedTime();
                        Com_Printf(15, "It's been %i seconds, fetching playlists\n", (int)(v6 - PlaylistFetchedTime) / 1000);
                        dwEnterDeferredCritsec();
                        LiveStorage_ForceWADFetch();
                        dwLeaveDeferredCritsec();
                    }
                }
            }
        }
        if ( svs.g_msgTime >= svs.nextHeartbeatTime )
        {
            svs.nextHeartbeatTime = svs.g_msgTime + 10000;
            notifyCount = 0;
            for ( i = 0; i < com_maxclients->current.integer; ++i )
            {
                if ( svs.clients[i].header.state >= CS_CONNECTED && !svs.clients[i].bIsTestClient && !svs.clients[i].bIsDemoClient )
                {
                    if ( svs.clients[i].notifyJoin )
                    {
                        v8 = i;
                        v9 = notifyCount;
                        clients = svs.clients;
                        LODWORD(toNotify[notifyCount]) = svs.clients[i].dw_userID;
                        HIDWORD(toNotify[v9]) = HIDWORD(clients[v8].dw_userID);
                        ++notifyCount;
                        svs.clients[i].notifyJoin = 0;
                    }
                }
            }
            if ( notifyCount )
            {
                matchmaking = dwGetMatchmaking(0);
                if ( matchmaking )
                {
                    Com_Printf(0, "Server: Starting notifyjoin for %u clients\n", notifyCount);
                    SessionID = dwGetSessionID();
                    notifyJoinTask = *(bdRemoteTask **)bdMatchMaking::notifyJoin(
                                                                                             matchmaking,
                                                                                             (int)&v15,
                                                                                             (int)SessionID,
                                                                                             (int)toNotify,
                                                                                             notifyCount);
                    bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v15);
                    outstandingnotifyjoin = 1;
                }
            }
            if ( g_notifyLeave[0] )
            {
                for ( notifyCount = 0;
                            notifyCount < 0x20 && HIDWORD(g_notifyLeave[notifyCount]) | LODWORD(g_notifyLeave[notifyCount]);
                            ++notifyCount )
                {
                    ;
                }
                v18 = dwGetMatchmaking(0);
                if ( v18 )
                {
                    v13 = notifyCount;
                    v12 = dwGetSessionID();
                    notifyLeaveTask = *(bdRemoteTask **)bdMatchMaking::notifyLeave(
                                                                                                v18,
                                                                                                (int)&v14,
                                                                                                (int)v12,
                                                                                                (int)g_notifyLeave,
                                                                                                v13);
                    bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v14);
                    outstandingnotifyleave = 1;
                    memset(g_notifyLeave, 0, sizeof(g_notifyLeave));
                }
            }
#ifdef KISAK_LIVE_SERVICE
            if ( live_service && live_service->current.enabled )
            {
                for ( j = 0; j < com_maxclients->current.integer; ++j )
                {
                    drop = &svs.clients[j];
                    if ( drop->lastConnectTime )
                    {
                        if ( drop->dwchallenge )
                        {
                            if ( drop->header.sendAsActive )
                            {
                                Com_DPrintf(15, "Still waiting for challengeresponse from client %s\n", drop->name);
                                if ( svs.g_msgTime - drop->lastConnectTime > 60000 )
                                {
                                    Com_DPrintf(
                                        15,
                                        "DWCHALLENGERESPONSE: Dropping client %s because it's been %i msec and we've had no challengeresponse\n",
                                        drop->name,
                                        svs.g_msgTime - drop->lastConnectTime);
                                    if ( drop->header.state == CS_ACTIVE )
                                        SV_DropClient(drop, "EXE_BAD_CHALLENGE", 1, 1);
                                    else
                                        NET_OutOfBandPrint(NS_SERVER, drop->header.netchan.remoteAddress, "error\nEXE_BAD_CHALLENGE");
                                }
                            }
                        }
                    }
                }
            }
#endif
        }
    }
#endif
}

void SV_ReadWhiteListfile()
{
    char *v0; // eax
    _iobuf *fp; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    i = 0;
    fp = fopen("reservedslots.txt", "r");
    if ( fp )
    {
        while ( !feof(fp) && i < sv_numreservedslots->current.integer && i < com_maxclients->current.integer )
        {
            fgets(s_reservedSlots[i].pass, 24, fp);
            v0 = strchr(s_reservedSlots[i].pass, 0xAu);
            if ( v0 )
                *v0 = 0;
            ++i;
        }
        s_numslotpasses = i;
    }
    Com_DPrintf(15, "Parsed %i reserved slot passwords\n", s_numslotpasses);
}

int s_dwBackOff = 2000;
int s_lastDwGoodUpdate;
int SV_QuitIfNeeded()
{
    int result; // eax

    if (!s_lastDwGoodUpdate || (int)(Sys_Milliseconds() - s_lastDwGoodUpdate) > 10800000)
    {
        Com_PrintWarning(0, "Session creation failed. Killing server\n");
        Sys_Quit();
    }
    result = 2 * s_dwBackOff;
    s_dwBackOff *= 2;
    return result;
}

void __cdecl SV_FlushRedirect(char *outputbuf)
{
    char c; // [esp+13h] [ebp-49Dh]
    char buf[1168]; // [esp+18h] [ebp-498h] BYREF
    int len; // [esp+4ACh] [ebp-4h]

    len = strlen(outputbuf);
    while ( len > 1156 )
    {
        c = outputbuf[1156];
        outputbuf[1156] = 0;
        buf[0] = 1;
        Com_sprintf(&buf[1], 0x48Bu, "print\n%s", outputbuf);
#ifdef KISAK_LIVE
        dwRawSendTo(&svs.redirectAddress, (unsigned __int8 *)buf, 0x48Cu);
#else
        NET_OutOfBandPrint(NS_SERVER, svs.redirectAddress, buf);
#endif
        len -= 1156;
        outputbuf += 1156;
        *outputbuf = c;
    }
    buf[0] = 1;
    Com_sprintf(&buf[1], 0x48Bu, "print\n%s", outputbuf);
#ifdef KISAK_LIVE
    dwRawSendTo(&svs.redirectAddress, (unsigned __int8 *)buf, 0x48Cu);
#else
    NET_OutOfBandPrint(NS_SERVER, svs.redirectAddress, buf);
#endif
}

static int lasttime;
void __cdecl SVC_RemoteCommand(netadr_t from)
{
    char *v1; // eax
    char *v2; // eax
    const char *v3; // eax
    int v4; // [esp-Ch] [ebp-C2Ch]
    const char *v5; // [esp-4h] [ebp-C24h]
    const char *v6; // [esp-4h] [ebp-C24h]
    char remaining[1024]; // [esp+18h] [ebp-C08h] BYREF
    char sv_outputbuf[2032]; // [esp+418h] [ebp-808h] BYREF
    int valid; // [esp+C08h] [ebp-18h]
    int len; // [esp+C0Ch] [ebp-14h]
    const char *password; // [esp+C10h] [ebp-10h]
    int time; // [esp+C14h] [ebp-Ch]
    int i; // [esp+C18h] [ebp-8h]

    time = Sys_Milliseconds();
    if ( !lasttime || time - lasttime >= 500 )
    {
        lasttime = time;
        password = SV_Cmd_Argv(1);
        if ( *(_BYTE *)rcon_password->current.integer && !strcmp(password, rcon_password->current.string) )
        {
            valid = 1;
            v6 = SV_Cmd_Argv(2);
            v2 = NET_AdrToStringDW(from);
            Com_Printf(15, "Rcon from %s:\n%s\n", v2, v6);
        }
        else
        {
            valid = 0;
            v5 = SV_Cmd_Argv(2);
            v1 = NET_AdrToStringDW(from);
            Com_Printf(15, "Bad rcon from %s:\n%s\n", v1, v5);
        }
        svs.redirectAddress = from;
        Com_BeginRedirect(sv_outputbuf, 0x7F0u, SV_FlushRedirect);
        if ( *(_BYTE *)rcon_password->current.integer )
        {
            if ( valid )
            {
                len = 0;
                for ( i = 2; i < SV_Cmd_Argc(); ++i )
                {
                    v4 = len;
                    v3 = SV_Cmd_Argv(i);
                    len = Com_AddToString(v3, remaining, v4, 1024, 1);
                    len = Com_AddToString(" ", remaining, len, 1024, 0);
                }
                if ( len < 1024 )
                {
                    remaining[len] = 0;
                    Con_Restricted_ExecuteBuf(0, 0, remaining);
                }
            }
            else if ( *password )
            {
                Com_Printf(15, "Invalid password.\n");
            }
            else
            {
                Com_Printf(15, "You must log in with 'rcon login <password>' before using 'rcon'.\n");
            }
        }
        else
        {
            Com_Printf(15, "The server must set 'rcon_password' for clients to use 'rcon'.\n");
        }
        Com_EndRedirect();
    }
}

void __cdecl SV_MatchEnd()
{
    void *v0; // ecx

    if ( onlinegame )
    {
        if ( onlinegame->current.enabled )
        {
            Com_DPrintf(0, "\n*******SERVER: SV_MatchEnd called, uploading leaderboards.\n");
            SV_CommitClientLeaderboards();
        }
    }
}

void __cdecl SV_SysLog_LogMessage(int severity, const char *msg)
{
#ifdef KISAK_LIVE
    netadr_t to; // [esp+10h] [ebp-420h] BYREF
    char syslogMsg[1036]; // [esp+20h] [ebp-410h] BYREF

    if ( (int)(Sys_Milliseconds() - lastCall) >= 1000 )
    {
        lastCall = Sys_Milliseconds();
        memset(syslogMsg, 0, 1025);
        to.type = NA_IP;
        to.port = 514;
        to.ip[0] = -48;
        to.ip[1] = -89;
        to.ip[2] = -7;
        to.ip[3] = -95;
        Com_sprintf(syslogMsg, 0x400u, "<%u>BLACKOPS: %s: %s", severity + 128, sv_hostname->current.string, msg);
        dwSendSysLogPacket(&to, syslogMsg, &syslogMsg[strlen(syslogMsg) + 1] - &syslogMsg[1]);
    }
#endif
}

void __cdecl SV_SysLog_LogMessage_f()
{
    const char *v0; // eax
    const char *message; // [esp+18h] [ebp-8h]
    int severity; // [esp+1Ch] [ebp-4h]

    if ( Cmd_Argc() == 3 )
    {
        message = Cmd_Argv(2);
        v0 = Cmd_Argv(1);
        severity = atoi(v0);
        SV_SysLog_LogMessage(severity, message);
    }
    else
    {
        Com_Printf(0, "Usage: logmessage severity message\n");
    }
}

unsigned __int64 __cdecl SV_GetOwnerID()
{
    unsigned __int64 retval; // [esp+0h] [ebp-8h] BYREF

    retval = 0;
    if ( sv_ownerid && *(_BYTE *)sv_ownerid->current.integer )
        StringToXUID(sv_ownerid->current.string, &retval);
    return retval;
}

bool __cdecl SV_CanLoadCustomGameType()
{
    int LicenseType; // eax

    LicenseType = SV_GetLicenseType();
    return !SV_IsServerRanked(LicenseType) && !playlist_enabled->current.enabled;
}

void __cdecl SV_RegisterRconKey_f()
{
#ifdef KISAK_LIVE
    unsigned __int8 *v0; // eax
    unsigned int v1; // [esp+0h] [ebp-5Ch]
    int v2; // [esp+18h] [ebp-44h]
    int v3; // [esp+1Ch] [ebp-40h]
    bdSecurityID rconkeyIdString; // [esp+30h] [ebp-2Ch] BYREF
    bdSecurityID rconkeyID; // [esp+3Ch] [ebp-20h] BYREF
    int size; // [esp+44h] [ebp-18h]
    bdSecurityKey rconkey; // [esp+48h] [ebp-14h] BYREF

    if ( Cmd_Argc() == 2 )
    {
        strcpy((char *)&rconkeyIdString, "rconkey1");
        bdSecurityKey::bdSecurityKey(&rconkey);
        bdSecurityID::bdSecurityID(&rconkeyID);
        memset(&rconkey, 0, sizeof(rconkey));
        *(unsigned int *)rconkeyID.ab = 0;
        *(unsigned int *)&rconkeyID.ab[4] = 0;
        v2 = strlen(Cmd_Argv(1));
        if ( v2 < 16 )
            v3 = v2;
        else
            v3 = 16;
        if ( v3 > 1 )
            v1 = v3;
        else
            v1 = 1;
        size = v1;
        v0 = (unsigned __int8 *)Cmd_Argv(1);
        memcpy(rconkey.ab, v0, v1);
        rconkeyID = rconkeyIdString;
        dwRegisterSecIDAndKey(&rconkeyID, &rconkey);
    }
    else
    {
        Com_Printf(15, "usage: setrconkey key\n");
    }
#endif
}

