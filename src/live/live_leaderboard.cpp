#include "live_leaderboard.h"
#include <universal/assertive.h>
#include <win32/win_shared.h>
#include <cstring>
#include <DW/dwUtils_pc.h>
#include <DW/dwStats.h>
#include <DW/dwTasks.h>
#include <qcommon/com_clients.h>
#include "live_win_common.h"
#include <client/cl_rank.h>
#include "live_win.h"
#include <client_mp/cl_ui_pc_mp.h>
#include "live_friends_pc.h"
#include <game_mp/g_main_mp.h>
#include "live_stats.h"
#include "live.h"
#include "live_storage_win.h"
#include <DW/dwUtils.h>

const int GlobalLbStat[4][19] =
{
  { 16, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 8, 9, 10, 17, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 1, 2, 3, 18, 4, 11, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

const int lbStatCounts[8] =
{ 4, 3, 4, 3, 4, 4, 4, 4 };

const int GlobalLbStatCounts[4] =
{ 3, 4, 7, 3 };

const char *lbFilterEnum_6[4] =
{ "none", "friends", "lobbymembers", NULL };

const char *lbResetPeriodEnum_6[4] =
{ "all_time", "weekly", "monthly", NULL };

const int LbStatPerLbGameMode[8][17] =
{
  { 6, 7, 8, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 6, 7, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 10, 11, 12, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 10, 12, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 14, 15, 12, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 14, 15, 12, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 10, 16, 12, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 14, 15, 12, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

const char *GlobalLbStatStrings[73] =
{
  "SCORE_PER_MINUTE",
  "KILLS",
  "DEATHS",
  "KDRATIO",
  "ASSISTS",
  "RANKXP",
  "PLEVEL",
  "TIME_PLAYED_TOTAL",
  "WINS",
  "LOSSES",
  "WLRATIO",
  "HEADSHOTS",
  "ACCURACY",
  "CODPOINTS",
  "CURRENCYEARNED",
  "CURRENCYSPENT",
  "SCORE",
  "WIN_STREAK",
  "KILL_STREAK",
  "bad allocation",
  "CA_DISCONNECTED",
  "CA_CINEMATIC",
  "CA_UICINEMATIC",
  "CA_LOGO",
  "CA_CONNECTING",
  "CA_CHALLENGING",
  "CA_CONNECTED",
  "CA_SENDINGSTATS",
  "CA_LOADING",
  "CA_PRIMED",
  "CA_ACTIVE",
  "bad allocation",
  "CA_DISCONNECTED",
  "CA_CINEMATIC",
  "CA_UICINEMATIC",
  "CA_LOGO",
  "CA_CONNECTING",
  "CA_CHALLENGING",
  "CA_CONNECTED",
  "CA_SENDINGSTATS",
  "CA_LOADING",
  "CA_PRIMED",
  "CA_ACTIVE",
  "Gamestate",
  "Snapshots",
  "MatchStates",
  "PlayerStates",
  "Entities",
  "Clients",
  "Actors",
  "AnimCmds",
  "none",
  "segmentName",
  "clipNameIngame",
  "clipDescIngame",
  "screenshotNameIngame",
  "screenshotDescIngame",
  "fileshareFileName",
  "fileshareFileDescription",
  "MENU_SEGMENT_TAG_KILL",
  "MENU_SEGMENT_TAG_DEATH",
  "MENU_SEGMENT_TAG_KILLSTREAK",
  "MENU_SEGMENT_TAG_PERK",
  "MENU_SEGMENT_TAG_WEAPON",
  "MENU_SEGMENT_TAG_EQUIPMENT",
  "MENU_SEGMENT_TAG_GRENADE",
  "MENU_SEGMENT_TAG_MELEE",
  "MENU_DEMO_TRANSITION_CUT",
  "MENU_DEMO_TRANSITION_FADE",
  "listen server",
  "dedicated LAN server",
  "dedicated internet server",
  NULL,
};


const char *lbStatStrings[18] =
{
  "MATCHES_PLAYED",
  "SCORE",
  "TIME_PLAYED_TOTAL",
  "SCORE_PER_MINUTE",
  "RANKXP",
  "PLEVEL",
  "KILLS",
  "DEATHS",
  "KDRATIO",
  "ASSISTS",
  "CAPTURES",
  "RETURNS",
  "DEFENDS",
  "PICKUPS",
  "PLANTS",
  "DEFUSES",
  "DESTRUCTIONS",
  NULL
};

const char *lbTypeEnum_6[17] =
{
  "tdm",
  "dm",
  "ctf",
  "dom",
  "sab",
  "sd",
  "koth",
  "dem",
  "hctdm",
  "hcdm",
  "hcctf",
  "hcdom",
  "hcsab",
  "hcsd",
  "hckoth",
  "hcdem",
  NULL
};

const int GlobalLbViewIds[4] =
{ 3000, 3001, 3002, 3003 };

const int lbViewIds[160] =
{
  1000,
  1001,
  1002,
  1003,
  1004,
  1005,
  1006,
  1007,
  1100,
  1101,
  1102,
  1103,
  1104,
  1105,
  1106,
  1107,
  1200,
  1201,
  1202,
  1203,
  1204,
  1205,
  1206,
  1207,
  1300,
  1301,
  1302,
  1303,
  1304,
  1305,
  1306,
  1307,
  1400,
  1401,
  1402,
  1403,
  1404,
  1405,
  1406,
  1407,
  1500,
  1501,
  1502,
  1503,
  1504,
  1505,
  1506,
  1507,
  1600,
  1601,
  1602,
  1603,
  1604,
  1605,
  1606,
  1607,
  1700,
  1701,
  1702,
  1703,
  1704,
  1705,
  1706,
  1707,
  1800,
  1801,
  1802,
  1803,
  1804,
  1805,
  1806,
  1807,
  1900,
  1901,
  1902,
  1903,
  1904,
  1905,
  1906,
  1907,
  2000,
  2001,
  2002,
  2003,
  2004,
  2005,
  2006,
  2007,
  2100,
  2101,
  2102,
  2103,
  2104,
  2105,
  2106,
  2107,
  2200,
  2201,
  2202,
  2203,
  2204,
  2205,
  2206,
  2207,
  2300,
  2301,
  2302,
  2303,
  2304,
  2305,
  2306,
  2307,
  2400,
  2401,
  2402,
  2403,
  2404,
  2405,
  2406,
  2407,
  2500,
  2501,
  2502,
  2503,
  2504,
  2505,
  2506,
  2507,
  2600,
  2601,
  2602,
  2603,
  2604,
  2605,
  2606,
  2607,
  2700,
  2701,
  2702,
  2703,
  2704,
  2705,
  2706,
  2707,
  2800,
  2801,
  2802,
  2803,
  2804,
  2805,
  2806,
  2807,
  2900,
  2901,
  2902,
  2903,
  2904,
  2905,
  2906,
  2907
};



const dvar_s *lb_maxrows;
const dvar_s *lb_escrowTimeout;
const dvar_s *lb_typeByResetPeriod;
const dvar_s *lb_type;
const dvar_s *lb_prestige;
const dvar_s *lb_filter;
const dvar_s *lb_LastFetchTime;
const dvar_s *lb_escrowRefresh;
LbLookup g_LbLookup;
const dvar_s *lb_minrefresh;
const dvar_s *lb_forceLbWrite;

LbGlob g_lbGlob;

bool __cdecl LB_NeedToReadLeaderBoard()
{
    if ( !lb_LastFetchTime
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                    38,
                    0,
                    "%s",
                    "lb_LastFetchTime") )
    {
        __debugbreak();
    }
    return lb_LastFetchTime && (int)(Sys_Milliseconds() - lb_LastFetchTime->current.integer) > 30000;
}

void __cdecl LB_ForceRefresh()
{
    g_lbGlob.feederText[15520 * lb_type->current.integer - 232996] = 1;
}

void __cdecl LB_InitStructure(LbStructure *const structure, int type)
{
    if ( !structure
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp", 56, 0, "%s", "structure") )
    {
        __debugbreak();
    }
    structure->numColums = 0;
    structure->type = type;
    memset(structure->columns, 0, sizeof(structure->columns));
}

void __cdecl LB_RegisterStat(LbStructure *const structure, LbStatEnum statType, bool isRating)
{
    if ( !structure
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp", 66, 0, "%s", "structure") )
    {
        __debugbreak();
    }
    if ( structure->numColums >= 0x11
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                    69,
                    0,
                    "%s",
                    "structure->numColums < numCols") )
    {
        __debugbreak();
    }
    if ( structure->numColums < 0x11 )
    {
        structure->columns[structure->numColums] = statType;
        if ( isRating )
            structure->ratingCol = structure->numColums;
        ++structure->numColums;
    }
}

void __cdecl LB_RegisterStructure(LbLookup *const lookup, LbStructure structure, int type)
{
    if ( !lookup
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp", 88, 0, "%s", "lookup") )
    {
        __debugbreak();
    }
    if ( lookup->numLbs >= 0x10
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                    90,
                    0,
                    "%s",
                    "lookup->numLbs < numLbs") )
    {
        __debugbreak();
    }
    if ( lookup->numLbs < 0x10 )
    {
        memcpy(&lookup->structures[type], &structure, sizeof(lookup->structures[type]));
        ++lookup->numLbs;
    }
}

void __cdecl LB_InitGlobalStructure(LbGlobalStructure *const globalStructures, int lbIndex)
{
    if ( !globalStructures
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                    102,
                    0,
                    "%s",
                    "globalStructures") )
    {
        __debugbreak();
    }
    globalStructures->numColums = 0;
    globalStructures->lbIndex = lbIndex;
    memset(globalStructures->columns, 0, sizeof(globalStructures->columns));
}

void __cdecl LB_RegisterGlobalStat(
                LbGlobalStructure *const globalStructures,
                GlobalLbStatEnum globalStatType,
                bool isRating)
{
    if ( !globalStructures
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                    112,
                    0,
                    "%s",
                    "globalStructures") )
    {
        __debugbreak();
    }
    if ( globalStructures->numColums >= 0x13
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                    114,
                    0,
                    "%s",
                    "globalStructures->numColums < numCols") )
    {
        __debugbreak();
    }
    if ( globalStructures->numColums < 0x13 )
    {
        globalStructures->columns[globalStructures->numColums] = globalStatType;
        if ( isRating )
            globalStructures->ratingCol = globalStructures->numColums;
        ++globalStructures->numColums;
    }
}

void __cdecl LB_RegisterGlobalStructure(LbLookup *const lookup, LbGlobalStructure globalStructures, int lbIndex)
{
    if ( !lookup
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp", 133, 0, "%s", "lookup") )
    {
        __debugbreak();
    }
    if ( lookup->numGlobalLbs >= 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                    135,
                    0,
                    "%s",
                    "lookup->numGlobalLbs < numLbs") )
    {
        __debugbreak();
    }
    if ( lookup->numGlobalLbs < 4 )
    {
        memcpy(&lookup->globalStructures[lbIndex], &globalStructures, sizeof(lookup->globalStructures[lbIndex]));
        ++lookup->numGlobalLbs;
    }
}

LbType __cdecl LB_GetLbIndex(LbType type, LbResetPeriod resetPeriod, bool isPrestigeLb, bool isHiddenLb)
{
    int lbIndex; // [esp+0h] [ebp-4h]

    if ( resetPeriod )
    {
        lbIndex = type + 32 * (resetPeriod - 1) + 16;
        if ( isHiddenLb )
            lbIndex = type + 32 * (resetPeriod - 1) + 32;
    }
    else
    {
        lbIndex = type;
    }
    if ( isPrestigeLb )
        lbIndex += 80;
    return (LbType)lbIndex;
}

char __cdecl LB_GetNumMatchesPlayedForPlayerFromLb(Leaderboard *lb, int controllerIndex)
{
#ifdef KISAK_LIVE_STUBS
    LbResetPeriod Int; // eax
    bool Bool; // [esp-4h] [ebp-1Ch]
    LbType lbIndex; // [esp+8h] [ebp-10h]
    overlappedTask *ov; // [esp+Ch] [ebp-Ch]
    unsigned __int64 userID; // [esp+10h] [ebp-8h] BYREF

    if ( !lb && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp", 186, 0, "%s", "lb") )
        __debugbreak();
    if ( controllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                    187,
                    0,
                    "controllerIndex doesn't index MAX_GPAD_COUNT\n\t%i not in [0, %i)",
                    controllerIndex,
                    1) )
    {
        __debugbreak();
    }
    if ( !lb )
        return 0;
    if ( !lb_typeByResetPeriod->current.integer )
        return 1;
    Bool = Dvar_GetBool("lb_prestige");
    Int = (LbResetPeriod)Dvar_GetInt("lb_typeByResetPeriod");
    lbIndex = LB_GetLbIndex(lb->type, Int, Bool, 1);
    if ( dwGetOnlineUserID(controllerIndex, &userID) )
    {
        ov = TaskManager_GetOpenTaskSlot(g_lbGlob.tasks, controllerIndex, 16);
        if ( !ov
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp", 207, 0, "%s", "ov != 0") )
        {
            __debugbreak();
        }
        if ( ov )
        {
            dwReadStatsByUser(ov, lbViewIds[lbIndex], userID, &lb->playerLbRow);
            ov->data = lb;
            if ( dwGetTaskStatus(ov) )
            {
                Com_PrintError(22, "dwReadStatsByUser() failed.\n");
                TaskManager_ClearTask(ov);
                return 0;
            }
            else
            {
                //bdRemoteTask::setTimeout(ov->overlappedIO.m_ptr, 20.0);
                ov->overlappedIO.m_ptr.setTimeout(20.0);
                return 1;
            }
        }
        else
        {
            return 0;
        }
    }
    else
    {
        Com_PrintError(22, "dwGetOnlineUserID() failed, probably called before NP logon has completed\n");
        return 0;
    }
#else
    return 0;
#endif
}

char __cdecl LB_GetByRank(Leaderboard *lb, int controllerIndex, unsigned int topRank)
{
#ifdef KISAK_LIVE_STUBS
    LbType lbIndex; // [esp+8h] [ebp-8h]
    overlappedTask *ov; // [esp+Ch] [ebp-4h]

    if ( !lb && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp", 229, 0, "%s", "lb") )
        __debugbreak();
    if ( lb->isUpdating
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                    230,
                    0,
                    "%s",
                    "!lb->isUpdating") )
    {
        __debugbreak();
    }
    if ( controllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                    231,
                    0,
                    "controllerIndex doesn't index MAX_GPAD_COUNT\n\t%i not in [0, %i)",
                    controllerIndex,
                    1) )
    {
        __debugbreak();
    }
    ov = TaskManager_GetOpenTaskSlot(g_lbGlob.tasks, controllerIndex, 0);
    if ( !ov
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp", 234, 0, "%s", "ov != 0") )
    {
        __debugbreak();
    }
    if ( !ov )
        return 0;
    lbIndex = LB_GetLbIndex(
                            lb->type,
                            (LbResetPeriod)lb_typeByResetPeriod->current.integer,
                            lb_prestige->current.enabled,
                            0);
    Com_DPrintf(22, "getting starting at rank %i\n", topRank);
    dwReadStatsByRank(
        ov,
        lbViewIds[lbIndex],
        topRank,
        lb_maxrows->current.unsignedInt,
        &lb->userStats.m_leaderBoardEntries[0]);
    ov->data = lb;
    if ( dwGetTaskStatus(ov) )
    {
        Com_PrintError(22, "dwReadStatsByRank() failed.\n");
        TaskManager_ClearTask(ov);
        return 0;
    }
    else
    {
        bdRemoteTask::setTimeout(ov->overlappedIO.m_ptr, 20.0);
        if ( !LB_GetNumMatchesPlayedForPlayerFromLb(lb, controllerIndex) )
            Com_PrintError(22, "Error Reading number of matches played from leaderboard\n");
        return 1;
    }
#else
    return false;
#endif
}

char __cdecl LB_GetByPlayer(Leaderboard *lb, int localClientNum)
{
#ifdef KISAK_LIVE_STUBS
    int ControllerIndex; // eax
    int v4; // eax
    int v5; // eax
    LbType lbIndex; // [esp+8h] [ebp-10h]
    overlappedTask *ov; // [esp+Ch] [ebp-Ch]
    unsigned __int64 userID; // [esp+10h] [ebp-8h] BYREF

    if ( !lb && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp", 266, 0, "%s", "lb") )
        __debugbreak();
    if ( lb->isUpdating
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                    267,
                    0,
                    "%s",
                    "!lb->isUpdating") )
    {
        __debugbreak();
    }
    lbIndex = LB_GetLbIndex(
                            lb->type,
                            (LbResetPeriod)lb_typeByResetPeriod->current.integer,
                            lb_prestige->current.enabled,
                            0);
    ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    if ( dwGetOnlineUserID(ControllerIndex, &userID) )
    {
        v4 = Com_LocalClient_GetControllerIndex(localClientNum);
        ov = TaskManager_GetOpenTaskSlot(g_lbGlob.tasks, v4, 0);
        if ( !ov
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp", 280, 0, "%s", "ov != 0") )
        {
            __debugbreak();
        }
        if ( ov )
        {
            dwReadStatsByPivot(
                ov,
                lbViewIds[lbIndex],
                userID,
                lb_maxrows->current.unsignedInt,
                &lb->userStats.m_leaderBoardEntries[0]);
            ov->data = lb;
            if ( dwGetTaskStatus(ov) )
            {
                Com_PrintError(22, "dwReadStatsByUser() failed.\n");
                TaskManager_ClearTask(ov);
                return 0;
            }
            else
            {
                bdRemoteTask::setTimeout(ov->overlappedIO.m_ptr, 20.0);
                v5 = Com_LocalClient_GetControllerIndex(localClientNum);
                if ( !LB_GetNumMatchesPlayedForPlayerFromLb(lb, v5) )
                    Com_PrintError(22, "Error Reading number of matches played from leaderboard\n");
                return 1;
            }
        }
        else
        {
            return 0;
        }
    }
    else
    {
        Com_PrintError(22, "dwGetOnlineUserID() failed, probably called before NP logon has completed\n");
        return 0;
    }
#else
    return false;
#endif
}

char __cdecl LB_GetStatsByXuids(int controllerIndex, Leaderboard *lb)
{
#ifdef KISAK_LIVE_STUBS
    LbType lbIndex; // [esp+8h] [ebp-8h]
    overlappedTask *ov; // [esp+Ch] [ebp-4h]

    if ( !g_lbGlob.numXuids
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                    308,
                    0,
                    "%s",
                    "g_lbGlob.numXuids > 0") )
    {
        __debugbreak();
    }
    if ( !g_lbGlob.numXuids )
        return 0;
    ov = TaskManager_GetOpenTaskSlot(g_lbGlob.tasks, controllerIndex, 0);
    if ( !ov
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp", 318, 0, "%s", "ov != 0") )
    {
        __debugbreak();
    }
    if ( !ov )
        return 0;
    lbIndex = LB_GetLbIndex(
                            lb->type,
                            (LbResetPeriod)lb_typeByResetPeriod->current.integer,
                            lb_prestige->current.enabled,
                            0);
    dwReadStatsForUsers(ov, lbViewIds[lbIndex], g_lbGlob.xuids, g_lbGlob.numXuids, &lb->userStats.m_leaderBoardEntries[0]);
    ov->data = lb;
    if ( dwGetTaskStatus(ov) )
    {
        Com_PrintError(22, "dwReadStatsForUsers() failed.\n");
        TaskManager_ClearTask(ov);
        return 0;
    }
    else
    {
        bdRemoteTask::setTimeout(ov->overlappedIO.m_ptr, 20.0);
        if ( !LB_GetNumMatchesPlayedForPlayerFromLb(lb, controllerIndex) )
            Com_PrintError(22, "Error Reading number of matches played from leaderboard\n");
        return 1;
    }
#else
    return false;
#endif
}

char __cdecl LB_GetPlayerStats(int controllerIndex, Leaderboard *lb)
{
#ifdef KISAK_LIVE
    unsigned __int64 userID; // [esp+0h] [ebp-8h] BYREF

    g_lbGlob.numXuids = 0;
    if ( dwGetOnlineUserID(controllerIndex, &userID) )
    {
        g_lbGlob.xuids[0] = userID;
        g_lbGlob.numXuids = 1;
        return LB_GetStatsByXuids(controllerIndex, lb);
    }
    else
    {
        Com_PrintError(22, "dwGetOnlineUserID() failed, probably called before NP logon has completed\n");
        return 0;
    }
#else
    return 0;
#endif
}

char __cdecl LB_GetFriends(Leaderboard *lb, int localClientNum)
{
#ifdef KISAK_LIVE
    unsigned int numXuids; // edx
    int i; // [esp+0h] [ebp-D4h]
    FriendInfo info; // [esp+4h] [ebp-D0h] BYREF
    int controllerIndex; // [esp+C4h] [ebp-10h]
    int friendCount; // [esp+C8h] [ebp-Ch]
    unsigned __int64 userID; // [esp+CCh] [ebp-8h] BYREF

    if ( !lb && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp", 404, 0, "%s", "lb") )
        __debugbreak();
    if ( lb->isUpdating
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                    405,
                    0,
                    "%s",
                    "!lb->isUpdating") )
    {
        __debugbreak();
    }
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    friendCount = Friends_GetCount(controllerIndex, 0);
    if ( dwGetOnlineUserID(controllerIndex, &userID) )
        g_lbGlob.xuids[0] = userID;
    g_lbGlob.numXuids = 1;
    for ( i = 0; i < friendCount; ++i )
    {
        if ( Friends_GetByIndex(controllerIndex, 0, i, &info) )
        {
            Com_DPrintf(22, "%s\n", info.name);
            numXuids = g_lbGlob.numXuids;
            LODWORD(g_lbGlob.xuids[g_lbGlob.numXuids]) = info.dwid;
            HIDWORD(g_lbGlob.xuids[numXuids]) = HIDWORD(info.dwid);
            ++g_lbGlob.numXuids;
        }
    }
    return LB_GetStatsByXuids(controllerIndex, lb);
#else
    return false;
#endif
}

char __cdecl LB_CalcOffset(int offset, Leaderboard *lb)
{
    int v3; // [esp+0h] [ebp-14h]
    int v4; // [esp+4h] [ebp-10h]
    int top; // [esp+10h] [ebp-4h]

    if ( !lb && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp", 458, 0, "%s", "lb") )
        __debugbreak();
    if ( lb->filter )
    {
        lb->offset = 0;
        return 0;
    }
    else if ( lb->numResults )
    {
        top = lb->offset;
        if ( offset < lb->numResults + top )
        {
            if ( offset >= top )
            {
                return 0;
            }
            else
            {
                if ( offset - (lb_maxrows->current.integer / 2 - 1) > 0 )
                    v3 = offset - (lb_maxrows->current.integer / 2 - 1);
                else
                    v3 = 0;
                lb->offset = v3;
                return 1;
            }
        }
        else
        {
            if ( offset - (lb_maxrows->current.integer / 2 - 1) > 0 )
                v4 = offset - (lb_maxrows->current.integer / 2 - 1);
            else
                v4 = 0;
            lb->offset = v4;
            return 1;
        }
    }
    else
    {
        return 0;
    }
}

LeaderBoardResult<LeaderBoardRow<10>,100> *__cdecl LB_UpdateLeaderboard(int localClientNum, LbType lbType, int index)
{
    const char *v4; // eax
    int filter; // [esp+0h] [ebp-2Ch]
    bool v6; // [esp+4h] [ebp-28h]
    char v7; // [esp+8h] [ebp-24h]
    bool v8; // [esp+Ch] [ebp-20h]
    Leaderboard *lb; // [esp+10h] [ebp-1Ch]
    bool notSameFilter; // [esp+16h] [ebp-16h]
    int controllerIndex; // [esp+18h] [ebp-14h]
    bool notSameResetPeriod; // [esp+1Fh] [ebp-Dh]
    unsigned int ms; // [esp+24h] [ebp-8h]
    bool notSamePrestigeMode; // [esp+2Ah] [ebp-2h]

    lb = (Leaderboard *)&g_lbGlob.feederText[15520 * lbType - 248320];
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    if ( lb->isUpdating )
        return (LeaderBoardResult<LeaderBoardRow<10>,100> *)&g_lbGlob.feederText[15520 * lbType - 248320];
    ms = Sys_Milliseconds();
    notSameResetPeriod = lb->lbTypeByResetPeriod != lb_typeByResetPeriod->current.integer;
    notSameFilter = lb->filter != lb_filter->current.integer;
    notSamePrestigeMode = lb->prestigeMode != lb_prestige->current.color[0];
    if ( lb_minrefresh->current.integer )
        v8 = (int)(ms - lb->lastUpdate) >= 1000 * lb_minrefresh->current.integer;
    else
        v8 = 0;
    if ( lb->filter == lb_filter->current.integer
        && lb->lbTypeByResetPeriod == lb_typeByResetPeriod->current.integer
        && lb->prestigeMode == lb_prestige->current.color[0] )
    {
        v7 = LB_CalcOffset(index, lb);
    }
    else
    {
        v7 = 0;
    }
    v6 = notSameFilter || v7 || v8 || notSameResetPeriod || notSamePrestigeMode;
    if ( lb->forceRefresh || v6 )
    {
        lb->filter = lb_filter->current.integer;
        lb->lbTypeByResetPeriod = lb_typeByResetPeriod->current.integer;
        lb->prestigeMode = lb_prestige->current.enabled;
        if ( (int)(ms - lb->lastUpdate) < 30000 && !v6 )
        {
            UI_CloseMenu(localClientNum, "popup_gettinglb");
            return (LeaderBoardResult<LeaderBoardRow<10>,100> *)&g_lbGlob.feederText[15520 * lbType - 248320];
        }
        filter = lb->filter;
        if ( filter )
        {
            if ( filter == 1 )
            {
                lb->isUpdating = LB_GetFriends(lb, localClientNum);
            }
            else if ( filter == 2 )
            {
                lb->isUpdating = LB_GetPlayerStats(controllerIndex, lb);
            }
            else
            {
                v4 = va("Unknown filter type: %i\n", *(unsigned int *)&g_lbGlob.feederText[15520 * lbType - 232984]);
                if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp", 541, 0, v4) )
                    __debugbreak();
            }
        }
        else if ( notSameFilter || notSameResetPeriod || notSamePrestigeMode )
        {
            lb->isUpdating = LB_GetByPlayer(lb, localClientNum);
        }
        else
        {
            lb->isUpdating = LB_GetByRank(lb, controllerIndex, lb->offset + 1);
        }
        if ( lb->isUpdating )
        {
            lb->lastUpdate = ms;
            lb->notSameFilter = notSameFilter;
            lb->notSameResetPeriod = notSameResetPeriod;
            lb->notSamePrestigeMode = notSamePrestigeMode;
        }
        else
        {
            UI_CloseMenu(localClientNum, "popup_gettinglb");
        }
    }
    return (LeaderBoardResult<LeaderBoardRow<10>,100> *)&g_lbGlob.feederText[15520 * lbType - 248320];
}

int __cdecl LB_FeederCount(int localClientNum)
{
    Leaderboard *lb; // [esp+0h] [ebp-Ch]
    LbType lbType; // [esp+4h] [ebp-8h]

    lbType = (LbType)lb_type->current.integer;
    Com_LocalClient_GetControllerIndex(localClientNum);
    if ( !LB_NeedToReadLeaderBoard() )
        return 0;
    LB_UpdateLeaderboard(localClientNum, lbType, *(unsigned int *)&g_lbGlob.feederText[15520 * lbType - 233004]);
    lb = (Leaderboard *)&g_lbGlob.feederText[15520 * lbType - 248320];
    if ( lb->isUpdating )
        return 0;
    if ( lb->filter == 1 || lb->filter == 2 )
        return lb->numResults;
    return lb->totalResults;
}

void __cdecl LB_FeederSelection(int index)
{
    *(unsigned int *)&g_lbGlob.feederText[15520 * lb_type->current.integer - 233000] = index;
}

void __cdecl LB_OnSelect(int localClientNum)
{
    LbType lbType; // [esp+4h] [ebp-Ch]
    LeaderBoardRow<10> *row; // [esp+8h] [ebp-8h]

    lbType = (LbType)lb_type->current.integer;
    if ( !g_lbGlob.feederText[15520 * lbType - 232995] )
    {
        row = &LB_UpdateLeaderboard(localClientNum, lbType, *(unsigned int *)&g_lbGlob.feederText[15520 * lbType - 233000])->m_leaderBoardEntries[*(unsigned int *)&g_lbGlob.feederText[15520 * lbType - 233000] - *(unsigned int *)&g_lbGlob.feederText[15520 * lbType - 233004]];
        Live_ShowPlayerProfile(localClientNum, row->m_entityID, row->m_entityName);
    }
}

void __cdecl LB_GetSelectedPlayerInfo(int localClientNum, LBSelectedPlayerInfo *info)
{
    LeaderBoardRow<10> *v2; // eax
    LbType lbType; // [esp+4h] [ebp-Ch]

    lbType = (LbType)lb_type->current.integer;
    if ( !g_lbGlob.feederText[15520 * lbType - 232995] )
    {
        v2 = &LB_UpdateLeaderboard(localClientNum, lbType, *(unsigned int *)&g_lbGlob.feederText[15520 * lbType - 233000])->m_leaderBoardEntries[*(unsigned int *)&g_lbGlob.feederText[15520 * lbType - 233000] - *(unsigned int *)&g_lbGlob.feederText[15520 * lbType - 233004]];
        info->playerXuid = v2->m_entityID;
        I_strncpyz(info->gamerTag, v2->m_entityName, 64);
    }
}

void __cdecl LB_OverrideCursorPos(int viewmax, int *cursorPos, int *startPos)
{
    int v3; // [esp+0h] [ebp-Ch]
    Leaderboard *lb; // [esp+8h] [ebp-4h]

    if ( !cursorPos
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp", 643, 0, "%s", "cursorPos") )
    {
        __debugbreak();
    }
    if ( !startPos
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp", 644, 0, "%s", "startPos") )
    {
        __debugbreak();
    }
    if ( *cursorPos < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                    645,
                    0,
                    "%s",
                    "*cursorPos >= 0") )
    {
        __debugbreak();
    }
    if ( *startPos < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                    646,
                    0,
                    "%s",
                    "*startPos >= 0") )
    {
        __debugbreak();
    }
    lb = (Leaderboard *)&g_lbGlob.feederText[15520 * lb_type->current.integer - 248320];
    if ( !lb->isUpdating )
    {
        *cursorPos = lb->curIdx;
        if ( lb->notSameFilter || lb->notSameResetPeriod || lb->notSamePrestigeMode )
        {
            if ( *cursorPos - viewmax / 2 + 1 > 0 )
                v3 = *cursorPos - viewmax / 2 + 1;
            else
                v3 = 0;
            *startPos = v3;
            lb->notSameFilter = 0;
            lb->notSameResetPeriod = 0;
            lb->notSamePrestigeMode = 0;
        }
        else if ( *startPos <= *cursorPos )
        {
            if ( *startPos <= *cursorPos - viewmax )
                *startPos = *cursorPos - viewmax + 1;
        }
        else
        {
            *startPos = *cursorPos;
        }
        if ( *cursorPos < 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                        673,
                        1,
                        "%s",
                        "*cursorPos >= 0") )
        {
            __debugbreak();
        }
        if ( *startPos < 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                        674,
                        1,
                        "%s",
                        "*startPos >= 0") )
        {
            __debugbreak();
        }
        if ( *cursorPos - *startPos >= (unsigned int)viewmax
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                        675,
                        1,
                        "(*cursorPos - *startPos) doesn't index viewmax\n\t%i not in [0, %i)",
                        *cursorPos - *startPos,
                        viewmax) )
        {
            __debugbreak();
        }
    }
}

int __cdecl LB_GetXUserCol(LbLookup *g_lbLookup, LeaderBoardRow<10> *row, int type, LbStatEnum colId)
{
    unsigned int j; // [esp+0h] [ebp-18h]
    unsigned int i; // [esp+8h] [ebp-10h]
    LbStructure *structure; // [esp+10h] [ebp-8h]
    int value; // [esp+14h] [ebp-4h]

    value = 0;
    if ( !row && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp", 682, 0, "%s", "row") )
        __debugbreak();
    if ( type >= 8 )
        type -= 8;
    structure = 0;
    for ( i = 0; !structure && i < g_lbLookup->numLbs; ++i )
    {
        if ( g_lbLookup->structures[i].type == type )
            structure = &g_lbLookup->structures[i];
    }
    if ( !structure
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp", 700, 0, "%s", "structure") )
    {
        __debugbreak();
    }
    if ( structure && row )
    {
        for ( j = 0; j < structure->numColums; ++j )
        {
            if ( structure->columns[j] == colId )
                return row->m_columns[j];
        }
    }
    return value;
}

void __cdecl LB_GetRankIconForXp(LeaderBoardRow<10> *row, LbType type, Material **handle)
{
    int prestige; // [esp+0h] [ebp-Ch]
    int rank; // [esp+4h] [ebp-8h]
    int xp; // [esp+8h] [ebp-4h]

    if ( !row && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp", 727, 0, "%s", "row") )
        __debugbreak();
    xp = LB_GetXUserCol(&g_LbLookup, row, type, LB_STAT_XP);
    prestige = LB_GetXUserCol(&g_LbLookup, row, type, LB_STAT_PRESTIGE_LEVEL);
    rank = CL_GetRankForXp(xp);
    CL_GetRankIcon(rank, prestige, handle);
}

unsigned int __cdecl LB_CalcPlayerIndex(int localControllerIndex, Leaderboard *lb)
{
#ifdef KISAK_LIVE
    unsigned int numStats; // [esp+8h] [ebp-14h]
    unsigned int i; // [esp+10h] [ebp-Ch]
    unsigned __int64 userID; // [esp+14h] [ebp-8h] BYREF

    i = 0;
    if ( dwGetOnlineUserID(localControllerIndex, &userID) )
    {
        numStats = lb->numResults;
        for ( i = 0; i < numStats && lb->userStats.m_leaderBoardEntries[i].m_entityID != userID; ++i )
            ;
        if ( i == numStats )
            return 0;
    }
    return i;
#else
    return 0;
#endif
}

int __cdecl LB_CompareXUserStatsRowRanks(_QWORD *r1, _QWORD *r2)
{
    if (!r1 && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp", 762, 0, "%s", "r1"))
        __debugbreak();
    if (!r2 && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp", 763, 0, "%s", "r2"))
        __debugbreak();
    if (!r1[4])
        return r2[4] != 0;
    if (!r2[4])
        return -1;
    if (r1[4] >= r2[4])
        return r1[4] > r2[4];
    return -1;
}

void __cdecl LB_SortXUserStatsByRank(Leaderboard *lb)
{
    int count; // [esp+10h] [ebp-4h]

    qsort(
        lb->userStats.m_leaderBoardEntries,
        lb->numResults,
        0x98u,
        (int (__cdecl *)(const void *, const void *))LB_CompareXUserStatsRowRanks);
    for ( count = 0; count < lb->numResults && lb->userStats.m_leaderBoardEntries[count].m_rank; ++count )
        ;
    lb->numResults = count;
}

bool __cdecl LB_MakeRow(
                int controllerIndex,
                const LbPlayerStat *const stats,
                int type,
                int lbIndex,
                LeaderBoardRow<10> *const row,
                bool statWriteTypeAdd)
{
#ifdef KISAK_LIVE
    bool v7; // [esp+4h] [ebp-20h]
    LbStatEnum statType; // [esp+8h] [ebp-1Ch]
    unsigned int j; // [esp+Ch] [ebp-18h]
    unsigned int numColumsInLb; // [esp+10h] [ebp-14h]
    unsigned int i; // [esp+14h] [ebp-10h]
    unsigned int numLbs; // [esp+18h] [ebp-Ch]
    LbStructure *structure; // [esp+1Ch] [ebp-8h]

    if ( !row && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp", 818, 0, "%s", "row") )
        __debugbreak();
    if ( !stats
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp", 819, 0, "%s", "stats") )
    {
        __debugbreak();
    }
    numLbs = g_LbLookup.numLbs;
    structure = 0;
    row->m_writeType = (bdWriteType)statWriteTypeAdd;
    for ( i = 0; !structure && i < numLbs; ++i )
    {
        if ( g_LbLookup.structures[i].type == type )
            structure = &g_LbLookup.structures[i];
    }
    v7 = structure && row;
    if ( v7 )
    {
        numColumsInLb = structure->numColums;
        if ( numColumsInLb > 0xA
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                        844,
                        0,
                        "%s",
                        "numColumsInLb <= row->getNumColumns()") )
        {
            __debugbreak();
        }
        for ( j = 0; j < numColumsInLb; ++j )
        {
            statType = structure->columns[j];
            row->m_columns[j] = stats->values[statType];
            if ( structure->ratingCol == j )
                row->m_rating = stats->values[statType];
        }
        dwGetOnlineUserID(controllerIndex, &row->m_entityID);
        Live_UserGetName(controllerIndex, row->m_entityName, 65);
        row->m_leaderboardID = lbViewIds[lbIndex];
    }
    return v7;
#else
    return 0;
#endif
}

bool __cdecl LB_MakeGlobalRow(
                int controllerIndex,
                const LbPlayerStat *const stats,
                int lbIndex,
                LeaderBoardRow<10> *const row)
{
#ifdef KISAK_LIVE
    bool v5; // [esp+4h] [ebp-20h]
    GlobalLbStatEnum statType; // [esp+8h] [ebp-1Ch]
    unsigned int j; // [esp+Ch] [ebp-18h]
    unsigned int numColumsInLb; // [esp+10h] [ebp-14h]
    unsigned int i; // [esp+14h] [ebp-10h]
    unsigned int numLbs; // [esp+18h] [ebp-Ch]
    LbGlobalStructure *structure; // [esp+1Ch] [ebp-8h]

    if ( !row && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp", 876, 0, "%s", "row") )
        __debugbreak();
    if ( !stats
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp", 877, 0, "%s", "stats") )
    {
        __debugbreak();
    }
    numLbs = g_LbLookup.numGlobalLbs;
    structure = 0;
    row->m_writeType = STAT_WRITE_REPLACE;
    for ( i = 0; !structure && i < numLbs; ++i )
    {
        if ( g_LbLookup.globalStructures[i].lbIndex == lbIndex )
            structure = &g_LbLookup.globalStructures[i];
    }
    v5 = structure && row;
    if ( v5 )
    {
        numColumsInLb = structure->numColums;
        if ( numColumsInLb > 0xA
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                        897,
                        0,
                        "%s",
                        "numColumsInLb <= row->getNumColumns()") )
        {
            __debugbreak();
        }
        for ( j = 0; j < numColumsInLb; ++j )
        {
            statType = structure->columns[j];
            row->m_columns[j] = stats->values[statType];
            if ( j == structure->ratingCol )
                row->m_rating = stats->values[statType];
        }
        dwGetOnlineUserID(controllerIndex, &row->m_entityID);
        Live_UserGetName(controllerIndex, row->m_entityName, 65);
        row->m_leaderboardID = GlobalLbViewIds[lbIndex];
    }
    return v5;
#else
    return 0;
#endif
}

void __cdecl LB_openDownloadingMenuCmd()
{
    UI_OpenMenu(0, "popup_gettinglb");
}

void __cdecl LB_ReportUser()
{
    char *v0; // eax

    Com_DPrintf(14, "Implement me\n");
    v0 = UI_SafeTranslateString("MENU_SENT_CAPS");
    UI_OpenToastPopup(0, "menu_mp_killstreak_select", "", v0, 2700);
}

void __cdecl LB_IncrementEscrow()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    int Primary; // eax
    LeaderBoardRow<10> updateTimeRow; // [esp+18h] [ebp-148h] BYREF
    unsigned __int64 playerXuid; // [esp+B0h] [ebp-B0h]
    LeaderBoardRow<10> balanceRow; // [esp+B8h] [ebp-A8h] BYREF
    bdStatsInfo *statsInfo[2]; // [esp+158h] [ebp-8h] BYREF

    if ( Cmd_Argc() == 3 )
    {
        v1 = Cmd_Argv(1);
        playerXuid = I_atoi64(v1);
        //bdStatsInfo::bdStatsInfo(&balanceRow);
        //balanceRow.__vftable = (LeaderBoardRow<10>_vtbl *)&LeaderBoardRow<10>::`vftable';
        memset(balanceRow.m_columns, 0, sizeof(balanceRow.m_columns));
        //bdStatsInfo::bdStatsInfo(&updateTimeRow);
        //updateTimeRow.__vftable = (LeaderBoardRow<10>_vtbl *)&LeaderBoardRow<10>::`vftable';
        memset(updateTimeRow.m_columns, 0, sizeof(updateTimeRow.m_columns));
        balanceRow.m_entityID = playerXuid;
        balanceRow.m_leaderboardID = 5013;
        balanceRow.m_writeType = STAT_WRITE_ADD;
        balanceRow.m_rating = 0;
        v2 = Cmd_Argv(2);
        balanceRow.m_columns[0] = atoi(v2);
        updateTimeRow.m_entityID = playerXuid;
        updateTimeRow.m_leaderboardID = 5014;
        updateTimeRow.m_writeType = STAT_WRITE_REPLACE;
        updateTimeRow.m_rating = 0;
        updateTimeRow.m_columns[0] = LiveStorage_GetUTC();
        statsInfo[0] = &balanceRow;
        statsInfo[1] = &updateTimeRow;
        Primary = Com_ControllerIndexes_GetPrimary();
        LB_WriteRows(Primary, statsInfo, 2u, 15);
        //bdStatsInfo::~bdStatsInfo(&updateTimeRow);
        //bdStatsInfo::~bdStatsInfo(&balanceRow);
    }
    else
    {
        v0 = va("Usage: incrementescrow( <playerXuid>, <amount> )\n");
        if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp", 963, 0, v0) )
            __debugbreak();
    }
}

char __cdecl LB_SetGametypeDvar()
{
    const dvar_s *gtdvar; // [esp+0h] [ebp-Ch]
    int gameMode; // [esp+4h] [ebp-8h]
    bool hardcoreMode; // [esp+Bh] [ebp-1h]

    gameMode = 0;
    hardcoreMode = Dvar_GetBool("scr_hardcore");
    gtdvar = g_gametype;
    while ( lbTypeEnum_6[gameMode] )
    {
        if ( !_stricmp(lbTypeEnum_6[gameMode], gtdvar->current.string) )
        {
            Dvar_SetInt((dvar_s *)lb_type, gameMode + (hardcoreMode ? 8 : 0));
            return 1;
        }
        ++gameMode;
    }
    Dvar_SetInt((dvar_s *)lb_type, 0);
    return 0;
}

taskCompleteResults __cdecl LB_ReadHiddenLeaderboardComplete(unsigned int slot)
{
#ifdef KISAK_LIVE_STUBS
    int v2; // [esp+0h] [ebp-1A0h]
    taskCompleteResults v3; // [esp+Ch] [ebp-194h]
    unsigned int i; // [esp+10h] [ebp-190h]
    LbType lbType; // [esp+14h] [ebp-18Ch]
    overlappedTask *uploadStatsToLb; // [esp+18h] [ebp-188h]
    int currentLbType; // [esp+1Ch] [ebp-184h]
    const char *gameModePrefix; // [esp+20h] [ebp-180h]
    LeaderBoardRow<10> *rowReadFromHiddenLb; // [esp+24h] [ebp-17Ch]
    int prestige; // [esp+2Ch] [ebp-174h] BYREF
    int totalResults; // [esp+30h] [ebp-170h] BYREF
    taskCompleteResults result; // [esp+34h] [ebp-16Ch]
    int currRow; // [esp+38h] [ebp-168h]
    LbTaskEnum uploadTask; // [esp+3Ch] [ebp-164h]
    int controllerIndex; // [esp+40h] [ebp-160h]
    bool writeStatsToPrestigeLb; // [esp+47h] [ebp-159h]
    int numResults; // [esp+48h] [ebp-158h] BYREF
    int lbIndex; // [esp+4Ch] [ebp-154h]
    LbResetPeriod resetPeriod; // [esp+50h] [ebp-150h]
    int numMatchesPlayed; // [esp+54h] [ebp-14Ch]
    LeaderBoardRow<10> rowToBeWrittenToLb; // [esp+58h] [ebp-148h] BYREF
    int rankXP; // [esp+F4h] [ebp-ACh] BYREF
    LeaderBoardRow<10> rowToBeWrittenToHiddenLb; // [esp+F8h] [ebp-A8h] BYREF
    overlappedTask *readStatsOverlappedIO; // [esp+194h] [ebp-Ch]
    bdStatsInfo *statsInfo[2]; // [esp+198h] [ebp-8h] BYREF

    readStatsOverlappedIO = &g_lbGlob.tasks[slot];
    if ( slot >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                    1085,
                    0,
                    "slot doesn't index MAX_OVERLAPPED_TASKS\n\t%i not in [0, %i)",
                    slot,
                    32) )
    {
        __debugbreak();
    }
    if ( !g_lbGlob.tasks[slot].active
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                    1086,
                    0,
                    "%s",
                    "g_lbGlob.tasks[ slot ].active") )
    {
        __debugbreak();
    }
    if ( !g_lbGlob.tasks[slot].data
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                    1087,
                    0,
                    "%s",
                    "g_lbGlob.tasks[ slot ].data") )
    {
        __debugbreak();
    }
    uploadTask = LB_TASK_UPLOADSTATS_WEEKLY_LB;
    result = dwReadStatsComplete(readStatsOverlappedIO, &numResults, &totalResults);
    controllerIndex = readStatsOverlappedIO->controllerIndex;
    bdStatsInfo::bdStatsInfo(&rowToBeWrittenToHiddenLb);
    rowToBeWrittenToHiddenLb.__vftable = (LeaderBoardRow<10>_vtbl *)&LeaderBoardRow<10>::`vftable';
    memset(rowToBeWrittenToHiddenLb.m_columns, 0, sizeof(rowToBeWrittenToHiddenLb.m_columns));
    bdStatsInfo::bdStatsInfo(&rowToBeWrittenToLb);
    rowToBeWrittenToLb.__vftable = (LeaderBoardRow<10>_vtbl *)&LeaderBoardRow<10>::`vftable';
    memset(rowToBeWrittenToLb.m_columns, 0, sizeof(rowToBeWrittenToLb.m_columns));
    currRow = 0;
    lbIndex = 0;
    rankXP = 0;
    prestige = 0;
    resetPeriod = LB_RESETPERIOD_WEEKLY;
    writeStatsToPrestigeLb = 0;
    if ( result == TASK_COMPLETE )
    {
        rowReadFromHiddenLb = (LeaderBoardRow<10> *)g_lbGlob.tasks[slot].data;
        lbType = Dvar_GetInt("lb_type");
        gameModePrefix = lbTypeEnum_6[lbType];
        currentLbType = lbType;
        if ( lbType >= LB_BASIC_GAMEMODE_COUNT )
            currentLbType = lbType - 8;
        switch ( g_lbGlob.tasks[slot].type )
        {
            case 1:
                resetPeriod = LB_RESETPERIOD_WEEKLY;
                uploadTask = LB_TASK_UPLOADSTATS_WEEKLY_LB;
                lbIndex = LB_GetLbIndex(lbType, LB_RESETPERIOD_WEEKLY, 0, 1);
                break;
            case 2:
                resetPeriod = LB_RESETPERIOD_WEEKLY;
                uploadTask = LB_TASK_UPLOADSTATS_PRESTIGE_WEEKLY_LB;
                writeStatsToPrestigeLb = 1;
                lbIndex = LB_GetLbIndex(lbType, LB_RESETPERIOD_WEEKLY, 1, 1);
                break;
            case 3:
                resetPeriod = LB_RESETPERIOD_MONTHLY;
                uploadTask = LB_TASK_UPLOADSTATS_MONTHLY_LB;
                lbIndex = LB_GetLbIndex(lbType, LB_RESETPERIOD_MONTHLY, 0, 1);
                break;
            case 4:
                resetPeriod = LB_RESETPERIOD_MONTHLY;
                uploadTask = LB_TASK_UPLOADSTATS_PRESTIGE_MONTHLY_LB;
                writeStatsToPrestigeLb = 1;
                lbIndex = LB_GetLbIndex(lbType, LB_RESETPERIOD_MONTHLY, 1, 1);
                break;
            default:
                break;
        }
        LB_SetStatsForRow(controllerIndex, gameModePrefix, 1);
        LB_MakeRow(
            controllerIndex,
            &g_lbGlob.playerStats[controllerIndex],
            currentLbType,
            lbIndex,
            &rowToBeWrittenToHiddenLb,
            1);
        statsInfo[currRow++] = &rowToBeWrittenToHiddenLb;
        if ( numResults > 0 )
        {
            numMatchesPlayed = rowReadFromHiddenLb->m_columns[0] + 1;
            if ( numMatchesPlayed >= lbWriteMinRequirementValues_0[resetPeriod] )
            {
                lbIndex = LB_GetLbIndex(lbType, resetPeriod, writeStatsToPrestigeLb, 0);
                for ( i = 0; i < 0xA; ++i )
                    rowToBeWrittenToLb.m_columns[i] = rowToBeWrittenToHiddenLb.m_columns[i] + rowReadFromHiddenLb->m_columns[i];
                if ( rowToBeWrittenToLb.m_columns[2] / 60 > 1 )
                    v2 = rowToBeWrittenToLb.m_columns[2] / 60;
                else
                    v2 = 1;
                if ( !LiveStats_GetIntPlayerStat(controllerIndex, &rankXP, "RANKXP") )
                    Com_DPrintf(22, "Warning: couldnt read stat RANKXP\n");
                if ( !LiveStats_GetIntPlayerStat(controllerIndex, &prestige, "PLEVEL") )
                    Com_DPrintf(22, "Warning: couldnt read stat PLEVEL\n");
                rowToBeWrittenToLb.m_columns[4] = rankXP;
                rowToBeWrittenToLb.m_columns[5] = prestige;
                rowToBeWrittenToLb.m_columns[3] = (int)(float)((float)((float)rowToBeWrittenToLb.m_columns[1] * 1000.0)
                                                                                                         / (float)v2);
                rowToBeWrittenToLb.m_rating = rowToBeWrittenToLb.m_columns[3];
                rowToBeWrittenToLb.m_entityID = rowToBeWrittenToHiddenLb.m_entityID;
                rowToBeWrittenToLb.m_leaderboardID = lbViewIds[lbIndex];
                rowToBeWrittenToLb.m_writeType = STAT_WRITE_REPLACE;
                Live_UserGetName(controllerIndex, rowToBeWrittenToLb.m_entityName, 65);
                statsInfo[currRow++] = &rowToBeWrittenToLb;
            }
        }
        uploadStatsToLb = TaskManager_GetOpenTaskSlot(g_lbGlob.tasks, controllerIndex, uploadTask);
        dwWriteStats(uploadStatsToLb, statsInfo, currRow);
    }
    if ( result )
        TaskManager_ClearTask(readStatsOverlappedIO);
    v3 = result;
    bdStatsInfo::~bdStatsInfo(&rowToBeWrittenToLb);
    bdStatsInfo::~bdStatsInfo(&rowToBeWrittenToHiddenLb);
    return v3;
#else
    return TASK_NOTCOMPLETE;
#endif
}

void __cdecl LB_SetStatsForRow(int controllerIndex, const char *gameModePrefix, bool setMatchDelta)
{
    int v3; // [esp+0h] [ebp-24h]
    int TotalMatchesPlayedByGameMode; // [esp+4h] [ebp-20h]
    int timePlayed; // [esp+10h] [ebp-14h] BYREF
    int totalScore; // [esp+14h] [ebp-10h] BYREF
    int statValue; // [esp+18h] [ebp-Ch] BYREF
    const char *currentStatName; // [esp+1Ch] [ebp-8h]
    int statId; // [esp+20h] [ebp-4h]

    for ( statId = 0; statId < 17; ++statId )
    {
        currentStatName = lbStatStrings[statId];
        statValue = 0;
        totalScore = 0;
        timePlayed = 0;
        if ( !gameModePrefix || statId == 4 || statId == 5 )
        {
            if ( !setMatchDelta && !LiveStats_GetIntPlayerStat(controllerIndex, &statValue, currentStatName) )
                Com_DPrintf(22, "Warning: unknown mapping %i -> %s, zeroing\n", statId, currentStatName);
        }
        else if ( statId )
        {
            if ( statId == 3 )
            {
                if ( !setMatchDelta )
                {
                    LiveStats_GetIntPlayerStatByGameMode(controllerIndex, &totalScore, gameModePrefix, "SCORE");
                    LiveStats_GetIntPlayerStatByGameMode(controllerIndex, &timePlayed, gameModePrefix, "TIME_PLAYED_TOTAL");
                    if ( timePlayed / 60 > 1 )
                        v3 = timePlayed / 60;
                    else
                        v3 = 1;
                    timePlayed = v3;
                    statValue = (int)(float)((float)((float)totalScore * 1000.0) / (float)v3);
                }
            }
            else if ( setMatchDelta )
            {
                if ( !LiveStats_GetIntPlayerStatMatchDeltaByGameMode(
                                controllerIndex,
                                &statValue,
                                gameModePrefix,
                                currentStatName) )
                    Com_DPrintf(22, "Warning: unknown mapping %i -> %s, zeroing\n", statId, currentStatName);
            }
            else if ( !LiveStats_GetIntPlayerStatByGameMode(controllerIndex, &statValue, gameModePrefix, currentStatName) )
            {
                Com_DPrintf(22, "Warning: unknown mapping %i -> %s, zeroing\n", statId, currentStatName);
            }
        }
        else
        {
            if ( setMatchDelta )
                TotalMatchesPlayedByGameMode = 1;
            else
                TotalMatchesPlayedByGameMode = LiveStats_GetTotalMatchesPlayedByGameMode(controllerIndex, gameModePrefix);
            statValue = TotalMatchesPlayedByGameMode;
        }
        LB_SetStat(controllerIndex, statId, statValue);
    }
}

taskCompleteResults __cdecl LB_ReadLeaderboardMatchesCountComplete(unsigned int slot)
{
#ifdef KISAK_LIVE
    int totalResults; // [esp+4h] [ebp-10h] BYREF
    taskCompleteResults result; // [esp+8h] [ebp-Ch]
    int numResults; // [esp+Ch] [ebp-8h] BYREF
    overlappedTask *readStatsOverlappedIO; // [esp+10h] [ebp-4h]

    readStatsOverlappedIO = &g_lbGlob.tasks[slot];
    if ( slot >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                    1234,
                    0,
                    "slot doesn't index MAX_OVERLAPPED_TASKS\n\t%i not in [0, %i)",
                    slot,
                    32) )
    {
        __debugbreak();
    }
    if ( !g_lbGlob.tasks[slot].active
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                    1235,
                    0,
                    "%s",
                    "g_lbGlob.tasks[ slot ].active") )
    {
        __debugbreak();
    }
    if ( g_lbGlob.tasks[slot].type != 16
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                    1236,
                    0,
                    "%s",
                    "g_lbGlob.tasks[ slot ].type == LB_TASK_READLB_FOR_MATCHES_PLAYED") )
    {
        __debugbreak();
    }
    if ( !g_lbGlob.tasks[slot].data
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                    1237,
                    0,
                    "%s",
                    "g_lbGlob.tasks[ slot ].data") )
    {
        __debugbreak();
    }
    result = dwReadStatsComplete(readStatsOverlappedIO, &numResults, &totalResults);
    if ( result == TASK_COMPLETE )
        *((unsigned int *)g_lbGlob.tasks[slot].data + 3878) = *((unsigned int *)g_lbGlob.tasks[slot].data + 3868);
    if ( result )
        TaskManager_ClearTask(readStatsOverlappedIO);
    return result;
#else
    return TASK_COMPLETE;
#endif
}

taskCompleteResults __cdecl LB_ReadLeaderboardComplete(unsigned int slot)
{
#ifdef KISAK_LIVE
    const char *v1; // eax
    int filter; // [esp+0h] [ebp-1Ch]
    int controllerIndex; // [esp+4h] [ebp-18h]
    Leaderboard *lb; // [esp+Ch] [ebp-10h]
    taskCompleteResults result; // [esp+10h] [ebp-Ch]
    bool isUpdatingBackup; // [esp+17h] [ebp-5h]
    overlappedTask *readStatsOverlappedIO; // [esp+18h] [ebp-4h]

    readStatsOverlappedIO = &g_lbGlob.tasks[slot];
    if ( slot >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                    1274,
                    0,
                    "slot doesn't index MAX_OVERLAPPED_TASKS\n\t%i not in [0, %i)",
                    slot,
                    32) )
    {
        __debugbreak();
    }
    if ( !g_lbGlob.tasks[slot].active
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                    1275,
                    0,
                    "%s",
                    "g_lbGlob.tasks[ slot ].active") )
    {
        __debugbreak();
    }
    if ( g_lbGlob.tasks[slot].type
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                    1276,
                    0,
                    "%s",
                    "g_lbGlob.tasks[ slot ].type == LB_TASK_READLB") )
    {
        __debugbreak();
    }
    if ( !g_lbGlob.tasks[slot].data
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                    1277,
                    0,
                    "%s",
                    "g_lbGlob.tasks[ slot ].data") )
    {
        __debugbreak();
    }
    lb = (Leaderboard *)g_lbGlob.tasks[slot].data;
    isUpdatingBackup = lb->isUpdating;
    lb->isUpdating = 0;
    result = dwReadStatsComplete(readStatsOverlappedIO, &lb->numResults, &lb->totalResults);
    lb->isUpdating = isUpdatingBackup;
    if ( result )
    {
        lb = (Leaderboard *)g_lbGlob.tasks[slot].data;
        lb->isUpdating = 0;
        lb->forceRefresh = 0;
    }
    if ( result == TASK_COMPLETE )
    {
        filter = lb->filter;
        if ( filter )
        {
            if ( filter > 0 && filter <= 2 )
            {
                LB_SortXUserStatsByRank(lb);
                if ( lb->notSameFilter || lb->notSameResetPeriod || lb->notSamePrestigeMode )
                    lb->curIdx = LB_CalcPlayerIndex(g_lbGlob.tasks[slot].controllerIndex, lb);
            }
            else
            {
                v1 = va("Unknown filter type: %i\n", lb->filter);
                if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp", 1319, 0, v1) )
                    __debugbreak();
            }
        }
        else if ( lb->notSameFilter || lb->notSameResetPeriod || lb->notSamePrestigeMode )
        {
            lb->offset = LODWORD(lb->userStats.m_leaderBoardEntries[0].m_rank) - 1;
            lb->curIdx = lb->offset + LB_CalcPlayerIndex(g_lbGlob.tasks[slot].controllerIndex, lb);
        }
    }
    if ( result == TASK_ERROR )
    {
        lb->numResults = 0;
        lb->totalResults = 0;
        if ( readStatsOverlappedIO->finalStatus == bdRemoteTask::bdStatus::BD_TIMED_OUT )
        {
            controllerIndex = readStatsOverlappedIO->controllerIndex;
            TaskManager_ClearTask(readStatsOverlappedIO);
            Live_DemonwareDisconnectCleanup(controllerIndex);
        }
        if ( !lb->filter && (lb->notSameFilter || lb->notSameResetPeriod || lb->notSamePrestigeMode) )
        {
            Com_DPrintf(22, "%s\n", "Player not globally ranked, fetching from the top");
            LB_GetByRank(lb, readStatsOverlappedIO->controllerIndex, 1u);
            lb->curIdx = 0;
            lb->offset = 0;
        }
    }
    if ( result )
        TaskManager_ClearTask(readStatsOverlappedIO);
    return result;
#else
    return TASK_COMPLETE;
#endif
}

void __cdecl LB_ClearPlayerStats(int localControllerIndex)
{
    LbPlayerStat *v1; // ecx

    if ( localControllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                    1367,
                    0,
                    "localControllerIndex doesn't index MAX_GPAD_COUNT\n\t%i not in [0, %i)",
                    localControllerIndex,
                    1) )
    {
        __debugbreak();
    }
    v1 = &g_lbGlob.playerStats[localControllerIndex];
    LODWORD(v1->userID) = 0;
    HIDWORD(v1->userID) = 0;
}

void __cdecl LB_SetStat(int localControllerIndex, int statId, int value)
{
    if ( localControllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                    1376,
                    0,
                    "localControllerIndex doesn't index MAX_GPAD_COUNT\n\t%i not in [0, %i)",
                    localControllerIndex,
                    1) )
    {
        __debugbreak();
    }
    g_lbGlob.playerStats[localControllerIndex].values[statId] = value;
    Com_DPrintf(22, "setting stats (%i) stat %i => %i\n", localControllerIndex, statId, value);
}

void __cdecl LB_UpdateNeeded(int localControllerIndex)
{
#ifdef KISAK_LIVE
    LbPlayerStat *stat; // [esp+0h] [ebp-Ch]
    unsigned __int64 userID; // [esp+4h] [ebp-8h] BYREF

    if ( localControllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                    1388,
                    0,
                    "localControllerIndex doesn't index MAX_GPAD_COUNT\n\t%i not in [0, %i)",
                    localControllerIndex,
                    1) )
    {
        __debugbreak();
    }
    stat = &g_lbGlob.playerStats[localControllerIndex];
    if ( dwGetOnlineUserID(localControllerIndex, &userID) )
    {
        stat->userID = userID;
    }
    else
    {
        LODWORD(stat->userID) = 0;
        HIDWORD(stat->userID) = 0;
        Com_PrintError(22, "dwGetOnlineUserID() failed, probably called before NP logon has completed\n");
    }
#endif
}

void __cdecl LB_UploadPlayerGlobalStats(int localControllerIndex)
{
    int totalCodPointsSpent; // [esp+0h] [ebp-28h] BYREF
    int totalCodPoints; // [esp+4h] [ebp-24h] BYREF
    int toalCodPointsEarned; // [esp+8h] [ebp-20h]
    const char *ops[2]; // [esp+Ch] [ebp-1Ch] BYREF
    int timePlayed; // [esp+14h] [ebp-14h] BYREF
    int totalScore; // [esp+18h] [ebp-10h] BYREF
    int statValue; // [esp+1Ch] [ebp-Ch] BYREF
    const char *currentStatName; // [esp+20h] [ebp-8h]
    int statId; // [esp+24h] [ebp-4h]

    for ( statId = 0; statId < 19; ++statId )
    {
        currentStatName = GlobalLbStatStrings[statId];
        statValue = 0;
        if ( statId )
        {
            if ( statId == 17 || statId == 18 )
            {
                ops[0] = "HighestStats";
                ops[1] = currentStatName;
                statValue = LiveStats_GetDDLIntStat(localControllerIndex, ops, 2);
            }
            else if ( statId == 14 )
            {
                totalCodPoints = 0;
                totalCodPointsSpent = 0;
                toalCodPointsEarned = 0;
                LiveStats_GetIntPlayerStat(localControllerIndex, &totalCodPoints, "CODPOINTS");
                LiveStats_GetIntPlayerStat(localControllerIndex, &totalCodPointsSpent, "CURRENCYSPENT");
                toalCodPointsEarned = totalCodPoints - totalCodPointsSpent;
            }
            else if ( !LiveStats_GetIntPlayerStat(localControllerIndex, &statValue, currentStatName) )
            {
                Com_DPrintf(22, "Warning: unknown mapping %i -> %s, zeroing\n", statId, currentStatName);
            }
        }
        else
        {
            totalScore = 0;
            timePlayed = 0;
            LiveStats_GetIntPlayerStat(localControllerIndex, &totalScore, "SCORE");
            LiveStats_GetIntPlayerStat(localControllerIndex, &timePlayed, "TIME_PLAYED_TOTAL");
            timePlayed /= 60;
            if ( !timePlayed )
                timePlayed = 1;
            statValue = (int)(float)((float)((float)totalScore * 1000.0) / (float)timePlayed);
        }
        LB_SetStat(localControllerIndex, statId, statValue);
    }
}

void __cdecl Lb_ReadHiddenLeaderboardEntry(
                int controllerIndex,
                LeaderBoardRow<10> *userStats,
                int lbIndex,
                LbTaskEnum task)
{
#ifdef KISAK_LIVE
    overlappedTask *ov; // [esp+0h] [ebp-Ch]
    unsigned __int64 userID; // [esp+4h] [ebp-8h] BYREF

    if ( dwGetOnlineUserID(controllerIndex, &userID) )
    {
        ov = TaskManager_GetOpenTaskSlot(g_lbGlob.tasks, controllerIndex, task);
        if ( !ov
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp", 1467, 0, "%s", "ov != 0") )
        {
            __debugbreak();
        }
        dwReadStatsByUser(ov, lbViewIds[lbIndex], userID, userStats);
        ov->data = userStats;
        if ( dwGetTaskStatus(ov) )
        {
            Com_PrintError(22, "dwReadStatsForUsers() failed.\n");
            TaskManager_ClearTask(ov);
        }
    }
    else
    {
        Com_PrintError(22, "dwGetOnlineUserID() failed, probably called before NP logon has completed\n");
    }
#endif
}

void __cdecl Lb_InitiateWeeklyAndMonthlyLbWrite(int controllerIndex, LbType type)
{
    LbType v2; // eax
    LbType v3; // eax
    LbType lbIndex; // [esp+0h] [ebp-4h]
    LbType lbIndexa; // [esp+0h] [ebp-4h]

    v2 = LB_GetLbIndex(type, LB_RESETPERIOD_WEEKLY, 0, 1);
    Lb_ReadHiddenLeaderboardEntry(
        controllerIndex,
        &g_lbGlob.weeklyEntry[controllerIndex],
        v2,
        LB_TASK_READ_WEEKLY_HIDDEN_LB);
    lbIndex = LB_GetLbIndex(type, LB_RESETPERIOD_MONTHLY, 0, 1);
    Lb_ReadHiddenLeaderboardEntry(
        controllerIndex,
        &g_lbGlob.monthlyEntry[controllerIndex],
        lbIndex,
        LB_TASK_READ_MONTHLY_HIDDEN_LB);
    if ( LiveStats_PrestigeLeaderboardsEnabled(controllerIndex) )
    {
        v3 = LB_GetLbIndex(type, LB_RESETPERIOD_WEEKLY, 1, 1);
        Lb_ReadHiddenLeaderboardEntry(
            controllerIndex,
            &g_lbGlob.prestigeWeeklyEntry[controllerIndex],
            v3,
            LB_TASK_READ_PRESTIGE_WEEKLY_HIDDEN_LB);
        lbIndexa = LB_GetLbIndex(type, LB_RESETPERIOD_MONTHLY, 1, 1);
        Lb_ReadHiddenLeaderboardEntry(
            controllerIndex,
            &g_lbGlob.prestigeMonthlyEntry[controllerIndex],
            lbIndexa,
            LB_TASK_READ_PRESTIGE_MONTHLY_HIDDEN_LB);
    }
}

char __cdecl LB_UploadPlayerStats(int localControllerIndex)
{
#ifdef KISAK_LIVE_STUBS
    __int64 v2; // [esp-Ch] [ebp-408h]
    __int64 v3; // [esp-Ch] [ebp-408h]
    int v4; // [esp+0h] [ebp-3FCh]
    bdStatsInfo *k; // [esp+4h] [ebp-3F8h]
    int v6; // [esp+8h] [ebp-3F4h]
    bdStatsInfo *j; // [esp+Ch] [ebp-3F0h]
    int v8; // [esp+10h] [ebp-3ECh]
    bdStatsInfo *m; // [esp+14h] [ebp-3E8h]
    int v10; // [esp+18h] [ebp-3E4h]
    bdStatsInfo *n; // [esp+1Ch] [ebp-3E0h]
    int v12; // [esp+20h] [ebp-3DCh]
    LeaderBoardRow<10> *i; // [esp+24h] [ebp-3D8h]
    int index; // [esp+2Ch] [ebp-3D0h]
    bool writeStatsToLb; // [esp+33h] [ebp-3C9h]
    LbType lbType; // [esp+34h] [ebp-3C8h]
    LbType lbIndex; // [esp+38h] [ebp-3C4h]
    LbType lbIndexa; // [esp+38h] [ebp-3C4h]
    int currentLbType; // [esp+3Ch] [ebp-3C0h]
    const char *gameModePrefix; // [esp+40h] [ebp-3BCh]
    LeaderBoardRow<10> lbRow[6]; // [esp+44h] [ebp-3B8h] BYREF
    int v22; // [esp+3D4h] [ebp-28h] BYREF
    overlappedTask *ov; // [esp+3DCh] [ebp-20h]
    bdStatsInfo *statsInfo[6]; // [esp+3E0h] [ebp-1Ch] BYREF
    unsigned int viewIndex; // [esp+3F8h] [ebp-4h]

    v12 = 6;
    for ( i = lbRow; --v12 >= 0; ++i )
    {
        bdStatsInfo::bdStatsInfo(i);
        i->__vftable = (LeaderBoardRow<10>_vtbl *)&LeaderBoardRow<10>::`vftable';
        i->m_columns[0] = 0;
        i->m_columns[1] = 0;
        i->m_columns[2] = 0;
        i->m_columns[3] = 0;
        i->m_columns[4] = 0;
        i->m_columns[5] = 0;
        i->m_columns[6] = 0;
        i->m_columns[7] = 0;
        i->m_columns[8] = 0;
        i->m_columns[9] = 0;
    }
    if ( Live_IsSignedIn(localControllerIndex) )
    {
        if ( Live_IsInLiveGame() )
        {
            LB_SetGametypeDvar();
            gameModePrefix = lbTypeEnum_6[lb_type->current.integer];
            if ( !gameModePrefix
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                            1534,
                            0,
                            "%s",
                            "gameModePrefix") )
            {
                __debugbreak();
            }
            lbType = lb_type->current.integer;
            currentLbType = lbType;
            if ( lbType >= LB_BASIC_GAMEMODE_COUNT )
                currentLbType = lbType - 8;
            LB_UpdateNeeded(localControllerIndex);
            LB_SetStatsForRow(localControllerIndex, gameModePrefix, 0);
            viewIndex = 0;
            writeStatsToLb = lb_forceLbWrite->current.enabled;
            if ( LiveStats_GetTotalMatchesPlayedByGameMode(localControllerIndex, gameModePrefix) >= lbWriteMinRequirementValues_0[0] )
                writeStatsToLb = 1;
            if ( writeStatsToLb )
            {
                lbIndex = LB_GetLbIndex(lbType, LB_RESETPERIOD_ALL_TIME, 0, 0);
                HIDWORD(v2) = HIDWORD(g_lbGlob.playerStats[localControllerIndex].userID);
                LODWORD(v2) = g_lbGlob.playerStats[localControllerIndex].userID;
                Com_DPrintf(
                    22,
                    "Writing client_num %i with userID %llx to leaderboard %i\n",
                    localControllerIndex,
                    v2,
                    lbViewIds[lbIndex]);
                LB_MakeRow(
                    localControllerIndex,
                    &g_lbGlob.playerStats[localControllerIndex],
                    currentLbType,
                    lbIndex,
                    &lbRow[viewIndex],
                    0);
                statsInfo[viewIndex] = &lbRow[viewIndex];
                ++viewIndex;
                if ( LiveStats_PrestigeLeaderboardsEnabled(localControllerIndex) )
                {
                    lbIndexa = LB_GetLbIndex(lbType, LB_RESETPERIOD_ALL_TIME, 1, 0);
                    HIDWORD(v3) = HIDWORD(g_lbGlob.playerStats[localControllerIndex].userID);
                    LODWORD(v3) = g_lbGlob.playerStats[localControllerIndex].userID;
                    Com_DPrintf(
                        22,
                        "Writing client_num %i with userID %llx to leaderboard %i\n",
                        localControllerIndex,
                        v3,
                        lbViewIds[lbIndexa]);
                    LB_MakeRow(
                        localControllerIndex,
                        &g_lbGlob.playerStats[localControllerIndex],
                        currentLbType,
                        lbIndexa,
                        &lbRow[viewIndex],
                        0);
                    statsInfo[viewIndex] = &lbRow[viewIndex];
                    ++viewIndex;
                }
            }
            Lb_InitiateWeeklyAndMonthlyLbWrite(localControllerIndex, lbType);
            LB_UploadPlayerGlobalStats(localControllerIndex);
            for ( index = 0; index < 4; ++index )
            {
                LB_MakeGlobalRow(localControllerIndex, &g_lbGlob.playerStats[localControllerIndex], index, &lbRow[viewIndex]);
                statsInfo[viewIndex] = &lbRow[viewIndex];
                ++viewIndex;
            }
            ov = TaskManager_GetOpenTaskSlot(g_lbGlob.tasks, localControllerIndex, 7);
            dwWriteStats(ov, statsInfo, viewIndex);
            if ( dwGetTaskStatus(ov) )
            {
                Com_PrintError(22, "dwWriteStats() failed in LB_UploadPlayerStats()\n");
                TaskManager_ClearTask(ov);
                v6 = 6;
                for ( j = (bdStatsInfo *)&v22; --v6 >= 0; bdStatsInfo::~bdStatsInfo(j) )
                    j = (bdStatsInfo *)((char *)j - 152);
                return 0;
            }
            else
            {
                v4 = 6;
                for ( k = (bdStatsInfo *)&v22; --v4 >= 0; bdStatsInfo::~bdStatsInfo(k) )
                    k = (bdStatsInfo *)((char *)k - 152);
                return 1;
            }
        }
        else
        {
            Com_DPrintf(22, "%s\n", "Not in XBL game, not uploading any stats");
            v8 = 6;
            for ( m = (bdStatsInfo *)&v22; --v8 >= 0; bdStatsInfo::~bdStatsInfo(m) )
                m = (bdStatsInfo *)((char *)m - 152);
            return 1;
        }
    }
    else
    {
        Com_DPrintf(22, "local controller index %i not signed in to live\n", localControllerIndex);
        v10 = 6;
        for ( n = (bdStatsInfo *)&v22; --v10 >= 0; bdStatsInfo::~bdStatsInfo(n) )
            n = (bdStatsInfo *)((char *)n - 152);
        return 1;
    }
#else
    return false;
#endif
}

taskCompleteResults __cdecl LB_UploadPlayerStatsComplete(unsigned int slot)
{
#ifdef KISAK_LIVE
    taskCompleteResults result; // [esp+0h] [ebp-8h]
    overlappedTask *uploadStatsOverlappedIO; // [esp+4h] [ebp-4h]

    uploadStatsOverlappedIO = &g_lbGlob.tasks[slot];
    if ( slot >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                    1663,
                    0,
                    "slot doesn't index MAX_OVERLAPPED_TASKS\n\t%i not in [0, %i)",
                    slot,
                    32) )
    {
        __debugbreak();
    }
    if ( !g_lbGlob.tasks[slot].active
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                    1664,
                    0,
                    "%s",
                    "g_lbGlob.tasks[ slot ].active") )
    {
        __debugbreak();
    }
    if ( g_lbGlob.tasks[slot].type != 7
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                    1665,
                    0,
                    "%s",
                    "g_lbGlob.tasks[ slot ].type == LB_TASK_UPLOADSTATS") )
    {
        __debugbreak();
    }
    result = dwWriteStatsComplete(uploadStatsOverlappedIO);
    if ( result )
        TaskManager_ClearTask(uploadStatsOverlappedIO);
    return result;
#else
    return TASK_COMPLETE;
#endif
}

taskCompleteResults __cdecl LB_WriteStatsToLbComplete(unsigned int slot)
{
#ifdef KISAK_LIVE
    taskCompleteResults result; // [esp+0h] [ebp-8h]
    overlappedTask *uploadStatsOverlappedIO; // [esp+4h] [ebp-4h]

    uploadStatsOverlappedIO = &g_lbGlob.tasks[slot];
    if ( slot >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                    1681,
                    0,
                    "slot doesn't index MAX_OVERLAPPED_TASKS\n\t%i not in [0, %i)",
                    slot,
                    32) )
    {
        __debugbreak();
    }
    if ( !g_lbGlob.tasks[slot].active
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                    1682,
                    0,
                    "%s",
                    "g_lbGlob.tasks[ slot ].active") )
    {
        __debugbreak();
    }
    result = dwWriteStatsComplete(uploadStatsOverlappedIO);
    if ( result )
        TaskManager_ClearTask(uploadStatsOverlappedIO);
    return result;
#else
    return TASK_COMPLETE;
#endif
}

taskCompleteResults __cdecl LB_IncrementEscrowComplete(unsigned int slot)
{
#ifdef KISAK_LIVE
    overlappedTask *incrementEscrowOverlappedIO; // [esp+0h] [ebp-8h]
    taskCompleteResults result; // [esp+4h] [ebp-4h]

    incrementEscrowOverlappedIO = &g_lbGlob.tasks[slot];
    if ( slot >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                    1698,
                    0,
                    "slot doesn't index MAX_OVERLAPPED_TASKS\n\t%i not in [0, %i)",
                    slot,
                    32) )
    {
        __debugbreak();
    }
    if ( !g_lbGlob.tasks[slot].active
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                    1699,
                    0,
                    "%s",
                    "g_lbGlob.tasks[ slot ].active") )
    {
        __debugbreak();
    }
    if ( g_lbGlob.tasks[slot].type != 15
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                    1700,
                    0,
                    "%s",
                    "g_lbGlob.tasks[ slot ].type == LB_TASK_INCREMENTESCROW") )
    {
        __debugbreak();
    }
    result = dwWriteStatsComplete(incrementEscrowOverlappedIO);
    if ( result )
        TaskManager_ClearTask(incrementEscrowOverlappedIO);
    return result;
#else
    return TASK_COMPLETE;
#endif
}

taskCompleteResults __cdecl LB_ResolveEscrowComplete(unsigned int slot)
{
#ifdef KISAK_LIVE
    int timeDifference; // [esp+Ch] [ebp-20h]
    int lastUpdatedTime; // [esp+10h] [ebp-1Ch]
    int i; // [esp+14h] [ebp-18h]
    const LeaderBoardResult<LeaderBoardRow<10>,100> *userStats; // [esp+18h] [ebp-14h]
    int totalResults; // [esp+1Ch] [ebp-10h] BYREF
    taskCompleteResults result; // [esp+20h] [ebp-Ch]
    int numResults; // [esp+24h] [ebp-8h] BYREF
    overlappedTask *resolveEscrowOverlappedIO; // [esp+28h] [ebp-4h]

    resolveEscrowOverlappedIO = &g_lbGlob.tasks[slot];
    if ( slot >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                    1717,
                    0,
                    "slot doesn't index MAX_OVERLAPPED_TASKS\n\t%i not in [0, %i)",
                    slot,
                    32) )
    {
        __debugbreak();
    }
    if ( !g_lbGlob.tasks[slot].active
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                    1718,
                    0,
                    "%s",
                    "g_lbGlob.tasks[ slot ].active") )
    {
        __debugbreak();
    }
    if ( g_lbGlob.tasks[slot].type != 12
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                    1719,
                    0,
                    "%s",
                    "g_lbGlob.tasks[ slot ].type == LB_TASK_RESOLVEESCROW") )
    {
        __debugbreak();
    }
    result = dwReadStatsComplete(resolveEscrowOverlappedIO, &numResults, &totalResults);
    if ( result )
    {
        if ( LiveStorage_IsTimeSynced() )
        {
            userStats = (const LeaderBoardResult<LeaderBoardRow<10>,100> *)resolveEscrowOverlappedIO->data;
            for ( i = 0; i < numResults; ++i )
            {
                if ( LODWORD(userStats->m_entityID) == LODWORD(userStats->m_leaderBoardEntries[i].m_entityID)
                    && HIDWORD(userStats->m_entityID) == HIDWORD(userStats->m_leaderBoardEntries[i].m_entityID) )
                {
                    lastUpdatedTime = userStats->m_leaderBoardEntries[i].m_columns[0];
                    timeDifference = LiveStorage_GetUTC() - lastUpdatedTime;
                    if ( lastUpdatedTime > 0 && timeDifference > lb_escrowTimeout->current.integer )
                    {
                        TaskManager_ClearTask(resolveEscrowOverlappedIO);
                        LB_ReadRowForUser(
                            resolveEscrowOverlappedIO->controllerIndex,
                            userStats->m_entityID,
                            0x1395u,
                            13,
                            &g_lbGlob.escrowData[resolveEscrowOverlappedIO->controllerIndex]);
                        return result;
                    }
                    break;
                }
            }
        }
        TaskManager_ClearTask(resolveEscrowOverlappedIO);
    }
    return result;
#else
    return TASK_COMPLETE;
#endif
}

taskCompleteResults __cdecl LB_RetrieveEscrowBalanceComplete(unsigned int slot)
{
#ifdef KISAK_LIVE_STUBS
    taskCompleteResults v2; // [esp+Ch] [ebp-15Ch]
    LeaderBoardRow<10> updateTimeRow; // [esp+10h] [ebp-158h] BYREF
    LeaderBoardRow<10> balanceRow; // [esp+A8h] [ebp-C0h] BYREF
    bdStatsInfo *statsInfo[2]; // [esp+148h] [ebp-20h] BYREF
    int i; // [esp+150h] [ebp-18h]
    const LeaderBoardResult<LeaderBoardRow<10>,100> *userStats; // [esp+154h] [ebp-14h]
    int totalResults; // [esp+158h] [ebp-10h] BYREF
    taskCompleteResults result; // [esp+15Ch] [ebp-Ch]
    int numResults; // [esp+160h] [ebp-8h] BYREF
    overlappedTask *retrieveEscrowBalanceOverlappedIO; // [esp+164h] [ebp-4h]

    retrieveEscrowBalanceOverlappedIO = &g_lbGlob.tasks[slot];
    if ( slot >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                    1756,
                    0,
                    "slot doesn't index MAX_OVERLAPPED_TASKS\n\t%i not in [0, %i)",
                    slot,
                    32) )
    {
        __debugbreak();
    }
    if ( !g_lbGlob.tasks[slot].active
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                    1757,
                    0,
                    "%s",
                    "g_lbGlob.tasks[ slot ].active") )
    {
        __debugbreak();
    }
    if ( g_lbGlob.tasks[slot].type != 13
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                    1758,
                    0,
                    "%s",
                    "g_lbGlob.tasks[ slot ].type == LB_TASK_RETRIEVEESCROWBALANCE") )
    {
        __debugbreak();
    }
    result = dwReadStatsComplete(retrieveEscrowBalanceOverlappedIO, &numResults, &totalResults);
    if ( result )
    {
        userStats = (const LeaderBoardResult<LeaderBoardRow<10>,100> *)retrieveEscrowBalanceOverlappedIO->data;
        for ( i = 0; i < numResults; ++i )
        {
            if ( LODWORD(userStats->m_entityID) == LODWORD(userStats->m_leaderBoardEntries[i].m_entityID)
                && HIDWORD(userStats->m_entityID) == HIDWORD(userStats->m_leaderBoardEntries[i].m_entityID) )
            {
                bdStatsInfo::bdStatsInfo(&balanceRow);
                balanceRow.__vftable = (LeaderBoardRow<10>_vtbl *)&LeaderBoardRow<10>::`vftable';
                memset(balanceRow.m_columns, 0, sizeof(balanceRow.m_columns));
                bdStatsInfo::bdStatsInfo(&updateTimeRow);
                updateTimeRow.__vftable = (LeaderBoardRow<10>_vtbl *)&LeaderBoardRow<10>::`vftable';
                memset(updateTimeRow.m_columns, 0, sizeof(updateTimeRow.m_columns));
                g_lbGlob.escrowBalance[retrieveEscrowBalanceOverlappedIO->controllerIndex] = userStats->m_leaderBoardEntries[i].m_columns[0];
                balanceRow.m_entityID = userStats->m_entityID;
                balanceRow.m_leaderboardID = 5013;
                balanceRow.m_writeType = STAT_WRITE_ADD;
                balanceRow.m_rating = 0;
                balanceRow.m_columns[0] = -g_lbGlob.escrowBalance[retrieveEscrowBalanceOverlappedIO->controllerIndex];
                updateTimeRow.m_entityID = userStats->m_entityID;
                updateTimeRow.m_leaderboardID = 5014;
                updateTimeRow.m_writeType = STAT_WRITE_REPLACE;
                updateTimeRow.m_rating = 0;
                updateTimeRow.m_columns[0] = LiveStorage_GetUTC();
                statsInfo[0] = &balanceRow;
                statsInfo[1] = &updateTimeRow;
                TaskManager_ClearTask(retrieveEscrowBalanceOverlappedIO);
                LB_WriteRows(retrieveEscrowBalanceOverlappedIO->controllerIndex, statsInfo, 2u, 14);
                v2 = result;
                bdStatsInfo::~bdStatsInfo(&updateTimeRow);
                bdStatsInfo::~bdStatsInfo(&balanceRow);
                return v2;
            }
        }
        TaskManager_ClearTask(retrieveEscrowBalanceOverlappedIO);
    }
    return result;
#else
    return TASK_NOTCOMPLETE;
#endif
}

taskCompleteResults __cdecl LB_ClearEscrowComplete(int slot)
{
#ifdef KISAK_LIVE
    int LocalClientNum; // eax
    int escrowBalance; // [esp+0h] [ebp-Ch]
    taskCompleteResults result; // [esp+4h] [ebp-8h]
    overlappedTask *clearEscrowOverlappedIO; // [esp+8h] [ebp-4h]

    clearEscrowOverlappedIO = &g_lbGlob.tasks[slot];
    result = dwWriteStatsComplete(clearEscrowOverlappedIO);
    if ( result )
    {
        escrowBalance = g_lbGlob.escrowBalance[clearEscrowOverlappedIO->controllerIndex];
        if ( escrowBalance > 0 )
        {
            LiveStats_SpendCurrency(clearEscrowOverlappedIO->controllerIndex, -escrowBalance, POINTSSPENT_ESCROW, 0);
            LiveStorage_UploadStatsForController();
            Dvar_SetIntByName("ui_escrowReturn", escrowBalance);
            LocalClientNum = Com_ControllerIndex_GetLocalClientNum(clearEscrowOverlappedIO->controllerIndex);
            UI_OpenMenu(LocalClientNum, "ui_escrow_popup");
        }
        g_lbGlob.escrowBalance[clearEscrowOverlappedIO->controllerIndex] = 0;
        TaskManager_ClearTask(clearEscrowOverlappedIO);
    }
    return result;
#else
    return TASK_COMPLETE;
#endif
}

char *__cdecl LB_FeederItemText(int localClientNum, unsigned int index, int column, Material **material)
{
    char *result; // eax
    int XUserCol; // eax
    const char *RankData; // eax
    double v7; // xmm0_8
    double v8; // xmm0_8
    int v9; // eax
    int v10; // [esp+8h] [ebp-20h]
    int v11; // [esp+10h] [ebp-18h]
    LbType lbType; // [esp+14h] [ebp-14h]
    int rank; // [esp+18h] [ebp-10h]
    LeaderBoardRow<10> *row; // [esp+1Ch] [ebp-Ch]
    LeaderBoardResult<LeaderBoardRow<10>,100> *stats; // [esp+20h] [ebp-8h]

    lbType = (LbType)lb_type->current.integer;
    Com_LocalClient_GetControllerIndex(localClientNum);
    if ( !LB_NeedToReadLeaderBoard() )
        return 0;
    stats = LB_UpdateLeaderboard(localClientNum, lbType, index);
    if ( g_lbGlob.feederText[15520 * lbType - 232995] )
        return 0;
    if ( index >= *(unsigned int *)&g_lbGlob.feederText[15520 * lbType - 232968]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                    1915,
                    0,
                    "index doesn't index g_lbGlob.leaderboards[ lbType ].totalResults\n\t%i not in [0, %i)",
                    index,
                    *(unsigned int *)&g_lbGlob.feederText[15520 * lbType - 232968]) )
    {
        __debugbreak();
    }
    g_lbGlob.feederText[0] = 0;
    if ( (int)(index - *(unsigned int *)&g_lbGlob.feederText[15520 * lbType - 233004]) < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                    1919,
                    0,
                    "%s",
                    "index - g_lbGlob.leaderboards[ lbType ].offset >= 0") )
    {
        __debugbreak();
    }
    row = &stats->m_leaderBoardEntries[index - *(unsigned int *)&g_lbGlob.feederText[15520 * lbType - 233004]];
    switch ( column )
    {
        case 0:
            Com_sprintf(
                g_lbGlob.feederText,
                0x20u,
                "%lld",
                stats->m_leaderBoardEntries[index - *(unsigned int *)&g_lbGlob.feederText[15520 * lbType - 233004]].m_rank);
            result = g_lbGlob.feederText;
            break;
        case 1:
            LB_GetRankIconForXp(row, lbType, material);
            result = (char *)"";
            break;
        case 2:
            XUserCol = LB_GetXUserCol(&g_LbLookup, row, lbType, LB_STAT_XP);
            rank = CL_GetRankForXp(XUserCol);
            RankData = CL_GetRankData(rank, MP_RANKTABLE_DISPLAYLEVEL);
            Com_sprintf(g_lbGlob.feederText, 0x20u, "%s", RankData);
            result = g_lbGlob.feederText;
            break;
        case 3:
            result = stats->m_leaderBoardEntries[index - *(unsigned int *)&g_lbGlob.feederText[15520 * lbType - 233004]].m_entityName;
            break;
        case 4:
            v7 = (float)((float)LB_GetXUserCol(&g_LbLookup, row, lbType, LB_STAT_SCORE_PER_MINUTE) / 1000.0);
            Com_sprintf(g_lbGlob.feederText, 0x20u, "%.2f", v7);
            result = g_lbGlob.feederText;
            break;
        default:
            if ( GlobalLbStat[3][17 * (lbType % 8) + 14 + column] == 8 )
            {
                v11 = LB_GetXUserCol(&g_LbLookup, row, 0, LB_STAT_DEATHS);
                if ( v11 > 1 )
                    v10 = v11;
                else
                    v10 = 1;
                v8 = (float)((float)LB_GetXUserCol(&g_LbLookup, row, 0, LB_STAT_KILLS) / (float)v10);
                Com_sprintf(g_lbGlob.feederText, 0x20u, "%.2f", v8);
            }
            else
            {
                v9 = LB_GetXUserCol(&g_LbLookup, row, lbType, (LbStatEnum)GlobalLbStat[3][17 * (lbType % 8) + 14 + column]);
                Com_sprintf(g_lbGlob.feederText, 0x20u, "%i", v9);
            }
            if ( &g_lbGlob == (LbGlob *)-249720
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                            1946,
                            0,
                            "%s",
                            "g_lbGlob.feederText") )
            {
                __debugbreak();
            }
            result = g_lbGlob.feederText;
            break;
    }
    return result;
}

void __cdecl LB_ReadRowForUser(
                int controllerIndex,
                unsigned __int64 entityID,
                unsigned int leaderboardID,
                int taskType,
                LeaderBoardRow<10> *userStats)
{
#ifdef KISAK_LIVE
    bool statsReadStartSuccessful; // [esp+3h] [ebp-5h]
    overlappedTask *ov; // [esp+4h] [ebp-4h]

    statsReadStartSuccessful = 1;
    LB_EndOngoingTasks();
    if ( controllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                    2039,
                    0,
                    "controllerIndex doesn't index MAX_GPAD_COUNT\n\t%i not in [0, %i)",
                    controllerIndex,
                    1) )
    {
        __debugbreak();
    }
    if ( controllerIndex < 1 && Live_IsUserSignedInToLive() )
    {
        ov = TaskManager_GetOpenTaskSlot(g_lbGlob.tasks, controllerIndex, taskType);
        if ( !ov && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp", 2046, 0, "%s", "ov") )
            __debugbreak();
        userStats->m_entityID = entityID;
        dwReadStatsByUser(ov, leaderboardID, entityID, userStats);
        ov->data = userStats;
        if ( dwGetTaskStatus(ov) )
        {
            Com_PrintError(22, "dwReadStatsByUser() failed in LB_ReadRowForUser()\n");
            TaskManager_ClearTask(ov);
            statsReadStartSuccessful = 0;
        }
        if ( !statsReadStartSuccessful )
            dwConnectionErrorHandler(8);
    }
#endif
}

void __cdecl LB_WriteRows(int controllerIndex, bdStatsInfo **statsInfo, unsigned int numStats, int taskType)
{
#ifdef KISAK_LIVE
    bool statsWriteStartSuccessful; // [esp+3h] [ebp-5h]
    overlappedTask *ov; // [esp+4h] [ebp-4h]

    statsWriteStartSuccessful = 1;
    LB_EndOngoingTasks();
    if ( controllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp",
                    2073,
                    0,
                    "controllerIndex doesn't index MAX_GPAD_COUNT\n\t%i not in [0, %i)",
                    controllerIndex,
                    1) )
    {
        __debugbreak();
    }
    if ( controllerIndex < 1 && Live_IsUserSignedInToLive() )
    {
        ov = TaskManager_GetOpenTaskSlot(g_lbGlob.tasks, controllerIndex, taskType);
        if ( !ov && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp", 2080, 0, "%s", "ov") )
            __debugbreak();
        dwWriteStats(ov, statsInfo, numStats);
        if ( dwGetTaskStatus(ov) )
        {
            Com_PrintError(22, "dwWriteStats() failed in LB_WriteRows()\n");
            TaskManager_ClearTask(ov);
            statsWriteStartSuccessful = 0;
        }
        if ( !statsWriteStartSuccessful )
            dwConnectionErrorHandler(6);
    }
#endif
}

void __cdecl LB_InitAndRegisterStructure(LbStructure *const structure, int type)
{
    LbStructure v2; // [esp-54h] [ebp-60h] BYREF
    int currStat; // [esp+8h] [ebp-4h]

    LB_InitStructure(structure, type);
    LB_RegisterStat(structure, LB_STAT_MATCHES_PLAYED, 0);
    LB_RegisterStat(structure, LB_STAT_SCORE, 0);
    LB_RegisterStat(structure, LB_STAT_TIME_PLAYED_TOTAL, 0);
    LB_RegisterStat(structure, LB_STAT_SCORE_PER_MINUTE, 1);
    LB_RegisterStat(structure, LB_STAT_XP, 0);
    LB_RegisterStat(structure, LB_STAT_PRESTIGE_LEVEL, 0);
    for ( currStat = 0; currStat < lbStatCounts[type]; ++currStat )
        LB_RegisterStat(structure, (LbStatEnum)LbStatPerLbGameMode[type][currStat], 0);
    memcpy(&v2, structure, sizeof(v2));
    LB_RegisterStructure(&g_LbLookup, v2, type);
}

void __cdecl LB_InitAndRegisterStructureForSpecialLeaderboards(
                LbGlobalStructure *const globalStructures,
                int lbIndex,
                int type)
{
    LbGlobalStructure v3; // [esp-5Ch] [ebp-68h] BYREF
    int currStat; // [esp+8h] [ebp-4h]

    LB_InitGlobalStructure(globalStructures, lbIndex);
    LB_RegisterGlobalStat(globalStructures, (GlobalLbStatEnum)GlobalLbStat[type][0], 1);
    for ( currStat = 1; currStat < GlobalLbStatCounts[type]; ++currStat )
        LB_RegisterGlobalStat(globalStructures, (GlobalLbStatEnum)GlobalLbStat[type][currStat], 0);
    memcpy(&v3, globalStructures, sizeof(v3));
    LB_RegisterGlobalStructure(&g_LbLookup, v3, lbIndex);
}

cmd_function_s LB_IncrementEscrow_VAR;
cmd_function_s LB_openDownloadingMenuCmd_VAR;
cmd_function_s LB_ReportUser_VAR;

void __cdecl LB_Init()
{
    int gpad; // [esp+4h] [ebp-BCh]
    LbStructure structure; // [esp+8h] [ebp-B8h] BYREF
    int currIndex; // [esp+5Ch] [ebp-64h]
    int lbIdx; // [esp+60h] [ebp-60h]
    int type; // [esp+64h] [ebp-5Ch]
    LbGlobalStructure globalStructures; // [esp+68h] [ebp-58h] BYREF

    TaskManager_ClearOverlappedTasks(g_lbGlob.tasks);
    memset(&g_LbLookup, 0, sizeof(g_LbLookup));
    for ( type = 0; type < 8; ++type )
        LB_InitAndRegisterStructure(&structure, type);
    type = 0;
    for ( currIndex = 0; currIndex < 4; ++currIndex )
        LB_InitAndRegisterStructureForSpecialLeaderboards(&globalStructures, currIndex, type++);
    Cmd_AddCommandInternal("incrementescrow", LB_IncrementEscrow, &LB_IncrementEscrow_VAR);
    Cmd_AddCommandInternal("downloadingleaderboard", LB_openDownloadingMenuCmd, &LB_openDownloadingMenuCmd_VAR);
    Cmd_AddCommandInternal("reportuser", LB_ReportUser, &LB_ReportUser_VAR);
    for ( gpad = 0; gpad < 1; ++gpad )
        LB_ClearPlayerStats(gpad);
    for ( lbIdx = 0; lbIdx < 16; ++lbIdx )
    {
        *(unsigned int *)&g_lbGlob.feederText[15520 * lbIdx - 232988] = 0;
        *(unsigned int *)&g_lbGlob.feederText[15520 * lbIdx - 232984] = 3;
        *(unsigned int *)&g_lbGlob.feederText[15520 * lbIdx - 232980] = 3;
        *(unsigned int *)&g_lbGlob.feederText[15520 * lbIdx - 233008] = lbIdx;
        *(unsigned int *)&g_lbGlob.feederText[15520 * lbIdx - 233004] = 0;
        *(unsigned int *)&g_lbGlob.feederText[15520 * lbIdx - 233000] = 0;
        g_lbGlob.feederText[15520 * lbIdx - 232996] = 1;
    }
    g_lbGlob.currentLbType = LB_COUNT;
    lb_minrefresh = _Dvar_RegisterInt(
                                        "lb_minrefresh",
                                        0,
                                        0,
                                        30,
                                        0x80u,
                                        "Minimum time (in seconds) between leaderboard fetches");
    lb_filter = _Dvar_RegisterEnum(
                                "lb_filter",
                                lbFilterEnum_6,
                                0,
                                0,
                                "Filter applied to the leaderboard display: ('none','friends')");
    lb_maxrows = _Dvar_RegisterInt("lb_maxrows", 50, 1, 100, 0x80u, "Maximum number of rows to fetch");
    lb_type = _Dvar_RegisterEnum("lb_type", lbTypeEnum_6, 0, 0, "Leaderboard type to display");
    lb_typeByResetPeriod = _Dvar_RegisterEnum(
                                                     "lb_typeByResetPeriod",
                                                     lbResetPeriodEnum_6,
                                                     0,
                                                     0,
                                                     "Reset period for Leaderboards ( 'all_time', 'monthly', weekly )");
    lb_prestige = _Dvar_RegisterBool("lb_prestige", 0, 0, "True when prestige leaderboards have been purchased");
    lb_forceLbWrite = _Dvar_RegisterBool(
                                            "lb_forceLbWrite",
                                            0,
                                            0,
                                            "Set this to to true for writing to the leaderboards without checking for the minimum requirements "
                                            "for different leaderboard types.");
    lb_LastFetchTime = _Dvar_RegisterInt(
                                             "lb_LastFetchTime",
                                             0,
                                             0,
                                             0x7FFFFFFF,
                                             0,
                                             "Time in milliseconds you wait before fetching the leaderboard");
    lb_escrowTimeout = _Dvar_RegisterInt(
                                             "lb_escrowTimeout",
                                             1200,
                                             0,
                                             0x7FFFFFFF,
                                             0,
                                             "Time to wait before funds in escrow are returned to clients");
    lb_escrowRefresh = _Dvar_RegisterInt(
                                             "lb_escrowRefresh",
                                             1200,
                                             0,
                                             0x7FFFFFFF,
                                             0,
                                             "How often to check escrow for unclaimed funds");
}

void __cdecl LB_CheckOngoingTasks()
{
    const char *v0; // eax
    taskCompleteResults LeaderboardComplete; // [esp+4h] [ebp-2Ch]
    taskCompleteResults LeaderboardMatchesCountComplete; // [esp+8h] [ebp-28h]
    int localClientNum; // [esp+28h] [ebp-8h]
    int tasknum; // [esp+2Ch] [ebp-4h]

    for ( tasknum = 0; tasknum < 32; ++tasknum )
    {
        if ( g_lbGlob.tasks[tasknum].active )
        {
            localClientNum = Com_ControllerIndex_GetLocalClientNum(g_lbGlob.tasks[tasknum].controllerIndex);
            switch ( g_lbGlob.tasks[tasknum].type )
            {
                case 0:
                    LeaderboardComplete = LB_ReadLeaderboardComplete(tasknum);
                    if ( LeaderboardComplete )
                    {
                        if ( LeaderboardComplete == TASK_COMPLETE )
                            goto LABEL_28;
                        goto LABEL_26;
                    }
                    break;
                case 1:
                case 2:
                case 3:
                case 4:
                    if ( (unsigned int)LB_ReadHiddenLeaderboardComplete(tasknum) >= TASK_ERROR )
                        Com_Printf(16, "Error reading hidden leaderboard stats\n");
                    break;
                case 7:
                    if ( (unsigned int)LB_UploadPlayerStatsComplete(tasknum) >= TASK_ERROR )
                        Com_Printf(16, "Error uploading leaderboard stats\n");
                    break;
                case 8:
                case 9:
                case 0xA:
                case 0xB:
                    if ( (unsigned int)LB_WriteStatsToLbComplete(tasknum) >= TASK_ERROR )
                        Com_Printf(16, "Error uploading leaderboard stats to leaderboards \n");
                    break;
                case 0xC:
                    if ( (unsigned int)LB_ResolveEscrowComplete(tasknum) >= TASK_ERROR )
                        Com_Printf(16, "Error resolving escrow\n");
                    break;
                case 0xD:
                    if ( (unsigned int)LB_RetrieveEscrowBalanceComplete(tasknum) >= TASK_ERROR )
                        Com_Printf(16, "Error getting escrow balance\n");
                    break;
                case 0xE:
                    if ( (unsigned int)LB_ClearEscrowComplete(tasknum) >= TASK_ERROR )
                        Com_Printf(16, "Error clearing escrow\n");
                    break;
                case 0xF:
                    if ( (unsigned int)LB_IncrementEscrowComplete(tasknum) >= TASK_ERROR )
                        Com_Printf(16, "Error incrementing escrow\n");
                    break;
                case 0x10:
                    LeaderboardMatchesCountComplete = LB_ReadLeaderboardMatchesCountComplete(tasknum);
                    if ( LeaderboardMatchesCountComplete )
                    {
                        if ( LeaderboardMatchesCountComplete == TASK_COMPLETE )
                        {
LABEL_28:
                            UI_CloseMenu(localClientNum, "popup_gettinglb");
                        }
                        else
                        {
LABEL_26:
                            Com_Printf(16, "Error reading leaderboard stats\n");
                            UI_CloseMenu(localClientNum, "popup_gettinglb");
                        }
                    }
                    break;
                default:
                    v0 = va("Unknown live leaderboard task type %i\n", g_lbGlob.tasks[tasknum].type);
                    if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_leaderboard.cpp", 2598, 0, v0) )
                        __debugbreak();
                    break;
            }
        }
    }
}

void __cdecl LB_EndOngoingTasks()
{
    unsigned int start; // [esp+0h] [ebp-4h]

    start = Sys_Milliseconds();
    while ( TaskManager_AnyTaskInProgress(g_lbGlob.tasks) )
    {
        LB_CheckOngoingTasks();
        if ( (int)(Sys_Milliseconds() - start) >= 1000 )
        {
            Com_PrintWarning(22, "Aborting LB_EndOngoingTasks...\n");
            return;
        }
    }
}

//LbGlob *__thiscall LbGlob::LbGlob(LbGlob *this)
//{
//    int v3; // [esp+4h] [ebp-40h]
//    LeaderBoardRow<10> *kk; // [esp+8h] [ebp-3Ch]
//    int v5; // [esp+Ch] [ebp-38h]
//    LeaderBoardRow<10> *jj; // [esp+10h] [ebp-34h]
//    int v7; // [esp+14h] [ebp-30h]
//    LeaderBoardRow<10> *ii; // [esp+18h] [ebp-2Ch]
//    int v9; // [esp+1Ch] [ebp-28h]
//    LeaderBoardRow<10> *n; // [esp+20h] [ebp-24h]
//    int v11; // [esp+24h] [ebp-20h]
//    Leaderboard *m; // [esp+28h] [ebp-1Ch]
//    int v13; // [esp+2Ch] [ebp-18h]
//    LeaderBoardRow<10> *k; // [esp+30h] [ebp-14h]
//    int v15; // [esp+34h] [ebp-10h]
//    LbPlayerStat *j; // [esp+38h] [ebp-Ch]
//    int v17; // [esp+3Ch] [ebp-8h]
//    LbGlob *i; // [esp+40h] [ebp-4h]
//
//    v17 = 32;
//    for ( i = this; --v17 >= 0; i = (LbGlob *)((char *)i + 36) )
//        i->tasks[0].overlappedIO.m_ptr = 0;
//    v15 = 1;
//    for ( j = this->playerStats; --v15 >= 0; ++j )
//    {
//        LODWORD(j->userID) = 0;
//        HIDWORD(j->userID) = 0;
//    }
//    v13 = 1;
//    for ( k = this->escrowData; --v13 >= 0; ++k )
//    {
//        bdStatsInfo::bdStatsInfo(k);
//        k->__vftable = (LeaderBoardRow<10>_vtbl *)&LeaderBoardRow<10>::`vftable';
//        k->m_columns[0] = 0;
//        k->m_columns[1] = 0;
//        k->m_columns[2] = 0;
//        k->m_columns[3] = 0;
//        k->m_columns[4] = 0;
//        k->m_columns[5] = 0;
//        k->m_columns[6] = 0;
//        k->m_columns[7] = 0;
//        k->m_columns[8] = 0;
//        k->m_columns[9] = 0;
//    }
//    v11 = 16;
//    for ( m = this->leaderboards; --v11 >= 0; ++m )
//        Leaderboard::Leaderboard(m);
//    v9 = 1;
//    for ( n = this->weeklyEntry; --v9 >= 0; ++n )
//    {
//        bdStatsInfo::bdStatsInfo(n);
//        n->__vftable = (LeaderBoardRow<10>_vtbl *)&LeaderBoardRow<10>::`vftable';
//        n->m_columns[0] = 0;
//        n->m_columns[1] = 0;
//        n->m_columns[2] = 0;
//        n->m_columns[3] = 0;
//        n->m_columns[4] = 0;
//        n->m_columns[5] = 0;
//        n->m_columns[6] = 0;
//        n->m_columns[7] = 0;
//        n->m_columns[8] = 0;
//        n->m_columns[9] = 0;
//    }
//    v7 = 1;
//    for ( ii = this->prestigeWeeklyEntry; --v7 >= 0; ++ii )
//    {
//        bdStatsInfo::bdStatsInfo(ii);
//        ii->__vftable = (LeaderBoardRow<10>_vtbl *)&LeaderBoardRow<10>::`vftable';
//        ii->m_columns[0] = 0;
//        ii->m_columns[1] = 0;
//        ii->m_columns[2] = 0;
//        ii->m_columns[3] = 0;
//        ii->m_columns[4] = 0;
//        ii->m_columns[5] = 0;
//        ii->m_columns[6] = 0;
//        ii->m_columns[7] = 0;
//        ii->m_columns[8] = 0;
//        ii->m_columns[9] = 0;
//    }
//    v5 = 1;
//    for ( jj = this->monthlyEntry; --v5 >= 0; ++jj )
//    {
//        bdStatsInfo::bdStatsInfo(jj);
//        jj->__vftable = (LeaderBoardRow<10>_vtbl *)&LeaderBoardRow<10>::`vftable';
//        jj->m_columns[0] = 0;
//        jj->m_columns[1] = 0;
//        jj->m_columns[2] = 0;
//        jj->m_columns[3] = 0;
//        jj->m_columns[4] = 0;
//        jj->m_columns[5] = 0;
//        jj->m_columns[6] = 0;
//        jj->m_columns[7] = 0;
//        jj->m_columns[8] = 0;
//        jj->m_columns[9] = 0;
//    }
//    v3 = 1;
//    for ( kk = this->prestigeMonthlyEntry; --v3 >= 0; ++kk )
//    {
//        bdStatsInfo::bdStatsInfo(kk);
//        kk->__vftable = (LeaderBoardRow<10>_vtbl *)&LeaderBoardRow<10>::`vftable';
//        kk->m_columns[0] = 0;
//        kk->m_columns[1] = 0;
//        kk->m_columns[2] = 0;
//        kk->m_columns[3] = 0;
//        kk->m_columns[4] = 0;
//        kk->m_columns[5] = 0;
//        kk->m_columns[6] = 0;
//        kk->m_columns[7] = 0;
//        kk->m_columns[8] = 0;
//        kk->m_columns[9] = 0;
//    }
//    return this;
//}

//Leaderboard *__thiscall Leaderboard::Leaderboard(Leaderboard *this)
//{
//    LeaderBoardResult<LeaderBoardRow<10>,100>::LeaderBoardResult<LeaderBoardRow<10>,100>(&this->userStats);
//    bdStatsInfo::bdStatsInfo(&this->playerLbRow);
//    this->playerLbRow.__vftable = (LeaderBoardRow<10>_vtbl *)&LeaderBoardRow<10>::`vftable';
//    this->playerLbRow.m_columns[0] = 0;
//    this->playerLbRow.m_columns[1] = 0;
//    this->playerLbRow.m_columns[2] = 0;
//    this->playerLbRow.m_columns[3] = 0;
//    this->playerLbRow.m_columns[4] = 0;
//    this->playerLbRow.m_columns[5] = 0;
//    this->playerLbRow.m_columns[6] = 0;
//    this->playerLbRow.m_columns[7] = 0;
//    this->playerLbRow.m_columns[8] = 0;
//    this->playerLbRow.m_columns[9] = 0;
//    return this;
//}
//
//LeaderBoardResult<LeaderBoardRow<10>,100> *__thiscall LeaderBoardResult<LeaderBoardRow<10>,100>::LeaderBoardResult<LeaderBoardRow<10>,100>(
//                LeaderBoardResult<LeaderBoardRow<10>,100> *this)
//{
//    int v3; // [esp+4h] [ebp-8h]
//    LeaderBoardRow<10> *i; // [esp+8h] [ebp-4h]
//
//    bdStatsInfo::bdStatsInfo(this);
//    this->__vftable = (LeaderBoardResult<LeaderBoardRow<10>,100>_vtbl *)&LeaderBoardResult<LeaderBoardRow<10>,100>::`vftable';
//    v3 = 100;
//    for ( i = this->m_leaderBoardEntries; --v3 >= 0; ++i )
//    {
//        bdStatsInfo::bdStatsInfo(i);
//        i->__vftable = (LeaderBoardRow<10>_vtbl *)&LeaderBoardRow<10>::`vftable';
//        i->m_columns[0] = 0;
//        i->m_columns[1] = 0;
//        i->m_columns[2] = 0;
//        i->m_columns[3] = 0;
//        i->m_columns[4] = 0;
//        i->m_columns[5] = 0;
//        i->m_columns[6] = 0;
//        i->m_columns[7] = 0;
//        i->m_columns[8] = 0;
//        i->m_columns[9] = 0;
//    }
//    return this;
//}
//
//void __thiscall LeaderBoardResult<LeaderBoardRow<10>,100>::~LeaderBoardResult<LeaderBoardRow<10>,100>(
//                LeaderBoardResult<LeaderBoardRow<10>,100> *this)
//{
//    int v2; // [esp+4h] [ebp-8h]
//    LeaderBoardResult<LeaderBoardRow<10>,100> *i; // [esp+8h] [ebp-4h]
//
//    v2 = 100;
//    for ( i = this + 1; --v2 >= 0; bdStatsInfo::~bdStatsInfo(i) )
//        i = (LeaderBoardResult<LeaderBoardRow<10>,100> *)((char *)i - 152);
//    bdStatsInfo::~bdStatsInfo(this);
//}
//
//void __thiscall LbGlob::~LbGlob(LbGlob *this)
//{
//    int v2; // [esp+4h] [ebp-44h]
//    bdReference<bdCommonAddr> *jj; // [esp+8h] [ebp-40h]
//    int v4; // [esp+18h] [ebp-30h]
//    bdStatsInfo *ii; // [esp+1Ch] [ebp-2Ch]
//    int v6; // [esp+20h] [ebp-28h]
//    Leaderboard *n; // [esp+24h] [ebp-24h]
//    int v8; // [esp+28h] [ebp-20h]
//    LeaderBoardRow<10> *m; // [esp+2Ch] [ebp-1Ch]
//    int v10; // [esp+30h] [ebp-18h]
//    LeaderBoardRow<10> *k; // [esp+34h] [ebp-14h]
//    int v12; // [esp+38h] [ebp-10h]
//    LeaderBoardRow<10> *j; // [esp+3Ch] [ebp-Ch]
//    int v14; // [esp+40h] [ebp-8h]
//    bdStatsInfo *i; // [esp+44h] [ebp-4h]
//
//    v14 = 1;
//    for ( i = (bdStatsInfo *)&this[1]; --v14 >= 0; bdStatsInfo::~bdStatsInfo(i) )
//        i = (bdStatsInfo *)((char *)i - 152);
//    v12 = 1;
//    for ( j = this->prestigeMonthlyEntry; --v12 >= 0; bdStatsInfo::~bdStatsInfo(j) )
//        --j;
//    v10 = 1;
//    for ( k = this->monthlyEntry; --v10 >= 0; bdStatsInfo::~bdStatsInfo(k) )
//        --k;
//    v8 = 1;
//    for ( m = this->prestigeWeeklyEntry; --v8 >= 0; bdStatsInfo::~bdStatsInfo(m) )
//        --m;
//    v6 = 16;
//    for ( n = (Leaderboard *)this->feederText; --v6 >= 0; Leaderboard::~Leaderboard(n) )
//        --n;
//    v4 = 1;
//    for ( ii = (bdStatsInfo *)this->escrowBalance; --v4 >= 0; bdStatsInfo::~bdStatsInfo(ii) )
//        ii = (bdStatsInfo *)((char *)ii - 152);
//    v2 = 32;
//    for ( jj = (bdReference<bdCommonAddr> *)this->playerStats;
//                --v2 >= 0;
//                bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(jj) )
//    {
//        jj -= 9;
//    }
//}
//
//Leaderboard::~Leaderboard()
//{
//    bdStatsInfo::~bdStatsInfo(&this->playerLbRow);
//    LeaderBoardResult<LeaderBoardRow<10>,100>::~LeaderBoardResult<LeaderBoardRow<10>,100>(&this->userStats);
//}

