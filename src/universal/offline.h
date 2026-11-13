#pragma once

// What is left of the online layer: the types the game still stores and
// the entry points it still calls. None of them talk to a service.

#include <qcommon/common.h>
#include <server_mp/sv_main_mp.h>
#include <demo/demo_common.h>
#include <universal/com_stringtable.h>
#include <universal/dvar.h>
#include <gfx_d3d/r_material.h>
#include <client_mp/cl_ui_pc_mp.h>
#include <universal/q_shared.h>
#include <Windows.h>
#include <client/cl_compositing.h>
#include <ddl/ddl_api.h>
#include <bgame/bg_local.h>
#include <client_mp/cl_main_mp.h>
#include <ui/ui_shared.h>
#include <bgame/bg_weapons_attachment.h>
#include <client/cl_milestone.h>
#include <cstdio>
#include <steam/steam_api.h>
#include <steam/isteamgameserver.h>
#include <qcommon/net_chan_mp.h>
#include <universal/UserInfo.h>

int __cdecl Live_CountGuestsInUse();
bool __cdecl Live_IsPublicOnlineMatch();
bool __cdecl Live_IsWagerMatch();
bool __cdecl Live_IsCombatTrainingMatch();
void __cdecl Live_Init();

char *__cdecl Clan_GetName(int controllerIndex);

enum sortedItemParams_t : __int32
{                                       // XREF: ?LiveCombatRecord_GetSortedItemData@@YAMH_NW4sortedItemParams_t@@@Z/r
    ITEM_INDEX               = 0x0,
    PARAM1                   = 0x1,
    PARAM2                   = 0x2,
    PARAM3                   = 0x3,
    SORT_KEY                 = 0x4,
    SORTEDITEM_PARAM_COUNT   = 0x5,
    SORTEDITEM_PARAM_INVALID = 0xFFFFFFFF,
};

enum itemSortingCriteria_t : __int32
{                                       // XREF: LiveCombatRecord_CombatRecordViewUpdateCmd/r
                                        // LiveCombatRecord_BuildSortedItemListByStats/r ...
    CRITERION_NULL             = 0xFFFFFFFF,
    PERSONAL_BESTS             = 0x0,
    RECENT_PERFORMANCE         = 0x1,
    FAVORITE_WEAPON            = 0x2,
    WEAPON_STATS               = 0x3,
    WEAPON_ACCURACY            = 0x4,
    WEAPON_HEADSHOTS           = 0x5,
    WEAPON_HITLOCATION_HEATMAP = 0x6,
    EQUIPMENT_KILLS            = 0x7,
    EQUIPMENT_USAGE            = 0x8,
    TIME_SPENT_USING_ITEM      = 0x9,
    MATCH_TYPES                = 0xA,
    WAGER_MATCH_TYPES          = 0xB,
    MATCH_PERFORMANCE          = 0xC,
    RECENT_EARNINGS            = 0xD,
    KILLSTREAKS_FAVORITES      = 0xE,
    KILLSTREAKS_CALLED         = 0xF,
    KILLSTREAKS_KILLS          = 0x10,
};

enum playerStatsKeyIndex_t : __int32;

enum sortedStatLoadoutSlots_t : __int32
{                                       // XREF: LiveCombatRecord_FilterCurrentSortedItem/r
    SLOT_NULL                    = 0xFFFFFFFF,
    PRIMARY_WEAPON               = 0x0,
    SECONDARY_WEAPON             = 0x1,
    PRIMARY_GRENADE              = 0x2,
    SPECIAL_GRENADE              = 0x3,
    EQUIPMENT                    = 0x4,
    FACEPAINT_PATTERN            = 0x5,
    BODY                         = 0x6,
    SPECIALTY1                   = 0x7,
    SPECIALTY2                   = 0x8,
    SORTEDSTAT_LOADOUTSLOT_COUNT = 0x9,
};

enum loadoutSlot_t : __int32
{                                       // XREF: itemInfo_t/r
                                        // LiveCombatRecord_CompareItemsByStats/r ...
    LOADOUTSLOT_INVALID              = 0xFFFFFFFF,
    LOADOUTSLOT_FIRST                = 0x0,
    LOADOUTSLOT_PRIMARY_WEAPON       = 0x0,
    LOADOUTSLOT_PRIMARY_ATTACHMENT   = 0x1,
    LOADOUTSLOT_PRIMARY_ATTACHMENT_TOP = 0x2,
    LOADOUTSLOT_PRIMARY_ATTACHMENT_BOTTOM = 0x3,
    LOADOUTSLOT_PRIMARY_ATTACHMENT_TRIGGER = 0x4,
    LOADOUTSLOT_PRIMARY_ATTACHMENT_MUZZLE = 0x5,
    LOADOUTSLOT_PRIMARY_CAMO         = 0x6,
    LOADOUTSLOT_PRIMARY_RETICLE      = 0x7,
    LOADOUTSLOT_PRIMARY_RETICLE_COLOR = 0x8,
    LOADOUTSLOT_PRIMARY_LENS         = 0x9,
    LOADOUTSLOT_PRIMARY_EMBLEM       = 0xA,
    LOADOUTSLOT_PRIMARY_TAG          = 0xB,
    LOADOUTSLOT_SECONDARY_WEAPON     = 0xC,
    LOADOUTSLOT_SECONDARY_ATTACHMENT = 0xD,
    LOADOUTSLOT_SECONDARY_ATTACHMENT_TOP = 0xE,
    LOADOUTSLOT_SECONDARY_ATTACHMENT_BOTTOM = 0xF,
    LOADOUTSLOT_SECONDARY_ATTACHMENT_TRIGGER = 0x10,
    LOADOUTSLOT_SECONDARY_ATTACHMENT_MUZZLE = 0x11,
    LOADOUTSLOT_SECONDARY_CAMO       = 0x12,
    LOADOUTSLOT_SECONDARY_RETICLE    = 0x13,
    LOADOUTSLOT_SECONDARY_RETICLE_COLOR = 0x14,
    LOADOUTSLOT_SECONDARY_LENS       = 0x15,
    LOADOUTSLOT_SECONDARY_EMBLEM     = 0x16,
    LOADOUTSLOT_SECONDARY_TAG        = 0x17,
    LOADOUTSLOT_FIRST_SPECIALTY      = 0x18,
    LOADOUTSLOT_SPECIALTY1           = 0x18,
    LOADOUTSLOT_SPECIALTY2           = 0x19,
    LOADOUTSLOT_SPECIALTY3           = 0x1A,
    LOADOUTSLOT_LAST_SPECIALTY       = 0x1A,
    LOADOUTSLOT_CLASSBONUS           = 0x1B,
    LOADOUTSLOT_HEAD                 = 0x1C,
    LOADOUTSLOT_BODY                 = 0x1D,
    LOADOUTSLOT_PRIMARY_GRENADE      = 0x1E,
    LOADOUTSLOT_SPECIAL_GRENADE      = 0x1F,
    LOADOUTSLOT_EQUIPMENT            = 0x20,
    LOADOUTSLOT_FACEPAINT_PATTERN    = 0x21,
    LOADOUTSLOT_FACEPAINT_COLOR      = 0x22,
    LOADOUTSLOT_GLOBAL_ITEMS_START   = 0x23,
    LOADOUTSLOT_KILLSTREAK1          = 0x23,
    LOADOUTSLOT_KILLSTREAK2          = 0x24,
    LOADOUTSLOT_KILLSTREAK3          = 0x25,
    LOADOUTSLOT_DEATHSTREAK          = 0x26,
    LOADOUTSLOT_COUNT                = 0x27,
};
inline loadoutSlot_t &operator++(loadoutSlot_t &t)
{
    t = static_cast<loadoutSlot_t>((static_cast<int>(t) + 1));
    return t;
}
inline loadoutSlot_t operator++(loadoutSlot_t &t, int)
{
    loadoutSlot_t old = t;
    t = static_cast<loadoutSlot_t>((static_cast<int>(t) + 1));
    return old;
}

struct sortedItemsData_t // sizeof=0x14
{                                       // XREF: LiveCombatRecord_BuildSortedItemListByStats+50C/o
                                        // .data:s_otherPlayerSortedItemList/r ...
    int itemIndex;
    float sortKey;
    int param1;
    int param2;
    int param3;
};
double __cdecl LiveCombatRecord_GetSortedItemData(int index, bool forFriend, sortedItemParams_t param);
int __cdecl LiveCombatRecord_GetSortedItemPositionByItemIndex(int itemIndex, bool forFriend);
const char *__cdecl LiveCombatRecord_GetEquipmentUsageStringSuffix(const char *equipmentName);
const char *__cdecl LiveCombatRecord_GetPersonalBestStatNameByIndex(int index);
const char *__cdecl LiveCombatRecord_GetGameTypeName(unsigned int index);
int __cdecl LiveCombatRecord_GetGameTypeLbIndex(unsigned int index);
int __cdecl LiveCombatRecord_GetGameTypeCount();
const char *__cdecl LiveCombatRecord_GetHitLocationImageRef(hitLocation_t hitLocation);

extern const dvar_t *ui_showFriendsCombatRecord;
extern const dvar_t *ui_combatCurrViewNum;
extern const dvar_t *ui_combatComparisonModeOn;
extern const dvar_t *ui_combatHistogramCurrGametype;

enum fileShareLocation : __int32
{                                                                             // XREF: fileShareWriteFileInfo/r
    FILESHARE_LOCATION_INVALID = 0x0,
    FILESHARE_LOCATION_PUBLISHERSTORAGE = 0x1,
    FILESHARE_LOCATION_USERSTORAGE = 0x2,
    FILESHARE_LOCATION_POOLEDSTORAGE = 0x3,
};
enum statsLocation : __int32
{                                       // XREF: ?LiveStorage_GetStatsBuffer@@YAPAEHW4statsLocation@@_N@Z/r
                                        // LiveStorage_GetPersStatsBuffer/r ...
    STATS_LOCATION_NORMAL        = 0x0,
    STATS_LOCATION_FORCE_NORMAL  = 0x1,
    STATS_LOCATION_BACKUP        = 0x2,
    STATS_LOCATION_STABLE        = 0x3,
    STATS_LOCATION_OTHERPLAYER   = 0x4,
    STATS_LOCATION_BASICTRAINING = 0x5,
    STATS_LOCATION_GLOBAL        = 0x6,
    STATS_LOCATION_GLOBALSTABLE  = 0x7,
    STATS_LOCATION_COUNT         = 0x8,
};

enum TaskState : __int32
{                                       // XREF: TaskRecord/r
                                        // TaskManager2_StateToString/r
    TASK_STATE_INVALID       = 0x0,
    TASK_STATE_INIT          = 0x1,
    TASK_STATE_INPROGRESS    = 0x2,
    TASK_STATE_CHILDCOMPLETE = 0x3,
    TASK_STATE_CHILDFAILED   = 0x4,
    TASK_STATE_COMPLETED     = 0x5,
    TASK_STATE_FAILED        = 0x6,
};

enum fileShareBufferLocation : __int32
{                                       // XREF: ?LiveStorage_FileShare_GetFileShareData@@YAPAUfileSharePrivateData@@W4fileShareBufferLocation@@@Z/r
                                        // ?LiveStorage_FileShare_ReadListing@@YAPAUTaskRecord@@H_KW4fileShareBufferLocation@@@Z/r ...
    FILESHARE_BUFFER_PRIMARY   = 0x0,
    FILESHARE_BUFFER_SECONDARY = 0x1,
};

enum LobbyErrorCode : __int32
{
    LOBBY_NO_ERROR = 0x0,
};

// handle to a pending online task; nothing creates these any more
struct RemoteTask
{
    enum TaskStatus : __int32
    {
        TASK_STATUS_EMPTY      = 0x0,
        TASK_STATUS_PENDING    = 0x1,
        TASK_STATUS_DONE       = 0x2,
        TASK_STATUS_FAILED     = 0x3,
        TASK_STATUS_TIMED_OUT  = 0x4,
        TASK_STATUS_CANCELLED  = 0x5,
        TASK_STATUS_MAX = 0x6,
    };
};

struct overlappedTask // sizeof=0x24
{                                       // XREF: .data:overlappedIO/r
    RemoteTask *overlappedIO;
    RemoteTask::TaskStatus finalStatus;
    LobbyErrorCode errorCode;
    int type;                           // XREF: MatchRecordEventComplete+51/r
                                        // LB_ReadHiddenLeaderboardComplete(int)+213/r ...
    bool active;                        // XREF: Live_CheckOngoingMatchRecordingTasks(void)+24/r
                                        // MatchRecordEventComplete+1A/r ...
    // padding byte
    // padding byte
    // padding byte
    int controllerIndex;                // XREF: MatchRecordEventComplete+86/r
                                        // LB_ReadLeaderboardComplete(int)+1E6/r ...
    void *data;                         // XREF: LB_ReadHiddenLeaderboardComplete(int)+8B/r
                                        // LB_ReadHiddenLeaderboardComplete(int)+1B7/r ...
    void *miscData;
    float startMS;
};

struct TaskRecord // sizeof=0x34
{                                       // XREF: .data:s_taskActiveHead/r
                                        // .data:s_taskRecords/r ...
    TaskRecord *next;                   // XREF: TaskManager2_Init(void)+4/w
                                        // TaskManager2_Init(void)+E/w ...
    const struct TaskDefinition *definition;   // XREF: TaskManager2_PickUpDeferredTasks(void)+101/r
                                        // TaskManager2_PickUpDeferredTasks(void)+120/r ...
    TaskState state;
    int controllerIndex;
    unsigned int lastPoll;
    unsigned int lastPollMS;
    int startMS;
    int timeOut;
    RemoteTask *remoteTask;
                                        // XREF: TaskManager2_PickUpDeferredTasks(void)+69/r
                                        // TaskManager2_PickUpDeferredTasks(void)+E5/r ...
    TaskRecord *nestedTask;
    void *payload;                      // XREF: TaskManager2_ProcessRemoteTask(TaskRecord *)+205/w
                                        // TaskManager2_ProcessRemoteTask(TaskRecord *)+386/w ...
    bool isChildTask;
    // padding byte
    // padding byte
    // padding byte
    int ownerThread;                    // XREF: TaskManager2_PickUpDeferredTasks(void)+AB/r
                                        // TaskManager2_ProcessRemoteTask(TaskRecord *)+21A/w ...
};

const struct __declspec(align(8)) TaskDefinition // sizeof=0x20
{                                       // XREF: .rdata:task_RequestCACValidate/r
                                        // .rdata:task_QuickMatchFindSessions/r ...
    unsigned __int64 category;
    const char *name;
    int payloadSize;
    void (__cdecl *completed_callback)(TaskRecord *);
    void (__cdecl *failure_callback)(TaskRecord *);
    TaskRecord *nestedTask;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct persistentStats // sizeof=0xA004
{                                       // XREF: .data:s_otherPlayerStats/r
    unsigned __int8 statsBuffer[40960];
    bool isChecksumValid;
    bool statsWriteNeeded;
    bool statsValidatedWithDDL;
    bool statsFetched;
};

enum StatWriteType
{                                       // XREF: LbStatsInfo/r
    STAT_WRITE_REPLACE = 0x0,
    STAT_WRITE_ADD     = 0x1,
    STAT_WRITE_MAX     = 0x2,
    STAT_WRITE_MIN     = 0x3,
    STAT_WRITE_REPLACE_WHEN_RATING_INCREASE = 0x4,
    STAT_WRITE_ADD_WHEN_RATING_INCREASE = 0x5,
    STAT_WRITE_MAX_WHEN_RATING_INCREASE = 0x6,
    STAT_WRITE_MIN_WHEN_RATING_INCREASE = 0x7,
};

struct LbStatsInfo : TaskResult // sizeof=0x70
{                                       // XREF: VoteRankStatsInfo/r
                                        // LeaderBoardRow<10>/r ...
    unsigned int m_leaderboardID;       // XREF: LB_IncrementEscrow(void)+F4/w
                                        // LB_IncrementEscrow(void)+14A/w ...
    unsigned __int64 m_entityID;        // XREF: LB_IncrementEscrow(void)+E2/w
                                        // LB_IncrementEscrow(void)+EE/w ...
    StatWriteType m_writeType;   // XREF: LB_IncrementEscrow(void)+FE/w
                                        // LB_IncrementEscrow(void)+154/w ...
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    __int64 m_rating;                   // XREF: LB_IncrementEscrow(void)+108/w
                                        // LB_IncrementEscrow(void)+112/w ...
    unsigned __int64 m_rank;
    char m_entityName[65];              // XREF: LB_ReadHiddenLeaderboardComplete(int)+572/o
    // padding byte
    // padding byte
    // padding byte
    unsigned int m_secondsSinceUpdate;

    inline LbStatsInfo() : TaskResult()
    {
        this->m_leaderboardID = 0;
        this->m_entityID = 0;
        this->m_writeType = STAT_WRITE_ADD;
        this->m_rating = 0;
        this->m_rank = 0;
        this->m_secondsSinceUpdate = 0;
        memset(this->m_entityName, 0, sizeof(this->m_entityName));
    }
};

struct __declspec(align(8)) VoteRankStatsInfo : LbStatsInfo // sizeof=0xC8
{                                       // XREF: fileSharePrivateData/r
                                        // fileShareSearchResults/r ...
    unsigned __int64 m_fileOwnerID;
    char m_fileOwnerName[65];
    // padding byte
    // padding byte
    // padding byte
    unsigned int m_totalVotes;
    unsigned int m_avgVoteValue;
    // padding byte
    // padding byte
    // padding byte
    // padding byte

    VoteRankStatsInfo() : LbStatsInfo()
    {
        this->m_fileOwnerID = 0;
        this->m_totalVotes = 0;
        this->m_avgVoteValue = 0;
        memset(this->m_fileOwnerName, 0, sizeof(this->m_fileOwnerName));
    }

    ~VoteRankStatsInfo() = default;
};

struct __declspec(align(8)) fileSharePrivateData // sizeof=0xB418
{                                       // XREF: .data:s_fileSharePrimary/r
                                        // .data:s_fileShareSecondary/r
    //~fileSharePrivateData();
    //fileSharePrivateData();

    bool valid;
    // padding byte
    // padding byte
    // padding byte
    int lastRead;
    unsigned __int64 playerXuid;
    FileShareMetaData descriptors[20];
    VoteRankStatsInfo fileRatings[20];
    int descriptorCount;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct fileRetryInfo // sizeof=0xC
{                                       // XREF: .data:s_UTCRetryInfo/r
                                        // FileOperationInfo/r
    int lastAttemptTime;
    int lastAttemptInterval;
    int retryCount;
};

enum taskCompleteResults : __int32
{                                       // XREF: Live_SetPlayerTeamRanksComplete/r
                                        // Live_QoSProbeComplete/r ...
    TASK_NOTCOMPLETE = 0x0,
    TASK_COMPLETE    = 0x1,
    TASK_ERROR       = 0x2,
};

struct StorageFileData : TaskResult // sizeof=0xC
{                                       // XREF: FileTask/r
    void *m_fileData;
    unsigned int m_fileSize;
};

struct __declspec(align(8)) StorageFileInfo : TaskResult // sizeof=0xB0
{                                       // XREF: FileTask/r
    enum Visibility : __int32
    {
        VISIBLE_PUBLIC = 0x0,
        VISIBLE_PRIVATE = 0x1,
        MAX_VISIBILITY_TYPE = 0x2,
    };
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    unsigned __int64 m_fileID;
    unsigned int m_createTime;
    unsigned int m_modifedTime;
    StorageFileInfo::Visibility m_visibility;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    unsigned __int64 m_ownerID;
    char m_fileName[128];
    unsigned int m_fileSize;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct __declspec(align(4)) FileTask // sizeof=0xE0
{                                       // XREF: .data:s_fetchPlaylistsFileTask/r
    FileTask()
    {
        this->m_filename = 0;
        this->m_error = LOBBY_NO_ERROR;
        this->m_fileID = 0;
        this->m_fileSize = 0;
        this->m_bufferSize = 0;
        this->m_buffer = 0;
        this->m_optional = 0;
    }

    char *m_filename;                   // XREF: SV_AP_GetControlFile(void)+31/w
    StorageFileData m_fileData;
    LobbyErrorCode m_error;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    unsigned __int64 m_fileID;
    unsigned int m_fileSize;
    unsigned int m_bufferSize;
    StorageFileInfo m_fileInfo;
    void *m_buffer;
    bool m_optional;
    // padding byte
    // padding byte
    // padding byte
};

struct __declspec(align(8)) FileOperationInfo // sizeof=0x120
{                                       // XREF: .data:s_finfo/r
    FileTask fileTask;                // XREF: SV_AP_GetControlFile(void)+31/w
                                        // LiveStorage_FetchOnlineWAD(int)+16A/w ...
    fileRetryInfo retryInfo;
    int taskType;
    bool fetchCompleted;                // XREF: LiveStorage_FetchOnlineWAD(int)+55/r
                                        // LiveStorage_FetchOnlineWAD(int)+98/w ...
    bool writeCompleted;
    bool isUserFile;                    // XREF: SV_AP_GetControlFile(void)+23/w
                                        // LiveStorage_FetchOnlineWAD(int)+15C/w
    bool isCompressedFile;              // XREF: SV_AP_GetControlFile(void)+2A/w
                                        // LiveStorage_FetchOnlineWAD(int)+163/w
    const char *menuDef;
    unsigned __int8 *fileBuffer;        // XREF: SV_AP_GetControlFile(void)+3B/w
                                        // LiveStorage_FetchOnlineWAD(int)+174/w ...
    int bufferSize;                     // XREF: SV_AP_GetControlFile(void)+45/w
                                        // LiveStorage_FetchOnlineWAD(int)+17E/w
    void (__cdecl *fileOperationSucessFunction)(const int, void *);
                                        // XREF: SV_AP_GetControlFile(void)+4F/w
                                        // LiveStorage_FetchOnlineWAD(int)+188/w
    taskCompleteResults (__cdecl *fileNotFoundFunction)(const int, void *);
                                        // XREF: SV_AP_GetControlFile(void)+59/w
                                        // LiveStorage_FetchOnlineWAD(int)+192/w
    void (__cdecl *fileOperationFailureFunction)(const int, void *);
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    unsigned __int64 ownerID;
    bool *alreadyUploaded;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct FileShareGetRatingTask // sizeof=0x24
{                                       // XREF: playerFileOperations/r
    fileShareLocation location;
    unsigned __int64 *fileIDs;
    int maxFilesToFetch;
    int leaderboardID;
    VoteRankStatsInfo *outStatsInfo;
    int *outNumRatingsReceived;
    int cacheLoadedFileCount;
    void (__cdecl *successCallback)();
    void (__cdecl *failureCallback)();
};

struct FileShareFileID : TaskResult // sizeof=0x10
{                                       // XREF: .data:searchFileIDs/r
                                        // fileShareSearchResults/r ...
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    unsigned __int64 m_fileID;          // XREF: Live_FileShare_GetLastPlayedGameDetails(int)+34/w
                                        // Live_FileShare_GetLastPlayedGameDetails(int)+39/w ...
};

struct FileShareDescriptorsTask // sizeof=0x24
{                                       // XREF: playerFileOperations/r
    fileShareLocation location;
    unsigned int numFiles;
    FileShareFileID *fileIDs;
    int cacheLoadedFileCount;
    FileShareMetaData *descriptors;
    int *descriptorCount;
    bool getFirstSummary;
    bool getLastSummary;
    // padding byte
    // padding byte
    void (__cdecl *successCallback)();
    void (__cdecl *failureCallback)();
};

struct __declspec(align(8)) fileShareReadFileInfo // sizeof=0x30
{                                       // XREF: ?Demo_DownloadFile@@YAXHPAD_KH_N2@Z/r
                                        // ?UI_ScreenshotDownload@@YAXHH_KI@Z/r ...
    unsigned __int64 fileID;            // XREF: UI_Gametype_DownloadFromFileShare_f(void)+56/w
                                        // UI_Gametype_DownloadFromFileShare_f(void)+5C/w ...
    unsigned int fileSize;              // XREF: UI_Gametype_DownloadFromFileShare_f(void)+62/w
                                        // SV_GameType_DownloadFromFileshare_f(void)+10D/w ...
    fileShareLocation location;         // XREF: UI_Gametype_DownloadFromFileShare_f(void)+65/w
                                        // SV_GameType_DownloadFromFileshare_f(void)+110/w ...
    char *buffer;                       // XREF: UI_Gametype_DownloadFromFileShare_f(void)+80/w
                                        // UI_Gametype_DownloadFromFileShare_f(void)+8E/r ...
    unsigned int bufferSize;            // XREF: UI_Gametype_DownloadFromFileShare_f(void)+83/w
                                        // SV_GameType_DownloadFromFileshare_f(void)+12E/w ...
    bool isStreamed;                    // XREF: UI_Gametype_DownloadFromFileShare_f(void)+8A/w
                                        // SV_GameType_DownloadFromFileshare_f(void)+135/w ...
    // padding byte
    // padding byte
    // padding byte
    void *cacheBuffer;                  // XREF: UI_Gametype_DownloadFromFileShare_f(void)+91/w
                                        // SV_GameType_DownloadFromFileshare_f(void)+13C/w ...
    void (__cdecl *dataCallback)(void *, unsigned int, unsigned int, unsigned int);
                                        // XREF: Demo_DownloadFile(int,char *,unsigned __int64,int,bool,bool)+A6/w
    void (__cdecl *successCallback)(FileShareReadFileTask *);
                                        // XREF: UI_Gametype_DownloadFromFileShare_f(void)+94/w
                                        // SV_GameType_DownloadFromFileshare_f(void)+13F/w ...
    void (__cdecl *failureCallback)(FileShareReadFileTask *);
                                        // XREF: UI_Gametype_DownloadFromFileShare_f(void)+9B/w
                                        // SV_GameType_DownloadFromFileshare_f(void)+146/w ...
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct fileShareDownloadInterceptor : DownloadInterceptor // sizeof=0xC
{                                       // XREF: .data:downloadInterceptor/r
    FileShareReadFileTask *task;      // XREF: LiveStorage_FileShare_ReadFile(int,fileShareReadFileInfo *)+1BF/w
    unsigned int (__cdecl *callback)(void *, unsigned int, FileShareReadFileTask *);
                                        // XREF: LiveStorage_FileShare_ReadFile(int,fileShareReadFileInfo *)+1B2/w

    unsigned int handleDownload(void *data, unsigned int dataSize);
};

struct UploadInterceptor // sizeof=0x4
{                                       // XREF: fileShareUploadInterceptor/r
    //UploadInterceptor_vtbl *__vftable;
                                        // XREF: LiveStorage_FileShare_WriteFile(int,fileShareWriteFileInfo *)+29/w

    virtual ~UploadInterceptor() = default;
    virtual unsigned int handleUpload(void *, unsigned int, unsigned int);
};

struct __declspec(align(8)) FileShareWriteFileTask // sizeof=0x50
{                                       // XREF: playerFileOperations/r
    fileShareLocation location;
    unsigned __int16 fileSlot;
    // padding byte
    // padding byte
    const char *menuDef;
    void *fileData;
    UploadInterceptor *uploadHandler;
    unsigned int fileSize;
    const char *fileName;
    unsigned __int16 category;
    // padding byte
    // padding byte
    const void *thumbData;
    unsigned int thumbDataSize;
    unsigned int numTags;
    FileShareTag *tags;
    FileShareFileID outFileID;
    unsigned int (__cdecl *dataCallback)(void *, unsigned int, unsigned int);
    void (__cdecl *successCallback)(int, unsigned __int64);
    void (__cdecl *failureCallback)(int);
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct __declspec(align(8)) FileShareListTask // sizeof=0x28
{                                       // XREF: playerFileOperations/r
    unsigned __int64 ownerID;
    int startDate;
    int bufferLocation;
    unsigned __int16 category;
    unsigned __int16 maxNumResults;
    unsigned __int16 offset;
    // padding byte
    // padding byte
    char *fileName;
    FileShareMetaData *descriptors;
    const char *menuDef;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct FileShareSearchTask // sizeof=0x38
{                                       // XREF: playerFileOperations/r
    fileShareLocation location;
    unsigned int startIndex;
    unsigned int maxNumResults;
    unsigned int numTags;
    unsigned int sortOrder;
    bool orderNewestFirst;
    // padding byte
    // padding byte
    // padding byte
    FileShareTag *tags;
    int *resultFileCount;
    FileShareFileID *resultFileIDs;
    int *resultRatingCount;
    VoteRankStatsInfo *resultRatings;
    int *resultTotalFileCount;
    void (__cdecl *successCallback)();
    void (__cdecl *failureCallback)();
};

struct FileShareTagTask // sizeof=0x18
{                                       // XREF: playerFileOperations/r
    fileShareLocation location;
    FileShareTag *tags;
    unsigned int numTags;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    unsigned __int64 entityID;
};

struct FileShareTransferTask // sizeof=0x138
{                                       // XREF: playerFileOperations/r
    unsigned __int64 fileID;
    fileShareLocation location;
    unsigned __int16 fileSlot;
    // padding byte
    // padding byte
    char *fileName;
    unsigned __int16 category;
    char metaData[255];
    // padding byte
    // padding byte
    // padding byte
    unsigned int metaDataSize;
    unsigned int numTags;
    FileShareTag *tags;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    FileShareFileID outFileID;
};

struct FileShareRemoveTask // sizeof=0x4
{                                       // XREF: playerFileOperations/r
    unsigned int fileSlot;
};

struct FileShareSummaryTask // sizeof=0x30
{                                       // XREF: playerFileOperations/r
    unsigned __int64 fileID;
    void *summaryData;
    unsigned int summaryDataSize;
    const void *metaData;
    unsigned int metaDataSize;
    unsigned int numTags;
    FileShareTag *tags;
    fileShareLocation location;
    bool showSuccess;
    // padding byte
    // padding byte
    // padding byte
    void (__cdecl *successCallback)();
    void (__cdecl *failureCallback)();
};

struct __declspec(align(8)) FileRatingInfo : TaskResult // sizeof=0x18
{                                       // XREF: FileShareSubmitRatingTask/r
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    unsigned __int64 m_entityID;
    unsigned __int8 m_rating;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct __declspec(align(8)) FileShareSubmitRatingTask // sizeof=0x20
{                                       // XREF: playerFileOperations/r
    FileRatingInfo rating[1];
    unsigned int numRatings;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct __declspec(align(8)) FileShareGetTopRatedTask // sizeof=0x4E48
{                                       // XREF: playerFileOperations/r
    unsigned __int64 startRank;
    int maxFiles;
    unsigned int leaderboardID;
    VoteRankStatsInfo statsInfo[100];
    int *outNumFiles;
    int *outTotalNumFiles;
    FileShareFileID *outFileIDs;
    void (__cdecl *successCallback)();
    void (__cdecl *failureCallback)();
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct playerFileOperations // sizeof=0x6F58
{                                       // XREF: .data:controllerFileOps/r
    ~playerFileOperations() = default;
    playerFileOperations() = default;

    FileOperationInfo statsBackupFileInfo;
    FileOperationInfo readEmailFileInfo;
    FileOperationInfo saveEmailFileInfo;
    FileOperationInfo squadMessagesFileInfo;
    FileOperationInfo codMessagesFileInfo;
    FileOperationInfo eventMessagesFileInfo;
    FileOperationInfo hiddenOfferIdsFileInfo;
    FileOperationInfo fbStatusFileInfo;
    FileOperationInfo clanMOTDInfo;
    FileOperationInfo corruptedStatsBackupFileInfo;
                                        // XREF: LiveStorage_ResetAllFileOps+29/o
    FileOperationInfo fetchMetPlayerListFileInfo;
                                        // XREF: LiveStorage_ResetAllFileOps+41/o
    FileOperationInfo saveMetPlayerListFileInfo;
                                        // XREF: LiveStorage_ResetAllFileOps+A0/o
    FileOperationInfo getOtherPlayerStatsFileInfo;
                                        // XREF: LiveStorage_ResetAllFileOps+59/o
    FileOperationInfo getPlayerStatsFileInfo;
                                        // XREF: LiveStorage_ResetAllFileOps+70/o
    FileOperationInfo getBasicTrainingStatsFileInfo;
                                        // XREF: LiveStorage_ResetAllFileOps+88/o
    FileOperationInfo saveStatsFileInfo;
                                        // XREF: LiveStorage_ResetAllFileOps+B7/o
    FileOperationInfo saveBasicTrainingStatsFileInfo;
    FileOperationInfo contractsFileInfo;
    FileOperationInfo demoFileInfo;
    FileShareListTask fileShareListTask;
    FileShareSearchTask fileShareSearchTask;
    FileShareDescriptorsTask fileShareDescriptorsTask;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    FileShareReadFileTask fileShareReadFileTask;
    FileShareWriteFileTask fileShareWriteFileTask;
    FileShareTagTask fileShareTagTask;
    FileShareTransferTask fileShareTransferTask;
    FileShareRemoveTask fileShareRemoveTask;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    FileShareSummaryTask fileShareSummaryTask;
    FileShareSubmitRatingTask fileShareSubmitRatingTask;
    FileShareGetRatingTask fileShareGetRatingTask;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    FileShareGetTopRatedTask fileShareGetTopRatedTask;
    FileShareGetRatingTask fileShareFilmRatingTask;
    FileShareGetRatingTask fileShareClipRatingTask;
    FileShareGetRatingTask fileShareScreenshotRatingTask;
    FileShareGetRatingTask fileShareCustomGameRatingTask;
};

struct fileShareWriteFileInfo // sizeof=0x34
{                                       // XREF: ?Demo_SaveScreenshot@@YAXHI@Z/r
                                        // ?Demo_WriteRecordedClip@@YAXHH_N@Z/r ...
    unsigned __int16 fileSlot;          // XREF: UI_Gametype_Custom_UploadToFileShare(int,uint,char const *,MemoryFile &)+39/w
                                        // Demo_StartRecord_f(void)+16E/w ...
    // padding byte
    // padding byte
    void *fileData;                     // XREF: UI_Gametype_Custom_UploadToFileShare(int,uint,char const *,MemoryFile &)+22/w
                                        // UI_Gametype_Custom_UploadToFileShare(int,uint,char const *,MemoryFile &)+3D/r ...
    unsigned int fileSize;              // XREF: UI_Gametype_Custom_UploadToFileShare(int,uint,char const *,MemoryFile &)+2B/w
                                        // Demo_StartRecord_f(void)+180/w ...
    const char *fileName;               // XREF: UI_Gametype_Custom_UploadToFileShare(int,uint,char const *,MemoryFile &)+57/w
                                        // Demo_StartRecord_f(void)+151/w ...
    unsigned __int16 category;          // XREF: UI_Gametype_Custom_UploadToFileShare(int,uint,char const *,MemoryFile &)+19/w
                                        // Demo_StartRecord_f(void)+13B/w ...
    // padding byte
    // padding byte
    const void *thumbData;              // XREF: UI_Gametype_Custom_UploadToFileShare(int,uint,char const *,MemoryFile &)+5A/w
                                        // Demo_StartRecord_f(void)+172/w ...
    unsigned int thumbDataSize;         // XREF: UI_Gametype_Custom_UploadToFileShare(int,uint,char const *,MemoryFile &)+61/w
                                        // Demo_StartRecord_f(void)+179/w ...
    unsigned int numTags;               // XREF: UI_Gametype_Custom_UploadToFileShare(int,uint,char const *,MemoryFile &)+46/w
                                        // Demo_StartRecord_f(void)+154/w ...
    fileShareLocation location;         // XREF: UI_Gametype_Custom_UploadToFileShare(int,uint,char const *,MemoryFile &)+2E/w
                                        // Demo_StartRecord_f(void)+162/w ...
    FileShareTag *tags;                        // XREF: UI_Gametype_Custom_UploadToFileShare(int,uint,char const *,MemoryFile &)+4D/w
                                        // Demo_StartRecord_f(void)+15B/w ...
    unsigned int (__cdecl *dataCallback)(void *, unsigned int, unsigned int);
                                        // XREF: UI_Gametype_Custom_UploadToFileShare(int,uint,char const *,MemoryFile &)+68/w
                                        // Demo_StartRecord_f(void)+187/w ...
    void (__cdecl *successCallback)(int, unsigned __int64);
                                        // XREF: UI_Gametype_Custom_UploadToFileShare(int,uint,char const *,MemoryFile &)+6F/w
                                        // Demo_StartRecord_f(void)+18E/w ...
    void (__cdecl *failureCallback)(int);
                                        // XREF: UI_Gametype_Custom_UploadToFileShare(int,uint,char const *,MemoryFile &)+76/w
                                        // Demo_StartRecord_f(void)+195/w ...
};

persistentStats *__cdecl LiveStorage_GetStatsBuffer(
                int controllerIndex,
                statsLocation playerStatsLocation,
                bool verifyLocation);
bool __cdecl LiveStorage_GetStatsWriteNeeded(int controllerIndex, statsLocation location);
void __cdecl LiveStorage_SetStatsWriteNeeded(int controllerIndex, bool isWriteNeeded, statsLocation location);
bool __cdecl LiveStorage_DoWeHaveAllStats(int controllerIndex);
fileSharePrivateData *__cdecl LiveStorage_FileShare_GetFileShareData(fileShareBufferLocation bufferLocation);
bool __cdecl LiveStorage_IsTimeSynced();
void __cdecl LiveStorage_FileShare_GetDownloadProgress(int controllerIndex, unsigned int *bytes, float *dataRate);
void __cdecl LiveStorage_FileShare_GetUploadProgress(int controllerIndex, unsigned int *bytes, float *dataRate);
void __cdecl LiveStorage_FileShare_AbortOperation(int controllerIndex, fileShareLocation location);
unsigned int __cdecl LiveStorage_FileShare_GetDownloadingFileTotalSize(int controllerIndex);
unsigned int __cdecl LiveStorage_FileShare_GetUploadingFileTotalSize(int controllerIndex);
TaskRecord *__cdecl LiveStorage_FileShare_ReadFile(int controllerIndex, fileShareReadFileInfo *fileInfo);
TaskRecord *__cdecl LiveStorage_FileShare_WriteFile(int controllerIndex, fileShareWriteFileInfo *writeFileInfo);
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
                bool showSuccess);
int __cdecl LiveContracts_GetTimeUntilNextCycle();
unsigned int __cdecl LiveContracts_GetContractsCountForList();
int __cdecl LiveContracts_GetContractIndexFromListIndex(int controllerIndex, int listIndex);
char *__cdecl LiveContracts_GetContractName(int index);
char *__cdecl LiveContracts_GetContractDesc(int index);
char *__cdecl LiveContracts_GetContractStatType(int index);
char *__cdecl LiveContracts_GetContractStatName(int index);
int __cdecl LiveContracts_GetContractCost(int index);
char *__cdecl LiveContracts_GetRequirementType(int contractIndex, unsigned int requirementIndex);
char *__cdecl LiveContracts_GetRequirementData(int contractIndex, unsigned int requirementIndex);
char *__cdecl LiveContracts_GetResetConditions(int contractIndex);
int __cdecl LiveContracts_GetContractRewardXP(int contractIndex);
int __cdecl LiveContracts_GetContractRewardCP(int contractIndex);
int __cdecl LiveContracts_GetContractRequiredCount(int index);
char *__cdecl LiveContracts_GetExpirationType(int index);
int __cdecl LiveContracts_GetExpirationData(int index);
int __cdecl LiveContracts_GetTimesPurchasable(int contractIndex);
int __cdecl LiveContracts_GetCooldownTime(int contractIndex);
int __cdecl LiveContracts_GetUnlockLevel(int contractIndex);
bool __cdecl LiveContracts_IsContractLocked(int controllerIndex, int contractIndex);
unsigned int __cdecl LiveContracts_GetTimesPurchased(int controllerIndex, int contractIndex);
int __cdecl LiveContracts_GetTimeUntilCooledDown(int controllerIndex, int contractIndex);
int __cdecl LiveContracts_GetNumActiveContracts(int controllerIndex);
int __cdecl LiveContracts_GetNumContractsInProgress(int controllerIndex);
int __cdecl LiveContracts_GetActiveContractIndex(int controllerIndex, int contractIndex);
unsigned int __cdecl LiveContracts_GetIndexForActiveContract(int controllerIndex, unsigned int activeContractIndex);
int __cdecl LiveContracts_GetActiveContractProgress(int controllerIndex, unsigned int activeContractIndex);
int __cdecl LiveContracts_GetCombatTimeLeft(int controllerIndex, unsigned int activeContractIndex);
int __cdecl LiveContracts_GetActiveContractStatus(int controllerIndex, unsigned int activeContractIndex);
unsigned int __cdecl LiveContracts_SV_GetIndexForActiveContract(
                unsigned int clientNum,
                unsigned int activeContractIndex);
int __cdecl LiveContracts_SV_GetActiveContractProgress(unsigned int clientNum, unsigned int activeContractIndex);
void __cdecl LiveContracts_SV_IncrementActiveContractProgress(
                unsigned int clientNum,
                unsigned int activeContractIndex,
                int increment);
void __cdecl LiveContracts_SV_ResetActiveContractProgress(unsigned int clientNum, unsigned int activeContractIndex);
void __cdecl LiveContracts_SV_IncrementActiveContractTime(
                unsigned int clientNum,
                unsigned int activeContractIndex,
                int increment);
int __cdecl LiveContracts_SV_GetActiveContractStatus(unsigned int clientNum, unsigned int activeContractIndex);
int __cdecl LiveContracts_SV_GetCombatTimePassed(unsigned int clientNum, unsigned int activeContractIndex);
void __cdecl LiveCounter_IncrementCounterValueByName(const char *counterType, unsigned __int64 increment);
void __cdecl LiveCounter_IncrementCounterValue(int counterID, __int64 increment);
int __cdecl LiveCounter_GetCounterTotalValue(int counterID);
int __cdecl LiveCounter_CounterStringToID(const char *counterType);

enum fileShareKeyIndex : __int32
{                                       // XREF: ?Live_FileShare_GetItemInfo@@YA_NHW4fileShareKeyIndex@@HW4fileShareInfoLocation@@PAPBDPAM@Z/r
                                        // ?Live_FileShare_GetMySlotInfo@@YA_NHHPBDPAPBDPAM@Z/r ...
    FILESHARE_KEY_SLOT               = 0x0,
    FILESHARE_KEY_INGAMESLOT         = 0x1,
    FILESHARE_KEY_SLOTOCCUPIED       = 0x2,
    FILESHARE_KEY_DATETIME           = 0x3,
    FILESHARE_KEY_LENGTH             = 0x4,
    FILESHARE_KEY_SIZE               = 0x5,
    FILESHARE_KEY_AUTHOR             = 0x6,
    FILESHARE_KEY_MAP                = 0x7,
    FILESHARE_KEY_MAPNAME            = 0x8,
    FILESHARE_KEY_GAMETYPE           = 0x9,
    FILESHARE_KEY_GAMETYPENAME       = 0xA,
    FILESHARE_KEY_FILEID             = 0xB,
    FILESHARE_KEY_FILESIZE           = 0xC,
    FILESHARE_KEY_FILETYPE           = 0xD,
    FILESHARE_KEY_FILENAME           = 0xE,
    FILESHARE_KEY_HASSUMMARY         = 0xF,
    FILESHARE_KEY_GAMETYPEANDMAPNAME = 0x10,
    FILESHARE_KEY_RATING             = 0x11,
    FILESHARE_KEY_TYPEICONNAME       = 0x12,
    FILESHARE_KEY_SELECTEDIMAGENAME  = 0x13,
    FILESHARE_KEY_NAME               = 0x14,
    FILESHARE_KEY_ISMODIFIEDNAME     = 0x15,
    FILESHARE_KEY_DESCRIPTION        = 0x16,
    FILESHARE_KEY_ISMODIFIEDDESCRIPTION = 0x17,
    FILESHARE_KEY_USERTAG            = 0x18,
    FILESHARE_KEY_USERTAGINDEX       = 0x19,
    FILESHARE_KEY_GAMETYPEIMAGE      = 0x1A,
    FILESHARE_KEY_TOTALVOTES         = 0x1B,
    FILESHARE_KEY_ISSUMMARYCACHED    = 0x1C,
    FILESHARE_KEY_COUNT              = 0x1D,
};

enum fileShareInfoLocation : __int32
{                                       // XREF: Live_FileShare_GetDate/r
                                        // Live_FileShare_GetLength/r ...
    FILESHARE_INFOLOCATION_INVALID = 0x0,
    FILESHARE_INFOLOCATION_SEARCHRESULTS = 0x1,
    FILESHARE_INFOLOCATION_MYFILESHARE = 0x2,
    FILESHARE_INFOLOCATION_MYFILESHARE_INGAME = 0x3,
    FILESHARE_INFOLOCATION_OTHERFILESHARE = 0x4,
};

enum fileSharePooledFileContext_t : __int32
{                                       // XREF: fileSharePooledDetails_t/r
                                        // ?Live_FileShare_SetPooledFileDetailsContext@@YAXW4fileSharePooledFileContext_t@@@Z/r ...
    FILESHARE_POOLEDFILEDETAILS_INVALID = 0x0,
    FILESHARE_POOLEDFILEDETAILS_SELF = 0x1,
    FILESHARE_POOLEDFILEDETAILS_FRIEND = 0x2,
};

struct __declspec(align(8)) ratingHistory_t // sizeof=0xC88
{                                       // XREF: .data:s_fileShareRatingHistory/r
    _BYTE files[3200];                  // XREF: Live_FileShare_SaveRating(int,unsigned __int64,uchar)+43/w
                                        // Live_FileShare_IsRated(int,unsigned __int64,uchar *)+64/r
    int next;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct listBoxDef_s;
struct itemDef_s;
struct rectDef_s;
char __cdecl Live_FileShare_WritePublicMetaDataCreateTime(char *buffer, unsigned int createTime);
char __cdecl Live_FileShare_WritePublicMetaDataLength(char *buffer, unsigned int duration);
char __cdecl Live_FileShare_WritePublicMetaDataTags(char *buffer, FileShareTag *tags, int numTags);
void __cdecl Live_FileShare_AddTag(
                unsigned __int64 priTag,
                unsigned __int64 secTag,
                int *numTags,
                FileShareTag *tags,
                int maxNumTags);
char *__cdecl Live_FileShare_GetLocalizedUserTagFromIndex(int index);
int __cdecl Live_FileShare_GetIndexFromGameType(const char *gameType);
char *__cdecl Live_FileShare_GetFilterList();
char __cdecl Live_FileShare_IsRated(int controllerIndex, unsigned __int64 fileID, unsigned __int8 *rating);
char __cdecl Live_FileShare_Theater_GetType(int *type);
char __cdecl Live_FileShare_Theater_GetName(char **name);
char __cdecl Live_FileShare_Theater_GetDescription(char **description);
int __cdecl Live_FileShare_GetSearchResultsCount();
char *__cdecl Live_FileShare_SearchResultsItemText(
                int controllerIndex,
                int startIndex,
                int selectedIndex,
                int itemIndex,
                bool *useOwnerDraw);
int __cdecl Live_FileShare_GetMyPrivateSlotsCount();
int __cdecl Live_FileShare_GetOtherPrivateSlotsCount();
char *__cdecl Live_FileShare_PrivateSlotsText(
                int controllerIndex,
                int feederID,
                unsigned int itemIndex,
                int column,
                Material **handle);
const char *__cdecl Live_FileShare_PrivateSlotsIngameText(
                int controllerIndex,
                int itemIndex,
                int column,
                Material **handle);
void __cdecl Live_FileShare_FeederColor(
                listBoxDef_s *listPtr,
                itemDef_s *item,
                int contextIndex,
                unsigned int index,
                int column,
                fileShareBufferLocation bufferLocation,
                DvarValue *color);
double __cdecl Live_FileShare_LookupTotalVotes(unsigned __int64 fileID, VoteRankStatsInfo *ratings, int numRatings);
const char *__cdecl Live_FileShare_GetPooledFileInfo(const char *field);
char __cdecl Live_FileShare_GetItemInfo(
                int controllerIndex,
                fileShareKeyIndex keyIndex,
                int index,
                fileShareInfoLocation location,
                const char **stringResult,
                float *floatResult);
char __cdecl Live_FileShare_GetMySlotInfo(
                int controllerIndex,
                int slotNum,
                const char *field,
                const char **stringResult,
                float *floatResult);
char __cdecl Live_FileShare_GetFeederData(
                int controllerIndex,
                int feederID,
                int index,
                const char *field,
                const char **stringResult,
                float *floatResult);
void __cdecl Live_FileShare_RatingOwnerdraw(
                int contextIndex,
                const rectDef_s *rect,
                const itemDef_s *item,
                const float *color);
void __cdecl Live_FileShare_PrivateRatingOwnerdraw(
                int contextIndex,
                const rectDef_s *rect,
                const itemDef_s *item,
                const float *color);
void __cdecl Live_FileShare_SubmitRatingOwnerdraw(
                int contextIndex,
                const rectDef_s *rect,
                const itemDef_s *item,
                const float *color);
void __cdecl Live_FileShare_MyRatingOwnerdraw(
                int localClientNum,
                int contextIndex,
                const rectDef_s *rect,
                const itemDef_s *item,
                const float *color);
void __cdecl Live_FileShare_AvgSubmitRatingOwnerdraw(
                int contextIndex,
                const rectDef_s *rect,
                const itemDef_s *item,
                const float *color);
void __cdecl Live_FileShare_ClearSearchState();
extern const dvar_t *fsShowStreamingGraph;
extern const dvar_t *fsPrivateSlotCol;
extern const dvar_t *fsOtherUserPrivateSlotCol;
extern const dvar_t *fsSelectedFileID;
extern const dvar_t *fsSelectedFileName;
extern const dvar_t *fsIsSelectedFileNameModified;
extern const dvar_t *fsSelectedFileDescription;
extern const dvar_t *fsIsSelectedFileDescriptionModified;
extern const dvar_t *fsSelectedFileTagIndex;
extern const dvar_t *fshSelectLastSlotRow;
extern const dvar_t *fshSelectFirstSlotRow;

struct __declspec(align(8)) fshCacheFileDetails_t // sizeof=0x840
{
    FileShareMetaData descriptor;
    fileShareLocation location;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct __declspec(align(8)) fshCacheRatingDetails_t // sizeof=0xD8
{
    unsigned __int64 fileID;
    VoteRankStatsInfo rating;
    fileShareLocation location;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};
void __cdecl Live_FileShare_CacheShutdown();

struct listBoxDef_s;
struct itemDef_s;

struct __declspec(align(8)) fshSearchContext_t // sizeof=0x3E0
{                                       // XREF: fshSearch_t/r
    FileShareTag tags[40];
    fileShareLocation location;         // XREF: Live_FileShareSearch_SetContext(int,FileShareTag * const,int,fileShareLocation,bool,int)+9A/w
                                        // Live_FileShareSearch_ClearSearch+7F/w ...
    int numTags;                        // XREF: Live_FileShareSearch_SetContext(int,FileShareTag * const,int,fileShareLocation,bool,int)+92/w
                                        // Live_FileShareSearch_ClearSearch+90/w ...
    int timestamp;                      // XREF: Live_FileShareSearch_SetContext(int,FileShareTag * const,int,fileShareLocation,bool,int)+8A/w
                                        // Live_FileShareSearch_NeedToPerformSearch+67/r
    int fileType;                       // XREF: Live_FileShareSearch_SetContext(int,FileShareTag * const,int,fileShareLocation,bool,int)+B1/w
                                        // Live_FileShareSearch_NeedToPerformSearch+33/r ...
    bool sortHighestRated;              // XREF: Live_FileShareSearch_SetContext(int,FileShareTag * const,int,fileShareLocation,bool,int)+A3/w
                                        // Live_FileShareSearch_NeedToPerformSearch+41/r ...
    bool isValid;                       // XREF: Live_FileShareSearch_SetContext(int,FileShareTag * const,int,fileShareLocation,bool,int)+B7/w
                                        // Live_FileShareSearch_ClearSearch+89/w ...
    // padding byte
    // padding byte
    int maxPastDays;                    // XREF: Live_FileShareSearch_SetIndexOverride(int,int)+1E/r
                                        // Live_FileShareSearch_SetIndexOverride(int,int)+95/r ...
    int totalOverride;                  // XREF: Live_FileShareSearch_SetIndexOverride(int,int)+27/w
                                        // Live_FileShareSearch_SetIndexOverride(int,int)+BE/r ...
    // padding byte
    // padding byte
    // padding byte
    // padding byte

    //~fshSearchContext_t();
};
void __cdecl Live_FileShareSearch_GetRatings(VoteRankStatsInfo **ratings, int *numRatings);
char __cdecl Live_FileShareSearch_FeederItemEnabled(int index);
char __cdecl Live_FileShareSearch_FeederColor(
                listBoxDef_s *listPtr,
                itemDef_s *item,
                int contextIndex,
                int index,
                int column,
                DvarValue *color);

enum LiveSteam_PersonaState : __int32
{                                       // XREF: ?LiveSteam_GetFriendPersonaState@@YA?AW4LiveSteam_PersonaState@@_K@Z/r
    LIVE_STEAM_PERSONA_STATE_OFFLINE = 0x0,
    LIVE_STEAM_PERSONA_STATE_ONLINE  = 0x1,
    LIVE_STEAM_PERSONA_STATE_BUSY    = 0x2,
    LIVE_STEAM_PERSONA_STATE_SNOOZE  = 0x3,
};
int __cdecl Friends_GetCount(int __formal, int type);
char __cdecl Friends_GetByIndex(int controllerIndex, int filter, int index, FriendInfo *info);
char __cdecl Friends_GetByID(int controllerIndex, unsigned __int64 id, FriendInfo *info);

//CSteamID *__thiscall CSteamID::CSteamID(CSteamID *this);
//bool __thiscall CSteamID::IsValid(CSteamID *this);

//int __cdecl _GetSortedFriendIndex(int index);
//int __cdecl _GetNumFriends<4>();
//int __cdecl _GetNumFriends<16>();
//unsigned __int64 __cdecl _GetFriendXuid<4>(int index);
//void __cdecl _GetFriendPersonaName<4>(int index, char *buf, int size);
//void __cdecl _GetFriendPersonaName<16>(int index, char *buf, int size);
//char __cdecl _GetFriendPersonaNameByXuid<4>(unsigned __int64 xuid, char *buf, int size);
//

enum servertype_t : __int32
{                                       // XREF: ?LiveGroups_GetCountByType@@YAIW4servertype_t@@@Z/r
    SERVER_GROUP_RANKED   = 0x1EA,
    SERVER_GROUP_UNRANKED = 0x1EB,
    SERVER_GROUP_WAGER    = 0x1EC,
};
unsigned int __cdecl LiveGroups_GetCount(char *path);
unsigned int __cdecl LiveGroups_GetCountByType(servertype_t type);

enum LbTaskEnum : __int32
{                                       // XREF: ?LB_ReadHiddenLeaderboardComplete@@YA?AW4taskCompleteResults@@H@Z/r
                                        // ?Lb_ReadHiddenLeaderboardEntry@@YAXHPAV?$LeaderBoardRow@$09@@HW4LbTaskEnum@@@Z/r
    LB_TASK_READLB                 = 0x0,
    LB_TASK_READ_WEEKLY_HIDDEN_LB  = 0x1,
    LB_TASK_READ_PRESTIGE_WEEKLY_HIDDEN_LB = 0x2,
    LB_TASK_READ_MONTHLY_HIDDEN_LB = 0x3,
    LB_TASK_READ_PRESTIGE_MONTHLY_HIDDEN_LB = 0x4,
    LB_TASK_GETFRIENDS             = 0x5,
    LB_TASK_AM_UPLOADSTATS         = 0x6,
    LB_TASK_UPLOADSTATS            = 0x7,
    LB_TASK_UPLOADSTATS_WEEKLY_LB  = 0x8,
    LB_TASK_UPLOADSTATS_PRESTIGE_WEEKLY_LB = 0x9,
    LB_TASK_UPLOADSTATS_MONTHLY_LB = 0xA,
    LB_TASK_UPLOADSTATS_PRESTIGE_MONTHLY_LB = 0xB,
    LB_TASK_RESOLVEESCROW          = 0xC,
    LB_TASK_RETRIEVEESCROWBALANCE  = 0xD,
    LB_TASK_CLEARESCROW            = 0xE,
    LB_TASK_INCREMENTESCROW        = 0xF,
    LB_TASK_READLB_FOR_MATCHES_PLAYED = 0x10,
    LB_TASK_COUNT                  = 0x11,
};

enum LbStatEnum : __int32
{                                       // XREF: LbStructure/r
                                        // ?SV_MakeClientLBRow@@YA_NHQBULbPlayerStat@@HHQAV?$LeaderBoardRow@$09@@_N@Z/r ...
    LB_STAT_MATCHES_PLAYED    = 0x0,
    LB_STAT_SCORE             = 0x1,
    LB_STAT_TIME_PLAYED_TOTAL = 0x2,
    LB_STAT_SCORE_PER_MINUTE  = 0x3,
    LB_STAT_XP                = 0x4,
    LB_STAT_PRESTIGE_LEVEL    = 0x5,
    LB_STAT_KILLS             = 0x6,
    LB_STAT_DEATHS            = 0x7,
    LB_STAT_KDRATIO           = 0x8,
    LB_STAT_ASSISTS           = 0x9,
    LB_STAT_CAPTURES          = 0xA,
    LB_STAT_RETURNS           = 0xB,
    LB_STAT_DEFENDS           = 0xC,
    LB_STAT_PICKUPS           = 0xD,
    LB_STAT_PLANTS            = 0xE,
    LB_STAT_DEFUSES           = 0xF,
    LB_STAT_DESTRUCTIONS      = 0x10,
    LB_STAT_COUNT             = 0x11,
};

enum LbType : __int32
{                                       // XREF: Leaderboard/r LbGlob/r ...
    LB_TDM                  = 0x0,
    LB_GAMEMODE_START       = 0x0,
    LB_DM                   = 0x1,
    LB_CTF                  = 0x2,
    LB_DOM                  = 0x3,
    LB_SAB                  = 0x4,
    LB_SD                   = 0x5,
    LB_KOTH                 = 0x6,
    LB_DEM                  = 0x7,
    LB_BASIC_GAMEMODE_COUNT = 0x8,
    LB_HCTDM                = 0x8,
    LB_HARDCORE_TYPES_START = 0x8,
    LB_HCDM                 = 0x9,
    LB_HCCTF                = 0xA,
    LB_HCDOM                = 0xB,
    LB_HCSAB                = 0xC,
    LB_HCSD                 = 0xD,
    LB_HCKOTH               = 0xE,
    LB_HCDEM                = 0xF,
    LB_COUNT                = 0x10,
    LB_GAMEMODE_COUNT       = 0x10,
};

enum LbResetPeriod : __int32
{                                       // XREF: ?LB_GetLbIndex@@YAHW4LbType@@W4LbResetPeriod@@_N2@Z/r
                                        // ?LB_ReadHiddenLeaderboardComplete@@YA?AW4taskCompleteResults@@H@Z/r
    LB_RESETPERIOD_ALL_TIME = 0x0,
    LB_RESETPERIOD_WEEKLY   = 0x1,
    LB_RESETPERIOD_MONTHLY  = 0x2,
    LB_RESETPERIOD_COUNT    = 0x3,
};

enum GlobalLbStatEnum : __int32
{                                       // XREF: LbGlobalStructure/r
                                        // ?SV_MakeClientGlobalLBRow@@YA_NHQBULbPlayerStat@@HQAV?$LeaderBoardRow@$09@@@Z/r ...
    GLOBAL_LB_STAT_SCORE_PER_MINUTE = 0x0,
    GLOBAL_LB_STAT_KILLS            = 0x1,
    GLOBAL_LB_STAT_DEATHS           = 0x2,
    GLOBAL_LB_STAT_KDRATIO          = 0x3,
    GLOBAL_LB_STAT_ASSISTS          = 0x4,
    GLOBAL_LB_STAT_XP               = 0x5,
    GLOBAL_LB_STAT_PRESTIGE_LEVEL   = 0x6,
    GLOBAL_LB_STAT_TIMEPLAYED       = 0x7,
    GLOBAL_LB_STAT_WINS             = 0x8,
    GLOBAL_LB_STAT_LOSSES           = 0x9,
    GLOBAL_LB_STAT_WLRATIO          = 0xA,
    GLOBAL_LB_STAT_HEADSHOTS        = 0xB,
    GLOBAL_LB_STAT_ACCURACY         = 0xC,
    GLOBAL_LB_STAT_CODPOINTS        = 0xD,
    GLOBAL_LB_STAT_CURRENCYEARNED   = 0xE,
    GLOBAL_LB_STAT_CURRENCYSPENT    = 0xF,
    GLOBAL_LB_STAT_SCORE            = 0x10,
    GLOBAL_LB_STAT_WIN_STREAK       = 0x11,
    GLOBAL_LB_STAT_KILL_STREAK      = 0x12,
    GLOBAL_LB_STAT_COUNT            = 0x13,
};

template <int SIZE>
struct LeaderBoardRow : LbStatsInfo // sizeof=0x98
{                                       // XREF: LeaderBoardResult<LeaderBoardRow<10>,100>/r
    int m_columns[SIZE];                  // XREF: LB_IncrementEscrow(void)+82/o
};

template <typename T, int SIZE>
struct LeaderBoardResult : LbStatsInfo //<LeaderBoardRow<10>,100> : LbStatsInfo // sizeof=0x3BD0
{                                       // XREF: Leaderboard/r
                                        // LbUserStats/r
    LeaderBoardRow<10> m_leaderBoardEntries[SIZE];// 100];
                                        // XREF: .rdata:off_D0DB08/o

    //LeaderBoardResult();
    //~LeaderBoardResult();
};

struct __declspec(align(8)) Leaderboard // sizeof=0x3CA0
{                                       // XREF: LbGlob/r
    LeaderBoardResult<LeaderBoardRow<10>,100> userStats;
                                        // XREF: .rdata:off_D0DB08/o
    LbType type;                        // XREF: LB_Init(void)+17C/w
    int offset;                         // XREF: LB_FeederCount(int)+36/r
                                        // LB_FeederItemText(int,int,int,Material * *)+C5/r ...
    int curIdx;                         // XREF: LB_FeederSelection(int)+14/w
                                        // LB_Init(void)+19E/w
    bool forceRefresh;                  // XREF: LB_ForceRefresh(void)+11/w
                                        // LB_Init(void)+1B1/w
    bool isUpdating;                    // XREF: LB_FeederItemText(int,int,int,Material * *)+57/r
    bool notSameFilter;
    bool notSameResetPeriod;
    bool notSamePrestigeMode;
    // padding byte
    // padding byte
    // padding byte
    int lastUpdate;                     // XREF: LB_Init(void)+140/w
    int filter;                         // XREF: LB_UpdateLeaderboard(int,LbType,int)+297/r
                                        // LB_Init(void)+153/w
    int lbTypeByResetPeriod;            // XREF: LB_Init(void)+166/w
    bool prestigeMode;
    // padding byte
    // padding byte
    // padding byte
    int numResults;
    int totalResults;                   // XREF: LB_FeederItemText(int,int,int,Material * *)+75/r
                                        // LB_FeederItemText(int,int,int,Material * *)+86/r
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    LeaderBoardRow<10> playerLbRow;
    int numMatchesPlayed;
    // padding byte
    // padding byte
    // padding byte
    // padding byte

    Leaderboard() = default;
    ~Leaderboard() = default;
};

struct LBSelectedPlayerInfo // sizeof=0x48
{                                       // XREF: .data:info_2/r
    unsigned __int64 playerXuid;        // XREF: UI_FeederDataExtended(int,itemDef_s *,char const *,char const * *,float *)+22B/r
                                        // UI_FeederDataExtended(int,itemDef_s *,char const *,char const * *,float *)+232/r
    char gamerTag[64];                  // XREF: UI_FeederDataExtended(int,itemDef_s *,char const *,char const * *,float *)+268/o
};
void __cdecl LB_ForceRefresh();
int __cdecl LB_FeederCount(int localClientNum);
void __cdecl LB_FeederSelection(int index);
void __cdecl LB_OnSelect(int localClientNum);
void __cdecl LB_GetSelectedPlayerInfo(int localClientNum, LBSelectedPlayerInfo *info);
void __cdecl LB_OverrideCursorPos(int viewmax, int *cursorPos, int *startPos);
char __cdecl LB_UploadPlayerStats(int localControllerIndex);
char *__cdecl LB_FeederItemText(int localClientNum, unsigned int index, int column, Material **material);

struct XuidInfo // sizeof=0x30
{                                       // XREF: .data:recentPlayerInfo/r
    unsigned __int64 playerXuids;       // XREF: UI_FeederItemText_CommmonPlayerListHandler+CE/r
                                        // UI_FeederItemText_CommmonPlayerListHandler+D6/r ...
    char gamertag[32];                  // XREF: UI_FeederItemText_CommmonPlayerListHandler+DF/o
                                        // UI_FeederDataExtended_RecentPlayers+E9/o ...
    _FILETIME systemTime;
};

struct __declspec(align(8)) MetPlayerInfo // sizeof=0x970
{                                       // XREF: .data:metPlayersXuidinfo/r
    unsigned __int8 version;            // XREF: LiveMeetPlayer_ResetData(int)+2A/w
                                        // LiveMeetPlayer_CheckDownloadedVersionAndResetData(int)+13/r ...
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    XuidInfo playersXuidInfo[50];       // XREF: LiveMeetPlayer_CheckDownloadedVersionAndResetData(int)+88/r
                                        // LiveMeetPlayer_GetXUIDPosInMetPlayerList(int,unsigned __int64)+4B/r ...
    unsigned __int8 metPlayerCount;     // XREF: LiveMeetPlayer_CheckDownloadedVersionAndResetData(int)+61/r
                                        // LiveMeetPlayer_GetXUIDPosInMetPlayerList(int,unsigned __int64)+21/r ...
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};
int __cdecl LiveMeetPlayer_GetNoOfMetPlayers(int localControllerIndex);
void __cdecl LiveMeetPlayer_SortMetPlayers(int localControllerIndex, unsigned int noOfPlayers);
unsigned int __cdecl LiveMeetPlayer_GetLastMetTime(int localControllerIndex, int index);
XuidInfo *__cdecl LiveMeetPlayer_GetMetFriendInfo(XuidInfo *result, int localControllerIndex, int index);

enum streamName_t : __int32;

struct PCachePublicProfile;
struct CmdArgs;

struct __declspec(align(8)) PCacheComponent // sizeof=0x20
{                                       // XREF: PCachePublicProfile/r
                                        // PCachePlayerEmblem/r
    unsigned __int64 xuid;
    int controllerIndex;
    int type;
    unsigned int updateTime;
    unsigned int touchTime;
    unsigned int state;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct PCachePublicProfile // sizeof=0x120
{                                       // XREF: .data:s_publicProfiles/r
    PCacheComponent c;
    char ddl[256];
};

struct PCachePlayerEmblem // sizeof=0x1B0
{                                       // XREF: .data:s_playerEmblems/r
    PCacheComponent c;
    CompositeEmblemLayer layers[12];
    int jobID;
    GfxImage *image;
    __int16 backgroundID;
    // padding byte
    // padding byte
    Material *background;
};
void __cdecl PCache_Lock();
void __cdecl PCache_Unlock();
PCacheComponent *__cdecl PCache_GetComponent(int controllerIndex, unsigned __int64 xuid, unsigned int type);
bool __cdecl PCache_TouchComponent(PCacheComponent *component);
void __cdecl PCache_InvalidatePlayerEmblem(int controllerIndex, unsigned __int64 xuid);
GfxImage *__cdecl PCache_GetPlayerEmblem(int controllerIndex, unsigned __int64 xuid);
__int16 __cdecl PCache_GetPlayerEmblemBackgroundID(int controllerIndex, unsigned __int64 xuid);
void __cdecl PCache_NukeProfile(int controlleridx);
char __cdecl PCache_GetRank(int controllerIndex, unsigned __int64 xuid, int *rank, int *prestige);
char __cdecl PCache_GetCodpoints(int controllerIndex, unsigned __int64 xuid, int *codpoints);
void __cdecl PCache_GetProfileEmblem(
                PCachePublicProfile *profile,
                CompositeEmblemLayer *layers,
                int layerCount,
                __int16 *backgroundID);
void __cdecl PCache_SetProfileEmblem(
                int controllerIndex,
                const CompositeEmblemLayer *layers,
                int layerCount,
                __int16 backgroundID);

int __cdecl Live_GetMapCount();
void __cdecl Live_SetPlaylistNum(int controllerIndex, int playlistNum);
int __cdecl Live_GetMapIndex();
int __cdecl Live_GetMapSource(int mapIndex);

struct SessionData_s;
bool __cdecl Session_IsHost(SessionData_s *session, int clientNum);
int __cdecl Session_HostNum(SessionData_s *session);

enum taskCompleteResults : __int32;

struct XSESSION_INFO // sizeof=0x31
{                                       // XREF: SessionData_s/r
                                        // LaunchData/r ...
    XNKID sessionID;
    XNADDR hostAddress;
    XNKEY keyExchangeKey;
};

struct RegisteredUser // sizeof=0x28
{                                       // XREF: SessionData_s/r
    bool active;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    unsigned __int64 xuid;
    netadr_t addr;
    bool privateSlot;
    // padding byte
    // padding byte
    // padding byte
    int performanceValue;
};

struct __declspec(align(4)) RemoteTalkerInfo // sizeof=0x20
{                                       // XREF: SessionData_s/r
    bool registered;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    unsigned __int64 xuid;
    int natType;
    int voiceConnectivityBits;
    int lastConnectivityTestTime[1];
    bool muted;
    // padding byte
    // padding byte
    // padding byte
};

struct LocalTalkerInfo // sizeof=0x1
{                                       // XREF: SessionData_s/r
    bool enabled;
};

struct __declspec(align(8)) SessionData_s // sizeof=0x978
{                                       // XREF: .data:SessionData_s g_serverSession/r
                                        // .data:SessionData_s g_partySession/r ...
    char *sessionName;                  // XREF: Session_Init(void)+38/w
    bool sessionHandle;                 // XREF: SV_SpawnServer(int,char const *,int,int)+690/r
                                        // Live_IsInLiveGame(void)+4/r
    XSESSION_INFO sessionInfo;          // XREF: _dynamic_initializer_for__g_serverSession__+4/o
                                        // _dynamic_initializer_for__g_partySession__+4/o ...
    bool keysGenerated;
    bool sessionStartCalled;
    bool registerUsersWithVoice;        // XREF: Session_Init(void)+42/w
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    unsigned __int64 sessionNonce;
    int privateSlots;                   // XREF: Live_Frame_MP(int,int)+1FB/r
    int publicSlots;                    // XREF: Live_Frame_MP(int,int)+202/r
    int flags;
    struct PS3VoiceEngine *voiceEngine;
    bool qosListenEnabled;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    RegisteredUser registeredUsers[32];
    RemoteTalkerInfo remoteTalkers[32];
    LocalTalkerInfo localTalkers[4];
    int voiceConnectivityBits;
    int sessionCreateController;
    int ackedMeetBits;
    int lastMeetSendTime;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct __declspec(align(8)) SessionJoinData // sizeof=0x20
{                                       // XREF: .data:sessionJoinData/r
    bool active;
    // padding byte
    // padding byte
    // padding byte
    int privateSlot;
    int slot;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    unsigned __int64 player;
    SessionData_s *session;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};
int __cdecl Session_FindRegisteredUser(const SessionData_s *session, unsigned __int64 xuid);
void __cdecl Session_DeleteSession(SessionData_s *session);
void __cdecl Session_StartHost(SessionData_s *session, int sessionFlags, int numPrivateSlots, int numPublicSlots);
void __cdecl Session_Init();

extern SessionData_s g_serverSession;

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
    CHALLENGE_PARAM_INVALID      = 0xFFFFFFFF,
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
    SORTORDER_INVALID             = 0xFFFFFFFF,
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
unsigned int __cdecl LiveStats_GetDDLIntStat(int controllerIndex, ddlState_t *searchState);
unsigned int __cdecl LiveStats_GetDDLIntStat(int controllerIndex, const char **operands, int nOperands);
unsigned int __cdecl LiveStats_GetDIntStat(int controllerIndex, ddlState_t *searchState);
unsigned int __cdecl LiveStats_GetDIntStatFromBase(
                int controllerIndex,
                ddlState_t *searchState,
                persistentStats *buffer);
char __cdecl LiveStats_GetIntPlayerStatFromBase(int *outInt, const char *statName, char *base);
unsigned int __cdecl LiveStats_GetDIntStatFromForcedBase(int controllerIndex, ddlState_t *searchState, char *buffer);
unsigned int __cdecl LiveStats_GetDInt64StatFromForcedBase(int controllerIndex, ddlState_t *searchState, char *buffer);
char *__cdecl LiveStats_GetDStringStatFromForcedBase(int controllerIndex, ddlState_t *searchState, char *buffer);
char __cdecl LiveStats_GetIntPlayerStat(int controllerIndex, int *outInt, const char *statName);
char __cdecl LiveStats_GetIntPlayerStatByGameMode(
                int controllerIndex,
                int *outInt,
                const char *gameMode,
                const char *statName);
char __cdecl LiveStats_GetIntOtherPlayerStatByGameMode(
                int controllerIndex,
                int *outInt,
                const char *gameMode,
                const char *statName);
char __cdecl LiveStats_GetIntPlayerStatByKey(int controllerIndex, int *outInt, playerStatsKeyIndex_t keyIndex);
char __cdecl LiveStats_GetIntOtherPlayerStatByKey(int controllerIndex, int *outInt, playerStatsKeyIndex_t keyIndex);
const char *__cdecl LiveStats_GetPlayerStatStringByKey(playerStatsKeyIndex_t keyIndex);
void __cdecl LiveStats_SetStatChanged(int controllerIndex, const char *hexMsg);
void __cdecl LiveStats_MakeStableStatsBuffer(int controllerIndex);
bool __cdecl LiveStats_IsStableStatsBufferInitialized(int controllerIndex);
void __cdecl LiveStats_MakeStableGlobalStatsBuffer(int controllerIndex);
int __cdecl LiveStats_GetTotalMatchesPlayed(int controllerIndex);
int __cdecl LiveStats_GetTotalMatchesPlayedByGameMode(int controllerIndex, const char *gameModePrefix);
unsigned int __cdecl LiveStats_GetNumStatsMilestones(int controllerIndex);
unsigned int __cdecl LiveStats_GetStatsMilestoneTier(int controllerIndex, unsigned int statsMilestoneNum);
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
char *__cdecl LiveStats_GetPersonalBestPrefix(int controllerIndex, unsigned int personalBestNum);
int __cdecl LiveStats_GetPersonalBestValue(int controllerIndex, unsigned int personalBestNum);
int __cdecl LiveStats_GetPersonalBestDelta(int controllerIndex, unsigned int personalBestNum);
int __cdecl LiveStats_GetPersonalBestRating(int controllerIndex, unsigned int personalBestNum);
unsigned int __cdecl LiveStats_GetNumRecentlyUnlockedItems(int controllerIndex);
int __cdecl LiveStats_GetRecentlyUnlockedItemIndex(int controllerIndex, unsigned int itemNum);
char __cdecl LiveStats_MoveToProItemVersion(ddlState_t *searchState, int itemIndex, int challengeIndex);
char __cdecl LiveStats_GetIntPlayerStatByKey(
                int controllerIndex,
                int *outInt,
                statsLocation location,
                playerStatsKeyIndex_t statKey);
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
bool __cdecl LiveStats_WritePrestigeToStats(int controllerIndex);
bool __cdecl LiveStats_GetBasicTrainingState(unsigned __int8 *buffer);
void __cdecl LiveStats_GetNemesisXuid(int controllerIndex, unsigned __int64 *nemesisXuid);
void __cdecl SV_UpdatePersonalBestsForClient(int clientnum);
extern const dvar_t *debugStats;
extern const dvar_t *ui_numPersonalBests;
extern const dvar_t *ui_challengeGameMode;
void __cdecl LiveSteam_CheckAccess();
const char *__cdecl LiveSteam_GetCountryCode();
unsigned __int64 __cdecl LiveSteam_GetUid();
bool __cdecl LiveSteam_LaunchOtherApp(const char *cmd);
void __cdecl LiveSteam_Init();
void __cdecl LiveSteam_Shutdown();
void __cdecl LiveSteam_Frame();
int __cdecl LiveSteam_Client_ConnectToSteamServer(unsigned __int64 serverID, void *authBlob, unsigned int bufferSize);
void LiveSteam_Client_SteamDisconnect();

extern const dvar_t *livesteam_sv_vac;

void __cdecl LiveSteam_Server_Init();
void __cdecl LiveSteam_Server_Shutdown();
void __cdecl LiveSteam_Server_RunCallbacks();
void __cdecl LiveSteam_Server_ClientSteamAuthentication(
                unsigned __int64 clientSteamID,
                netadr_t netAddr,
                void *authBlob,
                unsigned int authBlobSize);
void __cdecl LiveSteam_Server_ClientSteamDisconnect(unsigned __int64 clientSteamID);

bool __cdecl LiveStorage_DoWeHaveContracts();
bool __cdecl LiveStorage_DoWeHavePlaylists();
bool __cdecl LiveStorage_FetchingOnlineWAD();

struct FileOperationInfo;

enum blobtype_t : __int32
{                                       // XREF: ?Live_CACValidate_DispatchMessage@@YA_N_KPAXI@Z/r
    BLOB_TYPE_CAC     = 0x0,
    BLOB_TYPE_GLOBAL  = 0x1,
    BLOB_TYPE_INVALID = 0x2,
};

enum sv_cacvalidate_state_t : __int32
{
    CAC_IDLE         = 0x0,
    CAC_WAITING      = 0x1,
    CAC_FETCHONE     = 0x2,
    CAC_FETCHTWO     = 0x3,
    CAC_WAITINGAGAIN = 0x4,
    CAC_WRITEONE     = 0x5,
    CAC_WRITETWO     = 0x6,
};

enum day_t : __int32
{                                       // XREF: ?SV_DoTimedStatsForClient@@YAXHPBDPAV?$LeaderBoardRow@$09@@H1H@Z/r
                                        // ?SV_DoTimedStatsForClient@@YAXHPBDPAV?$LeaderBoardRow@$09@@H1H@Z/r
    SUNDAY = 0x0,
    MONDAY = 0x1,
    TUESDAY = 0x2,
    WEDNESDAY = 0x3,
    THURSDAY = 0x4,
    FRIDAY = 0x5,
    SATURDAY = 0x6,
};
void __cdecl LiveStorage_ReadStatsIfDirChanged();
void __cdecl LiveStorage_UploadStats();
void __cdecl LiveStorage_UploadStatsForController();
void __cdecl LiveStorage_FetchPlaylistsLocal(int controllerIndex);
int __cdecl LiveStorage_GetMatchesPlayed(const char *gameModePrefix, int resetPeriod);
void SV_CommitClientLeaderboards();
void __cdecl SV_OnlineReadClientStats(client_t *client);
void __cdecl SV_OnlineReadClientCAC(client_t *client);
void __cdecl SV_OnlineWriteClientStats(client_t *client);
char __cdecl SV_CACValidate_SetIntStat(unsigned __int8 *buffer, const char *stat, unsigned int value);
void __cdecl SV_SetShouldMapRotate(bool should);
void __cdecl SV_FetchWADDeferred();
bool __cdecl LiveStorage_FirstTimeRunning();
void __cdecl LiveStorage_SetFirstTimeRunning(bool running);
void __cdecl LiveStorage_DeleteGlobalStats(int controllerIndex);

extern sv_cacvalidate_state_t g_cacvalidateState;
extern const dvar_t *presell;

enum streamName_t : __int32
{                                       // XREF: streams_t/r ticker_t/r ...
    TICKER_STREAM_COD       = 0x0,
    TICKER_STREAM_SQUADS    = 0x1,
    TICKER_STREAM_FRIENDS   = 0x2,
    TICKER_STREAM_EVENTS    = 0x3,
    TICKER_STREAM_CONTRACTS = 0x4,
    TICKER_STREAM_NONE      = 0x5,
    TICKER_STREAM_COUNT     = 0x5,
};

enum messageState_e : __int32
{                                       // XREF: messages_t/r
    MESSAGE_NOT_DISPLAYING  = 0x0,
    MESSAGE_SCROLLING_UP_A  = 0x1,
    MESSAGE_PAUSED_A        = 0x2,
    MESSAGE_SCROLLING_LEFT  = 0x3,
    MESSAGE_PAUSED_B        = 0x4,
    MESSAGE_SCROLLING_UP_B  = 0x5,
    MESSAGE_DONE_DISPLAYING = 0x6,
};

struct messages_t // sizeof=0x204
{                                       // XREF: streams_t/r
    char text[512];
    messageState_e state;
};

struct streams_t // sizeof=0x1434
{                                       // XREF: ticker_t/r
    streamName_t streamName;            // XREF: LiveTicker_DumpTickerInternal(void)+60/r
                                        // LiveTicker_InitializeInternal(void)+46/w ...
    messages_t messages[10];
    int messageCount;
    int currentMessageIndex;
};

struct __declspec(align(4)) ticker_t // sizeof=0x656C
{                                       // XREF: .data:s_ticker/r
    streams_t streams[5];               // XREF: LiveTicker_DumpTickerInternal(void)+60/r
                                        // LiveTicker_InitializeInternal(void)+46/w ...
    streamName_t streamDisplayOrder[5]; // XREF: LiveTicker_SetDefaultDisplayOrder(void)+4/w
                                        // LiveTicker_SetDefaultDisplayOrder(void)+E/w ...
    int streamBatchSize[5];             // XREF: LiveTicker_SetDefaultDisplayOrder(void)+51/w
                                        // LiveTicker_MoveToNextMessage(streamName_t)+61/r ...
    streamName_t currentStream;         // XREF: LiveTicker_InitializeInternal(void)+57/w
                                        // LiveTicker_MoveToFirstStream(void)+4/r ...
    streamName_t previousStream;        // XREF: LiveTicker_InitializeInternal(void)+61/w
                                        // LiveTicker_MoveToNextStream(void)+74/w ...
    int totalMessageCount;              // XREF: LiveTicker_DumpTickerInternal(void)+26/r
                                        // LiveTicker_InitializeInternal(void)+6B/w ...
    int currentStreamIndex;
    bool isHeaderAnimating;             // XREF: LiveTicker_InitializeInternal(void)+75/w
                                        // LiveTicker_GetCurrentMessage(float,float,float,float,Font_s * const,float,float &,float &,float &):loc_98E924/r ...
    // padding byte
    // padding byte
    // padding byte
    int pauseStartTimeA;                // XREF: LiveTicker_InitializeInternal(void)+7C/w
                                        // LiveTicker_GetCurrentMessage(float,float,float,float,Font_s * const,float,float &,float &,float &)+163/w ...
    int scrollLeftStartTime;            // XREF: LiveTicker_InitializeInternal(void)+86/w
                                        // LiveTicker_GetCurrentMessage(float,float,float,float,Font_s * const,float,float &,float &,float &)+16D/w ...
    int scrollUpStartTimeB;             // XREF: LiveTicker_InitializeInternal(void)+90/w
                                        // LiveTicker_GetCurrentMessage(float,float,float,float,Font_s * const,float,float &,float &,float &)+177/w ...
    int pauseStartTimeB;                // XREF: LiveTicker_InitializeInternal(void)+9A/w
                                        // LiveTicker_GetCurrentMessage(float,float,float,float,Font_s * const,float,float &,float &,float &)+181/w ...
    float scrollLeftDuration;           // XREF: LiveTicker_InitializeInternal(void)+AC/w
                                        // LiveTicker_GetCurrentMessage(float,float,float,float,Font_s * const,float,float &,float &,float &)+193/w ...
    float scrollLeftFinalX;             // XREF: LiveTicker_InitializeInternal(void)+BC/w
                                        // LiveTicker_GetCurrentMessage(float,float,float,float,Font_s * const,float,float &,float &,float &)+1A3/w ...
    bool hasMessageScrolledLeft;        // XREF: LiveTicker_InitializeInternal(void)+C4/w
                                        // LiveTicker_GetCurrentMessage(float,float,float,float,Font_s * const,float,float &,float &,float &)+1AB/w ...
    // padding byte
    // padding byte
    // padding byte
    int scrollUpStartTimeA;             // XREF: LiveTicker_InitializeInternal(void)+CB/w
                                        // LiveTicker_GetCurrentMessage(float,float,float,float,Font_s * const,float,float &,float &,float &)+1B7/w ...
    int messagedWidthInPixels;          // XREF: LiveTicker_InitializeInternal(void)+D5/w
                                        // LiveTicker_GetCurrentMessage(float,float,float,float,Font_s * const,float,float &,float &,float &)+1D5/w ...
    int headerAnimationStartTime;       // XREF: LiveTicker_GetCurrentMessageCategory(float,float,float,Font_s *,float,float &,float &,float &)+8C/r
                                        // LiveTicker_GetCurrentMessageCategory(float,float,float,Font_s *,float,float &,float &,float &)+9F/w ...
    bool isUserOnline;                  // XREF: LiveTicker_InitializeInternal(void)+E6/w
    // padding byte
    // padding byte
    // padding byte
};

struct Font_s;
char *__cdecl LiveTicker_GetCurrentMessage(
                float scrX,
                float baseX,
                float baseY,
                float baseAlpha,
                Font_s *const font,
                float scale,
                float *xCoord,
                float *yCoord,
                float *alpha);
char *__cdecl LiveTicker_GetCurrentMessageCategory(
                float baseAlpha,
                float baseX,
                float baseWidth,
                Font_s *font,
                float baseScale,
                float *outXCoord,
                float *outAlpha,
                float *outScale);

enum EUserTier : __int32
{                                       // XREF: XenonUserData/r
                                        // GetIsSuperUser/r
    USER_TIER_NONE      = 0x0,
    USER_TIER_SUPER     = 0x1,
    USER_TIER_DEVELOPER = 0x2,
    USER_TIER_FRIEND    = 0x3,
    USER_TIER_ENEMY     = 0x29A,
};

struct __declspec(align(8)) PlayerRank // sizeof=0x10
{                                       // XREF: Live_UpdatePerformanceValuesComplete/r
    unsigned __int64 xuid;              // XREF: Live_UpdatePerformanceValuesComplete+F3/r
                                        // Live_UpdatePerformanceValuesComplete+FB/r
    int rank;                           // XREF: Live_UpdatePerformanceValuesComplete+139/r
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct __declspec(align(8)) recentServer_t // sizeof=0x10
{                                       // XREF: .data:s_recentServers/r
    unsigned __int64 serverID;          // XREF: Live_FindRecentServers(void)+84/r
                                        // Live_SaveRecentServers(void)+31/r ...
    int joinTime;                       // XREF: Live_FindRecentServers(void)+6A/r
                                        // Live_SaveRecentServers(void)+74/r ...
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};
bool __cdecl Live_IsUserSignedInToLive();
bool __cdecl Live_IsUserSignedInToOnlineService(int controllerIndex);
char __cdecl Live_RequireUserToPlayOnline();
char *__cdecl Live_ControllerIndex_GetClientName(int controllerIndex);
void __cdecl Live_AcceptLastInvite_f();
void __cdecl Live_GiveAchievement(int localControllerIndex, const char *achievementName);
bool __cdecl Live_IsSignedIn(int controllerIndex);
bool __cdecl Live_IsSignedInToLive();
unsigned long long __cdecl Live_GetXuid(int controllerIndex);
int __cdecl Live_GetTier(int controllerIndex);
char __cdecl Live_ShowMarketplaceUI();
int __cdecl Live_GetUploadSpeed();
void __cdecl PC_InitSigninState();
void __cdecl Live_DelayedComError(const char *comErrorString);
void __cdecl Live_GetFriendsOnServer(unsigned __int64 serverId, unsigned __int64 *friendIDs, int *numfriends);
char __cdecl Live_AddFavourite_Ingame(unsigned __int64 serverid, unsigned __int64 serveruid);
void __cdecl Live_AddFavourite(unsigned __int64 serverid, unsigned __int64 serveruid);
void __cdecl Live_DeleteFavourite(unsigned __int64 serverid);
void __cdecl Live_AddRecentPlayers(unsigned __int64 *uids, const char **names, int numIDs);
void __cdecl Live_JoinWagerFromInvite();
void __cdecl Live_AcceptInvite(unsigned __int64 frienduid);
int __cdecl Live_GetInvitesCount();
int __cdecl Live_GetInviteFriend(int index);

extern unsigned __int64 s_selectedPlayerXUID;
extern unsigned __int64 s_selectedMetPlayerXUID;
extern const dvar_t *xblive_clanmatch;
extern const dvar_t *xblive_theater;
extern const dvar_t *xblive_wagermatch;
extern const dvar_t *xblive_basictraining;
extern const dvar_t *xblive_basictraining_popup;
extern const dvar_t *bot_tips;
extern const dvar_t *xblive_matchEndingSoon;
extern const dvar_t *bot_friends;
extern const dvar_t *bot_enemies;
extern const dvar_t *steamid;
extern const dvar_t *scr_bot_difficulty;
extern const dvar_t *clanName;
extern const dvar_t *bot_difficulty;
void __cdecl SocketRouter_EmergencyFrame();
