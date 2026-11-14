#include "offhandweapons.h"
#include <cgame_mp/cg_local_mp.h>
#include <cgame_mp/cg_newDraw_mp.h>
#include <bgame/bg_weapons_def.h>
#include <ui/ui_atoms.h>
#include <bgame/bg_mantle.h>
#include <sound/snd_bank.h>
#include "cg_sound.h"
#include "cg_event.h"
#include <clientscript/scr_const.h>
#include "cg_local.h"
#include <cgame_mp/cg_ents_mp.h>
#include <qcommon/dobj_management.h>
#include <cgame_mp/cg_servercmds_mp.h>
#include <qcommon/common.h>

const char *offhandStrings[5] =
{
  "",
  "WEAPON_FRAGGRENADE",
  "WEAPON_SMOKEGRENADE",
  "WEAPON_FLASHGRENADE",
  NULL
};


const dvar_t *hud_flash_time_offhand;
const dvar_t *hud_flash_period_offhand;

void __cdecl CG_OffhandRegisterDvars()
{
    hud_flash_time_offhand = _Dvar_RegisterFloat(
                                                         "hud_flash_time_offhand",
                                                         2.0,
                                                         0.0,
                                                         30.0,
                                                         1u,
                                                         "Offhand weapons flash duration on changing weapon");
    hud_flash_period_offhand = _Dvar_RegisterFloat(
                                                             "hud_flash_period_offhand",
                                                             0.5,
                                                             0.0,
                                                             30.0,
                                                             1u,
                                                             "Offhand weapons flash period on changing weapon");
}

void __cdecl CG_DrawOffHandIcon(
                int localClientNum,
                const rectDef_s *rect,
                const float *color,
                Material *material,
                OffhandSlot weaponType)
{
    float v5; // [esp+20h] [ebp-84h]
    float v6; // [esp+2Ch] [ebp-78h]
    float v7; // [esp+38h] [ebp-6Ch]
    float v8; // [esp+44h] [ebp-60h]
    int ammoCount; // [esp+64h] [ebp-40h]
    const cg_s *cgameGlob; // [esp+68h] [ebp-3Ch]
    float drawColor; // [esp+70h] [ebp-34h]
    float drawColor_4; // [esp+74h] [ebp-30h]
    float drawColor_8; // [esp+78h] [ebp-2Ch]
    float drawColor_12; // [esp+7Ch] [ebp-28h]
    unsigned int weapIndex; // [esp+80h] [ebp-24h]
    float xPos; // [esp+84h] [ebp-20h]
    const WeaponDef *equippedWeapDef; // [esp+8Ch] [ebp-18h]
    float tempColor[4]; // [esp+94h] [ebp-10h] BYREF

    if ( !rect && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\offhandweapons.cpp", 140, 0, "%s", "rect") )
        __debugbreak();
    if ( !color
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\offhandweapons.cpp", 141, 0, "%s", "color") )
    {
        __debugbreak();
    }
    if ( weaponType <= OFFHAND_SLOT_NONE
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\offhandweapons.cpp",
                    142,
                    0,
                    "%s",
                    "weaponType > OFFHAND_SLOT_NONE") )
    {
        __debugbreak();
    }
    if ( weaponType >= OFFHAND_SLOT_COUNT
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\offhandweapons.cpp",
                    143,
                    0,
                    "%s",
                    "weaponType < OFFHAND_SLOT_COUNT") )
    {
        __debugbreak();
    }
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( IsOffHandDisplayVisible(cgameGlob) )
    {
        if ( GetBestOffhand(&cgameGlob->predictedPlayerState, weaponType) )
        {
            drawColor_12 = CG_FadeHudMenu(
                                             localClientNum,
                                             hud_fade_offhand,
                                             cgameGlob->offhandFadeTime,
                                             SnapFloatToInt((float)(hud_fade_offhand->current.value * 1000.0)))
                                     * color[3];
            if ( drawColor_12 != 0.0 )
            {
                drawColor = *color;
                drawColor_4 = color[1];
                drawColor_8 = color[2];
                weapIndex = 0;
                if ( cgameGlob->equippedOffHand && BG_GetWeaponDef(cgameGlob->equippedOffHand)->offhandSlot == weaponType )
                    weapIndex = cgameGlob->equippedOffHand;
                if ( !weapIndex )
                    weapIndex = GetBestOffhand(&cgameGlob->predictedPlayerState, weaponType);
                if ( weapIndex )
                {
                    equippedWeapDef = BG_GetWeaponDef(weapIndex);
                    for ( ammoCount = CalcOffHandAmmo(&cgameGlob->predictedPlayerState, weaponType) - 1; ammoCount >= 0; --ammoCount )
                    {
                        xPos = (float)((float)(rect->w * 0.22) * (float)ammoCount) + rect->x;
                        tempColor[0] = drawColor;
                        tempColor[1] = drawColor_4;
                        tempColor[2] = drawColor_8;
                        tempColor[3] = drawColor_12;
                        v8 = _Pow_int<float>(0.80000001, ammoCount);
                        tempColor[0] = tempColor[0] * v8;
                        v7 = _Pow_int<float>(0.80000001, ammoCount);
                        tempColor[1] = tempColor[1] * v7;
                        v6 = _Pow_int<float>(0.80000001, ammoCount);
                        tempColor[2] = tempColor[2] * v6;
                        v5 = _Pow_int<float>(0.80000001, ammoCount);
                        tempColor[3] = tempColor[3] * v5;
                        UI_DrawHandlePic(
                            &scrPlaceView[localClientNum],
                            xPos,
                            rect->y,
                            rect->w,
                            rect->h,
                            rect->horzAlign,
                            rect->vertAlign,
                            tempColor,
                            equippedWeapDef->hudIcon);
                    }
                }
            }
        }
    }
}

int __cdecl GetBestOffhand(const playerState_s *predictedPlayerState, int offhandSlot)
{
    int newOffhand; // [esp+0h] [ebp-4h]

    newOffhand = BG_GetFirstAvailableOffhand(predictedPlayerState, offhandSlot);
    if ( !newOffhand )
        return BG_GetFirstEquippedOffhand(predictedPlayerState, offhandSlot);
    return newOffhand;
}

bool __cdecl IsOffHandDisplayVisible(const cg_s *cgameGlob)
{
    if ( cgameGlob->predictedPlayerState.pm_type >= 9 )
        return 0;
    if ( (cgameGlob->predictedPlayerState.weapFlags & 0x80) != 0 )
        return 0;
    if ( (cgameGlob->predictedPlayerState.weapFlags & 0x100) != 0 )
        return 0;
    if ( (cgameGlob->predictedPlayerState.eFlags & 0x4000) != 0 )
        return 0;
    return (cgameGlob->predictedPlayerState.eFlags & 0x300) == 0
            || cgameGlob->predictedPlayerState.weaponstate == 35
            || cgameGlob->predictedPlayerState.weaponstate == 36
            || cgameGlob->predictedPlayerState.weaponstate == 37;
}

int __cdecl CalcOffHandAmmo(const playerState_s *predictedPlayerState, int weaponType)
{
    int slot; // [esp+8h] [ebp-10h]
    int ammoCount; // [esp+Ch] [ebp-Ch]
    unsigned int weapIndex; // [esp+10h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+14h] [ebp-4h]

    ammoCount = 0;
    for ( slot = 0; slot < 15; ++slot )
    {
        weapIndex = predictedPlayerState->heldWeapons[slot].weapon;
        if ( weapIndex )
        {
            weapDef = BG_GetWeaponDef(weapIndex);
            if ( !weapDef
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\offhandweapons.cpp", 118, 0, "%s", "weapDef") )
            {
                __debugbreak();
            }
            if ( weapDef->offhandSlot == weaponType )
                ammoCount += BG_GetAmmoInClip(predictedPlayerState, weapIndex);
        }
    }
    return ammoCount;
}

void __cdecl CG_DrawOffHandHighlight(
                int localClientNum,
                const rectDef_s *rect,
                const float *color,
                Material *material,
                OffhandSlot weaponType)
{
    const cg_s *cgameGlob; // [esp+34h] [ebp-28h]
    float drawColor[4]; // [esp+38h] [ebp-24h] BYREF
    float flashColor[4]; // [esp+48h] [ebp-14h] BYREF
    const WeaponDef *weapDef; // [esp+58h] [ebp-4h]

    if ( !rect && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\offhandweapons.cpp", 235, 0, "%s", "rect") )
        __debugbreak();
    if ( !color
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\offhandweapons.cpp", 236, 0, "%s", "color") )
    {
        __debugbreak();
    }
    if ( weaponType <= OFFHAND_SLOT_NONE
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\offhandweapons.cpp",
                    237,
                    0,
                    "%s",
                    "weaponType > OFFHAND_SLOT_NONE") )
    {
        __debugbreak();
    }
    if ( weaponType >= OFFHAND_SLOT_COUNT
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\offhandweapons.cpp",
                    238,
                    0,
                    "%s",
                    "weaponType < OFFHAND_SLOT_COUNT") )
    {
        __debugbreak();
    }
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( IsOffHandDisplayVisible(cgameGlob) )
    {
        if ( GetBestOffhand(&cgameGlob->predictedPlayerState, weaponType) )
        {
            if ( cgameGlob->equippedOffHand )
            {
                drawColor[3] = CG_FadeHudMenu(
                                                 localClientNum,
                                                 hud_fade_offhand,
                                                 cgameGlob->offhandFadeTime,
                                                 SnapFloatToInt((float)(hud_fade_offhand->current.value * 1000.0)))
                                         * color[3];
                if ( drawColor[3] != 0.0 )
                {
                    weapDef = BG_GetWeaponDef(cgameGlob->equippedOffHand);
                    if ( weaponType == weapDef->offhandSlot )
                    {
                        if ( CalcOffHandAmmo(&cgameGlob->predictedPlayerState, weaponType) )
                        {
                            drawColor[0] = *color;
                            drawColor[1] = color[1];
                            drawColor[2] = color[2];
                        }
                        else
                        {
                            drawColor[0] = 0.89f;
                            drawColor[1] = 0.18000001;
                            drawColor[2] = 0.01f;
                        }
                        OffHandFlash(cgameGlob, drawColor, flashColor);
                        UI_DrawHandlePic(
                            &scrPlaceView[localClientNum],
                            rect->x,
                            rect->y,
                            rect->w,
                            rect->h,
                            rect->horzAlign,
                            rect->vertAlign,
                            flashColor,
                            material);
                    }
                }
            }
        }
    }
}

void __cdecl OffHandFlash(const cg_s *cgameGlob, const float *base_color, float *out_color)
{
    float v3; // xmm0_4
    float flashTime; // [esp+8h] [ebp-4h]

    if (!base_color
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\offhandweapons.cpp", 206, 0, "%s", "base_color"))
    {
        __debugbreak();
    }
    if (!out_color
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\offhandweapons.cpp", 207, 0, "%s", "out_color"))
    {
        __debugbreak();
    }
    if (!hud_flash_time_offhand
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\offhandweapons.cpp",
            208,
            0,
            "%s",
            "hud_flash_time_offhand"))
    {
        __debugbreak();
    }
    if (!hud_flash_period_offhand
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\offhandweapons.cpp",
            209,
            0,
            "%s",
            "hud_flash_period_offhand"))
    {
        __debugbreak();
    }
    *out_color = *base_color;
    out_color[1] = base_color[1];
    out_color[2] = base_color[2];
    out_color[3] = base_color[3];
    flashTime = (float)(cgameGlob->time - cgameGlob->offhandFlashTime) / 1000.0;
    if (hud_flash_time_offhand->current.value > flashTime)
    {
        if (hud_flash_period_offhand->current.value <= 0.0
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\offhandweapons.cpp",
                216,
                0,
                "%s",
                "hud_flash_period_offhand->current.value > 0.0f"))
        {
            __debugbreak();
        }
        //v3 = __libm_sse2_cos((float)((float)(6.2831855 * flashTime) / hud_flash_period_offhand->current.value));
        v3 = cos((float)((float)(6.2831855 * flashTime) / hud_flash_period_offhand->current.value));
        out_color[3] = (float)((float)(v3 * 0.5) + 0.5) * base_color[3];
    }
}

void __cdecl CG_DrawOffHandAmmo(
                int localClientNum,
                const rectDef_s *rect,
                Font_s *font,
                float scale,
                const float *color,
                int textStyle,
                OffhandSlot weaponType)
{
    int ammoCount; // [esp+30h] [ebp-1Ch]
    const cg_s *cgameGlob; // [esp+34h] [ebp-18h]
    float drawColor[4]; // [esp+38h] [ebp-14h] BYREF
    const char *ammoCountString; // [esp+48h] [ebp-4h]

    if ( !rect && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\offhandweapons.cpp", 279, 0, "%s", "rect") )
        __debugbreak();
    if ( !color
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\offhandweapons.cpp", 280, 0, "%s", "color") )
    {
        __debugbreak();
    }
    if ( weaponType <= OFFHAND_SLOT_NONE
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\offhandweapons.cpp",
                    281,
                    0,
                    "%s",
                    "weaponType > OFFHAND_SLOT_NONE") )
    {
        __debugbreak();
    }
    if ( weaponType >= OFFHAND_SLOT_COUNT
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\offhandweapons.cpp",
                    282,
                    0,
                    "%s",
                    "weaponType < OFFHAND_SLOT_COUNT") )
    {
        __debugbreak();
    }
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( IsOffHandDisplayVisible(cgameGlob) )
    {
        if ( GetBestOffhand(&cgameGlob->predictedPlayerState, weaponType) )
        {
            drawColor[3] = CG_FadeHudMenu(
                                             localClientNum,
                                             hud_fade_offhand,
                                             cgameGlob->offhandFadeTime,
                                             SnapFloatToInt((float)(hud_fade_offhand->current.value * 1000.0)))
                                     * color[3];
            if ( drawColor[3] != 0.0 )
            {
                ammoCount = CalcOffHandAmmo(&cgameGlob->predictedPlayerState, weaponType);
                ammoCountString = va("%i", ammoCount);
                if ( ammoCount )
                {
                    drawColor[0] = *color;
                    drawColor[1] = color[1];
                    drawColor[2] = color[2];
                }
                else
                {
                    drawColor[0] = 0.89f;
                    drawColor[1] = 0.18000001;
                    drawColor[2] = 0.01f;
                }
                UI_DrawText(
                    &scrPlaceView[localClientNum],
                    (char *)ammoCountString,
                    0x7FFFFFFF,
                    font,
                    rect->x,
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

void __cdecl CG_DrawOffHandName(
                int localClientNum,
                const rectDef_s *rect,
                Font_s *font,
                float scale,
                const float *color,
                int textStyle,
                OffhandSlot weaponType)
{
    const cg_s *cgameGlob; // [esp+30h] [ebp-18h]
    float drawColor[4]; // [esp+34h] [ebp-14h] BYREF
    const char *ammoNameString; // [esp+44h] [ebp-4h]

    if ( !rect && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\offhandweapons.cpp", 316, 0, "%s", "rect") )
        __debugbreak();
    if ( !color
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\offhandweapons.cpp", 317, 0, "%s", "color") )
    {
        __debugbreak();
    }
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( IsOffHandDisplayVisible(cgameGlob) && GetBestOffhand(&cgameGlob->predictedPlayerState, weaponType) )
    {
        drawColor[3] = CG_FadeHudMenu(
                                         localClientNum,
                                         hud_fade_offhand,
                                         cgameGlob->offhandFadeTime,
                                         SnapFloatToInt((float)(hud_fade_offhand->current.value * 1000.0)))
                                 * color[3];
        if ( drawColor[3] != 0.0 )
        {
            drawColor[0] = *color;
            drawColor[1] = color[1];
            drawColor[2] = color[2];
            ammoNameString = UI_SafeTranslateString(offhandStrings[weaponType]);
            UI_DrawText(
                &scrPlaceView[localClientNum],
                (char *)ammoNameString,
                0x7FFFFFFF,
                font,
                rect->x,
                rect->y,
                rect->horzAlign,
                rect->vertAlign,
                scale,
                drawColor,
                textStyle);
        }
    }
}

void __cdecl CG_SwitchOffHandCmd(int localClientNum)
{
    const cg_s *cgameGlob; // [esp+0h] [ebp-Ch]
    int newOffhand; // [esp+4h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+8h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->equippedOffHand )
    {
        weapDef = BG_GetWeaponDef(cgameGlob->equippedOffHand);
        if ( weapDef->offhandClass == OFFHAND_CLASS_NONE
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\offhandweapons.cpp",
                        352,
                        0,
                        "%s",
                        "weapDef->offhandClass != OFFHAND_CLASS_NONE") )
        {
            __debugbreak();
        }
        newOffhand = BG_GetFirstAvailableOffhand(&cgameGlob->predictedPlayerState, weapDef->offhandSlot);
        if ( newOffhand )
            CG_SetEquippedOffHand(localClientNum, newOffhand);
    }
}

void __cdecl CG_PrepOffHand(int localClientNum, const entityState_s *ent, unsigned int weaponIndex, bool isPlayerView)
{
    unsigned int AliasId; // eax
    int dObjHandle; // [esp+8h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+Ch] [ebp-4h]

    if ( ent->eType != 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\offhandweapons.cpp",
                    368,
                    0,
                    "%s",
                    "ent->eType == ET_PLAYER") )
    {
        __debugbreak();
    }
    if ( (!weaponIndex || weaponIndex >= BG_GetNumWeapons())
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\offhandweapons.cpp",
                    369,
                    0,
                    "%s\n\t(weaponIndex) = %i",
                    "(weaponIndex > 0 && weaponIndex < BG_GetNumWeapons())",
                    weaponIndex) )
    {
        __debugbreak();
    }
    weapDef = BG_GetWeaponDef(weaponIndex);
    if ( isPlayerView )
        AliasId = SND_FindAliasId(weapDef->pullbackSoundPlayer);
    else
        AliasId = SND_FindAliasId(weapDef->pullbackSound);
    CG_PlaySound(localClientNum, ent->number, 0, 0, 0, 1.0, AliasId);
    if ( isPlayerView )
    {
        dObjHandle = CG_WeaponDObjHandle(localClientNum);
        if ( weapDef->tagFlash_preparationEffect )
            CG_PlayBoltedEffect(localClientNum, weapDef->tagFlash_preparationEffect, dObjHandle, scr_const.tag_flash);
        if ( weapDef->tagFx_preparationEffect )
            CG_PlayBoltedEffect(localClientNum, weapDef->tagFx_preparationEffect, dObjHandle, scr_const.tag_fx);
    }
}

void __cdecl CG_UseOffHand(int localClientNum, const centity_s *cent, unsigned int weaponIndex, bool isPlayerView)
{
    unsigned int AliasId; // eax
    DObj *obj; // [esp+Ch] [ebp-1Ch]
    DObj *obja; // [esp+Ch] [ebp-1Ch]
    float origin[3]; // [esp+10h] [ebp-18h] BYREF
    cg_s *cgameGlob; // [esp+1Ch] [ebp-Ch]
    const ViewModelInfo *viewModelInfo; // [esp+20h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+24h] [ebp-4h]

    if ( cent->nextState.eType == 1 )
    {
        if ( (!weaponIndex || weaponIndex >= BG_GetNumWeapons())
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\offhandweapons.cpp",
                        408,
                        0,
                        "%s\n\t(weaponIndex) = %i",
                        "(weaponIndex > 0 && weaponIndex < BG_GetNumWeapons())",
                        weaponIndex) )
        {
            __debugbreak();
        }
        viewModelInfo = CG_GetLocalClientViewModelInfo(localClientNum);
        weapDef = BG_GetWeaponDef(weaponIndex);
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        if ( cent->nextState.number == cgameGlob->nextSnap->ps.clientNum )
        {
            obj = viewModelInfo->viewModelDObj;
            CG_UpdateViewModelPose(viewModelInfo->viewModelDObj, localClientNum);
            if ( !obj || !CG_DObjGetWorldTagPos(&cgameGlob->viewModelPose, obj, scr_const.tag_flash, origin) )
                BG_EvaluateTrajectory(&cent->nextState.lerp.pos, cgameGlob->time, origin);
        }
        else
        {
            obja = Com_GetClientDObj(cent->nextState.number, localClientNum);
            if ( !obja || !CG_DObjGetWorldTagPos(&cent->pose, obja, scr_const.tag_flash, origin) )
                BG_EvaluateTrajectory(&cent->nextState.lerp.pos, cgameGlob->time, origin);
        }
        if ( isPlayerView )
            AliasId = SND_FindAliasId(weapDef->fireSoundPlayer);
        else
            AliasId = SND_FindAliasId(weapDef->fireSound);
        CG_PlaySound(localClientNum, cent->nextState.number, origin, 0, 0, 1.0, AliasId);
    }
}

void __cdecl CG_SetEquippedOffHand(int localClientNum, int offHandIndex)
{
    const char *v2; // eax
    const char *v3; // eax

    if ( offHandIndex )
    {
        if ( BG_GetWeaponDef(offHandIndex)->offhandClass == OFFHAND_CLASS_NONE )
        {
            v2 = BG_WeaponName(offHandIndex);
            v3 = va("offHandIndex = %d (%s)\n", offHandIndex, v2);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\offhandweapons.cpp",
                            439,
                            0,
                            "%s\n\t%s",
                            "offHandIndex == WP_NONE || BG_GetWeaponDef( offHandIndex )->offhandClass != OFFHAND_CLASS_NONE",
                            v3) )
                __debugbreak();
        }
    }
    CG_GetLocalClientGlobals(localClientNum)->equippedOffHand = offHandIndex;
    CG_MenuShowNotify(localClientNum, 5);
}

//double __cdecl _Pow_int<float>(float _X, int _Y)
//{
//    float _Z; // [esp+4h] [ebp-8h]
//    unsigned int _N; // [esp+8h] [ebp-4h]
//
//    if ( _Y < 0 )
//        _N = -_Y;
//    else
//        _N = _Y;
//    _Z = 1.0f;
//    while ( 1 )
//    {
//        if ( (_N & 1) != 0 )
//            _Z = _Z * _X;
//        _N >>= 1;
//        if ( !_N )
//            break;
//        _X = _X * _X;
//    }
//    if ( _Y >= 0 )
//        return _Z;
//    else
//        return (float)(1.0 / _Z);
//}

