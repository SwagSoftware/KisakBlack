#pragma once
#include "cl_main_mp.h"

enum quickmatchstate_t : __int32
{
    QM_DORMANT = 0x0,
    QM_FINDING = 0x1,
    QM_PINGING = 0x2,
    QM_CHOOSE  = 0x3,
    QM_FAILED  = 0x4,
};

struct serverStatusInfoResponse_s // sizeof=0x402C
{                                       // XREF: .data:serverStatusInfoResponse_s * cl_serverStatusScoreBoardList/r
                                        // .data:serverStatusInfoResponse_s * cl_serverStatusList/r ...
    char string[16384];
    netadr_t address;                   // XREF: CL_ServerStatus(char *,char *,int)+39/w
                                        // CL_ServerStatusScoreBoard(char *,char *,int)+39/w
    int time;
    int startTime;                      // XREF: CL_GetServerStatus(XNKID *)+C6/r
                                        // CL_GetServerStatus(XNKID *)+E0/r ...
    int pending;
    int print;
    int retrieved;                      // XREF: CL_GetServerStatus(XNKID *)+73/r
                                        // CL_GetServerStatusScoreBoard(XNKID *)+73/r ...
    XNKID secId;                 // XREF: CL_GetServerStatus(XNKID *)+27/o
                                        // CL_GetServerStatusScoreBoard(XNKID *)+27/o ...
};

struct TaskRecord;
struct MatchMakingInfo;

netadr_t *__cdecl CL_GetLastRconAddress();
void __cdecl CL_SetServerInfo(serverInfo_t *server, char *info, __int16 ping);
void __cdecl CL_ServerInfoPacket(XNKID *secID, msg_t *msg, int time);
void __cdecl CL_Connect(serverInfo_t *server);
void __cdecl CL_ConnectHackOnline();
void __cdecl CL_Connect_f();
void __cdecl CL_PC_SignInLive();
void __cdecl CL_InitServerInfo(serverInfo_t *server, netadr_t adr);
int __cdecl CL_RawPingSetupBuffer(
                unsigned __int8 *buffer,
                int buffersize,
                unsigned __int8 opcode,
                const XNKID *secID);
void __cdecl CL_RawPingServer(serverInfo_t *server, unsigned __int8 opcode);
void __cdecl CL_ServersResponsePacket(struct MatchMakingInfo *mminfo, int numResults, bool geo);
void __cdecl CL_FindServers_f();
void __cdecl CL_RconInit();
void __cdecl CL_Rcon_f();
void CL_RconLogin();
void CL_RconLogout();
void CL_RconHost();
serverStatusInfoResponse_s *__cdecl CL_GetServerStatus(XNKID *secID);
serverStatusInfoResponse_s *__cdecl CL_GetServerStatusScoreBoard(XNKID *secID);
int __cdecl CL_ServerStatus(char *serversecurityID, char *serverStatusString, int maxLen);
int __cdecl CL_ServerStatusScoreBoard(char *serversecurityID, char *serverStatusString, int maxLen);
void __cdecl CL_ServerStatusScoreBoardResponse(msg_t *msg, XNKID *secID);
void __cdecl CL_ServerStatusResponse(msg_t *msg, XNKID *secID);
void __cdecl CL_ResetPlayerMuting(unsigned int muteClientIndex);
void __cdecl CL_MutePlayer(int localClientNum, unsigned int muteClientIndex);
bool __cdecl CL_IsPlayerMuted(int localClientNum, unsigned int muteClientIndex);
void __cdecl CL_ClearMutedList();
void __cdecl CL_FinishMotdDownload();
void __cdecl CL_WWWDownload();
void __cdecl CL_Platform_RegisterCommands();
void __cdecl CL_PC_RequireLiveSignin();
void __cdecl CL_LanSessions_f();
void __cdecl CL_LanConnect_f();
void __cdecl CL_Prestige_f();
void __cdecl CL_RequestCACValidateSuccess();
void __cdecl CL_RequestCACValidateFailure();
void __cdecl CL_CACValidateHandleNACK(unsigned __int64 uid);
char __cdecl CL_CACValidateServerMatches(unsigned __int64 uid);
void __cdecl CL_CACValidateHandleOK(unsigned __int64 uid);
void __cdecl CL_CACValidateHandleBad(unsigned __int64 uid);
char __cdecl CL_RequestCACValidate(unsigned __int64 serverId);
void __cdecl CL_CACValidateRequest_f();
bool __cdecl CL_CACValidateInProgress();
char __cdecl CL_PrestigeRequest();
bool __cdecl CL_CACValidate_IsTimedOut();
void __cdecl CL_CACValidate_Frame();
void __cdecl CL_QuickMatchConnect_f();
bool __cdecl CL_QuickMatch_InProgress();
void __cdecl CL_QuickMatch_FindSessionsSuccess(TaskRecord *task);
void __cdecl CL_QuickMatch_FindSessionsFailure();
void __cdecl CL_QuickMatch_Start(
                unsigned int servertype,
                const char *mapname,
                unsigned int playlist,
                int minPlayers,
                int maxPlayers,
                int maxPing);
void __cdecl CL_QuickWager_Start();
void __cdecl CL_QuickMatch_f();
void __cdecl CL_QuickWager_f();
void __cdecl CL_QuickMatch_InitDvars();
void __cdecl CL_QuickMatch_Init();
bool __cdecl CL_QuickMatch_ServerMatches(struct MatchMakingInfo *server);
void __cdecl CL_QuickMatch_PingServers();
bool __cdecl CL_QuickMatch_GoodSessionFound();
void __cdecl CL_QuickMatch_PingResponse(XNKID *secID, msg_t *msg);
char __cdecl CL_QuickMatch_ChooseSession();
int __cdecl CL_QuickMatch_CompareServers(unsigned int *sv1, unsigned int *sv2);
void __cdecl CL_QuickMatch_Frame();
bool __cdecl CL_QuickMatch_ShouldChooseSession();
char *__cdecl CL_LongNameForShortName(const char *shortname);

void __cdecl CL_PC_SignIn();

extern const dvar_t *cl_quickmatch_resultspercent;
extern const dvar_t *cl_quickmatch_pingweight;
extern const dvar_t *cl_quickmatch_fullnessweight;
extern const dvar_t *cl_wager_firstplaylist;
extern const dvar_t *cl_wager_lastplaylist;
extern const dvar_t *cl_wager_maxping;