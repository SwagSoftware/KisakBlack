#include "cscr_memorytree.h"
#include <cstring>
#include <qcommon/common.h>
#include "cscr_stringlist.h"
#include <win32/win_common.h>
#include <qcommon/threads.h>
#include "cscr_vm.h"

const char *mt_type_names[23] =
{
  "empty",
  "thread",
  "vector",
  "notetrack",
  "anim tree",
  "small anim tree",
  "external",
  "temp",
  "surface",
  "anim part",
  "model part",
  "model part map",
  "duplicate parts",
  "model list",
  "script parse",
  "script string",
  "class",
  "tag info",
  "animscripted",
  "config string",
  "debugger string",
  "vehicle",
  "generic"
};


scrMemTreeGlob_t gScrMemTreeGlob[2];
scrMemTreePub_t gScrMemTreePub[2];
scrMemTreeDebugGlob_t gScrMemTreeDebugGlob[2];

char byte_9D4A088[196608];

void __cdecl MT_DumpTree(scriptInstance_t inst)
{
    const char *v1; // eax
    int subTreeSize; // [esp+8h] [ebp-80h]
    unsigned int nodeNum; // [esp+Ch] [ebp-7Ch]
    int size; // [esp+10h] [ebp-78h]
    int totalAllocBuckets; // [esp+18h] [ebp-70h]
    int totalAlloc; // [esp+1Ch] [ebp-6Ch]
    int mt_type_usage[23]; // [esp+20h] [ebp-68h] BYREF
    int type; // [esp+80h] [ebp-8h]
    int totalBuckets; // [esp+84h] [ebp-4h]

    memset(mt_type_usage, 0, sizeof(mt_type_usage));
    Com_Printf(24, "********************************\n");
    totalAlloc = 0;
    totalAllocBuckets = 0;
    for (nodeNum = 0; nodeNum < 0x10000; ++nodeNum)
    {
        type = gScrMemTreeDebugGlob[inst].mt_usage[nodeNum];
        if (type)
        {
            v1 = MT_NodeInfoString(inst, nodeNum);
            Com_Printf(24, "%s\n", v1);
            ++totalAlloc;
            totalAllocBuckets += 1 << byte_9D4A088[0x20000 * inst + nodeNum];
            mt_type_usage[type] += 1 << byte_9D4A088[0x20000 * inst + nodeNum];
        }
    }
    if (gScrMemTreeGlob[inst].totalAlloc != totalAlloc
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_memorytree.cpp",
            164,
            0,
            "%s",
            "gScrMemTreeGlob[inst].totalAlloc == totalAlloc"))
    {
        __debugbreak();
    }
    if (gScrMemTreeGlob[inst].totalAllocBuckets != totalAllocBuckets
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_memorytree.cpp",
            165,
            0,
            "%s",
            "gScrMemTreeGlob[inst].totalAllocBuckets == totalAllocBuckets"))
    {
        __debugbreak();
    }
    Com_Printf(24, "********************************\n");
    totalBuckets = gScrMemTreeGlob[inst].totalAllocBuckets;
    for (size = 0; size <= 16; ++size)
    {
        subTreeSize = MT_GetSubTreeSize(inst, gScrMemTreeGlob[inst].head[size]);
        totalBuckets += subTreeSize * (1 << size);
        Com_Printf(
            24,
            "%d subtree has %d * %d = %d free buckets\n",
            size,
            subTreeSize,
            1 << size,
            subTreeSize * (1 << size));
    }
    Com_Printf(24, "********************************\n");
    for (type = 1; type < 23; ++type)
        Com_Printf(24, "'%s' allocated: %d\n", mt_type_names[type], mt_type_usage[type]);
    Com_Printf(24, "********************************\n");
    Com_Printf(
        24,
        "total memory alloc buckets: %d (%d instances)\n",
        gScrMemTreeGlob[inst].totalAllocBuckets,
        gScrMemTreeGlob[inst].totalAlloc);
    Com_Printf(24, "total memory free buckets: %d\n", 0xFFFF - gScrMemTreeGlob[inst].totalAllocBuckets);
    Com_Printf(24, "********************************\n");
    Com_Printf(
        24,
        "avg alloc: %f\n",
        (float)((float)gScrMemTreeGlob[inst].avgAllocBytes / (float)gScrMemTreeGlob[inst].avgAlloc));
    if (totalBuckets != 0xFFFF
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_memorytree.cpp",
            206,
            0,
            "%s",
            "totalBuckets == (1 << MEMORY_NODE_BITS) - 1"))
    {
        __debugbreak();
    }
}
int __cdecl MT_GetSubTreeSize(scriptInstance_t inst, int nodeNum)
{
    int SubTreeSize; // esi

    if ( !nodeNum )
        return 0;
    SubTreeSize = MT_GetSubTreeSize(inst, gScrMemTreeGlob[inst].nodes[nodeNum].prev);
    return SubTreeSize + MT_GetSubTreeSize(inst, gScrMemTreeGlob[inst].nodes[nodeNum].next) + 1;
}

const char *__cdecl MT_NodeInfoString(scriptInstance_t inst, unsigned int nodeNum)
{
    int v4; // [esp-4h] [ebp-Ch]
    int type; // [esp+4h] [ebp-4h]

    type = gScrMemTreeDebugGlob[inst].mt_usage[nodeNum];
    if ( !gScrMemTreeDebugGlob[inst].mt_usage[nodeNum] )
        return "<FREE>";
    v4 = (unsigned __int8)byte_9D4A088[0x20000 * inst + nodeNum];
    return va("%s: '%s' (%d)", mt_type_names[type], SL_DebugConvertToString(nodeNum, inst), v4);
}

int __cdecl Scr_GetStringUsage(scriptInstance_t inst)
{
    return gScrMemTreeGlob[inst].totalAllocBuckets;
}

void __cdecl MT_Init(scriptInstance_t inst)
{
    int i; // [esp+0h] [ebp-4h]
    signed int ia; // [esp+0h] [ebp-4h]

    Sys_EnterCriticalSection(CRITSECT_MEMORY_TREE);
    gScrMemTreePub[inst].mt_buffer = (char *)&gScrMemTreeGlob[inst];
    MT_InitBits(inst);
    for (i = 0; i <= 16; ++i)
        gScrMemTreeGlob[inst].head[i] = 0;
    gScrMemTreeGlob[inst].nodes[0].prev = 0;
    gScrMemTreeGlob[inst].nodes[0].next = 0;
    for (ia = 0; ia < 16; ++ia)
        MT_AddMemoryNode(inst, 1 << ia, ia);
    gScrMemTreeGlob[inst].totalAlloc = 0;
    gScrMemTreeGlob[inst].totalAllocBuckets = 0;
    memset(gScrMemTreeDebugGlob[inst].mt_usage, 0, sizeof(gScrMemTreeDebugGlob[inst].mt_usage));
    memset(&byte_9D4A088[0x20000 * inst], 0, 0x10000u);
    Sys_LeaveCriticalSection(CRITSECT_MEMORY_TREE);
}

void __cdecl MT_InitBits(scriptInstance_t inst)
{
    unsigned __int8 bits; // [esp+0h] [ebp-Ch]
    unsigned __int8 bitsa; // [esp+0h] [ebp-Ch]
    unsigned __int8 bitsb; // [esp+0h] [ebp-Ch]
    int temp; // [esp+4h] [ebp-8h]
    int tempa; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    for (i = 0; i < 256; ++i)
    {
        bits = 0;
        for (temp = i; temp; temp >>= 1)
        {
            if ((temp & 1) != 0)
                ++bits;
        }
        gScrMemTreeGlob[inst].numBits[i] = bits;
        for (bitsa = 8; (i & ((1 << bitsa) - 1)) != 0; --bitsa)
            ;
        gScrMemTreeGlob[inst].leftBits[i] = bitsa;
        bitsb = 0;
        for (tempa = i; tempa; tempa >>= 1)
            ++bitsb;
        gScrMemTreeGlob[inst].logBits[i] = bitsb;
    }
}

void __cdecl MT_AddMemoryNode(scriptInstance_t inst, int newNode, unsigned int size)
{
    MemoryNode *v3; // edx
    MemoryNode *v4; // eax
    int node; // [esp+0h] [ebp-18h]
    int nodeNum; // [esp+4h] [ebp-14h]
    int newScore; // [esp+8h] [ebp-10h]
    unsigned __int16 *parentNode; // [esp+Ch] [ebp-Ch]
    int level; // [esp+10h] [ebp-8h]
    int score; // [esp+14h] [ebp-4h]

    if (size > 0x10
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_memorytree.cpp",
            290,
            0,
            "%s",
            "size >= 0 && size <= MEMORY_NODE_BITS"))
    {
        __debugbreak();
    }
    parentNode = &gScrMemTreeGlob[inst].head[size];
    node = *parentNode;
    if (*parentNode)
    {
        newScore = MT_GetScore(inst, newNode);
        nodeNum = 0;
        level = 0x10000;
        while (1)
        {
            if (newNode == node
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_memorytree.cpp",
                    303,
                    0,
                    "%s",
                    "newNode != node"))
            {
                __debugbreak();
            }
            score = MT_GetScore(inst, node);
            if (score == newScore
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_memorytree.cpp",
                    306,
                    0,
                    "%s",
                    "score != newScore"))
            {
                __debugbreak();
            }
            if (score < newScore)
                break;
            level >>= 1;
            if (newNode == nodeNum
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_memorytree.cpp",
                    340,
                    0,
                    "%s",
                    "newNode != nodeNum"))
            {
                __debugbreak();
            }
            if (newNode >= nodeNum)
            {
                parentNode = &gScrMemTreeGlob[inst].nodes[node].next;
                nodeNum += level;
            }
            else
            {
                parentNode = &gScrMemTreeGlob[inst].nodes[node].prev;
                nodeNum -= level;
            }
            node = *parentNode;
            if (!*parentNode)
                goto LABEL_34;
        }
        while (1)
        {
            if (node != *parentNode
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_memorytree.cpp",
                    312,
                    0,
                    "%s",
                    "node == *parentNode"))
            {
                __debugbreak();
            }
            if (node == newNode
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_memorytree.cpp",
                    313,
                    0,
                    "%s",
                    "node != newNode"))
            {
                __debugbreak();
            }
            *parentNode = newNode;
            v3 = &gScrMemTreeGlob[inst].nodes[node];
            v4 = &gScrMemTreeGlob[inst].nodes[newNode];
            *(_DWORD *)&v4->prev = *(_DWORD *)&v3->prev;
            v4->padding[0] = v3->padding[0];
            v4->padding[1] = v3->padding[1];
            v4->padding[2] = v3->padding[2];
            if (!node)
                break;
            level >>= 1;
            if (node == nodeNum
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_memorytree.cpp",
                    322,
                    0,
                    "%s",
                    "node != nodeNum"))
            {
                __debugbreak();
            }
            if (node >= nodeNum)
            {
                parentNode = &gScrMemTreeGlob[inst].nodes[newNode].next;
                nodeNum += level;
            }
            else
            {
                parentNode = &gScrMemTreeGlob[inst].nodes[newNode].prev;
                nodeNum -= level;
            }
            newNode = node;
            node = *parentNode;
        }
    }
    else
    {
    LABEL_34:
        *parentNode = newNode;
        gScrMemTreeGlob[inst].nodes[newNode].prev = 0;
        gScrMemTreeGlob[inst].nodes[newNode].next = 0;
    }
}

int __cdecl MT_GetScore(scriptInstance_t inst, int num)
{
    unsigned __int8 bits; // [esp+8h] [ebp-8h]
    int numa; // [esp+1Ch] [ebp+Ch]

    if (!num
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_memorytree.cpp", 261, 0, "%s", "num"))
    {
        __debugbreak();
    }
    numa = 0x10000 - num;
    if (!numa
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_memorytree.cpp", 263, 0, "%s", "num"))
    {
        __debugbreak();
    }
    bits = gScrMemTreeGlob[inst].leftBits[(unsigned __int8)numa];
    if (!(_BYTE)numa)
        bits += gScrMemTreeGlob[inst].leftBits[BYTE1(numa)];
    return numa
        - (gScrMemTreeGlob[inst].numBits[BYTE1(numa)]
            + gScrMemTreeGlob[inst].numBits[(unsigned __int8)numa])
        + (1 << bits);
}

unsigned __int16 __cdecl MT_AllocIndex(scriptInstance_t inst, int numBytes, int type)
{
    const char *v3; // eax
    const char *v4; // eax
    unsigned int nodeNum; // [esp+0h] [ebp-Ch]
    unsigned int size; // [esp+4h] [ebp-8h]
    signed int newSize; // [esp+8h] [ebp-4h]

    size = MT_GetSize(inst, numBytes);
    if (size > 0x10
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_memorytree.cpp",
            599,
            0,
            "%s",
            "size >= 0 && size <= MEMORY_NODE_BITS"))
    {
        __debugbreak();
    }
    Sys_EnterCriticalSection(CRITSECT_MEMORY_TREE);
    for (newSize = size; ; ++newSize)
    {
        if (newSize > 16)
        {
            Sys_LeaveCriticalSection(CRITSECT_MEMORY_TREE);
            MT_Error(inst, "MT_AllocIndex", numBytes);
            return 0;
        }
        nodeNum = gScrMemTreeGlob[inst].head[newSize];
        if (gScrMemTreeGlob[inst].head[newSize])
            break;
    }
    MT_RemoveHeadMemoryNode(inst, newSize);
    while (newSize != size)
    {
        --newSize;
        MT_AddMemoryNode(inst, nodeNum + (1 << newSize), newSize);
    }
    ++gScrMemTreeGlob[inst].totalAlloc;
    ++gScrMemTreeGlob[inst].avgAlloc;
    gScrMemTreeGlob[inst].avgAllocBytes += numBytes;
    gScrMemTreeGlob[inst].totalAllocBuckets += 1 << size;
    if (!type
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_memorytree.cpp", 630, 0, "%s", "type"))
    {
        __debugbreak();
    }
    if (gScrMemTreeDebugGlob[inst].mt_usage[nodeNum])
    {
        v3 = MT_NodeInfoString(inst, nodeNum);
        if (!Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_memorytree.cpp",
            631,
            0,
            "%s\n\t(MT_NodeInfoString( inst, nodeNum )) = %s",
            "(!gScrMemTreeDebugGlob[inst].mt_usage[nodeNum])",
            v3))
            __debugbreak();
    }
    if (byte_9D4A088[0x20000 * inst + nodeNum])
    {
        v4 = MT_NodeInfoString(inst, nodeNum);
        if (!Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_memorytree.cpp",
            632,
            0,
            "%s\n\t(MT_NodeInfoString( inst, nodeNum )) = %s",
            "(!gScrMemTreeDebugGlob[inst].mt_usage_size[nodeNum])",
            v4))
            __debugbreak();
    }
    gScrMemTreeDebugGlob[inst].mt_usage[nodeNum] = type;
    byte_9D4A088[0x20000 * inst + nodeNum] = size;
    Sys_LeaveCriticalSection(CRITSECT_MEMORY_TREE);
    return nodeNum;
}

void __cdecl MT_RemoveHeadMemoryNode(scriptInstance_t inst, unsigned int size)
{
    MemoryNode *v2; // ecx
    MemoryNode *v3; // ecx
    int oldNode; // [esp+0h] [ebp-30h]
    int tempNodeValue; // [esp+4h] [ebp-2Ch]
    unsigned int tempNodeValue_4; // [esp+8h] [ebp-28h]
    unsigned int tempNodeValue_8; // [esp+Ch] [ebp-24h]
    unsigned int tempNodeValue_12; // [esp+10h] [ebp-20h]
    int oldNodeValue; // [esp+14h] [ebp-1Ch]
    unsigned int oldNodeValue_4; // [esp+18h] [ebp-18h]
    unsigned int oldNodeValue_8; // [esp+1Ch] [ebp-14h]
    unsigned int oldNodeValue_12; // [esp+20h] [ebp-10h]
    unsigned __int16 *parentNode; // [esp+24h] [ebp-Ch]
    int prevScore; // [esp+28h] [ebp-8h]
    int nextScore; // [esp+2Ch] [ebp-4h]

    if (size > 0x10
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_memorytree.cpp",
            460,
            0,
            "%s",
            "size >= 0 && size <= MEMORY_NODE_BITS"))
    {
        __debugbreak();
    }
    parentNode = &gScrMemTreeGlob[inst].head[size];
    v2 = &gScrMemTreeGlob[inst].nodes[*parentNode];
    oldNodeValue = *(_DWORD *)&v2->prev;
    oldNodeValue_4 = v2->padding[0];
    oldNodeValue_8 = v2->padding[1];
    oldNodeValue_12 = v2->padding[2];
    while ((_WORD)oldNodeValue)
    {
        if (HIWORD(oldNodeValue))
        {
            prevScore = MT_GetScore(inst, (unsigned __int16)oldNodeValue);
            nextScore = MT_GetScore(inst, HIWORD(oldNodeValue));
            if (prevScore == nextScore
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_memorytree.cpp",
                    489,
                    0,
                    "%s",
                    "prevScore != nextScore"))
            {
                __debugbreak();
            }
            if (prevScore >= nextScore)
            {
                oldNode = (unsigned __int16)oldNodeValue;
                *parentNode = oldNodeValue;
                parentNode = &gScrMemTreeGlob[inst].nodes[(unsigned __int16)oldNodeValue].prev;
            }
            else
            {
                oldNode = HIWORD(oldNodeValue);
                *parentNode = HIWORD(oldNodeValue);
                parentNode = &gScrMemTreeGlob[inst].nodes[HIWORD(oldNodeValue)].next;
            }
        }
        else
        {
            oldNode = (unsigned __int16)oldNodeValue;
            *parentNode = oldNodeValue;
            parentNode = &gScrMemTreeGlob[inst].nodes[(unsigned __int16)oldNodeValue].prev;
        }
    LABEL_16:
        if (!oldNode
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_memorytree.cpp",
                504,
                0,
                "%s",
                "oldNode"))
        {
            __debugbreak();
        }
        tempNodeValue = oldNodeValue;
        tempNodeValue_4 = oldNodeValue_4;
        tempNodeValue_8 = oldNodeValue_8;
        tempNodeValue_12 = oldNodeValue_12;
        v3 = &gScrMemTreeGlob[inst].nodes[oldNode];
        oldNodeValue = *(_DWORD *)&v3->prev;
        oldNodeValue_4 = v3->padding[0];
        oldNodeValue_8 = v3->padding[1];
        oldNodeValue_12 = v3->padding[2];
        *(_DWORD *)&v3->prev = tempNodeValue;
        v3->padding[0] = tempNodeValue_4;
        v3->padding[1] = tempNodeValue_8;
        v3->padding[2] = tempNodeValue_12;
    }
    oldNode = HIWORD(oldNodeValue);
    *parentNode = HIWORD(oldNodeValue);
    if (HIWORD(oldNodeValue))
    {
        parentNode = &gScrMemTreeGlob[inst].nodes[HIWORD(oldNodeValue)].next;
        goto LABEL_16;
    }
}
void __cdecl MT_Error(scriptInstance_t inst, const char *funcName, int numBytes)
{
    MT_DumpTree(inst);
    Com_Printf(24, "%s: failed memory allocation of %d bytes for script usage\n", funcName, numBytes);
    if ( Sys_IsServerThread() )
        Scr_TerminalError("failed memory allocation for script usage", inst);
    else
        Com_Error(ERR_DROP, "%s: failed allocation of %d bytes for script usage", funcName, numBytes);
}

int __cdecl MT_GetSize(scriptInstance_t inst, int numBytes)
{
    int numBuckets; // [esp+4h] [ebp-4h]

    if (numBytes <= 0
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_memorytree.cpp",
            574,
            0,
            "%s",
            "numBytes > 0"))
    {
        __debugbreak();
    }
    if (numBytes >= 0x10000)
    {
        MT_Error(inst, "MT_GetSize: max allocation exceeded", numBytes);
        return 0;
    }
    else
    {
        numBuckets = (numBytes + 15) / 16 - 1;
        if (numBuckets > 255)
            return gScrMemTreeGlob[inst].logBits[numBuckets >> 8] + 8;
        else
            return gScrMemTreeGlob[inst].logBits[numBuckets];
    }
}

void __cdecl MT_FreeIndex(scriptInstance_t inst, unsigned int nodeNum, int numBytes)
{
    const char *v3; // eax
    signed int size; // [esp+0h] [ebp-8h]
    int lowBit; // [esp+4h] [ebp-4h]

    size = MT_GetSize(inst, numBytes);
    if ((unsigned int)size > 0x10
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_memorytree.cpp",
            666,
            0,
            "%s",
            "size >= 0 && size <= MEMORY_NODE_BITS"))
    {
        __debugbreak();
    }
    if ((!nodeNum || nodeNum >= 0x10000)
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_memorytree.cpp",
            667,
            0,
            "%s",
            "nodeNum > 0 && nodeNum < MEMORY_NODE_COUNT"))
    {
        __debugbreak();
    }
    Sys_EnterCriticalSection(CRITSECT_MEMORY_TREE);
    --gScrMemTreeGlob[inst].totalAlloc;
    gScrMemTreeGlob[inst].totalAllocBuckets -= 1 << size;
    if (!gScrMemTreeDebugGlob[inst].mt_usage[nodeNum]
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_memorytree.cpp",
            683,
            0,
            "%s",
            "gScrMemTreeDebugGlob[inst].mt_usage[nodeNum]"))
    {
        __debugbreak();
    }
    if ((unsigned __int8)byte_9D4A088[0x20000 * inst + nodeNum] != size)
    {
        v3 = MT_NodeInfoString(inst, nodeNum);
        if (!Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_memorytree.cpp",
            684,
            0,
            "%s\n\t(MT_NodeInfoString( inst, nodeNum )) = %s",
            "(gScrMemTreeDebugGlob[inst].mt_usage_size[nodeNum] == size)",
            v3))
            __debugbreak();
    }
    gScrMemTreeDebugGlob[inst].mt_usage[nodeNum] = 0;
    byte_9D4A088[0x20000 * inst + nodeNum] = 0;
    while (1)
    {
        if (size > 16
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_memorytree.cpp",
                691,
                0,
                "%s",
                "size <= MEMORY_NODE_BITS"))
        {
            __debugbreak();
        }
        lowBit = 1 << size;
        if (nodeNum != (nodeNum & ~((1 << size) - 1))
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_memorytree.cpp",
                694,
                0,
                "%s",
                "nodeNum == (nodeNum & ~(lowBit - 1))"))
        {
            __debugbreak();
        }
        if (size == 16 || !MT_RemoveMemoryNode(inst, lowBit ^ nodeNum, size))
            break;
        nodeNum &= ~lowBit;
        ++size;
    }
    MT_AddMemoryNode(inst, nodeNum, size);
    Sys_LeaveCriticalSection(CRITSECT_MEMORY_TREE);
}

char __cdecl MT_RemoveMemoryNode(scriptInstance_t inst, int oldNode, unsigned int size)
{
    MemoryNode *v3; // ecx
    MemoryNode *v5; // ecx
    int node; // [esp+0h] [ebp-38h]
    int tempNodeValue; // [esp+4h] [ebp-34h]
    unsigned int tempNodeValue_4; // [esp+8h] [ebp-30h]
    unsigned int tempNodeValue_8; // [esp+Ch] [ebp-2Ch]
    unsigned int tempNodeValue_12; // [esp+10h] [ebp-28h]
    int oldNodeValue; // [esp+14h] [ebp-24h]
    unsigned int oldNodeValue_4; // [esp+18h] [ebp-20h]
    unsigned int oldNodeValue_8; // [esp+1Ch] [ebp-1Ch]
    unsigned int oldNodeValue_12; // [esp+20h] [ebp-18h]
    int nodeNum; // [esp+24h] [ebp-14h]
    unsigned __int16 *parentNode; // [esp+28h] [ebp-10h]
    int prevScore; // [esp+2Ch] [ebp-Ch]
    int nextScore; // [esp+30h] [ebp-8h]
    int level; // [esp+34h] [ebp-4h]
    int oldNodea; // [esp+44h] [ebp+Ch]

    if (size > 0x10
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_memorytree.cpp",
            372,
            0,
            "%s",
            "size >= 0 && size <= MEMORY_NODE_BITS"))
    {
        __debugbreak();
    }
    nodeNum = 0;
    level = 0x10000;
    parentNode = &gScrMemTreeGlob[inst].head[size];
    for (node = *parentNode; node; node = *parentNode)
    {
        if (oldNode == node)
        {
            v3 = &gScrMemTreeGlob[inst].nodes[oldNode];
            oldNodeValue = *(_DWORD *)&v3->prev;
            oldNodeValue_4 = v3->padding[0];
            oldNodeValue_8 = v3->padding[1];
            oldNodeValue_12 = v3->padding[2];
            while (1)
            {
                if ((_WORD)oldNodeValue)
                {
                    if (HIWORD(oldNodeValue))
                    {
                        prevScore = MT_GetScore(inst, (unsigned __int16)oldNodeValue);
                        nextScore = MT_GetScore(inst, HIWORD(oldNodeValue));
                        if (prevScore == nextScore
                            && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_memorytree.cpp",
                                407,
                                0,
                                "%s",
                                "prevScore != nextScore"))
                        {
                            __debugbreak();
                        }
                        if (prevScore >= nextScore)
                        {
                            oldNodea = (unsigned __int16)oldNodeValue;
                            *parentNode = oldNodeValue;
                            parentNode = &gScrMemTreeGlob[inst].nodes[(unsigned __int16)oldNodeValue].prev;
                        }
                        else
                        {
                            oldNodea = HIWORD(oldNodeValue);
                            *parentNode = HIWORD(oldNodeValue);
                            parentNode = &gScrMemTreeGlob[inst].nodes[HIWORD(oldNodeValue)].next;
                        }
                    }
                    else
                    {
                        oldNodea = (unsigned __int16)oldNodeValue;
                        *parentNode = oldNodeValue;
                        parentNode = &gScrMemTreeGlob[inst].nodes[(unsigned __int16)oldNodeValue].prev;
                    }
                }
                else
                {
                    oldNodea = HIWORD(oldNodeValue);
                    *parentNode = HIWORD(oldNodeValue);
                    if (!HIWORD(oldNodeValue))
                        return 1;
                    parentNode = &gScrMemTreeGlob[inst].nodes[HIWORD(oldNodeValue)].next;
                }
                if (!oldNodea
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_memorytree.cpp",
                        422,
                        0,
                        "%s",
                        "oldNode"))
                {
                    __debugbreak();
                }
                tempNodeValue = oldNodeValue;
                tempNodeValue_4 = oldNodeValue_4;
                tempNodeValue_8 = oldNodeValue_8;
                tempNodeValue_12 = oldNodeValue_12;
                v5 = &gScrMemTreeGlob[inst].nodes[oldNodea];
                oldNodeValue = *(_DWORD *)&v5->prev;
                oldNodeValue_4 = v5->padding[0];
                oldNodeValue_8 = v5->padding[1];
                oldNodeValue_12 = v5->padding[2];
                *(_DWORD *)&v5->prev = tempNodeValue;
                v5->padding[0] = tempNodeValue_4;
                v5->padding[1] = tempNodeValue_8;
                v5->padding[2] = tempNodeValue_12;
            }
        }
        if (oldNode == nodeNum)
            return 0;
        level >>= 1;
        if (oldNode >= nodeNum)
        {
            parentNode = &gScrMemTreeGlob[inst].nodes[node].next;
            nodeNum += level;
        }
        else
        {
            parentNode = &gScrMemTreeGlob[inst].nodes[node].prev;
            nodeNum -= level;
        }
    }
    return 0;
}
char *__cdecl MT_Alloc(int numBytes, int type, scriptInstance_t inst)
{
    return (char *)&gScrMemTreeGlob[inst] + 16 * MT_AllocIndex(inst, numBytes, type);
}

void __cdecl MT_Free(_BYTE *p, int numBytes, scriptInstance_t inst)
{
    if ( (unsigned int)((p - (_BYTE *)&gScrMemTreeGlob[inst]) >> 4) >= 0x10000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\clientscript\\cscr_memorytree.cpp",
                    776,
                    0,
                    "%s\n\t((MemoryNode *) p - gScrMemTreeGlob[inst].nodes) = %i",
                    "((MemoryNode *) p - gScrMemTreeGlob[inst].nodes >= 0 && (MemoryNode *) p - gScrMemTreeGlob[inst].nodes < MEMORY_NODE_COUNT)",
                    (p - (_BYTE *)&gScrMemTreeGlob[inst]) >> 4) )
    {
        __debugbreak();
    }
    MT_FreeIndex(inst, (p - (_BYTE *)&gScrMemTreeGlob[inst]) >> 4, numBytes);
}

bool __cdecl MT_Realloc(scriptInstance_t inst, int oldNumBytes, int newNumbytes)
{
    int Size; // esi

    Size = MT_GetSize(inst, oldNumBytes);
    return Size >= MT_GetSize(inst, newNumbytes);
}

