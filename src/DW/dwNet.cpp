#ifdef KISAK_DW

#include "dwNet.h"

void __cdecl dwRegisterRawInterceptor(bdPacketInterceptor *rawinterceptor)
{
    bdSocketRouter *socketRouter; // [esp+0h] [ebp-4h]

    socketRouter = dwGetSocketRouter();
    if ( socketRouter )
    {
        bdSocketRouter::registerInterceptor(socketRouter, rawinterceptor);
        Com_DPrintf(15, "Registered raw interceptor\n");
        s_interceptorStarted = 1;
    }
}

int __cdecl dwRawSendTo(netadr_t *adr, unsigned __int8 *data, unsigned int size)
{
    bdNetImpl *Instance; // eax
    bdAddr newaddr; // [esp+50h] [ebp-14h] BYREF
    bdSocket *socket; // [esp+58h] [ebp-Ch]
    unsigned __int8 *newbuf; // [esp+5Ch] [ebp-8h]
    int retval; // [esp+60h] [ebp-4h]

    if ( !s_interceptorStarted
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwNet.cpp", 422, 0, "%s", "s_interceptorStarted") )
    {
        __debugbreak();
    }
    if ( !s_interceptorStarted )
        Com_PrintError(0, "sendto before interceptor started, show this to Ewan!\n");
    retval = 0;
    if ( size < 0x4F0 )
    {
        newbuf = (unsigned __int8 *)operator new[](size + 4);
        if ( newbuf )
        {
            newbuf[3] = -1;
            newbuf[2] = -1;
            newbuf[1] = -1;
            *newbuf = -1;
            memcpy(newbuf + 4, data, size);
            Instance = bdSingleton<bdNetImpl>::getInstance();
            socket = *(bdSocket **)(bdCommonAddr::getLocalAddrs(Instance) + 4);
            bdAddr::bdAddr(&newaddr);
            Com_sprintf(s, 0x40u, "%i.%i.%i.%i:%i", adr->ip[0], adr->ip[1], adr->ip[2], adr->ip[3], adr->port);
            bdAddr::set(&newaddr, s);
            retval = socket->sendTo(socket, &newaddr, newbuf, size + 4);
            operator delete[](newbuf);
            bdInetAddr::~bdInetAddr(&newaddr.m_address);
        }
    }
    return retval;
}

int __cdecl dwSendSysLogPacket(netadr_t *adr, void *data, unsigned int size)
{
    bdNetImpl *Instance; // eax
    bdAddr newaddr; // [esp+48h] [ebp-10h] BYREF
    bdSocket *socket; // [esp+50h] [ebp-8h]
    int retval; // [esp+54h] [ebp-4h]

    retval = 0;
    if ( size < 0x4F0 )
    {
        Instance = bdSingleton<bdNetImpl>::getInstance();
        socket = *(bdSocket **)(bdCommonAddr::getLocalAddrs(Instance) + 4);
        bdAddr::bdAddr(&newaddr);
        Com_sprintf(s_0, 0x40u, "%i.%i.%i.%i:%i", adr->ip[0], adr->ip[1], adr->ip[2], adr->ip[3], adr->port);
        bdAddr::set(&newaddr, s_0);
        retval = socket->sendTo(socket, &newaddr, data, size);
        bdInetAddr::~bdInetAddr(&newaddr.m_address);
    }
    return retval;
}

void __cdecl dwNetStart(bool online)
{
    bdInetAddr *v1; // eax
    bdString v2; // [esp+C4h] [ebp-6Ch] BYREF
    bdString v3; // [esp+C8h] [ebp-68h] BYREF
    bdString v4; // [esp+CCh] [ebp-64h] BYREF
    bdString value; // [esp+D0h] [ebp-60h] BYREF
    bdInetAddr v6; // [esp+D4h] [ebp-5Ch] BYREF
    bdNetStartParams startParams; // [esp+D8h] [ebp-58h] BYREF
    int memoryAllocation; // [esp+124h] [ebp-Ch]
    bdNetImpl *net; // [esp+128h] [ebp-8h]
    bool restart; // [esp+12Fh] [ebp-1h]

    restart = 1;
    if ( g_dwNetOnlineFailed )
        online = 0;
    switch ( g_dwNetStatus )
    {
        case DW_NET_ERROR_START_FAILED:
        case DW_NET_ERROR_NO_LOCAL_IP:
        case DW_NET_NOT_STARTED:
            restart = 1;
            break;
        case DW_NET_STARTING_LAN:
            restart = online;
            break;
        case DW_NET_STARTED_LAN:
            restart = online;
            break;
        case DW_NET_STARTING_ONLINE:
            restart = !online;
            break;
        case DW_NET_STARTED_ONLINE:
            restart = 0;
            break;
        default:
            break;
    }
    if ( restart )
    {
        restart = dwGetNumAddrHandles() == 0;
        memoryAllocation = 1;
        net = bdSingleton<bdNetImpl>::getInstance();
        if ( !net && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwNet.cpp", 570, 0, "%s", "net") )
            __debugbreak();
        if ( net )
        {
            dwNetStop();
            bdNetStartParams::bdNetStartParams(&startParams);
            if ( dwPlatformInit(&startParams) )
            {
                v1 = bdInetAddr::bdInetAddr(&v6, "127.0.0.1");
                bdArray<bdInetAddr>::pushBack(&startParams.m_localAddresses, v1);
                bdInetAddr::~bdInetAddr(&v6);
            }
            bdArray<bdString>::clear(&startParams.m_natTravHosts);
            bdString::bdString(&value, "cod7-stun.us.demonware.net");
            bdArray<bdString>::pushBack(&startParams.m_natTravHosts, &value);
            bdString::~bdString(&value);
            bdString::bdString(&v4, "cod7-stun.eu.demonware.net");
            bdArray<bdString>::pushBack(&startParams.m_natTravHosts, &v4);
            bdString::~bdString(&v4);
            bdString::bdString(&v3, "cod7-stun.jp.demonware.net");
            bdArray<bdString>::pushBack(&startParams.m_natTravHosts, &v3);
            bdString::~bdString(&v3);
            bdString::bdString(&v2, "cod7-stun.au.demonware.net");
            bdArray<bdString>::pushBack(&startParams.m_natTravHosts, &v2);
            bdString::~bdString(&v2);
            startParams.m_onlineGame = online;
            startParams.m_UPnPConfig.m_discoveryTimeout = 1.0f;
            if ( bdNetImpl::start(net, &startParams) )
            {
                if ( online )
                    g_dwNetStatus = DW_NET_STARTING_ONLINE;
                else
                    g_dwNetStatus = DW_NET_STARTING_LAN;
            }
            else if ( g_dwNetStatus )
            {
                Com_Printf(16, "Failed to start network system.\n");
                g_dwNetStatus = DW_NET_ERROR_START_FAILED;
            }
            bdNetStartParams::~bdNetStartParams(&startParams);
        }
        else if ( g_dwNetStatus != DW_NET_ERROR_NO_LOCAL_IP )
        {
            Com_Printf(16, "Could not obtain local IP address.\n");
            g_dwNetStatus = DW_NET_ERROR_NO_LOCAL_IP;
        }
    }
}

void __thiscall bdNetStartParams::~bdNetStartParams(bdNetStartParams *this)
{
    bdInetAddr::~bdInetAddr(&this->m_UPnPConfig.m_gatewayAddr);
    bdArray<bdInetAddr>::clear(&this->m_localAddresses);
    bdArray<bdString>::clear(&this->m_natTravHosts);
}

void __cdecl dwNetStop()
{
    bdAddressMap *addressMap; // [esp+48h] [ebp-8h]
    bdNetImpl *net; // [esp+4Ch] [ebp-4h]

    Sys_EnterCriticalSection(CRITSECT_DEMONWARE);
    addressMap = dwGetAddressMap();
    //BLOPS_NULLSUB();
    if ( addressMap )
        bdArray<bdReference<bdCommonAddr>>::~bdArray<bdReference<bdCommonAddr>>(addressMap);
    net = bdSingleton<bdNetImpl>::getInstance();
    if ( net && bdNetImpl::getStatus(net) )
        bdNetImpl::stop(net);
    s_interceptorStarted = 0;
    Sys_LeaveCriticalSection(CRITSECT_DEMONWARE);
}

void __cdecl dwNetPump()
{
    bdSocketRouter *socketRouter; // [esp+4Ch] [ebp-Ch]
    enum bdNetImpl::bdNetStatus status; // [esp+50h] [ebp-8h]
    bdNetImpl *net; // [esp+54h] [ebp-4h]

    Sys_EnterCriticalSection(CRITSECT_DEMONWARE);
    net = bdSingleton<bdNetImpl>::getInstance();
    if ( !net && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwNet.cpp", 776, 0, "%s", "net") )
        __debugbreak();
    if ( net )
    {
        if ( bdNetImpl::getStatus(net) == BD_NET_PENDING )
            bdNetImpl::pump(net);
        status = bdNetImpl::getStatus(net);
        if ( status == BD_NET )
        {
            if ( !s_interceptorStarted )
                dwRegisterRawInterceptor(&rawlistener);
            if ( g_dwNetStatus == DW_NET_STARTING_LAN )
            {
                g_dwNetStatus = DW_NET_STARTED_LAN;
            }
            else if ( g_dwNetStatus == DW_NET_STARTING_ONLINE )
            {
                g_dwNetStatus = DW_NET_STARTED_ONLINE;
            }
            socketRouter = dwGetSocketRouter();
            if ( socketRouter )
                socketRouter->pump(socketRouter);
            else
                Com_Printf(16, "dwNetPump():No socket router.\n");
            dwCheckConnections();
        }
        else if ( (unsigned int)status > 0xFFFFFFFC )
        {
            if ( g_dwNetStatus == DW_NET_STARTING_ONLINE )
                g_dwNetOnlineFailed = 1;
            g_dwNetStatus = DW_NET_ERROR_START_FAILED;
        }
    }
    Sys_LeaveCriticalSection(CRITSECT_DEMONWARE);
}

void dwCheckConnections()
{
    bdReference<bdAddrHandle> v0[3]; // [esp-4h] [ebp-58h] BYREF
    bdAddrHandle *v1; // [esp+8h] [ebp-4Ch]
    volatile int v2; // [esp+Ch] [ebp-48h]
    volatile int m_refCount; // [esp+10h] [ebp-44h]
    bdReference<bdAddrHandle> *v4; // [esp+20h] [ebp-34h]
    bdAddrHandle *v5; // [esp+24h] [ebp-30h]
    bdReference<bdAddrHandle> *v6; // [esp+28h] [ebp-2Ch]
    bdAddrHandle *v7; // [esp+2Ch] [ebp-28h]
    bdAddrHandle *v8; // [esp+30h] [ebp-24h]
    bdAddrHandle *m_ptr; // [esp+34h] [ebp-20h]
    bdDTLSAssociationStatus status; // [esp+40h] [ebp-14h]
    bdReference<bdAddrHandle> addrHandle; // [esp+44h] [ebp-10h] BYREF
    CountedAddrHandle *countedAddrHandle; // [esp+48h] [ebp-Ch]
    int i; // [esp+4Ch] [ebp-8h]
    bdSocketRouter *socketRouter; // [esp+50h] [ebp-4h]

    socketRouter = dwGetSocketRouter();
    if ( socketRouter )
    {
        for ( i = 0; i < maxAddrHandles; ++i )
        {
            countedAddrHandle = &g_addrHandles[i];
            m_ptr = countedAddrHandle->m_addrHandle.m_ptr;
            addrHandle.m_ptr = m_ptr;
            if ( m_ptr )
            {
                v8 = addrHandle.m_ptr;
                InterlockedIncrement(&addrHandle.m_ptr->m_refCount);
                m_refCount = v8->m_refCount;
            }
            if ( addrHandle.m_ptr )
            {
                v6 = v0;
                v0[0] = addrHandle;
                v7 = addrHandle.m_ptr;
                InterlockedIncrement(&addrHandle.m_ptr->m_refCount);
                v2 = v7->m_refCount;
                v1 = (bdAddrHandle *)((int (__thiscall *)(bdSocketRouter *, bdAddrHandle *))socketRouter->getStatus)(
                                                             socketRouter,
                                                             v0[0].m_ptr);
                status = (bdDTLSAssociationStatus)v1;
                v0[2].m_ptr = v1;
                if ( v1 == (bdAddrHandle *)3 )
                {
                    Com_Printf(16, "Secure socket timed out.\n");
                    v4 = v0;
                    v0[0] = addrHandle;
                    if ( addrHandle.m_ptr )
                    {
                        v5 = v4->m_ptr;
                        InterlockedIncrement(&v5->m_refCount);
                        v0[1] = (bdReference<bdAddrHandle>)v5->m_refCount;
                    }
                    dwHandleLostConnection(v0[0]);
                }
            }
            bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>((bdReference<bdCommonAddr> *)&addrHandle);
        }
    }
    else
    {
        Com_Printf(16, "dwCheckConnections():No socket router.\n");
    }
}

void __cdecl dwHandleLostConnection(bdReference<bdAddrHandle> addrHandle)
{
    bdSocketRouter *SocketRouter; // eax
    CountedAddrHandle *countedAddrHandle; // [esp+18h] [ebp-10h]
    int i; // [esp+20h] [ebp-8h]
    bool found; // [esp+27h] [ebp-1h]

    if ( !addrHandle.m_ptr
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwNet.cpp", 676, 0, "%s", "addrHandle.notNull()") )
    {
        __debugbreak();
    }
    if ( addrHandle.m_ptr )
    {
        found = 0;
        for ( i = 0; !found && i < maxAddrHandles; ++i )
        {
            if ( g_addrHandles[i].m_addrHandle.m_ptr )
                found = bdAddrHandle::operator==(addrHandle.m_ptr, (int)g_addrHandles[i].m_addrHandle.m_ptr);
        }
        if ( found )
        {
            countedAddrHandle = &g_addrHandles[i - 1];
            SocketRouter = dwGetSocketRouter();
            SocketRouter->disconnect(SocketRouter, &countedAddrHandle->m_addrHandle);
            dwClearAddrHandle((bdReference<bdRemoteTask> *)countedAddrHandle);
        }
    }
    bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>((bdReference<bdCommonAddr> *)&addrHandle);
}

void __cdecl dwClearAddrHandle(bdReference<bdRemoteTask> *countedAddrHandle)
{
    Sys_EnterCriticalSection(CRITSECT_DEMONWARE);
    if ( Sys_IsMainThread() )
    {
        Com_Printf(0, "Clearing addrhandle on main thread\n");
    }
    else if ( Sys_IsServerThread() )
    {
        Com_Printf(0, "Clearing addrhandle on server thread\n");
    }
    bdReference<bdCommonAddr>::operator=(countedAddrHandle, 0);
    countedAddrHandle[8].m_ptr = (bdRemoteTask *)-1;
    countedAddrHandle[1].m_ptr = 0;
    countedAddrHandle[2].m_ptr = 0;
    countedAddrHandle[3].m_ptr = 0;
    countedAddrHandle[4].m_ptr = 0;
    countedAddrHandle[5].m_ptr = 0;
    countedAddrHandle[6].m_ptr = 0;
    LOWORD(countedAddrHandle[7].m_ptr) = 0;
    BYTE2(countedAddrHandle[7].m_ptr) = 0;
    Sys_LeaveCriticalSection(CRITSECT_DEMONWARE);
}

void __cdecl dwCreateSecIDAndKey(bdSecurityID *id, bdSecurityKey *key)
{
    Sys_EnterCriticalSection(CRITSECT_DEMONWARE);
    if ( dwGetSecurityKeyMap() )
        bdSecurityKeyMap::initKey(id, key);
    Sys_LeaveCriticalSection(CRITSECT_DEMONWARE);
}

char __cdecl dwRegisterSecIDAndKey(bdSecurityID *id, bdSecurityKey *key)
{
    bdSecurityKeyMap *keyMap; // [esp+0h] [ebp-8h]
    char registered; // [esp+7h] [ebp-1h]

    registered = 0;
    Sys_EnterCriticalSection(CRITSECT_DEMONWARE);
    keyMap = dwGetSecurityKeyMap();
    if ( keyMap )
        registered = bdSecurityKeyMap::registerKey(keyMap, id, key);
    Sys_LeaveCriticalSection(CRITSECT_DEMONWARE);
    return registered;
}

int __cdecl dwSendTo(unsigned int length, unsigned __int8 *data, netadr_t to)
{
    bdAddrHandle *v4; // [esp-Ch] [ebp-550h] BYREF
    unsigned __int8 *v5; // [esp-8h] [ebp-54Ch]
    bdAddrHandle *m_ptr; // [esp-4h] [ebp-548h] BYREF
    int v7; // [esp+0h] [ebp-544h]
    volatile int v8; // [esp+4h] [ebp-540h]
    bdDTLSAssociationStatus v9; // [esp+8h] [ebp-53Ch]
    volatile int m_refCount; // [esp+Ch] [ebp-538h]
    bdAddrHandle **v11; // [esp+1Ch] [ebp-528h]
    bdAddrHandle *v12; // [esp+20h] [ebp-524h]
    bdAddrHandle **v13; // [esp+24h] [ebp-520h]
    bdAddrHandle *v14; // [esp+28h] [ebp-51Ch]
    int v15; // [esp+2Ch] [ebp-518h]
    unsigned __int16 swappedDataLength; // [esp+38h] [ebp-50Ch]
    unsigned __int8 sendBuffer[1264]; // [esp+3Ch] [ebp-508h] BYREF
    unsigned __int16 dataLength; // [esp+52Ch] [ebp-18h]
    bdDTLSAssociationStatus status; // [esp+530h] [ebp-14h]
    bdReference<bdAddrHandle> addrHandle; // [esp+534h] [ebp-10h] BYREF
    int bytesSent; // [esp+538h] [ebp-Ch]
    bdSocketRouter *socketRouter; // [esp+53Ch] [ebp-8h]

    Sys_EnterCriticalSection(CRITSECT_DEMONWARE);
    bytesSent = -9;
    dwNetadrToAddrHandle(&addrHandle, &to);
    socketRouter = dwGetSocketRouter();
    if ( addrHandle.m_ptr && socketRouter )
    {
        v13 = &m_ptr;
        m_ptr = addrHandle.m_ptr;
        v14 = *v13;
        InterlockedIncrement(&v14->m_refCount);
        m_refCount = v14->m_refCount;
        v9 = ((int (__thiscall *)(bdSocketRouter *, bdAddrHandle *))socketRouter->getStatus)(socketRouter, m_ptr);
        status = v9;
        if ( v9 == BD_SOCKET_CONNECTED )
        {
            if ( length > 0x4EE )
            {
                bytesSent = -6;
            }
            else
            {
                dataLength = length;
                swappedDataLength = length;
                *(_WORD *)sendBuffer = length;
                memcpy(&sendBuffer[2], data, length);
                m_ptr = (bdAddrHandle *)(length + 2);
                v5 = sendBuffer;
                v11 = &v4;
                v4 = addrHandle.m_ptr;
                if ( addrHandle.m_ptr )
                {
                    v12 = *v11;
                    InterlockedIncrement(&v12->m_refCount);
                    v8 = v12->m_refCount;
                }
                v7 = ((int (__thiscall *)(bdSocketRouter *, bdAddrHandle *, unsigned __int8 *, bdAddrHandle *))socketRouter->sendTo)(
                             socketRouter,
                             v4,
                             v5,
                             m_ptr);
                bytesSent = v7;
            }
        }
        else
        {
            Com_PrintError(16, "DROPPING %i byte packet because we're still connecting to the remote address\n", length);
        }
    }
    else
    {
        Com_PrintError(
            16,
            "DROPPING %i byte packet because we're still connecting to the remote address (addrHandle=%d, socketRouter=%d)\n",
            length,
            addrHandle.m_ptr != 0,
            socketRouter != 0);
    }
    Sys_LeaveCriticalSection(CRITSECT_DEMONWARE);
    v15 = bytesSent;
    bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>((bdReference<bdCommonAddr> *)&addrHandle);
    return v15;
}

int __cdecl dwRecvFrom(netadr_t *const from, unsigned __int8 *data, unsigned int size)
{
    bdAddr *RealAddr; // eax
    bdAddr *Address; // eax
    bdAddr *v6; // eax
    bdReference<bdAddrHandle> v7; // [esp-10h] [ebp-6Ch] BYREF
    bdSecurityID *p_secID; // [esp-Ch] [ebp-68h]
    unsigned int *p_index; // [esp-8h] [ebp-64h]
    bool *p_newHandle; // [esp-4h] [ebp-60h]
    volatile int m_refCount; // [esp+4h] [ebp-58h]
    bdAddrHandle *v13; // [esp+14h] [ebp-48h]
    bdAddrHandle *v14; // [esp+18h] [ebp-44h]
    bdReference<bdAddrHandle> *v15; // [esp+1Ch] [ebp-40h]
    bdAddrHandle *m_ptr; // [esp+20h] [ebp-3Ch]
    bdAddr v17; // [esp+24h] [ebp-38h] BYREF
    bdAddr v18; // [esp+2Ch] [ebp-30h] BYREF
    unsigned int ip32; // [esp+38h] [ebp-24h]
    bool newHandle; // [esp+3Fh] [ebp-1Dh] BYREF
    bdSecurityID secID; // [esp+40h] [ebp-1Ch] BYREF
    unsigned int index; // [esp+4Ch] [ebp-10h] BYREF
    bdReference<bdAddrHandle> addrHandle; // [esp+50h] [ebp-Ch] BYREF
    bdSocketRouter *socketRouter; // [esp+54h] [ebp-8h]
    int bytesRecv; // [esp+58h] [ebp-4h]

    if ( G_ExitAfterToolComplete() )
        return 0;
    Sys_EnterCriticalSection(CRITSECT_DEMONWARE);
    bytesRecv = -9;
    socketRouter = dwGetSocketRouter();
    if ( socketRouter && bdSocketRouter::getStatus(socketRouter) == BD_SOCKET_ROUTER_INITIALIZED )
    {
        addrHandle.m_ptr = 0;
        bytesRecv = socketRouter->receiveFrom(socketRouter, &addrHandle, data, size);
        if ( bytesRecv > 0 && addrHandle.m_ptr )
        {
            index = 0;
            bdSecurityID::bdSecurityID(&secID);
            bdAddrHandle::getSecID(addrHandle.m_ptr, &secID);
            p_newHandle = &newHandle;
            p_index = &index;
            p_secID = &secID;
            v15 = &v7;
            v7.m_ptr = addrHandle.m_ptr;
            if ( addrHandle.m_ptr )
            {
                m_ptr = v15->m_ptr;
                InterlockedIncrement(&m_ptr->m_refCount);
                m_refCount = m_ptr->m_refCount;
            }
            if ( dwRegisterAddrHandle(v7, p_secID, p_index, p_newHandle) )
            {
                bytesRecv -= 2;
                memmove(data, data + 2, bytesRecv);
                v14 = addrHandle.m_ptr;
                RealAddr = bdAddrHandle::getRealAddr(addrHandle.m_ptr, &v18);
                Address = bdAddr::getAddress(RealAddr);
                ip32 = (unsigned int)bdString::getBuffer((bdString *)Address);
                bdInetAddr::~bdInetAddr(&v18.m_address);
                *(unsigned int *)from->ip = ip32;
                v13 = addrHandle.m_ptr;
                v6 = bdAddrHandle::getRealAddr(addrHandle.m_ptr, &v17);
                from->port = bdAddr::getPort(v6);
                bdInetAddr::~bdInetAddr(&v17.m_address);
                from->type = NA_IP;
                from->addrHandleIndex = index;
                if ( from->addrHandleIndex > maxAddrHandles
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwNet.cpp",
                                1158,
                                0,
                                "%s",
                                "from->addrHandleIndex <= maxAddrHandles") )
                {
                    __debugbreak();
                }
            }
            else
            {
                if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwNet.cpp", 1162, 0, "%s", "false") )
                    __debugbreak();
                bytesRecv = 0;
            }
        }
        bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>((bdReference<bdCommonAddr> *)&addrHandle);
    }
    Sys_LeaveCriticalSection(CRITSECT_DEMONWARE);
    return bytesRecv;
}

bool __cdecl dwRegisterAddrHandle(
                bdReference<bdAddrHandle> addrHandle,
                bdSecurityID *secID,
                unsigned int *index,
                bool *newHandle)
{
    int v4; // eax
    unsigned int v5; // ecx
    CountedAddrHandle *v6; // esi
    bdCommonAddr *v7; // ecx
    bdReference<bdCommonAddr> *v8; // edx
    bool v10; // [esp+2Bh] [ebp-51h]
    char addrStr[32]; // [esp+2Ch] [ebp-50h] BYREF
    unsigned int numFreeSlots; // [esp+50h] [ebp-2Ch]
    bdReference<bdCommonAddr> *v13; // [esp+54h] [ebp-28h]
    unsigned int j; // [esp+58h] [ebp-24h]
    unsigned int secIDGapIndex; // [esp+5Ch] [ebp-20h]
    bool secIDFoundGap; // [esp+62h] [ebp-1Ah]
    bool existingSecID; // [esp+63h] [ebp-19h]
    unsigned int numSecIDSlots; // [esp+64h] [ebp-18h]
    CountedAddrHandle *countedAddrHandle; // [esp+68h] [ebp-14h]
    bool foundGap; // [esp+6Fh] [ebp-Dh]
    int i; // [esp+70h] [ebp-Ch]
    unsigned int gapIndex; // [esp+74h] [ebp-8h]
    bool found; // [esp+7Ah] [ebp-2h]
    bool ok; // [esp+7Bh] [ebp-1h]

    Sys_EnterCriticalSection(CRITSECT_DEMONWARE);
    ok = 0;
    *newHandle = 0;
    if ( addrHandle.m_ptr )
    {
        found = 0;
        foundGap = 0;
        gapIndex = 0;
        for ( i = 0; !found && i < maxAddrHandles; ++i )
        {
            if ( g_addrHandles[i].m_addrHandle.m_ptr )
            {
                found = bdAddrHandle::operator==(addrHandle.m_ptr, (int)g_addrHandles[i].m_addrHandle.m_ptr);
            }
            else if ( !foundGap )
            {
                gapIndex = i;
                foundGap = 1;
            }
        }
        if ( !found
            && !foundGap
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwNet.cpp", 960, 0, "%s", "found || foundGap") )
        {
            __debugbreak();
        }
        if ( found )
        {
            *index = i - 1;
            countedAddrHandle = &g_addrHandles[*index];
            numSecIDSlots = 3;
            if ( countedAddrHandle->m_primaryIndex <= -1
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwNet.cpp",
                            975,
                            0,
                            "%s",
                            "countedAddrHandle.m_primaryIndex > -1") )
            {
                __debugbreak();
            }
            existingSecID = 0;
            secIDFoundGap = 0;
            secIDGapIndex = 0;
            for ( j = 0; !existingSecID && j < 3; ++j )
            {
                if ( countedAddrHandle->m_secIDs[j].m_active )
                {
                    existingSecID = bdSecurityID::operator==(countedAddrHandle->m_secIDs[j].m_secID.ab, secID);
                }
                else if ( !secIDFoundGap )
                {
                    secIDFoundGap = 1;
                    secIDGapIndex = j;
                }
            }
            if ( !existingSecID
                && !secIDFoundGap
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwNet.cpp",
                            994,
                            0,
                            "%s",
                            "existingSecID || secIDFoundGap") )
            {
                __debugbreak();
            }
            if ( existingSecID )
            {
                if ( countedAddrHandle->m_primaryIndex == -1
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwNet.cpp",
                                1019,
                                0,
                                "%s",
                                "countedAddrHandle.m_primaryIndex != -1") )
                {
                    __debugbreak();
                }
                if ( !countedAddrHandle->m_secIDs[countedAddrHandle->m_primaryIndex].m_active
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwNet.cpp",
                                1020,
                                0,
                                "%s",
                                "countedAddrHandle.m_secIDs[countedAddrHandle.m_primaryIndex].m_active") )
                {
                    __debugbreak();
                }
                ok = 1;
            }
            else
            {
                if ( !secIDFoundGap
                    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwNet.cpp", 998, 0, "%s", "secIDFoundGap") )
                {
                    __debugbreak();
                }
                if ( secIDFoundGap )
                {
                    if ( countedAddrHandle->m_secIDs[secIDGapIndex].m_active
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwNet.cpp",
                                    1004,
                                    0,
                                    "%s",
                                    "countedAddrHandle.m_secIDs[secIDGapIndex].m_active == false") )
                    {
                        __debugbreak();
                    }
                    countedAddrHandle->m_secIDs[secIDGapIndex].m_active = 1;
                    v4 = *(unsigned int *)&secID->ab[4];
                    v5 = secIDGapIndex;
                    v6 = countedAddrHandle;
                    *(unsigned int *)countedAddrHandle->m_secIDs[v5].m_secID.ab = *(unsigned int *)secID->ab;
                    *(unsigned int *)&v6->m_secIDs[v5].m_secID.ab[4] = v4;
                    ok = 1;
                }
            }
        }
        else if ( foundGap )
        {
            v13 = (bdReference<bdCommonAddr> *)&g_addrHandles[gapIndex];
            if ( v13[8].m_ptr != (bdCommonAddr *)-1
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwNet.cpp",
                            1027,
                            0,
                            "%s",
                            "countedAddrHandle.m_primaryIndex == -1") )
            {
                __debugbreak();
            }
            v13[8].m_ptr = 0;
            v7 = *(bdCommonAddr **)&secID->ab[4];
            v8 = v13;
            v13[1].m_ptr = *(bdCommonAddr **)secID->ab;
            v8[2].m_ptr = v7;
            LOBYTE(v13[3].m_ptr) = 1;
            bdReference<bdCommonAddr>::operator=(v13, (const bdReference<bdCommonAddr> *)&addrHandle);
            *index = gapIndex;
            ok = 1;
            *newHandle = 1;
        }
        if ( ok )
        {
            if ( *newHandle )
            {
                dwAddrIndexToAddrString(*index, addrStr, 0x20u);
                numFreeSlots = dwGetNumFreeAddrHandleSlots();
                Com_Printf(
                    16,
                    "Registered new address handle (%s) in slot %u of %u, %u remain\n",
                    addrStr,
                    *index,
                    maxAddrHandles,
                    numFreeSlots);
            }
        }
        else
        {
            Com_PrintWarning(16, "WARNING: Out of security associations\n");
        }
    }
    Sys_LeaveCriticalSection(CRITSECT_DEMONWARE);
    v10 = ok;
    bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>((bdReference<bdCommonAddr> *)&addrHandle);
    return v10;
}

unsigned int __cdecl dwGetNumFreeAddrHandleSlots()
{
    int i; // [esp+0h] [ebp-8h]
    unsigned int numFreeSlots; // [esp+4h] [ebp-4h]

    numFreeSlots = 0;
    for ( i = 0; i < maxAddrHandles; ++i )
    {
        if ( !g_addrHandles[i].m_addrHandle.m_ptr )
        {
            if ( dword_336ADB8[9 * i] != -1
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwNet.cpp",
                            920,
                            0,
                            "%s",
                            "g_addrHandles[i].m_primaryIndex == -1") )
            {
                __debugbreak();
            }
            ++numFreeSlots;
        }
    }
    return numFreeSlots;
}

bool __cdecl dwCommonAddrToNetadr(netadr_t *const adr, bool *commonAddrBuf, bdCommonAddr *secID)
{
    int *AddressMap; // eax
    bdAddr *RealAddr; // eax
    bdAddr *Address; // eax
    bdAddr *v6; // eax
    bdReference<bdAddrHandle> v8; // [esp-10h] [ebp-104h] BYREF
    bdReference<bdCommonAddr> v9; // [esp-Ch] [ebp-100h] BYREF
    bdReference<bdCommonAddr> v10; // [esp-8h] [ebp-FCh] BYREF
    bool *p_addrHandle; // [esp-4h] [ebp-F8h] BYREF
    bdAddrHandle *v12; // [esp+0h] [ebp-F4h] BYREF
    volatile int v13; // [esp+4h] [ebp-F0h]
    int v14; // [esp+8h] [ebp-ECh]
    int v15; // [esp+Ch] [ebp-E8h]
    volatile int v17; // [esp+14h] [ebp-E0h]
    volatile int v19; // [esp+1Ch] [ebp-D8h]
    char v20; // [esp+23h] [ebp-D1h]
    volatile int v21; // [esp+24h] [ebp-D0h]
    volatile int m_refCount; // [esp+28h] [ebp-CCh]
    bdAddrHandle *v23; // [esp+50h] [ebp-A4h]
    bdAddrHandle *v24; // [esp+54h] [ebp-A0h]
    bdAddrHandle **v25; // [esp+58h] [ebp-9Ch]
    bdAddrHandle *v26; // [esp+5Ch] [ebp-98h]
    bool **p_p_addrHandle; // [esp+60h] [ebp-94h]
    bool *v28; // [esp+64h] [ebp-90h]
    bdReference<bdAddrHandle> *v29; // [esp+68h] [ebp-8Ch]
    bdAddrHandle *v30; // [esp+6Ch] [ebp-88h]
    bdReference<bdCommonAddr> *v31; // [esp+70h] [ebp-84h]
    bdCommonAddr *v32; // [esp+74h] [ebp-80h]
    bdCommonAddr *v33; // [esp+90h] [ebp-64h]
    bdCommonAddr *v34; // [esp+94h] [ebp-60h]
    bdReference<bdCommonAddr> *v35; // [esp+98h] [ebp-5Ch]
    bdCommonAddr *v36; // [esp+9Ch] [ebp-58h]
    bdCommonAddr *v37; // [esp+A0h] [ebp-54h]
    bdCommonAddr *m_ptr; // [esp+A4h] [ebp-50h]
    bdAddr v39; // [esp+A8h] [ebp-4Ch] BYREF
    bdAddr v40[3]; // [esp+B0h] [ebp-44h] BYREF
    bdCommonAddr *v41; // [esp+CCh] [ebp-28h]
    bdSocketRouter *socketRouter; // [esp+D0h] [ebp-24h]
    unsigned int ip32; // [esp+D4h] [ebp-20h]
    bool newHandle; // [esp+DBh] [ebp-19h] BYREF
    unsigned int index; // [esp+DCh] [ebp-18h] BYREF
    bdNetImpl *net; // [esp+E0h] [ebp-14h]
    bdReference<bdCommonAddr> commonAddr; // [esp+E4h] [ebp-10h] BYREF
    bdReference<bdAddrHandle> addrHandle; // [esp+E8h] [ebp-Ch] BYREF
    bdReference<bdCommonAddr> localCommonAddr; // [esp+ECh] [ebp-8h] BYREF
    bool ok; // [esp+F3h] [ebp-1h]

    Sys_EnterCriticalSection(CRITSECT_DEMONWARE);
    ok = 0;
    if ( adr )
    {
        dwCheckConnections();
        adr->type = NA_BOT;
        *(unsigned int *)adr->ip = 0;
        *(unsigned int *)&adr->port = 0;
        adr->addrHandleIndex = 0;
        adr->addrHandleIndex = -1;
        addrHandle.m_ptr = 0;
        dwGetLocalCommonAddr(&localCommonAddr);
        if ( localCommonAddr.m_ptr )
        {
            v41 = (bdCommonAddr *)bdMemory::allocate(0x28u);
            if ( v41 )
                v37 = bdCommonAddr::bdCommonAddr(v41);
            else
                v37 = 0;
            commonAddr.m_ptr = v37;
            if ( v37 )
            {
                m_ptr = commonAddr.m_ptr;
                InterlockedIncrement(&commonAddr.m_ptr->m_refCount);
                m_refCount = commonAddr.m_ptr->m_refCount;
            }
            p_addrHandle = commonAddrBuf;
            v35 = &v10;
            v10.m_ptr = localCommonAddr.m_ptr;
            if ( localCommonAddr.m_ptr )
            {
                v36 = v35->m_ptr;
                InterlockedIncrement(&v36->m_refCount);
                v21 = v36->m_refCount;
            }
            v34 = commonAddr.m_ptr;
            v20 = bdCommonAddr::deserialize((int)commonAddr.m_ptr, v10, (unsigned __int8 *)p_addrHandle);
            ok = v20;
            if ( v20 && (v33 = commonAddr.m_ptr, bdCommonAddr::isLoopback(commonAddr.m_ptr)) )
            {
                adr->ip[0] = 127;
                adr->ip[1] = 0;
                adr->ip[2] = 0;
                adr->ip[3] = 1;
                adr->type = NA_IP;
                adr->addrHandleIndex = dwGetLoopbackIndex();
                if ( adr->addrHandleIndex > maxAddrHandles
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwNet.cpp",
                                1207,
                                0,
                                "%s",
                                "adr->addrHandleIndex <= maxAddrHandles") )
                {
                    __debugbreak();
                }
                net = bdSingleton<bdNetImpl>::getInstance();
                if ( !net && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwNet.cpp", 1210, 0, "%s", "net") )
                    __debugbreak();
                if ( net )
                    adr->port = *(_WORD *)(bdCommonAddr::getLocalAddrs(net) + 2);
            }
            else if ( ok )
            {
                p_addrHandle = (bool *)&addrHandle;
                v10.m_ptr = secID;
                v31 = &v9;
                v9.m_ptr = commonAddr.m_ptr;
                if ( commonAddr.m_ptr )
                {
                    v32 = v31->m_ptr;
                    InterlockedIncrement(&v32->m_refCount);
                    v19 = v32->m_refCount;
                }
                AddressMap = (int *)dwGetAddressMap();
                if ( bdAddressMap::getAddrHandle(AddressMap, v9, v10, (bdReference<bdRemoteTask> *)p_addrHandle) )
                {
                    if ( addrHandle.m_ptr )
                    {
                        p_addrHandle = &newHandle;
                        v10.m_ptr = (bdCommonAddr *)&index;
                        v9.m_ptr = secID;
                        v29 = &v8;
                        v8.m_ptr = addrHandle.m_ptr;
                        v30 = addrHandle.m_ptr;
                        InterlockedIncrement(&addrHandle.m_ptr->m_refCount);
                        v17 = v30->m_refCount;
                        if ( dwRegisterAddrHandle(v8, (bdSecurityID *)v9.m_ptr, (unsigned int *)v10.m_ptr, p_addrHandle) )
                        {
                            socketRouter = dwGetSocketRouter();
                            if ( !socketRouter
                                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwNet.cpp", 1223, 0, "%s", "socketRouter") )
                            {
                                __debugbreak();
                            }
                            if ( socketRouter )
                            {
                                if ( newHandle )
                                    goto LABEL_35;
                                p_p_addrHandle = &p_addrHandle;
                                p_addrHandle = (bool *)addrHandle.m_ptr;
                                if ( addrHandle.m_ptr )
                                {
                                    v28 = *p_p_addrHandle;
                                    InterlockedIncrement((volatile int *)v28 + 1);
                                    v15 = *((unsigned int *)v28 + 1);
                                }
                                v14 = ((int (__thiscall *)(bdSocketRouter *, bool *))socketRouter->getStatus)(
                                                socketRouter,
                                                p_addrHandle);
                                if ( v14 == 3 )
LABEL_35:
                                    socketRouter->connect(socketRouter, &addrHandle);
                            }
                            ((void (__thiscall *)(bdSocketRouter *, bdAddrHandle *))socketRouter->pump)(socketRouter, v12);
                            v25 = &v12;
                            v12 = addrHandle.m_ptr;
                            if ( addrHandle.m_ptr )
                            {
                                v26 = *v25;
                                InterlockedIncrement(&v26->m_refCount);
                                v13 = v26->m_refCount;
                            }
                            v12 = (bdAddrHandle *)((int (__thiscall *)(bdSocketRouter *, bdAddrHandle *, volatile int, int, int))socketRouter->getStatus)(
                                                                            socketRouter,
                                                                            v12,
                                                                            v13,
                                                                            v14,
                                                                            v15);
                            if ( v12 != (bdAddrHandle *)2 )
                                Com_DPrintf(
                                    0,
                                    "We're doing commonaddrtonetadr before the socket is BD_SOCKET_CONNECTED. Bad and wrong.\n");
                            v24 = addrHandle.m_ptr;
                            RealAddr = bdAddrHandle::getRealAddr(addrHandle.m_ptr, v40);
                            Address = bdAddr::getAddress(RealAddr);
                            ip32 = (unsigned int)bdString::getBuffer((bdString *)Address);
                            bdInetAddr::~bdInetAddr(&v40[0].m_address);
                            *(unsigned int *)adr->ip = ip32;
                            v23 = addrHandle.m_ptr;
                            v6 = bdAddrHandle::getRealAddr(addrHandle.m_ptr, &v39);
                            adr->port = bdAddr::getPort(v6);
                            bdInetAddr::~bdInetAddr(&v39.m_address);
                            adr->type = NA_IP;
                            adr->addrHandleIndex = index;
                            if ( adr->addrHandleIndex > maxAddrHandles
                                && !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwNet.cpp",
                                            1238,
                                            0,
                                            "%s",
                                            "adr->addrHandleIndex <= maxAddrHandles") )
                            {
                                __debugbreak();
                            }
                            ok = 1;
                        }
                    }
                }
            }
            bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&commonAddr);
        }
        bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&localCommonAddr);
        bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>((bdReference<bdCommonAddr> *)&addrHandle);
    }
    Sys_LeaveCriticalSection(CRITSECT_DEMONWARE);
    return ok;
}

bdReference<bdAddrHandle> *__cdecl dwAddrHandleIndexToAddrHandle(
                bdReference<bdAddrHandle> *result,
                int addrHandleIndex)
{
    bdReference<bdAddrHandle> addrHandle; // [esp+24h] [ebp-4h] BYREF

    Sys_EnterCriticalSection(CRITSECT_DEMONWARE);
    addrHandle.m_ptr = 0;
    if ( (addrHandleIndex < 0 || addrHandleIndex > maxAddrHandles)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwNet.cpp",
                    1279,
                    0,
                    "%s",
                    "addrHandleIndex >= 0 && addrHandleIndex <= maxAddrHandles") )
    {
        __debugbreak();
    }
    if ( addrHandleIndex >= 0 && addrHandleIndex < maxAddrHandles )
        bdReference<bdCommonAddr>::operator=(
            (bdReference<bdCommonAddr> *)&addrHandle,
            (const bdReference<bdCommonAddr> *)&g_addrHandles[addrHandleIndex]);
    Sys_LeaveCriticalSection(CRITSECT_DEMONWARE);
    result->m_ptr = addrHandle.m_ptr;
    if ( result->m_ptr )
        InterlockedIncrement(&result->m_ptr->m_refCount);
    bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>((bdReference<bdCommonAddr> *)&addrHandle);
    return result;
}

bdReference<bdAddrHandle> *__cdecl dwNetadrToAddrHandle(bdReference<bdAddrHandle> *result, const netadr_t *const adr)
{
    bdReference<bdAddrHandle> addrHandle; // [esp+14h] [ebp-4h] BYREF

    if ( adr )
    {
        dwAddrHandleIndexToAddrHandle(result, adr->addrHandleIndex);
        return result;
    }
    else
    {
        addrHandle.m_ptr = 0;
        result->m_ptr = 0;
        if ( result->m_ptr )
            InterlockedIncrement(&result->m_ptr->m_refCount);
        bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>((bdReference<bdCommonAddr> *)&addrHandle);
        return result;
    }
}

unsigned int __cdecl dwGetNumAddrHandles()
{
    int i; // [esp+4h] [ebp-8h]
    unsigned int numConnections; // [esp+8h] [ebp-4h]

    numConnections = 0;
    for ( i = 0; i < maxAddrHandles; ++i )
    {
        if ( g_addrHandles[i].m_addrHandle.m_ptr )
            ++numConnections;
    }
    return numConnections;
}

bool __cdecl dwCloseConnection(netadr_t *const adr)
{
    char *v1; // eax
    char *v2; // eax
    bdReference<bdAddrHandle> v4[5]; // [esp-4h] [ebp-28h] BYREF
    bdReference<bdAddrHandle> *v5; // [esp+10h] [ebp-14h]
    bdAddrHandle *m_ptr; // [esp+14h] [ebp-10h]
    bdReference<bdAddrHandle> addrHandle; // [esp+1Ch] [ebp-8h] BYREF
    bool killed; // [esp+23h] [ebp-1h]

    Sys_EnterCriticalSection(CRITSECT_DEMONWARE);
    killed = 0;
    if ( adr )
    {
        v1 = NET_AdrToString(*adr);
        Com_DPrintf(23, "Attempting to close socketrouter connection to %s\n", v1);
        dwNetadrToAddrHandle(&addrHandle, adr);
        if ( addrHandle.m_ptr )
        {
            v2 = NET_AdrToString(*adr);
            Com_DPrintf(23, "Closed socketrouter connection to %s\n", v2);
            v5 = v4;
            v4[0] = addrHandle;
            if ( addrHandle.m_ptr )
            {
                m_ptr = v5->m_ptr;
                InterlockedIncrement(&m_ptr->m_refCount);
                v4[1] = (bdReference<bdAddrHandle>)m_ptr->m_refCount;
            }
            dwHandleLostConnection(v4[0]);
            killed = 1;
        }
        bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>((bdReference<bdCommonAddr> *)&addrHandle);
    }
    Sys_LeaveCriticalSection(CRITSECT_DEMONWARE);
    return killed;
}

int __cdecl dwGetLoopbackIndex()
{
    return maxAddrHandles;
}

bool __cdecl dwAddrIndexToAddrString(int addrHandleIndex, char *const str, unsigned int size)
{
    bdReference<bdAddrHandle> addrHandle; // [esp+18h] [ebp-8h] BYREF
    bool ok; // [esp+1Fh] [ebp-1h]

    Sys_EnterCriticalSection(CRITSECT_DEMONWARE);
    ok = 0;
    if ( addrHandleIndex > maxAddrHandles
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwNet.cpp",
                    1545,
                    0,
                    "%s",
                    "addrHandleIndex <= maxAddrHandles") )
    {
        __debugbreak();
    }
    memset(str, 0, size);
    if ( addrHandleIndex == dwGetLoopbackIndex() )
    {
        bdSnprintf(str, size, "loopback");
        ok = 1;
    }
    else if ( addrHandleIndex < maxAddrHandles )
    {
        addrHandle.m_ptr = g_addrHandles[addrHandleIndex].m_addrHandle.m_ptr;
        if ( addrHandle.m_ptr )
        {
            InterlockedIncrement(&addrHandle.m_ptr->m_refCount);
            dwGetAddressMap();
            bdAddressMap::addrToString(&addrHandle.m_ptr, str, size);
            ok = 1;
        }
        else
        {
            bdSnprintf(str, size, "unregistered address");
        }
        bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>((bdReference<bdCommonAddr> *)&addrHandle);
    }
    Sys_LeaveCriticalSection(CRITSECT_DEMONWARE);
    return ok;
}

int bdSnprintf(char *buf, unsigned int maxlen, const char *format, ...)
{
    int v4; // [esp+4h] [ebp-Ch]
    va_list va; // [esp+24h] [ebp+14h] BYREF

    va_start(va, format);
    v4 = _vscprintf(format, va);
    vsnprintf_s(buf, maxlen, 0xFFFFFFFF, format, va);
    return v4;
}

taskCompleteResults __cdecl dwGetAddrHandleConnectionTaskStatus(int addrHandleIndex)
{
    unsigned int v2[2]; // [esp-4h] [ebp-3Ch] BYREF
    bdDTLSAssociationStatus v3; // [esp+4h] [ebp-34h]
    volatile int m_refCount; // [esp+8h] [ebp-30h]
    unsigned int *v5; // [esp+18h] [ebp-20h]
    bdAddrHandle *m_ptr; // [esp+1Ch] [ebp-1Ch]
    taskCompleteResults v7; // [esp+20h] [ebp-18h]
    bdDTLSAssociationStatus status; // [esp+28h] [ebp-10h]
    taskCompleteResults connected; // [esp+2Ch] [ebp-Ch]
    bdReference<bdAddrHandle> addrHandle; // [esp+30h] [ebp-8h] BYREF
    bdSocketRouter *socketRouter; // [esp+34h] [ebp-4h]

    Sys_EnterCriticalSection(CRITSECT_DEMONWARE);
    connected = TASK_ERROR;
    socketRouter = dwGetSocketRouter();
    dwAddrHandleIndexToAddrHandle(&addrHandle, addrHandleIndex);
    if ( !socketRouter
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwNet.cpp", 1580, 0, "%s", "socketRouter") )
    {
        __debugbreak();
    }
    if ( socketRouter && addrHandle.m_ptr )
    {
        v5 = v2;
        v2[0] = addrHandle.m_ptr;
        m_ptr = addrHandle.m_ptr;
        InterlockedIncrement(&addrHandle.m_ptr->m_refCount);
        m_refCount = m_ptr->m_refCount;
        v3 = ((unsigned int (__thiscall *)(bdSocketRouter *, unsigned int))socketRouter->getStatus)(socketRouter, v2[0]);
        status = v3;
        v2[1] = v3;
        switch ( v3 )
        {
            case BD_SOCKET_IDLE:
                connected = TASK_ERROR;
                break;
            case BD_SOCKET_PENDING:
                connected = TASK_NOTCOMPLETE;
                break;
            case BD_SOCKET_CONNECTED:
                connected = TASK_COMPLETE;
                break;
            case BD_SOCKET_LOST:
                connected = TASK_ERROR;
                break;
            default:
                break;
        }
    }
    Sys_LeaveCriticalSection(CRITSECT_DEMONWARE);
    v7 = connected;
    bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>((bdReference<bdCommonAddr> *)&addrHandle);
    return v7;
}

void __thiscall bdArray<bdString>::clear(bdArray<bdString> *this)
{
    unsigned int m_size; // [esp+8h] [ebp-10h]
    bdString *m_data; // [esp+Ch] [ebp-Ch]
    unsigned int i; // [esp+14h] [ebp-4h]

    m_size = this->m_size;
    m_data = this->m_data;
    for ( i = 0; i < m_size; ++i )
        bdString::~bdString(&m_data[i]);
    bdMemory::deallocate(this->m_data);
    this->m_data = 0;
    this->m_size = 0;
    this->m_capacity = 0;
}

void __thiscall bdArray<bdString>::pushBack(bdArray<bdString> *this, const bdString *value)
{
    if ( this->m_size == this->m_capacity )
        bdArray<bdString>::increaseCapacity(this, 1u);
    if ( &this->m_data[this->m_size] )
        bdString::bdString(&this->m_data[this->m_size], value);
    ++this->m_size;
}

void __thiscall bdArray<bdInetAddr>::pushBack(bdArray<bdInetAddr> *this, const bdInetAddr *value)
{
    if ( this->m_size == this->m_capacity )
        bdArray<bdInetAddr>::increaseCapacity(this, 1u);
    if ( &this->m_data[this->m_size] )
        bdInetAddr::bdInetAddr(&this->m_data[this->m_size], value);
    ++this->m_size;
}

void __thiscall bdArray<bdString>::increaseCapacity(bdArray<bdString> *this, unsigned int increase)
{
    unsigned int m_capacity; // [esp+4h] [ebp-3Ch]
    unsigned int v4; // [esp+10h] [ebp-30h]
    bdString *v5; // [esp+14h] [ebp-2Ch]
    unsigned int j; // [esp+1Ch] [ebp-24h]
    unsigned int m_size; // [esp+20h] [ebp-20h]
    bdString *m_data; // [esp+24h] [ebp-1Ch]
    bdString *v9; // [esp+28h] [ebp-18h]
    unsigned int i; // [esp+34h] [ebp-Ch]
    bdString *newData; // [esp+38h] [ebp-8h]
    unsigned int newCapacity; // [esp+3Ch] [ebp-4h]

    if ( increase <= this->m_capacity )
        m_capacity = this->m_capacity;
    else
        m_capacity = increase;
    newCapacity = m_capacity + this->m_capacity;
    newData = 0;
    if ( newCapacity )
    {
        newData = (bdString *)bdMemory::allocate(4 * newCapacity);
        m_size = this->m_size;
        m_data = this->m_data;
        for ( i = 0; i < m_size; ++i )
        {
            v9 = &newData[i];
            if ( v9 )
                bdString::bdString(v9, &m_data[i]);
        }
    }
    v4 = this->m_size;
    v5 = this->m_data;
    for ( j = 0; j < v4; ++j )
        bdString::~bdString(&v5[j]);
    bdMemory::deallocate(this->m_data);
    this->m_data = newData;
    this->m_capacity = newCapacity;
}

void __thiscall bdArray<bdInetAddr>::increaseCapacity(bdArray<bdInetAddr> *this, unsigned int increase)
{
    unsigned int m_capacity; // [esp+4h] [ebp-3Ch]
    unsigned int v4; // [esp+10h] [ebp-30h]
    bdInetAddr *v5; // [esp+14h] [ebp-2Ch]
    unsigned int j; // [esp+1Ch] [ebp-24h]
    unsigned int m_size; // [esp+20h] [ebp-20h]
    bdInetAddr *m_data; // [esp+24h] [ebp-1Ch]
    bdInetAddr *v9; // [esp+28h] [ebp-18h]
    unsigned int i; // [esp+34h] [ebp-Ch]
    bdInetAddr *newData; // [esp+38h] [ebp-8h]
    unsigned int newCapacity; // [esp+3Ch] [ebp-4h]

    if ( increase <= this->m_capacity )
        m_capacity = this->m_capacity;
    else
        m_capacity = increase;
    newCapacity = m_capacity + this->m_capacity;
    newData = 0;
    if ( newCapacity )
    {
        newData = (bdInetAddr *)bdMemory::allocate(4 * newCapacity);
        m_size = this->m_size;
        m_data = this->m_data;
        for ( i = 0; i < m_size; ++i )
        {
            v9 = &newData[i];
            if ( v9 )
                bdInetAddr::bdInetAddr(v9, &m_data[i]);
        }
    }
    v4 = this->m_size;
    v5 = this->m_data;
    for ( j = 0; j < v4; ++j )
        bdInetAddr::~bdInetAddr(&v5[j]);
    bdMemory::deallocate(this->m_data);
    this->m_data = newData;
    this->m_capacity = newCapacity;
}

void __thiscall bdArray<bdInetAddr>::clear(bdArray<bdInetAddr> *this)
{
    unsigned int m_size; // [esp+8h] [ebp-10h]
    bdInetAddr *m_data; // [esp+Ch] [ebp-Ch]
    unsigned int i; // [esp+14h] [ebp-4h]

    m_size = this->m_size;
    m_data = this->m_data;
    for ( i = 0; i < m_size; ++i )
        bdInetAddr::~bdInetAddr(&m_data[i]);
    bdMemory::deallocate(this->m_data);
    this->m_data = 0;
    this->m_size = 0;
    this->m_capacity = 0;
}

bool __thiscall dwRawInterceptor::acceptPacket(
                dwRawInterceptor *this,
                bdSocket *socket,
                bdAddr addr,
                char *data,
                unsigned int size,
                unsigned __int8 type)
{
    bdAddr *Address; // eax
    bdSecurityID *SessionSecID; // eax
    bdAddr *v9; // eax
    unsigned intv10; // eax
    bdSecurityID *v11; // eax
    bdAddr *v12; // eax
    char *v13; // eax
    bdSecurityID *v14; // eax
    bdAddr *v15; // eax
    char *v16; // eax
    bdSecurityID *v17; // eax
    bdAddr *v18; // eax
    _BYTE v19[24]; // [esp-18h] [ebp-2F0h] BYREF
    const char *v20; // [esp+0h] [ebp-2D8h]
    const char *v21; // [esp+8h] [ebp-2D0h]
    const char *string; // [esp+Ch] [ebp-2CCh]
    int v23; // [esp+10h] [ebp-2C8h]
    char *v24; // [esp+18h] [ebp-2C0h]
    char *v25; // [esp+1Ch] [ebp-2BCh]
    int v26; // [esp+20h] [ebp-2B8h]
    char *v27; // [esp+28h] [ebp-2B0h]
    char *v28; // [esp+2Ch] [ebp-2ACh]
    unsigned int count; // [esp+30h] [ebp-2A8h]
    char *v30; // [esp+38h] [ebp-2A0h]
    char *v31; // [esp+3Ch] [ebp-29Ch]
    int v32; // [esp+40h] [ebp-298h]
    dwRawInterceptor *thisa; // [esp+44h] [ebp-294h]
    char *v34; // [esp+48h] [ebp-290h]
    char *v35; // [esp+4Ch] [ebp-28Ch]
    char *v36; // [esp+50h] [ebp-288h]
    char *v37; // [esp+54h] [ebp-284h]
    char *v38; // [esp+58h] [ebp-280h]
    char *v39; // [esp+5Ch] [ebp-27Ch]
    char *v40; // [esp+60h] [ebp-278h]
    char *v41; // [esp+64h] [ebp-274h]
    bool v42; // [esp+8Fh] [ebp-249h]
    msg_t v43; // [esp+A0h] [ebp-238h] BYREF
    char v44[24]; // [esp+D0h] [ebp-208h] BYREF
    bdSecurityID v45; // [esp+E8h] [ebp-1F0h] BYREF
    unsigned __int8 *v46; // [esp+F0h] [ebp-1E8h]
    int v47; // [esp+F4h] [ebp-1E4h]
    int v48; // [esp+F8h] [ebp-1E0h]
    const char *v49; // [esp+FCh] [ebp-1DCh]
    int v50; // [esp+100h] [ebp-1D8h]
    int v51; // [esp+104h] [ebp-1D4h]
    bdSecurityID v52; // [esp+108h] [ebp-1D0h] BYREF
    const char *v53; // [esp+110h] [ebp-1C8h]
    msg_t v54; // [esp+114h] [ebp-1C4h] BYREF
    char v55[24]; // [esp+144h] [ebp-194h] BYREF
    bdSecurityID v56; // [esp+15Ch] [ebp-17Ch] BYREF
    unsigned __int8 *v57; // [esp+164h] [ebp-174h]
    int v58; // [esp+168h] [ebp-170h]
    int v59; // [esp+16Ch] [ebp-16Ch]
    const char *v60; // [esp+170h] [ebp-168h]
    int v61; // [esp+174h] [ebp-164h]
    int v62; // [esp+178h] [ebp-160h]
    bdSecurityID v63; // [esp+17Ch] [ebp-15Ch] BYREF
    const char *v64; // [esp+184h] [ebp-154h]
    msg_t buf; // [esp+188h] [ebp-150h] BYREF
    char v66[24]; // [esp+1B8h] [ebp-120h] BYREF
    bdSecurityID v67; // [esp+1D0h] [ebp-108h] BYREF
    unsigned __int8 *v68; // [esp+1D8h] [ebp-100h]
    int length; // [esp+1DCh] [ebp-FCh]
    int v70; // [esp+1E0h] [ebp-F8h]
    const char *v71; // [esp+1E4h] [ebp-F4h]
    int v72; // [esp+1E8h] [ebp-F0h]
    int v73; // [esp+1ECh] [ebp-ECh]
    bdSecurityID v74; // [esp+1F0h] [ebp-E8h] BYREF
    const char *v75; // [esp+1F8h] [ebp-E0h]
    msg_t serverinfo; // [esp+1FCh] [ebp-DCh] BYREF
    char dst[24]; // [esp+22Ch] [ebp-ACh] BYREF
    bdSecurityID secID; // [esp+244h] [ebp-94h] BYREF
    unsigned __int8 *serverinfo_p; // [esp+24Ch] [ebp-8Ch]
    int serverinfo_size; // [esp+250h] [ebp-88h]
    int v81; // [esp+254h] [ebp-84h]
    const char *v82; // [esp+258h] [ebp-80h]
    int v83; // [esp+25Ch] [ebp-7Ch]
    int v84; // [esp+260h] [ebp-78h]
    bdSecurityID remoteID; // [esp+264h] [ebp-74h] BYREF
    const char *Buffer; // [esp+26Ch] [ebp-6Ch]
    char printcmd[8]; // [esp+270h] [ebp-68h] BYREF
    char *buffptr; // [esp+278h] [ebp-60h]
    netadr_t from; // [esp+27Ch] [ebp-5Ch] BYREF
    char rconcmd[8]; // [esp+28Ch] [ebp-4Ch] BYREF
    int messagelen; // [esp+298h] [ebp-40h]
    msg_t msg; // [esp+29Ch] [ebp-3Ch] BYREF
    unsigned int ip32; // [esp+2CCh] [ebp-Ch]
    char *pkt; // [esp+2D0h] [ebp-8h]
    bool retval; // [esp+2D7h] [ebp-1h]

    thisa = this;
    retval = 0;
    if ( type == 255 )
    {
        pkt = data;
        retval = 1;
        if ( size < 0xD )
        {
            Com_DPrintf(0, "Disregarding runt packet\n");
            bdInetAddr::~bdInetAddr(&addr.m_address);
            return retval;
        }
        if ( size >= 0x4F0 )
        {
            Com_DPrintf(0, "Disregarding jumbo packet. Hax?\n");
            bdInetAddr::~bdInetAddr(&addr.m_address);
            return retval;
        }
        v32 = pkt[4];
        switch ( v32 )
        {
            case 0:
                if ( Sys_IsServerThread() && com_sv_running->current.enabled )
                {
                    Address = bdAddr::getAddress(&addr);
                    ip32 = (unsigned int)bdString::getBuffer((bdString *)Address);
                    *(unsigned int *)from.ip = ip32;
                    from.port = bdAddr::getPort(&addr);
                    from.type = NA_IP;
                    strcpy(rconcmd, "rcon ");
                    messagelen = size - 5;
                    memset(s_rconbuffer, 0, sizeof(s_rconbuffer));
                    *(unsigned int *)s_rconbuffer = *(unsigned int *)pkt;
                    v30 = &rconcmd[1];
                    v31 = &rconcmd[strlen(rconcmd) + 1];
                    count = v31 - &rconcmd[1];
                    memcpy(&s_rconbuffer[4], (unsigned __int8 *)rconcmd, v31 - &rconcmd[1]);
                    v27 = &rconcmd[1];
                    v28 = &rconcmd[strlen(rconcmd) + 1];
                    v26 = v28 - &rconcmd[1];
                    memcpy((unsigned __int8 *)(v28 - &rconcmd[1] + 53913764), (unsigned __int8 *)pkt + 5, messagelen);
                    MSG_InitReadOnly(&msg, s_rconbuffer, messagelen + 9);
                    NET_DeferPacketToClient(&from, &msg);
                }
                break;
            case 1:
                bdAddr::bdAddr((bdAddr *)&v19[16], &addr);
                if ( checkRconAddr(*(bdAddr *)&v19[16]) )
                {
                    strcpy(printcmd, "print ");
                    v25 = printcmd;
                    v24 = &printcmd[1];
                    v25 += strlen(v25);
                    v23 = ++v25 - &printcmd[1];
                    buffptr = &data[v25 - &printcmd[1] + 5];
                    Com_Printf(0, "%s", buffptr);
                }
                break;
            case 2:
                if ( !com_sv_running->current.enabled || xblive_basictraining->current.enabled )
                    break;
                bdSecurityID::bdSecurityID(&remoteID);
                v41 = data + 5;
                remoteID = *(bdSecurityID *)(data + 5);
                Com_DPrintf(15, "GOT RAWPING PACKET!\n");
                SessionSecID = (bdSecurityID *)dwGetSessionSecID();
                if ( bdSecurityID::operator!=(SessionSecID, &remoteID) )
                {
                    Com_DPrintf(15, "Rawping: Got secid that doesn't match, dropping\n");
                    bdInetAddr::~bdInetAddr(&addr.m_address);
                    return 1;
                }
                v9 = bdAddr::getAddress(&addr);
                Buffer = bdString::getBuffer((bdString *)v9);
                v82 = Buffer;
                LOWORD(v83) = bdAddr::getPort(&addr);
                *(unsigned int *)v19 = v81;
                *(unsigned int *)&v19[4] = v82;
                *(unsigned int *)&v19[8] = v83;
                *(unsigned int *)&v19[12] = v84;
                SVC_Info(*(netadr_t *)v19, &remoteID, 0);
                break;
            case 3:
                if ( com_sv_running->current.enabled )
                {
                    bdAddr::toString(&addr, dst, 0x16u);
                    Com_PrintWarning(15, "Got ping ack when I'm a server -    packet from %s\n", dst);
                    bdInetAddr::~bdInetAddr(&addr.m_address);
                    return 1;
                }
                Com_DPrintf(15, "GOT RAWPING RESPONSE!\n");
                bdSecurityID::bdSecurityID(&secID);
                v40 = data + 5;
                secID = *(bdSecurityID *)(data + 5);
                serverinfo_p = (unsigned __int8 *)(pkt + 13);
                serverinfo_size = size + 5;
                if ( (int)(size + 5) > 0 && serverinfo_size < 1024 )
                {
                    MSG_InitReadOnly(&serverinfo, serverinfo_p, serverinfo_size);
                    MSG_BeginReading(&serverinfo);
                    v10 = Sys_Milliseconds();
                    CL_ServerInfoPacket(&secID, &serverinfo, v10);
                    UI_RefreshServers();
                }
                break;
            case 4:
                if ( !com_sv_running->current.enabled )
                    break;
                bdSecurityID::bdSecurityID(&v74);
                v39 = data + 5;
                v74 = *(bdSecurityID *)(data + 5);
                Com_DPrintf(15, "GOT RAWSTATUS PACKET!\n");
                v11 = (bdSecurityID *)dwGetSessionSecID();
                if ( bdSecurityID::operator!=(v11, &v74) )
                {
                    Com_DPrintf(15, "Rawping: Got secid that doesn't match, dropping\n");
                    bdInetAddr::~bdInetAddr(&addr.m_address);
                    return 1;
                }
                v12 = bdAddr::getAddress(&addr);
                v75 = bdString::getBuffer((bdString *)v12);
                v71 = v75;
                LOWORD(v72) = bdAddr::getPort(&addr);
                *(unsigned int *)&v19[8] = v70;
                *(unsigned int *)&v19[12] = v71;
                *(unsigned int *)&v19[16] = v72;
                *(unsigned int *)&v19[20] = v73;
                v13 = NET_AdrToString(*(netadr_t *)&v19[8]);
                Com_DPrintf(15, "Got status request from %s\n", v13);
                *(unsigned int *)&v19[4] = v70;
                *(unsigned int *)&v19[8] = v71;
                *(unsigned int *)&v19[12] = v72;
                *(unsigned int *)&v19[16] = v73;
                SVC_Status(*(netadr_t *)&v19[4], &v74);
                break;
            case 5:
                if ( com_sv_running->current.enabled )
                {
                    bdAddr::toString(&addr, v66, 0x16u);
                    Com_PrintWarning(15, "Got ping ack when I'm a server -    packet from %s\n", v66);
                    bdInetAddr::~bdInetAddr(&addr.m_address);
                    return 1;
                }
                bdSecurityID::bdSecurityID(&v67);
                v38 = data + 5;
                v67 = *(bdSecurityID *)(data + 5);
                v68 = (unsigned __int8 *)(pkt + 13);
                length = size + 5;
                if ( (int)(size + 5) > 0 && length < 1024 )
                {
                    MSG_InitReadOnly(&buf, v68, length);
                    MSG_BeginReading(&buf);
                    CL_ServerStatusResponse(&buf, &v67);
                }
                break;
            case 6:
                if ( !com_sv_running->current.enabled )
                    break;
                bdSecurityID::bdSecurityID(&v63);
                v37 = data + 5;
                v63 = *(bdSecurityID *)(data + 5);
                Com_DPrintf(15, "GOT RAWSCORE PACKET!\n");
                v14 = (bdSecurityID *)dwGetSessionSecID();
                if ( bdSecurityID::operator!=(v14, &v63) )
                {
                    Com_DPrintf(15, "Rawping: Got secid that doesn't match, dropping\n");
                    bdInetAddr::~bdInetAddr(&addr.m_address);
                    return 1;
                }
                v15 = bdAddr::getAddress(&addr);
                v64 = bdString::getBuffer((bdString *)v15);
                v60 = v64;
                LOWORD(v61) = bdAddr::getPort(&addr);
                *(unsigned int *)&v19[8] = v59;
                *(unsigned int *)&v19[12] = v60;
                *(unsigned int *)&v19[16] = v61;
                *(unsigned int *)&v19[20] = v62;
                v16 = NET_AdrToString(*(netadr_t *)&v19[8]);
                Com_DPrintf(15, "Got status scoreboard request from %s\n", v16);
                *(unsigned int *)&v19[4] = v59;
                *(unsigned int *)&v19[8] = v60;
                *(unsigned int *)&v19[12] = v61;
                *(unsigned int *)&v19[16] = v62;
                SVC_StatusScoreBoard(*(netadr_t *)&v19[4], &v63);
                break;
            case 7:
                if ( com_sv_running->current.enabled )
                {
                    bdAddr::toString(&addr, v55, 0x16u);
                    Com_PrintWarning(15, "Got ping ack when I'm a server -    packet from %s\n", v55);
                    bdInetAddr::~bdInetAddr(&addr.m_address);
                    return 1;
                }
                bdSecurityID::bdSecurityID(&v56);
                v36 = data + 5;
                v56 = *(bdSecurityID *)(data + 5);
                v57 = (unsigned __int8 *)(pkt + 13);
                v58 = size + 5;
                if ( (int)(size + 5) > 0 && v58 < 0x4000 )
                {
                    MSG_InitReadOnly(&v54, v57, v58);
                    MSG_BeginReading(&v54);
                    CL_ServerStatusScoreBoardResponse(&v54, &v56);
                }
                break;
            case 8:
                if ( !com_sv_running->current.enabled )
                    break;
                string = g_password->current.string;
                v21 = string + 1;
                string += strlen(string) + 1;
                v20 = (const char *)(string - v21);
                if ( string != v21 )
                    break;
                bdSecurityID::bdSecurityID(&v52);
                v35 = data + 5;
                v52 = *(bdSecurityID *)(data + 5);
                Com_DPrintf(15, "Got quickmatch ping!\n");
                v17 = (bdSecurityID *)dwGetSessionSecID();
                if ( bdSecurityID::operator!=(v17, &v52) )
                {
                    Com_DPrintf(15, "Rawping: Got secid that doesn't match, dropping\n");
                    bdInetAddr::~bdInetAddr(&addr.m_address);
                    return 1;
                }
                v18 = bdAddr::getAddress(&addr);
                v53 = bdString::getBuffer((bdString *)v18);
                v49 = v53;
                LOWORD(v50) = bdAddr::getPort(&addr);
                *(unsigned int *)v19 = v48;
                *(unsigned int *)&v19[4] = v49;
                *(unsigned int *)&v19[8] = v50;
                *(unsigned int *)&v19[12] = v51;
                SVC_Info(*(netadr_t *)v19, &v52, 1);
                break;
            case 9:
                if ( com_sv_running->current.enabled )
                {
                    bdAddr::toString(&addr, v44, 0x16u);
                    Com_PrintWarning(15, "Got ping ack when I'm a server -    packet from %s\n", v44);
                    bdInetAddr::~bdInetAddr(&addr.m_address);
                    return 1;
                }
                Com_DPrintf(15, "Got quickmatch response!\n");
                bdSecurityID::bdSecurityID(&v45);
                v34 = data + 5;
                v45 = *(bdSecurityID *)(data + 5);
                v46 = (unsigned __int8 *)(pkt + 13);
                v47 = size + 5;
                if ( (int)(size + 5) > 0 && v47 < 1024 )
                {
                    MSG_InitReadOnly(&v43, v46, v47);
                    MSG_BeginReading(&v43);
                    CL_QuickMatch_PingResponse(&v45, &v43);
                }
                break;
            default:
                Com_Printf(15, "Got unknown raw packet: %u\n", type);
                break;
        }
    }
    v42 = retval;
    bdInetAddr::~bdInetAddr(&addr.m_address);
    return v42;
}

char __cdecl checkRconAddr(bdAddr fromAddr)
{
    bdAddr v2; // [esp+4h] [ebp-20h] BYREF
    clientConnection_t *clc; // [esp+Ch] [ebp-18h]
    const char *serveradrstring; // [esp+10h] [ebp-14h]
    bdAddr serverbdAddr; // [esp+14h] [ebp-10h] BYREF
    netadr_t *serveraddr; // [esp+1Ch] [ebp-8h]
    const char *adrstring; // [esp+20h] [ebp-4h]

    if ( CL_GetLocalClientConnectionState(0) == 10 )
    {
        clc = CL_GetLocalClientConnection(0);
        serveradrstring = NET_AdrToString(clc->netchan.remoteAddress);
        bdAddr::bdAddr(&v2, (char *)serveradrstring);
        if ( bdAddr::operator==((int)&v2, (int)&fromAddr) )
        {
            bdInetAddr::~bdInetAddr(&v2.m_address);
            bdInetAddr::~bdInetAddr(&fromAddr.m_address);
            return 1;
        }
        bdInetAddr::~bdInetAddr(&v2.m_address);
    }
    serveraddr = CL_GetLastRconAddress();
    adrstring = NET_AdrToString(*serveraddr);
    bdAddr::bdAddr(&serverbdAddr, (char *)adrstring);
    if ( bdAddr::operator==((int)&serverbdAddr, (int)&fromAddr) )
    {
        bdInetAddr::~bdInetAddr(&serverbdAddr.m_address);
        bdInetAddr::~bdInetAddr(&fromAddr.m_address);
        return 1;
    }
    else
    {
        bdInetAddr::~bdInetAddr(&serverbdAddr.m_address);
        bdInetAddr::~bdInetAddr(&fromAddr.m_address);
        return 0;
    }
}

SecID *__thiscall SecID::SecID(SecID *this)
{
    bdSecurityID::bdSecurityID(&this->m_secID);
    this->m_active = 0;
    return this;
}

#endif