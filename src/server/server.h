#pragma once
#include <qcommon/msg_mp.h>
#include <bgame/bg_local.h>
#include <qcommon/net_chan_mp.h>
#include <server_mp/sv_snapshot_mp.h>
#include <server_mp/sv_voice_mp.h>

#ifdef KISAK_DEMON 
#define KISAK_STATS
#endif

#define ALL_STATS_PACKETS_RECEIVED 0x7FFFFFFFF

struct gentity_s;

enum SignonState // LWSS: not a real enum name!
{
    CS_FREE = 0,
    CS_ZOMBIE = 1,
    CS_RECONNECTING = 2,
    CS_CONNECTED = 3,
    CS_CLIENTLOADING = 4,
    CS_ACTIVE = 5,
};

struct PredictedVehicleInfo // sizeof=0x34
{                                                                             // XREF: clientActive_t/r
                                                                                // clientHeader_t/r
    bool inVehicle;
    // padding byte
    // padding byte
    // padding byte
    float origin[3];
    float angles[3];
    float tVel[3];
    float aVel[3];
};

struct clientHeader_t // sizeof=0x714
{                                       // XREF: client_t/r
    //int state;
    SignonState state;
    int sendAsActive;
    int deltaMessage;
    int rateDelayed;
    netchan_t netchan;
    float predictedOrigin[3];
    int predictedOriginServerTime;
    PredictedVehicleInfo vehicle;
};

struct svscmd_info_t // sizeof=0xC
{                                       // XREF: client_t/r
    char *cmd;
    int time;
    int type;
};

struct client_t // sizeof=0x84D00
{
    clientHeader_t header;
    const char *dropReason;
    char userinfo[1024];
    char reliableCommandBuffer[16384];
    int reliableCommandBufferNext;
    svscmd_info_t reliableCommandInfo[128];
    int reliableSequence;
    int reliableAcknowledge;
    int reliableSent;
    int messageAcknowledge;
    int gamestateMessageNum;
    int challenge;
    usercmd_s lastUsercmd;
    int lastClientCommand;
    char lastClientCommandString[1024];
    gentity_s *gentity;
    char name[32];
    char clanAbbrev[5];
    // padding byte
    // padding byte
    // padding byte
    unsigned __int64 xuid;
    int downloading;
    char downloadName[64];
    int download;
    int downloadSize;
    int downloadCount;
    int downloadClientBlock;
    int downloadCurrentBlock;
    int downloadXmitBlock;
    unsigned __int8 *downloadBlocks[8];
    int downloadBlockSize[8];
    int downloadEOF;
    int downloadSendTime;
    char downloadURL[256];
    int wwwOk;
    int downloadingWWW;
    int clientDownloadingWWW;
    int wwwFallback;
    int nextReliableTime;
    int nextReliableCount;
    char reservedSlot;
    // padding byte
    // padding byte
    // padding byte
    int lastPacketTime;
    int lastConnectTime;
    int nextSnapshotTime;
    int lastSnapshotTime;
    int timeoutCount;
    clientSnapshot_t frames[32];
    int ping;
    int rate;
    int snapshotMsec;
    int pureAuthentic;
    char netchanOutgoingBuffer[65536];
    char netchanIncomingBuffer[2048];
    int guid;
    unsigned int scriptId;
    bool bIsSplitscreenClient;
    // padding byte
    // padding byte
    // padding byte
    int bIsTestClient;
    int bIsDemoClient;
    int serverId;
    VoicePacket_t voicePackets[40];
    int voicePacketCount;
    bool muteList[32];
    bool sendVoice;
    unsigned __int8 stats[40960];
    unsigned __int8 purchasedItems[32];
    unsigned __int8 globalStats[40960];
    unsigned __int8 globalStatsStable[40960];
    unsigned __int8 modifiedStatBytes[5120];
    // padding byte
    // padding byte
    int statsSentIndex;
    int statsModified;
    __int64 statPacketsReceived; // ctrl-f "KISAK_STATS"
    int statsValidated;
    char PBguid[33];
    char clientPBguid[33];
    bool steamAuthorized;
    unsigned __int8 steamAuthFailCount;
    unsigned __int64 steamID;
    bool tempPacketDebugging;
    // padding byte
    // padding byte
    // padding byte
    int snapshotHistoryTime;
    int snapshotHistoryFrames;
    int messageHistorySize;
    unsigned __int64 dw_userID;
    int notifyJoin;
    unsigned int dwchallenge;
};