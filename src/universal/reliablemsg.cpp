#include "reliablemsg.h"
#include <cstring>
#include <win32/win_shared.h>
#include <qcommon/common.h>


reliableClient s_reliableClientMessages[64];

const dvar_t *reliableResendTime;
const dvar_t *reliableTimeoutTime;

void __cdecl RMsg_DropAllPacketsForClient(unsigned int clientSlot)
{
    if ( clientSlot >= 0x40
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\reliablemsg.cpp",
                    111,
                    0,
                    "clientSlot doesn't index MAX_DEST_CLIENTS\n\t%i not in [0, %i)",
                    clientSlot,
                    64) )
    {
        __debugbreak();
    }
    memset(&s_reliableClientMessages[clientSlot], 0, sizeof(reliableClient));
}

int __cdecl RMsg_FindSlotForAddr(netadr_t *to)
{
    int client; // [esp+0h] [ebp-4h]
    int clienta; // [esp+0h] [ebp-4h]

    for ( client = 0; client < 64; ++client )
    {
        if ( NET_CompareAdr(*to, s_reliableClientMessages[client].addr) )
            return client;
    }
    for ( clienta = 0; clienta < 64; ++clienta )
    {
        if ( !s_reliableClientMessages[clienta].bufferUsed )
        {
            s_reliableClientMessages[clienta].addr = *to;
            return clienta;
        }
    }
    return -1;
}

void __cdecl RMsg_Init()
{
    signed int clientSlot; // [esp+0h] [ebp-4h]

    for ( clientSlot = 0; clientSlot < 64; ++clientSlot )
        RMsg_DropAllPacketsForClient(clientSlot);
    reliableResendTime = _Dvar_RegisterInt(
                                                 "reliableResendTime",
                                                 300,
                                                 0,
                                                 0x7FFFFFFF,
                                                 0,
                                                 "Amount of time (in ms) to wait before resending an unack'd reliable message");
    reliableTimeoutTime = _Dvar_RegisterInt(
                                                    "reliableTimeoutTime",
                                                    20000,
                                                    0,
                                                    0x7FFFFFFF,
                                                    0,
                                                    "Amount of time (in ms) to wait before timing out a client for not ack'ing a message");
}

unsigned __int8 s_tempBuf[1100];
void __cdecl RMsg_SendMessages()
{
    char *v0; // eax
    char *v1; // eax
    int integer; // [esp-4h] [ebp-40h]
    reliableClient *client; // [esp+0h] [ebp-3Ch]
    signed int clientSlot; // [esp+4h] [ebp-38h]
    msg_t msg; // [esp+8h] [ebp-34h] BYREF
    int now; // [esp+38h] [ebp-4h]

    now = Sys_Milliseconds();
    for ( clientSlot = 0; clientSlot < 64; ++clientSlot )
    {
        client = &s_reliableClientMessages[clientSlot];
        if ( client->bufferUsed && now - client->lastSendTime >= reliableResendTime->current.integer )
        {
            if ( now - client->firstSendTime <= reliableTimeoutTime->current.integer )
            {
                client->lastSendTime = now;
                MSG_Init(&msg, s_tempBuf, 1100);
                RMsg_AddRMsgsToPacket(clientSlot, &msg);
                if ( !msg.cursize
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\universal\\reliablemsg.cpp",
                                437,
                                0,
                                "%s",
                                "msg.cursize") )
                {
                    __debugbreak();
                }
                if ( msg.overflowed
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\universal\\reliablemsg.cpp",
                                438,
                                0,
                                "%s",
                                "!msg.overflowed") )
                {
                    __debugbreak();
                }
                if ( client->addr.type == NA_BOT
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\universal\\reliablemsg.cpp",
                                441,
                                0,
                                "%s",
                                "client->addr.type != NA_BOT") )
                {
                    __debugbreak();
                }
                NET_OutOfBandData(NS_SERVER, client->addr, msg.data, msg.cursize);
                if ( !NET_OutOfBandData(NS_SERVER, client->addr, msg.data, msg.cursize) )
                {
                    v1 = NET_AdrToString(client->addr);
                    Com_PrintWarning(
                        16,
                        "Dropping all reliable packets for client at '%s' since we got a packet error trying to send to them\n",
                        v1);
                    RMsg_DropAllPacketsForClient(clientSlot);
                }
            }
            else
            {
                integer = reliableTimeoutTime->current.integer;
                v0 = NET_AdrToString(client->addr);
                Com_PrintWarning(
                    16,
                    "Dropping all reliable packets for client at '%s' since they haven't ack'd a reliable message in %ims\n",
                    v0,
                    integer);
                RMsg_DropAllPacketsForClient(clientSlot);
            }
        }
    }
}

void __cdecl RMsg_AddRMsgsToPacket(unsigned int clientSlot, msg_t *outMsg)
{
    reliableClient *client; // [esp+0h] [ebp-10h]
    int seq; // [esp+4h] [ebp-Ch]
    int msgSlot; // [esp+8h] [ebp-8h]
    bool headerWritten; // [esp+Fh] [ebp-1h]

    if ( clientSlot >= 0x40
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\reliablemsg.cpp",
                    369,
                    0,
                    "clientSlot doesn't index MAX_DEST_CLIENTS\n\t%i not in [0, %i)",
                    clientSlot,
                    64) )
    {
        __debugbreak();
    }
    headerWritten = 0;
    client = &s_reliableClientMessages[clientSlot];
    seq = RMsg_FindOldestSeq(clientSlot);
    if ( seq == 0x7FFFFFFF
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\reliablemsg.cpp",
                    375,
                    0,
                    "%s",
                    "seq != INT_MAX") )
    {
        __debugbreak();
    }
    while ( 1 )
    {
        msgSlot = RMsg_FindMessageSlot(clientSlot, seq);
        if ( msgSlot < 0 )
            break;
        if ( !headerWritten )
        {
            MSG_WriteString(outMsg, "R");
            headerWritten = 1;
        }
        MSG_WriteByte(outMsg, client->msgs[msgSlot].sequenceNum);
        if ( showpackets->current.integer )
            Com_Printf(
                16,
                "reliable send (%i): %s\n",
                client->msgs[msgSlot].sequenceNum,
                (const char *)client->msgs[msgSlot].msg.data);
        MSG_WriteData(outMsg, client->msgs[msgSlot].msg.data, client->msgs[msgSlot].msg.cursize);
        LOBYTE(seq) = seq + 1;
    }
}

int __cdecl RMsg_FindMessageSlot(unsigned int clientSlot, unsigned __int8 sequenceNum)
{
    reliableClient *client; // [esp+0h] [ebp-8h]
    int msgSlot; // [esp+4h] [ebp-4h]

    if ( clientSlot >= 0x40
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\reliablemsg.cpp",
                    59,
                    0,
                    "clientSlot doesn't index MAX_DEST_CLIENTS\n\t%i not in [0, %i)",
                    clientSlot,
                    64) )
    {
        __debugbreak();
    }
    client = &s_reliableClientMessages[clientSlot];
    for ( msgSlot = 0; msgSlot < 20; ++msgSlot )
    {
        if ( client->msgs[msgSlot].active && client->msgs[msgSlot].sequenceNum == sequenceNum )
            return msgSlot;
    }
    return -1;
}

int __cdecl RMsg_FindOldestSeq(unsigned int clientSlot)
{
    reliableClient *client; // [esp+0h] [ebp-Ch]
    int oldestSeq; // [esp+4h] [ebp-8h]
    int msgSlot; // [esp+8h] [ebp-4h]

    oldestSeq = 0x7FFFFFFF;
    if ( clientSlot >= 0x40
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\reliablemsg.cpp",
                    77,
                    0,
                    "clientSlot doesn't index MAX_DEST_CLIENTS\n\t%i not in [0, %i)",
                    clientSlot,
                    64) )
    {
        __debugbreak();
    }
    client = &s_reliableClientMessages[clientSlot];
    for ( msgSlot = 0; msgSlot < 20; ++msgSlot )
    {
        if ( client->msgs[msgSlot].active && client->msgs[msgSlot].sequenceNum < oldestSeq )
            oldestSeq = client->msgs[msgSlot].sequenceNum;
    }
    if ( !oldestSeq )
    {
        oldestSeq = 256;
        if ( RMsg_FindMessageSlot(clientSlot, 0xFFu) >= 0 )
        {
            do
                --oldestSeq;
            while ( RMsg_FindMessageSlot(clientSlot, oldestSeq - 1) >= 0 );
        }
        else
        {
            return 0;
        }
    }
    return oldestSeq;
}

void __cdecl RMsg_AckSequence(netadr_t *from, unsigned __int8 sequenceNum)
{
    const char *v2; // eax

    v2 = va("RA %i", sequenceNum);
    NET_OutOfBandPrint(NS_SERVER, *from, v2);
}

void __cdecl RMsg_MarkPacketReceived(netadr_t *from, unsigned __int8 sequenceNum)
{
    const char *v2; // eax
    int oldestSeq; // [esp+4h] [ebp-Ch]
    signed int clientSlot; // [esp+8h] [ebp-8h]
    signed int msgSlot; // [esp+Ch] [ebp-4h]

    clientSlot = RMsg_FindSlotForAddr(from);
    if ( clientSlot >= 0 )
    {
        if ( RMsg_FindMessageSlot(clientSlot, sequenceNum) >= 0 )
        {
            for ( oldestSeq = RMsg_FindOldestSeq(clientSlot); oldestSeq <= sequenceNum; oldestSeq = RMsg_FindOldestSeq(clientSlot) )
            {
                if ( (unsigned __int8)oldestSeq != oldestSeq
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\universal\\reliablemsg.cpp",
                                554,
                                0,
                                "%s\n\t(oldestSeq) = %i",
                                "(static_cast<byte>( oldestSeq ) == oldestSeq)",
                                oldestSeq) )
                {
                    __debugbreak();
                }
                msgSlot = RMsg_FindMessageSlot(clientSlot, oldestSeq);
                if ( msgSlot < 0 )
                {
                    v2 = va("Unable to find oldest Sequence %i", oldestSeq);
                    if ( !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\reliablemsg.cpp",
                                    556,
                                    0,
                                    "%s\n\t%s",
                                    "msgSlot >= 0",
                                    v2) )
                        __debugbreak();
                }
                RMsg_AckMsgSlot(&s_reliableClientMessages[clientSlot], clientSlot, msgSlot);
            }
        }
    }
    else
    {
        Com_PrintError(
            16,
            "Received reliable packet ack for sequence %i from unknown IP %u.%u.%u.%u\n",
            sequenceNum,
            from->ip[0],
            from->ip[1],
            from->ip[2],
            from->ip[3]);
    }
}

void __cdecl RMsg_AckMsgSlot(reliableClient *client, unsigned int clientSlot, unsigned int msgSlot)
{
    const char *v3; // eax
    int tmpSlot; // [esp+0h] [ebp-4h]

    if ( msgSlot >= 0x14
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\reliablemsg.cpp",
                    501,
                    0,
                    "msgSlot doesn't index MAX_RELIABLE_MSGS\n\t%i not in [0, %i)",
                    msgSlot,
                    20) )
    {
        __debugbreak();
    }
    if ( client->msgs[msgSlot].msg.cursize <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\reliablemsg.cpp",
                    502,
                    0,
                    "%s\n\t(client->msgs[msgSlot].msg.cursize) = %i",
                    "(client->msgs[msgSlot].msg.cursize > 0)",
                    client->msgs[msgSlot].msg.cursize) )
    {
        __debugbreak();
    }
    if ( client->bytesAckd >= 0x3E8u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\reliablemsg.cpp",
                    503,
                    0,
                    "client->bytesAckd doesn't index PER_CLIENT_BUF_SIZE\n\t%i not in [0, %i)",
                    client->bytesAckd,
                    1000) )
    {
        __debugbreak();
    }
    client->bytesAckd += client->msgs[msgSlot].msg.cursize;
    client->msgs[msgSlot].active = 0;
    for ( tmpSlot = 0; tmpSlot < 20; ++tmpSlot )
    {
        if ( client->msgs[tmpSlot].active && client->msgs[tmpSlot].msg.data - client->reliableBuf < client->bytesAckd )
        {
            v3 = va(
                         "for client %i, acking msg slot %i is changing bytesAckd from %i to %i, but active msgSlot %i's buffer would"
                         " get blown away by that because it spans from %i to %i",
                         client - s_reliableClientMessages,
                         msgSlot,
                         client->bytesAckd,
                         client->bytesAckd - client->msgs[msgSlot].msg.cursize,
                         tmpSlot,
                         client->msgs[tmpSlot].msg.data - client->reliableBuf,
                         client->msgs[tmpSlot].msg.cursize + client->msgs[tmpSlot].msg.data - client->reliableBuf);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\reliablemsg.cpp",
                            511,
                            0,
                            "%s\n\t%s",
                            "client->msgs[tmpSlot].msg.data - client->reliableBuf >= client->bytesAckd",
                            v3) )
                __debugbreak();
        }
    }
    if ( client->bytesAckd == client->bufferUsed )
    {
        client->bytesAckd = 0;
        client->bufferUsed = 0;
        if ( RMsg_CountOutstandingMessages(clientSlot) )
        {
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\reliablemsg.cpp",
                            520,
                            0,
                            "%s",
                            "RMsg_CountOutstandingMessages( clientSlot ) == 0") )
                __debugbreak();
        }
    }
}

int __cdecl RMsg_CountOutstandingMessages(unsigned int clientSlot)
{
    int total; // [esp+4h] [ebp-8h]
    int msgSlot; // [esp+8h] [ebp-4h]

    if ( clientSlot >= 0x40
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\reliablemsg.cpp",
                    481,
                    0,
                    "clientSlot doesn't index MAX_DEST_CLIENTS\n\t%i not in [0, %i)",
                    clientSlot,
                    64) )
    {
        __debugbreak();
    }
    total = 0;
    for ( msgSlot = 0; msgSlot < 20; ++msgSlot )
    {
        if ( s_reliableClientMessages[clientSlot].msgs[msgSlot].active )
            ++total;
    }
    return total;
}

