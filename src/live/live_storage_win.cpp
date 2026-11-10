#include "live_storage_win.h"

#include <Windows.h>
#include <universal/assertive.h>
#include <qcommon/common.h>
#include <ddl/ddl_api.h>
#include "live_stats.h"
#include <server_mp/sv_main_pc_mp.h>
#include <game_mp/g_main_mp.h>
#include <stringed/stringed_hooks.h>
#include <qcommon/com_gamemodes.h>
#include "live_storage.h"
#include <client_mp/cl_main_pc_mp.h>
#include "live_storage_pub.h"
#include "live_presence_win.h"
#include <ui/ui_playlists.h>
#include <client_mp/sv_client_mp.h>
#include "live_leaderboard.h"
#include <server_mp/sv_main_mp.h>
#include <client/cl_rank.h>
#include <bgame/bg_unlockable_items.h>
#include <ctime>
#include <server/sv_live_stats.h>
#include <DW/dwLogOn_pc.h>
#include <universal/mem_largelocal.h>
#include <DW/dwUtils.h>
#include <win32/win_shared.h>
#include <bgame/bg_emblems.h>
#include "live_contracts.h"
#include <DW/dwMessaging.h>
#include <universal/com_tasks.h>
#include <qcommon/md4.h>
#include <universal/com_files.h>
#include "live_win.h"
#include <win32/win_gamerprofile.h>

int g_svLastPlaylistFetch;
bool s_shouldMapRotate;
bool s_fetchingStats;
sv_cacvalidate_state_t g_cacvalidateState;

const int GlobalLbViewIds_0[4] =
{ 3000, 3001, 3002, 3003 };


const int lbViewIds_0[160] =
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

bool s_firstTimeRunning = true;

const dvar_t *stats_backup;
const dvar_t *presell;
const dvar_t *sv_playlistFetchInterval;

LbPlayerStat g_playerStats[32];
dwFileOperationInfo fileops[32];
char g_statsDir[256];

int __cdecl SystemTimeToInt()
{
    _SYSTEMTIME systemTime; // [esp+0h] [ebp-14h] BYREF

    GetSystemTime(&systemTime);
    return (systemTime.wYear << 16) | (systemTime.wMonth << 8) | systemTime.wDay;
}

void __cdecl LiveStorage_ResetStats(unsigned __int8 *buffer)
{
    unsigned int Time; // eax
    unsigned int v2; // eax

    if ( !buffer
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_storage_win.cpp", 384, 0, "%s", "buffer") )
    {
        __debugbreak();
    }
    if ( !stat_version->current.integer )
        Com_Error(ERR_DROP, "stat_version is zero\n");
    if ( buffer )
    {
        Com_Printf(
            16,
            "LiveStorage_ResetStats: resetstats called - writing statversion %i to buffer\n",
            stat_version->current.integer);
        memset(buffer, 0, 0xA000u);
        DDL_AssociateBuffer((char *)buffer, 40960, g_statsDDL);
        LiveStats_WriteChecksumToBuffer(buffer, 40960);
        LiveStats_SetPlayerStatByKey(
            "PlayerStatsList",
            MP_PLAYERSTATSKEY_STATS_VERSION,
            stat_version->current.unsignedInt,
            buffer);
        Time = SV_GetTime();
        LiveStats_SetPlayerStatByKey("PlayerStatsList", MP_PLAYERSTATSKEY_WEEKLYTIMESTAMP, Time, buffer);
        v2 = SV_GetTime();
        LiveStats_SetPlayerStatByKey("PlayerStatsList", MP_PLAYERSTATSKEY_MONTHLYTIMESTAMP, v2, buffer);
    }
}

static void __cdecl SetDvarFromLocString(int controllerIndex, const char *dvarName, char *preLocalizedText)
{
    char *localizedText; // [esp+0h] [ebp-4h]

    if (!Dvar_IsValidName(dvarName)
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_storage_win.cpp",
            414,
            0,
            "%s",
            "Dvar_IsValidName( dvarName )"))
    {
        __debugbreak();
    }
    localizedText = SEH_LocalizeTextMessage(preLocalizedText, "dvar string", LOCMSG_NOERR);
    if (localizedText && *localizedText)
        Dvar_SetCommand(dvarName, localizedText);
    else
        Dvar_SetCommand(dvarName, preLocalizedText);
}

static void __cdecl ResetCreateAClassNames(int controllerIndex)
{
    SetDvarFromLocString(controllerIndex, "customclass1",   (char*)"CLASS_SLOT1_CAPS");
    SetDvarFromLocString(controllerIndex, "customclass2",   (char*)"CLASS_SLOT2_CAPS");
    SetDvarFromLocString(controllerIndex, "customclass3",   (char*)"CLASS_SLOT3_CAPS");
    SetDvarFromLocString(controllerIndex, "customclass4",   (char*)"CLASS_SLOT4_CAPS");
    SetDvarFromLocString(controllerIndex, "customclass5",   (char*)"CLASS_SLOT5_CAPS");
    SetDvarFromLocString(controllerIndex, "prestigeclass1", (char*)"CLASS_PRESTIGE1");
    SetDvarFromLocString(controllerIndex, "prestigeclass2", (char*)"CLASS_PRESTIGE2");
    SetDvarFromLocString(controllerIndex, "prestigeclass3", (char*)"CLASS_PRESTIGE3");
    SetDvarFromLocString(controllerIndex, "prestigeclass4", (char*)"CLASS_PRESTIGE4");
    SetDvarFromLocString(controllerIndex, "prestigeclass5", (char*)"CLASS_PRESTIGE5");
}

void __cdecl LiveStorage_ReadStats(int __formal, bool validate, bool silent)
{
#ifdef KISAK_LIVE_SERVICE
    if ( live_service->current.enabled )
    {
        LiveStorage_ReadBasicTrainingStats(0, validate, silent);
        LiveStorage_ReadPlayerGlobalBlob();
        LiveStorage_InitCustomClassesNames();
    }
#endif
}

void LiveStorage_InitCustomClassesNames()
{
    int allUnitialized; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    if ( !customclass
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_storage_win.cpp", 448, 0, "%s", "customclass") )
    {
        __debugbreak();
    }
    allUnitialized = 1;
    for ( i = 0; i < 10; ++i )
    {
        if ( !customclass[i]
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_storage_win.cpp",
                        453,
                        0,
                        "%s",
                        "customclass[i]") )
        {
            __debugbreak();
        }
        if ( *(_BYTE *)customclass[i]->current.integer )
            allUnitialized = 0;
    }
    if ( allUnitialized )
        ResetCreateAClassNames(0);
}

void __cdecl LiveStorage_ReadStatsIfDirChanged()
{
    if ( !G_ExitAfterToolComplete() )
    {
        // commented out for no effect
        //if (I_stricmp(g_statsDir, fs_gameDirVar->current.string))
        //{
        //    //BLOPS_NULLSUB();
        //}
    }
}

void __cdecl LiveStorage_UploadStats()
{
    if ( !G_ExitAfterToolComplete() )
    {
        if ( xblive_basictraining->current.enabled )
        {
            LiveStorage_WriteBasicTrainingStats(0);
            LiveStats_MakeStableStatsBuffer(0);
        }
        else
        {
            CL_CACValidateRequest_f();
        }
    }
}

void __cdecl LiveStorage_UploadStatsForController()
{
    LiveStorage_UploadStats();
}

void __cdecl SV_ResetFileOp(dwFileOperationInfo *data)
{
    LiveStorage_ResetFileInfo(data);
}

dwFileOperationInfo *__cdecl SV_GetFreeFileOp()
{
    int i; // [esp+0h] [ebp-8h]
    dwFileOperationInfo *retp; // [esp+4h] [ebp-4h]

    retp = 0;
    for ( i = 0; i < 32; ++i )
    {
        if ( !fileops[i].fileBuffer )
        {
            retp = &fileops[i];
            Com_Printf(0, "returned fileop %i\n", i);
            break;
        }
    }
    if ( !retp
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_storage_win.cpp", 837, 0, "%s", "retp != NULL") )
    {
        __debugbreak();
    }
    return retp;
}

void __cdecl SV_DumpFileOps()
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < 32; ++i )
    {
        if ( fileops[i].fileBuffer )
            Com_Printf(0, "%i\n", i);
    }
}

void __cdecl SV_UseFileOp()
{
    SV_GetFreeFileOp()->fileBuffer = (unsigned __int8 *)1;
}

void __cdecl SV_FreeFileOp()
{
    const char *v0; // eax
    int slot; // [esp+8h] [ebp-4h]

    v0 = Cmd_Argv(1);
    slot = atoi(v0);
    SV_ResetFileOp(&fileops[slot]);
}

void __cdecl CL_GetXP_f()
{
    int xp; // [esp+0h] [ebp-4h] BYREF

    xp = 0;
    LiveStats_GetIntPlayerStatByKey(0, &xp, MP_PLAYERSTATSKEY_RANKXP);
    Com_Printf(14, "clientside xp is %i\n", xp);
}

cmd_function_s CL_GetXP_f_VAR;
cmd_function_s SV_DumpFileOps_VAR;
cmd_function_s SV_UseFileOp_VAR;
cmd_function_s SV_FreeFileOp_VAR;
void __cdecl LiveStorage_Init_Platform()
{
    stats_backup = _Dvar_RegisterBool("stats_backup", 1, 1u, "Backup stats file every time the stats file is saved");
    collectors = _Dvar_RegisterBool("collectors", 0, 0x40u, "Set to true if the player has the collector's edition");
    presell = _Dvar_RegisterBool("presell", 0, 0x40u, "Set to true if the player has preordered");
    primaryWeaponOffset = _Dvar_RegisterInt(
                                                    "primaryWeaponOffset",
                                                    0,
                                                    0,
                                                    7,
                                                    0x40u,
                                                    "Primary Weapon Offset for CE and Presell");
    sv_playlistFetchInterval = _Dvar_RegisterInt(
                                                             "sv_playlistFetchInterval",
                                                             3600,
                                                             600,
                                                             0x7FFFFFFF,
                                                             0,
                                                             "Interval in seconds between playlist fetches");
    g_statsDir[0] = 0;
    Cmd_AddCommandInternal("getxp", CL_GetXP_f, &CL_GetXP_f_VAR);
    Cmd_AddCommandInternal("dumpfs", SV_DumpFileOps, &SV_DumpFileOps_VAR);
    Cmd_AddCommandInternal("usefs", SV_UseFileOp, &SV_UseFileOp_VAR);
    Cmd_AddCommandInternal("freefs", SV_FreeFileOp, &SV_FreeFileOp_VAR);
}

void __cdecl LiveStorage_FetchPlaylistsLocal(int controllerIndex)
{
    if ( Playlist_ReadFromDisk() )
    {
        LiveStorage_SetHavePlaylists(1);
        Live_SetPlaylistVersion(controllerIndex);
        Playlist_ValidatePlaylistNum();
    }
}

void __cdecl LiveStorage_CheckOngoingSessionTasks_Platform()
{
    LiveStorage_HandlePublisherStorageTasks();
}

bool __cdecl SV_GetStatFromBlob(char *buffer, const char *stat, int *outInt)
{
    ddlState_t searchState; // [esp+4h] [ebp-14h] BYREF
    bool retval; // [esp+17h] [ebp-1h]

    if ( !stat
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_storage_win.cpp", 1373, 0, "%s", "stat") )
    {
        __debugbreak();
    }
    if ( !buffer
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_storage_win.cpp", 1374, 0, "%s", "buffer") )
    {
        __debugbreak();
    }
    searchState = *LiveStats_GetRootDDLState();
    retval = 0;
    if ( stat )
    {
        if ( !DDL_MoveToName(&searchState, &searchState, "PlayerStatsList")
            || !DDL_MoveToName(&searchState, &searchState, stat) )
        {
            Com_PrintWarning(15, "getdstat: Could not find member name.");
            return 0;
        }
        if ( searchState.member->type == 2 )
        {
            *outInt = DDL_GetInt(&searchState, buffer);
            return 1;
        }
        else
        {
            Com_PrintWarning(15, "called SV_GetClientDDLStat and got a non-int stat. Ask Ewan");
        }
    }
    else
    {
        Com_Printf(15, "SV_GetClientDDLStat called with null stat, you're doing it wrong. Ask Ewan\n");
    }
    return retval;
}

bool __cdecl SV_GetIntClientStatByGameMode(
                unsigned int clientNum,
                int *outInt,
                const char *gameMode,
                const char *statName)
{
    const ddlState_t *RootDDLState; // eax
    const char *path[3]; // [esp+0h] [ebp-20h] BYREF
    ddlState_t searchState; // [esp+Ch] [ebp-14h] BYREF
    bool retval; // [esp+1Fh] [ebp-1h]

    if ( clientNum >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_storage_win.cpp",
                    1416,
                    0,
                    "%s",
                    "clientNum >= 0 && clientNum < MAX_CLIENTS") )
    {
        __debugbreak();
    }
    if ( (!outInt || !gameMode || !statName)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_storage_win.cpp",
                    1417,
                    0,
                    "%s",
                    "outInt && gameMode && statName") )
    {
        __debugbreak();
    }
    retval = 0;
    if ( (clientNum & 0x80000000) == 0 && outInt && gameMode && statName )
    {
        path[0] = "PlayerStatsByGameMode";
        path[1] = gameMode;
        path[2] = statName;
        RootDDLState = LiveStats_GetRootDDLState();
        if ( !DDL_MoveToPath(RootDDLState, &searchState, 3, path) )
        {
            Com_PrintError(15, "DDL:Error getting player stat value for %s.\n", statName);
            return 0;
        }
        Com_DPrintf(15, "Attempting to get stat %s|%s for clientnum %i\n", path[1], path[2], clientNum);
        *outInt = SV_GetClientDIntStat(clientNum, &searchState);
        return 1;
    }
    return retval;
}

void __cdecl SV_DWInitStats()
{
    int v0; // eax
    int j; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    for (i = 0; i < com_maxclients->current.integer; ++i)
    {
        v0 = i;
        LODWORD(g_playerStats[v0].userID) = 0;
        HIDWORD(g_playerStats[v0].userID) = 0;
        for (j = 0; j < 19; ++j)
            g_playerStats[i].values[j] = 0;
    }
}

bool __cdecl SV_MakeClientLBRow(
                int clientNum,
                const LbPlayerStat *const stats,
                int type,
                int lbIndex,
                LeaderBoardRow<10> *const row,
                bool statWriteTypeAdd)
{
    bool v7; // [esp+4h] [ebp-24h]
    LbStatEnum statType; // [esp+8h] [ebp-20h]
    unsigned int j; // [esp+Ch] [ebp-1Ch]
    unsigned int numColumsInLb; // [esp+10h] [ebp-18h]
    unsigned int i; // [esp+14h] [ebp-14h]
    unsigned int numLbs; // [esp+18h] [ebp-10h]
    LbStructure *structure; // [esp+1Ch] [ebp-Ch]
    client_t *v14; // [esp+20h] [ebp-8h]

    if ( !row && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_storage_win.cpp", 1483, 0, "%s", "row") )
        __debugbreak();
    if ( !stats
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_storage_win.cpp", 1484, 0, "%s", "stats") )
    {
        __debugbreak();
    }
    numLbs = g_LbLookup.numLbs;
    structure = 0;
    row->m_writeType = (bdWriteType)statWriteTypeAdd;
    if ( (clientNum < 0 || clientNum > com_maxclients->current.integer)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_storage_win.cpp",
                    1495,
                    0,
                    "clientNum not in [0, com_maxclients->current.integer]\n\t%i not in [%i, %i]",
                    clientNum,
                    0,
                    com_maxclients->current.integer) )
    {
        __debugbreak();
    }
    v14 = &svs.clients[clientNum];
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
                        "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_storage_win.cpp",
                        1513,
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
        row->m_entityID = v14->dw_userID;
        row->m_leaderboardID = lbViewIds_0[lbIndex];
    }
    return v7;
}

void __cdecl SV_CommitClientLeaderboardsSuccess()
{
#ifdef KISAK_LIVE_STUBS
    if ( !lbRemoteTask.m_ptr
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_storage_win.cpp", 1536, 0, "%s", "lbRemoteTask") )
    {
        __debugbreak();
    }
    if ( lbRemoteTask.m_ptr )
        bdReference<bdCommonAddr>::operator=(&lbRemoteTask, 0);
#endif
}

void __cdecl SV_CommitClientLeaderboardsFailure()
{
#ifdef KISAK_LIVE_STUBS
    if ( !lbRemoteTask.m_ptr
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_storage_win.cpp", 1545, 0, "%s", "lbRemoteTask") )
    {
        __debugbreak();
    }
    if ( lbRemoteTask.m_ptr )
        bdReference<bdCommonAddr>::operator=(&lbRemoteTask, 0);
#endif
}

void __cdecl SV_DoTimedStatsForClient(
                unsigned int clientnum,
                const char *gameModePrefix,
                LeaderBoardRow<10> *weeklyRow,
                int weeklylbIndex,
                LeaderBoardRow<10> *monthlyRow,
                int monthlylBIndex)
{
    int v6; // edx
    const ddlState_t *RootDDLState; // eax
    const ddlState_t *v8; // eax
    const ddlState_t *v9; // eax
    const ddlState_t *v10; // eax
    char *v11; // [esp-4h] [ebp-8Ch]
    char *v12; // [esp-4h] [ebp-8Ch]
    char *v13; // [esp-4h] [ebp-8Ch]
    char *v14; // [esp-4h] [ebp-8Ch]
    int v15; // [esp+0h] [ebp-88h]
    int v16; // [esp+4h] [ebp-84h]
    int k; // [esp+14h] [ebp-74h]
    int j; // [esp+18h] [ebp-70h]
    int i; // [esp+1Ch] [ebp-6Ch]
    day_t writeday; // [esp+20h] [ebp-68h] BYREF
    tm *tm_lastweeklyWrite; // [esp+24h] [ebp-64h]
    day_t currentday; // [esp+28h] [ebp-60h]
    int numDays; // [esp+2Ch] [ebp-5Ch]
    client_t *client; // [esp+30h] [ebp-58h]
    int timePlayed; // [esp+34h] [ebp-54h]
    bool resetweekly; // [esp+3Bh] [ebp-4Dh]
    int prestige; // [esp+3Ch] [ebp-4Ch] BYREF
    int elapsedseconds; // [esp+40h] [ebp-48h]
    bool resetmonthly; // [esp+47h] [ebp-41h]
    ddlState_t localState; // [esp+48h] [ebp-40h] BYREF
    __int64 currentTime; // [esp+58h] [ebp-30h] BYREF
    tm *tm_currentTime; // [esp+64h] [ebp-24h]
    char *stats; // [esp+68h] [ebp-20h]
    int rankXP; // [esp+6Ch] [ebp-1Ch] BYREF
    __int64 lastwriteTime; // [esp+70h] [ebp-18h] BYREF
    int serverUTC; // [esp+78h] [ebp-10h]
    int lastweekly; // [esp+7Ch] [ebp-Ch] BYREF
    tm *tm_lastmonthlyWrite; // [esp+80h] [ebp-8h]
    int lastmonthly; // [esp+84h] [ebp-4h] BYREF

    client = &svs.clients[clientnum];
    if ( (!client || !gameModePrefix)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_storage_win.cpp",
                    1610,
                    0,
                    "%s",
                    "client && gameModePrefix") )
    {
        __debugbreak();
    }
    if ( client )
    {
        if ( client->dw_userID )
        {
            if ( !xblive_basictraining->current.enabled )
            {
                stats = (char *)client->globalStats;
                lastweekly = 0;
                lastmonthly = 0;
                if ( SV_GetStatFromBlob((char *)client->globalStats, "WEEKLY_TIMESTAMP", &lastweekly)
                    && SV_GetStatFromBlob(stats, "MONTHLY_TIMESTAMP", &lastmonthly) )
                {
                    serverUTC = SV_GetTime();
                    resetweekly = 0;
                    resetmonthly = 0;
                    elapsedseconds = serverUTC - lastweekly;
                    currentTime = serverUTC;
                    tm_currentTime = _gmtime64(&currentTime);
                    if ( elapsedseconds >= 604800 )
                    {
                        resetweekly = 1;
                    }
                    else
                    {
                        lastwriteTime = lastweekly;
                        tm_lastweeklyWrite = _gmtime64(&lastwriteTime);
                        numDays = tm_currentTime->tm_wday - tm_lastweeklyWrite->tm_wday;
                        Com_DPrintf(15, "%i days since last weekly write\n", numDays);
                        currentday = (day_t)tm_currentTime->tm_wday;
                        if ( numDays || currentday != MONDAY )
                        {
                            writeday = (day_t)tm_lastweeklyWrite->tm_wday;
                            while ( 1 )
                            {
                                v6 = numDays--;
                                if ( v6 <= 0 )
                                    break;
                                //if ( *operator++(&writeday) == TUESDAY )
                                if (writeday == (TUESDAY) - 1)
                                    resetweekly = 1;
                            }
                        }
                        else if ( tm_currentTime->tm_hour > 11 )
                        {
                            resetweekly = tm_lastweeklyWrite->tm_hour < 11;
                        }
                    }
                    lastwriteTime = lastmonthly;
                    tm_lastmonthlyWrite = _gmtime64(&lastwriteTime);
                    if ( tm_lastmonthlyWrite->tm_mon != tm_currentTime->tm_mon )
                        resetmonthly = 1;
                    if ( resetweekly )
                    {
                        LiveStats_SetPlayerStatByKey(
                            "PlayerStatsList",
                            MP_PLAYERSTATSKEY_WEEKLYTIMESTAMP,
                            serverUTC,
                            client->globalStats);
                        for ( i = 0; i < 10; ++i )
                        {
                            weeklyRow->m_columns[i] = 0;
                            v11 = va("%d", i);
                            RootDDLState = LiveStats_GetRootDDLState();
                            if ( DDL_MoveTo(RootDDLState, &localState, 4, "PlayerStatsByGameMode", gameModePrefix, "lbWeekly", v11) )
                                DDL_SetInt(&localState, 0, stats);
                        }
                    }
                    if ( resetmonthly )
                    {
                        LiveStats_SetPlayerStatByKey(
                            "PlayerStatsList",
                            MP_PLAYERSTATSKEY_MONTHLYTIMESTAMP,
                            serverUTC,
                            client->globalStats);
                        for ( j = 0; j < 10; ++j )
                        {
                            monthlyRow->m_columns[j] = 0;
                            v12 = va("%d", j);
                            v8 = LiveStats_GetRootDDLState();
                            if ( DDL_MoveTo(v8, &localState, 4, "PlayerStatsByGameMode", gameModePrefix, "lbMonthly", v12) )
                                DDL_SetInt(&localState, 0, stats);
                        }
                    }
                    rankXP = 0;
                    prestige = 0;
                    for ( k = 0; k < 10; ++k )
                    {
                        v13 = va("%d", k);
                        v9 = LiveStats_GetRootDDLState();
                        if ( DDL_MoveTo(v9, &localState, 4, "PlayerStatsByGameMode", gameModePrefix, "lbWeekly", v13) )
                        {
                            weeklyRow->m_columns[k] += DDL_GetInt(&localState, stats);
                            DDL_SetInt(&localState, weeklyRow->m_columns[k], stats);
                        }
                        v14 = va("%d", k);
                        v10 = LiveStats_GetRootDDLState();
                        if ( DDL_MoveTo(v10, &localState, 4, "PlayerStatsByGameMode", gameModePrefix, "lbMonthly", v14) )
                        {
                            monthlyRow->m_columns[k] += DDL_GetInt(&localState, stats);
                            DDL_SetInt(&localState, monthlyRow->m_columns[k], stats);
                        }
                    }
                    SV_GetClientDDLStat(clientnum, "RANKXP", &rankXP);
                    SV_GetClientDDLStat(clientnum, "PLEVEL", &prestige);
                    if ( weeklyRow->m_columns[2] / 60 > 1 )
                        v16 = weeklyRow->m_columns[2] / 60;
                    else
                        v16 = 1;
                    timePlayed = v16;
                    weeklyRow->m_columns[4] = rankXP;
                    weeklyRow->m_columns[5] = prestige;
                    weeklyRow->m_columns[3] = (int)(float)((float)((float)weeklyRow->m_columns[1] * 1000.0) / (float)timePlayed);
                    weeklyRow->m_entityID = client->dw_userID;
                    weeklyRow->m_leaderboardID = lbViewIds_0[weeklylbIndex];
                    weeklyRow->m_writeType = STAT_WRITE_REPLACE;
                    weeklyRow->m_rating = weeklyRow->m_columns[3];
                    if ( monthlyRow->m_columns[2] / 60 > 1 )
                        v15 = monthlyRow->m_columns[2] / 60;
                    else
                        v15 = 1;
                    timePlayed = v15;
                    monthlyRow->m_columns[4] = rankXP;
                    monthlyRow->m_columns[5] = prestige;
                    monthlyRow->m_columns[3] = (int)(float)((float)((float)weeklyRow->m_columns[1] * 1000.0) / (float)timePlayed);
                    monthlyRow->m_entityID = client->dw_userID;
                    monthlyRow->m_leaderboardID = lbViewIds_0[monthlylBIndex];
                    monthlyRow->m_writeType = STAT_WRITE_REPLACE;
                    monthlyRow->m_rating = monthlyRow->m_columns[3];
                }
            }
        }
    }
}

bool __cdecl SV_GetClientDDLStat(unsigned int clientnum, const char *stat, int *outInt)
{
    ddlState_t searchState; // [esp+4h] [ebp-14h] BYREF
    bool retval; // [esp+17h] [ebp-1h]

    if ( !stat
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_storage_win.cpp", 1330, 0, "%s", "stat") )
    {
        __debugbreak();
    }
    if ( clientnum > 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_storage_win.cpp",
                    1331,
                    0,
                    "%s",
                    "clientnum >= 0 && clientnum <= MAX_CLIENTS") )
    {
        __debugbreak();
    }
    searchState = *LiveStats_GetRootDDLState();
    retval = 0;
    if ( stat )
    {
        if ( !DDL_MoveToName(&searchState, &searchState, "PlayerStatsList")
            || !DDL_MoveToName(&searchState, &searchState, stat) )
        {
            Com_PrintWarning(15, "getdstat: Could not find member name.");
            return 0;
        }
        if ( searchState.member->type == 2 )
        {
            *outInt = SV_GetClientDIntStat(clientnum, &searchState);
            return 1;
        }
        else
        {
            Com_PrintWarning(15, "called SV_GetClientDDLStat and got a non-int stat. Ask Ewan");
        }
    }
    else
    {
        Com_Printf(15, "SV_GetClientDDLStat called with null stat, you're doing it wrong. Ask Ewan\n");
    }
    return retval;
}

int __cdecl LiveStorage_GetMatchesPlayed(const char *gameModePrefix, int resetPeriod)
{
    const ddlState_t *RootDDLState; // eax
    persistentStats *StatsBuffer; // eax
    char *v5; // [esp-4h] [ebp-28h]
    ddlState_t localState; // [esp+0h] [ebp-24h] BYREF
    int retval; // [esp+10h] [ebp-14h]
    char period[12]; // [esp+14h] [ebp-10h] BYREF

    retval = 0;
    memset(period, 0, 10);
    if ( resetPeriod == 1 )
        I_strncpyz(period, "lbWeekly", 10);
    else
        I_strncpyz(period, "lbMonthly", 10);
    v5 = va("%d", 0);
    RootDDLState = LiveStats_GetRootDDLState();
    if ( DDL_MoveTo(RootDDLState, &localState, 4, "PlayerStatsByGameMode", gameModePrefix, period, v5) )
    {
        StatsBuffer = LiveStorage_GetStatsBuffer(0, STATS_LOCATION_GLOBAL, 1);
        return DDL_GetInt(&localState, (char *)StatsBuffer);
    }
    return retval;
}

char __cdecl SV_GetIntClientStatMatchDeltaByGameMode(
                int clientNum,
                int *outInt,
                const char *gameMode,
                const char *statName)
{
    char *liveStatsBuffer; // [esp+4h] [ebp-10h]
    int valueBeforeMatch; // [esp+Ch] [ebp-8h] BYREF
    int valueAfterMatch; // [esp+10h] [ebp-4h] BYREF

    liveStatsBuffer = (char *)svs.clients[clientNum].globalStats;
    if ( LiveStats_GetIntPlayerStatByGameModeFromBase(
                 0,
                 &valueBeforeMatch,
                 gameMode,
                 statName,
                 (char *)svs.clients[clientNum].globalStatsStable)
        && LiveStats_GetIntPlayerStatByGameModeFromBase(0, &valueAfterMatch, gameMode, statName, liveStatsBuffer) )
    {
        *outInt = valueAfterMatch - valueBeforeMatch;
        return 1;
    }
    else
    {
        DDL_PrintError("DDL:Error getting player stat value for %s.\n", statName);
        return 0;
    }
}

int __cdecl SV_GetTotalMatchesPlayedByGameModeForClient(unsigned int clientNum, const char *gameModePrefix)
{
    int losses; // [esp+0h] [ebp-Ch] BYREF
    int wins; // [esp+4h] [ebp-8h] BYREF
    int ties; // [esp+8h] [ebp-4h] BYREF

    SV_GetIntClientStatByGameMode(clientNum, &wins, gameModePrefix, "WINS");
    SV_GetIntClientStatByGameMode(clientNum, &losses, gameModePrefix, "LOSSES");
    SV_GetIntClientStatByGameMode(clientNum, &ties, gameModePrefix, "TIES");
    return ties + losses + wins;
}

bool __cdecl SV_ArePrestigeLBsEnabledForClient(client_t *client)
{
    int ItemIndexFromName; // eax
    int RankForXp; // [esp-4h] [ebp-Ch]
    int clientXP; // [esp+4h] [ebp-4h] BYREF

    clientXP = 0;
    LiveStats_GetIntPlayerStatFromBase(&clientXP, "RANKXP", (char *)client->globalStats);
    LiveStats_GetIntPlayerStatFromBase(&clientXP, "PLEVEL", (char *)client->globalStats);
    RankForXp = CL_GetRankForXp(clientXP);
    ItemIndexFromName = BG_UnlockablesGetItemIndexFromName("FEATURE_PRESTIGE_LB");
    return SV_CacValidate_IsItemPurchased(client->stats, ItemIndexFromName, 0, RankForXp);
}

void __cdecl SV_SetGlobalLBsForClient(unsigned int clientNum)
{
    const ddlState_t *RootDDLState; // eax
    const char *v2; // [esp-4h] [ebp-38h]
    int totalCodPointsSpent; // [esp+0h] [ebp-34h] BYREF
    int totalCodPoints; // [esp+4h] [ebp-30h] BYREF
    int toalCodPointsEarned; // [esp+8h] [ebp-2Ch]
    ddlState_t searchState; // [esp+Ch] [ebp-28h] BYREF
    int timePlayed; // [esp+1Ch] [ebp-18h] BYREF
    int totalScore; // [esp+20h] [ebp-14h] BYREF
    int statValue; // [esp+24h] [ebp-10h] BYREF
    const char *currentStatName; // [esp+28h] [ebp-Ch]
    int statId; // [esp+2Ch] [ebp-8h]
    char *stats; // [esp+30h] [ebp-4h]

    stats = (char *)svs.clients[clientNum].globalStats;
    for ( statId = 0; statId < 19; ++statId )
    {
        currentStatName = GlobalLbStatStrings[statId];
        statValue = 0;
        if ( statId )
        {
            if ( statId == 17 || statId == 18 )
            {
                v2 = currentStatName;
                RootDDLState = LiveStats_GetRootDDLState();
                if ( DDL_MoveTo(RootDDLState, &searchState, 2, "HighestStats", v2) )
                    statValue = DDL_GetInt(&searchState, stats);
            }
            else if ( statId == 14 )
            {
                totalCodPoints = 0;
                totalCodPointsSpent = 0;
                toalCodPointsEarned = 0;
                SV_GetStatFromBlob(stats, "CODPOINTS", &totalCodPoints);
                SV_GetStatFromBlob(stats, "CURRENCYSPENT", &totalCodPointsSpent);
                toalCodPointsEarned = totalCodPoints - totalCodPointsSpent;
            }
            else if ( !SV_GetStatFromBlob(stats, currentStatName, &statValue) )
            {
                Com_DPrintf(22, "Warning: unknown mapping %i -> %s, zeroing\n", statId, currentStatName);
            }
        }
        else
        {
            totalScore = 0;
            timePlayed = 0;
            SV_GetStatFromBlob(stats, "SCORE", &totalScore);
            SV_GetStatFromBlob(stats, "TIME_PLAYED_TOTAL", &timePlayed);
            timePlayed /= 60;
            if ( !timePlayed )
                timePlayed = 1;
            statValue = (int)(float)((float)((float)totalScore * 1000.0) / (float)timePlayed);
        }
        SV_SetClientLBStat(clientNum, statId, statValue);
    }
}

void __cdecl SV_SetClientLBStat(unsigned int clientNum, int statId, int value)
{
    if ( clientNum >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_storage_win.cpp",
                    1464,
                    0,
                    "%s",
                    "clientNum >= 0 && clientNum < MAX_CLIENTS") )
    {
        __debugbreak();
    }
    g_playerStats[clientNum].values[statId] = value;
    Com_DPrintf(22, "setting stats (%i) stat %i => %i\n", clientNum, statId, value);
}

bool __cdecl SV_MakeClientGlobalLBRow(
                int clientNum,
                const LbPlayerStat *const stats,
                int lbIndex,
                LeaderBoardRow<10> *const row)
{
    bool v5; // [esp+4h] [ebp-24h]
    GlobalLbStatEnum statType; // [esp+8h] [ebp-20h]
    unsigned int j; // [esp+Ch] [ebp-1Ch]
    unsigned int numColumsInLb; // [esp+10h] [ebp-18h]
    unsigned int i; // [esp+14h] [ebp-14h]
    client_t *client; // [esp+18h] [ebp-10h]
    unsigned int numLbs; // [esp+1Ch] [ebp-Ch]
    LbGlobalStructure *structure; // [esp+20h] [ebp-8h]

    if ( !row && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_storage_win.cpp", 1938, 0, "%s", "row") )
        __debugbreak();
    if ( !stats
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_storage_win.cpp", 1939, 0, "%s", "stats") )
    {
        __debugbreak();
    }
    client = &svs.clients[clientNum];
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
                        "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_storage_win.cpp",
                        1961,
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
        row->m_entityID = client->dw_userID;
        row->m_leaderboardID = GlobalLbViewIds_0[lbIndex];
    }
    return v5;
}

void SV_CommitClientLeaderboards()
{
#ifdef KISAK_LIVE_STUBS
    const bdReference<bdCommonAddr> *v1; // eax
    char *v2; // [esp+0h] [ebp-15F8Ch]
    char *v3; // [esp+4h] [ebp-15F88h]
    char *v4; // [esp+8h] [ebp-15F84h]
    int v5; // [esp+Ch] [ebp-15F80h]
    bdStatsInfo *n; // [esp+10h] [ebp-15F7Ch]
    int v7; // [esp+34h] [ebp-15F58h]
    bdStatsInfo *i; // [esp+38h] [ebp-15F54h]
    bdReference<bdCommonAddr> v9; // [esp+3Ch] [ebp-15F50h] BYREF
    signed int m; // [esp+40h] [ebp-15F4Ch]
    signed int k; // [esp+44h] [ebp-15F48h]
    bdStats *Stats; // [esp+48h] [ebp-15F44h]
    int j; // [esp+4Ch] [ebp-15F40h]
    bool v14; // [esp+53h] [ebp-15F39h]
    int weeklylbIndex; // [esp+54h] [ebp-15F38h]
    LeaderBoardRow<10> *row; // [esp+58h] [ebp-15F34h]
    int lbIndex; // [esp+5Ch] [ebp-15F30h]
    int monthlylBIndex; // [esp+60h] [ebp-15F2Ch]
    LeaderBoardRow<10> *monthlyRow; // [esp+64h] [ebp-15F28h]
    int v20; // [esp+68h] [ebp-15F24h]
    int clientNum; // [esp+6Ch] [ebp-15F20h]
    LbType type; // [esp+70h] [ebp-15F1Ch]
    int integer; // [esp+74h] [ebp-15F18h]
    char *gameModePrefix; // [esp+78h] [ebp-15F14h]
    _OWORD v25[5472]; // [esp+7Ch] [ebp-15F10h] BYREF
    int v26; // [esp+1567Ch] [ebp-910h] BYREF
    unsigned int v27[577]; // [esp+15684h] [ebp-908h] BYREF
    signed int v28; // [esp+15F88h] [ebp-4h]

    if ( Com_CanWriteLeaderboards() )
    {
        if ( lbRemoteTask.m_ptr
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_storage_win.cpp",
                        1990,
                        0,
                        "%s",
                        "!lbRemoteTask") )
        {
            __debugbreak();
        }
        if ( lbRemoteTask.m_ptr )
        {
            Com_Printf(0, "Cancelled commmit, commit already in progress.\n");
        }
        else
        {
            if ( !svs.clients
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_storage_win.cpp",
                            1996,
                            0,
                            "%s",
                            "svs.clients") )
            {
                __debugbreak();
            }
            if ( svs.clients )
            {
                v7 = 576;
                for ( i = (bdStatsInfo *)v25; --v7 >= 0; i = (bdStatsInfo *)((char *)i + 152) )
                {
                    bdStatsInfo::bdStatsInfo(i);
                    i->__vftable = (bdStatsInfo_vtbl *)&LeaderBoardRow<10>::`vftable';
                    i[1].__vftable = 0;
                    i[1].m_leaderboardID = 0;
                    LODWORD(i[1].m_entityID) = 0;
                    HIDWORD(i[1].m_entityID) = 0;
                    i[1].m_writeType = STAT_WRITE_REPLACE;
                    *((unsigned int *)&i[1].m_writeType + 1) = 0;
                    LODWORD(i[1].m_rating) = 0;
                    HIDWORD(i[1].m_rating) = 0;
                    LODWORD(i[1].m_rank) = 0;
                    HIDWORD(i[1].m_rank) = 0;
                }
                gameModePrefix = 0;
                LB_SetGametypeDvar();
                gameModePrefix = (char *)lbTypeEnum_9[lb_type->current.integer];
                if ( !gameModePrefix
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_storage_win.cpp",
                                2008,
                                0,
                                "%s",
                                "gameModePrefix") )
                {
                    __debugbreak();
                }
                type = lb_type->current.integer;
                integer = lb_type->current.integer;
                if ( integer >= 8 )
                    integer -= 8;
                v28 = 0;
                for ( clientNum = 0; clientNum < com_maxclients->current.integer; ++clientNum )
                {
                    if ( svs.clients[clientNum].header.state >= CS_CONNECTED
                        && svs.clients[clientNum].header.netchan.remoteAddress.type
                        && svs.clients[clientNum].statsValidated
                        && svs.clients[clientNum].header.sendAsActive )
                    {
                        SV_DWWriteClientStats(&svs.clients[clientNum]);
                        SV_SetClientStatsForRow(clientNum, gameModePrefix, 0);
                        v20 = 0;
                        lbIndex = 0;
                        v14 = SV_GetTotalMatchesPlayedByGameModeForClient(clientNum, gameModePrefix) >= lbWriteMinRequirementValues_1[0];
                        if ( v14 )
                        {
                            lbIndex = LB_GetLbIndex(type, LB_RESETPERIOD_ALL_TIME, 0, 0);
                            SV_MakeClientLBRow(
                                clientNum,
                                &g_playerStats[clientNum],
                                integer,
                                lbIndex,
                                (LeaderBoardRow<10> *const)&v25[171 * clientNum] + v20,
                                0);
                            v4 = (char *)&v25[171 * clientNum] + 152 * v20++;
                            v27[v28++] = v4;
                        }
                        if ( v14 && SV_ArePrestigeLBsEnabledForClient(&svs.clients[clientNum]) )
                        {
                            lbIndex = LB_GetLbIndex(type, LB_RESETPERIOD_ALL_TIME, 1, 0);
                            SV_MakeClientLBRow(
                                clientNum,
                                &g_playerStats[clientNum],
                                integer,
                                lbIndex,
                                (LeaderBoardRow<10> *const)&v25[171 * clientNum] + v20,
                                0);
                            v3 = (char *)&v25[171 * clientNum] + 152 * v20++;
                            v27[v28++] = v3;
                        }
                        weeklylbIndex = LB_GetLbIndex(type, LB_RESETPERIOD_WEEKLY, 0, 0);
                        monthlylBIndex = LB_GetLbIndex(type, LB_RESETPERIOD_MONTHLY, 0, 0);
                        SV_SetClientStatsForRow(clientNum, gameModePrefix, 1);
                        row = (LeaderBoardRow<10> *)((char *)&v25[171 * clientNum] + 152 * v20++);
                        monthlyRow = (LeaderBoardRow<10> *)((char *)&v25[171 * clientNum] + 152 * v20++);
                        SV_MakeClientLBRow(clientNum, &g_playerStats[clientNum], integer, weeklylbIndex, row, 1);
                        SV_MakeClientLBRow(clientNum, &g_playerStats[clientNum], integer, monthlylBIndex, monthlyRow, 1);
                        SV_DoTimedStatsForClient(clientNum, gameModePrefix, row, weeklylbIndex, monthlyRow, monthlylBIndex);
                        if ( SV_GetTotalMatchesPlayedByGameModeForClient(clientNum, gameModePrefix) >= lbWriteMinRequirementValues_1[1] )
                            v27[v28++] = row;
                        if ( SV_GetTotalMatchesPlayedByGameModeForClient(clientNum, gameModePrefix) >= lbWriteMinRequirementValues_1[2] )
                            v27[v28++] = monthlyRow;
                        SV_SetGlobalLBsForClient(clientNum);
                        for ( j = 0; j < 4; ++j )
                        {
                            SV_MakeClientGlobalLBRow(
                                clientNum,
                                &g_playerStats[clientNum],
                                j,
                                (LeaderBoardRow<10> *const)&v25[171 * clientNum] + v20);
                            v2 = (char *)&v25[171 * clientNum] + 152 * v20++;
                            v27[v28++] = v2;
                        }
                    }
                }
                if ( v28 )
                {
                    Com_DPrintf(22, "DirtyLBnum\tLeaderboardID\tEntityID\n");
                    for ( k = 0; k < v28; ++k )
                        Com_DPrintf(22, "%i\t%u\t%llu\n", k, *(unsigned int *)(v27[k] + 4), *(_QWORD *)(v27[k] + 8));
                    Com_DPrintf(15, "Starting write for %i leaderboards\n", v28);
                    Stats = dwGetStats(0);
                    if ( Stats )
                    {
                        Com_DPrintf(22, "Writing leaderboards: ");
                        for ( m = 0; m < v28; ++m )
                            Com_DPrintf(22, "%u ", *(unsigned int *)(v27[m] + 4));
                        Com_DPrintf(22, "\n");
                        dwEnterDeferredCritsec();
                        v1 = (const bdReference<bdCommonAddr> *)bdStats::writeStats(Stats, (int)&v9, (int)v27, v28);
                        bdReference<bdCommonAddr>::operator=((bdReference<bdCommonAddr> *)&lbRemoteTask, v1);
                        bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v9);
                        dwLeaveDeferredCritsec();
                        if ( lbRemoteTask.m_ptr )
                        {
                            if ( bdRemoteTask::getStatus(lbRemoteTask.m_ptr) == BD_PENDING )
                                TaskManager2_DeferTaskToMainThread(lbRemoteTask.m_ptr, task_writeclientleaderboards, 0);
                        }
                    }
                }
                v5 = 576;
                for ( n = (bdStatsInfo *)&v26; --v5 >= 0; bdStatsInfo::~bdStatsInfo(n) )
                    n = (bdStatsInfo *)((char *)n - 152);
            }
        }
    }
#endif
}

void __cdecl SV_SetClientStatsForRow(int clientNum, const char *gameModePrefix, bool delta)
{
#ifdef KISAK_LIVE_STUBS
    int v3; // [esp+0h] [ebp-24h]
    int TotalMatchesPlayedByGameModeForClient; // [esp+4h] [ebp-20h]
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
            if ( !SV_GetClientDDLStat(clientNum, currentStatName, &statValue) )
                Com_DPrintf(22, "Warning: unknown mapping %i -> %s, zeroing\n", statId, currentStatName);
        }
        else if ( statId )
        {
            if ( statId == 3 )
            {
                if ( !delta )
                {
                    SV_GetIntClientStatByGameMode(clientNum, &totalScore, gameModePrefix, "SCORE");
                    SV_GetIntClientStatByGameMode(clientNum, &timePlayed, gameModePrefix, "TIME_PLAYED_TOTAL");
                    if ( timePlayed / 60 > 1 )
                        v3 = timePlayed / 60;
                    else
                        v3 = 1;
                    timePlayed = v3;
                    statValue = (int)(float)((float)((float)totalScore * 1000.0) / (float)v3);
                }
            }
            else if ( delta )
            {
                if ( !SV_GetIntClientStatMatchDeltaByGameMode(clientNum, &statValue, gameModePrefix, currentStatName) )
                    Com_DPrintf(22, "Warning: unknown mapping %i -> %s, zeroing\n", statId, currentStatName);
            }
            else if ( !SV_GetIntClientStatMatchDeltaByGameMode(clientNum, &statValue, gameModePrefix, currentStatName) )
            {
                Com_DPrintf(22, "Warning: unknown mapping %i -> %s, zeroing\n", statId, currentStatName);
            }
        }
        else
        {
            if ( delta )
                TotalMatchesPlayedByGameModeForClient = 1;
            else
                TotalMatchesPlayedByGameModeForClient = SV_GetTotalMatchesPlayedByGameModeForClient(clientNum, gameModePrefix);
            statValue = TotalMatchesPlayedByGameModeForClient;
        }
        SV_SetClientLBStat(clientNum, statId, statValue);
    }
#endif
}

void __cdecl SV_ReadClientFileSuccess(TaskRecord *task)
{
#ifdef KISAK_LIVE_STUBS
    client_t *ClientFromFileInfo; // eax
    dwFileOperationInfo *fileTask; // [esp+0h] [ebp-14h]
    unsigned __int8 *tempCompressedFileBuffer; // [esp+4h] [ebp-10h]
    dwFileOperationInfo *fileInfo; // [esp+10h] [ebp-4h]

    if ( task->payload )
    {
        fileInfo = (dwFileOperationInfo *)task->payload;
        if ( fileInfo->isCompressedFile )
        {
            LargeLocal tempCompressedFileBuffer_large_local(0x10000); // [esp+8h] [ebp-Ch] BYREF

            //LargeLocal::LargeLocal(&tempCompressedFileBuffer_large_local, 0x10000);
            tempCompressedFileBuffer = tempCompressedFileBuffer_large_local.GetBuf(); // LargeLocal::GetBuf(&tempCompressedFileBuffer_large_local);
            fileTask = fileInfo;
            if ( fileInfo->fileTask.m_bufferSize > 0x10000 )
            {
                Com_PrintError(16, "Insufficient space to decompress file %s", fileInfo->fileTask.m_filename);
                LiveStorage_GetUserFileFailure(task);
                //LargeLocal::~LargeLocal(&tempCompressedFileBuffer_large_local);
                return;
            }
            if ( SV_GetClientFromFileInfo((_QWORD *)task->payload) )
            {
                ClientFromFileInfo = SV_GetClientFromFileInfo((uint64*)fileInfo);
                Com_DPrintf(16, "Decompressing file %s for user %s\n", fileTask->fileTask.m_filename, ClientFromFileInfo->name);
            }
            memcpy(
                tempCompressedFileBuffer,
                (unsigned __int8 *)fileTask->fileTask.m_buffer,
                fileTask->fileTask.m_fileData.m_fileSize);
            if ( !fileTask->fileTask.m_fileData.m_fileSize
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_storage_win.cpp",
                            2308,
                            0,
                            "%s",
                            "fileTask->m_fileData.m_fileSize") )
            {
                __debugbreak();
            }
            MSG_DecompressWithZLib(
                tempCompressedFileBuffer,
                fileTask->fileTask.m_fileData.m_fileSize,
                (unsigned __int8 *)fileTask->fileTask.m_buffer,
                fileTask->fileTask.m_bufferSize);
            //LargeLocal::~LargeLocal(&tempCompressedFileBuffer_large_local);
        }
        if ( fileInfo->fileOperationSucessFunction )
            fileInfo->fileOperationSucessFunction(0, task->payload);
    }
#endif
}

void __cdecl SV_WriteClientFileFailure(TaskRecord *task)
{
    dwFileOperationInfo *fileInfo; // [esp+0h] [ebp-4h]

    if ( task->payload )
    {
        fileInfo = (dwFileOperationInfo *)task->payload;
        if ( fileInfo->fileNotFoundFunction )
            fileInfo->fileNotFoundFunction(0, task->payload);
    }
}

void __cdecl SV_WriteClientFileSuccess(TaskRecord *task)
{
    dwFileOperationInfo *fileInfo; // [esp+0h] [ebp-4h]

    if ( task->payload )
    {
        fileInfo = (dwFileOperationInfo *)task->payload;
        if ( fileInfo->fileOperationSucessFunction )
            fileInfo->fileOperationSucessFunction(0, task->payload);
    }
}

void __cdecl SV_ReadDWFileDeferred(dwFileOperationInfo *fileInfo)
{
#ifdef KISAK_LIVE_STUBS
    bdReference<bdCommonAddr> v1; // [esp+24h] [ebp-14h] BYREF
    bdReference<bdCommonAddr> v2; // [esp+28h] [ebp-10h] BYREF
    dwFileTask *fileTask; // [esp+2Ch] [ebp-Ch]
    bdRemoteTask *task; // [esp+30h] [ebp-8h]
    bdStorage *storage; // [esp+34h] [ebp-4h]

    storage = dwGetStorage(0);
    if ( storage && fileInfo )
    {
        fileTask = &fileInfo->fileTask;
        fileInfo->fileTask.m_buffer = fileInfo->fileBuffer;
        fileTask->m_bufferSize = fileInfo->bufferSize;
        fileTask->m_fileData.m_fileData = fileTask->m_buffer;
        fileTask->m_fileData.m_fileSize = fileTask->m_bufferSize;
        task = 0;
        dwEnterDeferredCritsec();
        if ( LODWORD(fileInfo->ownerID) || HIDWORD(fileInfo->ownerID) )
        {
            task = *(bdRemoteTask **)bdStorage::getFile(
                                                                 storage,
                                                                 (int)&v2,
                                                                 (unsigned __int8 *)fileInfo->fileTask.m_filename,
                                                                 &fileInfo->fileTask.m_fileData,
                                                                 fileInfo->ownerID);
            bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v2);
        }
        else
        {
            task = *(bdRemoteTask **)bdStorage::getPublisherFile(
                                                                 storage,
                                                                 (int)&v1,
                                                                 (unsigned __int8 *)fileInfo->fileTask.m_filename,
                                                                 &fileInfo->fileTask.m_fileData);
            bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v1);
        }
        dwLeaveDeferredCritsec();
        if ( task )
        {
            if ( bdRemoteTask::getStatus(task) == BD_PENDING )
                TaskManager2_DeferTaskToMainThread(task, task_svreadclientfile, fileInfo);
        }
    }
#endif
}

void __cdecl SV_WriteClientFileDeferred(client_t *client, dwFileOperationInfo *fileInfo)
{
#ifdef KISAK_LIVE_STUBS
    bdReference<bdCommonAddr> v2; // [esp+10h] [ebp-20h] BYREF
    unsigned __int8 (*tempCompressedFileBuffer)[65536]; // [esp+14h] [ebp-1Ch]
    LargeLocal tempCompressedFileBuffer_large_local; // [esp+18h] [ebp-18h] BYREF
    bdRemoteTask *task; // [esp+20h] [ebp-10h]
    int fileSize; // [esp+24h] [ebp-Ch]
    unsigned __int8 *fileBuffer; // [esp+28h] [ebp-8h]
    bdStorage *storage; // [esp+2Ch] [ebp-4h]

    storage = dwGetStorage(0);
    if ( storage && client && fileInfo )
    {
        fileSize = fileInfo->bufferSize;
        fileBuffer = fileInfo->fileBuffer;
        if ( fileInfo->isCompressedFile )
        {
            LargeLocal::LargeLocal(&tempCompressedFileBuffer_large_local, 0x10000);
            tempCompressedFileBuffer = (unsigned __int8 (*)[65536])LargeLocal::GetBuf(&tempCompressedFileBuffer_large_local);
            if ( fileInfo->bufferSize > 0x10000u
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_storage_win.cpp",
                            2408,
                            0,
                            "%s",
                            "sizeof( tempCompressedFileBuffer ) >= fileInfo->bufferSize") )
            {
                __debugbreak();
            }
            fileBuffer = (unsigned __int8 *)tempCompressedFileBuffer;
            fileSize = MSG_CompressWithZLib(
                                     fileInfo->fileBuffer,
                                     fileInfo->bufferSize,
                                     (unsigned __int8 *)tempCompressedFileBuffer,
                                     0x10000u);
            if ( fileSize <= 0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_storage_win.cpp",
                            2413,
                            0,
                            "%s",
                            "fileSize > 0") )
            {
                __debugbreak();
            }
            if ( fileSize <= 0 || fileSize > fileInfo->bufferSize )
            {
                LargeLocal::~LargeLocal(&tempCompressedFileBuffer_large_local);
                return;
            }
            LargeLocal::~LargeLocal(&tempCompressedFileBuffer_large_local);
        }
        dwEnterDeferredCritsec();
        task = *(bdRemoteTask **)bdStorage::uploadFile(
                                                             storage,
                                                             (int)&v2,
                                                             (unsigned __int8 *)fileInfo->fileTask.m_filename,
                                                             fileBuffer,
                                                             fileSize,
                                                             0,
                                                             fileInfo->ownerID,
                                                             &fileInfo->fileTask.m_fileInfo);
        bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v2);
        dwLeaveDeferredCritsec();
        if ( task )
        {
            if ( bdRemoteTask::getStatus(task) == BD_PENDING )
                TaskManager2_DeferTaskToMainThread(task, task_svwriteclientfile, fileInfo);
        }
    }
#endif
}

void __cdecl SV_DWReadClientStats(client_t *client)
{
    dwFileOperationInfo *fileInfo; // [esp+4h] [ebp-4h]

    if ( client && client->dw_userID )
    {
        if ( xblive_basictraining->current.enabled )
        {
            ++client->statPacketsReceived;
        }
        else
        {
            Com_DPrintf(15, "Attempting to get stats for client %s\n", client->name);
            fileInfo = SV_GetFreeFileOp();
            if ( fileInfo )
            {
                fileInfo->isUserFile = 1;
                fileInfo->isCompressedFile = 1;
                fileInfo->fileTask.m_filename = (char*)"globalstatsCompressed";
                fileInfo->fileBuffer = client->globalStats;
                fileInfo->bufferSize = 40960;
                fileInfo->fileOperationSucessFunction = (void (__cdecl *)(const int, void *))SV_DWReadClientGlobalStatsSuccess;
                fileInfo->fileNotFoundFunction = (taskCompleteResults (__cdecl *)(const int, void *))SV_DWReadClientGlobalStatsFailure;
                fileInfo->ownerID = client->dw_userID;
                SV_ReadDWFileDeferred(fileInfo);
            }
            else
            {
                Com_DPrintf(15, "Warning: ran out of client fileops. This is bad. Ask Ewan.\n");
            }
        }
    }
}

void __cdecl SV_DWReadClientGlobalStatsSuccess(const int controllerIndex, _QWORD *data)
{
    unsigned __int8 *purchasedItems; // eax
    client_t *v3; // eax
    int outInt; // [esp+9CFCh] [ebp-Ch] BYREF
    int clientnum; // [esp+9D00h] [ebp-8h]
    client_t *ClientFromFileInfo; // [esp+9D04h] [ebp-4h]

    ClientFromFileInfo = SV_GetClientFromFileInfo(data);
    if ( ClientFromFileInfo )
    {
        Com_Printf(15, "Successfully read stats for user %s\n", ClientFromFileInfo->name);
        if ( SV_IsStatsBlobOK((char *)ClientFromFileInfo->globalStats) )
        {
            ClientFromFileInfo->statsValidated = 1;
            memcpy(
                ClientFromFileInfo->globalStatsStable,
                ClientFromFileInfo->globalStats,
                sizeof(ClientFromFileInfo->globalStatsStable));
            ++ClientFromFileInfo->statPacketsReceived;
            if ( LODWORD(ClientFromFileInfo->statPacketsReceived) == 2 && !HIDWORD(ClientFromFileInfo->statPacketsReceived) )
            {
                purchasedItems = ClientFromFileInfo->purchasedItems;
                *(unsigned int *)ClientFromFileInfo->purchasedItems = 0;
                *((unsigned int *)purchasedItems + 1) = 0;
                *((unsigned int *)purchasedItems + 2) = 0;
                *((unsigned int *)purchasedItems + 3) = 0;
                *((unsigned int *)purchasedItems + 4) = 0;
                *((unsigned int *)purchasedItems + 5) = 0;
                *((unsigned int *)purchasedItems + 6) = 0;
                *((unsigned int *)purchasedItems + 7) = 0;
                BG_UnlockablesSetPurchasedBits(ClientFromFileInfo->stats, ClientFromFileInfo->purchasedItems);
                v3 = ClientFromFileInfo;
                LODWORD(ClientFromFileInfo->statPacketsReceived) = -1;
                HIDWORD(v3->statPacketsReceived) = 7;
                NET_OutOfBandPrint(NS_SERVER, ClientFromFileInfo->header.netchan.remoteAddress, "statsOK\n");
            }
            clientnum = SV_GetClientNumForBdOnlineUserID(ClientFromFileInfo->dw_userID);
            outInt = 0;
            SV_GetClientDDLStat(clientnum, "RANKXP", &outInt);
            Com_Printf(15, "Client has xp of %i\n", outInt);
        }
        else
        {
            NET_OutOfBandPrint(NS_SERVER, ClientFromFileInfo->header.netchan.remoteAddress, "error\nPATCH_BAD_STATS");
        }
    }
    else
    {
        Com_PrintWarning(15, "Got successful globalstats for user %llu, but no matching user on server found!\n", data[34]);
    }
    SV_ResetFileOp((dwFileOperationInfo*)data);
}

char __cdecl SV_IsStatsBlobOK(char *data)
{
    char backupBuffer[40964]; // [esp+0h] [ebp-9CF8h] BYREF
    char *buffer; // [esp+9CF0h] [ebp-8h]
    char v4; // [esp+9CF7h] [ebp-1h]

    v4 = 0;
    buffer = data;
    if ( DDL_AssociateBuffer(data, 40960, g_statsDDL) )
        return 1;
    if ( DDL_FixBufferVersion(buffer, g_statsDDL, "ddl_mp/stats.ddl", backupBuffer, 40960)
        || DDL_FixBufferVersion(buffer, g_statsDDL, "ddl_mp/stats_archive.ddl", backupBuffer, 40960) )
    {
        DDL_NoCheckPrintWarning("DDL: Stats buffer updated to version %d\n", g_statsDDL->version);
        return 1;
    }
    return v4;
}

int __cdecl SV_DWReadClientGlobalStatsFailure(int controllerIndex, _QWORD *data)
{
    client_t *client; // [esp+4h] [ebp-4h]

    client = SV_GetClientFromFileInfo(data);
    if ( client )
    {
        Com_Printf(15, "Couldn't get stats for user %s!", client->name);
        Com_DPrintf(15, "DWSTATS - No stats for user %s, giving them fresh stats\n", client->name);
        LiveStorage_ResetStats(client->globalStats);
        client->statsValidated = 1;
        memcpy(client->globalStatsStable, client->globalStats, sizeof(client->globalStatsStable));
        ++client->statPacketsReceived;
        if ( LODWORD(client->statPacketsReceived) == 2 && !HIDWORD(client->statPacketsReceived) )
        {
            *(unsigned int *)client->purchasedItems = 0;
            *(unsigned int *)&client->purchasedItems[4] = 0;
            *(unsigned int *)&client->purchasedItems[8] = 0;
            *(unsigned int *)&client->purchasedItems[12] = 0;
            *(unsigned int *)&client->purchasedItems[16] = 0;
            *(unsigned int *)&client->purchasedItems[20] = 0;
            *(unsigned int *)&client->purchasedItems[24] = 0;
            *(unsigned int *)&client->purchasedItems[28] = 0;
            BG_UnlockablesSetPurchasedBits(client->stats, client->purchasedItems);
            LODWORD(client->statPacketsReceived) = -1;
            HIDWORD(client->statPacketsReceived) = 7;
            NET_OutOfBandPrint(NS_SERVER, client->header.netchan.remoteAddress, "statsOK\n");
        }
    }
    SV_ResetFileOp((dwFileOperationInfo*)data);
    return 1;
}

void __cdecl SV_DWReadClientCAC(client_t *client)
{
    const char *v1; // [esp+0h] [ebp-Ch]
    dwFileOperationInfo *fileInfo; // [esp+8h] [ebp-4h]

    if ( client && client->dw_userID )
    {
        Com_DPrintf(15, "Attempting to get CAC for client %s\n", client->name);
        fileInfo = SV_GetFreeFileOp();
        if ( fileInfo )
        {
            fileInfo->isUserFile = 1;
            fileInfo->isCompressedFile = 1;
            if ( xblive_basictraining->current.enabled )
                v1 = "mpstatsBasicTraining";
            else
                v1 = "mpstatsCompressed";
            fileInfo->fileTask.m_filename = (char *)v1;
            fileInfo->fileBuffer = client->stats;
            fileInfo->bufferSize = 40960;
            fileInfo->fileOperationSucessFunction = (void (__cdecl *)(const int, void *))SV_DWReadClientCACSuccess;
            fileInfo->fileNotFoundFunction = (taskCompleteResults (__cdecl *)(const int, void *))SV_DWReadClientCACFailure;
            fileInfo->ownerID = client->dw_userID;
            SV_ReadDWFileDeferred(fileInfo);
        }
        else
        {
            Com_DPrintf(15, "Warning: ran out of client fileops. This is bad. Ask Ewan.\n");
        }
    }
}

void __cdecl SV_DWReadClientCACSuccess(const int controllerIndex, _QWORD *data)
{
    client_t *ClientFromFileInfo; // [esp+9CF8h] [ebp-4h]

    ClientFromFileInfo = SV_GetClientFromFileInfo(data);
    if ( ClientFromFileInfo )
    {
        Com_Printf(15, "Successfully read stats for user %s\n", ClientFromFileInfo->name);
        if ( SV_IsStatsBlobOK((char *)ClientFromFileInfo->stats) )
        {
            ++ClientFromFileInfo->statPacketsReceived;
            if ( LODWORD(ClientFromFileInfo->statPacketsReceived) == 2 && !HIDWORD(ClientFromFileInfo->statPacketsReceived) )
            {
                *(unsigned int *)ClientFromFileInfo->purchasedItems = 0;
                *(unsigned int *)&ClientFromFileInfo->purchasedItems[4] = 0;
                *(unsigned int *)&ClientFromFileInfo->purchasedItems[8] = 0;
                *(unsigned int *)&ClientFromFileInfo->purchasedItems[12] = 0;
                *(unsigned int *)&ClientFromFileInfo->purchasedItems[16] = 0;
                *(unsigned int *)&ClientFromFileInfo->purchasedItems[20] = 0;
                *(unsigned int *)&ClientFromFileInfo->purchasedItems[24] = 0;
                *(unsigned int *)&ClientFromFileInfo->purchasedItems[28] = 0;
                BG_UnlockablesSetPurchasedBits(ClientFromFileInfo->stats, ClientFromFileInfo->purchasedItems);
                LODWORD(ClientFromFileInfo->statPacketsReceived) = -1;
                HIDWORD(ClientFromFileInfo->statPacketsReceived) = 7;
                NET_OutOfBandPrint(NS_SERVER, ClientFromFileInfo->header.netchan.remoteAddress, "statsOK\n");
            }
        }
        else
        {
            NET_OutOfBandPrint(NS_SERVER, ClientFromFileInfo->header.netchan.remoteAddress, "error\nPATCH_BAD_STATS");
        }
    }
    SV_ResetFileOp((dwFileOperationInfo*)data);
}

int __cdecl SV_DWReadClientCACFailure(int controllerIndex, _QWORD *data)
{
    char *v2; // eax
    const char *v3; // eax
    client_t *client; // [esp+4h] [ebp-4h]

    client = SV_GetClientFromFileInfo(data);
    if ( client )
    {
        Com_Printf(15, "Couldn't get CAC for user %s!", client->name);
        v2 = va("DWSTATS - No CAC for user %s, giving them fresh CAC\n", client->name);
        Com_PrintWarning(15, v2);
        v3 = va("DWSTATS - No CAC for user %s, giving them fresh CAC\n", client->name);
        SV_SysLog_LogMessage(1, v3);
        LiveStorage_ResetStats(client->stats);
        ++client->statPacketsReceived;
        if ( LODWORD(client->statPacketsReceived) == 2 && !HIDWORD(client->statPacketsReceived) )
        {
            *(unsigned int *)client->purchasedItems = 0;
            *(unsigned int *)&client->purchasedItems[4] = 0;
            *(unsigned int *)&client->purchasedItems[8] = 0;
            *(unsigned int *)&client->purchasedItems[12] = 0;
            *(unsigned int *)&client->purchasedItems[16] = 0;
            *(unsigned int *)&client->purchasedItems[20] = 0;
            *(unsigned int *)&client->purchasedItems[24] = 0;
            *(unsigned int *)&client->purchasedItems[28] = 0;
            BG_UnlockablesSetPurchasedBits(client->stats, client->purchasedItems);
            LODWORD(client->statPacketsReceived) = -1;
            HIDWORD(client->statPacketsReceived) = 7;
            NET_OutOfBandPrint(NS_SERVER, client->header.netchan.remoteAddress, "statsOK\n");
        }
    }
    SV_ResetFileOp((dwFileOperationInfo*)data);
    return 1;
}

void __cdecl SV_DWWriteClientStats(client_t *client)
{
    void *v1; // ecx
    char *v2; // eax
    const char *v3; // eax
    int clientxp; // [esp+Ch] [ebp-14h] BYREF
    int *checksum; // [esp+10h] [ebp-10h]
    int clientnum; // [esp+14h] [ebp-Ch]
    dwFileOperationInfo *fileInfo; // [esp+18h] [ebp-8h]
    int clientStableXP; // [esp+1Ch] [ebp-4h] BYREF

#ifdef KISAK_LIVE
    if ( Com_CanWriteLeaderboards(v1) )
    {
        if ( client && client->dw_userID && !xblive_basictraining->current.enabled )
        {
            if ( LODWORD(client->statPacketsReceived) == -1 && HIDWORD(client->statPacketsReceived) == 7 )
            {
                Com_DPrintf(15, "Attempting to write stats for client %s\n", client->name);
                fileInfo = SV_GetFreeFileOp();
                if ( fileInfo )
                {
                    fileInfo->isUserFile = 1;
                    fileInfo->isCompressedFile = 1;
                    fileInfo->fileTask.m_filename = (char*)"globalstatsCompressed";
                    fileInfo->fileBuffer = client->globalStats;
                    fileInfo->bufferSize = 40960;
                    fileInfo->fileOperationSucessFunction = (void (__cdecl *)(const int, void *))SV_DWWriteClientGlobalStatsSuccess;
                    fileInfo->ownerID = client->dw_userID;
                    checksum = (int *)client->globalStats;
                    *(unsigned int *)client->globalStats = LiveStats_ChecksumGamerStats(&client->globalStats[4], 40956);
                    clientnum = SV_GetClientNumForBdOnlineUserID(client->dw_userID);
                    clientxp = 0;
                    clientStableXP = 0;
                    if ( SV_GetClientDDLStat(clientnum, "RANKXP", &clientxp)
                        && SV_GetStatFromBlob((char *)client->globalStatsStable, "RANKXP", &clientStableXP) )
                    {
                        if ( clientxp < clientStableXP )
                        {
                            v2 = va(
                                         "Client %s has stableXP of %i, but new XP of %i, aborting write!!\n",
                                         client->name,
                                         clientStableXP,
                                         clientxp);
                            Com_DPrintf(15, v2);
                            v3 = va(
                                         "Client %s has stableXP of %i, but new XP of %i, aborting write!!\n",
                                         client->name,
                                         clientStableXP,
                                         clientxp);
                            SV_SysLog_LogMessage(0, v3);
                        }
                        else
                        {
                            Com_Printf(0, "Client we're writing stats for has xp of %i\n", clientxp);
                            SV_WriteClientFileDeferred(client, fileInfo);
                        }
                    }
                    else
                    {
                        Com_PrintError(0, "Couldn't get xp stats for client %s, aborting write!\n", client->name);
                    }
                }
                else
                {
                    Com_DPrintf(15, "Warning: ran out of client fileops. This is bad. Ask Ewan.\n");
                }
            }
            else
            {
                Com_PrintError(0, "Aborting write for %s, don't have stats!\n", client->name);
            }
        }
    }
    else if ( client )
    {
        Com_DPrintf(15, "Not writing stats for %s, don't have permission\n", client->name);
    }
    else
    {
        Com_DPrintf(15, "Not writing stats for %s, don't have permission\n", "unknown");
    }
#else
    Com_DPrintf(15, "[KISAK] Lol this got triggered.");
#endif
}

void __cdecl SV_DWWriteClientGlobalStatsSuccess(int controllerIndex, unsigned __int8 **data)
{
    client_t *client; // [esp+0h] [ebp-10h]
    unsigned __int64 uid; // [esp+8h] [ebp-8h]

    client = SV_GetClientFromFileInfo((uint64*)data);
    if ( client )
    {
        Com_Printf(15, "Write stats success for user %s\n", client->name);
        memcpy(client->globalStatsStable, client->globalStats, sizeof(client->globalStatsStable));
        uid = client->dw_userID;
    }
    else
    {
        uid = SV_GetBdUidFromFileInfo((uint64*)data);
    }
    LiveStorage_SendStatsBufferToClient(uid, data[62], 40960, BLOB_TYPE_GLOBAL, 0);
    SV_ResetFileOp((dwFileOperationInfo*)data);
}

void __cdecl LiveStorage_SendStatsBufferToClient(
                unsigned __int64 uid,
                unsigned __int8 *buffer,
                int buffersize,
                blobtype_t blobtype,
                bool sendOK)
{
#ifdef KISAK_LIVE_STUBS
    char *v5; // eax
    const char *v6; // eax
    int v7; // [esp+14h] [ebp-9D04h]
    _WORD payload[20092]; // [esp+18h] [ebp-9D00h] BYREF
    unsigned __int8 *to; // [esp+9D14h] [ebp-4h]

    memset(payload, 0, 40970);
    to = (unsigned __int8 *)&payload[5];
    if ( blobtype )
    {
        if ( blobtype != BLOB_TYPE_GLOBAL )
        {
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_storage_win.cpp",
                            1299,
                            0,
                            "Invalid blob type!\n") )
                __debugbreak();
            return;
        }
        LOBYTE(payload[0]) = 7;
    }
    else
    {
        LOBYTE(payload[0]) = 8;
    }
    HIBYTE(payload[0]) = sendOK ? 5 : 0;
    v7 = MSG_CompressWithZLib(buffer, 0xA000u, to, 0xA000u);
    if ( v7 <= 0 )
    {
        Com_DPrintf(15, "Couldn't compress stats! :(\n");
        SV_SysLog_LogMessage(0, "Couldn't compress stats! :(\n");
    }
    else
    {
        *(unsigned int *)&payload[1] = v7;
        *(unsigned int *)&payload[3] = Com_BlockChecksumKey32(to, v7, 0);
        if ( (unsigned int)(*(unsigned int *)&payload[1] + 10) > 0x4000 )
        {
            v5 = va(
                         "Attempted to send blob of size %u to %llu, but max size is %u\n",
                         *(unsigned int *)&payload[1] + 10,
                         uid,
                         0x4000);
            Com_DPrintf(15, v5);
            v6 = va(
                         "Attempted to send blob of size %u to %llu, but max size is %u\n",
                         *(unsigned int *)&payload[1] + 10,
                         uid,
                         0x4000);
            SV_SysLog_LogMessage(0, v6);
        }
        else
        {
            dwMessaging_SendDeferredInstantMessage(uid, (unsigned __int8 *)payload, *(unsigned int *)&payload[1] + 10);
        }
    }
#endif
}

char __cdecl SV_CACValidate_SetIntStat(unsigned __int8 *buffer, const char *stat, unsigned int value)
{
    const ddlState_t *RootDDLState; // eax
    ddlState_t searchState; // [esp+0h] [ebp-10h] BYREF

    if ( (!buffer || !stat)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_storage_win.cpp",
                    2618,
                    0,
                    "%s",
                    "buffer && stat") )
    {
        __debugbreak();
    }
    RootDDLState = LiveStats_GetRootDDLState();
    if ( DDL_MoveTo(RootDDLState, &searchState, 2, "PlayerStatsList", stat) )
    {
        DDL_SetInt(&searchState, value, (char *)buffer);
        return 1;
    }
    else
    {
        DDL_PrintError("DDL: Error could not find item %s\n", stat);
        return 0;
    }
}

void __cdecl SV_PrestigeReset(char *liveStatsBuffer)
{
    const ddlState_t *RootDDLState; // eax
    const ddlState_t *v2; // eax
    const ddlState_t *v3; // eax
    const ddlState_t *v4; // eax
    char *v5; // [esp-8h] [ebp-3Ch]
    ddlState_t searchStateGroupStats; // [esp+0h] [ebp-34h] BYREF
    unsigned int itemNumber; // [esp+10h] [ebp-24h]
    ddlState_t searchStateChallenges; // [esp+14h] [ebp-20h] BYREF
    ddlState_t searchStateStats; // [esp+24h] [ebp-10h] BYREF

    for ( itemNumber = 0; itemNumber < 0x100; ++itemNumber )
    {
        if ( BG_UnlockablesIsItemValidNotNull(itemNumber) )
        {
            v5 = va("%d", itemNumber);
            RootDDLState = LiveStats_GetRootDDLState();
            if ( DDL_MoveTo(RootDDLState, &searchStateStats, 3, "ItemStats", v5)
                && DDL_IterateFirst(&searchStateStats, &searchStateStats) )
            {
                do
                {
                    LiveStats_SetItemStat(&searchStateStats, "challengeTier", liveStatsBuffer, 0);
                    LiveStats_SetItemStat(&searchStateStats, "challengeValue", liveStatsBuffer, 0);
                }
                while ( DDL_IterateNext(&searchStateStats, &searchStateStats) );
            }
            SV_CACValidate_ClearWeaponInfo(liveStatsBuffer, itemNumber);
        }
    }
    v2 = LiveStats_GetRootDDLState();
    if ( DDL_MoveTo(v2, &searchStateStats, 1, "GroupStats") && DDL_IterateFirst(&searchStateStats, &searchStateStats) )
    {
        do
        {
            if ( DDL_MoveTo(&searchStateStats, &searchStateGroupStats, 1, "stats")
                && DDL_IterateFirst(&searchStateGroupStats, &searchStateGroupStats) )
            {
                do
                {
                    LiveStats_SetItemStat(&searchStateGroupStats, "challengeTier", liveStatsBuffer, 0);
                    LiveStats_SetItemStat(&searchStateGroupStats, "challengeValue", liveStatsBuffer, 0);
                }
                while ( DDL_IterateNext(&searchStateGroupStats, &searchStateGroupStats) );
            }
        }
        while ( DDL_IterateNext(&searchStateStats, &searchStateStats) );
    }
    v3 = LiveStats_GetRootDDLState();
    if ( DDL_MoveTo(v3, &searchStateChallenges, 1, "ChallengeTier") )
    {
        v4 = LiveStats_GetRootDDLState();
        if ( DDL_MoveTo(v4, &searchStateStats, 1, "ChallengeValue") )
        {
            if ( DDL_IterateFirst(&searchStateChallenges, &searchStateChallenges)
                && DDL_IterateFirst(&searchStateStats, &searchStateStats) )
            {
                do
                {
                    DDL_SetInt(&searchStateChallenges, 0, liveStatsBuffer);
                    DDL_SetInt(&searchStateStats, 0, liveStatsBuffer);
                }
                while ( DDL_IterateNext(&searchStateStats, &searchStateStats)
                         && DDL_IterateNext(&searchStateChallenges, &searchStateChallenges) );
            }
        }
    }
    LiveStats_ResetGamemodeChallenges(liveStatsBuffer);
}

bool __cdecl SV_ValidateClientCAC(
                unsigned __int8 *oldcacblob,
                int oldcacblobsize,
                unsigned __int8 *newcacblob,
                int newcacblobsize,
                unsigned __int8 *globalblob,
                int globalblobsize,
                unsigned __int64 clientUID)
{
    char *v7; // eax
    const char *v8; // eax
    char *v9; // eax
    const char *v10; // eax
    int LayerCost; // eax
    const char *ClanTagFeature; // eax
    int v14; // eax
    const char *ItemName; // eax
    char *v16; // eax
    const char *v17; // eax
    const char *v18; // eax
    const char *v19; // eax
    int v20; // eax
    int ItemAttachmentCost; // eax
    int WeaponOptionCost; // eax
    char *ContractName; // eax
    char *v24; // eax
    char *v25; // eax
    const char *v26; // eax
    int v27; // eax
    char *v28; // eax
    char *v29; // eax
    const char *v30; // eax
    char *v31; // eax
    const char *v32; // eax
    char *v33; // eax
    const char *v34; // eax
    int v35; // [esp-4h] [ebp-A0h]
    int v36; // [esp-4h] [ebp-A0h]
    int ItemCost; // [esp-4h] [ebp-A0h]
    int v38; // [esp-4h] [ebp-A0h]
    int v39; // [esp-4h] [ebp-A0h]
    signed int PurchasedEmblemLayers; // [esp+0h] [ebp-9Ch]
    int numpurchased; // [esp+4h] [ebp-98h] BYREF
    int contractcost; // [esp+8h] [ebp-94h]
    int contractsIdx; // [esp+Ch] [ebp-90h]
    int unlockRank; // [esp+10h] [ebp-8Ch]
    int cp_spent; // [esp+14h] [ebp-88h] BYREF
    int oldindex; // [esp+18h] [ebp-84h]
    int newindex; // [esp+1Ch] [ebp-80h]
    int activeindex; // [esp+20h] [ebp-7Ch]
    int optionIdx; // [esp+24h] [ebp-78h]
    int attachNum; // [esp+28h] [ebp-74h]
    int weaponoptioncount; // [esp+2Ch] [ebp-70h]
    unsigned int itemNumber; // [esp+30h] [ebp-6Ch]
    unsigned int index; // [esp+34h] [ebp-68h]
    int k; // [esp+38h] [ebp-64h]
    int j; // [esp+3Ch] [ebp-60h]
    int i; // [esp+40h] [ebp-5Ch]
    int maxPrestige; // [esp+44h] [ebp-58h]
    int maxXP; // [esp+48h] [ebp-54h]
    int timeStarted; // [esp+4Ch] [ebp-50h]
    int startemblem; // [esp+50h] [ebp-4Ch]
    int endemblem; // [esp+54h] [ebp-48h]
    int newPrestige; // [esp+58h] [ebp-44h] BYREF
    int validateTime; // [esp+5Ch] [ebp-40h]
    int clientRank; // [esp+60h] [ebp-3Ch]
    int oldbalance; // [esp+64h] [ebp-38h] BYREF
    char from[20]; // [esp+68h] [ebp-34h] BYREF
    int clientPrestige; // [esp+80h] [ebp-1Ch] BYREF
    bool globalok; // [esp+86h] [ebp-16h] BYREF
    bool oldcacok; // [esp+87h] [ebp-15h] BYREF
    int clientXP; // [esp+88h] [ebp-14h] BYREF
    int newmoneyspent; // [esp+8Ch] [ebp-10h]
    bool ok; // [esp+93h] [ebp-9h]
    int newbalance; // [esp+94h] [ebp-8h]
    int oldmoneyspent; // [esp+98h] [ebp-4h] BYREF

    if ( g_cacvalidateState != CAC_FETCHTWO
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_storage_win.cpp",
                    2776,
                    0,
                    "%s",
                    "CAC_FETCHTWO == g_cacvalidateState") )
    {
        __debugbreak();
    }
    ok = 1;
    globalok = 0;
    oldcacok = 0;
    clientRank = 0;
    clientPrestige = 0;
    clientXP = 0;
    oldbalance = 0;
    newbalance = 0;
    oldmoneyspent = 0;
    newmoneyspent = 0;
    memset(from, 0, 17);
    XUIDToString(&clientUID, from);
    SV_CACValidate_EvaluateStatsBlobs(
        &oldcacok,
        &globalok,
        (char *)oldcacblob,
        (char *)globalblob,
        oldcacblobsize,
        globalblobsize);
    if ( globalok )
    {
        LiveStats_GetIntPlayerStatFromBase(&clientXP, "RANKXP", (char *)globalblob);
        LiveStats_GetIntPlayerStatFromBase(&clientPrestige, "PLEVEL", (char *)globalblob);
        clientRank = CL_GetRankForXp(clientXP);
        LiveStats_GetIntPlayerStatFromBase(&oldbalance, "CODPOINTS", (char *)globalblob);
        LiveStats_GetIntPlayerStatFromBase(&oldmoneyspent, "CURRENCYSPENT", (char *)globalblob);
    }
    if ( DDL_AssociateBuffer((char *)newcacblob, newcacblobsize, g_statsDDL) )
    {
        timeStarted = Sys_Milliseconds();
        if ( !SV_CACValidate_CheckIfEquippedItemsArePurchased(newcacblob, -1, -1) )
        {
            v9 = va("rejected cac from %s, items equipped that are not purchased!\n", from);
            Com_DPrintf(15, v9);
            v10 = va("rejected cac from %s, items equipped that are not purchased!\n", from);
            SV_SysLog_LogMessage(0, v10);
            return 0;
        }
        newPrestige = 0;
        LiveStats_GetIntPlayerStatFromBase(&newPrestige, "PLEVEL", (char *)newcacblob);
        if ( newPrestige > clientPrestige )
        {
            maxXP = CL_GetMaxXP();
            maxPrestige = CL_GetMaxPrestige();
            Com_DPrintf(15, "Client claims to have gone up in prestige level!\n");
            if ( newPrestige - clientPrestige <= 1 && clientXP >= maxXP && newPrestige <= maxPrestige )
            {
                Com_DPrintf(15, "Setting client prestige to %i from %i\n", newPrestige, clientPrestige);
                SV_PrestigeReset((char *)globalblob);
                SV_PrestigeReset((char *)newcacblob);
                SV_CACValidate_SetIntStat(globalblob, "RANKXP", 0);
                SV_CACValidate_SetIntStat(globalblob, "RANK", 0);
                SV_CACValidate_SetIntStat(globalblob, "CODPOINTS", 0);
                SV_CACValidate_SetIntStat(globalblob, "PLEVEL", newPrestige);
                return 1;
            }
            else
            {
                Com_DPrintf(
                    15,
                    "Treason uncloaked! User claims to have gone up more than one prestige level, or has insufficient xp, or has ex"
                    "ceeded max prestige!\n");
                SV_SysLog_LogMessage(
                    0,
                    "Treason uncloaked! User claims to have gone up more than one prestige level, or has insufficient xp, or has ex"
                    "ceeded max prestige!\n");
                return 0;
            }
        }
        if ( oldcacok )
            PurchasedEmblemLayers = SV_CACValidate_GetPurchasedEmblemLayers(oldcacblob);
        else
            PurchasedEmblemLayers = 0;
        startemblem = PurchasedEmblemLayers;
        endemblem = SV_CACValidate_GetPurchasedEmblemLayers(newcacblob);
        if ( endemblem > PurchasedEmblemLayers )
        {
            for ( i = startemblem; i < endemblem; ++i )
            {
                LayerCost = BG_EmblemsGetLayerCost(i);
                newmoneyspent += LayerCost;
            }
        }
        for ( j = 0; j < BG_EmblemsGetTotalBackgroundCount(); ++j )
        {
            if ( SV_CACValidate_IsBackgroundPurchased(newcacblob, clientRank, j)
                && (!oldcacok || !SV_CACValidate_IsBackgroundPurchased(oldcacblob, clientRank, j)) )
            {
                newmoneyspent += BG_EmblemsGetBackgroundCost(j);
            }
        }
        for ( k = 0; k < BG_EmblemsGetIconCount(); ++k )
        {
            if ( SV_CACValidate_IsIconPurchased(newcacblob, clientRank, k)
                && (!oldcacok || !SV_CACValidate_IsIconPurchased(oldcacblob, clientRank, k)) )
            {
                newmoneyspent += BG_EmblemsGetIconCost(k);
            }
        }
        for ( index = 0; (int)index < BG_UnlockablesGetClanTagFeatureCount(); ++index )
        {
            if ( SV_CACValidate_IsClanTagFeaturePurchased(newcacblob, index, clientPrestige, clientRank)
                && (!oldcacok || !SV_CACValidate_IsClanTagFeaturePurchased(oldcacblob, index, clientPrestige, clientRank)) )
            {
                ClanTagFeature = BG_UnlockablesGetClanTagFeature(index, CLANTAG_COL_COST);
                v14 = atoi(ClanTagFeature);
                newmoneyspent += v14;
            }
        }
        for ( itemNumber = 0; itemNumber < 0x100; ++itemNumber )
        {
            if ( BG_UnlockablesIsItemValidNotNull(itemNumber)
                && SV_CacValidate_IsItemPurchased(newcacblob, itemNumber, clientPrestige, clientRank) )
            {
                if ( BG_UnlockablesIsItemLockedForRank(clientPrestige, clientRank, itemNumber) )
                {
                    v35 = clientRank;
                    ItemName = BG_UnlockablesGetItemName(itemNumber);
                    v16 = va("Rejecting cac from client %s, item %s is locked for rank %i\n", from, ItemName, v35);
                    Com_DPrintf(15, v16);
                    v36 = clientRank;
                    v17 = BG_UnlockablesGetItemName(itemNumber);
                    v18 = va("Rejecting cac from client %s, item %s is locked for rank %i\n", from, v17, v36);
                    SV_SysLog_LogMessage(0, v18);
                    ok = 0;
                    break;
                }
                if ( !oldcacok || !SV_CacValidate_IsItemPurchased(oldcacblob, itemNumber, clientPrestige, clientRank) )
                {
                    ItemCost = BG_UnlockablesGetItemCost(itemNumber);
                    v19 = BG_UnlockablesGetItemName(itemNumber);
                    Com_DPrintf(15, "CACValidate: New item bought: %s, cost %i\n", v19, ItemCost);
                    v20 = BG_UnlockablesGetItemCost(itemNumber);
                    newmoneyspent += v20;
                }
                for ( attachNum = 0; attachNum < SV_GetMaxAttachCount(); ++attachNum )
                {
                    if ( SV_CACValidate_IsItemAttachmentPurchased(newcacblob, itemNumber, attachNum, clientPrestige, clientRank)
                        && (!oldcacok
                         || !SV_CACValidate_IsItemAttachmentPurchased(oldcacblob, itemNumber, attachNum, clientPrestige, clientRank)) )
                    {
                        ItemAttachmentCost = BG_UnlockablesGetItemAttachmentCost(itemNumber, attachNum);
                        newmoneyspent += ItemAttachmentCost;
                    }
                }
                weaponoptioncount = BG_GetWeaponOptionCount();
                for ( optionIdx = 0; optionIdx < weaponoptioncount; ++optionIdx )
                {
                    if ( SV_CACValidate_IsItemOptionPurchased(newcacblob, itemNumber, optionIdx, clientPrestige, clientRank)
                        && (!oldcacok
                         || !SV_CACValidate_IsItemOptionPurchased(oldcacblob, itemNumber, optionIdx, clientPrestige, clientRank)) )
                    {
                        WeaponOptionCost = BG_GetWeaponOptionCost(optionIdx);
                        newmoneyspent += WeaponOptionCost;
                    }
                }
            }
        }
        for ( activeindex = 0; activeindex < 3; ++activeindex )
        {
            oldindex = -1;
            newindex = -1;
            if ( oldcacok )
                oldindex = SV_CACValidate_GetIndexForActiveContract(oldcacblob, activeindex);
            newindex = SV_CACValidate_GetIndexForActiveContract(newcacblob, activeindex);
            if ( oldindex != newindex && newindex != -1 )
            {
                unlockRank = LiveContracts_GetUnlockLevel(newindex);
                contractsIdx = BG_UnlockablesGetItemIndexFromName("FEATURE_CONTRACTS");
                if ( unlockRank > clientRank || BG_UnlockablesIsItemLockedForRank(clientPrestige, clientRank, contractsIdx) )
                {
                    v38 = clientRank;
                    ContractName = LiveContracts_GetContractName(newindex);
                    v24 = va(
                                    "CACValidate: Rejecting cac from client %s, contract %s is locked for rank %i\n",
                                    from,
                                    ContractName,
                                    v38);
                    Com_DPrintf(15, v24);
                    v39 = clientRank;
                    v25 = LiveContracts_GetContractName(newindex);
                    v26 = va("CACValidate: Rejecting cac from client %s, contract %s is locked for rank %i\n", from, v25, v39);
                    SV_SysLog_LogMessage(0, v26);
                    ok = 0;
                    break;
                }
                cp_spent = 0;
                numpurchased = 0;
                SV_GetStatFromBlob((char *)globalblob, "CONTRACTS_CP_SPENT", &cp_spent);
                v27 = LiveContracts_GetContractCost(newindex);
                SV_CACValidate_SetIntStat(globalblob, "CONTRACTS_CP_SPENT", cp_spent + v27);
                SV_GetStatFromBlob((char *)globalblob, "CONTRACTS_PURCHASED", &numpurchased);
                SV_CACValidate_SetIntStat(globalblob, "CONTRACTS_PURCHASED", ++numpurchased);
                v28 = LiveContracts_GetContractName(newindex);
                Com_DPrintf(15, "CACValidate: New contract: %s\n", v28);
                contractcost = LiveContracts_GetContractCost(newindex);
                newmoneyspent += contractcost;
                Com_DPrintf(15, "CACValidate: New contract: cost %i\n", contractcost);
            }
        }
        validateTime = Sys_Milliseconds() - timeStarted;
        if ( validateTime > 10 )
        {
            v29 = va("Validate took %ims\n", validateTime);
            Com_PrintWarning(15, v29);
            v30 = va("Validate took %ims\n", validateTime);
            SV_SysLog_LogMessage(1, v30);
        }
    }
    else
    {
        v7 = va("Rejecting cac from client %s, couldn't associate new buffer\n", from);
        Com_DPrintf(15, v7);
        v8 = va("Rejecting cac from client %s, couldn't associate new buffer\n", from);
        SV_SysLog_LogMessage(0, v8);
        ok = 0;
    }
    if ( oldbalance >= newmoneyspent )
    {
        if ( newmoneyspent > 0 )
        {
            newbalance = oldbalance - newmoneyspent;
            oldmoneyspent += newmoneyspent;
            if ( !SV_CACValidate_SetIntStat(globalblob, "CODPOINTS", oldbalance - newmoneyspent)
                || !SV_CACValidate_SetIntStat(globalblob, "CURRENCYSPENT", oldmoneyspent) )
            {
                v33 = va("Rejecting cac from client %s, couldn't set currency stats\n", from);
                Com_DPrintf(15, v33);
                v34 = va("Rejecting cac from client %s, couldn't set currency stats\n", from);
                SV_SysLog_LogMessage(0, v34);
                return 0;
            }
        }
    }
    else
    {
        v31 = va(
                        "CACValidate: Rejecting cac from client %s, money spent (%i) exceeds balance (%i)\n",
                        from,
                        newmoneyspent,
                        oldbalance);
        Com_DPrintf(15, v31);
        v32 = va(
                        "CACValidate: Rejecting cac from client %s, money spent (%i) exceeds balance (%i)\n",
                        from,
                        newmoneyspent,
                        oldbalance);
        SV_SysLog_LogMessage(0, v32);
        return 0;
    }
    return ok;
}

void __cdecl SV_CACValidate_EvaluateStatsBlobs(
                bool *oldcacok,
                bool *globalok,
                char *oldcacblob,
                char *globalblob,
                int oldcacsize,
                int globalsize)
{
    unsigned __int8 dst[40964]; // [esp+0h] [ebp-9CF0h] BYREF

    if ( (!globalok || !oldcacok)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_storage_win.cpp",
                    2637,
                    0,
                    "%s",
                    "globalok && oldcacok") )
    {
        __debugbreak();
    }
    *globalok = 0;
    *oldcacok = 0;
    if ( globalsize > 0 )
    {
        if ( DDL_AssociateBuffer(globalblob, 40960, g_statsDDL) )
        {
            *globalok = 1;
        }
        else
        {
            memset(dst, 0, 0xA000u);
            if ( DDL_FixBufferVersion(globalblob, g_statsDDL, "ddl_mp/stats.ddl", (char *)dst, 40960)
                || DDL_FixBufferVersion(globalblob, g_statsDDL, "ddl_mp/stats_archive.ddl", (char *)dst, 40960) )
            {
                DDL_NoCheckPrintWarning("CACValidate: Globalbuffer updated to version %d\n", g_statsDDL->version);
                *globalok = 1;
            }
        }
    }
    if ( oldcacsize > 0 )
    {
        if ( DDL_AssociateBuffer(oldcacblob, 40960, g_statsDDL) )
        {
            *oldcacok = 1;
        }
        else
        {
            memset(dst, 0, 0xA000u);
            if ( DDL_FixBufferVersion(oldcacblob, g_statsDDL, "ddl_mp/stats.ddl", (char *)dst, 40960)
                || DDL_FixBufferVersion(oldcacblob, g_statsDDL, "ddl_mp/stats_archive.ddl", (char *)dst, 40960) )
            {
                DDL_NoCheckPrintWarning("CACValidate: Oldcacbuffer updated to version %d\n", g_statsDDL->version);
                *oldcacok = 1;
            }
        }
    }
}

void __cdecl SV_CACValidateSendClientMsgSuccess()
{
    Com_DPrintf(15, "send client success\n");
}

void __cdecl SV_CACValidateSendClientMsgFailure()
{
    Com_DPrintf(15, "send client failure\n");
}

void __cdecl SV_CACValidateSendClientMsg(unsigned __int64 uid, unsigned int msg)
{
#ifdef KISAK_LIVE_STUBS
    TaskRecord *ptask; // [esp+0h] [ebp-Ch]
    TaskRecord *childTask; // [esp+4h] [ebp-8h]
    unsigned __int8 ackBuf[1]; // [esp+Bh] [ebp-1h] BYREF

    Com_DPrintf(15, "CACValidate: Sending opcode %u to client %llu\n", msg, uid);
    ackBuf[0] = msg;
    childTask = dwMessaging_SendInstantMessage(uid, (char *)ackBuf, 1u);
    if ( childTask )
    {
        ptask = TaskManager2_CreateTask(task_SVCACValidateSendClientMsg, 0, childTask, 0);
        if ( ptask )
            TaskManager2_StartTask(ptask);
    }
#endif
}

void __cdecl SV_CACValidateWriteCAC(unsigned __int64 client, unsigned __int8 *cacblob, unsigned int cacsize)
{
    dwFileOperationInfo *fileInfo; // [esp+4h] [ebp-4h]

    Com_DPrintf(15, "CACValidate: Attempting to write cacblob for client %llu\n", client);
    fileInfo = SV_GetFreeFileOp();
    if ( fileInfo )
    {
        fileInfo->isUserFile = 1;
        fileInfo->isCompressedFile = 1;
        fileInfo->fileTask.m_filename = (char*)"mpstatsCompressed";
        fileInfo->fileBuffer = cacblob;
        fileInfo->bufferSize = cacsize;
        fileInfo->ownerID = client;
        fileInfo->fileOperationSucessFunction = (void (__cdecl *)(const int, void *))SV_CACValidateWriteCACSuccess;
        fileInfo->fileOperationFailureFunction = (void (__cdecl *)(const int, void *))SV_CACValidateWriteCACFailure;
        *(unsigned int *)cacblob = LiveStats_ChecksumGamerStats(cacblob + 4, 40956);
        LiveStorage_WriteDWUserFile(0, fileInfo, client);
    }
    else
    {
        Com_DPrintf(15, "Warning: ran out of client fileops. This is bad. Ask Ewan.\n");
    }
}

void __cdecl SV_CACValidateWriteCACSuccess(int controllerIndex, void *data)
{
#ifdef KISAK_LIVE_STUBS
    Com_DPrintf(15, "write cac success\n");
    if ( *operator++(&g_cacvalidateState) == CAC_WRITETWO )
        g_cacvalidateState = CAC_IDLE;
    LiveStorage_SendStatsBufferToClient(
        *((_QWORD *)data + 34),
        *((unsigned __int8 **)data + 62),
        40960,
        BLOB_TYPE_CAC,
        g_cacvalidateState == CAC_IDLE);
    SV_ResetFileOp(data);
#endif
}

void __cdecl SV_CACValidateWriteCACFailure(int controllerIndex, _QWORD *data)
{
    Com_DPrintf(15, "write cac failure\n");
    SV_CACValidateSendClientMsg(data[34], 6u);
    g_cacvalidateState = CAC_IDLE;
    SV_ResetFileOp((dwFileOperationInfo*)data);
}

void __cdecl SV_CACValidateWriteGlobal(unsigned __int64 client, unsigned __int8 *globalblob, unsigned int globalsize)
{
#ifdef KISAK_LIVE_STUBS
    dwFileOperationInfo *fileInfo; // [esp+4h] [ebp-4h]

    Com_DPrintf(15, "CACValidate: Attempting to write globalblob for client %llu with size %i\n", client, globalsize);
    fileInfo = SV_GetFreeFileOp();
    if ( fileInfo )
    {
        if ( !globalsize )
        {
            LiveStorage_ResetStats(globalblob);
            globalsize = 40960;
        }
        if ( (int)g_newCACBlobSize <= 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_storage_win.cpp",
                        3170,
                        0,
                        "%s",
                        "g_newCACBlobSize > 0") )
        {
            __debugbreak();
        }
        if ( (int)g_newCACBlobSize > 0 )
            LiveContracts_SVMergeBuffers(g_newCacBlob, globalblob);
        fileInfo->isUserFile = 1;
        fileInfo->isCompressedFile = 1;
        fileInfo->fileTask.m_filename = "globalstatsCompressed";
        fileInfo->fileBuffer = globalblob;
        fileInfo->bufferSize = globalsize;
        fileInfo->ownerID = client;
        fileInfo->fileOperationSucessFunction = (void (__cdecl *)(const int, void *))SV_CACValidateWriteGlobalSuccess;
        fileInfo->fileOperationFailureFunction = (void (__cdecl *)(const int, void *))SV_CACValidateWriteGlobalFailure;
        *(unsigned int *)globalblob = LiveStats_ChecksumGamerStats(globalblob + 4, 40956);
        LiveStorage_WriteDWUserFile(0, fileInfo, client);
    }
    else
    {
        Com_DPrintf(15, "Warning: ran out of client fileops. This is bad. Ask Ewan.\n");
    }
#endif
}

void __cdecl SV_CACValidateWriteGlobalSuccess(int controllerIndex, void *data)
{
#ifdef KISAK_LIVE_STUBS
    Com_DPrintf(15, "write global success\n");
    if ( *operator++(&g_cacvalidateState) == CAC_WRITETWO )
        g_cacvalidateState = CAC_IDLE;
    LiveStorage_SendStatsBufferToClient(
        *((_QWORD *)data + 34),
        *((unsigned __int8 **)data + 62),
        40960,
        BLOB_TYPE_GLOBAL,
        g_cacvalidateState == CAC_IDLE);
    SV_ResetFileOp(data);
#endif
}

void __cdecl SV_CACValidateWriteGlobalFailure(int controllerIndex, _QWORD *data)
{
    Com_DPrintf(15, "write global failure\n");
    SV_CACValidateSendClientMsg(data[34], 6u);
    g_cacvalidateState = CAC_IDLE;
    SV_ResetFileOp((dwFileOperationInfo*)data);
}

TaskRecord *__cdecl SV_CACValidateReadCAC(unsigned __int64 client, unsigned __int8 *cacblob, unsigned int cacsize)
{
    dwFileOperationInfo *fileInfo; // [esp+0h] [ebp-8h]
    TaskRecord *retval; // [esp+4h] [ebp-4h]

    Com_DPrintf(15, "cac read\n");
    if ( g_cacvalidateState
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_storage_win.cpp",
                    3250,
                    0,
                    "%s",
                    "CAC_IDLE == g_cacvalidateState") )
    {
        __debugbreak();
    }
    retval = 0;
    Com_DPrintf(15, "CACValidate: Attempting to read cacblob for client %llu\n", client);
    fileInfo = SV_GetFreeFileOp();
    if ( fileInfo )
    {
        fileInfo->isUserFile = 1;
        fileInfo->isCompressedFile = 1;
        fileInfo->fileTask.m_filename = (char*)"mpstatsCompressed";
        fileInfo->fileBuffer = cacblob;
        fileInfo->bufferSize = cacsize;
        fileInfo->ownerID = client;
        fileInfo->fileOperationSucessFunction = (void (__cdecl *)(const int, void *))SV_CACValidateReadCACSuccess;
        fileInfo->fileNotFoundFunction = (taskCompleteResults (__cdecl *)(const int, void *))SV_CACValidateReadCACFailure;
        retval = LiveStorage_ReadDWFileByUserID(0, fileInfo, client);
    }
    else
    {
        Com_DPrintf(15, "Warning: ran out of client fileops. This is bad. Ask Ewan.\n");
    }
    if ( !retval )
        Com_Printf(15, "Couldn't read old cac immediate for %llu\n", client);
    return retval;
}

void __cdecl SV_CACValidateReadCACSuccess(int controllerIndex, void *data)
{
#ifdef KISAK_LIVE_STUBS
    Com_DPrintf(15, "cac read success\n");
    if ( !*((unsigned int *)data + 9)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_storage_win.cpp",
                    3201,
                    0,
                    "%s",
                    "fileTask->m_bufferSize > 0") )
    {
        __debugbreak();
    }
    g_oldCACBlobSize = *((unsigned int *)data + 9);
    if ( *operator++(&g_cacvalidateState) == CAC_FETCHTWO )
    {
        if ( SV_ValidateClientCAC(
                     g_oldCACBlob,
                     *((unsigned int *)data + 9),
                     g_newCacBlob,
                     g_newCACBlobSize,
                     g_cac_globalBlob,
                     g_globalBlobSize,
                     *((_QWORD *)data + 34)) )
        {
            SV_CACValidateWriteCAC(*((_QWORD *)data + 34), g_newCacBlob, *((unsigned int *)data + 9));
            SV_CACValidateWriteGlobal(*((_QWORD *)data + 34), g_cac_globalBlob, g_globalBlobSize);
            operator++(&g_cacvalidateState);
        }
        else
        {
            SV_CACValidateSendClientMsg(*((_QWORD *)data + 34), 6u);
            g_cacvalidateState = CAC_IDLE;
        }
    }
    SV_ResetFileOp(data);
#endif
}

int __cdecl SV_CACValidateReadCACFailure(int controllerIndex, void *data)
{
#ifdef KISAK_LIVE_STUBS
    Com_DPrintf(15, "cac read failure\n");
    if ( !*((unsigned int *)data + 9)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_storage_win.cpp",
                    3226,
                    0,
                    "%s",
                    "fileTask->m_bufferSize > 0") )
    {
        __debugbreak();
    }
    g_oldCACBlobSize = 0;
    if ( *operator++(&g_cacvalidateState) == CAC_FETCHTWO )
    {
        if ( SV_ValidateClientCAC(
                     g_oldCACBlob,
                     g_oldCACBlobSize,
                     g_newCacBlob,
                     g_newCACBlobSize,
                     g_cac_globalBlob,
                     g_globalBlobSize,
                     *((_QWORD *)data + 34)) )
        {
            SV_CACValidateWriteCAC(*((_QWORD *)data + 34), g_newCacBlob, *((unsigned int *)data + 9));
            SV_CACValidateWriteGlobal(*((_QWORD *)data + 34), g_cac_globalBlob, g_globalBlobSize);
            operator++(&g_cacvalidateState);
        }
        else
        {
            SV_CACValidateSendClientMsg(*((_QWORD *)data + 34), 6u);
            g_cacvalidateState = CAC_IDLE;
        }
    }
    SV_ResetFileOp(data);
    return 1;
}

TaskRecord *__cdecl SV_CACValidateReadGlobal(
                unsigned __int64 client,
                unsigned __int8 *globalblob,
                unsigned int globalblobsize)
{
    dwFileOperationInfo *fileInfo; // [esp+0h] [ebp-8h]
    TaskRecord *retval; // [esp+4h] [ebp-4h]

    retval = 0;
    if ( g_cacvalidateState
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_storage_win.cpp",
                    3343,
                    0,
                    "%s",
                    "CAC_IDLE == g_cacvalidateState") )
    {
        __debugbreak();
    }
    Com_DPrintf(15, "CACValidate: Attempting to read cacblob for client %llu\n", client);
    fileInfo = SV_GetFreeFileOp();
    if ( fileInfo )
    {
        fileInfo->isUserFile = 1;
        fileInfo->isCompressedFile = 1;
        fileInfo->fileTask.m_filename = "globalstatsCompressed";
        fileInfo->fileBuffer = globalblob;
        fileInfo->bufferSize = globalblobsize;
        fileInfo->ownerID = client;
        fileInfo->fileOperationSucessFunction = (void (__cdecl *)(const int, void *))SV_CACValidateReadGlobalSuccess;
        fileInfo->fileNotFoundFunction = (taskCompleteResults (__cdecl *)(const int, void *))SV_CACValidateReadGlobalFailure;
        retval = LiveStorage_ReadDWFileByUserID(0, fileInfo, client);
    }
    else
    {
        Com_DPrintf(15, "Warning: ran out of client fileops. This is bad. Ask Ewan.\n");
    }
    if ( !retval )
        Com_Printf(15, "Couldn't read global immediate for %llu\n", client);
    return retval;
#else
    return 0;
#endif
}

void __cdecl SV_CACValidateReadGlobalSuccess(int controllerIndex, void *data)
{
#ifdef KISAK_LIVE_STUBS
    Com_DPrintf(15, "global read success\n");
    if ( !*((unsigned int *)data + 9)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_storage_win.cpp",
                    3294,
                    0,
                    "%s",
                    "fileTask->m_bufferSize > 0") )
    {
        __debugbreak();
    }
    g_globalBlobSize = *((unsigned int *)data + 9);
    if ( *operator++(&g_cacvalidateState) == CAC_FETCHTWO )
    {
        if ( SV_ValidateClientCAC(
                     g_oldCACBlob,
                     g_oldCACBlobSize,
                     g_newCacBlob,
                     g_newCACBlobSize,
                     *((unsigned __int8 **)data + 54),
                     *((unsigned int *)data + 9),
                     *((_QWORD *)data + 34)) )
        {
            SV_CACValidateWriteCAC(*((_QWORD *)data + 34), g_newCacBlob, g_newCACBlobSize);
            SV_CACValidateWriteGlobal(*((_QWORD *)data + 34), g_cac_globalBlob, g_globalBlobSize);
            operator++(&g_cacvalidateState);
        }
        else
        {
            SV_CACValidateSendClientMsg(*((_QWORD *)data + 34), 6u);
            g_cacvalidateState = CAC_IDLE;
        }
    }
    SV_ResetFileOp(data);
#endif
}

int __cdecl SV_CACValidateReadGlobalFailure(int controllerIndex, _QWORD *data)
{
#ifdef KISAK_LIVE_STUBS
    Com_DPrintf(15, "global read failure\n");
    g_globalBlobSize = 0;
    if ( *operator++(&g_cacvalidateState) == CAC_FETCHTWO )
    {
        if ( SV_ValidateClientCAC(
                     g_oldCACBlob,
                     g_oldCACBlobSize,
                     g_newCacBlob,
                     g_newCACBlobSize,
                     g_cac_globalBlob,
                     g_globalBlobSize,
                     data[34]) )
        {
            SV_CACValidateWriteCAC(data[34], g_newCacBlob, g_newCACBlobSize);
            SV_CACValidateWriteGlobal(data[34], g_cac_globalBlob, g_globalBlobSize);
            operator++(&g_cacvalidateState);
        }
        else
        {
            SV_CACValidateSendClientMsg(data[34], 6u);
            g_cacvalidateState = CAC_IDLE;
        }
    }
    SV_ResetFileOp(data);
    return 1;
#else
    return 1;
#endif
}

void __cdecl SV_CACValidateHandleRequest(
                unsigned __int64 clientID,
                unsigned __int8 *compressedcac,
                unsigned int cacsize)
{
#ifdef KISAK_LIVE_STUBS
    unsigned intv3; // eax
    bool ok; // [esp+3h] [ebp-1h]

    if ( (!compressedcac || !cacsize || !clientID)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_storage_win.cpp",
                    3385,
                    0,
                    "%s",
                    "compressedcac && cacsize && clientID") )
    {
        __debugbreak();
    }
    if ( (_S1_13 & 1) == 0 )
    {
        _S1_13 |= 1u;
        lastrequest = Sys_Milliseconds() - 1501;
    }
    ok = 0;
    if ( g_cacvalidateState == CAC_IDLE )
    {
        if ( (int)(Sys_Milliseconds() - lastrequest) <= 1500 )
        {
            v3 = Sys_Milliseconds();
            Com_DPrintf(15, "Rejecting cac, it's only been %ims since last validate\n", v3 - lastrequest);
        }
        else
        {
            lastrequest = Sys_Milliseconds();
            g_globalBlobSize = 0;
            g_oldCACBlobSize = 0;
            g_newCACBlobSize = MSG_DecompressWithZLib(compressedcac, cacsize, g_newCacBlob, 0xA000u);
            if ( (int)g_newCACBlobSize <= 0 )
            {
                Com_PrintWarning(0, "CACValidate: Couldn't decompress stats blob from %llu\n", clientID);
            }
            else
            {
                Com_DPrintf(15, "CACValidate: stats decompressed to %i bytes\n", g_newCACBlobSize);
                if ( SV_CACValidateReadCAC(clientID, g_oldCACBlob, 0xA000u)
                    && SV_CACValidateReadGlobal(clientID, g_cac_globalBlob, 0xA000u) )
                {
                    operator++(&g_cacvalidateState);
                    ok = 1;
                }
                else
                {
                    Com_DPrintf(15, "Couldn't read stats immediate!\n");
                }
            }
        }
    }
    if ( !ok )
    {
        Com_DPrintf(15, "immediate NACK\n");
        SV_CACValidateSendClientMsg(clientID, 4u);
    }
#endif
}

void __cdecl Live_OnNewStatsFromServer(unsigned __int8 *compressedblob, unsigned int blobsize, blobtype_t blobtype)
{
    persistentStats *v3; // eax
    persistentStats *v4; // eax
    persistentStats *v6; // [esp-4h] [ebp-9D00h]
    persistentStats *v7; // [esp-4h] [ebp-9D00h]
    persistentStats *StatsBuffer; // [esp+4h] [ebp-9CF8h]
    persistentStats *v9; // [esp+8h] [ebp-9CF4h]
    unsigned __int8 to[40964]; // [esp+Ch] [ebp-9CF0h] BYREF

    memset(to, 0, 40960);
    if ( blobtype )
    {
        if ( blobtype == BLOB_TYPE_GLOBAL )
        {
            StatsBuffer = LiveStorage_GetStatsBuffer(0, STATS_LOCATION_GLOBAL, 1);
            MSG_DecompressWithZLib(compressedblob, blobsize, to, 0xA000u);
            memcpy(StatsBuffer->statsBuffer, to, 0xA000u);
            LiveStats_ValidateGlobalWithDDL(0);
            LiveStorage_SetStatsWriteNeeded(0, 0, STATS_LOCATION_GLOBAL);
            v7 = LiveStorage_GetStatsBuffer(0, STATS_LOCATION_GLOBAL, 1);
            v4 = LiveStorage_GetStatsBuffer(0, STATS_LOCATION_FORCE_NORMAL, 1);
            LiveContracts_CLMergeBuffers(v4->statsBuffer, v7->statsBuffer);
            if ( !xblive_basictraining->current.enabled )
                LiveStats_MakeStableGlobalStatsBuffer(0);
        }
        else
        {
            Com_PrintError(16, "Invalid blob type!\n");
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_storage_win.cpp",
                            3464,
                            0,
                            "Invalid blob type!\n") )
                __debugbreak();
        }
    }
    else
    {
        v9 = LiveStorage_GetStatsBuffer(0, STATS_LOCATION_FORCE_NORMAL, 1);
        MSG_DecompressWithZLib(compressedblob, blobsize, to, 0xA000u);
        memcpy(v9->statsBuffer, to, 0xA000u);
        LiveStorage_SetStatsWriteNeeded(0, 0, STATS_LOCATION_NORMAL);
        v6 = LiveStorage_GetStatsBuffer(0, STATS_LOCATION_GLOBAL, 1);
        v3 = LiveStorage_GetStatsBuffer(0, STATS_LOCATION_FORCE_NORMAL, 1);
        LiveContracts_CLMergeBuffers(v3->statsBuffer, v6->statsBuffer);
    }
}

char __cdecl Live_CACValidate_DispatchMessage(
                unsigned __int64 senderID,
                unsigned __int8 *message,
                unsigned int messageSize)
{
    int v4; // [esp+0h] [ebp-13A38h]
    unsigned int v5; // [esp+4h] [ebp-13A34h]
    blobtype_t blobtype; // [esp+Ch] [ebp-13A2Ch]
    unsigned __int8 compressedblob[40960]; // [esp+10h] [ebp-13A28h] BYREF
    int v8; // [esp+9CF8h] [ebp-9D40h]
    _BYTE v9[5]; // [esp+9CFFh] [ebp-9D39h] BYREF
    int v10; // [esp+9D04h] [ebp-9D34h] BYREF
    unsigned __int8 compressedcac[40964]; // [esp+9D08h] [ebp-9D30h] BYREF
    int v12; // [esp+139F8h] [ebp-40h]
    int len; // [esp+139FCh] [ebp-3Ch] BYREF
    int v14; // [esp+13A00h] [ebp-38h] BYREF
    msg_t buf; // [esp+13A04h] [ebp-34h] BYREF
    char v16; // [esp+13A36h] [ebp-2h]
    unsigned __int8 v17; // [esp+13A37h] [ebp-1h]

    v16 = 0;
    MSG_InitReadOnly(&buf, message, messageSize);
    MSG_BeginReading(&buf);
    v17 = MSG_ReadByte(&buf);
    v4 = v17 - 3;
    switch ( v17 )
    {
        case 3u:
            MSG_ReadByte(&buf);
            v14 = 0;
            MSG_ReadData(&buf, (unsigned __int8 *)&len, 4);
            if ( (unsigned int)len >= 0xA000 )
            {
                Com_PrintWarning(15, "MAX_CAC_SIZE exceeded!\n");
                return v16;
            }
            MSG_ReadData(&buf, (unsigned __int8 *)&v14, 4);
            memset(compressedcac, 0, 40960);
            MSG_ReadData(&buf, compressedcac, len);
            v12 = Com_BlockChecksumKey32(compressedcac, len, 0);
            if ( v12 == v14 )
            {
                SV_CACValidateHandleRequest(senderID, compressedcac, len);
                return 1;
            }
            Com_PrintError(
                0,
                "CACValidate: checksum mismatch for client %llu: server checksum %u client checksum %u\n",
                senderID,
                v12,
                v14);
            return 1;
        case 4u:
            CL_CACValidateHandleNACK(senderID);
            return 1;
        case 5u:
            return 1;
        case 6u:
            CL_CACValidateHandleBad(senderID);
            return 1;
        case 7u:
        case 8u:
            v9[4] = 0;
            v10 = 0;
            *(unsigned int *)v9 = (unsigned __int8)MSG_ReadByte(&buf);
            MSG_ReadData(&buf, &v9[1], 4);
            if ( *(unsigned int *)&v9[1] > messageSize )
            {
                //Com_PrintError(0, "blobsize %u exceeds message size %u!\n, blobsize, messagesize", v4, v5);
                return 1;
            }
            if ( *(unsigned int *)&v9[1] >= 0xA000u )
            {
                Com_PrintError(15, "MAX_CAC_SIZE exceeded!\n");
            }
            else
            {
                MSG_ReadData(&buf, (unsigned __int8 *)&v10, 4);
                memset(compressedblob, 0, sizeof(compressedblob));
                if ( v9[0] == 5 )
                    CL_CACValidateHandleOK(senderID);
                MSG_ReadData(&buf, compressedblob, *(int *)&v9[1]);
                v8 = Com_BlockChecksumKey32(compressedblob, *(unsigned int *)&v9[1], 0);
                if ( v10 != v8 )
                {
                    Com_PrintError(0, "Server checksum %u doesn't match client checksum %u!\n", v10, v8);
                    return 1;
                }
                blobtype = BLOB_TYPE_INVALID;
                if ( v17 == 8 )
                {
                    Live_OnNewStatsFromServer(compressedblob, *(unsigned int *)&v9[1], BLOB_TYPE_CAC);
                }
                else
                {
                    if ( v17 == 7 )
                        blobtype = BLOB_TYPE_GLOBAL;
                    Live_OnNewStatsFromServer(compressedblob, *(unsigned int *)&v9[1], blobtype);
                }
                return 1;
            }
            return v16;
        default:
            Com_DPrintf(15, "No handler found for instant message type %u\n", v17);
            return v16;
    }
}


void __cdecl SV_SetPlaylistFetchedTime()
{
    g_svLastPlaylistFetch = Sys_Milliseconds();
}

int __cdecl SV_GetPlaylistFetchedTime()
{
    return g_svLastPlaylistFetch;
}

void __cdecl SV_SetShouldMapRotate(bool should)
{
    s_shouldMapRotate = should;
}

bool __cdecl SV_ShouldMapRotate()
{
    return s_shouldMapRotate;
}

void __cdecl SV_FetchWADDeferred()
{
#ifdef KISAK_LIVE
    bool v0; // [esp+0h] [ebp-4h]

    SV_SetPlaylistFetchedTime();
    v0 = fs_gameDirVar && *(_BYTE *)fs_gameDirVar->current.integer;
    if ( !v0 && Dvar_GetBool("playlist_enabled") && !LiveStorage_FetchingOnlineWAD() )
    {
        dwEnterDeferredCritsec();
        LiveStorage_ForceWADFetch();
        dwLeaveDeferredCritsec();
    }
#endif
}

TaskRecord *__cdecl LiveStorage_ReadPlayerGlobalBlob()
{
#ifdef KISAK_LIVE_STUBS
    TaskRecord *nestedTask; // [esp+0h] [ebp-10h]
    TaskRecord *task; // [esp+4h] [ebp-Ch]
    dwFileOperationInfo *fileInfo; // [esp+Ch] [ebp-4h]

    if ( TaskManager2_TaskIsInProgressForController(task_FetchGlobalBlob, 0) )
        return 0;
    fileInfo = SV_GetFreeFileOp();
    if ( fileInfo )
    {
        fileInfo->retryInfo.lastAttemptTime = 0;
        fileInfo->retryInfo.lastAttemptInterval = 0;
        fileInfo->retryInfo.retryCount = 0;
        task = 0;
        LiveStats_GetRootDDLState();
        fileInfo->isUserFile = 1;
        fileInfo->isCompressedFile = 1;
        fileInfo->fileTask.m_filename = "globalstatsCompressed";
        fileInfo->fileBuffer = s_tempGlobalStatsBuffer;
        fileInfo->bufferSize = 40960;
        fileInfo->fileTask.m_optional = 1;
        fileInfo->fileOperationSucessFunction = (void (__cdecl *)(const int, void *))LiveStorage_GetGlobalBlobSuccess;
        fileInfo->fileNotFoundFunction = (taskCompleteResults (__cdecl *)(const int, void *))LiveStorage_GetGlobalBlobFileNotFound;
        fileInfo->menuDef = "popup_fetchstats";
        nestedTask = LiveStorage_ReadDWFile(0, fileInfo);
        if ( nestedTask )
        {
            task = LiveStorage_SetupNestedTask(task_FetchGlobalBlob, 0, nestedTask, fileInfo);
            if ( task )
                LiveStorage_BeginStatsFetch();
        }
        return task;
    }
    else
    {
        Com_DPrintf(15, "Warning: ran out of client fileops. This is bad. Ask Ewan.\n");
        return 0;
    }
#else
    return NULL;
#endif
}

void __cdecl LiveStorage_GetGlobalBlobSuccess(int controllerIndex, void *data)
{
#ifdef KISAK_LIVE_STUBS
    persistentStats *StatsBuffer; // eax
    unsigned int StatsBufferSize; // [esp-4h] [ebp-4h]

    LiveStorage_FinishStatsFetch();
    StatsBufferSize = LiveStorage_GetStatsBufferSize();
    StatsBuffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_GLOBAL, 1);
    memcpy(StatsBuffer->statsBuffer, s_tempGlobalStatsBuffer, StatsBufferSize);
    LiveStorage_ReadPlayerStats(0, 0, 0);
    LiveStats_ValidateGlobalWithDDL(controllerIndex);
    SV_ResetFileOp(data);
    CL_GetXP_f();
    LiveStats_MakeStableGlobalStatsBuffer(controllerIndex);
#endif
}

int __cdecl LiveStorage_GetGlobalBlobFileNotFound(int controlleridx, void *data)
{
#ifdef KISAK_LIVE_STUBS
    LiveStorage_ReadPlayerStats(0, 0, 0);
    LiveStorage_FinishStatsFetch();
    SV_ResetFileOp(data);
    return 1;
#else
    return 1;
#endif
}

bool __cdecl LiveStorage_AreWeFetchingStats()
{
    return s_fetchingStats;
}

void __cdecl LiveStorage_BeginStatsFetch()
{
    if ( s_fetchingStats
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_storage_win.cpp",
                    3786,
                    0,
                    "%s",
                    "false == s_fetchingStats") )
    {
        __debugbreak();
    }
    s_fetchingStats = 1;
}

void __cdecl LiveStorage_FinishStatsFetch()
{
    if ( !s_fetchingStats
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_storage_win.cpp",
                    3792,
                    0,
                    "%s",
                    "true == s_fetchingStats") )
    {
        __debugbreak();
    }
    s_fetchingStats = 0;
}

bool __cdecl LiveStorage_FirstTimeRunning()
{
    return s_firstTimeRunning;
}

void __cdecl LiveStorage_SetFirstTimeRunning(bool running)
{
    s_firstTimeRunning = running;
}

void __cdecl LiveStorage_SaveRecentServersComplete(int controllerindex, void *data)
{
    if ( !data
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_storage_win.cpp", 3812, 0, "%s", "data") )
    {
        __debugbreak();
    }
    SV_ResetFileOp((dwFileOperationInfo* )data);
}

TaskRecord *__cdecl LiveStorage_SaveRecentServers(unsigned __int8 *buffer, int buffSize)
{
#ifdef KISAK_LIVE_STUBS
    TaskRecord *nestedTask; // [esp+0h] [ebp-10h]
    dwFileOperationInfo *fileInfo; // [esp+Ch] [ebp-4h]

    if ( TaskManager2_TaskIsInProgressForController(task_saveRecentServers, 0) )
        return 0;
    fileInfo = SV_GetFreeFileOp();
    if ( fileInfo )
    {
        fileInfo->retryInfo.lastAttemptTime = 0;
        fileInfo->retryInfo.lastAttemptInterval = 0;
        fileInfo->retryInfo.retryCount = 0;
        fileInfo->isUserFile = 1;
        fileInfo->isCompressedFile = 0;
        fileInfo->fileTask.m_filename = "recentservers.dat";
        fileInfo->fileBuffer = buffer;
        fileInfo->bufferSize = buffSize;
        fileInfo->fileTask.m_optional = 1;
        fileInfo->fileOperationSucessFunction = (void (__cdecl *)(const int, void *))LiveStorage_SaveRecentServersComplete;
        nestedTask = LiveStorage_WriteDWUserFile(0, fileInfo, 0);
        return LiveStorage_SetupNestedTask(task_saveRecentServers, 0, nestedTask, fileInfo);
    }
    else
    {
        Com_DPrintf(15, "Warning: ran out of client fileops. This is bad. Ask Ewan.\n");
        return 0;
    }
#else
    return 0;
#endif
}

TaskRecord *__cdecl LiveStorage_ReadRecentServers(unsigned __int8 *buf, int bufsize)
{
#ifdef KISAK_LIVE_STUBS
    TaskRecord *nestedTask; // [esp+0h] [ebp-10h]
    dwFileOperationInfo *fileInfo; // [esp+Ch] [ebp-4h]

    if ( TaskManager2_TaskIsInProgressForController(task_readRecentServers, 0) )
        return 0;
    fileInfo = SV_GetFreeFileOp();
    if ( fileInfo )
    {
        fileInfo->retryInfo.lastAttemptTime = 0;
        fileInfo->retryInfo.lastAttemptInterval = 0;
        fileInfo->retryInfo.retryCount = 0;
        fileInfo->isUserFile = 1;
        fileInfo->isCompressedFile = 0;
        fileInfo->fileTask.m_filename = "recentservers.dat";
        fileInfo->fileBuffer = buf;
        fileInfo->bufferSize = bufsize;
        fileInfo->fileTask.m_optional = 1;
        fileInfo->fileOperationSucessFunction = (void (__cdecl *)(const int, void *))LiveStorage_ReadRecentServersSuccess;
        nestedTask = LiveStorage_ReadDWFile(0, fileInfo);
        return LiveStorage_SetupNestedTask(task_readRecentServers, 0, nestedTask, fileInfo);
    }
    else
    {
        Com_DPrintf(15, "Warning: ran out of client fileops. This is bad. Ask Ewan.\n");
        return 0;
    }
#else
    return 0;
#endif
}

void __cdecl LiveStorage_ReadRecentServersSuccess(int controllerIndex, void *data)
{
    Live_PopulateRecentServers(*((unsigned __int8 **)data + 62), *((unsigned int *)data + 63));
}

void __cdecl LiveStorage_DeleteGlobalStats(int controllerIndex)
{
    dwFileOperationInfo *fileInfo; // [esp+0h] [ebp-4h]

    fileInfo = SV_GetFreeFileOp();
    if ( !fileInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_storage_win.cpp", 3910, 0, "%s", "fileInfo") )
    {
        __debugbreak();
    }
    fileInfo->fileTask.m_filename = (char*)"globalstatsCompressed";
    fileInfo->isUserFile = 1;
    fileInfo->fileOperationSucessFunction = (void (__cdecl *)(const int, void *))LiveStorage_DeleteGlobalStatsSuccess;
    fileInfo->fileOperationFailureFunction = (void (__cdecl *)(const int, void *))LiveStorage_DeleteGlobalStatsNotFound;
    LiveStorage_DeleteDWUserFile(controllerIndex, fileInfo);
}

void __cdecl LiveStorage_DeleteGlobalStatsSuccess()
{
    LiveStats_ResetStats(0, 1);
    LiveStats_ResetBasicTrainingStats(0);
}

void __cdecl LiveStorage_DeleteGlobalStatsNotFound()
{
    LiveStats_ResetStats(0, 0);
    LiveStats_ResetBasicTrainingStats(0);
}