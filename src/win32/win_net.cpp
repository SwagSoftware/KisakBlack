#include "win_net.h"
#include <universal/q_shared.h>
#include <qcommon/net_chan_mp.h>
#include <game_mp/g_main_mp.h>
#include "win_shared.h"
#include <qcommon/threads.h>
#include <server_mp/sv_main_mp.h>
#include <clientscript/cscr_debugger.h>
#include <universal/com_memory.h>

const dvar_t *ip;
const dvar_t *port;

const dvar_t *net_noudp;
const dvar_t *net_socksEnabled;
const dvar_t *net_socksServer;
const dvar_t *net_socksPort;
const dvar_t *net_socksUsername;
const dvar_t *net_socksPassword;

unsigned int ip_debugSocket[2];

int numIP;
unsigned __int8 localIP[16][4];
unsigned __int8 g_debugPacket[1][8192];

int g_debugServer;
SOCKET ip_socket = INVALID_SOCKET;
SOCKET socks_socket;
int cl_nat_overflow_detected;
socketpool_t poolsockets[50];
int cl_socketpool_lastpacket_time;
int networkingEnabled;
int g_debugClient;

WSAData winsockdata;
unsigned int ip_debugServerSocket[2];
int sys_debugMessageType[1];
int g_debugPacketPos[1];
char *g_debugReadBytes;
int g_debugReadBytesSent;
int g_debugWriteBytes;
int g_debugReadBytesRemote;
int winsockInitialized;

/*
====================
NET_IPSocket
====================
*/
// LWSS ADD: from kcod4. Plain UDP sockets
unsigned int __cdecl NET_IPSocket(const char *net_interface, int port)
{
    const char *v2; // eax
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    sockaddr address; // [esp+0h] [ebp-24h] BYREF
    int _true; // [esp+18h] [ebp-Ch] BYREF
    int i; // [esp+1Ch] [ebp-8h] BYREF
    unsigned int newsocket; // [esp+20h] [ebp-4h]

    _true = 1;
    i = 1;
    if (net_interface)
        Com_Printf(16, "Opening IP socket: %s:%i\n", net_interface, port);
    else
        Com_Printf(16, "Opening IP socket: localhost:%i\n", port);
    newsocket = socket(2, 2, 17);
    if (newsocket == -1)
    {
        if (WSAGetLastError() != 10047)
        {
            v2 = NET_ErrorString();
            Com_PrintWarning(16, "WARNING: UDP_OpenSocket: socket: %s\n", v2);
        }
        return 0;
    }
    else if (ioctlsocket(newsocket, 0x8004667E, (unsigned long *)&_true) == -1)
    {
        v4 = NET_ErrorString();
        Com_PrintWarning(16, "WARNING: UDP_OpenSocket: ioctl FIONBIO: %s\n", v4);
        return 0;
    }
    else if (setsockopt(newsocket, 0xFFFF, 32, (const char *)&i, 4) == -1)
    {
        v5 = NET_ErrorString();
        Com_PrintWarning(16, "WARNING: UDP_OpenSocket: setsockopt SO_BROADCAST: %s\n", v5);
        return 0;
    }
    else
    {
        if (net_interface && *net_interface && I_stricmp(net_interface, "localhost"))
            Sys_StringToSockaddr(net_interface, &address);
        else
            *(_DWORD *)&address.sa_data[2] = 0;
        if (port == -1)
            *(_WORD *)address.sa_data = 0;
        else
            *(_WORD *)address.sa_data = htons(port);
        address.sa_family = 2;
        if (bind(newsocket, &address, 16) == -1)
        {
            v6 = NET_ErrorString();
            Com_PrintWarning(16, "WARNING: UDP_OpenSocket: bind: %s\n", v6);
            closesocket(newsocket);
            return 0;
        }
        else
        {
            return newsocket;
        }
    }
}


void __cdecl NET_Sleep(unsigned int msec)
{
    Sleep(msec);
}

const char *__cdecl NET_ErrorString()
{
    const char *result; // eax
    int Error; // [esp+0h] [ebp-8h]

    Error = WSAGetLastError();
    if ( Error > 11001 )
    {
        switch ( Error )
        {
            case 11002:
                return "WSATRY_AGAIN";
            case 11003:
                return "WSANO_RECOVERY";
            case 11004:
                return "WSANO_DATA";
            default:
                return "NO ERROR";
        }
    }
    else if ( Error == 11001 )
    {
        return "WSAHOST_NOT_FOUND";
    }
    else
    {
        switch ( Error )
        {
            case 10004:
                result = "WSAEINTR";
                break;
            case 10009:
                result = "WSAEBADF";
                break;
            case 10013:
                result = "WSAEACCES";
                break;
            case 10014:
                result = "WSAEFAULT";
                break;
            case 10022:
                result = "WSAEINVAL";
                break;
            case 10024:
                result = "WSAEMFILE";
                break;
            case 10035:
                result = "WSAEWOULDBLOCK";
                break;
            case 10036:
                result = "WSAEINPROGRESS";
                break;
            case 10037:
                result = "WSAEALREADY";
                break;
            case 10038:
                result = "WSAENOTSOCK";
                break;
            case 10039:
                result = "WSAEDESTADDRREQ";
                break;
            case 10040:
                result = "WSAEMSGSIZE";
                break;
            case 10041:
                result = "WSAEPROTOTYPE";
                break;
            case 10042:
                result = "WSAENOPROTOOPT";
                break;
            case 10043:
                result = "WSAEPROTONOSUPPORT";
                break;
            case 10044:
                result = "WSAESOCKTNOSUPPORT";
                break;
            case 10045:
                result = "WSAEOPNOTSUPP";
                break;
            case 10046:
                result = "WSAEPFNOSUPPORT";
                break;
            case 10047:
                result = "WSAEAFNOSUPPORT";
                break;
            case 10048:
                result = "WSAEADDRINUSE";
                break;
            case 10049:
                result = "WSAEADDRNOTAVAIL";
                break;
            case 10050:
                result = "WSAENETDOWN";
                break;
            case 10051:
                result = "WSAENETUNREACH";
                break;
            case 10052:
                result = "WSAENETRESET";
                break;
            case 10053:
                result = "WSWSAECONNABORTEDAEINTR";
                break;
            case 10054:
                result = "WSAECONNRESET";
                break;
            case 10055:
                result = "WSAENOBUFS";
                break;
            case 10056:
                result = "WSAEISCONN";
                break;
            case 10057:
                result = "WSAENOTCONN";
                break;
            case 10058:
                result = "WSAESHUTDOWN";
                break;
            case 10059:
                result = "WSAETOOMANYREFS";
                break;
            case 10060:
                result = "WSAETIMEDOUT";
                break;
            case 10061:
                result = "WSAECONNREFUSED";
                break;
            case 10062:
                result = "WSAELOOP";
                break;
            case 10063:
                result = "WSAENAMETOOint";
                break;
            case 10064:
                result = "WSAEHOSTDOWN";
                break;
            case 10091:
                result = "WSASYSNOTREADY";
                break;
            case 10092:
                result = "WSAVERNOTSUPPORTED";
                break;
            case 10093:
                result = "WSANOTINITIALISED";
                break;
            case 10101:
                result = "WSAEDISCON";
                break;
            default:
                return "NO ERROR";
        }
    }
    return result;
}

int __cdecl Sys_StringToSockaddr(const char *s, sockaddr *sadr)
{
    struct hostent *h; // [esp+0h] [ebp-4h]

    *(unsigned int *)&sadr->sa_family = 0;
    *(unsigned int *)&sadr->sa_data[2] = 0;
    *(unsigned int *)&sadr->sa_data[6] = 0;
    *(unsigned int *)&sadr->sa_data[10] = 0;
    sadr->sa_family = 2;
    *(_WORD *)sadr->sa_data = 0;
    if ( I_isdigit(*s) )
    {
        *(unsigned int *)&sadr->sa_data[2] = inet_addr(s);
    }
    else
    {
        h = gethostbyname(s);
        if ( !h )
            return 0;
        *(unsigned int *)&sadr->sa_data[2] = **(unsigned int **)h->h_addr_list;
    }
    return 1;
}

void __cdecl SockadrToNetadr(sockaddr *s, netadr_t *a)
{
    if ( s->sa_family == 2 )
    {
        a->type = NA_IP;
        *(unsigned int *)a->ip = *(unsigned int *)&s->sa_data[2];
        a->port = *(_WORD *)s->sa_data;
    }
}

int __cdecl Sys_StringToAdr(const char *s, netadr_t *a)
{
    sockaddr sadr; // [esp+0h] [ebp-14h] BYREF

    if ( !Sys_StringToSockaddr(s, &sadr) )
        return 0;
    SockadrToNetadr(&sadr, a);
    return 1;
}

int __cdecl Sys_GetPacket(netadr_t *net_from, msg_t *net_message)
{
    const char *v2; // eax
    const char *v3; // eax
    sockaddr from; // [esp+8h] [ebp-28h] BYREF
    int err; // [esp+1Ch] [ebp-14h]
    int ret; // [esp+20h] [ebp-10h]
    int protocol; // [esp+24h] [ebp-Ch]
    int fromlen; // [esp+28h] [ebp-8h] BYREF
    unsigned int net_socket; // [esp+2Ch] [ebp-4h]

    //for (protocol = 0; protocol < 2; ++protocol)
    {
        //if (protocol)
        //    net_socket = ipx_socket;
        //else
            net_socket = ip_socket;
        if (net_socket)
        {
            fromlen = 16;
            ret = recvfrom(net_socket, (char *)net_message->data, net_message->maxsize, 0, &from, &fromlen);
            if (ret == -1)
            {
                err = WSAGetLastError();
                if (err != 10035 && err != 10054)
                {
                    v2 = NET_ErrorString();
                    Com_PrintError(16, "NET_GetPacket: %s\n", v2);
                }
            }
            else
            {
                if (net_socket == ip_socket)
                {
                    *(_DWORD *)&from.sa_data[6] = 0;
                    *(_DWORD *)&from.sa_data[10] = 0;
                }
                //if (usingSocks && net_socket == ip_socket && !memcmp(&from, &socksRelayAddr, fromlen))
                //{
                //    if (ret < 10
                //        || *net_message->data
                //        || net_message->data[1]
                //        || net_message->data[2]
                //        || net_message->data[3] != 1)
                //    {
                //        continue;
                //    }
                //    net_from->type = NA_IP;
                //    *(_DWORD *)net_from->ip = *((_DWORD *)net_message->data + 1);
                //    net_from->port = *(net_message->data + 4);
                //    net_message->readcount = 10;
                //}
                //else
                {
                    SockadrToNetadr(&from, net_from);
                    net_message->readcount = 0;
                }
                if (ret != net_message->maxsize)
                {
                    net_message->cursize = ret;
                    return 1;
                }
                v3 = NET_AdrToString(*net_from);
                Com_Printf(16, "Oversize packet from %s\n", v3);
            }
        }
    }
    return 0;
}

void NetadrToSockadr(netadr_t *a, struct sockaddr *s) {
    memset(s, 0, sizeof(*s));

    if (a->type == NA_BROADCAST) {
        ((struct sockaddr_in *)s)->sin_family = AF_INET;
        ((struct sockaddr_in *)s)->sin_port = a->port;
        ((struct sockaddr_in *)s)->sin_addr.s_addr = INADDR_BROADCAST;
    }
    else if (a->type == NA_IP) {
        ((struct sockaddr_in *)s)->sin_family = AF_INET;
        ((struct sockaddr_in *)s)->sin_addr.s_addr = *(int *)&a->ip;
        ((struct sockaddr_in *)s)->sin_port = a->port;
    }
    // LWSS: remove IPX, noone uses this
//#ifndef _XBOX
//	else if( a->type == NA_IPX ) {
//		((struct sockaddr_ipx *)s)->sa_family = AF_IPX;
//		memcpy( ((struct sockaddr_ipx *)s)->sa_netnum, &a->ipx[0], 4 );
//		memcpy( ((struct sockaddr_ipx *)s)->sa_nodenum, &a->ipx[4], 6 );
//		((struct sockaddr_ipx *)s)->sa_socket = a->port;
//	}
//	else if( a->type == NA_BROADCAST_IPX ) {
//		((struct sockaddr_ipx *)s)->sa_family = AF_IPX;
//		memset( ((struct sockaddr_ipx *)s)->sa_netnum, 0, 4 );
//		memset( ((struct sockaddr_ipx *)s)->sa_nodenum, 0xff, 6 );
//		((struct sockaddr_ipx *)s)->sa_socket = a->port;
//	}
//#endif //_XBOX
}

char __cdecl Sys_SendPacket(unsigned int length, unsigned __int8 *data, netadr_t to)
{
    const char *v4; // eax
    int err; // [esp+0h] [ebp-20h]
    sockaddr addr; // [esp+4h] [ebp-1Ch] BYREF
    int ret; // [esp+14h] [ebp-Ch]
    unsigned int net_socket; // [esp+18h] [ebp-8h]

    net_socket = 0;
    switch (to.type)
    {
    case NA_BROADCAST:
        net_socket = ip_socket;
        break;
    case NA_IP:
        net_socket = ip_socket;
        break;
        // LWSS: remove IPX, noone uses this
            //case NA_IPX:
            //	net_socket = ipx_socket;
            //	break;
            //case NA_BROADCAST_IPX:
            //	net_socket = ipx_socket;
            //	break;
    default:
        Com_Error(ERR_FATAL, "Sys_SendPacket: bad address type");
        break;
    }
    if (!net_socket)
        return 1;
    NetadrToSockadr(&to, &addr);
    //if (usingSocks && to.type == NA_IP)
    //{
    //    socksBuf[0] = 0;
    //    socksBuf[1] = 0;
    //    socksBuf[2] = 0;
    //    socksBuf[3] = 1;
    //    *(_DWORD *)&socksBuf[4] = *(_DWORD *)&addr.sa_data[2];
    //    *(_WORD *)&socksBuf[8] = *(_WORD *)addr.sa_data;
    //    memcpy((unsigned __int8 *)&socksBuf[10], data, length);
    //    ret = sendto(net_socket, socksBuf, length + 10, 0, &socksRelayAddr, 16);
    //}
    //else
    {
        ret = sendto(net_socket, (const char *)data, length, 0, &addr, 16);
    }
    if (ret != -1)
        return 1;
    err = WSAGetLastError();
    if (err == 10035)
        return 1;
    //	if (err == 10049 && (to.type == NA_BROADCAST || to.type == NA_BROADCAST_IPX))
    if (err == 10049 && (to.type == NA_BROADCAST))
        return 1;
    v4 = NET_ErrorString();
    Com_PrintError(16, "Sys_SendPacket: %s\n", v4);
    return 0;
}

bool __cdecl Sys_IsLANAddress_IgnoreSubnet(netadr_t adr)
{
    if ( adr.type == NA_LOOPBACK )
        return 1;
    if ( adr.type == NA_BOT )
        return 1;
    if ( adr.type != NA_IP )
        return 0;
    if ( adr.ip[0] == 10 )
        return 1;
    if ( adr.ip[0] == 127 )
        return 1;
    if ( adr.ip[0] == 169 && adr.ip[1] == 254 )
        return 1;
    if ( adr.ip[0] == 172 && (adr.ip[1] & 0xF0) == 0x10 )
        return 1;
    return adr.ip[0] == 192 && adr.ip[1] == 168;
}

int __cdecl Sys_IsLANAddress(netadr_t adr)
{
    int i; // [esp+0h] [ebp-8h]

    if ( Sys_IsLANAddress_IgnoreSubnet(adr) )
        return 1;
    for ( i = 0; i < numIP; ++i )
    {
        if ( adr.ip[0] == localIP[i][0] && adr.ip[1] == localIP[i][1] && adr.ip[2] == localIP[i][2] )
            return 1;
    }
    return 0;
}

void __cdecl Sys_ShowIP()
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < numIP; ++i )
        Com_Printf(16, "IP: %i.%i.%i.%i\n", localIP[i][0], localIP[i][1], localIP[i][2], localIP[i][3]);
}

/*
=====================
NET_GetLocalAddress
// LWSS ADD: from cod4
=====================
*/

#ifndef _XBOX
int NET_GetLocalAddress()
{
    int result; // eax
    u_long v1; // [esp+0h] [ebp-114h]
    char hostname[256]; // [esp+4h] [ebp-110h] BYREF
    hostent *hostInfo; // [esp+108h] [ebp-Ch]
    int n; // [esp+10Ch] [ebp-8h]
    char *p; // [esp+110h] [ebp-4h]

    if (gethostname(hostname, 256) == -1)
        return WSAGetLastError();
    hostInfo = gethostbyname(hostname);
    if (!hostInfo)
        return WSAGetLastError();
    Com_Printf(16, "Hostname: %s\n", hostInfo->h_name);
    n = 0;
    while (1)
    {
        p = hostInfo->h_aliases[n++];
        if (!p)
            break;
        Com_Printf(16, "Alias: %s\n", p);
    }
    result = hostInfo->h_addrtype;
    if (result == 2)
    {
        for (numIP = 0; ; ++numIP)
        {
            result = numIP;
            p = hostInfo->h_addr_list[numIP];
            if (!p || numIP >= 16)
                break;
            v1 = ntohl(*(_DWORD *)p);
            localIP[numIP][0] = *p;
            localIP[numIP][1] = p[1];
            localIP[numIP][2] = p[2];
            localIP[numIP][3] = p[3];
            Com_Printf(16, "IP: %i.%i.%i.%i\n", HIBYTE(v1), BYTE2(v1), BYTE1(v1), (unsigned __int8)v1);
        }
    }
    return result;
}
#endif

void __cdecl NET_OpenIP()
{
    ip = _Dvar_RegisterString("net_ip", "localhost", 0x20u, "Network IP Address");
    port = _Dvar_RegisterInt("net_port", 3074, 0, 0xFFFF, 0x20u, "Network port");

    // LWSS ADD - network augmentation (cod4)
    for (int i = 0; i < 10; ++i)
    {
        ip_socket = NET_IPSocket(ip->current.string, i + port->current.integer);
        if (ip_socket)
        {
            Dvar_SetInt((dvar_s *)port, i + port->current.integer);
            NET_GetLocalAddress();
            return;
        }
    }
    Com_PrintWarning(16, "WARNING: Couldn't allocate IP port\n");
    // LWSS END
}

void __cdecl NET_SocketPool_Init()
{
    Dvar_GetBool("cl_socketpool_enabled");
}

void __cdecl Sys_CheckForNATOverflow()
{
    int i; // [esp+0h] [ebp-4h]
    int ia; // [esp+0h] [ebp-4h]

    if ( Dvar_GetBool("cl_socketpool_enabled") )
    {
        if ( cl_nat_overflow_detected <= 0 )
        {
            for ( i = 0; i < Dvar_GetInt("cl_socketpool_size"); ++i )
            {
                if ( !poolsockets[i].ready && (int)(Sys_Milliseconds() - poolsockets[i].sendtime) > 2000 )
                {
                    Com_Printf(0, "SOCKETPOOL: Timing out socket %i\n", i);
                    if ( (int)(Sys_Milliseconds() - cl_socketpool_lastpacket_time) > 5000 )
                    {
                        cl_nat_overflow_detected = Sys_Milliseconds();
                        for ( ia = 0; ia < Dvar_GetInt("cl_socketpool_size"); ++ia )
                            poolsockets[ia].ready = 1;
                        return;
                    }
                    poolsockets[i].ready = 1;
                }
            }
        }
        else if ( (int)(Sys_Milliseconds() - cl_nat_overflow_detected) > 20000 )
        {
            Com_Printf(0, "SOCKETPOOL: Clearing timeout\n");
            cl_nat_overflow_detected = 0;
        }
    }
}

int __cdecl Sys_SocketPool_GetPacket(netadr_t *net_from, msg_t *net_message)
{
    const char *v3; // eax
    int i; // [esp+0h] [ebp-24h]
    sockaddr from; // [esp+4h] [ebp-20h] BYREF
    int err; // [esp+18h] [ebp-Ch]
    int ret; // [esp+1Ch] [ebp-8h]
    int fromlen; // [esp+20h] [ebp-4h] BYREF

    if ( !Sys_IsMainThread() )
        return 0;
    if ( !Dvar_GetBool("cl_socketpool_enabled") )
        return 0;
    fromlen = 16;
    for ( i = 0; i < Dvar_GetInt("cl_socketpool_size"); ++i )
    {
        if ( !poolsockets[i].ready )
        {
            ret = recvfrom(poolsockets[i].m_sock, (char *)net_message->data, net_message->maxsize, 0, &from, &fromlen);
            if ( ret != -1 )
            {
                cl_socketpool_lastpacket_time = Sys_Milliseconds();
                SockadrToNetadr(&from, net_from);
                poolsockets[i].ready = 1;
                net_message->cursize = ret;
                return 1;
            }
            err = WSAGetLastError();
            if ( err != 10035 && err != 10054 )
            {
                v3 = NET_ErrorString();
                Com_PrintError(16, "NET_GetPacket: %s\n", v3);
            }
        }
    }
    return 0;
}

void __cdecl NET_Config(int enableNetworking)
{
    int start; // [esp+0h] [ebp-Ch]
    int stop; // [esp+4h] [ebp-8h]
    bool modified; // [esp+8h] [ebp-4h]

    modified = NET_GetDvars();
    if ( net_noudp->current.enabled )
        enableNetworking = 0;
    if ( enableNetworking != networkingEnabled || modified )
    {
        if ( enableNetworking == networkingEnabled )
        {
            if ( enableNetworking )
            {
                stop = 1;
                start = 1;
            }
            else
            {
                stop = 0;
                start = 0;
            }
        }
        else
        {
            if ( enableNetworking )
            {
                stop = 0;
                start = 1;
            }
            else
            {
                stop = 1;
                start = 0;
            }
            networkingEnabled = enableNetworking;
        }
        if ( stop )
        {
            if ( ip_socket && ip_socket != -1 )
            {
                closesocket(ip_socket);
                ip_socket = 0;
            }
            if ( socks_socket && socks_socket != -1 )
            {
                closesocket(socks_socket);
                socks_socket = 0;
            }
        }
        if ( start )
        {
            if ( !net_noudp->current.enabled )
            {
                NET_OpenIP();
                NET_SocketPool_Init();
            }
        }
    }
}


bool __cdecl NET_GetDvars()
{
    bool modified; // [esp+0h] [ebp-4h]

    modified = 0;
    if ( net_noudp )
        modified = net_noudp->modified;
    net_noudp = _Dvar_RegisterBool("net_noudp", 0, 0x21u, "Disable UDP");
    if ( net_socksEnabled && net_socksEnabled->modified )
        modified = 1;
    net_socksEnabled = _Dvar_RegisterBool("net_socksEnabled", 0, 0x21u, "Enable network sockets");
    if ( net_socksServer && net_socksServer->modified )
        modified = 1;
    net_socksServer = _Dvar_RegisterString("net_socksServer", (char *)"", 0x21u, "Network socket server");
    if ( net_socksPort && net_socksPort->modified )
        modified = 1;
    net_socksPort = _Dvar_RegisterInt("net_socksPort", 1080, 0, 0xFFFF, 0x21u, "Network socket port");
    if ( net_socksUsername && net_socksUsername->modified )
        modified = 1;
    net_socksUsername = _Dvar_RegisterString(
                                                "net_socksUsername",
                                                (char *)"",
                                                0x21u,
                                                "Network socket username");
    if ( net_socksPassword && net_socksPassword->modified )
        modified = 1;
    net_socksPassword = _Dvar_RegisterString(
                                                "net_socksPassword",
                                                (char *)"",
                                                0x21u,
                                                "Network socket password");
    return modified;
}

void __cdecl NET_Restart()
{
    NET_Config(networkingEnabled);
}

void __cdecl NET_InitDebug()
{
    g_debugClient = 0;
    g_debugServer = 0;
    NET_InitDebugStreams();
}



int NET_InitDebugStreams()
{
    int result; // eax
    int i; // [esp+0h] [ebp-4h]
    int ia; // [esp+0h] [ebp-4h]

    for ( i = 0; i < 2; ++i )
    {
        ip_debugServerSocket[i] = 0;
        ip_debugSocket[i] = 0;
        result = i + 1;
    }
    for ( ia = 0; ia < 1; ++ia )
    {
        sys_debugMessageType[ia] = 0;
        g_debugPacketPos[ia] = 0;
        result = ia + 1;
    }
    g_debugReadBytes = 0;
    g_debugReadBytesSent = 0;
    g_debugWriteBytes = 0;
    g_debugReadBytesRemote = 0;
    return result;
}

void __cdecl NET_Init()
{
    int r; // [esp+0h] [ebp-4h]

    r = WSAStartup(0x101u, &winsockdata);
    if ( r )
    {
        Com_PrintWarning(16, "WARNING: Winsock initialization failed, returned %d\n", r);
    }
    else
    {
        winsockInitialized = 1;
        Com_Printf(16, "Winsock Initialized\n");
        NET_GetDvars();
        NET_Config(1);
        NET_InitDebug();
    }
}

void __cdecl Sys_Listen_f()
{
    const char *v0; // eax
    const char *v1; // eax
    int i; // [esp+0h] [ebp-4h]

    if ( !com_sv_running->current.enabled )
    {
        if ( g_debugClient )
            Com_Error(ERR_DROP, "Net connection already started");
        if ( g_debugServer )
            Com_Error(ERR_DROP, "Net connection already started");
        for ( i = 0; i < 2; ++i )
        {
            ip_debugServerSocket[i] = NET_TCPIPSocket((char*)"127.0.0.1", i + 28970, 0);
            if ( !ip_debugServerSocket[i] )
            {
                Sys_DebugSocketError("Sys_Listen_f: failed to connect");
                return;
            }
            if ( listen(ip_debugServerSocket[i], 5) == -1 )
            {
                v0 = NET_ErrorString();
                v1 = va("Sys_Listen_f: listen: %s\n", v0);
                Sys_DebugSocketError(v1);
                return;
            }
        }
        if ( g_debugServer )
        {
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_net.cpp", 1868, 0, "%s", "!g_debugServer") )
                __debugbreak();
        }
        g_debugClient = 1;
        SystemParametersInfoA(0x2001u, 0, 0, 3u);
    }
}

unsigned int __cdecl NET_TCPIPSocket(char *net_interface, int port, int type)
{
    const char *v3; // eax
    const char *v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    sockaddr_in address; // [esp+4h] [ebp-20h] BYREF
    int err; // [esp+18h] [ebp-Ch]
    int _true; // [esp+1Ch] [ebp-8h] BYREF
    unsigned int newsocket; // [esp+20h] [ebp-4h]

    _true = 1;
    if ( net_interface )
        Com_Printf(16, "Opening IP socket: %s:%i\n", net_interface, port);
    else
        Com_Printf(16, "Opening IP socket: localhost:%i\n", port);
    newsocket = socket(2, 1, 6);
    if ( newsocket == -1 )
    {
        if ( WSAGetLastError() != 10047 )
        {
            v3 = NET_ErrorString();
            Com_PrintWarning(16, "WARNING: NET_TCPIPSocket: socket: %s\n", v3);
        }
        return 0;
    }
    else
    {
        if ( ioctlsocket(newsocket, -2147195266, (u_long *)&_true) == -1 )
        {
            v5 = NET_ErrorString();
            Com_PrintWarning(16, "WARNING: NET_TCPIPSocket: ioctl FIONBIO: %s\n", v5);
            return 0;
        }
        if ( net_interface && *net_interface && I_stricmp(net_interface, "localhost") )
            Sys_StringToSockaddr(net_interface, (sockaddr *)&address);
        else
            address.sin_addr.S_un.S_addr = 0;
        if ( port == -1 )
            address.sin_port = 0;
        else
            address.sin_port = htons(port);
        address.sin_family = 2;
        if ( type )
        {
            if ( type == 1 && connect(newsocket, (const struct sockaddr *)&address, 16) == -1 )
            {
                err = WSAGetLastError();
                if ( err != 10035 )
                {
                    v7 = NET_ErrorString();
                    Com_PrintWarning(16, "WARNING: NET_TCPIPSocket: connect: %s\n", v7);
                    closesocket(newsocket);
                    return 0;
                }
                if ( !NET_Select(newsocket) )
                {
                    Com_PrintWarning(16, "WARNING: NET_TCPIPSocket: connect failed\n");
                    closesocket(newsocket);
                    return 0;
                }
            }
        }
        else if ( bind(newsocket, (const struct sockaddr *)&address, 16) == -1 )
        {
            v6 = NET_ErrorString();
            Com_PrintWarning(16, "WARNING: NET_TCPIPSocket: bind: %s\n", v6);
            closesocket(newsocket);
            return 0;
        }
        return newsocket;
    }
}

int __cdecl NET_Select(unsigned int socket)
{
    const char *v2; // eax
    fd_set readfds; // [esp+0h] [ebp-220h] BYREF
    int err; // [esp+10Ch] [ebp-114h]
    fd_set writefds; // [esp+110h] [ebp-110h] BYREF
    timeval time; // [esp+218h] [ebp-8h] BYREF

    readfds.fd_count = 1;
    readfds.fd_array[0] = socket;
    writefds.fd_count = 1;
    writefds.fd_array[0] = socket;
    time.tv_sec = 5;
    time.tv_usec = 0;
    err = select(0, &readfds, &writefds, 0, &time);
    if ( err )
    {
        if ( err == -1 )
        {
            v2 = NET_ErrorString();
            Com_PrintWarning(16, "WARNING: NET_Select: connect: %s\n", v2);
            return 0;
        }
        else
        {
            return 1;
        }
    }
    else
    {
        Com_Printf(16, "NET_Select: NET_Select: timeout\n");
        return 0;
    }
}

void __cdecl Sys_DebugSocketError(const char *message)
{
    NET_RestartDebug();
    Com_Printf(16, "%s\n", message);
}

int __cdecl Sys_IsRemoteDebugServer()
{
    return g_debugServer;
}

char __cdecl Sys_StartRemoteDebugServer()
{
    if ( g_debugServer
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_net.cpp", 1897, 0, "%s", "!g_debugServer") )
    {
        __debugbreak();
    }
    g_debugServer = 1;
    return 1;
}

int __cdecl Sys_IsRemoteDebugClient()
{
    return g_debugClient;
}

void __cdecl Sys_Connect_f()
{
    const char *v0; // [esp+0h] [ebp-1Ch]
    int i; // [esp+18h] [ebp-4h]

    if ( g_debugClient || g_debugServer )
    {
        Com_Printf(0, "Net connection already started\n");
    }
    else
    {
        if ( Cmd_Argc() <= 1 )
            v0 = "127.0.0.1";
        else
            v0 = (char *)Cmd_Argv(1);
        for ( i = 0; i < 2; ++i )
        {
            ip_debugSocket[i] = NET_TCPIPSocket((char *)v0, i + 28970, 1);
            if ( !ip_debugSocket[i] )
            {
                Sys_DebugSocketError("Sys_Connect_f: failed to connect");
                return;
            }
        }
        if ( g_debugClient
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_net.cpp", 1946, 0, "%s", "!g_debugClient") )
        {
            __debugbreak();
        }
        if ( g_debugServer )
        {
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_net.cpp", 1947, 0, "%s", "!g_debugServer") )
                __debugbreak();
        }
        SystemParametersInfoA(0x2001u, 0, 0, 3u);
        Scr_ConnectRemote(SCRIPTINSTANCE_SERVER);
    }
}

void __cdecl NET_ShutdownDebug()
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < 2; ++i )
    {
        if ( ip_debugSocket[i] )
        {
            closesocket(ip_debugSocket[i]);
            ip_debugSocket[i] = 0;
        }
        if ( ip_debugServerSocket[i] )
        {
            closesocket(ip_debugServerSocket[i]);
            ip_debugServerSocket[i] = 0;
        }
    }
    //if ( Sys_IsRemoteDebugClient() )
        //BLOPS_NULLSUB();
}

void __cdecl NET_RestartDebug()
{
    NET_ShutdownDebug();
    NET_InitDebug();
}

int __cdecl Sys_ReadDebugSocketData(char *buffer, int len, int blocking)
{
    const char *v4; // eax
    const char *v5; // eax
    int err; // [esp+0h] [ebp-10h]
    int read; // [esp+Ch] [ebp-4h]

    while ( 1 )
    {
        while ( 1 )
        {
            if ( !len )
                return 1;
            if ( !ip_debugSocket[0]
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_net.cpp",
                            2048,
                            0,
                            "%s",
                            "ip_debugSocket[0]") )
            {
                __debugbreak();
            }
            read = recvfrom(ip_debugSocket[0], buffer, len, 0, 0, 0);
            if ( read )
                break;
block:
            if ( !blocking )
                return 0;
            Sys_SendDebugReadBytesInternal();
            NET_Sleep(1u);
        }
        if ( read == -1 )
            break;
        Sys_SendDebugReadBytes(read);
        if ( read > len
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_net.cpp", 2077, 0, "%s", "read <= remaining") )
        {
            __debugbreak();
        }
        len -= read;
        buffer += read;
    }
    err = WSAGetLastError();
    if ( err == 10035 )
        goto block;
    if ( err == 10054 )
    {
        Sys_DebugSocketError("Sys_ReadDebugSocketData: Socket closed");
    }
    else
    {
        v4 = NET_ErrorString();
        v5 = va("Sys_ReadDebugSocketData: %s", v4);
        Sys_DebugSocketError(v5);
    }
    return 0;
}

void Sys_SendDebugReadBytesInternal()
{
    const char *v0; // eax
    const char *v1; // eax

    if ( *(unsigned int *)g_debugReadBytes != g_debugReadBytesSent )
    {
        g_debugReadBytesSent = *(unsigned int *)g_debugReadBytes;
        if ( !ip_debugSocket[1]
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_net.cpp",
                        2012,
                        0,
                        "%s",
                        "ip_debugSocket[DEBUG_SOCKET_MSG_REPLY_CHANNEL]") )
        {
            __debugbreak();
        }
        if ( send(ip_debugSocket[1], g_debugReadBytes, 4, 0) == -1 )
        {
            v0 = NET_ErrorString();
            v1 = va("Sys_SendDebugReadBytes: %s", v0);
            Sys_DebugSocketError(v1);
        }
    }
}

void __cdecl Sys_SendDebugReadBytes(int read)
{
    *(unsigned int *)g_debugReadBytes += read;
    if ( *(unsigned int *)g_debugReadBytes - g_debugReadBytesSent >= 0x2000 )
        Sys_SendDebugReadBytesInternal();
}

int __cdecl Sys_ReadDebugSocketInt()
{
    int value; // [esp+0h] [ebp-4h] BYREF

    Sys_ReadDebugSocketData((char *)&value, 4, 1);
    return value;
}

char *__cdecl Sys_ReadDebugSocketString()
{
    unsigned int buffer; // [esp+0h] [ebp-100Ch] BYREF
    char in[4100]; // [esp+4h] [ebp-1008h] BYREF

    Sys_ReadDebugSocketData((char *)&buffer, 4, 1);
    if ( buffer >= 0x1000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_net.cpp",
                    2114,
                    0,
                    "%s\n\t(len) = %i",
                    "(len < sizeof( text ))",
                    buffer) )
    {
        __debugbreak();
    }
    Sys_ReadDebugSocketData(in, buffer, 1);
    in[buffer] = 0;
    return CopyString(in, "Sys_ReadDebugSocketString", 0, SCRIPTINSTANCE_SERVER);
}

void __cdecl Sys_WriteDebugSocketData(unsigned __int8 *buffer, int len)
{
    int pos; // [esp+0h] [ebp-10h]
    unsigned int copyLen; // [esp+8h] [ebp-8h]

    if ( !com_errorEntered && Sys_IsRemoteDebugServer() )
    {
        pos = g_debugPacketPos[0];
        while ( len )
        {
            if ( len <= 0
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_net.cpp", 2240, 0, "%s", "len > 0") )
            {
                __debugbreak();
            }
            copyLen = len;
            if ( len > 0x2000 - pos )
                copyLen = 0x2000 - pos;
            memcpy(&g_debugPacket[0][pos], buffer, copyLen);
            buffer += copyLen;
            len -= copyLen;
            pos += copyLen;
            g_debugPacketPos[0] = pos;
            if ( pos > 0x2000
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_net.cpp",
                            2251,
                            0,
                            "%s",
                            "pos <= MAX_DEBUG_PACKETLEN") )
            {
                __debugbreak();
            }
            if ( pos == 0x2000 )
            {
                pos = 0;
                g_debugPacketPos[0] = 0;
                Sys_DebugSend(0, (const char *)g_debugPacket, 0x2000, "Sys_WriteDebugSocketData");
            }
        }
    }
}

void __cdecl Sys_DebugSend(int channel, const char *buf, int len, const char *name)
{
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    int err; // [esp+0h] [ebp-Ch]
    int debugReadBytesRemote; // [esp+4h] [ebp-8h] BYREF
    int read; // [esp+8h] [ebp-4h]

    if ( ip_debugSocket[channel] )
    {
        g_debugWriteBytes += len;
        while ( !Sys_DebugCanSend() )
        {
            while ( 1 )
            {
                if ( !ip_debugSocket[1]
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_net.cpp",
                                2171,
                                0,
                                "%s",
                                "ip_debugSocket[DEBUG_SOCKET_MSG_REPLY_CHANNEL]") )
                {
                    __debugbreak();
                }
                read = recvfrom(ip_debugSocket[1], (char *)&debugReadBytesRemote, 4, 0, 0, 0);
                if ( read == -1 )
                    break;
                g_debugReadBytesRemote = debugReadBytesRemote;
            }
            err = WSAGetLastError();
            if ( err != 10035 )
            {
                if ( err == 10054 )
                {
                    Sys_DebugSocketError("Sys_DebugSend: Socket closed");
                }
                else
                {
                    v4 = NET_ErrorString();
                    v5 = va("Sys_DebugSend: %s", v4);
                    Sys_DebugSocketError(v5);
                }
                return;
            }
            if ( !Sys_DebugCanSend() )
                NET_Sleep(1u);
        }
        if ( !ip_debugSocket[channel]
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_net.cpp",
                        2195,
                        0,
                        "%s",
                        "ip_debugSocket[channel]") )
        {
            __debugbreak();
        }
        while ( send(ip_debugSocket[channel], buf, len, 0) == -1 )
        {
            if ( WSAGetLastError() != 10035 )
            {
                v6 = NET_ErrorString();
                v7 = va("%s: %s", name, v6);
                Sys_DebugSocketError(v7);
                return;
            }
            NET_Sleep(1u);
        }
    }
    else
    {
        Sys_DebugSocketError("Sys_DebugSend: Socket closed");
    }
}

bool __cdecl Sys_DebugCanSend()
{
    return g_debugWriteBytes - g_debugReadBytesRemote <= 40960;
}

void __cdecl Sys_WriteDebugSocketInt(int value)
{
    Sys_WriteDebugSocketData((unsigned __int8 *)&value, 4);
}

void __cdecl Sys_FlushDebugSocketData()
{
    int pos; // [esp+0h] [ebp-8h]
    int channel; // [esp+4h] [ebp-4h]

    for ( channel = 0; channel < 1; ++channel )
    {
        pos = g_debugPacketPos[channel];
        if ( pos )
        {
            g_debugPacketPos[channel] = 0;
            Sys_DebugSend(channel, (const char *)g_debugPacket[channel], pos, "Sys_FlushDebugSocketData");
        }
    }
}

bool __cdecl Sys_DebugSocketReady(int channel)
{
    return ip_debugSocket[channel] != 0;
}

void __cdecl Sys_WriteDebugSocketMessageType(unsigned __int8 type)
{
    Sys_EnterCriticalSection(CRITSECT_DEBUG_SOCKET);
    Sys_WriteDebugSocketData(&type, 1);
}

void __cdecl Sys_EndWriteDebugSocket()
{
    Sys_LeaveCriticalSection(CRITSECT_DEBUG_SOCKET);
}

void __cdecl Sys_WriteDebugSocketString(char *text)
{
    int len; // [esp+10h] [ebp-4h] BYREF

    len = strlen(text);
    Sys_WriteDebugSocketData((unsigned __int8 *)&len, 4);
    Sys_WriteDebugSocketData((unsigned __int8 *)text, len);
}

int __cdecl Sys_UpdateDebugSocket()
{
    const char *v1; // eax
    const char *v2; // eax
    unsigned __int8 type; // [esp+7h] [ebp-5h] BYREF
    int i; // [esp+8h] [ebp-4h]

    if ( sys_debugMessageType[0] )
        return sys_debugMessageType[0];
    for ( i = 0; i < 2; ++i )
    {
        if ( !ip_debugSocket[i] )
        {
            if ( !ip_debugServerSocket[i] )
                return 0;
            ip_debugSocket[i] = accept(ip_debugServerSocket[i], 0, 0);
            if ( ip_debugSocket[i] == -1 )
            {
                ip_debugSocket[i] = 0;
                if ( WSAGetLastError() != 10035 )
                {
                    v1 = NET_ErrorString();
                    v2 = va("Sys_UpdateDebugSocket: %s\n", v1);
                    Sys_DebugSocketError(v2);
                }
                return 0;
            }
        }
    }
    if ( Sys_ReadDebugSocketMessageType(&type, 0) )
    {
        if ( type >= 0x2Eu
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_net.cpp",
                        2401,
                        0,
                        "%s",
                        "(unsigned)type < DEBUG_MSG_COUNT") )
        {
            __debugbreak();
        }
        sys_debugMessageType[0] = type;
    }
    return sys_debugMessageType[0];
}

int __cdecl Sys_ReadDebugSocketMessageType(unsigned __int8 *type, int blocking)
{
    return Sys_ReadDebugSocketData((char *)type, 1, blocking);
}

void __cdecl Sys_AckDebugSocket()
{
    sys_debugMessageType[0] = 0;
}

