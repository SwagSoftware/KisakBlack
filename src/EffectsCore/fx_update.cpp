#include "fx_update.h"
#include "fx_random.h"
#include <demo/demo_common.h>
#include "fx_wind.h"
#include "fx_system.h"
#include "fx_draw.h"
#include "fx_sort.h"
#include <cgame/cg_sound.h>
#include <sound/snd_public_async.h>
#include <qcommon/dobj_management.h>
#include <cgame_mp/cg_ents_mp.h>
#include <xanim/dobj_utils.h>
#include "fx_archive.h"
#include "fx_update_util.h"
#include "fx_unique_handle.h"
#include <universal/com_math_anglevectors.h>
#include <DynEntity/DynEntity_load_obj.h>
#include "fx_dvars.h"
#include <gfx_d3d/r_workercmds_common.h>
#include "fx_beam.h"
#include "fx_postlight.h"
#include "fx_sprite.h"
#include <gfx_d3d/r_water_sim.h>
#include "fx_marks.h"

#include <universal/q_shared.h>
#include <bgame/bg_wind.h>
#include <cgame_mp/cg_local_mp.h>


bool __cdecl FX_IsDemoPlaying()
{
    return Demo_IsPlaying();
}

void __cdecl FX_SpawnAllFutureLooping(
                FxSystem *system,
                FxEffect *effect,
                FxEffectContainer *remoteEffect,
                int elemDefFirst,
                int elemDefCount,
                const FxSpatialFrame *frameBegin,
                const FxSpatialFrame *frameEnd,
                int msecWhenPlayed,
                int msecUpdateBegin)
{
    int elemDefIndex; // [esp+88h] [ebp-4h]

    if ( !effect
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp", 516, 0, "%s", "effect") )
    {
        __debugbreak();
    }
    if ( !effect->def
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp", 519, 0, "%s", "effectDef") )
    {
        __debugbreak();
    }
    for ( elemDefIndex = elemDefFirst; elemDefIndex != elemDefCount + elemDefFirst; ++elemDefIndex )
    {
        if ( effect->def->elemDefsEA[elemDefIndex].spawn.looping.count != 0x7FFFFFFF )
            FX_SpawnLoopingElems(
                system,
                effect,
                remoteEffect,
                elemDefIndex,
                frameBegin,
                frameEnd,
                msecWhenPlayed,
                msecUpdateBegin,
                0x7FFFFFFF);
    }
}

void __cdecl FX_SpawnLoopingElems(
                FxSystem *system,
                FxEffect *effect,
                FxEffectContainer *remoteEffect,
                int elemDefIndex,
                const FxSpatialFrame *frameBegin,
                const FxSpatialFrame *frameEnd,
                int msecWhenPlayed,
                int msecUpdateBegin,
                int msecUpdateEnd)
{
    float v9; // [esp+20h] [ebp-80h]
    float v10; // [esp+34h] [ebp-6Ch]
    float v11; // [esp+38h] [ebp-68h]
    const FxEffectDef *effectDef; // [esp+60h] [ebp-40h]
    const FxElemDef *elemDef; // [esp+64h] [ebp-3Ch]
    int msecNextSpawn; // [esp+68h] [ebp-38h]
    float lerp; // [esp+6Ch] [ebp-34h]
    int spawnedCount; // [esp+74h] [ebp-2Ch]
    FxSpatialFrame frameWhenPlayed; // [esp+78h] [ebp-28h] BYREF
    int maxUpdateMsec; // [esp+94h] [ebp-Ch]
    int updateMsec; // [esp+98h] [ebp-8h]
    int intervalMsec; // [esp+9Ch] [ebp-4h]

    if ( !effect
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp", 377, 0, "%s", "effect") )
    {
        __debugbreak();
    }
    effectDef = effect->def;
    if ( !effect->def
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp", 380, 0, "%s", "effectDef") )
    {
        __debugbreak();
    }
    if ( elemDefIndex >= (unsigned int)(effectDef->elemDefCountEmission
                                                                        + effectDef->elemDefCountOneShot
                                                                        + effectDef->elemDefCountLooping)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
                    382,
                    0,
                    "elemDefIndex doesn't index effectDef->elemDefCountLooping + effectDef->elemDefCountOneShot + effectDef->elemDe"
                    "fCountEmission\n"
                    "\t%i not in [0, %i)",
                    elemDefIndex,
                    effectDef->elemDefCountEmission + effectDef->elemDefCountOneShot + effectDef->elemDefCountLooping) )
    {
        __debugbreak();
    }
    if ( elemDefIndex >= effectDef->elemDefCountLooping
        && elemDefIndex < effectDef->elemDefCountOneShot + effectDef->elemDefCountLooping
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
                    383,
                    0,
                    "%s",
                    "elemDefIndex < effectDef->elemDefCountLooping || elemDefIndex >= effectDef->elemDefCountLooping + effectDef->e"
                    "lemDefCountOneShot") )
    {
        __debugbreak();
    }
    if ( (msecWhenPlayed > msecUpdateBegin || msecUpdateBegin > msecUpdateEnd)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
                    384,
                    0,
                    "msecUpdateBegin not in [msecWhenPlayed, msecUpdateEnd]\n\t%g not in [%g, %g]",
                    (float)msecUpdateBegin,
                    (float)msecWhenPlayed,
                    (float)msecUpdateEnd) )
    {
        __debugbreak();
    }
    elemDef = &effect->def->elemDefsEA[elemDefIndex];
    if ( elemDef->elemType != 5 )
    {
        if ( msecUpdateEnd != 0x7FFFFFFF )
        {
            updateMsec = msecUpdateEnd - msecUpdateBegin;
            if ( msecUpdateEnd - msecUpdateBegin > 128 )
            {
                maxUpdateMsec = FX_LimitStabilizeTimeForElemDef_Recurse(elemDef, 0, updateMsec) + 1;
                elemDef = &effect->def->elemDefsEA[elemDefIndex];
                if ( updateMsec > maxUpdateMsec )
                    msecUpdateBegin += updateMsec - maxUpdateMsec;
            }
        }
        intervalMsec = elemDef->spawn.looping.intervalMsec;
        if ( elemDef->maxWindStrength )
        {
            v10 = FX_GetGlobalWind()->windVectorMagnitude / (float)elemDef->maxWindStrength;
            if ( (float)(v10 - 1.0) < 0.0 )
                v11 = v10;
            else
                v11 = 1.0f;
            if ( (float)(0.0 - v10) < 0.0 )
                v9 = v11;
            else
                v9 = 0.0f;
            intervalMsec = (int)(float)((float)((float)(1.0 - v9) * (float)intervalMsec)
                                                                + (float)((float)elemDef->spawnIntervalAtMaxWind * v9));
        }
        spawnedCount = (msecUpdateBegin - msecWhenPlayed) / intervalMsec + 1;
        msecNextSpawn = msecWhenPlayed + intervalMsec * spawnedCount;
        memcpy(&frameWhenPlayed, frameBegin, sizeof(frameWhenPlayed));
        while ( msecNextSpawn <= msecUpdateEnd && spawnedCount < elemDef->spawn.looping.count )
        {
            lerp = (float)(msecNextSpawn - msecUpdateBegin) / (float)(msecUpdateEnd - msecUpdateBegin);
            Vec3Lerp(frameBegin->origin, frameEnd->origin, lerp, frameWhenPlayed.origin);
            Vec4Lerp(frameBegin->quat, frameEnd->quat, lerp, frameWhenPlayed.quat);
            Vec4Normalize(frameWhenPlayed.quat);
            FX_SpawnElem(system, effect, remoteEffect, elemDefIndex, &frameWhenPlayed, msecNextSpawn, 0.0, spawnedCount);
            elemDef = &effect->def->elemDefsEA[elemDefIndex];
            ++spawnedCount;
            msecNextSpawn += intervalMsec;
        }
    }
}

int __cdecl FX_LimitStabilizeTimeForElemDef_Recurse(
                const FxElemDef *elemDef,
                bool needToSpawnSystem,
                int originalUpdateTime)
{
    int v5; // [esp+4h] [ebp-48h]
    int v6; // [esp+8h] [ebp-44h]
    int v7; // [esp+Ch] [ebp-40h]
    int v8; // [esp+10h] [ebp-3Ch]
    int v9; // [esp+14h] [ebp-38h]
    int v10; // [esp+18h] [ebp-34h]
    int v11; // [esp+1Ch] [ebp-30h]
    int v12; // [esp+20h] [ebp-2Ch]
    int v13; // [esp+24h] [ebp-28h]
    int selfStabilizeTime; // [esp+38h] [ebp-14h]
    FxElemVisuals *visArray; // [esp+3Ch] [ebp-10h]
    int maxStabilizeTime; // [esp+40h] [ebp-Ch]
    int visIndex; // [esp+48h] [ebp-4h]

    if ( !elemDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp", 281, 0, "%s", "elemDef") )
    {
        __debugbreak();
    }
    selfStabilizeTime = FX_LimitStabilizeTimeForElemDef_SelfOnly(elemDef, needToSpawnSystem);
    maxStabilizeTime = selfStabilizeTime;
    if ( selfStabilizeTime >= originalUpdateTime )
        return originalUpdateTime;
    if ( elemDef->elemType == 12 )
    {
        if ( elemDef->visualCount == 1 )
        {
            v13 = FX_LimitStabilizeTimeForEffectDef_Recurse(elemDef->visuals.instance.effectDef.handle, originalUpdateTime);
            if ( selfStabilizeTime < v13 )
                v8 = v13;
            else
                v8 = selfStabilizeTime;
            maxStabilizeTime = v8;
            if ( v8 >= originalUpdateTime )
                return originalUpdateTime;
        }
        else if ( elemDef->visualCount )
        {
            visArray = elemDef->visuals.array;
            for ( visIndex = 0; visIndex < elemDef->visualCount; ++visIndex )
            {
                v12 = FX_LimitStabilizeTimeForEffectDef_Recurse(visArray[visIndex].effectDef.handle, originalUpdateTime);
                if ( maxStabilizeTime < v12 )
                    v7 = v12;
                else
                    v7 = maxStabilizeTime;
                maxStabilizeTime = v7;
                if ( v7 >= originalUpdateTime )
                    return originalUpdateTime;
            }
        }
    }
    if ( elemDef->effectOnDeath.handle )
    {
        v11 = selfStabilizeTime
                + FX_LimitStabilizeTimeForEffectDef_Recurse(elemDef->effectOnDeath.handle, originalUpdateTime);
        v6 = maxStabilizeTime < v11 ? v11 : maxStabilizeTime;
        maxStabilizeTime = v6;
        if ( v6 >= originalUpdateTime )
            return originalUpdateTime;
    }
    if ( elemDef->effectOnImpact.handle )
    {
        v10 = selfStabilizeTime
                + FX_LimitStabilizeTimeForEffectDef_Recurse(elemDef->effectOnImpact.handle, originalUpdateTime);
        v5 = maxStabilizeTime < v10 ? v10 : maxStabilizeTime;
        maxStabilizeTime = v5;
        if ( v5 >= originalUpdateTime )
            return originalUpdateTime;
    }
    if ( elemDef->effectEmitted.handle )
    {
        v9 = selfStabilizeTime
             + FX_LimitStabilizeTimeForEffectDef_Recurse(elemDef->effectEmitted.handle, originalUpdateTime);
        if ( maxStabilizeTime < v9 )
            return v9;
        else
            return maxStabilizeTime;
    }
    return maxStabilizeTime;
}

int __cdecl FX_LimitStabilizeTimeForElemDef_SelfOnly(const FxElemDef *elemDef, bool needToSpawnSystem)
{
    int result; // [esp+4h] [ebp-4h]

    if ( elemDef->elemType == 5 )
        return 0x7FFFFFFF;
    result = elemDef->spawnDelayMsec.amplitude
                 + elemDef->spawnDelayMsec.base
                 + elemDef->lifeSpanMsec.amplitude
                 + elemDef->lifeSpanMsec.base;
    if ( needToSpawnSystem && elemDef->spawn.looping.count > 0 )
    {
        if ( elemDef->spawn.looping.count == 0x7FFFFFFF )
            return 0x7FFFFFFF;
        result += elemDef->spawn.looping.intervalMsec * (elemDef->spawn.looping.count - 1);
    }
    return result;
}

int __cdecl FX_LimitStabilizeTimeForEffectDef_Recurse(const FxEffectDef *remoteEffectDef, int originalUpdateTime)
{
    int v3; // [esp+0h] [ebp-28h]
    int v4; // [esp+4h] [ebp-24h]
    int elemIter; // [esp+10h] [ebp-18h]
    const FxElemDef *elemDefs; // [esp+18h] [ebp-10h]
    int maxStabilizeTime; // [esp+20h] [ebp-8h]
    int elemCount; // [esp+24h] [ebp-4h]

    if ( !remoteEffectDef
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
                    243,
                    0,
                    "%s",
                    "remoteEffectDef") )
    {
        __debugbreak();
    }
    if ( !remoteEffectDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp", 247, 0, "%s", "effectDef") )
    {
        __debugbreak();
    }
    elemCount = remoteEffectDef->elemDefCountEmission
                        + remoteEffectDef->elemDefCountOneShot
                        + remoteEffectDef->elemDefCountLooping;
    maxStabilizeTime = 0;
    if ( elemCount )
    {
        elemDefs = remoteEffectDef->elemDefsEA;
        for ( elemIter = 0; elemIter != elemCount; ++elemIter )
        {
            v4 = FX_LimitStabilizeTimeForElemDef_Recurse(&elemDefs[elemIter], 1, originalUpdateTime);
            if ( maxStabilizeTime < v4 )
                v3 = v4;
            else
                v3 = maxStabilizeTime;
            maxStabilizeTime = v3;
            if ( v3 >= originalUpdateTime )
                return originalUpdateTime;
        }
    }
    return maxStabilizeTime;
}

void __cdecl FX_BeginLooping(
                FxSystem *system,
                FxEffect *effect,
                FxEffectContainer *remoteEffect,
                int elemDefFirst,
                int elemDefCount,
                FxSpatialFrame *frameWhenPlayed,
                const FxSpatialFrame *frameNow,
                int msecWhenPlayed,
                int msecNow)
{
    const FxElemDef *elemDef; // [esp+74h] [ebp-18h]
    unsigned __int16 trailHandle; // [esp+78h] [ebp-14h]
    FxPool<FxTrail,FxTrail> *trail; // [esp+7Ch] [ebp-10h]
    int elemDefStop; // [esp+84h] [ebp-8h]
    int elemDefIndex; // [esp+88h] [ebp-4h]
    int elemDefIndexa; // [esp+88h] [ebp-4h]

    if ( !effect
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp", 543, 0, "%s", "effect") )
    {
        __debugbreak();
    }
    if ( !effect->def
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp", 546, 0, "%s", "effectDef") )
    {
        __debugbreak();
    }
    elemDefStop = elemDefCount + elemDefFirst;
    for ( elemDefIndex = elemDefFirst; elemDefIndex != elemDefStop; ++elemDefIndex )
    {
        if ( effect->def->elemDefsEA[elemDefIndex].elemType != 5 )
            FX_SpawnElem(system, effect, remoteEffect, elemDefIndex, frameWhenPlayed, msecWhenPlayed, 0.0, 0);
    }
    for ( trailHandle = effect->firstTrailHandle; trailHandle != 0xFFFF; trailHandle = trail->item.nextTrailHandle )
    {
        trail = FX_TrailFromHandle(system, trailHandle);
        elemDefIndexa = trail->item.defIndex;
        if ( elemDefIndexa >= elemDefFirst && elemDefIndexa < elemDefStop )
        {
            elemDef = &effect->def->elemDefsEA[elemDefIndexa];
            if ( elemDef->elemType != 5
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
                            568,
                            0,
                            "%s\n\t(elemDef->elemType) = %i",
                            "(elemDef->elemType == FX_ELEM_TYPE_TRAIL)",
                            elemDef->elemType) )
            {
                __debugbreak();
            }
            FX_SpawnTrailElem_NoCull(
                system,
                effect,
                &remoteEffect->effect,
                (FxTrail *)trail,
                frameWhenPlayed,
                msecWhenPlayed,
                0.0);
            if ( msecNow <= msecWhenPlayed )
            {
                FX_SpawnTrailElem_NoCull(
                    system,
                    effect,
                    &remoteEffect->effect,
                    (FxTrail *)trail,
                    frameWhenPlayed,
                    msecWhenPlayed,
                    0.0);
            }
            else
            {
                FX_SpawnTrailLoopingElems(
                    system,
                    effect,
                    &remoteEffect->effect,
                    (FxTrail *)trail,
                    frameWhenPlayed,
                    frameNow,
                    msecWhenPlayed,
                    msecWhenPlayed,
                    msecNow,
                    0.0,
                    effect->distanceTraveled);
                FX_SpawnTrailElem_NoCull(system, effect, &remoteEffect->effect, (FxTrail *)trail, frameNow, msecNow, 0.0);
            }
        }
    }
}

void __cdecl FX_SpawnTrailLoopingElems(
                FxSystem *system,
                FxEffect *effect,
                FxEffect *remoteEffect,
                FxTrail *trail,
                const FxSpatialFrame *frameBegin,
                const FxSpatialFrame *frameEnd,
                int msecWhenPlayed,
                int msecUpdateBegin,
                int msecUpdateEnd,
                float distanceTravelledBegin,
                float distanceTravelledEnd)
{
    float v11; // [esp+18h] [ebp-5Ch]
    float v12; // [esp+24h] [ebp-50h]
    float lerp; // [esp+30h] [ebp-44h]
    int msecSpawn; // [esp+34h] [ebp-40h]
    FxSpatialFrame frameWhenPlayed; // [esp+38h] [ebp-3Ch] BYREF
    float distSpawn; // [esp+54h] [ebp-20h]
    float normalizedTotalDistance; // [esp+58h] [ebp-1Ch]
    const FxEffectDef *effectDef; // [esp+5Ch] [ebp-18h]
    const FxElemDef *elemDef; // [esp+60h] [ebp-14h]
    float normalizedDistanceTraversed; // [esp+64h] [ebp-10h]
    float normalizedDistanceRemaining; // [esp+68h] [ebp-Ch]
    float normalizedDistanceBeforeSpawn; // [esp+6Ch] [ebp-8h]
    int splitDist; // [esp+70h] [ebp-4h]

    if ( !effect
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp", 136, 0, "%s", "effect") )
    {
        __debugbreak();
    }
    effectDef = effect->def;
    if ( !effectDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp", 139, 0, "%s", "effectDef") )
    {
        __debugbreak();
    }
    if ( trail->defIndex >= (unsigned int)(effectDef->elemDefCountEmission
                                                                             + effectDef->elemDefCountOneShot
                                                                             + effectDef->elemDefCountLooping)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
                    141,
                    0,
                    "trail->defIndex doesn't index effectDef->elemDefCountLooping + effectDef->elemDefCountOneShot + effectDef->ele"
                    "mDefCountEmission\n"
                    "\t%i not in [0, %i)",
                    trail->defIndex,
                    effectDef->elemDefCountEmission + effectDef->elemDefCountOneShot + effectDef->elemDefCountLooping) )
    {
        __debugbreak();
    }
    if ( trail->defIndex >= effectDef->elemDefCountLooping
        && trail->defIndex < effectDef->elemDefCountOneShot + effectDef->elemDefCountLooping
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
                    142,
                    0,
                    "%s",
                    "trail->defIndex < effectDef->elemDefCountLooping || trail->defIndex >= effectDef->elemDefCountLooping + effect"
                    "Def->elemDefCountOneShot") )
    {
        __debugbreak();
    }
    if ( (msecWhenPlayed > msecUpdateBegin || msecUpdateBegin > msecUpdateEnd)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
                    143,
                    0,
                    "msecUpdateBegin not in [msecWhenPlayed, msecUpdateEnd]\n\t%g not in [%g, %g]",
                    (float)msecUpdateBegin,
                    (float)msecWhenPlayed,
                    (float)msecUpdateEnd) )
    {
        __debugbreak();
    }
    elemDef = &effect->def->elemDefsEA[trail->defIndex];
    if ( elemDef->elemType != 5
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
                    147,
                    0,
                    "%s\n\t(elemDef->elemType) = %i",
                    "(elemDef->elemType == FX_ELEM_TYPE_TRAIL)",
                    elemDef->elemType) )
    {
        __debugbreak();
    }
    if ( !elemDef->localTrailDef
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
                    148,
                    0,
                    "%s",
                    "elemDef->trailDef") )
    {
        __debugbreak();
    }
    splitDist = elemDef->localTrailDef->splitDist;
    normalizedTotalDistance = (float)(distanceTravelledEnd - distanceTravelledBegin) / (float)splitDist;
    v12 = distanceTravelledBegin / (float)splitDist;
    v11 = floor(v12);
    normalizedDistanceBeforeSpawn = 1.0 - (v12 - v11);
    normalizedDistanceTraversed = 0.0f;
    normalizedDistanceRemaining = normalizedTotalDistance;
    while ( normalizedDistanceRemaining > normalizedDistanceBeforeSpawn )
    {
        normalizedDistanceTraversed = normalizedDistanceTraversed + normalizedDistanceBeforeSpawn;
        lerp = normalizedDistanceTraversed / normalizedTotalDistance;
        msecSpawn = msecUpdateBegin
                            + (int)(float)((float)(msecUpdateEnd - msecUpdateBegin)
                                                     * (float)(normalizedDistanceTraversed / normalizedTotalDistance));
        distSpawn = (float)((float)(distanceTravelledEnd - distanceTravelledBegin)
                                            * (float)(normalizedDistanceTraversed / normalizedTotalDistance))
                            + distanceTravelledBegin;
        Vec3Lerp(
            frameBegin->origin,
            frameEnd->origin,
            normalizedDistanceTraversed / normalizedTotalDistance,
            frameWhenPlayed.origin);
        Vec4Lerp(frameBegin->quat, frameEnd->quat, lerp, frameWhenPlayed.quat);
        Vec4Normalize(frameWhenPlayed.quat);
        FX_SpawnTrailElem_Cull(system, effect, remoteEffect, trail, &frameWhenPlayed, msecSpawn, distSpawn);
        normalizedDistanceRemaining = normalizedDistanceRemaining - normalizedDistanceBeforeSpawn;
        normalizedDistanceBeforeSpawn = 1.0f;
    }
}

void __cdecl FX_TriggerOneShot(
                FxSystem *system,
                FxEffect *effect,
                FxEffectContainer *remoteEffect,
                int elemDefFirst,
                int elemDefCount,
                FxSpatialFrame *frameWhenPlayed,
                int msecWhenPlayed)
{
    const FxEffectDef *effectDef; // [esp+18h] [ebp-Ch]
    int elemDefIndex; // [esp+20h] [ebp-4h]

    if ( !effect
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp", 594, 0, "%s", "effect") )
    {
        __debugbreak();
    }
    effectDef = effect->def;
    if ( !effect->def
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp", 597, 0, "%s", "effectDef") )
    {
        __debugbreak();
    }
    if ( elemDefCount
        && (elemDefFirst < 0
         || elemDefFirst >= effectDef->elemDefCountEmission
                                            + effectDef->elemDefCountOneShot
                                            + effectDef->elemDefCountLooping)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
                    599,
                    0,
                    "%s",
                    "elemDefCount == 0 || (elemDefFirst >= 0 && elemDefFirst < effectDef->elemDefCountLooping + effectDef->elemDefC"
                    "ountOneShot + effectDef->elemDefCountEmission)") )
    {
        __debugbreak();
    }
    if ( (elemDefCount < 0
         || elemDefCount + elemDefFirst > effectDef->elemDefCountEmission
                                                                        + effectDef->elemDefCountOneShot
                                                                        + effectDef->elemDefCountLooping)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
                    600,
                    0,
                    "%s",
                    "elemDefCount >= 0 && elemDefFirst + elemDefCount <= effectDef->elemDefCountLooping + effectDef->elemDefCountOn"
                    "eShot + effectDef->elemDefCountEmission") )
    {
        __debugbreak();
    }
    for ( elemDefIndex = elemDefFirst; elemDefIndex != elemDefCount + elemDefFirst; ++elemDefIndex )
        FX_SpawnOneShotElems(system, effect, remoteEffect, elemDefIndex, frameWhenPlayed, msecWhenPlayed);
}

void __cdecl FX_SpawnOneShotElems(
                FxSystem *system,
                FxEffect *effect,
                FxEffectContainer *remoteEffect,
                int elemDefIndex,
                FxSpatialFrame *frameWhenPlayed,
                int msecWhenPlayed)
{
    const FxEffectDef *effectDef; // [esp+10h] [ebp-10h]
    const FxElemDef *elemDef; // [esp+14h] [ebp-Ch]
    int spawnCount; // [esp+18h] [ebp-8h]
    int spawnIndex; // [esp+1Ch] [ebp-4h]

    if ( !effect
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp", 449, 0, "%s", "effect") )
    {
        __debugbreak();
    }
    effectDef = effect->def;
    if ( !effect->def
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp", 452, 0, "%s", "effectDef") )
    {
        __debugbreak();
    }
    if ( effectDef )
    {
        elemDef = &effect->def->elemDefsEA[elemDefIndex];
        if ( elemDef->elemType != 5 )
        {
            spawnCount = elemDef->spawn.looping.intervalMsec;
            if ( elemDef->spawn.looping.count )
                spawnCount += ((elemDef->spawn.looping.count + 1) * LOWORD(fx_randomTable[effect->randomSeed + 19])) >> 16;
            for ( spawnIndex = 0; spawnIndex < spawnCount; ++spawnIndex )
                FX_SpawnElem(system, effect, remoteEffect, elemDefIndex, frameWhenPlayed, msecWhenPlayed, 0.0, spawnIndex);
        }
    }
}

void __cdecl FX_StartNewEffect(FxSystem *system, FxEffect *effect, FxEffectContainer *remoteEffect)
{
    const FxEffectDef *def; // [esp+0h] [ebp-4h]

    def = effect->def;
    if ( !effect->def
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp", 614, 0, "%s", "def") )
    {
        __debugbreak();
    }
    FX_BeginLooping(
        system,
        effect,
        remoteEffect,
        0,
        def->elemDefCountLooping,
        &effect->frameAtSpawn,
        &effect->frameNow,
        effect->msecBegin,
        system->msecNow);
    FX_TriggerOneShot(
        system,
        effect,
        remoteEffect,
        def->elemDefCountLooping,
        def->elemDefCountOneShot,
        &effect->frameAtSpawn,
        effect->msecBegin);
    FX_SortNewElemsInEffect(system, effect);
}

void __cdecl FX_PlayElementSpawnSound(int localClientNum, const FxElemDef *elemDef, const float *origin)
{
    SndEntHandle entHandle; // [esp+14h] [ebp-4h]

    if ( elemDef->spawnSound.spawnSound )
    {
        entHandle.field = CG_SndEntHandle(localClientNum, 1022).field;
        SND_Play((char *)elemDef->spawnSound.spawnSound, 0, 1.0, entHandle, origin, 0, 0);
    }
}

bool __cdecl FX_GetBoltTemporalBits(int localClientNum, unsigned int dobjHandle)
{
    return dobjHandle < 0x600 && (CG_GetEntity(localClientNum, dobjHandle)->nextState.lerp.eFlags & 2) != 0;
}

char __cdecl FX_GetBoneOrientation(int localClientNum, unsigned int dobjHandle, int boneIndex, orientation_t *orient)
{
    DObj *remoteObj; // [esp+64h] [ebp-Ch]
    const cpose_t *pose; // [esp+68h] [ebp-8h]

    if ( dobjHandle >= 0x601
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
                    1617,
                    0,
                    "dobjHandle doesn't index CLIENT_DOBJ_HANDLE_MAX\n\t%i not in [0, %i)",
                    dobjHandle,
                    1537) )
    {
        __debugbreak();
    }
    if ( !orient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp", 1618, 0, "%s", "orient") )
    {
        __debugbreak();
    }
    if ( !FX_GetBoneOrientation_IsDObjEntityValid(localClientNum, dobjHandle) )
        return 0;
    if ( boneIndex < 0 || boneIndex == 511 )
    {
        CG_GetDObjOrientation(localClientNum, dobjHandle, orient->axis, orient->origin);
        if ( ((LODWORD(orient->axis[0][0]) & 0x7F800000) == 0x7F800000
             || (LODWORD(orient->axis[0][1]) & 0x7F800000) == 0x7F800000
             || (LODWORD(orient->axis[0][2]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
                        1629,
                        0,
                        "%s",
                        "!IS_NAN((orient->axis[0])[0]) && !IS_NAN((orient->axis[0])[1]) && !IS_NAN((orient->axis[0])[2])") )
        {
            __debugbreak();
        }
        if ( ((LODWORD(orient->axis[1][0]) & 0x7F800000) == 0x7F800000
             || (LODWORD(orient->axis[1][1]) & 0x7F800000) == 0x7F800000
             || (LODWORD(orient->axis[1][2]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
                        1630,
                        0,
                        "%s",
                        "!IS_NAN((orient->axis[1])[0]) && !IS_NAN((orient->axis[1])[1]) && !IS_NAN((orient->axis[1])[2])") )
        {
            __debugbreak();
        }
        if ( ((LODWORD(orient->axis[2][0]) & 0x7F800000) == 0x7F800000
             || (LODWORD(orient->axis[2][1]) & 0x7F800000) == 0x7F800000
             || (LODWORD(orient->axis[2][2]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
                        1631,
                        0,
                        "%s",
                        "!IS_NAN((orient->axis[2])[0]) && !IS_NAN((orient->axis[2])[1]) && !IS_NAN((orient->axis[2])[2])") )
        {
            __debugbreak();
        }
        if ( ((LODWORD(orient->origin[0]) & 0x7F800000) == 0x7F800000
             || (LODWORD(orient->origin[1]) & 0x7F800000) == 0x7F800000
             || (LODWORD(orient->origin[2]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
                        1632,
                        0,
                        "%s",
                        "!IS_NAN((orient->origin)[0]) && !IS_NAN((orient->origin)[1]) && !IS_NAN((orient->origin)[2])") )
        {
            __debugbreak();
        }
        return 1;
    }
    else
    {
        remoteObj = Com_GetClientDObj(dobjHandle, localClientNum);
        if ( remoteObj )
        {
            if ( boneIndex < DObjNumBones(remoteObj) )
            {
                pose = CG_GetPose(localClientNum, dobjHandle);
                if ( CG_DObjGetWorldBoneMatrix(pose, remoteObj, boneIndex, orient->axis, orient->origin) )
                {
                    if ( ((LODWORD(orient->origin[0]) & 0x7F800000) == 0x7F800000
                         || (LODWORD(orient->origin[1]) & 0x7F800000) == 0x7F800000
                         || (LODWORD(orient->origin[2]) & 0x7F800000) == 0x7F800000)
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
                                    1666,
                                    0,
                                    "%s",
                                    "!IS_NAN((orient->origin)[0]) && !IS_NAN((orient->origin)[1]) && !IS_NAN((orient->origin)[2])") )
                    {
                        __debugbreak();
                    }
                    if ( ((LODWORD(orient->axis[0][0]) & 0x7F800000) == 0x7F800000
                         || (LODWORD(orient->axis[0][1]) & 0x7F800000) == 0x7F800000
                         || (LODWORD(orient->axis[0][2]) & 0x7F800000) == 0x7F800000)
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
                                    1667,
                                    0,
                                    "%s",
                                    "!IS_NAN((orient->axis[0])[0]) && !IS_NAN((orient->axis[0])[1]) && !IS_NAN((orient->axis[0])[2])") )
                    {
                        __debugbreak();
                    }
                    if ( ((LODWORD(orient->axis[1][0]) & 0x7F800000) == 0x7F800000
                         || (LODWORD(orient->axis[1][1]) & 0x7F800000) == 0x7F800000
                         || (LODWORD(orient->axis[1][2]) & 0x7F800000) == 0x7F800000)
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
                                    1668,
                                    0,
                                    "%s",
                                    "!IS_NAN((orient->axis[1])[0]) && !IS_NAN((orient->axis[1])[1]) && !IS_NAN((orient->axis[1])[2])") )
                    {
                        __debugbreak();
                    }
                    if ( ((LODWORD(orient->axis[2][0]) & 0x7F800000) == 0x7F800000
                         || (LODWORD(orient->axis[2][1]) & 0x7F800000) == 0x7F800000
                         || (LODWORD(orient->axis[2][2]) & 0x7F800000) == 0x7F800000)
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
                                    1669,
                                    0,
                                    "%s",
                                    "!IS_NAN((orient->axis[2])[0]) && !IS_NAN((orient->axis[2])[1]) && !IS_NAN((orient->axis[2])[2])") )
                    {
                        __debugbreak();
                    }
                    return 1;
                }
                else
                {
                    return 0;
                }
            }
            else
            {
                return 0;
            }
        }
        else
        {
            return 0;
        }
    }
}

bool __cdecl FX_GetBoneOrientation_IsDObjEntityValid(int localClientNum, int dobjHandle)
{
    const centity_s *cent; // [esp+8h] [ebp-8h]

    if ( dobjHandle >= 1536 )
        return 1;
    cent = CG_GetEntity(localClientNum, dobjHandle);
    if ( ((*((unsigned int *)cent + 201) >> 1) & 1) == 0 )
        return 0;
    return (cent->nextState.lerp.eFlags & 2) == 0 || (cent->nextState.lerp.eFlags & 0x20) == 0;
}

void __cdecl FX_UpdateEffectPartial(
                FxSystem *system,
                FxEffect *effect,
                FxEffectContainer *remoteEffect,
                int msecUpdateBegin,
                int msecUpdateEnd,
                float distanceTravelledBegin,
                float distanceTravelledEnd,
                unsigned __int16 *elemHandleStart,
                unsigned __int16 *elemHandleStop,
                volatile unsigned __int16 *trailElemStart,
                volatile unsigned __int16 *trailElemStop)
{
    const FxEffectDef *def; // [esp+14h] [ebp-20h]
    volatile unsigned __int16 stopHandle; // [esp+18h] [ebp-1Ch]
    unsigned __int16 trailHandle; // [esp+1Ch] [ebp-18h]
    FxPool<FxTrail,FxTrail> *trail; // [esp+20h] [ebp-14h]
    unsigned int trailIter; // [esp+24h] [ebp-10h]
    volatile unsigned __int16 startHandle; // [esp+2Ch] [ebp-8h]
    unsigned int elemClass; // [esp+30h] [ebp-4h]

    startHandle = -1;
    stopHandle = -1;
    if ( effect->msecLastUpdate > msecUpdateEnd
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
                    1900,
                    0,
                    "effect->msecLastUpdate <= msecUpdateEnd\n\t%g, %g",
                    (float)effect->msecLastUpdate,
                    (float)msecUpdateEnd) )
    {
        __debugbreak();
    }
    if ( (remoteEffect->atomics.status & 0x10000) != 0 )
    {
        def = effect->def;
        FX_ProcessLooping(
            system,
            effect,
            remoteEffect,
            0,
            effect->def->elemDefCountLooping,
            &effect->framePrev,
            &effect->frameNow,
            effect->msecBegin,
            msecUpdateBegin,
            msecUpdateEnd,
            distanceTravelledBegin,
            distanceTravelledEnd);
        if ( msecUpdateEnd - effect->msecBegin > def->msecLoopingLife )
            FX_StopEffect(system, remoteEffect);
    }
    for ( elemClass = 0; elemClass < 3; ++elemClass )
        FX_UpdateEffectPartialForClass(
            system,
            effect,
            remoteEffect,
            msecUpdateBegin,
            msecUpdateEnd,
            elemHandleStart[elemClass],
            elemHandleStop[elemClass],
            elemClass);
    trailIter = 0;
    for ( trailHandle = effect->firstTrailHandle; trailHandle != 0xFFFF; trailHandle = trail->item.nextTrailHandle )
    {
        trail = FX_TrailFromHandle(system, trailHandle);
        if ( trailElemStart )
            startHandle = trailElemStart[trailIter];
        if ( trailElemStop )
            stopHandle = trailElemStop[trailIter];
        FX_UpdateEffectPartialTrail(
            system,
            effect,
            remoteEffect,
            (FxTrail *)trail,
            msecUpdateBegin,
            msecUpdateEnd,
            distanceTravelledBegin,
            distanceTravelledEnd,
            startHandle,
            stopHandle,
            &effect->frameNow);
        ++trailIter;
    }
    effect->msecLastUpdate = msecUpdateEnd;
}

void __cdecl FX_ProcessLooping(
                FxSystem *system,
                FxEffect *effect,
                FxEffectContainer *remoteEffect,
                int elemDefFirst,
                int elemDefCount,
                const FxSpatialFrame *frameBegin,
                const FxSpatialFrame *frameEnd,
                int msecWhenPlayed,
                int msecUpdateBegin,
                int msecUpdateEnd,
                float distanceTravelledBegin,
                float distanceTravelledEnd)
{
    unsigned __int16 trailHandle; // [esp+80h] [ebp-14h]
    FxPool<FxTrail,FxTrail> *trail; // [esp+84h] [ebp-10h]
    int elemDefEnd; // [esp+8Ch] [ebp-8h]
    int elemDefIndex; // [esp+90h] [ebp-4h]
    int elemDefIndexa; // [esp+90h] [ebp-4h]

    if ( !effect
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp", 484, 0, "%s", "effect") )
    {
        __debugbreak();
    }
    if ( !effect->def
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp", 487, 0, "%s", "effectDef") )
    {
        __debugbreak();
    }
    elemDefEnd = elemDefCount + elemDefFirst;
    for ( elemDefIndex = elemDefFirst; elemDefIndex != elemDefEnd; ++elemDefIndex )
        FX_SpawnLoopingElems(
            system,
            effect,
            remoteEffect,
            elemDefIndex,
            frameBegin,
            frameEnd,
            msecWhenPlayed,
            msecUpdateBegin,
            msecUpdateEnd);
    for ( trailHandle = effect->firstTrailHandle; trailHandle != 0xFFFF; trailHandle = trail->item.nextTrailHandle )
    {
        trail = FX_TrailFromHandle(system, trailHandle);
        elemDefIndexa = trail->item.defIndex;
        if ( elemDefIndexa >= elemDefFirst && elemDefIndexa < elemDefEnd )
            FX_SpawnTrailLoopingElems(
                system,
                effect,
                &remoteEffect->effect,
                (FxTrail *)trail,
                frameBegin,
                frameEnd,
                msecWhenPlayed,
                msecUpdateBegin,
                msecUpdateEnd,
                distanceTravelledBegin,
                distanceTravelledEnd);
    }
}

void __cdecl FX_UpdateEffectPartialForClass(
                FxSystem *system,
                FxEffect *effect,
                FxEffectContainer *remoteEffect,
                int msecUpdateBegin,
                int msecUpdateEnd,
                unsigned __int16 elemHandleStart,
                unsigned __int16 elemHandleStop,
                unsigned int elemClass)
{
    const FxIntRange *p_lifeSpanMsec; // [esp+4h] [ebp-2Ch]
    unsigned __int16 elemHandle; // [esp+14h] [ebp-1Ch]
    FxUpdateResult updateResult; // [esp+1Ch] [ebp-14h]
    unsigned __int16 elemHandleNext; // [esp+20h] [ebp-10h]
    FxPool<FxElem,FxElemContainer> *elem; // [esp+24h] [ebp-Ch]
    FxPool<FxElem,FxElemContainer> *elema; // [esp+24h] [ebp-Ch]
    unsigned int passCount; // [esp+28h] [ebp-8h]
    unsigned __int16 elemHandleFirstExisting; // [esp+2Ch] [ebp-4h]

    if ( effect->msecLastUpdate <= msecUpdateEnd )
    {
        elemHandleFirstExisting = effect->firstElemHandle[elemClass];
        passCount = 1;
        do
        {
            for ( elemHandle = elemHandleStart; elemHandle != elemHandleStop; elemHandle = elemHandleNext )
            {
                if ( elemHandle == 0xFFFF )
                {
                    Com_Printf(0, "---- EFFECT ABOUT TO ASSERT ----\n");
                    Com_Printf(0, "effect '%s' spawned at %i pass %i\n", effect->def->name, effect->msecBegin, passCount);
                    Com_Printf(
                        0,
                        "looping from %i to %i, first existing is %i\n",
                        elemHandleStart,
                        elemHandleStop,
                        elemHandleFirstExisting);
                    Com_Printf(
                        0,
                        "update period is from %d to %d (%d ms)\n",
                        msecUpdateBegin,
                        msecUpdateEnd,
                        msecUpdateEnd - msecUpdateBegin);
                    Com_Printf(0, "here's the active elem list:\n");
                    for ( elemHandle = effect->firstElemHandle[elemClass];
                                elemHandle != 0xFFFF;
                                elemHandle = elem->item.shared.nextElemHandleInEffect )
                    {
                        elem = FX_ElemFromHandle(system, elemHandle);
                        p_lifeSpanMsec = &effect->def->elemDefsEA[elem->item.elem.defIndex].lifeSpanMsec;
                        Com_Printf(
                            0,
                            "    elem %i def %i seq %i spawn %i die %i\n",
                            elemHandle,
                            elem->item.elem.defIndex,
                            elem->item.elem.sequence,
                            elem->item.elem.msecBegin,
                            p_lifeSpanMsec->base
                        + (((p_lifeSpanMsec->amplitude + 1)
                            * LOWORD(fx_randomTable[(296 * elem->item.elem.sequence
                                                                         + elem->item.elem.msecBegin
                                                                         + (unsigned int)effect->randomSeed)
                                                                        % 0x1DF
                                                                        + 17])) >> 16)
                        + elem->item.elem.msecBegin);
                    }
                    if ( !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
                                    1769,
                                    0,
                                    "Big bad effects assert.    Include assert log.") )
                        __debugbreak();
                }
                if ( elemHandle == 0xFFFF
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
                                1775,
                                0,
                                "this would have happened.\n") )
                {
                    __debugbreak();
                }
                elema = FX_ElemFromHandle(system, elemHandle);
                updateResult = FX_UpdateElement(system, effect, (FxElem *)elema, msecUpdateBegin, msecUpdateEnd);
                elemHandleNext = elema->item.shared.nextElemHandleInEffect;
                if ( elemHandle == elemHandleNext
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
                                1795,
                                0,
                                "%s\n\t(elemHandle) = %i",
                                "(elemHandle != elemHandleNext)",
                                elemHandle) )
                {
                    __debugbreak();
                }
                if ( updateResult == FX_UPDATE_REMOVE )
                {
                    FX_FreeElem(system, elemHandle, effect, remoteEffect, elemClass);
                    if ( elemHandleFirstExisting == elemHandle )
                        elemHandleFirstExisting = elemHandleNext;
                }
            }
            elemHandleStop = elemHandleFirstExisting;
            elemHandleFirstExisting = effect->firstElemHandle[elemClass];
            elemHandleStart = elemHandleFirstExisting;
            ++passCount;
        }
        while ( elemHandleFirstExisting != elemHandleStop );
    }
}

FxUpdateResult __cdecl FX_UpdateElement(
                FxSystem *system,
                FxEffect *effect,
                FxElem *elem,
                int msecUpdateBegin,
                int msecUpdateEnd)
{
    bool v6; // [esp+14h] [ebp-174h]
    int physObjId; // [esp+D8h] [ebp-B0h]
    FxUpdateElem update; // [esp+F0h] [ebp-98h] BYREF
    const FxElemDef *elemDef; // [esp+170h] [ebp-18h]
    FxUpdateResult updateResult; // [esp+174h] [ebp-14h] BYREF
    float elemOriginPrev[3];
    bool goToRest; // [esp+187h] [ebp-1h]

    if ( !elem
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp", 1519, 0, "%s", "elem") )
    {
        __debugbreak();
    }
    updateResult = FX_UPDATE_KEEP;
    if ( FX_UpdateElement_SetupUpdate(
                 effect,
                 msecUpdateBegin,
                 msecUpdateEnd,
                 elem->defIndex,
                 elem->atRestFraction,
                 elem->msecBegin,
                 elem->sequence,
                 elem->origin,
                 (float)elem->windInterp / 65535.0,
                 &update) )
    {
        FX_UpdateElement_TruncateToElemEnd(&update, &updateResult);
        elemDef = FX_GetUpdateElemDef(&update);
        if ( updateResult )
        {
            if ( !FX_UpdateElement_TruncateToElemBegin(&update, &updateResult) )
                return updateResult;
            //elemOriginPrev = elem->20;
            memcpy(elemOriginPrev, elem->origin, sizeof(elemOriginPrev));
            physObjId = elem->physObjId;
            update.elemBaseVel = elem->baseVel;
            update.physObjId = physObjId;
            update.onGround = 0;
            updateResult = (FxUpdateResult)FX_UpdateElementPosition(system, &update);
            FX_UpdateElement_HandleEmitting(system, elem, &update, elemOriginPrev, &updateResult);
            FX_UpdateElement_HandleAttachedEffect(system, elem, &update);
            if ( update.msecUpdateBegin == update.msecElemBegin )
                FX_PlayElementSpawnSound(system->localClientNum, elemDef, update.posWorld);
        }
        if ( updateResult )
        {
            v6 = update.atRestFraction == 255 && Vec3Compare(elem->origin, elemOriginPrev);
            goToRest = v6;
            if ( v6 && ((elemDef->flags & 0x100) == 0 || update.onGround) )
                elem->atRestFraction = (int)FX_GetAtRestFraction(&update, (float)update.msecUpdateEnd);
            else
                elem->atRestFraction = update.atRestFraction;
        }
        else if ( elemDef->effectOnDeath.handle )
        {
            FX_SpawnDeathEffect(system, &update);
        }
    }
    return updateResult;
}

const FxElemDef *__cdecl FX_GetUpdateElemDef(const FxUpdateElem *update)
{
    if ( !update->effect
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp", 97, 0, "%s", "update->effect") )
    {
        __debugbreak();
    }
    return &update->effect->def->elemDefsEA[update->elemIndex];
}

double __cdecl FX_GetAtRestFraction(const FxUpdateElem *update, float msec)
{
    return (float)ceil((float)((float)((float)((float)(msec - (float)update->msecElemBegin) * 255.0) / update->msecLifeSpan)
                                                     - 0.25));
}

int __cdecl FX_UpdateElementPosition(FxSystem *system, FxUpdateElem *update)
{
    const FxElemDef *elemDef; // [esp+4h] [ebp-4h]

    elemDef = FX_GetUpdateElemDef(update);
    if ( elemDef->elemType == 7 && (elemDef->flags & 0x8000000) != 0 )
        return 1;
    if ( (elemDef->flags & 0x100) != 0 )
        return FX_UpdateElementPosition_Colliding(system, update);
    if ( (elemDef->flags & 0x6000000) != 0 )
        return FX_UpdateElementPosition_NonColliding(update);
    return FX_UpdateElementPosition_Local(update);
}

int __cdecl FX_UpdateElementPosition_Colliding(FxSystem *system, FxUpdateElem *update)
{
    int msecUpdateBegin; // [esp+0h] [ebp-14h]
    float xyzWorldOld[3]; // [esp+4h] [ebp-10h] BYREF
    int msecUpdatePartial; // [esp+10h] [ebp-4h]

    if ( update->atRestFraction == 255 )
    {
        FX_OrientationPosToWorldPos(&update->orient, update->elemOrigin, xyzWorldOld);
        msecUpdateBegin = update->msecUpdateBegin;
        for ( msecUpdatePartial = msecUpdateBegin + 50; msecUpdatePartial < update->msecUpdateEnd; msecUpdatePartial += 50 )
        {
            if ( !FX_UpdateElementPosition_CollidingStep(system, update, msecUpdateBegin, msecUpdatePartial, xyzWorldOld) )
                return 0;
            msecUpdateBegin = msecUpdatePartial;
        }
        return FX_UpdateElementPosition_CollidingStep(system, update, msecUpdateBegin, update->msecUpdateEnd, xyzWorldOld);
    }
    else
    {
        FX_OrientationPosToWorldPos(&update->orient, update->elemOrigin, update->posWorld);
        return 1;
    }
}

int __cdecl FX_UpdateElementPosition_CollidingStep(
                FxSystem *system,
                FxUpdateElem *update,
                int msecUpdateBegin,
                int msecUpdateEnd,
                float *xyzWorldOld)
{
    const FxElemDef *elemDef; // [esp+2Ch] [ebp-44h]
    int traceMask; // [esp+30h] [ebp-40h]
    trace_t trace; // [esp+34h] [ebp-3Ch] BYREF

    traceMask = 2065;

    //col_context_t::col_context_t(&context, 2065);
    col_context_t context(traceMask);

    elemDef = FX_GetUpdateElemDef(update);
    if ( (elemDef->flags & 0x2000) != 0 )
        traceMask |= 0x400u;
    traceMask |= 0x2000u;
    do
    {
        update->onGround = 0;
        FX_NextElementPosition(update, msecUpdateBegin, msecUpdateEnd);
        CM_BoxTrace(&trace, xyzWorldOld, update->posWorld, elemDef->collMins, elemDef->collMaxs, traceMask, &context);
        if ( !FX_TraceHitSomething(&trace) )
            break;
        if ( trace.normal.vec.v[2] > 0.69999999 )
            update->onGround = 1;
        msecUpdateBegin = FX_CollisionResponse(system, update, &trace, msecUpdateBegin, msecUpdateEnd, xyzWorldOld);
        elemDef = FX_GetUpdateElemDef(update);
        if ( (elemDef->flags & 0x200) != 0 )
            return 0;
    }
    while ( msecUpdateBegin != msecUpdateEnd );
    FX_OrientationPosFromWorldPos(&update->orient, update->posWorld, update->elemOrigin);
    *xyzWorldOld = update->posWorld[0];
    xyzWorldOld[1] = update->posWorld[1];
    xyzWorldOld[2] = update->posWorld[2];
    return 1;
}

void __cdecl FX_NextElementPosition(FxUpdateElem *update, int msecUpdateBegin, int msecUpdateEnd)
{
    char *v4; // eax
    float *elemBaseVel; // [esp+Ch] [ebp-28h]
    float *elemOrigin; // [esp+14h] [ebp-20h]
    float secDuration; // [esp+1Ch] [ebp-18h]
    const FxElemDef *elemDef; // [esp+20h] [ebp-14h]
    float posLocal[3]; // [esp+24h] [ebp-10h] BYREF
    float deltaVelFromGravity; // [esp+30h] [ebp-4h]

    if (msecUpdateEnd - msecUpdateBegin <= 0)
    {
        v4 = va("[%d, %d]", msecUpdateBegin, msecUpdateEnd);
        if (!Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
            924,
            0,
            "%s\n\t%s",
            "msecUpdateEnd - msecUpdateBegin > 0",
            v4))
            __debugbreak();
    }
    elemOrigin = update->elemOrigin;
    posLocal[0] = *elemOrigin;
    posLocal[1] = elemOrigin[1];
    posLocal[2] = elemOrigin[2];
    FX_NextElementPosition_NoExternalForces(update, msecUpdateBegin, msecUpdateEnd, posLocal, update->posWorld);
    secDuration = (float)(msecUpdateEnd - msecUpdateBegin) * 0.001;
    elemBaseVel = update->elemBaseVel;
    update->posWorld[0] = (float)(secDuration * *elemBaseVel) + update->posWorld[0];
    update->posWorld[1] = (float)(secDuration * elemBaseVel[1]) + update->posWorld[1];
    update->posWorld[2] = (float)(secDuration * elemBaseVel[2]) + update->posWorld[2];
    elemDef = FX_GetUpdateElemDef(update);
    deltaVelFromGravity = (float)((float)((float)(fx_randomTable[update->randomSeed + 15] * elemDef->gravity.amplitude)
        + elemDef->gravity.base)
        * 800.0)
        * secDuration;
    update->elemBaseVel[2] = update->elemBaseVel[2] - deltaVelFromGravity;
    update->posWorld[2] = update->posWorld[2] - (float)((float)(deltaVelFromGravity * secDuration) * 0.5);
}

void __cdecl FX_NextElementPosition_NoExternalForces(
                FxUpdateElem *update,
                int msecUpdateBegin,
                int msecUpdateEnd,
                float *posLocal,
                float *posWorld)
{
    const char *v5; // eax

    if ( msecUpdateEnd - msecUpdateBegin <= 0 )
    {
        v5 = va("[%d, %d]", msecUpdateBegin, msecUpdateEnd);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
                        869,
                        0,
                        "%s\n\t%s",
                        "msecUpdateEnd - msecUpdateBegin > 0",
                        v5) )
            __debugbreak();
    }
    if ( !posLocal
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp", 870, 0, "%s", "posLocal") )
    {
        __debugbreak();
    }
    FX_IntegrateVelocity(
        update,
        (float)(msecUpdateBegin - update->msecElemBegin) / update->msecLifeSpan,
        (float)(msecUpdateEnd - update->msecElemBegin) / update->msecLifeSpan,
        posLocal,
        posWorld);
}

void __cdecl FX_IntegrateVelocity(const FxUpdateElem *update, float t0, float t1, float *posLocal, float *posWorld)
{
    char *v6; // eax
    char *v7; // eax
    char *v8; // eax
    char *v9; // eax
    double v10; // [esp+18h] [ebp-80h]
    int v11; // [esp+20h] [ebp-78h]
    int v12; // [esp+24h] [ebp-74h]
    float integralScale; // [esp+64h] [ebp-34h]
    float startPoint; // [esp+68h] [ebp-30h]
    float endPoint; // [esp+6Ch] [ebp-2Ch]
    const FxElemDef *elemDef; // [esp+70h] [ebp-28h]
    float startLerp; // [esp+74h] [ebp-24h]
    int startIndex; // [esp+78h] [ebp-20h]
    const FxElemVelStateSample *samples; // [esp+7Ch] [ebp-1Ch]
    int intervalCount; // [esp+80h] [ebp-18h]
    int endIndex; // [esp+84h] [ebp-14h]
    float rangeLerp[3]; // [esp+88h] [ebp-10h] BYREF
    float endLerp; // [esp+94h] [ebp-4h]

    if (!update
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp", 824, 0, "%s", "update"))
    {
        __debugbreak();
    }
    elemDef = FX_GetUpdateElemDef(update);
    if (!elemDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp", 827, 0, "%s", "elemDef"))
    {
        __debugbreak();
    }
    if (!elemDef->velSamples
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
            828,
            0,
            "%s",
            "elemDef->velSamples"))
    {
        __debugbreak();
    }
    if (!elemDef->velIntervalCount
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
            829,
            0,
            "%s\n\t(elemDef->velIntervalCount) = %i",
            "(elemDef->velIntervalCount >= 1)",
            elemDef->velIntervalCount))
    {
        __debugbreak();
    }
    if (t0 < 0.0 || t1 <= t0 || t1 > 1.0)
    {
        v6 = va("%g, %g", t0, t1);
        if (!Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
            830,
            0,
            "%s\n\t%s",
            "0.0f <= t0 && t0 < t1 && t1 <= 1.0f",
            v6))
            __debugbreak();
    }
    rangeLerp[0] = fx_randomTable[update->randomSeed];
    rangeLerp[1] = fx_randomTable[update->randomSeed + 1];
    rangeLerp[2] = fx_randomTable[update->randomSeed + 2];
    integralScale = update->msecLifeSpan;
    samples = elemDef->velSamples;
    intervalCount = elemDef->velIntervalCount;
    if (intervalCount == 1)
    {
        FX_IntegrateVelocityInSegment(
            elemDef->flags,
            &update->orient,
            samples,
            t0,
            t1,
            rangeLerp,
            integralScale,
            posLocal,
            posWorld);
    }
    else
    {
        startPoint = (float)intervalCount * t0;
        *((float *)&v10 + 1) = floor(startPoint);
        startIndex = (int)*((float *)&v10 + 1);
        startLerp = startPoint - (float)startIndex;
        endPoint = (float)intervalCount * t1;
        *(float *)&v10 = ceil(endPoint);
        endIndex = (int)*(float *)&v10 - 1;
        endLerp = endPoint - (float)endIndex;

        iassert(startIndex <= endIndex);
        //if (startIndex > endIndex)
        //{
        //    v7 = va("%i > %i for %g to %g on %i intervals", startIndex, endIndex, t0, t1, intervalCount);
        //    if (!Assert_MyHandler(
        //        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
        //        853,
        //        1,
        //        "%s\n\t%s",
        //        "startIndex <= endIndex",
        //        v7))
        //        __debugbreak();
        //}

        iassert(startIndex >= 0 && startIndex < intervalCount);
        //if (startIndex < 0 || startIndex >= intervalCount)
        //{
        //    v8 = va("%i for %g on %i intervals", startIndex, t0, intervalCount);
        //    if (!Assert_MyHandler(
        //        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
        //        854,
        //        1,
        //        "%s\n\t(va( \"%i for %g on %i intervals\", startIndex, t0, intervalCount )) = %i",
        //        "(startIndex >= 0 && startIndex < intervalCount)",
        //        v8,
        //        v10,
        //        v11,
        //        v12))
        //        __debugbreak();
        //}

        iassert(endIndex >= 0 && endIndex < intervalCount);
        //if (endIndex < 0 || endIndex >= intervalCount)
        //{
        //    v9 = va("%i for %g on %i intervals", endIndex, t1, intervalCount);
        //    if (!Assert_MyHandler(
        //        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
        //        855,
        //        1,
        //        "%s\n\t(va( \"%i for %g on %i intervals\", endIndex, t1, intervalCount )) = %i",
        //        "(endIndex >= 0 && endIndex < intervalCount)",
        //        v9,
        //        v10,
        //        v11,
        //        v12))
        //        __debugbreak();
        //}
        if (startIndex == endIndex)
            FX_IntegrateVelocityInSegment(
                elemDef->flags,
                &update->orient,
                &samples[startIndex],
                startLerp,
                endLerp,
                rangeLerp,
                integralScale,
                posLocal,
                posWorld);
        else
            FX_IntegrateVelocityAcrossSegments(
                elemDef->flags,
                &update->orient,
                &samples[startIndex],
                &samples[endIndex],
                startLerp,
                endLerp,
                rangeLerp,
                integralScale,
                posLocal,
                posWorld);
    }
}

void __cdecl FX_IntegrateVelocityAcrossSegments(
                int elemDefFlags,
                const orientation_t *orient,
                const FxElemVelStateSample *velState0,
                const FxElemVelStateSample *velState1,
                float t0,
                float t1,
                const float *amplitudeScale,
                float integralScale,
                float *posLocal,
                float *posWorld)
{
    float w0[2]; // [esp+20h] [ebp-18h] BYREF
    const FxElemVelStateSample *localVelState0; // [esp+28h] [ebp-10h]
    float w1[2]; // [esp+2Ch] [ebp-Ch] BYREF
    float t0ScaledByIntegral; // [esp+34h] [ebp-4h]

    localVelState0 = velState0;
    if ( (t0 < 0.0 || t0 > 1.0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
                    737,
                    0,
                    "t0 not in [0.0f, 1.0f]\n\t%g not in [%g, %g]",
                    t0,
                    0.0,
                    1.0) )
    {
        __debugbreak();
    }
    if ( (t1 < 0.0 || t1 > 1.0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
                    738,
                    0,
                    "t1 not in [0.0f, 1.0f]\n\t%g not in [%g, %g]",
                    t1,
                    0.0,
                    1.0) )
    {
        __debugbreak();
    }
    t0ScaledByIntegral = (-(integralScale)) * t0;
    w0[1] = (float)(0.5 * t0) * t0ScaledByIntegral;
    w0[0] = t0ScaledByIntegral - w0[1];
    w1[1] = (float)(0.5 * t1) * (float)(t1 * integralScale);
    w1[0] = (float)(t1 * integralScale) - w1[1];
    if ((elemDefFlags & 0x1000000) != 0)
    {
        FX_IntegrateVelocityFromZeroInSegment(
            &velState1->local,
            &velState1[1].local,
            w1,
            amplitudeScale,
            integralScale,
            posLocal);
        FX_IntegrateVelocityFromZeroInSegment(
            &localVelState0->local,
            &localVelState0[1].local,
            w0,
            amplitudeScale,
            (-(integralScale)),
            posLocal);
    }
    FX_OrientationPosToWorldPos(orient, posLocal, posWorld);
    if ((elemDefFlags & 0x2000000) != 0)
    {
        FX_IntegrateVelocityFromZeroInSegment(
            &velState1->world,
            &velState1[1].world,
            w1,
            amplitudeScale,
            integralScale,
            posWorld);
        FX_IntegrateVelocityFromZeroInSegment(
            &localVelState0->world,
            &localVelState0[1].world,
            w0,
            amplitudeScale,
            (-(integralScale)),
            posWorld);
    }
}

void __cdecl FX_IntegrateVelocityFromZeroInSegment(
                const FxElemVelStateInFrame *statePrev,
                const FxElemVelStateInFrame *stateNext,
                float *weight,
                const float *amplitudeScale,
                float integralScale,
                float *pos)
{
    *pos = (float)((float)((float)(*amplitudeScale * statePrev->totalDelta.amplitude[0]) + statePrev->totalDelta.base[0])
                             * integralScale)
             + *pos;
    *pos = (float)((float)((float)(*amplitudeScale * statePrev->velocity.amplitude[0]) + statePrev->velocity.base[0])
                             * *weight)
             + *pos;
    *pos = (float)((float)((float)(*amplitudeScale * stateNext->velocity.amplitude[0]) + stateNext->velocity.base[0])
                             * weight[1])
             + *pos;
    pos[1] = (float)((float)((float)(amplitudeScale[1] * statePrev->totalDelta.amplitude[1])
                                                 + statePrev->totalDelta.base[1])
                                 * integralScale)
                 + pos[1];
    pos[1] = (float)((float)((float)(amplitudeScale[1] * statePrev->velocity.amplitude[1]) + statePrev->velocity.base[1])
                                 * *weight)
                 + pos[1];
    pos[1] = (float)((float)((float)(amplitudeScale[1] * stateNext->velocity.amplitude[1]) + stateNext->velocity.base[1])
                                 * weight[1])
                 + pos[1];
    pos[2] = (float)((float)((float)(amplitudeScale[2] * statePrev->totalDelta.amplitude[2])
                                                 + statePrev->totalDelta.base[2])
                                 * integralScale)
                 + pos[2];
    pos[2] = (float)((float)((float)(amplitudeScale[2] * statePrev->velocity.amplitude[2]) + statePrev->velocity.base[2])
                                 * *weight)
                 + pos[2];
    pos[2] = (float)((float)((float)(amplitudeScale[2] * stateNext->velocity.amplitude[2]) + stateNext->velocity.base[2])
                                 * weight[1])
                 + pos[2];
}

void __cdecl FX_IntegrateVelocityInSegment(
                int elemDefFlags,
                const orientation_t *orient,
                const FxElemVelStateSample *velState,
                float t0,
                float t1,
                const float *amplitudeScale,
                float integralScale,
                float *posLocal,
                float *posWorld)
{
    float weight[2]; // [esp+Ch] [ebp-8h] BYREF

    if ( (t0 < 0.0 || t0 > 1.0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
                    791,
                    0,
                    "%s\n\t(t0) = %g",
                    "(t0 >= 0.0f && t0 <= 1.0f)",
                    t0) )
    {
        __debugbreak();
    }
    if ( (t1 < 0.0 || t1 > 1.0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
                    792,
                    0,
                    "%s\n\t(t1) = %g",
                    "(t1 >= 0.0f && t1 <= 1.0f)",
                    t1) )
    {
        __debugbreak();
    }
    weight[1] = (float)(integralScale * 0.5) * (float)((float)(t1 * t1) - (float)(t0 * t0));
    weight[0] = (float)((float)(t1 - t0) * integralScale) - weight[1];
    if ((elemDefFlags & 0x1000000) != 0)
        FX_IntegrateVelocityInSegmentInFrame(
            &velState->local,
            &velState[1].local,
            weight,
            amplitudeScale,
            integralScale,
            posLocal);
    FX_OrientationPosToWorldPos(orient, posLocal, posWorld);
    if ((elemDefFlags & 0x2000000) != 0)
        FX_IntegrateVelocityInSegmentInFrame(
            &velState->world,
            &velState[1].world,
            weight,
            amplitudeScale,
            integralScale,
            posWorld);
}

void __cdecl FX_IntegrateVelocityInSegmentInFrame(
                const FxElemVelStateInFrame *statePrev,
                const FxElemVelStateInFrame *stateNext,
                const float *weight,
                const float *amplitudeScale,
                float integralScale,
                float *pos)
{
    *pos = (float)((float)((float)(*amplitudeScale * statePrev->velocity.amplitude[0]) + statePrev->velocity.base[0])
                             * *weight)
             + *pos;
    *pos = (float)((float)((float)(*amplitudeScale * stateNext->velocity.amplitude[0]) + stateNext->velocity.base[0])
                             * weight[1])
             + *pos;
    pos[1] = (float)((float)((float)(amplitudeScale[1] * statePrev->velocity.amplitude[1]) + statePrev->velocity.base[1])
                                 * *weight)
                 + pos[1];
    pos[1] = (float)((float)((float)(amplitudeScale[1] * stateNext->velocity.amplitude[1]) + stateNext->velocity.base[1])
                                 * weight[1])
                 + pos[1];
    pos[2] = (float)((float)((float)(amplitudeScale[2] * statePrev->velocity.amplitude[2]) + statePrev->velocity.base[2])
                                 * *weight)
                 + pos[2];
    pos[2] = (float)((float)((float)(amplitudeScale[2] * stateNext->velocity.amplitude[2]) + stateNext->velocity.base[2])
                                 * weight[1])
                 + pos[2];
}

bool __cdecl FX_TraceHitSomething(const trace_t *trace)
{
    if ( !trace
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp", 946, 0, "%s", "trace") )
    {
        __debugbreak();
    }
    return !trace->startsolid && !trace->allsolid && trace->fraction != 1.0;
}

int __cdecl FX_CollisionResponse(
                FxSystem *system,
                FxUpdateElem *update,
                const trace_t *trace,
                int msecUpdateBegin,
                int msecUpdateEnd,
                float *xyzWorldOld)
{
    char *v8; // eax
    float *elemBaseVel; // edx
    float v10; // [esp+18h] [ebp-9Ch]
    float *v11; // [esp+2Ch] [ebp-88h]
    const FxElemDef *elemDef; // [esp+78h] [ebp-3Ch]
    int msecOnImpact; // [esp+7Ch] [ebp-38h]
    float velDelta_4; // [esp+84h] [ebp-30h]
    float velDelta_8; // [esp+88h] [ebp-2Ch]
    float scaledPreImpactVelocity; // [esp+90h] [ebp-24h]
    float scaledPreImpactVelocity_4; // [esp+94h] [ebp-20h]
    float scaledPreImpactVelocity_8; // [esp+98h] [ebp-1Ch]
    float velocityAlongNormal; // [esp+9Ch] [ebp-18h]
    float preImpactVelocity[3]; // [esp+A0h] [ebp-14h] BYREF
    float reflectionFactor; // [esp+ACh] [ebp-8h]
    float overshotDeltaVelFromGravity; // [esp+B0h] [ebp-4h]

    elemDef = FX_GetUpdateElemDef(update);
    if (msecUpdateBegin >= msecUpdateEnd
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
            986,
            0,
            "%s",
            "msecUpdateBegin < msecUpdateEnd"))
    {
        __debugbreak();
    }
    Vec3Lerp(xyzWorldOld, update->posWorld, trace->fraction, update->posWorld);
    v10 = floor((float)((float)(msecUpdateEnd - msecUpdateBegin) * trace->fraction));
    msecOnImpact = msecUpdateBegin + (int)v10;
    if (msecOnImpact >= msecUpdateEnd
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
            990,
            0,
            "%s",
            "msecOnImpact < msecUpdateEnd"))
    {
        __debugbreak();
    }
    if ((elemDef->flags & 0x200) != 0 || msecOnImpact == update->msecElemEnd)
    {
        if (elemDef->effectOnImpact.handle)
            FX_SpawnImpactEffect(system, update, elemDef->effectOnImpact.handle, msecOnImpact, trace->normal.vec.v);
        return msecUpdateBegin + (int)v10;
    }
    else
    {
        overshotDeltaVelFromGravity = (float)((float)((float)(fx_randomTable[update->randomSeed + 15]
            * elemDef->gravity.amplitude)
            + elemDef->gravity.base)
            * 0.80000001)
            * (float)(msecUpdateEnd - msecOnImpact);
        update->elemBaseVel[2] = update->elemBaseVel[2] + overshotDeltaVelFromGravity;
        FX_GetVelocityAtTime(
            elemDef,
            update->randomSeed,
            update->msecLifeSpan,
            (float)(msecOnImpact - update->msecElemBegin),
            &update->orient,
            update->elemBaseVel,
            preImpactVelocity);
        if (((LODWORD(preImpactVelocity[0]) & 0x7F800000) == 0x7F800000
            || (LODWORD(preImpactVelocity[1]) & 0x7F800000) == 0x7F800000
            || (LODWORD(preImpactVelocity[2]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
                1006,
                0,
                "%s",
                "!IS_NAN((preImpactVelocity)[0]) && !IS_NAN((preImpactVelocity)[1]) && !IS_NAN((preImpactVelocity)[2])"))
        {
            __debugbreak();
        }
        if ((float)((float)((float)(preImpactVelocity[0] * preImpactVelocity[0])
            + (float)(preImpactVelocity[1] * preImpactVelocity[1]))
            + (float)(preImpactVelocity[2] * preImpactVelocity[2])) >= 1.0e12)
        {
            v8 = va("%g %g %g", preImpactVelocity[0], preImpactVelocity[1], preImpactVelocity[2]);
            if (!Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
                1007,
                0,
                "%s\n\t%s",
                "Vec3LengthSq( preImpactVelocity ) < 1.0e12f",
                v8))
                __debugbreak();
        }
        reflectionFactor = (float)(fx_randomTable[update->randomSeed + 16] * elemDef->reflectionFactor.amplitude)
            + elemDef->reflectionFactor.base;
        scaledPreImpactVelocity = reflectionFactor * preImpactVelocity[0];
        scaledPreImpactVelocity_4 = reflectionFactor * preImpactVelocity[1];
        scaledPreImpactVelocity_8 = reflectionFactor * preImpactVelocity[2];
        if (elemDef->effectOnImpact.handle
            && (float)((float)((float)(preImpactVelocity[0] * preImpactVelocity[0])
                + (float)(preImpactVelocity[1] * preImpactVelocity[1]))
                + (float)(preImpactVelocity[2] * preImpactVelocity[2])) > 1.0)
        {
            FX_SpawnImpactEffect(system, update, elemDef->effectOnImpact.handle, msecOnImpact, trace->normal.vec.v);
            FX_GetUpdateElemDef(update);
        }
        if (msecOnImpact == msecUpdateBegin
            && (++msecOnImpact,
                (float)((float)((float)(scaledPreImpactVelocity * scaledPreImpactVelocity)
                    + (float)(scaledPreImpactVelocity_4 * scaledPreImpactVelocity_4))
                    + (float)(scaledPreImpactVelocity_8 * scaledPreImpactVelocity_8)) <= 1.0)
            && trace->normal.vec.v[2] > 0.69999999)
        {
            update->atRestFraction = (int)FX_GetAtRestFraction(update, (float)msecOnImpact);
            if (FX_IsDemoPlaying()
                && (float)((float)(*update->elemBaseVel * *update->elemBaseVel)
                    + (float)(update->elemBaseVel[1] * update->elemBaseVel[1])) < 1.0)
            {
                elemBaseVel = update->elemBaseVel;
                *elemBaseVel =   0.0f;
                elemBaseVel[1] = 0.0f;
                elemBaseVel[2] = 0.0f;
            }
            return msecUpdateEnd;
        }
        else
        {
            velocityAlongNormal = (float)((float)(scaledPreImpactVelocity * trace->normal.vec.v[0])
                + (float)(scaledPreImpactVelocity_4 * trace->normal.vec.v[1]))
                + (float)(scaledPreImpactVelocity_8 * trace->normal.vec.v[2]);
            if ((LODWORD(velocityAlongNormal) & 0x7F800000) == 0x7F800000
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
                    1037,
                    0,
                    "%s",
                    "!IS_NAN(velocityAlongNormal)"))
            {
                __debugbreak();
            }
            if ((fabs(velocityAlongNormal)) >= 1000000.0
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
                    1038,
                    0,
                    "%s\n\t(velocityAlongNormal) = %g",
                    "(I_fabs( velocityAlongNormal ) < 1.0e6f)",
                    velocityAlongNormal))
            {
                __debugbreak();
            }
            velDelta_4 = (float)((float)((float)(-2.0 * velocityAlongNormal) * trace->normal.vec.v[1])
                + scaledPreImpactVelocity_4)
                - preImpactVelocity[1];
            velDelta_8 = (float)((float)((float)(-2.0 * velocityAlongNormal) * trace->normal.vec.v[2])
                + scaledPreImpactVelocity_8)
                - preImpactVelocity[2];
            v11 = update->elemBaseVel;
            *v11 = *v11
                + (float)((float)((float)((float)(-2.0 * velocityAlongNormal) * trace->normal.vec.v[0])
                    + scaledPreImpactVelocity)
                    - preImpactVelocity[0]);
            v11[1] = v11[1] + velDelta_4;
            v11[2] = v11[2] + velDelta_8;
            
            nanassertvec3(update->elemBaseVel);

            FX_OrientationPosFromWorldPos(&update->orient, update->posWorld, update->elemOrigin);
            *xyzWorldOld = update->posWorld[0];
            xyzWorldOld[1] = update->posWorld[1];
            xyzWorldOld[2] = update->posWorld[2];
            return msecOnImpact;
        }
    }
}

void __cdecl FX_SpawnImpactEffect(
                FxSystem *system,
                const FxUpdateElem *update,
                const FxEffectDef *impactEffect,
                int msecOnImpact,
                const float *impactNormal)
{
    FxEffectContainer *effect; // [esp+4h] [ebp-2Ch]
    unsigned int handle; // [esp+8h] [ebp-28h]
    float axis[3][3]; // [esp+Ch] [ebp-24h] BYREF

    *(_QWORD *)&axis[0][0] = *(_QWORD *)impactNormal;
    axis[0][2] = impactNormal[2];
    Vec3Basis_RightHanded(axis[0], axis[1], axis[2]);
    handle = FX_SpawnEffect(
                         system,
                         impactEffect,
                         msecOnImpact,
                         update->posWorld,
                         axis,
                         0x7FFu,
                         511,
                         255,
                         update->effect->owner,
                         0x3FFu,
                         &orIdentity);
    effect = UniqueHandleToEffect(system, handle);
    if ( effect )
        FX_DelRefToEffect(system, effect, 0);
}

int __cdecl FX_UpdateElementPosition_NonColliding(FxUpdateElem *update)
{
    FX_NextElementPosition(update, update->msecUpdateBegin, update->msecUpdateEnd);
    FX_OrientationPosFromWorldPos(&update->orient, update->posWorld, update->elemOrigin);
    return 1;
}

int __cdecl FX_UpdateElementPosition_Local(FxUpdateElem *update)
{
    const char *v1; // eax
    float *elemBaseVel; // [esp+1Ch] [ebp-4h]

    elemBaseVel = update->elemBaseVel;
    if ( *elemBaseVel != 0.0 || elemBaseVel[1] != 0.0 || elemBaseVel[2] != 0.0 )
    {
        v1 = va(
                     "effect %s def %i baseVel %g %g %g",
                     update->effect->def->name,
                     update->elemIndex,
                     *update->elemBaseVel,
                     update->elemBaseVel[1],
                     update->elemBaseVel[2]);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
                        1147,
                        0,
                        "%s\n\t%s",
                        "Vec3Compare( update->elemBaseVel, vec3_origin )",
                        v1) )
            __debugbreak();
    }
    FX_NextElementPosition_NoExternalForces(
        update,
        update->msecUpdateBegin,
        update->msecUpdateEnd,
        update->elemOrigin,
        update->posWorld);
    return 1;
}

void __cdecl FX_SpawnDeathEffect(FxSystem *system, FxUpdateElem *update)
{
    FxEffectContainer *effect; // [esp+Ch] [ebp-58h]
    unsigned int handle; // [esp+10h] [ebp-54h]
    FxSpatialFrame frame; // [esp+14h] [ebp-50h] BYREF
    const FxElemDef *elemDef; // [esp+30h] [ebp-34h]
    orientation_t orientPrev; // [esp+34h] [ebp-30h] BYREF

    elemDef = FX_GetUpdateElemDef(update);
    FX_GetOrientation(elemDef, &update->effect->frameAtSpawn, &update->effect->framePrev, update->randomSeed, &orientPrev);
    FX_OrientationPosToWorldPos(&orientPrev, update->elemOrigin, frame.origin);
    handle = FX_SpawnEffect(
                         system,
                         elemDef->effectOnDeath.handle,
                         update->msecUpdateBegin,
                         frame.origin,
                         orientPrev.axis,
                         0x7FFu,
                         511,
                         255,
                         update->effect->owner,
                         0x3FFu,
                         &orIdentity);
    effect = UniqueHandleToEffect(system, handle);
    if ( effect )
        FX_DelRefToEffect(system, effect, 0);
}

char __cdecl FX_UpdateElement_SetupUpdate(
                FxEffect *effect,
                int msecUpdateBegin,
                int msecUpdateEnd,
                int elemDefIndex,
                int elemAtRestFraction,
                int elemMsecBegin,
                int elemSequence,
                float *elemOrigin,
                float elemWindInterp,
                FxUpdateElem *update)
{
    int ElemLifeSpanMsec; // eax
    const FxElemDef *elemDef; // [esp+10h] [ebp-4h]

    memset(update, 0xD0u, sizeof(FxUpdateElem));
    update->effect = effect;
    update->msecUpdateBegin = msecUpdateBegin;
    update->msecUpdateEnd = msecUpdateEnd;
    update->msecElemBegin = elemMsecBegin;
    if ( update->msecUpdateBegin > update->msecUpdateEnd
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
                    1235,
                    0,
                    "update->msecUpdateBegin <= update->msecUpdateEnd\n\t%i, %i",
                    update->msecUpdateBegin,
                    update->msecUpdateEnd) )
    {
        __debugbreak();
    }
    if ( update->msecUpdateEnd < update->msecElemBegin )
        return 0;
    if ( elemDefIndex >= effect->def->elemDefCountEmission
                                         + effect->def->elemDefCountOneShot
                                         + effect->def->elemDefCountLooping )
        return 0;
    update->elemIndex = elemDefIndex;
    update->atRestFraction = elemAtRestFraction;
    update->randomSeed = (296 * elemSequence + elemMsecBegin + (unsigned int)effect->randomSeed) % 0x1DF;
    update->sequence = elemSequence;
    elemDef = FX_GetUpdateElemDef(update);
    ElemLifeSpanMsec = FX_GetElemLifeSpanMsec(update->randomSeed, elemWindInterp, elemDef);
    update->msecElemEnd = ElemLifeSpanMsec + update->msecElemBegin;
    update->msecLifeSpan = (float)ElemLifeSpanMsec;
    update->elemOrigin = elemOrigin;
    return 1;
}

void __cdecl FX_UpdateElement_TruncateToElemEnd(FxUpdateElem *update, FxUpdateResult *outUpdateResult)
{
    if ( update->msecUpdateEnd >= update->msecElemEnd )
    {
        if ( FX_GetUpdateElemDef(update)->effectEmitted.handle )
        {
            update->msecUpdateEnd = update->msecElemEnd;
            if ( update->msecUpdateBegin > update->msecUpdateEnd )
                update->msecUpdateBegin = update->msecUpdateEnd;
        }
        else
        {
            *outUpdateResult = FX_UPDATE_REMOVE;
        }
    }
}

void __cdecl FX_UpdateElement_HandleEmitting(
                FxSystem *system,
                FxElem *elem,
                const FxUpdateElem *update,
                const float *elemOriginPrev,
                FxUpdateResult *outUpdateResult)
{
    FxSpatialFrame frameBegin; // [esp+B0h] [ebp-70h] BYREF
    const FxElemDef *elemDef; // [esp+CCh] [ebp-54h]
    FxEffect *effect; // [esp+D0h] [ebp-50h]
    orientation_t orientPrev; // [esp+D4h] [ebp-4Ch] BYREF
    FxSpatialFrame frameEnd; // [esp+104h] [ebp-1Ch] BYREF

    elemDef = FX_GetUpdateElemDef(update);
    if ( elemDef->effectEmitted.handle )
    {
        effect = update->effect;
        FX_GetOrientation(elemDef, &effect->frameAtSpawn, &effect->framePrev, update->randomSeed, &orientPrev);
        FX_OrientationPosToWorldPos(&orientPrev, elemOriginPrev, frameBegin.origin);
        FX_GetQuatForOrientation(effect, elemDef, &effect->framePrev, &orientPrev, frameBegin.quat);
        frameEnd.origin[0] = update->posWorld[0];
        frameEnd.origin[1] = update->posWorld[1];
        frameEnd.origin[2] = update->posWorld[2];
        FX_GetQuatForOrientation(effect, elemDef, &effect->frameNow, &update->orient, frameEnd.quat);
        elem->emitResidual = FX_ProcessEmitting(system, update, elem->emitResidual, &frameBegin, &frameEnd);
        if ( update->msecUpdateEnd == update->msecElemEnd )
            *outUpdateResult = FX_UPDATE_REMOVE;
    }
}

unsigned __int8 __cdecl FX_ProcessEmitting(
                FxSystem *system,
                const FxUpdateElem *update,
                unsigned __int8 emitResidual,
                const FxSpatialFrame *frameBegin,
                const FxSpatialFrame *frameEnd)
{
    char *v7; // eax
    float v8; // [esp+10h] [ebp-B4h]
    float v9; // [esp+14h] [ebp-B0h]
    FxEffectContainer *effect; // [esp+20h] [ebp-A4h]
    unsigned int handle; // [esp+24h] [ebp-A0h]
    const FxElemDef *elemDef; // [esp+60h] [ebp-64h]
    float maxDistPerEmit; // [esp+64h] [ebp-60h]
    float baseDistPerEmit; // [esp+6Ch] [ebp-58h]
    float distInUpdate; // [esp+70h] [ebp-54h]
    float residual; // [esp+74h] [ebp-50h]
    FxSpatialFrame frameElemNow; // [esp+78h] [ebp-4Ch] BYREF
    float axisSpawn[3][3]; // [esp+94h] [ebp-30h] BYREF
    float distNextEmit; // [esp+B8h] [ebp-Ch]
    int msecAtSpawn; // [esp+BCh] [ebp-8h]
    float distLastEmit; // [esp+C0h] [ebp-4h]

    elemDef = FX_GetUpdateElemDef(update);
    if (elemDef->elemType == 7 && (elemDef->flags & 0x8000000) != 0)
        return emitResidual;
    axisSpawn[0][0] = frameEnd->origin[0] - frameBegin->origin[0];
    axisSpawn[0][1] = frameEnd->origin[1] - frameBegin->origin[1];
    axisSpawn[0][2] = frameEnd->origin[2] - frameBegin->origin[2];
    distInUpdate = Vec3Normalize(axisSpawn[0]);
    if (distInUpdate == 0.0)
        return emitResidual;
    baseDistPerEmit = (float)((float)(fx_randomTable[update->randomSeed + 20] * elemDef->emitDist.amplitude)
        + elemDef->emitDist.base)
        + elemDef->emitDistVariance.base;
    maxDistPerEmit = baseDistPerEmit + elemDef->emitDistVariance.amplitude;
    distNextEmit = -((float)((float)emitResidual * maxDistPerEmit) * 0.00390625);
    while (1)
    {
        distLastEmit = distNextEmit;
        distNextEmit = (float)((float)((float)((float)rand() * 0.000030517578) * elemDef->emitDistVariance.amplitude)
            + baseDistPerEmit)
            + distNextEmit;
        if (distNextEmit > distInUpdate)
            break;
        if ((float)(distNextEmit - 0.0) < 0.0)
            v9 = 0.0f;
        else
            v9 = distNextEmit;
        distNextEmit = v9;
        v8 = floor((float)((float)(update->msecUpdateEnd - update->msecUpdateBegin) * (float)(v9 / distInUpdate)));
        msecAtSpawn = update->msecUpdateBegin + (int)v8;
        Vec3Lerp(frameBegin->origin, frameEnd->origin, v9 / distInUpdate, frameElemNow.origin);
        Vec4Lerp(frameBegin->quat, frameEnd->quat, v9 / distInUpdate, frameElemNow.quat);
        Vec4Normalize(frameElemNow.quat);
        PerpendicularVector(axisSpawn[0], axisSpawn[1]);
        Vec3Cross(axisSpawn[0], axisSpawn[1], axisSpawn[2]);
        handle = FX_SpawnEffect(
            system,
            elemDef->effectEmitted.handle,
            msecAtSpawn,
            frameElemNow.origin,
            axisSpawn,
            0x7FFu,
            511,
            255,
            update->effect->owner,
            0x3FFu,
            &orIdentity);
        effect = UniqueHandleToEffect(system, handle);
        if (effect)
            FX_DelRefToEffect(system, effect, 0);
        elemDef = FX_GetUpdateElemDef(update);
    }
    residual = distInUpdate - distLastEmit;
    if ((float)(distInUpdate - distLastEmit) < -0.001 || (float)(maxDistPerEmit + 0.001) < residual)
    {
        v7 = va("%g, %g", residual, maxDistPerEmit);
        if (!Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
            678,
            0,
            "%s\n\t%s",
            "residual >= -0.001f && residual <= maxDistPerEmit + 0.001f",
            v7))
            __debugbreak();
    }
    return (int)((float)((float)(256.0 * residual) / maxDistPerEmit) + 9.313225746154785e-10);
}

void __cdecl FX_GetQuatForOrientation(
                const FxEffect *effect,
                const FxElemDef *elemDef,
                const FxSpatialFrame *frameNow,
                const orientation_t *orient,
                float *quat)
{
    int runFlags; // [esp+4h] [ebp-4h]

    runFlags = elemDef->flags & 0xC0;
    if ( runFlags )
    {
        if ( runFlags == 64 )
        {
            *quat = effect->frameAtSpawn.quat[0];
            quat[1] = effect->frameAtSpawn.quat[1];
            quat[2] = effect->frameAtSpawn.quat[2];
            quat[3] = effect->frameAtSpawn.quat[3];
        }
        else if ( runFlags == 128 )
        {
            *quat = frameNow->quat[0];
            quat[1] = frameNow->quat[1];
            quat[2] = frameNow->quat[2];
            quat[3] = frameNow->quat[3];
        }
        else
        {
            if ( runFlags != 192
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
                            1204,
                            0,
                            "%s\n\t(runFlags) = %i",
                            "(runFlags == FX_ELEM_RUN_RELATIVE_TO_OFFSET)",
                            runFlags) )
            {
                __debugbreak();
            }
            AxisToQuat(orient->axis, quat);
        }
    }
    else
    {
        *quat = 0.0f;
        quat[1] = 0.0f;
        quat[2] = 0.0f;
        quat[3] = 1.0f;
    }
}

void __cdecl FX_UpdateElement_HandleAttachedEffect(FxSystem *system, FxElem *elem, const FxUpdateElem *update)
{
    float *origin; // [esp+0h] [ebp-48h]
    float quat[4]; // [esp+Ch] [ebp-3Ch] BYREF
    float axis[3][3]; // [esp+1Ch] [ebp-2Ch] BYREF
    const FxElemDef *elemDef; // [esp+40h] [ebp-8h]
    FxEffect *attachedFx; // [esp+44h] [ebp-4h]

    if ( elem->attachedEffectHandle )
    {
        attachedFx = (FxEffect *)UniqueHandleToEffect(system, elem->attachedEffectHandle);
        if ( attachedFx )
        {
            while ( _InterlockedExchangeAdd((volatile unsigned __int32 *)&attachedFx[1], 0x20000000u) >= 0x20000000 )
                _InterlockedExchangeAdd((volatile unsigned __int32 *)&attachedFx[1], 0xE0000000);
            origin = attachedFx->frameNow.origin;
            attachedFx->frameNow.origin[0] = update->posWorld[0];
            origin[1] = update->posWorld[1];
            origin[2] = update->posWorld[2];
            _InterlockedExchangeAdd((volatile unsigned __int32 *)&attachedFx[1], 0xE0000000);
        }
    }
    else
    {
        elemDef = FX_GetUpdateElemDef(update);
        if ( elemDef->effectAttached.handle )
        {
            FX_GetQuatForOrientation(update->effect, elemDef, &update->effect->frameNow, &update->orient, quat);
            QuatToAxis(quat, axis);
            elem->attachedEffectHandle = FX_PlayOrientedEffect(
                                                                         system,
                                                                         elemDef->effectAttached.handle,
                                                                         update->msecUpdateBegin,
                                                                         update->posWorld,
                                                                         axis);
        }
    }
}

char __cdecl FX_UpdateElement_TruncateToElemBegin(FxUpdateElem *update, FxUpdateResult *outUpdateResult)
{
    const char *v3; // eax
    const FxElemDef *UpdateElemDef; // eax
    FxSpatialFrame *p_frameAtSpawn; // [esp-8h] [ebp-14h]
    FxSpatialFrame *p_frameNow; // [esp-4h] [ebp-10h]
    int randomSeed; // [esp+0h] [ebp-Ch]
    const FxElemDef *elemDef; // [esp+8h] [ebp-4h]

    if ( update->msecUpdateBegin < update->msecElemBegin )
        update->msecUpdateBegin = update->msecElemBegin;
    if ( update->msecUpdateBegin == update->msecUpdateEnd )
    {
        *outUpdateResult = (FxUpdateResult)(update->msecUpdateBegin < update->msecElemEnd);
        return 0;
    }
    else
    {
        elemDef = FX_GetUpdateElemDef(update);
        if ( update->msecLifeSpan <= 0.0 )
        {
            v3 = va("%i + [0,%i) = %g", elemDef->lifeSpanMsec.base, elemDef->lifeSpanMsec.amplitude, update->msecLifeSpan);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
                            1378,
                            0,
                            "%s\n\t%s",
                            "update->msecLifeSpan > 0.0f",
                            v3) )
                __debugbreak();
        }
        update->msecElapsed = (float)(update->msecUpdateEnd - update->msecElemBegin);
        update->normTimeUpdateEnd = update->msecElapsed / update->msecLifeSpan;
        if ( (update->normTimeUpdateEnd < 0.0 || update->normTimeUpdateEnd > 1.0)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
                        1383,
                        0,
                        "%s\n\t(update->normTimeUpdateEnd) = %g",
                        "(update->normTimeUpdateEnd >= 0.0f && update->normTimeUpdateEnd <= 1.0f)",
                        update->normTimeUpdateEnd) )
        {
            __debugbreak();
        }
        randomSeed = update->randomSeed;
        p_frameNow = &update->effect->frameNow;
        p_frameAtSpawn = &update->effect->frameAtSpawn;
        UpdateElemDef = FX_GetUpdateElemDef(update);
        FX_GetOrientation(UpdateElemDef, p_frameAtSpawn, p_frameNow, randomSeed, &update->orient);
        return 1;
    }
}

void __cdecl FX_UpdateEffectPartialTrail(
                FxSystem *system,
                FxEffect *effect,
                FxEffectContainer *remoteEffect,
                FxTrail *trail,
                int msecUpdateBegin,
                int msecUpdateEnd,
                float distanceTravelledBegin,
                float distanceTravelledEnd,
                unsigned __int16 trailElemHandleStart,
                unsigned __int16 trailElemHandleStop,
                FxSpatialFrame *frameNow)
{
    float basis[2][3]; // [esp+20h] [ebp-34h] BYREF
    FxTrailElem *trailElem; // [esp+3Ch] [ebp-18h]
    FxTrailElem *remoteTrailElem; // [esp+40h] [ebp-14h]
    unsigned __int16 trailElemHandleNext; // [esp+44h] [ebp-10h]
    unsigned __int16 trailElemHandleLast; // [esp+48h] [ebp-Ch]
    unsigned __int16 trailElemHandle; // [esp+4Ch] [ebp-8h]
    bool removable; // [esp+53h] [ebp-1h]

    trailElemHandleLast = -1;
    if ( trailElemHandleStart == 0xFFFF )
        trailElemHandle = trail->firstElemHandle;
    else
        trailElemHandle = trailElemHandleStart;
    trailElem = 0;
    remoteTrailElem = 0;
    removable = trailElemHandle == trail->firstElemHandle;
    while ( trailElemHandle != trailElemHandleStop )
    {
        if ( trailElemHandle == 0xFFFF
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
                        1841,
                        0,
                        "%s",
                        "trailElemHandle != FX_HANDLE_NONE") )
        {
            __debugbreak();
        }
        remoteTrailElem = (FxTrailElem *)FX_TrailElemFromHandle(system, trailElemHandle);
        trailElem = remoteTrailElem;
        trailElemHandleNext = remoteTrailElem->nextTrailElemHandle;
        if ( FX_UpdateTrailElement(system, effect, trail, remoteTrailElem, msecUpdateBegin, msecUpdateEnd) )
            removable = 0;
        if ( removable && trailElemHandleLast != 0xFFFF )
            FX_FreeTrailElem(system, trailElemHandleLast, effect, remoteEffect, trail);
        trailElemHandleLast = trailElemHandle;
        trailElemHandle = trailElemHandleNext;
    }
    if ( trailElemHandleLast != 0xFFFF && trailElemHandleLast == trail->lastElemHandle )
    {
        if ( removable )
        {
            FX_FreeTrailElem(system, trailElemHandleLast, effect, remoteEffect, trail);
        }
        else if ( (remoteEffect->atomics.status & 0x10000) != 0 )
        {
            if ( !trailElem
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
                            1875,
                            0,
                            "%s",
                            "trailElem") )
            {
                __debugbreak();
            }
            trailElem->spawnDist = effect->distanceTraveled;
            FX_GetOriginForTrailElem(
                effect,
                &effect->def->elemDefsEA[trail->defIndex],
                frameNow,
                (296 * trailElem->sequence + trailElem->msecBegin + (unsigned int)effect->randomSeed) % 0x1DF,
                trailElem->origin,
                basis[0],
                basis[1]);
            FX_TrailElem_CompressBasis(basis, trailElem->basis);
        }
    }
}

FxUpdateResult __cdecl FX_UpdateTrailElement(
                FxSystem *system,
                FxEffect *effect,
                FxTrail *trail,
                FxTrailElem *trailElem,
                int msecUpdateBegin,
                int msecUpdateEnd)
{
    int v7; // [esp+8h] [ebp-B0h]
    int v8; // [esp+Ch] [ebp-ACh]
    float v9; // [esp+14h] [ebp-A4h]
    FxUpdateElem update; // [esp+28h] [ebp-90h] BYREF
    FxUpdateResult updateResult; // [esp+A8h] [ebp-10h] BYREF
    float baseVel[3]; // [esp+ACh] [ebp-Ch] BYREF

    updateResult = FX_UPDATE_KEEP;
    if ( FX_UpdateElement_SetupUpdate(
                 effect,
                 msecUpdateBegin,
                 msecUpdateEnd,
                 trail->defIndex,
                 0,
                 trailElem->msecBegin,
                 0,
                 trailElem->origin,
                 0.0,
                 &update) )
    {
        FX_UpdateElement_TruncateToElemEnd(&update, &updateResult);
        if ( updateResult )
        {
            if ( !FX_UpdateElement_TruncateToElemBegin(&update, &updateResult) )
                return updateResult;
            v9 = (float)trailElem->baseVelZ * 0.001;
            baseVel[0] = 0.0f;
            baseVel[1] = 0.0f;
            baseVel[2] = v9;
            update.elemBaseVel = baseVel;
            update.physObjId = 0;
            update.onGround = 0;
            updateResult = (FxUpdateResult)FX_UpdateElementPosition(system, &update);
            v8 = (int)(float)(baseVel[2] / 0.001);
            if ( v8 >= -32768 )
            {
                if ( v8 <= 0x7FFF )
                    v7 = (int)(float)(baseVel[2] / 0.001);
                else
                    LOWORD_NAMEHACK(v7) = 0x7FFF;
            }
            else
            {
                LOWORD_NAMEHACK(v7) = 0x8000;
            }
            trailElem->baseVelZ = v7;
        }
    }
    return updateResult;
}

void __cdecl FX_UpdateBatch(
                FxSystem *system,
                FxSystem *remoteSystem,
                int localClientNum,
                bool nonBoltedEffectsOnly,
                int firstEffect,
                int lastEffect)
{
    FxEffectContainer *remoteEffect; // [esp+28h] [ebp-14h]

    PROF_SCOPED("FX_UpdateBatch");

    if ( !system
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp", 2131, 0, "%s", "system") )
    {
        __debugbreak();
    }
    FX_BeginIteratingOverEffects_Cooperative(system);
    while ( firstEffect != lastEffect )
    {
        remoteEffect = FX_EffectFromHandle(system, system->shared->allEffectHandles[firstEffect & 0x3FF]);
        if ( FX_ShouldProcessEffect(system, &remoteEffect->effect, remoteEffect, nonBoltedEffectsOnly)
            && (remoteEffect->atomics.status & 0x3FFF) != 0 )
        {
            if ( FX_AcquireEffectLock_IsLiveEffect(&remoteEffect->effect) )
            {
                FX_UpdateEffect(system, &remoteEffect->effect, remoteEffect);
                _InterlockedExchangeAdd(&remoteEffect->atomics.status, 0xE0000000);
            }
        }
        ++firstEffect;
    }
    if ( !_InterlockedDecrement(&system->shared->iteratorCount) )
        FX_RunGarbageCollectionAndPrioritySort(system);
}

char __cdecl FX_AcquireEffectLock_IsLiveEffect(FxEffect *effect)
{
    while ( _InterlockedExchangeAdd((volatile unsigned __int32 *)&effect[1], 0x20000000u) >= 0x20000000 )
    {
        _InterlockedExchangeAdd((volatile unsigned __int32 *)&effect[1], 0xE0000000);
        if ( ((int)effect[1].def & 0x3FFF) == 0 )
            return 0;
    }
    return 1;
}

void __cdecl FX_UpdateEffect(FxSystem *system, FxEffect *effect, FxEffectContainer *remoteEffect)
{
    unsigned __int16 lastElemHandle[4]; // [esp+30h] [ebp-14h] BYREF
    float newDistanceTraveled; // [esp+3Ch] [ebp-8h]
    unsigned int elemClass; // [esp+40h] [ebp-4h]

    if ( (remoteEffect->atomics.status & 0x3FFF) != 0 && effect->msecLastUpdate <= system->msecNow )
    {
        FX_UpdateEffectBolt(system, effect, remoteEffect);
        newDistanceTraveled = Vec3Distance(effect->framePrev.origin, effect->frameNow.origin) + effect->distanceTraveled;
        for ( elemClass = 0; elemClass < 3; ++elemClass )
            lastElemHandle[elemClass] = -1;
        FX_UpdateEffectPartial(
            system,
            effect,
            remoteEffect,
            effect->msecLastUpdate,
            system->msecNow,
            effect->distanceTraveled,
            newDistanceTraveled,
            effect->firstElemHandle,
            lastElemHandle,
            0,
            0);
        FX_SortNewElemsInEffect(system, effect);
        memcpy(&effect->framePrev, &effect->frameNow, sizeof(effect->framePrev));
        effect->distanceTraveled = newDistanceTraveled;
    }
}

#if 0
void __cdecl FX_UpdateEffectBolt(FxSystem *system, FxEffect *effect, FxEffectContainer *remoteEffect)
{
    const DynEntityPose *ClientPose; // eax
    orientation_t fullOrient; // [esp+14h] [ebp-6Ch] BYREF
    int localClientNum; // [esp+44h] [ebp-3Ch]
    orientation_t orient; // [esp+48h] [ebp-38h] BYREF
    const DynEntityPose *dynEntPose; // [esp+78h] [ebp-8h]
    bool temporalBitsValid; // [esp+7Fh] [ebp-1h]

    if ( ((*(unsigned int *)&effect->boltAndSortOrder.0 >> 29) & 3) == 2 )
    {
        if ( DynEnt_Valid(*(unsigned int *)&effect->boltAndSortOrder.0 >> 8) )
        {
            ClientPose = DynEnt_GetClientPose(*(unsigned int *)&effect->boltAndSortOrder.0 >> 8);
            dynEntPose = ClientPose;
            effect->frameNow.origin[0] = ClientPose->pose.origin[0];
            effect->frameNow.origin[1] = ClientPose->pose.origin[1];
            effect->frameNow.origin[2] = ClientPose->pose.origin[2];
            effect->frameNow.quat[0] = dynEntPose->pose.quat[0];
            effect->frameNow.quat[1] = dynEntPose->pose.quat[1];
            effect->frameNow.quat[2] = dynEntPose->pose.quat[2];
            effect->frameNow.quat[3] = dynEntPose->pose.quat[3];
        }
        else
        {
            FX_KillEffect(system, remoteEffect);
        }
    }
    else if ( ((*(unsigned int *)&effect->boltAndSortOrder.0 >> 29) & 3) == 1 )
    {
        localClientNum = system->localClientNum;
        temporalBitsValid = FX_GetBoltTemporalBits(localClientNum, (*(unsigned int *)&effect->boltAndSortOrder.0 >> 8) & 0x7FF) == ((*(unsigned int *)&effect->boltAndSortOrder.0 >> 19) & 1);
        if ( temporalBitsValid
            && FX_GetBoneOrientation(
                     localClientNum,
                     (*(unsigned int *)&effect->boltAndSortOrder.0 >> 8) & 0x7FF,
                     (*(unsigned int *)&effect->boltAndSortOrder.0 >> 20) & 0x1FF,
                     &orient) )
        {
            OrientationConcatenate(&effect->boneOffset, &orient, &fullOrient);
            effect->frameNow.origin[0] = fullOrient.origin[0];
            effect->frameNow.origin[1] = fullOrient.origin[1];
            effect->frameNow.origin[2] = fullOrient.origin[2];
            AxisToQuat(fullOrient.axis, effect->frameNow.quat);
        }
        else
        {
            if ( FX_CanKillImmediate(effect->def) )
                FX_KillEffect(system, remoteEffect);
            else
                FX_StopEffect(system, remoteEffect);
            *(unsigned int *)&effect->boltAndSortOrder.0 |= 0x1FF00000u;
            *(unsigned int *)&effect->boltAndSortOrder.0 |= 0x7FF00u;
            *(unsigned int *)&effect->boltAndSortOrder.0 &= 0x9FFFFFFF;
            *(unsigned int *)&effect->boltAndSortOrder.0 |= 0x80000000;
        }
    }
}
#endif

// aislopped unionization
void FX_UpdateEffectBolt(
    FxSystem *system,
    FxEffect *effect,
    FxEffectContainer *remoteEffect)
{
    orientation_t orient;
    orientation_t fullOrient;

    FxBoltAndSortOrder &bolt = effect->boltAndSortOrder;

    /*
        Case 1: Absolute dynamic entity (type == 2)
    */
    if (bolt.type == 2)
    {
        if (DynEnt_Valid(bolt.dobjHandle))
        {
            const DynEntityPose *dynEntPose =
                DynEnt_GetClientPose(bolt.dobjHandle);

            effect->frameNow.origin[0] = dynEntPose->pose.origin[0];
            effect->frameNow.origin[1] = dynEntPose->pose.origin[1];
            effect->frameNow.origin[2] = dynEntPose->pose.origin[2];

            effect->frameNow.quat[0] = dynEntPose->pose.quat[0];
            effect->frameNow.quat[1] = dynEntPose->pose.quat[1];
            effect->frameNow.quat[2] = dynEntPose->pose.quat[2];
            effect->frameNow.quat[3] = dynEntPose->pose.quat[3];
        }
        else
        {
            FX_KillEffect(system, remoteEffect);
        }

        return;
    }

    /*
        Case 2: DObj bone bolt (type == 1)
    */
    if (bolt.type == 1)
    {
        const int localClientNum = system->localClientNum;

        bool temporalBitsValid =
            FX_GetBoltTemporalBits(localClientNum,
                bolt.dobjHandle) == bolt.temporalBits;

        if (temporalBitsValid &&
            FX_GetBoneOrientation(localClientNum,
                bolt.dobjHandle,
                bolt.boneIndex,
                &orient))
        {
            OrientationConcatenate(&effect->boneOffset,
                &orient,
                &fullOrient);

            effect->frameNow.origin[0] = fullOrient.origin[0];
            effect->frameNow.origin[1] = fullOrient.origin[1];
            effect->frameNow.origin[2] = fullOrient.origin[2];

            AxisToQuat(fullOrient.axis,
                effect->frameNow.quat);
        }
        else
        {
            if (FX_CanKillImmediate(effect->def))
                FX_KillEffect(system, remoteEffect);
            else
                FX_StopEffect(system, remoteEffect);

            /*
                Old bit mutation replaced cleanly
            */
            bolt.boneIndex = 0x1FF;
            bolt.dobjHandle = 0x7FF;
            bolt.type = 0;
            bolt.rewindKill = 1;
        }
    }
}


char __cdecl FX_CanKillImmediate(const FxEffectDef *def)
{
    int elemDefIndex; // [esp+4h] [ebp-4h]

    if ( !def && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp", 1943, 0, "%s", "def") )
        __debugbreak();
    if ( def->elemDefCountEmission )
        return 0;
    for ( elemDefIndex = 0; elemDefIndex < def->elemDefCountOneShot + def->elemDefCountLooping; ++elemDefIndex )
    {
        if ( def->elemDefsEA[elemDefIndex].elemType == 5 )
            return 0;
    }
    return 1;
}

#if 0
bool __cdecl FX_ShouldProcessEffect(
                FxSystem *system,
                FxEffect *effect,
                FxEffectContainer *remoteEffect,
                bool nonBoltedEffectsOnly)
{
    if ( nonBoltedEffectsOnly && ((*(unsigned int *)&effect->boltAndSortOrder.0 >> 29) & 3) != 0 )
        return 0;
    if ( _InterlockedExchange(&remoteEffect->effect.frameCount, system->frameCount) == system->frameCount )
        return 0;
    return system->shared->activeSpotLightEffectCount <= 0
            || remoteEffect != FX_EffectFromHandle(system, system->shared->activeSpotLightEffectHandle);
}
#endif

// aislop unionization
bool FX_ShouldProcessEffect(
    FxSystem *system,
    FxEffect *effect,
    FxEffectContainer *remoteEffect,
    bool nonBoltedEffectsOnly)
{
    const FxBoltAndSortOrder &bolt = effect->boltAndSortOrder;

    // Skip bolted effects if requested
    if (nonBoltedEffectsOnly && bolt.type != 0)
        return false;

    // Prevent double-processing within same frame
    if (_InterlockedExchange(&remoteEffect->effect.frameCount,
        system->frameCount) == system->frameCount)
        return false;

    // Spotlight special-case
    return system->shared->activeSpotLightEffectCount <= 0
        || remoteEffect != FX_EffectFromHandle(
            system,
            system->shared->activeSpotLightEffectHandle);
}

void __cdecl FX_Update(FxSystem *system, FxSystem *remoteSystem, int localClientNum, bool nonBoltedEffectsOnly)
{
    PROF_SCOPED("FX_Update()");
    FX_UpdateBatch(
        system,
        remoteSystem,
        localClientNum,
        nonBoltedEffectsOnly,
        system->shared->firstActiveEffect,
        system->shared->firstNewEffect);
}

void __cdecl FX_UpdateSpotLight(FxCmd *cmd)
{
    FxEffectContainer *effect; // [esp+10h] [ebp-8h]
    FxSystem *system; // [esp+14h] [ebp-4h]

    if ( fx_enable->current.enabled )
    {
        system = cmd->system;
        if ( !cmd->system
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp", 2260, 0, "%s", "system") )
        {
            __debugbreak();
        }
        FX_BeginIteratingOverEffects_Cooperative(system);
        if ( system->shared->activeSpotLightEffectCount > 0 )
        {
            if ( system->shared->activeSpotLightEffectCount != 1
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
                            2265,
                            0,
                            "%s",
                            "system->shared->activeSpotLightEffectCount == 1") )
            {
                __debugbreak();
            }
            for ( effect = FX_EffectFromHandle(system, system->shared->activeSpotLightEffectHandle);
                        _InterlockedExchangeAdd(&effect->atomics.status, 0x20000000u) >= 0x20000000;
                        _InterlockedExchangeAdd(&effect->atomics.status, 0xE0000000) )
            {
                ;
            }
            FX_UpdateSpotLightEffect(system, effect, effect);
            _InterlockedExchangeAdd(&effect->atomics.status, 0xE0000000);
        }
        if ( !_InterlockedDecrement(&system->shared->iteratorCount) )
            FX_RunGarbageCollectionAndPrioritySort(system);
        if ( fx_draw->current.enabled )
            FX_DrawSpotLight(system);
    }
}

#if 0
void __cdecl FX_UpdateSpotLightEffect(FxSystem *system, FxEffectContainer *effect, FxEffectContainer *remoteEffect)
{
    unsigned __int16 lastElemHandle[4]; // [esp+30h] [ebp-14h] BYREF
    float newDistanceTraveled; // [esp+3Ch] [ebp-8h]
    unsigned int elemClass; // [esp+40h] [ebp-4h]

    if ( (remoteEffect->atomics.status & 0x3FFF) != 0 && effect->effect.msecLastUpdate <= system->msecNow )
    {
        FX_UpdateEffectBolt(system, &effect->effect, remoteEffect);
        if ( ((*(unsigned int *)&effect->effect.boltAndSortOrder.0 >> 29) & 3) == 2
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
                        2069,
                        0,
                        "%s",
                        "effect->boltAndSortOrder.type != FX_BOLT_TYPE_DYNENT") )
        {
            __debugbreak();
        }
        system->activeSpotLightBoltDobj = (*(unsigned int *)&effect->effect.boltAndSortOrder.0 >> 8) & 0x7FF;
        newDistanceTraveled = Vec3Distance(effect->effect.framePrev.origin, effect->effect.frameNow.origin)
                                                + effect->effect.distanceTraveled;
        for ( elemClass = 0; elemClass < 3; ++elemClass )
            lastElemHandle[elemClass] = -1;
        FX_UpdateSpotLightEffectPartial(system, effect, effect->effect.msecLastUpdate, system->msecNow);
        FX_UpdateEffectPartial(
            system,
            &effect->effect,
            remoteEffect,
            effect->effect.msecLastUpdate,
            system->msecNow,
            effect->effect.distanceTraveled,
            newDistanceTraveled,
            effect->effect.firstElemHandle,
            lastElemHandle,
            0,
            0);
        FX_SortNewElemsInEffect(system, &effect->effect);
        memcpy(&effect->effect.framePrev, &effect->effect.frameNow, sizeof(effect->effect.framePrev));
        effect->effect.distanceTraveled = newDistanceTraveled;
    }
}
#endif

// aislop unionization
void FX_UpdateSpotLightEffect(
    FxSystem *system,
    FxEffectContainer *effect,
    FxEffectContainer *remoteEffect)
{
    uint16_t lastElemHandle[4];
    float newDistanceTraveled;
    unsigned int elemClass;

    if ((remoteEffect->atomics.status & 0x3FFF) != 0 &&
        effect->effect.msecLastUpdate <= system->msecNow)
    {
        FX_UpdateEffectBolt(system, &effect->effect, remoteEffect);

        FxBoltAndSortOrder &bolt = effect->effect.boltAndSortOrder;

        /*
            Assert: spotlight effects must NOT be dynent bolts (type 2)
        */
        if (bolt.type == 2 &&
            !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
                2069,
                0,
                "%s",
                "effect->boltAndSortOrder.type != FX_BOLT_TYPE_DYNENT"))
        {
            __debugbreak();
        }

        /*
            Old: (>> 8) & 0x7FF
        */
        system->activeSpotLightBoltDobj = bolt.dobjHandle;

        newDistanceTraveled =
            Vec3Distance(effect->effect.framePrev.origin,
                effect->effect.frameNow.origin)
            + effect->effect.distanceTraveled;

        for (elemClass = 0; elemClass < 3; ++elemClass)
            lastElemHandle[elemClass] = static_cast<uint16_t>(-1);

        FX_UpdateSpotLightEffectPartial(
            system,
            effect,
            effect->effect.msecLastUpdate,
            system->msecNow);

        FX_UpdateEffectPartial(
            system,
            &effect->effect,
            remoteEffect,
            effect->effect.msecLastUpdate,
            system->msecNow,
            effect->effect.distanceTraveled,
            newDistanceTraveled,
            effect->effect.firstElemHandle,
            lastElemHandle,
            0,
            0);

        FX_SortNewElemsInEffect(system, &effect->effect);

        memcpy(&effect->effect.framePrev,
            &effect->effect.frameNow,
            sizeof(effect->effect.framePrev));

        effect->effect.distanceTraveled = newDistanceTraveled;
    }
}


void __cdecl FX_UpdateSpotLightEffectPartial(
                FxSystem *system,
                FxEffectContainer *effect,
                int msecUpdateBegin,
                int msecUpdateEnd)
{
    FxPool<FxElem,FxElemContainer> *elem; // [esp+14h] [ebp-4h]

    if ( system->shared->activeSpotLightEffectCount != 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
                    1702,
                    0,
                    "%s",
                    "system->shared->activeSpotLightEffectCount == 1") )
    {
        __debugbreak();
    }
    if ( effect != FX_EffectFromHandle(system, system->shared->activeSpotLightEffectHandle)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
                    1703,
                    0,
                    "%s",
                    "effect == FX_EffectFromHandle( system, system->shared->activeSpotLightEffectHandle )") )
    {
        __debugbreak();
    }
    if ( effect->effect.msecLastUpdate > msecUpdateEnd
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
                    1704,
                    0,
                    "effect->msecLastUpdate <= msecUpdateEnd\n\t%g, %g",
                    (float)effect->effect.msecLastUpdate,
                    (float)msecUpdateEnd) )
    {
        __debugbreak();
    }
    if ( system->shared->activeSpotLightElemCount )
    {
        elem = FX_ElemFromHandle(system, system->shared->activeSpotLightElemHandle);
        if ( FX_UpdateElement(system, &effect->effect, (FxElem *)elem, msecUpdateBegin, msecUpdateEnd) == FX_UPDATE_REMOVE )
            FX_FreeSpotLightElem(system, system->shared->activeSpotLightElemHandle, effect);
    }
}

void __cdecl FX_UpdateNonDependent(FxCmd *cmd)
{
    if ( fx_enable->current.enabled )
        FX_Update(cmd->system, cmd->system, cmd->localClientNum, 1);
}

void __cdecl FX_UpdateRemaining(FxCmd *cmd)
{
    if ( fx_enable->current.enabled )
        FX_Update(cmd->system, cmd->system, cmd->localClientNum, 0);
}

void __cdecl FX_BeginUpdate(int localClientNum)
{
    FxSystemContainer *system; // [esp+8h] [ebp-4h]

    FX_Beam_Begin();
    FX_PostLight_Begin();
    FX_SpriteBegin();
    system = FX_GetSystem(localClientNum);
    FX_Priority_Begin(&system->system);
    memcpy(&system->system.extraCameraPre, &system->system.extraCamera, sizeof(system->system.extraCameraPre));
    system->system.extraCamera.isValid = 0;
}

void __cdecl FX_EndUpdate(int localClientNum)
{
    FxSystemContainer *system; // [esp+8h] [ebp-4h]

    if ( fx_enable->current.enabled )
    {
        system = FX_GetSystem(localClientNum);
        if ( !system
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp", 2405, 0, "%s", "system") )
        {
            __debugbreak();
        }
        memcpy(&system->system.cameraPrev, system, sizeof(system->system.cameraPrev));
        if ( !system->system.cameraPrev.isValid
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
                        2408,
                        0,
                        "%s",
                        "system->cameraPrev.isValid") )
        {
            __debugbreak();
        }
    }
}

void __cdecl FX_AddNonSpriteDrawSurfs(FxCmd *cmd)
{
    FxReflectParams reflect; // [esp+8h] [ebp-10h] BYREF
    FxSystem *system; // [esp+14h] [ebp-4h]

    system = cmd->system;
    if ( !system
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp", 2425, 0, "%s", "system") )
    {
        __debugbreak();
    }
    if ( fx_enable->current.enabled && fx_draw->current.enabled )
    {
        FX_SortEffects(system);
        FX_GetNullReflection(&reflect);
        FX_DrawNonSpriteElems(system, &reflect, cmd->updateCameraType);
        FX_GetReflection(cmd->useReflection, cmd->reflectionHeight, &reflect);
        if ( reflect.enabled )
            FX_DrawNonSpriteElems(system, &reflect, cmd->updateCameraType);
    }
}

void __cdecl FX_RewindTo_Setup(int localClientNum, int time)
{
    int v2; // eax
    unsigned __int16 effectHandle; // [esp+14h] [ebp-14h]
    FxEffectContainer *effect; // [esp+1Ch] [ebp-Ch]
    FxSystemContainer *system; // [esp+20h] [ebp-8h]
    int activeIndex; // [esp+24h] [ebp-4h]

    system = FX_GetSystem(localClientNum);
    if ( !system
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp", 2464, 0, "%s", "system") )
    {
        __debugbreak();
    }
    if ( time < system->system.msecNow )
    {
        system->system.msecNow = time;
        memset(system->system.restartList, 0, sizeof(system->system.restartList));
        FX_BeginIteratingOverEffects_Cooperative(&system->system);
        for ( activeIndex = system->system.shared->firstActiveEffect;
                    activeIndex != system->system.shared->firstNewEffect;
                    ++activeIndex )
        {
            effectHandle = system->system.shared->allEffectHandles[activeIndex & 0x3FF];
            effect = FX_EffectFromHandle(&system->system, effectHandle);
            if ( (effect->atomics.status & 0x3FFF) != 0 )
            {
                if ( effect->effect.msecBegin < time && effect->effect.owner == effectHandle )
                {
                    FX_AddRefToEffect(&system->system, &effect->effect);
                    v2 = FX_IndexFromEffectHandle(effectHandle);
                    Com_BitSetAssert(system->system.restartList, v2, 128);
                }
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

unsigned int __cdecl FX_IndexFromEffectHandle(unsigned __int16 handle)
{
    unsigned int index; // [esp+0h] [ebp-4h]

    if ( handle % 0x30u
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\effectscore\\fx_system.h",
                    439,
                    0,
                    "%s",
                    "handle % ( sizeof( FxEffect ) / FxEffect::HANDLE_SCALE ) == 0") )
    {
        __debugbreak();
    }
    index = handle / 0x30u;
    if ( index >= 0x400
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\effectscore\\fx_system.h",
                    441,
                    0,
                    "index doesn't index FX_EFFECT_LIMIT\n\t%i not in [0, %i)",
                    index,
                    1024) )
    {
        __debugbreak();
    }
    return handle / 0x30u;
}

void __cdecl FX_RewindTo_Finish(int localClientNum)
{
    unsigned __int16 v1; // ax
    volatile unsigned __int32 *p_status; // [esp+0h] [ebp-18h]
    signed __int32 v3; // [esp+4h] [ebp-14h]
    FxEffectContainer *effect; // [esp+Ch] [ebp-Ch]
    FxSystemContainer *system; // [esp+10h] [ebp-8h]
    unsigned int effectIndex; // [esp+14h] [ebp-4h]

    system = FX_GetSystem(localClientNum);
    if ( !system
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp", 2512, 0, "%s", "system") )
    {
        __debugbreak();
    }
    for ( effectIndex = 0; effectIndex < 0x400; ++effectIndex )
    {
        if ( Com_BitCheckAssert(system->system.restartList, effectIndex, 128) )
        {
            v1 = FX_EffectHandleFromIndex(effectIndex);
            effect = FX_EffectFromHandle(&system->system, v1);
            effect->effect.msecLastUpdate = effect->effect.msecBegin;
            if ( !effect->effect.def
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
                            2522,
                            0,
                            "%s",
                            "effect->def") )
            {
                __debugbreak();
            }
            if ( effect->effect.def )
            {
                if ( effect->effect.def->msecLoopingLife )
                {
                    p_status = &effect->atomics.status;
                    do
                        v3 = *p_status;
                    while ( _InterlockedCompareExchange(p_status, v3 | 0x10000, v3) != v3 );
                    FX_StartNewEffect(&system->system, &effect->effect, effect);
                }
                else
                {
                    FX_StartNewEffect(&system->system, &effect->effect, effect);
                    FX_DelRefToEffect(&system->system, effect, 0);
                }
            }
            else
            {
                FX_DelRefToEffect(&system->system, effect, 0);
            }
        }
    }
    memset(system->system.restartList, 0, sizeof(system->system.restartList));
    FX_RunGarbageCollectionAndPrioritySort(&system->system);
}

unsigned int __cdecl FX_EffectHandleFromIndex(unsigned int index)
{
    if ( index >= 0x400
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\effectscore\\fx_system.h",
                    428,
                    0,
                    "index doesn't index FX_EFFECT_LIMIT\n\t%i not in [0, %i)",
                    index,
                    1024) )
    {
        __debugbreak();
    }
    return 48 * index;
}

bool bRestart = true;

void __cdecl FX_RewindTo(int localClientNum, int time)
{
    volatile unsigned __int32 *p_status; // [esp+0h] [ebp-103Ch]
    signed __int32 v3; // [esp+4h] [ebp-1038h]
    bool IsDObjEntityValid; // [esp+1Fh] [ebp-101Dh]
    unsigned __int16 v5; // [esp+20h] [ebp-101Ch]
    FxEffectContainer *effect; // [esp+28h] [ebp-1014h]
    FxEffectContainer *effecta; // [esp+28h] [ebp-1014h]
    FxEffectContainer *effectb; // [esp+28h] [ebp-1014h]
    int bitNum; // [esp+2Ch] [ebp-1010h]
    int bitNuma; // [esp+2Ch] [ebp-1010h]
    FxSystemContainer *system; // [esp+30h] [ebp-100Ch]
    unsigned int v12[1024]; // [esp+34h] [ebp-1008h]
    int v13; // [esp+1034h] [ebp-8h]
    volatile int i; // [esp+1038h] [ebp-4h]

    system = FX_GetSystem(localClientNum);
    if ( !system
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp", 2565, 0, "%s", "system") )
    {
        __debugbreak();
    }
    if ( time < system->system.msecNow )
    {
        system->system.msecNow = time;
        v13 = 0;
        memset(system->system.restartList, 0, sizeof(system->system.restartList));
        FX_BeginIteratingOverEffects_Cooperative(&system->system);
        for ( i = system->system.shared->firstActiveEffect; i != system->system.shared->firstNewEffect; ++i )
        {
            v5 = system->system.shared->allEffectHandles[i & 0x3FF];
            effect = FX_EffectFromHandle(&system->system, v5);
            v12[v13++] = (unsigned int)effect;
            if ( (effect->atomics.status & 0x3FFF) != 0 && effect->effect.msecBegin < time && effect->effect.owner == v5 )
            {
                IsDObjEntityValid = 1;
#if 0 // aislop unionization (hey, its convenient)
                if ( FX_IsDemoPlaying() )
                {
                    if ( ((*(unsigned int *)&effect->effect.boltAndSortOrder.0 >> 8) & 0x7FF) != 0x7FF )
                        IsDObjEntityValid = FX_GetBoneOrientation_IsDObjEntityValid(
                                                                    localClientNum,
                                                                    (*(unsigned int *)&effect->effect.boltAndSortOrder.0 >> 8) & 0x7FF);
                    if ( IsDObjEntityValid && *(unsigned int *)&effect->effect.boltAndSortOrder.0 < 0 )
                        IsDObjEntityValid = 0;
                }
#endif
                if (FX_IsDemoPlaying())
                {
                    FxBoltAndSortOrder &bolt = effect->effect.boltAndSortOrder;

                    if (bolt.dobjHandle != 0x7FF)
                    {
                        IsDObjEntityValid =
                            FX_GetBoneOrientation_IsDObjEntityValid(
                                localClientNum,
                                bolt.dobjHandle);
                    }

                    if (IsDObjEntityValid && bolt.rewindKill)
                    {
                        IsDObjEntityValid = false;
                    }
                }
                if ( IsDObjEntityValid )
                {
                    FX_AddRefToEffect(&system->system, &effect->effect);
                    Com_BitSetAssert(system->system.restartList, v13 - 1, 128);
                }
            }
        }
        for ( bitNum = 0; bitNum < v13; ++bitNum )
        {
            effecta = (FxEffectContainer *)v12[bitNum];
            if ( (effecta->atomics.status & 0x3FFF) != 0 )
            {
                while ( _InterlockedExchangeAdd(&effecta->atomics.status, 0x20000000u) >= 0x20000000 )
                    _InterlockedExchangeAdd(&effecta->atomics.status, 0xE0000000);
                FX_KillEffect(&system->system, effecta);
                _InterlockedExchangeAdd(&effecta->atomics.status, 0xE0000000);
            }
        }
        if ( !_InterlockedDecrement(&system->system.shared->iteratorCount) )
            FX_RunGarbageCollectionAndPrioritySort(&system->system);
        if ( bRestart )
        {
            for ( bitNuma = 0; bitNuma < v13; ++bitNuma )
            {
                if ( Com_BitCheckAssert(system->system.restartList, bitNuma, 128) )
                {
                    effectb = (FxEffectContainer *)v12[bitNuma];
                    effectb->effect.msecLastUpdate = effectb->effect.msecBegin;
                    if ( effectb->effect.def )
                    {
                        if ( effectb->effect.def->msecLoopingLife )
                        {
                            p_status = &effectb->atomics.status;
                            do
                                v3 = *p_status;
                            while ( _InterlockedCompareExchange(p_status, v3 | 0x10000, v3) != v3 );
                            FX_StartNewEffect(&system->system, &effectb->effect, effectb);
                        }
                        else
                        {
                            FX_StartNewEffect(&system->system, &effectb->effect, effectb);
                            if ( (effectb->atomics.status & 0x3FFF) != 0 )
                                FX_DelRefToEffect(&system->system, effectb, 0);
                        }
                    }
                }
            }
        }
        memset(system->system.restartList, 0, sizeof(system->system.restartList));
        FX_RunGarbageCollectionAndPrioritySort(&system->system);
    }
}

void __cdecl DB_RestartEffects_Begin()
{
    int localClientNum; // [esp+0h] [ebp-8h]
    FxSystemContainer *system; // [esp+4h] [ebp-4h]

    for ( localClientNum = 0; localClientNum < fx_maxLocalClients; ++localClientNum )
    {
        system = FX_GetSystem(localClientNum);
        FX_RewindTo_Setup(localClientNum, system->system.msecNow);
    }
}

void __cdecl DB_RestartEffects_Finish()
{
    int localClientNum; // [esp+0h] [ebp-8h]

    for ( localClientNum = 0; localClientNum < fx_maxLocalClients; ++localClientNum )
    {
        FX_GetSystem(localClientNum);
        FX_RewindTo_Finish(localClientNum);
    }
}

void __cdecl FX_GetCameraUpdateFromRefdefAndZFar(FxCameraUpdate *cameraUpdate, const refdef_s *refdef, float zfar)
{
    cameraUpdate->vieworg[0] = refdef->vieworg[0];
    cameraUpdate->vieworg[1] = refdef->vieworg[1];
    cameraUpdate->vieworg[2] = refdef->vieworg[2];
    AxisCopy(refdef->viewaxis, cameraUpdate->viewaxis);
    cameraUpdate->tanHalfFovX = refdef->tanHalfFovX;
    cameraUpdate->tanHalfFovY = refdef->tanHalfFovY;
    cameraUpdate->zfar = zfar;
}

void __cdecl FX_SetExtraCamera(int localClientNum, const FxCameraUpdate *cameraUpdate)
{
    FxSystemContainer *system; // [esp+Ch] [ebp-4h]

    system = FX_GetSystem(localClientNum);
    FX_SetupCamera(
        &system->system.extraCamera,
        cameraUpdate->vieworg,
        cameraUpdate->viewaxis,
        cameraUpdate->tanHalfFovX,
        cameraUpdate->tanHalfFovY,
        cameraUpdate->zfar);
    system->system.extraCamera.isValid = 1;
}

void __cdecl FX_SetupCamera(
                FxCamera *camera,
                const float *vieworg,
                const float (*viewaxis)[3],
                float tanHalfFovX,
                float tanHalfFovY,
                float zfar)
{
    double v6; // st7
    const char *v7; // eax
    float cosHalfFov; // [esp+60h] [ebp-8h]
    float cosHalfFova; // [esp+60h] [ebp-8h]
    unsigned int planeIndex; // [esp+64h] [ebp-4h]

    camera->origin[0] = *vieworg;
    camera->origin[1] = vieworg[1];
    camera->origin[2] = vieworg[2];
    AxisCopy(viewaxis, camera->axis);
    camera->frustum[0][0] = (*viewaxis)[0];
    camera->frustum[0][1] = (*viewaxis)[1];
    camera->frustum[0][2] = (*viewaxis)[2];
    cosHalfFov = 1.0 / sqrtf((float)(tanHalfFovX * tanHalfFovX) + 1.0);
    camera->frustum[1][0] = (float)((float)(tanHalfFovX * cosHalfFov) * (*viewaxis)[0])
                                                + (float)(cosHalfFov * (float)(*viewaxis)[3]);
    camera->frustum[1][1] = (float)((float)(tanHalfFovX * cosHalfFov) * (float)(*viewaxis)[1])
                                                + (float)(cosHalfFov * (float)(*viewaxis)[4]);
    camera->frustum[1][2] = (float)((float)(tanHalfFovX * cosHalfFov) * (float)(*viewaxis)[2])
                                                + (float)(cosHalfFov * (float)(*viewaxis)[5]);
    camera->frustum[2][0] = (float)((float)(tanHalfFovX * cosHalfFov) * (*viewaxis)[0])        + (float)((-(cosHalfFov)) * (float)(*viewaxis)[3]);
    camera->frustum[2][1] = (float)((float)(tanHalfFovX * cosHalfFov) * (float)(*viewaxis)[1]) + (float)((-(cosHalfFov)) * (float)(*viewaxis)[4]);
    camera->frustum[2][2] = (float)((float)(tanHalfFovX * cosHalfFov) * (float)(*viewaxis)[2]) + (float)((-(cosHalfFov)) * (float)(*viewaxis)[5]);
    cosHalfFova = 1.0 / sqrtf((float)(tanHalfFovY * tanHalfFovY) + 1.0);
    camera->frustum[3][0] = (float)((float)(tanHalfFovY * cosHalfFova) * (*viewaxis)[0])
                                                + (float)(cosHalfFova * (float)(*viewaxis)[6]);
    camera->frustum[3][1] = (float)((float)(tanHalfFovY * cosHalfFova) * (float)(*viewaxis)[1])
                                                + (float)(cosHalfFova * (float)(*viewaxis)[7]);
    camera->frustum[3][2] = (float)((float)(tanHalfFovY * cosHalfFova) * (float)(*viewaxis)[2])
                                                + (float)(cosHalfFova * (float)(*viewaxis)[8]);
    camera->frustum[4][0] = (float)((float)(tanHalfFovY * cosHalfFova) * (*viewaxis)[0]) +        (float)((-(cosHalfFova)) * (float)(*viewaxis)[6]);
camera->frustum[4][1] = (float)((float)(tanHalfFovY * cosHalfFova) * (float)(*viewaxis)[1]) +     (float)((-(cosHalfFova)) * (float)(*viewaxis)[7]);
    camera->frustum[4][2] = (float)((float)(tanHalfFovY * cosHalfFova) * (float)(*viewaxis)[2]) + (float)((-(cosHalfFova)) * (float)(*viewaxis)[8]);
    camera->frustumPlaneCount = 5;
    for ( planeIndex = 0; planeIndex < camera->frustumPlaneCount; ++planeIndex )
    {
        if ( !Vec3IsNormalized(camera->frustum[planeIndex]) )
        {
            v6 = Vec3Length(camera->frustum[planeIndex]);
            v7 = va(
                         "(%g %g %g) len %g",
                         camera->frustum[planeIndex][0],
                         camera->frustum[planeIndex][1],
                         camera->frustum[planeIndex][2],
                         v6);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
                            2737,
                            0,
                            "%s\n\t%s",
                            "Vec3IsNormalized( camera->frustum[planeIndex] )",
                            v7) )
                __debugbreak();
        }
        camera->frustum[planeIndex][3] = (float)((float)(camera->origin[0] * camera->frustum[planeIndex][0])
                                                                                     + (float)(camera->origin[1] * camera->frustum[planeIndex][1]))
                                                                     + (float)(camera->origin[2] * camera->frustum[planeIndex][2]);
    }
    if ( zfar > 0.0 )
    {
        (camera->frustum[5][0]) = -((*viewaxis)[0]);
        (camera->frustum[5][1]) = -((*viewaxis)[1]);
        (camera->frustum[5][2]) = -((*viewaxis)[2]);
        camera->frustum[5][3] = (-(camera->frustum[0][3])) - zfar;
        camera->frustumPlaneCount = 6;
    }
}

void __cdecl FX_SetNextUpdateCamera(int localClientNum, const FxCameraUpdate *cameraUpdate)
{
    FxSystemContainer *system; // [esp+Ch] [ebp-4h]

    system = FX_GetSystem(localClientNum);
    FX_SetupCamera(
        &system->system.camera,
        cameraUpdate->vieworg,
        cameraUpdate->viewaxis,
        cameraUpdate->tanHalfFovX,
        cameraUpdate->tanHalfFovY,
        cameraUpdate->zfar);
    _InterlockedExchange(&system->system.camera.isValid, 1);
}

void __cdecl FX_SetNextUpdateTime(int localClientNum, int time)
{
    FxSystemContainer *system; // [esp+0h] [ebp-4h]

    system = FX_GetSystem(localClientNum);
    if ( !FX_IsDemoPlaying()
        && time < system->system.msecNow
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update.cpp",
                    2790,
                    0,
                    "time >= system->msecNow\n\t%i, %i",
                    time,
                    system->system.msecNow) )
    {
        __debugbreak();
    }
    _InterlockedExchange(&system->system.camera.isValid, 0);
    _InterlockedExchange(&system->system.msecDraw, time);
    system->system.msecNow = time;
    if ( ++system->system.frameCount <= 0 )
        system->system.frameCount = 1;
}

void __cdecl FX_FillUpdateCmd(int localClientNum, FxCmd *cmd)
{
    cmd->system = (FxSystem *)FX_GetSystem(localClientNum);
    cmd->localClientNum = localClientNum;
    cmd->useReflection = R_WaterSimulationGetAverageHeight(&cmd->reflectionHeight);
    cmd->updateCameraType = 0;
}

