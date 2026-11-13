#include "cl_ui_pc_mp.h"
#include <universal/offline.h>
#include "cl_main_pc_mp.h"

int __cdecl LAN_AddServer(int source, const char *name, const char *address)
{
    int i; // [esp+4h] [ebp-20h]
    serverInfo_t *servers; // [esp+8h] [ebp-1Ch] BYREF
    int *count; // [esp+Ch] [ebp-18h] BYREF
    netadr_t adr; // [esp+10h] [ebp-14h] BYREF

    servers = 0;
    if ( !CL_GetServerList(source, &servers, &count) || !servers || *count >= 128 )
        return -1;
    if ( !NET_StringToAdr(address, &adr) )
        return -2;
    for ( i = 0; i < *count && !NET_CompareAdr(servers[i].adr, adr); ++i )
        ;
    if ( i < *count )
        return 0;
    servers[*count].adr = adr;
    I_strncpyz(servers[*count].hostName, name, 32);
    servers[*count].dirty = 1;
    servers[(*count)++].gotInfo = 0;
    return 1;
}

void __cdecl LAN_GetServerAddressString(int source, int n, char *buf, int buflen)
{
    char *v4; // eax
    serverInfo_t *servers; // [esp+0h] [ebp-8h] BYREF
    int *count; // [esp+4h] [ebp-4h] BYREF

    if ( !buf && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_ui_pc_mp.cpp", 108, 0, "%s", "buf") )
        __debugbreak();
    servers = 0;
    if ( CL_GetServerList(source, &servers, &count) && n > 0 && n < *count )
    {
        v4 = NET_AdrToString(servers[n].adr);
        I_strncpyz(buf, v4, buflen);
    }
    *buf = 0;
}

void __cdecl LAN_GetServerSecurityId(int source, int n, XNKID *buf, int buflen)
{
    serverInfo_t *servers; // [esp+4h] [ebp-8h] BYREF
    int *count; // [esp+8h] [ebp-4h] BYREF

    if ( !buf && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_ui_pc_mp.cpp", 126, 0, "%s", "buf") )
        __debugbreak();
    if ( buflen < 8
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_ui_pc_mp.cpp",
                    127,
                    0,
                    "%s",
                    "buflen >= XNKID_LENGTH") )
    {
        __debugbreak();
    }
    servers = 0;
    if ( CL_GetServerList(source, &servers, &count) && servers && n < *count )
        *buf = servers[n].xnkid;
}

void __cdecl LAN_ResetPings(int source)
{
    int i; // [esp+0h] [ebp-Ch]
    int *count; // [esp+4h] [ebp-8h] BYREF
    serverInfo_t *servers; // [esp+8h] [ebp-4h] BYREF

    servers = 0;
    if ( CL_GetServerList(source, &servers, &count) && servers )
    {
        cls.pingedServerCount = 0;
        for ( i = 0; i < *count; ++i )
        {
            servers[i].ping = -1;
            servers[i].pingedTime = 0;
            servers[i].gotInfo = 0;
        }
    }
}

int __cdecl LAN_GetServerCount(int source)
{
    serverInfo_t *servers; // [esp+0h] [ebp-8h] BYREF
    int *count; // [esp+4h] [ebp-4h] BYREF

    servers = 0;
    if ( CL_GetServerList(source, &servers, &count) && servers )
        return *count;
    else
        return 0;
}

int __cdecl LAN_WaitServerResponse()
{
    return cls.waitfindsessionsresponse;
}

void __cdecl LAN_GetServerInfo(int source, int n, char *buf, int buflen)
{
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    const char *v9; // eax
    char *v10; // eax
    const char *v11; // eax
    const char *v12; // eax
    const char *v13; // eax
    const char *v14; // eax
    const char *v15; // eax
    const char *v16; // eax
    const char *v17; // eax
    const char *v18; // eax
    const char *v19; // eax
    const char *v20; // eax
    const char *v21; // eax
    const char *v22; // eax
    const char *v23; // eax
    const char *v24; // eax
    const char *v25; // eax
    const char *v26; // eax
    int v27; // eax
    int v28; // eax
    unsigned __int64 v29; // [esp-Ch] [ebp-DA4h]
    FriendInfo finfo; // [esp+0h] [ebp-D98h] BYREF
    unsigned __int64 userIDs[256]; // [esp+B8h] [ebp-CE0h] BYREF
    char *strptr; // [esp+8B8h] [ebp-4E0h]
    int i; // [esp+8BCh] [ebp-4DCh]
    int numFriends; // [esp+8C0h] [ebp-4D8h] BYREF
    int stringpos; // [esp+8C4h] [ebp-4D4h]
    char userString[128]; // [esp+8C8h] [ebp-4D0h] BYREF
    char xuidstring[64]; // [esp+948h] [ebp-450h] BYREF
    char info[1024]; // [esp+988h] [ebp-410h] BYREF
    serverInfo_t *server; // [esp+D8Ch] [ebp-Ch]
    serverInfo_t *servers; // [esp+D90h] [ebp-8h] BYREF
    int *count; // [esp+D94h] [ebp-4h] BYREF

    servers = 0;
    server = 0;
    info[0] = 0;
    if ( CL_GetServerList(source, &servers, &count) && servers && n < *count )
        server = &servers[n];
    if ( server && buf )
    {
        *buf = 0;
        Info_SetValueForKey(info, "hostname", server->hostName);
        Info_SetValueForKey(info, "mapname", server->mapName);
        v4 = va("%i", server->clients);
        Info_SetValueForKey(info, "clients", v4);
        v5 = va("%i", server->maxClients);
        Info_SetValueForKey(info, "com_maxclients", v5);
        v6 = va("%i", server->ping);
        Info_SetValueForKey(info, "ping", v6);
        v7 = va("%i", server->minPing);
        Info_SetValueForKey(info, "minping", v7);
        v8 = va("%i", server->maxPing);
        Info_SetValueForKey(info, "maxping", v8);
        Info_SetValueForKey(info, "game", server->game);
        Info_SetValueForKey(info, "gametype", server->gameType);
        v9 = va("%i", server->netType);
        Info_SetValueForKey(info, "nettype", v9);
        v10 = NET_AdrToString(server->adr);
        Info_SetValueForKey(info, "addr", v10);
        v11 = va("%i", server->bPassword);
        Info_SetValueForKey(info, "pswrd", v11);
        v12 = va("%i", server->consoleDisabled);
        Info_SetValueForKey(info, "con_disabled", v12);
        v13 = va("%i", server->pure);
        Info_SetValueForKey(info, "pure", v13);
        v14 = va("%i", server->allowAnonymous);
        Info_SetValueForKey(info, "sv_allowAnonymous", v14);
        v15 = va("%i", server->friendlyfire);
        Info_SetValueForKey(info, "ff", v15);
        v16 = va("%i", server->killcam);
        Info_SetValueForKey(info, "kc", v16);
        v17 = va("%i", server->hardcore);
        Info_SetValueForKey(info, "hc", v17);
        v18 = va("%i", server->hardware);
        Info_SetValueForKey(info, "hw", v18);
        v19 = va("%i", server->mod);
        Info_SetValueForKey(info, "mod", v19);
        v20 = va("%i", server->voice);
        Info_SetValueForKey(info, "voice", v20);
        v21 = va("%i", server->punkbuster);
        Info_SetValueForKey(info, "pb", v21);
        v22 = va("%i", server->wagerBet);
        Info_SetValueForKey(info, "wagerbet", v22);
        v23 = va("%i", server->playlist);
        Info_SetValueForKey(info, "playlist", v23);
        v24 = va("%i", server->region);
        Info_SetValueForKey(info, "region", v24);
        Info_SetValueForKey(info, "geolocation", server->city);
        v25 = va("%i", server->wagerBet > 0);
        Info_SetValueForKey(info, "wager", v25);
        v26 = va("%i", server->licensetype);
        Info_SetValueForKey(info, "licensetype", v26);
        Info_SetValueForKey(info, "countrycode", server->countrycode);
        memset(xuidstring, 0, sizeof(xuidstring));
        XUIDToString(&server->serverUID, xuidstring);
        Info_SetValueForKey(info, "dwuserid", xuidstring);
        if ( source == 4 )
        {
            Live_GetFriendsOnServer(server->serverUID, userIDs, &numFriends);
            stringpos = 0;
            strptr = userString;
            for ( i = 0; i < numFriends && i < 3; ++i )
            {
                HIDWORD(v29) = HIDWORD(userIDs[i]);
                LODWORD(v29) = userIDs[i];
                Friends_GetByID(0, v29, &finfo);
                v27 = sprintf_s(strptr, 128 - stringpos, "%s\n", finfo.name);
                stringpos += v27;
                strptr = &userString[stringpos];
            }
            if ( numFriends <= 3 )
            {
                if ( *--strptr == 10 )
                    *strptr = 0;
            }
            else
            {
                v28 = sprintf_s(strptr, 128 - stringpos, "...");
                stringpos += v28;
                strptr = &userString[stringpos];
            }
            Info_SetValueForKey(info, "friendsOnServer", userString);
        }
        I_strncpyz(buf, info, buflen);
    }
    else if ( buf )
    {
        *buf = 0;
    }
}

int __cdecl LAN_GetServerPing(int source, int n)
{
    serverInfo_t *server; // [esp+0h] [ebp-Ch]
    serverInfo_t *servers; // [esp+4h] [ebp-8h] BYREF
    int *count; // [esp+8h] [ebp-4h] BYREF

    server = 0;
    servers = 0;
    if ( CL_GetServerList(source, &servers, &count) && servers && n < *count && n >= 0 )
        server = &servers[n];
    if ( server )
        return server->ping;
    else
        return -1;
}

serverInfo_t *__cdecl LAN_GetServerPtr(int source, int n)
{
    serverInfo_t *server; // [esp+0h] [ebp-Ch]
    serverInfo_t *servers; // [esp+4h] [ebp-8h] BYREF
    int *count; // [esp+8h] [ebp-4h] BYREF

    server = 0;
    servers = 0;
    if ( CL_GetServerList(source, &servers, &count) && servers && n < *count && n >= 0 )
        return &servers[n];
    return server;
}

void __cdecl LAN_CleanHostname(const char *hostName, char *cleanHostName)
{
    unsigned __int8 c; // [esp+3h] [ebp-1h]

    while ( 1 )
    {
        c = *hostName;
        if ( !*hostName )
            break;
        if ( isalpha(c) )
            *cleanHostName++ = c;
        ++hostName;
    }
    *cleanHostName = 0;
}

int __cdecl LAN_CompareHostname(const char *hostName1, const char *hostName2)
{
    char cleanHostName2[32]; // [esp+4h] [ebp-48h] BYREF
    char cleanHostName1[32]; // [esp+24h] [ebp-28h] BYREF
    int res; // [esp+48h] [ebp-4h]

    LAN_CleanHostname(hostName1, cleanHostName1);
    LAN_CleanHostname(hostName2, cleanHostName2);
    res = I_stricmp(cleanHostName1, cleanHostName2);
    if ( res )
        return res;
    else
        return I_stricmp(hostName1, hostName2);
}

int __cdecl LAN_CompareServers(int source, int sortKey, int sortDir, int s1, int s2)
{
    char *v6; // eax
    char *v7; // eax
    char *MapDisplayName; // [esp-4h] [ebp-18h]
    char *GameTypeDisplayName; // [esp-4h] [ebp-18h]
    serverInfo_t *server1; // [esp+8h] [ebp-Ch]
    serverInfo_t *server2; // [esp+Ch] [ebp-8h]
    int res; // [esp+10h] [ebp-4h]

    server1 = LAN_GetServerPtr(source, s1);
    server2 = LAN_GetServerPtr(source, s2);
    if ( !server1 || !server2 )
        return 0;
    res = 0;
    switch ( sortKey )
    {
        case 0:
            res = server1->licensetype - server2->licensetype;
            if ( !res )
                goto sort_ping;
            break;
        case 1:
            res = server1->punkbuster - server2->punkbuster;
            if ( !res )
                goto sort_ping;
            break;
        case 2:
            res = server1->bPassword - server2->bPassword;
            if ( !res )
                goto sort_ping;
            break;
        case 3:
            res = LAN_CompareHostname(server1->hostName, server2->hostName);
            if ( !res )
                goto sort_ping;
            break;
        case 4:
            MapDisplayName = UI_GetMapDisplayName(server2->mapName);
            v6 = UI_GetMapDisplayName(server1->mapName);
            res = I_stricmp(v6, MapDisplayName);
            if ( !res )
                goto sort_ping;
            break;
        case 5:
            res = server1->clients - server2->clients;
            if ( !res )
                goto sort_ping;
            break;
        case 6:
            GameTypeDisplayName = UI_GetGameTypeDisplayName(server2->gameType);
            v7 = UI_GetGameTypeDisplayName(server1->gameType);
            res = I_stricmp(v7, GameTypeDisplayName);
            if ( !res )
                goto sort_ping;
            break;
        case 7:
            res = server1->region - server2->region;
            if ( !res )
                goto sort_ping;
            break;
        case 8:
            goto $LN3_50;
        case 9:
            res = server1->mod - server2->mod;
            if ( res )
                break;
            goto sort_ping;
        case 10:
            res = server1->voice - server2->voice;
            if ( res )
                break;
sort_ping:
            sortDir = 0;
$LN3_50:
            res = server1->ping - server2->ping;
            if ( !res )
            {
                res = I_stricmp(server1->gameType, server2->gameType);
                if ( !res )
                    res = LAN_CompareHostname(server1->hostName, server2->hostName);
            }
            break;
        default:
            break;
    }
    if ( sortDir )
        return -res;
    else
        return res;
}

void __cdecl LAN_MarkServerDirty(int source, int n, unsigned __int8 dirty)
{
    int *count; // [esp+0h] [ebp-8h] BYREF
    serverInfo_t *servers; // [esp+4h] [ebp-4h] BYREF
    int na; // [esp+14h] [ebp+Ch]

    if ( CL_GetServerList(source, &servers, &count) && servers )
    {
        if ( n == -1 )
        {
            for ( na = 0; na < *count; ++na )
                servers[na].dirty = dirty;
        }
        else
        {
            servers[n].dirty = dirty;
        }
    }
}

int __cdecl LAN_ServerIsDirty(int source, int n)
{
    int *count; // [esp+0h] [ebp-8h] BYREF
    serverInfo_t *servers; // [esp+4h] [ebp-4h] BYREF

    if ( CL_GetServerList(source, &servers, &count) && servers && n >= 0 && n < *count )
        return servers[n].dirty;
    else
        return 0;
}

int __cdecl LAN_UpdateDirtyPings(int localClientNum, unsigned int source)
{
    return CL_UpdateDirtyPings(localClientNum, source);
}

int __cdecl LAN_GetServerStatus(char *serverSecurityID, char *serverStatus, int maxLen)
{
    return CL_ServerStatus(serverSecurityID, serverStatus, maxLen);
}

int __cdecl LAN_GetServerStatusScoreBoard(char *serverSecurityID, char *serverStatus, int maxLen)
{
    return CL_ServerStatusScoreBoard(serverSecurityID, serverStatus, maxLen);
}

void __cdecl CLUI_SetPbClStatus(int status)
{
    //if ( status )
    //{
    //    EnablePbCl();
    //    if ( !isPbClEnabled() )
    //        Com_SetErrorMessage("MPUI_NOPUNKBUSTER");
    //}
    //else
    //{
    //    DisablePbCl();
    //}
}

