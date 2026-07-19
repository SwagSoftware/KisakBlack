#include "cl_main_pc_mp.h"

#include <live/live_storage_win.h>
#include <live/live_win.h>
#include <live/live_sessions_win.h>
#include <live/live_stats.h>
#include <live/live_steam.h>

#include <qcommon/net_chan_mp.h>
#include <DW/dwMatchMaking.h>
#include <client/cl_main.h>
#include <cgame_mp/cg_newDraw_mp.h>
#include <cgame_mp/cg_main_mp.h>
#include <client/client.h>
#include <sound/snd_public_async.h>
#include <server_mp/sv_main_mp.h>
#include <universal/com_constantconfigstrings.h>
#include <qcommon/com_clients.h>
#include <client/cl_console.h>
#include <DW/dwNet.h>
#include <client/splitscreen.h>
#include "cl_cgame_mp.h"
#include <ui/ui_main_pc.h>
#include <qcommon/com_gamemodes.h>
#include <universal/com_files.h>
#include <win32/win_net.h>
#include <DW/dwLogOn_pc.h>
#include "cl_scrn_mp.h"
#include <win32/win_shared.h>
#include <DW/dwUtils_pc.h>
#include <DW/MatchMakingInfo_win32.h>

#include <DW/dwQoS.h>
#include <DW/dwUtils.h>
#include <qcommon/dl_main.h>
#include <qcommon/legacyhacks.h>
#include <qcommon/md4.h>
#include <DW/MatchMakingQueries_win32.h>
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
            memset((unsigned __int8 *)server->city, 0, sizeof(server->city));
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

void __cdecl CL_ServerInfoPacket(bdSecurityID *secID, msg_t *msg, int time)
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

#ifdef KISAK_DW
    bdSessionID sessionID; // [esp+14h] [ebp-10h] BYREF
    //bdSessionID::bdSessionID(&sessionID);
    sessionID.m_sessionID = server->xnkid;
    dwSetSessionID(&sessionID);
#endif

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
#ifdef KISAK_DW
    dwRegisterSecIDAndKey(&server->xnkid, &server->xnkey);
    dwCommonAddrToNetadr(&server->adr, (bool *)server, (bdCommonAddr *)&server->xnkid);
#endif
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
        cls.serveruid = server->bdUserID;
        UI_ClearErrors();
        v3 = UI_SafeTranslateString("MENU_CONNECTING_CAPS");
        Dvar_SetStringByName("statusinfo_popmenuTitle", v3);
        Dvar_SetStringByName("statusinfo_popmenuMessage", cls.servername);
        Dvar_SetStringByName("statusinfo_onEscArg", (char*)"disconnect;");
        UI_OpenMenu(0, "code_statusinfo_popmenu");
        //bdTaskResult::~bdTaskResult(&sessionID);
    }
    else
    {
        Com_Error(ERR_DROP, "EXE_ERR_INVALID_CD_KEY");
        //bdTaskResult::~bdTaskResult(&sessionID);
    }
}

void __cdecl CL_ConnectHackDW()
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
    // Steam auth-ticket integration is deferred on the Linux port (Steamworks SDK
    // not yet wired in); validate as a no-op for now.
    (void)addr;
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
#ifdef KISAK_DW
                    || dwGetLogOnStatus(0) == 4 
#endif
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
                const bdSecurityID *secID)
{
    if ( buffersize >= 9 )
    {
        *buffer = opcode;
        *(bdSecurityID *)(buffer + 1) = *secID;
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
#if 0 // KISAKTODO
    netadr_t privserveraddr; // [esp+0h] [ebp-30h] BYREF
    unsigned __int8 sendBuf[12]; // [esp+10h] [ebp-20h] BYREF
    netadr_t pubserveraddr; // [esp+1Ch] [ebp-14h] BYREF

    memset(sendBuf, 0, 9);
    CL_RawPingSetupBuffer(sendBuf, 9, opcode, &server->xnkid);
    server->pingedTime = Sys_Milliseconds();
    dwXnaddrtonetadr(server->xnaddr.addrBuff, &privserveraddr, &pubserveraddr);
    dwRawSendTo(&pubserveraddr, sendBuf, 9u);
    if ( !NET_CompareAdr(privserveraddr, pubserveraddr) )
        dwRawSendTo(&privserveraddr, sendBuf, 9u);
#endif
}

void __cdecl CL_ServersResponsePacket(MatchMakingInfo *mminfo, int numResults, bool geo)
{
#if 0 // KISAKTODO: hate
    bdCommonAddr *v3; // ecx
    const char *CountryCode; // eax
    int v5; // eax
    bdCommonAddr *v6; // eax
    bdReference<bdCommonAddr> *v7; // ecx
    unsigned int *v8; // edx
    bdReference<bdCommonAddr> *v9; // eax
    bdReference<bdCommonAddr> v10; // [esp-4h] [ebp-216Ch] BYREF
    int v11; // [esp+4h] [ebp-2164h]
    bdReference<bdCommonAddr> *j; // [esp+8h] [ebp-2160h]
    int v13; // [esp+34h] [ebp-2134h]
    bdQoSRemoteAddr *i; // [esp+38h] [ebp-2130h]
    bdReference<bdCommonAddr> *p_m_addr; // [esp+40h] [ebp-2128h]
    netadr_t pubadr; // [esp+44h] [ebp-2124h] BYREF
    netadr_t privadr; // [esp+54h] [ebp-2114h] BYREF
    unsigned int *v18; // [esp+64h] [ebp-2104h]
    bdReference<bdCommonAddr> other; // [esp+68h] [ebp-2100h] BYREF
    int v20; // [esp+6Ch] [ebp-20FCh]
    bdQoSRemoteAddr serverAddrs[300]; // [esp+70h] [ebp-20F8h] BYREF
    int v22; // [esp+2140h] [ebp-28h] BYREF
    int v23; // [esp+2144h] [ebp-24h]
    serverInfo_t *servers; // [esp+2148h] [ebp-20h] BYREF
    unsigned int numProbes; // [esp+214Ch] [ebp-1Ch]
    int *count[5]; // [esp+2150h] [ebp-18h] BYREF

    v13 = 300;
    for ( i = serverAddrs; --v13 >= 0; ++i )
    {
        i->m_addr.m_ptr = 0;
        bdSecurityID::bdSecurityID(&i->m_id);
        bdSecurityKey::bdSecurityKey(&i->m_key);
    }

    static dwQoSMultiProbeListener s_qoslistener;
    //if ( (_S1_3 & 1) == 0 )
    //{
    //    _S1_3 |= 1u;
    //    dwQoSMultiProbeListener::dwQoSMultiProbeListener(&s_qoslistener);
    //    atexit(CL_ServersResponsePacket_::_2_::_dynamic_atexit_destructor_for__s_qoslistener__);
    //}

    Com_DPrintf(14, "CL_ServersResponsePacket\n");
    v23 = 0;
    v20 = 0;
    numProbes = 0;
    if ( CL_GetServerList(cls.pingUpdateSource, &servers, count) && servers )
    {
        servers += *count[0];
        v23 = 0;
        v20 = *count[0];
        while ( v23 < numResults && v20 < 20000 )
        {
            v18 = &mminfo[v23].__vftable;
            if ( LiveStats_GetStatsDDL()->version == v18[132] )
            {
                v3 = (bdCommonAddr *)geo;
                if ( geo
                    || (CountryCode = LiveSteam_GetCountryCode(), v5 = dwRegionCodeFromCountryCode(CountryCode), v18[114] != v5) )
                {
                    v10.m_ptr = v3;
                    dwGetLocalCommonAddr(&v10);
                    bdMatchMakingInfo::getHostAddrAsCommonAddr(v18, (bdReference<bdRemoteTask> *)&other, v10);
                    bdCommonAddr::serialize(other.m_ptr, servers->xnaddr.addrBuff);
                    memset(&count[2], 0, 12);
                    count[1] = (int *)4;
                    CL_InitServerInfo(servers, (netadr_t)4u);
                    servers->maxClients = *((_BYTE *)v18 + 276);
                    I_strncpyz(servers->hostName, (const char *)v18 + 305, 32);
                    I_strncpyz(servers->mapName, (const char *)v18 + 384, 32);
                    servers->xnkey = (bdSecurityKey)*((_OWORD *)v18 + 18);
                    servers->xnkid = *(bdSecurityID *)(v18 + 1);
                    Com_IntToGametype(servers->gameType, v18[105]);
                    servers->region = v18[114];
                    servers->licensetype = v18[133];
                    if ( servers->licensetype == 5 )
                    {
                        servers->clients = *((_BYTE *)v18 + 280);
                        dwXnaddrtonetadr(servers->xnaddr.addrBuff, &privadr, &pubadr);
                        if ( !NET_CompareAdr(privadr, pubadr) )
                        {
                            p_m_addr = &serverAddrs[numProbes++].m_addr;
                            bdReference<bdCommonAddr>::operator=(p_m_addr, &other);
                            v6 = (bdCommonAddr *)v18[2];
                            v7 = p_m_addr;
                            p_m_addr[1].m_ptr = (bdCommonAddr *)v18[1];
                            v7[2].m_ptr = v6;
                            v8 = v18 + 72;
                            v9 = p_m_addr + 3;
                            p_m_addr[3].m_ptr = (bdCommonAddr *)v18[72];
                            v9[1].m_ptr = (bdCommonAddr *)v8[1];
                            v9[2].m_ptr = (bdCommonAddr *)v8[2];
                            v9[3].m_ptr = (bdCommonAddr *)v8[3];
                        }
                    }
                    else
                    {
                        servers->clients = v18[70] - 1;
                    }
                    servers->bdUserID = mminfo[v23].m_memberdemonwareID;
                    ++servers;
                    ++v20;
                    bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&other);
                }
            }
            ++v23;
        }
        *count[0] = v20;
        Com_Printf(14, "%d servers parsed (total %d)\n", v23, *count[0]);
        if ( numProbes )
        {
            dwClearQoSProbes();
            dwStartQoSProbes(&s_qoslistener, numProbes, serverAddrs);
        }
        cls.waitdwfindsessionsresponse = 0;
    }
    v11 = 300;
    for ( j = (bdReference<bdCommonAddr> *)&v22; --v11 >= 0; bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(j) )
        j -= 7;
#endif
}

void __cdecl CL_FindServers_f()
{
#if 0 // KISAKTODO
    const char *v0; // eax
    unsigned int servertype; // [esp+14h] [ebp-14h]
    int i; // [esp+18h] [ebp-10h]
    serverInfo_t *server; // [esp+1Ch] [ebp-Ch]
    int *count; // [esp+20h] [ebp-8h] BYREF
    serverInfo_t *servers; // [esp+24h] [ebp-4h] BYREF

    servers = 0;
    if ( !dwIsPagedFindInProgress() && !Live_FindFavouritesInProgress() )
    {
        if ( Cmd_Argc() >= 2 )
        {
            v0 = Cmd_Argv(1);
            servertype = atoi(v0);
            if ( servertype < 6 )
            {
                if ( CL_GetServerList(servertype, &servers, &count) )
                {
                    for ( i = 0; i < *count; ++i )
                    {
                        server = &servers[i];
                        if ( !++server->requestCount )
                            --server->requestCount;
                    }
                    Com_Printf(0, "Contacting Demonware..\n");
                    switch ( servertype )
                    {
                        case 0u:
                        case 1u:
                        case 2u:
                            Live_FindSessionsStart(1, servertype);
                            break;
                        case 3u:
                            Live_FindFavouriteServers();
                            break;
                        case 4u:
                            Live_FindFriendServers();
                            break;
                        case 5u:
                            Live_FindRecentServers();
                            break;
                        default:
                            break;
                    }
                    cls.waitdwfindsessionsresponse = 1;
                    cls.pingUpdateSource = servertype;
                    *count = 0;
                    if ( cls.pingUpdateSource == 1 )
                        cls.lastFindSessionsTime = Sys_Milliseconds();
                }
            }
            else
            {
                Com_Printf(
                    0,
                    "argument should be 0 for unranked, 1 for ranked, 2 for wager, 3 for favourite, 4 for friends, 5 for recent\n");
            }
        }
        else
        {
            Com_Printf(0, "usage: findservers [unranked|ranked|wager|favourites|friends|recent]\n");
        }
    }
#endif
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

serverStatusInfoResponse_s *__cdecl CL_GetServerStatus(bdSecurityID *secID)
{
    int oldest; // [esp+0h] [ebp-Ch]
    int i; // [esp+4h] [ebp-8h]
    int ia; // [esp+4h] [ebp-8h]
    int ib; // [esp+4h] [ebp-8h]
    int oldestTime; // [esp+8h] [ebp-4h]

    for ( i = 0; i < 16; ++i )
    {
        //if ( bdSecurityID::operator==(secID, cl_serverStatusList[i].secId.ab) )
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

serverStatusInfoResponse_s *__cdecl CL_GetServerStatusScoreBoard(bdSecurityID *secID)
{
    int oldest; // [esp+0h] [ebp-Ch]
    int i; // [esp+4h] [ebp-8h]
    int ia; // [esp+4h] [ebp-8h]
    int ib; // [esp+4h] [ebp-8h]
    int oldestTime; // [esp+8h] [ebp-4h]

    for ( i = 0; i < 16; ++i )
    {
        //if ( bdSecurityID::operator==(secID, cl_serverStatusScoreBoardList[i].secId.ab) )
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
    bdSecurityID *p_secId; // edx
    bdSecurityID secId; // [esp+0h] [ebp-18h] BYREF
    serverStatusInfoResponse_s *serverStatus; // [esp+Ch] [ebp-Ch]
    int i; // [esp+10h] [ebp-8h]
    serverInfo_t *server; // [esp+14h] [ebp-4h]

    if ( serversecurityID )
    {
        bdSecurityID::bdSecurityID(&secId);
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
        //if ( bdSecurityID::operator==(&secId, serverStatus->secId.ab) )
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
    bdSecurityID *p_secId; // edx
    bdSecurityID secId; // [esp+0h] [ebp-18h] BYREF
    serverStatusInfoResponse_s *serverStatus; // [esp+Ch] [ebp-Ch]
    int i; // [esp+10h] [ebp-8h]
    serverInfo_t *server; // [esp+14h] [ebp-4h]

    if ( serversecurityID )
    {
        bdSecurityID::bdSecurityID(&secId);
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
        //if ( bdSecurityID::operator==(&secId, serverStatus->secId.ab) )
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

void __cdecl CL_ServerStatusScoreBoardResponse(msg_t *msg, bdSecurityID *secID)
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
        //if ( bdSecurityID::operator==(secID, cl_serverStatusScoreBoardList[i].secId.ab) )
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

void __cdecl CL_ServerStatusResponse(msg_t *msg, bdSecurityID *secID)
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
        //if ( bdSecurityID::operator==(secID, cl_serverStatusList[i].secId.ab) )
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
#if KISAK_DW
    dwStartLanDiscovery();
#endif
}

void __cdecl CL_LanConnect_f()
{
#if KISAK_DW
    const char *v0; // eax
    unsigned int serverInfoIdx; // [esp+3Ch] [ebp-198h]
    bdReference<bdCommonAddr> hostAddr; // [esp+40h] [ebp-194h] BYREF
    netadr_t nadr; // [esp+44h] [ebp-190h] BYREF
    serverInfo_t server; // [esp+54h] [ebp-180h] BYREF

    bdSecurityKey::bdSecurityKey(&server.xnkey);
    bdSecurityID::bdSecurityID(&server.xnkid);
    hostAddr.m_ptr = 0;
    if ( Cmd_Argc() == 1 )
    {
        serverInfoIdx = 0;
    }
    else
    {
        if ( Cmd_Argc() != 2 )
        {
            Com_Printf(0, "usage: lanconnect [idx]\n");
            //bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&hostAddr);
            return;
        }
        v0 = Cmd_Argv(1);
        serverInfoIdx = strtol(v0, 0, 10);
    }
    memset(nadr.ip, 0, 12);
    nadr.type = NA_IP;
    CL_InitServerInfo(&server, nadr);
    if ( dwGetLanSession(serverInfoIdx, &hostAddr, &server.xnkid, &server.xnkey) )
    {
        server.adr.port = 3074;
        if ( !I_strncmp(server.hostName, "localhost", 32) )
            SV_KillLocalServer();
        FS_DisablePureCheck(0);
        //bdCommonAddr::serialize(hostAddr.m_ptr, server.xnaddr.addrBuff);
        hostAddr.m_ptr->serialize(server.xnaddr.addrBuff);
        CL_Connect(&server);
        //bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&hostAddr);
    }
    else
    {
        Com_Printf(0, "lanconnect [idx]: ot of range server info index. use lansessions to obtain all lan sessions.\n");
        //bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&hostAddr);
    }
#endif
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
#ifdef KISAK_CAC_STUBS
    TaskRecord *task; // [esp+4h] [ebp-9D24h]
    TaskRecord *nestTask; // [esp+8h] [ebp-9D20h]
    persistentStats *buffer; // [esp+Ch] [ebp-9D1Ch]
    unsigned int v5; // [esp+18h] [ebp-9D10h]
    char v6; // [esp+27h] [ebp-9D01h]
    char payload[2]; // [esp+28h] [ebp-9D00h] BYREF
    unsigned int v8; // [esp+2Ah] [ebp-9CFEh]
    int v9; // [esp+2Eh] [ebp-9CFAh]
    unsigned __int8 to[40178]; // [esp+32h] [ebp-9CF6h] BYREF

    Com_DPrintf(14, "CACValidate: Compressing stats\n");
    v6 = 0;
    buffer = LiveStorage_GetStatsBuffer(0, STATS_LOCATION_FORCE_NORMAL, 1);
    if ( !LiveStorage_DoWeHaveAllStats(0) )
        return 0;
    if ( stat_version->current.integer == LiveStats_ReadVersionFromBuffer((char *)buffer) )
    {
        if ( LiveStats_GetBasicTrainingState(buffer->statsBuffer) )
        {
            Com_PrintError(0, "Basic training stats in force-normal. bad.\n");
            return 0;
        }
        else
        {
            v5 = MSG_CompressWithZLib(buffer->statsBuffer, 0x9CE8u, to, 0x9CE8u);
            payload[0] = 3;
            payload[1] = 0;
            v8 = v5;
            v9 = Com_BlockChecksumKey32(to, v5, 0);
            Com_Printf(14, "CACValidate: Stats compressed to %i bytes, using checksum %u\n", v5, v9);
            if ( v5 >= 0x4000 )
            {
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_pc_mp.cpp",
                                2279,
                                0,
                                "compressed ddl blob is >= DW_MAX_MESSAGE_SIZE, show this to Ewan!\n") )
                    __debugbreak();
                Com_PrintError(0, "CACValidate: Blob is too big to send\n");
            }
            else
            {
                nestTask = dwMessaging_SendInstantMessage(serverId, payload, v5 + 10);
                if ( nestTask )
                {
                    task = TaskManager2_CreateTask(task_RequestCACValidate, 0, nestTask, 0);
                    if ( task )
                    {
                        TaskManager2_StartTask(task);
                        v6 = 1;
                        g_cacValidateServer.uid = serverId;
                    }
                }
            }
            return v6;
        }
    }
    else
    {
        Com_PrintError(14, "Not doing cacvalidate, no version in buffer!\n");
        return 0;
    }
#else
    return 0;
#endif
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
#ifdef KISAK_CAC_STUBS
    unsigned intv0; // eax
    bdTrulyRandomImpl *Instance; // eax
    unsigned int v2; // eax
    __int64 v3; // [esp-8h] [ebp-74h]
    unsigned __int64 v4; // [esp-8h] [ebp-74h]
    unsigned int servernum; // [esp+68h] [ebp-4h]

    static int lastvalidate = Sys_Milliseconds();

    //if ( (_S2_3 & 1) == 0 )
    //{
    //    _S2_3 |= 1u;
    //    lastvalidate = Sys_Milliseconds();
    //}

    switch ( g_cacValidateState )
    {
        case CAC_DORMANT:
        case CAC_DONE:
            return;
        case CAC_NOTVALIDATED:
            if ( (int)(Sys_Milliseconds() - lastvalidate) >= 2000 )
            {
                lastvalidate = Sys_Milliseconds();
                nackcount = 0;
                if ( lastvalidate - cls.lastFindSessionsTime >= 300000 || cls.numrankedservers < 50 )
                {
                    if ( dwFindSessionsPaged(1044, 1, useRand, !useRand) )
                    {
                        cls.pingUpdateSource = 1;
                        cls.lastFindSessionsTime = Sys_Milliseconds();
                        UI_OpenMenu(0, "popup_gettingdata");
                        Com_DPrintf(14, "CAC_NOTVALIDATED --> CAC_FINDING\n");
                        g_cacValidateState = CAC_FINDING;
                        g_cacValidateTimeout = Sys_Milliseconds();
                    }
                    else
                    {
                        g_cacValidateState = CAC_FAILED;
                    }
                }
                else
                {
                    UI_OpenMenu(0, "popup_gettingdata");
                    Com_Printf(14, "CAC_NOTVALIDATED --> CAC_FOUND\n");
                    g_cacValidateState = CAC_FOUND;
                }
            }
            break;
        case CAC_FINDING:
            if ( dwIsPagedFindTaskAlreadyStarted() )
            {
                if ( CL_CACValidate_IsTimedOut() )
                {
                    UI_CloseMenu(0, "popup_gettingdata");
                    v0 = Sys_Milliseconds();
                    Com_Printf(
                        14,
                        "Waited %ims for cac findserver response, retrying\n CAC_FINDING --> CAC_NOTVALIDATED\n",
                        v0 - g_cacValidateTimeout);
                    g_cacValidateState = CAC_NOTVALIDATED;
                    Com_PrintError(0, "CACValidate TIMEDOUT\n");
                }
            }
            else
            {
                g_cacValidateState = CAC_FOUND;
            }
            break;
        case CAC_FAILED:
            Com_DPrintf(14, "Couldn't find a server\n CAC_FAILED --> CAC_DORMANT\n");
            Com_PrintError(0, "CACValidate FAILED\n");
            if ( cls.numrankedservers <= 0 )
            {
                UI_CloseMenu(0, "popup_gettingdata");
                g_cacValidateState = CAC_DORMANT;
            }
            else if ( ++nackcount <= 5 )
            {
                g_cacValidateState = CAC_FOUND;
            }
            else
            {
                UI_CloseMenu(0, "popup_gettingdata");
                Com_PrintWarning(0, "MAX_NACKS hit, going back to NOTVALIDATED\n");
                g_cacValidateState = CAC_NOTVALIDATED;
            }
            break;
        case CAC_FOUND:
            if ( cls.numrankedservers )
            {
                useRand = 1;
                Instance = bdSingleton<bdTrulyRandomImpl>::getInstance();
                servernum = bdTrulyRandomImpl::getRandomUInt(Instance) % cls.numrankedservers;
                HIDWORD(v3) = HIDWORD(cls.rankedServers[servernum].bdUserID);
                LODWORD(v3) = cls.rankedServers[servernum].bdUserID;
                Com_DPrintf(
                    14,
                    "CACValidate: Using server number %u of %u, bdonlineuserid %llu\n",
                    servernum,
                    cls.numrankedservers,
                    v3);
                if ( (int)(Sys_Milliseconds() - cls.rankedServers[servernum].lastRequestTime) > 2000
                    && (HIDWORD(v4) = HIDWORD(cls.rankedServers[servernum].bdUserID),
                            LODWORD(v4) = cls.rankedServers[servernum].bdUserID,
                            CL_RequestCACValidate(v4)) )
                {
                    cls.rankedServers[servernum].lastRequestTime = Sys_Milliseconds();
                    Com_Printf(14, "CAC_FOUND --> CAC_REQUESTSENT\n");
                    g_cacValidateState = CAC_REQUESTSENT;
                    g_cacValidateTimeout = Sys_Milliseconds();
                }
                else
                {
                    g_cacValidateState = CAC_FAILED;
                }
            }
            else if ( useRand )
            {
                useRand = 0;
                Com_PrintWarning(14, "Randsearch failed, falling back to sequential. CAC_FOUND-->CAC_NOT_VALIDATED\n");
                g_cacValidateState = CAC_NOTVALIDATED;
            }
            else
            {
                Com_PrintWarning(14, "Couldn't find any servers. CAC_FOUND-->CAC_FAILED\n");
                g_cacValidateState = CAC_FAILED;
            }
            break;
        case CAC_REQUESTSENT:
            if ( CL_CACValidate_IsTimedOut() )
            {
                v2 = Sys_Milliseconds();
                Com_DPrintf(
                    14,
                    "Waited %ims for cac validate response, retrying\n CAC_REQUESTSENT --> CAC_NOTVALIDATED\n",
                    v2 - g_cacValidateTimeout);
                g_cacValidateState = CAC_NOTVALIDATED;
                Com_PrintError(0, "CACValidate TIMED OUT\n");
            }
            break;
        case CAC_VALIDATED:
            UI_CloseMenu(0, "popup_gettingdata");
            Com_Printf(14, "CAC_VALIDATED --> CAC\n");
            LiveStorage_SetStatsWriteNeeded(0, 0, STATS_LOCATION_NORMAL);
            g_cacValidateState = CAC_DORMANT;
            break;
        case CAC_REJECTED:
            UI_CloseMenu(0, "popup_gettingdata");
            Com_DPrintf(14, "Server rejected our CAC\n CAC_REJECTED --> CAC_DORMANT\n");
            Com_PrintError(0, "CACValidate REJECTED\n");
            LiveStorage_ReadPlayerGlobalBlob();
            g_cacValidateState = CAC_DORMANT;
            break;
        default:
            if ( !"Unknown CACValidate state - %i\n"
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_pc_mp.cpp",
                            2521,
                            0,
                            "%s\n\t%s",
                            "\"Unknown CACValidate state - %i\\n\"",
                            (const char *)g_cacValidateState) )
            {
                __debugbreak();
            }
            break;
    }
#endif
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
#ifdef KISAK_LIVE_STUBS
    s_numServers = bdTaskByteBuffer::getHeaderSize((bdTaskByteBuffer *)task->remoteTask.m_ptr);
    Com_DPrintf(14, "QuickMatch: Backend returned %i results\n", s_numServers);
    if ( s_numServers )
        s_quickmatchstate = QM_PINGING;
    else
        s_quickmatchstate = QM_FAILED;
#endif
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
#ifdef KISAK_LIVE_STUBS
    bdReference<bdCommonAddr> v6; // [esp+20h] [ebp-2Ch] BYREF
    MatchMaking_PC_QUICKMATCH_Query query; // [esp+24h] [ebp-28h] BYREF
    TaskRecord *task; // [esp+40h] [ebp-Ch]
    bdRemoteTask *remoteTask; // [esp+44h] [ebp-8h]
    bdMatchMaking *matchmaking; // [esp+48h] [ebp-4h]

    if ( CL_QuickMatch_InProgress() )
    {
        Com_DPrintf(0, "Not doing quickmatch, quickmatch already in progress\n");
    }
    else if ( LiveStorage_DoWeHavePlaylists() )
    {
        s_lastPing = 0;
        s_numPingResponses = 0;
        s_numServers = 0;
        s_lastpingedserver = 0;
        s_serveriter = 0;
        matchmaking = dwGetMatchmaking(0);
        if ( !matchmaking
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_pc_mp.cpp",
                        2640,
                        0,
                        "%s",
                        "matchmaking") )
        {
            __debugbreak();
        }
        if ( matchmaking )
        {
            task = TaskManager2_CreateTask(task_QuickMatchFindSessions, 0, 0, 0);
            MatchMaking_PC_QUICKMATCH_Query::MatchMaking_PC_QUICKMATCH_Query(&query);
            query.m_memberPARAM_SERVERTYPE = servertype;
            query.m_memberPARAM_NETCODE_VERSION = 1044;
            query.m_memberPARAM_PLAYLIST = playlist;
            query.m_memberPARAM_PLAYLIST_VERSION = Playlist_GetVersionNumber();
            query.m_memberPARAM_LICENSE = -1;
            remoteTask = *(bdRemoteTask **)bdMatchMaking::findSessions(
                                                                             matchmaking,
                                                                             (int)&v6,
                                                                             3u,
                                                                             0,
                                                                             0x1F4u,
                                                                             (int)&query,
                                                                             &s_quickMatchServers);
            bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v6);
            if ( remoteTask )
            {
                bdReference<bdCommonAddr>::operator=(&task->remoteTask, remoteTask);
                TaskManager2_StartTask(task);
                s_quickmatchstate = QM_FINDING;
                I_strncpyz(s_quickmatch_params.mapname, mapname, 4);
                s_quickmatch_params.maxping = maxPing;
                s_quickmatch_params.minplayers = minPlayers;
                s_quickmatch_params.maxplayers = maxPlayers;
                s_quickmatch_params.isWager = 0;
            }
            bdSessionParams::~bdSessionParams(&query);
        }
    }
#endif
}

void __cdecl CL_QuickWager_Start()
{
#ifdef KISAK_LIVE_STUBS
    int ControllerIndex; // eax
    bdReference<bdCommonAddr> v1; // [esp+20h] [ebp-30h] BYREF
    MatchMaking_PC_WAGER_Query query; // [esp+24h] [ebp-2Ch] BYREF
    TaskRecord *task; // [esp+40h] [ebp-10h]
    bdRemoteTask *remoteTask; // [esp+44h] [ebp-Ch]
    char mapname[4]; // [esp+48h] [ebp-8h] BYREF
    bdMatchMaking *matchmaking; // [esp+4Ch] [ebp-4h]

    if ( CL_QuickMatch_InProgress() )
    {
        Com_DPrintf(0, "Not doing quickmatch, quickmatch already in progress\n");
    }
    else if ( LiveStorage_DoWeHavePlaylists() )
    {
        s_lastPing = 0;
        s_numPingResponses = 0;
        s_numServers = 0;
        s_lastpingedserver = 0;
        s_serveriter = 0;
        strcpy(mapname, "any");
        matchmaking = dwGetMatchmaking(0);
        if ( !matchmaking
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_pc_mp.cpp",
                        2684,
                        0,
                        "%s",
                        "matchmaking") )
        {
            __debugbreak();
        }
        if ( matchmaking )
        {
            task = TaskManager2_CreateTask(task_QuickMatchFindSessions, 0, 0, 0);
            MatchMaking_PC_WAGER_Query::MatchMaking_PC_WAGER_Query(&query);
            query.m_memberPARAM_SERVERTYPE = 2;
            query.m_memberPARAM_NETCODE_VERSION = 1044;
            query.m_memberPARAM_PLAYLIST_MIN = cl_wager_firstplaylist->current.integer;
            query.m_memberPARAM_PLAYLIST_MAX = cl_wager_lastplaylist->current.integer;
            query.m_memberPARAM_PLAYLIST_VERSION = Playlist_GetVersionNumber();
            query.m_memberPARAM_LICENSE = 0x7FFFFFFF;
            remoteTask = *(bdRemoteTask **)bdMatchMaking::findSessions(
                                                                             matchmaking,
                                                                             (int)&v1,
                                                                             5u,
                                                                             0,
                                                                             0x1F4u,
                                                                             (int)&query,
                                                                             &s_quickMatchServers);
            bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&v1);
            if ( remoteTask )
            {
                bdReference<bdCommonAddr>::operator=(&task->remoteTask, remoteTask);
                TaskManager2_StartTask(task);
                s_quickmatchstate = QM_FINDING;
                I_strncpyz(s_quickmatch_params.mapname, mapname, 4);
                s_quickmatch_params.maxping = cl_wager_maxping->current.integer;
                s_quickmatch_params.isWager = 1;
                ControllerIndex = Com_LocalClient_GetControllerIndex(0);
                LiveStats_GetIntPlayerStat(ControllerIndex, &s_quickmatch_params.codpoints, "CODPOINTS");
            }
            bdSessionParams::~bdSessionParams(&query);
        }
    }
#endif 
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
#ifdef KISAK_LIVE_STUBS
    netadr_t v0; // [esp-10h] [ebp-28h]
    unsigned int i; // [esp+0h] [ebp-18h]
    __int64 adr_8; // [esp+Ch] [ebp-Ch]

    HIDWORD(adr_8) = -1;
    LOWORD(adr_8) = 0;
    for ( i = 0; i < 0x32; ++i )
    {
        *(_QWORD *)&v0.type = 4;
        *(_QWORD *)&v0.port = adr_8;
        CL_InitServerInfo(&s_quickmatchCandidates[i], v0);
        s_quickmatchCandidates[i].ping = -1;
    }
    Cmd_AddCommandInternal("quickmatch", CL_QuickMatch_f, &CL_QuickMatch_f_VAR);
    Cmd_AddCommandInternal("wagermatch", CL_QuickWager_f, &CL_QuickWager_f_VAR);
    CL_QuickMatch_InitDvars();
#endif
}

bool __cdecl CL_QuickMatch_ServerMatches(MatchMakingInfo *server)
{
#ifdef KISAK_LIVE_STUBS
    bool v3; // [esp+4h] [ebp-14h]
    bool v4; // [esp+8h] [ebp-10h]
    bool retval; // [esp+17h] [ebp-1h]

    retval = 1;
    if ( I_strcmp(s_quickmatch_params.mapname, "any") )
        retval = I_strcmp(server->m_membermapname, s_quickmatch_params.mapname) == 0;
    if ( s_quickmatch_params.isWager )
    {
        return retval && server->m_memberserverType == 2;
    }
    else
    {
        v4 = retval && server->m_memberserverType == 1;
        v3 = v4 && server->m_numPlayers <= s_quickmatch_params.maxplayers;
        return v3 && server->m_numPlayers >= s_quickmatch_params.minplayers;
    }
#else
    return false;
#endif
}

void __cdecl CL_QuickMatch_PingServers()
{
#ifdef KISAK_LIVE_STUBS
    int v0; // eax
    int integer; // ecx
    int v2; // ecx
    netadr_t v3; // [esp-10h] [ebp-1B8h] BYREF
    serverInfo_t fakeserver; // [esp+10h] [ebp-198h] BYREF
    bdReference<bdCommonAddr> serveraddr; // [esp+188h] [ebp-20h] BYREF
    netadr_t fakeaddr; // [esp+18Ch] [ebp-1Ch]
    MatchMakingInfo *server; // [esp+1A0h] [ebp-8h]
    int i; // [esp+1A4h] [ebp-4h]

    if ( !cl_maxppf
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_pc_mp.cpp", 2809, 0, "%s", "cl_maxppf") )
    {
        __debugbreak();
    }
    i = 0;
    while ( 1 )
    {
        v0 = i;
        integer = cl_maxppf->current.integer;
        ++i;
        if ( v0 >= integer || s_serveriter >= s_numServers )
            break;
        server = (MatchMakingInfo *)(&s_quickMatchServers + 138 * s_serveriter++);
        if ( CL_QuickMatch_ServerMatches(server) )
        {
            ++s_lastpingedserver;
            Com_DPrintf(14, "Pinging %s\n", server->m_memberservername);
            server->m_pingedtime = Sys_Milliseconds();
            Com_DPrintf(14, "Server %i pingedtime set to %u\n", s_lastpingedserver, server->m_pingedtime);
            bdSecurityKey::bdSecurityKey(&fakeserver.xnkey);
            bdSecurityID::bdSecurityID(&fakeserver.xnkid);
            *(unsigned int *)fakeaddr.ip = 0;
            fakeaddr.port = 0;
            fakeaddr.type = NA_IP;
            v3.type = NA_IP;
            *(unsigned int *)v3.ip = 0;
            *(unsigned int *)&v3.port = *(unsigned int *)&fakeaddr.port;
            v3.addrHandleIndex = fakeaddr.addrHandleIndex;
            CL_InitServerInfo(&fakeserver, v3);
            v3.addrHandleIndex = v2;
            dwGetLocalCommonAddr((bdReference<bdCommonAddr> *)&v3.addrHandleIndex);
            bdMatchMakingInfo::getHostAddrAsCommonAddr(
                server,
                (bdReference<bdRemoteTask> *)&serveraddr,
                (bdReference<bdCommonAddr>)v3.addrHandleIndex);
            bdCommonAddr::serialize(serveraddr.m_ptr, fakeserver.xnaddr.addrBuff);
            fakeserver.xnkid = server->m_sessionID;
            CL_RawPingServer(&fakeserver, 8u);
            s_lastPing = Sys_Milliseconds();
            bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&serveraddr);
        }
        else
        {
            Com_DPrintf(14, "Skipping server %s\n", server->m_memberservername);
        }
    }
#endif
}

bool __cdecl CL_QuickMatch_GoodSessionFound()
{
#ifdef KISAK_LIVE_STUBS
    return s_serveriter == s_numServers || s_lastPing && (int)(Sys_Milliseconds() - s_lastPing) > 2000;
#else
    return false;
#endif
}

void __cdecl CL_QuickMatch_PingResponse(bdSecurityID *secID, msg_t *msg)
{
#ifdef KISAK_LIVE_STUBS
    int v2; // ecx
    char *v3; // eax
    unsigned __int8 v4; // al
    char *v5; // eax
    int v6; // eax
    __int64 v7; // [esp+8h] [ebp-45Ch] BYREF
    int v8; // [esp+18h] [ebp-44Ch]
    int v9; // [esp+1Ch] [ebp-448h]
    MatchMakingInfo *serverinfo; // [esp+30h] [ebp-434h]
    bdReference<bdCommonAddr> hostAddress; // [esp+34h] [ebp-430h] BYREF
    float fullscore; // [esp+38h] [ebp-42Ch]
    serverInfo_t *server; // [esp+3Ch] [ebp-428h]
    float fullness; // [esp+40h] [ebp-424h]
    netadr_t adr; // [esp+44h] [ebp-420h] BYREF
    float pingscore; // [esp+54h] [ebp-410h]
    unsigned int i; // [esp+58h] [ebp-40Ch]
    char strBuf[1024]; // [esp+5Ch] [ebp-408h] BYREF
    char *infoString; // [esp+460h] [ebp-4h]

    memset(strBuf, 0, sizeof(strBuf));
    for ( i = 0; i < s_numServers; ++i )
    {
        v9 = memcmp((const char *)secID, (const char *)&unk_2D3C49C + 552 * i, 8);
        v8 = v9;
        if ( !v9 && dword_2D3C6B0[138 * i] )
        {
            server = &s_quickmatchCandidates[s_numPingResponses++];
            infoString = MSG_ReadString(msg, strBuf, 0x400u);
            serverinfo = (MatchMakingInfo *)(&s_quickMatchServers + 138 * i);
            memset(adr.ip, 0, 12);
            adr.type = NA_IP;
            CL_InitServerInfo(server, (netadr_t)4uLL);
            HIDWORD(v7) = v2;
            dwGetLocalCommonAddr((bdReference<bdCommonAddr> *)&v7 + 1);
            bdMatchMakingInfo::getHostAddrAsCommonAddr(
                serverinfo,
                (bdReference<bdRemoteTask> *)&hostAddress,
                *(bdReference<bdCommonAddr> *)((char *)&v7 + 4));
            bdCommonAddr::serialize(hostAddress.m_ptr, server->xnaddr.addrBuff);
            server->ping = Sys_Milliseconds() - LOWORD(serverinfo->m_pingedtime);
            serverinfo->m_pingedtime = 0;
            v3 = Info_ValueForKey(infoString, "clients");
            v4 = atoi(v3);
            server->clients = v4;
            server->maxClients = serverinfo->m_maxPlayers;
            v5 = Info_ValueForKey(infoString, "wagerbet");
            v6 = atoi(v5);
            server->wagerBet = v6;
            I_strncpyz(server->hostName, serverinfo->m_memberservername, 32);
            I_strncpyz(server->mapName, serverinfo->m_membermapname, 32);
            server->xnkey = *(bdSecurityKey *)serverinfo->m_membersecKey;
            server->xnkid = serverinfo->m_sessionID;
            pingscore = (float)cl_quickmatch_pingweight->current.integer / (float)server->ping;
            fullness = (float)((float)server->clients / (float)server->maxClients) * 100.0;
            Com_DPrintf(14, "Server %s has pingscore of %f, fullness of %f\n", server->hostName, pingscore, fullness);
            if ( fullness >= 100.0 )
            {
                fullness = 0.0f;
                pingscore = 0.0f;
            }
            if ( s_quickmatch_params.isWager && server->wagerBet > s_quickmatch_params.codpoints )
            {
                fullness = -1.0f;
                pingscore = 1.0f;
            }
            fullscore = (float)cl_quickmatch_fullnessweight->current.integer * fullness;
            Com_IntToGametype(server->gameType, serverinfo->m_memberGAME_TYPE);
            fullscore = fullscore + 1.0;
            server->score = (int)(float)(fullscore * pingscore);
            server->bdUserID = serverinfo->m_memberdemonwareID;
            bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&hostAddress);
            return;
        }
    }
#endif
}

char __cdecl CL_QuickMatch_ChooseSession()
{
#ifdef KISAK_LIVE_STUBS
    unsigned int i; // [esp+0h] [ebp-8h]
    bool retval; // [esp+7h] [ebp-1h]

    if ( s_quickmatchstate != QM_CHOOSE
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_main_pc_mp.cpp",
                    2928,
                    0,
                    "%s",
                    "QM_CHOOSE == s_quickmatchstate") )
    {
        __debugbreak();
    }
    retval = 0;
    if ( s_numPingResponses )
    {
        qsort(
            s_quickmatchCandidates,
            s_numPingResponses,
            0x178u,
            (int (__cdecl *)(const void *, const void *))CL_QuickMatch_CompareServers);
        Com_DPrintf(14, "Hostname\tPing\t\tClients\tScore\n");
        for ( i = 0; i < s_numPingResponses; ++i )
            Com_DPrintf(
                14,
                "%s\t\t%i\t\t%i\t\t%i\n",
                s_quickmatchCandidates[i].hostName,
                s_quickmatchCandidates[i].ping,
                s_quickmatchCandidates[i].clients,
                s_quickmatchCandidates[i].score);
        if ( s_quickmatch_params.isWager )
        {
            Dvar_SetIntByName("ui_wagerbet", s_quickmatchCandidates[0].wagerBet);
            if ( s_quickmatch_params.codpoints < s_quickmatchCandidates[0].wagerBet )
                Cmd_ExecuteSingleCommand(0, 0, "openmenu WagerDeadBeat");
            else
                Cmd_ExecuteSingleCommand(0, 0, "openmenu QuickWagerConfirmation");
        }
        else
        {
            CL_Connect(s_quickmatchCandidates);
        }
        return 1;
    }
    return retval;
#else
    return 0;
#endif
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
#ifdef KISAK_LIVE_STUBS
    float responsepercent; // [esp+1Ch] [ebp-4h]

    responsepercent = (double)s_numPingResponses / (double)s_lastpingedserver * 100.0;
    Com_DPrintf(14, "CACValidate %f\n", responsepercent);
    return CL_QuickMatch_GoodSessionFound()
            || s_numPingResponses && responsepercent >= (float)cl_quickmatch_resultspercent->current.integer;
#else
    return false;
#endif
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

