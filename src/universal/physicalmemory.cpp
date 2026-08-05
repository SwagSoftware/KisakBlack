#include <Windows.h>

#include "physicalmemory.h"
#include <memoryapi.h>
#include "assertive.h"
#include "q_shared.h"
#include <qcommon/common.h>
#include <qcommon/mem_track.h>
#include "../../tl/tl_system.h"
#include <win32/win_main.h>

bool g_physicalMemoryInit;
PhysicalMemory g_mem;

thread_local int g_alloc_type;
int freeTot;
int g_overAllocatedSize;

void __cdecl PMem_Init()
{
    unsigned __int8 *memory; // [esp+0h] [ebp-4h]

    if ( !g_physicalMemoryInit )
    {
        g_physicalMemoryInit = 1;
        memory = (unsigned __int8 *)VirtualAlloc(0, 0x14800000u, 0x1000u, 4u);
        PMem_InitPhysicalMemory(&g_mem, "main", memory, 0x14800000u);
    }
}

void __cdecl PMem_InitPhysicalMemory(
                PhysicalMemory *pmem,
                const char *name,
                unsigned __int8 *memory,
                unsigned int memorySize)
{
    if ( !pmem
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\physicalmemory.cpp", 316, 0, "%s", "pmem") )
    {
        __debugbreak();
    }
    if ( !memory
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\physicalmemory.cpp", 317, 0, "%s", "memory") )
    {
        __debugbreak();
    }
    memset(pmem, 0, sizeof(PhysicalMemory));
    pmem->name = name;
    pmem->buf = memory;
    pmem->prim[1].pos = memorySize;
    pmem->size = memorySize;
}

void __cdecl PMem_BeginAlloc(const char *name, unsigned int allocType, EMemTrack memTrack)
{
    //*(unsigned int *)(*((unsigned int *)NtCurrentTeb()->ThreadLocalStoragePointer + _tls_index) + 28) = allocType;
    g_alloc_type = allocType;
    if ( allocType >= 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\physicalmemory.cpp",
                    589,
                    0,
                    "allocType doesn't index PHYS_ALLOC_COUNT\n\t%i not in [0, %i)",
                    allocType,
                    2) )
    {
        __debugbreak();
    }
    PMem_BeginAllocInPrim(&g_mem.prim[allocType], name, memTrack);
}

void __cdecl PMem_BeginAllocInPrim(PhysicalMemoryPrim *prim, const char *name, EMemTrack memTrack)
{
    PhysicalMemoryAllocation *allocEntry; // [esp+0h] [ebp-4h]

    if ( prim->allocName
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\physicalmemory.cpp",
                    566,
                    0,
                    "%s",
                    "!prim->allocName") )
    {
        __debugbreak();
    }
    if ( prim->allocListCount >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\physicalmemory.cpp",
                    567,
                    0,
                    "%s",
                    "prim->allocListCount < MAX_PHYSICAL_ALLOCATIONS") )
    {
        __debugbreak();
    }
    prim->allocName = name;
    prim->memTrack = memTrack;
    allocEntry = &prim->allocList[prim->allocListCount++];
    allocEntry->name = name;
    allocEntry->pos = prim->pos;
}

void __cdecl PMem_EndAlloc(const char *name, unsigned int allocType)
{
    if ( allocType >= 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\physicalmemory.cpp",
                    612,
                    0,
                    "allocType doesn't index PHYS_ALLOC_COUNT\n\t%i not in [0, %i)",
                    allocType,
                    2) )
    {
        __debugbreak();
    }
    PMem_EndAllocInPrim(&g_mem.prim[allocType], name);
}

void __cdecl PMem_EndAllocInPrim(PhysicalMemoryPrim *prim, const char *name)
{
    if ( I_stricmp(prim->allocName, name)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\physicalmemory.cpp",
                    601,
                    0,
                    "%s",
                    "I_stricmp( prim->allocName, name ) == 0") )
    {
        __debugbreak();
    }
    prim->allocName = 0;
}

void __cdecl PMem_Free(const char *name)
{
    int i; // [esp+4h] [ebp-4h]

    for ( i = 0; i < 2; ++i )
    {
        if ( i )
            Com_Printf(16, "PMem_Free( %s, %s )\n", name, "High");
        else
            Com_Printf(16, "PMem_Free( %s, %s )\n", name, "Low");
        PMem_FreeInPrim(&g_mem.prim[i], name, 0);
    }
}

void __cdecl PMem_FreeInPrim(PhysicalMemoryPrim *prim, const char *name, int location)
{
    unsigned int allocIndex; // [esp+0h] [ebp-8h]

    for ( allocIndex = 0; allocIndex < prim->allocListCount; ++allocIndex )
    {
        if ( prim->allocList[allocIndex].name == name )
        {
            PMem_FreeIndex(prim, allocIndex, location);
            return;
        }
    }
}

void __cdecl PMem_FreeIndex(PhysicalMemoryPrim *prim, unsigned int allocIndex, int location)
{
    __int64 v3; // rax
    __int64 v4; // rax
    PhysicalMemoryAllocation *allocEntry; // [esp+8h] [ebp-8h]
    const char *name; // [esp+Ch] [ebp-4h]

    if ( prim->allocName
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\physicalmemory.cpp",
                    636,
                    0,
                    "%s",
                    "!prim->allocName") )
    {
        __debugbreak();
    }
    allocEntry = &prim->allocList[allocIndex];
    name = allocEntry->name;
    if ( !allocEntry->name
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\physicalmemory.cpp", 642, 0, "%s", "name") )
    {
        __debugbreak();
    }
    allocEntry->name = 0;
    track_flush_physical_alloc(name, prim->memTrack);
    if ( allocIndex == prim->allocListCount - 1 )
    {
        v3 = prim->pos - prim->allocList[allocIndex].pos;
        track_physical_alloc(HIDWORD(v3) - (HIDWORD(v3) ^ v3), name, prim->memTrack, location);
        v4 = prim->pos - prim->allocList[allocIndex].pos;
        freeTot += HIDWORD(v4) - (HIDWORD(v4) ^ v4);
        tlPrintf("freed %.2f\n", (float)((float)freeTot / 1048576.0));
        do
        {
            prim->pos = allocEntry->pos;
            if ( !prim->allocListCount
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\physicalmemory.cpp",
                            672,
                            0,
                            "%s",
                            "prim->allocListCount") )
            {
                __debugbreak();
            }
            if ( !--prim->allocListCount )
                break;
            allocEntry = (PhysicalMemoryAllocation *)(&prim->allocListCount + 2 * prim->allocListCount);
        }
        while ( !allocEntry->name );
    }
    else
    {
        track_PrintInfo();
        Com_Error(ERR_FATAL, "free does not match allocation");
    }
}

int __cdecl PMem_GetOverAllocatedSize()
{
    return g_overAllocatedSize;
}

unsigned __int8 *__cdecl _PMem_AllocNamed(
                unsigned int size,
                unsigned int alignment,
                signed int type,
                unsigned int allocType,
                const char *name,
                EMemTrack memTrack)
{
    unsigned int highPos; // [esp+8h] [ebp-20h]
    unsigned __int8 *result; // [esp+Ch] [ebp-1Ch]
    unsigned int lowPos; // [esp+1Ch] [ebp-Ch]
    int allocedSize; // [esp+20h] [ebp-8h]
    unsigned int alignmenta; // [esp+34h] [ebp+Ch]

    PMem_Init();
    if ( !size
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\physicalmemory.cpp", 972, 0, "%s", "size") )
    {
        __debugbreak();
    }
    if ( !alignment
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\physicalmemory.cpp", 973, 0, "%s", "alignment") )
    {
        __debugbreak();
    }
    alignmenta = alignment - 1;
    if ( allocType )
    {
        if ( allocType != 1
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\universal\\physicalmemory.cpp",
                        1064,
                        0,
                        "%s",
                        "allocType == PHYS_ALLOC_HIGH") )
        {
            __debugbreak();
        }
        lowPos = ~alignmenta & (g_mem.prim[allocType].pos - size);
        g_overAllocatedSize = g_mem.prim[0].pos - lowPos;
        if ( g_overAllocatedSize > 0 )
        {
            Com_Printf(16, "requested size:    %d    over allocation:    %d\n", size, g_overAllocatedSize);
            Sys_OutOfMemErrorInternal("C:\\projects_pc\\cod\\codsrc\\src\\universal\\physicalmemory.cpp", 1090);
        }
        allocedSize = g_mem.prim[allocType].pos - lowPos;
        g_mem.prim[allocType].pos = lowPos;
    }
    else
    {
        lowPos = ~alignmenta & (alignmenta + g_mem.prim[0].pos);
        highPos = size + lowPos;
        g_overAllocatedSize = size + lowPos - g_mem.prim[1].pos;
        if ( g_overAllocatedSize > 0 )
        {
            Com_PrintError(
                16,
                "Need %i more bytes of '%s' physical ram for alloc to succeed\n",
                g_overAllocatedSize,
                g_mem.name);
            Sys_OutOfMemErrorInternal("C:\\projects_pc\\cod\\codsrc\\src\\universal\\physicalmemory.cpp", 1056);
        }
        allocedSize = highPos - g_mem.prim[allocType].pos;
        g_mem.prim[allocType].pos = highPos;
    }
    result = &g_mem.buf[lowPos];
    track_physical_alloc(allocedSize, name, memTrack, type < 0);
    return result;
}

unsigned __int8 *__cdecl _PMem_Alloc(
                unsigned int size,
                unsigned int alignment,
                unsigned int type,
                unsigned int allocType,
                EMemTrack memTrack,
                const char *file,
                int lineNum)
{
    char allocIdBuf[260]; // [esp+0h] [ebp-108h] BYREF

    sprintf(allocIdBuf, "%s::%s %d", g_mem.prim[allocType].allocName, file, lineNum);
    return _PMem_AllocNamed(size, alignment, type, allocType, allocIdBuf, memTrack);
}

