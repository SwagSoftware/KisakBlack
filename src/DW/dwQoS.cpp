#ifdef KISAK_DW

#include "dwQoS.h"

dwQoSMultiProbeListener::dwQoSMultiProbeListener()
    : bdQoSProbeListener()
    , m_probeAddrs{}
    , m_probeSucceeded{}
    , m_numProbes(0)
    , m_successes{}
    , m_successIDs{}
    , m_numSuccesses(0)
    , m_numFailures(0)
{
    // Matches final call in decompiled constructor
    clean();
}



//dwQoSMultiProbeListener::~dwQoSMultiProbeListener()
//{
//    int v2; // [esp+4h] [ebp-28h]
//    bdReference<bdCommonAddr> *j; // [esp+8h] [ebp-24h]
//    int v4; // [esp+18h] [ebp-14h]
//    bdReference<bdCommonAddr> *i; // [esp+1Ch] [ebp-10h]
//
//    v4 = 500;
//    for (i = (bdReference<bdCommonAddr> *)this->m_successIDs;
//        --v4 >= 0;
//        bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(i))
//    {
//        i -= 9;
//        bdInetAddr::~bdInetAddr((bdInetAddr *)&i[1]);
//    }
//    v2 = 500;
//    for (j = (bdReference<bdCommonAddr> *)this->m_probeSucceeded;
//        --v2 >= 0;
//        bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(j))
//    {
//        j -= 7;
//    }
//    bdQoSProbeListener::~bdQoSProbeListener(this);
//}

void __thiscall dwQoSMultiProbeListener::addProbe(bdQoSRemoteAddr addr)
{
#ifdef KISAK_DEMON
    int v2; // ecx
    bdQoSRemoteAddr *v4; // [esp+10h] [ebp-18h]

    if ( this->m_numProbes >= 0x1F4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwQoS.cpp",
                    33,
                    0,
                    "%s",
                    "m_numProbes < MAX_SEARCHRESULTS") )
    {
        __debugbreak();
    }
    if ( this->m_numProbes < 0x1F4 )
    {
        v4 = &this->m_probeAddrs[this->m_numProbes];
        bdReference<bdCommonAddr>::operator=(&v4->m_addr, &addr.m_addr);
        v2 = *(unsigned int *)&addr.m_id.ab[4];
        *(unsigned int *)v4->m_id.ab = *(unsigned int *)addr.m_id.ab;
        *(unsigned int *)&v4->m_id.ab[4] = v2;
        v4->m_key = addr.m_key;
        this->m_probeSucceeded[this->m_numProbes++] = 0;
    }
    bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&addr.m_addr);
#endif
}

void __thiscall dwQoSMultiProbeListener::onQoSProbeSuccess(const bdQoSProbeInfo *info)
{
#ifdef KISAK_DEMON
    int v2; // ecx
    unsigned int m_numSuccesses; // edx
    unsigned int i; // [esp+28h] [ebp-8h]
    bool found; // [esp+2Fh] [ebp-1h]

    if ( this->m_numSuccesses >= 0x1F4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwQoS.cpp",
                    68,
                    0,
                    "%s",
                    "m_numSuccesses < MAX_SEARCHRESULTS") )
    {
        __debugbreak();
    }
    found = 0;
    for ( i = 0; !found && i < 0x1F4; ++i )
    {
        if ( this->m_probeAddrs[i].m_addr.m_ptr )
        {
            found = bdCommonAddr::operator==((int)this->m_probeAddrs[i].m_addr.m_ptr, (int)info->m_addr.m_ptr);
            if ( found && !this->m_probeSucceeded[i] )
            {
                this->m_probeSucceeded[i] = 1;
                bdQoSProbeInfo::operator=(&this->m_successes[this->m_numSuccesses], info);
                *(unsigned int *)&qos_data[i][0] = *(unsigned int *)info->m_data;
                this->m_successes[this->m_numSuccesses].m_data = (unsigned __int8 *)(4 * i + 53925536); // big num!
                v2 = *(unsigned int *)&this->m_probeAddrs[i].m_id.ab[4];
                m_numSuccesses = this->m_numSuccesses;
                *(unsigned int *)this->m_successIDs[m_numSuccesses].ab = *(unsigned int *)this->m_probeAddrs[i].m_id.ab;
                *(unsigned int *)&this->m_successIDs[m_numSuccesses].ab[4] = v2;
                ++this->m_numSuccesses;
                CL_SetPingFromQOS((int)(float)(info->m_latency * 1000.0), &this->m_probeAddrs[i].m_id);
                UI_RefreshServers();
            }
        }
    }
#endif
}

void __thiscall dwQoSMultiProbeListener::onQoSProbeFail(bdReference<bdCommonAddr> addr)
{
#ifdef KISAK_DEMON
    if ( this->m_numSuccesses >= 0x1F4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\Dw\\dwQoS.cpp",
                    106,
                    0,
                    "%s",
                    "m_numSuccesses < MAX_SEARCHRESULTS") )
    {
        __debugbreak();
    }
    ++this->m_numFailures;
    bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&addr);
#endif
}

void __thiscall dwQoSMultiProbeListener::clean()
{
#ifdef KISAK_DEMON
    unsigned int i; // [esp+24h] [ebp-4h]

    this->m_numProbes = 0;
    this->m_numSuccesses = 0;
    this->m_numFailures = 0;
    for ( i = 0; i < 0x1F4; ++i )
    {
        bdReference<bdCommonAddr>::operator=((bdReference<bdRemoteTask> *)&this->m_probeAddrs[i], 0);
        bdReference<bdCommonAddr>::operator=((bdReference<bdRemoteTask> *)&this->m_successes[i], 0);
    }
    memset(this->m_probeAddrs, 0, sizeof(this->m_probeAddrs));
    memset(this->m_successes, 0, sizeof(this->m_successes));
    memset(this->m_successIDs[0].ab, 0, 0xFA0u);
    memset(qos_data[0], 0, sizeof(qos_data));
#endif
}

int __thiscall dwQoSMultiProbeListener::complete()
{
    taskCompleteResults complete; // [esp+4h] [ebp-4h]

    complete = TASK_ERROR;
    if ( this->m_numFailures + this->m_numSuccesses < this->m_numProbes )
        return 0;
    if ( this->m_numFailures + this->m_numSuccesses == this->m_numProbes )
        return 1;
    return complete;
}

//bdQoSRemoteAddr *__thiscall bdQoSRemoteAddr::bdQoSRemoteAddr(bdQoSRemoteAddr *this, const bdQoSRemoteAddr *__that)
//{
//    this->m_addr.m_ptr = __that->m_addr.m_ptr;
//    if ( this->m_addr.m_ptr )
//        InterlockedIncrement(&this->m_addr.m_ptr->m_refCount);
//    bdSecurityID::bdSecurityID(&this->m_id, &__that->m_id);
//    bdSecurityKey::bdSecurityKey(&this->m_key, &__that->m_key);
//    return this;
//}

void __cdecl dwStartQoSProbes(dwQoSMultiProbeListener *listener, unsigned int numProbes, bdQoSRemoteAddr *serverAddrs)
{
#ifdef KISAK_DEMON
    bdQoSRemoteAddr v3; // [esp-1Ch] [ebp-3Ch] BYREF
    unsigned int i; // [esp+14h] [ebp-Ch]
    bdReference<bdCommonAddr> localAddr; // [esp+18h] [ebp-8h] BYREF
    bdQoSProbe *qosProbe; // [esp+1Ch] [ebp-4h]

    qosProbe = dwGetQoSProbe();
    dwGetLocalCommonAddr(&localAddr);
    if ( qosProbe && localAddr.m_ptr && listener )
    {
        dwQoSMultiProbeListener::clean(listener);
        for ( i = 0; i < numProbes; ++i )
        {
            if ( bdQoSProbe::probe(qosProbe, &serverAddrs[i], listener) )
            {
                bdQoSRemoteAddr::bdQoSRemoteAddr(&v3, &serverAddrs[i]);
                dwQoSMultiProbeListener::addProbe(listener, v3);
            }
            else
            {
                Com_Printf(16, "Failed to start QoS probe.\n");
            }
        }
    }
    bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>(&localAddr);
#endif
}

void __cdecl dwClearQoSProbes()
{
#ifdef KISAK_DEMON
    bdQoSProbe *qosProbe; // [esp+0h] [ebp-4h]

    if ( g_dwNetStatus == DW_NET_STARTED_ONLINE )
    {
        qosProbe = dwGetQoSProbe();
        if ( qosProbe )
            bdQoSProbe::cancelProbes(qosProbe);
    }
#endif
}

//void __thiscall bdReference<bdCommonAddr>::operator=(bdReference<bdRemoteTask> *this, bdRemoteTask *p)
//{
//    bdRemoteTask *m_ptr; // [esp+10h] [ebp-Ch]
//
//    if ( this->m_ptr )
//    {
//        m_ptr = this->m_ptr;
//        InterlockedDecrement(&this->m_ptr->m_refCount);
//        if ( !m_ptr->m_refCount )
//        {
//            if ( this->m_ptr )
//                ((void (__thiscall *)(bdRemoteTask *, int))this->m_ptr->~bdRemoteTask)(this->m_ptr, 1);
//        }
//    }
//    this->m_ptr = p;
//    if ( this->m_ptr )
//        InterlockedIncrement(&this->m_ptr->m_refCount);
//}
//

#endif