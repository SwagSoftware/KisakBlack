#include "rb_backend.h"
#include "rb_stats.h"
#include "r_image.h"
#include "rb_shade.h"
#include "r_state_utils.h"
#include <physics/phys_render.h>
#include "r_foliage.h"
#include "r_state.h"
#include "r_dvars.h"
#include "rb_logfile.h"
#include "rb_draw3d.h"
#include <EffectsCore/fx_sprite.h>
#include "r_draw_shadowablelight.h"
#include "r_water_sim.h"
#include "r_ui3d.h"
#include "r_wind.h"
#include "r_shader_constant_set.h"
#include "r_draw_material.h"
#include "rb_pixelcost.h"
#include "r_shade.h"
#include <cgame/cg_drawtools.h>
#include <win32/win_shared.h>
#include <stringed/stringed_hooks.h>
#include "r_cinematic.h"
#include "r_model_lighting.h"
#include "r_staticmodelcache.h"
#include <xanim/xmodel_utils.h>
#include <mjpeg/mjpeg.h>
#include <qcommon/threads.h>
#include <win32/win_net.h>
#include <win32/win_common.h>
#include <client/cl_compositing.h>
#include "r_pixelcost_load_obj.h"
#include "r_setstate_d3d.h"
#include <intrin.h>
#include "r_streamalloc.h"
#include "r_stream.h"
#include "rb_resource.h"
#include <client_mp/cl_scrn_mp.h>
#include "rb_compositing.h"
#include <ui/ui_main_pc.h>

int g_msgTime;
bool assets_released;
bool g_showCursor;
const float MY_OFFSETS[4][2] = { { -1.0, -1.0 }, { -1.0, 1.0 }, { 1.0, -1.0 }, { 1.0, 1.0 } };

const char *gfxRenderCommandNames[] =
{
    "RC_END_OF_LIST",
    "RC_SET_CUSTOM_CONSTANT",
    "RC_SET_MATERIAL_COLOR",
    "RC_SAVE_SCREEN",
    "RC_SAVE_SCREEN_SECTION",
    "RC_CLEAR_SCREEN",
    "RC_SET_VIEWPORT",
    "RC_SET_SCISSOR",
    "RC_RESOLVE_COMPOSITE",
    "RC_PC_COPY_IMAGE_GEN_MIP",
    "RC_STRETCH_PIC",
    "RC_STRETCH_PIC_FLIP_ST",
    "RC_STRETCH_PIC_ROTATE_XY",
    "RC_STRETCH_PIC_ROTATE_ST",
    "RC_STRETCH_RAW",
    "RC_DRAW_QUAD_PIC",
    "RC_DRAW_FULL_SCREEN_COLORED_QUAD",
    "RC_DRAW_TEXT_2D",
    "RC_DRAW_TEXT_3D",
    "RC_BLEND_SAVED_SCREEN_BLURRED",
    "RC_BLEND_SAVED_SCREEN_FLASHED",
    "RC_DRAW_POINTS",
    "RC_DRAW_LINES",
    "RC_DRAW_TRIANGLES",
    "RC_DRAW_QUADLIST_2D",
    "RC_DRAW_EMBLEM_LAYER",
    "RC_STRETCH_COMPOSITE",
    "RC_PROJECTION_SET",
    "RC_DRAW_FRAMED"
};

void(__cdecl *const RB_RenderCommandTable[29])(GfxRenderCommandExecState *) =
{
  NULL,
  &RB_SetCustomConstantCmd,
  &RB_SetMaterialColorCmd,
  &RB_BlendSavedScreenFlashedCmd,
  &RB_BlendSavedScreenFlashedCmd,
  &RB_ClearScreenCmd,
  &RB_SetViewportCmd,
  &RB_SetScissorCmd,
  &RB_ResolveCompositeCmd,
  &RB_PCCopyImageGenMIPCmd,
  &RB_StretchPicCmd,
  &RB_StretchPicCmdFlipST,
  &RB_StretchPicRotateXYCmd,
  &RB_StretchPicRotateSTCmd,
  &RB_StretchRawCmd,
  &RB_DrawQuadPicCmd,
  &RB_DrawFullScreenColoredQuadCmd,
  &RB_DrawText2DCmd,
  &RB_DrawText3DCmd,
  &RB_BlendSavedScreenFlashedCmd,
  &RB_BlendSavedScreenFlashedCmd,
  &RB_DrawPointsCmd,
  &RB_DrawLinesCmd,
  &RB_DrawTrianglesCmd,
  &RB_DrawQuadList2DCmd,
  &RB_DrawEmblemLayer,
  &RB_StretchCompositeCmd,
  &RB_ProjectionSetCmd,
  &RB_DrawFramedCmd
};

const unsigned __int8 color_table[17][4] =
{
  { 0u, 0u, 0u, 255u },
  { 255u, 51u, 51u, 255u },
  { 0u, 255u, 0u, 255u },
  { 255u, 255u, 128u, 255u },
  { 0u, 0u, 255u, 255u },
  { 0u, 255u, 255u, 255u },
  { 255u, 92u, 255u, 255u },
  { 255u, 255u, 255u, 255u },
  { 255u, 255u, 255u, 255u },
  { 255u, 255u, 255u, 255u },
  { 216u, 0u, 0u, 255u },
  { 60u, 184u, 0u, 120u },
  { 247u, 148u, 29u, 255u },
  { 0u, 121u, 197u, 255u },
  { 173u, 190u, 200u, 255u },
  { 128u, 71u, 149u, 255u },
  { 165u, 89u, 0u, 255u }
};

unsigned __int8 gJitterData[176] =
{
  0u,
  0u,
  1u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  2u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  1u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  1u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  1u,
  0u,
  0u,
  0u,
  0u,
  1u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  1u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  1u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  1u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  1u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  1u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u,
  0u
};

const unsigned __int8 MY_ALTCOLOR_TWO[4] = { 230u, 255u, 230u, 220u };

unsigned int(__cdecl *const rb_tessTable[16])(const GfxDrawSurfListArgs *, GfxCmdBufContext) =
{
  &R_TessTrianglesList,
  &R_TessTrianglesPreTessList,
  &R_TessStaticModelRigidDrawSurfList,
  &R_TessStaticModelPreTessList,
  &R_TessStaticModelCachedList,
  &R_TessStaticModelSkinnedDrawSurfList,
  &R_TessBModel,
  &R_TessXModelRigidDrawSurfList,
  &R_TessXModelRigidSkinnedDrawSurfList,
  &R_TessXModelSkinnedDrawSurfList,
  &R_TessXModelWaterList,
  &R_TessCodeMeshList,
  &R_TessMarkMeshList,
  &R_TessParticleCloudList,
  &R_TessRopeMeshList,
  &R_TessGlassMeshList
};

GfxRenderTarget gfxRenderTargets[R_RENDERTARGET_COUNT];

GfxDrawConsts g_drawConsts;
r_backEndGlobals_t backEnd;
materialCommands_t tess;
GfxBackEndData *backEndData;

int rb_execCmdsMS;
int rb_swapMS;

void __cdecl RB_CopyBackendStats()
{
    if ( rg.stats )
    {
        rg.stats->c_indexes = g_frameStatsCur.geoIndexCount;
        rg.stats->c_fxIndexes = g_frameStatsCur.fxIndexCount;
        rg.stats->c_viewIndexes = RB_Stats_ViewIndexCount(g_frameStatsCur.viewStats);
        rg.stats->c_shadowIndexes = RB_Stats_ViewIndexCount(&g_frameStatsCur.viewStats[1]);
        rg.stats->c_vertexes = RB_Stats_TotalVertexCount();
        rg.stats->c_batches = RB_Stats_TotalPrimCount();
    }
    if ( rg.globalstats )
        R_SumOfUsedImages(&rg.globalstats->c_imageUsage);
}

void __cdecl RB_SetIdentity()
{
  if ( gfxCmdBufSourceState.viewMode != VIEW_MODE_IDENTITY )
  {
    if ( tess.indexCount )
      RB_EndTessSurface();
    gfxCmdBufSourceState.viewMode = VIEW_MODE_IDENTITY;
    memcpy(&gfxCmdBufSourceState.viewParms, &rg.identityViewParms, sizeof(gfxCmdBufSourceState.viewParms));
    gfxCmdBufSourceState.eyeOffset[0] = 0.0f;
    gfxCmdBufSourceState.eyeOffset[1] = 0.0f;
    gfxCmdBufSourceState.eyeOffset[2] = 0.0f;
    gfxCmdBufSourceState.eyeOffset[3] = 1.0f;
    R_CmdBufSet3D(&gfxCmdBufSourceState);
  }
}

void __cdecl R_SetVertex2d(GfxVertex *vert, float x, float y, float s, float t, unsigned int color)
{
    vert->xyzw[0] = x;
    vert->xyzw[1] = y;
    vert->xyzw[2] = 0.0f;
    vert->xyzw[3] = 1.0f;
    vert->normal.packed = 1073643391;
    vert->color.packed = color;
    vert->texCoord[0] = s;
    vert->texCoord[1] = t;
}

void __cdecl RB_DrawFullSceneQuad(
    const Material *material,
    unsigned __int8 renderTargetId,
    float s0,
    float t0,
    float s1,
    float t1,
    GfxPrimStatsTarget statsTarget)
{
    unsigned __int16 vertCount; // [esp+14h] [ebp-14h]
    float h; // [esp+20h] [ebp-8h]
    float w; // [esp+24h] [ebp-4h]

    w = (float)gfxRenderTargets[renderTargetId].width;
    h = (float)gfxRenderTargets[renderTargetId].height;

    iassert(gfxCmdBufSourceState.viewMode == VIEW_MODE_2D);

    RB_SetTessTechnique(material, 4u);
    R_TrackPrims(&gfxCmdBufState.prim, statsTarget);
    RB_CheckTessOverflow(4, 6);
    vertCount = tess.vertexCount;
    tess.indices[tess.indexCount] = LOWORD(tess.vertexCount) + 3;
    tess.indices[tess.indexCount + 1] = vertCount;
    tess.indices[tess.indexCount + 2] = vertCount + 2;
    tess.indices[tess.indexCount + 3] = vertCount + 2;
    tess.indices[tess.indexCount + 4] = vertCount;
    tess.indices[tess.indexCount + 5] = vertCount + 1;
    R_SetVertex2d(&tess.verts[tess.vertexCount], 0.0, 0.0, s0, t0, 0xFFFFFFFF);
    R_SetVertex2d(&tess.verts[tess.vertexCount + 1], w + 0.0, 0.0, s1, t0, 0xFFFFFFFF);
    R_SetVertex2d(&tess.verts[tess.vertexCount + 2], w + 0.0, h + 0.0, s1, t1, 0xFFFFFFFF);
    R_SetVertex2d(&tess.verts[tess.vertexCount + 3], 0.0, h + 0.0, s0, t1, 0xFFFFFFFF);
    tess.vertexCount += 4;
    tess.indexCount += 6;
}

void __cdecl RB_DrawSW4Quads(const Material *material, int ParticleCount)
{
    int i; // [esp+14h] [ebp-8h]
    unsigned __int16 vertCount; // [esp+18h] [ebp-4h]

    RB_SetTessTechnique(material, 4u);
    R_TrackPrims(&gfxCmdBufState.prim, GFX_PRIM_STATS_CODE);
    RB_CheckTessOverflow(4 * ParticleCount, 6 * ParticleCount);
    for ( i = 0; i < ParticleCount; ++i )
    {
        vertCount = tess.vertexCount;
        tess.indices[tess.indexCount] = LOWORD(tess.vertexCount) + 3;
        tess.indices[tess.indexCount + 1] = vertCount;
        tess.indices[tess.indexCount + 2] = vertCount + 2;
        tess.indices[tess.indexCount + 3] = vertCount + 2;
        tess.indices[tess.indexCount + 4] = vertCount;
        tess.indices[tess.indexCount + 5] = vertCount + 1;
        R_SetVertex2d(&tess.verts[tess.vertexCount], (float)i, 0.0, 0.0, 0.0, 0xFFFFFFFF);
        R_SetVertex2d(&tess.verts[tess.vertexCount + 1], (float)i, 0.0, 1.0, 0.0, 0xFFFFFFFF);
        R_SetVertex2d(&tess.verts[tess.vertexCount + 2], (float)i, 0.0, 1.0, 1.0, 0xFFFFFFFF);
        R_SetVertex2d(&tess.verts[tess.vertexCount + 3], (float)i, 0.0, 0.0, 1.0, 0xFFFFFFFF);
        tess.vertexCount += 4;
        tess.indexCount += 6;
    }
    RB_EndTessSurface();
}

void __cdecl RB_DrawStretchPic(
                const Material *material,
                float x,
                float y,
                float w,
                float h,
                float s0,
                float t0,
                float s1,
                float t1,
                unsigned int color,
                GfxPrimStatsTarget statsTarget)
{
    unsigned __int16 vertCount; // [esp+14h] [ebp-4h]

    iassert(gfxCmdBufSourceState.viewMode == VIEW_MODE_2D);

    RB_SetTessTechnique(material, 4u);
    R_TrackPrims(&gfxCmdBufState.prim, statsTarget);
    RB_CheckTessOverflow(4, 6);

    vertCount = tess.vertexCount;
    tess.indices[tess.indexCount] = vertCount + 3;
    tess.indices[tess.indexCount + 1] = vertCount;
    tess.indices[tess.indexCount + 2] = vertCount + 2;
    tess.indices[tess.indexCount + 3] = vertCount + 2;
    tess.indices[tess.indexCount + 4] = vertCount;
    tess.indices[tess.indexCount + 5] = vertCount + 1;

    R_SetVertex2d(&tess.verts[tess.vertexCount], x, y, s0, t0, color);
    R_SetVertex2d(&tess.verts[tess.vertexCount + 1], x + w, y, s1, t0, color);
    R_SetVertex2d(&tess.verts[tess.vertexCount + 2], x + w, y + h, s1, t1, color);
    R_SetVertex2d(&tess.verts[tess.vertexCount + 3], x, y + h, s0, t1, color);

    tess.vertexCount += 4;
    tess.indexCount += 6;
}

void __cdecl RB_DrawStretchPicW(
                const Material *material,
                float x,
                float y,
                float w0,
                float w,
                float h,
                float s0,
                float t0,
                float s1,
                float t1,
                unsigned int color,
                GfxPrimStatsTarget statsTarget)
{
    unsigned __int16 vertCount; // [esp+1Ch] [ebp-4h]

    iassert(gfxCmdBufSourceState.viewMode == VIEW_MODE_2D);

    RB_SetTessTechnique(material, 4u);
    R_TrackPrims(&gfxCmdBufState.prim, statsTarget);
    RB_CheckTessOverflow(4, 6);
    vertCount = tess.vertexCount;
    tess.indices[tess.indexCount] = LOWORD(tess.vertexCount) + 3;
    tess.indices[tess.indexCount + 1] = vertCount;
    tess.indices[tess.indexCount + 2] = vertCount + 2;
    tess.indices[tess.indexCount + 3] = vertCount + 2;
    tess.indices[tess.indexCount + 4] = vertCount;
    tess.indices[tess.indexCount + 5] = vertCount + 1;
    R_SetVertex4d(&tess.verts[tess.vertexCount], x, y, 0.0, w0, s0, t0, color);
    R_SetVertex4d(&tess.verts[tess.vertexCount + 1], x + w, y, 0.0, w0, s1, t0, color);
    R_SetVertex4d(&tess.verts[tess.vertexCount + 2], x + w, y + h, 0.0, w0, s1, t1, color);
    R_SetVertex4d(&tess.verts[tess.vertexCount + 3], x, y + h, 0.0, w0, s0, t1, color);
    tess.vertexCount += 4;
    tess.indexCount += 6;
}

void __cdecl R_SetVertex4d(GfxVertex *vert, float x, float y, float z, float w, float s, float t, unsigned int color)
{
    vert->xyzw[0] = x;
    vert->xyzw[1] = y;
    vert->xyzw[2] = z;
    vert->xyzw[3] = w;
    vert->normal.packed = 1073643391;
    vert->color.packed = color;
    vert->texCoord[0] = s;
    vert->texCoord[1] = t;
}

void __cdecl RB_DrawStretchPicZ(
                const Material *material,
                float x,
                float y,
                float z,
                float w,
                float h,
                float s0,
                float t0,
                float s1,
                float t1,
                unsigned int color,
                GfxPrimStatsTarget statsTarget)
{
    unsigned __int16 vertCount; // [esp+18h] [ebp-8h]
    GfxColor unpackedColor; // [esp+1Ch] [ebp-4h] BYREF

    iassert(gfxCmdBufSourceState.viewMode == VIEW_MODE_2D);
    
    RB_SetTessTechnique(material, 4u);
    R_TrackPrims(&gfxCmdBufState.prim, statsTarget);
    RB_CheckTessOverflow(4, 6);
    vertCount = tess.vertexCount;
    tess.indices[tess.indexCount] = LOWORD(tess.vertexCount) + 3;
    tess.indices[tess.indexCount + 1] = vertCount;
    tess.indices[tess.indexCount + 2] = vertCount + 2;
    tess.indices[tess.indexCount + 3] = vertCount + 2;
    tess.indices[tess.indexCount + 4] = vertCount;
    tess.indices[tess.indexCount + 5] = vertCount + 1;
    unpackedColor.packed = color;
    R_SetVertex3d(&tess.verts[tess.vertexCount], x, y, z, s0, t0, (const unsigned __int8 *)&unpackedColor);
    R_SetVertex3d(&tess.verts[tess.vertexCount + 1], x + w, y, z, s1, t0, (const unsigned __int8 *)&unpackedColor);
    R_SetVertex3d(&tess.verts[tess.vertexCount + 2], x + w, y + h, z, s1, t1, (const unsigned __int8 *)&unpackedColor);
    R_SetVertex3d(&tess.verts[tess.vertexCount + 3], x, y + h, z, s0, t1, (const unsigned __int8 *)&unpackedColor);
    tess.vertexCount += 4;
    tess.indexCount += 6;
}

void __cdecl R_SetVertex3d(GfxVertex *vert, float x, float y, float z, float s, float t, const unsigned __int8 *color)
{
    vert->xyzw[0] = x;
    vert->xyzw[1] = y;
    vert->xyzw[2] = z;
    vert->xyzw[3] = 1.0f;
    vert->normal.packed = 1073643391;
    vert->color.packed = *(unsigned int *)color;
    vert->texCoord[0] = s;
    vert->texCoord[1] = t;
}

void __cdecl RB_DrawFullScreenColoredQuad(
        const Material *material,
        float s0,
        float t0,
        float s1,
        float t1,
        unsigned int color)
{
  if ( tess.indexCount )
    RB_EndTessSurface();
  R_Set2D(&gfxCmdBufSourceState);
  RB_DrawStretchPic(
    material,
    0.0,
    0.0,
    (float)gfxCmdBufSourceState.renderTargetWidth,
    (float)gfxCmdBufSourceState.renderTargetHeight,
    s0,
    t0,
    s1,
    t1,
    color,
    GFX_PRIM_STATS_CODE);
  RB_EndTessSurface();
}

void __cdecl RB_FullScreenColoredFilter(const Material *material, unsigned int color)
{
    RB_DrawFullScreenColoredQuad(material, 0.0, 0.0, 1.0, 1.0, color);
}

void __cdecl RB_FullScreenFilter(const Material *material)
{
  R_UpdateCodeConstant(&gfxCmdBufSourceState, CONST_SRC_CODE_VIEWPORT_DIMENSIONS, 0.0, 0.0, 1.0, 1.0);
  RB_FullScreenColoredFilter(material, 0xFFFFFFFF);
}

void __cdecl RB_Filter(const Material *material, const GfxViewInfo *viewInfo)
{
    if ( (viewInfo->sceneComposition.renderingMode & 7) != 0 || viewInfo->isRenderingFullScreen )
        RB_FullScreenFilter(material);
    else
        RB_SplitScreenFilter(material, viewInfo, 0xFFFFFFFF);
}

void __cdecl RB_ColoredFilter(const Material *material, const GfxViewInfo *viewInfo, unsigned int color)
{
    if ( viewInfo->isRenderingFullScreen )
        RB_FullScreenColoredFilter(material, color);
    else
        RB_SplitScreenFilter(material, viewInfo, color);
}

void __cdecl RB_SplitScreenFilter(const Material *material, const GfxViewInfo *viewInfo, unsigned int color)
{
  float t0; // [esp+30h] [ebp-28h] BYREF
  float t1; // [esp+34h] [ebp-24h] BYREF
  float s1; // [esp+38h] [ebp-20h] BYREF
  float _t1; // [esp+3Ch] [ebp-1Ch]
  float s0; // [esp+40h] [ebp-18h] BYREF
  float x; // [esp+44h] [ebp-14h]
  float y; // [esp+48h] [ebp-10h]
  float h; // [esp+4Ch] [ebp-Ch]
  float w; // [esp+50h] [ebp-8h]
  float _s1; // [esp+54h] [ebp-4h]

  if ( tess.indexCount )
    RB_EndTessSurface();
  R_Set2D(&gfxCmdBufSourceState);
  x = (float)viewInfo->cullViewInfo.displayViewport.x;
  y = (float)viewInfo->cullViewInfo.displayViewport.y;
  w = (float)viewInfo->cullViewInfo.displayViewport.width;
  h = (float)viewInfo->cullViewInfo.displayViewport.height;
  RB_SplitScreenTexCoords(x, y, w, h, &s0, &t0, &s1, &t1);
  _s1 = 1.0 / (float)(s1 - s0);
  _t1 = 1.0 / (float)(t1 - t0);
  R_UpdateCodeConstant(&gfxCmdBufSourceState, CONST_SRC_CODE_VIEWPORT_DIMENSIONS, s0, t0, _s1, _t1);
  RB_DrawStretchPic(material, 0.0, 0.0, w, h, s0, t0, s1, t1, color, GFX_PRIM_STATS_CODE);
  RB_EndTessSurface();
}

void __cdecl RB_SplitScreenTexCoords(float x, float y, float w, float h, float *s0, float *t0, float *s1, float *t1)
{
    float xa; // [esp+10h] [ebp+8h]
    float ya; // [esp+14h] [ebp+Ch]
    float wa; // [esp+18h] [ebp+10h]
    float ha; // [esp+1Ch] [ebp+14h]

    if (!s0 && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp", 854, 0, "%s", "s0"))
        __debugbreak();
    if (!s1 && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp", 855, 0, "%s", "s1"))
        __debugbreak();
    if (!t0 && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp", 856, 0, "%s", "t0"))
        __debugbreak();
    if (!t1 && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp", 857, 0, "%s", "t1"))
        __debugbreak();
    xa = x / (float)gfxCmdBufSourceState.renderTargetWidth;
    ya = y / (float)gfxCmdBufSourceState.renderTargetHeight;
    wa = w / (float)gfxCmdBufSourceState.renderTargetWidth;
    ha = h / (float)gfxCmdBufSourceState.renderTargetHeight;
    *s0 = xa;
    *t0 = ya;
    *s1 = xa + wa;
    *t1 = ya + ha;
}

static void R_GetIntZHandles()
{
#ifdef KISAK_NVAPI_INTZ
    NVDX_ObjectHandle__ *h; // [esp+0h] [ebp-4h] BYREF

    if (!NvAPI_D3D9_GetCurrentZBufferHandle(dx.device, &h))
    {
        dx.nvDepthBufferHandle = h;

        if (!NvAPI_D3D9_GetTextureHandle(gfxRenderTargets[R_RENDERTARGET_FLOAT_Z].image->texture.map, &h))
        {
            dx.nvFloatZBufferHandle = h;
        }
    }
#endif
}

void R_ResolveIntZ_PC()
{
    NVDX_ObjectHandle__ *v2; // ecx
    float dummy[3]; // [esp+4h] [ebp-10h] BYREF
    NvAPI_Status status; // [esp+10h] [ebp-4h]

    iassert(dx.supportsIntZ);

#ifdef KISAK_NVAPI_INTZ
    if (dx.nvInitialized)
    {
        if (!dx.nvDepthBufferHandle || !dx.nvFloatZBufferHandle)
            R_GetIntZHandles();
    
        if (NvAPI_D3D9_StretchRect(dx.device, (NVDX_ObjectHandle)dx.nvDepthBufferHandle, 0, (NVDX_ObjectHandle)dx.nvFloatZBufferHandle, 0, D3DTEXF_POINT))
        {
            R_GetIntZHandles();
            status = NvAPI_D3D9_StretchRect(
                dx.device,
                (NVDX_ObjectHandle)dx.nvDepthBufferHandle,
                0,
                (NVDX_ObjectHandle)dx.nvFloatZBufferHandle,
                0,
                D3DTEXF_POINT);
        }
    }
    else
#endif
    {
        memset(dummy, 0, sizeof(dummy));
        dx.device->SetTexture(0, gfxRenderTargets[R_RENDERTARGET_FLOAT_Z].image->texture.basemap);
        dx.device->SetRenderState(D3DRS_POINTSIZE, 0);
        dx.device->DrawPrimitiveUP(D3DPT_POINTLIST, 1u, dummy, 12u);
        dx.device->SetRenderState(D3DRS_POINTSIZE, 2141212672u);
        dx.device->DrawPrimitiveUP(D3DPT_POINTLIST, 1u, dummy, 12u);
        gfxCmdBufContext.state->samplerTexture[0] = 0;
        R_ClearAllStreamSources(&gfxCmdBufContext.state->prim);
        if (gfxCmdBufContext.state->prim.indexBuffer)
            R_ChangeIndices(&gfxCmdBufContext.state->prim, 0);
    }
}

void __cdecl R_Resolve(GfxCmdBufContext context, GfxImage *image)
{
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    const char *v6; // eax
    int v8; // [esp+4h] [ebp-7Ch]
    int v10; // [esp+Ch] [ebp-74h]
    int semaphore; // [esp+10h] [ebp-70h]
    int hr; // [esp+14h] [ebp-6Ch]
    float gf; // [esp+18h] [ebp-68h]
    unsigned __int16 r; // [esp+1Ch] [ebp-64h]
    unsigned __int16 g; // [esp+28h] [ebp-58h]
    unsigned __int16 b; // [esp+2Ch] [ebp-54h]
    float rf; // [esp+34h] [ebp-4Ch]
    unsigned __int16 v18; // [esp+38h] [ebp-48h]
    float bf; // [esp+3Ch] [ebp-44h]
    int x; // [esp+40h] [ebp-40h]
    int y; // [esp+44h] [ebp-3Ch]
    IDirect3DSurface9 *surfx; // [esp+48h] [ebp-38h]
    unsigned __int8 *bitsd; // [esp+4Ch] [ebp-34h]
    _D3DLOCKED_RECT rd; // [esp+50h] [ebp-30h] BYREF
    IDirect3DSurface9 *surfxd; // [esp+58h] [ebp-28h]
    _D3DLOCKED_RECT rs; // [esp+5Ch] [ebp-24h] BYREF
    IDirect3DSurface9 *surf; // [esp+64h] [ebp-1Ch]
    IDirect3DSurface9 *surfd; // [esp+68h] [ebp-18h]
    unsigned __int16 *bitss; // [esp+6Ch] [ebp-14h]
    int h; // [esp+70h] [ebp-10h]
    int w; // [esp+74h] [ebp-Ch]
    IDirect3DSurface9 *imageSurface; // [esp+78h] [ebp-8h]

    iassert(image);
    iassert(image->width == gfxRenderTargets[context.state->renderTargetId].width);
    iassert(image->height == gfxRenderTargets[context.state->renderTargetId].height);
    iassert(image != gfxRenderTargets[context.state->renderTargetId].image);

    imageSurface = Image_GetSurface(image);
    iassert(imageSurface);

    if (context.state->renderTargetId == 3 && r_reflectionProbeGenerate->current.enabled)
    {
        w = gfxRenderTargets[R_RENDERTARGET_SCENE].width;
        h = gfxRenderTargets[R_RENDERTARGET_SCENE].height;
        surf = gfxRenderTargets[R_RENDERTARGET_FRAME_BUFFER].surface.color;
        surfd = gfxRenderTargets[R_RENDERTARGET_16BIT_SYSTEM].surface.color;
        surfx = gfxRenderTargets[17].surface.color;
        surfxd = gfxRenderTargets[R_RENDERTARGET_8BIT_SWAPCHAIN_BACKBUFFER].surface.color;
        context.state->prim.device->GetRenderTargetData(
            gfxRenderTargets[R_RENDERTARGET_FRAME_BUFFER].surface.color,
            gfxRenderTargets[R_RENDERTARGET_16BIT_SYSTEM].surface.color);
        Sleep(100);
        surfd->LockRect(&rs, 0, 0);
        //((void(__thiscall *)(IDirect3DSurface9 *, IDirect3DSurface9 *, _D3DLOCKED_RECT *, _DWORD, _DWORD))surfd->LockRect)(
        //    surfd,
        //    surfd,
        //    &rs,
        //    0,
        //    0);
        bitss = (unsigned __int16 *)rs.pBits;
        //((void(__thiscall *)(IDirect3DSurface9 *, IDirect3DSurface9 *, _D3DLOCKED_RECT *, _DWORD, _DWORD))surfx->LockRect)(
        //    surfx,
        //    surfx,
        //    &rd,
        //    0,
        //    0);
        surfx->LockRect(&rd, 0, 0);

        bitsd = (unsigned __int8 *)rd.pBits;
        if (!bitss || !rd.pBits)
        {
            surfx->UnlockRect();
            //((void(__thiscall *)(IDirect3DSurface9 *, IDirect3DSurface9 *))surfd->UnlockRect)(surfd, surfd);
            surfd->UnlockRect();
            return;
        }
        for (y = 0; y < h; ++y)
        {
            for (x = 0; x < w; ++x)
            {
                r = bitss[2];
                g = bitss[1];
                b = *bitss;
                v18 = bitss[3];
                bitss += 4;
                LODWORD(rf) = ((r & 0x3FF) << 13) | (((r & 0x7C00) + 114688) << 13) | ((r & 0x8000) << 16);
                LODWORD(gf) = ((g & 0x3FF) << 13) | (((g & 0x7C00) + 114688) << 13) | ((g & 0x8000) << 16);
                LODWORD(bf) = ((b & 0x3FF) << 13) | (((b & 0x7C00) + 114688) << 13) | ((b & 0x8000) << 16);
                if (rf > 1.0)
                    rf = 1.0f;
                if (gf > 1.0)
                    gf = 1.0f;
                if (bf > 1.0)
                    bf = 1.0f;
                *bitsd = (int)(float)(rf * 255.0);
                bitsd[1] = (int)(float)(gf * 255.0);
                bitsd[2] = (int)(float)(bf * 255.0);
                bitsd[3] = v18;
                bitsd += 4;
            }
            bitss -= 4 * w;
            bitss += rs.Pitch / 2;
            bitsd = &bitsd[rd.Pitch + -4 * w];
        }
        //((void(__thiscall *)(IDirect3DSurface9 *, IDirect3DSurface9 *))surfx->UnlockRect)(surfx, surfx);
        surfx->UnlockRect();
        surfd->UnlockRect();
        context.state->prim.device->UpdateSurface(surfx, 0, surfxd, 0);
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint(
                "context.state->prim.device->StretchRect( gfxRenderTargets[R_RENDERTARGET_SCENE].surface.color, 0, imageSurface, "
                "0, D3DTEXF_LINEAR)\n");
        hr = context.state->prim.device->StretchRect(
            gfxRenderTargets[R_RENDERTARGET_SCENE].surface.color,
            0,
            imageSurface,
            0,
            D3DTEXF_LINEAR);
        if (hr < 0)
        {
            ++g_disableRendering;
            v4 = R_ErrorDescription(hr);
            Com_Error(
                ERR_FATAL,
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp (%i) context.state->prim.device->StretchRect( gfxRend"
                "erTargets[R_RENDERTARGET_SCENE].surface.color, 0, imageSurface, 0, D3DTEXF_LINEAR) failed: %s\n",
                1348,
                v4);
        }
    }
    else
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint(
                "context.state->prim.device->StretchRect( gfxRenderTargets[context.state->renderTargetId].surface.color, 0, imageSurface, 0, D3DTEXF_NONE)\n");
        v10 = context.state->prim.device->StretchRect(gfxRenderTargets[context.state->renderTargetId].surface.color, 0, imageSurface, 0, D3DTEXF_NONE);
        if (v10 < 0)
        {
            ++g_disableRendering;
            Com_Error(
                ERR_FATAL,
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp (%i) context.state->prim.device->StretchRect( gfxRend"
                "erTargets[context.state->renderTargetId].surface.color, 0, imageSurface, 0, D3DTEXF_NONE) failed: %s\n",
                1352,
                R_ErrorDescription(v10));
        }
    }
    if (r_logFile && r_logFile->current.integer)
        RB_LogPrint("imageSurface->Release()\n");
    v8 = imageSurface->Release();
    if (v8 < 0)
    {
        ++g_disableRendering;
        v6 = R_ErrorDescription(v8);
        Com_Error(
            ERR_FATAL,
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp (%i) imageSurface->Release() failed: %s\n",
            1357,
            v6);
    }
}

void __cdecl RB_StretchCompositeCmd(GfxRenderCommandExecState *execState)
{
  const GfxCmdStretchComposite *cmd; // [esp+2Ch] [ebp-4h]

  cmd = (const GfxCmdStretchComposite *)execState->cmd;
  if ( tess.indexCount )
    RB_EndTessSurface();
  R_SetCodeImageTexture(&gfxCmdBufSourceState, 0x29u, cmd->image);
  R_SetCodeImageSamplerState(&gfxCmdBufSourceState, 0x29u, 0x72u);
  RB_DrawStretchPic(
    rgp.compositeResult,
    cmd->x,
    cmd->y,
    cmd->w,
    cmd->h,
    cmd->s0,
    cmd->t0,
    cmd->s1,
    cmd->t1,
    cmd->color.packed,
    GFX_PRIM_STATS_HUD);
  if ( tess.indexCount )
    RB_EndTessSurface();
  execState->cmd = (char *)execState->cmd + *(unsigned __int16 *)execState->cmd;
}

void __cdecl R_SetCodeImageSamplerState(
                GfxCmdBufSourceState *source,
                unsigned int codeTexture,
                unsigned __int8 samplerState)
{
    bcassert(codeTexture, TEXTURE_SRC_CODE_COUNT);
    iassert(samplerState & SAMPLER_FILTER_MASK);

    source->input.codeImageSamplerStates[codeTexture] = samplerState;
}

void __cdecl RB_StretchPicCmd(GfxRenderCommandExecState *execState)
{
    const char *v1; // eax
    const GfxCmdStretchPic *cmd; // [esp+3Ch] [ebp-4h]

    cmd = (const GfxCmdStretchPic *)execState->cmd;

    if ( **((unsigned int **)execState->cmd + 1) )
        v1 = va("\"%s\"", cmd->material->info.name);
    else
        v1 = va("\"%s\"", "noname");

    PROF_SCOPED("RB_StretchPicCmd");
    ZoneTextF("(%s)", v1);

    RB_DrawStretchPicW(
        cmd->material,
        cmd->x,
        cmd->y,
        cmd->w0,
        cmd->w,
        cmd->h,
        cmd->s0,
        cmd->t0,
        cmd->s1,
        cmd->t1,
        cmd->color.packed,
        GFX_PRIM_STATS_HUD);
    execState->cmd = (char *)execState->cmd + *(unsigned __int16 *)execState->cmd;
}

void __cdecl RB_StretchPicCmdFlipST(GfxRenderCommandExecState *execState)
{
    RB_DrawStretchPicFlipST(
        *((const Material **)execState->cmd + 1),
        *((float *)execState->cmd + 2),
        *((float *)execState->cmd + 3),
        *((float *)execState->cmd + 5),
        *((float *)execState->cmd + 6),
        *((float *)execState->cmd + 7),
        *((float *)execState->cmd + 8),
        *((float *)execState->cmd + 9),
        *((float *)execState->cmd + 10),
        *((unsigned int *)execState->cmd + 11),
        GFX_PRIM_STATS_HUD);
    execState->cmd = (char *)execState->cmd + *(unsigned __int16 *)execState->cmd;
}

void __cdecl RB_DrawStretchPicFlipST(
                const Material *material,
                float x,
                float y,
                float w,
                float h,
                float s0,
                float t0,
                float s1,
                float t1,
                unsigned int color,
                GfxPrimStatsTarget statsTarget)
{
    unsigned __int16 vertCount; // [esp+14h] [ebp-4h]

    iassert(gfxCmdBufSourceState.viewMode == VIEW_MODE_2D);

    RB_SetTessTechnique(material, 4u);
    R_TrackPrims(&gfxCmdBufState.prim, statsTarget);
    RB_CheckTessOverflow(4, 6);
    vertCount = tess.vertexCount;
    tess.indices[tess.indexCount] = LOWORD(tess.vertexCount) + 3;
    tess.indices[tess.indexCount + 1] = vertCount;
    tess.indices[tess.indexCount + 2] = vertCount + 2;
    tess.indices[tess.indexCount + 3] = vertCount + 2;
    tess.indices[tess.indexCount + 4] = vertCount;
    tess.indices[tess.indexCount + 5] = vertCount + 1;
    R_SetVertex2d(&tess.verts[tess.vertexCount], x, y, s0, t0, color);
    R_SetVertex2d(&tess.verts[tess.vertexCount + 1], x + w, y, s0, t1, color);
    R_SetVertex2d(&tess.verts[tess.vertexCount + 2], x + w, y + h, s1, t1, color);
    R_SetVertex2d(&tess.verts[tess.vertexCount + 3], x, y + h, s1, t0, color);
    tess.vertexCount += 4;
    tess.indexCount += 6;
}

void __cdecl RB_StretchPicRotateXYCmd(GfxRenderCommandExecState *execState)
{
    float v1; // [esp+28h] [ebp-44h]
    float halfWidth; // [esp+2Ch] [ebp-40h]
    float stepY; // [esp+30h] [ebp-3Ch]
    float cosAngle; // [esp+3Ch] [ebp-30h]
    int indexCount; // [esp+48h] [ebp-24h]
    float midX; // [esp+4Ch] [ebp-20h]
    float sinAngle; // [esp+50h] [ebp-1Ch]
    unsigned __int16 vertCount; // [esp+54h] [ebp-18h]
    float midY; // [esp+58h] [ebp-14h]
    const GfxCmdStretchPicRotateXY *cmd; // [esp+5Ch] [ebp-10h]
    float halfHeight; // [esp+60h] [ebp-Ch]
    unsigned int col; // [esp+68h] [ebp-4h]

    cmd = (const GfxCmdStretchPicRotateXY *)execState->cmd;
    
    iassert(gfxCmdBufSourceState.viewMode == VIEW_MODE_2D);

    RB_SetTessTechnique(cmd->material, 4u);
    R_TrackPrims(&gfxCmdBufState.prim, GFX_PRIM_STATS_HUD);
    RB_CheckTessOverflow(4, 6);
    vertCount = tess.vertexCount;
    indexCount = tess.indexCount;
    tess.vertexCount += 4;
    tess.indexCount += 6;
    tess.indices[indexCount] = vertCount + 3;
    tess.indices[indexCount + 1] = vertCount;
    tess.indices[indexCount + 2] = vertCount + 2;
    tess.indices[indexCount + 3] = vertCount + 2;
    tess.indices[indexCount + 4] = vertCount;
    tess.indices[indexCount + 5] = vertCount + 1;
    halfWidth = (float)(cmd->w * 0.5) / 1.0;
    halfHeight = (float)(cmd->h * 0.5) / 1.0;
    midX = (float)(cmd->x / 1.0) + halfWidth;
    midY = (float)(cmd->y / 1.0) + halfHeight;
    v1 = cmd->rotation * 0.017453292;
    cosAngle = cos(v1);
    sinAngle = sin(v1);
    //stepY = COERCE_FLOAT(LODWORD(halfHeight) ^ _mask__NegFloat_) * sinAngle;
    stepY = -halfHeight * sinAngle;
    col = cmd->color.packed;
    R_SetVertex4d(
        &tess.verts[vertCount],
        (float)((float)(midX - (float)(halfWidth * cosAngle)) - stepY) * 1.0,
        (float)((float)(midY - (float)(halfWidth * sinAngle)) - (float)(halfHeight * cosAngle)) * 1.0,
        0.0,
        cmd->w0,
        cmd->s0,
        cmd->t0,
        col);
    R_SetVertex4d(
        &tess.verts[vertCount + 1],
        (float)((float)(midX + (float)(halfWidth * cosAngle)) - stepY) * 1.0,
        (float)((float)(midY + (float)(halfWidth * sinAngle)) - (float)(halfHeight * cosAngle)) * 1.0,
        0.0,
        cmd->w0,
        cmd->s1,
        cmd->t0,
        col);
    R_SetVertex4d(
        &tess.verts[vertCount + 2],
        (float)((float)(midX + (float)(halfWidth * cosAngle)) + stepY) * 1.0,
        (float)((float)(midY + (float)(halfWidth * sinAngle)) + (float)(halfHeight * cosAngle)) * 1.0,
        0.0,
        cmd->w0,
        cmd->s1,
        cmd->t1,
        col);
    R_SetVertex4d(
        &tess.verts[vertCount + 3],
        (float)((float)(midX - (float)(halfWidth * cosAngle)) + stepY) * 1.0,
        (float)((float)(midY - (float)(halfWidth * sinAngle)) + (float)(halfHeight * cosAngle)) * 1.0,
        0.0,
        cmd->w0,
        cmd->s0,
        cmd->t1,
        col);
    execState->cmd = (char *)execState->cmd + *(unsigned __int16 *)execState->cmd;
}

void __cdecl RB_StretchPicRotateSTCmd(GfxRenderCommandExecState *execState)
{
    float v1; // [esp+20h] [ebp-48h]
    float cosAngle; // [esp+24h] [ebp-44h]
    int indexCount; // [esp+28h] [ebp-40h]
    float sinAngle; // [esp+2Ch] [ebp-3Ch]
    float texS_4; // [esp+34h] [ebp-34h]
    float texS_8; // [esp+38h] [ebp-30h]
    float texS_12; // [esp+3Ch] [ebp-2Ch]
    unsigned __int16 vertCount; // [esp+40h] [ebp-28h]
    float stepT; // [esp+44h] [ebp-24h]
    float stepT_4; // [esp+48h] [ebp-20h]
    const GfxCmdStretchPicRotateST *cmd; // [esp+4Ch] [ebp-1Ch]
    float stepS; // [esp+50h] [ebp-18h]
    float stepS_4; // [esp+54h] [ebp-14h]
    float texT_4; // [esp+5Ch] [ebp-Ch]
    float texT_8; // [esp+60h] [ebp-8h]
    float texT_12; // [esp+64h] [ebp-4h]

    cmd = (const GfxCmdStretchPicRotateST *)execState->cmd;

    iassert(gfxCmdBufSourceState.viewMode == VIEW_MODE_2D);

    RB_SetTessTechnique(cmd->material, 4u);
    R_TrackPrims(&gfxCmdBufState.prim, GFX_PRIM_STATS_HUD);
    vertCount = tess.vertexCount;
    indexCount = tess.indexCount;
    RB_CheckTessOverflow(4, 6);
    tess.vertexCount += 4;
    tess.indexCount += 6;
    tess.indices[indexCount] = vertCount + 3;
    tess.indices[indexCount + 1] = vertCount;
    tess.indices[indexCount + 2] = vertCount + 2;
    tess.indices[indexCount + 3] = vertCount + 2;
    tess.indices[indexCount + 4] = vertCount;
    tess.indices[indexCount + 5] = vertCount + 1;
    v1 = cmd->rotation * 0.017453292;
    cosAngle = cos(v1);
    sinAngle = sin(v1);
    stepS = (float)(cmd->radiusST * cosAngle) * cmd->scaleFinalS;
    stepS_4 = (float)(cmd->radiusST * sinAngle) * cmd->scaleFinalT;
    //stepT = (float)(COERCE_FLOAT(LODWORD(cmd->radiusST) ^ _mask__NegFloat_) * sinAngle) * cmd->scaleFinalS;
    stepT = (float)(-cmd->radiusST) * sinAngle * cmd->scaleFinalS;
    stepT_4 = (float)(cmd->radiusST * cosAngle) * cmd->scaleFinalT;
    texS_4 = (float)(cmd->centerS + stepS) - stepT;
    texT_4 = (float)(cmd->centerT + stepS_4) - stepT_4;
    texS_8 = (float)(cmd->centerS + stepS) + stepT;
    texT_8 = (float)(cmd->centerT + stepS_4) + stepT_4;
    texS_12 = (float)(cmd->centerS - stepS) + stepT;
    texT_12 = (float)(cmd->centerT - stepS_4) + stepT_4;
    R_SetVertex2d(
        &tess.verts[vertCount],
        cmd->x,
        cmd->y,
        (float)(cmd->centerS - stepS) - stepT,
        (float)(cmd->centerT - stepS_4) - stepT_4,
        cmd->color.packed);
    R_SetVertex2d(&tess.verts[vertCount + 1], cmd->x + cmd->w, cmd->y, texS_4, texT_4, cmd->color.packed);
    R_SetVertex2d(&tess.verts[vertCount + 2], cmd->x + cmd->w, cmd->y + cmd->h, texS_8, texT_8, cmd->color.packed);
    R_SetVertex2d(&tess.verts[vertCount + 3], cmd->x, cmd->y + cmd->h, texS_12, texT_12, cmd->color.packed);
    execState->cmd = (char *)execState->cmd + *(unsigned __int16 *)execState->cmd;
}

void __cdecl RB_DrawQuadPicCmd(GfxRenderCommandExecState *execState)
{
    int indexCount; // [esp+20h] [ebp-10h]
    unsigned __int16 vertCount; // [esp+24h] [ebp-Ch]
    const GfxCmdDrawQuadPic *cmd; // [esp+28h] [ebp-8h]
    float w; // [esp+2Ch] [ebp-4h]

    cmd = (const GfxCmdDrawQuadPic *)execState->cmd;

    iassert(gfxCmdBufSourceState.viewMode == VIEW_MODE_2D);

    RB_SetTessTechnique(cmd->material, 4u);
    R_TrackPrims(&gfxCmdBufState.prim, GFX_PRIM_STATS_HUD);
    RB_CheckTessOverflow(4, 6);
    vertCount = tess.vertexCount;
    indexCount = tess.indexCount;
    tess.vertexCount += 4;
    tess.indexCount += 6;
    tess.indices[indexCount] = vertCount + 3;
    tess.indices[indexCount + 1] = vertCount;
    tess.indices[indexCount + 2] = vertCount + 2;
    tess.indices[indexCount + 3] = vertCount + 2;
    tess.indices[indexCount + 4] = vertCount;
    tess.indices[indexCount + 5] = vertCount + 1;
    w = cmd->w;
    R_SetVertex4d(&tess.verts[vertCount], cmd->verts[0][0], cmd->verts[0][1], 0.0, w, 0.0, 0.0, cmd->color.packed);
    R_SetVertex4d(&tess.verts[vertCount + 1], cmd->verts[1][0], cmd->verts[1][1], 0.0, w, 1.0, 0.0, cmd->color.packed);
    R_SetVertex4d(&tess.verts[vertCount + 2], cmd->verts[2][0], cmd->verts[2][1], 0.0, w, 1.0, 1.0, cmd->color.packed);
    R_SetVertex4d(&tess.verts[vertCount + 3], cmd->verts[3][0], cmd->verts[3][1], 0.0, w, 0.0, 1.0, cmd->color.packed);
    execState->cmd = (char *)execState->cmd + *(unsigned __int16 *)execState->cmd;
}

void __cdecl RB_DrawQuadList2DCmd(GfxRenderCommandExecState *execState)
{
    int n; // [esp+18h] [ebp-18h]
    int indexCount; // [esp+1Ch] [ebp-14h]
    GfxQuadVertex *verts; // [esp+20h] [ebp-10h]
    unsigned __int16 vertCount; // [esp+24h] [ebp-Ch]
    const GfxCmdDrawQuadList2D *cmd; // [esp+28h] [ebp-8h]
    int quadCount; // [esp+2Ch] [ebp-4h]

    cmd = (const GfxCmdDrawQuadList2D *)execState->cmd;

    iassert(gfxCmdBufSourceState.viewMode == VIEW_MODE_2D);

    RB_SetTessTechnique(cmd->material, 4u);
    R_TrackPrims(&gfxCmdBufState.prim, GFX_PRIM_STATS_HUD);
    quadCount = cmd->quadCount;
    RB_CheckTessOverflow(4 * quadCount, 6 * quadCount);
    vertCount = tess.vertexCount;
    indexCount = tess.indexCount;
    tess.vertexCount += 4 * quadCount;
    tess.indexCount += 6 * quadCount;
    verts = (GfxQuadVertex *)&cmd[1];
    for ( n = 0; n < quadCount; ++n )
    {
        tess.indices[indexCount] = vertCount + 3;
        tess.indices[indexCount + 1] = vertCount;
        tess.indices[indexCount + 2] = vertCount + 2;
        tess.indices[indexCount + 3] = vertCount + 2;
        tess.indices[indexCount + 4] = vertCount;
        tess.indices[indexCount + 5] = vertCount + 1;
        R_SetVertex2d(&tess.verts[vertCount], verts->xy[0], verts->xy[1], verts->st[0], verts->st[1], verts->color.packed);
        R_SetVertex2d(
            &tess.verts[vertCount + 1],
            verts[1].xy[0],
            verts[1].xy[1],
            verts[1].st[0],
            verts[1].st[1],
            verts[1].color.packed);
        R_SetVertex2d(
            &tess.verts[vertCount + 2],
            verts[2].xy[0],
            verts[2].xy[1],
            verts[2].st[0],
            verts[2].st[1],
            verts[2].color.packed);
        R_SetVertex2d(
            &tess.verts[vertCount + 3],
            verts[3].xy[0],
            verts[3].xy[1],
            verts[3].st[0],
            verts[3].st[1],
            verts[3].color.packed);
        verts += 4;
        vertCount += 4;
        indexCount += 6;
    }
    execState->cmd = (char *)execState->cmd + *(unsigned __int16 *)execState->cmd;
}

void __cdecl RB_DrawEmblemLayer(GfxRenderCommandExecState *execState)
{
  int indexCount; // [esp+20h] [ebp-Ch]
  unsigned __int16 vertCount; // [esp+24h] [ebp-8h]
  const GfxCmdDrawEmblemLayer *cmd; // [esp+28h] [ebp-4h]

  cmd = (const GfxCmdDrawEmblemLayer *)execState->cmd;

  iassert(gfxCmdBufSourceState.viewMode == VIEW_MODE_2D);

  RB_SetTessTechnique(cmd->material, 4u);
  R_SetCodeImageTexture(&gfxCmdBufSourceState, 0x22u, cmd->image);
  R_SetCodeImageSamplerState(&gfxCmdBufSourceState, 0x22u, 0x62u);
  R_UpdateCodeConstant(&gfxCmdBufSourceState, CONST_SRC_CODE_EMBLEM_LUT_SELECTOR, (float)cmd->colorIdx, cmd->smoothSize, cmd->outlineSize, 0.0);
  R_TrackPrims(&gfxCmdBufState.prim, GFX_PRIM_STATS_CODE);
  RB_CheckTessOverflow(4, 6);
  vertCount = tess.vertexCount;
  indexCount = tess.indexCount;
  tess.vertexCount += 4;
  tess.indexCount += 6;
  tess.indices[indexCount] = vertCount + 3;
  tess.indices[indexCount + 1] = vertCount;
  tess.indices[indexCount + 2] = vertCount + 2;
  tess.indices[indexCount + 3] = vertCount + 2;
  tess.indices[indexCount + 4] = vertCount;
  tess.indices[indexCount + 5] = vertCount + 1;
  R_SetVertex2d(
    &tess.verts[vertCount],
    cmd->verts[0].xy[0],
    cmd->verts[0].xy[1],
    cmd->verts[0].st[0],
    cmd->verts[0].st[1],
    cmd->verts[0].color.packed);
  R_SetVertex2d(
    &tess.verts[vertCount + 1],
    cmd->verts[1].xy[0],
    cmd->verts[1].xy[1],
    cmd->verts[1].st[0],
    cmd->verts[1].st[1],
    cmd->verts[1].color.packed);
  R_SetVertex2d(
    &tess.verts[vertCount + 2],
    cmd->verts[2].xy[0],
    cmd->verts[2].xy[1],
    cmd->verts[2].st[0],
    cmd->verts[2].st[1],
    cmd->verts[2].color.packed);
  R_SetVertex2d(
    &tess.verts[vertCount + 3],
    cmd->verts[3].xy[0],
    cmd->verts[3].xy[1],
    cmd->verts[3].st[0],
    cmd->verts[3].st[1],
    cmd->verts[3].color.packed);
  RB_EndTessSurface();
  execState->cmd = (char *)execState->cmd + *(unsigned __int16 *)execState->cmd;
}

void __cdecl RB_DrawFullScreenColoredQuadCmd(GfxRenderCommandExecState *execState)
{
  RB_DrawFullScreenColoredQuad(
    *((const Material **)execState->cmd + 1),
    *((float *)execState->cmd + 2),
    *((float *)execState->cmd + 3),
    *((float *)execState->cmd + 4),
    *((float *)execState->cmd + 5),
    *((unsigned int *)execState->cmd + 6));
  execState->cmd = (char *)execState->cmd + *(unsigned __int16 *)execState->cmd;
}

void __cdecl RB_StretchRawCmd(GfxRenderCommandExecState *execState)
{
    RB_StretchRaw(
        *((unsigned int *)execState->cmd + 1),
        *((unsigned int *)execState->cmd + 2),
        *((unsigned int *)execState->cmd + 3),
        *((unsigned int *)execState->cmd + 4),
        *((unsigned int *)execState->cmd + 5),
        *((unsigned int *)execState->cmd + 6),
        *((const unsigned __int8 **)execState->cmd + 7));
    execState->cmd = (char *)execState->cmd + *(unsigned __int16 *)execState->cmd;
}

void __cdecl RB_StretchRaw(int x, int y, int w, int h, int cols, int rows, const unsigned __int8 *data)
{
    const char *v7; // eax
    const char *v8; // eax
    const char *v9; // eax
    int v11; // [esp+Ch] [ebp-40h]
    int v13; // [esp+14h] [ebp-38h]
    int semaphore; // [esp+18h] [ebp-34h]
    int hr; // [esp+1Ch] [ebp-30h]
    _D3DLOCKED_RECT lockedRect; // [esp+20h] [ebp-2Ch] BYREF
    IDirect3DSurface9 *rawSurf; // [esp+28h] [ebp-24h] BYREF
    unsigned __int8 *dest; // [esp+2Ch] [ebp-20h]
    tagRECT dstRect; // [esp+30h] [ebp-1Ch] BYREF
    int colIndex; // [esp+40h] [ebp-Ch]
    int newline; // [esp+44h] [ebp-8h]
    int rowIndex; // [esp+48h] [ebp-4h]

    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("dx.device->CreateOffscreenPlainSurface( cols, rows, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &rawSurf, 0 )\n");
    //hr = ((int (__thiscall *)(IDirect3DDevice9 *, IDirect3DDevice9 *, int, int, int, unsigned int, IDirect3DSurface9 **, unsigned int))dx.device->CreateOffscreenPlainSurface)(
    //             dx.device,
    //             dx.device,
    //             cols,
    //             rows,
    //             22,
    //             0,
    //             &rawSurf,
    //             0);
    hr = dx.device->CreateOffscreenPlainSurface(cols, rows, (D3DFORMAT)22, (D3DPOOL)0, &rawSurf, 0);
    if ( hr < 0 )
    {
        ++g_disableRendering;
        v7 = R_ErrorDescription(hr);
        Com_Error(
            ERR_FATAL,
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp (%i) dx.device->CreateOffscreenPlainSurface( cols, rows"
            ", D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &rawSurf, 0 ) failed: %s\n",
            1787,
            v7);
    }
    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("rawSurf->LockRect( &lockedRect, 0, 0x00002000L )\n");
    //v13 = ((int (__thiscall *)(IDirect3DSurface9 *, IDirect3DSurface9 *, _D3DLOCKED_RECT *, unsigned int, int))rawSurf->LockRect)(
    //                rawSurf,
    //                rawSurf,
    //                &lockedRect,
    //                0,
    //                0x2000);
    v13 = rawSurf->LockRect(&lockedRect, 0, 0x2000);
    if ( v13 < 0 )
    {
        ++g_disableRendering;
        v8 = R_ErrorDescription(v13);
        Com_Error(
            ERR_FATAL,
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp (%i) rawSurf->LockRect( &lockedRect, 0, 0x00002000L ) failed: %s\n",
            1789,
            v8);
    }
    dest = (unsigned __int8 *)lockedRect.pBits;
    newline = lockedRect.Pitch - 4 * cols;
    for ( rowIndex = 0; rowIndex < rows; ++rowIndex )
    {
        for ( colIndex = 0; colIndex < cols; ++colIndex )
        {
            Byte4CopyRgbaToVertexColor(data, dest);
            data += 4;
            dest += 4;
        }
        dest += newline;
    }
    //((void (__thiscall *)(IDirect3DSurface9 *, IDirect3DSurface9 *))rawSurf->UnlockRect)(rawSurf, rawSurf);
    rawSurf->UnlockRect();
    dstRect.left = x;
    dstRect.top = y;
    dstRect.right = w + x;
    dstRect.bottom = h + y;
    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint(
            "dx.device->StretchRect( rawSurf, 0, gfxRenderTargets[R_RENDERTARGET_FRAME_BUFFER].surface.color, &dstRect, D3DTEXF_LINEAR )\n");
    v11 = dx.device->StretchRect(rawSurf, 0, gfxRenderTargets[R_RENDERTARGET_FRAME_BUFFER].surface.color, &dstRect, D3DTEXF_LINEAR);
    if ( v11 < 0 )
    {
        ++g_disableRendering;
        v9 = R_ErrorDescription(v11);
        Com_Error(
            ERR_FATAL,
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp (%i) dx.device->StretchRect( rawSurf, 0, gfxRenderTarge"
            "ts[R_RENDERTARGET_FRAME_BUFFER].surface.color, &dstRect, D3DTEXF_LINEAR ) failed: %s\n",
            1808,
            v9);
    }
    rawSurf->Release();
}

void __cdecl RB_DrawFramedCmd(GfxRenderCommandExecState *execState)
{
    float v1; // xmm0_4
    float v2; // xmm0_4
    float t[4]; // [esp+18h] [ebp-50h]
    unsigned __int16 vertexCount; // [esp+28h] [ebp-40h]
    int row; // [esp+2Ch] [ebp-3Ch]
    float s[4]; // [esp+30h] [ebp-38h]
    float x[4]; // [esp+40h] [ebp-28h]
    float y[4]; // [esp+50h] [ebp-18h]
    const GfxCmdDrawFramed2D *cmd; // [esp+60h] [ebp-8h]
    int col; // [esp+64h] [ebp-4h]

    cmd = (const GfxCmdDrawFramed2D *)execState->cmd;

    iassert(gfxCmdBufSourceState.viewMode == VIEW_MODE_2D);

    RB_SetTessTechnique(cmd->material, 4u);
    R_TrackPrims(&gfxCmdBufState.prim, GFX_PRIM_STATS_HUD);
    RB_CheckTessOverflow(16, 48);
    vertexCount = tess.vertexCount;
    x[0] = cmd->x;
    x[3] = cmd->x + cmd->w;
    if ( (cmd->sides & 1) != 0 )
        x[1] = cmd->x + cmd->thicknessW;
    else
        x[1] = cmd->x;
    if ( (cmd->sides & 2) != 0 )
        v1 = (float)(cmd->x + cmd->w) - cmd->thicknessW;
    else
        v1 = cmd->x + cmd->w;
    x[2] = v1;
    y[0] = cmd->y;
    y[3] = cmd->y + cmd->h;
    if ( (cmd->sides & 4) != 0 )
        y[1] = cmd->y + cmd->thicknessH;
    else
        y[1] = cmd->y;
    if ( (cmd->sides & 8) != 0 )
        v2 = (float)(cmd->y + cmd->h) - cmd->thicknessH;
    else
        v2 = cmd->y + cmd->h;
    y[2] = v2;
    s[0] = 0.0f;
    s[3] = 1.0f;
    s[1] = cmd->thicknessTex;
    s[2] = 1.0 - cmd->thicknessTex;
    t[0] = 0.0f;
    t[3] = 1.0f;
    t[1] = cmd->thicknessTex;
    t[2] = 1.0 - cmd->thicknessTex;
    for ( row = 0; row < 4; ++row )
    {
        for ( col = 0; col < 4; ++col )
            R_SetVertex2d(&tess.verts[vertexCount++], x[col], y[row], s[col], t[row], cmd->color.packed);
    }
    if ( (cmd->sides & 4) != 0 )
    {
        if ( (cmd->sides & 1) != 0 )
            TessQuad(0, 1u, 5u, 4u);
        TessQuad(1u, 2u, 6u, 5u);
        if ( (cmd->sides & 2) != 0 )
            TessQuad(2u, 3u, 7u, 6u);
    }
    if ( (cmd->sides & 1) != 0 )
        TessQuad(4u, 5u, 9u, 8u);
    TessQuad(5u, 6u, 0xAu, 9u);
    if ( (cmd->sides & 2) != 0 )
        TessQuad(6u, 7u, 0xBu, 0xAu);
    if ( (cmd->sides & 8) != 0 )
    {
        if ( (cmd->sides & 1) != 0 )
            TessQuad(8u, 9u, 0xDu, 0xCu);
        TessQuad(9u, 0xAu, 0xEu, 0xDu);
        if ( (cmd->sides & 2) != 0 )
            TessQuad(0xAu, 0xBu, 0xFu, 0xEu);
    }
    tess.vertexCount = vertexCount;
    execState->cmd = (char *)execState->cmd + *(unsigned __int16 *)execState->cmd;
}

void __cdecl TessQuad(unsigned __int16 v0, unsigned __int16 v1, unsigned __int16 v2, unsigned __int16 v3)
{
    tess.indices[tess.indexCount] = v0 + LOWORD(tess.vertexCount);
    tess.indices[tess.indexCount + 1] = v1 + LOWORD(tess.vertexCount);
    tess.indices[tess.indexCount + 2] = v2 + LOWORD(tess.vertexCount);
    tess.indexCount += 3;
    tess.indices[tess.indexCount] = v0 + LOWORD(tess.vertexCount);
    tess.indices[tess.indexCount + 1] = v2 + LOWORD(tess.vertexCount);
    tess.indices[tess.indexCount + 2] = v3 + LOWORD(tess.vertexCount);
    tess.indexCount += 3;
}

void __cdecl RB_SetEyeOffsetConstant(GfxCmdBufSourceState *source, const float *drawSurfListViewOrigin)
{
    float eyeOffset[4]; // [esp+8h] [ebp-10h] BYREF

    if ( drawSurfListViewOrigin[3] == 0.0 )
    {
        memset(eyeOffset, 0, sizeof(eyeOffset));
    }
    else
    {
        eyeOffset[0] = *drawSurfListViewOrigin;
        eyeOffset[1] = drawSurfListViewOrigin[1];
        eyeOffset[2] = drawSurfListViewOrigin[2];
        eyeOffset[3] = drawSurfListViewOrigin[3];
    }
    R_UpdateCodeConstantFromVec4(source, CONST_SRC_CODE_EYEOFFSET, eyeOffset);
}

void __cdecl R_DrawSurfs(GfxCmdBufContext context, GfxCmdBufState *prepassState, const GfxDrawSurfListInfo *info)
{
    PROF_SCOPED("R_DrawSurfs");

    float v3; // [esp+10h] [ebp-58h]
    float v4; // [esp+14h] [ebp-54h]
    float v5; // [esp+18h] [ebp-50h]
    GfxViewport viewport; // [esp+30h] [ebp-38h] BYREF
    GfxCmdBufContext prepassContext; // [esp+40h] [ebp-28h]
    GfxDrawSurfListArgs listArgs; // [esp+48h] [ebp-20h] BYREF
    unsigned int drawMatCount; // [esp+58h] [ebp-10h]
    unsigned int processedDrawSurfCount; // [esp+5Ch] [ebp-Ch]
    unsigned int drawSurfCount; // [esp+60h] [ebp-8h]
    const GfxUI3DBackend *localUI3DBackend; // [esp+64h] [ebp-4h]

    prepassContext.source = 0;
    prepassContext.state = 0;

    iassert(context.source->cameraView == info->cameraView);

    context.state->origMaterial = 0;
    R_SetDrawSurfsShadowableLight(context.source, info);
    R_Set3D(context.source);

    prepassContext.source = prepassState != 0 ? context.source : 0;
    prepassContext.state = prepassState;

    if (context.source->viewportIsDirty)
    {
        R_GetViewport(context.source, &viewport);
        R_SetViewport(context.state, &viewport);
        if (prepassState)
            R_SetViewport(prepassContext.state, &viewport);
        R_UpdateViewport(context.source, &viewport);
    }

    iassert(dx.d3d9 && dx.device);

    //if (context.state->prim.device != dx.device)
    //    PIXBeginNamedEvent(-1, "draw surf");

    R_BeginPixMaterials(context.state);
    if (prepassContext.state)
        R_BeginPixMaterials(prepassContext.state);
    drawSurfCount = info->drawSurfCount;
    R_SetWaterSimulationConstants(context.source, 0.0);

    R_SetCodeConstant(context.source, CONST_SRC_CODE_DESTRUCTIBLE_PARMS, 0.0f, 0.0f, 0.0f, 0.0f);
    R_SetCodeConstant(context.source, CONST_SRC_CODE_CHARACTER_CHARRED_AMOUNT, 0.0f, 0.0f, 0.0f, 0.0f);
    R_SetCodeConstant(context.source, CONST_SRC_CODE_CROSSFADE_PARMS, 1.0f, 1.0f, 1.0f, 1.0f);
    R_SetCodeConstant(context.source, CONST_SRC_CODE_CHARACTER_DISSOLVE_COLOR, 0.0f, 0.0f, 0.0f, 0.0f);

    R_FoliageSetDefaultShaderConstants(context.source);

    R_SetCodeConstantFromVec4(context.source,
        CONST_SRC_CODE_CHARACTER_DISSOLVE_COLOR,
        r_swrk_override_characterDissolveColor->current.vector);

    localUI3DBackend = 0;
    if (info->viewInfo)
        localUI3DBackend = &info->viewInfo->rbUI3D;

    RB_UI3D_SetShaderConstants(context.source, localUI3DBackend);
    RB_SetCustomWindConstants(context.source, info->viewOrigin);
    RB_SetEyeOffsetConstant(context.source, info->viewOrigin);

    listArgs.context = context;
    listArgs.firstDrawSurfIndex = 0;
    listArgs.info = info;

    drawMatCount = 0;
    while (listArgs.firstDrawSurfIndex != drawSurfCount)
    {
        processedDrawSurfCount = R_RenderDrawSurfListMaterial(&listArgs, prepassContext);
        listArgs.firstDrawSurfIndex += processedDrawSurfCount;
        ++drawMatCount;
    }

    context.state->prim.viewStats->drawSurfCount += drawSurfCount;
    context.state->prim.viewStats->drawMatCount += drawMatCount;
    R_EndPixMaterials(context.state);

    if (prepassContext.state)
        R_EndPixMaterials(prepassContext.state);

    //if (context.state->prim.device != dx.device && GetCurrentThreadId() == g_DXDeviceThread)
    //    D3DPERF_EndEvent();

    R_TessEnd(context, prepassContext);
    context.state->origMaterial = 0;
}

unsigned int __cdecl R_RenderDrawSurfListMaterial(const GfxDrawSurfListArgs *listArgs, GfxCmdBufContext prepassContext)
{
    PROF_SCOPED("RenderMatBatch");

    const ShaderConstantSet *v3; // eax
    unsigned int v5; // [esp+10h] [ebp-B8h]
    unsigned int v6; // [esp+14h] [ebp-B4h]
    unsigned int passIndex; // [esp+18h] [ebp-B0h]
    GfxCmdBufSourceState *passPrepassContext; // [esp+1Ch] [ebp-ACh]
    GfxCmdBufState *passPrepassContext_4; // [esp+20h] [ebp-A8h]
    unsigned int firstDrawSurfIndex; // [esp+24h] [ebp-A4h]
    GfxDrawSurf drawSurf; // [esp+28h] [ebp-A0h] BYREF
    unsigned int subListCount; // [esp+B4h] [ebp-14h]
    unsigned __int16 passCount; // [esp+B8h] [ebp-10h]
    const GfxDrawSurf *drawSurfList; // [esp+BCh] [ebp-Ch]
    bool isPixelCostEnabled; // [esp+C3h] [ebp-5h]
    unsigned int drawSurfCount; // [esp+C4h] [ebp-4h]

    firstDrawSurfIndex = listArgs->firstDrawSurfIndex;
    drawSurfCount = listArgs->info->drawSurfCount - firstDrawSurfIndex;
    drawSurfList = &listArgs->info->drawSurfs[firstDrawSurfIndex];
    iassert(*((unsigned int *)drawSurfList) != 0xb0b0b0b0);
    drawSurf.packed = drawSurfList->packed;
    v3 = RB_ShaderConstantSetFromDrawSurf(listArgs->context.source->input.data, &drawSurf);

    ScopedShaderConstantSetUndo shaderConstantUndo(listArgs->context.source, v3); // [esp+30h] [ebp-98h] BYREF
    //ScopedShaderConstantSetUndo::ScopedShaderConstantSetUndo(&shaderConstantUndo, listArgs->context.source, v3);

    if ( R_SetTechnique(listArgs->context, &prepassContext, listArgs->info, drawSurf) )
    {
        R_SetPixPrimarySortKey(listArgs->context.state, (drawSurf.packed >> 58) & 0x3F);
        R_BeginPixMaterial(listArgs->context.state);
        if ( prepassContext.state )
        {
            R_SetPixPrimarySortKey(prepassContext.state, (drawSurf.packed >> 58) & 0x3F);
            R_BeginPixMaterial(prepassContext.state);
        }
        isPixelCostEnabled = pixelCostMode != GFX_PIXEL_COST_MODE_OFF;
        if ( pixelCostMode )
            R_PixelCost_BeginSurface(listArgs->context);

        iassert(!prepassContext.state || (prepassContext.state->technique->passCount == 1));

        passPrepassContext = prepassContext.source;
        subListCount = 0;
        passCount = listArgs->context.state->technique->passCount;
        for ( passIndex = 0; passIndex < passCount; ++passIndex )
        {
            R_UpdateMaterialTime(listArgs->context.source, 0.0, 0.0, 0.0, 0.0);
            R_SetupPass(listArgs->context, passIndex);
            if ( passIndex || !prepassContext.state )
            {
                passPrepassContext_4 = 0;
            }
            else
            {
                R_SetupPass(prepassContext, 0);
                passPrepassContext_4 = prepassContext.state;
            }
            subListCount = ((int (__cdecl *)(const GfxDrawSurfListArgs *, GfxCmdBufSourceState *, GfxCmdBufState *))rb_tessTable[(drawSurf.packed >> 51) & 0xF])(
                                             listArgs,
                                             passPrepassContext,
                                             passPrepassContext_4);
        }

        if ( isPixelCostEnabled )
            R_PixelCost_EndSurface(listArgs->context);
        R_EndPixMaterial(listArgs->context.state);
        if ( prepassContext.state )
            R_EndPixMaterial(prepassContext.state);
        return subListCount;
    }
    else
    {
        return R_SkipDrawSurfListMaterial(drawSurfList, drawSurfCount);
    }
}

unsigned int __cdecl R_SkipDrawSurfListMaterial(const GfxDrawSurf *drawSurfList, unsigned int drawSurfCount)
{
    unsigned int subListCount; // [esp+14h] [ebp-Ch]

    for ( subListCount = 0;
                subListCount < drawSurfCount
         && (*(unsigned int *)&drawSurfList[subListCount].fields & 0xF00F0000) == (*(unsigned int *)&drawSurfList->fields & 0xF00F0000)
         && HIDWORD(drawSurfList[subListCount].packed) == HIDWORD(drawSurfList->packed);
                ++subListCount )
    {
        ;
    }
    return subListCount;
}

void __cdecl R_TessEnd(GfxCmdBufContext context, GfxCmdBufContext prepassContext)
{
    GfxCmdBufSourceState *commonSource = context.source;

    iassert(prepassContext.state == NULL || commonSource == prepassContext.source);

    commonSource->viewMode = VIEW_MODE_NONE;

    R_ChangeDepthHackNearClip(commonSource, 0);

    GfxDepthRangeType depthRangeType = commonSource->cameraView != 0 ? GFX_DEPTH_RANGE_SCENE : GFX_DEPTH_RANGE_FULL;

    if (depthRangeType != context.state->depthRangeType)
    {
        R_ChangeDepthRange(context.state, depthRangeType);
    }

    if (prepassContext.state)
    {
        depthRangeType = prepassContext.source->cameraView != 0 ? GFX_DEPTH_RANGE_SCENE : GFX_DEPTH_RANGE_FULL;
        if (depthRangeType != prepassContext.state->depthRangeType)
        {
            R_ChangeDepthRange(prepassContext.state, depthRangeType);
        }
    }
}

void __cdecl RB_ClearScreenCmd(GfxRenderCommandExecState *execState)
{
    if ( tess.indexCount )
        RB_EndTessSurface();
    R_ClearScreen(
        gfxCmdBufState.prim.device,
        *((_BYTE *)execState->cmd + 4),
        (const float *)execState->cmd + 3,
        *((float *)execState->cmd + 2),
        *((_BYTE *)execState->cmd + 5),
        0);
    execState->cmd = (char *)execState->cmd + *(unsigned __int16 *)execState->cmd;
}

void __cdecl RB_SetGammaRamp(const GfxGammaRamp *gammaTable)
{
    int colorIndex; // [esp+0h] [ebp-60Ch]
    _D3DGAMMARAMP d3dGammaRamp; // [esp+4h] [ebp-608h] BYREF

    if ( !gammaTable
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp",
                    2436,
                    0,
                    "%s",
                    "gammaTable != NULL") )
    {
        __debugbreak();
    }
    if ( !vidConfig.deviceSupportsGamma
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp",
                    2437,
                    0,
                    "%s",
                    "vidConfig.deviceSupportsGamma == true") )
    {
        __debugbreak();
    }
    if ( !dx.device
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp",
                    2438,
                    0,
                    "%s",
                    "dx.device != NULL") )
    {
        __debugbreak();
    }
    for ( colorIndex = 0; colorIndex < 256; ++colorIndex )
    {
        d3dGammaRamp.red[colorIndex] = gammaTable->entries[colorIndex];
        d3dGammaRamp.green[colorIndex] = gammaTable->entries[colorIndex];
        d3dGammaRamp.blue[colorIndex] = gammaTable->entries[colorIndex];
    }
    dx.device->SetGammaRamp(dx.windowCount, 0, &d3dGammaRamp);
}

void __cdecl RB_BlendSavedScreenFlashedCmd(GfxRenderCommandExecState *execState)
{
    execState->cmd = (char *)execState->cmd + *(unsigned __int16 *)execState->cmd;
}

void __cdecl RB_DrawPointsCmd(GfxRenderCommandExecState *execState)
{
    const GfxCmdDrawPoints *cmd; // [esp+4h] [ebp-4h]

    cmd = (const GfxCmdDrawPoints *)execState->cmd;
    if ( *((_BYTE *)execState->cmd + 7) == 2 )
    {
        RB_DrawPoints2D(cmd);
    }
    else
    {
        if ( cmd->dimensions != 3
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp",
                        2691,
                        1,
                        "%s\n\t(cmd->dimensions) = %i",
                        "(cmd->dimensions == 3)",
                        cmd->dimensions) )
        {
            __debugbreak();
        }
        RB_DrawPoints3D(cmd);
    }
    execState->cmd = (char *)execState->cmd + *(unsigned __int16 *)execState->cmd;
}

void __cdecl R_SetVertex4d_0(
    GfxVertex *vert,
    float x,
    float y,
    float z,
    float w,
    float s,
    float t,
    const unsigned __int8 *color)
{
    vert->xyzw[0] = x;
    vert->xyzw[1] = y;
    vert->xyzw[2] = z;
    vert->xyzw[3] = w;
    vert->normal.packed = 0x3FFE7F7F;
    vert->color.packed = *(_DWORD *)color;
    vert->texCoord[0] = s;
    vert->texCoord[1] = t;
}

void __cdecl RB_DrawPoints2D(const GfxCmdDrawPoints *cmd)
{
    float size; // [esp+1Ch] [ebp-Ch]
    int pointIndex; // [esp+20h] [ebp-8h]
    GfxPointVertex *v; // [esp+24h] [ebp-4h]

    iassert(gfxCmdBufSourceState.viewMode == VIEW_MODE_2D);

    RB_SetTessTechnique(rgp.whiteMaterial, 4u);
    R_TrackPrims(&gfxCmdBufState.prim, GFX_PRIM_STATS_DEBUG);
    size = (float)cmd->size * 0.5;
    pointIndex = 0;
    v = (GfxPointVertex*)cmd->verts;
    while ( pointIndex < cmd->pointCount )
    {
        RB_CheckTessOverflow(4, 6);
        tess.indices[tess.indexCount] = LOWORD(tess.vertexCount) + 1;
        tess.indices[tess.indexCount + 1] = tess.vertexCount;
        tess.indices[tess.indexCount + 2] = LOWORD(tess.vertexCount) + 2;
        tess.indices[tess.indexCount + 3] = LOWORD(tess.vertexCount) + 2;
        tess.indices[tess.indexCount + 4] = tess.vertexCount;
        tess.indices[tess.indexCount + 5] = LOWORD(tess.vertexCount) + 3;
        tess.indexCount += 6;
        R_SetVertex4d_0(
            &tess.verts[tess.vertexCount],
            v->xyz[0] - size,
            v->xyz[1] - size,
            v->xyz[2],
            1.0,
            0.0,
            0.0,
            v->color);
        R_SetVertex4d_0(
            &tess.verts[tess.vertexCount + 1],
            v->xyz[0] - size,
            v->xyz[1] + size,
            v->xyz[2],
            1.0,
            0.0,
            1.0,
            v->color);
        R_SetVertex4d_0(
            &tess.verts[tess.vertexCount + 2],
            v->xyz[0] + size,
            v->xyz[1] + size,
            v->xyz[2],
            1.0,
            1.0,
            1.0,
            v->color);
        R_SetVertex4d_0(
            &tess.verts[tess.vertexCount + 3],
            v->xyz[0] + size,
            v->xyz[1] - size,
            v->xyz[2],
            1.0,
            1.0,
            0.0,
            v->color);
        tess.vertexCount += 4;
        ++pointIndex;
        ++v;
    }
}

void __cdecl RB_DrawPoints3D(const GfxCmdDrawPoints *cmd)
{
    const float *transform; // [esp+1Ch] [ebp-2Ch]
    float xyz; // [esp+20h] [ebp-28h]
    float xyz_4; // [esp+24h] [ebp-24h]
    float xyz_8; // [esp+28h] [ebp-20h]
    float xyz_12; // [esp+2Ch] [ebp-1Ch]
    float invWidth; // [esp+30h] [ebp-18h]
    float invHeight; // [esp+34h] [ebp-14h]
    int pointIndex; // [esp+40h] [ebp-8h]
    GfxPointVertex *v; // [esp+44h] [ebp-4h]

    RB_SetTessTechnique(rgp.pointMaterial, 4u);
    R_TrackPrims(&gfxCmdBufState.prim, GFX_PRIM_STATS_DEBUG);
    RB_SetIdentity();
    transform = (const float *)&gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix;
    invWidth = (float)((float)cmd->size * 1.0) / (float)gfxCmdBufSourceState.renderTargetWidth;
    invHeight = (float)((float)cmd->size * 1.0) / (float)gfxCmdBufSourceState.renderTargetHeight;
    pointIndex = 0;
    v = (GfxPointVertex*)cmd->verts;
    while (pointIndex < cmd->pointCount)
    {
        xyz = (float)((float)((float)(v->xyz[0] * *transform) + (float)(v->xyz[1] * transform[4]))
            + (float)(v->xyz[2] * transform[8]))
            + transform[12];
        xyz_4 = (float)((float)((float)(v->xyz[0] * transform[1]) + (float)(v->xyz[1] * transform[5]))
            + (float)(v->xyz[2] * transform[9]))
            + transform[13];
        xyz_12 = (float)((float)((float)(v->xyz[0] * transform[3]) + (float)(v->xyz[1] * transform[7]))
            + (float)(v->xyz[2] * transform[11]))
            + transform[15];
        xyz_8 = (float)((float)((float)((float)(v->xyz[0] * transform[2]) + (float)(v->xyz[1] * transform[6]))
            + (float)(v->xyz[2] * transform[10]))
            + transform[14])
            - (float)(0.001 * xyz_12);
        RB_CheckTessOverflow(4, 6);
        tess.indices[tess.indexCount] = LOWORD(tess.vertexCount) + 3;
        tess.indices[tess.indexCount + 1] = tess.vertexCount;
        tess.indices[tess.indexCount + 2] = LOWORD(tess.vertexCount) + 2;
        tess.indices[tess.indexCount + 3] = LOWORD(tess.vertexCount) + 2;
        tess.indices[tess.indexCount + 4] = tess.vertexCount;
        tess.indices[tess.indexCount + 5] = LOWORD(tess.vertexCount) + 1;
        tess.indexCount += 6;
        R_SetVertex4d_0(
            &tess.verts[tess.vertexCount],
            xyz - (float)(invWidth * xyz_12),
            xyz_4 - (float)(invHeight * xyz_12),
            xyz_8,
            xyz_12,
            0.0,
            0.0,
            v->color);
        R_SetVertex4d_0(
            &tess.verts[tess.vertexCount + 1],
            xyz - (float)(invWidth * xyz_12),
            xyz_4 + (float)(invHeight * xyz_12),
            xyz_8,
            xyz_12,
            0.0,
            1.0,
            v->color);
        R_SetVertex4d_0(
            &tess.verts[tess.vertexCount + 2],
            xyz + (float)(invWidth * xyz_12),
            xyz_4 + (float)(invHeight * xyz_12),
            xyz_8,
            xyz_12,
            1.0,
            1.0,
            v->color);
        R_SetVertex4d_0(
            &tess.verts[tess.vertexCount + 3],
            xyz + (float)(invWidth * xyz_12),
            xyz_4 - (float)(invHeight * xyz_12),
            xyz_8,
            xyz_12,
            1.0,
            0.0,
            v->color);
        tess.vertexCount += 4;
        ++pointIndex;
        ++v;
    }
    RB_EndTessSurface();
}

void __cdecl RB_DrawLines2D(int count, int width, const GfxPointVertex *verts)
{
    float delta[2]; // [esp+24h] [ebp-14h] BYREF
    int lineIndex; // [esp+2Ch] [ebp-Ch]
    const GfxPointVertex *v[2]; // [esp+30h] [ebp-8h]

    if ( count <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp",
                    2710,
                    0,
                    "%s\n\t(count) = %i",
                    "(count > 0)",
                    count) )
    {
        __debugbreak();
    }

    iassert(gfxCmdBufSourceState.viewMode == VIEW_MODE_2D);

    RB_SetTessTechnique(rgp.whiteMaterial, 4u);
    R_TrackPrims(&gfxCmdBufState.prim, GFX_PRIM_STATS_DEBUG);
    for ( lineIndex = 0; lineIndex < count; ++lineIndex )
    {
        v[0] = &verts[2 * lineIndex];
        v[1] = &verts[2 * lineIndex + 1];
        delta[0] = v[1]->xyz[1] - v[0]->xyz[1];
        delta[1] = v[0]->xyz[0] - v[1]->xyz[0];
        Vec2Normalize(delta);
        delta[0] = delta[0] * 0.5;
        delta[1] = delta[1] * 0.5;
        RB_CheckTessOverflow(4, 6);
        tess.indices[tess.indexCount] = LOWORD(tess.vertexCount) + 1;
        tess.indices[tess.indexCount + 1] = tess.vertexCount;
        tess.indices[tess.indexCount + 2] = LOWORD(tess.vertexCount) + 2;
        tess.indices[tess.indexCount + 3] = LOWORD(tess.vertexCount) + 2;
        tess.indices[tess.indexCount + 4] = tess.vertexCount;
        tess.indices[tess.indexCount + 5] = LOWORD(tess.vertexCount) + 3;
        tess.indexCount += 6;
        R_SetVertex3d(
            &tess.verts[tess.vertexCount],
            v[0]->xyz[0] - delta[0],
            v[0]->xyz[1] - delta[1],
            v[0]->xyz[2],
            0.0,
            0.0,
            v[0]->color);
        R_SetVertex3d(
            &tess.verts[tess.vertexCount + 1],
            v[1]->xyz[0] - delta[0],
            v[1]->xyz[1] - delta[1],
            v[1]->xyz[2],
            0.0,
            1.0,
            v[1]->color);
        R_SetVertex3d(
            &tess.verts[tess.vertexCount + 2],
            v[1]->xyz[0] + delta[0],
            v[1]->xyz[1] + delta[1],
            v[1]->xyz[2],
            1.0,
            1.0,
            v[1]->color);
        R_SetVertex3d(
            &tess.verts[tess.vertexCount + 3],
            v[0]->xyz[0] + delta[0],
            v[0]->xyz[1] + delta[1],
            v[0]->xyz[2],
            1.0,
            0.0,
            v[0]->color);
        tess.vertexCount += 4;
    }
}
void __cdecl RB_DrawLines3D(int count, int width, const GfxPointVertex *verts, bool depthTest)
{
    const float *transform; // [esp+2Ch] [ebp-50h]
    float delta[2]; // [esp+30h] [ebp-4Ch] BYREF
    float xyz[2][4]; // [esp+38h] [ebp-44h]
    float invWidth; // [esp+58h] [ebp-24h]
    float invHeight; // [esp+5Ch] [ebp-20h]
    float offset[2][2]; // [esp+60h] [ebp-1Ch]
    int lineIndex; // [esp+70h] [ebp-Ch]
    const GfxPointVertex *v[2]; // [esp+74h] [ebp-8h]

    if (depthTest)
        RB_SetTessTechnique(rgp.lineMaterial, 4u);
    else
        RB_SetTessTechnique(rgp.lineMaterialNoDepth, 4u);
    R_TrackPrims(&gfxCmdBufState.prim, GFX_PRIM_STATS_DEBUG);
    RB_SetIdentity();
    transform = (const float *)&gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix;
    invWidth = (float)width / (float)gfxCmdBufSourceState.renderTargetWidth;
    invHeight = (float)width / (float)gfxCmdBufSourceState.renderTargetHeight;
    for (lineIndex = 0; lineIndex < count; ++lineIndex)
    {
        v[0] = &verts[2 * lineIndex];
        xyz[0][0] = (float)((float)((float)(v[0]->xyz[0] * *transform) + (float)(v[0]->xyz[1] * transform[4]))
            + (float)(v[0]->xyz[2] * transform[8]))
            + transform[12];
        xyz[0][1] = (float)((float)((float)(v[0]->xyz[0] * transform[1]) + (float)(v[0]->xyz[1] * transform[5]))
            + (float)(v[0]->xyz[2] * transform[9]))
            + transform[13];
        xyz[0][2] = (float)((float)((float)(v[0]->xyz[0] * transform[2]) + (float)(v[0]->xyz[1] * transform[6]))
            + (float)(v[0]->xyz[2] * transform[10]))
            + transform[14];
        xyz[0][3] = (float)((float)((float)(v[0]->xyz[0] * transform[3]) + (float)(v[0]->xyz[1] * transform[7]))
            + (float)(v[0]->xyz[2] * transform[11]))
            + transform[15];
        v[1] = &verts[2 * lineIndex + 1];
        xyz[1][0] = (float)((float)((float)(v[1]->xyz[0] * *transform) + (float)(v[1]->xyz[1] * transform[4]))
            + (float)(v[1]->xyz[2] * transform[8]))
            + transform[12];
        xyz[1][1] = (float)((float)((float)(v[1]->xyz[0] * transform[1]) + (float)(v[1]->xyz[1] * transform[5]))
            + (float)(v[1]->xyz[2] * transform[9]))
            + transform[13];
        xyz[1][2] = (float)((float)((float)(v[1]->xyz[0] * transform[2]) + (float)(v[1]->xyz[1] * transform[6]))
            + (float)(v[1]->xyz[2] * transform[10]))
            + transform[14];
        xyz[1][3] = (float)((float)((float)(v[1]->xyz[0] * transform[3]) + (float)(v[1]->xyz[1] * transform[7]))
            + (float)(v[1]->xyz[2] * transform[11]))
            + transform[15];
        delta[0] = (float)(xyz[1][1] * xyz[0][3]) - (float)(xyz[0][1] * xyz[1][3]);
        delta[1] = (float)(xyz[0][0] * xyz[1][3]) - (float)(xyz[1][0] * xyz[0][3]);
        Vec2Normalize(delta);
        delta[0] = delta[0] * invWidth;
        delta[1] = delta[1] * invHeight;
        offset[0][0] = xyz[0][3] * delta[0];
        offset[0][1] = xyz[0][3] * delta[1];
        offset[1][0] = xyz[1][3] * delta[0];
        offset[1][1] = xyz[1][3] * delta[1];
        RB_CheckTessOverflow(4, 6);
        tess.indices[tess.indexCount] = LOWORD(tess.vertexCount) + 3;
        tess.indices[tess.indexCount + 1] = tess.vertexCount;
        tess.indices[tess.indexCount + 2] = LOWORD(tess.vertexCount) + 2;
        tess.indices[tess.indexCount + 3] = LOWORD(tess.vertexCount) + 2;
        tess.indices[tess.indexCount + 4] = tess.vertexCount;
        tess.indices[tess.indexCount + 5] = LOWORD(tess.vertexCount) + 1;
        tess.indexCount += 6;
        R_SetVertex4d_0(
            &tess.verts[tess.vertexCount],
            xyz[0][0] - offset[0][0],
            xyz[0][1] - offset[0][1],
            xyz[0][2],
            xyz[0][3],
            0.0,
            0.0,
            v[0]->color);
        R_SetVertex4d_0(
            &tess.verts[tess.vertexCount + 1],
            xyz[1][0] - offset[1][0],
            xyz[1][1] - offset[1][1],
            xyz[1][2],
            xyz[1][3],
            0.0,
            1.0,
            v[1]->color);
        R_SetVertex4d_0(
            &tess.verts[tess.vertexCount + 2],
            xyz[1][0] + offset[1][0],
            xyz[1][1] + offset[1][1],
            xyz[1][2],
            xyz[1][3],
            1.0,
            1.0,
            v[1]->color);
        R_SetVertex4d_0(
            &tess.verts[tess.vertexCount + 3],
            xyz[0][0] + offset[0][0],
            xyz[0][1] + offset[0][1],
            xyz[0][2],
            xyz[0][3],
            1.0,
            0.0,
            v[0]->color);
        tess.vertexCount += 4;
    }
    RB_EndTessSurface();
}

void __cdecl RB_DrawLinesCmd(GfxRenderCommandExecState *execState)
{
    const GfxCmdDrawLines *cmd; // [esp+4h] [ebp-4h]

    cmd = (const GfxCmdDrawLines *)execState->cmd;
    if ( *((_BYTE *)execState->cmd + 7) == 2 )
    {
        RB_DrawLines2D(cmd->lineCount, cmd->width, cmd->verts);
    }
    else
    {
        if ( cmd->dimensions != 3
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp",
                        2831,
                        1,
                        "%s\n\t(cmd->dimensions) = %i",
                        "(cmd->dimensions == 3)",
                        cmd->dimensions) )
        {
            __debugbreak();
        }
        RB_DrawLines3D(cmd->lineCount, cmd->width, cmd->verts, 1);
    }
    execState->cmd = (char *)execState->cmd + *(unsigned __int16 *)execState->cmd;
}

void __cdecl RB_DrawTrianglesCmd(GfxRenderCommandExecState *execState)
{
    int stOffset; // [esp+4h] [ebp-3Ch]
    int normalOffset; // [esp+2Ch] [ebp-14h]
    int normalSize; // [esp+30h] [ebp-10h]

    normalOffset = 16 * *((__int16 *)execState->cmd + 6) + 16;
    normalSize = 12 * *((__int16 *)execState->cmd + 6);
    stOffset = 4 * *((__int16 *)execState->cmd + 6) + normalSize + normalOffset;
    RB_DrawTriangles_Internal(
        *((const Material **)execState->cmd + 1),
        *((_BYTE *)execState->cmd + 8),
        *((_WORD *)execState->cmd + 5),
        (const unsigned __int16 *)((char *)execState->cmd + 8 * *((__int16 *)execState->cmd + 6) + stOffset),
        *((_WORD *)execState->cmd + 6),
        (const float (*)[4])((const float *)execState->cmd + 1),
        (const float (*)[3])((char *)execState->cmd + normalOffset),
        (const GfxColor *)((char *)execState->cmd + normalSize + normalOffset),
        (const float (*)[2])((char *)execState->cmd + stOffset));
    execState->cmd = (char *)execState->cmd + *(unsigned __int16 *)execState->cmd;
}

void __cdecl RB_DrawTriangles_Internal(
        const Material *material,
        unsigned __int8 techType,
        __int16 indexCount,
        const unsigned __int16 *indices,
        __int16 vertexCount,
        const float (*xyzw)[4],
        const float (*normal)[3],
        const GfxColor *color,
        const float (*st)[2])
{
  int index; // [esp+28h] [ebp-4h]
  int indexa; // [esp+28h] [ebp-4h]

  if ( tess.indexCount )
    RB_EndTessSurface();
  R_Set3D(&gfxCmdBufSourceState);
  RB_SetTessTechnique(material, techType);
  R_TrackPrims(&gfxCmdBufState.prim, GFX_PRIM_STATS_DEBUG);
  RB_CheckTessOverflow(vertexCount, indexCount);
  for ( index = 0; index < indexCount; ++index )
    tess.indices[index + tess.indexCount] = LOWORD(tess.vertexCount) + indices[index];
  for ( indexa = 0; indexa < vertexCount; ++indexa )
    R_SetVertex4dWithNormal(
      &tess.verts[indexa + tess.vertexCount],
      (*xyzw)[4 * indexa],
      (*xyzw)[4 * indexa + 1],
      (*xyzw)[4 * indexa + 2],
      (*xyzw)[4 * indexa + 3],
      (*normal)[3 * indexa],
      (*normal)[3 * indexa + 1],
      (*normal)[3 * indexa + 2],
      (*st)[2 * indexa],
      (*st)[2 * indexa + 1],
      (const unsigned __int8 *)&color[indexa]);
  tess.indexCount += indexCount;
  tess.vertexCount += vertexCount;
  RB_EndTessSurface();
}

void __cdecl R_SetVertex4dWithNormal(
                GfxVertex *vert,
                float x,
                float y,
                float z,
                float w,
                float nx,
                float ny,
                float nz,
                float s,
                float t,
                const unsigned __int8 *color)
{
    PackedUnitVec v11; // [esp+4h] [ebp-28h]

    vert->xyzw[0] = x;
    vert->xyzw[1] = y;
    vert->xyzw[2] = z;
    vert->xyzw[3] = w;
    v11.array[0] = (int)(float)((float)(nx * 127.0) + 127.5);
    v11.array[1] = (int)(float)((float)(ny * 127.0) + 127.5);
    v11.array[2] = (int)(float)((float)(nz * 127.0) + 127.5);
    v11.array[3] = 63;
    vert->normal = v11;
    vert->color.packed = *(unsigned int *)color;
    vert->texCoord[0] = s;
    vert->texCoord[1] = t;
}

void __cdecl RB_SetCustomConstantCmd(GfxRenderCommandExecState *execState)
{
  const GfxCmdSetCustomConstant *cmd; // [esp+8h] [ebp-4h]

  cmd = (const GfxCmdSetCustomConstant *)execState->cmd;
  if ( tess.indexCount )
    RB_EndTessSurface();
  R_SetCodeConstantFromVec4(&gfxCmdBufSourceState, cmd->type, (float*)cmd->vec);
  execState->cmd = (char *)execState->cmd + *(unsigned __int16 *)execState->cmd;
}

void __cdecl RB_SetMaterialColorCmd(GfxRenderCommandExecState *execState)
{
  const GfxCmdSetMaterialColor *cmd; // [esp+8h] [ebp-4h]

  cmd = (const GfxCmdSetMaterialColor *)execState->cmd;
  if ( tess.indexCount )
    RB_EndTessSurface();
  R_SetCodeConstantFromVec4(&gfxCmdBufSourceState, CONST_SRC_CODE_MATERIAL_COLOR, (float*)cmd->color);
  execState->cmd = (char *)execState->cmd + *(unsigned __int16 *)execState->cmd;
}

void __cdecl RB_SetViewportCmd(GfxRenderCommandExecState *execState)
{
  if ( tess.indexCount )
    RB_EndTessSurface();
  R_SetViewportStruct(&gfxCmdBufSourceState, (const GfxViewport *)((char *)execState->cmd + 4));
  execState->cmd = (char *)execState->cmd + *(unsigned __int16 *)execState->cmd;
}

void __cdecl RB_SetScissorCmd(GfxRenderCommandExecState *execState)
{
  if ( tess.indexCount )
    RB_EndTessSurface();
  if ( *((unsigned int *)execState->cmd + 1) )
    R_SetScissorStruct(&gfxCmdBufSourceState, (const GfxViewport *)((char *)execState->cmd + 8));
  else
    R_ClearScissorStruct(&gfxCmdBufSourceState);
  execState->cmd = (char *)execState->cmd + *(unsigned __int16 *)execState->cmd;
}

void __cdecl RB_ResolveCompositeCmd(GfxRenderCommandExecState *execState)
{
    const GfxCmdResolveComposite *cmd; // [esp+4h] [ebp-4h]

    if (tess.indexCount)
        RB_EndTessSurface();
    cmd = (const GfxCmdResolveComposite *)execState->cmd;
    R_SetRenderTargetSize(&gfxCmdBufSourceState, R_RENDERTARGET_FRAME_BUFFER);
    R_SetRenderTarget(gfxCmdBufContext, R_RENDERTARGET_FRAME_BUFFER);
    cmd->callback(gfxRenderTargets[R_RENDERTARGET_COMPOSITE].image);
    execState->cmd = (char *)execState->cmd + *(unsigned __int16 *)execState->cmd;
}

void __cdecl RB_PCCopyImageGenMIPCmd(GfxRenderCommandExecState *execState)
{
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    const char *v9; // eax
    const char *v10; // eax
    const char *v11; // eax
    int v13; // [esp+8h] [ebp-E0h]
    int v15; // [esp+10h] [ebp-D8h]
    int v17; // [esp+18h] [ebp-D0h]
    int v19; // [esp+20h] [ebp-C8h]
    int xx; // [esp+24h] [ebp-C4h]
    int yy; // [esp+28h] [ebp-C0h]
    int r; // [esp+2Ch] [ebp-BCh]
    int g; // [esp+30h] [ebp-B8h]
    int b; // [esp+34h] [ebp-B4h]
    int a; // [esp+38h] [ebp-B0h]
    int x; // [esp+3Ch] [ebp-ACh]
    unsigned __int8 *in; // [esp+40h] [ebp-A8h]
    int y; // [esp+44h] [ebp-A4h]
    int v30; // [esp+4Ch] [ebp-9Ch]
    int v32; // [esp+54h] [ebp-94h]
    unsigned __int8 *out; // [esp+58h] [ebp-90h]
    unsigned __int8 *outa; // [esp+58h] [ebp-90h]
    unsigned __int8 *linear; // [esp+5Ch] [ebp-8Ch]
    int step; // [esp+60h] [ebp-88h]
    unsigned int destMipLevel; // [esp+64h] [ebp-84h]
    signed int v39; // [esp+6Ch] [ebp-7Ch]
    int v41; // [esp+74h] [ebp-74h]
    int v43; // [esp+7Ch] [ebp-6Ch]
    int v45; // [esp+84h] [ebp-64h]
    int semaphore; // [esp+88h] [ebp-60h]
    int hr; // [esp+8Ch] [ebp-5Ch]
    signed int outHeight; // [esp+94h] [ebp-54h]
    IDirect3DTexture9 *resolveTexture; // [esp+98h] [ebp-50h] BYREF
    IDirect3DSurface9 *resolveSurface; // [esp+9Ch] [ebp-4Ch] BYREF
    IDirect3DSurface9 *renderSurface; // [esp+A0h] [ebp-48h]
    _D3DLOCKED_RECT srcRect; // [esp+A4h] [ebp-44h] BYREF
    _D3DLOCKED_RECT mipRect; // [esp+ACh] [ebp-3Ch] BYREF
    int inHeight; // [esp+B4h] [ebp-34h]
    IDirect3DTexture9 *destTexture; // [esp+B8h] [ebp-30h]
    int outWidth; // [esp+BCh] [ebp-2Ch]
    const GfxCmdPCCopyImageGenMIP *cmd; // [esp+C0h] [ebp-28h]
    unsigned int numMipLevels; // [esp+C4h] [ebp-24h]
    _D3DSURFACE_DESC mipDesc; // [esp+C8h] [ebp-20h] BYREF

    cmd = (const GfxCmdPCCopyImageGenMIP *)execState->cmd;
    renderSurface = gfxRenderTargets[R_RENDERTARGET_COMPOSITE].surface.color;
    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("dx.device->CreateTexture(256, 256, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &resolveTexture, 0)\n");
    hr = dx.device->CreateTexture(256u, 256u, 1u, 0, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &resolveTexture, 0);
    if ( hr < 0 )
    {
        ++g_disableRendering;
        v1 = R_ErrorDescription(hr);
        Com_Error(
            ERR_FATAL,
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp (%i) dx.device->CreateTexture(256, 256, 1, 0, D3DFMT_A8"
            "R8G8B8, D3DPOOL_SYSTEMMEM, &resolveTexture, 0) failed: %s\n",
            3127,
            v1);
    }
    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("resolveTexture->GetSurfaceLevel(0, &resolveSurface)\n");
    v45 = resolveTexture->GetSurfaceLevel(0, &resolveSurface);
    if ( v45 < 0 )
    {
        ++g_disableRendering;
        v2 = R_ErrorDescription(v45);
        Com_Error(
            ERR_FATAL,
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp (%i) resolveTexture->GetSurfaceLevel(0, &resolveSurface) failed: %s\n",
            3128,
            v2);
    }
    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("dx.device->GetRenderTargetData(renderSurface, resolveSurface)\n");
    v43 = dx.device->GetRenderTargetData(renderSurface, resolveSurface);
    if ( v43 < 0 )
    {
        ++g_disableRendering;
        v3 = R_ErrorDescription(v43);
        Com_Error(
            ERR_FATAL,
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp (%i) dx.device->GetRenderTargetData(renderSurface, reso"
            "lveSurface) failed: %s\n",
            3129,
            v3);
    }
    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("resolveSurface->LockRect(&srcRect, 0, 0x00000010L)\n");
    v41 = resolveSurface->LockRect(&srcRect, 0, 16u);
    if ( v41 < 0 )
    {
        ++g_disableRendering;
        v4 = R_ErrorDescription(v41);
        Com_Error(
            ERR_FATAL,
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp (%i) resolveSurface->LockRect(&srcRect, 0, 0x00000010L) failed: %s\n",
            3131,
            v4);
    }
    destTexture = cmd->image->texture.map;
    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("numMipLevels = destTexture->GetLevelCount()\n");
    numMipLevels = destTexture->GetLevelCount();
    v39 = numMipLevels;
    if ( v39 < 0 )
    {
        ++g_disableRendering;
        v5 = R_ErrorDescription(v39);
        Com_Error(
            ERR_FATAL,
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp (%i) numMipLevels = destTexture->GetLevelCount() failed: %s\n",
            3134,
            v5);
    }
    inHeight = 256;
    for ( destMipLevel = 0; destMipLevel < numMipLevels; ++destMipLevel )
    {
        if ( r_logFile && r_logFile->current.integer )
            RB_LogPrint("destTexture->LockRect(destMipLevel, &mipRect, 0, 0x00002000L)\n");
        v32 = destTexture->LockRect(destMipLevel, &mipRect, 0, 0x2000u);
        if ( v32 < 0 )
        {
            ++g_disableRendering;
            v6 = R_ErrorDescription(v32);
            Com_Error(
                ERR_FATAL,
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp (%i) destTexture->LockRect(destMipLevel, &mipRect, 0,"
                " 0x00002000L) failed: %s\n",
                3141,
                v6);
        }
        if ( r_logFile && r_logFile->current.integer )
            RB_LogPrint("destTexture->GetLevelDesc(destMipLevel, &mipDesc)\n");
        //v30 = ((int (__thiscall *)(IDirect3DTexture9 *, IDirect3DTexture9 *, unsigned int, _D3DSURFACE_DESC *))destTexture->GetLevelDesc)(
        //                destTexture,
        //                destTexture,
        //                destMipLevel,
        //                &mipDesc);
        v30 = destTexture->GetLevelDesc(destMipLevel, &mipDesc);
        if ( v30 < 0 )
        {
            ++g_disableRendering;
            v7 = R_ErrorDescription(v30);
            Com_Error(
                ERR_FATAL,
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp (%i) destTexture->GetLevelDesc(destMipLevel, &mipDesc) failed: %s\n",
                3142,
                v7);
        }
        outWidth = mipDesc.Width;
        outHeight = mipDesc.Height;
        step = 256 / (signed int)mipDesc.Width;
        out = (unsigned __int8 *)mipRect.pBits;
        linear = (unsigned __int8 *)srcRect.pBits;
        for ( y = 0; y < outHeight; ++y )
        {
            in = &linear[step * (y << 10)];
            for ( x = 0; x < outWidth; ++x )
            {
                r = 0;
                g = 0;
                b = 0;
                a = 0;
                for ( yy = 0; yy < step; ++yy )
                {
                    for ( xx = 0; xx < 4 * step; xx += 4 )
                    {
                        a += in[1024 * yy + xx];
                        r += in[1024 * yy + 1 + xx];
                        g += in[1024 * yy + 2 + xx];
                        b += in[1024 * yy + 3 + xx];
                    }
                }
                in += 4 * step;
                *out = a / (step * step);
                outa = out + 1;
                *outa++ = r / (step * step);
                *outa++ = g / (step * step);
                *outa = b / (step * step);
                out = outa + 1;
            }
        }
        if ( r_logFile && r_logFile->current.integer )
            RB_LogPrint("destTexture->UnlockRect(destMipLevel)\n");
        v19 = destTexture->UnlockRect(destMipLevel);
        if ( v19 < 0 )
        {
            ++g_disableRendering;
            v8 = R_ErrorDescription(v19);
            Com_Error(
                ERR_FATAL,
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp (%i) destTexture->UnlockRect(destMipLevel) failed: %s\n",
                3177,
                v8);
        }
    }
    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("resolveSurface->UnlockRect()\n");
    v17 = resolveSurface->UnlockRect();
    if ( v17 < 0 )
    {
        ++g_disableRendering;
        v9 = R_ErrorDescription(v17);
        Com_Error(
            ERR_FATAL,
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp (%i) resolveSurface->UnlockRect() failed: %s\n",
            3180,
            v9);
    }
    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("resolveSurface->Release()\n");
    //v15 = ((int (__thiscall *)(IDirect3DSurface9 *, IDirect3DSurface9 *))resolveSurface->Release)(
    //                resolveSurface,
    //                resolveSurface);
    v15 = resolveSurface->Release();
    if ( v15 < 0 )
    {
        ++g_disableRendering;
        v10 = R_ErrorDescription(v15);
        Com_Error(
            ERR_FATAL,
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp (%i) resolveSurface->Release() failed: %s\n",
            3181,
            v10);
    }
    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("resolveTexture->Release()\n");
    v13 = resolveTexture->Release();
    if ( v13 < 0 )
    {
        ++g_disableRendering;
        v11 = R_ErrorDescription(v13);
        Com_Error(
            ERR_FATAL,
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp (%i) resolveTexture->Release() failed: %s\n",
            3182,
            v11);
    }
    cmd->callback(cmd->job);
    execState->cmd = (char *)execState->cmd + *(unsigned __int16 *)execState->cmd;
}

void __cdecl GetShiftColor(
                GfxColor targetColor,
                int fxBirthTime,
                int fxDecayStart,
                int fxDecayDuration,
                GfxColor startColor,
                GfxColor *shiftColor)
{
    float frac; // [esp+0h] [ebp-Ch]
    int timeElapsed; // [esp+4h] [ebp-8h]

    timeElapsed = gfxCmdBufSourceState.sceneDef.time - fxBirthTime;

    iassert(timeElapsed >= 0);
    
    if ( timeElapsed <= fxDecayDuration + fxDecayStart )
    {
        if ( timeElapsed <= fxDecayStart )
        {
            shiftColor->packed = startColor.packed;
        }
        else
        {
            frac = (float)(timeElapsed - fxDecayStart) / (float)fxDecayDuration;
            shiftColor->array[2] = (int)(float)((float)startColor.array[2]
                                                                                + (float)((float)(targetColor.array[2] - startColor.array[2]) * frac));
            shiftColor->array[1] = (int)(float)((float)startColor.array[1]
                                                                                + (float)((float)(targetColor.array[1] - startColor.array[1]) * frac));
            shiftColor->array[0] = (int)(float)((float)startColor.array[0]
                                                                                + (float)((float)(targetColor.array[0] - startColor.array[0]) * frac));
            shiftColor->array[3] = startColor.array[3];
        }
    }
    else
    {
        shiftColor->packed = targetColor.packed;
    }
}

void __cdecl RB_DrawText(const char *text, Font_s *font, float x, float y, GfxColor color)
{
    DrawText2D(text, x, y, 1.0, font, 1.0, 1.0, 0.0, 1.0, color, 0x7FFFFFFF, 0, 0, 0, 0.0, color, 0, 0, 0, 0, 0, 0, 0, 0);
}

int lastFlashTime;
int flashTime;
bool bFlashToggle = true;
void __cdecl DrawText2D(
                const char *text,
                float x,
                float y,
                float w,
                Font_s *font,
                float xScale,
                float yScale,
                float sinAngle,
                float cosAngle,
                GfxColor color,
                int maxLength,
                int renderFlags,
                int cursorPos,
                char cursorLetter,
                float padding,
                GfxColor glowForcedColor,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration,
                int fxRedactDecayStartTime,
                int fxRedactDecayDuration,
                const Material *fxMaterial,
                const Material *fxMaterialGlow)
{
    int v24; // esi
    unsigned int v25; // eax
    const char *v26; // eax
    char v27; // [esp+38h] [ebp-29Ch]
    unsigned __int8 v28; // [esp+3Ch] [ebp-298h]
    char v29; // [esp+40h] [ebp-294h]
    int v30; // [esp+D0h] [ebp-204h]
    unsigned int v31; // [esp+D4h] [ebp-200h]
    int v32; // [esp+D8h] [ebp-1FCh]
    int v33; // [esp+F4h] [ebp-1E0h]
    int v34; // [esp+100h] [ebp-1D4h]
    float *v35; // [esp+10Ch] [ebp-1C8h]
    float *v36; // [esp+110h] [ebp-1C4h]
    int j; // [esp+120h] [ebp-1B4h]
    int i; // [esp+124h] [ebp-1B0h]
    unsigned int fadeColor; // [esp+128h] [ebp-1ACh]
    Material *mat; // [esp+12Ch] [ebp-1A8h] BYREF
    float u; // [esp+130h] [ebp-1A4h] BYREF
    float out_rect[4]; // [esp+134h] [ebp-1A0h] BYREF
    float resizeOffsY; // [esp+144h] [ebp-190h]
    int offIdx; // [esp+148h] [ebp-18Ch]
    float resizeOffsX; // [esp+14Ch] [ebp-188h]
    float (*rect)[4]; // [esp+150h] [ebp-184h]
    float ofs; // [esp+154h] [ebp-180h]
    GfxColor shiftColor; // [esp+158h] [ebp-17Ch] BYREF
    GfxColor targetColor; // [esp+15Ch] [ebp-178h] BYREF
    const Glyph *glyphOriginal; // [esp+160h] [ebp-174h]
    int tempSeed; // [esp+164h] [ebp-170h] BYREF
    float iconWidth; // [esp+168h] [ebp-16Ch]
    GfxColor lookupColor; // [esp+16Ch] [ebp-168h] BYREF
    const unsigned __int8 *altColorTwo; // [esp+170h] [ebp-164h]
    bool drawExtraFxChar; // [esp+177h] [ebp-15Dh]
    const Glyph *glyph; // [esp+178h] [ebp-15Ch]
    float yAdj; // [esp+17Ch] [ebp-158h]
    float decayOffset; // [esp+180h] [ebp-154h]
    float xAdj; // [esp+184h] [ebp-150h]
    bool skipDrawing; // [esp+18Bh] [ebp-149h]
    unsigned int letter; // [esp+18Ch] [ebp-148h]
    int extraFxChar; // [esp+190h] [ebp-144h]
    float deltaX; // [esp+194h] [ebp-140h]
    unsigned int origLetter; // [esp+198h] [ebp-13Ch]
    float yRot; // [esp+19Ch] [ebp-138h] BYREF
    int passRandSeed; // [esp+1A0h] [ebp-134h] BYREF
    int maxLengthRemaining; // [esp+1A4h] [ebp-130h]
    float xRot; // [esp+1A8h] [ebp-12Ch] BYREF
    bool subtitleAllowGlow; // [esp+1AFh] [ebp-125h]
    GfxColor currentColor; // [esp+1B0h] [ebp-124h]
    const char *curText; // [esp+1B4h] [ebp-120h] BYREF
    int count; // [esp+1B8h] [ebp-11Ch]
    int xdelta; // [esp+1BCh] [ebp-118h]
    int ydelta; // [esp+1C0h] [ebp-114h]
    int jitterSeed; // [esp+1C4h] [ebp-110h] BYREF
    bool drawLargeCharAtEnd; // [esp+1CBh] [ebp-109h]
    GfxColor finalColor; // [esp+1CCh] [ebp-108h] BYREF
    int redactLeftCount; // [esp+1D0h] [ebp-104h]
    int passIdx; // [esp+1D4h] [ebp-100h]
    float maxY; // [esp+1D8h] [ebp-FCh]
    bool bFlash; // [esp+1DFh] [ebp-F5h]
    int MAX_FLASH_TIME; // [esp+1E0h] [ebp-F4h]
    GfxColor dropShadowColor; // [esp+1E4h] [ebp-F0h]
    const Material *material; // [esp+1E8h] [ebp-ECh]
    bool drawRandomCharAtEnd; // [esp+1EFh] [ebp-E5h] BYREF
    int randSeed; // [esp+1F0h] [ebp-E4h] BYREF
    bool shiftColorOn; // [esp+1F7h] [ebp-DDh]
    float startX; // [esp+1F8h] [ebp-DCh]
    int redactCount; // [esp+1FCh] [ebp-D8h]
    int randomCharsLength; // [esp+200h] [ebp-D4h] BYREF
    bool drawUnderscoreCharAtEnd; // [esp+207h] [ebp-CDh]
    int decayTimeElapsed; // [esp+208h] [ebp-CCh] BYREF
    char shiftColorTarget; // [esp+20Fh] [ebp-C5h]
    const Material *glowMaterial; // [esp+210h] [ebp-C4h]
    bool continueDrawing; // [esp+216h] [ebp-BEh]
    bool redactOn; // [esp+217h] [ebp-BDh]
    int redactAlpha; // [esp+218h] [ebp-BCh]
    float monospaceWidth; // [esp+21Ch] [ebp-B8h]
    float total_rect[4]; // [esp+220h] [ebp-B4h] BYREF
    float sizeIncrease; // [esp+230h] [ebp-A4h] BYREF
    bool decaying; // [esp+237h] [ebp-9Dh] BYREF
    float startY; // [esp+238h] [ebp-9Ch]
    float redactLeftRect[17]; // [esp+23Ch] [ebp-98h] BYREF
    bool redactLeftOn; // [esp+283h] [ebp-51h]
    float redactRect[4][4]; // [esp+284h] [ebp-50h] BYREF
    float minY; // [esp+2C4h] [ebp-10h]
    int passCount; // [esp+2C8h] [ebp-Ch]
    unsigned __int8 fadeAlpha; // [esp+2CFh] [ebp-5h] BYREF
    int MAX_REDACT_RECT; // [esp+2D0h] [ebp-4h]
    float xa; // [esp+2E0h] [ebp+Ch]
    float ya; // [esp+2E4h] [ebp+10h]

    sizeIncrease = 1.0f;
    redactLeftOn = 0;
    redactOn = 0;
    redactLeftCount = 0;
    redactCount = 0;
    MAX_REDACT_RECT = 4;
    minY = FLT_MAX;
    maxY = -FLT_MAX;
    shiftColorOn = 0;
    shiftColorTarget = 0;
    redactAlpha = 255;
    bFlash = 0;
    MAX_FLASH_TIME = 175;
    if ( !text && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp", 3993, 0, "%s", "text") )
        __debugbreak();
    if ( !font && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp", 3994, 0, "%s", "font") )
        __debugbreak();
    dropShadowColor.packed = 0;
    dropShadowColor.array[3] = color.array[3];
    randSeed = 1;
    randomCharsLength = 0;
    drawRandomCharAtEnd = 0;
    drawUnderscoreCharAtEnd = 0;
    drawLargeCharAtEnd = 0;
    monospaceWidth = GetMonospaceWidth(font, renderFlags);
    glowMaterial = 0;
    material = Material_FromHandle(font->material);
    if ( !material
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp", 4007, 0, "%s", "material") )
    {
        __debugbreak();
    }
    if ( (renderFlags & 0x40) != 0
        && (!fxMaterial || !fxMaterial->localTechniqueSet)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp",
                    4009,
                    0,
                    "%s",
                    "!(renderFlags & TEXT_RENDERFLAG_FX_DECODE) || (fxMaterial && fxMaterial->techniqueSet)") )
    {
        __debugbreak();
    }
    if ( (renderFlags & 0x40) != 0
        && (!fxMaterialGlow || !fxMaterialGlow->localTechniqueSet)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp",
                    4010,
                    0,
                    "%s",
                    "!(renderFlags & TEXT_RENDERFLAG_FX_DECODE) || (fxMaterialGlow && fxMaterialGlow->techniqueSet)") )
    {
        __debugbreak();
    }
    if ( !lastFlashTime )
        lastFlashTime = Sys_Milliseconds();
    flashTime = Sys_Milliseconds() - lastFlashTime;
    if ( flashTime > 175 )
    {
        lastFlashTime = Sys_Milliseconds();
        bFlashToggle = !bFlashToggle;
    }
    if ( (renderFlags & 0x800) != 0 )
    {
        continueDrawing = SetupFadeinFXVars(
                                                text,
                                                maxLength,
                                                renderFlags,
                                                fxBirthTime,
                                                fxLetterTime,
                                                fxDecayStartTime,
                                                fxDecayDuration,
                                                &drawRandomCharAtEnd,
                                                &randSeed,
                                                &decaying,
                                                &decayTimeElapsed);
    }
    else if ( (renderFlags & 0x400) != 0 )
    {
        continueDrawing = SetupTypewriterFXVars(
                                                text,
                                                maxLength,
                                                renderFlags,
                                                fxBirthTime,
                                                fxLetterTime,
                                                fxDecayStartTime,
                                                fxDecayDuration,
                                                &drawRandomCharAtEnd,
                                                &randSeed,
                                                &maxLength,
                                                &decaying,
                                                &decayTimeElapsed);
    }
    else if ( (renderFlags & 0x1000) != 0 )
    {
        drawUnderscoreCharAtEnd = 1;
        continueDrawing = SetupTypewriterFXVars(
                                                text,
                                                maxLength,
                                                renderFlags,
                                                fxBirthTime,
                                                fxLetterTime,
                                                fxDecayStartTime,
                                                fxDecayDuration,
                                                &drawRandomCharAtEnd,
                                                &randSeed,
                                                &maxLength,
                                                &decaying,
                                                &decayTimeElapsed);
    }
    else if ( (renderFlags & 0x2000) != 0 )
    {
        drawLargeCharAtEnd = 1;
        continueDrawing = SetupPopInFXVars(
                                                text,
                                                maxLength,
                                                renderFlags,
                                                fxBirthTime,
                                                fxLetterTime,
                                                fxDecayStartTime,
                                                fxDecayDuration,
                                                &sizeIncrease,
                                                &randSeed,
                                                &maxLength,
                                                &decaying,
                                                &decayTimeElapsed);
    }
    else if ( (renderFlags & 0x4000) != 0 )
    {
        continueDrawing = SetupRedactTextFXVars(
                                                text,
                                                maxLength,
                                                renderFlags,
                                                fxBirthTime,
                                                fxLetterTime,
                                                fxDecayStartTime,
                                                fxDecayDuration,
                                                &drawRandomCharAtEnd,
                                                &randSeed,
                                                &maxLength,
                                                &decaying,
                                                &decayTimeElapsed);
    }
    else if ( (renderFlags & 0x10000) != 0 )
    {
        continueDrawing = SetupCOD7DecodeFXVars(
                                                text,
                                                maxLength,
                                                fxBirthTime,
                                                fxLetterTime,
                                                fxDecayStartTime,
                                                fxDecayDuration,
                                                &randSeed,
                                                &randomCharsLength,
                                                &maxLength,
                                                &decaying,
                                                &decayTimeElapsed);
    }
    else
    {
        continueDrawing = SetupPulseFXVars(
                                                text,
                                                maxLength,
                                                renderFlags,
                                                fxBirthTime,
                                                fxLetterTime,
                                                fxDecayStartTime,
                                                fxDecayDuration,
                                                &drawRandomCharAtEnd,
                                                &randSeed,
                                                &maxLength,
                                                &decaying,
                                                &decayTimeElapsed);
    }
    if ( continueDrawing )
    {
        passCount = 1;
        if ( (renderFlags & 0x10) != 0 )
        {
            glowMaterial = Material_FromHandle(font->glowMaterial);
            if ( !glowMaterial
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp",
                            4066,
                            0,
                            "%s",
                            "glowMaterial") )
            {
                __debugbreak();
            }
            ++passCount;
        }
        if ( (renderFlags & 0x40) != 0 )
        {
            if ( (renderFlags & 0x10) != 0
                && !fxMaterialGlow
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp",
                            4073,
                            0,
                            "%s",
                            "fxMaterialGlow") )
            {
                __debugbreak();
            }
            if ( !fxMaterial
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp", 4074, 0, "%s", "fxMaterial") )
            {
                __debugbreak();
            }
        }
        if ((renderFlags & 0x8000) != 0 && gJitterData[(gfxCmdBufSourceState.sceneDef.time >> 5) % 0xB0u])
        {
            jitterSeed = gfxCmdBufSourceState.sceneDef.time >> 5;
            if (gJitterData[(gfxCmdBufSourceState.sceneDef.time >> 5) % 0xB0u] == 1)
            {
                x = (float)((int)RandWithSeed(&jitterSeed) % 7 - 3) + x;
                y = (float)((int)RandWithSeed(&jitterSeed) % 11 - 5) + y;
            }
            if (gJitterData[(gfxCmdBufSourceState.sceneDef.time >> 5) % 0xB0u] == 2)
            {
                xdelta = (int)RandWithSeed(&jitterSeed) % 10 + 45;
                ydelta = (int)RandWithSeed(&jitterSeed) % 10 + 45;
                if ((int)RandWithSeed(&jitterSeed) % 2)
                    ydelta = -ydelta;
                x = (float)xdelta + x;
                y = (float)ydelta + y;
            }
        }
        startX = x - (float)(0.5 * xScale);
        startY = y - (float)(0.5 * yScale);
        total_rect[0] = FLT_MAX;
        total_rect[1] = FLT_MAX;
        total_rect[2] = -FLT_MAX;
        total_rect[3] = -FLT_MAX;
        for ( passIdx = 0; passIdx < passCount; ++passIdx )
        {
            maxLengthRemaining = maxLength;
            passRandSeed = randSeed;
            currentColor.packed = color.packed;
            xa = startX;
            ya = startY;
            subtitleAllowGlow = 0;
            count = 0;
            curText = text;
            while ( *curText && maxLengthRemaining )
            {
                drawExtraFxChar = 0;
                letter = SEH_ReadCharFromString(&curText, 0);
                skipDrawing = 0;
                fadeAlpha = 0;
                drawExtraFxChar = 0;
                extraFxChar = 0;
                if ( letter == 94 && curText && *curText != 94 && *curText >= 48 && *curText <= 64 )
                {
                    subtitleAllowGlow = 0;
                    v24 = ColorIndex(*curText);
                    if ( v24 == ColorIndex(0x37u) )
                    {
                        currentColor.packed = color.packed;
                    }
                    else if ( (renderFlags & 0x100) != 0 && ColorIndex(*curText) == 2 )
                    {
                        altColorTwo = MY_ALTCOLOR_TWO;
                        currentColor.array[3] = ModulateByteColors(MY_ALTCOLOR_TWO[3], color.array[3]);
                        currentColor.array[0] = altColorTwo[2];
                        currentColor.array[1] = altColorTwo[1];
                        currentColor.array[2] = *altColorTwo;
                        subtitleAllowGlow = 1;
                    }
                    else
                    {
                        RB_LookupColor(*curText, (unsigned __int8 *)&lookupColor);
                        currentColor.array[3] = color.array[3];
                        currentColor.array[0] = lookupColor.array[2];
                        currentColor.array[1] = lookupColor.array[1];
                        currentColor.array[2] = lookupColor.array[0];
                    }
                    ++curText;
                    count += 2;
                }
                else if ( letter == 94 && curText && *curText == 70 && !r_glob.isMultiplayer )
                {
                    bFlash = !bFlash;
                    ++curText;
                    count += 2;
                }
                else if ( (renderFlags & 0x4000) != 0 && letter == 94 )
                {
                    v29 = *curText;
                    if ( *curText == 67 )
                    {
                        if ( shiftColorOn )
                        {
                            shiftColorOn = 0;
                            ++curText;
                            ++count;
                        }
                        else
                        {
                            shiftColorOn = 1;
                            shiftColorTarget = *++curText;
                            ++curText;
                            count += 3;
                        }
                    }
                    else if ( v29 == 76 )
                    {
                        if ( redactLeftOn )
                        {
                            redactLeftOn = 0;
                            ++redactLeftCount;
                            ++curText;
                            count += 2;
                        }
                        else if ( redactLeftCount < 4 )
                        {
                            v36 = &redactLeftRect[4 * redactLeftCount];
                            *v36 = FLT_MAX;
                            v36[1] = FLT_MAX;
                            v36[2] = -FLT_MAX;
                            v36[3] = -FLT_MAX;
                            redactLeftOn = 1;
                            ++curText;
                            count += 2;
                        }
                    }
                    else if ( v29 == 85 )
                    {
                        if ( redactOn )
                        {
                            redactOn = 0;
                            ++redactCount;
                            ++curText;
                            count += 2;
                        }
                        else if ( redactCount < 4 )
                        {
                            v35 = redactRect[redactCount];
                            *v35 = FLT_MAX;
                            v35[1] = FLT_MAX;
                            v35[2] = -FLT_MAX;
                            v35[3] = -FLT_MAX;
                            redactOn = 1;
                            ++curText;
                            count += 2;
                        }
                    }
                }
                else
                {
                    if ( maxLengthRemaining > randomCharsLength || letter == 32 )
                    {
                        if ( randomCharsLength <= 0 )
                        {
                            if ( drawRandomCharAtEnd && maxLengthRemaining == 1 )
                            {
                                letter = R_FontGetRandomLetter(font, passRandSeed);
                                fadeAlpha = -64;
                                if ( (int)RandWithSeed(&passRandSeed) % 2 )
                                {
                                    drawExtraFxChar = 1;
                                    letter = 79;
                                }
                            }
                        }
                        else
                        {
                            fadeAlpha = -1;
                        }
                    }
                    else
                    {
                        v25 = RandWithSeed(&passRandSeed);
                        letter = R_FontGetRandomNumberCharacter(font, v25);
                        fadeAlpha = -64;
                    }
                    if ( drawUnderscoreCharAtEnd && maxLengthRemaining == 1 )
                        letter = 95;
                    if ( letter == 94 && (*curText == 1 || *curText == 2) )
                    {
                        RotateXY(cosAngle, sinAngle, startX, startY, xa, ya, &xRot, &yRot);
                        if ( !bFlash || bFlashToggle )
                            iconWidth = RB_DrawHudIcon(
                                                        curText,
                                                        xRot,
                                                        yRot,
                                                        sinAngle,
                                                        cosAngle,
                                                        font,
                                                        xScale,
                                                        yScale,
                                                        currentColor.packed);
                        else
                            iconWidth = (float)((font->pixelHeight * (curText[1] - 16) + 16) / 32) * xScale;
                        if ( iconWidth <= 0.0 )
                        {
                            v26 = va("Invalid hud icon.    Text: \"%s\"", text);
                            if ( !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp",
                                            4294,
                                            0,
                                            "%s\n\t%s",
                                            "iconWidth > 0",
                                            v26) )
                                __debugbreak();
                        }
                        xa = xa + iconWidth;
                        if ( (renderFlags & 0x80) != 0 )
                            xa = (float)(padding * xScale) + xa;
                        curText += 7;
                        ++count;
                        --maxLengthRemaining;
                    }
                    else if ( letter == 10 )
                    {
                        xa = startX;
                        ya = (float)((float)font->pixelHeight * yScale) + ya;
                    }
                    else if ( letter == 13 )
                    {
                        xa = startX;
                    }
                    else
                    {
                        origLetter = letter;
                        if ( decaying )
                        {
                            if ( (renderFlags & 0x1000) != 0 )
                            {
                                v30 = decayTimeElapsed;
                                v31 = letter;
                                v34 = (int)(float)(30.0 * (float)((float)fxDecayDuration / 1000.0));
                                v33 = fxDecayDuration / v34;
                                skipDrawing = v30 >= v33 * ((int)RandWithSeed(&passRandSeed) % v34);
                                letter = v31;
                                if ( (int)((float)(255.0 * 1.0) + 9.313225746154785e-10) < 255 )
                                    v32 = (int)((float)(255.0 * 1.0) + 9.313225746154785e-10);
                                else
                                    v32 = 255;
                                if ( v32 > 0 )
                                    v28 = v32;
                                else
                                    v28 = 0;
                                fadeAlpha = v28;
                                drawExtraFxChar = 0;
                            }
                            else
                            {
                                GetDecayingStringAlphaInfo(decayTimeElapsed, fxDecayDuration, currentColor.array[3], &fadeAlpha);
                            }
                        }
                        if ( drawExtraFxChar )
                        {
                            tempSeed = passRandSeed;
                            extraFxChar = RandWithSeed(&tempSeed);
                        }
                        glyph = R_GetCharacterGlyph(font, letter);
                        if ( letter == origLetter )
                        {
                            decayOffset = 0.0f;
                            deltaX = (float)glyph->dx;
                        }
                        else
                        {
                            glyphOriginal = R_GetCharacterGlyph(font, origLetter);
                            decayOffset = (float)((float)glyphOriginal->pixelWidth * 0.5) - (float)((float)glyph->pixelWidth * 0.5);
                            deltaX = (float)glyphOriginal->dx;
                        }
                        xAdj = (float)((float)glyph->x0 + decayOffset) * xScale;
                        yAdj = (float)glyph->y0 * yScale;
                        if ( drawLargeCharAtEnd && maxLengthRemaining == 1 )
                        {
                            yScale = yScale * sizeIncrease;
                            xScale = xScale * sizeIncrease;
                            yAdj = (float)((float)(sizeIncrease + 1.0) * yAdj) / 2.0;
                        }
                        finalColor.packed = RB_ProcessLetterColor(currentColor.packed, letter).packed;
                        if ( decaying || drawRandomCharAtEnd && maxLengthRemaining == 1 || randomCharsLength > 0 )
                        {
                            finalColor.array[3] = ModulateByteColors(finalColor.array[3], fadeAlpha);
                            dropShadowColor.array[3] = fadeAlpha;
                        }
                        if ( shiftColorOn )
                        {
                            RB_LookupColor(shiftColorTarget, (unsigned __int8 *)&targetColor);
                            targetColor.packed = (finalColor.array[3] << 24)
                                                                 | targetColor.array[2]
                                                                 | (targetColor.array[1] << 8)
                                                                 | (targetColor.array[0] << 16);
                            GetShiftColor(
                                targetColor,
                                fxBirthTime,
                                fxRedactDecayDuration + fxRedactDecayStartTime,
                                fxRedactDecayDuration,
                                finalColor,
                                &shiftColor);
                            finalColor.packed = shiftColor.packed;
                        }
                        if ( !skipDrawing )
                        {
                            if ( passIdx )
                            {
                                if ( passIdx == 1 && ((renderFlags & 0x100) == 0 || subtitleAllowGlow) )
                                {
                                    GlowColor(&finalColor, finalColor, glowForcedColor, renderFlags);
                                    //resizeOffsX = (float)((float)((float)glyph->pixelWidth * COERCE_FLOAT(LODWORD(0.75f) ^ _mask__NegFloat_)) * 0.5) * xScale;
                                    resizeOffsX = (float)((float)((float)glyph->pixelWidth * (-(0.75f))) * 0.5) * xScale;
                                    //resizeOffsY = (float)((float)((float)glyph->pixelHeight * COERCE_FLOAT(LODWORD(0.1f25) ^ _mask__NegFloat_)) * 0.5) * yScale;
                                    resizeOffsY = (float)((float)((float)glyph->pixelHeight * (-(0.125f))) * 0.5) * yScale;
                                    for ( offIdx = 0; offIdx < 4; ++offIdx )
                                    {
                                        xRot = (float)((float)(xa + xAdj) + resizeOffsX) + (float)((float)((float)MY_OFFSETS[offIdx][0] * 2.0) * xScale);
                                        yRot = (float)((float)(ya + yAdj) + resizeOffsY) + (float)((float)((float)MY_OFFSETS[offIdx][1] * 2.0) * yScale);
                                        RotateXY(cosAngle, sinAngle, startX, startY, xRot, yRot, &xRot, &yRot);
                                        if ( !glowMaterial
                                            && !Assert_MyHandler(
                                                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp",
                                                        4485,
                                                        0,
                                                        "%s",
                                                        "glowMaterial") )
                                        {
                                            __debugbreak();
                                        }
                                        if ( !bFlash || bFlashToggle )
                                        {
                                            if ( drawExtraFxChar )
                                                DrawTextFxExtraCharacter(
                                                    fxMaterialGlow,
                                                    extraFxChar,
                                                    xRot,
                                                    yRot,
                                                    (float)glyph->pixelWidth * xScale,
                                                    (float)glyph->pixelHeight * yScale,
                                                    sinAngle,
                                                    cosAngle,
                                                    finalColor.packed);
                                            else
                                                RB_DrawStretchPicRotate(
                                                    glowMaterial,
                                                    xRot,
                                                    yRot,
                                                    w,
                                                    (float)((float)glyph->pixelWidth * xScale) * (float)(1.0 + 0.75),
                                                    (float)((float)glyph->pixelHeight * yScale) * (float)(1.0 + 0.125),
                                                    glyph->s0,
                                                    glyph->t0,
                                                    glyph->s1,
                                                    glyph->t1,
                                                    sinAngle,
                                                    cosAngle,
                                                    finalColor.packed,
                                                    GFX_PRIM_STATS_HUD);
                                        }
                                    }
                                }
                            }
                            else
                            {
                                if ( (renderFlags & 4) != 0 )
                                {
                                    ofs = 1.0f;
                                    if ( (renderFlags & 8) != 0 )
                                        ofs = ofs + 1.0;
                                    xRot = (float)(xa + xAdj) + (float)(ofs * xScale);
                                    yRot = (float)(ya + yAdj) + (float)(ofs * yScale);
                                    RotateXY(cosAngle, sinAngle, startX, startY, xRot, yRot, &xRot, &yRot);
                                    if ( !bFlash || bFlashToggle )
                                    {
                                        if ( drawExtraFxChar )
                                            DrawTextFxExtraCharacter(
                                                fxMaterial,
                                                extraFxChar,
                                                xRot,
                                                yRot,
                                                (float)glyph->pixelWidth * xScale,
                                                (float)glyph->pixelHeight * yScale,
                                                sinAngle,
                                                cosAngle,
                                                dropShadowColor.packed);
                                        else
                                            RB_DrawStretchPicRotate(
                                                material,
                                                xRot,
                                                yRot,
                                                w,
                                                (float)glyph->pixelWidth * xScale,
                                                (float)glyph->pixelHeight * yScale,
                                                glyph->s0,
                                                glyph->t0,
                                                glyph->s1,
                                                glyph->t1,
                                                sinAngle,
                                                cosAngle,
                                                dropShadowColor.packed,
                                                GFX_PRIM_STATS_HUD);
                                    }
                                }
                                xRot = xa + xAdj;
                                yRot = ya + yAdj;
                                RotateXY(cosAngle, sinAngle, startX, startY, xa + xAdj, ya + yAdj, &xRot, &yRot);
                                if ( !bFlash || bFlashToggle )
                                {
                                    if ( drawExtraFxChar )
                                        DrawTextFxExtraCharacter(
                                            fxMaterial,
                                            extraFxChar,
                                            xRot,
                                            yRot,
                                            (float)glyph->pixelWidth * xScale,
                                            (float)glyph->pixelHeight * yScale,
                                            sinAngle,
                                            cosAngle,
                                            finalColor.packed);
                                    else
                                        RB_DrawStretchPicRotate(
                                            material,
                                            xRot,
                                            yRot,
                                            w,
                                            (float)glyph->pixelWidth * xScale,
                                            (float)glyph->pixelHeight * yScale,
                                            glyph->s0,
                                            glyph->t0,
                                            glyph->s1,
                                            glyph->t1,
                                            sinAngle,
                                            cosAngle,
                                            finalColor.packed,
                                            GFX_PRIM_STATS_HUD);
                                }
                                if ( redactLeftOn || redactOn )
                                {
                                    redactAlpha = finalColor.array[3];
                                    if ( redactLeftOn )
                                        rect = (float (*)[4])&redactLeftRect[4 * redactLeftCount];
                                    else
                                        rect = (float (*)[4])redactRect[redactCount];
                                    if ( (*rect)[0] > xRot )
                                        (*rect)[0] = xRot;
                                    if ( (float)((float)((float)glyph->dx * xScale) + xa) > (*rect)[2] )
                                        (*rect)[2] = (float)((float)glyph->dx * xScale) + xa;
                                    if ( glyph->letter != 32 )
                                    {
                                        if ( minY > (float)(ya - (float)glyph->pixelHeight) )
                                            minY = ya - (float)glyph->pixelHeight;
                                        if ( ya > maxY )
                                            maxY = ya;
                                    }
                                }
                                if ( total_rect[0] > xRot )
                                    total_rect[0] = xRot;
                                if ( (float)((float)((float)glyph->dx * xScale) + xa) > total_rect[2] )
                                    total_rect[2] = (float)((float)glyph->dx * xScale) + xa;
                                if ( (renderFlags & 2) != 0 && count == cursorPos )
                                {
                                    xRot = xa + xAdj;
                                    RotateXY(cosAngle, sinAngle, startX, startY, xa + xAdj, ya, &xRot, &yRot);
                                    RB_DrawCursor(
                                        material,
                                        cursorLetter,
                                        xRot,
                                        yRot,
                                        sinAngle,
                                        cosAngle,
                                        font,
                                        xScale,
                                        yScale,
                                        finalColor.packed);
                                }
                            }
                        }
                        if ( (renderFlags & 1) != 0 )
                            xa = (float)(monospaceWidth * xScale) + xa;
                        else
                            xa = (float)(deltaX * xScale) + xa;
                        if ( (renderFlags & 0x80) != 0 )
                            xa = (float)(padding * xScale) + xa;
                        ++count;
                        --maxLengthRemaining;
                    }
                }
            }
            if ( (renderFlags & 2) != 0 && count == cursorPos )
            {
                xRot = xa;
                RotateXY(cosAngle, sinAngle, startX, startY, xa, ya, &xRot, &yRot);
                RB_DrawCursor(material, cursorLetter, xRot, yRot, sinAngle, cosAngle, font, xScale, yScale, color.packed);
            }
        }
        if ( (renderFlags & 0x4000) != 0 )
        {
            fadeColor = 0;
            if ( 16 * redactAlpha > 255 )
                v27 = -1;
            else
                v27 = 16 * redactAlpha;
            HIBYTE(fadeColor) = v27;
            for ( i = 0; i < redactLeftCount; ++i )
            {
                SetupRedactFXVars(
                    &randSeed,
                    fxBirthTime,
                    maxLength,
                    fxRedactDecayStartTime,
                    fxRedactDecayDuration,
                    1,
                    &redactLeftRect[4 * i],
                    total_rect,
                    out_rect,
                    &u,
                    &mat);
                if ( u != 0.0 )
                    RB_DrawStretchPic(
                        mat,
                        out_rect[0],
                        minY,
                        out_rect[2] - out_rect[0],
                        maxY - minY,
                        0.0,
                        0.0,
                        u,
                        1.0,
                        fadeColor,
                        GFX_PRIM_STATS_HUD);
            }
            for ( j = 0; j < redactCount; ++j )
                RB_DrawStretchPic(
                    rgp.redactMaterial1,
                    redactRect[j][0],
                    minY,
                    redactRect[j][2] - redactRect[j][0],
                    maxY - minY,
                    0.0,
                    0.0,
                    1.0,
                    1.0,
                    fadeColor,
                    GFX_PRIM_STATS_HUD);
        }
    }
}

void __cdecl RB_DrawStretchPicRotate(
                const Material *material,
                float x,
                float y,
                float w,
                float width,
                float height,
                float s0,
                float t0,
                float s1,
                float t1,
                float sinAngle,
                float cosAngle,
                unsigned int color,
                GfxPrimStatsTarget statsTarget)
{
    float stepY; // [esp+1Ch] [ebp-18h]
    int indexCount; // [esp+2Ch] [ebp-8h]
    unsigned __int16 vertCount; // [esp+30h] [ebp-4h]

    if (gfxCmdBufSourceState.viewMode != VIEW_MODE_2D
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp",
            722,
            0,
            "%s",
            "gfxCmdBufSourceState.viewMode == VIEW_MODE_2D"))
    {
        __debugbreak();
    }
    RB_SetTessTechnique(material, 4u);
    R_TrackPrims(&gfxCmdBufState.prim, statsTarget);
    RB_CheckTessOverflow(4, 6);
    vertCount = tess.vertexCount;
    indexCount = tess.indexCount;
    tess.vertexCount += 4;
    tess.indexCount += 6;
    tess.indices[indexCount] = vertCount + 3;
    tess.indices[indexCount + 1] = vertCount;
    tess.indices[indexCount + 2] = vertCount + 2;
    tess.indices[indexCount + 3] = vertCount + 2;
    tess.indices[indexCount + 4] = vertCount;
    tess.indices[indexCount + 5] = vertCount + 1;
    stepY = (-(height)) * sinAngle;
    R_SetVertex4d(&tess.verts[vertCount], x, y, 0.0, w, s0, t0, color);
    R_SetVertex4d(
        &tess.verts[vertCount + 1],
        x + (float)(width * cosAngle),
        y + (float)(width * sinAngle),
        0.0,
        w,
        s1,
        t0,
        color);
    R_SetVertex4d(
        &tess.verts[vertCount + 2],
        (float)(x + (float)(width * cosAngle)) + stepY,
        (float)(y + (float)(width * sinAngle)) + (float)(height * cosAngle),
        0.0,
        w,
        s1,
        t1,
        color);
    R_SetVertex4d(&tess.verts[vertCount + 3], x + stepY, y + (float)(height * cosAngle), 0.0, w, s0, t1, color);
}

void __cdecl RB_LookupColor(unsigned __int8 c, unsigned __int8 *color)
{
    unsigned __int8 v2; // al

    if ( c == 56 )
    {
        *(unsigned int *)color = rg.color_allies.packed;
    }
    else if ( c == 57 )
    {
        *(unsigned int *)color = rg.color_axis.packed;
    }
    else
    {
        v2 = ColorIndex(c);
        if ( v2 >= 0x11u )
            *(unsigned int *)color = -1;
        else
            *(unsigned int *)color = *(unsigned int *)&color_table[v2][0];
    }
}

double __cdecl RB_DrawHudIcon(
                const char *text,
                float x,
                float y,
                float sinAngle,
                float cosAngle,
                Font_s *font,
                float xScale,
                float yScale,
                unsigned int color)
{
    const Material *material; // [esp+38h] [ebp-14h]
    float s1; // [esp+3Ch] [ebp-10h]
    float s0; // [esp+40h] [ebp-Ch]
    float h; // [esp+44h] [ebp-8h]
    float w; // [esp+48h] [ebp-4h]
    float ya; // [esp+5Ch] [ebp+10h]

    if ( !text && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp", 3251, 0, "%s", "text") )
        __debugbreak();
    if ( *text == 1 )
    {
        s0 = 0.0f;
        s1 = 1.0f;
    }
    else
    {
        if ( *text != 2
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp",
                        3260,
                        0,
                        "text[0] == CONTXTCMD_TYPE_HUDICON_FLIP\n\t%i, %i",
                        *text,
                        2) )
        {
            __debugbreak();
        }
        s0 = 1.0f;
        s1 = 0.0f;
    }
    w = (float)((font->pixelHeight * (text[1] - 16) + 16) / 32) * xScale;
    h = (float)((font->pixelHeight * (text[2] - 16) + 16) / 32) * yScale;
    ya = y - (float)((float)((float)((float)font->pixelHeight * yScale) + h) * 0.5);
    if ( w <= 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp", 3269, 0, "%s", "w > 0") )
    {
        __debugbreak();
    }
    if ( h <= 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp", 3270, 0, "%s", "h > 0") )
    {
        __debugbreak();
    }
    if ( !IsValidMaterialHandle(*(Material *const *)(text + 3)) )
        return 0.0;
    material = Material_FromHandle(*(Material **)(text + 3));
    RB_DrawStretchPicRotate(material, x, ya, 1.0, w, h, s0, 0.0, s1, 1.0, sinAngle, cosAngle, color, GFX_PRIM_STATS_HUD);
    return w;
}

GfxColor __cdecl RB_ProcessLetterColor(unsigned int color, unsigned int letter)
{
    GfxColor newColor; // [esp+4h] [ebp-4h]

    if ( ValidGamePadButtonIcon(letter) )
    {
        newColor.packed = -1;
        newColor.array[3] = HIBYTE(color);
        return newColor;
    }
    else
    {
        return (GfxColor)color;
    }
}

void __cdecl RB_DrawCursor(
                const Material *material,
                unsigned __int8 cursor,
                float x,
                float y,
                float sinAngle,
                float cosAngle,
                Font_s *font,
                float xScale,
                float yScale,
                unsigned int color)
{
    const Glyph *cursorGlyph; // [esp+40h] [ebp-8h]
    GfxColor newColor; // [esp+44h] [ebp-4h]

    if ( !font && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp", 3344, 0, "%s", "font") )
        __debugbreak();
    if ( ((CL_ScaledMilliseconds() / 256) & 1) == 0 )
    {
        cursorGlyph = R_GetCharacterGlyph(font, cursor);
        newColor.packed = RB_ProcessLetterColor(color, cursor).packed;
        RB_DrawStretchPicRotate(
            material,
            x,
            (float)((float)cursorGlyph->y0 * yScale) + y,
            1.0,
            (float)cursorGlyph->pixelWidth * xScale,
            (float)cursorGlyph->pixelHeight * yScale,
            cursorGlyph->s0,
            cursorGlyph->t0,
            cursorGlyph->s1,
            cursorGlyph->t1,
            sinAngle,
            cosAngle,
            newColor.packed,
            GFX_PRIM_STATS_HUD);
    }
}

void __cdecl RotateXY(
                float cosAngle,
                float sinAngle,
                float pivotX,
                float pivotY,
                float x,
                float y,
                float *outX,
                float *outY)
{
    *outX = (float)((float)((float)(x - pivotX) * cosAngle) + pivotX) - (float)((float)(y - pivotY) * sinAngle);
    *outY = (float)((float)((float)(y - pivotY) * cosAngle) + pivotY) + (float)((float)(x - pivotX) * sinAngle);
}

double __cdecl GetMonospaceWidth(Font_s *font, char renderFlags)
{
    if ( (renderFlags & 1) != 0 )
        return (double)R_GetCharacterGlyph(font, 0x6Fu)->dx;
    else
        return 0.0;
}

void __cdecl GlowColor(GfxColor *result, GfxColor baseColor, GfxColor forcedGlowColor, char renderFlags)
{
    if ( (renderFlags & 0x20) != 0 )
    {
        *(_WORD *)((char *)&result->packed + 1) = *(_WORD *)((char *)&forcedGlowColor.packed + 1);
        result->array[0] = forcedGlowColor.array[0];
    }
    else
    {
        result->array[2] = (int)(float)((float)baseColor.array[2] * 0.059999999);
        result->array[1] = (int)(float)((float)baseColor.array[1] * 0.059999999);
        result->array[0] = (int)(float)((float)baseColor.array[0] * 0.059999999);
    }
}

char __cdecl SetupFadeinFXVars(
                const char *text,
                int maxLength,
                int renderFlags,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration,
                bool *resultDrawRandChar,
                int *resultRandSeed,
                bool *resultDecaying,
                int *resultdecayTimeElapsed)
{
    int timeElapsed; // [esp+0h] [ebp-10h]
    int decayTimeElapsed; // [esp+4h] [ebp-Ch]
    bool decaying; // [esp+Fh] [ebp-1h]

    decaying = 0;
    timeElapsed = gfxCmdBufSourceState.sceneDef.time - fxBirthTime;
    if (gfxCmdBufSourceState.sceneDef.time - fxBirthTime < 0
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp",
            3404,
            0,
            "%s",
            "timeElapsed >= 0"))
    {
        __debugbreak();
    }
    if ( timeElapsed <= fxDecayDuration + fxDecayStartTime )
    {
        decayTimeElapsed = 0;
        if ( timeElapsed <= fxBirthTime )
        {
            decaying = 1;
            decayTimeElapsed = fxDecayDuration - timeElapsed;
        }
        else if ( timeElapsed > fxDecayStartTime )
        {
            decaying = 1;
            decayTimeElapsed = timeElapsed - fxDecayStartTime;
        }
        *resultDrawRandChar = 0;
        *resultRandSeed = 0;
        *resultDecaying = decaying;
        *resultdecayTimeElapsed = decayTimeElapsed;
        return 1;
    }
    else
    {
        *resultDrawRandChar = 0;
        *resultRandSeed = 1;
        *resultDecaying = 0;
        *resultdecayTimeElapsed = 0;
        return 0;
    }
}

char __cdecl SetupCOD7DecodeFXVars(
                const char *text,
                int maxLength,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration,
                int *randSeed,
                int *resultRandomCharsLength,
                int *resultMaxLength,
                bool *resultDecaying,
                int *resultDecayTimeElapsed)
{
    int timeElapsed; // [esp+4h] [ebp-Ch]
    int strLength; // [esp+Ch] [ebp-4h]

    strLength = SEH_PrintStrlen(text);
    if ( strLength > maxLength )
        strLength = maxLength;
    timeElapsed = gfxCmdBufSourceState.sceneDef.time - fxBirthTime;
    if (gfxCmdBufSourceState.sceneDef.time - fxBirthTime <= fxDecayDuration + fxDecayStartTime)
    {
        if ( timeElapsed <= fxLetterTime * strLength || timeElapsed <= fxDecayStartTime )
        {
            *resultDecaying = 0;
            *resultDecayTimeElapsed = 0;
            *resultMaxLength = strLength;
            *resultRandomCharsLength = strLength - timeElapsed / fxLetterTime;
            *randSeed = gfxCmdBufSourceState.sceneDef.time / 50;
            RandWithSeed(randSeed);
            RandWithSeed(randSeed);
            return 1;
        }
        else
        {
            *resultDecaying = 1;
            *resultDecayTimeElapsed = timeElapsed - fxDecayStartTime;
            *resultMaxLength = maxLength;
            *resultRandomCharsLength = 0;
            return 1;
        }
    }
    else
    {
        *resultDecaying = 1;
        *resultDecayTimeElapsed = 0;
        *resultMaxLength = maxLength;
        *resultRandomCharsLength = 0;
        return 0;
    }
}

char __cdecl SetupTypewriterFXVars(
                const char *text,
                int maxLength,
                int renderFlags,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration,
                bool *resultDrawRandChar,
                int *resultRandSeed,
                int *resultMaxLength,
                bool *resultDecaying,
                int *resultdecayTimeElapsed)
{
    int timeElapsed; // [esp+8h] [ebp-14h]
    int strLength; // [esp+10h] [ebp-Ch]
    int decayTimeElapsed; // [esp+14h] [ebp-8h]
    bool decaying; // [esp+1Bh] [ebp-1h]

    decaying = 0;
    timeElapsed = gfxCmdBufSourceState.sceneDef.time - fxBirthTime;
    if (gfxCmdBufSourceState.sceneDef.time - fxBirthTime < 0
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp",
            3497,
            0,
            "%s",
            "timeElapsed >= 0"))
    {
        __debugbreak();
    }
    strLength = SEH_PrintStrlen(text);
    if (strLength > maxLength)
        strLength = maxLength;
    if (timeElapsed <= fxDecayDuration + fxDecayStartTime)
    {
        decayTimeElapsed = 0;
        if (timeElapsed <= fxLetterTime * strLength)
        {
            if (!fxLetterTime
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp",
                    3531,
                    0,
                    "%s",
                    "fxLetterTime"))
            {
                __debugbreak();
            }
            maxLength = timeElapsed / fxLetterTime;
        }
        else if (timeElapsed > fxDecayStartTime)
        {
            decaying = 1;
            decayTimeElapsed = timeElapsed - fxDecayStartTime;
        }
        *resultDrawRandChar = 0;
        *resultRandSeed = 0;
        *resultMaxLength = maxLength;
        *resultDecaying = decaying;
        *resultdecayTimeElapsed = decayTimeElapsed;
        return 1;
    }
    else
    {
        *resultDrawRandChar = 0;
        *resultRandSeed = 1;
        *resultMaxLength = maxLength;
        *resultDecaying = 0;
        *resultdecayTimeElapsed = 0;
        return 0;
    }
}

char __cdecl SetupPopInFXVars(
                const char *text,
                int maxLength,
                int renderFlags,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration,
                float *sizeIncrease,
                int *resultRandSeed,
                int *resultMaxLength,
                bool *resultDecaying,
                int *resultdecayTimeElapsed)
{
    int timeElapsed; // [esp+8h] [ebp-14h]
    int strLength; // [esp+10h] [ebp-Ch]
    int decayTimeElapsed; // [esp+14h] [ebp-8h]
    bool decaying; // [esp+1Bh] [ebp-1h]

    decaying = 0;
    timeElapsed = gfxCmdBufSourceState.sceneDef.time - fxBirthTime;
    if (gfxCmdBufSourceState.sceneDef.time - fxBirthTime < 0
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp",
            3559,
            0,
            "%s",
            "timeElapsed >= 0"))
    {
        __debugbreak();
    }
    strLength = SEH_PrintStrlen(text);
    if (strLength > maxLength)
        strLength = maxLength;
    if (timeElapsed <= fxDecayDuration + fxDecayStartTime)
    {
        decayTimeElapsed = 0;
        if (timeElapsed <= fxLetterTime * strLength)
        {
            if (!fxLetterTime
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp",
                    3591,
                    0,
                    "%s",
                    "fxLetterTime"))
            {
                __debugbreak();
            }
            maxLength = timeElapsed / fxLetterTime;
        }
        else if (timeElapsed > fxDecayStartTime)
        {
            decaying = 1;
            decayTimeElapsed = timeElapsed - fxDecayStartTime;
        }
        *sizeIncrease = 2.0f;
        *resultRandSeed = 0;
        *resultMaxLength = maxLength;
        *resultDecaying = decaying;
        *resultdecayTimeElapsed = decayTimeElapsed;
        return 1;
    }
    else
    {
        *resultRandSeed = 1;
        *resultMaxLength = maxLength;
        *resultDecaying = 0;
        *resultdecayTimeElapsed = 0;
        return 0;
    }
}

char __cdecl SetupPulseFXVars(
                const char *text,
                int maxLength,
                char renderFlags,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration,
                bool *resultDrawRandChar,
                int *resultRandSeed,
                int *resultMaxLength,
                bool *resultDecaying,
                int *resultdecayTimeElapsed)
{
    int timeRemainder; // [esp+0h] [ebp-24h]
    int timeElapsed; // [esp+8h] [ebp-1Ch]
    int randSeed; // [esp+10h] [ebp-14h] BYREF
    int strLength; // [esp+14h] [ebp-10h]
    bool drawRandCharAtEnd; // [esp+1Bh] [ebp-9h]
    int decayTimeElapsed; // [esp+1Ch] [ebp-8h]
    bool decaying; // [esp+23h] [ebp-1h]
    int maxLengtha; // [esp+30h] [ebp+Ch]

    if ((renderFlags & 0x40) != 0)
    {
        drawRandCharAtEnd = 0;
        randSeed = 1;
        decaying = 0;
        decayTimeElapsed = 0;
        timeElapsed = gfxCmdBufSourceState.sceneDef.time - fxBirthTime;
        if (gfxCmdBufSourceState.sceneDef.time - fxBirthTime < 0
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp",
                3636,
                0,
                "%s",
                "timeElapsed >= 0"))
        {
            __debugbreak();
        }
        strLength = SEH_PrintStrlen(text);
        if (strLength > maxLength)
            strLength = maxLength;
        if (timeElapsed <= fxDecayDuration + fxDecayStartTime)
        {
            if (timeElapsed < fxLetterTime * strLength)
            {
                if (!fxLetterTime
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp",
                        3671,
                        0,
                        "%s",
                        "fxLetterTime"))
                {
                    __debugbreak();
                }
                maxLengtha = timeElapsed / fxLetterTime;
                drawRandCharAtEnd = 1;
                timeRemainder = timeElapsed % fxLetterTime;
                if (fxLetterTime / 4)
                    timeRemainder /= fxLetterTime / 4;
                randSeed = maxLengtha + timeRemainder + strLength + fxBirthTime;
                RandWithSeed(&randSeed);
                RandWithSeed(&randSeed);
                maxLength = maxLengtha + 1;
            }
            else if (timeElapsed > fxDecayStartTime)
            {
                decaying = 1;
                randSeed = strLength + fxBirthTime;
                RandWithSeed(&randSeed);
                RandWithSeed(&randSeed);
                decayTimeElapsed = timeElapsed - fxDecayStartTime;
            }
            *resultDrawRandChar = drawRandCharAtEnd;
            *resultRandSeed = randSeed;
            *resultMaxLength = maxLength;
            *resultDecaying = decaying;
            *resultdecayTimeElapsed = decayTimeElapsed;
            return 1;
        }
        else
        {
            *resultDrawRandChar = 0;
            *resultRandSeed = 1;
            *resultMaxLength = maxLength;
            *resultDecaying = 0;
            *resultdecayTimeElapsed = 0;
            return 0;
        }
    }
    else
    {
        *resultDrawRandChar = 0;
        *resultRandSeed = 1;
        *resultMaxLength = maxLength;
        *resultDecaying = 0;
        *resultdecayTimeElapsed = 0;
        return 1;
    }
}

char __cdecl SetupRedactTextFXVars(
                const char *text,
                int maxLength,
                int renderFlags,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration,
                bool *resultDrawRandChar,
                int *resultRandSeed,
                int *resultMaxLength,
                bool *resultDecaying,
                int *resultdecayTimeElapsed)
{
    int timeElapsed; // [esp+0h] [ebp-8h]
    int decayTimeElapsed; // [esp+4h] [ebp-4h]

    decayTimeElapsed = 0;
    *resultDrawRandChar = 0;
    *resultRandSeed = 1;
    *resultMaxLength = maxLength;
    *resultDecaying = 0;
    *resultdecayTimeElapsed = 0;
    timeElapsed = gfxCmdBufSourceState.sceneDef.time - fxBirthTime;
    if (gfxCmdBufSourceState.sceneDef.time - fxBirthTime < 0
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp",
            3735,
            0,
            "%s",
            "timeElapsed >= 0"))
    {
        __debugbreak();
    }
    if (timeElapsed > fxDecayDuration + fxDecayStartTime)
        return 0;
    if (timeElapsed > fxDecayStartTime)
    {
        decayTimeElapsed = timeElapsed - fxDecayStartTime;
        *resultDecaying = 1;
    }
    *resultdecayTimeElapsed = decayTimeElapsed;
    return 1;
}

void __cdecl SetupRedactFXVars(
                int *randSeed,
                int fxBirthTime,
                int maxLength,
                int fxRedactDecayStartTime,
                int fxRedactDecayDuration,
                int mode,
                float *src_rect,
                float *max_rect,
                float *out_rect,
                float *u,
                Material **mat)
{
    float v11; // [esp+0h] [ebp-54h]
    float v12; // [esp+4h] [ebp-50h]
    float v13; // [esp+8h] [ebp-4Ch]
    int v14; // [esp+Ch] [ebp-48h]
    int v15; // [esp+14h] [ebp-40h]
    int timeElapsed; // [esp+38h] [ebp-1Ch]
    float frac; // [esp+3Ch] [ebp-18h]
    float desired_rect; // [esp+44h] [ebp-10h]
    float desired_rect_4; // [esp+48h] [ebp-Ch]
    float desired_rect_8; // [esp+4Ch] [ebp-8h]
    float desired_rect_12; // [esp+50h] [ebp-4h]

    timeElapsed = gfxCmdBufSourceState.sceneDef.time - fxBirthTime;
    if (gfxCmdBufSourceState.sceneDef.time - fxBirthTime < 0
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp",
            3761,
            0,
            "%s",
            "timeElapsed >= 0"))
    {
        __debugbreak();
    }
    if (timeElapsed > fxRedactDecayDuration + fxRedactDecayStartTime)
        timeElapsed = fxRedactDecayDuration + fxRedactDecayStartTime;
    v15 = (int)RandWithSeed(randSeed) % 3;
    if (v15)
    {
        if (v15 == 1)
        {
            *mat = rgp.redactMaterial2;
        }
        else if (v15 == 2)
        {
            *mat = rgp.redactMaterial3;
        }
    }
    else
    {
        *mat = rgp.redactMaterial1;
    }
    *u = 0.0f;
    if (timeElapsed >= fxRedactDecayStartTime)
    {
        if (mode == 1)
        {
            desired_rect = *src_rect;
            desired_rect_4 = src_rect[1];
            desired_rect_8 = (float)(src_rect[2] - *src_rect) + *src_rect;
            desired_rect_12 = src_rect[3];
            if ((float)(*max_rect - *src_rect) < 0.0)
                v14 = *(_DWORD *)src_rect;
            else
                v14 = *(_DWORD *)max_rect;
            *(_DWORD *)out_rect = v14;
            if ((float)(max_rect[1] - desired_rect_4) < 0.0)
                v13 = desired_rect_4;
            else
                v13 = max_rect[1];
            out_rect[1] = v13;
            frac = (float)(timeElapsed - fxRedactDecayStartTime) / (float)fxRedactDecayDuration;
            if ((float)(desired_rect_8 - (float)((float)((float)(max_rect[2] - *max_rect) * frac) + *max_rect)) < 0.0)
                v12 = desired_rect_8;
            else
                v12 = (float)((float)(max_rect[2] - *max_rect) * frac) + *max_rect;
            out_rect[2] = v12;
            if ((float)(desired_rect_12 - max_rect[3]) < 0.0)
                v11 = desired_rect_12;
            else
                v11 = max_rect[3];
            out_rect[3] = v11;
            *u = (float)(out_rect[2] - *out_rect) / (float)(desired_rect_8 - desired_rect);
        }
    }
    else
    {
        *out_rect = *src_rect;
        out_rect[1] = src_rect[1];
        out_rect[2] = src_rect[2];
        out_rect[3] = src_rect[3];
    }
}

void __cdecl GetDecayingStringAlphaInfo(
                int decayTimeElapsed,
                int fxDecayDuration,
                unsigned __int8 alpha,
                unsigned __int8 *resultAlpha)
{
    unsigned __int8 v4; // [esp+0h] [ebp-54h]
    unsigned __int8 v5; // [esp+4h] [ebp-50h]
    int v6; // [esp+Ch] [ebp-48h]
    int v7; // [esp+28h] [ebp-2Ch]
    float fade; // [esp+48h] [ebp-Ch]

    fade = 1.0f;
    if ( (int)(float)(30.0 * (float)((float)fxDecayDuration / 1000.0)) > 0 )
    {
        if ( decayTimeElapsed <= fxDecayDuration )
            fade = (float)((float)alpha / 255.0) * (float)(1.0 - (float)((float)decayTimeElapsed / (float)fxDecayDuration));
        if ( (int)((float)(255.0 * fade) + 9.313225746154785e-10) < 255 )
            v6 = (int)((float)(255.0 * fade) + 9.313225746154785e-10);
        else
            v6 = 255;
        if ( v6 > 0 )
            v4 = v6;
        else
            v4 = 0;
        *resultAlpha = v4;
    }
    else
    {
        if ( (int)((float)(255.0 * 0.0) + 9.313225746154785e-10) < 255 )
            v7 = (int)((float)(255.0 * 0.0) + 9.313225746154785e-10);
        else
            v7 = 255;
        if ( v7 > 0 )
            v5 = v7;
        else
            v5 = 0;
        *resultAlpha = v5;
    }
}

void __cdecl DrawTextFxExtraCharacter(
                const Material *material,
                int charIndex,
                float x,
                float y,
                float w,
                float h,
                float sinAngle,
                float cosAngle,
                unsigned int color)
{
    RB_DrawStretchPicRotate(
        material,
        x,
        y,
        1.0,
        w,
        h,
        (float)(charIndex % 16) * 0.0625,
        0.0,
        (float)((float)(charIndex % 16) * 0.0625) + 0.0625,
        1.0,
        sinAngle,
        cosAngle,
        color,
        GFX_PRIM_STATS_HUD);
}

int __cdecl ModulateByteColors(unsigned __int8 colorA, unsigned __int8 colorB)
{
    return (int)(float)((float)((float)((float)colorA / 255.0) * (float)((float)colorB / 255.0)) * 255.0);
}

void __cdecl RB_DrawTextInSpace(
        const char *text,
        Font_s *font,
        const float *org,
        const float *xPixelStep,
        const float *yPixelStep,
        unsigned int color)
{
  float dx; // [esp+0h] [ebp-54h]
  float pixelHeight; // [esp+4h] [ebp-50h]
  float pixelWidth; // [esp+8h] [ebp-4Ch]
  float y0; // [esp+Ch] [ebp-48h]
  float x0; // [esp+10h] [ebp-44h]
  float curOrg; // [esp+14h] [ebp-40h]
  float curOrg_4; // [esp+18h] [ebp-3Ch]
  float curOrg_8; // [esp+1Ch] [ebp-38h]
  float v14[3]; // [esp+20h] [ebp-34h] BYREF
  const Glyph *glyph; // [esp+2Ch] [ebp-28h]
  float xyz[3]; // [esp+30h] [ebp-24h] BYREF
  const Material *material; // [esp+3Ch] [ebp-18h]
  unsigned int letter; // [esp+40h] [ebp-14h]
  float dy[3]; // [esp+44h] [ebp-10h] BYREF
  unsigned int newColor; // [esp+50h] [ebp-4h]

  if ( !text && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp", 4597, 0, "%s", "text") )
    __debugbreak();
  if ( !font && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp", 4598, 0, "%s", "font") )
    __debugbreak();
  material = Material_FromHandle(font->material);
  if ( !material
    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp", 4601, 0, "%s", "material") )
  {
    __debugbreak();
  }
  if ( tess.indexCount )
    RB_EndTessSurface();
  R_Set3D(&gfxCmdBufSourceState);
  curOrg = (float)(-0.5 * *yPixelStep) + (float)((float)(-0.5 * *xPixelStep) + *org);
  curOrg_4 = (float)(-0.5 * yPixelStep[1]) + (float)((float)(-0.5 * xPixelStep[1]) + org[1]);
  curOrg_8 = (float)(-0.5 * yPixelStep[2]) + (float)((float)(-0.5 * xPixelStep[2]) + org[2]);
  while ( *text )
  {
    letter = SEH_ReadCharFromString(&text, 0);
    if ( !text && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp", 4611, 0, "%s", "text") )
      __debugbreak();
    glyph = R_GetCharacterGlyph(font, letter);
    newColor = RB_ProcessLetterColor(color, letter);
    x0 = (float)glyph->x0;
    xyz[0] = (float)(x0 * *xPixelStep) + curOrg;
    xyz[1] = (float)(x0 * xPixelStep[1]) + curOrg_4;
    xyz[2] = (float)(x0 * xPixelStep[2]) + curOrg_8;
    y0 = (float)glyph->y0;
    xyz[0] = (float)(y0 * *yPixelStep) + xyz[0];
    xyz[1] = (float)(y0 * yPixelStep[1]) + xyz[1];
    xyz[2] = (float)(y0 * yPixelStep[2]) + xyz[2];
    pixelWidth = (float)glyph->pixelWidth;
    v14[0] = pixelWidth * *xPixelStep;
    v14[1] = pixelWidth * xPixelStep[1];
    v14[2] = pixelWidth * xPixelStep[2];
    pixelHeight = (float)glyph->pixelHeight;
    dy[0] = pixelHeight * *yPixelStep;
    dy[1] = pixelHeight * yPixelStep[1];
    dy[2] = pixelHeight * yPixelStep[2];
    RB_DrawCharInSpace(material, xyz, v14, dy, glyph, newColor);
    dx = (float)glyph->dx;
    curOrg = (float)(dx * *xPixelStep) + curOrg;
    curOrg_4 = (float)(dx * xPixelStep[1]) + curOrg_4;
    curOrg_8 = (float)(dx * xPixelStep[2]) + curOrg_8;
  }
  if ( tess.indexCount )
    RB_EndTessSurface();
}

void __cdecl RB_DrawCharInSpace(
                const Material *material,
                float *xyz,
                const float *dx,
                const float *dy,
                const Glyph *glyph,
                unsigned int color)
{
    int indexCount; // [esp+18h] [ebp-Ch]
    unsigned __int16 vertCount; // [esp+1Ch] [ebp-8h]
    GfxColor unpackedColor; // [esp+20h] [ebp-4h] BYREF

    RB_SetTessTechnique(material, 4u);
    R_TrackPrims(&gfxCmdBufState.prim, GFX_PRIM_STATS_DEBUG);
    RB_CheckTessOverflow(4, 6);
    vertCount = tess.vertexCount;
    indexCount = LOWORD(tess.indexCount);
    tess.vertexCount += 4;
    tess.indexCount += 6;
    tess.indices[indexCount] = vertCount + 3;
    tess.indices[indexCount + 1] = vertCount;
    tess.indices[indexCount + 2] = vertCount + 2;
    tess.indices[indexCount + 3] = vertCount + 2;
    tess.indices[indexCount + 4] = vertCount;
    tess.indices[indexCount + 5] = vertCount + 1;
    unpackedColor.packed = color;
    R_SetVertex3d(
        &tess.verts[vertCount],
        *xyz,
        xyz[1],
        xyz[2],
        glyph->s0,
        glyph->t0,
        (const unsigned __int8 *)&unpackedColor);
    R_SetVertex3d(
        &tess.verts[vertCount + 1],
        *xyz + *dx,
        xyz[1] + dx[1],
        xyz[2] + dx[2],
        glyph->s1,
        glyph->t0,
        (const unsigned __int8 *)&unpackedColor);
    R_SetVertex3d(
        &tess.verts[vertCount + 2],
        (float)(*xyz + *dx) + *dy,
        (float)(xyz[1] + dx[1]) + dy[1],
        (float)(xyz[2] + dx[2]) + dy[2],
        glyph->s1,
        glyph->t1,
        (const unsigned __int8 *)&unpackedColor);
    R_SetVertex3d(
        &tess.verts[vertCount + 3],
        *xyz + *dy,
        xyz[1] + dy[1],
        xyz[2] + dy[2],
        glyph->s0,
        glyph->t1,
        (const unsigned __int8 *)&unpackedColor);
}

void __cdecl RB_DrawText2DCmd(GfxRenderCommandExecState *execState)
{
    float v1; // [esp+70h] [ebp-10h]
    float cosAngle; // [esp+74h] [ebp-Ch]
    float sinAngle; // [esp+78h] [ebp-8h]
    const GfxCmdDrawText2D *cmd; // [esp+7Ch] [ebp-4h]

    cmd = (const GfxCmdDrawText2D *)execState->cmd;
    v1 = *((float *)execState->cmd + 4) * 0.017453292;
    cosAngle = cos(v1);
    sinAngle = sin(v1);
    PROF_SCOPED("RB_DrawText2DCmd");
    ZoneTextF("(%s)", cmd->text);
    DrawText2D(
        cmd->text,
        cmd->x,
        cmd->y,
        cmd->w,
        cmd->font,
        cmd->xScale,
        cmd->yScale,
        sinAngle,
        cosAngle,
        cmd->color,
        cmd->maxChars,
        cmd->renderFlags,
        cmd->cursorPos,
        cmd->cursorLetter,
        cmd->padding,
        cmd->glowForceColor,
        cmd->fxBirthTime,
        cmd->fxLetterTime,
        cmd->fxDecayStartTime,
        cmd->fxDecayDuration,
        cmd->fxRedactDecayStartTime,
        cmd->fxRedactDecayDuration,
        cmd->fxMaterial,
        cmd->fxMaterialGlow);
    execState->cmd = (char *)execState->cmd + *(unsigned __int16 *)execState->cmd;
}

void __cdecl RB_DrawText3DCmd(GfxRenderCommandExecState *execState)
{
    RB_DrawTextInSpace(
        (const char *)execState->cmd + 48,
        *((Font_s **)execState->cmd + 4),
        (const float *)execState->cmd + 1,
        (const float *)execState->cmd + 5,
        (const float *)execState->cmd + 8,
        *((unsigned int *)execState->cmd + 11));
    execState->cmd = (char *)execState->cmd + *(unsigned __int16 *)execState->cmd;
}

void __cdecl RB_ProjectionSetCmd(GfxRenderCommandExecState *execState)
{
  int v1; // [esp+0h] [ebp-Ch]

  v1 = *((unsigned int *)execState->cmd + 1);
  if ( v1 )
  {
    if ( v1 == 1 )
    {
      if ( tess.indexCount )
        RB_EndTessSurface();
      R_Set3D(&gfxCmdBufSourceState);
    }
    else if ( !Assert_MyHandler(
                 "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp",
                 4687,
                 0,
                 "Invalid projection type") )
    {
      __debugbreak();
    }
  }
  else
  {
    if ( tess.indexCount )
      RB_EndTessSurface();
    R_Set2D(&gfxCmdBufSourceState);
  }
  execState->cmd = (char *)execState->cmd + *(unsigned __int16 *)execState->cmd;
}

void __cdecl RB_ResetStatTracking(int viewIndex)
{
    RB_Stats_UpdateMaxs(&g_frameStatsCur, &backEnd.frameStatsMax);
    memset(&g_frameStatsCur, 0, sizeof(g_frameStatsCur));
    g_viewStats = (GfxViewStats *)&g_frameStatsCur;
    if ( rg.globalstats )
        rg.stats = &rg.globalstats->views[viewIndex];
    else
        rg.stats = 0;
}

void __cdecl RB_BeginFrame(GfxBackEndData *data)
{
    const char *v1; // eax
    int semaphore; // [esp+0h] [ebp-Ch]
    int hr; // [esp+4h] [ebp-8h]
    int rt; // [esp+8h] [ebp-4h]

    backEndData = data;
    if ((data->drawType & 1) != 0)
    {
        if (!rg.renderHiResShot)
            ++r_glob.backEndFrameCount;
        for (rt = 0; rt < 44; ++rt)
            gfxRenderTargets[rt].cleared = 0;
        R_Cinematic_UpdateFrame(0);
        RB_UpdateBackEndDvarOptions();
        RB_PatchStaticModelCache();
        RB_PatchModelLighting(backEndData->modelLightingPatchList, backEndData->modelLightingPatchCount);
        
        iassert(dx.device);
        iassert(!dx.inScene);

        dx.inScene = 1;
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("dx.device->BeginScene()\n");
        hr = dx.device->BeginScene();
        if (hr < 0)
        {
            ++g_disableRendering;
            v1 = R_ErrorDescription(hr);
            Com_Error(
                ERR_FATAL,
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp (%i) dx.device->BeginScene() failed: %s\n",
                5068,
                v1);
        }
        R_InitCmdBufSamplerState(&gfxCmdBufState);
        R_Cinematic_UpdateFrame(0);
        tess.indexCount = 0;
        tess.vertexCount = 0;
    }
}

const GfxBackEndData *RB_PatchStaticModelCache()
{
    const GfxBackEndData *result; // eax
    IDirect3DVertexBuffer9 *handle; // [esp+0h] [ebp-24h]
    static_model_leaf_t *cachedSurf; // [esp+8h] [ebp-1Ch]
    unsigned int firstPatchVert; // [esp+14h] [ebp-10h]
    unsigned int patchIter; // [esp+18h] [ebp-Ch]
    void *bufferData; // [esp+1Ch] [ebp-8h]
    unsigned int vertCount; // [esp+20h] [ebp-4h]

    result = backEndData;
    if ( backEndData->smcPatchCount )
    {
        handle = gfxBuf.smodelCacheVb;
        firstPatchVert = 0;
        for ( patchIter = 0; ; ++patchIter )
        {
            result = backEndData;
            if ( patchIter >= backEndData->smcPatchCount )
                break;
            cachedSurf = R_GetCachedSModelSurf(backEndData->smcPatchList[patchIter]);
            vertCount = XModelGetStaticModelCacheVertCount(rgp.world->dpvs.smodelDrawInsts[cachedSurf->cachedSurf.smodelIndex].model, cachedSurf->cachedSurf.lodIndex);
            bufferData = R_LockVertexBuffer(handle, 32 * cachedSurf->cachedSurf.baseVertIndex, 32 * vertCount, 0x1000u);
            Com_Memcpy(bufferData, &backEndData->smcPatchVerts[firstPatchVert], 32 * vertCount);
            R_UnlockVertexBuffer(handle);
            firstPatchVert += vertCount;
        }
    }
    return result;
}

void __cdecl RB_EndFrame(char drawType)
{
    if ( (drawType & 2) != 0 )
    {
        if ( r_logFile->current.integer )
            RB_LogPrint("***************** RB_SwapBuffers *****************\n\n\n");
        RB_SwapBuffers();
        RB_UpdateLogging();
        if ( !r_gamma
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp", 5655, 0, "%s", "r_gamma") )
        {
            __debugbreak();
        }
        if ( !r_ignoreHwGamma
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp",
                        5656,
                        0,
                        "%s",
                        "r_ignoreHwGamma") )
        {
            __debugbreak();
        }
        if ( r_gamma->modified || r_ignoreHwGamma->modified )
        {
            Dvar_ClearModified(r_gamma);
            Dvar_ClearModified(r_ignoreHwGamma);
            if ( !r_ignoreHwGamma->current.enabled )
                R_SetColorMappings();
        }
    }
}

void RB_SwapBuffers()
{
    unsigned __int8 stereoActivated; // [esp+2Fh] [ebp-2Dh] BYREF
    int semaphore; // [esp+30h] [ebp-2Ch]
    int actualShow; // [esp+34h] [ebp-28h]
    HRESULT hr; // [esp+38h] [ebp-24h]
    tagMSG msg; // [esp+3Ch] [ebp-20h] BYREF
    int desiredShow; // [esp+58h] [ebp-4h]

    PROF_SCOPED("RB_SwapBuffers"); // LWSS ADD

    iassert(dx.targetWindowIndex >= 0 && dx.targetWindowIndex < dx.windowCount);

    {
        PROF_SCOPED("Present()");

        if (rg.renderHiResShot || dx.resizeWindow)
            hr = 0;
        else
            hr = dx.windows[dx.targetWindowIndex].swapChain->Present(0, 0, 0, 0, 0);
    }

    FrameMark; // backend thread Frame marker

    mjpeg_draw();

    if ( dx.resizeWindow )
    {
        R_ResizeWindow();
        hr = -2005530520;
    }

    if ( hr == -2005530520 )
    {
        Sys_ClearD3DDeviceOKEvent();
        if ( !r_glob.isRenderingRemoteUpdate )
        {
            PROF_SCOPED("Sys_QueryRenderEvent() Loop"); // LWSS ADD
            while ( Sys_QueryRenderEvent() )
            {
                NET_Sleep(1u);
            }
        }
    }

    {
        PROF_SCOPED("Win32 Msg Pump"); // LWSS ADD

        desiredShow = g_showCursor - 1;
        for (actualShow = ShowCursor(g_showCursor); actualShow != desiredShow; actualShow = ShowCursor(actualShow < desiredShow))
            ;
        while (PeekMessageA(&msg, 0, 0, 0, 0))
        {
            if (!GetMessageA(&msg, 0, 0, 0))
                Sys_SetWin32QuitEvent();
            g_msgTime = msg.time;
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }

    dx.nvStereoActivated = 0;
    if ( dx.nvStereoHandle )
    {
        stereoActivated = 0;
        if ( !NvAPI_Stereo_IsActivated(dx.nvStereoHandle, &stereoActivated) )
            dx.nvStereoActivated = stereoActivated != 0;
    }

    if (dx.nvStereoActivated && !r_use_driver_convergence->current.enabled)
    {
        //((void(__cdecl *)(unsigned int, unsigned int))NvAPI_Stereo_SetConvergence)(dx.nvStereoHandle, r_convergence->current.value);
        NvAPI_Stereo_SetConvergence(dx.nvStereoHandle, r_convergence->current.value);
    }

    if ( hr == -2005530520 )
    {
        while ( hr )
        {
            hr = dx.device->TestCooperativeLevel();
            if ( dx.resizeWindow )
            {
                hr = -2005530519;
                dx.resizeWindow = 0;
            }
            if ( hr == -2005530520 )
            {
                if ( !assets_released )
                {
                    R_ReleaseLostDeviceAssets2();
                    R_ReleaseForShutdownOrReset();
                    assets_released = 1;
                }
                NET_Sleep(5u);
            }
            else if ( hr == -2005530519 )
            {
                if ( !assets_released )
                {
                    R_ReleaseLostDeviceAssets2();
                    R_ReleaseForShutdownOrReset();
                    assets_released = 1;
                }
                if ( R_RecoverLostDevice() )
                    assets_released = 0;
            }
            else if ( hr )
            {
                Com_Error(ERR_FATAL, "Direct3DDevice9::Present failed: %s (%d)\n", R_ErrorDescription(hr), hr);
            }

            PROF_SCOPED("(#2) Win32 Msg Pump"); // LWSS ADD

            while ( PeekMessageA(&msg, 0, 0, 0, 0) )
            {
                if ( !GetMessageA(&msg, 0, 0, 0) )
                    Sys_SetWin32QuitEvent();
                g_msgTime = msg.time;
                TranslateMessage(&msg);
                DispatchMessageA(&msg);
            }
        }
        Sys_SetD3DDeviceOKEvent();
    }
    else if ( hr )
    {
        if ( r_glob.isRenderingRemoteUpdate )
            Sys_LeaveCriticalSection(CRITSECT_DBHASH);
        Com_Error(ERR_FATAL, "Direct3DDevice9::Present failed: %s (%d)\n", R_ErrorDescription(hr), hr);
    }

    R_HW_InsertFence(&dx.swapFence[r_glob.backEndFrameCount % dx.gpuCount]);
    gfxBuf.dynamicIndexBuffer->used = 0;
}

void RB_UpdateBackEndDvarOptions()
{
    if ( !dx.device->TestCooperativeLevel() )
    {
        if ( R_CheckDvarModified(r_texFilterAnisoMax)
            || R_CheckDvarModified(r_texFilterDisable)
            || R_CheckDvarModified(r_texFilterAnisoMin)
            || R_CheckDvarModified(r_texFilterMipMode)
            || R_CheckDvarModified(r_texFilterMipBias) )
        {
            R_SetTexFilter();
        }
        if ( R_CheckDvarModified(r_showPixelCost) && !r_showPixelCost->current.integer )
            R_PixelCost_PrintColorCodeKey();
        if ( R_CheckDvarModified(r_aaAlpha) )
        {
            if ( gfxMetrics.hasTransparencyMsaa )
                R_SetAlphaAntiAliasingState(gfxCmdBufState.prim.device, gfxCmdBufState.activeStateBits[0]);
        }
    }
}

void __cdecl RB_ExecuteRenderCommandsLoop(const void *cmds, int *ui3dTextureWindow)
{
    bool v2; // [esp+0h] [ebp-34h]
    int v3; // [esp+4h] [ebp-30h]
    const GfxCmdHeader *header; // [esp+20h] [ebp-14h]
    bool shouldRun; // [esp+26h] [ebp-Eh]
    GfxRenderCommandExecState execState; // [esp+2Ch] [ebp-8h] BYREF
    const void *prevCmd; // [esp+30h] [ebp-4h]

    PROF_SCOPED("RB_ExecuteRenderCommandsLoop");

    if (gfxRenderTargets[R_RENDERTARGET_MISSILE_CAM].image)
        R_SetCodeImageTexture(&gfxCmdBufSourceState, 0x28u, gfxRenderTargets[R_RENDERTARGET_MISSILE_CAM].image);
    if (rgp.heatMapImage)
        R_SetCodeImageTexture(&gfxCmdBufSourceState, 0x2Au, rgp.heatMapImage);
    else
        R_SetCodeImageTexture(&gfxCmdBufSourceState, 0x2Au, rgp.whiteImage);
    if (gfxRenderTargets[R_RENDERTARGET_UI3D].image)
        R_SetCodeImageTexture(&gfxCmdBufSourceState, 0x27u, gfxRenderTargets[R_RENDERTARGET_UI3D].image);
    if (tess.indexCount
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp",
            5986,
            0,
            "%s\n\t(tess.indexCount) = %i",
            "(!tess.indexCount)",
            tess.indexCount))
    {
        __debugbreak();
    }
    execState.cmd = cmds;
    prevCmd = cmds;
    if (ui3dTextureWindow)
        v3 = *ui3dTextureWindow;
    else
        v3 = -1;
    while (1)
    {
        if (((int)execState.cmd & 3) != 0
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp",
                5999,
                0,
                "%s",
                "(reinterpret_cast< psize_int >( execState.cmd ) & 3) == 0"))
        {
            __debugbreak();
        }
        header = (const GfxCmdHeader *)execState.cmd;
        if (!*((_BYTE *)execState.cmd + 2))
            break;
        if ((*((_BYTE *)execState.cmd + 3) & 0x80) != 0)
        {
            v2 = v3 >= 0 && (*((_BYTE *)execState.cmd + 3) & 0x7F) == v3;
            shouldRun = v2;
        }
        else
        {
            shouldRun = v3 == -1;
        }
        if (shouldRun)
        {
            if (*((unsigned __int8 *)execState.cmd + 2) >= 0x1Du
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp",
                    6018,
                    0,
                    "%s\n\t(header->id) = %i",
                    "(header->id < (sizeof( RB_RenderCommandTable ) / (sizeof( RB_RenderCommandTable[0] ) * (sizeof( RB_RenderC"
                    "ommandTable ) != 4 || sizeof( RB_RenderCommandTable[0] ) <= 4))))",
                    *((unsigned __int8 *)execState.cmd + 2)))
            {
                __debugbreak();
            }
            if (!RB_RenderCommandTable[header->id]
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp",
                    6019,
                    0,
                    "%s\n\t(header->id) = %i",
                    "(RB_RenderCommandTable[header->id])",
                    header->id))
            {
                __debugbreak();
            }

            {
                PROF_SCOPED_RUNTIME_NAME(gfxRenderCommandNames[header->id]);
                RB_RenderCommandTable[header->id](&execState);
            }
            
        }
        else
        {
            execState.cmd = (char *)execState.cmd + *(unsigned __int16 *)execState.cmd;
        }
        if (execState.cmd == prevCmd
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp",
                6029,
                0,
                "%s",
                "execState.cmd != prevCmd"))
        {
            __debugbreak();
        }
        prevCmd = execState.cmd;
        if (gfxCmdBufState.prim.primStats
            && !tess.indexCount
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp",
                6034,
                0,
                "%s",
                "!gfxCmdBufState.prim.primStats || tess.indexCount"))
        {
            __debugbreak();
        }
    }
    if (tess.indexCount)
        RB_EndTessSurface();
    //if (g_DXDeviceThread == GetCurrentThreadId())
    //    D3DPERF_EndEvent();
}

void __cdecl RB_Draw3D()
{
    char *v0; // [esp+8h] [ebp-28h]
    char *Name; // [esp+14h] [ebp-1Ch]
    int v; // [esp+1Ch] [ebp-14h]
    int endView; // [esp+24h] [ebp-Ch]
    const GfxBackEndData *data; // [esp+28h] [ebp-8h]
    const GfxViewInfo *viewInfo; // [esp+2Ch] [ebp-4h]
    const GfxViewInfo *viewInfoa; // [esp+2Ch] [ebp-4h]

    data = backEndData;
    if ( backEndData->viewInfoCount )
    {
        R_WindStartBackend();
        viewInfo = &data->viewInfo[data->viewInfoIndex];
        if ( viewInfo->isMissileCamera )
        {
            endView = data->viewInfoIndex - 1;
            for ( v = data->viewInfoIndex; v >= endView; --v )
            {
                if ( v >= data->viewInfoCount
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp",
                                6072,
                                0,
                                "v doesn't index data->viewInfoCount\n\t%i not in [0, %i)",
                                v,
                                data->viewInfoCount) )
                {
                    __debugbreak();
                }
                viewInfoa = &data->viewInfo[v];
                Name = va("RB_Draw3D c=%d v=%d/%d", viewInfoa->localClientNum, v, data->viewInfoCount);

                {
                    PROF_SCOPED_RUNTIME_NAME(Name);
                    RB_ResetStatTracking(v);
                    RB_Draw3DInternal((GfxViewInfo *)viewInfoa);
                }
            }
        }
        else
        {
            v0 = va("RB_Draw3D c=%d v=%d/%d", viewInfo->localClientNum, data->viewInfoIndex, data->viewInfoCount);
            {
                PROF_SCOPED_RUNTIME_NAME(v0);
                RB_ResetStatTracking(data->viewInfoIndex);
                RB_Draw3DInternal((GfxViewInfo *)viewInfo);
            }
        }
    }
}

void __cdecl RB_CallExecuteRenderCommands()
{
  const char *v0; // eax
  int semaphore; // [esp+18h] [ebp-Ch]
  int hr; // [esp+1Ch] [ebp-8h]

  PROF_SCOPED("RB_CallExecuteRenderCommands");

  if ( !backEndData
    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp", 6163, 0, "%s", "backEndData") )
  {
    __debugbreak();
  }
  if ( (backEndData->drawType & 0x12) != 0 )
  {
    if ( tess.indexCount
      && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp",
            6182,
            0,
            "%s\n\t(tess.indexCount) = %i",
            "(!tess.indexCount)",
            tess.indexCount) )
    {
      __debugbreak();
    }
    if ( backEndData->viewInfoCount )
      RB_Draw3DCommon();
    if ( tess.indexCount
      && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp",
            6190,
            0,
            "%s\n\t(tess.indexCount) = %i",
            "(!tess.indexCount)",
            tess.indexCount) )
    {
      __debugbreak();
    }
    RB_UI3D_RenderToTexture(backEndData->cmds, &backEndData->rbUI3D, &gfxCmdBufInput);
    R_InitCmdBufSourceState(&gfxCmdBufSourceState, &gfxCmdBufInput, 0);
    gfxCmdBufSourceState.input.data = backEndData;
    R_InitLocalCmdBufState(&gfxCmdBufState);
    R_SetRenderTargetSize(&gfxCmdBufSourceState, R_RENDERTARGET_FRAME_BUFFER);
    R_SetRenderTarget(gfxCmdBufContext, R_RENDERTARGET_FRAME_BUFFER);
    RB_InitSceneViewport();
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
    if ( rgp.heatMapImage )
      R_SetCodeImageTexture(&gfxCmdBufSourceState, 0x2Au, rgp.heatMapImage);
    else
      R_SetCodeImageTexture(&gfxCmdBufSourceState, 0x2Au, rgp.whiteImage);
    RB_SetUI3DSamplerAndConstants(&gfxCmdBufSourceState, &backEndData->rbUI3D);
    if ( backEndData->cmds )
    {
        PROF_SCOPED("backEndData->cmds");
        RB_ExecuteRenderCommandsLoop(backEndData->cmds, 0);
    }
    if ( r_drawPrimHistogram->current.enabled )
      RB_DrawPrimHistogramOverlay();
    if ( tess.indexCount )
      RB_EndTessSurface();
    memcpy(gfxCmdBufState.refSamplerState, gfxCmdBufState.refSamplerState, sizeof(gfxCmdBufState));
    if ( gfxCmdBufState.prim.indexBuffer )
      R_ChangeIndices(&gfxCmdBufState.prim, 0);
    R_ClearAllStreamSources(&gfxCmdBufState.prim);
    if ( tess.indexCount
      && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp",
            6331,
            0,
            "%s\n\t(tess.indexCount) = %i",
            "(!tess.indexCount)",
            tess.indexCount) )
    {
      __debugbreak();
    }
    if ( !dx.device
      && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp", 6334, 0, "%s", "dx.device") )
    {
      __debugbreak();
    }
    iassert(dx.inScene);

    if ( r_logFile && r_logFile->current.integer )
      RB_LogPrint("dx.device->EndScene()\n");
    hr = dx.device->EndScene();
    if ( hr < 0 )
    {
      ++g_disableRendering;
      v0 = R_ErrorDescription(hr);
      Com_Error(
        ERR_FATAL,
        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp (%i) dx.device->EndScene() failed: %s\n",
        6336,
        v0);
    }
    dx.inScene = 0;
    if ( !r_glob.isRenderingRemoteUpdate )
    {
      if ( dx.gpuSync )
        R_FinishGpuFence();
      else
        dx.gpuSyncDelay = 0;
    }
  }
  backEndData = 0;
}

void __cdecl RB_UpdateDynamicBuffers(GfxBackEndData *backendData)
{
    unsigned __int8 *v1; // eax
    unsigned int v2; // [esp+0h] [ebp-20h]
    unsigned int v3; // [esp+4h] [ebp-1Ch]
    unsigned int used; // [esp+8h] [ebp-18h]
    unsigned int total; // [esp+Ch] [ebp-14h]
    unsigned __int8 *buffer; // [esp+18h] [ebp-8h]
    unsigned __int8 *buffera; // [esp+18h] [ebp-8h]
    unsigned __int8 *bufferb; // [esp+18h] [ebp-8h]
    unsigned __int8 *bufferc; // [esp+18h] [ebp-8h]

    PROF_SCOPED("RB_UpdateDynamicBuffers");
    buffer = (unsigned __int8 *)R_LockVertexBuffer(
                                                                backendData->skinnedCacheVb->buffer,
                                                                0,
                                                                backendData->skinnedCacheVb->used,
                                                                0x2000u);
    memcpy(buffer, backendData->skinnedCacheVb->verts, backendData->skinnedCacheVb->used);
    R_UnlockVertexBuffer(backendData->skinnedCacheVb->buffer);
    if ( backendData->markMesh.vb.buffer && backendData->markMesh.vb.used )
    {
        if ( backendData->markMesh.vb.used >= backendData->markMesh.vb.total )
            total = backendData->markMesh.vb.total;
        else
            total = backendData->markMesh.vb.used;
        v1 = (unsigned __int8 *)R_LockVertexBuffer(backendData->markMesh.vb.buffer, 0, total, 0x2000u);
        memcpy(v1, backendData->markMesh.vb.verts, total);
        R_UnlockVertexBuffer(backendData->markMesh.vb.buffer);
    }
    if ( backendData->codeMeshPtr->vb.buffer && backendData->codeMeshPtr->vb.used )
    {
        if ( backendData->codeMeshPtr->vb.used >= backendData->codeMeshPtr->vb.total )
            used = backendData->codeMeshPtr->vb.total;
        else
            used = backendData->codeMeshPtr->vb.used;
        buffera = (unsigned __int8 *)R_LockVertexBuffer(backendData->codeMeshPtr->vb.buffer, 0, used, 0x2000u);
        memcpy(buffera, backendData->codeMeshPtr->vb.verts, used);
        R_UnlockVertexBuffer(backendData->codeMeshPtr->vb.buffer);
    }
    if ( backendData->glassMesh.vb.buffer && backendData->glassMesh.vb.used )
    {
        if ( backendData->glassMesh.vb.used >= backendData->glassMesh.vb.total )
            v3 = backendData->glassMesh.vb.total;
        else
            v3 = backendData->glassMesh.vb.used;
        bufferb = (unsigned __int8 *)R_LockVertexBuffer(backendData->glassMesh.vb.buffer, 0, v3, 0x2000u);
        memcpy(bufferb, backendData->glassMesh.vb.verts, v3);
        R_UnlockVertexBuffer(backendData->glassMesh.vb.buffer);
    }
    if ( backendData->preTessIb && backendData->preTessIb->used )
    {
        if ( 2 * backendData->preTessIb->used >= (unsigned int)(2 * backendData->preTessIb->total) )
            v2 = 2 * backendData->preTessIb->total;
        else
            v2 = 2 * backendData->preTessIb->used;
        bufferc = (unsigned __int8 *)R_LockIndexBuffer(backendData->preTessIb->buffer, 0, v2, 0x2000u);
        memcpy(bufferc, (unsigned __int8 *)backendData->preTessIb->indices, v2);
        R_UnlockIndexBuffer(backendData->preTessIb->buffer);
    }
    *backendData->dynamicBufferCurrentFrame = 0;
}

const void *data;
void     RB_RenderThread(unsigned int threadContext)
{
    void *Value; // eax
    int v3; // [esp+0h] [ebp-10h]
    int v4; // [esp+4h] [ebp-Ch]
    int v5; // [esp+8h] [ebp-8h]
    int semaphore; // [esp+Ch] [ebp-4h]

    iassert(threadContext == THREAD_CONTEXT_BACKEND);

    while ( 1 )
    {
        Value = Sys_GetValue(2);
        //if ( !_setjmp3(Value, 0) )
        if ( !_setjmp(*(jmp_buf*)Value) )
            break;
        if ( r_glob.isRenderingRemoteUpdate )
        {
            r_glob.isRenderingRemoteUpdate = 0;
            if ( r_glob.screenUpdateNotify )
            {
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp",
                                6800,
                                0,
                                "%s",
                                "!r_glob.screenUpdateNotify") )
                    __debugbreak();
            }
            r_glob.screenUpdateNotify = 1;
            data = 0;
        }
        else if (data)
        {
            Com_ErrorAbort();
        }
    }
    while ( 1 )
    {
        {
            PROF_SCOPED("R_StreamUpdate_ProcessFileCallbacks"); // LWSS ADD
            R_StreamAlloc_Lock();
            R_StreamUpdate_ProcessFileCallbacks();
            R_StreamAlloc_Unlock();
        }
        {
            PROF_SCOPED("RB_Resource_Update"); // LWSS ADD
            RB_Resource_Update(5);
        }
        Sys_StopRenderer();
        Sys_StartRenderer();

        if ( Sys_WaitBackendEvent(1) )
        {
            data = (GfxBackEndData *)Sys_RendererSleep();
            if (data)
            {
                RB_UpdateDynamicBuffers((GfxBackEndData*)data);
                RB_RenderCommandFrame((GfxBackEndData *)data);
            }
            data = 0;
        }
        else if ( Sys_QueryRGRegisteredEvent() )
        {
            R_BeginRegistrationInternal();
            Sys_SetD3DDeviceStartEvent();
            Sys_SetD3DDeviceOKEvent();
            Sys_ClearRGRegisteredEvent();
        }
        else if ( Sys_QueryD3DShutdownEvent() )
        {
            R_ShutdownInternal();
            Sys_ClearD3DShutdownEvent();
            Sys_ClearD3DDeviceStartEvent();
        }
        else if ( dx.resizeWindow )
        {
            RB_SwapBuffers();
        }
        else
        {
            NET_Sleep(0);
        }
        if ( r_glob.remoteScreenUpdateNesting )
        {
            if ( r_glob.screenUpdateNotify
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp",
                            6919,
                            0,
                            "%s",
                            "!r_glob.screenUpdateNotify") )
            {
                __debugbreak();
            }
            r_glob.screenUpdateNotify = 1;
            if ( r_glob.isRenderingRemoteUpdate
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp",
                            6922,
                            0,
                            "%s",
                            "!r_glob.isRenderingRemoteUpdate") )
            {
                __debugbreak();
            }
            r_glob.isRenderingRemoteUpdate = 1;
            do
            {
                R_StreamAlloc_Lock();
                R_StreamUpdate_ProcessFileCallbacks();
                R_StreamAlloc_Unlock();
                RB_Resource_Update(8);
                if ( Sys_TryEnterCriticalSection(CRITSECT_DBHASH) )
                {
                    SCR_UpdateScreen();
                    Sys_LeaveCriticalSection(CRITSECT_DBHASH);
                    if ( Sys_IsDatabaseReady() )
                        NET_Sleep(5u);
                }
                NET_Sleep(1u);
            }
            while ( r_glob.remoteScreenUpdateNesting );
            if ( !r_glob.isRenderingRemoteUpdate
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp",
                            6955,
                            0,
                            "%s",
                            "r_glob.isRenderingRemoteUpdate") )
            {
                __debugbreak();
            }
            r_glob.isRenderingRemoteUpdate = 0;
            if ( r_glob.screenUpdateNotify
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp",
                            6958,
                            0,
                            "%s",
                            "!r_glob.screenUpdateNotify") )
            {
                __debugbreak();
            }
            r_glob.screenUpdateNotify = 1;
        }
    }
}

void __cdecl RB_RenderCommandFrame(const GfxBackEndData *data)
{
    char *Name; // [esp+18h] [ebp-28h]
    int semaphore; // [esp+1Ch] [ebp-24h]
    unsigned int viewInfoIdx; // [esp+24h] [ebp-1Ch]
    int clientNum; // [esp+30h] [ebp-10h]
    unsigned int drawType; // [esp+34h] [ebp-Ch]
    int renderStartMS; // [esp+3Ch] [ebp-4h]

    renderStartMS = Sys_Milliseconds();
    viewInfoIdx = -1;
    clientNum = -1;
    if (data->viewInfoCount)
    {
        viewInfoIdx = data->viewInfoIndex;
        clientNum = data->viewInfo[viewInfoIdx].localClientNum;
    }
    Name = va("exec cmds c=%d v=%d", clientNum, viewInfoIdx);

    {
        PROF_SCOPED_RUNTIME_NAME(Name);
        RB_BeginFrame((GfxBackEndData *)data);
        RB_DrawComposites();
        RB_Draw3D();
        drawType = backEndData->drawType;
        RB_CallExecuteRenderCommands();
        rb_execCmdsMS = Sys_Milliseconds() - renderStartMS;
        backEndData = 0;
    }

    {
        PROF_SCOPED("Sys_RenderCompleted()");
        Sys_RenderCompleted();
    }

    {
        PROF_SCOPED("end frame");
        RB_EndFrame(drawType);
    }
    
    rb_swapMS = Sys_Milliseconds() - renderStartMS - rb_execCmdsMS;
}

void __cdecl RB_InitBackendGlobalStructs()
{
    memset(&backEnd, 0, sizeof(backEnd));
    RB_InitSceneViewport();
    RB_InitCodeImages();
}

void __cdecl RB_SetBspImages()
{
    iassert(!rgp.world->skyImage || (rgp.world->skySamplerState & SAMPLER_FILTER_MASK));

    gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_OUTDOOR] = rgp.world->outdoorImage;
    gfxCmdBufInput.codeImages[TEXTURE_SRC_CODE_SKY] = rgp.world->skyImage;
    gfxCmdBufInput.codeImageSamplerStates[TEXTURE_SRC_CODE_SKY] = rgp.world->skySamplerState;
}

void __cdecl RB_InitCodeImages()
{
    RB_InitSingleCodeImage(TEXTURE_SRC_CODE_BLACK,                    rgp.blackImage,                                           1,                                   "TEXTURE_SRC_CODE_BLACK");
    RB_InitSingleCodeImage(TEXTURE_SRC_CODE_WHITE,                    rgp.whiteImage,                                           1,                                   "TEXTURE_SRC_CODE_WHITE");
    RB_InitSingleCodeImage(TEXTURE_SRC_CODE_IDENTITY_NORMAL_MAP,      rgp.identityNormalMapImage,                               1,                                   "TEXTURE_SRC_CODE_IDENTITY_NORMAL_MAP");
    RB_InitSingleCodeImage(TEXTURE_SRC_CODE_MODEL_LIGHTING,           0,                                                        0xE2,                                "TEXTURE_SRC_CODE_MODEL_LIGHTING");
    RB_InitSingleCodeImage(TEXTURE_SRC_CODE_SHADOWMAP_SUN,            gfxRenderTargets[R_RENDERTARGET_SHADOWMAP_SUN].image,     gfxMetrics.shadowmapSamplerState,    "shadowmapSamplerSun");
    RB_InitSingleCodeImage(TEXTURE_SRC_CODE_SHADOWMAP_SPOT,           0,                                                        gfxMetrics.shadowmapSamplerState,    "shadowmapSamplerSpot");
    RB_InitSingleCodeImage(TEXTURE_SRC_CODE_FEEDBACK,                 0,                                                        0xE2,                                "feedbackSampler");
    RB_InitSingleCodeImage(TEXTURE_SRC_CODE_RESOLVED_POST_SUN,        0,                                                        98,                                  "TEXTURE_SRC_CODE_RESOLVED_POST_SUN");
    RB_InitSingleCodeImage(TEXTURE_SRC_CODE_RESOLVED_SCENE,           gfxRenderTargets[R_RENDERTARGET_RESOLVED_SCENE].image,    98,                                  "resolvedScene");
    RB_InitSingleCodeImage(TEXTURE_SRC_CODE_POST_EFFECT_SRC,          gfxRenderTargets[R_RENDERTARGET_POST_EFFECT_SRC].image,   98,                                  "postEffectSrc");
    RB_InitSingleCodeImage(TEXTURE_SRC_CODE_POST_EFFECT_GODRAYS,      gfxRenderTargets[R_RENDERTARGET_POST_EFFECT_GODRAYS].image, 98,                                "postEffectGodRays");
    RB_InitSingleCodeImage(TEXTURE_SRC_CODE_POST_EFFECT_0,            gfxRenderTargets[R_RENDERTARGET_POST_EFFECT_0].image,     98,                                  "postEffect0");
    RB_InitSingleCodeImage(TEXTURE_SRC_CODE_POST_EFFECT_1,            gfxRenderTargets[R_RENDERTARGET_POST_EFFECT_1].image,     98,                                  "postEffect1");
    RB_InitSingleCodeImage(TEXTURE_SRC_CODE_SKY,                      0,                                                        0,                                   "sampler.sky");
    RB_InitSingleCodeImage(TEXTURE_SRC_CODE_LIGHT_ATTENUATION,        0,                                                        0,                                   "attenuationSampler");
    RB_InitSingleCodeImage(TEXTURE_SRC_CODE_DLIGHT_ATTENUATION,       0,                                                        0,                                   "dlightAttenuationSampler");
    RB_InitSingleCodeImage(TEXTURE_SRC_CODE_OUTDOOR,                  0,                                                        98,                                  "TEXTURE_SRC_CODE_OUTDOOR");
    RB_InitSingleCodeImage(TEXTURE_SRC_CODE_FLOATZ,                   gfxRenderTargets[R_RENDERTARGET_FLOAT_Z].image,           97,                                  "floatz");
    RB_InitSingleCodeImage(TEXTURE_SRC_CODE_CINEMATIC_Y,              0,                                                        98,                                  "cinematicY");
    RB_InitSingleCodeImage(TEXTURE_SRC_CODE_CINEMATIC_CR,             0,                                                        98,                                  "cinematicCr");
    RB_InitSingleCodeImage(TEXTURE_SRC_CODE_CINEMATIC_CB,             0,                                                        98,                                  "cinematicCb");
    RB_InitSingleCodeImage(TEXTURE_SRC_CODE_CINEMATIC_A,              0,                                                        98,                                  "cinematicA");
    RB_InitSingleCodeImage(TEXTURE_SRC_CODE_TERRAIN_SCORCH_TEXTURE_0, 0,                                                        10,                                  "terrainScorchTexture0");
    RB_InitSingleCodeImage(TEXTURE_SRC_CODE_TERRAIN_SCORCH_TEXTURE_1, 0,                                                        10,                                  "terrainScorchTexture1");
    RB_InitSingleCodeImage(TEXTURE_SRC_CODE_TERRAIN_SCORCH_TEXTURE_2, 0,                                                        10,                                  "terrainScorchTexture2");
    RB_InitSingleCodeImage(TEXTURE_SRC_CODE_TERRAIN_SCORCH_TEXTURE_3, 0,                                                        10,                                  "terrainScorchTexture3");
    RB_InitSingleCodeImage(TEXTURE_SRC_CODE_TEXTURE_0,                0,                                                        0x62u,                               "TEXTURE_SRC_CODE_TEXTURE_0");
    RB_InitSingleCodeImage(TEXTURE_SRC_CODE_TEXTURE_1,                0,                                                        0x62u,                               "TEXTURE_SRC_CODE_TEXTURE_1");
    RB_InitSingleCodeImage(TEXTURE_SRC_CODE_TEXTURE_2,                0,                                                        0x62u,                               "TEXTURE_SRC_CODE_TEXTURE_2");
    RB_InitSingleCodeImage(TEXTURE_SRC_CODE_TEXTURE_3,                0,                                                        0x62u,                               "TEXTURE_SRC_CODE_TEXTURE_3");
    RB_InitSingleCodeImage(TEXTURE_SRC_CODE_IMPACT_MASK,              rgp.whiteImage,                                           0x61u,                               "impactMaskSampler");
    RB_InitSingleCodeImage(TEXTURE_SRC_CODE_UI3D,                     rgp.whiteImage,                                           0x72u,                               "ui3dSampler");
    RB_InitSingleCodeImage(TEXTURE_SRC_CODE_MISSILE_CAM,              rgp.whiteImage,                                           0x72u,                               "missileCamSampler");
    RB_InitSingleCodeImage(TEXTURE_SRC_CODE_HEATMAP,                  rgp.whiteImage,                                           0x72u,                               "heatMapSampler");

    for (int i = 0; i < TEXTURE_SRC_CODE_COUNT; ++i)
        gfxCmdBufInput.codeImageRenderTargetControl[i].packed = 0;
}

void __cdecl RB_InitSingleCodeImage(
                unsigned int codeTexture,
                const GfxImage *image,
                unsigned __int8 samplerState,
                const char *name)
{
    gfxCmdBufInput.codeImages[codeTexture] = image;
    gfxCmdBufInput.codeImageSamplerStates[codeTexture] = samplerState;
    rg.codeImageNames[codeTexture] = name;
}

void __cdecl RB_RegisterBackendAssets()
{
    backEnd.debugFont = R_RegisterFont("fonts/smalldevfont", 1);
}

//void __cdecl RB_SaveScreen_BlendBlurred(const GfxBlendSaveScreenBlurredParam *p, const GfxViewInfo *viewInfo)
//{
//    float v2; // xmm0_4
//    long double v3; // [esp+28h] [ebp-54h]
//    long double image; // [esp+30h] [ebp-4Ch]
//    float t0; // [esp+54h] [ebp-28h]
//    float s1; // [esp+5Ch] [ebp-20h]
//    const SavedScreenParams *ssParams; // [esp+60h] [ebp-1Ch]
//    float s0; // [esp+64h] [ebp-18h]
//    int frameTime; // [esp+68h] [ebp-14h]
//    float scrnH; // [esp+6Ch] [ebp-10h]
//    float scrnW; // [esp+70h] [ebp-Ch]
//    float alpha; // [esp+78h] [ebp-4h]
//
//    if (p->enabled)
//    {
//        //PIXBeginNamedEvent(-1, "RB_SaveScreen_BlendBlurred");
//        if (tess.indexCount)
//            RB_EndTessSurface();
//        R_Set2D(&gfxCmdBufSourceState);
//        if (p->fadeMsec <= 0
//            && !Assert_MyHandler(
//                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp",
//                7247,
//                0,
//                "%s",
//                "p->fadeMsec > 0"))
//        {
//            __debugbreak();
//        }
//        if (p->screenTimerId >= 4u
//            && !Assert_MyHandler(
//                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp",
//                7248,
//                0,
//                "p->screenTimerId doesn't index ARRAY_COUNT( rgp.savedScreenTimes )\n\t%i not in [0, %i)",
//                p->screenTimerId,
//                4))
//        {
//            __debugbreak();
//        }
//        frameTime = gfxCmdBufSourceState.sceneDef.time - rgp.savedScreenTimes[p->screenTimerId];
//        if (frameTime >= 0 && frameTime < p->fadeMsec)
//        {
//            __libm_sse2_pow(v3, image);
//            alpha = 0.009999999776482582;
//            if ((float)0.009999999776482582 > 0.99000001)
//                alpha = 0.99000001;
//            if ((viewInfo->sceneComposition.renderingMode & 7) != 0)
//            {
//                scrnW = (float)gfxRenderTargets[viewInfo->sceneComposition.mainSceneFinal].width;
//                scrnH = (float)gfxRenderTargets[viewInfo->sceneComposition.mainSceneFinal].height;
//            }
//            else
//            {
//                scrnW = (float)gfxCmdBufSourceState.renderTargetWidth * p->ds;
//                scrnH = (float)gfxCmdBufSourceState.renderTargetHeight * p->dt;
//            }
//            ssParams = &rgp.savedScreenParams[p->screenTimerId];
//            if (ssParams->isSet)
//            {
//                s0 = ssParams->s0;
//                s1 = ssParams->s0 + ssParams->ds;
//                t0 = ssParams->t0;
//                v2 = t0 + ssParams->dt;
//            }
//            else
//            {
//                s0 = p->s0;
//                s1 = s0 + p->ds;
//                t0 = p->t0;
//                v2 = t0 + p->dt;
//            }
//            if ((viewInfo->sceneComposition.renderingMode & 7) != 0)
//                gfxCmdBufSourceState.input.codeImageRenderTargetControl[8].packed = (viewInfo->sceneComposition.mainSceneSaved << 24)
//                | 0x800062;
//            else
//                R_SetCodeImageTexture(&gfxCmdBufSourceState, 8u, gfxRenderTargets[R_RENDERTARGET_SAVED_SCREEN].image);
//            RB_DrawStretchPic(
//                rgp.shellShockBlurredMaterial,
//                0.0,
//                0.0,
//                scrnW,
//                scrnH,
//                s0,
//                t0,
//                s1,
//                v2,
//                ((unsigned __int8)(int)((float)(255.0 * alpha) + 9.313225746154785e-10) << 24) | 0xFFFFFF,
//                GFX_PRIM_STATS_CODE);
//            if (tess.indexCount)
//                RB_EndTessSurface();
//        }
//        //if (g_DXDeviceThread == GetCurrentThreadId())
//        //    D3DPERF_EndEvent();
//    }
//}

// aislop (pow)
void __cdecl RB_SaveScreen_BlendBlurred(
    const GfxBlendSaveScreenBlurredParam *p,
    const GfxViewInfo *viewInfo)
{
    if (!p->enabled)
        return;

    if (tess.indexCount)
        RB_EndTessSurface();

    R_Set2D(&gfxCmdBufSourceState);

    // fadeMsec > 0
    if (p->fadeMsec <= 0 &&
        !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp",
            7247,
            0,
            "%s",
            "p->fadeMsec > 0"))
    {
        __debugbreak();
    }

    // screenTimerId bounds
    if (p->screenTimerId >= 4 &&
        !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp",
            7248,
            0,
            "p->screenTimerId doesn't index ARRAY_COUNT( rgp.savedScreenTimes )\n\t%i not in [0, %i)",
            p->screenTimerId,
            4))
    {
        __debugbreak();
    }

    int frameTime =
        gfxCmdBufSourceState.sceneDef.time -
        rgp.savedScreenTimes[p->screenTimerId];

    if (frameTime < 0 || frameTime >= p->fadeMsec)
        return;

    /* -----------------------------------------
       Alpha computation (SSE pow path)
       ----------------------------------------- */

    float t = (float)frameTime / (float)p->fadeMsec;

    // pow(t, 0.01f)
    float alpha = powf(t, 0.01f);

    if (alpha > 0.99f)
        alpha = 0.99f;

    /* -----------------------------------------
       Screen size
       ----------------------------------------- */

    float scrnW, scrnH;

    if (viewInfo->sceneComposition.renderingMode & 7)
    {
        const GfxRenderTarget *rt =
            &gfxRenderTargets[viewInfo->sceneComposition.mainSceneFinal];

        scrnW = (float)rt->width;
        scrnH = (float)rt->height;
    }
    else
    {
        scrnW = (float)gfxCmdBufSourceState.renderTargetWidth * p->ds;
        scrnH = (float)gfxCmdBufSourceState.renderTargetHeight * p->dt;
    }

    /* -----------------------------------------
       Texture coordinates
       ----------------------------------------- */

    float s0, s1, t0, t1;

    const SavedScreenParams *ssParams =
        &rgp.savedScreenParams[p->screenTimerId];

    if (ssParams->isSet)
    {
        s0 = ssParams->s0;
        s1 = ssParams->s0 + ssParams->ds;
        t0 = ssParams->t0;
        t1 = ssParams->t0 + ssParams->dt;
    }
    else
    {
        s0 = p->s0;
        s1 = p->s0 + p->ds;
        t0 = p->t0;
        t1 = p->t0 + p->dt;
    }

    /* -----------------------------------------
       Bind blurred screen texture
       ----------------------------------------- */

    if (viewInfo->sceneComposition.renderingMode & 7)
    {
        gfxCmdBufSourceState.input
            .codeImageRenderTargetControl[8]
            .packed =
            (viewInfo->sceneComposition.mainSceneSaved << 24) |
            0x00800062;
    }
    else
    {
        R_SetCodeImageTexture(
            &gfxCmdBufSourceState,
            8,
            gfxRenderTargets[R_RENDERTARGET_SAVED_SCREEN].image);
    }

    /* -----------------------------------------
       Color packing (matches FPU rounding)
       ----------------------------------------- */

    int a = (int)(alpha * 255.0f + 0.5f);
    if (a < 0)   a = 0;
    if (a > 255) a = 255;

    unsigned int color =
        (a << 24) | 0x00FFFFFF;

    /* -----------------------------------------
       Draw
       ----------------------------------------- */

    RB_DrawStretchPic(
        rgp.shellShockBlurredMaterial,
        0.0f,
        0.0f,
        scrnW,
        scrnH,
        s0,
        t0,
        s1,
        t1,
        color,
        GFX_PRIM_STATS_CODE);

    if (tess.indexCount)
        RB_EndTessSurface();

    return;
}


void __cdecl RB_SaveScreen_BlendFlashed(const GfxBlendSaveScreenFlashedParam *p, const GfxViewInfo *viewInfo)
{
    float v2; // xmm0_4
    float screenWidth; // [esp+64h] [ebp-24h]
    float t0; // [esp+68h] [ebp-20h]
    float s1; // [esp+70h] [ebp-18h]
    unsigned __int8 whiteout; // [esp+76h] [ebp-12h]
    const SavedScreenParams *ssParams; // [esp+78h] [ebp-10h]
    float s0; // [esp+7Ch] [ebp-Ch]
    unsigned int color; // [esp+80h] [ebp-8h]
    float screenHeight; // [esp+84h] [ebp-4h]

    if (p->enabled)
    {
        PROF_SCOPED("RB_SaveScreen_BlendFlashed");
        if (tess.indexCount)
            RB_EndTessSurface();
        R_Set2D(&gfxCmdBufSourceState);
        whiteout = (int)((float)(255.0 * p->intensityWhiteout) + 9.313225746154785e-10);
        color = ((unsigned __int8)(int)((float)(255.0 * p->intensityScreengrab) + 9.313225746154785e-10) << 24)
            | whiteout
            | (whiteout << 8)
            | (whiteout << 16);
        if ((viewInfo->sceneComposition.renderingMode & 7) != 0)
        {
            screenWidth = (float)gfxRenderTargets[viewInfo->sceneComposition.mainSceneFinal].width;
            screenHeight = (float)gfxRenderTargets[viewInfo->sceneComposition.mainSceneFinal].height;
        }
        else
        {
            screenWidth = (float)gfxCmdBufSourceState.renderTargetWidth * p->ds;
            screenHeight = (float)gfxCmdBufSourceState.renderTargetHeight * p->dt;
        }
        ssParams = &rgp.savedScreenParams[p->screenTimerId];
        if (ssParams->isSet)
        {
            s0 = ssParams->s0;
            s1 = ssParams->s0 + ssParams->ds;
            t0 = ssParams->t0;
            v2 = t0 + ssParams->dt;
        }
        else
        {
            s0 = p->s0;
            s1 = s0 + p->ds;
            t0 = p->t0;
            v2 = t0 + p->dt;
        }
        if ((viewInfo->sceneComposition.renderingMode & 7) != 0)
            gfxCmdBufSourceState.input.codeImageRenderTargetControl[8].packed = (viewInfo->sceneComposition.mainSceneSaved << 24)
            | 0x800062;
        else
            R_SetCodeImageTexture(&gfxCmdBufSourceState, 8u, gfxRenderTargets[R_RENDERTARGET_SAVED_SCREEN].image);
        RB_DrawStretchPic(
            rgp.shellShockFlashedMaterial,
            0.0,
            0.0,
            screenWidth,
            screenHeight,
            s0,
            t0,
            s1,
            v2,
            color,
            GFX_PRIM_STATS_CODE);
        if (tess.indexCount)
            RB_EndTessSurface();
    }
}

void __cdecl RB_SaveScreen(const GfxSaveScreenParam *p, const GfxViewInfo *viewInfo)
{
    float t0; // [esp+34h] [ebp-34h]
    int clientId; // [esp+38h] [ebp-30h]
    int x0; // [esp+3Ch] [ebp-2Ch]
    float t1; // [esp+40h] [ebp-28h]
    float s1; // [esp+44h] [ebp-24h]
    int halfWidth; // [esp+48h] [ebp-20h]
    int renderTargetWidth; // [esp+4Ch] [ebp-1Ch]
    int renderTargetHeight; // [esp+50h] [ebp-18h]
    float s0; // [esp+58h] [ebp-10h]
    int y0; // [esp+5Ch] [ebp-Ch]
    int halfHeight; // [esp+60h] [ebp-8h]
    SavedScreenParams *ssParams; // [esp+64h] [ebp-4h]

    if (p->mode)
    {
        PROF_SCOPED("RB_SaveScreen");
        if (p->screenTimerId >= 4u
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp",
                7424,
                0,
                "p->screenTimerId doesn't index ARRAY_COUNT( rgp.savedScreenTimes )\n\t%i not in [0, %i)",
                p->screenTimerId,
                4))
        {
            __debugbreak();
        }
        ssParams = &rgp.savedScreenParams[p->screenTimerId];
        if ((viewInfo->sceneComposition.renderingMode & 7) != 0)
        {
            ssParams->s0 = 0.0f;
            ssParams->t0 = 0.0f;
            ssParams->ds = 1.0f;
            ssParams->dt = 1.0f;
            rgp.savedScreenTimes[p->screenTimerId] = gfxCmdBufSourceState.sceneDef.time;
            ssParams->isSet = 1;
        }
        else if (p->mode == 1)
        {
            R_Resolve(gfxCmdBufContext, gfxRenderTargets[R_RENDERTARGET_SAVED_SCREEN].image);
            rgp.savedScreenTimes[p->screenTimerId] = gfxCmdBufSourceState.sceneDef.time;
            ssParams->s0 = 0.0f;
            ssParams->t0 = 0.0f;
            ssParams->ds = 1.0f;
            ssParams->dt = 1.0f;
            ssParams->isSet = 1;
        }
        else if (p->mode == 2)
        {
            R_ResolveSection(gfxCmdBufContext, gfxRenderTargets[R_RENDERTARGET_RESOLVED_SCENE].image);
            rgp.savedScreenTimes[p->screenTimerId] = gfxCmdBufSourceState.sceneDef.time;
            R_SetRenderTargetSize(&gfxCmdBufSourceState, R_RENDERTARGET_SAVED_SCREEN);
            R_SetRenderTarget(gfxCmdBufContext, R_RENDERTARGET_SAVED_SCREEN);
            R_Set2D(&gfxCmdBufSourceState);
            renderTargetWidth = gfxCmdBufSourceState.renderTargetWidth;
            renderTargetHeight = gfxCmdBufSourceState.renderTargetHeight;
            s0 = p->s0;
            s1 = p->s0 + p->ds;
            t0 = p->t0;
            t1 = t0 + p->dt;
            halfWidth = gfxCmdBufSourceState.renderTargetWidth / 2;
            halfHeight = gfxCmdBufSourceState.renderTargetHeight / 2;
            clientId = p->screenTimerId;
            x0 = gfxCmdBufSourceState.renderTargetWidth / 2 * (clientId % 2);
            y0 = gfxCmdBufSourceState.renderTargetHeight / 2 * (clientId / 2);
            R_SetCodeImageTexture(&gfxCmdBufSourceState, 8u, gfxRenderTargets[R_RENDERTARGET_RESOLVED_SCENE].image);
            RB_DrawStretchPic(
                rgp.feedbackReplaceMaterial,
                (float)x0,
                (float)y0,
                (float)halfWidth,
                (float)halfHeight,
                s0,
                t0,
                s1,
                t1,
                0xFFFFFFFF,
                GFX_PRIM_STATS_CODE);
            if (tess.indexCount)
                RB_EndTessSurface();
            ssParams->s0 = (float)x0 / (float)renderTargetWidth;
            ssParams->t0 = (float)y0 / (float)renderTargetHeight;
            ssParams->ds = (float)halfWidth / (float)renderTargetWidth;
            ssParams->dt = (float)halfHeight / (float)renderTargetHeight;
            ssParams->isSet = 1;
            R_SetRenderTargetSize(&gfxCmdBufSourceState, R_RENDERTARGET_SCENE);
            R_SetRenderTarget(gfxCmdBufContext, R_RENDERTARGET_SCENE);
        }
    }
}

void __cdecl R_ResolveSection(GfxCmdBufContext context, GfxImage *image)
{
    iassert(image);

    iassert(0);
    //if ( !Assert_MyHandler(
    //                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_backend.cpp",
    //                1415,
    //                0,
    //                "R_ResolveSection(): Not implemented on win32.") )
    //    __debugbreak();
}

