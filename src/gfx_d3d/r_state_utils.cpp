#include "r_state_utils.h"
#include "rb_state.h"
#include <universal/assertive.h>
#include "r_state.h"
#include "r_dvars.h"
#include "rb_logfile.h"
#include "r_init.h"
#include "r_draw_material.h"
#include "r_foliage.h"
#include "r_material_load_obj.h"

void __cdecl R_InitCmdBufSourceState(GfxCmdBufSourceState *source, const GfxCmdBufInput *input, int cameraView)
{
    float *v3; // eax
    unsigned int constant; // [esp+Ch] [ebp-4h]

    if (!source
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state_utils.cpp", 33, 0, "%s", "source"))
    {
        __debugbreak();
    }
    if (!input
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state_utils.cpp", 34, 0, "%s", "input"))
    {
        __debugbreak();
    }
    memset(source, 0, sizeof(GfxCmdBufSourceState));
    memcpy(&source->input, input, sizeof(source->input));
    for (constant = 0; constant < 0xC5; ++constant)
    {
        if (input == &gfxCmdBufInput || s_codeConstUpdateFreq[constant] != MTL_UPDATE_RARELY || constant < 0x2F)
        {
            source->input.consts[constant][0] = FLT_MAX;
            v3 = source->input.consts[constant];
            v3[1] = FLT_MAX;
            v3[2] = FLT_MAX;
            v3[3] = 0.0f;
        }
        else
        {
            source->constVersions[constant] = 1;
        }
    }
    if (source->shadowableLightForShadowLookupMatrix
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state_utils.cpp",
            68,
            1,
            "%s",
            "source->shadowableLightForShadowLookupMatrix == PRIMARY_LIGHT_NONE"))
    {
        __debugbreak();
    }
    source->skinnedPlacement.base.quat[3] = 1.0f;
    source->skinnedPlacement.scale = 1.0f;
    source->cameraView = cameraView;
}

void __cdecl R_InitCmdBufState(GfxCmdBufState *state)
{
    unsigned int samplerIndex; // [esp+14h] [ebp-Ch]
    IDirect3DDevice9 *device; // [esp+18h] [ebp-8h]
    unsigned int forceSamplerState; // [esp+1Ch] [ebp-4h]

    if ( !state
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state_utils.cpp", 131, 0, "%s", "state") )
    {
        __debugbreak();
    }
    device = state->prim.device;
    memset(state->refSamplerState, 0, sizeof(GfxCmdBufState));
    state->prim.device = device;
    state->depthRangeType = GFX_DEPTH_RANGE_FULL;
    state->depthRangeNear = 0.0f;
    state->depthRangeFar = 1.0f;
    state->renderTargetId = 0;
    R_SetDefaultStateBits(state->activeStateBits);
    R_SetDefaultStateBits(state->refStateBits);
    R_SetCompleteState(device, state->activeStateBits);
    R_SetDefaultAlphaTestFunction(state);
    forceSamplerState = R_DecodeSamplerState(1u);
    for ( samplerIndex = 0; samplerIndex < 0x10; ++samplerIndex )
    {
        R_HW_ForceSamplerState(state->prim.device, samplerIndex, forceSamplerState);
        state->refSamplerState[samplerIndex] = 1;
        state->samplerState[samplerIndex] = forceSamplerState;
    }
    state->prim.vertexDecl = (IDirect3DVertexDeclaration9 *)-1;
    state->pixelShader = (const MaterialPixelShader *)-1;
    state->stateOverride = 0;
    state->prim.viewStats = state->prim.frameStats.viewStats;
}

void __cdecl R_HW_ForceSamplerState(IDirect3DDevice9 *device, unsigned int samplerIndex, unsigned int samplerState)
{
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    const char *v9; // eax
    int v11; // [esp+4h] [ebp-48h]
    int v13; // [esp+Ch] [ebp-40h]
    int v15; // [esp+14h] [ebp-38h]
    int v17; // [esp+1Ch] [ebp-30h]
    int v19; // [esp+24h] [ebp-28h]
    int v21; // [esp+2Ch] [ebp-20h]
    int hr; // [esp+34h] [ebp-18h]

    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("device->SetSamplerState( samplerIndex, D3DSAMP_MINFILTER, minFilter )\n");
    hr = device->SetSamplerState(samplerIndex, (D3DSAMPLERSTATETYPE)6, (unsigned __int16)(samplerState & 0xF00) >> 8);
    //hr = ((int (__thiscall *)(IDirect3DDevice9 *, IDirect3DDevice9 *, unsigned int, int, int))device->SetSamplerState)(
    //             device,
    //             device,
    //             samplerIndex,
    //             6,
    //             (unsigned __int16)(samplerState & 0xF00) >> 8);
    if ( hr < 0 )
    {
        ++g_disableRendering;
        v3 = R_ErrorDescription(hr);
        Com_Error(
            ERR_FATAL,
            "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetSamplerState( samplerIndex, D3DSAMP_M"
            "INFILTER, minFilter ) failed: %s\n",
            288,
            v3);
    }
    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("device->SetSamplerState( samplerIndex, D3DSAMP_MAGFILTER, magFilter )\n");
    v21 = device->SetSamplerState(
                    samplerIndex,
                    D3DSAMP_MAGFILTER,
                    (unsigned __int16)(samplerState & 0xF000) >> 12);
    if ( v21 < 0 )
    {
        ++g_disableRendering;
        v4 = R_ErrorDescription(v21);
        Com_Error(
            ERR_FATAL,
            "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetSamplerState( samplerIndex, D3DSAMP_M"
            "AGFILTER, magFilter ) failed: %s\n",
            291,
            v4);
    }
    if ( (unsigned __int8)samplerState > 1u )
    {
        if ( r_logFile && r_logFile->current.integer )
            RB_LogPrint("device->SetSamplerState( samplerIndex, D3DSAMP_MAXANISOTROPY, anisotropy )\n");
        v19 = device->SetSamplerState(samplerIndex, D3DSAMP_MAXANISOTROPY, (unsigned __int8)samplerState);
        if ( v19 < 0 )
        {
            ++g_disableRendering;
            v5 = R_ErrorDescription(v19);
            Com_Error(
                ERR_FATAL,
                "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetSamplerState( samplerIndex, D3DSAMP"
                "_MAXANISOTROPY, anisotropy ) failed: %s\n",
                295,
                v5);
        }
    }
    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("device->SetSamplerState( samplerIndex, D3DSAMP_MIPFILTER, mipFilter )\n");
    //v17 = ((int (__thiscall *)(IDirect3DDevice9 *, IDirect3DDevice9 *, unsigned int, int, unsigned int))device->SetSamplerState)(
    //                device,
    //                device,
    //                samplerIndex,
    //                7,
    //                (samplerState & 0xF0000) >> 16);
    v17 = device->SetSamplerState(samplerIndex, (D3DSAMPLERSTATETYPE)7, (samplerState & 0xF0000) >> 16);
    if ( v17 < 0 )
    {
        ++g_disableRendering;
        v6 = R_ErrorDescription(v17);
        Com_Error(
            ERR_FATAL,
            "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetSamplerState( samplerIndex, D3DSAMP_M"
            "IPFILTER, mipFilter ) failed: %s\n",
            298,
            v6);
    }
    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("device->SetSamplerState( samplerIndex, D3DSAMP_ADDRESSU, address )\n");
    v15 = device->SetSamplerState(samplerIndex, D3DSAMP_ADDRESSU, (samplerState & 0x300000) >> 20);
    if ( v15 < 0 )
    {
        ++g_disableRendering;
        v7 = R_ErrorDescription(v15);
        Com_Error(
            ERR_FATAL,
            "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetSamplerState( samplerIndex, D3DSAMP_A"
            "DDRESSU, address ) failed: %s\n",
            305,
            v7);
    }
    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("device->SetSamplerState( samplerIndex, D3DSAMP_ADDRESSV, address )\n");
    v13 = device->SetSamplerState(
                    samplerIndex,
                    D3DSAMP_ADDRESSV,
                    (0xC00000 & samplerState) >> 22);
    if ( v13 < 0 )
    {
        ++g_disableRendering;
        v8 = R_ErrorDescription(v13);
        Com_Error(
            ERR_FATAL,
            "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetSamplerState( samplerIndex, D3DSAMP_A"
            "DDRESSV, address ) failed: %s\n",
            308,
            v8);
    }
    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("device->SetSamplerState( samplerIndex, D3DSAMP_ADDRESSW, address )\n");
    //v11 = ((int (__thiscall *)(IDirect3DDevice9 *, IDirect3DDevice9 *, unsigned int, int, unsigned int))device->SetSamplerState)(
    //                device,
    //                device,
    //                samplerIndex,
    //                3,
    //                (samplerState & 0x3000000) >> 24);
    v11 = device->SetSamplerState(samplerIndex, (D3DSAMPLERSTATETYPE)3, (samplerState & 0x3000000) >> 24);
    if ( v11 < 0 )
    {
        ++g_disableRendering;
        v9 = R_ErrorDescription(v11);
        Com_Error(
            ERR_FATAL,
            "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetSamplerState( samplerIndex, D3DSAMP_A"
            "DDRESSW, address ) failed: %s\n",
            311,
            v9);
    }
}

void __cdecl R_SetDefaultAlphaTestFunction(GfxCmdBufState *state)
{
    const char *v1; // eax
    int hr; // [esp+4h] [ebp-8h]
    IDirect3DDevice9 *device; // [esp+8h] [ebp-4h]

    device = state->prim.device;
    if ( !device && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.h", 2011, 0, "%s", "device") )
        __debugbreak();
    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("device->SetRenderState( D3DRS_ALPHAREF, 0 )\n");
    hr = device->SetRenderState(D3DRS_ALPHAREF, 0);
    if ( hr < 0 )
    {
        ++g_disableRendering;
        v1 = R_ErrorDescription(hr);
        Com_Error(
            ERR_FATAL,
            "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.h (%i) device->SetRenderState( D3DRS_ALPHAREF, 0 ) failed: %s\n",
            2013,
            v1);
    }
    state->alphaRef = 0;
}

unsigned int __cdecl R_DecodeSamplerState(unsigned __int8 samplerState)
{
    unsigned int tableIndex; // [esp+0h] [ebp-8h]

    tableIndex = samplerState & 0x1F;
    if (tableIndex >= 0x18
        && !Assert_MyHandler(
            "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.h",
            2421,
            0,
            "tableIndex doesn't index ARRAY_COUNT(s_decodeSamplerFilterState)\n\t%i not in [0, %i)",
            tableIndex,
            24))
    {
        __debugbreak();
    }
    return s_decodeSamplerFilterState[tableIndex]
        | 0x1500000
        | ((samplerState & 0x20) << 16)
        | ((samplerState & 0x40) << 17)
        | ((samplerState & 0x80) << 18);
}

void __cdecl R_SetDefaultStateBits(unsigned int *stateBits)
{
    *stateBits = 0;
    stateBits[1] = 0;
    *stateBits |= 0x800u;
    *stateBits |= 0x18000000u;
    *stateBits |= 0x4000u;
    *stateBits = *stateBits;
    *stateBits = *stateBits;
    stateBits[1] |= 2u;
    stateBits[1] = stateBits[1];
}

void __cdecl R_InitCmdBufSamplerState(GfxCmdBufState *state)
{
    unsigned int samplerIndex; // [esp+8h] [ebp-Ch]
    unsigned int forceSamplerState; // [esp+10h] [ebp-4h]

    iassert(state);

    forceSamplerState = R_DecodeSamplerState(1);
    for (samplerIndex = 0; samplerIndex < 0x10; ++samplerIndex)
    {
        R_HW_ForceSamplerState(state->prim.device, samplerIndex, forceSamplerState);
        state->refSamplerState[samplerIndex] = 1;
        state->samplerState[samplerIndex] = forceSamplerState;
    }
}

GfxCmdBufSourceState *__cdecl R_GetActiveWorldMatrix(GfxCmdBufSourceState *source)
{
    R_WorldMatrixChanged(source);
    return source;
}

void __cdecl R_WorldMatrixChanged(GfxCmdBufSourceState *source)
{
    ++source->matrixVersions[0];
    ++source->matrixVersions[3];
    ++source->matrixVersions[5];
    ++source->matrixVersions[7];

    source->constVersions[CONST_SRC_CODE_WORLD_MATRIX] = source->matrixVersions[0];
}

void __cdecl R_Set2D(GfxCmdBufSourceState *source)
{
    GfxViewport viewport; // [esp+10h] [ebp-10h] BYREF

    if (source->viewMode != VIEW_MODE_2D)
    {
        PROF_SCOPED("R_Set2D");
        source->viewMode = VIEW_MODE_2D;
        source->viewportIsDirty = 1;
        source->eyeOffset[0] = 0.0f;
        source->eyeOffset[1] = 0.0f;
        source->eyeOffset[2] = 0.0f;
        source->eyeOffset[3] = 1.0f;
        R_GetViewport(source, &viewport);
        R_CmdBufSet2D(source, &viewport);
        R_GetViewport(source, &viewport);
        R_CmdBufSet2D(source, &viewport);
    }
}

// local variable allocation has failed, the output may be wrong!
void    R_CmdBufSet2D(GfxCmdBufSourceState *source, GfxViewport *viewport)
{
    // kcod4
    GfxViewParms *v2; // ebp
    float v3[16]; // [esp-8h] [ebp-9Ch] BYREF
    GfxMatrix identity_52; // [esp+38h] [ebp-5Ch] BYREF
    GfxViewParms *transform_56; // [esp+7Ch] [ebp-18h]
    float transform_60; // [esp+80h] [ebp-14h]
    float v7; // [esp+84h] [ebp-10h]
    GfxViewParms *viewParms; // [esp+88h] [ebp-Ch]
    float invHeight; // [esp+8Ch] [ebp-8h]
    float retaddr; // [esp+94h] [ebp+0h]

    //viewParms = v2;
    //invHeight = retaddr;
    iassert(viewport->width > 0);
    iassert(viewport->height > 0);
    
    v7 = 1.0 / (double)viewport->width;
    transform_60 = 1.0 / (double)viewport->height;
    transform_56 = &source->viewParms;
    memset(&identity_52, 0, sizeof(identity_52));
    identity_52.m[0][0] = v7 * 2.0;
    identity_52.m[1][1] = transform_60 * -2.0;
    identity_52.m[3][0] = -1.0 - v7;
    identity_52.m[3][1] = transform_60 + 1.0;
    identity_52.m[3][2] = 1.0;
    identity_52.m[3][3] = 1.0;
    R_MatrixIdentity44((float (*)[4])v3);
    R_MatrixIdentity44(transform_56->viewMatrix.m);
    memcpy(&transform_56->projectionMatrix, &identity_52, sizeof(transform_56->projectionMatrix));
    memcpy(&transform_56->viewProjectionMatrix, &identity_52, sizeof(transform_56->viewProjectionMatrix));
    memset(
        (unsigned __int8 *)&transform_56->inverseViewProjectionMatrix,
        0,
        sizeof(transform_56->inverseViewProjectionMatrix));
    ++source->matrixVersions[1];
    ++source->matrixVersions[2];
    ++source->matrixVersions[4];

    memcpy(R_GetActiveWorldMatrix(source), v3, 0x40u);
}

void __cdecl R_MatrixIdentity44(float (*out)[4])
{
    iassert(out);
    memcpy(out, g_identityMatrix44, 0x40u);
}

void __cdecl R_CmdBufSet3D(GfxCmdBufSourceState *source)
{
  GfxCmdBufSourceState *worldMatrix; // [esp+Ch] [ebp-4h]

  ++source->matrixVersions[1];
  ++source->matrixVersions[2];
  ++source->matrixVersions[4];
  worldMatrix = R_GetActiveWorldMatrix(source);
  R_MatrixIdentity44((float (*)[4])worldMatrix);
  worldMatrix->matrices.matrix[0].m[3][0] = worldMatrix->matrices.matrix[0].m[3][0] - source->eyeOffset[0];
  worldMatrix->matrices.matrix[0].m[3][1] = worldMatrix->matrices.matrix[0].m[3][1] - source->eyeOffset[1];
  worldMatrix->matrices.matrix[0].m[3][2] = worldMatrix->matrices.matrix[0].m[3][2] - source->eyeOffset[2];
}

void __cdecl R_Set3D(GfxCmdBufSourceState *source)
{
    iassert(source->viewParms3D);

    if (source->viewMode != VIEW_MODE_3D)
    {
        PROF_SCOPED("R_Set3D");
        source->viewMode = VIEW_MODE_3D;
        memcpy(&source->viewParms, source->viewParms3D, sizeof(source->viewParms));
        if (source->viewParms.origin[3] == 0.0)
        {
            source->eyeOffset[0] = 0.0f;
            source->eyeOffset[1] = 0.0f;
            source->eyeOffset[2] = 0.0f;
        }
        else
        {
            source->eyeOffset[0] = source->viewParms.origin[0];
            source->eyeOffset[1] = source->viewParms.origin[1];
            source->eyeOffset[2] = source->viewParms.origin[2];
        }
        source->eyeOffset[3] = 1.0f;
        R_CmdBufSet3D(source);
        UpdateVPosToWorld(source);
    }
}

void __cdecl R_BeginView(GfxCmdBufSourceState *source, const GfxSceneDef *sceneDef, const GfxViewParms *viewParms)
{
    source->sceneDef = *sceneDef;
    source->viewParms3D = viewParms;
    source->skinnedPlacement.base.origin[0] = sceneDef->viewOffset[0];
    source->skinnedPlacement.base.origin[1] = sceneDef->viewOffset[1];
    source->skinnedPlacement.base.origin[2] = sceneDef->viewOffset[2];
    source->viewMode = VIEW_MODE_NONE;
    R_Set3D(source);
    source->materialTime = 0.0f;
    source->destructibleBurnAmount = 0.0f;
    source->destructibleFadeAmount = 0.0f;
    source->wetness = 0.0f;
    R_SetGameTime(source, source->sceneDef.floatTime);
    R_DeriveNearPlaneConstantsForView(source);
}

void __cdecl R_DeriveNearPlaneConstantsForView(GfxCmdBufSourceState *source)
{
    const GfxViewParms *viewParms = &source->viewParms;
    const GfxMatrix *mtx = &viewParms->inverseViewProjectionMatrix;

    iassert(fabs(mtx->m[0][3]) < 1.0e-5f * mtx->m[3][3]);
    iassert(fabs(mtx->m[1][3]) < 1.0e-5f * mtx->m[3][3]);
    iassert(mtx->m[3][3] != 0);

    float scale = 1.0 / source->viewParms.inverseViewProjectionMatrix.m[3][3];

    R_SetCodeConstant(source,
        CONST_SRC_CODE_NEARPLANE_ORG,
    (scale * source->viewParms.inverseViewProjectionMatrix.m[3][0]) - source->viewParms.origin[0],
    (scale * source->viewParms.inverseViewProjectionMatrix.m[3][1]) - source->viewParms.origin[1],
    (scale * source->viewParms.inverseViewProjectionMatrix.m[3][2]) - source->viewParms.origin[2],
    0.0f
    );

    float scale2 = scale + scale;

    R_SetCodeConstant(
        source,
        CONST_SRC_CODE_NEARPLANE_DX,
        scale2 * source->viewParms.inverseViewProjectionMatrix.m[0][0],
        scale2 * source->viewParms.inverseViewProjectionMatrix.m[0][1],
        scale2 * source->viewParms.inverseViewProjectionMatrix.m[0][2],
        0.0f
        );
    R_SetCodeConstant(
        source,
        CONST_SRC_CODE_NEARPLANE_DY,
        -scale2 * source->viewParms.inverseViewProjectionMatrix.m[1][0],
        -scale2 * source->viewParms.inverseViewProjectionMatrix.m[1][1],
        -scale2 * source->viewParms.inverseViewProjectionMatrix.m[1][2],
        0.0f
    );
}

void __cdecl R_SetShadowLookupMatrix(GfxCmdBufSourceState *source, const GfxMatrix *matrix)
{
    ++source->matrixVersions[6];
    memcpy(&source->shadowLookupMatrix, matrix, sizeof(source->shadowLookupMatrix));
}

