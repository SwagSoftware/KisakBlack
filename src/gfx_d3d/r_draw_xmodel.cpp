#include "r_draw_xmodel.h"
#include "r_state_utils.h"
#include "r_model_lighting.h"
#include "r_state.h"
#include "r_draw_model_util.h"
#include "r_shade.h"
#include "r_dvars.h"

char __cdecl R_DrawXModelSurfCheckBreak(
                unsigned int gfxEntIndex,
                const GfxBackEndData *data,
                unsigned int depthHackFlags,
                float materialTime,
                float curDestructibleBurnAmount,
                float curDestructibleFadeAmount,
                float curWetness,
                bool TEXTURE_OVERRIDE,
                int currTextureOverride,
                unsigned int drawOutlineFlags)
{
    if ( gfxEntIndex )
    {
        if ( (data->gfxEnts[gfxEntIndex].renderFxFlags & 2) != depthHackFlags )
            return 1;
        if ( data->gfxEnts[gfxEntIndex].materialTime != materialTime )
            return 1;
        if ( data->gfxEnts[gfxEntIndex].destructibleBurnAmount != curDestructibleBurnAmount )
            return 1;
        if ( data->gfxEnts[gfxEntIndex].destructibleFadeAmount != curDestructibleFadeAmount )
            return 1;
        if ( data->gfxEnts[gfxEntIndex].wetness != curWetness )
            return 1;
        if ( TEXTURE_OVERRIDE && data->gfxEnts[gfxEntIndex].textureOverrideIndex != currTextureOverride )
            return 1;
    }
    else
    {
        if ( depthHackFlags )
            return 1;
        if ( materialTime != 0.0 )
            return 1;
        if ( curDestructibleBurnAmount != 0.0 )
            return 1;
        if ( curDestructibleFadeAmount != 0.0 )
            return 1;
        if ( curWetness != 0.0 )
            return 1;
        if ( TEXTURE_OVERRIDE && currTextureOverride >= 0 )
            return 1;
        if ( drawOutlineFlags )
            return 1;
    }
    return 0;
}

unsigned int __cdecl R_DrawXModelSurfLit(
                const GfxDrawSurf *drawSurfList,
                unsigned int drawSurfCount,
                GfxCmdBufContext context)
{
    return R_DrawXModelSurfLitInternal(drawSurfList, drawSurfCount, context);
}

unsigned int __cdecl R_DrawXModelSurfLitInternal(
                const GfxDrawSurf *drawSurfList,
                unsigned int drawSurfCount,
                GfxCmdBufContext context)
{
    GfxCmdBufSourceState *ActiveWorldMatrix; // eax
    int v5; // eax
    vector4 worldMat; // [esp+A8h] [ebp-A0h] BYREF
    unsigned int baseGfxEntIndex; // [esp+ECh] [ebp-5Ch]
    GfxDrawSurf drawSurf; // [esp+F0h] [ebp-58h]
    const GfxBackEndData *data; // [esp+FCh] [ebp-4Ch]
    GfxCmdBufSourceState *commonSource; // [esp+100h] [ebp-48h]
    float curWetness; // [esp+104h] [ebp-44h]
    unsigned int drawSurfIndex; // [esp+108h] [ebp-40h]
    float4 eyeOffset; // [esp+10Ch] [ebp-3Ch]
    GfxDrawSurf drawSurfMask; // [esp+120h] [ebp-28h]
    const GfxModelRigidSurface *modelSurf; // [esp+128h] [ebp-20h]
    float curDestructibleBurnAmount; // [esp+12Ch] [ebp-1Ch]
    unsigned int depthHackFlags; // [esp+130h] [ebp-18h]
    float curDestructibleFadeAmount; // [esp+134h] [ebp-14h]
    XSurface *xsurf; // [esp+138h] [ebp-10h]
    float materialTime; // [esp+13Ch] [ebp-Ch]
    unsigned __int64 drawSurfKey; // [esp+140h] [ebp-8h]

    commonSource = context.source;
    data = context.source->input.data;
    drawSurf.fields = drawSurfList->fields;
    drawSurfMask.packed = -234946560;
    drawSurfKey = drawSurf.packed & 0xFFFFFFFFF1FF0000uLL;
    drawSurfIndex = 0;
    *(_QWORD *)eyeOffset.v = *(_QWORD *)context.source->eyeOffset;
    eyeOffset.u[2] = LODWORD(context.source->eyeOffset[2]);
    eyeOffset.v[0] = eyeOffset.v[0] - 0.0;
    eyeOffset.v[1] = eyeOffset.v[1] - 0.0;
    eyeOffset.v[2] = eyeOffset.v[2] - 0.0;
    eyeOffset.v[3] = 0.0 - 1.0;
    modelSurf = (const GfxModelRigidSurface *)((char *)data + 4 * LOWORD(drawSurf.packed));
    baseGfxEntIndex = modelSurf->surf.info.gfxEntIndex;
    depthHackFlags = context.source->depthHackFlags;
    materialTime = context.source->materialTime;
    curDestructibleBurnAmount = context.source->destructibleBurnAmount;
    curDestructibleFadeAmount = context.source->destructibleFadeAmount;
    curWetness = context.source->wetness;
    while (1)
    {
        xsurf = modelSurf->surf.xsurf;
        R_GetWorldMatrixForModelSurf(modelSurf, eyeOffset, &worldMat);
        ActiveWorldMatrix = R_GetActiveWorldMatrix(commonSource);
        GfxMatrix tmp; // KISAKTODO: this sucks
        tmp = worldMat;
        ActiveWorldMatrix->matrices.matrix[0] = tmp;
        R_SetModelLightingCoordsForSource(modelSurf->surf.info.lightingHandle, commonSource);
        R_SetReflectionProbe(context, (drawSurf.packed >> 25) & 7);
        R_DrawXModelRigidModelSurf(context, xsurf);
        if (++drawSurfIndex == drawSurfCount)
            break;
        drawSurf.fields = drawSurfList[drawSurfIndex].fields;
        if ((drawSurfMask.packed & drawSurf.packed) != drawSurfKey)
            break;
        v5 = 4 * LOWORD(drawSurf.packed);
        modelSurf = (const GfxModelRigidSurface *)&data->surfsBuffer[v5];
        if (*(unsigned __int16 *)&data->surfsBuffer[v5 + 14] != baseGfxEntIndex)
        {
            if (R_DrawXModelSurfCheckBreak(
                modelSurf->surf.info.gfxEntIndex,
                data,
                depthHackFlags,
                materialTime,
                curDestructibleBurnAmount,
                curDestructibleFadeAmount,
                curWetness,
                0,
                -1,
                0))
            {
                break;
            }
        }
    }
    return drawSurfIndex;
}

void __cdecl R_GetWorldMatrixForModelSurf(const GfxModelRigidSurface *modelSurf, float4 eyeOffset, vector4 *worldMat)
{
    float v3; // [esp+24h] [ebp-58h]
    float v4; // [esp+28h] [ebp-54h]
    float v5; // [esp+30h] [ebp-4Ch]
    float v6; // [esp+34h] [ebp-48h]
    float v7; // [esp+40h] [ebp-3Ch]
    float v8; // [esp+44h] [ebp-38h]
    float v9; // [esp+48h] [ebp-34h]
    float v10; // [esp+4Ch] [ebp-30h]
    float v11; // [esp+50h] [ebp-2Ch]
    float4 quat; // [esp+54h] [ebp-28h] BYREF
    float scale; // [esp+64h] [ebp-18h]
    float4 scaleVec; // [esp+68h] [ebp-14h]

    // Source quat[4] field isn't guaranteed 16-aligned on Linux; the decompiled
    // float4 cast (alignas(16) -> movaps) would fault. Copy the four components.
    quat.v[0] = modelSurf->placement.base.quat[0];
    quat.v[1] = modelSurf->placement.base.quat[1];
    quat.v[2] = modelSurf->placement.base.quat[2];
    quat.v[3] = modelSurf->placement.base.quat[3];
    if ( !Vec4IsNormalized(quat.v)
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\universal\\com_vector4_novec.h",
                    972,
                    0,
                    "%s",
                    "Vec4IsNormalized( quat.v )") )
    {
        __debugbreak();
    }
    v10 = (float)(quat.v[0] + quat.v[0]) * quat.v[0];
    v4 = (float)(quat.v[0] + quat.v[0]) * quat.v[1];
    v8 = (float)(quat.v[0] + quat.v[0]) * quat.v[2];
    v11 = (float)(quat.v[0] + quat.v[0]) * quat.v[3];
    v3 = (float)(quat.v[1] + quat.v[1]) * quat.v[1];
    v9 = (float)(quat.v[1] + quat.v[1]) * quat.v[2];
    v7 = (float)(quat.v[1] + quat.v[1]) * quat.v[3];
    v6 = (float)(quat.v[2] + quat.v[2]) * quat.v[3];
    v5 = (float)(quat.v[2] + quat.v[2]) * quat.v[2];
    worldMat->x.v[0] = 1.0 - (float)(v3 + v5);
    worldMat->x.v[1] = v4 + v6;
    worldMat->x.v[2] = v8 - v7;
    worldMat->x.u[3] = 0;
    worldMat->y.v[0] = v4 - v6;
    worldMat->y.v[1] = 1.0 - (float)(v10 + v5);
    worldMat->y.v[2] = v9 + v11;
    worldMat->y.u[3] = 0;
    worldMat->z.v[0] = v8 + v7;
    worldMat->z.v[1] = v9 - v11;
    worldMat->z.v[2] = 1.0 - (float)(v10 + v3);
    worldMat->z.u[3] = 0;
    scale = modelSurf->placement.scale;
    scaleVec.v[0] = scale;
    scaleVec.v[1] = scale;
    scaleVec.v[2] = scale;
    scaleVec.v[3] = scale;
    worldMat->w.v[0] = modelSurf->placement.base.origin[0];
    worldMat->w.v[1] = modelSurf->placement.base.origin[1];
    worldMat->w.v[2] = modelSurf->placement.base.origin[2];
    worldMat->w.u[3] = 0;
    worldMat->x.v[0] = worldMat->x.v[0] * scaleVec.v[0];
    worldMat->x.v[1] = worldMat->x.v[1] * scaleVec.v[1];
    worldMat->x.v[2] = worldMat->x.v[2] * scaleVec.v[2];
    worldMat->x.v[3] = worldMat->x.v[3] * scaleVec.v[3];
    worldMat->y.v[0] = worldMat->y.v[0] * scaleVec.v[0];
    worldMat->y.v[1] = worldMat->y.v[1] * scaleVec.v[1];
    worldMat->y.v[2] = worldMat->y.v[2] * scaleVec.v[2];
    worldMat->y.v[3] = worldMat->y.v[3] * scaleVec.v[3];
    worldMat->z.v[0] = worldMat->z.v[0] * scaleVec.v[0];
    worldMat->z.v[1] = worldMat->z.v[1] * scaleVec.v[1];
    worldMat->z.v[2] = worldMat->z.v[2] * scaleVec.v[2];
    worldMat->z.v[3] = worldMat->z.v[3] * scaleVec.v[3];
    worldMat->w.v[0] = worldMat->w.v[0] - eyeOffset.v[0];
    worldMat->w.v[1] = worldMat->w.v[1] - eyeOffset.v[1];
    worldMat->w.v[2] = worldMat->w.v[2] - eyeOffset.v[2];
    worldMat->w.v[3] = worldMat->w.v[3] - eyeOffset.v[3];
}

void __cdecl R_DrawXModelRigidModelSurf(GfxCmdBufContext context, XSurface *xsurf)
{
    GfxCmdBufSourceState *ActiveWorldMatrix; // eax
    IDirect3DIndexBuffer9 *ib; // [esp+4h] [ebp-10h]
    GfxDrawPrimArgs args; // [esp+8h] [ebp-Ch] BYREF

    if ( !xsurf
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_draw_xmodel.cpp", 319, 0, "%s", "xsurf") )
    {
        __debugbreak();
    }
    args.baseIndex = 0;
    args.vertexCount = xsurf->vertCount;
    args.triCount = xsurf->triCount;
    context.state->prim.frameStats.geoIndexCount += 3 * args.triCount;
    ib = xsurf->indexBuffer;
    if ( !ib && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_draw_xmodel.cpp", 332, 0, "%s", "ib") )
        __debugbreak();
    if ( context.state->prim.indexBuffer != ib )
        R_ChangeIndices(&context.state->prim, ib);
    R_SetStreamsForXModelSurface(xsurf, &context.state->prim, 0);
    R_SetupPassPerPrimArgs(context);
    R_DrawIndexedPrimitive(&context.state->prim, &args);
    if ( context.state->prim.primStats )
    {
        context.state->prim.primStats->staticIndexCount += 3 * args.triCount;
        context.state->prim.primStats->staticVertexCount += args.vertexCount;
    }
    if ( r_showTess->current.integer )
    {
        ActiveWorldMatrix = R_GetActiveWorldMatrix(context.source);
        RB_ShowTess(context, ActiveWorldMatrix->matrices.matrix[0].m[3], "XMRigid", colorWhite);
    }
}

unsigned int __cdecl R_DrawXModelSurfCamera(
                const GfxDrawSurf *drawSurfList,
                unsigned int drawSurfCount,
                GfxCmdBufContext context)
{
    return R_DrawXModelSurfCameraInternal(drawSurfList, drawSurfCount, context);
}

unsigned int __cdecl R_DrawXModelSurfCameraInternal(
                const GfxDrawSurf *drawSurfList,
                unsigned int drawSurfCount,
                GfxCmdBufContext context)
{
    int v4; // eax
    GfxCmdBufSourceState *matrix; // [esp+A4h] [ebp-ACh]
    vector4 worldMat; // [esp+A8h] [ebp-A8h] BYREF
    unsigned int baseGfxEntIndex; // [esp+ECh] [ebp-64h]
    GfxDrawSurf drawSurf; // [esp+F0h] [ebp-60h]
    const GfxBackEndData *data; // [esp+F8h] [ebp-58h]
    GfxCmdBufSourceState *commonSource; // [esp+FCh] [ebp-54h]
    float curWetness; // [esp+100h] [ebp-50h]
    unsigned int drawSurfIndex; // [esp+104h] [ebp-4Ch]
    float4 eyeOffset; // [esp+108h] [ebp-48h]
    const GfxEntity *gfxEnt; // [esp+11Ch] [ebp-34h]
    GfxDrawSurf drawSurfMask; // [esp+120h] [ebp-30h]
    const GfxModelRigidSurface *modelSurf; // [esp+12Ch] [ebp-24h]
    float curDestructibleBurnAmount; // [esp+130h] [ebp-20h]
    unsigned int depthHackFlags; // [esp+134h] [ebp-1Ch]
    float curDestructibleFadeAmount; // [esp+138h] [ebp-18h]
    XSurface *xsurf; // [esp+13Ch] [ebp-14h]
    float materialTime; // [esp+140h] [ebp-10h]
    int currTextureOverride; // [esp+144h] [ebp-Ch]
    unsigned __int64 drawSurfKey; // [esp+148h] [ebp-8h]

    commonSource = context.source;
    data = context.source->input.data;
    drawSurf.fields = drawSurfList->fields;
    drawSurfMask.packed = -267452416;
    drawSurfKey = drawSurf.packed & 0xFFFFFFFFF00F0000uLL;
    drawSurfIndex = 0;
    eyeOffset.u[0] = LODWORD(context.source->eyeOffset[0]);
    *(_QWORD *)&eyeOffset.unitVec[1].packed = *(_QWORD *)&context.source->eyeOffset[1];
    eyeOffset.v[0] = eyeOffset.v[0] - 0.0;
    eyeOffset.v[1] = eyeOffset.v[1] - 0.0;
    eyeOffset.v[2] = eyeOffset.v[2] - 0.0;
    eyeOffset.v[3] = 0.0 - 1.0;
    modelSurf = (const GfxModelRigidSurface *)((char *)data + 4 * LOWORD(drawSurf.packed));
    baseGfxEntIndex = modelSurf->surf.info.gfxEntIndex;
    depthHackFlags = context.source->depthHackFlags;
    materialTime = context.source->materialTime;
    curDestructibleBurnAmount = context.source->destructibleBurnAmount;
    curDestructibleFadeAmount = context.source->destructibleFadeAmount;
    curWetness = context.source->wetness;
    if (baseGfxEntIndex)
    {
        gfxEnt = &data->gfxEnts[baseGfxEntIndex];
        currTextureOverride = data->gfxEnts[baseGfxEntIndex].textureOverrideIndex;
    }
    else
    {
        currTextureOverride = -1;
    }
    while (1)
    {
        xsurf = modelSurf->surf.xsurf;
        R_GetWorldMatrixForModelSurf(modelSurf, eyeOffset, &worldMat);
        matrix = R_GetActiveWorldMatrix(commonSource);
        GfxMatrix tmp;
        tmp = worldMat; // KISAKTODO: sucks
        matrix->matrices.matrix[0] = tmp;
        if (currTextureOverride >= 0)
            R_TextureOverride(data, context, modelSurf->surf.info.dobjModelIndex, currTextureOverride);
        R_DrawXModelRigidModelSurf(context, xsurf);
        if (++drawSurfIndex == drawSurfCount)
            break;
        drawSurf.fields = drawSurfList[drawSurfIndex].fields;
        if ((drawSurfMask.packed & drawSurf.packed) != drawSurfKey)
            break;
        v4 = 4 * LOWORD(drawSurf.packed);
        modelSurf = (const GfxModelRigidSurface *)&data->surfsBuffer[v4];
        if (*(unsigned __int16 *)&data->surfsBuffer[v4 + 14] != baseGfxEntIndex)
        {
            if (R_DrawXModelSurfCheckBreak(
                modelSurf->surf.info.gfxEntIndex,
                data,
                depthHackFlags,
                materialTime,
                curDestructibleBurnAmount,
                curDestructibleFadeAmount,
                curWetness,
                1,
                currTextureOverride,
                0))
            {
                break;
            }
        }
    }
    return drawSurfIndex;
}

unsigned int __cdecl R_DrawXModelSurf(
                const GfxDrawSurf *drawSurfList,
                unsigned int drawSurfCount,
                GfxCmdBufContext context)
{
    return R_DrawXModelSurfInternal(drawSurfList, drawSurfCount, context);
}

unsigned int __cdecl R_DrawXModelSurfInternal(
    const GfxDrawSurf *drawSurfList,
    unsigned int drawSurfCount,
    GfxCmdBufContext context)
{
    GfxCmdBufSourceState *ActiveWorldMatrix; // eax
    vector4 worldMat; // [esp+8Ch] [ebp-80h] BYREF
    GfxDrawSurf drawSurf; // [esp+CCh] [ebp-40h]
    const GfxBackEndData *data; // [esp+D4h] [ebp-38h]
    GfxCmdBufSourceState *commonSource; // [esp+D8h] [ebp-34h]
    unsigned int drawSurfIndex; // [esp+DCh] [ebp-30h]
    float4 eyeOffset; // [esp+E0h] [ebp-2Ch]
    GfxDrawSurf drawSurfMask; // [esp+F4h] [ebp-18h]
    const GfxModelRigidSurface *modelSurf; // [esp+FCh] [ebp-10h]
    XSurface *xsurf; // [esp+100h] [ebp-Ch]
    unsigned __int64 drawSurfKey; // [esp+104h] [ebp-8h]

    commonSource = context.source;
    data = context.source->input.data;
    drawSurf.fields = drawSurfList->fields;
    drawSurfMask.packed = -267452416;
    drawSurfKey = drawSurf.packed & 0xFFFFFFFFF00F0000uLL;
    drawSurfIndex = 0;
    *(_QWORD *)eyeOffset.v = *(_QWORD *)context.source->eyeOffset;
    eyeOffset.u[2] = LODWORD(context.source->eyeOffset[2]);
    eyeOffset.v[0] = eyeOffset.v[0] - 0.0;
    eyeOffset.v[1] = eyeOffset.v[1] - 0.0;
    eyeOffset.v[2] = eyeOffset.v[2] - 0.0;
    eyeOffset.v[3] = 0.0 - 1.0;
    do
    {
        modelSurf = (const GfxModelRigidSurface *)((char *)data + 4 * LOWORD(drawSurf.packed));
        xsurf = modelSurf->surf.xsurf;
        R_GetWorldMatrixForModelSurf(modelSurf, eyeOffset, &worldMat);
        ActiveWorldMatrix = R_GetActiveWorldMatrix(commonSource);
        memcpy(&ActiveWorldMatrix->matrices.matrix[0], &worldMat, sizeof(GfxMatrix));
        R_DrawXModelRigidModelSurf(context, xsurf);
        if (++drawSurfIndex == drawSurfCount)
            break;
        drawSurf.packed = drawSurfList[drawSurfIndex].packed;
    } while ((drawSurfMask.packed & drawSurf.packed) == drawSurfKey);
    return drawSurfIndex;
}
