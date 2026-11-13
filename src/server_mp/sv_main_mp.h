#pragma once
#include <qcommon/msg_mp.h>
#include <server/server.h>
#include <client_mp/client_mp.h>

enum serverState_t : __int32
{                                       // XREF: server_t/r
    SS_DEAD    = 0x0,
    SS_LOADING = 0x1,
    SS_GAME    = 0x2,
};

enum svscmd_type : __int32
{                                       // XREF: ?SV_AddServerCommand@@YAXPAUclient_t@@W4svscmd_type@@PBD@Z/r
                                        // ?SV_SendServerCommand@@YAXPAUclient_t@@W4svscmd_type@@PBDZZ/r ...
    SV_CMD_CAN_IGNORE = 0x0,
    SV_CMD_RELIABLE   = 0x1,
};

struct cachedSnapshot_t // sizeof=0x24
{
    int archivedFrame;
    int time;
    int physicsTime;
    int matchState;
    int num_entities;
    int first_entity;
    int num_clients;
    int first_client;
    int usesDelta;
};

struct archivedSnapshot_s // sizeof=0x8
{                                       // XREF: archivedSnapshot_t/r
    int start;
    int size;
};

struct __declspec(align(8)) cachedClient_s // sizeof=0x2780
{                                       // XREF: cachedClient_t/r
    int playerStateExists;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    clientState_s cs;
    playerState_s ps;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct __declspec(align(4)) challenge_t // sizeof=0x70
{                                       // XREF: serverStatic_t/r
    netadr_t adr;
    int challenge;                      // XREF: SV_DirectConnect(netadr_t)+325/r
    int time;
    int pingTime;                       // XREF: SV_DirectConnect(netadr_t)+3CF/r
    int firstTime;
    int firstPing;                      // XREF: SV_DirectConnect(netadr_t)+3BA/r
                                        // SV_DirectConnect(netadr_t)+3E7/w ...
    int connected;                      // XREF: SV_DirectConnect(netadr_t)+446/w
    int guid;                           // XREF: SV_DirectConnect(netadr_t)+333/r
    uint64_t steamID64; // LWSS ADD 
    char PBguid[33];                    // XREF: SV_DirectConnect(netadr_t)+344/o
    char clientPBguid[33];              // XREF: SV_DirectConnect(netadr_t)+362/o
    // padding byte
    // padding byte
};

struct tempBanSlot_t // sizeof=0x8
{                                       // XREF: serverStatic_t/r
    int guid;                           // XREF: SV_IsTempBannedGuid(int)+29/r
                                        // SV_BanGuidBriefly(int)+12/w ...
    int banTime;                        // XREF: SV_IsTempBannedGuid(int)+40/r
                                        // SV_BanGuidBriefly(int)+21/w ...
};

struct __declspec(align(128)) serverStatic_t // sizeof=0x1C880
{                                       // XREF: .data:serverStatic_t svs/r
    cachedSnapshot_t *cachedSnapshotFrames;
                                        // XREF: SV_AllocateClientMemory(HunkUser *,int,int)+147/w
                                        // SV_AllocateClientMemory(HunkUser *,int,int)+153/r ...
    archivedEntity_s *cachedSnapshotEntities;
                                        // XREF: SV_AllocateClientMemory(HunkUser *,int,int)+1D1/w
                                        // SV_AllocateClientMemory(HunkUser *,int,int)+1E2/r ...
    int initialized;                    // XREF: SV_Startup(int)+3/r
                                        // SV_Startup(int):loc_7102B8/w
    int time;                           // XREF: SV_BotSetScriptEnemy(client_t const *,gentity_s const *)+85/r
                                        // SV_BotPressUseButton(client_t const *,int)+50/r ...
    int physicsTime;                    // XREF: SV_SetServerStaticHeader(void)+1F/w
                                        // SV_SetServerStaticHeader(void)+25/r ...
    int oldestArchiveTime;              // XREF: SV_GetEarliestArchivedClientInfoTime(void)+D6/w
                                        // SV_GetEarliestArchivedClientInfoTime(void)+DB/r
    int snapFlagServerBit;              // XREF: SV_MapRestart(int)+23A/r
                                        // SV_MapRestart(int)+243/w ...
    client_t *clients;                  // XREF: MatchRecord_SearchCreateForPlayer+180/r
                                        // MatchRecordMovement(void):loc_5AD9B7/r ...
    int numCachedSnapshotMatchStates;   // XREF: SV_AllocateClientMemory(HunkUser *,int,int)+165/w
                                        // SV_AllocateClientMemory(HunkUser *,int,int)+171/r ...
    int numCachedSnapshotEntities;      // XREF: SV_AllocateClientMemory(HunkUser *,int,int)+1AC/w
                                        // SV_AllocateClientMemory(HunkUser *,int,int)+1BC/r ...
    int numCachedSnapshotClients;       // XREF: SV_AllocateClientMemory(HunkUser *,int,int)+221/w
                                        // SV_AllocateClientMemory(HunkUser *,int,int)+22D/r ...
    int numSnapshotMatchStates;         // XREF: SV_AllocateClientMemory(HunkUser *,int,int)+48/w
                                        // SV_AllocateClientMemory(HunkUser *,int,int)+55/r ...
    int numSnapshotEntities;            // XREF: SV_AllocateClientMemory(HunkUser *,int,int)+94/w
                                        // SV_AllocateClientMemory(HunkUser *,int,int)+A0/r ...
    int numSnapshotClients;             // XREF: SV_AllocateClientMemory(HunkUser *,int,int)+E5/w
                                        // SV_AllocateClientMemory(HunkUser *,int,int)+F2/r ...
    int nextSnapshotMatchStates;        // XREF: SV_Netchan_OutgoingSequenceIncremented+35/r
                                        // SV_BuildClientSnapshot(client_t *)+8B/r ...
    int nextSnapshotEntities;           // XREF: SV_SpawnServer(int,char const *,int,int)+4AA/w
                                        // SV_CheckOverflow+6E/r ...
    int nextSnapshotClients;            // XREF: SV_SpawnServer(int,char const *,int,int)+4B4/w
                                        // SV_CheckOverflow+276/r ...
    struct MatchState *snapshotMatchStates;    // XREF: SV_AllocateClientMemory(HunkUser *,int,int)+6B/w
                                        // SV_AllocateClientMemory(HunkUser *,int,int)+7C/r ...
    entityState_s *snapshotEntities;    // XREF: SV_AllocateClientMemory(HunkUser *,int,int)+B9/w
                                        // SV_AllocateClientMemory(HunkUser *,int,int)+CC/r ...
    clientState_s *snapshotClients;     // XREF: SV_AllocateClientMemory(HunkUser *,int,int)+10A/w
                                        // SV_AllocateClientMemory(HunkUser *,int,int)+11E/r ...
    int nextArchivedSnapshotFrames;     // XREF: SV_InitArchivedSnapshot(void)+3/w
                                        // SV_CheckOverflow:loc_715D53/r ...
    archivedSnapshot_s *archivedSnapshotFrames;
                                        // XREF: SV_AllocateClientMemory(HunkUser *,int,int)+280/w
                                        // SV_AllocateClientMemory(HunkUser *,int,int)+28C/r ...
    unsigned __int8 *archivedSnapshotBuffer;
                                        // XREF: SV_AllocateClientMemory(HunkUser *,int,int)+2B2/w
                                        // SV_AllocateClientMemory(HunkUser *,int,int)+2BE/r ...
    int nextArchivedSnapshotBuffer;     // XREF: SV_InitArchivedSnapshot(void)+D/w
                                        // SV_CheckOverflow:loc_715DA7/r ...
    int nextCachedSnapshotMatchStates;  // XREF: SV_InitArchivedSnapshot(void)+35/w
                                        // SV_GetCachedSnapshotInternal:loc_719F61/r ...
    int nextCachedSnapshotEntities;     // XREF: SV_InitArchivedSnapshot(void)+17/w
                                        // SV_CheckOverflow+C9/r ...
    int nextCachedSnapshotClients;      // XREF: SV_InitArchivedSnapshot(void)+21/w
                                        // SV_CheckOverflow+124/r ...
    int nextCachedSnapshotFrames;       // XREF: SV_InitArchivedSnapshot(void)+2B/w
                                        // SV_CheckOverflow:loc_715DFB/r ...
    MatchState *cachedSnapshotMatchStates;
                                        // XREF: SV_AllocateClientMemory(HunkUser *,int,int)+187/w
                                        // SV_AllocateClientMemory(HunkUser *,int,int)+197/r ...
    cachedClient_s *cachedSnapshotClients;
                                        // XREF: SV_AllocateClientMemory(HunkUser *,int,int)+246/w
                                        // SV_AllocateClientMemory(HunkUser *,int,int)+259/r ...
    int nextHeartbeatTime;              // XREF: SV_Heartbeat_f(void)+3/w
                                        // SV_MasterHeartbeat(int,char const *)+849/r ...
    int nextStatusResponseTime;
    challenge_t challenges[1024];       // XREF: SV_GetChallenge(netadr_t)+58/o
                                        // SV_GetChallenge(netadr_t)+106/o ...
    netadr_t redirectAddress;           // XREF: SV_FlushRedirect(char *)+CE/o
                                        // SV_FlushRedirect(char *)+133/o ...
    netProfileInfo_t OOBProf;           // XREF: SV_Netchan_AddOOBProfilePacket(int)+E/o
                                        // SV_Netchan_AddOOBProfilePacket(int)+21/o
    tempBanSlot_t tempBans[16];         // XREF: SV_IsTempBannedGuid(int)+29/r
                                        // SV_IsTempBannedGuid(int)+40/r ...
    unsigned __int64 xuids[32];         // XREF: PlayerCmd_IsLocalToHost+154/r
                                        // PlayerCmd_IsLocalToHost+15E/r ...
    float mapCenter[3];                 // XREF: G_PopulateMatchState+48/r
                                        // G_PopulateMatchState+58/r ...
    int dataSentLastSec;
    int activeEntities;                 // XREF: SV_BuildClientSnapshot(client_t *)+861/w
    int archivedEntityCount;            // XREF: SV_SetServerStaticHeader(void)+163/r
                                        // SV_GetServerStaticHeader(void)+4E7/w
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

struct serverStaticHeader_t // sizeof=0xA0
{                                       // XREF: .data:serverStaticHeader_t svsHeader/r
    client_t *clients;                  // XREF: SV_ArchiveSnapshot(msg_t *)+380/r
                                        // SV_ArchiveSnapshot(msg_t *)+BCF/r ...
    int time;                           // XREF: Com_Frame_Try_Block_Function+549/r
                                        // SV_ArchiveSnapshot(msg_t *)+287/r ...
    int physicsTime;                    // XREF: SV_ArchiveSnapshot(msg_t *)+299/r
                                        // SV_ArchiveSnapshot(msg_t *)+A5C/r ...
    int snapFlagServerBit;              // XREF: SV_WriteSnapshotToClient(client_t *,msg_t *)+2C6/r
                                        // SV_SetServerStaticHeader(void)+35/w ...
    int numCachedSnapshotMatchStates;   // XREF: SV_ArchiveSnapshot(msg_t *)+2BF/r
                                        // SV_ArchiveSnapshot(msg_t *)+B3A/r ...
    int numCachedSnapshotEntities;      // XREF: SV_ArchiveSnapshot(msg_t *)+23E/r
                                        // SV_ArchiveSnapshot(msg_t *)+F3F/r ...
    int numCachedSnapshotClients;       // XREF: SV_ArchiveSnapshot(msg_t *)+256/r
                                        // SV_ArchiveSnapshot(msg_t *)+3CD/r ...
    int numSnapshotMatchStates;         // XREF: SV_WriteSnapshotToClient(client_t *,msg_t *)+362/r
                                        // SV_WriteSnapshotToClient(client_t *,msg_t *)+38A/r ...
    int numSnapshotEntities;            // XREF: SV_WriteSnapshotToClient(client_t *,msg_t *)+216/r
                                        // SV_EmitPacketEntities_GetOldEnt+26/r ...
    int numSnapshotClients;             // XREF: SV_EmitPacketClients+19A/r
                                        // SV_EmitPacketClients_GetOldClient+26/r ...
    int nextSnapshotMatchStates;        // XREF: SV_SetServerStaticHeader(void)+85/w
                                        // SV_GetServerStaticHeader(void):loc_71DBBF/r
    int nextSnapshotEntities;           // XREF: SV_WriteSnapshotToClient(client_t *,msg_t *)+211/r
                                        // SV_SetServerStaticHeader(void)+90/w ...
    MatchState *snapshotMatchStates;    // XREF: SV_WriteSnapshotToClient(client_t *,msg_t *)+36B/r
                                        // SV_WriteSnapshotToClient(client_t *,msg_t *)+393/r ...
    entityState_s *snapshotEntities;    // XREF: SV_EmitPacketEntities_GetOldEnt+32/r
                                        // SV_SetServerStaticHeader(void)+A7/w ...
    clientState_s *snapshotClients;     // XREF: SV_EmitPacketClients_GetOldClient+32/r
                                        // SV_SetServerStaticHeader(void)+B2/w ...
    struct svEntity_s *svEntities;             // XREF: SV_ArchiveSnapshot(msg_t *)+7B9/r
                                        // SV_ArchiveSnapshot(msg_t *)+812/r ...
    float mapCenter[3];                 // XREF: CL_WriteUncompressedDemoInfo(int)+762/w
                                        // CL_WriteUncompressedDemoInfo(int)+772/w ...
    MatchState *cachedSnapshotMatchStates;
                                        // XREF: SV_ArchiveSnapshot(msg_t *)+2C8/r
                                        // SV_ArchiveSnapshot(msg_t *)+B43/r ...
    archivedEntity_s *cachedSnapshotEntities;
                                        // XREF: SV_ArchiveSnapshot(msg_t *):loc_6F6AE0/r
                                        // SV_ArchiveSnapshot(msg_t *)+F48/r ...
    cachedClient_s *cachedSnapshotClients;
                                        // XREF: SV_ArchiveSnapshot(msg_t *):loc_6F6B0F/r
                                        // SV_ArchiveSnapshot(msg_t *)+3D9/r ...
    unsigned __int8 *archivedSnapshotBuffer;
                                        // XREF: SV_ArchiveSnapshot(msg_t *):loc_6F6B3E/r
                                        // SV_SetServerStaticHeader(void)+119/w ...
    cachedSnapshot_t *cachedSnapshotFrames;
                                        // XREF: SV_ArchiveSnapshot(msg_t *):loc_6F6B6D/r
                                        // SV_ArchiveSnapshot(msg_t *)+1C1/r ...
    int nextCachedSnapshotFrames;       // XREF: SV_ArchiveSnapshot(msg_t *):loc_6F6BD9/r
                                        // SV_ArchiveSnapshot(msg_t *)+1CD/r ...
    int nextArchivedSnapshotFrames;     // XREF: SV_ArchiveSnapshot(msg_t *):loc_6F6BF5/r
                                        // SV_ArchiveSnapshot(msg_t *)+AB2/r ...
    int nextCachedSnapshotMatchStates;  // XREF: SV_ArchiveSnapshot(msg_t *)+AC0/r
                                        // SV_ArchiveSnapshot(msg_t *)+B34/r ...
    int nextCachedSnapshotEntities;     // XREF: SV_ArchiveSnapshot(msg_t *):loc_6F6C88/r
                                        // SV_ArchiveSnapshot(msg_t *)+ADB/r ...
    int nextCachedSnapshotClients;      // XREF: SV_ArchiveSnapshot(msg_t *):loc_6F6CA1/r
                                        // SV_ArchiveSnapshot(msg_t *)+AF6/r ...
    int archivedEntityCount;            // XREF: SV_ArchiveSnapshot(msg_t *)+1B7/w
                                        // SV_ArchiveSnapshot(msg_t *)+9BC/r ...
    int num_entities;                   // XREF: SV_ArchiveSnapshot(msg_t *)+687/r
                                        // SV_ArchiveSnapshot(msg_t *)+D74/r ...
    int maxclients;                     // XREF: SV_ArchiveSnapshot(msg_t *)+301/r
                                        // SV_ArchiveSnapshot(msg_t *)+C04/r ...
    int fps;                            // XREF: SV_ArchiveSnapshot(msg_t *)+1AB/r
                                        // SV_SetServerStaticHeader(void)+190/w ...
    int clientArchive;                  // XREF: MSG_WriteDeltaPlayerstate(SnapshotInfo_s *,msg_t *,int,playerState_s const *,playerState_s const *)+13C/r
                                        // SV_SetServerStaticHeader(void)+1A0/w ...
    gentity_s *gentities;               // XREF: SV_GentityNumLocal+38/r
                                        // SV_SetServerStaticHeader(void)+1AB/w ...
    int gentitySize;                    // XREF: SV_GentityNumLocal:loc_6F7C1F/r
                                        // SV_SetServerStaticHeader(void)+1B7/w ...
    clientState_s *firstClientState;    // XREF: G_GetClientStateLocal+38/r
                                        // SV_SetServerStaticHeader(void)+1C7/w ...
    playerState_s *firstPlayerState;    // XREF: GetFollowPlayerStateLocal+3E/r
                                        // SV_SetServerStaticHeader(void)+1D6/w ...
    MatchState *matchState;             // XREF: G_GetMatchStateLocal:loc_6F7C9F/r
                                        // SV_SetServerStaticHeader(void)+1EA/w ...
    int clientSize;                     // XREF: G_GetClientStateLocal:loc_6F7C5F/r
                                        // GetFollowPlayerStateLocal:loc_6F7CE4/r ...
};

struct svEntity_s // sizeof=0x168
{                                       // XREF: server_t/r
                                        // svEntity_t/r
    unsigned __int16 worldSector;
    unsigned __int16 nextEntityInWorldSector;
    archivedEntity_s baseline;          // XREF: CM_GetWaterHeight(float const * const,float,float)+107/o
                                        // CM_UnlinkEntity(svEntity_s *)+75/o ...
    int numClusters;
    int clusternums[16];                // XREF: SV_CreateBaseline(void)+A6/w
                                        // SV_CreateBaseline(void)+BE/w ...
    int lastCluster;
    int linkcontents;
    float linkmin[3];
    float linkmax[3];
};

struct __declspec(align(4)) server_t // sizeof=0x5C28C
{                                       // XREF: .data:server_t sv/r
    serverState_t state;                // XREF: SV_MapRestart(int)+274/w
                                        // SV_ShutdownGameProgs(void)+8/w ...
    int physicsTime;                    // XREF: SV_RunFrame(void)+29/w
                                        // SV_BuildClientSnapshot(client_t *)+34/r ...
    int timeResidual;                   // XREF: SV_FrameInternal+74/r
                                        // SV_FrameInternal:loc_715A85/r ...
    bool inFrame;                       // XREF: SV_InitSnapshot(void)+81/w
    bool smp;                           // XREF: SV_ArchiveSnapshot(msg_t *)+158/r
                                        // SV_PacketEvent(netadr_t,msg_t *)+12/r ...
    bool allowNetPackets;               // XREF: SV_RunEventLoop+42/r
                                        // SV_AllowPackets(bool)+54/w
    // padding byte
    int restarting;                     // XREF: SV_MapRestart(int)+27E/w
                                        // SV_SetConfigstring(int,char const *)+1A5/r ...
    int start_frameTime;                // XREF: SV_MapRestart(int)+15F/r
                                        // SV_MapRestart(int)+26E/w ...
    int checksumFeed;                   // XREF: SV_SendClientGameState(client_t *)+1022/r
                                        // SV_VerifyIwds_f:loc_70ADDB/r ...
    cmodel_t *models[512];
    unsigned __int16 emptyConfigString; // XREF: SV_SendClientGameState(client_t *)+64A/r
                                        // SV_SendClientGameState(client_t *)+736/r ...
    unsigned __int16 configstrings[0xCBC/*MAX_CONFIGSTRINGS*/];
    svEntity_s svEntities[1024];        // XREF: CM_GetWaterHeight(float const * const,float,float)+107/o
                                        // CM_UnlinkEntity(svEntity_s *)+75/o ...
    gentity_s *gentities;
    int gentitySize;
    int num_entities;
    playerState_s *gameClients;
    int gameClientSize;
    int checksum;
    int skelTimeStamp;
    int skelMemPos;
    int bpsWindow[20];                  // XREF: DrawEntityCounts+1B/r
                                        // DrawEntityCounts+25/r ...
    int bpsWindowSteps;
    int bpsTotalBytes;
    int bpsMaxBytes;
    int ubpsWindow[20];                 // XREF: SV_SendMessageToClient(msg_t *,client_t *,bool):loc_71D247/r
                                        // SV_SendMessageToClient(msg_t *,client_t *,bool)+4A0/w ...
    int ubpsTotalBytes;
    int ubpsMaxBytes;
    float ucompAve;
    int ucompNum;
    volatile int serverFrameTime;
    volatile int serverFrameTimeMin;
    volatile int serverFrameTimeMax;
    char gametype[64];                  // XREF: Com_Statmon:loc_6D7105/r
                                        // SV_UpdatePerformanceFrame+C1/w ...
    bool killServer;                    // XREF: SV_MapRestart(int)+A1/o
                                        // SV_MapRestart(int)+CE/o ...
    // padding byte
    // padding byte
    // padding byte
    const char *killReason;
    unsigned __int8 xpmods;
    // padding byte
    // padding byte
    // padding byte
};
static_assert(sizeof(server_t) == 377548);

struct XNKID;

char *__cdecl SV_ExpandNewlines(char *in);
void __cdecl SV_AddServerCommand(client_t *client, svscmd_type type, char *cmd);
int __cdecl SV_CanReplaceServerCommand(client_t *client, const char *cmd);
bool __cdecl SV_IsFirstTokenEqual(const char *str1, const char *str2);
void __cdecl SV_CullIgnorableServerCommands(client_t *client);
void SV_SendServerCommand(client_t *cl, svscmd_type type, const char *fmt, ...);
client_t *__cdecl SV_FindClientByAddress(netadr_t from, int qport);
void __cdecl SV_VoicePacket(netadr_t from, msg_t *msg);
void __cdecl SVC_Status(netadr_t from, XNKID *secID);
void __cdecl SVC_StatusScoreBoard(netadr_t from, XNKID *secID);
void __cdecl SVC_Info(netadr_t from, XNKID *secID, bool quick);
void __cdecl SV_ConnectionlessPacket(netadr_t from, msg_t *msg);
void __cdecl SV_PacketEvent(netadr_t from, msg_t *msg);
void __cdecl SV_CalcPings();
void __cdecl SV_FreeClientScriptId(client_t *cl);
void __cdecl SV_CheckTimeouts();
int __cdecl SV_CheckPaused();
void    SV_RunFrame();
void __cdecl SV_UpdatePerformanceFrame(int time);
void __cdecl SV_UpdateBots();
void __cdecl SV_InitThreadVariables();
void __cdecl SV_InitServerThread();
void     SV_ServerThread(unsigned int threadContext);
void __cdecl SV_PostFrame();
void SV_RunEventLoop();
void __cdecl SV_WaitServer();
void __cdecl SV_InitSnapshot();
void __cdecl SV_KillLocalServer();
void __cdecl SV_PreFrame();
int __cdecl SV_Frame(int controllerIndex, int msec);
void __cdecl SV_FrameInternal(int controllerIndex, int msec);
void SV_WakeServer();
char __cdecl SV_CheckOverflow();
void __cdecl SV_AllowPackets(bool allow);


extern serverStatic_t svs;
extern serverStaticHeader_t svsHeader;
extern int svsHeaderValid;
extern server_t sv;

extern int gameInitialized;
extern unsigned __int8 tempServerMsgBuf[65536];
