#pragma once
#include <qcommon/net_chan_mp.h>
#include <ddl/ddl_api.h>
#include <server/server.h>

enum svc_ops_e : __int32
{
    svc_nop               = 0x0,
    svc_gamestate         = 0x1,
    svc_configstring      = 0x2,
    svc_gameinformation   = 0x3,
    svc_baseline          = 0x4,
    svc_entitycache       = 0x5,
    svc_dynentstate       = 0x6,
    svc_destructiblestate = 0x7,
    svc_ropestate         = 0x8,
    svc_glassstate        = 0x9,
    svc_serverCommand     = 0xA,
    svc_download          = 0xB,
    svc_snapshot          = 0xC,
    svc_servercommands    = 0xD,
    svc_EOF               = 0xE,
};

void __cdecl SV_HandleChallengeResponse(netadr_t from, msg_t *msg);
void __cdecl SV_GetChallenge(netadr_t from);
void __cdecl SV_CacheClientStatChange(unsigned int clientNum, ddlState_t *searchState);
void __cdecl SV_SetClientDIntStat(unsigned int clientNum, ddlState_t *searchState, unsigned int value);
void __cdecl SV_SetClientDStringStat(unsigned int clientNum, ddlState_t *searchState, const char *value);
void __cdecl SV_SetClientDInt64Stat(unsigned int clientNum, ddlState_t *searchState, unsigned __int64 value);
unsigned int __cdecl SV_GetClientDIntStat(unsigned int clientNum, ddlState_t *searchState);
char *__cdecl SV_GetClientDStringStat(unsigned int clientNum, ddlState_t *searchState);
unsigned int __cdecl SV_GetClientDInt64Stat(unsigned int clientNum, ddlState_t *searchState);
void __cdecl SV_UploadStats(int clientNum);
void __cdecl SV_UploadStats();
void __cdecl SV_SteamAuthClientRequest(netadr_t to, unsigned __int64 serverSteamID);
void __cdecl SV_SteamAuthClient(netadr_t from, msg_t *msg);
int __cdecl SV_IsTempBannedGuid(int guid);
int __cdecl SV_IsBannedGuid(int guid);
void __cdecl SV_BanGuidBriefly(int guid);
unsigned int __cdecl SV_FindFreeTempBanSlot();
void __cdecl SV_BanClient(client_t *cl);
void __cdecl SV_UnbanClient(const char *name);
void __cdecl SV_UpdateSplitscreenStateForAddr();
void __cdecl SV_FreeClient(client_t *cl);
void __cdecl SV_FreeClients();
void __cdecl SV_DirectConnect(netadr_t from);
void __cdecl SV_FreeClientScriptPers();
void __cdecl SV_SendDisconnect(
                client_t *client,
                int state,
                const char *reason,
                bool translationForReason,
                const char *clientName);
void __cdecl SV_DropClient(client_t *drop, const char *reason, bool tellThem, bool writeStats);
void __cdecl SV_DelayDropClient(client_t *drop, const char *reason);
void __cdecl SV_SendClientGameState(client_t *client);
bool __cdecl SV_ConfigStringIsConstant(int configStringNum);
void __cdecl SV_ClientEnterWorld(client_t *client, usercmd_s *cmd);
void __cdecl SV_CloseDownload(client_t *cl);
void __cdecl SV_StopDownload_f(client_t *cl);
void __cdecl SV_Download_Clear(client_t *cl);
void __cdecl SV_DoneDownload_f(client_t *cl);
void __cdecl SV_RetransmitDownload_f(client_t *cl);
void __cdecl SV_NextDownload_f(client_t *cl);
void __cdecl SV_BeginDownload_f(client_t *cl);
void __cdecl SV_WWWDownload_f(client_t *cl);
void __cdecl SV_WWWDownLoad_Clear(client_t *cl);
void __cdecl SV_WWWRedirectClient(client_t *cl, msg_t *msg);
void __cdecl SV_WriteDownloadErrorMessage(client_t *cl, msg_t *msg, const char *errorMessage);
void __cdecl SV_WriteDownloadToClient(client_t *cl, msg_t *msg);
void __cdecl SV_Disconnect_f(client_t *cl);
void __cdecl SV_VerifyIwds_f(client_t *cl);
void __cdecl SV_ResetPureClient_f(client_t *cl);
void __cdecl SV_StringToLower(const char *source, char *dest, int size);
char __cdecl SV_ValidateName(client_t *newcl, const char *name);
bool __cdecl SV_SanitizeName(const char *name, char *sanitizedName, int size);
void __cdecl SV_UserinfoChanged(client_t *cl);
void __cdecl SV_UpdateUserinfo_f(client_t *cl);
void __cdecl SV_MutePlayer_f(client_t *cl);
void __cdecl SV_UnmutePlayer_f(client_t *cl);
void __cdecl SV_ExecuteClientCommand(client_t *cl, const char *s, int clientOK, int fromOldServer);
void __cdecl SV_ClientThink(client_t *cl, usercmd_s *cmd);
void __cdecl SV_UserMove(client_t *cl, msg_t *msg, int delta);
void __cdecl SV_ExecuteClientMessage(client_t *cl, msg_t *msg);
int __cdecl SV_ProcessClientCommands(client_t *cl, msg_t *msg, int fromOldServer, int *lastCommand);
int __cdecl SV_ClientCommand(client_t *cl, msg_t *msg, int fromOldServer);
gentity_s *__cdecl SV_AddTestClient();
bool __cdecl SV_IsTestClient(unsigned int clientNum);
bool __cdecl SV_IsDemoClient(unsigned int clientNum);
char __cdecl SV_AddDemoClient();
void __cdecl SV_RemoveDemoClient();

extern int sv_serverId_value;
