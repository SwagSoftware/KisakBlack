#include "cg_draw_indicators.h"
#include <demo/demo_playback.h>
#include <client_mp/cl_cgame_mp.h>
#include <ui/ui_atoms.h>
#include "cg_draw_reticles.h"
#include <cgame_mp/cg_main_mp.h>
#include "cg_drawtools.h"
#include <cgame_mp/cg_view_mp.h>
#include <cgame_mp/cg_draw_mp.h>
#include <cgame_mp/cg_ents_mp.h>
#include <bgame/bg_weapons_def.h>
#include <bgame/bg_misc.h>
#include <gfx_d3d/r_rendercmds.h>

unsigned int g_hudGrenadeCount;
HudGrenade g_hudGrenades[32];

void __cdecl CG_DrawFlashDamage(const cg_s *cgameGlob)
{
    int displayHeight; // [esp+18h] [ebp-24h] BYREF
    float sidebuffer; // [esp+1Ch] [ebp-20h]
    float displayAspect; // [esp+20h] [ebp-1Ch] BYREF
    int displayWidth; // [esp+24h] [ebp-18h] BYREF
    float redFlash; // [esp+28h] [ebp-14h]
    float col[4]; // [esp+2Ch] [ebp-10h] BYREF

    if ( cgameGlob->nextSnap
        && cgameGlob->cameraMode != 1
        && !Demo_IsThirdPersonCamera()
        && !Demo_IsMovieCamera()
        && cgameGlob->v_dmg_time > cgameGlob->time )
    {
        redFlash = fabs((float)((float)(cgameGlob->v_dmg_time - cgameGlob->time) * cgameGlob->v_dmg_pitch) / 500.0);
        if ( redFlash > 5.0 )
            redFlash = 5.0f;
        col[0] = 0.2f;
        col[1] = 0.0f;
        col[2] = 0.0f;
        col[3] = (float)(redFlash / 5.0) * 0.69999999;
        sidebuffer = 10.0f;
        CL_GetScreenDimensions(&displayWidth, &displayHeight, &displayAspect);
        UI_FillRectPhysical(
            -(sidebuffer),
            -(sidebuffer),
            (float)displayWidth + sidebuffer,
            (float)displayHeight + sidebuffer,
            col);
    }
}

void __cdecl CG_DrawDamageDirectionIndicators(int localClientNum)
{
    float v1; // [esp+18h] [ebp-80h]
    float v2; // [esp+2Ch] [ebp-6Ch]
    float v3; // [esp+2Ch] [ebp-6Ch]
    float v4; // [esp+30h] [ebp-68h]
    float v5; // [esp+30h] [ebp-68h]
    float value; // [esp+34h] [ebp-64h]
    float v7; // [esp+34h] [ebp-64h]
    int t; // [esp+38h] [ebp-60h]
    const ScreenPlacement *scrPlace; // [esp+3Ch] [ebp-5Ch]
    float xy[4][2]; // [esp+40h] [ebp-58h] BYREF
    const cg_s *cgameGlob; // [esp+60h] [ebp-38h]
    int slot; // [esp+64h] [ebp-34h]
    float halfWidth; // [esp+68h] [ebp-30h]
    float yaw; // [esp+6Ch] [ebp-2Ch]
    float height; // [esp+70h] [ebp-28h]
    float angle; // [esp+74h] [ebp-24h] BYREF
    float radius; // [esp+78h] [ebp-20h]
    int maxTime; // [esp+7Ch] [ebp-1Ch]
    float centerY; // [esp+80h] [ebp-18h] BYREF
    float color[4]; // [esp+84h] [ebp-14h] BYREF
    float centerX; // [esp+94h] [ebp-4h] BYREF

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->nextSnap
        && cgameGlob->cameraMode != 1
        && !Demo_IsThirdPersonCamera()
        && !Demo_IsMovieCamera()
        && !CG_Flashbanged(localClientNum)
        && (cgameGlob->predictedPlayerState.eFlags2 & 0x10000000) == 0 )
    {
        scrPlace = &scrPlaceView[localClientNum];
        if ( CG_GetWeapReticleZoom(cgameGlob, &angle) )
        {
            if ( !cg_hudDamageIconInScope->current.enabled )
                return;
            CG_CalcCrosshairPosition(cgameGlob, &centerX, &centerY);
            centerX = ScrPlace_ApplyX(scrPlace, centerX, 2);
            centerY = ScrPlace_ApplyY(scrPlace, centerY, 2);
        }
        else
        {
            centerX = ScrPlace_ApplyX(scrPlace, 0.0, 2);
            centerY = ScrPlace_ApplyY(scrPlace, 0.0, 2);
        }
        value = cg_hudDamageIconWidth->current.value;
        v7 = ScrPlace_HiResGetScale() * value;
        halfWidth = (float)(v7 * scrPlace->scaleVirtualToReal[0]) * 0.5;
        v4 = cg_hudDamageIconHeight->current.value;
        v5 = ScrPlace_HiResGetScale() * v4;
        height = v5 * scrPlace->scaleVirtualToReal[1];
        v2 = cg_hudDamageIconOffset->current.value;
        v3 = ScrPlace_HiResGetScale() * v2;
        radius = v3 * scrPlace->scaleVirtualToReal[1];
        LODWORD(xy[0][0]) = halfWidth;
        xy[0][1] = radius + height;
        //LODWORD(xy[1][0]) = halfWidth ^ _mask__NegFloat_;
        xy[1][0] = -halfWidth;
        xy[1][1] = radius + height;
        //LODWORD(xy[2][0]) = halfWidth ^ _mask__NegFloat_;
        xy[2][0] = -halfWidth;
        LODWORD(xy[2][1]) = radius;
        *(_QWORD *)&xy[3][0] = __PAIR64__(radius, halfWidth);
        color[0] = colorWhite[0];
        color[1] = 1.0;
        color[2] = 1.0;
        color[3] = 1.0;
        for ( slot = 0; slot < 8; ++slot )
        {
            maxTime = cgameGlob->viewDamage[slot].duration;
            t = cgameGlob->time - cgameGlob->viewDamage[slot].time;
            if ( t > 0 && t < maxTime )
            {
                yaw = vectoyaw(cgameGlob->refdef.viewaxis[0]);
                angle = yaw - cgameGlob->viewDamage[slot].yaw;
                if ( (float)((float)(2.0 - (float)((float)((float)t * 2.0) / (float)maxTime)) - 1.0) < 0.0 )
                    v1 = 2.0 - (float)((float)((float)t * 2.0) / (float)maxTime);
                else
                    v1 = 1.0f;
                color[3] = v1;
                CG_DrawRotatedQuadPic(scrPlace, centerX, centerY, xy, angle, color, cgMedia.damageMaterial);
            }
        }
    }
}

void __cdecl CG_ClearHudGrenades()
{
    g_hudGrenadeCount = 0;
}

double __cdecl CG_AddHudGrenade_PositionCheck(cg_s *cgameGlob, const centity_s *grenadeEnt, const WeaponDef *weapDef)
{
    float v5; // [esp+4h] [ebp-58h]
    float iExplosionRadius; // [esp+Ch] [ebp-50h]
    float v8; // [esp+14h] [ebp-48h]
    float grenadeOffset; // [esp+30h] [ebp-2Ch]
    float grenadeOffset_4; // [esp+34h] [ebp-28h]
    float grenadeOffset_8; // [esp+38h] [ebp-24h]
    float maxOffsetSquared; // [esp+3Ch] [ebp-20h]
    float playerOrigin[3]; // [esp+48h] [ebp-14h] BYREF
    float grenadeOffsetSquared; // [esp+54h] [ebp-8h]
    float maxOffset; // [esp+58h] [ebp-4h]

    GetCeilingHeight(cgameGlob);
    if ( !grenadeEnt
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_draw_indicators.cpp", 893, 0, "%s", "grenadeEnt") )
    {
        __debugbreak();
    }
    CG_GrenadeIndicator_GetPlayerPosition(cgameGlob, playerOrigin);
    grenadeOffset = grenadeEnt->pose.origin[0] - playerOrigin[0];
    grenadeOffset_4 = grenadeEnt->pose.origin[1] - playerOrigin[1];
    grenadeOffset_8 = grenadeEnt->pose.origin[2] - playerOrigin[2];
    if ( weapDef->projExplosion == WEAPPROJEXP_GRENADE
        || weapDef->projExplosion == WEAPPROJEXP_HEAVY
        || weapDef->projExplosion == WEAPPROJEXP_FIRE
        || weapDef->bShowIndicator )
    {
        if ( IsHardcoreMode(cgameGlob->localClientNum) )
        {
            maxOffset = 0.0f;
        }
        else if ( weapDef->iIndicatorRadius )
        {
            maxOffset = (float)weapDef->iIndicatorRadius;
        }
        else if ( cg_hudGrenadeIconMaxRangeFrag->current.value == 0.0 )
        {
            maxOffset = 0.0f;
        }
        else
        {
            if ( (float)(cg_hudGrenadeIconMaxRangeFrag->current.value - (float)weapDef->iExplosionRadius) < 0.0 )
                iExplosionRadius = (float)weapDef->iExplosionRadius;
            else
                iExplosionRadius = cg_hudGrenadeIconMaxRangeFrag->current.value;
            maxOffset = iExplosionRadius;
        }
    }
    else
    {
        if ( weapDef->projExplosion != WEAPPROJEXP_FLASHBANG
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_draw_indicators.cpp",
                        917,
                        0,
                        "%s",
                        "weapDef->projExplosion == WEAPPROJEXP_FLASHBANG") )
        {
            __debugbreak();
        }
        maxOffset = cg_hudGrenadeIconMaxRangeFlash->current.value;
    }
    maxOffsetSquared = maxOffset * maxOffset;
    grenadeOffsetSquared = (float)((float)(grenadeOffset * grenadeOffset) + (float)(grenadeOffset_4 * grenadeOffset_4))
                                             + (float)(grenadeOffset_8 * grenadeOffset_8);
    if ( grenadeOffsetSquared > (float)(maxOffset * maxOffset) )
        return 0.0;
    if ( grenadeOffset_8 >= 0.0 )
    {
        if ( weapDef->stickiness != WEAPSTICKINESS_ALL && weapDef->stickiness != WEAPSTICKINESS_ALL_NO_SENTIENTS )
        {
            v5 = (float)((float)(cgameGlob->heightToCeiling + 70.0)
                                 - (float)(cg_hudGrenadeIconMaxHeight->current.value + cgameGlob->predictedPlayerState.viewHeightCurrent)) < 0.0
                 ? cgameGlob->heightToCeiling + 70.0
                 : cg_hudGrenadeIconMaxHeight->current.value + cgameGlob->predictedPlayerState.viewHeightCurrent;
            if ( grenadeOffset_8 > v5 )
                return 0.0;
        }
    }
    //else if ( COERCE_FLOAT(cg_hudGrenadeIconMaxHeight->current.integer ^ _mask__NegFloat_) > grenadeOffset_8 )
    else if ( -cg_hudGrenadeIconMaxHeight->current.value > grenadeOffset_8 )
    {
        return 0.0;
    }
    if ( (float)((float)(grenadeOffsetSquared / maxOffsetSquared) - 1.0) < 0.0 )
        v8 = grenadeOffsetSquared / maxOffsetSquared;
    else
        v8 = 1.0f;
    if ( (float)(0.001 - (float)(grenadeOffsetSquared / maxOffsetSquared)) < 0.0 )
        return v8;
    else
        return 0.001f;
}

void __cdecl CG_GrenadeIndicator_GetPlayerPosition(const cg_s *cgameGlob, float *playerOrigin)
{
    float *EntityOrigin; // [esp+0h] [ebp-8h]

    *playerOrigin = cgameGlob->predictedPlayerState.origin[0];
    playerOrigin[1] = cgameGlob->predictedPlayerState.origin[1];
    playerOrigin[2] = cgameGlob->predictedPlayerState.origin[2];
    if ( (cgameGlob->predictedPlayerState.eFlags & 0x4000) != 0
        && (cgameGlob->predictedPlayerState.eFlags2 & 0x10000000) != 0 )
    {
        EntityOrigin = CG_GetEntityOrigin(
                                         cgameGlob->localClientNum,
                                         cgameGlob->bgs.clientinfo[cgameGlob->nextSnap->ps.clientNum].attachedVehEntNum);
        *playerOrigin = *EntityOrigin;
        playerOrigin[1] = EntityOrigin[1];
        playerOrigin[2] = EntityOrigin[2];
    }
}

void __cdecl CG_AddHudGrenade(cg_s *cgameGlob, const centity_s *grenadeEnt)
{
    float v2; // [esp+4h] [ebp-44h]
    float v3; // [esp+8h] [ebp-40h]
    float v4; // [esp+Ch] [ebp-3Ch]
    HudGrenade *v5; // [esp+10h] [ebp-38h]
    const entityState_s *state; // [esp+2Ch] [ebp-1Ch]
    int globalClientNum; // [esp+30h] [ebp-18h]
    Material *material; // [esp+34h] [ebp-14h]
    float maxSpeedSq; // [esp+38h] [ebp-10h]
    float danger_ratio; // [esp+3Ch] [ebp-Ch]
    int grenadeParentClientNum; // [esp+40h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+44h] [ebp-4h]

    state = &grenadeEnt->nextState;
    if ( grenadeEnt->nextState.eType != 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_draw_indicators.cpp",
                    963,
                    0,
                    "%s\n\t(state->eType) = %i",
                    "(state->eType == ET_MISSILE)",
                    grenadeEnt->nextState.eType) )
    {
        __debugbreak();
    }
    if ( ((LODWORD(grenadeEnt->nextState.lerp.pos.trDelta[0]) & 0x7F800000) == 0x7F800000
         || (LODWORD(grenadeEnt->nextState.lerp.pos.trDelta[1]) & 0x7F800000) == 0x7F800000
         || (LODWORD(grenadeEnt->nextState.lerp.pos.trDelta[2]) & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_draw_indicators.cpp",
                    965,
                    0,
                    "%s",
                    "!IS_NAN((state->lerp.pos.trDelta)[0]) && !IS_NAN((state->lerp.pos.trDelta)[1]) && !IS_NAN((state->lerp.pos.trDelta)[2])") )
    {
        __debugbreak();
    }
    if ( (!grenadeEnt->linkInfo || grenadeEnt->linkInfo->linkEnt != cgameGlob->predictedPlayerState.clientNum)
        && !grenadeEnt->nextState.lerp.u.turret.ownerNum )
    {
        globalClientNum = cgameGlob->clientNum;
        grenadeParentClientNum = grenadeEnt->nextState.lerp.u.loopFx.period;
        weapDef = BG_GetWeaponDef(grenadeEnt->nextState.weapon);
        if ( (weapDef->weapType == WEAPTYPE_GRENADE
             || weapDef->weapType == WEAPTYPE_MINE
             || weapDef->weapType == WEAPTYPE_PROJECTILE)
            && (weapDef->projExplosion == WEAPPROJEXP_GRENADE
             || weapDef->projExplosion == WEAPPROJEXP_FIRE
             || weapDef->projExplosion == WEAPPROJEXP_FLASHBANG
             || weapDef->projExplosion == WEAPPROJEXP_HEAVY
             || weapDef->bShowIndicator)
            && weapDef->timedDetonation )
        {
            danger_ratio = CG_AddHudGrenade_PositionCheck(cgameGlob, grenadeEnt, weapDef);
            if ( danger_ratio != 0.0
                && (cgameGlob->bgs.clientinfo[grenadeParentClientNum].team != cgameGlob->bgs.clientinfo[globalClientNum].team
                 || cgameGlob->bgs.clientinfo[globalClientNum].team == TEAM_FREE
                 || grenadeParentClientNum == globalClientNum
                 || BG_GetFriendlyFireStatus()) )
            {
                maxSpeedSq = weapDef->projExplosion == WEAPPROJEXP_GRENADE
                                    || weapDef->projExplosion == WEAPPROJEXP_HEAVY
                                    || weapDef->projExplosion == WEAPPROJEXP_FIRE
                                     ? bg_maxGrenadeIndicatorSpeed->current.value * bg_maxGrenadeIndicatorSpeed->current.value
                                     : 1.0f;
                if ( (grenadeEnt->nextState.clientLinkInfo.parentEnt
                     || (float)((float)((float)(state->lerp.pos.trDelta[0] * state->lerp.pos.trDelta[0])
                                                        + (float)(state->lerp.pos.trDelta[1] * state->lerp.pos.trDelta[1]))
                                        + (float)(state->lerp.pos.trDelta[2] * state->lerp.pos.trDelta[2])) <= maxSpeedSq)
                    && g_hudGrenadeCount < 0x20 )
                {
                    if ( weapDef->projExplosion == WEAPPROJEXP_GRENADE
                        || weapDef->projExplosion == WEAPPROJEXP_HEAVY
                        || weapDef->projExplosion == WEAPPROJEXP_FIRE )
                    {
                        if ( cgameGlob->nextSnap->ps.cursorHint >= 8 && cgameGlob->nextSnap->ps.cursorHintEntIndex == state->number )
                        {
                            material = cgMedia.grenadeIconThrowBack;
                        }
                        else if ( weapDef->indicatorIcon )
                        {
                            material = weapDef->indicatorIcon;
                        }
                        else
                        {
                            material = cgMedia.grenadeIconFrag;
                        }
LABEL_48:
                        v5 = &g_hudGrenades[g_hudGrenadeCount];
                        v5->origin[0] = grenadeEnt->pose.origin[0];
                        v5->origin[1] = grenadeEnt->pose.origin[1];
                        v5->origin[2] = grenadeEnt->pose.origin[2];
                        g_hudGrenades[g_hudGrenadeCount].predicted_dmg_ratio = 1.0 - danger_ratio;
                        g_hudGrenades[g_hudGrenadeCount].material = material;
                        if ( grenadeEnt->nextState.lerp.u.actor.team )
                        {
                            v3 = (float)(cgameGlob->time - grenadeEnt->nextState.lerp.u.actor.actorNum)
                                 / (float)grenadeEnt->nextState.lerp.u.actor.team;
                            if ( (float)(v3 - 1.0) < 0.0 )
                                v4 = (float)(cgameGlob->time - grenadeEnt->nextState.lerp.u.actor.actorNum)
                                     / (float)grenadeEnt->nextState.lerp.u.actor.team;
                            else
                                v4 = 1.0f;
                            if ( (float)(0.0 - v3) < 0.0 )
                                v2 = v4;
                            else
                                v2 = 0.0f;
                            g_hudGrenades[g_hudGrenadeCount].predicted_time_ratio = v2;
                        }
                        else
                        {
                            g_hudGrenades[g_hudGrenadeCount].predicted_time_ratio = 1.0f;
                        }
                        ++g_hudGrenadeCount;
                        return;
                    }
                    if ( weapDef->projExplosion != WEAPPROJEXP_FLASHBANG
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_draw_indicators.cpp",
                                    1034,
                                    0,
                                    "%s\n\t(weapDef->projExplosion) = %i",
                                    "(weapDef->projExplosion == WEAPPROJEXP_FLASHBANG)",
                                    weapDef->projExplosion) )
                    {
                        __debugbreak();
                    }
                    if ( cg_hudGrenadeIconEnabledFlash->current.enabled )
                    {
                        material = cgMedia.grenadeIconFlash;
                        goto LABEL_48;
                    }
                }
            }
        }
    }
}

void __cdecl CG_DrawGrenadeIndicators(int localClientNum)
{
    double v1; // xmm0_8
    long double v2; // [esp+18h] [ebp-6Ch]
    float v3; // [esp+20h] [ebp-64h]
    float predicted_dmg_ratio; // [esp+28h] [ebp-5Ch]
    float v5; // [esp+2Ch] [ebp-58h]
    float amplitude; // [esp+3Ch] [ebp-48h]
    float bias; // [esp+40h] [ebp-44h]
    float grenadeOffset[3]; // [esp+48h] [ebp-3Ch] BYREF
    unsigned int entityIndex; // [esp+54h] [ebp-30h]
    const cg_s *cgameGlob; // [esp+58h] [ebp-2Ch]
    float playerOrigin[3]; // [esp+5Ch] [ebp-28h] BYREF
    float angle; // [esp+68h] [ebp-1Ch] BYREF
    float centerY; // [esp+6Ch] [ebp-18h] BYREF
    float color[4]; // [esp+70h] [ebp-14h] BYREF
    float centerX; // [esp+80h] [ebp-4h] BYREF

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->nextSnap
        && cgameGlob->predictedPlayerState.pm_type != 4
        && (cgameGlob->predictedPlayerState.otherFlags & 2) == 0
        && cgameGlob->cameraMode != 1
        && !Demo_IsGameHudHidden()
        && g_hudGrenadeCount )
    {
        if ( CG_GetWeapReticleZoom(cgameGlob, &angle) )
        {
            if ( !cg_hudGrenadeIconInScope->current.enabled )
                return;
            CG_CalcCrosshairPosition(cgameGlob, &centerX, &centerY);
            centerX = ScrPlace_ApplyX(&scrPlaceView[localClientNum], centerX, 2);
            centerY = ScrPlace_ApplyY(&scrPlaceView[localClientNum], centerY, 2);
        }
        else
        {
            centerX = ScrPlace_ApplyX(&scrPlaceView[localClientNum], 0.0, 2);
            centerY = ScrPlace_ApplyY(&scrPlaceView[localClientNum], 0.0, 2);
        }
        CG_GrenadeIndicator_GetPlayerPosition(cgameGlob, playerOrigin);
        for ( entityIndex = 0; entityIndex < g_hudGrenadeCount; ++entityIndex )
        {
            grenadeOffset[0] = g_hudGrenades[entityIndex].origin[0] - playerOrigin[0];
            grenadeOffset[1] = g_hudGrenades[entityIndex].origin[1] - playerOrigin[1];
            grenadeOffset[2] = g_hudGrenades[entityIndex].origin[2] - playerOrigin[2];
            if ( cg_hudGrenadeIndicatorFadeUp->current.enabled )
            {
                Vec4Lerp(
                    &cg_hudGrenadeIndicatorStartColor->current.value,
                    &cg_hudGrenadeIndicatorTargetColor->current.value,
                    g_hudGrenades[entityIndex].predicted_time_ratio,
                    color);
                predicted_dmg_ratio = g_hudGrenades[entityIndex].predicted_dmg_ratio;
                if ( (float)(predicted_dmg_ratio - 1.0) < 0.0 )
                    v5 = g_hudGrenades[entityIndex].predicted_dmg_ratio;
                else
                    v5 = 1.0f;
                if ( (float)(0.1 - predicted_dmg_ratio) < 0.0 )
                    *((float *)&v2 + 1) = v5;
                else
                    *((float *)&v2 + 1) = 0.1f;
                color[3] = *((float *)&v2 + 1);
            }
            else
            {
                color[0] = colorWhite[0];
                color[1] = 1.0;
                color[2] = 1.0;
                color[3] = 1.0;
                amplitude = (float)(cg_hudGrenadePointerPulseMax->current.value - cg_hudGrenadePointerPulseMin->current.value)
                                    * 0.5;
                bias = cg_hudGrenadePointerPulseMin->current.value + amplitude;
                v1 = (float)((float)((float)cgameGlob->time * 0.0062831854) * cg_hudGrenadePointerPulseFreq->current.value);
                //__libm_sse2_sin(v2);
                //*(float *)&v1 = v1;
                v1 = sin(v1);
                //color[3] = (float)(*(float *)&v1 * amplitude) + bias;
                color[3] = (float)(v1 * amplitude) + bias;
            }
            if ( (float)(color[3] - 1.0) < 0.0 )
                v3 = color[3];
            else
                v3 = 1.0f;
            if ( (float)(0.0 - color[3]) < 0.0 )
                *(float *)&v2 = v3;
            else
                LODWORD(v2) = 0;
            color[3] = *(float *)&v2;
            CG_DrawWarningPointer(localClientNum, centerX, centerY, grenadeOffset, color, 0.0);
            CG_DrawWarningIcon(
                localClientNum,
                centerX,
                centerY,
                grenadeOffset,
                color,
                g_hudGrenades[entityIndex].material,
                0.0);
        }
    }
}

void __cdecl CG_DrawWarningPointer(
                int localClientNum,
                float centerX,
                float centerY,
                const float *grenadeOffset,
                const float *color,
                float radiusOffset)
{
    long double v6; // st7
    float angle; // [esp+14h] [ebp-78h]
    float v8; // [esp+18h] [ebp-74h]
    float v9; // [esp+1Ch] [ebp-70h]
    float v10; // [esp+20h] [ebp-6Ch]
    float v11; // [esp+24h] [ebp-68h]
    float v12; // [esp+28h] [ebp-64h]
    float v13; // [esp+2Ch] [ebp-60h]
    const ScreenPlacement *scrPlace; // [esp+3Ch] [ebp-50h]
    float radiusScale; // [esp+40h] [ebp-4Ch]
    const cg_s *cgameGlob; // [esp+44h] [ebp-48h]
    float width; // [esp+48h] [ebp-44h]
    float height; // [esp+4Ch] [ebp-40h]
    float yaw; // [esp+50h] [ebp-3Ch]
    float sinYaw; // [esp+54h] [ebp-38h]
    float pointerPos; // [esp+58h] [ebp-34h]
    float pointerPos_4; // [esp+5Ch] [ebp-30h]
    float cosYaw; // [esp+60h] [ebp-2Ch]
    float pivot; // [esp+64h] [ebp-28h]
    float pivot_4; // [esp+68h] [ebp-24h]
    float grenade_vertices[4][2]; // [esp+6Ch] [ebp-20h] BYREF

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    scrPlace = &scrPlaceView[localClientNum];
    width = cg_hudGrenadePointerWidth->current.value;
    height = cg_hudGrenadePointerHeight->current.value;
    radiusScale = cg_hudGrenadeIconOffset->current.value + radiusOffset;
    pivot = cg_hudGrenadePointerPivot->current.value;
    pivot_4 = cg_hudGrenadePointerPivot->current.vector[1];
    angle = vectoyaw(grenadeOffset) - cgameGlob->predictedPlayerState.viewangles[1];
    yaw = AngleNormalize360(angle);
    v6 = (float)(DEG2RAD(yaw));
    cosYaw = cos(v6);
    sinYaw = sin(v6);
    v13 = ScrPlace_HiResGetScale() * (float)(sinYaw * radiusScale);
    pointerPos = centerX - (float)(v13 * scrPlace->scaleVirtualToReal[0]);
    v12 = ScrPlace_HiResGetScale() * (float)(cosYaw * radiusScale);
    pointerPos_4 = centerY - (float)(v12 * scrPlace->scaleVirtualToReal[1]);
    //v11 = ScrPlace_HiResGetScale() * COERCE_FLOAT(LODWORD(pivot) ^ _mask__NegFloat_);
    v11 = ScrPlace_HiResGetScale() * -pivot;
    grenade_vertices[3][0] = v11 * scrPlace->scaleVirtualToReal[0];
    grenade_vertices[0][0] = grenade_vertices[3][0];
    v10 = ScrPlace_HiResGetScale() * (float)(width - pivot);
    grenade_vertices[2][0] = v10 * scrPlace->scaleVirtualToReal[0];
    grenade_vertices[1][0] = grenade_vertices[2][0];
    //v9 = ScrPlace_HiResGetScale() * COERCE_FLOAT(LODWORD(pivot_4) ^ _mask__NegFloat_);
    v9 = ScrPlace_HiResGetScale() * -pivot_4;
    grenade_vertices[1][1] = v9 * scrPlace->scaleVirtualToReal[1];
    grenade_vertices[0][1] = grenade_vertices[1][1];
    v8 = ScrPlace_HiResGetScale() * (float)(height - pivot_4);
    grenade_vertices[3][1] = v8 * scrPlace->scaleVirtualToReal[1];
    grenade_vertices[2][1] = grenade_vertices[3][1];
    CG_DrawRotatedQuadPic(
        scrPlace,
        pointerPos,
        pointerPos_4,
        grenade_vertices,
        //COERCE_FLOAT(LODWORD(yaw) ^ _mask__NegFloat_),
        -yaw,
        color,
        cgMedia.grenadePointer);
}

void __cdecl CG_DrawWarningIcon(
                int localClientNum,
                float centerX,
                float centerY,
                const float *grenadeOffset,
                const float *color,
                Material *material,
                float radiusOffset)
{
    long double v7; // st7
    float angle; // [esp+0h] [ebp-60h]
    float v9; // [esp+4h] [ebp-5Ch]
    float v10; // [esp+8h] [ebp-58h]
    float v11; // [esp+Ch] [ebp-54h]
    float v12; // [esp+10h] [ebp-50h]
    const ScreenPlacement *scrPlace; // [esp+20h] [ebp-40h]
    float radiusScale; // [esp+24h] [ebp-3Ch]
    const cg_s *cgameGlob; // [esp+28h] [ebp-38h]
    float width; // [esp+2Ch] [ebp-34h]
    float height; // [esp+30h] [ebp-30h]
    float yaw; // [esp+34h] [ebp-2Ch]
    float sinYaw; // [esp+38h] [ebp-28h]
    float cosYaw; // [esp+3Ch] [ebp-24h]
    float grenade_vertices[4][2]; // [esp+40h] [ebp-20h] BYREF

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    scrPlace = &scrPlaceView[localClientNum];
    width = cg_hudGrenadeIconWidth->current.value;
    height = cg_hudGrenadeIconHeight->current.value;
    radiusScale = cg_hudGrenadeIconOffset->current.value + radiusOffset;
    angle = vectoyaw(grenadeOffset) - cgameGlob->predictedPlayerState.viewangles[1];
    yaw = AngleNormalize360(angle);
    v7 = (float)(DEG2RAD(yaw));
    cosYaw = cos(v7);
    sinYaw = sin(v7);
    //v12 = ScrPlace_HiResGetScale() * (float)(COERCE_FLOAT(COERCE_UNSIGNED_INT(width * 0.5) ^ _mask__NegFloat_) - (float)(sinYaw * radiusScale));
    v12 = ScrPlace_HiResGetScale() * (float)(-(width * 0.5)) - (float)(sinYaw * radiusScale);
    grenade_vertices[3][0] = (float)(v12 * scrPlace->scaleVirtualToReal[0]) + centerX;
    grenade_vertices[0][0] = grenade_vertices[3][0];
    v11 = ScrPlace_HiResGetScale() * (float)((float)(width * 0.5) - (float)(sinYaw * radiusScale));
    grenade_vertices[2][0] = (float)(v11 * scrPlace->scaleVirtualToReal[0]) + centerX;
    grenade_vertices[1][0] = grenade_vertices[2][0];
    //v10 = ScrPlace_HiResGetScale() * (float)(COERCE_FLOAT(COERCE_UNSIGNED_INT(height * 0.5) ^ _mask__NegFloat_) - (float)(cosYaw * radiusScale));
    v10 = ScrPlace_HiResGetScale() * (float)(-(height * 0.5) - (float)(cosYaw * radiusScale));
    grenade_vertices[1][1] = (float)(v10 * scrPlace->scaleVirtualToReal[1]) + centerY;
    grenade_vertices[0][1] = grenade_vertices[1][1];
    v9 = ScrPlace_HiResGetScale() * (float)((float)(height * 0.5) - (float)(cosYaw * radiusScale));
    grenade_vertices[3][1] = (float)(v9 * scrPlace->scaleVirtualToReal[1]) + centerY;
    grenade_vertices[2][1] = grenade_vertices[3][1];
    R_AddCmdDrawQuadPic(grenade_vertices, color, material);
}

