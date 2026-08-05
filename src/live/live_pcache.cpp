#include "live_pcache.h"
#include <ctime>
#include "live_pcache_profile.h"
#include <qcommon/threads.h>
#include "live_storage_win.h"
#include <qcommon/com_clients.h>
#include <bgame/bg_emblems.h>
#include "live_win.h"
#include <gfx_d3d/r_stream.h>
#include <demo/demo_ui.h>
#include <client_mp/cl_cgame_mp.h>
#include <client/client.h>
#include <database/db_registry.h>
#include <cgame_mp/cg_local_mp.h>

FastCriticalSection s_lock;

PCachePublicProfile s_publicProfiles[256];
PCachePlayerEmblem s_playerEmblems[32];

PCacheEntry s_entries[256];

PCacheComponentPool s_componentPools[2] =
{
  { 256, 288, &s_publicProfiles, 600u, NULL },
  { 32, 432, &s_playerEmblems, 600u, &PCache_ReleasePlayerEmblem }
};

__int64 __cdecl PCache_Time()
{ 
    return _time64(0);
}

void __cdecl PCache_Lock()
{
    Sys_LockWrite(&s_lock);
}

void __cdecl PCache_Unlock()
{
    Sys_UnlockWrite(&s_lock);
}

bool __cdecl PCache_IsLocked()
{
    return s_lock.writeCount > 0;
}

unsigned __int64 __cdecl PCache_Hash(int controllerIndex, unsigned __int64 xuid)
{
    return xuid ^ HIDWORD(xuid);
}

void __cdecl PCache_Init()
{
    int v0; // eax
    int index; // [esp+0h] [ebp-4h]
    int indexa; // [esp+0h] [ebp-4h]

#ifdef KISAK_LIVE_SERVICE
    if ( live_service->current.enabled )
    {
        memset(s_entries, 0, sizeof(s_entries));
        for ( index = 0; index < 2; ++index )
            memset(
                (unsigned __int8 *)s_componentPools[index].array,
                0,
                s_componentPools[index].componentSize * s_componentPools[index].count);
        for ( indexa = 0; indexa < 256; ++indexa )
        {
            v0 = indexa;
            LODWORD(s_entries[v0].xuid) = 0;
            *(unsigned int *)(v0 * 32 + 173170852) = 0;
        }
        PCache_ProfileInit();
        Cmd_AddCommandInternal("pcacherank", PCache_GetRank_f, &PCache_GetRank_f_VAR);
    }
#endif
}

void __cdecl PCache_GetRank_f()
{
    const char *v0; // eax
    unsigned __int64 v1; // rax
    const char *v2; // eax
    unsigned __int64 v3; // rax
    int rank; // [esp+10h] [ebp-4h] BYREF

    v0 = Cmd_Argv(1);
    v1 = I_atoi64(v0);
    if ( PCache_GetRank(0, v1, &rank, 0) )
        printf("rank: %d\n", rank);
    v2 = Cmd_Argv(2);
    v3 = I_atoi64(v2);
    if ( PCache_GetRank(0, v3, &rank, 0) )
        printf("rank: %d\n", rank);
}

void __cdecl PCache_Update(int controllerIndex)
{
    if ( !Sys_IsMainThread()
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache.cpp", 150, 0, "%s", "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    if ( !LiveStorage_AreWeFetchingStats() && LiveStorage_DoWeHaveAllStats(0) )
    {
        PCache_BatchUpdatePublicProfiles(controllerIndex, s_publicProfiles, 256);
        PCache_BatchUpdatePlayerEmblems(controllerIndex, s_playerEmblems, 32);
    }
}

PCacheComponent *__cdecl PCache_GetOrphanComponent(int controllerIndex, unsigned __int64 xuid, unsigned int type)
{
    PCacheComponentPool *pool; // [esp+8h] [ebp-1Ch]
    unsigned int touchTime; // [esp+Ch] [ebp-18h]
    PCacheComponent *component; // [esp+10h] [ebp-14h]
    unsigned int bestTime; // [esp+14h] [ebp-10h]
    int index; // [esp+18h] [ebp-Ch]
    unsigned __int8 *poolMem; // [esp+1Ch] [ebp-8h]
    PCacheComponent *bestComponent; // [esp+20h] [ebp-4h]

    bestComponent = 0;
    bestTime = -1;
    if ( type >= 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache.cpp",
                    203,
                    0,
                    "type doesn't index PCACHE_NUM_COMPONENTS\n\t%i not in [0, %i)",
                    type,
                    2) )
    {
        __debugbreak();
    }
    if ( !PCache_IsLocked()
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache.cpp", 205, 0, "%s", "PCache_IsLocked()") )
    {
        __debugbreak();
    }
    pool = &s_componentPools[type];
    for ( index = 0; index < pool->count; ++index )
    {
        poolMem = (unsigned __int8 *)s_componentPools[type].array;
        component = (PCacheComponent *)&poolMem[pool->componentSize * index];
        touchTime = component->touchTime;
        if ( component->controllerIndex != -1 )
            ++touchTime;
        if ( component->xuid == xuid && component->controllerIndex == controllerIndex )
        {
            if ( component->type != type
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache.cpp",
                            221,
                            0,
                            "%s",
                            "component->type == type") )
            {
                __debugbreak();
            }
            return component;
        }
        if ( (component->state & 8) == 0 && (component->state & 1) == 0 )
        {
            if ( component->xuid || !bestTime )
            {
                if ( touchTime < bestTime )
                {
                    bestComponent = (PCacheComponent *)&poolMem[pool->componentSize * index];
                    bestTime = touchTime;
                }
            }
            else
            {
                bestComponent = (PCacheComponent *)&poolMem[pool->componentSize * index];
                bestTime = 0;
            }
        }
    }
    if ( !bestComponent
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache.cpp",
                    240,
                    0,
                    "%s",
                    "bestComponent != NULL") )
    {
        __debugbreak();
    }
    if ( s_componentPools[type].release )
        s_componentPools[type].release(bestComponent);
    memset(bestComponent, 0, pool->componentSize);
    bestComponent->xuid = xuid;
    bestComponent->controllerIndex = controllerIndex;
    bestComponent->touchTime = PCache_Time();
    bestComponent->updateTime = 0;
    bestComponent->type = type;
    return bestComponent;
}

PCacheEntry *__cdecl PCache_GetEntry(int controllerIndex, unsigned __int64 xuid)
{
    DWORD v3; // eax
    PCacheEntry *entry; // [esp+10h] [ebp-1Ch]
    PCacheEntry *entrya; // [esp+10h] [ebp-1Ch]
    PCacheEntry *entryb; // [esp+10h] [ebp-1Ch]
    unsigned int baseIndex; // [esp+14h] [ebp-18h]
    int bestOffset; // [esp+18h] [ebp-14h]
    unsigned int neighborhood; // [esp+1Ch] [ebp-10h]
    int offset; // [esp+20h] [ebp-Ch]
    int offseta; // [esp+20h] [ebp-Ch]
    unsigned int bestTime; // [esp+24h] [ebp-8h]
    PCacheEntry *baseEntry; // [esp+28h] [ebp-4h]

    if ( !xuid
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache.cpp",
                    268,
                    0,
                    "%s",
                    "xuid != PCACHE_INVALID_XUID") )
    {
        __debugbreak();
    }
    if ( !PCache_IsLocked()
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache.cpp", 269, 0, "%s", "PCache_IsLocked()") )
    {
        __debugbreak();
    }
    baseIndex = (unsigned int)PCache_Hash(controllerIndex, xuid) % 0x100;
    baseEntry = &s_entries[baseIndex];
    neighborhood = baseEntry->neighborhood;
    while ( 1 )
    {
        if (!_BitScanReverse(&v3, neighborhood))
            v3 = 63;// `CountLeadingZeros'::`2': : notFound;
        offset = 31 - (v3 ^ 0x1F);
        if ( offset == -1 )
            break;
        neighborhood &= ~(1 << offset);
        entry = &s_entries[(offset + baseIndex) % 0x100];
        if ( entry->controllerIndex == controllerIndex && entry->xuid == xuid )
        {
            entry->touchTime = PCache_Time();
            return &s_entries[(offset + baseIndex) % 0x100];
        }
        if ( entry->hash != baseIndex )
            baseEntry->neighborhood &= ~(1 << offset);
    }
    bestOffset = -1;
    bestTime = -1;
    for ( offseta = 0; offseta < 32; ++offseta )
    {
        entrya = &s_entries[(offseta + baseIndex) % 0x100];
        if ( entrya->xuid == xuid
            && entrya->controllerIndex == controllerIndex
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache.cpp",
                        306,
                        0,
                        "%s",
                        "!( entry->xuid == xuid && entry->controllerIndex == controllerIndex )") )
        {
            __debugbreak();
        }
        if ( !entrya->xuid )
        {
            bestOffset = offseta;
            break;
        }
        if ( entrya->touchTime < bestTime )
        {
            bestOffset = offseta;
            bestTime = entrya->touchTime;
        }
    }
    if ( bestOffset == -1
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache.cpp", 320, 0, "%s", "bestOffset != -1") )
    {
        __debugbreak();
    }
    entryb = &s_entries[(bestOffset + baseIndex) % 0x100];
    entryb->neighborhood &= ~1u;
    entryb->hash = baseIndex;
    entryb->xuid = xuid;
    entryb->controllerIndex = controllerIndex;
    entryb->touchTime = PCache_Time();
    entryb->components[0] = 0;
    entryb->components[1] = 0;
    baseEntry->neighborhood |= 1 << bestOffset;
    return entryb;
}

PCacheComponent *__cdecl PCache_GetEntryComponent(PCacheEntry *entry, unsigned int type)
{
    PCacheComponent *component; // [esp+8h] [ebp-4h]

    if ( !entry && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache.cpp", 337, 0, "%s", "entry") )
        __debugbreak();
    if ( type >= 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache.cpp",
                    338,
                    0,
                    "type doesn't index PCACHE_NUM_COMPONENTS\n\t%i not in [0, %i)",
                    type,
                    2) )
    {
        __debugbreak();
    }
    if ( !PCache_IsLocked()
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache.cpp", 339, 0, "%s", "PCache_IsLocked()") )
    {
        __debugbreak();
    }
    component = entry->components[type];
    if ( component
        && LODWORD(entry->xuid) == LODWORD(component->xuid)
        && HIDWORD(entry->xuid) == HIDWORD(component->xuid)
        && entry->controllerIndex == component->controllerIndex )
    {
        return entry->components[type];
    }
    entry->components[type] = PCache_GetOrphanComponent(entry->controllerIndex, entry->xuid, type);
    if ( !entry->components[type]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache.cpp",
                    352,
                    0,
                    "%s",
                    "entry->components[type] != NULL") )
    {
        __debugbreak();
    }
    return entry->components[type];
}

PCacheComponent *__cdecl PCache_GetComponent(int controllerIndex, unsigned __int64 xuid, unsigned int type)
{
    PCacheEntry *entry; // [esp+0h] [ebp-4h]

    if ( type >= 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache.cpp",
                    361,
                    0,
                    "type doesn't index PCACHE_NUM_COMPONENTS\n\t%i not in [0, %i)",
                    type,
                    2) )
    {
        __debugbreak();
    }
    if ( !xuid
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache.cpp",
                    362,
                    0,
                    "%s",
                    "xuid != PCACHE_INVALID_XUID") )
    {
        __debugbreak();
    }
    if ( !PCache_IsLocked()
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache.cpp", 363, 0, "%s", "PCache_IsLocked()") )
    {
        __debugbreak();
    }
    entry = PCache_GetEntry(controllerIndex, xuid);
    if ( !entry && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache.cpp", 366, 0, "%s", "entry") )
        __debugbreak();
    return PCache_GetEntryComponent(entry, type);
}

bool __cdecl PCache_TouchComponent(PCacheComponent *component)
{
    if ( !component
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache.cpp", 373, 0, "%s", "component != NULL") )
    {
        __debugbreak();
    }
    if ( component->type >= 2u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache.cpp",
                    374,
                    0,
                    "component->type doesn't index PCACHE_NUM_COMPONENTS\n\t%i not in [0, %i)",
                    component->type,
                    2) )
    {
        __debugbreak();
    }
    if ( !PCache_IsLocked()
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache.cpp", 375, 0, "%s", "PCache_IsLocked()") )
    {
        __debugbreak();
    }
    component->touchTime = PCache_Time();
    component->state |= 0x10u;
    if ( component->touchTime - component->updateTime > s_componentPools[component->type].updateInterval )
        component->state |= 4u;
    return (component->state & 2) != 0;
}

void __cdecl PCache_InvalidateComponent(PCacheComponent *component)
{
    if ( !component
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache.cpp", 388, 0, "%s", "component != NULL") )
    {
        __debugbreak();
    }
    if ( component->type >= 2u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache.cpp",
                    389,
                    0,
                    "component->type doesn't index PCACHE_NUM_COMPONENTS\n\t%i not in [0, %i)",
                    component->type,
                    2) )
    {
        __debugbreak();
    }
    if ( !PCache_IsLocked()
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache.cpp", 390, 0, "%s", "PCache_IsLocked()") )
    {
        __debugbreak();
    }
    component->touchTime = PCache_Time();
    component->state |= 4u;
}

void __cdecl PCache_ReleasePlayerEmblem(PCacheComponent *comp)
{
    if ( LODWORD(comp[13].xuid) )
        CL_CompositePushCancel(comp[13].xuid);
    if ( HIDWORD(comp[13].xuid) )
        CL_CompositeReleaseImage((GfxImage *)HIDWORD(comp[13].xuid));
    HIDWORD(comp[13].xuid) = 0;
    comp[13].type = 0;
    LODWORD(comp[13].xuid) = 0;
}

void __cdecl PCache_BatchUpdatePlayerEmblems(int controllerIndex, PCachePlayerEmblem *emblems, int count)
{
    int LocalClientNum; // eax
    unsigned __int64 v4; // rax
    GfxImage *v5; // eax
    int v6; // eax
    int v7; // eax
    char *ConfigString; // eax
    Material *BackgroundMaterial; // eax
    int v10; // eax
    __int16 *p_backgroundID; // [esp-Ch] [ebp-54h]
    unsigned __int64 xuid; // [esp-8h] [ebp-50h]
    unsigned __int64 v13; // [esp-8h] [ebp-50h]
    CompositeEmblemLayer *layers; // [esp-8h] [ebp-50h]
    int layer; // [esp+14h] [ebp-34h]
    bool valid; // [esp+1Bh] [ebp-2Dh]
    PCachePublicProfile *profile; // [esp+1Ch] [ebp-2Ch]
    int i; // [esp+20h] [ebp-28h]
    bool generate; // [esp+27h] [ebp-21h]
    int prestige; // [esp+28h] [ebp-20h] BYREF
    int rank; // [esp+30h] [ebp-18h] BYREF
    cg_s *cgameGlob; // [esp+34h] [ebp-14h]
    bool forceDefault; // [esp+3Bh] [ebp-Dh]
    PCachePlayerEmblem *emblem; // [esp+3Ch] [ebp-Ch]
    int index; // [esp+40h] [ebp-8h]
    int clientNum; // [esp+44h] [ebp-4h]

    cgameGlob = 0;
    if ( cgArray )
    {
        LocalClientNum = Com_ControllerIndex_GetLocalClientNum(controllerIndex);
        cgameGlob = CG_GetLocalClientGlobals(LocalClientNum);
    }
    PCache_Lock();
    if ( bg_overridePlayerEmblemColor->modified || bg_overridePlayerEmblemIcon->modified )
    {
        LODWORD(v4) = Live_GetXuid(controllerIndex);
        emblem = (PCachePlayerEmblem *)PCache_GetComponent(controllerIndex, v4, 1u);
        if ( !emblem
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache.cpp", 471, 0, "%s", "emblem") )
        {
            __debugbreak();
        }
        emblem->c.state = 4;
        emblem->c.updateTime = PCache_Time();
        CL_CompositePushCancel(emblem->jobID);
        emblem->jobID = 0;
        if ( emblem->image )
            CL_CompositeReleaseImage(emblem->image);
        emblem->image = 0;
        for ( i = 0; i < 12; ++i )
            CL_CompositeClearLayer(&emblem->layers[i]);
        emblem->layers[0].icon = bg_overridePlayerEmblemIcon->current.integer;
        emblem->layers[0].colorIdx = bg_overridePlayerEmblemColor->current.integer;
        Dvar_ClearModified(bg_overridePlayerEmblemIcon);
        Dvar_ClearModified(bg_overridePlayerEmblemColor);
    }
    for ( index = 0; index < count; ++index )
    {
        emblem = &emblems[index];
        generate = 0;
        forceDefault = 0;
        rank = 0;
        if ( (emblem->c.controllerIndex == controllerIndex || emblem->c.controllerIndex == -1) && emblem->c.xuid )
        {
            if ( emblem->background )
                R_StreamTouchMaterial(emblem->background);
            if ( emblem->jobID )
            {
                if ( CL_CompositeIsComplete(emblem->jobID) )
                {
                    if ( emblem->image )
                        CL_CompositeReleaseImage(emblem->image);
                    v5 = CL_CompositePopImage(emblem->jobID);
                    emblem->image = v5;
                    if ( !emblem->image
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache.cpp",
                                    522,
                                    0,
                                    "%s",
                                    "emblem->image != NULL") )
                    {
                        __debugbreak();
                    }
                    emblem->jobID = 0;
                }
            }
            else if ( (emblem->c.state & 4) != 0 || (emblem->c.state & 0x10) != 0 && !CL_CompositeValidateImage(emblem->image) )
            {
                emblem->c.state &= ~0x10u;
                if ( (emblem->c.xuid & 0xFFFFFF00) == 0xFFFFFF00 && HIDWORD(emblem->c.xuid) == -1 )
                {
                    clientNum = (unsigned __int8)LODWORD(emblem->c.xuid);
                    if ( (unsigned int)clientNum >= 0x20
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache.cpp",
                                    535,
                                    0,
                                    "clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                                    clientNum,
                                    32) )
                    {
                        __debugbreak();
                    }
                    if ( cgameGlob )
                    {
                        rank = cgameGlob->bgs.clientinfo[clientNum].rank;
                        generate = 1;
                    }
                    forceDefault = 1;
                    emblem->backgroundID = clientNum % BG_EmblemsGetBackgroundCount();
                }
                else if ( emblem->c.controllerIndex == -1 )
                {
                    if ( Demo_IsPlaying() )
                    {
                        xuid = emblem->c.xuid;
                        v6 = Com_ControllerIndex_GetLocalClientNum(controllerIndex);
                        clientNum = Demo_GetClientNumForXuid(v6, xuid);
                    }
                    else
                    {
                        v13 = emblem->c.xuid;
                        v7 = Com_ControllerIndex_GetLocalClientNum(controllerIndex);
                        clientNum = PCache_GetClientNumForXuid(v7, v13);
                    }
                    if ( clientNum >= 0 )
                    {
                        if ( (unsigned int)clientNum >= 0x20
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache.cpp",
                                        561,
                                        0,
                                        "clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                                        clientNum,
                                        32) )
                        {
                            __debugbreak();
                        }
                        layers = emblem->layers;
                        p_backgroundID = &emblem->backgroundID;
                        ConfigString = CL_GetConfigString(clientNum + 355);
                        generate = BG_EmblemsReadString(ConfigString, p_backgroundID, layers, 12);
                        forceDefault = !Live_CanConsoleViewContentFromUser(emblem->c.xuid);
                        if ( !cgameGlob
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache.cpp",
                                        564,
                                        0,
                                        "%s",
                                        "cgameGlob") )
                        {
                            __debugbreak();
                        }
                        if ( cgameGlob )
                            rank = cgameGlob->bgs.clientinfo[clientNum].rank;
                    }
                }
                else
                {
                    profile = (PCachePublicProfile *)PCache_GetComponent(controllerIndex, emblem->c.xuid, 0);
                    if ( !profile
                        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache.cpp", 572, 0, "%s", "profile") )
                    {
                        __debugbreak();
                    }
                    if ( (LODWORD(profile->c.xuid) != LODWORD(emblem->c.xuid)
                         || HIDWORD(profile->c.xuid) != HIDWORD(emblem->c.xuid))
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache.cpp",
                                    573,
                                    0,
                                    "%s",
                                    "profile->c.xuid == emblem->c.xuid") )
                    {
                        __debugbreak();
                    }
                    if ( (profile->c.state & 0x20) != 0 )
                        emblem->c.state |= 0x20u;
                    if ( PCache_TouchComponent(&profile->c) )
                    {
                        if ( profile->c.updateTime > emblem->c.updateTime )
                            PCache_GetProfileEmblem(profile, emblem->layers, 12, &emblem->backgroundID);
                        PCache_GetRankInternal(profile, &rank, &prestige);
                        generate = 1;
                        forceDefault = !Live_CanConsoleViewContentFromUser(emblem->c.xuid);
                    }
                }
                if ( generate )
                {
                    valid = 0;
                    for ( layer = 0; layer < 12; ++layer )
                    {
                        if ( !CL_CompositeIsLayerEmpty(&emblem->layers[layer]) )
                        {
                            valid = 1;
                            break;
                        }
                    }
                    if ( !valid || forceDefault )
                        BG_EmblemsGenerateDefault(controllerIndex, rank, emblem->layers, 12);
                    BackgroundMaterial = BG_EmblemsGetBackgroundMaterial(emblem->backgroundID);
                    emblem->background = BackgroundMaterial;
                    if ( emblem->background )
                        R_StreamTouchMaterial(emblem->background);
                    v10 = CL_CompositePushEmblem(emblem->layers, 12);
                    emblem->jobID = v10;
                    if ( emblem->jobID )
                    {
                        emblem->c.state &= ~4u;
                        emblem->c.state |= 2u;
                        emblem->c.updateTime = PCache_Time();
                    }
                }
            }
        }
    }
    PCache_Unlock();
}

int __cdecl PCache_GetClientNumForXuid(int localClientNum, unsigned __int64 xuid)
{
    clientActive_t *LocalClientGlobals; // [esp+4h] [ebp-10h]
    int i; // [esp+Ch] [ebp-8h]
    int clientNum; // [esp+10h] [ebp-4h]

    clientNum = -1;
    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    if ( LocalClientGlobals->snap.valid )
    {
        for ( i = 0; i < LocalClientGlobals->snap.numClients; ++i )
        {
            if ( LocalClientGlobals->parseClients[((_WORD)i + (unsigned __int16)LocalClientGlobals->snap.parseClientsNum)
                                                                                    & 0x7FF].xuid == xuid )
                return LocalClientGlobals->parseClients[((_WORD)i + (unsigned __int16)LocalClientGlobals->snap.parseClientsNum)
                                                                                            & 0x7FF].clientIndex;
        }
    }
    return clientNum;
}

void __cdecl PCache_InvalidatePlayerEmblem(int controllerIndex, unsigned __int64 xuid)
{
    PCacheComponent *emblem; // [esp+4h] [ebp-4h]

    PCache_Lock();
    if ( xuid )
    {
        emblem = PCache_GetComponent(controllerIndex, xuid, 1u);
        if ( !emblem
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache.cpp", 642, 0, "%s", "emblem") )
        {
            __debugbreak();
        }
        PCache_InvalidateComponent(emblem);
        PCache_Unlock();
    }
    else
    {
        PCache_Unlock();
    }
}

GfxImage *__cdecl PCache_GetPlayerEmblem(int controllerIndex, unsigned __int64 xuid)
{
    GfxImage *image; // [esp+0h] [ebp-14h]
    PCachePlayerEmblem *emblem; // [esp+10h] [ebp-4h]

    PCache_Lock();
    if ( !xuid )
        goto LABEL_2;
    emblem = (PCachePlayerEmblem *)PCache_GetComponent(controllerIndex, xuid, 1u);
    if ( !emblem && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache.cpp", 658, 0, "%s", "emblem") )
        __debugbreak();
    if ( PCache_TouchComponent(&emblem->c) )
    {
        image = emblem->image;
        PCache_Unlock();
        return image;
    }
    else
    {
LABEL_2:
        PCache_Unlock();
        return 0;
    }
}

__int16 __cdecl PCache_GetPlayerEmblemBackgroundID(int controllerIndex, unsigned __int64 xuid)
{
    __int16 backgroundID; // [esp+0h] [ebp-Ch]
    PCachePlayerEmblem *emblem; // [esp+8h] [ebp-4h]

    PCache_Lock();
    if ( !xuid )
        goto LABEL_2;
    emblem = (PCachePlayerEmblem *)PCache_GetComponent(controllerIndex, xuid, 1u);
    if ( !emblem && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\live\\live_pcache.cpp", 676, 0, "%s", "emblem") )
        __debugbreak();
    if ( PCache_TouchComponent(&emblem->c) )
    {
        backgroundID = emblem->backgroundID;
        PCache_Unlock();
        return backgroundID;
    }
    else
    {
LABEL_2:
        PCache_Unlock();
        return -1;
    }
}

