#include "cg_ammocounter.h"
#include <cgame_mp/cg_main_mp.h>
#include <bgame/bg_mantle.h>
#include <client_mp/cl_cgame_mp.h>
#include <bgame/bg_misc.h>
#include <bgame/bg_weapons_ammo.h>
#include <cgame_mp/cg_newDraw_mp.h>
#include "cg_weapons.h"
#include <qcommon/common.h>
#include <ui/ui_shared.h>
#include <client/splitscreen.h>
#include <ui/ui_atoms.h>

const dvar_t *ammoCounterHide;
const dvar_t *actionSlotsHide;
const dvar_t *lowAmmoWarningColor1;
const dvar_t *lowAmmoWarningColor2;
const dvar_t *lowAmmoWarningPulseFreq;
const dvar_t *lowAmmoWarningPulseMax;
const dvar_t *lowAmmoWarningPulseMin;
const dvar_t *lowAmmoWarningNoReloadColor1;
const dvar_t *lowAmmoWarningNoReloadColor2;
const dvar_t *lowAmmoWarningNoAmmoColor1;
const dvar_t *lowAmmoWarningNoAmmoColor2;
const dvar_t *cg_fuelHudVersion;
const dvar_t *ui_right_ammo_width;
const dvar_t *ui_ammo_stock_width;

static const float colorLowAmmo[4] =
{ 1.0, 0.3f, 0.3f, 1.0 };

void __cdecl CG_AmmoCounterRegisterDvars()
{
    ammoCounterHide = _Dvar_RegisterBool("ammoCounterHide", 0, 0x1000u, "Hide the Ammo Counter");
    actionSlotsHide = _Dvar_RegisterBool("actionSlotsHide", 0, 0x1000u, "Hide the actionslots.");
    lowAmmoWarningColor1 = _Dvar_RegisterColor(
                                                     "lowAmmoWarningColor1",
                                                     0.89999998,
                                                     0.89999998,
                                                     0.89999998,
                                                     0.80000001,
                                                     0,
                                                     "Color 1 of 2 to oscilate between");
    lowAmmoWarningColor2 = _Dvar_RegisterColor(
                                                     "lowAmmoWarningColor2",
                                                     1.0,
                                                     1.0,
                                                     1.0,
                                                     1.0,
                                                     0,
                                                     "Color 2 of 2 to oscilate between");
    lowAmmoWarningPulseFreq = _Dvar_RegisterFloat(
                                                            "lowAmmoWarningPulseFreq",
                                                            1.7,
                                                            0.0,
                                                            3.4028235e38,
                                                            0,
                                                            "Frequency of the pulse (oscilation between the 2 colors)");
    lowAmmoWarningPulseMax = _Dvar_RegisterFloat(
                                                         "lowAmmoWarningPulseMax",
                                                         1.5,
                                                         0.0,
                                                         3.4028235e38,
                                                         0,
                                                         "Min of oscilation range: 0 is color1 and 1.0 is color2.    Can be < 0, and the wave will clip at 0.");
    lowAmmoWarningPulseMin = _Dvar_RegisterFloat(
                                                         "lowAmmoWarningPulseMin",
                                                         0.0,
                                                         -3.4028235e38,
                                                         1.0,
                                                         0,
                                                         "Max of oscilation range: 0 is color1 and 1.0 is color2.    Can be > 1.0, and the wave will clip at 1.0.");
    lowAmmoWarningNoReloadColor1 = _Dvar_RegisterColor(
                                                                     "lowAmmoWarningNoReloadColor1",
                                                                     0.69999999,
                                                                     0.69999999,
                                                                     0.0,
                                                                     0.80000001,
                                                                     0,
                                                                     "Like lowAmmoWarningColor1, but when no ammo to reload with.");
    lowAmmoWarningNoReloadColor2 = _Dvar_RegisterColor(
                                                                     "lowAmmoWarningNoReloadColor2",
                                                                     1.0,
                                                                     1.0,
                                                                     0.0,
                                                                     1.0,
                                                                     0,
                                                                     "lowAmmoWarningColor2, but when no ammo to reload with.");
    lowAmmoWarningNoAmmoColor1 = _Dvar_RegisterColor(
                                                                 "lowAmmoWarningNoAmmoColor1",
                                                                 0.80000001,
                                                                 0.0,
                                                                 0.0,
                                                                 0.80000001,
                                                                 0,
                                                                 "Like lowAmmoWarningColor1, but when no ammo.");
    lowAmmoWarningNoAmmoColor2 = _Dvar_RegisterColor(
                                                                 "lowAmmoWarningNoAmmoColor2",
                                                                 1.0,
                                                                 0.0,
                                                                 0.0,
                                                                 1.0,
                                                                 0,
                                                                 "lowAmmoWarningColor2, but when no ammo.");
    cg_fuelHudVersion = _Dvar_RegisterInt(
                                                "cg_fuelHudVersion",
                                                1,
                                                0,
                                                3,
                                                0x80u,
                                                "Determines what version of the hud to show");
    ui_right_ammo_width = _Dvar_RegisterInt(
                                                    "ui_right_ammo_width",
                                                    0,
                                                    0,
                                                    0x7FFFFFFF,
                                                    0,
                                                    "Width of the ammo shown on the right.");
    ui_ammo_stock_width = _Dvar_RegisterInt(
                                                    "ui_ammo_stock_width",
                                                    0,
                                                    0,
                                                    0x7FFFFFFF,
                                                    0,
                                                    "Width of the ammo stock shown on the right.");
}

void __cdecl CG_DrawAmmoCounterSingle(int localClientNum, rectDef_s *rect, float *color, bool last)
{
    int i; // [esp+38h] [ebp-18h]
    cg_s *cgameGlob; // [esp+3Ch] [ebp-14h]
    int weapIdx; // [esp+40h] [ebp-10h]
    int numRounds; // [esp+44h] [ebp-Ch]
    const playerState_s *ps; // [esp+48h] [ebp-8h]
    float placementX; // [esp+4Ch] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    weapIdx = GetWeaponIndex(cgameGlob);
    ps = CG_GetPredictedPlayerState(localClientNum);
    numRounds = BG_GetAmmoInClip(ps, weapIdx);
    placementX = rect->x;
    for ( i = 0; i < numRounds; ++i )
    {
        if ( !last && i < numRounds - 1 || last && i == numRounds - 1 )
            CL_DrawStretchPic(
                &scrPlaceView[localClientNum],
                placementX,
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
                cgMedia.ammoCounterSingle);
        placementX = placementX - 16.0;
    }
}

unsigned int __cdecl GetWeaponIndex(const cg_s *cgameGlob)
{
    if ( cgameGlob->weaponSelect < BG_GetNumWeapons()
        && BG_PlayerHasWeapon(&cgameGlob->predictedPlayerState, cgameGlob->weaponSelect) )
    {
        return cgameGlob->weaponSelect;
    }
    else
    {
        return cgameGlob->predictedPlayerState.weapon;
    }
}

void __cdecl CG_DrawPlayerWeaponAmmoStock(
                int localClientNum,
                const rectDef_s *rect,
                Font_s *font,
                float scale,
                float *color,
                Material *material,
                int textStyle)
{
    unsigned int WeaponIndex; // eax
    int v8; // eax
    char str[64]; // [esp+30h] [ebp-68h] BYREF
    cg_s *cgameGlob; // [esp+74h] [ebp-24h]
    int ammoStock; // [esp+78h] [ebp-20h]
    int weapIndex; // [esp+80h] [ebp-18h]
    const playerState_s *ps; // [esp+84h] [ebp-14h]
    float colorMod[4]; // [esp+88h] [ebp-10h] BYREF

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    ps = &cgameGlob->predictedPlayerState;
    if ( (cgameGlob->predictedPlayerState.eFlags & 0x4000) == 0
        && ((ps->eFlags & 0x300) == 0 || ps->weaponstate == 35 || ps->weaponstate == 36 || ps->weaponstate == 37) )
    {
        colorMod[0] = *color;
        colorMod[1] = color[1];
        colorMod[2] = color[2];
        colorMod[3] = color[3];
        colorMod[3] = AmmoCounterFadeAlpha(localClientNum, cgameGlob) * colorMod[3];
        if ( colorMod[3] != 0.0 )
        {
            WeaponIndex = GetWeaponIndex(cgameGlob);
            weapIndex = ClipCounterWeapIdx(cgameGlob, WeaponIndex);
            if ( weapIndex )
            {
                if ( !BG_GetWeaponDef(weapIndex)->suppressAmmoReserveDisplay )
                {
                    ammoStock = BG_GetTotalAmmoReserve(ps, weapIndex);
                    if ( CG_CheckPlayerForLowAmmoSpecific(cgameGlob, weapIndex) )
                    {
                        colorMod[0] = colorLowAmmo[0];
                        colorMod[1] = colorLowAmmo[1];
                        colorMod[2] = colorLowAmmo[2];
                    }
                    Com_sprintf(str, 64, "/ %i", ammoStock);
                    UI_DrawText(
                        &scrPlaceView[localClientNum],
                        str,
                        0x7FFFFFFF,
                        font,
                        rect->x,
                        rect->y,
                        rect->horzAlign,
                        rect->vertAlign,
                        scale,
                        colorMod,
                        textStyle);
                    v8 = UI_TextWidth(str, 6, font, scale);
                    Dvar_SetIntIfChanged((dvar_s*)ui_ammo_stock_width, v8);
                }
            }
        }
    }
}

unsigned int __cdecl ClipCounterWeapIdx(const cg_s *cgameGlob, unsigned int weapIndex)
{
    const WeaponDef *weapDef; // [esp+0h] [ebp-4h]

    if ( !weapIndex )
        return 0;
    weapDef = BG_GetWeaponDef(weapIndex);
    if ( weapDef->ammoCounterClip == AMMO_COUNTER_CLIP_NONE && !weapDef->fuelTankWeapon )
        return 0;
    if ( weapDef->ammoCounterClip == AMMO_COUNTER_CLIP_ALTWEAPON )
        return GetWeaponAltIndex(cgameGlob, weapIndex);
    return weapIndex;
}

unsigned int __cdecl GetWeaponAltIndex(const cg_s *cgameGlob, unsigned int weaponIndex)
{
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // [esp-4h] [ebp-10h]
    const WeaponVariantDef *weapVariantDef; // [esp+0h] [ebp-Ch]
    const WeaponDef *weapDef; // [esp+8h] [ebp-4h]

    weapVariantDef = BG_GetWeaponVariantDef(weaponIndex);
    weapDef = BG_GetWeaponDef(weaponIndex);
    if ( !weapDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_ammocounter.cpp", 96, 0, "%s", "weapDef") )
    {
        __debugbreak();
    }
    if ( weapDef->inventoryType == WEAPINVENTORY_ALTMODE )
        return CG_GetAltWeapon(cgameGlob, weaponIndex);
    if ( weapVariantDef->altWeaponIndex )
    {
        if ( BG_GetWeaponDef(weapVariantDef->altWeaponIndex)->ammoCounterClip == AMMO_COUNTER_CLIP_ALTWEAPON )
        {
            v5 = BG_WeaponName(weapVariantDef->altWeaponIndex);
            v4 = BG_WeaponName(weaponIndex);
            Com_PrintWarning(
                17,
                "Weapon \"%s\" and it's altweapon \"%s\" both have their ammoCounterClip property set to \"AltWeapon\".\n",
                v4,
                v5);
            return 0;
        }
        else
        {
            return weapVariantDef->altWeaponIndex;
        }
    }
    else
    {
        v3 = BG_WeaponName(weaponIndex);
        Com_PrintWarning(
            17,
            "Weapon \"%s\" ammoCounterClip property is set to \"AltWeapon\", but it has no alternate weapon.\n",
            v3);
        return 0;
    }
}

double __cdecl AmmoCounterFadeAlpha(int localClientNum, cg_s *cgameGlob)
{
    if ( !cgameGlob
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_ammocounter.cpp", 157, 0, "%s", "cgameGlob") )
    {
        __debugbreak();
    }
    if ( (cgameGlob->predictedPlayerState.weapFlags & 0x80) != 0 )
        return 0.0;
    else
        return CG_FadeHudMenu(
                         localClientNum,
                         hud_fade_ammodisplay,
                         cgameGlob->ammoFadeTime,
                         SnapFloatToInt((float)(hud_fade_ammodisplay->current.value * 1000.0)));
}

void __cdecl CG_DrawFuelTankPercentage(
                int localClientNum,
                const rectDef_s *rect,
                Font_s *font,
                float scale,
                float *color,
                Material *material,
                int textStyle,
                float text_x,
                float text_y)
{
    unsigned int WeaponIndex; // eax
    char str[64]; // [esp+3Ch] [ebp-68h] BYREF
    cg_s *cgameGlob; // [esp+80h] [ebp-24h]
    int weapIndex; // [esp+84h] [ebp-20h]
    const playerState_s *ps; // [esp+88h] [ebp-1Ch]
    float colorMod[4]; // [esp+8Ch] [ebp-18h] BYREF
    const WeaponDef *weapDef; // [esp+9Ch] [ebp-8h]
    int fuelValue; // [esp+A0h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    ps = &cgameGlob->predictedPlayerState;
    if ( (cgameGlob->predictedPlayerState.eFlags & 0x4000) == 0
        && ((ps->eFlags & 0x300) == 0 || ps->weaponstate == 35 || ps->weaponstate == 36 || ps->weaponstate == 37) )
    {
        colorMod[0] = *color;
        colorMod[1] = color[1];
        colorMod[2] = color[2];
        colorMod[3] = color[3];
        colorMod[3] = AmmoCounterFadeAlpha(localClientNum, cgameGlob) * colorMod[3];
        if ( colorMod[3] != 0.0 )
        {
            WeaponIndex = GetWeaponIndex(cgameGlob);
            weapIndex = ClipCounterWeapIdx(cgameGlob, WeaponIndex);
            weapDef = BG_GetWeaponDef(weapIndex);
            if ( weapIndex )
            {
                if ( weapDef->fuelTankWeapon
                    && cg_fuelHudVersion->current.integer == 1
                    && !BG_GetWeaponDef(weapIndex)->suppressAmmoReserveDisplay
                    && cg_fuelHudVersion->current.integer == 1 )
                {
                    fuelValue = 100
                                        - (int)(float)((float)((float)BG_PlayerFuelTankTime(ps, weapIndex) / (float)weapDef->iTankLifeTime)
                                                                 * 100.0);
                    if ( fuelValue < 30 )
                    {
                        colorMod[0] = colorLowAmmo[0];
                        colorMod[1] = colorLowAmmo[1];
                        colorMod[2] = colorLowAmmo[2];
                    }
                    Com_sprintf(str, 0x40u, "%03i", fuelValue);
                    UI_DrawText(
                        &scrPlaceView[localClientNum],
                        str,
                        0x7FFFFFFF,
                        font,
                        rect->x + text_x,
                        rect->y + text_y,
                        rect->horzAlign,
                        rect->vertAlign,
                        scale,
                        colorMod,
                        textStyle);
                }
            }
        }
    }
}

void __cdecl CG_DrawPlayerWeaponAmmoClip(
                int localClientNum,
                const rectDef_s *rect,
                Font_s *font,
                float scale,
                float *color,
                Material *material,
                int textStyle,
                float text_x,
                float text_y,
                char textAlignMode)
{
    unsigned int WeaponIndex; // eax
    int v11; // eax
    char str[68]; // [esp+50h] [ebp-88h] BYREF
    const WeaponVariantDef *weapVarDef; // [esp+98h] [ebp-40h]
    cg_s *cgameGlob; // [esp+9Ch] [ebp-3Ch]
    int clipSize; // [esp+A0h] [ebp-38h]
    int weapIndex; // [esp+A4h] [ebp-34h]
    const playerState_s *ps; // [esp+A8h] [ebp-30h]
    float colorMod[4]; // [esp+ACh] [ebp-2Ch] BYREF
    int clipCnt; // [esp+BCh] [ebp-1Ch]
    rectDef_s textRect; // [esp+C0h] [ebp-18h] BYREF

    if ( !rect && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_ammocounter.cpp", 570, 0, "%s", "rect") )
        __debugbreak();
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    ps = &cgameGlob->predictedPlayerState;
    if ( (cgameGlob->predictedPlayerState.eFlags & 0x4000) == 0
        && ((ps->eFlags & 0x300) == 0 || ps->weaponstate == 35 || ps->weaponstate == 36 || ps->weaponstate == 37) )
    {
        colorMod[0] = *color;
        colorMod[1] = color[1];
        colorMod[2] = color[2];
        colorMod[3] = color[3];
        colorMod[3] = AmmoCounterFadeAlpha(localClientNum, cgameGlob) * colorMod[3];
        if ( colorMod[3] != 0.0 )
        {
            WeaponIndex = GetWeaponIndex(cgameGlob);
            weapIndex = ClipCounterWeapIdx(cgameGlob, WeaponIndex);
            if ( weapIndex )
            {
                if ( !BG_GetWeaponDef(weapIndex)->suppressAmmoReserveDisplay )
                {
                    weapVarDef = BG_GetWeaponVariantDef(weapIndex);
                    clipCnt = BG_GetAmmoInClipForWeaponDef(ps, weapVarDef);
                    clipSize = BG_GetClipSize(weapIndex);
                    AmmoColor(cgameGlob, colorMod, weapIndex);
                    if ( clipSize < 100 )
                    {
                        if ( clipSize < 10 )
                            Com_sprintf(str, 0x40u, "%01i", clipCnt);
                        else
                            Com_sprintf(str, 0x40u, "%02i", clipCnt);
                    }
                    else
                    {
                        Com_sprintf(str, 0x40u, "%03i", clipCnt);
                    }
                    v11 = UI_TextWidth(str, 3, font, scale);
                    Dvar_SetIntIfChanged((dvar_s *)ui_right_ammo_width, v11);
                    UI_DrawWrappedText(
                        &scrPlaceView[localClientNum],
                        str,
                        rect,
                        font,
                        rect->x + text_x,
                        rect->y + text_y,
                        scale,
                        colorMod,
                        textStyle,
                        textAlignMode,
                        &textRect);
                }
            }
        }
    }
}

float MYFLASHTERM = 40.0f;
void __cdecl AmmoColor(cg_s *cgameGlob, float *color, unsigned int weapIndex)
{
    float t;
    int idx;

    if (CG_CheckPlayerForLowClipSpecific(cgameGlob, weapIndex))
    {
        t = (float)(cgameGlob->time - cgameGlob->lastClipFlashTime) /
            (float)(MYFLASHTERM * M_PI);

        t = sinf(t);
        t = t * 0.5f + 0.5f;

        for (idx = 0; idx < 3; ++idx)
            color[idx] = (colorLowAmmo[idx] - color[idx]) * t + color[idx];
    }
}


void __cdecl CG_DrawPlayerWeaponAmmoClipDualWield(
                int localClientNum,
                const rectDef_s *rect,
                Font_s *font,
                float scale,
                float *color,
                Material *material,
                int textStyle)
{
    unsigned int WeaponIndex; // eax
    const WeaponVariantDef *WeaponVariantDef; // eax
    char str[68]; // [esp+50h] [ebp-70h] BYREF
    cg_s *cgameGlob; // [esp+98h] [ebp-28h]
    int clipSize; // [esp+9Ch] [ebp-24h]
    int weapIndex; // [esp+A0h] [ebp-20h]
    const playerState_s *ps; // [esp+A4h] [ebp-1Ch]
    const WeaponDef *weapDef; // [esp+A8h] [ebp-18h]
    float colorMod[4]; // [esp+ACh] [ebp-14h] BYREF
    int clipCnt; // [esp+BCh] [ebp-4h]

    if ( !rect && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_ammocounter.cpp", 636, 0, "%s", "rect") )
        __debugbreak();
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    ps = &cgameGlob->predictedPlayerState;
    if ( (cgameGlob->predictedPlayerState.eFlags & 0x4000) == 0
        && ((ps->eFlags & 0x300) == 0 || ps->weaponstate == 35 || ps->weaponstate == 36 || ps->weaponstate == 37) )
    {
        colorMod[0] = *color;
        colorMod[1] = color[1];
        colorMod[2] = color[2];
        colorMod[3] = color[3];
        colorMod[3] = AmmoCounterFadeAlpha(localClientNum, cgameGlob) * colorMod[3];
        if ( colorMod[3] != 0.0 )
        {
            WeaponIndex = GetWeaponIndex(cgameGlob);
            weapIndex = ClipCounterWeapIdx(cgameGlob, WeaponIndex);
            if ( weapIndex )
            {
                if ( !BG_GetWeaponDef(weapIndex)->suppressAmmoReserveDisplay )
                {
                    weapDef = BG_GetWeaponDef(weapIndex);
                    if ( weapDef->bDualWield )
                    {
                        if ( weapDef->dualWieldWeaponIndex )
                        {
                            WeaponVariantDef = BG_GetWeaponVariantDef(weapDef->dualWieldWeaponIndex);
                            clipCnt = BG_GetAmmoInClipForWeaponDef(ps, WeaponVariantDef);
                            clipSize = BG_GetClipSize(weapIndex);
                            AmmoColor(cgameGlob, colorMod, weapDef->dualWieldWeaponIndex);
                            if ( clipSize < 10 )
                                Com_sprintf(str, 64, "%01i", clipCnt);
                            else
                                Com_sprintf(str, 64, "%02i", clipCnt);
                            UI_DrawText(
                                &scrPlaceView[localClientNum],
                                str,
                                0x7FFFFFFF,
                                font,
                                rect->x,
                                rect->y,
                                rect->horzAlign,
                                rect->vertAlign,
                                scale,
                                colorMod,
                                textStyle);
                        }
                    }
                }
            }
        }
    }
}

float __cdecl CG_GetHudAlphaDPad(int localClientNum)
{
    cg_s *LocalClientGlobals; // eax

    LocalClientGlobals = CG_GetLocalClientGlobals(localClientNum);
    return DpadFadeAlpha(localClientNum, LocalClientGlobals);
}

double __cdecl DpadFadeAlpha(int localClientNum, cg_s *cgameGlob)
{
    unsigned int idx; // [esp+14h] [ebp-4h]

    if ( !cgameGlob
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_ammocounter.cpp", 878, 0, "%s", "cgameGlob") )
    {
        __debugbreak();
    }
    if ( (cgameGlob->predictedPlayerState.weapFlags & 0x80) != 0 )
        return 0.0;
    for ( idx = 0; idx < 4; ++idx )
    {
        if ( ActionSlotIsActive(localClientNum, idx) )
            return 1.0;
    }
    return AmmoCounterFadeAlpha(localClientNum, cgameGlob);
}

bool __cdecl ActionSlotIsActive(int localClientNum, unsigned int slotIdx)
{
    bool v3; // [esp+0h] [ebp-38h]
    ActionSlotType v4; // [esp+4h] [ebp-34h]
    cg_s *cgameGlob; // [esp+20h] [ebp-18h]
    unsigned int weapIdxa; // [esp+28h] [ebp-10h]
    unsigned int weapIdx; // [esp+28h] [ebp-10h]
    const playerState_s *ps; // [esp+2Ch] [ebp-Ch]
    const WeaponDef *weapDef; // [esp+30h] [ebp-8h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    ps = &cgameGlob->predictedPlayerState;
    v4 = cgameGlob->predictedPlayerState.actionSlotType[slotIdx];
    switch ( v4 )
    {
        case ACTIONSLOTTYPE_SPECIFYWEAPON:
            weapIdx = ps->actionSlotParam[slotIdx].specifyWeapon.index;
            if ( weapIdx )
            {
                if ( BG_PlayerHasWeapon(ps, weapIdx) )
                {
                    BG_GetWeaponVariantDef(weapIdx);
                    weapDef = BG_GetWeaponDef(weapIdx);
                    v3 = !BG_GetAmmoInClip(ps, weapIdx) && !BG_GetAmmoNotInClip(ps, weapIdx) && !weapDef->unlimitedAmmo;
                    return (weapDef->offhandSlot != OFFHAND_SLOT_EQUIPMENT || !v3 || cgameGlob->extraCamEntity != 1023)
                            && (!slotIdx || slotIdx == 3);
                }
                else
                {
                    return 0;
                }
            }
            else
            {
                return 0;
            }
        case ACTIONSLOTTYPE_ALTWEAPONTOGGLE:
            weapIdxa = cgameGlob->weaponSelect;
            BG_GetWeaponVariantDef(weapIdxa);
            return (BG_GetWeaponDef(weapIdxa)->inventoryType == WEAPINVENTORY_ALTMODE
                     || CG_AltWeaponToggleIndex(localClientNum, cgameGlob, 0))
                    && slotIdx == 2;
        case ACTIONSLOTTYPE_NIGHTVISION:
            return 0;
        default:
            if ( ps->actionSlotType[slotIdx] )
            {
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_ammocounter.cpp",
                                867,
                                0,
                                "%s",
                                "ps->actionSlotType[slotIdx] == ACTIONSLOTTYPE_DONOTHING") )
                    __debugbreak();
            }
            return 0;
    }
}

float __cdecl CG_GetHudAlphaAmmoCounter(int localClientNum)
{
    cg_s *LocalClientGlobals; // eax

    LocalClientGlobals = CG_GetLocalClientGlobals(localClientNum);
    return AmmoCounterFadeAlpha(localClientNum, LocalClientGlobals);
}

char __cdecl CG_ActionSlotIsUsable(int localClientNum, unsigned int slotIdx)
{
    ActionSlotType v3; // [esp+0h] [ebp-1Ch]
    cg_s *cgameGlob; // [esp+10h] [ebp-Ch]
    unsigned int weapIdx; // [esp+14h] [ebp-8h]
    const playerState_s *ps; // [esp+18h] [ebp-4h]

    if ( slotIdx > 3
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_ammocounter.cpp",
                    910,
                    0,
                    "slotIdx not in [0, (ACTIONSLOTS_NUM - 1)]\n\t%i not in [%i, %i]",
                    slotIdx,
                    0,
                    3) )
    {
        __debugbreak();
    }
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    ps = &cgameGlob->predictedPlayerState;
    v3 = cgameGlob->predictedPlayerState.actionSlotType[slotIdx];
    switch ( v3 )
    {
        case ACTIONSLOTTYPE_SPECIFYWEAPON:
            weapIdx = ps->actionSlotParam[slotIdx].specifyWeapon.index;
            if ( weapIdx && BG_PlayerHasWeapon(ps, weapIdx) )
                return 1;
            break;
        case ACTIONSLOTTYPE_ALTWEAPONTOGGLE:
            if ( CG_AltWeaponToggleIndex(localClientNum, cgameGlob, 0) )
                return 1;
            break;
        case ACTIONSLOTTYPE_NIGHTVISION:
            return 1;
        default:
            if ( ps->actionSlotType[slotIdx] )
            {
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_ammocounter.cpp",
                                930,
                                0,
                                "%s",
                                "ps->actionSlotType[slotIdx] == ACTIONSLOTTYPE_DONOTHING") )
                    __debugbreak();
            }
            break;
    }
    return 0;
}

void __cdecl CG_DrawPlayerActionSlotDpad(
                int localClientNum,
                const rectDef_s *rect,
                const float *color,
                Material *material)
{
    cg_s *LocalClientGlobals; // eax
    float colorMod[4]; // [esp+3Ch] [ebp-10h] BYREF

    if ( !rect && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_ammocounter.cpp", 944, 0, "%s", "rect") )
        __debugbreak();
    if ( CG_ActionSlotIsUsable(localClientNum, 0)
        || CG_ActionSlotIsUsable(localClientNum, 1u)
        || CG_ActionSlotIsUsable(localClientNum, 2u)
        || CG_ActionSlotIsUsable(localClientNum, 3u) )
    {
        LocalClientGlobals = CG_GetLocalClientGlobals(localClientNum);
        colorMod[0] = *color;
        colorMod[1] = color[1];
        colorMod[2] = color[2];
        colorMod[3] = color[3];
        colorMod[3] = DpadFadeAlpha(localClientNum, LocalClientGlobals) * colorMod[3];
        if ( colorMod[3] != 0.0 )
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
                colorMod,
                material);
    }
}

void __cdecl CG_DrawPlayerActionSlotBack(
                int localClientNum,
                const rectDef_s *rect,
                unsigned int slotIdx,
                const float *color,
                Material *material)
{
    cg_s *cgameGlob; // [esp+34h] [ebp-18h]
    float colorMod[4]; // [esp+3Ch] [ebp-10h] BYREF

    if ( !rect && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_ammocounter.cpp", 973, 0, "%s", "rect") )
        __debugbreak();
    if ( CG_ActionSlotIsUsable(localClientNum, slotIdx) )
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        colorMod[0] = *color;
        colorMod[1] = color[1];
        colorMod[2] = color[2];
        colorMod[3] = color[3];
        colorMod[3] = DpadFadeAlpha(localClientNum, cgameGlob) * colorMod[3];
        if ( colorMod[3] != 0.0 )
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
                colorMod,
                material);
    }
}

void __cdecl CG_DrawPlayerActionSlotArrow(
                int localClientNum,
                const rectDef_s *rect,
                unsigned int slotIdx,
                float *color,
                Material *material)
{
    cg_s *cgameGlob; // [esp+48h] [ebp-14h]
    float colorMod[4]; // [esp+4Ch] [ebp-10h] BYREF

    if ( !rect && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_ammocounter.cpp", 998, 0, "%s", "rect") )
        __debugbreak();
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    colorMod[0] = *color;
    colorMod[1] = color[1];
    colorMod[2] = color[2];
    colorMod[3] = color[3];
    if ( ActionSlotIsActive(localClientNum, slotIdx) )
        colorMod[3] = 0.8f;
    colorMod[3] = CG_FadeHudMenu(
                                    localClientNum,
                                    hud_fade_ammodisplay,
                                    cgameGlob->ammoFadeTime,
                                    SnapFloatToInt((float)(hud_fade_ammodisplay->current.value * 1000.0)))
                            * colorMod[3];
    if ( colorMod[3] != 0.0 )
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
            colorMod,
            material);
}

void __cdecl CG_DrawPlayerActionSlot(
                int localClientNum,
                const rectDef_s *rect,
                unsigned int slotIdx,
                float *color,
                Font_s *textFont,
                float textScale,
                int textStyle)
{
    float v7; // xmm0_4
    float v8; // xmm0_4
    bool v9; // [esp+30h] [ebp-114h]
    ActionSlotType v10; // [esp+34h] [ebp-110h]
    char str[68]; // [esp+8Ch] [ebp-B8h] BYREF
    int ammo; // [esp+D4h] [ebp-70h]
    float circleX; // [esp+D8h] [ebp-6Ch]
    float ammoAlpha; // [esp+DCh] [ebp-68h]
    float topHighlightSize; // [esp+E0h] [ebp-64h]
    cg_s *cgameGlob; // [esp+E4h] [ebp-60h]
    float circleSize; // [esp+E8h] [ebp-5Ch]
    float leftHighlightX; // [esp+ECh] [ebp-58h]
    float circleY; // [esp+F0h] [ebp-54h]
    const WeaponVariantDef *weapVariantDef; // [esp+F4h] [ebp-50h]
    float highlightSize; // [esp+F8h] [ebp-4Ch]
    float leftHighlightOffset; // [esp+FCh] [ebp-48h]
    unsigned int weapIdx; // [esp+100h] [ebp-44h]
    float black[4]; // [esp+104h] [ebp-40h] BYREF
    const playerState_s *ps; // [esp+114h] [ebp-30h]
    float colorMod[4]; // [esp+118h] [ebp-2Ch] BYREF
    float x; // [esp+128h] [ebp-1Ch] BYREF
    float y; // [esp+12Ch] [ebp-18h] BYREF
    const WeaponDef *weapDef; // [esp+130h] [ebp-14h]
    float leftHighlightY; // [esp+134h] [ebp-10h]
    float h; // [esp+138h] [ebp-Ch] BYREF
    float w; // [esp+13Ch] [ebp-8h] BYREF
    bool new_weapon_has_no_ammo; // [esp+143h] [ebp-1h]

    highlightSize = 94.0f;
    topHighlightSize = 96.0f;
    leftHighlightOffset = 2.0f;
    circleSize = 11.0f;
    circleX = 0.0f;
    circleY = 0.0f;
    ammoAlpha = 0.5f;
    memset(black, 0, 12);
    black[3] = 0.80000001f;
    if (!rect && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_ammocounter.cpp", 1045, 0, "%s", "rect"))
        __debugbreak();
    if (slotIdx > 3
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_ammocounter.cpp",
            1046,
            0,
            "slotIdx not in [0, (ACTIONSLOTS_NUM - 1)]\n\t%i not in [%i, %i]",
            slotIdx,
            0,
            3))
    {
        __debugbreak();
    }
    if (CL_LocalClient_GetActiveCount() > 1)
    {
        highlightSize = highlightSize * 2.0;
        topHighlightSize = topHighlightSize * 2.0;
        leftHighlightOffset = leftHighlightOffset * 2.0;
        circleSize = circleSize * 2.0;
    }
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    ps = &cgameGlob->predictedPlayerState;
    colorMod[0] = *color;
    colorMod[1] = color[1];
    colorMod[2] = color[2];
    colorMod[3] = color[3];
    ammoAlpha = AmmoCounterFadeAlpha(localClientNum, cgameGlob) * ammoAlpha;
    black[3] = AmmoCounterFadeAlpha(localClientNum, cgameGlob) * black[3];
    colorMod[3] = AmmoCounterFadeAlpha(localClientNum, cgameGlob) * colorMod[3];
    if (colorMod[3] != 0.0)
    {
        v10 = ps->actionSlotType[slotIdx];
        if (v10 == ACTIONSLOTTYPE_SPECIFYWEAPON)
        {
            weapIdx = ps->actionSlotParam[slotIdx].specifyWeapon.index;
            if (weapIdx && BG_PlayerHasWeapon(ps, weapIdx))
            {
                weapVariantDef = BG_GetWeaponVariantDef(weapIdx);
                weapDef = BG_GetWeaponDef(weapIdx);
                v9 = !BG_GetAmmoInClip(ps, weapIdx) && !BG_GetAmmoNotInClip(ps, weapIdx) && !weapDef->unlimitedAmmo;
                new_weapon_has_no_ammo = v9;
                if (weapDef->offhandSlot == OFFHAND_SLOT_EQUIPMENT && new_weapon_has_no_ammo)
                {
                    if (cgameGlob->extraCamEntity == 1023 && I_strcmp(weapVariantDef->szInternalName, "satchel_charge_mp"))
                        return;
                    colorMod[3] = colorMod[3] * 0.5;
                }
                DpadIconDims(rect, slotIdx, weapVariantDef, &x, &y, &w, &h);
                if (slotIdx)
                {
                    if (slotIdx == 3)
                    {
                        leftHighlightX = (float)(x - (float)((float)(highlightSize - w) / 2.0)) - leftHighlightOffset;
                        leftHighlightY = y - (float)((float)(highlightSize - h) / 2.0);
                        UI_DrawHandlePicRotated(
                            &scrPlaceView[localClientNum],
                            leftHighlightX,
                            leftHighlightY,
                            highlightSize,
                            highlightSize,
                            rect->horzAlign,
                            rect->vertAlign,
                            colorMod,
                            180.0,
                            cgMedia.hudDpadLeftHighlight);
                    }
                }
                else
                {
                    leftHighlightX = (float)(x - (float)((float)(topHighlightSize - w) / 2.0))
                        - (float)(leftHighlightOffset / 2.0);
                    leftHighlightY = (float)(y - (float)((float)(topHighlightSize - h) / 2.0)) + leftHighlightOffset;
                    UI_DrawHandlePicRotated(
                        &scrPlaceView[localClientNum],
                        leftHighlightX,
                        leftHighlightY,
                        topHighlightSize,
                        topHighlightSize,
                        rect->horzAlign,
                        rect->vertAlign,
                        colorMod,
                        90.0,
                        cgMedia.hudDpadLeftHighlight);
                }
                if (!I_strcmp(weapVariantDef->szInternalName, "minigun_drop_mp")
                    || !I_strcmp(weapVariantDef->szInternalName, "supplydrop_mp")
                    || !I_strcmp(weapVariantDef->szInternalName, "rcbomb_mp")
                    || !I_strcmp(weapVariantDef->szInternalName, "helicopter_gunner_mp"))
                {
                    v7 = w * 1.1;
                    w = v7;
                    v8 = h * 1.1;
                    h = v8;
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
                    1.0,
                    1.0,
                    colorMod,
                    weapVariantDef->dpadIcon);
                ammo = BG_WeaponAmmo(ps, weapIdx);
                if (!ammo)
                {
                    colorMod[0] = colorLowAmmo[0];
                    colorMod[1] = colorLowAmmo[1];
                    colorMod[2] = colorLowAmmo[2];
                }
                if (weapDef->iStartAmmo > 1 && !weapDef->bIsCarriedKillstreakWeapon)
                {
                    circleX = (float)((float)(rect->w / 2.0) + x) - (float)(circleSize / 2.0);
                    circleY = (float)((float)(rect->h / 2.0) + y) - (float)(circleSize / 2.0);
                    CL_DrawStretchPic(
                        &scrPlaceView[localClientNum],
                        circleX,
                        circleY,
                        circleSize,
                        circleSize,
                        rect->horzAlign,
                        rect->vertAlign,
                        0.0,
                        0.0,
                        1.0,
                        1.0,
                        black,
                        cgMedia.hudDpadCircle);
                    DpadTextPos(localClientNum, rect, slotIdx, weapVariantDef, &x, &y);
                    Com_sprintf(str, 0x40u, "%3i", ammo);
                    colorMod[3] = ammoAlpha;
                    UI_DrawText(
                        &scrPlaceView[localClientNum],
                        str,
                        0x7FFFFFFF,
                        textFont,
                        x,
                        y,
                        rect->horzAlign,
                        rect->vertAlign,
                        textScale,
                        colorMod,
                        textStyle);
                }
            }
        }
        else if (v10 == ACTIONSLOTTYPE_ALTWEAPONTOGGLE)
        {
            weapIdx = cgameGlob->weaponSelect;
            weapVariantDef = BG_GetWeaponVariantDef(weapIdx);
            weapDef = BG_GetWeaponDef(weapIdx);
            if (weapDef->inventoryType != WEAPINVENTORY_ALTMODE)
            {
                weapIdx = CG_AltWeaponToggleIndex(localClientNum, cgameGlob, 0);
                if (!weapIdx)
                    return;
                weapVariantDef = BG_GetWeaponVariantDef(weapIdx);
            }
            DpadIconDims(rect, slotIdx, weapVariantDef, &x, &y, &w, &h);
            if (slotIdx == 2)
            {
                leftHighlightX = (float)(x - (float)((float)(highlightSize - w) / 2.0)) + leftHighlightOffset;
                leftHighlightY = (float)(y - (float)((float)(highlightSize - h) / 2.0)) + leftHighlightOffset;
                UI_DrawHandlePicRotated(
                    &scrPlaceView[localClientNum],
                    leftHighlightX,
                    leftHighlightY,
                    highlightSize,
                    highlightSize,
                    rect->horzAlign,
                    rect->vertAlign,
                    colorMod,
                    0.0,
                    cgMedia.hudDpadLeftHighlight);
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
                1.0,
                1.0,
                colorMod,
                weapVariantDef->dpadIcon);
            circleX = (float)((float)(rect->w / 2.0) + x) - (float)(circleSize / 2.0);
            circleY = (float)((float)(rect->h / 2.0) + y) - (float)(circleSize / 2.0);
            CL_DrawStretchPic(
                &scrPlaceView[localClientNum],
                circleX,
                circleY,
                circleSize,
                circleSize,
                rect->horzAlign,
                rect->vertAlign,
                0.0,
                0.0,
                1.0,
                1.0,
                black,
                cgMedia.hudDpadCircle);
            ammo = BG_WeaponAmmo(ps, weapIdx);
            if (!ammo)
            {
                colorMod[0] = colorLowAmmo[0];
                colorMod[1] = colorLowAmmo[1];
                colorMod[2] = colorLowAmmo[2];
            }
            DpadTextPos(localClientNum, rect, slotIdx, weapVariantDef, &x, &y);
            Com_sprintf(str, 0x40u, "%3i", ammo);
            colorMod[3] = ammoAlpha;
            UI_DrawText(
                &scrPlaceView[localClientNum],
                str,
                0x7FFFFFFF,
                textFont,
                x,
                y,
                rect->horzAlign,
                rect->vertAlign,
                textScale,
                colorMod,
                textStyle);
        }
        else if (v10 != ACTIONSLOTTYPE_NIGHTVISION
            && ps->actionSlotType[slotIdx]
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_ammocounter.cpp",
                1225,
                0,
                "%s",
                "ps->actionSlotType[slotIdx] == ACTIONSLOTTYPE_DONOTHING"))
        {
            __debugbreak();
        }
    }
}

void __cdecl DpadIconDims(
                const rectDef_s *rect,
                unsigned int slotIdx,
                const WeaponVariantDef *weapVariantDef,
                float *x,
                float *y,
                float *w,
                float *h)
{
    if ( !rect && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_ammocounter.cpp", 694, 0, "%s", "rect") )
        __debugbreak();
    if ( !weapVariantDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_ammocounter.cpp", 695, 0, "%s", "weapVariantDef") )
    {
        __debugbreak();
    }
    if ( !x && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_ammocounter.cpp", 696, 0, "%s", "x") )
        __debugbreak();
    if ( !y && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_ammocounter.cpp", 697, 0, "%s", "y") )
        __debugbreak();
    if ( !w && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_ammocounter.cpp", 698, 0, "%s", "w") )
        __debugbreak();
    if ( !h && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_ammocounter.cpp", 699, 0, "%s", "h") )
        __debugbreak();
    if ( weapVariantDef->dpadIconRatio == WEAPON_ICON_RATIO_4TO1 )
    {
        if ( slotIdx <= 1 )
        {
            *x = rect->x - (float)(rect->w * 0.5);
            *w = rect->w * 2.0;
        }
        else if ( slotIdx == 2 )
        {
            *x = rect->x - rect->w;
            *w = rect->w * 2.0;
        }
        else
        {
            if ( slotIdx != 3
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_ammocounter.cpp",
                            715,
                            0,
                            "%s",
                            "slotIdx == 3") )
            {
                __debugbreak();
            }
            *x = rect->x;
            *w = rect->w * 2.0;
        }
        *y = (float)(rect->h * 0.25) + rect->y;
        *h = rect->h * 0.5;
    }
    else if ( weapVariantDef->dpadIconRatio == WEAPON_ICON_RATIO_2TO1 )
    {
        if ( slotIdx <= 1 )
        {
            *x = rect->x - (float)(rect->w * 0.5);
            *w = rect->w * 2.0;
        }
        else if ( slotIdx == 2 )
        {
            *x = rect->x - rect->w;
            *w = rect->w * 2.0;
        }
        else
        {
            if ( slotIdx != 3
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_ammocounter.cpp",
                            736,
                            0,
                            "%s",
                            "slotIdx == 3") )
            {
                __debugbreak();
            }
            *x = rect->x;
            *w = rect->w * 2.0;
        }
        *y = rect->y;
        *h = rect->h;
    }
    else
    {
        *x = rect->x;
        *y = rect->y;
        *w = rect->w;
        *h = rect->h;
    }
}

void __cdecl DpadTextPos(
                int localClientNum,
                const rectDef_s *rect,
                unsigned int slotIdx,
                const WeaponVariantDef *weapVariantDef,
                float *x,
                float *y)
{
    float MYADJUST_Y; // [esp+0h] [ebp-10h]
    float MYADJUST_X_INDEX_2; // [esp+4h] [ebp-Ch]
    float slot_adjust_x; // [esp+8h] [ebp-8h]
    float MYADJUST_X; // [esp+Ch] [ebp-4h]

    MYADJUST_X = -7.0f;
    MYADJUST_X_INDEX_2 = -0.5f;
    MYADJUST_Y = 5.0f;
    if ( !rect && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_ammocounter.cpp", 759, 0, "%s", "rect") )
        __debugbreak();
    if ( !weapVariantDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_ammocounter.cpp", 760, 0, "%s", "weapVariantDef") )
    {
        __debugbreak();
    }
    if ( !x && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_ammocounter.cpp", 761, 0, "%s", "x") )
        __debugbreak();
    if ( !y && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_ammocounter.cpp", 762, 0, "%s", "y") )
        __debugbreak();
    if ( CL_LocalClient_GetActiveCount() > 1 )
    {
        MYADJUST_X = -7.0 * 2.0;
        MYADJUST_X_INDEX_2 = -0.5 * 2.0;
        MYADJUST_Y = (float)(5.0 * 2.0) + 1.0;
    }
    slot_adjust_x = 0.0f;
    if ( slotIdx == 2 )
        slot_adjust_x = MYADJUST_X_INDEX_2;
    *x = (float)((float)((float)(rect->w / 2.0) + rect->x) + MYADJUST_X) + slot_adjust_x;
    *y = (float)((float)(rect->h / 2.0) + rect->y) + MYADJUST_Y;
    if ( weapVariantDef->dpadIconRatio == WEAPON_ICON_RATIO_2TO1
        || weapVariantDef->dpadIconRatio == WEAPON_ICON_RATIO_4TO1 )
    {
        *x = (float)(rect->w * 0.5) + *x;
        if ( slotIdx == 2 )
            *x = *x - rect->w;
    }
}

void __cdecl CG_DrawPlayerActionSlotBind(
                int localClientNum,
                const rectDef_s *rect,
                unsigned int slotIdx,
                float *color,
                Font_s *textFont,
                float textScale,
                int textStyle,
                int textAlignMode,
                itemDef_s *item,
                char *text)
{
    cg_s *cgameGlob; // [esp+30h] [ebp-18h]
    float colorMod[4]; // [esp+38h] [ebp-10h] BYREF

    if ( !rect && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_ammocounter.cpp", 1236, 0, "%s", "rect") )
        __debugbreak();
    if ( slotIdx > 3
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_ammocounter.cpp",
                    1237,
                    0,
                    "slotIdx not in [0, (ACTIONSLOTS_NUM - 1)]\n\t%i not in [%i, %i]",
                    slotIdx,
                    0,
                    3) )
    {
        __debugbreak();
    }
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    colorMod[0] = *color;
    colorMod[1] = color[1];
    colorMod[2] = color[2];
    colorMod[3] = color[3];
    if ( ActionSlotIsActive(localClientNum, slotIdx) )
    {
        colorMod[0] = colorMod[0] * 1.0;
        colorMod[1] = colorMod[1] * 0.80000001;
        colorMod[2] = colorMod[2] * 0.40000001;
        colorMod[3] = 0.8f;
    }
    colorMod[3] = CG_FadeHudMenu(
                                    localClientNum,
                                    hud_fade_ammodisplay,
                                    cgameGlob->ammoFadeTime,
                                    SnapFloatToInt((float)(hud_fade_ammodisplay->current.value * 1000.0)))
                            * colorMod[3];
    if ( colorMod[3] != 0.0 )
        UI_DrawText(
            &scrPlaceView[localClientNum],
            text,
            0x7FFFFFFF,
            textFont,
            rect->x,
            rect->y,
            rect->horzAlign,
            rect->vertAlign,
            textScale,
            colorMod,
            textStyle);
}

void __cdecl CG_DrawPlayerWeaponBackground(
                int localClientNum,
                const rectDef_s *rect,
                const float *color,
                Material *material)
{
    cg_s *cgameGlob; // [esp+48h] [ebp-14h]
    float colorMod[4]; // [esp+4Ch] [ebp-10h] BYREF

    if ( !rect && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_ammocounter.cpp", 1265, 0, "%s", "rect") )
        __debugbreak();
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    colorMod[0] = *color;
    colorMod[1] = color[1];
    colorMod[2] = color[2];
    colorMod[3] = color[3];
    colorMod[3] = CG_FadeHudMenu(
                                    localClientNum,
                                    hud_fade_ammodisplay,
                                    cgameGlob->ammoFadeTime,
                                    SnapFloatToInt((float)(hud_fade_ammodisplay->current.value * 1000.0)))
                            * colorMod[3];
    if ( colorMod[3] != 0.0 )
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
            colorMod,
            material);
}

void __cdecl CG_DrawPlayerWeaponAmmoClipGraphic(int localClientNum, const rectDef_s *rect, const float *color)
{
    const WeaponVariantDef *weapVarDef; // [esp+14h] [ebp-28h]
    cg_s *cgameGlob; // [esp+18h] [ebp-24h]
    int weapIdx; // [esp+1Ch] [ebp-20h]
    float base[2]; // [esp+20h] [ebp-1Ch] BYREF
    float colorMod[4]; // [esp+28h] [ebp-14h] BYREF
    const WeaponDef *weapDef; // [esp+38h] [ebp-4h]

    if ( !rect && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_ammocounter.cpp", 1319, 0, "%s", "rect") )
        __debugbreak();
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    colorMod[0] = *color;
    colorMod[1] = color[1];
    colorMod[2] = color[2];
    colorMod[3] = color[3];
    colorMod[3] = AmmoCounterFadeAlpha(localClientNum, cgameGlob) * colorMod[3];
    if ( colorMod[3] != 0.0
        && ((cgameGlob->predictedPlayerState.eFlags & 0x300) == 0
         || cgameGlob->predictedPlayerState.weaponstate == 35
         || cgameGlob->predictedPlayerState.weaponstate == 36
         || cgameGlob->predictedPlayerState.weaponstate == 37)
        && (cgameGlob->predictedPlayerState.eFlags & 0x4000) == 0 )
    {
        weapIdx = GetWeaponIndex(cgameGlob);
        if ( weapIdx )
        {
            weapVarDef = BG_GetWeaponVariantDef(weapIdx);
            weapDef = BG_GetWeaponDef(weapIdx);
            GetBaseRectPos(localClientNum, rect, base);
            DrawClipAmmo(cgameGlob, base, weapIdx, weapDef, weapVarDef, colorMod);
        }
    }
}

void __cdecl GetBaseRectPos(int localClientNum, const rectDef_s *rect, float *base)
{
    float dummyW; // [esp+0h] [ebp-8h] BYREF
    float dummyH; // [esp+4h] [ebp-4h] BYREF

    if ( !rect && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_ammocounter.cpp", 75, 0, "%s", "rect") )
        __debugbreak();
    *base = rect->x;
    base[1] = rect->y;
    ScrPlace_ApplyRect(&scrPlaceView[localClientNum], base, base + 1, &dummyW, &dummyH, rect->horzAlign, rect->vertAlign);
}

void __cdecl DrawClipAmmo(
                cg_s *cgameGlob,
                float *base,
                unsigned int weapIdx,
                const WeaponDef *weapDef,
                const WeaponVariantDef *weapVarDef,
                float *color)
{
    const WeaponVariantDef *weapVarDefAlt; // [esp+4h] [ebp-Ch]
    const WeaponDef *weapDefAlt; // [esp+8h] [ebp-8h]
    int weapIdxAlt; // [esp+Ch] [ebp-4h]

    if ( !cgameGlob
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_ammocounter.cpp", 385, 0, "%s", "cgameGlob") )
    {
        __debugbreak();
    }
    if ( !weapDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_ammocounter.cpp", 386, 0, "%s", "weapDef") )
    {
        __debugbreak();
    }
    if ( !weapVarDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_ammocounter.cpp", 387, 0, "%s", "weapVarDef") )
    {
        __debugbreak();
    }
    switch ( weapDef->ammoCounterClip )
    {
        case AMMO_COUNTER_CLIP_MAGAZINE:
            DrawClipAmmoMagazine(cgameGlob, base, weapIdx, weapVarDef, color);
            break;
        case AMMO_COUNTER_CLIP_SHORTMAGAZINE:
            DrawClipAmmoShortMagazine(cgameGlob, base, weapIdx, weapVarDef, color);
            break;
        case AMMO_COUNTER_CLIP_SHOTGUN:
            DrawClipAmmoShotgunShells(cgameGlob, base, weapIdx, weapVarDef, color);
            break;
        case AMMO_COUNTER_CLIP_ROCKET:
            DrawClipAmmoRockets(cgameGlob, base, weapIdx, weapVarDef, color);
            break;
        case AMMO_COUNTER_CLIP_BELTFED:
            DrawClipAmmoBeltfed(cgameGlob, base, weapIdx, weapDef, weapVarDef, color);
            break;
        case AMMO_COUNTER_CLIP_ALTWEAPON:
            weapIdxAlt = GetWeaponAltIndex(cgameGlob, weapIdx);
            if ( weapIdxAlt )
            {
                weapDefAlt = BG_GetWeaponDef(weapIdxAlt);
                weapVarDefAlt = BG_GetWeaponVariantDef(weapIdxAlt);
                DrawClipAmmo(cgameGlob, base, weapIdxAlt, weapDefAlt, weapVarDefAlt, color);
            }
            break;
        default:
            if ( weapDef->ammoCounterClip
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_ammocounter.cpp",
                            418,
                            0,
                            "%s",
                            "weapDef->ammoCounterClip == AMMO_COUNTER_CLIP_NONE") )
            {
                __debugbreak();
            }
            break;
    }
}

const float colorSpentRound[4] = { 0.30000001, 0.30000001, 0.30000001, 1.0 };

void __cdecl DrawClipAmmoMagazine(
                cg_s *cgameGlob,
                const float *base,
                unsigned int weapIdx,
                const WeaponVariantDef *weapVarDef,
                float *color)
{
    int clipSize; // [esp+54h] [ebp-18h]
    float bulletX; // [esp+58h] [ebp-14h]
    float bulletY; // [esp+5Ch] [ebp-10h]
    int clipIdx; // [esp+60h] [ebp-Ch]
    int clipCnt; // [esp+68h] [ebp-4h]

    if ( !cgameGlob
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_ammocounter.cpp", 186, 0, "%s", "cgameGlob") )
    {
        __debugbreak();
    }
    if ( !weapVarDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_ammocounter.cpp", 187, 0, "%s", "weapVarDef") )
    {
        __debugbreak();
    }
    bulletX = *base - 4.0;
    bulletY = base[1] - (float)(8.0 * 0.5);
    clipCnt = BG_GetAmmoInClipForWeaponDef(&cgameGlob->predictedPlayerState, weapVarDef);
    AmmoColor(cgameGlob, color, weapIdx);
    clipSize = BG_GetClipSize(weapIdx);
    for ( clipIdx = 0; clipIdx < clipSize; ++clipIdx )
    {
        if ( clipIdx == clipCnt )
        {
            *color = colorSpentRound[0];
            color[1] = colorSpentRound[1];
            color[2] = colorSpentRound[2];
        }
        CL_DrawStretchPicPhysical(bulletX, bulletY, 4.0, 8.0, 0.0, 0.0, 1.0, 1.0, color, cgMedia.ammoCounterBullet);
        bulletX = bulletX - 4.0;
    }
}

void __cdecl DrawClipAmmoShortMagazine(
                cg_s *cgameGlob,
                const float *base,
                unsigned int weapIdx,
                const WeaponVariantDef *weapVarDef,
                float *color)
{
    int clipSize; // [esp+54h] [ebp-18h]
    float bulletX; // [esp+58h] [ebp-14h]
    float bulletY; // [esp+5Ch] [ebp-10h]
    int clipIdx; // [esp+60h] [ebp-Ch]
    int clipCnt; // [esp+68h] [ebp-4h]

    if ( !cgameGlob
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_ammocounter.cpp", 218, 0, "%s", "cgameGlob") )
    {
        __debugbreak();
    }
    if ( !weapVarDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_ammocounter.cpp", 219, 0, "%s", "weapVarDef") )
    {
        __debugbreak();
    }
    bulletX = *base - 32.0;
    bulletY = base[1] - (float)(8.0 * 0.5);
    clipCnt = BG_GetAmmoInClipForWeaponDef(&cgameGlob->predictedPlayerState, weapVarDef);
    AmmoColor(cgameGlob, color, weapIdx);
    clipSize = BG_GetClipSize(weapIdx);
    for ( clipIdx = 0; clipIdx < clipSize; ++clipIdx )
    {
        if ( clipIdx == clipCnt )
        {
            *color = colorSpentRound[0];
            color[1] = colorSpentRound[1];
            color[2] = colorSpentRound[2];
        }
        CL_DrawStretchPicPhysical(bulletX, bulletY, 32.0, 8.0, 0.0, 0.0, 1.0, 1.0, color, cgMedia.ammoCounterRifleBullet);
        bulletX = bulletX - 40.0;
    }
}

float TEST_bullet_wh_1[2] = { 16.0, 8.0 };
float TEST_bullet_step_1[2] = { 20.0, 12.0 };

void __cdecl DrawClipAmmoShotgunShells(
    cg_s *cgameGlob,
    const float *base,
    unsigned int weapIdx,
    const WeaponVariantDef *weapVarDef,
    float *color)
{
    int magCnt; // [esp+54h] [ebp-18h]
    int clipSize; // [esp+58h] [ebp-14h]
    float bulletX; // [esp+5Ch] [ebp-10h]
    float bulletY; // [esp+60h] [ebp-Ch]
    int magIdx; // [esp+68h] [ebp-4h]

    if (!cgameGlob
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_ammocounter.cpp", 250, 0, "%s", "cgameGlob"))
    {
        __debugbreak();
    }
    if (!weapVarDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_ammocounter.cpp", 251, 0, "%s", "weapVarDef"))
    {
        __debugbreak();
    }
    bulletX = *base - TEST_bullet_wh_1[0];
    bulletY = base[1] - (float)(TEST_bullet_wh_1[1] * 0.5);
    magCnt = BG_GetAmmoInClipForWeaponDef(&cgameGlob->predictedPlayerState, weapVarDef);
    AmmoColor(cgameGlob, color, weapIdx);
    clipSize = BG_GetClipSize(weapIdx);
    for (magIdx = 0; magIdx < clipSize; ++magIdx)
    {
        if (magIdx == magCnt)
        {
            *color = colorSpentRound[0];
            color[1] = colorSpentRound[1];
            color[2] = colorSpentRound[2];
        }
        CL_DrawStretchPicPhysical(
            bulletX,
            bulletY,
            TEST_bullet_wh_1[0],
            TEST_bullet_wh_1[1],
            0.0,
            0.0,
            1.0,
            1.0,
            color,
            cgMedia.ammoCounterShotgunShell);
        bulletX = bulletX - TEST_bullet_step_1[0];
    }
}

float TEST_bullet_wh_2[2] =
{ 64.0, 16.0 };
float TEST_bullet_wh_3[2] =
{ 8.0, 4.0 };
float TEST_bullet_step_2[2] =
{ 72.0, 12.0 };

void __cdecl DrawClipAmmoRockets(
    cg_s *cgameGlob,
    const float *base,
    unsigned int weapIdx,
    const WeaponVariantDef *weapVarDef,
    float *color)
{
    int magCnt; // [esp+54h] [ebp-18h]
    int clipSize; // [esp+58h] [ebp-14h]
    float bulletX; // [esp+5Ch] [ebp-10h]
    float bulletY; // [esp+60h] [ebp-Ch]
    int magIdx; // [esp+68h] [ebp-4h]

    if (!cgameGlob
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_ammocounter.cpp", 282, 0, "%s", "cgameGlob"))
    {
        __debugbreak();
    }
    if (!weapVarDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_ammocounter.cpp", 283, 0, "%s", "weapVarDef"))
    {
        __debugbreak();
    }
    bulletX = *base - TEST_bullet_wh_2[0];
    bulletY = base[1] - (float)(TEST_bullet_wh_2[1] * 0.5);
    magCnt = BG_GetAmmoInClipForWeaponDef(&cgameGlob->predictedPlayerState, weapVarDef);
    AmmoColor(cgameGlob, color, weapIdx);
    clipSize = BG_GetClipSize(weapIdx);
    for (magIdx = 0; magIdx < clipSize; ++magIdx)
    {
        if (magIdx == magCnt)
        {
            *color = colorSpentRound[0];
            color[1] = colorSpentRound[1];
            color[2] = colorSpentRound[2];
        }
        CL_DrawStretchPicPhysical(
            bulletX,
            bulletY,
            TEST_bullet_wh_2[0],
            TEST_bullet_wh_2[1],
            0.0,
            0.0,
            1.0,
            1.0,
            color,
            cgMedia.ammoCounterRocket);
        bulletX = bulletX - TEST_bullet_step_2[0];
    }
}

float TEST_bullet_step_3[2] =
{ 8.0, -6.0 };
int TEST_bullet_rowCnt =
20;


void __cdecl DrawClipAmmoBeltfed(
    cg_s *cgameGlob,
    float *base,
    unsigned int weapIdx,
    const WeaponDef *weapDef,
    const WeaponVariantDef *weapVarDef,
    float *color)
{
    float v6; // [esp+28h] [ebp-ACh]
    int ClipSize; // [esp+9Ch] [ebp-38h]
    float stepX; // [esp+A0h] [ebp-34h]
    float x; // [esp+A4h] [ebp-30h]
    float y; // [esp+A8h] [ebp-2Ch]
    int i; // [esp+ACh] [ebp-28h]
    int AmmoInClipForWeaponDef; // [esp+B4h] [ebp-20h]
    int percent; // [esp+B8h] [ebp-1Ch]
    float bulletX; // [esp+C0h] [ebp-14h]
    float bulletY; // [esp+C4h] [ebp-10h]
    int clipIdx; // [esp+C8h] [ebp-Ch]
    int clipCnt; // [esp+D0h] [ebp-4h]

    if (weapDef->fireType == WEAPON_FIRETYPE_MINIGUN)
    {
        if (!cgameGlob
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_ammocounter.cpp", 316, 0, "%s", "cgameGlob"))
        {
            __debugbreak();
        }
        if (!weapDef
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_ammocounter.cpp", 317, 0, "%s", "weapDef"))
        {
            __debugbreak();
        }
        bulletX = *base - 4.0;
        bulletY = base[1] - (float)(8.0 * 0.5);
        clipCnt = BG_GetAmmoInClipForWeaponDef(&cgameGlob->predictedPlayerState, weapVarDef);
        AmmoColor(cgameGlob, color, weapIdx);
        percent = (int)(float)((float)((float)clipCnt / (float)BG_GetClipSize(weapIdx)) * 20.0);
        for (clipIdx = 0; clipIdx < 20; ++clipIdx)
        {
            if (clipIdx >= percent)
            {
                *color = colorSpentRound[0];
                color[1] = colorSpentRound[1];
                color[2] = colorSpentRound[2];
            }
            CL_DrawStretchPicPhysical(bulletX, bulletY, 4.0, 8.0, 0.0, 0.0, 1.0, 1.0, color, cgMedia.ammoCounterBullet);
            bulletX = bulletX - 4.0;
        }
    }
    else
    {
        if (!cgameGlob
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_ammocounter.cpp", 351, 0, "%s", "cgameGlob"))
        {
            __debugbreak();
        }
        if (!weapDef
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_ammocounter.cpp", 352, 0, "%s", "weapDef"))
        {
            __debugbreak();
        }
        stepX = TEST_bullet_step_3[0];
        x = *base;
        v6 = TEST_bullet_wh_3[1] * 0.25;
        y = (float)(v6 * (float)(BG_GetClipSize(weapIdx) / TEST_bullet_rowCnt)) + base[1];
        AmmoInClipForWeaponDef = BG_GetAmmoInClipForWeaponDef(&cgameGlob->predictedPlayerState, weapVarDef);
        AmmoColor(cgameGlob, color, weapIdx);
        ClipSize = BG_GetClipSize(weapIdx);
        for (i = 0; i < ClipSize; ++i)
        {
            if (i == AmmoInClipForWeaponDef)
            {
                *color = colorSpentRound[0];
                color[1] = colorSpentRound[1];
                color[2] = colorSpentRound[2];
            }
            if (!(i % TEST_bullet_rowCnt))
            {
                stepX = stepX * -1.0;
                y = y + TEST_bullet_step_3[1];
                x = x + stepX;
            }
            CL_DrawStretchPicPhysical(
                x,
                y,
                TEST_bullet_wh_3[0],
                TEST_bullet_wh_3[1],
                0.0,
                0.0,
                1.0,
                1.0,
                color,
                cgMedia.ammoCounterBeltBullet);
            x = x + stepX;
        }
    }
}

void __cdecl CG_DrawPlayerWeaponAmmoClipGraphicDualWield(int localClientNum, const rectDef_s *rect, const float *color)
{
    const WeaponDef *wd; // eax
    const WeaponVariantDef *WeaponVariantDef; // [esp-8h] [ebp-40h]
    cg_s *cgameGlob; // [esp+14h] [ebp-24h]
    int weapIdx; // [esp+18h] [ebp-20h]
    float base[2]; // [esp+1Ch] [ebp-1Ch] BYREF
    float colorMod[4]; // [esp+24h] [ebp-14h] BYREF
    const WeaponDef *weapDef; // [esp+34h] [ebp-4h]

    if (!rect && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_ammocounter.cpp", 1355, 0, "%s", "rect"))
        __debugbreak();
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    colorMod[0] = *color;
    colorMod[1] = color[1];
    colorMod[2] = color[2];
    colorMod[3] = color[3];
    colorMod[3] = AmmoCounterFadeAlpha(localClientNum, cgameGlob) * colorMod[3];
    if (colorMod[3] != 0.0
        && ((cgameGlob->predictedPlayerState.eFlags & 0x300) == 0
            || cgameGlob->predictedPlayerState.weaponstate == 35
            || cgameGlob->predictedPlayerState.weaponstate == 36
            || cgameGlob->predictedPlayerState.weaponstate == 37)
        && (cgameGlob->predictedPlayerState.eFlags & 0x4000) == 0)
    {
        weapIdx = GetWeaponIndex(cgameGlob);
        if (weapIdx)
        {
            weapDef = BG_GetWeaponDef(weapIdx);
            if (weapDef->bDualWield)
            {
                if (weapDef->dualWieldWeaponIndex)
                {
                    GetBaseRectPos(localClientNum, rect, base);
                    WeaponVariantDef = BG_GetWeaponVariantDef(weapDef->dualWieldWeaponIndex);
                    wd = BG_GetWeaponDef(weapDef->dualWieldWeaponIndex);
                    DrawClipAmmo(cgameGlob, base, weapDef->dualWieldWeaponIndex, wd, WeaponVariantDef, colorMod);
                }
            }
        }
    }
}

void __cdecl CG_DrawPlayerWeaponIcon(int localClientNum, const rectDef_s *rect, const float *color)
{
    cg_s *cgameGlob; // [esp+14h] [ebp-1Ch]
    unsigned int weapIdx; // [esp+18h] [ebp-18h]
    float colorMod[4]; // [esp+1Ch] [ebp-14h] BYREF
    const WeaponDef *weapDef; // [esp+2Ch] [ebp-4h]

    if ( !rect && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_ammocounter.cpp", 1425, 0, "%s", "rect") )
        __debugbreak();
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    colorMod[0] = *color;
    colorMod[1] = color[1];
    colorMod[2] = color[2];
    colorMod[3] = color[3];
    colorMod[3] = AmmoCounterFadeAlpha(localClientNum, cgameGlob) * colorMod[3];
    if ( colorMod[3] != 0.0 )
    {
        weapIdx = GetWeaponIndex(cgameGlob);
        if ( weapIdx )
        {
            weapDef = BG_GetWeaponDef(weapIdx);
            DrawStretchPicGun(
                &scrPlaceView[localClientNum],
                rect,
                colorMod,
                weapDef->ammoCounterIcon,
                weapDef->ammoCounterIconRatio);
        }
    }
}

void __cdecl DrawStretchPicGun(
                const ScreenPlacement *scrPlace,
                const rectDef_s *rect,
                const float *color,
                Material *material,
                weaponIconRatioType_t ratio)
{
    float x; // [esp+2Ch] [ebp-10h] BYREF
    float y; // [esp+30h] [ebp-Ch] BYREF
    float h; // [esp+34h] [ebp-8h] BYREF
    float w; // [esp+38h] [ebp-4h] BYREF

    if ( !rect && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_ammocounter.cpp", 1394, 0, "%s", "rect") )
        __debugbreak();
    x = rect->x;
    y = rect->y;
    w = rect->w;
    h = rect->h;
    ScrPlace_ApplyRect(scrPlace, &x, &y, &w, &h, rect->horzAlign, rect->vertAlign);
    if ( ratio )
    {
        if ( ratio == WEAPON_ICON_RATIO_2TO1 )
        {
            x = x - w;
            w = w * 2.0;
        }
        else
        {
            if ( ratio != WEAPON_ICON_RATIO_4TO1
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_ammocounter.cpp",
                            1410,
                            0,
                            "%s\n\t(ratio) = %i",
                            "(ratio == WEAPON_ICON_RATIO_4TO1)",
                            ratio) )
            {
                __debugbreak();
            }
            x = x - (float)(w * 3.0);
            w = w * 4.0;
        }
    }
    CL_DrawStretchPicPhysical(x, y, w, h, 0.0, 0.0, 1.0, 1.0, color, material);
}

void __cdecl CG_DrawPlayerWeaponLowAmmoWarning(
                int localClientNum,
                const rectDef_s *rect,
                Font_s *font,
                float textScale,
                int textStyle,
                float text_x,
                float text_y,
                int textAlignMode,
                Material *material)
{
    double v9; // xmm0_8
    const char *v10; // eax
    const char *v11; // eax
    long double v12; // [esp+30h] [ebp-1BCh]
    bool canReloadLeft; // [esp+6Fh] [ebp-17Dh]
    float fade; // [esp+70h] [ebp-17Ch]
    cg_s *cgameGlob; // [esp+7Ch] [ebp-170h]
    float amplitude; // [esp+80h] [ebp-16Ch]
    bool canReload; // [esp+86h] [ebp-166h]
    bool empty; // [esp+87h] [ebp-165h]
    const char *localizedString; // [esp+88h] [ebp-164h]
    float color1[4]; // [esp+8Ch] [ebp-160h] BYREF
    int weapIndex; // [esp+9Ch] [ebp-150h]
    float bias; // [esp+A0h] [ebp-14Ch]
    char binding[260]; // [esp+A4h] [ebp-148h] BYREF
    float colorMod[4]; // [esp+1ACh] [ebp-40h] BYREF
    const WeaponDef *weapDef; // [esp+1BCh] [ebp-30h]
    const char *text; // [esp+1C0h] [ebp-2Ch]
    rectDef_s textRect; // [esp+1C4h] [ebp-28h] BYREF
    float color2[4]; // [esp+1DCh] [ebp-10h] BYREF

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->predictedPlayerState.pm_type < 9
        && cgameGlob->predictedPlayerState.waterlevel < 3
        && cgameGlob->predictedPlayerState.pm_type != 4
        && (cgameGlob->predictedPlayerState.eFlags & 0x300) == 0
        && cgameGlob->predictedPlayerState.weaponstate != 10
        && cgameGlob->predictedPlayerState.weaponstate != 12
        && cgameGlob->predictedPlayerState.weaponstate != 14
        && cgameGlob->predictedPlayerState.weaponstate != 13
        && cgameGlob->predictedPlayerState.weaponstate != 11
        && cgameGlob->predictedPlayerState.weaponstate != 15
        && cgameGlob->predictedPlayerState.weaponstate != 16 )
    {
        if ( cgameGlob->predictedPlayerState.weapon )
        {
            fade = AmmoCounterFadeAlpha(localClientNum, cgameGlob);
            if ( fade != 0.0 )
            {
                weapIndex = GetWeaponIndex(cgameGlob);
                weapDef = BG_GetWeaponDef(weapIndex);
                if ( weapDef->bDualWield )
                {
                    if ( !CG_CheckPlayerForLowClipSpecific(cgameGlob, weapIndex)
                        && !CG_CheckPlayerForLowClipSpecific(cgameGlob, weapDef->dualWieldWeaponIndex) )
                    {
                        return;
                    }
                }
                else if ( !CG_CheckPlayerForLowClipSpecific(cgameGlob, weapIndex) )
                {
                    return;
                }
                if ( (cgameGlob->predictedPlayerState.eFlags & 0x4300) == 0 )
                {
                    canReload = BG_GetAmmoNotInClip(&cgameGlob->predictedPlayerState, weapIndex) > 0;
                    if ( weapDef->bDualWield )
                    {
                        canReloadLeft = BG_GetAmmoNotInClip(&cgameGlob->predictedPlayerState, weapDef->dualWieldWeaponIndex) > 0;
                        LODWORD(v12) = !BG_GetAmmoInClip(&cgameGlob->predictedPlayerState, weapIndex)
                                                && !BG_GetAmmoInClip(&cgameGlob->predictedPlayerState, weapDef->dualWieldWeaponIndex);
                        empty = LOBYTE(v12);
                    }
                    else
                    {
                        empty = BG_GetAmmoInClip(&cgameGlob->predictedPlayerState, weapIndex) == 0;
                        canReloadLeft = 0;
                    }
                    if ( weapDef->ammoCounterClip )
                    {
                        if ( canReload || canReloadLeft )
                        {
                            if ( weapDef->bNoPartialReload || BG_GetClipSize(weapIndex) == 1 )
                                return;
                            text = "PLATFORM_RELOAD";
                            Byte4UnpackRgba((const unsigned __int8 *)&lowAmmoWarningColor1->current, color1);
                            Byte4UnpackRgba((const unsigned __int8 *)&lowAmmoWarningColor2->current, color2);
                        }
                        else if ( !empty || weapDef->bIsCarriedKillstreakWeapon )
                        {
                            if ( weapDef->bNoPartialReload
                                || weapDef->guidedMissileType == MISSILE_GUIDANCE_TVGUIDED
                                || weapDef->bDualWield
                                && (!CG_CheckPlayerForLowClipSpecific(cgameGlob, weapIndex)
                                 || !CG_CheckPlayerForLowClipSpecific(cgameGlob, weapDef->dualWieldWeaponIndex)) )
                            {
                                return;
                            }
                            text = "PLATFORM_LOW_AMMO_NO_RELOAD";
                            Byte4UnpackRgba((const unsigned __int8 *)&lowAmmoWarningNoReloadColor1->current, color1);
                            Byte4UnpackRgba((const unsigned __int8 *)&lowAmmoWarningNoReloadColor2->current, color2);
                        }
                        else
                        {
                            text = "WEAPON_NO_AMMO";
                            Byte4UnpackRgba((const unsigned __int8 *)&lowAmmoWarningNoAmmoColor1->current, color1);
                            Byte4UnpackRgba((const unsigned __int8 *)&lowAmmoWarningNoAmmoColor2->current, color2);
                        }
                        amplitude = (float)(lowAmmoWarningPulseMax->current.value - lowAmmoWarningPulseMin->current.value) * 0.5;
                        bias = lowAmmoWarningPulseMin->current.value + amplitude;

                        //*((float *)&v12 + 1) = (float)((float)cgameGlob->time * 0.0062831854) * lowAmmoWarningPulseFreq->current.value;
                        //v9 = *((float *)&v12 + 1);
                        //__libm_sse2_sin(v12);
                        //*(float *)&v9 = v9;

                        //Vec4Lerp(color1, color2, (float)(*(float *)&v9 * amplitude) + bias, colorMod);
                        Vec4Lerp(color1, color2, (float)(sin(((float)cgameGlob->time * 0.0062831854) * lowAmmoWarningPulseFreq->current.value)* amplitude) + bias, colorMod);
                        colorMod[3] = colorMod[3] * fade;
                        if ( material )
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
                                colorMod,
                                material);
                        if ( !UI_GetKeyBindingLocalizedString(localClientNum, "+reload", binding, 0) )
                        {
                            v10 = UI_SafeTranslateString("KEY_UNBOUND");
                            I_strncpyz(binding, v10, 256);
                        }
                        v11 = UI_SafeTranslateString(text);
                        localizedString = UI_ReplaceConversionString((char*)v11, binding);
                        UI_DrawWrappedText(
                            &scrPlaceView[localClientNum],
                            localizedString,
                            rect,
                            font,
                            rect->x + text_x,
                            rect->y + text_y,
                            textScale,
                            colorMod,
                            textStyle,
                            textAlignMode,
                            &textRect);
                    }
                }
            }
        }
    }
}

