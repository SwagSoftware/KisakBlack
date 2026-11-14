#include "rb_sky.h"
#include "rb_shade.h"
#include "r_dvars.h"
#include "rb_logfile.h"
#include "r_state.h"
#include "r_foliage.h"
#include "r_state_utils.h"
#include "rb_stats.h"
#include "rb_corona.h"
#include "rb_pixelcost.h"
#include <qcommon/common.h>

SunFlareDynamic sunFlareArray[4];

unsigned int __cdecl RB_CalcSunSpriteSamples()
{
  const char *v1; // eax
  const char *v2; // eax
  const char *v3; // eax
  const char *v4; // eax
  int v6; // [esp+3Ch] [ebp-28h]
  int v8; // [esp+44h] [ebp-20h]
  int v10; // [esp+4Ch] [ebp-18h]
  int v12; // [esp+54h] [ebp-10h]
  IDirect3DQuery9 *occlusionQuery; // [esp+58h] [ebp-Ch]
  HRESULT hr; // [esp+5Ch] [ebp-8h]
  unsigned int sampleCount; // [esp+60h] [ebp-4h] BYREF

  if ( vidConfig.displayWidth < 0x10
    && !Assert_MyHandler(
          "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_sky.cpp",
          48,
          0,
          "%s\n\t(vidConfig.displayWidth) = %i",
          "(vidConfig.displayWidth >= 16)",
          vidConfig.displayWidth) )
  {
    __debugbreak();
  }
  if ( vidConfig.displayHeight < 0x10
    && !Assert_MyHandler(
          "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_sky.cpp",
          49,
          0,
          "%s\n\t(vidConfig.displayHeight) = %i",
          "(vidConfig.displayHeight >= 16)",
          vidConfig.displayHeight) )
  {
    __debugbreak();
  }
  occlusionQuery = sunFlareArray[0].sunQuery[0];
  if ( !sunFlareArray[0].sunQuery[0] )
    return 0;
  RB_ResetStatTracking(0);
  if ( tess.indexCount )
    RB_EndTessSurface();
  if ( r_logFile && r_logFile->current.integer )
    RB_LogPrint("dx.device->BeginScene()\n");
  v12 = dx.device->BeginScene();
  if ( v12 < 0 )
  {
    ++g_disableRendering;
    v1 = R_ErrorDescription(v12);
    Com_Error(
      ERR_FATAL,
      "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_sky.cpp (%i) dx.device->BeginScene() failed: %s\n",
      61,
      v1);
  }
  R_ClearScreenInternal(gfxCmdBufState.prim.device, 2u, colorWhite, 1.0, 0, 0);
  R_SetCodeConstantFromVec4(&gfxCmdBufSourceState, CONST_SRC_CODE_MATERIAL_COLOR, (float*)colorWhite);
  if ( r_logFile && r_logFile->current.integer )
    RB_LogPrint("occlusionQuery->Issue( (1 << 1) )\n");
  v10 = occlusionQuery->Issue(2);
  if ( v10 < 0 )
  {
    ++g_disableRendering;
    v2 = R_ErrorDescription(v10);
    Com_Error(
      ERR_FATAL,
      "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_sky.cpp (%i) occlusionQuery->Issue( (1 << 1) ) failed: %s\n",
      65,
      v2);
  }
  R_Set2D(&gfxCmdBufSourceState);
  RB_DrawStretchPic(rgp.additiveMaterial, 0.0, 0.0, 16.0, 16.0, 0.0, 0.0, 1.0, 1.0, 0xFFFFFFFF, GFX_PRIM_STATS_CODE);
  if ( gfxCmdBufSourceState.viewMode != VIEW_MODE_2D
    && !Assert_MyHandler(
          "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_sky.cpp",
          68,
          1,
          "%s",
          "gfxCmdBufSourceState.viewMode == VIEW_MODE_2D") )
  {
    __debugbreak();
  }
  RB_EndTessSurface();
  if ( r_logFile && r_logFile->current.integer )
    RB_LogPrint("occlusionQuery->Issue( (1 << 0) )\n");
  //v8 = ((int (__thiscall *)(IDirect3DQuery9 *, IDirect3DQuery9 *, int))occlusionQuery->Issue)(
  //       occlusionQuery,
  //       occlusionQuery,
  //       1);
  v8 = occlusionQuery->Issue(1);
  if ( v8 < 0 )
  {
    ++g_disableRendering;
    v3 = R_ErrorDescription(v8);
    Com_Error(
      ERR_FATAL,
      "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_sky.cpp (%i) occlusionQuery->Issue( (1 << 0) ) failed: %s\n",
      70,
      v3);
  }
  if ( r_logFile && r_logFile->current.integer )
    RB_LogPrint("dx.device->EndScene()\n");
  v6 = dx.device->EndScene();
  if ( v6 < 0 )
  {
    ++g_disableRendering;
    v4 = R_ErrorDescription(v6);
    Com_Error(
      ERR_FATAL,
      "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_sky.cpp (%i) dx.device->EndScene() failed: %s\n",
      72,
      v4);
  }
  while ( 1 )
  {
    //hr = ((int (__thiscall *)(IDirect3DQuery9 *, IDirect3DQuery9 *, unsigned int *, int, int))occlusionQuery->GetData)(
    //       occlusionQuery,
    //       occlusionQuery,
    //       &sampleCount,
    //       4,
    //       1);
      hr = occlusionQuery->GetData(&sampleCount, 4, 1);
    if ( hr != 1 )
      break;
    Sleep(0);
  }
  if ( hr )
    return 256;
  return sampleCount;
}

void __cdecl RB_DrawSun(unsigned int localClientNum)
{
    SunFlareDynamic *sunFlare; // [esp+18h] [ebp-8h]

    PROF_SCOPED("RB_DrawSun");

    if ( !rgp.world
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_sky.cpp", 618, 0, "%s", "rgp.world") )
    {
        __debugbreak();
    }
    if ( localClientNum >= gfxCfg.maxClientViews
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_sky.cpp",
                    619,
                    0,
                    "localClientNum doesn't index gfxCfg.maxClientViews\n\t%i not in [0, %i)",
                    localClientNum,
                    gfxCfg.maxClientViews) )
    {
        __debugbreak();
    }
    if ( !r_drawSun->current.enabled )
    {
        return;
    }
    if ( !rgp.world->sun.hasValidData )
    {
        return;
    }
    sunFlare = &sunFlareArray[localClientNum];
    RB_SetIdentity();
    RB_DrawSunQuerySprite(sunFlare);
    RB_AddSunEffects(sunFlare);

    if ( tess.indexCount )
        RB_EndTessSurface();
}

void __cdecl RB_DrawSunQuerySprite(SunFlareDynamic *sunFlare)
{
    float v2; // [esp+10h] [ebp-4Ch]
    unsigned int sunSpriteSamples; // [esp+44h] [ebp-18h]
    unsigned int drawnSampleCount; // [esp+48h] [ebp-14h]
    float lastVisibilitya; // [esp+54h] [ebp-8h]
    float lastVisibility; // [esp+54h] [ebp-8h]
    int queryIndex; // [esp+58h] [ebp-4h]

    PROF_SCOPED("RB_DrawSunQuerySprite");

    if (!sunFlare
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_sky.cpp", 322, 0, "%s", "sunFlare"))
    {
        __debugbreak();
    }
    queryIndex = r_glob.backEndFrameCount % 2;
    if ((unsigned int)(r_glob.backEndFrameCount % 2) >= 2
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_sky.cpp",
            325,
            0,
            "queryIndex doesn't index ARRAY_COUNT( sunFlare->sunQuery )\n\t%i not in [0, %i)",
            r_glob.backEndFrameCount % 2,
            2))
    {
        __debugbreak();
    }
    if (!sunFlare->sunQuery[queryIndex])
    {
        RB_UpdateSunVisibilityWithoutQuery(sunFlare);
        return;
    }
    RB_SetTessTechnique(rgp.additiveMaterial, 4u);
    R_TrackPrims(&gfxCmdBufState.prim, GFX_PRIM_STATS_FX);
    RB_SetIdentity();
    RB_TessSunBillboard(
        16.0 / (float)gfxCmdBufSourceState.renderTargetWidth,
        16.0 / (float)gfxCmdBufSourceState.renderTargetHeight,
        (GfxColor)-16777216);
    if (sunFlare->sunQueryIssued[queryIndex])
    {
        drawnSampleCount = RB_HW_ReadOcclusionQuery(sunFlare->sunQuery[queryIndex]);
        if (drawnSampleCount == -1)
            sunFlare->error = 1;
        sunSpriteSamples = dx.sunSpriteSamples;
        if (!dx.sunSpriteSamples
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_sky.cpp",
                362,
                0,
                "%s\n\t(sunSpriteSamples) = %i",
                "(sunSpriteSamples > 0)",
                0))
        {
            __debugbreak();
        }
        if (drawnSampleCount > sunSpriteSamples)
            sunFlare->error = 1;
        lastVisibilitya = (double)drawnSampleCount / (double)sunSpriteSamples;
        if ((float)(1.0 - lastVisibilitya) < 0.0)
            v2 = 1.0f;
        else
            v2 = (double)drawnSampleCount / (double)sunSpriteSamples;
        lastVisibility = v2;
    }
    else
    {
        lastVisibility = 0.0f;
    }
    if (!sunFlare->error)
        sunFlare->lastVisibility = lastVisibility;
    sunFlare->error = 0;
    if (pixelCostMode == GFX_PIXEL_COST_MODE_MEASURE_COST || pixelCostMode == GFX_PIXEL_COST_MODE_MEASURE_MSEC)
    {
        RB_EndTessSurface();
    }
    else
    {
        RB_HW_BeginOcclusionQuery(sunFlare->sunQuery[queryIndex]);
        RB_EndTessSurface();
        sunFlare->sunQuery[queryIndex]->Issue(1);
        sunFlare->sunQueryIssued[queryIndex] = 1;
    }
}

GfxVertex *__cdecl RB_SetTessQuad_0(GfxColor color)
{
    unsigned __int16 vertCount; // [esp+10h] [ebp-8h]
    GfxVertex *vert; // [esp+14h] [ebp-4h]

    if (tess.vertexCount
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_sky.cpp",
            156,
            0,
            "%s",
            "tess.vertexCount == 0"))
    {
        __debugbreak();
    }
    if (tess.indexCount
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_sky.cpp", 157, 0, "%s", "tess.indexCount == 0"))
    {
        __debugbreak();
    }
    vertCount = tess.vertexCount;
    tess.indices[tess.indexCount] = LOWORD(tess.vertexCount) + 3;
    tess.indices[tess.indexCount + 1] = vertCount;
    tess.indices[tess.indexCount + 2] = vertCount + 2;
    tess.indices[tess.indexCount + 3] = vertCount + 2;
    tess.indices[tess.indexCount + 4] = vertCount;
    tess.indices[tess.indexCount + 5] = vertCount + 1;
    vert = &tess.verts[vertCount];
    vert->normal.packed = 0x3FFE7F7F;
    vert->texCoord[0] = 0.0f;
    vert->texCoord[1] = 0.0f;
    vert->color = color;
    vert[1].normal.packed = 0x3FFE7F7F;
    vert[1].texCoord[0] = 1.0f;
    vert[1].texCoord[1] = 0.0f;
    vert[1].color = color;
    vert[2].normal.packed = 0x3FFE7F7F;
    vert[2].texCoord[0] = 1.0f;
    vert[2].texCoord[1] = 1.0f;
    vert[2].color = color;
    vert[3].normal.packed = 0x3FFE7F7F;
    vert[3].texCoord[0] = 0.0f;
    vert[3].texCoord[1] = 1.0f;
    vert[3].color = color;
    tess.vertexCount += 4;
    tess.indexCount += 6;
    return vert;
}

void __cdecl RB_TessSunBillboard(float widthInClipSpace, float heightInClipSpace, GfxColor color)
{
    const GfxMatrix *transform; // [esp+18h] [ebp-28h]
    float sunTransformedPosition; // [esp+2Ch] [ebp-14h]
    float sunTransformedPosition_4; // [esp+30h] [ebp-10h]
    float sunTransformedPosition_8; // [esp+34h] [ebp-Ch]
    float sunTransformedPosition_12; // [esp+38h] [ebp-8h]
    GfxVertex *vert; // [esp+3Ch] [ebp-4h]

    transform = &gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix;
    if (!rgp.world
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_sky.cpp", 201, 0, "%s", "rgp.world"))
    {
        __debugbreak();
    }
    sunTransformedPosition = (float)((float)(rgp.world->sun.sunFxPosition[0] * transform->m[0][0])
        + (float)(rgp.world->sun.sunFxPosition[1] * transform->m[1][0]))
        + (float)(rgp.world->sun.sunFxPosition[2] * transform->m[2][0]);
    sunTransformedPosition_4 = (float)((float)(rgp.world->sun.sunFxPosition[0] * transform->m[0][1])
        + (float)(rgp.world->sun.sunFxPosition[1] * transform->m[1][1]))
        + (float)(rgp.world->sun.sunFxPosition[2] * transform->m[2][1]);
    sunTransformedPosition_8 = (float)((float)(rgp.world->sun.sunFxPosition[0] * transform->m[0][2])
        + (float)(rgp.world->sun.sunFxPosition[1] * transform->m[1][2]))
        + (float)(rgp.world->sun.sunFxPosition[2] * transform->m[2][2]);
    sunTransformedPosition_12 = (float)((float)(rgp.world->sun.sunFxPosition[0] * transform->m[0][3])
        + (float)(rgp.world->sun.sunFxPosition[1] * transform->m[1][3]))
        + (float)(rgp.world->sun.sunFxPosition[2] * transform->m[2][3]);
    vert = RB_SetTessQuad_0(color);
    vert->xyzw[0] = sunTransformedPosition;
    vert->xyzw[1] = sunTransformedPosition_4;
    vert->xyzw[2] = sunTransformedPosition_8;
    vert->xyzw[3] = sunTransformedPosition_12;
    vert[1].xyzw[0] = sunTransformedPosition;
    vert[1].xyzw[1] = sunTransformedPosition_4;
    vert[1].xyzw[2] = sunTransformedPosition_8;
    vert[1].xyzw[3] = sunTransformedPosition_12;
    vert[2].xyzw[0] = sunTransformedPosition;
    vert[2].xyzw[1] = sunTransformedPosition_4;
    vert[2].xyzw[2] = sunTransformedPosition_8;
    vert[2].xyzw[3] = sunTransformedPosition_12;
    vert[3].xyzw[0] = sunTransformedPosition;
    vert[3].xyzw[1] = sunTransformedPosition_4;
    vert[3].xyzw[2] = sunTransformedPosition_8;
    vert[3].xyzw[3] = sunTransformedPosition_12;
    vert->xyzw[0] = vert->xyzw[0] + (float)(widthInClipSpace * sunTransformedPosition_12);
    vert->xyzw[1] = vert->xyzw[1] + (float)(heightInClipSpace * sunTransformedPosition_12);
    vert->xyzw[2] = vert->xyzw[2] + (float)(-0.00050000002 * sunTransformedPosition_12);
    vert[1].xyzw[0] = vert[1].xyzw[0] + (float)(widthInClipSpace * sunTransformedPosition_12);
    vert[1].xyzw[1] = vert[1].xyzw[1] - (float)(heightInClipSpace * sunTransformedPosition_12);
    vert[1].xyzw[2] = vert[1].xyzw[2] + (float)(-0.00050000002 * sunTransformedPosition_12);
    vert[2].xyzw[0] = vert[2].xyzw[0] - (float)(widthInClipSpace * sunTransformedPosition_12);
    vert[2].xyzw[1] = vert[2].xyzw[1] - (float)(heightInClipSpace * sunTransformedPosition_12);
    vert[2].xyzw[2] = vert[2].xyzw[2] + (float)(-0.00050000002 * sunTransformedPosition_12);
    vert[3].xyzw[0] = vert[3].xyzw[0] - (float)(widthInClipSpace * sunTransformedPosition_12);
    vert[3].xyzw[1] = vert[3].xyzw[1] + (float)(heightInClipSpace * sunTransformedPosition_12);
    vert[3].xyzw[2] = vert[3].xyzw[2] + (float)(-0.00050000002 * sunTransformedPosition_12);
}

void __cdecl RB_UpdateSunVisibilityWithoutQuery(SunFlareDynamic *sunFlare)
{
    float sunTraceEnd[3]; // [esp+8h] [ebp-Ch] BYREF

    if (!sunFlare
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_sky.cpp", 297, 0, "%s", "sunFlare"))
    {
        __debugbreak();
    }
    sunFlare->lastVisibility = RB_GetSunSampleRectRelativeArea(16, 16);
    if (sunFlare->lastVisibility != 0.0)
    {
        sunTraceEnd[0] = (float)(262144.0 * rgp.world->sun.sunFxPosition[0]) + gfxCmdBufSourceState.viewParms3D->origin[0];
        sunTraceEnd[1] = (float)(262144.0 * rgp.world->sun.sunFxPosition[1]) + gfxCmdBufSourceState.viewParms3D->origin[1];
        sunTraceEnd[2] = (float)(262144.0 * rgp.world->sun.sunFxPosition[2]) + gfxCmdBufSourceState.viewParms3D->origin[2];
        sunFlare->hitNum = CM_BoxSightTrace(
            sunFlare->hitNum,
            gfxCmdBufSourceState.viewParms3D->origin,
            sunTraceEnd,
            vec3_origin,
            vec3_origin,
            0,
            8195);
        if (sunFlare->hitNum)
            sunFlare->lastVisibility = 0.0f;
    }
}

double __cdecl RB_GetSunSampleRectRelativeArea(int widthInPixels, int heightInPixels)
{
    const GfxMatrix *transform; // [esp+28h] [ebp-30h]
    int top; // [esp+2Ch] [ebp-2Ch]
    signed int right; // [esp+30h] [ebp-28h]
    int left; // [esp+38h] [ebp-20h]
    float sunTransformedPosition_12; // [esp+4Ch] [ebp-Ch]
    signed int bottom; // [esp+50h] [ebp-8h]

    transform = &gfxCmdBufSourceState.viewParms3D->viewProjectionMatrix;
    if (!rgp.world
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_sky.cpp", 258, 0, "%s", "rgp.world"))
    {
        __debugbreak();
    }
    sunTransformedPosition_12 = (float)((float)(rgp.world->sun.sunFxPosition[0] * transform->m[0][3])
        + (float)(rgp.world->sun.sunFxPosition[1] * transform->m[1][3]))
        + (float)(rgp.world->sun.sunFxPosition[2] * transform->m[2][3]);
    if (sunTransformedPosition_12 <= 0.0)
        return 0.0;
    left = SnapFloatToInt((float)((float)((float)((float)((float)((float)((float)((float)(rgp.world->sun.sunFxPosition[0]
        * transform->m[0][0])
        + (float)(rgp.world->sun.sunFxPosition[1]
            * transform->m[1][0]))
        + (float)(rgp.world->sun.sunFxPosition[2]
            * transform->m[2][0]))
        / sunTransformedPosition_12)
        + 1.0)
        * (float)(int)vidConfig.displayWidth)
        - (float)widthInPixels)
        * 0.5));
    top = SnapFloatToInt((float)((float)((float)((float)((float)((float)((float)((float)(rgp.world->sun.sunFxPosition[0]
        * transform->m[0][1])
        + (float)(rgp.world->sun.sunFxPosition[1]
            * transform->m[1][1]))
        + (float)(rgp.world->sun.sunFxPosition[2]
            * transform->m[2][1]))
        / sunTransformedPosition_12)
        + 1.0)
        * (float)(int)vidConfig.displayHeight)
        - (float)heightInPixels)
        * 0.5));
    right = widthInPixels + left;
    bottom = heightInPixels + top;
    if (left < 0)
        left = 0;
    if (right > (int)vidConfig.displayWidth)
        right = vidConfig.displayWidth;
    if (top < 0)
        top = 0;
    if (bottom > (int)vidConfig.displayHeight)
        bottom = vidConfig.displayHeight;
    if (right > left && bottom > top)
        return (double)((bottom - top) * (right - left)) / (double)(heightInPixels * widthInPixels);
    else
        return 0.0;
}

void __cdecl RB_AddSunEffects(SunFlareDynamic *sunFlare)
{
    if (!rgp.world
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_sky.cpp", 574, 0, "%s", "rgp.world"))
    {
        __debugbreak();
    }
    if (!sunFlare
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_sky.cpp", 575, 0, "%s", "sunFlare"))
    {
        __debugbreak();
    }
    sunFlare->lastDot = (float)((float)(rgp.world->sun.sunFxPosition[0] * gfxCmdBufSourceState.viewParms3D->axis[0][0])
        + (float)(rgp.world->sun.sunFxPosition[1] * gfxCmdBufSourceState.viewParms3D->axis[0][1]))
        + (float)(rgp.world->sun.sunFxPosition[2] * gfxCmdBufSourceState.viewParms3D->axis[0][2]);
    //if (sunFlare->lastDot > 0.0)
    //    BG_EvalVehicleName();
}

void __cdecl RB_DrawSunPostEffects(unsigned int localClientNum, float sunVisibilityAdjust)
{
    int frameTime; // [esp+18h] [ebp-Ch]
    SunFlareDynamic *sunFlare; // [esp+1Ch] [ebp-8h]

    PROF_SCOPED("RB_DrawSunPostEffects");

    if (localClientNum >= 4
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_sky.cpp",
            650,
            0,
            "localClientNum doesn't index ARRAY_COUNT( sunFlareArray )\n\t%i not in [0, %i)",
            localClientNum,
            4))
    {
        __debugbreak();
    }
    sunFlare = &sunFlareArray[localClientNum];
    sunFlare->sunVisibilityAdjust = sunVisibilityAdjust;
    if (sunFlare->lastTime && sunFlare->lastTime <= gfxCmdBufSourceState.sceneDef.time)
        frameTime = gfxCmdBufSourceState.sceneDef.time - sunFlare->lastTime;
    else
        frameTime = 10;
    sunFlare->lastTime = gfxCmdBufSourceState.sceneDef.time;
    if (r_drawSun->current.enabled)
    {
        if (!rgp.world
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_sky.cpp", 667, 0, "%s", "rgp.world"))
        {
            __debugbreak();
        }
        if (rgp.world->sun.hasValidData)
        {
            RB_DrawSunFlare(sunFlare, frameTime);
            RB_DrawBlindAndGlare(sunFlare, frameTime);
        }
    }
}

double __cdecl R_UpdateOverTime_0(float fCurrent, float fGoal, int iFadeInTime, int iFadeOutTime, int frametime)
{
  if ( frametime < 0
    && !Assert_MyHandler(
          "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_sky.cpp",
          122,
          0,
          "%s\n\t(frametime) = %i",
          "(frametime >= 0)",
          frametime) )
  {
    __debugbreak();
  }
  if ( iFadeInTime < 0
    && !Assert_MyHandler(
          "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_sky.cpp",
          123,
          0,
          "%s\n\t(iFadeInTime) = %i",
          "(iFadeInTime >= 0)",
          iFadeInTime) )
  {
    __debugbreak();
  }
  if ( iFadeOutTime < 0
    && !Assert_MyHandler(
          "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_sky.cpp",
          124,
          0,
          "%s\n\t(iFadeOutTime) = %i",
          "(iFadeOutTime >= 0)",
          iFadeOutTime) )
  {
    __debugbreak();
  }
  if ( !frametime )
    return fGoal;
  if ( fGoal <= fCurrent )
  {
    if ( fCurrent > fGoal )
    {
      if ( iFadeOutTime <= 0 )
        return fGoal;
      fCurrent = fCurrent - (float)((float)((float)frametime * 1.0) / (float)iFadeOutTime);
      if ( fGoal > fCurrent )
        return fGoal;
    }
  }
  else
  {
    if ( iFadeInTime <= 0 )
      return fGoal;
    fCurrent = (float)((float)((float)frametime * 1.0) / (float)iFadeInTime) + fCurrent;
    if ( fCurrent > fGoal )
      return fGoal;
  }
  return fCurrent;
}

void __cdecl RB_DrawSunFlare(SunFlareDynamic *sunFlare, int frameTime)
{
    float lerp; // [esp+34h] [ebp-10h]
    float size; // [esp+38h] [ebp-Ch]
    float alpha; // [esp+40h] [ebp-4h]

    PROF_SCOPED("RB_DrawSunFlare");

    if ( !sunFlare
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_sky.cpp", 485, 0, "%s", "sunFlare") )
    {
        __debugbreak();
    }
    if ( R_StereoActivated() )
    {
        return;
    }
    if ( !rgp.world->sun.flareMaterial )
    {
        return;
    }
    if ( rgp.world->sun.flareMinDot >= sunFlare->lastDot )
    {
        return;
    }
    if ( sunFlare->lastDot < rgp.world->sun.flareMaxDot )
        lerp = (float)(sunFlare->lastDot - rgp.world->sun.flareMinDot)
                 / (float)(rgp.world->sun.flareMaxDot - rgp.world->sun.flareMinDot);
    else
        lerp = 1.0f;
    if ( (lerp < 0.0 || lerp > 1.0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_sky.cpp",
                    503,
                    1,
                    "%s\n\t(lerp) = %g",
                    "(lerp >= 0.0f && lerp <= 1.0f)",
                    lerp) )
    {
        __debugbreak();
    }
    if ( rgp.world->sun.flareMaxAlpha < 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_sky.cpp",
                    504,
                    1,
                    "%s\n\t(rgp.world->sun.flareMaxAlpha) = %g",
                    "(rgp.world->sun.flareMaxAlpha >= 0.0f)",
                    rgp.world->sun.flareMaxAlpha) )
    {
        __debugbreak();
    }
    alpha = lerp * rgp.world->sun.flareMaxAlpha;
    size = (float)(lerp * rgp.world->sun.flareMaxSize) + rgp.world->sun.flareMinSize;
    if ( sunFlare->flareIntensity < 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_sky.cpp",
                    509,
                    0,
                    "%s\n\t(sunFlare->flareIntensity) = %g",
                    "(sunFlare->flareIntensity >= 0.0f)",
                    sunFlare->flareIntensity) )
    {
        __debugbreak();
    }
    if ( sunFlare->lastVisibility < 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_sky.cpp",
                    510,
                    0,
                    "%s\n\t(sunFlare->lastVisibility) = %g",
                    "(sunFlare->lastVisibility >= 0.0f)",
                    sunFlare->lastVisibility) )
    {
        __debugbreak();
    }
    sunFlare->flareIntensity = R_UpdateOverTime_0(
                                                             sunFlare->flareIntensity,
                                                             sunFlare->lastVisibility,
                                                             rgp.world->sun.flareFadeInTime,
                                                             rgp.world->sun.flareFadeOutTime,
                                                             frameTime);
    if ( sunFlare->flareIntensity < 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_sky.cpp",
                    514,
                    0,
                    "%s\n\t(sunFlare->flareIntensity) = %g",
                    "(sunFlare->flareIntensity >= 0.0f)",
                    sunFlare->flareIntensity) )
    {
        __debugbreak();
    }
    if ( alpha < 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_sky.cpp",
                    515,
                    0,
                    "%s\n\t(alpha) = %g",
                    "(alpha >= 0.0f)",
                    alpha) )
    {
        __debugbreak();
    }
    RB_DrawSunFlareCore((float)(sunFlare->flareIntensity * alpha) * sunFlare->sunVisibilityAdjust, size);
}

void __cdecl RB_DrawSunFlareCore(float alpha, float sizeIn640x480)
{
    RB_SetTessTechnique(rgp.world->sun.flareMaterial, 4u);
    R_TrackPrims(&gfxCmdBufState.prim, GFX_PRIM_STATS_FX);
    if ( (alpha < 0.0 || alpha > 1.0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_sky.cpp",
                    468,
                    0,
                    "%s\n\t(alpha) = %g",
                    "(alpha >= 0 && alpha <= 1)",
                    alpha) )
    {
        __debugbreak();
    }
    RB_SetIdentity();
    RB_TessSunBillboard(
        sizeIn640x480 / 640.0,
        sizeIn640x480 / 480.0,
        (GfxColor)((unsigned __int8)SnapFloatToInt((float)(alpha * 255.0))
                         | ((unsigned __int8)SnapFloatToInt((float)(alpha * 255.0)) << 8)
                         | ((unsigned __int8)SnapFloatToInt((float)(alpha * 255.0)) << 16)
                         | 0xFF000000));
}

void __cdecl RB_DrawBlindAndGlare(SunFlareDynamic *sunFlare, int frameTime)
{
    float colorVec[4]; // [esp+1Ch] [ebp-24h] BYREF
    float blind; // [esp+2Ch] [ebp-14h] BYREF
    float epsilon; // [esp+30h] [ebp-10h]
    float glare; // [esp+34h] [ebp-Ch] BYREF
    GfxColor color; // [esp+3Ch] [ebp-4h] BYREF

    PROF_SCOPED("RB_DrawBlindAndGlare");

    if ( !sunFlare
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_sky.cpp", 592, 0, "%s", "sunFlare") )
    {
        __debugbreak();
    }
    RB_CalcSunBlind(sunFlare, frameTime, &blind, &glare);
    epsilon = 0.001f;
    if ( glare <= 0.001 && blind <= epsilon )
    {
        if ( tess.indexCount )
            RB_EndTessSurface();
    }
    else
    {
        colorVec[0] = glare;
        colorVec[1] = glare;
        colorVec[2] = glare;
        colorVec[3] = blind;
        R_ConvertColorToBytes(colorVec, (unsigned __int8 *)&color);
        RB_DrawFullScreenColoredQuad(rgp.glareBlindMaterial, 0.0, 0.0, 1.0, 1.0, color.packed);
    }
}

void __cdecl RB_CalcSunBlind(SunFlareDynamic *sunFlare, int frameTime, float *blind, float *glare)
{
    float blindLerp; // [esp+1Ch] [ebp-Ch]
    float blindLerpa; // [esp+1Ch] [ebp-Ch]
    float glareLerp; // [esp+20h] [ebp-8h]
    float sunDot; // [esp+24h] [ebp-4h]

    if (!sunFlare
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_sky.cpp", 526, 0, "%s", "sunFlare"))
    {
        __debugbreak();
    }
    if (!rgp.world
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_sky.cpp", 527, 0, "%s", "rgp.world"))
    {
        __debugbreak();
    }
    sunDot = (float)((float)(rgp.world->sun.sunFxPosition[0] * gfxCmdBufSourceState.viewParms3D->axis[0][0])
        + (float)(rgp.world->sun.sunFxPosition[1] * gfxCmdBufSourceState.viewParms3D->axis[0][1]))
        + (float)(rgp.world->sun.sunFxPosition[2] * gfxCmdBufSourceState.viewParms3D->axis[0][2]);
    if (rgp.world->sun.blindMaxDarken > 0.0)
    {
        if (rgp.world->sun.blindMinDot < sunDot)
        {
            if (sunDot < rgp.world->sun.blindMaxDot)
                blindLerp = (float)(sunDot - rgp.world->sun.blindMinDot)
                / (float)(rgp.world->sun.blindMaxDot - rgp.world->sun.blindMinDot);
            else
                blindLerp = 1.0f;
        }
        else
        {
            blindLerp = 0.0f;
        }
        if (blindLerp < 0.0
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_sky.cpp",
                543,
                0,
                "%s\n\t(blindLerp) = %g",
                "(blindLerp >= 0)",
                blindLerp))
        {
            __debugbreak();
        }
        if (blindLerp > 1.0
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_sky.cpp",
                544,
                0,
                "%s\n\t(blindLerp) = %g",
                "(blindLerp <= 1)",
                blindLerp))
        {
            __debugbreak();
        }
        blindLerpa = (float)(sunFlare->lastVisibility * sunFlare->sunVisibilityAdjust) * blindLerp;
        if (blindLerpa < 0.0
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_sky.cpp",
                546,
                0,
                "%s\n\t(blindLerp) = %g",
                "(blindLerp >= 0)",
                blindLerpa))
        {
            __debugbreak();
        }
        if (blindLerpa > 1.0
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_sky.cpp",
                547,
                0,
                "%s\n\t(blindLerp) = %g",
                "(blindLerp <= 1)",
                blindLerpa))
        {
            __debugbreak();
        }
        sunFlare->currentBlind = R_UpdateOverTime_0(
            sunFlare->currentBlind,
            blindLerpa,
            rgp.world->sun.blindFadeInTime,
            rgp.world->sun.blindFadeOutTime,
            frameTime);
        if (sunFlare->currentBlind < 0.0
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_sky.cpp",
                549,
                0,
                "%s\n\t(sunFlare->currentBlind) = %g",
                "(sunFlare->currentBlind >= 0)",
                sunFlare->currentBlind))
        {
            __debugbreak();
        }
        if (sunFlare->currentBlind > 1.0
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_sky.cpp",
                550,
                0,
                "%s\n\t(sunFlare->currentBlind) = %g",
                "(sunFlare->currentBlind <= 1)",
                sunFlare->currentBlind))
        {
            __debugbreak();
        }
        *blind = sunFlare->currentBlind * rgp.world->sun.blindMaxDarken;
    }
    else
    {
        *blind = 0.0f;
    }
    if (rgp.world->sun.glareMaxLighten > 0.0)
    {
        if (rgp.world->sun.glareMinDot < sunDot)
        {
            if (sunDot < rgp.world->sun.glareMaxDot)
                glareLerp = (float)(sunDot - rgp.world->sun.glareMinDot)
                / (float)(rgp.world->sun.glareMaxDot - rgp.world->sun.glareMinDot);
            else
                glareLerp = 1.0f;
        }
        else
        {
            glareLerp = 0.0f;
        }
        sunFlare->currentGlare = R_UpdateOverTime_0(
            sunFlare->currentGlare,
            (float)(sunFlare->lastVisibility * sunFlare->sunVisibilityAdjust) * glareLerp,
            rgp.world->sun.glareFadeInTime,
            rgp.world->sun.glareFadeOutTime,
            frameTime);
        *glare = sunFlare->currentGlare * rgp.world->sun.glareMaxLighten;
    }
    else
    {
        *glare = 0.0f;
    }
}

void __cdecl RB_AllocSunSpriteQueries()
{
    unsigned int viewIndex; // [esp+8h] [ebp-8h]
    unsigned int queryIndex; // [esp+Ch] [ebp-4h]

    for ( viewIndex = 0; viewIndex < gfxCfg.maxClientViews; ++viewIndex )
    {
        for ( queryIndex = 0; queryIndex < 2; ++queryIndex )
        {
            sunFlareArray[viewIndex].sunQuery[queryIndex] = RB_HW_AllocOcclusionQuery();
            sunFlareArray[viewIndex].sunQueryIssued[queryIndex] = 0;
        }
    }
}

void __cdecl RB_FreeSunSpriteQueries()
{
    IDirect3DQuery9 *varCopy; // [esp+0h] [ebp-Ch]
    unsigned int viewIndex; // [esp+4h] [ebp-8h]
    unsigned int queryIndex; // [esp+8h] [ebp-4h]

    for ( viewIndex = 0; viewIndex < 4; ++viewIndex )
    {
        for ( queryIndex = 0; queryIndex < 2; ++queryIndex )
        {
            if ( sunFlareArray[viewIndex].sunQuery[queryIndex] )
            {
                do
                {
                    if ( r_logFile )
                    {
                        if ( r_logFile->current.integer )
                            RB_LogPrint("sunFlareArray[viewIndex].sunQuery[queryIndex]->Release()\n");
                    }
                    varCopy = sunFlareArray[viewIndex].sunQuery[queryIndex];
                    sunFlareArray[viewIndex].sunQuery[queryIndex] = 0;
                    R_ReleaseAndSetNULL<IDirect3DIndexBuffer9>(
                        (IDirect3DSurface9 *)varCopy,
                        "sunFlareArray[viewIndex].sunQuery[queryIndex]",
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_sky.cpp",
                        707);
                }
                while ( alwaysfails );
            }
        }
    }
}

