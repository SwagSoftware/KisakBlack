#pragma once
#include "msg_mp.h"

#include <universal/dvar.h>

#define MAX_MSGLEN 0x10000

enum netadrtype_t : __int32
{                                                                             // XREF: netadr_t/r
        NA_BOT             = 0x0,
        NA_BAD             = 0x1,
        NA_LOOPBACK    = 0x2,
        NA_BROADCAST = 0x3,
        NA_IP                = 0x4,
};

struct netadr_t // sizeof=0x10
{                                                                             // XREF: G_InitGrenadeEntity(gentity_s *,gentity_s *)+355/o
        netadrtype_t type;                                    // XREF: CL_ConnectionlessPacket(int,netadr_t,msg_t *,int)+1FF/r
                                                                                // CL_ConnectionlessPacket(int,netadr_t,msg_t *,int)+24B/r ...
        unsigned __int8 ip[4];                            // XREF: CL_ConnectionlessPacket(int,netadr_t,msg_t *,int)+204/r
                                                                                // CL_ConnectionlessPacket(int,netadr_t,msg_t *,int)+250/r ...
        unsigned __int16 port;                            // XREF: CL_ConnectionlessPacket(int,netadr_t,msg_t *,int)+20A/r
                                                                                // CL_ConnectionlessPacket(int,netadr_t,msg_t *,int)+256/r ...
        // padding byte
        // padding byte
        int addrHandleIndex;                                // XREF: CL_ConnectionlessPacket(int,netadr_t,msg_t *,int)+210/r
};

struct netProfilePacket_t // sizeof=0xC
{                                                                             // XREF: netProfileStream_t/r
        int iTime;
        int iSize;
        int bFragment;
};

struct netProfileStream_t // sizeof=0x2F0
{                                                                             // XREF: netProfileInfo_t/r
                                                                                // netProfileInfo_t/r
        netProfilePacket_t packets[60];
        int iCurrPacket;
        int iBytesPerSecond;
        int iLastBPSCalcTime;
        int iCountedPackets;
        int iCountedFragments;
        int iFragmentPercentage;
        int iLargestPacket;
        int iSmallestPacket;
};

struct netProfileInfo_t // sizeof=0x5E0
{                                                                             // XREF: netchan_t/r
                                                                                // clientConnection_t/r ...
        netProfileStream_t send;
        netProfileStream_t recieve;
};

enum netsrc_t : __int32;

struct netchan_t // sizeof=0x6C0
{                                                                             // XREF: clientConnection_t/r
                                                                                // clientHeader_t/r
        int outgoingSequence;
        netsrc_t sock;
        int dropped;
        int incomingSequence;
        netadr_t remoteAddress;
        int qport;
        int fragmentSequence;
        int fragmentLength;
        unsigned __int8 *fragmentBuffer;
        int fragmentBufferSize;
        int unsentFragments;
        int unsentFragmentStart;
        int unsentLength;
        unsigned __int8 *unsentBuffer;
        int unsentBufferSize;
        int reliable_fragments;
        unsigned __int8 fragment_send_count[128];
        unsigned int fragment_ack[4];
        int lowest_send_count;
        netProfileInfo_t prof;
};

struct loopmsg_t // sizeof=0x4F8
{                                       // XREF: loopback_t/r
    unsigned __int8 data[1264];
    int datalen;
    int port;
};

struct loopback_t // sizeof=0x4F88
{                                       // XREF: .data:loopbacks/r
    loopmsg_t msgs[16];
    volatile unsigned int get;
    volatile unsigned int send;
};

struct DeferredMsg // sizeof=0x508
{                                       // XREF: DeferredQueue/r
    netadr_t addr;
    unsigned __int8 data[1264];
    int datalen;
    netsrc_t targetLocalNetID;
};

struct DeferredQueue // sizeof=0x5088
{                                       // XREF: .data:deferredQueue/r
    DeferredMsg msgs[16];
    volatile unsigned int get;                   // XREF: NET_GetDeferredClientPacket(netadr_t *,msg_t *)+96/r
    volatile unsigned int send;                  // XREF: NET_DeferPacketToClient(netadr_t *,msg_t *):loc_6EAC58/r
                                        // NET_DeferPacketToClient(netadr_t *,msg_t *)+AC/o ...
};

char *__cdecl NET_AdrToString(netadr_t a);
char *__cdecl NET_AdrToStringOnline(netadr_t a);
void __cdecl NetProf_PrepProfiling(netProfileInfo_t *prof);
void __cdecl NetProf_AddPacket(netProfileStream_t *pProfStream, int iSize, int bFragment);
void __cdecl NetProf_NewSendPacket(netchan_t *pChan, int iSize, int bFragment);
void __cdecl NetProf_NewRecievePacket(netchan_t *pChan, int iSize, int bFragment);
void __cdecl NetProf_UpdateStatistics(netProfileStream_t *pStream);
void __cdecl Net_DisplayProfile(int localClientNum);
void __cdecl Netchan_Init(__int16 port);
void __cdecl Net_Dump_Profile_f();
void __cdecl Net_GetQPort_f();
void __cdecl Net_SetQPort_f();
void __cdecl Netchan_Setup(
                netsrc_t sock,
                netchan_t *chan,
                netadr_t adr,
                int qport,
                char *outgoingBuffer,
                int outgoingBufferSize,
                char *incomingBuffer,
                int incomingBufferSize);
int __cdecl Netchan_TransmitFragment(
                netchan_t *chan,
                int fragmentLength,
                int fragmentIndex,
                unsigned __int8 maxFragmentIndex);
bool __cdecl Netchan_TransmitNextFragment(netchan_t *chan);
bool __cdecl Netchan_Transmit(netchan_t *chan, int length, char *data, bool reliable_fragments);
int __cdecl Netchan_Process(netchan_t *chan, msg_t *msg);
int __cdecl NET_CompareBaseAdrSigned(netadr_t *a, netadr_t *b);
bool __cdecl NET_CompareBaseAdr(netadr_t a, netadr_t b);
int __cdecl NET_CompareAdrSigned(netadr_t *a, netadr_t *b);
bool __cdecl NET_CompareAdr(netadr_t a, netadr_t b);
bool __cdecl NET_IsLocalAddress(netadr_t adr);
void __cdecl NET_SetNetAdrLoopbackIP(netadr_t *src);
int __cdecl NET_GetClientPacket(netadr_t *net_from, msg_t *net_message);
int __cdecl NET_GetLoopPacket_Real(netsrc_t sock, netadr_t *net_from, msg_t *net_message);
int __cdecl NET_GetLoopPacket(netsrc_t sock, netadr_t *net_from, msg_t *net_message);
void __cdecl NET_SendLoopPacket(netsrc_t sock, unsigned int length, unsigned __int8 *data, netadr_t to);
void __cdecl NET_DeferPacketToClient(netadr_t *net_from, msg_t *net_message);
bool __cdecl NET_GetDeferredClientPacket(netadr_t *net_from, msg_t *net_message);
char __cdecl NET_SendPacket(netsrc_t sock, unsigned int length, unsigned __int8 *data, netadr_t to);
bool __cdecl NET_OutOfBandPrint(netsrc_t sock, netadr_t adr, const char *data);
bool __cdecl NET_OutOfBandData(netsrc_t sock, netadr_t adr, const unsigned __int8 *format, int len);
bool __cdecl NET_OutOfBandVoiceData(netsrc_t sock, netadr_t adr, unsigned __int8 *format, unsigned int len);
int __cdecl NET_StringToAdr(const char *s, netadr_t *a);
void __cdecl Int64ToString(__int64 int64, char *str);
__int64 __cdecl StringToInt64(const char *str);
void __cdecl XUIDToString(unsigned __int64 *xuid, char *str);
void __cdecl StringToXUID(const char *str, unsigned __int64 *xuid);


extern const dvar_t *showpackets;
extern const dvar_t *showdrop;
extern const dvar_t *packetDebug;
extern const dvar_t *net_profile;
extern const dvar_t *net_showprofile;
extern const dvar_t *net_minigraph;
extern const dvar_t *net_lanauthorize;
extern const dvar_t *msg_printEntityNums;
extern const dvar_t *msg_dumpEnts;
extern const dvar_t *msg_zlibCompress;
extern const dvar_t *msg_zlibCompressOutput;
extern const dvar_t *msg_hudelemspew;

extern int g_qport;