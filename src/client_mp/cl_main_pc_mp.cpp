#include "cl_main_pc_mp.h"

#include <universal/offline.h>

#include <qcommon/net_chan_mp.h>
#include <client/cl_main.h>
#include <cgame_mp/cg_newDraw_mp.h>
#include <cgame_mp/cg_main_mp.h>
#include <client/client.h>
#include <sound/snd_public_async.h>
#include <server_mp/sv_main_mp.h>
#include <universal/com_constantconfigstrings.h>
#include <qcommon/com_clients.h>
#include <client/cl_console.h>
#include <client/splitscreen.h>
#include "cl_cgame_mp.h"
#include <ui/ui_main_pc.h>
#include <qcommon/com_gamemodes.h>
#include <universal/com_files.h>
#include <win32/win_net.h>
#include "cl_scrn_mp.h"
#include <win32/win_shared.h>

#include <qcommon/dl_main.h>
#include <qcommon/legacyhacks.h>
#include <qcommon/md4.h>
#include <win32/win_steam.h>


const dvar_t *cl_quickmatch_resultspercent;
const dvar_t *cl_quickmatch_pingweight;
const dvar_t *cl_quickmatch_fullnessweight;
const dvar_t *cl_wager_firstplaylist;
const dvar_t *cl_wager_lastplaylist;
const dvar_t *cl_wager_maxping;

unsigned int s_numServers;
int serverStatusCount;
serverStatusInfoResponse_s cl_serverStatusList[16];
serverStatusInfoResponse_s cl_serverStatusScoreBoardList[16];
bool s_playerMute[32];
serverInfo_t s_quickmatchCandidates[50];
quickmatchstate_t s_quickmatchstate;

struct cacvalidateserver_t // sizeof=0x8
{                                       // XREF: .data:cacvalidateserver_t g_cacValidateServer/r
    unsigned __int64 uid;               // XREF: CL_CACValidateServerMatches+B/r
};
cacvalidateserver_t g_cacValidateServer;

int g_cacValidateTimeout;

enum cacvalidatestate_t : __int32
{
    CAC_DORMANT      = 0x0,
    CAC_DONE         = 0x1,
    CAC_NOTVALIDATED = 0x2,
    CAC_FINDING      = 0x3,
    CAC_FAILED       = 0x4,
    CAC_FOUND        = 0x5,
    CAC_REQUESTSENT  = 0x6,
    CAC_VALIDATED    = 0x7,
    CAC_REJECTED     = 0x8,
};
cacvalidatestate_t g_cacValidateState; // NOT TO BE CONFUSED WITH LOWERCASE `g_cacvalidateState` !!! ! ! !

struct  //$A3082F8D06891D11850E9B8F334529D3 // sizeof=0x28
{                                       // XREF: .data:rconGlob/r
    char password[24];                  // XREF: CL_RconInit(void)+3/w
                                        // CL_Rcon_f(void):loc_5696BB/r ...
    netadr_t host;                      // XREF: CL_GetLastRconAddress(void)+3/o
                                        // CL_RconInit(void)+A/w ...
} rconGlob;

netadr_t *__cdecl CL_GetLastRconAddress()
{
    return &rconGlob.host;
}

void __cdecl CL_SetServerInfo(serverInfo_t *server, char *info, __int16 ping)
{
    char *v3; // eax
    char *v4; // eax
    char *v5; // eax
    char *v6; // eax
    char *v7; // eax
    char *v8; // eax
    char *v9; // eax
    char *v10; // eax
    char *v11; // eax
    char *v12; // eax
    char *v13; // eax
    char *v14; // eax
    char *v15; // eax
    char *v16; // eax
    char *v17; // eax
    char *v18; // eax
    char *v19; // eax
    char *v20; // eax
    char *v21; // eax
    char *v22; // eax
    char *v23; // eax
    char *countrycode; // [esp+0h] [ebp-8h]
    char *longname; // [esp+4h] [ebp-4h]

    if ( server )
    {
        if ( info )
        {
            v3 = Info_ValueForKey(info, "nettype");
            server->netType = atoi(v3);
            v4 = Info_ValueForKey(info, "minping");
            server->minPing = atoi(v4);
            v5 = Info_ValueForKey(info, "maxping");
            server->maxPing = atoi(v5);
            v6 = Info_ValueForKey(info, "sv_allowAnonymous");
            server->allowAnonymous = atoi(v6);
            v7 = Info_ValueForKey(info, "con_disabled");
            server->consoleDisabled = atoi(v7);
            v8 = Info_ValueForKey(info, "pswrd");
            server->bPassword = atoi(v8);
            v9 = Info_ValueForKey(info, "pure");
            server->pure = atoi(v9);
            v10 = Info_ValueForKey(info, "ff");
            server->friendlyfire = atoi(v10);
            v11 = Info_ValueForKey(info, "kc");
            server->killcam = atoi(v11);
            v12 = Info_ValueForKey(info, "hc");
            server->hardcore = atoi(v12);
            v13 = Info_ValueForKey(info, "hw");
            server->hardware = atoi(v13);
            v14 = Info_ValueForKey(info, "mod");
            server->mod = atoi(v14);
            v15 = Info_ValueForKey(info, "wagerbet");
            server->wagerBet = atoi(v15);
            v16 = Info_ValueForKey(info, "playlist");
            server->playlist = atoi(v16);
            v17 = Info_ValueForKey(info, "clients");
            server->clients = atoi(v17);
            v18 = Info_ValueForKey(info, "voice");
            server->voice = atoi(v18);
            v19 = Info_ValueForKey(info, "pb");
            server->punkbuster = atoi(v19) > 0;
            v20 = Info_ValueForKey(info, "basic");
            server->basictraining = atoi(v20) > 0;
            v21 = Info_ValueForKey(info, "customclass");
            server->customclassmode = atoi(v21) > 0;
            memset(server->city, 0, sizeof(server->city));
            v22 = Info_ValueForKey(info, "geolocation");
            longname = CL_LongNameForShortName(v22);
            if ( longname )
            {
                I_strncpyz(server->city, longname, 64);
            }
            else
            {
                v23 = Info_ValueForKey(info, "geolocation");
                I_strncpyz(server->city, v23, 64);
            }
            if ( server->licensetype == 5 )
            {
                countrycode = Info_ValueForKey(info, "countrycode");
                I_strncpyz(server->countrycode, countrycode, 3);
            }
        }
        server->gotInfo = 1;
        server->ping = ping;
    }
}

void __cdecl CL_ServerInfoPacket(XNKID *secID, msg_t *msg, int time)
{
#if 0 // KISAKTODO
    char *v3; // eax
    int v4; // [esp+0h] [ebp-424h]
    int prot; // [esp+4h] [ebp-420h]
    int *numServers; // [esp+8h] [ebp-41Ch] BYREF
    char strBuf[1024]; // [esp+Ch] [ebp-418h] BYREF
    const char *ptr; // [esp+410h] [ebp-14h]
    serverInfo_t *server; // [esp+414h] [ebp-10h]
    char *infoString; // [esp+418h] [ebp-Ch]
    serverInfo_t *servers; // [esp+41Ch] [ebp-8h] BYREF
    int pingTime; // [esp+420h] [ebp-4h]

    CL_GetServerList(cls.pingUpdateSource, &servers, &numServers);
    if ( servers )
    {
        infoString = MSG_ReadString(msg, strBuf, 0x400u);
        v3 = Info_ValueForKey(infoString, "protocol");
        prot = atoi(v3);
        ptr = Dvar_GetString("debug_protocol");
        if ( *ptr )
            v4 = atoi(ptr);
        else
            v4 = 1044;
        if ( prot == v4 )
        {
            server = FindServerBySecID(secID);
            if ( server )
            {
                if ( server->pingedTime )
                {
                    pingTime = time - server->pingedTime;
                    if ( pingTime >= 0 )
                    {
                        Com_DPrintf(14, "ping time %dms from %s\n", pingTime, server->hostName);
                        CL_SetServerInfo(server, infoString, pingTime);
                    }
                }
            }
            else
            {
                Com_DPrintf(14, "Got ping response from %llu, but no match was found. Hax?\n", *secID);
            }
        }
        else
        {
            Com_DPrintf(14, "Different protocol info packet: %s\n", infoString);
        }
    }
#endif
}

void __cdecl CL_Connect(serverInfo_t *server)
{
    int ControllerIndex; // eax
    __int16 v2; // ax
    char *v3; // eax
    clientUIActive_t *clUI; // [esp+Ch] [ebp-18h]
    clientConnection_t *clc; // [esp+10h] [ebp-14h]


    clUI = CL_GetLocalClientUIGlobals(0);
    CL_GetLocalClientGlobals(0);
    clc = CL_GetLocalClientConnection(0);
    clc->serverMessage[0] = 0;
    if ( CL_GetLocalClientConnectionState(0) < 6 )
    {
        Dvar_SetBoolByName("cl_wasconnected", 0);
    }
    else
    {
        Dvar_SetBoolByName("cl_wasconnected", 1);
        SND_StopSounds(SND_STOP_ALL);
        clUI->keyCatchers = 0;
    }
    CL_SetupClientsForIngame();
    CL_AllocatePerLocalClientMemory();
    SV_KillLocalServer();
    CCS_ClearConstantConfigStrings();
    cl_serverLoadingMap = 0;
    ControllerIndex = Com_LocalClient_GetControllerIndex(0);
    SV_Frame(ControllerIndex, 0);
    Cmd_ExecuteSingleCommand(0, 0, (char*)"fileShareAbortOperation");
    CL_Disconnect(0, 1);
    Con_Close(0);
    I_strncpyz(cls.servername, server->hostName, 256);
    clc->serverAddress = server->adr;
    if ( !clc->serverAddress.port )
        clc->serverAddress.port = BigShort(3074);
    v2 = BigShort(clc->serverAddress.port);
    Com_Printf(
        0,
        "%s resolved to %i.%i.%i.%i:%i\n",
        cls.servername,
        clc->serverAddress.ip[0],
        clc->serverAddress.ip[1],
        clc->serverAddress.ip[2],
        clc->serverAddress.ip[3],
        v2);
    if ( NET_IsLocalAddress(clc->serverAddress) || CL_LocalClient_GetActiveCount() )
    {
        if ( NET_IsLocalAddress(clc->serverAddress) )
            CL_SetLocalClientConnectionState(0, CA_CHALLENGING);
        else
            CL_SetLocalClientConnectionState(0, CA_CONNECTING);
        CL_LocalClient_ClearCUIFlag(0, 64);
        clc->connectTime = -99999;
        clc->connectPacketCount = 0;
        clc->clientChallenge = 0;
        clc->nonce = 0;
        if ( server->basictraining )
        {
            Dvar_SetBool((dvar_s *)xblive_basictraining, 1);
        }
        else
        {
            LiveStorage_SetFirstTimeRunning(0);
            if ( !xblive_basictraining->current.enabled )
                CL_CACValidateRequest_f();
        }
        if ( server->customclassmode )
            Dvar_SetBoolByName("custom_class_mode", 1);
        UI_UseAltColorPalette(cls.servername);
        clc->qport = g_qport;
        cls.serveruid = server->serverUID;
        UI_ClearErrors();
        v3 = UI_SafeTranslateString("MENU_CONNECTING_CAPS");
        Dvar_SetStringByName("statusinfo_popmenuTitle", v3);
        Dvar_SetStringByName("statusinfo_popmenuMessage", cls.servername);
        Dvar_SetStringByName("statusinfo_onEscArg", (char*)"disconnect;");
        UI_OpenMenu(0, "code_statusinfo_popmenu");
        //TaskResult::~TaskResult(&sessionID);
    }
    else
    {
        Com_Error(ERR_DROP, "EXE_ERR_INVALID_CD_KEY");
        //TaskResult::~TaskResult(&sessionID);
    }
}

void __cdecl CL_ConnectHackOnline()
{
    const char *v0; // eax

    v0 = va("connect %s\n", cls.servername);
    Cbuf_AddText(0, v0);
}

bool __cdecl CL_CDKeyValidate(netadr_t addr)
{
#ifdef WIN32
    return Steam_UpdateClientAuthTicket(addr);
#else
#error Steam Auth for Arch
    return false;
#endif
}

void __cdecl CL_Connect_f()
{
    int ControllerIndex; // eax
    __int16 v1; // ax
    clientUIActive_t *clUI; // [esp+28h] [ebp-Ch]
    clientConnection_t *clc; // [esp+2Ch] [ebp-8h]
    const char *server; // [esp+30h] [ebp-4h]

    if ( Cmd_Argc() == 2 )
    {
        SND_StopSounds(SND_STOP_ALL);
        CL_AllocatePerLocalClientMemory();
        clUI = CL_GetLocalClientUIGlobals(0);
        CL_GetLocalClientGlobals(0);
        clc = CL_GetLocalClientConnection(0);
        clc->serverMessage[0] = 0;
        server = Cmd_Argv(1);
        if ( !strcmp(server, "localhost") )
            SV_KillLocalServer();
        cl_serverLoadingMap = 0;
        g_waitingForServer = 0;
        FS_DisablePureCheck(0);
        ControllerIndex = Com_LocalClient_GetControllerIndex(0);
        SV_Frame(ControllerIndex, 0);
        CL_Disconnect(0, 1);
        Con_Close(0);
        I_strncpyz(cls.servername, server, 256);
        if ( NET_StringToAdr(cls.servername, &clc->serverAddress) )
        {
            if ( !clc->serverAddress.port )
                clc->serverAddress.port = BigShort(3074);
            v1 = BigShort(clc->serverAddress.port);
            Com_Printf(
                0,
                "%s resolved to %i.%i.%i.%i:%i\n",
                cls.servername,
                clc->serverAddress.ip[0],
                clc->serverAddress.ip[1],
                clc->serverAddress.ip[2],
                clc->serverAddress.ip[3],
                v1);
            if ( NET_IsLocalAddress(clc->serverAddress) || CL_CDKeyValidate(clc->serverAddress) ) // LWSS ADD CDKey for steam
            {
                if ( Sys_IsLANAddress(clc->serverAddress) 
                    )
                {
                    if ( NET_IsLocalAddress(clc->serverAddress) )
                        CL_SetLocalClientConnectionState(0, CA_CHALLENGING);
                    else
                        CL_SetLocalClientConnectionState(0, CA_CONNECTING);
                    clUI->keyCatchers = 0;
                    CL_LocalClient_ClearCUIFlag(0, 64);
                    clc->connectTime = -99999;
                    clc->connectPacketCount = 0;
                    clc->qport = g_qport;
                    Cbuf_ExecuteBuffer(0, 0, (char*)"selectStringTableEntryInDvar mp/didyouknow.csv 0 didyouknow");
                    UI_CloseAll(0);
                    UI_ClearErrors();
                    SCR_UpdateLoadScreen();
                }
                else
                {
                    _Dvar_RegisterBool("dw_connectafterlogin", 1, 0x40u, "");
                    CL_SetLocalClientConnectionState(0, CA_DISCONNECTED);
                }
            }
            else
            {
                Com_Error(ERR_DROP, "EXE_ERR_INVALID_CD_KEY");
            }
        }
        else
        {
            Com_Printf(0, "Bad server address\n");
            CL_SetLocalClientConnectionState(0, CA_DISCONNECTED);
            Com_Error(ERR_DISCONNECT, "Bad server address");
        }
    }
    else
    {
        Com_Printf(0, "usage: connect [server]\n");
    }
}

void __cdecl CL_PC_SignInLive()
{
    //BLOPS_NULLSUB();
}

void __cdecl CL_InitServerInfo(serverInfo_t *server, netadr_t adr)
{
    server->adr = adr;
    server->pingedTime = 0;
    server->gotInfo = 0;
    server->clients = 0;
    server->hostName[0] = 0;
    server->mapName[0] = 0;
    server->maxClients = 0;
    server->maxPing = 0;
    server->minPing = 0;
    server->ping = -1;
    server->game[0] = 0;
    server->gameType[0] = 0;
    server->netType = 0;
    server->allowAnonymous = 0;
    server->dirty = 1;
    server->requestCount = 0;
    server->basictraining = 0;
    server->lastRequestTime = 0;
    server->city[0] = 0;
    server->licensetype = 0;
}

int __cdecl CL_RawPingSetupBuffer(
                unsigned __int8 *buffer,
                int buffersize,
                unsigned __int8 opcode,
                const XNKID *secID)
{
    if ( buffersize >= 9 )
    {
        *buffer = opcode;
        *(XNKID *)(buffer + 1) = *secID;
        return 9;
    }
    else
    {
        Com_PrintWarning(0, "Rawpingbuffersetup: buffer is too small.\n");
        return 0;
    }
}

void __cdecl CL_RawPingServer(serverInfo_t *server, unsigned __int8 opcode)
{
}

void __cdecl CL_ServersResponsePacket(MatchMakingInfo *mminfo, int numResults, bool geo)
{
}

void __cdecl CL_FindServers_f()
{
}

void __cdecl CL_RconInit()
{
    rconGlob.password[0] = 0;
    rconGlob.host.type = NA_BAD;
}

void __cdecl CL_Rcon_f()
{
    const char *v0; // eax
    netsrc_t NetworkID; // eax
    netadr_t v2; // [esp-18h] [ebp-474h]
    int v3; // [esp-Ch] [ebp-468h]
    int v4; // [esp-8h] [ebp-464h]
    connstate_t connstate; // [esp+30h] [ebp-42Ch]
    char message[1024]; // [esp+34h] [ebp-428h] BYREF
    int maxlen; // [esp+434h] [ebp-28h]
    int len; // [esp+438h] [ebp-24h]
    const clientConnection_t *clc; // [esp+43Ch] [ebp-20h]
    int i; // [esp+440h] [ebp-1Ch]
    netadr_t to; // [esp+444h] [ebp-18h]
    const char *cmd; // [esp+458h] [ebp-4h]

    if ( Cmd_Argc() < 2 )
    {
        Com_Printf(0, "USAGE: rcon <command> <options...>\n");
        return;
    }
    cmd = Cmd_Argv(1);
    if ( !I_stricmp(cmd, "login") )
    {
        CL_RconLogin();
        return;
    }
    if ( !I_stricmp(cmd, "logout") )
    {
        CL_RconLogout();
        return;
    }
    if ( !I_stricmp(cmd, "host") )
    {
        CL_RconHost();
        return;
    }
    if ( !rconGlob.password[0] )
    {
        Com_Printf(0, "You need to log in with 'rcon login <password>' before using rcon.\n");
        return;
    }
    maxlen = 1024;
    len = Com_AddToString("rcon ", message, 0, 1024, 0);
    len = Com_AddToString(rconGlob.password, message, len, maxlen, 0);
    for ( i = 1; i < Cmd_Argc(); ++i )
    {
        len = Com_AddToString(" ", message, len, maxlen, 0);
        v4 = maxlen;
        v3 = len;
        v0 = Cmd_Argv(i);
        len = Com_AddToString(v0, message, v3, v4, 1);
    }
    if ( len == maxlen )
    {
        Com_Printf(0, "rcon commands are limited to %i characters\n", maxlen - 1);
        return;
    }
    message[len] = 0;
    connstate = CL_GetLocalClientConnectionState(0);
    clc = CL_GetLocalClientConnection(0);
    if ( connstate < CA_CONNECTED )
    {
        if ( rconGlob.host.type == NA_BAD )
        {
            Com_Printf(0, "Can't determine rcon target.    You can fix this by either:\n");
            Com_Printf(0, "1) Joining the server as a player.\n");
            Com_Printf(0, "2) Setting the host server with 'rcon host <address>'.\n");
            return;
        }
        to = rconGlob.host;
        to.port = BigShort(rconGlob.host.port);
    }
    else
    {
        to = clc->netchan.remoteAddress;
    }
    v2 = to;
    NetworkID = Com_LocalClient_GetNetworkID(0);
    NET_OutOfBandData(NetworkID, v2, (const unsigned __int8 *)message, &message[strlen(message) + 1] - &message[1] + 1);
}

void CL_RconLogin()
{
    unsigned int v0; // [esp+Ch] [ebp-1Ch]
    char *password; // [esp+24h] [ebp-4h]

    if ( Cmd_Argc() == 3 )
    {
        password = (char *)Cmd_Argv(2);
        v0 = (unsigned int)&password[strlen(password) + 1];
        if ( v0 - (unsigned int)(password + 1) < 0x18 )
            memcpy((unsigned __int8 *)&rconGlob, (unsigned __int8 *)password, v0 - (unsigned int)password);
        else
            Com_Printf(14, "rcon password must be %i characters or less\n", 24);
    }
    else
    {
        Com_Printf(14, "USAGE: rcon login <password>\n");
    }
}

void CL_RconLogout()
{
    if ( rconGlob.password[0] )
        rconGlob.password[0] = 0;
    else
        Com_Printf(14, "Not logged in\n");
}

void CL_RconHost()
{
    const char *hostName; // [esp+10h] [ebp-4h]

    if ( Cmd_Argc() == 3 )
    {
        hostName = Cmd_Argv(2);
        if ( NET_StringToAdr(hostName, &rconGlob.host) )
        {
            if ( rconGlob.host.type == NA_BAD
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_pc_mp.cpp",
                            1185,
                            1,
                            "%s",
                            "rconGlob.host.type != NA_BAD") )
            {
                __debugbreak();
            }
            if ( !rconGlob.host.port )
                rconGlob.host.port = BigShort(3074);
        }
        else
        {
            Com_Printf(14, "bad host address\n");
            if ( rconGlob.host.type != NA_BAD
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_pc_mp.cpp",
                            1182,
                            1,
                            "%s\n\t(rconGlob.host.type) = %i",
                            "(rconGlob.host.type == NA_BAD)",
                            rconGlob.host.type) )
            {
                __debugbreak();
            }
        }
    }
    else
    {
        Com_Printf(14, "USAGE: rcon host <address>\n");
    }
}

serverStatusInfoResponse_s *__cdecl CL_GetServerStatus(XNKID *secID)
{
    int oldest; // [esp+0h] [ebp-Ch]
    int i; // [esp+4h] [ebp-8h]
    int ia; // [esp+4h] [ebp-8h]
    int ib; // [esp+4h] [ebp-8h]
    int oldestTime; // [esp+8h] [ebp-4h]

    for ( i = 0; i < 16; ++i )
    {
        //if ( XNKID::operator==(secID, cl_serverStatusList[i].secId.ab) )
        if (*secID == cl_serverStatusList[i].secId)
            return &cl_serverStatusList[i];
    }
    for ( ia = 0; ia < 16; ++ia )
    {
        if ( cl_serverStatusList[ia].retrieved )
            return &cl_serverStatusList[ia];
    }
    oldest = -1;
    oldestTime = 0;
    for ( ib = 0; ib < 16; ++ib )
    {
        if ( oldest == -1 || cl_serverStatusList[ib].startTime < oldestTime )
        {
            oldest = ib;
            oldestTime = cl_serverStatusList[ib].startTime;
        }
    }
    if ( oldest == -1 )
        return &cl_serverStatusList[++serverStatusCount & 0xF];
    else
        return &cl_serverStatusList[oldest];
}

serverStatusInfoResponse_s *__cdecl CL_GetServerStatusScoreBoard(XNKID *secID)
{
    int oldest; // [esp+0h] [ebp-Ch]
    int i; // [esp+4h] [ebp-8h]
    int ia; // [esp+4h] [ebp-8h]
    int ib; // [esp+4h] [ebp-8h]
    int oldestTime; // [esp+8h] [ebp-4h]

    for ( i = 0; i < 16; ++i )
    {
        //if ( XNKID::operator==(secID, cl_serverStatusScoreBoardList[i].secId.ab) )
        if ( *secID == cl_serverStatusScoreBoardList[i].secId )
            return &cl_serverStatusScoreBoardList[i];
    }
    for ( ia = 0; ia < 16; ++ia )
    {
        if ( cl_serverStatusScoreBoardList[ia].retrieved )
            return &cl_serverStatusScoreBoardList[ia];
    }
    oldest = -1;
    oldestTime = 0;
    for ( ib = 0; ib < 16; ++ib )
    {
        if ( oldest == -1 || cl_serverStatusScoreBoardList[ib].startTime < oldestTime )
        {
            oldest = ib;
            oldestTime = cl_serverStatusScoreBoardList[ib].startTime;
        }
    }
    if ( oldest == -1 )
        return &cl_serverStatusScoreBoardList[++serverStatusCount & 0xF];
    else
        return &cl_serverStatusScoreBoardList[oldest];
}

int __cdecl CL_ServerStatus(char *serversecurityID, char *serverStatusString, int maxLen)
{
#if 0 // KISAKTODO
    XNKID *p_secId; // edx
    XNKID secId; // [esp+0h] [ebp-18h] BYREF
    serverStatusInfoResponse_s *serverStatus; // [esp+Ch] [ebp-Ch]
    int i; // [esp+10h] [ebp-8h]
    serverInfo_t *server; // [esp+14h] [ebp-4h]

    if ( serversecurityID )
    {
        XNKID::XNKID(&secId);
        *(unsigned int *)secId.ab = *(unsigned int *)serversecurityID;
        *(unsigned int *)&secId.ab[4] = *((unsigned int *)serversecurityID + 1);
        server = FindServerBySecID(&secId);
        if ( !server )
            return 0;
        serverStatus = CL_GetServerStatus(&secId);
        if ( !serverStatusString )
        {
            serverStatus->retrieved = 1;
            return 0;
        }
        //if ( XNKID::operator==(&secId, serverStatus->secId.ab) )
        if ( secId == serverStatus->secId )
        {
            if ( !serverStatus->pending )
            {
                I_strncpyz(serverStatusString, serverStatus->string, maxLen);
                serverStatus->retrieved = 1;
                serverStatus->startTime = 0;
                return 1;
            }
            if ( serverStatus->startTime < (signed int)(Sys_Milliseconds() - cl_serverStatusResendTime->current.integer) )
            {
                serverStatus->print = 0;
                serverStatus->pending = 1;
                serverStatus->retrieved = 0;
                serverStatus->time = 0;
                serverStatus->startTime = Sys_Milliseconds();
                CL_RawPingServer(server, 4u);
                return 0;
            }
        }
        else if ( serverStatus->retrieved )
        {
            serverStatus->print = 0;
            serverStatus->pending = 1;
            serverStatus->retrieved = 0;
            serverStatus->startTime = Sys_Milliseconds();
            serverStatus->time = 0;
            serverStatus->secId = secId;
            CL_RawPingServer(server, 4u);
            return 0;
        }
        return 0;
    }
    else
    {
        for ( i = 0; i < 16; ++i )
        {
            cl_serverStatusList[i].address.port = 0;
            cl_serverStatusList[i].retrieved = 1;
            p_secId = &cl_serverStatusList[i].secId;
            *(unsigned int *)p_secId->ab = 0;
            *(unsigned int *)&p_secId->ab[4] = 0;
        }
        return 0;
    }
#else
    return 0;
#endif
}

int __cdecl CL_ServerStatusScoreBoard(char *serversecurityID, char *serverStatusString, int maxLen)
{
#if 0 // KISAKTODO
    XNKID *p_secId; // edx
    XNKID secId; // [esp+0h] [ebp-18h] BYREF
    serverStatusInfoResponse_s *serverStatus; // [esp+Ch] [ebp-Ch]
    int i; // [esp+10h] [ebp-8h]
    serverInfo_t *server; // [esp+14h] [ebp-4h]

    if ( serversecurityID )
    {
        XNKID::XNKID(&secId);
        *(unsigned int *)secId.ab = *(unsigned int *)serversecurityID;
        *(unsigned int *)&secId.ab[4] = *((unsigned int *)serversecurityID + 1);
        server = FindServerBySecID(&secId);
        if ( !server )
            return 0;
        serverStatus = CL_GetServerStatusScoreBoard(&secId);
        if ( !serverStatusString )
        {
            serverStatus->retrieved = 1;
            return 0;
        }
        //if ( XNKID::operator==(&secId, serverStatus->secId.ab) )
        if ( secId == serverStatus->secId )
        {
            if ( !serverStatus->pending )
            {
                I_strncpyz(serverStatusString, serverStatus->string, maxLen);
                serverStatus->retrieved = 1;
                serverStatus->startTime = 0;
                return 1;
            }
            if ( serverStatus->startTime < (signed int)(Sys_Milliseconds() - cl_serverStatusResendTime->current.integer) )
            {
                serverStatus->print = 0;
                serverStatus->pending = 1;
                serverStatus->retrieved = 0;
                serverStatus->time = 0;
                serverStatus->startTime = Sys_Milliseconds();
                CL_RawPingServer(server, 6u);
                return 0;
            }
        }
        else if ( serverStatus->retrieved )
        {
            serverStatus->print = 0;
            serverStatus->pending = 1;
            serverStatus->retrieved = 0;
            serverStatus->startTime = Sys_Milliseconds();
            serverStatus->time = 0;
            serverStatus->secId = secId;
            CL_RawPingServer(server, 6u);
            return 0;
        }
        return 0;
    }
    else
    {
        for ( i = 0; i < 16; ++i )
        {
            cl_serverStatusScoreBoardList[i].address.port = 0;
            cl_serverStatusScoreBoardList[i].retrieved = 1;
            p_secId = &cl_serverStatusScoreBoardList[i].secId;
            *(unsigned int *)p_secId->ab = 0;
            *(unsigned int *)&p_secId->ab[4] = 0;
        }
        return 0;
    }
#else
    return 0;
#endif
}

void __cdecl CL_ServerStatusScoreBoardResponse(msg_t *msg, XNKID *secID)
{
    unsigned int v2; // [esp+0h] [ebp-4044h]
    unsigned int v3; // [esp+10h] [ebp-4034h]
    unsigned int v4; // [esp+20h] [ebp-4024h]
    char string[16384]; // [esp+34h] [ebp-4010h] BYREF
    char *StringLine; // [esp+4038h] [ebp-Ch]
    serverStatusInfoResponse_s *v7; // [esp+403Ch] [ebp-8h]
    int i; // [esp+4040h] [ebp-4h]

    v7 = 0;
    for ( i = 0; i < 16; ++i )
    {
        //if ( XNKID::operator==(secID, cl_serverStatusScoreBoardList[i].secId.ab) )
        if ( *secID == cl_serverStatusScoreBoardList[i].secId )
        {
            v7 = &cl_serverStatusScoreBoardList[i];
            break;
        }
    }
    if ( v7 )
    {
        StringLine = MSG_ReadStringLine(msg, string, 0x4000u);
        Com_sprintf(v7->string, 0x4000u, "%s", StringLine);
        v4 = strlen(v7->string);
        Com_sprintf(&v7->string[v4], 0x4000 - v4, "\\");
        i = 0;
        StringLine = MSG_ReadStringLine(msg, string, 0x4000u);
        while ( *StringLine )
        {
            v3 = strlen(v7->string);
            Com_sprintf(&v7->string[v3], 0x4000 - v3, "\\%s", StringLine);
            StringLine = MSG_ReadStringLine(msg, string, 0x4000u);
            ++i;
        }
        v2 = strlen(v7->string);
        Com_sprintf(&v7->string[v2], 0x4000 - v2, "\\");
        v7->time = Sys_Milliseconds();
        v7->pending = 0;
        if ( v7->print )
            v7->retrieved = 1;
    }
}

void __cdecl CL_ServerStatusResponse(msg_t *msg, XNKID *secID)
{
    char info[1024]; // [esp+20h] [ebp-818h] BYREF
    int l; // [esp+420h] [ebp-418h]
    int len; // [esp+424h] [ebp-414h]
    char strBuf[1024]; // [esp+428h] [ebp-410h] BYREF
    char *s; // [esp+82Ch] [ebp-Ch]
    serverStatusInfoResponse_s *serverStatus; // [esp+830h] [ebp-8h]
    int i; // [esp+834h] [ebp-4h]

    serverStatus = 0;
    for ( i = 0; i < 16; ++i )
    {
        //if ( XNKID::operator==(secID, cl_serverStatusList[i].secId.ab) )
        if ( *secID == cl_serverStatusList[i].secId )
        {
            serverStatus = &cl_serverStatusList[i];
            break;
        }
    }
    if ( serverStatus )
    {
        s = MSG_ReadStringLine(msg, strBuf, 0x400u);
        len = 0;
        Com_sprintf(serverStatus->string, 0x4000u, "%s", s);
        if ( serverStatus->print )
        {
            Com_Printf(14, "Server settings:\n");
            while ( *s )
            {
                for ( i = 0; i < 2 && *s; ++i )
                {
                    if ( *s == 92 )
                        ++s;
                    l = 0;
                    do
                    {
                        if ( !*s )
                            break;
                        info[l++] = *s;
                        if ( l >= 1023 )
                            break;
                        ++s;
                    }
                    while ( *s != 92 );
                    info[l] = 0;
                    if ( i )
                        Com_Printf(14, "%s\n", info);
                    else
                        Com_Printf(14, "%-24s", info);
                }
            }
        }
        len = strlen(serverStatus->string);
        Com_sprintf(&serverStatus->string[len], 0x4000 - len, "\\");
        len = strlen(serverStatus->string);
        Com_sprintf(&serverStatus->string[len], 0x4000 - len, "\\");
        serverStatus->time = Sys_Milliseconds();
        serverStatus->pending = 0;
        if ( serverStatus->print )
            serverStatus->retrieved = 1;
    }
}

void __cdecl CL_ResetPlayerMuting(unsigned int muteClientIndex)
{
    if ( muteClientIndex >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_pc_mp.cpp",
                    1944,
                    0,
                    "%s",
                    "muteClientIndex >= 0 && muteClientIndex < MAX_CLIENTS") )
    {
        __debugbreak();
    }
    s_playerMute[muteClientIndex] = 0;
}

void __cdecl CL_MutePlayer(int localClientNum, unsigned int muteClientIndex)
{
    const char *v2; // eax

    if ( muteClientIndex >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_pc_mp.cpp",
                    1953,
                    0,
                    "muteClientIndex doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                    muteClientIndex,
                    32) )
    {
        __debugbreak();
    }
    s_playerMute[muteClientIndex] = !s_playerMute[muteClientIndex];
    if ( s_playerMute[muteClientIndex] )
        v2 = va("muteplayer %i\n", muteClientIndex);
    else
        v2 = va("unmuteplayer %i\n", muteClientIndex);
    Cbuf_AddText(0, v2);
}

bool __cdecl CL_IsPlayerMuted(int localClientNum, unsigned int muteClientIndex)
{
    if ( muteClientIndex >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_pc_mp.cpp",
                    1966,
                    0,
                    "%s",
                    "muteClientIndex >= 0 && muteClientIndex < MAX_CLIENTS") )
    {
        __debugbreak();
    }
    return s_playerMute[muteClientIndex];
}

void __cdecl CL_ClearMutedList()
{
    *(unsigned int *)s_playerMute = 0;
    *(unsigned int *)&s_playerMute[4] = 0;
    *(unsigned int *)&s_playerMute[8] = 0;
    *(unsigned int *)&s_playerMute[12] = 0;
    *(unsigned int *)&s_playerMute[16] = 0;
    *(unsigned int *)&s_playerMute[20] = 0;
    *(unsigned int *)&s_playerMute[24] = 0;
    *(unsigned int *)&s_playerMute[28] = 0;
}

void __cdecl CL_FinishMotdDownload()
{
    void *buf; // [esp+0h] [ebp-8h] BYREF
    int fileSize; // [esp+4h] [ebp-4h]

    fileSize = FS_ReadFile("motd.txt", &buf);
    if ( fileSize >= 0 )
    {
        Dvar_SetStringByName("motd", (char *)buf);
        FS_FreeFile(buf);
    }
}

void __cdecl CL_WWWDownload()
{
    char *error; // [esp+0h] [ebp-114h]
    dlStatus_t ret; // [esp+8h] [ebp-10Ch]
    char to_ospath[260]; // [esp+Ch] [ebp-108h] BYREF

    ret = (dlStatus_t)DL_DownloadLoop();
    if ( ret )
    {
        if ( DL_DLIsMotd() )
        {
            if ( ret == DL_DONE )
                CL_FinishMotdDownload();
            cls.wwwDlInProgress = 0;
        }
        else if ( ret == DL_DONE)
        {
            cls.download = 0;
            FS_BuildOSPath((char *)fs_homepath->current.integer, 0, cls.originalDownloadName, to_ospath);
            remove(to_ospath);
            if ( rename(cls.downloadTempName, to_ospath) )
            {
                FS_CopyFile(cls.downloadTempName, to_ospath);
                remove(cls.downloadTempName);
            }
            cls.downloadName[0] = 0;
            cls.downloadTempName[0] = 0;
            I_strncpyz(legacyHacks.cl_downloadName, "", 64);
            if ( !cls.wwwDlDisconnected )
                CL_AddReliableCommand(0, "wwwdl done");
            cls.wwwDlInProgress = 0;
            CL_NextDownload(0);
        }
        else if ( cls.wwwDlDisconnected )
        {
            error = va("Download failure while getting '%s'\n", cls.downloadName);
            cls.wwwDlDisconnected = 0;
            CL_ClearStaticDownload();
            Com_Error(ERR_DROP, error);
        }
        else
        {
            Com_Printf(14, "Download failure while getting '%s'\n", cls.downloadName);
            CL_AddReliableCommand(0, "wwwdl fail");
            cls.wwwDlInProgress = 0;
        }
    }
}

cmd_function_s CL_PC_SignInLive_VAR;
cmd_function_s CL_PC_SignIn_VAR;
cmd_function_s CL_PC_RequireLiveSignin_VAR;
cmd_function_s CL_LanSessions_f_VAR;
cmd_function_s CL_LanConnect_f_VAR;
cmd_function_s CL_CACValidateRequest_f_VAR;
cmd_function_s CL_Prestige_f_VAR;
cmd_function_s CL_QuickMatchConnect_f_VAR;
void __cdecl CL_Platform_RegisterCommands()
{
    Cmd_AddCommandInternal("xsigninlive", CL_PC_SignInLive, &CL_PC_SignInLive_VAR);
    Cmd_AddCommandInternal("xsignin", CL_PC_SignIn, &CL_PC_SignIn_VAR);
    Cmd_AddCommandInternal("xrequirelivesignin", CL_PC_RequireLiveSignin, &CL_PC_RequireLiveSignin_VAR);
    Cmd_AddCommandInternal("lansessions", CL_LanSessions_f, &CL_LanSessions_f_VAR);
    Cmd_AddCommandInternal("lanconnect", CL_LanConnect_f, &CL_LanConnect_f_VAR);
    Cmd_AddCommandInternal("cacvalidate", CL_CACValidateRequest_f, &CL_CACValidateRequest_f_VAR);
    Cmd_AddCommandInternal("prestigerequest", CL_Prestige_f, &CL_Prestige_f_VAR);
    Cmd_AddCommandInternal("quickmatchconnect", CL_QuickMatchConnect_f, &CL_QuickMatchConnect_f_VAR);
}

void __cdecl CL_PC_RequireLiveSignin()
{
    Live_RequireUserToPlayOnline();
}

void __cdecl CL_LanSessions_f()
{
}

void __cdecl CL_LanConnect_f()
{
}

void __cdecl CL_Prestige_f()
{
    CL_PrestigeRequest();
}

void __cdecl CL_RequestCACValidateSuccess()
{
    Com_Printf(14, "CACValidate: Successfully sent message to server\n");
}

void __cdecl CL_RequestCACValidateFailure()
{
    Com_DPrintf(14, "CACValidate: Failed to send message to server!\n");
}

void __cdecl CL_CACValidateHandleNACK(unsigned __int64 uid)
{
    if ( g_cacValidateState == CAC_REQUESTSENT )
    {
        if ( CL_CACValidateServerMatches(uid) )
        {
            Com_DPrintf(14, "CACValidate: Got NACK from the server\n CAC_REQUESTSENT --> CAC_NOTVALIDATED\n");
            g_cacValidateState = CAC_FAILED;
        }
    }
}

char __cdecl CL_CACValidateServerMatches(unsigned __int64 uid)
{
    bool retval; // [esp+3h] [ebp-1h]

    retval = 0;
    if ( uid == g_cacValidateServer.uid )
    {
        Com_DPrintf(15, "CACValidate: ACK received from %llu\n", uid);
        return 1;
    }
    else
    {
        Com_DPrintf(15, "CACValidate: ACK received, expected uid %llu, got uid %llu\n", g_cacValidateServer.uid, uid);
    }
    return retval;
}

void __cdecl CL_CACValidateHandleOK(unsigned __int64 uid)
{
    if ( g_cacValidateState == CAC_REQUESTSENT )
    {
        if ( CL_CACValidateServerMatches(uid) )
        {
            Com_DPrintf(15, "CACValidate: Got OK from server\n CAC_REQUESTSENT --> CAC_VALIDATED\n");
            UI_CloseMenu(0, "popup_gettingdata");
            g_cacValidateState = CAC_VALIDATED;
            LiveStorage_SetFirstTimeRunning(0);
        }
    }
}

void __cdecl CL_CACValidateHandleBad(unsigned __int64 uid)
{
    if ( g_cacValidateState == CAC_REQUESTSENT )
    {
        if ( CL_CACValidateServerMatches(uid) )
        {
            Com_DPrintf(14, "CACValidate: Got BAD from server\n CAC_REQUESTSENT --> CAC_REJECTED\n");
            g_cacValidateState = CAC_REJECTED;
        }
    }
}

char __cdecl CL_RequestCACValidate(unsigned __int64 serverId)
{
    return 0;
}

void __cdecl CL_CACValidateRequest_f()
{
    persistentStats *StatsBuffer; // eax

    if ( Dvar_GetBool("allItemsUnlocked") || Dvar_GetBool("allItemsPurchased") )
    {
        Com_PrintError(0, "Not doing cacvalidate, allitemspurchased or allitemsunlocked is active!\n");
    }
    else if ( !LiveStorage_FirstTimeRunning() )
    {
        if ( LiveStorage_DoWeHaveAllStats(0) )
        {
            StatsBuffer = LiveStorage_GetStatsBuffer(0, STATS_LOCATION_NORMAL, 1);
            if ( LiveStats_GetBasicTrainingState(StatsBuffer->statsBuffer) )
            {
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_pc_mp.cpp",
                                2317,
                                0,
                                "Attempted to cacvalidate in basic training. Show me to Ewan!\n") )
                    __debugbreak();
                Com_PrintError(0, "Attempted to cacvalidate in basic training, show me to Ewan!\n");
            }
            else if ( !LiveStorage_GetStatsWriteNeeded(0, STATS_LOCATION_NORMAL) || CL_CACValidateInProgress() )
            {
                Com_DPrintf(14, "CACValidate: Not requesting validate, validation already in progress or no stats change\n");
            }
            else
            {
                LiveStorage_SetStatsWriteNeeded(0, 0, STATS_LOCATION_NORMAL);
                g_cacValidateState = CAC_NOTVALIDATED;
            }
        }
        else
        {
            Com_PrintError(0, "Not cacvalidating, don't have all required files..\n");
        }
    }
}

bool __cdecl CL_CACValidateInProgress()
{
    return g_cacValidateState != CAC_DORMANT;
}

char __cdecl CL_PrestigeRequest()
{
    bool retval; // [esp+3h] [ebp-1h]

    retval = 0;
    if ( LiveStats_WritePrestigeToStats(0) )
    {
        Com_DPrintf(14, "Prestige incremented!\n");
        CL_CACValidateRequest_f();
        return 1;
    }
    else
    {
        Com_PrintError(14, "Couldn't increment prestige. Max prestige or insufficient xp!\n");
    }
    return retval;
}

bool __cdecl CL_CACValidate_IsTimedOut()
{
    return (int)(Sys_Milliseconds() - g_cacValidateTimeout) > 25000;
}

void __cdecl CL_CACValidate_Frame()
{
}

void __cdecl CL_QuickMatchConnect_f()
{
    CL_Connect(s_quickmatchCandidates);
}

bool __cdecl CL_QuickMatch_InProgress()
{
    return s_quickmatchstate != QM_DORMANT;
}

void __cdecl CL_QuickMatch_FindSessionsSuccess(TaskRecord *task)
{
}

void __cdecl CL_QuickMatch_FindSessionsFailure()
{
    s_quickmatchstate = QM_DORMANT;
}

void __cdecl CL_QuickMatch_Start(
                unsigned int servertype,
                const char *mapname,
                unsigned int playlist,
                int minPlayers,
                int maxPlayers,
                int maxPing)
{
}

void __cdecl CL_QuickWager_Start()
{
}

void __cdecl CL_QuickMatch_f()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    int minplayers; // [esp+30h] [ebp-18h]
    int maxplayers; // [esp+34h] [ebp-14h]
    unsigned int playlist; // [esp+3Ch] [ebp-Ch]
    const char *mapname; // [esp+40h] [ebp-8h]
    int maxping; // [esp+44h] [ebp-4h]

    if ( Cmd_Argc() == 6 )
    {
        v0 = Cmd_Argv(1);
        playlist = atoi(v0);
        mapname = Cmd_Argv(2);
        v1 = Cmd_Argv(3);
        minplayers = atoi(v1);
        v2 = Cmd_Argv(4);
        maxplayers = atoi(v2);
        v3 = Cmd_Argv(5);
        maxping = atoi(v3);
        CL_QuickMatch_Start(1u, mapname, playlist, minplayers, maxplayers, maxping);
    }
    else
    {
        CL_QuickMatch_Start(1u, "any", 0xFFFFFFFF, 0, 24, 150);
    }
}

void __cdecl CL_QuickWager_f()
{
    CL_QuickWager_Start();
}

void __cdecl CL_QuickMatch_InitDvars()
{
    cl_quickmatch_resultspercent = _Dvar_RegisterInt(
                                                                     "quickmatch_resultspercent",
                                                                     75,
                                                                     1,
                                                                     100,
                                                                     0,
                                                                     "Percentage of ping results to wait for before choosing a session");
    cl_quickmatch_pingweight = _Dvar_RegisterInt(
                                                             "quickmatch_pingweight",
                                                             100,
                                                             1,
                                                             0x7FFFFFFF,
                                                             0,
                                                             "Ping weight. Higher number makes it more likely to quickmatch based on ping");
    cl_quickmatch_fullnessweight = _Dvar_RegisterInt(
                                                                     "quickmatch_fullnessweight",
                                                                     1,
                                                                     1,
                                                                     0x7FFFFFFF,
                                                                     0,
                                                                     "Fullness weight. Higher number makes it more likely to quickmatch based on %fullness");
    cl_wager_firstplaylist = _Dvar_RegisterInt("wager_firstplaylist", 21, 0, 64, 0x40u, "First wager playlist");
    cl_wager_lastplaylist = _Dvar_RegisterInt(
                                                        "wager_lastplaylist",
                                                        29,
                                                        cl_wager_firstplaylist->current.integer,
                                                        64,
                                                        0x40u,
                                                        "Last wager playlist");
    cl_wager_maxping = _Dvar_RegisterInt("wager_maxping", 150, 1, 999, 0x40u, "Max ping for wager quickmatch");
}

void __cdecl CL_QuickMatch_Init()
{
}

bool __cdecl CL_QuickMatch_ServerMatches(MatchMakingInfo *server)
{
    return false;
}

void __cdecl CL_QuickMatch_PingServers()
{
}

bool __cdecl CL_QuickMatch_GoodSessionFound()
{
    return false;
}

void __cdecl CL_QuickMatch_PingResponse(XNKID *secID, msg_t *msg)
{
}

char __cdecl CL_QuickMatch_ChooseSession()
{
    return 0;
}

int __cdecl CL_QuickMatch_CompareServers(unsigned int *sv1, unsigned int *sv2)
{
    return sv2[72] - sv1[72];
}

void __cdecl CL_QuickMatch_Frame()
{
    char *v0; // eax
    char *v1; // [esp-8h] [ebp-10h]

    switch ( s_quickmatchstate )
    {
        case QM_DORMANT:
        case QM_FINDING:
            return;
        case QM_PINGING:
            if ( CL_QuickMatch_ShouldChooseSession() )
                s_quickmatchstate = QM_CHOOSE;
            else
                CL_QuickMatch_PingServers();
            break;
        case QM_CHOOSE:
            if ( CL_QuickMatch_ChooseSession() )
                s_quickmatchstate = QM_DORMANT;
            else
                s_quickmatchstate = QM_FAILED;
            break;
        case QM_FAILED:
            v1 = UI_SafeTranslateString("PLATFORM_QUICKMATCH_FAILED");
            v0 = UI_SafeTranslateString("");
            UI_OpenToastPopup(0, "menu_mp_killstreak_select", v0, v1, 2700);
            s_quickmatchstate = QM_DORMANT;
            break;
        default:
            if ( !"Unknown quickmatchstate! %i\n"
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_pc_mp.cpp",
                            3007,
                            0,
                            "%s\n\t%s",
                            "\"Unknown quickmatchstate! %i\\n\"",
                            (const char *)s_quickmatchstate) )
            {
                __debugbreak();
            }
            break;
    }
}

bool __cdecl CL_QuickMatch_ShouldChooseSession()
{
    return false;
}

struct cityname_t // sizeof=0x44
{                                       // XREF: .data:s_cityNames/r
    char shortname[4];                  // XREF: CL_LongNameForShortName(char const *)+25/r
    char longname[64];                  // XREF: CL_LongNameForShortName(char const *)+56/o
};
cityname_t s_cityNames[1024];
char *__cdecl CL_LongNameForShortName(const char *shortname)
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < 1024 && s_cityNames[i].shortname[0]; ++i )
    {
        if ( !I_stricmp(shortname, s_cityNames[i].shortname) )
            return s_cityNames[i].longname;
    }
    return 0;
}

void __cdecl CL_PC_SignIn()
{
    //BLOPS_NULLSUB();
}

