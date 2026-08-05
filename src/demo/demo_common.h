#pragma once
#include <qcommon/msg_mp.h>
#include <ddl/ddl_api.h>
#include <DemonWare/bdCore/bdMemory/bdMemory.h>
#include <DemonWare/bdCore/bdReference/bdReferencable.h>
#include <DemonWare/bdCore/bdContainers/bdByteBuffer.h>
#include <DemonWare/bdCore/bdSocket/bdAddr.h>
#include <cstring>
#include <cgame_mp/cg_scoreboard_mp.h>


struct bdDownloadInterceptor // sizeof=0x4
{                                                                             // XREF: fileShareDownloadInterceptor/r
        //bdDownloadInterceptor_vtbl *__vftable;
                                                                                // XREF: LiveStorage_FileShare_ReadFile(int,fileShareReadFileInfo *)+29/w
        virtual ~bdDownloadInterceptor() = default;
        virtual unsigned int handleDownload(void *, unsigned int);
};

struct bdTaskResult // sizeof=0x4
{                                                                             // XREF: .data:s_quickMatchServers/r
    bdTaskResult()
    {

    }
    virtual ~bdTaskResult() = default;
    //virtual bool deserialize(bdReference<bdByteBuffer>) = 0;
    //virtual unsigned int sizeOf() = 0;

    // HACK: I really dont give 2 shits about this class

    inline bool deserialize()
    {
        return false;
    }
    inline int sizeOf()
    {
        return sizeof(this);
    }
};

struct bdSessionID : bdTaskResult // sizeof=0xC
{                                       // XREF: .data:bdSessionID g_sessionID/r
    bdSecurityID m_sessionID;           // XREF: CL_Disconnect(int,bool)+271/w
};

struct bdTag : bdTaskResult // sizeof=0x18
{                                                                             // XREF: bdFileMetaData/r
                                                                                // demoRecordedFileUploadInfo/r ...
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        unsigned __int64 m_priTag;
        unsigned __int64 m_secTag;

        bdTag() : bdTaskResult()
        {
            //bdTaskResult::bdTaskResult(this);
            //this->__vftable = (bdTaskResult_vtbl *)&bdTag::`vftable';
            //* ((_DWORD *)this + 2) = 0;
            //*((_DWORD *)this + 3) = 0;
            //*((_DWORD *)this + 4) = 0;
            //*((_DWORD *)this + 5) = 0;
            m_priTag = 0;
            m_secTag = 0;
        }

        virtual ~bdTag() = default;

        void set(unsigned __int64 a2, unsigned __int64 a3)
        {
            this->m_priTag = a2;
            this->m_secTag = a3;
        }

        void serialize(struct bdByteBuffer *a2)
        {
            //bdByteBuffer::writeUInt64(a2, this->m_priTag);
            a2->writeUInt64(this->m_priTag);
            //bdByteBuffer::writeUInt64(a2, this->m_secTag);
            a2->writeUInt64(this->m_secTag);
        }
};

struct bdFileMetaData : bdTaskResult // sizeof=0x838
{                                                                             // XREF: .data:s_theaterFileMetaData/r
                                                                                // .data:descriptor/r ...
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        unsigned __int64 m_fileID;                    // XREF: Live_FileShare_GetLastPlayedGameDetails(int)+52/r
                                                                                // Live_FileShare_GetLastPlayedGameDetails(int)+60/r ...
        unsigned int m_createTime;
        unsigned int m_modifedTime;
        unsigned int m_fileSize;                        // XREF: Live_FileShare_Theater_GetSize(int *)+17/r
                                                                                // LiveStorage_FileShare_TransferFromPooled(int,unsigned __int64,uint):loc_985947/r ...
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        unsigned __int64 m_ownerID;
        char m_ownerName[64];
        unsigned __int16 m_fileSlot;
        char m_fileName[128];                             // XREF: LiveStorage_FileShare_TransferFromPooled(int,unsigned __int64,uint)+189/o
        char m_url[384];
        unsigned __int16 m_category;                // XREF: Live_FileShare_Theater_GetType(int *):loc_95EF64/r
                                                                                // LiveStorage_FileShare_TransferFromPooled(int,unsigned __int64,uint)+17C/r
        unsigned int m_numTags;                         // XREF: Live_FileShare_PopulateLastPlayedGameDvars(void)+1B/r
                                                                                // LiveStorage_FileShare_TransferFromPooled(int,unsigned __int64,uint)+1CF/r
        unsigned __int8 m_metaData[512];        // XREF: Live_FileShare_Theater_GetName(char const * *)+6/o
                                                                                // Live_FileShare_Theater_GetLength(int *)+6/o ...
        unsigned int m_metaDataSize;
        unsigned int m_summaryFileSize;
        bdTag m_tags[40];                                     // XREF: LiveStorage_FileShare_TransferFromPooled(int,unsigned __int64,uint)+1DE/o

        void resetArrays()
        {
            unsigned int i; // [esp+4h] [ebp-4h]

            memset(this->m_fileName, 0, sizeof(this->m_fileName));
            memset(this->m_url, 0, sizeof(this->m_url));
            memset(this->m_ownerName, 0, sizeof(this->m_ownerName));
            memset(this->m_metaData, 0, sizeof(this->m_metaData));

            for (i = 0; i < this->m_numTags; ++i)
            {
                //bdTag::set(&this->m_tags[i], 0, 0);
                this->m_tags[i].set(0, 0);
            }
        }

        bdFileMetaData() : bdTaskResult()
        {
            //bdTaskResult::bdTaskResult(this);
            //this->__vftable = (bdFileMetaData_vtbl *)&bdFileMetaData::`vftable';
                LODWORD(this->m_fileID) = 0;
            HIDWORD(this->m_fileID) = 0;
            this->m_createTime = 0;
            this->m_modifedTime = 0;
            this->m_fileSize = 0;
            LODWORD(this->m_ownerID) = 0;
            HIDWORD(this->m_ownerID) = 0;
            this->m_fileSlot = 0;
            this->m_numTags = 0;
            this->m_metaDataSize = 0;
            this->m_summaryFileSize = 0;
            //`eh vector constructor iterator'(this->m_tags, 0x18u, 40, (void (__thiscall *)(void *))bdTag::bdTag, bdTag::~bdTag);
            //    bdFileMetaData::resetArrays(this);
            resetArrays();
            //return this;
        }

        virtual ~bdFileMetaData() = default;

        // aislop 
        bdFileMetaData &operator=(const bdFileMetaData &that)
        {
            if (this == &that)
                return *this;

            // Base class
            bdTaskResult::operator=(that);

            // Scalars
            m_fileID = that.m_fileID;
            m_createTime = that.m_createTime;
            m_modifedTime = that.m_modifedTime;
            m_fileSize = that.m_fileSize;
            m_ownerID = that.m_ownerID;
            m_fileSlot = that.m_fileSlot;
            m_category = that.m_category;
            m_numTags = that.m_numTags;
            m_metaDataSize = that.m_metaDataSize;
            m_summaryFileSize = that.m_summaryFileSize;

            // Fixed-size arrays
            memcpy(m_ownerName, that.m_ownerName, sizeof(m_ownerName));
            memcpy(m_fileName, that.m_fileName, sizeof(m_fileName));
            memcpy(m_url, that.m_url, sizeof(m_url));
            memcpy(m_metaData, that.m_metaData, sizeof(m_metaData));

            // Tags
            for (unsigned int i = 0; i < 40; ++i)
            {
                m_tags[i] = that.m_tags[i];
            }

            return *this;
        }

};

struct demoConnectedPlayersInfo // sizeof=0x18
{                                                                             // XREF: demoMetaInfo/r
        unsigned __int8 playerIndex;                // XREF: Demo_InfoPlayerConnected(int)+21/w
                                                                                // Demo_InfoPlayerDisconnected(int)+42/r
        unsigned __int8 clientNum;                    // XREF: Demo_SetDefaultClient(int,int)+CC/r
                                                                                // Demo_InfoPlayerConnected(int)+30/w ...
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        unsigned __int64 xuid;                            // XREF: Demo_TagPlayers+55/r
                                                                                // Demo_TagPlayers+5C/r ...
        int connectTime;                                        // XREF: Demo_SetDefaultClient(int,int)+115/r
                                                                                // Demo_InfoPlayerConnected(int)+41/w ...
        int disconnectTime;                                 // XREF: Demo_InfoPlayerDisconnected(int)+AF/w
                                                                                // Demo_GetConnectionTimeInfoForPlayer(int,int *,int *)+6F/r ...
};

struct __declspec(align(4)) demoDefaultBookmarkInfo // sizeof=0xC
{                                                                             // XREF: demoMetaInfo/r
        unsigned __int8 type;                             // XREF: Demo_AddBookmark(int,int,int,int)+23/w
                                                                                // Demo_AddBookmark(int,int,int,int)+5C/r ...
        // padding byte
        // padding byte
        // padding byte
        int time;                                                     // XREF: Demo_AddBookmark(int,int,int,int)+32/w
                                                                                // Demo_GetNextDefaultBookmarkForPlayer(int,int,int *,int *,float (*)[4])+7B/r ...
        unsigned __int8 clientNum1;                 // XREF: Demo_AddBookmark(int,int,int,int)+41/w
                                                                                // Demo_AddBookmark(int,int,int,int)+6E/r ...
        unsigned __int8 clientNum2;                 // XREF: Demo_AddBookmark(int,int,int,int)+50/w
                                                                                // Demo_AddBookmark(int,int,int,int)+80/r ...
        // padding byte
        // padding byte
};

struct demoHeliPatchesInfo // sizeof=0x18
{                                                                             // XREF: demoMetaInfo/r
        int brushmodel;
        float origin[3];
        unsigned __int16 targetname;
        // padding byte
        // padding byte
        int enabled;
};

struct demoMetaInfo // sizeof=0x3918
{                                                                             // XREF: demoPlayback/r
                                                                                // demoMain/r
        bool type;                                                    // XREF: Demo_WriteGamestateToBuffer(int,int,msg_t *,int)+F8/w
        // padding byte
        // padding byte
        // padding byte
        int startTime;                                            // XREF: Demo_StreamingSuccessCallback(int,unsigned __int64)+233/r
                                                                                // Demo_ReadInfoData(char *,int)+183/o ...
        int endTime;                                                // XREF: Demo_StreamingSuccessCallback(int,unsigned __int64)+22D/r
                                                                                // Demo_End(bool)+2F/w ...
        char gameType[256];                                 // XREF: Demo_Screenshot_f(void)+B6/o
                                                                                // Demo_ReadInfoData(char *,int)+252/o ...
        char mapName[256];                                    // XREF: Demo_Screenshot_f(void)+D5/o
                                                                                // Demo_ReadInfoData(char *,int)+277/o ...
        char author[32];                                        // XREF: Demo_ReadInfoData(char *,int)+22D/o
                                                                                // Demo_GetAuthor(void)+3/o
        char name[22];                                            // XREF: Demo_StreamingSuccessCallback(int,unsigned __int64)+264/o
                                                                                // Demo_Screenshot_f(void)+63/o ...
        char description[22];                             // XREF: Demo_StreamingSuccessCallback(int,unsigned __int64)+278/o
                                                                                // Demo_Screenshot_f(void)+8C/o ...
        bool isModifiedName;
        bool isModifiedDescription;
        // padding byte
        // padding byte
        int week;                                                     // XREF: Demo_ReadInfoData(char *,int)+2DD/o
        int month;                                                    // XREF: Demo_ReadInfoData(char *,int)+2FF/o
                                                                                // Demo_GetTimeInfo(void)+4/r
        int year;                                                     // XREF: Demo_ReadInfoData(char *,int)+321/o
                                                                                // Demo_GetTimeInfo(void)+2F/r
        int day;                                                        // XREF: Demo_ReadInfoData(char *,int)+343/o
                                                                                // Demo_GetTimeInfo(void)+35/r
        int hour;                                                     // XREF: Demo_ReadInfoData(char *,int)+365/o
                                                                                // Demo_GetTimeInfo(void)+28/r
        int min;                                                        // XREF: Demo_ReadInfoData(char *,int)+387/o
                                                                                // Demo_GetTimeInfo(void)+21/r
        unsigned int createTime;                        // XREF: Demo_StreamingSuccessCallback(int,unsigned __int64)+247/r
        int numConnectedPlayersInfoCount;     // XREF: Demo_ReadInfoData(char *,int)+1C7/o
                                                                                // Demo_ReadInfoData(char *,int)+3C1/r ...
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        demoConnectedPlayersInfo connectedPlayers[50];
                                                                                // XREF: Demo_TagPlayers+55/r
                                                                                // Demo_TagPlayers+5C/r ...
        int numDefaultBookmarksCount;             // XREF: Demo_ReadInfoData(char *,int)+1E9/o
                                                                                // Demo_ReadInfoData(char *,int)+424/r ...
        demoDefaultBookmarkInfo defaultBookmarks[1000];
                                                                                // XREF: Demo_ReadInfoData(char *,int)+43E/o
                                                                                // Demo_AddBookmark(int,int,int,int)+23/w ...
        int numHeliPatchesCount;                        // XREF: Demo_RegisterHeliPatches(void)+4/r
                                                                                // Demo_RegisterHeliPatches(void)+23/r ...
        demoHeliPatchesInfo heliPatches[32];
                                                                                // XREF: Demo_RegisterHeliPatches(void)+31/o
                                                                                // Demo_ReadInfoData(char *,int)+4A2/o
};

enum fileShareSearchFileType : __int32
{                                                                             // XREF: ?Demo_SetTags@@YAXHPAHPAVbdTag@@PAUdemoMetaInfo@@W4fileShareSearchFileType@@@Z/r
                                                                                // ?Demo_SetTags@@YAXHPAHPAVbdTag@@PAUscreenshotMetaInfo@@W4fileShareSearchFileType@@@Z/r ...
        FILESHARE_FILETYPE_ALL                = 0x0,
        FILESHARE_FILETYPE_FILM             = 0x1,
        FILESHARE_FILETYPE_CLIP             = 0x2,
        FILESHARE_FILETYPE_SCREENSHOT = 0x3,
        FILESHARE_FILETYPE_CUSTOM_GAME_MODE = 0x4,
        FILESHARE_PUBLICFILES_START     = 0x8000,
        FILESHARE_FILETYPE_AVI                = 0x8000,
};

struct demoTagPlayers // sizeof=0xF8
{                                                                             // XREF: screenshotMetaInfo/r
                                                                                // clipSegment/r
        int count;
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        unsigned __int64 playerXuids[30];
};

struct __declspec(align(8)) screenshotMetaInfo // sizeof=0x348
{                                                                             // XREF: demoPlayback/r
        demoTagPlayers screenshotPlayers;
        char gameType[256];
        char mapName[256];
        char name[22];
        char description[22];
        bool isModifiedName;
        bool isModifiedDescription;
        // padding byte
        // padding byte
        int createTime;
        int week;
        int month;
        int year;
        int day;
        int hour;
        int min;
        // padding byte
        // padding byte
        // padding byte
        // padding byte
};

struct demoGameSettings // sizeof=0x6
{                                                                             // XREF: demoHeader/r
        bool onlinegame;                                        // XREF: Demo_StartRecord_f(void)+DF/w
                                                                                // Demo_IsPublicOnlineMatch(void):loc_9BDB33/r ...
        bool xblive_privatematch;                     // XREF: Demo_StartRecord_f(void)+F1/w
                                                                                // Demo_IsPublicOnlineMatch(void)+1E/r ...
        bool xblive_wagermatch;                         // XREF: Demo_StartRecord_f(void)+103/w
                                                                                // Demo_IsWagerMatch(void):loc_9BDBA3/r ...
        bool systemlink;                                        // XREF: Demo_StartRecord_f(void)+115/w
                                                                                // Demo_ReadHeader(void)+B0/w ...
        bool combatTraining;                                // XREF: Demo_StartRecord_f(void)+127/w
                                                                                // Demo_IsCombatTrainingMatch(void):loc_9BDB73/r
        bool customGameMode;                                // XREF: Demo_StartRecord_f(void)+131/w
                                                                                // Demo_IsCustomGameMode(void):loc_9BDBD3/r ...
};

struct __declspec(align(2)) demoHeader // sizeof=0x10
{                                                                             // XREF: demoMain/r
        int version;                                                // XREF: Demo_StartRecord_f(void)+BA/w
                                                                                // Demo_ReadHeader(void)+10/w ...
        int maxClients;                                         // XREF: Demo_StreamingSuccessCallback(int,unsigned __int64)+56/r
                                                                                // Demo_StartRecord_f(void)+CD/w ...
        bool isDemoClip;                                        // XREF: Demo_IsClipPlaying(void):loc_9BD6C3/r
                                                                                // Demo_ReadHeader(void)+50/w
        demoGameSettings settings;                    // XREF: Demo_StartRecord_f(void)+DF/w
                                                                                // Demo_StartRecord_f(void)+F1/w ...
        // padding byte
};

struct demoKeyboardData // sizeof=0x14
{                                                                             // XREF: demoMain/r
        int mode;                                                     // XREF: Demo_Keyboard_f(void)+6B/w
                                                                                // Demo_Keyboard_f(void):loc_9BAE82/r ...
        const char *title;                                    // XREF: Demo_Keyboard_f(void)+118/w
                                                                                // Demo_Keyboard_f(void)+154/w ...
        const char *defaultText;                        // XREF: Demo_Keyboard_f(void)+11D/w
                                                                                // Demo_Keyboard_f(void)+159/w ...
        int textSize;                                             // XREF: Demo_Keyboard_f(void)+127/w
                                                                                // Demo_Keyboard_f(void)+163/w ...
        int currentProcessingSegmentIndex;    // XREF: Demo_Keyboard_f(void)+F3/w
                                                                                // Demo_Keyboard_f(void)+106/w ...
};

struct ArchivedMatchState // sizeof=0xC
{                                                                             // XREF: MatchState/r
        int matchUIVisibilityFlags;                 // XREF: GScr_SetMatchFlag+287/r
                                                                                // GScr_SetMatchFlag+28D/w ...
        int bombTimer[2];                                     // XREF: GScr_SetBombTimer+65/w
                                                                                // GScr_SetBombTimer+9A/w
};

struct UnarchivedMatchState // sizeof=0x30
{                                                                             // XREF: MatchState/r
        int alliesScore;                                        // XREF: G_PopulateMatchState+12/w
        int axisScore;                                            // XREF: G_PopulateMatchState+1D/w
        int scoreLimit;                                         // XREF: G_PopulateMatchState+43/w
        int matchUIVisibilityFlags;                 // XREF: GScr_SetMatchFlag+2C4/r
                                                                                // GScr_SetMatchFlag+2CA/w ...
        scoreboardColumnType_t scoreboardColumnTypes[4];
                                                                                // XREF: GScr_SetScoreboardColumns+6B/w
                                                                                // GScr_SetScoreboardColumns+75/r
        float mapCenter[3];                                 // XREF: G_PopulateMatchState+50/w
                                                                                // G_PopulateMatchState+60/w ...
        unsigned int talkFlags;                         // XREF: GScr_SetMatchTalkFlag+FC/r
                                                                                // GScr_SetMatchTalkFlag+104/w ...
};

struct __declspec(align(64)) MatchState // sizeof=0x80
{                                                                             // XREF: snapshot_s/r
                                                                                // clientActive_t/r ...
        int index;                                                    // XREF: G_PopulateMatchState+3/w
        ArchivedMatchState archivedState;     // XREF: GScr_SetBombTimer+65/w
                                                                                // GScr_SetBombTimer+9A/w ...
        UnarchivedMatchState unarchivedState;
                                                                                // XREF: G_PopulateMatchState+12/w
                                                                                // G_PopulateMatchState+1D/w ...
        unsigned int pad[1];
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
};

struct demoMemBlock // sizeof=0xC008
{                                                                             // XREF: demoMain/r
        int compressedSize;                                 // XREF: Demo_Load(void)+5C/w
                                                                                // Demo_Load(void)+B4/r ...
        int uncompressedSize;                             // XREF: Demo_Load(void)+93/w
                                                                                // Demo_Load(void)+122/o ...
        unsigned __int8 compressedMsgBuf[49152]; // XREF: Demo_Load(void)+12/o
                                                                                // Demo_Load(void)+D4/o ...
};

struct buttonHoldCmd // sizeof=0xC
{                                                                             // XREF: demoPlayback/r
        bool active;
        // padding byte
        // padding byte
        // padding byte
        int key;
        int activatedTime;
};

struct demoKeyFrame // sizeof=0x30
{                                                                             // XREF: demoPlayback/r
                                                                                // demoPlayback/r
        int keyframeBufferStart;
        int demoFileMemoryBlockStart;
        int keyframeSnapshotTime;
        int keyframeClipServerTime;
        int firstCmdSequence;
        int size;
        msg_bookmark_t msgBookmark;
};

struct clipSegment // sizeof=0x120
{                                                                             // XREF: demoPlayback/r
        char name[22];
        // padding byte
        // padding byte
        int clipBufferIndex;
        int size;
        int transition;
        int duration;
        demoTagPlayers taggedPlayers;
};

struct __declspec(align(4)) demoPlayback // sizeof=0x12DEE0
{
        bool overridePause;
        // padding byte
        // padding byte
        // padding byte
        int transition;
        int transitionScreenTime;
        bool jumpTimeFlag;
        bool switchedPlayers;
        // padding byte
        // padding byte
        int metaInformationScreenTime;
        buttonHoldCmd holdCmd;
        int buttonPressTime[10];
        int screenshotSize;
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        screenshotMetaInfo screenshotInfo;
        bool displayPartyScoreboard;
        // padding byte
        // padding byte
        // padding byte
        msg_t keyframeMsg;
        int keyframeIndex;
        int keyframeBufferIndex;
        int lastProcessedCmd;
        bool keyframeGenerated;
        // padding byte
        // padding byte
        // padding byte
        int lastJumpedKeyframe;
        int keyframeJumpCount;
        demoKeyFrame keyFrame[250];
        int demoCmdInProgress;
        bool completed;
        bool hideGameHud;
        bool hideDemoHud;
        // padding byte
        int cameraMode;
        float clipCameraOrigin[3];
        float clipCameraAngles[3];
        float prevClipCameraOrigin[3];
        float prevClipCameraAngles[3];
        bool clipPausedState;
        // padding byte
        // padding byte
        // padding byte
        msg_t clipRecordingMsg;
        int prevClipRecordBufIndex;
        int clipRecordBufIndex;
        unsigned __int8 mainClipRecordingBuf[1048576];
        unsigned __int8 subClipRecordingBuf[49152];
        playerState_s clipRecordPS;
        unsigned __int8 clipRecordClient;
        // padding byte
        // padding byte
        // padding byte
        float clipRecordTimescale;
        unsigned __int8 clipCameraMode;
        bool clipHudHidden;
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        demoMetaInfo clipRecordInfo;
        bool forceWriteClipCommands;
        // padding byte
        // padding byte
        // padding byte
        int clipState;
        int clipTime;
        int clipServerTime;
        int prevClipTime;
        int originalFilmTime;
        bool clipModified;
        bool pendingSegmentSave;
        bool previewAllSegments;
        bool cancelPreview;
        bool forceTeleportClipCamera;
        // padding byte
        // padding byte
        // padding byte
        clipSegment segments[10];
        int segmentCount;
        int segmentGameStateSize;
        int defaultSegmentTag;
        int clipPreviewBufferIndex;
        int clipPreviewSegmentNumber;
        unsigned __int8 demoPreviewBuf[2][49152];
        demoKeyFrame demoPreview[2];
        bool demoCompleteStateForRestorationPoint[2];
        // padding byte
        // padding byte
};

struct demoProfile // sizeof=0x40
{                                                                             // XREF: demoMain/r
        int memUsed[8];                                         // XREF: Demo_RecordProfileData(int,int)+6/r
                                                                                // Demo_RecordProfileData(int,int)+13/w ...
        int count[8];                                             // XREF: Demo_RecordProfileData(int,int)+1D/r
                                                                                // Demo_RecordProfileData(int,int)+2A/w ...
};


struct __declspec(align(128)) demoMain // sizeof=0xB0B00
{                                                                             // XREF: .data:demoMain demo/r
        char demoName[128];
        int demoFileHandle;                                 // XREF: Demo_Forward_f(void)+329/r
                                                                                // Demo_Back_f(void)+2F2/r ...
        int state;                                                    // XREF: Demo_SetDemoState(int)+6/w
                                                                                // Demo_GetDemoState(void)+3/r
        msg_t msg;                                                    // XREF: Demo_StartRecord_f(void)+1D0/o
                                                                                // Demo_Forward_f(void)+351/o ...
        unsigned __int8 msgBuf0[49152];         // XREF: Demo_Init(void)+78/o
                                                                                // Demo_Load(void)+24/o ...
        unsigned __int8 msgBuf1[49152];         // XREF: Demo_Forward_f(void)+39C/o
                                                                                // Demo_Forward_f(void)+3DB/o ...
        bool msgBufFlag;                                        // XREF: Demo_StartSaveProcess(void)+56/r
                                                                                // Demo_GetActiveMemoryBuffer(void)+4/r ...
        // padding byte
        // padding byte
        // padding byte
        int totalSize;                                            // XREF: Demo_Forward_f(void)+30E/w
                                                                                // Demo_Back_f(void)+2D7/w ...
        int sessionFlags;
        demoHeader header;                                    // XREF: Demo_StreamingSuccessCallback(int,unsigned __int64)+56/r
                                                                                // Demo_StartRecord_f(void)+BA/w ...
        demoKeyboardData keyboard;                    // XREF: Demo_Keyboard_f(void)+6B/w
                                                                                // Demo_Keyboard_f(void):loc_9BAE82/r ...
        bool shutdownInProgress;                        // XREF: Demo_End(bool)+C/w
                                                                                // Demo_End(bool):loc_9BB9B2/w ...
        // padding byte
        // padding byte
        // padding byte
        int lastProcessedTime;                            // XREF: Demo_Init(void)+B9/w
                                                                                // Demo_ShouldGenerateKeyFrameSnapshot(int,int):loc_9C3C83/r ...
        int lastProcessedMsgNum;                        // XREF: Demo_Init(void)+C3/w
                                                                                // Demo_ReadDemoMessage(int)+68B/w ...
        int lastReliableCommandRecorded[32];
                                                                                // XREF: Demo_StartRecord_f(void)+24A/w
                                                                                // Demo_ClientConnected(int)+6/w ...
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        MatchState matchState;                            // XREF: Demo_ResetSnapshotData(void)+1E/o
                                                                                // Demo_ParseSnapshot(int,msg_t *)+1E4/o ...
        playerState_s ps[32];                             // XREF: Demo_SwitchPlayer_f(void)+181/o
                                                                                // Demo_SwitchPlayer(int,bool,int,bool)+151/r ...
        entityState_s ent[1024];                        // XREF: Demo_Init(void)+91/o
                                                                                // Demo_HasClientSpawnedIn(int)+C/r ...
        int entClientMask[1024][2];                 // XREF: Demo_ShouldEntityBeVisibleToClient(int)+28/r
                                                                                // Demo_ResetSnapshotData(void)+46/o ...
        int prevNumEntities;                                // XREF: Demo_ReadPacketEntities(int,msg_t *,int,int)+8F/w
                                                                                // Demo_GenerateUncompressedSnapshot(int,msg_t *,int,int,int,int,int,bool)+366/r ...
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        clientState_s client[32];                     // XREF: Demo_Init(void)+A5/o
                                                                                // Demo_SwitchPlayer(int,bool,int,bool)+B9/r ...
        int prevNumClients;                                 // XREF: Demo_ReadPacketClients(int,msg_t *,int,int)+88/w
                                                                                // Demo_GenerateUncompressedSnapshot(int,msg_t *,int,int,int,int,int,bool)+550/r ...
        int prevNumParsedEntities;                    // XREF: Demo_ResetSnapshotData(void)+67/w
                                                                                // Demo_ReadPacketEntities(int,msg_t *,int,int)+5D/r ...
        int prevNumParsedClients;                     // XREF: Demo_ResetSnapshotData(void)+71/w
                                                                                // Demo_ReadPacketClients(int,msg_t *,int,int)+56/r ...
        int playerSessionState[32];                 // XREF: Demo_RecordPlayerSessionState(int,int)+9/w
                                                                                // Demo_IsPlayerSpectating(int)+7/r ...
        float intermissionPointOrigin[3];     // XREF: Demo_SetIntermissionPoint(float * const,float * const)+A/w
                                                                                // Demo_SetIntermissionPoint(float * const,float * const)+1A/w ...
        float intermissionPointAngles[3];     // XREF: Demo_SetIntermissionPoint(float * const,float * const)+39/w
                                                                                // Demo_SetIntermissionPoint(float * const,float * const)+49/w ...
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        demoMetaInfo info;                                    // XREF: Demo_StreamingSuccessCallback(int,unsigned __int64)+1C1/o
                                                                                // Demo_StreamingSuccessCallback(int,unsigned __int64)+22D/r ...
        bool renderingMovie;                                // XREF: Demo_Play_f(void)+1E2/w
                                                                                // Demo_End(bool)+BD/w ...
        // padding byte
        // padding byte
        // padding byte
        demoMemBlock memBlock;                            // XREF: Demo_Load(void)+12/o
                                                                                // Demo_Load(void)+5C/w ...
        demoPlayback *playback;                         // XREF: Demo_SetTags(int,int *,bdTag *,screenshotMetaInfo *,fileShareSearchFileType):loc_9B7624/r
                                                                                // Demo_SetTags(int,int *,bdTag *,screenshotMetaInfo *,fileShareSearchFileType)+104/r ...
        bool playbackInited;                                // XREF: Demo_End(bool)+F9/w
                                                                                // Demo_IsPlaybackInited(void)+3/r ...
        // padding byte
        // padding byte
        // padding byte
        demoProfile profile;                                // XREF: Demo_RecordProfileData(int,int)+6/r
                                                                                // Demo_RecordProfileData(int,int)+13/w ...
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
};

enum fileShareLocation : __int32;

struct dwFileShareReadFileTask // sizeof=0x878
{                                                                             // XREF: playerFileOperations/r
        fileShareLocation location;
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        unsigned __int64 fileID;
        unsigned int fileSize;
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        bdFileMetaData descriptor;
        const char *menuDef;
        void *buffer;
        void *cacheBuffer;
        unsigned int bufferSize;
        unsigned int bytesDownloaded;
        bdDownloadInterceptor *downloadHandler;
        bool loadedFromCache;
        // padding byte
        // padding byte
        // padding byte
        void (__cdecl *dataCallback)(void *, unsigned int, unsigned int, unsigned int);
        void (__cdecl *successCallback)(dwFileShareReadFileTask *);
        void (__cdecl *failureCallback)(dwFileShareReadFileTask *);
};

struct __declspec(align(8)) demoRecordedFileUploadInfo // sizeof=0x5D8
{
    unsigned __int64 fileID;
    char metaData[512];                 // XREF: Demo_StreamingSuccessCallback(int,unsigned __int64)+205/o
                                        // Demo_StreamingSuccessCallback(int,unsigned __int64)+220/o ...
    int metaDataSize;                   // XREF: Demo_StreamingSuccessCallback(int,unsigned __int64)+25A/w
                                        // Demo_StreamingSuccessCallback(int,unsigned __int64)+2BD/r
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    bdTag tags[40];                     // XREF: Demo_StreamingSuccessCallback(int,unsigned __int64)+1EB/o
                                        // _dynamic_initializer_for__s_demoUploadInfo__+D/o
    int numTags;                        // XREF: Demo_StreamingSuccessCallback(int,unsigned __int64)+1F8/w
    // padding byte
    // padding byte
    // padding byte
    // padding byte

    ~demoRecordedFileUploadInfo() = default;
};

struct demoClient_s // sizeof=0x4
{                                       // XREF: .data:demoClient_s demoClient/r
    int state;                          // XREF: Demo_SetDemoClientState(int)+6/w
};

void __cdecl Demo_RegisterDvars();
void __cdecl Demo_RegisterCommands();
void __cdecl Demo_AddDemoClient_f();
void __cdecl Demo_RemoveDemoClient_f();
void __cdecl Demo_SetTags(
                int controllerIndex,
                int *numTags,
                bdTag *tags,
                demoMetaInfo *info,
                fileShareSearchFileType fileType);
void __cdecl Demo_SetTags(
                int controllerIndex,
                int *numTags,
                bdTag *tags,
                screenshotMetaInfo *info,
                fileShareSearchFileType fileType);
char __cdecl Demo_SetMetaData(
                int controllerIndex,
                char *metaData,
                int *metaDataSize,
                demoMetaInfo *dInfo,
                screenshotMetaInfo *sInfo,
                fileShareSearchFileType fileType);
void __cdecl Demo_StreamingSuccessCallback(int controllerIndex, unsigned __int64 fileID);
void __cdecl Demo_StreamingFailureCallback();
void __cdecl Demo_StartRecord_f();
void __cdecl Demo_StopRecord_f();
void __cdecl Demo_Play_f();
void __cdecl Demo_Stop_f();
void __cdecl Demo_Forward_f();
void __cdecl Demo_Back_f();
void __cdecl Demo_JumpToStart_f();
void __cdecl Demo_Pause_f();
void __cdecl Demo_TimeScale_f();
void __cdecl Demo_ToggleGameHud_f();
void __cdecl Demo_ToggleDemoHud_f();
void __cdecl Demo_SwitchPlayer_f();
void __cdecl Demo_SwitchCamera_f();
void __cdecl Demo_StartClipRecord_f();
void __cdecl Demo_PauseClipRecord_f();
void __cdecl Demo_PreviewClip_f();
void __cdecl Demo_DeleteClip_f();
void __cdecl Demo_SaveAndUploadClip_f();
void __cdecl Demo_SaveSegment_f();
void __cdecl Demo_MoveSegment_f();
void __cdecl Demo_DeleteSegment_f();
void __cdecl Demo_PreviewSegment_f();
void __cdecl Demo_SwitchDefaultSegmentTag_f();
void __cdecl Demo_SwitchTransition_f();
void __cdecl Demo_Keyboard_f();
void    Demo_Screenshot_f();
void __cdecl Demo_SaveScreenshot_f();
void __cdecl Demo_RenderMovie_f();
void __cdecl Demo_SwitchControls_f();
bool __cdecl Demo_IsEnabled();
void __cdecl Demo_SetDemoState(int state);
int __cdecl Demo_GetDemoState();
void __cdecl Demo_SetDemoClientState(int state);
int __cdecl Demo_GetDemoClientState();
bool __cdecl Demo_IsIdle();
bool __cdecl Demo_IsRecording();
bool __cdecl Demo_IsPlaying();
demoMain *__cdecl Demo_GetDemoName();
void __cdecl Demo_Printf(int channel, const char *string);
void __cdecl Demo_Init();
bool __cdecl Demo_InitWrite();
bool __cdecl Demo_InitRead();
void __cdecl Demo_Frame(int msec);
void __cdecl Demo_End(bool abnormalTermination);
void __cdecl Demo_ReadDataCallback(char *data, unsigned int dataSize);
void __cdecl Demo_ReadDataSuccessPlayIt(dwFileShareReadFileTask *task);
void __cdecl Demo_ReadDataSuccessRenderIt(dwFileShareReadFileTask *task);
void __cdecl Demo_ReadDataFailure();
void __cdecl Demo_DownloadFile(
                int controllerIndex,
                char *filmName,
                unsigned __int64 fileId,
                int fileSize,
                bool isUserFile,
                bool renderIt);
//void __thiscall demoRecordedFileUploadInfo::~demoRecordedFileUploadInfo(demoRecordedFileUploadInfo *this);


extern const dvar_t *demo_enabled;
extern const dvar_t *demo_recordBasicTraining;
extern const dvar_t *demo_recordPrivateMatch;
extern const dvar_t *demo_filesizeLimit;
extern const dvar_t *demo_debug;
extern const dvar_t *demo_drawdebuginformation;
extern const dvar_t *demo_errortitle;
extern const dvar_t *demo_errormessage;
extern const dvar_t *demo_client;
extern const dvar_t *demo_recordingrate;
extern const dvar_t *demo_keyframerate;
extern const dvar_t *demo_pause;
extern const dvar_t *demo_usefilesystem;
extern const dvar_t *demo_save_smp;
extern const dvar_t *demo_timescale;
extern const dvar_t *demo_timeScaleRate;
extern const dvar_t *demo_cmdNum;
extern const dvar_t *demo_bookmarkEventThresholdTime;
extern const dvar_t *demo_controllerConfig;
extern const dvar_t *demo_defaultSegmentTag;
extern const dvar_t *demo_selectedSegmentIndex;
extern const dvar_t *demo_desiredTime;
extern const dvar_t *demo_desiredClient;
extern const dvar_t *demo_packetsPerSecondMin;
extern const dvar_t *demo_bytesPerSecondMin;
extern const dvar_t *demo_packetsPerSecondMax;
extern const dvar_t *demo_bytesPerSecondMax;

extern demoMain demo;
extern ddlState_t g_fileShareRootState;
extern ddlDef_t *g_fileshareDDL;

extern unsigned __int8 *g_keyframeBuf;

extern entityState_s g_defaultEntityState;
extern clientState_s g_defaultClientState;