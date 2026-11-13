#pragma once
#include "cl_main_mp.h"

struct FriendInfo // sizeof=0xB8
{                                       // XREF: .data:friendInfo/r
                                        // .data:info_0/r ...
    int status;                         // XREF: SV_BotNameRandomFriend(int,char *,int)+97/r
                                        // UI_FeederItemText_CommmonPlayerListHandler+26C/r ...
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    unsigned __int64 dwid;              // XREF: UI_FeederItemText_CommmonPlayerListHandler+81/r
                                        // UI_FeederItemText_CommmonPlayerListHandler+8A/r ...
    unsigned __int64 steamid;
    char name[32];                      // XREF: LAN_GetServerInfo(int,int,char *,int)+602/o
                                        // SV_BotNameRandomFriend(int,char *,int):loc_7030D4/o ...
    char presence[128];                 // XREF: UI_FeederItemText_CommmonPlayerListHandler+295/o
};

int __cdecl LAN_AddServer(int source, const char *name, const char *address);
void __cdecl LAN_GetServerAddressString(int source, int n, char *buf, int buflen);
void __cdecl LAN_GetServerSecurityId(int source, int n, XNKID *buf, int buflen);
void __cdecl LAN_ResetPings(int source);
int __cdecl LAN_GetServerCount(int source);
int __cdecl LAN_WaitServerResponse();
void __cdecl LAN_GetServerInfo(int source, int n, char *buf, int buflen);
int __cdecl LAN_GetServerPing(int source, int n);
serverInfo_t *__cdecl LAN_GetServerPtr(int source, int n);
void __cdecl LAN_CleanHostname(const char *hostName, char *cleanHostName);
int __cdecl LAN_CompareHostname(const char *hostName1, const char *hostName2);
int __cdecl LAN_CompareServers(int source, int sortKey, int sortDir, int s1, int s2);
void __cdecl LAN_MarkServerDirty(int source, int n, unsigned __int8 dirty);
int __cdecl LAN_ServerIsDirty(int source, int n);
int __cdecl LAN_UpdateDirtyPings(int localClientNum, unsigned int source);
int __cdecl LAN_GetServerStatus(char *serverSecurityID, char *serverStatus, int maxLen);
int __cdecl LAN_GetServerStatusScoreBoard(char *serverSecurityID, char *serverStatus, int maxLen);
void __cdecl CLUI_SetPbClStatus(int status);
