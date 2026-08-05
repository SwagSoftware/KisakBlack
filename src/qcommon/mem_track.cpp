#include "mem_track.h"
#include <win32/win_common.h>
#include <string.h>

// LWSS: I am stubbing this memtracker out for now

void __cdecl track_static_alloc_internal_simple(void *ptr, int size, const char *name, int memTrack)
{
#if 0
    char v4; // dl
    mem_track_t *v5; // [esp+8h] [ebp-10h]
    const char *v6; // [esp+Ch] [ebp-Ch]
    mem_track_t *mem_track; // [esp+10h] [ebp-8h]

    track_init();
    if ( g_staticsMemTrackCount >= 2048
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\mem_track.cpp",
                    264,
                    0,
                    "%s",
                    "g_staticsMemTrackCount < MAX_MEM_TRACK") )
    {
        __debugbreak();
    }
    if ( size <= 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\mem_track.cpp", 265, 0, "%s", "size>0") )
    {
        __debugbreak();
    }
    mem_track = &g_staticsMemTrack[g_staticsMemTrackCount];
    mem_track->size[0] = size;
    if ( !*name && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\mem_track.cpp", 269, 0, "%s", "name[0]") )
        __debugbreak();
    v6 = name;
    v5 = mem_track;
    do
    {
        v4 = *v6;
        v5->name[0] = *v6++;
        v5 = (mem_track_t *)((char *)v5 + 1);
    }
    while ( v4 );
    mem_track->type = memTrack;
    mem_track->usageType = 0;
    mem_track->filename = g_mem_track_filename;
    track_addbasicinfo(&g_info, memTrack, 0, size);
    ++g_staticsMemTrackCount;
#endif
}

void __cdecl track_addbasicinfo(meminfo_t *info, int type, int location, int size)
{
#if 0
    int v4; // [esp+8h] [ebp-8h]
    int v5; // [esp+Ch] [ebp-4h]

    info->typeTotal[type][location] += size;
    info->total[location] += size;
    switch ( type )
    {
        case 0:
        case 1:
        case 3:
        case 36:
        case 51:
            v5 = 0;
            break;
        default:
            v5 = 1;
            break;
    }
    if ( v5 )
        info->nonSwapTotal += size;
    switch ( type )
    {
        case 0:
        case 1:
        case 3:
        case 15:
        case 17:
        case 36:
        case 51:
            v4 = 0;
            break;
        default:
            v4 = 1;
            break;
    }
    if ( v4 )
        info->nonSwapMinSpecTotal += size;
#endif
}

void __cdecl track_static_alloc_internal(void *ptr, int size, const char *name, int type)
{
#if 0
    Sys_InitializeCriticalSections();
    Sys_EnterCriticalSection(CRITSECT_MEMTRACK);
    track_static_alloc_internal_simple(ptr, size, name, type);
    Sys_LeaveCriticalSection(CRITSECT_MEMTRACK);
#endif
}

void __cdecl track_flush_physical_alloc(const char *name, unsigned __int8 type)
{
#if 0
    int k; // [esp+10h] [ebp-A0h]
    int j; // [esp+14h] [ebp-9Ch]
    char test_name[132]; // [esp+18h] [ebp-98h] BYREF
    int len; // [esp+A0h] [ebp-10h]
    TempMemInfo *mainTempMemInfo; // [esp+A4h] [ebp-Ch]
    int i; // [esp+A8h] [ebp-8h]
    TempMemInfo *tempMemInfo; // [esp+ACh] [ebp-4h]

    sprintf(test_name, "%s::", name);
    len = &test_name[strlen(test_name) + 1] - &test_name[1];
    mainTempMemInfo = GetTempMemInfo(0, name, type, 7, g_physicalMemInfoArray, &g_physicalMemInfoCount, 0);
    i = 0;
    while ( i < g_physicalMemInfoCount )
    {
        tempMemInfo = &g_physicalMemInfoArray[i];
        if ( !I_strnicmp(tempMemInfo->data.name, test_name, len) )
        {
            if ( mainTempMemInfo )
            {
                for ( j = 0; j < 2; ++j )
                    mainTempMemInfo->data.size[j] += tempMemInfo->data.size[j];
            }
            for ( k = 0; k < 2; ++k )
                tempMemInfo->data.size[k] = 0;
        }
        ++i;
        ++tempMemInfo;
    }
#endif
}

TempMemInfo *__cdecl GetTempMemInfo(
                int permanent,
                const char *name,
                unsigned __int8 type,
                int usageType,
                TempMemInfo *tempMemInfoArray,
                int *tempMemInfoCount,
                bool add_if_missing)
{
#if 0
    char v8; // dl
    int *size; // ecx
    mem_track_t *p_data; // [esp+10h] [ebp-C8h]
    const char *v11; // [esp+14h] [ebp-C4h]
    TempMemInfo foundTempMemInfo; // [esp+18h] [ebp-C0h] BYREF
    int i; // [esp+CCh] [ebp-Ch]
    TempMemInfo *tempMemInfo; // [esp+D0h] [ebp-8h]
    int count; // [esp+D4h] [ebp-4h]

    count = *tempMemInfoCount;
    i = 0;
    tempMemInfo = tempMemInfoArray;
    while ( i < count )
    {
        tempMemInfo = &tempMemInfoArray[i];
        if ( (!name || !_stricmp(tempMemInfo->data.name, name))
            && tempMemInfo->permanent == permanent
            && tempMemInfo->data.usageType == usageType )
        {
            if ( !i )
                return tempMemInfo;
            goto LABEL_22;
        }
        ++i;
        ++tempMemInfo;
    }
    if ( !add_if_missing )
        return 0;
    if ( !name && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\mem_track.cpp", 329, 0, "%s", "name") )
        __debugbreak();
    if ( ++*tempMemInfoCount >= 1500
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\mem_track.cpp",
                    331,
                    0,
                    "%s\n\t(*tempMemInfoCount) = %i",
                    "((*tempMemInfoCount) < 1500)",
                    *tempMemInfoCount) )
    {
        __debugbreak();
    }
    tempMemInfo->permanent = permanent;
    v11 = name;
    p_data = &tempMemInfo->data;
    do
    {
        v8 = *v11;
        p_data->name[0] = *v11++;
        p_data = (mem_track_t *)((char *)p_data + 1);
    }
    while ( v8 );
    tempMemInfo->data.type = type;
    tempMemInfo->data.usageType = usageType;
    size = tempMemInfo->data.size;
    tempMemInfo->data.size[0] = 0;
    size[1] = 0;
    tempMemInfo->data.filename = "";
    tempMemInfo->data.count = 0;
    tempMemInfo->high = 0;
    tempMemInfo->low = 0;
    tempMemInfo->hunkSize = 0;
    tempMemInfo->highExtra = 0;
LABEL_22:
    memcpy(&foundTempMemInfo, tempMemInfo, sizeof(foundTempMemInfo));
    while ( i > 0 )
    {
        memcpy(&tempMemInfoArray[i], &tempMemInfoArray[i - 1], sizeof(TempMemInfo));
        --i;
    }
    memcpy(tempMemInfoArray, &foundTempMemInfo, sizeof(TempMemInfo));
    return tempMemInfoArray;
#endif
    return 0;
}

void __cdecl track_set_max_memory_level()
{
#if 0
    g_maxMemoryLevel = 230686720;
#endif
}

void __cdecl track_z_alloc(int size, const char *name, int type, char *pos, int project, int overhead)
{
#if 0
    char v6; // [esp+3h] [ebp-25h]
    char *v7; // [esp+8h] [ebp-20h]
    const char *v8; // [esp+Ch] [ebp-1Ch]
    mem_track_node_s *node; // [esp+20h] [ebp-8h]

    if ( size < 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\mem_track.cpp", 515, 0, "%s", "size>=0") )
    {
        __debugbreak();
    }
    Sys_EnterCriticalSection(CRITSECT_MEMTRACK);
    g_staticsMemTrack[0].size[0] += overhead;
    track_addbasicinfo(&g_info, 0, 0, overhead);
    if ( type != 51 )
    {
        AddTempMemInfo(size, 0, 0, name, type, 0, 2, g_mallocMemInfoArray, &g_mallocMemInfoCount);
        g_malloc_mem_size += size;
        if ( g_malloc_mem_high < g_malloc_mem_size )
            g_malloc_mem_high = g_malloc_mem_size;
    }
    node = (mem_track_node_s *)(pos - 164);
    if ( !*name && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\mem_track.cpp", 532, 0, "%s", "name[0]") )
        __debugbreak();
    v8 = name;
    v7 = pos - 164;
    do
    {
        v6 = *v8;
        *v7++ = *v8++;
    }
    while ( v6 );
    node->data.filename = "";
    node->data.size[0] = size;
    node->data.size[1] = 0;
    node->data.type = type;
    node->data.usageType = 2;
    node->next = g_ZMallocMemTrackList;
    node->prev = 0;
    node->project = project;
    if ( g_ZMallocMemTrackList )
        g_ZMallocMemTrackList->prev = node;
    g_ZMallocMemTrackList = (mem_track_node_s *)(pos - 164);
    track_addbasicinfo(&g_info, type, 0, size);
    Sys_LeaveCriticalSection(CRITSECT_MEMTRACK);
#endif
}

void __cdecl AddTempMemInfo(
                int size,
                int hunkSize,
                int permanent,
                const char *name,
                unsigned __int8 type,
                int location,
                int usageType,
                TempMemInfo *tempMemInfoArray,
                int *tempMemInfoCount)
{
#if 0
    TempMemInfo *TempMemInfo; // eax

    TempMemInfo = GetTempMemInfo(permanent, name, type, usageType, tempMemInfoArray, tempMemInfoCount, 1);
    TempMemInfo->data.size[location] += size;
    ++TempMemInfo->data.count;
    CheckHighMemInfo(TempMemInfo, hunkSize, location);
#endif
}

void __cdecl CheckHighMemInfo(TempMemInfo *tempMemInfo, int hunkSize, int location)
{
    int size; // [esp+0h] [ebp-4h]

    size = tempMemInfo->data.size[location];
    if ( tempMemInfo->high < size )
        tempMemInfo->high = size;
    if ( tempMemInfo->hunkSize < hunkSize )
    {
        tempMemInfo->hunkSize = hunkSize;
        tempMemInfo->highExtra = size;
    }
}

void __cdecl track_z_free(int type, char *pos, int overhead)
{
#if 0
    mem_track_node_s *node; // [esp+10h] [ebp-8h]

    Sys_EnterCriticalSection(CRITSECT_MEMTRACK);
    node = (mem_track_node_s *)(pos - 164);
    if ( (unsigned __int8)*(pos - 20) != type
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\mem_track.cpp",
                    558,
                    0,
                    "node->data.type == type\n\t%i, %i",
                    node->data.type,
                    type) )
    {
        __debugbreak();
    }
    g_staticsMemTrack[0].size[0] -= overhead;
    track_addbasicinfo(&g_info, 0, 0, -overhead);
    if ( type != 51 )
    {
        g_malloc_mem_size -= node->data.size[0];
        if ( g_malloc_mem_size < 0 )
            g_malloc_mem_size = 0;
        RemoveTempMemInfo(node->data.size[0], 0, node->data.name, type, 0, 2, g_mallocMemInfoArray, &g_mallocMemInfoCount);
    }
    track_addbasicinfo(&g_info, type, 0, -node->data.size[0]);
    if ( node == g_ZMallocMemTrackList )
    {
        g_ZMallocMemTrackList = node->next;
        if ( g_ZMallocMemTrackList )
            g_ZMallocMemTrackList->prev = 0;
    }
    else
    {
        if ( !node->prev
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\mem_track.cpp", 584, 0, "%s", "node->prev") )
        {
            __debugbreak();
        }
        node->prev->next = node->next;
        if ( node->next )
            node->next->prev = node->prev;
    }
    Sys_LeaveCriticalSection(CRITSECT_MEMTRACK);
#endif
}

void __cdecl RemoveTempMemInfo(
                int size,
                int permanent,
                const char *name,
                unsigned __int8 type,
                int location,
                int usageType,
                TempMemInfo *tempMemInfoArray,
                int *tempMemInfoCount)
{
    TempMemInfo *tempMemInfo; // [esp+0h] [ebp-4h]

    tempMemInfo = GetTempMemInfo(permanent, name, type, usageType, tempMemInfoArray, tempMemInfoCount, 0);
    if ( tempMemInfo )
    {
        tempMemInfo->data.size[location] -= size;
        --tempMemInfo->data.count;
        CheckLowMemInfo(tempMemInfo, location);
    }
}

void __cdecl CheckLowMemInfo(TempMemInfo *tempMemInfo, int location)
{
    if ( tempMemInfo->low > tempMemInfo->data.size[location] )
        tempMemInfo->low = tempMemInfo->data.size[location];
}

void __cdecl track_z_commit(int size, int type)
{
#if 0
    Sys_EnterCriticalSection(CRITSECT_MEMTRACK);
    track_addbasicinfo(&g_virtualMemInfo, type, 0, size);
    track_addbasicinfo(&g_info, type, 0, size);
    Sys_LeaveCriticalSection(CRITSECT_MEMTRACK);
#endif
}

void __cdecl track_z_decommit(int size, int type)
{
#if 0
    Sys_EnterCriticalSection(CRITSECT_MEMTRACK);
    track_addbasicinfo(&g_virtualMemInfo, type, 0, -size);
    track_addbasicinfo(&g_info, type, 0, -size);
    Sys_LeaveCriticalSection(CRITSECT_MEMTRACK);
#endif
}

void __cdecl track_physical_alloc(int size, const char *name, int type, int location)
{
#if 0
    track_init();
    if ( size )
    {
        Sys_EnterCriticalSection(CRITSECT_MEMTRACK);
        if ( g_physicalMemInfoCount >= 1500
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\mem_track.cpp",
                        621,
                        0,
                        "%s",
                        "g_physicalMemInfoCount < TEMP_MEM_INFO_COUNT") )
        {
            __debugbreak();
        }
        if ( size <= 0 )
            RemoveTempMemInfo(-size, 0, name, type, location, 7, g_physicalMemInfoArray, &g_physicalMemInfoCount);
        else
            AddTempMemInfo(size, 0, 0, name, type, location, 7, g_physicalMemInfoArray, &g_physicalMemInfoCount);
        track_addbasicinfo(&g_info, type, location, size);
        Sys_LeaveCriticalSection(CRITSECT_MEMTRACK);
    }
#endif
}

void __cdecl track_temp_alloc(int size, int hunkSize, int permanent, const char *name)
{
    //Sys_EnterCriticalSection(CRITSECT_MEMTRACK);
    //AddTempMemInfo(size, hunkSize, permanent, name, 0xBu, 0, 6, g_tempMemInfoArray, &g_tempMemInfoCount);
    //Sys_LeaveCriticalSection(CRITSECT_MEMTRACK);
}

void __cdecl track_temp_free(int size, int permanent, const char *name)
{
    //Sys_EnterCriticalSection(CRITSECT_MEMTRACK);
    //RemoveTempMemInfo(size, permanent, name, 0xBu, 0, 6, g_tempMemInfoArray, &g_tempMemInfoCount);
    //Sys_LeaveCriticalSection(CRITSECT_MEMTRACK);
}

void __cdecl track_temp_clear(int permanent)
{
    //int i; // [esp+8h] [ebp-Ch]

    //Sys_EnterCriticalSection(CRITSECT_MEMTRACK);
    //for ( i = 0; i < g_tempMemInfoCount; ++i )
    //{
    //    if ( g_tempMemInfoArray[i].permanent == permanent )
    //        g_tempMemInfoArray[i].data.size[0] = 0;
    //}
    //Sys_LeaveCriticalSection(CRITSECT_MEMTRACK);
}

void __cdecl track_temp_high_alloc(int size, int hunkSize, int permanent, const char *name)
{
    //Sys_EnterCriticalSection(CRITSECT_MEMTRACK);
    //AddTempMemInfo(size, hunkSize, permanent, name, 0xBu, 0, 6, g_tempHighMemInfoArray, &g_tempHighMemInfoCount);
    //Sys_LeaveCriticalSection(CRITSECT_MEMTRACK);
}

void __cdecl track_temp_high_clear(int permanent)
{
    //int i; // [esp+8h] [ebp-Ch]
    //TempMemInfo *tempMemInfo; // [esp+Ch] [ebp-8h]

    //Sys_EnterCriticalSection(CRITSECT_MEMTRACK);
    //i = 0;
    //tempMemInfo = g_tempHighMemInfoArray;
    //while ( i < g_tempHighMemInfoCount )
    //{
    //    if ( tempMemInfo->permanent == permanent )
    //        tempMemInfo->data.size[0] = 0;
    //    ++i;
    //    ++tempMemInfo;
    //}
    //Sys_LeaveCriticalSection(CRITSECT_MEMTRACK);
}

void __cdecl track_userhunk_freerange(void *low, unsigned int size)
{
    //int i; // [esp+10h] [ebp-8h]

    //Sys_EnterCriticalSection(CRITSECT_MEMTRACK);
    //for ( i = 0; i < g_userhunk_track_count; ++i )
    //{
    //    if ( g_userhunk_track[i].pos >= (int)low && g_userhunk_track[i].pos < (int)((int)low + size) )
    //    {
    //        memcpy(&g_userhunk_track[i], &g_userhunk_track[g_userhunk_track_count - 1], sizeof(mem_track_t));
    //        memset(&g_userhunk_track[g_userhunk_track_count - 1], 0, sizeof(mem_track_t));
    //        --g_userhunk_track_count;
    //        --i;
    //    }
    //}
    //Sys_LeaveCriticalSection(CRITSECT_MEMTRACK);
}

void __cdecl track_userhunk_free(void *ptr)
{
    //int i; // [esp+10h] [ebp-8h]

    //Sys_EnterCriticalSection(CRITSECT_MEMTRACK);
    //for ( i = 0; i < g_userhunk_track_count; ++i )
    //{
    //    if ( (void *)g_userhunk_track[i].pos == ptr )
    //    {
    //        if ( i != g_userhunk_track_count - 1 )
    //            memcpy(&g_userhunk_track[i], &g_userhunk_track[g_userhunk_track_count - 1], sizeof(mem_track_t));
    //        break;
    //    }
    //}
    //if ( --g_userhunk_track_count < 0
    //    && !Assert_MyHandler(
    //                "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\mem_track.cpp",
    //                746,
    //                0,
    //                "%s",
    //                "g_userhunk_track_count >= 0") )
    //{
    //    __debugbreak();
    //}
    //Sys_LeaveCriticalSection(CRITSECT_MEMTRACK);
}

void __cdecl track_userhunk_alloc(int size, int pos, const char *name, int type)
{
    //char v4; // [esp+3h] [ebp-1Dh]
    //mem_track_t *v5; // [esp+8h] [ebp-18h]
    //const char *v6; // [esp+Ch] [ebp-14h]
    //mem_track_t *mem_track; // [esp+18h] [ebp-8h]

    //Sys_EnterCriticalSection(CRITSECT_MEMTRACK);
    //if ( g_userhunk_track_count >= 256
    //    && !Assert_MyHandler(
    //                "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\mem_track.cpp",
    //                757,
    //                0,
    //                "%s",
    //                "g_userhunk_track_count < MAX_USERHUNK_TRACK") )
    //{
    //    __debugbreak();
    //}
    //mem_track = &g_userhunk_track[g_userhunk_track_count];
    //mem_track->size[0] = size;
    //if ( !*name && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\mem_track.cpp", 761, 0, "%s", "name[0]") )
    //    __debugbreak();
    //v6 = name;
    //v5 = mem_track;
    //do
    //{
    //    v4 = *v6;
    //    v5->name[0] = *v6++;
    //    v5 = (mem_track_t *)((char *)v5 + 1);
    //}
    //while ( v4 );
    //mem_track->type = type;
    //mem_track->usageType = 8;
    //mem_track->filename = "";
    //mem_track->pos = pos;
    //track_addbasicinfo(&g_info, type, 0, size);
    //++g_userhunk_track_count;
    //Sys_LeaveCriticalSection(CRITSECT_MEMTRACK);
}

void __cdecl track_hunk_alloc(int size, int pos, const char *name, int type)
{
    //char v4; // [esp+3h] [ebp-1Dh]
    //mem_track_t *v5; // [esp+8h] [ebp-18h]
    //const char *v6; // [esp+Ch] [ebp-14h]
    //mem_track_t *mem_track; // [esp+18h] [ebp-8h]

    //Sys_EnterCriticalSection(CRITSECT_MEMTRACK);
    //g_staticsMemTrack[1].size[0] -= size;
    //track_addbasicinfo(&g_info, 1, 0, -size);
    //if ( g_hunk_track_count >= 0x80000
    //    && !Assert_MyHandler(
    //                "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\mem_track.cpp",
    //                784,
    //                0,
    //                "%s",
    //                "g_hunk_track_count < MAX_HUNK_TRACK") )
    //{
    //    __debugbreak();
    //}
    //mem_track = &g_hunk_track[g_hunk_track_count];
    //mem_track->size[0] = size;
    //if ( !*name && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\mem_track.cpp", 788, 0, "%s", "name[0]") )
    //    __debugbreak();
    //v6 = name;
    //v5 = mem_track;
    //do
    //{
    //    v4 = *v6;
    //    v5->name[0] = *v6++;
    //    v5 = (mem_track_t *)((char *)v5 + 1);
    //}
    //while ( v4 );
    //mem_track->type = type;
    //mem_track->usageType = 3;
    //mem_track->filename = "";
    //mem_track->pos = pos;
    //track_addbasicinfo(&g_info, type, 0, size);
    //++g_hunk_track_count;
    //Sys_LeaveCriticalSection(CRITSECT_MEMTRACK);
}

void __cdecl track_hunk_allocLow(int size, int pos, const char *name, int type)
{
    //char v4; // [esp+3h] [ebp-1Dh]
    //mem_track_t *v5; // [esp+8h] [ebp-18h]
    //const char *v6; // [esp+Ch] [ebp-14h]
    //mem_track_t *mem_track; // [esp+18h] [ebp-8h]

    //Sys_EnterCriticalSection(CRITSECT_MEMTRACK);
    //g_staticsMemTrack[1].size[0] -= size;
    //track_addbasicinfo(&g_info, 1, 0, -size);
    //if ( g_staticsMemTrack[1].size[0] < 0
    //    && !Assert_MyHandler(
    //                "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\mem_track.cpp",
    //                811,
    //                0,
    //                "%s",
    //                "g_staticsMemTrack[TRACK_HUNK].size[TRACK_LOCATION_MRAM] >= 0") )
    //{
    //    __debugbreak();
    //}
    //if ( g_hunklow_track_count >= 0x10000
    //    && !Assert_MyHandler(
    //                "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\mem_track.cpp",
    //                813,
    //                0,
    //                "%s",
    //                "g_hunklow_track_count < MAX_HUNKLOW_TRACK") )
    //{
    //    __debugbreak();
    //}
    //mem_track = &g_hunklow_track[g_hunklow_track_count];
    //mem_track->size[0] = size;
    //if ( !*name && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\mem_track.cpp", 817, 0, "%s", "name[0]") )
    //    __debugbreak();
    //v6 = name;
    //v5 = mem_track;
    //do
    //{
    //    v4 = *v6;
    //    v5->name[0] = *v6++;
    //    v5 = (mem_track_t *)((char *)v5 + 1);
    //}
    //while ( v4 );
    //mem_track->type = type;
    //mem_track->usageType = 3;
    //mem_track->filename = "";
    //mem_track->pos = pos;
    //track_addbasicinfo(&g_info, type, 0, size);
    //++g_hunklow_track_count;
    //Sys_LeaveCriticalSection(CRITSECT_MEMTRACK);
}

void __cdecl track_set_hunk_size(int size)
{
    //Sys_EnterCriticalSection(CRITSECT_MEMTRACK);
    //g_staticsMemTrack[1].size[0] = size;
    //Sys_LeaveCriticalSection(CRITSECT_MEMTRACK);
}

void __cdecl track_hunk_ClearToMarkHigh(int mark)
{
    //mem_track_t *info; // [esp+8h] [ebp-10h]
    //int size; // [esp+Ch] [ebp-Ch]

    //Sys_EnterCriticalSection(CRITSECT_MEMTRACK);
    //while ( g_hunk_track_count )
    //{
    //    info = &g_hunk_track[g_hunk_track_count - 1];
    //    if ( info->pos <= mark )
    //        break;
    //    --g_hunk_track_count;
    //    size = info->size[0];
    //    track_addbasicinfo(&g_info, info->type, 0, -size);
    //    track_addbasicinfo(&g_info, 1, 0, size);
    //    g_staticsMemTrack[1].size[0] += size;
    //}
    //Sys_LeaveCriticalSection(CRITSECT_MEMTRACK);
}

void __cdecl track_hunk_ClearToMarkLow(int mark)
{
    //mem_track_t *info; // [esp+8h] [ebp-10h]
    //int size; // [esp+Ch] [ebp-Ch]

    //Sys_EnterCriticalSection(CRITSECT_MEMTRACK);
    //while ( g_hunklow_track_count )
    //{
    //    info = &g_hunklow_track[g_hunklow_track_count - 1];
    //    if ( info->pos <= mark )
    //        break;
    //    --g_hunklow_track_count;
    //    size = info->size[0];
    //    track_addbasicinfo(&g_info, info->type, 0, -size);
    //    track_addbasicinfo(&g_info, 1, 0, size);
    //    g_staticsMemTrack[1].size[0] += size;
    //}
    //Sys_LeaveCriticalSection(CRITSECT_MEMTRACK);
}

void __cdecl track_hunk_ClearToStart()
{
    Sys_EnterCriticalSection(CRITSECT_MEMTRACK);
    track_temp_reset();
    track_temp_high_reset();
    Sys_LeaveCriticalSection(CRITSECT_MEMTRACK);
}

void track_temp_reset()
{
    //g_tempMemInfoCount = 0;
}

void track_temp_high_reset()
{
    //g_tempHighMemInfoCount = 0;
}

void __cdecl track_init()
{
    //mem_track_t *mem_track; // [esp+Ch] [ebp-8h]

    //Sys_InitializeCriticalSections();
    //Sys_EnterCriticalSection(CRITSECT_MEMTRACK);
    //if ( inited_0 )
    //{
    //    Sys_LeaveCriticalSection(CRITSECT_MEMTRACK);
    //}
    //else
    //{
    //    inited_0 = 1;
    //    memset(&g_info, 0, sizeof(g_info));
    //    memset(&g_virtualMemInfo, 0, sizeof(g_virtualMemInfo));
    //    for ( g_staticsMemTrackCount = 0; g_staticsMemTrackCount < 54; ++g_staticsMemTrackCount )
    //    {
    //        mem_track = &g_staticsMemTrack[g_staticsMemTrackCount];
    //        mem_track->size[0] = 0;
    //        mem_track->size[1] = 0;
    //        strcpy(mem_track->name, "internal");
    //        mem_track->type = g_staticsMemTrackCount;
    //        mem_track->usageType = 4;
    //        mem_track->filename = "";
    //    }
    //    strcpy(g_staticsMemTrack[1].name, "(hunk memory)");
    //    strcpy(g_staticsMemTrack[23].name, "(agp memory)");
    //    g_userhunk_track_count = 0;
    //    g_hunk_track_count = 0;
    //    g_hunklow_track_count = 0;
    //    g_mem_track_filename = "scr_memorytree";
    //    TRACK_scr_memorytree();
    //    g_mem_track_filename = "scr_debugger";
    //    TRACK_scr_debugger();
    //    g_mem_track_filename = "scr_evaluate";
    //    TRACK_scr_evaluate();
    //    g_mem_track_filename = "scr_variable";
    //    TRACK_scr_variable();
    //    g_mem_track_filename = "scr_compiler";
    //    TRACK_scr_compiler();
    //    g_mem_track_filename = "scr_parser";
    //    TRACK_scr_parser();
    //    g_mem_track_filename = "scr_animtree";
    //    TRACK_scr_animtree();
    //    g_mem_track_filename = "scr_stringlist";
    //    TRACK_scr_stringlist();
    //    g_mem_track_filename = "scr_vm";
    //    TRACK_scr_vm();
    //    g_mem_track_filename = "net_chan";
    //    TRACK_net_chan();
    //    g_mem_track_filename = "xmodel";
    //    TRACK_xmodel();
    //    g_mem_track_filename = "sv_main";
    //    TRACK_sv_main();
    //    g_mem_track_filename = "cm_load";
    //    TRACK_cm_load();
    //    g_mem_track_filename = "cm_showcollision";
    //    TRACK_cm_showcollision();
    //    g_mem_track_filename = "cm_world";
    //    TRACK_cm_world();
    //    g_mem_track_filename = "cmd";
    //    TRACK_cmd();
    //    g_mem_track_filename = "com_files";
    //    TRACK_com_files();
    //    g_mem_track_filename = "com_math";
    //    TRACK_com_math();
    //    g_mem_track_filename = "fx_draw";
    //    //BLOPS_NULLSUB();
    //    g_mem_track_filename = "fx_random";
    //    TRACK_fx_random();
    //    g_mem_track_filename = "fx_marks";
    //    TRACK_fx_marks();
    //    g_mem_track_filename = "fx_system";
    //    TRACK_fx_system();
    //    g_mem_track_filename = "ui_viewer";
    //    TRACK_ui_viewer();
    //    g_mem_track_filename = "dvar";
    //    TRACK_dvar();
    //    g_mem_track_filename = "dvar_cmds";
    //    TRACK_dvar_cmds();
    //    g_mem_track_filename = "msg";
    //    TRACK_msg();
    //    g_mem_track_filename = "q_parse";
    //    TRACK_q_parse();
    //    g_mem_track_filename = "win_common";
    //    TRACK_win_common();
    //    g_mem_track_filename = "win_syscon";
    //    TRACK_win_syscon();
    //    g_mem_track_filename = "win_net";
    //    TRACK_win_net();
    //    g_mem_track_filename = "g_vehicle_path";
    //    TRACK_g_vehicle_path();
    //    g_mem_track_filename = "q_shared";
    //    TRACK_q_shared();
    //    g_mem_track_filename = "common";
    //    TRACK_common();
    //    g_mem_track_filename = "dobj_management";
    //    TRACK_dobj_management();
    //    g_mem_track_filename = "files";
    //    TRACK_files();
    //    g_mem_track_filename = "memtrack";
    //    TRACK_memtrack();
    //    g_mem_track_filename = "scr_animtree";
    //    TRACK_scr_animtree();
    //    g_mem_track_filename = "com_memory";
    //    TRACK_com_memory();
    //    g_mem_track_filename = "sv_init";
    //    //BLOPS_NULLSUB();
    //    g_mem_track_filename = "zutil";
    //    //BLOPS_NULLSUB();
    //    g_mem_track_filename = "memfile";
    //    //BLOPS_NULLSUB();
    //    g_mem_track_filename = "cl_main";
    //    TRACK_cl_main();
    //    g_mem_track_filename = "cl_console";
    //    TRACK_cl_console();
    //    g_mem_track_filename = "cl_input";
    //    TRACK_cl_input();
    //    g_mem_track_filename = "cl_keys";
    //    TRACK_cl_keys();
    //    g_mem_track_filename = "cl_parse";
    //    TRACK_cl_parse();
    //    g_mem_track_filename = "win_input";
    //    TRACK_win_input();
    //    g_mem_track_filename = "cl_srcn";
    //    //BLOPS_NULLSUB();
    //    g_mem_track_filename = "cl_cgame";
    //    TRACK_cl_cgame();
    //    g_mem_track_filename = "statmonitor";
    //    TRACK_statmonitor();
    //    g_mem_track_filename = "snd";
    //    TRACK_snd();
    //    g_mem_track_filename = "snd_driver";
    //    //BLOPS_NULLSUB();
    //    g_mem_track_filename = "stringed_hooks";
    //    TRACK_stringed_hooks();
    //    g_mem_track_filename = "devgui";
    //    TRACK_devgui();
    //    g_mem_track_filename = "win_gamepad";
    //    TRACK_win_gamepad();
    //    g_mem_track_filename = "assertive";
    //    TRACK_assertive();
    //    g_mem_track_filename = "db_registry";
    //    TRACK_db_registry();
    //    g_mem_track_filename = "com_profilemapload";
    //    TRACK_com_profilemapload();
    //    g_mem_track_filename = "profileMem";
    //    TRACK_profileMem();
    //    g_mem_track_filename = "missile_attractors";
    //    TRACK_missile_attractors();
    //    g_mem_track_filename = "DynEntityCollWorld";
    //    TRACK_DynEntityCollWorld();
    //    g_mem_track_filename = "ragdoll";
    //    TRACK_ragdoll();
    //    g_mem_track_filename = "flame_system";
    //    TRACK_flame_system();
    //    CG_track_init();
    //    G_track_init();
    //    UI_track_init();
    //    R_Track_Init();
    //    g_mem_track_filename = "";
    //    Sys_LeaveCriticalSection(CRITSECT_MEMTRACK);
    //}
}

void __cdecl track_getbasicinfo(meminfo_t *info)
{
    //if ( !info && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\mem_track.cpp", 1374, 0, "%s", "info") )
    //    __debugbreak();
    //Sys_EnterCriticalSection(CRITSECT_MEMTRACK);
    //memset(info, 0, sizeof(meminfo_t));
    //track_addbasicmeminfo(info, &g_info);
    //Sys_LeaveCriticalSection(CRITSECT_MEMTRACK);
}

void __cdecl track_addbasicmeminfo(meminfo_t *sum, meminfo_t *in)
{
    int j; // [esp+0h] [ebp-8h]
    int type; // [esp+4h] [ebp-4h]

    for ( j = 0; j < 2; ++j )
    {
        for ( type = 0; type < 54; ++type )
            sum->typeTotal[type][j] += in->typeTotal[type][j];
        sum->total[j] += in->total[j];
    }
    sum->nonSwapTotal += in->nonSwapTotal;
    sum->nonSwapMinSpecTotal += in->nonSwapMinSpecTotal;
}

void __cdecl track_shutdown(int project)
{
    //const char *v1; // eax
    //mem_track_node_s *node; // [esp+8h] [ebp-Ch]
    //int leak_size; // [esp+Ch] [ebp-8h]

    //Sys_EnterCriticalSection(CRITSECT_MEMTRACK);
    //leak_size = 0;
    //for ( node = g_ZMallocMemTrackList; node; node = node->next )
    //{
    //    if ( node->project == project && !project && !I_stricmp(node->data.name, "MSS_MallocCallback") )
    //    {
    //        leak_size += node->data.size[0];
    //        Com_Printf_NoFilter("Error: memory leak of '%s' in %s", node->data.name, projName[0]);
    //    }
    //}
    //if ( leak_size )
    //{
    //    v1 = va("Total memory leak in '%s': %d", projName[project], leak_size);
    //    if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\mem_track.cpp", 1433, 0, v1) )
    //        __debugbreak();
    //}
    //Sys_LeaveCriticalSection(CRITSECT_MEMTRACK);
}

void __cdecl track_PrintInfo()
{
    //const char *v0; // eax
    //double v1; // st7
    //double v2; // st7
    //double v3; // st7
    //double v4; // st7
    //double v5; // [esp+8h] [ebp-268h]
    //double v6; // [esp+8h] [ebp-268h]
    //int v7; // [esp+4Ch] [ebp-224h]
    //int v8; // [esp+50h] [ebp-220h]
    //int k; // [esp+5Ch] [ebp-214h]
    //int j; // [esp+60h] [ebp-210h]
    //int ja; // [esp+60h] [ebp-210h]
    //int jb; // [esp+60h] [ebp-210h]
    //mem_track_t *mem_track; // [esp+64h] [ebp-20Ch]
    //mem_track_t *mem_tracka; // [esp+64h] [ebp-20Ch]
    //mem_track_t *mem_trackb; // [esp+64h] [ebp-20Ch]
    //mem_track_t *mem_trackc; // [esp+64h] [ebp-20Ch]
    //mem_track_node_s *node; // [esp+68h] [ebp-208h]
    //mem_track_node_s *nodea; // [esp+68h] [ebp-208h]
    //mem_track_t *mem_track2; // [esp+6Ch] [ebp-204h]
    //meminfo_t info; // [esp+70h] [ebp-200h] BYREF
    //int len2; // [esp+230h] [ebp-40h]
    //int len; // [esp+234h] [ebp-3Ch]
    //bitarray<54> physExclusions; // [esp+238h] [ebp-38h] BYREF
    //int lramTotal; // [esp+240h] [ebp-30h]
    //mem_track_t *sorted_mem_track; // [esp+244h] [ebp-2Ch]
    //bool addedLine; // [esp+24Bh] [ebp-25h]
    //int reportedStreamBufferTot; // [esp+24Ch] [ebp-24h]
    //int nodeCount; // [esp+250h] [ebp-20h]
    //int reportedPhysTot[2]; // [esp+254h] [ebp-1Ch]
    //int mramTotal; // [esp+25Ch] [ebp-14h]
    //int type; // [esp+260h] [ebp-10h]
    //int i; // [esp+264h] [ebp-Ch]
    //Smp_CriticalSectionBlock csb; // [esp+268h] [ebp-8h]
    //int reportedExtraStreamBufferTot; // [esp+26Ch] [ebp-4h]

    //csb.mCS = CRITSECT_MEMTRACK;
    //Sys_EnterCriticalSection(CRITSECT_MEMTRACK);
    //memset(&info, 0, sizeof(info));
    //len2 = g_hunklow_track_count + g_hunk_track_count + g_userhunk_track_count + g_staticsMemTrackCount;
    //nodeCount = 0;
    //for ( node = g_ZMallocMemTrackList; node; node = node->next )
    //    ++nodeCount;
    //nodeCount += g_physicalMemInfoCount;
    //len2 += nodeCount;
    //sorted_mem_track = (mem_track_t *)malloc(152 * len2);
    //if ( sorted_mem_track )
    //{
    //    len = 0;
    //    Com_Memcpy(sorted_mem_track, g_staticsMemTrack, 152 * g_staticsMemTrackCount);
    //    len += g_staticsMemTrackCount;
    //    Com_Memcpy(&sorted_mem_track[len], g_userhunk_track, 152 * g_userhunk_track_count);
    //    len += g_userhunk_track_count;
    //    Com_Memcpy(&sorted_mem_track[len], g_hunk_track, 152 * g_hunk_track_count);
    //    len += g_hunk_track_count;
    //    Com_Memcpy(&sorted_mem_track[len], g_hunklow_track, 152 * g_hunklow_track_count);
    //    len += g_hunklow_track_count;
    //    mem_track = &sorted_mem_track[len];
    //    nodea = g_ZMallocMemTrackList;
    //    while ( nodea )
    //    {
    //        memcpy(mem_track, nodea, sizeof(mem_track_t));
    //        nodea = nodea->next;
    //        ++mem_track;
    //    }
    //    bitarray<54>::bitarray<54>(&physExclusions, 0xCu, 37, 38, 1, 37, -1);
    //    reportedPhysTot[0] = 0;
    //    reportedPhysTot[1] = 0;
    //    reportedExtraStreamBufferTot = 0;
    //    reportedStreamBufferTot = 0;
    //    i = 0;
    //    while ( i < g_physicalMemInfoCount )
    //    {
    //        memcpy(mem_track, &g_physicalMemInfoArray[i].data, sizeof(mem_track_t));
    //        if ( !bitarray<54>::testBit(&physExclusions, mem_track->type) && mem_track->usageType == 7 )
    //        {
    //            if ( mem_track->type == 45 )
    //            {
    //                reportedStreamBufferTot += mem_track->size[1];
    //            }
    //            else if ( mem_track->type == 46 )
    //            {
    //                reportedExtraStreamBufferTot += mem_track->size[1];
    //            }
    //            else
    //            {
    //                reportedPhysTot[0] += mem_track->size[0];
    //                reportedPhysTot[1] += mem_track->size[1];
    //            }
    //        }
    //        ++i;
    //        ++mem_track;
    //    }
    //    len += nodeCount;
    //    if ( len != len2
    //        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\mem_track.cpp", 1613, 0, "%s", "len == len2") )
    //    {
    //        __debugbreak();
    //    }
    //    for ( i = len - 1; i >= 0; --i )
    //    {
    //        for ( j = i - 1; j >= 0; --j )
    //        {
    //            mem_tracka = &sorted_mem_track[i];
    //            mem_track2 = &sorted_mem_track[j];
    //            if ( mem_tracka->type == mem_track2->type
    //                && mem_tracka
    //                && mem_track2
    //                && mem_tracka->filename
    //                && mem_track2->filename
    //                && !strcmp(sorted_mem_track[i].name, sorted_mem_track[j].name)
    //                && !strcmp(mem_tracka->filename, mem_track2->filename) )
    //            {
    //                for ( k = 0; k < 2; ++k )
    //                    mem_track2->size[k] += mem_tracka->size[k];
    //                memcpy(mem_tracka, &sorted_mem_track[--len], sizeof(mem_track_t));
    //                break;
    //            }
    //        }
    //    }
    //    for ( i = 0; i < len; ++i )
    //    {
    //        mem_trackb = &sorted_mem_track[i];
    //        if ( mem_trackb->usageType != 8 )
    //        {
    //            type = mem_trackb->type;
    //            if ( mem_trackb->size[0] < 0 && mem_trackb->size[1] < 0 )
    //            {
    //                v0 = va(
    //                             "%s, %s, %i, %i, %s, %s\n",
    //                             gMemTrackNames[type],
    //                             usageTypeName[mem_trackb->usageType],
    //                             mem_trackb->size[0],
    //                             mem_trackb->size[1],
    //                             mem_trackb->name,
    //                             mem_trackb->filename);
    //                if ( !Assert_MyHandler(
    //                                "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\mem_track.cpp",
    //                                1642,
    //                                0,
    //                                "%s\n\t%s",
    //                                "mem_track->size[TRACK_LOCATION_MRAM] >= 0 || mem_track->size[TRACK_LOCATION_LRAM] >= 0",
    //                                v0) )
    //                    __debugbreak();
    //            }
    //            for ( ja = 0; ja < 2; ++ja )
    //            {
    //                if ( mem_trackb->size[ja] > 0 )
    //                {
    //                    info.typeTotal[type][ja] += mem_trackb->size[ja];
    //                    info.total[ja] += mem_trackb->size[ja];
    //                    switch ( type )
    //                    {
    //                        case 0:
    //                        case 1:
    //                        case 3:
    //                        case 36:
    //                        case 51:
    //                            v8 = 0;
    //                            break;
    //                        default:
    //                            v8 = 1;
    //                            break;
    //                    }
    //                    if ( v8 )
    //                        info.nonSwapTotal += mem_trackb->size[ja];
    //                    switch ( type )
    //                    {
    //                        case 0:
    //                        case 1:
    //                        case 3:
    //                        case 15:
    //                        case 17:
    //                        case 36:
    //                        case 51:
    //                            v7 = 0;
    //                            break;
    //                        default:
    //                            v7 = 1;
    //                            break;
    //                    }
    //                    if ( v7 )
    //                        info.nonSwapMinSpecTotal += mem_trackb->size[ja];
    //                }
    //            }
    //        }
    //    }
    //    qsort(sorted_mem_track, len, 0x98u, (int (__cdecl *)(const void *, const void *))mem_track_compare);
    //    info.typeTotal[4][0] = -1;
    //    info.typeTotal[18][0] = -1;
    //    info.typeTotal[25][0] = -1;
    //    info.typeTotal[33][0] = -1;
    //    info.typeTotal[52][0] = -1;
    //    Com_Printf_NoFilter(
    //        "\n%-18s    %-10s    %7s    %7s    %-26s    %s\n\n",
    //        "-type-",
    //        "-alocator-",
    //        "-MRAM-",
    //        "-LRAM-",
    //        "-instance-",
    //        "-filename-");
    //    for ( jb = 0; jb < 9; ++jb )
    //    {
    //        for ( i = 0; i < len; ++i )
    //        {
    //            mem_trackc = &sorted_mem_track[i];
    //            type = mem_trackc->type;
    //            if ( mem_trackc->usageType == jb && (mem_trackc->size[0] >= 0x2000 || mem_trackc->size[1] >= 0x2000) )
    //                Com_Printf_NoFilter(
    //                    "%-18s, %-10s, %7i, %7i, %-26s, %s\n",
    //                    gMemTrackNames[type],
    //                    usageTypeName[mem_trackc->usageType],
    //                    mem_trackc->size[0] / 1024,
    //                    mem_trackc->size[1] / 1024,
    //                    mem_trackc->name,
    //                    mem_trackc->filename);
    //        }
    //    }
    //    Com_Printf_NoFilter("\ntotals:\n");
    //    Com_Printf_NoFilter("%-20s -MRAM- \t-LRAM-\n", "-type-");
    //    addedLine = 0;
    //    mramTotal = 0;
    //    lramTotal = 0;
    //    for ( i = 0; i < 54; ++i )
    //    {
    //        if ( info.typeTotal[i][0] < 0 )
    //        {
    //            if ( addedLine )
    //                Com_Printf_NoFilter("------------------------------------\n");
    //        }
    //        else
    //        {
    //            mramTotal += info.typeTotal[i][0];
    //            lramTotal += info.typeTotal[i][1];
    //            if ( info.typeTotal[i][0] > 1024 || info.typeTotal[i][1] > 1024 )
    //            {
    //                v5 = mb(info.typeTotal[i][1]);
    //                v1 = mb(info.typeTotal[i][0]);
    //                Com_Printf_NoFilter("%-20s %6.2f \t%6.2f\n", gMemTrackNames[i], v1, v5);
    //                addedLine = 1;
    //            }
    //        }
    //    }
    //    v6 = mb(lramTotal);
    //    v2 = mb(mramTotal);
    //    Com_Printf_NoFilter("%-20s %6.2f \t%6.2f\n", "TOTAL", v2, v6);
    //    v3 = mb(info.nonSwapTotal);
    //    Com_Printf_NoFilter("current total            %5.1f\n", v3);
    //    v4 = mb(info.nonSwapMinSpecTotal);
    //    Com_Printf_NoFilter("min pc total             %5.1f\n", v4);
    //    Com_Printf_NoFilter("\n\n");
    //    free(sorted_mem_track);
    //    Sys_LeaveCriticalSection(csb.mCS);
    //}
    //else
    //{
    //    Com_Printf_NoFilter("track_PrintInfo: out of memory\n");
    //    Sys_LeaveCriticalSection(csb.mCS);
    //}
}

double __cdecl mb(int n)
{
    return (double)n / 1048576.0;
}

int __cdecl mem_track_compare(const char *elem1, const char *elem2)
{
    int i; // [esp+18h] [ebp-Ch]

    for ( i = 0; i < 2; ++i )
    {
        if ( *(unsigned int *)&elem1[4 * i + 132] != *(unsigned int *)&elem2[4 * i + 132] )
            return *(unsigned int *)&elem1[4 * i + 132] - *(unsigned int *)&elem2[4 * i + 132];
    }
    if ( *((unsigned __int8 *)elem1 + 144) < (int)*((unsigned __int8 *)elem2 + 144) )
        return -1;
    if ( *((unsigned __int8 *)elem1 + 144) > (int)*((unsigned __int8 *)elem2 + 144) )
        return 1;
    if ( *((unsigned __int8 *)elem1 + 145) < (int)*((unsigned __int8 *)elem2 + 145) )
        return -1;
    if ( *((unsigned __int8 *)elem1 + 145) > (int)*((unsigned __int8 *)elem2 + 145) )
        return 1;
    if ( !elem1 )
        return -1;
    if ( elem2 )
        return strcmp(elem1, elem2);
    return 1;
}

void __cdecl track_PrintAllInfo()
{
    //if ( Sys_IsMainThread() )
    //{
    //    track_PrintInfo();
    //    //BLOPS_NULLSUB();
    //    //BLOPS_NULLSUB();
    //}
}

