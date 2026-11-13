#include "offline.h"

#include <demo/demo_common.h>
#include <demo/demo_playback.h>
#include <bgame/bg_unlockable_items.h>
#include <client/cl_rank.h>
#include <ddl/ddl_api.h>
#include <ddl/ddl_cmd.h>
#include <qcommon/threads.h>
#include <ui_mp/ui_gametype_custom_mp.h>
#include <qcommon/com_gamemodes.h>
#include <ui/ui_playlists.h>
#include <win32/win_gamerprofile.h>
#include <universal/q_shared.h>
#include <steam/steam_api.h>
#include <steam/steam_gameserver.h>
#include <shellapi.h>
#include <cstdio>
#include <cstring>

static const char *bot_difficulties[5] = { "easy", "normal", "hard", "fu", NULL };

static bool s_havePlaylists;
static bool s_firstTimeRunning = true;
static persistentStats s_statsBuffers[STATS_LOCATION_COUNT];
static ddlDef_t *s_statsDDL;
static ddlState_t s_statsRootState;
static ddlState_t s_statsCacState;

const dvar_t *ui_showFriendsCombatRecord;
const dvar_t *ui_combatCurrViewNum;
const dvar_t *ui_combatComparisonModeOn;
const dvar_t *ui_combatHistogramCurrGametype;
const dvar_t *fsShowStreamingGraph;
const dvar_t *fsPrivateSlotCol;
const dvar_t *fsOtherUserPrivateSlotCol;
const dvar_t *fsSelectedFileID;
const dvar_t *fsSelectedFileName;
const dvar_t *fsIsSelectedFileNameModified;
const dvar_t *fsSelectedFileDescription;
const dvar_t *fsIsSelectedFileDescriptionModified;
const dvar_t *fsSelectedFileTagIndex;
const dvar_t *fshSelectLastSlotRow;
const dvar_t *fshSelectFirstSlotRow;
SessionData_s g_serverSession;
const dvar_t *debugStats;
const dvar_t *ui_numPersonalBests;
const dvar_t *ui_challengeGameMode;
const dvar_t *livesteam_sv_vac;
sv_cacvalidate_state_t g_cacvalidateState;
const dvar_t *presell;
unsigned __int64 s_selectedPlayerXUID;
unsigned __int64 s_selectedMetPlayerXUID;
const dvar_t *xblive_clanmatch;
const dvar_t *xblive_theater;
const dvar_t *xblive_wagermatch;
const dvar_t *xblive_basictraining;
const dvar_t *xblive_basictraining_popup;
const dvar_t *bot_tips;
const dvar_t *xblive_matchEndingSoon;
const dvar_t *bot_friends;
const dvar_t *bot_enemies;
const dvar_t *steamid;
const dvar_t *scr_bot_difficulty;
const dvar_t *bot_difficulty;

int __cdecl Live_CountGuestsInUse()
{
    return {};
}

bool __cdecl Live_IsPublicOnlineMatch()
{
    if ( Demo_IsPlaying() )
        return Demo_IsPublicOnlineMatch();
    else
        return Com_GameMode_IsPublicOnlineGame();
}

bool __cdecl Live_IsWagerMatch()
{
    if ( Demo_IsPlaying() )
        return Demo_IsWagerMatch();
    else
        return Com_GameMode_IsGameMode(GAMEMODE_WAGER_MATCH);
}

bool __cdecl Live_IsCombatTrainingMatch()
{
    if ( Demo_IsPlaying() )
        return Demo_IsCombatTrainingMatch();
    else
        return Com_GameMode_IsGameMode(GAMEMODE_BASIC_TRAINING);
}

void __cdecl Live_Init()
{
    steamid = _Dvar_RegisterString("steamid", (char *)"", 0x40u, "Player's SteamID");
    xblive_clanmatch = _Dvar_RegisterBool("xblive_clanmatch", 0, 0, "Current game is a clan match");
    xblive_theater = _Dvar_RegisterBool("xblive_theater", 0, 0, "Current game is a theater mode");
    xblive_wagermatch = _Dvar_RegisterBool("xblive_wagermatch", 0, 4u, "Current game is a wager match");
    xblive_basictraining = _Dvar_RegisterBool("xblive_basictraining", 0, 4u, "Current game is basic training");
    xblive_basictraining_popup = _Dvar_RegisterBool(
        "xblive_basictraining_popup",
        0,
        1u,
        "The user has seen the basic training popup description");
    xblive_matchEndingSoon = _Dvar_RegisterBool("xblive_matchEndingSoon", 0, 0, "True if the match is ending soon");
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
    clanName = _Dvar_RegisterString("clanName", (char *)"", 0, "Your clan abbreviation");
    presell = _Dvar_RegisterBool("presell", 0, 0x40u, "Set to true if the player has preordered");
    debugStats = _Dvar_RegisterBool("debugStats", 1, 0, "Print messages showing when persistent stats are set");
    ui_numPersonalBests = _Dvar_RegisterInt(
        "ui_numPersonalBests",
        3,
        0x80000000,
        0x7FFFFFFF,
        0,
        "Number of personal bests to display in the AAR");
    ui_challengeGameMode = _Dvar_RegisterString(
        "ui_challengeGameMode",
        (char *)"",
        0,
        "Game mode type for the challenge you are viewing");
    ui_showFriendsCombatRecord = _Dvar_RegisterBool(
        "ui_friendsCombatRecordOpen",
        0,
        0,
        "True when friend's or clan member's combat record is open.");
    ui_combatCurrViewNum = _Dvar_RegisterInt(
        "ui_combatCurrViewNum",
        0,
        0,
        21,
        0,
        "Current View number in the combat record weapons tab.");
    ui_combatComparisonModeOn = _Dvar_RegisterBool(
        "ui_combatComparisonModeOn",
        0,
        0,
        "True when comparing stats with other player in combat record");
    ui_combatHistogramCurrGametype = _Dvar_RegisterInt(
        "ui_combatHistogramCurrGametype",
        0,
        0x80000000,
        0x7FFFFFFF,
        0,
        "Current scroll bar Position for combat record");
    fsShowStreamingGraph = _Dvar_RegisterBool(
        "fsShowStreamingGraph",
        0,
        0,
        "Shows a graph in-game with streaming history information.");
    fsPrivateSlotCol = _Dvar_RegisterInt("fsPrivateSlotCol", 0, 0, 2, 0, "Selected column in the 2d private slots feeder.");
    fsOtherUserPrivateSlotCol = _Dvar_RegisterInt(
        "fsOtherUserPrivateSlotCol",
        0,
        0,
        2,
        0,
        "Selected column in the 2d private slots feeder for the other user.");
    fsSelectedFileID = _Dvar_RegisterString(
        "fsSelectedFileID",
        (char *)"",
        0,
        "File ID currently selected. Use to lookup up rating in ownerdraw.");
    fsSelectedFileName = _Dvar_RegisterString(
        "fsSelectedFileName",
        (char *)"",
        0,
        "Name when a file is selected to be transferred to My File Share");
    fsIsSelectedFileNameModified = _Dvar_RegisterBool(
        "fsIsSelectedFileNameModified",
        0,
        0,
        "A flag which gives us information if a selected filename is modified.");
    fsSelectedFileDescription = _Dvar_RegisterString(
        "fsSelectedFileDescription",
        (char *)"",
        0,
        "Description when a file is selected to be transferred to My File Share");
    fsIsSelectedFileDescriptionModified = _Dvar_RegisterBool(
        "fsIsSelectedFileDescriptionModified",
        0,
        0,
        "A flag which gives us information if a selected file's description is modified.");
    fsSelectedFileTagIndex = _Dvar_RegisterInt(
        "fsSelectedFileTagIndex",
        0,
        0,
        0x7FFFFFFF,
        0,
        "The tag index of the selected file.");
    fshSelectLastSlotRow = _Dvar_RegisterInt(
        "fshSelectLastSlotRow",
        0,
        0,
        0x7FFFFFFF,
        0,
        "Indicates if the last row in the 2d slot feeder must be selected when focused.");
    fshSelectFirstSlotRow = _Dvar_RegisterInt(
        "fshSelectFirstSlotRow",
        0,
        0,
        0x7FFFFFFF,
        0,
        "Indicates if the first row in the 2d slot feeder must be selected when focused.");
    Session_Init();

    // game tables that were set up alongside the stats layout
    s_statsDDL = DDL_LoadAsset("ddl_mp/stats.ddl");
    DDL_Reset(&s_statsCacState, s_statsDDL);
    DDL_MoveToName(&s_statsCacState, &s_statsCacState, "CacLoadouts");
    DDL_Reset(&s_statsRootState, s_statsDDL);
    CL_BuildRankXPTable();
    BG_UnlockableItemsInit();
    UI_Gametype_BuildDefaultCustomClasses();
    DDL_Cmd_Init();
    Sys_InitDemoStreamingEvent();
}

char *__cdecl Clan_GetName(int controllerIndex)
{
    return GamerProfile_GetProfileSettings(controllerIndex)->clanPrefix;
}

double __cdecl LiveCombatRecord_GetSortedItemData(int index, bool forFriend, sortedItemParams_t param)
{
    return {};
}

int __cdecl LiveCombatRecord_GetSortedItemPositionByItemIndex(int itemIndex, bool forFriend)
{
    return {};
}

const char *__cdecl LiveCombatRecord_GetEquipmentUsageStringSuffix(const char *equipmentName)
{
    return (char *)"";
}

const char *__cdecl LiveCombatRecord_GetPersonalBestStatNameByIndex(int index)
{
    return (char *)"";
}

const char *__cdecl LiveCombatRecord_GetGameTypeName(unsigned int index)
{
    return (char *)"";
}

int __cdecl LiveCombatRecord_GetGameTypeLbIndex(unsigned int index)
{
    return {};
}

int __cdecl LiveCombatRecord_GetGameTypeCount()
{
    return {};
}

const char *__cdecl LiveCombatRecord_GetHitLocationImageRef(hitLocation_t hitLocation)
{
    return (char *)"";
}

persistentStats *__cdecl LiveStorage_GetStatsBuffer(
                int controllerIndex,
                statsLocation playerStatsLocation,
                bool verifyLocation)
{
    if ( (playerStatsLocation == STATS_LOCATION_NORMAL || playerStatsLocation == STATS_LOCATION_GLOBAL)
        && xblive_basictraining->current.enabled )
    {
        playerStatsLocation = STATS_LOCATION_BASICTRAINING;
    }
    if ( playerStatsLocation < STATS_LOCATION_NORMAL || playerStatsLocation >= STATS_LOCATION_COUNT )
        playerStatsLocation = STATS_LOCATION_NORMAL;
    return &s_statsBuffers[playerStatsLocation];
}

bool __cdecl LiveStorage_GetStatsWriteNeeded(int controllerIndex, statsLocation location)
{
    return {};
}

void __cdecl LiveStorage_SetStatsWriteNeeded(int controllerIndex, bool isWriteNeeded, statsLocation location)
{
}

bool __cdecl LiveStorage_DoWeHaveAllStats(int controllerIndex)
{
    return {};
}

fileSharePrivateData *__cdecl LiveStorage_FileShare_GetFileShareData(fileShareBufferLocation bufferLocation)
{
    static fileSharePrivateData s_fileShareData;

    return &s_fileShareData;
}

bool __cdecl LiveStorage_IsTimeSynced()
{
    return {};
}

void __cdecl LiveStorage_FileShare_GetDownloadProgress(int controllerIndex, unsigned int *bytes, float *dataRate)
{
}

void __cdecl LiveStorage_FileShare_GetUploadProgress(int controllerIndex, unsigned int *bytes, float *dataRate)
{
}

void __cdecl LiveStorage_FileShare_AbortOperation(int controllerIndex, fileShareLocation location)
{
}

unsigned int __cdecl LiveStorage_FileShare_GetDownloadingFileTotalSize(int controllerIndex)
{
    return {};
}

unsigned int __cdecl LiveStorage_FileShare_GetUploadingFileTotalSize(int controllerIndex)
{
    return {};
}

TaskRecord *__cdecl LiveStorage_FileShare_ReadFile(int controllerIndex, fileShareReadFileInfo *fileInfo)
{
    return {};
}

TaskRecord *__cdecl LiveStorage_FileShare_WriteFile(int controllerIndex, fileShareWriteFileInfo *writeFileInfo)
{
    return {};
}

TaskRecord *__cdecl LiveStorage_FileShare_WriteSummary(
                int controllerIndex,
                unsigned __int64 fileID,
                fileShareLocation location,
                void *summaryData,
                unsigned int summaryDataSize,
                const void *metaData,
                unsigned int metaDataSize,
                FileShareTag *tags,
                unsigned int numTags,
                bool showSuccess)
{
    return {};
}

int __cdecl LiveContracts_GetTimeUntilNextCycle()
{
    return {};
}

unsigned int __cdecl LiveContracts_GetContractsCountForList()
{
    return {};
}

int __cdecl LiveContracts_GetContractIndexFromListIndex(int controllerIndex, int listIndex)
{
    return {};
}

char *__cdecl LiveContracts_GetContractName(int index)
{
    return (char *)"";
}

char *__cdecl LiveContracts_GetContractDesc(int index)
{
    return (char *)"";
}

char *__cdecl LiveContracts_GetContractStatType(int index)
{
    return (char *)"";
}

char *__cdecl LiveContracts_GetContractStatName(int index)
{
    return (char *)"";
}

int __cdecl LiveContracts_GetContractCost(int index)
{
    return {};
}

char *__cdecl LiveContracts_GetRequirementType(int contractIndex, unsigned int requirementIndex)
{
    return (char *)"";
}

char *__cdecl LiveContracts_GetRequirementData(int contractIndex, unsigned int requirementIndex)
{
    return (char *)"";
}

char *__cdecl LiveContracts_GetResetConditions(int contractIndex)
{
    return (char *)"";
}

int __cdecl LiveContracts_GetContractRewardXP(int contractIndex)
{
    return {};
}

int __cdecl LiveContracts_GetContractRewardCP(int contractIndex)
{
    return {};
}

int __cdecl LiveContracts_GetContractRequiredCount(int index)
{
    return {};
}

char *__cdecl LiveContracts_GetExpirationType(int index)
{
    return (char *)"";
}

int __cdecl LiveContracts_GetExpirationData(int index)
{
    return {};
}

int __cdecl LiveContracts_GetTimesPurchasable(int contractIndex)
{
    return {};
}

int __cdecl LiveContracts_GetCooldownTime(int contractIndex)
{
    return {};
}

int __cdecl LiveContracts_GetUnlockLevel(int contractIndex)
{
    return {};
}

bool __cdecl LiveContracts_IsContractLocked(int controllerIndex, int contractIndex)
{
    return {};
}

unsigned int __cdecl LiveContracts_GetTimesPurchased(int controllerIndex, int contractIndex)
{
    return {};
}

int __cdecl LiveContracts_GetTimeUntilCooledDown(int controllerIndex, int contractIndex)
{
    return {};
}

int __cdecl LiveContracts_GetNumActiveContracts(int controllerIndex)
{
    return {};
}

int __cdecl LiveContracts_GetNumContractsInProgress(int controllerIndex)
{
    return {};
}

int __cdecl LiveContracts_GetActiveContractIndex(int controllerIndex, int contractIndex)
{
    return {};
}

unsigned int __cdecl LiveContracts_GetIndexForActiveContract(int controllerIndex, unsigned int activeContractIndex)
{
    return {};
}

int __cdecl LiveContracts_GetActiveContractProgress(int controllerIndex, unsigned int activeContractIndex)
{
    return {};
}

int __cdecl LiveContracts_GetCombatTimeLeft(int controllerIndex, unsigned int activeContractIndex)
{
    return {};
}

int __cdecl LiveContracts_GetActiveContractStatus(int controllerIndex, unsigned int activeContractIndex)
{
    return {};
}

unsigned int __cdecl LiveContracts_SV_GetIndexForActiveContract(
                unsigned int clientNum,
                unsigned int activeContractIndex)
{
    return {};
}

int __cdecl LiveContracts_SV_GetActiveContractProgress(unsigned int clientNum, unsigned int activeContractIndex)
{
    return {};
}

void __cdecl LiveContracts_SV_IncrementActiveContractProgress(
                unsigned int clientNum,
                unsigned int activeContractIndex,
                int increment)
{
}

void __cdecl LiveContracts_SV_ResetActiveContractProgress(unsigned int clientNum, unsigned int activeContractIndex)
{
}

void __cdecl LiveContracts_SV_IncrementActiveContractTime(
                unsigned int clientNum,
                unsigned int activeContractIndex,
                int increment)
{
}

int __cdecl LiveContracts_SV_GetActiveContractStatus(unsigned int clientNum, unsigned int activeContractIndex)
{
    return {};
}

int __cdecl LiveContracts_SV_GetCombatTimePassed(unsigned int clientNum, unsigned int activeContractIndex)
{
    return {};
}

void __cdecl LiveCounter_IncrementCounterValueByName(const char *counterType, unsigned __int64 increment)
{
}

void __cdecl LiveCounter_IncrementCounterValue(int counterID, __int64 increment)
{
}

int __cdecl LiveCounter_GetCounterTotalValue(int counterID)
{
    return {};
}

int __cdecl LiveCounter_CounterStringToID(const char *counterType)
{
    return {};
}

char __cdecl Live_FileShare_WritePublicMetaDataCreateTime(char *buffer, unsigned int createTime)
{
    return {};
}

char __cdecl Live_FileShare_WritePublicMetaDataLength(char *buffer, unsigned int duration)
{
    return {};
}

char __cdecl Live_FileShare_WritePublicMetaDataTags(char *buffer, FileShareTag *tags, int numTags)
{
    return {};
}

void __cdecl Live_FileShare_AddTag(
                unsigned __int64 priTag,
                unsigned __int64 secTag,
                int *numTags,
                FileShareTag *tags,
                int maxNumTags)
{
}

char *__cdecl Live_FileShare_GetLocalizedUserTagFromIndex(int index)
{
    return (char *)"";
}

int __cdecl Live_FileShare_GetIndexFromGameType(const char *gameType)
{
    return {};
}

char *__cdecl Live_FileShare_GetFilterList()
{
    return (char *)"";
}

char __cdecl Live_FileShare_IsRated(int controllerIndex, unsigned __int64 fileID, unsigned __int8 *rating)
{
    return {};
}

char __cdecl Live_FileShare_Theater_GetType(int *type)
{
    return {};
}

char __cdecl Live_FileShare_Theater_GetName(char **name)
{
    return {};
}

char __cdecl Live_FileShare_Theater_GetDescription(char **description)
{
    return {};
}

int __cdecl Live_FileShare_GetSearchResultsCount()
{
    return {};
}

char *__cdecl Live_FileShare_SearchResultsItemText(
                int controllerIndex,
                int startIndex,
                int selectedIndex,
                int itemIndex,
                bool *useOwnerDraw)
{
    return (char *)"";
}

int __cdecl Live_FileShare_GetMyPrivateSlotsCount()
{
    return {};
}

int __cdecl Live_FileShare_GetOtherPrivateSlotsCount()
{
    return {};
}

char *__cdecl Live_FileShare_PrivateSlotsText(
                int controllerIndex,
                int feederID,
                unsigned int itemIndex,
                int column,
                Material **handle)
{
    return (char *)"";
}

const char *__cdecl Live_FileShare_PrivateSlotsIngameText(
                int controllerIndex,
                int itemIndex,
                int column,
                Material **handle)
{
    return (char *)"";
}

void __cdecl Live_FileShare_FeederColor(
                listBoxDef_s *listPtr,
                itemDef_s *item,
                int contextIndex,
                unsigned int index,
                int column,
                fileShareBufferLocation bufferLocation,
                DvarValue *color)
{
}

double __cdecl Live_FileShare_LookupTotalVotes(unsigned __int64 fileID, VoteRankStatsInfo *ratings, int numRatings)
{
    return {};
}

const char *__cdecl Live_FileShare_GetPooledFileInfo(const char *field)
{
    return (char *)"";
}

char __cdecl Live_FileShare_GetItemInfo(
                int controllerIndex,
                fileShareKeyIndex keyIndex,
                int index,
                fileShareInfoLocation location,
                const char **stringResult,
                float *floatResult)
{
    return {};
}

char __cdecl Live_FileShare_GetMySlotInfo(
                int controllerIndex,
                int slotNum,
                const char *field,
                const char **stringResult,
                float *floatResult)
{
    return {};
}

char __cdecl Live_FileShare_GetFeederData(
                int controllerIndex,
                int feederID,
                int index,
                const char *field,
                const char **stringResult,
                float *floatResult)
{
    return {};
}

void __cdecl Live_FileShare_RatingOwnerdraw(
                int contextIndex,
                const rectDef_s *rect,
                const itemDef_s *item,
                const float *color)
{
}

void __cdecl Live_FileShare_PrivateRatingOwnerdraw(
                int contextIndex,
                const rectDef_s *rect,
                const itemDef_s *item,
                const float *color)
{
}

void __cdecl Live_FileShare_SubmitRatingOwnerdraw(
                int contextIndex,
                const rectDef_s *rect,
                const itemDef_s *item,
                const float *color)
{
}

void __cdecl Live_FileShare_MyRatingOwnerdraw(
                int localClientNum,
                int contextIndex,
                const rectDef_s *rect,
                const itemDef_s *item,
                const float *color)
{
}

void __cdecl Live_FileShare_AvgSubmitRatingOwnerdraw(
                int contextIndex,
                const rectDef_s *rect,
                const itemDef_s *item,
                const float *color)
{
}

void __cdecl Live_FileShare_ClearSearchState()
{
}

void __cdecl Live_FileShare_CacheShutdown()
{
}

void __cdecl Live_FileShareSearch_GetRatings(VoteRankStatsInfo **ratings, int *numRatings)
{
}

char __cdecl Live_FileShareSearch_FeederItemEnabled(int index)
{
    return {};
}

char __cdecl Live_FileShareSearch_FeederColor(
                listBoxDef_s *listPtr,
                itemDef_s *item,
                int contextIndex,
                int index,
                int column,
                DvarValue *color)
{
    return {};
}

int __cdecl Friends_GetCount(int __formal, int type)
{
    return {};
}

char __cdecl Friends_GetByIndex(int controllerIndex, int filter, int index, FriendInfo *info)
{
    return {};
}

char __cdecl Friends_GetByID(int controllerIndex, unsigned __int64 id, FriendInfo *info)
{
    return {};
}

unsigned int __cdecl LiveGroups_GetCount(char *path)
{
    return {};
}

unsigned int __cdecl LiveGroups_GetCountByType(servertype_t type)
{
    return {};
}

void __cdecl LB_ForceRefresh()
{
}

int __cdecl LB_FeederCount(int localClientNum)
{
    return {};
}

void __cdecl LB_FeederSelection(int index)
{
}

void __cdecl LB_OnSelect(int localClientNum)
{
}

void __cdecl LB_GetSelectedPlayerInfo(int localClientNum, LBSelectedPlayerInfo *info)
{
}

void __cdecl LB_OverrideCursorPos(int viewmax, int *cursorPos, int *startPos)
{
}

char __cdecl LB_UploadPlayerStats(int localControllerIndex)
{
    return {};
}

char *__cdecl LB_FeederItemText(int localClientNum, unsigned int index, int column, Material **material)
{
    return (char *)"";
}

int __cdecl LiveMeetPlayer_GetNoOfMetPlayers(int localControllerIndex)
{
    return {};
}

void __cdecl LiveMeetPlayer_SortMetPlayers(int localControllerIndex, unsigned int noOfPlayers)
{
}

unsigned int __cdecl LiveMeetPlayer_GetLastMetTime(int localControllerIndex, int index)
{
    return {};
}

XuidInfo *__cdecl LiveMeetPlayer_GetMetFriendInfo(XuidInfo *result, int localControllerIndex, int index)
{
    memset(result, 0, sizeof(*result));
    return result;
}

void __cdecl PCache_Lock()
{
}

void __cdecl PCache_Unlock()
{
}

PCacheComponent *__cdecl PCache_GetComponent(int controllerIndex, unsigned __int64 xuid, unsigned int type)
{
    return {};
}

bool __cdecl PCache_TouchComponent(PCacheComponent *component)
{
    return {};
}

void __cdecl PCache_InvalidatePlayerEmblem(int controllerIndex, unsigned __int64 xuid)
{
}

GfxImage *__cdecl PCache_GetPlayerEmblem(int controllerIndex, unsigned __int64 xuid)
{
    return {};
}

__int16 __cdecl PCache_GetPlayerEmblemBackgroundID(int controllerIndex, unsigned __int64 xuid)
{
    return {};
}

void __cdecl PCache_NukeProfile(int controlleridx)
{
}

char __cdecl PCache_GetRank(int controllerIndex, unsigned __int64 xuid, int *rank, int *prestige)
{
    return {};
}

char __cdecl PCache_GetCodpoints(int controllerIndex, unsigned __int64 xuid, int *codpoints)
{
    return {};
}

void __cdecl PCache_GetProfileEmblem(
                PCachePublicProfile *profile,
                CompositeEmblemLayer *layers,
                int layerCount,
                __int16 *backgroundID)
{
}

void __cdecl PCache_SetProfileEmblem(
                int controllerIndex,
                const CompositeEmblemLayer *layers,
                int layerCount,
                __int16 backgroundID)
{
}

int __cdecl Live_GetMapCount()
{
    return 9;
}

void __cdecl Live_SetPlaylistNum(int controllerIndex, int playlistNum)
{
}

int __cdecl Live_GetMapIndex()
{
    return -1;
}

int __cdecl Live_GetMapSource(int mapIndex)
{
    return mapIndex < 0 ? 2 : 0;
}

bool __cdecl Session_IsHost(SessionData_s *session, int clientNum)
{
    if ( IsDedicatedServer() )
        return false;
    if ( Demo_IsEnabled() )
        return clientNum == 1;
    return clientNum == 0;
}

int __cdecl Session_HostNum(SessionData_s *session)
{
    return {};
}

int __cdecl Session_FindRegisteredUser(const SessionData_s *session, unsigned __int64 xuid)
{
    int slot;

    for ( slot = 0; slot < 32; ++slot )
    {
        if ( session->registeredUsers[slot].active && session->registeredUsers[slot].xuid == xuid )
            return slot;
    }
    return -1;
}

void __cdecl Session_DeleteSession(SessionData_s *session)
{
}

void __cdecl Session_StartHost(SessionData_s *session, int sessionFlags, int numPrivateSlots, int numPublicSlots)
{
}

void __cdecl Session_Init()
{
    g_serverSession.sessionName = (char *)"gameSession";
    g_serverSession.registerUsersWithVoice = 1;
}

unsigned int __cdecl LiveStats_GetDDLIntStat(int controllerIndex, ddlState_t *searchState)
{
    return {};
}

unsigned int __cdecl LiveStats_GetDDLIntStat(int controllerIndex, const char **operands, int nOperands)
{
    return {};
}

unsigned int __cdecl LiveStats_GetDIntStat(int controllerIndex, ddlState_t *searchState)
{
    return {};
}

unsigned int __cdecl LiveStats_GetDIntStatFromBase(
                int controllerIndex,
                ddlState_t *searchState,
                persistentStats *buffer)
{
    return {};
}

char __cdecl LiveStats_GetIntPlayerStatFromBase(int *outInt, const char *statName, char *base)
{
    return {};
}

unsigned int __cdecl LiveStats_GetDIntStatFromForcedBase(int controllerIndex, ddlState_t *searchState, char *buffer)
{
    return {};
}

unsigned int __cdecl LiveStats_GetDInt64StatFromForcedBase(int controllerIndex, ddlState_t *searchState, char *buffer)
{
    return {};
}

char *__cdecl LiveStats_GetDStringStatFromForcedBase(int controllerIndex, ddlState_t *searchState, char *buffer)
{
    *buffer = 0;
    return buffer;
}

char __cdecl LiveStats_GetIntPlayerStat(int controllerIndex, int *outInt, const char *statName)
{
    return {};
}

char __cdecl LiveStats_GetIntPlayerStatByGameMode(
                int controllerIndex,
                int *outInt,
                const char *gameMode,
                const char *statName)
{
    return {};
}

char __cdecl LiveStats_GetIntOtherPlayerStatByGameMode(
                int controllerIndex,
                int *outInt,
                const char *gameMode,
                const char *statName)
{
    return {};
}

char __cdecl LiveStats_GetIntPlayerStatByKey(int controllerIndex, int *outInt, playerStatsKeyIndex_t keyIndex)
{
    return {};
}

char __cdecl LiveStats_GetIntOtherPlayerStatByKey(int controllerIndex, int *outInt, playerStatsKeyIndex_t keyIndex)
{
    return {};
}

const char *__cdecl LiveStats_GetPlayerStatStringByKey(playerStatsKeyIndex_t keyIndex)
{
    return (char *)"";
}

void __cdecl LiveStats_SetStatChanged(int controllerIndex, const char *hexMsg)
{
}

void __cdecl LiveStats_MakeStableStatsBuffer(int controllerIndex)
{
}

bool __cdecl LiveStats_IsStableStatsBufferInitialized(int controllerIndex)
{
    return {};
}

void __cdecl LiveStats_MakeStableGlobalStatsBuffer(int controllerIndex)
{
}

int __cdecl LiveStats_GetTotalMatchesPlayed(int controllerIndex)
{
    return {};
}

int __cdecl LiveStats_GetTotalMatchesPlayedByGameMode(int controllerIndex, const char *gameModePrefix)
{
    return {};
}

unsigned int __cdecl LiveStats_GetNumStatsMilestones(int controllerIndex)
{
    return {};
}

unsigned int __cdecl LiveStats_GetStatsMilestoneTier(int controllerIndex, unsigned int statsMilestoneNum)
{
    return {};
}

int __cdecl LiveStats_GetStatsMilestoneItemIndex(int controllerIndex, unsigned int statsMilestoneNum)
{
    return {};
}

const char *__cdecl LiveStats_GetStatsMilestoneName(int controllerIndex, unsigned int statsMilestoneNum)
{
    return (char *)"";
}

int __cdecl LiveStats_GetStatsMilestoneType(int controllerIndex, unsigned int statsMilestoneNum)
{
    return {};
}

unsigned int __cdecl LiveStats_GetChallengeCount(int type)
{
    return {};
}

bool __cdecl StringBeginsWith(const char *prefix, const char *name)
{
    unsigned int prefixLen;

    if ( !prefix || !name )
        return false;
    prefixLen = strlen(prefix);
    return prefixLen && prefixLen <= strlen(name) && !I_strnicmp(prefix, name, prefixLen);
}

bool __cdecl LiveStats_GetChallengeInfo(challengeInfo_t **challenge, unsigned int challengeNum, int type)
{
    return {};
}

bool __cdecl LiveStats_GetChallengeInfoParam(
                sortedChallengeParams_t param,
                unsigned int challengeNum,
                const char **stringValue,
                int *value,
                int type)
{
    return {};
}

unsigned int __cdecl LiveStats_GetNumPersonalBests(int controllerIndex)
{
    return {};
}

char *__cdecl LiveStats_GetPersonalBestName(int controllerIndex, unsigned int personalBestNum)
{
    return (char *)"";
}

char *__cdecl LiveStats_GetPersonalBestPrefix(int controllerIndex, unsigned int personalBestNum)
{
    return (char *)"";
}

int __cdecl LiveStats_GetPersonalBestValue(int controllerIndex, unsigned int personalBestNum)
{
    return {};
}

int __cdecl LiveStats_GetPersonalBestDelta(int controllerIndex, unsigned int personalBestNum)
{
    return {};
}

int __cdecl LiveStats_GetPersonalBestRating(int controllerIndex, unsigned int personalBestNum)
{
    return {};
}

unsigned int __cdecl LiveStats_GetNumRecentlyUnlockedItems(int controllerIndex)
{
    return {};
}

int __cdecl LiveStats_GetRecentlyUnlockedItemIndex(int controllerIndex, unsigned int itemNum)
{
    return {};
}

char __cdecl LiveStats_MoveToProItemVersion(ddlState_t *searchState, int itemIndex, int challengeIndex)
{
    return {};
}

char __cdecl LiveStats_GetIntPlayerStatByKey(
                int controllerIndex,
                int *outInt,
                statsLocation location,
                playerStatsKeyIndex_t statKey)
{
    return {};
}

int __cdecl LiveStats_GetClanTagFeature(int controllerIndex)
{
    return {};
}

ddlState_t *__cdecl LiveStats_GetCacDDLState()
{
    return &s_statsCacState;
}

ddlState_t *__cdecl LiveStats_GetRootDDLState()
{
    return &s_statsRootState;
}

ddlDef_t *__cdecl LiveStats_GetStatsDDL()
{
    return s_statsDDL;
}

unsigned int __cdecl LiveStats_GetXp(int controllerIndex)
{
    return {};
}

int __cdecl LiveStats_GetRank(int controllerIndex)
{
    return {};
}

int __cdecl LiveStats_GetPrestige(int controllerIndex)
{
    return {};
}

int __cdecl LiveStats_GetCurrency(int controllerIndex)
{
    return {};
}

char __cdecl LiveStats_SpendCurrency(
                int controllerIndex,
                int currencyAmount,
                pointsSpent_t reasonType,
                int reasonIndex)
{
    return {};
}

bool __cdecl LiveStats_WritePrestigeToStats(int controllerIndex)
{
    return {};
}

bool __cdecl LiveStats_GetBasicTrainingState(unsigned __int8 *buffer)
{
    return {};
}

void __cdecl LiveStats_GetNemesisXuid(int controllerIndex, unsigned __int64 *nemesisXuid)
{
}

void __cdecl SV_UpdatePersonalBestsForClient(int clientnum)
{
}

void __cdecl LiveSteam_CheckAccess()
{
}

const char *__cdecl LiveSteam_GetCountryCode()
{
    const char *loc;

    loc = Dvar_GetString("cl_forceloc");
    if ( loc && *loc )
        return loc;
    return SteamUtils()->GetIPCountry();
}

unsigned __int64 __cdecl LiveSteam_GetUid()
{
    if ( SteamUser() )
        return SteamUser()->GetSteamID().GetAccountID();
    return 0;
}

bool __cdecl LiveSteam_LaunchOtherApp(const char *cmd)
{
    const char *url;

    if ( !I_stricmp(cmd, "CoDSP_rd.exe") )
        url = "steam://run/42700";
    else if ( !I_stricmp(cmd, "CoDMP_rd.exe") )
        url = "steam://run/42710";
    else
        return false;
    return (int)ShellExecuteA(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL) > 32;
}

void __cdecl LiveSteam_Init()
{
    FILE *f;

    DeleteFileA("steam_appid.txt");
    f = fopen("noship_create_steam_appid.txt", "r");
    if ( f )
    {
        fclose(f);
        f = fopen("steam_appid.txt", "w");
        if ( f )
        {
            fprintf(f, "42710");
            fclose(f);
        }
    }
    livesteam_sv_vac = _Dvar_RegisterBool("sv_vac", 1, 1u, "Enable VAC on this server");
}

void __cdecl LiveSteam_Shutdown()
{
}

void __cdecl LiveSteam_Frame()
{
    SteamAPI_RunCallbacks();
}

int __cdecl LiveSteam_Client_ConnectToSteamServer(unsigned __int64 serverID, void *authBlob, unsigned int bufferSize)
{
    return {};
}

void LiveSteam_Client_SteamDisconnect()
{
}

void __cdecl LiveSteam_Server_Init()
{
}

void __cdecl LiveSteam_Server_Shutdown()
{
}

void __cdecl LiveSteam_Server_RunCallbacks()
{
    SteamGameServer_RunCallbacks();
}

void __cdecl LiveSteam_Server_ClientSteamAuthentication(
                unsigned __int64 clientSteamID,
                netadr_t netAddr,
                void *authBlob,
                unsigned int authBlobSize)
{
}

void __cdecl LiveSteam_Server_ClientSteamDisconnect(unsigned __int64 clientSteamID)
{
}

bool __cdecl LiveStorage_DoWeHaveContracts()
{
    return {};
}

bool __cdecl LiveStorage_DoWeHavePlaylists()
{
    return s_havePlaylists;
}

bool __cdecl LiveStorage_FetchingOnlineWAD()
{
    return {};
}

void __cdecl LiveStorage_ReadStatsIfDirChanged()
{
}

void __cdecl LiveStorage_UploadStats()
{
}

void __cdecl LiveStorage_UploadStatsForController()
{
}

void __cdecl LiveStorage_FetchPlaylistsLocal(int controllerIndex)
{
    if ( Playlist_ReadFromDisk() )
    {
        s_havePlaylists = true;
        Playlist_ValidatePlaylistNum();
    }
}

int __cdecl LiveStorage_GetMatchesPlayed(const char *gameModePrefix, int resetPeriod)
{
    return {};
}

void SV_CommitClientLeaderboards()
{
}

void __cdecl SV_OnlineReadClientStats(client_t *client)
{
}

void __cdecl SV_OnlineReadClientCAC(client_t *client)
{
}

void __cdecl SV_OnlineWriteClientStats(client_t *client)
{
}

char __cdecl SV_CACValidate_SetIntStat(unsigned __int8 *buffer, const char *stat, unsigned int value)
{
    return {};
}

void __cdecl SV_SetShouldMapRotate(bool should)
{
}

void __cdecl SV_FetchWADDeferred()
{
}

bool __cdecl LiveStorage_FirstTimeRunning()
{
    return s_firstTimeRunning;
}

void __cdecl LiveStorage_SetFirstTimeRunning(bool running)
{
    s_firstTimeRunning = running;
}

void __cdecl LiveStorage_DeleteGlobalStats(int controllerIndex)
{
}

char *__cdecl LiveTicker_GetCurrentMessage(
                float scrX,
                float baseX,
                float baseY,
                float baseAlpha,
                Font_s *const font,
                float scale,
                float *xCoord,
                float *yCoord,
                float *alpha)
{
    return (char *)"";
}

char *__cdecl LiveTicker_GetCurrentMessageCategory(
                float baseAlpha,
                float baseX,
                float baseWidth,
                Font_s *font,
                float baseScale,
                float *outXCoord,
                float *outAlpha,
                float *outScale)
{
    return (char *)"";
}

bool __cdecl Live_IsUserSignedInToLive()
{
    return SteamUser() && SteamUser()->GetSteamID().IsValid() && SteamUser()->BLoggedOn();
}

bool __cdecl Live_IsUserSignedInToOnlineService(int controllerIndex)
{
    return {};
}

char __cdecl Live_RequireUserToPlayOnline()
{
    return SteamUser() && SteamUser()->GetSteamID().IsValid() && SteamUser()->BLoggedOn();
}

char *__cdecl Live_ControllerIndex_GetClientName(int controllerIndex)
{
    return (char *)"kisak";
}

void __cdecl Live_AcceptLastInvite_f()
{
}

void __cdecl Live_GiveAchievement(int localControllerIndex, const char *achievementName)
{
}

bool __cdecl Live_IsSignedIn(int controllerIndex)
{
    return {};
}

bool __cdecl Live_IsSignedInToLive()
{
    return SteamUser() && SteamUser()->GetSteamID().IsValid() && SteamUser()->BLoggedOn();
}

unsigned long long __cdecl Live_GetXuid(int controllerIndex)
{
    return {};
}

int __cdecl Live_GetTier(int controllerIndex)
{
    return {};
}

char __cdecl Live_ShowMarketplaceUI()
{
    return {};
}

int __cdecl Live_GetUploadSpeed()
{
    return {};
}

void __cdecl PC_InitSigninState()
{
}

void __cdecl Live_DelayedComError(const char *comErrorString)
{
}

void __cdecl Live_GetFriendsOnServer(unsigned __int64 serverId, unsigned __int64 *friendIDs, int *numfriends)
{
}

char __cdecl Live_AddFavourite_Ingame(unsigned __int64 serverid, unsigned __int64 serveruid)
{
    return {};
}

void __cdecl Live_AddFavourite(unsigned __int64 serverid, unsigned __int64 serveruid)
{
}

void __cdecl Live_DeleteFavourite(unsigned __int64 serverid)
{
}

void __cdecl Live_AddRecentPlayers(unsigned __int64 *uids, const char **names, int numIDs)
{
}

void __cdecl Live_JoinWagerFromInvite()
{
}

void __cdecl Live_AcceptInvite(unsigned __int64 frienduid)
{
}

int __cdecl Live_GetInvitesCount()
{
    return {};
}

int __cdecl Live_GetInviteFriend(int index)
{
    return {};
}

void __cdecl SocketRouter_EmergencyFrame()
{
}
