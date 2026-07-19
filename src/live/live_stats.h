#pragma once
#include <ddl/ddl_api.h>
#include <universal/com_stringtable.h>
#include <ui/ui_shared.h>
#include <bgame/bg_weapons_attachment.h>
#include <universal/dvar.h>
#include <client/cl_milestone.h>
#include "live_storage.h"

enum sortedChallengeParams_t : __int32
{                                       // XREF: ?LiveStats_GetChallengeInfoParam@@YA_NW4sortedChallengeParams_t@@IPAPBDPAHH@Z/r
    CHALLENGE_PARAM_TYPE         = 0x0,
    CHALLENGE_PARAM_TIER         = 0x1,
    CHALLENGE_PARAM_NAME         = 0x2,
    CHALLENGE_PARAM_ITEM_INDEX   = 0x3,
    CHALLENGE_PARAM_IS_ITEM_CLASSIFIED = 0x4,
    CHALLENGE_PARAM_NUM_TO_TARGET_VALUE = 0x5,
    CHALLENGE_PARAM_TARGETVALUE  = 0x6,
    CHALLENGE_PARAM_CURRENTVALUE = 0x7,
    CHALLENGE_PARAM_PREVTARGET   = 0x8,
    CHALLENGE_PARAM_XP_REWARD    = 0x9,
    CHALLENGE_PARAM_CP_REWARD    = 0xA,
    CHALLENGE_PARAM_INVALID      = -1,
};

enum challengeSortOrder_e : __int32
{                                       // XREF: ?LiveStats_BuildChallengeListCmd@@YAXXZ/r
                                        // LiveStats_BuildChallengeList/r ...
    SORTORDER_FIRST               = 0x0,
    SORTORDER_NEAREST             = 0x0,
    SORTORDER_REWARD              = 0x1,
    SORTORDER_PERCENTAGECOMPLETED = 0x2,
    SORTORDER_DEFAULT             = 0x3,
    SORTORDER_COUNT               = 0x4,
    SORTORDER_INVALID             = -1,
};

enum pointsSpent_t : __int32
{                                       // XREF: ?LiveStats_SpendCurrency@@YA_NHHW4pointsSpent_t@@H@Z/r
                                        // ?MatchRecordPointsSpent@@YAXHHW4pointsSpent_t@@H@Z/r
    POINTSSPENT_IGNORE             = 0x0,
    POINTSSPENT_EMBLEM_LAYER       = 0x1,
    POINTSSPENT_EMBLEM_ICON        = 0x2,
    POINTSSPENT_CONTRACT           = 0x3,
    POINTSSPENT_UNLOCKABLE_CLANTAG = 0x4,
    POINTSSPENT_UNLOCKABLE_ITEM    = 0x5,
    POINTSSPENT_UNLOCKABLE_SELL    = 0x6,
    POINTSSPENT_UNLOCKABLE_ATTACHMENT = 0x7,
    POINTSSPENT_UNLOCKABLE_ATTACHMENT_POINT = 0x8,
    POINTSSPENT_UNLOCKABLE_ATTACHMENT_OPTION = 0x9,
    POINTSSPENT_ESCROW             = 0xA,
    POINTSSPENT_GUEST_SIGNIN       = 0xB,
};

enum playerStatsKeyIndex_t : __int32
{                                       // XREF: ?LiveStats_GetIntPlayerStatByKey@@YA_NHPAHW4playerStatsKeyIndex_t@@@Z/r
                                        // ?LiveStats_GetIntOtherPlayerStatByKey@@YA_NHPAHW4playerStatsKeyIndex_t@@@Z/r ...
    MP_PLAYERSTATSKEY_RANKXP         = 0x0,
    MP_PLAYERSTATSKEY_PLEVEL         = 0x1,
    MP_PLAYERSTATSKEY_SCORE          = 0x2,
    MP_PLAYERSTATSKEY_TIMEPLAYEDTOTAL = 0x3,
    MP_PLAYERSTATSKEY_GAMETYPEBAN    = 0x4,
    MP_PLAYERSTATSKEY_TIMEWHENNEXTHOST = 0x5,
    MP_PLAYERSTATSKEY_BADHOSTCOUNT   = 0x6,
    MP_PLAYERSTATSKEY_LEADERBOARDFAILURES = 0x7,
    MP_PLAYERSTATSKEY_LASTSTATSBACKUP = 0x8,
    MP_PLAYERSTATSKEY_MAPPACKMASK    = 0x9,
    MP_PLAYERSTATSKEY_STATSBACKUPVERSION = 0xA,
    MP_PLAYERSTATSKEY_MP_MAP_SKIPPED = 0xB,
    MP_PLAYERSTATSKEY_COOP_MAP_SKIPPED = 0xC,
    MP_PLAYERSTATSKEY_CODPOINTS      = 0xD,
    MP_PLAYERSTATSKEY_CURRENCYSPENT  = 0xE,
    MP_PLAYERSTATSKEY_STATS_VERSION  = 0xF,
    MP_PLAYERSTATSKEY_KILLS          = 0x10,
    MP_PLAYERSTATSKEY_DEATHS         = 0x11,
    MP_PLAYERSTATSKEY_DEATHSDURINGUSE = 0x12,
    MP_PLAYERSTATSKEY_HEADSHOTS      = 0x13,
    MP_PLAYERSTATSKEY_KDRATIO        = 0x14,
    MP_PLAYERSTATSKEY_HIGHEST_KDRATIO = 0x15,
    MP_PLAYERSTATSKEY_ACCURACY       = 0x16,
    MP_PLAYERSTATSKEY_HIGHEST_ACCURACY = 0x17,
    MP_PLAYERSTATSKEY_SHOTS          = 0x18,
    MP_PLAYERSTATSKEY_HITS           = 0x19,
    MP_PLAYERSTATSKEY_TIMEUSED       = 0x1A,
    MP_PLAYERSTATSKEY_USED           = 0x1B,
    MP_PLAYERSTATSKEY_DESTROYED      = 0x1C,
    MP_PLAYERSTATSKEY_GAMETYPE       = 0x1D,
    MP_PLAYERSTATSKEY_WINS           = 0x1E,
    MP_PLAYERSTATSKEY_LOSSES         = 0x1F,
    MP_PLAYERSTATSKEY_TIES           = 0x20,
    MP_PLAYERSTATSKEY_LIFETIME_EARNINGS = 0x21,
    MP_PLAYERSTATSKEY_LIFETIME_BUYIN = 0x22,
    MP_PLAYERSTATSKEY_CONTRACTS_PURCHASED = 0x23,
    MP_PLAYERSTATSKEY_CONTRACTS_COMPLETED = 0x24,
    MP_PLAYERSTATSKEY_LASTESCROW     = 0x25,
    MP_PLAYERSTATSKEY_WEEKLYTIMESTAMP = 0x26,
    MP_PLAYERSTATSKEY_MONTHLYTIMESTAMP = 0x27,
    MP_PLAYERSTATSKEY_COUNT          = 0x28,
};

struct personalBest_t // sizeof=0x14
{                                       // XREF: .data:s_personalBests/r
    const char *name;
    const char *prefix;
    int value;
    int delta;
    int rating;
};

struct statsMilestone_t // sizeof=0xC
{                                       // XREF: .data:s_statsMilestonesCompleted/r
    statsMilestoneTypes_t type;         // XREF: LiveStats_PublishNews(int)+1DF/r
    unsigned __int8 milestone;          // XREF: LiveStats_PublishNews(int)+24C/r
    unsigned __int8 index;              // XREF: LiveStats_PublishNews(int)+1FC/r
                                        // LiveStats_PublishNews(int)+21F/r
    // padding byte
    // padding byte
    const char *name;                   // XREF: LiveStats_PublishNews(int)+2AF/r
};

struct challengeInfo_t // sizeof=0x24
{                                       // XREF: .data:s_sortedWeaponGroupList/r
                                        // .data:s_sortedAttachmentsList/r ...
    unsigned __int8 milestone;
    unsigned __int8 index;
    unsigned __int16 flags;
    statsMilestoneTypes_t type;
    int numToTargetValue;
    int xpReward;
    int cpReward;
    int targetValue;
    int currentValue;
    int prevTarget;
    const char *name;
};

int __cdecl LiveStats_GetDDLHeaderVersion(unsigned __int8 *statsBuffer);
int __cdecl LiveStats_ValidateGlobalWithDDL(int controllerIndex);
int __cdecl LiveStats_CanPerformStatOperation(int controllerIndex);
unsigned int __cdecl LiveStats_GetDDLIntStat(int controllerIndex, ddlState_t *searchState);
unsigned int __cdecl LiveStats_GetDDLIntStat(int controllerIndex, const char **operands, int nOperands);
unsigned int __cdecl LiveStats_GetDIntStat(int controllerIndex, ddlState_t *searchState);
unsigned int __cdecl LiveStats_GetDIntStatFromBase(
                int controllerIndex,
                ddlState_t *searchState,
                persistentStats *buffer);
unsigned int __cdecl LiveStats_GetDInt64Stat(int controllerIndex, ddlState_t *searchState);
char *__cdecl LiveStats_GetDStringStat(int controllerIndex, ddlState_t *searchState);
char __cdecl LiveStats_GetIntPlayerStatInternal(int *outInt, int pathDepth, const char **statName, char *buffer);
char __cdecl LiveStats_GetIntPlayerStatFromBase(int *outInt, const char *statName, char *base);
char __cdecl LiveStats_GetIntPlayerStatFromForcedBase(int *outInt, const char *statName, char *base);
unsigned int __cdecl LiveStats_GetDIntStatFromForcedBase(int controllerIndex, ddlState_t *searchState, char *buffer);
unsigned int __cdecl LiveStats_GetDInt64StatFromForcedBase(int controllerIndex, ddlState_t *searchState, char *buffer);
char *__cdecl LiveStats_GetDStringStatFromForcedBase(int controllerIndex, ddlState_t *searchState, char *buffer);
char __cdecl LiveStats_GetIntPlayerStat(int controllerIndex, int *outInt, const char *statName);
char __cdecl LiveStats_GetIntPlayerStatByGameMode(
                int controllerIndex,
                int *outInt,
                const char *gameMode,
                const char *statName);
char __cdecl LiveStats_GetIntPlayerStatByGameModeFromBase(
                int controllerIndex,
                int *outInt,
                const char *gameMode,
                const char *statName,
                char *base);
char __cdecl LiveStats_GetIntOtherPlayerStat(int controllerIndex, int *outInt, const char *statName);
char __cdecl LiveStats_GetIntOtherPlayerStatByGameMode(
                int controllerIndex,
                int *outInt,
                const char *gameMode,
                const char *statName);
char __cdecl LiveStats_GetIntPlayerStatByKey(int controllerIndex, int *outInt, playerStatsKeyIndex_t keyIndex);
char __cdecl LiveStats_GetIntOtherPlayerStatByKey(int controllerIndex, int *outInt, playerStatsKeyIndex_t keyIndex);
const char *__cdecl LiveStats_GetPlayerStatStringByKey(playerStatsKeyIndex_t keyIndex);
char __cdecl LiveStats_SetIntPlayerStatInternal(
                int controllerIndex,
                int pathDepth,
                const char **path,
                char *buffer,
                unsigned int statValue);
bool __cdecl LiveStats_WriteVersionToBuffer(char *buffer);
unsigned int __cdecl LiveStats_ReadVersionFromBuffer(char *buffer);
char __cdecl LiveStats_GetIntPlayerStatMatchDeltaByGameMode(
                int controllerIndex,
                int *outInt,
                const char *gameMode,
                const char *statName);
char __cdecl LiveStats_SetIntPlayerStat(int controllerIndex, const char *statName, unsigned int statValue);
char __cdecl LiveStats_GetIntPlayerBackupStatByKey(int controllerIndex, int *outInt, playerStatsKeyIndex_t keyIndex);
int __cdecl LiveStats_ChecksumGamerStats(unsigned __int8 *buffer, int len);
void __cdecl LiveStats_SetDInt64Stat(int controllerIndex, ddlState_t *searchState, unsigned __int64 value);
int __cdecl LiveStats_HexToDec(const char *hexValue, int hexLength);
void __cdecl LiveStats_ProcessStatChangedData(
                int controllerIndex,
                char *buffer,
                int bufferSize,
                int offset,
                int size,
                const char *data);
void __cdecl LiveStats_SetStatChanged(int controllerIndex, const char *hexMsg);
void __cdecl LiveStats_SetStatByKey(int controllerIndex, playerStatsKeyIndex_t keyIndex, int value);
void __cdecl LiveStats_MakeStableStatsBuffer(int controllerIndex);
bool __cdecl LiveStats_IsStableStatsBufferInitialized(int controllerIndex);
void __cdecl LiveStats_MakeStableGlobalStatsBuffer(int controllerIndex);
void __cdecl LiveStats_SetPlayerStatByKey(
                const char *statsList,
                playerStatsKeyIndex_t key,
                unsigned int newvalue,
                unsigned __int8 *buffer);
void __cdecl LiveStats_SetPlayerStat(const char *statsList, const char *statName, char *buffer, unsigned int statValue);
void __cdecl LiveStats_SetItemStat(
                ddlState_t *searchStateStats,
                const char *statMember,
                char *buffer,
                unsigned int value);
int __cdecl LiveStats_GetMilestoneRowNum(
                XAssetHeader *statsMilestonestable,
                int currentMilestone,
                const char *statType,
                const char *statName);
void __cdecl LiveStats_CheckCurrentValue(
                const StringTable *statsMilestonestable,
                int rowNum,
                int targetColumn,
                int *currentValue);
int __cdecl LiveStats_GetTotalMatchesPlayed(int controllerIndex);
int __cdecl LiveStats_GetTotalMatchesPlayedByGameMode(int controllerIndex, const char *gameModePrefix);
unsigned int __cdecl LiveStats_GetNumStatsMilestones(int controllerIndex);
unsigned int __cdecl LiveStats_GetStatsMilestoneTier(int controllerIndex, unsigned int statsMilestoneNum);
statsMilestone_t *__cdecl LiveStats_GetStatsMilestone(int controllerIndex, unsigned int statsMilestoneNum);
int __cdecl LiveStats_GetStatsMilestoneItemIndex(int controllerIndex, unsigned int statsMilestoneNum);
const char *__cdecl LiveStats_GetStatsMilestoneName(int controllerIndex, unsigned int statsMilestoneNum);
int __cdecl LiveStats_GetStatsMilestoneType(int controllerIndex, unsigned int statsMilestoneNum);
unsigned int __cdecl LiveStats_GetChallengeCount(int type);
bool __cdecl StringBeginsWith(const char *prefix, const char *name);
bool __cdecl LiveStats_GetChallengeInfo(challengeInfo_t **challenge, unsigned int challengeNum, int type);
bool __cdecl LiveStats_GetChallengeInfoParam(
                sortedChallengeParams_t param,
                unsigned int challengeNum,
                const char **stringValue,
                int *value,
                int type);
unsigned int __cdecl LiveStats_GetNumPersonalBests(int controllerIndex);
char *__cdecl LiveStats_GetPersonalBestName(int controllerIndex, unsigned int personalBestNum);
personalBest_t *__cdecl LiveStats_GetPersonalBest(int controllerIndex, unsigned int personalBestNum);
char *__cdecl LiveStats_GetPersonalBestPrefix(int controllerIndex, unsigned int personalBestNum);
int __cdecl LiveStats_GetPersonalBestValue(int controllerIndex, unsigned int personalBestNum);
int __cdecl LiveStats_GetPersonalBestDelta(int controllerIndex, unsigned int personalBestNum);
int __cdecl LiveStats_GetPersonalBestRating(int controllerIndex, unsigned int personalBestNum);
unsigned int __cdecl LiveStats_GetNumRecentlyUnlockedItems(int controllerIndex);
int __cdecl LiveStats_GetRecentlyUnlockedItemIndex(int controllerIndex, unsigned int itemNum);
void __cdecl LiveStats_PublishNews(int controllerIndex);
char __cdecl LiveStats_MoveToProItemVersion(ddlState_t *searchState, int itemIndex, int challengeIndex);
void __cdecl LiveStats_CompareStatsVsStableBuffer(int controllerIndex);
char __cdecl LiveStats_UpdateItemPersonalBest(
                char *liveStatsBuffer,
                char *oldStatsBuffer,
                ddlState_t *searchStateStats);
unsigned int __cdecl LiveStats_GetItemStat(ddlState_t *searchStateStats, const char *statMember, char *buffer);
char __cdecl LiveStats_UpdateItemStatsMilestones(
                int controllerIndex,
                char *liveStatsBuffer,
                char *oldStatsBuffer,
                ddlState_t *searchStateStats,
                unsigned __int8 itemIndex);
char __cdecl LiveStats_UpdateGroupStatsMilestones(
                int controllerIndex,
                char *liveStatsBuffer,
                char *oldStatsBuffer,
                ddlState_t *searchStateStats,
                itemGroup_t groupIndex);
char __cdecl LiveStats_UpdateAttachmentStatsMilestones(
                int controllerIndex,
                char *liveStatsBuffer,
                char *oldStatsBuffer,
                ddlState_t *searchStateStats,
                eAttachment attachmentIndex);
char __cdecl LiveStats_UpdatePersonalBest(
                int __formal,
                char *liveStatsBuffer,
                char *oldStatsBuffer,
                ddlState_t *searchStateStats,
                ddlState_t *searchStateBests);
char __cdecl LiveStats_UpdateStatsMilestones(
                int controllerIndex,
                char *liveStatsBuffer,
                char *oldStatsBuffer,
                ddlState_t *searchStateStats);
char __cdecl LiveStats_UpdateGameModeStatsMilestones(
                int controllerIndex,
                char *liveStatsBuffer,
                char *oldStatsBuffer,
                ddlState_t *searchStateStats,
                const char *gameModeName);
char __cdecl LiveStats_UpdatePersonalBestHighest(
                int controllerIndex,
                char *liveStatsBuffer,
                char *oldStatsBuffer,
                ddlState_t *searchStateStats);
char __cdecl LiveStats_UpdatetBestMatchAccuracy(char *liveStatsBuffer, char *oldStatsBuffer);
unsigned int __cdecl LiveStats_GetPlayerStat(const char *statsList, const char *statName, char *buffer);
char __cdecl LiveStats_UpdatetBestMatchKdRatio(char *liveStatsBuffer, char *oldStatsBuffer);
void __cdecl LiveStats_SortPersonalBests(int controllerIndex);
int __cdecl LiveStats_ComparePersonalBests(unsigned int *arg0, unsigned int *arg1);
int __cdecl isPositive(int value);
char __cdecl LiveStats_ProVersionUnlocked(int itemIndex, char *statsBuffer);
void __cdecl LiveStats_CompareStatsVsStableBufferCmd();
void __cdecl LiveStats_SortPercentageCompleted(int type);
int __cdecl LiveStats_ChallengeComparePercentageCompleted(const challengeInfo_t *arg0, const challengeInfo_t *arg1);
int __cdecl LiveStats_SecondaryChallengeComparison(
                const challengeInfo_t *challenge0,
                const challengeInfo_t *challenge1,
                int delta);
void __cdecl LiveStats_SortNearestToCompletion(int type);
int __cdecl LiveStats_ChallengeCompareNumToTargetValue(const challengeInfo_t *arg0, const challengeInfo_t *arg1);
void __cdecl LiveStats_SortReward(int type);
int __cdecl LiveStats_ChallengeCompareReward(const challengeInfo_t *arg0, const challengeInfo_t *arg1);
void __cdecl LiveStats_SortDefault(int type);
int __cdecl LiveStats_ChallengeCompareDefault(const challengeInfo_t *arg0, const challengeInfo_t *arg1);
void __cdecl LiveStats_AddChallenge(
                int controllerIndex,
                char *liveStatsBuffer,
                int itemNumber,
                statsMilestoneTypes_t challengeType);
char __cdecl LiveStats_AddItemChallenge(
                int controllerIndex,
                char *liveStatsBuffer,
                ddlState_t *searchStateStats,
                int itemIndex,
                statsMilestoneTypes_t type);
void __cdecl LiveStats_AddChallengeForRow(
                const StringTable *statsMilestonestable,
                challengeInfo_t *challenge,
                int currentMilestone,
                int currentValue,
                int rowNum,
                const char *name,
                const char *statTypeName,
                statsMilestoneTypes_t type);
void __cdecl LiveStats_SetChallengeFlags(int controllerIndex, int itemIndex, challengeInfo_t *challenge);
void __cdecl LiveStats_BuildChallengeListWithLocks(
                int controllerIndex,
                char *liveStatsBuffer,
                statsMilestoneTypes_t challengeType,
                int weaponGroup);
bool __cdecl LiveStats_AddItemForLock(int controllerIndex, int itemNumber, int lock);
void __cdecl LiveStats_BuildChallengeListCmd();
void __cdecl LiveStats_BuildChallengeList(
                int controllerIndex,
                statsMilestoneTypes_t challengeType,
                challengeSortOrder_e sortOrder,
                itemGroup_t weaponGroup);
char __cdecl LiveStats_AddGroupChallenge(char *liveStatsBuffer, ddlState_t *searchStateStats, int groupIndex);
void __cdecl LiveStats_SortChallenges(challengeSortOrder_e sortOrder, int type);
bool __cdecl FilterKillstreakChallengesType(ddlState_t *searchStateStats, const char *killstreakType);
char __cdecl FilterGamemodeStat(ddlState_t *searchGamemodeStateStats, const char *gamemode);
void __cdecl LiveStats_BuildKillstreakChallengeListCmd();
void __cdecl LiveStats_BuildKillstreakChallengeList(
                int controllerIndex,
                const char *killstreakType,
                challengeSortOrder_e sortOrder);
void __cdecl LiveStats_AddGlobalKillstreakChallenges(char *liveStatsBuffer, const char *killstreakType);
char __cdecl LiveStats_AddGlobalChallenge(
                char *liveStatsBuffer,
                ddlState_t *searchStateStats,
                challengeInfo_t *challenge,
                const char *globalChallengesType);
void __cdecl LiveStats_BuildLifeTimeGlobalChallengesList(char *liveStatsBuffer);
char __cdecl LiveStats_AddGlobalLifetimeChallenge(
                char *liveStatsBuffer,
                ddlState_t *searchStateStats,
                challengeInfo_t *challenge);
void __cdecl LiveStats_BuildGamemodeChallengesList(int controllerIndex, char *liveStatsBuffer, const char *gamemode);
char __cdecl LiveStats_AddGamemodeChallenge(
                int controllerIndex,
                char *liveStatsBuffer,
                ddlState_t *searchStateStats,
                ddlState_t *searchGameModeStateStats);
void __cdecl LiveStats_BuildGlobalChallengesListCmd();
void __cdecl LiveStats_BuildGlobalChallengesList(
                int controllerIndex,
                const char *globalChallengesType,
                challengeSortOrder_e sortOrder);
void __cdecl LiveStats_AddUnderBarrelAttachments(int controllerIndex, char *liveStatsBuffer);
char __cdecl LiveStats_AddUnderBarrelAttachmentChallenge(
                int controllerIndex,
                char *liveStatsBuffer,
                ddlState_t *searchStateStats,
                int itemIndex,
                statsMilestoneTypes_t type);
const char *__cdecl LiveStats_GetAttachmenNameFromItemIndex(int itemIndex);
void __cdecl LiveStats_BuildAttachmentsListCmd();
void __cdecl LiveStats_BuildAttachmentsList(int controllerIndex, challengeSortOrder_e sortOrder);
char __cdecl LiveStats_AddAttachmentsChallenge(
                int controllerIndex,
                char *liveStatsBuffer,
                ddlState_t *searchAttachmentStateStats,
                const char *attachmentName);
bool __cdecl LiveStats_ValidateChecksum(unsigned __int8 *buffer, int len);
char __cdecl LiveStats_ValidateStatsBackup(int controllerIndex);
void __cdecl LiveStats_ValidateStats(int controllerIndex);
char __cdecl LiveStats_GetIntPlayerStatByKey(
                int controllerIndex,
                int *outInt,
                statsLocation location,
                playerStatsKeyIndex_t statKey);
void __cdecl LiveStats_ClearCustomClassWarning(int controllerIndex);
int __cdecl LiveStats_GetClanTagFeature(int controllerIndex);
ddlState_t *__cdecl LiveStats_GetCacDDLState();
ddlState_t *__cdecl LiveStats_GetRootDDLState();
ddlDef_t *__cdecl LiveStats_GetStatsDDL();
unsigned int __cdecl LiveStats_GetXp(int controllerIndex);
int __cdecl LiveStats_GetRank(int controllerIndex);
int __cdecl LiveStats_GetPrestige(int controllerIndex);
int __cdecl LiveStats_GetCurrency(int controllerIndex);
char __cdecl LiveStats_SpendCurrency(
                int controllerIndex,
                int currencyAmount,
                pointsSpent_t reasonType,
                int reasonIndex);
void __cdecl LiveStats_TrackSpending(int controllerIndex, int currencyAmount);
char __cdecl LiveStats_WriteXUIDToStats(int controllerIndex);
unsigned __int64 __cdecl LiveStats_ReadXUIDFromStats();
bool __cdecl LiveStats_WritePrestigeToStats(int controllerIndex);
void __cdecl LiveStats_ResetBasicTrainingStats(int controllerIndex);
void __cdecl LiveStats_SetBufferInitialised(int controllerIndex, bool isInitialised);
void __cdecl LiveStats_InitStatsBuffer(int controllerIndex);
void __cdecl LiveStats_ResetStats(int controllerIndex, bool versionChanged);
char __cdecl LiveStats_GetActiveContractStatState(
                unsigned int activeContractIndex,
                const char *statName,
                ddlState_t *searchState);
char __cdecl LiveStats_GetContractStatState(unsigned int contractIndex, const char *statName, ddlState_t *searchState);
unsigned int __cdecl LiveStats_GetActiveContractStat(
                char *buffer,
                unsigned int activeContractIndex,
                const char *statName);
bool __cdecl LiveStats_SetActiveContractStat(
                char *buffer,
                unsigned int activeContractIndex,
                const char *statName,
                unsigned int newValue);
unsigned int __cdecl LiveStats_GetContractStat(char *buffer, unsigned int contractIndex, const char *statName);
bool __cdecl LiveStats_SetContractStat(
                char *buffer,
                unsigned int contractIndex,
                const char *statName,
                unsigned int newValue);
void __cdecl LiveStats_SetOnlineRankedState(unsigned __int8 *buffer, bool value);
bool __cdecl LiveStats_GetOnlineRankedState(unsigned __int8 *buffer);
void __cdecl LiveStats_SetBasicTrainingState(unsigned __int8 *buffer, bool value);
bool __cdecl LiveStats_GetBasicTrainingState(unsigned __int8 *buffer);
void __cdecl LiveStats_GetNemesisXuid(int controllerIndex, unsigned __int64 *nemesisXuid);
bool __cdecl LiveStats_PrestigeLeaderboardsEnabled(int controllerIndex);
void __cdecl LiveStats_ResetGamemodeChallenges(char *liveStatsBuffer);
void __cdecl LiveStats_PresetigeStatsResetCmd();
void __cdecl SV_UpdatePersonalBestsForClient(int clientnum);
void __cdecl LiveStats_Init();
void __cdecl LiveStats_StatSetByNameCmd();
void __cdecl LiveStats_StatAddByNameCmd();
void __cdecl LiveStats_StatGetByNameInDvarCmd();
void __cdecl LiveStats_StatReadDDLCmd();
void __cdecl LiveStats_StatWriteDDLCmd();
void __cdecl LiveStats_ActivateContractCmd();
void __cdecl LiveStats_SetCurrentContractCmd();
void __cdecl LiveStats_WriteChecksumToBuffer(unsigned __int8 *buffer, int len);

extern const dvar_t *ui_challengeSort;
extern const dvar_t *debugStats;
extern const dvar_t *ui_numPersonalBests;
extern const dvar_t *ui_challengeSort;
extern const dvar_t *recordPointsSpent;
extern const dvar_t *ui_challengeGameMode;

extern ddlDef_t *g_statsDDL;