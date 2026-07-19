#include "bg_weapons_view.h"
#include "bg_local.h"
#include "bg_public.h"
#include "bg_misc.h"
#include "bg_weapons.h"
#include "bg_weapons_def.h"
#include <cgame/cg_weapons.h>
#include "bg_mantle.h"
#include "bg_perks.h"
#include <universal/com_math_anglevectors.h>
#include <algorithm>

double __cdecl BG_GetBobCycle(const playerState_s *ps)
{
    return (double)(unsigned __int8)ps->bobCycle / 255.0 * 3.1415927
             + (double)(unsigned __int8)ps->bobCycle / 255.0 * 3.1415927
             + 6.2831855;
}

double __cdecl BG_CalculateWeaponMovement_VerticalBobFactor(const playerState_s *ps, float cycle, float speed)
{
    float amplitude;
    const WeaponDef *weapDef = BG_GetWeaponDef(ps->weapon);

    if (ps->waterlevel < 3)
    {
        if (ps->pm_flags & 0x400000)
            amplitude = speed * bg_weaponBobAmplitudeDtp->current.vector[1] * weapDef->vDtpBob[1];
        else if (ps->viewHeightTarget == 11)
            amplitude = speed * bg_weaponBobAmplitudeProne->current.vector[1];
        else if (ps->viewHeightTarget == 40)
            amplitude = (1.0f - ps->fWeaponPosFrac) * speed * bg_weaponBobAmplitudeDucked->current.vector[1];
        else if (ps->pm_flags & 0x8000)
            amplitude = speed * bg_weaponBobAmplitudeSprinting->current.vector[1] * weapDef->vSprintBob[1];
        else
            amplitude = (1.0f - ps->fWeaponPosFrac) * speed * bg_weaponBobAmplitudeStanding->current.vector[1];
    }
    else
    {
        amplitude = bg_weaponBobAmplitudeSwimming->current.vector[1];
    }

    if (amplitude > bg_weaponBobMax->current.value)
        amplitude = bg_weaponBobMax->current.value;

    float sinA = sinf(cycle * 2.0f);
    float sinB = sinf(cycle * 4.0f + 1.5707964f);

    return (sinA + sinB * 0.2f) * 0.75f * amplitude;
}

double __cdecl BG_CalculateWeaponMovement_HorizontalBobFactor(const playerState_s *ps, float cycle, float speed)
{
    float v4; // [esp+0h] [ebp-Ch]
    float amplitude; // [esp+4h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+8h] [ebp-4h]

    weapDef = BG_GetWeaponDef(ps->weapon);
    if ( ps->waterlevel < 3 )
    {
        if ( (ps->pm_flags & 0x400000) != 0 )
        {
            amplitude = (float)(speed * bg_weaponBobAmplitudeDtp->current.value) * weapDef->vDtpBob[0];
        }
        else if ( ps->viewHeightTarget == 11 )
        {
            amplitude = speed * bg_weaponBobAmplitudeProne->current.value;
        }
        else if ( ps->viewHeightTarget == 40 )
        {
            amplitude = (float)((float)(1.0 - ps->fWeaponPosFrac) * speed) * bg_weaponBobAmplitudeDucked->current.value;
        }
        else if ( (ps->pm_flags & 0x8000) != 0 )
        {
            amplitude = (float)(speed * bg_weaponBobAmplitudeSprinting->current.value) * weapDef->vSprintBob[0];
        }
        else
        {
            amplitude = (float)((float)(1.0 - ps->fWeaponPosFrac) * speed) * bg_weaponBobAmplitudeStanding->current.value;
        }
    }
    else
    {
        amplitude = bg_weaponBobAmplitudeSwimming->current.value;
    }
    if ( amplitude > bg_weaponBobMax->current.value )
        amplitude = bg_weaponBobMax->current.value;
    v4 = sin(cycle);
    return v4 * amplitude;
}

void __cdecl BG_CalculateWeaponMovement(weaponState_t *ws, float *origin, float *angles)
{
    BG_CalculateWeaponMovement_Base(ws);
    BG_CalculateWeaponMovement_Sway(ws);
    BG_CalculateWeaponMovement_Recoil(ws);
    BG_CalculateWeaponMovement_Bob(ws);
    if ( angles )
        BG_SetWeaponMovementAngles(ws, angles);
    if ( origin )
        BG_SetWeaponMovementOrigin(ws, origin);
}

void __cdecl BG_CalculateWeaponMovement_Base(weaponState_t *ws)
{
    bool v1; // [esp+Ch] [ebp-A8h]
    bool v2; // [esp+10h] [ebp-A4h]
    bool isCrouched; // [esp+78h] [ebp-3Ch]
    float targetAngles[3]; // [esp+84h] [ebp-30h] BYREF
    float targetPos[3]; // [esp+90h] [ebp-24h] BYREF
    int weapIndex; // [esp+9Ch] [ebp-18h]
    float minSpeed; // [esp+A0h] [ebp-14h]
    const playerState_s *ps; // [esp+A4h] [ebp-10h]
    const WeaponDef *weapDef; // [esp+A8h] [ebp-Ch]
    int isClimbingLadder; // [esp+ACh] [ebp-8h]
    int isProne; // [esp+B0h] [ebp-4h]

    ps = ws->ps;
    weapIndex = BG_GetViewmodelWeaponIndex(ps);
    weapDef = BG_GetWeaponDef(weapIndex);
    isProne = (ps->eFlags & 8) != 0;
    isCrouched = (ps->eFlags & 4) != 0;
    memset(targetPos, 0, sizeof(targetPos));
    memset(targetAngles, 0, sizeof(targetAngles));
    if ( isProne )
    {
        minSpeed = weapDef->fProneMoveMinSpeed;
    }
    else if ( isCrouched )
    {
        minSpeed = weapDef->fDuckedMoveMinSpeed;
    }
    else
    {
        minSpeed = weapDef->fStandMoveMinSpeed;
    }
    if ( minSpeed < 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_view.cpp",
                    631,
                    1,
                    "%s\n\t(minSpeed) = %g",
                    "(minSpeed >= 0.f)",
                    minSpeed) )
    {
        __debugbreak();
    }
    v2 = ws->xyspeed > minSpeed && (float)ps->speed > minSpeed;
    v1 = ps->weaponstate == 10 || ps->weaponstateLeft == 10;
    isClimbingLadder = (ps->pm_flags & 8) != 0;
    if ( isProne || isCrouched )
        BG_CalculateWeaponMovement_Base_Internal_Stance(ws, targetPos, targetAngles);
    if ( v2 && !isClimbingLadder && !v1 )
        BG_CalculateWeaponMovement_Base_Internal_Movement(ws, minSpeed, targetPos, targetAngles);
    BG_CalculateWeaponMovement_Base_Internal_WeaponState(ws, targetPos, targetAngles);
    BG_CalculateWeaponMovement_Base_Internal_Save(ws, targetPos, targetAngles);
}

void __cdecl BG_CalculateWeaponMovement_Base_Internal_WeaponState(
                const weaponState_t *ws,
                float *targetPos,
                float *targetAngles)
{
    float lerp; // [esp+48h] [ebp-18h]
    const float *weaponStateMaxAngles; // [esp+4Ch] [ebp-14h]
    int weapIndex; // [esp+50h] [ebp-10h]
    const float *weaponStateMaxMovement; // [esp+54h] [ebp-Ch]
    const playerState_s *ps; // [esp+58h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+5Ch] [ebp-4h]

    if ( !ws && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_view.cpp", 409, 0, "%s", "ws") )
        __debugbreak();
    ps = ws->ps;
    weapIndex = BG_GetViewmodelWeaponIndex(ws->ps);
    weapDef = BG_GetWeaponDef(weapIndex);
    if ( ps->weaponstate < 27 || ps->weaponstate > 29 )
    {
        if ( ps->weaponstate < 47 || ps->weaponstate > 49 )
        {
            if ( ps->weaponstate < 40 || ps->weaponstate > 42 )
            {
                if ( ps->weaponstate == 43 )
                {
                    lerp = BG_CalculateWeaponMovement_GetSlideLerp(ws, weapDef);
                    weaponStateMaxMovement = weapDef->vSlideOfs;
                    weaponStateMaxAngles = weapDef->vSlideRot;
                }
                else
                {
                    if ( !Mantle_IsWeaponEquipped(ps) )
                        return;
                    lerp = Mantle_GetMantleFrac(ps);
                    weaponStateMaxMovement = weapDef->vMantleOfs;
                    weaponStateMaxAngles = weapDef->vMantleRot;
                }
            }
            else
            {
                lerp = BG_CalculateWeaponMovement_GetDTPLerp(ws, weapDef);
                weaponStateMaxMovement = weapDef->vDtpOfs;
                weaponStateMaxAngles = weapDef->vDtpRot;
            }
        }
        else
        {
            lerp = BG_CalculateWeaponMovement_GetLowReadyLerp(ws, weapDef);
            weaponStateMaxMovement = weapDef->vLowReadyOfs;
            weaponStateMaxAngles = weapDef->vLowReadyRot;
        }
    }
    else
    {
        lerp = BG_CalculateWeaponMovement_GetSprintLerp(ws, weapDef);
        weaponStateMaxMovement = weapDef->vSprintOfs;
        weaponStateMaxAngles = weapDef->vSprintRot;
    }
    if ( lerp > 0.0 )
    {
        *targetPos = (float)(lerp * *weaponStateMaxMovement) + *targetPos;
        targetPos[1] = (float)(lerp * weaponStateMaxMovement[1]) + targetPos[1];
        targetPos[2] = (float)(lerp * weaponStateMaxMovement[2]) + targetPos[2];
        *targetAngles = (float)(lerp * *weaponStateMaxAngles) + *targetAngles;
        targetAngles[1] = (float)(lerp * weaponStateMaxAngles[1]) + targetAngles[1];
        targetAngles[2] = (float)(lerp * weaponStateMaxAngles[2]) + targetAngles[2];
    }
}

double __cdecl BG_CalculateWeaponMovement_GetSprintLerp(const weaponState_t *ws, const WeaponDef *weapDef)
{
    int sprintInTime; // [esp+10h] [ebp-18h]
    int sprintOutTime; // [esp+14h] [ebp-14h]
    float v5; // [esp+18h] [ebp-10h]
    const playerState_s *ps; // [esp+1Ch] [ebp-Ch]
    float fFrac; // [esp+20h] [ebp-8h]

    sprintInTime = weapDef->sprintInTime;
    sprintOutTime = weapDef->sprintOutTime;
    ps = ws->ps;
    if ( ws->ps->weaponstate == 29 && sprintOutTime )
    {
        v5 = (float)ps->weaponTime / (float)sprintOutTime;
    }
    else if ( ps->weaponstate == 27 && sprintInTime )
    {
        v5 = 1.0 - (float)((float)ps->weaponTime / (float)sprintInTime);
    }
    else
    {
        v5 = 1.0f;
    }
    fFrac = v5;
    if ( (ws->ps->perks[1] & 0x4000) != 0
        && ws->ps->weaponstate == 29
        && perk_sprintRecoveryMultiplier->current.value != 0.0 )
    {
        return (float)(v5 * perk_sprintRecoveryMultiplier->current.value);
    }
    return fFrac;
}

double __cdecl BG_CalculateWeaponMovement_GetLowReadyLerp(const weaponState_t *ws, const WeaponDef *weapDef)
{
    int lowReadyInTime; // [esp+0h] [ebp-14h]
    int lowReadyOutTime; // [esp+4h] [ebp-10h]
    const playerState_s *ps; // [esp+Ch] [ebp-8h]

    lowReadyInTime = weapDef->lowReadyInTime;
    lowReadyOutTime = weapDef->lowReadyOutTime;
    ps = ws->ps;
    if ( ws->ps->weaponstate == 49 && lowReadyOutTime )
    {
        return (float)((float)ps->weaponTime / (float)lowReadyOutTime);
    }
    else if ( ps->weaponstate == 47 && lowReadyInTime )
    {
        return (float)(1.0 - (float)((float)ps->weaponTime / (float)lowReadyInTime));
    }
    else
    {
        return 1.0f;
    }
}

double __cdecl BG_CalculateWeaponMovement_GetDTPLerp(const weaponState_t *ws, const WeaponDef *weapDef)
{
    int dtpInTime; // [esp+0h] [ebp-10h]
    int dtpOutTime; // [esp+4h] [ebp-Ch]
    const playerState_s *ps; // [esp+Ch] [ebp-4h]

    dtpInTime = weapDef->dtpInTime;
    dtpOutTime = weapDef->dtpOutTime;
    ps = ws->ps;
    if ( ws->ps->weaponstate == 42 && dtpOutTime )
    {
        return (float)((float)ps->weaponTime / (float)dtpOutTime);
    }
    else if ( ps->weaponstate == 40 && dtpInTime )
    {
        return (float)(1.0 - (float)((float)ps->weaponTime / (float)dtpInTime));
    }
    else
    {
        return 1.0f;
    }
}

double __cdecl BG_CalculateWeaponMovement_GetSlideLerp(const weaponState_t *ws, const WeaponDef *weapDef)
{
    int slideInTime; // [esp+0h] [ebp-Ch]

    slideInTime = weapDef->slideInTime;
    if ( ws->ps->weaponstate == 43 && slideInTime )
        return (float)(1.0 - (float)((float)ws->ps->weaponTime / (float)slideInTime));
    else
        return 1.0f;
}

void __cdecl BG_CalculateWeaponMovement_Base_Internal_Stance(
                const weaponState_t *ws,
                float *targetPos,
                float *targetAngles)
{
    bool v3; // [esp+0h] [ebp-28h]
    int weapIndex; // [esp+18h] [ebp-10h]
    const playerState_s *ps; // [esp+1Ch] [ebp-Ch]
    const WeaponDef *weapDef; // [esp+20h] [ebp-8h]

    if ( !ws && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_view.cpp", 468, 0, "%s", "ws") )
        __debugbreak();
    ps = ws->ps;
    weapIndex = BG_GetViewmodelWeaponIndex(ws->ps);
    weapDef = BG_GetWeaponDef(weapIndex);
    v3 = ps->weaponstate == 17 || ps->weaponstate == 18 || ps->weaponstate == 19;
    if ( (ps->eFlags & 4) == 0 || v3 )
    {
        if ( (ps->eFlags & 8) != 0 && !v3 )
        {
            *targetPos = (float)(1.0 * weapDef->vProneOfs[0]) + *targetPos;
            targetPos[1] = (float)(1.0 * weapDef->vProneOfs[1]) + targetPos[1];
            targetPos[2] = (float)(1.0 * weapDef->vProneOfs[2]) + targetPos[2];
        }
    }
    else
    {
        *targetPos = (float)(1.0 * weapDef->vDuckedOfs[0]) + *targetPos;
        targetPos[1] = (float)(1.0 * weapDef->vDuckedOfs[1]) + targetPos[1];
        targetPos[2] = (float)(1.0 * weapDef->vDuckedOfs[2]) + targetPos[2];
    }
    if ( weapDef->aimDownSight )
        *targetAngles = (float)(ps->fWeaponPosFrac * weapDef->fAdsAimPitch) + *targetAngles;
}

void __cdecl BG_CalculateWeaponMovement_Base_Internal_Movement(
                const weaponState_t *ws,
                float minSpeed,
                float *targetPos,
                float *targetAngles)
{
    float v4; // [esp+18h] [ebp-58h]
    bool v5; // [esp+1Ch] [ebp-54h]
    const float *vStrafeRot; // [esp+20h] [ebp-50h]
    float v7; // [esp+24h] [ebp-4Ch]
    const float *vStrafeMove; // [esp+28h] [ebp-48h]
    float v9; // [esp+30h] [ebp-40h]
    float v10; // [esp+38h] [ebp-38h]
    float angleWeight; // [esp+3Ch] [ebp-34h]
    float rightViewDir[3]; // [esp+40h] [ebp-30h] BYREF
    bool isCrouched; // [esp+4Fh] [ebp-21h]
    const float (*locomotionMaxAngles)[3]; // [esp+50h] [ebp-20h]
    float strafeFrac; // [esp+54h] [ebp-1Ch]
    float speedFrac; // [esp+58h] [ebp-18h]
    const float (*locomotionMaxMovement)[3]; // [esp+5Ch] [ebp-14h]
    int weapIndex; // [esp+60h] [ebp-10h]
    const playerState_s *ps; // [esp+64h] [ebp-Ch]
    const WeaponDef *weapDef; // [esp+68h] [ebp-8h]
    bool isProne; // [esp+6Fh] [ebp-1h]

    ps = ws->ps;
    weapIndex = BG_GetViewmodelWeaponIndex(ps);
    weapDef = BG_GetWeaponDef(weapIndex);
    v5 = (ps->eFlags & 8) != 0 && (ps->pm_flags & 0x400000) == 0;
    isProne = v5;
    isCrouched = (ps->eFlags & 4) != 0;
    if ( v5 )
    {
        locomotionMaxMovement = (const float (*)[3])weapDef->vProneMove;
        locomotionMaxAngles = (const float (*)[3])weapDef->vProneRot;
    }
    else if ( isCrouched )
    {
        locomotionMaxMovement = (const float (*)[3])weapDef->vDuckedMove;
        locomotionMaxAngles = (const float (*)[3])weapDef->vDuckedRot;
    }
    else
    {
        locomotionMaxMovement = (const float (*)[3])weapDef->vStandMove;
        locomotionMaxAngles = (const float (*)[3])weapDef->vStandRot;
    }
    speedFrac = (float)(ws->xyspeed - minSpeed) / (float)((float)ps->speed - minSpeed);
    if ( (float)(speedFrac - 1.0) < 0.0 )
        v4 = speedFrac;
    else
        v4 = 1.0f;
    speedFrac = v4;
    if ( (v4 < 0.0 || speedFrac > 1.0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_view.cpp",
                    530,
                    1,
                    "speedFrac not in [0.0f, 1.0f]\n\t%g not in [%g, %g]",
                    speedFrac,
                    0.0,
                    1.0) )
    {
        __debugbreak();
    }
    angleWeight = 1.0 - ps->fWeaponPosFrac;
    if ( (angleWeight < 0.0 || angleWeight > 1.0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_view.cpp",
                    534,
                    1,
                    "angleWeight not in [0.0f, 1.0f]\n\t%g not in [%g, %g]",
                    angleWeight,
                    0.0,
                    1.0) )
    {
        __debugbreak();
    }
    *targetPos = (float)(speedFrac * (*locomotionMaxMovement)[0]) + *targetPos;
    targetPos[1] = (float)(speedFrac * (float)(*locomotionMaxMovement)[1]) + targetPos[1];
    targetPos[2] = (float)(speedFrac * (float)(*locomotionMaxMovement)[2]) + targetPos[2];
    v10 = speedFrac * angleWeight;
    *targetAngles = (float)((float)(speedFrac * angleWeight) * (*locomotionMaxAngles)[0]) + *targetAngles;
    targetAngles[1] = (float)(v10 * (float)(*locomotionMaxAngles)[1]) + targetAngles[1];
    targetAngles[2] = (float)(v10 * (float)(*locomotionMaxAngles)[2]) + targetAngles[2];
    BG_GetPlayerViewDirection(ps, 0, rightViewDir, 0);
    if ( ws->xyspeed <= 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_view.cpp",
                    542,
                    0,
                    "%s",
                    "ws->xyspeed > 0.0f") )
    {
        __debugbreak();
    }
    v9 = Vec2Length(ps->velocity) - ws->xyspeed;
    if ( fabs(v9) >= 0.001
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_view.cpp",
                    543,
                    0,
                    "%s",
                    "fabs(Vec2Length( ps->velocity ) - ws->xyspeed) < EQUAL_EPSILON") )
    {
        __debugbreak();
    }
    strafeFrac = (float)((float)((float)(ps->velocity[0] * rightViewDir[0]) + (float)(ps->velocity[1] * rightViewDir[1]))
                                         * speedFrac)
                         / ws->xyspeed;
    if ( (strafeFrac < -1.01 || strafeFrac > 1.01)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_view.cpp",
                    545,
                    1,
                    "strafeFrac not in [-1.01f, 1.01f]\n\t%g not in [%g, %g]",
                    strafeFrac,
                    -1.009999990463257,
                    1.009999990463257) )
    {
        __debugbreak();
    }
    vStrafeMove = weapDef->vStrafeMove;
    *targetPos = (float)(strafeFrac * weapDef->vStrafeMove[0]) + *targetPos;
    targetPos[1] = (float)(strafeFrac * vStrafeMove[1]) + targetPos[1];
    targetPos[2] = (float)(strafeFrac * vStrafeMove[2]) + targetPos[2];
    vStrafeRot = weapDef->vStrafeRot;
    v7 = strafeFrac * angleWeight;
    *targetAngles = (float)((float)(strafeFrac * angleWeight) * weapDef->vStrafeRot[0]) + *targetAngles;
    targetAngles[1] = (float)(v7 * vStrafeRot[1]) + targetAngles[1];
    targetAngles[2] = (float)(v7 * vStrafeRot[2]) + targetAngles[2];
}

void __cdecl BG_CalculateWeaponMovement_Base_Internal_Save(
                weaponState_t *ws,
                const float *targetPos,
                const float *targetAngles)
{
    float v3; // xmm0_4
    float v4; // [esp+18h] [ebp-50h]
    float v5; // [esp+1Ch] [ebp-4Ch]
    float originDelta_4; // [esp+3Ch] [ebp-2Ch]
    float originDelta_8; // [esp+40h] [ebp-28h]
    float angleDelta; // [esp+48h] [ebp-20h]
    float angleDelta_4; // [esp+4Ch] [ebp-1Ch]
    float angleDelta_8; // [esp+50h] [ebp-18h]
    int weapIndex; // [esp+54h] [ebp-14h]
    float originRate; // [esp+58h] [ebp-10h]
    const playerState_s *ps; // [esp+5Ch] [ebp-Ch]
    const WeaponDef *weapDef; // [esp+60h] [ebp-8h]

    if ( !ws && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_view.cpp", 564, 0, "%s", "ws") )
        __debugbreak();
    if ( !targetPos
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_view.cpp", 565, 0, "%s", "targetPos") )
    {
        __debugbreak();
    }
    if ( !targetAngles
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_view.cpp", 566, 0, "%s", "targetAngles") )
    {
        __debugbreak();
    }
    ps = ws->ps;
    if ( !ws->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_view.cpp", 568, 0, "%s", "ps") )
        __debugbreak();
    weapIndex = BG_GetViewmodelWeaponIndex(ps);
    weapDef = BG_GetWeaponDef(weapIndex);
    if ( (ps->eFlags & 8) != 0 )
    {
        originRate = ws->frametime * weapDef->fPosProneMoveRate;
        v3 = ws->frametime * weapDef->fPosProneRotRate;
    }
    else
    {
        originRate = ws->frametime * weapDef->fPosMoveRate;
        v3 = ws->frametime * weapDef->fPosRotRate;
    }
    if ( (float)(1.0 - originRate) < 0.0 )
        v5 = 1.0f;
    else
        v5 = originRate;
    if ( (float)(1.0 - v3) < 0.0 )
        v4 = 1.0f;
    else
        v4 = v3;
    if ( (v5 < 0.0 || v5 > 1.0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_view.cpp",
                    588,
                    1,
                    "originRate not in [0.0f, 1.0f]\n\t%g not in [%g, %g]",
                    v5,
                    0.0,
                    1.0) )
    {
        __debugbreak();
    }
    if ( (v4 < 0.0 || v4 > 1.0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_view.cpp",
                    589,
                    1,
                    "angleRate not in [0.0f, 1.0f]\n\t%g not in [%g, %g]",
                    v4,
                    0.0,
                    1.0) )
    {
        __debugbreak();
    }
    originDelta_4 = targetPos[1] - ws->baseOrigin[1];
    originDelta_8 = targetPos[2] - ws->baseOrigin[2];
    angleDelta = *targetAngles - ws->baseAngles[0];
    angleDelta_4 = targetAngles[1] - ws->baseAngles[1];
    angleDelta_8 = targetAngles[2] - ws->baseAngles[2];
    ws->baseOrigin[0] = (float)(v5 * (float)(*targetPos - ws->baseOrigin[0])) + ws->baseOrigin[0];
    ws->baseOrigin[1] = (float)(v5 * originDelta_4) + ws->baseOrigin[1];
    ws->baseOrigin[2] = (float)(v5 * originDelta_8) + ws->baseOrigin[2];
    ws->baseAngles[0] = (float)(v4 * angleDelta) + ws->baseAngles[0];
    ws->baseAngles[1] = (float)(v4 * angleDelta_4) + ws->baseAngles[1];
    ws->baseAngles[2] = (float)(v4 * angleDelta_8) + ws->baseAngles[2];
}

void __cdecl BG_CalculateWeaponMovement_Recoil(weaponState_t *ws)
{
    int v1; // eax
    float fTimeStep; // [esp+18h] [ebp-2Ch]
    float fGunKickAccel; // [esp+1Ch] [ebp-28h]
    float fGunKickSpeedDecay; // [esp+20h] [ebp-24h]
    float fGunKickStaticDecay; // [esp+24h] [ebp-20h]
    float fTotalTime; // [esp+28h] [ebp-1Ch]
    float fGunKickSpeedMax; // [esp+2Ch] [ebp-18h]
    int bCanStop; // [esp+30h] [ebp-14h]
    int weapIndex; // [esp+34h] [ebp-10h]
    float fPosLerp; // [esp+38h] [ebp-Ch]
    const playerState_s *ps; // [esp+3Ch] [ebp-8h]
    const WeaponDef *weapDef; // [esp+40h] [ebp-4h]

    ps = ws->ps;
    weapIndex = BG_GetViewmodelWeaponIndex(ws->ps);
    weapDef = BG_GetWeaponDef(weapIndex);
    if ( BG_IsAimDownSightWeapon(weapIndex) )
    {
        fPosLerp = ps->fWeaponPosFrac;
        fGunKickAccel = (float)((float)(weapDef->fAdsGunKickAccel - weapDef->fHipGunKickAccel) * fPosLerp)
                                    + weapDef->fHipGunKickAccel;
        fGunKickSpeedMax = (float)((float)(weapDef->fAdsGunKickSpeedMax - weapDef->fHipGunKickSpeedMax) * fPosLerp)
                                         + weapDef->fHipGunKickSpeedMax;
        fGunKickSpeedDecay = (float)((float)(weapDef->fAdsGunKickSpeedDecay - weapDef->fHipGunKickSpeedDecay) * fPosLerp)
                                             + weapDef->fHipGunKickSpeedDecay;
        fGunKickStaticDecay = (float)((float)(weapDef->fAdsGunKickStaticDecay - weapDef->fHipGunKickStaticDecay) * fPosLerp)
                                                + weapDef->fHipGunKickStaticDecay;
        fTotalTime = ws->frametime;
        while ( fTotalTime > 0.0 )
        {
            if ( fTotalTime <= 0.0049999999 )
            {
                fTimeStep = fTotalTime;
                fTotalTime = 0.0f;
                v1 = BG_CalculateWeaponMovement_GunRecoil_SingleAngle(
                             ws->recoilAngles,
                             ws->recoilSpeed,
                             fTimeStep,
                             weapDef->fGunMaxPitch,
                             fGunKickAccel,
                             fGunKickSpeedMax,
                             fGunKickSpeedDecay,
                             fGunKickStaticDecay);
            }
            else
            {
                fTimeStep = 0.005f;
                fTotalTime = fTotalTime - 0.0049999999;
                v1 = BG_CalculateWeaponMovement_GunRecoil_SingleAngle(
                             ws->recoilAngles,
                             ws->recoilSpeed,
                             0.0049999999,
                             weapDef->fGunMaxPitch,
                             fGunKickAccel,
                             fGunKickSpeedMax,
                             fGunKickSpeedDecay,
                             fGunKickStaticDecay);
            }
            bCanStop = v1;
            if ( BG_CalculateWeaponMovement_GunRecoil_SingleAngle(
                         &ws->recoilAngles[1],
                         &ws->recoilSpeed[1],
                         fTimeStep,
                         weapDef->fGunMaxYaw,
                         fGunKickAccel,
                         fGunKickSpeedMax,
                         fGunKickSpeedDecay,
                         fGunKickStaticDecay) )
            {
                if ( bCanStop )
                    break;
            }
        }
    }
}

int __cdecl BG_CalculateWeaponMovement_GunRecoil_SingleAngle(
                float *fOffset,
                float *speed,
                float fTimeStep,
                float fOfsCap,
                float fGunKickAccel,
                float fGunKickSpeedMax,
                float fGunKickSpeedDecay,
                float fGunKickStaticDecay)
{
    int bCanStop; // [esp+8h] [ebp-4h]

    bCanStop = 0;
    if ( fabs(*fOffset) >= 0.25
        || fabs(*speed) >= 1.0 )
    {
        *fOffset = (float)(*speed * fTimeStep) + *fOffset;
        if ( *fOffset <= fOfsCap )
        {
            //if ( COERCE_FLOAT(LODWORD(fOfsCap) ^ _mask__NegFloat_) > *fOffset )
            if ( -fOfsCap > *fOffset )
            {
                //*(unsigned int *)fOffset = LODWORD(fOfsCap) ^ _mask__NegFloat_;
                *fOffset = -fOfsCap;
                if ( *speed < 0.0 )
                    *speed = 0.0f;
            }
        }
        else
        {
            *fOffset = fOfsCap;
            if ( *speed > 0.0 )
                *speed = 0.0f;
        }
        if ( *fOffset <= 0.0 )
        {
            if ( *fOffset < 0.0 )
                *speed = (float)(fGunKickAccel * fTimeStep) + *speed;
        }
        else
        {
            *speed = *speed - (float)(fGunKickAccel * fTimeStep);
        }
        *speed = *speed - (float)((float)(*speed * fGunKickSpeedDecay) * fTimeStep);
        if ( *speed <= 0.0 )
        {
            *speed = (float)(fGunKickStaticDecay * fTimeStep) + *speed;
            if ( *speed > 0.0 )
                *speed = 0.0f;
        }
        else
        {
            *speed = *speed - (float)(fGunKickStaticDecay * fTimeStep);
            if ( *speed < 0.0 )
                *speed = 0.0f;
        }
        if ( *speed <= fGunKickSpeedMax )
        {
            //if ( COERCE_FLOAT(LODWORD(fGunKickSpeedMax) ^ _mask__NegFloat_) > *speed )
            //    *(unsigned int *)speed = LODWORD(fGunKickSpeedMax) ^ _mask__NegFloat_;
            if (-fGunKickSpeedMax > *speed)
            {
                *speed = -fGunKickSpeedMax;
            }
        }
        else
        {
            *speed = fGunKickSpeedMax;
        }
    }
    else
    {
        *fOffset = 0.0f;
        *speed = 0.0f;
        return 1;
    }
    return bCanStop;
}

void __cdecl BG_CalculateWeaponMovement_Sway(weaponState_t *ws)
{
    float v1; // [esp+10h] [ebp-8Ch]
    float v2; // [esp+14h] [ebp-88h]
    float v3; // [esp+24h] [ebp-78h]
    float v4; // [esp+2Ch] [ebp-70h]
    float v5; // [esp+30h] [ebp-6Ch]
    float deltaOffset_8; // [esp+5Ch] [ebp-40h]
    float swayPitchScale; // [esp+60h] [ebp-3Ch]
    float swayPitchScalea; // [esp+60h] [ebp-3Ch]
    float swayVertScale; // [esp+64h] [ebp-38h]
    float swayVertScalea; // [esp+64h] [ebp-38h]
    float swayLerpSpeed; // [esp+68h] [ebp-34h]
    float swayHorizScale; // [esp+6Ch] [ebp-30h]
    float swayHorizScalea; // [esp+6Ch] [ebp-30h]
    float swayYawScale; // [esp+70h] [ebp-2Ch]
    float swayYawScalea; // [esp+70h] [ebp-2Ch]
    const WeaponVariantDef *weapVariantDef; // [esp+74h] [ebp-28h]
    int weapIndex; // [esp+78h] [ebp-24h]
    float swayMaxAngle; // [esp+7Ch] [ebp-20h]
    const playerState_s *ps; // [esp+80h] [ebp-1Ch]
    const WeaponDef *weapDef; // [esp+84h] [ebp-18h]
    int ssDeltaTime; // [esp+88h] [ebp-14h]
    float ssSwayScale; // [esp+8Ch] [ebp-10h]
    float deltaViewAngles[3]; // [esp+90h] [ebp-Ch] BYREF

    if ( ws->frametime != 0.0 )
    {
        ps = ws->ps;
        weapIndex = BG_GetViewmodelWeaponIndex(ws->ps);
        weapVariantDef = BG_GetWeaponVariantDef(weapIndex);
        weapDef = BG_GetWeaponDef(weapIndex);
        if ( weapDef->aimDownSight )
        {
            if ( ps->fWeaponPosFrac > 0.0 && weapDef->overlayReticle )
            {
                ws->swayViewAngles[0] = ps->viewangles[0];
                ws->swayViewAngles[1] = ps->viewangles[1];
                ws->swayViewAngles[2] = ps->viewangles[2];
                return;
            }
            swayMaxAngle = (float)((float)(1.0 - ps->fWeaponPosFrac) * weapDef->swayMaxAngle)
                                     + (float)(weapDef->adsSwayMaxAngle * ps->fWeaponPosFrac);
            swayLerpSpeed = (float)((float)(1.0 - ps->fWeaponPosFrac) * weapDef->swayLerpSpeed)
                                        + (float)(weapDef->adsSwayLerpSpeed * ps->fWeaponPosFrac);
            swayPitchScale = (float)((float)(1.0 - ps->fWeaponPosFrac) * weapDef->swayPitchScale)
                                         + (float)(weapDef->adsSwayPitchScale * ps->fWeaponPosFrac);
            swayYawScale = (float)((float)(1.0 - ps->fWeaponPosFrac) * weapDef->swayYawScale)
                                     + (float)(weapDef->adsSwayYawScale * ps->fWeaponPosFrac);
            swayHorizScale = (float)((float)(1.0 - ps->fWeaponPosFrac) * weapDef->swayHorizScale)
                                         + (float)(weapVariantDef->fAdsSwayHorizScale * ps->fWeaponPosFrac);
            swayVertScale = (float)((float)(1.0 - ps->fWeaponPosFrac) * weapDef->swayVertScale)
                                        + (float)(weapVariantDef->fAdsSwayVertScale * ps->fWeaponPosFrac);
        }
        else
        {
            swayMaxAngle = weapDef->swayMaxAngle;
            swayLerpSpeed = weapDef->swayLerpSpeed;
            swayPitchScale = weapDef->swayPitchScale;
            swayYawScale = weapDef->swayYawScale;
            swayHorizScale = weapDef->swayHorizScale;
            swayVertScale = weapDef->swayVertScale;
        }
        ssDeltaTime = ws->shellShockDuration + ws->shellShockStart - ws->time;
        ssSwayScale = 1.0f;
        if ( (float)ssDeltaTime > 0.0 )
        {
            if ( ssDeltaTime < ws->shellShockFadeTime )
                ssSwayScale = (float)ssDeltaTime / (float)ws->shellShockFadeTime;
            ssSwayScale = (float)((float)(weapDef->swayShellShockScale - 1.0)
                                                    * (float)((float)((float)(3.0 - (float)(2.0 * ssSwayScale)) * ssSwayScale) * ssSwayScale))
                                    + 1.0;
        }
        swayPitchScalea = swayPitchScale * ssSwayScale;
        swayYawScalea = swayYawScale * ssSwayScale;
        swayHorizScalea = swayHorizScale * ssSwayScale;
        swayVertScalea = swayVertScale * ssSwayScale;
        AnglesSubtract(ps->viewangles, ws->swayViewAngles, deltaViewAngles);
        v5 = 1.0 / (float)(60.0 * ws->frametime);
        deltaViewAngles[0] = v5 * deltaViewAngles[0];
        deltaViewAngles[1] = v5 * deltaViewAngles[1];
        deltaViewAngles[2] = v5 * deltaViewAngles[2];
        if ( (float)(deltaViewAngles[0] - swayMaxAngle) < 0.0 )
            v4 = deltaViewAngles[0];
        else
            v4 = swayMaxAngle;
        //if ( (float)(COERCE_FLOAT(LODWORD(swayMaxAngle) ^ _mask__NegFloat_) - deltaViewAngles[0]) < 0.0 )
        if (-swayMaxAngle - deltaViewAngles[0] < 0.0f)
        {
            v2 = v4;
        }
        else
        {
            //v2 = -swayMaxAngle;
            //v2 = -swayMaxAngle;
            v2 = -swayMaxAngle;
        }
        deltaViewAngles[0] = v2;

        if ( (float)(deltaViewAngles[1] - swayMaxAngle) < 0.0 )
            v3 = deltaViewAngles[1];
        else
            v3 = swayMaxAngle;

        //if ((float)(COERCE_FLOAT(LODWORD(swayMaxAngle) ^ _mask__NegFloat_) - deltaViewAngles[1]) < 0.0)
        if ((-swayMaxAngle - deltaViewAngles[1]) < 0.0)
        {
            v1 = v3;
        }
        else
        {
            //v1 = -swayMaxAngle;
            v1 = -swayMaxAngle;
        }

        deltaViewAngles[1] = v1;
        deltaOffset_8 = deltaViewAngles[0] * swayVertScalea;
        ws->swayOrigin[1] = DiffTrack(v1 * swayHorizScalea, ws->swayOrigin[1], swayLerpSpeed, ws->frametime);
        ws->swayOrigin[2] = DiffTrack(deltaOffset_8, ws->swayOrigin[2], swayLerpSpeed, ws->frametime);
        deltaViewAngles[0] = deltaViewAngles[0] * swayPitchScalea;
        deltaViewAngles[1] = deltaViewAngles[1] * swayYawScalea;
        ws->swayAngles[0] = DiffTrackAngle(deltaViewAngles[0], ws->swayAngles[0], swayLerpSpeed, ws->frametime);
        ws->swayAngles[1] = DiffTrackAngle(deltaViewAngles[1], ws->swayAngles[1], swayLerpSpeed, ws->frametime);
        ws->swayViewAngles[0] = ps->viewangles[0];
        ws->swayViewAngles[1] = ps->viewangles[1];
        ws->swayViewAngles[2] = ps->viewangles[2];
    }
}

void __cdecl BG_CalculateWeaponMovement_Bob(weaponState_t *ws)
{
    float v1; // [esp+8h] [ebp-38h]
    float v2; // [esp+10h] [ebp-30h]
    float v3; // [esp+1Ch] [ebp-24h]
    float bobCycle; // [esp+20h] [ebp-20h]
    float cycle; // [esp+24h] [ebp-1Ch]
    int weapIndex; // [esp+28h] [ebp-18h]
    float speed; // [esp+2Ch] [ebp-14h]
    const playerState_s *ps; // [esp+34h] [ebp-Ch]
    const WeaponDef *weapDef; // [esp+38h] [ebp-8h]
    float adsBobFactor; // [esp+3Ch] [ebp-4h]

    ps = ws->ps;
    weapIndex = BG_GetViewmodelWeaponIndex(ws->ps);
    weapDef = BG_GetWeaponDef(weapIndex);
    bobCycle = BG_GetBobCycle(ps);
    cycle = (float)((float)(3.1415927 * bg_weaponBobLag->current.value) + bobCycle) + 6.2831855;
    speed = ws->xyspeed * bg_weaponBobAmplitudeBase->current.value;
    v2 = -1.0 * 1.0;
    ws->bobAngles[0] = BG_CalculateWeaponMovement_VerticalBobFactor(ps, cycle, speed) * v2;
    ws->bobAngles[1] = BG_CalculateWeaponMovement_HorizontalBobFactor(ps, cycle, speed) * v2;
    v3 = BG_CalculateWeaponMovement_HorizontalBobFactor(
                 ps,
                 cycle - 0.47123894,
                 speed * bg_weaponBobAmplitudeRoll->current.value);
    if ( (float)(0.0 - v3) < 0.0 )
        v1 = 0.0f;
    else
        v1 = v3;
    ws->bobAngles[2] = v1;
    if ( ps->fWeaponPosFrac > 0.0 )
    {
        adsBobFactor = 1.0 - (float)((float)(1.0 - weapDef->fAdsBobFactor) * ps->fWeaponPosFrac);
        ws->bobAngles[0] = adsBobFactor * ws->bobAngles[0];
        ws->bobAngles[1] = adsBobFactor * ws->bobAngles[1];
        ws->bobAngles[2] = adsBobFactor * ws->bobAngles[2];
    }
}

void __cdecl BG_SetWeaponMovementAngles(weaponState_t *ws, float *angles)
{
    float LeanFraction; // [esp+20h] [ebp-2Ch]

    *angles = 0.0f;
    angles[1] = 0.0f;
    angles[2] = 0.0f;
    BG_ApplyWeaponMovement_BaseAngles(ws, angles);
    BG_ApplyWeaponMovement_SwayAngles(ws, angles);
    if ( ws->ps->leanf != 0.0 )
    {
        LeanFraction = GetLeanFraction(ws->ps->leanf);
        angles[2] = angles[2] - (float)(LeanFraction * 2.0);
    }
    *angles = *angles + ws->recoilAngles[0];
    angles[1] = angles[1] + ws->recoilAngles[1];
    angles[2] = angles[2] + ws->recoilAngles[2];
    BG_ApplyWeaponMovement_BobAngles(ws, angles);
    BG_ComputeAndApplyWeaponMovement_IdleAngles(ws, angles);
    BG_ComputeAndApplyWeaponMovement_DamageKickAngles(ws, angles);
}

void __cdecl BG_ComputeAndApplyWeaponMovement_IdleAngles(weaponState_t *ws, float *angles)
{
    const playerState_s *ps = ws->ps;

    int weapIndex = BG_GetViewmodelWeaponIndex(ps);
    const WeaponDef *weapDef = BG_GetWeaponDef(weapIndex);

    float fTargScale;
    float idleSpeed;

    if (weapDef->aimDownSight)
    {
        fTargScale =
            (weapDef->fAdsIdleAmount - weapDef->fHipIdleAmount) * ps->fWeaponPosFrac +
            weapDef->fHipIdleAmount;

        idleSpeed =
            (weapDef->adsIdleSpeed - weapDef->hipIdleSpeed) * ps->fWeaponPosFrac +
            weapDef->hipIdleSpeed;

        if (zero_idle_movement->current.enabled)
        {
            fTargScale = 0.0f;
            idleSpeed = 0.0f;
        }
    }
    else
    {
        fTargScale = weapDef->fHipIdleAmount;
        idleSpeed = weapDef->hipIdleSpeed;
    }

    float fTargFactor;
    if (ps->eFlags & 8)
        fTargFactor = weapDef->fIdleProneFactor;
    else if (ps->eFlags & 4)
        fTargFactor = weapDef->fIdleCrouchFactor;
    else
        fTargFactor = 1.0f;

    const float factorSpeed = ws->frametime * 0.5f;
    if (fTargFactor > ws->fLastIdleFactor)
        ws->fLastIdleFactor = std::min(ws->fLastIdleFactor + factorSpeed, fTargFactor);
    else
        ws->fLastIdleFactor = std::max(ws->fLastIdleFactor - factorSpeed, fTargFactor);

    fTargScale *= ws->fLastIdleFactor;

    if (weapDef->overlayReticle)
        fTargScale *= (1.0f - ps->fWeaponPosFrac);

    *ws->weapIdleTime += (int)(ws->frametime * 1000.0f * idleSpeed);

    const int idleTime = *ws->weapIdleTime;
    const float scale = fTargScale * 0.01f;

    angles[2] += scale * sinf(idleTime * 0.0005f);
    angles[1] += scale * sinf(idleTime * 0.0007f);
    angles[0] += scale * sinf(idleTime * 0.0010f);
}

void __cdecl BG_ComputeAndApplyWeaponMovement_DamageKickAngles(weaponState_t *ws, float *angles)
{
    float fReturnTime; // [esp+4h] [ebp-1Ch]
    float fDeflectTime; // [esp+8h] [ebp-18h]
    float fRatio; // [esp+Ch] [ebp-14h]
    float fRatiob; // [esp+Ch] [ebp-14h]
    float fRatioc; // [esp+Ch] [ebp-14h]
    float fRatioa; // [esp+Ch] [ebp-14h]
    float fRatiod; // [esp+Ch] [ebp-14h]
    float fRatioe; // [esp+Ch] [ebp-14h]
    int weapIndex; // [esp+10h] [ebp-10h]
    float fFactor; // [esp+14h] [ebp-Ch]
    const playerState_s *ps; // [esp+18h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+1Ch] [ebp-4h]

    if ( ws->damageTime )
    {
        ps = ws->ps;
        weapIndex = BG_GetViewmodelWeaponIndex(ws->ps);
        weapDef = BG_GetWeaponDef(weapIndex);
        fFactor = (float)(ps->fWeaponPosFrac * 0.5) + 0.5;
        fDeflectTime = 100.0 * fFactor;
        fReturnTime = 400.0 * fFactor;
        if ( ps->fWeaponPosFrac != 0.0 && weapDef->overlayReticle )
            fFactor = (float)(1.0 - (float)(ps->fWeaponPosFrac * 0.75)) * fFactor;
        fRatio = (float)(ws->time - ws->damageTime);
        if ( fDeflectTime <= fRatio )
        {
            fRatioa = 1.0 - (float)((float)(fRatio - fDeflectTime) / fReturnTime);
            if ( fRatioa > 0.0 )
            {
                fRatiod = 1.0 - GetLeanFraction(1.0 - fRatioa);
                fRatioe = fRatiod * fFactor;
                *angles = (float)((float)(fRatioe * ws->v_dmg_pitch) * 0.5) + *angles;
                angles[1] = angles[1] - (float)(fRatioe * ws->v_dmg_roll);
                angles[2] = (float)((float)(fRatioe * ws->v_dmg_roll) * 0.5) + angles[2];
            }
        }
        else
        {
            fRatiob = GetLeanFraction(fRatio / fDeflectTime);
            fRatioc = fRatiob * fFactor;
            *angles = (float)((float)(fRatioc * ws->v_dmg_pitch) * 0.5) + *angles;
            angles[1] = angles[1] - (float)(fRatioc * ws->v_dmg_roll);
            angles[2] = (float)((float)(fRatioc * ws->v_dmg_roll) * 0.5) + angles[2];
        }
    }
}

void __cdecl BG_ApplyWeaponMovement_BaseAngles(weaponState_t *ws, float *angles)
{
    float adsFrac; // [esp+8h] [ebp-4h]

    if ( !ws && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_view.cpp", 233, 0, "%s", "ws") )
        __debugbreak();
    if ( !angles
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_view.cpp",
                    234,
                    0,
                    "%s",
                    "angles != NULL") )
    {
        __debugbreak();
    }
    adsFrac = 1.0 - (float)(ws->ps->fWeaponPosFrac * 2.0);
    if ( adsFrac > 0.0 )
    {
        *angles = (float)(adsFrac * ws->baseAngles[0]) + *angles;
        angles[1] = (float)(adsFrac * ws->baseAngles[1]) + angles[1];
        angles[2] = (float)(adsFrac * ws->baseAngles[2]) + angles[2];
    }
}

void __cdecl BG_ApplyWeaponMovement_SwayAngles(weaponState_t *ws, float *angles)
{
    *angles = AngleNormalize180(*angles - ws->swayAngles[0]);
    angles[1] = AngleNormalize180(angles[1] - ws->swayAngles[1]);
}

void __cdecl BG_ApplyWeaponMovement_BobAngles(weaponState_t *ws, float *angles)
{
    float v2; // xmm0_4
    float v3; // [esp+8h] [ebp-10h]
    int weapIndex; // [esp+Ch] [ebp-Ch]
    const playerState_s *ps; // [esp+10h] [ebp-8h]

    ps = ws->ps;
    weapIndex = BG_GetViewmodelWeaponIndex(ws->ps);
    if ( BG_GetWeaponDef(weapIndex)->overlayReticle )
    {
        v3 = 1.0 - ps->fWeaponPosFrac;
        *angles = (float)(v3 * ws->bobAngles[0]) + *angles;
        angles[1] = (float)(v3 * ws->bobAngles[1]) + angles[1];
        v2 = (float)(v3 * ws->bobAngles[2]) + angles[2];
    }
    else
    {
        *angles = *angles + ws->bobAngles[0];
        angles[1] = angles[1] + ws->bobAngles[1];
        v2 = angles[2] + ws->bobAngles[2];
    }
    angles[2] = v2;
}

void __cdecl BG_SetWeaponMovementOrigin(weaponState_t *ws, float *origin)
{
    *origin = 0.0f;
    origin[1] = 0.0f;
    origin[2] = 0.0f;
    BG_ApplyWeaponMovement_BaseOrigin(ws, origin);
    origin[1] = origin[1] - ws->swayOrigin[1];
    origin[2] = origin[2] + ws->swayOrigin[2];
    BG_ApplyWeaponMovement_LeanOrigin(ws, origin);
    BG_ApplyWeaponMovement_BobOrigin(ws, origin);
}

void __cdecl BG_ApplyWeaponMovement_BaseOrigin(weaponState_t *ws, float *origin)
{
    float adsFrac; // [esp+8h] [ebp-4h]

    if ( !ws && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_view.cpp", 250, 0, "%s", "ws") )
        __debugbreak();
    if ( !origin
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_view.cpp",
                    251,
                    0,
                    "%s",
                    "origin != NULL") )
    {
        __debugbreak();
    }
    adsFrac = 1.0 - (float)(ws->ps->fWeaponPosFrac * 2.0);
    if ( adsFrac > 0.0 )
    {
        *origin = (float)(adsFrac * ws->baseOrigin[0]) + *origin;
        origin[1] = (float)(adsFrac * ws->baseOrigin[1]) + origin[1];
        origin[2] = (float)(adsFrac * ws->baseOrigin[2]) + origin[2];
    }
}

void __cdecl BG_ApplyWeaponMovement_LeanOrigin(weaponState_t *ws, float *origin)
{
    float fLean; // [esp+4h] [ebp-24h]
    float right[3]; // [esp+8h] [ebp-20h] BYREF
    float fDist; // [esp+14h] [ebp-14h]
    const playerState_s *ps; // [esp+18h] [ebp-10h]
    float rollAngles[3]; // [esp+1Ch] [ebp-Ch] BYREF

    ps = ws->ps;
    if ( ps->leanf != 0.0 && ps->fWeaponPosFrac < 1.0 )
    {
        memset(rollAngles, 0, sizeof(rollAngles));
        fLean = GetLeanFraction(ps->leanf);
        rollAngles[2] = 0.0 - (float)(fLean * 2.0);
        fDist = (float)((float)(1.0 - ps->fWeaponPosFrac) * fLean) * 1.6;
        AngleVectors(rollAngles, 0, right, 0);
        *origin = (float)(fDist * right[0]) + *origin;
        origin[1] = (float)(fDist * right[1]) + origin[1];
        origin[2] = (float)(fDist * right[2]) + origin[2];
    }
}

void __cdecl BG_ApplyWeaponMovement_BobOrigin(weaponState_t *ws, float *origin)
{
    float originCopy[3]; // [esp+0h] [ebp-30h] BYREF
    float bobAxis[3][3]; // [esp+Ch] [ebp-24h] BYREF

    originCopy[0] = *origin;
    originCopy[1] = origin[1];
    originCopy[2] = origin[2];
    AnglesToAxis(ws->bobAngles, bobAxis);
    MatrixTransformVector(originCopy, bobAxis, origin);
}

double __cdecl BG_CalculateViewMovement_VerticalBobFactor(const playerState_s *ps, float cycle, float speed)
{
    float amplitude;

    if (ps->waterlevel < 3)
    {
        if (ps->pm_flags & 0x400000)
        {
            amplitude = speed * bg_viewBobAmplitudeDtp->current.vector[1];
        }
        else if (ps->viewHeightTarget == 11)
        {
            amplitude = speed * bg_viewBobAmplitudeProne->current.vector[1];
        }
        else if (ps->viewHeightTarget == 40)
        {
            const float frac = ps->fWeaponPosFrac;
            amplitude =
                ((1.0f - frac) * bg_viewBobAmplitudeDucked->current.vector[1] +
                    frac * bg_viewBobAmplitudeDuckedAds->current.vector[1]) *
                speed;
        }
        else if (ps->pm_flags & 0x8000)
        {
            amplitude = speed * bg_viewBobAmplitudeSprinting->current.vector[1];
        }
        else
        {
            const float frac = ps->fWeaponPosFrac;
            amplitude =
                ((1.0f - frac) * bg_viewBobAmplitudeStanding->current.vector[1] +
                    frac * bg_viewBobAmplitudeStandingAds->current.vector[1]) *
                speed;
        }
    }
    else
    {
        amplitude = bg_viewBobAmplitudeSwimming->current.vector[1];
    }

    if (amplitude > bg_viewBobMax->current.value)
        amplitude = bg_viewBobMax->current.value;

    const float sinA = sinf(cycle * 2.0f);
    const float sinB = sinf(cycle * 4.0f + 1.5707964f);

    return (sinA + sinB * 0.2f) * 0.75f * amplitude;
}

double __cdecl BG_CalculateViewMovement_HorizontalBobFactor(const playerState_s *ps, float cycle, float speed)
{
    float v4; // [esp+0h] [ebp-10h]
    float amplitude; // [esp+Ch] [ebp-4h]

    if ( ps->waterlevel < 3 )
    {
        if ( (ps->pm_flags & 0x400000) != 0 )
        {
            amplitude = speed * bg_viewBobAmplitudeDtp->current.value;
        }
        else if ( ps->viewHeightTarget == 11 )
        {
            amplitude = speed * bg_viewBobAmplitudeProne->current.value;
        }
        else if ( ps->viewHeightTarget == 40 )
        {
            amplitude = (float)((float)((float)(1.0 - ps->fWeaponPosFrac) * bg_viewBobAmplitudeDucked->current.value)
                                                + (float)(bg_viewBobAmplitudeDuckedAds->current.value * ps->fWeaponPosFrac))
                                * speed;
        }
        else if ( (ps->pm_flags & 0x8000) != 0 )
        {
            amplitude = speed * bg_viewBobAmplitudeSprinting->current.value;
        }
        else
        {
            amplitude = (float)((float)((float)(1.0 - ps->fWeaponPosFrac) * bg_viewBobAmplitudeStanding->current.value)
                                                + (float)(bg_viewBobAmplitudeStandingAds->current.value * ps->fWeaponPosFrac))
                                * speed;
        }
    }
    else
    {
        amplitude = bg_viewBobAmplitudeSwimming->current.value;
    }
    if ( amplitude > bg_viewBobMax->current.value )
        amplitude = bg_viewBobMax->current.value;
    v4 = sin(cycle);
    return v4 * amplitude;
}

void __cdecl BG_CalculateViewMovementAngles(viewState_t *vs, float *angles)
{
    *angles = 0.0f;
    angles[1] = 0.0f;
    angles[2] = 0.0f;
    BG_CalculateViewMovement_DamageKick(vs, angles);
    BG_CalculateViewMovement_IdleAngles(vs, angles);
    BG_CalculateViewMovement_BobAngles(vs, angles);
    BG_CalculateViewMovement_AdsBob(vs, angles);
}

void __cdecl BG_CalculateViewMovement_DamageKick(viewState_t *vs, float *angles)
{
    float fRatio; // [esp+4h] [ebp-14h]
    float fRatiob; // [esp+4h] [ebp-14h]
    float fRatioc; // [esp+4h] [ebp-14h]
    float fRatioa; // [esp+4h] [ebp-14h]
    float fRatiod; // [esp+4h] [ebp-14h]
    float fRatioe; // [esp+4h] [ebp-14h]
    int weapIndex; // [esp+8h] [ebp-10h]
    float fFactor; // [esp+Ch] [ebp-Ch]
    playerState_s *ps; // [esp+10h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+14h] [ebp-4h]

    if ( vs->damageTime )
    {
        ps = vs->ps;
        weapIndex = BG_GetViewmodelWeaponIndex(vs->ps);
        weapDef = BG_GetWeaponDef(weapIndex);
        fFactor = 1.0 - (float)(ps->fWeaponPosFrac * 0.5);
        if ( ps->fWeaponPosFrac != 0.0 && weapDef->overlayReticle )
            fFactor = (float)((float)(ps->fWeaponPosFrac * 0.5) + 1.0) * fFactor;
        fRatio = (float)(vs->time - vs->damageTime);
        if ( fRatio >= 100.0 )
        {
            fRatioa = 1.0 - (float)((float)(fRatio - 100.0) / 400.0);
            if ( fRatioa > 0.0 )
            {
                fRatiod = 1.0 - GetLeanFraction(1.0 - fRatioa);
                fRatioe = fRatiod * fFactor;
                *angles = (float)(fRatioe * vs->v_dmg_pitch) + *angles;
                angles[2] = (float)(fRatioe * vs->v_dmg_roll) + angles[2];
            }
        }
        else
        {
            fRatiob = GetLeanFraction(fRatio / 100.0);
            fRatioc = fRatiob * fFactor;
            *angles = (float)(fRatioc * vs->v_dmg_pitch) + *angles;
            angles[2] = (float)(fRatioc * vs->v_dmg_roll) + angles[2];
        }
    }
}

void __cdecl BG_CalculateViewMovement_IdleAngles(viewState_t *vs, float *angles)
{
    playerState_s *ps = vs->ps;

    int weapIndex = BG_GetViewmodelWeaponIndex(ps);
    const WeaponDef *weapDef = BG_GetWeaponDef(weapIndex);

    if (!weapDef->overlayReticle || ps->fWeaponPosFrac == 0.0f)
        return;

    float fTargScale;
    float idleSpeed;

    if (BG_IsAimDownSightWeapon(weapIndex))
    {
        fTargScale =
            (weapDef->fAdsIdleAmount - weapDef->fHipIdleAmount) *
            ps->fWeaponPosFrac +
            weapDef->fHipIdleAmount;

        idleSpeed =
            (weapDef->adsIdleSpeed - weapDef->hipIdleSpeed) *
            ps->fWeaponPosFrac +
            weapDef->hipIdleSpeed;

        if (zero_idle_movement->current.enabled)
        {
            fTargScale = 0.0f;
            idleSpeed = 0.0f;
        }
    }
    else if (weapDef->fHipIdleAmount != 0.0f)
    {
        fTargScale = weapDef->fHipIdleAmount;
        idleSpeed = weapDef->hipIdleSpeed;
    }
    else
    {
        fTargScale = 80.0f;
        idleSpeed = 1.0f;
    }

    float fTargFactor;
    if ((ps->eFlags & 8) && !(ps->pm_flags & 0x400000))
        fTargFactor = weapDef->fIdleProneFactor;
    else if (ps->eFlags & 4)
        fTargFactor = weapDef->fIdleCrouchFactor;
    else
        fTargFactor = 1.0f;

    const float factorSpeed = vs->frametime * 0.5f;
    if (fTargFactor > vs->fLastIdleFactor)
        vs->fLastIdleFactor = std::min(vs->fLastIdleFactor + factorSpeed, fTargFactor);
    else
        vs->fLastIdleFactor = std::max(vs->fLastIdleFactor - factorSpeed, fTargFactor);

    fTargScale *= vs->fLastIdleFactor;
    fTargScale *= ps->fWeaponPosFrac;
    fTargScale *= ps->holdBreathScale;

    *vs->weapIdleTime += (int)(
        ps->holdBreathScale *
        vs->frametime *
        1000.0f *
        idleSpeed);

    const int idleTime = *vs->weapIdleTime;

    angles[1] += fTargScale * sinf(idleTime * 0.0007f) * 0.01f;
    angles[0] += fTargScale * sinf(idleTime * 0.0010f) * 0.01f;
}

void __cdecl BG_CalculateViewMovement_BobAngles(viewState_t *vs, float *angles)
{
    float v2; // [esp+8h] [ebp-30h]
    float v3; // [esp+Ch] [ebp-2Ch]
    float vAngOfs; // [esp+10h] [ebp-28h]
    float vAngOfs_4; // [esp+14h] [ebp-24h]
    float vAngOfs_8; // [esp+18h] [ebp-20h]
    float fBobCycle; // [esp+1Ch] [ebp-1Ch]
    float cycle; // [esp+20h] [ebp-18h]
    float speed; // [esp+24h] [ebp-14h]
    float speeda; // [esp+24h] [ebp-14h]
    int weapIndex; // [esp+28h] [ebp-10h]
    float fPositionLerp; // [esp+2Ch] [ebp-Ch]
    playerState_s *ps; // [esp+30h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+34h] [ebp-4h]

    ps = vs->ps;
    weapIndex = BG_GetViewmodelWeaponIndex(vs->ps);
    weapDef = BG_GetWeaponDef(weapIndex);
    if ( weapDef->overlayReticle )
    {
        fBobCycle = BG_GetBobCycle(ps);
        cycle = (float)((float)(3.1415927 * bg_weaponBobLag->current.value) + fBobCycle) + 6.2831855;
        speed = vs->xyspeed * bg_weaponBobAmplitudeBase->current.value;
        vAngOfs = BG_CalculateWeaponMovement_VerticalBobFactor(ps, cycle, speed) * -1.0;
        vAngOfs_4 = BG_CalculateWeaponMovement_HorizontalBobFactor(ps, cycle, speed) * -1.0;
        v3 = BG_CalculateWeaponMovement_HorizontalBobFactor(
                     ps,
                     cycle - 0.47123894,
                     speed * bg_weaponBobAmplitudeRoll->current.value);
        if ( (float)(0.0 - v3) < 0.0 )
            v2 = 0.0f;
        else
            v2 = v3;
        vAngOfs_8 = v2;
        fPositionLerp = ps->fWeaponPosFrac;
        if ( fPositionLerp != 0.0 )
        {
            speeda = 1.0 - (float)((float)(1.0 - weapDef->fAdsBobFactor) * fPositionLerp);
            vAngOfs = vAngOfs * speeda;
            vAngOfs_4 = vAngOfs_4 * speeda;
            vAngOfs_8 = v2 * speeda;
        }
        *angles = *angles + (float)(fPositionLerp * vAngOfs);
        angles[1] = angles[1] + (float)(fPositionLerp * vAngOfs_4);
        angles[2] = angles[2] + (float)(fPositionLerp * vAngOfs_8);
    }
}

void __cdecl BG_CalculateViewMovement_AdsBob(viewState_t *vs, float *angles)
{
    float bobCycle; // [esp+8h] [ebp-14h]
    int weapIndex; // [esp+Ch] [ebp-10h]
    float adsBobFactor; // [esp+10h] [ebp-Ch]
    playerState_s *ps; // [esp+14h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+18h] [ebp-4h]

    ps = vs->ps;
    if ( vs->ps->fWeaponPosFrac > 0.0 && (ps->eFlags & 0x300) == 0 )
    {
        weapIndex = BG_GetViewmodelWeaponIndex(ps);
        weapDef = BG_GetWeaponDef(weapIndex);
        if ( weapDef->fAdsViewBobMult > 0.0 )
        {
            adsBobFactor = ps->fWeaponPosFrac * weapDef->fAdsViewBobMult;
            bobCycle = BG_GetBobCycle(ps);
            *angles = *angles - BG_CalculateViewMovement_VerticalBobFactor(ps, bobCycle, vs->xyspeed) * adsBobFactor;
            angles[1] = angles[1] - BG_CalculateViewMovement_HorizontalBobFactor(ps, bobCycle, vs->xyspeed) * adsBobFactor;
        }
    }
}

