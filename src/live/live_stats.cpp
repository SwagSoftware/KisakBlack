#include "live_stats.h"
#include <universal/assertive.h>
#include "live_storage.h"
#include "live_win.h"
#include <qcommon/com_gamemodes.h>
#include <ui_mp/ui_gametype_custom_mp.h>
#include <qcommon/md4.h>
#include <client/cl_rank.h>
#include <qcommon/com_clients.h>
#include <cgame/cg_draw_names.h>
#include <bgame/bg_unlockable_items.h>
#include "live_contracts.h"
#include "live_counter.h"
#include <DW/MatchRecorder.h>
#include <cgame/cg_compass.h>
#include <ui_mp/ui_gametype_variants_mp.h>
#include <client_mp/cl_main_pc_mp.h>
#include <server_mp/sv_main_mp.h>
#include "live_storage_win.h"
#include <ddl/ddl_cmd.h>

const char *lbTypeEnum_7[17] =
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

const char *playerStatKeys[40] =
{
  "RANKXP",
  "PLEVEL",
  "SCORE",
  "TIME_PLAYED_TOTAL",
  "GAMETYPEBAN",
  "TIMEWHENNEXTHOST",
  "BADHOSTCOUNT",
  "LEADERBOARDFAILURES",
  "LASTSTATSBACKUP",
  "MAPPACKMASK",
  "STATSBACKUPVERSION",
  "MP_AIRFIELD_SKIP_PASSED",
  "MAK_SKIP_PASSED",
  "CODPOINTS",
  "CURRENCYSPENT",
  "STATS_VERSION",
  "KILLS",
  "DEATHS",
  "DEATHSDURINGUSE",
  "HEADSHOTS",
  "KDRATIO",
  "HIGHEST_KDRATIO",
  "ACCURACY",
  "HIGHEST_ACCURACY",
  "SHOTS",
  "HITS",
  "TIMEUSED",
  "USED",
  "DESTROYED",
  "GAMETYPE",
  "WINS",
  "LOSSES",
  "TIES",
  "LIFETIME_EARNINGS",
  "LIFETIME_BUYIN",
  "CONTRACTS_PURCHASED",
  "CONTRACTS_COMPLETED",
  "LAST_ESCROW",
  "WEEKLY_TIMESTAMP",
  "MONTHLY_TIMESTAMP"
};



ddlDef_t *g_statsDDL;

const dvar_t *ui_challengeSort;
const dvar_t *debugStats;
const dvar_t *ui_numPersonalBests;
const dvar_t *recordPointsSpent;
const dvar_t *ui_challengeGameMode;

ddlState_t g_statsRootState;

bool s_stableStatsBufferInitialized[1];

unsigned int s_numSortedChallenges;
unsigned int s_numSortedGlobalChallenges;
unsigned int s_numSortedWeaponGroup;
unsigned int s_numSortedAttachments;
challengeInfo_t s_sortedChallengeList[1000];
challengeInfo_t s_sortedWeaponGroupList[16];
challengeInfo_t s_sortedAttachmentsList[24];
challengeInfo_t s_sortedGlobalChallengeList[50];
unsigned int s_currentPersonalBest[1];
personalBest_t s_personalBests[1][1000];
unsigned int s_numRecentlyUnlockedItems[1];
int s_recentlyUnlockedItems[1][256];
unsigned int s_totalNumberOfUnlockedItems;
ddlState_t g_statsCacState;
ddlState_t g_statsRankXPState;
bool s_statsBufferInitialised[1];
ddlState_t g_statsActiveContractsState;
ddlState_t g_statsContractsState;

int __cdecl LiveStats_GetDDLHeaderVersion(unsigned __int8 *statsBuffer)
{
    if ( !g_statsDDL
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp", 233, 0, "%s", "g_statsDDL") )
    {
        __debugbreak();
    }
    return DDL_GetHeaderVersion((char *)statsBuffer);
}

int __cdecl LiveStats_ValidateGlobalWithDDL(int controllerIndex)
{
    char backupBuffer[40172]; // [esp+0h] [ebp-9CF8h] BYREF
    char *buffer; // [esp+9CF0h] [ebp-8h]
    int bufferSize; // [esp+9CF4h] [ebp-4h]

    if ( !g_statsDDL
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp", 245, 0, "%s", "g_statsDDL") )
    {
        __debugbreak();
    }
    buffer = (char *)LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_GLOBAL, 1);
    bufferSize = LiveStorage_GetStatsBufferSize();
    if ( DDL_AssociateBuffer(buffer, bufferSize, g_statsDDL) )
    {
        LiveStorage_SetStatsDDLValidated(controllerIndex, STATS_LOCATION_NORMAL, 1);
        return 1;
    }
    else if ( DDL_FixBufferVersion(buffer, g_statsDDL, "ddl_mp/stats.ddl", backupBuffer, 40168)
                 || DDL_FixBufferVersion(buffer, g_statsDDL, "ddl_mp/stats_archive.ddl", backupBuffer, 40168) )
    {
        DDL_NoCheckPrintWarning(
            "DDL: Stats buffer updated to version %d for controller index %d.\n",
            g_statsDDL->version,
            controllerIndex);
        LiveStorage_SetStatsDDLValidated(controllerIndex, STATS_LOCATION_GLOBAL, 1);
        return 1;
    }
    else
    {
        return 0;
    }
}

int __cdecl LiveStats_CanPerformStatOperation(int controllerIndex)
{
    if ( !Live_IsUserSignedInToLive() || !Live_IsUserSignedInToDemonware(controllerIndex) )
        return 0;
    if ( LiveStorage_DoWeHaveCurrentStats(controllerIndex) )
    {
        if ( LiveStorage_AreStatsDDLValidated(controllerIndex, STATS_LOCATION_NORMAL)
            || LiveStorage_ValidateWithDDL(controllerIndex, STATS_LOCATION_NORMAL) )
        {
            return 1;
        }
        else
        {
            DDL_PrintError("DDL: Error validating stats buffer for controller %d.\n", controllerIndex);
            return 0;
        }
    }
    else
    {
        Com_PrintError(16, "Tried to perform a stats operation before the stats have been fetched.\n");
        return 0;
    }
}

unsigned int __cdecl LiveStats_GetDDLIntStat(int controllerIndex, ddlState_t *searchState)
{
    bool v3; // [esp+3h] [ebp-51h]
    persistentStats *buffer; // [esp+8h] [ebp-4Ch]

    if ( controllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    333,
                    0,
                    "%s\n\t(controllerIndex) = %i",
                    "(controllerIndex >= 0 && controllerIndex < 1)",
                    controllerIndex) )
    {
        __debugbreak();
    }
    if ( !LiveStats_CanPerformStatOperation(controllerIndex) )
        return 0;
    buffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
    v3 = xblive_basictraining && xblive_basictraining->current.enabled
        || LiveStats_GetBasicTrainingState(buffer->statsBuffer);
    if ( !v3 && searchState->member->permission == 2 )
        buffer = LiveStorage_GetStatsBuffer(0, STATS_LOCATION_GLOBAL, 1);
    return DDL_GetInt(searchState, (char *)buffer);
}

unsigned int __cdecl LiveStats_GetDDLIntStat(int controllerIndex, const char **operands, int nOperands)
{
    int i; // [esp+0h] [ebp-14h]
    ddlState_t searchState; // [esp+4h] [ebp-10h] BYREF

    if ( Dvar_GetInt(ui_useCustomClassInfo) )
        return GetCustomClassLoadoutItemForClassName(controllerIndex, operands[1], operands[2]);
    searchState = g_statsRootState;
    for ( i = 0; i < nOperands; ++i )
    {
        if ( searchState.member && searchState.member->arraySize > 1 && searchState.member->enumIndex == -1 )
        {
            DDL_PrintError("DDL: array index (integer) expected. Received a string instead.");
            return -1;
        }
        if ( !DDL_MoveToName(&searchState, &searchState, operands[i]) )
        {
            DDL_PrintError("DDL: Could not find member name.");
            return -1;
        }
    }
    if ( !searchState.member
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp", 381, 0, "%s", "searchState.member") )
    {
        __debugbreak();
    }
    return LiveStats_GetDDLIntStat(controllerIndex, &searchState);
}

unsigned int __cdecl LiveStats_GetDIntStat(int controllerIndex, ddlState_t *searchState)
{
    bool v3; // [esp+3h] [ebp-9h]
    persistentStats *buffer; // [esp+8h] [ebp-4h]

    if ( controllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    441,
                    0,
                    "%s\n\t(controllerIndex) = %i",
                    "(controllerIndex >= 0 && controllerIndex < 1)",
                    controllerIndex) )
    {
        __debugbreak();
    }
    if ( !LiveStats_CanPerformStatOperation(controllerIndex) )
        return 0;
    buffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
    v3 = xblive_basictraining && xblive_basictraining->current.enabled
        || LiveStats_GetBasicTrainingState(buffer->statsBuffer);
    if ( !v3 && searchState->member->permission == 2 )
        buffer = LiveStorage_GetStatsBuffer(0, STATS_LOCATION_GLOBAL, 1);
    return DDL_GetInt(searchState, (char *)buffer);
}

unsigned int __cdecl LiveStats_GetDIntStatFromBase(
                int controllerIndex,
                ddlState_t *searchState,
                persistentStats *buffer)
{
    bool v4; // [esp+3h] [ebp-5h]

    if ( controllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    461,
                    0,
                    "%s\n\t(controllerIndex) = %i",
                    "(controllerIndex >= 0 && controllerIndex < 1)",
                    controllerIndex) )
    {
        __debugbreak();
    }
    if ( !LiveStats_CanPerformStatOperation(controllerIndex) )
        return 0;
    v4 = xblive_basictraining && xblive_basictraining->current.enabled
        || LiveStats_GetBasicTrainingState(buffer->statsBuffer);
    if ( !v4 && searchState->member->permission == 2 )
        buffer = LiveStorage_GetStatsBuffer(0, STATS_LOCATION_GLOBAL, 1);
    return DDL_GetInt(searchState, (char *)buffer);
}

unsigned int __cdecl LiveStats_GetDInt64Stat(int controllerIndex, ddlState_t *searchState)
{
    bool v3; // [esp+3h] [ebp-9h]
    persistentStats *buffer; // [esp+8h] [ebp-4h]

    if ( controllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    481,
                    0,
                    "%s\n\t(controllerIndex) = %i",
                    "(controllerIndex >= 0 && controllerIndex < 1)",
                    controllerIndex) )
    {
        __debugbreak();
    }
    if ( !LiveStats_CanPerformStatOperation(controllerIndex) )
        return 0;
    buffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
    v3 = xblive_basictraining && xblive_basictraining->current.enabled
        || LiveStats_GetBasicTrainingState(buffer->statsBuffer);
    if ( !v3 && searchState->member->permission == 2 )
        buffer = LiveStorage_GetStatsBuffer(0, STATS_LOCATION_GLOBAL, 1);
    return DDL_GetInt64(searchState, (char *)buffer);
}

char *__cdecl LiveStats_GetDStringStat(int controllerIndex, ddlState_t *searchState)
{
    bool v3; // [esp+3h] [ebp-9h]
    persistentStats *buffer; // [esp+8h] [ebp-4h]

    if ( controllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    522,
                    0,
                    "%s\n\t(controllerIndex) = %i",
                    "(controllerIndex >= 0 && controllerIndex < 1)",
                    controllerIndex) )
    {
        __debugbreak();
    }
    if ( !LiveStats_CanPerformStatOperation(controllerIndex) )
        return 0;
    buffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
    v3 = xblive_basictraining && xblive_basictraining->current.enabled
        || LiveStats_GetBasicTrainingState(buffer->statsBuffer);
    if ( !v3 && searchState->member->permission == 2 )
        buffer = LiveStorage_GetStatsBuffer(0, STATS_LOCATION_GLOBAL, 1);
    return DDL_GetString(searchState, (char *)buffer);
}

char __cdecl LiveStats_GetIntPlayerStatInternal(int *outInt, int pathDepth, const char **statName, char *buffer)
{
    ddlResult_t result; // [esp+8h] [ebp-58h] BYREF
    ddlState_t searchState; // [esp+50h] [ebp-10h] BYREF

    if ( !outInt && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp", 558, 0, "%s", "outInt") )
        __debugbreak();
    if ( !statName
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp", 559, 0, "%s", "statName") )
    {
        __debugbreak();
    }
    if ( !*statName
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp", 560, 0, "%s", "statName[ 0 ]") )
    {
        __debugbreak();
    }
    if ( !buffer && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp", 561, 0, "%s", "buffer") )
        __debugbreak();
    if ( pathDepth > 8
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    562,
                    0,
                    "%s",
                    "pathDepth <= DDL_MAX_PATH_DEPTH") )
    {
        __debugbreak();
    }
    if ( !outInt || !statName || !*statName || pathDepth > 8 )
        goto LABEL_27;
    if ( !DDL_MoveToPath(&g_statsRootState, &searchState, pathDepth, statName) )
    {
        DDL_PrintError("DDL:Error getting player stat value for %s.\n", (const char *)statName);
        return 0;
    }
    if ( !xblive_basictraining || !xblive_basictraining->current.enabled )
        LiveStats_GetBasicTrainingState((unsigned __int8 *)buffer);
    if ( DDL_GetValue(&searchState, &result, buffer) )
    {
        *outInt = result.intValue;
        return 1;
    }
    else
    {
LABEL_27:
        DDL_PrintError("DDL:Error getting player stat value for %s.\n", (const char *)statName);
        return 0;
    }
}

char __cdecl LiveStats_GetIntPlayerStatFromBase(int *outInt, const char *statName, char *base)
{
    const char *path[2]; // [esp+0h] [ebp-8h] BYREF

    path[0] = "PlayerStatsList";
    path[1] = statName;
    return LiveStats_GetIntPlayerStatInternal(outInt, 2, path, base);
}

char __cdecl LiveStats_GetIntPlayerStatFromForcedBase(int *outInt, const char *statName, char *base)
{
    const char *path[2]; // [esp+0h] [ebp-60h] BYREF
    ddlResult_t result; // [esp+8h] [ebp-58h] BYREF
    ddlState_t searchState; // [esp+50h] [ebp-10h] BYREF

    path[0] = "PlayerStatsList";
    path[1] = statName;
    if ( DDL_MoveToPath(&g_statsRootState, &searchState, 2, path) )
    {
        if ( DDL_GetValue(&searchState, &result, base) )
        {
            *outInt = result.intValue;
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        DDL_PrintError("DDL:Error getting player stat value for %s.\n", statName);
        return 0;
    }
}

unsigned int __cdecl LiveStats_GetDIntStatFromForcedBase(int controllerIndex, ddlState_t *searchState, char *buffer)
{
    if ( controllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    635,
                    0,
                    "%s\n\t(controllerIndex) = %i",
                    "(controllerIndex >= 0 && controllerIndex < 1)",
                    controllerIndex) )
    {
        __debugbreak();
    }
    if ( LiveStats_CanPerformStatOperation(controllerIndex) )
        return DDL_GetInt(searchState, buffer);
    else
        return 0;
}

unsigned int __cdecl LiveStats_GetDInt64StatFromForcedBase(int controllerIndex, ddlState_t *searchState, char *buffer)
{
    if ( controllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    647,
                    0,
                    "%s\n\t(controllerIndex) = %i",
                    "(controllerIndex >= 0 && controllerIndex < 1)",
                    controllerIndex) )
    {
        __debugbreak();
    }
    if ( LiveStats_CanPerformStatOperation(controllerIndex) )
        return DDL_GetInt64(searchState, buffer);
    else
        return 0;
}

char *__cdecl LiveStats_GetDStringStatFromForcedBase(int controllerIndex, ddlState_t *searchState, char *buffer)
{
    if ( controllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    659,
                    0,
                    "%s\n\t(controllerIndex) = %i",
                    "(controllerIndex >= 0 && controllerIndex < 1)",
                    controllerIndex) )
    {
        __debugbreak();
    }
    if ( LiveStats_CanPerformStatOperation(controllerIndex) )
        return DDL_GetString(searchState, buffer);
    else
        return 0;
}

char __cdecl LiveStats_GetIntPlayerStat(int controllerIndex, int *outInt, const char *statName)
{
    persistentStats *buffer; // [esp+0h] [ebp-4h]

    buffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
    return LiveStats_GetIntPlayerStatFromBase(outInt, statName, (char *)buffer);
}

char __cdecl LiveStats_GetIntPlayerStatByGameMode(
                int controllerIndex,
                int *outInt,
                const char *gameMode,
                const char *statName)
{
    persistentStats *buffer; // [esp+0h] [ebp-10h]
    const char *path[3]; // [esp+4h] [ebp-Ch] BYREF

    buffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
    path[0] = "PlayerStatsByGameMode";
    path[1] = gameMode;
    path[2] = statName;
    return LiveStats_GetIntPlayerStatInternal(outInt, 3, path, (char *)buffer);
}

char __cdecl LiveStats_GetIntPlayerStatByGameModeFromBase(
                int controllerIndex,
                int *outInt,
                const char *gameMode,
                const char *statName,
                char *base)
{
    const char *path[3]; // [esp+0h] [ebp-Ch] BYREF

    path[0] = "PlayerStatsByGameMode";
    path[1] = gameMode;
    path[2] = statName;
    return LiveStats_GetIntPlayerStatInternal(outInt, 3, path, base);
}

char __cdecl LiveStats_GetIntOtherPlayerStat(int controllerIndex, int *outInt, const char *statName)
{
    persistentStats *buffer; // [esp+0h] [ebp-4h]

    buffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_OTHERPLAYER, 1);
    return LiveStats_GetIntPlayerStatFromBase(outInt, statName, (char *)buffer);
}

char __cdecl LiveStats_GetIntOtherPlayerStatByGameMode(
                int controllerIndex,
                int *outInt,
                const char *gameMode,
                const char *statName)
{
    persistentStats *buffer; // [esp+0h] [ebp-10h]
    const char *path[3]; // [esp+4h] [ebp-Ch] BYREF

    buffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_OTHERPLAYER, 1);
    path[0] = "PlayerStatsByGameMode";
    path[1] = gameMode;
    path[2] = statName;
    return LiveStats_GetIntPlayerStatInternal(outInt, 3, path, (char *)buffer);
}

char __cdecl LiveStats_GetIntPlayerStatByKey(int controllerIndex, int *outInt, playerStatsKeyIndex_t keyIndex)
{
    return LiveStats_GetIntPlayerStat(controllerIndex, outInt, playerStatKeys[keyIndex]);
}

char __cdecl LiveStats_GetIntOtherPlayerStatByKey(int controllerIndex, int *outInt, playerStatsKeyIndex_t keyIndex)
{
    return LiveStats_GetIntOtherPlayerStat(controllerIndex, outInt, playerStatKeys[keyIndex]);
}

const char *__cdecl LiveStats_GetPlayerStatStringByKey(playerStatsKeyIndex_t keyIndex)
{
    return playerStatKeys[keyIndex];
}

char __cdecl LiveStats_SetIntPlayerStatInternal(
                int controllerIndex,
                int pathDepth,
                const char **path,
                char *buffer,
                unsigned int statValue)
{
    ddlState_t searchState; // [esp+8h] [ebp-10h] BYREF

    if ( !path && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp", 765, 0, "%s", "path") )
        __debugbreak();
    if ( !*path && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp", 766, 0, "%s", "path[ 0 ]") )
        __debugbreak();
    if ( !buffer && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp", 767, 0, "%s", "buffer") )
        __debugbreak();
    if ( pathDepth > 8
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    768,
                    0,
                    "%s",
                    "pathDepth <= DDL_MAX_PATH_DEPTH") )
    {
        __debugbreak();
    }
    if ( path && *path && pathDepth <= 8 && DDL_MoveToPath(&g_statsRootState, &searchState, pathDepth, path) )
    {
        if ( !xblive_basictraining || !xblive_basictraining->current.enabled )
            LiveStats_GetBasicTrainingState((unsigned __int8 *)buffer);
        if ( DDL_SetInt(&searchState, statValue, buffer) )
            return 1;
    }
    DDL_PrintError("DDL:Error setting player stat value for %s.\n", path[pathDepth - 1]);
    return 0;
}

bool __cdecl LiveStats_WriteVersionToBuffer(char *buffer)
{
    const char *path[2]; // [esp+4h] [ebp-1Ch] BYREF
    int pathDepth; // [esp+Ch] [ebp-14h]
    ddlState_t searchState; // [esp+10h] [ebp-10h] BYREF

    path[0] = "PlayerStatsList";
    path[1] = "STATS_VERSION";
    pathDepth = 2;
    if ( DDL_MoveToPath(&g_statsRootState, &searchState, 2, path) )
        return DDL_SetInt(&searchState, stat_version->current.unsignedInt, buffer);
    DDL_PrintError("DDL:Error setting player stat value for %s.\n", path[pathDepth - 1]);
    return 0;
}

unsigned int __cdecl LiveStats_ReadVersionFromBuffer(char *buffer)
{
    const char *path[2]; // [esp+4h] [ebp-1Ch] BYREF
    int pathDepth; // [esp+Ch] [ebp-14h]
    ddlState_t searchState; // [esp+10h] [ebp-10h] BYREF

    path[0] = "PlayerStatsList";
    path[1] = "STATS_VERSION";
    pathDepth = 2;
    if ( DDL_MoveToPath(&g_statsRootState, &searchState, 2, path) )
        return DDL_GetInt(&searchState, buffer);
    DDL_PrintError("DDL:Error setting player stat value for %s.\n", path[pathDepth - 1]);
    return 0;
}

char __cdecl LiveStats_GetIntPlayerStatMatchDeltaByGameMode(
                int controllerIndex,
                int *outInt,
                const char *gameMode,
                const char *statName)
{
    persistentStats *liveStatsBuffer; // [esp+0h] [ebp-10h]
    persistentStats *oldStatsBuffer; // [esp+4h] [ebp-Ch]
    int valueBeforeMatch; // [esp+8h] [ebp-8h] BYREF
    int valueAfterMatch; // [esp+Ch] [ebp-4h] BYREF

    liveStatsBuffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
    oldStatsBuffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_STABLE, 1);
    if ( LiveStats_GetIntPlayerStatByGameModeFromBase(
                 controllerIndex,
                 &valueBeforeMatch,
                 gameMode,
                 statName,
                 (char *)oldStatsBuffer)
        && LiveStats_GetIntPlayerStatByGameModeFromBase(
                 controllerIndex,
                 &valueAfterMatch,
                 gameMode,
                 statName,
                 (char *)liveStatsBuffer) )
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

char __cdecl LiveStats_SetIntPlayerStat(int controllerIndex, const char *statName, unsigned int statValue)
{
    persistentStats *buffer; // [esp+0h] [ebp-Ch]
    const char *path[2]; // [esp+4h] [ebp-8h] BYREF

    buffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
    path[0] = "PlayerStatsList";
    path[1] = statName;
    return LiveStats_SetIntPlayerStatInternal(controllerIndex, 2, path, (char *)buffer, statValue);
}

char __cdecl LiveStats_GetIntPlayerBackupStatByKey(int controllerIndex, int *outInt, playerStatsKeyIndex_t keyIndex)
{
    persistentStats *StatsBuffer; // eax

    StatsBuffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_BACKUP, 1);
    return LiveStats_GetIntPlayerStatFromBase(outInt, playerStatKeys[keyIndex], (char *)StatsBuffer);
}

int __cdecl LiveStats_ChecksumGamerStats(unsigned __int8 *buffer, int len)
{
    return Com_BlockChecksumKey32(buffer, len, 0);
}

void __cdecl LiveStats_SetDInt64Stat(int controllerIndex, ddlState_t *searchState, unsigned __int64 value)
{
    bool v3; // [esp+3h] [ebp-9h]
    persistentStats *buffer; // [esp+8h] [ebp-4h]

    if ( controllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    1048,
                    0,
                    "%s\n\t(controllerIndex) = %i",
                    "(controllerIndex >= 0 && controllerIndex < 1)",
                    controllerIndex) )
    {
        __debugbreak();
    }
    if ( LiveStats_CanPerformStatOperation(controllerIndex) )
    {
        buffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
        v3 = xblive_basictraining && xblive_basictraining->current.enabled
            || LiveStats_GetBasicTrainingState(buffer->statsBuffer);
        if ( !v3 && searchState->member->permission == 2 )
            buffer = LiveStorage_GetStatsBuffer(0, STATS_LOCATION_GLOBAL, 1);
        if ( !DDL_SetInt64(searchState, value, (char *)buffer) )
            DDL_PrintError("DDL: Could not set stat '%s'", searchState->member->name);
    }
}

int __cdecl LiveStats_HexToDec(const char *hexValue, int hexLength)
{
    int i; // [esp+0h] [ebp-Ch]
    int result; // [esp+4h] [ebp-8h]
    int cumulativeResult; // [esp+8h] [ebp-4h]

    cumulativeResult = 0;
    for ( i = 0; i < hexLength; ++i )
    {
        if ( hexValue[i] > 57 )
            result = hexValue[i] - 55;
        else
            result = hexValue[i] - 48;
        cumulativeResult += result * (1 << (4 * (hexLength - 1 - i)));
    }
    return cumulativeResult;
}

void __cdecl LiveStats_ProcessStatChangedData(
                int controllerIndex,
                char *buffer,
                int bufferSize,
                int offset,
                int size,
                const char *data)
{
    int i; // [esp+0h] [ebp-58h]
    char tempResult; // [esp+4h] [ebp-54h]
    int byteCount; // [esp+8h] [ebp-50h]
    char binaryData[68]; // [esp+10h] [ebp-48h] BYREF

    byteCount = 0;
    memset(binaryData, 0, 0x40u);
    for ( i = 0; i < 2 * size; ++i )
    {
        if ( data[i] > 57 )
            tempResult = data[i] - 55;
        else
            tempResult = data[i] - 48;
        if ( i % 2 )
        {
            binaryData[byteCount] *= 16;
            binaryData[byteCount++] |= tempResult & 0xF;
        }
        else
        {
            binaryData[byteCount] = 0;
            binaryData[byteCount] = tempResult & 0xF;
        }
    }
    DDL_SetRawBytes(buffer, offset, size, binaryData);
}

void __cdecl LiveStats_SetStatChanged(int controllerIndex, const char *hexMsg)
{
    unsigned int i; // [esp+10h] [ebp-60h]
    unsigned int runCount; // [esp+14h] [ebp-5Ch]
    persistentStats *buffer; // [esp+18h] [ebp-58h]
    unsigned int startOffset; // [esp+1Ch] [ebp-54h]
    char statChangeCommand[68]; // [esp+20h] [ebp-50h] BYREF
    unsigned int size; // [esp+68h] [ebp-8h]
    bool sizeFound; // [esp+6Eh] [ebp-2h]
    bool offsetFound; // [esp+6Fh] [ebp-1h]

    if ( controllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    1157,
                    0,
                    "%s\n\t(controllerIndex) = %i",
                    "(controllerIndex >= 0 && controllerIndex < 1)",
                    controllerIndex) )
    {
        __debugbreak();
    }
    if ( LiveStats_CanPerformStatOperation(controllerIndex) )
    {
        buffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_GLOBAL, 1);
        offsetFound = 0;
        sizeFound = 0;
        size = 0;
        startOffset = 0;
        i = 0;
        runCount = 0;
        while ( i < strlen(hexMsg) )
        {
            statChangeCommand[runCount] = hexMsg[i];
            if ( sizeFound && runCount == 2 * size - 1 )
            {
                LiveStats_ProcessStatChangedData(controllerIndex, (char *)buffer, 40168, startOffset, size, statChangeCommand);
                runCount = 0;
                offsetFound = 0;
                sizeFound = 0;
            }
            else if ( runCount != 3 || offsetFound || sizeFound )
            {
                if ( runCount || !offsetFound || sizeFound )
                {
                    ++runCount;
                }
                else
                {
                    size = LiveStats_HexToDec(statChangeCommand, 1) + 1;
                    sizeFound = 1;
                    runCount = 0;
                }
            }
            else
            {
                startOffset = LiveStats_HexToDec(statChangeCommand, 4);
                offsetFound = 1;
                runCount = 0;
            }
            ++i;
        }
    }
}

void __cdecl LiveStats_SetStatByKey(int controllerIndex, playerStatsKeyIndex_t keyIndex, int value)
{
    int returnVal; // [esp+0h] [ebp-4h]

    if ( (unsigned int)keyIndex >= MP_PLAYERSTATSKEY_COUNT
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    1208,
                    0,
                    "keyIndex doesn't index MP_PLAYERSTATSKEY_COUNT\n\t%i not in [0, %i)",
                    keyIndex,
                    40) )
    {
        __debugbreak();
    }
    returnVal = 0;
    if ( keyIndex < MP_PLAYERSTATSKEY_COUNT )
        returnVal = (unsigned __int8)LiveStats_SetIntPlayerStat(controllerIndex, playerStatKeys[keyIndex], value);
    if ( !returnVal
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp", 1216, 0, "%s", "returnVal") )
    {
        __debugbreak();
    }
}

void __cdecl LiveStats_MakeStableStatsBuffer(int controllerIndex)
{
    persistentStats *v1; // eax
    persistentStats *StatsBuffer; // [esp-8h] [ebp-8h]
    unsigned int StatsBufferSize; // [esp-4h] [ebp-4h]

    StatsBufferSize = LiveStorage_GetStatsBufferSize();
    StatsBuffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
    v1 = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_STABLE, 1);
    memcpy(v1->statsBuffer, StatsBuffer->statsBuffer, StatsBufferSize);
    s_stableStatsBufferInitialized[controllerIndex] = 1;
}

bool __cdecl LiveStats_IsStableStatsBufferInitialized(int controllerIndex)
{
    return s_stableStatsBufferInitialized[controllerIndex];
}

bool s_statsalreadycompared = true;
void __cdecl LiveStats_MakeStableGlobalStatsBuffer(int controllerIndex)
{
    persistentStats *v1; // eax
    persistentStats *StatsBuffer; // [esp-8h] [ebp-8h]
    unsigned int StatsBufferSize; // [esp-4h] [ebp-4h]

    s_statsalreadycompared = 0;
    StatsBufferSize = LiveStorage_GetStatsBufferSize();
    StatsBuffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_GLOBAL, 1);
    v1 = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_GLOBALSTABLE, 1);
    memcpy(v1->statsBuffer, StatsBuffer->statsBuffer, StatsBufferSize);
}

void __cdecl LiveStats_SetPlayerStatByKey(
                const char *statsList,
                playerStatsKeyIndex_t key,
                unsigned int newvalue,
                unsigned __int8 *buffer)
{
    if ( (!statsList || !buffer)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    1376,
                    0,
                    "%s",
                    "statsList && buffer") )
    {
        __debugbreak();
    }
    if ( statsList )
    {
        if ( buffer )
            LiveStats_SetPlayerStat(statsList, playerStatKeys[key], (char *)buffer, newvalue);
    }
}

void __cdecl LiveStats_SetPlayerStat(const char *statsList, const char *statName, char *buffer, unsigned int statValue)
{
    ddlState_t searchState; // [esp+0h] [ebp-10h] BYREF

    if ( !statName
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp", 1354, 0, "%s", "statName") )
    {
        __debugbreak();
    }
    if ( !buffer && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp", 1355, 0, "%s", "buffer") )
        __debugbreak();
    if ( statName && buffer )
    {
        if ( DDL_MoveTo(&g_statsRootState, &searchState, 2, statsList, statName) )
            DDL_SetInt(&searchState, statValue, buffer);
        else
            Com_PrintError(29, "DDL: Error could not find player stat %s\n", statName);
    }
}

void __cdecl LiveStats_SetItemStat(
                ddlState_t *searchStateStats,
                const char *statMember,
                char *buffer,
                unsigned int value)
{
    ddlState_t searchState; // [esp+0h] [ebp-10h] BYREF

    if ( !searchStateStats
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp", 1417, 0, "%s", "searchStateStats") )
    {
        __debugbreak();
    }
    if ( !statMember
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp", 1418, 0, "%s", "statMember") )
    {
        __debugbreak();
    }
    if ( !buffer && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp", 1419, 0, "%s", "buffer") )
        __debugbreak();
    if ( searchStateStats && statMember && buffer )
    {
        if ( !DDL_MoveTo(searchStateStats, &searchState, 1, statMember) )
            Com_PrintError(29, "DDL: Error could not find member %s\n", statMember);
        DDL_SetInt(&searchState, value, buffer);
    }
}

int __cdecl LiveStats_GetMilestoneRowNum(
                XAssetHeader *statsMilestonestable,
                int currentMilestone,
                const char *statType,
                const char *statName)
{
    const char *v4; // eax
    const char *v5; // eax
    int rowNum; // [esp+0h] [ebp-4h]

    v4 = va("%d", currentMilestone);
    rowNum = CL_GetMilestoneRowNum(statsMilestonestable, 1, v4, 3, statType, 4, statName);
    if ( rowNum != -1 || !currentMilestone )
        return rowNum;
    v5 = va("%d", currentMilestone - 1);
    return CL_GetMilestoneRowNum(statsMilestonestable, 1, v5, 3, statType, 4, statName);
}

void __cdecl LiveStats_CheckCurrentValue(
                const StringTable *statsMilestonestable,
                int rowNum,
                int targetColumn,
                int *currentValue)
{
    const char *ColumnValueForRow; // eax
    int targetValue; // [esp+0h] [ebp-4h]

    ColumnValueForRow = StringTable_GetColumnValueForRow(statsMilestonestable, rowNum, targetColumn);
    targetValue = atoi(ColumnValueForRow);
    if ( *currentValue >= targetValue )
        *currentValue = targetValue;
}

int __cdecl LiveStats_GetTotalMatchesPlayed(int controllerIndex)
{
    int gameType; // [esp+0h] [ebp-14h]
    int losses; // [esp+4h] [ebp-10h] BYREF
    int wins; // [esp+8h] [ebp-Ch] BYREF
    int numMatches; // [esp+Ch] [ebp-8h]
    int ties; // [esp+10h] [ebp-4h] BYREF

    numMatches = 0;
    wins = 0;
    losses = 0;
    ties = 0;
    for ( gameType = 0; gameType < 16; ++gameType )
    {
        LiveStats_GetIntPlayerStatByGameMode(controllerIndex, &wins, lbTypeEnum_7[gameType], playerStatKeys[30]);
        LiveStats_GetIntPlayerStatByGameMode(controllerIndex, &losses, lbTypeEnum_7[gameType], playerStatKeys[31]);
        LiveStats_GetIntPlayerStatByGameMode(controllerIndex, &ties, lbTypeEnum_7[gameType], playerStatKeys[32]);
        numMatches += ties + losses + wins;
    }
    return numMatches;
}

int __cdecl LiveStats_GetTotalMatchesPlayedByGameMode(int controllerIndex, const char *gameModePrefix)
{
    int losses; // [esp+0h] [ebp-10h] BYREF
    int wins; // [esp+4h] [ebp-Ch] BYREF
    int numMatches; // [esp+8h] [ebp-8h]
    int ties; // [esp+Ch] [ebp-4h] BYREF

    numMatches = 0;
    wins = 0;
    losses = 0;
    ties = 0;
    LiveStats_GetIntPlayerStatByGameMode(controllerIndex, &wins, gameModePrefix, playerStatKeys[30]);
    LiveStats_GetIntPlayerStatByGameMode(controllerIndex, &losses, gameModePrefix, playerStatKeys[31]);
    LiveStats_GetIntPlayerStatByGameMode(controllerIndex, &ties, gameModePrefix, playerStatKeys[32]);
    return numMatches + ties + losses + wins;
}

unsigned int s_currentStatsMilestone[1];
unsigned int __cdecl LiveStats_GetNumStatsMilestones(int controllerIndex)
{
    if ( controllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    2241,
                    0,
                    "controllerIndex doesn't index MAX_GPAD_COUNT\n\t%i not in [0, %i)",
                    controllerIndex,
                    1) )
    {
        __debugbreak();
    }
    return s_currentStatsMilestone[controllerIndex];
}

unsigned int __cdecl LiveStats_GetStatsMilestoneTier(int controllerIndex, unsigned int statsMilestoneNum)
{
    statsMilestone_t *statsMilestone; // [esp+0h] [ebp-4h]

    if ( controllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    2248,
                    0,
                    "controllerIndex doesn't index MAX_GPAD_COUNT\n\t%i not in [0, %i)",
                    controllerIndex,
                    1) )
    {
        __debugbreak();
    }
    statsMilestone = LiveStats_GetStatsMilestone(controllerIndex, statsMilestoneNum);
    if ( statsMilestone )
        return statsMilestone->milestone;
    else
        return 0;
}

statsMilestone_t s_statsMilestonesCompleted[1][1000];
statsMilestone_t *__cdecl LiveStats_GetStatsMilestone(int controllerIndex, unsigned int statsMilestoneNum)
{
    statsMilestone_t *statsMilestone; // [esp+0h] [ebp-4h]

    if ( controllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    2228,
                    0,
                    "controllerIndex doesn't index MAX_GPAD_COUNT\n\t%i not in [0, %i)",
                    controllerIndex,
                    1) )
    {
        __debugbreak();
    }
    statsMilestone = 0;
    if ( statsMilestoneNum < s_currentStatsMilestone[controllerIndex] )
        return &s_statsMilestonesCompleted[controllerIndex][statsMilestoneNum];
    return statsMilestone;
}

int __cdecl LiveStats_GetStatsMilestoneItemIndex(int controllerIndex, unsigned int statsMilestoneNum)
{
    statsMilestone_t *statsMilestone; // [esp+0h] [ebp-4h]

    if ( controllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    2262,
                    0,
                    "controllerIndex doesn't index MAX_GPAD_COUNT\n\t%i not in [0, %i)",
                    controllerIndex,
                    1) )
    {
        __debugbreak();
    }
    statsMilestone = LiveStats_GetStatsMilestone(controllerIndex, statsMilestoneNum);
    if ( statsMilestone )
        return statsMilestone->index;
    else
        return -1;
}

const char *__cdecl LiveStats_GetStatsMilestoneName(int controllerIndex, unsigned int statsMilestoneNum)
{
    statsMilestone_t *statsMilestone; // [esp+0h] [ebp-4h]

    if ( controllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    2276,
                    0,
                    "controllerIndex doesn't index MAX_GPAD_COUNT\n\t%i not in [0, %i)",
                    controllerIndex,
                    1) )
    {
        __debugbreak();
    }
    statsMilestone = LiveStats_GetStatsMilestone(controllerIndex, statsMilestoneNum);
    if ( statsMilestone )
        return statsMilestone->name;
    else
        return 0;
}

int __cdecl LiveStats_GetStatsMilestoneType(int controllerIndex, unsigned int statsMilestoneNum)
{
    statsMilestone_t *statsMilestone; // [esp+0h] [ebp-4h]

    if ( controllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    2290,
                    0,
                    "controllerIndex doesn't index MAX_GPAD_COUNT\n\t%i not in [0, %i)",
                    controllerIndex,
                    1) )
    {
        __debugbreak();
    }
    statsMilestone = LiveStats_GetStatsMilestone(controllerIndex, statsMilestoneNum);
    if ( statsMilestone )
        return statsMilestone->type;
    else
        return -1;
}

unsigned int __cdecl LiveStats_GetChallengeCount(int type)
{
    switch ( type )
    {
        case 1:
        case 6:
        case 7:
            return s_numSortedChallenges;
        case 4:
            return s_numSortedWeaponGroup;
        case 0:
            return s_numSortedGlobalChallenges;
        case 5:
            return s_numSortedAttachments;
    }
    return 0;
}

bool __cdecl StringBeginsWith(const char *prefix, const char *name)
{
    bool result; // al
    unsigned int prefixLen; // [esp+20h] [ebp-4h]

    if ( !prefix && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp", 2318, 0, "%s", "prefix") )
        __debugbreak();
    if ( !name && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp", 2319, 0, "%s", "name") )
        __debugbreak();
    result = 0;
    if ( prefix )
    {
        if ( name )
        {
            prefixLen = strlen(prefix);
            if ( prefixLen )
            {
                if ( prefixLen <= strlen(name) && !I_strnicmp(prefix, name, prefixLen) )
                    return 1;
            }
        }
    }
    return result;
}

bool __cdecl LiveStats_GetChallengeInfo(challengeInfo_t **challenge, unsigned int challengeNum, int type)
{
    if ( type == 1 || type == 6 || type == 7 )
    {
        if ( challengeNum >= s_numSortedChallenges )
            return 0;
        *challenge = &s_sortedChallengeList[challengeNum];
    }
    else if ( type == 4 )
    {
        if ( challengeNum >= s_numSortedWeaponGroup )
            return 0;
        *challenge = &s_sortedWeaponGroupList[challengeNum];
    }
    else if ( type )
    {
        if ( type == 5 )
        {
            if ( challengeNum >= s_numSortedAttachments )
                return 0;
            *challenge = &s_sortedAttachmentsList[challengeNum];
        }
    }
    else
    {
        if ( challengeNum >= s_numSortedGlobalChallenges )
            return 0;
        *challenge = &s_sortedGlobalChallengeList[challengeNum];
    }
    return *challenge != 0;
}

bool __cdecl LiveStats_GetChallengeInfoParam(
                sortedChallengeParams_t param,
                unsigned int challengeNum,
                const char **stringValue,
                int *value,
                int type)
{
    bool result; // al
    challengeInfo_t *challenge; // [esp+4h] [ebp-4h] BYREF

    if ( !LiveStats_GetChallengeInfo(&challenge, challengeNum, type) )
        return 0;
    switch ( param )
    {
        case CHALLENGE_PARAM_TYPE:
            *value = challenge->type;
            result = 1;
            break;
        case CHALLENGE_PARAM_TIER:
            *value = challenge->milestone;
            result = 1;
            break;
        case CHALLENGE_PARAM_NAME:
            *stringValue = challenge->name;
            result = 1;
            break;
        case CHALLENGE_PARAM_ITEM_INDEX:
            *value = challenge->index;
            result = 1;
            break;
        case CHALLENGE_PARAM_IS_ITEM_CLASSIFIED:
            *value = (challenge->flags & 4) != 0;
            result = 1;
            break;
        case CHALLENGE_PARAM_NUM_TO_TARGET_VALUE:
            *value = challenge->numToTargetValue;
            result = 1;
            break;
        case CHALLENGE_PARAM_TARGETVALUE:
            *value = challenge->targetValue;
            result = 1;
            break;
        case CHALLENGE_PARAM_CURRENTVALUE:
            *value = challenge->currentValue;
            result = 1;
            break;
        case CHALLENGE_PARAM_PREVTARGET:
            *value = challenge->prevTarget;
            result = 1;
            break;
        case CHALLENGE_PARAM_XP_REWARD:
            *value = challenge->xpReward;
            result = 1;
            break;
        case CHALLENGE_PARAM_CP_REWARD:
            *value = challenge->cpReward;
            result = 1;
            break;
        default:
            result = 0;
            break;
    }
    return result;
}

unsigned int __cdecl LiveStats_GetNumPersonalBests(int controllerIndex)
{
    if ( controllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    2433,
                    0,
                    "controllerIndex doesn't index MAX_GPAD_COUNT\n\t%i not in [0, %i)",
                    controllerIndex,
                    1) )
    {
        __debugbreak();
    }
    return s_currentPersonalBest[controllerIndex];
}

char *__cdecl LiveStats_GetPersonalBestName(int controllerIndex, unsigned int personalBestNum)
{
    personalBest_t *personalBest; // [esp+0h] [ebp-4h]

    personalBest = LiveStats_GetPersonalBest(controllerIndex, personalBestNum);
    if ( personalBest && personalBest->name )
        return va("%s", personalBest->name);
    else
        return 0;
}

personalBest_t *__cdecl LiveStats_GetPersonalBest(int controllerIndex, unsigned int personalBestNum)
{
    personalBest_t *personalBest; // [esp+0h] [ebp-4h]

    if ( controllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    2439,
                    0,
                    "controllerIndex doesn't index MAX_GPAD_COUNT\n\t%i not in [0, %i)",
                    controllerIndex,
                    1) )
    {
        __debugbreak();
    }
    personalBest = 0;
    if ( personalBestNum < s_currentPersonalBest[controllerIndex] )
        return &s_personalBests[controllerIndex][personalBestNum];
    return personalBest;
}

char *__cdecl LiveStats_GetPersonalBestPrefix(int controllerIndex, unsigned int personalBestNum)
{
    personalBest_t *personalBest; // [esp+0h] [ebp-4h]

    personalBest = LiveStats_GetPersonalBest(controllerIndex, personalBestNum);
    if ( personalBest && personalBest->prefix )
        return va("%s", personalBest->prefix);
    else
        return 0;
}

int __cdecl LiveStats_GetPersonalBestValue(int controllerIndex, unsigned int personalBestNum)
{
    personalBest_t *personalBest; // [esp+0h] [ebp-4h]

    personalBest = LiveStats_GetPersonalBest(controllerIndex, personalBestNum);
    if ( personalBest )
        return personalBest->value;
    else
        return 0;
}

int __cdecl LiveStats_GetPersonalBestDelta(int controllerIndex, unsigned int personalBestNum)
{
    personalBest_t *personalBest; // [esp+0h] [ebp-4h]

    personalBest = LiveStats_GetPersonalBest(controllerIndex, personalBestNum);
    if ( personalBest )
        return personalBest->delta;
    else
        return 0;
}

int __cdecl LiveStats_GetPersonalBestRating(int controllerIndex, unsigned int personalBestNum)
{
    personalBest_t *personalBest; // [esp+0h] [ebp-4h]

    personalBest = LiveStats_GetPersonalBest(controllerIndex, personalBestNum);
    if ( personalBest )
        return personalBest->rating;
    else
        return 0;
}

unsigned int __cdecl LiveStats_GetNumRecentlyUnlockedItems(int controllerIndex)
{
    if ( controllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    2597,
                    0,
                    "controllerIndex doesn't index MAX_GPAD_COUNT\n\t%i not in [0, %i)",
                    controllerIndex,
                    1) )
    {
        __debugbreak();
    }
    return s_numRecentlyUnlockedItems[controllerIndex];
}

int __cdecl LiveStats_GetRecentlyUnlockedItemIndex(int controllerIndex, unsigned int itemNum)
{
    if ( controllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    2603,
                    0,
                    "controllerIndex doesn't index MAX_GPAD_COUNT\n\t%i not in [0, %i)",
                    controllerIndex,
                    1) )
    {
        __debugbreak();
    }
    if ( itemNum < s_numRecentlyUnlockedItems[controllerIndex] && itemNum <= 0x100 )
        return s_recentlyUnlockedItems[controllerIndex][itemNum];
    else
        return 0;
}

void __cdecl LiveStats_PublishNews(int controllerIndex)
{
    int LocalClientNum; // eax
    int v2; // eax
    const char *ItemName; // eax
    int v4; // eax
    const char *v5; // eax
    int v6; // eax
    int v7; // eax
    char *v8; // [esp-4h] [ebp-ACh]
    char *v9; // [esp-4h] [ebp-ACh]
    char *v10; // [esp-4h] [ebp-ACh]
    char *v11; // [esp-4h] [ebp-ACh]
    char *v12; // [esp-4h] [ebp-ACh]
    char *v14; // [esp+Ch] [ebp-9Ch]
    const char *v15; // [esp+1Ch] [ebp-8Ch]
    const char *name; // [esp+20h] [ebp-88h]
    unsigned int v17; // [esp+20h] [ebp-88h]
    signed int contractIndex; // [esp+24h] [ebp-84h]
    signed int activeContractIndex; // [esp+28h] [ebp-80h]
    int rowNum; // [esp+2Ch] [ebp-7Ch]
    unsigned int j; // [esp+30h] [ebp-78h]
    unsigned int i; // [esp+34h] [ebp-74h]
    const char *level; // [esp+38h] [ebp-70h]
    const char *localized_weaponname_string; // [esp+3Ch] [ebp-6Ch]
    persistentStats *oldStatsBuffer; // [esp+40h] [ebp-68h]
    int oldRank; // [esp+44h] [ebp-64h]
    const char *targetValue; // [esp+48h] [ebp-60h]
    const char *weaponType; // [esp+4Ch] [ebp-5Ch]
    int oldRankXP; // [esp+50h] [ebp-58h] BYREF
    int newRank; // [esp+54h] [ebp-54h] BYREF
    char statsName[64]; // [esp+58h] [ebp-50h] BYREF
    int newPrestigeLevel; // [esp+9Ch] [ebp-Ch]
    const StringTable *statsMilestonestable; // [esp+A0h] [ebp-8h] BYREF
    int oldPrestigeLevel; // [esp+A4h] [ebp-4h] BYREF

    if ( controllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    2617,
                    0,
                    "controllerIndex doesn't index MAX_GPAD_COUNT\n\t%i not in [0, %i)",
                    controllerIndex,
                    1) )
    {
        __debugbreak();
    }
    oldStatsBuffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_GLOBALSTABLE, 1);
    oldRankXP = 0;
    newRank = LiveStats_GetRank(controllerIndex);
    LiveStats_GetIntPlayerStatFromBase(&oldRankXP, "RANKXP", (char *)oldStatsBuffer);
    oldRank = CL_GetRankForXp(oldRankXP);
    if ( newRank > oldRank )
    {
        level = CL_GetRankData(newRank, MP_RANKTABLE_DISPLAYLEVEL);
        v8 = va("userPublishNews TICKER_LEVELUPDATE %s\n", level);
        LocalClientNum = Com_ControllerIndex_GetLocalClientNum(controllerIndex);
        Cmd_ExecuteSingleCommand(LocalClientNum, controllerIndex, v8);
    }
    oldPrestigeLevel = 0;
    newPrestigeLevel = 0;
    LiveStats_GetIntPlayerStatFromBase(&oldPrestigeLevel, "PLEVEL", (char *)oldStatsBuffer);
    newPrestigeLevel = LiveStats_GetPrestige(controllerIndex);
    if ( newPrestigeLevel > oldPrestigeLevel )
    {
        v9 = va("userPublishNews TICKER_PRESTIGED \n");
        v2 = Com_ControllerIndex_GetLocalClientNum(controllerIndex);
        Cmd_ExecuteSingleCommand(v2, controllerIndex, v9);
    }
    for ( i = 0; i < s_numRecentlyUnlockedItems[controllerIndex]; ++i )
    {
        ItemName = BG_UnlockablesGetItemName(s_recentlyUnlockedItems[controllerIndex][i]);
        v10 = va("userPublishNews TICKER_UNLOCKITEMUPDATE %s\n", ItemName);
        v4 = Com_ControllerIndex_GetLocalClientNum(controllerIndex);
        Cmd_ExecuteSingleCommand(v4, controllerIndex, v10);
    }
    for ( j = 0; j < s_currentStatsMilestone[controllerIndex]; ++j )
    {
        I_strncpyz(statsName, "STATS_MILESTONE_STATNAME_", 64);
        if ( s_statsMilestonesCompleted[controllerIndex][j].type == MILESTONE_WEAPON )
        {
            localized_weaponname_string = BG_UnlockablesGetItemName(s_statsMilestonesCompleted[controllerIndex][j].index);
            weaponType = BG_UnlockablesGetItemGroup(s_statsMilestonesCompleted[controllerIndex][j].index);
            v5 = va("%d", s_statsMilestonesCompleted[controllerIndex][j].milestone - 1);
            rowNum = CL_GetMilestoneRowNum((XAssetHeader *)&statsMilestonestable, 1, v5, 3, weaponType, -1, 0);
            if ( statsMilestonestable )
            {
                if ( rowNum != -1 )
                {
                    targetValue = StringTable_GetColumnValueForRow(statsMilestonestable, rowNum, 2);
                    name = s_statsMilestonesCompleted[controllerIndex][j].name;
                    v17 = (unsigned int)&name[strlen(name) + 1];
                    v14 = (char *)&newRank + 3;
                    while ( *++v14 )
                        ;
                    v15 = s_statsMilestonesCompleted[controllerIndex][j].name;
                    memcpy(v14, v15, v17 - (unsigned int)v15);
                    v11 = va(
                                    "userPublishNews TICKER_STATSMILESTONEUPDATE %s %s %s\n",
                                    localized_weaponname_string,
                                    targetValue,
                                    statsName);
                    v6 = Com_ControllerIndex_GetLocalClientNum(controllerIndex);
                    Cmd_ExecuteSingleCommand(v6, controllerIndex, v11);
                }
            }
        }
        statsName[0] = 0;
    }
    for ( activeContractIndex = 0; activeContractIndex < 3; ++activeContractIndex )
    {
        if ( LiveContracts_GetActiveContractStatus_Internal((char *)oldStatsBuffer, activeContractIndex) != 2
            && LiveContracts_GetActiveContractStatus(controllerIndex, activeContractIndex) == 2 )
        {
            contractIndex = LiveContracts_GetIndexForActiveContract(controllerIndex, activeContractIndex);
            if ( contractIndex >= 0 )
            {
                v12 = va("userPublishNews %s %d\n", "TICKER_CONTRACTCOMPLETED", contractIndex);
                v7 = Com_ControllerIndex_GetLocalClientNum(controllerIndex);
                Cmd_ExecuteSingleCommand(v7, controllerIndex, v12);
            }
        }
    }
}

char __cdecl LiveStats_MoveToProItemVersion(ddlState_t *searchState, int itemIndex, int challengeIndex)
{
    const ddlState_t *RootDDLState; // eax
    char *v5; // [esp-Ch] [ebp-Ch]

    va("%d", challengeIndex);
    v5 = va("%d", itemIndex);
    RootDDLState = LiveStats_GetRootDDLState();
    if ( DDL_MoveTo(RootDDLState, searchState, 4, "itemStats", v5) )
        return 1;
    DDL_PrintError("DDL: Error could not find item version unlocked %d challenge %d\n", itemIndex, challengeIndex);
    return 0;
}

void __cdecl LiveStats_CompareStatsVsStableBuffer(int controllerIndex)
{
    char *v1; // eax
    eAttachment AttachmentIndex; // eax
    ddlState_t searchStateGameModeMilestones; // [esp+0h] [ebp-D8h] BYREF
    char *v4; // [esp+10h] [ebp-C8h]
    int i; // [esp+14h] [ebp-C4h]
    int memberCount; // [esp+18h] [ebp-C0h]
    const char *statName; // [esp+1Ch] [ebp-BCh]
    ddlState_t searchStateGameModeBests; // [esp+20h] [ebp-B8h] BYREF
    const char *gameModeName; // [esp+30h] [ebp-A8h]
    ddlState_t searchStateGameModeCurrentStats; // [esp+34h] [ebp-A4h] BYREF
    int currentGameMode; // [esp+44h] [ebp-94h]
    int numGameModes; // [esp+48h] [ebp-90h]
    const char *attachmentName; // [esp+4Ch] [ebp-8Ch]
    ddlState_t searchStateGroupStats; // [esp+50h] [ebp-88h] BYREF
    const char *groupName; // [esp+60h] [ebp-78h]
    int currItemGroup; // [esp+64h] [ebp-74h]
    unsigned int clanTagFeature; // [esp+68h] [ebp-70h]
    int proItemIndex; // [esp+6Ch] [ebp-6Ch]
    int oldProUnlocked; // [esp+70h] [ebp-68h]
    int newProUnlocked; // [esp+74h] [ebp-64h]
    unsigned int itemNumber; // [esp+78h] [ebp-60h]
    int oldPLevel; // [esp+7Ch] [ebp-5Ch] BYREF
    char *liveStatsBuffer; // [esp+80h] [ebp-58h]
    ddlState_t searchStateGameModeStats; // [esp+84h] [ebp-54h] BYREF
    ddlState_t searchAttachmentStateStats; // [esp+94h] [ebp-44h] BYREF
    char *oldStatsBuffer; // [esp+A4h] [ebp-34h]
    int oldRank; // [esp+A8h] [ebp-30h]
    int newPLevel; // [esp+ACh] [ebp-2Ch]
    ddlState_t searchStateBests; // [esp+B0h] [ebp-28h] BYREF
    int oldRankXP; // [esp+C0h] [ebp-18h] BYREF
    int newRank; // [esp+C4h] [ebp-14h]
    ddlState_t searchStateStats; // [esp+C8h] [ebp-10h] BYREF

    if ( controllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    2747,
                    0,
                    "controllerIndex doesn't index MAX_GPAD_COUNT\n\t%i not in [0, %i)",
                    controllerIndex,
                    1) )
    {
        __debugbreak();
    }
    liveStatsBuffer = 0;
    oldStatsBuffer = 0;
    if ( xblive_basictraining && xblive_basictraining->current.enabled )
    {
        liveStatsBuffer = (char *)LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
        oldStatsBuffer = (char *)LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_STABLE, 1);
    }
    else
    {
        liveStatsBuffer = (char *)LiveStorage_GetStatsBuffer(0, STATS_LOCATION_GLOBAL, 1);
        oldStatsBuffer = (char *)LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_GLOBALSTABLE, 1);
    }
    s_currentStatsMilestone[controllerIndex] = 0;
    oldRankXP = 0;
    newRank = LiveStats_GetRank(controllerIndex);
    newPLevel = LiveStats_GetPrestige(controllerIndex);
    oldPLevel = 0;
    if ( xblive_basictraining->current.enabled )
    {
        LiveStats_GetIntPlayerStatFromBase(&oldRankXP, "RANKXP", oldStatsBuffer);
        LiveStats_GetIntPlayerStatFromBase(&oldPLevel, "PLEVEL", oldStatsBuffer);
    }
    else
    {
        LiveStats_GetIntPlayerStatFromForcedBase(&oldRankXP, "RANKXP", oldStatsBuffer);
        LiveStats_GetIntPlayerStatFromForcedBase(&oldPLevel, "PLEVEL", oldStatsBuffer);
    }
    oldRank = CL_GetRankForXp(oldRankXP);
    memset(s_recentlyUnlockedItems[controllerIndex], 0, sizeof(int[256]));
    s_numRecentlyUnlockedItems[controllerIndex] = 0;
    for ( itemNumber = 0; itemNumber < 0x100; ++itemNumber )
    {
        if ( BG_UnlockablesIsItemValidNotNull(itemNumber) )
        {
            if ( newRank > oldRank
                && !BG_UnlockablesIsItemLockedForRank(newPLevel, newRank, itemNumber)
                && BG_UnlockablesIsItemLockedForRank(oldPLevel, oldRank, itemNumber)
                && !BG_UnlockablesIsItemInDefaultClass(itemNumber) )
            {
                s_recentlyUnlockedItems[controllerIndex][s_numRecentlyUnlockedItems[controllerIndex]++] = itemNumber;
                BG_UnlockablesSetItemNew(controllerIndex, itemNumber);
            }
            v1 = va("%d", itemNumber);
            if ( DDL_MoveTo(&g_statsRootState, &searchStateStats, 3, "ItemStats", v1)
                && DDL_IterateFirst(&searchStateStats, &searchStateStats) )
            {
                do
                {
                    LiveStats_UpdateItemPersonalBest(liveStatsBuffer, oldStatsBuffer, &searchStateStats);
                    LiveStats_UpdateItemStatsMilestones(
                        controllerIndex,
                        liveStatsBuffer,
                        oldStatsBuffer,
                        &searchStateStats,
                        itemNumber);
                }
                while ( DDL_IterateNext(&searchStateStats, &searchStateStats) );
            }
            oldProUnlocked = (unsigned __int8)LiveStats_ProVersionUnlocked(itemNumber, oldStatsBuffer);
            newProUnlocked = (unsigned __int8)LiveStats_ProVersionUnlocked(itemNumber, liveStatsBuffer);
            if ( newProUnlocked > oldProUnlocked )
            {
                proItemIndex = BG_UnlockablesGetProItem(itemNumber);
                if ( proItemIndex != -1 )
                {
                    s_recentlyUnlockedItems[controllerIndex][s_numRecentlyUnlockedItems[controllerIndex]++] = proItemIndex;
                    BG_UnlockablesSetItemNew(controllerIndex, proItemIndex);
                }
            }
        }
    }
    for ( clanTagFeature = 0; clanTagFeature < 0x100; ++clanTagFeature )
    {
        if ( BG_UnlockablesGetClanTagFeature(clanTagFeature, CLANTAG_COL_UNLOCKLVL)
            && newRank > oldRank
            && !BG_UnlockablesIsClanTagFeatureLockedForRank(newPLevel, newRank, clanTagFeature)
            && BG_UnlockablesIsClanTagFeatureLockedForRank(oldPLevel, oldRank, clanTagFeature) )
        {
            BG_UnlockablesSetClanTagFeatureNew(controllerIndex, clanTagFeature);
        }
    }
    for ( currItemGroup = 0; currItemGroup < 24; ++currItemGroup )
    {
        groupName = BG_UnlockablesGetItemGroupName((itemGroup_t)currItemGroup);
        if ( DDL_MoveTo(&g_statsRootState, &searchStateStats, 2, "GroupStats", groupName)
            && DDL_MoveTo(&searchStateStats, &searchStateGroupStats, 1, "stats")
            && DDL_IterateFirst(&searchStateGroupStats, &searchStateGroupStats) )
        {
            do
                LiveStats_UpdateGroupStatsMilestones(
                    controllerIndex,
                    liveStatsBuffer,
                    oldStatsBuffer,
                    &searchStateGroupStats,
                    (itemGroup_t)currItemGroup);
            while ( DDL_IterateNext(&searchStateGroupStats, &searchStateGroupStats) );
        }
    }
    if ( DDL_MoveTo(&g_statsRootState, &searchStateStats, 1, "Attachments")
        && DDL_IterateFirst(&searchStateStats, &searchStateStats) )
    {
        do
        {
            attachmentName = searchStateStats.ddl->enumList[searchStateStats.member->enumIndex].members[searchStateStats.arrayIndex];
            if ( DDL_MoveTo(&searchStateStats, &searchAttachmentStateStats, 1, "stats")
                && DDL_IterateFirst(&searchAttachmentStateStats, &searchAttachmentStateStats) )
            {
                do
                {
                    AttachmentIndex = BG_GetAttachmentIndex(attachmentName);
                    LiveStats_UpdateAttachmentStatsMilestones(
                        controllerIndex,
                        liveStatsBuffer,
                        oldStatsBuffer,
                        &searchAttachmentStateStats,
                        AttachmentIndex);
                }
                while ( DDL_IterateNext(&searchAttachmentStateStats, &searchAttachmentStateStats) );
            }
        }
        while ( DDL_IterateNext(&searchStateStats, &searchStateStats) );
    }
    s_currentPersonalBest[controllerIndex] = 0;
    if ( DDL_MoveTo(&g_statsRootState, &searchStateStats, 1, "PlayerStatsList")
        && DDL_MoveTo(&g_statsRootState, &searchStateBests, 1, "PersonalBests")
        && DDL_IterateFirst(&searchStateStats, &searchStateStats)
        && DDL_IterateFirst(&searchStateBests, &searchStateBests) )
    {
        do
            LiveStats_UpdatePersonalBest(
                controllerIndex,
                liveStatsBuffer,
                oldStatsBuffer,
                &searchStateStats,
                &searchStateBests);
        while ( DDL_IterateNext(&searchStateStats, &searchStateStats)
                 && DDL_IterateNext(&searchStateBests, &searchStateBests) );
    }
    if ( DDL_MoveTo(&g_statsRootState, &searchStateStats, 1, "challengeTier")
        && DDL_IterateFirst(&searchStateStats, &searchStateStats) )
    {
        do
            LiveStats_UpdateStatsMilestones(controllerIndex, liveStatsBuffer, oldStatsBuffer, &searchStateStats);
        while ( DDL_IterateNext(&searchStateStats, &searchStateStats) );
    }
    LiveStats_UpdatetBestMatchAccuracy(liveStatsBuffer, oldStatsBuffer);
    LiveStats_UpdatetBestMatchKdRatio(liveStatsBuffer, oldStatsBuffer);
    if ( DDL_MoveTo(&g_statsRootState, &searchStateStats, 1, "HighestStats")
        && DDL_IterateFirst(&searchStateStats, &searchStateStats) )
    {
        do
            LiveStats_UpdatePersonalBestHighest(controllerIndex, liveStatsBuffer, oldStatsBuffer, &searchStateStats);
        while ( DDL_IterateNext(&searchStateStats, &searchStateStats) );
    }
    if ( DDL_MoveTo(&g_statsRootState, &searchStateStats, 1, "PlayerBestStatsByGameMode")
        && DDL_MoveTo(&g_statsRootState, &searchStateGameModeStats, 1, "PlayerStatsByGameMode") )
    {
        numGameModes = searchStateStats.ddl->enumList[searchStateStats.member->enumIndex].memberCount;
        for ( currentGameMode = 0; currentGameMode < numGameModes; ++currentGameMode )
        {
            gameModeName = searchStateStats.ddl->enumList[searchStateStats.member->enumIndex].members[currentGameMode];
            if ( DDL_MoveTo(&searchStateStats, &searchStateGameModeBests, 2, gameModeName, "stats")
                && DDL_IterateFirst(&searchStateGameModeBests, &searchStateGameModeBests) )
            {
                do
                {
                    statName = searchStateGameModeBests.ddl->enumList[searchStateGameModeBests.member->enumIndex].members[searchStateGameModeBests.arrayIndex];
                    if ( DDL_MoveTo(&searchStateGameModeStats, &searchStateGameModeCurrentStats, 2, gameModeName, statName) )
                        LiveStats_UpdatePersonalBest(
                            controllerIndex,
                            liveStatsBuffer,
                            oldStatsBuffer,
                            &searchStateGameModeCurrentStats,
                            &searchStateGameModeBests);
                }
                while ( DDL_IterateNext(&searchStateGameModeBests, &searchStateGameModeBests) );
            }
        }
    }
    if ( DDL_MoveTo(&g_statsRootState, &searchStateStats, 1, "CurrentGameModeMilestone") )
    {
        memberCount = searchStateStats.ddl->enumList[searchStateStats.member->enumIndex].memberCount;
        for ( i = 0; i < memberCount; ++i )
        {
            v4 = (char *)searchStateStats.ddl->enumList[searchStateStats.member->enumIndex].members[i];
            if ( DDL_MoveTo(&searchStateStats, &searchStateGameModeMilestones, 2, v4, "milestones")
                && DDL_IterateFirst(&searchStateGameModeMilestones, &searchStateGameModeMilestones) )
            {
                do
                    LiveStats_UpdateGameModeStatsMilestones(
                        controllerIndex,
                        liveStatsBuffer,
                        oldStatsBuffer,
                        &searchStateGameModeMilestones,
                        v4);
                while ( DDL_IterateNext(&searchStateGameModeMilestones, &searchStateGameModeMilestones) );
            }
        }
    }
    LiveStats_SortPersonalBests(controllerIndex);
    LiveStats_PublishNews(controllerIndex);
}

char __cdecl LiveStats_UpdateItemPersonalBest(
                char *liveStatsBuffer,
                char *oldStatsBuffer,
                ddlState_t *searchStateStats)
{
    unsigned int currentValue; // [esp+0h] [ebp-10h]
    unsigned int lastValue; // [esp+Ch] [ebp-4h]

    if ( (!liveStatsBuffer || !oldStatsBuffer || !searchStateStats)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    1464,
                    0,
                    "%s",
                    "liveStatsBuffer && oldStatsBuffer && searchStateStats") )
    {
        __debugbreak();
    }
    if ( !liveStatsBuffer )
        return 0;
    if ( !oldStatsBuffer )
        return 0;
    if ( !searchStateStats )
        return 0;
    currentValue = LiveStats_GetItemStat(searchStateStats, "statValue", liveStatsBuffer);
    lastValue = LiveStats_GetItemStat(searchStateStats, "statValue", oldStatsBuffer);
    if ( currentValue - lastValue <= LiveStats_GetItemStat(searchStateStats, "personalBest", liveStatsBuffer) )
        return 0;
    LiveStats_SetItemStat(searchStateStats, "personalBest", liveStatsBuffer, currentValue - lastValue);
    return 1;
}

unsigned int __cdecl LiveStats_GetItemStat(ddlState_t *searchStateStats, const char *statMember, char *buffer)
{
    ddlState_t searchState; // [esp+8h] [ebp-10h] BYREF

    if ( !searchStateStats
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp", 1436, 0, "%s", "searchStateStats") )
    {
        __debugbreak();
    }
    if ( !statMember
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp", 1437, 0, "%s", "statMember") )
    {
        __debugbreak();
    }
    if ( !buffer && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp", 1438, 0, "%s", "buffer") )
        __debugbreak();
    if ( !searchStateStats || !statMember || !buffer )
        return 0;
    if ( DDL_MoveTo(searchStateStats, &searchState, 1, statMember) )
    {
        if ( !xblive_basictraining || !xblive_basictraining->current.enabled )
            LiveStats_GetBasicTrainingState((unsigned __int8 *)buffer);
        return DDL_GetInt(&searchState, buffer);
    }
    else
    {
        Com_PrintError(29, "DDL: Error could not find member %s\n", statMember);
        return 0;
    }
}

char __cdecl LiveStats_UpdateItemStatsMilestones(
                int controllerIndex,
                char *liveStatsBuffer,
                char *oldStatsBuffer,
                ddlState_t *searchStateStats,
                unsigned __int8 itemIndex)
{
    statsMilestone_t *statsMilestone; // [esp+0h] [ebp-Ch]
    unsigned int currentMilestone; // [esp+4h] [ebp-8h]

    if ( controllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    1503,
                    0,
                    "controllerIndex doesn't index MAX_GPAD_COUNT\n\t%i not in [0, %i)",
                    controllerIndex,
                    1) )
    {
        __debugbreak();
    }
    if ( (!liveStatsBuffer || !oldStatsBuffer || !searchStateStats)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    1504,
                    0,
                    "%s",
                    "liveStatsBuffer && oldStatsBuffer && searchStateStats") )
    {
        __debugbreak();
    }
    if ( s_currentStatsMilestone[controllerIndex] < 0x3E8 )
    {
        if ( liveStatsBuffer
            && oldStatsBuffer
            && searchStateStats
            && (currentMilestone = LiveStats_GetItemStat(searchStateStats, "challengeTier", liveStatsBuffer),
                    currentMilestone > LiveStats_GetItemStat(searchStateStats, "challengeTier", oldStatsBuffer)) )
        {
            statsMilestone = &s_statsMilestonesCompleted[controllerIndex][s_currentStatsMilestone[controllerIndex]];
            statsMilestone->index = itemIndex;
            statsMilestone->milestone = currentMilestone;
            statsMilestone->type = MILESTONE_WEAPON;
            statsMilestone->name = searchStateStats->ddl->enumList[searchStateStats->member->enumIndex].members[searchStateStats->arrayIndex];
            ++s_currentStatsMilestone[controllerIndex];
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        Com_PrintError(16, "Tried to add too many stats milestones to list - only %d are supported.\n", 1000);
        return 0;
    }
}

char __cdecl LiveStats_UpdateGroupStatsMilestones(
                int controllerIndex,
                char *liveStatsBuffer,
                char *oldStatsBuffer,
                ddlState_t *searchStateStats,
                itemGroup_t groupIndex)
{
    statsMilestone_t *statsMilestone; // [esp+0h] [ebp-Ch]
    unsigned int currentMilestone; // [esp+4h] [ebp-8h]

    if ( controllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    1749,
                    0,
                    "controllerIndex doesn't index MAX_GPAD_COUNT\n\t%i not in [0, %i)",
                    controllerIndex,
                    1) )
    {
        __debugbreak();
    }
    if ( (!liveStatsBuffer || !oldStatsBuffer || !searchStateStats)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    1750,
                    0,
                    "%s",
                    "liveStatsBuffer && oldStatsBuffer && searchStateStats") )
    {
        __debugbreak();
    }
    if ( s_currentStatsMilestone[controllerIndex] < 0x3E8 )
    {
        if ( liveStatsBuffer
            && oldStatsBuffer
            && searchStateStats
            && (currentMilestone = LiveStats_GetItemStat(searchStateStats, "challengeTier", liveStatsBuffer),
                    currentMilestone > LiveStats_GetItemStat(searchStateStats, "challengeTier", oldStatsBuffer)) )
        {
            statsMilestone = &s_statsMilestonesCompleted[controllerIndex][s_currentStatsMilestone[controllerIndex]];
            statsMilestone->milestone = currentMilestone;
            statsMilestone->type = MILESTONE_GROUP;
            statsMilestone->name = searchStateStats->ddl->enumList[searchStateStats->member->enumIndex].members[searchStateStats->arrayIndex];
            statsMilestone->index = groupIndex;
            ++s_currentStatsMilestone[controllerIndex];
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        Com_PrintError(16, "Tried to add too many stats milestones to list - only %d are supported.\n", 1000);
        return 0;
    }
}

char __cdecl LiveStats_UpdateAttachmentStatsMilestones(
                int controllerIndex,
                char *liveStatsBuffer,
                char *oldStatsBuffer,
                ddlState_t *searchStateStats,
                eAttachment attachmentIndex)
{
    statsMilestone_t *statsMilestone; // [esp+0h] [ebp-Ch]
    unsigned int currentMilestone; // [esp+4h] [ebp-8h]

    if ( controllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    1780,
                    0,
                    "controllerIndex doesn't index MAX_GPAD_COUNT\n\t%i not in [0, %i)",
                    controllerIndex,
                    1) )
    {
        __debugbreak();
    }
    if ( (!liveStatsBuffer || !oldStatsBuffer || !searchStateStats)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    1781,
                    0,
                    "%s",
                    "liveStatsBuffer && oldStatsBuffer && searchStateStats") )
    {
        __debugbreak();
    }
    if ( s_currentStatsMilestone[controllerIndex] < 0x3E8 )
    {
        if ( liveStatsBuffer
            && oldStatsBuffer
            && searchStateStats
            && (currentMilestone = LiveStats_GetItemStat(searchStateStats, "challengeTier", liveStatsBuffer),
                    currentMilestone > LiveStats_GetItemStat(searchStateStats, "challengeTier", oldStatsBuffer)) )
        {
            statsMilestone = &s_statsMilestonesCompleted[controllerIndex][s_currentStatsMilestone[controllerIndex]];
            statsMilestone->milestone = currentMilestone;
            statsMilestone->type = MILESTONE_ATTACHMENTS;
            statsMilestone->name = searchStateStats->ddl->enumList[searchStateStats->member->enumIndex].members[searchStateStats->arrayIndex];
            statsMilestone->index = attachmentIndex;
            ++s_currentStatsMilestone[controllerIndex];
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        Com_PrintError(16, "Tried to add too many stats milestones to list - only %d are supported.\n", 1000);
        return 0;
    }
}

char __cdecl LiveStats_UpdatePersonalBest(
                int __formal,
                char *liveStatsBuffer,
                char *oldStatsBuffer,
                ddlState_t *searchStateStats,
                ddlState_t *searchStateBests)
{
    unsigned int currentValue; // [esp+0h] [ebp-10h]
    signed int currentBest; // [esp+4h] [ebp-Ch]
    signed int matchDelta; // [esp+8h] [ebp-8h]
    unsigned int lastValue; // [esp+Ch] [ebp-4h]

    if ( (!liveStatsBuffer || !oldStatsBuffer || !searchStateStats || !searchStateBests)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    1815,
                    0,
                    "%s",
                    "liveStatsBuffer && oldStatsBuffer && searchStateStats && searchStateBests") )
    {
        __debugbreak();
    }
    if ( !liveStatsBuffer || !oldStatsBuffer || !searchStateStats || !searchStateBests )
        return 0;
    currentValue = DDL_GetInt(searchStateStats, liveStatsBuffer);
    lastValue = DDL_GetInt(searchStateStats, oldStatsBuffer);
    currentBest = DDL_GetInt(searchStateBests, liveStatsBuffer);
    matchDelta = currentValue - lastValue;
    if ( (int)(currentValue - lastValue) < 0 )
    {
        Com_PrintWarning(
            15,
            "Negative delta, %u currentvalue, %u lastvalue, stat %s %i\n",
            currentValue,
            lastValue,
            searchStateStats->member->name,
            searchStateStats->member->offset);
        return 0;
    }
    if ( matchDelta > currentBest )
        DDL_SetInt(searchStateBests, matchDelta, liveStatsBuffer);
    return 1;
}

char __cdecl LiveStats_UpdateStatsMilestones(
                int controllerIndex,
                char *liveStatsBuffer,
                char *oldStatsBuffer,
                ddlState_t *searchStateStats)
{
    statsMilestone_t *statsMilestone; // [esp+0h] [ebp-Ch]
    unsigned int currentMilestone; // [esp+4h] [ebp-8h]

    if ( controllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    1879,
                    0,
                    "controllerIndex doesn't index MAX_GPAD_COUNT\n\t%i not in [0, %i)",
                    controllerIndex,
                    1) )
    {
        __debugbreak();
    }
    if ( (!liveStatsBuffer || !oldStatsBuffer || !searchStateStats)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    1880,
                    0,
                    "%s",
                    "liveStatsBuffer && oldStatsBuffer && searchStateStats") )
    {
        __debugbreak();
    }
    if ( s_currentStatsMilestone[controllerIndex] < 0x3E8 )
    {
        if ( liveStatsBuffer
            && oldStatsBuffer
            && searchStateStats
            && (currentMilestone = DDL_GetInt(searchStateStats, liveStatsBuffer),
                    currentMilestone > DDL_GetInt(searchStateStats, oldStatsBuffer)) )
        {
            statsMilestone = &s_statsMilestonesCompleted[controllerIndex][s_currentStatsMilestone[controllerIndex]];
            statsMilestone->milestone = currentMilestone;
            statsMilestone->type = MILESTONE_GLOBAL;
            statsMilestone->name = searchStateStats->ddl->enumList[searchStateStats->member->enumIndex].members[searchStateStats->arrayIndex];
            ++s_currentStatsMilestone[controllerIndex];
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        Com_PrintError(16, "Tried to add too many stats milestones to list - only %d are supported.\n", 1000);
        return 0;
    }
}

char __cdecl LiveStats_UpdateGameModeStatsMilestones(
                int controllerIndex,
                char *liveStatsBuffer,
                char *oldStatsBuffer,
                ddlState_t *searchStateStats,
                const char *gameModeName)
{
    statsMilestone_t *statsMilestone; // [esp+0h] [ebp-10h]
    int currGameMode; // [esp+4h] [ebp-Ch]
    unsigned int currentMilestone; // [esp+8h] [ebp-8h]

    if ( controllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    1910,
                    0,
                    "controllerIndex doesn't index MAX_GPAD_COUNT\n\t%i not in [0, %i)",
                    controllerIndex,
                    1) )
    {
        __debugbreak();
    }
    if ( (!liveStatsBuffer || !oldStatsBuffer || !searchStateStats)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    1911,
                    0,
                    "%s",
                    "liveStatsBuffer && oldStatsBuffer && searchStateStats") )
    {
        __debugbreak();
    }
    if ( s_currentStatsMilestone[controllerIndex] < 0x3E8 )
    {
        if ( liveStatsBuffer )
        {
            if ( oldStatsBuffer )
            {
                if ( searchStateStats )
                {
                    currentMilestone = DDL_GetInt(searchStateStats, liveStatsBuffer);
                    if ( currentMilestone > DDL_GetInt(searchStateStats, oldStatsBuffer) )
                    {
                        for ( currGameMode = 0; currGameMode < 16; ++currGameMode )
                        {
                            if ( !I_stricmp(lbTypeEnum_7[currGameMode], gameModeName) )
                            {
                                statsMilestone = &s_statsMilestonesCompleted[controllerIndex][s_currentStatsMilestone[controllerIndex]];
                                statsMilestone->milestone = currentMilestone;
                                statsMilestone->type = MILESTONE_GAMEMODE;
                                statsMilestone->name = searchStateStats->ddl->enumList[searchStateStats->member->enumIndex].members[searchStateStats->arrayIndex];
                                statsMilestone->index = currGameMode;
                                ++s_currentStatsMilestone[controllerIndex];
                                return 1;
                            }
                        }
                    }
                }
            }
        }
        return 0;
    }
    else
    {
        Com_PrintError(16, "Tried to add too many stats milestones to list - only %d are supported.\n", 1000);
        return 0;
    }
}

char __cdecl LiveStats_UpdatePersonalBestHighest(
                int controllerIndex,
                char *liveStatsBuffer,
                char *oldStatsBuffer,
                ddlState_t *searchStateStats)
{
    personalBest_t *personalBest; // [esp+0h] [ebp-10h]
    unsigned int currentValue; // [esp+4h] [ebp-Ch]
    unsigned int lastValue; // [esp+Ch] [ebp-4h]

    if ( controllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    2072,
                    0,
                    "controllerIndex doesn't index MAX_GPAD_COUNT\n\t%i not in [0, %i)",
                    controllerIndex,
                    1) )
    {
        __debugbreak();
    }
    if ( (!liveStatsBuffer || !oldStatsBuffer || !searchStateStats)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    2073,
                    0,
                    "%s",
                    "liveStatsBuffer && oldStatsBuffer && searchStateStats") )
    {
        __debugbreak();
    }
    if ( s_currentPersonalBest[controllerIndex] < 0x3E8 )
    {
        if ( liveStatsBuffer
            && oldStatsBuffer
            && searchStateStats
            && (currentValue = DDL_GetInt(searchStateStats, liveStatsBuffer),
                    lastValue = DDL_GetInt(searchStateStats, oldStatsBuffer),
                    currentValue != lastValue) )
        {
            personalBest = &s_personalBests[controllerIndex][s_currentPersonalBest[controllerIndex]];
            personalBest->value = currentValue;
            personalBest->delta = currentValue - lastValue;
            personalBest->name = searchStateStats->ddl->enumList[searchStateStats->member->enumIndex].members[searchStateStats->arrayIndex];
            personalBest->prefix = "";
            ++s_currentPersonalBest[controllerIndex];
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        Com_PrintError(16, "Tried to add too many personal bests to list - only %d are supported.\n", 1000);
        return 0;
    }
}

char __cdecl LiveStats_UpdatetBestMatchAccuracy(char *liveStatsBuffer, char *oldStatsBuffer)
{
    unsigned int oldShots; // [esp+4h] [ebp-1Ch]
    unsigned int currentShots; // [esp+8h] [ebp-18h]
    unsigned int matchHits; // [esp+14h] [ebp-Ch]
    unsigned int matchShots; // [esp+18h] [ebp-8h]
    unsigned int oldHits; // [esp+1Ch] [ebp-4h]

    if ( (!liveStatsBuffer || !oldStatsBuffer)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    2103,
                    0,
                    "%s",
                    "liveStatsBuffer && oldStatsBuffer") )
    {
        __debugbreak();
    }
    if ( !liveStatsBuffer || !oldStatsBuffer )
        return 0;
    oldShots = LiveStats_GetPlayerStat("PlayerStatsList", "TOTAL_SHOTS", oldStatsBuffer);
    currentShots = LiveStats_GetPlayerStat("PlayerStatsList", "TOTAL_SHOTS", liveStatsBuffer);
    oldHits = LiveStats_GetPlayerStat("PlayerStatsList", "HITS", oldStatsBuffer);
    matchShots = currentShots - oldShots;
    matchHits = LiveStats_GetPlayerStat("PlayerStatsList", "HITS", liveStatsBuffer) - oldHits;
    if ( currentShots == oldShots )
        return 0;
    if ( 10000 * matchHits / matchShots <= LiveStats_GetPlayerStat("HighestStats", "HIGHEST_ACCURACY", oldStatsBuffer) )
        return 0;
    LiveStats_SetPlayerStat("HighestStats", "HIGHEST_ACCURACY", liveStatsBuffer, 10000 * matchHits / matchShots);
    return 1;
}

unsigned int __cdecl LiveStats_GetPlayerStat(const char *statsList, const char *statName, char *buffer)
{
    ddlState_t searchState; // [esp+8h] [ebp-10h] BYREF

    if ( !statName
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp", 1326, 0, "%s", "statName") )
    {
        __debugbreak();
    }
    if ( !buffer && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp", 1327, 0, "%s", "buffer") )
        __debugbreak();
    if ( !statName || !buffer )
        return 0;
    if ( DDL_MoveTo(&g_statsRootState, &searchState, 2, statsList, statName) )
    {
        if ( !xblive_basictraining || !xblive_basictraining->current.enabled )
            LiveStats_GetBasicTrainingState((unsigned __int8 *)buffer);
        return DDL_GetInt(&searchState, buffer);
    }
    else
    {
        Com_PrintError(29, "DDL: Error could not find player stat %s\n", statName);
        return 0;
    }
}

char __cdecl LiveStats_UpdatetBestMatchKdRatio(char *liveStatsBuffer, char *oldStatsBuffer)
{
    unsigned int matchDeaths; // [esp+0h] [ebp-20h]
    unsigned int matchKills; // [esp+4h] [ebp-1Ch]
    unsigned int currentKills; // [esp+8h] [ebp-18h]
    unsigned int oldKills; // [esp+Ch] [ebp-14h]
    unsigned int oldDeaths; // [esp+10h] [ebp-10h]

    if ( (!liveStatsBuffer || !oldStatsBuffer)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    2135,
                    0,
                    "%s",
                    "liveStatsBuffer && oldStatsBuffer") )
    {
        __debugbreak();
    }
    if ( !liveStatsBuffer || !oldStatsBuffer )
        return 0;
    oldKills = LiveStats_GetPlayerStat("PlayerStatsList", "KILLS", oldStatsBuffer);
    currentKills = LiveStats_GetPlayerStat("PlayerStatsList", "KILLS", liveStatsBuffer);
    oldDeaths = LiveStats_GetPlayerStat("PlayerStatsList", "DEATHS", oldStatsBuffer);
    matchKills = currentKills - oldKills;
    matchDeaths = LiveStats_GetPlayerStat("PlayerStatsList", "DEATHS", liveStatsBuffer) - oldDeaths;
    if ( currentKills == oldKills )
        return 0;
    if ( !matchDeaths )
        matchDeaths = 1;
    if ( 1000 * matchKills / matchDeaths <= LiveStats_GetPlayerStat("HighestStats", "HIGHEST_KDRATIO", oldStatsBuffer) )
        return 0;
    LiveStats_SetPlayerStat("HighestStats", "HIGHEST_KDRATIO", liveStatsBuffer, 1000 * matchKills / matchDeaths);
    return 1;
}

void __cdecl LiveStats_SortPersonalBests(int controllerIndex)
{
    const char *v1; // eax
    const char *ColumnValueForRow; // eax
    const char *v3; // eax
    unsigned int personalBestNum; // [esp+0h] [ebp-24h]
    int minDelta; // [esp+4h] [ebp-20h]
    int delta; // [esp+8h] [ebp-1Ch]
    int multiplier; // [esp+Ch] [ebp-18h]
    personalBest_t *personalBestPtr; // [esp+10h] [ebp-14h]
    int personalBestRow; // [esp+14h] [ebp-10h]
    unsigned int personalBest; // [esp+18h] [ebp-Ch]
    const StringTable *statsTable; // [esp+1Ch] [ebp-8h] BYREF
    unsigned int numPersonalBests; // [esp+20h] [ebp-4h]

    if ( controllerIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    2542,
                    0,
                    "controllerIndex doesn't index MAX_GPAD_COUNT\n\t%i not in [0, %i)",
                    controllerIndex,
                    1) )
    {
        __debugbreak();
    }
    StringTable_GetAsset("mp/personalbests.csv", (XAssetHeader *)&statsTable);
    if ( !statsTable
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp", 2545, 0, "%s", "statsTable") )
    {
        __debugbreak();
    }
    if ( statsTable )
    {
        for ( personalBest = 0; personalBest < LiveStats_GetNumPersonalBests(controllerIndex); ++personalBest )
        {
            personalBestPtr = &s_personalBests[controllerIndex][personalBest];
            personalBestPtr->rating = 0;
            v1 = va("%s%s", personalBestPtr->prefix, personalBestPtr->name);
            personalBestRow = StringTable_LookupRowNumForValue(statsTable, 0, v1);
            if ( personalBestRow == -1 )
            {
                personalBestPtr->delta = -2147483647;
            }
            else
            {
                ColumnValueForRow = StringTable_GetColumnValueForRow(statsTable, personalBestRow, 1);
                minDelta = atoi(ColumnValueForRow);
                delta = LiveStats_GetPersonalBestDelta(controllerIndex, personalBest);
                v3 = StringTable_GetColumnValueForRow(statsTable, personalBestRow, 2);
                multiplier = atoi(v3);
                if ( multiplier && (delta >= 0 || (int)abs(delta) <= minDelta) )
                    personalBestPtr->rating = multiplier * personalBestPtr->value;
                else
                    personalBestPtr->delta = -2147483647;
            }
        }
        qsort(
            s_personalBests[controllerIndex],
            s_currentPersonalBest[controllerIndex],
            0x14u,
            (int (__cdecl *)(const void *, const void *))LiveStats_ComparePersonalBests);
        numPersonalBests = 0;
        for ( personalBestNum = 0; personalBestNum < LiveStats_GetNumPersonalBests(controllerIndex); ++personalBestNum )
        {
            if ( LiveStats_GetPersonalBestRating(controllerIndex, personalBestNum) )
                ++numPersonalBests;
        }
        s_currentPersonalBest[controllerIndex] = numPersonalBests;
    }
}

int __cdecl LiveStats_ComparePersonalBests(unsigned int *arg0, unsigned int *arg1)
{
    int bestVal1; // [esp+4h] [ebp-Ch]
    int bestVal0; // [esp+8h] [ebp-8h]

    bestVal0 = isPositive(arg0[3]);
    bestVal1 = isPositive(arg1[3]);
    if ( bestVal0 == bestVal1 )
        return arg1[4] - arg0[4];
    else
        return bestVal1 - bestVal0;
}

int __cdecl isPositive(int value)
{
    return (value > 0) - (value < 0);
}

char __cdecl LiveStats_ProVersionUnlocked(int itemIndex, char *statsBuffer)
{
    int proChallenges; // [esp+0h] [ebp-14h]
    ddlState_t searchState; // [esp+4h] [ebp-10h] BYREF

    if ( !statsBuffer
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp", 2717, 0, "%s", "statsBuffer") )
    {
        __debugbreak();
    }
    if ( !statsBuffer )
        return 0;
    for ( proChallenges = 0; proChallenges < 3; ++proChallenges )
    {
        if ( !LiveStats_MoveToProItemVersion(&searchState, itemIndex, proChallenges) )
            return 0;
        if ( !DDL_GetInt(&searchState, statsBuffer) )
            return 0;
    }
    return 1;
}

void __cdecl LiveStats_CompareStatsVsStableBufferCmd()
{
#ifdef KISAK_LIVE
    if ( !s_statsalreadycompared )
    {
        s_statsalreadycompared = 1;
        LiveStats_CompareStatsVsStableBuffer(0);
    }
#endif
}

void __cdecl LiveStats_SortPercentageCompleted(int type)
{
    switch ( type )
    {
        case 1:
        case 7:
            qsort(
                s_sortedChallengeList,
                s_totalNumberOfUnlockedItems,
                0x24u,
                (int (__cdecl *)(const void *, const void *))LiveStats_ChallengeComparePercentageCompleted);
            break;
        case 5:
            qsort(
                s_sortedAttachmentsList,
                s_numSortedAttachments,
                0x24u,
                (int (__cdecl *)(const void *, const void *))LiveStats_ChallengeComparePercentageCompleted);
            break;
        case 4:
            qsort(
                s_sortedWeaponGroupList,
                s_numSortedWeaponGroup,
                0x24u,
                (int (__cdecl *)(const void *, const void *))LiveStats_ChallengeComparePercentageCompleted);
            break;
        default:
            if ( type )
            {
                if ( type == 6 )
                    qsort(
                        s_sortedChallengeList,
                        s_numSortedChallenges,
                        0x24u,
                        (int (__cdecl *)(const void *, const void *))LiveStats_ChallengeComparePercentageCompleted);
            }
            else
            {
                qsort(
                    s_sortedGlobalChallengeList,
                    s_numSortedGlobalChallenges,
                    0x24u,
                    (int (__cdecl *)(const void *, const void *))LiveStats_ChallengeComparePercentageCompleted);
            }
            break;
    }
}

int __cdecl LiveStats_ChallengeComparePercentageCompleted(const challengeInfo_t *arg0, const challengeInfo_t *arg1)
{
    return LiveStats_SecondaryChallengeComparison(
                     arg0,
                     arg1,
                     100 * arg1->currentValue / arg1->targetValue - 100 * arg0->currentValue / arg0->targetValue);
}

int __cdecl LiveStats_SecondaryChallengeComparison(
                const challengeInfo_t *challenge0,
                const challengeInfo_t *challenge1,
                int delta)
{
    int deltaa; // [esp+14h] [ebp+10h]

    if ( challenge0->flags != challenge1->flags )
        return challenge0->flags - challenge1->flags;
    if ( delta )
        return delta;
    if ( challenge0->type == MILESTONE_GLOBAL
        && challenge1->type == MILESTONE_GLOBAL
        && challenge0->index != challenge1->index )
    {
        return challenge0->index - challenge1->index;
    }
    deltaa = I_stricmp(challenge0->name, challenge1->name);
    if ( deltaa )
        return deltaa;
    else
        return challenge0->index - challenge1->index;
}

void __cdecl LiveStats_SortNearestToCompletion(int type)
{
    switch ( type )
    {
        case 1:
        case 7:
            qsort(
                s_sortedChallengeList,
                s_totalNumberOfUnlockedItems,
                0x24u,
                (int (__cdecl *)(const void *, const void *))LiveStats_ChallengeCompareNumToTargetValue);
            break;
        case 5:
            qsort(
                s_sortedAttachmentsList,
                s_numSortedAttachments,
                0x24u,
                (int (__cdecl *)(const void *, const void *))LiveStats_ChallengeCompareNumToTargetValue);
            break;
        case 4:
            qsort(
                s_sortedWeaponGroupList,
                s_numSortedWeaponGroup,
                0x24u,
                (int (__cdecl *)(const void *, const void *))LiveStats_ChallengeCompareNumToTargetValue);
            break;
        default:
            if ( type )
            {
                if ( type == 6 )
                    qsort(
                        s_sortedChallengeList,
                        s_numSortedChallenges,
                        0x24u,
                        (int (__cdecl *)(const void *, const void *))LiveStats_ChallengeCompareNumToTargetValue);
            }
            else
            {
                qsort(
                    s_sortedGlobalChallengeList,
                    s_numSortedGlobalChallenges,
                    0x24u,
                    (int (__cdecl *)(const void *, const void *))LiveStats_ChallengeCompareNumToTargetValue);
            }
            break;
    }
}

int __cdecl LiveStats_ChallengeCompareNumToTargetValue(const challengeInfo_t *arg0, const challengeInfo_t *arg1)
{
    return LiveStats_SecondaryChallengeComparison(arg0, arg1, arg0->numToTargetValue - arg1->numToTargetValue);
}

void __cdecl LiveStats_SortReward(int type)
{
    switch ( type )
    {
        case 1:
        case 7:
            qsort(
                s_sortedChallengeList,
                s_totalNumberOfUnlockedItems,
                0x24u,
                (int (__cdecl *)(const void *, const void *))LiveStats_ChallengeCompareReward);
            break;
        case 5:
            qsort(
                s_sortedAttachmentsList,
                s_numSortedAttachments,
                0x24u,
                (int (__cdecl *)(const void *, const void *))LiveStats_ChallengeCompareReward);
            break;
        case 4:
            qsort(
                s_sortedWeaponGroupList,
                s_numSortedWeaponGroup,
                0x24u,
                (int (__cdecl *)(const void *, const void *))LiveStats_ChallengeCompareReward);
            break;
        default:
            if ( type )
            {
                if ( type == 6 )
                    qsort(
                        s_sortedChallengeList,
                        s_numSortedChallenges,
                        0x24u,
                        (int (__cdecl *)(const void *, const void *))LiveStats_ChallengeCompareReward);
            }
            else
            {
                qsort(
                    s_sortedGlobalChallengeList,
                    s_numSortedGlobalChallenges,
                    0x24u,
                    (int (__cdecl *)(const void *, const void *))LiveStats_ChallengeCompareReward);
            }
            break;
    }
}

int __cdecl LiveStats_ChallengeCompareReward(const challengeInfo_t *arg0, const challengeInfo_t *arg1)
{
    return LiveStats_SecondaryChallengeComparison(
                     arg0,
                     arg1,
                     arg1->cpReward + arg1->xpReward - (arg0->cpReward + arg0->xpReward));
}

void __cdecl LiveStats_SortDefault(int type)
{
    switch ( type )
    {
        case 1:
        case 7:
            qsort(
                s_sortedChallengeList,
                s_totalNumberOfUnlockedItems,
                0x24u,
                (int (__cdecl *)(const void *, const void *))LiveStats_ChallengeCompareDefault);
            break;
        case 5:
            qsort(
                s_sortedAttachmentsList,
                s_numSortedAttachments,
                0x24u,
                (int (__cdecl *)(const void *, const void *))LiveStats_ChallengeCompareDefault);
            break;
        case 4:
            qsort(
                s_sortedWeaponGroupList,
                s_numSortedWeaponGroup,
                0x24u,
                (int (__cdecl *)(const void *, const void *))LiveStats_ChallengeCompareDefault);
            break;
        default:
            if ( type )
            {
                if ( type == 6 )
                    qsort(
                        s_sortedChallengeList,
                        s_numSortedChallenges,
                        0x24u,
                        (int (__cdecl *)(const void *, const void *))LiveStats_ChallengeCompareDefault);
            }
            else
            {
                qsort(
                    s_sortedGlobalChallengeList,
                    s_numSortedGlobalChallenges,
                    0x24u,
                    (int (__cdecl *)(const void *, const void *))LiveStats_ChallengeCompareDefault);
            }
            break;
    }
}

int __cdecl LiveStats_ChallengeCompareDefault(const challengeInfo_t *arg0, const challengeInfo_t *arg1)
{
    return LiveStats_SecondaryChallengeComparison(arg0, arg1, 0);
}

void __cdecl LiveStats_AddChallenge(
                int controllerIndex,
                char *liveStatsBuffer,
                int itemNumber,
                statsMilestoneTypes_t challengeType)
{
    char *v4; // eax
    ddlState_t searchStateStats; // [esp+0h] [ebp-10h] BYREF

    v4 = va("%d", itemNumber);
    if ( DDL_MoveTo(&g_statsRootState, &searchStateStats, 3, "ItemStats", v4)
        && DDL_IterateFirst(&searchStateStats, &searchStateStats) )
    {
        do
            LiveStats_AddItemChallenge(controllerIndex, liveStatsBuffer, &searchStateStats, itemNumber, challengeType);
        while ( DDL_IterateNext(&searchStateStats, &searchStateStats) );
    }
}

char __cdecl LiveStats_AddItemChallenge(
                int controllerIndex,
                char *liveStatsBuffer,
                ddlState_t *searchStateStats,
                int itemIndex,
                statsMilestoneTypes_t type)
{
    challengeInfo_t *challenge; // [esp+0h] [ebp-20h]
    int rowNum; // [esp+4h] [ebp-1Ch]
    int currentValue; // [esp+8h] [ebp-18h] BYREF
    unsigned int currentMilestone; // [esp+Ch] [ebp-14h]
    const char *statName; // [esp+10h] [ebp-10h]
    const char *name; // [esp+14h] [ebp-Ch]
    const char *group; // [esp+18h] [ebp-8h]
    const StringTable *statsMilestonestable; // [esp+1Ch] [ebp-4h] BYREF

    if ( (!liveStatsBuffer || !searchStateStats)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    1542,
                    0,
                    "%s",
                    "liveStatsBuffer && searchStateStats") )
    {
        __debugbreak();
    }
    if ( s_numSortedChallenges >= 0x3E8 || s_totalNumberOfUnlockedItems >= 0x3E8 )
    {
        Com_PrintError(16, "Tried to add too many stats milestones to list - only %d are supported.\n", 1000);
        return 0;
    }
    if ( !liveStatsBuffer || !searchStateStats )
        return 0;
    currentMilestone = LiveStats_GetItemStat(searchStateStats, "challengeTier", liveStatsBuffer);
    statsMilestonestable = 0;
    name = searchStateStats->ddl->enumList[searchStateStats->member->enumIndex].members[searchStateStats->arrayIndex];
    group = BG_UnlockablesGetItemGroup(itemIndex);
    statName = BG_UnlockablesGetItemName(itemIndex);
    if ( !I_stricmp(statName, "WEAPON_STRELA") )
        return 0;
    if ( !name || !group || !*name || !*group )
        return 0;
    rowNum = LiveStats_GetMilestoneRowNum((XAssetHeader *)&statsMilestonestable, currentMilestone, group, name);
    if ( statsMilestonestable )
    {
        if ( rowNum != -1 )
        {
            challenge = &s_sortedChallengeList[s_numSortedChallenges];
            currentValue = LiveStats_GetItemStat(searchStateStats, "challengeValue", liveStatsBuffer);
            LiveStats_CheckCurrentValue(statsMilestonestable, rowNum, 2, &currentValue);
            LiveStats_AddChallengeForRow(
                statsMilestonestable,
                challenge,
                currentMilestone,
                currentValue,
                rowNum,
                name,
                group,
                type);
            challenge->index = itemIndex;
            LiveStats_SetChallengeFlags(controllerIndex, itemIndex, challenge);
            ++s_numSortedChallenges;
            if ( !BG_UnlockablesIsItemLocked(controllerIndex, itemIndex) && (challenge->flags & 4) == 0 )
                ++s_totalNumberOfUnlockedItems;
        }
    }
    return 1;
}

void __cdecl LiveStats_AddChallengeForRow(
                const StringTable *statsMilestonestable,
                challengeInfo_t *challenge,
                int currentMilestone,
                int currentValue,
                int rowNum,
                const char *name,
                const char *statTypeName,
                statsMilestoneTypes_t type)
{
    const char *ColumnValueForRow; // eax
    const char *v9; // eax
    int v10; // eax
    const char *v11; // eax
    int v12; // eax
    const char *v13; // eax
    unsigned __int8 v14; // al
    const char *v15; // eax
    const char *v16; // eax
    int v17; // eax
    const char *v18; // [esp-Ch] [ebp-10h]
    const char *v19; // [esp-4h] [ebp-8h]
    int targetValue; // [esp+0h] [ebp-4h]

    if ( !challenge
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp", 1387, 0, "%s", "challenge") )
    {
        __debugbreak();
    }
    if ( !statsMilestonestable
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    1388,
                    0,
                    "%s",
                    "statsMilestonestable") )
    {
        __debugbreak();
    }
    if ( rowNum == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    1389,
                    0,
                    "%s",
                    "rowNum != STRINGTABLE_ROW_NOT_FOUND") )
    {
        __debugbreak();
    }
    if ( challenge )
    {
        if ( statsMilestonestable )
        {
            if ( rowNum != -1 )
            {
                ColumnValueForRow = StringTable_GetColumnValueForRow(statsMilestonestable, rowNum, 2);
                targetValue = atoi(ColumnValueForRow);
                challenge->targetValue = targetValue;
                challenge->currentValue = currentValue;
                challenge->numToTargetValue = targetValue - challenge->currentValue;
                v9 = StringTable_GetColumnValueForRow(statsMilestonestable, rowNum, 6);
                v10 = atoi(v9);
                challenge->xpReward = v10;
                v11 = StringTable_GetColumnValueForRow(statsMilestonestable, rowNum, 7);
                v12 = atoi(v11);
                challenge->cpReward = v12;
                challenge->milestone = currentMilestone;
                challenge->type = type;
                challenge->name = name;
                challenge->prevTarget = 0;
                v13 = StringTable_GetColumnValueForRow(statsMilestonestable, rowNum, 0);
                v14 = atoi(v13);
                challenge->index = v14;
                if ( currentMilestone > 0 )
                {
                    v19 = name;
                    v18 = statTypeName;
                    v15 = va("%d", currentMilestone - 1);
                    rowNum = CL_GetMilestoneRowNum((XAssetHeader *)&statsMilestonestable, 1, v15, 3, v18, 4, v19);
                    if ( rowNum != -1 )
                    {
                        v16 = StringTable_GetColumnValueForRow(statsMilestonestable, rowNum, 2);
                        v17 = atoi(v16);
                        challenge->prevTarget = v17;
                    }
                }
            }
        }
    }
}

void __cdecl LiveStats_SetChallengeFlags(int controllerIndex, int itemIndex, challengeInfo_t *challenge)
{
    challenge->flags = 0;
    challenge->flags |= BG_UnlockablesIsItemClassifiedGeneric(controllerIndex, itemIndex) ? 4 : 0;
    challenge->flags |= BG_UnlockablesIsItemLocked(controllerIndex, itemIndex) ? 2 : 0;
    challenge->flags |= !BG_UnlockablesIsItemPurchased(controllerIndex, itemIndex);
}

void __cdecl LiveStats_BuildChallengeListWithLocks(
                int controllerIndex,
                char *liveStatsBuffer,
                statsMilestoneTypes_t challengeType,
                int weaponGroup)
{
    unsigned int itemNumber; // [esp+0h] [ebp-8h]
    int lock; // [esp+4h] [ebp-4h]

    s_numSortedChallenges = 0;
    s_totalNumberOfUnlockedItems = 0;
    for ( lock = 0; lock < 4; ++lock )
    {
        for ( itemNumber = 0; itemNumber < 0x100; ++itemNumber )
        {
            if ( BG_UnlockablesIsItemValidNotNull(itemNumber) && LiveStats_AddItemForLock(controllerIndex, itemNumber, lock) )
            {
                if ( challengeType == MILESTONE_WEAPON )
                {
                    if ( weaponGroup == BG_UnlockablesGetItemGroupEnum(itemNumber)
                        && !BG_UnlockablesItemIsDualWield(itemNumber)
                        && BG_UnlockablesGetItemCost(itemNumber) != -1 )
                    {
                        LiveStats_AddChallenge(controllerIndex, liveStatsBuffer, itemNumber, MILESTONE_WEAPON);
                    }
                }
                else
                {
                    if ( BG_UnlockablesGetItemGroupEnum(itemNumber) == 10
                        && !BG_UnlockablesItemIsDualWield(itemNumber)
                        && !BG_UnlockablesIsItemPassive(itemNumber)
                        && BG_UnlockablesGetItemLoadoutSlot(itemNumber) == 30 )
                    {
                        LiveStats_AddChallenge(controllerIndex, liveStatsBuffer, itemNumber, challengeType);
                    }
                    if ( BG_UnlockablesGetItemGroupEnum(itemNumber) == 9
                        && !BG_UnlockablesItemIsDualWield(itemNumber)
                        && !BG_UnlockablesIsItemPassive(itemNumber)
                        && BG_UnlockablesGetItemLoadoutSlot(itemNumber) == 32 )
                    {
                        LiveStats_AddChallenge(controllerIndex, liveStatsBuffer, itemNumber, challengeType);
                    }
                }
            }
        }
    }
}

bool __cdecl LiveStats_AddItemForLock(int controllerIndex, int itemNumber, int lock)
{
    bool IsItemClassifiedGeneric; // al
    bool result; // al
    bool isPurchased; // [esp+5h] [ebp-3h]
    bool isUnlocked; // [esp+6h] [ebp-2h]

    isPurchased = BG_UnlockablesIsItemPurchased(controllerIndex, itemNumber);
    isUnlocked = !BG_UnlockablesIsItemLocked(controllerIndex, itemNumber);
    IsItemClassifiedGeneric = BG_UnlockablesIsItemClassifiedGeneric(controllerIndex, itemNumber);
    switch ( lock )
    {
        case 0:
            if ( !isPurchased )
                goto LABEL_16;
            result = 1;
            break;
        case 1:
            if ( isPurchased || !isUnlocked || IsItemClassifiedGeneric )
                goto LABEL_16;
            result = 1;
            break;
        case 2:
            if ( isUnlocked || IsItemClassifiedGeneric )
                goto LABEL_16;
            result = 1;
            break;
        case 3:
            if ( !IsItemClassifiedGeneric )
                goto LABEL_16;
            result = 1;
            break;
        default:
LABEL_16:
            result = 0;
            break;
    }
    return result;
}

void __cdecl LiveStats_BuildChallengeListCmd()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    challengeSortOrder_e sortOrder; // [esp+20h] [ebp-10h]
    itemGroup_t weaponGroup; // [esp+28h] [ebp-8h]
    statsMilestoneTypes_t challengeType; // [esp+2Ch] [ebp-4h]

    if ( Cmd_Argc() >= 4 )
    {
        v0 = Cmd_Argv(1);
        challengeType = (statsMilestoneTypes_t)atoi(v0);
        v1 = Cmd_Argv(2);
        sortOrder = (challengeSortOrder_e)atoi(v1);
        v2 = Cmd_Argv(3);
        weaponGroup = (itemGroup_t)atoi(v2);
        if ( challengeType >= MILESTONE_COUNT || challengeType < MILESTONE_INVALID )
        {
            Com_PrintError(15, "Invalid Challenge Type %d\n", challengeType);
            challengeType = MILESTONE_INVALID;
        }
        if ( sortOrder >= SORTORDER_COUNT || sortOrder < SORTORDER_INVALID )
        {
            Com_PrintError(15, "Invalid Sort Order %d\n", sortOrder);
            sortOrder = SORTORDER_FIRST;
        }
        LiveStats_BuildChallengeList(0, challengeType, sortOrder, weaponGroup);
    }
    else
    {
        Com_PrintError(
            15,
            "BuildChallengeList usage: BuildChallengeList <type> <sortorder> <weapon group> <loadoutSlot(optional)>\n");
    }
}

void __cdecl LiveStats_BuildChallengeList(
                int controllerIndex,
                statsMilestoneTypes_t challengeType,
                challengeSortOrder_e sortOrder,
                itemGroup_t weaponGroup)
{
    const char *ItemGroupName; // eax
    ddlState_t searchStateGroupStats; // [esp+0h] [ebp-24h] BYREF
    char *liveStatsBuffer; // [esp+10h] [ebp-14h]
    ddlState_t searchStateStats; // [esp+14h] [ebp-10h] BYREF

    liveStatsBuffer = (char *)LiveStorage_GetStatsBuffer(0, STATS_LOCATION_GLOBAL, 1);
    if ( challengeType == MILESTONE_WEAPON || challengeType == MILESTONE_GRENADES )
    {
        LiveStats_BuildChallengeListWithLocks(controllerIndex, liveStatsBuffer, challengeType, weaponGroup);
    }
    else if ( challengeType == MILESTONE_GROUP )
    {
        s_numSortedWeaponGroup = 0;
        ItemGroupName = BG_UnlockablesGetItemGroupName(weaponGroup);
        if ( DDL_MoveTo(&g_statsRootState, &searchStateStats, 2, "GroupStats", ItemGroupName) )
        {
            if ( DDL_MoveTo(&searchStateStats, &searchStateGroupStats, 1, "stats")
                && DDL_IterateFirst(&searchStateGroupStats, &searchStateGroupStats) )
            {
                do
                    LiveStats_AddGroupChallenge(liveStatsBuffer, &searchStateGroupStats, weaponGroup);
                while ( DDL_IterateNext(&searchStateGroupStats, &searchStateGroupStats) );
            }
        }
    }
    if ( sortOrder != SORTORDER_DEFAULT && (challengeType == MILESTONE_WEAPON || challengeType == MILESTONE_GRENADES) )
    {
        LiveStats_SortChallenges(sortOrder, challengeType);
    }
    else if ( sortOrder != SORTORDER_DEFAULT && challengeType == MILESTONE_GROUP )
    {
        LiveStats_SortChallenges(sortOrder, 4);
    }
}

char __cdecl LiveStats_AddGroupChallenge(char *liveStatsBuffer, ddlState_t *searchStateStats, int groupIndex)
{
    challengeInfo_t *challenge; // [esp+0h] [ebp-18h]
    int rowNum; // [esp+4h] [ebp-14h]
    int currentValue; // [esp+8h] [ebp-10h] BYREF
    unsigned int currentMilestone; // [esp+Ch] [ebp-Ch]
    const char *name; // [esp+10h] [ebp-8h]
    const StringTable *statsMilestonestable; // [esp+14h] [ebp-4h] BYREF

    if ( (!liveStatsBuffer || !searchStateStats)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    1660,
                    0,
                    "%s",
                    "liveStatsBuffer && searchStateStats") )
    {
        __debugbreak();
    }
    if ( s_numSortedWeaponGroup < 0x10 )
    {
        if ( liveStatsBuffer && searchStateStats )
        {
            if ( groupIndex == -1 || groupIndex == 14 || groupIndex == 17 || groupIndex == 19 || groupIndex == 21 )
            {
                return 0;
            }
            else
            {
                currentMilestone = LiveStats_GetItemStat(searchStateStats, "challengeTier", liveStatsBuffer);
                statsMilestonestable = 0;
                name = searchStateStats->ddl->enumList[searchStateStats->member->enumIndex].members[searchStateStats->arrayIndex];
                if ( (groupIndex == 6 || groupIndex == 7) && !I_stricmp("headshots", name) )
                {
                    return 0;
                }
                else
                {
                    rowNum = LiveStats_GetMilestoneRowNum((XAssetHeader *)&statsMilestonestable, currentMilestone, "group", name);
                    if ( statsMilestonestable )
                    {
                        if ( rowNum != -1 )
                        {
                            challenge = &s_sortedWeaponGroupList[s_numSortedWeaponGroup];
                            currentValue = LiveStats_GetItemStat(searchStateStats, "challengeValue", liveStatsBuffer);
                            LiveStats_CheckCurrentValue(statsMilestonestable, rowNum, 2, &currentValue);
                            LiveStats_AddChallengeForRow(
                                statsMilestonestable,
                                challenge,
                                currentMilestone,
                                currentValue,
                                rowNum,
                                name,
                                "group",
                                MILESTONE_GROUP);
                            challenge->index = groupIndex;
                            ++s_numSortedWeaponGroup;
                        }
                    }
                    return 1;
                }
            }
        }
        else
        {
            return 0;
        }
    }
    else
    {
        Com_PrintError(16, "Tried to add too many stats milestones to list - only %d are supported.\n", 1000);
        return 0;
    }
}

void __cdecl LiveStats_SortChallenges(challengeSortOrder_e sortOrder, int type)
{
    switch ( sortOrder )
    {
        case SORTORDER_FIRST:
            LiveStats_SortNearestToCompletion(type);
            break;
        case SORTORDER_REWARD:
            LiveStats_SortReward(type);
            break;
        case SORTORDER_PERCENTAGECOMPLETED:
            LiveStats_SortPercentageCompleted(type);
            break;
        case SORTORDER_DEFAULT:
            LiveStats_SortDefault(type);
            break;
        default:
            return;
    }
}

bool __cdecl FilterKillstreakChallengesType(ddlState_t *searchStateStats, const char *killstreakType)
{
    const char *statType; // [esp+0h] [ebp-4h]

    statType = searchStateStats->ddl->enumList[searchStateStats->member->enumIndex].members[searchStateStats->arrayIndex];
    if ( !I_stricmp(killstreakType, "KILLSTREAK_RECON") || !I_stricmp(killstreakType, "KILLSTREAK_COUNTER_UAV") )
        return !I_stricmp(statType, "destroyed") || !I_stricmp(statType, "used");
    if ( !I_stricmp(killstreakType, "KILLSTREAK_MORTAR")
        || !I_stricmp(killstreakType, "KILLSTREAK_NAPALM")
        || !I_stricmp(killstreakType, "KILLSTREAK_AIRSTRIKE") )
    {
        return !I_stricmp(statType, "kills") || !I_stricmp(statType, "used");
    }
    return I_stricmp(killstreakType, "KILLSTREAK_SATELLITE") || I_stricmp(statType, "used") == 0;
}

char __cdecl FilterGamemodeStat(ddlState_t *searchGamemodeStateStats, const char *gamemode)
{
    const char *statType; // [esp+0h] [ebp-4h]

    statType = searchGamemodeStateStats->ddl->enumList[searchGamemodeStateStats->member->enumIndex].members[searchGamemodeStateStats->arrayIndex];
    if ( !I_stricmp(statType, "SCORE")
        || !I_stricmp(statType, "TOP3")
        || !I_stricmp(statType, "TOPPLAYER")
        || !I_stricmp(statType, "WINS") )
    {
        return 1;
    }
    if ( !I_stricmp(gamemode, "dom") || !I_stricmp(gamemode, "koth") )
        return !I_stricmp(statType, "CAPTURES") || !I_stricmp(statType, "OFFENDS") || !I_stricmp(statType, "DEFENDS");
    if ( !I_stricmp(gamemode, "SD")
        || !I_stricmp(gamemode, "HCSD")
        || !I_stricmp(gamemode, "DEM")
        || !I_stricmp(gamemode, "SAB") )
    {
        return !I_stricmp(statType, "PLANTS")
                || !I_stricmp(statType, "DEFUSES")
                || !I_stricmp(statType, "DESTRUCTIONS")
                || !I_stricmp(statType, "OFFENDS")
                || !I_stricmp(statType, "DEFENDS");
    }
    if ( !I_stricmp(gamemode, "CTF") )
        return !I_stricmp(statType, "CAPTURES")
                || !I_stricmp(statType, "RETURNS")
                || !I_stricmp(statType, "OFFENDS")
                || !I_stricmp(statType, "DEFENDS");
    return 0;
}

void __cdecl LiveStats_BuildKillstreakChallengeListCmd()
{
    const char *v0; // eax
    const char *v1; // eax
    challengeSortOrder_e sortOrder; // [esp+18h] [ebp-8h]

    if ( Cmd_Argc() == 3 )
    {
        v0 = Cmd_Argv(2);
        sortOrder = (challengeSortOrder_e)atoi(v0);
        if ( sortOrder >= SORTORDER_COUNT || sortOrder < SORTORDER_INVALID )
        {
            Com_PrintError(15, "Invalid Sort Order %d\n", sortOrder);
            sortOrder = SORTORDER_FIRST;
        }
        v1 = Cmd_Argv(1);
        LiveStats_BuildKillstreakChallengeList(0, v1, sortOrder);
    }
    else
    {
        Com_PrintError(15, "BuildChallengeList usage: BuildKillstreakChallengeList <KillstreakType> <sortorder> \n");
    }
}

void __cdecl LiveStats_BuildKillstreakChallengeList(
                int controllerIndex,
                const char *killstreakType,
                challengeSortOrder_e sortOrder)
{
    const char *ItemName; // eax
    char *v4; // eax
    unsigned int itemNumber; // [esp+0h] [ebp-18h]
    persistentStats *liveStatsBuffer; // [esp+4h] [ebp-14h]
    ddlState_t searchStateStats; // [esp+8h] [ebp-10h] BYREF

    liveStatsBuffer = LiveStorage_GetStatsBuffer(0, STATS_LOCATION_GLOBAL, 1);
    s_numSortedChallenges = 0;
    for ( itemNumber = 0; itemNumber < 0x100; ++itemNumber )
    {
        if ( BG_UnlockablesIsItemValidNotNull(itemNumber) )
        {
            ItemName = BG_UnlockablesGetItemName(itemNumber);
            if ( !I_stricmp(killstreakType, ItemName) && BG_UnlockablesGetItemCost(itemNumber) != -1 )
            {
                v4 = va("%d", itemNumber);
                if ( DDL_MoveTo(&g_statsRootState, &searchStateStats, 3, "ItemStats", v4) )
                {
                    if ( DDL_IterateFirst(&searchStateStats, &searchStateStats) )
                    {
                        do
                        {
                            if ( FilterKillstreakChallengesType(&searchStateStats, killstreakType) )
                                LiveStats_AddItemChallenge(
                                    controllerIndex,
                                    (char *)liveStatsBuffer,
                                    &searchStateStats,
                                    itemNumber,
                                    MILESTONE_KILLSTREAKS);
                        }
                        while ( DDL_IterateNext(&searchStateStats, &searchStateStats) );
                    }
                }
            }
        }
    }
    if ( killstreakType )
    {
        if ( I_stricmp(killstreakType, "KILLSTREAK_RECON") )
        {
            if ( I_stricmp(killstreakType, "KILLSTREAK_SATELLITE") )
            {
                if ( !I_stricmp(killstreakType, "KILLSTREAK_COUNTER_UAV") )
                    LiveStats_AddGlobalKillstreakChallenges((char *)liveStatsBuffer, "KILLSTREAK_KILLS_COUNTER_U2");
            }
            else
            {
                LiveStats_AddGlobalKillstreakChallenges((char *)liveStatsBuffer, "KILLSTREAK_KILLS_SATELLITE");
            }
        }
        else
        {
            LiveStats_AddGlobalKillstreakChallenges((char *)liveStatsBuffer, "KILLSTREAK_KILLS_U2");
        }
    }
    if ( sortOrder != SORTORDER_DEFAULT )
        LiveStats_SortChallenges(sortOrder, 6);
}

void __cdecl LiveStats_AddGlobalKillstreakChallenges(char *liveStatsBuffer, const char *killstreakType)
{
    challengeInfo_t *challenge; // [esp+0h] [ebp-14h]
    ddlState_t searchStateStats; // [esp+4h] [ebp-10h] BYREF

    if ( DDL_MoveTo(&g_statsRootState, &searchStateStats, 1, "challengeTier")
        && DDL_IterateFirst(&searchStateStats, &searchStateStats) )
    {
        while ( 1 )
        {
            challenge = &s_sortedChallengeList[s_numSortedChallenges];
            if ( LiveStats_AddGlobalChallenge(liveStatsBuffer, &searchStateStats, challenge, killstreakType) )
            {
                challenge->flags = 0;
                if ( ++s_numSortedChallenges > 0x3E8 )
                    break;
            }
            if ( !DDL_IterateNext(&searchStateStats, &searchStateStats) )
                return;
        }
        Com_PrintError(16, "Tried to add too many killstreak challenges to list - only %d are supported.\n", 999);
    }
}

char __cdecl LiveStats_AddGlobalChallenge(
                char *liveStatsBuffer,
                ddlState_t *searchStateStats,
                challengeInfo_t *challenge,
                const char *globalChallengesType)
{
    int rowNum; // [esp+10h] [ebp-14h]
    int currentValue; // [esp+14h] [ebp-10h] BYREF
    unsigned int currentMilestone; // [esp+18h] [ebp-Ch]
    const char *name; // [esp+1Ch] [ebp-8h]
    const StringTable *statsMilestonestable; // [esp+20h] [ebp-4h] BYREF

    if ( (!liveStatsBuffer || !searchStateStats)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    1948,
                    0,
                    "%s",
                    "liveStatsBuffer && searchStateStats") )
    {
        __debugbreak();
    }
    if ( !liveStatsBuffer || !searchStateStats )
        return 0;
    name = searchStateStats->ddl->enumList[searchStateStats->member->enumIndex].members[searchStateStats->arrayIndex];
    if ( globalChallengesType && I_strnicmp(globalChallengesType, name, strlen(globalChallengesType)) )
        return 0;
    if ( Com_GameMode_IsGameMode(GAMEMODE_BASIC_TRAINING) )
    {
        if ( !I_stricmp(name, "perks_lightweight_offense_medal") )
            return 0;
        if ( !I_stricmp(name, "perks_flak_jacket_demolished") )
            return 0;
        if ( !I_stricmp(name, "perks_marathon_capture_medal") )
            return 0;
        if ( !I_stricmp(name, "perks_ninja_plants") )
            return 0;
    }
    currentMilestone = DDL_GetInt(searchStateStats, liveStatsBuffer);
    statsMilestonestable = 0;
    rowNum = LiveStats_GetMilestoneRowNum((XAssetHeader *)&statsMilestonestable, currentMilestone, "global", name);
    if ( !statsMilestonestable || rowNum == -1 )
        return 0;
    currentValue = LiveStats_GetPlayerStat("ChallengeValue", name, liveStatsBuffer);
    LiveStats_CheckCurrentValue(statsMilestonestable, rowNum, 2, &currentValue);
    LiveStats_AddChallengeForRow(
        statsMilestonestable,
        challenge,
        currentMilestone,
        currentValue,
        rowNum,
        name,
        "global",
        MILESTONE_GLOBAL);
    return 1;
}

void __cdecl LiveStats_BuildLifeTimeGlobalChallengesList(char *liveStatsBuffer)
{
    ddlState_t searchStateStats; // [esp+4h] [ebp-10h] BYREF

    if ( DDL_MoveTo(&g_statsRootState, &searchStateStats, 1, "LifeChallengeTier")
        && DDL_IterateFirst(&searchStateStats, &searchStateStats) )
    {
        while ( 1 )
        {
            if ( LiveStats_AddGlobalLifetimeChallenge(
                         liveStatsBuffer,
                         &searchStateStats,
                         &s_sortedGlobalChallengeList[s_numSortedGlobalChallenges]) )
            {
                if ( ++s_numSortedGlobalChallenges > 0x32 )
                    break;
            }
            if ( !DDL_IterateNext(&searchStateStats, &searchStateStats) )
                return;
        }
        Com_PrintError(16, "Tried to add too many stats perks to list - only %d are supported.\n", 49);
    }
}

char __cdecl LiveStats_AddGlobalLifetimeChallenge(
                char *liveStatsBuffer,
                ddlState_t *searchStateStats,
                challengeInfo_t *challenge)
{
    const char *v3; // eax
    int rowNum; // [esp+0h] [ebp-14h]
    int currentValue; // [esp+4h] [ebp-10h] BYREF
    unsigned int currentMilestone; // [esp+8h] [ebp-Ch]
    const char *name; // [esp+Ch] [ebp-8h]
    const StringTable *statsMilestonestable; // [esp+10h] [ebp-4h] BYREF

    if ( (!liveStatsBuffer || !searchStateStats)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    2044,
                    0,
                    "%s",
                    "liveStatsBuffer && searchStateStats") )
    {
        __debugbreak();
    }
    if ( !liveStatsBuffer )
        return 0;
    if ( !searchStateStats )
        return 0;
    name = searchStateStats->ddl->enumList[searchStateStats->member->enumIndex].members[searchStateStats->arrayIndex];
    currentMilestone = DDL_GetInt(searchStateStats, liveStatsBuffer);
    statsMilestonestable = 0;
    v3 = va("%s%s", "lifetime_", name);
    rowNum = LiveStats_GetMilestoneRowNum((XAssetHeader *)&statsMilestonestable, currentMilestone, "global", v3);
    if ( !statsMilestonestable || rowNum == -1 )
        return 0;
    currentValue = LiveStats_GetPlayerStat("PlayerStatsList", name, liveStatsBuffer);
    LiveStats_CheckCurrentValue(statsMilestonestable, rowNum, 2, &currentValue);
    LiveStats_AddChallengeForRow(
        statsMilestonestable,
        challenge,
        currentMilestone,
        currentValue,
        rowNum,
        name,
        "global",
        MILESTONE_GLOBAL);
    return 1;
}

void __cdecl LiveStats_BuildGamemodeChallengesList(int controllerIndex, char *liveStatsBuffer, const char *gamemode)
{
    ddlState_t searchStateStats; // [esp+0h] [ebp-20h] BYREF
    ddlState_t searchGamemodeStateStats; // [esp+10h] [ebp-10h] BYREF

    if ( DDL_MoveTo(&g_statsRootState, &searchStateStats, 2, "PlayerChallengeByGameMode", gamemode)
        && DDL_MoveTo(&g_statsRootState, &searchGamemodeStateStats, 3, "CurrentGameModeMilestone", gamemode, "milestones")
        && DDL_IterateFirst(&searchGamemodeStateStats, &searchGamemodeStateStats) )
    {
        do
        {
            if ( FilterGamemodeStat(&searchGamemodeStateStats, gamemode) )
                LiveStats_AddGamemodeChallenge(controllerIndex, liveStatsBuffer, &searchStateStats, &searchGamemodeStateStats);
        }
        while ( DDL_IterateNext(&searchGamemodeStateStats, &searchGamemodeStateStats) );
    }
}

char __cdecl LiveStats_AddGamemodeChallenge(
                int controllerIndex,
                char *liveStatsBuffer,
                ddlState_t *searchStateStats,
                ddlState_t *searchGameModeStateStats)
{
    challengeInfo_t *challenge; // [esp+0h] [ebp-18h]
    int rowNum; // [esp+4h] [ebp-14h]
    int currentValue; // [esp+8h] [ebp-10h] BYREF
    unsigned int currentMilestone; // [esp+Ch] [ebp-Ch]
    const char *name; // [esp+10h] [ebp-8h]
    const StringTable *statsMilestonestable; // [esp+14h] [ebp-4h] BYREF

    if ( (!liveStatsBuffer || !searchGameModeStateStats || !searchStateStats)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    2005,
                    0,
                    "%s",
                    "liveStatsBuffer && searchGameModeStateStats && searchStateStats") )
    {
        __debugbreak();
    }
    if ( s_numSortedGlobalChallenges < 0x20 )
    {
        if ( liveStatsBuffer
            && searchGameModeStateStats
            && searchStateStats
            && (currentMilestone = DDL_GetInt(searchGameModeStateStats, liveStatsBuffer),
                    statsMilestonestable = 0,
                    (name = searchGameModeStateStats->ddl->enumList[searchGameModeStateStats->member->enumIndex].members[searchGameModeStateStats->arrayIndex]) != 0)
            && *name )
        {
            rowNum = LiveStats_GetMilestoneRowNum((XAssetHeader *)&statsMilestonestable, currentMilestone, "gamemode", name);
            if ( statsMilestonestable )
            {
                if ( rowNum != -1 )
                {
                    challenge = &s_sortedGlobalChallengeList[s_numSortedGlobalChallenges];
                    currentValue = LiveStats_GetItemStat(searchStateStats, name, liveStatsBuffer);
                    LiveStats_CheckCurrentValue(statsMilestonestable, rowNum, 2, &currentValue);
                    LiveStats_AddChallengeForRow(
                        statsMilestonestable,
                        challenge,
                        currentMilestone,
                        currentValue,
                        rowNum,
                        name,
                        "gamemode",
                        MILESTONE_GAMEMODE);
                    ++s_numSortedGlobalChallenges;
                }
            }
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        Com_PrintError(16, "Tried to add too many stats milestones to list - only %d are supported.\n", 32);
        return 0;
    }
}

void __cdecl LiveStats_BuildGlobalChallengesListCmd()
{
    const char *v0; // eax
    const char *v1; // eax
    challengeSortOrder_e v2; // [esp-4h] [ebp-24h]

    if ( Cmd_Argc() == 3 )
    {
        v0 = Cmd_Argv(2);
        v2 = (challengeSortOrder_e)atoi(v0);
        v1 = Cmd_Argv(1);
        LiveStats_BuildGlobalChallengesList(0, v1, v2);
    }
    else
    {
        Com_PrintError(15, "BuildGlobalChallengesList usage: BuildGlobalChallengesList <globalchallengestype>\n");
    }
}

void __cdecl LiveStats_BuildGlobalChallengesList(
                int controllerIndex,
                const char *globalChallengesType,
                challengeSortOrder_e sortOrder)
{
    persistentStats *liveStatsBuffer; // [esp+4h] [ebp-14h]
    ddlState_t searchStateStats; // [esp+8h] [ebp-10h] BYREF

    liveStatsBuffer = LiveStorage_GetStatsBuffer(0, STATS_LOCATION_GLOBAL, 1);
    s_numSortedGlobalChallenges = 0;
    if ( !globalChallengesType || !I_stricmp(globalChallengesType, "LIFETIME_") )
    {
        if ( globalChallengesType && !I_stricmp(globalChallengesType, "LIFETIME_") )
            LiveStats_BuildLifeTimeGlobalChallengesList((char *)liveStatsBuffer);
        goto LABEL_15;
    }
    if ( !DDL_MoveTo(&g_statsRootState, &searchStateStats, 1, "challengeTier")
        || !DDL_IterateFirst(&searchStateStats, &searchStateStats) )
    {
LABEL_9:
        if ( StringBeginsWith("GM_", globalChallengesType) )
            LiveStats_BuildGamemodeChallengesList(
                controllerIndex,
                (char *)liveStatsBuffer,
                ui_challengeGameMode->current.string);
LABEL_15:
        LiveStats_SortChallenges(sortOrder, 0);
        return;
    }
    while ( 1 )
    {
        if ( LiveStats_AddGlobalChallenge(
                     (char *)liveStatsBuffer,
                     &searchStateStats,
                     &s_sortedGlobalChallengeList[s_numSortedGlobalChallenges],
                     globalChallengesType) )
        {
            if ( ++s_numSortedGlobalChallenges > 0x32 )
                break;
        }
        if ( !DDL_IterateNext(&searchStateStats, &searchStateStats) )
            goto LABEL_9;
    }
    Com_PrintError(16, "Tried to add too many stats perks to list - only %d are supported.\n", 49);
}

void __cdecl LiveStats_AddUnderBarrelAttachments(int controllerIndex, char *liveStatsBuffer)
{
    char *v2; // eax
    unsigned int itemNumber; // [esp+0h] [ebp-14h]
    ddlState_t searchStateStats; // [esp+4h] [ebp-10h] BYREF

    if ( !liveStatsBuffer
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp", 3732, 0, "%s", "liveStatsBuffer") )
    {
        __debugbreak();
    }
    for ( itemNumber = 0; itemNumber < 0x100; ++itemNumber )
    {
        if ( BG_UnlockablesIsItemValidNotNull(itemNumber)
            && BG_UnlockablesGetItemCost(itemNumber) != -1
            && (BG_UnlockablesGetItemGroupEnum(itemNumber) == 11
             || BG_UnlockablesGetItemGroupEnum(itemNumber) == 12
             || BG_UnlockablesGetItemGroupEnum(itemNumber) == 13) )
        {
            v2 = va("%d", itemNumber);
            if ( DDL_MoveTo(&g_statsRootState, &searchStateStats, 3, "ItemStats", v2)
                && DDL_IterateFirst(&searchStateStats, &searchStateStats) )
            {
                do
                    LiveStats_AddUnderBarrelAttachmentChallenge(
                        controllerIndex,
                        liveStatsBuffer,
                        &searchStateStats,
                        itemNumber,
                        MILESTONE_ATTACHMENTS);
                while ( DDL_IterateNext(&searchStateStats, &searchStateStats) );
            }
        }
    }
}

char __cdecl LiveStats_AddUnderBarrelAttachmentChallenge(
                int controllerIndex,
                char *liveStatsBuffer,
                ddlState_t *searchStateStats,
                int itemIndex,
                statsMilestoneTypes_t type)
{
    challengeInfo_t *challenge; // [esp+0h] [ebp-20h]
    const char *attachmentName; // [esp+4h] [ebp-1Ch]
    int rowNum; // [esp+8h] [ebp-18h]
    int currentValue; // [esp+Ch] [ebp-14h] BYREF
    unsigned int currentMilestone; // [esp+10h] [ebp-10h]
    const char *name; // [esp+14h] [ebp-Ch]
    const char *group; // [esp+18h] [ebp-8h]
    const StringTable *statsMilestonestable; // [esp+1Ch] [ebp-4h] BYREF

    if ( (!liveStatsBuffer || !searchStateStats)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    1612,
                    0,
                    "%s",
                    "liveStatsBuffer && searchStateStats") )
    {
        __debugbreak();
    }
    if ( s_numSortedAttachments < 0x18 )
    {
        if ( liveStatsBuffer
            && searchStateStats
            && (currentMilestone = LiveStats_GetItemStat(searchStateStats, "challengeTier", liveStatsBuffer),
                    statsMilestonestable = 0,
                    name = searchStateStats->ddl->enumList[searchStateStats->member->enumIndex].members[searchStateStats->arrayIndex],
                    group = BG_UnlockablesGetItemGroup(itemIndex),
                    name)
            && group
            && *name
            && *group )
        {
            rowNum = LiveStats_GetMilestoneRowNum((XAssetHeader *)&statsMilestonestable, currentMilestone, group, name);
            if ( statsMilestonestable && rowNum != -1 )
            {
                challenge = &s_sortedAttachmentsList[s_numSortedAttachments];
                currentValue = LiveStats_GetItemStat(searchStateStats, "challengeValue", liveStatsBuffer);
                LiveStats_CheckCurrentValue(statsMilestonestable, rowNum, 2, &currentValue);
                LiveStats_AddChallengeForRow(
                    statsMilestonestable,
                    challenge,
                    currentMilestone,
                    currentValue,
                    rowNum,
                    name,
                    group,
                    type);
                challenge->index = itemIndex;
                attachmentName = LiveStats_GetAttachmenNameFromItemIndex(itemIndex);
                challenge->flags = 0;
                if ( attachmentName )
                {
                    if ( !BG_UnlockablesGetAttachmentPurchased(controllerIndex, attachmentName) )
                        challenge->flags |= 1u;
                }
                ++s_numSortedAttachments;
            }
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        Com_PrintError(16, "Tried to add too many stats milestones to list - only %d are supported.\n", 24);
        return 0;
    }
}

const char *__cdecl LiveStats_GetAttachmenNameFromItemIndex(int itemIndex)
{
    int ItemGroupEnum; // [esp+0h] [ebp-8h]

    ItemGroupEnum = BG_UnlockablesGetItemGroupEnum(itemIndex);
    switch ( ItemGroupEnum )
    {
        case 11:
            return "mk";
        case 12:
            return "gl";
        case 13:
            return "ft";
    }
    return 0;
}

void __cdecl LiveStats_BuildAttachmentsListCmd()
{
    const char *v0; // eax
    challengeSortOrder_e sortOrder; // [esp+10h] [ebp-8h]

    if ( Cmd_Argc() == 2 )
    {
        v0 = Cmd_Argv(1);
        sortOrder = (challengeSortOrder_e)atoi(v0);
        LiveStats_BuildAttachmentsList(0, sortOrder);
    }
    else
    {
        Com_PrintError(15, "BuildAttachmentsList usage: BuildAttachmentsList <sortOrder> \n");
    }
}

void __cdecl LiveStats_BuildAttachmentsList(int controllerIndex, challengeSortOrder_e sortOrder)
{
    const char *attachmentName; // [esp+0h] [ebp-28h]
    persistentStats *liveStatsBuffer; // [esp+4h] [ebp-24h]
    ddlState_t searchAttachmentStateStats; // [esp+8h] [ebp-20h] BYREF
    ddlState_t searchStateStats; // [esp+18h] [ebp-10h] BYREF

    liveStatsBuffer = LiveStorage_GetStatsBuffer(0, STATS_LOCATION_GLOBAL, 1);
    s_numSortedAttachments = 0;
    if ( DDL_MoveTo(&g_statsRootState, &searchStateStats, 1, "Attachments")
        && DDL_IterateFirst(&searchStateStats, &searchStateStats) )
    {
        do
        {
            attachmentName = searchStateStats.ddl->enumList[searchStateStats.member->enumIndex].members[searchStateStats.arrayIndex];
            if ( DDL_MoveTo(&searchStateStats, &searchAttachmentStateStats, 1, "stats")
                && DDL_IterateFirst(&searchAttachmentStateStats, &searchAttachmentStateStats) )
            {
                do
                    LiveStats_AddAttachmentsChallenge(
                        controllerIndex,
                        (char *)liveStatsBuffer,
                        &searchAttachmentStateStats,
                        attachmentName);
                while ( DDL_IterateNext(&searchAttachmentStateStats, &searchAttachmentStateStats) );
            }
        }
        while ( DDL_IterateNext(&searchStateStats, &searchStateStats) );
    }
    LiveStats_AddUnderBarrelAttachments(controllerIndex, (char *)liveStatsBuffer);
    LiveStats_SortChallenges(sortOrder, 5);
}

char __cdecl LiveStats_AddAttachmentsChallenge(
                int controllerIndex,
                char *liveStatsBuffer,
                ddlState_t *searchAttachmentStateStats,
                const char *attachmentName)
{
    challengeInfo_t *challenge; // [esp+0h] [ebp-18h]
    int rowNum; // [esp+4h] [ebp-14h]
    int currentValue; // [esp+8h] [ebp-10h] BYREF
    unsigned int currentMilestone; // [esp+Ch] [ebp-Ch]
    const char *name; // [esp+10h] [ebp-8h]
    const StringTable *statsMilestonestable; // [esp+14h] [ebp-4h] BYREF

    if ( (!liveStatsBuffer || !searchAttachmentStateStats)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    1707,
                    0,
                    "%s",
                    "liveStatsBuffer && searchAttachmentStateStats") )
    {
        __debugbreak();
    }
    if ( s_numSortedAttachments < 0x18 )
    {
        if ( liveStatsBuffer
            && searchAttachmentStateStats
            && (currentMilestone = LiveStats_GetItemStat(searchAttachmentStateStats, "challengeTier", liveStatsBuffer),
                    statsMilestonestable = 0,
                    (name = searchAttachmentStateStats->ddl->enumList[searchAttachmentStateStats->member->enumIndex].members[searchAttachmentStateStats->arrayIndex]) != 0)
            && *name
            && attachmentName
            && *attachmentName )
        {
            rowNum = LiveStats_GetMilestoneRowNum((XAssetHeader *)&statsMilestonestable, currentMilestone, "attachment", name);
            if ( statsMilestonestable && rowNum != -1 )
            {
                challenge = &s_sortedAttachmentsList[s_numSortedAttachments];
                currentValue = LiveStats_GetItemStat(searchAttachmentStateStats, "challengeValue", liveStatsBuffer);
                LiveStats_CheckCurrentValue(statsMilestonestable, rowNum, 2, &currentValue);
                LiveStats_AddChallengeForRow(
                    statsMilestonestable,
                    challenge,
                    currentMilestone,
                    currentValue,
                    rowNum,
                    name,
                    "attachment",
                    MILESTONE_ATTACHMENTS);
                challenge->index = BG_GetAttachmentIndex(attachmentName);
                challenge->flags = 0;
                if ( !BG_UnlockablesGetAttachmentPurchased(controllerIndex, attachmentName) )
                    challenge->flags |= 1u;
                ++s_numSortedAttachments;
            }
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        Com_PrintError(16, "Tried to add too many stats milestones to list - only %d are supported.\n", 24);
        return 0;
    }
}

bool __cdecl LiveStats_ValidateChecksum(unsigned __int8 *buffer, int len)
{
    return *(unsigned int *)buffer == LiveStats_ChecksumGamerStats(buffer + 4, len);
}

char __cdecl LiveStats_ValidateStatsBackup(int controllerIndex)
{
    persistentStats *StatsBuffer; // eax
    int v3; // [esp-4h] [ebp-8h]
    int statsBackupVersion; // [esp+0h] [ebp-4h] BYREF

    v3 = LiveStorage_GetStatsBufferSize() - 4;
    StatsBuffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_BACKUP, 1);
    if ( LiveStats_ValidateChecksum(StatsBuffer->statsBuffer, v3) )
    {
        statsBackupVersion = 0;
        LiveStats_GetIntPlayerBackupStatByKey(controllerIndex, &statsBackupVersion, MP_PLAYERSTATSKEY_STATS_VERSION);
        if ( statsBackupVersion == stat_version->current.integer )
        {
            LiveStorage_StatsBackupFetchCompleted(controllerIndex);
            return 1;
        }
        else
        {
            Com_Printf(
                16,
                "Stats backup stat version is %d expecting %d\n",
                statsBackupVersion,
                stat_version->current.integer);
            return 0;
        }
    }
    else
    {
        Com_Printf(16, "Stats backup checksum is incorrect\n");
        return 0;
    }
}

void __cdecl LiveStats_ValidateStats(int controllerIndex)
{
    int backupStatsValue; // [esp+0h] [ebp-68h] BYREF
    int currentStatsValue; // [esp+4h] [ebp-64h] BYREF
    playerStatsKeyIndex_t statsKeys[21]; // [esp+8h] [ebp-60h]
    int statsInitialVersion; // [esp+5Ch] [ebp-Ch] BYREF
    int currentStatsKey; // [esp+60h] [ebp-8h]
    int statsBackupVersion; // [esp+64h] [ebp-4h] BYREF

    if (!LiveStorage_GetStatsChecksumValid(controllerIndex, STATS_LOCATION_NORMAL))
    {
        //BLOPS_NULLSUB();
    }
    statsKeys[0] = MP_PLAYERSTATSKEY_SCORE;
    statsKeys[1] = MP_PLAYERSTATSKEY_PLEVEL;
    statsKeys[2] = MP_PLAYERSTATSKEY_TIMEPLAYEDTOTAL;
    statsKeys[3] = MP_PLAYERSTATSKEY_COUNT;
    statsKeys[4] = MP_PLAYERSTATSKEY_KILLS;
    statsKeys[5] = MP_PLAYERSTATSKEY_DEATHS;
    statsKeys[6] = MP_PLAYERSTATSKEY_DEATHSDURINGUSE;
    statsKeys[7] = MP_PLAYERSTATSKEY_HEADSHOTS;
    statsKeys[8] = MP_PLAYERSTATSKEY_SHOTS;
    statsKeys[9] = MP_PLAYERSTATSKEY_HITS;
    statsKeys[10] = MP_PLAYERSTATSKEY_TIMEUSED;
    statsKeys[11] = MP_PLAYERSTATSKEY_USED;
    statsKeys[12] = MP_PLAYERSTATSKEY_DESTROYED;
    statsKeys[13] = MP_PLAYERSTATSKEY_WINS;
    statsKeys[14] = MP_PLAYERSTATSKEY_LOSSES;
    statsKeys[15] = MP_PLAYERSTATSKEY_TIES;
    statsKeys[16] = MP_PLAYERSTATSKEY_LIFETIME_EARNINGS;
    statsKeys[17] = MP_PLAYERSTATSKEY_LIFETIME_BUYIN;
    statsKeys[18] = MP_PLAYERSTATSKEY_CONTRACTS_PURCHASED;
    statsKeys[19] = MP_PLAYERSTATSKEY_CONTRACTS_COMPLETED;
    for ( currentStatsKey = 0; statsKeys[currentStatsKey] != MP_PLAYERSTATSKEY_COUNT; ++currentStatsKey )
    {
        backupStatsValue = 0;
        currentStatsValue = 0;
        if ( LiveStats_GetIntPlayerStatByKey(
                     controllerIndex,
                     &backupStatsValue,
                     STATS_LOCATION_BACKUP,
                     statsKeys[currentStatsKey])
            && LiveStats_GetIntPlayerStatByKey(
                     controllerIndex,
                     &currentStatsValue,
                     STATS_LOCATION_FORCE_NORMAL,
                     statsKeys[currentStatsKey]) )
        {
            if (currentStatsValue < backupStatsValue)
            {
                //BLOPS_NULLSUB();
            }
        }
        else
        {
            Com_Printf(16, "Cannot read stat %s\n", playerStatKeys[statsKeys[currentStatsKey]]);
        }
    }
    Com_Printf(16, "Backup stats are below or equal to current stats\n");
    statsBackupVersion = 0;
    statsInitialVersion = 0;
    LiveStats_GetIntPlayerStatByKey(
        controllerIndex,
        &statsBackupVersion,
        STATS_LOCATION_BACKUP,
        MP_PLAYERSTATSKEY_STATS_VERSION);
    LiveStats_GetIntPlayerStatByKey(
        controllerIndex,
        &statsInitialVersion,
        STATS_LOCATION_FORCE_NORMAL,
        MP_PLAYERSTATSKEY_STATS_VERSION);
    if (statsBackupVersion != statsInitialVersion)
    {
        //BLOPS_NULLSUB();
    }
}

char __cdecl LiveStats_GetIntPlayerStatByKey(
                int controllerIndex,
                int *outInt,
                statsLocation location,
                playerStatsKeyIndex_t statKey)
{
    persistentStats *statsBuffer; // [esp+0h] [ebp-4h]

    statsBuffer = LiveStorage_GetStatsBuffer(controllerIndex, location, 1);
    return LiveStats_GetIntPlayerStatFromBase(outInt, playerStatKeys[statKey], (char *)statsBuffer);
}

void __cdecl LiveStats_ClearCustomClassWarning(int controllerIndex)
{
    persistentStats *StatsBuffer; // eax
    ddlState_t searchState; // [esp+0h] [ebp-10h] BYREF

    searchState = g_statsCacState;
    if ( DDL_MoveTo(&g_statsCacState, &searchState, 1, "resetWarningDislayed") )
    {
        StatsBuffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
        DDL_SetValue(&searchState, "1", (char *)StatsBuffer);
    }
    else if ( !Assert_MyHandler(
                             "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                             4287,
                             0,
                             "Could not find 'resetWarningDislayed' in stats DDL") )
    {
        __debugbreak();
    }
}

int __cdecl LiveStats_GetClanTagFeature(int controllerIndex)
{
    persistentStats *StatsBuffer; // eax
    ddlState_t searchState; // [esp+0h] [ebp-14h] BYREF
    int retVal; // [esp+10h] [ebp-4h]

    retVal = 0;
    if ( DDL_MoveTo(&g_statsRootState, &searchState, 2, "clanTagStats", "selectedFeature") )
    {
        StatsBuffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
        return DDL_GetInt(&searchState, (char *)StatsBuffer);
    }
    else if ( !Assert_MyHandler(
                             "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                             4302,
                             0,
                             "Could not find 'clanTagStats.selectedFeature' in stats DDL") )
    {
        __debugbreak();
    }
    return retVal;
}

ddlState_t *__cdecl LiveStats_GetCacDDLState()
{
    return &g_statsCacState;
}

ddlState_t *__cdecl LiveStats_GetRootDDLState()
{
    return &g_statsRootState;
}

ddlDef_t *__cdecl LiveStats_GetStatsDDL()
{
    return g_statsDDL;
}

unsigned int __cdecl LiveStats_GetXp(int controllerIndex)
{
    persistentStats *buffer; // [esp+0h] [ebp-8h]

    buffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_GLOBAL, 1);
    return DDL_GetInt(&g_statsRankXPState, (char *)buffer);
}

int __cdecl LiveStats_GetRank(int controllerIndex)
{
    unsigned int Xp; // eax

    Xp = LiveStats_GetXp(controllerIndex);
    return CL_GetRankForXp(Xp);
}

int __cdecl LiveStats_GetPrestige(int controllerIndex)
{
    int prestige; // [esp+0h] [ebp-4h] BYREF

    prestige = 0;
    LiveStats_GetIntPlayerStatByKey(controllerIndex, &prestige, MP_PLAYERSTATSKEY_PLEVEL);
    return prestige;
}

int __cdecl LiveStats_GetCurrency(int controllerIndex)
{
    int currentCodPoints; // [esp+0h] [ebp-4h] BYREF

    currentCodPoints = 0;
    if ( !LiveStats_GetIntPlayerStatByKey(controllerIndex, &currentCodPoints, MP_PLAYERSTATSKEY_CODPOINTS) )
        Com_PrintError(15, "Could not retrieve currency for controller %d\n", controllerIndex);
    return currentCodPoints;
}

char __cdecl LiveStats_SpendCurrency(
                int controllerIndex,
                int currencyAmount,
                pointsSpent_t reasonType,
                int reasonIndex)
{
#ifdef KISAK_LIVE
    int currentCodPoints; // [esp+0h] [ebp-4h]

    currentCodPoints = LiveStats_GetCurrency(controllerIndex);
    if ( currentCodPoints >= currencyAmount )
    {
        LiveStats_SetStatByKey(controllerIndex, MP_PLAYERSTATSKEY_CODPOINTS, currentCodPoints - currencyAmount);
        LiveStats_TrackSpending(controllerIndex, currencyAmount);
        if ( currencyAmount < 0 )
            currencyAmount = -currencyAmount;
        LiveCounter_IncrementCounterValueByName("global_moneyspent", currencyAmount);
        if ( recordPointsSpent )
        {
            if ( recordPointsSpent->current.enabled )
                MatchRecordPointsSpent(controllerIndex, currencyAmount, reasonType, reasonIndex);
        }
        return 1;
    }
    else
    {
        Com_PrintError(15, "Item costs %d and you only have %d\n", currencyAmount, currentCodPoints);
        return 0;
    }
#else
    return 0;
#endif
}

void __cdecl LiveStats_TrackSpending(int controllerIndex, int currencyAmount)
{
    int currentCurrencySpent; // [esp+0h] [ebp-4h] BYREF

    currentCurrencySpent = 0;
    if ( LiveStats_GetIntPlayerStatByKey(controllerIndex, &currentCurrencySpent, MP_PLAYERSTATSKEY_CURRENCYSPENT) )
        LiveStats_SetStatByKey(controllerIndex, MP_PLAYERSTATSKEY_CURRENCYSPENT, currencyAmount + currentCurrencySpent);
    else
        Com_PrintError(15, "Could not retrieve currency spent for controller %d\n", controllerIndex);
}

char __cdecl LiveStats_WriteXUIDToStats(int controllerIndex)
{
    unsigned __int64 v1; // rax
    persistentStats *buff; // [esp+0h] [ebp-1Ch]
    ddlState_t searchState; // [esp+4h] [ebp-18h] BYREF
    unsigned __int64 myXuid; // [esp+14h] [ebp-8h]

    LODWORD(v1) = Live_GetXuid(controllerIndex);
    myXuid = v1;
    if ( !v1 && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp", 4478, 0, "%s", "myXuid") )
        __debugbreak();
    if ( myXuid )
    {
        if ( DDL_MoveTo(&g_statsRootState, &searchState, 1, "PlayerXUID") )
        {
            buff = LiveStorage_GetStatsBuffer(0, STATS_LOCATION_NORMAL, 1);
            DDL_SetInt64(&searchState, myXuid, (char *)buff);
            return 1;
        }
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                        4492,
                        0,
                        "Could not find 'PlayerXUID' in stats DDL") )
            __debugbreak();
    }
    return 0;
}

unsigned __int64 __cdecl LiveStats_ReadXUIDFromStats()
{
    __int64 v0; // rax
    persistentStats *buff; // [esp+0h] [ebp-1Ch]
    ddlState_t searchState; // [esp+4h] [ebp-18h] BYREF
    unsigned __int64 statsXuid; // [esp+14h] [ebp-8h]

    statsXuid = 0;
    if ( DDL_MoveTo(&g_statsRootState, &searchState, 1, "PlayerXUID") )
    {
        buff = LiveStorage_GetStatsBuffer(0, STATS_LOCATION_NORMAL, 1);
        LODWORD(v0) = DDL_GetInt64(&searchState, (char *)buff);
        return v0;
    }
    else if ( !Assert_MyHandler(
                             "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                             4511,
                             0,
                             "Could not find 'PlayerXUID' in stats DDL") )
    {
        __debugbreak();
    }
    return statsXuid;
}

bool __cdecl LiveStats_WritePrestigeToStats(int controllerIndex)
{
    persistentStats *StatsBuffer; // eax
    int prestige; // [esp+0h] [ebp-28h]
    const char *path[2]; // [esp+4h] [ebp-24h] BYREF
    int maxPrestige; // [esp+Ch] [ebp-1Ch]
    ddlState_t searchState; // [esp+10h] [ebp-18h] BYREF
    int maxXP; // [esp+20h] [ebp-8h]
    int xp; // [esp+24h] [ebp-4h]

    xp = LiveStats_GetXp(controllerIndex);
    prestige = LiveStats_GetPrestige(controllerIndex);
    maxXP = CL_GetMaxXP();
    maxPrestige = CL_GetMaxPrestige();
    if ( xp < maxXP || prestige >= maxPrestige )
        return 0;
    path[0] = "PlayerStatsList";
    path[1] = playerStatKeys[1];
    if ( DDL_MoveToPath(&g_statsRootState, &searchState, 2, path) )
    {
        StatsBuffer = LiveStorage_GetStatsBuffer(0, STATS_LOCATION_NORMAL, 1);
        return DDL_SetInt(&searchState, prestige + 1, (char *)StatsBuffer);
    }
    else
    {
        DDL_PrintError("DDL:Error setting player stat value for %s.\n", playerStatKeys[1]);
        return 0;
    }
}

void __cdecl LiveStats_ResetBasicTrainingStats(int controllerIndex)
{
    persistentStats *StatsBuffer; // eax
    persistentStats *v2; // eax
    persistentStats *v3; // eax
    persistentStats *v4; // eax
    unsigned int StatsBufferSize; // [esp-4h] [ebp-1Ch]
    int v6; // [esp-4h] [ebp-1Ch]
    int savedUiItemsNoCost; // [esp+0h] [ebp-18h]
    int savedUiUseCustomClassInfo; // [esp+4h] [ebp-14h]
    bool wasBasicTraining; // [esp+Bh] [ebp-Dh]
    int savedCustomClassMode; // [esp+10h] [ebp-8h]
    int savedCustomKillstreakMode; // [esp+14h] [ebp-4h]

    if ( Live_IsSignedIn(controllerIndex) )
    {
        if ( !CG_IsShowingZombieMap() )
        {
            wasBasicTraining = xblive_basictraining->current.enabled;
            Dvar_SetBool((dvar_s *)xblive_basictraining, 1);
            savedUiItemsNoCost = ui_items_no_cost->current.integer;
            savedCustomClassMode = custom_class_mode->current.integer;
            savedCustomKillstreakMode = custom_killstreak_mode->current.integer;
            savedUiUseCustomClassInfo = ui_useCustomClassInfo->current.integer;
            Dvar_Reset((dvar_s *)ui_items_no_cost, DVAR_SOURCE_INTERNAL);
            Dvar_Reset((dvar_s *)custom_class_mode, DVAR_SOURCE_INTERNAL);
            Dvar_Reset((dvar_s *)custom_killstreak_mode, DVAR_SOURCE_INTERNAL);
            Dvar_Reset((dvar_s *)ui_useCustomClassInfo, DVAR_SOURCE_INTERNAL);
            StatsBufferSize = LiveStorage_GetStatsBufferSize();
            StatsBuffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
            memset(StatsBuffer->statsBuffer, 0, StatsBufferSize);
            LiveStorage_ValidateWithDDL(controllerIndex, STATS_LOCATION_BASICTRAINING);
            Cmd_ExecuteSingleCommand(-1, controllerIndex, (char*)"exec mp/stats_init.cfg");
            v6 = LiveStorage_GetStatsBufferSize();
            v2 = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
            LiveStats_WriteChecksumToBuffer(v2->statsBuffer, v6);
            LiveStats_SetStatByKey(controllerIndex, MP_PLAYERSTATSKEY_STATS_VERSION, stat_version->current.integer);
            v3 = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
            LiveStats_SetBasicTrainingState(v3->statsBuffer, 1);
            v4 = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
            LiveStats_SetOnlineRankedState(v4->statsBuffer, 0);
            LiveStorage_SetStatsWriteNeeded(controllerIndex, 1, STATS_LOCATION_BASICTRAINING);
            LiveStorage_SetStatsChecksumValid(controllerIndex, STATS_LOCATION_BASICTRAINING, 1);
            LiveStats_WriteXUIDToStats(controllerIndex);
            Dvar_SetInt((dvar_s *)ui_items_no_cost, savedUiItemsNoCost);
            Dvar_SetInt((dvar_s *)custom_class_mode, savedCustomClassMode);
            Dvar_SetInt((dvar_s *)custom_killstreak_mode, savedCustomKillstreakMode);
            Dvar_SetInt((dvar_s *)ui_useCustomClassInfo, savedUiUseCustomClassInfo);
            Dvar_SetBool((dvar_s *)xblive_basictraining, wasBasicTraining);
            LiveStorage_WriteBasicTrainingStats(controllerIndex);
        }
    }
}

void __cdecl LiveStats_SetBufferInitialised(int controllerIndex, bool isInitialised)
{
    s_statsBufferInitialised[controllerIndex] = isInitialised;
}

void __cdecl LiveStats_InitStatsBuffer(int controllerIndex)
{
    persistentStats *StatsBuffer; // eax
    unsigned int StatsBufferSize; // [esp-4h] [ebp-18h]
    int savedUiItemsNoCost; // [esp+0h] [ebp-14h]
    int savedUiUseCustomClassInfo; // [esp+4h] [ebp-10h]
    int savedCustomClassMode; // [esp+Ch] [ebp-8h]
    int savedCustomKillstreakMode; // [esp+10h] [ebp-4h]

    savedUiItemsNoCost = ui_items_no_cost->current.integer;
    savedCustomClassMode = custom_class_mode->current.integer;
    savedCustomKillstreakMode = custom_killstreak_mode->current.integer;
    savedUiUseCustomClassInfo = ui_useCustomClassInfo->current.integer;
    Dvar_Reset((dvar_s *)ui_items_no_cost, DVAR_SOURCE_INTERNAL);
    Dvar_Reset((dvar_s *)custom_class_mode, DVAR_SOURCE_INTERNAL);
    Dvar_Reset((dvar_s *)custom_killstreak_mode, DVAR_SOURCE_INTERNAL);
    Dvar_Reset((dvar_s *)ui_useCustomClassInfo, DVAR_SOURCE_INTERNAL);
    LiveStats_SetBufferInitialised(controllerIndex, 1);
    StatsBufferSize = LiveStorage_GetStatsBufferSize();
    StatsBuffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
    memset(StatsBuffer->statsBuffer, 0, StatsBufferSize);
    LiveStorage_ValidateWithDDL(controllerIndex, STATS_LOCATION_NORMAL);
    Cmd_ExecuteSingleCommand(-1, controllerIndex, (char*)"exec mp/stats_init.cfg");
    Cmd_ExecuteSingleCommand(-1, controllerIndex, (char*)"updateprofilefromstats");
    Dvar_SetInt((dvar_s *)ui_items_no_cost, savedUiItemsNoCost);
    Dvar_SetInt((dvar_s *)custom_class_mode, savedCustomClassMode);
    Dvar_SetInt((dvar_s *)custom_killstreak_mode, savedCustomKillstreakMode);
    Dvar_SetInt((dvar_s *)ui_useCustomClassInfo, savedUiUseCustomClassInfo);
}

void __cdecl LiveStats_ResetStats(int controllerIndex, bool versionChanged)
{
    int LocalClientNum; // eax
    int v3; // eax
    persistentStats *StatsBuffer; // eax
    persistentStats *v5; // eax
    persistentStats *v6; // eax
    persistentStats *v7; // eax
    int StatsBufferSize; // [esp-4h] [ebp-8h]
    bool wasBasicTraining; // [esp+3h] [ebp-1h]

    wasBasicTraining = xblive_basictraining->current.enabled;
    Dvar_SetBool((dvar_s *)xblive_basictraining, 0);
    LiveStats_InitStatsBuffer(controllerIndex);
    LocalClientNum = Com_ControllerIndex_GetLocalClientNum(controllerIndex);
    Cmd_ExecuteSingleCommand(LocalClientNum, controllerIndex, (char *)"updategamerprofile");
    v3 = Com_ControllerIndex_GetLocalClientNum(controllerIndex);
    Cmd_ExecuteSingleCommand(v3, controllerIndex, (char *)"updatedvarsfromprofile");
    StatsBufferSize = LiveStorage_GetStatsBufferSize();
    StatsBuffer = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
    LiveStats_WriteChecksumToBuffer(StatsBuffer->statsBuffer, StatsBufferSize);
    v5 = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
    LiveStats_WriteVersionToBuffer((char *)v5);
    v6 = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
    LiveStats_SetBasicTrainingState(v6->statsBuffer, 0);
    v7 = LiveStorage_GetStatsBuffer(controllerIndex, STATS_LOCATION_NORMAL, 1);
    LiveStats_SetOnlineRankedState(v7->statsBuffer, 1);
    LiveStorage_SetStatsWriteNeeded(controllerIndex, 1, STATS_LOCATION_NORMAL);
    LiveStorage_SetStatsChecksumValid(controllerIndex, STATS_LOCATION_NORMAL, 1);
    LiveStats_WriteXUIDToStats(controllerIndex);
    if ( !versionChanged )
        LiveStats_ClearCustomClassWarning(controllerIndex);
    CL_CACValidateRequest_f();
    Dvar_SetBool((dvar_s *)xblive_basictraining, wasBasicTraining);
}

char __cdecl LiveStats_GetActiveContractStatState(
                unsigned int activeContractIndex,
                const char *statName,
                ddlState_t *searchState)
{
    char indexString[4]; // [esp+0h] [ebp-4h] BYREF

    indexString[0] = activeContractIndex + 48;
    indexString[1] = 0;
    if ( activeContractIndex > 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    4790,
                    0,
                    "%s",
                    "( activeContractIndex >= 0 ) && ( activeContractIndex < MAX_ACTIVE_CONTRACTS )") )
    {
        __debugbreak();
    }
    if ( DDL_MoveTo(&g_statsActiveContractsState, searchState, 2, indexString, statName) )
        return 1;
    DDL_PrintError("DDL: Error could not find stat named %s in active contract %d\n", statName, activeContractIndex);
    return 0;
}

char __cdecl LiveStats_GetContractStatState(unsigned int contractIndex, const char *statName, ddlState_t *searchState)
{
    char *indexString; // [esp+0h] [ebp-4h]

    indexString = va("%d", contractIndex);
    if ( contractIndex >= 0x200
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    4808,
                    0,
                    "%s",
                    "( contractIndex >= 0 ) && ( contractIndex < MAX_CONTRACTS )") )
    {
        __debugbreak();
    }
    if ( DDL_MoveTo(&g_statsContractsState, searchState, 2, indexString, statName) )
        return 1;
    DDL_PrintError("DDL: Error could not find stat named %s in contract %d\n", statName, contractIndex);
    return 0;
}

unsigned int __cdecl LiveStats_GetActiveContractStat(
                char *buffer,
                unsigned int activeContractIndex,
                const char *statName)
{
    ddlState_t searchState; // [esp+0h] [ebp-10h] BYREF

    if ( LiveStats_GetActiveContractStatState(activeContractIndex, statName, &searchState) )
        return DDL_GetInt(&searchState, buffer);
    else
        return 0;
}

bool __cdecl LiveStats_SetActiveContractStat(
                char *buffer,
                unsigned int activeContractIndex,
                const char *statName,
                unsigned int newValue)
{
    ddlState_t searchState; // [esp+0h] [ebp-10h] BYREF

    return LiveStats_GetActiveContractStatState(activeContractIndex, statName, &searchState)
            && DDL_SetInt(&searchState, newValue, buffer);
}

unsigned int __cdecl LiveStats_GetContractStat(char *buffer, unsigned int contractIndex, const char *statName)
{
    ddlState_t searchState; // [esp+0h] [ebp-10h] BYREF

    if ( LiveStats_GetContractStatState(contractIndex, statName, &searchState) )
        return DDL_GetInt(&searchState, buffer);
    else
        return 0;
}

bool __cdecl LiveStats_SetContractStat(
                char *buffer,
                unsigned int contractIndex,
                const char *statName,
                unsigned int newValue)
{
    ddlState_t searchState; // [esp+0h] [ebp-10h] BYREF

    return LiveStats_GetContractStatState(contractIndex, statName, &searchState)
            && DDL_SetInt(&searchState, newValue, buffer);
}

void __cdecl LiveStats_SetOnlineRankedState(unsigned __int8 *buffer, bool value)
{
    DDL_SetUserFlag((char *)buffer, 1, value);
}

bool __cdecl LiveStats_GetOnlineRankedState(unsigned __int8 *buffer)
{
    return DDL_GetUserFlag((char *)buffer, 1);
}

void __cdecl LiveStats_SetBasicTrainingState(unsigned __int8 *buffer, bool value)
{
    DDL_SetUserFlag((char *)buffer, 2, value);
}

bool __cdecl LiveStats_GetBasicTrainingState(unsigned __int8 *buffer)
{
    return DDL_GetUserFlag((char *)buffer, 2);
}

void __cdecl LiveStats_GetNemesisXuid(int controllerIndex, unsigned __int64 *nemesisXuid)
{
#ifdef KISAK_LIVE_SERVICE
    const ddlState_t *RootDDLState; // eax
    int v3; // edx
    ddlState_t ddlState; // [esp+0h] [ebp-10h] BYREF

    RootDDLState = LiveStats_GetRootDDLState();
    DDL_MoveTo(RootDDLState, &ddlState, 2, "AfterActionReportStats", "nemesisXuid");
    *(unsigned int *)nemesisXuid = LiveStats_GetDInt64Stat(controllerIndex, &ddlState);
    *((unsigned int *)nemesisXuid + 1) = v3;
#endif
}

bool __cdecl LiveStats_PrestigeLeaderboardsEnabled(int controllerIndex)
{
    int ItemIndexFromName; // eax

    ItemIndexFromName = BG_UnlockablesGetItemIndexFromName("FEATURE_PRESTIGE_LB");
    return BG_UnlockablesIsItemPurchased(controllerIndex, ItemIndexFromName);
}

void __cdecl LiveStats_ResetGamemodeChallenges(char *liveStatsBuffer)
{
    ddlState_t searchStateGameModeMilestones; // [esp+0h] [ebp-6Ch] BYREF
    const char *v2; // [esp+10h] [ebp-5Ch]
    int i; // [esp+14h] [ebp-58h]
    int memberCount; // [esp+18h] [ebp-54h]
    const char *statName; // [esp+1Ch] [ebp-50h]
    ddlState_t searchStateGameModeBests; // [esp+20h] [ebp-4Ch] BYREF
    const char *gameModeName; // [esp+30h] [ebp-3Ch]
    ddlState_t searchStateGameModeCurrentStats; // [esp+34h] [ebp-38h] BYREF
    int currentGameMode; // [esp+44h] [ebp-28h]
    int numGameModes; // [esp+48h] [ebp-24h]
    ddlState_t searchStateGameModeStats; // [esp+4Ch] [ebp-20h] BYREF
    ddlState_t searchStateStats; // [esp+5Ch] [ebp-10h] BYREF

    if ( !liveStatsBuffer
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp", 5325, 0, "%s", "liveStatsBuffer") )
    {
        __debugbreak();
    }
    if ( liveStatsBuffer )
    {
        if ( DDL_MoveTo(&g_statsRootState, &searchStateStats, 1, "PlayerBestStatsByGameMode")
            && DDL_MoveTo(&g_statsRootState, &searchStateGameModeStats, 1, "PlayerChallengeByGameMode") )
        {
            numGameModes = searchStateStats.ddl->enumList[searchStateStats.member->enumIndex].memberCount;
            for ( currentGameMode = 0; currentGameMode < numGameModes; ++currentGameMode )
            {
                gameModeName = searchStateStats.ddl->enumList[searchStateStats.member->enumIndex].members[currentGameMode];
                if ( DDL_MoveTo(&searchStateStats, &searchStateGameModeBests, 2, gameModeName, "stats")
                    && DDL_IterateFirst(&searchStateGameModeBests, &searchStateGameModeBests) )
                {
                    do
                    {
                        statName = searchStateGameModeBests.ddl->enumList[searchStateGameModeBests.member->enumIndex].members[searchStateGameModeBests.arrayIndex];
                        if ( DDL_MoveTo(&searchStateGameModeStats, &searchStateGameModeCurrentStats, 2, gameModeName, statName) )
                            DDL_SetInt(&searchStateGameModeCurrentStats, 0, liveStatsBuffer);
                    }
                    while ( DDL_IterateNext(&searchStateGameModeBests, &searchStateGameModeBests) );
                }
            }
        }
        if ( DDL_MoveTo(&g_statsRootState, &searchStateStats, 1, "CurrentGameModeMilestone") )
        {
            memberCount = searchStateStats.ddl->enumList[searchStateStats.member->enumIndex].memberCount;
            for ( i = 0; i < memberCount; ++i )
            {
                v2 = searchStateStats.ddl->enumList[searchStateStats.member->enumIndex].members[i];
                if ( DDL_MoveTo(&searchStateStats, &searchStateGameModeMilestones, 2, v2, "milestones")
                    && DDL_IterateFirst(&searchStateGameModeMilestones, &searchStateGameModeMilestones) )
                {
                    do
                        DDL_SetInt(&searchStateGameModeMilestones, 0, liveStatsBuffer);
                    while ( DDL_IterateNext(&searchStateGameModeMilestones, &searchStateGameModeMilestones) );
                }
            }
        }
    }
}

void __cdecl LiveStats_PresetigeStatsResetCmd()
{
    char *v0; // eax
    ddlState_t searchStateGroupStats; // [esp+0h] [ebp-3Ch] BYREF
    unsigned int itemNumber; // [esp+10h] [ebp-2Ch]
    char *liveStatsBuffer; // [esp+14h] [ebp-28h]
    int controllerIndex; // [esp+18h] [ebp-24h]
    ddlState_t searchStateChallenges; // [esp+1Ch] [ebp-20h] BYREF
    ddlState_t searchStateStats; // [esp+2Ch] [ebp-10h] BYREF

    controllerIndex = 0;
    liveStatsBuffer = (char *)LiveStorage_GetStatsBuffer(0, STATS_LOCATION_NORMAL, 1);
    s_numSortedChallenges = 0;
    for ( itemNumber = 0; itemNumber < 0x100; ++itemNumber )
    {
        if ( BG_UnlockablesIsItemValidNotNull(itemNumber) )
        {
            v0 = va("%d", itemNumber);
            if ( DDL_MoveTo(&g_statsRootState, &searchStateStats, 3, "ItemStats", v0)
                && DDL_IterateFirst(&searchStateStats, &searchStateStats) )
            {
                do
                {
                    LiveStats_SetItemStat(&searchStateStats, "challengeTier", liveStatsBuffer, 0);
                    LiveStats_SetItemStat(&searchStateStats, "challengeValue", liveStatsBuffer, 0);
                }
                while ( DDL_IterateNext(&searchStateStats, &searchStateStats) );
            }
            BG_UnlockablesClearWeaponInfo(controllerIndex, itemNumber);
        }
    }
    if ( DDL_MoveTo(&g_statsRootState, &searchStateStats, 1, "GroupStats")
        && DDL_IterateFirst(&searchStateStats, &searchStateStats) )
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
    if ( DDL_MoveTo(&g_statsRootState, &searchStateChallenges, 1, "ChallengeTier")
        && DDL_MoveTo(&g_statsRootState, &searchStateStats, 1, "ChallengeValue")
        && DDL_IterateFirst(&searchStateChallenges, &searchStateChallenges)
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
    LiveStats_ResetGamemodeChallenges(liveStatsBuffer);
    LiveCounter_IncrementCounterValueByName("global_prestiged", 1u);
}

void __cdecl SV_UpdatePersonalBestsForClient(int clientnum)
{
#ifdef KISAK_LIVE
    char *v1; // eax
    ddlState_t searchStateGameModeBests; // [esp+4h] [ebp-94h] BYREF
    const char *gameModeName; // [esp+14h] [ebp-84h]
    ddlState_t searchStateGameModeCurrentStats; // [esp+18h] [ebp-80h] BYREF
    int currentGameMode; // [esp+28h] [ebp-70h]
    int numGameModes; // [esp+2Ch] [ebp-6Ch]
    unsigned int clanTagFeature; // [esp+30h] [ebp-68h]
    int proItemIndex; // [esp+34h] [ebp-64h]
    int oldProUnlocked; // [esp+38h] [ebp-60h]
    int newProUnlocked; // [esp+3Ch] [ebp-5Ch]
    unsigned int itemNumber; // [esp+40h] [ebp-58h]
    client_t *client; // [esp+44h] [ebp-54h]
    char *liveStatsBuffer; // [esp+48h] [ebp-50h]
    int newRankXP; // [esp+4Ch] [ebp-4Ch] BYREF
    ddlState_t searchStateGameModeStats; // [esp+50h] [ebp-48h] BYREF
    int oldPrestige; // [esp+60h] [ebp-38h] BYREF
    char *oldStatsBuffer; // [esp+64h] [ebp-34h]
    int oldRank; // [esp+68h] [ebp-30h]
    ddlState_t searchStateBests; // [esp+6Ch] [ebp-2Ch] BYREF
    int oldRankXP; // [esp+7Ch] [ebp-1Ch] BYREF
    int newRank; // [esp+80h] [ebp-18h]
    ddlState_t searchStateStats; // [esp+84h] [ebp-14h] BYREF
    int newPrestige; // [esp+94h] [ebp-4h] BYREF

    if ( (clientnum < 0 || clientnum > com_maxclients->current.integer)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp",
                    5476,
                    0,
                    "clientnum not in [0, com_maxclients->current.integer]\n\t%i not in [%i, %i]",
                    clientnum,
                    0,
                    com_maxclients->current.integer) )
    {
        __debugbreak();
    }
    client = &svs.clients[clientnum];
    if ( !client && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_stats.cpp", 5479, 0, "%s", "client") )
        __debugbreak();
    if ( client->statsValidated )
    {
        liveStatsBuffer = (char *)client->globalStats;
        oldStatsBuffer = (char *)client->globalStatsStable;
        oldRankXP = 0;
        newRankXP = 0;
        oldRank = 0;
        newRank = 0;
        oldPrestige = 0;
        newPrestige = 0;
        SV_GetStatFromBlob((char *)client->globalStats, "RANKXP", &newRankXP);
        newRank = CL_GetRankForXp(newRankXP);
        SV_GetStatFromBlob((char *)client->globalStats, "PLEVEL", &newPrestige);
        SV_GetStatFromBlob(oldStatsBuffer, "RANKXP", &oldRankXP);
        oldRank = CL_GetRankForXp(oldRankXP);
        SV_GetStatFromBlob(oldStatsBuffer, "PLEVEL", &oldPrestige);
        for ( itemNumber = 0; itemNumber < 0x100; ++itemNumber )
        {
            if ( BG_UnlockablesIsItemValidNotNull(itemNumber) && !BG_UnlockablesIsItemInDefaultClass(itemNumber) )
            {
                if ( newRank > oldRank
                    && !BG_UnlockablesIsItemLockedForRank(newPrestige, newRank, itemNumber)
                    && BG_UnlockablesIsItemLockedForRank(oldPrestige, oldRank, itemNumber) )
                {
                    SV_SetItemNewForClient(liveStatsBuffer, itemNumber, newRank, newPrestige);
                }
                v1 = va("%d", itemNumber);
                if ( DDL_MoveTo(&g_statsRootState, &searchStateStats, 3, "ItemStats", v1)
                    && DDL_IterateFirst(&searchStateStats, &searchStateStats) )
                {
                    do
                        LiveStats_UpdateItemPersonalBest(liveStatsBuffer, oldStatsBuffer, &searchStateStats);
                    while ( DDL_IterateNext(&searchStateStats, &searchStateStats) );
                }
                oldProUnlocked = (unsigned __int8)LiveStats_ProVersionUnlocked(itemNumber, oldStatsBuffer);
                newProUnlocked = (unsigned __int8)LiveStats_ProVersionUnlocked(itemNumber, liveStatsBuffer);
                if ( newProUnlocked > oldProUnlocked )
                {
                    proItemIndex = BG_UnlockablesGetProItem(itemNumber);
                    if ( proItemIndex != -1 )
                        SV_SetItemNewForClient(liveStatsBuffer, proItemIndex, newRank, newPrestige);
                }
            }
        }
        for ( clanTagFeature = 0; clanTagFeature < 0x100; ++clanTagFeature )
        {
            if ( BG_UnlockablesGetClanTagFeature(clanTagFeature, CLANTAG_COL_UNLOCKLVL)
                && newRank > oldRank
                && !BG_UnlockablesIsClanTagFeatureLockedForRank(newPrestige, newRank, clanTagFeature)
                && BG_UnlockablesIsClanTagFeatureLockedForRank(oldPrestige, oldRank, clanTagFeature) )
            {
                SV_SetClanTagFeatureNewForClient(liveStatsBuffer, newRank, newPrestige, clanTagFeature);
            }
        }
        s_currentPersonalBest[0] = 0;
        if ( DDL_MoveTo(&g_statsRootState, &searchStateStats, 1, "PlayerStatsList")
            && DDL_MoveTo(&g_statsRootState, &searchStateBests, 1, "PersonalBests")
            && DDL_IterateFirst(&searchStateStats, &searchStateStats)
            && DDL_IterateFirst(&searchStateBests, &searchStateBests) )
        {
            do
                LiveStats_UpdatePersonalBest(0, liveStatsBuffer, oldStatsBuffer, &searchStateStats, &searchStateBests);
            while ( DDL_IterateNext(&searchStateStats, &searchStateStats)
                     && DDL_IterateNext(&searchStateBests, &searchStateBests) );
        }
        LiveStats_UpdatetBestMatchAccuracy(liveStatsBuffer, oldStatsBuffer);
        LiveStats_UpdatetBestMatchKdRatio(liveStatsBuffer, oldStatsBuffer);
        if ( DDL_MoveTo(&g_statsRootState, &searchStateStats, 1, "PlayerBestStatsByGameMode")
            && DDL_MoveTo(&g_statsRootState, &searchStateGameModeStats, 1, "PlayerStatsByGameMode") )
        {
            numGameModes = searchStateStats.ddl->enumList[searchStateStats.member->enumIndex].memberCount;
            for ( currentGameMode = 0; currentGameMode < numGameModes; ++currentGameMode )
            {
                gameModeName = searchStateStats.ddl->enumList[searchStateStats.member->enumIndex].members[currentGameMode];
                if ( DDL_MoveTo(&searchStateStats, &searchStateGameModeBests, 2, gameModeName, "stats")
                    && DDL_IterateFirst(&searchStateGameModeBests, &searchStateGameModeBests) )
                {
                    do
                    {
                        if ( DDL_MoveTo(
                                     &searchStateGameModeStats,
                                     &searchStateGameModeCurrentStats,
                                     2,
                                     gameModeName,
                                     searchStateGameModeBests.ddl->enumList[searchStateGameModeBests.member->enumIndex].members[searchStateGameModeBests.arrayIndex]) )
                        {
                            LiveStats_UpdatePersonalBest(
                                0,
                                liveStatsBuffer,
                                oldStatsBuffer,
                                &searchStateGameModeCurrentStats,
                                &searchStateGameModeBests);
                        }
                    }
                    while ( DDL_IterateNext(&searchStateGameModeBests, &searchStateGameModeBests) );
                }
            }
        }
    }
#endif
}

cmd_function_s LiveStats_StatSetByNameCmd_VAR;
cmd_function_s LiveStats_StatAddByNameCmd_VAR;
cmd_function_s LiveStats_StatGetByNameInDvarCmd_VAR;
cmd_function_s LiveStats_StatReadDDLCmd_VAR;
cmd_function_s LiveStats_StatWriteDDLCmd_VAR;
cmd_function_s LiveStats_ActivateContractCmd_VAR;
cmd_function_s LiveStats_SetCurrentContractCmd_VAR;
cmd_function_s LiveContracts_SetFilterCmd_VAR;
cmd_function_s LiveStats_CompareStatsVsStableBufferCmd_VAR;
cmd_function_s LiveStats_BuildChallengeListCmd_VAR;
cmd_function_s LiveStats_BuildKillstreakChallengeListCmd_VAR;
cmd_function_s LiveStats_BuildGlobalChallengesListCmd_VAR;
cmd_function_s LiveStats_BuildAttachmentsListCmd_VAR;
cmd_function_s LiveStats_PresetigeStatsResetCmd_VAR;

bool s_initCalledOnce = false;
void __cdecl LiveStats_Init()
{
    g_statsDDL = DDL_LoadAsset("ddl_mp/stats.ddl");
    DDL_Reset(&g_statsCacState, g_statsDDL);
    DDL_MoveToName(&g_statsCacState, &g_statsCacState, "CacLoadouts");
    DDL_Reset(&g_statsActiveContractsState, g_statsDDL);
    DDL_MoveToName(&g_statsActiveContractsState, &g_statsActiveContractsState, "activeContracts");
    DDL_Reset(&g_statsContractsState, g_statsDDL);
    DDL_MoveToName(&g_statsContractsState, &g_statsContractsState, "contracts");
    DDL_Reset(&g_statsRootState, g_statsDDL);
    DDL_MoveTo(&g_statsRootState, &g_statsRankXPState, 2, "PlayerStatsList", "RANKXP");
    CL_BuildRankXPTable();
    BG_UnlockableItemsInit();
    UI_Gametype_BuildDefaultCustomClasses();
    if ( !s_initCalledOnce )
    {
        s_initCalledOnce = 1;
        Cmd_AddCommandInternal("statSetByName", LiveStats_StatSetByNameCmd, &LiveStats_StatSetByNameCmd_VAR);
        Cmd_AddCommandInternal("statAddByName", LiveStats_StatAddByNameCmd, &LiveStats_StatAddByNameCmd_VAR);
        Cmd_AddCommandInternal(
            "statGetByNameInDvar",
            LiveStats_StatGetByNameInDvarCmd,
            &LiveStats_StatGetByNameInDvarCmd_VAR);
        Cmd_AddCommandInternal("statReadDDL", LiveStats_StatReadDDLCmd, &LiveStats_StatReadDDLCmd_VAR);
        Cmd_AddCommandInternal("statWriteDDL", LiveStats_StatWriteDDLCmd, &LiveStats_StatWriteDDLCmd_VAR);
        Cmd_AddCommandInternal("activateContract", LiveStats_ActivateContractCmd, &LiveStats_ActivateContractCmd_VAR);
        Cmd_AddCommandInternal("setCurrentContract", LiveStats_SetCurrentContractCmd, &LiveStats_SetCurrentContractCmd_VAR);
        Cmd_AddCommandInternal("setContractFilter", LiveContracts_SetFilterCmd, &LiveContracts_SetFilterCmd_VAR);
        Cmd_AddCommandInternal(
            "CompareStatsVsStableBuffer",
            LiveStats_CompareStatsVsStableBufferCmd,
            &LiveStats_CompareStatsVsStableBufferCmd_VAR);
        Cmd_AddCommandInternal("BuildChallengeList", LiveStats_BuildChallengeListCmd, &LiveStats_BuildChallengeListCmd_VAR);
        Cmd_AddCommandInternal(
            "BuildKillstreakChallengeList",
            LiveStats_BuildKillstreakChallengeListCmd,
            &LiveStats_BuildKillstreakChallengeListCmd_VAR);
        Cmd_AddCommandInternal(
            "BuildGlobalChallengesList",
            LiveStats_BuildGlobalChallengesListCmd,
            &LiveStats_BuildGlobalChallengesListCmd_VAR);
        Cmd_AddCommandInternal(
            "BuildAttachmentsList",
            LiveStats_BuildAttachmentsListCmd,
            &LiveStats_BuildAttachmentsListCmd_VAR);
        Cmd_AddCommandInternal(
            "PrestigeStatsReset",
            LiveStats_PresetigeStatsResetCmd,
            &LiveStats_PresetigeStatsResetCmd_VAR);
        ui_challengeSort = _Dvar_RegisterInt("ui_challengeSort", 3, 0, 4, 0, "Sort order for challenges");
        debugStats = _Dvar_RegisterBool("debugStats", 1, 0, "Print messages showing when persistent stats are set");
        ui_numPersonalBests = _Dvar_RegisterInt(
                                                        "ui_numPersonalBests",
                                                        3,
                                                        0x80000000,
                                                        0x7FFFFFFF,
                                                        0,
                                                        "Number of personal bests to display in the AAR");
        recordPointsSpent = _Dvar_RegisterBool("recordPointsSpent", 1, 0, "Record spending through eventRecordBin");
        ui_challengeGameMode = _Dvar_RegisterString(
                                                         "ui_challengeGameMode",
                                                         (char *)"",
                                                         0,
                                                         "Game mode type for the challenge you are viewing");
        DDL_Cmd_Init();
    }
}

void __cdecl LiveStats_StatSetByNameCmd()
{
    const char *v0; // eax
    const char *v1; // eax
    unsigned int v2; // [esp-4h] [ebp-20h]

    if ( Cmd_Argc() >= 2 )
    {
        v0 = Cmd_Argv(2);
        v2 = atoi(v0);
        v1 = Cmd_Argv(1);
        LiveStats_SetIntPlayerStat(0, v1, v2);
    }
    else
    {
        Com_PrintError(15, "statsetbyname usage: statset <statname> <value>\n");
    }
}

void __cdecl LiveStats_StatAddByNameCmd()
{
    const char *v0; // eax
    const char *v1; // eax
    int v2; // eax
    const char *v3; // eax
    unsigned int v4; // [esp-4h] [ebp-2Ch]
    int statValue; // [esp+24h] [ebp-4h] BYREF

    if ( Cmd_Argc() >= 2 )
    {
        v0 = Cmd_Argv(1);
        if ( LiveStats_GetIntPlayerStat(0, &statValue, v0) )
        {
            v1 = Cmd_Argv(2);
            v2 = atoi(v1);
            v4 = statValue + v2;
            v3 = Cmd_Argv(1);
            LiveStats_SetIntPlayerStat(0, v3, v4);
        }
    }
    else
    {
        Com_PrintError(15, "stataddbyname usage: statset <statname> <value>\n");
    }
}

void __cdecl LiveStats_StatGetByNameInDvarCmd()
{
    const char *v0; // eax
    int v1; // [esp-4h] [ebp-28h]
    const char *statName; // [esp+1Ch] [ebp-8h]
    int statValue; // [esp+20h] [ebp-4h] BYREF

    if ( Cmd_Argc() >= 2 )
    {
        statName = Cmd_Argv(1);
        if ( LiveStats_GetIntPlayerStat(0, &statValue, statName) )
        {
            v1 = statValue;
            v0 = Cmd_Argv(2);
            Dvar_SetIntByName(v0, v1);
        }
    }
    else
    {
        Com_PrintError(15, "statgetbynameindvar usage: statgetindvar <statname> <dvar>\n");
    }
}

void __cdecl LiveStats_StatReadDDLCmd()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    int v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    int i; // [esp+38h] [ebp-60h]
    ddlState_t state; // [esp+3Ch] [ebp-5Ch] BYREF
    char *buffer; // [esp+4Ch] [ebp-4Ch]
    ddlResult_t result; // [esp+50h] [ebp-48h] BYREF

    if ( !g_statsDDL )
    {
        DDL_PrintError("DDL: Could not find the global stats ddl definition.\n");
        return;
    }
    DDL_Reset(&state, g_statsDDL);
    DDL_Printf("DDL: Attempting to move.. \n");
    if ( Cmd_Argc() < 2 )
    {
        DDL_Printf("DDL: statsDDLTest usage: statsDDLTest <path>\n");
        return;
    }
    for ( i = 1; i < Cmd_Argc(); ++i )
    {
        if ( state.member )
        {
            if ( state.member->arraySize > 1 && state.member->enumIndex == -1 && state.arrayIndex == -1 )
            {
                v3 = Cmd_Argv(i);
                v4 = atoi(v3);
                if ( !DDL_MoveToIndex(&state, &state, v4, 1) )
                {
                    v5 = Cmd_Argv(i);
                    DDL_PrintError("DDL: Array index '%s' not found.\n", v5);
                    return;
                }
LABEL_20:
                v7 = Cmd_Argv(i);
                DDL_Printf("[%s]", v7);
                continue;
            }
            v6 = Cmd_Argv(i);
            if ( !DDL_MoveToName(&state, &state, v6) )
            {
LABEL_10:
                v1 = Cmd_Argv(i);
                DDL_PrintError("DDL: Member '%s' not found.\n", v1);
                return;
            }
            if ( state.member->arraySize > 1 )
                goto LABEL_20;
            v8 = Cmd_Argv(i);
            DDL_Printf(".%s", v8);
        }
        else
        {
            v0 = Cmd_Argv(i);
            if ( !DDL_MoveToName(&state, &state, v0) )
                goto LABEL_10;
            v2 = Cmd_Argv(i);
            DDL_Printf("%s", v2);
        }
    }
    DDL_Printf("\nFound and reached member. Attempting to read..\n");
    if ( LiveStats_CanPerformStatOperation(0) )
    {
        buffer = (char *)LiveStorage_GetStatsBuffer(0, STATS_LOCATION_NORMAL, 1);
        if ( DDL_GetValue(&state, &result, buffer) )
        {
            switch ( state.member->type )
            {
                case 0:
                case 1:
                case 2:
                    DDL_NoCheckPrintf("DDL: Domain is integer. Value is %u.\n", result.intValue);
                    break;
                case 3:
                    DDL_NoCheckPrintf("DDL: Domain is integer 64. Value is %llu.\n", result.int64Value);
                    break;
                case 5:
                    DDL_NoCheckPrintf("DDL: Domain is string. Value is %s.\n", (const char *)&result);
                    break;
                default:
                    return;
            }
        }
        else
        {
            DDL_PrintError("DDL: Could not read value.\n");
        }
    }
    else
    {
        DDL_PrintError("DDL: Could not perform stat read operation.\n");
    }
}

void __cdecl LiveStats_StatWriteDDLCmd()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    int v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    int v9; // eax
    const char *v10; // eax
    int v11; // eax
    const char *v12; // eax
    const char *v13; // eax
    int v14; // eax
    const char *v15; // eax
    char *v16; // [esp-4h] [ebp-40h]
    int i; // [esp+24h] [ebp-18h]
    ddlState_t state; // [esp+28h] [ebp-14h] BYREF
    char *buffer; // [esp+38h] [ebp-4h]

    if ( !g_statsDDL )
    {
        DDL_PrintError("DDL: Could not find the global stats ddl definition.\n");
        return;
    }
    DDL_Reset(&state, g_statsDDL);
    DDL_Printf("DDL: Attempting to move.. \n");
    if ( Cmd_Argc() < 2 )
    {
        DDL_Printf("DDL: statsDDLTest usage: statsDDLTest <path>\n");
        return;
    }
    for ( i = 1; i < Cmd_Argc() - 1; ++i )
    {
        if ( state.member )
        {
            if ( state.member->arraySize > 1 && state.member->enumIndex == -1 && state.arrayIndex == -1 )
            {
                v3 = Cmd_Argv(i);
                v4 = atoi(v3);
                if ( !DDL_MoveToIndex(&state, &state, v4, 1) )
                {
                    v5 = Cmd_Argv(i);
                    DDL_PrintError("DDL: Array index '%s' not found.\n", v5);
                    return;
                }
LABEL_20:
                v7 = Cmd_Argv(i);
                DDL_Printf("[%s]", v7);
                continue;
            }
            v6 = Cmd_Argv(i);
            if ( !DDL_MoveToName(&state, &state, v6) )
            {
LABEL_10:
                v1 = Cmd_Argv(i);
                DDL_PrintError("DDL: Member '%s' not found.\n", v1);
                return;
            }
            if ( state.member->arraySize > 1 )
                goto LABEL_20;
            v8 = Cmd_Argv(i);
            DDL_Printf(".%s", v8);
        }
        else
        {
            v0 = Cmd_Argv(i);
            if ( !DDL_MoveToName(&state, &state, v0) )
                goto LABEL_10;
            v2 = Cmd_Argv(i);
            DDL_Printf("%s", v2);
        }
    }
    DDL_Printf("\nFound and reached member. Attempting to write..\n");
    if ( LiveStats_CanPerformStatOperation(0) )
    {
        buffer = (char *)LiveStorage_GetStatsBuffer(0, STATS_LOCATION_NORMAL, 1);
        v16 = buffer;
        v9 = Cmd_Argc();
        v10 = Cmd_Argv(v9 - 1);
        if ( DDL_SetValue(&state, v10, v16) )
        {
            v14 = Cmd_Argc();
            v15 = Cmd_Argv(v14 - 1);
            DDL_NoCheckPrintf("Value %s written to stat (but stats have not necessarily been uploaded).\n", v15);
        }
        else
        {
            v11 = Cmd_Argc();
            v12 = Cmd_Argv(v11 - 1);
            v13 = va("DDL: Could not write value %s.\n", v12);
            DDL_PrintError(v13);
        }
    }
    else
    {
        DDL_PrintError("DDL: Could not perform stat write operation.\n");
    }
}

void __cdecl LiveStats_ActivateContractCmd()
{
    const char *v0; // eax
    const char *v1; // eax
    int v2; // eax
    unsigned int v3; // [esp-4h] [ebp-20h]

    if ( Cmd_Argc() == 3 )
    {
        v0 = Cmd_Argv(2);
        v3 = atoi(v0);
        v1 = Cmd_Argv(1);
        v2 = atoi(v1);
        LiveContracts_ActivateContract(0, v2, v3);
    }
}

void __cdecl LiveStats_SetCurrentContractCmd()
{
    const char *v0; // eax
    uiInfo_s *uiInfo; // [esp+10h] [ebp-4h]

    if ( Cmd_Argc() == 2 )
    {
        uiInfo = UI_GetInfo(0);
        v0 = Cmd_Argv(1);
        uiInfo->contractIndex = atoi(v0);
    }
}

void __cdecl LiveStats_WriteChecksumToBuffer(unsigned __int8 *buffer, int len)
{
    *(unsigned int *)buffer = LiveStats_ChecksumGamerStats(buffer + 4, len - 4);
}

