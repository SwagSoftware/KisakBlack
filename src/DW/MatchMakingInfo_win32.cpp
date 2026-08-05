#ifdef KISAK_DW

#include "MatchMakingInfo_win32.h"

MatchMakingInfo::MatchMakingInfo() : bdMatchMakingInfo()
{
    //bdMatchMakingInfo::bdMatchMakingInfo(this);
    //this->__vftable = (MatchMakingInfo_vtbl *)&MatchMakingInfo::`vftable';
    this->m_memberNETCODE_VERSION = 0;
    this->m_membermaprotation = 0;
    this->m_memberGAME_TYPE = 0;
    this->m_memberPLAYLIST = 0;
    this->m_memberPLAYLIST_VERSION = 0;
    this->m_memberPARTY_STATE = 0;
    this->m_memberMAPPACKS = 0;
    this->m_memberGAME_MODE = 0;
    this->m_memberSKILL = 0;
    this->m_memberTEAM_SIZE_MIN = 0;
    this->m_memberTEAM_SIZE_MAX = 0;
    this->m_memberGEOGRAPHICAL_LOCATION = 0;
    this->m_memberdemonwareID = 0;
    this->m_memberserverType = 0;
    this->m_memberroundTime = 0;
    this->m_memberaddressport = 0;
    this->m_memberDDL_VERSION = 0;
    this->m_memberLICENSE = 0;
    this->m_pingedtime = 0;
    this->m_active = 0;
    //*(unsigned int *)this->m_membersecKey = 0;
    //*(unsigned int *)&this->m_membersecKey[4] = 0;
    //*(unsigned int *)&this->m_membersecKey[8] = 0;
    //*(unsigned int *)&this->m_membersecKey[12] = 0;
    //this->m_membersecKey[16] = 0;
    memset(this->m_membersecKey, 0, sizeof(this->m_membersecKey));
    memset(this->m_memberservername, 0, sizeof(this->m_memberservername));
    //*(unsigned int *)this->m_membermapname = 0;
    //*(unsigned int *)&this->m_membermapname[4] = 0;
    //*(unsigned int *)&this->m_membermapname[8] = 0;
    //*(unsigned int *)&this->m_membermapname[12] = 0;
    //*(unsigned int *)&this->m_membermapname[16] = 0;
    //*(unsigned int *)&this->m_membermapname[20] = 0;
    //*(unsigned int *)&this->m_membermapname[24] = 0;
    //*(unsigned int *)&this->m_membermapname[28] = 0;
    //this->m_membermapname[32] = 0;
    memset(this->m_membermapname, 0, sizeof(this->m_membermapname));
    memset(this->m_membermod, 0, sizeof(this->m_membermod));

    //*(unsigned int *)this->m_membermod = 0;
    //*(unsigned int *)&this->m_membermod[4] = 0;
    //*(unsigned int *)&this->m_membermod[8] = 0;
    //*(unsigned int *)&this->m_membermod[12] = 0;
    //*(unsigned int *)&this->m_membermod[16] = 0;
    //*(unsigned int *)&this->m_membermod[20] = 0;
    //*(unsigned int *)&this->m_membermod[24] = 0;
    //*(unsigned int *)&this->m_membermod[28] = 0;
    //this->m_membermod[32] = 0;
    //return this;
}

void MatchMakingInfo::serialize(bdByteBuffer *buffer)
{
#ifdef KISAK_DW_STUBS
    bdMatchMakingInfo::serialize(this, buffer);
    bdByteBuffer::writeInt32(buffer, this->m_memberNETCODE_VERSION);
    bdByteBuffer::writeBlob(buffer, this->m_membersecKey, 0x10u);
    bdByteBuffer::writeString(buffer, this->m_memberservername, 0x40u);
    bdByteBuffer::writeUInt64(buffer, this->m_membermaprotation);
    bdByteBuffer::writeString(buffer, this->m_membermapname, 0x20u);
    bdByteBuffer::writeInt32(buffer, this->m_memberGAME_TYPE);
    bdByteBuffer::writeInt32(buffer, this->m_memberPLAYLIST);
    bdByteBuffer::writeInt32(buffer, this->m_memberPLAYLIST_VERSION);
    bdByteBuffer::writeInt32(buffer, this->m_memberPARTY_STATE);
    bdByteBuffer::writeInt32(buffer, this->m_memberMAPPACKS);
    bdByteBuffer::writeUInt32(buffer, this->m_memberLICENSE);
    bdByteBuffer::writeInt32(buffer, this->m_memberSKILL);
    bdByteBuffer::writeInt32(buffer, this->m_memberTEAM_SIZE_MIN);
    bdByteBuffer::writeInt32(buffer, this->m_memberTEAM_SIZE_MAX);
    bdByteBuffer::writeInt32(buffer, this->m_memberGEOGRAPHICAL_LOCATION);
    bdByteBuffer::writeUInt64(buffer, this->m_memberdemonwareID);
    bdByteBuffer::writeInt32(buffer, this->m_memberserverType);
    bdByteBuffer::writeInt32(buffer, this->m_memberroundTime);
    bdByteBuffer::writeString(buffer, this->m_membermod, 0x20u);
    bdByteBuffer::writeUInt64(buffer, this->m_memberaddressport);
    bdByteBuffer::writeUInt32(buffer, this->m_memberDDL_VERSION);
#endif
}

bool __thiscall MatchMakingInfo::deserialize(bdReference<bdByteBuffer> buffer)
{
#ifdef KISAK_DW_STUBS
    bdReference<bdCommonAddr> v3; // [esp-4h] [ebp-DCh] BYREF
    bool v4; // [esp+0h] [ebp-D8h]
    bool v5; // [esp+4h] [ebp-D4h]
    bool v6; // [esp+8h] [ebp-D0h]
    bool v7; // [esp+Ch] [ebp-CCh]
    bool v8; // [esp+10h] [ebp-C8h]
    bool v9; // [esp+14h] [ebp-C4h]
    bool v10; // [esp+18h] [ebp-C0h]
    bool v11; // [esp+1Ch] [ebp-BCh]
    bool v12; // [esp+20h] [ebp-B8h]
    bool v13; // [esp+24h] [ebp-B4h]
    bool v14; // [esp+28h] [ebp-B0h]
    bool v15; // [esp+2Ch] [ebp-ACh]
    bool v16; // [esp+30h] [ebp-A8h]
    bool v17; // [esp+34h] [ebp-A4h]
    bool v18; // [esp+38h] [ebp-A0h]
    bool v19; // [esp+3Ch] [ebp-9Ch]
    bool v20; // [esp+40h] [ebp-98h]
    bool v21; // [esp+44h] [ebp-94h]
    bool v22; // [esp+48h] [ebp-90h]
    bool v23; // [esp+4Ch] [ebp-8Ch]
    bool v24; // [esp+50h] [ebp-88h]
    bool v25; // [esp+57h] [ebp-81h]
    volatile int m_refCount; // [esp+58h] [ebp-80h]
    MatchMakingInfo *thisa; // [esp+5Ch] [ebp-7Ch]
    bdByteBuffer *v28; // [esp+6Ch] [ebp-6Ch]
    bdByteBuffer *v29; // [esp+70h] [ebp-68h]
    bdByteBuffer *v30; // [esp+74h] [ebp-64h]
    bdByteBuffer *v31; // [esp+78h] [ebp-60h]
    bdByteBuffer *v32; // [esp+7Ch] [ebp-5Ch]
    bdByteBuffer *v33; // [esp+80h] [ebp-58h]
    bdByteBuffer *v34; // [esp+84h] [ebp-54h]
    bdByteBuffer *v35; // [esp+88h] [ebp-50h]
    bdByteBuffer *v36; // [esp+8Ch] [ebp-4Ch]
    bdByteBuffer *v37; // [esp+90h] [ebp-48h]
    bdByteBuffer *v38; // [esp+94h] [ebp-44h]
    bdByteBuffer *v39; // [esp+98h] [ebp-40h]
    bdByteBuffer *v40; // [esp+9Ch] [ebp-3Ch]
    bdByteBuffer *v41; // [esp+A0h] [ebp-38h]
    bdByteBuffer *v42; // [esp+A4h] [ebp-34h]
    bdByteBuffer *v43; // [esp+A8h] [ebp-30h]
    bdByteBuffer *v44; // [esp+ACh] [ebp-2Ch]
    bdByteBuffer *v45; // [esp+B0h] [ebp-28h]
    bdByteBuffer *v46; // [esp+B4h] [ebp-24h]
    bdByteBuffer *v47; // [esp+B8h] [ebp-20h]
    bdByteBuffer *v48; // [esp+BCh] [ebp-1Ch]
    bdReference<bdCommonAddr> *v49; // [esp+C0h] [ebp-18h]
    bdCommonAddr *m_ptr; // [esp+C4h] [ebp-14h]
    bool v51; // [esp+CBh] [ebp-Dh]
    unsigned int tempBlobLength_secKey; // [esp+D0h] [ebp-8h] BYREF
    bool ok; // [esp+D7h] [ebp-1h]

    thisa = this;
    v49 = &v3;
    v3.m_ptr = (bdCommonAddr *)buffer.m_ptr;
    if ( buffer.m_ptr )
    {
        m_ptr = v49->m_ptr;
        InterlockedIncrement(&m_ptr->m_refCount);
        m_refCount = m_ptr->m_refCount;
    }
    v25 = bdMatchMakingInfo::deserialize((int)thisa, v3);
    ok = v25;
    v24 = v25 && (v48 = buffer.m_ptr, bdByteBuffer::readInt32(buffer.m_ptr, &thisa->m_memberNETCODE_VERSION));
    ok = v24;
    tempBlobLength_secKey = 16;
    v23 = v24 && (v47 = buffer.m_ptr, bdByteBuffer::readBlob(buffer.m_ptr, thisa->m_membersecKey, &tempBlobLength_secKey));
    ok = v23;
    v22 = v23 && (v46 = buffer.m_ptr, bdByteBuffer::readString(buffer.m_ptr, thisa->m_memberservername, 0x40u));
    ok = v22;
    v21 = v22 && (v45 = buffer.m_ptr, bdByteBuffer::readUInt64(buffer.m_ptr, &thisa->m_membermaprotation));
    ok = v21;
    v20 = v21 && (v44 = buffer.m_ptr, bdByteBuffer::readString(buffer.m_ptr, thisa->m_membermapname, 0x20u));
    ok = v20;
    v19 = v20 && (v43 = buffer.m_ptr, bdByteBuffer::readInt32(buffer.m_ptr, &thisa->m_memberGAME_TYPE));
    ok = v19;
    v18 = v19 && (v42 = buffer.m_ptr, bdByteBuffer::readInt32(buffer.m_ptr, &thisa->m_memberPLAYLIST));
    ok = v18;
    v17 = v18 && (v41 = buffer.m_ptr, bdByteBuffer::readInt32(buffer.m_ptr, &thisa->m_memberPLAYLIST_VERSION));
    ok = v17;
    v16 = v17 && (v40 = buffer.m_ptr, bdByteBuffer::readInt32(buffer.m_ptr, &thisa->m_memberPARTY_STATE));
    ok = v16;
    v15 = v16 && (v39 = buffer.m_ptr, bdByteBuffer::readInt32(buffer.m_ptr, &thisa->m_memberMAPPACKS));
    ok = v15;
    v14 = v15 && (v38 = buffer.m_ptr, bdByteBuffer::readUInt32(buffer.m_ptr, &thisa->m_memberLICENSE));
    ok = v14;
    v13 = v14 && (v37 = buffer.m_ptr, bdByteBuffer::readInt32(buffer.m_ptr, &thisa->m_memberSKILL));
    ok = v13;
    v12 = v13 && (v36 = buffer.m_ptr, bdByteBuffer::readInt32(buffer.m_ptr, &thisa->m_memberTEAM_SIZE_MIN));
    ok = v12;
    v11 = v12 && (v35 = buffer.m_ptr, bdByteBuffer::readInt32(buffer.m_ptr, &thisa->m_memberTEAM_SIZE_MAX));
    ok = v11;
    v10 = v11 && (v34 = buffer.m_ptr, bdByteBuffer::readInt32(buffer.m_ptr, &thisa->m_memberGEOGRAPHICAL_LOCATION));
    ok = v10;
    v9 = v10 && (v33 = buffer.m_ptr, bdByteBuffer::readUInt64(buffer.m_ptr, &thisa->m_memberdemonwareID));
    ok = v9;
    v8 = v9 && (v32 = buffer.m_ptr, bdByteBuffer::readInt32(buffer.m_ptr, &thisa->m_memberserverType));
    ok = v8;
    v7 = v8 && (v31 = buffer.m_ptr, bdByteBuffer::readInt32(buffer.m_ptr, &thisa->m_memberroundTime));
    ok = v7;
    v6 = v7 && (v30 = buffer.m_ptr, bdByteBuffer::readString(buffer.m_ptr, thisa->m_membermod, 0x20u));
    ok = v6;
    v5 = v6 && (v29 = buffer.m_ptr, bdByteBuffer::readUInt64(buffer.m_ptr, &thisa->m_memberaddressport));
    ok = v5;
    v4 = v5 && (v28 = buffer.m_ptr, bdByteBuffer::readUInt32(buffer.m_ptr, &thisa->m_memberDDL_VERSION));
    ok = v4;
    v51 = v4;
    bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>((bdReference<bdCommonAddr> *)&buffer);
    return v51;
#else
    return false;
#endif
}

unsigned int __thiscall MatchMakingInfo::sizeOf()
{
    return 552;
}

char __thiscall MatchMakingInfo::setSessionKey(const bdSecurityKey *secKey)
{
    *(bdSecurityKey *)this->m_membersecKey = *secKey;
    return 1;
}

char __thiscall MatchMakingInfo::getSessionKey(bdSecurityKey *secKey)
{
    *secKey = *(bdSecurityKey *)this->m_membersecKey;
    return 1;
}

bool __thiscall MatchMakingInfo::doUpdate(
                int msec,
                unsigned int updateIntervalMsec,
                unsigned int forceUpdateIntervalMsec)
{
#ifdef KISAK_DW_STUBS
    const char *MapName; // eax
    const char *v5; // eax
    const char *HostName; // eax
    const char *v7; // eax
    const char *String; // eax
    const char *v9; // eax
    unsigned int numPlayers; // [esp+4h] [ebp-8h]
    bool allowUpdate; // [esp+Bh] [ebp-1h]

    allowUpdate = 0;
    if ( this->m_active )
    {
        timeSinceUpdate += msec;
        allowUpdate = timeSinceUpdate >= forceUpdateIntervalMsec;
        if ( timeSinceUpdate >= updateIntervalMsec && this->m_dirty )
            allowUpdate = 1;
        MapName = SV_GetMapName();
        if ( I_strcmp(this->m_membermapname, MapName) )
        {
            v5 = SV_GetMapName();
            I_strncpyz(this->m_membermapname, v5, 32);
            allowUpdate = 1;
        }
        HostName = SV_GetHostName();
        if ( I_strcmp(this->m_memberservername, HostName) )
        {
            v7 = SV_GetHostName();
            I_strncpyz(this->m_memberservername, v7, 64);
            allowUpdate = 1;
        }
        String = Dvar_GetString("g_gametype");
        if ( Com_GametypeToInt(String) != this->m_memberGAME_TYPE )
        {
            v9 = Dvar_GetString("g_gametype");
            this->m_memberGAME_TYPE = Com_GametypeToInt(v9);
            allowUpdate = 1;
        }
        numPlayers = SV_GetClientCount();
        if ( this->m_numPlayers != numPlayers )
        {
            this->m_numPlayers = numPlayers;
            allowUpdate = 1;
        }
    }
    if ( allowUpdate )
        timeSinceUpdate = 0;
    return allowUpdate;
#else
    return false;
#endif
}


#endif