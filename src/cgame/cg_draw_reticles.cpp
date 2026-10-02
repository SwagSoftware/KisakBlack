#include "cg_draw_reticles.h"
#include <universal/com_math_anglevectors.h>
#include <bgame/bg_weapons_def.h>
#include <cgame_mp/cg_main_mp.h>
#include <client_mp/cl_cgame_mp.h>
#include <cgame_mp/cg_view_mp.h>
#include <cgame_mp/cg_newDraw_mp.h>
#include <bgame/bg_misc.h>
#include <cgame_mp/cg_draw_mp.h>
#include <gfx_d3d/r_init.h>
#include "cg_world.h"
#include <gfx_d3d/r_dvars.h>
#include "cg_compass.h"
#include <cgame_mp/cg_vehicles_mp.h>
#include <demo/demo_playback.h>
#include <ui_mp/ui_main_mp.h>
#include "cg_drawtools.h"

void __cdecl CG_CalcCrosshairPosition(const cg_s *cgameGlob, float *x, float *y)
{
    float gunYaw; // [esp+10h] [ebp-24h]
    float v4; // [esp+14h] [ebp-20h]
    float gunDir[3]; // [esp+18h] [ebp-1Ch] BYREF
    float gunAng[3]; // [esp+24h] [ebp-10h] BYREF
    float dot; // [esp+30h] [ebp-4h]

    gunYaw = cgameGlob->gunYaw;
    v4 = cgameGlob->refdefViewAngles[2];
    gunAng[0] = cgameGlob->gunPitch;
    gunAng[1] = gunYaw;
    gunAng[2] = v4;
    AngleVectors(gunAng, gunDir, 0, 0);
    dot = (float)((float)(cgameGlob->refdef.viewaxis[0][0] * gunDir[0])
                            + (float)(cgameGlob->refdef.viewaxis[0][1] * gunDir[1]))
            + (float)(cgameGlob->refdef.viewaxis[0][2] * gunDir[2]);
    if ( dot <= 0.0 || cgameGlob->refdef.tanHalfFovX <= 0.0 )
    {
        *x = 0.0f;
        *y = 0.0f;
    }
    else
    {
        *x = (float)((float)((float)((float)(cgameGlob->refdef.viewaxis[1][0] * gunDir[0])
                                                             + (float)(cgameGlob->refdef.viewaxis[1][1] * gunDir[1]))
                                             + (float)(cgameGlob->refdef.viewaxis[1][2] * gunDir[2]))
                             / (float)(dot * cgameGlob->refdef.tanHalfFovX))
             * -320.0;
        *y = (float)((float)((float)((float)(cgameGlob->refdef.viewaxis[2][0] * gunDir[0])
                                                             + (float)(cgameGlob->refdef.viewaxis[2][1] * gunDir[1]))
                                             + (float)(cgameGlob->refdef.viewaxis[2][2] * gunDir[2]))
                             / (float)(dot * cgameGlob->refdef.tanHalfFovY))
             * -240.0;
    }
}

char __cdecl CG_GetWeapReticleZoom(const cg_s *cgameGlob, float *zoom)
{
    const WeaponVariantDef *weapVariantDef; // [esp+0h] [ebp-10h]
    int weapIndex; // [esp+4h] [ebp-Ch]
    float fPosLerp; // [esp+8h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+Ch] [ebp-4h]

    weapIndex = BG_GetViewmodelWeaponIndex(&cgameGlob->predictedPlayerState);
    weapVariantDef = BG_GetWeaponVariantDef(weapIndex);
    weapDef = BG_GetWeaponDef(weapIndex);
    fPosLerp = cgameGlob->predictedPlayerState.fWeaponPosFrac;
    *zoom = 0.0f;
    if ( !weapVariantDef
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_draw_reticles.cpp",
                    131,
                    0,
                    "%s",
                    "weapVariantDef") )
    {
        __debugbreak();
    }
    if ( !weapDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_draw_reticles.cpp", 132, 0, "%s", "weapDef") )
    {
        __debugbreak();
    }
    if ( !weapVariantDef->overlayMaterial && weapDef->overlayReticle == WEAPOVERLAYRETICLE_NONE )
        return 0;
    if ( fPosLerp == 0.0 )
        return 0;
    if ( cgameGlob->playerEntity.bPositionToADS )
    {
        *zoom = fPosLerp - (float)(1.0 - weapVariantDef->fAdsZoomInFrac);
        if ( *zoom > 0.0 )
            *zoom = *zoom / weapVariantDef->fAdsZoomInFrac;
    }
    else
    {
        *zoom = fPosLerp - (float)(1.0 - weapVariantDef->fAdsZoomOutFrac);
        if ( *zoom > 0.0 )
            *zoom = *zoom / weapVariantDef->fAdsZoomOutFrac;
    }
    if ( *zoom <= 0.0099999998 )
        return 0;
    if ( *zoom > 1.0 )
        *zoom = 1.0f;
    return 1;
}

void __cdecl CG_DrawNightVisionOverlay(int localClientNum)
{
    CG_GetLocalClientGlobals(localClientNum);
    if (CG_LookingThroughNightVision(localClientNum))
    {
        if (cgMedia.nightVisionOverlay)
            CL_DrawStretchPic(
                &scrPlaceView[localClientNum],
                0.0,
                0.0,
                640.0,
                480.0,
                4,
                4,
                0.0,
                0.0,
                1.0,
                1.0,
                colorWhite,
                cgMedia.nightVisionOverlay);
        else
            Com_PrintWarning(14, "CG_DrawNightVisionOverlay(): Nightvision Assets not Precached.\n");
    }
}

bool __cdecl CG_UsingLowResViewPort(int localClientNum)
{
    return scrPlaceView[localClientNum].realViewportSize[1] <= 480.0;
}

bool __cdecl CG_ColorCrosshairs(int time, int startMsec, int lastMsec, int fadeInMsec, int fadeOutMsec)
{
    return time - lastMsec < fadeOutMsec;
}

bool __cdecl ShouldShowCrosshairOnVehicle(const cg_s *cgameGlob, const WeaponDef *weapDef)
{
    if ( !weapDef->bKeepCrosshairWhenADS
        && cgameGlob->predictedPlayerState.fWeaponPosFrac != 0.0
        && cgameGlob->predictedPlayerState.vehiclePos >= 1
        && cgameGlob->predictedPlayerState.vehiclePos <= 4 )
    {
        return 0;
    }
    if ( weapDef->reticleCenter )
        return cg_crosshairAlpha->current.value >= 0.0099999998;
    return 0;
}

bool __cdecl ShouldDrawCrosshair(const cg_s *cgameGlob, const playerState_s *ps)
{
    if ( cgameGlob->cameraMode == 1 )
        return 0;
    if ( ps->weaponstate >= 47 && ps->weaponstate <= 49 )
        return 0;
    if ( cgameGlob->renderingThirdPerson == TP_FOR_MODEL && (ps->eFlags & 0x4000) == 0 )
        return 0;
    if ( cgameGlob->renderingThirdPerson == TP_FOR_MODEL && CG_KillCamEntityEnabled(cgameGlob->localClientNum) )
        return 0;
    if ( ps->pm_type < 9 )
        return 1;
    if ( (ps->eFlags & 0x4000) != 0 && ps->vehicleType == 6 )
        return 1;
    return (ps->eFlags2 & 0x10000000) != 0;
}

void __cdecl CG_DrawCrosshair(int localClientNum)
{
    float v1; // [esp+10h] [ebp-14Ch]
    float start[3]; // [esp+B4h] [ebp-A8h] BYREF
    float end[3]; // [esp+C0h] [ebp-9Ch] BYREF
    trace_t trace; // [esp+CCh] [ebp-90h] BYREF
    float contact[3]; // [esp+108h] [ebp-54h] BYREF
    float centerW; // [esp+114h] [ebp-48h]
    float posLerp; // [esp+11Ch] [ebp-40h]
    const cg_s *cgameGlob; // [esp+120h] [ebp-3Ch]
    bool drawHud; // [esp+126h] [ebp-36h]
    bool forceCrosshairsOn; // [esp+127h] [ebp-35h]
    const cgs_t *cgs; // [esp+128h] [ebp-34h]
    float reticleAlpha; // [esp+12Ch] [ebp-30h]
    float centerY; // [esp+130h] [ebp-2Ch] BYREF
    int weapIndex; // [esp+134h] [ebp-28h]
    const playerState_s *ps; // [esp+138h] [ebp-24h]
    const WeaponDef *weapDef; // [esp+13Ch] [ebp-20h]
    float color[4]; // [esp+140h] [ebp-1Ch] BYREF
    float transShift; // [esp+150h] [ebp-Ch] BYREF
    float transScale; // [esp+154h] [ebp-8h] BYREF
    float centerX; // [esp+158h] [ebp-4h] BYREF

    PROF_SCOPED("CG_DrawCrosshair");

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    ps = &cgameGlob->predictedPlayerState;
    if ( !cg_drawGun
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_draw_reticles.cpp", 1202, 0, "%s", "cg_drawGun") )
    {
        __debugbreak();
    }
    if ( !cg_crosshairDynamic
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_draw_reticles.cpp",
                    1203,
                    0,
                    "%s",
                    "cg_crosshairDynamic") )
    {
        __debugbreak();
    }
    if ( !ShouldDrawCrosshair(cgameGlob, ps) )
    {
        return;
    }
    drawHud = CG_ShouldDrawHud(localClientNum);
    posLerp = ps->fWeaponPosFrac;
    transScale = 1.0f;
    transShift = 0.0f;
    if ( (ps->eFlags & 0x300) == 0 )
    {
        if ( (ps->eFlags & 0x4000) != 0 )
        {
            CG_DrawVehicleCrossHair(localClientNum);
        }
        else if ( show_reticle_during_swimming->current.enabled || ps->waterlevel < 3 )
        {
            weapIndex = BG_GetViewmodelWeaponIndex(&cgameGlob->predictedPlayerState);
            if ( !weapIndex )
            {
                return;
            }
            weapDef = BG_GetWeaponDef(weapIndex);
            reticleAlpha = CG_DrawWeapReticle(localClientNum);
            if ( CG_Flashbanged(localClientNum) )
            {
                return;
            }
            else
            {
                if ( CG_Flared(localClientNum) )
                {
                    return;
                }
                if ( drawHud )
                {
                    cgs = CG_GetLocalClientStaticGlobals(localClientNum);
                    CG_CalcCrosshairColor(localClientNum, reticleAlpha, color);
                    if ( color[3] >= 0.0099999998 )
                    {
                        forceCrosshairsOn = weapDef->bKeepCrosshairWhenADS;
                        if ( posLerp == 1.0 && cg_drawGun->current.enabled && !forceCrosshairsOn )
                        {
                            return;
                        }
                        else
                        {
                            if ( AllowedToDrawCrosshair(localClientNum, &cgameGlob->predictedPlayerState) )
                            {
                                CG_CalcCrosshairPosition(cgameGlob, &centerX, &centerY);
                                if ( posLerp != 0.0 && !weapDef->bKeepCrosshairWhenADS )
                                {
                                    CG_TransitionToAds(cgameGlob, weapDef, posLerp, &transScale, &transShift);
                                    CG_DrawAdsAimIndicator(localClientNum, weapDef, color, centerX, centerY, transScale);
                                }
                                if ( posLerp != 1.0 || cg_drawGun->current.enabled )
                                {
                                    if ( !cg_crosshairDynamic->current.enabled )
                                    {
                                        centerX = 0.0f;
                                        centerY = transShift;
                                    }
                                    centerW = 1.0f;
                                    if ( !IsHardcoreMode(localClientNum) )
                                    {
                                        if ( R_StereoActivated() )
                                        {
                                            start[0] = cgameGlob->refdef.vieworg[0];
                                            start[1] = cgameGlob->refdef.vieworg[1];
                                            start[2] = cgameGlob->refdef.vieworg[2];
                                            end[0] = (float)(15000.0 * cgameGlob->refdef.viewaxis[0][0]) + start[0];
                                            end[1] = (float)(15000.0 * cgameGlob->refdef.viewaxis[0][1]) + start[1];
                                            end[2] = (float)(15000.0 * cgameGlob->refdef.viewaxis[0][2]) + start[2];
                                            CG_LocationalTrace(&trace, start, end, cgameGlob->nextSnap->ps.clientNum, 0x280B001, 0, 0);
                                            Vec3Lerp(start, end, trace.fraction, contact);
                                            centerW = Vec3Distance(start, contact);
                                            if ( centerW >= 10000.0 )
                                                v1 = 10000.0f;
                                            else
                                                v1 = centerW;
                                            centerW = v1;
                                        }
                                        CG_DrawReticleCenter(localClientNum, weapDef, color, centerX, centerY, centerW, transScale);
                                        CG_DrawReticleSides(localClientNum, weapIndex, color, centerX, centerY, centerW, transScale);
                                        //if ( g_DXDeviceThread == GetCurrentThreadId() )
                                            //D3DPERF_EndEvent();
                                        return;
                                    }
                                    //if ( GetCurrentThreadId() != g_DXDeviceThread )
                                    //    return;
                                }
                                //else if ( g_DXDeviceThread != GetCurrentThreadId() )
                                //{
                                //    return;
                                //}
                                //D3DPERF_EndEvent();
                                return;
                            }
                            //if ( g_DXDeviceThread != GetCurrentThreadId() )
                            //    return;
                        }
                    }
                    //else if ( GetCurrentThreadId() != g_DXDeviceThread )
                    //{
                    //    return;
                    //}
                    //D3DPERF_EndEvent();
                    return;
                }
                //if ( GetCurrentThreadId() != g_DXDeviceThread )
                //    return;
            }
        }
        //else if ( GetCurrentThreadId() != g_DXDeviceThread )
        //{
        //    return;
        //}
        return;
    }
    weapIndex = CG_PlayerTurretWeaponIdx(localClientNum);
    if ( weapIndex && BG_GetWeaponVariantDef(weapIndex)->overlayMaterial )
    {
        CG_DrawAdsOverlay(localClientNum, weapIndex, colorWhite, vec2_origin);
        return;
    }
    if ( CG_Flashbanged(localClientNum) )
    {
        return;
    }
    if ( CG_Flared(localClientNum) )
    {
        return;
    }
    if ( drawHud && ps->viewlocked_entNum != 1023 )
        CG_DrawTurretCrossHair(localClientNum);
}

void __cdecl CG_DrawAdsOverlay(int localClientNum, int weaponIndex, const float *color, const float *crosshairPos)
{
    float v4; // [esp+34h] [ebp-A8h]
    float start[3]; // [esp+50h] [ebp-8Ch] BYREF
    float end[3]; // [esp+5Ch] [ebp-80h] BYREF
    trace_t trace; // [esp+68h] [ebp-74h] BYREF
    float contact[3]; // [esp+A4h] [ebp-38h] BYREF
    float centerW; // [esp+B0h] [ebp-2Ch]
    const ScreenPlacement *scrPlace; // [esp+B4h] [ebp-28h]
    float drawPos[2]; // [esp+B8h] [ebp-24h] BYREF
    Material *material; // [esp+C0h] [ebp-1Ch]
    cg_s *cgameGlob; // [esp+C4h] [ebp-18h]
    int vertAlign; // [esp+C8h] [ebp-14h]
    float drawSize[2]; // [esp+CCh] [ebp-10h] BYREF
    int horzAlign; // [esp+D4h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+D8h] [ebp-4h]

    weapDef = BG_GetWeaponDef(weaponIndex);
    if ( !weapDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_draw_reticles.cpp", 296, 0, "%s", "weapDef") )
    {
        __debugbreak();
    }
    material = CG_GetWeaponOverlay(localClientNum, weaponIndex);
    if ( material )
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        horzAlign = 2;
        vertAlign = 2;
        drawSize[0] = weapDef->overlayWidth;
        drawSize[1] = weapDef->overlayHeight;
        scrPlace = &scrPlaceView[localClientNum];
        if ( drawSize[0] <= 320.0 && drawSize[1] <= 240.0 )
        {
            drawPos[0] = *crosshairPos - drawSize[0];
            drawPos[1] = crosshairPos[1] - drawSize[1];
            CL_DrawStretchPic(
                scrPlace,
                drawPos[0],
                drawPos[1],
                drawSize[0],
                drawSize[1],
                horzAlign,
                vertAlign,
                0.0,
                0.0,
                1.0,
                1.0,
                color,
                material);
            drawPos[0] = *crosshairPos;
            drawPos[1] = crosshairPos[1] - drawSize[1];
            CL_DrawStretchPic(
                scrPlace,
                drawPos[0],
                drawPos[1],
                drawSize[0],
                drawSize[1],
                horzAlign,
                vertAlign,
                1.0,
                0.0,
                0.0,
                1.0,
                color,
                material);
            drawPos[0] = *crosshairPos - drawSize[0];
            drawPos[1] = crosshairPos[1];
            CL_DrawStretchPic(
                scrPlace,
                drawPos[0],
                drawPos[1],
                drawSize[0],
                drawSize[1],
                horzAlign,
                vertAlign,
                0.0,
                1.0,
                1.0,
                0.0,
                color,
                material);
            drawPos[0] = *crosshairPos;
            drawPos[1] = crosshairPos[1];
            CL_DrawStretchPic(
                scrPlace,
                drawPos[0],
                drawPos[1],
                drawSize[0],
                drawSize[1],
                horzAlign,
                vertAlign,
                1.0,
                1.0,
                0.0,
                0.0,
                color,
                material);
            drawPos[0] = *crosshairPos - drawSize[0];
            drawPos[1] = crosshairPos[1] - drawSize[1];
            ScrPlace_ApplyRect(scrPlace, drawPos, &drawPos[1], drawSize, &drawSize[1], horzAlign, vertAlign);
            CG_DrawFrameOverlay(
                drawPos[0],
                (float)(drawSize[0] * 2.0) + drawPos[0],
                drawPos[1],
                (float)(drawSize[1] * 2.0) + drawPos[1],
                1.0,
                color,
                material);
        }
        else
        {
            centerW = 1.0f;
            if ( R_StereoActivated() )
            {
                start[0] = cgameGlob->refdef.vieworg[0];
                start[1] = cgameGlob->refdef.vieworg[1];
                start[2] = cgameGlob->refdef.vieworg[2];
                end[0] = (float)(15000.0 * cgameGlob->refdef.viewaxis[0][0]) + start[0];
                end[1] = (float)(15000.0 * cgameGlob->refdef.viewaxis[0][1]) + start[1];
                end[2] = (float)(15000.0 * cgameGlob->refdef.viewaxis[0][2]) + start[2];
                CG_LocationalTrace(&trace, start, end, cgameGlob->nextSnap->ps.clientNum, 0x280B001, 0, 0);
                Vec3Lerp(start, end, trace.fraction, contact);
                centerW = Vec3Distance(start, contact);
                if ( centerW >= 10000.0 )
                    v4 = 10000.0f;
                else
                    v4 = centerW;
                centerW = v4;
            }
            drawPos[0] = *crosshairPos - (float)(drawSize[0] * 0.5);
            drawPos[1] = crosshairPos[1] - (float)(drawSize[1] * 0.5);
            if ( !cg_debug_overlay_viewport->current.enabled )
                CL_DrawStretchPicW(
                    scrPlace,
                    drawPos[0],
                    drawPos[1],
                    centerW,
                    drawSize[0],
                    drawSize[1],
                    horzAlign,
                    vertAlign,
                    0.0,
                    0.0,
                    1.0,
                    1.0,
                    color,
                    material);
            cgameGlob->genericOverlayParamADS[0] = drawPos[0];
            cgameGlob->genericOverlayParamADS[1] = drawPos[1];
            cgameGlob->genericOverlayParamADS[2] = drawSize[0];
            cgameGlob->genericOverlayParamADS[3] = drawSize[1];
            cgameGlob->genericOverlayParamADS[4] = *color;
            cgameGlob->genericOverlayParamADS[5] = color[1];
            cgameGlob->genericOverlayParamADS[6] = color[2];
            cgameGlob->genericOverlayParamADS[7] = color[3];
            if ( cgameGlob->genericMaterialMap[0] )
                cgameGlob->genericOverlayEnableADS = 1;
            ScrPlace_ApplyRect(scrPlace, drawPos, &drawPos[1], drawSize, &drawSize[1], horzAlign, vertAlign);
            if ( !cg_debug_overlay_viewport->current.enabled )
                CG_DrawFrameOverlay(
                    drawPos[0],
                    drawPos[0] + drawSize[0],
                    drawPos[1],
                    drawPos[1] + drawSize[1],
                    centerW,
                    color,
                    material);
        }
    }
}

void __cdecl CG_DrawFrameOverlay(
                float innerLeft,
                float innerRight,
                float innerTop,
                float innerBottom,
                float centerW,
                const float *color,
                Material *material)
{
    float screenWidth; // [esp+2Ch] [ebp-24h]
    int displayHeight; // [esp+30h] [ebp-20h] BYREF
    float screenTop; // [esp+34h] [ebp-1Ch]
    float screenRight; // [esp+38h] [ebp-18h]
    float displayAspect; // [esp+3Ch] [ebp-14h] BYREF
    int displayWidth; // [esp+40h] [ebp-10h] BYREF
    float screenHeight; // [esp+44h] [ebp-Ch]
    float screenBottom; // [esp+48h] [ebp-8h]
    float screenLeft; // [esp+4Ch] [ebp-4h]

    CL_GetScreenDimensions(&displayWidth, &displayHeight, &displayAspect);
    screenWidth = (float)displayWidth;
    screenHeight = (float)displayHeight;
    if ( R_StereoActivated() )
    {
        screenLeft = -screenWidth;
        screenRight = 2.0 * screenWidth;
        screenTop = -screenHeight;
        screenBottom = 2.0 * screenHeight;
    }
    else
    {
        screenLeft = 0.0f;
        screenRight = screenWidth;
        screenTop = 0.0f;
        screenBottom = screenHeight;
    }
    if ( innerLeft > screenLeft )
        CL_DrawStretchPicPhysicalW(
            screenLeft,
            screenTop,
            centerW,
            innerLeft - screenLeft,
            screenBottom - screenTop,
            0.0,
            0.0,
            0.0,
            1.0,
            color,
            material);
    if ( screenRight > innerRight )
        CL_DrawStretchPicPhysicalW(
            innerRight,
            screenTop,
            centerW,
            screenRight - innerRight,
            screenBottom - screenTop,
            0.0,
            0.0,
            0.0,
            1.0,
            color,
            material);
    if ( innerTop > screenTop )
        CL_DrawStretchPicPhysicalW(
            innerLeft,
            screenTop,
            centerW,
            innerRight - innerLeft,
            innerTop - screenTop,
            0.0,
            0.0,
            1.0,
            0.0,
            color,
            material);
    if ( screenBottom > innerBottom )
        CL_DrawStretchPicPhysicalW(
            innerLeft,
            innerBottom,
            centerW,
            innerRight - innerLeft,
            screenBottom - innerBottom,
            0.0,
            0.0,
            1.0,
            0.0,
            color,
            material);
}

double __cdecl CG_DrawWeapReticle(int localClientNum)
{
    float fOverlayAlphaScale; // [esp+8h] [ebp-4Ch]
    const cg_s *cgameGlob; // [esp+24h] [ebp-30h]
    const WeaponVariantDef *weapVariantDef; // [esp+28h] [ebp-2Ch]
    int weapIndex; // [esp+30h] [ebp-24h]
    float crossHairAlpha; // [esp+34h] [ebp-20h]
    float color[4]; // [esp+38h] [ebp-1Ch] BYREF
    float zoomFrac; // [esp+48h] [ebp-Ch] BYREF
    float crosshairPos[2]; // [esp+4Ch] [ebp-8h] BYREF

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( CG_GetWeapReticleZoom(cgameGlob, &zoomFrac) )
    {
        weapIndex = BG_GetViewmodelWeaponIndex(&cgameGlob->predictedPlayerState);
        weapVariantDef = BG_GetWeaponVariantDef(weapIndex);
        if ( weapVariantDef->fOverlayAlphaScale >= 1.0 )
            fOverlayAlphaScale = weapVariantDef->fOverlayAlphaScale;
        else
            fOverlayAlphaScale = 1.0f;
        color[0] = 1.0f;
        color[1] = 1.0f;
        color[2] = 1.0f;
        color[3] = zoomFrac / fOverlayAlphaScale;
        CG_CalcCrosshairPosition(cgameGlob, crosshairPos, &crosshairPos[1]);
        CG_DrawAdsOverlay(localClientNum, weapIndex, color, crosshairPos);
        crossHairAlpha = 1.0 - (float)(zoomFrac / fOverlayAlphaScale);
    }
    else
    {
        crossHairAlpha = 1.0f;
    }
    if ( R_StereoActivated() )
        Dvar_SetFloat(
            (dvar_s*)r_convergence,
            (float)((float)(1.0 - cgameGlob->predictedPlayerState.fWeaponPosFrac) * 6.06253)
        + (float)(1.9224 * cgameGlob->predictedPlayerState.fWeaponPosFrac));
    if ( (crossHairAlpha < 0.0 || crossHairAlpha > 1.0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_draw_reticles.cpp",
                    457,
                    1,
                    "%s\n\t(crossHairAlpha) = %g",
                    "(crossHairAlpha >= 0 && crossHairAlpha <= 1)",
                    crossHairAlpha) )
    {
        __debugbreak();
    }
    return crossHairAlpha;
}

void __cdecl CG_CalcCrosshairColor(int localClientNum, float alpha, float *color)
{
    cg_s *cgameGlob; // [esp+8h] [ebp-20h]
    int isInVehicle; // [esp+18h] [ebp-10h]
    int weaponIndex; // [esp+1Ch] [ebp-Ch]
    const WeaponDef *weapDef; // [esp+20h] [ebp-8h]

    if ( !cg_crosshairAlpha
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_draw_reticles.cpp",
                    489,
                    0,
                    "%s",
                    "cg_crosshairAlpha") )
    {
        __debugbreak();
    }
    if ( !cg_crosshairEnemyColor
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_draw_reticles.cpp",
                    490,
                    0,
                    "%s",
                    "cg_crosshairEnemyColor") )
    {
        __debugbreak();
    }
    if ( (alpha < 0.0 || alpha > 1.0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_draw_reticles.cpp",
                    491,
                    0,
                    "%s\n\t(alpha) = %g",
                    "(alpha >= 0.0f && alpha <= 1.0f)",
                    alpha) )
    {
        __debugbreak();
    }
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    CG_GetLocalClientStaticGlobals(localClientNum);
    isInVehicle = (cgameGlob->predictedPlayerState.eFlags & 0x4000) != 0;
    weaponIndex = CG_GetPlayerWeapon(&cgameGlob->predictedPlayerState, localClientNum);
    weapDef = BG_GetWeaponDef(weaponIndex);
    if ( !weapDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_draw_reticles.cpp", 503, 0, "%s", "weapDef") )
    {
        __debugbreak();
    }
    if ( !(isInVehicle | weapDef->crosshairColorChange) )
        goto LABEL_27;
    if ( (cgameGlob->predictedPlayerState.weapFlags & 8) != 0 )
    {
        if ( CG_ColorCrosshairs(
                     cgameGlob->time,
                     cgameGlob->crosshairClientStartTime,
                     cgameGlob->crosshairClientLastTime,
                     cg_friendlyNameFadeIn->current.integer,
                     cg_friendlyNameFadeOut->current.integer) )
        {
            if ( team_indicator->current.integer == 3 )
                Dvar_GetUnpackedColor(cg_TeamColor_MyTeamAlt, color);
            else
                Dvar_GetUnpackedColor(cg_TeamColor_MyTeam, color);
        }
        else
        {
            *color = colorWhite[0];
            color[1] = 1.0;
            color[2] = 1.0;
        }
        goto LABEL_28;
    }
    if ( (cgameGlob->predictedPlayerState.weapFlags & 0x10) == 0
        || !cg_crosshairEnemyColor->current.enabled
        || !CG_ColorCrosshairs(
                    cgameGlob->time,
                    cgameGlob->crosshairClientStartTime,
                    cgameGlob->crosshairClientLastTime,
                    cg_enemyNameFadeIn->current.integer,
                    cg_enemyNameFadeOut->current.integer) )
    {
LABEL_27:
        *color = colorWhite[0];
        color[1] = 1.0;
        color[2] = 1.0;
        goto LABEL_28;
    }
    if ( team_indicator->current.integer == 3 )
        Dvar_GetUnpackedColor(cg_TeamColor_EnemyTeamAlt, color);
    else
        Dvar_GetUnpackedColor(cg_TeamColor_EnemyTeam, color);
LABEL_28:
    color[3] = alpha * cg_crosshairAlpha->current.value;
}

void __cdecl CG_DrawTurretCrossHair(int localClientNum)
{
    bool v1; // [esp+30h] [ebp-48h]
    const cg_s *cgameGlob; // [esp+38h] [ebp-40h]
    centity_s *cent; // [esp+40h] [ebp-38h]
    float drawSize_4; // [esp+50h] [ebp-28h]
    const WeaponDef *weapDef; // [esp+5Ch] [ebp-1Ch]
    float reticleColor[4]; // [esp+60h] [ebp-18h] BYREF
    float x; // [esp+70h] [ebp-8h]
    float y; // [esp+74h] [ebp-4h]

    if ( !cg_drawTurretCrosshair
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_draw_reticles.cpp",
                    620,
                    0,
                    "%s",
                    "cg_drawTurretCrosshair") )
    {
        __debugbreak();
    }
    if ( !cg_paused
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_draw_reticles.cpp", 621, 0, "%s", "cg_paused") )
    {
        __debugbreak();
    }
    if ( !cg_drawpaused
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_draw_reticles.cpp",
                    622,
                    0,
                    "%s",
                    "cg_drawpaused") )
    {
        __debugbreak();
    }
    if ( !cg_crosshairAlpha
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_draw_reticles.cpp",
                    623,
                    0,
                    "%s",
                    "cg_crosshairAlpha") )
    {
        __debugbreak();
    }
    if ( cg_drawTurretCrosshair->current.enabled
        && (CG_IsShowingZombieMap() || !cg_paused->current.integer || !cg_drawpaused->current.enabled) )
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        if ( cgameGlob->predictedPlayerState.viewlocked_entNum == 1023
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_draw_reticles.cpp",
                        630,
                        0,
                        "%s",
                        "predictedPlayerState->viewlocked_entNum != ENTITYNUM_NONE") )
        {
            __debugbreak();
        }
        cent = CG_GetEntity(localClientNum, cgameGlob->predictedPlayerState.viewlocked_entNum);
        if ( cent->nextState.eType == 11 && cent->nextState.weapon )
        {
            weapDef = BG_GetWeaponDef(cent->nextState.weapon);
            if ( weapDef->weapClass != WEAPCLASS_TURRET
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_draw_reticles.cpp",
                            640,
                            0,
                            "%s",
                            "weapDef->weapClass == WEAPCLASS_TURRET") )
            {
                __debugbreak();
            }
            if ( weapDef->reticleCenter )
            {
                if ( cg_crosshairAlpha->current.value >= 0.0099999998 )
                {
                    CG_CalcCrosshairColor(localClientNum, 1.0, reticleColor);
                    drawSize_4 = (float)weapDef->iReticleCenterSize;
                    x = -0.5 * drawSize_4;
                    y = -0.5 * drawSize_4;
                    CL_DrawStretchPic(
                        &scrPlaceView[localClientNum],
                        -0.5 * drawSize_4,
                        -0.5 * drawSize_4,
                        drawSize_4,
                        drawSize_4,
                        2,
                        2,
                        0.0,
                        0.0,
                        1.0,
                        1.0,
                        reticleColor,
                        weapDef->reticleCenter);
                }
            }
        }
    }
}

void __cdecl CG_DrawVehicleCrossHair(int localClientNum)
{
    float v1; // [esp+34h] [ebp-BCh]
    float start[3]; // [esp+50h] [ebp-A0h] BYREF
    float end[3]; // [esp+5Ch] [ebp-94h] BYREF
    trace_t trace; // [esp+68h] [ebp-88h] BYREF
    float contact[3]; // [esp+A4h] [ebp-4Ch] BYREF
    float centerW; // [esp+B0h] [ebp-40h]
    const cg_s *cgameGlob; // [esp+B4h] [ebp-3Ch]
    Material *material; // [esp+B8h] [ebp-38h]
    int weapon; // [esp+BCh] [ebp-34h]
    int vertAlign; // [esp+C0h] [ebp-30h]
    int horzAlign; // [esp+C4h] [ebp-2Ch]
    const WeaponDef *weapDef; // [esp+C8h] [ebp-28h]
    float color[4]; // [esp+CCh] [ebp-24h] BYREF
    float x; // [esp+DCh] [ebp-14h]
    float y; // [esp+E0h] [ebp-10h]
    float h; // [esp+E4h] [ebp-Ch]
    float alpha; // [esp+E8h] [ebp-8h]
    float w; // [esp+ECh] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cg_crosshairAlpha
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_draw_reticles.cpp",
                    772,
                    0,
                    "%s",
                    "cg_crosshairAlpha") )
    {
        __debugbreak();
    }
    if ( (cgameGlob->predictedPlayerState.eFlags & 0x4000) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_draw_reticles.cpp",
                    773,
                    0,
                    "%s",
                    "cgameGlob->predictedPlayerState.eFlags & EF_VEHICLE_ACTIVE") )
    {
        __debugbreak();
    }
    weapon = CG_GetPlayerWeapon(&cgameGlob->predictedPlayerState, localClientNum);
    if ( weapon )
    {
        weapDef = BG_GetWeaponDef(weapon);
        if ( ShouldShowCrosshairOnVehicle(cgameGlob, weapDef) )
        {
            alpha = CG_GetVehicleCrossHairAlpha(localClientNum);
            CG_CalcCrosshairColor(localClientNum, alpha, color);
            w = (float)weapDef->iReticleCenterSize;
            h = w;
            x = -0.5 * w;
            y = -0.5 * w;
            material = weapDef->reticleCenter;
            horzAlign = 2;
            vertAlign = 2;
            centerW = 1.0f;
            if ( R_StereoActivated() )
            {
                start[0] = cgameGlob->refdef.vieworg[0];
                start[1] = cgameGlob->refdef.vieworg[1];
                start[2] = cgameGlob->refdef.vieworg[2];
                end[0] = (float)(15000.0 * cgameGlob->refdef.viewaxis[0][0]) + start[0];
                end[1] = (float)(15000.0 * cgameGlob->refdef.viewaxis[0][1]) + start[1];
                end[2] = (float)(15000.0 * cgameGlob->refdef.viewaxis[0][2]) + start[2];
                CG_LocationalTrace(
                    &trace,
                    start,
                    end,
                    cgameGlob->nextSnap->ps.clientNum,
                    0x280B001,
                    0,
                    0);
                Vec3Lerp(start, end, trace.fraction, contact);
                centerW = Vec3Distance(start, contact);
                if ( centerW >= 10000.0 )
                    v1 = 10000.0f;
                else
                    v1 = centerW;
                centerW = v1;
            }
            CL_DrawStretchPicW(
                &scrPlaceView[localClientNum],
                x,
                y,
                centerW,
                w,
                h,
                horzAlign,
                vertAlign,
                0.0,
                0.0,
                1.0,
                1.0,
                color,
                material);
        }
    }
}

float s_arcEpsilon =
0.0099999998;

double __cdecl CG_GetVehicleCrossHairAlpha(int localClientNum)
{
    const vehicle_info_t *info; // [esp+10h] [ebp-30h]
    const cg_s *cgameGlob; // [esp+14h] [ebp-2Ch]
    int weapon; // [esp+18h] [ebp-28h]
    centity_s *cent; // [esp+1Ch] [ebp-24h]
    int gunnerIndex; // [esp+20h] [ebp-20h]
    centity_s *vehicle; // [esp+24h] [ebp-1Ch]
    const WeaponDef *weapDef; // [esp+28h] [ebp-18h]
    float angleDiff; // [esp+30h] [ebp-10h]
    float angleDiff_4; // [esp+34h] [ebp-Ch]
    int seatIndex; // [esp+3Ch] [ebp-4h]

    if ( CG_IsVehicleMayhemGameType() )
        return 1.0;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    weapon = CG_GetPlayerWeapon(&cgameGlob->predictedPlayerState, localClientNum);
    weapDef = BG_GetWeaponDef(weapon);
    vehicle = CG_GetEntity(localClientNum, cgameGlob->predictedPlayerEntity.nextState.otherEntityNum);
    cent = CG_GetEntity(localClientNum, cgameGlob->predictedPlayerState.viewlocked_entNum);
    info = CG_GetVehicleInfo(cent->nextState.vehicleState.vehicleInfoIndex);
    seatIndex = cgameGlob->predictedPlayerState.vehiclePos;
    gunnerIndex = -1;
    if ( seatIndex < 1 || seatIndex > 4 )
    {
        if ( !seatIndex && !cent->nextState.weapon && info->driverControlledGunPos >= 0 )
            gunnerIndex = info->driverControlledGunPos;
        if ( seatIndex || !cent->nextState.weapon )
        {
            if ( gunnerIndex < 0 )
                return 0.0;
            angleDiff_4 = (float)vehicle->currentState.u.vehicle.gunnerAngles[gunnerIndex].yaw * 0.0054931641;
            angleDiff = (float)vehicle->currentState.u.vehicle.gunnerAngles[gunnerIndex].pitch * 0.0054931641;
        }
        else
        {
            angleDiff_4 = (float)vehicle->currentState.u.vehicle.gunYaw * 0.0054931641;
            angleDiff = (float)vehicle->currentState.u.vehicle.gunPitch * 0.0054931641;
        }
        if ( (float)(angleDiff_4 + s_arcEpsilon) < weapDef->leftArc
            //&& COERCE_FLOAT(LODWORD(weapDef->rightArc) ^ _mask__NegFloat_) < (float)(angleDiff_4 - s_arcEpsilon) )
            -(weapDef->rightArc) < (float)(angleDiff_4 - s_arcEpsilon) )
        {
            if ( (float)(angleDiff + s_arcEpsilon) < weapDef->bottomArc
                //&& COERCE_FLOAT(LODWORD(weapDef->topArc) ^ _mask__NegFloat_) < (float)(angleDiff - s_arcEpsilon) )
                && -weapDef->topArc < (float)(angleDiff - s_arcEpsilon) )
            {
                return 1.0;
            }
            else
            {
                return 0.0;
            }
        }
        else
        {
            return 0.0;
        }
    }
    else if ( *(_WORD *)&info->gunnerWeapon[3][2 * seatIndex + 62] )
    {
        return 1.0;
    }
    else
    {
        return 0.0;
    }
}

bool __cdecl AllowedToDrawCrosshair(int localClientNum, const playerState_s *predictedPlayerState)
{
    if ( !predictedPlayerState
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_draw_reticles.cpp",
                    840,
                    0,
                    "%s",
                    "predictedPlayerState") )
    {
        __debugbreak();
    }
    if ( !cg_paused
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_draw_reticles.cpp", 841, 0, "%s", "cg_paused") )
    {
        __debugbreak();
    }
    if ( !CG_IsShowingZombieMap() && cg_paused->current.integer && cg_drawpaused->current.enabled )
        return 0;
    if ( CG_IsReticleTurnedOff(localClientNum) )
        return 0;
    if ( Demo_IsGameHudHidden() )
        return 0;
    if ( (predictedPlayerState->locationSelectionInfo & 0xF) != 0 )
        return 0;
    if ( predictedPlayerState->weaponstate == 17
        || predictedPlayerState->weaponstate == 18
        || predictedPlayerState->weaponstate == 19 )
    {
        return 0;
    }
    if ( BG_GetWeaponDef(predictedPlayerState->weapon)->bDualWield )
    {
        if ( predictedPlayerState->weaponstate == 10 && predictedPlayerState->weaponstateLeft == 10 )
            return 0;
    }
    else if ( predictedPlayerState->weaponstate == 10 )
    {
        return 0;
    }
    return predictedPlayerState->weaponstate != 1
            && predictedPlayerState->weaponstate != 2
            && predictedPlayerState->weaponstate != 3
            && predictedPlayerState->weaponstate != 4
            && predictedPlayerState->weaponstate != 5;
}

bool __cdecl CG_IsReticleTurnedOff(int localClientNum)
{
    return IsHardcoreMode(localClientNum) || !cg_drawCrosshair->current.enabled || !UI_ShouldDrawCrosshair();
}

void __cdecl CG_DrawAdsAimIndicator(
                int localClientNum,
                const WeaponDef *weapDef,
                const float *color,
                float centerX,
                float centerY,
                float transScale)
{
    Material *material; // [esp+30h] [ebp-1Ch]
    float h; // [esp+44h] [ebp-8h]

    if ( !weapDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_draw_reticles.cpp", 896, 0, "%s", "weapDef") )
    {
        __debugbreak();
    }
    if ( !cg_drawGun
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_draw_reticles.cpp", 897, 0, "%s", "cg_drawGun") )
    {
        __debugbreak();
    }
    if ( !cg_drawGun->current.enabled && transScale < 1.0 )
    {
        material = weapDef->reticleCenter;
        if ( material )
        {
            h = (float)(1.5 - transScale) * (float)weapDef->iReticleCenterSize;
            CL_DrawStretchPic(
                &scrPlaceView[localClientNum],
                centerX - (float)(0.5 * h),
                centerY - (float)(0.5 * h),
                h,
                h,
                2,
                2,
                0.0,
                0.0,
                1.0,
                1.0,
                color,
                material);
        }
    }
}

void __cdecl CG_TransitionToAds(
                const cg_s *cgameGlob,
                const WeaponDef *weapDef,
                float posLerp,
                float *transScale,
                float *transShift)
{
    double v5; // xmm0_8
    long double v6; // [esp+8h] [ebp-Ch]
    float f; // [esp+10h] [ebp-4h]
    float fa; // [esp+10h] [ebp-4h]
    float fb; // [esp+10h] [ebp-4h]

    if ( !cgameGlob
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_draw_reticles.cpp", 930, 0, "%s", "cgameGlob") )
    {
        __debugbreak();
    }
    if ( !weapDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_draw_reticles.cpp", 931, 0, "%s", "weapDef") )
    {
        __debugbreak();
    }
    if ( !transScale
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_draw_reticles.cpp", 932, 0, "%s", "transScale") )
    {
        __debugbreak();
    }
    if ( !transShift
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_draw_reticles.cpp", 933, 0, "%s", "transShift") )
    {
        __debugbreak();
    }
    if ( cgameGlob->playerEntity.bPositionToADS )
    {
        f = posLerp - (float)(1.0 - weapDef->fAdsCrosshairInFrac);
        if ( f <= 0.0 )
            return;
        if ( weapDef->fAdsCrosshairInFrac == 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_draw_reticles.cpp",
                        943,
                        0,
                        "%s\n\t(weapDef->fAdsCrosshairInFrac) = %g",
                        "(weapDef->fAdsCrosshairInFrac != 0.0f)",
                        weapDef->fAdsCrosshairInFrac) )
        {
            __debugbreak();
        }
        fa = f / weapDef->fAdsCrosshairInFrac;
    }
    else
    {
        fb = posLerp - (float)(1.0 - weapDef->fAdsCrosshairOutFrac);
        if ( fb <= 0.0 )
            return;
        if ( weapDef->fAdsCrosshairOutFrac == 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_draw_reticles.cpp",
                        952,
                        0,
                        "%s\n\t(weapDef->fAdsCrosshairOutFrac) = %g",
                        "(weapDef->fAdsCrosshairOutFrac != 0.0f)",
                        weapDef->fAdsCrosshairOutFrac) )
        {
            __debugbreak();
        }
        fa = fb / weapDef->fAdsCrosshairOutFrac;
    }
    if ( cgameGlob->refdef.tanHalfFovY == 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_draw_reticles.cpp",
                    956,
                    0,
                    "%s\n\t(cgameGlob->refdef.tanHalfFovY) = %g",
                    "(cgameGlob->refdef.tanHalfFovY != 0.0f)",
                    cgameGlob->refdef.tanHalfFovY) )
    {
        __debugbreak();
    }
    *transScale = 1.0 - (float)(fa * 0.5);
    //*((float *)&v6 + 1) = weapDef->fAdsAimPitch * 0.017453292;
    //v5 = *((float *)&v6 + 1);
    //__libm_sse2_tan(v6);
    //*(float *)&v5 = v5;
    //*transShift = (float)((float)(fa * 240.0) / cgameGlob->refdef.tanHalfFovY) * *(float *)&v5;
    *transShift = (float)((float)(fa * 240.0) / cgameGlob->refdef.tanHalfFovY) * tan(DEG2RAD(weapDef->fAdsAimPitch));
}

void __cdecl CG_DrawReticleCenter(
                int localClientNum,
                const WeaponDef *weapDef,
                const float *color,
                float centerX,
                float centerY,
                float centerW,
                float transScale)
{
    Material *material; // [esp+34h] [ebp-20h]
    int grenadeTimeLeft; // [esp+38h] [ebp-1Ch]
    float drawSize; // [esp+40h] [ebp-14h]

    if ( !weapDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_draw_reticles.cpp", 977, 0, "%s", "weapDef") )
    {
        __debugbreak();
    }
    material = weapDef->reticleCenter;
    if ( material )
    {
        drawSize = (float)weapDef->iReticleCenterSize;
        if ( (weapDef->weapType == WEAPTYPE_GRENADE || weapDef->weapType == WEAPTYPE_MINE) && weapDef->bCookOffHold )
        {
            grenadeTimeLeft = CG_GetLocalClientGlobals(localClientNum)->predictedPlayerState.grenadeTimeLeft;
            if ( grenadeTimeLeft )
                drawSize = (float)((float)(grenadeTimeLeft % 1000) / 100.0) + drawSize;
        }
        CL_DrawStretchPicW(
            &scrPlaceView[localClientNum],
            centerX - (float)(0.5 * (float)(drawSize * transScale)),
            centerY - (float)(0.5 * (float)(drawSize * transScale)),
            centerW,
            drawSize * transScale,
            drawSize * transScale,
            2,
            2,
            0.0,
            0.0,
            1.0,
            1.0,
            color,
            material);
    }
}

void __cdecl CG_DrawReticleSides(
                int localClientNum,
                unsigned int weaponIndex,
                const float *baseColor,
                float centerX,
                float centerY,
                float centerW,
                float transScale)
{
    float fOverlayAlphaScale; // [esp+28h] [ebp-5Ch]
    float spread[2]; // [esp+30h] [ebp-54h] BYREF
    const ScreenPlacement *scrPlace; // [esp+38h] [ebp-4Ch]
    const cg_s *cgameGlob; // [esp+3Ch] [ebp-48h]
    Material *material; // [esp+40h] [ebp-44h]
    float drawPos[2]; // [esp+44h] [ebp-40h]
    int vertAlign; // [esp+4Ch] [ebp-38h]
    float imageTexelOffset[2]; // [esp+50h] [ebp-34h] BYREF
    float reticleAlpha; // [esp+58h] [ebp-2Ch]
    float drawSize[2]; // [esp+5Ch] [ebp-28h] BYREF
    const WeaponVariantDef *weapVariantDef; // [esp+64h] [ebp-20h]
    int horzAlign; // [esp+68h] [ebp-1Ch]
    float reticleColor[4]; // [esp+6Ch] [ebp-18h] BYREF
    const WeaponDef *weapDef; // [esp+7Ch] [ebp-8h]
    float zoomFrac; // [esp+80h] [ebp-4h] BYREF

    weapVariantDef = BG_GetWeaponVariantDef(weaponIndex);
    weapDef = BG_GetWeaponDef(weaponIndex);
    if ( !weapVariantDef
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_draw_reticles.cpp",
                    1090,
                    0,
                    "%s",
                    "weapVariantDef") )
    {
        __debugbreak();
    }
    if ( !weapDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_draw_reticles.cpp", 1091, 0, "%s", "weapDef") )
    {
        __debugbreak();
    }
    material = weapDef->reticleSide;
    if ( material )
    {
        horzAlign = 2;
        vertAlign = 2;
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        if ( CG_GetWeapReticleZoom(cgameGlob, &zoomFrac) )
        {
            if ( weapVariantDef->fOverlayAlphaScale >= 1.0 )
                fOverlayAlphaScale = weapVariantDef->fOverlayAlphaScale;
            else
                fOverlayAlphaScale = 1.0f;
            reticleAlpha = 1.0 - (float)(zoomFrac / fOverlayAlphaScale);
        }
        else
        {
            reticleAlpha = 1.0f;
        }
        CG_CalcReticleColor(baseColor, reticleAlpha, cgameGlob->predictedPlayerState.aimSpreadScale, reticleColor);
        if ( reticleColor[3] >= 0.0099999998 )
        {
            drawSize[0] = (float)weapDef->iReticleSideSize * transScale;
            drawSize[1] = drawSize[0];
            CG_CalcReticleSpread(cgameGlob, weapDef, drawSize, transScale, spread);
            scrPlace = &scrPlaceView[localClientNum];
            CG_CalcReticleImageOffset(drawSize, imageTexelOffset);
            drawPos[0] = centerX - (float)(drawSize[0] * 0.5);
            drawPos[1] = (float)((float)(centerY - drawSize[1]) - spread[1]) - imageTexelOffset[1];
            CG_DrawRotatedPicW(
                scrPlace,
                drawPos[0],
                drawPos[1],
                centerW,
                drawSize[0],
                drawSize[1],
                horzAlign,
                vertAlign,
                0.0,
                reticleColor,
                material);
            drawPos[0] = centerX + spread[0];
            drawPos[1] = centerY - (float)(drawSize[1] * 0.5);
            CG_DrawRotatedPicW(
                scrPlace,
                centerX + spread[0],
                drawPos[1],
                centerW,
                drawSize[0],
                drawSize[1],
                horzAlign,
                vertAlign,
                90.0,
                reticleColor,
                material);
            drawPos[0] = (float)(centerX - (float)(drawSize[0] * 0.5)) - imageTexelOffset[0];
            drawPos[1] = centerY + spread[1];
            CG_DrawRotatedPicW(
                scrPlace,
                drawPos[0],
                centerY + spread[1],
                centerW,
                drawSize[0],
                drawSize[1],
                horzAlign,
                vertAlign,
                180.0,
                reticleColor,
                material);
            CG_DrawRotatedPicW(
                scrPlace,
                (float)((float)(centerX - drawSize[0]) - spread[0]) - imageTexelOffset[0],
                (float)(centerY - (float)(drawSize[1] * 0.5)) - imageTexelOffset[1],
                centerW,
                drawSize[0],
                drawSize[1],
                horzAlign,
                vertAlign,
                270.0,
                reticleColor,
                material);
        }
    }
}

void __cdecl CG_CalcReticleSpread(
                const cg_s *cgameGlob,
                const WeaponDef *weapDef,
                const float *drawSize,
                float transScale,
                float *spread)
{
    double v5; // xmm0_8
    long double v6; // [esp+0h] [ebp-14h]
    float maxSpread; // [esp+8h] [ebp-Ch] BYREF
    float f; // [esp+Ch] [ebp-8h] BYREF
    float scale; // [esp+10h] [ebp-4h]

    if ( !weapDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_draw_reticles.cpp", 1016, 0, "%s", "weapDef") )
    {
        __debugbreak();
    }
    BG_GetSpreadForWeapon(&cgameGlob->predictedPlayerState, weapDef, &f, &maxSpread);
    f = (float)((float)((float)(maxSpread - f) * (float)(cgameGlob->predictedPlayerState.aimSpreadScale / 255.0)) + f)
        * transScale;
    //*((float *)&v6 + 1) = f * 0.017453292;
    //v5 = (float)(f * 0.017453292);
    //__libm_sse2_tan(v6);
    //*(float *)&v5 = v5;
    scale = (float)(240.0 * tan(DEG2RAD(f))) / cgameGlob->refdef.tanHalfFovY;
    if ( (float)weapDef->iReticleMinOfs > scale )
        scale = (float)weapDef->iReticleMinOfs;
    *spread = scale - (float)(weapDef->fHipReticleSidePos * *drawSize);
    spread[1] = scale - (float)(weapDef->fHipReticleSidePos * drawSize[1]);
}

void __cdecl CG_CalcReticleColor(const float *baseColor, float alpha, float aimSpreadScale, float *reticleColor)
{
    if ( !cg_crosshairAlpha
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_draw_reticles.cpp",
                    1040,
                    0,
                    "%s",
                    "cg_crosshairAlpha") )
    {
        __debugbreak();
    }
    if ( !cg_crosshairAlphaMin
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_draw_reticles.cpp",
                    1041,
                    0,
                    "%s",
                    "cg_crosshairAlphaMin") )
    {
        __debugbreak();
    }
    if ( (alpha < 0.0 || alpha > 1.0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_draw_reticles.cpp",
                    1042,
                    0,
                    "%s\n\t(alpha) = %g",
                    "(alpha >= 0 && alpha <= 1.0f)",
                    alpha) )
    {
        __debugbreak();
    }
    *reticleColor = *baseColor;
    reticleColor[1] = baseColor[1];
    reticleColor[2] = baseColor[2];
    reticleColor[3] = (float)(alpha * cg_crosshairAlpha->current.value) * (float)(1.0 - (float)(aimSpreadScale / 255.0));
    if ( cg_crosshairAlphaMin->current.value > reticleColor[3] )
        reticleColor[3] = cg_crosshairAlphaMin->current.value;
    if ( (reticleColor[3] < 0.0 || reticleColor[3] > 1.0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_draw_reticles.cpp",
                    1054,
                    1,
                    "%s\n\t(reticleColor[3]) = %g",
                    "(reticleColor[3] >= 0 && reticleColor[3] <= 1.0f)",
                    reticleColor[3]) )
    {
        __debugbreak();
    }
}

void __cdecl CG_CalcReticleImageOffset(const float *drawSize, float *imageTexelOffset)
{
    *imageTexelOffset = *drawSize / 8.0;
    imageTexelOffset[1] = drawSize[1] / 8.0;
}

