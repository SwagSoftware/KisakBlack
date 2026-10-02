#include "r_draw_lit.h"
#include "r_state_utils.h"
#include "r_state.h"
#include "r_wind.h"
#include "rb_draw3d.h"
#include "r_foliage.h"
#include "r_adszscale.h"
#include "rb_fog.h"
#include "r_dvars.h"

void    R_SetDrawLitConstants(
                GfxCmdBufSourceState *source,
                const GfxViewInfo *viewInfo,
                const GfxViewParms *viewParms)
{
    GfxMatrix skyColorMatrix;

    R_SetWindShaderConstants(source);
    R_SetSunConstants(&source->input);

    const float *cloudArea = (float*)R_GetCloudArea();
    R_SetCodeConstant(source,
        CONST_SRC_CODE_CLOUD_WORLD_AREA,
        cloudArea[0] - viewParms->origin[0],
        cloudArea[1] - viewParms->origin[1],
        cloudArea[2],
        cloudArea[3]);
    R_SetADSZScaleConstants(viewInfo->localClientNum, source);
    R_SetFrameFog(&source->input, viewInfo);
    R_SetCodeConstant(source,
        CONST_SRC_CODE_TREECANOPY_PARMS,
        rgp.world->sunParse.treeScatterIntensity,
        rgp.world->sunParse.treeScatterAmount,
        0.0f,
        0.0f);

    colorTempMatrix(skyColorMatrix.m, r_skyColorTemp->current.value);

    R_SetCodeConstant(source,
        CONST_SRC_CODE_SKY_TRANSITION,
        r_skyTransition->current.value,
        0.0f,
        (float)r_treeScale->current.integer,
        r_testScale->current.value);

    R_SetCodeConstant(source,
        CONST_SRC_CODE_COLOR_MATRIX_R,
        skyColorMatrix.m[0][0],
        skyColorMatrix.m[1][0],
        skyColorMatrix.m[2][0],
        skyColorMatrix.m[3][0]);

    R_SetCodeConstant(source,
        CONST_SRC_CODE_COLOR_MATRIX_G,
        skyColorMatrix.m[0][1],
        skyColorMatrix.m[1][1],
        skyColorMatrix.m[2][1],
        skyColorMatrix.m[3][1]);

    R_SetCodeConstant(source,
        CONST_SRC_CODE_COLOR_MATRIX_B,
        skyColorMatrix.m[0][2],
        skyColorMatrix.m[1][2],
        skyColorMatrix.m[2][2],
        skyColorMatrix.m[3][2]);
}

void R_DrawCloakHDR(const GfxViewInfo *viewInfo, GfxCmdBuf *cmdBuf, CloakPhaseID phase)
{
    const float *CloudArea; // [esp-1AA4h] [ebp-1AB0h]
    GfxCmdBufSourceState state; // [esp-1AA0h] [ebp-1AACh] BYREF

    PROF_SCOPED("R_DrawCloak");

    R_InitCmdBufSourceState(&state, &viewInfo->input, 1);
    R_SetRenderTargetSize(&state, viewInfo->sceneComposition.mainSceneMSAA);
    R_SetViewportStruct(&state, &viewInfo->cullViewInfo.sceneViewport);
    R_SetWindShaderConstants(&state);

    CloudArea = (const float *)R_GetCloudArea();

    R_SetCodeConstant(&state, 
        CONST_SRC_CODE_CLOUD_WORLD_AREA,
        CloudArea[0] - viewInfo->cullViewInfo.viewParms.origin[0],
        CloudArea[1] - viewInfo->cullViewInfo.viewParms.origin[1],
        CloudArea[2],
        CloudArea[3]
        );

    R_SetCodeConstant(&state,
        CONST_SRC_CODE_TREECANOPY_PARMS,
        rgp.world->sunParse.treeScatterIntensity,
        rgp.world->sunParse.treeScatterAmount,
        0.0f,
        0.0f
    );

    R_SetADSZScaleConstants(viewInfo->localClientNum, &state);

    if (phase)
    {
        if (phase == CLOAK_PHASE_CLOAKED)
            R_DrawCall(
                R_DrawCloakPostEmissiveCallbackHDR,
                viewInfo,
                &state,
                viewInfo,
                &viewInfo->drawList[6],
                &viewInfo->cullViewInfo.viewParms,
                cmdBuf,
                0);
    }
    else
    {
        R_DrawCall(
            R_DrawCloakPrePassCallbackHDR,
            viewInfo,
            &state,
            viewInfo,
            &viewInfo->drawList[6],
            &viewInfo->cullViewInfo.viewParms,
            cmdBuf,
            0);
    }
}

void __cdecl R_DrawCloakPrePassCallbackHDR(
                const void *userData,
                GfxCmdBufContext context,
                GfxCmdBufContext prepassContext)
{
    int v3; // [esp+30h] [ebp-60h]
    jqBatchGroup *i; // [esp+34h] [ebp-5Ch]
    GfxDrawSurfListInfo info; // [esp+38h] [ebp-58h] BYREF
    const GfxViewInfo *viewInfo; // [esp+8Ch] [ebp-4h]

    viewInfo = (const GfxViewInfo *)userData;

    R_SetRenderTarget(context, viewInfo->sceneComposition.mainSceneMSAA);
    if ( (viewInfo->sceneComposition.renderingMode & 7) == 0 )
        R_HW_EnableScissor(
            context.state->prim.device,
            viewInfo->cullViewInfo.scissorViewport.x,
            viewInfo->cullViewInfo.scissorViewport.y,
            viewInfo->cullViewInfo.scissorViewport.width,
            viewInfo->cullViewInfo.scissorViewport.height);
    memcpy(&info, &viewInfo->drawList[6], sizeof(info));
    info.baseTechType = 10;
    R_DrawSurfs(context, prepassContext.state, &viewInfo->drawList[6]);
    if ( (viewInfo->sceneComposition.renderingMode & 7) == 0 )
        R_HW_DisableScissor(context.state->prim.device);
}

void __cdecl R_DrawCloakPostEmissiveCallbackHDR(
                const void *userData,
                GfxCmdBufContext context,
                GfxCmdBufContext prepassContext)
{
    int v3; // [esp+30h] [ebp-60h]
    jqBatchGroup *i; // [esp+34h] [ebp-5Ch]
    GfxDrawSurfListInfo info; // [esp+38h] [ebp-58h] BYREF
    const GfxViewInfo *viewInfo; // [esp+8Ch] [ebp-4h]

    viewInfo = (const GfxViewInfo * )userData;

    R_SetRenderTarget(context, viewInfo->sceneComposition.mainSceneMSAA);
    if ( (viewInfo->sceneComposition.renderingMode & 7) == 0 )
        R_HW_EnableScissor(
            context.state->prim.device,
            viewInfo->cullViewInfo.scissorViewport.x,
            viewInfo->cullViewInfo.scissorViewport.y,
            viewInfo->cullViewInfo.scissorViewport.width,
            viewInfo->cullViewInfo.scissorViewport.height);
    memcpy(&info, &viewInfo->drawList[6], sizeof(info));
    info.baseTechType = 4;
    R_DrawSurfs(context, prepassContext.state, &info);
    if ( (viewInfo->sceneComposition.renderingMode & 7) == 0 )
        R_HW_DisableScissor(context.state->prim.device);
}

void    R_DrawLit(
                const GfxViewInfo *viewInfo,
                GfxCmdBuf *cmdBuf,
                GfxCmdBuf *prepassCmdBuf,
                LitPhaseID phase)
{
    void *v5; // esp
    GfxCmdBufSourceState v6; // [esp-1AA0h] [ebp-1AACh] BYREF

    R_InitCmdBufSourceState(&v6, &viewInfo->input, 1);
    R_SetRenderTargetSize(&v6, viewInfo->sceneComposition.mainSceneMSAA);
    R_SetViewportStruct(&v6, &viewInfo->cullViewInfo.sceneViewport);
    R_SetDrawLitConstants(&v6, viewInfo, &viewInfo->cullViewInfo.viewParms);
    if ( phase )
    {
        if ( phase == LIT_PHASE_POST_RESOLVE )
        {
            R_DrawCall(
                R_DrawLitPostResolveCallback,
                viewInfo,
                &v6,
                viewInfo,
                &viewInfo->drawList[1],
                &viewInfo->cullViewInfo.viewParms,
                cmdBuf,
                prepassCmdBuf);
        }
        else if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_draw_lit.cpp", 636, 0, "invalid phase") )
        {
            __debugbreak();
        }
    }
    else
    {
        R_DrawCall(
            R_DrawLitCallback,
            viewInfo,
            &v6,
            viewInfo,
            viewInfo->drawList,
            &viewInfo->cullViewInfo.viewParms,
            cmdBuf,
            prepassCmdBuf);
    }
}

void __cdecl R_DrawLitCallback(const void *userData, GfxCmdBufContext context, GfxCmdBufContext prepassContext)
{
    const GfxViewInfo *viewInfo = (const GfxViewInfo *)userData;

    R_SetRenderTarget(context, viewInfo->sceneComposition.mainSceneMSAA);
    if (prepassContext.state)
        R_SetRenderTarget(prepassContext, viewInfo->sceneComposition.mainSceneMSAA);
    if ((viewInfo->sceneComposition.renderingMode & 7) == 0)
        R_HW_EnableScissor(
            context.state->prim.device,
            viewInfo->cullViewInfo.scissorViewport.x,
            viewInfo->cullViewInfo.scissorViewport.y,
            viewInfo->cullViewInfo.scissorViewport.width,
            viewInfo->cullViewInfo.scissorViewport.height);
    R_DrawSurfs(context, prepassContext.state, viewInfo->drawList);
    if ((viewInfo->sceneComposition.renderingMode & 7) == 0)
        R_HW_DisableScissor(context.state->prim.device);
}

void __cdecl R_DrawLitPostResolveCallback(const void *userData, GfxCmdBufContext context, GfxCmdBufContext prepassContext)
{
    const GfxViewInfo *viewInfo = (const GfxViewInfo *)userData;

    R_SetRenderTarget(context, viewInfo->sceneComposition.mainSceneMSAA);
    if (prepassContext.state)
        R_SetRenderTarget(prepassContext, viewInfo->sceneComposition.mainSceneMSAA);
    if ((viewInfo->sceneComposition.renderingMode & 7) == 0)
        R_HW_EnableScissor(
            context.state->prim.device,
            viewInfo->cullViewInfo.scissorViewport.x,
            viewInfo->cullViewInfo.scissorViewport.y,
            viewInfo->cullViewInfo.scissorViewport.width,
            viewInfo->cullViewInfo.scissorViewport.height);
    R_DrawSurfs(context, prepassContext.state, &viewInfo->drawList[1]);
    if ((viewInfo->sceneComposition.renderingMode & 7) == 0)
        R_HW_DisableScissor(context.state->prim.device);
}

void R_DrawDecal(const GfxViewInfo *viewInfo, GfxCmdBuf *cmdBuf, GfxCmdBuf *prepassCmdBuf)
{
    const float *CloudArea; // [esp-1AA4h] [ebp-1AB0h]
    GfxCmdBufSourceState v10; // [esp-1AA0h] [ebp-1AACh] BYREF

    PROF_SCOPED("R_DrawDecal");

    R_InitCmdBufSourceState(&v10, &viewInfo->input, 1);
    R_SetRenderTargetSize(&v10, viewInfo->sceneComposition.mainSceneMSAA);
    R_SetViewportStruct(&v10, &viewInfo->cullViewInfo.sceneViewport);
    R_SetWindShaderConstants(&v10);

    CloudArea = (const float *)R_GetCloudArea();
    v10.input.consts[115][0] = CloudArea[0] - viewInfo->cullViewInfo.viewParms.origin[0];
    v10.input.consts[115][1] = CloudArea[1] - viewInfo->cullViewInfo.viewParms.origin[1];
    v10.input.consts[115][2] = CloudArea[2];
    v10.input.consts[115][3] = CloudArea[3];
    R_DirtyCodeConstant(&v10, CONST_SRC_CODE_CLOUD_WORLD_AREA);

    v10.input.consts[119][0] = rgp.world->sunParse.treeScatterIntensity;
    v10.input.consts[119][1] = rgp.world->sunParse.treeScatterAmount;
    v10.input.consts[119][2] = 0.0f;
    v10.input.consts[119][3] = 0.0f;
    R_DirtyCodeConstant(&v10, CONST_SRC_CODE_TREECANOPY_PARMS);

    R_SetADSZScaleConstants(viewInfo->localClientNum, &v10);
    R_DrawCall(
        R_DrawDecalCallback,
        viewInfo,
        &v10,
        viewInfo,
        &viewInfo->drawList[3],
        &viewInfo->cullViewInfo.viewParms,
        cmdBuf,
        prepassCmdBuf);
}

void __cdecl R_DrawDecalCallback(const void *userData, GfxCmdBufContext context, GfxCmdBufContext prepassContext)
{
    const GfxViewInfo *viewInfo = (const GfxViewInfo *)userData;

    R_SetRenderTarget(context, viewInfo->sceneComposition.mainSceneMSAA);
    if (prepassContext.state)
        R_SetRenderTarget(prepassContext, viewInfo->sceneComposition.mainSceneMSAA);
    if ((viewInfo->sceneComposition.renderingMode & 7) == 0)
        R_HW_EnableScissor(
            context.state->prim.device,
            viewInfo->cullViewInfo.scissorViewport.x,
            viewInfo->cullViewInfo.scissorViewport.y,
            viewInfo->cullViewInfo.scissorViewport.width,
            viewInfo->cullViewInfo.scissorViewport.height);
    R_DrawSurfs(context, prepassContext.state, &viewInfo->drawList[3]);
    if ((viewInfo->sceneComposition.renderingMode & 7) == 0)
        R_HW_DisableScissor(context.state->prim.device);
}

