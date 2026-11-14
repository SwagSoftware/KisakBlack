#include "cg_newDraw_mp.h"
#include <client_mp/cl_main_mp.h>
#include <cgame/cg_drawtools.h>
#include <bgame/bg_weapons_ammo.h>
#include <bgame/bg_weapons_def.h>
#include <cgame/cg_ammocounter.h>
#include <bgame/bg_mantle.h>
#include <client_mp/cl_cgame_mp.h>
#include "cg_main_mp.h"
#include <cgame/cg_compass.h>
#include <cgame/offhandweapons.h>
#include <ui/ui_atoms.h>
#include <bgame/bg_misc.h>
#include <bgame/bg_pmove.h>
#include <demo/demo_playback.h>
#include "cg_vehicles_mp.h"
#include <qcommon/dobj_management.h>
#include "cg_ents_mp.h"
#include <clientscript/scr_const.h>
#include <universal/com_shared.h>
#include <demo/demo_ui.h>
#include "cg_scoreboard_mp.h"
#include <win32/win_shared.h>
#include <cgame/cg_hudelem.h>
#include "cg_draw_mp.h"
#include <client/cl_keys.h>
#include <qcommon/com_clients.h>
#include <stringed/stringed_hooks.h>
#include <qcommon/common.h>

const char *cg_drawTalkNames[5] =
{ "NONE", "ALL", "FRIENDLY", "ENEMY", NULL };


const dvar_t *hud_fadeout_speed;
const dvar_t *hud_enable;
const dvar_t *hud_fade_ammodisplay;
const dvar_t *hud_fade_healthbar;
const dvar_t *hud_fade_compass;
const dvar_t *hud_fade_stance;
const dvar_t *hud_fade_offhand;
const dvar_t *hud_fade_sprint;
const dvar_t *hud_fade_vehiclecontrols;
const dvar_t *hud_health_startpulse_injured;
const dvar_t *hud_health_startpulse_critical;
const dvar_t *hud_health_pulserate_injured;
const dvar_t *hud_health_pulserate_critical;
const dvar_t *hud_deathQuoteFadeTime;
const dvar_t *hud_healthOverlay_regenPauseTime;
const dvar_t *hud_healthOverlay_pulseStart;
const dvar_t *hud_healthOverlay_pulseStop;
const dvar_t *hud_healthOverlay_phaseOne_toAlphaAdd;
const dvar_t *hud_healthOverlay_phaseOne_pulseDuration;
const dvar_t *hud_healthOverlay_phaseTwo_toAlphaMultiplier;
const dvar_t *hud_healthOverlay_phaseTwo_pulseDuration;
const dvar_t *hud_healthOverlay_phaseThree_toAlphaMultiplier;
const dvar_t *hud_healthOverlay_phaseThree_pulseDuration;
const dvar_t *hud_healthOverlay_phaseEnd_fromAlpha;
const dvar_t *hud_healthOverlay_phaseEnd_toAlpha;
const dvar_t *hud_healthOverlay_phaseEnd_pulseDuration;
const dvar_t *cg_sprintMeterFullColor;
const dvar_t *cg_sprintMeterEmptyColor;
const dvar_t *cg_sprintMeterDisabledColor;
const dvar_t *cg_drawTalk;

void __cdecl CG_AntiBurnInHUD_RegisterDvars()
{
    hud_fadeout_speed = _Dvar_RegisterFloat("hud_fadeout_speed", 0.1, 0.0, 1.0, 1u, "The speed that the HUD will fade at");
    hud_enable = _Dvar_RegisterBool("hud_enable", 1, 1u, "Enable hud elements");
    hud_fade_ammodisplay = _Dvar_RegisterFloat(
                                                     "hud_fade_ammodisplay",
                                                     0.0,
                                                     0.0,
                                                     30.0,
                                                     1u,
                                                     "The time for the ammo display to fade in seconds");
    hud_fade_healthbar = _Dvar_RegisterFloat(
                                                 "hud_fade_healthbar",
                                                 2.0,
                                                 0.0,
                                                 30.0,
                                                 1u,
                                                 "The time for the health bar to fade in seconds");
    hud_fade_compass = _Dvar_RegisterFloat(
                                             "hud_fade_compass",
                                             0.0,
                                             0.0,
                                             30.0,
                                             1u,
                                             "The time for the compass to fade in seconds");
    hud_fade_stance = _Dvar_RegisterFloat(
                                            "hud_fade_stance",
                                            1.7,
                                            0.0,
                                            30.0,
                                            1u,
                                            "The time for the stance to fade in seconds");
    hud_fade_offhand = _Dvar_RegisterFloat(
                                             "hud_fade_offhand",
                                             0.0,
                                             0.0,
                                             30.0,
                                             1u,
                                             "The time for the offhand weapons to fade in seconds");
    hud_fade_sprint = _Dvar_RegisterFloat(
                                            "hud_fade_sprint",
                                            1.7,
                                            0.0,
                                            30.0,
                                            1u,
                                            "The time for the sprint meter to fade in seconds");
    hud_fade_vehiclecontrols = _Dvar_RegisterFloat(
                                                             "hud_fade_vehiclecontrols",
                                                             4.0,
                                                             0.0,
                                                             30.0,
                                                             1u,
                                                             "The time for the vehicle controls to fade in seconds");
    hud_health_startpulse_injured = _Dvar_RegisterFloat(
                                                                        "hud_health_startpulse_injured",
                                                                        1.0,
                                                                        0.0,
                                                                        1.1,
                                                                        1u,
                                                                        "The health level at which to start the 'injured' pulse effect");
    hud_health_startpulse_critical = _Dvar_RegisterFloat(
                                                                         "hud_health_startpulse_critical",
                                                                         0.33000001,
                                                                         0.0,
                                                                         1.1,
                                                                         1u,
                                                                         "The health level at which to start the 'critical' pulse effect");
    hud_health_pulserate_injured = _Dvar_RegisterFloat(
                                                                     "hud_health_pulserate_injured",
                                                                     1.0,
                                                                     0.1,
                                                                     3.0,
                                                                     1u,
                                                                     "The pulse rate of the 'injured' pulse effect");
    hud_health_pulserate_critical = _Dvar_RegisterFloat(
                                                                        "hud_health_pulserate_critical",
                                                                        0.5,
                                                                        0.1,
                                                                        3.0,
                                                                        1u,
                                                                        "The pulse rate of the 'critical' pulse effect");
    hud_deathQuoteFadeTime = _Dvar_RegisterInt(
                                                         "hud_deathQuoteFadeTime",
                                                         1000,
                                                         0,
                                                         100000,
                                                         1u,
                                                         "The time for the death quote to fade");
    hud_healthOverlay_regenPauseTime = _Dvar_RegisterInt(
                                                                             "hud_healthOverlay_regenPauseTime",
                                                                             8000,
                                                                             0,
                                                                             10000,
                                                                             0x80u,
                                                                             "The time in milliseconds before the health regeneration kicks in");
    hud_healthOverlay_pulseStart = _Dvar_RegisterFloat(
                                                                     "hud_healthOverlay_pulseStart",
                                                                     0.55000001,
                                                                     0.0,
                                                                     1.0,
                                                                     0x80u,
                                                                     "The percentage of full health at which the low-health warning overlay begins flashing");
    hud_healthOverlay_pulseStop = _Dvar_RegisterFloat(
                                                                    "hud_healthOverlay_pulseStop",
                                                                    0.89999998,
                                                                    0.0,
                                                                    1.0,
                                                                    0x80u,
                                                                    "The percentage of full health above which the health overlay fades out");
    hud_healthOverlay_phaseOne_toAlphaAdd = _Dvar_RegisterFloat(
                                                                                        "hud_healthOverlay_phaseOne_toAlphaAdd",
                                                                                        0.30000001,
                                                                                        0.0,
                                                                                        1.0,
                                                                                        0x80u,
                                                                                        "Alpha value to add to the first health overlay phase");
    hud_healthOverlay_phaseOne_pulseDuration = _Dvar_RegisterInt(
                                                                                             "hud_healthOverlay_phaseOne_pulseDuration",
                                                                                             150,
                                                                                             0,
                                                                                             1000,
                                                                                             0x80u,
                                                                                             "Pulse duration of the first health overlay phase");
    hud_healthOverlay_phaseTwo_toAlphaMultiplier = _Dvar_RegisterFloat(
                                                                                                     "hud_healthOverlay_phaseTwo_toAlphaMultiplier",
                                                                                                     1.0,
                                                                                                     0.0,
                                                                                                     1.0,
                                                                                                     0x80u,
                                                                                                     "Alpha multiplier for the second health overlay phase (percentage of the pulse peak)");
    hud_healthOverlay_phaseTwo_pulseDuration = _Dvar_RegisterInt(
                                                                                             "hud_healthOverlay_phaseTwo_pulseDuration",
                                                                                             320,
                                                                                             0,
                                                                                             1000,
                                                                                             0x80u,
                                                                                             "Time in milliseconds to fade the alpha to hud_healthOverlay_phaseTwo_toAlphaMultiplier");
    hud_healthOverlay_phaseThree_toAlphaMultiplier = _Dvar_RegisterFloat(
                                                                                                         "hud_healthOverlay_phaseThree_toAlphaMultiplier",
                                                                                                         1.0,
                                                                                                         0.0,
                                                                                                         1.0,
                                                                                                         0x80u,
                                                                                                         "Alpha multiplier for the third health overlay phase (percentage of the pulse peak)");
    hud_healthOverlay_phaseThree_pulseDuration = _Dvar_RegisterInt(
                                                                                                 "hud_healthOverlay_phaseThree_pulseDuration",
                                                                                                 400,
                                                                                                 0,
                                                                                                 1000,
                                                                                                 0x80u,
                                                                                                 "Time in milliseconds to fade the alpha to hud_healthOverlay_phaseThree_"
                                                                                                 "toAlphaMultiplier");
    hud_healthOverlay_phaseEnd_fromAlpha = _Dvar_RegisterFloat(
                                                                                     "hud_healthOverlay_phaseEnd_fromAlpha",
                                                                                     0.2,
                                                                                     0.0,
                                                                                     1.0,
                                                                                     0x80u,
                                                                                     "Starting alpha of the final health overlay phase");
    hud_healthOverlay_phaseEnd_toAlpha = _Dvar_RegisterFloat(
                                                                                 "hud_healthOverlay_phaseEnd_toAlpha",
                                                                                 0.0,
                                                                                 0.0,
                                                                                 1.0,
                                                                                 0x80u,
                                                                                 "Alpha multiplier to fade to before turning off the overlay (percentage of the pulse peak)");
    hud_healthOverlay_phaseEnd_pulseDuration = _Dvar_RegisterInt(
                                                                                             "hud_healthOverlay_phaseEnd_pulseDuration",
                                                                                             700,
                                                                                             0,
                                                                                             1000,
                                                                                             0x80u,
                                                                                             "Time in milliseconds to fade out the health overlay after it is done flashing");
    cg_sprintMeterFullColor = _Dvar_RegisterVec4(
                                                            "cg_sprintMeterFullColor",
                                                            (0.80000001),
                                                            (0.80000001),
                                                            (0.80000001),
                                                            (0.80000001),
                                                            0.0,
                                                            1.0,
                                                            1u,
                                                            "The color of the sprint meter when the sprint meter is full");
    cg_sprintMeterEmptyColor = _Dvar_RegisterVec4(
                                                             "cg_sprintMeterEmptyColor",
                                                             (0.69999999),
                                                             (0.5),
                                                             (0.2),
                                                             (0.80000001),
                                                             0.0,
                                                             1.0,
                                                             1u,
                                                             "The color of the sprint meter when the sprint meter is empty");
    cg_sprintMeterDisabledColor = _Dvar_RegisterVec4(
                                                                    "cg_sprintMeterDisabledColor",
                                                                    (0.80000001),
                                                                    (0.1),
                                                                    (0.1),
                                                                    (0.2),
                                                                    0.0,
                                                                    1.0,
                                                                    1u,
                                                                    "The color of the sprint meter when the sprint meter is disabled");
    cg_drawTalk = _Dvar_RegisterEnum(
                                    "cg_drawTalk",
                                    cg_drawTalkNames,
                                    1,
                                    0,
                                    "Controls which icons CG_TALKER ownerdraw draws");
}

bool __cdecl CG_ShouldDrawHud(int localClientNum)
{
    cg_s *cgameGlob; // [esp+4h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    return cgameGlob && cgameGlob->drawHud && CL_ShouldDisplayHud(localClientNum);
}

double __cdecl CG_FadeHudMenu(int localClientNum, const dvar_s *fadeDvar, int displayStartTime, int duration)
{
    cg_s *LocalClientGlobals; // eax
    float *fadeColor; // [esp+4h] [ebp-4h]

    if ( !hud_enable->current.enabled )
        return 0.0;
    if ( !CG_ShouldDrawHud(localClientNum) )
        return 0.0;
    if ( fadeDvar->current.value == 0.0 )
        return 1.0;
    LocalClientGlobals = CG_GetLocalClientGlobals(localClientNum);
    fadeColor = CG_FadeColor(LocalClientGlobals->time, displayStartTime, duration, 700);
    if ( fadeColor )
        return fadeColor[3];
    else
        return 0.0;
}

bool __cdecl CG_CheckPlayerForLowAmmoSpecific(const cg_s *cgameGlob, unsigned int weapIndex)
{
    int maxAmmo; // [esp+4h] [ebp-10h]
    int curAmmo; // [esp+8h] [ebp-Ch]
    const playerState_s *ps; // [esp+Ch] [ebp-8h]

    ps = &cgameGlob->predictedPlayerState;
    if ( !weapIndex )
        return 0;
    BG_AmmoForWeapon(weapIndex);
    curAmmo = BG_GetTotalAmmoReserve(ps, weapIndex);
    if ( curAmmo > 999 )
        curAmmo = 999;
    BG_GetWeaponDef(weapIndex);
    maxAmmo = BG_GetAmmoPlayerMax(ps, weapIndex, 0);
    if ( maxAmmo > 999 )
        maxAmmo = 999;
    return maxAmmo >= 0 && (float)((float)maxAmmo * 0.2) >= (float)curAmmo;
}

bool __cdecl CG_CheckPlayerForLowAmmo(const cg_s *cgameGlob)
{
    int weapIndex; // [esp+0h] [ebp-4h]

    weapIndex = GetWeaponIndex(cgameGlob);
    return CG_CheckPlayerForLowAmmoSpecific(cgameGlob, weapIndex);
}

bool __cdecl CG_CheckPlayerForLowClipSpecific(const cg_s *cgameGlob, unsigned int weapIndex)
{
    int curClipVal; // [esp+20h] [ebp-10h]
    const WeaponDef *weapDef; // [esp+28h] [ebp-8h]
    int fullClipVal; // [esp+2Ch] [ebp-4h]

    if ( !weapIndex )
        return 0;
    if ( BG_WeaponIsClipOnly(weapIndex) )
        return 0;
    curClipVal = BG_GetAmmoInClip(&cgameGlob->predictedPlayerState, weapIndex);
    if ( curClipVal < 0 )
        return 0;
    if ( curClipVal > 999 )
        curClipVal = 999;
    fullClipVal = BG_GetClipSize(weapIndex);
    if ( fullClipVal > 999 )
        fullClipVal = 999;
    weapDef = BG_GetWeaponDef(weapIndex);
    return fullClipVal > 0 && (float)((float)fullClipVal * weapDef->lowAmmoWarningThreshold) >= (float)curClipVal;
}

bool __cdecl CG_CheckPlayerForLowClip(const cg_s *cgameGlob)
{
    int weapIndex; // [esp+0h] [ebp-4h]

    weapIndex = GetWeaponIndex(cgameGlob);
    return CG_CheckPlayerForLowClipSpecific(cgameGlob, weapIndex);
}

double __cdecl CG_CalcPlayerHealth(const playerState_s *ps)
{
    float v3; // [esp+4h] [ebp-8h]
    float health; // [esp+8h] [ebp-4h]

    if ( !ps->stats[0] || !ps->stats[2] || ps->pm_type == 9 )
        return 0.0;
    health = (float)ps->stats[0] / (float)ps->stats[2];
    if ( (float)(health - 1.0) < 0.0 )
        v3 = (float)ps->stats[0] / (float)ps->stats[2];
    else
        v3 = 1.0f;
    if ( (float)(0.0 - health) < 0.0 )
        return v3;
    else
        return 0.0f;
}

void __cdecl CG_ResetLowHealthOverlay(cg_s *cgameGlob)
{
    cgameGlob->healthOverlayHurt = 0;
    cgameGlob->healthOverlayToAlpha = hud_healthOverlay_phaseEnd_toAlpha->current.value;
    cgameGlob->healthOverlayPulseDuration = 0;
    cgameGlob->healthOverlayPulsePhase = 0;
    cgameGlob->healthOverlayPulseIndex = 0;
    cgameGlob->healthOverlayOldHealth = 1.0f;
}

void __cdecl CG_ResetDirectionalDamageIndicators(int localClientNum, int time)
{
    int i; // [esp+0h] [ebp-8h]
    cg_s *cgameGlob; // [esp+4h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    for ( i = 0; i < 4; ++i )
    {
        if ( cgameGlob->directionalHitIndicator[i].time > time )
            cgameGlob->directionalHitIndicator[i].time = 0;
    }
}

int __cdecl CG_ServerMaterialName(int localClientNum, int index, char *materialName, unsigned int maxLen)
{
    char v5; // al
    char *v7; // [esp+Ch] [ebp-18h]
    char *string; // [esp+20h] [ebp-4h]

    if ( index <= 0 || index >= 256 )
        return 0;
    string = CL_GetConfigString(index + 2580);
    if ( !*string )
        return 0;
    if ( strlen(string) >= maxLen )
        return 0;
    v7 = string;
    do
    {
        v5 = *v7;
        *materialName++ = *v7++;
    }
    while ( v5 );
    return 1;
}

Material *__cdecl CG_ObjectiveIcon(int localClientNum, int icon, int type)
{
    char shaderName[68]; // [esp+0h] [ebp-48h] BYREF

    if ( type
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_newDraw_mp.cpp",
                    1540,
                    0,
                    "%s",
                    "type >= 0 && static_cast<unsigned int>( type ) < ARRAY_COUNT( cgMedia.objectiveMaterials )") )
    {
        __debugbreak();
    }
    if ( icon && CG_ServerMaterialName(localClientNum, icon, shaderName, 0x40u) )
        return Material_RegisterHandle(shaderName, 7);
    else
        return cgMedia.objectiveMaterials[type];
}

char *__cdecl CG_ScriptMainMenu(int localClientNum)
{
    return CG_GetLocalClientGlobals(localClientNum)->scriptMainMenu;
}

void __cdecl CG_OwnerDraw(
                int localClientNum,
                rectDef_s parentRect,
                float x,
                float y,
                float w,
                float h,
                int horzAlign,
                int vertAlign,
                int ownerDraw,
                int ownerDrawFlags,
                float *color,
                Material *material)
{
    bool v12; // [esp+8h] [ebp-30h]
    const cg_s *cgameGlob; // [esp+14h] [ebp-24h]
    clientUIActive_t *clUI; // [esp+18h] [ebp-20h]
    rectDef_s rect; // [esp+1Ch] [ebp-1Ch] BYREF
    const playerState_s *ps; // [esp+34h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    clUI = CL_GetLocalClientUIGlobals(localClientNum);
    ps = CG_GetPredictedPlayerState(localClientNum);
    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_newDraw_mp.cpp", 3869, 0, "%s", "ps") )
        __debugbreak();
    if ( ps->offhandSecondary >= (unsigned int)PLAYER_OFFHAND_SECONDARIES_TOTAL
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_newDraw_mp.cpp",
                    3877,
                    0,
                    "%s",
                    "ps->offhandSecondary == PLAYER_OFFHAND_SECONDARY_FLASH") )
    {
        __debugbreak();
    }
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    rect.horzAlign = horzAlign;
    rect.vertAlign = vertAlign;
    v12 = (clUI->keyCatchers & 0x10) != 0 && UI_GetActiveMenu(localClientNum) == UIMENU_INGAME;
    if ( ownerDraw > 284 )
    {
        switch ( ownerDraw )
        {
            case 285:
                if ( !cgameGlob->inKillCam )
                    CG_DrawWarMomentumMultiplier(localClientNum, &rect, color, material);
                break;
            case 286:
                if ( !cgameGlob->inKillCam )
                    CG_DrawWarMomentumMultiplierDetail(localClientNum, &rect, color, material);
                break;
            case 287:
                if ( !cgameGlob->inKillCam )
                    CG_DrawWarMomentumMultiplierBlitzkrieg(localClientNum, &rect, color, material);
                break;
            case 290:
            case 291:
            case 292:
            case 293:
                CG_DrawPlayerActionSlotBack(localClientNum, &rect, ownerDraw - 290, color, material);
                break;
            case 294:
            case 295:
            case 296:
            case 297:
                CG_DrawPlayerActionSlotArrow(localClientNum, &rect, ownerDraw - 294, color, material);
                break;
            case 312:
                CG_CompassDrawFakeFire(localClientNum, COMPASS_TYPE_PARTIAL, &parentRect, &rect, color);
                break;
            case 316:
                CG_DrawWristWatch(localClientNum, &parentRect, &rect, material, color);
                break;
            case 317:
                if ( Dvar_GetBool("compassGridEnabled") )
                    CG_CompassDrawPlayerMap(localClientNum, COMPASS_TYPE_PARTIAL, &parentRect, &rect, material, color, 1);
                break;
            case 324:
                CG_CompassDrawScrambler(localClientNum, COMPASS_TYPE_PARTIAL, &parentRect, &rect, material, color);
                break;
            case 325:
                CG_CompassDrawScramblerFriendly(localClientNum, COMPASS_TYPE_PARTIAL, &parentRect, &rect, color);
                break;
            case 328:
                CG_CompassDrawStatic(localClientNum, COMPASS_TYPE_PARTIAL, &parentRect, &rect, material, color);
                break;
            case 330:
                CG_DrawPlayerDirectionalHitIndicator(localClientNum, &rect, material, color);
                break;
            case 331:
                CG_CompassDrawRound(localClientNum, &rect, material, color);
                break;
            case 332:
                CG_CompassDrawRadarEffects(localClientNum, COMPASS_TYPE_PARTIAL, &parentRect, &rect, color);
                break;
            case 335:
            case 379:
                CG_DrawPlayerTargetHighlights(localClientNum, &rect, material, color);
                break;
            case 339:
                CG_DrawPlayerImageSwing(localClientNum, &rect, &parentRect, material, color);
                break;
            case 351:
                CG_DrawDemoControls(localClientNum, &rect, material, color);
                break;
            case 376:
                CG_CompassDrawFlicker(localClientNum, COMPASS_TYPE_PARTIAL, &parentRect, &rect, color, material);
                break;
            case 391:
                CG_DrawPlayerTargetHighlightsFriendly(localClientNum, &rect, material, color);
                break;
            default:
LABEL_111:
                if ( !cgameGlob->inKillCam )
                {
                    switch ( ownerDraw )
                    {
                        case 176:
                            CG_CompassDrawDogs(
                                localClientNum,
                                COMPASS_TYPE_FULL,
                                17,
                                &parentRect,
                                &rect,
                                cgMedia.compass_dogs_enemy,
                                color);
                            break;
                        case 177:
                            CG_CompassDrawVehicles(localClientNum, COMPASS_TYPE_FULL, 14, &parentRect, &rect, color);
                            CG_CompassDrawVehicles(localClientNum, COMPASS_TYPE_FULL, 13, &parentRect, &rect, color);
                            CG_CompassDrawHelicopter(localClientNum, COMPASS_TYPE_FULL, 12, &parentRect, &rect, color);
                            break;
                        case 178:
                            CG_CompassDrawArtilleryIcon(localClientNum, COMPASS_TYPE_FULL, &parentRect, &rect, color);
                            CG_CompassDrawIncomingArtilleryIcon(localClientNum, COMPASS_TYPE_FULL, &parentRect, &rect, color);
                            break;
                        case 180:
                            CG_CompassDrawPlayerBack(localClientNum, COMPASS_TYPE_FULL, &parentRect, &rect, material, color);
                            break;
                        case 181:
                            CG_CompassDrawPlayerMap(localClientNum, COMPASS_TYPE_FULL, &parentRect, &rect, material, color, 0);
                            break;
                        case 182:
                            CG_CompassDrawPlayerPointers_MiniMap(
                                localClientNum,
                                COMPASS_TYPE_FULL,
                                &parentRect,
                                &rect,
                                material,
                                color);
                            break;
                        case 183:
                            CG_CompassDrawPlayer(localClientNum, COMPASS_TYPE_FULL, &parentRect, &rect, material, color);
                            break;
                        case 185:
                            CG_CompassDrawFriendlies(localClientNum, COMPASS_TYPE_FULL, &parentRect, &rect, color);
                            break;
                        case 186:
                            CG_CompassDrawPlayerSelectedLocations(
                                localClientNum,
                                COMPASS_TYPE_FULL,
                                &parentRect,
                                &rect,
                                material,
                                color);
                            CG_CompassDrawPlayerMapLocationSelector(
                                localClientNum,
                                COMPASS_TYPE_FULL,
                                &parentRect,
                                &rect,
                                material,
                                color);
                            break;
                        case 187:
                            CG_CompassDrawBorder(localClientNum, COMPASS_TYPE_FULL, &parentRect, &rect, material, color);
                            break;
                        case 188:
                            CG_CompassDrawEnemies(localClientNum, COMPASS_TYPE_FULL, &parentRect, &rect, color);
                            break;
                        case 189:
                            CG_CompassDrawTurrets(localClientNum, COMPASS_TYPE_FULL, 11, &parentRect, &rect, color);
                            break;
                        case 190:
                            CG_CompassDrawGuidedMissile(localClientNum, COMPASS_TYPE_FULL, 4, &parentRect, &rect, color);
                            break;
                        case 313:
                            CG_CompassDrawFakeFire(localClientNum, COMPASS_TYPE_FULL, &parentRect, &rect, color);
                            break;
                        case 318:
                            if ( Dvar_GetBool("compassGridEnabled") )
                                CG_CompassDrawPlayerMap(localClientNum, COMPASS_TYPE_FULL, &parentRect, &rect, material, color, 1);
                            break;
                        case 326:
                            CG_CompassDrawScrambler(localClientNum, COMPASS_TYPE_FULL, &parentRect, &rect, material, color);
                            break;
                        case 327:
                            CG_CompassDrawScramblerFriendly(localClientNum, COMPASS_TYPE_FULL, &parentRect, &rect, color);
                            break;
                        case 333:
                            CG_CompassDrawRadarEffects(localClientNum, COMPASS_TYPE_FULL, &parentRect, &rect, color);
                            break;
                        case 360:
                            CG_CompassDrawStatic(localClientNum, COMPASS_TYPE_FULL, &parentRect, &rect, material, color);
                            break;
                        default:
                            return;
                    }
                }
                break;
        }
    }
    else if ( ownerDraw == 284 )
    {
$LN38_2:
        if ( !cgameGlob->inKillCam )
            CG_DrawWarMomentumProgress(localClientNum, &rect, color, material, ownerDraw);
    }
    else
    {
        switch ( ownerDraw )
        {
            case 6:
                CG_DrawPlayerAmmoBackdrop(localClientNum, &rect, color, material);
                return;
            case 7:
                if ( !v12 )
                    CG_DrawPlayerHeatValue(localClientNum, &rect, color, 0);
                return;
            case 8:
                if ( !v12 )
                    CG_DrawPlayerHeatValue(localClientNum, &rect, color, 1);
                return;
            case 9:
                if ( !v12 )
                    CG_DrawPlayerReloadValue(localClientNum, &rect, color, material);
                return;
            case 10:
                if ( !v12 )
                    CG_DrawPlayerReloadedValue(localClientNum, &rect, color, material);
                return;
            case 11:
                if ( !v12 )
                    CG_DrawPlayerFuelAmmoValue(localClientNum, &rect, color);
                return;
            case 12:
                CG_DrawGuidedMissileFuel(localClientNum, &rect, material);
                return;
            case 79:
                goto $LN46_1;
            case 91:
                if ( !v12 )
                    draw_tank_body_component(localClientNum, &rect, material, color, _tank_component_left_tread);
                return;
            case 92:
                if ( !v12 )
                    draw_tank_body_component(localClientNum, &rect, material, color, _tank_component_right_tread);
                return;
            case 93:
                if ( !v12 )
                    draw_tank_body_component(localClientNum, &rect, material, color, _tank_component_driver_seat);
                return;
            case 94:
                if ( !v12 )
                    draw_tank_turret_component(localClientNum, &rect, material, color, _tank_component_gunner_seat);
                return;
            case 95:
                if ( !v12 )
                    draw_tank_body_component(localClientNum, &rect, material, color, _tank_component_core);
                return;
            case 96:
                if ( !v12 )
                    draw_tank_turret_component(localClientNum, &rect, material, color, _tank_component_turret);
                return;
            case 98:
                CG_DrawPlayerBarHealthBack(localClientNum, &rect, material, color);
                return;
            case 103:
                CG_DrawOffHandIcon(localClientNum, &rect, color, material, OFFHAND_SLOT_LETHAL_GRENADE);
                return;
            case 104:
                CG_DrawOffHandIcon(localClientNum, &rect, color, material, OFFHAND_SLOT_TACTICAL_GRENADE);
                return;
            case 109:
                CG_DrawOffHandHighlight(localClientNum, &rect, color, material, OFFHAND_SLOT_LETHAL_GRENADE);
                return;
            case 110:
                CG_DrawOffHandHighlight(localClientNum, &rect, color, material, OFFHAND_SLOT_TACTICAL_GRENADE);
                return;
            case 112:
                CG_DrawPlayerLowHealthOverlay(localClientNum, &rect, material, color);
                return;
            case 114:
                CG_DrawPlayerSprintMeter(localClientNum, &rect, material, color);
                return;
            case 115:
                CG_DrawPlayerSprintBack(localClientNum, &rect, material, color);
                return;
            case 116:
                CG_DrawPlayerWeaponBackground(localClientNum, &rect, color, material);
                return;
            case 117:
                CG_DrawPlayerWeaponIcon(localClientNum, &rect, color);
                return;
            case 118:
                CG_DrawPlayerWeaponAmmoClipGraphic(localClientNum, &rect, color);
                return;
            case 119:
                CG_DrawPlayerWeaponAmmoClipGraphicDualWield(localClientNum, &rect, color);
                return;
            case 126:
                CG_DrawLifeCounter(localClientNum, &rect, color, 1);
                return;
            case 127:
                CG_DrawLifeCounter(localClientNum, &rect, color, 0);
                return;
            case 128:
                CG_DrawAmmoCounterSingle(localClientNum, &rect, color, 0);
                return;
            case 129:
                CG_DrawAmmoCounterSingle(localClientNum, &rect, color, 1);
$LN46_1:
                CG_DrawPlayerBarHealth(localClientNum, &rect, material, color);
                break;
            case 146:
                CG_CompassDrawGuidedMissile(localClientNum, COMPASS_TYPE_PARTIAL, 4, &parentRect, &rect, color);
                break;
            case 147:
                CG_CompassDrawDogs(
                    localClientNum,
                    COMPASS_TYPE_PARTIAL,
                    17,
                    &parentRect,
                    &rect,
                    cgMedia.compass_dogs_enemy,
                    color);
                break;
            case 148:
                CG_CompassDrawTurrets(localClientNum, COMPASS_TYPE_PARTIAL, 11, &parentRect, &rect, color);
                break;
            case 149:
                CG_CompassDrawArtilleryIcon(localClientNum, COMPASS_TYPE_PARTIAL, &parentRect, &rect, color);
                CG_CompassDrawIncomingArtilleryIcon(localClientNum, COMPASS_TYPE_PARTIAL, &parentRect, &rect, color);
                break;
            case 150:
                CG_CompassDrawPlayer(localClientNum, COMPASS_TYPE_PARTIAL, &parentRect, &rect, material, color);
                break;
            case 151:
                CG_CompassDrawPlayerBack(localClientNum, COMPASS_TYPE_PARTIAL, &parentRect, &rect, material, color);
                break;
            case 152:
                CG_CompassDrawPlayerPointers_MiniMap(localClientNum, COMPASS_TYPE_PARTIAL, &parentRect, &rect, material, color);
                break;
            case 154:
                CG_CompassDrawVehicles(localClientNum, COMPASS_TYPE_PARTIAL, 14, &parentRect, &rect, color);
                break;
            case 155:
                CG_CompassDrawHelicopter(localClientNum, COMPASS_TYPE_PARTIAL, 12, &parentRect, &rect, color);
                break;
            case 156:
                CG_CompassDrawVehicles(localClientNum, COMPASS_TYPE_PARTIAL, 13, &parentRect, &rect, color);
                break;
            case 158:
                CG_CompassDrawFriendlies(localClientNum, COMPASS_TYPE_PARTIAL, &parentRect, &rect, color);
                break;
            case 159:
                CG_CompassDrawPlayerMap(localClientNum, COMPASS_TYPE_PARTIAL, &parentRect, &rect, material, color, 0);
                break;
            case 165:
                CG_DrawPlayerActionSlotDpad(localClientNum, &rect, color, material);
                break;
            case 170:
                CG_CompassDrawEnemies(localClientNum, COMPASS_TYPE_PARTIAL, &parentRect, &rect, color);
                break;
            case 171:
            case 172:
            case 173:
            case 174:
                goto $LN38_2;
            default:
                goto LABEL_111;
        }
    }
}

clientUIActive_t *__cdecl CL_GetLocalClientUIGlobals(int localClientNum)
{
    if ( localClientNum
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\live\\../client/client.h",
                    160,
                    0,
                    "%s\n\t(localClientNum) = %i",
                    "(localClientNum == 0)",
                    localClientNum) )
    {
        __debugbreak();
    }
    return clientUIActives;
}

void __cdecl CG_DrawPlayerAmmoBackdrop(
                int localClientNum,
                const rectDef_s *rect,
                const float *color,
                Material *material)
{
    const cg_s *cgameGlob; // [esp+34h] [ebp-14h]
    float drawColor[4]; // [esp+38h] [ebp-10h] BYREF

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->predictedPlayerState.weapon )
    {
        drawColor[3] = CG_FadeHudMenu(
                                         localClientNum,
                                         hud_fade_ammodisplay,
                                         cgameGlob->ammoFadeTime,
                                         SnapFloatToInt((float)(hud_fade_ammodisplay->current.value * 1000.0)));
        if ( drawColor[3] != 0.0 )
        {
            if ( CG_CheckPlayerForLowAmmo(cgameGlob) )
            {
                drawColor[0] = 0.89f;
                drawColor[1] = 0.18000001;
                drawColor[2] = 0.01f;
            }
            else
            {
                drawColor[0] = *color;
                drawColor[1] = color[1];
                drawColor[2] = color[2];
            }
            UI_DrawHandlePic(
                &scrPlaceView[localClientNum],
                rect->x,
                rect->y,
                rect->w,
                rect->h,
                rect->horzAlign,
                rect->vertAlign,
                drawColor,
                material);
        }
    }
}

void __cdecl CG_DrawPlayerHeatValue(
                int localClientNum,
                const rectDef_s *rect,
                float *color,
                bool drawVehicleTurretVersion)
{
    unsigned int v4; // [esp+30h] [ebp-68h]
    int weapon; // [esp+34h] [ebp-64h]
    bool v6; // [esp+3Bh] [ebp-5Dh]
    float heatValue; // [esp+54h] [ebp-44h]
    const WeaponDef *weaponDefinition; // [esp+58h] [ebp-40h]
    float flashColor[4]; // [esp+5Ch] [ebp-3Ch] BYREF
    float iconColor[4]; // [esp+6Ch] [ebp-2Ch] BYREF
    bool overheating; // [esp+7Fh] [ebp-19h]
    int weaponIndex; // [esp+84h] [ebp-14h]
    bool local_client_is_gunner; // [esp+8Ah] [ebp-Eh]
    bool render; // [esp+8Bh] [ebp-Dh]
    cg_s *cgameGlob; // [esp+8Ch] [ebp-Ch]
    const centity_s *cent; // [esp+90h] [ebp-8h]
    const playerState_s *ps; // [esp+94h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    cent = CG_GetEntity(localClientNum, cgameGlob->predictedPlayerState.viewlocked_entNum);
    ps = &cgameGlob->predictedPlayerState;
    v6 = (cgameGlob->predictedPlayerState.eFlags & 0x4000) != 0 && ps->vehiclePos == 1;
    local_client_is_gunner = v6;
    render = drawVehicleTurretVersion == v6;
    if ( drawVehicleTurretVersion == v6 )
    {
        if ( (ps->eFlags & 0x300) != 0 )
        {
            weapon = cent->nextState.weapon;
        }
        else
        {
            v4 = (ps->eFlags & 0x4000) != 0 ? CG_GetPlayerWeapon(ps, localClientNum) : GetWeaponIndex(cgameGlob);
            weapon = v4;
        }
        weaponIndex = weapon;
        if ( weapon > 0 && BG_IsOverheatingWeapon(weaponIndex) )
        {
            weaponDefinition = BG_GetWeaponDef(weaponIndex);
            overheating = 0;
            if ( (ps->eFlags & 0x300) != 0 )
            {
                heatValue = cent->nextState.lerp.u.turret.heatVal * 100.0;
                overheating = cent->nextState.lerp.u.turret.overheating != 0;
            }
            else
            {
                heatValue = BG_PlayerWeaponHeat(ps, weaponIndex);
                overheating = BG_PlayerWeaponOverheating(ps, weaponIndex);
            }
            iconColor[0] = 0.89f;
            iconColor[1] = 0.18000001;
            iconColor[2] = 0.01f;
            iconColor[3] = 1.0f;
            flashColor[0] = 1.0f;
            flashColor[1] = 1.0f;
            flashColor[2] = 0.0f;
            CL_DrawStretchPic(
                &scrPlaceView[localClientNum],
                rect->x,
                rect->y,
                rect->w,
                rect->h,
                rect->horzAlign,
                rect->vertAlign,
                0.0,
                0.0,
                1.0,
                1.0,
                color,
                weaponDefinition->hudIcon);
            if ( overheating )
            {
                if ( cgameGlob->lastClipFlashTime > cgameGlob->time || cgameGlob->lastClipFlashTime + 800 < cgameGlob->time )
                    cgameGlob->lastClipFlashTime = cgameGlob->time;
                flashColor[3] = (float)(cgameGlob->lastClipFlashTime + 800 - cgameGlob->time) / 800.0;
                CL_DrawStretchPic(
                    &scrPlaceView[localClientNum],
                    rect->x,
                    rect->y,
                    rect->w * 1.01,
                    rect->h * 1.01,
                    rect->horzAlign,
                    rect->vertAlign,
                    0.0,
                    0.0,
                    1.0,
                    1.0,
                    flashColor,
                    weaponDefinition->hudIcon);
            }
            if ( heatValue > 0.0 )
                CL_DrawStretchPic(
                    &scrPlaceView[localClientNum],
                    rect->x,
                    rect->y,
                    (float)(rect->w * heatValue) / 100.0,
                    rect->h,
                    rect->horzAlign,
                    rect->vertAlign,
                    0.0,
                    0.0,
                    heatValue / 100.0,
                    1.0,
                    iconColor,
                    weaponDefinition->hudIcon);
        }
    }
}

double __cdecl BG_PlayerWeaponHeat(const playerState_s *ps, unsigned int weaponIndex)
{
    const PlayerHeldWeapon *heldWeapon; // [esp+8h] [ebp-4h]

    if ( !ps && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\bgame\\../bgame/bg_weapons.h", 502, 0, "%s", "ps") )
        __debugbreak();
    heldWeapon = BG_GetHeldWeaponConst(ps, weaponIndex);
    if ( heldWeapon )
        return heldWeapon->heatPercent;
    else
        return 0.0;
}

void __cdecl CG_DrawPlayerFuelAmmoValue(int localClientNum, const rectDef_s *rect, float *color)
{
    float iconColor[4]; // [esp+4Ch] [ebp-28h] BYREF
    float fuelValue; // [esp+5Ch] [ebp-18h]
    cg_s *cgameGlob; // [esp+64h] [ebp-10h]
    int weaponIndex; // [esp+68h] [ebp-Ch]
    const playerState_s *ps; // [esp+6Ch] [ebp-8h]
    const WeaponDef *weapDef; // [esp+70h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    ps = &cgameGlob->predictedPlayerState;
    weaponIndex = cgameGlob->predictedPlayerState.weapon;
    weapDef = BG_GetWeaponDef(weaponIndex);
    if ( weapDef->fuelTankWeapon )
    {
        fuelValue = (float)BG_PlayerFuelTankTime(ps, weaponIndex) / (float)weapDef->iTankLifeTime;
        iconColor[0] = 0.89f;
        iconColor[1] = 0.18000001;
        iconColor[2] = 0.01f;
        iconColor[3] = 1.0f;
        if ( !cg_fuelHudVersion->current.integer )
        {
            CL_DrawStretchPic(
                &scrPlaceView[localClientNum],
                rect->x,
                rect->y,
                rect->w,
                rect->h,
                rect->horzAlign,
                rect->vertAlign,
                0.0,
                0.0,
                1.0,
                1.0,
                color,
                weapDef->hudIcon);
            if ( fuelValue > 0.0 )
                CL_DrawStretchPic(
                    &scrPlaceView[localClientNum],
                    rect->x,
                    rect->y,
                    rect->w * fuelValue,
                    rect->h,
                    rect->horzAlign,
                    rect->vertAlign,
                    0.0,
                    0.0,
                    fuelValue,
                    1.0,
                    iconColor,
                    weapDef->hudIcon);
        }
    }
}

void __cdecl CG_DrawPlayerReloadValue(int localClientNum, const rectDef_s *rect, float *color, Material *material)
{
    float v4; // [esp+30h] [ebp-3Ch]
    float v5; // [esp+34h] [ebp-38h]
    float dtMilliseconds; // [esp+48h] [ebp-24h]
    float reloadTimeMilliseconds; // [esp+50h] [ebp-1Ch]
    float reloadPercentage; // [esp+54h] [ebp-18h]
    int weaponIndex; // [esp+58h] [ebp-14h]
    cg_s *cgameGlob; // [esp+5Ch] [ebp-10h]
    const centity_s *cent; // [esp+60h] [ebp-Ch]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    cent = CG_GetEntity(localClientNum, cgameGlob->predictedPlayerState.viewlocked_entNum);
    if ( (cgameGlob->predictedPlayerState.eFlags & 0x4000) != 0
        && !cgameGlob->predictedPlayerState.vehiclePos
        && (cgameGlob->predictedPlayerState.eFlags2 & 0x10000000) == 0 )
    {
        weaponIndex = CG_GetPlayerWeapon(&cgameGlob->predictedPlayerState, localClientNum);
        if ( weaponIndex > 0 )
        {
            if ( cent->vehicle && cent->vehicle->fireTime > 0 )
            {
                reloadTimeMilliseconds = (float)BG_GetWeaponDef(weaponIndex)->iFireTime;
                dtMilliseconds = (float)(cgameGlob->time - cent->vehicle->fireTime);
                if ( reloadTimeMilliseconds <= 0.0 || dtMilliseconds < 0.0 || reloadTimeMilliseconds <= dtMilliseconds )
                {
                    reloadPercentage = 1.0f;
                }
                else
                {
                    if ( (float)((float)(dtMilliseconds / reloadTimeMilliseconds) - 0.0) < 0.0 )
                        v5 = 0.0f;
                    else
                        v5 = dtMilliseconds / reloadTimeMilliseconds;
                    if ( (float)(1.0 - v5) < 0.0 )
                        v4 = 1.0f;
                    else
                        v4 = v5;
                    reloadPercentage = v4;
                }
            }
            else
            {
                reloadPercentage = 1.0f;
            }
            if ( reloadPercentage < 1.0 )
                CL_DrawStretchPic(
                    &scrPlaceView[localClientNum],
                    rect->x,
                    rect->y,
                    rect->w,
                    rect->h,
                    rect->horzAlign,
                    rect->vertAlign,
                    0.0,
                    0.0,
                    1.0,
                    1.0,
                    color,
                    material);
        }
    }
}

void __cdecl CG_DrawPlayerReloadedValue(int localClientNum, const rectDef_s *rect, float *color, Material *material)
{
    float v4; // [esp+30h] [ebp-3Ch]
    float v5; // [esp+34h] [ebp-38h]
    float dtMilliseconds; // [esp+48h] [ebp-24h]
    float reloadTimeMilliseconds; // [esp+50h] [ebp-1Ch]
    float reloadPercentage; // [esp+54h] [ebp-18h]
    int weaponIndex; // [esp+58h] [ebp-14h]
    cg_s *cgameGlob; // [esp+5Ch] [ebp-10h]
    const centity_s *cent; // [esp+60h] [ebp-Ch]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    cent = CG_GetEntity(localClientNum, cgameGlob->predictedPlayerState.viewlocked_entNum);
    if ( (cgameGlob->predictedPlayerState.eFlags & 0x4000) != 0
        && !cgameGlob->predictedPlayerState.vehiclePos
        && (cgameGlob->predictedPlayerState.eFlags2 & 0x10000000) == 0 )
    {
        weaponIndex = CG_GetPlayerWeapon(&cgameGlob->predictedPlayerState, localClientNum);
        if ( weaponIndex > 0 )
        {
            if ( cent->vehicle && cent->vehicle->fireTime > 0 )
            {
                reloadTimeMilliseconds = (float)BG_GetWeaponDef(weaponIndex)->iFireTime;
                dtMilliseconds = (float)(cgameGlob->time - cent->vehicle->fireTime);
                if ( reloadTimeMilliseconds <= 0.0 || dtMilliseconds < 0.0 || reloadTimeMilliseconds <= dtMilliseconds )
                {
                    reloadPercentage = 1.0f;
                }
                else
                {
                    if ( (float)((float)(dtMilliseconds / reloadTimeMilliseconds) - 0.0) < 0.0 )
                        v5 = 0.0f;
                    else
                        v5 = dtMilliseconds / reloadTimeMilliseconds;
                    if ( (float)(1.0 - v5) < 0.0 )
                        v4 = 1.0f;
                    else
                        v4 = v5;
                    reloadPercentage = v4;
                }
            }
            else
            {
                reloadPercentage = 1.0f;
            }
            if ( reloadPercentage > 0.0 )
                CL_DrawStretchPic(
                    &scrPlaceView[localClientNum],
                    rect->x,
                    rect->y,
                    rect->w * reloadPercentage,
                    rect->h,
                    rect->horzAlign,
                    rect->vertAlign,
                    0.0,
                    0.0,
                    reloadPercentage,
                    1.0,
                    color,
                    material);
        }
    }
}

void __cdecl CG_DrawPlayerSprintBack(int localClientNum, const rectDef_s *rect, Material *material, float *color)
{
    cg_s *cgameGlob; // [esp+44h] [ebp-18h]
    float drawColor[4]; // [esp+48h] [ebp-14h] BYREF
    float fadeAlpha; // [esp+58h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    fadeAlpha = CG_FadeHudMenu(
                                localClientNum,
                                hud_fade_sprint,
                                cgameGlob->sprintFadeTime,
                                SnapFloatToInt((float)(hud_fade_sprint->current.value * 1000.0)));
    if ( fadeAlpha != 0.0 )
    {
        drawColor[0] = *color;
        drawColor[1] = color[1];
        drawColor[2] = color[2];
        drawColor[3] = color[3] * fadeAlpha;
        CL_DrawStretchPic(
            &scrPlaceView[localClientNum],
            rect->x,
            rect->y,
            rect->w,
            rect->h,
            rect->horzAlign,
            rect->vertAlign,
            0.0,
            0.0,
            1.0,
            1.0,
            drawColor,
            material);
    }
}

void __cdecl CG_DrawPlayerSprintMeter(int localClientNum, const rectDef_s *rect, Material *material, float *color)
{
    float sprint; // [esp+44h] [ebp-38h]
    cg_s *cgameGlob; // [esp+48h] [ebp-34h]
    float drawColor[4]; // [esp+4Ch] [ebp-30h] BYREF
    int sprintLeft; // [esp+5Ch] [ebp-20h]
    float x; // [esp+60h] [ebp-1Ch]
    float y; // [esp+64h] [ebp-18h]
    playerState_s *ps; // [esp+68h] [ebp-14h]
    int maxSprint; // [esp+6Ch] [ebp-10h]
    float h; // [esp+70h] [ebp-Ch]
    float fadeAlpha; // [esp+74h] [ebp-8h]
    float w; // [esp+78h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    ps = &cgameGlob->predictedPlayerState;
    fadeAlpha = CG_FadeHudMenu(
                                localClientNum,
                                hud_fade_sprint,
                                cgameGlob->sprintFadeTime,
                                SnapFloatToInt((float)(hud_fade_sprint->current.value * 1000.0)));
    if ( fadeAlpha != 0.0 )
    {
        sprintLeft = PM_GetSprintLeft(ps, cgameGlob->time);
        maxSprint = BG_GetMaxSprintTime(ps);
        sprint = (float)sprintLeft / (float)maxSprint;
        if ( sprint > 0.0 )
        {
            x = rect->x;
            y = rect->y;
            w = rect->w * sprint;
            h = rect->h;
            if ( !material )
                material = cgMedia.whiteMaterial;
            CG_CalcPlayerSprintColor(cgameGlob, ps, color);
            drawColor[0] = *color;
            drawColor[1] = color[1];
            drawColor[2] = color[2];
            drawColor[3] = color[3] * fadeAlpha;
            CL_DrawStretchPic(
                &scrPlaceView[localClientNum],
                x,
                y,
                w,
                h,
                rect->horzAlign,
                rect->vertAlign,
                0.0,
                0.0,
                sprint,
                1.0,
                drawColor,
                material);
        }
    }
}

void __cdecl CG_CalcPlayerSprintColor(const cg_s *cgameGlob, const playerState_s *ps, float *color)
{
    const DvarValue *p_current; // [esp+8h] [ebp-14h]
    int sprintLeft; // [esp+14h] [ebp-8h]
    int maxSprint; // [esp+18h] [ebp-4h]

    maxSprint = BG_GetMaxSprintTime(ps);
    if ( ps->pm_type == 9 || !maxSprint )
    {
        p_current = &cg_sprintMeterFullColor->current;
        *color = cg_sprintMeterFullColor->current.value;
        color[1] = p_current->vector[1];
        color[2] = p_current->vector[2];
    }
    else
    {
        if ( PM_IsSprinting(ps) )
            sprintLeft = PM_GetSprintLeft(ps, cgameGlob->time);
        else
            sprintLeft = PM_GetSprintLeftLastTime(ps);
        if ( sprintLeft )
            Vec4Lerp(
                &cg_sprintMeterEmptyColor->current.value,
                &cg_sprintMeterFullColor->current.value,
                (float)sprintLeft / (float)maxSprint,
                color);
        else
            *(DvarValue *)color = cg_sprintMeterDisabledColor->current;
    }
}

void __cdecl CG_DrawPlayerBarHealth(int localClientNum, const rectDef_s *rect, Material *material, float *color)
{
    float v4; // [esp+30h] [ebp-44h]
    float v5; // [esp+34h] [ebp-40h]
    cg_s *cgameGlob; // [esp+4Ch] [ebp-28h]
    float health; // [esp+50h] [ebp-24h]
    const playerState_s *ps; // [esp+5Ch] [ebp-18h]
    float x; // [esp+60h] [ebp-14h]
    float xa; // [esp+60h] [ebp-14h]
    float y; // [esp+64h] [ebp-10h]
    float ya; // [esp+64h] [ebp-10h]
    float h; // [esp+68h] [ebp-Ch]
    float ha; // [esp+68h] [ebp-Ch]
    float w; // [esp+70h] [ebp-4h]
    float wa; // [esp+70h] [ebp-4h]

    if ( cg_drawHealth->current.enabled )
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        health = CG_CalcPlayerHealth(&cgameGlob->nextSnap->ps);
        color[3] = CG_FadeHudMenu(
                                 localClientNum,
                                 hud_fade_healthbar,
                                 cgameGlob->healthFadeTime,
                                 SnapFloatToInt((float)(hud_fade_healthbar->current.value * 1000.0)));
        if ( color[3] != 0.0 )
        {
            ps = &cgameGlob->nextSnap->ps;
            if ( (float)(health - 1.0) < 0.0 )
                v5 = health;
            else
                v5 = 1.0f;
            if ( (float)(0.0 - health) < 0.0 )
                v4 = v5;
            else
                v4 = 0.0f;
            if ( health > 0.0 )
            {
                x = rect->x;
                y = rect->y;
                w = rect->w * health;
                h = rect->h;
                if ( v4 <= 0.5 )
                {
                    color[1] = (float)(v4 + 0.2) * color[1];
                    color[1] = color[1] + 0.30000001;
                }
                else
                {
                    *color = (float)((float)(1.0 - v4) * 2.0) * *color;
                    color[2] = (float)((float)(1.0 - v4) * 2.0) * color[2];
                }
                CL_DrawStretchPic(
                    &scrPlaceView[localClientNum],
                    x,
                    y,
                    w,
                    h,
                    rect->horzAlign,
                    rect->vertAlign,
                    0.0,
                    0.0,
                    health,
                    1.0,
                    color,
                    material);
            }
            if ( cgameGlob->lastHealthClient == ps->clientNum )
            {
                if ( cgameGlob->lastHealth <= health )
                {
                    cgameGlob->lastHealth = health;
                    cgameGlob->lastHealthLerpDelay = 1;
                }
                else if ( cgameGlob->lastHealthLerpDelay )
                {
                    cgameGlob->lastHealthLerpDelay -= cgameGlob->frametime;
                    if ( cgameGlob->lastHealthLerpDelay < 0 )
                        cgameGlob->lastHealthLerpDelay = 0;
                }
                else
                {
                    cgameGlob->lastHealth = cgameGlob->lastHealth - (float)((float)cgameGlob->frametime * 0.0012000001);
                    if ( health >= cgameGlob->lastHealth )
                    {
                        cgameGlob->lastHealth = health;
                        cgameGlob->lastHealthLerpDelay = 1;
                    }
                }
            }
            else
            {
                cgameGlob->lastHealthClient = ps->clientNum;
                cgameGlob->lastHealth = health;
                cgameGlob->lastHealthLerpDelay = 1;
            }
            if ( cgameGlob->lastHealth > health )
            {
                xa = (float)(rect->w * health) + rect->x;
                ya = rect->y;
                wa = (float)(cgameGlob->lastHealth - health) * rect->w;
                ha = rect->h;
                *color = 1.0f;
                color[1] = 0.0f;
                color[2] = 0.0f;
                CL_DrawStretchPic(
                    &scrPlaceView[localClientNum],
                    xa,
                    ya,
                    wa,
                    ha,
                    rect->horzAlign,
                    rect->vertAlign,
                    health,
                    0.0,
                    cgameGlob->lastHealth,
                    1.0,
                    color,
                    material);
            }
        }
    }
}

void __cdecl CG_DrawPlayerBarHealthBack(int localClientNum, const rectDef_s *rect, Material *material, float *color)
{
    int flashTime; // [esp+64h] [ebp-20h]
    cg_s *cgameGlob; // [esp+68h] [ebp-1Ch]
    float health; // [esp+6Ch] [ebp-18h]
    float x; // [esp+70h] [ebp-14h]
    float y; // [esp+74h] [ebp-10h]
    float h; // [esp+78h] [ebp-Ch]
    float fadeAlpha; // [esp+7Ch] [ebp-8h]
    float w; // [esp+80h] [ebp-4h]

    if ( cg_drawHealth->current.enabled )
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        fadeAlpha = CG_FadeHudMenu(
                                    localClientNum,
                                    hud_fade_healthbar,
                                    cgameGlob->healthFadeTime,
                                    SnapFloatToInt((float)(hud_fade_healthbar->current.value * 1000.0)));
        if ( fadeAlpha != 0.0 )
        {
            color[3] = fadeAlpha;
            x = rect->x;
            y = rect->y;
            w = rect->w;
            h = rect->h;
            CL_DrawStretchPic(
                &scrPlaceView[localClientNum],
                rect->x,
                y,
                w,
                h,
                rect->horzAlign,
                rect->vertAlign,
                0.0,
                0.0,
                1.0,
                1.0,
                color,
                material);
            health = CG_CalcPlayerHealth(&cgameGlob->nextSnap->ps);
            if ( health != 0.0 )
            {
                if ( hud_health_startpulse_critical->current.value <= health )
                    flashTime = hud_health_startpulse_injured->current.value <= health
                                        ? 0
                                        : SnapFloatToInt((float)(hud_health_pulserate_injured->current.value * 1000.0));
                else
                    flashTime = SnapFloatToInt((float)(hud_health_pulserate_critical->current.value * 1000.0));
                if ( flashTime )
                {
                    if ( cgameGlob->lastHealthPulseTime > cgameGlob->time
                        || flashTime + cgameGlob->lastHealthPulseTime < cgameGlob->time )
                    {
                        cgameGlob->lastHealthPulseTime = cgameGlob->time;
                    }
                    *color = 0.89f;
                    color[1] = 0.18000001;
                    color[2] = 0.01f;
                    color[3] = (float)(flashTime + cgameGlob->lastHealthPulseTime - cgameGlob->time) / (float)flashTime;
                    if ( color[3] > fadeAlpha )
                        color[3] = fadeAlpha;
                    CL_DrawStretchPic(
                        &scrPlaceView[localClientNum],
                        x,
                        y,
                        w,
                        h,
                        rect->horzAlign,
                        rect->vertAlign,
                        0.0,
                        0.0,
                        1.0,
                        1.0,
                        color,
                        material);
                }
            }
        }
    }
}

void __cdecl CG_DrawPlayerLowHealthOverlay(int localClientNum, const rectDef_s *rect, Material *material, float *color)
{
    cg_s *cgameGlob; // [esp+30h] [ebp-Ch]
    float healthRatio; // [esp+34h] [ebp-8h]
    const playerState_s *ps; // [esp+38h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    healthRatio = CG_CalcPlayerHealth(&cgameGlob->nextSnap->ps);
    ps = &cgameGlob->nextSnap->ps;
    if ( healthRatio == 0.0 )
    {
        CG_ResetLowHealthOverlay(cgameGlob);
    }
    else
    {
        CG_PulseLowHealthOverlay(cgameGlob, healthRatio);
        color[3] = CG_FadeLowHealthOverlay(cgameGlob);
        if ( color[3] != 0.0
            && !CG_KillCamEntityEnabled(localClientNum)
            && !Demo_IsThirdPersonCamera()
            && !Demo_IsMovieCamera()
            && cgameGlob->cameraMode != 1
            && (ps->eFlags2 & 0x10000000) == 0 )
        {
            CL_DrawStretchPic(
                &scrPlaceView[localClientNum],
                rect->x,
                rect->y,
                rect->w,
                rect->h,
                rect->horzAlign,
                rect->vertAlign,
                0.0,
                0.0,
                1.0,
                1.0,
                color,
                material);
        }
    }
}

double __cdecl CG_FadeLowHealthOverlay(const cg_s *cgameGlob)
{
    float curAlpha; // [esp+8h] [ebp-Ch]
    int timeSinceFadeStarted; // [esp+10h] [ebp-4h]

    timeSinceFadeStarted = cgameGlob->time - cgameGlob->healthOverlayPulseTime;
    if ( timeSinceFadeStarted < 0 )
        timeSinceFadeStarted = 0;
    if ( cgameGlob->healthOverlayPulseDuration <= 0 || timeSinceFadeStarted >= cgameGlob->healthOverlayPulseDuration )
    {
        curAlpha = cgameGlob->healthOverlayToAlpha;
    }
    else
    {
        if ( !cgameGlob->healthOverlayPulseDuration
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_newDraw_mp.cpp",
                        1304,
                        0,
                        "%s\n\t(cgameGlob->healthOverlayPulseDuration) = %i",
                        "(cgameGlob->healthOverlayPulseDuration)",
                        cgameGlob->healthOverlayPulseDuration) )
        {
            __debugbreak();
        }
        curAlpha = (float)((float)(cgameGlob->healthOverlayToAlpha - cgameGlob->healthOverlayFromAlpha)
                                         * (float)((float)timeSinceFadeStarted / (float)cgameGlob->healthOverlayPulseDuration))
                         + cgameGlob->healthOverlayFromAlpha;
    }
    if ( (curAlpha < 0.0 || curAlpha > 1.0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_newDraw_mp.cpp",
                    1313,
                    0,
                    "%s\n\t(curAlpha) = %g",
                    "(curAlpha >= 0.0f && curAlpha <= 1.0f)",
                    curAlpha) )
    {
        __debugbreak();
    }
    return curAlpha;
}

void __cdecl CG_PulseLowHealthOverlay(cg_s *cgameGlob, float healthRatio)
{
    const char *v2; // eax
    float v3; // [esp+0h] [ebp-40h]
    float v4; // [esp+4h] [ebp-3Ch]
    float v5; // [esp+8h] [ebp-38h]
    int healthOverlayPulsePhase; // [esp+Ch] [ebp-34h]
    float v7; // [esp+10h] [ebp-30h]
    float v8; // [esp+14h] [ebp-2Ch]
    float v9; // [esp+18h] [ebp-28h]
    float v10; // [esp+1Ch] [ebp-24h]
    float v11; // [esp+20h] [ebp-20h]
    float v12; // [esp+24h] [ebp-1Ch]
    float pulseMags[4]; // [esp+30h] [ebp-10h]

    pulseMags[0] = 1.0f;
    pulseMags[1] = 1.0f;
    pulseMags[2] = 1.0f;
    pulseMags[3] = 0.3f;
    if ( cgameGlob->healthOverlayOldHealth > healthRatio && hud_healthOverlay_pulseStart->current.value > healthRatio )
    {
        cgameGlob->healthOverlayLastHitTime = cgameGlob->time;
        cgameGlob->healthOverlayPulseIndex = 0;
    }
    cgameGlob->healthOverlayOldHealth = healthRatio;
    if ( cgameGlob->healthOverlayPulseDuration + cgameGlob->healthOverlayPulseTime <= cgameGlob->time )
    {
        if ( hud_healthOverlay_pulseStart->current.value > healthRatio || cgameGlob->healthOverlayHurt )
        {
            if ( !cgameGlob->healthOverlayHurt )
                cgameGlob->healthOverlayHurt = 1;
            cgameGlob->healthOverlayPulseTime = cgameGlob->time;
            cgameGlob->healthOverlayFromAlpha = cgameGlob->healthOverlayToAlpha;
            if ( cgameGlob->healthOverlayPulseIndex < 4u )
            {
                healthOverlayPulsePhase = cgameGlob->healthOverlayPulsePhase;
                if ( healthOverlayPulsePhase )
                {
                    if ( healthOverlayPulsePhase == 1 )
                    {
                        v9 = pulseMags[cgameGlob->healthOverlayPulseIndex]
                             * hud_healthOverlay_phaseTwo_toAlphaMultiplier->current.value;
                        if ( (float)(v9 - 1.0) < 0.0 )
                            v10 = pulseMags[cgameGlob->healthOverlayPulseIndex]
                                    * hud_healthOverlay_phaseTwo_toAlphaMultiplier->current.value;
                        else
                            v10 = 1.0f;
                        if ( (float)(0.0 - v9) < 0.0 )
                            v4 = v10;
                        else
                            v4 = 0.0f;
                        cgameGlob->healthOverlayToAlpha = v4;
                        cgameGlob->healthOverlayPulseDuration = hud_healthOverlay_phaseTwo_pulseDuration->current.integer;
                        ++cgameGlob->healthOverlayPulsePhase;
                    }
                    else if ( healthOverlayPulsePhase == 2 )
                    {
                        v7 = pulseMags[cgameGlob->healthOverlayPulseIndex]
                             * hud_healthOverlay_phaseThree_toAlphaMultiplier->current.value;
                        if ( (float)(v7 - 1.0) < 0.0 )
                            v8 = pulseMags[cgameGlob->healthOverlayPulseIndex]
                                 * hud_healthOverlay_phaseThree_toAlphaMultiplier->current.value;
                        else
                            v8 = 1.0f;
                        if ( (float)(0.0 - v7) < 0.0 )
                            v3 = v8;
                        else
                            v3 = 0.0f;
                        cgameGlob->healthOverlayToAlpha = v3;
                        cgameGlob->healthOverlayPulseDuration = hud_healthOverlay_phaseThree_pulseDuration->current.integer;
                        cgameGlob->healthOverlayPulsePhase = 0;
                        if ( cgameGlob->time >= hud_healthOverlay_regenPauseTime->current.integer
                                                                    + cgameGlob->healthOverlayLastHitTime
                                                                    - 3
                                                                    * (hud_healthOverlay_phaseThree_pulseDuration->current.integer
                                                                     + hud_healthOverlay_phaseTwo_pulseDuration->current.integer
                                                                     + hud_healthOverlay_phaseOne_pulseDuration->current.integer) )
                            ++cgameGlob->healthOverlayPulseIndex;
                    }
                    else
                    {
                        v2 = va("Invalid health overlay pulse phase: %i", cgameGlob->healthOverlayPulsePhase);
                        if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_newDraw_mp.cpp", 1371, 0, v2) )
                            __debugbreak();
                    }
                }
                else
                {
                    v11 = pulseMags[cgameGlob->healthOverlayPulseIndex];
                    if ( (float)(v11 - 1.0) < 0.0 )
                        v12 = pulseMags[cgameGlob->healthOverlayPulseIndex];
                    else
                        v12 = 1.0f;
                    if ( (float)(0.0 - v11) < 0.0 )
                        v5 = v12;
                    else
                        v5 = 0.0f;
                    cgameGlob->healthOverlayToAlpha = v5;
                    cgameGlob->healthOverlayPulseDuration = hud_healthOverlay_phaseOne_pulseDuration->current.integer;
                    ++cgameGlob->healthOverlayPulsePhase;
                }
            }
        }
        if ( healthRatio > hud_healthOverlay_pulseStop->current.value && cgameGlob->healthOverlayHurt )
        {
            cgameGlob->healthOverlayFromAlpha = CG_FadeLowHealthOverlay(cgameGlob);
            cgameGlob->healthOverlayHurt = 0;
            cgameGlob->healthOverlayPulseTime = cgameGlob->time;
            cgameGlob->healthOverlayToAlpha = hud_healthOverlay_phaseEnd_toAlpha->current.value;
            cgameGlob->healthOverlayPulseDuration = hud_healthOverlay_phaseEnd_pulseDuration->current.integer;
            cgameGlob->healthOverlayPulsePhase = 0;
        }
    }
}

void __cdecl CG_DrawPlayerDirectionalHitIndicator(
                int localClientNum,
                const rectDef_s *rect,
                Material *material,
                float *color)
{
    int i; // [esp+60h] [ebp-40h]
    float playerYaw; // [esp+64h] [ebp-3Ch]
    cg_s *cgameGlob; // [esp+68h] [ebp-38h]
    float playerOrigin; // [esp+6Ch] [ebp-34h]
    float playerOrigin_4; // [esp+70h] [ebp-30h]
    float yaw; // [esp+78h] [ebp-28h]
    float victimDirection[3]; // [esp+80h] [ebp-20h] BYREF
    float victimYaw; // [esp+8Ch] [ebp-14h]
    float playerDir[3]; // [esp+90h] [ebp-10h] BYREF
    float current; // [esp+9Ch] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->cameraMode != 1 )
    {
        playerOrigin = cgameGlob->predictedPlayerState.origin[0];
        playerOrigin_4 = cgameGlob->predictedPlayerState.origin[1];
        for ( i = 0; i < 4; ++i )
        {
            if ( cgameGlob->directionalHitIndicator[i].time )
            {
                if ( cg_hudDamageDirectionalIconTime->current.integer + cgameGlob->directionalHitIndicator[i].time > cgameGlob->time )
                {
                    victimDirection[0] = cgameGlob->directionalHitIndicator[i].entOrigin[0] - playerOrigin;
                    victimDirection[1] = cgameGlob->directionalHitIndicator[i].entOrigin[1] - playerOrigin_4;
                    victimDirection[2] = cgameGlob->directionalHitIndicator[i].entOrigin[2] - 0.0;
                    Vec3Normalize(victimDirection);
                    victimYaw = vectoyaw(victimDirection);
                    playerDir[0] = cgameGlob->refdef.viewaxis[0][0];
                    playerDir[1] = cgameGlob->refdef.viewaxis[0][1];
                    playerDir[2] = 0.0f;
                    Vec3Normalize(playerDir);
                    playerYaw = vectoyaw(playerDir);
                    yaw = AngleNormalize180(playerYaw - victimYaw);
                    current = (float)(cgameGlob->time - cgameGlob->directionalHitIndicator[i].time);
                    color[3] = 1.0 - (float)(current / (float)cg_hudDamageDirectionalIconTime->current.integer);
                    CL_DrawStretchPicRotatedST(
                        &scrPlaceView[localClientNum],
                        rect->x,
                        rect->y,
                        rect->w,
                        rect->h,
                        rect->horzAlign,
                        rect->vertAlign,
                        0.5,
                        0.5,
                        0.5,
                        1.0,
                        1.0,
                        (-(yaw)),
                        color,
                        material);
                }
            }
        }
    }
}

void __cdecl draw_tank_body_component(
                int localClientNum,
                const rectDef_s *rect,
                Material *material,
                const float *color,
                e_tank_hud_component component)
{
    float health_fraction; // [esp+18h] [ebp-50h]
    float health_fractiona; // [esp+18h] [ebp-50h]
    float health_fractionb; // [esp+18h] [ebp-50h]
    float angle; // [esp+44h] [ebp-24h]
    float tint_color[4]; // [esp+48h] [ebp-20h] BYREF
    const vehicle_info_t *info; // [esp+58h] [ebp-10h]
    __int16 vehType; // [esp+5Ch] [ebp-Ch]
    const centity_s *cent; // [esp+60h] [ebp-8h]
    const cg_s *cgameGlob; // [esp+64h] [ebp-4h]

    if ( (unsigned int)component > _tank_component_right_tread
        && component != _tank_component_driver_seat
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_newDraw_mp.cpp",
                    2597,
                    0,
                    "%s",
                    "component==_tank_component_core || component==_tank_component_left_tread || component==_tank_component_right_t"
                    "read || component==_tank_component_driver_seat") )
    {
        __debugbreak();
    }
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( (cgameGlob->predictedPlayerState.eFlags & 0x4000) != 0 )
    {
        cent = CG_GetEntity(localClientNum, cgameGlob->predictedPlayerState.viewlocked_entNum);
        if ( cent->nextState.eType == 14 )
        {
            vehType = cent->nextState.vehicleState.vehicleInfoIndex;
            info = CG_GetVehicleInfo(vehType);
            if ( info )
            {
                if ( info->type == 2 )
                {
                    angle = AngleNormalize180(cgameGlob->refdefViewAngles[1] - cent->pose.angles[1]);
                    switch ( component )
                    {
                        case _tank_component_core:
                            health_fraction = CG_VehGetHealthPercentageEntity(cent);
                            get_tank_hud_health_tint_color(health_fraction, tint_color, 0.0);
                            break;
                        case _tank_component_left_tread:
                            health_fractiona = CG_VehGetHealthPercentageLeftTread(cent);
                            get_tank_hud_health_tint_color(health_fractiona, tint_color, 1.0);
                            break;
                        case _tank_component_right_tread:
                            health_fractionb = CG_VehGetHealthPercentageRightTread(cent);
                            get_tank_hud_health_tint_color(health_fractionb, tint_color, 1.0);
                            break;
                        case _tank_component_driver_seat:
                            get_tank_seat_tint_color(cent, 0, tint_color);
                            break;
                        default:
                            break;
                    }
                    tint_color[0] = tint_color[0] * *color;
                    tint_color[1] = tint_color[1] * color[1];
                    tint_color[2] = tint_color[2] * color[2];
                    tint_color[3] = tint_color[3] * color[3];
                    CG_DrawRotatedPic(
                        &scrPlaceView[localClientNum],
                        rect->x,
                        rect->y,
                        rect->w,
                        rect->h,
                        rect->horzAlign,
                        rect->vertAlign,
                        angle,
                        tint_color,
                        material);
                }
            }
        }
    }
}

void __cdecl get_tank_seat_tint_color(const centity_s *cent, unsigned int seat_index, float *rgba)
{
    float v3; // [esp+0h] [ebp-14h]
    float v4; // [esp+4h] [ebp-10h]
    bool v5; // [esp+8h] [ebp-Ch]
    int seat_occupation_flags; // [esp+Ch] [ebp-8h]

    seat_occupation_flags = CG_VehGetSeatOccupancyFlags(cent);
    v5 = seat_index < 2 && (seat_occupation_flags & (1 << seat_index)) != 0;
    if ( v5 )
        v4 = 0.0f;
    else
        v4 = 1.0f;
    *rgba = v4;
    rgba[1] = 1.0f;
    if ( v5 )
        v3 = 0.0f;
    else
        v3 = 1.0f;
    rgba[2] = v3;
    rgba[3] = 1.0f;
}

void __cdecl get_tank_hud_health_tint_color(float health_fraction, float *rgba, float black_when_dead)
{
    float v3; // [esp+0h] [ebp-4h]

    if ( black_when_dead == 0.0 || health_fraction > 0.0 )
    {
        *rgba = 1.0f;
        rgba[1] = health_fraction;
        if ( health_fraction < 0.94999999 )
            v3 = 0.0f;
        else
            v3 = 1.0f;
        rgba[2] = v3;
        rgba[3] = 1.0f;
    }
    else
    {
        *rgba = 0.25f;
        rgba[1] = 0.25f;
        rgba[2] = 0.25f;
        rgba[3] = 1.0f;
    }
}

void __cdecl draw_tank_turret_component(
                int localClientNum,
                const rectDef_s *rect,
                Material *material,
                const float *color,
                e_tank_hud_component component)
{
    float health_fraction; // [esp+Ch] [ebp-ECh]
    float xy[4][2]; // [esp+64h] [ebp-94h] BYREF
    float angle; // [esp+84h] [ebp-74h]
    float angles[3]; // [esp+88h] [ebp-70h] BYREF
    float x; // [esp+94h] [ebp-64h] BYREF
    float y; // [esp+98h] [ebp-60h] BYREF
    float h; // [esp+9Ch] [ebp-5Ch] BYREF
    float tint_color[4]; // [esp+A0h] [ebp-58h] BYREF
    float w; // [esp+B0h] [ebp-48h] BYREF
    DObj *obj; // [esp+B4h] [ebp-44h]
    float tagMtx[4][3]; // [esp+B8h] [ebp-40h] BYREF
    const vehicle_info_t *info; // [esp+E8h] [ebp-10h]
    __int16 vehType; // [esp+ECh] [ebp-Ch]
    const centity_s *cent; // [esp+F0h] [ebp-8h]
    const cg_s *cgameGlob; // [esp+F4h] [ebp-4h]

    if ( component != _tank_component_turret
        && component != _tank_component_gunner_seat
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_newDraw_mp.cpp",
                    2639,
                    0,
                    "%s",
                    "component==_tank_component_turret || component==_tank_component_gunner_seat") )
    {
        __debugbreak();
    }
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( (cgameGlob->predictedPlayerState.eFlags & 0x4000) != 0 )
    {
        cent = CG_GetEntity(localClientNum, cgameGlob->predictedPlayerState.viewlocked_entNum);
        if ( cent->nextState.eType == 14 )
        {
            vehType = cent->nextState.vehicleState.vehicleInfoIndex;
            info = CG_GetVehicleInfo(vehType);
            if ( info )
            {
                if ( info->type == 2 )
                {
                    obj = Com_GetClientDObj(cent->nextState.number, localClientNum);
                    if ( obj )
                    {
                        if ( CG_DObjGetWorldTagMatrix(&cent->pose, obj, scr_const.tag_turret, tagMtx, tagMtx[3]) )
                        {
                            AxisToAngles(tagMtx, angles);
                            angle = AngleNormalize180(cgameGlob->refdefViewAngles[1] - angles[1]);
                            x = rect->x;
                            y = rect->y;
                            w = rect->w;
                            h = rect->h;
                            ScrPlace_ApplyRect(&scrPlaceView[localClientNum], &x, &y, &w, &h, rect->horzAlign, rect->vertAlign);
                            xy[0][0] = w * 0.5;
                            (xy[0][1]) = -(h * 0.75);
                            //*(_QWORD *)&xy[1][0] = __PAIR64__(LODWORD(xy[0][1]), w * 0.5) ^ (unsigned int)_mask__NegFloat_;
                            xy[1][0] = -(w * 0.5);
                            xy[1][1] = -xy[0][1];
                            xy[2][0] = xy[1][0];
                            xy[2][1] = h * 0.25;
                            xy[3][0] = w * 0.5;
                            xy[3][1] = h * 0.25;
                            if ( component == _tank_component_turret )
                            {
                                health_fraction = CG_VehGetHealthPercentageEntity(cent);
                                get_tank_hud_health_tint_color(health_fraction, tint_color, 0.0);
                            }
                            else if ( component == _tank_component_gunner_seat )
                            {
                                get_tank_seat_tint_color(cent, 1u, tint_color);
                            }
                            tint_color[0] = tint_color[0] * *color;
                            tint_color[1] = tint_color[1] * color[1];
                            tint_color[2] = tint_color[2] * color[2];
                            tint_color[3] = tint_color[3] * color[3];
                            CG_DrawRotatedQuadPic(&scrPlaceView[localClientNum], x, y, xy, angle, tint_color, material);
                        }
                    }
                }
            }
        }
    }
}

void __cdecl CG_DrawWarMomentumProgress(
                int localClientNum,
                const rectDef_s *rect,
                float *color,
                Material *material,
                int ownerDraw)
{
    float blitzKriegColor[4]; // [esp+34h] [ebp-2Ch] BYREF
    float bonusColor[4]; // [esp+44h] [ebp-1Ch] BYREF
    cg_s *cgameGlob; // [esp+54h] [ebp-Ch]
    float sourceWidth; // [esp+58h] [ebp-8h]
    float w; // [esp+5Ch] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    switch ( ownerDraw )
    {
        case 171:
            if ( cgameGlob->warMomentumMultiplier < 2 )
                CL_DrawStretchPic(
                    &scrPlaceView[localClientNum],
                    rect->x,
                    rect->y,
                    rect->w,
                    rect->h,
                    rect->horzAlign,
                    rect->vertAlign,
                    0.0,
                    0.0,
                    1.0,
                    1.0,
                    color,
                    material);
            break;
        case 172:
            if ( cgameGlob->warMomentumMultiplier >= 2 || cgameGlob->warCurrentMomentumProgress <= 0.0 )
            {
                if ( cgameGlob->warMomentumMultiplier < 2 )
                {
                    w = 0.0f;
                    sourceWidth = 0.0f;
                }
                else
                {
                    w = rect->w;
                    sourceWidth = 1.0f;
                }
            }
            else
            {
                w = rect->w * cgameGlob->warCurrentMomentumProgress;
                sourceWidth = cgameGlob->warCurrentMomentumProgress;
            }
            CL_DrawStretchPic(
                &scrPlaceView[localClientNum],
                rect->x,
                rect->y,
                w,
                rect->h,
                rect->horzAlign,
                rect->vertAlign,
                0.0,
                0.0,
                sourceWidth,
                1.0,
                color,
                material);
            break;
        case 173:
            if ( cgameGlob->warMomentumMultiplier >= 2 )
                CL_DrawStretchPic(
                    &scrPlaceView[localClientNum],
                    rect->x,
                    rect->y,
                    rect->w,
                    rect->h,
                    rect->horzAlign,
                    rect->vertAlign,
                    0.0,
                    0.0,
                    1.0,
                    1.0,
                    color,
                    material);
            break;
        case 174:
            if ( cgameGlob->warMomentumMultiplier >= 2 && cgameGlob->warCurrentMomentumProgress > 0.0 )
                CL_DrawStretchPic(
                    &scrPlaceView[localClientNum],
                    rect->x,
                    rect->y,
                    rect->w * cgameGlob->warCurrentMomentumProgress,
                    rect->h,
                    rect->horzAlign,
                    rect->vertAlign,
                    0.0,
                    0.0,
                    cgameGlob->warCurrentMomentumProgress,
                    1.0,
                    color,
                    material);
            break;
        case 284:
            bonusColor[0] = 0.7f;
            bonusColor[1] = 0.5f;
            bonusColor[2] = 0.2f;
            bonusColor[3] = 1.0f;
            blitzKriegColor[0] = 0.7f;
            blitzKriegColor[1] = 0.3f;
            blitzKriegColor[2] = 0.0f;
            blitzKriegColor[3] = 1.0f;
            if ( cgameGlob->warMomentumMultiplier >= 2 )
            {
                if ( cgameGlob->warMomentumMultiplier >= 3 )
                {
                    CL_DrawStretchPic(
                        &scrPlaceView[localClientNum],
                        rect->x,
                        rect->y,
                        rect->w,
                        rect->h,
                        rect->horzAlign,
                        rect->vertAlign,
                        0.0,
                        0.0,
                        1.0,
                        1.0,
                        blitzKriegColor,
                        material);
                }
                else
                {
                    CL_DrawStretchPic(
                        &scrPlaceView[localClientNum],
                        rect->x,
                        rect->y,
                        rect->w,
                        rect->h,
                        rect->horzAlign,
                        rect->vertAlign,
                        0.0,
                        0.0,
                        1.0,
                        1.0,
                        bonusColor,
                        material);
                    if ( cgameGlob->warCurrentMomentumProgress > 0.0 )
                    {
                        w = rect->w * cgameGlob->warCurrentMomentumProgress;
                        sourceWidth = cgameGlob->warCurrentMomentumProgress;
                        CL_DrawStretchPic(
                            &scrPlaceView[localClientNum],
                            rect->x,
                            rect->y,
                            w,
                            rect->h,
                            rect->horzAlign,
                            rect->vertAlign,
                            0.0,
                            0.0,
                            sourceWidth,
                            1.0,
                            blitzKriegColor,
                            material);
                    }
                }
            }
            else
            {
                CL_DrawStretchPic(
                    &scrPlaceView[localClientNum],
                    rect->x,
                    rect->y,
                    rect->w,
                    rect->h,
                    rect->horzAlign,
                    rect->vertAlign,
                    0.0,
                    0.0,
                    1.0,
                    1.0,
                    color,
                    material);
                if ( cgameGlob->warCurrentMomentumProgress > 0.0 )
                {
                    w = rect->w * cgameGlob->warCurrentMomentumProgress;
                    sourceWidth = cgameGlob->warCurrentMomentumProgress;
                    CL_DrawStretchPic(
                        &scrPlaceView[localClientNum],
                        rect->x,
                        rect->y,
                        w,
                        rect->h,
                        rect->horzAlign,
                        rect->vertAlign,
                        0.0,
                        0.0,
                        sourceWidth,
                        1.0,
                        bonusColor,
                        material);
                }
            }
            break;
        default:
            return;
    }
}

void __cdecl CG_DrawWarMomentumMultiplier(int localClientNum, const rectDef_s *rect, float *color, Material *material)
{
    cg_s *cgameGlob; // [esp+30h] [ebp-18h]
    float iconColor[4]; // [esp+34h] [ebp-14h] BYREF

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->warMomentumMultiplier >= 2 )
        iconColor[0] = 1.0f;
    else
        iconColor[0] = 0.01f;
    iconColor[1] = 0.9f;
    iconColor[2] = 0.01f;
    iconColor[3] = 1.0f;
    if ( cgameGlob->warMomentumMultiplier == 2 )
    {
        *color = 0.01f;
        color[1] = 0.9f;
        color[2] = 0.01f;
        color[3] = 1.0f;
    }
    else if ( cgameGlob->warMomentumMultiplier == 3 )
    {
        *color = 1.0f;
        color[1] = 0.9f;
        color[2] = 0.01f;
        color[3] = 1.0f;
    }
    CL_DrawStretchPic(
        &scrPlaceView[localClientNum],
        rect->x,
        rect->y,
        rect->w,
        rect->h,
        rect->horzAlign,
        rect->vertAlign,
        0.0,
        0.0,
        1.0,
        1.0,
        color,
        material);
    if ( cgameGlob->warCurrentMomentumProgress > 0.0 && cgameGlob->warMomentumMultiplier < 3 )
        CL_DrawStretchPic(
            &scrPlaceView[localClientNum],
            rect->x,
            rect->y,
            rect->w * cgameGlob->warCurrentMomentumProgress,
            rect->h,
            rect->horzAlign,
            rect->vertAlign,
            0.0,
            0.0,
            cgameGlob->warCurrentMomentumProgress,
            1.0,
            iconColor,
            material);
}

void __cdecl CG_DrawWarMomentumMultiplierDetail(
                int localClientNum,
                const rectDef_s *rect,
                float *color,
                Material *material)
{
    cg_s *cgameGlob; // [esp+30h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->warMomentumMultiplier == 2 )
    {
        *color = 1.0f;
        color[1] = 0.9f;
        color[2] = 0.4f;
        color[3] = 1.0f;
LABEL_6:
        CL_DrawStretchPic(
            &scrPlaceView[localClientNum],
            rect->x,
            rect->y,
            rect->w,
            rect->h,
            rect->horzAlign,
            rect->vertAlign,
            0.0,
            0.0,
            1.0,
            1.0,
            color,
            material);
        return;
    }
    if ( cgameGlob->warMomentumMultiplier == 3 )
    {
        *color = 1.0f;
        color[1] = 0.9f;
        color[2] = 0.4f;
        color[3] = 1.0f;
        goto LABEL_6;
    }
}

void __cdecl CG_DrawWarMomentumMultiplierBlitzkrieg(
                int localClientNum,
                const rectDef_s *rect,
                float *color,
                Material *material)
{
    float v4; // [esp+30h] [ebp-24h]
    cg_s *cgameGlob; // [esp+38h] [ebp-1Ch]
    float flashColor[4]; // [esp+40h] [ebp-14h] BYREF
    float k_mimimum_transparency; // [esp+50h] [ebp-4h]

    k_mimimum_transparency = 0.25f;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->warMomentumMultiplier == 3 )
    {
        flashColor[0] = *color;
        flashColor[1] = color[1];
        flashColor[2] = color[2];
        flashColor[3] = color[3];
        if ( cgameGlob->warMomentumBlitzkriegTime >= 10 )
        {
            if ( cgameGlob->warMomentumFlashTime > cgameGlob->time || cgameGlob->warMomentumFlashTime + 2000 < cgameGlob->time )
                cgameGlob->warMomentumFlashTime = cgameGlob->time;
            flashColor[3] = (float)(cgameGlob->warMomentumFlashTime + 2000 - cgameGlob->time) / 2000.0;
        }
        else
        {
            if ( cgameGlob->warMomentumFlashTime > cgameGlob->time || cgameGlob->warMomentumFlashTime + 650 < cgameGlob->time )
                cgameGlob->warMomentumFlashTime = cgameGlob->time;
            flashColor[3] = (float)(cgameGlob->warMomentumFlashTime + 650 - cgameGlob->time) / 650.0;
        }
        if ( (float)(flashColor[3] - k_mimimum_transparency) < 0.0 )
            v4 = k_mimimum_transparency;
        else
            v4 = flashColor[3];
        flashColor[3] = v4;
        CL_DrawStretchPic(
            &scrPlaceView[localClientNum],
            rect->x,
            rect->y,
            rect->w,
            rect->h,
            rect->horzAlign,
            rect->vertAlign,
            0.0,
            0.0,
            1.0,
            1.0,
            flashColor,
            material);
    }
}

void __cdecl CG_DrawLifeCounter(int localClientNum, rectDef_s *rect, float *color, bool alive)
{
    if (alive)
        CL_DrawStretchPic(
            &scrPlaceView[localClientNum],
            rect->x,
            rect->y,
            rect->w,
            rect->h,
            rect->horzAlign,
            rect->vertAlign,
            0.0,
            0.0,
            1.0,
            1.0,
            color,
            cgMedia.lifeCounterAlive);
    else
        CL_DrawStretchPic(
            &scrPlaceView[localClientNum],
            rect->x,
            rect->y,
            rect->w,
            rect->h,
            rect->horzAlign,
            rect->vertAlign,
            0.0,
            0.0,
            1.0,
            1.0,
            color,
            cgMedia.lifeCounterDead);
}

void __cdecl CG_DrawWristWatch(
                int localClientNum,
                const rectDef_s *parentRect,
                const rectDef_s *rect,
                Material *material,
                float *color)
{
    float v5; // [esp+24h] [ebp-48h]
    float v6; // [esp+28h] [ebp-44h]
    float v7; // [esp+2Ch] [ebp-40h]
    ScreenPlacement *scrPlace; // [esp+3Ch] [ebp-30h]
    qtime_s systime; // [esp+48h] [ebp-24h] BYREF

    if ( CG_GetLocalClientGlobals(localClientNum)->nextSnap )
    {
        scrPlace = &scrPlaceView[localClientNum];
        CG_DrawRotatedPic(
            scrPlace,
            0.0,
            0.0,
            parentRect->w,
            parentRect->h,
            rect->horzAlign,
            rect->vertAlign,
            0.0,
            color,
            cgMedia.watch_face);
        Com_RealTime(&systime, 1);
        v7 = floor((float)((float)((float)systime.tm_hour + (float)((float)systime.tm_min / 60.0)) * 30.0));
        CG_DrawRotatedPic(
            scrPlace,
            0.0,
            0.0,
            parentRect->w,
            parentRect->h,
            rect->horzAlign,
            rect->vertAlign,
            v7,
            color,
            cgMedia.watch_hour);
        v6 = floor((float)((float)systime.tm_min * 6.0));
        CG_DrawRotatedPic(
            scrPlace,
            0.0,
            0.0,
            parentRect->w,
            parentRect->h,
            rect->horzAlign,
            rect->vertAlign,
            v6,
            color,
            cgMedia.watch_minute);
        v5 = floor((float)((float)systime.tm_sec * 6.0));
        CG_DrawRotatedPic(
            scrPlace,
            0.0,
            0.0,
            parentRect->w,
            parentRect->h,
            rect->horzAlign,
            rect->vertAlign,
            v5,
            color,
            cgMedia.watch_second);
    }
}

void __cdecl CG_DrawDemoControls(int localClientNum, const rectDef_s *rect, Material *material, float *color)
{
    float v4; // xmm0_4
    float v5; // xmm0_4
    float v6; // xmm0_4
    float v7; // xmm0_4
    float v8; // xmm0_4
    float w; // xmm0_4
    float v10; // xmm0_4
    double TimeScale; // st7
    double v12; // st7
    float v13; // xmm0_4
    float v14; // xmm0_4
    float v15; // xmm0_4
    float v16; // xmm0_4
    float v17; // xmm0_4
    float s1; // [esp+18h] [ebp-268h]
    float v19; // [esp+34h] [ebp-24Ch]
    float v20; // [esp+38h] [ebp-248h]
    int ClipTime; // [esp+3Ch] [ebp-244h]
    float v22; // [esp+9Ch] [ebp-1E4h]
    float v23; // [esp+A0h] [ebp-1E0h]
    int i; // [esp+B8h] [ebp-1C8h]
    float timelineColor[4]; // [esp+C4h] [ebp-1BCh] BYREF
    int timeSec; // [esp+D4h] [ebp-1ACh]
    Font_s *font; // [esp+D8h] [ebp-1A8h]
    int textStyle; // [esp+DCh] [ebp-1A4h]
    int duration; // [esp+E0h] [ebp-1A0h]
    float innerGreyBoxY; // [esp+E4h] [ebp-19Ch]
    float timelineWidth; // [esp+E8h] [ebp-198h]
    const char *displayText; // [esp+ECh] [ebp-194h]
    bool result; // [esp+F3h] [ebp-18Dh]
    const ScreenPlacement *scrPlace; // [esp+F4h] [ebp-18Ch]
    float innerGreyBoxYSpacing; // [esp+F8h] [ebp-188h]
    float timelineCursorWidth; // [esp+FCh] [ebp-184h]
    cg_s *cgameGlob; // [esp+100h] [ebp-180h]
    float percentCompleted; // [esp+104h] [ebp-17Ch]
    float controlNamesBoxIconSetWidth; // [esp+108h] [ebp-178h]
    int endTime; // [esp+10Ch] [ebp-174h] BYREF
    float activityBlockColor[4]; // [esp+110h] [ebp-170h] BYREF
    float keyframeColor[4]; // [esp+120h] [ebp-160h] BYREF
    float textColor[4]; // [esp+130h] [ebp-150h] BYREF
    float disabledText[4]; // [esp+140h] [ebp-140h]
    int startTime; // [esp+150h] [ebp-130h] BYREF
    float activityBlockX; // [esp+154h] [ebp-12Ch]
    float innerGreyBoxXSpacing; // [esp+158h] [ebp-128h]
    float timelineX; // [esp+15Ch] [ebp-124h]
    float timelineColorFaded[4]; // [esp+160h] [ebp-120h] BYREF
    float controlNamesBoxWidth; // [esp+170h] [ebp-110h]
    int disconnectTime; // [esp+174h] [ebp-10Ch] BYREF
    int bookmarkIndex; // [esp+178h] [ebp-108h]
    int bookmarkTime; // [esp+17Ch] [ebp-104h] BYREF
    float bookmarkColor[4]; // [esp+180h] [ebp-100h] BYREF
    float innerGreyBoxHeight; // [esp+190h] [ebp-F0h]
    float timelineCursorHeight; // [esp+194h] [ebp-ECh]
    float controlNamesBoxY; // [esp+198h] [ebp-E8h]
    float displayTextWidth; // [esp+19Ch] [ebp-E4h]
    bool enableHostControls; // [esp+1A3h] [ebp-DDh]
    float spacing; // [esp+1A4h] [ebp-DCh]
    char timeText[8]; // [esp+1A8h] [ebp-D8h] BYREF
    int bookmarkType; // [esp+1B4h] [ebp-CCh] BYREF
    float controlNamesBoxColor[4]; // [esp+1B8h] [ebp-C8h] BYREF
    float controlNamesBoxColorFaded[4]; // [esp+1C8h] [ebp-B8h] BYREF
    int currentTime; // [esp+1D8h] [ebp-A8h]
    int connectTime; // [esp+1DCh] [ebp-A4h] BYREF
    float innerGreyBoxWidth; // [esp+1E0h] [ebp-A0h]
    float timelineY; // [esp+1E4h] [ebp-9Ch]
    float controlNamesBoxIconHeight; // [esp+1E8h] [ebp-98h]
    float controlNamesBoxIconSpacing; // [esp+1ECh] [ebp-94h]
    float activityBlockW; // [esp+1F0h] [ebp-90h]
    float timelineTextWidth; // [esp+1F4h] [ebp-8Ch]
    float controlNamesBoxX; // [esp+1F8h] [ebp-88h]
    float controlNamesBoxIconY; // [esp+1FCh] [ebp-84h]
    float controlNamesBoxIconWidth; // [esp+200h] [ebp-80h]
    float iconColor[4]; // [esp+204h] [ebp-7Ch] BYREF
    float keyframeX; // [esp+214h] [ebp-6Ch]
    int keyframeTime; // [esp+218h] [ebp-68h]
    Material *controlsNamesBoxBackground; // [esp+21Ch] [ebp-64h]
    float controlNamesBoxHeight; // [esp+220h] [ebp-60h]
    float colorCalculation; // [esp+224h] [ebp-5Ch]
    float bookmarkX; // [esp+228h] [ebp-58h]
    float outerBoxColor[4]; // [esp+22Ch] [ebp-54h] BYREF
    float innerGreyBoxColor[4]; // [esp+23Ch] [ebp-44h] BYREF
    bool enableClipAndScreenshot; // [esp+24Fh] [ebp-31h]
    float bookmarkEventColor[4]; // [esp+250h] [ebp-30h]
    float innerGreyBoxX; // [esp+260h] [ebp-20h]
    float playerActivityColor[4]; // [esp+264h] [ebp-1Ch]
    float outlineBoxSpacing; // [esp+274h] [ebp-Ch]
    float timelineHeight; // [esp+278h] [ebp-8h]
    float controlNamesBoxIconX; // [esp+27Ch] [ebp-4h]

    disabledText[0] = 0.35f;
    disabledText[1] = 0.35f;
    disabledText[2] = 0.35f;
    disabledText[3] = 1.0f;
    bookmarkIndex = -1;
    activityBlockColor[0] = 0.6f;
    activityBlockColor[1] = 0.0f;
    activityBlockColor[2] = 0.0f;
    activityBlockColor[3] = 1.0f;
    keyframeColor[0] = 1.0f;
    keyframeColor[1] = 1.0f;
    keyframeColor[2] = 1.0f;
    keyframeColor[3] = 0.5f;
    outerBoxColor[0] = 0.05f;
    outerBoxColor[1] = 0.03f;
    outerBoxColor[2] = 0.03f;
    outerBoxColor[3] = 1.0f;
    outlineBoxSpacing = 10.0f;
    innerGreyBoxX = rect->x + 10.0;
    innerGreyBoxY = rect->y + 10.0;
    innerGreyBoxWidth = rect->w - (float)(2.0 * 10.0);
    innerGreyBoxHeight = rect->h - (float)(3.0 * 10.0);
    innerGreyBoxXSpacing = 5.0f;
    innerGreyBoxYSpacing = 6.0f;
    innerGreyBoxColor[0] = 0.14;
    innerGreyBoxColor[1] = 0.14;
    innerGreyBoxColor[2] = 0.14;
    innerGreyBoxColor[3] = 1.0f;
    timelineX = innerGreyBoxX + 5.0;
    timelineY = innerGreyBoxY + 6.0;
    v4 = innerGreyBoxWidth - 10.0 * 3.5;
    timelineWidth = v4;
    v5 = 10.0 * 0.8;
    timelineHeight = v5;
    timelineCursorWidth = 10.0f;
    timelineCursorHeight = 5.0f;
    memset(timelineColor, 0, 12);
    timelineColor[3] = 1.0f;
    timelineColorFaded[0] = 1.0f;
    timelineColorFaded[1] = 1.0f;
    timelineColorFaded[2] = 1.0f;
    timelineColorFaded[3] = 0.1f;
    controlNamesBoxY = (float)((float)(innerGreyBoxY + 6.0) + v5) + 6.0;
    v6 = 10.0 * 1.6;
    controlNamesBoxHeight = v6;
    memset(controlNamesBoxColor, 0, 12);
    controlNamesBoxColor[3] = 1.0f;
    controlNamesBoxColorFaded[0] = 1.0f;
    controlNamesBoxColorFaded[1] = 1.0f;
    controlNamesBoxColorFaded[2] = 1.0f;
    controlNamesBoxColorFaded[3] = 0.05f;
    controlNamesBoxIconSpacing = 4.0f;
    if ( !Demo_IsClipPreviewRunning()
        && !Demo_IsSaveDialogOpen(localClientNum)
        && !Demo_IsErrorDialogOpen(localClientNum)
        && !Demo_IsPauseMenuOpen(localClientNum)
        && !CG_IsScoreboardDisplayed(localClientNum) )
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        scrPlace = &scrPlaceView[localClientNum];
        spacing = 20.0f;
        textStyle = 6;
        font = UI_GetFontHandle(scrPlace, 3, 0.33333334);
        controlsNamesBoxBackground = Material_RegisterHandle("demo_button_outline", 7);
        enableClipAndScreenshot = 1;
        enableHostControls = 1;
        CL_DrawStretchPic(
            scrPlace,
            rect->x,
            rect->y,
            rect->w,
            rect->h,
            rect->horzAlign,
            rect->vertAlign,
            0.0,
            0.0,
            1.0,
            1.0,
            outerBoxColor,
            cgMedia.demoTimelineFaded);
        if ( Demo_IsClipPlaying() )
        {
            textColor[0] = disabledText[0];
            textColor[1] = disabledText[1];
            textColor[2] = disabledText[2];
            textColor[3] = disabledText[3];
        }
        else
        {
            textColor[0] = colorWhite[0];
            textColor[1] = 1.0;
            textColor[2] = 1.0;
            textColor[3] = 1.0;
            Demo_GetActivatedColorForButton(4, (float (*)[4])textColor);
        }
        displayText = UI_SafeTranslateString("PATCH_DEMO_TOGGLE_HUD");
        displayTextWidth = (float)UI_TextWidth(displayText, 0x7FFFFFFF, font, 0.25);
        UI_DrawText(
            scrPlace,
            (char *)displayText,
            0x7FFFFFFF,
            font,
            (float)(rect->x + rect->w) - (float)(displayTextWidth + 5.0),
            rect->y - 3.0,
            rect->horzAlign,
            rect->vertAlign,
            0.25,
            textColor,
            textStyle);
        textColor[0] = colorWhite[0];
        textColor[1] = 1.0;
        textColor[2] = 1.0;
        textColor[3] = 1.0;
        displayText = UI_SafeTranslateString("PATCH_DEMO_TOGGLE_CONTROLS_HUD");
        displayTextWidth = (float)UI_TextWidth(displayText, 0x7FFFFFFF, font, 0.25);
        Demo_GetActivatedColorForButton(5, (float (*)[4])textColor);
        UI_DrawText(
            scrPlace,
            (char *)displayText,
            0x7FFFFFFF,
            font,
            rect->x,
            rect->y - 3.0,
            rect->horzAlign,
            rect->vertAlign,
            0.25,
            textColor,
            textStyle);
        CL_DrawStretchPic(
            &scrPlaceView[localClientNum],
            innerGreyBoxX,
            innerGreyBoxY,
            innerGreyBoxWidth,
            innerGreyBoxHeight,
            rect->horzAlign,
            rect->vertAlign,
            0.0,
            0.0,
            1.0,
            1.0,
            innerGreyBoxColor,
            cgMedia.whiteMaterial);
        CL_DrawStretchPic(
            &scrPlaceView[localClientNum],
            timelineX,
            timelineY,
            timelineWidth,
            timelineHeight,
            rect->horzAlign,
            rect->vertAlign,
            0.0,
            0.0,
            1.0,
            1.0,
            timelineColor,
            cgMedia.demoTimelineSolid);
        CL_DrawStretchPic(
            &scrPlaceView[localClientNum],
            timelineX + 0.69999999,
            timelineY + 0.69999999,
            timelineWidth - 1.4,
            timelineHeight / 2.0,
            rect->horzAlign,
            rect->vertAlign,
            0.0,
            0.0,
            1.0,
            1.0,
            timelineColorFaded,
            cgMedia.whiteMaterial);
        Demo_GetStartAndEndTime(&startTime, &endTime);
        Demo_GetConnectionTimeInfoForPlayer(localClientNum, &connectTime, &disconnectTime);
        if ( Demo_IsClipPlaying() )
            ClipTime = Demo_GetClipTime();
        else
            ClipTime = cgameGlob->time;
        currentTime = ClipTime;
        duration = endTime - startTime;
        if ( Demo_IsCompleted() )
            v20 = 1.0f;
        else
            v20 = (float)(currentTime - startTime) / (float)duration;
        percentCompleted = v20;
        if ( v20 > 1.0 )
            percentCompleted = 1.0f;
        playerActivityColor[0] = colorMdYellow[0];
        playerActivityColor[1] = 0.5;
        playerActivityColor[2] = 0.0;
        playerActivityColor[3] = 1.0;
        bookmarkEventColor[0] = colorWhite[0];
        bookmarkEventColor[1] = 1.0;
        bookmarkEventColor[2] = 1.0;
        bookmarkEventColor[3] = 1.0;
        timeSec = duration % 60000 / 1000;
        sprintf(timeText, "%02i:%02i", duration / 60000, timeSec);
        v7 = (float)(timelineY + timelineHeight) + outlineBoxSpacing * 0.2;
        s1 = v7;
        v8 = (float)(timelineX + timelineWidth) + outlineBoxSpacing * 0.3;
        UI_DrawText(
            scrPlace,
            timeText,
            0x7FFFFFFF,
            font,
            v8,
            s1,
            rect->horzAlign,
            rect->vertAlign,
            0.25,
            textColor,
            textStyle);
        do
        {
            if ( Demo_IsMovieCamera() )
                break;
            bookmarkEventColor[3] = 0.0f;
            bookmarkIndex = Demo_GetNextDefaultBookmarkForPlayer(
                                                localClientNum,
                                                bookmarkIndex,
                                                &bookmarkType,
                                                &bookmarkTime,
                                                (float (*)[4])bookmarkColor);
            if ( bookmarkIndex >= 0 )
            {
                bookmarkX = (float)SnapFloatToInt((float)((float)((float)(bookmarkTime - startTime) / (float)duration) * timelineWidth))
                                    + timelineX;
                v22 = 1.0
                        - (float)((float)(int)abs(currentTime - bookmarkTime)
                                        / (float)(demo_bookmarkEventThresholdTime->current.value * 1000.0));
                if ( (float)(v22 - 1.0) < 0.0 )
                    v23 = v22;
                else
                    v23 = 1.0f;
                if ( (float)(0.0 - v22) < 0.0 )
                    v19 = v23;
                else
                    v19 = 0.0f;
                bookmarkEventColor[3] = v19;
                Demo_DrawVerticalLine(
                    &scrPlaceView[localClientNum],
                    bookmarkX,
                    timelineY,
                    1.0,
                    timelineHeight,
                    rect->horzAlign,
                    rect->vertAlign,
                    0.0,
                    0.0,
                    1.0,
                    1.0,
                    bookmarkColor,
                    cgMedia.demoTimelineBookmark);
            }
        }
        while ( bookmarkIndex != -1 );
        for ( i = 0; i < 250; ++i )
        {
            keyframeTime = Demo_GetKeyFrameTime(i);
            if ( keyframeTime >= 0 )
            {
                if ( keyframeTime < startTime )
                    keyframeTime = startTime;
                keyframeX = (float)(int)((float)((float)((float)((keyframeTime - startTime) / 1000) / (float)(duration / 1000))
                                                                             * timelineWidth)
                                                             - 0.4999999990686774)
                                    + timelineX;
                Demo_DrawVerticalLine(
                    &scrPlaceView[localClientNum],
                    keyframeX,
                    timelineY,
                    1.0,
                    timelineHeight,
                    rect->horzAlign,
                    rect->vertAlign,
                    0.0,
                    0.0,
                    1.0,
                    1.0,
                    keyframeColor,
                    cgMedia.demoTimelineBookmark);
            }
        }
        if ( !Demo_IsClipPlaying() )
        {
            activityBlockX = timelineX;
            activityBlockW = fabs((float)(startTime - connectTime) / (float)duration)
                                         * timelineWidth;
            CL_DrawStretchPic(
                &scrPlaceView[localClientNum],
                timelineX,
                timelineY,
                activityBlockW,
                timelineHeight,
                rect->horzAlign,
                rect->vertAlign,
                0.0,
                0.0,
                1.0,
                1.0,
                activityBlockColor,
                cgMedia.whiteMaterial);
            activityBlockX = (float)(fabs((float)(disconnectTime - startTime) / (float)duration)
                                                         * timelineWidth)
                                         + timelineX;
            activityBlockW = fabs((float)(endTime - disconnectTime) / (float)duration)
                                         * timelineWidth;
            CL_DrawStretchPic(
                &scrPlaceView[localClientNum],
                activityBlockX,
                timelineY,
                activityBlockW,
                timelineHeight,
                rect->horzAlign,
                rect->vertAlign,
                0.0,
                0.0,
                1.0,
                1.0,
                activityBlockColor,
                cgMedia.whiteMaterial);
        }
        if ( percentCompleted == 1.0 )
        {
            timeSec = duration % 60000 / 1000;
            sprintf(timeText, "%02i:%02i", duration / 60000, timeSec);
        }
        else
        {
            timeSec = (currentTime - startTime) % 60000 / 1000;
            sprintf(timeText, "%02i:%02i", (currentTime - startTime) / 60000, timeSec);
        }
        Demo_DrawVerticalLine(
            &scrPlaceView[localClientNum],
            (float)SnapFloatToInt((float)(timelineWidth * percentCompleted)) + timelineX,
            timelineY - (float)(timelineHeight * 0.30000001),
            1.0,
            timelineHeight * 1.3,
            rect->horzAlign,
            rect->vertAlign,
            0.0,
            0.0,
            1.0,
            1.0,
            colorWhite,
            cgMedia.demoTimelineBookmark);
        CG_DrawRotatedPic(
            &scrPlaceView[localClientNum],
            (float)((float)SnapFloatToInt((float)(timelineWidth * percentCompleted)) + timelineX)
        - (float)(timelineCursorWidth / 2.3),
            timelineY - innerGreyBoxYSpacing,
            timelineCursorWidth,
            timelineCursorHeight,
            rect->horzAlign,
            rect->vertAlign,
            180.0,
            colorWhite,
            cgMedia.demoTimelineCursor);
        timelineTextWidth = (float)UI_TextWidth(timeText, 0x7FFFFFFF, font, 0.25);
        UI_DrawText(
            scrPlace,
            timeText,
            0x7FFFFFFF,
            font,
            (float)((float)SnapFloatToInt((float)(timelineWidth * percentCompleted)) + timelineX)
        - (float)(timelineTextWidth / 2.0),
            timelineY - (float)(0.89999998 * innerGreyBoxYSpacing),
            rect->horzAlign,
            rect->vertAlign,
            0.25,
            color,
            textStyle);
        controlNamesBoxX = timelineX;
        w = innerGreyBoxWidth * 0.23;
        controlNamesBoxWidth = w;
        CL_DrawStretchPic(
            &scrPlaceView[localClientNum],
            timelineX,
            controlNamesBoxY,
            w,
            controlNamesBoxHeight / 2.0,
            rect->horzAlign,
            rect->vertAlign,
            0.0,
            0.0,
            1.0,
            1.0,
            controlNamesBoxColorFaded,
            cgMedia.whiteMaterial);
        CL_DrawStretchPic(
            &scrPlaceView[localClientNum],
            controlNamesBoxX,
            controlNamesBoxY,
            controlNamesBoxWidth,
            controlNamesBoxHeight,
            rect->horzAlign,
            rect->vertAlign,
            0.0,
            0.0,
            1.0,
            1.0,
            controlNamesBoxColor,
            controlsNamesBoxBackground);
        if ( Demo_IsClipPlaying() )
        {
            textColor[0] = disabledText[0];
            textColor[1] = disabledText[1];
            textColor[2] = disabledText[2];
            textColor[3] = disabledText[3];
        }
        else
        {
            textColor[0] = colorWhite[0];
            textColor[1] = 1.0;
            textColor[2] = 1.0;
            textColor[3] = 1.0;
            Demo_GetActivatedColorForButton(3, (float (*)[4])textColor);
        }
        if ( Demo_IsMovieCamera() )
        {
            displayText = UI_SafeTranslateString("MENU_DEMO_DVR_FREE_CAMERA");
        }
        else if ( Demo_IsThirdPersonCamera() )
        {
            displayText = UI_SafeTranslateString("MENU_DEMO_DVR_THIRD_PERSON");
        }
        else
        {
            displayText = UI_SafeTranslateString("MENU_DEMO_DVR_FIRST_PERSON");
        }
        displayTextWidth = (float)UI_TextWidth(displayText, 0x7FFFFFFF, font, 0.25);
        UI_DrawText(
            scrPlace,
            (char *)displayText,
            0x7FFFFFFF,
            font,
            (float)((float)(controlNamesBoxWidth - displayTextWidth) / 2.0) + controlNamesBoxX,
            (float)(controlNamesBoxHeight * 0.89999998) + controlNamesBoxY,
            rect->horzAlign,
            rect->vertAlign,
            0.25,
            textColor,
            textStyle);
        textColor[0] = colorWhite[0];
        textColor[1] = 1.0;
        textColor[2] = 1.0;
        textColor[3] = 1.0;
        displayText = UI_SafeTranslateString("PATCH_DEMO_CHANGE_CAMERA");
        displayTextWidth = (float)UI_TextWidth(displayText, 0x7FFFFFFF, font, 0.25);
        UI_DrawText(
            scrPlace,
            (char *)displayText,
            0x7FFFFFFF,
            font,
            (float)((float)(controlNamesBoxWidth - displayTextWidth) / 2.0) + controlNamesBoxX,
            (float)(controlNamesBoxHeight * 2.2) + controlNamesBoxY,
            rect->horzAlign,
            rect->vertAlign,
            0.25,
            textColor,
            textStyle);
        controlNamesBoxX = (float)(controlNamesBoxWidth + innerGreyBoxXSpacing) + controlNamesBoxX;
        v10 = innerGreyBoxWidth * 0.35;
        controlNamesBoxWidth = v10;
        CL_DrawStretchPic(
            &scrPlaceView[localClientNum],
            controlNamesBoxX,
            controlNamesBoxY,
            v10,
            controlNamesBoxHeight / 2.0,
            rect->horzAlign,
            rect->vertAlign,
            0.0,
            0.0,
            1.0,
            1.0,
            controlNamesBoxColorFaded,
            cgMedia.whiteMaterial);
        CL_DrawStretchPic(
            &scrPlaceView[localClientNum],
            controlNamesBoxX,
            controlNamesBoxY,
            controlNamesBoxWidth,
            controlNamesBoxHeight,
            rect->horzAlign,
            rect->vertAlign,
            0.0,
            0.0,
            1.0,
            1.0,
            controlNamesBoxColor,
            controlsNamesBoxBackground);
        controlNamesBoxIconSetWidth = controlNamesBoxWidth - (float)(2.0 * controlNamesBoxIconSpacing);
        controlNamesBoxIconWidth = (float)(controlNamesBoxIconSetWidth - (float)(controlNamesBoxIconSpacing * 4.0)) / 5.0;
        controlNamesBoxIconHeight = controlNamesBoxHeight * 0.89999998;
        controlNamesBoxIconY = controlNamesBoxY + 1.0;
        if ( !Demo_IsClipRecording() && Demo_GetKeyFrameForJumpBack(localClientNum, -1) >= 0 && enableHostControls )
        {
            iconColor[0] = colorWhite[0];
            iconColor[1] = 1.0;
            iconColor[2] = 1.0;
            iconColor[3] = 1.0;
            Demo_GetActivatedColorForButton(6, (float (*)[4])iconColor);
        }
        else
        {
            iconColor[0] = disabledText[0];
            iconColor[1] = disabledText[1];
            iconColor[2] = disabledText[2];
            iconColor[3] = disabledText[3];
        }
        controlNamesBoxIconX = controlNamesBoxX + controlNamesBoxIconSpacing;
        CG_DrawRotatedPic(
            &scrPlaceView[localClientNum],
            controlNamesBoxX + controlNamesBoxIconSpacing,
            controlNamesBoxIconY,
            controlNamesBoxIconWidth,
            controlNamesBoxIconHeight,
            rect->horzAlign,
            rect->vertAlign,
            180.0,
            iconColor,
            cgMedia.demoStateJump);
        controlNamesBoxIconX = (float)((float)(controlNamesBoxIconSpacing * 2.0) + controlNamesBoxX)
                                                 + (float)(controlNamesBoxIconWidth * 1.0);
        if ( Demo_IsClipPlaying() || !enableHostControls || Demo_IsCompleted() )
        {
            iconColor[0] = disabledText[0];
            iconColor[1] = disabledText[1];
            iconColor[2] = disabledText[2];
            iconColor[3] = disabledText[3];
            CL_DrawStretchPic(
                &scrPlaceView[localClientNum],
                controlNamesBoxIconX,
                controlNamesBoxIconY,
                controlNamesBoxIconWidth,
                controlNamesBoxIconHeight,
                rect->horzAlign,
                rect->vertAlign,
                0.0,
                0.0,
                1.0,
                1.0,
                iconColor,
                cgMedia.demoStateForwardSlow);
        }
        else
        {
            iconColor[0] = colorWhite[0];
            iconColor[1] = 1.0;
            iconColor[2] = 1.0;
            iconColor[3] = 1.0;
            result = Demo_GetActivatedColorForButton(8, (float (*)[4])iconColor);
            if ( Com_GetTimeScale() >= 1.0 )
            {
                CL_DrawStretchPic(
                    &scrPlaceView[localClientNum],
                    controlNamesBoxIconX,
                    controlNamesBoxIconY,
                    controlNamesBoxIconWidth,
                    controlNamesBoxIconHeight,
                    rect->horzAlign,
                    rect->vertAlign,
                    0.0,
                    0.0,
                    1.0,
                    1.0,
                    iconColor,
                    cgMedia.demoStateForwardSlow);
            }
            else
            {
                TimeScale = Com_GetTimeScale();
                sprintf(timeText, "%.1fx", TimeScale);
                UI_DrawText(
                    scrPlace,
                    timeText,
                    0x7FFFFFFF,
                    font,
                    controlNamesBoxIconX,
                    (float)(controlNamesBoxIconY + controlNamesBoxIconHeight) - 2.0,
                    rect->horzAlign,
                    rect->vertAlign,
                    0.25,
                    iconColor,
                    textStyle);
            }
        }
        if ( !Demo_IsCompleted() && enableHostControls )
        {
            iconColor[0] = colorWhite[0];
            iconColor[1] = 1.0;
            iconColor[2] = 1.0;
            iconColor[3] = 1.0;
            Demo_GetActivatedColorForButton(0, (float (*)[4])iconColor);
        }
        else
        {
            iconColor[0] = disabledText[0];
            iconColor[1] = disabledText[1];
            iconColor[2] = disabledText[2];
            iconColor[3] = disabledText[3];
        }
        controlNamesBoxIconX = (float)((float)(controlNamesBoxIconSpacing * 3.0) + controlNamesBoxX)
                                                 + (float)(controlNamesBoxIconWidth * 2.0);
        if ( Demo_IsCompleted() )
        {
            CL_DrawStretchPic(
                &scrPlaceView[localClientNum],
                controlNamesBoxIconX,
                controlNamesBoxIconY,
                controlNamesBoxIconWidth,
                controlNamesBoxIconHeight,
                rect->horzAlign,
                rect->vertAlign,
                0.0,
                0.0,
                1.0,
                1.0,
                iconColor,
                cgMedia.demoStateStop);
        }
        else if ( Demo_IsPaused() )
        {
            CL_DrawStretchPic(
                &scrPlaceView[localClientNum],
                controlNamesBoxIconX,
                controlNamesBoxIconY,
                controlNamesBoxIconWidth,
                controlNamesBoxIconHeight,
                rect->horzAlign,
                rect->vertAlign,
                0.0,
                0.0,
                1.0,
                1.0,
                iconColor,
                cgMedia.demoStatePlay);
        }
        else
        {
            CL_DrawStretchPic(
                &scrPlaceView[localClientNum],
                controlNamesBoxIconX,
                controlNamesBoxIconY,
                controlNamesBoxIconWidth,
                controlNamesBoxIconHeight,
                rect->horzAlign,
                rect->vertAlign,
                0.0,
                0.0,
                1.0,
                1.0,
                iconColor,
                cgMedia.demoStatePaused);
        }
        textColor[0] = colorWhite[0];
        textColor[1] = 1.0;
        textColor[2] = 1.0;
        textColor[3] = 1.0;
        displayText = UI_SafeTranslateString("PLATFORM_DEMO_PLAY");
        displayTextWidth = (float)UI_TextWidth(displayText, 0x7FFFFFFF, font, 0.25);
        UI_DrawText(
            scrPlace,
            (char *)displayText,
            0x7FFFFFFF,
            font,
            (float)((float)(controlNamesBoxIconWidth - displayTextWidth) / 2.0) + controlNamesBoxIconX,
            (float)(controlNamesBoxHeight * 2.2) + controlNamesBoxY,
            rect->horzAlign,
            rect->vertAlign,
            0.25,
            textColor,
            textStyle);
        controlNamesBoxIconX = (float)((float)(controlNamesBoxIconSpacing * 4.0) + controlNamesBoxX)
                                                 + (float)(controlNamesBoxIconWidth * 3.0);
        if ( Demo_IsClipPlaying() || !enableHostControls || Demo_IsCompleted() )
        {
            iconColor[0] = disabledText[0];
            iconColor[1] = disabledText[1];
            iconColor[2] = disabledText[2];
            iconColor[3] = disabledText[3];
            CL_DrawStretchPic(
                &scrPlaceView[localClientNum],
                controlNamesBoxIconX,
                controlNamesBoxIconY,
                controlNamesBoxIconWidth,
                controlNamesBoxIconHeight,
                rect->horzAlign,
                rect->vertAlign,
                0.0,
                0.0,
                1.0,
                1.0,
                iconColor,
                cgMedia.demoStateForwardFast);
        }
        else
        {
            iconColor[0] = colorWhite[0];
            iconColor[1] = 1.0;
            iconColor[2] = 1.0;
            iconColor[3] = 1.0;
            result = Demo_GetActivatedColorForButton(9, (float (*)[4])iconColor);
            if ( Com_GetTimeScale() <= 1.0 )
            {
                CL_DrawStretchPic(
                    &scrPlaceView[localClientNum],
                    controlNamesBoxIconX,
                    controlNamesBoxIconY,
                    controlNamesBoxIconWidth,
                    controlNamesBoxIconHeight,
                    rect->horzAlign,
                    rect->vertAlign,
                    0.0,
                    0.0,
                    1.0,
                    1.0,
                    iconColor,
                    cgMedia.demoStateForwardFast);
            }
            else
            {
                v12 = Com_GetTimeScale();
                sprintf(timeText, "%.1fx", v12);
                UI_DrawText(
                    scrPlace,
                    timeText,
                    0x7FFFFFFF,
                    font,
                    controlNamesBoxIconX,
                    (float)(controlNamesBoxIconY + controlNamesBoxIconHeight) - 2.0,
                    rect->horzAlign,
                    rect->vertAlign,
                    0.25,
                    iconColor,
                    textStyle);
            }
        }
        if ( !Demo_IsClipRecording() && Demo_GetKeyFrameForJumpForward(localClientNum, -1) >= 0 && enableHostControls )
        {
            iconColor[0] = colorWhite[0];
            iconColor[1] = 1.0;
            iconColor[2] = 1.0;
            iconColor[3] = 1.0;
            Demo_GetActivatedColorForButton(7, (float (*)[4])iconColor);
        }
        else
        {
            iconColor[0] = disabledText[0];
            iconColor[1] = disabledText[1];
            iconColor[2] = disabledText[2];
            iconColor[3] = disabledText[3];
        }
        controlNamesBoxIconX = (float)((float)(controlNamesBoxIconSpacing * 5.0) + controlNamesBoxX)
                                                 + (float)(controlNamesBoxIconWidth * 4.0);
        CL_DrawStretchPic(
            &scrPlaceView[localClientNum],
            controlNamesBoxIconX,
            controlNamesBoxIconY,
            controlNamesBoxIconWidth,
            controlNamesBoxIconHeight,
            rect->horzAlign,
            rect->vertAlign,
            0.0,
            0.0,
            1.0,
            1.0,
            iconColor,
            cgMedia.demoStateJump);
        textColor[0] = colorWhite[0];
        textColor[1] = 1.0;
        textColor[2] = 1.0;
        textColor[3] = 1.0;
        Demo_GetCurrentControllerConfig();
        controlNamesBoxIconX = controlNamesBoxX + controlNamesBoxIconSpacing;
        displayText = UI_SafeTranslateString("PATCH_DEMO_SKIPBACK");
        displayTextWidth = (float)UI_TextWidth(displayText, 0x7FFFFFFF, font, 0.25);
        CL_DrawStretchPic(
            &scrPlaceView[localClientNum],
            (float)((float)(controlNamesBoxIconWidth - displayTextWidth) / 2.0) + controlNamesBoxIconX,
            (float)((float)(controlNamesBoxHeight * 2.2) + controlNamesBoxY) - 14.0,
            15.0,
            15.0,
            rect->horzAlign,
            rect->vertAlign,
            0.0,
            0.0,
            1.0,
            1.0,
            colorWhite,
            cgMedia.theaterLeftArrow);
        controlNamesBoxIconX = (float)((float)(controlNamesBoxIconSpacing * 2.0) + controlNamesBoxX)
                                                 + (float)(controlNamesBoxIconWidth * 1.0);
        displayText = UI_SafeTranslateString("PATCH_DEMO_SPEED_DOWN");
        displayTextWidth = (float)UI_TextWidth(displayText, 0x7FFFFFFF, font, 0.25);
        CL_DrawStretchPic(
            &scrPlaceView[localClientNum],
            (float)((float)((float)(controlNamesBoxIconWidth - displayTextWidth) / 2.0) + controlNamesBoxIconX) - 2.0,
            (float)((float)(controlNamesBoxHeight * 2.2) + controlNamesBoxY) - 14.0,
            15.0,
            15.0,
            rect->horzAlign,
            rect->vertAlign,
            0.0,
            0.0,
            1.0,
            1.0,
            colorWhite,
            cgMedia.theaterDownArrow);
        controlNamesBoxIconX = (float)((float)(controlNamesBoxIconSpacing * 4.0) + controlNamesBoxX)
                                                 + (float)(controlNamesBoxIconWidth * 3.0);
        displayText = UI_SafeTranslateString("PATCH_DEMO_SPEED_UP");
        displayTextWidth = (float)UI_TextWidth(displayText, 0x7FFFFFFF, font, 0.25);
        CL_DrawStretchPic(
            &scrPlaceView[localClientNum],
            (float)((float)(controlNamesBoxIconWidth - displayTextWidth) / 2.0) + controlNamesBoxIconX,
            (float)((float)(controlNamesBoxHeight * 2.2) + controlNamesBoxY) - 14.0,
            15.0,
            15.0,
            rect->horzAlign,
            rect->vertAlign,
            0.0,
            0.0,
            1.0,
            1.0,
            colorWhite,
            cgMedia.theaterUpArrow);
        controlNamesBoxIconX = (float)((float)(controlNamesBoxIconSpacing * 5.0) + controlNamesBoxX)
                                                 + (float)(controlNamesBoxIconWidth * 4.0);
        displayText = UI_SafeTranslateString("PATCH_DEMO_SKIPFOWARD");
        displayTextWidth = (float)UI_TextWidth(displayText, 0x7FFFFFFF, font, 0.25);
        CL_DrawStretchPic(
            &scrPlaceView[localClientNum],
            (float)((float)(controlNamesBoxIconWidth - displayTextWidth) / 2.0) + controlNamesBoxIconX,
            (float)((float)(controlNamesBoxHeight * 2.2) + controlNamesBoxY) - 14.0,
            15.0,
            15.0,
            rect->horzAlign,
            rect->vertAlign,
            0.0,
            0.0,
            1.0,
            1.0,
            colorWhite,
            cgMedia.theaterRightArrow);
        controlNamesBoxX = (float)(controlNamesBoxWidth + innerGreyBoxXSpacing) + controlNamesBoxX;
        v13 = innerGreyBoxWidth * 0.15;
        controlNamesBoxWidth = v13;
        if ( Demo_IsClipRecording() )
        {
            iconColor[0] = colorRed[0];
            iconColor[1] = 0.0;
            iconColor[2] = 0.0;
            iconColor[3] = 1.0;
            colorCalculation = (float)((int)Sys_Milliseconds() % 1000) / 500.0;
            if ( colorCalculation >= 1.0 )
                iconColor[3] = 2.0 - colorCalculation;
            else
                iconColor[3] = colorCalculation;
            CL_DrawStretchPic(
                &scrPlaceView[localClientNum],
                controlNamesBoxX,
                controlNamesBoxY,
                controlNamesBoxWidth,
                controlNamesBoxHeight,
                rect->horzAlign,
                rect->vertAlign,
                0.0,
                0.0,
                1.0,
                1.0,
                iconColor,
                cgMedia.whiteMaterial);
        }
        else
        {
            CL_DrawStretchPic(
                &scrPlaceView[localClientNum],
                controlNamesBoxX,
                controlNamesBoxY,
                controlNamesBoxWidth,
                controlNamesBoxHeight / 2.0,
                rect->horzAlign,
                rect->vertAlign,
                0.0,
                0.0,
                1.0,
                1.0,
                controlNamesBoxColorFaded,
                cgMedia.whiteMaterial);
            CL_DrawStretchPic(
                &scrPlaceView[localClientNum],
                controlNamesBoxX,
                controlNamesBoxY,
                controlNamesBoxWidth,
                controlNamesBoxHeight,
                rect->horzAlign,
                rect->vertAlign,
                0.0,
                0.0,
                1.0,
                1.0,
                controlNamesBoxColor,
                controlsNamesBoxBackground);
        }
        if ( Demo_IsClipPlaying() || !enableClipAndScreenshot || Demo_IsCompleted() )
        {
            textColor[0] = disabledText[0];
            textColor[1] = disabledText[1];
            textColor[2] = disabledText[2];
            textColor[3] = disabledText[3];
        }
        else
        {
            textColor[0] = colorWhite[0];
            textColor[1] = 1.0;
            textColor[2] = 1.0;
            textColor[3] = 1.0;
            Demo_GetActivatedColorForButton(1, (float (*)[4])textColor);
        }
        displayText = UI_SafeTranslateString("MENU_DEMO_DVR_RECORD");
        displayTextWidth = (float)UI_TextWidth(displayText, 0x7FFFFFFF, font, 0.25);
        v14 = controlNamesBoxY + controlNamesBoxHeight * 0.9;
        UI_DrawText(
            scrPlace,
            (char *)displayText,
            0x7FFFFFFF,
            font,
            (float)((float)(controlNamesBoxWidth - displayTextWidth) / 2.0) + controlNamesBoxX,
            v14,
            rect->horzAlign,
            rect->vertAlign,
            0.25,
            textColor,
            textStyle);
        textColor[0] = colorWhite[0];
        textColor[1] = 1.0;
        textColor[2] = 1.0;
        textColor[3] = 1.0;
        displayText = UI_SafeTranslateString("PLATFORM_DEMO_RECORD");
        displayTextWidth = (float)UI_TextWidth(displayText, 0x7FFFFFFF, font, 0.25);
        UI_DrawText(
            scrPlace,
            (char *)displayText,
            0x7FFFFFFF,
            font,
            (float)((float)(controlNamesBoxWidth - displayTextWidth) / 2.0) + controlNamesBoxX,
            (float)(controlNamesBoxHeight * 2.2) + controlNamesBoxY,
            rect->horzAlign,
            rect->vertAlign,
            0.25,
            textColor,
            textStyle);
        v15 = controlNamesBoxX + controlNamesBoxWidth + innerGreyBoxXSpacing * 0.6;
        controlNamesBoxX = v15;
        v16 = innerGreyBoxWidth * 0.2;
        controlNamesBoxWidth = v16;
        CL_DrawStretchPic(
            &scrPlaceView[localClientNum],
            controlNamesBoxX,
            controlNamesBoxY,
            v16,
            controlNamesBoxHeight / 2.0,
            rect->horzAlign,
            rect->vertAlign,
            0.0,
            0.0,
            1.0,
            1.0,
            controlNamesBoxColorFaded,
            cgMedia.whiteMaterial);
        CL_DrawStretchPic(
            &scrPlaceView[localClientNum],
            controlNamesBoxX,
            controlNamesBoxY,
            controlNamesBoxWidth,
            controlNamesBoxHeight,
            rect->horzAlign,
            rect->vertAlign,
            0.0,
            0.0,
            1.0,
            1.0,
            controlNamesBoxColor,
            controlsNamesBoxBackground);
        if ( enableClipAndScreenshot && !Demo_IsClipRecording() )
        {
            textColor[0] = colorWhite[0];
            textColor[1] = 1.0;
            textColor[2] = 1.0;
            textColor[3] = 1.0;
        }
        else
        {
            textColor[0] = disabledText[0];
            textColor[1] = disabledText[1];
            textColor[2] = disabledText[2];
            textColor[3] = disabledText[3];
        }
        displayText = UI_SafeTranslateString("MENU_DEMO_DVR_SCREENSHOT");
        displayTextWidth = (float)UI_TextWidth(displayText, 0x7FFFFFFF, font, 0.25);
        v17 = controlNamesBoxY + controlNamesBoxHeight * 0.9;
        UI_DrawText(
            scrPlace,
            (char *)displayText,
            0x7FFFFFFF,
            font,
            (float)((float)(controlNamesBoxWidth - displayTextWidth) / 2.0) + controlNamesBoxX,
            v17,
            rect->horzAlign,
            rect->vertAlign,
            0.25,
            textColor,
            textStyle);
        textColor[0] = colorWhite[0];
        textColor[1] = 1.0;
        textColor[2] = 1.0;
        textColor[3] = 1.0;
        displayText = UI_SafeTranslateString("PLATFORM_DEMO_TAKE_SCREENSHOT");
        displayTextWidth = (float)UI_TextWidth(displayText, 0x7FFFFFFF, font, 0.25);
        UI_DrawText(
            scrPlace,
            (char *)displayText,
            0x7FFFFFFF,
            font,
            (float)((float)(controlNamesBoxWidth - displayTextWidth) / 2.0) + controlNamesBoxX,
            (float)(controlNamesBoxHeight * 2.2) + controlNamesBoxY,
            rect->horzAlign,
            rect->vertAlign,
            0.25,
            textColor,
            textStyle);
    }
}

void    CG_DrawPlayerImageSwing(
                int localClientNum,
                const rectDef_s *rect,
                const rectDef_s *parentRect,
                Material *material,
                float *color)
{
    long double v6; // [esp+1Ch] [ebp-48Ch]
    long double v7; // [esp+1Ch] [ebp-48Ch]
    long double v8; // [esp+1Ch] [ebp-48Ch]
    long double v9; // [esp+1Ch] [ebp-48Ch]
    float rankUpColor[4]; // [esp+24h] [ebp-484h] BYREF
    Material *rankUpMaterial; // [esp+34h] [ebp-474h]
    const char *rankUpMaterialName; // [esp+38h] [ebp-470h]
    char rankUpMaterialDvarName[1024]; // [esp+3Ch] [ebp-46Ch] BYREF
    float v14; // [esp+440h] [ebp-68h]
    float v16; // [esp+448h] [ebp-60h]
    float v18; // [esp+450h] [ebp-58h]
    float v20; // [esp+458h] [ebp-50h]
    float rotatedY; // [esp+460h] [ebp-48h]
    float rotatedX; // [esp+464h] [ebp-44h]
    float swingPoint; // [esp+468h] [ebp-40h]
    float previousPopUpAngle; // [esp+46Ch] [ebp-3Ch]
    float ratio; // [esp+470h] [ebp-38h]
    int time; // [esp+474h] [ebp-34h]
    const ScreenPlacement *scrPlace; // [esp+478h] [ebp-30h]
    cg_s *cgameGlob; // [esp+47Ch] [ebp-2Ch]
    float maxSwayAngle; // [esp+480h] [ebp-28h]
    float rotation; // [esp+484h] [ebp-24h]
    float swingLength; // [esp+488h] [ebp-20h]
    float swayMaxTime; // [esp+48Ch] [ebp-1Ch]
    float maxAngleChange; // [esp+490h] [ebp-18h]
    int swayStartTime; // [esp+494h] [ebp-14h]
    //_UNKNOWN *v36; // [esp+49Ch] [ebp-Ch]
    //int localClientNuma; // [esp+4A0h] [ebp-8h]
    //const rectDef_s *parentRecta; // [esp+4A8h] [ebp+0h]
    //
    //v36 = a1;
    //localClientNuma = (int)parentRecta;
    swayStartTime = 0;
    maxAngleChange = 0.5f;
    swayMaxTime = 2500.0f;
    swingLength = 100.0f;
    rotation = 0.0f;
    maxSwayAngle = 7.0f;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    scrPlace = &scrPlaceView[localClientNum];
    swayStartTime = cgameGlob->popUpSwayStartTime;
    time = cgameGlob->time;
    if (swayStartTime)
    {
        if ((float)time > (float)((float)swayStartTime + swayMaxTime))
        {
            cgameGlob->popupRotationAngle = 0.0f;
            cgameGlob->popUpAngleDelta = 0.0f;
        }
        ratio = (float)((float)((float)swayStartTime + swayMaxTime) - (float)time) / swayMaxTime;
        previousPopUpAngle = cgameGlob->popupRotationAngle;
        if (cgameGlob->popupRotationAngle > (float)((float)(ratio * ratio) * maxSwayAngle))
            cgameGlob->popUpAngleDelta = (-(maxAngleChange)) * (float)(ratio * ratio);
        if ((float)((-(maxSwayAngle)) * (float)(ratio * ratio)) > cgameGlob->popupRotationAngle)
            cgameGlob->popUpAngleDelta = (float)(ratio * ratio) * maxAngleChange;
        cgameGlob->popupRotationAngle = cgameGlob->popupRotationAngle + cgameGlob->popUpAngleDelta;
        if (Demo_IsPaused() || Demo_IsCompleted() || Demo_GetClipPausedState())
            cgameGlob->popupRotationAngle = previousPopUpAngle;
        swingPoint = swingLength + rect->y;
        rotatedX = rect->x;
        rotatedY = rect->y;
        if (swayStartTime && (float)((float)swayStartTime + swayMaxTime) > (float)time)
        {
            //v21 = cgameGlob->popupRotationAngle * 0.017453292;
            //__libm_sse2_cos(v6);
            v20 = cos(DEG2RAD(cgameGlob->popupRotationAngle));// v21;

            //v19 = (-(cgameGlob->popupRotationAngle)) * 0.017453292;
            //__libm_sse2_sin(v7);
            v18 = sin(DEG2RAD((-(cgameGlob->popupRotationAngle))));

            rotatedX = (float)((float)((float)(rect->w / 2.0) + rect->x) + (float)(1.0 - v20)) + (float)(v18 * swingPoint);

            //v17 = cgameGlob->popupRotationAngle * 0.017453292;
            //__libm_sse2_sin(v8);
            v16 = sin(DEG2RAD(cgameGlob->popupRotationAngle));// v17;

            //v15 = (-(cgameGlob->popupRotationAngle)) * 0.017453292;
            //__libm_sse2_cos(v9);
            v14 = cos(DEG2RAD((-(cgameGlob->popupRotationAngle))));// v15;
            rotatedY = (float)(rect->y - v16) + (float)((float)(v14 * swingPoint) - swingPoint);
            rotation = cgameGlob->popupRotationAngle;
        }
        if (!localClientNum)
        {
            Com_sprintf(rankUpMaterialDvarName, 0x400u, "rankUpPopUpIcon%d", 0);
            rankUpMaterialName = Dvar_GetString(rankUpMaterialDvarName);
            rankUpMaterial = Material_RegisterHandle((char *)rankUpMaterialName, 7);
            rankUpColor[0] = 1.0f;
            rankUpColor[1] = 1.0f;
            rankUpColor[2] = 1.0f;
            rankUpColor[3] = color[3];
            CL_DrawStretchPicRotatedXY(scrPlace, rotatedX, rotatedY, rect->w, rect->h, rotation, rankUpColor, rankUpMaterial);
        }
    }
}

void __cdecl CG_DrawPlayerTargetHighlights(int localClientNum, const rectDef_s *rect, Material *material, float *color)
{
    int clientNum; // ecx
    int v5; // ecx
    const clientInfo_t *client; // [esp+54h] [ebp-30h]
    centity_s *cent; // [esp+58h] [ebp-2Ch]
    int i; // [esp+5Ch] [ebp-28h]
    const clientInfo_t *localClientInfo; // [esp+60h] [ebp-24h]
    float distanceToTarget; // [esp+64h] [ebp-20h]
    cg_s *cgameGlob; // [esp+68h] [ebp-1Ch]
    float currentTargetSize; // [esp+6Ch] [ebp-18h]
    float midPoint[3]; // [esp+70h] [ebp-14h] BYREF
    float screenPos[2]; // [esp+7Ch] [ebp-8h] BYREF

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_newDraw_mp.cpp", 3725, 0, "%s", "cgameGlob") )
    {
        __debugbreak();
    }
    clientNum = cgameGlob->clientNum;
    localClientInfo = &cgameGlob->bgs.clientinfo[clientNum];
    if ( (cg_s *)((char *)cgameGlob + clientNum * 1480) == (cg_s *)-389096
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_newDraw_mp.cpp",
                    3728,
                    0,
                    "%s",
                    "localClientInfo") )
    {
        __debugbreak();
    }
    if ( ShouldDrawPlayerTargetHighlights(localClientNum, cgameGlob) )
    {
        for ( i = 0; i < 32; ++i )
        {
            cent = CG_GetEntity(localClientNum, i);
            if ( ((*((unsigned int *)cent + 201) >> 1) & 1) != 0
                && cent->nextState.number != cgameGlob->clientNum
                && cent->nextState.eType == 1
                && (cent->nextState.lerp.eFlags & 0x40000) == 0 )
            {
                v5 = cent->nextState.clientNum;
                client = &cgameGlob->bgs.clientinfo[v5];
                if ( (cg_s *)((char *)cgameGlob + v5 * 1480) == (cg_s *)-389096
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_newDraw_mp.cpp",
                                3753,
                                0,
                                "%s",
                                "client") )
                {
                    __debugbreak();
                }
                if ( (client->team == TEAM_FREE || client->team != localClientInfo->team) && (client->perks[1] & 8) == 0 )
                {
                    midPoint[0] = cent->pose.origin[0];
                    midPoint[1] = cent->pose.origin[1];
                    midPoint[2] = cent->pose.origin[2];
                    midPoint[2] = (float)(70.0 / 2.0) + midPoint[2];
                    distanceToTarget = Vec3Distance(cgameGlob->refdef.vieworg, midPoint);
                    currentTargetSize = (float)(100.0 / distanceToTarget) * cg_playerHighlightTargetSize->current.value;
                    WorldPosToScreenPos(localClientNum, midPoint, screenPos);
                    screenPos[0] = screenPos[0] - (float)(currentTargetSize / 2.0);
                    screenPos[1] = screenPos[1] - (float)(currentTargetSize / 2.0);
                    CL_DrawStretchPicPhysical(
                        screenPos[0],
                        screenPos[1],
                        currentTargetSize,
                        currentTargetSize,
                        0.0,
                        0.0,
                        1.0,
                        1.0,
                        color,
                        material);
                }
            }
        }
    }
}

char __cdecl ShouldDrawPlayerTargetHighlights(int localClientNum, const cg_s *cgameGlob)
{
    centity_s *vehicle; // [esp+10h] [ebp-Ch]
    int seatIndex; // [esp+18h] [ebp-4h]

    if ( cgameGlob->cameraMode == 1 )
        return 0;
    if ( (cgameGlob->predictedPlayerState.eFlags2 & 0x10000000) == 0 )
        return 0;
    if ( (cgameGlob->predictedPlayerState.eFlags & 0x4000) != 0 )
    {
        if ( cgameGlob->predictedPlayerState.vehicleType != 6 )
            return 0;
        seatIndex = cgameGlob->predictedPlayerState.vehiclePos;
        if ( seatIndex < 1 || seatIndex > 4 )
        {
            if ( seatIndex >= 5 && seatIndex <= 10 )
                return 0;
        }
        else
        {
            vehicle = CG_GetEntity(localClientNum, cgameGlob->predictedPlayerState.viewlocked_entNum);
            if ( !*(_WORD *)&CG_GetVehicleInfo(vehicle->nextState.vehicleState.vehicleInfoIndex)->gunnerWeapon[3][2 * seatIndex + 62] )
                return 0;
        }
    }
    return 1;
}

void __cdecl CG_DrawPlayerTargetHighlightsFriendly(
                int localClientNum,
                const rectDef_s *rect,
                Material *material,
                float *color)
{
    float distanceToTarget; // [esp+44h] [ebp-24h]
    cg_s *cgameGlob; // [esp+48h] [ebp-20h]
    centity_s *cent; // [esp+4Ch] [ebp-1Ch]
    float currentTargetSize; // [esp+50h] [ebp-18h]
    float midPoint[3]; // [esp+54h] [ebp-14h] BYREF
    float screenPos[2]; // [esp+60h] [ebp-8h] BYREF

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_newDraw_mp.cpp", 3783, 0, "%s", "cgameGlob") )
    {
        __debugbreak();
    }
    if ( ShouldDrawPlayerTargetHighlights(localClientNum, cgameGlob) )
    {
        cent = CG_GetEntity(localClientNum, cgameGlob->clientNum);
        if ( ((*((unsigned int *)cent + 201) >> 1) & 1) != 0
            && cent->nextState.eType == 1
            && (cent->nextState.lerp.eFlags & 0x40000) == 0 )
        {
            midPoint[0] = cent->pose.origin[0];
            midPoint[1] = cent->pose.origin[1];
            midPoint[2] = cent->pose.origin[2];
            midPoint[2] = (float)(70.0 / 2.0) + midPoint[2];
            distanceToTarget = Vec3Distance(cgameGlob->refdef.vieworg, midPoint);
            currentTargetSize = (float)(100.0 / distanceToTarget) * cg_playerHighlightTargetSize->current.value;
            WorldPosToScreenPos(localClientNum, midPoint, screenPos);
            CL_DrawStretchPicPhysical(
                screenPos[0] - (float)(currentTargetSize / 2.0),
                screenPos[1] - (float)(currentTargetSize / 2.0),
                currentTargetSize,
                currentTargetSize,
                0.0,
                0.0,
                1.0,
                1.0,
                color,
                material);
        }
    }
}

void __cdecl CG_DrawGuidedMissileFuel(int localClientNum, const rectDef_s *rect, Material *material)
{
    float v3; // [esp+30h] [ebp-54h]
    float v4; // [esp+34h] [ebp-50h]
    float iconColor[4]; // [esp+4Ch] [ebp-38h] BYREF
    float fuelValue; // [esp+5Ch] [ebp-28h]
    float h; // [esp+60h] [ebp-24h]
    float UsedColor[4]; // [esp+64h] [ebp-20h] BYREF
    cg_s *cgameGlob; // [esp+74h] [ebp-10h]
    int weaponIndex; // [esp+78h] [ebp-Ch]
    const playerState_s *ps; // [esp+7Ch] [ebp-8h]
    const WeaponDef *weapDef; // [esp+80h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    ps = &cgameGlob->predictedPlayerState;
    weaponIndex = cgameGlob->predictedPlayerState.weapon;
    weapDef = BG_GetWeaponDef(weaponIndex);
    if ( (cgameGlob->predictedPlayerState.eFlags2 & 0x40000) != 0
        || weapDef->guidedMissileType == MISSILE_GUIDANCE_TVGUIDED && ps->fWeaponPosFrac >= 0.94999999 )
    {
        if ( (ps->eFlags2 & 0x40000) != 0 )
            fuelValue = (float)BG_PlayerFuelTankTime(ps, weaponIndex) / (float)(weapDef->projLifetime * 1000.0);
        else
            fuelValue = 0.0f;
        if ( (float)(1.0 - (float)(fuelValue + 0.2)) < 0.0 )
            v4 = 1.0f;
        else
            v4 = fuelValue + 0.2;
        if ( (float)((float)(0.80000001 - fuelValue) - 0.0) < 0.0 )
            v3 = 0.0f;
        else
            v3 = 0.80000001 - fuelValue;
        iconColor[0] = v4;
        iconColor[1] = v3;
        iconColor[2] = 0.1f;
        iconColor[3] = 1.0f;
        UsedColor[0] = 0.4f;
        UsedColor[1] = 0.4f;
        UsedColor[2] = 0.4f;
        UsedColor[3] = 1.0f;
        if ( fuelValue >= 0.0 )
        {
            h = rect->h * fuelValue;
            CL_DrawStretchPic(
                &scrPlaceView[localClientNum],
                rect->x,
                rect->y,
                rect->w,
                rect->h,
                rect->horzAlign,
                rect->vertAlign,
                0.0,
                0.0,
                1.0,
                1.0,
                iconColor,
                material);
            CL_DrawStretchPic(
                &scrPlaceView[localClientNum],
                rect->x,
                rect->y,
                rect->w,
                h,
                rect->horzAlign,
                rect->vertAlign,
                0.0,
                0.0,
                fuelValue,
                1.0,
                UsedColor,
                material);
        }
    }
}

void __cdecl CG_OwnerDrawText(
                int localClientNum,
                rectDef_s parentRect,
                float x,
                float y,
                float w,
                float h,
                int horzAlign,
                int vertAlign,
                float text_x,
                float text_y,
                int ownerDraw,
                int ownerDrawFlags,
                int align,
                Font_s *font,
                float scale,
                float *color,
                Material *material,
                int textStyle,
                int textAlignMode,
                itemDef_s *item,
                char *text)
{
    const cg_s *cgameGlob; // [esp+28h] [ebp-24h]
    rectDef_s rect; // [esp+30h] [ebp-1Ch] BYREF
    const playerState_s *ps; // [esp+48h] [ebp-4h]
    int savedregs; // [esp+4Ch] [ebp+0h] BYREF

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    CL_GetLocalClientUIGlobals(localClientNum);
    ps = CG_GetPredictedPlayerState(localClientNum);
    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_newDraw_mp.cpp", 4218, 0, "%s", "ps") )
        __debugbreak();
    if ( ps->offhandSecondary >= (unsigned int)PLAYER_OFFHAND_SECONDARIES_TOTAL
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_newDraw_mp.cpp",
                    4226,
                    0,
                    "%s",
                    "ps->offhandSecondary == PLAYER_OFFHAND_SECONDARY_FLASH") )
    {
        __debugbreak();
    }
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    rect.horzAlign = horzAlign;
    rect.vertAlign = vertAlign;
    if ( ownerDraw > 311 )
    {
        switch ( ownerDraw )
        {
            case 319:
                if ( Dvar_GetBool("compassGridEnabled") )
                    CG_CompassDrawGridPoints(
                        localClientNum,
                        COMPASS_TYPE_PARTIAL,
                        &parentRect,
                        &rect,
                        color,
                        font,
                        scale,
                        textStyle);
                break;
            case 329:
                CG_DrawPlayerPopUps(localClientNum, &rect, font, scale, color, material, textStyle, item, text, 1);
                break;
            case 336:
                CG_DrawPlayerPopUps(localClientNum, &rect, font, scale, color, material, textStyle, item, text, 0);
                break;
            case 337:
                CG_DrawPlayerCOD7TypeWriter(localClientNum, &rect, font, scale, color, material, textStyle, item, text);
                break;
            case 338:
                CG_DrawPlayerPopInText(localClientNum, &rect, font, scale, color, material, textStyle, item, text);
                break;
            case 364:
                CG_DrawGuidedMissileBoostHint(localClientNum, &rect, font, scale, textStyle);
                break;
            case 365:
                CG_DrawGuidedMissileDetonateHint(localClientNum, &rect, font, scale, textStyle);
                break;
            case 366:
                CG_DrawGuidedMissileInfo(localClientNum, &rect, font, scale, textStyle, GUIDED_MISSILE_VIEWPOS);
                break;
            case 367:
                CG_DrawGuidedMissileInfo(localClientNum, &rect, font, scale, textStyle, GUIDED_MISSILE_DIST_TO_TARG);
                break;
            case 368:
                CG_DrawGuidedMissileInfo(localClientNum, &rect, font, scale, textStyle, GUIDED_MISSILE_DIST_FROM_OWNER);
                break;
            case 371:
                CG_DrawRCBombHints(localClientNum, &rect, font, scale, textStyle, 371);
                break;
            case 372:
                CG_DrawRCBombHints(localClientNum, &rect, font, scale, textStyle, 372);
                break;
            case 373:
                CG_DrawRCBombHints(localClientNum, &rect, font, scale, textStyle, 373);
                break;
            case 374:
                CG_DrawRCBombHints(localClientNum, &rect, font, scale, textStyle, 374);
                break;
            case 375:
                CG_DrawRCBombHints(localClientNum, &rect, font, scale, textStyle, 375);
                break;
            case 378:
                CG_DrawTurretPlaceHint(localClientNum, &rect, font, scale, color, textStyle);
                break;
            case 383:
                CG_DrawPlayerRedationText(localClientNum, &rect, font, scale, color, material, textStyle, item, text);
                break;
            case 392:
            case 393:
            case 394:
            case 395:
                CG_DrawPlayerActionSlotBind(
                    localClientNum,
                    &rect,
                    ownerDraw - 392,
                    color,
                    font,
                    scale,
                    textStyle,
                    textAlignMode,
                    item,
                    text);
                break;
            default:
LABEL_59:
                if ( !cgameGlob->inKillCam && ownerDraw == 320 && Dvar_GetBool("compassGridEnabled") )
                    CG_CompassDrawGridPoints(localClientNum, COMPASS_TYPE_FULL, &parentRect, &rect, color, font, scale, textStyle);
                break;
        }
    }
    else if ( ownerDraw == 311 )
    {
        CG_DrawHoldBreathZoomHint(localClientNum, &rect, font, scale, textStyle);
    }
    else
    {
        switch ( ownerDraw )
        {
            case 5:
                CG_DrawPlayerAmmoValue(localClientNum, &rect, font, scale, color, material, textStyle);
                break;
            case 20:
                CG_DrawPlayerStance(localClientNum, &rect, color, font, scale, textStyle);
                break;
            case 71:
                CG_DrawHoldBreathHint(localClientNum, &rect, font, scale, textStyle);
                break;
            case 72:
                CG_DrawCursorhint(localClientNum, &rect, font, scale, color, textStyle);
                break;
            case 80:
                CG_DrawMantleHint(localClientNum, &rect, font, scale, color, textStyle);
                break;
            case 81:
                CG_DrawPlayerWeaponName(localClientNum, &rect, font, scale, color, textStyle);
                break;
            case 82:
                CG_DrawPlayerWeaponNameBack(localClientNum, &rect, font, scale, color, material);
                break;
            case 90:
                CG_DrawCenterString(localClientNum, &rect, font, scale, color, textStyle);
                break;
            case 105:
                CG_DrawOffHandAmmo(localClientNum, &rect, font, scale, color, textStyle, OFFHAND_SLOT_LETHAL_GRENADE);
                break;
            case 106:
                CG_DrawOffHandAmmo(localClientNum, &rect, font, scale, color, textStyle, OFFHAND_SLOT_TACTICAL_GRENADE);
                break;
            case 107:
                CG_DrawOffHandName(localClientNum, &rect, font, scale, color, textStyle, OFFHAND_SLOT_LETHAL_GRENADE);
                break;
            case 108:
                CG_DrawOffHandName(localClientNum, &rect, font, scale, color, textStyle, OFFHAND_SLOT_TACTICAL_GRENADE);
                break;
            case 113:
                CG_DrawInvalidCmdHint(localClientNum, &rect, font, scale, color, textStyle);
                break;
            case 120:
                CG_DrawPlayerWeaponAmmoClip(
                    localClientNum,
                    &rect,
                    font,
                    scale,
                    color,
                    material,
                    textStyle,
                    text_x,
                    text_y,
                    textAlignMode);
                break;
            case 121:
                CG_DrawPlayerWeaponAmmoClipDualWield(localClientNum, &rect, font, scale, color, material, textStyle);
                break;
            case 122:
                CG_DrawPlayerWeaponAmmoStock(localClientNum, &rect, font, scale, color, material, textStyle);
                break;
            case 123:
                CG_DrawPlayerWeaponLowAmmoWarning(
                    localClientNum,
                    &rect,
                    font,
                    scale,
                    textStyle,
                    text_x,
                    text_y,
                    textAlignMode,
                    material);
                break;
            case 130:
                CG_DrawFuelTankPercentage(localClientNum, &rect, font, scale, color, material, textStyle, text_x, text_y);
                break;
            case 134:
                CG_DrawWarText(cgameGlob, &rect, font, scale, color, textStyle, text_x, text_y);
                break;
            case 144:
                CG_CompassDrawTickertape(
                    localClientNum,
                    COMPASS_TYPE_PARTIAL,
                    &parentRect,
                    &rect,
                    material,
                    color,
                    font,
                    scale,
                    textStyle,
                    1);
                break;
            case 145:
                CG_CompassDrawTickertape(
                    localClientNum,
                    COMPASS_TYPE_PARTIAL,
                    &parentRect,
                    &rect,
                    material,
                    color,
                    font,
                    scale,
                    textStyle,
                    0);
                break;
            case 160:
                CG_CompassDrawPlayerNorthCoord(
                    localClientNum,
                    COMPASS_TYPE_PARTIAL,
                    &parentRect,
                    &rect,
                    font,
                    material,
                    color,
                    textStyle);
                break;
            case 161:
                CG_CompassDrawPlayerEastCoord(
                    localClientNum,
                    COMPASS_TYPE_PARTIAL,
                    &parentRect,
                    &rect,
                    font,
                    material,
                    color,
                    textStyle);
                break;
            case 162:
                CG_CompassDrawPlayerNCoordScroll(
                    localClientNum,
                    COMPASS_TYPE_PARTIAL,
                    &parentRect,
                    &rect,
                    font,
                    material,
                    color,
                    textStyle);
                break;
            case 163:
                CG_CompassDrawPlayerECoordScroll(
                    localClientNum,
                    COMPASS_TYPE_PARTIAL,
                    &parentRect,
                    &rect,
                    font,
                    material,
                    color,
                    textStyle);
                break;
            case 166:
            case 167:
            case 168:
            case 169:
                CG_DrawPlayerActionSlot(localClientNum, &rect, ownerDraw - 166, color, font, scale, textStyle);
                break;
            case 195:
            case 196:
            case 197:
            case 198:
                CG_DrawTalkerNum(localClientNum, ownerDraw - 195, &rect, font, color, scale, textStyle);
                break;
            default:
                goto LABEL_59;
        }
    }
}

void __cdecl CG_DrawPlayerAmmoValue(
                int localClientNum,
                const rectDef_s *rect,
                Font_s *font,
                float scale,
                float *color,
                Material *material,
                int textStyle)
{
    float v7; // [esp+1Ch] [ebp-27Ch]
    float ammoColor[4]; // [esp+44h] [ebp-254h] BYREF
    int flashTime; // [esp+54h] [ebp-244h]
    const ScreenPlacement *scrPlace; // [esp+58h] [ebp-240h]
    int ammoVal; // [esp+5Ch] [ebp-23Ch]
    cg_s *cgameGlob; // [esp+60h] [ebp-238h]
    bool drawAmmo; // [esp+67h] [ebp-231h]
    char clipString[260]; // [esp+68h] [ebp-230h] BYREF
    const centity_s *cent; // [esp+16Ch] [ebp-12Ch]
    char ammoString[256]; // [esp+170h] [ebp-128h] BYREF
    bool lowAmmo; // [esp+275h] [ebp-23h]
    bool lowClip; // [esp+276h] [ebp-22h]
    bool drawClip; // [esp+277h] [ebp-21h]
    float flashColor[4]; // [esp+278h] [ebp-20h] BYREF
    int weapIndex; // [esp+288h] [ebp-10h]
    const playerState_s *ps; // [esp+28Ch] [ebp-Ch]
    float x; // [esp+290h] [ebp-8h]
    int clipVal; // [esp+294h] [ebp-4h]

    flashTime = 800;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->predictedPlayerState.weapon )
    {
        color[3] = CG_FadeHudMenu(
                                 localClientNum,
                                 hud_fade_ammodisplay,
                                 cgameGlob->ammoFadeTime,
                                 SnapFloatToInt((float)(hud_fade_ammodisplay->current.value * 1000.0)));
        if ( color[3] != 0.0 )
        {
            if ( !cgameGlob->nextSnap
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_newDraw_mp.cpp",
                            371,
                            0,
                            "%s",
                            "cgameGlob->nextSnap") )
            {
                __debugbreak();
            }
            cent = CG_GetEntity(localClientNum, cgameGlob->nextSnap->ps.clientNum);
            if ( (cgameGlob->nextSnap->ps.otherFlags & 4) != 0 && cgameGlob->weaponSelect < BG_GetNumWeapons() )
                weapIndex = cgameGlob->weaponSelect;
            else
                weapIndex = cent->nextState.weapon;
            if ( weapIndex )
            {
                drawAmmo = 1;
                drawClip = 1;
                lowAmmo = 0;
                lowClip = 0;
                ps = &cgameGlob->predictedPlayerState;
                ammoVal = BG_GetTotalAmmoReserve(&cgameGlob->predictedPlayerState, weapIndex);
                if ( BG_WeaponIsClipOnly(weapIndex) )
                    clipVal = -1;
                else
                    clipVal = BG_GetAmmoInClip(ps, weapIndex);
                if ( clipVal < 0 )
                    drawClip = 0;
                if ( clipVal > 999 )
                    clipVal = 999;
                if ( ammoVal < 0 )
                    drawAmmo = 0;
                if ( ammoVal > 999 )
                    ammoVal = 999;
                if ( drawClip )
                {
                    sprintf(clipString, "%2i", clipVal);
                    lowClip = CG_CheckPlayerForLowClip(cgameGlob);
                }
                if ( drawAmmo )
                {
                    sprintf(ammoString, "%3i", ammoVal);
                    lowAmmo = CG_CheckPlayerForLowAmmo(cgameGlob);
                }
                if ( lowClip )
                {
                    if ( cgameGlob->lastClipFlashTime > cgameGlob->time || cgameGlob->lastClipFlashTime + 800 < cgameGlob->time )
                        cgameGlob->lastClipFlashTime = cgameGlob->time;
                    flashColor[0] = 0.89f;
                    flashColor[1] = 0.18000001;
                    flashColor[2] = 0.01f;
                    flashColor[3] = (float)(cgameGlob->lastClipFlashTime + 800 - cgameGlob->time) / 800.0;
                    if ( flashColor[3] > color[3] )
                        flashColor[3] = color[3];
                }
                if ( lowAmmo )
                {
                    ammoColor[0] = 0.89f;
                    ammoColor[1] = 0.18000001;
                    ammoColor[2] = 0.01f;
                }
                else
                {
                    ammoColor[0] = *color;
                    ammoColor[1] = color[1];
                    ammoColor[2] = color[2];
                }
                ammoColor[3] = color[3];
                scrPlace = &scrPlaceView[localClientNum];
                if ( drawClip && drawAmmo )
                {
                    UI_DrawText(
                        scrPlace,
                        clipString,
                        0x7FFFFFFF,
                        font,
                        rect->x,
                        rect->y,
                        rect->horzAlign,
                        rect->vertAlign,
                        scale,
                        color,
                        textStyle);
                    if ( lowClip )
                        UI_DrawText(
                            scrPlace,
                            clipString,
                            0x7FFFFFFF,
                            font,
                            rect->x,
                            rect->y,
                            rect->horzAlign,
                            rect->vertAlign,
                            scale,
                            flashColor,
                            textStyle);
                    v7 = rect->x + rect->w;
                    x = v7 - (float)UI_TextWidth(ammoString, 0, font, scale);
                    UI_DrawText(
                        scrPlace,
                        ammoString,
                        0x7FFFFFFF,
                        font,
                        x,
                        rect->y,
                        rect->horzAlign,
                        rect->vertAlign,
                        scale,
                        ammoColor,
                        textStyle);
                    x = (float)((float)((float)(rect->w - (float)UI_TextWidth("|", 0, font, scale)) * 0.5) + rect->x) - 5.0;
                    UI_DrawText(
                        scrPlace,
                        (char*)"|",
                        0x7FFFFFFF,
                        font,
                        x,
                        rect->y,
                        rect->horzAlign,
                        rect->vertAlign,
                        scale,
                        ammoColor,
                        textStyle);
                }
                else if ( drawClip )
                {
                    x = (float)((float)(rect->w - (float)UI_TextWidth(clipString, 0, font, scale)) * 0.5) + rect->x;
                    UI_DrawText(
                        scrPlace,
                        clipString,
                        0x7FFFFFFF,
                        font,
                        x,
                        rect->y,
                        rect->horzAlign,
                        rect->vertAlign,
                        scale,
                        color,
                        textStyle);
                    if ( lowClip )
                        UI_DrawText(
                            scrPlace,
                            clipString,
                            0x7FFFFFFF,
                            font,
                            x,
                            rect->y,
                            rect->horzAlign,
                            rect->vertAlign,
                            scale,
                            flashColor,
                            textStyle);
                }
                else if ( drawAmmo )
                {
                    x = (float)((float)(rect->w - (float)UI_TextWidth(ammoString, 0, font, scale)) * 0.5) + rect->x;
                    UI_DrawText(
                        scrPlace,
                        ammoString,
                        0x7FFFFFFF,
                        font,
                        x,
                        rect->y,
                        rect->horzAlign,
                        rect->vertAlign,
                        scale,
                        ammoColor,
                        textStyle);
                }
            }
        }
    }
}

void __cdecl CG_DrawPlayerWeaponName(
                int localClientNum,
                const rectDef_s *rect,
                Font_s *font,
                float scale,
                float *color,
                int textStyle)
{
    float v6; // [esp+1Ch] [ebp-30h]
    weaponInfo_s *weapInfo; // [esp+20h] [ebp-2Ch]
    float *fadeColor; // [esp+24h] [ebp-28h]
    const cg_s *cgameGlob; // [esp+28h] [ebp-24h]
    char *string; // [esp+2Ch] [ebp-20h]
    float drawColor[4]; // [esp+30h] [ebp-1Ch] BYREF
    int weapIndex; // [esp+40h] [ebp-Ch]
    const WeaponDef *weapDef; // [esp+44h] [ebp-8h]
    float x; // [esp+48h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    fadeColor = CG_FadeColor(cgameGlob->time, cgameGlob->weaponSelectTime, 1800, 700);
    if ( fadeColor )
    {
        if ( (cgameGlob->predictedPlayerState.weapFlags & 0x80) == 0
            && (cgameGlob->predictedPlayerState.eFlags & 0x4000) == 0 )
        {
            drawColor[3] = fadeColor[3];
            drawColor[0] = *color;
            drawColor[1] = color[1];
            drawColor[2] = color[2];
            weapIndex = GetWeaponIndex(cgameGlob);
            if ( weapIndex )
            {
                weapInfo = CG_GetLocalClientWeaponInfo(localClientNum, weapIndex);
                weapDef = BG_GetWeaponDef(weapIndex);
                if ( *weapDef->szModeName )
                    string = va("%s / %s", weapInfo->translatedDisplayName, weapInfo->translatedModename);
                else
                    string = va("%s", weapInfo->translatedDisplayName);
                v6 = rect->x + rect->w;
                x = (float)(v6 - (float)UI_TextWidth(string, 0, font, scale)) - 28.0;
                UI_DrawText(
                    &scrPlaceView[localClientNum],
                    string,
                    0x7FFFFFFF,
                    font,
                    x,
                    rect->y,
                    rect->horzAlign,
                    rect->vertAlign,
                    scale,
                    drawColor,
                    textStyle);
            }
        }
    }
}

void __cdecl CG_DrawPlayerWeaponNameBack(
                int localClientNum,
                const rectDef_s *rect,
                Font_s *font,
                float scale,
                const float *color,
                Material *material)
{
    const weaponInfo_s *weapInfo; // [esp+20h] [ebp-2Ch]
    const cg_s *cgameGlob; // [esp+24h] [ebp-28h]
    char *string; // [esp+28h] [ebp-24h]
    float drawColor[4]; // [esp+2Ch] [ebp-20h] BYREF
    int weapIndex; // [esp+3Ch] [ebp-10h]
    const WeaponDef *weapDef; // [esp+40h] [ebp-Ch]
    float w; // [esp+48h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    drawColor[3] = CG_FadeHudMenu(localClientNum, hud_fade_ammodisplay, cgameGlob->weaponSelectTime, 1800);
    if ( drawColor[3] != 0.0 )
    {
        drawColor[0] = *color;
        drawColor[1] = color[1];
        drawColor[2] = color[2];
        weapIndex = GetWeaponIndex(cgameGlob);
        if ( weapIndex )
        {
            weapInfo = CG_GetLocalClientWeaponInfo(localClientNum, weapIndex);
            weapDef = BG_GetWeaponDef(weapIndex);
            if ( *weapDef->szModeName )
                string = va("%s / %s", weapInfo->translatedDisplayName, weapInfo->translatedModename);
            else
                string = va("%s", weapInfo->translatedDisplayName);
            w = (float)((float)UI_TextWidth(string, 0, font, scale) + 28.0) + 8.0;
            UI_DrawHandlePic(
                &scrPlaceView[localClientNum],
                (float)(rect->x + rect->w) - w,
                rect->y,
                w,
                rect->h,
                rect->horzAlign,
                rect->vertAlign,
                drawColor,
                material);
        }
    }
}

void    CG_DrawPlayerStance(
                int localClientNum,
                const rectDef_s *rect,
                const float *color,
                Font_s *font,
                float scale,
                int textStyle)
{
    double v7; // xmm0_8
    long double v8; // [esp+1Ch] [ebp-5Ch]
    float v9; // [esp+2Ch] [ebp-4Ch]
    const char *v10; // [esp+30h] [ebp-48h]
    float stanceColor[4];
    float drawColor[4]; // [esp+40h] [ebp-38h] BYREF
    float y; // [esp+50h] [ebp-28h]
    const cgs_t *x; // [esp+54h] [ebp-24h]
    const cgs_t *cgs; // [esp+58h] [ebp-20h]
    double fadeAlpha; // [esp+5Ch] [ebp-1Ch]
    float v19; // [esp+64h] [ebp-14h]
    cg_s *cgameGlob; // [esp+68h] [ebp-10h]
    //int v21; // [esp+6Ch] [ebp-Ch]
    //void *v22; // [esp+70h] [ebp-8h]
    //void *retaddr; // [esp+78h] [ebp+0h]
    //
    //v21 = a1;
    //v22 = retaddr;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    v19 = hud_fade_stance->current.value * 1000.0;
    //fadeAlpha = DOUBLE_9_313225746154785eN10;
    fadeAlpha = 9.313225746154785e-10;
    cgs = (const cgs_t *)SnapFloatToInt(v19);
    x = cgs;
    y = CG_FadeHudMenu(localClientNum, hud_fade_stance, cgameGlob->stanceFadeTime, (int)cgs);
    if ( y != 0.0 )
    {
        if ( cg_hudStanceHintPrints->current.enabled )
        {
            if ( cgameGlob->lastStance != (cgameGlob->predictedPlayerState.pm_flags & 3) )
                cgameGlob->lastStanceChangeTime = cgameGlob->time;
        }
        else
        {
            cgameGlob->lastStanceChangeTime = 0;
        }
        cgameGlob->lastStance = cgameGlob->predictedPlayerState.pm_flags & 3;
        //LODWORD(drawColor[3]) = CG_GetLocalClientStaticGlobals(localClientNum);
        drawColor[2] = (float)((float)((float)(compassSize->current.value - 1.0) * CG_GetLocalClientStaticGlobals(localClientNum)->compassWidth)
                                                 * 0.69999999)
                                 + rect->x;
        drawColor[1] = rect->y;
        //BLOPS_NULLSUB((jpeg_decompress_struct *)&drawColor[2]);
        stanceColor[0] = color[0];
        stanceColor[1] = color[1];
        stanceColor[2] = color[2];
        if ( (cgameGlob->predictedPlayerState.pm_flags & 0x1000) != 0 && cgameGlob->proneBlockedEndTime < cgameGlob->time )
            cgameGlob->proneBlockedEndTime = cgameGlob->time + 1500;
        if ( cgameGlob->proneBlockedEndTime > cgameGlob->time )
        {
            if ( BG_WeaponBlocksProne(cgameGlob->predictedPlayerState.weapon) )
                v10 = UI_SafeTranslateString("CGAME_PRONE_BLOCKED_WEAPON");
            else
                v10 = UI_SafeTranslateString("CGAME_PRONE_BLOCKED");
            v9 = (float)UI_TextWidth(v10, 0, font, scale) * 0.5;
            v7 = (float)(DEG2RAD((float)((float)((float)(cgameGlob->proneBlockedEndTime - cgameGlob->time) / 1500.0) * 540.0)));
            //__libm_sse2_sin(v8);
            //*(float *)&v7 = v7;
            stanceColor[3] = fabs(sin(v7));
            UI_DrawText(
                &scrPlaceView[localClientNum],
                (char*)v10,
                0x7FFFFFFF,
                font,
                (-(v9)),
                cg_hudProneY->current.value,
                7,
                3,
                scale,
                stanceColor,
                textStyle);
        }
        if ( cg_hudStanceHintPrints->current.enabled && cgameGlob->lastStanceChangeTime + 3000 > cgameGlob->time )
            CG_DrawStanceHintPrints(localClientNum, rect, drawColor[2], color, y, font, scale, textStyle);
        stanceColor[3] = color[3] * y;
        CG_DrawStanceIcon(localClientNum, rect, stanceColor, drawColor[2], drawColor[1], y);
    }
}

void __cdecl CG_DrawStanceIcon(
                int localClientNum,
                const rectDef_s *rect,
                float *drawColor,
                float x,
                float y,
                float fadeAlpha)
{
    Material *icon; // [esp+20h] [ebp-10h]
    const cg_s *cgameGlob; // [esp+24h] [ebp-Ch]
    float width; // [esp+28h] [ebp-8h]
    float height; // [esp+2Ch] [ebp-4h]

    if ( !rect
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_newDraw_mp.cpp", 836, 0, "%s", "rect") )
    {
        __debugbreak();
    }
    width = rect->w;
    height = rect->h;
    //BLOPS_NULLSUB();
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( (cgameGlob->lastStance & 1) != 0 )
    {
        icon = cgMedia.stanceMaterials[2];
    }
    else if ( (cgameGlob->lastStance & 2) != 0 )
    {
        icon = cgMedia.stanceMaterials[1];
    }
    else if ( cgameGlob->predictedPlayerState.waterlevel < 3 )
    {
        icon = cgMedia.stanceMaterials[0];
    }
    else
    {
        icon = cgMedia.stanceMaterials[4];
    }
    UI_DrawHandlePic(
        &scrPlaceView[localClientNum],
        x,
        y,
        width,
        height,
        rect->horzAlign,
        rect->vertAlign,
        drawColor,
        icon);
    if ( cgameGlob->lastStanceChangeTime + 1000 > cgameGlob->time )
    {
        Dvar_GetUnpackedColor(cg_hudStanceFlash, drawColor);
        drawColor[3] = (float)((float)(cgameGlob->lastStanceChangeTime + 1000 - cgameGlob->time) / 1000.0) * 0.80000001;
        if ( drawColor[3] > fadeAlpha )
            drawColor[3] = fadeAlpha;
        UI_DrawHandlePic(
            &scrPlaceView[localClientNum],
            x,
            y,
            width,
            height,
            rect->horzAlign,
            rect->vertAlign,
            drawColor,
            cgMedia.stanceMaterials[3]);
    }
}

void __cdecl CG_DrawStanceHintPrints(
                int localClientNum,
                const rectDef_s *rect,
                float x,
                const float *color,
                float fadeAlpha,
                Font_s *font,
                float scale,
                int textStyle)
{
    char keyBinding[256]; // [esp+20h] [ebp-230h] BYREF
    int j; // [esp+124h] [ebp-12Ch]
    const cg_s *cgameGlob; // [esp+128h] [ebp-128h]
    const char *string; // [esp+12Ch] [ebp-124h]
    const char *proneCmds[3][6]; // [esp+130h] [ebp-120h] BYREF
    float height; // [esp+17Ch] [ebp-D4h]
    float drawColor[4]; // [esp+180h] [ebp-D0h] BYREF
    const char *duckCmds[3][6]; // [esp+190h] [ebp-C0h] BYREF
    const char *hintLineCmds[3]; // [esp+1DCh] [ebp-74h]
    const char *standCmds[3][6]; // [esp+1E8h] [ebp-68h] BYREF
    const char *hintTypeStrings[3]; // [esp+234h] [ebp-1Ch]
    int numHintLines; // [esp+240h] [ebp-10h]
    int i; // [esp+244h] [ebp-Ch]
    const char *binding; // [esp+248h] [ebp-8h]
    float y; // [esp+24Ch] [ebp-4h]

    memset(standCmds, 0, 24);
    standCmds[1][0] = "gocrouch";
    standCmds[1][1] = "togglecrouch";
    standCmds[1][2] = "lowerstance";
    standCmds[1][3] = "+movedown";
    *(_QWORD *)&standCmds[1][4] = 0;
    standCmds[2][0] = "goprone";
    standCmds[2][1] = "+prone";
    *(_QWORD *)&standCmds[2][2] = 0;
    *(_QWORD *)&standCmds[2][4] = 0;
    duckCmds[0][0] = "+gostand";
    duckCmds[0][1] = "raisestance";
    duckCmds[0][2] = "+moveup";
    *(_QWORD *)&duckCmds[0][3] = 0;
    duckCmds[0][5] = 0;
    memset(duckCmds[1], 0, sizeof(const char *[6]));
    duckCmds[2][0] = "goprone";
    duckCmds[2][1] = "lowerstance";
    duckCmds[2][2] = "toggleprone";
    duckCmds[2][3] = "+prone";
    *(_QWORD *)&duckCmds[2][4] = 0;
    proneCmds[0][0] = "+gostand";
    proneCmds[0][1] = "toggleprone";
    *(_QWORD *)&proneCmds[0][2] = 0;
    *(_QWORD *)&proneCmds[0][4] = 0;
    proneCmds[1][0] = "gocrouch";
    proneCmds[1][1] = "togglecrouch";
    proneCmds[1][2] = "raisestance";
    proneCmds[1][3] = "+movedown";
    proneCmds[1][4] = "+moveup";
    proneCmds[1][5] = 0;
    memset(proneCmds[2], 0, sizeof(const char *[6]));
    hintTypeStrings[0] = "PLATFORM_STANCEHINT_STAND";
    hintTypeStrings[1] = "PLATFORM_STANCEHINT_CROUCH";
    hintTypeStrings[2] = "PLATFORM_STANCEHINT_PRONE";
    drawColor[0] = *color;
    drawColor[1] = color[1];
    drawColor[2] = color[2];
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->lastStanceChangeTime + 3000 - cgameGlob->time <= 1000 )
        drawColor[3] = (float)(cgameGlob->lastStanceChangeTime + 3000 - cgameGlob->time) * 0.001;
    else
        drawColor[3] = 1.0f;
    height = (float)UI_TextHeight(font, scale);
    numHintLines = 0;
    for ( i = 0; i < 3; ++i )
    {
        hintLineCmds[i] = 0;
        j = 0;
        if ( (cgameGlob->lastStance & 1) != 0 )
        {
            binding = proneCmds[i][j];
        }
        else if ( (cgameGlob->lastStance & 2) != 0 )
        {
            binding = duckCmds[i][j];
        }
        else
        {
            binding = standCmds[i][j];
        }
        while ( j < 6 && binding )
        {
            if ( Key_IsCommandBound(localClientNum, binding, 0) )
            {
                hintLineCmds[i] = binding;
                ++numHintLines;
                break;
            }
            ++j;
            if ( (cgameGlob->lastStance & 1) != 0 )
            {
                binding = proneCmds[i][j];
            }
            else if ( (cgameGlob->lastStance & 2) != 0 )
            {
                binding = duckCmds[i][j];
            }
            else
            {
                binding = standCmds[i][j];
            }
        }
    }
    y = (float)((float)(rect->h * 0.5) + rect->y) - 1.5;
    if ( numHintLines == 1 )
    {
        y = (float)(height * 0.5) + y;
    }
    else if ( numHintLines == 3 )
    {
        y = y - (float)((float)(height * 0.5) + 1.5);
    }
    if ( drawColor[3] > fadeAlpha )
        drawColor[3] = fadeAlpha;
    for ( i = 0; i < 3; ++i )
    {
        if ( hintLineCmds[i] )
        {
            UI_GetKeyBindingLocalizedString(localClientNum, hintLineCmds[i], keyBinding, 0);
            string = UI_SafeTranslateString(hintTypeStrings[i]);
            string = UI_ReplaceConversionString((char*)string, keyBinding);
            UI_DrawText(
                &scrPlaceView[localClientNum],
                (char *)string,
                0x7FFFFFFF,
                font,
                x + rect->w,
                y,
                rect->horzAlign,
                rect->vertAlign,
                scale,
                drawColor,
                textStyle);
            y = (float)(height + 1.5) + y;
        }
    }
}

void    CG_DrawCursorhint(
                int localClientNum,
                const rectDef_s *rect,
                Font_s *font,
                float fontscale,
                float *color,
                int textStyle)
{
    char *v7; // eax
    float v8; // [esp+4h] [ebp-198h]
    float w; // [esp+8h] [ebp-194h]
    int h; // [esp+Ch] [ebp-190h]
    int vertAlign; // [esp+10h] [ebp-18Ch]
    long double v12; // [esp+20h] [ebp-17Ch]
    float iconWidth; // [esp+2Ch] [ebp-170h]
    float secondaryLength; // [esp+30h] [ebp-16Ch]
    ScreenPlacement *scrPlace; // [esp+34h] [ebp-168h]
    char binding[256]; // [esp+38h] [ebp-164h] BYREF
    weaponInfo_s *weapInfo; // [esp+138h] [ebp-64h]
    weaponIconRatioType_t hudIconRatio; // [esp+13Ch] [ebp-60h]
    int weaponIndex; // [esp+140h] [ebp-5Ch]
    Material *hintIcon; // [esp+144h] [ebp-58h]
    const playerState_s *ps; // [esp+148h] [ebp-54h]
    float y; // [esp+14Ch] [ebp-50h]
    float x; // [esp+150h] [ebp-4Ch]
    float height; // [esp+154h] [ebp-48h]
    float length; // [esp+158h] [ebp-44h]
    //float v26; // [esp+15Ch] [ebp-40h]
    float scale; // [esp+160h] [ebp-3Ch]
    float halfscale; // [esp+164h] [ebp-38h]
    float v29; // [esp+168h] [ebp-34h]
    const char *secondaryString; // [esp+170h] [ebp-2Ch] BYREF
    const char *displayString; // [esp+174h] [ebp-28h]
    float heightScale; // [esp+178h] [ebp-24h]
    float widthOfs; // [esp+17Ch] [ebp-20h]
    float widthScale; // [esp+180h] [ebp-1Ch]
    cg_s *cgameGlob; // [esp+184h] [ebp-18h]
    const WeaponDef *weapDef; // [esp+188h] [ebp-14h]
    //_UNKNOWN *v38; // [esp+190h] [ebp-Ch]
    //int localClientNuma; // [esp+194h] [ebp-8h]
    //const rectDef_s *recta; // [esp+198h] [ebp-4h] BYREF
    //Font_s *fonta; // [esp+19Ch] [ebp+0h]
    //
    //v38 = a1;
    //localClientNuma = (int)fonta;
    weapDef = 0;
    if (cg_cursorHints->current.integer)
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        CG_UpdateCursorHints(cgameGlob);
        if (!IsHardcoreMode(localClientNum) || cgameGlob->cursorHintIcon == 3)
        {
            *((float *)color + 3) = CG_FadeAlpha(cgameGlob->time, cgameGlob->cursorHintTime, cgameGlob->cursorHintFade, 100)
                * color[3];
            if (color[3] == 0.0)
            {
                cgameGlob->cursorHintIcon = 0;
                return;
            }
            widthScale = 1.0f;
            widthOfs = 0.0f;
            heightScale = 1.0f;
            displayString = 0;
            secondaryString = 0;
            if (cg_cursorHints->current.integer == 3)
            {
                //v30 = (float)cgameGlob->time / 150.0;
                //__libm_sse2_sin(v12);
                v29 = sin((float)cgameGlob->time / 150.0);// v30;
                *((float *)color + 3) = (float)((float)(0.5 * v29) + 0.5) * color[3];
            }
            if (cg_cursorHints->current.integer < 3)
            {
                if (cg_cursorHints->current.integer == 2)
                {
                    scale = (float)(cgameGlob->cursorHintTime % 1000) / 100.0;
                }
                else
                {
                    //v26 = (float)cgameGlob->time / 150.0;
                    //__libm_sse2_sin(v12);
                    scale = (float)((float)(sin((float)cgameGlob->time / 150.0) * 0.5) + 0.5) * 10.0;
                }
                halfscale = scale * 0.5;
            }
            else
            {
                halfscale = 0.0f;
                scale = 0.0f;
            }
            if (cgameGlob->cursorHintIcon == 1 || cgameGlob->cursorHintIcon == 2)
            {
                if (cgameGlob->cursorHintString < 0)
                    return;
                displayString = CG_GetUseString(localClientNum);
                if (!displayString || !*displayString)
                    return;
            LABEL_20:
                length = (float)UI_TextWidth(displayString, 0, font, fontscale);
                height = (float)UI_TextHeight(font, fontscale);
                x = (float)(scale + length) * -0.5;
                y = rect->y - (float)(rect->h * 0.5);
                UI_DrawText(
                    &scrPlaceView[localClientNum],
                    (char *)displayString,
                    0x7FFFFFFF,
                    font,
                    x,
                    (float)(height * 0.5) + rect->y,
                    rect->horzAlign,
                    rect->vertAlign,
                    fontscale,
                    color,
                    textStyle);
                return;
            }
            if (cgameGlob->cursorHintIcon == 6)
            {
                displayString = UI_SafeTranslateString("CGAME_SPECTATOR");
                if (displayString && *displayString)
                    goto LABEL_20;
            }
            else
            {
                ps = &cgameGlob->nextSnap->ps;
                if ((ps->otherFlags & 0x18) == 0)
                {
                    hintIcon = cgMedia.hintMaterials[cgameGlob->cursorHintIcon];
                    if (hintIcon || cgameGlob->cursorHintIcon == 3)
                    {
                        if (cgameGlob->cursorHintIcon < 8 || cgameGlob->cursorHintIcon > 2055)
                        {
                            if (cgameGlob->cursorHintString < 0)
                            {
                                if (cgameGlob->cursorHintIcon == 4)
                                {
                                    UI_GetKeyBindingLocalizedString(localClientNum, "+activate", binding, 0);
                                    displayString = UI_SafeTranslateString("PLATFORM_PICKUPHEALTH");
                                    displayString = UI_ReplaceConversionString((char *)displayString, binding);
                                }
                            }
                            else
                            {
                                displayString = CG_GetUseString(localClientNum);
                            }
                        }
                        else
                        {
                            weaponIndex = cgameGlob->cursorHintIcon - 7;
                            weapDef = BG_GetWeaponDef(weaponIndex);
                            if (weapDef->hudIcon)
                            {
                                hudIconRatio = weapDef->hudIconRatio;
                                if (hudIconRatio)
                                {
                                    if (hudIconRatio == WEAPON_ICON_RATIO_2TO1)
                                    {
                                        widthScale = 2.0f;
                                        widthOfs = rect->w * -0.5;
                                        heightScale = 1.0f;
                                    }
                                    else
                                    {
                                        if (weapDef->hudIconRatio != WEAPON_ICON_RATIO_4TO1
                                            && !Assert_MyHandler(
                                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_newDraw_mp.cpp",
                                                1806,
                                                0,
                                                "%s",
                                                "weapDef->hudIconRatio == WEAPON_ICON_RATIO_4TO1"))
                                        {
                                            __debugbreak();
                                        }
                                        widthScale = 2.0f;
                                        widthOfs = rect->w * -0.5;
                                        heightScale = 0.5f;
                                    }
                                }
                            }
                            if (weapDef->weapClass == WEAPCLASS_TURRET)
                            {
                                if (cgameGlob->cursorHintString >= 0)
                                    displayString = CG_GetUseString(localClientNum);
                                weapInfo = CG_GetLocalClientWeaponInfo(localClientNum, weaponIndex);
                                secondaryString = weapInfo->translatedDisplayName;
                            }
                            else
                            {
                                displayString = CG_GetWeaponUseString(localClientNum, &secondaryString);
                            }
                        }
                        scrPlace = &scrPlaceView[localClientNum];
                        if (cgameGlob->cursorHintIcon == 3)
                        {
                            length = (float)UI_TextWidth(displayString, 0, font, fontscale);
                            height = (float)UI_TextHeight(font, fontscale);
                            x = (float)((float)((float)(rect->w + scale) + length) * -0.5) - rect->x;
                            y = rect->y - (float)((float)(rect->h * 0.5) * heightScale);
                            UI_DrawText(
                                scrPlace,
                                (char *)displayString,
                                0x7FFFFFFF,
                                font,
                                x,
                                (float)(height * 0.5) + rect->y,
                                rect->horzAlign,
                                rect->vertAlign,
                                fontscale,
                                color,
                                textStyle);
                        }
                        else if (displayString && *displayString)
                        {
                            length = (float)UI_TextWidth(displayString, 0, font, fontscale);
                            height = (float)UI_TextHeight(font, fontscale);
                            if (secondaryString && cg_weaponHintsCoD1Style->current.enabled)
                            {
                                secondaryLength = (float)UI_TextWidth(secondaryString, 0, font, fontscale);
                                x = (float)(length + secondaryLength) * -0.5;
                                y = rect->y - (float)((float)(rect->h * 0.5) * heightScale);
                                UI_DrawText(
                                    scrPlace,
                                    (char *)displayString,
                                    0x7FFFFFFF,
                                    font,
                                    x,
                                    (float)(height * 0.5) + rect->y,
                                    rect->horzAlign,
                                    rect->vertAlign,
                                    fontscale,
                                    color,
                                    textStyle);
                                vertAlign = rect->vertAlign;
                                h = rect->horzAlign;
                                w = (float)(height * 0.5) + rect->y;
                                v8 = x + length;
                                v7 = va(" %s", secondaryString);
                                UI_DrawText(scrPlace, v7, 0x7FFFFFFF, font, v8, w, h, vertAlign, fontscale, color, textStyle);
                                x = (float)((float)(rect->w * widthScale) + scale) * -0.5;
                                if (weapDef && weapDef->bDualWield)
                                {
                                    iconWidth = (float)(rect->w * widthScale) + scale;
                                    UI_DrawHandlePic(
                                        scrPlace,
                                        x - (float)(iconWidth / 2.0),
                                        (float)(height * 2.0) + y,
                                        iconWidth,
                                        (float)(rect->h * heightScale) + scale,
                                        rect->horzAlign,
                                        rect->vertAlign,
                                        color,
                                        hintIcon);
                                    UI_DrawHandlePic(
                                        scrPlace,
                                        (float)(iconWidth / 2.0) + x,
                                        (float)(height * 2.0) + y,
                                        (-(iconWidth)),
                                        (float)(rect->h * heightScale) + scale,
                                        rect->horzAlign,
                                        rect->vertAlign,
                                        color,
                                        hintIcon);
                                }
                                else
                                {
                                    UI_DrawHandlePic(
                                        scrPlace,
                                        x,
                                        (float)(height * 2.0) + y,
                                        (float)(rect->w * widthScale) + scale,
                                        (float)(rect->h * heightScale) + scale,
                                        rect->horzAlign,
                                        rect->vertAlign,
                                        color,
                                        hintIcon);
                                }
                            }
                            else
                            {
                                x = (float)((float)((float)(rect->w * widthScale) + scale) + length) * -0.5;
                                y = rect->y - (float)((float)(rect->h * 0.5) * heightScale);
                                UI_DrawText(
                                    scrPlace,
                                    (char *)displayString,
                                    0x7FFFFFFF,
                                    font,
                                    x,
                                    (float)(height * 0.5) + rect->y,
                                    rect->horzAlign,
                                    rect->vertAlign,
                                    fontscale,
                                    color,
                                    textStyle);
                                UI_DrawHandlePic(
                                    scrPlace,
                                    x + length,
                                    y,
                                    (float)(rect->w * widthScale) + scale,
                                    (float)(rect->h * heightScale) + scale,
                                    rect->horzAlign,
                                    rect->vertAlign,
                                    color,
                                    hintIcon);
                            }
                        }
                        else
                        {
                            x = rect->x - (float)((float)((float)(rect->w + halfscale) + widthOfs) * 0.5);
                            y = rect->y - (float)(halfscale * heightScale);
                            UI_DrawHandlePic(
                                scrPlace,
                                x,
                                y,
                                (float)(rect->w * widthScale) + scale,
                                (float)(rect->h * heightScale) + scale,
                                rect->horzAlign,
                                rect->vertAlign,
                                color,
                                hintIcon);
                        }
                    }
                }
            }
        }
    }
}

void __cdecl CG_UpdateCursorHints(cg_s *cgameGlob)
{
    if ( cgameGlob->renderingThirdPerson != TP_FOR_MODEL )
    {
        if ( cgameGlob->predictedPlayerState.cursorHint )
        {
            cgameGlob->cursorHintTime = cgameGlob->time;
            cgameGlob->cursorHintFade = cg_hintFadeTime->current.integer;
            cgameGlob->cursorHintIcon = cgameGlob->predictedPlayerState.cursorHint;
            cgameGlob->cursorHintString = cgameGlob->predictedPlayerState.cursorHintString;
        }
        if ( cgameGlob->predictedPlayerState.cursorHint == 2 )
            cgameGlob->cursorHintFade = cg_seatHintFadeTime->current.integer;
    }
}

const char *__cdecl CG_GetWeaponUseString(int localClientNum, const char **secondaryString)
{
    const weaponInfo_s *weapInfo; // [esp+0h] [ebp-120h]
    const cg_s *cgameGlob; // [esp+4h] [ebp-11Ch]
    int weaponIndex; // [esp+8h] [ebp-118h]
    const char *displayString; // [esp+Ch] [ebp-114h]
    char binding[260]; // [esp+10h] [ebp-110h] BYREF
    const WeaponDef *weapDef; // [esp+118h] [ebp-8h]
    const playerState_s *ps; // [esp+11Ch] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( (cgameGlob->cursorHintIcon < 8 || cgameGlob->cursorHintIcon > 2055)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_newDraw_mp.cpp",
                    1595,
                    0,
                    "%s",
                    "(cgameGlob->cursorHintIcon >= FIRST_WEAPON_HINT) && (cgameGlob->cursorHintIcon <= LAST_WEAPON_HINT)") )
    {
        __debugbreak();
    }
    weaponIndex = cgameGlob->cursorHintIcon - 7;
    ps = &cgameGlob->predictedPlayerState;
    weapDef = BG_GetWeaponDef(weaponIndex);
    weapInfo = CG_GetLocalClientWeaponInfo(localClientNum, weaponIndex);
    if ( weapDef->inventoryType )
    {
        if ( weapDef->offhandClass == OFFHAND_CLASS_FRAG_GRENADE )
        {
            displayString = UI_SafeTranslateString("PLATFORM_THROWBACKGRENADE");
            UI_GetKeyBindingLocalizedString(localClientNum, "+frag", binding, 0);
        }
        else
        {
            displayString = UI_SafeTranslateString("PLATFORM_PICKUPNEWWEAPON");
            UI_GetKeyBindingLocalizedString(localClientNum, "+activate", binding, 0);
            *secondaryString = weapInfo->translatedDisplayName;
        }
    }
    else
    {
        UI_GetKeyBindingLocalizedString(localClientNum, "+activate", binding, 0);
        if ( BG_PlayerWeaponCountPrimaryTypes(ps) >= 2 )
            displayString = UI_SafeTranslateString("PLATFORM_SWAPWEAPONS");
        else
            displayString = UI_SafeTranslateString("PLATFORM_PICKUPNEWWEAPON");
        *secondaryString = weapInfo->translatedDisplayName;
    }
    return UI_ReplaceConversionString((char*)displayString, binding);
}

const char *__cdecl CG_GetUseString(int localClientNum)
{
    const char *v2; // eax
    const cg_s *cgameGlob; // [esp+0h] [ebp-510h]
    const char *displayString; // [esp+4h] [ebp-50Ch]
    char *displayStringa; // [esp+4h] [ebp-50Ch]
    char binding[256]; // [esp+8h] [ebp-508h] BYREF
    char textWithBinding[1028]; // [esp+108h] [ebp-408h] BYREF

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->cursorHintString < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_newDraw_mp.cpp",
                    1646,
                    0,
                    "%s",
                    "cgameGlob->cursorHintString >= 0") )
    {
        __debugbreak();
    }
    displayString = CL_GetConfigString(cgameGlob->cursorHintString + 419);
    if ( !displayString || !*displayString )
        return 0;
    if ( !UI_GetKeyBindingLocalizedString(localClientNum, "+activate", binding, 0) )
    {
        v2 = UI_SafeTranslateString("KEY_USE");
        I_strncpyz(binding, v2, 256);
    }
    displayStringa = (char *)SEH_LocalizeTextMessage(displayString, "Hint String", LOCMSG_SAFE);
    if ( SEH_ReplaceDirectiveInStringWithBinding(localClientNum, displayStringa, textWithBinding) )
        displayStringa = textWithBinding;
    return UI_ReplaceConversionString(displayStringa, binding);
}

void __cdecl CG_DrawHoldBreathHint(
                int localClientNum,
                const rectDef_s *rect,
                Font_s *font,
                float fontscale,
                int textStyle)
{
    cg_s *cgameGlob; // eax
    unsigned int ViewmodelWeaponIndex; // eax
    const char *v7; // eax
    const char *string; // [esp+30h] [ebp-114h]
    char binding[256]; // [esp+34h] [ebp-110h] BYREF
    const playerState_s *ps; // [esp+138h] [ebp-Ch]
    const WeaponDef *weapDef; // [esp+13Ch] [ebp-8h]
    float x; // [esp+140h] [ebp-4h]

    if ( cg_drawBreathHint->current.enabled )
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        ps = &cgameGlob->predictedPlayerState;
        if ( (cgameGlob->predictedPlayerState.weapFlags & 4) == 0 && (ps->eFlags & 0x4300) == 0 )
        {
            ViewmodelWeaponIndex = BG_GetViewmodelWeaponIndex(ps);
            weapDef = BG_GetWeaponDef(ViewmodelWeaponIndex);
            if ( weapDef->overlayReticle )
            {
                if ( weapDef->weapClass != WEAPCLASS_ITEM && ps->fWeaponPosFrac == 1.0 )
                {
                    if ( !UI_GetKeyBindingLocalizedString(localClientNum, "+holdbreath", binding, 0)
                        && !UI_GetKeyBindingLocalizedString(localClientNum, "+melee_breath", binding, 0) )
                    {
                        UI_GetKeyBindingLocalizedString(localClientNum, "+breath_sprint", binding, 0);
                    }
                    v7 = UI_SafeTranslateString("PLATFORM_HOLD_BREATH");
                    string = UI_ReplaceConversionString((char*)v7, binding);
                    x = rect->x
                        - (float)SnapFloatToInt((float)((float)UI_TextWidth(string, 0, font, fontscale) * 0.5));
                    UI_DrawText(
                        &scrPlaceView[localClientNum],
                        (char *)string,
                        0x7FFFFFFF,
                        font,
                        x,
                        rect->y,
                        rect->horzAlign,
                        rect->vertAlign,
                        fontscale,
                        colorWhite,
                        textStyle);
                }
            }
        }
    }
}

void __cdecl CG_DrawHoldBreathZoomHint(
                int localClientNum,
                const rectDef_s *rect,
                Font_s *font,
                float fontscale,
                int textStyle)
{
    cg_s *LocalClientGlobals; // eax
    char *v6; // eax
    char *string; // [esp+30h] [ebp-11Ch]
    const WeaponVariantDef *weapVariantDef; // [esp+34h] [ebp-118h]
    int weaponIndex; // [esp+38h] [ebp-114h]
    char binding[256]; // [esp+3Ch] [ebp-110h] BYREF
    const playerState_s *ps; // [esp+140h] [ebp-Ch]
    const WeaponDef *weapDef; // [esp+144h] [ebp-8h]
    float x; // [esp+148h] [ebp-4h]

    if ( cg_drawBreathHint->current.enabled )
    {
        LocalClientGlobals = CG_GetLocalClientGlobals(localClientNum);
        ps = &LocalClientGlobals->predictedPlayerState;
        if ( (LocalClientGlobals->predictedPlayerState.weapFlags & 4) == 0 && (ps->eFlags & 0x4300) == 0 )
        {
            weaponIndex = BG_GetViewmodelWeaponIndex(ps);
            weapDef = BG_GetWeaponDef(weaponIndex);
            if ( weapDef->overlayReticle )
            {
                if ( weapDef->weapClass != WEAPCLASS_ITEM && ps->fWeaponPosFrac == 1.0 )
                {
                    weapVariantDef = BG_GetWeaponVariantDef(weaponIndex);
                    if ( weapVariantDef->fAdsZoomFov1 != weapVariantDef->fAdsZoomFov2
                        || weapVariantDef->fAdsZoomFov2 != weapVariantDef->fAdsZoomFov3 )
                    {
                        UI_GetKeyBindingLocalizedString(localClientNum, "+melee", binding, 0);
                        v6 = UI_SafeTranslateString("PLATFORM_HOLD_BREATH_ZOOM");
                        string = UI_ReplaceConversionString(v6, binding);
                        x = rect->x
                            - (float)SnapFloatToInt((float)((float)UI_TextWidth(string, 0, font, fontscale) * 0.5));
                        UI_DrawText(
                            &scrPlaceView[localClientNum],
                            string,
                            0x7FFFFFFF,
                            font,
                            x,
                            rect->y,
                            rect->horzAlign,
                            rect->vertAlign,
                            fontscale,
                            colorWhite,
                            textStyle);
                    }
                }
            }
        }
    }
}

void __cdecl CG_DrawRCBombHints(
                int localClientNum,
                const rectDef_s *rect,
                Font_s *font,
                float fontscale,
                int textStyle,
                int hintType)
{
    const vehicle_info_t *info; // [esp+3Ch] [ebp-12Ch]
    cg_s *cgameGlob; // [esp+40h] [ebp-128h]
    char *string; // [esp+44h] [ebp-124h]
    centity_s *vehicle; // [esp+48h] [ebp-120h]
    bool useBinding; // [esp+4Fh] [ebp-119h]
    char binding[256]; // [esp+50h] [ebp-118h] BYREF
    const playerState_s *ps; // [esp+154h] [ebp-14h]
    float color[4]; // [esp+158h] [ebp-10h] BYREF

    if ( cg_drawBreathHint->current.enabled )
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        ps = &cgameGlob->predictedPlayerState;
        if ( (cgameGlob->predictedPlayerState.eFlags & 0x4000) != 0 )
        {
            color[0] = colorWhite[0];
            color[1] = 1.0;
            color[2] = 1.0;
            color[3] = CG_FadeHudMenu(
                                     localClientNum,
                                     hud_fade_vehiclecontrols,
                                     cgameGlob->vehicleControlsFadeTime,
                                     SnapFloatToInt((float)(hud_fade_vehiclecontrols->current.value * 1000.0)))
                             * 1.0;
            vehicle = CG_GetEntity(localClientNum, ps->viewlocked_entNum);
            info = CG_GetVehicleInfo(vehicle->nextState.vehicleState.vehicleInfoIndex);
            if ( info->remoteControl )
            {
                if ( info->isNitrous && info->type != 6 && vehicle->nitrousVeh )
                {
                    useBinding = 1;
                    switch ( hintType )
                    {
                        case 371:
                            if ( vehicle->nitrousVeh->m_vehicle_info->boostDuration <= vehicle->nitrousVeh->m_boost_time_pool )
                            {
                                if ( !UI_GetKeyBindingLocalizedString(localClientNum, "+vehicleboost", binding, 0) )
                                    UI_GetKeyBindingLocalizedString(localClientNum, "+vehicleboost", binding, 1);
                                string = UI_SafeTranslateString("PLATFORM_RC_BOMB_BOOST");
                                goto LABEL_17;
                            }
                            return;
                        case 372:
                            UI_GetKeyBindingLocalizedString(localClientNum, "+attack", binding, 0);
                            string = UI_SafeTranslateString("PLATFORM_RC_BOMB_DETONATE");
                            goto LABEL_17;
                        case 373:
                            string = UI_SafeTranslateString("PLATFORM_RC_BOMB_STEERING");
                            useBinding = 0;
                            goto LABEL_17;
                        case 374:
                            string = UI_SafeTranslateString("PLATFORM_RC_BOMB_ACCELERATE");
                            useBinding = 0;
                            goto LABEL_17;
                        case 375:
                            UI_GetKeyBindingLocalizedString(localClientNum, "+speed_throw", binding, 0);
                            string = UI_SafeTranslateString("PLATFORM_RC_BOMB_HANDBRAKE");
LABEL_17:
                            if ( useBinding )
                                string = UI_ReplaceConversionString(string, binding);
                            UI_DrawText(
                                &scrPlaceView[localClientNum],
                                string,
                                0x7FFFFFFF,
                                font,
                                rect->x,
                                rect->y,
                                rect->horzAlign,
                                rect->vertAlign,
                                fontscale,
                                color,
                                textStyle);
                            break;
                        default:
                            return;
                    }
                }
            }
        }
    }
}

void __cdecl CG_DrawGuidedMissileBoostHint(
                int localClientNum,
                const rectDef_s *rect,
                Font_s *font,
                float fontscale,
                int textStyle)
{
    cg_s *LocalClientGlobals; // eax
    unsigned int ViewmodelWeaponIndex; // eax
    char *v7; // eax
    char *string; // [esp+1Ch] [ebp-114h]
    char binding[260]; // [esp+20h] [ebp-110h] BYREF
    const playerState_s *ps; // [esp+128h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+12Ch] [ebp-4h]

    if ( cg_drawBreathHint->current.enabled )
    {
        LocalClientGlobals = CG_GetLocalClientGlobals(localClientNum);
        ps = &LocalClientGlobals->predictedPlayerState;
        if ( (LocalClientGlobals->predictedPlayerState.eFlags2 & 0x40000) != 0 )
        {
            ViewmodelWeaponIndex = BG_GetViewmodelWeaponIndex(ps);
            weapDef = BG_GetWeaponDef(ViewmodelWeaponIndex);
            if ( weapDef->guidedMissileType == MISSILE_GUIDANCE_TVGUIDED )
            {
                UI_GetKeyBindingLocalizedString(localClientNum, "+smoke", binding, 0);
                string = UI_SafeTranslateString("PLATFORM_GUIDED_MISSILE_BOOST");
                v7 = UI_ReplaceConversionString(string, binding);
                UI_DrawText(
                    &scrPlaceView[localClientNum],
                    v7,
                    0x7FFFFFFF,
                    font,
                    rect->x,
                    rect->y,
                    rect->horzAlign,
                    rect->vertAlign,
                    fontscale,
                    colorWhite,
                    textStyle);
            }
        }
    }
}

void __cdecl CG_DrawGuidedMissileDetonateHint(
                int localClientNum,
                const rectDef_s *rect,
                Font_s *font,
                float fontscale,
                int textStyle)
{
    cg_s *LocalClientGlobals; // eax
    unsigned int ViewmodelWeaponIndex; // eax
    char *v7; // eax
    char *string; // [esp+1Ch] [ebp-114h]
    char binding[260]; // [esp+20h] [ebp-110h] BYREF
    const playerState_s *ps; // [esp+128h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+12Ch] [ebp-4h]

    LocalClientGlobals = CG_GetLocalClientGlobals(localClientNum);
    ps = &LocalClientGlobals->predictedPlayerState;
    if ( (LocalClientGlobals->predictedPlayerState.eFlags2 & 0x40000) != 0 )
    {
        ViewmodelWeaponIndex = BG_GetViewmodelWeaponIndex(ps);
        weapDef = BG_GetWeaponDef(ViewmodelWeaponIndex);
        if ( weapDef->guidedMissileType == MISSILE_GUIDANCE_TVGUIDED )
        {
            UI_GetKeyBindingLocalizedString(localClientNum, "+attack", binding, 0);
            v7 = UI_SafeTranslateString("PLATFORM_GUIDED_MISSILE_DETONATE");
            string = UI_ReplaceConversionString(v7, binding);
            UI_DrawText(
                &scrPlaceView[localClientNum],
                string,
                0x7FFFFFFF,
                font,
                rect->x,
                rect->y,
                rect->horzAlign,
                rect->vertAlign,
                fontscale,
                colorWhite,
                textStyle);
        }
    }
}

void __cdecl CG_DrawGuidedMissileInfo(
                int localClientNum,
                const rectDef_s *rect,
                Font_s *font,
                float fontscale,
                int textStyle,
                eGuidedMissileInfo infoType)
{
    unsigned int ViewmodelWeaponIndex; // eax
    double ClosestTargetToGuidedMissile; // st7
    double DistanceGuidedMissileAndOwner; // st7
    const cg_s *cgameGlob; // [esp+34h] [ebp-114h]
    char string[256]; // [esp+38h] [ebp-110h] BYREF
    const playerState_s *ps; // [esp+13Ch] [ebp-Ch]
    const WeaponDef *weapDef; // [esp+140h] [ebp-8h]
    float x; // [esp+144h] [ebp-4h]

    if ( cg_drawBreathHint->current.enabled )
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        ps = &cgameGlob->predictedPlayerState;
        ViewmodelWeaponIndex = BG_GetViewmodelWeaponIndex(&cgameGlob->predictedPlayerState);
        weapDef = BG_GetWeaponDef(ViewmodelWeaponIndex);
        if ( ((cgameGlob->predictedPlayerState.eFlags2 & 0x40000) != 0
             || BG_GetWeaponDef(ps->weapon)->guidedMissileType == MISSILE_GUIDANCE_TVGUIDED && ps->fWeaponPosFrac >= 0.95)
            && weapDef->guidedMissileType == MISSILE_GUIDANCE_TVGUIDED )
        {
            if ( infoType )
            {
                if ( infoType == GUIDED_MISSILE_DIST_TO_TARG )
                {
                    ClosestTargetToGuidedMissile = CG_GetClosestTargetToGuidedMissile(localClientNum);
                    Com_sprintf(string, 0x100u, "DTT %.2f", ClosestTargetToGuidedMissile);
                }
                else
                {
                    if ( infoType != GUIDED_MISSILE_DIST_FROM_OWNER )
                        return;
                    DistanceGuidedMissileAndOwner = CG_GetDistanceGuidedMissileAndOwner(localClientNum);
                    Com_sprintf(string, 0x100u, "DFO %.2f", DistanceGuidedMissileAndOwner);
                }
            }
            else
            {
                Com_sprintf(
                    string,
                    0x100u,
                    "VP %.2f/%.2f/%.2f",
                    cgameGlob->refdef.vieworg[0],
                    cgameGlob->refdef.vieworg[1],
                    cgameGlob->refdef.vieworg[2]);
            }
            x = rect->x - (float)SnapFloatToInt((float)((float)UI_TextWidth(string, 0, font, fontscale) * 0.5));
            UI_DrawText(
                &scrPlaceView[localClientNum],
                string,
                0x7FFFFFFF,
                font,
                x,
                rect->y,
                rect->horzAlign,
                rect->vertAlign,
                fontscale,
                colorWhite,
                textStyle);
        }
    }
}

double __cdecl CG_GetClosestTargetToGuidedMissile(int localClientNum)
{
    int clientNum; // ecx
    int v2; // ecx
    int i; // [esp+2Ch] [ebp-2Ch]
    const clientInfo_t *client; // [esp+30h] [ebp-28h]
    float closestEntDist; // [esp+34h] [ebp-24h]
    const clientInfo_t *localClientInfo; // [esp+38h] [ebp-20h]
    float currentEntDist; // [esp+48h] [ebp-10h]
    const cg_s *cgameGlob; // [esp+4Ch] [ebp-Ch]
    centity_s *cent; // [esp+50h] [ebp-8h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_newDraw_mp.cpp", 2135, 0, "%s", "cgameGlob") )
    {
        __debugbreak();
    }
    clientNum = cgameGlob->clientNum;
    localClientInfo = &cgameGlob->bgs.clientinfo[clientNum];
    if ( (const cg_s *)((char *)cgameGlob + clientNum * 1480) == (const cg_s *)-389096
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_newDraw_mp.cpp",
                    2138,
                    0,
                    "%s",
                    "localClientInfo") )
    {
        __debugbreak();
    }
    closestEntDist = -1.0f;
    for ( i = 0; i < 32; ++i )
    {
        cent = CG_GetEntity(localClientNum, i);
        if ( ((*((unsigned int *)cent + 201) >> 1) & 1) != 0
            && cent->nextState.number != cgameGlob->clientNum
            && cent->nextState.eType == 1
            && (cent->nextState.lerp.eFlags & 0x40000) == 0 )
        {
            v2 = cent->nextState.clientNum;
            client = &cgameGlob->bgs.clientinfo[v2];
            if ( (const cg_s *)((char *)cgameGlob + v2 * 1480) == (const cg_s *)-389096
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_newDraw_mp.cpp", 2160, 0, "%s", "client") )
            {
                __debugbreak();
            }
            if ( client->team == TEAM_FREE || client->team != localClientInfo->team )
            {
                currentEntDist = Vec3Distance(cgameGlob->refdef.vieworg, cent->pose.origin);
                Vec3DistanceSq(cgameGlob->refdef.vieworg, cent->pose.origin);
                if ( closestEntDist > currentEntDist || closestEntDist == -1.0 )
                    closestEntDist = currentEntDist;
            }
        }
    }
    if ( closestEntDist == -1.0 )
        return 0.0f;
    return closestEntDist;
}

double __cdecl CG_GetDistanceGuidedMissileAndOwner(int localClientNum)
{
    cg_s *LocalClientGlobals; // eax
    const cg_s *cgameGlob; // [esp+10h] [ebp-Ch]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_newDraw_mp.cpp", 2187, 0, "%s", "cgameGlob") )
    {
        __debugbreak();
    }
    LocalClientGlobals = CG_GetLocalClientGlobals(localClientNum);
    if ( LocalClientGlobals->predictedPlayerState.pm_type < 9 )
        return (float)Vec3Distance(cgameGlob->refdef.vieworg, LocalClientGlobals->predictedPlayerState.origin);
    else
        return 0.0f;
}

void __cdecl CG_DrawTurretPlaceHint(
                int localClientNum,
                const rectDef_s *rect,
                Font_s *font,
                float fontscale,
                const float *color,
                int textStyle)
{
    cg_s *LocalClientGlobals; // eax
    int v7; // eax
    char *v8; // eax
    char *string; // [esp+1Ch] [ebp-120h]
    float height; // [esp+20h] [ebp-11Ch]
    char binding[260]; // [esp+24h] [ebp-118h] BYREF
    const playerState_s *ps; // [esp+12Ch] [ebp-10h]
    float length; // [esp+130h] [ebp-Ch]

    LocalClientGlobals = CG_GetLocalClientGlobals(localClientNum);
    ps = &LocalClientGlobals->predictedPlayerState;
    if ( (LocalClientGlobals->predictedPlayerState.weapFlags & 0x400000) != 0 )
    {
        if ( ((unsigned int)0x800000 & ps->weapFlags) != 0 )
        {
            string = UI_SafeTranslateString("MP_INVALID_TURRET_LOCATION");
            v7 = UI_TextWidth(string, 0, font, fontscale);
        }
        else
        {
            UI_GetKeyBindingLocalizedString(localClientNum, "+attack", binding, 0);
            v8 = UI_SafeTranslateString("PLATFORM_PLACE_TURRET_HINT");
            string = UI_ReplaceConversionString(v8, binding);
            v7 = UI_TextWidth(string, 0, font, fontscale);
        }
        length = (float)v7;
        height = (float)UI_TextHeight(font, fontscale);
        UI_DrawText(
            &scrPlaceView[localClientNum],
            string,
            0x7FFFFFFF,
            font,
            rect->x - (float)((float)(rect->w + length) * 0.5),
            (float)(height * 0.5) + rect->y,
            rect->horzAlign,
            rect->vertAlign,
            fontscale,
            colorWhite,
            textStyle);
    }
}

float adjustB;
void __cdecl CG_DrawMantleHint(
                int localClientNum,
                const rectDef_s *rect,
                Font_s *font,
                float fontscale,
                const float *color,
                int textStyle)
{
    cg_s *LocalClientGlobals; // eax
    char *v7; // eax
    char *v8; // eax
    float heightAdjust; // [esp+20h] [ebp-124h]
    char *string; // [esp+24h] [ebp-120h]
    char *stringa; // [esp+24h] [ebp-120h]
    float height; // [esp+28h] [ebp-11Ch]
    float heighta; // [esp+28h] [ebp-11Ch]
    char binding[260]; // [esp+2Ch] [ebp-118h] BYREF
    const playerState_s *ps; // [esp+134h] [ebp-10h]
    float length; // [esp+138h] [ebp-Ch]
    float x; // [esp+13Ch] [ebp-8h]
    float y; // [esp+140h] [ebp-4h]

    if ( !cgMedia.mantleHint
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_newDraw_mp.cpp",
                    2297,
                    0,
                    "%s",
                    "cgMedia.mantleHint") )
    {
        __debugbreak();
    }
    if ( cg_drawMantleHint->current.enabled )
    {
        LocalClientGlobals = CG_GetLocalClientGlobals(localClientNum);
        ps = &LocalClientGlobals->predictedPlayerState;
        if ( (LocalClientGlobals->predictedPlayerState.mantleState.flags & 0x10) != 0 || (ps->mantleState.flags & 0x80) != 0 )
        {
            if ( (ps->mantleState.flags & 0x10) != 0 )
            {
                if ( !UI_GetKeyBindingLocalizedString(localClientNum, "+gostand", binding, 0) )
                    UI_GetKeyBindingLocalizedString(localClientNum, "+moveup", binding, 0);
                v7 = UI_SafeTranslateString("PLATFORM_MANTLE");
                string = UI_ReplaceConversionString(v7, binding);
                length = (float)UI_TextWidth(string, 0, font, fontscale);
                height = (float)UI_TextHeight(font, fontscale);
                heightAdjust = 0.0f;
                if ( (ps->mantleState.flags & 0x80) != 0 )
                    heightAdjust = -35.0f;
                x = rect->x - (float)((float)(rect->w + length) * 0.5);
                y = (float)(rect->y + heightAdjust) + (float)(height * 0.5);
                UI_DrawText(
                    &scrPlaceView[localClientNum],
                    string,
                    0x7FFFFFFF,
                    font,
                    x,
                    y,
                    rect->horzAlign,
                    rect->vertAlign,
                    fontscale,
                    colorWhite,
                    textStyle);
                x = x + length;
                y = y - (float)((float)(rect->h * 0.25) + height);
                UI_DrawHandlePic(
                    &scrPlaceView[localClientNum],
                    x,
                    y,
                    rect->w,
                    rect->h,
                    rect->horzAlign,
                    rect->vertAlign,
                    colorWhite,
                    cgMedia.mantleHint);
            }
            if ( (ps->mantleState.flags & 0x80) != 0 )
            {
                if ( !UI_GetKeyBindingLocalizedString(localClientNum, "+usereload", binding, 0) )
                    UI_GetKeyBindingLocalizedString(localClientNum, "+activate", binding, 0);
                v8 = UI_SafeTranslateString("PLATFORM_MOUNT");
                stringa = UI_ReplaceConversionString(v8, binding);
                length = (float)UI_TextWidth(stringa, 0, font, fontscale);
                heighta = (float)UI_TextHeight(font, fontscale);
                UI_DrawText(
                    &scrPlaceView[localClientNum],
                    stringa,
                    0x7FFFFFFF,
                    font,
                    rect->x - (float)((float)(rect->w + length) * 0.5),
                    (float)(rect->y + adjustB) + (float)(heighta * 0.5),
                    rect->horzAlign,
                    rect->vertAlign,
                    fontscale,
                    colorWhite,
                    textStyle);
            }
        }
    }
}

void __cdecl CG_DrawInvalidCmdHint(
                int localClientNum,
                const rectDef_s *rect,
                Font_s *font,
                float fontscale,
                float *color,
                int textStyle)
{
    cg_s *cgameGlob; // [esp+34h] [ebp-10h]
    const char *string; // [esp+38h] [ebp-Ch]
    float x; // [esp+3Ch] [ebp-8h]
    int blinkInterval; // [esp+40h] [ebp-4h]

    if ( !rect
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_newDraw_mp.cpp", 2380, 0, "%s", "rect") )
    {
        __debugbreak();
    }
    if ( !color
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_newDraw_mp.cpp", 2381, 0, "%s", "color") )
    {
        __debugbreak();
    }
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cg_invalidCmdHintDuration->current.integer + cgameGlob->invalidCmdHintTime < cgameGlob->time )
        cgameGlob->invalidCmdHintType = INVALID_CMD_NONE;
    switch ( cgameGlob->invalidCmdHintType )
    {
        case INVALID_CMD_NO_AMMO_BULLETS:
            string = UI_SafeTranslateString("WEAPON_NO_AMMO");
            goto LABEL_29;
        case INVALID_CMD_NO_AMMO_FRAG_GRENADE:
            string = UI_SafeTranslateString("WEAPON_NO_FRAG_GRENADE");
            goto LABEL_29;
        case INVALID_CMD_NO_AMMO_SPECIAL_GRENADE:
            string = UI_SafeTranslateString("WEAPON_NO_SPECIAL_GRENADE");
            goto LABEL_29;
        case INVALID_CMD_NO_AMMO_EQUIPMENT:
            string = UI_SafeTranslateString("WEAPON_NO_EQUIPMENT");
            goto LABEL_29;
        case INVALID_CMD_STAND_BLOCKED:
            string = UI_SafeTranslateString("GAME_STAND_BLOCKED");
            goto LABEL_29;
        case INVALID_CMD_CROUCH_BLOCKED:
            string = UI_SafeTranslateString("GAME_CROUCH_BLOCKED");
            goto LABEL_29;
        case INVALID_CMD_TARGET_TOO_CLOSE:
            if ( BG_WeaponBlocksProne(cgameGlob->predictedPlayerState.weapon) )
                string = UI_SafeTranslateString("CGAME_PRONE_BLOCKED_WEAPON");
            else
                string = UI_SafeTranslateString("CGAME_PRONE_BLOCKED");
            goto LABEL_29;
        case INVALID_CMD_LOCKON_REQUIRED:
            string = UI_SafeTranslateString("WEAPON_TARGET_TOO_CLOSE");
            goto LABEL_29;
        case INVALID_CMD_NOT_ENOUGH_CLEARANCE:
            string = UI_SafeTranslateString("WEAPON_LOCKON_REQUIRED");
            goto LABEL_29;
        case INVALID_CMD_CANT_PLACE_TURRET:
            string = UI_SafeTranslateString("WEAPON_TARGET_NOT_ENOUGH_CLEARANCE");
            goto LABEL_29;
        case INVALID_CMD_CANT_EQUIP_WHILE_PRONE:
            string = UI_SafeTranslateString("WEAPON_CANT_PLACE_TURRET");
            goto LABEL_29;
        case INVALID_CMD_CANT_PLANT_EQUIPMENT:
            string = UI_SafeTranslateString("WEAPON_CANT_EQUIP_WHILE_PRONE");
            goto LABEL_29;
        case INVALID_CMD_CANT_EQUIP_WHILE_PRONE|INVALID_CMD_NO_AMMO_FRAG_GRENADE:
            string = UI_SafeTranslateString("WEAPON_CANT_PLANT_EQUIPMENT");
LABEL_29:
            blinkInterval = cg_invalidCmdHintBlinkInterval->current.integer;
            if ( blinkInterval <= 0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_newDraw_mp.cpp",
                            2450,
                            0,
                            "%s",
                            "blinkInterval > 0") )
            {
                __debugbreak();
            }
            color[3] = (float)((cgameGlob->time - cgameGlob->invalidCmdHintTime) % blinkInterval) / (float)blinkInterval;
            x = rect->x - (float)SnapFloatToInt((float)((float)UI_TextWidth(string, 0, font, fontscale) * 0.5));
            UI_DrawText(
                &scrPlaceView[localClientNum],
                (char*)string,
                0x7FFFFFFF,
                font,
                x,
                rect->y,
                rect->horzAlign,
                rect->vertAlign,
                fontscale,
                color,
                textStyle);
            break;
        default:
            if ( cgameGlob->invalidCmdHintType
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_newDraw_mp.cpp",
                            2390,
                            0,
                            "%s",
                            "cgameGlob->invalidCmdHintType == INVALID_CMD_NONE") )
            {
                __debugbreak();
            }
            break;
    }
}

void __cdecl CG_DrawTalkerNum(
                int localClientNum,
                int num,
                rectDef_s *rect,
                Font_s *font,
                float *color,
                float textScale,
                int style)
{
    int UIContextIndex; // eax
    bool v8; // [esp+20h] [ebp-38h]
    team_t team; // [esp+28h] [ebp-30h]
    int client; // [esp+2Ch] [ebp-2Ch]
    Material *material; // [esp+30h] [ebp-28h]
    cg_s *cgameGlob; // [esp+34h] [ebp-24h]
    float textColor[4]; // [esp+38h] [ebp-20h] BYREF
    char *name; // [esp+48h] [ebp-10h]
    int drawTalk; // [esp+4Ch] [ebp-Ch]
    int textHeight; // [esp+50h] [ebp-8h]
    bool isEnemy; // [esp+57h] [ebp-1h]

    if ( !color
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_newDraw_mp.cpp", 2475, 0, "%s", "color") )
    {
        __debugbreak();
    }
    if ( !rect
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_newDraw_mp.cpp", 2476, 0, "%s", "rect") )
    {
        __debugbreak();
    }
    drawTalk = cg_drawTalk->current.integer;
    if ( drawTalk )
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        UIContextIndex = Com_LocalClient_GetUIContextIndex(localClientNum);
        client = UI_GetTalkerClientNum(localClientNum, UIContextIndex, num);
        if ( client >= 0 )
        {
            name = cgameGlob->bgs.clientinfo[client].name;
            if ( cgameGlob->bgs.clientinfo[client].infoValid )
            {
                v8 = 0;
                if ( cgameGlob->bgs.clientinfo[cgameGlob->clientNum].team )
                {
                    team = cgameGlob->bgs.clientinfo[cgameGlob->clientNum].team;
                    if ( team == TEAM_FREE || team != cgameGlob->bgs.clientinfo[client].team )
                        v8 = 1;
                }
                isEnemy = v8;
                if ( (drawTalk != 2 || !isEnemy) && (drawTalk != 3 || isEnemy) )
                {
                    textColor[0] = *color;
                    textColor[1] = color[1];
                    textColor[2] = color[2];
                    textColor[3] = color[3];
                    material = Material_RegisterHandle("voice_on", 7);
                    textHeight = UI_TextHeight(font, textScale);
                    UI_DrawHandlePic(
                        &scrPlaceView[localClientNum],
                        rect->x,
                        rect->y,
                        rect->w,
                        rect->h,
                        rect->horzAlign,
                        rect->vertAlign,
                        color,
                        material);
                    UI_DrawText(
                        &scrPlaceView[localClientNum],
                        name,
                        32,
                        font,
                        (float)(rect->x + rect->w) + 2.0,
                        (float)((float)textHeight + rect->y) + (float)((float)(rect->h - (float)textHeight) / 2.0),
                        rect->horzAlign,
                        rect->vertAlign,
                        textScale,
                        textColor,
                        style);
                }
            }
            else
            {
                Com_PrintWarning(13, "client %i has invalid info but they are talking\n", client);
            }
        }
    }
}

void __cdecl CG_DrawWarText(
                const cg_s *cgameGlob,
                rectDef_s *rect,
                Font_s *font,
                float fontscale,
                float *color,
                int textStyle,
                float text_x,
                float text_y)
{
    const char *v8; // eax
    const char *v9; // eax
    const char *v10; // eax
    const char *v11; // eax
    char *v12; // eax
    const char *wartextText; // [esp+1Ch] [ebp-290h]
    const dvar_s *locationDvar; // [esp+20h] [ebp-28Ch]
    const char *teamName; // [esp+24h] [ebp-288h]
    char flag; // [esp+2Bh] [ebp-281h]
    char *wartexta; // [esp+2Ch] [ebp-280h]
    char *wartext; // [esp+2Ch] [ebp-280h]
    char *wartextb; // [esp+2Ch] [ebp-280h]
    const dvar_s *teamDvar; // [esp+30h] [ebp-27Ch]
    const dvar_s *teamDvara; // [esp+30h] [ebp-27Ch]
    team_t localTeam; // [esp+34h] [ebp-278h]
    char action; // [esp+3Bh] [ebp-271h]
    const char *location; // [esp+4Ch] [ebp-260h]
    const dvar_s *wartextDvar; // [esp+50h] [ebp-25Ch]
    ConversionArguments convArgs; // [esp+54h] [ebp-258h] BYREF
    rectDef_s textRect; // [esp+7Ch] [ebp-230h] BYREF
    float otherTeamCappingColor[4]; // [esp+94h] [ebp-218h]
    char convertedStr[516]; // [esp+A4h] [ebp-208h] BYREF

    otherTeamCappingColor[0] = 1.0f;
    otherTeamCappingColor[1] = 0.3f;
    otherTeamCappingColor[2] = 0.3f;
    otherTeamCappingColor[3] = 1.0f;
    localTeam = cgameGlob->bgs.clientinfo[cgameGlob->predictedPlayerState.clientNum].team;
    if ( localTeam == TEAM_ALLIES || localTeam == TEAM_AXIS )
    {
        wartextDvar = Dvar_FindVar("war_sb");
        if ( wartextDvar )
        {
            wartextText = wartextDvar->current.string;
            flag = wartextText[2];
            if ( flag >= 97 || flag <= 101 )
            {
                v8 = va("war_%c", flag);
                locationDvar = Dvar_FindVar(v8);
                if ( locationDvar )
                {
                    location = locationDvar->current.string;
                    if ( *location == 64 )
                        ++location;
                    action = *wartextText;
                    if ( *wartextText == 97 || action == 120 || action == 99 )
                    {
                        switch ( action )
                        {
                            case 'c':
                                wartexta = UI_SafeTranslateString("MENU_WAR_STATUS_CONTESTED");
                                v9 = SEH_LocalizeTextMessage(location, "game message", LOCMSG_SAFE);
                                wartext = UI_ReplaceConversionString(wartexta, v9);
                                *color = 1.0f;
                                color[1] = 1.0f;
                                color[2] = 0.3f;
                                color[3] = 1.0f;
LABEL_26:
                                UI_DrawWrappedText(
                                    &scrPlaceFull,
                                    wartext,
                                    rect,
                                    font,
                                    rect->x + text_x,
                                    rect->y + text_y,
                                    fontscale,
                                    color,
                                    textStyle,
                                    1,
                                    &textRect);
                                return;
                            case 'a':
                                if ( localTeam == TEAM_ALLIES )
                                {
LABEL_16:
                                    wartextb = UI_SafeTranslateString("MENU_WAR_STATUS_WERE_RAISING");
                                    v10 = SEH_LocalizeTextMessage(location, "game message", LOCMSG_SAFE);
                                    wartext = UI_ReplaceConversionString(wartextb, v10);
                                    goto LABEL_26;
                                }
                                teamDvar = Dvar_FindVar("scr_allies");
                                if ( teamDvar )
                                {
                                    teamName = teamDvar->current.string;
                                    if ( *teamName )
                                    {
LABEL_19:
                                        convArgs.argCount = 2;
                                        v11 = va("MPUI_%s", teamName);
                                        convArgs.args[0] = SEH_LocalizeTextMessage(v11, "game message", LOCMSG_SAFE);
                                        convArgs.args[1] = SEH_LocalizeTextMessage(location, "game message", LOCMSG_SAFE);
                                        v12 = UI_SafeTranslateString("MENU_WAR_STATUS_TAKING");
                                        UI_ReplaceConversions(v12, &convArgs, convertedStr, 512);
                                        *color = otherTeamCappingColor[0];
                                        color[1] = otherTeamCappingColor[1];
                                        color[2] = otherTeamCappingColor[2];
                                        color[3] = otherTeamCappingColor[3];
                                        UI_DrawWrappedText(
                                            &scrPlaceFull,
                                            convertedStr,
                                            rect,
                                            font,
                                            rect->x + text_x,
                                            rect->y + text_y,
                                            fontscale,
                                            color,
                                            textStyle,
                                            1,
                                            &textRect);
                                    }
                                }
                                break;
                            case 'x':
                                if ( localTeam == TEAM_AXIS )
                                    goto LABEL_16;
                                teamDvara = Dvar_FindVar("scr_axis");
                                if ( teamDvara )
                                {
                                    teamName = teamDvara->current.string;
                                    if ( *teamName )
                                        goto LABEL_19;
                                }
                                break;
                        }
                    }
                }
            }
        }
    }
}

