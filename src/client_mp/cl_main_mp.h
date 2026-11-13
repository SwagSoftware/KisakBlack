#pragma once

#include <bgame/bg_local.h>
#include <qcommon/bitarray.h>
#include <qcommon/msg_mp.h>
#include <demo/demo_common.h>
#include <qcommon/net_chan_mp.h>
#include <client/screen_placement.h>
#include <universal/dvar.h>
#include <ui/ui_main.h>
#include <gfx_d3d/fxprimitives.h>
#include <cgame/cg_shellshock.h>
#include <ui/ui_shared.h>
#include <server/server.h>
#include <gfx_d3d/r_init.h>
#include <server_mp/sv_init_mp.h>

#define MAX_ACTOR_CORPSES 8

struct ClientArchiveData // sizeof=0x30
{                                                                             // XREF: demoInitialStateBuffer_t/r
        int serverTime;
        float origin[3];
        float velocity[3];
        int bobCycle;
        int movementDir;
        float viewangles[3];
};

struct demoInitialStateBuffer_t // sizeof=0x13010
{                                                                             // XREF: clientConnection_t/r
        unsigned __int8 compressedBuf[65536];
        int compressedBufSize;
        int serverMessageSequence;
        ClientArchiveData archive[256];
        int lastClientArchiveIndex;
        int continuousBufferStartingIndex;
};

struct demoContinuousStateBuffer_t // sizeof=0x1C
{                                                                             // XREF: clientConnection_t/r
        int serverMessageSequence;
        unsigned __int8 *msgData;
        int msgLen;
        unsigned __int8 *archive;
        int startClientArchiveIndex;
        int endClientArchiveIndex;
        int intialBufferStateIndex;
};

struct clSnapshot_t // sizeof=0x26DC
{                                                                             // XREF: .data:newSnap/r
                                                                                // .data:newSnap_0/r ...
        int valid;                                                    // XREF: CL_ParseSnapshot+BF/w
                                                                                // CL_ParseSnapshot:loc_571E2F/w ...
        int snapFlags;                                            // XREF: CL_ParseSnapshot+B1/w
                                                                                // Demo_ParseSnapshot(int,msg_t *)+D8/w ...
        int serverTime;                                         // XREF: CL_ParseSnapshot+52/w
                                                                                // CL_ParseSnapshot+21A/r ...
        int physicsTime;                                        // XREF: CL_ParseSnapshot+63/w
                                                                                // Demo_ParseSnapshot(int,msg_t *)+8A/w ...
        int messageNum;                                         // XREF: CL_ParseSnapshot+71/w
                                                                                // CL_ParseSnapshot:loc_571CD8/r ...
        int deltaNum;                                             // XREF: CL_ParseSnapshot+8C/w
                                                                                // CL_ParseSnapshot+A0/w ...
        int ping;
        int cmdNum;
        playerState_s ps;                                     // XREF: CL_ParseSnapshot+267/o
                                                                                // CL_ParseSnapshot+28E/o ...
        int numEntities;                                        // XREF: Demo_ParseSnapshot(int,msg_t *)+31E/w
                                                                                // Demo_ParseSnapshot(int,msg_t *)+32C/r
        int numClients;                                         // XREF: Demo_ParseSnapshot(int,msg_t *)+387/w
                                                                                // Demo_ParseSnapshot(int,msg_t *)+395/r
        int parseMatchStateNum;                         // XREF: Demo_ParseSnapshot(int,msg_t *)+1DE/w
        int parseEntitiesNum;                             // XREF: Demo_ParseSnapshot(int,msg_t *)+332/w
        int parseClientsNum;                                // XREF: Demo_ParseSnapshot(int,msg_t *)+39B/w
        int serverCommandNum;                             // XREF: CL_ParseSnapshot+41/w
                                                                                // Demo_ParseSnapshot(int,msg_t *)+68/w ...
};

enum StanceState : __int32
{                                                                             // XREF: clientActive_t/r
                                                                                // clientActive_t/r ...
        CL_STANCE_STAND                 = 0x0,
        CL_STANCE_CROUCH                = 0x1,
        CL_STANCE_PRONE                 = 0x2,
        CL_STANCE_DIVE_TO_PRONE = 0x3,
};

struct outPacket_t // sizeof=0xC
{                                                                             // XREF: clientActive_t/r
        int p_cmdNumber;
        int p_serverTime;
        int p_realtime;
};

struct clientActive_t // sizeof=0x1A6100
{
        bool usingAds;
        // padding byte
        // padding byte
        // padding byte
        int timeoutcount;
        clSnapshot_t snap;
        bool alwaysFalse;
        // padding byte
        // padding byte
        // padding byte
        int serverTime;
        int oldServerTime;
        int oldFrameServerTime;
        int serverTimeDelta;
        int oldSnapServerTime;
        int extrapolatedSnapshot;
        int newSnapshots;
        int serverId;
        char mapname[64];
        int parseMatchStateNum;
        int parseEntitiesNum;
        int parseClientsNum;
        int mouseDx[2];
        int mouseDy[2];
        int mouseIndex;
        bool stanceHeld;
        // padding byte
        // padding byte
        // padding byte
        StanceState stance;
        StanceState stancePosition;
        int stanceTime;
        int cgameUserCmdWeapon;
        int cgameUserCmdOffHandIndex;
        int cgameUserCmdLastWeaponForAlt;
        float cgameFOVSensitivityScale;
        float cgameMaxPitchSpeed;
        float cgameMaxYawSpeed;
        float cgameKickAngles[3];
        float cgameOrigin[3];
        float cgameVelocity[3];
        float cgameViewangles[3];
        int cgameBobCycle;
        int cgameMovementDir;
        bitarray<51> cgameExtraButton_bits;
        int cgamePredictedDataServerTime;
        PredictedVehicleInfo cgameVehicle;
        float viewangles[3];
        int skelTimeStamp;
        volatile unsigned int skelMemPos;
        char skelMemory[262144];
        char *skelMemoryStart;
        bool allowedAllocSkel;
        // padding byte
        // padding byte
        // padding byte
        usercmd_s cmds[128];
        int cmdNumber;
        ClientArchiveData clientArchive[256];
        int clientArchiveIndex;
        outPacket_t outPackets[32];
        clSnapshot_t snapshots[32];
        entityState_s entityBaselines[1024];
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
        MatchState parseMatchStates[32];
        entityState_s parseEntities[2048];
        clientState_s parseClients[2048];
        int corruptedTranslationFile;
        char translationVersion[256];
        int lastFireTime;
        bool useHeld;
        // padding byte
        // padding byte
        // padding byte
        int useTime;
        int useCount;
        int wasInVehicle;
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

struct clientLogo_t // sizeof=0x18
{                                                                             // XREF: clientStatic_t/r
        int startTime;                                            // XREF: CL_DrawLogo(int)+42/r
                                                                                // CL_PlayLogo_f+1E2/w
        int duration;                                             // XREF: CL_DrawLogo(int):loc_564A0E/r
                                                                                // CL_DrawLogo(int)+7F/r ...
        int fadein;                                                 // XREF: CL_DrawLogo(int)+4E/r
                                                                                // CL_DrawLogo(int)+5B/r ...
        int fadeout;                                                // XREF: CL_DrawLogo(int)+74/r
                                                                                // CL_DrawLogo(int)+8B/r ...
        Material *material[2];                            // XREF: CL_DrawLogo(int)+15A/r
                                                                                // CL_DrawLogo(int)+19F/r ...
};

struct XNADDR // sizeof=0x19
{                                                                             // XREF: serverInfo_t/r
    unsigned __int8 addrBuff[25];             // XREF: SND_SurfaceTypeToReflectance+19C/o
};

struct __declspec(align(4)) serverInfo_t // sizeof=0x178
{                                                                             // XREF: .data:s_quickmatchCandidates/r
        XNADDR xnaddr;                                            // XREF: SND_SurfaceTypeToReflectance+19C/o
        XNKEY xnkey;                                // XREF: CL_LanConnect_f+13/o
                                                                                // CL_LanConnect_f+EC/o ...
        XNKID xnkid;                                 // XREF: PM_Weapon_FireWeapon+17D/o
                                                                                // CL_LanConnect_f+1E/o ...
        // padding byte
        // padding byte
        // padding byte
        int publicSlots;
        int publicSlotsUsed;
        int privateSlots;
        int privateSlotsUsed;
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        unsigned __int64 nonce;                         // XREF: SND_SurfaceTypeToReflectance+164/o
                                                                                // .rdata:bits_ac_luminance/o ...
        netadr_t adr;                                             // XREF: CL_LanConnect_f+138/w
                                                                                // G_InitGrenadeEntity(gentity_s *,gentity_s *)+355/o ...
        unsigned __int8 allowAnonymous;
        unsigned __int8 bPassword;
        unsigned __int8 pure;
        char consoleDisabled;
        unsigned __int8 netType;
        unsigned __int8 clients;                        // XREF: CL_QuickMatch_ChooseSession(void)+A9/r
        unsigned __int8 maxClients;
        unsigned __int8 dirty;
        char friendlyfire;
        char killcam;
        char hardcore;
        unsigned __int8 hardware;
        unsigned __int8 mod;
        unsigned __int8 voice;
        unsigned __int8 punkbuster;
        unsigned __int8 requestCount;
        __int16 minPing;                                        // XREF: SV_ServerThread+1D6/o
                                                                                // SND_SurfaceTypeToReflectance+1B0/o ...
        __int16 maxPing;
        __int16 ping;                                             // XREF: CL_QuickMatch_Init(void)+80/w
                                                                                // CL_QuickMatch_ChooseSession(void)+BA/r
        // padding byte
        // padding byte
        int pingedTime;
        unsigned __int8 gotInfo;
        char hostName[32];                                    // XREF: CG_Calc3rdPersonVehicleViewValues(int)+F49/o
                                                                                // CG_PredictPlayerState_Internal+289/o ...
        char mapName[32];                                     // XREF: CG_CompassUpdateActors(int)+5E2/o
                                                                                // TRACK_cl_main(void)+20/o ...
        char game[64];                                            // XREF: AimTarget_ProcessEntityInternal(int,centity_s const *)+1FD/o
                                                                                // AimTarget_GetTargetBounds+34D/o ...
        char gameType[16];                                    // XREF: SND_SurfaceTypeToReflectance:loc_94A4B6/o
                                                                                // SND_SurfaceTypeToReflectance+11F/o ...
        // padding byte
        // padding byte
        // padding byte
        unsigned __int64 serverUID;                    // XREF: CG_CanSeeFriendlyHead(int,centity_s const *)+3D2/o
                                                                                // CG_ScanForCrosshairEntityInternal+30A/o ...
        int lastRequestTime;                                // XREF: CL_CACValidate_Frame(void)+25B/r
                                                                                // CL_CACValidate_Frame(void)+29C/w
        int region;
        int score;                                                    // XREF: CL_QuickMatch_ChooseSession(void)+99/r
                                                                                // Flame_Server_Trace+80/o ...
        int wagerBet;                                             // XREF: CL_QuickMatch_ChooseSession(void)+EE/r
                                                                                // CL_QuickMatch_ChooseSession(void)+108/r ...
        int playlist;                                             // XREF: VEH_TouchEntities+2E9/o
        int licensetype;
        unsigned __int8 basictraining;            // XREF: BG_ParseCommands(char const * *,animScriptItem_t *,animScriptData_t *)+524/o
                                                                                // BG_ParseCommands(char const * *,animScriptItem_t *,animScriptData_t *)+8DF/o ...
        unsigned __int8 customclassmode;
        char city[64];                                            // XREF: PM_playerTrace(pmove_t *,trace_t *,float const * const,float const * const,float const * const,float const * const,int,int)+11A/o
                                                                                // PM_Weapon_FireWeapon+18D/o ...
        char countrycode[3];                                // XREF: CG_Laser_Add_Core+1C0/o
                                                                                // CScr_BulletTrace+21/o ...
        // padding byte
        // padding byte
        // padding byte
};

struct trDebugString_t // sizeof=0x80
{
        float xyz[3];
        float color[4];
        float scale;
        char text[96];
};

struct clientDebugStringInfo_t // sizeof=0x10
{                                                                             // XREF: clientDebug_t/r
                                                                                // clientDebug_t/r ...
        int max;                                                        // XREF: CreateDebugStringsIfNeeded:loc_54DF9F/w
                                                                                // CreateDebugStringsIfNeeded+D9/w ...
        int num;                                                        // XREF: CreateDebugStringsIfNeeded+124/w
                                                                                // CreateDebugStringsIfNeeded+165/w ...
        trDebugString_t *strings;                     // XREF: CreateDebugStringsIfNeeded+6/r
                                                                                // CreateDebugStringsIfNeeded+F/r ...
        int *durations;                                         // XREF: CreateDebugStringsIfNeeded:loc_54DF0B/r
                                                                                // CreateDebugStringsIfNeeded+4D/r ...
};

struct trDebugLine_t // sizeof=0x2C
{
        float start[3];
        float end[3];
        float color[4];
        int depthTest;
};

struct clientDebugLineInfo_t // sizeof=0x10
{                                                                             // XREF: clientDebug_t/r
                                                                                // clientDebug_t/r ...
        int max;                                                        // XREF: CreateDebugLinesIfNeeded:loc_54E5C9/w
                                                                                // CreateDebugLinesIfNeeded+B3/w ...
        int num;                                                        // XREF: CreateDebugLinesIfNeeded+F5/w
                                                                                // CreateDebugLinesIfNeeded+12D/w ...
        trDebugLine_t *lines;                             // XREF: CreateDebugLinesIfNeeded+6/r
                                                                                // CreateDebugLinesIfNeeded+18/r ...
        int *durations;                                         // XREF: CreateDebugLinesIfNeeded+F/r
                                                                                // CreateDebugLinesIfNeeded+21/r ...
};

struct trDebugSphere_t // sizeof=0x28
{
        float center[3];
        float radius;
        int sideCount;
        float color[4];
        int depthTest;
};

struct clientDebugSphereInfo_t // sizeof=0x10
{                                                                             // XREF: clientDebug_t/r
                                                                                // clientDebug_t/r ...
        int max;                                                        // XREF: CreateDebugSpheresIfNeeded:loc_54E2C3/w
                                                                                // CreateDebugSpheresIfNeeded+AD/w ...
        int num;                                                        // XREF: CreateDebugSpheresIfNeeded+EF/w
                                                                                // CreateDebugSpheresIfNeeded+127/w ...
        trDebugSphere_t *spheres;                     // XREF: CreateDebugSpheresIfNeeded+6/r
                                                                                // CreateDebugSpheresIfNeeded+18/r ...
        int *durations;                                         // XREF: CreateDebugSpheresIfNeeded+F/r
                                                                                // CreateDebugSpheresIfNeeded+21/r ...
};

struct clientDebug_t // sizeof=0x98
{                                                                             // XREF: clientStatic_t/r
        int prevFromServer;
        int fromServer;                                         // XREF: CL_AddDebugString(float const * const,float const * const,float,char const *,int)+77/w
                                                                                // CL_AddDebugSphere(float const * const,float,float const * const,int,int,int)+7B/w ...
        clientDebugStringInfo_t clStrings;    // XREF: CL_AddDebugString(float const * const,float const * const,float,char const *,int):loc_54DE87/o
                                                                                // CreateDebugStringsIfNeeded+6/r ...
        clientDebugStringInfo_t svStringsBuffer;
                                                                                // XREF: CreateDebugStringsIfNeeded+56/r
                                                                                // CreateDebugStringsIfNeeded:loc_54DF73/r ...
        clientDebugStringInfo_t svStrings;    // XREF: CL_AddDebugString(float const * const,float const * const,float,char const *,int)+3E/o
                                                                                // CreateDebugStringsIfNeeded+F/r ...
        clientDebugLineInfo_t clLines;            // XREF: CL_AddDebugLine(float const * const,float const * const,float const * const,int,int):loc_54E4D7/o
                                                                                // CreateDebugLinesIfNeeded+6/r ...
        clientDebugLineInfo_t svLinesBuffer;
                                                                                // XREF: CreateDebugLinesIfNeeded+2A/r
                                                                                // CreateDebugLinesIfNeeded:loc_54E59A/r ...
        clientDebugLineInfo_t svLines;            // XREF: CL_AddDebugLine(float const * const,float const * const,float const * const,int,int)+3E/o
                                                                                // CreateDebugLinesIfNeeded+18/r ...
        clientDebugSphereInfo_t clSpheres;    // XREF: CL_AddDebugSphere(float const * const,float,float const * const,int,int,int):loc_54E1D7/o
                                                                                // CreateDebugSpheresIfNeeded+6/r ...
        clientDebugSphereInfo_t svSpheresBuffer;
                                                                                // XREF: CreateDebugSpheresIfNeeded+2A/r
                                                                                // CreateDebugSpheresIfNeeded:loc_54E297/r ...
        clientDebugSphereInfo_t svSpheres;    // XREF: CL_AddDebugSphere(float const * const,float,float const * const,int,int,int)+3E/o
};

struct gameState_t // sizeof=0x132F8
{                                                                             // XREF: clientStatic_t/r
        int stringOffsets[MAX_CONFIGSTRINGS];                        // XREF: CL_ConfigstringModified+1FF/w
                                                                                // CL_GetConfigString(int)+6A/r ...
        char stringData[65536];                         // XREF: CL_ConfigstringModified+13B/o
                                                                                // CL_ConfigstringModified+216/o ...
        int dataCount;                                            // XREF: CL_ConfigstringModified+148/w
                                                                                // CL_ConfigstringModified+1D3/r ...
        int matchUIVisibilityFlags;                 // XREF: CG_SetupGameInformation+16/r
                                                                                // CL_WriteUncompressedDemoInfo(int)+83B/r ...
};
static_assert(sizeof(gameState_t) == 78584);

struct clientStatic_t // sizeof=0x1CF2800
{                                                                             // XREF: .data:clientStatic_t cls/r
        int hunkUsersStarted;                             // XREF: CL_ShutdownHunkUsers(void)+8/r
                                                                                // CL_ShutdownHunkUsers(void)+73/w ...
        char servername[256];                             // XREF: CL_SetCGameTime(int)+18F/o
                                                                                // CL_MapLoading(char const *,bool)+B4/o ...
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        unsigned __int64 serveruid;                 // XREF: CL_Connect(serverInfo_t *)+34D/w
                                                                                // CL_Connect(serverInfo_t *)+359/w ...
        int rendererStarted;                                // XREF: CL_AddDebugString(float const * const,float const * const,float,char const *,int)+6/r
                                                                                // CL_AddDebugSphere(float const * const,float,float const * const,int,int,int)+6/r ...
        int soundStarted;                                     // XREF: CL_StartHunkUsers(void):loc_5645F3/r
                                                                                // Com_Init_Try_Block_Function+5F4/w
        int uiStarted;                                            // XREF: CL_GamepadButtonEvent(int,int,int,int,uint,int)+CB2/r
                                                                                // CL_GamepadButtonEvent(int,int,int,int,uint,int)+D07/r ...
        int devGuiStarted;                                    // XREF: CL_ShutdownHunkUsers(void)+1D/r
                                                                                // CL_ShutdownHunkUsers(void)+2B/w ...
        int frametime;                                            // XREF: CL_AdjustAngles+26/r
                                                                                // CL_GamepadMove:loc_5538FF/r ...
        int inputRealMsec;                                    // XREF: CL_GamepadMove+338/r
                                                                                // Com_Frame_Try_Block_Function+302/w
        int realtime;                                             // XREF: CL_FirstSnapshot(int)+63/r
                                                                                // CL_SetCGameTime(int)+221/w ...
        int realFrametime;                                    // XREF: CL_RunOncePerClientFrame(int,int)+C4/w
        clientLogo_t logo;                                    // XREF: CL_DrawLogo(int)+42/r
                                                                                // CL_DrawLogo(int)+4E/r ...
        float mapCenter[3];                                 // XREF: CL_WriteUncompressedDemoInfo(int)+75A/r
                                                                                // CL_WriteUncompressedDemoInfo(int)+76A/r ...
        int numlocalservers;
        serverInfo_t localServers[128];
        int lastServerPinged;                             // XREF: CL_UpdateDirtyPings(int,int)+66/r
                                                                                // CL_UpdateDirtyPings(int,int)+70/w ...
        int pingedServerCount;                            // XREF: CL_UpdateDirtyPings(int,int)+1B3/r
                                                                                // CL_UpdateDirtyPings(int,int)+1BB/w ...
        int totalServersParsed;
        int waitfindsessionsresponse;         // XREF: CL_ServersResponsePacket(MatchMakingInfo *,int,bool):loc_5693FC/w
                                                                                // CL_FindServers_f(void):def_569581/w ...
        int numrankedservers;                             // XREF: CL_GetServerList(int,serverInfo_t * *,int * *)+A3/o
                                                                                // CL_CACValidate_Frame(void)+8A/r ...
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        serverInfo_t rankedServers[20000];    // XREF: AimTarget_ProcessEntityInternal(int,centity_s const *)+1FD/o
                                                                                // AimTarget_GetTargetBounds+34D/o ...
        int numunrankedservers;                         // XREF: CL_GetServerList(int,serverInfo_t * *,int * *)+88/o
                                                                                // GetServerCounts:$LN5_114/r
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        serverInfo_t unrankedServers[20000]; // XREF: TRACK_cl_main(void)+20/o
                                                                                // CL_Shutdown(int)+26E/o ...
        int numwagerservers;                                // XREF: CL_GetServerList(int,serverInfo_t * *,int * *)+BE/o
                                                                                // GetServerCounts:$LN4_136/r
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        serverInfo_t wagerServers[20000];     // XREF: BG_ParseCommands(char const * *,animScriptItem_t *,animScriptData_t *)+524/o
                                                                                // BG_ParseCommands(char const * *,animScriptItem_t *,animScriptData_t *)+8DF/o ...
        int numfriendsservers;                            // XREF: CL_GetServerList(int,serverInfo_t * *,int * *)+D6/o
                                                                                // GetServerCounts:$LN2_138/r
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        serverInfo_t friendsServers[256];     // XREF: CL_GetServerList(int,serverInfo_t * *,int * *)+CD/o
        int numrecentservers;                             // XREF: CL_GetServerList(int,serverInfo_t * *,int * *)+106/o
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        serverInfo_t recentServers[20000];    // XREF: CG_Calc3rdPersonVehicleViewValues(int)+F49/o
                                                                                // CG_CanSeeFriendlyHead(int,centity_s const *)+3D2/o ...
        int lastFindSessionsTime;                     // XREF: CL_FindServers_f(void)+164/w
                                                                                // CL_CACValidate_Frame(void)+7C/r ...
        int numfavoriteservers;                         // XREF: CL_GetServerList(int,serverInfo_t * *,int * *)+EE/o
                                                                                // GetServerCounts:$LN3_139/r
        serverInfo_t favoriteServers[128];    // XREF: CL_GetServerList(int,serverInfo_t * *,int * *)+E5/o
        int pingUpdateSource;                             // XREF: CL_UpdateDirtyPings(int,int)+3A/w
                                                                                // CL_ServerInfoPacket(XNKID *,msg_t *,int)+1E/r ...
        Material *whiteMaterial;                        // XREF: ConDraw_Box:loc_54B8D0/r
                                                                                // ConDraw_Box+C7/r ...
        Material *consoleMaterial;                    // XREF: CL_InitRenderer(void)+E9/w
                                                                                // CL_ShutdownRenderer+7D/w
        Material *spinnerMaterial;                    // XREF: CL_DrawSpinnerPhysical(float,float,float,float,float const * const)+1BD/r
                                                                                // CL_DrawSpinnerLoadbarPhysical(float,float,float,float,float const * const,float):loc_55D532/r ...
        Font_s *consoleFont;                                // XREF: Con_CheckResize(void)+100/r
                                                                                // Con_CheckResize(void)+10D/r ...
        vidConfig_t vidConfig;                            // XREF: CG_Init(int,int,int,int)+A21/r
                                                                                // CL_GetScreenDimensions(int *,int *,float *)+81/r ...
        clientDebug_t debug;                                // XREF: CL_AddDebugString(float const * const,float const * const,float,char const *,int)+3E/o
                                                                                // CL_AddDebugString(float const * const,float const * const,float,char const *,int):loc_54DE87/o ...
        int download;                                             // XREF: CL_Disconnect(int,bool)+112/r
                                                                                // CL_Disconnect(int,bool)+11B/r ...
        char downloadTempName[256];                 // XREF: CL_ReadyToSendPacket:loc_55294D/r
                                                                                // CL_ReadyToSendPacket+8B/r ...
        char downloadName[256];                         // XREF: CL_ClearStaticDownload(void)+43/w
                                                                                // CL_Disconnect(int,bool):loc_55FCF4/w ...
        int downloadNumber;
        int downloadBlock;                                    // XREF: CL_BeginDownload(char const *,char const *)+8C/w
                                                                                // CL_ParseDownload:loc_572BD8/r ...
        int downloadCount;                                    // XREF: CL_BeginDownload(char const *,char const *)+96/w
                                                                                // CL_ParseDownload+278/r ...
        int downloadSize;                                     // XREF: CL_ParseWWWDownload(int,msg_t *)+6C/w
                                                                                // CL_ParseWWWDownload(int,msg_t *):loc_571901/r ...
        char downloadList[1024];                        // XREF: CL_NextDownload(int)+15/r
                                                                                // CL_NextDownload(int):loc_560D98/o ...
        int downloadRestart;                                // XREF: CL_ClearStaticDownload(void):loc_55FAC2/w
                                                                                // CL_DownloadsComplete(int)+22/r ...
        int doVidRestart;                                     // XREF: CL_DownloadsComplete(int):loc_560AC9/r
                                                                                // CL_SystemInfoChanged(int)+1AD/w ...
        int wwwDlDisconnected;                            // XREF: CL_ClearStaticDownload(void)+3/r
                                                                                // CL_ClearStaticDownload(void)+5B/w ...
        int wwwDlInProgress;                                // XREF: CL_ClearStaticDownload(void)+51/w
                                                                                // CL_InitDownloads(int)+25/w ...
        int downloadFlags;                                    // XREF: CL_ClearStaticDownload(void)+65/w
                                                                                // CL_ParseWWWDownload(int,msg_t *)+7D/w ...
        char originalDownloadName[64];            // XREF: CL_ClearStaticDownload(void)+4A/w
                                                                                // CL_WWWDownload(void)+82/o ...
        volatile unsigned int scriptError;                     // XREF: CL_Frame(int,int)+49/o
                                                                                // Com_Error(errorParm_t,char const *,...)+1A7/o
        float debugRenderForward[3];                // XREF: CL_GetDebugViewForward(float * const)+6/r
                                                                                // CL_GetDebugViewForward(float * const)+15/r ...
        float debugRenderPos[3];                        // XREF: CL_GetDebugViewPos(float * const)+6/r
                                                                                // CL_GetDebugViewPos(float * const)+15/r ...
        int serverId;                                             // XREF: CG_InitVote(int)+4A/r
                                                                                // CL_ConfigstringModified+E3/r ...
        gameState_t gameState;                            // XREF: CG_SetupGameInformation+16/r
                                                                                // CL_ConfigstringModified+10F/o ...
        // padding byte
        // padding byte
        // padding byte
        // padding byte

        // should be implicit now with constructors
        //clientStatic_t();
};


struct __declspec(align(8)) clientConnection_t // sizeof=0xAFF48
{
        int qport;
        int clientNum;
        int lastPacketSentTime;
        int lastPacketTime;
        netadr_t serverAddress;
        int connectTime;
        int connectPacketCount;
        char serverMessage[256];
        int challenge;
        int checksumFeed;
        int reliableSequence;
        int reliableAcknowledge;
        char *reliableCommands[128];
        int reliableCommandBufferNext;
        char reliableCommandBuffer[16384];
        int serverMessageSequence;
        int serverCommandSequence;
        int lastExecutedServerCommand;
        char serverCommands[128][1024];
        bool isServerRestarting;
        // padding byte
        // padding byte
        // padding byte
        int lastClientArchiveIndex;
        char demoName[64];
        int demorecording;
        int demoplaying;
        int isTimeDemo;
        int demowaiting;
        int firstDemoFrameSkipped;
        int demofile;
        int demoRecordStartTime;
        int demoUseMemoryBuffer;
        int demoMemoryPoolBegin;
        int demoMemoryPoolEnd;
        unsigned __int8 demoMemoryPool[262144];
        int demoRequestUncompressedPacket;
        int demoRequestUncompressedPacketTime;
        int demoReceivedUncompressedPacket;
        demoInitialStateBuffer_t demoInitialStateBuffer[3];
        demoContinuousStateBuffer_t demoContinuousStateBuffer[200];
        int demoInitialStateBufferIndex;
        int demoContinuousStateBufferIndex;
        int demoCoolRecordExtendedCapture;
        int demoCoolRecordProcessingExtendedCapture;
        int demoCoolRecordProcessingTime;
        int demoLiveStream;
        int demoPrevServerTime;
        int timeDemoLog;
        int timeDemoFrames;
        int timeDemoStart;
        int timeDemoPrev;
        int timeDemoBaseTime;
        netchan_t netchan;
        char netchanOutgoingBuffer[2048];
        char netchanIncomingBuffer[65536];
        netProfileInfo_t OOBProf;
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        __int64 statPacketsToSend;
        int statPacketSendTime[34];
        unsigned int serverChallenge;
        unsigned int clientChallenge;
        unsigned int nonce;
        // padding byte
        // padding byte
        // padding byte
        // padding byte
};

struct cgs_t // sizeof=0x31A4
{
    int viewX;
    int viewY;
    int viewWidth;
    int viewHeight;
    float viewAspect;
    int serverCommandSequence;
    int processedSnapshotNum;
    int localServer;
    char gametype[32];
    char szHostName[256];
    int maxclients;
    char mapname[64];
    int gameEndTime;
    int voteTime;
    int voteYes;
    int voteNo;
    char voteString[256];
    XModel *gameModels[512];
    const FxEffectDef *fxs[196];
    const FxEffectDef *grenadeFx[27];
    const FxEffectDef *playerFireFx[3];
    shellshock_parms_t holdBreathParams;
    char teamChatMsgs[8][241];
    int teamChatMsgTimes[8];
    int teamChatPos;
    int teamLastChatPos;
    float compassWidth;
    float compassHeight;
    float compassY;
    char _pad[4];
    clientInfo_t corpseinfo[4];
    actorInfo_t actorCorpseInfo[MAX_ACTOR_CORPSES];
    bool entUpdateToggleContextKey;
    // padding byte
    // padding byte
    // padding byte
};

struct voiceCommunication_t;
struct GfxConfiguration;
struct serverInfo_t;
struct Font_s;

void __cdecl CL_AddReliableCommand(int localClientNum, const char *cmd);
void __cdecl CL_AnimateUI_f();
void __cdecl CL_ShowContextualItemUI_f();
void __cdecl CL_ToggleDemoRecording_f();
void __cdecl CL_WriteDemoEpilog(int localClientNum);
void __cdecl CL_WriteDemoClientArchive(int demoFile, unsigned __int8 msgType, ClientArchiveData *archive, int index);
void __cdecl CL_WriteDemoFromContinuousStateBuffer(int localClientNum, int continuousStateBufferIndex);
void __cdecl CL_StopRecord_f();
void __cdecl CL_Record_f();
void __cdecl CL_WriteInitialStateToDemo(
                int demoFile,
                unsigned __int8 type,
                int serverMessageSequence,
                unsigned __int8 *compressedBuf,
                int compressedBufSize);
void __cdecl CL_WriteUncompressedDemoInfo(int localClientNum);
void __cdecl CL_WriteAllDemoClientArchive(int localClientNum);
void __cdecl CL_WriteAllDemoClientArchiveToMemory(int localClientNum);
void __cdecl CL_WriteDemoClientArchiveToMemory(const clientActive_t *cl, ClientArchiveData *archive, int index);
void __cdecl CL_ReadDemoMessage(int localClientNum);
void __cdecl CL_DemoCompleted(int localClientNum);
void __cdecl CL_ReadDemoClientArchive(int localClientNum);
void __cdecl CL_ReadDemoNetworkPacket(int localClientNum);
void __cdecl CL_NextDemo(int localClientNum);
void __cdecl CL_ShutdownDevGui();
void __cdecl CL_ShutdownHunkUsers();
void __cdecl CL_ShutdownAll();
void __cdecl CL_ShutdownDemo();
void __cdecl CL_MapLoading(const char *mapname);
void __cdecl CL_ResetSkeletonCache(unsigned int localClientNum);
void __cdecl CL_ClearStaticDownload();
void __cdecl CL_ClearState(int localClientNum);
void __cdecl CL_UploadStatsForController(int localControllerIndex);
void __cdecl CL_Disconnect(unsigned int localClientNum, bool deactivateClient);
void __cdecl CL_UpdateProfileAfterGame(int controllerIndex);
void __cdecl CL_CanNotConnect(int localClientNum, const char *msg);
void __cdecl CL_ForwardCommandToServer(int localClientNum, const char *string);
void __cdecl CL_ForwardToServer_f();
void __cdecl CL_Setenv_f();
void __cdecl CL_DisconnectLocalClient(int localClientNum);
void __cdecl CL_Reconnect_f();
void __cdecl CL_SendPureChecksums(int localClientNum);
void __cdecl CL_Vid_Restart_f();
void __cdecl CL_Snd_Restart_f();
void __cdecl CL_OpenedIWDList_f();
void __cdecl CL_ReferencedIWDList_f();
void __cdecl CL_Configstrings_f();
void __cdecl CL_Clientinfo_f();
bool __cdecl CL_WasMapAlreadyLoaded();
void __cdecl CL_DownloadsComplete(int localClientNum);
void __cdecl LoadMapLoadscreen(char *mapname);
void __cdecl CL_BeginDownload(const char *localName, const char *remoteName);
void __cdecl CL_NextDownload(int localClientNum);
void __cdecl CL_InitDownloads(int localClientNum);
void __cdecl CL_CheckForResend(int localClientNum);
void __cdecl CL_BuildMd5StrFromCDKey(char *md5Str);
char *__cdecl strtrm(char *str);
void __cdecl CL_DisconnectError(const char *message);
bool __cdecl CL_HandleVoiceTypePacket(int localClientNum, msg_t *msg);
char __cdecl CL_ConnectionlessPacket(int localClientNum, netadr_t from, msg_t *msg, int time);
char    CL_DispatchConnectionlessPacket(int localClientNum, netadr_t from, msg_t *msg);
void __cdecl CL_DisconnectPacket(int localClientNum, netadr_t from, const char *reason);
void __cdecl CL_SteamServerAuth(unsigned __int64 serverSteamID);
void __cdecl CL_AllocatePerLocalClientMemory();
void __cdecl AllocatePerLocalClientMemory(int maxLocalClients, int maxClients, unsigned int flags);
void __cdecl CL_FreePerLocalClientMemory();
void    CL_InitLoad(char *mapname, const char *gametype);
bool __cdecl CL_PacketEvent(int localClientNum, netadr_t from, msg_t *msg, int time, bool connectionlesss_packets_only);
void __cdecl CL_WriteNewDemoClientArchive(int localClientNum);
void __cdecl CL_WriteNewDemoClientArchiveToMemory(int localClientNum);
unsigned __int8 *__cdecl CL_DemoMemoryPoolAllocate(clientConnection_t *clc, int size);
void __cdecl CL_DemoMemoryPoolDeallocate(clientConnection_t *clc, int size);
int __cdecl CL_GetClientArchiveCount(int start, int end);
void __cdecl CL_WriteDemoMessage(int localClientNum, msg_t *msg, int headerBytes);
void __cdecl CL_WriteDemoMessageToMemory(int localClientNum, msg_t *msg, int headerBytes);
void __cdecl CL_VoiceTransmit(int localClientNum);
voiceCommunication_t *__cdecl CL_GetLocalClientVoiceCommunication(int localClientNum);
int __cdecl Client_SendVoiceData(int bytes, char *enc_buffer);
char __cdecl Voice_SendVoiceData();
void __cdecl CL_RunOncePerClientFrame(int localClientNum, int msec);
void __cdecl CL_Frame(int localClientNum, int msec);
void __cdecl CL_CheckTimeout(int localClientNum);
void __cdecl CL_ServerTimedOut(int localClientNum);
void __cdecl CL_CheckUserinfo(int localClientNum);
void __cdecl CL_UpdateInGameState(int localClientNum);
void __cdecl CL_VoiceFrame(int localClientNum);
char __cdecl CL_IsClientLocal(int clientNum);
void __cdecl CL_ParseBadPacket_f();
void __cdecl CL_ShutdownRef();
void __cdecl CL_ShutdownWorld();
void __cdecl CL_SetupViewport();
void __cdecl CL_InitRenderer();
void __cdecl CL_ShutdownRenderer(int destroyWindow);
void __cdecl CL_StartHunkUsers();
void CL_InitDevGui();
void __cdecl CL_DevGuiDvar_f();
void __cdecl CL_DevGuiCmd_f();
void __cdecl CL_DevGuiOpen_f();
int __cdecl CL_ScaledMilliseconds();
void __cdecl CL_InitRef();
void __cdecl SetupGfxConfig(GfxConfiguration *config);
void __cdecl CL_InitDedicated();
void __cdecl CL_startSingleplayer_f();
void __cdecl CL_DrawLogo(int localClientNum);
void __cdecl CL_StopLogo(int localClientNum);
void __cdecl CL_StopLogoOrCinematic(int localClientNum);
void __cdecl CL_ToggleMenu_f();
bool __cdecl CL_ShouldAllowInGameMenu(int localClientNum);
char __cdecl Playlist_ReadFromDisk();
void __cdecl CL_PauseGame(bool state);
void __cdecl CL_InitOnceForAllClients();
void __cdecl CL_PlayDemo_f();
void __cdecl CL_Disconnect_f();
void __cdecl CL_SetupViewport_f();
void __cdecl CL_PlayLogo_f();
void __cdecl CL_OpenScriptMenu_f();
void __cdecl Playlist_CmdReadFromDisk();
void __cdecl Playlist_CmdSetCategoryFilter();
void __cdecl Playlist_CmdSetPrevPlaylist();
void __cdecl Playlist_CmdGetPrevPlaylist();
void __cdecl CL_CmdGetOldCustomName();
void __cdecl CL_CmdSetNewCustomName();
void __cdecl CL_ResetStats_f();
void __cdecl CL_Init(int localClientNum);
int __cdecl CountBitsEnabled(unsigned int num);
void __cdecl CL_Shutdown(int localClientNum);
bool __cdecl CL_GetServerList(int source, serverInfo_t **servers, int **count);
int __cdecl CL_UpdateDirtyPings(int localClientNum, unsigned int source);
void __cdecl CL_ShowIP_f();
char *__cdecl CL_GetServerIPAddress();
void    CL_SetupForNewServerMap(int localClientNum, char *pszMapName, const char *pszGametype);
bool __cdecl CL_IsServerLoadingMap();
bool __cdecl CL_IsWaitingOnServerToLoadMap(int localClientNum);
void __cdecl CL_SetWaitingOnServerToLoadMap(int localClientNum, bool waiting);
void __cdecl CL_SyncGpu();
void __cdecl CL_DrawTextPhysical(
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                float w,
                float xScale,
                float yScale,
                const float *color,
                int style);
void __cdecl CL_DrawTextPhysicalWithEffects(
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                float w,
                float xScale,
                float yScale,
                const float *color,
                int style,
                const float *glowColor,
                Material *fxMaterial,
                Material *fxMaterialGlow,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration);
void __cdecl CL_DrawText(
                const ScreenPlacement *scrPlace,
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                int horzAlign,
                int vertAlign,
                float xScale,
                float yScale,
                const float *color,
                int style);
void __cdecl CL_DrawTextWithEffects(
                const ScreenPlacement *scrPlace,
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                float rotation,
                int horzAlign,
                int vertAlign,
                float xScale,
                float yScale,
                const float *color,
                int style,
                const float *glowColor,
                Material *fxMaterial,
                Material *fxMaterialGlow,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration);
void __cdecl CL_DrawTextWithCOD7TypeWriterEffects(
                const ScreenPlacement *scrPlace,
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                float rotation,
                int horzAlign,
                int vertAlign,
                float xScale,
                float yScale,
                const float *color,
                int style,
                const float *glowColor,
                Material *fxMaterial,
                Material *fxMaterialGlow,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration);
void __cdecl CL_DrawTextWithRedactEffects(
                const ScreenPlacement *scrPlace,
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                int horzAlign,
                int vertAlign,
                float xScale,
                float yScale,
                const float *color,
                int style,
                const float *glowColor,
                Material *fxMaterial,
                Material *fxMaterialGlow,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration,
                int fxRedactDecayStartTime,
                int fxRedactDecayDuration);
void __cdecl CL_DrawTextPhysicalWithCOD7DecodeEffects(
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                float xScale,
                float yScale,
                const float *color,
                int style,
                const float *glowColor,
                Material *fxMaterial,
                Material *fxMaterialGlow,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration);
void __cdecl CL_DrawTextPhysicalWithRedactEffects(
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                float xScale,
                float yScale,
                const float *color,
                int style,
                const float *glowColor,
                Material *fxMaterial,
                Material *fxMaterialGlow,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration,
                int fxRedactDecayStartTime,
                int fxRedactDecayDuration);
void __cdecl CL_DrawTextWithPopInEffects(
                const ScreenPlacement *scrPlace,
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                float rotation,
                int horzAlign,
                int vertAlign,
                float xScale,
                float yScale,
                const float *color,
                int style,
                const float *glowColor,
                Material *fxMaterial,
                Material *fxMaterialGlow,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration);
void __cdecl CL_DrawTextRotate(
                const ScreenPlacement *scrPlace,
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                float rotation,
                int horzAlign,
                int vertAlign,
                float xScale,
                float yScale,
                const float *color,
                int style);
void __cdecl CL_DrawTextPhysicalWithCursor(
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                float xScale,
                float yScale,
                const float *color,
                int style,
                int cursorPos,
                char cursor);
void __cdecl CL_DrawTextWithCursor(
                const ScreenPlacement *scrPlace,
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                int horzAlign,
                int vertAlign,
                float xScale,
                float yScale,
                const float *color,
                int style,
                int cursorPos,
                char cursor);
bool __cdecl CL_ShouldDisplayHud(int localClientNum);
bool __cdecl CL_IsUIActive(int localClientNum);
Font_s *__cdecl CL_RegisterFont(const char *fontName, int imageTrack);
float (*__cdecl CL_GetMapCenter())[3];
void __cdecl CL_SanitizeClanName();
int __cdecl CL_FilterChar(unsigned __int8 input);
//clientStatic_t *__thiscall clientStatic_t::clientStatic_t(clientStatic_t *this);
bool __cdecl CL_IsLocalClientInGame(int localClientNum);


extern clientStatic_t cls;
extern clientConnection_t *clientConnections;
extern UiContext cgDC[1];

extern bool cl_serverLoadingMap;
extern bool g_waitingForServer;
extern bool cl_waitingOnServerToLoadMap[1];


extern const dvar_t *cl_noprint;
extern const dvar_t *playlist;
extern const dvar_t *category;
extern const dvar_t *categoryPlaylist;
extern const dvar_t *wagerCategory;
extern const dvar_t *wagerCategoryPlaylist;
extern const dvar_t *customGameMode;
extern const dvar_t *customclass[10];
extern const dvar_t *cl_hudDrawsBehindUI;
extern const dvar_t *cl_voice;
extern const dvar_t *cl_timeout;
extern const dvar_t *cl_connectTimeout;
extern const dvar_t *cl_connectionAttempts;
extern const dvar_t *cl_shownet;
extern const dvar_t *cl_shownuments;
extern const dvar_t *cl_showServerCommands;
extern const dvar_t *cl_showSend;
extern const dvar_t *cl_showTimeDelta;
extern const dvar_t *cl_freezeDemo;
extern const dvar_t *cl_activeAction;
extern const dvar_t *cl_avidemo;
extern const dvar_t *cl_forceavidemo;
extern const dvar_t *cl_clientDemoName;
extern const dvar_t *cl_clientDemoUseMemoryBuffer;
extern const dvar_t *cl_clientDemoType;
extern const dvar_t *cl_clientDemoMaxRecordTime;
extern const dvar_t *cl_clientDemoRequestPingTime;
extern const dvar_t *cl_clientDemoCoolMomentExtendEnabled;
extern const dvar_t *cl_clientDemoCoolMomentExtendTime;
extern const dvar_t *cl_yawspeed;
extern const dvar_t *cl_pitchspeed;
extern const dvar_t *cl_anglespeedkey;
extern const dvar_t *cl_maxpackets;
extern const dvar_t *cl_packetdup;
extern const dvar_t *cl_sensitivity;
extern const dvar_t *cl_mouseAccel;
extern const dvar_t *cl_freelook;
extern const dvar_t *cl_showMouseRate;
extern const dvar_t *cl_allowDownload;
extern const dvar_t *cl_wwwDownload;
extern const dvar_t *cl_talking;
extern const dvar_t *cl_inGameVideo;
extern const dvar_t *cl_serverStatusResendTime;
extern const dvar_t *cl_maxppf;
extern const dvar_t *cl_bypassMouseInput;
extern const dvar_t *m_pitch;
extern const dvar_t *m_yaw;
extern const dvar_t *m_forward;
extern const dvar_t *m_side;
extern const dvar_t *m_filter;
extern const dvar_t *ui_custom_newname;
extern const dvar_t *cl_socketpool_enabled;
extern const dvar_t *cl_socketpool_size;
extern const dvar_t *ui_clantag_new;
extern const dvar_t *cl_demoFFSpeed;
extern const dvar_t *cl_demoSkipToTime;
extern const dvar_t *cl_demoLiveStreaming;
extern const dvar_t *input_invertPitch;
extern const dvar_t *input_viewSensitivity;
extern const dvar_t *team_indicator;
extern const dvar_t *cl_playerRank;
extern const dvar_t *cl_playerPrestige;
extern const dvar_t *cl_motdString;
extern const dvar_t *cl_ingame;
extern const dvar_t *cl_wasconnected;
extern const dvar_t *cl_maxPing;
extern const dvar_t *cl_profileTextHeight;
extern const dvar_t *cl_profileTextY;
extern const dvar_t *name;
extern const dvar_t *clanName;
extern const dvar_t *rate;
extern const dvar_t *snaps;
extern const dvar_t *cl_punkbuster;
extern const dvar_t *password;
extern const dvar_t *nextdemo;
extern const dvar_t *mortarStrikesLeft;
extern const dvar_t *hud_enable;
extern const dvar_t *cg_blood;
extern const dvar_t *cg_allow_mature;
extern const dvar_t *cg_mature;
extern const dvar_t *cl_serverchallenge;
extern const dvar_t *cl_debugMessageKey;
extern const dvar_t *motd;

extern unsigned int frame_msec;
extern int old_com_frameTime;


static const char *customClassDvars[11] =
{
  "customclass1",
  "customclass2",
  "customclass3",
  "customclass4",
  "customclass5",
  "prestigeclass1",
  "prestigeclass2",
  "prestigeclass3",
  "prestigeclass4",
  "prestigeclass5",
  NULL
};