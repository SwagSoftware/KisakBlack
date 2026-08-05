#include "fx_draw.h"
#include "fx_random.h"
#include "fx_dvars.h"
#include "fx_update_util.h"
#include "fx_archive.h"
#include <gfx_d3d/r_drawsurf.h>
#include <gfx_d3d/r_dvars.h>
#include <gfx_d3d/r_rendercmds.h>
#include "fx_system.h"
#include "fx_sprite.h"
#include "fx_beam.h"
#include "fx_postlight.h"
#include <glass/glass_client.h>
#include <xanim/xanim.h>
#include <gfx_d3d/r_scene.h>

void(__cdecl *const s_drawElemHandler[10])(FxDrawState *) =
{
  &FX_DrawElem_BillboardSprite,
  &FX_DrawElem_OrientedSprite,
  &FX_DrawElem_RotatedSprite,
  &FX_DrawElem_Tail,
  &FX_DrawElem_Line,
  NULL,
  &FX_DrawElem_Cloud,
  &FX_DrawElem_Model,
  &FX_DrawElem_Light,
  &FX_DrawElem_SpotLight
};



int __cdecl FX_RegionForReflect(const FxReflectParams *reflect)
{
    return reflect->enabled ? 16 : 13;
}

void __cdecl FX_GetNullReflection(FxReflectParams *params)
{
    params->enabled = 0;
    params->zAdd = 0.0f;
    params->zScale = 1.0f;
}

void __cdecl FX_GetReflection(bool on, float height, FxReflectParams *params)
{
    params->enabled = on;
    params->zScale = -2.0f;
    params->zAdd = height - (float)(height * params->zScale);
}

void __cdecl FX_EvaluateVisAlpha(FxElemPreVisualState *preVisState, FxElemVisualState *visState)
{
    float valueLerp; // [esp+34h] [ebp-4h]

    valueLerp = fx_randomTable[preVisState->randomSeed + 23];
    visState->color[3] = FX_InterpolateColor(
        preVisState->refState,
        valueLerp,
        1.0 - valueLerp,
        preVisState->sampleLerp,
        preVisState->sampleLerpInv,
        3);
}

unsigned __int8 __cdecl FX_InterpolateColor(
                const FxElemVisStateSample *refState,
                float valueLerp,
                float valueLerpInv,
                float sampleLerp,
                float sampleLerpInv,
                int channel)
{
    return (int)((float)((float)((float)((float)((float)refState->base.color[channel] * valueLerpInv)
                                                                         + (float)((float)refState->amplitude.color[channel] * valueLerp))
                                                         * sampleLerpInv)
                                         + (float)((float)((float)((float)refState[1].base.color[channel] * valueLerpInv)
                                                                         + (float)((float)refState[1].amplitude.color[channel] * valueLerp))
                                                         * sampleLerp))
                         + 9.313225746154785e-10);
}

void __cdecl FX_SetupVisualState(
                const FxElemDef *elemDef,
                const FxEffect *effect,
                int randomSeed,
                float normTimeUpdateEnd,
                FxElemPreVisualState *preVisState)
{
    float v5; // [esp+8h] [ebp-Ch]
    float samplePoint; // [esp+Ch] [ebp-8h]

    samplePoint = (float)elemDef->visStateIntervalCount * normTimeUpdateEnd;
    v5 = floor(samplePoint);
    preVisState->sampleLerp = samplePoint - (float)(int)v5;
    preVisState->sampleLerpInv = 1.0 - preVisState->sampleLerp;
    preVisState->elemDef = elemDef;
    preVisState->effect = effect;
    preVisState->refState = &elemDef->visSamples[(int)v5];
    preVisState->randomSeed = randomSeed;
    preVisState->distanceFade = 255;
}

void __cdecl FX_EvaluateSize(FxElemPreVisualState *preVisState, FxElemVisualState *visState)
{
    float v2; // [esp+Ch] [ebp-14h]
    float v3; // [esp+1Ch] [ebp-4h]

    v3 = fx_randomTable[preVisState->randomSeed + 26];
    visState->size[0] = (float)((float)((float)(v3 * preVisState->refState->amplitude.size[0])
        + preVisState->refState->base.size[0])
        * preVisState->sampleLerpInv)
        + (float)((float)((float)(v3 * preVisState->refState[1].amplitude.size[0])
            + preVisState->refState[1].base.size[0])
            * preVisState->sampleLerp);
    if ((preVisState->elemDef->flags & 0x10000000) != 0)
    {
        v2 = fx_randomTable[preVisState->randomSeed + 27];
        visState->size[1] = (float)((float)((float)(v2 * preVisState->refState->amplitude.size[1])
            + preVisState->refState->base.size[1])
            * preVisState->sampleLerpInv)
            + (float)((float)((float)(v2 * preVisState->refState[1].amplitude.size[1])
                + preVisState->refState[1].base.size[1])
                * preVisState->sampleLerp);
    }
    else
    {
        visState->size[1] = visState->size[0];
    }
}

void __cdecl FX_EvaluateVisualState(
    int localClientNum,
    FxElemPreVisualState *preVisState,
    float msecLifeSpan,
    FxElemVisualState *visState)
{
    float t; // [esp+A8h] [ebp-50h]
    float exposure; // [esp+B0h] [ebp-48h]
    float tintColor; // [esp+BCh] [ebp-3Ch]
    float tintColora; // [esp+BCh] [ebp-3Ch]
    float tintColor_4; // [esp+C0h] [ebp-38h]
    float tintColor_4a; // [esp+C0h] [ebp-38h]
    float tintColor_8; // [esp+C4h] [ebp-34h]
    float tintColor_8a; // [esp+C4h] [ebp-34h]
    const FxElemDef *elemDef; // [esp+CCh] [ebp-2Ch]
    int randomSeed; // [esp+D0h] [ebp-28h]
    float valueLerp; // [esp+D4h] [ebp-24h]
    float lightGridColor_4; // [esp+DCh] [ebp-1Ch]
    float lightGridColor_8; // [esp+E0h] [ebp-18h]
    float sampleLerpInv; // [esp+E8h] [ebp-10h]
    const FxElemVisStateSample *refState; // [esp+ECh] [ebp-Ch]
    float sampleLerp; // [esp+F0h] [ebp-8h]

    elemDef = preVisState->elemDef;
    if (!elemDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_draw.cpp", 257, 0, "%s", "elemDef"))
    {
        __debugbreak();
    }
    refState = preVisState->refState;
    randomSeed = preVisState->randomSeed;
    valueLerp = fx_randomTable[randomSeed + 23];
    sampleLerp = preVisState->sampleLerp;
    sampleLerpInv = preVisState->sampleLerpInv;
    visState->color[0] = FX_InterpolateColor(
        refState,
        valueLerp,
        1.0 - valueLerp,
        preVisState->sampleLerp,
        sampleLerpInv,
        0);
    visState->color[1] = FX_InterpolateColor(refState, valueLerp, 1.0 - valueLerp, sampleLerp, sampleLerpInv, 1);
    visState->color[2] = FX_InterpolateColor(refState, valueLerp, 1.0 - valueLerp, sampleLerp, sampleLerpInv, 2);
    visState->color[3] = FX_InterpolateColor(refState, valueLerp, 1.0 - valueLerp, sampleLerp, sampleLerpInv, 3);
    if (elemDef->lightingFrac)
    {
        tintColor = (float)visState->color[2];
        tintColor_4 = (float)visState->color[1];
        tintColor_8 = (float)visState->color[0];
        exposure = CG_GetLocalClientGlobals(localClientNum)->refdef.exposureValue.blackPoint[3];
        lightGridColor_4 = (float)preVisState->effect->lightG;
        lightGridColor_8 = (float)preVisState->effect->lightB;
        t = (float)elemDef->lightingFrac / 255.0;
        tintColora = FX_LerpFloat((float)((float)preVisState->effect->lightR / 8.0) * exposure, tintColor, t);
        tintColor_4a = FX_LerpFloat((float)(lightGridColor_4 / 8.0) * exposure, tintColor_4, t);
        tintColor_8a = FX_LerpFloat((float)(lightGridColor_8 / 8.0) * exposure, tintColor_8, t);
        visState->color[2] = (int)tintColora;
        visState->color[1] = (int)tintColor_4a;
        visState->color[0] = (int)tintColor_8a;
    }
    visState->rotationTotal = (float)(fx_randomTable[randomSeed + 24] * elemDef->initialRotation.amplitude)
        + elemDef->initialRotation.base;
    visState->rotationTotal = FX_IntegrateRotationFromZero(
        refState,
        randomSeed,
        FXRAND_ROTATION_DELTA,
        sampleLerp,
        msecLifeSpan)
        + visState->rotationTotal;
    visState->color[3] = (unsigned __int16)(LOWORD(preVisState->distanceFade) * visState->color[3]) >> 8;
}

double __cdecl FX_IntegrateRotationFromZero(
                const FxElemVisStateSample *refState,
                int randomSeed,
                FxRandKey randomKey,
                float sampleLerp,
                float msecLifeSpan)
{
    return (float)((float)((float)((float)(fx_randomTable[randomKey + randomSeed] * refState[1].amplitude.rotationDelta)
                                                             + refState[1].base.rotationDelta)
                                             * (float)((float)(sampleLerp * sampleLerp) * 0.5))
                             + (float)((float)((float)((float)(fx_randomTable[randomKey + randomSeed]
                                                                                             * refState->amplitude.rotationDelta)
                                                                             + refState->base.rotationDelta)
                                                             * (float)(sampleLerp - (float)((float)(sampleLerp * sampleLerp) * 0.5)))
                                             + (float)((float)(fx_randomTable[randomKey + randomSeed] * refState->amplitude.rotationTotal)
                                                             + refState->base.rotationTotal)))
             * msecLifeSpan;
}

double __cdecl FX_LerpFloat(float grid, float tint, float fraction)
{
    float result; // [esp+0h] [ebp-4h]

    result = (float)((float)((float)(1.0 - fraction) * (float)(tint / 255.0))
                                 + (float)(sqrtf((float)(tint / 255.0) * (float)(grid / 255.0)) * fraction))
                 * 255.0;
    if ( result > 255.0 )
        result = 255.0f;
    if ( result < 0.0 )
        return 0.0f;
    return result;
}

void __cdecl FX_DrawElem_BillboardSprite(FxDrawState *draw)
{
    if ( !FX_CullElementForDraw_Sprite(draw) )
        FX_DrawElem_BillboardSprite_NoCull(draw);
}

bool __cdecl FX_CullElementForDraw_Sprite(const FxDrawState *draw)
{
    float v2; // [esp+4h] [ebp-44h]
    float v3; // [esp+8h] [ebp-40h]
    float v4; // [esp+Ch] [ebp-3Ch]
    float pivy; // [esp+2Ch] [ebp-1Ch]
    float pivx; // [esp+30h] [ebp-18h]
    float pos[3]; // [esp+34h] [ebp-14h] BYREF
    float radius; // [esp+40h] [ebp-8h]
    unsigned int frustumPlaneCount; // [esp+44h] [ebp-4h]

    if ( !fx_cull_elem_draw->current.enabled )
        return 0;
    frustumPlaneCount = FX_CullElementForDraw_FrustumPlaneCount(draw);
    if ( (float)(draw->visState.size[0] - draw->visState.size[1]) < 0.0 )
        v4 = draw->visState.size[1];
    else
        v4 = draw->visState.size[0];
    radius = v4;
    if ( (draw->elemDef->flags & 0x200000) != 0 )
    {
        pivx = fabs(draw->elemDef->billboardPivot[0]) * draw->visState.size[0];
        pivy = fabs(draw->elemDef->billboardPivot[1]) * draw->visState.size[1];
        if ( (float)(pivx - pivy) < 0.0 )
            v3 = fabs(draw->elemDef->billboardPivot[1]) * draw->visState.size[1];
        else
            v3 = fabs(draw->elemDef->billboardPivot[0]) * draw->visState.size[0];
        if ( (float)(pivy - pivx) < 0.0 )
            v2 = fabs(draw->elemDef->billboardPivot[1]) * draw->visState.size[1];
        else
            v2 = fabs(draw->elemDef->billboardPivot[0]) * draw->visState.size[0];
        radius = (float)((float)(v2 * 0.5) + v3) + radius;
    }
    pos[0] = draw->posWorld[0];
    pos[1] = draw->posWorld[1];
    pos[2] = draw->posWorld[2];
    pos[2] = (float)(pos[2] * draw->reflect.zScale) + draw->reflect.zAdd;
    return FX_CullSphere(draw->camera, frustumPlaneCount, pos, radius) != 0;
}

unsigned int __cdecl FX_CullElementForDraw_FrustumPlaneCount(const FxDrawState *draw)
{
    if ( (!draw || !draw->camera || draw->camera->frustumPlaneCount < 5)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_draw.cpp",
                    768,
                    0,
                    "%s",
                    "draw && draw->camera && draw->camera->frustumPlaneCount >= 5") )
    {
        __debugbreak();
    }
    if ( (draw->elemDef->flags & 0x400) != 0 )
        return 5;
    else
        return draw->camera->frustumPlaneCount;
}

void __cdecl FX_DrawElem_BillboardSprite_NoCull(FxDrawState *draw)
{
    const float *v1; // edx
    const float *origin; // edx
    const FxCamera *v3; // edx
    unsigned int v4; // [esp+0h] [ebp-98h]
    const FxCamera *camera; // [esp+28h] [ebp-70h]
    float normal[3]; // [esp+74h] [ebp-24h] BYREF
    float tangent[3]; // [esp+80h] [ebp-18h] BYREF
    float binormal[3]; // [esp+8Ch] [ebp-Ch] BYREF

    v4 = draw->elemDef->flags & 0x180000;
    if ( v4 > 0x100000 )
    {
        if ( v4 == 1572864 )
        {
            camera = draw->camera;
            normal[0] = camera->origin[0] - draw->posWorld[0];
            normal[1] = camera->origin[1] - draw->posWorld[1];
            normal[2] = camera->origin[2] - draw->posWorld[2];
            Vec3Normalize(normal);
            binormal[0] = 0.0f;
            binormal[1] = 0.0f;
            binormal[2] = 1.0f;
            tangent[0] = -normal[1];
            tangent[1] = normal[0];
            tangent[2] = 0.0f;
            Vec3Normalize(tangent);
        }
    }
    else if ( v4 == 0x100000 )
    {
        normal[0] = draw->camera->origin[0] - draw->posWorld[0];
        normal[1] = draw->camera->origin[1] - draw->posWorld[1];
        normal[2] = draw->camera->origin[2] - draw->posWorld[2];
        Vec3Normalize(normal);
        binormal[0] = draw->camera->axis[2][0];
        binormal[1] = draw->camera->axis[2][1];
        binormal[2] = draw->camera->axis[2][2];
        Vec3Cross(binormal, normal, tangent);
        Vec3Normalize(tangent);
    }
    else if ( v4 )
    {
        if ( v4 == 0x80000 )
        {
            v3 = draw->camera;
            normal[0] = -v3->axis[0][0];
            normal[1] = -v3->axis[0][1];
            normal[2] = -v3->axis[0][2];
            binormal[0] = 0.0f;
            binormal[1] = 0.0f;
            binormal[2] = 1.0f;
            tangent[0] = -normal[1];
            tangent[1] = normal[0];
            tangent[2] = 0.0f;
            Vec3Normalize(tangent);
        }
    }
    else
    {
        //LODWORD(normal[0]) = *(unsigned int *)v8 ^ _mask__NegFloat_;
        //LODWORD(normal[1]) = *((unsigned int *)v8 + 1) ^ _mask__NegFloat_;
        //LODWORD(normal[2]) = *((unsigned int *)v8 + 2) ^ _mask__NegFloat_;
        normal[0] = -draw->camera->axis[0][0];
        normal[1] = -draw->camera->axis[0][1];
        normal[2] = -draw->camera->axis[0][2];

        //LODWORD(tangent[0]) = *(unsigned int *)v7 ^ _mask__NegFloat_;
        //LODWORD(tangent[1]) = *((unsigned int *)v7 + 1) ^ _mask__NegFloat_;
        //LODWORD(tangent[2]) = *((unsigned int *)v7 + 2) ^ _mask__NegFloat_;
        tangent[0] = -draw->camera->axis[1][0];
        tangent[1] = -draw->camera->axis[1][1];
        tangent[2] = -draw->camera->axis[1][2];

        binormal[0] = draw->camera->axis[2][0];
        binormal[1] = draw->camera->axis[2][1];
        binormal[2] = draw->camera->axis[2][2];
    }
    FX_GenSpriteVerts(draw, tangent, binormal, normal);
}

void __cdecl FX_GenSpriteVerts(FxDrawState *draw, const float *tangent, const float *binormal, const float *normal)
{
    __int16 v4; // [esp+8h] [ebp-2BCh]
    __int16 v5; // [esp+Ch] [ebp-2B8h]
    __int16 v6; // [esp+10h] [ebp-2B4h]
    __int16 v7; // [esp+14h] [ebp-2B0h]
    __int16 v8; // [esp+18h] [ebp-2ACh]
    __int16 v9; // [esp+1Ch] [ebp-2A8h]
    __int16 v10; // [esp+20h] [ebp-2A4h]
    __int16 v11; // [esp+24h] [ebp-2A0h]
    int v12; // [esp+2Ch] [ebp-298h]
    int v13; // [esp+40h] [ebp-284h]
    GfxPackedVertex *v14; // [esp+5Ch] [ebp-268h]
    float v15; // [esp+60h] [ebp-264h]
    GfxPackedVertex *v16; // [esp+64h] [ebp-260h]
    GfxPackedVertex *v17; // [esp+68h] [ebp-25Ch]
    int v18; // [esp+70h] [ebp-254h]
    int v19; // [esp+84h] [ebp-240h]
    GfxPackedVertex *v20; // [esp+A0h] [ebp-224h]
    float v21; // [esp+A4h] [ebp-220h]
    GfxPackedVertex *v22; // [esp+A8h] [ebp-21Ch]
    GfxPackedVertex *v23; // [esp+ACh] [ebp-218h]
    int v24; // [esp+B4h] [ebp-210h]
    int v25; // [esp+C8h] [ebp-1FCh]
    GfxPackedVertex *v26; // [esp+E4h] [ebp-1E0h]
    float v27; // [esp+E8h] [ebp-1DCh]
    GfxPackedVertex *v28; // [esp+ECh] [ebp-1D8h]
    GfxPackedVertex *v29; // [esp+F0h] [ebp-1D4h]
    int v30; // [esp+F8h] [ebp-1CCh]
    int v31; // [esp+10Ch] [ebp-1B8h]
    float v32; // [esp+128h] [ebp-19Ch]
    PackedUnitVec v33; // [esp+12Ch] [ebp-198h]
    PackedUnitVec v34; // [esp+148h] [ebp-17Ch]
    float v35; // [esp+1A0h] [ebp-124h]
    float v36; // [esp+1A4h] [ebp-120h]
    float v37; // [esp+1A8h] [ebp-11Ch]
    float v38; // [esp+1ACh] [ebp-118h]
    float rotationTotal; // [esp+1C8h] [ebp-FCh]
    int i; // [esp+1E8h] [ebp-DCh]
    float world; // [esp+210h] [ebp-B4h]
    float world_4; // [esp+214h] [ebp-B0h]
    float world_8; // [esp+218h] [ebp-ACh]
    int t0; // [esp+21Ch] [ebp-A8h] BYREF
    r_double_index_t *baseIndices; // [esp+220h] [ebp-A4h] BYREF
    float dt; // [esp+224h] [ebp-A0h] BYREF
    FxElemVisuals visuals; // [esp+228h] [ebp-9Ch]
    float leftSide[3]; // [esp+22Ch] [ebp-98h]
    PackedUnitVec packedNormal; // [esp+238h] [ebp-8Ch]
    PackedUnitVec packedTangent; // [esp+23Ch] [ebp-88h]
    float rightSide[3]; // [esp+240h] [ebp-84h]
    FxSpriteInfo *sprite; // [esp+24Ch] [ebp-78h]
    float testBinormal[3]; // [esp+250h] [ebp-74h] BYREF
    float bottomSideWidth; // [esp+25Ch] [ebp-68h]
    float bottomSideSoffset; // [esp+260h] [ebp-64h]
    float cosRot; // [esp+264h] [ebp-60h]
    float left[3]; // [esp+268h] [ebp-5Ch]
    float v58; // [esp+274h] [ebp-50h] BYREF
    GfxPackedVertex *verts; // [esp+278h] [ebp-4Ch]
    float sinRot; // [esp+27Ch] [ebp-48h]
    float topSideSoffset; // [esp+280h] [ebp-44h]
    r_double_index_t *indices; // [esp+284h] [ebp-40h]
    r_double_index_t index; // [esp+288h] [ebp-3Ch]
    float rotatedTangent[3]; // [esp+28Ch] [ebp-38h] BYREF
    float up[3]; // [esp+298h] [ebp-2Ch]
    const FxSystem *system; // [esp+2A4h] [ebp-20h]
    float s0; // [esp+2A8h] [ebp-1Ch] BYREF
    float topSideWidth; // [esp+2ACh] [ebp-18h]
    unsigned __int16 baseVertex; // [esp+2B0h] [ebp-14h] BYREF
    float rotatedBinormal[3]; // [esp+2B4h] [ebp-10h]
    GfxPackedVertex *baseVerts; // [esp+2C0h] [ebp-4h]

    system = draw->system;
    sprite = (FxSpriteInfo*)&system->sprite;
    visuals.anonymous = FX_GetElemVisuals(draw->elemDef, draw->randomSeed).anonymous;
    if ( !visuals.anonymous
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_draw.cpp",
                    467,
                    0,
                    "%s",
                    "visuals.material") )
    {
        __debugbreak();
    }
    if ( sprite->material != visuals.anonymous && sprite->indexCount )
    {
        if ( !sprite->name
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_draw.cpp", 476, 0, "%s", "sprite->name") )
        {
            __debugbreak();
        }
        if ( !sprite->material
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_draw.cpp",
                        477,
                        0,
                        "%s",
                        "sprite->material") )
        {
            __debugbreak();
        }
        if ( !sprite->indices
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_draw.cpp",
                        478,
                        0,
                        "%s",
                        "sprite->indices") )
        {
            __debugbreak();
        }
        R_AddCodeMeshDrawSurf(sprite->material, sprite->indices, sprite->indexCount, 0, 0, sprite->name, draw->region);
        sprite->indexCount = 0;
    }
    if ( R_ReserveCodeMeshVerts(4, &baseVertex) && R_ReserveCodeMeshIndices(6, &baseIndices) )
    {
        if ( sprite->material != visuals.anonymous )
        {
            sprite->name = draw->effect->def->name;
            sprite->material = visuals.material;
            sprite->indices = baseIndices;
        }
        sprite->indexCount += 6;
        FX_EvaluateVisualState(draw->system->localClientNum, &draw->preVisState, draw->msecLifeSpan, &draw->visState);
        rotationTotal = draw->visState.rotationTotal;
        cosRot = cos(rotationTotal);
        sinRot = sin(rotationTotal);
        rotatedTangent[0] = (float)(cosRot * *tangent) + (float)(sinRot * *binormal);
        rotatedTangent[1] = (float)(cosRot * tangent[1]) + (float)(sinRot * binormal[1]);
        rotatedTangent[2] = (float)(cosRot * tangent[2]) + (float)(sinRot * binormal[2]);
        rotatedBinormal[0] = (float)(sinRot * *tangent) +   (float)((-(cosRot)) * *binormal);
        rotatedBinormal[1] = (float)(sinRot * tangent[1]) + (float)((-(cosRot)) * binormal[1]);
        rotatedBinormal[2] = (float)(sinRot * tangent[2]) + (float)((-(cosRot)) * binormal[2]);
        v38 = draw->visState.size[0];
        left[0] = v38 * rotatedTangent[0];
        left[1] = v38 * rotatedTangent[1];
        left[2] = v38 * rotatedTangent[2];
        v37 = draw->visState.size[1];
        up[0] = v37 * rotatedBinormal[0];
        up[1] = v37 * rotatedBinormal[1];
        up[2] = v37 * rotatedBinormal[2];
        if ( (draw->elemDef->flags & 0x200000) != 0 )
        {
            v36 = draw->elemDef->billboardPivot[0];
            v35 = draw->elemDef->billboardPivot[1];
            world = draw->posWorld[0] + (float)((float)(v36 * left[0]) + (float)(v35 * up[0]));
            world_4 = draw->posWorld[1] + (float)((float)(v36 * left[1]) + (float)(v35 * up[1]));
            world_8 = draw->posWorld[2] + (float)((float)(v36 * left[2]) + (float)(v35 * up[2]));
            leftSide[0] = world - left[0];
            leftSide[1] = world_4 - left[1];
            leftSide[2] = world_8 - left[2];
            rightSide[0] = world + left[0];
            rightSide[1] = world_4 + left[1];
            rightSide[2] = world_8 + left[2];
        }
        else
        {
            leftSide[0] = draw->posWorld[0] - left[0];
            leftSide[1] = draw->posWorld[1] - left[1];
            leftSide[2] = draw->posWorld[2] - left[2];
            rightSide[0] = draw->posWorld[0] + left[0];
            rightSide[1] = draw->posWorld[1] + left[1];
            rightSide[2] = draw->posWorld[2] + left[2];
        }
        if ( draw->reflect.enabled )
        {
            indices = baseIndices;
            index.value[0] = baseVertex;
            index.value[1] = baseVertex + 3;
            *baseIndices = index;
            ++indices;
            index.value[0] = baseVertex + 2;
            index.value[1] = baseVertex + 2;
            *indices++ = index;
            index.value[0] = baseVertex + 1;
        }
        else
        {
            indices = baseIndices;
            index.value[0] = baseVertex;
            index.value[1] = baseVertex + 1;
            *baseIndices = index;
            ++indices;
            index.value[0] = baseVertex + 2;
            index.value[1] = baseVertex + 2;
            *indices++ = index;
            index.value[0] = baseVertex + 3;
        }
        index.value[1] = baseVertex;
        *indices++ = index;
        leftSide[2] = (float)(leftSide[2] * draw->reflect.zScale) + draw->reflect.zAdd;
        rightSide[2] = (float)(rightSide[2] * draw->reflect.zScale) + draw->reflect.zAdd;
        up[2] = up[2] * draw->reflect.zScale;
        FX_GetSpriteTexCoords(draw, &s0, &v58, (float *)&t0, &dt);
        v34.array[0] = (int)(float)((float)(*normal * 127.0) + 127.5);
        v34.array[1] = (int)(float)((float)(normal[1] * 127.0) + 127.5);
        v34.array[2] = (int)(float)((float)(normal[2] * 127.0) + 127.5);
        v34.array[3] = 63;
        packedNormal.packed = v34.packed;
        v33.array[0] = (int)(float)((float)(rotatedTangent[0] * 127.0) + 127.5);
        v33.array[1] = (int)(float)((float)(rotatedTangent[1] * 127.0) + 127.5);
        v33.array[2] = (int)(float)((float)(rotatedTangent[2] * 127.0) + 127.5);
        v33.array[3] = 63;
        packedTangent.packed = v33.packed;
        topSideWidth = draw->elemDef->u.billboard.topWidth;
        topSideSoffset = (float)(1.0 - topSideWidth) * 0.5;
        bottomSideWidth = draw->elemDef->u.billboard.bottomWidth;
        bottomSideSoffset = (float)(1.0 - bottomSideWidth) * 0.5;
        Vec3Cross(normal, rotatedTangent, testBinormal);
        if ( (float)((float)((float)(testBinormal[0] * rotatedBinormal[0]) + (float)(testBinormal[1] * rotatedBinormal[1]))
                             + (float)(testBinormal[2] * rotatedBinormal[2])) > 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_draw.cpp",
                        585,
                        0,
                        "%s",
                        "Vec3Dot( testBinormal, rotatedBinormal ) <= 0.0f") )
        {
            __debugbreak();
        }
        baseVerts = R_GetCodeMeshVerts(baseVertex);
        verts = baseVerts;
        for ( i = 0; i < 4; ++i )
        {
            verts[i].binormalSign = -1.0f;
            verts[i].color.packed = *(unsigned int *)draw->visState.color;
            verts[i].normal = packedNormal;
            verts[i].tangent = packedTangent;
        }
        verts->xyz[0] = leftSide[0] + up[0];
        verts->xyz[1] = leftSide[1] + up[1];
        verts->xyz[2] = leftSide[2] + up[2];
        v32 = 1.0 - bottomSideWidth;
        verts->xyz[0] = (float)((float)(1.0 - bottomSideWidth) * left[0]) + verts->xyz[0];
        verts->xyz[1] = (float)(v32 * left[1]) + verts->xyz[1];
        verts->xyz[2] = (float)(v32 * left[2]) + verts->xyz[2];
        verts->color.packed = *(unsigned int *)draw->visState.color;

        if ( (int)((2 * COERCE_INT((float)(v58 * bottomSideSoffset) + s0)) ^ 0x80000000) >> 14 < 0x3FFF )
            v31 = (int)((2 * COERCE_INT((float)(v58 * bottomSideSoffset) + s0)) ^ 0x80000000) >> 14;
        else
            v31 = 0x3FFF;
        if ( v31 > -16384 )
            v11 = v31;
        else
            v11 = -16384;
        if ( (int)((2 * COERCE_INT(*(float *)&t0 + dt)) ^ 0x80000000) >> 14 < 0x3FFF )
            v30 = (int)((2 * COERCE_INT(*(float *)&t0 + dt)) ^ 0x80000000) >> 14;
        else
            v30 = 0x3FFF;
        if ( v30 > -16384 )
            v10 = v30;
        else
            v10 = -16384;
        verts->texCoord.packed = (v10 & 0x3FFF | (int(*(float *)&t0 + dt) >> 16) & 0xC000) + ((v11 & 0x3FFF | (int((float)(v58 * bottomSideSoffset) + s0) >> 16) & 0xC000) << 16);
        v29 = verts + 1;
        verts[1].xyz[0] = leftSide[0] - up[0];
        v29->xyz[1] = leftSide[1] - up[1];
        v29->xyz[2] = leftSide[2] - up[2];
        v26 = verts + 1;
        v27 = 1.0 - topSideWidth;
        v28 = verts + 1;
        verts[1].xyz[0] = (float)((float)(1.0 - topSideWidth) * left[0]) + verts[1].xyz[0];
        v26->xyz[1] = (float)(v27 * left[1]) + v28->xyz[1];
        v26->xyz[2] = (float)(v27 * left[2]) + v28->xyz[2];
        verts[1].color.packed = *(unsigned int *)draw->visState.color;
        if ( (int)((2 * COERCE_INT((float)(v58 * topSideSoffset) + s0)) ^ 0x80000000) >> 14 < 0x3FFF )
            v25 = (int)((2 * COERCE_INT((float)(v58 * topSideSoffset) + s0)) ^ 0x80000000) >> 14;
        else
            v25 = 0x3FFF;
        if ( v25 > -16384 )
            v9 = v25;
        else
            v9 = -16384;
        if ( (int)((2 * t0) ^ 0x80000000) >> 14 < 0x3FFF )
            v24 = (int)((2 * t0) ^ 0x80000000) >> 14;
        else
            v24 = 0x3FFF;
        if ( v24 > -16384 )
            v8 = v24;
        else
            v8 = -16384;
        verts[1].texCoord.packed = (v8 & 0x3FFF | (t0 >> 16) & 0xC000)
                                                         + ((v9 & 0x3FFF | (int((float)(v58 * topSideSoffset) + s0) >> 16) & 0xC000) << 16);
        v23 = verts + 2;
        verts[2].xyz[0] = rightSide[0] - up[0];
        v23->xyz[1] = rightSide[1] - up[1];
        v23->xyz[2] = rightSide[2] - up[2];
        v20 = verts + 2;
        //LODWORD(v21) = COERCE_UNSIGNED_INT(1.0 - topSideWidth) ^ _mask__NegFloat_;
        v21 = -(1.0 - topSideWidth);
        v22 = verts + 2;
        verts[2].xyz[0] = (float)(v21 * left[0]) + verts[2].xyz[0];
        v20->xyz[1] = (float)(v21 * left[1]) + v22->xyz[1];
        v20->xyz[2] = (float)(v21 * left[2]) + v22->xyz[2];
        verts[2].color.packed = *(unsigned int *)draw->visState.color;
        if ( (int)((2 * COERCE_INT((float)((float)(1.0 - topSideSoffset) * v58) + s0)) ^ 0x80000000) >> 14 < 0x3FFF )
            v19 = (int)((2 * COERCE_INT((float)((float)(1.0 - topSideSoffset) * v58) + s0)) ^ 0x80000000) >> 14;
        else
            v19 = 0x3FFF;
        if ( v19 > -16384 )
            v7 = v19;
        else
            v7 = -16384;
        if ( (int)((2 * t0) ^ 0x80000000) >> 14 < 0x3FFF )
            v18 = (int)((2 * t0) ^ 0x80000000) >> 14;
        else
            v18 = 0x3FFF;
        if ( v18 > -16384 )
            v6 = v18;
        else
            v6 = -16384;
        verts[2].texCoord.packed = (v6 & 0x3FFF | (t0 >> 16) & 0xC000) + ((v7 & 0x3FFF | (int((float)((float)(1.0 - topSideSoffset) * v58) + s0) >> 16) & 0xC000) << 16);
        v17 = verts + 3;
        verts[3].xyz[0] = rightSide[0] + up[0];
        v17->xyz[1] = rightSide[1] + up[1];
        v17->xyz[2] = rightSide[2] + up[2];
        v14 = verts + 3;
        //LODWORD(v15) = COERCE_UNSIGNED_INT(1.0 - bottomSideWidth) ^ _mask__NegFloat_;
        v15 = -(1.0 - bottomSideWidth);
        v16 = verts + 3;
        verts[3].xyz[0] = (float)(v15 * left[0]) + verts[3].xyz[0];
        v14->xyz[1] = (float)(v15 * left[1]) + v16->xyz[1];
        v14->xyz[2] = (float)(v15 * left[2]) + v16->xyz[2];
        if ( (int)((2 * COERCE_INT((float)((float)(1.0 - bottomSideSoffset) * v58) + s0)) ^ 0x80000000) >> 14 < 0x3FFF )
            v13 = (int)((2 * COERCE_INT((float)((float)(1.0 - bottomSideSoffset) * v58) + s0)) ^ 0x80000000) >> 14;
        else
            v13 = 0x3FFF;
        if ( v13 > -16384 )
            v5 = v13;
        else
            v5 = -16384;
        if ( (int)((2 * COERCE_INT(*(float *)&t0 + dt)) ^ 0x80000000) >> 14 < 0x3FFF )
            v12 = (int)((2 * COERCE_INT(*(float *)&t0 + dt)) ^ 0x80000000) >> 14;
        else
            v12 = 0x3FFF;
        if ( v12 > -16384 )
            v4 = v12;
        else
            v4 = -16384;
        verts[3].texCoord.packed = (v4 & 0x3FFF | (int(*(float *)&t0 + dt) >> 16) & 0xC000)
                                                         + ((v5 & 0x3FFF
                                                             | (int((float)((float)(1.0 - bottomSideSoffset) * v58) + s0) >> 16) & 0xC000) << 16);
    }
}

void __cdecl FX_GetSpriteTexCoords(const FxDrawState *draw, float *s0, float *ds, float *t0, float *dt)
{
    unsigned __int8 colBits; // [esp+10h] [ebp-24h]
    int atlasIndex; // [esp+14h] [ebp-20h]
    int atlasIndexa; // [esp+14h] [ebp-20h]
    const FxElemDef *elemDef; // [esp+18h] [ebp-1Ch]
    int startBehavior; // [esp+20h] [ebp-14h]
    int atlasCount; // [esp+24h] [ebp-10h]
    int atlasIndexRange; // [esp+28h] [ebp-Ch]
    float _dt; // [esp+30h] [ebp-4h]

    if ( !draw && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_draw.cpp", 343, 0, "%s", "draw") )
        __debugbreak();
    elemDef = draw->elemDef;
    if ( !elemDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_draw.cpp", 345, 0, "%s", "elemDef") )
    {
        __debugbreak();
    }
    atlasCount = elemDef->atlas.entryCountAndIndexRange & 0x1FF;
    if ( atlasCount == 1 )
    {
        *s0 = 0.0f;
        *ds = 1.0f;
        *t0 = 0.0f;
        *dt = 1.0f;
    }
    else
    {
        if ( ((elemDef->atlas.entryCountAndIndexRange & 0x1FFu) <= 1
             || (elemDef->atlas.entryCountAndIndexRange & 0x1FFu) > 0x100
             || (atlasCount & (atlasCount - 1)) != 0)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_draw.cpp",
                        356,
                        0,
                        "%s\n\t(atlasCount) = %i",
                        "(atlasCount > 1 && atlasCount <= FxElemAtlas::ENTRY_COUNT_MAX && (((atlasCount) & ((atlasCount) - 1)) == 0))",
                        atlasCount) )
        {
            __debugbreak();
        }
        startBehavior = elemDef->atlas.behavior & 3;
        atlasIndexRange = atlasCount;
        if ( (elemDef->atlas.behavior & 3) != 0 )
        {
            if ( startBehavior == 1 )
            {
                atlasIndex = (atlasCount * LOWORD(fx_randomTable[draw->randomSeed + 22])) >> 16;
            }
            else if ( startBehavior == 3 )
            {
                atlasIndex = 0;
                atlasIndexRange = ((int)elemDef->atlas.entryCountAndIndexRange >> 9) & 0x7F;
            }
            else
            {
                if ( startBehavior != 2
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_draw.cpp",
                                377,
                                1,
                                "startBehavior == FX_ATLAS_START_INDEXED\n\t%i, %i",
                                startBehavior,
                                2) )
                {
                    __debugbreak();
                }
                atlasIndex = (atlasCount - 1) & draw->elem->sequence;
            }
        }
        else
        {
            atlasIndex = elemDef->atlas.index;
        }
        if ( (elemDef->atlas.behavior & 4) != 0 )
        {
            atlasIndex += (int)(float)((float)(draw->msecElapsed / draw->msecLifeSpan) * (float)atlasIndexRange);
        }
        else if ( elemDef->atlas.fps )
        {
            atlasIndex += elemDef->atlas.fps * (int)draw->msecElapsed / 1000;
        }
        if ( startBehavior == 3 )
        {
            if ( (elemDef->atlas.behavior & 8) != 0 && atlasIndex >= atlasIndexRange * elemDef->atlas.loopCount )
                atlasIndex = atlasIndexRange - 1;
            atlasIndex = atlasIndex % atlasIndexRange + elemDef->atlas.index;
        }
        else if ( (elemDef->atlas.behavior & 8) != 0 && atlasIndex >= atlasCount * elemDef->atlas.loopCount )
        {
            atlasIndex = atlasCount - 1;
        }
        atlasIndexa = atlasIndex & (atlasCount - 1);
        colBits = elemDef->atlas.colIndexBits;
        _dt = 1.0 / (float)(1 << elemDef->atlas.rowIndexBits);
        *ds = 1.0 / (float)(1 << colBits);
        *s0 = (float)(atlasIndexa & ((1 << colBits) - 1)) * (float)(1.0 / (float)(1 << colBits));
        *dt = _dt;
        *t0 = (float)(atlasIndexa >> colBits) * _dt;
    }
}

void __cdecl FX_DrawElem_OrientedSprite(FxDrawState *draw)
{
    if ( !FX_CullElementForDraw_Sprite(draw) )
        FX_DrawElem_OrientedSprite_NoCull(draw);
}

void __cdecl FX_DrawElem_OrientedSprite_NoCull(FxDrawState *draw)
{
    FX_GenSpriteVerts(draw, draw->orient.axis[1], draw->orient.axis[2], draw->orient.axis[0]);
}

void __cdecl FX_DrawElem_Tail(FxDrawState *draw)
{
    FX_GetVelocityAtTime(
        draw->elemDef,
        draw->randomSeed,
        draw->msecLifeSpan,
        draw->msecElapsed,
        &draw->orient,
        draw->elem->baseVel,
        draw->velDirWorld);
    Vec3Normalize(draw->velDirWorld);
    if ( !FX_CullElementForDraw_Tail(draw) )
        FX_DrawElem_Tail_Main(draw);
}

bool __cdecl FX_CullElementForDraw_Tail(const FxDrawState *draw)
{
    bool result; // al
    float v2; // [esp+8h] [ebp-18h]
    float endpoint[3]; // [esp+10h] [ebp-10h] BYREF
    unsigned int frustumPlaneCount; // [esp+1Ch] [ebp-4h]

    result = 0;
    if ( fx_cull_elem_draw->current.enabled )
    {
        frustumPlaneCount = FX_CullElementForDraw_FrustumPlaneCount(draw);
        v2 = draw->visState.size[1] * -2.0;
        endpoint[0] = (float)(v2 * draw->velDirWorld[0]) + draw->posWorld[0];
        endpoint[1] = (float)(v2 * draw->velDirWorld[1]) + draw->posWorld[1];
        endpoint[2] = (float)(v2 * draw->velDirWorld[2]) + draw->posWorld[2];
        if ( FX_CullCylinder(draw->camera, frustumPlaneCount, draw->posWorld, endpoint, draw->visState.size[0]) )
            return 1;
    }
    return result;
}

char __cdecl FX_CullCylinder(
                const FxCamera *camera,
                unsigned int frustumPlaneCount,
                const float *posWorld0,
                const float *posWorld1,
                float radius)
{
    const char *v5; // eax
    double v6; // st7
    const char *v7; // eax
    unsigned int planeIndex; // [esp+34h] [ebp-4h]

    if ( !camera->isValid
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_draw.cpp", 898, 0, "%s", "camera->isValid") )
    {
        __debugbreak();
    }
    if ( frustumPlaneCount != camera->frustumPlaneCount && frustumPlaneCount != 5 )
    {
        v5 = va("%i, %i", frustumPlaneCount, camera->frustumPlaneCount);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_draw.cpp",
                        899,
                        0,
                        "%s\n\t%s",
                        "frustumPlaneCount == camera->frustumPlaneCount || frustumPlaneCount == 5",
                        v5) )
            __debugbreak();
    }
    for ( planeIndex = 0; planeIndex < frustumPlaneCount; ++planeIndex )
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
                            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_draw.cpp",
                            903,
                            0,
                            "%s\n\t%s",
                            "Vec3IsNormalized( camera->frustum[planeIndex] )",
                            v7) )
                __debugbreak();
        }
        if ( (float)((float)((float)((float)(camera->frustum[planeIndex][0] * *posWorld0)
                                                             + (float)(camera->frustum[planeIndex][1] * posWorld0[1]))
                                             + (float)(camera->frustum[planeIndex][2] * posWorld0[2]))
                             - camera->frustum[planeIndex][3]) <= (-(radius))
            && (float)((float)((float)((float)(camera->frustum[planeIndex][0] * *posWorld1)
                                                             + (float)(camera->frustum[planeIndex][1] * posWorld1[1]))
                                             + (float)(camera->frustum[planeIndex][2] * posWorld1[2]))
                             - camera->frustum[planeIndex][3]) <= (-(radius)) )
        {
            return 1;
        }
    }
    return 0;
}

void __cdecl FX_DrawElem_Tail_Main(FxDrawState *draw)
{
    const FxCamera *camera; // [esp+10h] [ebp-38h]
    float v2; // [esp+1Ch] [ebp-2Ch]
    float normal[3]; // [esp+24h] [ebp-24h] BYREF
    float deltaCamera[3]; // [esp+30h] [ebp-18h] BYREF
    float tangent[3]; // [esp+3Ch] [ebp-Ch] BYREF

    v2 = -draw->visState.size[1];
    draw->posWorld[0] = (float)(v2 * draw->velDirWorld[0]) + draw->posWorld[0];
    draw->posWorld[1] = (float)(v2 * draw->velDirWorld[1]) + draw->posWorld[1];
    draw->posWorld[2] = (float)(v2 * draw->velDirWorld[2]) + draw->posWorld[2];
    camera = draw->camera;
    deltaCamera[0] = camera->origin[0] - draw->posWorld[0];
    deltaCamera[1] = camera->origin[1] - draw->posWorld[1];
    deltaCamera[2] = camera->origin[2] - draw->posWorld[2];
    Vec3Cross(draw->velDirWorld, deltaCamera, tangent);
    Vec3Normalize(tangent);
    Vec3Cross(tangent, draw->velDirWorld, normal);
    FX_GenSpriteVerts(draw, tangent, draw->velDirWorld, normal);
}

void __cdecl FX_DrawElem_Line(FxDrawState *draw)
{
    FX_GetVelocityAtTime(
        draw->elemDef,
        draw->randomSeed,
        draw->msecLifeSpan,
        draw->msecElapsed,
        &draw->orient,
        draw->elem->baseVel,
        draw->velDirWorld);
    Vec3Normalize(draw->velDirWorld);
    if ( !FX_CullElement_Line(draw) )
        FX_DrawElem_Line_Main(draw);
}

bool __cdecl FX_CullElement_Line(const FxDrawState *draw)
{
    bool result; // al
    float v2; // [esp+8h] [ebp-18h]
    float endpoint[3]; // [esp+10h] [ebp-10h] BYREF
    unsigned int frustumPlaneCount; // [esp+1Ch] [ebp-4h]

    result = 0;
    if ( fx_cull_elem_draw->current.enabled )
    {
        frustumPlaneCount = FX_CullElementForDraw_FrustumPlaneCount(draw);
        v2 = draw->visState.size[1] * 2.0;
        endpoint[0] = (float)(v2 * draw->velDirWorld[0]) + draw->posWorld[0];
        endpoint[1] = (float)(v2 * draw->velDirWorld[1]) + draw->posWorld[1];
        endpoint[2] = (float)(v2 * draw->velDirWorld[2]) + draw->posWorld[2];
        if ( FX_CullCylinder(draw->camera, frustumPlaneCount, draw->posWorld, endpoint, draw->visState.size[0]) )
            return 1;
    }
    return result;
}

void __cdecl FX_DrawElem_Line_Main(FxDrawState *draw)
{
    const FxCamera *camera; // [esp+10h] [ebp-38h]
    float v2; // [esp+1Ch] [ebp-2Ch]
    float normal[3]; // [esp+24h] [ebp-24h] BYREF
    float deltaCamera[3]; // [esp+30h] [ebp-18h] BYREF
    float tangent[3]; // [esp+3Ch] [ebp-Ch] BYREF

    v2 = draw->visState.size[1];
    draw->posWorld[0] = (float)(v2 * draw->velDirWorld[0]) + draw->posWorld[0];
    draw->posWorld[1] = (float)(v2 * draw->velDirWorld[1]) + draw->posWorld[1];
    draw->posWorld[2] = (float)(v2 * draw->velDirWorld[2]) + draw->posWorld[2];
    camera = draw->camera;
    deltaCamera[0] = camera->origin[0] - draw->posWorld[0];
    deltaCamera[1] = camera->origin[1] - draw->posWorld[1];
    deltaCamera[2] = camera->origin[2] - draw->posWorld[2];
    Vec3Cross(draw->velDirWorld, deltaCamera, tangent);
    Vec3Normalize(tangent);
    Vec3Cross(tangent, draw->velDirWorld, normal);
    FX_GenSpriteVerts(draw, tangent, draw->velDirWorld, normal);
}

void __cdecl FX_DrawElem_RotatedSprite(FxDrawState *draw)
{
    if ( !FX_CullElementForDraw_Sprite(draw) )
        FX_DrawElem_RotatedSprite_NoCull(draw);
}

void __cdecl FX_DrawElem_RotatedSprite_NoCull(FxDrawState *draw)
{
    float angles[3]; // [esp+2Ch] [ebp-30h] BYREF
    float axis[3][3]; // [esp+38h] [ebp-24h] BYREF

    angles[0] = (float)(fx_randomTable[draw->randomSeed + 12] * draw->elemDef->spawnAngles[0].amplitude)
        + draw->elemDef->spawnAngles[0].base;
    angles[1] = (float)(fx_randomTable[draw->randomSeed + 13] * draw->elemDef->spawnAngles[1].amplitude)
        + draw->elemDef->spawnAngles[1].base;
    angles[2] = (float)(fx_randomTable[draw->randomSeed + 14] * draw->elemDef->spawnAngles[2].amplitude)
        + draw->elemDef->spawnAngles[2].base;
    FX_AnglesToOrientedAxis(angles, &draw->orient, axis);
    *(_QWORD *)&draw->orient.axis[0][0] = *(_QWORD *)&axis[0][0];
    draw->orient.axis[0][2] = axis[0][2];
    draw->orient.axis[1][0] = axis[1][0];
    draw->orient.axis[1][1] = axis[1][1];
    draw->orient.axis[1][2] = axis[1][2];
    draw->orient.axis[2][0] = axis[2][0];
    draw->orient.axis[2][1] = axis[2][1];
    draw->orient.axis[2][2] = axis[2][2];
    FX_GenSpriteVerts(draw, draw->orient.axis[1], draw->orient.axis[2], draw->orient.axis[0]);
}

void __cdecl FX_DrawElem_Cloud(FxDrawState *draw)
{
    float v2; // [esp+20h] [ebp-10h]

    if (fx_drawClouds->current.enabled)
    {
        FX_GetVelocityAtTime(
            draw->elemDef,
            draw->randomSeed,
            draw->msecLifeSpan,
            draw->msecElapsed,
            &draw->orient,
            draw->elem->baseVel,
            draw->velDirWorld);
        Vec3Normalize(draw->velDirWorld);
        v2 = fx_randomTable[draw->preVisState.randomSeed + 28];
        draw->visState.scale = (float)((float)((float)(v2 * draw->preVisState.refState->amplitude.scale)
            + draw->preVisState.refState->base.scale)
            * draw->preVisState.sampleLerpInv)
            + (float)((float)((float)(v2 * draw->preVisState.refState[1].amplitude.scale)
                + draw->preVisState.refState[1].base.scale)
                * draw->preVisState.sampleLerp);
        if (draw->visState.scale != 0.0 && !FX_CullElementForDraw_Cloud(draw))
            FX_DrawElem_Cloud_Main(draw);
    }
}

void __cdecl FX_DrawElem_Cloud_Main(FxDrawState *draw)
{
    int v1; // [esp+8h] [ebp-40h]
    int v2; // [esp+24h] [ebp-24h]
    GfxParticleCloud *cloud; // [esp+2Ch] [ebp-1Ch]
    FxElemVisuals visuals; // [esp+30h] [ebp-18h]
    unsigned int offset; // [esp+3Ch] [ebp-Ch]

    if ( draw->elemDef->u.cloudDensityRange.base
         + (((draw->elemDef->u.cloudDensityRange.amplitude + 1) * LOWORD(fx_randomTable[draw->randomSeed + 29])) >> 16) < 1024 )
        v2 = draw->elemDef->u.cloudDensityRange.base
             + (((draw->elemDef->u.cloudDensityRange.amplitude + 1) * LOWORD(fx_randomTable[draw->randomSeed + 29])) >> 16);
    else
        v2 = 1024;
    if ( v2 > 0 )
        v1 = v2;
    else
        v1 = 0;
    if ( v1 )
    {
        offset = ((1024 - v1 + 1) * LOWORD(fx_randomTable[draw->randomSeed + 29])) >> 16;
        _InterlockedExchangeAdd((volatile unsigned long long* )&draw->system->gfxCloudCount, 1u);
        visuals.anonymous = FX_GetElemVisuals(draw->elemDef, draw->randomSeed).anonymous;
        if ( !visuals.anonymous
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_draw.cpp",
                        1151,
                        0,
                        "%s",
                        "visuals.material") )
        {
            __debugbreak();
        }
        cloud = R_AddParticleCloudToScene(visuals.material);
        if ( cloud )
        {
            FX_EvaluateVisualState(draw->system->localClientNum, &draw->preVisState, draw->msecLifeSpan, &draw->visState);
            FX_SetPlacement(draw, &cloud->placement);
            cloud->color.packed = *(unsigned int *)draw->visState.color;
            *(double *)cloud->radius = *(double *)draw->visState.size;
            cloud->count = v1;
            cloud->offset = offset;
            cloud->gaussian = (draw->elemDef->flags & 0x400000) != 0;
            cloud->endpos[0] = draw->posWorld[0] - draw->velDirWorld[0];
            cloud->endpos[1] = draw->posWorld[1] - draw->velDirWorld[1];
            cloud->endpos[2] = draw->posWorld[2] - draw->velDirWorld[2];
        }
    }
}

void __cdecl FX_SetPlacement(const FxDrawState *draw, GfxScaledPlacement *placement)
{
    float axis[3][3]; // [esp+14h] [ebp-28h] BYREF
    float msecForAxis; // [esp+38h] [ebp-4h]

    msecForAxis = FX_GetMsecForSamplingAxis(draw->msecElapsed, draw->msecLifeSpan, draw->elem->atRestFraction);
    FX_GetElemAxis(draw->elemDef, draw->randomSeed, &draw->orient, msecForAxis, axis);
    AxisToQuat(axis, placement->base.quat);
    placement->base.origin[0] = draw->posWorld[0];
    placement->base.origin[1] = draw->posWorld[1];
    placement->base.origin[2] = draw->posWorld[2];
    placement->scale = draw->visState.scale;
}

double __cdecl FX_GetMsecForSamplingAxis(float msecElapsed, float msecLifeSpan, int atRestFraction)
{
    float msecAtRest; // [esp+0h] [ebp-8h]
    float msecSinceAtRest; // [esp+4h] [ebp-4h]

    msecAtRest = (float)((float)atRestFraction * msecLifeSpan) * 0.0039215689;
    msecSinceAtRest = msecElapsed - msecAtRest;
    if ( (float)(msecElapsed - msecAtRest) <= 0.0 )
        return msecElapsed;
    if ( msecSinceAtRest < 300.0 )
        return msecElapsed - msecSinceAtRest * msecSinceAtRest * 0.0016666667;
    return msecAtRest + 150.0;
}

bool __cdecl FX_CullElementForDraw_Cloud(const FxDrawState *draw)
{
    bool result; // al
    float v2; // [esp+4h] [ebp-14h]
    unsigned int frustumPlaneCount; // [esp+14h] [ebp-4h]

    result = 0;
    if ( fx_cull_elem_draw->current.enabled )
    {
        frustumPlaneCount = FX_CullElementForDraw_FrustumPlaneCount(draw);
        v2 = (float)(draw->visState.size[0] - draw->visState.size[1]) < 0.0
             ? draw->visState.size[1]
             : draw->visState.size[0];
        if ( FX_CullSphere(draw->camera, frustumPlaneCount, draw->posWorld, draw->visState.scale + v2) )
            return 1;
    }
    return result;
}

void __cdecl FX_DrawElem_Model(FxDrawState *draw)
{
    float v2; // [esp+18h] [ebp-30h]
    FxElemVisuals visuals; // [esp+20h] [ebp-28h]
    GfxScaledPlacement placement; // [esp+28h] [ebp-20h] BYREF

    v2 = fx_randomTable[draw->preVisState.randomSeed + 28];
    draw->visState.scale = (float)((float)((float)(v2 * draw->preVisState.refState->amplitude.scale)
        + draw->preVisState.refState->base.scale)
        * draw->preVisState.sampleLerpInv)
        + (float)((float)((float)(v2 * draw->preVisState.refState[1].amplitude.scale)
            + draw->preVisState.refState[1].base.scale)
            * draw->preVisState.sampleLerp);
    if (draw->visState.scale != 0.0)
    {
        FX_SetPlacement(draw, &placement);
        if (draw->elemDef->elemType != 7
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_draw.cpp",
                1260,
                0,
                "%s\n\t(draw->elemDef->elemType) = %i",
                "(draw->elemDef->elemType == FX_ELEM_TYPE_MODEL)",
                draw->elemDef->elemType))
        {
            __debugbreak();
        }
        if ((draw->elemDef->flags & 0x8000000) != 0)
            FX_SetPlacementFromPhysics(draw, &placement.base);
        visuals.anonymous = FX_GetElemVisuals(draw->elemDef, draw->randomSeed).anonymous;
        if (!visuals.anonymous
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_draw.cpp",
                1265,
                0,
                "%s",
                "visuals.model"))
        {
            __debugbreak();
        }
        R_FilterXModelIntoScene(visuals.model, &placement, 0, (unsigned __int16 *)&draw->elem->u, 36.0);
    }
}

void __cdecl FX_SetPlacementFromPhysics(const FxDrawState *draw, GfxPlacement *placement)
{
    Sys_EnterCriticalSection(CRITSECT_PHYSICS);
    Phys_ObjGetInterpolatedState(draw->elem->physObjId, placement->origin, placement->quat);
    Sys_LeaveCriticalSection(CRITSECT_PHYSICS);
}

void __cdecl FX_DrawElem_Light(FxDrawState *draw)
{
    if ( !FX_CullElementForDraw_Light(draw) )
        FX_DrawElem_Light_NoCull(draw);
}

bool __cdecl FX_CullElementForDraw_Light(const FxDrawState *draw)
{
    bool result; // al
    unsigned int frustumPlaneCount; // [esp+4h] [ebp-4h]

    result = 0;
    if ( fx_cull_elem_draw->current.enabled )
    {
        frustumPlaneCount = FX_CullElementForDraw_FrustumPlaneCount(draw);
        if ( FX_CullSphere(draw->camera, frustumPlaneCount, draw->posWorld, draw->visState.size[0]) )
            return 1;
    }
    return result;
}

void __cdecl FX_DrawElem_Light_NoCull(FxDrawState *draw)
{
    float dummyAxis[3][3]; // [esp+1Ch] [ebp-24h] BYREF

    memset(dummyAxis, 0, sizeof(dummyAxis));
    FX_EvaluateVisualState(draw->system->localClientNum, &draw->preVisState, draw->msecLifeSpan, &draw->visState);
    R_AddOmniLightToScene(
        draw->posWorld,
        dummyAxis,
        int(draw->visState.size[0]),
        (float)draw->visState.color[2] * 0.0039215689,
        (float)draw->visState.color[1] * 0.0039215689,
        (float)draw->visState.color[0] * 0.0039215689);
}

void __cdecl FX_DrawElem_SpotLight(FxDrawState *draw)
{
    draw->visState.size[0] = r_flashLightRange->current.value;
    if ( !FX_CullElementForDraw_Light(draw) )
        FX_DrawElem_SpotLight_NoCull(draw);
}

void __cdecl FX_DrawElem_SpotLight_NoCull(FxDrawState *draw)
{
    FX_EvaluateVisualState(draw->system->localClientNum, &draw->preVisState, draw->msecLifeSpan, &draw->visState);
    R_AddSpotLightToScene(
        draw->posWorld,
        draw->orient.axis,
        draw->visState.size[0],
        (float)draw->visState.color[2] * 0.0039215689,
        (float)draw->visState.color[1] * 0.0039215689,
        (float)draw->visState.color[0] * 0.0039215689);
}

void __cdecl FX_DrawNonSpriteElems(FxSystem *system, const FxReflectParams *reflect, char cameraType)
{
    FxEffectContainer *effect; // [esp+18h] [ebp-8h]
    volatile int activeIndex; // [esp+1Ch] [ebp-4h]

    if ( !system
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_draw.cpp", 1977, 0, "%s", "system") )
    {
        __debugbreak();
    }
    if ( !system->camera.isValid
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_draw.cpp",
                    1978,
                    0,
                    "%s",
                    "system->camera.isValid") )
    {
        __debugbreak();
    }
    FX_BeginIteratingOverEffects_Cooperative(system);
    for ( activeIndex = system->shared->firstActiveEffect; activeIndex != system->shared->firstNewEffect; ++activeIndex )
    {
        effect = FX_EffectFromHandle(system, system->shared->allEffectHandles[activeIndex & 0x3FF]);
        FX_DrawNonSpriteEffect(system, &effect->effect, 1u, system->msecDraw, reflect, cameraType);
    }
    if ( !_InterlockedDecrement(&system->shared->iteratorCount) )
        FX_RunGarbageCollectionAndPrioritySort(system);
}

void __cdecl FX_BeginIteratingOverEffects_Cooperative(FxSystem *system)
{
    volatile int iteratorCount; // [esp+0h] [ebp-10h]

    if ( system->isArchiving
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\effectscore\\fx_system.h",
                    639,
                    0,
                    "%s",
                    "!system->isArchiving") )
    {
        __debugbreak();
    }
    do
    {
        if ( (volatile int)system->shared->iteratorCount < 0 )
            iteratorCount = 0;
        else
            iteratorCount = system->shared->iteratorCount;
    }
    while ( _InterlockedCompareExchange(&system->shared->iteratorCount, iteratorCount + 1, iteratorCount) != iteratorCount );
}

void __cdecl FX_DrawNonSpriteEffect(
                FxSystem *system,
                FxEffect *effect,
                unsigned int elemClass,
                int drawTime,
                const FxReflectParams *reflect,
                char cameraType)
{
    unsigned __int16 elemHandle; // [esp+4h] [ebp-F4h]
    FxDrawState drawState; // [esp+8h] [ebp-F0h] BYREF
    const FxElemDef *elemDef; // [esp+ECh] [ebp-Ch]
    const FxElemDef *elemDefs; // [esp+F0h] [ebp-8h]
    FxElem *elem; // [esp+F4h] [ebp-4h]

    memset(&drawState, 0, sizeof(drawState));
    drawState.effect = effect;
    drawState.msecDraw = drawTime;
    elemHandle = effect->firstElemHandle[elemClass];
    if ( elemHandle != 0xFFFF )
    {
        drawState.system = system;
        drawState.region = FX_RegionForReflect(reflect);
        drawState.reflect = *reflect;
        elemDefs = drawState.effect->def->elemDefsEA;
        while ( elemHandle != 0xFFFF )
        {
            elem = (FxElem *)FX_ElemFromHandle(system, elemHandle);
            elemDef = &elemDefs[elem->defIndex];
            if ( elemDef->elemType <= 5u
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_draw.cpp",
                            1959,
                            0,
                            "%s\n\t(elemDef->elemType) = %i",
                            "(elemDef->elemType > FX_ELEM_TYPE_LAST_SPRITE)",
                            elemDef->elemType) )
            {
                __debugbreak();
            }
            if ( elemDef->elemType != 7 || FX_ElemSpawnedVisibleToCamera(elem->elemSpawnVisBits, cameraType) )
                FX_DrawElement(system, elemDef, elem, &drawState);
            elemHandle = *(_WORD *)&elem[1].defIndex;
        }
    }
}

void __cdecl FX_DrawElement_Setup_1_(
    FxSystem *system,
    FxDrawState *draw,
    int elemMsecBegin,
    int elemSequence,
    const float *elemOrigin,
    float elemWindInterp,
    float *outRealNormTime)
{
    float opacity; // [esp+4h] [ebp-8Ch]
    float axis[3][3]; // [esp+5Ch] [ebp-34h] BYREF
    int msecElapsed; // [esp+80h] [ebp-10h]
    float normTime; // [esp+84h] [ebp-Ch]
    float msecElapsedFloat; // [esp+88h] [ebp-8h]
    unsigned int scaledAlpha; // [esp+8Ch] [ebp-4h]

    draw->randomSeed = (elemMsecBegin + (unsigned int)draw->effect->randomSeed + 296 * elemSequence) % 0x1DF;
    draw->msecLifeSpan = (float)FX_GetElemLifeSpanMsec(draw->randomSeed, elemWindInterp, draw->elemDef);
    msecElapsed = draw->msecDraw - elemMsecBegin;
    msecElapsedFloat = (float)msecElapsed;
    normTime = (float)msecElapsed / draw->msecLifeSpan;
    if (outRealNormTime)
        *outRealNormTime = normTime;
    if (msecElapsed < (int)draw->msecLifeSpan)
    {
        if (msecElapsed > (int)draw->msecLifeSpan
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_draw.cpp",
                1438,
                0,
                "msecElapsed <= static_cast< int >( draw->msecLifeSpan )\n\t%i, %i",
                msecElapsed,
                (int)draw->msecLifeSpan))
        {
            __debugbreak();
        }
        draw->msecElapsed = msecElapsedFloat;
        draw->normTimeUpdateEnd = normTime;
    }
    else
    {
        draw->msecElapsed = draw->msecLifeSpan - 0.0099999998;
        draw->normTimeUpdateEnd = 1.0f;
    }
    FX_GetOrientation(
        draw->elemDef,
        &draw->effect->frameAtSpawn,
        &draw->effect->frameNow,
        draw->randomSeed,
        &draw->orient);
    FX_OrientationPosToWorldPos(&draw->orient, elemOrigin, draw->posWorld);
    if ((0x800000 & draw->elemDef->flags) != 0)
    {
        MatrixMultiply(draw->rotationAxis, draw->orient.axis, axis);
        *(_QWORD *)&draw->orient.axis[0][0] = *(_QWORD *)&axis[0][0];
        draw->orient.axis[0][2] = axis[0][2];
        draw->orient.axis[1][0] = axis[1][0];
        draw->orient.axis[1][1] = axis[1][1];
        draw->orient.axis[1][2] = axis[1][2];
        draw->orient.axis[2][0] = axis[2][0];
        draw->orient.axis[2][1] = axis[2][1];
        draw->orient.axis[2][2] = axis[2][2];
    }
    FX_SetupVisualState(draw->elemDef, draw->effect, draw->randomSeed, draw->normTimeUpdateEnd, &draw->preVisState);
    FX_EvaluateSize(&draw->preVisState, &draw->visState);
    if ((draw->elemDef->flags & 0x1000) != 0)
        FX_EvaluateVisAlpha(&draw->preVisState, &draw->visState);
    draw->camera = &draw->system->camera;
    FX_EvaluateDistanceFade(draw);
    if ((draw->elemDef->flags & 0x1000) != 0)
    {
        scaledAlpha = (draw->preVisState.distanceFade * draw->visState.color[3]) >> 8;
        opacity = (double)scaledAlpha * 0.0039215689;
        FX_AddVisBlocker(system, draw->posWorld, draw->visState.size[0], opacity);
    }
}

void __cdecl FX_DrawElement(FxSystem *system, const FxElemDef *elemDef, FxElem *elem, FxDrawState *draw)
{
    float v4; // [esp+10h] [ebp-B8h]
    unsigned int rotationAxis; // [esp+84h] [ebp-44h]
    float v[4]; // [esp+88h] [ebp-40h] BYREF
    //float v7; // [esp+8Ch] [ebp-3Ch]
    //float v8; // [esp+90h] [ebp-38h]
    //float v9; // [esp+94h] [ebp-34h]
    float v10; // [esp+98h] [ebp-30h]
    float4 fAxis; // [esp+9Ch] [ebp-2Ch]
    float4 qAxis; // [esp+B0h] [ebp-18h] BYREF
    const FxEffect *effect; // [esp+C4h] [ebp-4h]

    if ( elemDef->elemType >= 0xAu
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_draw.cpp",
                    1503,
                    0,
                    "elemDef->elemType doesn't index FX_ELEM_TYPE_LAST_DRAWN + 1\n\t%i not in [0, %i)",
                    elemDef->elemType,
                    10) )
    {
        __debugbreak();
    }
    if ( elemDef->visualCount
        && elem->msecBegin <= draw->msecDraw
        && (!draw->reflect.enabled || (elemDef->flags & 0x40000000) != 0) )
    {
        effect = draw->effect;
        draw->elem = elem;
        if ( elemDef != draw->elemDef )
        {
            if ( (0x800000 & elemDef->flags) != 0 )
            {
                rotationAxis = elemDef->rotationAxis;
                v[0] = (float)(COERCE_FLOAT((rotationAxis & 0x1FF) + 1077936128 - 2 * (rotationAxis & 0x100)) - 3.0) * 16448.252;
                v[1] = (float)(COERCE_FLOAT(((rotationAxis >> 9) & 0x3FF) + 1077936128 - 2 * ((rotationAxis >> 9) & 0x200)) - 3.0) * 8208.0312;
                v[2] = (float)(COERCE_FLOAT(((rotationAxis >> 19) & 0x3FF) + 1077936128 - 2 * ((rotationAxis >> 19) & 0x200)) - 3.0) * 8208.0312;
                v[3] = 1.0f;
                v10 = Vec4Length(v);
                if ( (rotationAxis & 0x80000000) == 0 )
                    v4 = 1.0f;
                else
                    v4 = -1.0f;
                v[0] = (float)(v4 / v10) * v[0];
                v[1] = (float)(v4 / v10) * v[1];
                v[2] = (float)(v4 / v10) * v[2];
                v[3] = (float)(v4 / v10) * v[3];
                fAxis.v[0] = *(&v[0] + ((rotationAxis >> 29) & 3));
                fAxis.v[1] = *(&v[0] + ((((rotationAxis >> 29) & 3) + 1) & 3));
                fAxis.v[2] = *(&v[0] + ((((rotationAxis >> 29) & 3) + 2) & 3));
                fAxis.v[3] = *(&v[0] + ((((rotationAxis >> 29) & 3) + 3) & 3));
                qAxis = fAxis;
                UnitQuatToAxis(qAxis.v, draw->rotationAxis);
            }
            draw->elemDef = elemDef;
        }
        FX_DrawElement_Setup_1_(
            system,
            draw,
            elem->msecBegin,
            elem->sequence,
            elem->origin,
            (float)elem->windInterp / 65535.0,
            0);
        if ( (double)draw->preVisState.distanceFade >= 0.00390625 )
        {
            if ( !s_drawElemHandler[elemDef->elemType]
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_draw.cpp",
                            1548,
                            0,
                            "%s",
                            "s_drawElemHandler[elemDef->elemType]") )
            {
                __debugbreak();
            }
            s_drawElemHandler[elemDef->elemType](draw);
        }
    }
}

double __cdecl Vec4Length(const float *v)
{
    return (float)sqrt((float)((float)((float)((float)(*v * *v) + (float)(v[1] * v[1])) + (float)(v[2] * v[2]))
                                                     + (float)(v[3] * v[3])));
}

bool __cdecl FX_ElemSpawnedVisibleToCamera(unsigned int elemSpawnVisBits, char cameraType)
{
    return (elemSpawnVisBits & (1 << cameraType)) != 0;
}

void __cdecl FX_DrawSpotLight(FxSystem *system)
{
    FxEffectContainer *v1; // eax
    volatile int msecDraw; // [esp-4h] [ebp-14h]

    if ( !system
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_draw.cpp", 2000, 0, "%s", "system") )
    {
        __debugbreak();
    }
    if ( !system->camera.isValid
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_draw.cpp",
                    2001,
                    0,
                    "%s",
                    "system->camera.isValid") )
    {
        __debugbreak();
    }
    FX_BeginIteratingOverEffects_Cooperative(system);
    if ( system->shared->activeSpotLightElemCount > 0 )
    {
        if ( system->shared->activeSpotLightEffectCount != 1
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_draw.cpp",
                        2006,
                        0,
                        "%s",
                        "system->shared->activeSpotLightEffectCount == 1") )
        {
            __debugbreak();
        }
        if ( system->shared->activeSpotLightElemCount != 1
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_draw.cpp",
                        2007,
                        0,
                        "%s",
                        "system->shared->activeSpotLightElemCount == 1") )
        {
            __debugbreak();
        }
        msecDraw = system->msecDraw;
        v1 = FX_EffectFromHandle(system, system->shared->activeSpotLightEffectHandle);
        FX_DrawSpotLightEffect(system, &v1->effect, msecDraw);
    }
    if ( !_InterlockedDecrement(&system->shared->iteratorCount) )
        FX_RunGarbageCollectionAndPrioritySort(system);
}

void __cdecl FX_DrawSpotLightEffect(FxSystem *system, FxEffect *effect, int drawTime)
{
    FxDrawState drawState; // [esp+0h] [ebp-F0h] BYREF
    const FxElemDef *elemDef; // [esp+E4h] [ebp-Ch]
    const FxElemDef *elemDefs; // [esp+E8h] [ebp-8h]
    FxElem *elem; // [esp+ECh] [ebp-4h]

    memset(&drawState, 0, sizeof(drawState));
    if ( system->shared->activeSpotLightEffectCount <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_draw.cpp",
                    1912,
                    0,
                    "%s",
                    "system->shared->activeSpotLightEffectCount > 0") )
    {
        __debugbreak();
    }
    if ( system->shared->activeSpotLightElemCount <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_draw.cpp",
                    1913,
                    0,
                    "%s",
                    "system->shared->activeSpotLightElemCount > 0") )
    {
        __debugbreak();
    }
    drawState.effect = effect;
    drawState.system = system;
    drawState.msecDraw = drawTime;
    drawState.region = 13;
    FX_GetNullReflection(&drawState.reflect);
    elemDefs = drawState.effect->def->elemDefsEA;
    elem = (FxElem *)FX_ElemFromHandle(system, system->shared->activeSpotLightElemHandle);
    elemDef = &elemDefs[elem->defIndex];
    if ( elemDef->elemType != 9
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_draw.cpp",
                    1923,
                    0,
                    "%s",
                    "elemDef->elemType == FX_ELEM_TYPE_SPOT_LIGHT") )
    {
        __debugbreak();
    }
    FX_DrawElement(system, elemDef, elem, &drawState);
}

void __cdecl FX_DrawSpriteElems(FxSystem *system, int drawTime, const FxReflectParams *reflect, char cameraType)
{
    int region; // [esp+28h] [ebp-828h]
    int numTrailEffects; // [esp+2Ch] [ebp-824h]
    unsigned __int16 effectHandle; // [esp+30h] [ebp-820h]
    FxEffectContainer *effect; // [esp+38h] [ebp-818h]
    FxEffectContainer *effecta; // [esp+38h] [ebp-818h]
    FxSpriteInfo *sprite; // [esp+3Ch] [ebp-814h]
    unsigned __int16 trailEffects[1026]; // [esp+40h] [ebp-810h]
    int i; // [esp+848h] [ebp-8h]
    int activeIndex; // [esp+84Ch] [ebp-4h]

    if ( !system
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_draw.cpp", 2151, 0, "%s", "system") )
    {
        __debugbreak();
    }
    if ( !system->camera.isValid
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_draw.cpp",
                    2152,
                    0,
                    "%s",
                    "system->camera.isValid") )
    {
        __debugbreak();
    }
    system->gfxCloudCount = 0;
    sprite = &system->sprite;
    system->sprite.indices = 0;
    system->sprite.indexCount = 0;
    system->sprite.name = 0;
    system->sprite.material = 0;
    numTrailEffects = 0;
    FX_BeginIteratingOverEffects_Cooperative(system);

    {
        PROF_SCOPED("firstActiveEffect");

        for (activeIndex = system->shared->firstActiveEffect; activeIndex != system->shared->firstNewEffect; ++activeIndex)
        {
            effectHandle = system->shared->allEffectHandles[activeIndex & 0x3FF];
            effect = FX_EffectFromHandle(system, effectHandle);
            FX_DrawSpriteEffect(system, &effect->effect, drawTime, reflect);
            FX_DrawNonSpriteEffect(system, &effect->effect, 2u, drawTime, reflect, cameraType);
            if (effect->effect.firstTrailHandle != 0xFFFF)
                trailEffects[numTrailEffects++] = effectHandle;
        }
    }
    
    {
        PROF_SCOPED("numTrailEffects");
        if (numTrailEffects > 0)
        {
            for (i = 0; i < numTrailEffects; ++i)
            {
                effecta = FX_EffectFromHandle(system, trailEffects[i]);
                FX_DrawTrailsForEffect(system, &effecta->effect, drawTime, reflect);
            }
        }
    }

    if ( !_InterlockedDecrement(&system->shared->iteratorCount) )
        FX_RunGarbageCollectionAndPrioritySort(system);

    {
        PROF_SCOPED("R_AddCodeMeshDrawSurf");
        if (system->sprite.indexCount)
        {
            if (!system->sprite.name
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_draw.cpp", 2216, 0, "%s", "sprite->name"))
            {
                __debugbreak();
            }
            if (!system->sprite.material
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_draw.cpp",
                    2217,
                    0,
                    "%s",
                    "sprite->material"))
            {
                __debugbreak();
            }
            if (!sprite->indices
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_draw.cpp",
                    2218,
                    0,
                    "%s",
                    "sprite->indices"))
            {
                __debugbreak();
            }
            region = FX_RegionForReflect(reflect);
            R_AddCodeMeshDrawSurf(
                system->sprite.material,
                system->sprite.indices,
                system->sprite.indexCount,
                0,
                0,
                system->sprite.name,
                region);
            system->sprite.indexCount = 0;
            sprite->indices = 0;
        }
    }
}

void __cdecl FX_DrawTrailsForEffect(FxSystem *system, FxEffect *effect, int drawTime, const FxReflectParams *reflect)
{
    FxDrawState drawState; // [esp+0h] [ebp-F0h] BYREF
    unsigned __int16 trailHandle; // [esp+E8h] [ebp-8h]
    FxTrail *trail; // [esp+ECh] [ebp-4h]

    memset(&drawState, 0, sizeof(drawState));
    drawState.system = system;
    drawState.effect = effect;
    drawState.msecDraw = drawTime;
    drawState.region = FX_RegionForReflect(reflect);
    drawState.reflect = *reflect;
    drawState.elem = 0;
    for ( trailHandle = drawState.effect->firstTrailHandle; trailHandle != 0xFFFF; trailHandle = trail->nextTrailHandle )
    {
        trail = (FxTrail *)FX_TrailFromHandle(system, trailHandle);
        FX_DrawTrail(system, &drawState, trail);
    }
}

FxPool<FxTrail,FxTrail> *__cdecl FX_TrailFromHandle(FxSystem *system, unsigned __int16 handle)
{
    if ( !system
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\effectscore\\fx_system.h", 507, 0, "%s", "system") )
    {
        __debugbreak();
    }
    return FX_PoolFromHandle_Generic<FxTrail,FxTrail,128>(system->trails, handle);
}

void __cdecl FX_DrawTrail(FxSystem *system, FxDrawState *draw, FxTrail *trail)
{
    const char *v3; // eax
    float v4; // [esp+18h] [ebp-19Ch]
    FxTrailSegmentDrawState tailSegmentDrawState; // [esp+98h] [ebp-11Ch] BYREF
    float alpha; // [esp+D4h] [ebp-E0h]
    r_double_index_t *reservedIndices; // [esp+D8h] [ebp-DCh] BYREF
    int trailDefIndCount; // [esp+DCh] [ebp-D8h]
    FxTrailSegmentDrawState lastSegmentDrawState; // [esp+E0h] [ebp-D4h] BYREF
    FxElemVisuals visuals; // [esp+11Ch] [ebp-98h]
    unsigned __int16 vertsPerSegment; // [esp+120h] [ebp-94h]
    int upperBoundSegmentCount; // [esp+124h] [ebp-90h]
    const FxTrailDef *trailDef; // [esp+128h] [ebp-8Ch]
    FxTrailSegmentDrawState segmentDrawState; // [esp+12Ch] [ebp-88h] BYREF
    int exactSegmentCount; // [esp+16Ch] [ebp-48h]
    FxSpriteInfo *sprite; // [esp+170h] [ebp-44h]
    unsigned __int16 reservedBaseVertex; // [esp+174h] [ebp-40h] BYREF
    GfxPackedVertex *reservedVerts; // [esp+178h] [ebp-3Ch]
    int indicesToReserve; // [esp+17Ch] [ebp-38h]
    const FxTrailElem *trailElem; // [esp+180h] [ebp-34h]
    float uCoordOffset; // [esp+184h] [ebp-30h]
    float basis[2][3]; // [esp+188h] [ebp-2Ch] BYREF
    float lastSegmentNormTime; // [esp+1A0h] [ebp-14h]
    int trailDefVertCount; // [esp+1A4h] [ebp-10h]
    int curSegment; // [esp+1A8h] [ebp-Ch]
    unsigned __int16 trailElemHandle; // [esp+1ACh] [ebp-8h]
    float segmentNormTime; // [esp+1B0h] [ebp-4h] BYREF

    sprite = &system->sprite;
    draw->elemDef = &draw->effect->def->elemDefsEA[trail->defIndex];
    if ( draw->elemDef->visualCount )
    {
        trailElemHandle = trail->firstElemHandle;
        if ( trailElemHandle != 0xFFFF )
        {
            trailElem = (const FxTrailElem *)FX_TrailElemFromHandle(system, trailElemHandle);
            trailDef = draw->elemDef->localTrailDef;
            v4 = floor((float)(trailElem->spawnDist / (float)trailDef->repeatDist));
            uCoordOffset = -v4;
            if ( trailDef->scrollTimeMsec )
            {
                if ( trailDef->scrollTimeMsec <= 0 )
                    uCoordOffset = (float)(1.0
                                                             - (float)((float)(draw->msecDraw % trailDef->scrollTimeMsec)
                                                                             / (float)trailDef->scrollTimeMsec))
                                             + uCoordOffset;
                else
                    uCoordOffset = (float)((float)(draw->msecDraw % -trailDef->scrollTimeMsec) / (float)-trailDef->scrollTimeMsec)
                                             + uCoordOffset;
            }
            upperBoundSegmentCount = 0;
            for ( trailElemHandle = trail->firstElemHandle;
                        trailElemHandle != 0xFFFF;
                        trailElemHandle = trailElem->nextTrailElemHandle )
            {
                trailElem = (const FxTrailElem *)FX_TrailElemFromHandle(system, trailElemHandle);
                if ( trailElem->msecBegin <= draw->msecDraw )
                    ++upperBoundSegmentCount;
            }
            trailDefVertCount = trailDef->vertCount;
            trailDefIndCount = trailDef->indCount;
            if ( R_ReserveCodeMeshVerts(trailDefVertCount * upperBoundSegmentCount, &reservedBaseVertex) )
            {
                reservedVerts = R_GetCodeMeshVerts(reservedBaseVertex);
                exactSegmentCount = 0;
                lastSegmentNormTime = 1.0f;
                memset(&lastSegmentDrawState, 0, sizeof(lastSegmentDrawState));
                for ( trailElemHandle = trail->firstElemHandle;
                            trailElemHandle != 0xFFFF;
                            trailElemHandle = trailElem->nextTrailElemHandle )
                {
                    trailElem = (const FxTrailElem *)FX_TrailElemFromHandle(system, trailElemHandle);
                    if ( trailElem->msecBegin <= draw->msecDraw )
                    {
                        FX_DrawElement_Setup_1_(
                            system,
                            draw,
                            trailElem->msecBegin,
                            trailElem->sequence,
                            trailElem->origin,
                            0.0,
                            &segmentNormTime);
                        FX_TrailElem_UncompressBasis(trailElem->basis, basis);
                        FX_EvaluateVisualState(
                            draw->system->localClientNum,
                            &draw->preVisState,
                            draw->msecLifeSpan,
                            &draw->visState);
                        Fx_GenTrail_PopulateSegmentDrawState(draw, trailElem->spawnDist, uCoordOffset, basis, &segmentDrawState);
                        if ( segmentNormTime < 1.0 )
                        {
                            if ( trailElemHandle == trail->firstElemHandle )
                            {
                                if ( !trailElem->sequence )
                                    segmentDrawState.color[3] = 0;
                            }
                            else
                            {
                                if ( lastSegmentNormTime >= 1.0 )
                                {
                                    memcpy(&tailSegmentDrawState, &lastSegmentDrawState, sizeof(tailSegmentDrawState));
                                    alpha = (float)(1.0 - lastSegmentNormTime) / (float)(segmentNormTime - lastSegmentNormTime);
                                    tailSegmentDrawState.uCoord = (float)((float)(segmentDrawState.uCoord - lastSegmentDrawState.uCoord)
                                                                                                            * alpha)
                                                                                            + lastSegmentDrawState.uCoord;
                                    Vec3Lerp(
                                        lastSegmentDrawState.posWorld,
                                        segmentDrawState.posWorld,
                                        alpha,
                                        tailSegmentDrawState.posWorld);
                                    Vec3Lerp(
                                        lastSegmentDrawState.basis[0],
                                        segmentDrawState.basis[0],
                                        alpha,
                                        tailSegmentDrawState.basis[0]);
                                    Vec3Lerp(
                                        lastSegmentDrawState.basis[1],
                                        segmentDrawState.basis[1],
                                        alpha,
                                        tailSegmentDrawState.basis[1]);
                                    FX_GenTrail_VertsForSegment(
                                        &tailSegmentDrawState,
                                        &reservedVerts[trailDefVertCount * exactSegmentCount++]);
                                }
                                if ( trailElemHandle == trail->lastElemHandle )
                                    segmentDrawState.color[3] = 0;
                            }
                            FX_GenTrail_VertsForSegment(&segmentDrawState, &reservedVerts[trailDefVertCount * exactSegmentCount++]);
                        }
                        lastSegmentNormTime = segmentNormTime;
                        memcpy(&lastSegmentDrawState, &segmentDrawState, sizeof(lastSegmentDrawState));
                    }
                }
                if ( exactSegmentCount > upperBoundSegmentCount )
                {
                    v3 = va(
                                 "Too optimistic: exactSegmentCount = %i, upperBoundSegmentCount = %i",
                                 exactSegmentCount,
                                 upperBoundSegmentCount);
                    if ( !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_draw.cpp",
                                    1693,
                                    0,
                                    "%s\n\t%s",
                                    "exactSegmentCount <= upperBoundSegmentCount",
                                    v3) )
                        __debugbreak();
                }
                if ( exactSegmentCount > 1 )
                {
                    indicesToReserve = 3 * trailDefIndCount;
                    vertsPerSegment = trailDef->vertCount;
                    visuals.anonymous = FX_GetElemVisuals(draw->elemDef, draw->randomSeed).anonymous;
                    if ( sprite->material != visuals.anonymous && sprite->indexCount )
                    {
                        if ( !sprite->name
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_draw.cpp",
                                        1705,
                                        0,
                                        "%s",
                                        "sprite->name") )
                        {
                            __debugbreak();
                        }
                        if ( !sprite->material
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_draw.cpp",
                                        1706,
                                        0,
                                        "%s",
                                        "sprite->material") )
                        {
                            __debugbreak();
                        }
                        if ( !sprite->indices
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_draw.cpp",
                                        1707,
                                        0,
                                        "%s",
                                        "sprite->indices") )
                        {
                            __debugbreak();
                        }
                        R_AddCodeMeshDrawSurf(
                            sprite->material,
                            sprite->indices,
                            sprite->indexCount,
                            0,
                            0,
                            sprite->name,
                            draw->region);
                        sprite->indexCount = 0;
                    }
                    curSegment = 0;
                    while ( curSegment < exactSegmentCount - 1 && R_ReserveCodeMeshIndices(indicesToReserve, &reservedIndices) )
                    {
                        if ( sprite->material != visuals.anonymous )
                        {
                            sprite->name = draw->effect->def->name;
                            sprite->material = visuals.material;
                            sprite->indices = reservedIndices;
                        }
                        sprite->indexCount += indicesToReserve;
                        FX_GenTrail_IndsForSegment(draw, reservedBaseVertex, reservedIndices);
                        ++curSegment;
                        reservedBaseVertex += vertsPerSegment;
                    }
                }
            }
        }
    }
}

void __cdecl FX_TrailElem_UncompressBasis(const char (*inBasis)[3], float (*basis)[3])
{
    int basisVecIter; // [esp+0h] [ebp-8h]
    int dimIter; // [esp+4h] [ebp-4h]

    for ( basisVecIter = 0; basisVecIter != 2; ++basisVecIter )
    {
        for ( dimIter = 0; dimIter != 3; ++dimIter )
            (*basis)[3 * basisVecIter + dimIter] = (float)(*inBasis)[3 * basisVecIter + dimIter] * 0.0078740157;
    }
}

FxPool<FxTrailElem,FxTrailElem> *__cdecl FX_TrailElemFromHandle(FxSystem *system, unsigned __int16 handle)
{
    if ( !system
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\effectscore\\fx_system.h", 493, 0, "%s", "system") )
    {
        __debugbreak();
    }
    return FX_PoolFromHandle_Generic<FxTrailElem,FxTrailElem,2048>(system->trailElems, handle);
}

void __cdecl FX_GenTrail_IndsForSegment(
                FxDrawState *draw,
                unsigned __int16 reservedBaseVertex,
                r_double_index_t *outIndices)
{
    unsigned __int16 quadInds_2; // [esp+2h] [ebp-2Eh]
    r_double_index_t quadInds_4; // [esp+4h] [ebp-2Ch]
    r_double_index_t index; // [esp+Ch] [ebp-24h]
    int indCount; // [esp+10h] [ebp-20h]
    FxTrailDef *trailDef; // [esp+18h] [ebp-18h]
    unsigned __int16 farBase; // [esp+20h] [ebp-10h]
    unsigned __int16 *inds; // [esp+24h] [ebp-Ch]
    int indPairIter; // [esp+28h] [ebp-8h]
    r_double_index_t *outIndicesa; // [esp+40h] [ebp+10h]

    trailDef = draw->elemDef->localTrailDef;
    if ( !trailDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_draw.cpp", 643, 0, "%s", "trailDef") )
    {
        __debugbreak();
    }
    inds = trailDef->indsEA;
    indCount = trailDef->indCount;
    if ( 2 * (indCount / 2) != indCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_draw.cpp",
                    648,
                    0,
                    "%s\n\t(trailDef->indCount) = %i",
                    "(indPairCount * 2 == indCount)",
                    trailDef->indCount) )
    {
        __debugbreak();
    }
    farBase = LOWORD(trailDef->vertCount) + reservedBaseVertex;
    for ( indPairIter = 0; indPairIter != indCount / 2; ++indPairIter )
    {
        quadInds_2 = inds[2 * indPairIter + 1] + reservedBaseVertex;
        quadInds_4.value[0] = inds[2 * indPairIter] + farBase;
        quadInds_4.value[1] = inds[2 * indPairIter + 1] + farBase;
        index.value[0] = inds[2 * indPairIter] + reservedBaseVertex;
        index.value[1] = quadInds_2;
        *outIndices = index;
        outIndicesa = outIndices + 1;
        *outIndicesa = quadInds_4;
        index.value[0] = quadInds_4.value[0];
        index.value[1] = quadInds_2;
        outIndicesa[1] = index;
        outIndices = outIndicesa + 2;
    }
}

void __cdecl Fx_GenTrail_PopulateSegmentDrawState(
                FxDrawState *draw,
                float spawnDist,
                float uCoordOffset,
                const float (*basis)[3],
                FxTrailSegmentDrawState *outState)
{
    FxTrailDef *trailDef; // [esp+18h] [ebp-4h]

    trailDef = draw->elemDef->localTrailDef;
    outState->trailDef = trailDef;
    outState->posWorld[0] = draw->posWorld[0];
    outState->posWorld[1] = draw->posWorld[1];
    outState->posWorld[2] = draw->posWorld[2];
    outState->basis[0][0] = (*basis)[0];
    outState->basis[0][1] = (*basis)[1];
    outState->basis[0][2] = (*basis)[2];
    outState->basis[1][0] = (*basis)[3];
    outState->basis[1][1] = (*basis)[4];
    outState->basis[1][2] = (*basis)[5];
    outState->rotation = draw->visState.rotationTotal;
    *(double *)outState->size = *(double *)draw->visState.size;
    *(unsigned int *)outState->color = *(unsigned int *)draw->visState.color;
    outState->uCoord = (float)(spawnDist / (float)trailDef->repeatDist) + uCoordOffset;
}

void __cdecl FX_GenTrail_VertsForSegment(const FxTrailSegmentDrawState *segmentDrawState, GfxPackedVertex *remoteVerts)
{
    __int16 v2; // [esp+0h] [ebp-100h]
    __int16 v3; // [esp+4h] [ebp-FCh]
    PackedUnitVec v4; // [esp+8h] [ebp-F8h]
    float v5; // [esp+10h] [ebp-F0h]
    float v6; // [esp+14h] [ebp-ECh]
    int v7; // [esp+1Ch] [ebp-E4h]
    int texCoord_low; // [esp+28h] [ebp-D8h]
    int v9; // [esp+30h] [ebp-D0h]
    float v10; // [esp+50h] [ebp-B0h]
    float v11; // [esp+54h] [ebp-ACh]
    float rotation; // [esp+80h] [ebp-80h]
    FxTrailDef *trailDef; // [esp+B8h] [ebp-48h]
    float cosRot; // [esp+C0h] [ebp-40h]
    float left; // [esp+C4h] [ebp-3Ch]
    float left_4; // [esp+C8h] [ebp-38h]
    float left_8; // [esp+CCh] [ebp-34h]
    float sinRot; // [esp+D0h] [ebp-30h]
    float uCoord; // [esp+E8h] [ebp-18h]
    float up; // [esp+ECh] [ebp-14h]
    float up_4; // [esp+F0h] [ebp-10h]
    float up_8; // [esp+F4h] [ebp-Ch]
    int vertCount; // [esp+F8h] [ebp-8h]
    int vertIter; // [esp+FCh] [ebp-4h]

    trailDef = segmentDrawState->trailDef;
    if ( !segmentDrawState->trailDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_draw.cpp", 730, 0, "%s", "trailDef") )
    {
        __debugbreak();
    }
    if ( trailDef->vertCount <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_draw.cpp",
                    731,
                    0,
                    "%s",
                    "trailDef->vertCount > 0") )
    {
        __debugbreak();
    }
    if ( trailDef->indCount <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_draw.cpp",
                    732,
                    0,
                    "%s",
                    "trailDef->indCount > 0") )
    {
        __debugbreak();
    }
    rotation = segmentDrawState->rotation;
    cosRot = cos(rotation);
    sinRot = sin(rotation);
    left = (float)(cosRot * segmentDrawState->basis[0][0]) + (float)(sinRot * segmentDrawState->basis[1][0]);
    left_4 = (float)(cosRot * segmentDrawState->basis[0][1]) + (float)(sinRot * segmentDrawState->basis[1][1]);
    left_8 = (float)(cosRot * segmentDrawState->basis[0][2]) + (float)(sinRot * segmentDrawState->basis[1][2]);
    up = (float)(sinRot * segmentDrawState->basis[0][0]) +   (float)((-(cosRot)) * segmentDrawState->basis[1][0]);
    up_4 = (float)(sinRot * segmentDrawState->basis[0][1]) + (float)((-(cosRot)) * segmentDrawState->basis[1][1]);
    up_8 = (float)(sinRot * segmentDrawState->basis[0][2]) + (float)((-(cosRot)) * segmentDrawState->basis[1][2]);
    uCoord = segmentDrawState->uCoord;
    vertCount = trailDef->vertCount;
    for ( vertIter = 0; vertIter != vertCount; ++vertIter )
    {
        v10 = trailDef->vertsEA[vertIter].pos[0] * segmentDrawState->size[0];
        v11 = trailDef->vertsEA[vertIter].pos[1] * segmentDrawState->size[1];
        remoteVerts->xyz[0] = (float)((float)(v10 * left) + segmentDrawState->posWorld[0]) + (float)(v11 * up);
        remoteVerts->xyz[1] = (float)((float)(v10 * left_4) + segmentDrawState->posWorld[1]) + (float)(v11 * up_4);
        remoteVerts->xyz[2] = (float)((float)(v10 * left_8) + segmentDrawState->posWorld[2]) + (float)(v11 * up_8);
        remoteVerts->color.packed = *(unsigned int *)segmentDrawState->color;
        if ( (int)((2 * LODWORD(uCoord)) ^ 0x80000000) >> 14 < 0x3FFF )
            v9 = (int)((2 * LODWORD(uCoord)) ^ 0x80000000) >> 14;
        else
            v9 = 0x3FFF;
        if ( v9 > -16384 )
            v3 = v9;
        else
            v3 = -16384;
        texCoord_low = LODWORD(trailDef->vertsEA[vertIter].texCoord);
        if ( (int)((2 * texCoord_low) ^ 0x80000000) >> 14 < 0x3FFF )
            v7 = (int)((2 * texCoord_low) ^ 0x80000000) >> 14;
        else
            v7 = 0x3FFF;
        if ( v7 > -16384 )
            v2 = v7;
        else
            v2 = -16384;
        remoteVerts->texCoord.packed = (v2 & 0x3FFF | (texCoord_low >> 16) & 0xC000)
                                                                 + ((v3 & 0x3FFF | (SLODWORD(uCoord) >> 16) & 0xC000) << 16);
        v6 = trailDef->vertsEA[vertIter].normal[0];
        v5 = trailDef->vertsEA[vertIter].normal[1];
        v4.array[0] = (int)(float)((float)((float)((float)(v5 * up) + (float)(v6 * left)) * 127.0) + 127.5);
        v4.array[1] = (int)(float)((float)((float)((float)(v5 * up_4) + (float)(v6 * left_4)) * 127.0) + 127.5);
        v4.array[2] = (int)(float)((float)((float)((float)(v5 * up_8) + (float)(v6 * left_8)) * 127.0) + 127.5);
        v4.array[3] = 63;
        remoteVerts->normal = v4;
        remoteVerts->tangent.packed = 1065320446;
        ++remoteVerts;
    }
}

void __cdecl FX_DrawSpriteEffect(FxSystem *system, FxEffect *effect, int drawTime, const FxReflectParams *reflect)
{
    unsigned __int16 elemHandle; // [esp+8h] [ebp-F8h]
    const FxElemDef *elemDef; // [esp+Ch] [ebp-F4h]
    FxDrawState drawState; // [esp+10h] [ebp-F0h] BYREF
    const FxElemDef *elemDefs; // [esp+F4h] [ebp-Ch]
    FxElem *elem; // [esp+F8h] [ebp-8h]
    int count; // [esp+FCh] [ebp-4h]

    count = 0;
    memset(&drawState, 0, sizeof(drawState));
    drawState.effect = effect;
    drawState.msecDraw = drawTime;
    elemHandle = effect->firstElemHandle[0];
    if ( elemHandle != 0xFFFF )
    {
        drawState.system = system;
        drawState.region = FX_RegionForReflect(reflect);
        drawState.reflect = *reflect;
        elemDefs = drawState.effect->def->elemDefsEA;
        while ( elemHandle != 0xFFFF )
        {
            elem = (FxElem *)FX_ElemFromHandle(system, elemHandle);
            if ( count > 2048 )
            {
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_draw.cpp",
                                1854,
                                0,
                                "Loop in element list") )
                    __debugbreak();
                return;
            }
            if ( elem->defIndex >= drawState.effect->def->elemDefCountOneShot
                                                     + drawState.effect->def->elemDefCountLooping
                                                     + drawState.effect->def->elemDefCountEmission )
                return;
            if ( elemHandle == *(unsigned __int16 *)&elem[1].defIndex )
                return;
            elemDef = &elemDefs[elem->defIndex];
            if ( elemDef->elemType > 5u )
                return;
            ++count;
            if ( elemDef->visualCount && (elemDef->flags >= 0 || cg_blood->current.enabled && CG_IsMature()) )
                FX_DrawElement(system, elemDef, elem, &drawState);
            elemHandle = *(_WORD *)&elem[1].defIndex;
        }
    }
}

void __cdecl FX_GenerateVerts(FxGenerateVertsCmd *cmd)
{
    const char *v1; // eax
    volatile int drawTime; // [esp+50h] [ebp-14h]
    FxReflectParams reflect; // [esp+54h] [ebp-10h] BYREF
    FxSystem *localSystem; // [esp+60h] [ebp-4h]

    //v1 = va("FX_GenerateVerts(cl=%d)", cmd->localClientNum);
    PROF_SCOPED("FX_GenerateVerts");
    ZoneTextF("(cl=%d)", cmd->localClientNum);

    localSystem = cmd->system;
    R_BeginCodeMeshVerts();
    drawTime = localSystem->msecDraw;
    if ( drawTime >= 0 )
    {
        {
            PROF_SCOPED("FX_SpriteGenerateVerts");
            FX_SpriteGenerateVerts(cmd);
        }
        
        {
            PROF_SCOPED("FX_Beam_GenerateVerts");
            FX_Beam_GenerateVerts(cmd);
        }
        {
            PROF_SCOPED("FX_PostLight_GenerateVerts");
            FX_PostLight_GenerateVerts(cmd->postLightInfo, localSystem);
        }
        {
            PROF_SCOPED("Glass_GenerateVerts");
            GlassCl_GenerateVerts(cmd->localClientNum, 0);
        }
        
        FX_GetNullReflection(&reflect);

        {
            PROF_SCOPED("FX_DrawSpriteElems");
            if (fx_enable->current.enabled && fx_draw->current.enabled)
                FX_DrawSpriteElems(localSystem, drawTime, &reflect, cmd->genVertsCameraType);
        }
        
        {
            PROF_SCOPED("FX_GetReflection");
            FX_GetReflection(cmd->useReflection, cmd->reflectionHeight, &reflect);
            if (reflect.enabled && fx_enable->current.enabled && fx_draw->current.enabled)
                FX_DrawSpriteElems(localSystem, drawTime, &reflect, cmd->genVertsCameraType);
        }
        
        R_EndCodeMeshVerts();

        {
            PROF_SCOPED("FX_ToggleVisBlockerFrame");
            FX_ToggleVisBlockerFrame(localSystem);
        }
    }
    else
    {
        R_EndCodeMeshVerts();
    }
}

void __cdecl FX_FillGenerateVertsCmd(int localClientNum, FxGenerateVertsCmd *cmd)
{
    int v2; // eax
    cg_s *LocalClientGlobals; // eax
    int v4; // eax
    cg_s *cgameGlob; // eax

    if ( !cmd && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_draw.cpp", 2362, 0, "%s", "cmd") )
        __debugbreak();
    cmd->system = &FX_GetSystem(localClientNum)->system;
    cmd->beamInfo = FX_Beam_GetInfo();
    cmd->postLightInfo = FX_PostLight_GetInfo();
    cmd->spriteInfo = FX_SpriteGetInfo();
    cmd->localClientNum = localClientNum;
    v2 = R_GetLocalClientNum();
    LocalClientGlobals = CG_GetLocalClientGlobals(v2);
    cmd->vieworg[0] = LocalClientGlobals->refdef.vieworg[0];
    cmd->vieworg[1] = LocalClientGlobals->refdef.vieworg[1];
    cmd->vieworg[2] = LocalClientGlobals->refdef.vieworg[2];
    v4 = R_GetLocalClientNum();
    cgameGlob = CG_GetLocalClientGlobals(v4);
    AxisCopy(cgameGlob->refdef.viewaxis, cmd->viewaxis);
}

//FxPool<FxTrailElem,FxTrailElem> *__cdecl FX_PoolFromHandle_Generic<FxTrailElem,FxTrailElem,2048>(
//                FxPool<FxTrailElem,FxTrailElem> *poolArray,
//                unsigned __int16 handle)
//{
//    const char *v2; // eax
//
//    if ( handle >= 0x4000u || handle % 8u )
//    {
//        v2 = va("%p %i size=%d limit=%d handlescale=%d", poolArray, handle, 32, 2048, 4);
//        if ( !Assert_MyHandler(
//                        "c:\\projects_pc\\cod\\codsrc\\src\\effectscore\\fx_system.h",
//                        465,
//                        0,
//                        "%s\n\t%s",
//                        "handle < LIMIT * sizeof( CONTAINER_TYPE ) / ITEM_TYPE::HANDLE_SCALE && handle % (sizeof( CONTAINER_TYPE ) / "
//                        "ITEM_TYPE::HANDLE_SCALE) == 0",
//                        v2) )
//            __debugbreak();
//    }
//    return (FxPool<FxTrailElem,FxTrailElem> *)((char *)poolArray + 4 * handle);
//}
//
//FxPool<FxTrail,FxTrail> *__cdecl FX_PoolFromHandle_Generic<FxTrail,FxTrail,128>(
//                FxPool<FxTrail,FxTrail> *poolArray,
//                unsigned __int16 handle)
//{
//    const char *v2; // eax
//
//    if ( handle >= 0x100u || handle % 2u )
//    {
//        v2 = va("%p %i size=%d limit=%d handlescale=%d", poolArray, handle, 8, 128, 4);
//        if ( !Assert_MyHandler(
//                        "c:\\projects_pc\\cod\\codsrc\\src\\effectscore\\fx_system.h",
//                        465,
//                        0,
//                        "%s\n\t%s",
//                        "handle < LIMIT * sizeof( CONTAINER_TYPE ) / ITEM_TYPE::HANDLE_SCALE && handle % (sizeof( CONTAINER_TYPE ) / "
//                        "ITEM_TYPE::HANDLE_SCALE) == 0",
//                        v2) )
//            __debugbreak();
//    }
//    return (FxPool<FxTrail,FxTrail> *)((char *)poolArray + 4 * handle);
//}

void __cdecl FX_EvaluateDistanceFade(FxDrawState *draw)
{
    float v1; // [esp+10h] [ebp-3Ch]
    float v2; // [esp+18h] [ebp-34h]
    float v3; // [esp+1Ch] [ebp-30h]
    float v4; // [esp+24h] [ebp-28h]
    float v5; // [esp+28h] [ebp-24h]
    const FxElemDef *def; // [esp+3Ch] [ebp-10h]
    float dist; // [esp+40h] [ebp-Ch]
    float fadeInFrac; // [esp+44h] [ebp-8h]
    float fadeOutFrac; // [esp+48h] [ebp-4h]

    def = draw->elemDef;
    if ( def->fadeInRange.amplitude != 0.0 || def->fadeOutRange.amplitude != 0.0 )
    {
        fadeInFrac = 1.0f;
        fadeOutFrac = 1.0f;
        dist = Vec3Distance(draw->camera->origin, draw->posWorld);
        if ( def->fadeInRange.amplitude != 0.0 )
        {
            v4 = dist - def->fadeInRange.base;
            v5 = 0.0f;
            if ( v4 >= 0.0 )
            {
                if ( def->fadeInRange.amplitude > v4 )
                    v5 = 1.0 - (float)(v4 / def->fadeInRange.amplitude);
            }
            else
            {
                v5 = 1.0f;
            }
            fadeInFrac = v5;
        }
        if ( def->fadeOutRange.amplitude != 0.0 )
        {
            v2 = dist - def->fadeOutRange.base;
            v3 = 0.0f;
            if ( v2 >= 0.0 )
            {
                if ( def->fadeOutRange.amplitude > v2 )
                    v3 = 1.0 - (float)(v2 / def->fadeOutRange.amplitude);
            }
            else
            {
                v3 = 1.0f;
            }
            fadeOutFrac = 1.0 - v3;
        }
        if ( (float)(fadeOutFrac - fadeInFrac) < 0.0 )
            v1 = fadeOutFrac;
        else
            v1 = fadeInFrac;
        draw->preVisState.distanceFade = (__int64)(v1 * 255.0 + 0.5);
    }
}

