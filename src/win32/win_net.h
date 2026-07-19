#pragma once

// Winsock2 must precede Windows.h.  Otherwise Windows.h may include the legacy
// winsock.h first, hiding Winsock2 declarations such as gethostname.
#include <winsock2.h>
#include <Windows.h>

struct bdNetStartParams;

struct socketpool_t // sizeof=0x10
{                                       // XREF: .data:poolsockets/r
    int sendtime;                       // XREF: Sys_CheckForNATOverflow(void)+9C/r
    unsigned int m_sock;                // XREF: Sys_SocketPool_GetPacket(netadr_t *,msg_t *)+9E/r
    int ready;                          // XREF: Sys_CheckForNATOverflow(void)+84/r
                                        // Sys_CheckForNATOverflow(void)+102/w ...
    int sendcount;
};

struct netadr_t;
struct sockaddr;
struct msg_t;


void __cdecl NET_Sleep(unsigned int msec);
const char *__cdecl NET_ErrorString();
int __cdecl Sys_StringToSockaddr(const char *s, sockaddr *sadr);
void __cdecl SockadrToNetadr(sockaddr *s, netadr_t *a);
int __cdecl Sys_StringToAdr(const char *s, netadr_t *a);
int __cdecl Sys_GetPacket(netadr_t *net_from, msg_t *net_message);
char __cdecl Sys_SendPacket(unsigned int length, unsigned __int8 *data, netadr_t to);
bool __cdecl Sys_IsLANAddress_IgnoreSubnet(netadr_t adr);
int __cdecl Sys_IsLANAddress(netadr_t adr);
void __cdecl Sys_ShowIP();
void __cdecl NET_OpenIP();
int __cdecl dwPlatformInit(bdNetStartParams *params);
void __cdecl NET_SocketPool_Init();
void __cdecl Sys_CheckForNATOverflow();
int __cdecl Sys_SocketPool_GetPacket(netadr_t *net_from, msg_t *net_message);
void __cdecl NET_Config(int enableNetworking);
bool __cdecl NET_GetDvars();
void __cdecl NET_Restart();
void __cdecl NET_InitDebug();
int NET_InitDebugStreams();
void __cdecl NET_Init();
void __cdecl Sys_Listen_f();
unsigned int __cdecl NET_TCPIPSocket(char *net_interface, int port, int type);
int __cdecl NET_Select(unsigned int socket);
void __cdecl Sys_DebugSocketError(const char *message);
int __cdecl Sys_IsRemoteDebugServer();
char __cdecl Sys_StartRemoteDebugServer();
int __cdecl Sys_IsRemoteDebugClient();
void __cdecl Sys_Connect_f();
void __cdecl NET_ShutdownDebug();
void __cdecl NET_RestartDebug();
int __cdecl Sys_ReadDebugSocketData(char *buffer, int len, int blocking);
void Sys_SendDebugReadBytesInternal();
void __cdecl Sys_SendDebugReadBytes(int read);
int __cdecl Sys_ReadDebugSocketInt();
char *__cdecl Sys_ReadDebugSocketString();
void __cdecl Sys_WriteDebugSocketData(unsigned __int8 *buffer, int len);
void __cdecl Sys_DebugSend(int channel, const char *buf, int len, const char *name);
bool __cdecl Sys_DebugCanSend();
void __cdecl Sys_WriteDebugSocketInt(int value);
void __cdecl Sys_FlushDebugSocketData();
bool __cdecl Sys_DebugSocketReady(int channel);
void __cdecl Sys_WriteDebugSocketMessageType(unsigned __int8 type);
void __cdecl Sys_EndWriteDebugSocket();
void __cdecl Sys_WriteDebugSocketString(char *text);
int __cdecl Sys_UpdateDebugSocket();
int __cdecl Sys_ReadDebugSocketMessageType(unsigned __int8 *type, int blocking);
void __cdecl Sys_AckDebugSocket();
