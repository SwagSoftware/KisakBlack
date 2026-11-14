#include "r_state.h"
#include "r_bsp.h"
#include "r_dvars.h"
#include "rb_logfile.h"
#include "r_state_utils.h"
#include <ik/ik_math.h>
#include <qcommon/cm_mesh.h>
#include "rb_stats.h"
#include "rb_draw3d.h"
#include "r_foliage.h"
#include "rb_pixelcost.h"
#include "rb_shade.h"
#include "r_reflection_probe.h"
#include <qcommon/common.h>


const GfxViewportBehavior s_viewportBehaviorForRenderTarget[44] =
{
  GFX_USE_VIEWPORT_FULL,
  GFX_USE_VIEWPORT_FULL,
  GFX_USE_VIEWPORT_FOR_VIEW,
  GFX_USE_VIEWPORT_FOR_VIEW,
  GFX_USE_VIEWPORT_FOR_VIEW,
  GFX_USE_VIEWPORT_FULL,
  GFX_USE_VIEWPORT_FULL,
  GFX_USE_VIEWPORT_FOR_VIEW,
  GFX_USE_VIEWPORT_FULL,
  GFX_USE_VIEWPORT_FULL,
  GFX_USE_VIEWPORT_FULL,
  GFX_USE_VIEWPORT_FULL,
  GFX_USE_VIEWPORT_FULL,
  GFX_USE_VIEWPORT_FULL,
  GFX_USE_VIEWPORT_FOR_VIEW,
  GFX_USE_VIEWPORT_FOR_VIEW,
  GFX_USE_VIEWPORT_FOR_VIEW,
  GFX_USE_VIEWPORT_FOR_VIEW,
  GFX_USE_VIEWPORT_FOR_VIEW,
  GFX_USE_VIEWPORT_FULL,
  GFX_USE_VIEWPORT_FOR_VIEW,
  GFX_USE_VIEWPORT_FULL,
  GFX_USE_VIEWPORT_FULL,
  GFX_USE_VIEWPORT_FULL,
  GFX_USE_VIEWPORT_FULL,
  GFX_USE_VIEWPORT_FULL,
  GFX_USE_VIEWPORT_FULL,
  GFX_USE_VIEWPORT_FULL,
  GFX_USE_VIEWPORT_FULL,
  GFX_USE_VIEWPORT_FULL,
  GFX_USE_VIEWPORT_FULL,
  GFX_USE_VIEWPORT_FULL,
  GFX_USE_VIEWPORT_FULL,
  GFX_USE_VIEWPORT_FULL,
  GFX_USE_VIEWPORT_FULL,
  GFX_USE_VIEWPORT_FULL,
  GFX_USE_VIEWPORT_FULL,
  GFX_USE_VIEWPORT_FULL,
  GFX_USE_VIEWPORT_FULL,
  GFX_USE_VIEWPORT_FULL,
  GFX_USE_VIEWPORT_FULL,
  GFX_USE_VIEWPORT_FULL,
  GFX_USE_VIEWPORT_FULL,
  GFX_USE_VIEWPORT_FULL
};



thread_local bool s_pixRenderTargetImage;

const unsigned int s_cullTable_64[4] = { 0u, 1u, 3u, 2u };
const unsigned int s_blendTable_64[11] = { 0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u };
const unsigned int s_blendOpTable_64[6] = { 0u, 1u, 2u, 3u, 4u, 5u };
const unsigned int s_depthTestTable_64[4] = { 8u, 2u, 3u, 4u };
const unsigned int s_stencilOpTable_64[8] = { 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u };
const unsigned int s_stencilFuncTable_64[8] = { 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u };




const _D3DTEXTUREFILTERTYPE s_mipFilterTable[4][3] =
{
  { D3DTEXF_NONE, D3DTEXF_POINT, D3DTEXF_LINEAR },
  { D3DTEXF_NONE, D3DTEXF_LINEAR, D3DTEXF_LINEAR },
  { D3DTEXF_NONE, D3DTEXF_POINT, D3DTEXF_POINT },
  { D3DTEXF_NONE, D3DTEXF_NONE, D3DTEXF_NONE }
};

unsigned int s_decodeSamplerFilterState[24];

void __cdecl R_PixStartNamedRenderTarget(unsigned __int8 renderTargetId)
{
    const char *v1; // eax
    const char *v2; // eax

    R_PixEndNamedRenderTarget();
    if (gfxRenderTargets[renderTargetId].image)
    {
        v1 = va("Trgt: %s", gfxRenderTargets[renderTargetId].image->name);
    }
    else
    {
        v2 = R_RenderTargetName(renderTargetId);
        v1 = va("Trgt: %s", v2);
    }
    //PIXSetMarker(-1, v1);
    s_pixRenderTargetImage = 1;
}

void R_PixEndNamedRenderTarget()
{
    s_pixRenderTargetImage = 0;
}

void __cdecl R_SetTexFilter()
{
    int v0; // [esp+0h] [ebp-60h]
    int v1; // [esp+4h] [ebp-5Ch]
    int v2; // [esp+8h] [ebp-58h]
    int v3; // [esp+Ch] [ebp-54h]
    int v4; // [esp+10h] [ebp-50h]
    int v5; // [esp+14h] [ebp-4Ch]
    int v6; // [esp+1Ch] [ebp-44h]
    int integer; // [esp+24h] [ebp-3Ch]
    unsigned int entryIndex; // [esp+30h] [ebp-30h]
    int maxAniso; // [esp+34h] [ebp-2Ch]
    unsigned int mipFilterMode; // [esp+38h] [ebp-28h]
    int linearMippedAnisotropy; // [esp+3Ch] [ebp-24h]
    int linearNonMippedFilter; // [esp+40h] [ebp-20h]
    int linearMippedFilter; // [esp+44h] [ebp-1Ch]
    int texFilter; // [esp+48h] [ebp-18h]
    int anisotropicFilter; // [esp+50h] [ebp-10h]
    int anisotropyFor4x; // [esp+54h] [ebp-Ch]
    _D3DTEXTUREFILTERTYPE decoded; // [esp+58h] [ebp-8h]
    unsigned int decodeda; // [esp+58h] [ebp-8h]
    int anisotropyFor2x; // [esp+5Ch] [ebp-4h]

    maxAniso = r_texFilterAnisoMax->current.integer;
    if ( maxAniso > gfxMetrics.maxAnisotropy )
        maxAniso = gfxMetrics.maxAnisotropy;
    if ( maxAniso > 1 )
    {
        anisotropicFilter = (gfxMetrics.hasAnisotropicMinFilter ? 768 : 512)
                                            | (gfxMetrics.hasAnisotropicMagFilter ? 12288 : 0x2000);
    }
    else
    {
        maxAniso = 1;
        anisotropicFilter = 8704;
    }
    if ( maxAniso >= r_texFilterAnisoMin->current.integer )
        integer = r_texFilterAnisoMin->current.integer;
    else
        integer = maxAniso;
    if ( integer <= 1 )
        v6 = 1;
    else
        v6 = integer;
    if ( v6 != integer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp",
                    810,
                    0,
                    "%s",
                    "max( minAniso, min( maxAniso, 1 ) ) == minAniso") )
    {
        __debugbreak();
    }
    linearMippedAnisotropy = integer;
    if ( integer == 1 )
    {
        linearMippedFilter = 8704;
    }
    else
    {
        if ( integer <= 1
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp",
                        819,
                        0,
                        "%s",
                        "linearMippedAnisotropy > 1") )
        {
            __debugbreak();
        }
        linearMippedFilter = anisotropicFilter;
    }
    if ( maxAniso >= 2 )
        v5 = 2;
    else
        v5 = maxAniso;
    if ( integer <= v5 )
    {
        if ( maxAniso >= 2 )
            v3 = 2;
        else
            v3 = maxAniso;
        v4 = v3;
    }
    else
    {
        v4 = integer;
    }
    anisotropyFor2x = v4;
    if ( maxAniso >= 4 )
        v2 = 4;
    else
        v2 = maxAniso;
    if ( integer <= v2 )
    {
        if ( maxAniso >= 4 )
            v0 = 4;
        else
            v0 = maxAniso;
        v1 = v0;
    }
    else
    {
        v1 = integer;
    }
    anisotropyFor4x = v1;
    if ( r_texFilterDisable->current.enabled )
    {
        linearMippedAnisotropy = 1;
        anisotropyFor2x = 1;
        anisotropyFor4x = 1;
        anisotropicFilter = 4352;
        linearMippedFilter = 4352;
        linearNonMippedFilter = 4352;
        mipFilterMode = 3;
    }
    else
    {
        linearNonMippedFilter = 8704;
        mipFilterMode = r_texFilterMipMode->current.unsignedInt;
        if ( mipFilterMode >= 4
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp",
                        843,
                        0,
                        "mipFilterMode doesn't index R_MIP_FILTER_COUNT\n\t%i not in [0, %i)",
                        mipFilterMode,
                        4) )
        {
            __debugbreak();
        }
    }
    for ( entryIndex = 0; entryIndex < 0x18; ++entryIndex )
    {
        texFilter = entryIndex & 7;
        decoded = (_D3DTEXTUREFILTERTYPE)(s_mipFilterTable[mipFilterMode][(int)(entryIndex & 0x18) >> 3] << 16);
        switch ( texFilter )
        {
            case 2:
                if ( decoded )
                    decodeda = decoded | linearMippedAnisotropy | linearMippedFilter;
                else
                    decodeda = linearNonMippedFilter | 1;
                break;
            case 3:
                decodeda = decoded | anisotropyFor2x | anisotropicFilter;
                break;
            case 4:
                decodeda = decoded | anisotropyFor4x | anisotropicFilter;
                break;
            default:
                decodeda = decoded | 0x1101;
                break;
        }
        s_decodeSamplerFilterState[entryIndex] = decodeda;
    }
    dx.mipBias = r_texFilterMipBias->current.integer;
}

void __cdecl R_SetInitialContextState(IDirect3DDevice9 *device)
{
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    int v5; // [esp+4h] [ebp-14h]
    int v7; // [esp+Ch] [ebp-Ch]
    int hr; // [esp+14h] [ebp-4h]

    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("device->SetRenderState( D3DRS_SEPARATEALPHABLENDENABLE, 1 )\n");
    //hr = ((int (__thiscall *)(IDirect3DDevice9 *, IDirect3DDevice9 *, int, int))device->SetRenderState)(
    //             device,
    //             device,
    //             206,
    //             1);
    hr = device->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, 1);
    if ( hr < 0 )
    {
        ++g_disableRendering;
        v1 = R_ErrorDescription(hr);
        Com_Error(
            ERR_FATAL,
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp (%i) device->SetRenderState( D3DRS_SEPARATEALPHABLENDENABL"
            "E, 1 ) failed: %s\n",
            885,
            v1);
    }
    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("device->SetRenderState( D3DRS_TWOSIDEDSTENCILMODE, 1 )\n");
    v7 = device->SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, 1u);
    if ( v7 < 0 )
    {
        ++g_disableRendering;
        v2 = R_ErrorDescription(v7);
        Com_Error(
            ERR_FATAL,
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp (%i) device->SetRenderState( D3DRS_TWOSIDEDSTENCILMODE, 1 ) failed: %s\n",
            886,
            v2);
    }
    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("device->SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE )\n");
    v5 = device->SetRenderState(D3DRS_ZENABLE, 0);
    if ( v5 < 0 )
    {
        ++g_disableRendering;
        v3 = R_ErrorDescription(v5);
        Com_Error(
            ERR_FATAL,
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp (%i) device->SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE ) failed: %s\n",
            891,
            v3);
    }
}

void __cdecl R_ChangeDepthHackNearClip(GfxCmdBufSourceState *source, float depthHackFlags)
{
    iassert(source);

    if (depthHackFlags != source->depthHackFlags)
    {
        R_DepthHackNearClipChanged(source);
        source->depthHackFlags = depthHackFlags;
    }
}

void __cdecl R_DepthHackNearClipChanged(GfxCmdBufSourceState *source)
{
    ++source->matrixVersions[2];
    ++source->matrixVersions[4];
    ++source->matrixVersions[5];
    //LODWORD(source->input.consts[75][3]) ^= _mask__NegFloat_;
    (source->input.consts[75][3]) = -(source->input.consts[75][3]);
    ++source->constVersions[75];
}

void __cdecl R_ChangeObjectPlacement(GfxCmdBufSourceState *source, const GfxScaledPlacement *placement)
{
    R_ChangeObjectPlacement_Core(source, placement);
}

void __cdecl R_ChangeObjectPlacement_Core(GfxCmdBufSourceState *source, const GfxScaledPlacement *placement)
{
    double v3; // st7
    char *v4; // eax
    double v5; // st7
    char *v6; // eax
    double v7; // st7
    char *v8; // eax
    GfxCmdBufSourceState *matrix; // [esp+40h] [ebp-34h]
    float origin[3]; // [esp+44h] [ebp-30h] BYREF
    float axis[3][3]; // [esp+50h] [ebp-24h] BYREF

    if (!placement
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp", 980, 0, "%s", "placement"))
    {
        __debugbreak();
    }
    UnitQuatToAxis(placement->base.quat, axis);
    if (!Vec3IsNormalized(axis[0]))
    {
        v3 = Vec3Length(axis[0]);
        v4 = va("(%g %g %g) len %g", axis[0][0], axis[0][1], axis[0][2], v3);
        if (!Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp",
            983,
            0,
            "%s\n\t%s",
            "Vec3IsNormalized( axis[0] )",
            v4))
            __debugbreak();
    }
    if (!Vec3IsNormalized(axis[1]))
    {
        v5 = Vec3Length(axis[1]);
        v6 = va("(%g %g %g) len %g", axis[1][0], axis[1][1], axis[1][2], v5);
        if (!Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp",
            984,
            0,
            "%s\n\t%s",
            "Vec3IsNormalized( axis[1] )",
            v6))
            __debugbreak();
    }
    if (!Vec3IsNormalized(axis[2]))
    {
        v7 = Vec3Length(axis[2]);
        v8 = va("(%g %g %g) len %g", axis[2][0], axis[2][1], axis[2][2], v7);
        if (!Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp",
            985,
            0,
            "%s\n\t%s",
            "Vec3IsNormalized( axis[2] )",
            v8))
            __debugbreak();
    }
    matrix = R_GetActiveWorldMatrix(source);
    origin[0] = placement->base.origin[0] - source->eyeOffset[0];
    origin[1] = placement->base.origin[1] - source->eyeOffset[1];
    origin[2] = placement->base.origin[2] - source->eyeOffset[2];
    ikMatrixSet44((float (*)[4])matrix, origin, axis, placement->scale);
}

void __cdecl R_ChangeObjectPlacementRemote(GfxCmdBufSourceState *source, const GfxScaledPlacement *remotePlacement)
{
    R_ChangeObjectPlacement(source, remotePlacement);
}

float *__cdecl R_GetCodeMatrix(
    GfxCmdBufSourceState *source,
    unsigned int sourceIndex,
    unsigned int firstRow)
{
    unsigned int baseIndex; // [esp+10Ch] [ebp-18h]
    unsigned int transposeIndex; // [esp+110h] [ebp-14h]
    unsigned int matrixIndex; // [esp+118h] [ebp-Ch]
    unsigned int inverseIndex; // [esp+11Ch] [ebp-8h]
    unsigned int matrixVersion; // [esp+120h] [ebp-4h]

    iassert(source->matrixVersions[MATRIX_VERSIONS_INDEX(sourceIndex)]);
    bcassert2(firstRow, 3);

    matrixVersion = source->matrixVersions[MATRIX_VERSIONS_INDEX(sourceIndex)];
    matrixIndex = MATRIX_INDEX(sourceIndex);

    if (source->constVersions[sourceIndex] == matrixVersion)
    {
        return source->matrices.matrix[matrixIndex].m[firstRow];
    }

    baseIndex = matrixIndex & 0xFFFFFFFC;

    if (source->constVersions[(matrixIndex & 0xFFFFFFFC) + CONST_SRC_FIRST_CODE_MATRIX] != matrixVersion)
    {
        R_DeriveCodeMatrix(source, &source->matrices, baseIndex);
        if (matrixIndex == baseIndex)
            return source->matrices.matrix[matrixIndex].m[firstRow];

        iassert(source->constVersions[sourceIndex] != matrixVersion);
    }

    source->constVersions[sourceIndex] = matrixVersion;
    transposeIndex = matrixIndex ^ 2;
    if (source->constVersions[(matrixIndex ^ 2) + CONST_SRC_FIRST_CODE_MATRIX] == matrixVersion)
    {
        MatrixTranspose44(
            *(mat4x4*)&source->matrices.matrix[transposeIndex],
            *(mat4x4*)&source->matrices.matrix[matrixIndex]);
        return source->matrices.matrix[matrixIndex].m[firstRow];
    }

    inverseIndex = matrixIndex ^ 1;
    if (source->constVersions[(matrixIndex ^ 1) + CONST_SRC_FIRST_CODE_MATRIX] == matrixVersion)
    {
        MatrixInverse44(
            *(mat4x4*)&source->matrices.matrix[inverseIndex],
            *(mat4x4*)&source->matrices.matrix[matrixIndex]);
        return source->matrices.matrix[matrixIndex].m[firstRow];
    }

    iassert(matrixIndex == (baseIndex | CONST_SRC_MATRIX_INVERSE_BIT | CONST_SRC_MATRIX_TRANSPOSE_BIT));
    iassert(transposeIndex == (baseIndex | CONST_SRC_MATRIX_INVERSE_BIT));
    iassert(inverseIndex == (baseIndex | CONST_SRC_MATRIX_TRANSPOSE_BIT));

    MatrixTranspose44(*(mat4x4*)&source->matrices.matrix[baseIndex], *(mat4x4 *)&source->matrices.matrix[inverseIndex]);
    source->constVersions[inverseIndex + CONST_SRC_FIRST_CODE_MATRIX] = matrixVersion;
    MatrixInverse44(*(mat4x4*)&source->matrices.matrix[inverseIndex], *(mat4x4*)&source->matrices.matrix[matrixIndex]);

    return source->matrices.matrix[matrixIndex].m[firstRow];
}

void __cdecl R_DeriveCodeMatrix(GfxCmdBufSourceState *source, GfxCodeMatrices *activeMatrices, unsigned int baseIndex)
{
    const char *v3; // eax
    int savedregs; // [esp+110h] [ebp+0h] BYREF

    switch ( baseIndex )
    {
        case 4u:
            R_DeriveViewMatrix(source);
            break;
        case 8u:
            R_DeriveProjectionMatrix(source);
            break;
        case 0xCu:
            R_DeriveWorldViewMatrix(source);
            break;
        case 0x10u:
            R_DeriveViewProjectionMatrix(source);
            break;
        case 0x14u:
            R_DeriveWorldViewProjectionMatrix(source);
            break;
        case 0x18u:
            R_DeriveShadowLookupMatrix(source);
            break;
        case 0x1Cu:
            R_GenerateWorldOutdoorLookupMatrix(source, activeMatrices->matrix[baseIndex].m);
            break;
        default:
            v3 = va("unhandled case %i", baseIndex);
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp", 1262, 1, v3) )
                __debugbreak();
            break;
    }
}

void __cdecl R_DeriveViewMatrix(GfxCmdBufSourceState *source)
{
    memcpy(&source->matrices.matrix[4], &source->viewParms, sizeof(source->matrices.matrix[4]));
    MatrixTransformVector44(source->eyeOffset, source->matrices.matrix[4].m, source->matrices.matrix[4].m[3]);
    source->constVersions[201] = source->matrixVersions[1];
}

void    R_DeriveWorldViewMatrix(GfxCmdBufSourceState *source)
{
    const GfxViewParms *viewParms = &source->viewParms;
    GfxCodeMatrices *activeMatrices = &source->matrices;

    iassert(R_IsMatrixConstantUpToDate(source, CONST_SRC_CODE_WORLD_MATRIX));

    GfxMatrix world = activeMatrices->matrix[MATRIX_INDEX(CONST_SRC_CODE_WORLD_MATRIX)];
    Vec3Add(world.m[3], source->eyeOffset, world.m[3]);
    GfxMatrix *worldView = &activeMatrices->matrix[MATRIX_INDEX(CONST_SRC_CODE_WORLD_VIEW_MATRIX)];
    MatrixMultiply44(world.m, viewParms->viewMatrix.m, worldView->m);
    //source->constVersions[CONST_SRC_CODE_WORLD_VIEW_MATRIX] = source->matrixVersions[3];
    static_assert(MATRIX_VERSIONS_INDEX(CONST_SRC_CODE_WORLD_VIEW_MATRIX) == 3);
    source->constVersions[CONST_SRC_CODE_WORLD_VIEW_MATRIX] = source->matrixVersions[MATRIX_VERSIONS_INDEX(CONST_SRC_CODE_WORLD_VIEW_MATRIX)];
}

void __cdecl R_DeriveProjectionMatrix(GfxCmdBufSourceState *source)
{
    int i; // [esp+8h] [ebp-18h]
    int j; // [esp+Ch] [ebp-14h]
    float inv; // [esp+10h] [ebp-10h]

    memcpy(&source->matrices.matrix[8], &source->viewParms.projectionMatrix, sizeof(source->matrices.matrix[8]));
    if (source->depthHackFlags == 2)
        source->matrices.matrix[8].m[3][2] = source->viewParms.depthHackNearClip;
    if (source->matrices.matrix[8].m[2][3] != 0.0 && source->matrices.matrix[8].m[2][3] != 1.0)
    {
        inv = 1.0 / source->matrices.matrix[8].m[2][3];
        for (j = 0; j < 4; ++j)
        {
            for (i = 0; i < 4; ++i)
                source->matrices.matrix[8].m[i][j] = source->matrices.matrix[8].m[i][j] * inv;
        }
    }
    source->constVersions[205] = source->matrixVersions[2];
}

void __cdecl R_DeriveViewProjectionMatrix(GfxCmdBufSourceState *source)
{
    GfxMatrix *viewProj; // [esp+20h] [ebp-10h]

    viewProj = &source->matrices.matrix[16];
    if (source->depthHackFlags == 2)
    {
        if (source->constVersions[205] != source->matrixVersions[2])
            R_DeriveProjectionMatrix(source);
        MatrixMultiply44(source->viewParms.viewMatrix.m, source->matrices.matrix[8].m, viewProj->m);
    }
    else
    {
        memcpy(viewProj, &source->viewParms.viewProjectionMatrix, sizeof(GfxMatrix));
    }
    MatrixTransformVector44(source->eyeOffset, source->matrices.matrix[16].m, source->matrices.matrix[16].m[3]);
    source->constVersions[213] = source->matrixVersions[4];
}

void    R_DeriveWorldViewProjectionMatrix(GfxCmdBufSourceState *source)
{
    const GfxViewParms *viewParms = &source->viewParms;
    GfxCodeMatrices *activeMatrices = &source->matrices;

    iassert(R_IsMatrixConstantUpToDate(source, CONST_SRC_CODE_WORLD_MATRIX));

    GfxMatrix world = activeMatrices->matrix[MATRIX_INDEX(CONST_SRC_CODE_WORLD_MATRIX)];
    GfxMatrix *worldViewProj = &activeMatrices->matrix[MATRIX_INDEX(CONST_SRC_CODE_WORLD_VIEW_PROJECTION_MATRIX)];

    if (source->depthHackFlags == 2)
    {
        if (!R_IsMatrixConstantUpToDate(source, CONST_SRC_CODE_VIEW_PROJECTION_MATRIX))
        {
            R_DeriveViewProjectionMatrix(source);
        }
        const GfxMatrix *viewProj = &activeMatrices->matrix[MATRIX_INDEX(CONST_SRC_CODE_VIEW_PROJECTION_MATRIX)];
        MatrixMultiply44(world.m, viewProj->m, worldViewProj->m);
    }
    else
    {
        Vec3Add(world.m[3], source->eyeOffset, world.m[3]);
        MatrixMultiply44(world.m, viewParms->viewProjectionMatrix.m, worldViewProj->m);
    }

    static_assert(MATRIX_VERSIONS_INDEX(CONST_SRC_CODE_WORLD_VIEW_PROJECTION_MATRIX) == 5);
    source->constVersions[CONST_SRC_CODE_WORLD_VIEW_PROJECTION_MATRIX] = source->matrixVersions[MATRIX_VERSIONS_INDEX(CONST_SRC_CODE_WORLD_VIEW_PROJECTION_MATRIX)];
}

void __cdecl R_DeriveShadowLookupMatrix(GfxCmdBufSourceState *source)
{
    memcpy(&source->matrices.matrix[24], &source->shadowLookupMatrix, sizeof(source->matrices.matrix[24]));
    MatrixTransformVector44(source->eyeOffset, source->matrices.matrix[24].m, source->matrices.matrix[24].m[3]);
    source->constVersions[221] = source->matrixVersions[6];
}

void    R_GenerateWorldOutdoorLookupMatrix(GfxCmdBufSourceState *source, float (*outMatrix)[4])
{
    GfxMatrix worldMatrix; // [esp-Ch] [ebp-8Ch] BYREF
    float zInTimesInvViewTimesOutdoorLookup[4]; // [esp+40h] [ebp-40h] BYREF
    float zInTimesInvView[4]; // [esp+50h] [ebp-30h] BYREF
    float worldOffset[4]; // [esp+34h] [ebp-4Ch] BYREF
    float zIn[4]; // [esp+60h] [ebp-20h]
    const float *invViewProjMatrix; // [esp+70h] [ebp-10h]

    const float awayBias = r_outdoorAwayBias->current.value;
    const float downBias = r_outdoorDownBias->current.value;

    zInTimesInvView[0] = 0.0;
    zInTimesInvView[1] = 0.0;
    zInTimesInvView[2] = -awayBias;
    zInTimesInvView[3] = 0.0;

    MatrixTransformVector44(zInTimesInvView, (const float (*)[4])R_GetCodeMatrix(source, 202, 0), zInTimesInvViewTimesOutdoorLookup);
    zInTimesInvViewTimesOutdoorLookup[2] += downBias;
    MatrixTransformVector44(zInTimesInvViewTimesOutdoorLookup, g_drawConsts.outdoorLookupMatrix, worldOffset);

    iassert(R_IsMatrixConstantUpToDate(source, CONST_SRC_CODE_WORLD_MATRIX));

    memcpy(&worldMatrix, source, sizeof(worldMatrix));

    Vec3Add(worldMatrix.m[3], source->eyeOffset, worldMatrix.m[3]);

    MatrixMultiply44(worldMatrix.m, g_drawConsts.outdoorLookupMatrix, outMatrix);

    Vec4Add(&(*outMatrix)[12], worldOffset, &(*outMatrix)[12]);

    static_assert(MATRIX_VERSIONS_INDEX(CONST_SRC_CODE_WORLD_OUTDOOR_LOOKUP_MATRIX) == 7);
    source->constVersions[CONST_SRC_CODE_WORLD_OUTDOOR_LOOKUP_MATRIX] = source->matrixVersions[MATRIX_VERSIONS_INDEX(CONST_SRC_CODE_WORLD_OUTDOOR_LOOKUP_MATRIX)];
}

const GfxImage *__cdecl R_GetTextureFromCode(
                GfxCmdBufSourceState *source,
                unsigned int codeTexture,
                unsigned __int8 *samplerState)
{
    bcassert(codeTexture, TEXTURE_SRC_CODE_COUNT);
    iassert(source);

    *samplerState = source->input.codeImageSamplerStates[codeTexture];

    iassert(*samplerState & SAMPLER_FILTER_MASK);
    return source->input.codeImages[codeTexture];
}

void __cdecl R_TextureFromCodeError(const GfxCmdBufContext context, unsigned int codeTexture)
{
    if ( rg.codeImageNames[codeTexture] )
        Com_Error(
            ERR_DROP,
            "Code texture %u '%s' isn't valid. Material='%s', tech='%s', techType=%d\n",
            codeTexture,
            rg.codeImageNames[codeTexture],
            context.state->material->info.name,
            context.state->technique->name,
            context.state->techType);
    else
        Com_Error(
            ERR_DROP,
            "Code texture %u '%s' isn't valid. Material='%s', tech='%s', techType=%d\n",
            codeTexture,
            "noname",
            context.state->material->info.name,
            context.state->technique->name,
            context.state->techType);
}

const GfxImage *__cdecl R_OverrideGrayscaleImage(const dvar_s *dvar)
{
    const char *v2; // eax

    if ( dvar->current.integer == 2 )
        return rgp.whiteImage;
    if ( !dvar->current.integer )
        return rgp.blackImage;
    if ( dvar->current.integer != 3 )
    {
        v2 = va("%s = %i", dvar->name, dvar->current.integer);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp",
                        1359,
                        0,
                        "%s\n\t%s",
                        "dvar->current.integer == R_COLOR_OVERRIDE_GRAY",
                        v2) )
            __debugbreak();
    }
    return rgp.grayImage;
}

void __cdecl R_SetLightmap(GfxCmdBufContext context, unsigned int lmapIndex)
{
    const MaterialPass *pass; // [esp+0h] [ebp-8h]
    GfxImage *overrideImage; // [esp+4h] [ebp-4h]

    if ( !g_worldDraw
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp", 1372, 0, "%s", "g_worldDraw") )
    {
        __debugbreak();
    }
    pass = context.state->pass;
    if ( lmapIndex == 31 )
    {
        if ( (pass->customSamplerFlags & 6) != 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp",
                        1378,
                        0,
                        "%s",
                        "!(pass->customSamplerFlags & ((1 << CUSTOM_SAMPLER_LIGHTMAP_PRIMARY) | (1 << CUSTOM_SAMPLER_LIGHTMAP_SECONDARY)))") )
        {
            __debugbreak();
        }
    }
    else
    {
        if ( lmapIndex >= g_worldDraw->lightmapCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp",
                        1382,
                        0,
                        "lmapIndex doesn't index g_worldDraw->lightmapCount\n\t%i not in [0, %i)",
                        lmapIndex,
                        g_worldDraw->lightmapCount) )
        {
            __debugbreak();
        }
        if ( r_lightMap->current.integer == 1 )
        {
            if ( (pass->customSamplerFlags & 2) != 0 )
            {
                if ( context.source->input.data->prim.hasSunDirChanged )
                    R_SetSampler(context, 0xCu, 0x62u, rgp.whiteImage);
                else
                    R_SetSampler(context, 0xCu, 0x62u, g_worldDraw->lightmaps[lmapIndex].primary);
            }
            if ( (pass->customSamplerFlags & 4) != 0 )
                R_SetSampler(context, 0xDu, 0x62u, g_worldDraw->lightmaps[lmapIndex].secondary);
            if ( (pass->customSamplerFlags & 8) != 0 )
                R_SetSampler(context, 0xEu, 0x62u, g_worldDraw->lightmaps[lmapIndex].secondaryB);
        }
        else
        {
            overrideImage = rgp.r5g6b5Image;
            if ( (pass->customSamplerFlags & 2) != 0 )
                R_SetSampler(context, 0xCu, 0x62u, rgp.r5g6b5Image);
            if ( (pass->customSamplerFlags & 4) != 0 )
                R_SetSampler(context, 0xDu, 0x62u, overrideImage);
            if ( (pass->customSamplerFlags & 8) != 0 )
                R_SetSampler(context, 0xEu, 0x62u, rgp.g16r16Image);
        }
    }
}

void __cdecl R_SetReflectionProbe(GfxCmdBufContext context, unsigned int reflectionProbeIndex)
{
    if ( !g_worldDraw
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp", 1464, 0, "%s", "g_worldDraw") )
    {
        __debugbreak();
    }
    if ( reflectionProbeIndex == 255
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp",
                    1465,
                    0,
                    "%s",
                    "reflectionProbeIndex != REFLECTION_PROBE_INVALID") )
    {
        __debugbreak();
    }
    if ( reflectionProbeIndex >= g_worldDraw->reflectionProbeCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp",
                    1466,
                    0,
                    "reflectionProbeIndex doesn't index g_worldDraw->reflectionProbeCount\n\t%i not in [0, %i)",
                    reflectionProbeIndex,
                    g_worldDraw->reflectionProbeCount) )
    {
        __debugbreak();
    }
    if ( !g_worldDraw->reflectionProbes[reflectionProbeIndex].reflectionImage
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp",
                    1467,
                    0,
                    "%s",
                    "g_worldDraw->reflectionProbes[reflectionProbeIndex].reflectionImage") )
    {
        __debugbreak();
    }
    if ( (context.state->pass->customSamplerFlags & 1) != 0 )
        R_SetSampler(context, 0xFu, 0x72u, g_worldDraw->reflectionProbes[reflectionProbeIndex].reflectionImage);
}

void __cdecl R_ChangeDepthRange(GfxCmdBufState *state, GfxDepthRangeType depthRangeType)
{
    float v2; // [esp+30h] [ebp-Ch]
    float v3; // [esp+34h] [ebp-8h]
    IDirect3DDevice9 *device; // [esp+38h] [ebp-4h]

    if ( state->depthRangeType == depthRangeType
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp",
                    1507,
                    1,
                    "state->depthRangeType != depthRangeType\n\t%i, %i",
                    state->depthRangeType,
                    depthRangeType) )
    {
        __debugbreak();
    }
    state->depthRangeType = depthRangeType;
    if ( depthRangeType )
        v3 = 0.0f;
    else
        v3 = 0.015625f;
    state->depthRangeNear = v3;
    if ( depthRangeType )
        v2 = 0.015625f;
    else
        v2 = 1.0f;
    state->depthRangeFar = v2;
    device = state->prim.device;
    if ( !device && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp", 1516, 0, "%s", "device") )
        __debugbreak();
    R_HW_SetViewport(device, &state->viewport, state->depthRangeNear, state->depthRangeFar);
}

void __cdecl R_HW_SetViewport(IDirect3DDevice9 *device, const GfxViewport *viewport, float nearValue, float farValue)
{
    const char *v4; // eax
    int hr; // [esp+14h] [ebp-1Ch]
    _D3DVIEWPORT9 d3dViewport; // [esp+18h] [ebp-18h] BYREF

    d3dViewport.X = viewport->x;
    d3dViewport.Y = viewport->y;
    d3dViewport.Width = viewport->width;
    d3dViewport.Height = viewport->height;
    d3dViewport.MinZ = nearValue;
    d3dViewport.MaxZ = farValue;
    if ( farValue <= nearValue
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h",
                    334,
                    0,
                    "d3dViewport.MinZ < d3dViewport.MaxZ\n\t%g, %g",
                    d3dViewport.MinZ,
                    d3dViewport.MaxZ) )
    {
        __debugbreak();
    }
    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("device->SetViewport( &d3dViewport )\n");
    hr = device->SetViewport(&d3dViewport);
    if ( hr < 0 )
    {
        ++g_disableRendering;
        v4 = R_ErrorDescription(hr);
        Com_Error(
            ERR_FATAL,
            "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetViewport( &d3dViewport ) failed: %s\n",
            335,
            v4);
    }
}

int __cdecl R_BeginMaterial(GfxCmdBufState *state, const Material *material, unsigned __int8 techType)
{
    const char *v4; // eax
    const char *v5; // eax
    const MaterialTechnique *technique; // [esp+8h] [ebp-4h]

    if ( !material
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp", 1525, 0, "%s", "material") )
    {
        __debugbreak();
    }
    technique = Material_GetTechnique(material, techType);
    if ( !technique )
        return 0;
    if ( r_logFile->current.integer )
    {
        v4 = RB_LogTechniqueType(techType);
        v5 = va("R_BeginMaterial( %s, %s, %s )\n", material->info.name, technique->name, v4);
        RB_LogPrint(v5);
    }
    state->material = material;
    state->techType = techType;
    state->technique = technique;
    return 1;
}

void __cdecl R_ChangeIndices(GfxCmdBufPrimState *state, IDirect3DIndexBuffer9 *ib)
{
    const char *v2; // eax
    int hr; // [esp+4h] [ebp-8h]
    IDirect3DDevice9 *device; // [esp+8h] [ebp-4h]

    if ( ib == state->indexBuffer
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp",
                    1550,
                    0,
                    "%s",
                    "ib != state->indexBuffer") )
    {
        __debugbreak();
    }
    state->indexBuffer = ib;
    device = state->device;
    if ( !state->device
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp", 1554, 0, "%s", "device") )
    {
        __debugbreak();
    }
    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("device->SetIndices( ib )\n");
    hr = device->SetIndices(ib);
    if ( hr < 0 )
    {
        ++g_disableRendering;
        v2 = R_ErrorDescription(hr);
        Com_Error(
            ERR_FATAL,
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp (%i) device->SetIndices( ib ) failed: %s\n",
            1555,
            v2);
    }
}

void __cdecl R_ChangeStreamSource(
                GfxCmdBufPrimState *state,
                unsigned int streamIndex,
                IDirect3DVertexBuffer9 *vb,
                unsigned int vertexOffset,
                unsigned int vertexStride)
{
    const char *v5; // eax
    int hr; // [esp+4h] [ebp-8h]
    IDirect3DDevice9 *device; // [esp+8h] [ebp-4h]

    device = state->device;
    if ( !state->device
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp", 1561, 0, "%s", "device") )
    {
        __debugbreak();
    }
    if ( state->streams[streamIndex].vb == vb
        && state->streams[streamIndex].offset == vertexOffset
        && state->streams[streamIndex].stride == vertexStride
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp",
                    1563,
                    0,
                    "%s",
                    "state->streams[streamIndex].vb != vb || state->streams[streamIndex].offset != vertexOffset || state->streams[s"
                    "treamIndex].stride != vertexStride") )
    {
        __debugbreak();
    }
    state->streams[streamIndex].vb = vb;
    state->streams[streamIndex].offset = vertexOffset;
    state->streams[streamIndex].stride = vertexStride;
    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("device->SetStreamSource( streamIndex, vb, vertexOffset, vertexStride )\n");
    hr = device->SetStreamSource(streamIndex, vb, vertexOffset, vertexStride);
    if ( hr < 0 )
    {
        ++g_disableRendering;
        v5 = R_ErrorDescription(hr);
        Com_Error(
            ERR_FATAL,
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp (%i) device->SetStreamSource( streamIndex, vb, vertexOffse"
            "t, vertexStride ) failed: %s\n",
            1573,
            v5);
    }
}

void __cdecl R_ClearAllStreamSources(GfxCmdBufPrimState *state)
{
    if ( state->streams[0].vb || state->streams[0].offset || state->streams[0].stride )
        R_ChangeStreamSource(state, 0, 0, 0, 0);
    if ( state->streams[1].vb || state->streams[1].offset || state->streams[1].stride )
        R_ChangeStreamSource(state, 1u, 0, 0, 0);
    if ( state->streams[2].vb || state->streams[2].offset || state->streams[2].stride )
        R_ChangeStreamSource(state, 2u, 0, 0, 0);
}

void __cdecl R_DrawIndexedPrimitive(GfxCmdBufPrimState *state, const GfxDrawPrimArgs *args)
{
    const char *v2; // eax
    int hr; // [esp+4h] [ebp-Ch]
    int triCount; // [esp+8h] [ebp-8h]
    IDirect3DDevice9 *device; // [esp+Ch] [ebp-4h]

    triCount = args->triCount;
    if ( triCount >= r_drawPrimFloor->current.integer
        && (!r_drawPrimCap->current.integer || triCount <= r_drawPrimCap->current.integer) )
    {
        if ( r_skipDrawTris->current.enabled )
            triCount = 1;
        device = state->device;
        if ( !state->device
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp", 1605, 0, "%s", "device") )
        {
            __debugbreak();
        }
        RB_TrackDrawPrimCall(state, triCount);
        if ( r_logFile && r_logFile->current.integer )
            RB_LogPrint("device->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, args->vertexCount, args->baseIndex, triCount )\n");
        hr = device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, args->vertexCount, args->baseIndex, triCount);
        if ( hr < 0 )
        {
            ++g_disableRendering;
            v2 = R_ErrorDescription(hr);
            Com_Error(
                ERR_FATAL,
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp (%i) device->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0"
                ", 0, args->vertexCount, args->baseIndex, triCount ) failed: %s\n",
                1617,
                v2);
        }
    }
}

void __cdecl R_SetAlphaAntiAliasingState(IDirect3DDevice9 *device, __int16 stateBits0)
{
    const char *v2; // eax
    int hr; // [esp+4h] [ebp-Ch]
    _D3DFORMAT aaAlphaFormat; // [esp+8h] [ebp-8h]

    if ( (stateBits0 & 0xF00) != 0 )
    {
        aaAlphaFormat = D3DFMT_UNKNOWN;
    }
    else if ( r_aaAlpha->current.integer == 2 )
    {
        aaAlphaFormat = (_D3DFORMAT)1094800211;
    }
    else
    {
        aaAlphaFormat = (_D3DFORMAT)1129272385;
    }
    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("device->SetRenderState( D3DRS_ADAPTIVETESS_Y, aaAlphaFormat )\n");
    hr = device->SetRenderState(D3DRS_ADAPTIVETESS_Y, aaAlphaFormat);
    if ( hr < 0 )
    {
        ++g_disableRendering;
        v2 = R_ErrorDescription(hr);
        Com_Error(
            ERR_FATAL,
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp (%i) device->SetRenderState( D3DRS_ADAPTIVETESS_Y, aaAlpha"
            "Format ) failed: %s\n",
            1944,
            v2);
    }
}

void __cdecl R_ChangeState_0(GfxCmdBufState *state, unsigned int stateBits0)
{
    bool blendOpRgbWasEnabled; // [esp+43h] [ebp-Dh]
    int changedBits; // [esp+44h] [ebp-Ch]
    IDirect3DDevice9 *device; // [esp+4Ch] [ebp-4h]

    changedBits = state->activeStateBits[0] ^ stateBits0;
    if ( changedBits || ((state->refStateBits[0] ^ stateBits0) & 0x7000700) != 0 )
    {
        if ( r_logFile->current.integer )
            RB_LogPrintState_0(stateBits0, changedBits);
        if ( (!dx.d3d9 || !dx.device)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp",
                        2043,
                        0,
                        "%s",
                        "dx.d3d9 && dx.device") )
        {
            __debugbreak();
        }
        device = state->prim.device;
        if ( !device
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp", 2046, 0, "%s", "device") )
        {
            __debugbreak();
        }
        if ( (changedBits & 0x18000000) != 0 )
            R_HW_SetColorMask(device, stateBits0);
        if ( (state->stateOverride & 0x40000000) == 0 )
        {
            if ( (changedBits & 0x800) != 0 )
                R_HW_SetAlphaTestEnable(state->prim.device, stateBits0);
            if ( (stateBits0 & 0x800) != 0 )
            {
                if ( (stateBits0 & 0x3000) != 0
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp",
                                2063,
                                0,
                                "%s",
                                "(stateBits0 & GFXS0_ATEST_MASK) == 0") )
                {
                    __debugbreak();
                }
                stateBits0 |= state->activeStateBits[0] & 0x3000;
                changedBits &= 0xFFFFCFFF;
                if ( (state->activeStateBits[0] ^ stateBits0) != changedBits
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp",
                                2066,
                                0,
                                "%s",
                                "(stateBits0 ^ state->activeStateBits[0]) == changedBits") )
                {
                    __debugbreak();
                }
            }
            if ( (changedBits & 0x3000) != 0 )
                R_SetAlphaTestFunction(state, stateBits0);
        }
        if ( (changedBits & 0xC000) != 0 )
            R_HW_SetCullFace(device, stateBits0);
        if ( changedBits < 0 )
            R_HW_SetPolygonMode(device, stateBits0);
        blendOpRgbWasEnabled = (state->refStateBits[0] & 0x700) != 0;
        if ( (stateBits0 & 0x700) != 0 )
        {
            if ( (stateBits0 & 0x7000000) == 0 )
            {
                stateBits0 = stateBits0 & 0xF800FFFF | ((stateBits0 & 0x7FF) << 16);
                changedBits = changedBits & 0xF800FFFF | (state->activeStateBits[0] ^ stateBits0) & 0x7FF0000;
                if ( (state->activeStateBits[0] ^ stateBits0) != changedBits
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp",
                                2116,
                                0,
                                "%s",
                                "(stateBits0 ^ state->activeStateBits[0]) == changedBits") )
                {
                    __debugbreak();
                }
            }
            R_HW_SetBlend(device, blendOpRgbWasEnabled, changedBits, stateBits0);
        }
        else
        {
            if ( (stateBits0 & 0x7000000) != 0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp",
                            2092,
                            0,
                            "%s",
                            "(stateBits0 & GFXS0_BLENDOP_ALPHA_MASK) == (GFXS_BLENDOP_DISABLED << GFXS0_BLENDOP_ALPHA_SHIFT)") )
            {
                __debugbreak();
            }
            stateBits0 = stateBits0 & 0xF800F800 | state->activeStateBits[0] & 0x7FF07FF;
            changedBits &= 0xF800F800;
            if ( (state->activeStateBits[0] ^ stateBits0) != changedBits
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp",
                            2096,
                            0,
                            "%s",
                            "(stateBits0 ^ state->activeStateBits[0]) == changedBits") )
            {
                __debugbreak();
            }
            if ( blendOpRgbWasEnabled )
                R_HW_DisableBlend(device);
        }
        if ( gfxMetrics.hasTransparencyMsaa && r_aaAlpha->current.integer && (changedBits & 0xF00) != 0 )
            R_SetAlphaAntiAliasingState(device, stateBits0);
        state->activeStateBits[0] = stateBits0;
    }
}

void __cdecl R_HW_SetAlphaTestEnable(IDirect3DDevice9 *device, __int16 stateBits0)
{
    const char *v2; // eax
    int hr; // [esp+4h] [ebp-4h]

    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("device->SetRenderState( D3DRS_ALPHATESTENABLE, (stateBits0 & GFXS0_ATEST_DISABLE) ? 0 : 1 )\n");
    hr = device->SetRenderState(D3DRS_ALPHATESTENABLE, (stateBits0 & 0x800) == 0);
    if ( hr < 0 )
    {
        ++g_disableRendering;
        v2 = R_ErrorDescription(hr);
        Com_Error(
            ERR_FATAL,
            "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_ALPHATESTENABLE, ("
            "stateBits0 & GFXS0_ATEST_DISABLE) ? 0 : 1 ) failed: %s\n",
            393,
            v2);
    }
}

void __cdecl R_HW_SetColorMask(IDirect3DDevice9 *device, unsigned int stateBits0)
{
    const char *v2; // eax
    int hr; // [esp+4h] [ebp-8h]

    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("device->SetRenderState( D3DRS_COLORWRITEENABLE, mask )\n");
    hr = device->SetRenderState(D3DRS_COLORWRITEENABLE, ((stateBits0 & 0x8000000) != 0 ? 7 : 0) | ((stateBits0 & 0x10000000) != 0 ? 8 : 0));
    //hr = ((int (__thiscall *)(IDirect3DDevice9 *, IDirect3DDevice9 *, int, int))device->SetRenderState)(
    //             device,
    //             device,
    //             168,
    //             ((stateBits0 & 0x8000000) != 0 ? 7 : 0) | ((stateBits0 & 0x10000000) != 0 ? 8 : 0));
    if ( hr < 0 )
    {
        ++g_disableRendering;
        v2 = R_ErrorDescription(hr);
        Com_Error(
            ERR_FATAL,
            "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_COLORWRITEENABLE, "
            "mask ) failed: %s\n",
            403,
            v2);
    }
}

void __cdecl R_HW_SetCullFace(IDirect3DDevice9 *device, __int16 stateBits0)
{
    const char *v2; // eax
    int hr; // [esp+4h] [ebp-4h]

    if ( (stateBits0 & 0xC000) != 0x4000
        && (stateBits0 & 0xC000) != 0xC000
        && (stateBits0 & 0xC000) != 0x8000
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h",
                    409,
                    0,
                    "%s",
                    "(stateBits0 & GFXS0_CULL_MASK) == GFXS0_CULL_NONE || (stateBits0 & GFXS0_CULL_MASK) == GFXS0_CULL_FRONT || (st"
                    "ateBits0 & GFXS0_CULL_MASK) == GFXS0_CULL_BACK") )
    {
        __debugbreak();
    }
    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("device->SetRenderState( D3DRS_CULLMODE, s_cullTable[(stateBits0 & GFXS0_CULL_MASK) >> GFXS0_CULL_SHIFT] )\n");
    hr = device->SetRenderState(D3DRS_CULLMODE, s_cullTable_64[(unsigned __int16)(stateBits0 & 0xC000) >> 14]);
    if ( hr < 0 )
    {
        ++g_disableRendering;
        v2 = R_ErrorDescription(hr);
        Com_Error(
            ERR_FATAL,
            "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_CULLMODE, s_cullTa"
            "ble[(stateBits0 & GFXS0_CULL_MASK) >> GFXS0_CULL_SHIFT] ) failed: %s\n",
            410,
            v2);
    }
}

void __cdecl R_HW_SetPolygonMode(IDirect3DDevice9 *device, signed int stateBits0)
{
    const char *v2; // eax
    int hr; // [esp+4h] [ebp-4h]

    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint(
            "device->SetRenderState( D3DRS_FILLMODE, (stateBits0 & GFXS0_POLYMODE_LINE) ? D3DFILL_WIREFRAME : D3DFILL_SOLID )\n");
    hr = device->SetRenderState(D3DRS_FILLMODE, 3 - (stateBits0 < 0));
    if ( hr < 0 )
    {
        ++g_disableRendering;
        v2 = R_ErrorDescription(hr);
        Com_Error(
            ERR_FATAL,
            "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_FILLMODE, (stateBi"
            "ts0 & GFXS0_POLYMODE_LINE) ? D3DFILL_WIREFRAME : D3DFILL_SOLID ) failed: %s\n",
            416,
            v2);
    }
}

void __cdecl R_HW_DisableBlend(IDirect3DDevice9 *device)
{
    const char *v1; // eax
    int hr; // [esp+4h] [ebp-4h]

    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("device->SetRenderState( D3DRS_ALPHABLENDENABLE, 0 )\n");

    hr = device->SetRenderState(D3DRS_ALPHABLENDENABLE, 0);
    //hr = ((int (__thiscall *)(IDirect3DDevice9 *, IDirect3DDevice9 *, int, unsigned int))device->SetRenderState)(
    //             device,
    //             device,
    //             27,
    //             0);
    if ( hr < 0 )
    {
        ++g_disableRendering;
        v1 = R_ErrorDescription(hr);
        Com_Error(
            ERR_FATAL,
            "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_ALPHABLENDENABLE, "
            "0 ) failed: %s\n",
            423,
            v1);
    }
}

void __cdecl R_HW_SetBlend(
                IDirect3DDevice9 *device,
                bool blendWasEnabled,
                unsigned int changedBits,
                unsigned int stateBits0)
{
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    const char *v9; // eax
    const char *v10; // eax
    int v12; // [esp+4h] [ebp-34h]
    int v14; // [esp+Ch] [ebp-2Ch]
    int v16; // [esp+14h] [ebp-24h]
    int v18; // [esp+1Ch] [ebp-1Ch]
    int v20; // [esp+24h] [ebp-14h]
    int v22; // [esp+2Ch] [ebp-Ch]
    int hr; // [esp+34h] [ebp-4h]

    if ( !blendWasEnabled )
    {
        if ( r_logFile && r_logFile->current.integer )
            RB_LogPrint("device->SetRenderState( D3DRS_ALPHABLENDENABLE, 1 )\n");
        hr = device->SetRenderState(D3DRS_ALPHABLENDENABLE, 1u);
        if ( hr < 0 )
        {
            ++g_disableRendering;
            v4 = R_ErrorDescription(hr);
            Com_Error(
                ERR_FATAL,
                "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_ALPHABLENDENABLE"
                ", 1 ) failed: %s\n",
                449,
                v4);
        }
    }
    if ( (changedBits & 0x700) != 0 )
    {
        if ( r_logFile && r_logFile->current.integer )
            RB_LogPrint(
                "device->SetRenderState( D3DRS_BLENDOP, s_blendOpTable[(stateBits0 >> GFXS0_BLENDOP_RGB_SHIFT) & GFXS_BLENDOP_MASK] )\n");
        v22 = device->SetRenderState(D3DRS_BLENDOP, s_blendOpTable_64[(stateBits0 >> 8) & 7]);
        //v22 = ((int (__thiscall *)(IDirect3DDevice9 *, IDirect3DDevice9 *, int, const unsigned int))device->SetRenderState)(
        //                device,
        //                device,
        //                171,
        //                s_blendOpTable_64[(stateBits0 >> 8) & 7]);
        if ( v22 < 0 )
        {
            ++g_disableRendering;
            v5 = R_ErrorDescription(v22);
            Com_Error(
                ERR_FATAL,
                "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_BLENDOP, s_blend"
                "OpTable[(stateBits0 >> GFXS0_BLENDOP_RGB_SHIFT) & GFXS_BLENDOP_MASK] ) failed: %s\n",
                452,
                v5);
        }
    }
    if ( (changedBits & 0xF) != 0 )
    {
        if ( r_logFile && r_logFile->current.integer )
            RB_LogPrint(
                "device->SetRenderState( D3DRS_SRCBLEND, s_blendTable[(stateBits0 >> GFXS0_SRCBLEND_RGB_SHIFT) & GFXS_BLEND_MASK] )\n");
        v20 = device->SetRenderState(D3DRS_SRCBLEND, s_blendTable_64[stateBits0 & 0xF]);
        if ( v20 < 0 )
        {
            ++g_disableRendering;
            v6 = R_ErrorDescription(v20);
            Com_Error(
                ERR_FATAL,
                "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_SRCBLEND, s_blen"
                "dTable[(stateBits0 >> GFXS0_SRCBLEND_RGB_SHIFT) & GFXS_BLEND_MASK] ) failed: %s\n",
                455,
                v6);
        }
    }
    if ( (changedBits & 0xF0) != 0 )
    {
        if ( r_logFile && r_logFile->current.integer )
            RB_LogPrint(
                "device->SetRenderState( D3DRS_DESTBLEND, s_blendTable[(stateBits0 >> GFXS0_DSTBLEND_RGB_SHIFT) & GFXS_BLEND_MASK] )\n");
        v18 = device->SetRenderState(D3DRS_DESTBLEND, s_blendTable_64[(unsigned __int8)stateBits0 >> 4]);
        if ( v18 < 0 )
        {
            ++g_disableRendering;
            v7 = R_ErrorDescription(v18);
            Com_Error(
                ERR_FATAL,
                "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_DESTBLEND, s_ble"
                "ndTable[(stateBits0 >> GFXS0_DSTBLEND_RGB_SHIFT) & GFXS_BLEND_MASK] ) failed: %s\n",
                458,
                v7);
        }
    }
    if ( (changedBits & 0x7000000) != 0 )
    {
        if ( r_logFile && r_logFile->current.integer )
            RB_LogPrint(
                "device->SetRenderState( D3DRS_BLENDOPALPHA, s_blendOpTable[(stateBits0 >> GFXS0_BLENDOP_ALPHA_SHIFT) & GFXS_BLENDOP_MASK] )\n");
        v16 = device->SetRenderState(D3DRS_BLENDOPALPHA, s_blendOpTable_64[HIBYTE(stateBits0) & 7]);
        //v16 = ((int (__thiscall *)(IDirect3DDevice9 *, IDirect3DDevice9 *, int, const unsigned int))device->SetRenderState)(
        //                device,
        //                device,
        //                209,
        //                s_blendOpTable_64[HIBYTE(stateBits0) & 7]);
        if ( v16 < 0 )
        {
            ++g_disableRendering;
            v8 = R_ErrorDescription(v16);
            Com_Error(
                ERR_FATAL,
                "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_BLENDOPALPHA, s_"
                "blendOpTable[(stateBits0 >> GFXS0_BLENDOP_ALPHA_SHIFT) & GFXS_BLENDOP_MASK] ) failed: %s\n",
                461,
                v8);
        }
    }
    if ( (changedBits & 0xF0000) != 0 )
    {
        if ( r_logFile && r_logFile->current.integer )
            RB_LogPrint(
                "device->SetRenderState( D3DRS_SRCBLENDALPHA, s_blendTable[(stateBits0 >> GFXS0_SRCBLEND_ALPHA_SHIFT) & GFXS_BLEND_MASK] )\n");
        v14 = device->SetRenderState(D3DRS_SRCBLENDALPHA, s_blendTable_64[HIWORD(stateBits0) & 0xF]);
        if ( v14 < 0 )
        {
            ++g_disableRendering;
            v9 = R_ErrorDescription(v14);
            Com_Error(
                ERR_FATAL,
                "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_SRCBLENDALPHA, s"
                "_blendTable[(stateBits0 >> GFXS0_SRCBLEND_ALPHA_SHIFT) & GFXS_BLEND_MASK] ) failed: %s\n",
                464,
                v9);
        }
    }
    if ((changedBits & 0xF00000) != 0)
    {
        if ( r_logFile && r_logFile->current.integer )
            RB_LogPrint(
                "device->SetRenderState( D3DRS_DESTBLENDALPHA, s_blendTable[(stateBits0 >> GFXS0_DSTBLEND_ALPHA_SHIFT) & GFXS_BLEND_MASK] )\n");
        v12 = device->SetRenderState(D3DRS_DESTBLENDALPHA, s_blendTable_64[(stateBits0 >> 20) & 0xF]);
        if ( v12 < 0 )
        {
            ++g_disableRendering;
            v10 = R_ErrorDescription(v12);
            Com_Error(
                ERR_FATAL,
                "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_DESTBLENDALPHA, "
                "s_blendTable[(stateBits0 >> GFXS0_DSTBLEND_ALPHA_SHIFT) & GFXS_BLEND_MASK] ) failed: %s\n",
                467,
                v10);
        }
    }
}

void __cdecl R_SetAlphaTestFunction(GfxCmdBufState *state, __int16 stateBits0)
{
    const char *v2; // eax
    const char *v3; // eax
    int v5; // [esp+4h] [ebp-18h]
    int hr; // [esp+Ch] [ebp-10h]
    unsigned __int8 ref; // [esp+13h] [ebp-9h]
    IDirect3DDevice9 *device; // [esp+14h] [ebp-8h]
    unsigned int function; // [esp+18h] [ebp-4h]

    if ( (stateBits0 & 0x3000) == 0x1000 )
    {
        function = 5;
        ref = 0;
    }
    else if ( (stateBits0 & 0x3000) == 0x2000 )
    {
        function = 7;
        ref = -1;
    }
    else
    {
        if ( (stateBits0 & 0x3000) != 0x3000
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.h",
                        1990,
                        1,
                        "%s",
                        "(stateBits0 & GFXS0_ATEST_MASK) == GFXS0_ATEST_GE_128") )
        {
            __debugbreak();
        }
        function = 7;
        ref = 0x80;
    }
    device = state->prim.device;
    if ( !device && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.h", 1996, 0, "%s", "device") )
        __debugbreak();
    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("device->SetRenderState( D3DRS_ALPHAFUNC, function )\n");
    hr = device->SetRenderState(D3DRS_ALPHAFUNC, function);
    if ( hr < 0 )
    {
        ++g_disableRendering;
        v2 = R_ErrorDescription(hr);
        Com_Error(
            ERR_FATAL,
            "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.h (%i) device->SetRenderState( D3DRS_ALPHAFUNC, function ) failed: %s\n",
            1998,
            v2);
    }
    if ( state->alphaRef != ref )
    {
        if ( r_logFile && r_logFile->current.integer )
            RB_LogPrint("device->SetRenderState( D3DRS_ALPHAREF, ref )\n");
        v5 = device->SetRenderState(D3DRS_ALPHAREF, ref);
        if ( v5 < 0 )
        {
            ++g_disableRendering;
            v3 = R_ErrorDescription(v5);
            Com_Error(
                ERR_FATAL,
                "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.h (%i) device->SetRenderState( D3DRS_ALPHAREF, ref ) failed: %s\n",
                2001,
                v3);
        }
        state->alphaRef = ref;
    }
}

void __cdecl R_ChangeState_1(GfxCmdBufState *state, unsigned int stateBits1)
{
    int changedBits; // [esp+44h] [ebp-Ch]
    IDirect3DDevice9 *device; // [esp+4Ch] [ebp-4h]

    changedBits = state->activeStateBits[1] ^ stateBits1;
    if ( changedBits )
    {
        if ( !(stateBits1 & 0x40 | ((stateBits1 & 0x80) == 0))
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp",
                        2146,
                        0,
                        "%s",
                        "!(stateBits1 & GFXS1_STENCIL_BACK_ENABLE) | (stateBits1 & GFXS1_STENCIL_FRONT_ENABLE)") )
        {
            __debugbreak();
        }
        if ( r_logFile->current.integer )
            RB_LogPrintState_1(stateBits1, changedBits);
        if ( (!dx.d3d9 || !dx.device)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp",
                        2153,
                        0,
                        "%s",
                        "dx.d3d9 && dx.device") )
        {
            __debugbreak();
        }
        device = state->prim.device;
        if ( !device
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp", 2156, 0, "%s", "device") )
        {
            __debugbreak();
        }
        if ( state->stateOverride >= 0 )
        {
            if ( (changedBits & 1) != 0 )
                R_HW_SetDepthWriteEnable(device, stateBits1);
            if ( (changedBits & 2) != 0 )
                R_HW_SetDepthTestEnable(device, stateBits1);
            if ( (stateBits1 & 2) != 0 )
            {
                if ( (stateBits1 & 0xC) != 0
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp",
                                2167,
                                0,
                                "%s",
                                "(stateBits1 & GFXS1_DEPTHTEST_MASK) == 0") )
                {
                    __debugbreak();
                }
                stateBits1 |= state->activeStateBits[1] & 0xC;
                changedBits &= 0xFFFFFFF3;
                if ( (state->activeStateBits[1] ^ stateBits1) != changedBits
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp",
                                2170,
                                0,
                                "%s",
                                "(stateBits1 ^ state->activeStateBits[1]) == changedBits") )
                {
                    __debugbreak();
                }
            }
            if ( (changedBits & 0xC) != 0 )
                R_HW_SetDepthTestFunction(device, stateBits1);
        }
        if ( (changedBits & 0x30) != 0 )
            R_ForceSetPolygonOffset(device, stateBits1);
        if ( (stateBits1 & 0x40) != 0 )
        {
            if ( (changedBits & 0x40) != 0 )
                R_HW_EnableStencil(device);
        }
        else
        {
            if ( (changedBits & 0x40) != 0 )
                R_HW_DisableStencil(device);
            stateBits1 = stateBits1 & 0x7F | state->activeStateBits[1] & 0xFFFFFF80;
            changedBits &= 0x7Fu;
        }
        if ( (stateBits1 & 0x80) == 0 )
        {
            stateBits1 = stateBits1 & 0xFFFFF | ((stateBits1 & 0xFFF00) << 12);
            changedBits = state->activeStateBits[1] ^ stateBits1;
        }
        if ( (changedBits & 0x1FF00) != 0 )
            R_HW_SetFrontStencilOp(device, (stateBits1 >> 8) & 7, (stateBits1 >> 11) & 7, (stateBits1 >> 14) & 7);
        if ((changedBits & 0x1FF00000) != 0)
            R_HW_SetBackStencilOp(device, (stateBits1 >> 20) & 7, (stateBits1 >> 23) & 7, (stateBits1 >> 26) & 7);
        if ( (changedBits & 0xE0000) != 0 )
            R_HW_SetFrontStencilFunc(device, (stateBits1 >> 17) & 7);
        if ( (changedBits & 0xE0000000) != 0 )
            R_HW_SetBackStencilFunc(device, stateBits1 >> 29);
        state->activeStateBits[1] = stateBits1;
    }
}

void __cdecl R_HW_SetDepthWriteEnable(IDirect3DDevice9 *device, char stateBits1)
{
    const char *v2; // eax
    int hr; // [esp+4h] [ebp-4h]

    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("device->SetRenderState(D3DRS_ZWRITEENABLE, (stateBits1 & GFXS1_DEPTHWRITE) ? 1 : 0)\n");
    hr = device->SetRenderState(D3DRS_ZWRITEENABLE, (stateBits1 & 1) != 0);
    if ( hr < 0 )
    {
        ++g_disableRendering;
        v2 = R_ErrorDescription(hr);
        Com_Error(
            ERR_FATAL,
            "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState(D3DRS_ZWRITEENABLE, (stat"
            "eBits1 & GFXS1_DEPTHWRITE) ? 1 : 0) failed: %s\n",
            599,
            v2);
    }
}

void __cdecl R_HW_SetDepthTestEnable(IDirect3DDevice9 *device, char stateBits1)
{
    const char *v2; // eax
    int hr; // [esp+4h] [ebp-4h]

    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("device->SetRenderState(D3DRS_ZENABLE, (stateBits1 & GFXS1_DEPTHTEST_DISABLE) ? D3DZB_FALSE : D3DZB_TRUE)\n");
    hr = device->SetRenderState(D3DRS_ZENABLE, (stateBits1 & 2) == 0);
    if ( hr < 0 )
    {
        ++g_disableRendering;
        v2 = R_ErrorDescription(hr);
        Com_Error(
            ERR_FATAL,
            "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState(D3DRS_ZENABLE, (stateBits"
            "1 & GFXS1_DEPTHTEST_DISABLE) ? D3DZB_FALSE : D3DZB_TRUE) failed: %s\n",
            605,
            v2);
    }
}

void __cdecl R_HW_SetDepthTestFunction(IDirect3DDevice9 *device, char stateBits1)
{
    const char *v2; // eax
    int hr; // [esp+4h] [ebp-4h]

    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint(
            "device->SetRenderState( D3DRS_ZFUNC, s_depthTestTable[(stateBits1 & GFXS1_DEPTHTEST_MASK) >> GFXS1_DEPTHTEST_SHIFT] )\n");
    hr = device->SetRenderState(D3DRS_ZFUNC, s_depthTestTable_64[(unsigned __int8)(stateBits1 & 0xC) >> 2]);
    if ( hr < 0 )
    {
        ++g_disableRendering;
        v2 = R_ErrorDescription(hr);
        Com_Error(
            ERR_FATAL,
            "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_ZFUNC, s_depthTest"
            "Table[(stateBits1 & GFXS1_DEPTHTEST_MASK) >> GFXS1_DEPTHTEST_SHIFT] ) failed: %s\n",
            611,
            v2);
    }
}

void __cdecl R_HW_EnableStencil(IDirect3DDevice9 *device)
{
    const char *v1; // eax
    int hr; // [esp+4h] [ebp-4h]

    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("device->SetRenderState( D3DRS_STENCILENABLE, 1 )\n");
    hr = device->SetRenderState(D3DRS_STENCILENABLE, 1);
    //hr = ((int (__thiscall *)(IDirect3DDevice9 *, IDirect3DDevice9 *, int, int))device->SetRenderState)(
    //             device,
    //             device,
    //             52,
    //             1);
    if ( hr < 0 )
    {
        ++g_disableRendering;
        v1 = R_ErrorDescription(hr);
        Com_Error(
            ERR_FATAL,
            "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_STENCILENABLE, 1 ) failed: %s\n",
            617,
            v1);
    }
}

void __cdecl R_HW_DisableStencil(IDirect3DDevice9 *device)
{
    const char *v1; // eax
    int hr; // [esp+4h] [ebp-4h]

    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("device->SetRenderState( D3DRS_STENCILENABLE, 0 )\n");

    hr = device->SetRenderState(D3DRS_STENCILENABLE, 0);
    //hr = ((int (__thiscall *)(IDirect3DDevice9 *, IDirect3DDevice9 *, int, unsigned int))device->SetRenderState)(
    //             device,
    //             device,
    //             52,
    //             0);
    if ( hr < 0 )
    {
        ++g_disableRendering;
        v1 = R_ErrorDescription(hr);
        Com_Error(
            ERR_FATAL,
            "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_STENCILENABLE, 0 ) failed: %s\n",
            623,
            v1);
    }
}

void __cdecl R_HW_SetFrontStencilOp(
                IDirect3DDevice9 *device,
                unsigned int stencilOpPass,
                unsigned int stencilOpFail,
                unsigned int stencilOpZFail)
{
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    int v8; // [esp+4h] [ebp-14h]
    int v10; // [esp+Ch] [ebp-Ch]
    int hr; // [esp+14h] [ebp-4h]

    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("device->SetRenderState( D3DRS_STENCILPASS, s_stencilOpTable[stencilOpPass] )\n");
    hr = device->SetRenderState(D3DRS_STENCILPASS, s_stencilOpTable_64[stencilOpPass]);
    if ( hr < 0 )
    {
        ++g_disableRendering;
        v4 = R_ErrorDescription(hr);
        Com_Error(
            ERR_FATAL,
            "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_STENCILPASS, s_ste"
            "ncilOpTable[stencilOpPass] ) failed: %s\n",
            634,
            v4);
    }
    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("device->SetRenderState( D3DRS_STENCILFAIL, s_stencilOpTable[stencilOpFail] )\n");

    v10 = device->SetRenderState(D3DRS_STENCILFAIL, s_stencilOpTable_64[stencilOpFail]);
    //v10 = ((int (__thiscall *)(IDirect3DDevice9 *, IDirect3DDevice9 *, int, const unsigned int))device->SetRenderState)(
    //                device,
    //                device,
    //                53,
    //                s_stencilOpTable_64[stencilOpFail]);
    if ( v10 < 0 )
    {
        ++g_disableRendering;
        v5 = R_ErrorDescription(v10);
        Com_Error(
            ERR_FATAL,
            "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_STENCILFAIL, s_ste"
            "ncilOpTable[stencilOpFail] ) failed: %s\n",
            635,
            v5);
    }
    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("device->SetRenderState( D3DRS_STENCILZFAIL, s_stencilOpTable[stencilOpZFail] )\n");
    v8 = device->SetRenderState(D3DRS_STENCILZFAIL, s_stencilOpTable_64[stencilOpZFail]);
    if ( v8 < 0 )
    {
        ++g_disableRendering;
        v6 = R_ErrorDescription(v8);
        Com_Error(
            ERR_FATAL,
            "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_STENCILZFAIL, s_st"
            "encilOpTable[stencilOpZFail] ) failed: %s\n",
            636,
            v6);
    }
}

void __cdecl R_HW_SetBackStencilOp(
                IDirect3DDevice9 *device,
                unsigned int stencilOpPass,
                unsigned int stencilOpFail,
                unsigned int stencilOpZFail)
{
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    int v8; // [esp+4h] [ebp-14h]
    int v10; // [esp+Ch] [ebp-Ch]
    int hr; // [esp+14h] [ebp-4h]

    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("device->SetRenderState( D3DRS_CCW_STENCILPASS, s_stencilOpTable[stencilOpPass] )\n");
    hr = device->SetRenderState(D3DRS_CCW_STENCILPASS, s_stencilOpTable_64[stencilOpPass]);
    if ( hr < 0 )
    {
        ++g_disableRendering;
        v4 = R_ErrorDescription(hr);
        Com_Error(
            ERR_FATAL,
            "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_CCW_STENCILPASS, s"
            "_stencilOpTable[stencilOpPass] ) failed: %s\n",
            642,
            v4);
    }
    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("device->SetRenderState( D3DRS_CCW_STENCILFAIL, s_stencilOpTable[stencilOpFail] )\n");
    v10 = device->SetRenderState(D3DRS_CCW_STENCILFAIL, s_stencilOpTable_64[stencilOpFail]);
    //v10 = ((int (__thiscall *)(IDirect3DDevice9 *, IDirect3DDevice9 *, int, const unsigned int))device->SetRenderState)(
    //                device,
    //                device,
    //                186,
    //                s_stencilOpTable_64[stencilOpFail]);
    if ( v10 < 0 )
    {
        ++g_disableRendering;
        v5 = R_ErrorDescription(v10);
        Com_Error(
            ERR_FATAL,
            "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_CCW_STENCILFAIL, s"
            "_stencilOpTable[stencilOpFail] ) failed: %s\n",
            643,
            v5);
    }
    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("device->SetRenderState( D3DRS_CCW_STENCILZFAIL, s_stencilOpTable[stencilOpZFail] )\n");
    v8 = device->SetRenderState(D3DRS_CCW_STENCILZFAIL, s_stencilOpTable_64[stencilOpZFail]);
    if ( v8 < 0 )
    {
        ++g_disableRendering;
        v6 = R_ErrorDescription(v8);
        Com_Error(
            ERR_FATAL,
            "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_CCW_STENCILZFAIL, "
            "s_stencilOpTable[stencilOpZFail] ) failed: %s\n",
            644,
            v6);
    }
}

void __cdecl R_HW_SetFrontStencilFunc(IDirect3DDevice9 *device, unsigned int stencilFunc)
{
    const char *v2; // eax
    int hr; // [esp+4h] [ebp-4h]

    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("device->SetRenderState( D3DRS_STENCILFUNC, s_stencilFuncTable[stencilFunc] )\n");
    hr = device->SetRenderState(D3DRS_STENCILFUNC, s_stencilFuncTable_64[stencilFunc]);
    if ( hr < 0 )
    {
        ++g_disableRendering;
        v2 = R_ErrorDescription(hr);
        Com_Error(
            ERR_FATAL,
            "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_STENCILFUNC, s_ste"
            "ncilFuncTable[stencilFunc] ) failed: %s\n",
            650,
            v2);
    }
}

void __cdecl R_HW_SetBackStencilFunc(IDirect3DDevice9 *device, unsigned int stencilFunc)
{
    const char *v2; // eax
    int hr; // [esp+4h] [ebp-4h]

    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("device->SetRenderState( D3DRS_CCW_STENCILFUNC, s_stencilFuncTable[stencilFunc] )\n");
    hr = device->SetRenderState(D3DRS_CCW_STENCILFUNC, s_stencilFuncTable_64[stencilFunc]);
    if ( hr < 0 )
    {
        ++g_disableRendering;
        v2 = R_ErrorDescription(hr);
        Com_Error(
            ERR_FATAL,
            "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_CCW_STENCILFUNC, s"
            "_stencilFuncTable[stencilFunc] ) failed: %s\n",
            656,
            v2);
    }
}

void __cdecl R_ForceSetPolygonOffset(IDirect3DDevice9 *device, char stateBits1)
{
    __int64 v2; // [esp+10h] [ebp-2Ch]
    unsigned int offset; // [esp+30h] [ebp-Ch]
    float bias; // [esp+34h] [ebp-8h]
    float scale; // [esp+38h] [ebp-4h]

    offset = stateBits1 & 0x30;
    if ( offset == 48 )
    {
        bias = sm_polygonOffsetBias->current.value * 0.000015258789;
        scale = sm_polygonOffsetScale->current.value;
    }
    else
    {
        v2 = offset >> 4;
        bias = (double)v2 * r_polygonOffsetBias->current.value * 0.000015258789;
        scale = (double)v2 * r_polygonOffsetScale->current.value;
    }
    R_HW_SetPolygonOffset(device, scale, bias);
}

void __cdecl R_HW_SetPolygonOffset(IDirect3DDevice9 *device, float scale, float bias)
{
    const char *v3; // eax
    const char *v4; // eax
    int v6; // [esp+Ch] [ebp-Ch]
    int hr; // [esp+14h] [ebp-4h]

    if ( gfxMetrics.slopeScaleDepthBias )
    {
        if ( r_logFile && r_logFile->current.integer )
            RB_LogPrint("device->SetRenderState( D3DRS_SLOPESCALEDEPTHBIAS, FloatAsInt( &scale ) )\n");

        hr = device->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, LODWORD(scale));
        //hr = ((int (__thiscall *)(IDirect3DDevice9 *, IDirect3DDevice9 *, int, unsigned int))device->SetRenderState)(
        //             device,
        //             device,
        //             175,
        //             LODWORD(scale));
        if ( hr < 0 )
        {
            ++g_disableRendering;
            v3 = R_ErrorDescription(hr);
            Com_Error(
                ERR_FATAL,
                "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_SLOPESCALEDEPTHB"
                "IAS, FloatAsInt( &scale ) ) failed: %s\n",
                677,
                v3);
        }
    }
    else
    {
        bias = bias * 2.0;
    }
    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("device->SetRenderState( D3DRS_DEPTHBIAS, FloatAsInt( &bias ) )\n");
    v6 = device->SetRenderState(D3DRS_DEPTHBIAS, LODWORD(bias));
    if ( v6 < 0 )
    {
        ++g_disableRendering;
        v4 = R_ErrorDescription(v6);
        Com_Error(
            ERR_FATAL,
            "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetRenderState( D3DRS_DEPTHBIAS, FloatAs"
            "Int( &bias ) ) failed: %s\n",
            681,
            v4);
    }
}

void __cdecl R_SetTextureSamplerCodeImageRenderTarget(
                GfxCmdBufContext context,
                unsigned int samplerIndex,
                GfxCodeImageRenderTarget textureControl)
{
    unsigned int decodedSamplerState; // [esp+8h] [ebp-4h]

    context.state->samplerTexture[samplerIndex] = 0;
    decodedSamplerState = R_DecodeSamplerState(*(_QWORD *)&textureControl.fields >> 12);
    if ( context.state->samplerState[samplerIndex] != decodedSamplerState )
        context.state->samplerState[samplerIndex] = R_HW_SetSamplerState(
                                                                                                    context.state->prim.device,
                                                                                                    samplerIndex,
                                                                                                    decodedSamplerState,
                                                                                                    context.state->samplerState[samplerIndex]);
}

unsigned int __cdecl R_HW_SetSamplerState(
                IDirect3DDevice9 *device,
                unsigned int samplerIndex,
                unsigned int samplerState,
                unsigned int oldSamplerState)
{
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    const char *v9; // eax
    const char *v10; // eax
    int v13; // [esp+4h] [ebp-50h]
    int v15; // [esp+Ch] [ebp-48h]
    int v17; // [esp+14h] [ebp-40h]
    int v19; // [esp+1Ch] [ebp-38h]
    int v21; // [esp+24h] [ebp-30h]
    int v23; // [esp+2Ch] [ebp-28h]
    int hr; // [esp+34h] [ebp-20h]
    unsigned int finalSamplerState; // [esp+38h] [ebp-1Ch]
    unsigned int diffSamplerState; // [esp+48h] [ebp-Ch]

    finalSamplerState = samplerState;
    diffSamplerState = oldSamplerState ^ samplerState;
    if ( oldSamplerState == samplerState
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h",
                    199,
                    0,
                    "%s",
                    "diffSamplerState") )
    {
        __debugbreak();
    }
    if ( (diffSamplerState & 0xF00) != 0 )
    {
        if ( r_logFile && r_logFile->current.integer )
            RB_LogPrint("device->SetSamplerState( samplerIndex, D3DSAMP_MINFILTER, minFilter )\n");
        hr = device->SetSamplerState(samplerIndex, D3DSAMP_MINFILTER, (unsigned __int16)(samplerState & 0xF00) >> 8);
        if ( hr < 0 )
        {
            ++g_disableRendering;
            v4 = R_ErrorDescription(hr);
            Com_Error(
                ERR_FATAL,
                "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetSamplerState( samplerIndex, D3DSAMP"
                "_MINFILTER, minFilter ) failed: %s\n",
                204,
                v4);
        }
    }
    if ( (diffSamplerState & 0xF000) != 0 )
    {
        if ( r_logFile && r_logFile->current.integer )
            RB_LogPrint("device->SetSamplerState( samplerIndex, D3DSAMP_MAGFILTER, magFilter )\n");
        v23 = device->SetSamplerState(
                        samplerIndex,
                        D3DSAMP_MAGFILTER,
                        (unsigned __int16)(samplerState & 0xF000) >> 12);
        if ( v23 < 0 )
        {
            ++g_disableRendering;
            v5 = R_ErrorDescription(v23);
            Com_Error(
                ERR_FATAL,
                "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetSamplerState( samplerIndex, D3DSAMP"
                "_MAGFILTER, magFilter ) failed: %s\n",
                210,
                v5);
        }
    }
    if ( (_BYTE)diffSamplerState )
    {
        if ( (unsigned __int8)samplerState <= 1u )
        {
            finalSamplerState = (unsigned __int8)oldSamplerState | samplerState & 0xFFFFFF00;
        }
        else
        {
            if ( r_logFile && r_logFile->current.integer )
                RB_LogPrint("device->SetSamplerState( samplerIndex, D3DSAMP_MAXANISOTROPY, anisotropy )\n");
            v21 = device->SetSamplerState(samplerIndex, D3DSAMP_MAXANISOTROPY, (unsigned __int8)samplerState);
            if ( v21 < 0 )
            {
                ++g_disableRendering;
                v6 = R_ErrorDescription(v21);
                Com_Error(
                    ERR_FATAL,
                    "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetSamplerState( samplerIndex, D3DSA"
                    "MP_MAXANISOTROPY, anisotropy ) failed: %s\n",
                    217,
                    v6);
            }
        }
    }
    if ( (diffSamplerState & 0xF0000) != 0 )
    {
        if ( r_logFile && r_logFile->current.integer )
            RB_LogPrint("device->SetSamplerState( samplerIndex, D3DSAMP_MIPFILTER, mipFilter )\n");
        v19 = device->SetSamplerState(samplerIndex, D3DSAMP_MIPFILTER, (samplerState & 0xF0000) >> 16);
        if ( v19 < 0 )
        {
            ++g_disableRendering;
            v7 = R_ErrorDescription(v19);
            Com_Error(
                ERR_FATAL,
                "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetSamplerState( samplerIndex, D3DSAMP"
                "_MIPFILTER, mipFilter ) failed: %s\n",
                225,
                v7);
        }
    }
    if ( (0x3F00000 & diffSamplerState) != 0 )
    {
        if ( (diffSamplerState & 0x300000) != 0 )
        {
            if ( r_logFile && r_logFile->current.integer )
                RB_LogPrint("device->SetSamplerState( samplerIndex, D3DSAMP_ADDRESSU, address )\n");
            v17 = device->SetSamplerState(samplerIndex, D3DSAMP_ADDRESSU, (samplerState & 0x300000) >> 20);
            if ( v17 < 0 )
            {
                ++g_disableRendering;
                v8 = R_ErrorDescription(v17);
                Com_Error(
                    ERR_FATAL,
                    "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetSamplerState( samplerIndex, D3DSA"
                    "MP_ADDRESSU, address ) failed: %s\n",
                    233,
                    v8);
            }
        }
        if ( (0xC00000 & diffSamplerState) != 0 )
        {
            if ( r_logFile && r_logFile->current.integer )
                RB_LogPrint("device->SetSamplerState( samplerIndex, D3DSAMP_ADDRESSV, address )\n");
            v15 = device->SetSamplerState(
                            samplerIndex,
                            D3DSAMP_ADDRESSV,
                            (0xC00000 & samplerState) >> 22);
            if ( v15 < 0 )
            {
                ++g_disableRendering;
                v9 = R_ErrorDescription(v15);
                Com_Error(
                    ERR_FATAL,
                    "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetSamplerState( samplerIndex, D3DSA"
                    "MP_ADDRESSV, address ) failed: %s\n",
                    238,
                    v9);
            }
        }
        if ( (diffSamplerState & 0x3000000) != 0 )
        {
            if ( r_logFile && r_logFile->current.integer )
                RB_LogPrint("device->SetSamplerState( samplerIndex, D3DSAMP_ADDRESSW, address )\n");
            v13 = device->SetSamplerState(samplerIndex, D3DSAMP_ADDRESSW, (samplerState & 0x3000000) >> 24);
            if ( v13 < 0 )
            {
                ++g_disableRendering;
                v10 = R_ErrorDescription(v13);
                Com_Error(
                    ERR_FATAL,
                    "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetSamplerState( samplerIndex, D3DSA"
                    "MP_ADDRESSW, address ) failed: %s\n",
                    243,
                    v10);
            }
        }
    }
    return finalSamplerState;
}

void __cdecl R_SetSampler(
                GfxCmdBufContext context,
                unsigned int samplerIndex,
                unsigned __int8 samplerState,
                const GfxImage *image)
{
    const char *v4; // eax
    unsigned int decodedSamplerState; // [esp+10h] [ebp-8h]

    if ( !image && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp", 2320, 0, "%s", "image") )
        __debugbreak();
    if ( (const GfxImage *)context.state->samplerTexture[samplerIndex] != image )
    {
        context.state->samplerTexture[samplerIndex] = &image->texture;
        if ( r_logFile->current.integer )
        {
            v4 = va("---------- texture %i: %s\n", samplerIndex, image->name);
            RB_LogPrint(v4);
        }
        R_HW_SetSamplerTexture(context.state->prim.device, samplerIndex, &image->texture);
    }
    if ( (samplerState & 0x1F) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp",
                    2342,
                    0,
                    "%s\n\t(samplerState) = %i",
                    "((samplerState & (SAMPLER_FILTER_MASK | SAMPLER_MIPMAP_MASK)) != 0)",
                    samplerState) )
    {
        __debugbreak();
    }
    if ( context.state->refSamplerState[samplerIndex] != samplerState )
    {
        context.state->refSamplerState[samplerIndex] = samplerState;
        decodedSamplerState = R_DecodeSamplerState(samplerState);
        if ( context.state->samplerState[samplerIndex] != decodedSamplerState )
            context.state->samplerState[samplerIndex] = R_HW_SetSamplerState(
                                                                                                        context.state->prim.device,
                                                                                                        samplerIndex,
                                                                                                        decodedSamplerState,
                                                                                                        context.state->samplerState[samplerIndex]);
    }
}

void __cdecl R_HW_SetSamplerTexture(IDirect3DDevice9 *device, unsigned int samplerIndex, const GfxTexture *texture)
{
    const char *v3; // eax
    int hr; // [esp+4h] [ebp-4h]

    if ( !texture
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h", 173, 0, "%s", "texture") )
    {
        __debugbreak();
    }
    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("device->SetTexture( samplerIndex, texture->basemap )\n");
    hr = device->SetTexture(samplerIndex, texture->basemap);
    if ( hr < 0 )
    {
        ++g_disableRendering;
        v3 = R_ErrorDescription(hr);
        Com_Error(
            ERR_FATAL,
            "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetTexture( samplerIndex, texture->basem"
            "ap ) failed: %s\n",
            177,
            v3);
    }
}

void __cdecl R_SetSamplerState(GfxCmdBufState *state, unsigned int samplerIndex, unsigned __int8 samplerState)
{
    unsigned int decodedSamplerState; // [esp+8h] [ebp-4h]

    state->samplerTexture[samplerIndex] = 0;
    if ( (samplerState & 0x1F) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp",
                    2401,
                    0,
                    "%s\n\t(samplerState) = %i",
                    "((samplerState & (SAMPLER_FILTER_MASK | SAMPLER_MIPMAP_MASK)) != 0)",
                    samplerState) )
    {
        __debugbreak();
    }
    if ( state->refSamplerState[samplerIndex] != samplerState )
    {
        state->refSamplerState[samplerIndex] = samplerState;
        decodedSamplerState = R_DecodeSamplerState(samplerState);
        if ( state->samplerState[samplerIndex] != decodedSamplerState )
            state->samplerState[samplerIndex] = R_HW_SetSamplerState(
                                                                                        state->prim.device,
                                                                                        samplerIndex,
                                                                                        decodedSamplerState,
                                                                                        state->samplerState[samplerIndex]);
    }
}

void __cdecl R_TextureOverride(
                const GfxBackEndData *data,
                GfxCmdBufContext context,
                char modelIndex,
                int textureOverride)
{
    signed int i; // [esp+8h] [ebp-18h]
    const Material *material; // [esp+Ch] [ebp-14h]

    material = context.state->material;
    while ( textureOverride >= 0 )
    {
        if ( (!data->textureOverrides[textureOverride].material
             || data->textureOverrides[textureOverride].material == material)
            && ((unsigned __int16)(1 << modelIndex) & data->textureOverrides[textureOverride].dobjModelMask) != 0
            && data->textureOverrides[textureOverride].img2 )
        {
            for ( i = 0; i < 16; ++i )
            {
                if ( context.state->samplerTexture[i] == &data->textureOverrides[textureOverride].img1->texture )
                {
                    context.state->samplerTexture[i] = &data->textureOverrides[textureOverride].img2->texture;
                    R_HW_SetSamplerTexture(context.state->prim.device, i, &data->textureOverrides[textureOverride].img2->texture);
                }
            }
        }
        textureOverride = data->textureOverrides[textureOverride].prev;
    }
}

void __cdecl R_GetViewport(GfxCmdBufSourceState *source, GfxViewport *outViewport)
{
    int v2; // [esp+0h] [ebp-10h]
    int v3; // [esp+4h] [ebp-Ch]

    if (!source && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp", 2503, 0, "%s", "source"))
        __debugbreak();
    if (source->viewportBehavior == GFX_USE_VIEWPORT_FULL)
    {
        if (source->renderTargetWidth <= 0
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp",
                2507,
                0,
                "%s\n\t(source->renderTargetWidth) = %i",
                "(source->renderTargetWidth > 0)",
                source->renderTargetWidth))
        {
            __debugbreak();
        }
        if (source->renderTargetHeight <= 0
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp",
                2508,
                0,
                "%s\n\t(source->renderTargetHeight) = %i",
                "(source->renderTargetHeight > 0)",
                source->renderTargetHeight))
        {
            __debugbreak();
        }
        outViewport->x = 0;
        outViewport->y = 0;
        outViewport->width = source->renderTargetWidth;
        outViewport->height = source->renderTargetHeight;
    }
    else
    {
        if (source->sceneViewport.width <= 0
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp",
                2517,
                0,
                "%s\n\t(source->sceneViewport.width) = %i",
                "(source->sceneViewport.width > 0)",
                source->sceneViewport.width))
        {
            __debugbreak();
        }
        if (source->sceneViewport.height <= 0
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp",
                2518,
                0,
                "%s\n\t(source->sceneViewport.height) = %i",
                "(source->sceneViewport.height > 0)",
                source->sceneViewport.height))
        {
            __debugbreak();
        }
        *outViewport = source->sceneViewport;
        if (outViewport->width <= 0
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp",
                2521,
                0,
                "%s\n\t(outViewport->width) = %i",
                "(outViewport->width > 0)",
                outViewport->width))
        {
            __debugbreak();
        }
        if (outViewport->height <= 0
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp",
                2522,
                0,
                "%s\n\t(outViewport->height) = %i",
                "(outViewport->height > 0)",
                outViewport->height))
        {
            __debugbreak();
        }
    }
    if (source->viewMode != VIEW_MODE_2D && r_scaleViewport->current.value != 1.0)
    {
        outViewport->x += (int)(float)((float)((float)outViewport->width * (float)(1.0 - r_scaleViewport->current.value))
            * 0.5);
        outViewport->y += (int)(float)((float)((float)outViewport->height * (float)(1.0 - r_scaleViewport->current.value))
            * 0.5);
        if ((int)(float)((float)outViewport->width * r_scaleViewport->current.value) > 1)
            v3 = (int)(float)((float)outViewport->width * r_scaleViewport->current.value);
        else
            v3 = 1;
        outViewport->width = v3;
        if ((int)(float)((float)outViewport->height * r_scaleViewport->current.value) > 1)
            v2 = (int)(float)((float)outViewport->height * r_scaleViewport->current.value);
        else
            v2 = 1;
        outViewport->height = v2;
    }
}

void __cdecl R_SetViewport(GfxCmdBufState *state, const GfxViewport *viewport)
{
    const char *v2; // eax

    if ( r_logFile->current.integer )
    {
        v2 = va("Viewport at (%i, %i) with size %i x %i\n", viewport->x, viewport->y, viewport->width, viewport->height);
        RB_LogPrint(v2);
    }
    if ( viewport->x < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp",
                    2542,
                    0,
                    "%s\n\t(viewport->x) = %i",
                    "(viewport->x >= 0)",
                    viewport->x) )
    {
        __debugbreak();
    }
    if ( viewport->y < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp",
                    2543,
                    0,
                    "%s\n\t(viewport->y) = %i",
                    "(viewport->y >= 0)",
                    viewport->y) )
    {
        __debugbreak();
    }
    if ( viewport->width <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp",
                    2544,
                    0,
                    "%s\n\t(viewport->width) = %i",
                    "(viewport->width > 0)",
                    viewport->width) )
    {
        __debugbreak();
    }
    if ( viewport->height <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp",
                    2545,
                    0,
                    "%s\n\t(viewport->height) = %i",
                    "(viewport->height > 0)",
                    viewport->height) )
    {
        __debugbreak();
    }
    if ( viewport->x != state->viewport.x
        || viewport->y != state->viewport.y
        || viewport->width != state->viewport.width
        || viewport->height != state->viewport.height )
    {
        state->viewport = *viewport;
        R_HW_SetViewport(state->prim.device, viewport, state->depthRangeNear, state->depthRangeFar);
    }
}

void __cdecl R_SetViewportStruct(GfxCmdBufSourceState *source, const GfxViewport *viewport)
{
    source->sceneViewport = *viewport;
    iassert(source->sceneViewport.width > 0);
    iassert(source->sceneViewport.height > 0);

    source->viewMode = VIEW_MODE_NONE;
    source->viewportIsDirty = 1;
}

void __cdecl R_SetScissorStruct(GfxCmdBufSourceState *source, const GfxViewport *scissor)
{
    iassert(scissor->width > 0);
    iassert(scissor->height > 0);

    source->scissorViewport = *scissor;
    source->scissorViewport.x += source->sceneViewport.x;
    source->scissorViewport.y += source->sceneViewport.y;
    source->scissorEnabled = 1;
    source->viewportIsDirty = 1;
}

void __cdecl R_ClearScissorStruct(GfxCmdBufSourceState *source)
{
    source->scissorEnabled = 0;
    source->viewportIsDirty = 1;
}

char __cdecl R_GetScissor(GfxCmdBufSourceState *source, GfxViewport *outScissor)
{
    iassert(source);

    if (!source->scissorEnabled)
        return 0;

    *outScissor = source->scissorViewport;
    return 1;
}

void R_SetScissor(GfxCmdBufState *state, const GfxViewport *_scissor)
{
    char *v3; // eax
    GfxViewport scissor; // [esp+20h] [ebp-10h]

    scissor = *_scissor;
    if (r_logFile->current.integer)
    {
        v3 = va("Scissor at (%i, %i) with size %i x %i\n", scissor.x, scissor.y, scissor.width, scissor.height);
        RB_LogPrint(v3);
    }
    if (scissor.x < 0)
    {
        scissor.width += scissor.x;
        if (scissor.width <= 0)
            scissor.width = 0;
        scissor.x = 0;
    }
    if (scissor.y < 0)
    {
        scissor.height += scissor.y;
        if (scissor.height <= 0)
            scissor.height = 0;
        scissor.y = 0;
    }
    if (scissor.x > gfxRenderTargets[state->renderTargetId].width)
    {
        scissor.x = gfxRenderTargets[state->renderTargetId].width;
        scissor.width = 0;
    }
    if (scissor.width + scissor.x > gfxRenderTargets[state->renderTargetId].width)
        scissor.width = gfxRenderTargets[state->renderTargetId].width - scissor.x;
    if (scissor.y > gfxRenderTargets[state->renderTargetId].height)
    {
        scissor.y = gfxRenderTargets[state->renderTargetId].height;
        scissor.height = 0;
    }
    if (scissor.height + scissor.y > gfxRenderTargets[state->renderTargetId].height)
        scissor.height = gfxRenderTargets[state->renderTargetId].height - scissor.y;
    if (scissor.width < 0
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp",
            2661,
            0,
            "%s\n\t(scissor.width) = %i",
            "(scissor.width >= 0)",
            scissor.width))
    {
        __debugbreak();
    }
    if (scissor.height < 0
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp",
            2662,
            0,
            "%s\n\t(scissor.height) = %i",
            "(scissor.height >= 0)",
            scissor.height))
    {
        __debugbreak();
    }
    if (!state->scissorEnabled
        || scissor.x != state->scissor.x
        || scissor.y != state->scissor.y
        || scissor.width != state->scissor.width
        || scissor.height != state->scissor.height)
    {
        state->scissor = scissor;
        state->scissorEnabled = 1;
        R_HW_EnableScissor(state->prim.device, scissor.x, scissor.y, scissor.width, scissor.height);
    }
}

void __cdecl R_ClearScissor(GfxCmdBufState *state)
{
    const char *v1; // eax

    if ( r_logFile->current.integer )
    {
        v1 = va("Clear Scissor\n");
        RB_LogPrint(v1);
    }
    if ( state->scissorEnabled )
    {
        state->scissorEnabled = 0;
        R_HW_DisableScissor(state->prim.device);
    }
}

void __cdecl R_SetViewportValues(GfxCmdBufSourceState *source, int x, int y, int width, int height)
{
    GfxViewport viewport; // [esp+0h] [ebp-10h] BYREF

    viewport.x = x;
    viewport.y = y;
    viewport.width = width;
    viewport.height = height;
    R_SetViewportStruct(source, &viewport);
}

float renderTargetHeight; // [esp+1Ch] [ebp-40h]
unsigned int lookupScale; // [esp+34h] [ebp-28h]
float lookupScale_4; // [esp+38h] [ebp-24h]
__int64 invWidth; // [esp+3Ch] [ebp-20h]
__int64 lookupOffset; // [esp+54h] [ebp-8h]



void R_UpdateViewport(GfxCmdBufSourceState *source, GfxViewport *viewport)
{
    // (kcod4)
    float lookupScale[2];
    float invWidth;
    float invHeight;
    float lookupOffset[2];

    iassert( source );
    iassert( source->viewMode != VIEW_MODE_NONE );

    source->viewportIsDirty = 0;

    iassert(source->renderTargetWidth > 0);
    iassert(source->renderTargetHeight > 0);

    invWidth = 1.0 / (double)source->renderTargetWidth;
    invHeight = 1.0 / (double)source->renderTargetHeight;

    lookupScale[0] = 0.5 * invWidth * (double)viewport->width;
    lookupScale[1] = 0.5 * invHeight * (double)viewport->height;

    lookupOffset[0] = lookupScale[0] + (invWidth * (double)viewport->x);
    lookupOffset[1] = lookupScale[1] + (invHeight * (double)viewport->y);

    lookupOffset[0] += invWidth * 0.5;
    lookupOffset[1] += invHeight * 0.5;

    R_SetCodeConstant(source,
        CONST_SRC_CODE_RENDER_TARGET_SIZE,
        (float)source->renderTargetWidth,
        (float)source->renderTargetHeight,
        invWidth,
        invHeight
    );
    R_SetCodeConstant(source,
        CONST_SRC_CODE_CLIP_SPACE_LOOKUP_SCALE,
        lookupScale[0],
        -lookupScale[1],
        0.0,
        1.0
    );
    R_SetCodeConstant(source,
        CONST_SRC_CODE_CLIP_SPACE_LOOKUP_OFFSET,
        lookupOffset[0],
        lookupOffset[1],
        0.0,
        0.0
    );
}

void __cdecl R_DisableSampler(GfxCmdBufState *state, unsigned int samplerIndex)
{
    state->samplerTexture[samplerIndex] = 0;
    R_HW_DisableSampler(state->prim.device, samplerIndex);
}

void __cdecl R_HW_DisableSampler(IDirect3DDevice9 *device, unsigned int samplerIndex)
{
    const char *v2; // eax
    int hr; // [esp+4h] [ebp-4h]

    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("device->SetTexture( samplerIndex, 0 )\n");
    hr = device->SetTexture(samplerIndex, 0);
    if ( hr < 0 )
    {
        ++g_disableRendering;
        v2 = R_ErrorDescription(hr);
        Com_Error(
            ERR_FATAL,
            "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetTexture( samplerIndex, 0 ) failed: %s\n",
            184,
            v2);
    }
}

void __cdecl UpdateVPosToWorld(GfxCmdBufSourceState *source)
{
    float v1; // [esp+10h] [ebp-60h]
    float v2; // [esp+14h] [ebp-5Ch]
    float v3; // [esp+1Ch] [ebp-54h]
    float invWidth; // [esp+20h] [ebp-50h]
    float invHeight; // [esp+24h] [ebp-4Ch]
    float vposx_to_world[4]; // [esp+28h] [ebp-48h] BYREF
    float scale1[4]; // [esp+38h] [ebp-38h] BYREF
    const float (*projM44)[4]; // [esp+48h] [ebp-28h]
    float vpos1_to_world[4]; // [esp+4Ch] [ebp-24h] BYREF
    const float (*invViewM44)[4]; // [esp+5Ch] [ebp-14h]
    float vposy_to_world[4]; // [esp+60h] [ebp-10h] BYREF

    if (source->viewMode == VIEW_MODE_3D)
    {
        invWidth = 1.0 / (float)source->renderTargetWidth;
        invHeight = 1.0 / (float)source->renderTargetHeight;
        projM44 = (const float (*)[4])R_GetCodeMatrix(source, 0xCDu, 0);
        invViewM44 = (const float (*)[4])R_GetCodeMatrix(source, 0xCBu, 0);
        v3 = (float)(-2.0 * invWidth) / (*projM44)[0];
        vposx_to_world[0] = v3 * (*invViewM44)[0];
        vposx_to_world[1] = v3 * (float)(*invViewM44)[1];
        vposx_to_world[2] = v3 * (float)(*invViewM44)[2];
        vposx_to_world[3] = v3 * (float)(*invViewM44)[3];
        v2 = (float)(2.0 * invHeight) / (float)(*projM44)[5];
        vposy_to_world[0] = v2 * (float)(*invViewM44)[4];
        vposy_to_world[1] = v2 * (float)(*invViewM44)[5];
        vposy_to_world[2] = v2 * (float)(*invViewM44)[6];
        vposy_to_world[3] = v2 * (float)(*invViewM44)[7];
        v1 = -1.0 / (float)(*projM44)[5];
        scale1[0] = 1.0 / (*projM44)[0];
        scale1[1] = v1;
        scale1[2] = -1.0f;
        scale1[3] = 0.0f;
        MatrixTransformVector44(scale1, invViewM44, vpos1_to_world);
        R_SetCodeConstantFromVec4(source, CONST_SRC_VPOSX_TO_WORLD, vposx_to_world);
        R_SetCodeConstantFromVec4(source, CONST_SRC_VPOSY_TO_WORLD, vposy_to_world);
        R_SetCodeConstantFromVec4(source, CONST_SRC_VPOS1_TO_WORLD, vpos1_to_world);
    }
}

bool g_renderTargetIsOverridden;
void R_SetRenderTargetSize(GfxCmdBufSourceState *source, GfxRenderTargetId newTargetId)
{
    const char *v3; // eax
    const char *v4; // eax
    GfxRenderTargetId actualTargetId; // [esp+3h] [ebp-1h]

    if (!source
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp", 2819, 0, "%s", "source"))
    {
        __debugbreak();
    }
    if (!gfxRenderTargets
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp",
            2820,
            0,
            "%s",
            "gfxRenderTargets"))
    {
        __debugbreak();
    }
    g_renderTargetIsOverridden = 0;
    if (pixelCostMode > GFX_PIXEL_COST_MODE_MEASURE_MSEC)
    {
        actualTargetId = RB_PixelCost_OverrideRenderTarget(newTargetId);
        g_renderTargetIsOverridden = newTargetId != actualTargetId;
        newTargetId = actualTargetId;
    }
    if (newTargetId >= 0x2Cu
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp",
            2834,
            0,
            "%s\n\t(newTargetId) = %i",
            "(newTargetId < R_RENDERTARGET_COUNT)",
            newTargetId))
    {
        __debugbreak();
    }
    source->viewportBehavior = R_ViewportBehaviorForRenderTarget(newTargetId);
    source->renderTargetWidth = gfxRenderTargets[newTargetId].width;
    source->renderTargetHeight = gfxRenderTargets[newTargetId].height;
    if (source->renderTargetWidth <= 0)
    {
        v3 = R_RenderTargetName(newTargetId);
        if (!Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp",
            2852,
            0,
            "%s\n\t(R_RenderTargetName(newTargetId)) = %s",
            "(source->renderTargetWidth > 0)",
            v3))
            __debugbreak();
    }
    if (source->renderTargetHeight <= 0)
    {
        v4 = R_RenderTargetName(newTargetId);
        if (!Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp",
            2853,
            0,
            "%s\n\t(R_RenderTargetName(newTargetId)) = %s",
            "(source->renderTargetHeight > 0)",
            v4))
            __debugbreak();
    }
    UpdateVPosToWorld(source);
}

GfxViewportBehavior __cdecl R_ViewportBehaviorForRenderTarget(GfxRenderTargetId renderTargetId)
{
    if ( !s_viewportBehaviorForRenderTarget
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp",
                    2750,
                    0,
                    "%s",
                    "s_viewportBehaviorForRenderTarget") )
    {
        __debugbreak();
    }
    if ( renderTargetId >= 0x2Cu
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp",
                    2751,
                    0,
                    "%s\n\t(renderTargetId) = %i",
                    "(renderTargetId < R_RENDERTARGET_COUNT)",
                    renderTargetId) )
    {
        __debugbreak();
    }
    return s_viewportBehaviorForRenderTarget[renderTargetId];
}

void R_SetRenderTarget(GfxCmdBufContext context, GfxRenderTargetId newTargetId)
{
    if (pixelCostMode > GFX_PIXEL_COST_MODE_MEASURE_MSEC)
        newTargetId = RB_PixelCost_OverrideRenderTarget(newTargetId);

    if (newTargetId != context.state->renderTargetId)
    {
        if (r_logFile->current.integer)
        {
            RB_LogPrint(va("\n========== R_SetRenderTarget( %s ) ==========\n\n", R_RenderTargetName(newTargetId)));
        }

        R_UpdateStatsTarget(context);

        if (gfxRenderTargets[newTargetId].image)
            R_UnbindImage(context.state, gfxRenderTargets[newTargetId].image);

        iassert(context.source->renderTargetHeight == (int)gfxRenderTargets[newTargetId].height);
        iassert(context.source->renderTargetWidth > 0);
        iassert(context.source->renderTargetHeight > 0);

        R_HW_SetRenderTarget(context.state, newTargetId);

        context.state->renderTargetId = newTargetId;
        context.source->viewMode = VIEW_MODE_NONE;
        context.source->viewportIsDirty = 1;
    }
}

void R_HW_SetRenderTarget(GfxCmdBufState *state, GfxRenderTargetId newTargetId)
{
    const char *v3; // eax
    const char *v4; // eax
    int v6; // [esp+4h] [ebp-10h]
    int hr; // [esp+Ch] [ebp-8h]
    IDirect3DDevice9 *device; // [esp+10h] [ebp-4h]

    R_PixStartNamedRenderTarget(newTargetId);
    device = state->prim.device;
    if (!device
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.h", 2296, 0, "%s", "device"))
    {
        __debugbreak();
    }
    if (gfxRenderTargets[state->renderTargetId].surface.color != gfxRenderTargets[newTargetId].surface.color)
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("device->SetRenderTarget( 0, gfxRenderTargets[newTargetId].surface.color )\n");
        hr = device->SetRenderTarget(0, gfxRenderTargets[newTargetId].surface.color);
        if (hr < 0)
        {
            ++g_disableRendering;
            v3 = R_ErrorDescription(hr);
            Com_Error(
                ERR_FATAL,
                "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.h (%i) device->SetRenderTarget( 0, gfxRenderTargets[newTarge"
                "tId].surface.color ) failed: %s\n",
                2302,
                v3);
        }
        state->viewport.x = 0;
        state->viewport.y = 0;
        state->viewport.width = gfxRenderTargets[newTargetId].width;
        state->viewport.height = gfxRenderTargets[newTargetId].height;
        state->depthRangeType = GFX_DEPTH_RANGE_FULL;
        state->depthRangeNear = 0.0f;
        state->depthRangeFar = 1.0f;
    }
    if (gfxRenderTargets[state->renderTargetId].surface.depthStencil != gfxRenderTargets[newTargetId].surface.depthStencil)
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("device->SetDepthStencilSurface( gfxRenderTargets[newTargetId].surface.depthStencil )\n");
        v6 = device->SetDepthStencilSurface(gfxRenderTargets[newTargetId].surface.depthStencil);
        //v6 = ((int(__thiscall *)(IDirect3DDevice9 *, IDirect3DDevice9 *, IDirect3DSurface9 *))device->SetDepthStencilSurface)(
        //    device,
        //    device,
        //    gfxRenderTargets[newTargetId].surface.depthStencil);
        if (v6 < 0)
        {
            ++g_disableRendering;
            v4 = R_ErrorDescription(v6);
            Com_Error(
                ERR_FATAL,
                "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.h (%i) device->SetDepthStencilSurface( gfxRenderTargets[newT"
                "argetId].surface.depthStencil ) failed: %s\n",
                2315,
                v4);
        }
    }
}

void __cdecl R_UpdateStatsTarget(GfxCmdBufContext context)
{
    context.state->prim.viewStats = context.state->prim.frameStats.viewStats;
}

void __cdecl R_UnbindImage(GfxCmdBufState *state, const GfxImage *image)
{
    unsigned int samplerIndex; // [esp+0h] [ebp-8h]
    unsigned int maxTextureMaps; // [esp+4h] [ebp-4h]

    maxTextureMaps = vidConfig.maxTextureMaps;
    for ( samplerIndex = 0; samplerIndex < maxTextureMaps; ++samplerIndex )
    {
        if ( (const GfxImage *)state->samplerTexture[samplerIndex] == image )
            R_DisableSampler(state, samplerIndex);
    }
}

void R_ClearRenderTargetForMultiGpu(GfxCmdBufContext context, GfxRenderTargetId targetId)
{
    const char *v3; // eax
    int hr; // [esp+Ch] [ebp-4h]

    if (dx.gpuCount >= 2 && !gfxRenderTargets[targetId].cleared)
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("context.state->prim.device->Clear(0, 0, 0x00000001l, 0x00000000, 0, 0)\n");

        hr = context.state->prim.device->Clear(0, 0, 1, 0, 0.0f, 0);
        //hr = ((int(__stdcall *)(IDirect3DDevice9 *, _DWORD, _DWORD, int, _DWORD, _DWORD, _DWORD))context.state->prim.device->Clear)(
        //    context.state->prim.device,
        //    0,
        //    0,
        //    1,
        //    0,
        //    0.0,
        //    0);
        if (hr < 0)
        {
            ++g_disableRendering;
            v3 = R_ErrorDescription(hr);
            Com_Error(
                ERR_FATAL,
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp (%i) context.state->prim.device->Clear(0, 0, 0x00000001l"
                ", 0x00000000, 0, 0) failed: %s\n",
                2937,
                v3);
        }
        gfxRenderTargets[targetId].cleared = 1;
    }
}

void __cdecl R_ClearScreenInternal(
                IDirect3DDevice9 *device,
                unsigned __int8 whichToClear,
                const float *color,
                float depth,
                unsigned __int8 stencil,
                const GfxViewport *viewport)
{
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    const char *v9; // eax
    const char *v10; // eax
    int v12; // [esp+A0h] [ebp-20h]
    int hr; // [esp+A8h] [ebp-18h]
    _D3DRECT rect; // [esp+ACh] [ebp-14h] BYREF
    GfxColor nativeColor; // [esp+BCh] [ebp-4h] BYREF

    if ( !device && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp", 3025, 0, "%s", "device") )
        __debugbreak();
    if ( r_logFile->current.integer )
    {
        RB_LogPrint("---------- R_ClearScreenInternal\n");
        v6 = va(
                     "---------- (%c) color %g %g %g %g\n",
                     (whichToClear & 1) != 0 ? 42 : 32,
                     *color,
                     color[1],
                     color[2],
                     color[3]);
        RB_LogPrint(v6);
        v7 = va("---------- (%c) depth %g\n", (whichToClear & 2) != 0 ? 42 : 32, depth);
        RB_LogPrint(v7);
        v8 = va("---------- (%c) stencil %i\n", (whichToClear & 4) != 0 ? 42 : 32, stencil);
        RB_LogPrint(v8);
    }
    if ( !whichToClear
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp", 3037, 0, "%s", "whichToClear") )
    {
        __debugbreak();
    }
    if ( !color && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp", 3038, 0, "%s", "color") )
        __debugbreak();
    if ( (depth < 0.0 || depth > 1.0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp",
                    3039,
                    0,
                    "depth not in [0.0f, 1.0f]\n\t%g not in [%g, %g]",
                    depth,
                    0.0,
                    1.0) )
    {
        __debugbreak();
    }
    Byte4PackPixelColor(color, (unsigned __int8 *)&nativeColor);
    if ( viewport )
    {
        rect.x1 = viewport->x;
        rect.x2 = viewport->width + rect.x1;
        rect.y1 = viewport->y;
        rect.y2 = viewport->height + rect.y1;
        if ( r_logFile && r_logFile->current.integer )
            RB_LogPrint("device->Clear( 1, &rect, whichToClear, nativeColor.packed, depth, stencil )\n");

        hr = device->Clear(1, &rect, whichToClear, (D3DCOLOR)nativeColor.packed, depth, stencil);
        //hr = ((int (__stdcall *)(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int))device->Clear)(
        //             device,
        //             1,
        //             &rect,
        //             whichToClear,
        //             (GfxColor)nativeColor.packed,
        //             LODWORD(depth),
        //             stencil);
        if ( hr < 0 )
        {
            ++g_disableRendering;
            v9 = R_ErrorDescription(hr);
            Com_Error(
                ERR_FATAL,
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp (%i) device->Clear( 1, &rect, whichToClear, nativeColor."
                "packed, depth, stencil ) failed: %s\n",
                3075,
                v9);
        }
    }
    else
    {
        if ( r_logFile && r_logFile->current.integer )
            RB_LogPrint("device->Clear( 0, 0, whichToClear, nativeColor.packed, depth, stencil )\n");
        v12 = device->Clear(0, 0, whichToClear, nativeColor.packed, depth, stencil);
        //v12 = ((int (__stdcall *)(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int))device->Clear)(
        //                device,
        //                0,
        //                0,
        //                whichToClear,
        //                (GfxColor)nativeColor.packed,
        //                LODWORD(depth),
        //                stencil);
        if ( v12 < 0 )
        {
            ++g_disableRendering;
            v10 = R_ErrorDescription(v12);
            Com_Error(
                ERR_FATAL,
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp (%i) device->Clear( 0, 0, whichToClear, nativeColor.pack"
                "ed, depth, stencil ) failed: %s\n",
                3079,
                v10);
        }
    }
}

void __cdecl Byte4PackPixelColor(const float *from, unsigned __int8 *to)
{
    unsigned __int8 v2; // [esp+0h] [ebp-90h]
    unsigned __int8 v3; // [esp+4h] [ebp-8Ch]
    unsigned __int8 v4; // [esp+8h] [ebp-88h]
    unsigned __int8 v5; // [esp+Ch] [ebp-84h]
    int v6; // [esp+18h] [ebp-78h]
    int v7; // [esp+38h] [ebp-58h]
    int v8; // [esp+58h] [ebp-38h]
    int v9; // [esp+78h] [ebp-18h]

    if ( SnapFloatToInt((float)(255.0 * *from)) < 255 )
        v9 = SnapFloatToInt((float)(255.0 * *from));
    else
        v9 = 255;
    if ( v9 > 0 )
        v5 = v9;
    else
        v5 = 0;
    to[2] = v5;
    if ( SnapFloatToInt((float)(255.0 * from[1])) < 255 )
        v8 = SnapFloatToInt((float)(255.0 * from[1]));
    else
        v8 = 255;
    if ( v8 > 0 )
        v4 = v8;
    else
        v4 = 0;
    to[1] = v4;
    if ( SnapFloatToInt((float)(255.0 * from[2])) < 255 )
        v7 = SnapFloatToInt((float)(255.0 * from[2]));
    else
        v7 = 255;
    if ( v7 > 0 )
        v3 = v7;
    else
        v3 = 0;
    *to = v3;
    if ( SnapFloatToInt((float)(255.0 * from[3])) < 255 )
        v6 = SnapFloatToInt((float)(255.0 * from[3]));
    else
        v6 = 255;
    if ( v6 > 0 )
        v2 = v6;
    else
        v2 = 0;
    to[3] = v2;
}

void __cdecl R_ClearScreen(
                IDirect3DDevice9 *device,
                unsigned __int8 whichToClear,
                const float *color,
                float depth,
                unsigned __int8 stencil,
                const GfxViewport *viewport)
{
    if ( !whichToClear
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp", 3125, 0, "%s", "whichToClear") )
    {
        __debugbreak();
    }
    if ( (whichToClear & 0xF8) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp",
                    3126,
                    0,
                    "%s\n\t(whichToClear) = %i",
                    "((whichToClear & ~(0x00000001l | 0x00000002l | 0x00000004l)) == 0)",
                    whichToClear) )
    {
        __debugbreak();
    }
    if ( !color && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp", 3127, 0, "%s", "color") )
        __debugbreak();
    if ( (depth < 0.0 || depth > 1.0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp",
                    3128,
                    0,
                    "%s\n\t(depth) = %g",
                    "(depth >= 0.0f && depth <= 1.0f)",
                    depth) )
    {
        __debugbreak();
    }
    if ( pixelCostMode <= GFX_PIXEL_COST_MODE_MEASURE_MSEC || (whichToClear &= ~1u) != 0 )
        R_ClearScreenInternal(device, whichToClear, color, depth, stencil, viewport);
}

void __cdecl R_SetMeshStream(GfxCmdBufState *state, GfxMeshData *mesh)
{
    R_SetStreamSource(&state->prim, mesh->vb.buffer, 0, mesh->vertSize);
}

void __cdecl R_SetCompleteState(IDirect3DDevice9 *device, unsigned int *stateBits)
{
    if ( !device && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp", 3151, 0, "%s", "device") )
        __debugbreak();
    R_HW_SetColorMask(device, *stateBits);
    R_HW_SetAlphaTestEnable(device, *stateBits);
    if ( (*stateBits & 0x800) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.cpp",
                    3162,
                    0,
                    "%s",
                    "stateBits[0] & GFXS0_ATEST_DISABLE") )
    {
        __debugbreak();
    }
    R_HW_SetCullFace(device, *stateBits);
    R_HW_SetPolygonMode(device, *stateBits);
    R_ForceSetBlendState(device, *stateBits);
    R_HW_SetDepthWriteEnable(device, stateBits[1]);
    R_HW_SetDepthTestEnable(device, stateBits[1]);
    R_HW_SetDepthTestFunction(device, stateBits[1]);
    R_ForceSetPolygonOffset(device, stateBits[1]);
    R_ForceSetStencilState(device, stateBits[1]);
}

void __cdecl R_ForceSetBlendState(IDirect3DDevice9 *device, unsigned int stateBits0)
{
    if ( (stateBits0 & 0x700) != 0 )
        R_HW_SetBlend(device, 0, 0xFFFFFFFF, stateBits0);
    else
        R_HW_DisableBlend(device);
}

void __cdecl R_ForceSetStencilState(IDirect3DDevice9 *device, unsigned int stateBits1)
{
    if ( (stateBits1 & 0x40) != 0 )
        R_HW_EnableStencil(device);
    else
        R_HW_DisableStencil(device);
    if ( (stateBits1 & 0x80) == 0 )
        stateBits1 = stateBits1 & 0xFFFFF | ((stateBits1 & 0xFFF00) << 12);
    R_HW_SetFrontStencilOp(device, (stateBits1 >> 8) & 7, (stateBits1 >> 11) & 7, (stateBits1 >> 14) & 7);
    R_HW_SetBackStencilOp(device, (stateBits1 >> 20) & 7, (stateBits1 >> 23) & 7, (stateBits1 >> 26) & 7);
    R_HW_SetFrontStencilFunc(device, (stateBits1 >> 17) & 7);
    R_HW_SetBackStencilFunc(device, stateBits1 >> 29);
}

void __cdecl R_Set_Texture_SeeThruDecal(GfxCmdBufSourceState *source)
{
    R_SetCodeImageTexture(source, 0x26u, rgp.r32fWhiteImage);
}

void R_DrawCall(
    void(__cdecl *callback)(const void *, GfxCmdBufContext, GfxCmdBufContext),
    const void *userData,
    GfxCmdBufSourceState *source,
    const GfxViewInfo *viewInfo,
    GfxDrawSurfListInfo *info,
    const GfxViewParms *viewParms,
    GfxCmdBuf *cmdBufEA,
    GfxCmdBuf *prepassCmdBufEA)
{
    GfxCmdBufState prepassCmdBuf; // [esp-27D0h] [ebp-27DCh] BYREF
    GfxCmdBufState cmdBuf; // [esp-1400h] [ebp-140Ch] BYREF
    GfxCmdBufContext prepassContext; // [esp-14h] [ebp-20h]
    GfxCmdBufContext context;

    context.source = source;
    context.state = &cmdBuf;

    prepassContext.source = 0;
    prepassContext.state = 0;

    if (info)
        info->isMissileCamera = viewInfo->isMissileCamera;

    R_BeginView(source, &viewInfo->sceneDef, viewParms);

    cmdBuf.prim.device = cmdBufEA->device;
    R_InitLocalCmdBufState(&cmdBuf);

    R_Set_Texture_SeeThruDecal(source);
    R_SetCodeImageTexture(source, 0x27u, gfxRenderTargets[R_RENDERTARGET_UI3D].image);

    if (viewInfo->isMissileCamera)
        R_SetCodeImageTexture(source, 0x28u, rgp.blackImage);
    else
        R_SetCodeImageTexture(source, 0x28u, gfxRenderTargets[R_RENDERTARGET_MISSILE_CAM].image);

    if (prepassCmdBufEA)
    {
        prepassContext.source = source;
        prepassContext.state = &prepassCmdBuf;

        R_InitLocalCmdBufState(&prepassCmdBuf);

        callback(userData, context, prepassContext);
        memcpy(&gfxCmdBufState, &prepassCmdBuf, sizeof(gfxCmdBufState));
    }
    else
    {
        prepassContext.source = NULL;
        prepassContext.state = NULL;
        callback(userData, context, prepassContext);
    }
    memcpy(gfxCmdBufState.refSamplerState, cmdBuf.refSamplerState, sizeof(gfxCmdBufState));
}