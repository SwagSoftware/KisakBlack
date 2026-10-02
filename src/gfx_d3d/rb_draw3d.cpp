#include "rb_draw3d.h"
#include "r_dvars.h"
#include "r_state.h"
#include "rb_backend.h"
#include "r_state_utils.h"
#include "r_adszscale.h"
#include "r_foliage.h"
#include "rb_logfile.h"
#include "r_draw_method.h"
#include "r_cmdbuf.h"
#include "rb_sky.h"
#include <client/cl_compositing.h>
#include "rb_debug.h"
#include "rb_showcollision.h"
#include "r_ui3d.h"
#include "rb_sunshadow.h"
#include "rb_spotshadow.h"
#include "rb_depthprepass.h"
#include "r_draw_lit.h"
#include "rb_corona.h"
#include "rb_superflare.h"
#include <cgame/cg_hudelem.h>
#include "rb_shade.h"
#include "r_wind.h"
#include "rb_postfx.h"
#include "r_extracam.h"

float r_cloudArea[4] = { 0.0, 0.0, 1.0e7, 0.0 };

const float (*__cdecl R_GetCloudArea())[4]
{
    return (const float (*)[4])r_cloudArea;
}

void __cdecl R_CalcGameTimeVec(float gameTime, float *out)
{
    long double v2; // st7
    float v3; // [esp+8h] [ebp-1Ch]
    float cosOfFracPartOfGameTime; // [esp+18h] [ebp-Ch]
    float sinOfFracPartOfGameTime; // [esp+1Ch] [ebp-8h]
    float fracPartOfGameTime; // [esp+20h] [ebp-4h]

    v3 = floor(gameTime);
    fracPartOfGameTime = gameTime - v3;
    v2 = (float)(6.2831855 * fracPartOfGameTime);
    cosOfFracPartOfGameTime = cos(v2);
    sinOfFracPartOfGameTime = sin(v2);
    *out = sinOfFracPartOfGameTime;
    out[1] = cosOfFracPartOfGameTime;
    out[2] = fracPartOfGameTime;
    out[3] = gameTime;
}

void __cdecl R_ShowTris(GfxCmdBufContext context, const GfxDrawSurfListInfo *info)
{
    int v2; // [esp+14h] [ebp-60h]
    jqBatchGroup *i; // [esp+18h] [ebp-5Ch]
    GfxDrawSurfListInfo debugInfo; // [esp+1Ch] [ebp-58h] BYREF
    int showTris; // [esp+70h] [ebp-4h]

    showTris = r_showTris->current.integer;

    if (!showTris)
    {
        return;
    }

    if ( showTris != 3 || info == &info->viewInfo->drawList[DRAWLIST_EMISSIVE] )
    {
        if ( showTris == 2 )
            R_ClearScreen(context.state->prim.device, 6u, colorWhite, 1.0, 0, 0);

        debugInfo = *info;
        debugInfo.baseTechType = 125;
        R_DrawSurfs(context, 0, &debugInfo);
    }
}

void    R_DrawEmissive(const GfxViewInfo *viewInfo, GfxCmdBuf *cmdBuf)
{
    GfxCmdBufSourceState state; // [sp+50h] [-F00h] BYREF

    R_InitCmdBufSourceState(&state, &viewInfo->input, 1);
    R_SetRenderTargetSize(&state, viewInfo->sceneComposition.mainSceneMSAA);
    R_SetViewportStruct(&state, &viewInfo->cullViewInfo.sceneViewport);
    R_SetADSZScaleConstants(viewInfo->localClientNum, &state);
    R_DrawCall(
        R_DrawEmissiveCallback,
        viewInfo,
        &state,
        viewInfo,
        &viewInfo->drawList[4],
        &viewInfo->cullViewInfo.viewParms,
        cmdBuf,
        0);
}

void __cdecl R_DrawEmissiveCallback(const void *userData, GfxCmdBufContext context, GfxCmdBufContext prepassContext)
{
    GfxViewInfo *viewInfo = (GfxViewInfo *)userData;

    PROF_SCOPED("R_DrawEmissiveCallback");

    R_UpdateCodeConstant(context.source, CONST_SRC_CODE_FRAMEBUFFER_READ, 0.0, 0.0, 1.0, 1.0);

    {
        PROF_SCOPED("emissive");
        R_DrawSurfs(context, 0, &viewInfo->drawList[DRAWLIST_EMISSIVE]);
    }
    
    R_ShowTris(context, &viewInfo->drawList[DRAWLIST_LIT]);
    R_ShowTris(context, &viewInfo->drawList[DRAWLIST_DECAL]);
    R_ShowTris(context, &viewInfo->drawList[DRAWLIST_EMISSIVE]);

    if ((viewInfo->sceneComposition.renderingMode & 7) == 0)
        R_HW_DisableScissor(context.state->prim.device);
}

void __cdecl R_HW_DisableScissor(IDirect3DDevice9 *device)
{
    const char *v1; // eax
    int hr; // [esp+4h] [ebp-4h]

    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("device->SetRenderState( D3DRS_SCISSORTESTENABLE, 0 )\n");
    //hr = ((int (__thiscall *)(IDirect3DDevice9 *, IDirect3DDevice9 *, int, unsigned int))device->SetRenderState)(
    //             device,
    //             device,
    //             174,
    //             0);
    hr = device->SetRenderState(D3DRS_SCISSORTESTENABLE, 0);
    if ( hr < 0 )
    {
        ++g_disableRendering;
        v1 = R_ErrorDescription(hr);
        Com_Error(
            ERR_FATAL,
            "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_SCISSORTESTENABLE,"
            " 0 ) failed: %s\n",
            701,
            v1);
    }
}

void    R_DrawReflected(const GfxViewInfo *viewInfo, GfxCmdBuf *cmdBuf)
{
    GfxCmdBufSourceState state; // [esp-1AA0h] [ebp-1AACh] BYREF

    R_InitCmdBufSourceState(&state, &viewInfo->input, 1);
    R_SetRenderTargetSize(&state, viewInfo->sceneComposition.mainSceneMSAA);
    R_SetViewportStruct(&state, &viewInfo->cullViewInfo.sceneViewport);
    R_DrawCall(
        R_DrawReflectedCallback,
        viewInfo,
        &state,
        viewInfo,
        &viewInfo->drawList[5],
        &viewInfo->cullViewInfo.viewParms,
        cmdBuf,
        0);
}

void __cdecl R_DrawReflectedCallback(const void *userData, GfxCmdBufContext context, GfxCmdBufContext prepassContext)
{
    const GfxViewInfo *viewInfo = (const GfxViewInfo *)userData;

    R_InitLocalCmdBufState(context.state);
    R_SetRenderTargetSize(context.source, R_RENDERTARGET_POST_EFFECT_SRC);
    R_SetRenderTarget(context, R_RENDERTARGET_POST_EFFECT_SRC);
    R_Set3D(context.source);

    float clearColor[4]{ 0 }; // [esp+20h] [ebp-14h] BYREF

    R_ClearScreen(context.state->prim.device, 1, clearColor, 1.0, 0, 0);

    {
        PROF_SCOPED("reflected");
        R_DrawSurfs(context, 0, &viewInfo->drawList[DRAWLIST_REFLECTED]);
    }
    
    R_HW_DisableScissor(context.state->prim.device);
}

void __cdecl R_InitLocalCmdBufState(GfxCmdBufState *state)
{
    memcpy(state->refSamplerState, gfxCmdBufState.refSamplerState, sizeof(GfxCmdBufState));
    memset(state->vertexShaderConstState, 0, sizeof(state->vertexShaderConstState));
    memset(state->pixelShaderConstState, 0, sizeof(state->pixelShaderConstState));
}

bool __cdecl RB_ShouldDrawCoronas()
{
    //return !R_StereoActivated() && !r_reflectionProbeGenerate->current.enabled;
    return !R_StereoActivated() && !r_reflectionProbeGenerate->current.enabled && r_drawSun->current.enabled; // LWSS ADD drawSun condition so I can turn this off (It uses Occlusion event query)
}

void __cdecl RB_Draw3DInternal(GfxViewInfo *viewInfo)
{
  if ( !rgp.world
    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_draw3d.cpp", 7732, 0, "%s", "rgp.world") )
  {
    __debugbreak();
  }
  if ( gfxDrawMethod.drawScene )
  {
    if ( gfxDrawMethod.drawScene == GFX_DRAW_SCENE_FULLBRIGHT )
    {
      RB_FullbrightDrawCommands(viewInfo);
    }
    else if ( gfxDrawMethod.drawScene == GFX_DRAW_SCENE_DEBUGSHADER )
    {
      RB_DebugShaderDrawCommands(viewInfo);
    }
    else
    {
      if ( gfxDrawMethod.drawScene != GFX_DRAW_SCENE_STANDARD
        && !Assert_MyHandler(
              "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_draw3d.cpp",
              7775,
              0,
              "%s\n\t(gfxDrawMethod.drawScene) = %i",
              "(gfxDrawMethod.drawScene == GFX_DRAW_SCENE_STANDARD)",
              gfxDrawMethod.drawScene) )
      {
        __debugbreak();
      }
      RB_StandardDrawCommands(viewInfo);
    }
  }
  else
  {
    R_InitLocalCmdBufState(&gfxCmdBufState);
    R_SetRenderTargetSize(&gfxCmdBufSourceState, R_RENDERTARGET_FRAME_BUFFER);
    R_SetRenderTarget(gfxCmdBufContext, R_RENDERTARGET_FRAME_BUFFER);
    memcpy(gfxCmdBufState.refSamplerState, gfxCmdBufState.refSamplerState, sizeof(gfxCmdBufState));
  }
}

void __cdecl RB_FullbrightDrawCommands(const GfxViewInfo *viewInfo)
{
    const GfxBackEndData *data; // [esp+30h] [ebp-8h]
    GfxCmdBuf cmdBuf; // [esp+34h] [ebp-4h] BYREF

    PROF_SCOPED("RB_FullbrightDrawCommands");

    data = backEndData;
    R_SetAndClearSceneTarget(0, viewInfo);
    R_InitContext(data, &cmdBuf);
    R_DrawFullbright(viewInfo, &gfxCmdBufContext.source->input, &cmdBuf);
    R_InitCmdBufSourceState(gfxCmdBufContext.source, &viewInfo->input, 0);
    R_InitLocalCmdBufState(gfxCmdBufContext.state);
    R_SetRenderTargetSize(gfxCmdBufContext.source, viewInfo->sceneComposition.mainSceneMSAA);
    R_SetRenderTarget(gfxCmdBufContext, viewInfo->sceneComposition.mainSceneMSAA);
    R_BeginView(gfxCmdBufContext.source, &viewInfo->sceneDef, &viewInfo->cullViewInfo.viewParms);
    R_SetViewportStruct(gfxCmdBufContext.source, &viewInfo->cullViewInfo.sceneViewport);
    RB_DrawSun(viewInfo->localClientNum);
    memcpy(gfxCmdBufState.refSamplerState, gfxCmdBufContext.state->refSamplerState, sizeof(gfxCmdBufState));
    RB_EndSceneRendering(gfxCmdBufContext, &viewInfo->input, viewInfo);
}

void __cdecl RB_EndSceneRendering(GfxCmdBufContext context, const GfxCmdBufInput *input, const GfxViewInfo *viewInfo)
{
    PROF_SCOPED("RB_EndSceneRendering()");

    R_HW_InsertFence(&backEndData->endFence);
    R_InitCmdBufSourceState(context.source, input, 0);
    R_InitLocalCmdBufState(context.state);
    R_SetRenderTargetSize(&gfxCmdBufSourceState, viewInfo->sceneComposition.mainSceneMSAA);
    R_BeginView(&gfxCmdBufSourceState, &viewInfo->sceneDef, &viewInfo->cullViewInfo.viewParms);
    R_SetViewportStruct(&gfxCmdBufSourceState, &viewInfo->cullViewInfo.sceneViewport);
    R_SetRenderTarget(gfxCmdBufContext, viewInfo->sceneComposition.mainSceneMSAA);
    if ( developer->current.integer )
    {
    R_Set3D(&gfxCmdBufSourceState);
    if ( gfxCmdBufSourceState.viewMode != VIEW_MODE_3D
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_draw3d.cpp",
            4182,
            0,
            "%s\n\t(gfxCmdBufSourceState.viewMode) = %i",
            "(gfxCmdBufSourceState.viewMode == VIEW_MODE_3D)",
            gfxCmdBufSourceState.viewMode) )
    {
        __debugbreak();
    }
    RB_DrawDebug(&gfxCmdBufSourceState.viewParms);
    RB_ShowCollision(&gfxCmdBufSourceState.viewParms);
    }
    memcpy(gfxCmdBufState.refSamplerState, context.state->refSamplerState, sizeof(gfxCmdBufState));
}

void __cdecl R_SetAndClearSceneTarget(const GfxViewport *viewport, const GfxViewInfo *viewInfo)
{
    R_InitLocalCmdBufState(&gfxCmdBufState);

    PROF_SCOPED("Clear Render Target");
    R_SetRenderTargetSize(&gfxCmdBufSourceState, viewInfo->sceneComposition.mainSceneMSAA);
    R_SetRenderTarget(gfxCmdBufContext, viewInfo->sceneComposition.mainSceneMSAA);
    if ((viewInfo->sceneComposition.renderingMode & 7) != 0)
    {
        R_ClearForFrameBuffer(gfxCmdBufState.prim.device, 0);
    }
    else
    {
        R_ClearForFrameBuffer(gfxCmdBufState.prim.device, viewport);
    }
    memcpy(gfxCmdBufState.refSamplerState, gfxCmdBufState.refSamplerState, sizeof(gfxCmdBufState));
}

void __cdecl R_ClearForFrameBuffer(IDirect3DDevice9 *device, const GfxViewport *viewport)
{
    float clearColor[4]; // [esp+10h] [ebp-10h] BYREF

    R_GetClearColor(clearColor);
    R_ClearScreen(device, 7u, clearColor, 1.0, 0, viewport);
}

void __cdecl R_DrawFullbright(const GfxViewInfo *viewInfo, GfxCmdBufInput *input, GfxCmdBuf *cmdBuf)
{
    {
        PROF_SCOPED("R_DrawFullbrightOrDebugShader - Lit");
        R_DrawFullbrightOrDebugShader(
            R_DrawFullbrightLitCallback,
            viewInfo,
            viewInfo->drawList,
            cmdBuf);
    }
    {
        PROF_SCOPED("R_DrawFullbrightOrDebugShader - Decal");
        R_DrawFullbrightOrDebugShader(
            R_DrawFullbrightDecalCallback,
            viewInfo,
            &viewInfo->drawList[3],
            cmdBuf);
    }
    {
        PROF_SCOPED("R_DrawFullbrightOrDebugShader - Emissive");
        R_DrawFullbrightOrDebugShader(
            R_DrawFullbrightEmissiveCallback,
            viewInfo,
            &viewInfo->drawList[4],
            cmdBuf);
    }
}

void __cdecl R_DrawFullbrightLitCallback(const void *userData, GfxCmdBufContext context, GfxCmdBufContext prepassContext)
{
    const GfxViewInfo *viewInfo = (const GfxViewInfo *)userData;

    R_SetRenderTarget(context, viewInfo->sceneComposition.mainSceneMSAA);

    if ((viewInfo->sceneComposition.renderingMode & 7) == 0)
    {
        R_HW_EnableScissor(
            context.state->prim.device,
            viewInfo->cullViewInfo.scissorViewport.x,
            viewInfo->cullViewInfo.scissorViewport.y,
            viewInfo->cullViewInfo.scissorViewport.width,
            viewInfo->cullViewInfo.scissorViewport.height);
    }
        
    R_DrawSurfs(context, 0, viewInfo->drawList);

    if ((viewInfo->sceneComposition.renderingMode & 7) == 0)
        R_HW_DisableScissor(context.state->prim.device);
}

void __cdecl R_HW_EnableScissor(
                IDirect3DDevice9 *device,
                unsigned int x,
                unsigned int y,
                unsigned int w,
                unsigned int h)
{
    const char *v5; // eax
    const char *v6; // eax
    int v8; // [esp+4h] [ebp-1Ch]
    int hr; // [esp+Ch] [ebp-14h]
    tagRECT scissor; // [esp+10h] [ebp-10h] BYREF

    scissor.left = x;
    scissor.top = y;
    scissor.right = w + x;
    scissor.bottom = h + y;
    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("device->SetRenderState( D3DRS_SCISSORTESTENABLE, 1 )\n");
    hr = device->SetRenderState(D3DRS_SCISSORTESTENABLE, 1u);
    if ( hr < 0 )
    {
        ++g_disableRendering;
        v5 = R_ErrorDescription(hr);
        Com_Error(
            ERR_FATAL,
            "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_SCISSORTESTENABLE,"
            " 1 ) failed: %s\n",
            694,
            v5);
    }
    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("device->SetScissorRect( &scissor )\n");
    v8 = device->SetScissorRect(&scissor);
    if ( v8 < 0 )
    {
        ++g_disableRendering;
        v6 = R_ErrorDescription(v8);
        Com_Error(
            ERR_FATAL,
            "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetScissorRect( &scissor ) failed: %s\n",
            695,
            v6);
    }
}

void __cdecl R_DrawFullbrightDecalCallback(const void *userData, GfxCmdBufContext context, GfxCmdBufContext prepassContext)
{
    const GfxViewInfo *viewInfo = (const GfxViewInfo *)userData;

    R_SetRenderTarget(context, viewInfo->sceneComposition.mainSceneMSAA);

    if ((viewInfo->sceneComposition.renderingMode & 7) == 0)
        R_HW_EnableScissor(
            context.state->prim.device,
            viewInfo->cullViewInfo.scissorViewport.x,
            viewInfo->cullViewInfo.scissorViewport.y,
            viewInfo->cullViewInfo.scissorViewport.width,
            viewInfo->cullViewInfo.scissorViewport.height);

    R_DrawSurfs(context, 0, &viewInfo->drawList[DRAWLIST_DECAL]);

    if ((viewInfo->sceneComposition.renderingMode & 7) == 0)
        R_HW_DisableScissor(context.state->prim.device);
}

void __cdecl R_DrawFullbrightEmissiveCallback(const void *userData, GfxCmdBufContext context, GfxCmdBufContext prepassContext)
{
    const GfxViewInfo *viewInfo = (const GfxViewInfo *)userData;

    R_SetRenderTarget(context, viewInfo->sceneComposition.mainSceneMSAA);
    if ((viewInfo->sceneComposition.renderingMode & 7) == 0)
        R_HW_EnableScissor(
            context.state->prim.device,
            viewInfo->cullViewInfo.scissorViewport.x,
            viewInfo->cullViewInfo.scissorViewport.y,
            viewInfo->cullViewInfo.scissorViewport.width,
            viewInfo->cullViewInfo.scissorViewport.height);
    R_DrawSurfs(context, 0, &viewInfo->drawList[4]);
    R_ShowTris(context, viewInfo->drawList);
    R_ShowTris(context, &viewInfo->drawList[1]);
    R_ShowTris(context, &viewInfo->drawList[3]);
    R_ShowTris(context, &viewInfo->drawList[4]);
    if ((viewInfo->sceneComposition.renderingMode & 7) == 0)
        R_HW_DisableScissor(context.state->prim.device);
}

void R_DrawFullbrightOrDebugShader(
    void(__cdecl *callback)(const void *, GfxCmdBufContext , GfxCmdBufContext ),
    const GfxViewInfo *viewInfo,
    const GfxDrawSurfListInfo *info,
    GfxCmdBuf *cmdBuf)
{
    void *v5; // esp
    GfxCmdBufSourceState v9; // [esp-1AA0h] [ebp-1AACh] BYREF
    const float *CloudArea; // [esp-8h] [ebp-14h]
    int v11; // [esp+0h] [ebp-Ch]
    void *v12; // [esp+4h] [ebp-8h]
    void *retaddr; // [esp+Ch] [ebp+0h]

    CloudArea = (const float *)R_GetCloudArea();
    R_InitCmdBufSourceState(&v9, &viewInfo->input, 1);
    R_SetRenderTargetSize(&v9, viewInfo->sceneComposition.mainSceneMSAA);
    R_SetViewportStruct(&v9, &viewInfo->cullViewInfo.sceneViewport);
    R_SetWindShaderConstants(&v9);

    memset(v9.input.consts[114], 0, sizeof(v9.input.consts[114]));
    R_DirtyCodeConstant(&v9, CONST_SRC_CODE_DESTRUCTIBLE_PARMS);


    v9.input.consts[115][0] = CloudArea[0] - viewInfo->cullViewInfo.viewParms.origin[0];
    v9.input.consts[115][1] = CloudArea[1] - viewInfo->cullViewInfo.viewParms.origin[1];
    v9.input.consts[115][2] = CloudArea[2];
    v9.input.consts[115][3] = CloudArea[3];
    R_DirtyCodeConstant(&v9, CONST_SRC_CODE_CLOUD_WORLD_AREA);

    v9.input.consts[44][0] = 1.0f;
    v9.input.consts[44][1] = 0.0f;
    v9.input.consts[44][2] = 0.0f;
    v9.input.consts[44][3] = 0.0f;
    R_DirtyCodeConstant(&v9, CONST_SRC_CODE_COLOR_MATRIX_R);

    v9.input.consts[45][0] = 0.0f;
    v9.input.consts[45][1] = 1.0f;
    v9.input.consts[45][2] = 0.0f;
    v9.input.consts[45][3] = 0.0f;
    R_DirtyCodeConstant(&v9, CONST_SRC_CODE_COLOR_MATRIX_G);

    v9.input.consts[46][0] = 0.0f;
    v9.input.consts[46][1] = 0.0f;
    v9.input.consts[46][2] = 1.0f;
    v9.input.consts[46][3] = 0.0f;
    R_DirtyCodeConstant(&v9, CONST_SRC_CODE_COLOR_MATRIX_B);

    R_DrawCall(callback, viewInfo, &v9, viewInfo, info, &viewInfo->cullViewInfo.viewParms, cmdBuf, 0);
}

void __cdecl RB_DebugShaderDrawCommands(const GfxViewInfo *viewInfo)
{
    const GfxBackEndData *data; // [esp+8h] [ebp-8h]
    GfxCmdBuf cmdBuf; // [esp+Ch] [ebp-4h] BYREF

    data = backEndData;
    R_SetAndClearSceneTarget(0, viewInfo);
    R_InitContext(data, &cmdBuf);
    R_DrawDebugShader(viewInfo, &cmdBuf);
    RB_EndSceneRendering(gfxCmdBufContext, &viewInfo->input, viewInfo);
}

void __cdecl R_DrawDebugShader(const GfxViewInfo *viewInfo, GfxCmdBuf *cmdBuf)
{
    int savedregs; // [esp+0h] [ebp+0h] BYREF

    R_DrawFullbrightOrDebugShader(
        R_DrawDebugShaderLitCallback,
        viewInfo,
        viewInfo->drawList,
        cmdBuf);
    R_DrawFullbrightOrDebugShader(
        R_DrawDebugShaderDecalCallback,
        viewInfo,
        &viewInfo->drawList[3],
        cmdBuf);
    R_DrawFullbrightOrDebugShader(
        R_DrawDebugShaderEmissiveCallback,
        viewInfo,
        &viewInfo->drawList[4],
        cmdBuf);
}

void __cdecl R_DrawDebugShaderLitCallback(const void *userData, GfxCmdBufContext context, GfxCmdBufContext prepassContext)
{
    const GfxViewInfo *viewInfo = (const GfxViewInfo *)userData;

    R_SetRenderTarget(context, R_RENDERTARGET_SCENE);
    R_HW_EnableScissor(
        context.state->prim.device,
        viewInfo->cullViewInfo.scissorViewport.x,
        viewInfo->cullViewInfo.scissorViewport.y,
        viewInfo->cullViewInfo.scissorViewport.width,
        viewInfo->cullViewInfo.scissorViewport.height);
    R_DrawSurfs(context, 0, viewInfo->drawList);
    R_HW_DisableScissor(context.state->prim.device);
}

void __cdecl R_DrawDebugShaderDecalCallback(const void *userData, GfxCmdBufContext context, GfxCmdBufContext prepassContext)
{
    const GfxViewInfo *viewInfo = (const GfxViewInfo *)userData;

    R_SetRenderTarget(context, R_RENDERTARGET_SCENE);
    R_DrawSurfs(context, 0, &viewInfo->drawList[3]);
}

void __cdecl R_DrawDebugShaderEmissiveCallback(const void *userData, GfxCmdBufContext context, GfxCmdBufContext prepassContext)
{
    const GfxViewInfo *viewInfo = (const GfxViewInfo *)userData;

    R_SetRenderTarget(context, R_RENDERTARGET_SCENE);
    R_HW_EnableScissor(
        context.state->prim.device,
        viewInfo->cullViewInfo.scissorViewport.x,
        viewInfo->cullViewInfo.scissorViewport.y,
        viewInfo->cullViewInfo.scissorViewport.width,
        viewInfo->cullViewInfo.scissorViewport.height);
    R_DrawSurfs(context, 0, &viewInfo->drawList[4]);
    R_ShowTris(context, viewInfo->drawList);
    R_ShowTris(context, &viewInfo->drawList[1]);
    R_ShowTris(context, &viewInfo->drawList[3]);
    R_ShowTris(context, &viewInfo->drawList[4]);
    R_HW_DisableScissor(context.state->prim.device);
}

void __cdecl RB_StandardDrawCommands(GfxViewInfo *viewInfo)
{
  GfxRenderTargetId depthPrepassRT; // [esp+7Fh] [ebp-51h]
  float v2[4]; // [esp+80h] [ebp-50h] BYREF
  GfxRenderTargetId setupRenderTargetId; // [esp+91h] [ebp-3Fh]
  bool drawWaypoints; // [esp+93h] [ebp-3Dh]
  const GfxBackEndData *data; // [esp+94h] [ebp-3Ch]
  GfxCmdBuf cmdBuf; // [esp+98h] [ebp-38h] BYREF
  bool needsDepthPrepass; // [esp+9Ch] [ebp-34h]
  GfxRenderTargetId floatzRenderTarget; // [esp+9Dh] [ebp-33h]
  bool applyPostFX; // [esp+9Eh] [ebp-32h]
  bool resolveDistortion; // [esp+9Fh] [ebp-31h]
  ShadowType dynamicShadowType; // [esp+A0h] [ebp-30h]
  int isRenderingFullScreen; // [esp+A4h] [ebp-2Ch]
  float clearColor[4]; // [esp+A8h] [ebp-28h] BYREF
  GfxViewport oldSceneViewport; // [esp+B8h] [ebp-18h]
  unsigned __int8 whichToClearForScene; // [esp+CBh] [ebp-5h]
  bool isMissileCam; // [esp+CCh] [ebp-4h]
  unsigned __int8 whichToClearForSetup; // [esp+CDh] [ebp-3h]
  bool drawReflected; // [esp+CEh] [ebp-2h]
  bool need_float_z; // [esp+CFh] [ebp-1h]

  PROF_SCOPED("RB_StandardDrawCommands");

  data = backEndData;
  isMissileCam = viewInfo->isMissileCamera;
  floatzRenderTarget = isMissileCam ? R_RENDERTARGET_FLOAT_Z_MISSILE_CAM : R_RENDERTARGET_FLOAT_Z;
  resolveDistortion = 1;
  drawReflected = !isMissileCam;
  drawWaypoints = drawReflected;
  applyPostFX = drawReflected;
  if ( !isMissileCam )
    RB_UI3D_RenderToTexture(viewInfo->cmds, &viewInfo->rbUI3D, &viewInfo->input);
  dynamicShadowType = viewInfo->dynamicShadowType;
  isRenderingFullScreen = viewInfo->isRenderingFullScreen;
  oldSceneViewport = viewInfo->sceneViewport;
  if ( dynamicShadowType == SHADOW_MAP )
  {
    if ( Com_BitCheckAssert(data->shadowableLightHasShadowMap, rgp.world->sunPrimaryLightIndex, 32) )
      RB_SunShadowMaps(data, viewInfo);
    RB_SpotShadowMaps(data, viewInfo);
  }
  needsDepthPrepass = !isMissileCam;
  whichToClearForScene = 7;
  R_InitContext(data, &cmdBuf);
  if ( viewInfo->needsFloatZ
    && !gfxRenderTargets[viewInfo->isMissileCamera ? R_RENDERTARGET_FLOAT_Z_MISSILE_CAM : R_RENDERTARGET_FLOAT_Z].surface.color
    && !Assert_MyHandler(
          "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_draw3d.cpp",
          7133,
          0,
          "%s",
          "!viewInfo->needsFloatZ || R_HaveFloatZ(viewInfo->isMissileCamera)") )
  {
    __debugbreak();
  }
  need_float_z = viewInfo->needsFloatZ;
  viewInfo->needsFloatZ = 1;
  setupRenderTargetId = floatzRenderTarget;
  whichToClearForSetup = 7;
  if ( !dx.supportsIntZ || isMissileCam )
  {
    D3DPERF_BeginEvent(-1, L"Float Z");
    R_InitLocalCmdBufState(&gfxCmdBufState);
    R_SetRenderTargetSize(&gfxCmdBufSourceState, setupRenderTargetId);
    R_SetRenderTarget(gfxCmdBufContext, setupRenderTargetId);
    R_ClearScreen(gfxCmdBufState.prim.device, whichToClearForSetup, colorWhite, 1.0, 0, 0);
    memcpy(gfxCmdBufState.refSamplerState, gfxCmdBufState.refSamplerState, sizeof(gfxCmdBufState));
    R_InitContext(data, &cmdBuf);
    viewInfo->renderingFloatZ = 1;
    R_DepthPrepass(setupRenderTargetId, viewInfo, &cmdBuf);
    viewInfo->renderingFloatZ = 0;
  }
  if ( viewInfo->isMissileCamera )
  {
    R_InitLocalCmdBufState(&gfxCmdBufState);
    PROF_SCOPED("Clear ExtraCam Render Target");
    R_SetRenderTargetSize(&gfxCmdBufSourceState, R_RENDERTARGET_MISSILE_CAM);
    R_SetRenderTarget(gfxCmdBufContext, R_RENDERTARGET_MISSILE_CAM);
    memset(v2, 0, 12);
    v2[3] = 1.0f;
    R_ClearScreen(gfxCmdBufState.prim.device, 7u, v2, 1.0, 0, 0);
  }
  else
  {
    R_InitLocalCmdBufState(&gfxCmdBufState);
    R_SetRenderTargetSize(&gfxCmdBufSourceState, viewInfo->sceneComposition.mainSceneMSAA);
    R_SetRenderTarget(gfxCmdBufContext, viewInfo->sceneComposition.mainSceneMSAA);
    if ( R_GetClearColor(clearColor) || (whichToClearForScene & 0xFE) != 0 )
      R_ClearScreen(gfxCmdBufState.prim.device, 7u, clearColor, 1.0, 0, 0);
  }
  memcpy(gfxCmdBufState.refSamplerState, gfxCmdBufState.refSamplerState, sizeof(gfxCmdBufState));
  //if ( needsDepthPrepass )
  // LWSS: wire up this dvar, it's in the game but removed. 
  // It appears at one point that this was optional but it is now mandatory for some rendering (trees on `mp_mountain`)
  if ( needsDepthPrepass && r_depthPrepass->current.enabled ) 
  {
    PROF_SCOPED("R_DepthPrepass");
    R_InitContext(data, &cmdBuf);
    depthPrepassRT = dx.supportsSceneNullRenderTarget ? R_RENDERTARGET_SCENE_NULLCOLOR : R_RENDERTARGET_SCENE;
    R_SetRenderTargetSize(&gfxCmdBufSourceState, depthPrepassRT);
    R_SetRenderTarget(gfxCmdBufContext, depthPrepassRT);
    R_DepthPrepass(viewInfo->sceneComposition.mainSceneMSAA, viewInfo, &cmdBuf);
  }
  if ( viewInfo->isMissileCamera )
  {
    R_SetRenderTargetSize(&gfxCmdBufSourceState, R_RENDERTARGET_MISSILE_CAM);
    R_SetRenderTarget(gfxCmdBufContext, R_RENDERTARGET_MISSILE_CAM);
  }
  else if ( dx.supportsSceneNullRenderTarget )
  {
    R_SetRenderTargetSize(&gfxCmdBufSourceState, R_RENDERTARGET_SCENE);
    R_SetRenderTarget(gfxCmdBufContext, R_RENDERTARGET_SCENE);
  }
  viewInfo->needsFloatZ = need_float_z;
  if ( viewInfo->drawList[6].drawSurfCount )
  {
    R_InitContext(data, &cmdBuf);
    
    PROF_SCOPED("CloakPrePass");
    R_DrawCloakHDR(viewInfo, &cmdBuf, CLOAK_PHASE_PREPASS);
  }
  R_InitContext(data, &cmdBuf);

  {
      PROF_SCOPED("Lit");
      R_DrawLit(viewInfo, &cmdBuf, 0, LIT_PHASE_MAIN);
  }
  
  if ( dx.supportsIntZ && !viewInfo->isMissileCamera )
  {
    PROF_SCOPED("Resolve IntZ");
    R_InitCmdBufSourceState(&gfxCmdBufSourceState, &viewInfo->input, 0);
    R_InitLocalCmdBufState(&gfxCmdBufState);
    R_SetRenderTargetSize(gfxCmdBufContext.source, viewInfo->sceneComposition.mainSceneMSAA);
    R_SetRenderTarget(gfxCmdBufContext, viewInfo->sceneComposition.mainSceneMSAA);
    R_ResolveIntZ_PC();
    memcpy(gfxCmdBufState.refSamplerState, gfxCmdBufState.refSamplerState, sizeof(gfxCmdBufState));
  }
  if ( (viewInfo->sceneComposition.renderingMode & 7) == 0 )
    R_ResolveDistortion(viewInfo);
  if ( drawReflected )
  {
    R_InitContext(data, &cmdBuf);
    PROF_SCOPED("Reflected");
    R_DrawReflected(viewInfo, &cmdBuf);
  }

  {
      PROF_SCOPED("LitPostResolve");
      R_InitContext(data, &cmdBuf);
      R_DrawLit(viewInfo, &cmdBuf, 0, LIT_PHASE_POST_RESOLVE);
  }
  
  if ( r_setFrameBufferAlpha->current.enabled )
    RB_SetFrameBufferAlpha();
  R_InitContext(data, &cmdBuf);
  R_DrawDecal(viewInfo, &cmdBuf, 0);
  if ( viewInfo->isRenderingFullScreen != isRenderingFullScreen
    && !Assert_MyHandler(
          "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_draw3d.cpp",
          7334,
          0,
          "%s",
          "viewInfo->isRenderingFullScreen == isRenderingFullScreen") )
  {
    __debugbreak();
  }
  R_InitCmdBufSourceState(&gfxCmdBufSourceState, &viewInfo->input, 0);
  R_InitLocalCmdBufState(&gfxCmdBufState);
  R_SetRenderTargetSize(&gfxCmdBufSourceState, viewInfo->sceneComposition.mainSceneMSAA);
  R_SetRenderTarget(gfxCmdBufContext, viewInfo->sceneComposition.mainSceneMSAA);
  R_BeginView(&gfxCmdBufSourceState, &viewInfo->sceneDef, &viewInfo->cullViewInfo.viewParms);
  R_SetViewportStruct(&gfxCmdBufSourceState, &viewInfo->cullViewInfo.sceneViewport);
  RB_DrawSun(viewInfo->localClientNum);
  if ( RB_ShouldDrawCoronas() )
    RB_DrawCoronas(viewInfo->localClientNum);
  if ( !isMissileCam )
    RB_DrawSuperFlareOccluders(viewInfo);
  if ( dx.supportsIntZ && !viewInfo->isMissileCamera )
  {
      PROF_SCOPED("Resolve IntZ");
    R_InitCmdBufSourceState(&gfxCmdBufSourceState, &viewInfo->input, 0);
    R_InitLocalCmdBufState(&gfxCmdBufState);
    R_SetRenderTargetSize(gfxCmdBufContext.source, viewInfo->sceneComposition.mainSceneMSAA);
    R_SetRenderTarget(gfxCmdBufContext, viewInfo->sceneComposition.mainSceneMSAA);
    R_ResolveIntZ_PC();
    memcpy(gfxCmdBufState.refSamplerState, gfxCmdBufState.refSamplerState, sizeof(gfxCmdBufState));
  }
  memcpy(gfxCmdBufState.refSamplerState, gfxCmdBufState.refSamplerState, sizeof(gfxCmdBufState));
  if ( r_enableDlights->current.enabled )
  {
    R_InitContext(data, &cmdBuf);
    PROF_SCOPED("Dynamic Lights");
    R_DrawLights(viewInfo, &cmdBuf);
  }
  R_ResolveDistortion(viewInfo);
  R_InitContext(data, &cmdBuf);

  {
      PROF_SCOPED("Emissive");
      R_DrawEmissive(viewInfo, &cmdBuf);
  }
  
  if ( drawWaypoints )
  {
    R_InitCmdBufSourceState(&gfxCmdBufSourceState, &viewInfo->input, 0);
    R_InitLocalCmdBufState(&gfxCmdBufState);
    R_SetRenderTargetSize(&gfxCmdBufSourceState, viewInfo->sceneComposition.mainSceneMSAA);
    R_SetRenderTarget(gfxCmdBufContext, viewInfo->sceneComposition.mainSceneMSAA);
    R_BeginView(&gfxCmdBufSourceState, &viewInfo->sceneDef, &viewInfo->cullViewInfo.viewParms);
    R_SetViewportStruct(&gfxCmdBufSourceState, &viewInfo->cullViewInfo.sceneViewport);
    RB_DrawWaypoints(viewInfo->localClientNum);
    memcpy(gfxCmdBufState.refSamplerState, gfxCmdBufState.refSamplerState, sizeof(gfxCmdBufState));
  }
  if ( viewInfo->postEmissiveBrightening > 0.0 )
  {
      PROF_SCOPED("postEmissiveBrightening");
    R_InitLocalCmdBufState(&gfxCmdBufState);
    R_SetRenderTargetSize(&gfxCmdBufSourceState, viewInfo->sceneComposition.mainSceneMSAA);
    R_SetRenderTarget(gfxCmdBufContext, viewInfo->sceneComposition.mainSceneMSAA);
    R_SetViewportStruct(&gfxCmdBufSourceState, &viewInfo->cullViewInfo.sceneViewport);
    RB_DrawFullScreenColoredQuad(
      rgp.blendMaterialNoDepth,
      0.0,
      0.0,
      1.0,
      1.0,
      (unsigned __int8)(int)(float)((float)(viewInfo->postEmissiveBrightening * 255.0) + 0.5));
    memcpy(gfxCmdBufState.refSamplerState, gfxCmdBufState.refSamplerState, sizeof(gfxCmdBufState));
  }
  if ( viewInfo->drawList[6].drawSurfCount )
  {
    if ( (viewInfo->sceneComposition.renderingMode & 7) == 0 )
        R_Resolve(gfxCmdBufContext, gfxRenderTargets[R_RENDERTARGET_RESOLVED_SCENE].image);
    R_InitContext(data, &cmdBuf);

    PROF_SCOPED("CloakPostEmissive");
    R_DrawCloakHDR(viewInfo, &cmdBuf, CLOAK_PHASE_CLOAKED);
  }
  if ( !isMissileCam )
    RB_EndSceneRendering(gfxCmdBufContext, &viewInfo->input, viewInfo);
  if ( applyPostFX )
    RB_StandardPostEffects(viewInfo);
  if ( !isRenderingFullScreen && !isMissileCam )
    viewInfo->cullViewInfo.sceneViewport = oldSceneViewport;
}

void RB_SetFrameBufferAlpha()
{
  if ( tess.indexCount )
    RB_EndTessSurface();
  gfxCmdBufSourceState.viewportBehavior = GFX_USE_VIEWPORT_FULL;
  R_Set2D(&gfxCmdBufSourceState);
  R_InitLocalCmdBufState(&gfxCmdBufState);
  gfxCmdBufSourceState.viewParms.projectionMatrix.m[2][2] = 1.0f;
  gfxCmdBufSourceState.viewParms.viewProjectionMatrix.m[2][2] = 1.0f;
  RB_DrawStretchPicZ(
    rgp.setAlphaMaterial,
    0.0,
    0.0,
    -1.0,
    (float)gfxCmdBufSourceState.renderTargetWidth,
    (float)gfxCmdBufSourceState.renderTargetHeight,
    0.0,
    0.0,
    1.0,
    1.0,
    0xFFFFFFFF,
    GFX_PRIM_STATS_CODE);
  RB_DrawStretchPicZ(
    rgp.setAlphaMaterial,
    0.0,
    0.0,
    0.0,
    (float)gfxCmdBufSourceState.renderTargetWidth,
    (float)gfxCmdBufSourceState.renderTargetHeight,
    0.0,
    0.0,
    1.0,
    1.0,
    0xFFFFFFu,
    GFX_PRIM_STATS_CODE);
  RB_EndTessSurface();
  memcpy(gfxCmdBufState.refSamplerState, gfxCmdBufState.refSamplerState, sizeof(gfxCmdBufState));
}

void    R_DrawLights(const GfxViewInfo *viewInfo, GfxCmdBuf *cmdBuf)
{
    GfxCmdBufSourceState v6; // [esp-1AA0h] [ebp-1AACh] BYREF

    R_InitCmdBufSourceState(&v6, &viewInfo->input, 1);
    R_SetRenderTargetSize(&v6, viewInfo->sceneComposition.mainSceneMSAA);
    R_SetViewportStruct(&v6, &viewInfo->cullViewInfo.sceneViewport);
    R_SetWindShaderConstants(&v6);
    //memset(&v6.gap0[1824], 0, 16);
    memset(v6.input.consts[114], 0, sizeof(v6.input.consts[114]));
    R_DirtyCodeConstant(&v6, CONST_SRC_CODE_DESTRUCTIBLE_PARMS);
    v6.input.consts[68][0] = r_skyTransition->current.value;
    v6.input.consts[68][1] = 0.0f;
    v6.input.consts[68][2] = (float)r_treeScale->current.integer;
    v6.input.consts[68][3] = r_testScale->current.value;
    //*(float *)&v5 = (float)r_treeScale->current.integer;
    //integer = r_testScale->current.integer;
    //LODWORD(v6.input.consts[68][0]) = r_skyTransition->current.integer;
    //*(_QWORD *)&v6.gap0[1092] = __PAIR64__(v5, 0);
    //LODWORD(v6.input.consts[68][3]) = integer;

    R_DirtyCodeConstant(&v6, CONST_SRC_CODE_SKY_TRANSITION);
    R_DrawPointLitSurfs(&v6, viewInfo, cmdBuf);
}

void __cdecl R_DrawPointLitSurfs(GfxCmdBufSourceState *source, const GfxViewInfo *viewInfo, GfxCmdBuf *cmdBuf)
{
    float v3; // [esp+24h] [ebp-144h]
    float v4; // [esp+28h] [ebp-140h]
    float v5; // [esp+2Ch] [ebp-13Ch]
    float v6; // [esp+30h] [ebp-138h]
    float v7; // [esp+34h] [ebp-134h]
    float v8; // [esp+4Ch] [ebp-11Ch]
    float radius; // [esp+50h] [ebp-118h]
    float edgePoint; // [esp+88h] [ebp-E0h]
    float edgePoint_4; // [esp+8Ch] [ebp-DCh]
    float edgePoint_8; // [esp+90h] [ebp-D8h]
    GfxPointLitSurfsInfo info; // [esp+9Ch] [ebp-CCh] BYREF
    int pointLightCount; // [esp+B8h] [ebp-B0h]
    const GfxBackEndData *data; // [esp+BCh] [ebp-ACh]
    float perp[3]; // [esp+C0h] [ebp-A8h] BYREF
    float dist[4]; // [esp+CCh] [ebp-9Ch]
    float halfWidth; // [esp+DCh] [ebp-8Ch]
    float width; // [esp+E0h] [ebp-88h]
    float height; // [esp+E4h] [ebp-84h]
    const GfxLight *light; // [esp+E8h] [ebp-80h]
    float sign; // [esp+ECh] [ebp-7Ch]
    const PointLightPartition *pointLightPartition; // [esp+F0h] [ebp-78h]
    float offset[3]; // [esp+F4h] [ebp-74h] BYREF
    float edgeGoalPoint[3]; // [esp+100h] [ebp-68h]
    float tangentDist; // [esp+10Ch] [ebp-5Ch]
    float edgeDir[3]; // [esp+110h] [ebp-58h] BYREF
    float planeDist; // [esp+11Ch] [ebp-4Ch]
    int partitionIndex; // [esp+120h] [ebp-48h]
    const GfxViewParms *viewParms; // [esp+124h] [ebp-44h]
    float tangentDistSq; // [esp+128h] [ebp-40h]
    GfxColor color; // [esp+12Ch] [ebp-3Ch] BYREF
    float x; // [esp+130h] [ebp-38h]
    float y; // [esp+134h] [ebp-34h]
    unsigned int axis; // [esp+138h] [ebp-30h]
    float offsetDistSq; // [esp+13Ch] [ebp-2Ch]
    GfxDrawPrimArgs args; // [esp+140h] [ebp-28h]
    unsigned int planeIndex; // [esp+14Ch] [ebp-1Ch]
    float halfHeight; // [esp+150h] [ebp-18h]
    float perpDir[3]; // [esp+154h] [ebp-14h] BYREF
    float perpDist; // [esp+160h] [ebp-8h]
    float w; // [esp+164h] [ebp-4h]
    int savedregs; // [esp+168h] [ebp+0h] BYREF

    if (!viewInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_draw3d.cpp", 4872, 0, "%s", "viewInfo"))
    {
        __debugbreak();
    }
    pointLightCount = viewInfo->pointLightCount;
    if (pointLightCount)
    {
        R_ConvertColorToBytes(colorWhite, (unsigned __int8 *)&color);
        data = source->input.data;
        args.baseIndex = 0;
        args.vertexCount = 4;
        args.triCount = 2;
        viewParms = &viewInfo->cullViewInfo.viewParms;
        info.viewInfo = viewInfo;
        for (partitionIndex = 0; partitionIndex < pointLightCount; ++partitionIndex)
        {
            pointLightPartition = &viewInfo->pointLightPartitions[partitionIndex];
            light = &pointLightPartition->light;
            info.drawSurfInfo = &pointLightPartition->info;
            info.clearQuadMesh = (GfxMeshData*)&viewInfo->pointLightMeshData[partitionIndex];
            dist[0] = 1.0f;
            dist[1] = -1.0f;
            dist[2] = 1.0f;
            dist[3] = -1.0f;
            offset[0] = pointLightPartition->light.origin[0] - viewParms->origin[0];
            offset[1] = pointLightPartition->light.origin[1] - viewParms->origin[1];
            offset[2] = pointLightPartition->light.origin[2] - viewParms->origin[2];
            offsetDistSq = (float)((float)(offset[0] * offset[0]) + (float)(offset[1] * offset[1]))
                + (float)(offset[2] * offset[2]);
            tangentDistSq = offsetDistSq - (float)(pointLightPartition->light.radius * pointLightPartition->light.radius);
            if (tangentDistSq > 1.0)
            {
                tangentDist = sqrtf(tangentDistSq);
                perpDist = offsetDistSq / tangentDist;
                sign = 1.0f;
                planeIndex = 0;
                while (planeIndex < 4)
                {
                    axis = planeIndex >> 1;
                    perp[0] = sign * viewParms->axis[2 - (planeIndex >> 1)][0];
                    perp[1] = sign * viewParms->axis[2 - (planeIndex >> 1)][1];
                    perp[2] = sign * viewParms->axis[2 - (planeIndex >> 1)][2];
                    Vec3Cross(perp, offset, perpDir);
                    Vec3Normalize(perpDir);
                    edgeGoalPoint[0] = (float)((-(perpDist)) * perpDir[0]) + viewParms->origin[0];
                    edgeGoalPoint[1] = (float)((-(perpDist)) * perpDir[1]) + viewParms->origin[1];
                    edgeGoalPoint[2] = (float)((-(perpDist)) * perpDir[2]) + viewParms->origin[2];
                    edgeDir[0] = edgeGoalPoint[0] - light->origin[0];
                    edgeDir[1] = edgeGoalPoint[1] - light->origin[1];
                    edgeDir[2] = edgeGoalPoint[2] - light->origin[2];
                    Vec3Normalize(edgeDir);
                    radius = light->radius;
                    edgePoint = (float)(radius * edgeDir[0]) + light->origin[0];
                    edgePoint_4 = (float)(radius * edgeDir[1]) + light->origin[1];
                    edgePoint_8 = (float)(radius * edgeDir[2]) + light->origin[2];
                    if ((float)((float)((float)((float)(viewInfo->frustumPlanes[planeIndex][0] * edgePoint)
                        + (float)(viewInfo->frustumPlanes[planeIndex][1] * edgePoint_4))
                        + (float)(viewInfo->frustumPlanes[planeIndex][2] * edgePoint_8))
                        + viewInfo->frustumPlanes[planeIndex][3]) > 0.0)
                    {
                        w = (float)((float)((float)(edgePoint * viewParms->viewProjectionMatrix.m[0][3])
                            + (float)(edgePoint_4 * viewParms->viewProjectionMatrix.m[1][3]))
                            + (float)(edgePoint_8 * viewParms->viewProjectionMatrix.m[2][3]))
                            + viewParms->viewProjectionMatrix.m[3][3];
                        if (w > 0.0)
                        {
                            planeDist = (float)((float)((float)(edgePoint * viewParms->viewProjectionMatrix.m[0][axis])
                                + (float)(edgePoint_4 * viewParms->viewProjectionMatrix.m[1][axis]))
                                + (float)(edgePoint_8 * viewParms->viewProjectionMatrix.m[2][axis]))
                                + viewParms->viewProjectionMatrix.m[3][axis];
                            if ((float)((float)(planeDist / w) - 1.0) < 0.0)
                                v8 = planeDist / w;
                            else
                                v8 = 1.0f;
                            if ((float)(-1.0 - (float)(planeDist / w)) < 0.0)
                                v7 = v8;
                            else
                                v7 = -1.0f;
                            dist[planeIndex] = v7;
                        }
                    }
                    ++planeIndex;
                    //LODWORD(sign) ^= _mask__NegFloat_;
                    sign = -sign;
                }
            }
            halfWidth = (float)(viewInfo->cullViewInfo.sceneViewport.width >> 1);
            halfHeight = (float)(viewInfo->cullViewInfo.sceneViewport.height >> 1);
            x = (float)(dist[1] + 1.0) * halfWidth;
            y = (float)(1.0 - dist[2]) * halfHeight;
            width = (float)(dist[0] - dist[1]) * halfWidth;
            height = (float)(dist[2] - dist[3]) * halfHeight;
            v6 = floor(x);
            info.x = (int)v6;
            v5 = floor(y);
            info.y = (int)v5;
            v4 = ceil((float)((float)(dist[0] + 1.0) * halfWidth));
            info.w = (int)v4 - info.x;
            v3 = ceil((float)((float)(1.0 - dist[3]) * halfHeight));
            info.h = (int)v3 - info.y;
            info.x += viewInfo->cullViewInfo.sceneViewport.x;
            info.y += viewInfo->cullViewInfo.sceneViewport.y;
            R_SetQuadMeshData(info.clearQuadMesh, x, y, width, height, 0.0, 0.0, 1.0, 1.0, 0xFFFFFFFF);
            R_DrawCall(
                R_DrawPointLitSurfsCallback,
                &info,
                source,
                viewInfo,
                &pointLightPartition->info,
                &viewInfo->cullViewInfo.viewParms,
                cmdBuf,
                0);
        }
    }
}

void __cdecl R_DrawPointLitSurfsCallback(const void *userData, GfxCmdBufContext context, GfxCmdBufContext prepassContext)
{
    GfxPointLitSurfsInfo *info = (GfxPointLitSurfsInfo *)userData;

    R_SetRenderTarget(context, info->viewInfo->sceneComposition.mainSceneMSAA);
    R_Set2D(context.source);
    R_DrawQuadMesh(context, rgp.clearAlphaMaterial, info->clearQuadMesh);
    R_Set3D(context.source);
    if ((info->viewInfo->sceneComposition.renderingMode & 7) == 0)
        R_HW_EnableScissor(context.state->prim.device, info->x, info->y, info->w, info->h);
    R_DrawSurfs(context, 0, info->drawSurfInfo);
    if ((info->viewInfo->sceneComposition.renderingMode & 7) == 0)
        R_HW_DisableScissor(context.state->prim.device);
}

void __cdecl R_ResolveDistortion(const GfxViewInfo *viewInfo)
{
  GfxRenderTargetId resolveDst; // [esp+0h] [ebp-18h]
  GfxRenderTargetId resolveSrc; // [esp+17h] [ebp-1h]

  PROF_SCOPED("Resolve Distortion");

  resolveSrc = viewInfo->sceneComposition.mainSceneMSAA;
  if ( resolveSrc == R_RENDERTARGET_SCENE )
    resolveDst = R_RENDERTARGET_RESOLVED_POST_SUN;
  else
    resolveDst = viewInfo->sceneComposition.mainSceneMSAA;

  if ( resolveSrc == resolveDst )
  {
      return;
  }
  else
  {
    R_SetRenderTargetSize(&gfxCmdBufSourceState, resolveSrc);
    R_SetRenderTarget(gfxCmdBufContext, resolveSrc);
    R_Resolve(gfxCmdBufContext, gfxRenderTargets[resolveDst].image);
  }
}

void __cdecl RB_StandardPostEffects(GfxViewInfo *viewInfo)
{
  const GfxBackEndData *data; // [esp+Ch] [ebp-4h]

  data = backEndData;

  PROF_SCOPED("RB_StandardPostEffects");

  viewInfo->input.data = data;
  R_InitCmdBufSourceState(&gfxCmdBufSourceState, &viewInfo->input, 0);
  R_InitLocalCmdBufState(&gfxCmdBufState);
  R_SetResolvedScene(gfxCmdBufContext);
  R_BeginView(&gfxCmdBufSourceState, &viewInfo->sceneDef, &viewInfo->cullViewInfo.viewParms);
  R_SetViewportStruct(&gfxCmdBufSourceState, &viewInfo->cullViewInfo.sceneViewport);
  RB_ApplyLatePostEffects(viewInfo);
  R_SetRenderTargetSize(&gfxCmdBufSourceState, R_RENDERTARGET_SCENE);
  R_SetRenderTarget(gfxCmdBufContext, R_RENDERTARGET_SCENE);
  RB_DrawSunPostEffects(viewInfo->localClientNum, viewInfo->sunVisibility);
  memcpy(gfxCmdBufState.refSamplerState, gfxCmdBufState.refSamplerState, sizeof(gfxCmdBufState));
}

void __cdecl R_SetResolvedScene(GfxCmdBufContext context)
{
    R_SetRenderTargetSize(context.source, R_RENDERTARGET_SCENE);
    R_SetRenderTarget(context, R_RENDERTARGET_SCENE);
}

void __cdecl RB_ApplyLatePostEffects(const GfxViewInfo *viewInfo)
{
    PROF_SCOPED("RB_ApplyLatePostEffects");
    RB_ProcessPostEffects(viewInfo);
    R_SetRenderTargetSize(&gfxCmdBufSourceState, R_RENDERTARGET_SCENE);
    R_SetRenderTarget(gfxCmdBufContext, R_RENDERTARGET_SCENE);
    RB_DrawDebugPostEffects();
}

void RB_DrawDebugPostEffects()
{
    if ( !r_showFbColorDebug
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_draw3d.cpp",
                    5170,
                    0,
                    "%s",
                    "r_showFbColorDebug") )
    {
        __debugbreak();
    }
    if ( !r_showFloatZDebug
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_draw3d.cpp",
                    5171,
                    0,
                    "%s",
                    "r_showFloatZDebug") )
    {
        __debugbreak();
    }
    if ( r_showFbColorDebug->current.integer == 1 )
    {
        RB_ShowFbColorDebug_Screen();
    }
    else if ( r_showFbColorDebug->current.integer == 2 )
    {
        RB_ShowFbColorDebug_Feedback();
    }
    else if ( r_showFloatZDebug->current.enabled )
    {
        RB_ShowFloatZDebug();
    }
}

void RB_ShowFbColorDebug_Screen()
{
  float halfScreenWidth; // [esp+38h] [ebp-10h]
  float halfScreenHeight; // [esp+3Ch] [ebp-Ch]
  float quarterScreenWidth; // [esp+40h] [ebp-8h]

  if ( tess.indexCount )
    RB_EndTessSurface();
  R_Set2D(&gfxCmdBufSourceState);
  halfScreenWidth = (float)gfxCmdBufSourceState.renderTargetWidth * 0.5;
  halfScreenHeight = (float)gfxCmdBufSourceState.renderTargetHeight * 0.5;
  quarterScreenWidth = halfScreenWidth * 0.5;
  RB_DrawStretchPic(
    rgp.frameColorDebugMaterial,
    quarterScreenWidth,
    halfScreenHeight * 0.5,
    quarterScreenWidth,
    halfScreenHeight * 0.5,
    0.0,
    0.0,
    1.0,
    1.0,
    0xFFFF0000,
    GFX_PRIM_STATS_CODE);
  RB_DrawStretchPic(
    rgp.frameColorDebugMaterial,
    halfScreenWidth,
    halfScreenHeight * 0.5,
    halfScreenWidth * 0.5,
    halfScreenHeight * 0.5,
    0.0,
    0.0,
    1.0,
    1.0,
    0xFF00FF00,
    GFX_PRIM_STATS_CODE);
  RB_DrawStretchPic(
    rgp.frameColorDebugMaterial,
    quarterScreenWidth,
    halfScreenHeight,
    quarterScreenWidth,
    halfScreenHeight * 0.5,
    0.0,
    0.0,
    1.0,
    1.0,
    0xFF0000FF,
    GFX_PRIM_STATS_CODE);
  RB_DrawStretchPic(
    rgp.frameAlphaDebugMaterial,
    halfScreenWidth,
    halfScreenHeight,
    halfScreenWidth * 0.5,
    halfScreenHeight * 0.5,
    0.0,
    0.0,
    1.0,
    1.0,
    0xFFFFFFFF,
    GFX_PRIM_STATS_CODE);
  RB_EndTessSurface();
}

void RB_ShowFbColorDebug_Feedback()
{
  float quarterScreenHeight; // [esp+40h] [ebp-10h]
  float halfScreenWidth; // [esp+44h] [ebp-Ch]
  float halfScreenHeight; // [esp+48h] [ebp-8h]
  float quarterScreenWidth; // [esp+4Ch] [ebp-4h]

  if ( tess.indexCount )
    RB_EndTessSurface();
  halfScreenWidth = (float)gfxCmdBufSourceState.renderTargetWidth * 0.5;
  halfScreenHeight = (float)gfxCmdBufSourceState.renderTargetHeight * 0.5;
  quarterScreenWidth = halfScreenWidth * 0.5;
  quarterScreenHeight = halfScreenHeight * 0.5;
  R_SetRenderTargetSize(&gfxCmdBufSourceState, R_RENDERTARGET_FRAME_BUFFER);
  R_SetRenderTarget(gfxCmdBufContext, R_RENDERTARGET_FRAME_BUFFER);
  R_Set2D(&gfxCmdBufSourceState);
  gfxCmdBufSourceState.input.consts[44][0] = 1.0f;
  gfxCmdBufSourceState.input.consts[44][1] = 0.0f;
  gfxCmdBufSourceState.input.consts[44][2] = 0.0f;
  gfxCmdBufSourceState.input.consts[44][3] = 0.0f;
  R_DirtyCodeConstant(&gfxCmdBufSourceState, CONST_SRC_CODE_COLOR_MATRIX_R);
  gfxCmdBufSourceState.input.consts[45][0] = 0.0f;
  gfxCmdBufSourceState.input.consts[45][1] = 0.0f;
  gfxCmdBufSourceState.input.consts[45][2] = 0.0f;
  gfxCmdBufSourceState.input.consts[45][3] = 0.0f;
  R_DirtyCodeConstant(&gfxCmdBufSourceState, CONST_SRC_CODE_COLOR_MATRIX_G);
  gfxCmdBufSourceState.input.consts[46][0] = 0.0f;
  gfxCmdBufSourceState.input.consts[46][1] = 0.0f;
  gfxCmdBufSourceState.input.consts[46][2] = 0.0f;
  gfxCmdBufSourceState.input.consts[46][3] = 0.0f;
  R_DirtyCodeConstant(&gfxCmdBufSourceState, CONST_SRC_CODE_COLOR_MATRIX_B);
  RB_DrawStretchPic(
    rgp.colorChannelMixerMaterial,
    quarterScreenWidth,
    quarterScreenHeight,
    quarterScreenWidth,
    quarterScreenHeight,
    0.25,
    0.25,
    0.5,
    0.5,
    0xFFFFFFFF,
    GFX_PRIM_STATS_CODE);
  RB_EndTessSurface();
  gfxCmdBufSourceState.input.consts[44][0] = 0.0f;
  gfxCmdBufSourceState.input.consts[44][1] = 0.0f;
  gfxCmdBufSourceState.input.consts[44][2] = 0.0f;
  gfxCmdBufSourceState.input.consts[44][3] = 0.0f;
  R_DirtyCodeConstant(&gfxCmdBufSourceState, CONST_SRC_CODE_COLOR_MATRIX_R);
  gfxCmdBufSourceState.input.consts[45][0] = 0.0f;
  gfxCmdBufSourceState.input.consts[45][1] = 1.0f;
  gfxCmdBufSourceState.input.consts[45][2] = 0.0f;
  gfxCmdBufSourceState.input.consts[45][3] = 0.0f;
  R_DirtyCodeConstant(&gfxCmdBufSourceState, CONST_SRC_CODE_COLOR_MATRIX_G);
  gfxCmdBufSourceState.input.consts[46][0] = 0.0f;
  gfxCmdBufSourceState.input.consts[46][1] = 0.0f;
  gfxCmdBufSourceState.input.consts[46][2] = 0.0f;
  gfxCmdBufSourceState.input.consts[46][3] = 0.0f;
  R_DirtyCodeConstant(&gfxCmdBufSourceState, CONST_SRC_CODE_COLOR_MATRIX_B);
  RB_DrawStretchPic(
    rgp.colorChannelMixerMaterial,
    halfScreenWidth,
    quarterScreenHeight,
    halfScreenWidth * 0.5,
    quarterScreenHeight,
    0.5,
    0.25,
    0.75,
    0.5,
    0xFFFFFFFF,
    GFX_PRIM_STATS_CODE);
  RB_EndTessSurface();
  Vec4Set(gfxCmdBufSourceState.input.consts[44], 0.0, 0.0, 0.0, 0.0);
  R_DirtyCodeConstant(&gfxCmdBufSourceState, CONST_SRC_CODE_COLOR_MATRIX_R);
  Vec4Set(gfxCmdBufSourceState.input.consts[45], 0.0, 0.0, 0.0, 0.0);
  R_DirtyCodeConstant(&gfxCmdBufSourceState, CONST_SRC_CODE_COLOR_MATRIX_G);
  Vec4Set(gfxCmdBufSourceState.input.consts[46], 0.0, 0.0, 1.0, 0.0);
  R_DirtyCodeConstant(&gfxCmdBufSourceState, CONST_SRC_CODE_COLOR_MATRIX_B);
  RB_DrawStretchPic(
    rgp.colorChannelMixerMaterial,
    quarterScreenWidth,
    halfScreenHeight,
    quarterScreenWidth,
    halfScreenHeight * 0.5,
    0.25,
    0.5,
    0.5,
    0.75,
    0xFFFFFFFF,
    GFX_PRIM_STATS_CODE);
  RB_EndTessSurface();
  R_SetCodeConstant(&gfxCmdBufSourceState, CONST_SRC_CODE_COLOR_MATRIX_R, 0.0, 0.0, 0.0, 1.0);
  R_SetCodeConstant(&gfxCmdBufSourceState, CONST_SRC_CODE_COLOR_MATRIX_G, 0.0, 0.0, 0.0, 1.0);
  R_SetCodeConstant(&gfxCmdBufSourceState, CONST_SRC_CODE_COLOR_MATRIX_B, 0.0, 0.0, 0.0, 1.0);
  RB_DrawStretchPic(
    rgp.colorChannelMixerMaterial,
    halfScreenWidth,
    halfScreenHeight,
    halfScreenWidth * 0.5,
    halfScreenHeight * 0.5,
    0.5,
    0.5,
    0.75,
    0.75,
    0xFFFFFFFF,
    GFX_PRIM_STATS_CODE);
  RB_EndTessSurface();
}

void RB_ShowFloatZDebug()
{
    float halfScreenWidth; // [esp+2Ch] [ebp-Ch]
    float halfScreenHeight; // [esp+30h] [ebp-8h]

    if (gfxRenderTargets[R_RENDERTARGET_FLOAT_Z].surface.color)
    {
        if (tess.indexCount)
            RB_EndTessSurface();
        halfScreenWidth = (float)gfxCmdBufSourceState.renderTargetWidth * 0.5;
        halfScreenHeight = (float)gfxCmdBufSourceState.renderTargetHeight * 0.5;
        R_Set2D(&gfxCmdBufSourceState);
        RB_DrawStretchPic(
            rgp.floatZDisplayMaterial,
            halfScreenWidth * 0.5,
            halfScreenHeight * 0.5,
            halfScreenWidth,
            halfScreenHeight,
            0.25,
            0.25,
            0.75,
            0.75,
            0xFFFFFFFF,
            GFX_PRIM_STATS_CODE);
        RB_EndTessSurface();
    }
}

void __cdecl RB_Draw3DCommon()
{
    if ( !rgp.world
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_draw3d.cpp", 7786, 0, "%s", "rgp.world") )
    {
        __debugbreak();
    }
    if ( gfxDrawMethod.drawScene )
    {
        if ( gfxDrawMethod.drawScene == GFX_DRAW_SCENE_FULLBRIGHT )
        {
            RB_FullbrightDrawCommandsCommon();
        }
        else if ( gfxDrawMethod.drawScene == GFX_DRAW_SCENE_DEBUGSHADER )
        {
            RB_DebugShaderDrawCommandsCommon();
        }
        else
        {
            if ( gfxDrawMethod.drawScene != GFX_DRAW_SCENE_STANDARD
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_draw3d.cpp",
                            7816,
                            0,
                            "%s\n\t(gfxDrawMethod.drawScene) = %i",
                            "(gfxDrawMethod.drawScene == GFX_DRAW_SCENE_STANDARD)",
                            gfxDrawMethod.drawScene) )
            {
                __debugbreak();
            }
            RB_StandardDrawCommandsCommon();
        }
    }
}

void RB_FullbrightDrawCommandsCommon()
{
    const GfxBackEndData *data; // [esp+14h] [ebp-Ch]
    unsigned int viewInfoIndex; // [esp+1Ch] [ebp-4h]

    PROF_SCOPED("RB_FullbrightDrawCommandsCommon");

    data = backEndData;
    for (viewInfoIndex = 0; viewInfoIndex < data->viewInfoCount; ++viewInfoIndex)
        RB_FullbrightRenderCommands(&data->viewInfo[viewInfoIndex]);
}

void __cdecl RB_FullbrightRenderCommands(GfxViewInfo *viewInfo)
{
    const GfxBackEndData *data; // [esp+14h] [ebp-8h]

    PROF_SCOPED("RB_FullbrightRenderCommands");

    data = backEndData;
    viewInfo->input.data = backEndData;
    R_InitCmdBufSourceState(gfxCmdBufContext.source, &gfxCmdBufInput, 0);
    gfxCmdBufContext.source->input.data = data;
    R_InitLocalCmdBufState(gfxCmdBufContext.state);
    R_SetRenderTargetSize(gfxCmdBufContext.source, R_RENDERTARGET_FRAME_BUFFER);
    R_SetRenderTarget(gfxCmdBufContext, R_RENDERTARGET_FRAME_BUFFER);
    R_BeginView(gfxCmdBufContext.source, &viewInfo->sceneDef, &viewInfo->cullViewInfo.viewParms);
    R_SetViewportStruct(gfxCmdBufContext.source, &viewInfo->cullViewInfo.displayViewport);
    if ( viewInfo->cmds )
    {
        RB_SetUI3DSamplerAndConstants(gfxCmdBufContext.source, &viewInfo->rbUI3D);
        RB_ExecuteRenderCommandsLoop(viewInfo->cmds, 0);
    }
    memcpy(gfxCmdBufState.refSamplerState, gfxCmdBufContext.state->refSamplerState, sizeof(gfxCmdBufState));
}

GfxCmdBufSourceState *RB_DebugShaderDrawCommandsCommon()
{
    GfxCmdBufSourceState *result; // eax
    const GfxBackEndData *data; // [esp+8h] [ebp-Ch]
    unsigned int viewInfoIndex; // [esp+10h] [ebp-4h]

    result = gfxCmdBufContext.source;
    data = backEndData;
    for ( viewInfoIndex = 0; viewInfoIndex < data->viewInfoCount; ++viewInfoIndex )
    {
        RB_DebugShaderRenderCommands(&data->viewInfo[viewInfoIndex]);
        result = (GfxCmdBufSourceState *)(viewInfoIndex + 1);
    }
    return result;
}

void __cdecl RB_DebugShaderRenderCommands(GfxViewInfo *viewInfo)
{
    const GfxBackEndData *data; // [esp+14h] [ebp-8h]

    PROF_SCOPED("RB_DebugShaderRenderCommands");

    data = backEndData;
    viewInfo->input.data = backEndData;
    R_InitCmdBufSourceState(gfxCmdBufContext.source, &gfxCmdBufInput, 0);
    gfxCmdBufContext.source->input.data = data;
    R_InitLocalCmdBufState(gfxCmdBufContext.state);
    R_SetResolvedScene(gfxCmdBufContext);
    R_BeginView(gfxCmdBufContext.source, &viewInfo->sceneDef, &viewInfo->cullViewInfo.viewParms);
    R_SetViewportStruct(gfxCmdBufContext.source, &viewInfo->cullViewInfo.displayViewport);
    if ( viewInfo->cmds )
    {
        RB_SetUI3DSamplerAndConstants(gfxCmdBufContext.source, &viewInfo->rbUI3D);
        RB_ExecuteRenderCommandsLoop(viewInfo->cmds, 0);
    }
    memcpy(gfxCmdBufState.refSamplerState, gfxCmdBufContext.state->refSamplerState, sizeof(gfxCmdBufState));
}

void RB_StandardDrawCommandsCommon()
{
    float w; // [esp+40h] [ebp-34h]
    unsigned int viewInfoIndex; // [esp+64h] [ebp-10h]
    const GfxBackEndData *data; // [esp+6Ch] [ebp-8h]
    GfxViewInfo *viewInfo; // [esp+70h] [ebp-4h]
    GfxViewInfo *viewInfoa; // [esp+70h] [ebp-4h]

    PROF_SCOPED("RB_StandardDrawCommandsCommon");

    data = backEndData;
    if (!backEndData->viewInfoCount)
    {
        return;
    }
    for (viewInfoIndex = 0; viewInfoIndex < data->viewInfoCount; ++viewInfoIndex)
    {
        viewInfo = &data->viewInfo[viewInfoIndex];
        if (!viewInfo->isMissileCamera)
            RB_StandardRenderCommands(viewInfo);
    }
    viewInfoa = backEndData->viewInfo;
    R_InitCmdBufSourceState(&gfxCmdBufSourceState, &viewInfoa->input, 0);
    R_SetRenderTargetSize(&gfxCmdBufSourceState, R_RENDERTARGET_FRAME_BUFFER);
    R_SetRenderTarget(gfxCmdBufContext, R_RENDERTARGET_FRAME_BUFFER);
    R_BeginView(&gfxCmdBufSourceState, &viewInfoa->sceneDef, &viewInfoa->cullViewInfo.viewParms);
    R_SetViewportStruct(&gfxCmdBufSourceState, &viewInfoa->cullViewInfo.displayViewport);
    R_Set2D(&gfxCmdBufSourceState);
    R_InitLocalCmdBufState(&gfxCmdBufState);
    if (r_ui3d_debug_display->current.enabled)
    {
        R_SetCodeImageTexture(&gfxCmdBufSourceState, 8u, gfxRenderTargets[R_RENDERTARGET_UI3D].image);
        RB_DrawStretchPic(
            rgp.feedbackReplaceMaterial,
            50.0,
            40.0,
            (float)((float)gfxRenderTargets[R_RENDERTARGET_UI3D].width / (float)gfxRenderTargets[R_RENDERTARGET_UI3D].height) * 175.0,
            175.0,
            0.0,
            0.0,
            1.0,
            1.0,
            0xFFFFFFFF,
            GFX_PRIM_STATS_HUD);
        RB_EndTessSurface();
    }
    if (r_missile_cam_debug_display->current.integer)
    {
        w = R_ExtraCam_AspectRatio() * 200.0;
        R_SetCodeImageTexture(&gfxCmdBufSourceState, 8u, rgp.blackImage);
        RB_DrawStretchPic(
            rgp.feedbackReplaceMaterial,
            50.0 - 3.0,
            250.0 - 3.0,
            (float)(3.0 * 2.0) + w,
            (float)(3.0 * 2.0) + 200.0,
            0.0,
            0.0,
            1.0,
            1.0,
            0xFFFFFFFF,
            GFX_PRIM_STATS_HUD);
        RB_EndTessSurface();
        if (gfxRenderTargets[R_RENDERTARGET_MISSILE_CAM].image)
            R_SetCodeImageTexture(&gfxCmdBufSourceState, 8u, gfxRenderTargets[R_RENDERTARGET_MISSILE_CAM].image);
        RB_DrawStretchPic(
            rgp.feedbackReplaceMaterial,
            50.0,
            250.0,
            w,
            200.0,
            0.0,
            0.0,
            1.0,
            1.0,
            0xFFFFFFFF,
            GFX_PRIM_STATS_HUD);
        RB_EndTessSurface();
    }
    if (viewInfoa->dynamicShadowType == SHADOW_MAP && sm_showOverlay->current.integer)
    {
        if (sm_showOverlay->current.integer != 1
            && sm_showOverlay->current.integer != 2
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_draw3d.cpp",
                7710,
                0,
                "%s\n\t(sm_showOverlay->current.integer) = %i",
                "(sm_showOverlay->current.integer == GFX_SM_OVERLAY_SUN || sm_showOverlay->current.integer == GFX_SM_OVERLAY_SPOT)",
                sm_showOverlay->current.integer))
        {
            __debugbreak();
        }
        if (sm_showOverlay->current.integer == 1)
            RB_DrawSunShadowOverlay();
        else
            RB_DrawSpotShadowOverlay();
    }
    memcpy(gfxCmdBufState.refSamplerState, gfxCmdBufState.refSamplerState, sizeof(gfxCmdBufState));
}

void __cdecl R_SetCodeImageTexture(GfxCmdBufSourceState *source, unsigned int codeTexture, const GfxImage *image)
{
    iassert(source);
    bcassert(codeTexture, TEXTURE_SRC_CODE_COUNT);

    source->input.codeImages[codeTexture] = image;
    source->input.codeImageRenderTargetControl[codeTexture].packed = 0;
}

void __cdecl RB_StandardRenderCommands(GfxViewInfo *viewInfo)
{
  const GfxBackEndData *data; // [esp+18h] [ebp-8h]

  data = backEndData;

  PROF_SCOPED("RB_StandardRenderCommands");

  R_StartADSZScaleFrame();
  viewInfo->input.data = data;
  R_InitCmdBufSourceState(&gfxCmdBufSourceState, &gfxCmdBufInput, 0);
  gfxCmdBufSourceState.input.data = backEndData;
  R_InitLocalCmdBufState(&gfxCmdBufState);
  R_SetRenderTargetSize(&gfxCmdBufSourceState, R_RENDERTARGET_FRAME_BUFFER);
  R_SetRenderTarget(gfxCmdBufContext, R_RENDERTARGET_FRAME_BUFFER);
  if ( (viewInfo->sceneComposition.renderingMode & 7) != 0 )
  {
      if ((viewInfo->sceneComposition.renderingMode & 0x40000000) != 0)
          gfxCmdBufSourceState.input.codeImageRenderTargetControl[10].packed = (viewInfo->sceneComposition.mainSceneFinal << 24)
          | 0x800062;
      else
          gfxCmdBufSourceState.input.codeImageRenderTargetControl[10].packed = (viewInfo->sceneComposition.mainSceneSaved << 24)
          | 0x800062;
  }
  R_BeginView(&gfxCmdBufSourceState, &viewInfo->sceneDef, &viewInfo->cullViewInfo.viewParms);
  R_SetViewportStruct(&gfxCmdBufSourceState, &viewInfo->cullViewInfo.displayViewport);
  gfxCmdBufSourceState.input.consts[170][0] = 0.0f;
  gfxCmdBufSourceState.input.consts[170][1] = 0.0f;
  gfxCmdBufSourceState.input.consts[170][2] = 0.0f;
  gfxCmdBufSourceState.input.consts[170][3] = 0.0f;
  R_DirtyCodeConstant(&gfxCmdBufSourceState, CONST_SRC_CODE_CINEMATIC_BLUR_BOX);
  gfxCmdBufSourceState.input.consts[171][0] = 0.0f;
  gfxCmdBufSourceState.input.consts[171][1] = 0.0f;
  gfxCmdBufSourceState.input.consts[171][2] = 0.0f;
  gfxCmdBufSourceState.input.consts[171][3] = 0.0f;
  R_DirtyCodeConstant(&gfxCmdBufSourceState, CONST_SRC_CODE_CINEMATIC_BLUR_BOX2);
  if ( viewInfo->cmds )
  {
    RB_SetUI3DSamplerAndConstants(&gfxCmdBufSourceState, &viewInfo->rbUI3D);
    RB_ExecuteRenderCommandsLoop(viewInfo->cmds, 0);
  }
  memcpy(gfxCmdBufState.refSamplerState, gfxCmdBufState.refSamplerState, sizeof(gfxCmdBufState));
}

