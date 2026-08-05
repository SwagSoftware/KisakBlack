#include "net_chan_mp.h"
#include "cmd.h"
#include <client/cl_main.h>
#include "common.h"
#include <win32/win_shared.h>
#include <client_mp/cl_net_chan_mp.h>
#include <universal/com_files.h>
#include <DW/dwNet.h>
#include <win32/win_net.h>

#include <Windows.h>
#include "threads.h"
#include <server_mp/sv_net_chan_mp.h>

// (jedi academy)
#define MAX_PACKETLEN 1264
#define	FRAGMENT_SIZE			(MAX_PACKETLEN - 100)

loopback_t loopbacks[2];

const dvar_t *showpackets;
const dvar_t *showdrop;
const dvar_t *packetDebug;
const dvar_t *net_profile;
const dvar_t *net_showprofile;
const dvar_t *net_minigraph;
const dvar_t *net_lanauthorize;
const dvar_t *msg_printEntityNums;
const dvar_t *msg_dumpEnts;
const dvar_t *msg_zlibCompress;
const dvar_t *msg_zlibCompressOutput;
const dvar_t *msg_hudelemspew;

unsigned __int8 tempNetchanPacketBuf[65536];

DeferredQueue deferredQueue;

int net_iProfilingOn;
int g_qport;

const char *netsrcString[2] =
{ "client1", "server" };

const char *connectionString_135[11] =
{
  "CA_DISCONNECTED",
  "CA_CINEMATIC",
  "CA_UICINEMATIC",
  "CA_LOGO",
  "CA_CONNECTING",
  "CA_CHALLENGING",
  "CA_CONNECTED",
  "CA_SENDINGSTATS",
  "CA_LOADING",
  "CA_PRIMED",
  "CA_ACTIVE"
};

char s_1[64];
char *__cdecl NET_AdrToString(netadr_t a)
{
    __int16 v1; // ax

    Com_sprintf(s_1, 0x40u, "unknown");
    if ( a.type == NA_LOOPBACK )
    {
        Com_sprintf(s_1, 0x40u, "loopback");
    }
    else if ( a.type == NA_IP )
    {
        v1 = BigShort(a.port);
        Com_sprintf(s_1, 0x40u, "%i.%i.%i.%i:%i", a.ip[0], a.ip[1], a.ip[2], a.ip[3], v1);
    }
    return s_1;
}

char s_2[64];
char *__cdecl NET_AdrToStringDW(netadr_t a)
{
    Com_sprintf(s_2, 0x40u, "unknown");
    if ( a.type == NA_LOOPBACK )
    {
        Com_sprintf(s_2, 0x40u, "loopback");
    }
    else if ( a.type == NA_IP )
    {
        Com_sprintf(s_2, 0x40u, "%i.%i.%i.%i:%i", a.ip[0], a.ip[1], a.ip[2], a.ip[3], a.port);
    }
    return s_2;
}

void __cdecl NetProf_PrepProfiling(netProfileInfo_t *prof)
{
    if ( net_profile->current.integer )
    {
        if ( !net_iProfilingOn )
        {
            if ( !com_sv_running->current.enabled || CL_AnyLocalClientsRunning() && net_profile->current.integer == 2 )
                net_iProfilingOn = 1;
            else
                net_iProfilingOn = 2;
            Com_Printf(16, "Net Profiling turned on: %s\n", connectionString_135[net_iProfilingOn + 10]);
            memset(prof, 0, sizeof(netProfileInfo_t));
        }
    }
    else if ( net_iProfilingOn )
    {
        net_iProfilingOn = 0;
        Com_Printf(16, "Net Profiling turned off\n");
        memset(prof, 0, sizeof(netProfileInfo_t));
    }
}

void __cdecl NetProf_AddPacket(netProfileStream_t *pProfStream, int iSize, int bFragment)
{
    netProfilePacket_t *pPacket; // [esp+0h] [ebp-4h]

    if ( !net_iProfilingOn
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\net_chan_mp.cpp",
                    277,
                    0,
                    "%s",
                    "net_iProfilingOn") )
    {
        __debugbreak();
    }
    pProfStream->iCurrPacket = (pProfStream->iCurrPacket + 1) % 60;
    pPacket = &pProfStream->packets[pProfStream->iCurrPacket];
    pPacket->iTime = Sys_Milliseconds();
    pPacket->iSize = iSize;
    pPacket->bFragment = bFragment;
}

void __cdecl NetProf_NewSendPacket(netchan_t *pChan, int iSize, int bFragment)
{
    if ( net_iProfilingOn )
    {
        NetProf_AddPacket(&pChan->prof.send, iSize, bFragment);
        if ( (net_showprofile->current.integer & 2) != 0 )
        {
            if ( bFragment )
                Com_Printf(16, "[%s] send%s: %i\n", netsrcString[pChan->sock], " fragment", iSize);
            else
                Com_Printf(16, "[%s] send%s: %i\n", netsrcString[pChan->sock], "", iSize);
        }
    }
}

void __cdecl NetProf_NewRecievePacket(netchan_t *pChan, int iSize, int bFragment)
{
    if ( net_iProfilingOn )
    {
        NetProf_AddPacket(&pChan->prof.recieve, iSize, bFragment);
        if ( (net_showprofile->current.integer & 2) != 0 )
        {
            if ( bFragment )
                Com_Printf(16, "[%s] recieve%s: %i\n", netsrcString[pChan->sock], " fragment", iSize);
            else
                Com_Printf(16, "[%s] recieve%s: %i\n", netsrcString[pChan->sock], "", iSize);
        }
    }
}

void __cdecl NetProf_UpdateStatistics(netProfileStream_t *pStream)
{
    int v1; // esi
    int iTimeSpan; // [esp+4h] [ebp-2Ch]
    int iTotalBytes; // [esp+8h] [ebp-28h]
    int iLargestSize; // [esp+Ch] [ebp-24h]
    int iOldestPacket; // [esp+10h] [ebp-20h]
    int iSmallestSize; // [esp+14h] [ebp-1Ch]
    int iNumFragments; // [esp+18h] [ebp-18h]
    int i; // [esp+1Ch] [ebp-14h]
    int iNumPackets; // [esp+20h] [ebp-10h]
    int iOldestPacketTime; // [esp+2Ch] [ebp-4h]

    if ( !pStream
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\net_chan_mp.cpp", 348, 0, "%s", "pStream") )
    {
        __debugbreak();
    }
    if ( !net_iProfilingOn
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\net_chan_mp.cpp",
                    349,
                    0,
                    "%s",
                    "net_iProfilingOn") )
    {
        __debugbreak();
    }
    iNumPackets = 0;
    iNumFragments = 0;
    iOldestPacket = -1;
    iOldestPacketTime = Sys_Milliseconds();
    iTotalBytes = 0;
    iSmallestSize = 9999;
    iLargestSize = 0;
    for ( i = 0; i < 60; ++i )
    {
        if ( pStream->packets[i].iTime && (int)Sys_Milliseconds() <= pStream->packets[i].iTime + 1000 )
        {
            ++iNumPackets;
            if ( pStream->packets[i].bFragment )
                ++iNumFragments;
            if ( pStream->packets[i].iTime < iOldestPacketTime )
            {
                iOldestPacket = i;
                iOldestPacketTime = pStream->packets[i].iTime;
            }
            iTotalBytes += pStream->packets[i].iSize;
            if ( pStream->packets[i].iSize < iSmallestSize )
                iSmallestSize = pStream->packets[i].iSize;
            if ( pStream->packets[i].iSize > iLargestSize )
                iLargestSize = pStream->packets[i].iSize;
        }
    }
    if ( iNumPackets )
    {
        if ( iNumFragments )
            pStream->iFragmentPercentage = 100 * iNumFragments / iNumPackets;
        else
            pStream->iFragmentPercentage = 0;
        pStream->iLargestPacket = iLargestSize;
        pStream->iSmallestPacket = iSmallestSize;
        v1 = pStream->iLastBPSCalcTime + 100;
        if ( v1 < (int)Sys_Milliseconds() )
        {
            iTimeSpan = Sys_Milliseconds() - iOldestPacketTime;
            if ( iOldestPacket != -1 )
            {
                iTotalBytes -= pStream->packets[iOldestPacket].iSize;
                --iNumPackets;
                if ( pStream->packets[iOldestPacket].bFragment )
                    --iNumFragments;
            }
            if ( iTimeSpan >= 1 && iNumPackets )
            {
                if ( iTotalBytes )
                    pStream->iBytesPerSecond = (int)(float)((float)iTotalBytes / (float)((float)iTimeSpan * 0.001));
                else
                    pStream->iBytesPerSecond = 0;
                pStream->iLastBPSCalcTime = Sys_Milliseconds();
            }
            else
            {
                pStream->iBytesPerSecond = 0;
            }
        }
        pStream->iCountedPackets = iNumPackets;
        pStream->iCountedFragments = iNumFragments;
    }
    else
    {
        pStream->iBytesPerSecond = 0;
        pStream->iLastBPSCalcTime = 0;
        pStream->iCountedPackets = 0;
        pStream->iCountedFragments = 0;
        pStream->iFragmentPercentage = 0;
        pStream->iLargestPacket = 0;
        pStream->iSmallestPacket = 0;
    }
}

void __cdecl Net_DisplayProfile(int localClientNum)
{
    if ( !net_profile->current.integer )
        Dvar_SetInt((dvar_s *)net_profile, 2 - com_sv_running->current.enabled);
    if ( net_iProfilingOn )
    {
        if (net_iProfilingOn == 1)
        {
            CL_Netchan_PrintProfileStats(localClientNum, 0);
        }
        else
        {
            //BLOPS_NULLSUB();
        }
    }
}

void __cdecl Net_DumpProfile_f()
{
    if (net_iProfilingOn)
    {
        if (net_iProfilingOn == 1)
        {
            CL_Netchan_PrintProfileStats(0, 1);
        }
        else
        {
            //BLOPS_NULLSUB();
        }
    }
    else
    {
        Com_Printf(0, "Network profiling is not on. Set net_profile to turn on network profiling\n");
    }
}

cmd_function_s Net_DumpProfile_f_VAR;
cmd_function_s MSG_DumpNetFieldChanges_f_VAR;
cmd_function_s Net_GetQPort_f_VAR;
cmd_function_s Net_SetQPort_f_VAR;

void __cdecl Netchan_Init(__int16 port)
{
    showpackets = _Dvar_RegisterInt("showpackets", 0, 0, 2, 0, "Show packets");
    showdrop = _Dvar_RegisterBool("showdrop", 0, 0, "Show dropped packets");
    packetDebug = _Dvar_RegisterBool("packetDebug", 0, 0, "Enable packet debugging information");
    g_qport = port;
    net_profile = _Dvar_RegisterInt("net_profile", 0, 0, 2, 0, "Profile network performance");
    net_showprofile = _Dvar_RegisterInt("net_showprofile", 0, 0, 3, 0, "Show network profiling display");
    net_minigraph = _Dvar_RegisterBool("net_minigraph", 0, 0, "Show the small network profile display");
    net_lanauthorize = _Dvar_RegisterBool("net_lanauthorize", 0, 0, "Authorise CD keys when using a LAN");
    msg_printEntityNums = _Dvar_RegisterBool("msg_printEntityNums", 0, 0, "Print entity numbers");
    msg_dumpEnts = _Dvar_RegisterBool("msg_dumpEnts", 0, 0, "Print snapshot entity info");
    msg_zlibCompress = _Dvar_RegisterBool("msg_zlibCompress", 0, 0, "Enable zlib compression");
    msg_zlibCompressOutput = _Dvar_RegisterBool("msg_zlibCompressOutput", 0, 0, "Enable zlib compression console output");
    msg_hudelemspew = _Dvar_RegisterBool("msg_hudelemspew", 0, 0, "Debug hudelem fields changing");
    Cmd_AddCommandInternal("net_dumpprofile", Net_DumpProfile_f, &Net_DumpProfile_f_VAR);
    Cmd_AddCommandInternal("net_dumpnetfieldchanges", MSG_DumpNetFieldChanges_f, &MSG_DumpNetFieldChanges_f_VAR);
    Cmd_AddCommandInternal("getqport", Net_GetQPort_f, &Net_GetQPort_f_VAR);
    Cmd_AddCommandInternal("setqport", Net_SetQPort_f, &Net_SetQPort_f_VAR);
}

void __cdecl Net_GetQPort_f()
{
    Com_Printf(16, "qport = %i\n", g_qport);
}

void __cdecl Net_SetQPort_f()
{
    const char *v0; // eax

    if ( Cmd_Argc() < 1 )
        Com_PrintError(16, "setqport usage: setqport <qport>\n");
    v0 = Cmd_Argv(1);
    g_qport = (__int16)atoi(v0);
}

void __cdecl Netchan_Setup(
                netsrc_t sock,
                netchan_t *chan,
                netadr_t adr,
                int qport,
                char *outgoingBuffer,
                int outgoingBufferSize,
                char *incomingBuffer,
                int incomingBufferSize)
{
    memset(chan, 0, sizeof(netchan_t));
    chan->sock = sock;
    chan->remoteAddress = adr;
    chan->qport = qport;
    if ( adr.type
        && !qport
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\net_chan_mp.cpp",
                    1135,
                    0,
                    "%s",
                    "adr.type == NA_BOT || qport != 0") )
    {
        __debugbreak();
    }
    chan->incomingSequence = 0;
    chan->outgoingSequence = 1;
    chan->unsentBuffer = (unsigned __int8 *)outgoingBuffer;
    chan->unsentBufferSize = outgoingBufferSize;
    chan->fragmentBuffer = (unsigned __int8 *)incomingBuffer;
    chan->fragmentBufferSize = incomingBufferSize;
    NetProf_PrepProfiling(&chan->prof);
}

int __cdecl Netchan_TransmitFragment(
                netchan_t *chan,
                int fragmentLength,
                int fragmentIndex,
                unsigned __int8 maxFragmentIndex)
{
    msg_t send; // [esp+0h] [ebp-530h] BYREF
    unsigned __int8 send_buf[1268]; // [esp+30h] [ebp-500h] BYREF
    int res; // [esp+528h] [ebp-8h]
    int fragmentStart; // [esp+52Ch] [ebp-4h]

    MSG_Init(&send, send_buf, MAX_PACKETLEN);
    if ( chan->reliable_fragments )
        MSG_WriteLong(&send, chan->outgoingSequence | 0xC0000000);
    else
        MSG_WriteLong(&send, chan->outgoingSequence | 0x80000000);
    if ( chan->sock < NS_SERVER )
        MSG_WriteShort(&send, chan->qport);
    MSG_WriteByte(&send, fragmentIndex);
    MSG_WriteByte(&send, maxFragmentIndex);
    MSG_WriteShort(&send, FRAGMENT_SIZE);
    MSG_WriteShort(&send, fragmentLength);
    fragmentStart = FRAGMENT_SIZE * fragmentIndex;
    MSG_WriteData(&send, &chan->unsentBuffer[FRAGMENT_SIZE * fragmentIndex], fragmentLength);

    res = (unsigned __int8)NET_SendPacket(chan->sock, send.cursize, send.data, chan->remoteAddress);

    NetProf_NewSendPacket(chan, send.cursize, 1);
    if ( showpackets->current.integer && (showpackets->current.integer > 1 || chan->remoteAddress.type != NA_LOOPBACK) )
        Com_Printf(
            16,
            "[%s] send %4i : s=%i fragment=%i,%i\n",
            netsrcString[chan->sock],
            send.cursize,
            chan->outgoingSequence - 1,
            fragmentStart,
            fragmentLength);
    return res;
}

bool __cdecl Netchan_TransmitNextFragment(netchan_t *chan)
{
    int v2; // eax
    int fragmentLength; // [esp+0h] [ebp-24h]
    int fragmentIndex; // [esp+8h] [ebp-1Ch]
    int fragmentStart; // [esp+Ch] [ebp-18h]
    int i; // [esp+10h] [ebp-14h]
    int maxFragments; // [esp+1Ch] [ebp-8h]
    char message_is_acked; // [esp+23h] [ebp-1h]

    NetProf_PrepProfiling(&chan->prof);
    maxFragments = (chan->unsentLength + (FRAGMENT_SIZE-1)) / FRAGMENT_SIZE;
    message_is_acked = 1;
    for ( i = 0; i < maxFragments; ++i )
        message_is_acked &= (chan->fragment_ack[i >> 5] & (1 << (i & 0x1F))) != 0;
    if ( message_is_acked )
    {
        ++chan->outgoingSequence;
        chan->unsentFragments = 0;
        chan->reliable_fragments = 0;
        return 1;
    }
    else
    {
LABEL_6:
        for ( fragmentIndex = 0; ; ++fragmentIndex )
        {
            if ( fragmentIndex >= 128 )
                goto LABEL_6;
            fragmentStart = FRAGMENT_SIZE * fragmentIndex;
            if ( FRAGMENT_SIZE * fragmentIndex < chan->unsentLength
                && chan->fragment_send_count[fragmentIndex] <= chan->lowest_send_count
                && (chan->fragment_ack[fragmentIndex >> 5] & (1 << (fragmentIndex & 0x1F))) == 0 )
            {
                break;
            }
            if ( chan->reliable_fragments && fragmentStart >= chan->unsentLength )
            {
                ++chan->lowest_send_count;
                return 1;
            }
        }
        ++chan->fragment_send_count[fragmentIndex];
        if ( fragmentStart + FRAGMENT_SIZE <= chan->unsentLength )
        {
            fragmentLength = FRAGMENT_SIZE;
            v2 = Netchan_TransmitFragment(chan, FRAGMENT_SIZE, fragmentIndex, maxFragments - 1);
        }
        else
        {
            fragmentLength = chan->unsentLength - fragmentStart;
            v2 = Netchan_TransmitFragment(chan, fragmentLength, fragmentIndex, maxFragments - 1);
        }
        if ( !chan->reliable_fragments && fragmentLength + fragmentStart == chan->unsentLength )
        {
            ++chan->outgoingSequence;
            chan->unsentFragments = 0;
            chan->reliable_fragments = 0;
        }
        return v2 > 0;
    }
}

bool __cdecl Netchan_Transmit(netchan_t *chan, int length, char *data, bool reliable_fragments)
{
    int file; // [esp+0h] [ebp-52Ch]
    msg_t send; // [esp+4h] [ebp-528h] BYREF
    unsigned __int8 send_buf[MAX_PACKETLEN]; // [esp+34h] [ebp-4F8h] BYREF
    int res; // [esp+528h] [ebp-4h]

    if ( length > MAX_MSGLEN )
    {
        file = FS_FOpenFileWrite((char*)"badpacket.dat");
        if ( file )
        {
            FS_Write(data, length, file);
            FS_FCloseFile(file);
        }
        Com_Error(ERR_DROP, "Netchan_Transmit: length = %i", length);
    }

    chan->unsentFragmentStart = 0;

    if (chan->remoteAddress.type == NA_BOT)
    {
        ++chan->outgoingSequence;
        return 1;
    }

    if (length >= FRAGMENT_SIZE)
    {
        chan->unsentFragments = 1;
        chan->unsentLength = length;
        chan->reliable_fragments = reliable_fragments;
        chan->lowest_send_count = 0;
        chan->fragmentLength = 0;
        chan->fragment_ack[0] = 0;
        chan->fragment_ack[1] = 0;
        chan->fragment_ack[2] = 0;
        chan->fragment_ack[3] = 0;
        memset(chan->fragment_send_count, 0, sizeof(chan->fragment_send_count));

        iassert(chan->unsentBufferSize > length);
        
        Com_Memcpy(chan->unsentBuffer, data, length);
        Netchan_TransmitNextFragment(chan);
        return 1;
    }

    {
        NetProf_PrepProfiling(&chan->prof);
        MSG_Init(&send, send_buf, MAX_PACKETLEN);
        MSG_WriteLong(&send, chan->outgoingSequence);
        ++chan->outgoingSequence;
        if (chan->sock < NS_SERVER)
            MSG_WriteShort(&send, chan->qport);
        if (packetDebug->current.enabled)
            Com_Printf(16, "Adding %i byte payload to packet\n", length);
        MSG_WriteData(&send, (unsigned __int8 *)data, length);
        if (packetDebug->current.enabled)
            Com_Printf(16, "Sending %i byte packet\n", send.cursize);
        res = (unsigned __int8)NET_SendPacket(chan->sock, send.cursize, send.data, chan->remoteAddress);
        NetProf_NewSendPacket(chan, send.cursize, 0);
        if (showpackets->current.integer && (showpackets->current.integer > 1 || chan->remoteAddress.type != NA_LOOPBACK))
            Com_Printf(
                16,
                "[%s] send->%u.%u.%u.%u (%4i bytes) : s=%i ack=%i\n",
                netsrcString[chan->sock],
                chan->remoteAddress.ip[0],
                chan->remoteAddress.ip[1],
                chan->remoteAddress.ip[2],
                chan->remoteAddress.ip[3],
                send.cursize,
                chan->outgoingSequence - 1,
                chan->incomingSequence);
        return res > 0;
    }
}

int __cdecl Netchan_Process(netchan_t *chan, msg_t *msg)
{
    int incomingSequence; // [esp-4h] [ebp-88Ch]
    int i; // [esp+4h] [ebp-884h]
    unsigned __int8 data[2048]; // [esp+8h] [ebp-880h] BYREF
    msg_t buf; // [esp+80Ch] [ebp-7Ch] BYREF
    int res; // [esp+83Ch] [ebp-4Ch]
    int maxFragmentIndex; // [esp+840h] [ebp-48h]
    int fragmentLength; // [esp+844h] [ebp-44h]
    int fragmentIndex; // [esp+848h] [ebp-40h]
    int c; // [esp+84Ch] [ebp-3Ch]
    int ack_bit_index; // [esp+850h] [ebp-38h]
    bool messageComplete; // [esp+857h] [ebp-31h]
    int fragmentOffset; // [esp+858h] [ebp-30h]
    bool previously_acked; // [esp+85Fh] [ebp-29h]
    int fragmentSize; // [esp+860h] [ebp-28h]
    int fragment_ack[4]; // [esp+864h] [ebp-24h]
    int force_ack; // [esp+874h] [ebp-14h]
    int fragmented; // [esp+878h] [ebp-10h]
    int ack_requested; // [esp+87Ch] [ebp-Ch]
    int sequence; // [esp+880h] [ebp-8h]
    int qport; // [esp+884h] [ebp-4h]

    ack_requested = 0;
    force_ack = 0;

    NetProf_PrepProfiling(&chan->prof);
    MSG_BeginReading(msg);

    sequence = MSG_ReadLong(msg);

    if ( chan->sock == NS_SERVER )
        qport = MSG_ReadShort(msg);

    if ( sequence >= 0 )
    {
        fragmented = 0;
    }
    else
    {
        if ( (sequence & 0x40000000) != 0 )
            ack_requested = 1;
        if ( (sequence & 0x20000000) != 0 )
        {
            MSG_ReadByte(msg);
            MSG_ReadByte(msg);
            fragmentSize = MSG_ReadShort(msg);
            MSG_ReadShort(msg);
            if ( fragmentSize != FRAGMENT_SIZE )
            {
                Com_Printf(16, "%08d: [%s] Warning: Invalid fragment size %d\n", Sys_Milliseconds(), NET_AdrToString(chan->remoteAddress), fragmentSize);
            }
            fragment_ack[0] = MSG_ReadLong(msg);
            fragment_ack[1] = MSG_ReadLong(msg);
            fragment_ack[2] = MSG_ReadLong(msg);
            fragment_ack[3] = MSG_ReadLong(msg);
            sequence &= 0x1FFFFFFFu;
            if ( sequence == chan->outgoingSequence )
            {
                chan->fragment_ack[0] |= fragment_ack[0];
                chan->fragment_ack[1] |= fragment_ack[1];
                chan->fragment_ack[2] |= fragment_ack[2];
                chan->fragment_ack[3] |= fragment_ack[3];
            }
            return 0;
        }
        sequence &= 0x1FFFFFFFu;
        fragmented = 1;
    }
    if ( sequence <= chan->incomingSequence )
    {
        if ( sequence != chan->incomingSequence || !ack_requested )
        {
            if ( showdrop->current.enabled
                || showpackets->current.integer && (showpackets->current.integer > 1 || chan->remoteAddress.type != NA_LOOPBACK) )
            {
                incomingSequence = chan->incomingSequence;
                Com_Printf(
                    16,
                    "[%s] %s: Out of order packet %i at %i\n",
                    netsrcString[chan->sock],
                    NET_AdrToString(chan->remoteAddress),
                    sequence,
                    incomingSequence);
            }
            return 0;
        }
        force_ack = 1;
    }
    NetProf_NewRecievePacket(chan, msg->cursize, fragmented);
    if ( fragmented )
    {
        fragmentIndex = MSG_ReadByte(msg);
        maxFragmentIndex = MSG_ReadByte(msg);
        c = MSG_ReadShort(msg);
        fragmentLength = MSG_ReadShort(msg);
        if ( sequence > chan->fragmentSequence )
        {
            chan->fragmentSequence = sequence;
            chan->fragmentLength = 0;
            chan->fragment_ack[0] = 0;
            chan->fragment_ack[1] = 0;
            chan->fragment_ack[2] = 0;
            chan->fragment_ack[3] = 0;
        }
        fragmentOffset = c * fragmentIndex;
        ack_bit_index = fragmentIndex & 0x1F;
        previously_acked = (chan->fragment_ack[fragmentIndex >> 5] & (1 << (fragmentIndex & 0x1F))) != 0;
        if ( !force_ack )
            chan->fragment_ack[fragmentIndex >> 5] |= 1 << ack_bit_index;
        if ( ack_requested )
        {
            memset(&buf, 0, sizeof(buf));
            MSG_Init(&buf, data, 2048);
            MSG_WriteLong(&buf, chan->fragmentSequence | 0xA0000000);
            if ( chan->sock < NS_SERVER )
                MSG_WriteShort(&buf, chan->qport);
            MSG_WriteByte(&buf, fragmentIndex);
            MSG_WriteByte(&buf, maxFragmentIndex);
            MSG_WriteShort(&buf, c);
            MSG_WriteShort(&buf, fragmentLength);
            if ( force_ack )
            {
                MSG_WriteLong(&buf, -1);
                MSG_WriteLong(&buf, -1);
                MSG_WriteLong(&buf, -1);
                MSG_WriteLong(&buf, -1);
                res = (unsigned __int8)NET_SendPacket(chan->sock, buf.cursize, buf.data, chan->remoteAddress);
                return 0;
            }
            MSG_WriteLong(&buf, chan->fragment_ack[0]);
            MSG_WriteLong(&buf, chan->fragment_ack[1]);
            MSG_WriteLong(&buf, chan->fragment_ack[2]);
            MSG_WriteLong(&buf, chan->fragment_ack[3]);
            res = (unsigned __int8)NET_SendPacket(chan->sock, buf.cursize, buf.data, chan->remoteAddress);
        }
        if ( !previously_acked )
        {
            chan->fragmentLength += fragmentLength;
            memcpy(&chan->fragmentBuffer[fragmentOffset], &msg->data[msg->readcount], fragmentLength);
        }
        messageComplete = 1;
        for ( i = 0; i <= maxFragmentIndex; ++i )
        {
            ack_bit_index = i & 0x1F;
            messageComplete &= (chan->fragment_ack[i >> 5] & (1 << (i & 0x1F))) != 0;
            if ( !messageComplete )
                return 0;
        }
        if ( chan->fragmentLength > msg->maxsize )
            return 0;
        *(int *)msg->data = sequence;
        memcpy(msg->data + 4, chan->fragmentBuffer, chan->fragmentLength);
        msg->cursize = chan->fragmentLength + 4;
        chan->fragmentLength = 0;
        chan->fragment_ack[0] = 0;
        chan->fragment_ack[1] = 0;
        chan->fragment_ack[2] = 0;
        chan->fragment_ack[3] = 0;
        MSG_BeginReading(msg);
        MSG_ReadLong(msg);
    }
    chan->incomingSequence = sequence;
    return 1;
}

int __cdecl NET_CompareBaseAdrSigned(netadr_t *a, netadr_t *b)
{
    if ( a->type != b->type )
        return a->type - b->type;
    switch ( a->type )
    {
        case NA_LOOPBACK:
            return a->port - b->port;
        case NA_BOT:
            return a->port - b->port;
        case NA_IP:
            return memcmp((const char *)a->ip, (const char *)b->ip, 4);
    }
    Com_Printf(16, "NET_CompareBaseAdrSigned: bad address type\n");
    return 0;
}

bool __cdecl NET_CompareBaseAdr(netadr_t a, netadr_t b)
{
    return NET_CompareBaseAdrSigned(&a, &b) == 0;
}

int __cdecl NET_CompareAdrSigned(netadr_t *a, netadr_t *b)
{
    if ( a->type != b->type )
        return a->type - b->type;
    if ( a->type == NA_LOOPBACK )
        return 0;
    if ( a->type == NA_IP )
    {
        if ( a->port == b->port )
            return memcmp((const char *)a->ip, (const char *)b->ip, 4);
        else
            return a->port - b->port;
    }
    else
    {
        Com_Printf(16, "NET_CompareAdrSigned: bad address type\n");
        return 0;
    }
}

bool __cdecl NET_CompareAdr(netadr_t a, netadr_t b)
{
    return NET_CompareAdrSigned(&a, &b) == 0;
}

bool __cdecl NET_IsLocalAddress(netadr_t adr)
{
    return adr.type == NA_LOOPBACK || adr.type == NA_BOT;
}

void __cdecl NET_SetNetAdrLoopbackIP(netadr_t *src)
{
    if ( !src && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\net_chan_mp.cpp", 1919, 0, "%s", "src") )
        __debugbreak();
    src->ip[0] = 127;
    src->ip[1] = 0;
    src->ip[2] = 0;
    src->ip[3] = 1;
#ifdef KISAK_LIVE
    src->addrHandleIndex = dwGetLoopbackIndex();
#else
    src->addrHandleIndex = -1;
#endif
    src->type = NA_LOOPBACK;
    src->port = 0;
}

int __cdecl NET_GetClientPacket(netadr_t *net_from, msg_t *net_message)
{
    return Sys_GetPacket(net_from, net_message);
}

int __cdecl NET_GetLoopPacket_Real(netsrc_t sock, netadr_t *net_from, msg_t *net_message)
{
    loopback_t *loop; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    loop = &loopbacks[sock];
    if ( loop->send - loop->get > 16 )
        loop->get = loop->send - 16;
    if ( loop->get >= loop->send )
        return 0;
    i = loop->get & 0xF;
    _InterlockedExchangeAdd(&loop->get, 1u);
    memcpy(net_message->data, loop->msgs[i].data, loop->msgs[i].datalen);
    net_message->cursize = loop->msgs[i].datalen;
    net_from->type = NA_BOT;
    *(unsigned int *)net_from->ip = 0;
    *(unsigned int *)&net_from->port = 0;
    net_from->addrHandleIndex = 0;
    net_from->type = NA_LOOPBACK;
    net_from->port = loop->msgs[i].port;
    return 1;
}

int __cdecl NET_GetLoopPacket(netsrc_t sock, netadr_t *net_from, msg_t *net_message)
{
    return NET_GetLoopPacket_Real(sock, net_from, net_message);
}

void __cdecl NET_SendLoopPacket(netsrc_t sock, unsigned int length, unsigned __int8 *data, netadr_t to)
{
    loopback_t *loop; // [esp+0h] [ebp-10h]
    int i; // [esp+4h] [ebp-Ch]
    netsrc_t port; // [esp+8h] [ebp-8h]

    port = NS_CLIENT1;
    if ( sock >= NS_SERVER )
    {
        if ( sock == NS_SERVER )
            sock = (netsrc_t)to.port;
    }
    else
    {
        port = sock;
        sock = NS_SERVER;
    }
    loop = &loopbacks[sock];
    i = loop->send & 0xF;
    memcpy(loop->msgs[i].data, data, length);
    loop->msgs[i].datalen = length;
    loop->msgs[i].port = port;
    _InterlockedExchangeAdd(&loop->send, 1u);
}

void __cdecl NET_DeferPacketToClient(netadr_t *net_from, msg_t *net_message)
{
    DeferredMsg *msg; // [esp+0h] [ebp-8h]

    iassert(Sys_IsServerThread());

    msg = &deferredQueue.msgs[deferredQueue.send & 0xF];
    memcpy(msg->data, net_message->data, net_message->cursize);
    msg->datalen = net_message->cursize;
    msg->targetLocalNetID = net_message->targetLocalNetID;
    msg->addr = *net_from;
    _InterlockedExchangeAdd(&deferredQueue.send, 1u);
}

bool __cdecl NET_GetDeferredClientPacket(netadr_t *net_from, msg_t *net_message)
{
    DeferredMsg *msg; // [esp+0h] [ebp-8h]

    iassert(Sys_IsMainThread());
    iassert(net_from);
    iassert(net_message);

    if ( deferredQueue.send - deferredQueue.get > 16 )
        deferredQueue.get = deferredQueue.send - 16;

    if ( deferredQueue.get >= deferredQueue.send )
        return false;

    msg = &deferredQueue.msgs[deferredQueue.get & 0xF];
    memcpy(net_message->data, msg->data, msg->datalen);

    net_message->cursize = msg->datalen;
    net_message->targetLocalNetID = msg->targetLocalNetID;
    *net_from = msg->addr;

    _InterlockedExchangeAdd(&deferredQueue.get, 1u);
    return 1;
}

char __cdecl NET_SendPacket(netsrc_t sock, unsigned int length, unsigned __int8 *data, netadr_t to)
{
    netadr_t v5; // [esp-10h] [ebp-14h]

    if ( showpackets->current.integer && *(unsigned int *)data == -1 )
        Com_Printf(16, "[%s] send packet %4i\n", netsrcString[sock], length);
    if ( to.type == NA_LOOPBACK )
    {
        *(_QWORD *)&v5.type = __PAIR64__(*(unsigned int *)to.ip, 2);
        *(_QWORD *)&v5.port = *(_QWORD *)&to.port;
        NET_SendLoopPacket(sock, length, data, v5);
        return 1;
    }
    else if ( to.type == NA_BAD )
    {
        return 0;
    }
    else if ( to.type )
    {
        return Sys_SendPacket(length, data, to);
    }
    else
    {
        return 0;
    }
}

bool __cdecl NET_OutOfBandPrint(netsrc_t sock, netadr_t adr, const char *data)
{
    char *v3; // eax
    const char *v4; // eax
    unsigned int v6; // [esp+0h] [ebp-60h]
    int res; // [esp+58h] [ebp-8h]

    tempNetchanPacketBuf[0] = -1;
    tempNetchanPacketBuf[1] = -1;
    tempNetchanPacketBuf[2] = -1;
    tempNetchanPacketBuf[3] = -1;
    if ( showpackets->current.integer && (showpackets->current.integer > 1 || adr.type != NA_LOOPBACK) )
    {
        v3 = NET_AdrToString(adr);
        Com_DPrintf(16, "OOB Print->%s: %s\n", v3, data);
    }
    if ( strlen(data) + 1 <= 0xFFFC )
    {
        strcpy((char *)&tempNetchanPacketBuf[4], data);
        v6 = strlen((const char *)tempNetchanPacketBuf);
        res = (unsigned __int8)NET_SendPacket(sock, v6, tempNetchanPacketBuf, adr);
        if ( sock == NS_SERVER )
            SV_Netchan_AddOOBProfilePacket(v6);
        else
            CL_Netchan_AddOOBProfilePacket(sock, v6);
        return res > 0;
    }
    else
    {
        Com_DPrintf(16, "OOB Packet is %i bytes - too large to send\n", strlen(data));
        v4 = va("OOB Packet is %i bytes - too large to send\n", strlen(data));
        if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\net_chan_mp.cpp", 2281, 0, v4) )
            __debugbreak();
        return 0;
    }
}

bool __cdecl NET_OutOfBandData(netsrc_t sock, netadr_t adr, const unsigned __int8 *format, int len)
{
    unsigned int mbuf_20; // [esp+14h] [ebp-2Ch]
    int i; // [esp+30h] [ebp-10h]
    int res; // [esp+38h] [ebp-8h]

    tempNetchanPacketBuf[0] = -1;
    tempNetchanPacketBuf[1] = -1;
    tempNetchanPacketBuf[2] = -1;
    tempNetchanPacketBuf[3] = -1;
    for ( i = 0; i < len; ++i )
        tempNetchanPacketBuf[i + 4] = format[i];
    mbuf_20 = len + 4;
    if ( showpackets->current.integer && (showpackets->current.integer > 1 || adr.type != NA_LOOPBACK) )
        Com_DPrintf(16, "OOB Data->%u.%u.%u.%u: %i bytes\n", adr.ip[0], adr.ip[1], adr.ip[2], adr.ip[3], mbuf_20);
    res = (unsigned __int8)NET_SendPacket(sock, mbuf_20, tempNetchanPacketBuf, adr);
    if ( sock == NS_SERVER )
        SV_Netchan_AddOOBProfilePacket(mbuf_20);
    else
        CL_Netchan_AddOOBProfilePacket(sock, mbuf_20);
    return res > 0;
}

bool __cdecl NET_OutOfBandVoiceData(netsrc_t sock, netadr_t adr, unsigned __int8 *format, unsigned int len)
{
    int mbuf_20; // [esp+14h] [ebp-28h]
    int res; // [esp+34h] [ebp-8h]

    tempNetchanPacketBuf[0] = -1;
    tempNetchanPacketBuf[1] = -1;
    tempNetchanPacketBuf[2] = -1;
    tempNetchanPacketBuf[3] = -1;
    if ( (int)(len + 4) >= 0x10000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\net_chan_mp.cpp",
                    2489,
                    0,
                    "%s",
                    "len + index < static_cast<int>( sizeof( tempNetchanPacketBuf ) )") )
    {
        __debugbreak();
    }
    memcpy(&tempNetchanPacketBuf[4], format, len);
    mbuf_20 = len + 4;
    res = (unsigned __int8)NET_SendPacket(sock, len + 4, tempNetchanPacketBuf, adr);
    if ( sock == NS_SERVER )
        SV_Netchan_AddOOBProfilePacket(mbuf_20);
    else
        CL_Netchan_AddOOBProfilePacket(sock, mbuf_20);
    return res > 0;
}

int __cdecl NET_StringToAdr(const char *s, netadr_t *a)
{
    char *v3; // eax
    __int16 v4; // ax
    unsigned __int16 v5; // ax
    char base[1024]; // [esp+18h] [ebp-408h] BYREF
    char *port; // [esp+41Ch] [ebp-4h]

    if ( !strcmp(s, "localhost") )
    {
        a->type = NA_BOT;
        *(unsigned int *)a->ip = 0;
        *(unsigned int *)&a->port = 0;
        a->addrHandleIndex = 0;
        a->type = NA_LOOPBACK;
        return 1;
    }
    else
    {
        I_strncpyz(base, s, 1024);
        v3 = strstr(base, ":");
        port = v3;
        if ( v3 )
            *port++ = 0;
        if ( Sys_StringToAdr(base, a) )
        {
            if ( a->ip[0] == 255 && a->ip[1] == 255 && a->ip[2] == 255 && a->ip[3] == 255 )
            {
                a->type = NA_BAD;
                return 0;
            }
            else
            {
                if ( port )
                {
                    v4 = atoi(port);
                    v5 = BigShort(v4);
                }
                else
                {
                    v5 = BigShort(3074);
                }
                a->port = v5;
                return 1;
            }
        }
        else
        {
            a->type = NA_BAD;
            return 0;
        }
    }
}

void __cdecl Int64ToString(__int64 int64, char *str)
{
    _snprintf(str, 0x11u, "%08x%08x", (unsigned int)int64, HIDWORD(int64));
}

__int64 __cdecl StringToInt64(const char *str)
{
    __int64 int64; // [esp+0h] [ebp-10h] BYREF
    unsigned int *intptr; // [esp+Ch] [ebp-4h]

    intptr = (unsigned int *)&int64;
    sscanf(str, "%8x%8x", &int64, (char *)&int64 + 4);
    return int64;
}

void __cdecl XUIDToString(unsigned __int64 *xuid, char *str)
{
    _ui64toa(*xuid, str, 0x10u);
}

void __cdecl StringToXUID(const char *str, unsigned __int64 *xuid)
{
    *xuid = _strtoui64(str, 0, 16);
}

