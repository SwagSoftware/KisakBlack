#include "live_win.h"
#include <win32/win_tasks.h>
#include "live_steam.h"
#include <DW/dwLogOn_pc.h>
#include "live_friends_pc.h"
#include <qcommon/com_clients.h>
#include <client/client.h>
#include <client_mp/cl_main_pc_mp.h>
#include <game_mp/g_main_mp.h>
#include "live_sessions_win.h"
#include "live_groups_dw.h"
#include "live_pcache.h"
#include "live_meetplayer.h"
#include "live_leaderboard.h"
#include "live_counter.h"
#include <DW/dwUtils_pc.h>
#include "live_steam_achievements.h"
#include <universal/com_files.h>
#include <win32/win_net.h>
#include <DW/dwStats.h>
#include <win32/win_shared.h>
#include <win32/win_gamerprofile.h>
#include "live_storage_win.h"
#include <DW/dwUtils.h>
#include <client/splitscreen.h>
#include <server_mp/sv_main_mp.h>
#include <gfx_d3d/r_rendercmds.h>

const char *bot_difficulties[5] = { "easy", "normal", "hard", "fu", NULL };

PrivateProfileInfo s_profileInfo;
favourite_t s_favourites[42];
recentServer_t s_recentServers[30];
unsigned __int8 s_recentServersBuf[480];
unsigned int s_blockedListCount;
unsigned __int64 s_blockedList[50];

bool g_shouldComError;
const char *g_comErrorString;

bool g_presenceSecKeyAndIDRegistered;
bool s_updatePerformanceValues;
int s_performanceValueTimer;
int s_uploadBitsPerSec;

MatchMakingInfo *g_matchmakingInfo;
overlappedTask overlappedTasks_3[32];
XenonUserData xenonUserData[1];
unsigned __int64 s_lastInvite;

unsigned __int64 s_selectedPlayerXUID;
unsigned __int64 s_selectedMetPlayerXUID;

int s_signInRequirement[1];

bool g_shouldWeHost = true;



const dvar_t *live_service;
const dvar_t *dw_loggedin;
const dvar_t *dw_active;
const dvar_t *pc_newversionavailable;
const dvar_t *dw_dupe_key;
const dvar_t *xblive_loggedin;
const dvar_t *xenon_voiceDebug;
const dvar_t *xenon_voiceDegrade;
const dvar_t *getdlcmapsfrommaindrive;
const dvar_t *session_nonblocking;
const dvar_t *systemUiActive;
const dvar_t *xblive_showmarketplace;
const dvar_t *xblive_clanmatch;
const dvar_t *xblive_theater;
const dvar_t *xblive_hostingprivateparty;
const dvar_t *xblive_privatepartyclient;
const dvar_t *xblive_wagermatch;
const dvar_t *xblive_basictraining;
const dvar_t *xblive_basictraining_popup;
const dvar_t *bot_tips;
const dvar_t *party_simulateLongQoS;
const dvar_t *xblive_clanListChanged;
const dvar_t *teamsplitter_verbose;
const dvar_t *xblive_matchEndingSoon;
const dvar_t *ui_isClanMember;
const dvar_t *dw_numaccounts;
const dvar_t *xenon_maxVoicePacketsPerSec;
const dvar_t *xenon_maxVoicePacketsPerSecForServer;
const dvar_t *bandwidth_retry_interval;
const dvar_t *xblive_mappacks;
const dvar_t *bot_friends;
const dvar_t *bot_enemies;
const dvar_t *invite_waitPeriod;
const dvar_t *steamid;
const dvar_t *dw_popup;
const dvar_t *inviteText;
const dvar_t *scr_bot_difficulty;
const dvar_t *clancard_clanid;
//const dvar_t *clanName;
const dvar_t *bot_difficulty;
const dvar_t *dw_usernames[5];



void __cdecl Live_ClearDWOverlappedTasks()
{
    TaskManager_ClearOverlappedTasks(overlappedTasks_3);
}

char __cdecl Live_ContentRatingAllowed()
{
    if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_win.cpp", 603, 0, "Not implemented for PC") )
        __debugbreak();
    return 1;
}

bool __cdecl Live_IsUserSignedInToLive()
{
    return LiveSteam_IsClientSignedInOnline();
}

bool __cdecl Live_IsUserSignedInToDemonware(int controllerIndex)
{
#ifdef KISAK_LIVE
    return dwGetLogOnStatus(controllerIndex) == 4;
#else
    return false;
#endif
}

char __cdecl Live_RequireUserToPlayOnline()
{
    if ( LiveSteam_IsClientSignedInOnline() )
    {
        if ( !Live_IsUserSignedInToLive()
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_win.cpp",
                        718,
                        0,
                        "%s",
                        "Live_IsUserSignedInToLive( controllerIndex )") )
        {
            __debugbreak();
        }
        return 1;
    }
    else
    {
        //BLOPS_NULLSUB();
        return 0;
    }
}

char *__cdecl Live_ControllerIndex_GetClientName(int controllerIndex)
{
    return (char*)"kisak"; // KISAKTODO: idc
    //if ( controllerIndex < 0
    //    && !Assert_MyHandler(
    //                "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_win.cpp",
    //                809,
    //                0,
    //                "%s",
    //                (const char *)&stru_D50258.alloc) )
    //{
    //    __debugbreak();
    //}
    //if ( controllerIndex >= 1
    //    && !Assert_MyHandler(
    //                "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_win.cpp",
    //                810,
    //                0,
    //                "%s",
    //                "controllerIndex < MAX_GPAD_COUNT") )
    //{
    //    __debugbreak();
    //}
    //return &byte_A61C02C[80 * controllerIndex];
}

int __cdecl CL_ControllerIndex_GetSignInState(int controllerIndex)
{
    if ( controllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_win.cpp",
                    826,
                    0,
                    "%s\n\t(controllerIndex) = %i",
                    "(controllerIndex >= 0 && controllerIndex < 1)",
                    controllerIndex) )
    {
        __debugbreak();
    }
    return xenonUserData[controllerIndex].signinState;
}

void __cdecl Live_InitiateDemonWareConnect_f()
{
#ifdef KISAK_LIVE_SERVICE
    if ( dwGetLogOnStatus(0) == 2 )
    {
        dword_33267DC[0] = 0;
        dw_disconnect_detected = 0;
        dwLogOnStart(0);
    }
#endif
}

void __cdecl Live_SendInvite_f()
{
    const char *friendName; // [esp+10h] [ebp-4h]

    if ( Cmd_Argc() >= 2 )
    {
        friendName = Cmd_Argv(1);
        Live_SendInvite(friendName);
    }
    else
    {
        Com_PrintError(0, (char*)"PLATFORM_STEAM_OFFLINE");
    }
}

void __cdecl Live_AcceptInvite_f()
{
    const char *v0; // eax
    const char *v1; // eax
    unsigned __int64 friendid; // [esp+18h] [ebp-8h] BYREF

    if ( Cmd_Argc() == 2 )
    {
        v0 = Cmd_Argv(1);
        if ( Friend_GetByName(0, v0, &friendid) )
        {
            Live_AcceptInvite(friendid);
        }
        else
        {
            v1 = Cmd_Argv(1);
            //Com_PrintError(23, (char *)&stru_D50258.do_fancy_upsampling, v1);
            Com_PrintError(23, "%s", v1);
        }
    }
    else
    {
        Com_PrintError(0, (char *)"(kisak) need more args"); // (char *)&stru_D50258.output_components);
    }
}

void __cdecl Live_AcceptLastInvite_f()
{
    if ( s_lastInvite )
        Live_AcceptInvite(s_lastInvite);
}

void __cdecl Live_RevokeInvite_f()
{
    const char *v0; // eax
    const char *v1; // eax
    unsigned __int64 friendid; // [esp+18h] [ebp-8h] BYREF

    if ( Cmd_Argc() == 2 )
    {
        v0 = Cmd_Argv(1);
        if ( Friend_GetByName(0, v0, &friendid) )
        {
            Live_RevokeInvite(friendid);
        }
        else
        {
            v1 = Cmd_Argv(1);
            Com_PrintError(23, (char *)"Couldn't get id for friend %s", v1);
        }
    }
    else
    {
        Com_PrintError(0, (char *)"USAGE: revokeinvite <friendname>\n");
    }
}

void __cdecl Live_JoinSessionInProgress_f()
{
    const char *v0; // eax
    unsigned __int64 v1; // rax
    const char *v2; // eax
    unsigned __int64 friendid; // [esp+20h] [ebp-8h] BYREF

    if ( Cmd_Argc() == 2 )
    {
        v0 = Cmd_Argv(1);
        if ( Friend_GetByName(0, v0, &friendid) )
        {
            Live_JoinSessionInProgress(friendid, 0);
        }
        else if ( s_selectedPlayerXUID )
        {
            Live_JoinSessionInProgress(s_selectedPlayerXUID, 0);
        }
        else if ( __PAIR64__(s_selectedMetPlayerXUID, 0) == HIDWORD(s_selectedMetPlayerXUID) )
        {
            v2 = Cmd_Argv(1);
            Com_PrintError(23, (char *)"Couldn't get id for friend %s", v2);
        }
        else
        {
            LODWORD(v1) = LiveMeetPlayer_GetPlayerSessionByID(s_selectedMetPlayerXUID);
            if ( v1 )
                Live_JoinSessionInProgress(v1, 1);
        }
    }
    else
    {
        Com_PrintError(23, (char *)"Usage: joinsession <friendName>\n");
    }
}

void __cdecl Live_ToggleMute_f()
{
    const char *v1; // eax
    int LocalClientNum; // eax
    clientActive_t *LocalClientGlobals; // [esp+14h] [ebp-1Ch]
    unsigned int playerIndex; // [esp+18h] [ebp-18h]
    unsigned __int64 playerXuid; // [esp+20h] [ebp-10h]
    int i; // [esp+28h] [ebp-8h]

    if (Cmd_Argc() == 2)
    {
        v1 = Cmd_Argv(1);
        playerXuid = I_atoi64(v1);
        playerIndex = -1;
        LocalClientNum = Com_ControllerIndex_GetLocalClientNum(0);
        LocalClientGlobals = CL_GetLocalClientGlobals(LocalClientNum);
        if (LocalClientGlobals->snap.valid)
        {
            for (i = 0; i < LocalClientGlobals->snap.numClients; ++i)
            {
                if (LocalClientGlobals->parseClients[((_WORD)i + (unsigned __int16)LocalClientGlobals->snap.parseClientsNum)
                    & 0x7FF].xuid == playerXuid)
                {
                    playerIndex = LocalClientGlobals->parseClients[((_WORD)i
                        + (unsigned __int16)LocalClientGlobals->snap.parseClientsNum)
                        & 0x7FF].clientIndex;
                    break;
                }
            }
        }
        if (playerXuid)
            CL_MutePlayer(0, playerIndex);
        else
            Com_Printf(30, "party_mutePlayer: Invalid player xuid sent.\n");
    }
    else
    {
        Com_Printf(30, "USAGE: mp_mutePlayer <selected player xuid>\n");
    }
}

cmd_function_s Live_InitiateDemonWareConnect_f_VAR;
cmd_function_s Live_UpdateInfoForInGameList_f_VAR;
cmd_function_s Live_JoinSessionInProgress_f_VAR;
cmd_function_s Live_SendInvite_f_VAR;
cmd_function_s Live_AcceptInvite_f_VAR;
cmd_function_s Live_RevokeInvite_f_VAR;
cmd_function_s Live_ListInvites_f_VAR;
cmd_function_s Live_AddPlayerAsFriend_f_VAR;
cmd_function_s Live_AcceptLastInvite_f_VAR;
cmd_function_s Live_ToggleMute_f_VAR;

const char *dwUsers[5] =
{
    "dw_user0",
    "dw_user1",
    "dw_user2",
    "dw_user3",
    "dw_user4",
};

void __cdecl Live_InitPlatform()
{
    int i; // [esp+0h] [ebp-8h]
    int controllerIndex; // [esp+4h] [ebp-4h]

    memset(xenonUserData, 0, sizeof(xenonUserData));
    s_signInRequirement[0] = 0;
    live_service = _Dvar_RegisterBool("live_service", 1, 0x10u, "online service on/off");
    if ( G_ExitAfterToolComplete() )
    {
        Dvar_SetBool((dvar_s *)live_service, 0);
    }
    else
    {
        steamid = _Dvar_RegisterString("steamid", (char *)"", 0x40u, "Player's SteamID");
        dw_loggedin = _Dvar_RegisterBool("dw_loggedin", 0, 0x40u, "Every frame is updated with Demonware login status");
        dw_active = _Dvar_RegisterBool("dw_active", 1, 0, "Pumps Live_Frame() (and hence DW) if true");
        pc_newversionavailable = _Dvar_RegisterBool(
                                                             "pc_newversionavailable",
                                                             0,
                                                             0x40u,
                                                             "True if new version available for download");
        dw_dupe_key = _Dvar_RegisterBool("dw_dupe_key", 0, 0x40u, "True if key-in-use message from Demonware");
        dw_numaccounts = _Dvar_RegisterInt(
                                             "dw_numaccounts",
                                             -1,
                                             -1,
                                             5,
                                             0,
                                             "Number of online accounts registered for the license");
        for ( i = 5;
            i--; 
            dw_usernames[i] = _Dvar_RegisterString(dwUsers[i], (char *)"", 0, "Online user name registered for the license") )
        {
            ;
        }
        dw_popup = _Dvar_RegisterString("dw_popup", (char *)"", 0, "Online services popup");
        Live_InitFavourites();
        for ( controllerIndex = 0; controllerIndex < 1; ++controllerIndex )
            //BLOPS_NULLSUB();
        LiveGroups_Init();
        PCache_Init();
        Cmd_AddCommandInternal(
            "initiateDemonWareConnect",
            Live_InitiateDemonWareConnect_f,
            &Live_InitiateDemonWareConnect_f_VAR);
        Cmd_AddCommandInternal("updateInfoForInGameList", BLOPS_NULLSUB, &Live_UpdateInfoForInGameList_f_VAR);
        Cmd_AddCommandInternal("JoinsessionInProgress", Live_JoinSessionInProgress_f, &Live_JoinSessionInProgress_f_VAR);
        Cmd_AddCommandInternal("sendinvite", Live_SendInvite_f, &Live_SendInvite_f_VAR);
        Cmd_AddCommandInternal("acceptinvite", Live_AcceptInvite_f, &Live_AcceptInvite_f_VAR);
        Cmd_AddCommandInternal("revokeinvite", Live_RevokeInvite_f, &Live_RevokeInvite_f_VAR);
        Cmd_AddCommandInternal("listinvites", Live_ListInvites_f, &Live_ListInvites_f_VAR);
        Cmd_AddCommandInternal("xaddfriend", Live_AddPlayerAsFriend_f, &Live_AddPlayerAsFriend_f_VAR);
        Cmd_AddCommandInternal("acceptInvitation", Live_AcceptLastInvite_f, &Live_AcceptLastInvite_f_VAR);
        Cmd_AddCommandInternal("mp_toggleMute", Live_ToggleMute_f, &Live_ToggleMute_f_VAR);
        Session_Init();
        g_shouldWeHost = 1;
        LB_Init();
        LiveStorage_Init();
        Friends_Init();
        //BLOPS_NULLSUB();
        LiveCounter_Init();
        xblive_loggedin = _Dvar_RegisterBool("xblive_loggedin", 0, 0, "User is logged into online service");
        xenon_voiceDebug = _Dvar_RegisterBool("xenon_voiceDebug", 0, 0, "Debug voice communication");
        xenon_voiceDegrade = _Dvar_RegisterBool("xenon_voiceDegrade", 0, 0, "Degrade voice quality");
        xenon_maxVoicePacketsPerSec = _Dvar_RegisterInt(
                                                                        "xenon_maxVoicePacketsPerSec",
                                                                        100,
                                                                        0,
                                                                        0x7FFFFFFF,
                                                                        0,
                                                                        "Max voice packets per second a client will send");
        xenon_maxVoicePacketsPerSecForServer = _Dvar_RegisterInt(
                                                                                         "xenon_maxVoicePacketsPerSecForServer",
                                                                                         20,
                                                                                         0,
                                                                                         0x7FFFFFFF,
                                                                                         0,
                                                                                         "Max voice packets per second the server will send");
        getdlcmapsfrommaindrive = _Dvar_RegisterBool(
                                                                "getdlcmapsfrommaindrive",
                                                                0,
                                                                0,
                                                                "false = load the dlc maps from content packages, true = load the dlc maps from the local"
                                                                " machine harddrive");
        session_nonblocking = _Dvar_RegisterBool("session_nonblocking", 1, 0, "Non-blocking Session code");
        inviteText = _Dvar_RegisterString("inviteText", (char *)"", 0, "Text to display for the game invite");
        systemUiActive = _Dvar_RegisterBool("systemUiActive", 0, 0, "Is the system UI active");
        bandwidth_retry_interval = _Dvar_RegisterInt(
                                                                 "bandwidth_retry_interval",
                                                                 180000,
                                                                 0,
                                                                 0x7FFFFFFF,
                                                                 0,
                                                                 "Interval at which Bandwidth test will be retried");
        xblive_showmarketplace = _Dvar_RegisterBool(
                                                             "xblive_showmarketplace",
                                                             0,
                                                             0,
                                                             "true if the player doesn't have all the map packs");
        xblive_clanmatch = _Dvar_RegisterBool("xblive_clanmatch", 0, 0, "Current game is a clan match");
        xblive_theater = _Dvar_RegisterBool("xblive_theater", 0, 0, "Current game is a theater mode");
        xblive_hostingprivateparty = _Dvar_RegisterBool(
                                                                     "xblive_hostingprivateparty",
                                                                     0,
                                                                     0,
                                                                     "true only if we're hosting a party");
        xblive_privatepartyclient = _Dvar_RegisterBool(
                                                                    "xblive_privatepartyclient",
                                                                    0,
                                                                    0,
                                                                    "true only if we're in a party and not the host");
        xblive_mappacks = _Dvar_RegisterInt(
                                                "xblive_mappacks",
                                                0,
                                                0,
                                                2,
                                                0,
                                                "0 = original maps only, 1 = new maps only, 2 = both original and new");
        xblive_wagermatch = _Dvar_RegisterBool("xblive_wagermatch", 0, 4u, "Current game is a wager match");
        xblive_basictraining = _Dvar_RegisterBool("xblive_basictraining", 0, 4u, "Current game is basic training");
        xblive_basictraining_popup = _Dvar_RegisterBool(
                                                                     "xblive_basictraining_popup",
                                                                     0,
                                                                     1u,
                                                                     "The user has seen the basic training popup description");
        scr_bot_difficulty = _Dvar_RegisterString(
                                                     "scr_bot_difficulty",
                                                     (char *)"",
                                                     1u,
                                                     "Difficulty level of the basic training bots.");
        bot_friends = _Dvar_RegisterInt("bot_friends", 6, 1, 11, 0, "Number of friends allowed in basic training");
        bot_enemies = _Dvar_RegisterInt("bot_enemies", 6, 1, 11, 0, "Number of enemies allowed in basic training");
        bot_difficulty = _Dvar_RegisterEnum(
                                             "bot_difficulty",
                                             bot_difficulties,
                                             1,
                                             1u,
                                             "Difficulty level of the basic training bots");
        bot_tips = _Dvar_RegisterBool("bot_tips", 1, 1u, "Combat tips enabled in basic training");
        party_simulateLongQoS = _Dvar_RegisterBool(
                                                            "party_simulateLongQoS",
                                                            0,
                                                            0,
                                                            "simulate a real QoS which takes around 30 seconds");
        invite_waitPeriod = _Dvar_RegisterInt(
                                                    "invite_waitPeriod",
                                                    30000,
                                                    15000,
                                                    0x7FFFFFFF,
                                                    0,
                                                    "time in msec you have to wait between sending invites to the same friend");
        xblive_clanListChanged = _Dvar_RegisterBool("xblive_clanListChanged", 0, 0, "Clan list gets updated");
        teamsplitter_verbose = _Dvar_RegisterBool(
                                                         "teamsplitter_verbose",
                                                         0,
                                                         0,
                                                         "Verbose debug output while splitting teams if true.");
        xblive_matchEndingSoon = _Dvar_RegisterBool("xblive_matchEndingSoon", 0, 0, "True if the match is ending soon");
        clancard_clanid = _Dvar_RegisterString(
                                                "clancard_clanid",
                                                "0",
                                                0,
//                                                (const char *)&stru_D50258.Al);
                                                "");
        ui_isClanMember = _Dvar_RegisterBool(
                                                "ui_isClanMember",
                                                0,
                                                0,
                                                //(const char *)stru_D50258.MCU_membership);
                                                "");
        getdlcmapsfrommaindrive = _Dvar_RegisterBool(
                                                                "getdlcmapsfrommaindrive",
                                                                0,
                                                                0,
                                                                "false = load the dlc maps from content packages, true = load the dlc maps from the local"
                                                                " machine harddrive");
        clanName = _Dvar_RegisterString("clanName", (char *)"", 0, "Your clan abbreviation");
        g_presenceSecKeyAndIDRegistered = 0;
        s_updatePerformanceValues = 0;
        s_performanceValueTimer = 0;
    }
}

bool __cdecl Live_HandleDWChallengeResponse(
                unsigned __int64 senderID,
                unsigned __int8 *message,
                unsigned int messageSize)
{
#ifdef KISAK_LIVE
    unsigned int nonce; // [esp+4h] [ebp-44h] BYREF
    msg_t in_msg; // [esp+8h] [ebp-40h] BYREF
    unsigned int serverchallenge; // [esp+38h] [ebp-10h] BYREF
    bool retval; // [esp+3Fh] [ebp-9h]
    unsigned int clientchallenge; // [esp+40h] [ebp-8h] BYREF
    unsigned __int8 messagetype; // [esp+47h] [ebp-1h]

    retval = 0;
    clientchallenge = 0;
    serverchallenge = 0;
    nonce = 0;
    MSG_InitReadOnly(&in_msg, message, messageSize);
    MSG_BeginReading(&in_msg);
    messagetype = MSG_ReadByte(&in_msg);
    if ( messagetype == 1 )
    {
        MSG_ReadData(&in_msg, (unsigned __int8 *)&nonce, 4);
        MSG_ReadData(&in_msg, (unsigned __int8 *)&clientchallenge, 4);
        MSG_ReadData(&in_msg, (unsigned __int8 *)&serverchallenge, 4);
        Com_DPrintf(
            14,
            "CHALLENGERESPONSE: Read out nonce %u, clientchallenge %u, serverchallenge %u\n",
            nonce,
            clientchallenge,
            serverchallenge);
        CL_HandleSVDWChallenge(nonce, serverchallenge, clientchallenge);
        return 1;
    }
    return retval;
#else
    return 1;
#endif
}

int __cdecl Live_GetControllerFromXUID(unsigned __int64 player)
{
    int index; // [esp+4h] [ebp-4h]

    for ( index = 0; index < 1; ++index )
    {
        if (xenonUserData[index].signinState
            && __PAIR64__(HIDWORD(xenonUserData[index].xuid), xenonUserData[index].xuid) == player)
            return index;
    }
    return -1;
}

void __cdecl Live_GiveAchievement(int localControllerIndex, const char *achievementName)
{
    if ( !fs_gameDirVar || !*(_BYTE *)fs_gameDirVar->current.integer )
        LiveSteam_GiveAchievement(achievementName);
}

bool __cdecl Live_IsInLiveGame()
{
    return g_serverSession.sessionHandle && onlinegame->current.enabled;
}

void __cdecl Session_CleanUpStatsWrites()
{
    int tasknum; // [esp+4h] [ebp-4h]

    R_BeginRemoteScreenUpdate();
    for ( tasknum = 0; tasknum < 32; ++tasknum )
    {
        if ( overlappedTasks_3[tasknum].active && overlappedTasks_3[tasknum].type == 2 )
        {
            while ( Live_SetPlayerTeamRankComplete(tasknum) == TASK_NOTCOMPLETE )
                NET_Sleep(1u);
        }
    }
    R_EndRemoteScreenUpdate(0);
}

taskCompleteResults __cdecl Live_SetPlayerTeamRankComplete(int slot)
{
    return Live_SetPlayerTeamRanksComplete(slot);
}

taskCompleteResults __cdecl Live_SetPlayerTeamRanksComplete(int slot)
{
#ifdef KISAK_LIVE
    overlappedTask *statsWriteOverlapped; // [esp+0h] [ebp-8h]
    taskCompleteResults res; // [esp+4h] [ebp-4h]

    statsWriteOverlapped = &overlappedTasks_3[slot];
    if ( !overlappedTasks_3[slot].active
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_win.cpp",
                    1811,
                    0,
                    "%s",
                    "overlappedTasks[slot].active") )
    {
        __debugbreak();
    }
    if ( overlappedTasks_3[slot].type != 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_win.cpp",
                    1812,
                    0,
                    "%s",
                    "overlappedTasks[slot].type == TASK_WRITESTATS") )
    {
        __debugbreak();
    }
    res = dwWritePerformanceStatsComplete(statsWriteOverlapped->controllerIndex, statsWriteOverlapped);
    if ( res == TASK_COMPLETE )
        Com_Printf(16, "Performance stats write complete\n");
    if ( res )
        TaskManager_ClearTask(statsWriteOverlapped);
    if ( res == TASK_ERROR )
        Com_PrintError(16, "Error writing performance stats.\n");
    return res;
#else
    return TASK_COMPLETE;
#endif
}

unsigned __int64 g_fakeXUID; // KISAKTODO: value?
char __cdecl XUserGetXUID(int controllerIndex, unsigned __int64 *xuid)
{
#ifdef KISAK_LIVE
    if ( dwGetLogOnStatus(0) == 4 )
        return dwGetOnlineUserID(controllerIndex, xuid);
    *xuid = g_fakeXUID << controllerIndex;
    return 1;
#else
    return 1;
#endif
}

char __cdecl Live_UserGetName(int controllerIndex, char *buf, int bufsize)
{
#ifdef KISAK_LIVE
    char *v3; // eax
    char *v4; // eax
    char ok; // [esp+3h] [ebp-1h]

    ok = dwGetOnlineUserName(controllerIndex, buf, bufsize);
    if ( !ok && dwGetLogOnStatus(0) != 4 )
    {
        if ( controllerIndex )
        {
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_win.cpp",
                            2681,
                            0,
                            "No multiple controllers on PC") )
                __debugbreak();
            v4 = UI_SafeTranslateString("EXE_DEFAULT_PLAYER");
            _snprintf(buf, bufsize, "%s %d", v4, controllerIndex + 1);
        }
        else
        {
            v3 = UI_SafeTranslateString("EXE_DEFAULT_PLAYER");
            _snprintf(buf, bufsize, v3);
        }
        return 1;
    }
    return ok;
#else
    return 1;
#endif
}

bool __cdecl Live_UserSignedInLocally(int controllerIndex, char **disconnectMessage)
{
    unsigned int v2; // eax
    unsigned int oldState; // [esp+14h] [ebp-Ch]
    bool shouldDisconnect; // [esp+1Bh] [ebp-5h]
    unsigned int signInFunctionStartTime; // [esp+1Ch] [ebp-4h]

    Com_Printf(16, "Controller #%i signed in locally\n", controllerIndex);
    oldState = xenonUserData[controllerIndex].signinState;
    signInFunctionStartTime = Sys_Milliseconds();
    Live_UserSignedIn(controllerIndex);
    v2 = Sys_Milliseconds();
    Com_Printf(16, "Live_UserSignedIn took %ims\n", v2 - signInFunctionStartTime);
    xenonUserData[controllerIndex].signinState = 1;
    Sys_Milliseconds();
    if ( oldState < 2 || oldState == 2 && !onlinegame->current.enabled || strlen(Dvar_GetString("com_errorMessage")) )
        return 0;
    shouldDisconnect = 1;
    *disconnectMessage = (char*)"XBOXLIVE_SIGNINCHANGED";
    return shouldDisconnect;
}

void Live_UserSignedIn(int controllerIndex)
{
    int v2; // eax
    DWORD v3; // eax
    unsigned __int64 newXuid; // [esp+8h] [ebp-58h] BYREF
    char xuidStr[20]; // [esp+14h] [ebp-4Ch] BYREF
    char newGamertag[32]; // [esp+28h] [ebp-38h] BYREF
    int startTime; // [esp+4Ch] [ebp-14h]
    unsigned __int64 xuidCopy; // [esp+50h] [ebp-10h] BYREF
    bool res; // [esp+5Bh] [ebp-5h]
    int netCodeVersion; // [esp+5Ch] [ebp-4h]

    res = XUserGetXUID(controllerIndex, &newXuid);
    if (!res && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_win.cpp", 2705, 0, "%s", "res"))
        __debugbreak();
    res = Live_UserGetName(controllerIndex, newGamertag, 32);
    if (!res)
        Com_Error(ERR_DROP, "XBOXLIVE_SIGNEDOUTOFLIVE");
    Dvar_SetBool((dvar_s*)xblive_loggedin, 1);
    LiveStorage_NewUser(controllerIndex);
    I_strncpyz(xenonUserData[controllerIndex].gamertag, newGamertag, 32);
    v2 = controllerIndex;
    LODWORD(xenonUserData[v2].xuid) = newXuid;
    HIDWORD(xenonUserData[v2].xuid) = HIDWORD(newXuid);
    XUIDToString(&newXuid, xenonUserData[controllerIndex].xuidString);
    xenonUserData[controllerIndex].tier = USER_TIER_NONE;
    XUIDToString(&xenonUserData[controllerIndex].xuid, xuidStr);
    StringToXUID(xuidStr, &xuidCopy);
    if (memcmp(&xuidCopy, &xenonUserData[controllerIndex].xuid, 8u)
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_win.cpp",
            2728,
            0,
            "%s",
            "memcmp( &xuidCopy, &xenonUserData[controllerIndex].xuid, sizeof(XUID) ) == 0"))
    {
        __debugbreak();
    }
    Dvar_SetString((dvar_s*)name, xenonUserData[controllerIndex].gamertag);
    if (!xenonUserData[controllerIndex].signinState)
    {
        startTime = Sys_Milliseconds();
        GamerProfile_LogInProfile(controllerIndex);
        v3 = Sys_Milliseconds();
        Com_Printf(16, "GamerProfile_LogInProfile took %ims\n", v3 - startTime);
    }
    netCodeVersion = 1044;
    LiveGroups_RegisterPlayer(controllerIndex);
}

bool Live_UserSignedInToLive(int controllerIndex, char **disconnectMessage)
{
    int LocalClientNum; // eax
    int oldState; // [esp+10h] [ebp-8h]
    bool shouldDisconnect; // [esp+17h] [ebp-1h]

    Com_Printf(16, "Controller #%i signed in to live\n", controllerIndex);
    oldState = xenonUserData[controllerIndex].signinState;
    Live_UserSignedIn(controllerIndex);
    xenonUserData[controllerIndex].signinState = 2;
    xenonUserData[controllerIndex].isGuestUser = 0;
    Live_GetOurUploadBandwidth(controllerIndex);
    LiveStorage_FetchRequiredFiles(controllerIndex);
    LiveMeetPlayer_DownloadMetPlayersList(0);
    LiveCounter_Init();
    LiveCounter_SetupCounters();
    Live_GetPrivateProfile();
    Live_ReadRecentServers();
    Live_RequestSessionsFromFriends();
    LiveStorage_SetAllStatsNotFetched(controllerIndex);
    LiveStorage_ReadStats(controllerIndex, 0, 0);
    //BG_EvalVehicleName();
    if ((s_signInRequirement[controllerIndex] & 4) != 0)
    {
        //BG_EvalVehicleName();
        if (Live_ContentRatingAllowed())
        {
            LocalClientNum = Com_ControllerIndex_GetLocalClientNum(controllerIndex);
            Cbuf_ExecuteBuffer(LocalClientNum, controllerIndex, (char*)"xstartprivateparty");
        }
        else
        {
            Com_Printf(
                16,
                "Since the active profile doesn't have permission to play MP, so we are going to send them back to the main menu\n");
            *disconnectMessage = (char*)"XBOXLIVE_MPNOTALLOWED";
        }
    }
    if (!oldState
        || oldState == 2
        || oldState == 1 && !onlinegame->current.enabled
        || strlen(Dvar_GetString("com_errorMessage")))
    {
        return 0;
    }
    shouldDisconnect = 1;
    *disconnectMessage = (char *)"XBOXLIVE_SIGNINCHANGED";
    return shouldDisconnect;
}

bool __cdecl Live_IsSignedIn(int controllerIndex)
{
    if ( LiveSteam_IsInitialized()
        && !LiveSteam_IsClientSignedInLocally()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_win.cpp",
                    2909,
                    0,
                    "%s",
                    "!LiveSteam_IsInitialized() || LiveSteam_IsClientSignedInLocally()") )
    {
        __debugbreak();
    }
    return xenonUserData[controllerIndex].signinState > 0;
}

bool __cdecl Live_IsSignedInToLive()
{
    if ( !LiveSteam_IsClientSignedInOnline() )
        return 0;
    return dw_active && dw_active->current.enabled;
}

unsigned long long __cdecl Live_GetXuid(int controllerIndex)
{
    //if ( !Live_IsSignedIn(controllerIndex)
    //    && !Assert_MyHandler(
    //                "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_win.cpp",
    //                2972,
    //                0,
    //                "%s",
    //                "Live_IsSignedIn( controllerIndex )") )
    //{
    //    __debugbreak();
    //}
    return xenonUserData[controllerIndex].xuid;
}

int __cdecl Live_GetTier(int controllerIndex)
{
    if ( !Live_IsSignedIn(controllerIndex)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_win.cpp",
                    2989,
                    0,
                    "%s",
                    "Live_IsSignedIn( controllerIndex )") )
    {
        __debugbreak();
    }
    return xenonUserData[controllerIndex].tier;
}

char __cdecl Live_ShowMarketplaceUI()
{
    LiveSteam_ShowStore();
    return 1;
}

int __cdecl Live_GetUploadSpeed()
{
    return s_uploadBitsPerSec;
}

void __cdecl Live_GetOurUploadBandwidth(int localControllerIndex)
{
#ifdef KISAK_LIVE_STUBS
    struct bdLobbyService *Lobby; // eax

    if ( s_uploadBitsPerSec <= 0 )
    {
        Lobby = dwGetLobby(localControllerIndex);
        if ( bdBandwidthTestClient::init(g_bandwidthTestClient, Lobby) )
        {
            bdBandwidthTestClient::start(g_bandwidthTestClient, BD_UPLOAD_TEST);
            if ( bdBandwidthTestClient::getErrorCode(g_bandwidthTestClient) != BD_START_TASK_FAILED )
                TaskManager_GetOpenTaskSlot(overlappedTasks_3, localControllerIndex, 3);
        }
    }
#endif
}

char __cdecl Live_BandwidthTestInProgress()
{
    return TaskManager_TaskIsInProgress(overlappedTasks_3, 3);
}

void __cdecl Live_CheckOngoingTasks()
{
    const char *v0; // eax
    int tasknum; // [esp+1Ch] [ebp-4h]

    Sys_EnterCriticalSection(CRITSECT_LIVE);
    for ( tasknum = 0; tasknum < 32; ++tasknum )
    {
        if ( overlappedTasks_3[tasknum].active )
        {
            switch ( overlappedTasks_3[tasknum].type )
            {
                case 1:
                    if ( (unsigned int)Live_QoSProbeComplete(tasknum) > TASK_COMPLETE )
                        Com_PrintError(16, "Error getting player QoS results\n");
                    break;
                case 2:
                    if ( (unsigned int)Live_SetPlayerTeamRankComplete(tasknum) > TASK_COMPLETE )
                        Com_PrintError(16, "Error writing stats\n");
                    break;
                case 3:
                    if ( (unsigned int)Live_GetBandwidthTestComplete(tasknum) >= 2 )
                        Com_PrintError(16, "Error getting player bandwidth test results\n");
                    break;
                case 5:
                    if ( (unsigned int)Live_FetchPartyPerformanceValuesComplete() > 1 )
                        Com_PrintError(16, "Error fetching party performance values\n");
                    break;
                case 6:
                    if ( (unsigned int)Live_UpdatePerformanceValuesComplete(tasknum) > TASK_COMPLETE )
                        Com_PrintError(16, "Error fetching performance values\n");
                    break;
                case 7:
                    continue;
                case 8:
                    if ( (unsigned int)CL_LocalClient_GetActiveCount() > 1 )
                        Com_PrintError(16, "Error inviting friend\n");
                    break;
                default:
                    v0 = va("Unknown live task type %i\n", overlappedTasks_3[tasknum].type);
                    if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_win.cpp", 3533, 0, v0) )
                        __debugbreak();
                    break;
            }
        }
    }
    Sys_LeaveCriticalSection(CRITSECT_LIVE);
}

int __cdecl Live_GetBandwidthTestComplete(int slot)
{
#ifdef KISAK_LIVE_STUBS
    bdBandwidthTestResults *UploadResults; // eax
    bdBandwidthTestResults *v2; // eax
    double Bandwidth; // st7
    bdBandwidthTestResults *DownloadResults; // eax
    double v5; // st7
    enum bdLobbyErrorCode ErrorCode; // eax
    overlappedTask *bandwidthTestOverlappedIO; // [esp+10h] [ebp-4h]

    bandwidthTestOverlappedIO = &overlappedTasks_3[slot];
    if ( !bandwidthTestOverlappedIO->active
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_win.cpp",
                    3114,
                    0,
                    "%s",
                    "bandwidthTestOverlappedIO->active") )
    {
        __debugbreak();
    }
    if ( bandwidthTestOverlappedIO->type != 3
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_win.cpp",
                    3115,
                    0,
                    "%s",
                    "bandwidthTestOverlappedIO->type == TASK_BANDWIDTHTEST") )
    {
        __debugbreak();
    }
    bdBandwidthTestClient::pump(g_bandwidthTestClient);
    if ( bdBandwidthTestClient::getErrorCode(g_bandwidthTestClient) )
    {
        ErrorCode = bdBandwidthTestClient::getErrorCode(g_bandwidthTestClient);
        Com_PrintError(16, "Error '%i' trying to measure upload bandwidth\n", ErrorCode);
        bdBandwidthTestClient::stop(g_bandwidthTestClient);
        bdBandwidthTestClient::quit(g_bandwidthTestClient);
        s_uploadBitsPerSec = -1;
        TaskManager_ClearTask(bandwidthTestOverlappedIO);
        return 1;
    }
    else if ( bdBandwidthTestClient::getStatus(g_bandwidthTestClient) == BD_BANDWIDTH_TEST )
    {
        UploadResults = bdBandwidthTestClient::getUploadResults(g_bandwidthTestClient);
        s_uploadBitsPerSec = (int)(bdBandwidthTestResults::getBandwidth(UploadResults) * 1000.0);
        v2 = bdBandwidthTestClient::getUploadResults(g_bandwidthTestClient);
        Bandwidth = bdBandwidthTestResults::getBandwidth(v2);
        Com_Printf(16, "Upload Bandwidth: %.2f Kbits/s\n", Bandwidth);
        DownloadResults = bdBandwidthTestClient::getDownloadResults(g_bandwidthTestClient);
        v5 = bdBandwidthTestResults::getBandwidth(DownloadResults);
        Com_Printf(16, "Download Bandwidth: %.2f Kbits/s\n", v5);
        bdBandwidthTestClient::stop(g_bandwidthTestClient);
        bdBandwidthTestClient::quit(g_bandwidthTestClient);
        TaskManager_ClearTask(bandwidthTestOverlappedIO);
        return 1;
    }
    else
    {
        return 0;
    }
#else
    return 0;
#endif
}

taskCompleteResults __cdecl Live_QoSProbeComplete(int slot)
{
#ifdef KISAK_LIVE_STUBS
    overlappedTask *qosOverlappedIO; // [esp+0h] [ebp-Ch]
    dwQoSMultiProbeListener *listener; // [esp+4h] [ebp-8h]
    taskCompleteResults res; // [esp+8h] [ebp-4h]

    qosOverlappedIO = &overlappedTasks_3[slot];
    if ( !overlappedTasks_3[slot].active
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_win.cpp",
                    3232,
                    0,
                    "%s",
                    "overlappedTasks[slot].active") )
    {
        __debugbreak();
    }
    if ( overlappedTasks_3[slot].type != 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_win.cpp",
                    3233,
                    0,
                    "%s",
                    "overlappedTasks[slot].type == TASK_QOSPROBE") )
    {
        __debugbreak();
    }
    listener = (dwQoSMultiProbeListener *)TaskManager_GetTaskData(qosOverlappedIO);
    res = dwQoSMultiProbeListener::complete(listener);
    if ( Live_QoSProbeEarlyComplete(listener) )
        res = TASK_COMPLETE;
    if ( res == TASK_COMPLETE )
    {
        if ( party_simulateLongQoS->current.enabled )
        {
            R_BeginRemoteScreenUpdate();
            NET_Sleep(0x7530u);
            R_EndRemoteScreenUpdate(0);
        }
        dwQoSMultiProbeListener::clean(listener);
        TaskManager_ClearTask(qosOverlappedIO);
        if ( Live_BandwidthTestInProgress() )
            Com_Printf(16, "QoS test complete - Not doing anything now because we're still getting our bandwidth numbers\n");
        else
            Com_Printf(16, "QoS test complete - Not doing anything now because lobby isn't active\n");
    }
    else if ( res )
    {
        dwQoSMultiProbeListener::clean(listener);
        TaskManager_ClearTask(qosOverlappedIO);
    }
    return res;
#else
    return TASK_NOTCOMPLETE;
#endif
}

bool __cdecl Live_QoSProbeEarlyComplete(dwQoSMultiProbeListener *listener)
{
#ifdef KISAK_LIVE_STUBS
    unsigned inttime; // [esp+0h] [ebp-4h]

    if ( !listener->m_numSuccesses )
        return 0;
    g_msgTime = Sys_Milliseconds();
    if ( g_qosStatus.prevNumSuccesses != listener->m_numSuccesses )
    {
        if ( !g_qosStatus.firstMS )
            g_qosStatus.firstMS = g_msgTime;
        g_qosStatus.updateMS = g_msgTime;
        g_qosStatus.prevNumSuccesses = listener->m_numSuccesses;
    }
    if ( !listener->m_numSuccesses )
        return 0;
    if ( listener->m_numProbes < listener->m_numSuccesses
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_win.cpp",
                    3214,
                    0,
                    "%s",
                    "listener->m_numProbes >= listener->m_numSuccesses") )
    {
        __debugbreak();
    }
    if ( 100 * listener->m_numSuccesses < 25 * listener->m_numProbes )
        return 0;
    if ( (int)(g_msgTime - g_qosStatus.firstMS) >= 1000 )
        return (int)(g_msgTime - g_qosStatus.updateMS) >= 250;
    return 0;
#else
    return 0;
#endif
}

void __cdecl PC_InitSigninState()
{
#ifdef KISAK_LIVE_STUBS
    int LicenseType; // eax
    MatchMakingInfo *matched; // [esp+8h] [ebp-24h]
    bdLogSubscriber *v2; // [esp+Ch] [ebp-20h]
    MatchMakingInfo *v3; // [esp+14h] [ebp-18h]
    bdLogSubscriber *v4; // [esp+18h] [ebp-14h]

    if ( !live_service
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_win.cpp", 3551, 0, "%s", "live_service") )
    {
        __debugbreak();
    }
    bdCore::init(1);
    v4 = (bdLogSubscriber *)operator new(0x24u);
    if ( v4 )
        v2 = bdLogSubscriber::bdLogSubscriber(v4);
    else
        v2 = 0;
    g_logSubscriber = v2;
    bdLogSubscriber::addChannel(v2, "warn");
    bdLogSubscriber::addChannel(g_logSubscriber, "err");
    v3 = (MatchMakingInfo *)bdMemory::allocate(0x228u);
    if ( v3 )
        matched = MatchMakingInfo::MatchMakingInfo(v3);
    else
        matched = 0;
    g_matchmakingInfo = matched;
    dwNetStart(1);
    DWDedicatedLogon();
    LicenseType = SV_GetLicenseType();
    Dvar_SetIntByName("sv_ranked", LicenseType);
#endif
}

bool __cdecl Live_UserSignedOut(int controllerIndex)
{
#ifdef KISAK_LIVE_STUBS
    int v1; // eax
    int localClientNum; // [esp+0h] [ebp-8h]

    GamerProfile_LogOutProfile(controllerIndex);
    byte_A61C02C[80 * controllerIndex] = 0;
    xenonUserData[controllerIndex].signinState = 0;
    byte_A61C058[80 * controllerIndex] = 0;
    v1 = 20 * controllerIndex;
    dword_A61C050[v1] = 0;
    dword_A61C054[v1] = 0;
    Dvar_SetString((dvar_s *)name, &byte_A61C02C[80 * controllerIndex]);
    byte_A61C069[80 * controllerIndex] = 0;
    dword_A61C06C[20 * controllerIndex] = 0;
    localClientNum = Com_ControllerIndex_GetLocalClientNum(controllerIndex);
    LiveStorage_NewUser(controllerIndex);
    LiveStorage_FileShare_GetFileShareData(FILESHARE_BUFFER_PRIMARY)->valid = 0;
    LiveStorage_FileShare_GetFileShareData(FILESHARE_BUFFER_SECONDARY)->valid = 0;
    if ( Com_LocalClient_IsBeingUsed(localClientNum) )
    {
        Flame_GetLocalClientSourceRange();
        CL_GetLocalClientConnectionState(localClientNum);
        //BLOPS_NULLSUB();
        Com_LocalClient_SetBeingUsed(localClientNum, 0);
        if ( (s_signInRequirement[controllerIndex] & 7) == 0 )
            Com_Printf(16, "Controller #%i signed out\n", controllerIndex);
    }
    return 0;
#else
    return 0;
#endif
}

void __cdecl Live_DelayedComError(const char *comErrorString)
{
    g_shouldComError = 1;
    g_comErrorString = comErrorString;
}

void __cdecl Live_Frame()
{
    ;
}

int __cdecl Live_FetchPartyPerformanceValuesComplete()
{
    if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_win.cpp", 4924, 0, "PC MP not using Party") )
        __debugbreak();
    return 2;
}

taskCompleteResults __cdecl Live_UpdatePerformanceValuesComplete(int slot)
{
#ifdef KISAK_LIVE
    unsigned __int64 v2; // [esp-8h] [ebp-224h]
    int RegisteredUser; // [esp+0h] [ebp-21Ch]
    int i; // [esp+4h] [ebp-218h]
    taskCompleteResults result; // [esp+Ch] [ebp-210h]
    overlappedTask *updatePerformanceOverlappedIO; // [esp+10h] [ebp-20Ch]
    PlayerRank playerRanks[32]; // [esp+14h] [ebp-208h] BYREF
    int numPlayerRanks; // [esp+218h] [ebp-4h] BYREF

    updatePerformanceOverlappedIO = &overlappedTasks_3[slot];
    if ( !overlappedTasks_3[slot].active
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_win.cpp",
                    5080,
                    0,
                    "%s",
                    "overlappedTasks[slot].active") )
    {
        __debugbreak();
    }
    if ( overlappedTasks_3[slot].type != 6
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_win.cpp",
                    5081,
                    0,
                    "%s",
                    "overlappedTasks[slot].type == TASK_UPDATEPERFORMANCE") )
    {
        __debugbreak();
    }
    numPlayerRanks = 32;
    result = dwFetchPerformanceValuesComplete(updatePerformanceOverlappedIO, playerRanks, &numPlayerRanks);
    if ( result == TASK_COMPLETE )
    {
        for ( i = 0; i < numPlayerRanks; ++i )
        {
            HIDWORD(v2) = HIDWORD(playerRanks[i].xuid);
            LODWORD(v2) = playerRanks[i].xuid;
            RegisteredUser = Session_FindRegisteredUser(&g_serverSession, v2);
            if ( RegisteredUser >= 0 )
                g_serverSession.registeredUsers[RegisteredUser].performanceValue = playerRanks[i].rank;
        }
    }
    if ( result )
    {
        TaskManager_ClearTask(updatePerformanceOverlappedIO);
        if ( !TaskManager_TaskIsInProgress(overlappedTasks_3, 6) )
            Live_UpdateAveragePerformance();
    }
    if ( result == TASK_ERROR )
        Com_PrintError(16, "Unable to retreive performance value\n");
    return result;
#else
    return TASK_COMPLETE;
#endif
}

void Live_UpdateAveragePerformance()
{
#ifdef KISAK_LIVE_SERVICE
    int avgPerformance; // [esp+0h] [ebp-4h]

    if (g_matchmakingInfo->m_active)
    {
        avgPerformance = Live_GetAveragePerformance();
        if (g_matchmakingInfo->m_memberSKILL != avgPerformance)
        {
            Com_Printf(16, "Setting new session performance value %i\n", avgPerformance);
            g_matchmakingInfo->m_dirty = 1;
            g_matchmakingInfo->m_memberSKILL = avgPerformance;
        }
    }
#endif
}

int __cdecl Live_GetAveragePerformance()
{
    int avgPerformance; // [esp+0h] [ebp-20h]
    int sessionSlot; // [esp+4h] [ebp-1Ch]
    int numMembers; // [esp+8h] [ebp-18h]
    int totalPerformance; // [esp+Ch] [ebp-14h]
    int i; // [esp+10h] [ebp-10h]
    unsigned __int64 xuid; // [esp+18h] [ebp-8h]

    totalPerformance = 0;
    avgPerformance = 0;
    numMembers = 0;
    for ( i = 0; i < com_maxclients->current.integer; ++i )
    {
        xuid = 0;
        if ( svs.clients && svs.clients[i].header.state >= CS_CONNECTED )
        {
            LODWORD(xuid) = svs.xuids[i];
            HIDWORD(xuid) = LODWORD(svs.mapCenter[2 * i - 63]);
        }
        if ( xuid )
        {
            sessionSlot = Session_FindRegisteredUser(&g_serverSession, xuid);
            if ( sessionSlot >= 0 )
            {
                totalPerformance += g_serverSession.registeredUsers[sessionSlot].performanceValue;
                ++numMembers;
            }
        }
    }
    if ( numMembers > 0 )
        return totalPerformance / numMembers;
    return avgPerformance;
}

char __cdecl Live_IsUserBlocked(int controllerIndex, unsigned __int64 xuid)
{
    unsigned int idx; // [esp+4h] [ebp-4h]

    for ( idx = 0; idx < s_blockedListCount; ++idx )
    {
        if ( __PAIR64__(HIDWORD(s_blockedList[idx]), s_blockedList[idx]) == xuid )
            return 1;
    }
    return 0;
}

bool __cdecl Live_CanConsoleViewContentFromUser(unsigned __int64 xuid)
{
    int i; // [esp+0h] [ebp-8h]
    bool canView; // [esp+7h] [ebp-1h]

    canView = 1;
    for ( i = 0; i < 1; ++i )
    {
        if ( Live_IsUserSignedInToLive() && !Live_CanViewContentFromUser(i, xuid) )
            canView = 0;
    }
    return canView;
}

bool __cdecl Live_CanViewContentFromUser(int controllerIndex, unsigned __int64 xuid)
{
    return Live_IsUserBlocked(controllerIndex, xuid) == 0;
}

void __cdecl Live_SendInvite(const char *friendName)
{
#ifdef KISAK_LIVE_STUBS
    const struct bdSessionID *SessionID; // eax
    int LocalClientNum; // eax
    const struct bdSessionID *v3; // eax
    int v4; // eax
    const struct bdSessionID *v5; // eax
    int v6; // eax
    bdSessionID v7[2]; // [esp-Ch] [ebp-20h] BYREF
    unsigned __int64 friendID; // [esp+Ch] [ebp-8h] BYREF

    friendID = 0;
    if ( Friend_GetByName(0, friendName, &friendID) )
    {
        SessionID = dwGetSessionID();
        bdSessionID::bdSessionID(v7, SessionID);
        dwMatchMaking_InviteDWUserToSession(friendID, v7[0]);
        *(unsigned int *)&v7[0].m_sessionID.ab[4] = 2700;
        *(unsigned int *)v7[0].m_sessionID.ab = UI_SafeTranslateString("MENU_SENT_CAPS");
        v7[0].__vftable = (bdSessionID_vtbl *)"";
        LocalClientNum = Com_ControllerIndex_GetLocalClientNum(0);
        UI_OpenToastPopup(
            LocalClientNum,
            "menu_mp_killstreak_select",
            (const char *)v7[0].__vftable,
            *(const char **)v7[0].m_sessionID.ab,
            *(int *)&v7[0].m_sessionID.ab[4]);
    }
    else if ( s_selectedPlayerXUID )
    {
        v3 = dwGetSessionID();
        bdSessionID::bdSessionID(v7, v3);
        dwMatchMaking_InviteDWUserToSession(s_selectedPlayerXUID, v7[0]);
        *(unsigned int *)&v7[0].m_sessionID.ab[4] = 2700;
        *(unsigned int *)v7[0].m_sessionID.ab = UI_SafeTranslateString("MENU_SENT_CAPS");
        v7[0].__vftable = (bdSessionID_vtbl *)"";
        v4 = Com_ControllerIndex_GetLocalClientNum(0);
        UI_OpenToastPopup(
            v4,
            "menu_mp_killstreak_select",
            (const char *)v7[0].__vftable,
            *(const char **)v7[0].m_sessionID.ab,
            *(int *)&v7[0].m_sessionID.ab[4]);
    }
    else if ( __PAIR64__(s_selectedMetPlayerXUID, 0) == HIDWORD(s_selectedMetPlayerXUID) )
    {
        Com_PrintWarning(0, "Couldn't find friend %s\n", friendName);
    }
    else
    {
        v5 = dwGetSessionID();
        bdSessionID::bdSessionID(v7, v5);
        dwMatchMaking_InviteDWUserToSession(s_selectedMetPlayerXUID, v7[0]);
        *(unsigned int *)&v7[0].m_sessionID.ab[4] = 2700;
        *(unsigned int *)v7[0].m_sessionID.ab = UI_SafeTranslateString("MENU_SENT_CAPS");
        v7[0].__vftable = (bdSessionID_vtbl *)"";
        v6 = Com_ControllerIndex_GetLocalClientNum(0);
        UI_OpenToastPopup(
            v6,
            "menu_mp_killstreak_select",
            (const char *)v7[0].__vftable,
            *(const char **)v7[0].m_sessionID.ab,
            *(int *)&v7[0].m_sessionID.ab[4]);
    }
#endif
}

void __cdecl Live_DumpFavourites()
{
    unsigned int i; // [esp+4h] [ebp-4h]

    for ( i = 0; i < 0x2A; ++i )
    {
        if ( LODWORD(s_favourites[i].uid) || HIDWORD(s_favourites[i].uid) )
            Com_DPrintf(0, "%llu\n", *(_QWORD *)s_favourites[i].addressblob);
    }
}

void __cdecl Live_FindFavouriteServersSuccess(TaskRecord *task)
{
#ifdef KISAK_LIVE_STUBS
    int HeaderSize; // eax
    bdReference<bdRemoteTask> dwtask; // [esp+24h] [ebp-4h] BYREF

    dwtask.m_ptr = task->nestedTask->remoteTask.m_ptr;
    if ( dwtask.m_ptr )
    {
        InterlockedIncrement(&dwtask.m_ptr->m_refCount);
    }
    else if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_win.cpp", 5560, 0, "%s", "NULL != dwtask") )
    {
        __debugbreak();
    }
    if ( bdTaskByteBuffer::getHeaderSize((bdTaskByteBuffer *)dwtask.m_ptr) )
    {
        HeaderSize = bdTaskByteBuffer::getHeaderSize((bdTaskByteBuffer *)dwtask.m_ptr);
        CL_ServersResponsePacket(s_favouritesInfo, HeaderSize, 1);
    }
    bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>((bdReference<bdCommonAddr> *)&dwtask);
#endif
}

bool __cdecl Live_FindFavouritesInProgress()
{
#ifdef KISAK_LIVE_STUBS
    return TaskManager2_TaskIsInProgress(task_LiveFavouriteServers);
#else
    return false;
#endif
}

void __cdecl Live_FindFavouriteServers()
{
#ifdef KISAK_LIVE_STUBS
    TaskRecord *task; // [esp+4h] [ebp-160h]
    TaskRecord *nestedTask; // [esp+8h] [ebp-15Ch]
    unsigned int i; // [esp+Ch] [ebp-158h]
    signed int j; // [esp+10h] [ebp-154h]
    unsigned __int64 entityIDs[42]; // [esp+14h] [ebp-150h] BYREF

    memset(entityIDs, 0, sizeof(entityIDs));
    j = 0;
    for ( i = 0; i < 0x2A; ++i )
    {
        if ( LODWORD(s_favourites[i].uid) || HIDWORD(s_favourites[i].uid) )
        {
            LODWORD(entityIDs[j]) = s_favourites[i].uid;
            HIDWORD(entityIDs[j++]) = HIDWORD(s_favourites[i].uid);
        }
    }
    if ( j <= 0 )
    {
        Com_DPrintf(23, "No favourites to get details for!\n");
    }
    else
    {
        nestedTask = dwFindSessionsByEntityIDs(entityIDs, j, s_favouritesInfo);
        if ( nestedTask )
        {
            task = TaskManager2_CreateTask(task_LiveFavouriteServers, 0, nestedTask, 0);
            if ( task )
                TaskManager2_StartTask(task);
            else
                Com_DPrintf(0, "Couldn't start find favourites task, connected ok?\n");
        }
    }
#endif
}

void __cdecl Live_FindFriendServersSuccess(TaskRecord *task)
{
#ifdef KISAK_LIVE_STUBS
    int HeaderSize; // eax

    if ( !task->nestedTask->remoteTask.m_ptr
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_win.cpp",
                    5615,
                    0,
                    "%s",
                    "task->nestedTask->remoteTask") )
    {
        __debugbreak();
    }
    if ( bdTaskByteBuffer::getHeaderSize((bdTaskByteBuffer *)task->nestedTask->remoteTask.m_ptr) )
    {
        HeaderSize = bdTaskByteBuffer::getHeaderSize((bdTaskByteBuffer *)task->nestedTask->remoteTask.m_ptr);
        CL_ServersResponsePacket(s_friendServersInfo, HeaderSize, 1);
    }
#endif
}

void __cdecl Live_FindFriendServers()
{
#ifdef KISAK_LIVE_STUBS
    TaskRecord *task; // [esp+Ch] [ebp-814h]
    TaskRecord *nestedTask; // [esp+10h] [ebp-810h]
    int k; // [esp+14h] [ebp-80Ch]
    int i; // [esp+18h] [ebp-808h]
    signed int j; // [esp+1Ch] [ebp-804h]
    unsigned __int64 entityIDs[256]; // [esp+20h] [ebp-800h] BYREF

    memset(entityIDs, 0, sizeof(entityIDs));
    j = 0;
    for ( i = 0; i < 256; ++i )
    {
        if ( LODWORD(s_friendsServers[i].serverID) || HIDWORD(s_friendsServers[i].serverID) )
        {
            for ( k = j;
                        k >= 0
                 && (LODWORD(entityIDs[k]) != LODWORD(s_friendsServers[i].serverID)
                    || HIDWORD(entityIDs[k]) != HIDWORD(s_friendsServers[i].serverID));
                        --k )
            {
                ;
            }
            if ( k == -1 )
            {
                LODWORD(entityIDs[j]) = s_friendsServers[i].serverID;
                HIDWORD(entityIDs[j++]) = HIDWORD(s_friendsServers[i].serverID);
            }
        }
    }
    if ( j <= 0 )
    {
        Com_DPrintf(23, "No friends to get details for!\n");
    }
    else
    {
        nestedTask = dwFindSessionsByEntityIDs(entityIDs, j, s_friendServersInfo);
        if ( nestedTask )
        {
            task = TaskManager2_CreateTask(task_LiveFriendServers, 0, nestedTask, 0);
            if ( task )
                TaskManager2_StartTask(task);
        }
    }
#endif
}

void __cdecl Live_FindRecentServersSuccess(TaskRecord *task)
{
#ifdef KISAK_LIVE_STUBS
    int HeaderSize; // eax

    if ( !task->nestedTask->remoteTask.m_ptr
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_win.cpp",
                    5675,
                    0,
                    "%s",
                    "task->nestedTask->remoteTask") )
    {
        __debugbreak();
    }
    if ( bdTaskByteBuffer::getHeaderSize((bdTaskByteBuffer *)task->nestedTask->remoteTask.m_ptr) )
    {
        HeaderSize = bdTaskByteBuffer::getHeaderSize((bdTaskByteBuffer *)task->nestedTask->remoteTask.m_ptr);
        CL_ServersResponsePacket(s_recentServersInfo, HeaderSize, 1);
    }
#endif
}

void __cdecl Live_FindRecentServers()
{
#ifdef KISAK_LIVE_STUBS
    TaskRecord *task; // [esp+0h] [ebp-100h]
    TaskRecord *nestedTask; // [esp+4h] [ebp-FCh]
    int i; // [esp+8h] [ebp-F8h]
    signed int j; // [esp+Ch] [ebp-F4h]
    unsigned __int64 entityIDs[30]; // [esp+10h] [ebp-F0h] BYREF

    memset(entityIDs, 0, sizeof(entityIDs));
    j = 0;
    for ( i = 0; i < 30 && s_recentServers[i].joinTime; ++i )
    {
        LODWORD(entityIDs[j]) = s_recentServers[i].serverID;
        HIDWORD(entityIDs[j++]) = HIDWORD(s_recentServers[i].serverID);
    }
    if ( j <= 0 )
    {
        Com_DPrintf(23, "No friends to get details for!\n");
    }
    else
    {
        nestedTask = dwFindSessionsByEntityIDs(entityIDs, j, s_recentServersInfo);
        if ( nestedTask )
        {
            task = TaskManager2_CreateTask(task_LiveRecentServers, 0, nestedTask, 0);
            if ( task )
                TaskManager2_StartTask(task);
        }
    }
#endif
}

void __cdecl Live_SaveRecentServers()
{
    unsigned __int8 *ptr; // [esp+8h] [ebp-8h]
    unsigned __int8 *ptra; // [esp+8h] [ebp-8h]
    int i; // [esp+Ch] [ebp-4h]

    ptr = s_recentServersBuf;
    for ( i = 0; i < 30 && (LODWORD(s_recentServers[i].serverID) || HIDWORD(s_recentServers[i].serverID)); ++i )
    {
        *(unsigned int *)ptr = s_recentServers[i].serverID;
        *((unsigned int *)ptr + 1) = HIDWORD(s_recentServers[i].serverID);
        ptra = ptr + 8;
        *(unsigned int *)ptra = s_recentServers[i].joinTime;
        ptr = ptra + 4;
    }
    Com_DPrintf(23, "Serialized %i (%i bytes) recent servers\n", i, ptr - s_recentServersBuf);
    LiveStorage_SaveRecentServers(s_recentServersBuf, ptr - s_recentServersBuf);
}

void __cdecl Live_ReadRecentServers()
{
    LiveStorage_ReadRecentServers(s_recentServersBuf, 480);
}

void __cdecl Live_PopulateRecentServers(unsigned __int8 *buf, int bufsize)
{
    int v2; // ecx
    int i; // [esp+0h] [ebp-Ch]
    unsigned __int8 *ptr; // [esp+8h] [ebp-4h]

    ptr = buf;
    for ( i = 0; i < 30 && ptr != &buf[bufsize]; ++i )
    {
        v2 = i;
        LODWORD(s_recentServers[v2].serverID) = *(unsigned int *)ptr;
        *(unsigned int *)(v2 * 16 + 174315068) = *((unsigned int *)ptr + 1);
        s_recentServers[i].joinTime = *((unsigned int *)ptr + 2);
        ptr += 12;
    }
}

void __cdecl Live_AddRecentServer(unsigned __int64 serveruid)
{
    recentServer_t *serverent; // [esp+4h] [ebp-Ch]
    int currentTime; // [esp+8h] [ebp-8h]
    int i; // [esp+Ch] [ebp-4h]

    currentTime = LiveStorage_GetUTC();
    for ( i = 0; i < 30; ++i )
    {
        serverent = &s_recentServers[i];
        if ( serveruid == serverent->serverID )
        {
            serverent->joinTime = currentTime;
            break;
        }
        if ( !serverent->joinTime )
        {
            serverent->serverID = serveruid;
            serverent->joinTime = currentTime;
            break;
        }
    }
    if ( i == 30 )
    {
        s_recentServers[29].joinTime = currentTime;
        s_recentServers[29].serverID = serveruid;
    }
    qsort(s_recentServers, 0x1Eu, 0x10u, (int (__cdecl *)(const void *, const void *))compareRecentServers);
    Live_SaveRecentServers();
}

int __cdecl compareRecentServers(unsigned int *server1, unsigned int *server2)
{
    return server2[2] - server1[2];
}

void __cdecl Live_GetFriendsOnServer(unsigned __int64 serverId, unsigned __int64 *friendIDs, int *numfriends)
{
#ifdef KISAK_LIVE_STUBS
    int v3; // ecx
    int i; // [esp+Ch] [ebp-8h]
    int numsteamfriends; // [esp+10h] [ebp-4h]

    numsteamfriends = LiveSteam_GetNumFriends();
    *numfriends = 0;
    for ( i = 0; i < numsteamfriends; ++i )
    {
        if ( (LODWORD(s_friendsServers[i].friendID) || HIDWORD(s_friendsServers[i].friendID))
            && serverId == __PAIR64__(HIDWORD(s_friendsServers[i].serverID), s_friendsServers[i].serverID) )
        {
            v3 = *numfriends;
            LODWORD(friendIDs[v3]) = s_friendsServers[i].friendID;
            HIDWORD(friendIDs[v3]) = HIDWORD(s_friendsServers[i].friendID);
            ++*numfriends;
        }
    }
#endif
}

char __cdecl Live_AddFavourite_Ingame(unsigned __int64 serverid, unsigned __int64 serveruid)
{
#ifdef KISAK_LIVE_STUBS
    char *v2; // eax
    favourite_t *v4; // eax
    unsigned int v5; // eax
    char *v6; // eax
    const char *v7; // eax
    char *v8; // eax
    unsigned int i; // [esp+8h] [ebp-4h]

    for ( i = 0; i < 0x2A && (LODWORD(s_favourites[i].uid) || HIDWORD(s_favourites[i].uid)); ++i )
    {
        if ( serveruid == __PAIR64__(HIDWORD(s_favourites[i].uid), s_favourites[i].uid) )
        {
            v2 = UI_SafeTranslateString("EXE_FAVORITEINLIST");
            Com_Printf(13, "%s\n", v2);
            Dvar_SetStringByName("ui_favorite_message", "@EXE_FAVORITEINLIST");
            return 0;
        }
    }
    if ( i >= 0x2A )
    {
        v8 = UI_SafeTranslateString("EXE_FAVORITELISTFULL");
        Com_Printf(13, "%s\n", v8);
        Dvar_SetStringByName("ui_favorite_message", "@EXE_FAVORITELISTFULL");
        return 0;
    }
    else
    {
        v4 = &s_favourites[i];
        *(unsigned int *)v4->addressblob = serverid;
        *(_WORD *)&v4->addressblob[4] = WORD2(serverid);
        v5 = i;
        LODWORD(s_favourites[v5].uid) = serveruid;
        *(unsigned int *)(v5 * 16 + 174313908) = HIDWORD(serveruid);
        CL_SetFavourites_f();
        v6 = UI_SafeTranslateString("EXE_FAVORITEADDED");
        v7 = va("%s\n", v6);
        Com_Printf(13, v7);
        Dvar_SetStringByName("ui_favorite_message", "@EXE_FAVORITEADDED");
        return 1;
    }
#else
    return 0;
#endif
}

void __cdecl Live_AddFavourite(unsigned __int64 serverid, unsigned __int64 serveruid)
{
    favourite_t *v2; // ecx
    unsigned int v3; // ecx
    unsigned int i; // [esp+4h] [ebp-4h]

    for ( i = 0; i < 0x2A && (LODWORD(s_favourites[i].uid) || HIDWORD(s_favourites[i].uid)); ++i )
        ;
    if ( i < 0x2A )
    {
        v2 = &s_favourites[i];
        *(unsigned int *)v2->addressblob = serverid;
        *(_WORD *)&v2->addressblob[4] = WORD2(serverid);
        v3 = i;
        LODWORD(s_favourites[v3].uid) = serveruid;
        *(unsigned int *)(v3 * 16 + 174313908) = HIDWORD(serveruid);
    }
    CL_SetFavourites_f();
}

void __cdecl Live_DeleteFavourite(unsigned __int64 serverid)
{
    unsigned int v1; // eax
    unsigned int i; // [esp+4h] [ebp-4h]

    for ( i = 0; i < 0x2A && serverid != __PAIR64__(HIDWORD(s_favourites[i].uid), s_favourites[i].uid); ++i )
        ;
    if ( i < 0x2A )
    {
        v1 = i;
        LODWORD(s_favourites[v1].uid) = 0;
        *(unsigned int *)(v1 * 16 + 174313908) = 0;
        CL_SetFavourites_f();
    }
}

void __cdecl Live_ParseFavsBlobs(unsigned __int8 *addrblob, unsigned __int8 *uidblob)
{
    favourite_t *v2; // eax
    unsigned int v3; // edx
    unsigned int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < 0x2A && *uidblob; ++i )
    {
        v2 = &s_favourites[i];
        *(unsigned int *)v2->addressblob = *(unsigned int *)addrblob;
        *(_WORD *)&v2->addressblob[4] = *((_WORD *)addrblob + 2);
        v3 = i;
        LODWORD(s_favourites[v3].uid) = *(unsigned int *)uidblob;
        *(unsigned int *)(v3 * 16 + 174313908) = *((unsigned int *)uidblob + 1);
        addrblob += 6;
        uidblob += 8;
    }
}

void __cdecl Live_SetFavsBlobs(unsigned __int8 *addrblob, unsigned __int8 *uidblob)
{
    favourite_t *v2; // eax
    unsigned int i; // [esp+4h] [ebp-4h]

    memset(addrblob, 0, 0xFDu);
    memset(uidblob, 0, 0x151u);
    for ( i = 0; i < 0x2A; ++i )
    {
        if ( LODWORD(s_favourites[i].uid) || HIDWORD(s_favourites[i].uid) )
        {
            v2 = &s_favourites[i];
            *(unsigned int *)addrblob = *(unsigned int *)v2->addressblob;
            *((_WORD *)addrblob + 2) = *(_WORD *)&v2->addressblob[4];
            addrblob += 6;
            *(unsigned int *)uidblob = s_favourites[i].uid;
            *((unsigned int *)uidblob + 1) = HIDWORD(s_favourites[i].uid);
            uidblob += 8;
        }
    }
}

void __cdecl Live_GetPrivateProfileComplete()
{
    Live_ParseFavsBlobs(s_profileInfo.m_memberfavsblob, s_profileInfo.m_memberuids);
}

void __cdecl Live_GetPrivateProfileFailure()
{
    serverInfo_t *favservers; // [esp+0h] [ebp-8h] BYREF
    int *count; // [esp+4h] [ebp-4h] BYREF

    Com_PrintWarning(23, "Couldn't get favourites :(\n");
    favservers = 0;
    count = 0;
    if ( CL_GetServerList(3, &favservers, &count) )
        *count = 0;
}

void __cdecl Live_SetPrivateProfileComplete()
{
    Com_DPrintf(23, "Successfully saved favourites to DemonWare\n");
}

void __cdecl Live_SetPrivateProfileFailure()
{
    Com_PrintWarning(23, "Couldn't save favourites to DemonWare :(\n");
}

TaskRecord *__cdecl Live_GetPrivateProfile()
{
#ifdef KISAK_LIVE_STUBS
    const bdReference<bdCommonAddr> *PrivateInfo; // eax
    bdReference<bdCommonAddr> v2; // [esp+1Ch] [ebp-10h] BYREF
    bdProfiles *profileService; // [esp+20h] [ebp-Ch]
    bdLobbyService *lobbyService; // [esp+24h] [ebp-8h]
    TaskRecord *task; // [esp+28h] [ebp-4h]

    task = 0;
    lobbyService = dwGetLobby(0);
    if ( lobbyService )
    {
        profileService = bdLobbyService::getProfiles(lobbyService);
        if ( profileService )
        {
            task = TaskManager2_CreateTask(task_dwgetprivateprofile, 0, 0, 0);
            if ( task )
            {
                PrivateInfo = (const bdReference<bdCommonAddr> *)bdProfiles::getPrivateInfo(
                                                                                                                     profileService,
                                                                                                                     (int)&v2,
                                                                                                                     &s_profileInfo);
                bdReference<bdCommonAddr>::operator=((bdReference<bdCommonAddr> *)&task->remoteTask, PrivateInfo);
                bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v2);
                TaskManager2_StartTask(task);
            }
        }
    }
    return task;
#else
    return NULL;
#endif
}

TaskRecord *__cdecl Live_SetPrivateProfile()
{
#ifdef KISAK_LIVE_STUBS
    const bdReference<bdCommonAddr> *v0; // eax
    bdReference<bdCommonAddr> v2; // [esp+1Ch] [ebp-10h] BYREF
    bdProfiles *profileService; // [esp+20h] [ebp-Ch]
    bdLobbyService *lobbyService; // [esp+24h] [ebp-8h]
    TaskRecord *task; // [esp+28h] [ebp-4h]

    task = 0;
    lobbyService = dwGetLobby(0);
    if ( lobbyService )
    {
        profileService = bdLobbyService::getProfiles(lobbyService);
        if ( profileService )
        {
            task = TaskManager2_CreateTask(task_dwsetprivateprofile, 0, 0, 0);
            if ( task )
            {
                s_profileInfo.m_entityID = g_bdUserID;
                v0 = (const bdReference<bdCommonAddr> *)bdProfiles::setPrivateInfo(
                                                                                                    profileService,
                                                                                                    (int)&v2,
                                                                                                    (int)&s_profileInfo);
                bdReference<bdCommonAddr>::operator=((bdReference<bdCommonAddr> *)&task->remoteTask, v0);
                bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v2);
                TaskManager2_StartTask(task);
            }
        }
    }
    return task;
#else
    return NULL;
#endif
}

void __cdecl CL_GetFavourites_f()
{
    Live_GetPrivateProfile();
}

void __cdecl CL_SetFavourites_f()
{
    Live_SetFavsBlobs(s_profileInfo.m_memberfavsblob, s_profileInfo.m_memberuids);
    Live_SetPrivateProfile();
}

void __cdecl CL_AddFavourite_f()
{
    const char *v0; // eax
    const char *v1; // eax
    int v2; // eax
    unsigned __int64 v3; // [esp-8h] [ebp-18h]

    v0 = Cmd_Argv(2);
    v3 = atol(v0);
    v1 = Cmd_Argv(1);
    v2 = atol(v1);
    Live_AddFavourite(v2, v3);
}

void __cdecl CL_DeleteFavourite_f()
{
    const char *v0; // eax
    int servernum; // [esp+8h] [ebp-Ch]
    int *count; // [esp+Ch] [ebp-8h] BYREF
    serverInfo_t *servers; // [esp+10h] [ebp-4h] BYREF

    v0 = Cmd_Argv(1);
    servernum = atoi(v0);
    servers = 0;
    if ( CL_GetServerList(3, &servers, &count) )
        Live_DeleteFavourite(servers[servernum].bdUserID);
}

void __cdecl CL_DumpFavourites_f()
{
    Live_DumpFavourites();
}

void __cdecl CL_NukeFavourites_f()
{
    memset(s_profileInfo.m_memberfavsblob, 0, sizeof(s_profileInfo.m_memberfavsblob));
    memset(s_profileInfo.m_memberuids, 0, sizeof(s_profileInfo.m_memberuids));
    Live_SetPrivateProfile();
}

cmd_function_s CL_GetFavourites_f_VAR;
cmd_function_s CL_SetFavourites_f_VAR;
cmd_function_s CL_AddFavourite_f_VAR;
cmd_function_s CL_DeleteFavourite_f_VAR;
cmd_function_s CL_DumpFavourites_f_VAR;
cmd_function_s CL_NukeFavourites_f_VAR;

void __cdecl Live_InitFavourites()
{
    unsigned int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < 0x29; ++i )
    {
        s_favourites[i].uid = 0;
    }

    memset(s_profileInfo.m_memberfavsblob, 0, sizeof(s_profileInfo.m_memberfavsblob));
    Cmd_AddCommandInternal("getfavourites", CL_GetFavourites_f, &CL_GetFavourites_f_VAR);
    Cmd_AddCommandInternal("setfavourites", CL_SetFavourites_f, &CL_SetFavourites_f_VAR);
    Cmd_AddCommandInternal("addfavourite", CL_AddFavourite_f, &CL_AddFavourite_f_VAR);
    Cmd_AddCommandInternal("deletefavourite", CL_DeleteFavourite_f, &CL_DeleteFavourite_f_VAR);
    Cmd_AddCommandInternal("dumpfavourites", CL_DumpFavourites_f, &CL_DumpFavourites_f_VAR);
    Cmd_AddCommandInternal("nukfavourites", CL_NukeFavourites_f, &CL_NukeFavourites_f_VAR);
}

void __cdecl Live_AddRecentPlayers(unsigned __int64 *uids, const char **names, int numIDs)
{
#ifdef KISAK_LIVE
    unsigned __int64 v3; // rax
    int i; // [esp+10h] [ebp-30h]
    char namebuf[32]; // [esp+14h] [ebp-2Ch] BYREF
    unsigned __int64 myXuid; // [esp+38h] [ebp-8h]

    memset(namebuf, 0, sizeof(namebuf));
    LODWORD(v3) = Live_GetXuid(0);
    myXuid = v3;
    for ( i = 0; i < numIDs; ++i )
    {
        if ( myXuid != uids[i] && (LODWORD(uids[i]) || HIDWORD(uids[i])) )
        {
            Live_GetRecentPlayerName(uids[i], namebuf, 32);
            if ( namebuf[0] )
            {
                LiveMeetPlayer_UpdateMeetPlayerList(0, uids[i], namebuf);
                memset(namebuf, 0, sizeof(namebuf));
            }
            else
            {
                LiveMeetPlayer_UpdateMeetPlayerList(0, uids[i], names[i]);
            }
        }
    }
#endif
}

unsigned __int64 __cdecl Live_GetServerForFriend(unsigned __int64 friendId)
{
#ifdef KISAK_LIVE_STUBS
    friendonserver_t *friendserver; // [esp+8h] [ebp-Ch]
    int i; // [esp+Ch] [ebp-8h]
    int numsteamfriends; // [esp+10h] [ebp-4h]

    numsteamfriends = LiveSteam_GetNumFriends();
    for ( i = 0; i < numsteamfriends; ++i )
    {
        friendserver = &s_friendsServers[i];
        if ( (LODWORD(friendserver->friendID) || HIDWORD(s_friendsServers[i].friendID))
            && friendId == friendserver->friendID )
        {
            return friendserver->serverID;
        }
    }
    return 0;
#else
    return 0;
#endif
}

void __cdecl Live_JoinSessionInProgressComplete(TaskRecord *task)
{
#ifdef KISAK_LIVE_STUBS
    int ControllerIndex; // eax
    netadr_t v2; // [esp-10h] [ebp-4Ch]
    unsigned int v3; // [esp+1Ch] [ebp-20h]
    int codpoints; // [esp+2Ch] [ebp-10h] BYREF
    bool wasbasictraining; // [esp+33h] [ebp-9h]
    int playlistRes; // [esp+34h] [ebp-8h]
    connstate_t connectionState; // [esp+38h] [ebp-4h]

    if ( bdTaskByteBuffer::getHeaderSize((bdTaskByteBuffer *)task->nestedTask->remoteTask.m_ptr) )
    {
        connectionState = CL_GetLocalClientConnectionState(0);
        if ( connectionState > CA_DISCONNECTED
            && !memcmp((const char *)&dwGetSessionID()->m_sessionID, (const char *)&s_joininprogressinfo.m_sessionID, 8) )
        {
            Com_DPrintf(23, "Not joining in progress, already in session!\n");
            return;
        }
        LOWORD(v3) = 0;
        *(_QWORD *)&v2.type = 4;
        *(_QWORD *)&v2.port = v3 | 0xFFFFFFFF00000000uLL;
        CL_InitServerInfo(&s_inviteServerinfo, v2);
        s_inviteServerinfo.xnkey = *(bdSecurityKey *)s_joininprogressinfo.m_membersecKey;
        s_inviteServerinfo.xnkid = s_joininprogressinfo.m_sessionID;
        memcpy(&s_inviteServerinfo, s_joininprogressinfo.m_hostAddr, 0x19u);
        memcpy(s_inviteServerinfo.hostName, s_joininprogressinfo.m_memberservername, sizeof(s_inviteServerinfo.hostName));
        Com_DPrintf(23, "GetSessionByID successful, joining session..\n");
        if ( s_joininprogressinfo.m_memberserverType == 5 || !s_joininprogressinfo.m_memberserverType )
            goto LABEL_19;
        playlistRes = Playlist_GetWagerAmount(
                                        s_joininprogressinfo.m_memberPLAYLIST_VERSION,
                                        s_joininprogressinfo.m_memberPLAYLIST,
                                        &s_inviteServerinfo.wagerBet);
        if ( playlistRes < 0 )
        {
            Cmd_ExecuteSingleCommand(0, 0, "openmenu WagerPlaylistIsOld");
            return;
        }
        if ( playlistRes > 0 )
        {
            Cmd_ExecuteSingleCommand(0, 0, "openmenu WagerPlaylistIsNew");
            return;
        }
        if ( s_inviteServerinfo.wagerBet <= 0 )
        {
LABEL_19:
            CL_Connect(&s_inviteServerinfo);
        }
        else
        {
            codpoints = 0;
            wasbasictraining = xblive_basictraining->current.enabled;
            Dvar_SetBool((dvar_s *)xblive_basictraining, 0);
            ControllerIndex = Com_LocalClient_GetControllerIndex(0);
            LiveStats_GetIntPlayerStat(ControllerIndex, &codpoints, "CODPOINTS");
            Dvar_SetIntByName("ui_wagerbet", s_inviteServerinfo.wagerBet);
            if ( codpoints < s_inviteServerinfo.wagerBet )
            {
                Cmd_ExecuteSingleCommand(0, 0, "openmenu WagerDeadBeat");
            }
            else
            {
                Dvar_SetIntByName("ui_joiningwagerfrominvite", 1);
                if ( CL_GetLocalClientConnectionState(0) == 10 )
                    Cmd_ExecuteSingleCommand(0, 0, "openmenu WagerConfirmationingame");
                else
                    Cmd_ExecuteSingleCommand(0, 0, "openmenu WagerConfirmation");
                Cmd_ExecuteSingleCommand(0, 0, "openmenu WagerConfirmation");
            }
            Dvar_SetBool((dvar_s *)xblive_basictraining, wasbasictraining);
        }
    }
#endif
}

TaskRecord *__cdecl Live_JoinSessionInProgress(unsigned __int64 uid, bool recent)
{
#ifdef KISAK_LIVE_STUBS
    unsigned __int64 ServerForFriend; // [esp+0h] [ebp-1Ch]
    TaskRecord *nestedTask; // [esp+8h] [ebp-14h]
    unsigned __int64 serverid; // [esp+Ch] [ebp-10h] BYREF
    TaskRecord *task; // [esp+18h] [ebp-4h]

    task = 0;
    if ( recent )
        ServerForFriend = uid;
    else
        ServerForFriend = Live_GetServerForFriend(uid);
    serverid = ServerForFriend;
    if ( ServerForFriend )
    {
        nestedTask = dwFindSessionsByEntityIDs(&serverid, 1u, &s_joininprogressinfo);
        if ( nestedTask )
        {
            task = TaskManager2_CreateTask(task_joinSessionInProgress, 0, nestedTask, 0);
            if ( task )
                TaskManager2_StartTask(task);
            else
                Com_PrintError(23, "Join session in progress systemic failure, not connected?\n");
        }
    }
    return task;
#else
    return NULL;
#endif
}

void __cdecl Live_AddFriendServer(unsigned __int64 serverID, unsigned __int64 friendID)
{
#ifdef KISAK_LIVE_STUBS
    int v2; // ecx
    int v3; // edx
    int v4; // edx
    int i; // [esp+4h] [ebp-8h]
    int numfriends; // [esp+8h] [ebp-4h]

    numfriends = LiveSteam_GetNumFriends();
    for ( i = 0; i < numfriends; ++i )
    {
        if ( __PAIR64__(HIDWORD(s_friendsServers[i].friendID), s_friendsServers[i].friendID) == friendID )
        {
            v2 = i;
            LODWORD(s_friendsServers[v2].serverID) = serverID;
            *(unsigned int *)(v2 * 16 + 174309628) = HIDWORD(serverID);
            break;
        }
    }
    if ( i == numfriends )
    {
        v3 = s_numfriendsonservers;
        LODWORD(s_friendsServers[v3].friendID) = friendID;
        *(unsigned int *)(v3 * 16 + 174309620) = HIDWORD(friendID);
        v4 = s_numfriendsonservers;
        LODWORD(s_friendsServers[v4].serverID) = serverID;
        *(unsigned int *)(v4 * 16 + 174309628) = HIDWORD(serverID);
        ++s_numfriendsonservers;
    }
    Live_FindFriendServers();
#endif
}

void __cdecl Live_OnInvite(unsigned __int64 uid, bdSessionID sessionID, const char *password)
{
#ifdef KISAK_LIVE_STUBS
    char *v3; // eax
    int LocalClientNum; // eax
    char *v5; // eax
    int v6; // eax
    int v7; // edx
    bdSessionID *p_sessionID; // eax
    int v9; // ecx
    int v10; // edx
    int v11; // ecx
    bdSessionID *v12; // edx
    char *v13; // [esp-Ch] [ebp-130h]
    char *v14; // [esp-Ch] [ebp-130h]
    __int64 v15; // [esp+0h] [ebp-124h]
    const char *toastPopupDesc; // [esp+24h] [ebp-100h]
    char *toastMessage; // [esp+28h] [ebp-FCh]
    XuidInfo xinfo; // [esp+2Ch] [ebp-F8h] BYREF
    FriendInfo info; // [esp+5Ch] [ebp-C8h] BYREF
    int firstfree; // [esp+118h] [ebp-Ch]
    int i; // [esp+11Ch] [ebp-8h]

    if ( Friends_GetByID(0, uid, &info) )
    {
        v3 = UI_SafeTranslateString("MENU_INVITE_RECEIVED_FROM");
        toastMessage = UI_ReplaceConversionString(v3, info.name);
        v13 = UI_SafeTranslateString("");
        LocalClientNum = Com_ControllerIndex_GetLocalClientNum(0);
        UI_OpenToastPopup(LocalClientNum, "menu_mp_killstreak_select", v13, toastMessage, 2700);
    }
    else if ( LiveMeetPlayer_GetRecentPlayerInfoByID(0, uid, &xinfo) )
    {
        v5 = UI_SafeTranslateString("MENU_INVITE_RECEIVED_FROM");
        toastPopupDesc = UI_ReplaceConversionString(v5, xinfo.gamertag);
        v14 = UI_SafeTranslateString("");
        v6 = Com_ControllerIndex_GetLocalClientNum(0);
        UI_OpenToastPopup(v6, "menu_mp_killstreak_select", v14, toastPopupDesc, 2700);
    }
    firstfree = -1;
    if ( CL_GetLocalClientConnectionState(0) == 10
        && !memcmp((const char *)&sessionID.m_sessionID, (const char *)&dwGetSessionID()->m_sessionID, 8) )
    {
        Com_DPrintf(23, "Ignoring invite from %llu, already in session!\n", v15);
        bdTaskResult::~bdTaskResult(&sessionID);
    }
    else
    {
        for ( i = 0; i < 256; ++i )
        {
            if ( uid == __PAIR64__(HIDWORD(s_invites[i].from), s_invites[i].from) )
            {
                v7 = *(unsigned int *)&sessionID.m_sessionID.ab[4];
                p_sessionID = &s_invites[i].sessionID;
                *(unsigned int *)p_sessionID->m_sessionID.ab = *(unsigned int *)sessionID.m_sessionID.ab;
                *(unsigned int *)&p_sessionID->m_sessionID.ab[4] = v7;
                v9 = i;
                LODWORD(s_invites[v9].from) = uid;
                *(unsigned int *)(v9 * 56 + 174495612) = HIDWORD(uid);
                memcpy(s_invites[i].password, password, sizeof(s_invites[i].password));
                break;
            }
            if ( firstfree == -1 && !LODWORD(s_invites[i].from) && !HIDWORD(s_invites[i].from) )
                firstfree = i;
        }
        if ( i == 256 && firstfree != -1 )
        {
            v10 = firstfree;
            LODWORD(s_invites[v10].from) = uid;
            *(unsigned int *)(v10 * 56 + 174495612) = HIDWORD(uid);
            v11 = *(unsigned int *)&sessionID.m_sessionID.ab[4];
            v12 = &s_invites[firstfree].sessionID;
            *(unsigned int *)v12->m_sessionID.ab = *(unsigned int *)sessionID.m_sessionID.ab;
            *(unsigned int *)&v12->m_sessionID.ab[4] = v11;
            memcpy(s_invites[firstfree].password, password, sizeof(s_invites[firstfree].password));
        }
        s_lastInvite = uid;
        bdTaskResult::~bdTaskResult(&sessionID);
    }
#endif
}

void __cdecl Live_OnRevokeInvite(unsigned __int64 uid)
{
#ifdef KISAK_LIVE_STUBS
    int v1; // edx
    int i; // [esp+4h] [ebp-4h]

    for ( i = 0; i < 256; ++i )
    {
        if ( uid == __PAIR64__(HIDWORD(s_invites[i].from), s_invites[i].from) )
        {
            Com_DPrintf(23, "Removing invite from %llu\n", uid);
            v1 = i;
            LODWORD(s_invites[v1].from) = 0;
            *(unsigned int *)(v1 * 56 + 174495612) = 0;
            break;
        }
    }
    if ( s_lastInvite == uid )
        s_lastInvite = 0;
#endif
}

bool __cdecl Live_RevokeInvite(unsigned __int64 friendID)
{
#ifdef KISAK_LIVE_STUBS
    char msg; // [esp+7h] [ebp-1h] BYREF

    msg = 9;
    return dwMessaging_SendInstantMessage(friendID, &msg, 1u) != 0;
#else
    return false;
#endif
}

char __cdecl Live_FindInviteFromFriend(unsigned __int64 friendID, bdSessionID *sessionID, char **password)
{
#ifdef KISAK_LIVE_STUBS
    int v3; // edx
    int i; // [esp+4h] [ebp-8h]
    bool retval; // [esp+Bh] [ebp-1h]

    retval = 0;
    for ( i = 0; i < 256; ++i )
    {
        if ( friendID == __PAIR64__(HIDWORD(s_invites[i].from), s_invites[i].from) )
        {
            v3 = *(unsigned int *)&s_invites[i].sessionID.m_sessionID.ab[4];
            *(unsigned int *)sessionID->m_sessionID.ab = *(unsigned int *)s_invites[i].sessionID.m_sessionID.ab;
            *(unsigned int *)&sessionID->m_sessionID.ab[4] = v3;
            *password = s_invites[i].password;
            return 1;
        }
    }
    return retval;
#else
    return false;
#endif
}

char __cdecl Live_HandleInviteMessage(unsigned __int64 senderID, char *message)
{
    bool handled; // [esp+7h] [ebp-5h]

    handled = 0;
    if ( *message == 9 )
    {
        Live_OnRevokeInvite(senderID);
        return 1;
    }
    else
    {
        Com_PrintWarning(23, "Unknown live message %u\n", *message);
    }
    return handled;
}

void __cdecl Live_AcceptInviteAsyncFailure()
{
    Com_PrintError(23, "couldn't accept invite :(\n");
}

void __cdecl Live_JoinWagerFromInvite()
{
#ifdef KISAK_LIVE_STUBS
    CL_Connect(&s_inviteServerinfo);
#endif
}

void __cdecl Live_AcceptInviteAsyncComplete(TaskRecord *task)
{
#ifdef KISAK_LIVE_STUBS
    int ControllerIndex; // eax
    netadr_t v2; // [esp-10h] [ebp-40h]
    unsigned int v3; // [esp+10h] [ebp-20h]
    int codpoints; // [esp+20h] [ebp-10h] BYREF
    bool wasbasictraining; // [esp+27h] [ebp-9h]
    int playlistRes; // [esp+28h] [ebp-8h]
    MatchMakingInfo *mminfo; // [esp+2Ch] [ebp-4h]

    if ( (!task->nestedTask || !task->nestedTask->payload)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_win.cpp",
                    6413,
                    0,
                    "%s",
                    "task->nestedTask && task->nestedTask->payload") )
    {
        __debugbreak();
    }
    if ( bdTaskByteBuffer::getHeaderSize((bdTaskByteBuffer *)task->nestedTask->remoteTask.m_ptr) )
    {
        LOWORD(v3) = 0;
        *(_QWORD *)&v2.type = 4;
        *(_QWORD *)&v2.port = v3 | 0xFFFFFFFF00000000uLL;
        CL_InitServerInfo(&s_inviteServerinfo, v2);
        mminfo = (MatchMakingInfo *)task->nestedTask->payload;
        s_inviteServerinfo.xnkey = *(bdSecurityKey *)mminfo->m_membersecKey;
        s_inviteServerinfo.xnkid = mminfo->m_sessionID;
        memcpy(&s_inviteServerinfo, mminfo->m_hostAddr, 0x19u);
        memcpy(s_inviteServerinfo.hostName, mminfo->m_memberservername, sizeof(s_inviteServerinfo.hostName));
        if ( mminfo->m_memberserverType == 5 || !mminfo->m_memberserverType )
            goto LABEL_19;
        playlistRes = Playlist_GetWagerAmount(
                                        mminfo->m_memberPLAYLIST_VERSION,
                                        mminfo->m_memberPLAYLIST,
                                        &s_inviteServerinfo.wagerBet);
        if ( playlistRes < 0 )
        {
            Cmd_ExecuteSingleCommand(0, 0, "openmenu WagerPlaylistIsOld");
            return;
        }
        if ( playlistRes > 0 )
        {
            Cmd_ExecuteSingleCommand(0, 0, "openmenu WagerPlaylistIsNew");
            return;
        }
        if ( s_inviteServerinfo.wagerBet <= 0 )
        {
LABEL_19:
            Com_DPrintf(23, "Acceptinvite async successful, joining session..\n");
            CL_Connect(&s_inviteServerinfo);
        }
        else
        {
            codpoints = 0;
            wasbasictraining = xblive_basictraining->current.enabled;
            Dvar_SetBool((dvar_s *)xblive_basictraining, 0);
            ControllerIndex = Com_LocalClient_GetControllerIndex(0);
            LiveStats_GetIntPlayerStat(ControllerIndex, &codpoints, "CODPOINTS");
            Dvar_SetIntByName("ui_wagerbet", s_inviteServerinfo.wagerBet);
            if ( codpoints < s_inviteServerinfo.wagerBet )
            {
                Cmd_ExecuteSingleCommand(0, 0, "openmenu WagerDeadBeat");
            }
            else
            {
                Dvar_SetIntByName("ui_joiningwagerfrominvite", 1);
                if ( CL_GetLocalClientConnectionState(0) == 10 )
                    Cmd_ExecuteSingleCommand(0, 0, "openmenu WagerConfirmationingame");
                else
                    Cmd_ExecuteSingleCommand(0, 0, "openmenu WagerConfirmation");
            }
            Dvar_SetBool((dvar_s *)xblive_basictraining, wasbasictraining);
        }
    }
    else
    {
        Live_AcceptInviteAsyncFailure();
    }
#endif
}

TaskRecord *__cdecl Live_AcceptInviteAsync(bdSessionID sessionID)
{
#ifdef KISAK_LIVE_STUBS
    TaskRecord *nestedTask; // [esp+4h] [ebp-Ch]
    TaskRecord *task; // [esp+8h] [ebp-8h]

    nestedTask = dwMatchMaking_FindSessionByID(&sessionID);
    task = 0;
    if ( nestedTask )
    {
        task = TaskManager2_CreateTask(task_acceptInvite, 0, nestedTask, 0);
        if ( task )
            TaskManager2_StartTask(task);
    }
    bdTaskResult::~bdTaskResult(&sessionID);
    return task;
#else
    return NULL;
#endif
}

void __cdecl Live_AcceptInvite(unsigned __int64 frienduid)
{
#ifdef KISAK_LIVE_STUBS
    bdSessionID v1; // [esp-Ch] [ebp-3Ch] BYREF
    int v2; // [esp+Ch] [ebp-24h]
    int v3; // [esp+10h] [ebp-20h]
    bdSessionID friendSession; // [esp+1Ch] [ebp-14h] BYREF
    char *password; // [esp+2Ch] [ebp-4h] BYREF

    bdSessionID::bdSessionID(&friendSession);
    if ( Live_FindInviteFromFriend(frienduid, &friendSession, &password) )
    {
        Dvar_SetStringByName("password", password);
        Live_OnRevokeInvite(frienduid);
        if ( CL_GetLocalClientConnectionState(0) >= 4 )
        {
            v3 = memcmp((const char *)&dwGetSessionID()->m_sessionID, (const char *)&friendSession.m_sessionID, 8);
            v2 = v3;
            if ( !v3 )
            {
                bdTaskResult::~bdTaskResult(&friendSession);
                return;
            }
        }
        bdSessionID::bdSessionID(&v1, &friendSession);
        if ( !Live_AcceptInviteAsync(v1) )
            Com_PrintError(23, "Accept invite systemic failure, not connected?\n");
    }
    bdTaskResult::~bdTaskResult(&friendSession);
#endif
}

int __cdecl Live_GetInvitesCount()
{
#ifdef KISAK_LIVE_STUBS
    int i; // [esp+4h] [ebp-8h]
    int count; // [esp+8h] [ebp-4h]

    count = 0;
    for ( i = 0; i < 256; ++i )
    {
        if ( LODWORD(s_invites[i].from) || HIDWORD(s_invites[i].from) )
            ++count;
    }
    return count;
#else
    return 0;
#endif
}

int __cdecl Live_GetInviteFriend(int index)
{
#ifdef KISAK_LIVE_STUBS
    int i; // [esp+4h] [ebp-8h]
    int inviteIndex; // [esp+8h] [ebp-4h]

    inviteIndex = 0;
    for ( i = 0; i < 256; ++i )
    {
        if ( LODWORD(s_invites[i].from) || HIDWORD(s_invites[i].from) )
        {
            if ( index == inviteIndex )
                return s_invites[i].from;
            ++inviteIndex;
        }
    }
    return 0;
#else
    return 0;
#endif
}

void __cdecl Live_ListInvites_f()
{
#ifdef KISAK_LIVE_STUBS
    unsigned __int64 v0; // [esp-Ch] [ebp-D0h]
    __int64 v1; // [esp-4h] [ebp-C8h]
    FriendInfo finfo; // [esp+4h] [ebp-C0h] BYREF
    int i; // [esp+C0h] [ebp-4h]

    Com_Printf(23, "\n*********************\nFriend Name\t\tSessionId\n*********************\n");
    for ( i = 0; i < 256; ++i )
    {
        HIDWORD(v1) = 56 * i;
        if ( LODWORD(s_invites[i].from) || HIDWORD(s_invites[i].from) )
        {
            HIDWORD(v0) = HIDWORD(s_invites[i].from);
            LODWORD(v0) = s_invites[i].from;
            Friends_GetByID(0, v0, &finfo);
            LODWORD(v1) = &s_invites[i].sessionID.m_sessionID;
            Com_Printf(23, "%s\t\t%llu\n", finfo.name, v1);
        }
    }
    Com_Printf(23, "\n*********************\n");
#endif
}

void __cdecl Live_AddPlayerAsFriend_f()
{
    const char *v0; // eax
    __int64 xuid; // [esp+10h] [ebp-8h]

    if ( Cmd_Argc() == 2 )
    {
        v0 = Cmd_Argv(1);
        xuid = I_atoi64(v0);
        LiveSteam_PopOverlayForSteamID(xuid);
    }
    else
    {
        Com_Printf(14, "usage: xaddfriend <xuid>\n");
    }
}

bool __cdecl Live_ShouldBroadcastNewServer()
{
    return !Demo_IsPlaying();
}

void __cdecl Live_RespondToSessionRequest(unsigned __int64 from, unsigned __int8 flags)
{
#ifdef KISAK_LIVE_STUBS
    unsigned __int8 message[12]; // [esp+8h] [ebp-20h] BYREF
    unsigned __int64 sessionUid; // [esp+18h] [ebp-10h]
    unsigned __int8 messageflags; // [esp+27h] [ebp-1h]

    sessionUid = 0;
    if ( Live_ShouldBroadcastNewServer() )
    {
        messageflags = 0;
        if ( CL_GetLocalClientConnectionState(0) >= 6 )
            messageflags |= 0x10u;
        else
            cls.serveruid = 0;
        sessionUid = cls.serveruid;
        if ( (flags & 4) != 0 )
        {
            messageflags |= 4u;
        }
        else
        {
            if ( (flags & 8) == 0 )
            {
                Com_PrintWarning(23, "invalid message type from %llu!\n", from);
                return;
            }
            messageflags |= 8u;
        }
        message[0] = messageflags | 2;
        *(_QWORD *)&message[1] = sessionUid;
        LiveSteam_SendP2PMessage(from, message, 9u);
    }
#endif
}

void __cdecl Live_RequestSessionsFromFriends()
{
#ifdef KISAK_LIVE_STUBS
    unsigned __int64 FriendXuid; // rax
    int i; // [esp+0h] [ebp-8h]
    unsigned __int8 message; // [esp+7h] [ebp-1h] BYREF

    message = 5;
    for ( i = 0; i < LiveSteam_GetNumFriends(); ++i )
    {
        FriendXuid = LiveSteam_GetFriendXuid(i);
        LiveSteam_SendP2PMessage(FriendXuid, &message, 1u);
    }
#endif
}

void __cdecl Live_RequestSessionsFromRecentPlayers()
{
#ifdef KISAK_LIVE_STUBS
    XuidInfo result; // [esp+8h] [ebp-70h] BYREF
    unsigned __int64 v1[6]; // [esp+38h] [ebp-40h] BYREF
    unsigned __int64 recentUid; // [esp+68h] [ebp-10h]
    int i; // [esp+70h] [ebp-8h]
    unsigned __int8 message; // [esp+77h] [ebp-1h] BYREF

    message = 9;
    for ( i = 0; i < LiveMeetPlayer_GetNoOfMetPlayers(0); ++i )
    {
        memcpy(v1, LiveMeetPlayer_GetMetFriendInfo(&result, 0, i), sizeof(v1));
        recentUid = v1[0];
        LiveSteam_SendP2PMessage(v1[0], &message, 1u);
    }
#endif
}

void __cdecl Live_DispatchP2PMessage(unsigned __int8 *message, unsigned int messagesize, unsigned __int64 from)
{
#ifdef KISAK_LIVE_STUBS
    unsigned __int64 uid; // [esp+0h] [ebp-10h]
    unsigned __int8 flags; // [esp+Fh] [ebp-1h]

    if ( dwGetLogOnStatus(0) == 4 )
    {
        flags = *message;
        if ( (*message & 1) != 0 )
        {
            Live_RespondToSessionRequest(from, flags);
        }
        else if ( (flags & 2) != 0 )
        {
            if ( messagesize >= 9 )
            {
                uid = *(_QWORD *)(message + 1);
                if ( flags >= 0x10u )
                {
                    if ( (flags & 0x40) != 0 )
                        uid = 0;
                }
                else
                {
                    uid = 0;
                    Live_OnRevokeInvite(from);
                }
                if ( (flags & 4) != 0 )
                {
                    Live_AddFriendServer(uid, from);
                }
                else if ( (flags & 8) != 0 )
                {
                    if ( (flags & 0x20) != 0 )
                    {
                        uid = 0;
                        Live_OnRevokeInvite(from);
                    }
                    LiveMeetPlayer_UpdatePlayerSession(from, uid);
                }
            }
            else
            {
                Com_PrintWarning(14, "Dropped runt p2p message on the floor from %llu\n", from);
            }
        }
    }
#endif
}

void __cdecl Live_BroadcastSessionToFriends(unsigned __int64 sessionUID, unsigned __int8 flags)
{
#ifdef KISAK_LIVE_STUBS
    unsigned __int64 FriendXuid; // rax
    int i; // [esp+8h] [ebp-14h]
    unsigned __int8 message[12]; // [esp+Ch] [ebp-10h] BYREF

    message[0] = flags | 6;
    *(_QWORD *)&message[1] = sessionUID;
    for ( i = 0; i < Friends_GetCount(0, 0); ++i )
    {
        FriendXuid = LiveSteam_GetFriendXuid(i);
        LiveSteam_SendP2PMessage(FriendXuid, message, 9u);
    }
#endif
}

void __cdecl Live_BroadcastSessionToRecentPlayers(unsigned __int64 sessionUID, unsigned __int8 flags)
{
#ifdef KISAK_LIVE_STUBS
    XuidInfo result; // [esp+10h] [ebp-A8h] BYREF
    _BYTE v3[48]; // [esp+40h] [ebp-78h] BYREF
    XuidInfo recentinfo; // [esp+70h] [ebp-48h] BYREF
    int i; // [esp+A0h] [ebp-18h]
    unsigned __int8 message[12]; // [esp+A4h] [ebp-14h] BYREF
    int numplayers; // [esp+B4h] [ebp-4h]

    message[0] = flags | 0xA;
    *(_QWORD *)&message[1] = sessionUID;
    numplayers = LiveMeetPlayer_GetNoOfMetPlayers(0);
    for ( i = 0; i < numplayers; ++i )
    {
        memcpy(v3, LiveMeetPlayer_GetMetFriendInfo(&result, 0, i), sizeof(v3));
        memcpy(&recentinfo, v3, sizeof(recentinfo));
        LiveSteam_SendP2PMessage(recentinfo.playerXuids, message, 9u);
    }
#endif
}

void __cdecl Live_BroadcastSessionIfNeeded()
{
#ifdef KISAK_LIVE_STUBS
    unsigned __int8 flags; // [esp+0h] [ebp-2h]
    bool needtobroadcast; // [esp+1h] [ebp-1h]

    if ( (_S1_14 & 1) == 0 )
    {
        _S1_14 |= 1u;
        cachedState = CL_GetLocalClientConnectionState(0);
    }
    if ( cachedState != CL_GetLocalClientConnectionState(0) )
    {
        needtobroadcast = 0;
        if ( cachedState != CA_ACTIVE || CL_GetLocalClientConnectionState(0) )
        {
            if ( cachedState != CA_ACTIVE && CL_GetLocalClientConnectionState(0) == 10 )
            {
                if ( cls.serveruid )
                    Live_AddRecentServer(cls.serveruid);
                else
                    dwGetOnlineUserID(0, &cls.serveruid);
                if ( !Demo_IsPlaying() )
                    needtobroadcast = 1;
            }
        }
        else
        {
            cls.serveruid = 0;
            needtobroadcast = 1;
        }
        if ( needtobroadcast )
        {
            flags = 0;
            if ( cls.serveruid )
                flags = 16;
            Live_BroadcastSessionToFriends(cls.serveruid, flags);
            Live_BroadcastSessionToRecentPlayers(cls.serveruid, flags);
        }
        cachedState = CL_GetLocalClientConnectionState(0);
    }
#endif
}

