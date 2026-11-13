#pragma once

#include <qcommon/net_chan_mp.h>

struct reservedslot_t // sizeof=0x19
{                                       // XREF: .data:s_reservedSlots/r
    char pass[24];
    bool inUse;                         // XREF: SV_GetSlotForPasswordIfFree(char const *)+55/r
};

void __cdecl SV_ResetOnlineState();
int __cdecl SV_GetRegion();
void __cdecl SV_SetRegion(int region);
void __cdecl SV_SetTime(int time);
int __cdecl SV_GetTime();
void __cdecl SV_OnlineUserDisconnected(unsigned __int64 userID);
int __cdecl SV_GetSlotForPasswordIfFree(const char *password);
void __cdecl SV_FreeReservedSlot(int slot);
int __cdecl SV_DropClientForReservedSlot(const char *password);
void __cdecl SV_SetLicenseType(int licensetype);
int __cdecl SV_GetLicenseType();
bool __cdecl SV_IsWagerServer();
const char *__cdecl SV_GetMapName();
const char *__cdecl SV_GetHostName();
unsigned int __cdecl SV_GetClientCount();
bool __cdecl SV_HasInfoChanged();
bool __cdecl SV_IsServerRanked(int licensetype);
void __cdecl SV_MasterHeartbeat(int controllerIndex);
void SV_ReadWhiteListfile();
int SV_QuitIfNeeded();
void __cdecl SV_FlushRedirect(char *outputbuf);
void __cdecl SVC_RemoteCommand(netadr_t from);
void __cdecl SV_MatchEnd();
void __cdecl SV_SysLog_LogMessage(int severity, const char *msg);
void __cdecl SV_SysLog_LogMessage_f();
unsigned __int64 __cdecl SV_GetOwnerID();
bool __cdecl SV_CanLoadCustomGameType();
void __cdecl SV_RegisterRconKey_f();
