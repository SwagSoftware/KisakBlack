#include "fx_convert.h"
#include <gfx_d3d/r_material_load_obj.h>
#include "FxCurve.h"
#include <qcommon/common.h>

float4 NATIVE_VERTEX_PERM = { { 3.972236e-34, 0.0, 0.0, 0.0 } };



const FxEffectDef *__cdecl FX_Convert(const FxEditorEffectDef *editorEffect, void *(__cdecl *Alloc)(int))
{
    PhysPreset *v2; // eax
    int v4; // eax
    int v5; // eax
    int v6; // eax
    int v7; // eax
    int v8; // eax
    int v9; // eax
    int v10; // eax
    int v11; // eax
    int LoopingLife; // eax
    float v13; // [esp+0h] [ebp-234h]
    char v14; // [esp+17h] [ebp-21Dh]
    char *name; // [esp+1Ch] [ebp-218h]
    const FxEditorEffectDef *v16; // [esp+20h] [ebp-214h]
    float *v17; // [esp+34h] [ebp-200h]
    float *v18; // [esp+38h] [ebp-1FCh]
    float v19; // [esp+40h] [ebp-1F4h]
    float *boundingSphere; // [esp+4Ch] [ebp-1E8h]
    float *boundingBoxDim; // [esp+54h] [ebp-1E0h]
    int i; // [esp+64h] [ebp-1D0h]
    const FxElemDef *elemDef; // [esp+68h] [ebp-1CCh]
    int intervalLimit; // [esp+6Ch] [ebp-1C8h]
    int elemCountTotal; // [esp+70h] [ebp-1C4h]
    int velStateCount[33]; // [esp+74h] [ebp-1C0h] BYREF
    FxEffectDef *effect; // [esp+F8h] [ebp-13Ch]
    int effectFlags; // [esp+FCh] [ebp-138h]
    int visualIndex; // [esp+100h] [ebp-134h]
    int emitIndex[32]; // [esp+104h] [ebp-130h] BYREF
    int firstEmitted; // [esp+184h] [ebp-B0h]
    int totalBytesNeeded; // [esp+188h] [ebp-ACh]
    int visStateCount[33]; // [esp+18Ch] [ebp-A8h] BYREF
    const FxEditorElemDef *edElemDef; // [esp+210h] [ebp-24h]
    int elemIndex; // [esp+214h] [ebp-20h]
    unsigned __int8 *memPool; // [esp+218h] [ebp-1Ch] BYREF
    FxSampleChannel routing[5]; // [esp+21Ch] [ebp-18h] BYREF
    const FxElemVisuals *elemVisual; // [esp+230h] [ebp-4h]

    if ( !editorEffect
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp",
                    1533,
                    0,
                    "%s",
                    "editorEffect") )
    {
        __debugbreak();
    }
    memset(emitIndex, 0xFFu, sizeof(emitIndex));
    totalBytesNeeded = 292 * editorEffect->elemCount + 60;
    elemCountTotal = editorEffect->elemCount;
    for ( elemIndex = 0; elemIndex < editorEffect->elemCount; ++elemIndex )
    {
        edElemDef = &editorEffect->elems[elemIndex];
        if ( editorEffect->elems[elemIndex].elemType == 7 && (edElemDef->flags & 0x8000000) != 0 )
        {
            for ( visualIndex = 0; visualIndex < edElemDef->visualCount; ++visualIndex )
            {
                elemVisual = &edElemDef->visuals[visualIndex];
                if ( elemVisual->anonymous )
                {
                    if ( !*((unsigned int *)elemVisual->anonymous + 59) )
                    {
                        v2 = FX_RegisterPhysPreset("default");
                        *((unsigned int *)elemVisual->anonymous + 59) = (unsigned int)v2;
                        Com_PrintError(
                            20,
                            "ERROR: no physics preset specified for the FX model [%s]\n",
                            *(const char **)elemVisual->anonymous);
                    }
                }
            }
        }
    }
    for ( elemIndex = 0; elemIndex < editorEffect->elemCount; ++elemIndex )
    {
        edElemDef = &editorEffect->elems[elemIndex];
        if ( !FX_Validate(editorEffect, edElemDef) )
            return 0;
        FX_GetVisualSampleRouting(edElemDef, routing);
        intervalLimit = FX_DecideIntervalLimit(edElemDef);
        v4 = FX_DecideVelocitySampleCount(edElemDef, intervalLimit);
        velStateCount[elemIndex] = v4;
        v5 = FX_DecideVisualSampleCount(edElemDef, routing, intervalLimit);
        visStateCount[elemIndex] = v5;
        v6 = FX_AdditionalBytesNeededForElemDef(
                     edElemDef->elemType,
                     velStateCount[elemIndex],
                     visStateCount[elemIndex],
                     edElemDef->visualCount,
                     &edElemDef->spawnSound);
        totalBytesNeeded += v6;
        v7 = FX_AdditionalBytesNeededForGeomTrail(edElemDef);
        totalBytesNeeded += v7;
        if ( edElemDef->emission && edElemDef->emission->elemDefCountOneShot )
        {
            firstEmitted = FX_FindEmission(edElemDef->emission, editorEffect);
            if ( firstEmitted == elemIndex )
            {
                emitIndex[elemIndex] = elemCountTotal;
                elemCountTotal += edElemDef->emission->elemDefCountOneShot;
                v8 = FX_AdditionalBytesNeededForEmission(edElemDef->emission);
                totalBytesNeeded += v8;
            }
            else
            {
                emitIndex[elemIndex] = emitIndex[firstEmitted];
            }
        }
    }
    totalBytesNeeded += strlen(editorEffect->name) + 1;
    effect = (FxEffectDef *)Alloc(totalBytesNeeded);
    memPool = (unsigned __int8 *)&effect[1];
    effect->elemDefsEA = (const FxElemDef *)&effect[1];
    memPool += 292 * elemCountTotal;
    v9 = FX_ConvertElemDefsOfType((FxElemDef*)effect->elemDefsEA, editorEffect, 1u, velStateCount, visStateCount, emitIndex, &memPool);
    effect->elemDefCountLooping = v9;
    v10 = FX_ConvertElemDefsOfType(
        (FxElemDef *)&effect->elemDefsEA[effect->elemDefCountLooping],
                    editorEffect,
                    0,
                    velStateCount,
                    visStateCount,
                    emitIndex,
                    &memPool);
    effect->elemDefCountOneShot = v10;
    v11 = FX_CopyEmittedElemDefs(
        (FxElemDef *)&effect->elemDefsEA[effect->elemDefCountOneShot + effect->elemDefCountLooping],
                    editorEffect,
                    &memPool);
    effect->elemDefCountEmission = v11;
    if ( effect->elemDefCountEmission + effect->elemDefCountOneShot + effect->elemDefCountLooping != elemCountTotal
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp",
                    1619,
                    1,
                    "effect->elemDefCountLooping + effect->elemDefCountOneShot + effect->elemDefCountEmission == elemCountTotal\n"
                    "\t%i, %i",
                    effect->elemDefCountEmission + effect->elemDefCountOneShot + effect->elemDefCountLooping,
                    elemCountTotal) )
    {
        __debugbreak();
    }
    effect->efPriority = editorEffect->edPriority;
    effectFlags = editorEffect->flags & 0xFC;
    for ( elemIndex = 0; elemIndex != elemCountTotal; ++elemIndex )
    {
        if ( (float)effect->elemDefsEA[elemIndex].lightingFrac != 0.0 )
        {
            effectFlags |= 1u;
            break;
        }
    }
    for ( elemIndex = 0; elemIndex != elemCountTotal; ++elemIndex )
    {
        elemDef = &effect->elemDefsEA[elemIndex];
        if ( elemDef->elemType == 11 )
        {
            for ( i = 0; i < elemDef->visualCount; ++i )
            {
                if ( Material_HasImpactMaskTech(elemDef->visuals.markArray[i].materials[1]) )
                {
                    effectFlags |= 2u;
                    break;
                }
            }
            if ( (effectFlags & 2) != 0 )
                break;
        }
    }
    effect->flags = effectFlags;
    LoopingLife = FX_GetLoopingLife(effect);
    effect->msecLoopingLife = LoopingLife;
    effect->totalSize = totalBytesNeeded;
    effect->name = (const char *)memPool;
    v16 = editorEffect;
    name = (char *)effect->name;
    do
    {
        v14 = v16->name[0];
        *name = v16->name[0];
        v16 = (const FxEditorEffectDef *)((char *)v16 + 1);
        ++name;
    }
    while ( v14 );
    memPool += &effect->name[strlen(effect->name) + 1] - effect->name;
    if ( memPool - (unsigned __int8 *)effect != effect->totalSize
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp",
                    1663,
                    1,
                    "memPool - reinterpret_cast< byte * >( effect ) == effect->totalSize\n\t%i, %i",
                    memPool - (unsigned __int8 *)effect,
                    effect->totalSize) )
    {
        __debugbreak();
    }
    if ( (editorEffect->flags & 4) != 0 )
    {
        boundingBoxDim = effect->boundingBoxDim;
        effect->boundingBoxDim[0] = editorEffect->boundingBoxDim[0];
        boundingBoxDim[1] = editorEffect->boundingBoxDim[1];
        boundingBoxDim[2] = editorEffect->boundingBoxDim[2];
        boundingSphere = effect->boundingSphere;
        effect->boundingSphere[0] = editorEffect->boundingBoxCentre[0];
        boundingSphere[1] = editorEffect->boundingBoxCentre[1];
        boundingSphere[2] = editorEffect->boundingBoxCentre[2];
        if ( (float)(editorEffect->boundingBoxDim[1] - editorEffect->boundingBoxDim[2]) < 0.0 )
            v19 = editorEffect->boundingBoxDim[2];
        else
            v19 = editorEffect->boundingBoxDim[1];
        if ( (float)(editorEffect->boundingBoxDim[0] - v19) < 0.0 )
            v13 = v19;
        else
            v13 = editorEffect->boundingBoxDim[0];
        effect->boundingSphere[3] = 1.4142137 * v13;
    }
    else
    {
        v18 = effect->boundingBoxDim;
        effect->boundingBoxDim[0] = 0.0f;
        v18[1] = 0.0f;
        v18[2] = 0.0f;
        v17 = effect->boundingSphere;
        effect->boundingSphere[0] = 0.0f;
        v17[1] = 0.0f;
        v17[2] = 0.0f;
        v17[3] = 0.0f;
    }
    return effect;
}

int __cdecl FX_DecideVelocitySampleCount(const FxEditorElemDef *edElem, int intervalLimit)
{
    const FxCurve *curves[12]; // [esp+0h] [ebp-30h] BYREF

    *(_QWORD *)curves = *(_QWORD *)&edElem->velShape[0][0][0];
    *(_QWORD *)&curves[2] = *(_QWORD *)&edElem->velShape[0][1][0];
    *(_QWORD *)&curves[4] = *(_QWORD *)&edElem->velShape[0][2][0];
    *(_QWORD *)&curves[6] = *(_QWORD *)&edElem->velShape[1][0][0];
    *(_QWORD *)&curves[8] = *(_QWORD *)&edElem->velShape[1][1][0];
    *(_QWORD *)&curves[10] = *(_QWORD *)&edElem->velShape[1][2][0];
    return FX_DecideSampleCount(12, curves, intervalLimit);
}

int __cdecl FX_DecideSampleCount(int curveCount, const FxCurve **curves, int intervalLimit)
{
    int intervalCountBest; // [esp+4h] [ebp-18h]
    float errorBest; // [esp+8h] [ebp-14h]
    float error; // [esp+Ch] [ebp-10h]
    int intervalCount; // [esp+10h] [ebp-Ch]
    int curveIndex; // [esp+14h] [ebp-8h]
    float errorCumulative; // [esp+18h] [ebp-4h]

    errorBest = FLT_MAX;
    intervalCountBest = 1;
    for ( intervalCount = 1; intervalCount <= intervalLimit; ++intervalCount )
    {
        errorCumulative = 0.0f;
        for ( curveIndex = 0; curveIndex < curveCount; ++curveIndex )
        {
            error = FX_MaxErrorForIntervalCount(
                                curves[curveIndex]->dimensionCount,
                                curves[curveIndex]->keyCount,
                                curves[curveIndex]->keys,
                                intervalCount,
                                errorBest);
            if ( error > errorCumulative )
            {
                errorCumulative = error;
                if ( error > errorBest )
                    break;
            }
        }
        if ( errorBest > errorCumulative )
        {
            intervalCountBest = intervalCount;
            errorBest = errorCumulative - 0.02;
            if ( (float)(errorCumulative - 0.02) <= 0.0 )
                break;
        }
    }
    return intervalCountBest + 1;
}

double __cdecl FX_MaxErrorForIntervalCount(
                int dimensions,
                int sampleCount,
                const float *samples,
                int intervalCount,
                float errorCutoff)
{
    float timeNext; // [esp+1Ch] [ebp-54h]
    float lerpedValueIter[3]; // [esp+20h] [ebp-50h] BYREF
    float timePrev; // [esp+2Ch] [ebp-44h]
    const float *samplesTo; // [esp+30h] [ebp-40h]
    int componentIndex; // [esp+34h] [ebp-3Ch]
    int intervalIndex; // [esp+38h] [ebp-38h]
    int sampleIndexPrev; // [esp+3Ch] [ebp-34h]
    int sampleIndexIter; // [esp+40h] [ebp-30h]
    float error; // [esp+44h] [ebp-2Ch]
    float errorMax; // [esp+48h] [ebp-28h]
    int sampleIndexNext; // [esp+4Ch] [ebp-24h]
    float lerpedValueNext[3]; // [esp+50h] [ebp-20h] BYREF
    int componentCount; // [esp+5Ch] [ebp-14h]
    float lerpedValuePrev[3]; // [esp+60h] [ebp-10h] BYREF
    const float *samplesFrom; // [esp+6Ch] [ebp-4h]

    if ( dimensions <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp",
                    98,
                    0,
                    "%s\n\t(dimensions) = %i",
                    "(dimensions > 0)",
                    dimensions) )
    {
        __debugbreak();
    }
    if ( sampleCount <= 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp",
                    99,
                    0,
                    "%s\n\t(sampleCount) = %i",
                    "(sampleCount > 1)",
                    sampleCount) )
    {
        __debugbreak();
    }
    if ( !samples
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp", 100, 0, "%s", "samples") )
    {
        __debugbreak();
    }
    componentCount = dimensions + 1;
    errorMax = 0.0f;
    timePrev = 0.0f;
    sampleIndexPrev = 1;
    for ( componentIndex = 1; componentIndex != componentCount; ++componentIndex )
        lerpedValuePrev[componentIndex - 1] = samples[componentIndex];
    for ( intervalIndex = 1; intervalIndex <= intervalCount; ++intervalIndex )
    {
        for ( sampleIndexNext = sampleIndexPrev;
                    (float)intervalIndex > (float)((float)intervalCount * samples[componentCount * sampleIndexNext]);
                    ++sampleIndexNext )
        {
            ;
        }
        timeNext = (float)intervalIndex / (float)intervalCount;
        samplesTo = &samples[componentCount * sampleIndexNext];
        samplesFrom = &samplesTo[-componentCount];
        FX_InterpolateSamples(
            dimensions,
            *samplesFrom,
            samplesFrom + 1,
            *samplesTo,
            samplesTo + 1,
            timeNext,
            lerpedValueNext);
        for ( sampleIndexIter = sampleIndexPrev; sampleIndexIter < sampleIndexNext; ++sampleIndexIter )
        {
            FX_InterpolateSamples(
                dimensions,
                timePrev,
                lerpedValuePrev,
                timeNext,
                lerpedValueNext,
                samples[componentCount * sampleIndexIter],
                lerpedValueIter);
            for ( componentIndex = 1; componentIndex < componentCount; ++componentIndex )
            {
                //LODWORD(error) = COERCE_UNSIGNED_INT(samples[componentIndex + componentCount * sampleIndexIter] - lerpedValueIter[componentIndex - 1]) & _mask__AbsFloat_;
                error = -(samples[componentIndex + componentCount * sampleIndexIter] - lerpedValueIter[componentIndex - 1]);
                if ( error > errorMax )
                {
                    errorMax = error;
                    if ( error > errorCutoff )
                        return errorMax;
                }
            }
        }
        for ( componentIndex = 1; componentIndex != componentCount; ++componentIndex )
            lerpedValuePrev[componentIndex - 1] = lerpedValueNext[componentIndex - 1];
        sampleIndexPrev = sampleIndexNext;
        timePrev = timeNext;
    }
    return errorMax;
}

void __cdecl FX_InterpolateSamples(
                int dimensions,
                float time0,
                const float *samples0,
                float time1,
                const float *samples1,
                float timeEval,
                float *result)
{
    int dimIndex; // [esp+20h] [ebp-4h]

    if ( dimensions <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp",
                    44,
                    0,
                    "%s\n\t(dimensions) = %i",
                    "(dimensions > 0)",
                    dimensions) )
    {
        __debugbreak();
    }
    if ( time1 <= time0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp",
                    45,
                    0,
                    "time0 < time1\n\t%g, %g",
                    time0,
                    time1) )
    {
        __debugbreak();
    }
    if ( (timeEval < time0 || time1 < timeEval)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp",
                    46,
                    0,
                    "timeEval not in [time0, time1]\n\t%g not in [%g, %g]",
                    timeEval,
                    time0,
                    time1) )
    {
        __debugbreak();
    }
    if ( !samples0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp", 47, 0, "%s", "samples0") )
    {
        __debugbreak();
    }
    if ( !samples1
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp", 48, 0, "%s", "samples1") )
    {
        __debugbreak();
    }
    if ( !result
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp", 49, 0, "%s", "result") )
    {
        __debugbreak();
    }
    for ( dimIndex = 0; dimIndex < dimensions; ++dimIndex )
        result[dimIndex] = (float)((float)(samples1[dimIndex] - samples0[dimIndex])
                                                         * (float)((float)(timeEval - time0) / (float)(time1 - time0)))
                                         + samples0[dimIndex];
}

void __cdecl FX_GetVisualSampleRouting(const FxEditorElemDef *edElem, FxSampleChannel *routing)
{
    int v2; // [esp+0h] [ebp-8h]

    switch ( edElem->elemType )
    {
        case 0u:
        case 1u:
        case 2u:
        case 3u:
        case 4u:
        case 5u:
            *routing = FX_CHAN_RGBA;
            *((unsigned int *)routing + 1) = 1;
            if ( edElem->elemType == 3 || edElem->elemType == 4 || (edElem->flags & 0x10000000) != 0 )
                v2 = 2;
            else
                v2 = 1;
            *((unsigned int *)routing + 2) = v2;
            *((unsigned int *)routing + 4) = 4;
            *((unsigned int *)routing + 3) = 6;
            break;
        case 6u:
            *routing = FX_CHAN_RGBA;
            *((unsigned int *)routing + 1) = 1;
            *((unsigned int *)routing + 2) = 2;
            *((unsigned int *)routing + 4) = 6;
            *((unsigned int *)routing + 3) = 3;
            break;
        case 7u:
        case 0xAu:
            *routing = FX_CHAN_NONE;
            *((unsigned int *)routing + 1) = 6;
            *((unsigned int *)routing + 2) = 6;
            *((unsigned int *)routing + 4) = 6;
            *((unsigned int *)routing + 3) = 3;
            break;
        case 8u:
            *routing = FX_CHAN_RGBA;
            *((unsigned int *)routing + 1) = 1;
            *((unsigned int *)routing + 2) = 6;
            *((unsigned int *)routing + 4) = 6;
            *((unsigned int *)routing + 3) = 6;
            break;
        case 9u:
        case 0xBu:
            *routing = FX_CHAN_RGBA;
            *((unsigned int *)routing + 1) = 1;
            *((unsigned int *)routing + 2) = 6;
            *((unsigned int *)routing + 4) = 4;
            *((unsigned int *)routing + 3) = 6;
            break;
        case 0xCu:
            *routing = FX_CHAN_NONE;
            *((unsigned int *)routing + 1) = 6;
            *((unsigned int *)routing + 2) = 6;
            *((unsigned int *)routing + 4) = 6;
            *((unsigned int *)routing + 3) = 6;
            break;
        default:
            return;
    }
}

int __cdecl FX_DecideVisualSampleCount(
                const FxEditorElemDef *edElem,
                const FxSampleChannel *routing,
                int intervalLimit)
{
    unsigned int curveCount; // [esp+4h] [ebp-38h]
    unsigned int curveCounta; // [esp+4h] [ebp-38h]
    const FxCurve *curves[12]; // [esp+8h] [ebp-34h] BYREF
    unsigned int chanIndex; // [esp+38h] [ebp-4h]

    curveCount = 0;
    for ( chanIndex = 0; chanIndex < 5; ++chanIndex )
    {
        switch ( routing[chanIndex] )
        {
            case FX_CHAN_RGBA:
                curves[curveCount] = edElem->color[0];
                curveCounta = curveCount + 1;
                if ( (edElem->editorFlags & 2) != 0 )
                    curves[curveCounta++] = edElem->color[1];
                curves[curveCounta] = edElem->alpha[0];
                curveCount = curveCounta + 1;
                if ( (edElem->editorFlags & 4) != 0 )
                    curves[curveCount++] = edElem->alpha[1];
                break;
            case FX_CHAN_SIZE_0:
                curves[curveCount++] = edElem->sizeShape[0][0];
                if ( (edElem->editorFlags & 8) != 0 )
                    curves[curveCount++] = edElem->sizeShape[0][1];
                break;
            case FX_CHAN_SIZE_1:
                curves[curveCount++] = edElem->sizeShape[1][0];
                if ( (edElem->editorFlags & 0x10) != 0 )
                    curves[curveCount++] = edElem->sizeShape[1][1];
                break;
            case FX_CHAN_SCALE:
                curves[curveCount++] = edElem->scaleShape[0];
                if ( (edElem->editorFlags & 0x20) != 0 )
                    curves[curveCount++] = edElem->scaleShape[1];
                break;
            case FX_CHAN_ROTATION:
                curves[curveCount++] = edElem->rotationShape[0];
                if ( (edElem->editorFlags & 0x40) != 0 )
                    curves[curveCount++] = edElem->rotationShape[1];
                break;
            default:
                if ( routing[chanIndex] != FX_CHAN_NONE
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp",
                                333,
                                0,
                                "%s",
                                "routing[chanIndex] == FX_CHAN_NONE") )
                {
                    __debugbreak();
                }
                break;
        }
    }
    if ( curveCount > 0xC
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp",
                    338,
                    0,
                    "curveCount not in [0, ARRAY_COUNT( curves )]\n\t%i not in [%i, %i]",
                    curveCount,
                    0,
                    12) )
    {
        __debugbreak();
    }
    if ( curveCount )
        return FX_DecideSampleCount(curveCount, curves, intervalLimit);
    else
        return 0;
}

int __cdecl FX_DecideIntervalLimit(const FxEditorElemDef *edElemDef)
{
    int intervalLimit; // [esp+0h] [ebp-8h]

    intervalLimit = (edElemDef->lifeSpanMsec.base + edElemDef->lifeSpanMsec.amplitude / 2) / 100;
    if ( intervalLimit > 80 )
        return 80;
    return intervalLimit;
}

int __cdecl FX_AdditionalBytesNeededForGeomTrail(const FxEditorElemDef *elemDef)
{
    if ( elemDef->elemType == 5 )
        return 22 * elemDef->trailDef.indCount + 28;
    else
        return 0;
}

int __cdecl FX_FindEmission(const FxEffectDef *emission, const FxEditorEffectDef *editorEffect)
{
    int elemIndex; // [esp+0h] [ebp-4h]

    if ( !editorEffect
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp",
                    1203,
                    0,
                    "%s",
                    "editorEffect") )
    {
        __debugbreak();
    }
    if ( editorEffect == (const FxEditorEffectDef *)-96
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp",
                    1204,
                    0,
                    "%s",
                    "editorEffect->elems") )
    {
        __debugbreak();
    }
    if ( !editorEffect->elemCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp",
                    1205,
                    0,
                    "%s",
                    "editorEffect->elemCount") )
    {
        __debugbreak();
    }
    for ( elemIndex = 0; elemIndex < editorEffect->elemCount; ++elemIndex )
    {
        if ( emission == editorEffect->elems[elemIndex].emission )
            return elemIndex;
    }
    if ( !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp",
                    1218,
                    1,
                    "emission not in editor effect") )
        __debugbreak();
    return -1;
}

int __cdecl FX_ConvertElemDefsOfType(
                FxElemDef *elemDefArray,
                const FxEditorEffectDef *editorEffect,
                unsigned int loopingFlagState,
                const int *velStateCount,
                const int *visStateCount,
                const int *emitIndex,
                unsigned __int8 **memPool)
{
    FxElemDef *elemDef; // [esp+0h] [ebp-Ch]
    int elemIndex; // [esp+4h] [ebp-8h]
    int elemCount; // [esp+8h] [ebp-4h]

    if ( !elemDefArray
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp",
                    1229,
                    0,
                    "%s",
                    "elemDefArray") )
    {
        __debugbreak();
    }
    if ( !editorEffect
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp",
                    1230,
                    0,
                    "%s",
                    "editorEffect") )
    {
        __debugbreak();
    }
    if ( loopingFlagState >= 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp",
                    1231,
                    0,
                    "%s\n\t(loopingFlagState) = %i",
                    "(loopingFlagState == FX_ED_FLAG_LOOPING || loopingFlagState == 0)",
                    loopingFlagState) )
    {
        __debugbreak();
    }
    if ( !velStateCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp",
                    1232,
                    0,
                    "%s",
                    "velStateCount") )
    {
        __debugbreak();
    }
    if ( !visStateCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp",
                    1233,
                    0,
                    "%s",
                    "visStateCount") )
    {
        __debugbreak();
    }
    elemCount = 0;
    for ( elemIndex = 0; elemIndex < editorEffect->elemCount; ++elemIndex )
    {
        if ( (editorEffect->elems[elemIndex].editorFlags & 1) == loopingFlagState )
        {
            elemDef = &elemDefArray[elemCount++];
            FX_ConvertElemDef(
                elemDef,
                &editorEffect->elems[elemIndex],
                velStateCount[elemIndex],
                visStateCount[elemIndex],
                emitIndex[elemIndex],
                memPool);
        }
    }
    return elemCount;
}

void __cdecl FX_CopyCanonicalRange_FxIntRange_FxIntRange_(FxIntRange *to, const FxIntRange *from)
{
    if (from->amplitude >= 0)
    {
        *to = *from;
    }
    else
    {
        to->base = from->amplitude + from->base;
        to->amplitude = -from->amplitude;
    }
}

void __cdecl FX_CopyCanonicalRange_FxFloatRange_FxFloatRange_(FxFloatRange *to, const FxFloatRange *from)
{
    if (from->amplitude >= 0.0)
    {
        *to = *from;
    }
    else
    {
        to->base = from->base + from->amplitude;
        to->amplitude = -(from->amplitude);
    }
}

void __cdecl FX_ConvertElemDef(
                FxElemDef *elemDef,
                const FxEditorElemDef *edElemDef,
                int velStateCount,
                int visStateCount,
                int emitIndex,
                unsigned __int8 **memPool)
{
    int count; // eax
    int amplitude; // edx
    unsigned int v8; // eax
    int v9; // [esp+8h] [ebp-90h]
    int v10; // [esp+Ch] [ebp-8Ch]
    unsigned __int8 v11; // [esp+10h] [ebp-88h]
    unsigned __int8 v12; // [esp+28h] [ebp-70h]
    float billboardBottomWidth; // [esp+30h] [ebp-68h]
    float v14; // [esp+34h] [ebp-64h]
    float billboardTopWidth; // [esp+38h] [ebp-60h]
    float v16; // [esp+3Ch] [ebp-5Ch]
    int sortOrder; // [esp+44h] [ebp-54h]
    float v18; // [esp+58h] [ebp-40h]
    float v19; // [esp+5Ch] [ebp-3Ch]
    float v20; // [esp+68h] [ebp-30h]
    float v21; // [esp+6Ch] [ebp-2Ch]
    float in[4]; // [esp+74h] [ebp-24h] BYREF
    float ident[4]; // [esp+84h] [ebp-14h] BYREF
    bool notApplyRotationAxis; // [esp+97h] [ebp-1h]

    if ( !elemDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp", 1005, 0, "%s", "elemDef") )
    {
        __debugbreak();
    }
    if ( !edElemDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp", 1006, 0, "%s", "edElemDef") )
    {
        __debugbreak();
    }
    if ( velStateCount < 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp",
                    1007,
                    0,
                    "%s\n\t(velStateCount) = %i",
                    "(velStateCount >= 2)",
                    velStateCount) )
    {
        __debugbreak();
    }
    if ( visStateCount
        && visStateCount < 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp",
                    1008,
                    0,
                    "%s\n\t(visStateCount) = %i",
                    "(visStateCount == 0 || visStateCount >= 2)",
                    visStateCount) )
    {
        __debugbreak();
    }
    elemDef->flags = edElemDef->flags;
    if ( 0.0 == (float)((float)(edElemDef->billboardPivot[0] * edElemDef->billboardPivot[0])
                                        + (float)(edElemDef->billboardPivot[1] * edElemDef->billboardPivot[1])) )
        elemDef->flags &= ~0x200000u;
    FX_CopyCanonicalRange_FxFloatRange_FxFloatRange_(&elemDef->spawnRange, &edElemDef->spawnRange);
    FX_CopyCanonicalRange_FxFloatRange_FxFloatRange_(&elemDef->fadeInRange, &edElemDef->fadeInRange);
    FX_CopyCanonicalRange_FxFloatRange_FxFloatRange_(&elemDef->fadeOutRange, &edElemDef->fadeOutRange);
    elemDef->spawnFrustumCullRadius = edElemDef->spawnFrustumCullRadius;
    if ( (edElemDef->editorFlags & 1) != 0 )
    {
        count = edElemDef->spawnLooping.count;
        elemDef->spawn.looping.intervalMsec = edElemDef->spawnLooping.intervalMsec;
        elemDef->spawn.looping.count = count;
        if ( !elemDef->spawn.looping.count )
            elemDef->spawn.looping.count = 0x7FFFFFFF;
        if ( elemDef->spawn.looping.intervalMsec <= 0 )
            elemDef->spawn.looping.intervalMsec = 1;
    }
    else
    {
        amplitude = edElemDef->spawnOneShot.count.amplitude;
        elemDef->spawn.looping.intervalMsec = edElemDef->spawnOneShot.count.base;
        elemDef->spawn.looping.count = amplitude;
    }
    FX_CopyCanonicalRange_FxIntRange_FxIntRange_(&elemDef->spawnDelayMsec, &edElemDef->spawnDelayMsec);
    FX_CopyCanonicalRange_FxIntRange_FxIntRange_(&elemDef->lifeSpanMsec, &edElemDef->lifeSpanMsec);
    FX_CopyCanonicalRange_FxFloatRange_FxFloatRange_(elemDef->spawnOrigin, edElemDef->spawnOrigin);
    FX_CopyCanonicalRange_FxFloatRange_FxFloatRange_(&elemDef->spawnOrigin[1], &edElemDef->spawnOrigin[1]);
    FX_CopyCanonicalRange_FxFloatRange_FxFloatRange_(&elemDef->spawnOrigin[2], &edElemDef->spawnOrigin[2]);
    FX_CopyCanonicalRange_FxFloatRange_FxFloatRange_(&elemDef->spawnOffsetRadius, &edElemDef->spawnOffsetRadius);
    FX_CopyCanonicalRange_FxFloatRange_FxFloatRange_(&elemDef->spawnOffsetHeight, &edElemDef->spawnOffsetHeight);
    FX_ScaleFloatRange(elemDef->spawnAngles, edElemDef->spawnAngles, 0.017453292);
    FX_ScaleFloatRange(&elemDef->spawnAngles[1], &edElemDef->spawnAngles[1], 0.017453292);
    FX_ScaleFloatRange(&elemDef->spawnAngles[2], &edElemDef->spawnAngles[2], 0.017453292);
    FX_ScaleFloatRange(elemDef->angularVelocity, edElemDef->angularVelocity, 0.000017453294);
    FX_ScaleFloatRange(&elemDef->angularVelocity[1], &edElemDef->angularVelocity[1], 0.000017453294);
    FX_ScaleFloatRange(&elemDef->angularVelocity[2], &edElemDef->angularVelocity[2], 0.000017453294);
    FX_ScaleFloatRange(&elemDef->initialRotation, &edElemDef->initialRotation, 0.017453292);
    notApplyRotationAxis = 0;
    if ( !edElemDef->elemType || edElemDef->elemType == 3 || edElemDef->elemType == 4 )
        notApplyRotationAxis = 1;
    if ( notApplyRotationAxis )
    {
        memset(ident, 0, 12);
        ident[3] = 1.0f;
        elemDef->rotationAxis = Vec4PackQuat(ident);
    }
    else
    {
        if ( (0x800000 & edElemDef->flags) != 0 )
        {
            v8 = Vec4PackQuat(edElemDef->rotationAxis);
        }
        else
        {
            memset(in, 0, 12);
            in[3] = 1.0f;
            v8 = Vec4PackQuat(in);
        }
        elemDef->rotationAxis = v8;
    }
    FX_ScaleFloatRange(&elemDef->gravity, &edElemDef->gravity, 0.0099999998);
    if ( elemDef->gravity.base != 0.0 || elemDef->gravity.amplitude != 0.0 )
        elemDef->flags |= 0x4000000u;
    FX_CopyCanonicalRange_FxFloatRange_FxFloatRange_(&elemDef->reflectionFactor, &edElemDef->elasticity);
    FX_BoundFloatRange(&elemDef->reflectionFactor, 0.0, 1.0);
    FX_ConvertAtlas(elemDef, edElemDef);
    elemDef->windInfluence = edElemDef->windInfluence;
    elemDef->elemType = edElemDef->elemType;
    elemDef->visualCount = edElemDef->visualCount;
    elemDef->velIntervalCount = velStateCount - 1;
    if ( visStateCount )
        v12 = visStateCount - 1;
    else
        v12 = 0;
    elemDef->visStateIntervalCount = v12;
    FX_ReserveElemDefMemory(elemDef, memPool);
    FX_SampleVelocity(elemDef, edElemDef);
    if ( visStateCount )
        FX_SampleVisualState(elemDef, edElemDef);
    if ( elemDef->elemType == 11 )
    {
        FX_CopyMarkVisuals(edElemDef, elemDef->visuals.markArray);
    }
    else if ( elemDef->visualCount )
    {
        if ( elemDef->visualCount == 1 )
            FX_CopyVisuals(edElemDef, (FxElemVisuals *)&elemDef->visuals);
        else
            FX_CopyVisuals(edElemDef, elemDef->visuals.array);
    }
    else
    {
        elemDef->visuals.markArray = 0;
        if ( elemDef->elemType == 8 || elemDef->elemType == 9 )
        {
            elemDef->visualCount = 1;
        }
        else if ( elemDef->elemType == 7 )
        {
            elemDef->flags &= ~0x8000000u;
        }
    }
    if ( edElemDef->lightingFrac > 255.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp",
                    1130,
                    0,
                    "%s\n\t(edElemDef->lightingFrac) = %g",
                    "(edElemDef->lightingFrac <= 255.0f)",
                    edElemDef->lightingFrac) )
    {
        __debugbreak();
    }
    if ( (int)(float)(edElemDef->lightingFrac * 255.0) >= 256
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp",
                    1131,
                    0,
                    "%s\n\t(edElemDef->lightingFrac) = %g",
                    "(static_cast< int >( edElemDef->lightingFrac * 255.0f ) < 256)",
                    edElemDef->lightingFrac) )
    {
        __debugbreak();
    }
    elemDef->lightingFrac = (int)(float)(edElemDef->lightingFrac * 255.0);
    if ( (edElemDef->editorFlags & 0x10000) != 0 )
    {
        v20 = edElemDef->collOffset[1] - edElemDef->collRadius;
        v21 = edElemDef->collOffset[2] - edElemDef->collRadius;
        elemDef->collMins[0] = edElemDef->collOffset[0] - edElemDef->collRadius;
        elemDef->collMins[1] = v20;
        elemDef->collMins[2] = v21;
        v18 = edElemDef->collOffset[1] + edElemDef->collRadius;
        v19 = edElemDef->collOffset[2] + edElemDef->collRadius;
        elemDef->collMaxs[0] = edElemDef->collOffset[0] + edElemDef->collRadius;
        elemDef->collMaxs[1] = v18;
        elemDef->collMaxs[2] = v19;
    }
    else
    {
        elemDef->collMins[0] = 0.0f;
        elemDef->collMins[1] = 0.0f;
        elemDef->collMins[2] = 0.0f;
        elemDef->collMaxs[0] = 0.0f;
        elemDef->collMaxs[1] = 0.0f;
        elemDef->collMaxs[2] = 0.0f;
    }
    if ( (edElemDef->editorFlags & 0x2000) != 0 )
        FX_ConvertEffectDefRef(&elemDef->effectOnImpact, edElemDef->effectOnImpact);
    else
        FX_ConvertEffectDefRef(&elemDef->effectOnImpact, 0);
    if ( (edElemDef->editorFlags & 0x4000) != 0 )
        FX_ConvertEffectDefRef(&elemDef->effectOnDeath, edElemDef->effectOnDeath);
    else
        FX_ConvertEffectDefRef(&elemDef->effectOnDeath, 0);
    if ( (edElemDef->editorFlags & 0x8000) != 0 )
        FX_ConvertEffectDefRef(&elemDef->effectEmitted, edElemDef->emission);
    else
        FX_ConvertEffectDefRef(&elemDef->effectEmitted, 0);
    if ( (edElemDef->editorFlags & 0x40000) != 0 )
        FX_ConvertEffectDefRef(&elemDef->effectAttached, edElemDef->attachment);
    else
        FX_ConvertEffectDefRef(&elemDef->effectAttached, 0);
    FX_CopyCanonicalRange_FxFloatRange_FxFloatRange_(&elemDef->emitDist, &edElemDef->emitDist);
    FX_CopyCanonicalRange_FxFloatRange_FxFloatRange_(&elemDef->emitDistVariance, &edElemDef->emitDistVariance);
    if ( edElemDef->elemType == 5 )
    {
        FX_ConvertTrail(&elemDef->localTrailDef, edElemDef, memPool);
    }
    else if ( edElemDef->elemType == 6 )
    {
        FX_CopyCanonicalRange_FxIntRange_FxIntRange_((FxIntRange *)&elemDef->u, &edElemDef->cloudDensityRange);
    }
    if ( edElemDef->sortOrder < 255 )
        sortOrder = edElemDef->sortOrder;
    else
        sortOrder = 255;
    if ( sortOrder > 0 )
        v11 = sortOrder;
    else
        v11 = 0;
    elemDef->sortOrder = v11;
    elemDef->alphaFadeTimeMsec = edElemDef->alphaFadeTimeMsec;
    if ( (edElemDef->editorFlags & 1) != 0 )
    {
        elemDef->maxWindStrength = edElemDef->maxWindMagnitude;
        elemDef->lifespanAtMaxWind = edElemDef->lifespanAtMaxWind;
        elemDef->spawnIntervalAtMaxWind = edElemDef->spawnIntervalAtMaxWind;
    }
    else
    {
        elemDef->maxWindStrength = 0;
        elemDef->lifespanAtMaxWind = 0;
        elemDef->spawnIntervalAtMaxWind = 0;
    }
    if ( edElemDef->elemType != 6 )
    {
        billboardTopWidth = edElemDef->billboardTopWidth;
        if ( (float)(billboardTopWidth - 1.0) < 0.0 )
            v16 = edElemDef->billboardTopWidth;
        else
            v16 = 1.0f;
        if ( (float)(0.0 - billboardTopWidth) < 0.0 )
            v10 = LODWORD(v16);
        else
            v10 = 0;
        elemDef->u.cloudDensityRange.base = v10;
        billboardBottomWidth = edElemDef->billboardBottomWidth;
        if ( (float)(billboardBottomWidth - 1.0) < 0.0 )
            v14 = edElemDef->billboardBottomWidth;
        else
            v14 = 1.0f;
        if ( (float)(0.0 - billboardBottomWidth) < 0.0 )
            v9 = LODWORD(v14);
        else
            v9 = 0;
        elemDef->u.cloudDensityRange.amplitude = v9;
    }
    FX_ConvertElemSpawnSound(&elemDef->spawnSound, &edElemDef->spawnSound, memPool);
    if ( (edElemDef->flags & 0x200000) != 0 )
    {
        elemDef->billboardPivot[0] = edElemDef->billboardPivot[0] * 2.0;
        elemDef->billboardPivot[1] = edElemDef->billboardPivot[1] * -2.0;
    }
    else
    {
        elemDef->billboardPivot[0] = 0.0f;
        elemDef->billboardPivot[1] = 0.0f;
    }
}

void __cdecl FX_SampleVelocity(FxElemDef *elemDef, const FxEditorElemDef *edElemDef)
{
    float v2; // [esp+4h] [ebp-30h]
    float v3; // [esp+Ch] [ebp-28h]
    float velScale[2][3]; // [esp+14h] [ebp-20h] BYREF
    FxElemVelStateSample *velStateRange; // [esp+2Ch] [ebp-8h]
    int velStateStride; // [esp+30h] [ebp-4h]

    v3 = 1.0 / (float)((float)elemDef->velIntervalCount * 1000.0);
    velScale[0][0] = v3 * edElemDef->velScale[0][0];
    velScale[0][1] = v3 * edElemDef->velScale[0][1];
    velScale[0][2] = v3 * edElemDef->velScale[0][2];
    v2 = 1.0 / (float)((float)elemDef->velIntervalCount * 1000.0);
    velScale[1][0] = v2 * edElemDef->velScale[1][0];
    velScale[1][1] = v2 * edElemDef->velScale[1][1];
    velScale[1][2] = v2 * edElemDef->velScale[1][2];
    velStateStride = 2;
    velStateRange = (FxElemVelStateSample*)elemDef->velSamples;
    FX_SampleVelocityInFrame(elemDef, velScale, &velStateRange->local, 2, 0x1000000, edElemDef);
    FX_SampleVelocityInFrame(elemDef, velScale, &velStateRange->world, velStateStride, 0x2000000, edElemDef);
}

void __cdecl FX_SampleVelocityInFrame(
                FxElemDef *elemDef,
                const float (*velScale)[3],
                FxElemVelStateInFrame *velState,
                int velStateStride,
                int useGraphBit,
                const FxEditorElemDef *edElemDef)
{
    bool v6; // [esp+8h] [ebp-80h]
    bool v7; // [esp+Ch] [ebp-7Ch]
    float *amplitude; // [esp+18h] [ebp-70h]
    FxElemVec3Range *p_totalDelta; // [esp+30h] [ebp-58h]
    bool anyNonZero; // [esp+53h] [ebp-35h]
    float velEpsilonSq; // [esp+54h] [ebp-34h]
    float velEpsilonSqa; // [esp+54h] [ebp-34h]
    float sampleTime; // [esp+58h] [ebp-30h]
    bool useVelocityRand[4]; // [esp+5Ch] [ebp-2Ch]
    float deltaInSegment[3]; // [esp+60h] [ebp-28h]
    float velocitySample[3]; // [esp+6Ch] [ebp-1Ch]
    int sampleIndex; // [esp+78h] [ebp-10h]
    bool brokenCompatibilityMode; // [esp+7Fh] [ebp-9h]
    FxElemVelStateInFrame *velStatePrev; // [esp+80h] [ebp-8h]
    bool useVelocity[4]; // [esp+84h] [ebp-4h]

    useVelocity[0] = (edElemDef->editorFlags & 0x800) == (useGraphBit != 0x2000000 ? 0 : 0x800);
    useVelocity[1] = (edElemDef->editorFlags & 0x1000) == (useGraphBit != 0x2000000 ? 0 : 0x1000);
    v7 = useVelocity[0] && (edElemDef->editorFlags & 0x100) != 0;
    useVelocityRand[0] = v7;
    v6 = useVelocity[1] && (edElemDef->editorFlags & 0x200) != 0;
    useVelocityRand[1] = v6;
    brokenCompatibilityMode = (edElemDef->editorFlags & 0x400) != 0;
    velEpsilonSq = 0.0f;
    if ( useVelocity[0] )
        velEpsilonSq = (float)((float)((float)((*velScale)[0] * (*velScale)[0])
                                                                 + (float)((float)(*velScale)[1] * (float)(*velScale)[1]))
                                                 + (float)((float)(*velScale)[2] * (float)(*velScale)[2]))
                                 + 0.0;
    if ( useVelocity[1] )
        velEpsilonSq = (float)((float)((float)((float)(*velScale)[3] * (float)(*velScale)[3])
                                                                 + (float)((float)(*velScale)[4] * (float)(*velScale)[4]))
                                                 + (float)((float)(*velScale)[5] * (float)(*velScale)[5]))
                                 + velEpsilonSq;
    velEpsilonSqa = velEpsilonSq * 0.0000010000001;
    anyNonZero = 0;
    velStatePrev = 0;
    for ( sampleIndex = 0; sampleIndex <= elemDef->velIntervalCount; ++sampleIndex )
    {
        sampleTime = (float)sampleIndex / (float)elemDef->velIntervalCount;
        velState->velocity.base[0] = 0.0f;
        velState->velocity.base[1] = 0.0f;
        velState->velocity.base[2] = 0.0f;
        velState->velocity.amplitude[0] = 0.0f;
        velState->velocity.amplitude[1] = 0.0f;
        velState->velocity.amplitude[2] = 0.0f;
        if ( useVelocity[0] )
        {
            velocitySample[0] = FX_SampleCurve1D(edElemDef->velShape[0][0][0], (*velScale)[0], sampleTime);
            velocitySample[1] = FX_SampleCurve1D(edElemDef->velShape[0][1][0], (*velScale)[1], sampleTime);
            velocitySample[2] = FX_SampleCurve1D(edElemDef->velShape[0][2][0], (*velScale)[2], sampleTime);
            velState->velocity.base[0] = velState->velocity.base[0] + velocitySample[0];
            velState->velocity.base[1] = velState->velocity.base[1] + velocitySample[1];
            velState->velocity.base[2] = velState->velocity.base[2] + velocitySample[2];
            if ( useVelocityRand[0] )
            {
                velState->velocity.amplitude[0] = velState->velocity.amplitude[0] - velocitySample[0];
                velState->velocity.amplitude[1] = velState->velocity.amplitude[1] - velocitySample[1];
                velState->velocity.amplitude[2] = velState->velocity.amplitude[2] - velocitySample[2];
            }
        }
        if ( useVelocity[1] )
        {
            velocitySample[0] = FX_SampleCurve1D(edElemDef->velShape[1][0][0], (*velScale)[3], sampleTime);
            velocitySample[1] = FX_SampleCurve1D(edElemDef->velShape[1][1][0], (*velScale)[4], sampleTime);
            velocitySample[2] = FX_SampleCurve1D(edElemDef->velShape[1][2][0], (*velScale)[5], sampleTime);
            velState->velocity.base[0] = velState->velocity.base[0] + velocitySample[0];
            velState->velocity.base[1] = velState->velocity.base[1] + velocitySample[1];
            velState->velocity.base[2] = velState->velocity.base[2] + velocitySample[2];
            if ( useVelocityRand[!brokenCompatibilityMode] )
            {
                velState->velocity.amplitude[0] = velState->velocity.amplitude[0] - velocitySample[0];
                velState->velocity.amplitude[1] = velState->velocity.amplitude[1] - velocitySample[1];
                velState->velocity.amplitude[2] = velState->velocity.amplitude[2] - velocitySample[2];
            }
        }
        if ( velStatePrev )
        {
            deltaInSegment[0] = (float)(velStatePrev->velocity.base[0] + velState->velocity.base[0]) * 0.5;
            deltaInSegment[1] = (float)(velStatePrev->velocity.base[1] + velState->velocity.base[1]) * 0.5;
            deltaInSegment[2] = (float)(velStatePrev->velocity.base[2] + velState->velocity.base[2]) * 0.5;
            p_totalDelta = &velStatePrev->totalDelta;
            velState->totalDelta.base[0] = deltaInSegment[0] + velStatePrev->totalDelta.base[0];
            velState->totalDelta.base[1] = deltaInSegment[1] + p_totalDelta->base[1];
            velState->totalDelta.base[2] = deltaInSegment[2] + p_totalDelta->base[2];
            if ( !anyNonZero )
                anyNonZero = (float)((float)((float)(velState->totalDelta.base[0] * velState->totalDelta.base[0])
                                                                     + (float)(velState->totalDelta.base[1] * velState->totalDelta.base[1]))
                                                     + (float)(velState->totalDelta.base[2] * velState->totalDelta.base[2])) > velEpsilonSqa;
        }
        else
        {
            velState->totalDelta.base[0] = 0.0f;
            velState->totalDelta.base[1] = 0.0f;
            velState->totalDelta.base[2] = 0.0f;
        }
        if ( useVelocityRand[0] )
        {
            velState->velocity.amplitude[0] = FX_SampleCurve1D(edElemDef->velShape[0][0][1], (*velScale)[0], sampleTime)
                                                                            + velState->velocity.amplitude[0];
            velState->velocity.amplitude[1] = FX_SampleCurve1D(edElemDef->velShape[0][1][1], (*velScale)[1], sampleTime)
                                                                            + velState->velocity.amplitude[1];
            velState->velocity.amplitude[2] = FX_SampleCurve1D(edElemDef->velShape[0][2][1], (*velScale)[2], sampleTime)
                                                                            + velState->velocity.amplitude[2];
        }
        if ( useVelocityRand[1] )
        {
            velState->velocity.amplitude[0] = FX_SampleCurve1D(edElemDef->velShape[1][0][1], (*velScale)[3], sampleTime)
                                                                            + velState->velocity.amplitude[0];
            velState->velocity.amplitude[1] = FX_SampleCurve1D(edElemDef->velShape[1][1][1], (*velScale)[4], sampleTime)
                                                                            + velState->velocity.amplitude[1];
            velState->velocity.amplitude[2] = FX_SampleCurve1D(edElemDef->velShape[1][2][1], (*velScale)[5], sampleTime)
                                                                            + velState->velocity.amplitude[2];
        }
        if ( velStatePrev )
        {
            deltaInSegment[0] = (float)(velStatePrev->velocity.amplitude[0] + velState->velocity.amplitude[0]) * 0.5;
            deltaInSegment[1] = (float)(velStatePrev->velocity.amplitude[1] + velState->velocity.amplitude[1]) * 0.5;
            deltaInSegment[2] = (float)(velStatePrev->velocity.amplitude[2] + velState->velocity.amplitude[2]) * 0.5;
            amplitude = velStatePrev->totalDelta.amplitude;
            velState->totalDelta.amplitude[0] = deltaInSegment[0] + velStatePrev->totalDelta.amplitude[0];
            velState->totalDelta.amplitude[1] = deltaInSegment[1] + amplitude[1];
            velState->totalDelta.amplitude[2] = deltaInSegment[2] + amplitude[2];
            if ( !anyNonZero )
                anyNonZero = (float)((float)((float)(velState->totalDelta.amplitude[0] * velState->totalDelta.amplitude[0])
                                                                     + (float)(velState->totalDelta.amplitude[1] * velState->totalDelta.amplitude[1]))
                                                     + (float)(velState->totalDelta.amplitude[2] * velState->totalDelta.amplitude[2])) > velEpsilonSqa;
        }
        else
        {
            velState->totalDelta.amplitude[0] = 0.0f;
            velState->totalDelta.amplitude[1] = 0.0f;
            velState->totalDelta.amplitude[2] = 0.0f;
        }
        velStatePrev = velState;
        velState += velStateStride;
    }
    if ( anyNonZero )
        elemDef->flags |= useGraphBit;
}

double __cdecl FX_SampleCurve1D(const FxCurve *curve, float scale, float time)
{
    FxCurveIterator iter; // [esp+24h] [ebp-Ch] BYREF
    float value; // [esp+2Ch] [ebp-4h]

    FxCurveIterator_Create(&iter, curve);
    value = FxCurveIterator_SampleTime(&iter, time) * scale;
    FxCurveIterator_Release(&iter);
    return value;
}

double __cdecl FxCurveIterator_SampleTime(FxCurveIterator *source, float time)
{
    FxCurveIterator_MoveToTime(source, time);
    if ( source->currentKeyIndex >= (unsigned int)(source->master->keyCount - 1)
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\effectscore\\FxCurve.h",
                    126,
                    0,
                    "source->currentKeyIndex doesn't index source->master->keyCount - 1\n\t%i not in [0, %i)",
                    source->currentKeyIndex,
                    source->master->keyCount - 1) )
    {
        __debugbreak();
    }
    return (float)FxCurve_Interpolate1d((float*)&source->master->keys[2 * source->currentKeyIndex], time);
}

double __cdecl FxCurve_Interpolate1d(float *key, float intermediateTime)
{
    const char *v2; // eax
    float value1; // [esp+18h] [ebp-10h]
    float value0; // [esp+1Ch] [ebp-Ch]
    float time1; // [esp+20h] [ebp-8h]
    float time0; // [esp+24h] [ebp-4h]

    time0 = *key;
    time1 = key[2];
    value0 = key[1];
    value1 = key[3];
    if ( intermediateTime < *key || time1 < intermediateTime || time0 == time1 )
    {
        v2 = va("%g, %g, %g", time0, time1, intermediateTime);
        if ( !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\effectscore\\FxCurve.h",
                        56,
                        0,
                        "%s\n\t%s",
                        "time0 <= intermediateTime && intermediateTime <= time1 && time0 != time1",
                        v2) )
            __debugbreak();
    }
    return (intermediateTime - time0) * (value1 - value0) / (time1 - time0) + value0;
}

void __cdecl FxCurveIterator_MoveToTime(FxCurveIterator *source, float time)
{
    const char *v2; // eax
    int keySize; // [esp+8h] [ebp-8h]
    const float *key; // [esp+Ch] [ebp-4h]

    if ( !source && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\effectscore\\FxCurve.h", 85, 0, "%s", "source") )
        __debugbreak();
    if ( !source->master
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\effectscore\\FxCurve.h", 86, 0, "%s", "source->master") )
    {
        __debugbreak();
    }
    if ( source->master == (const FxCurve *)-8
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\effectscore\\FxCurve.h",
                    87,
                    0,
                    "%s",
                    "source->master->keys") )
    {
        __debugbreak();
    }
    if ( (time < 0.0 || time > 1.0)
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\effectscore\\FxCurve.h",
                    88,
                    0,
                    "%s\n\t(time) = %g",
                    "(time >= 0.0f && time <= 1.0f)",
                    time) )
    {
        __debugbreak();
    }
    if ( source->master->keyCount <= 0
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\effectscore\\FxCurve.h",
                    89,
                    0,
                    "%s\n\t(source->master->keyCount) = %i",
                    "(source->master->keyCount > 0)",
                    source->master->keyCount) )
    {
        __debugbreak();
    }
    if ( source->master->dimensionCount <= 0
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\effectscore\\FxCurve.h",
                    90,
                    0,
                    "%s\n\t(source->master->dimensionCount) = %i",
                    "(source->master->dimensionCount > 0)",
                    source->master->dimensionCount) )
    {
        __debugbreak();
    }
    if ( source->currentKeyIndex >= (unsigned int)source->master->keyCount
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\effectscore\\FxCurve.h",
                    91,
                    0,
                    "source->currentKeyIndex doesn't index source->master->keyCount\n\t%i not in [0, %i)",
                    source->currentKeyIndex,
                    source->master->keyCount) )
    {
        __debugbreak();
    }
    keySize = source->master->dimensionCount + 1;
    key = &source->master->keys[keySize * source->currentKeyIndex];
    if ( *key > time )
    {
        source->currentKeyIndex = 0;
        key = source->master->keys;
    }
    while ( time > key[keySize] )
    {
        ++source->currentKeyIndex;
        key += keySize;
    }
    if ( key != &source->master->keys[keySize * source->currentKeyIndex] )
    {
        v2 = va("%p != %p", key, &source->master->keys[keySize * source->currentKeyIndex]);
        if ( !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\effectscore\\FxCurve.h",
                        108,
                        1,
                        "%s\n\t%s",
                        "key == &source->master->keys[source->currentKeyIndex * keySize]",
                        v2) )
            __debugbreak();
    }
    if ( source->currentKeyIndex >= (unsigned int)source->master->keyCount
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\effectscore\\FxCurve.h",
                    109,
                    1,
                    "source->currentKeyIndex doesn't index source->master->keyCount\n\t%i not in [0, %i)",
                    source->currentKeyIndex,
                    source->master->keyCount) )
    {
        __debugbreak();
    }
}

void __cdecl FX_SampleVisualState(FxElemDef *elemDef, const FxEditorElemDef *edElemDef)
{
    double v2; // st7
    double v3; // st7
    float sampleTime; // [esp+24Ch] [ebp-3Ch]
    bool secondAlphaSrc; // [esp+250h] [ebp-38h]
    float rotationScale; // [esp+254h] [ebp-34h]
    int sampleIndex; // [esp+258h] [ebp-30h]
    float rgba[4]; // [esp+25Ch] [ebp-2Ch] BYREF
    int secondColorSrc; // [esp+26Ch] [ebp-1Ch]
    FxElemVisStateSample *visStateRange; // [esp+270h] [ebp-18h]
    FxSampleChannel routing[5]; // [esp+274h] [ebp-14h] BYREF

    FX_GetVisualSampleRouting(edElemDef, routing);
    rotationScale = (float)(edElemDef->rotationScale * 0.017453292)
                                / (float)((float)elemDef->visStateIntervalCount * 1000.0);
    secondColorSrc = (edElemDef->editorFlags & 2) != 0;
    secondAlphaSrc = (edElemDef->editorFlags & 4) != 0;
    for ( sampleIndex = 0; sampleIndex <= elemDef->visStateIntervalCount; ++sampleIndex )
    {
        sampleTime = (float)sampleIndex / (float)elemDef->visStateIntervalCount;
        visStateRange = (FxElemVisStateSample*)&elemDef->visSamples[sampleIndex];
        if ( routing[0] )
        {
            if ( routing[0] != FX_CHAN_NONE
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp",
                            563,
                            0,
                            "%s",
                            "routing[FX_CHAN_RGBA] == FX_CHAN_NONE") )
            {
                __debugbreak();
            }
            *(unsigned int *)visStateRange->base.color = -1;
            *(unsigned int *)visStateRange->amplitude.color = -1;
        }
        else
        {
            FX_SampleCurve3D(edElemDef->color[0], 1.0, sampleTime, rgba);
            rgba[3] = FX_SampleCurve1D(edElemDef->alpha[0], 1.0, sampleTime);
            if ( (edElemDef->editorFlags & 0x80) != 0 )
            {
                rgba[0] = rgba[3] * rgba[0];
                rgba[1] = rgba[3] * rgba[1];
                rgba[2] = rgba[3] * rgba[2];
                rgba[3] = 1.0f;
            }
            Byte4PackVertexColor(rgba, visStateRange->base.color);
            FX_SampleCurve3D(edElemDef->color[secondColorSrc], 1.0, sampleTime, rgba);
            rgba[3] = FX_SampleCurve1D(edElemDef->alpha[secondAlphaSrc], 1.0, sampleTime);
            if ( (edElemDef->editorFlags & 0x80) != 0 )
            {
                rgba[0] = rgba[3] * rgba[0];
                rgba[1] = rgba[3] * rgba[1];
                rgba[2] = rgba[3] * rgba[2];
                rgba[3] = 1.0f;
            }
            Byte4PackVertexColor(rgba, visStateRange->amplitude.color);
        }
        if ( routing[4] == FX_CHAN_ROTATION )
        {
            v2 = FX_SampleCurve1D(edElemDef->rotationShape[0], rotationScale, sampleTime);
            visStateRange->base.rotationDelta = v2;
            if ( sampleIndex )
                visStateRange->base.rotationTotal = (float)((float)(visStateRange[-1].base.rotationDelta
                                                                                                                    + visStateRange->base.rotationDelta)
                                                                                                    * 0.5)
                                                                                    + visStateRange[-1].base.rotationTotal;
            else
                visStateRange->base.rotationTotal = 0.0f;
            if ( (edElemDef->editorFlags & 0x40) != 0 )
            {
                v3 = FX_SampleCurve1D(edElemDef->rotationShape[1], rotationScale, sampleTime);
                visStateRange->amplitude.rotationDelta = v3 - visStateRange->base.rotationDelta;
                if ( sampleIndex )
                    visStateRange->amplitude.rotationTotal = (float)((float)(visStateRange[-1].amplitude.rotationDelta
                                                                                                                                 + visStateRange->amplitude.rotationDelta)
                                                                                                                 * 0.5)
                                                                                                 + visStateRange[-1].amplitude.rotationTotal;
                else
                    visStateRange->amplitude.rotationTotal = 0.0f;
            }
            else
            {
                visStateRange->amplitude.rotationDelta = 0.0f;
                visStateRange->amplitude.rotationTotal = 0.0f;
            }
        }
        else
        {
            if ( routing[4] != FX_CHAN_NONE
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp",
                            591,
                            0,
                            "%s",
                            "routing[FX_CHAN_ROTATION] == FX_CHAN_NONE") )
            {
                __debugbreak();
            }
            visStateRange->base.rotationDelta = 0.0f;
            visStateRange->base.rotationTotal = 0.0f;
            visStateRange->amplitude.rotationDelta = 0.0f;
            visStateRange->amplitude.rotationTotal = 0.0f;
        }
        FX_SampleVisualStateScalar(
            edElemDef,
            sampleTime,
            routing[1],
            0.5,
            visStateRange->base.size,
            visStateRange->amplitude.size);
        FX_SampleVisualStateScalar(
            edElemDef,
            sampleTime,
            routing[2],
            0.5,
            &visStateRange->base.size[1],
            &visStateRange->amplitude.size[1]);
        FX_SampleVisualStateScalar(
            edElemDef,
            sampleTime,
            routing[3],
            1.0,
            &visStateRange->base.scale,
            &visStateRange->amplitude.scale);
    }
}

void __cdecl Byte4PackVertexColor(const float *from, unsigned __int8 *to)
{
    float v2; // [esp+48h] [ebp-144h]
    unsigned int v3; // [esp+4Ch] [ebp-140h]
    unsigned int v4; // [esp+50h] [ebp-13Ch]
    unsigned int v5; // [esp+54h] [ebp-138h]
    unsigned int v6; // [esp+58h] [ebp-134h]
    unsigned int v7; // [esp+5Ch] [ebp-130h]
    unsigned int v8; // [esp+60h] [ebp-12Ch]
    unsigned int v9; // [esp+64h] [ebp-128h]
    float v10; // [esp+68h] [ebp-124h]
    float v11; // [esp+6Ch] [ebp-120h]
    float v12; // [esp+70h] [ebp-11Ch]
    float v13; // [esp+74h] [ebp-118h]
    float4 v14[3]; // [esp+7Ch] [ebp-110h]
    float4 *p_permed; // [esp+B0h] [ebp-DCh]
    unsigned int v16; // [esp+B4h] [ebp-D8h]
    unsigned int v17; // [esp+B8h] [ebp-D4h]
    unsigned int v18; // [esp+BCh] [ebp-D0h]
    unsigned int v19; // [esp+C0h] [ebp-CCh]
    unsigned int v20; // [esp+C4h] [ebp-C8h]
    unsigned int v21; // [esp+C8h] [ebp-C4h]
    unsigned int v22; // [esp+CCh] [ebp-C0h]
    unsigned int v23; // [esp+D0h] [ebp-BCh]
    unsigned int v24; // [esp+D4h] [ebp-B8h]
    unsigned int v25; // [esp+D8h] [ebp-B4h]
    unsigned int v26; // [esp+DCh] [ebp-B0h]
    unsigned int v27; // [esp+E0h] [ebp-ACh]
    unsigned int v28; // [esp+E4h] [ebp-A8h]
    unsigned int v29; // [esp+E8h] [ebp-A4h]
    unsigned int v30; // [esp+ECh] [ebp-A0h]
    float v31; // [esp+F0h] [ebp-9Ch]
    float4 minned; // [esp+F4h] [ebp-98h]
    float4 inted; // [esp+104h] [ebp-88h]
    float4 permed; // [esp+118h] [ebp-74h] BYREF
    float4 v35; // [esp+128h] [ebp-64h]
    float4 bumped; // [esp+138h] [ebp-54h]
    float4 maxxed; // [esp+148h] [ebp-44h]
    float4 scaled; // [esp+158h] [ebp-34h]
    float4 floored; // [esp+168h] [ebp-24h]
    float4 scale; // [esp+178h] [ebp-14h]

    scale.v[0] = 255.0f;
    scale.v[1] = 255.0f;
    scale.v[2] = 255.0f;
    scale.v[3] = 255.0f;
    *(_QWORD *)v35.v = *(_QWORD *)from;
    *(_QWORD *)&v35.unitVec[2].packed = *((_QWORD *)from + 1);
    scaled.v[0] = v35.v[0] * 255.0;
    scaled.v[1] = v35.v[1] * 255.0;
    scaled.v[2] = v35.v[2] * 255.0;
    scaled.v[3] = v35.v[3] * 255.0;
    bumped.v[0] = (float)(v35.v[0] * 255.0) + 0.5;
    bumped.v[1] = (float)(v35.v[1] * 255.0) + 0.5;
    bumped.v[2] = (float)(v35.v[2] * 255.0) + 0.5;
    bumped.v[3] = (float)(v35.v[3] * 255.0) + 0.5;
    v31 = bumped.v[0];
    v13 = floor(bumped.v[0]);
    floored.v[0] = v13;
    v30 = bumped.u[1];
    v12 = floor(bumped.v[1]);
    floored.v[1] = v12;
    v29 = bumped.u[2];
    v11 = floor(bumped.v[2]);
    floored.v[2] = v11;
    v28 = bumped.u[3];
    v10 = floor(bumped.v[3]);
    floored.v[3] = v10;
    if ( (float)(255.0 - v13) < 0.0 )
        v9 = scale.u[0];
    else
        v9 = floored.u[0];
    minned.u[0] = v9;
    v26 = floored.u[1];
    v27 = scale.u[1];
    if ( (float)(scale.v[1] - floored.v[1]) < 0.0 )
        v8 = v27;
    else
        v8 = v26;
    minned.u[1] = v8;
    v24 = floored.u[2];
    v25 = scale.u[2];
    if ( (float)(scale.v[2] - floored.v[2]) < 0.0 )
        v7 = v25;
    else
        v7 = v24;
    minned.u[2] = v7;
    v22 = floored.u[3];
    v23 = scale.u[3];
    if ( (float)(scale.v[3] - floored.v[3]) < 0.0 )
        v6 = v23;
    else
        v6 = v22;
    minned.u[3] = v6;
    if ( (float)(0.0 - minned.v[0]) < 0.0 )
        v5 = minned.u[0];
    else
        v5 = g_zero.u[0];
    maxxed.u[0] = v5;
    v20 = g_zero.u[1];
    v21 = minned.u[1];
    if ( (float)(0.0 - minned.v[1]) < 0.0 )
        v4 = v21;
    else
        v4 = v20;
    maxxed.u[1] = v4;
    v18 = g_zero.u[2];
    v19 = minned.u[2];
    if ( (float)(0.0 - minned.v[2]) < 0.0 )
        v3 = v19;
    else
        v3 = v18;
    maxxed.u[2] = v3;
    v16 = g_zero.u[3];
    v17 = minned.u[3];
    if ( (float)(0.0 - minned.v[3]) < 0.0 )
        v2 = *(float *)&v17;
    else
        v2 = *(float *)&v16;
    maxxed.v[3] = v2;
    inted.u[0] = (__int64)maxxed.v[0];
    inted.u[1] = (__int64)maxxed.v[1];
    inted.u[2] = (__int64)maxxed.v[2];
    inted.u[3] = (__int64)v2;
    v14[0] = inted;
    v14[1] = inted;
    v14[2] = NATIVE_VERTEX_PERM;
    p_permed = &permed;
    permed.unitVec[0].array[0] = v14[0].unitVec[0].array[NATIVE_VERTEX_PERM.unitVec[0].array[3]];
    permed.unitVec[0].array[1] = v14[0].unitVec[0].array[NATIVE_VERTEX_PERM.unitVec[0].array[2]];
    permed.unitVec[0].array[2] = v14[0].unitVec[0].array[NATIVE_VERTEX_PERM.unitVec[0].array[1]];
    permed.unitVec[0].array[3] = v14[0].unitVec[0].array[NATIVE_VERTEX_PERM.unitVec[0].array[0]];
    *(unsigned int *)to = permed.u[0];
}

void __cdecl FX_SampleCurve3D(const FxCurve *curve, float scale, float time, float *value)
{
    FxCurveIterator iter; // [esp+18h] [ebp-8h] BYREF

    FxCurveIterator_Create(&iter, curve);
    FxCurveIterator_SampleTimeVec3(&iter, value, time);
    *value = scale * *value;
    value[1] = scale * value[1];
    value[2] = scale * value[2];
    FxCurveIterator_Release(&iter);
}

void __cdecl FxCurveIterator_SampleTimeVec3(FxCurveIterator *source, float *replyVector, float time)
{
    FxCurveIterator_MoveToTime(source, time);
    if ( source->currentKeyIndex >= (unsigned int)(source->master->keyCount - 1)
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\effectscore\\FxCurve.h",
                    148,
                    0,
                    "source->currentKeyIndex doesn't index source->master->keyCount - 1\n\t%i not in [0, %i)",
                    source->currentKeyIndex,
                    source->master->keyCount - 1) )
    {
        __debugbreak();
    }
    FxCurve_Interpolate3d(&source->master->keys[4 * source->currentKeyIndex], time, replyVector);
}

void __cdecl FxCurve_Interpolate3d(const float *key, float intermediateTime, float *result)
{
    const char *v3; // eax
    float time1; // [esp+20h] [ebp-8h]
    float time0; // [esp+24h] [ebp-4h]

    time0 = *key;
    time1 = key[4];
    if ( intermediateTime < *key || time1 < intermediateTime || time0 == time1 )
    {
        v3 = va("%g, %g, %g", time0, time1, intermediateTime);
        if ( !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\effectscore\\FxCurve.h",
                        74,
                        0,
                        "%s\n\t%s",
                        "time0 <= intermediateTime && intermediateTime <= time1 && time0 != time1",
                        v3) )
            __debugbreak();
    }
    Vec3Lerp(key + 1, key + 5, (float)(intermediateTime - time0) / (float)(time1 - time0), result);
}

void __cdecl FX_SampleVisualStateScalar(
                const FxEditorElemDef *edElemDef,
                float sampleTime,
                FxSampleChannel routing,
                float scaleFactor,
                float *base,
                float *amplitude)
{
    switch ( routing )
    {
        case FX_CHAN_SIZE_0:
            *base = FX_SampleCurve1D(edElemDef->sizeShape[0][0], edElemDef->sizeScale[0] * scaleFactor, sampleTime);
            if ( (edElemDef->editorFlags & 8) != 0 )
                *amplitude = FX_SampleCurve1D(edElemDef->sizeShape[0][1], edElemDef->sizeScale[0] * scaleFactor, sampleTime)
                                     - *base;
            else
                *amplitude = 0.0f;
            break;
        case FX_CHAN_SIZE_1:
            *base = FX_SampleCurve1D(edElemDef->sizeShape[1][0], edElemDef->sizeScale[1] * scaleFactor, sampleTime);
            if ( (edElemDef->editorFlags & 0x10) != 0 )
                *amplitude = FX_SampleCurve1D(edElemDef->sizeShape[1][1], edElemDef->sizeScale[1] * scaleFactor, sampleTime)
                                     - *base;
            else
                *amplitude = 0.0f;
            break;
        case FX_CHAN_SCALE:
            *base = FX_SampleCurve1D(edElemDef->scaleShape[0], edElemDef->scaleScale * scaleFactor, sampleTime);
            if ( (edElemDef->editorFlags & 0x20) != 0 )
                *amplitude = FX_SampleCurve1D(edElemDef->scaleShape[1], edElemDef->scaleScale * scaleFactor, sampleTime) - *base;
            else
                *amplitude = 0.0f;
            break;
        default:
            if ( routing != FX_CHAN_NONE
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp",
                            509,
                            0,
                            "%s",
                            "routing == FX_CHAN_NONE") )
            {
                __debugbreak();
            }
            *base = 0.0f;
            *amplitude = 0.0f;
            break;
    }
}

void __cdecl FX_ConvertAtlas(FxElemDef *elemDef, const FxEditorElemDef *edElemDef)
{
    int v2; // [esp+0h] [ebp-40h]
    MaterialInfo mtlInfo; // [esp+8h] [ebp-38h] BYREF
    unsigned int entryCountBits; // [esp+34h] [ebp-Ch]
    int indexRange; // [esp+38h] [ebp-8h]
    unsigned int indexRangeBits; // [esp+3Ch] [ebp-4h]

    if ( !elemDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp", 681, 0, "%s", "elemDef") )
    {
        __debugbreak();
    }
    if ( !edElemDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp", 682, 0, "%s", "edElemDef") )
    {
        __debugbreak();
    }
    if ( edElemDef->visualCount && FX_ElemUsesMaterial(edElemDef) )
    {
        elemDef->atlas.behavior = edElemDef->atlas.behavior;
        elemDef->atlas.index = edElemDef->atlas.index;
        elemDef->atlas.fps = edElemDef->atlas.fps;
        elemDef->atlas.loopCount = edElemDef->atlas.loopCount + 1;
        elemDef->atlas.colIndexBits = edElemDef->atlas.colIndexBits;
        elemDef->atlas.rowIndexBits = edElemDef->atlas.rowIndexBits;
        Material_GetInfo(edElemDef->visuals[0].material, &mtlInfo);
        for ( elemDef->atlas.rowIndexBits = 0;
                    1 << elemDef->atlas.rowIndexBits < mtlInfo.textureAtlasRowCount;
                    ++elemDef->atlas.rowIndexBits )
        {
            ;
        }
        for ( elemDef->atlas.colIndexBits = 0;
                    1 << elemDef->atlas.colIndexBits < mtlInfo.textureAtlasColumnCount;
                    ++elemDef->atlas.colIndexBits )
        {
            ;
        }
        if ( edElemDef->atlas.indexRange < 1 )
            v2 = 1;
        else
            v2 = edElemDef->atlas.indexRange;
        indexRange = v2;
        entryCountBits = 1 << (elemDef->atlas.colIndexBits + elemDef->atlas.rowIndexBits);
        LOWORD(indexRangeBits) = (v2 & 0x7F) << 9;
        elemDef->atlas.entryCountAndIndexRange = indexRangeBits | entryCountBits;
    }
    else
    {
        *(unsigned int *)&elemDef->atlas.behavior = 0;
        *(unsigned int *)&elemDef->atlas.colIndexBits = 0;
    }
}

bool __cdecl FX_ElemUsesMaterial(const FxEditorElemDef *edElemDef)
{
    unsigned __int8 elemType; // [esp+0h] [ebp-4h]

    elemType = edElemDef->elemType;
    return elemType < 7u || elemType > 0xAu && elemType != 12;
}

void __cdecl FX_ScaleFloatRange(FxFloatRange *to, const FxFloatRange *from, float scale)
{
    to->base = from->base * scale;
    to->amplitude = from->amplitude * scale;
}

void __cdecl FX_ReserveElemDefMemory(FxElemDef *elemDef, unsigned __int8 **memPool)
{
    if ( !elemDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp", 728, 0, "%s", "elemDef") )
    {
        __debugbreak();
    }
    if ( !memPool
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp", 729, 0, "%s", "memPool") )
    {
        __debugbreak();
    }
    if ( !*memPool
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp", 730, 0, "%s", "*memPool") )
    {
        __debugbreak();
    }
    if ( !elemDef->velIntervalCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp",
                    732,
                    0,
                    "%s",
                    "elemDef->velIntervalCount") )
    {
        __debugbreak();
    }
    elemDef->velSamples = (const FxElemVelStateSample *)*memPool;
    *memPool += 96 * elemDef->velIntervalCount + 96;
    if ( elemDef->visStateIntervalCount )
    {
        elemDef->visSamples = (const FxElemVisStateSample *)*memPool;
        *memPool += 48 * elemDef->visStateIntervalCount + 48;
    }
    else
    {
        elemDef->visSamples = 0;
    }
    if ( elemDef->elemType == 11 )
    {
        elemDef->visuals.markArray = (FxElemMarkVisuals *)*memPool;
        *memPool += 8 * elemDef->visualCount;
    }
    else if ( elemDef->visualCount > 1u )
    {
        elemDef->visuals.markArray = (FxElemMarkVisuals *)*memPool;
        *memPool += 4 * elemDef->visualCount;
    }
}

void __cdecl FX_CopyMarkVisuals(const FxEditorElemDef *edElemDef, FxElemMarkVisuals *markVisualsArray)
{
    //memcpy((unsigned __int8 *)markVisualsArray, (unsigned __int8 *)&edElemDef->692, 8 * edElemDef->visualCount);
    memcpy((unsigned __int8 *)markVisualsArray, (unsigned __int8 *)&edElemDef->markVisuals, 8 * edElemDef->visualCount);
}

void __cdecl FX_CopyVisuals(const FxEditorElemDef *edElemDef, FxElemVisuals *visualsArray)
{
    memcpy((unsigned __int8 *)visualsArray, (unsigned __int8 *)&edElemDef->visuals, 4 * edElemDef->visualCount);
}

void __cdecl FX_ConvertEffectDefRef(FxEffectDefRef *ref, const FxEffectDef *effectDef)
{
    ref->handle = effectDef;
}

void __cdecl FX_BoundFloatRange(FxFloatRange *range, float lower, float upper)
{
    if ( range->amplitude < 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp",
                    813,
                    0,
                    "%s",
                    "range->amplitude >= 0.0f") )
    {
        __debugbreak();
    }
    if ( lower > range->base || (float)(range->base + range->amplitude) > upper )
    {
        range->base = range->base + 0.000099999997;
        range->amplitude = range->amplitude - (float)(0.000099999997 * 2.0);
        if ( range->base < lower
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp",
                        820,
                        0,
                        "range->base >= lower\n\t%g, %g",
                        range->base,
                        lower) )
        {
            __debugbreak();
        }
        if ( upper < (float)(range->base + range->amplitude)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp",
                        821,
                        0,
                        "range->base + range->amplitude <= upper\n\t%g, %g",
                        (float)(range->base + range->amplitude),
                        upper) )
        {
            __debugbreak();
        }
    }
}

void __cdecl FX_ConvertTrail(FxTrailDef **outTrailDef, const FxEditorElemDef *edElemDef, unsigned __int8 **mempool)
{
    if ( edElemDef->elemType != 5
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp",
                    971,
                    0,
                    "%s",
                    "edElemDef->elemType == FX_ELEM_TYPE_TRAIL") )
    {
        __debugbreak();
    }
    *outTrailDef = (FxTrailDef *)*mempool;
    *mempool += 28;
    FX_ConvertTrail_CompileVertices(edElemDef, *outTrailDef, mempool);
    if ( edElemDef->trailSplitDist <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp",
                    976,
                    0,
                    "%s",
                    "edElemDef->trailSplitDist > 0") )
    {
        __debugbreak();
    }
    if ( edElemDef->trailRepeatDist <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp",
                    977,
                    0,
                    "%s",
                    "edElemDef->trailRepeatDist > 0") )
    {
        __debugbreak();
    }
    (*outTrailDef)->splitDist = edElemDef->trailSplitDist;
    (*outTrailDef)->scrollTimeMsec = (int)(float)(edElemDef->trailScrollTime * 1000.0);
    (*outTrailDef)->repeatDist = edElemDef->trailRepeatDist;
}

void __cdecl FX_ConvertTrail_CompileVertices(
                const FxEditorElemDef *edElemDef,
                FxTrailDef *outTrailDef,
                unsigned __int8 **mempool)
{
    double v3; // st7
    double v4; // st7
    double v5; // st6
    float v6[2]; // [esp+14h] [ebp-B0h] BYREF
    float *normal; // [esp+1Ch] [ebp-A8h]
    FxTrailVertex *v8; // [esp+20h] [ebp-A4h]
    const float *v9; // [esp+24h] [ebp-A0h]
    const float *v10; // [esp+34h] [ebp-90h]
    const float *v11; // [esp+38h] [ebp-8Ch]
    float v12[2]; // [esp+40h] [ebp-84h] BYREF
    const float *pos; // [esp+48h] [ebp-7Ch]
    const float *v14; // [esp+4Ch] [ebp-78h]
    float v[2]; // [esp+54h] [ebp-70h] BYREF
    FxTrailVertex *emittedVertPtrIter; // [esp+5Ch] [ebp-68h]
    float secondaryEdgeNorm[2]; // [esp+60h] [ebp-64h] BYREF
    float primaryEdgeNorm[2]; // [esp+68h] [ebp-5Ch] BYREF
    float accumNorm[2]; // [esp+70h] [ebp-54h] BYREF
    int edgeIter; // [esp+78h] [ebp-4Ch]
    unsigned __int16 *emittedIndPtrBegin; // [esp+7Ch] [ebp-48h]
    int indCount; // [esp+80h] [ebp-44h]
    FxTrailVertex *outVertPtrBegin; // [esp+84h] [ebp-40h]
    const unsigned __int16 *primaryEdgeIndPtr; // [esp+88h] [ebp-3Ch]
    const unsigned __int16 *indPtrEnd; // [esp+8Ch] [ebp-38h]
    const FxEditorTrailDef *trailDef; // [esp+90h] [ebp-34h]
    int vertBytes; // [esp+94h] [ebp-30h]
    float SNAP_TOLERANCE_POS; // [esp+98h] [ebp-2Ch]
    FxTrailVertex *emittedVertPtrBegin; // [esp+9Ch] [ebp-28h]
    FxTrailVertex *outVertPtrIter; // [esp+A0h] [ebp-24h]
    float SMOOTH_THRESHOLD; // [esp+A4h] [ebp-20h]
    unsigned __int16 *emittedIndPtrEnd; // [esp+A8h] [ebp-1Ch]
    const unsigned __int16 *secondaryEdgeIndPtr; // [esp+ACh] [ebp-18h]
    float SNAP_TOLERANCE_NORM; // [esp+B0h] [ebp-14h]
    FxTrailVertex *emittedVertPtrEnd; // [esp+B4h] [ebp-10h]
    float SNAP_TOLERANCE_TEXCOORD; // [esp+B8h] [ebp-Ch]
    int indBytes; // [esp+BCh] [ebp-8h]
    FxTrailVertex *outVertPtrEnd; // [esp+C0h] [ebp-4h]

    SNAP_TOLERANCE_POS = 0.01f;
    SNAP_TOLERANCE_TEXCOORD = 0.01f;
    SNAP_TOLERANCE_NORM = 0.95f;
    SMOOTH_THRESHOLD = 0.0f;
    trailDef = &edElemDef->trailDef;
    if ( edElemDef != (const FxEditorElemDef *)-832 )
    {
        if ( !trailDef
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp", 875, 0, "%s", "trailDef") )
        {
            __debugbreak();
        }
        indCount = trailDef->indCount;
        if ( (indCount & 1) != 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp",
                        878,
                        0,
                        "%s",
                        "(indCount & 1) == 0") )
        {
            __debugbreak();
        }
        vertBytes = 20 * indCount;
        outVertPtrBegin = (FxTrailVertex *)*mempool;
        *mempool += 20 * indCount;
        outVertPtrEnd = &outVertPtrBegin[indCount];
        outVertPtrIter = outVertPtrBegin;
        indPtrEnd = &trailDef->inds[indCount];
        for ( primaryEdgeIndPtr = trailDef->inds; primaryEdgeIndPtr != indPtrEnd; primaryEdgeIndPtr += 2 )
        {
            FX_ConvertTrail_CalcNormForSegment(
                trailDef->verts[*primaryEdgeIndPtr].pos,
                trailDef->verts[primaryEdgeIndPtr[1]].pos,
                primaryEdgeNorm);
            for ( edgeIter = 0; edgeIter != 2; ++edgeIter )
            {
                accumNorm[0] = 0.0f;
                accumNorm[1] = 0.0f;
                for ( secondaryEdgeIndPtr = trailDef->inds; secondaryEdgeIndPtr != indPtrEnd; secondaryEdgeIndPtr += 2 )
                {
                    pos = trailDef->verts[primaryEdgeIndPtr[edgeIter]].pos;
                    v14 = trailDef->verts[*secondaryEdgeIndPtr].pos;
                    v[0] = *pos - *v14;
                    v[1] = pos[1] - v14[1];
                    v3 = Vec2Length(v);
                    if ( v3 > SNAP_TOLERANCE_POS )
                    {
                        v10 = trailDef->verts[primaryEdgeIndPtr[edgeIter]].pos;
                        v11 = trailDef->verts[secondaryEdgeIndPtr[1]].pos;
                        v12[0] = *v10 - *v11;
                        v12[1] = v10[1] - v11[1];
                        v4 = Vec2Length(v12);
                        if ( v4 > SNAP_TOLERANCE_POS )
                            continue;
                    }
                    FX_ConvertTrail_CalcNormForSegment(
                        trailDef->verts[*secondaryEdgeIndPtr].pos,
                        trailDef->verts[secondaryEdgeIndPtr[1]].pos,
                        secondaryEdgeNorm);
                    if ( (float)((float)(primaryEdgeNorm[0] * secondaryEdgeNorm[0])
                                         + (float)(primaryEdgeNorm[1] * secondaryEdgeNorm[1])) > SMOOTH_THRESHOLD )
                    {
                        accumNorm[0] = secondaryEdgeNorm[0] + accumNorm[0];
                        accumNorm[1] = secondaryEdgeNorm[1] + accumNorm[1];
                    }
                }
                Vec2Normalize(accumNorm);
                v8 = &outVertPtrIter[edgeIter];
                v9 = trailDef->verts[primaryEdgeIndPtr[edgeIter]].pos;
                v8->pos[0] = *v9;
                v8->pos[1] = v9[1];
                normal = outVertPtrIter[edgeIter].normal;
                *normal = accumNorm[0];
                normal[1] = accumNorm[1];
                outVertPtrIter[edgeIter].texCoord = trailDef->verts[primaryEdgeIndPtr[edgeIter]].texCoord;
            }
            outVertPtrIter += 2;
        }
        if ( outVertPtrIter != outVertPtrEnd
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp",
                        924,
                        0,
                        "%s",
                        "outVertPtrIter == outVertPtrEnd") )
        {
            __debugbreak();
        }
        emittedVertPtrBegin = outVertPtrBegin;
        emittedVertPtrEnd = outVertPtrBegin;
        indBytes = 2 * indCount;
        emittedIndPtrBegin = (unsigned __int16 *)*mempool;
        *mempool += 2 * indCount;
        emittedIndPtrEnd = emittedIndPtrBegin;
        for ( outVertPtrIter = outVertPtrBegin; outVertPtrIter != outVertPtrEnd; ++outVertPtrIter )
        {
            if ( emittedVertPtrEnd > outVertPtrIter
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp",
                            936,
                            0,
                            "%s",
                            "emittedVertPtrEnd <= outVertPtrIter") )
            {
                __debugbreak();
            }
            for ( emittedVertPtrIter = emittedVertPtrBegin; emittedVertPtrIter != emittedVertPtrEnd; ++emittedVertPtrIter )
            {
                v6[0] = emittedVertPtrIter->pos[0] - outVertPtrIter->pos[0];
                v6[1] = emittedVertPtrIter->pos[1] - outVertPtrIter->pos[1];
                v5 = Vec2Length(v6);
                if ( v5 <= SNAP_TOLERANCE_POS
                    && fabs(outVertPtrIter->texCoord - emittedVertPtrIter->texCoord) <= SNAP_TOLERANCE_TEXCOORD
                    && SNAP_TOLERANCE_NORM <= (float)((float)(outVertPtrIter->normal[0] * emittedVertPtrIter->normal[0])
                                                                                    + (float)(outVertPtrIter->normal[1] * emittedVertPtrIter->normal[1])) )
                {
                    break;
                }
            }
            if ( emittedVertPtrIter == emittedVertPtrEnd )
            {
                *emittedVertPtrIter = *outVertPtrIter;
                ++emittedVertPtrEnd;
            }
            *emittedIndPtrEnd = emittedVertPtrIter - emittedVertPtrBegin;
            if ( *emittedIndPtrEnd != emittedVertPtrIter - emittedVertPtrBegin
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp",
                            956,
                            0,
                            "%s",
                            "*emittedIndPtrEnd == emittedVertPtrIter - emittedVertPtrBegin") )
            {
                __debugbreak();
            }
            ++emittedIndPtrEnd;
        }
        outTrailDef->vertsEA = emittedVertPtrBegin;
        outTrailDef->vertCount = emittedVertPtrEnd - emittedVertPtrBegin;
        if ( 20 * outTrailDef->vertCount > (unsigned int)vertBytes
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp",
                        962,
                        0,
                        "%s",
                        "outTrailDef->vertCount * sizeof( FxTrailVertex ) <= static_cast< size_t >( vertBytes )") )
        {
            __debugbreak();
        }
        outTrailDef->indsEA = emittedIndPtrBegin;
        outTrailDef->indCount = emittedIndPtrEnd - emittedIndPtrBegin;
        if ( 2 * outTrailDef->indCount > (unsigned int)indBytes
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp",
                        965,
                        0,
                        "%s",
                        "outTrailDef->indCount * sizeof( ushort ) <= static_cast< size_t >( indBytes )") )
        {
            __debugbreak();
        }
    }
}

void __cdecl FX_ConvertTrail_CalcNormForSegment(const float *vert0, const float *vert1, float *outNormal)
{
    *outNormal = vert0[1] - vert1[1];
    outNormal[1] = *vert1 - *vert0;
    Vec2Normalize(outNormal);
}

void __cdecl FX_ConvertElemSpawnSound(FxElemSpawnSound *dest, const FxElemSpawnSound *src, unsigned __int8 **memPool)
{
    unsigned int bytesNeeded; // [esp+10h] [ebp-4h]

    if ( src->spawnSound && *src->spawnSound )
    {
        dest->spawnSound = (const char *)*memPool;
        bytesNeeded = &src->spawnSound[strlen(src->spawnSound) + 1] - src->spawnSound;
        *memPool += bytesNeeded;
        memcpy((unsigned __int8 *)dest->spawnSound, (unsigned __int8 *)src->spawnSound, bytesNeeded);
    }
    else
    {
        dest->spawnSound = 0;
    }
}

int __cdecl FX_CopyEmittedElemDefs(
                FxElemDef *elemDefArray,
                const FxEditorEffectDef *editorEffect,
                unsigned __int8 **memPool)
{
    const FxElemSpawnSound *elemDefEmit; // [esp+8h] [ebp-20h]
    const FxEffectDef *emission; // [esp+Ch] [ebp-1Ch]
    int elemIndexEmit; // [esp+10h] [ebp-18h]
    FxElemDef *elemDef; // [esp+14h] [ebp-14h]
    int elemIndex; // [esp+1Ch] [ebp-Ch]
    int elemIndexStop; // [esp+20h] [ebp-8h]
    int elemCount; // [esp+24h] [ebp-4h]

    if ( !editorEffect
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp",
                    1266,
                    0,
                    "%s",
                    "editorEffect") )
    {
        __debugbreak();
    }
    elemCount = 0;
    for ( elemIndex = 0; elemIndex < editorEffect->elemCount; ++elemIndex )
    {
        emission = editorEffect->elems[elemIndex].emission;
        if ( emission && emission->elemDefCountOneShot && FX_FindEmission(emission, editorEffect) == elemIndex )
        {
            elemIndexStop = emission->elemDefCountOneShot + emission->elemDefCountLooping;
            for ( elemIndexEmit = emission->elemDefCountLooping; elemIndexEmit != elemIndexStop; ++elemIndexEmit )
            {
                elemDef = &elemDefArray[elemCount++];
                elemDefEmit = (const FxElemSpawnSound *)&emission->elemDefsEA[elemIndexEmit];
                memcpy((void *)elemDef, elemDefEmit, sizeof(FxElemDef));
                if ( (elemDef->flags & 0xC0) == 0x40 )
                {
                    elemDef->flags &= 0xFFFFFF3F;
                    elemDef->flags = elemDef->flags;
                }
                FX_ReserveElemDefMemory(elemDef, memPool);
                memcpy(
                    (unsigned __int8 *)elemDef->velSamples,
                    (unsigned __int8 *)elemDefEmit[47].spawnSound,
                    96 * (elemDef->velIntervalCount + 1));
                if ( elemDef->visSamples )
                    memcpy(
                        (void*)elemDef->visSamples->base.color,
                        (unsigned __int8 *)elemDefEmit[48].spawnSound,
                        48 * (elemDef->visStateIntervalCount + 1));
                if ( elemDef->visualCount > 1u )
                    memcpy(
                        (unsigned __int8 *)elemDef->visuals.markArray,
                        (unsigned __int8 *)elemDefEmit[49].spawnSound,
                        4 * elemDef->visualCount);
                FX_ConvertElemSpawnSound(&elemDef->spawnSound, elemDefEmit + 70, memPool);
            }
        }
    }
    return elemCount;
}

int __cdecl FX_AdditionalBytesNeededForElemDef(
                unsigned __int8 elemType,
                int velStateSampleCount,
                int visStateSampleCount,
                int visualCount,
                const FxElemSpawnSound *elemSpawnSound)
{
    int bytesNeeded; // [esp+10h] [ebp-4h]

    bytesNeeded = 96 * velStateSampleCount + 48 * visStateSampleCount;
    if ( elemType == 11 )
    {
        bytesNeeded += 8 * visualCount;
    }
    else if ( visualCount > 1 )
    {
        bytesNeeded += 4 * visualCount;
    }
    if ( elemSpawnSound->spawnSound && *elemSpawnSound->spawnSound )
        bytesNeeded += &elemSpawnSound->spawnSound[strlen(elemSpawnSound->spawnSound) + 1] - elemSpawnSound->spawnSound;
    return bytesNeeded;
}

int __cdecl FX_AdditionalBytesNeededForEmission(const FxEffectDef *emission)
{
    int v2; // [esp+0h] [ebp-1Ch]
    const FxElemDef *elemDef; // [esp+8h] [ebp-14h]
    int bytesNeeded; // [esp+Ch] [ebp-10h]
    int elemDefStop; // [esp+14h] [ebp-8h]
    int elemDefIndex; // [esp+18h] [ebp-4h]

    if ( !emission
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp", 1343, 0, "%s", "emission") )
    {
        __debugbreak();
    }
    bytesNeeded = 292 * emission->elemDefCountOneShot;
    elemDefStop = emission->elemDefCountOneShot + emission->elemDefCountLooping;
    for ( elemDefIndex = emission->elemDefCountLooping; elemDefIndex != elemDefStop; ++elemDefIndex )
    {
        elemDef = &emission->elemDefsEA[elemDefIndex];
        if ( elemDef->visStateIntervalCount )
            v2 = elemDef->visStateIntervalCount + 1;
        else
            v2 = 0;
        bytesNeeded += FX_AdditionalBytesNeededForElemDef(
                                         elemDef->elemType,
                                         elemDef->velIntervalCount + 1,
                                         v2,
                                         elemDef->visualCount,
                                         &elemDef->spawnSound);
    }
    return bytesNeeded;
}

int __cdecl FX_GetLoopingLife(const FxEffectDef *effectDef)
{
    const FxElemDef *elemDef; // [esp+4h] [ebp-Ch]
    int elemIndex; // [esp+8h] [ebp-8h]
    int msecLoopingLifeMax; // [esp+Ch] [ebp-4h]

    msecLoopingLifeMax = 0;
    for ( elemIndex = 0; elemIndex < effectDef->elemDefCountLooping; ++elemIndex )
    {
        elemDef = &effectDef->elemDefsEA[elemIndex];
        if ( elemDef->spawn.looping.count == 0x7FFFFFFF )
            return 0x7FFFFFFF;
        if ( msecLoopingLifeMax < elemDef->spawn.looping.intervalMsec * (elemDef->spawn.looping.count - 1) )
            msecLoopingLifeMax = elemDef->spawn.looping.intervalMsec * (elemDef->spawn.looping.count - 1);
    }
    return msecLoopingLifeMax;
}

bool __cdecl FX_Validate(const FxEditorEffectDef *editorEffect, const FxEditorElemDef *edElemDef)
{
    if ( !FX_ValidateAtlasSettings(editorEffect, edElemDef) )
        return 0;
    if ( !FX_ValidateFlags(editorEffect, edElemDef) )
        return 0;
    if ( !FX_ValidateColor(editorEffect, edElemDef) )
        return 0;
    if ( FX_ValidateVisuals(editorEffect, edElemDef) )
        return FX_ValidatePhysics(editorEffect, edElemDef) != 0;
    return 0;
}

char __cdecl FX_ValidateAtlasSettings(const FxEditorEffectDef *editorEffect, const FxEditorElemDef *edElemDef)
{
    MaterialInfo mtlInfoRef; // [esp+0h] [ebp-58h] BYREF
    MaterialInfo mtlInfo; // [esp+28h] [ebp-30h] BYREF
    int visualIndex; // [esp+54h] [ebp-4h]

    if ( !edElemDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_convert.cpp", 629, 0, "%s", "edElemDef") )
    {
        __debugbreak();
    }
    if ( !edElemDef->visualCount )
        return 1;
    if ( !FX_ElemUsesMaterial(edElemDef) )
        return 1;
    Material_GetInfo(edElemDef->visuals[0].material, &mtlInfoRef);
    if ( mtlInfoRef.textureAtlasColumnCount * mtlInfoRef.textureAtlasRowCount <= 256 )
    {
        if ( (edElemDef->atlas.behavior & 3) == 3 && edElemDef->atlas.indexRange > 127 )
        {
            Com_PrintError(
                21,
                "effect '%s' segment '%s':\nmaterial %s. atlas range of %i exceeds %i\n",
                editorEffect->name,
                edElemDef->name,
                mtlInfoRef.name,
                edElemDef->atlas.indexRange,
                127);
            return 0;
        }
        else if ( ((mtlInfoRef.textureAtlasRowCount - 1) & mtlInfoRef.textureAtlasRowCount) != 0
                     || ((mtlInfoRef.textureAtlasColumnCount - 1) & mtlInfoRef.textureAtlasColumnCount) != 0 )
        {
            Com_PrintError(
                21,
                "effect '%s' segment '%s':\nmaterial %s is a %i x %i atlas, which is not a power of 2 on both axes\n",
                editorEffect->name,
                edElemDef->name,
                mtlInfoRef.name,
                mtlInfoRef.textureAtlasColumnCount,
                mtlInfoRef.textureAtlasRowCount);
            return 0;
        }
        else
        {
            for ( visualIndex = 1; visualIndex < edElemDef->visualCount; ++visualIndex )
            {
                Material_GetInfo(edElemDef->visuals[visualIndex].material, &mtlInfo);
                if ( mtlInfo.textureAtlasRowCount != mtlInfoRef.textureAtlasRowCount
                    || mtlInfo.textureAtlasColumnCount != mtlInfoRef.textureAtlasColumnCount )
                {
                    Com_PrintError(
                        21,
                        "effect '%s' segment '%s':\nmaterial %s is a %i x %i atlas, but material %s is a %i x %i atlas\n",
                        editorEffect->name,
                        edElemDef->name,
                        mtlInfoRef.name,
                        mtlInfoRef.textureAtlasColumnCount,
                        mtlInfoRef.textureAtlasRowCount,
                        mtlInfo.name,
                        mtlInfo.textureAtlasColumnCount,
                        mtlInfo.textureAtlasRowCount);
                    return 0;
                }
            }
            return 1;
        }
    }
    else
    {
        Com_PrintError(
            21,
            "effect '%s' segment '%s':\nmaterial %s is a %i x %i atlas, which exceeds 256 max frames per atlas\n",
            editorEffect->name,
            edElemDef->name,
            mtlInfoRef.name,
            mtlInfoRef.textureAtlasColumnCount,
            mtlInfoRef.textureAtlasRowCount);
        return 0;
    }
}

char __cdecl FX_ValidateFlags(const FxEditorEffectDef *editorEffect, const FxEditorElemDef *edElemDef)
{
    if ( (edElemDef->flags & 0xF0) != 0xC0 )
        return 1;
    Com_PrintError(
        21,
        "effect '%s' segment '%s'\nVelocity is 'relative to offset', but generation offset is 'none'\n",
        editorEffect->name,
        edElemDef->name);
    return 0;
}

char __cdecl FX_ValidateColor(const FxEditorEffectDef *editorEffect, const FxEditorElemDef *edElemDef)
{
    if ( edElemDef->elemType == 11 )
    {
        if ( edElemDef->lightingFrac != 0.0 )
        {
            Com_PrintError(
                21,
                "effect '%s' segment '%s'\nDecals cannot have a non-zero lighting fraction.\n",
                editorEffect->name,
                edElemDef->name);
            return 0;
        }
    }
    else
    {
        if ( edElemDef->lightingFrac < 0.0 )
        {
            Com_PrintError(21, "effect '%s' segment '%s'\nNegative lighting fraction.\n", editorEffect->name, edElemDef->name);
            return 0;
        }
        if ( edElemDef->lightingFrac > 1.0 )
        {
            Com_PrintError(
                21,
                "effect '%s' segment '%s'\nLighting fraction larger than 1.0.\n",
                editorEffect->name,
                edElemDef->name);
            return 0;
        }
    }
    return 1;
}

char __cdecl FX_ValidateVisuals(const FxEditorEffectDef *editorEffect, const FxEditorElemDef *edElemDef)
{
    int indIter; // [esp+0h] [ebp-4h]

    if ( (edElemDef->elemType == 11 || edElemDef->elemType == 12) && !edElemDef->visualCount )
    {
        Com_PrintError(
            21,
            "effect '%s' segment '%s'\nThis type of segment must have at least one visual specified.\n",
            editorEffect->name,
            edElemDef->name);
        return 0;
    }
    if ( edElemDef->elemType != 5 )
        return 1;
    if ( !edElemDef->trailDef.indCount || !edElemDef->trailDef.vertCount )
    {
        Com_PrintError(
            21,
            "effect '%s' segment '%s'\nTrail cross-section cannot be empty.\n",
            editorEffect->name,
            edElemDef->name);
        return 0;
    }
    for ( indIter = 0; indIter != edElemDef->trailDef.indCount; ++indIter )
    {
        if ( edElemDef->trailDef.inds[indIter] >= edElemDef->trailDef.vertCount )
        {
            Com_PrintError(
                21,
                "effect '%s' segment '%s'\nIndex references out of range vertex '%i'.\n",
                editorEffect->name,
                edElemDef->name,
                edElemDef->trailDef.inds[indIter]);
            return 0;
        }
    }
    if ( edElemDef->trailRepeatDist <= 0 )
    {
        Com_PrintError(21, "effect '%s' segment '%s'\nTrail repeat dist <= 0.\n", editorEffect->name, edElemDef->name);
        return 0;
    }
    if ( edElemDef->trailSplitDist <= 0 )
    {
        Com_PrintError(21, "effect '%s' segment '%s'\nTrail split dist <= 0.\n", editorEffect->name, edElemDef->name);
        return 0;
    }
    if ( (int)(float)((float)edElemDef->trailRepeatDist * 1000.0) > 0 )
        return 1;
    Com_PrintError(
        21,
        "effect '%s' segment '%s'\nTrail texture repeat dist too close to, or below 0.\n",
        editorEffect->name,
        edElemDef->name);
    return 0;
}

char __cdecl FX_ValidatePhysics(const FxEditorEffectDef *editorEffect, const FxEditorElemDef *edElemDef)
{
    float v3; // [esp+10h] [ebp-18h]
    float amplitude; // [esp+14h] [ebp-14h]
    float elasticityMin; // [esp+20h] [ebp-8h]
    float elasticityMax; // [esp+24h] [ebp-4h]

    if ( (edElemDef->flags & 0x100) == 0 )
        return 1;
    if ( (float)(edElemDef->elasticity.amplitude - 0.0) < 0.0 )
        amplitude = edElemDef->elasticity.amplitude;
    else
        amplitude = 0.0f;
    elasticityMin = edElemDef->elasticity.base + amplitude;
    if ( (float)(0.0 - edElemDef->elasticity.amplitude) < 0.0 )
        v3 = edElemDef->elasticity.amplitude;
    else
        v3 = 0.0f;
    elasticityMax = edElemDef->elasticity.base + v3;
    if ( elasticityMin >= -0.001 && elasticityMax <= 1.001 )
        return 1;
    Com_PrintError(
        21,
        "effect '%s' segment '%s'\nElasticity %g to %g can go outside the range 0 to 1.\n",
        editorEffect->name,
        edElemDef->name,
        elasticityMin,
        elasticityMax);
    return 0;
}

