#pragma once

#ifdef KISAK_DW

#include <DemonWare/bdCore/bdReference/bdReferencable.h>
#include <DemonWare/bdCore/bdSocket/bdAddr.h>
#include <client_mp/cl_main_mp.h>

struct bdQoSRemoteAddr // sizeof=0x1C
{                                       // XREF: dwQoSMultiProbeListener/r
                                        // ?addProbe@dwQoSMultiProbeListener@@QAEXVbdQoSRemoteAddr@@@Z/r ...
    bdReference<bdCommonAddr> m_addr;
    bdSecurityID m_id;                  // XREF: dwQoSMultiProbeListener::addProbe(bdQoSRemoteAddr)+6C/r
                                        // dwQoSMultiProbeListener::addProbe(bdQoSRemoteAddr)+6F/r
    bdSecurityKey m_key;                // XREF: dwQoSMultiProbeListener::addProbe(bdQoSRemoteAddr)+7B/o

    //bdQoSRemoteAddr(const bdQoSRemoteAddr *__that);
    bdQoSRemoteAddr(const bdQoSRemoteAddr &that) //aislop
        : m_addr() // don't auto-copy — we replicate binary behavior
        , m_id(that.m_id)
        , m_key(that.m_key)
    {
        // Manual ref pointer copy (matches decomp exactly)
        m_addr.m_ptr = that.m_addr.m_ptr;

        if (m_addr.m_ptr)
        {
            InterlockedIncrement(
                &m_addr.m_ptr->m_refCount
            );
        }
    }

    // Default constructor
    bdQoSRemoteAddr()
        : m_addr()   // ensures m_ptr is nullptr
        , m_id()     // default ctor
        , m_key()    // default ctor
    {
        // Explicitly match decompiled behavior
        m_addr.m_ptr = nullptr;
    }
};

struct bdQoSProbeListener // sizeof=0x4
{                                       // XREF: dwQoSMultiProbeListener/r
    //bdQoSProbeListener_vtbl *__vftable;

    virtual ~bdQoSProbeListener() = default;
};

struct bdQoSProbeInfo // sizeof=0x24
{                                       // XREF: dwQoSMultiProbeListener/r
    bdReference<bdCommonAddr> m_addr;
    bdAddr m_realAddr;
    float m_latency;
    unsigned __int8 *m_data;
    unsigned int m_dataSize;
    bool m_disabled;
    // padding byte
    // padding byte
    // padding byte
    unsigned int m_bandwidthDown;
    unsigned int m_bandwidthUp;

    //bdQoSProbeInfo *__thiscall bdQoSProbeInfo::operator=(bdQoSProbeInfo *this, const bdQoSProbeInfo *__that);
    bdQoSProbeInfo &operator=(const bdQoSProbeInfo &that) // aislopped
    {
        if (this == &that)
            return *this;

        // reference-counted assignment
        m_addr = that.m_addr;

        // bdAddr full copy
        m_realAddr = that.m_realAddr;

        m_latency = that.m_latency;

        // shallow copy (important — matches binary)
        m_data = that.m_data;
        m_dataSize = that.m_dataSize;

        m_disabled = that.m_disabled;
        m_bandwidthDown = that.m_bandwidthDown;
        m_bandwidthUp = that.m_bandwidthUp;

        return *this;
    }

};

struct dwQoSMultiProbeListener : bdQoSProbeListener // sizeof=0x8EA4
{                                       // XREF: .data:s_qoslistener/r
    bdQoSRemoteAddr m_probeAddrs[500];
    bool m_probeSucceeded[500];
    unsigned int m_numProbes;
    bdQoSProbeInfo m_successes[500];
    bdSecurityID m_successIDs[500];
    unsigned int m_numSuccesses;
    unsigned int m_numFailures;

    dwQoSMultiProbeListener();
    ~dwQoSMultiProbeListener() = default;

    void addProbe(bdQoSRemoteAddr addr);
    void onQoSProbeSuccess(const bdQoSProbeInfo *info);
    void onQoSProbeFail(bdReference<bdCommonAddr> addr);
    void clean();
    int  complete();
};




void __cdecl dwStartQoSProbes(dwQoSMultiProbeListener *listener, unsigned int numProbes, bdQoSRemoteAddr *serverAddrs);
void __cdecl dwClearQoSProbes();
//void __thiscall bdReference<bdCommonAddr>::operator=(bdReference<bdRemoteTask> *this, bdRemoteTask *p);

#endif