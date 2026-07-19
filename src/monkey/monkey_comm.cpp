#include "monkey_comm.h"

#include <winsock2.h>
#include <qcommon/common.h>
#include <universal/assertive.h>

bool g_MonkeyConnected;
SOCKET g_MonkeySock;

char __cdecl Monkey_InitCommunication(const char *server)
{
    int err; // [esp+0h] [ebp-1B4h]
    unsigned int addr; // [esp+4h] [ebp-1B0h]
    WSAData wsaData; // [esp+Ch] [ebp-1A8h] BYREF
    sockaddr_in zookeeper; // [esp+1A0h] [ebp-14h] BYREF

    if ( g_MonkeyConnected
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\monkey\\monkey_comm.cpp",
                    59,
                    0,
                    "%s",
                    "!g_MonkeyConnected") )
    {
        __debugbreak();
    }
    if ( (!server || !*server)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\monkey\\monkey_comm.cpp",
                    60,
                    0,
                    "%s",
                    "server && server[0]") )
    {
        __debugbreak();
    }
    if ( WSAStartup(0x202u, &wsaData) )
    {
        Com_Error(ERR_FATAL, "Error at WSAStartup() for monkey");
        return 0;
    }
    else
    {
        addr = inet_addr(server);
        g_MonkeySock = socket(2, 1, 6);
        if ( g_MonkeySock == -1 )
        {
            Com_Error(ERR_FATAL, "Error creating socket for monkey");
            return 0;
        }
        else
        {
            memset(&zookeeper.sin_port, 0, 14);
            zookeeper.sin_family = 2;
            zookeeper.sin_port = htons(7000);
            zookeeper.sin_addr.s_addr = addr;
            if ( connect(g_MonkeySock, (const struct sockaddr *)&zookeeper, 16) )
            {
                err = WSAGetLastError();
                Com_Error(
                    ERR_FATAL,
                    "Monkey could not open a connection to zookeeper '%s': WSAGetLastError returned %d",
                    server,
                    err);
                return 0;
            }
            else
            {
                g_MonkeyConnected = 1;
                return 1;
            }
        }
    }
}

void __cdecl Monkey_ShutdownCommunication()
{
    int ret; // [esp+0h] [ebp-8Ch]
    char buf[132]; // [esp+4h] [ebp-88h] BYREF

    if ( g_MonkeyConnected )
    {
        shutdown(g_MonkeySock, 1);
        do
            ret = recv(g_MonkeySock, buf, 128, 0);
        while ( ret && ret != -1 );
        closesocket(g_MonkeySock);
        g_MonkeyConnected = 0;
    }
}

void __cdecl    Monkey_DisconnectAndExit()
{
    Monkey_ShutdownCommunication();
    ExitProcess(0xFFFFFFFF);
}

void __cdecl Monkey_Send(const char *type, const char *string)
{
    if ( g_MonkeyConnected )
    {
        Monkey_SendString(type);
        Monkey_SendString(string);
    }
}

void __cdecl Monkey_SendString(const char *string)
{
    int nlength; // [esp+10h] [ebp-10h] BYREF
    int bytesLeft; // [esp+14h] [ebp-Ch]
    int sent; // [esp+18h] [ebp-8h]
    int length; // [esp+1Ch] [ebp-4h]

    length = strlen(string);
    nlength = htonl(length);
    sent = send(g_MonkeySock, (const char *)&nlength, 4, 0);
    if ( !sent )
        Monkey_DisconnectAndExit();
    bytesLeft = length;
    while ( bytesLeft > 0 )
    {
        sent = send(g_MonkeySock, &string[length - bytesLeft], bytesLeft, 0);
        bytesLeft -= sent;
        if ( !sent )
            Monkey_DisconnectAndExit();
    }
}

bool __cdecl Monkey_Recv(char *type, int type_size, char *string, int string_size)
{
    return Monkey_RecvString(type, type_size) && Monkey_RecvString(string, string_size);
}

char __cdecl Monkey_RecvString(char *data, int data_size)
{
    int length; // [esp+4h] [ebp-4h] BYREF

    if ( !data && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\monkey\\monkey_comm.cpp", 242, 0, "%s", "data") )
        __debugbreak();
    if ( data_size <= 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\monkey\\monkey_comm.cpp", 243, 0, "%s", "data_size > 0") )
    {
        __debugbreak();
    }
    if ( !Monkey_SocketHasData() )
        return 0;
    if ( recv(g_MonkeySock, (char *)&length, 4, 0) <= 0 )
        Monkey_DisconnectAndExit();
    length = ntohl(length);
    if ( data_size < length + 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\monkey\\monkey_comm.cpp",
                    258,
                    0,
                    "%s",
                    "data_size >= length+1") )
    {
        __debugbreak();
    }
    if ( !recv(g_MonkeySock, data, length, 0) )
        Monkey_DisconnectAndExit();
    data[length] = 0;
    return 1;
}

bool __cdecl Monkey_SocketHasData()
{
    int avail; // [esp+8h] [ebp-21Ch]
    fd_set readfds; // [esp+Ch] [ebp-218h] BYREF
    timeval timeout; // [esp+114h] [ebp-110h] BYREF
    fd_set errorfds; // [esp+11Ch] [ebp-108h] BYREF

    if ( !g_MonkeyConnected )
        return 0;
    // The decompiled original hand-rolled Win32 fd_set (fd_count/fd_array); on POSIX
    // use the FD_* macros, which build the same single-socket read/error sets.
    FD_ZERO(&readfds);  FD_SET(g_MonkeySock, &readfds);
    FD_ZERO(&errorfds); FD_SET(g_MonkeySock, &errorfds);
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    avail = select(g_MonkeySock + 1, &readfds, 0, &errorfds, &timeout);
    if ( avail == -1 || (avail && FD_ISSET(g_MonkeySock, &errorfds)) )
        Monkey_DisconnectAndExit();
    return avail && FD_ISSET(g_MonkeySock, &readfds);
}

