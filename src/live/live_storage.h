#pragma once
#include <DemonWare/bdCore/bdContainers/bdByteBuffer.h>
#include <DemonWare/bdPlatform/bdPlatformLog/bdPlatformLog.h>
#include <DemonWare/bdCore/bdContainers/bdArray.h>
#include <DW/dwStorage.h>
#include <DemonWare/bdCore/bdSocket/bdAddr.h>
#include <DemonWare/bdCore/bdTiming/bdStopwatch.h>
#include <DemonWare/bdCore/bdContainers/bdBitBuffer.h>

#include <demo/demo_common.h>

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

struct bdStats
{
    enum bdWriteType : __int32
    {                                       // XREF: bdStatsInfo/r
        STAT_WRITE_REPLACE = 0x0,
        STAT_WRITE_ADD = 0x1,
        STAT_WRITE_MAX = 0x2,
        STAT_WRITE_MIN = 0x3,
        STAT_WRITE_REPLACE_WHEN_RATING_INCREASE = 0x4,
        STAT_WRITE_ADD_WHEN_RATING_INCREASE = 0x5,
        STAT_WRITE_MAX_WHEN_RATING_INCREASE = 0x6,
        STAT_WRITE_MIN_WHEN_RATING_INCREASE = 0x7,
    };
};

struct bdTaskResultProcessor // sizeof=0x4
{                                       // XREF: bdPagingToken/r
    //bdTaskResultProcessor_vtbl *__vftable;
    virtual ~bdTaskResultProcessor() = default;
    virtual bool processResult(bdTaskResult *, unsigned int);
};

struct bdRemoteTask : bdReferencable // sizeof=0x40
{                                       // XREF: ?resolveHostIP@bdHTTPClient@@IAE_NAAVbdAddr@@@Z/r
    enum bdStatus : __int32
    {                                       // XREF: bdHTTPWrapperBase::bdStatus/r
                                            // bdRemoteTask/r ...
        BD_EMPTY      = 0x0,
        BD_PENDING    = 0x1,
        BD_DONE       = 0x2,
        BD_FAILED     = 0x3,
        BD_TIMED_OUT  = 0x4,
        BD_CANCELLED  = 0x5,
        BD_MAX_STATUS = 0x6,
    };
    bdStopwatch m_timer;
    float m_timeout;
    bdRemoteTask::bdStatus m_status;
    bdReference<bdByteBuffer> m_byteResults;
    bdTaskResult *m_taskResult;
    bdTaskResult **m_taskResultList;
    unsigned int m_numResults;
    unsigned int m_maxNumResults;
    unsigned int m_totalNumResults;
    unsigned __int64 m_transactionID;
    bdLobbyErrorCode m_errorCode;
    bdTaskResultProcessor *m_taskResultProcessor;
                                        // XREF: bdHTTPClient::resolveHostIP(bdAddr &)+72/o
                                        // bdHTTPClient::resolveHostIP(bdAddr &)+80/w ...

    bdRemoteTask();
    ~bdRemoteTask();

    bdRemoteTask::bdStatus getStatus()
    {
        float ElapsedTimeInSeconds; // [esp+1Ch] [ebp-4h]

        if (this->m_status == 1 && this->m_timeout > 0.0)
        {
            //ElapsedTimeInSeconds = bdStopwatch::getElapsedTimeInSeconds(&this->m_timer);
            ElapsedTimeInSeconds = this->m_timer.getElapsedTimeInSeconds();
            if (this->m_timeout <= (double)ElapsedTimeInSeconds)
            {
                this->m_status = (bdRemoteTask::bdStatus)4;
                //bdReference<bdBitBuffer>::operator=(&this->m_byteResults.m_ptr, 0);
                this->m_byteResults.m_ptr = NULL;
                bdLogMessage(
                    BD_LOG_INFO,
                    "info/",
                    "remote task",
                    "C:\\projects_pc\\cod\\codsrc\\DemonWare\\bdLobby\\bdRemoteTaskManager\\bdRemoteTask.cpp",
                    "bdRemoteTask::getStatus",
                    0x30u,
                    "Remote task timed out after %.3fs.",
                    this->m_timeout);
            }
        }
        return this->m_status;
    }

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
    bdReference<bdRemoteTask> remoteTask;
                                        // XREF: TaskManager2_PickUpDeferredTasks(void)+69/r
                                        // TaskManager2_PickUpDeferredTasks(void)+E5/r ...
    TaskRecord *nestedTask;
    void *payload;                      // XREF: TaskManager2_ProcessDemonwareTask(TaskRecord *)+205/w
                                        // TaskManager2_ProcessDemonwareTask(TaskRecord *)+386/w ...
    bool isChildTask;
    // padding byte
    // padding byte
    // padding byte
    int ownerThread;                    // XREF: TaskManager2_PickUpDeferredTasks(void)+AB/r
                                        // TaskManager2_ProcessDemonwareTask(TaskRecord *)+21A/w ...
};

struct __declspec(align(8)) TaskDefinition // sizeof=0x20
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

struct persistentStats // sizeof=0x996C
{                                       // XREF: .data:s_otherPlayerStats/r
    unsigned __int8 statsBuffer[39272];
    bool isChecksumValid;
    bool statsWriteNeeded;
    bool statsValidatedWithDDL;
    bool statsFetched;
};

enum bdWriteType //bdStats::bdWriteType : __int32
{                                       // XREF: bdStatsInfo/r
    STAT_WRITE_REPLACE = 0x0,
    STAT_WRITE_ADD     = 0x1,
    STAT_WRITE_MAX     = 0x2,
    STAT_WRITE_MIN     = 0x3,
    STAT_WRITE_REPLACE_WHEN_RATING_INCREASE = 0x4,
    STAT_WRITE_ADD_WHEN_RATING_INCREASE = 0x5,
    STAT_WRITE_MAX_WHEN_RATING_INCREASE = 0x6,
    STAT_WRITE_MIN_WHEN_RATING_INCREASE = 0x7,
};

struct bdStatsInfo : bdTaskResult // sizeof=0x70
{                                       // XREF: bdVoteRankStatsInfo/r
                                        // LeaderBoardRow<10>/r ...
    unsigned int m_leaderboardID;       // XREF: LB_IncrementEscrow(void)+F4/w
                                        // LB_IncrementEscrow(void)+14A/w ...
    unsigned __int64 m_entityID;        // XREF: LB_IncrementEscrow(void)+E2/w
                                        // LB_IncrementEscrow(void)+EE/w ...
    //bdStats::bdWriteType m_writeType;   // XREF: LB_IncrementEscrow(void)+FE/w
    bdWriteType m_writeType;   // XREF: LB_IncrementEscrow(void)+FE/w
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

    inline bdStatsInfo() : bdTaskResult()
    {
        this->m_leaderboardID = 0;
        this->m_entityID = 0;
        this->m_writeType = STAT_WRITE_ADD;
        this->m_rating = 0;
        this->m_rank = 0;
        this->m_secondsSinceUpdate = 0;
        memset((unsigned __int8 *)this->m_entityName, 0, sizeof(this->m_entityName));
    }

    inline bool deserialize(bdByteBuffer *buffer)
    {
        bool error = buffer->readUInt64(&m_entityID);
        error = error && buffer->readInt64(&m_rating);
        error = error && buffer->readUInt64(&m_rank);
        error = error && buffer->readString(m_entityName, 64);

        if (!error)
        {
            bdLogMessage(
                BD_LOG_ERROR,
                "err/",
                "statsInfo",
                "C:\\projects_pc\\cod\\codsrc\\DemonWare\\bdLobby\\bdStats\\bdStatsInfo.cpp",
                "bdStatsInfo::deserialize",
                0x32u,
                "Deserialization failed");
        }

        return error;
    }
};

struct __declspec(align(8)) bdVoteRankStatsInfo : bdStatsInfo // sizeof=0xC8
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

    bdVoteRankStatsInfo() : bdStatsInfo()
    {
        this->m_fileOwnerID = 0;
        this->m_totalVotes = 0;
        this->m_avgVoteValue = 0;
        memset((unsigned __int8 *)this->m_fileOwnerName, 0, sizeof(this->m_fileOwnerName));
    }

    ~bdVoteRankStatsInfo() = default;
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
    bdFileMetaData descriptors[20];
    bdVoteRankStatsInfo fileRatings[20];
    int descriptorCount;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct fileRetryInfo // sizeof=0xC
{                                       // XREF: .data:s_UTCRetryInfo/r
                                        // dwFileOperationInfo/r
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

struct __declspec(align(8)) dwFileOperationInfo // sizeof=0x120
{                                       // XREF: .data:s_finfo/r
    dwFileTask fileTask;                // XREF: SV_AP_GetControlFile(void)+31/w
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


struct dwFileShareGetRatingTask // sizeof=0x24
{                                       // XREF: playerFileOperations/r
    fileShareLocation location;
    unsigned __int64 *fileIDs;
    int maxFilesToFetch;
    int leaderboardID;
    bdVoteRankStatsInfo *outStatsInfo;
    int *outNumRatingsReceived;
    int cacheLoadedFileCount;
    void (__cdecl *successCallback)();
    void (__cdecl *failureCallback)();
};

struct bdFileID : bdTaskResult // sizeof=0x10
{                                       // XREF: .data:searchFileIDs/r
                                        // fileShareSearchResults/r ...
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    unsigned __int64 m_fileID;          // XREF: Live_FileShare_GetLastPlayedGameDetails(int)+34/w
                                        // Live_FileShare_GetLastPlayedGameDetails(int)+39/w ...
};

struct dwFileShareDescriptorsTask // sizeof=0x24
{                                       // XREF: playerFileOperations/r
    fileShareLocation location;
    unsigned int numFiles;
    bdFileID *fileIDs;
    int cacheLoadedFileCount;
    bdFileMetaData *descriptors;
    int *descriptorCount;
    bool getFirstSummary;
    bool getLastSummary;
    // padding byte
    // padding byte
    void (__cdecl *successCallback)();
    void (__cdecl *failureCallback)();
};

struct fileShareSearchInfo_t // sizeof=0x30
{                                       // XREF: Live_FileShareSearch_LoadBook/r
    bdTag *tags;                        // XREF: Live_FileShareSearch_LoadBook:loc_964DD6/w
    int numTags;                        // XREF: Live_FileShareSearch_LoadBook+F2/w
    fileShareLocation location;         // XREF: Live_FileShareSearch_LoadBook+FB/w
    int startIndex;                     // XREF: Live_FileShareSearch_LoadBook+104/w
    int maxFileIDs;                     // XREF: Live_FileShareSearch_LoadBook+107/w
    bdFileID *outFileIDs;               // XREF: Live_FileShareSearch_LoadBook+10E/w
    int *outFileCount;                  // XREF: Live_FileShareSearch_LoadBook+115/w
    bdVoteRankStatsInfo *outRatings;    // XREF: Live_FileShareSearch_LoadBook+11C/w
    int *outRatingCount;                // XREF: Live_FileShareSearch_LoadBook+123/w
    int *outTotalFileCount;             // XREF: Live_FileShareSearch_LoadBook+12A/w
    void (__cdecl *successCallback)();  // XREF: Live_FileShareSearch_LoadBook+131/w
    void (__cdecl *failureCallback)();  // XREF: Live_FileShareSearch_LoadBook+138/w
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
    void (__cdecl *successCallback)(dwFileShareReadFileTask *);
                                        // XREF: UI_Gametype_DownloadFromFileShare_f(void)+94/w
                                        // SV_GameType_DownloadFromFileshare_f(void)+13F/w ...
    void (__cdecl *failureCallback)(dwFileShareReadFileTask *);
                                        // XREF: UI_Gametype_DownloadFromFileShare_f(void)+9B/w
                                        // SV_GameType_DownloadFromFileshare_f(void)+146/w ...
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct fileShareDownloadInterceptor : bdDownloadInterceptor // sizeof=0xC
{                                       // XREF: .data:downloadInterceptor/r
    dwFileShareReadFileTask *task;      // XREF: LiveStorage_FileShare_ReadFile(int,fileShareReadFileInfo *)+1BF/w
    unsigned int (__cdecl *callback)(void *, unsigned int, dwFileShareReadFileTask *);
                                        // XREF: LiveStorage_FileShare_ReadFile(int,fileShareReadFileInfo *)+1B2/w

    unsigned int handleDownload(void *data, unsigned int dataSize);
};

struct bdUploadInterceptor // sizeof=0x4
{                                       // XREF: fileShareUploadInterceptor/r
    //bdUploadInterceptor_vtbl *__vftable;
                                        // XREF: LiveStorage_FileShare_WriteFile(int,fileShareWriteFileInfo *)+29/w

    virtual ~bdUploadInterceptor() = default;
    virtual unsigned int handleUpload(void *, unsigned int, unsigned int);
};

struct __declspec(align(8)) dwFileShareWriteFileTask // sizeof=0x50
{                                       // XREF: playerFileOperations/r
    fileShareLocation location;
    unsigned __int16 fileSlot;
    // padding byte
    // padding byte
    const char *menuDef;
    void *fileData;
    bdUploadInterceptor *uploadHandler;
    unsigned int fileSize;
    const char *fileName;
    unsigned __int16 category;
    // padding byte
    // padding byte
    const void *thumbData;
    unsigned int thumbDataSize;
    unsigned int numTags;
    bdTag *tags;
    bdFileID outFileID;
    unsigned int (__cdecl *dataCallback)(void *, unsigned int, unsigned int);
    void (__cdecl *successCallback)(int, unsigned __int64);
    void (__cdecl *failureCallback)(int);
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct __declspec(align(8)) dwFileShareListTask // sizeof=0x28
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
    bdFileMetaData *descriptors;
    const char *menuDef;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct dwFileShareSearchTask // sizeof=0x38
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
    bdTag *tags;
    int *resultFileCount;
    bdFileID *resultFileIDs;
    int *resultRatingCount;
    bdVoteRankStatsInfo *resultRatings;
    int *resultTotalFileCount;
    void (__cdecl *successCallback)();
    void (__cdecl *failureCallback)();
};

struct dwFileShareTagTask // sizeof=0x18
{                                       // XREF: playerFileOperations/r
    fileShareLocation location;
    bdTag *tags;
    unsigned int numTags;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    unsigned __int64 entityID;
};

struct dwFileShareTransferTask // sizeof=0x138
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
    bdTag *tags;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    bdFileID outFileID;
};

struct dwFileShareRemoveTask // sizeof=0x4
{                                       // XREF: playerFileOperations/r
    unsigned int fileSlot;
};

struct dwFileShareSummaryTask // sizeof=0x30
{                                       // XREF: playerFileOperations/r
    unsigned __int64 fileID;
    void *summaryData;
    unsigned int summaryDataSize;
    const void *metaData;
    unsigned int metaDataSize;
    unsigned int numTags;
    bdTag *tags;
    fileShareLocation location;
    bool showSuccess;
    // padding byte
    // padding byte
    // padding byte
    void (__cdecl *successCallback)();
    void (__cdecl *failureCallback)();
};

struct __declspec(align(8)) bdRatingInfo : bdTaskResult // sizeof=0x18
{                                       // XREF: dwFileShareSubmitRatingTask/r
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

struct __declspec(align(8)) dwFileShareSubmitRatingTask // sizeof=0x20
{                                       // XREF: playerFileOperations/r
    bdRatingInfo rating[1];
    unsigned int numRatings;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct __declspec(align(8)) dwFileShareGetTopRatedTask // sizeof=0x4E48
{                                       // XREF: playerFileOperations/r
    unsigned __int64 startRank;
    int maxFiles;
    unsigned int leaderboardID;
    bdVoteRankStatsInfo statsInfo[100];
    int *outNumFiles;
    int *outTotalNumFiles;
    bdFileID *outFileIDs;
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

    dwFileOperationInfo statsBackupFileInfo;
    dwFileOperationInfo readDWEmailFileInfo;
    dwFileOperationInfo saveDWEmailFileInfo;
    dwFileOperationInfo squadMessagesFileInfo;
    dwFileOperationInfo codMessagesFileInfo;
    dwFileOperationInfo eventMessagesFileInfo;
    dwFileOperationInfo hiddenOfferIdsFileInfo;
    dwFileOperationInfo fbStatusFileInfo;
    dwFileOperationInfo clanMOTDInfo;
    dwFileOperationInfo corruptedStatsBackupFileInfo;
                                        // XREF: LiveStorage_ResetAllFileOps+29/o
    dwFileOperationInfo fetchMetPlayerListFileInfo;
                                        // XREF: LiveStorage_ResetAllFileOps+41/o
    dwFileOperationInfo saveMetPlayerListFileInfo;
                                        // XREF: LiveStorage_ResetAllFileOps+A0/o
    dwFileOperationInfo getOtherPlayerStatsFileInfo;
                                        // XREF: LiveStorage_ResetAllFileOps+59/o
    dwFileOperationInfo getPlayerStatsFileInfo;
                                        // XREF: LiveStorage_ResetAllFileOps+70/o
    dwFileOperationInfo getBasicTrainingStatsFileInfo;
                                        // XREF: LiveStorage_ResetAllFileOps+88/o
    dwFileOperationInfo saveStatsFileInfo;
                                        // XREF: LiveStorage_ResetAllFileOps+B7/o
    dwFileOperationInfo saveBasicTrainingStatsFileInfo;
    dwFileOperationInfo contractsFileInfo;
    dwFileOperationInfo demoFileInfo;
    dwFileShareListTask fileShareListTask;
    dwFileShareSearchTask fileShareSearchTask;
    dwFileShareDescriptorsTask fileShareDescriptorsTask;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    dwFileShareReadFileTask fileShareReadFileTask;
    dwFileShareWriteFileTask fileShareWriteFileTask;
    dwFileShareTagTask fileShareTagTask;
    dwFileShareTransferTask fileShareTransferTask;
    dwFileShareRemoveTask fileShareRemoveTask;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    dwFileShareSummaryTask fileShareSummaryTask;
    dwFileShareSubmitRatingTask fileShareSubmitRatingTask;
    dwFileShareGetRatingTask fileShareGetRatingTask;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    dwFileShareGetTopRatedTask fileShareGetTopRatedTask;
    dwFileShareGetRatingTask fileShareFilmRatingTask;
    dwFileShareGetRatingTask fileShareClipRatingTask;
    dwFileShareGetRatingTask fileShareScreenshotRatingTask;
    dwFileShareGetRatingTask fileShareCustomGameRatingTask;
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
    bdTag *tags;                        // XREF: UI_Gametype_Custom_UploadToFileShare(int,uint,char const *,MemoryFile &)+4D/w
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

struct playerNetworkData // sizeof=0x3AD90
{                                       // XREF: .data:controllerNetworkData/r
    _BYTE playerStats[40172];
    _BYTE playerStatsBackup[40172];     // XREF: LiveStorage_GetPersStatsBuffer+81/o
    _BYTE stableStatsBuffer[40172];     // XREF: LiveStorage_GetPersStatsBuffer+94/o
    _BYTE basicTrainingStats[40172];    // XREF: LiveStorage_GetPersStatsBuffer+B1/o
                                        // LiveStorage_GetPersStatsBuffer+117/o
    _BYTE globalplayerStats[40172];     // XREF: LiveStorage_GetPersStatsBuffer+C5/o
    _BYTE globalStablePlayerStats[40172];
                                        // XREF: LiveStorage_GetPersStatsBuffer+D8/o
    bool firstTimeRunning;              // XREF: LiveStorage_PlayerStatsFileNotFound+49/w
                                        // LiveStorage_ReadPlayerStatsSuccessful+79/w ...
    bool fileShareFetched;
    // padding byte
    // padding byte
    playerFileOperations *fileOps;      // XREF: LiveStorage_StatsBackupFetchCompleted(int)+C/r
                                        // LiveStorage_StatsBackupFetchCompleted(int)+44/r ...
};

persistentStats *__cdecl LiveStorage_GetStatsBuffer(
                int controllerIndex,
                statsLocation playerStatsLocation,
                bool verifyLocation);
persistentStats *__cdecl LiveStorage_GetPersStatsBuffer(
                int controllerIndex,
                statsLocation playerStatsLocation,
                bool verifyLocation);
void __cdecl LiveStorage_VerifyCorrectStats(persistentStats *stats, statsLocation location);
void __cdecl LiveStorage_CorrectStatsError(char *msg);
int __cdecl LiveStorage_GetStatsBufferSize();
unsigned __int8 __cdecl LiveStorage_GetStatsChecksumValid(int controllerIndex, statsLocation playerStatsLocation);
void __cdecl LiveStorage_SetStatsChecksumValid(int controllerIndex, statsLocation playerStatsLocation, bool isValid);
bool __cdecl LiveStorage_GetStatsWriteNeeded(int controllerIndex, statsLocation location);
void __cdecl LiveStorage_SetStatsWriteNeeded(int controllerIndex, bool isWriteNeeded, statsLocation location);
int __cdecl LiveStorage_ValidateWithDDL(int controllerIndex, statsLocation location);
unsigned __int8 __cdecl LiveStorage_AreStatsDDLValidated(int controllerIndex, statsLocation playerStatsLocation);
void __cdecl LiveStorage_SetStatsDDLValidated(
                int controllerIndex,
                statsLocation playerStatsLocation,
                bool statsValidatedWithDDL);
void __cdecl LiveStorage_StatsBackupFetchCompleted(int controllerIndex);
unsigned __int8 __cdecl LiveStorage_DoWeHaveStats(int controllerIndex, statsLocation playerStatsLocation);
unsigned __int8 __cdecl LiveStorage_DoWeHaveCurrentStats(int controllerIndex);
bool __cdecl LiveStorage_DoWeHaveAllStats(int controllerIndex);
void __cdecl LiveStorage_SetStatsFetched(int localControllerIndex, statsLocation playerStatsLocation, bool isFetched);
fileSharePrivateData *__cdecl LiveStorage_FileShare_GetFileShareData(fileShareBufferLocation bufferLocation);
void __cdecl LiveStorage_FetchRequiredFiles(int controllerIndex);
TaskRecord *__cdecl LiveStorage_SetupNestedTask(
                const TaskDefinition *taskdef,
                int controllerIndex,
                TaskRecord *nestedTask,
                void *payload);
void __cdecl LiveStorage_ResetFileInfo(dwFileOperationInfo *fileInfo);
void __cdecl LiveStorage_DeleteUserFileFailure(TaskRecord *task);
void __cdecl LiveStorage_DeleteUserFileSuccess(TaskRecord *task);
TaskRecord *__cdecl LiveStorage_DeleteDWUserFile(int controllerIndex, dwFileOperationInfo *fileInfo);
void __cdecl LiveStorage_WriteUserFileFailure(TaskRecord *task);
void __cdecl LiveStorage_WriteUserFileSuccess(TaskRecord *task);
TaskRecord *__cdecl LiveStorage_WriteDWUserFile(
                int controllerIndex,
                dwFileOperationInfo *fileInfo,
                unsigned __int64 uid);
void __cdecl LiveStorage_GetUserFileFailure(TaskRecord *task);
void __cdecl LiveStorage_GetUserFileSuccess(TaskRecord *task);
TaskRecord *__cdecl LiveStorage_ReadDWFile(int controllerIndex, dwFileOperationInfo *fileInfo);
char __cdecl LiveStorage_TryFile(fileRetryInfo *retryInfo);
void __cdecl LiveStorage_UpdateFileTryData(fileRetryInfo *retryInfo);
TaskRecord *__cdecl LiveStorage_ReadDWFileByUserID(
                int controllerIndex,
                dwFileOperationInfo *fileInfo,
                unsigned __int64 xuidLocal);
void __cdecl LiveStorage_RestoreStatsFromBackup(int localControllerIndex);
TaskRecord *__cdecl LiveStorage_ReadStatsBackup(int controllerIndex);
void __cdecl LiveStorage_StatsBackupReadSuccessful(int controllerIndex);
int __cdecl LiveStorage_StatsBackupFileNotFound(int controllerIndex, const char **data);
TaskRecord *__cdecl LiveStorage_SyncTime(int controllerIndex);
void __cdecl LiveStorage_GetServerTimeComplete(TaskRecord *rec);
void __cdecl LiveStorage_GetServerTimeFailed(TaskRecord *rec);
__int64 __cdecl LiveStorage_GetUTC();
int __cdecl LiveStorage_GetUTCOffset();
bool __cdecl LiveStorage_IsTimeSynced();
TaskRecord *__cdecl LiveStorage_ReadMetPlayerList(
                int controllerIndex,
                char *fileName,
                unsigned __int8 *buffer,
                int sizeOfBuffer);
bool __cdecl LiveStorage_NeedToDownloadMetPlayerList(int controllerIndex);
int __cdecl LiveStorage_ReadMetPlayerListFileNotFound(int controllerIndex);
void __cdecl LiveStorage_SetLastMetPlayerListDownload(int controllerIndex);
void __cdecl LiveStorage_ReadMetPlayerListSuccessful(int controllerIndex);
TaskRecord *__cdecl LiveStorage_WriteMetPlayerList(
                int controllerIndex,
                char *fileName,
                unsigned __int8 *buffer,
                int sizeOfBuffer);
void __cdecl LiveStorage_WriteMetPlayerListSuccessful(int controllerIndex);
TaskRecord *__cdecl LiveStorage_ReadOtherPlayerStats(int controllerIndex, unsigned __int64 playerXuid);
void __cdecl LiveStorage_ReadOtherPlayerStatsSuccessful(int controllerIndex);
int __cdecl LiveStorage_OtherPlayerStatsFileNotFound(int controllerIndex, const char **data);
TaskRecord *__cdecl LiveStorage_ReadPlayerStats(int controllerIndex, bool validate, bool silent);
int __cdecl LiveStorage_PlayerStatsFileNotFound(int controllerIndex, const char **data);
void __cdecl LiveStorage_ReadPlayerStatsSuccessful(int controllerIndex);
TaskRecord *__cdecl LiveStorage_ReadCommonStats(
                int controllerIndex,
                bool silent,
                statsLocation location,
                const TaskDefinition *taskDef,
                dwFileOperationInfo *fileInfo);
TaskRecord *__cdecl LiveStorage_ReadBasicTrainingStats(int controllerIndex, bool validate, bool silent);
int __cdecl LiveStorage_BasicTrainingStatsFileNotFound(int controllerIndex, const char **data);
void __cdecl LiveStorage_ReadBasicTrainingStatsSuccessful(int controllerIndex);
void __cdecl LiveStorage_WritePlayerStatsFailure(TaskRecord *task);
void __cdecl LiveStorage_WriteBasicTrainingStatsFailure(TaskRecord *task);
TaskRecord *__cdecl LiveStorage_WriteStats(
                int controllerIndex,
                bool backup,
                statsLocation location,
                const TaskDefinition *taskDef,
                dwFileOperationInfo *fileInfo);
TaskRecord *__cdecl LiveStorage_WriteBasicTrainingStats(int controllerIndex);
void __cdecl LiveStorage_WriteBackupStats(int controllerIndex);
TaskRecord *__cdecl LiveStorage_WriteBackupStatsInternal(
                int controllerIndex,
                dwFileOperationInfo *fileInfo,
                const TaskDefinition *taskdef);
void __cdecl LiveStorage_BackupCorruptedStats(int controllerIndex, char *filename);
void __cdecl LiveStorage_SetAllStatsNotFetched(int controllerIndex);
void __cdecl LiveStorage_FileShare_UpdateMetaData(char *metaData);
bool __cdecl LiveStorage_FileShare_IsValidHTTPCode(int httpCode);
fileShareLocation __cdecl LiveStorage_FileShare_GetCurrentHTTPLocation(int controllerIndex);
void __cdecl LiveStorage_FileShare_GetDownloadProgress(int controllerIndex, unsigned int *bytes, float *dataRate);
void __cdecl LiveStorage_FileShare_GetUploadProgress(int controllerIndex, unsigned int *bytes, float *dataRate);
void __cdecl LiveStorage_FileShare_AbortOperation(int controllerIndex, fileShareLocation location);
TaskRecord *__cdecl LiveStorage_FileShare_ReadListing(
                int controllerIndex,
                unsigned __int64 playerXuid,
                fileShareBufferLocation bufferLocation);
void __cdecl LiveStorage_FileShare_ReadListingFailure(TaskRecord *task);
void __cdecl LiveStorage_FileShare_KickOffRatingsTask(
                int controllerIndex,
                fileSharePrivateData *fileShareData,
                int category,
                dwFileShareGetRatingTask *ratingTask,
                const TaskDefinition *sym,
                int *resultOffset);
void __cdecl LiveStorage_FileShare_ReadListingSuccess(TaskRecord *task);
bool __cdecl LiveStorage_FileShare_IsPageLoading(int controllerIndex);
void __cdecl LiveStorage_FileShare_PublicMetaToTags(bdFileMetaData *descriptors, int descriptorCount);
void __cdecl LiveStorage_FileShare_LoadBlockSuccessAction(dwFileShareDescriptorsTask *fsTask);
int __cdecl LiveStorage_FileShareSortComparator_MostRecent(unsigned int *arg0, unsigned int *arg1);
int __cdecl LiveStorage_FileShareSortComparator_MostPopular(const bdFileMetaData *arg0, const bdFileMetaData *arg1);
TaskRecord *__cdecl LiveStorage_FileShare_LoadPage(
                int controllerIndex,
                int numFiles,
                bdFileID *fileIDs,
                fileShareLocation location,
                bdFileMetaData *outDescriptors,
                int *outDescriptorCount,
                void (__cdecl *successCallback)(),
                void (__cdecl *failureCallback)());
void __cdecl LiveStorage_FileShare_LoadBlockFailure(TaskRecord *task);
void __cdecl LiveStorage_FileShare_LoadBlockSuccess(TaskRecord *task);
void __cdecl LiveStorage_FileShare_DownloadFile_f();
bool __cdecl LiveStorage_FileShare_IsPerformingSearch(int controllerIndex);
TaskRecord *__cdecl LiveStorage_FileShare_PerformSearch(int controllerIndex, fileShareSearchInfo_t *taskInfo);
void __cdecl LiveStorage_FileShare_SearchFailure(TaskRecord *task);
void __cdecl LiveStorage_FileShare_SearchSuccess(TaskRecord *task);
unsigned int __cdecl LiveStorage_FileShare_ReadFileData(
                void *data,
                unsigned int dataSize,
                dwFileShareReadFileTask *fileTask);
unsigned int __cdecl LiveStorage_FileShare_GetDownloadingFileTotalSize(int controllerIndex);
unsigned int __cdecl LiveStorage_FileShare_GetUploadingFileTotalSize(int controllerIndex);
TaskRecord *__cdecl LiveStorage_FileShare_ReadFile(int controllerIndex, fileShareReadFileInfo *fileInfo);
void __cdecl LiveStorage_FileShare_ReadFileFailure(TaskRecord *task);
void __cdecl LiveStorage_FileShare_ReadFileSuccess(TaskRecord *task);
char *__cdecl LiveStorage_GetMatchRecordBuffer();
int __cdecl LiveStorage_GetMatchRecordBufferSize();
unsigned int __cdecl LiveStorage_FileShare_WriteFileData(
                void *data,
                unsigned int dataSize,
                unsigned int bytesUploaded,
                dwFileShareWriteFileTask *fileTask);
TaskRecord *__cdecl LiveStorage_FileShare_WriteFile(int controllerIndex, fileShareWriteFileInfo *writeFileInfo);

void __cdecl LiveStorage_FileShare_WriteFileFailure(TaskRecord *task);
void __cdecl LiveStorage_FileShare_WriteFileSuccess(TaskRecord *task);
void __cdecl LiveStorage_FileShare_GenerateHeatmap(
                int controllerIndex,
                unsigned __int64 fileID,
                unsigned __int8 *buffer,
                unsigned int bufferSize);
TaskRecord *__cdecl LiveStorage_FileShare_WriteSummary(
                int controllerIndex,
                unsigned __int64 fileID,
                fileShareLocation location,
                void *summaryData,
                unsigned int summaryDataSize,
                const void *metaData,
                unsigned int metaDataSize,
                bdTag *tags,
                unsigned int numTags,
                bool showSuccess);
void __cdecl LiveStorage_FileShare_WriteSummaryFailure(TaskRecord *rec);
void __cdecl LiveStorage_FileShare_WriteSummarySuccess(TaskRecord *task);
TaskRecord *__cdecl LiveStorage_FileShare_ReadSummary(
                int controllerIndex,
                bdFileMetaData *descriptor,
                void (__cdecl *successCallback)(),
                void (__cdecl *failureCallback)());
void __cdecl LiveStorage_FileShare_ReadSummaryFailure(TaskRecord *task);
void __cdecl LiveStorage_FileShare_ReadSummarySuccess(TaskRecord *task);
TaskRecord *__cdecl LiveStorage_FileShare_ReadMetaDataByID(
                int controllerIndex,
                bdFileID *fileIDs,
                unsigned int numFiles,
                bdFileMetaData *outDescriptors,
                fileShareLocation location);
void __cdecl LiveStorage_FileShare_ReadMetaDataByIDFailure(TaskRecord *task);
void __cdecl LiveStorage_FileShare_ReadMetaDataByIDSuccess(TaskRecord *task);
TaskRecord *__cdecl LiveStorage_FileShare_RemoveFile(int controllerIndex, unsigned int fileSlot);
void __cdecl LiveStorage_FileShare_RemoveFileFailure(TaskRecord *rec);
void __cdecl LiveStorage_FileShare_RemoveFileSuccess(TaskRecord *task);
TaskRecord *__cdecl LiveStorage_FileShare_TransferLastUploaded(int controllerIndex, unsigned int userSlot);
TaskRecord *__cdecl LiveStorage_FileShare_TransferFromPooled(
                int controllerIndex,
                unsigned __int64 fileID,
                unsigned int userSlot);
TaskRecord *__cdecl LiveStorage_FileShare_TransferFromUser(
                int controllerIndex,
                unsigned __int64 fileID,
                unsigned int userSlot,
                bool isCommunity);
TaskRecord *__cdecl LiveStorage_FileShare_TransferFile(
                int controllerIndex,
                unsigned __int64 fileID,
                unsigned int userSlot,
                fileShareLocation location,
                bool isCommunityFile);
void __cdecl LiveStorage_FileShare_TransferFileFailure(TaskRecord *task);
void __cdecl LiveStorage_FileShare_TransferFileSuccess(TaskRecord *task);
TaskRecord *__cdecl LiveStorage_FileShare_WriteRating(
                int controllerIndex,
                unsigned __int64 fileID,
                unsigned __int8 rating);
void __cdecl LiveStorage_FileShare_WriteRatingFailure(TaskRecord *rec);
void __cdecl LiveStorage_FileShare_WriteRatingSuccess(TaskRecord *rec);
bool __cdecl LiveStorage_FileShare_IsReadingTopRated(int controllerIndex);
TaskRecord *__cdecl LiveStorage_FileShare_ReadTopRated(
                int controllerIndex,
                int fileType,
                int startIndex,
                int maxFiles,
                bdFileID *fileIDs,
                int *numFiles,
                int *totalNumFiles,
                void (__cdecl *successCallback)(),
                void (__cdecl *failureCallback)());
void __cdecl LiveStorage_FileShare_ReadTopRatedFailure(TaskRecord *task);
void __cdecl LiveStorage_FileShare_ReadTopRatedSuccess(TaskRecord *task);
bool __cdecl LiveStorage_FileShare_IsReadingRatings(int controllerIndex);
int __cdecl LiveStorage_FileShare_VoteRankComparator(unsigned int *arg0, unsigned int *arg1);
void __cdecl LiveStorage_FileShare_ReadRatingSuccessAction(dwFileShareGetRatingTask *ratingTask);
TaskRecord *__cdecl LiveStorage_FileShare_ReadRatingsCommunity(
                int controllerIndex,
                bdFileID *fileIDs,
                int numFileIDs,
                fileShareLocation location,
                bdVoteRankStatsInfo *outRatings,
                int *outNumRatings,
                void (__cdecl *successCallback)(),
                void (__cdecl *failureCallback)());
TaskRecord *__cdecl LiveStorage_FileShare_ReadRating(
                int controllerIndex,
                dwFileShareGetRatingTask *ratingTask,
                const TaskDefinition *sym);
void __cdecl LiveStorage_FileShare_ReadRatingFailure(TaskRecord *task);
void __cdecl LiveStorage_FileShare_ReadRatingSuccess(TaskRecord *task);
void __cdecl LiveStorage_FileShare_ReadFileRatingSuccess(TaskRecord *task);
void __cdecl LiveStorage_FileShare_WriteTagsFailure(TaskRecord *task);
void __cdecl LiveStorage_FileShare_WriteTagsSuccess(TaskRecord *task);
void __cdecl LiveStorage_ListCustomGameTypesForUser_f();
void __cdecl LiveStorage_NewUser(int controllerIndex);
void __cdecl LiveStorage_ClearPlayerStats(int controllerIndex);
int LiveStorage_ResetAllFileOps();
char __cdecl LiveStorage_Init();
void __cdecl LiveStorage_WriteBackupStatsCmd();
void __cdecl LiveStorage_ReadStatsBackupCmd();
void __cdecl LiveStorage_GetFriendStatsCmd();
void __cdecl LiveStorage_RestoreStatsFromBackupCmd();
void __cdecl LiveStorage_FakeComErrorCmd();
void __cdecl LiveStorage_CheckOngoingSessionTasks();


extern const dvar_t *stat_version;
extern const dvar_t *stats_version_check;
extern const dvar_t *maxStatsBackupInterval;
extern const dvar_t *dwFileFetchTryIntervalBase;
extern const dvar_t *dwFileFetchTryIntervalMax;
extern const dvar_t *dwFileFetchTryMaxAttempts;
extern const dvar_t *clanMessageLastFetchTime;
extern const dvar_t *eventMessageLastFetchTime;
extern const dvar_t *codMessageLastFetchTime;
extern const dvar_t *waitOnStatsTimeout;
extern const dvar_t *heatMapLoadDelay;
extern const dvar_t *minDelayForOtherPlayerStatsFetch;
extern const dvar_t *fshSearchTaskDelay;
extern const dvar_t *basicTrainingFatal;