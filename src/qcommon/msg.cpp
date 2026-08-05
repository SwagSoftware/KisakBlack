#include "msg.h"
#include "common.h"
#include <client_mp/cl_main_mp.h>
#include "sv_msg_write.h"

#include <cstring>

int __cdecl MSG_ReadValueNoXor(msg_t *msg, int bits)
{
    int v3; // [esp+0h] [ebp-10h]
    int j; // [esp+4h] [ebp-Ch]
    int value; // [esp+Ch] [ebp-4h]

    if ( bits < 0 && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg.cpp", 25, 0, "%s", "bits >= 0") )
        __debugbreak();
    if ( (bits & 7) != 0 )
        v3 = MSG_ReadBits(msg, bits & 7);
    else
        v3 = 0;
    value = v3;
    for ( j = bits & 7; j < bits; j += 8 )
        value |= MSG_ReadByte(msg) << j;
    return value;
}

int __cdecl MSG_ReadValue(msg_t *msg, const int *fromF, int *toF, int bits, int size)
{
    int v6; // [esp+0h] [ebp-14h]
    int mask; // [esp+8h] [ebp-Ch]
    int value; // [esp+10h] [ebp-4h]
    int valuea; // [esp+10h] [ebp-4h]

    if ( bits < 0 )
        v6 = -bits;
    else
        v6 = bits;
    value = MSG_ReadValueNoXor(msg, v6);
    if ( v6 > 32
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg.cpp",
                    48,
                    0,
                    "%s\n\t(absbits) = %i",
                    "(absbits <= 32)",
                    v6) )
    {
        __debugbreak();
    }
    if ( v6 == 32 )
        mask = -1;
    else
        mask = (1 << v6) - 1;
    valuea = value ^ mask & MSG_GetField(fromF, size);
    if ( bits < 0 && (valuea & (1 << (v6 - 1))) != 0 )
        return valuea | ~mask;
    return valuea;
}

int __cdecl MSG_GetField(const int *i, int size)
{
    switch ( size )
    {
        case 1:
            return *(char *)i;
        case 2:
            return *(__int16 *)i;
        case 4:
            return *i;
    }
    if ( !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\qcommon\\../qcommon/msg.h", 547, 0, "unknown field size") )
        __debugbreak();
    return 0;
}

double __cdecl MSG_ReadDeltaAngle(msg_t *msg, float oldFloat)
{
    unsigned int MinBitCountForNum; // eax
    __int16 Bits; // [esp+0h] [ebp-20h]
    __int16 delta; // [esp+8h] [ebp-18h]
    __int16 sign; // [esp+Ch] [ebp-14h]
    unsigned int index; // [esp+14h] [ebp-Ch]

    sign = 1;
    if ( MSG_ReadBit(msg) == 1 )
    {
        if ( MSG_ReadBit(msg) )
            sign = -1;
        MinBitCountForNum = GetMinBitCountForNum(7u);
        index = MSG_ReadBits(msg, MinBitCountForNum);
        if ( index >= 7
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg.cpp",
                        82,
                        0,
                        "index doesn't index COMMON_ANGLE_DELTA_ARRAYCOUNT\n\t%i not in [0, %i)",
                        index,
                        7) )
        {
            __debugbreak();
        }
        delta = g_commonAngleDeltas[index];
        if ( !delta && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg.cpp", 84, 0, "%s", "delta") )
            __debugbreak();
        return (float)((float)(__int16)(delta * sign + (int)(float)(oldFloat * 182.04445)) * 0.0054931641);
    }
    else if ( MSG_ReadBit(msg) )
    {
        if ( MSG_ReadBit(msg) )
            sign = -1;
        Bits = MSG_ReadBits(msg, 0xCu);
        if ( !Bits && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg.cpp", 106, 0, "%s", "delta") )
            __debugbreak();
        return (float)((float)(__int16)(Bits * sign + (int)(float)(oldFloat * 182.04445)) * 0.0054931641);
    }
    else
    {
        return (float)MSG_ReadAngle16(msg);
    }
}

int __cdecl MSG_ReadLastChangedField(msg_t *msg, int totalFields)
{
    const char *v2; // eax
    int lastChanged; // [esp+0h] [ebp-8h]
    unsigned int idealBits; // [esp+4h] [ebp-4h]

    idealBits = GetMinBitCountForNum(totalFields);
    lastChanged = MSG_ReadBits(msg, idealBits);
    if ( lastChanged > totalFields )
    {
        v2 = va("lastChanged was %i, totalFields is %i\n", lastChanged, totalFields);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg.cpp",
                        133,
                        0,
                        "%s\n\t%s",
                        "lastChanged <= totalFields",
                        v2) )
            __debugbreak();
    }
    return lastChanged;
}

int __cdecl MSG_ReadNumFieldsSkipped(msg_t *msg, int skippedFieldBits, int maxVal)
{
    int numSkipped; // [esp+0h] [ebp-8h]
    int numSkippeda; // [esp+0h] [ebp-8h]
    int bits; // [esp+4h] [ebp-4h]

    numSkipped = 0;
    if ( skippedFieldBits == 1 )
    {
        do
            ++numSkipped;
        while ( !MSG_ReadBit(msg) );
        if ( maxVal < numSkipped
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg.cpp", 170, 0, "%s", "maxVal >= numSkipped") )
        {
            __debugbreak();
        }
    }
    else
    {
        if ( MSG_ReadBit(msg) )
            return 1;
        numSkippeda = 1;
        bits = MSG_ReadBits(msg, skippedFieldBits);
        if ( bits > (1 << skippedFieldBits) - 1
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg.cpp",
                        204,
                        0,
                        "%s",
                        "bits <= ( 1 << skippedFieldBits ) - 1") )
        {
            __debugbreak();
        }
        while ( bits == (1 << skippedFieldBits) - 1 )
        {
            numSkippeda = numSkippeda + (1 << skippedFieldBits) - 1;
            bits = MSG_ReadBits(msg, skippedFieldBits);
            if ( bits > (1 << skippedFieldBits) - 1
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg.cpp",
                            216,
                            0,
                            "%s",
                            "bits <= ( 1 << skippedFieldBits ) - 1") )
            {
                __debugbreak();
            }
        }
        return bits + numSkippeda;
    }
    return numSkipped;
}

void __cdecl MSG_CopyFieldOver(const NetField *stateFields, char *from, char *to, int fieldNum)
{
    int size; // [esp+0h] [ebp-18h]
    int Field; // [esp+4h] [ebp-14h]
    int *toF; // [esp+10h] [ebp-8h]

    toF = (int *)&to[stateFields[fieldNum].offset];
    Field = MSG_GetField((const int *)&from[stateFields[fieldNum].offset], stateFields[fieldNum].size);
    size = stateFields[fieldNum].size;
    switch ( size )
    {
        case 1:
            *(_BYTE *)toF = Field;
            break;
        case 2:
            *(_WORD *)toF = Field;
            break;
        case 4:
            *toF = Field;
            break;
    }
}

void __cdecl MSG_ReadDeltaFields(
                msg_t *msg,
                int time,
                char *from,
                char *to,
                int numFields,
                const NetField *stateFields,
                int skippedFieldBits)
{
    int NumFieldsSkipped; // eax
    const char *v8; // eax
    const char *v9; // eax
    int lastChanged; // [esp+0h] [ebp-20h]
    int nextChanged; // [esp+4h] [ebp-1Ch]
    int lc; // [esp+14h] [ebp-Ch]
    int i; // [esp+1Ch] [ebp-4h]
    int ia; // [esp+1Ch] [ebp-4h]
    int ib; // [esp+1Ch] [ebp-4h]
    int ic; // [esp+1Ch] [ebp-4h]

    if ( MSG_ReadBit(msg) )
    {
        lastChanged = -1;
        nextChanged = -1;
        lc = MSG_ReadLastChangedField(msg, numFields);
        if ( lc <= numFields )
        {
            while ( nextChanged != lc )
            {
                NumFieldsSkipped = MSG_ReadNumFieldsSkipped(msg, skippedFieldBits, lc - lastChanged);
                nextChanged = lastChanged + NumFieldsSkipped;
                if ( lastChanged + NumFieldsSkipped <= lastChanged )
                {
                    v8 = va("lastChanged was %i, nextChanged is %i", lastChanged, nextChanged);
                    if ( !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg.cpp",
                                    327,
                                    0,
                                    "%s\n\t%s",
                                    "nextChanged > lastChanged",
                                    v8) )
                        __debugbreak();
                }
                if ( lc < nextChanged )
                {
                    v9 = va("nextChanged is %i, lc is %i", nextChanged, lc);
                    if ( !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg.cpp",
                                    328,
                                    0,
                                    "%s\n\t%s",
                                    "static_cast<int>( lc ) >= nextChanged",
                                    v9) )
                        __debugbreak();
                }
                for ( i = lastChanged + 1; i < nextChanged; ++i )
                    MSG_CopyFieldOver(stateFields, from, to, i);
                MSG_ReadDeltaField(msg, time, from, to, &stateFields[nextChanged], 0, 0);
                lastChanged = nextChanged;
            }
            for ( ia = 0; ia < lc; ++ia )
            {
                if ( stateFields[ia].changeHints == 2 )
                    MSG_ReadDeltaField(msg, time, from, to, &stateFields[ia], 0, 0);
            }
            for ( ib = lc + 1; ib < numFields; ++ib )
                MSG_CopyFieldOver(stateFields, from, to, ib);
        }
        else
        {
            Com_PrintError(30, "Got lastChanged field of %i, but there are only %i fields\n", lc, numFields);
            msg->overflowed = 1;
        }
    }
    else
    {
        for ( ic = 0; ic < numFields; ++ic )
            MSG_CopyFieldOver(stateFields, from, to, ic);
    }
}

int __cdecl MSG_ReadDeltaStruct(
                msg_t *msg,
                int time,
                char *from,
                char *to,
                unsigned int number,
                int numFields,
                char indexBits,
                const NetField *stateFields,
                int skippedFieldBits)
{
    if ( number >= 1 << indexBits
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\msg.cpp",
                    392,
                    0,
                    "%s",
                    "number < (1u << indexBits)") )
    {
        __debugbreak();
    }
    if ( MSG_ReadBit(msg) == 1 )
    {
        if ( cl_shownet && (cl_shownet->current.integer >= 2 || cl_shownet->current.integer == -1) )
            Com_Printf(30, "%3i: #%-3i remove\n", msg->readcount, number);
        return 1;
    }
    else
    {
        *(unsigned int *)to = number;
        MSG_ReadDeltaFields(msg, time, from, to, numFields, stateFields, skippedFieldBits);
        return 0;
    }
}

// local variable allocation has failed, the output may be wrong!
void    MSG_ReadDeltaMatchState(msg_t *msg, int time, MatchState *from, MatchState *to)
{
    bool IsPlaying; // al
    bool v6; // al
    unsigned __int8 v7[128]; // [esp-Ch] [ebp-10Ch] OVERLAPPED BYREF
    unsigned int count; // [esp+E8h] [ebp-18h]
    const NetField *array; // [esp+ECh] [ebp-14h]
    unsigned int EntityIndex; // [esp+F0h] [ebp-10h]
    //int numFields; // [esp+F4h] [ebp-Ch]
    //const NetField *fields; // [esp+F8h] [ebp-8h]
    const NetField *retaddr; // [esp+100h] [ebp+0h]

    //numFields = a1;
    //fields = retaddr;
    EntityIndex = MSG_ReadEntityIndex(msg, 1u);
    IsPlaying = Demo_IsPlaying();
    array = MSG_GetNetFieldList(NET_FIELD_TYPE_MATCHSTATE, IsPlaying)->array;
    v6 = Demo_IsPlaying();
    count = MSG_GetNetFieldList(NET_FIELD_TYPE_MATCHSTATE, v6)->count;
    if ( from )
    {
        MSG_ReadDeltaStruct(msg, time, (char *)from, (char *)to, EntityIndex, count, 1, array, 1);
    }
    else
    {
        memset(v7, 0, sizeof(v7));
        MSG_ReadDeltaStruct(msg, time, (char *)v7, (char *)to, EntityIndex, count, 1, array, 1);
    }
}

int __cdecl MSG_ReadDeltaClient(msg_t *msg, int time, clientState_s *from, clientState_s *to, unsigned int number)
{
    bool IsPlaying; // al
    bool v6; // al
    unsigned int numFields; // [esp+0h] [ebp-DCh]
    clientState_s dummy; // [esp+4h] [ebp-D8h] BYREF
    const NetField *fields; // [esp+D8h] [ebp-4h]

    IsPlaying = Demo_IsPlaying();
    fields = MSG_GetNetFieldList(NET_FIELD_TYPE_CLIENT_STATE, IsPlaying)->array;
    v6 = Demo_IsPlaying();
    numFields = MSG_GetNetFieldList(NET_FIELD_TYPE_CLIENT_STATE, v6)->count;
    if ( !from )
    {
        from = &dummy;
        memset(&dummy, 0, sizeof(dummy));
    }
    return MSG_ReadDeltaStruct(msg, time, (char *)from, (char *)to, number, numFields, 5, fields, 1);
}

int __cdecl MSG_ReadDeltaEntity(msg_t *msg, int time, entityState_s *from, entityState_s *to, unsigned int number)
{
    return MSG_ReadDeltaEntityStruct(msg, time, (char *)from, (char *)to, number, maxStateFieldCount, 3);
}

