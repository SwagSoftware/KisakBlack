#include "fx_system.h"
#include "fx_random.h"
#include "fx_marks.h"
#include "fx_dvars.h"
#include "fx_unique_handle.h"
#include "fx_load_obj.h"
#include "fx_archive.h"
#include "fx_draw.h"
#include "fx_update.h"
#include "fx_update_util.h"
#include <DynEntity/DynEntity_load_obj.h>
#include "fx_sort.h"
#include <gfx_d3d/r_warn.h>

#include <algorithm>
#include "fx_wind.h"
#include <cgame/cg_sound.h>
#include <sound/snd_public_async.h>
#include <gfx_d3d/rb_light.h>
#include <cgame_mp/cg_main_mp.h>
#include <bgame/bg_wind.h>

FxSystemContainer *fx_systemPool;
FxSystemBuffers *fx_systemBufferPool;
int fx_serverVisClient;

int __cdecl FX_AllocateClientMemory_SizeRequired(int maxLocalClients)
{
    return 405536 * maxLocalClients + 127 + 155688 * maxLocalClients + 254;
}

void __cdecl FX_AllocateClientMemory(HunkUser *hunk, int maxLocalClients)
{
    int i; // [esp+0h] [ebp-4h]

    fx_maxLocalClients = maxLocalClients;
    fx_systemPool = (FxSystemContainer *)Hunk_UserAlloc(hunk, 2976 * maxLocalClients, 128, "fx_systemPool");
    memset(fx_systemPool, 0, 2976 * maxLocalClients);
    fx_systemBufferPool = (FxSystemBuffers *)Hunk_UserAlloc(hunk, 402560 * maxLocalClients, 128, "fx_systemBufferPool");
    memset(fx_systemBufferPool, 0, 402560 * maxLocalClients);
    fx_marksSystemPool = (FxMarksSystem *)Hunk_UserAlloc(hunk, 155688 * maxLocalClients, 128, "fx_marksSystemPool");
    memset(fx_marksSystemPool, 0, 155688 * maxLocalClients);
    for ( i = 0; i < maxLocalClients; ++i )
    {
        fx_systemPool[i].system.shared = &fx_systemPool[i].shared;
        fx_marksSystemPool[i].frameCount = 1;
    }
}

void __cdecl FX_FreeClientMemory(HunkUser *hunk)
{
    fx_maxLocalClients = 0;
    if ( fx_systemBufferPool )
        Hunk_UserFree(hunk, fx_systemBufferPool);
    if ( fx_systemPool )
        Hunk_UserFree(hunk, fx_systemPool);
    if ( fx_marksSystemPool )
        Hunk_UserFree(hunk, fx_marksSystemPool);
    fx_systemPool = 0;
    fx_systemBufferPool = 0;
    fx_marksSystemPool = 0;
}

FxSystemContainer *__cdecl FX_GetSystem(int localClientNum)
{
    if ( localClientNum
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                    193,
                    0,
                    "%s\n\t(localClientNum) = %i",
                    "(localClientNum == 0)",
                    localClientNum) )
    {
        __debugbreak();
    }
    return fx_systemPool;
}

FxSystemContainer *__cdecl FX_GetSystemRemote(int localClientNum)
{
    return FX_GetSystem(localClientNum);
}

FxSystemBuffers *__cdecl FX_GetSystemBuffers(int localClientNum)
{
    if ( localClientNum
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                    215,
                    0,
                    "%s\n\t(localClientNum) = %i",
                    "(localClientNum == 0)",
                    localClientNum) )
    {
        __debugbreak();
    }
    return fx_systemBufferPool;
}

void __cdecl FX_LinkSystemBuffers(FxSystem *system, FxSystemBuffers *systemBuffers)
{
    system->elems = systemBuffers->elems;
    system->effects = (FxEffectContainer *)systemBuffers;
    system->trails = systemBuffers->trails;
    system->trailElems = systemBuffers->trailElems;
    system->visState = systemBuffers->visState;
    system->uniqueHandleDb = &systemBuffers->uniqueHandleDb;
}

void __cdecl FX_InitSystem(int localClientNum)
{
    FxMarksSystem *marksSystem; // [esp+4h] [ebp-Ch]
    FxSystemContainer *system; // [esp+8h] [ebp-8h]
    FxSystemBuffers *systemBuffers; // [esp+Ch] [ebp-4h]

    system = FX_GetSystem(localClientNum);
    if ( !system
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp", 499, 0, "%s", "system") )
    {
        __debugbreak();
    }
    memset(system, 0, 0x360u);
    system->system.shared = &system->shared;
    systemBuffers = FX_GetSystemBuffers(localClientNum);
    if ( !systemBuffers
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp", 505, 0, "%s", "systemBuffers") )
    {
        __debugbreak();
    }
    memset(systemBuffers, 0, sizeof(FxSystemBuffers));
    FX_LinkSystemBuffers(&system->system, systemBuffers);
    FX_RegisterDvars();
    FX_CreateDevGui();
    //BLOPS_NULLSUB();
    FX_ResetSystem(&system->system);
    system->system.msecNow = 0;
    system->system.msecDraw = -1;
    system->system.cameraPrev.isValid = 1;
    system->system.cameraPrev.frustumPlaneCount = 0;
    system->system.extraCamera.isValid = 0;
    system->system.extraCamera.frustumPlaneCount = 0;
    system->system.extraCameraPre.isValid = 0;
    system->system.extraCameraPre.frustumPlaneCount = 0;
    system->system.frameCount = 1;
    if ( system->system.shared->firstActiveEffect
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                    528,
                    1,
                    "%s",
                    "system->shared->firstActiveEffect == 0") )
    {
        __debugbreak();
    }
    if ( system->system.shared->firstNewEffect
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                    529,
                    1,
                    "%s",
                    "system->shared->firstNewEffect == 0") )
    {
        __debugbreak();
    }
    if ( system->system.shared->firstFreeEffect
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                    530,
                    1,
                    "%s",
                    "system->shared->firstFreeEffect == 0") )
    {
        __debugbreak();
    }
    marksSystem = FX_GetMarksSystem(localClientNum);
    FX_InitMarksSystem(marksSystem);
    system->system.localClientNum = localClientNum;
    system->system.isInitialized = 1;
    fx_serverVisClient = localClientNum;
    UniqueHandleInit(system->system.uniqueHandleDb);
}

void __cdecl FX_ResetSystem(FxSystem *system)
{
    FxPool<FxTrail,FxTrail> *trails; // [esp+0h] [ebp-28h]
    int k; // [esp+8h] [ebp-20h]
    FxPool<FxTrailElem,FxTrailElem> *trailElems; // [esp+Ch] [ebp-1Ch]
    int j; // [esp+14h] [ebp-14h]
    FxPool<FxElem,FxElemContainer> *elems; // [esp+18h] [ebp-10h]
    int i; // [esp+20h] [ebp-8h]
    int effectIndex; // [esp+24h] [ebp-4h]

    system->effects->effect.def = 0;
    for ( effectIndex = 0; effectIndex < 1024; ++effectIndex )
        system->shared->allEffectHandles[effectIndex] = FX_EffectToHandle(system, &system->effects[effectIndex]);
    system->shared->firstActiveEffect = 0;
    system->shared->firstNewEffect = 0;
    system->shared->firstFreeEffect = 0;
    system->shared->iteratorCount = 0;
    Fx_ResetPriority(system);
    elems = system->elems;
    system->shared->firstFreeElem = 0;
    for ( i = 0; i < 2047; ++i )
        elems[i].nextFree = i + 1;
    elems[i].nextFree = -1;
    system->shared->activeElemCount = 0;
    trailElems = system->trailElems;
    system->shared->firstFreeTrailElem = 0;
    for ( j = 0; j < 2047; ++j )
        trailElems[j].nextFree = j + 1;
    trailElems[j].nextFree = -1;
    system->shared->activeTrailElemCount = 0;
    trails = system->trails;
    system->shared->firstFreeTrail = 0;
    for ( k = 0; k < 127; ++k )
        trails[k].nextFree = k + 1;
    trails[k].nextFree = -1;
    system->shared->activeTrailCount = 0;
    system->shared->activeSpotLightEffectCount = 0;
    system->shared->activeSpotLightElemCount = 0;
    system->gfxCloudCount = 0;
    system->visState->blockerCount = 0;
    system->visStateBufferRead = system->visState;
    system->visStateBufferWrite = system->visState + 1;
}

int __cdecl FX_EffectToHandle(FxSystem *system, FxEffectContainer *effect)
{
    const char *v2; // eax

    if ( !system
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\effectscore\\fx_system.h", 384, 0, "%s", "system") )
    {
        __debugbreak();
    }
    if ( !effect || effect < system->effects || effect >= &system->effects[1024] )
    {
        v2 = va("%p %p", system->effects, effect);
        if ( !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\effectscore\\fx_system.h",
                        391,
                        0,
                        "%s\n\t%s",
                        "effect && effect >= &system->effects[0].effect && effect < &system->effects[FX_EFFECT_LIMIT].effect",
                        v2) )
            __debugbreak();
    }
    return ((char *)effect - (char *)system->effects) / 4;
}

void __cdecl FX_ShutdownSystem(int localClientNum)
{
    FxSystemContainer *system; // [esp+0h] [ebp-8h]
    FxSystemBuffers *systemBuffers; // [esp+4h] [ebp-4h]

    system = FX_GetSystem(localClientNum);
    systemBuffers = FX_GetSystemBuffers(localClientNum);
    if ( !system
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp", 556, 0, "%s", "system") )
    {
        __debugbreak();
    }
    memset(system, 0, 0x360u);
    if ( !systemBuffers
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp", 558, 0, "%s", "systemBuffers") )
    {
        __debugbreak();
    }
    memset(systemBuffers, 0, sizeof(FxSystemBuffers));
    if ( system->system.isInitialized
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                    560,
                    1,
                    "%s",
                    "!system->isInitialized") )
    {
        __debugbreak();
    }
    Fx_ResetPriority(&system->system);
    FX_UnregisterAll();
}

void __cdecl FX_DelRefToEffect(FxSystem *system, FxEffectContainer *effect, unsigned int stackCount)
{
    if ( !effect
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\effectscore\\fx_system.h", 526, 0, "%s", "effect") )
    {
        __debugbreak();
    }
    if ( (effect->atomics.status & 0x3FFF) == 1 )
        FX_EffectNoLongerReferenced(system, effect, stackCount);
    if ( (effect->atomics.status & 0x3FFF) != 0 )
        _InterlockedExchangeAdd(&effect->atomics.status, 0xFFFFFFFF);
}

void __cdecl FX_EffectNoLongerReferenced(FxSystem *system, FxEffectContainer *remoteEffect, unsigned int stackCount)
{
    const char *v3; // eax
    FxEffectContainer *owner; // [esp+Ch] [ebp-Ch]
    int oldStatusValue; // [esp+14h] [ebp-4h]

    if ( !remoteEffect
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp", 759, 0, "%s", "remoteEffect") )
    {
        __debugbreak();
    }
    if ( (remoteEffect->atomics.status & 0x3FFF) != 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                    763,
                    0,
                    "%s",
                    "FX_GetEffectRef(remoteEffect) == 1") )
    {
        __debugbreak();
    }
    if ( (remoteEffect->atomics.status & 0x7FE0000) != 0 )
    {
        v3 = va("%s, %i", remoteEffect->effect.def->name, remoteEffect->atomics.status);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                        765,
                        0,
                        "%s\n\t%s",
                        "(FX_GetAtomics(effect)->status & FX_STATUS_OWNED_EFFECTS_MASK) == 0",
                        v3) )
            __debugbreak();
    }
    owner = FX_EffectFromHandle(system, remoteEffect->effect.owner);
    if ( stackCount > 0x14 && stackCount < 0x28 )
    {
        Com_DPrintf(21, "-------------------------------------------------------------------\n");
        Com_DPrintf(21, "FX_EffectNoLongerReferenced() potential stack overflow, call %u\n", stackCount);
        if ( remoteEffect->effect.def && remoteEffect->effect.def->name )
            Com_DPrintf(
                21,
                "effect 0x%08x \"%s\", status 0x%08x, owner 0x%08x\n",
                remoteEffect,
                remoteEffect->effect.def->name,
                remoteEffect->atomics.status,
                owner);
        else
            Com_DPrintf(
                21,
                "effect 0x%08x \"%s\", status 0x%08x, owner 0x%08x\n",
                remoteEffect,
                "",
                remoteEffect->atomics.status,
                owner);
    }
    if ( remoteEffect != owner )
    {
        if ( (owner->atomics.status & 0x7FE0000) == 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                        785,
                        0,
                        "%s\n\t(FX_GetAtomics(owner)->status) = %i",
                        "((FX_GetAtomics(owner)->status & FX_STATUS_OWNED_EFFECTS_MASK) > 0)",
                        owner->atomics.status) )
        {
            __debugbreak();
        }
        oldStatusValue = _InterlockedExchangeAdd(&owner->atomics.status, 0xFFFE0000);
        if ( (oldStatusValue & 0xF801FFFF) != ((oldStatusValue - 0x20000) & 0xF801FFFF)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                        790,
                        0,
                        "%s\n\t(oldStatusValue) = %i",
                        "((oldStatusValue & ~FX_STATUS_OWNED_EFFECTS_MASK) == ((oldStatusValue - (1 << FX_STATUS_OWNED_EFFECTS_SHIFT)"
                        ") & ~FX_STATUS_OWNED_EFFECTS_MASK))",
                        oldStatusValue) )
        {
            __debugbreak();
        }
        FX_DelRefToEffect(system, (FxEffectContainer*)&owner->effect, stackCount + 1);
    }
    system->needsGarbageCollection = 1;
}

void __cdecl FX_RunPrioritySorting(FxSystem *system)
{
    int v1; // [esp+0h] [ebp-107Ch]
    FxEffectContainer *effect; // [esp+3Ch] [ebp-1040h]
    int j; // [esp+40h] [ebp-103Ch]
    volatile int status; // [esp+4Ch] [ebp-1030h]
    FxEffectContainer *v5; // [esp+58h] [ebp-1024h]
    int efPriority; // [esp+5Ch] [ebp-1020h]
    volatile int i; // [esp+64h] [ebp-1018h]
    unsigned __int16 v8; // [esp+68h] [ebp-1014h]
    unsigned int v9[1024]; // [esp+6Ch] [ebp-1010h] BYREF
    volatile int firstActiveEffect; // [esp+106Ch] [ebp-10h]
    int v11; // [esp+1070h] [ebp-Ch]
    int FrameWarningPriority; // [esp+1074h] [ebp-8h]
    volatile int firstNewEffect; // [esp+1078h] [ebp-4h]

    FrameWarningPriority = FX_GetFrameWarningPriority(system);
    v11 = 0;
    firstActiveEffect = system->shared->firstActiveEffect;
    firstNewEffect = system->shared->firstNewEffect;
    for ( i = firstActiveEffect; i != firstNewEffect; ++i )
    {
        v8 = system->shared->allEffectHandles[i & 0x3FF];
        v5 = FX_EffectFromHandle(system, v8);
        status = v5->atomics.status;
        if ( (status & 0x60000000) == 0 && (status & 0x3FFF) != 0 && v5->effect.owner == v8 && (status & 0x7FE0000) == 0 )
        {
            if ( v5->effect.def )
            {
                efPriority = v5->effect.def->efPriority;
                if ( efPriority < FrameWarningPriority && v5->effect.def->elemDefCountLooping <= 0 )
                {
                    if ( (unsigned int)v11 >= 0x400
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                                    943,
                                    0,
                                    "numRemovalCandidates not in [0, FX_EFFECT_LIMIT-1]\n\t%i not in [%i, %i]",
                                    v11,
                                    0,
                                    1023) )
                    {
                        __debugbreak();
                    }
                    v9[v11++] = v8 | (efPriority << 16);
                }
            }
        }
    }
    if ( v11 )
    {
        //std::_Sort<unsigned int *,int>(v9, &v9[v11], (4 * v11) >> 2);
        std::sort(&v9[0], &v9[v11]);
        if ( v11 > 2 )
            v1 = 2;
        else
            v1 = v11;
        for ( j = 0; j < v1; ++j )
        {
            for ( effect = FX_EffectFromHandle(system, v9[j]);
                        _InterlockedExchangeAdd(&effect->atomics.status, 0x20000000u) >= 0x20000000;
                        _InterlockedExchangeAdd(&effect->atomics.status, 0xE0000000) )
            {
                ;
            }
            FX_KillEffect(system, effect);
            _InterlockedExchangeAdd(&effect->atomics.status, 0xE0000000);
        }
        FX_InvalidateFrameWarningPriority(system);
    }
}

void __cdecl FX_RunGarbageCollectionAndPrioritySort(FxSystem *system)
{
    FxEffectContainer *localEffectContainer; // [esp+18h] [ebp-81Ch]
    unsigned __int16 effectHandle; // [esp+1Ch] [ebp-818h]
    unsigned int freedCount; // [esp+20h] [ebp-814h]
    unsigned __int16 freedHandles[1026]; // [esp+24h] [ebp-810h]
    FxEffect *effect; // [esp+82Ch] [ebp-8h]
    int activeIndex; // [esp+830h] [ebp-4h]

    effect = 0;
    if ( !system
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp", 1025, 0, "%s", "system") )
    {
        __debugbreak();
    }
    if ( system->needsGarbageCollection || system->hasStaleAttachedFx || FX_NeedsPrioritySorting(system) )
    {
        FX_RemoveStaleAttachedEffects(system);
        if ( FX_BeginIteratingOverEffects_Exclusive(system) )
        {
            if ( FX_NeedsPrioritySorting(system) )
                FX_RunPrioritySorting(system);
            if ( system->needsGarbageCollection )
            {
                system->needsGarbageCollection = 0;
                activeIndex = system->shared->firstNewEffect;
                freedCount = 0;
                while ( activeIndex != system->shared->firstActiveEffect )
                {
                    effectHandle = system->shared->allEffectHandles[--activeIndex & 0x3FF];
                    effect = (FxEffect *)FX_EffectFromHandle(system, effectHandle);
                    if ( ((int)effect[1].def & 0x3FFF) != 0 )
                    {
                        system->shared->allEffectHandles[((_WORD)freedCount + (_WORD)activeIndex) & 0x3FF] = effectHandle;
                    }
                    else
                    {
                        FX_RunGarbageCollection_FreeTrails(system, effect);
                        FX_RunGarbageCollection_FreeSpotLight(system, effectHandle);
                        freedHandles[freedCount++] = effectHandle;
                    }
                }
                while ( freedCount )
                {
                    system->shared->allEffectHandles[activeIndex++ & 0x3FF] = freedHandles[--freedCount];
                    effect = (FxEffect *)FX_EffectFromHandle(system, freedHandles[freedCount]);
                    localEffectContainer = (FxEffectContainer *)effect;
                    UniqueHandleRelease(system, effect);
                    localEffectContainer->effect.def = 0;
                }
                system->shared->firstActiveEffect = activeIndex;
                system->shared->iteratorCount = 0;
            }
            else
            {
                system->shared->iteratorCount = 0;
            }
        }
    }
}

bool __cdecl FX_BeginIteratingOverEffects_Exclusive(FxSystem *system)
{
    if ( system->isArchiving
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\effectscore\\fx_system.h",
                    687,
                    0,
                    "%s",
                    "!system->isArchiving") )
    {
        __debugbreak();
    }
    return _InterlockedCompareExchange(&system->shared->iteratorCount, -1, 0) == 0;
}

void __cdecl FX_RunGarbageCollection_FreeSpotLight(FxSystem *system, unsigned __int16 effectHandle)
{
    if ( system->shared->activeSpotLightEffectCount && system->shared->activeSpotLightEffectHandle == effectHandle )
    {
        if ( system->shared->activeSpotLightEffectCount != 1
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                        807,
                        0,
                        "%s",
                        "system->shared->activeSpotLightEffectCount == 1") )
        {
            __debugbreak();
        }
        _InterlockedExchangeAdd(&system->shared->activeSpotLightEffectCount, 0xFFFFFFFF);
    }
}

FxPool<FxTrail, FxTrail> *__cdecl FX_AllocPool_Generic_FxTrail_FxTrail_(
    FxEffect *remoteEffect,
    unsigned __int32 *firstFreeIndex,
    FxPool<FxTrail, FxTrail> *pool,
    volatile unsigned int *activeCount)
{
    unsigned __int32 itemIndex; // [esp+8Ch] [ebp-8h]
    unsigned __int32 nextFree; // [esp+90h] [ebp-4h]

    do
    {
        itemIndex = *firstFreeIndex;
        if (*firstFreeIndex == -1)
            return 0;
        nextFree = pool[itemIndex].nextFree;
    } while (_InterlockedCompareExchange((volatile unsigned __int32 *)firstFreeIndex, nextFree, itemIndex) != itemIndex);
    if (itemIndex != -1
        && itemIndex >= 0x80
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
            247,
            0,
            "%s",
            "itemIndex == -1 || (itemIndex >= 0 && itemIndex < ITEM_TYPE::POOL_SIZE)"))
    {
        __debugbreak();
    }
    if (nextFree != -1
        && nextFree >= 0x80
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
            248,
            0,
            "%s",
            "nextFree == -1 || (nextFree >= 0 && nextFree < ITEM_TYPE::POOL_SIZE)"))
    {
        __debugbreak();
    }
    _InterlockedExchangeAdd(activeCount, 1u);
    return &pool[itemIndex];
}

FxPool<FxTrailElem, FxTrailElem> *__cdecl FX_AllocPool_Generic_FxTrailElem_FxTrailElem_(
    FxEffect *remoteEffect,
    unsigned __int32 *firstFreeIndex,
    FxPool<FxTrailElem, FxTrailElem> *pool,
    volatile unsigned int *activeCount)
{
    unsigned __int32 itemIndex; // [esp+8Ch] [ebp-8h]
    unsigned __int32 nextFree; // [esp+90h] [ebp-4h]

    do
    {
        itemIndex = *firstFreeIndex;
        if (*firstFreeIndex == -1)
            return 0;
        nextFree = pool[itemIndex].nextFree;
    } while (_InterlockedCompareExchange((volatile unsigned __int32 *)firstFreeIndex, nextFree, itemIndex) != itemIndex);
    if (itemIndex != -1
        && itemIndex >= 0x800
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
            247,
            0,
            "%s",
            "itemIndex == -1 || (itemIndex >= 0 && itemIndex < ITEM_TYPE::POOL_SIZE)"))
    {
        __debugbreak();
    }
    if (nextFree != -1
        && nextFree >= 0x800
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
            248,
            0,
            "%s",
            "nextFree == -1 || (nextFree >= 0 && nextFree < ITEM_TYPE::POOL_SIZE)"))
    {
        __debugbreak();
    }
    _InterlockedExchangeAdd(activeCount, 1u);
    return &pool[itemIndex];
}

void __cdecl FX_FreePool_Generic_FxTrail_FxTrail_(
    FxEffect *remoteEffect,
    FxTrail *item_slim,
    unsigned __int32 *firstFreeIndex,
    FxPool<FxTrail, FxTrail> *pool)
{
    unsigned __int32 nextFree; // [esp+8Ch] [ebp-8h]
    unsigned __int32 freedIndex; // [esp+90h] [ebp-4h]

    freedIndex = ((char *)item_slim - (char *)pool) >> 3;
    if (freedIndex >= 0x80
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
            300,
            0,
            "%s",
            "freedIndex >= 0 && freedIndex < ITEM_TYPE::POOL_SIZE"))
    {
        __debugbreak();
    }
    do
    {
        nextFree = *firstFreeIndex;
        if (*firstFreeIndex == freedIndex
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                309,
                0,
                "%s",
                "nextFree != freedIndex"))
        {
            __debugbreak();
        }
        *(_DWORD *)&item_slim->nextTrailHandle = nextFree;
    } while (_InterlockedCompareExchange((volatile unsigned __int32 *)firstFreeIndex, freedIndex, nextFree) != nextFree);
    if (nextFree != -1
        && nextFree >= 0x80
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
            316,
            0,
            "%s",
            "nextFree == -1 || (nextFree >= 0 && nextFree < ITEM_TYPE::POOL_SIZE)"))
    {
        __debugbreak();
    }
}

void __cdecl FX_RunGarbageCollection_FreeTrails(FxSystem *system, FxEffect *effect)
{
    unsigned __int16 nextTrailHandle; // [esp+98h] [ebp-Ch]
    unsigned __int16 firstTrailHandle; // [esp+9Ch] [ebp-8h]
    FxPool<FxTrail,FxTrail> *remoteTrail; // [esp+A0h] [ebp-4h]

    if ( effect->firstTrailHandle != 0xFFFF
        && effect->firstTrailHandle == FX_TrailFromHandle(system, effect->firstTrailHandle)->item.nextTrailHandle
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                    821,
                    0,
                    "%s",
                    "effect->firstTrailHandle != remoteTrail->nextTrailHandle") )
    {
        __debugbreak();
    }
    firstTrailHandle = effect->firstTrailHandle;
    while ( firstTrailHandle != 0xFFFF )
    {
        remoteTrail = FX_TrailFromHandle(system, firstTrailHandle);
        nextTrailHandle = remoteTrail->item.nextTrailHandle;
        if ( firstTrailHandle == remoteTrail->item.nextTrailHandle
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                        834,
                        0,
                        "%s",
                        "firstTrailHandle != nextTrailHandle") )
        {
            __debugbreak();
        }
        firstTrailHandle = nextTrailHandle;
        remoteTrail->nextFree = 0;
        *(unsigned int *)&remoteTrail->item.lastElemHandle = 0;
        FX_FreePool_Generic_FxTrail_FxTrail_(
            0,
            (FxTrail *)remoteTrail,
            (unsigned __int32 *)&system->shared->firstFreeTrail,
            system->trails);
        _InterlockedExchangeAdd(&system->shared->activeTrailCount, 0xFFFFFFFF);
        if ( system->shared->activeTrailCount < 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                        847,
                        0,
                        "%s",
                        "system->shared->activeTrailCount >= 0") )
        {
            __debugbreak();
        }
    }
    effect->firstTrailHandle = -1;
}

void __cdecl FX_RemoveStaleAttachedEffects(FxSystem *system)
{
    volatile unsigned __int32 *p_status; // [esp+4h] [ebp-2Ch]
    signed __int32 v2; // [esp+8h] [ebp-28h]
    int status; // [esp+1Ch] [ebp-14h]
    FxEffectContainer *effect; // [esp+20h] [ebp-10h]
    volatile int activeIndex; // [esp+24h] [ebp-Ch]

    if ( system->hasStaleAttachedFx )
    {
        system->hasStaleAttachedFx = 0;
        FX_BeginIteratingOverEffects_Cooperative(system);
        for ( activeIndex = system->shared->firstActiveEffect; activeIndex != system->shared->firstNewEffect; ++activeIndex )
        {
            effect = FX_EffectFromHandle(system, system->shared->allEffectHandles[activeIndex & 0x3FF]);
            status = effect->atomics.status;
            if ( (status & 0x60000000) == 0 && (status & 0x4000) != 0 )
            {
                p_status = &effect->atomics.status;
                do
                    v2 = *p_status;
                while ( _InterlockedCompareExchange(p_status, v2 & 0xFFFFBFFF, v2) != v2 );
                FX_ThroughWithEffect(system, effect, 0);
            }
        }
        if ( !_InterlockedDecrement(&system->shared->iteratorCount) )
            FX_RunGarbageCollectionAndPrioritySort(system);
    }
}

char __cdecl FX_EffectAffectsGameplay(const FxEffectDef *remoteEffectDef)
{
    const FxElemDef *elemDefs; // [esp+4h] [ebp-24h]
    bool result; // [esp+Fh] [ebp-19h]
    const FxElemDef *elemDef; // [esp+10h] [ebp-18h]
    unsigned int elemDefCount; // [esp+14h] [ebp-14h]
    FxElemVisuals *visArray; // [esp+18h] [ebp-10h]
    unsigned int visIndex; // [esp+20h] [ebp-8h]
    unsigned int elemDefIndex; // [esp+24h] [ebp-4h]

    if ( !remoteEffectDef
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                    1135,
                    0,
                    "%s",
                    "remoteEffectDef") )
    {
        __debugbreak();
    }
    elemDefCount = remoteEffectDef->elemDefCountEmission
                             + remoteEffectDef->elemDefCountOneShot
                             + remoteEffectDef->elemDefCountLooping;
    result = 0;
    if ( elemDefCount )
    {
        elemDefs = remoteEffectDef->elemDefsEA;
        for ( elemDefIndex = 0; elemDefIndex < elemDefCount; ++elemDefIndex )
        {
            elemDef = &elemDefs[elemDefIndex];
            if ( (elemDef->flags & 0x1000) != 0 )
                return 1;
            if ( elemDef->effectOnDeath.handle && FX_EffectAffectsGameplay(elemDef->effectOnDeath.handle) )
                return 1;
            if ( elemDef->effectOnImpact.handle && FX_EffectAffectsGameplay(elemDef->effectOnImpact.handle) )
                return 1;
            if ( elemDef->effectEmitted.handle && FX_EffectAffectsGameplay(elemDef->effectEmitted.handle) )
                return 1;
            if ( elemDef->elemType == 12 )
            {
                if ( elemDef->visualCount == 1 )
                {
                    if ( FX_EffectAffectsGameplay(elemDef->visuals.instance.effectDef.handle) )
                        return 1;
                }
                else
                {
                    visArray = elemDef->visuals.array;
                    for ( visIndex = 0; visIndex < elemDef->visualCount; ++visIndex )
                    {
                        if ( FX_EffectAffectsGameplay(visArray[visIndex].effectDef.handle) )
                        {
                            result = 1;
                            break;
                        }
                    }
                }
            }
        }
    }
    return result;
}

void __cdecl FX_SpawnEffect_AllocTrails(FxSystem *system, FxEffect *effect, FxEffect *remoteEffect)
{
    const FxEffectDef *def; // [esp+Ch] [ebp-1Ch]
    FxPool<FxTrail,FxTrail> *remoteTrail; // [esp+14h] [ebp-14h]
    int elemDefCount; // [esp+18h] [ebp-10h]
    int elemDefIter; // [esp+1Ch] [ebp-Ch]
    int localTrail; // [esp+20h] [ebp-8h]
    int localTrail_4; // [esp+24h] [ebp-4h]

    def = effect->def;
    if ( !effect->def
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp", 1232, 0, "%s", "def") )
    {
        __debugbreak();
    }
    elemDefCount = def->elemDefCountOneShot + def->elemDefCountLooping + def->elemDefCountEmission;
    for ( elemDefIter = 0; elemDefIter != elemDefCount; ++elemDefIter )
    {
        if ( effect->def->elemDefsEA[elemDefIter].elemType == 5 )
        {
            remoteTrail = FX_AllocTrail(remoteEffect, system);
            if ( !remoteTrail )
                return;
            LOWORD(localTrail) = effect->firstTrailHandle;
            BYTE2(localTrail_4) = elemDefIter;
            if ( (char)elemDefIter != elemDefIter
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                            1248,
                            0,
                            "%s\n\t(elemDefIter) = %i",
                            "(localTrail.defIndex == elemDefIter)",
                            elemDefIter) )
            {
                __debugbreak();
            }
            HIWORD(localTrail) = -1;
            LOWORD(localTrail_4) = -1;
            HIBYTE(localTrail_4) = 0;
            effect->firstTrailHandle = FX_TrailToHandle(system, (FxTrail *)remoteTrail);
            remoteTrail->nextFree = localTrail;
            *(unsigned int *)&remoteTrail->item.lastElemHandle = localTrail_4;
        }
    }
}

int __cdecl FX_TrailToHandle(FxSystem *system, FxTrail *trail)
{
    if ( !system
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\effectscore\\fx_system.h", 500, 0, "%s", "system") )
    {
        __debugbreak();
    }
    return FX_PoolToHandle_Generic<FxTrail,FxTrail,128>(system->trails, trail);
}

FxPool<FxTrail,FxTrail> *__cdecl FX_AllocTrail(FxEffect *remoteEffect, FxSystem *system)
{
    return FX_AllocPool_Generic_FxTrail_FxTrail_(
                     remoteEffect,
                     (unsigned __int32 *)&system->shared->firstFreeTrail,
                     system->trails,
                     &system->shared->activeTrailCount);
}

void __cdecl FX_CalculatePackedLighting(FxEffect *effect, const float *origin)
{
    float color[3]; // [esp+0h] [ebp-Ch] BYREF

    R_GetAverageLightingAtPoint(origin, color);
    effect->lightR = (int)(float)(255.0 * color[0]);
    effect->lightG = (int)(float)(255.0 * color[1]);
    effect->lightB = (int)(float)(255.0 * color[2]);
}

bool __cdecl FX_NeedsPrioritySorting(const FxSystem *system)
{
    return fx_priority_enable && fx_priority_enable->current.enabled && FX_GetFrameWarningPriority(system) >= 0;
}

void __cdecl Fx_ResetPriority(FxSystem *system)
{
    system->priorityInfo.warningHigh[0] = -1;
    system->priorityInfo.warningHigh[1] = -1;
    system->priorityInfo.readIndex = 0;
}

void __cdecl FX_SetWarningPriority(FxSystem *system, unsigned __int8 priority)
{
    int v2; // [esp+0h] [ebp-10h]
    int writeIdx; // [esp+8h] [ebp-8h]

    if ( fx_priority_enable && fx_priority_enable->current.enabled )
    {
        if ( system->priorityInfo.readIndex >= 2u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                        1426,
                        0,
                        "system->priorityInfo.readIndex not in [0, 1]\n\t%i not in [%i, %i]",
                        system->priorityInfo.readIndex,
                        0,
                        1) )
        {
            __debugbreak();
        }
        writeIdx = 1 - system->priorityInfo.readIndex;
        if ( system->priorityInfo.warningHigh[writeIdx] < priority )
            v2 = priority;
        else
            v2 = system->priorityInfo.warningHigh[writeIdx];
        system->priorityInfo.warningHigh[writeIdx] = v2;
    }
}

void __cdecl FX_InvalidateFrameWarningPriority(FxSystem *system)
{
    if ( system->priorityInfo.readIndex >= 2u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                    1441,
                    0,
                    "system->priorityInfo.readIndex not in [0, 1]\n\t%i not in [%i, %i]",
                    system->priorityInfo.readIndex,
                    0,
                    1) )
    {
        __debugbreak();
    }
    system->priorityInfo.warningHigh[system->priorityInfo.readIndex] = -1;
}

int __cdecl FX_GetFrameWarningPriority(const FxSystem *system)
{
    if ( system->priorityInfo.readIndex >= 2u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                    1448,
                    0,
                    "system->priorityInfo.readIndex not in [0, 1]\n\t%i not in [%i, %i]",
                    system->priorityInfo.readIndex,
                    0,
                    1) )
    {
        __debugbreak();
    }
    return system->priorityInfo.warningHigh[system->priorityInfo.readIndex];
}

void __cdecl FX_Priority_Begin(FxSystem *system)
{
    if ( system->priorityInfo.readIndex >= 2u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                    1455,
                    0,
                    "system->priorityInfo.readIndex not in [0, 1]\n\t%i not in [%i, %i]",
                    system->priorityInfo.readIndex,
                    0,
                    1) )
    {
        __debugbreak();
    }
    FX_InvalidateFrameWarningPriority(system);
    system->priorityInfo.readIndex = 1 - system->priorityInfo.readIndex;
}

unsigned int __cdecl FX_SpawnEffect(
                FxSystem *system,
                const FxEffectDef *remoteDef,
                int msecBegin,
                const float *origin,
                const float (*axis)[3],
                unsigned int dobjHandle,
                int boneIndex,
                int runnerSortOrder,
                unsigned __int16 owner,
                unsigned int markEntnum,
                const orientation_t *boneOffset)
{
    unsigned int v12; // edx
    volatile unsigned int *p_firstNewEffect; // [esp+14h] [ebp-7Ch]
    volatile unsigned __int32 *p_status; // [esp+1Ch] [ebp-74h]
    signed __int32 v15; // [esp+20h] [ebp-70h]
    unsigned int uniqueHandle; // [esp+54h] [ebp-3Ch]
    FxEffectContainer *ownerEffect; // [esp+68h] [ebp-28h]
    unsigned __int16 effectHandle; // [esp+74h] [ebp-1Ch]
    signed __int32 allocIndex; // [esp+78h] [ebp-18h]
    FxEffectContainer *remoteEffect; // [esp+80h] [ebp-10h]
    int oldStatusValue; // [esp+84h] [ebp-Ch]
    char isSpotLightEffect; // [esp+8Bh] [ebp-5h]
    unsigned int elemClass; // [esp+8Ch] [ebp-4h]

    PROF_SCOPED("FX_SpawnEffect");

    if ( zombietron->current.enabled && system->localClientNum )
    {
        return 0;
    }

    iassert(system);
    iassert(!system->isArchiving);
    iassert(remoteDef);
    iassert(origin);
    iassert(axis);

    if ( fx_cull_effect_spawn->current.enabled
        && (!system->extraCameraPre.isValid || FX_CullEffectForSpawn(&system->extraCameraPre, remoteDef, origin))
        && FX_CullEffectForSpawn(&system->cameraPrev, remoteDef, origin) )
    {
        return 0;
    }

    isSpotLightEffect = FX_IsSpotLightEffect(system, remoteDef);
    if ( !isSpotLightEffect || FX_CanAllocSpotLightEffect(system) )
    {
        allocIndex = _InterlockedExchangeAdd(&system->shared->firstFreeEffect, 1u);
        while ( allocIndex - system->shared->firstActiveEffect >= 1024 )
        {
            if ( _InterlockedCompareExchange(&system->shared->firstFreeEffect, allocIndex, allocIndex + 1) == allocIndex + 1 )
            {
                FX_SetWarningPriority(system, remoteDef->efPriority);
                return 0;
            }
        }
        if ( allocIndex - system->shared->firstActiveEffect >= 1014 )
            FX_SetWarningPriority(system, remoteDef->efPriority);
        effectHandle = system->shared->allEffectHandles[allocIndex & 0x3FF];
        remoteEffect = FX_EffectFromHandle(system, effectHandle);
        for ( remoteEffect->atomics.status = 0;
                    _InterlockedExchangeAdd(&remoteEffect->atomics.status, 0x20000000u) >= 0x20000000;
                    _InterlockedExchangeAdd(&remoteEffect->atomics.status, 0xE0000000) )
        {
            ;
        }
        UniqueHandleRelease(system, &remoteEffect->effect);
        UniqueHandleAssignHandle(system, remoteEffect, remoteEffect);
        FX_AddRefToEffect(system, &remoteEffect->effect);
        remoteEffect->effect.def = remoteDef;
        if ( remoteDef->msecLoopingLife )
        {
            p_status = &remoteEffect->atomics.status;
            do
                v15 = *p_status;
            while ( _InterlockedCompareExchange(p_status, v15 | 0x10000, v15) != v15 );
            FX_AddRefToEffect(system, &remoteEffect->effect);
        }
        for ( elemClass = 0; elemClass < 3; ++elemClass )
            remoteEffect->effect.firstElemHandle[elemClass] = -1;
        remoteEffect->effect.firstSortedElemHandle = -1;
        if ( (remoteDef->flags & 1) != 0 )
        {
            FX_CalculatePackedLighting(&remoteEffect->effect, origin);
        }
        else
        {
            remoteEffect->effect.lightR = 255;
            remoteEffect->effect.lightG = 255;
            remoteEffect->effect.lightB = 255;
        }
        memcpy(&remoteEffect->effect.boneOffset, boneOffset, sizeof(remoteEffect->effect.boneOffset));
        remoteEffect->effect.msecBegin = msecBegin;
        remoteEffect->effect.msecLastUpdate = remoteEffect->effect.msecBegin;
        remoteEffect->effect.distanceTraveled = 0.0f;
        FX_SetEffectRandomSeed(&remoteEffect->effect, remoteDef);
        remoteEffect->effect.firstTrailHandle = -1;
        FX_SpawnEffect_AllocTrails(system, &remoteEffect->effect, &remoteEffect->effect);
        if ( isSpotLightEffect )
            FX_SpawnEffect_AllocSpotLightEffect(system, &remoteEffect->effect, remoteEffect);
        if ( (remoteEffect->atomics.status & 0x7FE0000) != 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                        1642,
                        0,
                        "%s\n\t(FX_GetAtomics(effect)->status) = %i",
                        "((FX_GetAtomics(effect)->status & FX_STATUS_OWNED_EFFECTS_MASK) == 0)",
                        remoteEffect->atomics.status) )
        {
            __debugbreak();
        }
        if ( owner == 0xFFFF )
        {
            remoteEffect->effect.owner = effectHandle;
        }
        else
        {
            remoteEffect->effect.owner = owner;
            ownerEffect = FX_EffectFromHandle(system, owner);
            FX_AddRefToEffect(system, &ownerEffect->effect);
            if ( remoteEffect == ownerEffect
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                            1658,
                            0,
                            "%s",
                            "remoteEffect != ownerEffect") )
            {
                __debugbreak();
            }
            oldStatusValue = _InterlockedExchangeAdd(&ownerEffect->atomics.status, 0x20000u);
            if ( (oldStatusValue & 0xF801FFFF) != ((oldStatusValue + 0x20000) & 0xF801FFFF)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                            1663,
                            0,
                            "%s\n\t(oldStatusValue) = %i",
                            "((oldStatusValue & ~FX_STATUS_OWNED_EFFECTS_MASK) == ((oldStatusValue + (1 << FX_STATUS_OWNED_EFFECTS_"
                            "SHIFT)) & ~FX_STATUS_OWNED_EFFECTS_MASK))",
                            oldStatusValue) )
            {
                __debugbreak();
            }
            if ( (ownerEffect->atomics.status & 0x7FE0000) == 0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                            1665,
                            0,
                            "%s\n\t(FX_GetAtomics(ownerEffect)->status) = %i",
                            "((FX_GetAtomics(ownerEffect)->status & FX_STATUS_OWNED_EFFECTS_MASK) > 0)",
                            ownerEffect->atomics.status) )
            {
                __debugbreak();
            }
        }

#if 0 // aislop used with union context
        remoteEffect->effect.boltAndSortOrder.0 = ($88D60DA77564256B22C3F95D9DD45A24)(((boneIndex & 0x1FF) << 20)
                                                                                                                                                                | *(unsigned int *)&remoteEffect->effect.boltAndSortOrder.0
                                                                                                                                                                & 0xE00FFFFF);
        *(unsigned int *)&remoteEffect->effect.boltAndSortOrder.0 &= ~0x80000u;
        if ( dobjHandle != 2047 || boneIndex == 511 )
        {
            if ( markEntnum == 1023 )
            {
                if ( dobjHandle == 2047 )
                    v12 = *(unsigned int *)&remoteEffect->effect.boltAndSortOrder.0 & 0x9FFFFFFF;
                else
                    v12 = *(unsigned int *)&remoteEffect->effect.boltAndSortOrder.0 & 0x9FFFFFFF | 0x20000000;
                remoteEffect->effect.boltAndSortOrder.0 = ($88D60DA77564256B22C3F95D9DD45A24)v12;
                if ( boneIndex < 0
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                                1699,
                                0,
                                "%s",
                                "boneIndex >= 0") )
                {
                    __debugbreak();
                }
                remoteEffect->effect.boltAndSortOrder.0 = ($88D60DA77564256B22C3F95D9DD45A24)(((dobjHandle & 0x7FF) << 8)
                                                                                                                                                                        | *(unsigned int *)&remoteEffect->effect.boltAndSortOrder.0
                                                                                                                                                                        & 0xFFF800FF);
                remoteEffect->effect.boltAndSortOrder.0 = ($88D60DA77564256B22C3F95D9DD45A24)((FX_GetBoltTemporalBits(
                                                                                                                                                                                    system->localClientNum,
                                                                                                                                                                                    dobjHandle) << 19)
                                                                                                                                                                        | *(unsigned int *)&remoteEffect->effect.boltAndSortOrder.0
                                                                                                                                                                        & 0xFFF7FFFF);
            }
            else
            {
                if ( boneIndex != 511
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                                1683,
                                0,
                                "%s",
                                "boneIndex == FX_BONE_INDEX_NONE") )
                {
                    __debugbreak();
                }
                if ( markEntnum >= 0x7FF
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                                1684,
                                0,
                                "markEntnum doesn't index FX_DOBJ_HANDLE_NONE\n\t%i not in [0, %i)",
                                markEntnum,
                                2047) )
                {
                    __debugbreak();
                }
                remoteEffect->effect.boltAndSortOrder.0 = ($88D60DA77564256B22C3F95D9DD45A24)(((markEntnum & 0x7FF) << 8)
                                                                                                                                                                        | *(unsigned int *)&remoteEffect->effect.boltAndSortOrder.0
                                                                                                                                                                        & 0xFFF800FF);
                *(unsigned int *)&remoteEffect->effect.boltAndSortOrder.0 |= 0x60000000u;
                if ( ((*(unsigned int *)&remoteEffect->effect.boltAndSortOrder.0 >> 8) & 0x7FF) != markEntnum
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                                1687,
                                0,
                                "%s\n\t(markEntnum) = %i",
                                "(effect->boltAndSortOrder.dobjHandle == markEntnum)",
                                markEntnum) )
                {
                    __debugbreak();
                }
            }
        }
        else
        {
            remoteEffect->effect.boltAndSortOrder.0 = ($88D60DA77564256B22C3F95D9DD45A24)(((boneIndex & 0x1FFFFF) << 8)
                                                                                                                                                                    | *(unsigned int *)&remoteEffect->effect.boltAndSortOrder.0
                                                                                                                                                                    & 0xE00000FF);
            remoteEffect->effect.boltAndSortOrder.0 = ($88D60DA77564256B22C3F95D9DD45A24)(*(unsigned int *)&remoteEffect->effect.boltAndSortOrder.0
                                                                                                                                                                    & 0x9FFFFFFF
                                                                                                                                                                    | 0x40000000);
            if ( ((*(unsigned int *)&remoteEffect->effect.boltAndSortOrder.0 >> 8) & 0x1FFFFF) != boneIndex
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                            1678,
                            0,
                            "effect->boltAndSortOrder.absDynEntId == static_cast<uint>( boneIndex )\n\t%i, %i",
                            (*(unsigned int *)&remoteEffect->effect.boltAndSortOrder.0 >> 8) & 0x1FFFFF,
                            boneIndex) )
            {
                __debugbreak();
            }
        }
        remoteEffect->effect.boltAndSortOrder.0 = ($88D60DA77564256B22C3F95D9DD45A24)((unsigned __int8)runnerSortOrder | *(unsigned int *)&remoteEffect->effect.boltAndSortOrder.0 & 0xFFFFFF00);

        if ( (unsigned __int8)*(unsigned int *)&remoteEffect->effect.boltAndSortOrder.0 != runnerSortOrder
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                        1707,
                        0,
                        "effect->boltAndSortOrder.sortOrder == static_cast<uint>( runnerSortOrder )\n\t%i, %i",
                        (unsigned __int8)*(unsigned int *)&remoteEffect->effect.boltAndSortOrder.0,
                        runnerSortOrder) )
        {
            __debugbreak();
        }
#else
    FxBoltAndSortOrder &bolt = remoteEffect->effect.boltAndSortOrder;

    /*
        Base bone index setup
    */
    bolt.boneIndex = boneIndex & 0xFF;
    bolt.rewindKill = 0;

    /*
        DObj / MarkEnt / AbsDynEnt handling
    */
    if (dobjHandle != 2047 || boneIndex == 511)
    {
        if (markEntnum == 1023)
        {
            // Normal DObj case

            bolt.type = (dobjHandle == 2047) ? 0 : 1;

            iassert(boneIndex >= 0);

            bolt.dobjHandle = dobjHandle & 0x7FF;
            bolt.temporalBits =
                FX_GetBoltTemporalBits(system->localClientNum, dobjHandle);
        }
        else
        {
            // Mark entity case

            iassert(boneIndex == 511);
            iassert(markEntnum < 2047);

            bolt.dobjHandle = markEntnum & 0x7FF;
            bolt.type = 3;  // matches 0x60000000

            iassert(bolt.dobjHandle == markEntnum);
        }
    }
    else
    {
        // Absolute dynamic entity case

        bolt.absDynEntId = boneIndex & 0x1FFFFF;
        bolt._type = 1; // matches 0x40000000

        iassert(bolt.absDynEntId == static_cast<uint32_t>(boneIndex));
    }

    /*
        Sort order (lowest 8 bits)
    */
    bolt.sortOrder = static_cast<uint8_t>(runnerSortOrder);

    iassert(bolt.sortOrder == static_cast<uint32_t>(runnerSortOrder));

#endif
        remoteEffect->effect.frameAtSpawn.origin[0] = *origin;
        remoteEffect->effect.frameAtSpawn.origin[1] = origin[1];
        remoteEffect->effect.frameAtSpawn.origin[2] = origin[2];
        AxisToQuat(axis, remoteEffect->effect.frameAtSpawn.quat);
        memcpy(
            &remoteEffect->effect.framePrev,
            &remoteEffect->effect.frameAtSpawn,
            sizeof(remoteEffect->effect.framePrev));
        memcpy(
            &remoteEffect->effect.frameNow,
            &remoteEffect->effect.frameAtSpawn,
            sizeof(remoteEffect->effect.frameNow));
        p_firstNewEffect = &system->shared->firstNewEffect;
        do
        {
            while ( *p_firstNewEffect != allocIndex )
                ;
        }
        while ( _InterlockedCompareExchange(p_firstNewEffect, allocIndex + 1, allocIndex) != allocIndex );
        FX_StartNewEffect(system, &remoteEffect->effect, remoteEffect);
        _InterlockedExchangeAdd(&remoteEffect->atomics.status, 0xE0000000);
        uniqueHandle = remoteEffect->effect.uniqueHandle;
        return uniqueHandle;
    }
    else
    {
        R_WarnOncePerFrame(R_WARN_SPOT_LIGHT_LIMIT);
        return 0;
    }
    
}

void __cdecl FX_AddRefToEffect(FxSystem *__formal, FxEffect *effect)
{
    if ( !effect
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\effectscore\\fx_system.h", 519, 0, "%s", "effect") )
    {
        __debugbreak();
    }
    _InterlockedExchangeAdd((volatile unsigned __int32 *)&effect[1], 1u);
}

char __cdecl FX_CullEffectForSpawn(const FxCamera *camera, const FxEffectDef *effectDef, const float *origin)
{
    const FxElemDef *localDefs; // [esp+18h] [ebp-Ch]
    int elemDefCount; // [esp+1Ch] [ebp-8h]
    int elemDefIndex; // [esp+20h] [ebp-4h]

    if ( !effectDef )
        return 1;
    elemDefCount = effectDef->elemDefCountOneShot + effectDef->elemDefCountLooping;
    localDefs = effectDef->elemDefsEA;
    for ( elemDefIndex = 0; elemDefIndex < elemDefCount; ++elemDefIndex )
    {
        if ( !FX_CullElemForSpawn(camera, &localDefs[elemDefIndex], origin) )
            return 0;
    }
    return 1;
}

bool __cdecl FX_CullElemForSpawn(const FxCamera *camera, const FxElemDef *elemDef, const float *origin)
{
    float v4; // [esp+4h] [ebp-18h]
    float dist2; // [esp+18h] [ebp-4h]

    if ( elemDef->spawnRange.amplitude != 0.0 )
    {
        dist2 = Vec3DistanceSq(origin, camera->origin);
        if ( (float)(elemDef->spawnRange.base * elemDef->spawnRange.base) > dist2 )
            return 1;
        v4 = elemDef->spawnRange.amplitude + elemDef->spawnRange.base;
        if ( dist2 > (float)(v4 * v4) )
            return 1;
    }
    return (elemDef->flags & 4) != 0
            && !FX_IsDemoPlaying()
            && FX_CullSphere(camera, camera->frustumPlaneCount, origin, elemDef->spawnFrustumCullRadius);
}

void __cdecl FX_SetEffectRandomSeed(FxEffect *effect, const FxEffectDef *remoteDef)
{
    if ( FX_EffectAffectsGameplay(remoteDef) )
        effect->randomSeed = (479 * ((unsigned int)(214013 * effect->msecBegin + 2531011) >> 17)) >> 15;
    else
        effect->randomSeed = 479 * rand() / 0x8000;
}

char __cdecl FX_IsSpotLightEffect(FxSystem *system, const FxEffectDef *def)
{
    int elemDefIter; // [esp+4h] [ebp-4h]

    for ( elemDefIter = 0;
                elemDefIter != def->elemDefCountOneShot + def->elemDefCountLooping + def->elemDefCountEmission;
                ++elemDefIter )
    {
        if ( def->elemDefsEA[elemDefIter].elemType == 9 )
            return 1;
    }
    return 0;
}

bool __cdecl FX_CanAllocSpotLightEffect(const FxSystem *system)
{
    return system->shared->activeSpotLightEffectCount < 1;
}

char __cdecl FX_SpawnEffect_AllocSpotLightEffect(FxSystem *system, FxEffect *effect, FxEffectContainer *remoteEffect)
{
    const FxEffectDef *def; // [esp+Ch] [ebp-10h]
    int elemDefCount; // [esp+14h] [ebp-8h]
    int elemDefIter; // [esp+18h] [ebp-4h]

    def = effect->def;
    if ( !effect->def
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp", 1298, 0, "%s", "def") )
    {
        __debugbreak();
    }
    elemDefCount = def->elemDefCountOneShot + def->elemDefCountLooping + def->elemDefCountEmission;
    for ( elemDefIter = 0; elemDefIter != elemDefCount; ++elemDefIter )
    {
        if ( effect->def->elemDefsEA[elemDefIter].elemType == 9 )
        {
            if ( system->shared->activeSpotLightEffectCount >= 1
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                            1308,
                            0,
                            "%s",
                            "system->shared->activeSpotLightEffectCount < FX_SPOT_LIGHT_LIMIT") )
            {
                __debugbreak();
            }
            _InterlockedExchangeAdd(&system->shared->activeSpotLightEffectCount, 1u);
            system->shared->activeSpotLightEffectHandle = FX_EffectToHandle(system, remoteEffect);
        }
    }
    return 1;
}

unsigned int __cdecl FX_SpawnOrientedEffect(
                int localClientNum,
                const FxEffectDef *def,
                int msecBegin,
                const float *origin,
                const float (*axis)[3],
                unsigned int markEntnum)
{
    FxSystemContainer *system; // [esp+0h] [ebp-4h]

    if ( !fx_enable->current.enabled )
        return 0;
    system = FX_GetSystem(localClientNum);
    if ( !system
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp", 1757, 0, "%s", "system") )
    {
        __debugbreak();
    }
    return FX_SpawnEffect(
                     &system->system,
                     def,
                     msecBegin,
                     origin,
                     axis,
                     0x7FFu,
                     511,
                     255,
                     0xFFFFu,
                     markEntnum,
                     &orIdentity);
}

void __cdecl FX_AssertAllocatedEffect(int localClientNum, unsigned int hEffect, const char *error_msg)
{
    FxEffectContainer *effect; // [esp+0h] [ebp-8h]
    FxSystemContainer *system; // [esp+4h] [ebp-4h]

    system = FX_GetSystem(localClientNum);
    if ( !system
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp", 1772, 0, "%s", "system") )
    {
        __debugbreak();
    }
    effect = UniqueHandleToEffect(localClientNum, hEffect);
    if ( !effect
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp", 1775, 0, "%s", "effect") )
    {
        __debugbreak();
    }
    FX_EffectToHandle(&system->system, effect);
    if ( (effect->atomics.status & 0x3FFF) == 0 )
        Com_Error(ERR_DROP, "FX_AssertAllocatedEffect: %s %s.", effect->effect.def->name, error_msg);
    if ( (effect->atomics.status & 0x3FFF) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                    1782,
                    0,
                    "%s",
                    "FX_GetEffectRef(effect) != 0") )
    {
        __debugbreak();
    }
}

unsigned int __cdecl FX_PlayOrientedEffectWithMarkEntity(
                int localClientNum,
                const FxEffectDef *def,
                int startMsec,
                const float *origin,
                const float (*axis)[3],
                unsigned int markEntnum)
{
    volatile unsigned __int32 *p_status; // [esp+0h] [ebp-18h]
    signed __int32 v8; // [esp+4h] [ebp-14h]
    unsigned int effect; // [esp+Ch] [ebp-Ch]
    FxEffectContainer *effectPtr; // [esp+10h] [ebp-8h]
    FxSystemContainer *system; // [esp+14h] [ebp-4h]

    system = FX_GetSystem(localClientNum);
    effect = FX_SpawnOrientedEffect(localClientNum, def, startMsec, origin, axis, markEntnum);
    effectPtr = UniqueHandleToEffect(&system->system, effect);
    if ( effectPtr )
    {
        FX_DelRefToEffect(&system->system, effectPtr, 0);
        p_status = &effectPtr->atomics.status;
        do
            v8 = *p_status;
        while ( _InterlockedCompareExchange(p_status, v8 | 0x8000, v8) != v8 );
    }
    return effect;
}

unsigned int __cdecl FX_PlayOrientedEffect(
                int localClientNum,
                const FxEffectDef *def,
                int startMsec,
                const float *origin,
                const float (*axis)[3])
{
    FxSystemContainer *System; // eax

    System = FX_GetSystem(localClientNum);
    return FX_PlayOrientedEffect(&System->system, def, startMsec, origin, axis);
}

unsigned int __cdecl FX_PlayOrientedEffect(
                FxSystem *system,
                const FxEffectDef *def,
                int startMsec,
                const float *origin,
                const float (*axis)[3])
{
    volatile unsigned __int32 *p_status; // [esp+0h] [ebp-14h]
    signed __int32 v7; // [esp+4h] [ebp-10h]
    unsigned int effect; // [esp+Ch] [ebp-8h]
    FxEffectContainer *effectPtr; // [esp+10h] [ebp-4h]

    effect = FX_SpawnEffect(system, def, startMsec, origin, axis, 0x7FFu, 511, 255, 0xFFFFu, 0x3FFu, &orIdentity);
    effectPtr = UniqueHandleToEffect(system, effect);
    if ( effectPtr )
    {
        FX_DelRefToEffect(system, effectPtr, 0);
        p_status = &effectPtr->atomics.status;
        do
            v7 = *p_status;
        while ( _InterlockedCompareExchange(p_status, v7 | 0x8000, v7) != v7 );
    }
    return effect;
}

unsigned int __cdecl FX_SpawnBoltedEffect(
                int localClientNum,
                const FxEffectDef *def,
                int msecBegin,
                unsigned int dobjHandle,
                unsigned int boneIndex,
                const float *origin,
                const float (*axis)[3])
{
    FxSystemContainer *System; // eax
    orientation_t invBoneOrient; // [esp+10h] [ebp-C4h] BYREF
    orientation_t boneOffset; // [esp+40h] [ebp-94h] BYREF
    orientation_t effectOrient; // [esp+70h] [ebp-64h] BYREF
    orientation_t boneOrient; // [esp+A4h] [ebp-30h] BYREF

    if ( !fx_enable->current.enabled )
        return 0;
    if ( !FX_GetBoneOrientation(localClientNum, dobjHandle, boneIndex, &boneOrient) )
        return 0;
    if ( FX_NeedsBoltUpdate(def) )
    {
        if ( dobjHandle >= 0x7FF
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                        1882,
                        0,
                        "dobjHandle doesn't index FX_DOBJ_HANDLE_NONE\n\t%i not in [0, %i)",
                        dobjHandle,
                        2047) )
        {
            __debugbreak();
        }
        if ( boneIndex >= 0x200
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                        1883,
                        0,
                        "boneIndex doesn't index FX_BONE_INDEX_NONE+1\n\t%i not in [0, %i)",
                        boneIndex,
                        512) )
        {
            __debugbreak();
        }
    }
    else
    {
        boneIndex = 511;
    }
    effectOrient.origin[0] = *origin;
    effectOrient.origin[1] = origin[1];
    effectOrient.origin[2] = origin[2];
    *(_QWORD *)&effectOrient.axis[0][0] = *(_QWORD *)&(*axis)[0];
    effectOrient.axis[0][2] = (*axis)[2];
    effectOrient.axis[1][0] = (*axis)[3];
    effectOrient.axis[1][1] = (*axis)[4];
    effectOrient.axis[1][2] = (*axis)[5];
    effectOrient.axis[2][0] = (*axis)[6];
    effectOrient.axis[2][1] = (*axis)[7];
    effectOrient.axis[2][2] = (*axis)[8];
    OrientationInvert(&boneOrient, &invBoneOrient);
    OrientationConcatenate(&effectOrient, &invBoneOrient, &boneOffset);
    System = FX_GetSystem(localClientNum);
    return FX_SpawnEffect(
                     &System->system,
                     def,
                     msecBegin,
                     effectOrient.origin,
                     effectOrient.axis,
                     dobjHandle,
                     boneIndex,
                     255,
                     0xFFFFu,
                     0x3FFu,
                     &boneOffset);
}

char __cdecl FX_NeedsBoltUpdate(const FxEffectDef *def)
{
    int elemDefIndex; // [esp+4h] [ebp-4h]

    if ( !def && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp", 1843, 0, "%s", "def") )
        __debugbreak();
    for ( elemDefIndex = 0; elemDefIndex < def->elemDefCountOneShot + def->elemDefCountLooping; ++elemDefIndex )
    {
        if ( def->elemDefsEA[elemDefIndex].elemType == 5 )
            return 1;
        if ( (def->elemDefsEA[elemDefIndex].flags & 0xC0) == 0x80 )
            return 1;
    }
    return 0;
}

unsigned int __cdecl FX_SpawnBoltedEffect(
                int localClientNum,
                const FxEffectDef *def,
                int msecBegin,
                unsigned int dobjHandle,
                int boneIndex)
{
    orientation_t orient; // [esp+0h] [ebp-30h] BYREF

    if ( !fx_enable->current.enabled )
        return 0;
    if ( FX_GetBoneOrientation(localClientNum, dobjHandle, boneIndex, &orient) )
        return FX_SpawnBoltedEffect(localClientNum, def, msecBegin, dobjHandle, boneIndex, orient.origin, orient.axis);
    return 0;
}

unsigned int __cdecl FX_PlayBoltedEffect_DynEnt(
                int localClientNum,
                const FxEffectDef *def,
                int startMsec,
                int absDynEntId)
{
    volatile unsigned __int32 *p_status; // [esp+0h] [ebp-58h]
    signed __int32 v6; // [esp+4h] [ebp-54h]
    const DynEntityPose *dynEntPose; // [esp+10h] [ebp-48h]
    int boneIndex; // [esp+14h] [ebp-44h]
    unsigned int effect; // [esp+1Ch] [ebp-3Ch]
    FxEffectContainer *effectPtr; // [esp+20h] [ebp-38h]
    orientation_t orient; // [esp+24h] [ebp-34h] BYREF
    FxSystem *system; // [esp+54h] [ebp-4h]

    if ( !fx_enable->current.enabled )
        return 0;
    boneIndex = absDynEntId;
    if ( FX_NeedsBoltUpdate(def) )
    {
        if ( absDynEntId > 0x200000 )
            return 0;
    }
    else
    {
        boneIndex = 511;
    }
    dynEntPose = DynEnt_GetClientPose(absDynEntId);
    orient.origin[0] = dynEntPose->pose.origin[0];
    orient.origin[1] = dynEntPose->pose.origin[1];
    orient.origin[2] = dynEntPose->pose.origin[2];
    UnitQuatToAxis(dynEntPose->pose.quat, orient.axis);
    system = (FxSystem *)FX_GetSystem(localClientNum);
    effect = FX_SpawnEffect(
                         system,
                         def,
                         startMsec,
                         orient.origin,
                         orient.axis,
                         0x7FFu,
                         boneIndex,
                         255,
                         0xFFFFu,
                         0x3FFu,
                         &orIdentity);
    effectPtr = UniqueHandleToEffect(system, effect);
    if ( effectPtr )
    {
        FX_DelRefToEffect(system, effectPtr, 0);
        p_status = &effectPtr->atomics.status;
        do
            v6 = *p_status;
        while ( _InterlockedCompareExchange(p_status, v6 | 0x8000, v6) != v6 );
    }
    return effect;
}

unsigned int __cdecl FX_PlayBoltedEffect(
                int localClientNum,
                const FxEffectDef *def,
                int startMsec,
                unsigned int dobjHandle,
                int boneIndex)
{
    volatile unsigned __int32 *p_status; // [esp+0h] [ebp-18h]
    signed __int32 v7; // [esp+4h] [ebp-14h]
    unsigned int effect; // [esp+Ch] [ebp-Ch]
    FxEffectContainer *effectPtr; // [esp+10h] [ebp-8h]
    FxSystemContainer *system; // [esp+14h] [ebp-4h]

    system = FX_GetSystem(localClientNum);
    effect = FX_SpawnBoltedEffect(localClientNum, def, startMsec, dobjHandle, boneIndex);
    effectPtr = UniqueHandleToEffect(&system->system, effect);
    if ( effectPtr )
    {
        FX_DelRefToEffect(&system->system, effectPtr, 0);
        p_status = &effectPtr->atomics.status;
        do
            v7 = *p_status;
        while ( _InterlockedCompareExchange(p_status, v7 | 0x8000, v7) != v7 );
    }
    return effect;
}

unsigned int __cdecl FX_PlayBoltedEffect(
                int localClientNum,
                const FxEffectDef *def,
                int startMsec,
                unsigned int dobjHandle,
                unsigned int boneIndex,
                const float *origin,
                const float (*axis)[3])
{
    volatile unsigned __int32 *p_status; // [esp+0h] [ebp-18h]
    signed __int32 v9; // [esp+4h] [ebp-14h]
    unsigned int effect; // [esp+Ch] [ebp-Ch]
    FxEffectContainer *effectPtr; // [esp+10h] [ebp-8h]
    FxSystemContainer *system; // [esp+14h] [ebp-4h]

    system = FX_GetSystem(localClientNum);
    effect = FX_SpawnBoltedEffect(localClientNum, def, startMsec, dobjHandle, boneIndex, origin, axis);
    effectPtr = UniqueHandleToEffect(&system->system, effect);
    if ( effectPtr )
    {
        FX_DelRefToEffect(&system->system, effectPtr, 0);
        p_status = &effectPtr->atomics.status;
        do
            v9 = *p_status;
        while ( _InterlockedCompareExchange(p_status, v9 | 0x8000, v9) != v9 );
    }
    return effect;
}

void __cdecl FX_RetriggerEffect(int localClientNum, FxEffectContainer *effect, int msecBegin)
{
    volatile unsigned __int32 *p_status; // [esp+18h] [ebp-54h]
    signed __int32 v4; // [esp+1Ch] [ebp-50h]
    unsigned __int16 lastOldTrailElemHandle[8]; // [esp+30h] [ebp-3Ch] BYREF
    int trailCount; // [esp+40h] [ebp-2Ch] BYREF
    unsigned __int16 lastElemHandle[5]; // [esp+44h] [ebp-28h] BYREF
    bool catchUpNewElems; // [esp+4Fh] [ebp-1Dh]
    unsigned __int16 firstOldElemHandle[4]; // [esp+50h] [ebp-1Ch] BYREF
    FxSystem *system; // [esp+60h] [ebp-Ch]
    bool hasPendingLoopElems; // [esp+67h] [ebp-5h]
    unsigned int elemClass; // [esp+68h] [ebp-4h]

    if ( (effect->atomics.status & 0x3FFF) != 0 )
    {
        system = (FxSystem *)FX_GetSystem(localClientNum);
        while ( _InterlockedExchangeAdd(&effect->atomics.status, 0x20000000u) >= 0x20000000 )
            _InterlockedExchangeAdd(&effect->atomics.status, 0xE0000000);
        FX_AddRefToEffect(system, &effect->effect);
        if ( (effect->atomics.status & 0x10000) != 0 )
        {
            FX_SpawnAllFutureLooping(
                system,
                &effect->effect,
                effect,
                0,
                effect->effect.def->elemDefCountLooping,
                &effect->effect.framePrev,
                &effect->effect.frameNow,
                effect->effect.msecBegin,
                effect->effect.msecLastUpdate);
            FX_StopEffect(system, effect);
        }
        for ( elemClass = 0; elemClass < 3; ++elemClass )
            firstOldElemHandle[elemClass] = effect->effect.firstElemHandle[elemClass];
        FX_GetTrailHandleList_Last(system, &effect->effect, lastOldTrailElemHandle, &trailCount);
        catchUpNewElems = msecBegin < effect->effect.msecLastUpdate;
        if ( msecBegin > effect->effect.msecLastUpdate )
        {
            for ( elemClass = 0; elemClass < 3; ++elemClass )
                lastElemHandle[elemClass] = -1;
            FX_UpdateEffectPartial(
                system,
                &effect->effect,
                effect,
                effect->effect.msecLastUpdate,
                msecBegin,
                0.0,
                0.0,
                firstOldElemHandle,
                lastElemHandle,
                0,
                lastOldTrailElemHandle);
        }
        effect->effect.msecBegin = msecBegin;
        effect->effect.distanceTraveled = 0.0f;
        FX_BeginLooping(
            system,
            &effect->effect,
            effect,
            0,
            effect->effect.def->elemDefCountLooping,
            &effect->effect.frameNow,
            &effect->effect.frameNow,
            msecBegin,
            msecBegin);
        FX_TriggerOneShot(
            system,
            &effect->effect,
            effect,
            effect->effect.def->elemDefCountLooping,
            effect->effect.def->elemDefCountOneShot,
            &effect->effect.frameNow,
            msecBegin);
        hasPendingLoopElems = effect->effect.def->msecLoopingLife != 0;
        if ( hasPendingLoopElems )
        {
            p_status = &effect->atomics.status;
            do
                v4 = *p_status;
            while ( _InterlockedCompareExchange(p_status, v4 | 0x10000, v4) != v4 );
        }
        if ( catchUpNewElems )
            FX_UpdateEffectPartial(
                system,
                &effect->effect,
                effect,
                effect->effect.msecBegin,
                effect->effect.msecLastUpdate,
                0.0,
                0.0,
                effect->effect.firstElemHandle,
                firstOldElemHandle,
                lastOldTrailElemHandle,
                0);
        FX_SortNewElemsInEffect(system, &effect->effect);
        if ( !hasPendingLoopElems )
            FX_DelRefToEffect(system, effect, 0);
        _InterlockedExchangeAdd(&effect->atomics.status, 0xE0000000);
    }
}

void __cdecl FX_GetTrailHandleList_Last(
                FxSystem *system,
                FxEffect *effect,
                unsigned __int16 *outHandleList,
                int *outTrailCount)
{
    unsigned __int16 trailHandle; // [esp+0h] [ebp-Ch]
    FxPool<FxTrail,FxTrail> *trail; // [esp+4h] [ebp-8h]
    unsigned int trailIndex; // [esp+8h] [ebp-4h]

    trailIndex = 0;
    for ( trailHandle = effect->firstTrailHandle; trailHandle != 0xFFFF; trailHandle = trail->item.nextTrailHandle )
    {
        trail = FX_TrailFromHandle(system, trailHandle);
        if ( trailIndex >= 2
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                        2018,
                        0,
                        "%s\n\t(trailIndex) = %i",
                        "(trailIndex < (sizeof( outHandleList ) / (sizeof( outHandleList[0] ) * (sizeof( outHandleList ) != 4 || size"
                        "of( outHandleList[0] ) <= 4))))",
                        trailIndex) )
        {
            __debugbreak();
        }
        outHandleList[trailIndex++] = trail->item.lastElemHandle;
    }
    *outTrailCount = trailIndex;
}

void __cdecl FX_RetriggerEffect(int localClientNum, unsigned int hEffect, int msecBegin)
{
    FxEffectContainer *effect; // [esp+0h] [ebp-4h]

    effect = UniqueHandleToEffect(localClientNum, hEffect);
    if ( effect )
        FX_RetriggerEffect(localClientNum, effect, msecBegin);
}

void __cdecl FX_ThroughWithEffect(int localClientNum, unsigned int hEffect, bool forcekill)
{
    FxEffectContainer *effect; // [esp+0h] [ebp-8h]
    FxSystemContainer *system; // [esp+4h] [ebp-4h]

    system = FX_GetSystem(localClientNum);
    if ( !system
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp", 2104, 0, "%s", "system") )
    {
        __debugbreak();
    }
    if ( system->system.isInitialized )
    {
        effect = UniqueHandleToEffect(&system->system, hEffect);
        if ( effect )
            FX_ThroughWithEffect(&system->system, effect, forcekill);
    }
}

void __cdecl FX_ThroughWithEffect(FxSystem *system, FxEffectContainer *effect, bool forcekill)
{
    if ( (effect->atomics.status & 0x3FFF) != 0 )
    {
        if ( forcekill )
        {
            while ( _InterlockedExchangeAdd(&effect->atomics.status, 0x20000000u) >= 0x20000000 )
                _InterlockedExchangeAdd(&effect->atomics.status, 0xE0000000);
            FX_KillEffect(system, effect);
            _InterlockedExchangeAdd(&effect->atomics.status, 0xE0000000);
            if ( (effect->atomics.status & 0x3FFF) != 0 )
                FX_DelRefToEffect(system, effect, 0);
        }
        else
        {
            while ( _InterlockedExchangeAdd(&effect->atomics.status, 0x20000000u) >= 0x20000000 )
                _InterlockedExchangeAdd(&effect->atomics.status, 0xE0000000);
            FX_StopEffect(system, effect);
            _InterlockedExchangeAdd(&effect->atomics.status, 0xE0000000);
            if ( (effect->atomics.status & 0x8000) == 0 )
                FX_DelRefToEffect(system, effect, 0);
        }
    }
}

void __cdecl FX_StopEffect(FxSystem *system, FxEffectContainer *effect)
{
    unsigned __int16 effectHandle; // [esp+14h] [ebp-10h]
    unsigned __int16 stoppedEffectHandle; // [esp+18h] [ebp-Ch]
    FxEffectContainer *otherEffect; // [esp+1Ch] [ebp-8h]
    volatile int activeIndex; // [esp+20h] [ebp-4h]

    if ( !effect
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp", 2166, 0, "%s", "effect") )
    {
        __debugbreak();
    }
    if ( (effect->atomics.status & 0x3FFF) != 0 )
    {
        FX_AddRefToEffect(system, &effect->effect);
        FX_StopEffectNonRecursive(system, effect);
        if ( (effect->atomics.status & 0x7FE0000) != 0 )
        {
            stoppedEffectHandle = FX_EffectToHandle(system, effect);
            FX_BeginIteratingOverEffects_Cooperative(system);
            for ( activeIndex = system->shared->firstActiveEffect; activeIndex != system->shared->firstNewEffect; ++activeIndex )
            {
                effectHandle = system->shared->allEffectHandles[activeIndex & 0x3FF];
                if ( effectHandle != stoppedEffectHandle )
                {
                    otherEffect = FX_EffectFromHandle(system, effectHandle);
                    if ( *(unsigned int *)&otherEffect->effect.owner == stoppedEffectHandle )
                        FX_StopEffect(system, (FxEffectContainer*)&otherEffect->effect);
                }
            }
            if ( !_InterlockedDecrement(&system->shared->iteratorCount) )
                FX_RunGarbageCollectionAndPrioritySort(system);
        }
        FX_DelRefToEffect(system, effect, 0);
    }
    else if ( (effect->atomics.status & 0x10000) != 0
                 && !Assert_MyHandler(
                             "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                             2171,
                             0,
                             "%s",
                             "!(FX_GetAtomics(effect)->status & FX_STATUS_HAS_PENDING_LOOP_ELEMS)") )
    {
        __debugbreak();
    }
}

void __cdecl FX_StopEffectNonRecursive(FxSystem *system, FxEffectContainer *effect)
{
    signed __int32 status; // [esp+0h] [ebp-4h]

    if ( !effect
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp", 2147, 0, "%s", "effect") )
    {
        __debugbreak();
    }
    while ( 1 )
    {
        status = effect->atomics.status;
        if ( (status & 0x10000) == 0 )
            break;
        if ( _InterlockedCompareExchange(&effect->atomics.status, status & 0xFFFEFFFF, status) == status )
        {
            FX_DelRefToEffect(system, effect, 0);
            return;
        }
    }
}

void __cdecl FX_KillEffect(FxSystem *system, FxEffectContainer *effect)
{
    unsigned __int16 ownedEffectHandle; // [esp+14h] [ebp-14h]
    FxEffectContainer *ownedEffect; // [esp+18h] [ebp-10h]
    unsigned __int16 killedEffectHandle; // [esp+1Ch] [ebp-Ch]
    int count; // [esp+20h] [ebp-8h]
    volatile int activeIndex; // [esp+24h] [ebp-4h]

    if ( !effect
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp", 2275, 0, "%s", "effect") )
    {
        __debugbreak();
    }
    if ( (effect->atomics.status & 0x3FFF) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                    2277,
                    0,
                    "%s\n\t(FX_GetAtomics(effect)->status) = %i",
                    "(FX_GetEffectRef(effect) > 0)",
                    effect->atomics.status) )
    {
        __debugbreak();
    }
    if ( (effect->atomics.status & 0x60000000) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                    2281,
                    0,
                    "%s\n\t(FX_GetAtomics(effect)->status) = %i",
                    "((FX_GetAtomics(effect)->status & FX_STATUS_IS_LOCKED_MASK) != 0)",
                    effect->atomics.status) )
    {
        __debugbreak();
    }
    FX_AddRefToEffect(system, &effect->effect);
    FX_RemoveAllEffectElems(system, effect);
    if ( (effect->atomics.status & 0x7FE0000) != 0 )
    {
        killedEffectHandle = FX_EffectToHandle(system, effect);
        FX_BeginIteratingOverEffects_Cooperative(system);
        count = 0;
        for ( activeIndex = system->shared->firstActiveEffect; (effect->atomics.status & 0x7FE0000) != 0; ++activeIndex )
        {
            if ( activeIndex == system->shared->firstNewEffect
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                            2306,
                            1,
                            "activeIndex != system->shared->firstNewEffect\n\t%i, %i",
                            activeIndex,
                            system->shared->firstNewEffect) )
            {
                __debugbreak();
            }
            if ( count >= 1024 )
                break;
            ownedEffectHandle = system->shared->allEffectHandles[activeIndex & 0x3FF];
            if ( ownedEffectHandle != killedEffectHandle )
            {
                ownedEffect = FX_EffectFromHandle(system, ownedEffectHandle);
                if ( ownedEffect->effect.owner == killedEffectHandle )
                {
                    while ( _InterlockedExchangeAdd(&ownedEffect->atomics.status, 0x20000000u) >= 0x20000000 )
                        _InterlockedExchangeAdd(&ownedEffect->atomics.status, 0xE0000000);
                    if ( (ownedEffect->atomics.status & 0x7FE0000) != 0
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                                    2328,
                                    0,
                                    "%s\n\t(FX_GetAtomics(ownedEffect)->status) = %i",
                                    "((FX_GetAtomics(ownedEffect)->status & FX_STATUS_OWNED_EFFECTS_MASK) == 0)",
                                    ownedEffect->atomics.status) )
                    {
                        __debugbreak();
                    }
                    if ( (ownedEffect->atomics.status & 0x3FFF) != 0 )
                        FX_RemoveAllEffectElems(system, ownedEffect);
                    if ( (ownedEffect->atomics.status & 0x3FFF) != 0
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                                    2335,
                                    0,
                                    "%s\n\t(FX_GetAtomics(ownedEffect)->status) = %i",
                                    "(FX_GetEffectRef(ownedEffect) == 0)",
                                    ownedEffect->atomics.status) )
                    {
                        __debugbreak();
                    }
                    _InterlockedExchangeAdd(&ownedEffect->atomics.status, 0xE0000000);
                }
            }
            ++count;
        }
        if ( !_InterlockedDecrement(&system->shared->iteratorCount) )
            FX_RunGarbageCollectionAndPrioritySort(system);
    }
    FX_DelRefToEffect(system, effect, 0);
}

void __cdecl FX_RemoveAllEffectElems(FxSystem *system, FxEffectContainer *effect)
{
    FxPool<FxTrail,FxTrail> *trail; // [esp+0h] [ebp-14h]
    unsigned __int16 elemHandle; // [esp+4h] [ebp-10h]
    unsigned __int16 trailHandle; // [esp+8h] [ebp-Ch]
    unsigned int elemClass; // [esp+10h] [ebp-4h]

    if ( !effect
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp", 2221, 0, "%s", "effect") )
    {
        __debugbreak();
    }
    if ( (effect->atomics.status & 0x3FFF) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                    2222,
                    0,
                    "%s",
                    "FX_GetEffectRef(effect) > 0") )
    {
        __debugbreak();
    }
    FX_AddRefToEffect(system, &effect->effect);
    FX_StopEffect(system, effect);
    for ( elemClass = 0; elemClass < 3; ++elemClass )
    {
        while ( effect->effect.firstElemHandle[elemClass] != 0xFFFF )
        {
            elemHandle = effect->effect.firstElemHandle[elemClass];
            FX_FreeElem(system, elemHandle, &effect->effect, effect, elemClass);
            if ( effect->effect.firstElemHandle[elemClass] == elemHandle
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                            2236,
                            0,
                            "%s",
                            "localEffect->firstElemHandle[elemClass] != elemHandle") )
            {
                __debugbreak();
            }
        }
    }
    for ( trailHandle = effect->effect.firstTrailHandle; trailHandle != 0xFFFF; trailHandle = trail->item.nextTrailHandle )
    {
        for ( trail = FX_TrailFromHandle(system, trailHandle);
                    trail->item.firstElemHandle != 0xFFFF;
                    FX_FreeTrailElem(system, trail->item.firstElemHandle, &effect->effect, effect, (FxTrail *)trail) )
        {
            ;
        }
    }
    if ( system->shared->activeSpotLightElemCount > 0
        && effect == FX_EffectFromHandle(system, system->shared->activeSpotLightEffectHandle) )
    {
        FX_FreeSpotLightElem(system, system->shared->activeSpotLightElemHandle, effect);
    }
    FX_DelRefToEffect(system, effect, 0);
}

void __cdecl FX_KillEffectDef(int localClientNum, const FxEffectDef *def)
{
    FxEffectContainer *effect; // [esp+14h] [ebp-Ch]
    FxSystemContainer *system; // [esp+18h] [ebp-8h]
    int activeIndex; // [esp+1Ch] [ebp-4h]

    system = FX_GetSystem(localClientNum);
    FX_BeginIteratingOverEffects_Cooperative(&system->system);
    for ( activeIndex = system->system.shared->firstActiveEffect;
                activeIndex != system->system.shared->firstFreeEffect;
                ++activeIndex )
    {
        effect = FX_EffectFromHandle(&system->system, system->system.shared->allEffectHandles[activeIndex & 0x3FF]);
        if ( effect->effect.def == def && (effect->atomics.status & 0x3FFF) != 0 )
        {
            while ( _InterlockedExchangeAdd(&effect->atomics.status, 0x20000000u) >= 0x20000000 )
                _InterlockedExchangeAdd(&effect->atomics.status, 0xE0000000);
            FX_KillEffect(&system->system, effect);
            _InterlockedExchangeAdd(&effect->atomics.status, 0xE0000000);
        }
    }
    if ( !_InterlockedDecrement(&system->system.shared->iteratorCount) )
        FX_RunGarbageCollectionAndPrioritySort(&system->system);
}

void __cdecl FX_KillAllEffects(int localClientNum)
{
    FxEffectContainer *effect; // [esp+18h] [ebp-Ch]
    FxSystemContainer *system; // [esp+1Ch] [ebp-8h]
    int activeIndex; // [esp+20h] [ebp-4h]

    system = FX_GetSystem(localClientNum);
    if ( !system
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp", 2420, 0, "%s", "system") )
    {
        __debugbreak();
    }
    if ( system->system.isInitialized )
    {
        FX_BeginIteratingOverEffects_Cooperative(&system->system);
        for ( activeIndex = system->system.shared->firstActiveEffect;
                    activeIndex != system->system.shared->firstNewEffect;
                    ++activeIndex )
        {
            effect = FX_EffectFromHandle(&system->system, system->system.shared->allEffectHandles[activeIndex & 0x3FF]);
            if ( (effect->atomics.status & 0x3FFF) != 0 )
            {
                while ( _InterlockedExchangeAdd(&effect->atomics.status, 0x20000000u) >= 0x20000000 )
                    _InterlockedExchangeAdd(&effect->atomics.status, 0xE0000000);
                FX_KillEffect(&system->system, effect);
                _InterlockedExchangeAdd(&effect->atomics.status, 0xE0000000);
            }
        }
        if ( !_InterlockedDecrement(&system->system.shared->iteratorCount) )
            FX_RunGarbageCollectionAndPrioritySort(&system->system);
    }
}

int __cdecl FX_GetEffectPlayedTime(int localClientNum, unsigned int hEffect)
{
    FxEffectContainer *effect; // [esp+0h] [ebp-4h]

    effect = UniqueHandleToEffect(localClientNum, hEffect);
    if ( !effect
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp", 2460, 0, "%s", "effect") )
    {
        __debugbreak();
    }
    return effect->effect.msecLastUpdate - effect->effect.msecBegin;
}

bool __cdecl FX_IsEffectDone(int localClientNum, unsigned int hEffect)
{
    return UniqueHandleToEffect(localClientNum, hEffect) == 0;
}

int __cdecl FX_MakePhysId(const FxElemDef *elemDef, const float *origin, const float *vel, int time, int sequence)
{
    return sequence
             + (((int)vel[2] + (int)vel[1] + (int)*vel + (int)origin[2] + (int)origin[1] + (int)*origin)
                ^ time
                ^ (unsigned int)elemDef);
}

void __cdecl FX_SpawnTrailElem_NoCull(
                FxSystem *system,
                FxEffect *effect,
                FxEffect *remoteEffect,
                FxTrail *trail,
                const FxSpatialFrame *effectFrameWhenPlayed,
                int msecWhenPlayed,
                float distanceWhenPlayed)
{
    bool v7; // [esp+1Bh] [ebp-45h]
    FxPool<FxTrailElem,FxTrailElem> *lastTrailElemInEffect; // [esp+24h] [ebp-3Ch]
    int msecBegin; // [esp+28h] [ebp-38h]
    const FxElemDef *elemDef; // [esp+34h] [ebp-2Ch]
    unsigned int randomSeed; // [esp+38h] [ebp-28h]
    FxPool<FxTrailElem,FxTrailElem> *remoteTrailElem; // [esp+40h] [ebp-20h]
    float basis[2][3]; // [esp+44h] [ebp-1Ch] BYREF
    unsigned __int16 trailElemHandle; // [esp+5Ch] [ebp-4h]

    if ( !system
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp", 2657, 0, "%s", "system") )
    {
        __debugbreak();
    }
    if ( !effect
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp", 2658, 0, "%s", "effect") )
    {
        __debugbreak();
    }
    if ( !effect->def
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp", 2660, 0, "%s", "effectDef") )
    {
        __debugbreak();
    }
    if ( !trail
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp", 2661, 0, "%s", "trail") )
    {
        __debugbreak();
    }
    elemDef = &effect->def->elemDefsEA[trail->defIndex];
    if ( elemDef->elemType != 5
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                    2664,
                    0,
                    "%s\n\t(elemDef->elemType) = %i",
                    "(elemDef->elemType == FX_ELEM_TYPE_TRAIL)",
                    elemDef->elemType) )
    {
        __debugbreak();
    }
    msecBegin = elemDef->spawnDelayMsec.base + msecWhenPlayed;
    if ( elemDef->spawnDelayMsec.amplitude )
        msecBegin += ((elemDef->spawnDelayMsec.amplitude + 1) * LOWORD(fx_randomTable[(296 * trail->sequence + msecBegin + (unsigned int)effect->randomSeed) % 0x1DF + 18])) >> 16;
    randomSeed = (296 * trail->sequence + msecBegin + (unsigned int)effect->randomSeed) % 0x1DF;
    if ( elemDef->effectOnImpact.handle )
    {
        v7 = 1;
    }
    else if ( elemDef->effectOnDeath.handle )
    {
        v7 = 1;
    }
    else
    {
        v7 = elemDef->effectEmitted.handle != 0;
    }
    if ( v7
        || msecBegin
         + (((elemDef->lifeSpanMsec.amplitude + 1) * LOWORD(fx_randomTable[randomSeed + 17])) >> 16)
         + elemDef->lifeSpanMsec.base > system->msecNow )
    {
        remoteTrailElem = FX_AllocTrailElem(remoteEffect, system);
        if ( remoteTrailElem )
        {
            FX_AddRefToEffect(system, remoteEffect);
            FX_GetOriginForTrailElem(
                effect,
                elemDef,
                effectFrameWhenPlayed,
                randomSeed,
                remoteTrailElem->item.origin,
                basis[0],
                basis[1]);
            trailElemHandle = FX_TrailElemToHandle(system, (FxTrailElem *)remoteTrailElem);
            if ( trail->lastElemHandle == 0xFFFF )
            {
                if ( trail->firstElemHandle != 0xFFFF
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                                2695,
                                0,
                                "%s",
                                "trail->firstElemHandle == FX_HANDLE_NONE") )
                {
                    __debugbreak();
                }
                trail->firstElemHandle = trailElemHandle;
            }
            else
            {
                lastTrailElemInEffect = FX_TrailElemFromHandle(system, trail->lastElemHandle);
                lastTrailElemInEffect->item.nextTrailElemHandle = trailElemHandle;
            }
            remoteTrailElem->item.nextTrailElemHandle = -1;
            trail->lastElemHandle = trailElemHandle;
            remoteTrailElem->item.msecBegin = msecBegin;
            remoteTrailElem->item.spawnDist = distanceWhenPlayed;
            remoteTrailElem->item.baseVelZ = 0;
            remoteTrailElem->item.sequence = trail->sequence++;
            FX_TrailElem_CompressBasis(basis, remoteTrailElem->item.basis);
        }
    }
}

void __cdecl FX_TrailElem_CompressBasis(const float (*inBasis)[3], char (*outBasis)[3])
{
    int v2; // [esp+0h] [ebp-10h]
    int v3; // [esp+4h] [ebp-Ch]
    int basisVecIter; // [esp+8h] [ebp-8h]
    int dimIter; // [esp+Ch] [ebp-4h]

    for ( basisVecIter = 0; basisVecIter != 2; ++basisVecIter )
    {
        for ( dimIter = 0; dimIter != 3; ++dimIter )
        {
            v3 = (int)(float)(127.0 * (float)(*inBasis)[3 * basisVecIter + dimIter]);
            if ( v3 >= -128 )
            {
                if ( v3 <= 127 )
                    v2 = (int)(float)(127.0 * (float)(*inBasis)[3 * basisVecIter + dimIter]);
                else
                    LOBYTE(v2) = 127;
            }
            else
            {
                LOBYTE(v2) = 0x80;
            }
            (*outBasis)[3 * basisVecIter + dimIter] = v2;
        }
    }
}

int __cdecl FX_TrailElemToHandle(FxSystem *system, FxTrailElem *trailElem)
{
    if ( !system
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\effectscore\\fx_system.h", 486, 0, "%s", "system") )
    {
        __debugbreak();
    }
    return FX_PoolToHandle_Generic<FxTrailElem,FxTrailElem,2048>(system->trailElems, trailElem);
}

FxPool<FxTrailElem,FxTrailElem> *__cdecl FX_AllocTrailElem(FxEffect *remoteEffect, FxSystem *system)
{
    return FX_AllocPool_Generic_FxTrailElem_FxTrailElem_(
                     remoteEffect,
                     (unsigned __int32 *)&system->shared->firstFreeTrailElem,
                     system->trailElems,
                     &system->shared->activeTrailElemCount);
}

void __cdecl FX_SpawnTrailElem_Cull(
                FxSystem *system,
                FxEffect *effect,
                FxEffect *remoteEffect,
                FxTrail *trail,
                const FxSpatialFrame *effectFrameWhenPlayed,
                int msecWhenPlayed,
                float distanceWhenPlayed)
{
    const FxElemDef *elemDef; // [esp+48h] [ebp-4h]

    if ( !system
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp", 2722, 0, "%s", "system") )
    {
        __debugbreak();
    }
    if ( !effect
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp", 2723, 0, "%s", "effect") )
    {
        __debugbreak();
    }
    if ( !effect->def
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp", 2725, 0, "%s", "effectDef") )
    {
        __debugbreak();
    }
    if ( !trail
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp", 2726, 0, "%s", "trail") )
    {
        __debugbreak();
    }
    elemDef = &effect->def->elemDefsEA[trail->defIndex];
    if ( elemDef->elemType != 5
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                    2729,
                    0,
                    "%s\n\t(elemDef->elemType) = %i",
                    "(elemDef->elemType == FX_ELEM_TYPE_TRAIL)",
                    elemDef->elemType) )
    {
        __debugbreak();
    }
    if ( (!system->extraCameraPre.isValid
         || FX_CullTrailElem(&system->extraCameraPre, elemDef, effectFrameWhenPlayed->origin, trail->sequence))
        && FX_CullTrailElem(&system->cameraPrev, elemDef, effectFrameWhenPlayed->origin, trail->sequence) )
    {
        ++trail->sequence;
    }
    else
    {
        FX_SpawnTrailElem_NoCull(
            system,
            effect,
            remoteEffect,
            trail,
            effectFrameWhenPlayed,
            msecWhenPlayed,
            distanceWhenPlayed);
    }
}

bool __cdecl FX_CullTrailElem(
                const FxCamera *camera,
                const FxElemDef *elemDef,
                const float *origin,
                unsigned __int8 sequence)
{
    float cutoffMultiple; // [esp+Ch] [ebp-10h]
    float distSq; // [esp+14h] [ebp-8h]
    float baseCutoffDist; // [esp+18h] [ebp-4h]

    baseCutoffDist = elemDef->spawnRange.base + elemDef->spawnRange.amplitude;
    if ( baseCutoffDist == 0.0 )
        return 0;
    if ( !sequence )
        return 0;
    cutoffMultiple = 1.0f;
    while ( (sequence & 1) == 0 )
    {
        cutoffMultiple = cutoffMultiple + 1.0;
        sequence >>= 1;
    }
    distSq = Vec3DistanceSq(origin, camera->origin);
    return distSq > (float)((float)(baseCutoffDist * cutoffMultiple) * (float)(baseCutoffDist * cutoffMultiple));
}

void __cdecl FX_SpawnSpotLightElem(FxSystem *system, FxElem *elem)
{
    if ( system->shared->activeSpotLightEffectCount <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                    2777,
                    0,
                    "%s",
                    "system->shared->activeSpotLightEffectCount > 0") )
    {
        __debugbreak();
    }
    if ( system->shared->activeSpotLightElemCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                    2778,
                    0,
                    "%s",
                    "system->shared->activeSpotLightElemCount == 0") )
    {
        __debugbreak();
    }
    _InterlockedExchangeAdd(&system->shared->activeSpotLightElemCount, 1u);
    system->shared->activeSpotLightElemHandle = FX_ElemToHandle(system, elem);
}

bool __cdecl FX_ImpactMarkWithinRange(const FxSystem *system, const float *markLocation)
{
    float range; // [esp+10h] [ebp-4h]

    if ( !system->camera.isValid )
        return 1;
    range = fx_marks_range->current.value;
    return range > 0.0 && range * range > Vec3DistanceSq(system->camera.origin, markLocation);
}

void __cdecl FX_SpawnElem(
                FxSystem *system,
                FxEffect *effect,
                FxEffectContainer *remoteEffect,
                int elemDefIndex,
                FxSpatialFrame *effectFrameWhenPlayed,
                int msecWhenPlayed,
                float distanceWhenPlayed,
                int sequence)
{
    unsigned __int16 v8; // ax
    float v9; // [esp+0h] [ebp-B4h]
    bool v10; // [esp+7h] [ebp-ADh]
    unsigned __int8 elemType; // [esp+1Bh] [ebp-99h]
    float v12; // [esp+68h] [ebp-4Ch]
    float v13; // [esp+6Ch] [ebp-48h]
    bool v14; // [esp+77h] [ebp-3Dh]
    int msecBegin; // [esp+88h] [ebp-2Ch]
    unsigned __int16 nextElemHandle; // [esp+8Ch] [ebp-28h]
    const FxElemDef *elemDef; // [esp+94h] [ebp-20h]
    unsigned __int8 spawnVisBits; // [esp+9Bh] [ebp-19h]
    int markEntnum; // [esp+9Ch] [ebp-18h]
    unsigned int randomSeed; // [esp+A0h] [ebp-14h]
    FxPool<FxElem,FxElemContainer> *remoteElem; // [esp+A4h] [ebp-10h]
    unsigned __int8 priority; // [esp+AAh] [ebp-Ah]
    unsigned int elemClass; // [esp+B0h] [ebp-4h]

    if ( !system
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp", 2849, 0, "%s", "system") )
    {
        __debugbreak();
    }
    if ( !effect
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp", 2850, 0, "%s", "effect") )
    {
        __debugbreak();
    }
    if ( !effect->def
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp", 2851, 0, "%s", "effect->def") )
    {
        __debugbreak();
    }
    elemDef = &effect->def->elemDefsEA[elemDefIndex];
    if ( !elemDef
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                    2853,
                    0,
                    "%s",
                    "elemDef != NULL") )
    {
        __debugbreak();
    }
    if ( elemDef )
    {
        if ( elemDef->elemType == 5
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                        2858,
                        0,
                        "%s",
                        "elemDef->elemType != FX_ELEM_TYPE_TRAIL") )
        {
            __debugbreak();
        }
        spawnVisBits = FX_ElemSpawnVisBits(system, elemDef, effectFrameWhenPlayed->origin);
        if ( spawnVisBits )
        {
            msecBegin = elemDef->spawnDelayMsec.base + msecWhenPlayed;
            if ( elemDef->spawnDelayMsec.amplitude )
                msecBegin += ((elemDef->spawnDelayMsec.amplitude + 1) * LOWORD(fx_randomTable[(296 * sequence + msecBegin + (unsigned int)effect->randomSeed) % 0x1DF + 18])) >> 16;
            randomSeed = (msecBegin + effect->randomSeed + 296 * (unsigned int)(unsigned __int8)sequence) % 0x1DF;
            priority = effect->def->efPriority;
            v10 = elemDef->flags < 0 && (!cg_blood->current.enabled || !CG_IsMature());
            switch ( elemDef->elemType )
            {
                case 0xCu:
                    FX_SpawnRunner(system, effect, elemDef, effectFrameWhenPlayed, randomSeed, msecBegin);
                    break;
                //case 0xBu:
                //    if ( !v10 )
                //    {
                //        if ( ((*(unsigned int *)&effect->boltAndSortOrder.0 >> 29) & 3) == 3 )
                //            markEntnum = (*(unsigned int *)&effect->boltAndSortOrder.0 >> 8) & 0x7FF;
                //        else
                //            markEntnum = 1023;
                //        if ( FX_ImpactMarkWithinRange(system, effectFrameWhenPlayed->origin) )
                //            FX_CreateImpactMark(system->localClientNum, elemDef, effectFrameWhenPlayed, randomSeed, markEntnum);
                //    }
                //    break;
                case 0xBu:
                {
                    if (!v10)
                    {
                        const FxBoltAndSortOrder &bolt = effect->boltAndSortOrder;

                        unsigned int markEntnum;

                        // Old: ((bolt >> 29) & 3) == 3
                        if (bolt.type == 3)
                        {
                            // Old: (bolt >> 8) & 0x7FF
                            markEntnum = bolt.dobjHandle;
                        }
                        else
                        {
                            markEntnum = 1023;
                        }

                        if (FX_ImpactMarkWithinRange(system,
                            effectFrameWhenPlayed->origin))
                        {
                            FX_CreateImpactMark(system->localClientNum,
                                elemDef,
                                effectFrameWhenPlayed,
                                randomSeed,
                                markEntnum);
                        }
                    }
                    break;
                }
                case 0xAu:
                    FX_SpawnSound(system->localClientNum, effect, elemDef, effectFrameWhenPlayed, randomSeed);
                    break;
                default:
                    if ( elemDef->elemType != 7 || !v10 )
                    {
                        if ( elemDef->effectOnImpact.handle )
                            v14 = 1;
                        else
                            v14 = elemDef->effectOnDeath.handle || elemDef->effectEmitted.handle != 0;
                        if ( v14
                            || msecBegin
                             + (((elemDef->lifeSpanMsec.amplitude + 1) * LOWORD(fx_randomTable[randomSeed + 17])) >> 16)
                             + elemDef->lifeSpanMsec.base > system->msecNow )
                        {
                            remoteElem = FX_AllocElem(&remoteEffect->effect, system);
                            if ( !remoteElem || system->shared->activeElemCount > 2018 )
                                FX_SetWarningPriority(system, priority);
                            if ( remoteElem )
                            {
                                FX_AddRefToEffect(system, &remoteEffect->effect);
                                remoteElem->item.elem.defIndex = elemDefIndex;
                                remoteElem->item.elem.sequence = sequence;
                                remoteElem->item.elem.atRestFraction = -1;
                                remoteElem->item.elem.emitResidual = 0;
                                remoteElem->item.elem.msecBegin = msecBegin;
                                remoteElem->item.elem.attachedEffectHandle = 0;
                                remoteElem->item.elem.elemSpawnVisBits = spawnVisBits;
                                if ( elemDef->maxWindStrength )
                                {
                                    v12 = FX_GetGlobalWind()->windVectorMagnitude / (float)elemDef->maxWindStrength;
                                    if ( (float)(v12 - 1.0) < 0.0 )
                                        v13 = v12;
                                    else
                                        v13 = 1.0f;
                                    if ( (float)(0.0 - v12) < 0.0 )
                                        v9 = v13;
                                    else
                                        v9 = 0.0f;
                                    remoteElem->item.elem.windInterp = (int)(float)(v9 * 65535.0);
                                }
                                else
                                {
                                    remoteElem->item.elem.windInterp = 0;
                                }
                                if ( randomSeed != (remoteElem->item.elem.msecBegin
                                                                    + effect->randomSeed
                                                                    + 296 * (unsigned int)remoteElem->item.elem.sequence)
                                                                 % 0x1DF
                                    && !Assert_MyHandler(
                                                "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                                                3002,
                                                0,
                                                "%s",
                                                "randomSeed == FX_ElemRandomSeed( effect->randomSeed, elem->msecBegin, elem->sequence )") )
                                {
                                    __debugbreak();
                                }
                                if ( elemDef->elemType != 7 || (elemDef->flags & 0x8000000) == 0 )
                                    FX_GetOriginForElem(effect, elemDef, effectFrameWhenPlayed, randomSeed, remoteElem->item.elem.origin);
                                remoteElem->item.elem.baseVel[0] = 0.0f;
                                remoteElem->item.elem.baseVel[1] = 0.0f;
                                remoteElem->item.elem.baseVel[2] = 0.0f;
                                if ( elemDef->elemType == 5 )
                                    remoteElem->item.elem.u.trailTexCoord = distanceWhenPlayed / (float)elemDef->localTrailDef->repeatDist;
                                remoteElem->item.shared.prevElemHandleInEffect = -1;
                                if ( elemDef->elemType == 9 )
                                {
                                    FX_SpawnSpotLightElem(system, (FxElem *)remoteElem);
                                }
                                else
                                {
                                    elemType = elemDef->elemType;
                                    if ( elemType > 5u )
                                    {
                                        if ( elemType == 6 )
                                            elemClass = 2;
                                        else
                                            elemClass = 1;
                                    }
                                    else
                                    {
                                        elemClass = 0;
                                    }
                                    nextElemHandle = effect->firstElemHandle[elemClass];
                                    remoteElem->item.shared.nextElemHandleInEffect = nextElemHandle;
                                    effect->firstElemHandle[elemClass] = FX_ElemToHandle(system, (FxElem *)remoteElem);
                                    if ( nextElemHandle != 0xFFFF )
                                        FX_ElemFromHandle(system, nextElemHandle)->item.shared.prevElemHandleInEffect = effect->firstElemHandle[elemClass];
                                    if ( elemDef->elemType == 7 )
                                    {
                                        remoteElem->item.elem.u.s.lightingHandle = 0;
                                        if ( (elemDef->flags & 0x8000000) != 0
                                            && !FX_SpawnModelPhysics(system, effect, elemDef, randomSeed, (FxElem *)remoteElem, sequence) )
                                        {
                                            v8 = FX_ElemToHandle(system, (FxElem *)remoteElem);
                                            FX_FreeElem(system, v8, effect, remoteEffect, elemClass);
                                        }
                                    }
                                    remoteElem->item.elem.u.s.fade.fadeAlphaPerSec = 0;
                                    remoteElem->item.elem.u.s.fade.fadeAlpha = 0;
                                }
                            }
                            else
                            {
                                R_WarnOncePerFrame(R_WARN_FX_ELEM_LIMIT);
                            }
                        }
                    }
                    break;
            }
        }
    }
}

FxPool<FxElem, FxElemContainer> *__cdecl FX_AllocPool_Generic_FxElem_FxElemContainer_(
    FxEffect *remoteEffect,
    unsigned __int32 *firstFreeIndex,
    FxPool<FxElem, FxElemContainer> *pool,
    volatile unsigned int *activeCount)
{
    unsigned __int32 itemIndex; // [esp+8Ch] [ebp-8h]
    unsigned __int32 nextFree; // [esp+90h] [ebp-4h]

    do
    {
        itemIndex = *firstFreeIndex;
        if (*firstFreeIndex == -1)
            return 0;
        nextFree = pool[itemIndex].nextFree;
    } while (_InterlockedCompareExchange((volatile unsigned __int32 *)firstFreeIndex, nextFree, itemIndex) != itemIndex);
    if (itemIndex != -1
        && itemIndex >= 0x800
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
            247,
            0,
            "%s",
            "itemIndex == -1 || (itemIndex >= 0 && itemIndex < ITEM_TYPE::POOL_SIZE)"))
    {
        __debugbreak();
    }
    if (nextFree != -1
        && nextFree >= 0x800
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
            248,
            0,
            "%s",
            "nextFree == -1 || (nextFree >= 0 && nextFree < ITEM_TYPE::POOL_SIZE)"))
    {
        __debugbreak();
    }
    _InterlockedExchangeAdd(activeCount, 1u);
    return &pool[itemIndex];
}

FxPool<FxElem,FxElemContainer> *__cdecl FX_AllocElem(FxEffect *remoteEffect, FxSystem *system)
{
    return FX_AllocPool_Generic_FxElem_FxElemContainer_(
                     remoteEffect,
                     (unsigned __int32 *)system->shared,
                     system->elems,
                     &system->shared->activeElemCount);
}

void FX_SpawnRunner(
    FxSystem *system,
    FxEffect *effect,
    const FxElemDef *remoteElemDef,
    const FxSpatialFrame *effectFrameWhenPlayed,
    int randomSeed,
    int msecWhenPlayed)
{
    FxEffectContainer *spawnedContainer;
    const FxEffectDef *effectDef;

    float spawnOrigin[3];
    float axis[3][3];
    float rotatedAxis[3][3];
    float *usedAxis;

    /*
        Spawn origin setup
    */
    FX_GetSpawnOrigin(effectFrameWhenPlayed, remoteElemDef, randomSeed, spawnOrigin);
    FX_OffsetSpawnOrigin(effectFrameWhenPlayed, remoteElemDef, randomSeed, spawnOrigin);

    effectDef = FX_GetElemVisuals(remoteElemDef, randomSeed).effectDef.handle;

    UnitQuatToAxis(effectFrameWhenPlayed->quat, axis);

    if (remoteElemDef->flags & 8)
    {
        FX_RandomlyRotateAxis(axis, randomSeed, rotatedAxis);
        usedAxis = rotatedAxis[0];
    }
    else
    {
        usedAxis = axis[0];
    }

    /*
        Sort order clamp
    */
    int sortOrder = (remoteElemDef->sortOrder == 255)
        ? 255
        : remoteElemDef->sortOrder;

    int runnerSortOrder = (sortOrder > 0) ? sortOrder : 0;

    FX_PlayElementSpawnSound(system->localClientNum,
        remoteElemDef,
        spawnOrigin);

    const FxBoltAndSortOrder &bolt = effect->boltAndSortOrder;

    unsigned int spawnedHandle;

    /*
        Case 1:
        type == 2
    */
    if (bolt.type == 2)
    {
        spawnedHandle = FX_SpawnEffect(
            system,
            effectDef,
            msecWhenPlayed,
            spawnOrigin,
            (const float (*)[3])usedAxis,
            2047,
            511,
            runnerSortOrder,
            effect->owner,
            0x3FFu,
            &orIdentity);
    }

    /*
        Case 2:
        boneIndex == 511 (0x1FF)
    */
    else if (bolt.boneIndex == 0x1FF)
    {
        unsigned int markEnt =
            (bolt.dobjHandle == 0x7FF)
            ? 0x3FFu
            : bolt.dobjHandle;

        spawnedHandle = FX_SpawnEffect(
            system,
            effectDef,
            msecWhenPlayed,
            spawnOrigin,
            (const float (*)[3])usedAxis,
            2047,
            511,
            runnerSortOrder,
            effect->owner,
            markEnt,
            &orIdentity);
    }

    /*
        Case 3:
        Normal DObj/Bone case
    */
    else
    {
        spawnedHandle = FX_SpawnEffect(
            system,
            effectDef,
            msecWhenPlayed,
            spawnOrigin,
            (const float (*)[3])usedAxis,
            bolt.dobjHandle,
            bolt.boneIndex,
            runnerSortOrder,
            effect->owner,
            0x3FFu,
            &orIdentity);
    }

    spawnedContainer = UniqueHandleToEffect(system, spawnedHandle);

    if (spawnedContainer)
    {
        FX_DelRefToEffect(system, spawnedContainer, 0);
    }
}


#if 0
void __cdecl FX_SpawnRunner(
                FxSystem *system,
                FxEffect *effect,
                const FxElemDef *remoteElemDef,
                const FxSpatialFrame *effectFrameWhenPlayed,
                int randomSeed,
                int msecWhenPlayed)
{
    FxEffectContainer *v6; // eax
    int v7; // [esp+0h] [ebp-88h]
    int sortOrder; // [esp+Ch] [ebp-7Ch]
    const FxEffectDef *effectDef; // [esp+20h] [ebp-68h]
    unsigned int spawnedEffecta; // [esp+28h] [ebp-60h]
    unsigned int spawnedEffectb; // [esp+28h] [ebp-60h]
    unsigned int spawnedEffect; // [esp+28h] [ebp-60h]
    float *usedAxis; // [esp+30h] [ebp-58h]
    float rotatedAxis[3][3]; // [esp+34h] [ebp-54h] BYREF
    float spawnOrigin[3]; // [esp+58h] [ebp-30h] BYREF
    float axis[3][3]; // [esp+64h] [ebp-24h] BYREF

    FX_GetSpawnOrigin(effectFrameWhenPlayed, remoteElemDef, randomSeed, spawnOrigin);
    FX_OffsetSpawnOrigin(effectFrameWhenPlayed, remoteElemDef, randomSeed, spawnOrigin);
    effectDef = FX_GetElemVisuals(remoteElemDef, randomSeed).effectDef.handle;
    UnitQuatToAxis(effectFrameWhenPlayed->quat, axis);
    if ( (remoteElemDef->flags & 8) != 0 )
    {
        FX_RandomlyRotateAxis(axis, randomSeed, rotatedAxis);
        usedAxis = rotatedAxis[0];
    }
    else
    {
        usedAxis = axis[0];
    }
    if ( remoteElemDef->sortOrder == 255 )
        sortOrder = 255;
    else
        sortOrder = remoteElemDef->sortOrder;
    if ( sortOrder > 0 )
        v7 = sortOrder;
    else
        v7 = 0;
    FX_PlayElementSpawnSound(system->localClientNum, remoteElemDef, spawnOrigin);
    if ( ((*(unsigned int *)&effect->boltAndSortOrder.0 >> 29) & 3) == 2 )
    {
        spawnedEffecta = FX_SpawnEffect(
                                             system,
                                             effectDef,
                                             msecWhenPlayed,
                                             spawnOrigin,
                                             (const float (*)[3])usedAxis,
                                             2047,
                                             511,
                                             v7,
                                             effect->owner,
                                             0x3FFu,
                                             &orIdentity);
        v6 = UniqueHandleToEffect(system, spawnedEffecta);
    }
    else if ( ((*(unsigned int *)&effect->boltAndSortOrder.0 >> 20) & 0x1FF) == 0x1FF )
    {
        if ( ((*(unsigned int *)&effect->boltAndSortOrder.0 >> 8) & 0x7FF) == 0x7FF )
            spawnedEffect = FX_SpawnEffect(
                                                system,
                                                effectDef,
                                                msecWhenPlayed,
                                                spawnOrigin,
                                                (const float (*)[3])usedAxis,
                                                2047,
                                                511,
                                                v7,
                                                effect->owner,
                                                0x3FFu,
                                                &orIdentity);
        else
            spawnedEffect = FX_SpawnEffect(
                                                system,
                                                effectDef,
                                                msecWhenPlayed,
                                                spawnOrigin,
                                                (const float (*)[3])usedAxis,
                                                2047,
                                                511,
                                                v7,
                                                effect->owner,
                                                (*(unsigned int *)&effect->boltAndSortOrder.0 >> 8) & 0x7FF,
                                                &orIdentity);
        v6 = UniqueHandleToEffect(system, spawnedEffect);
    }
    else
    {
        spawnedEffectb = FX_SpawnEffect(
                                             system,
                                             effectDef,
                                             msecWhenPlayed,
                                             spawnOrigin,
                                             (const float (*)[3])usedAxis,
                                             (*(unsigned int *)&effect->boltAndSortOrder.0 >> 8) & 0x7FF,
                                             (*(unsigned int *)&effect->boltAndSortOrder.0 >> 20) & 0x1FF,
                                             v7,
                                             effect->owner,
                                             0x3FFu,
                                             &orIdentity);
        v6 = UniqueHandleToEffect(system, spawnedEffectb);
    }
    if ( v6 )
        FX_DelRefToEffect(system, v6, 0);
}
#endif


void FX_RandomlyRotateAxis(const float (*axisIn)[3], int randomSeed, float (*axisOut)[3])
{
    double v4; // st7
    char *v5; // eax
    double v6; // st7
    char *v7; // eax
    double v8; // st7
    char *v9; // eax
    float rotation; // [esp+3Ch] [ebp-4h]

    rotation = 360.0 * fx_randomTable[randomSeed + 24];
    (*axisOut)[0] = (*axisIn)[0];
    (*axisOut)[1] = (*axisIn)[1];
    (*axisOut)[2] = (*axisIn)[2];
    RotatePointAroundVector(&(*axisOut)[3], (const float *)axisOut, &(*axisIn)[3], rotation);
    Vec3Cross((const float *)axisOut, &(*axisOut)[3], &(*axisOut)[6]);
    if (!Vec3IsNormalized((const float *)axisOut))
    {
        v4 = Vec3Length((const float *)axisOut);
        v5 = va("(%g %g %g) len %g", (*axisOut)[0], (*axisOut)[1], (*axisOut)[2], v4);
        if (!Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
            2484,
            0,
            "%s\n\t%s",
            "Vec3IsNormalized( axisOut[0] )",
            v5))
            __debugbreak();
    }
    if (!Vec3IsNormalized(&(*axisOut)[3]))
    {
        v6 = Vec3Length(&(*axisOut)[3]);
        v7 = va("(%g %g %g) len %g", (*axisOut)[3], (*axisOut)[4], (*axisOut)[5], v6);
        if (!Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
            2485,
            0,
            "%s\n\t%s",
            "Vec3IsNormalized( axisOut[1] )",
            v7))
            __debugbreak();
    }
    if (!Vec3IsNormalized(&(*axisOut)[6]))
    {
        v8 = Vec3Length(&(*axisOut)[6]);
        v9 = va("(%g %g %g) len %g", (*axisOut)[6], (*axisOut)[7], (*axisOut)[8], v8);
        if (!Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
            2486,
            0,
            "%s\n\t%s",
            "Vec3IsNormalized( axisOut[2] )",
            v9))
            __debugbreak();
    }
}

bool __cdecl FX_SpawnModelPhysics(
    FxSystem *system,
    FxEffect *effect,
    const FxElemDef *elemDef,
    int randomSeed,
    FxElem *elem,
    int sequence)
{
    float v7; // [esp+14h] [ebp-164h]
    unsigned int rotationAxis; // [esp+70h] [ebp-108h]
    float v[4]; // [esp+74h] [ebp-104h] BYREF
    float v10; // [esp+84h] [ebp-F4h]
    FxElemVisuals v11; // [esp+88h] [ebp-F0h]
    float4 fAxis; // [esp+8Ch] [ebp-ECh]
    float4 qAxis; // [esp+A0h] [ebp-D8h] BYREF
    float rotaxis[3][3]; // [esp+B0h] [ebp-C8h] BYREF
    float velocity[3]; // [esp+D4h] [ebp-A4h] BYREF
    float angularVelocity[3]; // [esp+E0h] [ebp-98h] BYREF
    FxElemVisuals visuals; // [esp+ECh] [ebp-8Ch]
    float msecLifeSpan; // [esp+F0h] [ebp-88h]
    float quat[4]; // [esp+F4h] [ebp-84h] BYREF
    orientation_t orient; // [esp+104h] [ebp-74h] BYREF
    float worldOrigin[3]; // [esp+134h] [ebp-44h] BYREF
    create_gjk_geom_collision_visitor collision_visitor; // [esp+140h] [ebp-38h] BYREF
    gjk_geom_list_t gjk_geom_list; // [esp+148h] [ebp-30h] BYREF
    float axis[3][3]; // [esp+150h] [ebp-28h] BYREF
    int id; // [esp+174h] [ebp-4h]
    int savedregs; // [esp+178h] [ebp+0h] BYREF

    FX_GetOrientation(elemDef, &effect->frameAtSpawn, &effect->frameNow, randomSeed, &orient);
    FX_OrientationPosToWorldPos(&orient, elem->origin, worldOrigin);
    if ((0x800000 & elemDef->flags) != 0)
    {
        rotationAxis = elemDef->rotationAxis;
        v[0] = (float)(COERCE_FLOAT((rotationAxis & 0x1FF) + 1077936128 - 2 * (rotationAxis & 0x100)) - 3.0) * 16448.252;
        v[1] = (float)(COERCE_FLOAT(((rotationAxis >> 9) & 0x3FF) + 1077936128 - 2 * ((rotationAxis >> 9) & 0x200)) - 3.0)
            * 8208.0312;
        v[2] = (float)(COERCE_FLOAT(((rotationAxis >> 19) & 0x3FF) + 1077936128 - 2 * ((rotationAxis >> 19) & 0x200)) - 3.0)
            * 8208.0312;
        v[3] = 1.0f;
        v10 = Vec4Length(v);
        if ((rotationAxis & 0x80000000) == 0)
            v7 = 1.0f;
        else
            v7 = -1.0f;
        Vec4Scale(v, v7 / v10, v);
        fAxis.v[0] = v[(rotationAxis >> 29) & 3];
        fAxis.v[1] = v[(((rotationAxis >> 29) & 3) + 1) & 3];
        fAxis.v[2] = v[(((rotationAxis >> 29) & 3) + 2) & 3];
        fAxis.v[3] = v[(((rotationAxis >> 29) & 3) + 3) & 3];
        qAxis = fAxis;
        UnitQuatToAxis(qAxis.v, rotaxis);
        MatrixMultiply(rotaxis, orient.axis, axis);
        *(_QWORD *)&orient.axis[0][0] = *(_QWORD *)&axis[0][0];
        orient.axis[0][2] = axis[0][2];
        orient.axis[1][0] = axis[1][0];
        orient.axis[1][1] = axis[1][1];
        orient.axis[1][2] = axis[1][2];
        orient.axis[2][0] = axis[2][0];
        orient.axis[2][1] = axis[2][1];
        orient.axis[2][2] = axis[2][2];
    }
    FX_GetElemAxis(elemDef, randomSeed, &orient, 0.0, axis);
    AxisToQuat(axis, quat);
    msecLifeSpan = (float)((((elemDef->lifeSpanMsec.amplitude + 1) * LOWORD(fx_randomTable[randomSeed + 17])) >> 16)
        + elemDef->lifeSpanMsec.base);
    FX_GetVelocityAtTime(elemDef, randomSeed, msecLifeSpan, 0.0, &orient, elem->baseVel, velocity);
    angularVelocity[0] = (float)((float)(fx_randomTable[randomSeed + 3] * elemDef->angularVelocity[0].amplitude)
        + elemDef->angularVelocity[0].base)
        * 1000.0;
    angularVelocity[1] = (float)((float)(fx_randomTable[randomSeed + 4] * elemDef->angularVelocity[1].amplitude)
        + elemDef->angularVelocity[1].base)
        * 1000.0;
    angularVelocity[2] = (float)((float)(fx_randomTable[randomSeed + 5] * elemDef->angularVelocity[2].amplitude)
        + elemDef->angularVelocity[2].base)
        * 1000.0;
    Sys_EnterCriticalSection(CRITSECT_PHYSICS_UPDATE);
    Sys_EnterCriticalSection(CRITSECT_PHYSICS);
    v11.anonymous = FX_GetElemVisuals(elemDef, randomSeed).anonymous;
    visuals.anonymous = v11.anonymous;
    if (!*((_DWORD *)v11.anonymous + 59)
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
            2591,
            0,
            "%s",
            "visuals.model->physPreset"))
    {
        __debugbreak();
    }
    id = FX_MakePhysId(elemDef, orient.origin, orient.axis[0], system->frameCount, sequence);
    gjk_geom_list.m_first_geom = 0;
    gjk_geom_list.m_geom_count = 0;
    //collision_visitor.__vftable = (create_gjk_geom_collision_visitor_vtbl *)&create_gjk_geom_collision_visitor::`vftable';
    collision_visitor.gjk_geom_list = &gjk_geom_list;
    create_xmodel_gjk_geom(
        visuals.model,
        &collision_visitor,
        -1,
        0x280EC93,
        1,
        0,
        0);
    if (elem->physObjId
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
            2598,
            0,
            "%s",
            "!elem->physObjId"))
    {
        __debugbreak();
    }
    elem->physObjId = (int)Phys_ObjCreate(
        1,
        worldOrigin,
        quat,
        velocity,
        *((const PhysPreset **)visuals.anonymous + 59),
        &gjk_geom_list,
        1,
        id);
    if (elem->physObjId)
        Phys_ObjSetAngularVelocity(elem->physObjId, angularVelocity);
    Sys_LeaveCriticalSection(CRITSECT_PHYSICS);
    Sys_LeaveCriticalSection(CRITSECT_PHYSICS_UPDATE);
    return elem->physObjId != 0;
}


void __cdecl Vec4Scale(const float *v, float scale, float *result)
{
    *result = scale * *v;
    result[1] = scale * v[1];
    result[2] = scale * v[2];
    result[3] = scale * v[3];
}

void __cdecl FX_GetOriginForElem(
                FxEffect *effect,
                const FxElemDef *elemDef,
                FxSpatialFrame *effectFrameWhenPlayed,
                int randomSeed,
                float *outOrigin)
{
    FxSpatialFrame *p_frameAtSpawn; // [esp+0h] [ebp-48h]
    float effectFrameAxis[3][3]; // [esp+10h] [ebp-38h] BYREF
    const FxSpatialFrame *effectFrame; // [esp+34h] [ebp-14h]
    float delta[3]; // [esp+38h] [ebp-10h]
    int runFlags; // [esp+44h] [ebp-4h]

    runFlags = elemDef->flags & 0xC0;
    if ( runFlags == 64 )
        p_frameAtSpawn = &effect->frameAtSpawn;
    else
        p_frameAtSpawn = effectFrameWhenPlayed;
    effectFrame = p_frameAtSpawn;
    if ( runFlags == 192 )
    {
        *outOrigin = 0.0f;
        outOrigin[1] = 0.0f;
        outOrigin[2] = 0.0f;
    }
    else
    {
        UnitQuatToAxis(effectFrame->quat, effectFrameAxis);
        FX_GetSpawnOrigin(effectFrame, elemDef, randomSeed, outOrigin);
        FX_OffsetSpawnOrigin(effectFrame, elemDef, randomSeed, outOrigin);
        if ( runFlags == 128 || runFlags == 64 )
        {
            delta[0] = *outOrigin - effectFrame->origin[0];
            delta[1] = outOrigin[1] - effectFrame->origin[1];
            delta[2] = outOrigin[2] - effectFrame->origin[2];
            *outOrigin = (float)((float)(delta[0] * effectFrameAxis[0][0]) + (float)(delta[1] * effectFrameAxis[0][1]))
                                 + (float)(delta[2] * effectFrameAxis[0][2]);
            outOrigin[1] = (float)((float)(delta[0] * effectFrameAxis[1][0]) + (float)(delta[1] * effectFrameAxis[1][1]))
                                     + (float)(delta[2] * effectFrameAxis[1][2]);
            outOrigin[2] = (float)((float)(delta[0] * effectFrameAxis[2][0]) + (float)(delta[1] * effectFrameAxis[2][1]))
                                     + (float)(delta[2] * effectFrameAxis[2][2]);
        }
    }
}

void __cdecl FX_SpawnSound(
                int localClientNumber,
                FxEffect *effect,
                const FxElemDef *elemDef,
                const FxSpatialFrame *effectFrameWhenPlayed,
                int randomSeed)
{
    SndEntHandle entHandle; // [esp+1Ch] [ebp-18h]
    char *visuals; // [esp+24h] [ebp-10h]
    float spawnOrigin[3]; // [esp+28h] [ebp-Ch] BYREF

    if ( elemDef->elemType != 10
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                    2752,
                    0,
                    "%s\n\t(elemDef->elemType) = %i",
                    "(elemDef->elemType == FX_ELEM_TYPE_SOUND)",
                    elemDef->elemType) )
    {
        __debugbreak();
    }
    FX_GetSpawnOrigin(effectFrameWhenPlayed, elemDef, randomSeed, spawnOrigin);
    FX_OffsetSpawnOrigin(effectFrameWhenPlayed, elemDef, randomSeed, spawnOrigin);
    if ( elemDef->visualCount )
    {
        visuals = (char *)FX_GetElemVisuals(elemDef, randomSeed).anonymous;
        entHandle.field = CG_SndEntHandle(localClientNumber, 1022).field;
        SND_Play(visuals, 0, 1.0, entHandle, spawnOrigin, 0, 0);
    }
    else
    {
        Com_PrintWarning(21, "No visuals for sound particle element in '%s'\n", effect->def->name);
    }
}

unsigned __int8 __cdecl FX_ElemSpawnVisBits(const FxSystem *system, const FxElemDef *elemDef, const float *origin)
{
    unsigned int bits; // [esp+34h] [ebp-4h]

    bits = 3;
    if ( !fx_cull_elem_spawn->current.enabled )
        return 3;
    if ( !system->extraCameraPre.isValid || FX_CullElemForSpawn(&system->extraCameraPre, elemDef, origin) )
        bits = 1;
    if ( FX_CullElemForSpawn(&system->cameraPrev, elemDef, origin) )
        bits &= ~1u;
    if ( (bits & 0xFFFFFF00) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                    2809,
                    0,
                    "%s",
                    "0 == ( bits & ~0xff )") )
    {
        __debugbreak();
    }
    return bits;
}

void __cdecl FX_FreePool_Generic_FxElem_FxElemContainer_(
    FxEffect *remoteEffect,
    FxElem *item_slim,
    unsigned __int32 *firstFreeIndex,
    FxPool<FxElem, FxElemContainer> *pool)
{
    unsigned __int32 nextFree; // [esp+8Ch] [ebp-8h]
    unsigned __int32 freedIndex; // [esp+90h] [ebp-4h]

    freedIndex = ((char *)item_slim - (char *)pool) / 48;
    if (freedIndex >= 0x800
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
            300,
            0,
            "%s",
            "freedIndex >= 0 && freedIndex < ITEM_TYPE::POOL_SIZE"))
    {
        __debugbreak();
    }
    do
    {
        nextFree = *firstFreeIndex;
        if (*firstFreeIndex == freedIndex
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                309,
                0,
                "%s",
                "nextFree != freedIndex"))
        {
            __debugbreak();
        }
        *(_DWORD *)&item_slim->defIndex = nextFree;
    } while (_InterlockedCompareExchange((volatile unsigned __int32 *)firstFreeIndex, freedIndex, nextFree) != nextFree);
    if (nextFree != -1
        && nextFree >= 0x800
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
            316,
            0,
            "%s",
            "nextFree == -1 || (nextFree >= 0 && nextFree < ITEM_TYPE::POOL_SIZE)"))
    {
        __debugbreak();
    }
}

void __cdecl FX_FreeElem(
                FxSystem *system,
                unsigned __int16 elemHandle,
                FxEffect *effect,
                FxEffectContainer *remoteEffect,
                unsigned int elemClass)
{
    volatile unsigned __int32 *p_status; // [esp+B0h] [ebp-2Ch]
    signed __int32 v6; // [esp+B4h] [ebp-28h]
    unsigned __int16 nextElemHandle; // [esp+BCh] [ebp-20h]
    const FxElemDef *elemDef; // [esp+C0h] [ebp-1Ch]
    FxEffectContainer *remoteAttachedEffect; // [esp+C4h] [ebp-18h]
    unsigned __int16 prevElemHandle; // [esp+C8h] [ebp-14h]
    FxPool<FxElem,FxElemContainer> *remoteElem; // [esp+D4h] [ebp-8h]

    remoteElem = FX_ElemFromHandle(system, elemHandle);
    remoteAttachedEffect = UniqueHandleToEffect(system, remoteElem->item.elem.attachedEffectHandle);
    if ( remoteAttachedEffect )
    {
        p_status = &remoteAttachedEffect->atomics.status;
        do
            v6 = *p_status;
        while ( _InterlockedCompareExchange(p_status, v6 | 0x4000, v6) != v6 );
        system->hasStaleAttachedFx = 1;
    }
    remoteElem->item.elem.attachedEffectHandle = 0;
    prevElemHandle = remoteElem->item.shared.prevElemHandleInEffect;
    nextElemHandle = remoteElem->item.shared.nextElemHandleInEffect;
    if ( prevElemHandle == elemHandle
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                    3072,
                    0,
                    "%s",
                    "prevElemHandle != elemHandle") )
    {
        __debugbreak();
    }
    if ( nextElemHandle == elemHandle
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                    3073,
                    0,
                    "%s",
                    "nextElemHandle != elemHandle") )
    {
        __debugbreak();
    }
    if ( !elemClass && effect->firstSortedElemHandle == elemHandle )
        effect->firstSortedElemHandle = nextElemHandle;
    if ( nextElemHandle != 0xFFFF )
        FX_ElemFromHandle(system, nextElemHandle)->item.shared.prevElemHandleInEffect = prevElemHandle;
    if ( prevElemHandle == 0xFFFF )
    {
        if ( effect->firstElemHandle[elemClass] != elemHandle
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                        3088,
                        0,
                        "%s",
                        "effect->firstElemHandle[elemClass] == elemHandle") )
        {
            __debugbreak();
        }
        effect->firstElemHandle[elemClass] = nextElemHandle;
    }
    else
    {
        FX_ElemFromHandle(system, prevElemHandle)->item.shared.nextElemHandleInEffect = nextElemHandle;
    }
    elemDef = &effect->def->elemDefsEA[remoteElem->item.elem.defIndex];
    if ( elemDef->elemType == 7 && (elemDef->flags & 0x8000000) != 0 && remoteElem->item.elem.physObjId )
    {
        Sys_EnterCriticalSection(CRITSECT_PHYSICS);
        Phys_ObjDestroy(1, remoteElem->item.elem.physObjId);
        Sys_LeaveCriticalSection(CRITSECT_PHYSICS);
    }
    memset(remoteElem, 0, 0x2Cu);
    FX_FreePool_Generic_FxElem_FxElemContainer_(
        &remoteEffect->effect,
        (FxElem *)remoteElem,
        (unsigned __int32 *)system->shared,
        system->elems);
    FX_DelRefToEffect(system, remoteEffect, 0);
    _InterlockedExchangeAdd(&system->shared->activeElemCount, 0xFFFFFFFF);
}

void __cdecl FX_FreePool_Generic_FxTrailElem_FxTrailElem_(
    FxEffect *remoteEffect,
    FxTrailElem *item_slim,
    unsigned __int32 *firstFreeIndex,
    FxPool<FxTrailElem, FxTrailElem> *pool)
{
    unsigned __int32 nextFree; // [esp+8Ch] [ebp-8h]
    unsigned __int32 freedIndex; // [esp+90h] [ebp-4h]

    freedIndex = ((char *)item_slim - (char *)pool) >> 5;
    if (freedIndex >= 0x800
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
            300,
            0,
            "%s",
            "freedIndex >= 0 && freedIndex < ITEM_TYPE::POOL_SIZE"))
    {
        __debugbreak();
    }
    do
    {
        nextFree = *firstFreeIndex;
        if (*firstFreeIndex == freedIndex
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                309,
                0,
                "%s",
                "nextFree != freedIndex"))
        {
            __debugbreak();
        }
        LODWORD(item_slim->origin[0]) = nextFree;
    } while (_InterlockedCompareExchange((volatile unsigned __int32 *)firstFreeIndex, freedIndex, nextFree) != nextFree);
    if (nextFree != -1
        && nextFree >= 0x800
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
            316,
            0,
            "%s",
            "nextFree == -1 || (nextFree >= 0 && nextFree < ITEM_TYPE::POOL_SIZE)"))
    {
        __debugbreak();
    }
}

void __cdecl FX_FreeTrailElem(
                FxSystem *system,
                unsigned __int16 trailElemHandle,
                FxEffect *effect,
                FxEffectContainer *remoteEffect,
                FxTrail *trail)
{
    FxPool<FxTrailElem,FxTrailElem> *remoteTrailElem; // [esp+98h] [ebp-8h]

    remoteTrailElem = FX_TrailElemFromHandle(system, trailElemHandle);
    if ( trail->firstElemHandle != trailElemHandle
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                    3138,
                    0,
                    "%s",
                    "trail->firstElemHandle == trailElemHandle") )
    {
        __debugbreak();
    }
    if ( trail->lastElemHandle == trailElemHandle )
    {
        if ( trail->firstElemHandle != trailElemHandle
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                        3142,
                        0,
                        "%s",
                        "trail->firstElemHandle == trailElemHandle") )
        {
            __debugbreak();
        }
        if ( remoteTrailElem->item.nextTrailElemHandle != 0xFFFF
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                        3143,
                        0,
                        "%s",
                        "trailElem->nextTrailElemHandle == FX_HANDLE_NONE") )
        {
            __debugbreak();
        }
        trail->lastElemHandle = -1;
    }
    trail->firstElemHandle = remoteTrailElem->item.nextTrailElemHandle;
    remoteTrailElem->nextFree = 0;
    remoteTrailElem->item.origin[1] = 0.0;
    remoteTrailElem->item.origin[2] = 0.0;
    remoteTrailElem->item.spawnDist = 0.0;
    remoteTrailElem->item.msecBegin = 0;
    *(unsigned int *)&remoteTrailElem->item.nextTrailElemHandle = 0;
    *(unsigned int *)&remoteTrailElem->item.basis[0][0] = 0;
    *(unsigned int *)&remoteTrailElem->item.basis[1][1] = 0;
    FX_FreePool_Generic_FxTrailElem_FxTrailElem_(
        &remoteEffect->effect,
        (FxTrailElem *)remoteTrailElem,
        (unsigned __int32 *)&system->shared->firstFreeTrailElem,
        system->trailElems);
    FX_DelRefToEffect(system, remoteEffect, 0);
    _InterlockedExchangeAdd(&system->shared->activeTrailElemCount, 0xFFFFFFFF);
}

void __cdecl FX_FreeSpotLightElem(FxSystem *system, unsigned __int16 elemHandle, FxEffectContainer *effect)
{
    FxPool<FxElem,FxElemContainer> *elem; // [esp+9Ch] [ebp-4h]

    if ( (system->shared->activeSpotLightEffectCount <= 0 || system->shared->activeSpotLightElemCount <= 0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                    3162,
                    0,
                    "%s",
                    "system->shared->activeSpotLightEffectCount > 0 && system->shared->activeSpotLightElemCount > 0") )
    {
        __debugbreak();
    }
    elem = FX_ElemFromHandle(system, system->shared->activeSpotLightElemHandle);
    FX_FreePool_Generic_FxElem_FxElemContainer_(
        &effect->effect,
        (FxElem *)elem,
        (unsigned __int32 *)system->shared,
        system->elems);
    FX_DelRefToEffect(system, effect, 0);
    _InterlockedExchangeAdd(&system->shared->activeElemCount, 0xFFFFFFFF);
    _InterlockedExchangeAdd(&system->shared->activeSpotLightElemCount, 0xFFFFFFFF);
}

double __cdecl FX_GetClientVisibility(int localClientNum, const float *start, const float *end)
{
    const FxVisBlocker *visBlocker; // [esp+18h] [ebp-50h]
    float totalVis; // [esp+1Ch] [ebp-4Ch]
    const FxVisState *visState; // [esp+20h] [ebp-48h]
    float dir[3]; // [esp+24h] [ebp-44h] BYREF
    float halfLen; // [esp+30h] [ebp-38h]
    int blockerIndex; // [esp+34h] [ebp-34h]
    float len; // [esp+38h] [ebp-30h]
    FxSystem *system; // [esp+3Ch] [ebp-2Ch]
    float projDir[3]; // [esp+40h] [ebp-28h]
    float projPt[3]; // [esp+4Ch] [ebp-1Ch] BYREF
    float dot; // [esp+58h] [ebp-10h]
    float distSq; // [esp+5Ch] [ebp-Ch]
    const FxVisState *visStateEA; // [esp+60h] [ebp-8h]
    float blockerRadius; // [esp+64h] [ebp-4h]

    system = (FxSystem *)FX_GetSystemRemote(localClientNum);
    Sys_EnterCriticalSection(CRITSECT_FX_VIS);
    visStateEA = system->visStateBufferRead;
    visState = visStateEA;
    if ( visStateEA && visStateEA->blockerCount )
    {
        dir[0] = *end - *start;
        dir[1] = end[1] - start[1];
        dir[2] = end[2] - start[2];
        len = Vec3Normalize(dir);
        if ( fx_visMinTraceDist->current.value <= len )
        {
            halfLen = len * 0.5;
            totalVis = 1.0f;
            if ( visState->blockerCount > 256
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp",
                            3233,
                            0,
                            "visState->blockerCount <= FX_VIS_BLOCKER_LIMIT\n\t%i, %i",
                            visState->blockerCount,
                            256) )
            {
                __debugbreak();
            }
            for ( blockerIndex = 0; blockerIndex < visState->blockerCount; ++blockerIndex )
            {
                visBlocker = &visState->blocker[blockerIndex];
                projDir[0] = visBlocker->origin[0] - *start;
                projDir[1] = visBlocker->origin[1] - start[1];
                projDir[2] = visBlocker->origin[2] - start[2];
                dot = (float)((float)(projDir[0] * dir[0]) + (float)(projDir[1] * dir[1])) + (float)(projDir[2] * dir[2]);
                if ( fabs(dot - halfLen) <= halfLen )
                {
                    projPt[0] = (float)(dot * dir[0]) + *start;
                    projPt[1] = (float)(dot * dir[1]) + start[1];
                    projPt[2] = (float)(dot * dir[2]) + start[2];
                    distSq = Vec3DistanceSq(visBlocker->origin, projPt);
                    blockerRadius = (float)visBlocker->radius * 0.0625;
                    if ( distSq < (float)(blockerRadius * blockerRadius) )
                        totalVis = (float)((float)visBlocker->visibility * 0.000015258789) * totalVis;
                }
            }
            Sys_LeaveCriticalSection(CRITSECT_FX_VIS);
            return totalVis;
        }
        else
        {
            Sys_LeaveCriticalSection(CRITSECT_FX_VIS);
            return 1.0;
        }
    }
    else
    {
        Sys_LeaveCriticalSection(CRITSECT_FX_VIS);
        return 1.0;
    }
}

bool __cdecl FX_ClientVisibilityTest(int localClientNum, const float *start, const float *end)
{
    float visibility; // [esp+0h] [ebp-4h]

    visibility = FX_GetClientVisibility(localClientNum, start, end);
    return visibility >= 0.000099999997;
}

char __cdecl FX_GetActiveSpotLightBoltDobj(int localClientNum, int *dobjHandleOut)
{
    FxSystemContainer *system; // [esp+0h] [ebp-4h]

    system = FX_GetSystem(localClientNum);
    if ( !system
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_system.cpp", 3326, 0, "%s", "system") )
    {
        __debugbreak();
    }
    if ( !system->system.shared->activeSpotLightEffectCount )
        return 0;
    if ( system->system.activeSpotLightBoltDobj == 2047 )
        return 0;
    if ( dobjHandleOut )
        *dobjHandleOut = system->system.activeSpotLightBoltDobj;
    return 1;
}

//int __cdecl FX_PoolToHandle_Generic<FxTrailElem,FxTrailElem,2048>(
//                FxPool<FxTrailElem,FxTrailElem> *poolArray,
//                FxTrailElem *item_slim)
//{
//    const char *v2; // eax
//
//    if ( !item_slim || item_slim < (FxTrailElem *)poolArray || item_slim >= (FxTrailElem *)&poolArray[2048] )
//    {
//        v2 = va("%p %p", poolArray, item_slim);
//        if ( !Assert_MyHandler(
//                        "c:\\projects_pc\\cod\\codsrc\\src\\effectscore\\fx_system.h",
//                        457,
//                        0,
//                        "%s\n\t%s",
//                        "item && item >= &poolArray[0].item && item < &poolArray[LIMIT].item",
//                        v2) )
//            __debugbreak();
//    }
//    return ((char *)item_slim - (char *)poolArray) / 4;
//}
//
//int __cdecl FX_PoolToHandle_Generic<FxTrail,FxTrail,128>(FxPool<FxTrail,FxTrail> *poolArray, FxTrail *item_slim)
//{
//    const char *v2; // eax
//
//    if ( !item_slim || item_slim < (FxTrail *)poolArray || item_slim >= (FxTrail *)&poolArray[128] )
//    {
//        v2 = va("%p %p", poolArray, item_slim);
//        if ( !Assert_MyHandler(
//                        "c:\\projects_pc\\cod\\codsrc\\src\\effectscore\\fx_system.h",
//                        457,
//                        0,
//                        "%s\n\t%s",
//                        "item && item >= &poolArray[0].item && item < &poolArray[LIMIT].item",
//                        v2) )
//            __debugbreak();
//    }
//    return ((char *)item_slim - (char *)poolArray) / 4;
//}

