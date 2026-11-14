#include "bg_weapons.h"
#include "bg_local.h"
#include "bg_public.h"
#include <universal/com_loadutils.h>
#include <universal/surfaceflags.h>
#include <qcommon/common.h>
#include <qcommon/cm_trace.h>
#include "bg_weapons_def.h"
#include "bg_weapons_ammo.h"
#include "bg_misc.h"
#include "bg_pmove.h"
#include "bg_mantle.h"
#include "bg_perks.h"
#include <game_mp/g_main_mp.h>
#include <cgame/cg_compass.h>
#include "bg_dtp.h"
#include "bg_weapons_load_obj.h"
#include <ui_mp/ui_gametype_variants_mp.h>
#include <client/splitscreen.h>
#include <universal/com_math_anglevectors.h>
#include <cgame/cg_drawtools.h>
#include <universal/com_files.h>
#include <cgame_mp/cg_predict_mp.h>

static const float vehicleGunnerADSLerpTimeMS = 300.0f;
static const float vehicleADSLerpTimeMS = 500.0f;
static const float trace_dist = 1000.0f;

static const char *impactTypeNames[15 + 1] =
{
  "bad allocation",
  "bullet_small",
  "bullet_large",
  "bullet_ap",
  "bullet_xtreme",
  "shotgun",
  "grenade_bounce",
  "grenade_explode",
  "rifle_grenade",
  "rocket_explode",
  "rocket_explode_xtreme",
  "projectile_dud",
  "mortar_shell",
  "tank_shell",
  "bolt",
  "blade"
};

bool penetrationDepthTableLoaded;
float penetrationDepthTable[4][31];

void __cdecl BG_LoadPenetrationDepthTable()
{
    char *buffer; // [esp+0h] [ebp-400Ch]
    char loadBuffer[16388]; // [esp+4h] [ebp-4008h] BYREF

    if ( !penetrationDepthTableLoaded )
    {
        buffer = Com_LoadInfoString(
                             (char*)"info/bullet_penetration_mp",
                             "bullet penetration table",
                             "BULLET_PEN_TABLE",
                             loadBuffer);
        Com_Memset((unsigned int *)penetrationDepthTable, 0, 496);
        BG_ParsePenetrationDepthTable("small", penetrationDepthTable[1], buffer);
        BG_ParsePenetrationDepthTable("medium", penetrationDepthTable[2], buffer);
        BG_ParsePenetrationDepthTable("large", penetrationDepthTable[3], buffer);
        penetrationDepthTableLoaded = 1;
    }
}

void __cdecl BG_ParsePenetrationDepthTable(const char *penetrateType, float *depthTable, char *buffer)
{
    const char *v3; // eax
    const char *v4; // eax
    int iTypeIndex; // [esp+0h] [ebp-2084h]
    char dest[7940]; // [esp+4h] [ebp-2080h] BYREF
    cspField_t pFieldList[31]; // [esp+1F0Ch] [ebp-178h] BYREF

    if ( !penetrateType
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 181, 0, "%s", "penetrateType") )
    {
        __debugbreak();
    }
    if ( !depthTable
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 182, 0, "%s", "depthTable") )
    {
        __debugbreak();
    }
    if ( !buffer && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 183, 0, "%s", "buffer") )
        __debugbreak();
    for ( iTypeIndex = 0; iTypeIndex < 31; ++iTypeIndex )
    {
        v3 = Com_SurfaceTypeToName(iTypeIndex);
        if ( Com_sprintf(&dest[256 * iTypeIndex], 0x100u, "%s_%s", penetrateType, v3) < 0 )
        {
            v4 = Com_SurfaceTypeToName(iTypeIndex);
            Com_Error(ERR_DROP, "Bullet penetration table param name [%s_%s] is to long.", penetrateType, v4);
        }
        pFieldList[iTypeIndex].szName = &dest[256 * iTypeIndex];
        pFieldList[iTypeIndex].iOffset = 4 * iTypeIndex;
        pFieldList[iTypeIndex].iFieldType = 7;
    }
    if ( !ParseConfigStringToStruct((unsigned __int8 *)depthTable, pFieldList, 31, buffer, 0, 0, BG_StringCopy) )
        Com_Error(ERR_DROP, "Error parsing bullet penetration table [%s].", penetrateType);
}

char __cdecl BG_AdvanceTrace(BulletFireParams *bp, BulletTraceResults *br, float dist)
{
    float dot; // [esp+28h] [ebp-4h]

    if ( !bp && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 225, 0, "%s", "bp") )
        __debugbreak();
    if ( !br && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 226, 0, "%s", "br") )
        __debugbreak();
    if ( br->trace.hitType == TRACE_HITTYPE_NONE
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp",
                    227,
                    0,
                    "%s",
                    "br->trace.hitType != TRACE_HITTYPE_NONE") )
    {
        __debugbreak();
    }
    bp->ignoreEntIndex = Trace_GetEntityHitId(&br->trace);
    if ( bp->ignoreEntIndex == 1022 && dist > 0.0 )
    {
        dot = -(
                                       (float)((float)(br->trace.normal.vec.v[0] * bp->dir[0])
                                     + (float)(br->trace.normal.vec.v[1] * bp->dir[1]))
                                     + (float)(br->trace.normal.vec.v[2] * bp->dir[2]));
        if ( dot < 0.125 )
        {
            bp->start[0] = (float)((float)(dist / 0.125) * bp->dir[0]) + br->hitPos[0];
            bp->start[1] = (float)((float)(dist / 0.125) * bp->dir[1]) + br->hitPos[1];
            bp->start[2] = (float)((float)(dist / 0.125) * bp->dir[2]) + br->hitPos[2];
            return 0;
        }
        bp->start[0] = (float)((float)(dist / dot) * bp->dir[0]) + br->hitPos[0];
        bp->start[1] = (float)((float)(dist / dot) * bp->dir[1]) + br->hitPos[1];
        bp->start[2] = (float)((float)(dist / dot) * bp->dir[2]) + br->hitPos[2];
    }
    else
    {
        bp->start[0] = br->hitPos[0];
        bp->start[1] = br->hitPos[1];
        bp->start[2] = br->hitPos[2];
    }
    return 1;
}

double __cdecl BG_GetSurfacePenetrationDepth(const WeaponDef *weapDef, unsigned int surfaceType)
{
    if ( !weapDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 257, 0, "%s", "weapDef") )
    {
        __debugbreak();
    }
    if ( weapDef->penetrateType == PENETRATE_TYPE_NONE
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp",
                    258,
                    0,
                    "%s",
                    "weapDef->penetrateType != PENETRATE_TYPE_NONE") )
    {
        __debugbreak();
    }
    if ( weapDef->penetrateType >= (unsigned int)PENETRATE_TYPE_COUNT
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp",
                    259,
                    0,
                    "weapDef->penetrateType doesn't index PENETRATE_TYPE_COUNT\n\t%i not in [0, %i)",
                    weapDef->penetrateType,
                    4) )
    {
        __debugbreak();
    }
    if ( surfaceType >= 0x1F
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp",
                    260,
                    0,
                    "surfaceType doesn't index SURF_TYPECOUNT\n\t%i not in [0, %i)",
                    surfaceType,
                    31) )
    {
        __debugbreak();
    }
    if ( surfaceType )
        return penetrationDepthTable[weapDef->penetrateType][surfaceType];
    else
        return 0.0;
}

void __cdecl PM_StartWeaponAnim(playerState_s *ps, int anim, int leftAnim)
{
    if ( ps->pm_type < 9 )
    {
        if ( leftAnim )
            ps->weapAnimLeft = anim | ps->weapAnimLeft & 0x400 ^ 0x400;
        else
            ps->weapAnim = anim | ps->weapAnim & 0x400 ^ 0x400;
    }
}

const char *__cdecl BG_GetImpactTypeName(int impactTypeIndex)
{
    if ( impactTypeIndex <= 0 || impactTypeIndex >= 16 )
        return "default";
    else
    {
        //return (&bad_alloc_Message_9)[impactTypeIndex];
        return impactTypeNames[impactTypeIndex];
    }
}

unsigned int __cdecl BG_GetViewmodelWeaponIndex(const playerState_s *ps)
{
    int weapIndex; // [esp+0h] [ebp-4h]

    if ( (ps->weapFlags & 2) != 0 && ps->weaponstate >= 20 && ps->weaponstate <= 25 )
    {
        weapIndex = ps->offHandIndex;
        if ( !weapIndex
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp",
                        333,
                        0,
                        "%s",
                        "weapIndex != WP_NONE") )
        {
            __debugbreak();
        }
    }
    else if ( (ps->weaponstate == 17 || ps->weaponstate == 18 || ps->weaponstate == 19)
                 && !BG_IsUseAsMeleeWeapon(ps->weapon) )
    {
        if ( ps->weaponstate == 19 )
            return ps->weapon;
        else
            return ps->meleeWeapon;
    }
    else
    {
        return ps->weapon;
    }
    return weapIndex;
}

int __cdecl BG_GetFirstAvailableOffhand(const playerState_s *ps, int offhandSlot)
{
    int slot; // [esp+0h] [ebp-Ch]
    int weapIndex; // [esp+4h] [ebp-8h]

    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 361, 0, "%s", "ps") )
        __debugbreak();
    for ( slot = 0; slot < 15; ++slot )
    {
        weapIndex = ps->heldWeapons[slot].weapon;
        if ( weapIndex
            && BG_GetWeaponDef(weapIndex)->offhandSlot == offhandSlot
            && (ps->throwBackGrenadeTimeLeft > 0 || BG_WeaponAmmo(ps, weapIndex) > 0) )
        {
            BG_AssertOffhandIndexOrNone(weapIndex);
            return weapIndex;
        }
    }
    return 0;
}

int __cdecl BG_GetFirstEquippedOffhand(const playerState_s *ps, int offhandSlot)
{
    int slot; // [esp+0h] [ebp-Ch]
    int weapIndex; // [esp+4h] [ebp-8h]

    for ( slot = 0; slot < 15; ++slot )
    {
        weapIndex = ps->heldWeapons[slot].weapon;
        if ( weapIndex && BG_GetWeaponDef(weapIndex)->offhandSlot == offhandSlot )
            return weapIndex;
    }
    return 0;
}

int *__cdecl BG_GetWeaponState(playerState_s *ps, bool leftGun)
{
    if ( leftGun )
        return &ps->weaponstateLeft;
    else
        return (int*)&ps->weaponstate;
}

bool __cdecl BG_IsAimDownSightWeapon(unsigned int weaponIndex)
{
    return BG_GetWeaponDef(weaponIndex)->aimDownSight;
}

bool __cdecl BG_CanPlayerHaveWeapon(unsigned int weaponIndex)
{
    return BG_GetWeaponDef(weaponIndex)->gunXModel != 0;
}

bool __cdecl BG_ValidateWeaponNumber(unsigned int weaponIndex)
{
    return weaponIndex < BG_GetNumWeapons();
}

bool __cdecl BG_IsWeaponValid(const playerState_s *ps, unsigned int weaponIndex)
{
    return BG_ValidateWeaponNumber(weaponIndex) && BG_PlayerHasWeapon(ps, weaponIndex);
}

bool __cdecl BG_WeaponBlocksProne(unsigned int weapIndex)
{
    return BG_GetWeaponDef(weapIndex)->blocksProne;
}

bool __cdecl BG_IsOverheatingWeapon(unsigned int weapIndex)
{
    return BG_GetWeaponDef(weapIndex)->overheatWeapon != 0;
}

bool __cdecl BG_IsBayonetWeapon(unsigned int weapIndex)
{
    return BG_GetWeaponDef(weapIndex)->bHasBayonet;
}

bool __cdecl BG_IsUseAsMeleeWeapon(unsigned int weapIndex)
{
    const WeaponDef *weapDef; // [esp+8h] [ebp-4h]

    weapDef = BG_GetWeaponDef(weapIndex);
    return weapDef->bHasBayonet || weapDef->bUseAsMelee;
}

int __cdecl BG_TakePlayerWeapon(playerState_s *ps, unsigned int weaponIndex)
{
    unsigned int curWeaponIndex; // [esp+18h] [ebp-4h]

    if ( !BG_PlayerHasWeapon(ps, weaponIndex) )
        return 0;
    BG_TakeHeldWeapon(ps, weaponIndex);
    if ( weaponIndex == ps->weapon )
        ps->weapon = 0;
    if ( weaponIndex == ps->meleeWeapon )
        ps->meleeWeapon = 0;
    for ( curWeaponIndex = BG_GetWeaponVariantDef(weaponIndex)->altWeaponIndex;
                curWeaponIndex && BG_PlayerHasWeapon(ps, curWeaponIndex);
                curWeaponIndex = BG_GetWeaponVariantDef(curWeaponIndex)->altWeaponIndex )
    {
        BG_TakeHeldWeapon(ps, curWeaponIndex);
        if ( weaponIndex == ps->weapon )
            ps->weapon = 0;
    }
    if ( ps->weaponstate == 35 || ps->weaponstate == 36 || ps->weaponstate == 37 )
    {
        ps->eFlags &= 0xFFFFFCFF;
        PM_ResetWeaponState(ps);
    }
    return 1;
}

void __cdecl BG_TakeHeldWeapon(playerState_s *ps, unsigned int weaponIndex)
{
    AmmoPool *pool; // [esp+0h] [ebp-1Ch]
    AmmoClip *clip; // [esp+4h] [ebp-18h]
    const WeaponVariantDef *weapVarDef; // [esp+8h] [ebp-14h]
    int ammoIndex; // [esp+Ch] [ebp-10h]
    int slot; // [esp+10h] [ebp-Ch]
    bool getRidOfAmmoPool; // [esp+1Bh] [ebp-1h]

    weapVarDef = BG_GetWeaponVariantDef(weaponIndex);
    ammoIndex = weapVarDef->iAmmoIndex;
    getRidOfAmmoPool = 1;
    for ( slot = 0; slot < 15; ++slot )
    {
        if ( ps->heldWeapons[slot].weapon == weaponIndex )
        {
            ps->heldWeapons[slot].weapon = 0;
        }
        else if ( ammoIndex == BG_GetWeaponVariantDef(ps->heldWeapons[slot].weapon)->iAmmoIndex )
        {
            getRidOfAmmoPool = 0;
        }
    }
    clip = BG_GetAmmoClip(ps, weapVarDef->iClipIndex);
    if ( clip )
    {
        clip->clipIndex = 0;
        clip->count = 0;
    }
    if ( getRidOfAmmoPool )
    {
        pool = BG_GetAmmoPool(ps, ammoIndex);
        if ( pool )
        {
            pool->ammoIndex = 0;
            pool->count = 0;
        }
    }
}

void __cdecl BG_GetSpreadForWeapon(
                const playerState_s *ps,
                const WeaponDef *weapDef,
                float *minSpread,
                float *maxSpread)
{
    float v4; // xmm0_4
    double stackFireAccuracyDecay; // xmm0_8
    long double v6; // [esp+0h] [ebp-28h]
    long double v7; // [esp+8h] [ebp-20h]
    float frac; // [esp+24h] [ebp-4h]
    float fraca; // [esp+24h] [ebp-4h]

    if ( ps->spreadOverrideState == 2 )
    {
        *minSpread = (float)ps->spreadOverride;
        *maxSpread = (float)ps->spreadOverride;
    }
    else
    {
        if ( ps->viewHeightCurrent <= 40.0 )
        {
            fraca = (float)(ps->viewHeightCurrent - 11.0) / 29.0;
            *minSpread = (float)((float)(weapDef->fHipSpreadDuckedMin - weapDef->fHipSpreadProneMin) * fraca)
                                 + weapDef->fHipSpreadProneMin;
            v4 = (float)((float)(weapDef->hipSpreadDuckedMax - weapDef->hipSpreadProneMax) * fraca)
                 + weapDef->hipSpreadProneMax;
        }
        else
        {
            frac = (float)(ps->viewHeightCurrent - 40.0) / 20.0;
            *minSpread = (float)((float)(weapDef->fHipSpreadStandMin - weapDef->fHipSpreadDuckedMin) * frac)
                                 + weapDef->fHipSpreadDuckedMin;
            v4 = (float)((float)(weapDef->hipSpreadStandMax - weapDef->hipSpreadDuckedMax) * frac)
                 + weapDef->hipSpreadDuckedMax;
        }
        *maxSpread = v4;
    }
    //if ( weapDef->fireType == WEAPON_FIRETYPE_STACKED && ps->stackFireCount > 1 )
    //{
    //    HIDWORD(v7) = ps->stackFireCount;
    //    stackFireAccuracyDecay = weapDef->stackFireAccuracyDecay;
    //    __libm_sse2_pow(v6, v7);
    //    *(float *)&stackFireAccuracyDecay = stackFireAccuracyDecay;
    //    *minSpread = *minSpread * *(float *)&stackFireAccuracyDecay;
    //    *maxSpread = *maxSpread * *(float *)&stackFireAccuracyDecay;
    //}
    if (weapDef->fireType == WEAPON_FIRETYPE_STACKED && ps->stackFireCount > 1)
    {
        float decay = powf(
            weapDef->stackFireAccuracyDecay,
            (float)ps->stackFireCount
        );

        *minSpread *= decay;
        *maxSpread *= decay;
    }
    if ( ps->spreadOverrideState == 1 )
        *maxSpread = (float)ps->spreadOverride;
    if ( (ps->perks[0] & 4) != 0 )
    {
        *minSpread = (float)(perk_weapSpreadMultiplier->current.value * 1.0) * *minSpread;
        *maxSpread = (float)(perk_weapSpreadMultiplier->current.value * 1.0) * *maxSpread;
    }
}

void __cdecl PM_UpdateAimDownSightFlag(pmove_t *pm, pml_t *pml)
{
    bool adsRequested; // [esp+2h] [ebp-6h]
    bool adsAllowed; // [esp+3h] [ebp-5h]
    playerState_s *ps; // [esp+4h] [ebp-4h]

    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 756, 0, "%s", "ps") )
        __debugbreak();
    ps->pm_flags &= ~0x10u;
    adsAllowed = PM_IsAdsAllowed(pm, pml);
    //adsRequested = bitarray<51>::testBit(&pm->cmd.button_bits, 0xBu);
    adsRequested = pm->cmd.button_bits.testBit(0xBu);
    //if ( bitarray<51>::testBit(&pm->cmd.button_bits, 1u)
    if ( pm->cmd.button_bits.testBit(1u)
        && (!BG_UsingSniperScope(ps) || !pm->cmd.button_bits.testBit(0xDu)) )
    {
        PM_ExitAimDownSight(ps);
        adsAllowed = 0;
    }
    if ( adsRequested && adsAllowed )
    {
        if ( (ps->pm_flags & 1) == 0 || BG_UsingSniperScope(ps) )
        {
            ps->pm_flags |= 0x10u;
            if ( (ps->otherFlags & 4) == 0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp",
                            789,
                            0,
                            "%s",
                            "ps->otherFlags & POF_PLAYER") )
            {
                __debugbreak();
            }
        }
        //else if ( !bitarray<51>::testBit(&pm->oldcmd.button_bits, 0xBu) || !pm->cmd.forwardmove && !pm->cmd.rightmove )
        else if ( !pm->oldcmd.button_bits.testBit(0xBu) || !pm->cmd.forwardmove && !pm->cmd.rightmove )
        {
            ps->pm_flags |= 0x10u;
            ps->pm_flags |= 0x200u;
            if ( (ps->otherFlags & 4) == 0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp",
                            783,
                            0,
                            "%s",
                            "ps->otherFlags & POF_PLAYER") )
            {
                __debugbreak();
            }
        }
    }
}

bool __cdecl PM_IsAdsAllowed(pmove_t *pm, pml_t *pml)
{
    bool result; // al
    unsigned int weapIndex; // [esp+Ch] [ebp-Ch]
    const WeaponDef *weapDef; // [esp+10h] [ebp-8h]
    playerState_s *ps; // [esp+14h] [ebp-4h]

    if ( !pm && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 677, 0, "%s", "pm") )
        __debugbreak();
    if ( !pml && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 678, 0, "%s", "pml") )
        __debugbreak();
    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 682, 0, "%s", "ps") )
        __debugbreak();
    switch ( ps->pm_type )
    {
        case 1:
            if ( !pml->almostGroundPlane )
                goto LABEL_17;
            result = 0;
            break;
        case 2:
        case 3:
        case 4:
        case 5:
        case 9:
        case 0xA:
            if ( (ps->eFlags & 0x4000) != 0 && (ps->eFlags2 & 0x10000000) != 0 )
                goto LABEL_17;
            result = 0;
            break;
        default:
LABEL_17:
            if ( (ps->otherFlags & 4) != 0 )
            {
                weapIndex = pmoveHandlers[pm->handler].getPlayerWeapon(ps, pm->localClientNum);
                weapDef = BG_GetWeaponDef(weapIndex);
                if ( weapDef->aimDownSight )
                {
                    if ( !player_disableWeaponsInWater->current.enabled || ps->waterlevel < 3 || ps->pm_type == 1 )
                    {
                        if ( ps->weaponstate < 20 || ps->weaponstate > 25 )
                        {
                            if ( ps->weaponstate == 1
                                || ps->weaponstate == 2
                                || ps->weaponstate == 3
                                || ps->weaponstate == 4
                                || ps->weaponstate == 5 )
                            {
                                result = 0;
                            }
                            else if ( ps->weaponstate < 40 || ps->weaponstate > 42 )
                            {
                                result = (ps->pm_flags & 4) == 0
                                            && (ps->weapFlags & 0x20) == 0
                                            && (!weapDef->noAdsWhenMagEmpty || BG_GetAmmoInClip(ps, weapIndex));
                            }
                            else
                            {
                                result = 0;
                            }
                        }
                        else
                        {
                            result = 0;
                        }
                    }
                    else
                    {
                        result = 0;
                    }
                }
                else
                {
                    result = 0;
                }
            }
            else
            {
                result = 0;
            }
            break;
    }
    return result;
}

void __cdecl PM_ExitAimDownSight(playerState_s *ps)
{
    PM_AddEvent(ps, 0x11u);
    ps->pm_flags &= ~0x10u;
}

void __cdecl PM_UpdateScopeZoom(pmove_t *pm)
{
    playerState_s *ps; // [esp+0h] [ebp-4h]

    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 804, 0, "%s", "ps") )
        __debugbreak();
    if ( !ps->adsZoomLatchState
        && pm->cmd.serverTime - ps->adsZoomLatchTime > 40
        && pm->cmd.button_bits.testBit(2u)
        && BG_UsingSniperScope(pm->ps)
        && BG_ADSFullyUp(pm->ps) )
    {
        ++ps->adsZoomSelect;
        ps->adsZoomLatchState = 1;
        ps->adsZoomLatchTime = pm->cmd.serverTime;
        PM_AddEvent(ps, 0xBEu);
    }
    else if ( pm->cmd.serverTime - ps->adsZoomLatchTime > 40
                 && !pm->cmd.button_bits.testBit(2u)
                 && BG_UsingSniperScope(pm->ps) )
    {
        if ( BG_ADSFullyUp(pm->ps) )
        {
            ps->adsZoomLatchState = 0;
            ps->adsZoomLatchTime = pm->cmd.serverTime;
        }
    }
}

void __cdecl PM_UpdateSpinLerp(pmove_t *pm, pml_t *pml)
{
    float v2; // [esp+0h] [ebp-1Ch]
    float v3; // [esp+4h] [ebp-18h]
    float v4; // [esp+8h] [ebp-14h]
    unsigned int weapIndex; // [esp+Ch] [ebp-10h]
    const WeaponDef *weapDef; // [esp+10h] [ebp-Ch]
    playerState_s *ps; // [esp+14h] [ebp-8h]
    float lerpRate; // [esp+18h] [ebp-4h]

    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 830, 0, "%s", "ps") )
        __debugbreak();
    weapIndex = pmoveHandlers[pm->handler].getPlayerWeapon(ps, pm->localClientNum);
    weapDef = BG_GetWeaponDef(weapIndex);
    if ( weapDef->fireType == WEAPON_FIRETYPE_MINIGUN )
    {
        if ( pm->cmd.button_bits.testBit(0xBu) || pm->cmd.button_bits.testBit(0) )
        {
            ps->eFlags2 |= 0x400000u;
            lerpRate = 1.0 / (float)weapDef->iSpinUpTime;
        }
        else
        {
            ps->eFlags2 &= ~0x400000u;
            lerpRate = -1.0 / (float)weapDef->iSpinDownTime;
        }
        v3 = (float)((float)pml->msec * lerpRate) + ps->weaponSpinLerp;
        if ( (float)(v3 - 1.0) < 0.0 )
            v4 = (float)((float)pml->msec * lerpRate) + ps->weaponSpinLerp;
        else
            v4 = 1.0f;
        if ( (float)(0.0 - v3) < 0.0 )
            v2 = v4;
        else
            v2 = 0.0f;
        ps->weaponSpinLerp = v2;
    }
    else
    {
        ps->weaponSpinLerp = 0.0f;
    }
}

void __cdecl PM_UpdateAimDownSightLerp(pmove_t *pm, pml_t *pml)
{
    float v2; // [esp+8h] [ebp-3Ch]
    float v3; // [esp+1Ch] [ebp-28h]
    float v4; // [esp+20h] [ebp-24h]
    float lerpRate; // [esp+28h] [ebp-1Ch]
    bool adsRequested; // [esp+2Fh] [ebp-15h]
    const WeaponVariantDef *weapVariantDef; // [esp+30h] [ebp-14h]
    unsigned int weapIndex; // [esp+34h] [ebp-10h]
    const WeaponDef *weapDef; // [esp+38h] [ebp-Ch]
    playerState_s *ps; // [esp+3Ch] [ebp-8h]

    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 865, 0, "%s", "ps") )
        __debugbreak();
    weapIndex = pmoveHandlers[pm->handler].getPlayerWeapon(ps, pm->localClientNum);
    weapVariantDef = BG_GetWeaponVariantDef(weapIndex);
    weapDef = BG_GetWeaponDef(weapIndex);
    if ( (ps->eFlags2 & 0x40000) != 0 && ps->fWeaponPosFrac > 0.0
        || player_scopeExitOnDamage->current.enabled && ps->damageCount && weapDef->overlayReticle )
    {
        PM_ExitAimDownSight(ps);
        ps->fWeaponPosFrac = 0.0f;
        ps->adsDelayTime = 0;
    }
    else if ( weapDef->aimDownSight )
    {
        adsRequested = 0;
        if ( !weapDef->bReloadWhileAds
            && !weapDef->bSegmentedReload
            && ps->weaponstate == 10
            && ps->weaponTime - weapDef->iPositionReloadTransTime > 0
            || !weapDef->bReloadWhileAds
            && weapDef->bSegmentedReload
            && (ps->weaponstate == 10
             || !weapDef->bReloadWhileAds && ps->weaponstate == 11
             || !weapDef->bReloadWhileAds && (ps->weaponstate == 12 || ps->weaponstate == 13)
             || !weapDef->bReloadWhileAds && ps->weaponstate == 14 && ps->weaponTime - weapDef->iPositionReloadTransTime > 0)
            || !weapDef->bRechamberWhileAds && ps->weaponstate == 7 )
        {
            adsRequested = 0;
        }
        else if ( (ps->eFlags & 0x4000) != 0 && ps->vehiclePos >= 1 && ps->vehiclePos <= 4 && ps->leanf > 0.0 )
        {
            adsRequested = 0;
        }
        else if ( (ps->pm_flags & 0x10) != 0 )
        {
            adsRequested = 1;
        }
        if ( weapDef->adsFireOnly
            && (ps->weaponDelay || weapDef->fireType == WEAPON_FIRETYPE_STACKED && (ps->weapFlags & 0x400) != 0)
            && (ps->weaponstate == 6 || ps->weaponstate == 31) )
        {
            adsRequested = 1;
        }
        if ( ps->fWeaponPosFrac != 1.0 || adsRequested || player_adsExitDelay->current.integer <= 0 )
        {
            ps->adsDelayTime = 0;
        }
        else
        {
            if ( !ps->adsDelayTime )
                ps->adsDelayTime = player_adsExitDelay->current.integer + pm->cmd.serverTime;
            if ( ps->adsDelayTime <= pm->cmd.serverTime )
                ps->adsDelayTime = 0;
            else
                adsRequested = 1;
        }
        if ( adsRequested && ps->fWeaponPosFrac != 1.0 || !adsRequested && ps->fWeaponPosFrac != 0.0 )
        {
            if ( (ps->eFlags & 0x4000) != 0 )
            {
                if ( ps->vehiclePos )
                    lerpRate = 1.0 / vehicleGunnerADSLerpTimeMS;
                else
                    lerpRate = 1.0 / vehicleADSLerpTimeMS;
                if (!adsRequested)
                {
                    //LODWORD(lerpRate) ^= _mask__NegFloat_;
                    lerpRate = -lerpRate;
                }
            }
            else
            {
                if (adsRequested)
                {
                    lerpRate = weapVariantDef->fOOPosAnimLength[0];
                }
                else
                {
                    //lerpRate = -weapVariantDef->fOOPosAnimLength[1];
                    lerpRate = -weapVariantDef->fOOPosAnimLength[1];
                }
                if ( (ps->perks[0] & 0x4000) != 0
                    && weapDef->playerAnimType != BG_GetPlayerAnimTypeIndex(1)
                    && weapDef->playerAnimType != BG_GetPlayerAnimTypeIndex(2) )
                {
                    if ( perk_weapAdsMultiplier->current.value <= 0.0
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp",
                                    971,
                                    0,
                                    "%s\n\t(perk_weapAdsMultiplier->current.value) = %g",
                                    "(perk_weapAdsMultiplier->current.value > 0.f)",
                                    perk_weapAdsMultiplier->current.value) )
                    {
                        __debugbreak();
                    }
                    lerpRate = lerpRate / perk_weapAdsMultiplier->current.value;
                }
            }
            v3 = (float)((float)pml->msec * lerpRate) + ps->fWeaponPosFrac;
            if ( (float)(v3 - 1.0) < 0.0 )
                v4 = (float)((float)pml->msec * lerpRate) + ps->fWeaponPosFrac;
            else
                v4 = 1.0f;
            if ( (float)(0.0 - v3) < 0.0 )
                v2 = v4;
            else
                v2 = 0.0f;
            ps->fWeaponPosFrac = v2;
        }
    }
    else
    {
        ps->fWeaponPosFrac = 0.0f;
        ps->adsDelayTime = 0;
    }
}

bool __cdecl BG_UsingVehicleWeapon(const playerState_s *ps)
{
    return ps && (ps->eFlags & 0x4000) != 0 && ps->vehiclePos < 5;
}

bool __cdecl BG_ADSFullyUp(const playerState_s *ps)
{
    return ps->fWeaponPosFrac >= 1.0;
}

bool __cdecl BG_UsingSniperScope(const playerState_s *ps)
{
    return BG_GetWeaponDef(ps->weapon)->overlayReticle && ps->fWeaponPosFrac > 0.0;
}

int __cdecl PM_InteruptWeaponWithProneMove(playerState_s *ps)
{
    if ( ps->weaponstate < 6u
        || ps->weaponstate == 10
        || ps->weaponstate == 12
        || ps->weaponstate == 14
        || ps->weaponstate == 13
        || ps->weaponstate == 11
        || ps->weaponstate == 15
        || ps->weaponstate == 16
        || ps->weaponstate == 7 )
    {
        return 1;
    }
    if ( ps->weaponstate == 26
        || ps->weaponstate == 6
        || ps->weaponstate == 7
        || ps->weaponstate == 31
        || ps->weaponstate == 17
        || ps->weaponstate == 18
        || ps->weaponstate == 19
        || ps->weaponstate >= 20 && ps->weaponstate <= 25
        || ps->weaponstate == 33
        || ps->weaponstate == 34 )
    {
        return 0;
    }
    PM_Weapon_Idle(ps);
    return 1;
}

bool __cdecl BG_PlayerWeaponOverheating(const playerState_s *ps, unsigned int weapon)
{
    const PlayerHeldWeapon *weaponState; // [esp+8h] [ebp-4h]

    if ( !BG_IsOverheatingWeapon(weapon) )
        return 0;
    weaponState = BG_GetHeldWeaponConst(ps, ps->weapon);
    return weaponState && (weaponState->overHeating || weaponState->heatPercent >= 100.0);
}

void __cdecl BG_PlayerWeaponOverheatUpdate(playerState_s *ps, unsigned int weapon, float amount)
{
    PlayerHeldWeapon *weaponState; // [esp+8h] [ebp-8h]

    if ( BG_IsOverheatingWeapon(weapon) )
    {
        weaponState = BG_GetHeldWeapon(ps, weapon);
        weaponState->heatPercent = weaponState->heatPercent + amount;
        if ( weaponState->heatPercent >= 100.0 )
        {
            weaponState->heatPercent = 100.0f;
            weaponState->overHeating = 1;
        }
        if ( weaponState->heatPercent < 0.0 )
            weaponState->fuelTankTime = 0;
        if ( BG_GetWeaponDef(weapon)->overheatEndVal >= weaponState->heatPercent )
            weaponState->overHeating = 0;
    }
}

PlayerHeldWeapon *__cdecl BG_GetHeldWeapon(playerState_s *ps, unsigned int weaponIndex)
{
    int slot; // [esp+4h] [ebp-4h]

    if ( !ps && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\bgame\\../bgame/bg_weapons.h", 329, 0, "%s", "ps") )
        __debugbreak();
    if ( !weaponIndex )
        return 0;
    slot = BG_GetHeldWeaponSlot(ps, weaponIndex);
    if ( slot == -1 )
        return 0;
    else
        return &ps->heldWeapons[slot];
}

bool __cdecl PM_IsWeaponMinFireTimeElapsed(playerState_s *ps, bool leftGun)
{
    if ( !leftGun && ps->weaponTime > 0 )
        return 1;
    return leftGun && ps->weaponTimeLeft > 0;
}

unsigned int __cdecl PM_GetWeaponIndexForHand(playerState_s *ps)
{
    if ( ps->bRunLeftGun )
        return BG_GetWeaponDef(ps->weapon)->dualWieldWeaponIndex;
    else
        return ps->weapon;
}

bool __cdecl BG_IsAltSwitch(playerState_s *ps, unsigned int lastWeapon, unsigned int weapon)
{
    if ( !lastWeapon )
        return 0;
    if ( !weapon )
        return 0;
    if ( weapon == BG_GetWeaponVariantDef(lastWeapon)->altWeaponIndex )
        return 1;
    return BG_GetWeaponDef(lastWeapon)->inventoryType == WEAPINVENTORY_ALTMODE && ps->lastWeaponAltModeSwitch == weapon;
}

bool __cdecl BG_CanFastSwitch(const WeaponDef *weapDef, int weaponState)
{
    return (weaponState == 1 || weaponState == 2 || weaponState == 3 || weaponState == 4 || weaponState == 5)
            && weapDef->offhandSlot != OFFHAND_SLOT_SPECIFIC_USE;
}

char __cdecl BG_IsQuickSwitch(unsigned int oldweapon, unsigned int newweapon, int weaponState)
{
    bool quick; // [esp+3h] [ebp-9h]
    const WeaponDef *oldWeapDef; // [esp+4h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+8h] [ebp-4h]

    oldWeapDef = BG_GetWeaponDef(oldweapon);
    weapDef = BG_GetWeaponDef(newweapon);
    quick = weaponState == 4;
    if ( !oldweapon && weapDef->weapType == WEAPTYPE_PROJECTILE )
        quick = 1;
    if ( oldWeapDef->bCanUseInVehicle || weapDef->bCanUseInVehicle )
        quick = 1;
    if ( weapDef->offhandSlot || oldWeapDef->offhandSlot )
        quick = 1;
    if ( newweapon && weapDef->weapClass == WEAPCLASS_PISTOL )
        return 1;
    return quick;
}

void __cdecl PM_AdjustAimSpreadScale(pmove_t *pm, pml_t *pml)
{
    float v2; // [esp+14h] [ebp-3Ch]
    float spreadOverrideScale; // [esp+24h] [ebp-2Ch]
    float wpnScale; // [esp+28h] [ebp-28h]
    int i; // [esp+34h] [ebp-1Ch]
    int ia; // [esp+34h] [ebp-1Ch]
    const WeaponDef *weapDef; // [esp+38h] [ebp-18h]
    playerState_s *ps; // [esp+3Ch] [ebp-14h]
    float increase; // [esp+40h] [ebp-10h]
    float speedSquared; // [esp+44h] [ebp-Ch]
    float decrease; // [esp+48h] [ebp-8h]
    float viewchange; // [esp+4Ch] [ebp-4h]

    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 2437, 0, "%s", "ps") )
        __debugbreak();
    weapDef = BG_GetWeaponDef(ps->weapon);
    spreadOverrideScale = 1.0f;
    wpnScale = weapDef->fHipSpreadDecayRate;
    if ( wpnScale == 0.0 )
    {
        increase = 0.0f;
        decrease = 1.0f;
    }
    else
    {
        spreadOverrideScale = (float)((float)ps->spreadOverride - weapDef->fHipSpreadStandMin)
                                                / (float)(weapDef->hipSpreadStandMax - weapDef->fHipSpreadStandMin);
        if ( ps->groundEntityNum != 1023 || ps->pm_type == 1 )
        {
            if ( (ps->eFlags & 8) != 0 )
            {
                wpnScale = wpnScale * weapDef->fHipSpreadProneDecay;
                spreadOverrideScale = (float)((float)ps->spreadOverride - weapDef->fHipSpreadProneMin)
                                                        / (float)(weapDef->hipSpreadProneMax - weapDef->fHipSpreadProneMin);
            }
            else if ( (ps->eFlags & 4) != 0 )
            {
                wpnScale = wpnScale * weapDef->fHipSpreadDuckedDecay;
                spreadOverrideScale = (float)((float)ps->spreadOverride - weapDef->fHipSpreadDuckedMin)
                                                        / (float)(weapDef->hipSpreadDuckedMax - weapDef->fHipSpreadDuckedMin);
            }
        }
        else
        {
            wpnScale = wpnScale * 0.5;
        }
        if ( ps->spreadOverrideState == 1 )
        {
            decrease = (float)(wpnScale * pml->frametime) / spreadOverrideScale;
            increase = 0.0f;
        }
        else
        {
            decrease = wpnScale * pml->frametime;
            if ( ps->fWeaponPosFrac == 1.0 )
            {
                increase = 0.0f;
            }
            else
            {
                viewchange = 0.0f;
                if ( weapDef->fHipSpreadTurnAdd != 0.0 )
                {
                    for ( i = 0; i < 2; ++i )
                    {
                        v2 = AngleNormalize180(
                                     (float)((float)pm->cmd.angles[i] * 0.0054931641)
                                 - (float)((float)pm->oldcmd.angles[i] * 0.0054931641));
                        viewchange = (float)((float)((float)(fabs(v2) * 0.0099999998)
                                                                             * weapDef->fHipSpreadTurnAdd)
                                                             / pml->frametime)
                                             + viewchange;
                    }
                }
                if ( weapDef->fHipSpreadMoveAdd != 0.0 && (pm->cmd.forwardmove || pm->cmd.rightmove) )
                {
                    speedSquared = (float)(ps->velocity[0] * ps->velocity[0]) + (float)(ps->velocity[1] * ps->velocity[1]);
                    if ( speedSquared > (float)(bg_aimSpreadMoveSpeedThreshold->current.value
                                                                        * bg_aimSpreadMoveSpeedThreshold->current.value) )
                        viewchange = (float)((float)(weapDef->fHipSpreadMoveAdd * sqrtf(speedSquared)) / (float)ps->speed)
                                             + viewchange;
                }
                if ( ps->groundEntityNum == 1023 && ps->pm_type != 1 )
                {
                    for ( ia = 0; ia < 2; ++ia )
                        viewchange = (float)(0.0099999998 * 128.0) + viewchange;
                }
                increase = viewchange * pml->frametime;
            }
        }
    }
    if ( increase <= 0.0 )
        ps->aimSpreadScale = ps->aimSpreadScale - (float)(decrease * 255.0);
    else
        ps->aimSpreadScale = (float)(increase * 255.0) + ps->aimSpreadScale;
    if ( ps->spreadOverrideState == 1 && (float)(ps->aimSpreadScale * spreadOverrideScale) < 255.0 )
    {
        ps->spreadOverrideState = 0;
        ps->aimSpreadScale = ps->aimSpreadScale * spreadOverrideScale;
    }
    if ( ps->aimSpreadScale >= 0.0 )
    {
        if ( ps->aimSpreadScale > 255.0 )
            ps->aimSpreadScale = 255.0f;
    }
    else
    {
        ps->aimSpreadScale = 0.0f;
    }
}

bool __cdecl ShotLimitReached(playerState_s *ps, const WeaponDef *weapDef)
{
    bool result; // al
    unsigned int *weaponShotCount; // [esp+4h] [ebp-4h]

    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 2543, 0, "%s", "ps") )
        __debugbreak();
    if ( !weapDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 2544, 0, "%s", "weapDef") )
    {
        __debugbreak();
    }
    weaponShotCount = BG_GetWeaponShotCount(ps, ps->bRunLeftGun);
    switch ( weapDef->fireType )
    {
        case WEAPON_FIRETYPE_SINGLESHOT:
            if ( !*weaponShotCount )
                goto LABEL_22;
            result = 1;
            break;
        case WEAPON_FIRETYPE_BURSTFIRE2:
            if ( *weaponShotCount < 2 )
                goto LABEL_22;
            result = 1;
            break;
        case WEAPON_FIRETYPE_BURSTFIRE3:
            if ( *weaponShotCount < 3 )
                goto LABEL_22;
            result = 1;
            break;
        case WEAPON_FIRETYPE_BURSTFIRE4:
            if ( *weaponShotCount < 4 )
                goto LABEL_22;
            result = 1;
            break;
        case WEAPON_FIRETYPE_STACKED:
            if ( *weaponShotCount < ps->stackFireCount )
                goto LABEL_22;
            result = 1;
            break;
        default:
LABEL_22:
            result = 0;
            break;
    }
    return result;
}

unsigned int *__cdecl BG_GetWeaponShotCount(playerState_s *ps, bool leftGun)
{
    if ( leftGun )
        return &ps->weaponShotCountLeft;
    else
        return &ps->weaponShotCount;
}

int __cdecl PM_GetWeaponFireButton(unsigned int weapon)
{
    const WeaponDef *weapDef; // [esp+0h] [ebp-4h]

    weapDef = BG_GetWeaponDef(weapon);
    if ( !weapDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 2989, 0, "%s", "weapDef") )
    {
        __debugbreak();
    }
    if ( (weapDef->weapType == WEAPTYPE_GRENADE || weapDef->weapType == WEAPTYPE_MINE) && weapDef->hasDetonator )
        return 24;
    else
        return 0;
}

void __cdecl PM_Weapon_Idle(playerState_s *ps)
{
    ps->weapFlags &= ~2u;
    ps->pm_flags &= ~0x200u;
    if ( G_IsServerGameSystem(ps->clientNum) )
        Com_Printf(19, "end weapon (idle)\n");
    ps->weaponTime = 0;
    ps->weaponDelay = 0;
    ps->weaponstate = WEAPON_READY;
    ps->weaponTimeLeft = 0;
    ps->weaponDelayLeft = 0;
    ps->weaponstateLeft = 0;
    PM_StartWeaponAnim(ps, 0, 0);
    PM_StartWeaponAnim(ps, 0, 1);
}

void __cdecl PM_Weapon(pmove_t *pm, pml_t *pml)
{
    const char *v2; // eax
    int weaponstateLeft; // [esp+0h] [ebp-14h]
    int delayedAction; // [esp+8h] [ebp-Ch]
    int delayedActiona; // [esp+8h] [ebp-Ch]
    playerState_s *ps; // [esp+Ch] [ebp-8h]
    const WeaponDef *weapDef; // [esp+10h] [ebp-4h]

    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 5280, 0, "%s", "ps") )
        __debugbreak();
    weapDef = BG_GetWeaponDef(ps->weapon);
    if ( !ps->bRunLeftGun || weapDef->bDualWield )
    {
        if ( ps->weaponstate != 21
            && ps->weaponstate != 23
            && ps->weaponstate != 22
            && ps->weaponstate != 24
            && ps->weaponstate != 1
            && ps->weaponstate != 2
            && ps->weaponstate != 3
            && ps->weaponstate != 4
            && ps->weaponstate != 5
            && (ps->weapFlags & 2) != 0 )
        {
            v2 = va("PWF_USING_OFFHAND set during non-offhand weapon state [%d]\n", ps->weaponstate);
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 5290, 0, v2) )
                __debugbreak();
        }
        if ( ps->pm_type >= 9 )
        {
            if ( (ps->eFlags2 & 0x10000000) == 0 )
                ps->weapon = 0;
            return;
        }
        if ( !CG_IsShowingZombieMap() && (ps->pm_flags & 0x400) == 0 && (ps->eFlags & 0x300) == 0 )
        {
            if ( ps->waterlevel >= 3 )
            {
                if ( player_disableWeaponsInWater->current.enabled )
                {
                    delayedAction = PM_Weapon_WeaponTimeAdjust(pm, pml);
                    if ( !BurstFirePending(ps) )
                        PM_Weapon_CheckForMelee(pm, pml, delayedAction);
                    if ( ps->weaponstate == 38
                        || ps->weaponstate >= 20 && ps->weaponstate < 25
                        || ps->weaponstate >= 17 && ps->weaponstate < 19 )
                    {
                        if ( ps->weaponstate == 38 && (ps->weapFlags & 0x2000) != 0 )
                        {
                            ps->weapFlags &= ~0x2000u;
                        }
                        else if ( ps->weaponstate == 38 )
                        {
                            return;
                        }
                    }
                    else
                    {
                        ps->weaponTime = ps->weaponstate != 39 ? 0 : 0x3E8;
                        ps->weaponstate = WEAPON_SWIM_IN;
                        ps->weaponDelay = 0;
                        ps->weapFlags |= 0x2000u;
                        PM_ExitAimDownSight(ps);
                        ps->fWeaponPosFrac = 0.0f;
                        ps->adsDelayTime = 0;
                    }
                }
            }
            else if ( ps->weaponstate == 38 )
            {
                ps->weaponstate = WEAPON_SWIM_OUT;
                ps->weaponTime = 1000;
            }
            PM_UpdateAimDownSightLerp(pm, pml);
            PM_UpdateSpinLerp(pm, pml);
            PM_UpdateScopeZoom(pm);
            PM_UpdateHoldBreath(pm, pml);
            if ( ps->bRunLeftGun && !BG_CheckForUniversalActionDW(ps) )
            {
                ps->weaponstateLeft = ps->weaponstate;
                ps->weaponDelayLeft = 0;
                ps->weaponTimeLeft = 0;
                ps->weapAnimLeft = 66;
                return;
            }
            if ( ps->bRunLeftGun && BG_CheckForUniversalActionDW(ps) && ps->weapAnimLeft == 66 )
            {
                ps->weaponstateLeft = 0;
                ps->weaponDelayLeft = 0;
                ps->weaponTimeLeft = 0;
                PM_StartWeaponAnim(ps, 0, 1);
                return;
            }
            if ( !PM_UpdateGrenadeThrow(ps, pml) )
            {
                UpdatePendingTriggerPull(pm);
                delayedActiona = PM_Weapon_WeaponTimeAdjust(pm, pml);
                if ( !BurstFirePending(ps) )
                {
                    PM_Weapon_CheckForNightVision(pm);
                    PM_Weapon_CheckForSprint(pm);
                    //BLOPS_NULLSUB();
                    PM_Weapon_CheckForDtp(pm);
                    PM_Weapon_CheckForSlide(pm);
                    PM_Weapon_CheckForDeploy(pm);
                    PM_Weapon_CheckForOffHand(pm);
                    PM_Weapon_CheckForChangeWeapon(pm);
                    PM_Weapon_CheckForReload(pm);
                    PM_Weapon_CheckFuelTime(pm, pml);
                    PM_Weapon_CheckForMelee(pm, pml, delayedActiona);
                    PM_Weapon_CheckForDetonation(pm);
                    PM_Weapon_CheckForGrenadeThrowCancel(pm);
                    PM_Weapon_CheckForMantle(pm);
                    PM_Weapon_CheckForContinuousFire(pm, delayedActiona);
                }
                if ( !PM_Weapon_CheckForRechamber(ps, delayedActiona) )
                {
                    if ( (ps->pm_flags & 1) != 0 && (pm->cmd.forwardmove || pm->cmd.rightmove) && ps->fWeaponPosFrac != 1.0
                        || ps->weaponstate == 17
                        || ps->weaponstate == 18
                        || ps->weaponstate == 19 )
                    {
                        ps->aimSpreadScale = 255.0f;
                    }
                    if ( (ps->weaponTime < 0 || ps->weaponDelay < 0)
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp",
                                    5416,
                                    0,
                                    "%s",
                                    "(ps->weaponTime >= 0) && (ps->weaponDelay >= 0)") )
                    {
                        __debugbreak();
                    }
                    CheckStackedFire(pm);
                    if ( ps->bRunLeftGun )
                    {
                        if ( !delayedActiona && (ps->weaponTimeLeft || ps->weaponDelayLeft) )
                            return;
                    }
                    else if ( !delayedActiona && (ps->weaponTime || ps->weaponDelay) )
                    {
                        return;
                    }
                    if ( ps->bRunLeftGun )
                    {
LABEL_103:
                        if ( ps->bRunLeftGun
                            && (weaponstateLeft = ps->weaponstateLeft, weaponstateLeft >= 10)
                            && weaponstateLeft <= 11 )
                        {
LABEL_106:
                            PM_Weapon_FinishReload(pm, delayedActiona);
                        }
                        else if ( ps->weapon )
                        {
                            if ( PM_Weapon_ShouldBeFiring(pm, delayedActiona, 0) )
                            {
                                if ( !PM_Weapon_CheckGrenadeHold(pm, delayedActiona)
                                    && (ps->pm_flags & 0x800) == 0
                                    && ps->weaponstate != 35 )
                                {
                                    PM_Weapon_FireWeapon(pm, delayedActiona);
                                    if ( (ps->weaponTime < 0 || ps->weaponDelay < 0)
                                        && !Assert_MyHandler(
                                                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp",
                                                    5655,
                                                    0,
                                                    "%s",
                                                    "(ps->weaponTime >= 0) && (ps->weaponDelay >= 0)") )
                                    {
                                        __debugbreak();
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        switch ( ps->weaponstate )
                        {
                            case 1:
                            case 2:
                                PM_Weapon_FinishWeaponRaise(ps);
                                break;
                            case 3:
                            case 4:
                            case 5:
                                PM_Weapon_FinishWeaponChange(pm, ps->weaponstate);
                                break;
                            case 0xA:
                            case 0xB:
                                goto LABEL_106;
                            case 0xC:
                            case 0xD:
                                PM_Weapon_FinishReloadStart(pm, delayedActiona);
                                break;
                            case 0xE:
                                PM_Weapon_FinishReloadEnd(ps);
                                break;
                            case 0x11:
                                PM_Weapon_MeleeFire(ps);
                                break;
                            case 0x12:
                                PM_Weapon_MeleeEnd(ps);
                                break;
                            case 0x13:
                            case 0x19:
                            case 0x1D:
                            case 0x20:
                            case 0x2A:
                            case 0x2B:
                            case 0x2E:
                                PM_Weapon_Idle(ps);
                                break;
                            case 0x14:
                                PM_Weapon_OffHandPrepare(pm);
                                break;
                            case 0x15:
                                PM_Weapon_OffHandHold(ps);
                                break;
                            case 0x16:
                                PM_Weapon_OffHand(pm);
                                break;
                            case 0x17:
                                PM_Weapon_OffHandStart(pm);
                                break;
                            case 0x18:
                                PM_Weapon_OffHandEnd(ps);
                                break;
                            case 0x1A:
                                PM_Detonate(ps, delayedActiona);
                                break;
                            case 0x1B:
                                Sprint_State_Loop(ps);
                                break;
                            case 0x1C:
                            case 0x24:
                                return;
                            case 0x1E:
                                ContinuousFire_State_Loop(ps);
                                goto LABEL_103;
                            case 0x21:
                                PM_Weapon_FinishNightVisionWear(ps);
                                break;
                            case 0x22:
                                PM_Weapon_FinishNightVisionRemove(ps);
                                break;
                            case 0x23:
                                PM_AddEvent(ps, 0x26u);
                                break;
                            case 0x25:
                                PM_Weapon_Idle(ps);
                                PM_AddEvent(ps, 0x28u);
                                break;
                            case 0x26:
                                PM_WeaponSwimIn(ps);
                                break;
                            case 0x27:
                                PM_WeaponSwimOut(ps);
                                break;
                            case 0x28:
                                Dtp_State_Loop(ps);
                                break;
                            case 0x29:
                                if ( Dtp_IsSliding(ps) || (ps->pm_flags & 0x400000) == 0 )
                                    Dtp_State_Out(ps);
                                break;
                            default:
                                goto LABEL_103;
                        }
                    }
                }
            }
        }
    }
}

void __cdecl PM_UpdateHoldBreath(pmove_t *pm, pml_t *pml)
{
    int lerp; // [esp+20h] [ebp-1Ch]
    int breathGaspTime; // [esp+24h] [ebp-18h]
    int weapIndex; // [esp+28h] [ebp-14h]
    float targetScale; // [esp+2Ch] [ebp-10h]
    int breathHoldTime; // [esp+30h] [ebp-Ch]
    const WeaponDef *weapDef; // [esp+34h] [ebp-8h]
    playerState_s *ps; // [esp+38h] [ebp-4h]

    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 1045, 0, "%s", "ps") )
        __debugbreak();
    weapIndex = BG_GetViewmodelWeaponIndex(ps);
    weapDef = BG_GetWeaponDef(weapIndex);
    breathHoldTime = (int)(float)(player_breath_hold_time->current.value * 1000.0);
    breathGaspTime = (int)(float)(player_breath_gasp_time->current.value * 1000.0);
    if ( (ps->perks[0] & 0x10000000) != 0 )
        breathHoldTime += (int)(float)(perk_extraBreath->current.value * 1000.0);
    if ( breathHoldTime > 0 )
    {
        if ( ps->fWeaponPosFrac == 1.0
            && weapDef->overlayReticle
            && weapDef->weapClass != WEAPCLASS_ITEM
            && pm->cmd.button_bits.testBit(0xDu) )
        {
            if ( !ps->holdBreathTimer )
                PM_StartHoldBreath(ps);
        }
        else
        {
            PM_EndHoldBreath(ps);
        }
        if ( (ps->weapFlags & 4) != 0 )
            ps->holdBreathTimer += pml->msec;
        else
            ps->holdBreathTimer -= pml->msec;
        if ( ps->holdBreathTimer < 0 )
            ps->holdBreathTimer = 0;
        if ( (ps->weapFlags & 4) != 0 && ps->holdBreathTimer > breathHoldTime )
        {
            ps->holdBreathTimer = breathGaspTime + breathHoldTime;
            PM_EndHoldBreath(ps);
        }
        if ( (ps->weapFlags & 4) != 0 )
        {
            targetScale = 0.0f;
            lerp = player_breath_hold_lerp->current.integer;
        }
        else
        {
            targetScale = (float)((float)(player_breath_gasp_scale->current.value - 1.0)
                                                    * (float)((float)ps->holdBreathTimer / (float)(breathGaspTime + breathHoldTime)))
                                    + 1.0;
            lerp = player_breath_gasp_lerp->current.integer;
        }
        ps->holdBreathScale = DiffTrack(
                                                        (float)((float)(targetScale - 1.0) * ps->fWeaponPosFrac) + 1.0,
                                                        ps->holdBreathScale,
                                                        *(float *)&lerp,
                                                        pml->frametime);
    }
    else
    {
        PM_EndHoldBreath(ps);
        ps->holdBreathScale = 1.0f;
        ps->holdBreathTimer = 0;
    }
}

void __cdecl PM_StartHoldBreath(playerState_s *ps)
{
    ps->weapFlags |= 4u;
}

void __cdecl PM_EndHoldBreath(playerState_s *ps)
{
    ps->weapFlags &= ~4u;
}

int __cdecl PM_Weapon_CheckForRechamber(playerState_s *ps, int delayedAction)
{
    const WeaponDef *weapDef; // [esp+18h] [ebp-4h]

    weapDef = BG_GetWeaponDef(ps->weapon);
    if ( ps->weaponstate >= 20 && ps->weaponstate <= 25 )
        return 0;
    if ( ps->weaponstate >= 40 && ps->weaponstate <= 42 )
        return 0;
    if ( weapDef->bBoltAction && BG_PlayerWeaponNeedsRechambered(ps, ps->weapon) )
    {
        if ( ps->weaponstate == 7 )
        {
            if ( delayedAction )
            {
                BG_PlayerWeaponSetNeedsRechamber(ps, ps->weapon, 0);
                PM_AddEvent(ps, 0x22u);
                if ( ps->weaponTime )
                    return 1;
            }
        }
        if ( !ps->weaponTime
            || ps->weaponstate != 6
            && ps->weaponstate != 7
            && ps->weaponstate != 31
            && ps->weaponstate != 17
            && ps->weaponstate != 18
            && ps->weaponstate != 19
            && !ps->weaponDelay )
        {
            if ( ps->weaponstate == 7 )
            {
                PM_Weapon_FinishRechamber(ps);
            }
            else if ( !ps->weaponstate )
            {
                if ( ps->fWeaponPosFrac <= 0.75 )
                    PM_StartWeaponAnim(ps, 6, 0);
                else
                    PM_StartWeaponAnim(ps, 9, 0);
                ps->weaponstate = WEAPON_RECHAMBERING;
                ps->weaponTime = weapDef->iRechamberTime;
                if ( weapDef->iRechamberBoltTime && weapDef->iRechamberBoltTime < weapDef->iRechamberTime )
                    ps->weaponDelay = weapDef->iRechamberBoltTime;
                else
                    ps->weaponDelay = 1;
                PM_AddEvent(ps, 0x21u);
            }
        }
    }
    return 0;
}

bool __cdecl BG_PlayerWeaponNeedsRechambered(const playerState_s *ps, unsigned int weaponIndex)
{
    const PlayerHeldWeapon *heldWeapon; // [esp+8h] [ebp-4h]

    if ( !ps && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\bgame\\../bgame/bg_weapons.h", 382, 0, "%s", "ps") )
        __debugbreak();
    heldWeapon = BG_GetHeldWeaponConst(ps, weaponIndex);
    return heldWeapon && heldWeapon->needsRechamber;
}

void __cdecl BG_PlayerWeaponSetNeedsRechamber(playerState_s *ps, unsigned int weaponIndex, bool needsRechamber)
{
    PlayerHeldWeapon *heldWeapon; // [esp+8h] [ebp-4h]

    if ( !ps && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\bgame\\../bgame/bg_weapons.h", 452, 0, "%s", "ps") )
        __debugbreak();
    heldWeapon = BG_GetHeldWeapon(ps, weaponIndex);
    if ( heldWeapon )
        heldWeapon->needsRechamber = needsRechamber;
}

void __cdecl PM_Weapon_FinishRechamber(playerState_s *ps)
{
    PM_ContinueWeaponAnim(ps, 0, 0);
    ps->weaponstate = WEAPON_READY;
}

void __cdecl PM_ContinueWeaponAnim(playerState_s *ps, int anim, int leftAnim)
{
    if ( ((ps->weapAnim & 0xFFFFFBFF) != anim || ps->bRunLeftGun)
        && ((ps->weapAnimLeft & 0xFFFFFBFF) != anim || !ps->bRunLeftGun) )
    {
        PM_StartWeaponAnim(ps, anim, leftAnim);
    }
}

void __cdecl PM_Weapon_FinishWeaponChange(pmove_t *pm, int previousWeaponState)
{
    bool HasWeapon; // eax
    bool v3; // [esp+8h] [ebp-68h]
    char quick; // [esp+3Fh] [ebp-31h]
    bool noammo; // [esp+40h] [ebp-30h]
    float aimspread; // [esp+48h] [ebp-28h]
    const WeaponVariantDef *weapVariantDef; // [esp+4Ch] [ebp-24h]
    unsigned int oldweapon; // [esp+50h] [ebp-20h]
    unsigned int anim; // [esp+58h] [ebp-18h]
    unsigned int weapontime; // [esp+5Ch] [ebp-14h]
    const WeaponDef *weapDef; // [esp+60h] [ebp-10h]
    playerState_s *ps; // [esp+64h] [ebp-Ch]
    unsigned int newweapon; // [esp+6Ch] [ebp-4h]

    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 1815, 0, "%s", "ps") )
        __debugbreak();
    if ( ps->weaponstate != 3
        && ps->weaponstate != 4
        && ps->weaponstate != 5
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp",
                    1817,
                    0,
                    "%s",
                    "WEAPONSTATE_DROPPING( ps->weaponstate )") )
    {
        __debugbreak();
    }
    BG_GetWeaponDef(ps->weapon);
    if ( Mantle_IsWeaponInactive(ps)
        || door_breach_weapondrop->current.enabled && (ps->pm_flags & 0x1000000) != 0
        || (ps->weapFlags & 0x20000) != 0
        || (ps->pm_flags & 8) != 0
        || !BG_PlayerHasWeapon(ps, pm->cmd.weapon)
        || (ps->weapFlags & 0x80) != 0
        || BG_UsingVehicleWeapon(ps) && !BG_GetWeaponDef(pm->cmd.weapon)->bCanUseInVehicle )
    {
        newweapon = 0;
        HasWeapon = BG_PlayerHasWeapon(ps, 0);
    }
    else
    {
        newweapon = pm->cmd.weapon;
        if ( newweapon >= BG_GetNumWeapons() )
            newweapon = 0;
        HasWeapon = BG_PlayerHasWeapon(ps, newweapon);
    }
    if ( !HasWeapon )
        newweapon = 0;
    oldweapon = ps->weapon;
    AssignToSmallerType<unsigned short>(&ps->weapon, newweapon);
    ps->lastWeaponAltModeSwitch = BG_GetValidPrimaryWeaponForAltMode(ps, newweapon, pm->cmd.lastWeaponAltModeSwitch);
    if ( ps->weapon != newweapon
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp",
                    1846,
                    0,
                    "%s",
                    "(uint)ps->weapon == newweapon") )
    {
        __debugbreak();
    }
    weapDef = BG_GetWeaponDef(ps->weapon);
    if ( weapDef->inventoryType == WEAPINVENTORY_ALTMODE && !ps->lastWeaponAltModeSwitch )
    {
        ps->weapon = 0;
        weapDef = BG_GetWeaponDef(ps->weapon);
    }
    weapVariantDef = BG_GetWeaponVariantDef(ps->weapon);
    ps->adsZoomSelect = 0;
    ps->adsZoomLatchTime = 0;
    if ( (ps->weapFlags & 0x20000) != 0 )
    {
        ps->weaponstate = WEAPON_OFFHAND_INIT;
    }
    else if ( oldweapon == newweapon )
    {
        ps->weaponstate = WEAPON_READY;
        ps->weaponstateLeft = 0;
        PM_StartWeaponAnim(ps, 0, 0);
        if ( weapDef->bDualWield )
            PM_StartWeaponAnim(ps, 0, 1);
    }
    else
    {
        v3 = !BG_PlayerEverHadWeapon(ps, ps->weapon) && ps->pm_type != 8 && ps->pm_type != 6;
        BG_PlayerSetEverHadWeapon(ps, ps->weapon, 1);
        if ( previousWeaponState == 5 )
        {
            weapontime = weapVariantDef->iAltRaiseTime;
            if ( ps->aimSpreadScale >= 128.0 )
                aimspread = ps->aimSpreadScale;
            else
                aimspread = 128.0f;
            anim = 22;
        }
        else
        {
            quick = BG_IsQuickSwitch(oldweapon, newweapon, previousWeaponState);
            aimspread = 255.0f;
            noammo = PM_WeaponClipEmpty(ps);
            if ( weapDef->bNoDropsOrRaises )
            {
                anim = 0;
                weapontime = 0;
            }
            else if ( noammo )
            {
                anim = 26;
                weapontime = weapDef->iEmptyRaiseTime;
            }
            else if ( v3 )
            {
                anim = 14;
                weapontime = weapDef->iFirstRaiseTime;
            }
            else if ( quick )
            {
                anim = 24;
                weapontime = weapDef->quickRaiseTime;
            }
            else
            {
                anim = 13;
                weapontime = weapDef->iRaiseTime;
            }
        }
        if ( weapDef->bNoDropsOrRaises )
        {
            ps->weaponstate = (weaponstate_t)anim;
            ps->weaponTime = weapontime;
            ps->aimSpreadScale = aimspread;
            PM_SetProneMovementOverride(ps);
            PM_StartWeaponAnim(ps, 0, 0);
            if ( weapDef->bDualWield )
                PM_StartWeaponAnim(ps, 0, 1);
        }
        else
        {
            PM_Weapon_BeginWeaponRaise(ps, anim, weapontime, aimspread, previousWeaponState == 5);
        }
        if ( !weapDef
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 1943, 0, "%s", "weapDef") )
        {
            __debugbreak();
        }
        BG_TakeClipOnlyWeaponIfEmpty(ps, oldweapon);
        if ( oldweapon && previousWeaponState != 5 )
        {
            if ( v3 )
                PM_AddEvent(ps, 0x19u);
            else
                PM_AddEvent(ps, 0x18u);
            if ( pm->ps->pm_type != 6
                && pm->ps->pm_type != 7
                && pm->ps->pm_type != 8
                && (ps->pm_flags & 0xA008) == 0
                && pm->ps->viewHeightCurrent == (float)pm->ps->viewHeightTarget )
            {
                BG_AnimScriptEvent(pm, ANIM_ET_RAISEWEAPON, 0, 1);
            }
        }
    }
}

bool __cdecl BG_PlayerEverHadWeapon(const playerState_s *ps, unsigned int weaponIndex)
{
    const PlayerHeldWeapon *heldWeapon; // [esp+8h] [ebp-4h]

    if ( !ps && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\bgame\\../bgame/bg_weapons.h", 356, 0, "%s", "ps") )
        __debugbreak();
    heldWeapon = BG_GetHeldWeaponConst(ps, weaponIndex);
    return heldWeapon && heldWeapon->heldBefore;
}

void __cdecl BG_PlayerSetEverHadWeapon(playerState_s *ps, unsigned int weaponIndex, bool hadWeapon)
{
    PlayerHeldWeapon *heldWeapon; // [esp+8h] [ebp-4h]

    if ( !ps && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\bgame\\../bgame/bg_weapons.h", 369, 0, "%s", "ps") )
        __debugbreak();
    heldWeapon = BG_GetHeldWeapon(ps, weaponIndex);
    if ( heldWeapon )
        heldWeapon->heldBefore = hadWeapon;
}

void __cdecl PM_Weapon_BeginWeaponRaise(
                playerState_s *ps,
                unsigned int anim,
                unsigned int time,
                float aim,
                int altSwitch)
{
    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 1716, 0, "%s", "ps") )
        __debugbreak();
    if ( aim < 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 1717, 0, "%s", "aim >= 0") )
    {
        __debugbreak();
    }
    ps->weaponstate = (weaponstate_t)((altSwitch != 0) + 1);
    ps->weaponTime = time;
    ps->aimSpreadScale = aim;
    PM_SetProneMovementOverride(ps);
    PM_StartWeaponAnim(ps, anim, 0);
    PM_StartWeaponAnim(ps, anim, 1);
}

void __cdecl BG_TakeClipOnlyWeaponIfEmpty(playerState_s *ps, unsigned int weaponIndex)
{
    if ( weaponIndex
        && BG_PlayerHasWeapon(ps, weaponIndex)
        && BG_WeaponIsClipOnly(weaponIndex)
        && !BG_GetAmmoInClip(ps, weaponIndex)
        && !BG_GetAmmoNotInClip(ps, weaponIndex)
        && !BG_GetWeaponDef(weaponIndex)->hasDetonator
        && !BG_GetWeaponDef(weaponIndex)->plantable )
    {
        BG_TakePlayerWeapon(ps, weaponIndex);
    }
}

int __cdecl BG_GetAmmoNotInClip(const playerState_s *ps, unsigned int weaponIndex)
{
    int v2; // eax

    if ( !ps
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\bgame\\../bgame/bg_weapons_ammo.h", 153, 0, "%s", "ps") )
    {
        __debugbreak();
    }
    v2 = BG_AmmoForWeapon(weaponIndex);
    return BG_GetAmmoNotInClipForAmmoIndex(ps, v2);
}

int __cdecl BG_GetAmmoNotInClipForAmmoIndex(const playerState_s *ps, int ammoIndex)
{
    int slot; // [esp+0h] [ebp-4h]

    if ( !ps
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\bgame\\../bgame/bg_weapons_ammo.h", 138, 0, "%s", "ps") )
    {
        __debugbreak();
    }
    for ( slot = 0; slot < 15; ++slot )
    {
        if ( ps->ammoNotInClip[slot].ammoIndex == ammoIndex )
            return ps->ammoNotInClip[slot].count;
    }
    return 0;
}

unsigned __int16 __cdecl BG_GetValidPrimaryWeaponForAltMode(
                const playerState_s *ps,
                unsigned int newWeapon,
                unsigned int lastWeaponAltModeSwitch)
{
    if ( !newWeapon )
        lastWeaponAltModeSwitch = 0;
    if ( lastWeaponAltModeSwitch >= BG_GetNumWeapons() )
        lastWeaponAltModeSwitch = 0;
    if ( !BG_PlayerHasWeapon(ps, lastWeaponAltModeSwitch) )
        lastWeaponAltModeSwitch = 0;
    return lastWeaponAltModeSwitch;
}

void __cdecl PM_Weapon_FinishWeaponRaise(playerState_s *ps)
{
    if ( ps->weaponstate != 1
        && ps->weaponstate != 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp",
                    1970,
                    0,
                    "%s",
                    "WEAPONSTATE_RAISING( ps->weaponstate )") )
    {
        __debugbreak();
    }
    ps->weaponstate = WEAPON_READY;
    ps->weaponstateLeft = 0;
    PM_StartWeaponAnim(ps, 0, 0);
    PM_StartWeaponAnim(ps, 0, 1);
}

void __cdecl PM_Weapon_FinishReloadStart(pmove_t *pm, int delayedAction)
{
    const WeaponVariantDef *weapVarDef; // [esp+14h] [ebp-Ch]
    const WeaponDef *weapDef; // [esp+18h] [ebp-8h]
    playerState_s *ps; // [esp+1Ch] [ebp-4h]

    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 2118, 0, "%s", "ps") )
        __debugbreak();
    weapDef = BG_GetWeaponDef(ps->weapon);
    weapVarDef = BG_GetWeaponVariantDef(ps->weapon);
    if ( ps->weaponstate != 12
        && ps->weaponstate != 13
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp",
                    2122,
                    0,
                    "%s",
                    "(ps->weaponstate == WEAPON_RELOAD_START) || (ps->weaponstate == WEAPON_RELOAD_START_INTERUPT)") )
    {
        __debugbreak();
    }
    if ( delayedAction )
        PM_Weapon_ReloadDelayedAction(ps);
    if ( !ps->weaponTime )
    {
        if ( weapDef->bSegmentedReload && pm->cmd.button_bits.testBit(0) )
            ps->weaponstate = WEAPON_RELOAD_START_INTERUPT;
        if ( ps->weaponstate == WEAPON_RELOAD_START_INTERUPT && BG_GetAmmoInClipForWeaponDef(ps, weapVarDef) || !PM_Weapon_AllowReload(ps) )
        {
            BG_PlayerWeaponSetNeedsRechamber(ps, ps->weapon, 0);
            if ( weapDef->iReloadEndTime )
            {
                ps->weaponstate = WEAPON_RELOAD_END;
                PM_StartWeaponAnim(ps, 18, 0);
                ps->weaponTime = weapDef->iReloadEndTime;
                PM_AddEvent(ps, 0x15u);
            }
            else
            {
                ps->weaponstate = WEAPON_READY;
                PM_StartWeaponAnim(ps, 0, 0);
                if ( weapDef->bDualWield )
                    PM_StartWeaponAnim(ps, 0, 1);
            }
        }
        else
        {
            PM_SetReloadingState(ps);
        }
    }
}

void __cdecl PM_SetReloadingState(playerState_s *ps)
{
    int *weaponTime; // [esp+18h] [ebp-18h]
    bool quickReload; // [esp+1Ch] [ebp-14h]
    const WeaponVariantDef *weapVariantDef; // [esp+20h] [ebp-10h]
    unsigned int weaponIndex; // [esp+24h] [ebp-Ch]
    const WeaponDef *weapDef; // [esp+28h] [ebp-8h]
    int *weaponState; // [esp+2Ch] [ebp-4h]

    weaponState = BG_GetWeaponState(ps, ps->bRunLeftGun);
    weaponTime = BG_GetWeaponTime(ps, ps->bRunLeftGun);
    weaponIndex = PM_GetWeaponIndexForHand(ps);
    weapVariantDef = BG_GetWeaponVariantDef(weaponIndex);
    weapDef = BG_GetWeaponDef(weaponIndex);
    quickReload = BG_PlayerWeaponQuickReload(ps, weaponIndex);
    if ( !weapDef->unlimitedAmmo )
    {
        if ( BG_GetAmmoInClip(ps, weaponIndex) || weapDef->weapType )
        {
            if ( weapVariantDef->bDualMag && quickReload )
            {
                PM_StartWeaponAnim(ps, 19, ps->bRunLeftGun);
                *weaponTime = weapVariantDef->iReloadQuickTime;
            }
            else
            {
                PM_StartWeaponAnim(ps, 15, ps->bRunLeftGun);
                *weaponTime = weapVariantDef->iReloadTime;
            }
            PM_AddEvent(ps, 0x12u);
        }
        else
        {
            if ( weapVariantDef->bDualMag && quickReload )
            {
                PM_StartWeaponAnim(ps, 20, ps->bRunLeftGun);
                *weaponTime = weapVariantDef->iReloadQuickEmptyTime;
            }
            else
            {
                PM_StartWeaponAnim(ps, 16, ps->bRunLeftGun);
                *weaponTime = weapVariantDef->iReloadEmptyTime;
            }
            PM_AddEvent(ps, 0x13u);
        }
        if ( *weaponState == 13 )
            *weaponState = 11;
        else
            *weaponState = 10;
        PM_SetWeaponReloadAddAmmoDelay(ps);
    }
}

bool __cdecl BG_PlayerWeaponQuickReload(const playerState_s *ps, unsigned int weaponIndex)
{
    const PlayerHeldWeapon *heldWeapon; // [esp+Ch] [ebp-4h]

    if ( !ps && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\bgame\\../bgame/bg_weapons.h", 419, 0, "%s", "ps") )
        __debugbreak();
    heldWeapon = BG_GetHeldWeaponConst(ps, weaponIndex);
    return BG_GetWeaponVariantDef(weaponIndex)->bDualMag && heldWeapon->quickReload;
}

int *__cdecl BG_GetWeaponTime(playerState_s *ps, bool leftGun)
{
    if ( leftGun )
        return &ps->weaponTimeLeft;
    else
        return &ps->weaponTime;
}

void __cdecl PM_SetWeaponReloadAddAmmoDelay(playerState_s *ps)
{
    int iReloadAddTime; // [esp+34h] [ebp-20h]
    int *weaponDelay; // [esp+38h] [ebp-1Ch]
    int *weaponTime; // [esp+3Ch] [ebp-18h]
    int reloadTime; // [esp+40h] [ebp-14h]
    const WeaponVariantDef *weapVariantDef; // [esp+44h] [ebp-10h]
    unsigned int weaponIndex; // [esp+48h] [ebp-Ch]
    const WeaponDef *weapDef; // [esp+4Ch] [ebp-8h]
    int *weaponState; // [esp+50h] [ebp-4h]

    weaponIndex = PM_GetWeaponIndexForHand(ps);
    weapVariantDef = BG_GetWeaponVariantDef(weaponIndex);
    weapDef = BG_GetWeaponDef(weaponIndex);
    weaponState = BG_GetWeaponState(ps, ps->bRunLeftGun);
    weaponTime = BG_GetWeaponTime(ps, ps->bRunLeftGun);
    weaponDelay = BG_GetWeaponDelay(ps, ps->bRunLeftGun);
    if ( *weaponState == 12 || *weaponState == 13 )
    {
        if ( weapDef->iReloadStartAddTime )
        {
            if ( weapDef->iReloadStartAddTime >= weapDef->iReloadStartTime )
                reloadTime = weapDef->iReloadStartTime;
            else
                reloadTime = weapDef->iReloadStartAddTime;
        }
        else
        {
            reloadTime = 0;
        }
    }
    else
    {
        if ( BG_PlayerWeaponQuickReload(ps, weaponIndex) )
            iReloadAddTime = weapDef->iReloadQuickAddTime;
        else
            iReloadAddTime = weapDef->iReloadAddTime;
        if ( BG_GetAmmoInClip(ps, weaponIndex) || weapDef->weapType )
        {
            reloadTime = weapVariantDef->iReloadTime;
        }
        else
        {
            reloadTime = weapVariantDef->iReloadEmptyTime;
            if ( BG_PlayerWeaponQuickReload(ps, weaponIndex) && weapDef->iReloadQuickEmptyAddTime )
            {
                iReloadAddTime = weapDef->iReloadQuickEmptyAddTime;
            }
            else if ( weapDef->iReloadEmptyAddTime )
            {
                iReloadAddTime = weapDef->iReloadEmptyAddTime;
            }
        }
        if ( iReloadAddTime && iReloadAddTime < reloadTime )
            reloadTime = iReloadAddTime;
    }
    if ( weapDef->bBoltAction && BG_PlayerWeaponNeedsRechambered(ps, weaponIndex) )
    {
        if ( !reloadTime )
            reloadTime = *weaponTime;
        if ( weapDef->iRechamberBoltTime < reloadTime )
            reloadTime = weapDef->iRechamberBoltTime;
        if ( !reloadTime )
            reloadTime = 1;
        *weaponDelay = reloadTime;
    }
    else if ( reloadTime )
    {
        *weaponDelay = reloadTime;
    }
}

int *__cdecl BG_GetWeaponDelay(playerState_s *ps, bool leftGun)
{
    if ( leftGun )
        return &ps->weaponDelayLeft;
    else
        return &ps->weaponDelay;
}

int __cdecl PM_Weapon_AllowReload(playerState_s *ps)
{
    int AmmoInClipForWeaponDef; // esi
    int ClipSize; // esi
    const WeaponVariantDef *weapVarDef; // [esp+68h] [ebp-10h]
    bool fuelReload; // [esp+6Fh] [ebp-9h]
    unsigned int weaponIndex; // [esp+70h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+74h] [ebp-4h]

    weaponIndex = PM_GetWeaponIndexForHand(ps);
    weapDef = BG_GetWeaponDef(weaponIndex);
    weapVarDef = BG_GetWeaponVariantDef(weaponIndex);
    fuelReload = 0;
    if ( weapDef->fuelTankWeapon )
        fuelReload = BG_GetFuelTankTime(ps, weaponIndex) > 0;
    if ( BG_GetAmmoNotInClip(ps, weaponIndex) )
    {
        AmmoInClipForWeaponDef = BG_GetAmmoInClipForWeaponDef(ps, weapVarDef);
        if ( AmmoInClipForWeaponDef < BG_GetClipSize(weaponIndex) || fuelReload )
        {
            if ( !weapDef->bNoPartialReload )
                return 1;
            if ( weapDef->iReloadAmmoAdd && weapDef->iReloadAmmoAdd < BG_GetClipSize(weaponIndex) )
            {
                ClipSize = BG_GetClipSize(weaponIndex);
                if ( ClipSize - BG_GetAmmoInClipForWeaponDef(ps, weapVarDef) >= weapDef->iReloadAmmoAdd )
                    return 1;
            }
            else if ( !BG_GetAmmoInClipForWeaponDef(ps, weapVarDef) )
            {
                return 1;
            }
        }
    }
    return 0;
}

int __cdecl BG_GetClipSize(unsigned int weaponIndex)
{
    const WeaponVariantDef *weapVariantDef; // [esp+14h] [ebp-4h]

    weapVariantDef = BG_GetWeaponVariantDef(weaponIndex);
    if ( weapVariantDef->iClipSize <= 1 )
        return weapVariantDef->iClipSize;
    if ( SnapFloatToInt((float)((float)weapVariantDef->iClipSize * player_clipSizeMultiplier->current.value)) > 0 )
        return SnapFloatToInt((float)((float)weapVariantDef->iClipSize * player_clipSizeMultiplier->current.value));
    return 1;
}

void __cdecl PM_Weapon_ReloadDelayedAction(playerState_s *ps)
{
    bool quickReload; // [esp+40h] [ebp-28h]
    int iReloadAddTime; // [esp+44h] [ebp-24h]
    int reloadTime; // [esp+48h] [ebp-20h]
    int reloadTimea; // [esp+48h] [ebp-20h]
    int rechamberTime; // [esp+4Ch] [ebp-1Ch]
    int *weaponDelay; // [esp+50h] [ebp-18h]
    int *weaponTime; // [esp+54h] [ebp-14h]
    const WeaponVariantDef *weapVariantDef; // [esp+58h] [ebp-10h]
    unsigned int weaponIndex; // [esp+5Ch] [ebp-Ch]
    const WeaponDef *weapDef; // [esp+60h] [ebp-8h]
    int *weaponState; // [esp+64h] [ebp-4h]

    weaponIndex = PM_GetWeaponIndexForHand(ps);
    weapVariantDef = BG_GetWeaponVariantDef(weaponIndex);
    weapDef = BG_GetWeaponDef(weaponIndex);
    weaponState = BG_GetWeaponState(ps, ps->bRunLeftGun);
    weaponTime = BG_GetWeaponTime(ps, ps->bRunLeftGun);
    weaponDelay = BG_GetWeaponDelay(ps, ps->bRunLeftGun);
    if ( weapDef->bBoltAction && BG_PlayerWeaponNeedsRechambered(ps, weaponIndex) )
    {
        BG_PlayerWeaponSetNeedsRechamber(ps, weaponIndex, 0);
        PM_AddEvent(ps, 0x22u);
        if ( *weaponState != 12 && *weaponState != 13 || weapDef->iReloadStartAddTime )
        {
            if ( !*weaponTime )
                goto LABEL_28;
            if ( *weaponState == 12 || *weaponState == 13 )
            {
                if ( !weapDef->iReloadStartAddTime
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp",
                                2053,
                                0,
                                "%s",
                                "weapDef->iReloadStartAddTime") )
                {
                    __debugbreak();
                }
                reloadTime = weapDef->iReloadStartAddTime >= weapDef->iReloadStartTime
                                     ? weapDef->iReloadStartTime
                                     : weapDef->iReloadStartAddTime;
            }
            else
            {
                iReloadAddTime = weapDef->iReloadAddTime;
                if ( BG_GetAmmoInClipForWeaponDef(ps, weapVariantDef) || weapDef->weapType )
                {
                    reloadTime = weapVariantDef->iReloadTime;
                }
                else
                {
                    reloadTime = weapVariantDef->iReloadEmptyTime;
                    if ( weapDef->iReloadEmptyAddTime )
                        iReloadAddTime = weapDef->iReloadEmptyAddTime;
                }
                if ( iReloadAddTime && iReloadAddTime < reloadTime )
                    reloadTime = iReloadAddTime;
            }
            rechamberTime = weapDef->iRechamberBoltTime >= reloadTime ? 1 : weapDef->iRechamberBoltTime;
            reloadTimea = reloadTime - rechamberTime;
            if ( reloadTimea >= 1 )
                *weaponDelay = reloadTimea;
            else
LABEL_28:
                PM_ReloadClip(ps);
        }
    }
    else
    {
        PM_ReloadClip(ps);
        quickReload = BG_PlayerWeaponQuickReload(ps, ps->weapon);
        if ( weapVariantDef->bDualMag )
            BG_PlayerSetWeaponQuickReload(ps, ps->weapon, !quickReload);
    }
}

void __cdecl BG_PlayerSetWeaponQuickReload(playerState_s *ps, unsigned int weaponIndex, bool quickReload)
{
    const WeaponVariantDef *weapVariantDef; // [esp+8h] [ebp-8h]
    PlayerHeldWeapon *heldWeapon; // [esp+Ch] [ebp-4h]

    if ( !ps && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\bgame\\../bgame/bg_weapons.h", 432, 0, "%s", "ps") )
        __debugbreak();
    heldWeapon = BG_GetHeldWeapon(ps, weaponIndex);
    weapVariantDef = BG_GetWeaponVariantDef(weaponIndex);
    if ( heldWeapon )
        heldWeapon->quickReload = weapVariantDef->bDualMag && quickReload;
}

void __cdecl PM_Weapon_FinishReload(pmove_t *pm, int delayedAction)
{
    int *weaponTime; // [esp+Ch] [ebp-18h]
    unsigned int weaponIndex; // [esp+14h] [ebp-10h]
    const WeaponDef *weapDef; // [esp+18h] [ebp-Ch]
    playerState_s *ps; // [esp+1Ch] [ebp-8h]
    int *weaponState; // [esp+20h] [ebp-4h]

    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 2168, 0, "%s", "ps") )
        __debugbreak();
    weaponState = BG_GetWeaponState(ps, ps->bRunLeftGun);
    weaponTime = BG_GetWeaponTime(ps, ps->bRunLeftGun);
    if ( *weaponState != 10
        && *weaponState != 11
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp",
                    2172,
                    0,
                    "%s",
                    "(*weaponState == WEAPON_RELOADING) || (*weaponState == WEAPON_RELOADING_INTERUPT)") )
    {
        __debugbreak();
    }
    weaponIndex = PM_GetWeaponIndexForHand(ps);
    BG_GetWeaponVariantDef(weaponIndex);
    weapDef = BG_GetWeaponDef(weaponIndex);
    if ( !delayedAction || (PM_Weapon_ReloadDelayedAction(ps), !*weaponTime) )
    {
        if ( !*weaponTime )
        {
            if ( weapDef->bSegmentedReload && pm->cmd.button_bits.testBit(0) )
                *weaponState = 11;
            BG_PlayerWeaponSetNeedsRechamber(ps, ps->weapon, 0);
            if ( !weapDef->bSegmentedReload )
                goto LABEL_20;
            if ( *weaponState != 11 && PM_Weapon_AllowReload(ps) )
            {
                PM_SetReloadingState(ps);
                return;
            }
            if ( weapDef->iReloadEndTime )
            {
                *weaponState = 14;
                PM_StartWeaponAnim(ps, 18, 0);
                *weaponTime = weapDef->iReloadEndTime;
                PM_AddEvent(ps, 0x15u);
            }
            else
            {
LABEL_20:
                *weaponState = 0;
                PM_StartWeaponAnim(ps, 0, ps->bRunLeftGun);
            }
        }
    }
}

void __cdecl PM_Weapon_FinishReloadEnd(playerState_s *ps)
{
    if ( ps->weaponstate != 14
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp",
                    2229,
                    0,
                    "%s",
                    "ps->weaponstate == WEAPON_RELOAD_END") )
    {
        __debugbreak();
    }
    ps->weaponstate = WEAPON_READY;
    PM_StartWeaponAnim(ps, 0, 0);
}

void __cdecl PM_Weapon_CheckFuelTime(pmove_t *pm, pml_t *pml)
{
    const WeaponVariantDef *weapVarDef; // [esp+0h] [ebp-10h]
    const WeaponDef *weapDef; // [esp+4h] [ebp-Ch]
    playerState_s *ps; // [esp+8h] [ebp-8h]
    int *weaponState; // [esp+Ch] [ebp-4h]

    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 2245, 0, "%s", "ps") )
        __debugbreak();
    weapDef = BG_GetWeaponDef(ps->weapon);
    weapVarDef = BG_GetWeaponVariantDef(ps->weapon);
    weaponState = BG_GetWeaponState(ps, ps->bRunLeftGun);
    if ( weapDef->fuelTankWeapon && (*weaponState == 6 || *weaponState == 7 || *weaponState == 31) )
    {
        BG_AddFuelTankTime(ps, ps->weapon, pml->msec);
        if ( BG_GetFuelTankTime(ps, ps->weapon) > weapDef->iTankLifeTime )
        {
            BG_SetFuelTankTime(ps, ps->weapon, weapDef->iTankLifeTime);
            BG_SetAmmoInClip(ps, weapVarDef->iClipIndex, 0);
            *weaponState = 0;
        }
    }
}

void __cdecl PM_Weapon_CheckForReload(pmove_t *pm)
{
    const WeaponVariantDef *weapVarDef; // [esp+20h] [ebp-20h]
    int doReload; // [esp+24h] [ebp-1Ch]
    int *weaponTime; // [esp+28h] [ebp-18h]
    unsigned int weaponIndex; // [esp+2Ch] [ebp-14h]
    const WeaponDef *weapDef; // [esp+30h] [ebp-10h]
    int *weaponState; // [esp+34h] [ebp-Ch]
    playerState_s *ps; // [esp+38h] [ebp-8h]
    bool reloadRequested; // [esp+3Ch] [ebp-4h]

    doReload = 0;
    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 2329, 0, "%s", "ps") )
        __debugbreak();
    weaponState = BG_GetWeaponState(ps, ps->bRunLeftGun);
    weaponTime = BG_GetWeaponTime(ps, ps->bRunLeftGun);
    weaponIndex = PM_GetWeaponIndexForHand(ps);
    weapDef = BG_GetWeaponDef(weaponIndex);
    weapVarDef = BG_GetWeaponVariantDef(weaponIndex);
    if ( !weapDef->bDualWield )
        ps->weapFlags &= ~0x10000u;
    if ( *weaponState == 19 && PM_WeaponClipEmpty(ps) && !BG_IsUseAsMeleeWeapon(weaponIndex) )
    {
        *weaponState = (*weaponState == 5) + 1;
        *weaponTime = weapDef->iEmptyRaiseTime;
        PM_StartWeaponAnim(ps, 26, 0);
    }
    else if ( (*weaponState < 20 || *weaponState > 25)
                 && *weaponState != 17
                 && *weaponState != 18
                 && *weaponState != 19
                 && (*weaponState < 40 || *weaponState > 42)
                 && (ps->pm_flags & 4) == 0
                 && (ps->waterlevel < 3 || !player_disableWeaponsInWater->current.enabled) )
    {
        reloadRequested = 0;
        if ( (ps->pm_flags & 0x800) == 0 )
        {
            reloadRequested = pm->cmd.button_bits.testBit(4u);
            if ( (ps->weapFlags & 1) != 0 )
            {
                ps->weapFlags &= ~1u;
                reloadRequested = 1;
            }
            if ( (ps->weapFlags & 0x10000) != 0 && ps->bRunLeftGun )
            {
                ps->weapFlags &= ~0x10000u;
                reloadRequested = 1;
            }
        }
        if ( weapDef->bSegmentedReload
            && (*weaponState == 12 || *weaponState == 10)
            && pm->cmd.button_bits.testBit(0)
            && !pm->oldcmd.button_bits.testBit(0) )
        {
            static const float MY_RELOADSTART_INTERUPT_IGNORE_FRAC = 0.4f;

            if ( *weaponState == 12 && weapDef->iReloadStartTime )
            {
                if ( (float)((float)(weapDef->iReloadStartTime - *weaponTime) / (float)weapDef->iReloadStartTime) > MY_RELOADSTART_INTERUPT_IGNORE_FRAC )
                    *weaponState = 13;
            }
            else if ( *weaponState == 10 )
            {
                *weaponState = 11;
            }
        }
        if ( reloadRequested && PM_Weapon_AllowReload(ps) )
            doReload = 1;
        if ( weapDef->fuelTankWeapon
            && BG_GetAmmoNotInClip(ps, weaponIndex)
            && BG_PlayerFuelTankTime(ps, ps->weapon) >= weapDef->iTankLifeTime
            && PM_CheckForReloadInterferingWeaponStates(pm, ps, *weaponState) )
        {
            doReload = 1;
        }
        if ( !BG_GetAmmoInClipForWeaponDef(ps, weapVarDef)
            && BG_GetAmmoNotInClip(ps, weaponIndex)
            && PM_CheckForReloadInterferingWeaponStates(pm, ps, *weaponState) )
        {
            doReload = 1;
        }
        if ( doReload )
            PM_BeginWeaponReload(pm);
    }
}

int __cdecl BG_PlayerFuelTankTime(const playerState_s *ps, unsigned int weaponIndex)
{
    const PlayerHeldWeapon *heldWeapon; // [esp+8h] [ebp-4h]

    if ( !ps && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\bgame\\../bgame/bg_weapons.h", 526, 0, "%s", "ps") )
        __debugbreak();
    heldWeapon = BG_GetHeldWeaponConst(ps, weaponIndex);
    if ( heldWeapon )
        return heldWeapon->fuelTankTime;
    else
        return 0;
}

void __cdecl PM_BeginWeaponReload(pmove_t *pm)
{
    int *weaponTime; // [esp+0h] [ebp-18h]
    unsigned int *weaponShotCount; // [esp+4h] [ebp-14h]
    unsigned int weaponIndex; // [esp+8h] [ebp-10h]
    const WeaponDef *weapDef; // [esp+Ch] [ebp-Ch]
    int *weaponState; // [esp+10h] [ebp-8h]
    playerState_s *ps; // [esp+14h] [ebp-4h]

    ps = pm->ps;
    weaponState = BG_GetWeaponState(ps, ps->bRunLeftGun);
    weaponTime = BG_GetWeaponTime(ps, ps->bRunLeftGun);
    weaponShotCount = BG_GetWeaponShotCount(ps, ps->bRunLeftGun);
    weaponIndex = PM_GetWeaponIndexForHand(ps);
    weapDef = BG_GetWeaponDef(weaponIndex);
    if ( (!*weaponState
         || *weaponState == 6
         || *weaponState == 31
         || *weaponState == 7
         || *weaponState >= 27 && *weaponState <= 29
         || *weaponState >= 47 && *weaponState <= 49)
        && weaponIndex
        && weaponIndex < BG_GetNumWeapons()
        && (ps->eFlags2 & 0x10000000) == 0
        && !Mantle_IsWeaponEquipped(ps) )
    {
        *weaponShotCount = 0;
        PM_AddEvent(ps, 0x11u);
        PM_AddEvent(ps, 0x16u);
        if ( weapDef->bSegmentedReload && weapDef->iReloadStartTime )
        {
            PM_StartWeaponAnim(ps, 17, ps->bRunLeftGun);
            *weaponTime = weapDef->iReloadStartTime;
            *weaponState = 12;
            PM_AddEvent(ps, 0x14u);
            PM_SetWeaponReloadAddAmmoDelay(ps);
        }
        else
        {
            PM_SetReloadingState(ps);
        }
        if ( weapDef->bDualWield && BG_PlayDWThirdPersonReload(ps) && !BG_WeaponIsClipOnly(weaponIndex) )
            BG_AnimScriptEvent(pm, ANIM_ET_RELOAD, 0, 1);
        if ( !weapDef->bDualWield && !BG_WeaponIsClipOnly(weaponIndex) )
            BG_AnimScriptEvent(pm, ANIM_ET_RELOAD, 0, 1);
    }
}

bool __cdecl BG_PlayDWThirdPersonReload(playerState_s *ps)
{
    const WeaponVariantDef *weapVariantDefDW; // [esp+0h] [ebp-Ch]
    const WeaponVariantDef *weapVariantDef; // [esp+4h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+8h] [ebp-4h]

    weapVariantDef = BG_GetWeaponVariantDef(ps->weapon);
    weapDef = BG_GetWeaponDef(ps->weapon);
    if ( !weapDef->bDualWield )
        return 0;
    weapVariantDefDW = BG_GetWeaponVariantDef(weapDef->dualWieldWeaponIndex);
    return ps->weaponstate == 10
            && ps->weaponstateLeft == 10
            && ps->weaponTime > weapVariantDef->iReloadTime >> 1
            && ps->weaponTimeLeft > weapVariantDefDW->iReloadTime >> 1;
}

bool __cdecl PM_CheckForReloadInterferingWeaponStates(pmove_t *pm, playerState_s *ps, int weaponState)
{
    bool result; // al
    unsigned int weaponIndex; // [esp+4h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+8h] [ebp-4h]

    weaponIndex = PM_GetWeaponIndexForHand(ps);
    weapDef = BG_GetWeaponDef(weaponIndex);
    switch ( weaponState )
    {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 10:
        case 11:
        case 14:
        case 38:
            result = 0;
            break;
        case 12:
        case 13:
            if ( pm->proneChange )
            {
                if ( weapDef->bDualWield && BG_PlayDWThirdPersonReload(ps) && !BG_WeaponIsClipOnly(weaponIndex) )
                    BG_AnimScriptEvent(pm, ANIM_ET_RELOAD, 0, 1);
                if ( !weapDef->bDualWield && !BG_WeaponIsClipOnly(weaponIndex) )
                    BG_AnimScriptEvent(pm, ANIM_ET_RELOAD, 0, 1);
            }
            result = 0;
            break;
        default:
            if ( weaponState == 6 || weaponState == 31 )
            {
                result = 0;
            }
            else if ( weaponState < 27 || weaponState > 29 )
            {
                if ( weaponState == 35 || weaponState == 36 || weaponState == 37 )
                {
                    result = 0;
                }
                else if ( (ps->eFlags & 0x300) != 0 )
                {
                    result = 0;
                }
                else
                {
                    result = !weapDef->unlimitedAmmo && (!weapDef->bNoADSAutoReload || ps->fWeaponPosFrac != 1.0);
                }
            }
            else
            {
                result = 0;
            }
            break;
    }
    return result;
}

bool __cdecl BurstFirePending(playerState_s *ps)
{
    const WeaponVariantDef *weapVarDef; // [esp+Ch] [ebp-10h]
    unsigned int *weaponShotCount; // [esp+10h] [ebp-Ch]
    unsigned int weaponIndex; // [esp+14h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+18h] [ebp-4h]

    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 2586, 0, "%s", "ps") )
        __debugbreak();
    weaponIndex = PM_GetWeaponIndexForHand(ps);
    weapDef = BG_GetWeaponDef(weaponIndex);
    weapVarDef = BG_GetWeaponVariantDef(weaponIndex);
    weaponShotCount = BG_GetWeaponShotCount(ps, ps->bRunLeftGun);
    if ( !weaponIndex )
        return 0;
    if ( weapDef->fireType == WEAPON_FIRETYPE_FULLAUTO )
        return 0;
    if ( weapDef->fireType == WEAPON_FIRETYPE_MINIGUN )
        return 0;
    if ( !*weaponShotCount )
        return 0;
    if ( BG_GetAmmoNotInClip(ps, weaponIndex) || BG_GetAmmoInClipForWeaponDef(ps, weapVarDef) )
        return !ShotLimitReached(ps, weapDef);
    return 0;
}

void __cdecl UpdatePendingTriggerPull(pmove_t *pm)
{
    playerState_s *ps; // [esp+4h] [ebp-4h]

    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 2643, 0, "%s", "ps") )
        __debugbreak();
    if ( BG_GetWeaponDef(ps->weapon)->fireType >= (unsigned int)WEAPON_FIRETYPE_BURSTFIRE2
        && pm->cmd.button_bits.testBit(0)
        && !pm->oldcmd.button_bits.testBit(0) )
    {
        ps->weapFlags |= 0x400u;
    }
}

void __cdecl CheckStackedFire(pmove_t *pm)
{
    const WeaponVariantDef *weapVarDef; // [esp+8h] [ebp-10h]
    unsigned int ammoInClip; // [esp+Ch] [ebp-Ch]
    const WeaponDef *weapDef; // [esp+10h] [ebp-8h]
    playerState_s *ps; // [esp+14h] [ebp-4h]

    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 2663, 0, "%s", "ps") )
        __debugbreak();
    weapDef = BG_GetWeaponDef(ps->weapon);
    weapVarDef = BG_GetWeaponVariantDef(ps->weapon);
    if ( weapDef->fireType == WEAPON_FIRETYPE_STACKED && weapDef->stackFire > 1 )
    {
        if ( !ps->stackFireCount )
            ps->stackFireCount = 1;
        ammoInClip = BG_GetAmmoInClipForWeaponDef(ps, weapVarDef);
        if ( ps->fWeaponPosFrac >= 1.0 )
        {
            if ( pm->cmd.button_bits.testBit(0xDu) )
            {
                if ( !pm->oldcmd.button_bits.testBit(0xDu) )
                {
                    PM_AddEvent(ps, 0xC0u);
                    if ( ++ps->stackFireCount > weapDef->stackFire || ps->stackFireCount > ammoInClip )
                        ps->stackFireCount = 1;
                }
            }
        }
        if ( !BurstFirePending(ps) && ps->stackFireCount > ammoInClip )
        {
            if ( ammoInClip )
                ps->stackFireCount = ammoInClip;
        }
    }
}

int __cdecl PM_Weapon_WeaponTimeAdjust(pmove_t *pm, pml_t *pml)
{
    bool v3; // [esp+8h] [ebp-ECh]
    bool v4; // [esp+Ch] [ebp-E8h]
    int v5; // [esp+28h] [ebp-CCh]
    int v6; // [esp+2Ch] [ebp-C8h]
    int weaponRestrictKickTime; // [esp+3Ch] [ebp-B8h]
    bool holdingFireBtn; // [esp+C7h] [ebp-2Dh]
    int *weaponDelay; // [esp+CCh] [ebp-28h]
    int msec; // [esp+D4h] [ebp-20h]
    int *weaponTime; // [esp+D8h] [ebp-1Ch]
    const WeaponVariantDef *weapVariantDef; // [esp+DCh] [ebp-18h]
    unsigned int *weaponShotCount; // [esp+E0h] [ebp-14h]
    unsigned int weaponIndex; // [esp+E4h] [ebp-10h]
    const WeaponDef *weapDef; // [esp+E8h] [ebp-Ch]
    playerState_s *ps; // [esp+ECh] [ebp-8h]
    int *weaponState; // [esp+F0h] [ebp-4h]

    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 2710, 0, "%s", "ps") )
        __debugbreak();
    weaponState = BG_GetWeaponState(ps, ps->bRunLeftGun);
    weaponTime = BG_GetWeaponTime(ps, ps->bRunLeftGun);
    weaponDelay = BG_GetWeaponDelay(ps, ps->bRunLeftGun);
    weaponShotCount = BG_GetWeaponShotCount(ps, ps->bRunLeftGun);
    weaponIndex = PM_GetWeaponIndexForHand(ps);
    weapVariantDef = BG_GetWeaponVariantDef(weaponIndex);
    weapDef = BG_GetWeaponDef(weaponIndex);
    if ( ps->weaponRestrictKickTime > 0 )
    {
        ps->weaponRestrictKickTime -= pml->msec;
        if ( ps->weaponRestrictKickTime < 0 )
            weaponRestrictKickTime = 0;
        else
            weaponRestrictKickTime = ps->weaponRestrictKickTime;
        ps->weaponRestrictKickTime = weaponRestrictKickTime;
    }
    if ( (*weaponState == 10
         || *weaponState == 12
         || *weaponState == 14
         || *weaponState == 13
         || *weaponState == 11
         || *weaponState == 15
         || *weaponState == 16)
        && (ps->perks[0] & 0x40000) != 0 )
    {
        if ( perk_weapReloadMultiplier->current.value == 0.0 )
        {
            if ( *weaponTime < *weaponDelay )
                v6 = *weaponDelay;
            else
                v6 = *weaponTime;
            msec = v6;
        }
        else
        {
            msec = SnapFloatToInt((float)((float)((float)pml->msec * (float)(1.0 - 1.0))
                                                 + (float)((float)((float)pml->msec / perk_weapReloadMultiplier->current.value) * 1.0)));
        }
    }
    else if ( (*weaponState == 6
                    || *weaponState == 7
                    || *weaponState == 31
                    || *weaponState == 17
                    || *weaponState == 18
                    || *weaponState == 19)
                 && *weaponState != 17
                 && *weaponState != 18
                 && *weaponState != 19
                 && weapVariantDef->bRapidFire )
    {
        if ( perk_weapRateMultiplier->current.value == 0.0 )
        {
            if ( *weaponTime < *weaponDelay )
                v5 = *weaponDelay;
            else
                v5 = *weaponTime;
            msec = v5;
        }
        else
        {
            msec = SnapFloatToInt((float)((float)pml->msec / perk_weapRateMultiplier->current.value));
        }
    }
    else if ( (ps->perks[0] & 0x80000) != 0 && BG_CanFastSwitch(weapDef, *weaponState) )
    {
        if ( perk_weapSwitchMultiplier->current.value <= 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp",
                        2758,
                        0,
                        "%s\n\t(perk_weapSwitchMultiplier->current.value) = %g",
                        "(perk_weapSwitchMultiplier->current.value > 0.f)",
                        perk_weapSwitchMultiplier->current.value) )
        {
            __debugbreak();
        }
        msec = SnapFloatToInt((float)((float)pml->msec / perk_weapSwitchMultiplier->current.value));
    }
    else if ( *weaponState == 19 && (ps->perks[0] & 0x20000) != 0 )
    {
        if ( perk_weapMeleeMultiplier->current.value <= 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp",
                        2763,
                        0,
                        "%s\n\t(perk_weapMeleeMultiplier->current.value) = %g",
                        "(perk_weapMeleeMultiplier->current.value > 0.f)",
                        perk_weapMeleeMultiplier->current.value) )
        {
            __debugbreak();
        }
        msec = SnapFloatToInt((float)((float)pml->msec / perk_weapMeleeMultiplier->current.value));
    }
    else if ( (*weaponState == 10
                    || *weaponState == 12
                    || *weaponState == 14
                    || *weaponState == 13
                    || *weaponState == 11
                    || *weaponState == 15
                    || *weaponState == 16)
                 && ui_gv_reloadSpeedModifier->current.value != 0.0 )
    {
        msec = SnapFloatToInt((float)((float)pml->msec * ui_gv_reloadSpeedModifier->current.value));
    }
    else
    {
        msec = pml->msec;
    }
    if ( *weaponTime )
    {
        *weaponTime -= msec;
        if ( weapDef->fireType == WEAPON_FIRETYPE_STACKED && BurstFirePending(ps) )
            *weaponTime = 0;
        if ( *weaponTime <= 0 )
        {
            if ( (*weaponState == 6 || *weaponState == 31) && WeaponUsesBurstCooldown(weaponIndex) && !BurstFirePending(ps) )
            {
                if ( player_burstFireCooldown->current.value == 0.0 )
                    *weaponTime = 1;
                else
                    *weaponTime = SnapFloatToInt((float)(player_burstFireCooldown->current.value * 1000.0));
                PM_ContinueWeaponAnim(ps, 0, 0);
                *weaponState = 0;
                return 0;
            }
            v4 = (ps->weapFlags & 0x400) == 0 && ShotLimitReached(ps, weapDef);
            v3 = (weapDef->weapType == WEAPTYPE_GRENADE || weapDef->weapType == WEAPTYPE_MINE) && weapDef->holdButtonToThrow;
            if ( ps->bRunLeftGun )
                holdingFireBtn = pm->cmd.button_bits.testBit(0x18u);
            else
                holdingFireBtn = pm->cmd.button_bits.testBit(0);
            if ( (*weaponState < 20 || *weaponState > 25)
                && (v4 || v3)
                && holdingFireBtn
                && ps->weapon == pm->cmd.weapon
                && (PM_WeaponAmmoAvailable(ps) || weapDef->unlimitedAmmo) )
            {
                *weaponTime = 1;
                if ( *weaponState == 10
                    || *weaponState == 12
                    || *weaponState == 14
                    || *weaponState == 13
                    || *weaponState == 11
                    || *weaponState == 15
                    || *weaponState == 16 )
                {
                    if ( G_IsServerGameSystem(ps->clientNum) )
                        Com_Printf(19, "end weapon (timeout)\n");
                    *weaponTime = 0;
                    *weaponShotCount = 0;
                }
                else if ( *weaponState == 7 )
                {
                    PM_Weapon_FinishRechamber(ps);
                }
                else if ( *weaponState == 6
                             || *weaponState == 7
                             || *weaponState == 31
                             || *weaponState == 17
                             || *weaponState == 18
                             || *weaponState == 19 )
                {
                    PM_ContinueWeaponAnim(ps, 0, 0);
                    *weaponState = 0;
                }
            }
            else
            {
                if ( G_IsServerGameSystem(ps->clientNum) )
                    Com_Printf(19, "end weapon (timeout)\n");
                if ( (!holdingFireBtn || (ps->weapFlags & 0x400) != 0) && !BurstFirePending(ps) )
                    *weaponShotCount = 0;
                *weaponTime = 0;
            }
        }
    }
    if ( !*weaponDelay )
        return 0;
    *weaponDelay -= msec;
    if ( *weaponDelay > 0 )
        return 0;
    *weaponDelay = 0;
    return 1;
}

bool __cdecl WeaponUsesBurstCooldown(unsigned int weaponIdx)
{
    weapFireType_t fireType; // [esp+0h] [ebp-8h]

    if ( !weaponIdx )
        return 0;
    fireType = BG_GetWeaponDef(weaponIdx)->fireType;
    return fireType >= WEAPON_FIRETYPE_BURSTFIRE2 && fireType <= WEAPON_FIRETYPE_BURSTFIRE4;
}

void __cdecl PM_Weapon_CheckForChangeWeapon(pmove_t *pm)
{
    char quick; // [esp+Fh] [ebp-5h]
    playerState_s *ps; // [esp+10h] [ebp-4h]

    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 2879, 0, "%s", "ps") )
        __debugbreak();
    if ( (ps->waterlevel < 3 || !player_disableWeaponsInWater->current.enabled)
        && ps->weaponstate != 17
        && ps->weaponstate != 18
        && ps->weaponstate != 19
        && (ps->weaponstate < 40 || ps->weaponstate > 42)
        && ps->weaponstate != 33
        && ps->weaponstate != 34
        && (ps->eFlags2 & 0x10000000) == 0
        && (ps->weapFlags & 0x100000) == 0
        && !Mantle_IsWeaponEquipped(ps)
        && (!ps->weaponTime
         || ps->weaponstate == 10
         || ps->weaponstate == 12
         || ps->weaponstate == 14
         || ps->weaponstate == 13
         || ps->weaponstate == 11
         || ps->weaponstate == 15
         || ps->weaponstate == 16
         || ps->weaponstate == 7
         || ps->weaponstate != 6
         && ps->weaponstate != 7
         && ps->weaponstate != 31
         && ps->weaponstate != 17
         && ps->weaponstate != 18
         && ps->weaponstate != 19
         && !ps->weaponDelay) )
    {
        if (Mantle_IsWeaponInactive(ps) || door_breach_weapondrop->current.enabled && (ps->pm_flags & 0x1000000) != 0)
        {
            if ( ps->weapon )
                PM_BeginWeaponChange(pm, 0, 1);
        }
        else if ( (ps->pm_flags & 8) != 0 )
        {
            if ( ps->weapon )
                PM_BeginWeaponChange(pm, 0, 1);
        }
        else if ( (ps->weapFlags & 0x20000) != 0 )
        {
            if ( ps->weapon )
                PM_BeginWeaponChange(pm, 0, 1);
        }
        else if ( (ps->weapFlags & 0x80) != 0 )
        {
            if ( ps->weapon )
                PM_BeginWeaponChange(pm, 0, 0);
        }
        else if ( BG_UsingVehicleWeapon(ps) )
        {
            if ( ps->weapon || ps->weapon != pm->cmd.weapon )
            {
                if ( BG_GetWeaponDef(pm->cmd.weapon)->bCanUseInVehicle )
                {
                    if ( ps->weapon != pm->cmd.weapon
                        && ps->weaponstate != 35
                        && ((ps->pm_flags & 0xC00) == 0 || !ps->weapon)
                        && (!pm->cmd.weapon || BG_IsWeaponValid(ps, pm->cmd.weapon)) )
                    {
                        PM_BeginWeaponChange(pm, pm->cmd.weapon, (ps->mantleState.flags & 0x100) != 0);
                    }
                }
                else if ( ps->weapon )
                {
                    PM_BeginWeaponChange(pm, 0, 0);
                }
            }
        }
        else if ( ps->weapon == pm->cmd.weapon
                     || ps->weaponstate == 35
                     || (ps->pm_flags & 0xC00) != 0 && ps->weapon
                     || pm->cmd.weapon && !BG_IsWeaponValid(ps, pm->cmd.weapon)
                     || ps->weaponstate >= 20 && ps->weaponstate <= 25 )
        {
            if ( ps->weapon == pm->cmd.weapon && (ps->weaponstate == 3 || ps->weaponstate == 4 || ps->weaponstate == 5) )
            {
                PM_Weapon_Idle(ps);
                PM_StartWeaponAnim(ps, 1, 0);
            }
            else if ( ps->weapon && !BG_PlayerHasWeapon(ps, ps->weapon) )
            {
                PM_BeginWeaponChange(pm, 0, 0);
            }
        }
        else if ( ps->weaponstate == 35 || ps->weaponstate == 36 || ps->weaponstate == 37 )
        {
            if ( ps->weaponstate == 36 && (ps->weapFlags & 0x1000) == 0 )
                ps->eFlags &= 0xFFFFFCFF;
        }
        else if ( (ps->eFlags & 0x300) != 0 )
        {
            ps->weapFlags |= 0x1000u;
            PM_AddEvent(pm->ps, 0x27u);
        }
        else if ( (ps->mantleState.flags & 0x100) != 0 || (ps->weapFlags & 0x80000) != 0 )
        {
            PM_BeginWeaponChange(pm, pm->cmd.weapon, 1);
        }
        else
        {
            quick = BG_IsQuickSwitch(ps->weapon, pm->cmd.weapon, ps->weaponstate);
            PM_BeginWeaponChange(pm, pm->cmd.weapon, quick);
        }
    }
}

void __cdecl PM_BeginWeaponChange(pmove_t *pm, unsigned int newweapon, bool quick)
{
    unsigned int NumWeapons; // eax
    int iEmptyDropTime; // ecx
    const char *v5; // eax
    bool altswitch; // [esp+18h] [ebp-18h]
    bool oldWeaponNoAmmo; // [esp+1Ch] [ebp-14h]
    const WeaponDef *weapDefNew; // [esp+20h] [ebp-10h]
    unsigned int oldweapon; // [esp+24h] [ebp-Ch]
    const WeaponDef *weapDefOld; // [esp+28h] [ebp-8h]
    playerState_s *ps; // [esp+2Ch] [ebp-4h]

    ps = pm->ps;
    if ( newweapon >= BG_GetNumWeapons() )
    {
        NumWeapons = BG_GetNumWeapons();
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp",
                        1571,
                        0,
                        "newweapon doesn't index BG_GetNumWeapons()\n\t%i not in [0, %i)",
                        newweapon,
                        NumWeapons) )
            __debugbreak();
    }
    if ( (!newweapon || BG_PlayerHasWeapon(ps, newweapon))
        && ps->weaponstate != 3
        && ps->weaponstate != 4
        && ps->weaponstate != 5 )
    {
        if ( ps->weaponstate == 10
            || ps->weaponstate == 12
            || ps->weaponstate == 14
            || ps->weaponstate == 13
            || ps->weaponstate == 11
            || ps->weaponstate == 15
            || ps->weaponstate == 16 )
        {
            BG_AddPredictableEventToPlayerstate(2u, ps->weaponstate, ps);
        }
        ps->weapFlags &= ~0x80000u;
        ps->weaponDelay = 0;
        oldweapon = ps->weapon;
        weapDefOld = BG_GetWeaponDef(oldweapon);
        weapDefNew = BG_GetWeaponDef(newweapon);
        if ( !weapDefNew->bCanUseInVehicle || !weapDefOld->bCanUseInVehicle )
        {
            if ( oldweapon && BG_PlayerHasWeapon(ps, oldweapon) && ps->grenadeTimeLeft <= 0 )
            {
                ps->adsZoomSelect = 0;
                ps->adsZoomLatchTime = 0;
                oldWeaponNoAmmo = PM_WeaponClipEmpty(ps);
                altswitch = BG_IsAltSwitch(ps, oldweapon, newweapon);
                ps->grenadeTimeLeft = 0;
                if ( (ps->weapFlags & 0x20000) != 0 && BG_GetWeaponDef(ps->offHandIndex)->rotateType == WEAPROTATE_BLADE_ROTATE )
                {
                    ps->weaponstate = WEAPON_DROPPING_QUICK;
                    ps->weaponTime = 0;
                }
                else
                {
                    if ( altswitch )
                    {
                        PM_AddEvent(ps, 0x1Bu);
                        PM_StartWeaponAnim(ps, 21, 0);
                    }
                    else
                    {
                        PM_AddEvent(ps, 0x1Au);
                        if ( (weapDefOld->inventoryType != WEAPINVENTORY_ITEM
                             || weapDefOld->offhandSlot != OFFHAND_SLOT_EQUIPMENT
                             && weapDefOld->offhandSlot != OFFHAND_SLOT_SPECIFIC_USE
                             || ps->weaponstate != 6)
                            && !weapDefOld->bNoDropsOrRaises )
                        {
                            if ( oldWeaponNoAmmo && (!quick || weapDefOld->bNoQuickDropWhenEmpty) )
                            {
                                PM_StartWeaponAnim(ps, 25, 0);
                            }
                            else if ( quick )
                            {
                                PM_StartWeaponAnim(ps, 23, 0);
                            }
                            else
                            {
                                PM_StartWeaponAnim(ps, 12, 0);
                            }
                            if ( weapDefOld->bDualWield )
                            {
                                if ( oldWeaponNoAmmo && (!quick || weapDefOld->bNoQuickDropWhenEmpty) )
                                {
                                    PM_StartWeaponAnim(ps, 25, 1);
                                }
                                else if ( quick )
                                {
                                    PM_StartWeaponAnim(ps, 23, 1);
                                }
                                else
                                {
                                    PM_StartWeaponAnim(ps, 12, 1);
                                }
                            }
                        }
                    }
                    if ( altswitch )
                    {
                        ps->weaponstate = WEAPON_DROPPING_ALTSWITCH;
                    }
                    else if ( quick )
                    {
                        ps->weaponstate = WEAPON_DROPPING_QUICK;
                    }
                    else
                    {
                        ps->weaponstate = WEAPON_DROPPING;
                    }
                    ps->weaponstateLeft = ps->weaponstate;
                    PM_SetProneMovementOverride(ps);
                    if ( weapDefOld->bNoDropsOrRaises )
                    {
                        ps->weaponTime = 0;
                    }
                    else if ( altswitch )
                    {
                        ps->weaponTime = weapDefOld->iAltDropTime;
                    }
                    else if ( quick )
                    {
                        ps->weaponTime = weapDefOld->quickDropTime;
                    }
                    else
                    {
                        if ( oldWeaponNoAmmo )
                            iEmptyDropTime = weapDefOld->iEmptyDropTime;
                        else
                            iEmptyDropTime = weapDefOld->iDropTime;
                        ps->weaponTime = iEmptyDropTime;
                    }
                    if ( pm->ps->pm_type != 6
                        && pm->ps->pm_type != 7
                        && pm->ps->pm_type != 8
                        && pm->ps->viewHeightCurrent == (float)pm->ps->viewHeightTarget
                        && !altswitch
                        && (ps->pm_flags & 0x8004) == 0
                        && weapDefNew->inventoryType != WEAPINVENTORY_OFFHAND )
                    {
                        v5 = BG_WeaponName(newweapon);
                        if ( strncmp(v5, "syrette_mp", 0xBu) )
                            BG_AnimScriptEvent(pm, ANIM_ET_DROPWEAPON, 0, 1);
                    }
                }
            }
            else
            {
                if ( G_IsServerGameSystem(ps->clientNum) )
                    Com_Printf(19, "end weapon (begin weapon change)\n");
                ps->weaponTime = 0;
                ps->weaponstate = (weaponstate_t)(quick + 3);
                ps->weapFlags &= ~2u;
                ps->grenadeTimeLeft = 0;
                PM_SetProneMovementOverride(ps);
            }
        }
    }
}

int __cdecl PM_Weapon_ShouldBeFiring(pmove_t *pm, int delayedAction, bool testOnly)
{
    unsigned int WeaponFireButton; // eax
    bool v5; // [esp+0h] [ebp-10h]
    bool shouldStartFiring; // [esp+7h] [ebp-9h]
    playerState_s *ps; // [esp+8h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+Ch] [ebp-4h]

    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 3006, 0, "%s", "ps") )
        __debugbreak();
    weapDef = BG_GetWeaponDef(ps->weapon);
    WeaponFireButton = PM_GetWeaponFireButton(ps->weapon);
    shouldStartFiring = pm->cmd.button_bits.testBit(WeaponFireButton);
    if ( weapDef->bDualWield && pm->cmd.button_bits.testBit(0x18u) && ps->bRunLeftGun )
    {
        shouldStartFiring = 1;
    }
    else if ( ps->bRunLeftGun && weapDef->bDualWield )
    {
        shouldStartFiring = 0;
    }
    if ( weapDef->freezeMovementWhenFiring && ps->groundEntityNum == 1023 )
        shouldStartFiring = 0;
    if ( weapDef->bCanUseInVehicle )
        shouldStartFiring = 0;
    v5 = delayedAction || BurstFirePending(ps);
    if ( weapDef->fireType == WEAPON_FIRETYPE_MINIGUN )
        shouldStartFiring = shouldStartFiring && ps->weaponSpinLerp >= 1.0;
    if ( shouldStartFiring || v5 )
        return 1;
    if ( !testOnly )
    {
        if ( ps->bRunLeftGun && weapDef->bDualWield )
        {
            if ( ps->weaponstateLeft == 6 )
                PM_ContinueWeaponAnim(ps, 0, 1);
            ps->weaponstateLeft = 0;
        }
        else
        {
            if ( ps->weaponstate == 6 && !Mantle_IsWeaponEquipped(ps) )
                PM_ContinueWeaponAnim(ps, 0, 0);
            ps->weaponstate = WEAPON_READY;
        }
    }
    return 0;
}

void __cdecl PM_Weapon_FireWeapon(pmove_t *pm, int delayedAction)
{
    int ActiveCount; // eax
    int v3; // eax
    float start[3]; // [esp+8h] [ebp-34h] BYREF
    float end[3]; // [esp+14h] [ebp-28h] BYREF
    float hit[3]; // [esp+20h] [ebp-1Ch] BYREF
    int surface; // [esp+2Ch] [ebp-10h]
    unsigned int *weaponShotCount; // [esp+30h] [ebp-Ch]
    playerState_s *ps; // [esp+34h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+38h] [ebp-4h]

    ps = pm->ps;
    weapDef = BG_GetWeaponDef(ps->weapon);
    weaponShotCount = BG_GetWeaponShotCount(ps, ps->bRunLeftGun);
    if (ps->waterlevel < 3
        || !player_disableWeaponsInWater->current.enabled
        || ps->weaponstate == 17 && ps->weaponstate == 18 && ps->weaponstate == 19)
    {
        if (weapDef->weapType != WEAPTYPE_MINE)
            goto LABEL_62;
        start[0] = ps->origin[0];
        start[1] = ps->origin[1];
        start[2] = ps->origin[2];
        end[0] = ps->origin[0];
        end[1] = ps->origin[1];
        end[2] = ps->origin[2];
        start[2] = start[2] + 10.0;
        end[2] = end[2] - 50.0;
        surface = CM_TracePointDown(start, end, 2097, 0x3F00000, hit, 0, 0);
        if ((surface & 0x3F00000) == 0x600000
            || (surface & 0x3F00000) == 0x800000
            || (surface & 0x3F00000) == 0xA00000
            || (surface & 0x3F00000) == 0xE00000
            || (surface & 0x3F00000) == 0x1200000
            || (surface & 0x3F00000) == 0x1300000)
        {
        LABEL_62:
            if (weapDef->weapType == WEAPTYPE_GRENADE && !PM_WeaponAllowPlant(pm))
            {
                ps->weaponTime = 500;
                PM_AddEvent(ps, 0xB9u);
                return;
            }
            if ((!weapDef->overheatWeapon || !BG_PlayerWeaponOverheating(ps, ps->weapon))
                && (weapDef->unlimitedAmmo || PM_Weapon_CheckFiringAmmo(pm)))
            {
                PM_Weapon_StartFiring(pm, delayedAction);
                if ((!ps->weaponDelay || ps->bRunLeftGun) && (!ps->weaponDelayLeft || !ps->bRunLeftGun))
                {
                    ps->weapFlags &= ~0x400u;
                    if (!weapDef->requireLockonToFire)
                        goto LABEL_61;
                    if ((ps->weapLockFlags & 2) == 0)
                    {
                        PM_AddEvent(ps, 0x66u);
                        return;
                    }
                    if ((ps->weapLockFlags & 0x10) != 0)
                    {
                        PM_AddEvent(ps, 0x64u);
                        return;
                    }
                    if ((ps->weapLockFlags & 0x20) != 0)
                    {
                        PM_AddEvent(ps, 0x65u);
                    }
                    else
                    {
                    LABEL_61:
                        if (PM_WeaponAmmoAvailable(ps) && !weapDef->fuelTankWeapon)
                        {
                            if ((ps->eFlags & 0x300) == 0 && ps->bRunLeftGun && weapDef->bDualWield)
                            {
                                ActiveCount = CL_LocalClient_GetActiveCount();
                                PM_WeaponUseAmmo(ps, weapDef->dualWieldWeaponIndex, ActiveCount);
                            }
                            else if ((ps->eFlags & 0x300) == 0)
                            {
                                v3 = CL_LocalClient_GetActiveCount();
                                PM_WeaponUseAmmo(ps, ps->weapon, v3);
                            }
                        }
                        if (weapDef->weapType == WEAPTYPE_GRENADE || weapDef->weapType == WEAPTYPE_MINE)
                            ps->weaponTime = weapDef->iFireTime;
                        if (pm->cmd.button_bits.testBit(0x26u))
                        {
                            PM_Weapon_Jam(ps);
                        }
                        else
                        {
                            if (ps->weaponstate < 30 || ps->weaponstate > 32)
                                PM_Weapon_SetFPSFireAnim(ps);
                            if (weapDef->guidedMissileType == MISSILE_GUIDANCE_TVGUIDED)
                                ps->weapFlags |= 0x8000u;
                            if (PM_WeaponClipEmpty(ps))
                            {
                                if (ps->bRunLeftGun)
                                    BG_AddPredictableEventToPlayerstate(0x20u, *weaponShotCount, ps);
                                else
                                    BG_AddPredictableEventToPlayerstate(0x1Eu, *weaponShotCount, ps);
                            }
                            else if (ps->bRunLeftGun)
                            {
                                BG_AddPredictableEventToPlayerstate(0x1Fu, *weaponShotCount, ps);
                            }
                            else
                            {
                                BG_AddPredictableEventToPlayerstate(0x1Du, *weaponShotCount, ps);
                            }
                        }
                        PM_HoldBreathFire(ps);
                        PM_Weapon_AddFiringAimSpreadScale(ps);
                        BG_SwitchWeaponsIfEmpty(ps);
                    }
                }
            }
        }
    }
}

void __cdecl PM_HoldBreathFire(playerState_s *ps)
{
    int weapIndex; // [esp+4h] [ebp-Ch]
    int breathHoldTime; // [esp+8h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+Ch] [ebp-4h]

    weapIndex = BG_GetViewmodelWeaponIndex(ps);
    weapDef = BG_GetWeaponDef(weapIndex);
    if ( ps->fWeaponPosFrac == 1.0 && weapDef->overlayReticle && weapDef->weapClass != WEAPCLASS_ITEM )
    {
        breathHoldTime = (int)(float)(player_breath_hold_time->current.value * 1000.0);
        if ( ps->holdBreathTimer < breathHoldTime )
        {
            ps->holdBreathTimer += (int)(float)(player_breath_fire_delay->current.value * 1000.0);
            if ( ps->holdBreathTimer > breathHoldTime )
                ps->holdBreathTimer = breathHoldTime;
        }
        PM_EndHoldBreath(ps);
    }
}

void __cdecl BG_SwitchWeaponsIfEmpty(playerState_s *ps)
{
    const WeaponDef *weapDef; // [esp+Ch] [ebp-4h]

    weapDef = BG_GetWeaponDef(ps->weapon);
    if ( !weapDef->unlimitedAmmo
        && (ps->weapFlags & 0x200) == 0
        && !BG_GetAmmoInClip(ps, ps->weapon)
        && !BG_GetAmmoNotInClip(ps, ps->weapon)
        && !BG_CheckIfDualWieldEmpty(ps)
        && !weapDef->hasDetonator
        && weapDef->guidedMissileType != MISSILE_GUIDANCE_TVGUIDED
        && weapDef->guidedMissileType != MISSILE_GUIDANCE_WIREGUIDED )
    {
        PM_AddEvent(ps, 0xDu);
    }
}

bool __cdecl BG_CheckIfDualWieldEmpty(playerState_s *ps)
{
    const WeaponDef *weapDef; // [esp+10h] [ebp-4h]

    weapDef = BG_GetWeaponDef(ps->weapon);
    return BG_GetAmmoInClip(ps, ps->weapon) || BG_GetAmmoInClip(ps, weapDef->dualWieldWeaponIndex);
}

void __cdecl PM_Weapon_StartFiring(pmove_t *pm, int delayedAction)
{
    int v2; // [esp+0h] [ebp-38h]
    int *weaponDelay; // [esp+18h] [ebp-20h]
    int *weaponTime; // [esp+1Ch] [ebp-1Ch]
    const WeaponVariantDef *weapVariantDef; // [esp+20h] [ebp-18h]
    unsigned int *weaponShotCount; // [esp+24h] [ebp-14h]
    unsigned int weaponIndex; // [esp+28h] [ebp-10h]
    const WeaponDef *weapDef; // [esp+2Ch] [ebp-Ch]
    int *weaponState; // [esp+30h] [ebp-8h]
    playerState_s *ps; // [esp+34h] [ebp-4h]

    ps = pm->ps;
    weaponState = BG_GetWeaponState(ps, ps->bRunLeftGun);
    weaponTime = BG_GetWeaponTime(ps, ps->bRunLeftGun);
    weaponDelay = BG_GetWeaponDelay(ps, ps->bRunLeftGun);
    weaponShotCount = BG_GetWeaponShotCount(ps, ps->bRunLeftGun);
    weaponIndex = PM_GetWeaponIndexForHand(ps);
    weapVariantDef = BG_GetWeaponVariantDef(weaponIndex);
    weapDef = BG_GetWeaponDef(weaponIndex);
    if ( !weaponIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp",
                    3135,
                    0,
                    "%s",
                    "weaponIndex != WP_NONE") )
    {
        __debugbreak();
    }
    if ( weapDef->weapType == WEAPTYPE_GRENADE || weapDef->weapType == WEAPTYPE_MINE )
    {
        if ( !delayedAction )
        {
            if ( PM_WeaponAmmoAvailable(ps) )
            {
                ps->grenadeTimeLeft = weapDef->fuseTime;
                PM_StartWeaponAnim(ps, 39, 0);
                PM_StartWeaponAnim(ps, 39, 1);
                BG_AddPredictableEventToPlayerstate(0x1Cu, ps->weapon, ps);
            }
            ps->weaponDelay = weapDef->iHoldFireTime;
            ps->weaponTime = 0;
            if ( G_IsServerGameSystem(ps->clientNum) )
                Com_Printf(19, "end weapon (start fire)\n");
            goto LABEL_35;
        }
LABEL_34:
        BG_AnimScriptEvent(pm, ANIM_ET_FIREWEAPON, 0, 1);
        goto LABEL_35;
    }
    if ( BG_CheckIfLastShot(ps, weaponIndex) && weapDef->iLastFireTime )
        *weaponTime = weapDef->iLastFireTime;
    else
        *weaponTime = weapDef->iFireTime;
    if ( weapDef->bBoltAction )
        BG_PlayerWeaponSetNeedsRechamber(ps, weaponIndex, 1);
    if ( *weaponState != 6 && *weaponState != 31 )
    {
        *weaponDelay = weapDef->iFireDelay;
        if ( ps->fWeaponPosFrac < 1.0 )
            ps->weaponRestrictKickTime = weapDef->iFireDelay + weapDef->hipGunKickReducedKickBullets * weapDef->iFireTime;
        else
            ps->weaponRestrictKickTime = weapDef->iFireDelay + weapDef->adsGunKickReducedKickBullets * weapDef->iFireTime;
    }
    if ( weapDef->adsFireOnly && weapDef->fireType != WEAPON_FIRETYPE_MINIGUN )
    {
        if ( (int)(float)((float)(1.0 - ps->fWeaponPosFrac) * (float)(1.0 / weapVariantDef->fOOPosAnimLength[0])) < *weaponDelay )
            v2 = *weaponDelay;
        else
            v2 = (int)(float)((float)(1.0 - ps->fWeaponPosFrac) * (float)(1.0 / weapVariantDef->fOOPosAnimLength[0]));
        *weaponDelay = v2;
    }
    if ( weapDef->guidedMissileType == MISSILE_GUIDANCE_TVGUIDED )
        ps->weapFlags |= 0x100000u;
    if ( !BG_IsShellshockAnim(ps) && ps->viewHeightCurrent == (float)ps->viewHeightTarget )
        goto LABEL_34;
LABEL_35:
    PM_SetProneMovementOverride(ps);
    if ( *weaponState < 30 || *weaponState > 32 )
        *weaponState = 6;
    if ( !*weaponDelay && weapDef->fireType && ++*weaponShotCount > 7 )
        *weaponShotCount = 7;
}

char __cdecl BG_CheckIfLastShot(playerState_s *ps, unsigned int weaponIndex)
{
    weapFireType_t fireType; // [esp+0h] [ebp-18h]
    const WeaponVariantDef *weapVarDef; // [esp+Ch] [ebp-Ch]
    int ammo; // [esp+10h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+14h] [ebp-4h]

    weapDef = BG_GetWeaponDef(weaponIndex);
    weapVarDef = BG_GetWeaponVariantDef(weaponIndex);
    ammo = BG_GetAmmoInClipForWeaponDef(ps, weapVarDef);
    fireType = weapDef->fireType;
    switch ( fireType )
    {
        case WEAPON_FIRETYPE_BURSTFIRE2:
            if ( ammo <= 2 )
                return 1;
            break;
        case WEAPON_FIRETYPE_BURSTFIRE3:
            if ( ammo <= 3 )
                return 1;
            break;
        case WEAPON_FIRETYPE_BURSTFIRE4:
            if ( ammo <= 4 )
                return 1;
            break;
        default:
            if ( ammo <= 1 )
                return 1;
            break;
    }
    return 0;
}

int __cdecl PM_Weapon_CheckFiringAmmo(pmove_t *pm)
{
    int *weaponTime; // [esp+10h] [ebp-20h]
    bool reloadingW; // [esp+14h] [ebp-1Ch]
    int ammoNeeded; // [esp+18h] [ebp-18h]
    unsigned int weaponIndex; // [esp+1Ch] [ebp-14h]
    const WeaponDef *weapDef; // [esp+20h] [ebp-10h]
    playerState_s *ps; // [esp+28h] [ebp-8h]

    ps = pm->ps;
    weaponIndex = PM_GetWeaponIndexForHand(pm->ps);
    weapDef = BG_GetWeaponDef(weaponIndex);
    if ( !weaponIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp",
                    3232,
                    0,
                    "%s",
                    "weaponIndex != WP_NONE") )
    {
        __debugbreak();
    }
    BG_GetWeaponState(ps, ps->bRunLeftGun);
    weaponTime = BG_GetWeaponTime(ps, ps->bRunLeftGun);
    ammoNeeded = CL_LocalClient_GetActiveCount();
    if ( ammoNeeded <= PM_WeaponAmmoAvailable(ps) )
        return 1;
    reloadingW = ammoNeeded <= BG_GetAmmoNotInClip(ps, weaponIndex);
    if ( weapDef->bNoADSAutoReload && ps->fWeaponPosFrac == 1.0 )
    {
        reloadingW = 0;
    }
    else if ( weapDef->weapType != WEAPTYPE_GRENADE && weapDef->weapType != WEAPTYPE_MINE && !reloadingW )
    {
        if ( ps->bRunLeftGun )
            PM_AddEvent(ps, 0xEu);
        else
            PM_AddEvent(ps, 0xDu);
    }
    if ( reloadingW )
    {
        PM_BeginWeaponReload(pm);
    }
    else if ( !Mantle_IsWeaponEquipped(ps) )
    {
        BG_PlayerWeaponSetNeedsRechamber(ps, weaponIndex, 0);
        PM_ContinueWeaponAnim(ps, 0, ps->bRunLeftGun);
        if ( weapDef->weapType != WEAPTYPE_GRENADE && weapDef->weapType != WEAPTYPE_MINE )
            *weaponTime += 500;
    }
    return 0;
}

void __cdecl PM_Weapon_Jam(playerState_s *ps)
{
    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 3277, 0, "%s", "ps") )
        __debugbreak();
    ps->weaponstate = WEAPON_JAMMED;
    ps->weaponTime = 2000;
    ps->weaponDelay = 2000;
    PM_StartWeaponAnim(ps, 53, 0);
    PM_SetProneMovementOverride(ps);
    PM_AddEvent(ps, 0xBFu);
}

void __cdecl PM_Weapon_SetFPSFireAnim(playerState_s *ps)
{
    int UseLeftAnim; // [esp+0h] [ebp-8h]
    int weapattackanim; // [esp+4h] [ebp-4h]

    if ( (ps->weapAnim & 0xFFFFFBFF) != 0x34 )
    {
        UseLeftAnim = 0;
        if ( ps->fWeaponPosFrac <= 0.75 )
        {
            if ( PM_WeaponClipEmpty(ps) )
                weapattackanim = 4;
            else
                weapattackanim = 2;
        }
        else if ( PM_WeaponClipEmpty(ps) )
        {
            weapattackanim = 8;
        }
        else
        {
            weapattackanim = 7;
        }
        if ( ps->bRunLeftGun )
            UseLeftAnim = 1;
        PM_StartWeaponAnim(ps, weapattackanim, UseLeftAnim);
    }
}

void __cdecl PM_Weapon_AddFiringAimSpreadScale(playerState_s *ps)
{
    const WeaponDef *weapDef; // [esp+0h] [ebp-4h]

    weapDef = BG_GetWeaponDef(ps->weapon);
    if ( ps->fWeaponPosFrac != 1.0 )
    {
        ps->aimSpreadScale = (float)(weapDef->fHipSpreadFireAdd * 255.0) + ps->aimSpreadScale;
        if ( ps->aimSpreadScale > 255.0 )
            ps->aimSpreadScale = 255.0f;
    }
}

char __cdecl PM_WeaponAllowPlant(pmove_t *pm)
{
    float iProjectileSpeedForward; // [esp+24h] [ebp-74h]
    float iProjectileSpeed; // [esp+44h] [ebp-54h]
    float hit_fraction; // [esp+48h] [ebp-50h] BYREF
    float dir[3]; // [esp+4Ch] [ebp-4Ch] BYREF
    float start[3]; // [esp+58h] [ebp-40h] BYREF
    float end[3]; // [esp+64h] [ebp-34h] BYREF
    float hit[3]; // [esp+70h] [ebp-28h] BYREF
    float forward[3]; // [esp+7Ch] [ebp-1Ch] BYREF
    const WeaponVariantDef *weapVariantDef; // [esp+88h] [ebp-10h]
    int surface; // [esp+8Ch] [ebp-Ch]
    playerState_s *ps; // [esp+90h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+94h] [ebp-4h]

    ps = pm->ps;
    weapDef = BG_GetWeaponDef(ps->weapon);
    if ( weapDef->weapType != WEAPTYPE_GRENADE || !weapDef->plantable )
        return 1;
    weapVariantDef = BG_GetWeaponVariantDef(ps->weapon);
    if ( ps->groundEntityNum == 1023
        || ps->groundEntityNum != 1022 && !I_strcmp(weapVariantDef->szInternalName, "tactical_insertion_mp") )
    {
        PM_StartWeaponAnim(ps, 24, 0);
        return 0;
    }
    AngleVectors(ps->viewangles, forward, 0, 0);
    iProjectileSpeed = (float)weapDef->iProjectileSpeed;
    dir[0] = iProjectileSpeed * forward[0];
    dir[1] = iProjectileSpeed * forward[1];
    dir[2] = iProjectileSpeed * forward[2];
    dir[2] = (float)weapDef->iProjectileSpeedUp + (float)(iProjectileSpeed * forward[2]);
    if ( weapDef->iProjectileSpeedForward )
    {
        Vec2NormalizeFast(forward);
        iProjectileSpeedForward = (float)weapDef->iProjectileSpeedForward;
        dir[0] = (float)(iProjectileSpeedForward * forward[0]) + dir[0];
        dir[1] = (float)(iProjectileSpeedForward * forward[1]) + dir[1];
    }
    Vec3Normalize(dir);
    start[0] = ps->origin[0];
    start[1] = ps->origin[1];
    start[2] = ps->origin[2];
    start[2] = start[2] + ps->viewHeightCurrent;
    if ( (ps->eFlags & 0x4000) == 0 )
        AddLeanToPosition(start, ps->viewangles[1], ps->leanf, 16.0, 20.0);
    end[0] = (float)(trace_dist * dir[0]) + start[0];
    end[1] = (float)(trace_dist * dir[1]) + start[1];
    end[2] = (float)(trace_dist * dir[2]) + start[2];
    DrawPlantDebug(start, end);
    surface = CM_TracePointDown(start, end, 2097, 0x3F00000, hit, 0, 0);
    if ((surface & 0x3F00000) != 0x1400000)
        return 1;
    if (!bg_disableWeaponPlantingInWater->current.enabled)
        return 1;
    start[0] = hit[0];
    start[1] = hit[1];
    start[2] = hit[2];
    end[0] = hit[0];
    end[1] = hit[1];
    end[2] = hit[2];
    end[2] = hit[2] - bg_plantInWaterDepth->current.value;
    hit_fraction = 1.0f;
    surface = CM_TracePointDown(start, end, 2065, 0x3F00000, hit, &hit_fraction, 0);
    if (hit_fraction != 1.0 || (surface & 0x3F00000) != 0)
        return 1;
    if (!PM_Weapon_IsHoldingGrenade(pm))
        PM_StartWeaponAnim(ps, 24, 0);
    return 0;
}

void __cdecl DrawPlantDebug(float *start, float *end)
{
    float color[4]; // [esp+0h] [ebp-10h] BYREF

    if (bgs == &level_bgs)
    {
        color[0] = 0.0f;
        color[1] = 0.0f;
        color[2] = 1.0f;
        color[3] = 1.0f;
        if ( g_debugBullets->current.integer >= 5 )
            CG_DebugLine(start, end, color, 1, 500);
    }
}

bool __cdecl PM_Weapon_IsHoldingGrenade(pmove_t *pm)
{
    unsigned int WeaponFireButton; // eax
    playerState_s *ps; // [esp+0h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+4h] [ebp-4h]

    if ( !pm && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 3410, 0, "%s", "pm") )
        __debugbreak();
    ps = pm->ps;
    if ( !pm->ps->weapon )
        return 0;
    weapDef = BG_GetWeaponDef(ps->weapon);
    if ( !weapDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 3418, 0, "%s", "weapDef") )
    {
        __debugbreak();
    }
    if ( weapDef->weapType != WEAPTYPE_GRENADE && weapDef->weapType != WEAPTYPE_MINE )
        return 0;
    if ( weapDef->holdButtonToThrow )
        return 0;
    WeaponFireButton = PM_GetWeaponFireButton(ps->weapon);
    return pm->cmd.button_bits.testBit(WeaponFireButton);
}

void __cdecl PM_Weapon_MeleeEnd(playerState_s *ps)
{
    const WeaponDef *weapDef; // [esp+4h] [ebp-4h]

    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 3712, 0, "%s", "ps") )
        __debugbreak();
    weapDef = BG_GetWeaponDef(ps->weapon);
    if ( weapDef == PM_Weapon_GetMeleeWeapDef(ps) )
    {
        PM_Weapon_Idle(ps);
    }
    else
    {
        ps->weaponstate = WEAPON_MELEE_END;
        ps->weaponTime = weapDef->quickRaiseTime;
        ps->weaponDelay = 0;
        if ( ps->waterlevel < 3 )
            PM_StartWeaponAnim(ps, 24, 0);
        PM_SetProneMovementOverride(ps);
    }
}

const WeaponDef *__cdecl PM_Weapon_GetMeleeWeapDef(playerState_s *ps)
{
    if ( BG_IsUseAsMeleeWeapon(ps->weapon) )
        return BG_GetWeaponDef(ps->weapon);
    else
        return BG_GetWeaponDef(ps->meleeWeapon);
}

void __cdecl PM_Weapon_MeleeFire(playerState_s *ps)
{
    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 3738, 0, "%s", "ps") )
        __debugbreak();
    PM_Weapon_GetMeleeWeapDef(ps);
    ps->weaponstate = WEAPON_MELEE_FIRE;
    PM_AddEvent(ps, 0x24u);
    PM_SetProneMovementOverride(ps);
}

void __cdecl PM_Weapon_CheckForMelee(pmove_t *pm, pml_t *pml, int delayedAction)
{
    int weaponstate; // [esp+0h] [ebp-18h]
    const WeaponDef *primaryWeapDef; // [esp+4h] [ebp-14h]
    const WeaponVariantDef *weapVariantDef; // [esp+Ch] [ebp-Ch]
    const WeaponDef *weapDef; // [esp+10h] [ebp-8h]
    playerState_s *ps; // [esp+14h] [ebp-4h]

    if ( !pm && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 3799, 0, "%s", "pm") )
        __debugbreak();
    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 3802, 0, "%s", "ps") )
        __debugbreak();
    if ( (ps->pm_flags & 8) == 0 && (ps->pm_flags & 0x2000) == 0 && (ps->weapFlags & 0x80) == 0 )
    {
        primaryWeapDef = BG_GetWeaponDef(ps->weapon);
        if ( BG_IsUseAsMeleeWeapon(ps->weapon) )
        {
            weapVariantDef = BG_GetWeaponVariantDef(ps->weapon);
            weapDef = BG_GetWeaponDef(ps->weapon);
        }
        else
        {
            weapVariantDef = BG_GetWeaponVariantDef(ps->meleeWeapon);
            weapDef = BG_GetWeaponDef(ps->meleeWeapon);
        }
        if ( ps->weaponstate != 17
            && ps->weaponstate != 18
            && ps->weaponstate != 19
            && (ps->weaponstate < 20 || ps->weaponstate > 25)
            && ps->weaponstate != 33
            && ps->weaponstate != 34
            && (ps->weaponstate < 40 || ps->weaponstate > 42)
            && (ps->pm_flags & 4) == 0
            && weapDef->iMeleeDamage
            && !delayedAction
            && (!ps->weaponDelay
             || ps->weaponstate == 10
             || ps->weaponstate == 12
             || ps->weaponstate == 14
             || ps->weaponstate == 13
             || ps->weaponstate == 11
             || ps->weaponstate == 15
             || ps->weaponstate == 16)
            && pm->cmd.button_bits.testBit(2u)
            && !pm->oldcmd.button_bits.testBit(2u)
            && (ps->fWeaponPosFrac <= 0.0 || primaryWeapDef->overlayReticle == WEAPOVERLAYRETICLE_NONE) )
        {
            if ( **((_BYTE **)weapVariantDef->szXAnims + 7) )
            {
                if ( (ps->eFlags2 & 0x10000000) == 0 )
                {
                    weaponstate = ps->weaponstate;
                    if ( weaponstate <= 0 || weaponstate > 5 )
                    {
                        PM_MeleeChargeStart(pm, pml);
                        PM_Weapon_MeleeInit(pm);
                    }
                }
            }
        }
    }
}

void __cdecl PM_Weapon_MeleeInit(pmove_t *pm)
{
    bool v1; // [esp+0h] [ebp-18h]
    float value; // [esp+4h] [ebp-14h]
    const WeaponDef *weapDef; // [esp+Ch] [ebp-Ch]
    playerState_s *ps; // [esp+10h] [ebp-8h]

    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 3757, 0, "%s", "ps") )
        __debugbreak();
    weapDef = PM_Weapon_GetMeleeWeapDef(ps);
    if ( weapDef->bHasBayonet )
        value = player_bayonetRange->current.value;
    else
        value = player_meleeRange->current.value;
    v1 = (ps->pm_flags & 0x20000) != 0 && (float)ps->meleeChargeDist > value;
    if ( v1 )
    {
        ps->weaponTime = weapDef->meleeChargeTime;
        ps->weaponDelay = weapDef->meleeChargeDelay;
        PM_StartWeaponAnim(ps, 11, 0);
    }
    else
    {
        ps->weaponTime = weapDef->iMeleeTime;
        ps->weaponDelay = weapDef->iMeleeDelay;
        PM_StartWeaponAnim(ps, 10, 0);
    }
    if ( *weapDef->gunXModel && v1 )
    {
        BG_AnimScriptEvent(pm, ANIM_ET_KNIFE_MELEE_CHARGE, 0, 1);
    }
    else if ( *weapDef->gunXModel )
    {
        BG_AnimScriptEvent(pm, ANIM_ET_KNIFE_MELEE, 0, 1);
    }
    else
    {
        BG_AnimScriptEvent(pm, ANIM_ET_MELEEATTACK, 0, 1);
    }
    ps->weaponstate = WEAPON_MELEE_INIT;
    PM_AddEvent(ps, 0x23u);
    PM_SetProneMovementOverride(ps);
}

void __cdecl PM_Weapon_OffHandPrepare(pmove_t *pm)
{
    const WeaponDef *WeaponDef; // eax
    playerState_s *ps; // [esp+4h] [ebp-4h]

    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 3973, 0, "%s", "ps") )
        __debugbreak();
    if ( !ps->offHandIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp",
                    3974,
                    0,
                    "%s",
                    "ps->offHandIndex != WP_NONE") )
    {
        __debugbreak();
    }
    if ( ps->waterlevel < 3 && !ps->weapon )
    {
        WeaponDef = BG_GetWeaponDef(ps->offHandIndex);
        ps->weaponstate = WEAPON_OFFHAND_PREPARE;
        ps->weaponTime = WeaponDef->iHoldFireTime;
        ps->weaponDelay = 0;
        ps->weapFlags |= 2u;
        ps->weapFlags &= ~0x20000u;
        if ( BG_ThrowingBackGrenade(ps) )
        {
            PM_StartWeaponAnim(ps, 22, 0);
        }
        else
        {
            BG_AddPredictableEventToPlayerstate(0x29u, ps->offHandIndex, ps);
            PM_StartWeaponAnim(ps, 39, 0);
            PM_StartWeaponAnim(ps, 39, 1);
            if ( !ps->weapon )
                BG_AnimScriptEvent(pm, ANIM_ET_PRIME_GRENADE, 1, 1);
        }
        PM_SetProneMovementOverride(ps);
    }
}

void __cdecl PM_Weapon_OffHandHold(playerState_s *ps)
{
    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 4011, 0, "%s", "ps") )
        __debugbreak();
    if ( !ps->offHandIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp",
                    4012,
                    0,
                    "%s",
                    "ps->offHandIndex != WP_NONE") )
    {
        __debugbreak();
    }
    ps->weaponstate = WEAPON_OFFHAND_START;
    ps->weaponTime = 0;
    ps->weaponDelay = 0;
    ps->weapFlags |= 2u;
    if ( !BG_ThrowingBackGrenade(ps) )
        ps->grenadeTimeLeft = BG_GetWeaponDef(ps->offHandIndex)->fuseTime;
    if ( G_IsServerGameSystem(ps->clientNum) )
        Com_Printf(19, "end weapon (offhand hold)\n");
}

void __cdecl PM_Weapon_OffHandStart(pmove_t *pm)
{
    const WeaponDef *weapDef; // [esp+0h] [ebp-8h]
    playerState_s *ps; // [esp+4h] [ebp-4h]

    if ( !pm && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 4038, 0, "%s", "pm") )
        __debugbreak();
    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 4042, 0, "%s", "ps") )
        __debugbreak();
    if ( !ps->offHandIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp",
                    4043,
                    0,
                    "%s",
                    "ps->offHandIndex != WP_NONE") )
    {
        __debugbreak();
    }
    weapDef = BG_GetWeaponDef(ps->offHandIndex);
    if ( weapDef->holdButtonToThrow
        || !pm->oldcmd.button_bits.testBit(0xEu) && !pm->oldcmd.button_bits.testBit(0xFu)
        || !pm->cmd.button_bits.testBit(0xEu) && !pm->cmd.button_bits.testBit(0xFu) )
    {
        goto LABEL_27;
    }
    if ( weapDef->offhandHoldIsCancelable && pm->cmd.button_bits.testBit(0x31u) )
    {
        PM_Weapon_OffHandEnd(ps);
        return;
    }
    if (   weapDef->offhandClass == OFFHAND_CLASS_SMOKE_GRENADE && pm->cmd.button_bits.testBit(0x1Eu)
        || weapDef->offhandClass == OFFHAND_CLASS_SMOKE_GRENADE && pm->cmd.button_bits.testBit(0x1Fu)
        || weapDef->offhandClass == OFFHAND_CLASS_SMOKE_GRENADE && pm->cmd.button_bits.testBit(0x20u)
        || weapDef->offhandClass == OFFHAND_CLASS_SMOKE_GRENADE && pm->cmd.button_bits.testBit(0x21u) )
    {
LABEL_27:
        ps->smokeColorIndex = 55;
        if ( pm->cmd.button_bits.testBit(0x1Eu) )
            ps->smokeColorIndex = 49;
        if ( pm->cmd.button_bits.testBit(0x1Fu) )
            ps->smokeColorIndex = 50;
        if ( pm->cmd.button_bits.testBit(0x20u) )
            ps->smokeColorIndex = 52;
        if ( pm->cmd.button_bits.testBit(0x21u) )
            ps->smokeColorIndex = 51;
        if ( ps->smokeColorIndex != 55 && weapDef->offhandClass == OFFHAND_CLASS_SMOKE_GRENADE )
            Cbuf_AddText(pm->localClientNum, "-smoke\n");
        ps->weaponstate = WEAPON_OFFHAND_HOLD;
        ps->weaponTime = weapDef->iFireTime;
        ps->weaponDelay = weapDef->iFireDelay;
        ps->weapFlags |= 2u;
        PM_StartWeaponAnim(ps, 2, 0);
        BG_AnimScriptEvent(pm, ANIM_ET_FIREWEAPON, 0, 1);
    }
    else
    {
        ps->weaponDelay = 1;
    }
}

void __cdecl PM_Weapon_OffHand(pmove_t *pm)
{
    playerState_s *ps; // [esp+4h] [ebp-4h]

    if ( !pm && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 4103, 0, "%s", "pm") )
        __debugbreak();
    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 4107, 0, "%s", "ps") )
        __debugbreak();
    if ( !ps->offHandIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp",
                    4108,
                    0,
                    "%s",
                    "ps->offHandIndex != WP_NONE") )
    {
        __debugbreak();
    }
    BG_GetWeaponDef(ps->offHandIndex);
    if ( ps->waterlevel < 3 )
    {
        BG_AddPredictableEventToPlayerstate(0x2Au, ps->offHandIndex, ps);
        if ( !BG_ThrowingBackGrenade(ps) )
        {
            if ( BG_WeaponAmmo(ps, ps->offHandIndex) )
                PM_WeaponUseAmmo(ps, ps->offHandIndex, 1);
            else
                PM_AddEvent(ps, 0x10u);
        }
        ps->weaponstate = WEAPON_OFFHAND;
        ps->weapFlags |= 2u;
    }
    else
    {
        BG_AddPredictableEventToPlayerstate(0x59u, ps->offHandIndex, ps);
        ps->weaponstate = WEAPON_SWIM_IN;
        ps->weapFlags &= ~2u;
        ps->weapFlags |= 0x2000u;
        ps->pm_flags &= ~0x200u;
    }
}

void __cdecl PM_Weapon_OffHandEnd(playerState_s *ps)
{
    bool v1; // eax
    const WeaponDef *weapDef; // [esp+4h] [ebp-4h]

    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 4145, 0, "%s", "ps") )
        __debugbreak();
    if ( ps->weapon )
    {
        weapDef = BG_GetWeaponDef(ps->weapon);
        v1 = PM_WeaponClipEmpty(ps);
        ps->weaponTime = weapDef->quickRaiseTime;
        ps->weaponDelay = 0;
        if ( v1 )
            PM_StartWeaponAnim(ps, 26, 0);
        else
            PM_StartWeaponAnim(ps, 24, 0);
        if ( weapDef->bDualWield )
            PM_StartWeaponAnim(ps, 24, 1);
    }
    else
    {
        ps->weaponTime = 0;
        ps->weaponDelay = 1;
        ps->weapFlags |= 0x80000u;
        if ( G_IsServerGameSystem(ps->clientNum) )
            Com_Printf(19, "end weapon (offhand end)\n");
    }
    ps->throwBackGrenadeTimeLeft = 0;
    ps->throwBackGrenadeOwner = 1023;
    ps->weaponstate = WEAPON_OFFHAND_END;
    ps->weapFlags &= ~2u;
    ps->pm_flags &= ~0x200u;
}

void __cdecl PM_Weapon_CheckForOffHand(pmove_t *pm)
{
    const char *v1; // eax
    const char *v2; // eax
    unsigned int FirstAvailableOffhand; // eax
    const char *v4; // eax
    const char *v5; // eax
    bool v6; // al
    const char *v7; // eax
    const char *v8; // eax
    OffhandSlot offhandSlot; // [esp+Ch] [ebp-10h]
    const WeaponDef *weapDef; // [esp+10h] [ebp-Ch]
    playerState_s *ps; // [esp+14h] [ebp-8h]
    unsigned int offHandIndex; // [esp+18h] [ebp-4h]

    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 4208, 0, "%s", "ps") )
        __debugbreak();
    if ( ps->waterlevel < 3
        && (ps->eFlags & 0x300) == 0
        && (ps->weapFlags & 0x80) == 0
        && (ps->weapFlags & 0x100) == 0
        && !BG_UsingVehicleWeapon(ps)
        && (ps->pm_flags & 0x8000) == 0
        && !PM_Weapon_IsHoldingGrenade(pm)
        && ps->weaponstate != 3
        && ps->weaponstate != 4
        && ps->weaponstate != 5
        && (ps->weaponstate < 20 || ps->weaponstate > 24)
        && (ps->weaponstate < 40 || ps->weaponstate > 42)
        && (ps->weaponstate < 47 || ps->weaponstate > 49)
        && (ps->eFlags2 & 0x10000000) == 0
        && (ps->weapFlags & 0x100000) == 0 )
    {
        if ( (ps->weapFlags & 2) != 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp",
                        4244,
                        0,
                        "%s",
                        "!(ps->weapFlags & PWF_USING_OFFHAND)") )
        {
            __debugbreak();
        }
        if ( BG_PlayerHasWeapon(ps, pm->cmd.offHandIndex) )
        {
            ps->offHandIndex = pm->cmd.offHandIndex;
            if ( ps->offHandIndex )
            {
                if ( BG_GetWeaponDef(ps->offHandIndex)->offhandClass == OFFHAND_CLASS_NONE )
                {
                    v1 = BG_WeaponName(ps->offHandIndex);
                    v2 = va("ps->offHandIndex = %d (%s)\n", ps->offHandIndex, v1);
                    if ( !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp",
                                    4249,
                                    0,
                                    "%s\n\t%s",
                                    "ps->offHandIndex == WP_NONE || BG_GetWeaponDef( ps->offHandIndex )->offhandClass != OFFHAND_CLASS_NONE",
                                    v2) )
                        __debugbreak();
                }
            }
        }
        if ( !pm->cmd.button_bits.testBit(0xEu) || pm->oldcmd.button_bits.testBit(0xEu) )
        {
            if ( !pm->cmd.button_bits.testBit(0xFu) || pm->oldcmd.button_bits.testBit(0xFu) )
                return;
            offhandSlot = OFFHAND_SLOT_TACTICAL_GRENADE;
            FirstAvailableOffhand = BG_GetFirstAvailableOffhand(ps, 2);
        }
        else
        {
            offhandSlot = OFFHAND_SLOT_LETHAL_GRENADE;
            FirstAvailableOffhand = BG_GetFirstAvailableOffhand(ps, 1);
        }
        offHandIndex = FirstAvailableOffhand;
        if ( FirstAvailableOffhand )
        {
            if ( BG_GetWeaponDef(FirstAvailableOffhand)->offhandClass == OFFHAND_CLASS_NONE )
            {
                v4 = BG_WeaponName(offHandIndex);
                v5 = va("offHandIndex = %d (%s)\n", offHandIndex, v4);
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp",
                                4267,
                                0,
                                "%s\n\t%s",
                                "offHandIndex == WP_NONE || BG_GetWeaponDef( offHandIndex )->offhandClass != OFFHAND_CLASS_NONE",
                                v5) )
                    __debugbreak();
            }
        }
        if ( offHandIndex )
        {
            BG_AddPredictableEventToPlayerstate(0x2Bu, offHandIndex, ps);
            ps->offHandIndex = offHandIndex;
            weapDef = BG_GetWeaponDef(ps->offHandIndex);
            v6 = pm->cmd.button_bits.testBit(0xEu);
            if ( !v6 || weapDef->offhandClass == OFFHAND_CLASS_FRAG_GRENADE )
            {
                if ( weapDef->weapType != WEAPTYPE_GRENADE && weapDef->weapType != WEAPTYPE_MINE )
                {
                    v7 = BG_WeaponName(ps->offHandIndex);
                    Com_Error(ERR_DROP, "[%s] Only grenades and mines are currently supported for off hand use\n", v7);
                }
                if ( weapDef->offhandClass == OFFHAND_CLASS_NONE )
                {
                    v8 = BG_WeaponName(ps->offHandIndex);
                    Com_Error(ERR_DROP, "[%s] No offhand class set\n", v8);
                }
                if ( ps->cursorHintEntIndex == 1023 && (!ps->weapon || ps->weaponstate == 25) )
                    PM_Weapon_OffHandPrepare(pm);
                else
                    PM_Weapon_OffHandInit(ps);
            }
        }
        else
        {
            PM_SendEmtpyOffhandEvent(ps, offhandSlot);
        }
    }
}

void __cdecl PM_Weapon_OffHandInit(playerState_s *ps)
{
    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 3948, 0, "%s", "ps") )
        __debugbreak();
    if ( !ps->offHandIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp",
                    3949,
                    0,
                    "%s",
                    "ps->offHandIndex != WP_NONE") )
    {
        __debugbreak();
    }
    if ( ps->waterlevel < 3 )
    {
        ps->weaponstate = WEAPON_OFFHAND_INIT;
        ps->weaponDelay = 0;
        ps->weapFlags &= ~2u;
        if ( ps->weapon )
            ps->weapFlags |= 0x20000u;
        ps->throwBackGrenadeOwner = 1023;
        PM_ExitAimDownSight(ps);
    }
}

void __cdecl PM_SendEmtpyOffhandEvent(playerState_s *ps, OffhandSlot offhandSlot)
{
    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 4185, 0, "%s", "ps") )
        __debugbreak();
    PM_AddEvent(ps, 0x10u);
    if ( BG_GetFirstEquippedOffhand(ps, offhandSlot) )
    {
        if ( offhandSlot == OFFHAND_SLOT_LETHAL_GRENADE )
        {
            PM_AddEvent(ps, 0x62u);
        }
        else if ( offhandSlot == OFFHAND_SLOT_TACTICAL_GRENADE )
        {
            PM_AddEvent(ps, 0x63u);
        }
    }
}

char __cdecl PM_UpdateGrenadeThrow(playerState_s *ps, pml_t *pml)
{
    int eventParm; // [esp+4h] [ebp-Ch]
    unsigned int weapIndex; // [esp+8h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+Ch] [ebp-4h]

    if ( (ps->weapFlags & 2) != 0 )
    {
        weapIndex = ps->offHandIndex;
        if ( !weapIndex
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp",
                        4306,
                        0,
                        "%s",
                        "weapIndex != WP_NONE") )
        {
            __debugbreak();
        }
    }
    else
    {
        weapIndex = ps->weapon;
        if ( !ps->weapon )
            return 0;
    }
    weapDef = BG_GetWeaponDef(weapIndex);
    if ( weapDef->weapType != WEAPTYPE_GRENADE && weapDef->weapType != WEAPTYPE_MINE )
        return 0;
    if ( ps->grenadeTimeLeft <= 0 )
        return 0;
    if ( weapDef->bCookOffHold )
        ps->grenadeTimeLeft -= pml->msec;
    if ( ps->waterlevel >= 3 && (ps->weapFlags & 2) != 0 && ps->weaponstate == 23 )
    {
        ps->weaponstate = WEAPON_OFFHAND_HOLD;
        PM_WeaponUseAmmo(ps, weapIndex, 1);
        return 0;
    }
    else if ( ps->grenadeTimeLeft > 0 )
    {
        return 0;
    }
    else
    {
        ps->grenadeTimeLeft = -1;
        eventParm = ps->offHandIndex;
        BG_AddPredictableEventToPlayerstate(0x5Au, eventParm, ps);
        if ( !BG_ThrowingBackGrenade(ps) )
            PM_WeaponUseAmmo(ps, eventParm, 1);
        return 1;
    }
}

char __cdecl PM_Weapon_CheckGrenadeHold(pmove_t *pm, int delayedAction)
{
    playerState_s *ps; // [esp+0h] [ebp-4h]

    if ( !pm && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 4372, 0, "%s", "pm") )
        __debugbreak();
    if ( !delayedAction )
        return 0;
    ps = pm->ps;
    if ( !pm->ps->weapon
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp",
                    4379,
                    0,
                    "%s",
                    "ps->weapon != WP_NONE") )
    {
        __debugbreak();
    }
    if ( !PM_Weapon_IsHoldingGrenade(pm) )
        return 0;
    ps->weaponDelay = 1;
    return 1;
}

void __cdecl PM_Weapon_CheckForDetonation(pmove_t *pm)
{
    playerState_s *ps; // [esp+0h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+4h] [ebp-4h]

    if ( !pm && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 4398, 0, "%s", "pm") )
        __debugbreak();
    ps = pm->ps;
    if ( pm->ps->weapon )
    {
        weapDef = BG_GetWeaponDef(ps->weapon);
        if ( !weapDef
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 4405, 0, "%s", "weapDef") )
        {
            __debugbreak();
        }
        if ( (weapDef->weapType == WEAPTYPE_GRENADE || weapDef->weapType == WEAPTYPE_MINE)
            && weapDef->hasDetonator
            && ps->weaponstate != 26
            && ps->weaponstate != 10
            && ps->weaponstate != 12
            && ps->weaponstate != 14
            && ps->weaponstate != 13
            && ps->weaponstate != 11
            && ps->weaponstate != 15
            && ps->weaponstate != 16
            && ps->weaponstate != 6
            && ps->weaponstate != 7
            && ps->weaponstate != 31
            && ps->weaponstate != 17
            && ps->weaponstate != 18
            && ps->weaponstate != 19
            && ps->weaponstate != 17
            && ps->weaponstate != 18
            && ps->weaponstate != 19
            && ps->weaponstate != 1
            && ps->weaponstate != 2
            && ps->weaponstate != 3
            && ps->weaponstate != 4
            && ps->weaponstate != 5
            && (ps->weaponstate < 20 || ps->weaponstate > 25)
            && (ps->weaponstate < 40 || ps->weaponstate > 42) )
        {
            if ( pm->cmd.button_bits.testBit(0) )
            {
                ps->weaponstate = WEAPON_DETONATING;
                ps->weaponTime = weapDef->iDetonateTime;
                ps->weaponDelay = weapDef->iDetonateDelay;
                PM_StartWeaponAnim(ps, 40, 0);
            }
        }
    }
}

void __cdecl PM_Weapon_CheckForGrenadeThrowCancel(pmove_t *pm)
{
    playerState_s *ps; // [esp+0h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+4h] [ebp-4h]
    const WeaponDef *weapDefa; // [esp+4h] [ebp-4h]
    const WeaponDef *weapDefb; // [esp+4h] [ebp-4h]

    if ( !pm && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 4444, 0, "%s", "pm") )
        __debugbreak();
    ps = pm->ps;
    if ( pm->ps->weaponstate == 21 )
    {
        weapDef = BG_GetWeaponDef(ps->offHandIndex);
        if ( !weapDef
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 4450, 0, "%s", "weapDef") )
        {
            __debugbreak();
        }
        if ( weapDef->holdButtonToThrow )
        {
            if ( pm->cmd.button_bits.testBit(0xEu) || pm->cmd.button_bits.testBit(0xFu) )
                return;
        }
        else if ( !weapDef->offhandHoldIsCancelable || !pm->cmd.button_bits.testBit(0x31u) )
        {
            return;
        }
        PM_Weapon_OffHandEnd(ps);
        goto LABEL_23;
    }
    if ( ps->weaponstate != 20 )
        goto LABEL_23;
    weapDefa = BG_GetWeaponDef(ps->offHandIndex);
    if ( !weapDefa
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 4471, 0, "%s", "weapDef") )
    {
        __debugbreak();
    }
    if ( !weapDefa->offhandHoldIsCancelable )
    {
LABEL_23:
        if ( ps->weaponstate < 20 || ps->weaponstate > 25 )
            weapDefb = BG_GetWeaponDef(ps->weapon);
        else
            weapDefb = BG_GetWeaponDef(ps->offHandIndex);
        if ( !weapDefb
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 4487, 0, "%s", "weapDef") )
        {
            __debugbreak();
        }
        if ( (weapDefb->weapType == WEAPTYPE_GRENADE || weapDefb->weapType == WEAPTYPE_MINE)
            && (weapDefb->offhandHoldIsCancelable
             || ps->weapon
             && (ps->weaponstate == 6
                || ps->weaponstate == 7
                || ps->weaponstate == 31
                || ps->weaponstate == 17
                || ps->weaponstate == 18
                || ps->weaponstate == 19)
             && weapDefb->holdButtonToThrow)
            && !pm->cmd.button_bits.testBit(0)
            && (!weapDefb->offhandHoldIsCancelable || pm->cmd.button_bits.testBit(0x31u)) )
        {
            PM_Weapon_Idle(ps);
            PM_StartWeaponAnim(ps, 1, 0);
        }
        return;
    }
    if ( pm->cmd.button_bits.testBit(0x31u) )
        PM_Weapon_OffHandEnd(ps);
}

void __cdecl PM_Detonate(playerState_s *ps, int delayedAction)
{
    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 4518, 0, "%s", "ps") )
        __debugbreak();
    if ( delayedAction && ps->weapon )
        PM_AddEvent(ps, 0x5Bu);
    else
        PM_Weapon_Idle(ps);
}

void __cdecl PM_Weapon_CheckForNightVision(pmove_t *pm)
{
    playerState_s *ps; // [esp+8h] [ebp-8h]

    if ( !pm && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 4536, 0, "%s", "pm") )
        __debugbreak();
    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 4538, 0, "%s", "ps") )
        __debugbreak();
    BG_GetWeaponDef(ps->weapon);
    if ( !pm->oldcmd.button_bits.testBit(0x12u) && pm->cmd.button_bits.testBit(0x12u) )
    {
        if ( (ps->weapFlags & 0x40) != 0 )
        {
            ps->weapFlags &= ~0x40u;
            PM_AddEvent(ps, 0x5Du);
        }
        else
        {
            ps->weapFlags |= 0x40u;
            PM_AddEvent(ps, 0x5Cu);
        }
    }
}

void __cdecl PM_Weapon_FinishNightVisionWear(playerState_s *ps)
{
    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 4602, 0, "%s", "ps") )
        __debugbreak();
    if ( ps->weaponstate != 33
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp",
                    4603,
                    0,
                    "%s",
                    "ps->weaponstate == WEAPON_NIGHTVISION_WEAR") )
    {
        __debugbreak();
    }
    if ( !ps->weaponTime )
    {
        ps->weaponstate = WEAPON_READY;
        PM_StartWeaponAnim(ps, 0, 0);
    }
}

void __cdecl PM_Weapon_FinishNightVisionRemove(playerState_s *ps)
{
    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 4614, 0, "%s", "ps") )
        __debugbreak();
    if ( ps->weaponstate != 34
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp",
                    4615,
                    0,
                    "%s",
                    "ps->weaponstate == WEAPON_NIGHTVISION_REMOVE") )
    {
        __debugbreak();
    }
    if ( !ps->weaponTime )
    {
        ps->weaponstate = WEAPON_READY;
        PM_StartWeaponAnim(ps, 0, 0);
    }
}

void __cdecl PM_WeaponSwimIn(playerState_s *ps)
{
    PM_AddEvent(ps, 0x1Au);
    if ( (ps->weapFlags & 0x2000) != 0 )
        PM_StartWeaponAnim(ps, 12, 0);
    else
        PM_ContinueWeaponAnim(ps, 12, 0);
}

void __cdecl PM_WeaponSwimOut(playerState_s *ps)
{
    ps->weaponstate = WEAPON_RAISING;
    PM_AddEvent(ps, 0x18u);
    PM_StartWeaponAnim(ps, 13, 0);
}

void __cdecl Sprint_State_Loop(playerState_s *ps)
{
    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 4670, 0, "%s", "ps") )
        __debugbreak();
    ps->weaponstate = WEAPON_SPRINT_LOOP;
    ps->weaponTime = 0;
    ps->weaponDelay = 0;
    if ( PM_WeaponClipEmpty(ps) )
    {
        PM_StartWeaponAnim(ps, 31, 0);
        PM_StartWeaponAnim(ps, 31, 1);
    }
    else
    {
        PM_StartWeaponAnim(ps, 28, 0);
        PM_StartWeaponAnim(ps, 28, 1);
    }
}

void __cdecl ContinuousFire_State_Loop(playerState_s *ps)
{
    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 4737, 0, "%s", "ps") )
        __debugbreak();
    ps->weaponstate = WEAPON_CONT_FIRE_LOOP;
    ps->weaponTime = 0;
    ps->weaponDelay = 0;
    PM_StartWeaponAnim(ps, 37, 0);
}

void __cdecl Dtp_State_Out(playerState_s *ps)
{
    const WeaponDef *WeaponDef; // eax

    WeaponDef = BG_GetWeaponDef(ps->weapon);
    ps->weaponstate = WEAPON_DTP_OUT;
    ps->weaponstateLeft = 42;
    ps->weaponTime = WeaponDef->dtpOutTime;
    ps->weaponDelay = 0;
    if ( PM_WeaponClipEmpty(ps) )
    {
        PM_ContinueWeaponAnim(ps, 50, 0);
        PM_ContinueWeaponAnim(ps, 50, 1);
    }
    else
    {
        PM_ContinueWeaponAnim(ps, 47, 0);
        PM_ContinueWeaponAnim(ps, 47, 1);
    }
}

void __cdecl Dtp_State_Loop(playerState_s *ps)
{
    BG_GetWeaponDef(ps->weapon);
    ps->weaponstate = WEAPON_DTP_LOOP;
    ps->weaponstateLeft = 41;
    ps->weaponTime = 0;
    ps->weaponDelay = 0;
    if ( PM_WeaponClipEmpty(ps) )
    {
        PM_ContinueWeaponAnim(ps, 49, 0);
        PM_ContinueWeaponAnim(ps, 49, 1);
    }
    else
    {
        PM_ContinueWeaponAnim(ps, 46, 0);
        PM_ContinueWeaponAnim(ps, 46, 1);
    }
}

void __cdecl PM_Weapon_CheckForSprint(pmove_t *pm)
{
    playerState_s *ps; // [esp+0h] [ebp-4h]

    if ( !pm && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 4901, 0, "%s", "pm") )
        __debugbreak();
    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 4903, 0, "%s", "ps") )
        __debugbreak();
    if ( ps->waterlevel < 3
        && pm->cmd.weapon
        && ps->weaponstate != 6
        && ps->weaponstate != 7
        && ps->weaponstate != 31
        && ps->weaponstate != 17
        && ps->weaponstate != 18
        && ps->weaponstate != 19
        && ps->weaponstateLeft != 6
        && ps->weaponstateLeft != 7
        && ps->weaponstateLeft != 31
        && ps->weaponstateLeft != 17
        && ps->weaponstateLeft != 18
        && ps->weaponstateLeft != 19
        && ps->weaponstate != 17
        && ps->weaponstate != 18
        && ps->weaponstate != 19
        && ps->weaponstate != 1
        && ps->weaponstate != 2
        && ps->weaponstate != 3
        && ps->weaponstate != 4
        && ps->weaponstate != 5
        && (ps->weaponstate < 20 || ps->weaponstate > 25)
        && (ps->weaponstate < 40 || ps->weaponstate > 42)
        && ps->weaponstate != 26
        && (ps->pm_flags & 0x400000) == 0 )
    {
        if ( (ps->pm_flags & 0x8000) != 0 && (ps->weaponstate < 27 || ps->weaponstate > 29) )
        {
            Sprint_State_Raise(ps);
        }
        else if ( (ps->pm_flags & 0x8000) == 0 && (ps->weaponstate == 27 || ps->weaponstate == 28) )
        {
            Sprint_State_Drop(ps);
        }
    }
}

void __cdecl Sprint_State_Raise(playerState_s *ps)
{
    const WeaponDef *WeaponDef; // eax

    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 4648, 0, "%s", "ps") )
        __debugbreak();
    WeaponDef = BG_GetWeaponDef(ps->weapon);
    ps->weaponstate = WEAPON_SPRINT_RAISE;
    ps->weaponTime = WeaponDef->sprintInTime;
    ps->weaponDelay = 0;
    if ( PM_WeaponClipEmpty(ps) )
    {
        PM_StartWeaponAnim(ps, 30, 0);
        PM_StartWeaponAnim(ps, 30, 1);
    }
    else
    {
        PM_StartWeaponAnim(ps, 27, 0);
        PM_StartWeaponAnim(ps, 27, 1);
    }
}

void __cdecl Sprint_State_Drop(playerState_s *ps)
{
    const WeaponDef *weapDef; // [esp+10h] [ebp-4h]

    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 4692, 0, "%s", "ps") )
        __debugbreak();
    weapDef = BG_GetWeaponDef(ps->weapon);
    ps->weaponstate = WEAPON_SPRINT_DROP;
    ps->weaponTime = weapDef->sprintOutTime;
    if ( (ps->perks[1] & 0x4000) != 0 )
        ps->weaponTime = (int)(float)((float)weapDef->sprintOutTime * perk_sprintRecoveryMultiplier->current.value);
    ps->weaponDelay = 0;
    if ( PM_WeaponClipEmpty(ps) )
    {
        PM_StartWeaponAnim(ps, 32, 0);
        PM_StartWeaponAnim(ps, 32, 1);
    }
    else
    {
        PM_StartWeaponAnim(ps, 29, 0);
        PM_StartWeaponAnim(ps, 29, 1);
    }
}

void __cdecl PM_Weapon_CheckForContinuousFire(pmove_t *pm, int delayedAction)
{
    int shouldBeFiring; // [esp+0h] [ebp-Ch]
    const WeaponDef *weapDef; // [esp+4h] [ebp-8h]
    playerState_s *ps; // [esp+8h] [ebp-4h]

    if ( !pm && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 4999, 0, "%s", "pm") )
        __debugbreak();
    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 5001, 0, "%s", "ps") )
        __debugbreak();
    if ( pm->cmd.weapon )
    {
        weapDef = BG_GetWeaponDef(ps->weapon);
        if ( weapDef->bContinuousFire
            && (ps->waterlevel < 3 || !player_disableWeaponsInWater->current.enabled)
            && (ps->weaponstate != 6
             && ps->weaponstate != 7
             && ps->weaponstate != 31
             && ps->weaponstate != 17
             && ps->weaponstate != 18
             && ps->weaponstate != 19
             || ps->weaponstate == 31)
            && ps->weaponstate != 17
            && ps->weaponstate != 18
            && ps->weaponstate != 19
            && ps->weaponstate != 1
            && ps->weaponstate != 2
            && ps->weaponstate != 3
            && ps->weaponstate != 4
            && ps->weaponstate != 5
            && (ps->weaponstate < 20 || ps->weaponstate > 25)
            && (ps->weaponstate < 40 || ps->weaponstate > 42)
            && ps->weaponstate != 26 )
        {
            shouldBeFiring = PM_Weapon_ShouldBeFiring(pm, delayedAction, 1);
            if ( weapDef->overheatWeapon && BG_PlayerWeaponOverheating(ps, ps->weapon) )
                shouldBeFiring = 0;
            if ( shouldBeFiring && (ps->weaponstate < 30 || ps->weaponstate > 32) )
            {
                ContinuousFire_State_In(ps);
            }
            else if ( !shouldBeFiring && (ps->weaponstate == 30 || ps->weaponstate == 31) )
            {
                ContinuousFire_State_Out(ps);
            }
        }
    }
}

void __cdecl ContinuousFire_State_In(playerState_s *ps)
{
    const WeaponDef *WeaponDef; // eax

    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 4724, 0, "%s", "ps") )
        __debugbreak();
    WeaponDef = BG_GetWeaponDef(ps->weapon);
    ps->weaponstate = WEAPON_CONT_FIRE_IN;
    ps->weaponTime = WeaponDef->contFireInTime;
    ps->weaponDelay = 0;
    PM_StartWeaponAnim(ps, 36, 0);
}

void __cdecl ContinuousFire_State_Out(playerState_s *ps)
{
    const WeaponDef *WeaponDef; // eax

    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 4750, 0, "%s", "ps") )
        __debugbreak();
    WeaponDef = BG_GetWeaponDef(ps->weapon);
    ps->weaponstate = WEAPON_CONT_FIRE_OUT;
    ps->weaponTime = WeaponDef->contFireOutTime;
    ps->weaponDelay = 0;
    PM_StartWeaponAnim(ps, 38, 0);
}

void __cdecl PM_Weapon_CheckForDtp(pmove_t *pm)
{
    playerState_s *ps; // [esp+0h] [ebp-4h]

    if ( !pm && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 5051, 0, "%s", "pm") )
        __debugbreak();
    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 5053, 0, "%s", "ps") )
        __debugbreak();
    if ( ps->waterlevel < 3
        && pm->cmd.weapon
        && ps->weaponstate != 6
        && ps->weaponstate != 7
        && ps->weaponstate != 31
        && ps->weaponstate != 17
        && ps->weaponstate != 18
        && ps->weaponstate != 19
        && ps->weaponstate != 17
        && ps->weaponstate != 18
        && ps->weaponstate != 19
        && ps->weaponstate != 1
        && ps->weaponstate != 2
        && ps->weaponstate != 3
        && ps->weaponstate != 4
        && ps->weaponstate != 5
        && (ps->weaponstate < 20 || ps->weaponstate > 25)
        && (ps->weaponstate < 40 || ps->weaponstate > 42)
        && (ps->pm_flags & 0x400000) != 0
        && !Dtp_IsSliding(ps) )
    {
        Dtp_State_In(ps);
    }
}

void __cdecl Dtp_State_In(playerState_s *ps)
{
    const WeaponDef *WeaponDef; // eax

    WeaponDef = BG_GetWeaponDef(ps->weapon);
    ps->weaponstate = WEAPON_DTP_IN;
    ps->weaponstateLeft = 40;
    ps->weaponTime = WeaponDef->dtpInTime;
    ps->weaponDelay = 0;
    if ( PM_WeaponClipEmpty(ps) )
    {
        PM_ContinueWeaponAnim(ps, 48, 0);
        PM_ContinueWeaponAnim(ps, 48, 1);
    }
    else
    {
        PM_ContinueWeaponAnim(ps, 45, 0);
        PM_ContinueWeaponAnim(ps, 45, 1);
    }
}

void __cdecl PM_Weapon_CheckForSlide(pmove_t *pm)
{
    playerState_s *ps; // [esp+0h] [ebp-4h]

    if ( !pm && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 5083, 0, "%s", "pm") )
        __debugbreak();
    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 5085, 0, "%s", "ps") )
        __debugbreak();
    if ( ps->waterlevel < 3
        && pm->cmd.weapon
        && ps->weaponstate != 6
        && ps->weaponstate != 7
        && ps->weaponstate != 31
        && ps->weaponstate != 17
        && ps->weaponstate != 18
        && ps->weaponstate != 19
        && ps->weaponstate != 17
        && ps->weaponstate != 18
        && ps->weaponstate != 19
        && ps->weaponstate != 1
        && ps->weaponstate != 2
        && ps->weaponstate != 3
        && ps->weaponstate != 4
        && ps->weaponstate != 5
        && (ps->weaponstate < 20 || ps->weaponstate > 25)
        && (ps->weaponstate < 40 || ps->weaponstate > 42)
        && ps->weaponstate != 43
        && (ps->pm_flags & 0x800000) != 0
        && ps->slideTime == pm->cmd.serverTime )
    {
        Slide_State_In(ps);
    }
}

void __cdecl Slide_State_In(playerState_s *ps)
{
    const WeaponDef *WeaponDef; // eax

    WeaponDef = BG_GetWeaponDef(ps->weapon);
    ps->weaponstate = WEAPON_SLIDE_IN;
    ps->weaponTime = WeaponDef->slideInTime;
    ps->weaponDelay = 0;
    PM_ContinueWeaponAnim(ps, 51, 0);
}

void __cdecl PM_Weapon_CheckForDeploy(pmove_t *pm)
{
    playerState_s *ps; // [esp+8h] [ebp-4h]

    if ( !pm && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 5118, 0, "%s", "pm") )
        __debugbreak();
    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 5120, 0, "%s", "ps") )
        __debugbreak();
    if ( BG_GetWeaponDef(ps->weapon)->mountableWeapon
        && (ps->weaponstate < 20 || ps->weaponstate > 25)
        && ps->weaponstate != 1
        && ps->weaponstate != 2
        && ps->weaponstate != 3
        && ps->weaponstate != 4
        && ps->weaponstate != 5
        && (ps->weaponstate < 40 || ps->weaponstate > 42) )
    {
        if ( (ps->weapFlags & 0x800) == 0
            || ps->weaponstate == 10
            || ps->weaponstate == 12
            || ps->weaponstate == 14
            || ps->weaponstate == 13
            || ps->weaponstate == 11
            || ps->weaponstate == 15
            || ps->weaponstate == 16
            || BG_GetAmmoInClip(ps, ps->weapon) <= 0 )
        {
            if ( (ps->weapFlags & 0x1000) != 0 )
            {
                ps->weapFlags &= ~0x1000u;
                Deploy_State_Breakdown(ps);
            }
        }
        else
        {
            ps->weapFlags &= ~0x800u;
            if ( ps->groundEntityNum != 1023 && ps->velocity[2] >= 0.0 )
                Deploy_State_Deploy(ps);
        }
    }
}

void __cdecl Deploy_State_Deploy(playerState_s *ps)
{
    const WeaponDef *WeaponDef; // eax

    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 4871, 0, "%s", "ps") )
        __debugbreak();
    WeaponDef = BG_GetWeaponDef(ps->weapon);
    ps->weaponstate = WEAPON_DEPLOYING;
    ps->weaponTime = WeaponDef->deployTime;
    ps->weaponDelay = 0;
    PM_StartWeaponAnim(ps, 43, 0);
}

void __cdecl Deploy_State_Breakdown(playerState_s *ps)
{
    const WeaponDef *WeaponDef; // eax

    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 4886, 0, "%s", "ps") )
        __debugbreak();
    WeaponDef = BG_GetWeaponDef(ps->weapon);
    ps->weaponstate = WEAPON_BREAKING_DOWN;
    ps->weaponTime = WeaponDef->breakdownTime;
    ps->weaponDelay = 0;
    PM_StartWeaponAnim(ps, 44, 0);
}

void __cdecl PM_Weapon_CheckForMantle(pmove_t *pm)
{
    playerState_s *ps; // [esp+0h] [ebp-4h]

    if ( !pm && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 5153, 0, "%s", "pm") )
        __debugbreak();
    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 5155, 0, "%s", "ps") )
        __debugbreak();
    if ( Mantle_IsWeaponEquipped(ps) )
    {
        if ( ps->weaponstate == 29 )
            ps->weaponTime = 0;
        PM_ContinueWeaponAnim(ps, 52, 0);
    }
    else if ( (ps->pm_flags & 4) == 0 && (ps->weapAnim & 0xFFFFFBFF) == 0x34 )
    {
        PM_Weapon_Idle(ps);
    }
}

int __cdecl BG_CheckForUniversalActionDW(playerState_s *ps)
{
    int result; // eax

    switch ( ps->weaponstate )
    {
        case 1:
        case 3:
        case 4:
        case 0xB:
        case 0xC:
        case 0xD:
        case 0xE:
        case 0x11:
        case 0x12:
        case 0x13:
        case 0x15:
        case 0x16:
        case 0x17:
        case 0x18:
        case 0x1A:
        case 0x1B:
        case 0x1C:
        case 0x1D:
        case 0x1E:
        case 0x1F:
        case 0x20:
        case 0x28:
        case 0x29:
        case 0x2A:
            result = 0;
            break;
        default:
            result = 1;
            break;
    }
    return result;
}

void __cdecl PM_Weapon_Turret(pmove_t *pm, pml_t *pml)
{
    playerState_s *ps; // [esp+0h] [ebp-4h]

    ps = pm->ps;
    if ( !pm->ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 5665, 0, "%s", "ps") )
        __debugbreak();
    if ( ps->pm_type < 9 )
    {
        if ( !CG_IsShowingZombieMap() && (ps->pm_flags & 0x400) == 0 && (ps->eFlags & 0x4000) == 0 && !BurstFirePending(ps) )
            PM_Weapon_CheckForChangeWeapon(pm);
    }
    else
    {
        ps->weapon = 0;
    }
}

void __cdecl PM_ResetWeaponState(playerState_s *ps)
{
    PM_Weapon_Idle(ps);
}

void __cdecl BG_WeaponFireRecoil(const playerState_s *ps, float *recoilSpeed, float *kickAVel)
{
    float adsGunKickReducedKickPercent; // xmm0_4
    float v4; // [esp+0h] [ebp-24h]
    const WeaponDef *weapDef; // [esp+4h] [ebp-20h]
    const WeaponDef *weapDefDW; // [esp+8h] [ebp-1Ch]
    float fReducePercent; // [esp+Ch] [ebp-18h]
    float fYawKick; // [esp+10h] [ebp-14h]
    float fYawKicka; // [esp+10h] [ebp-14h]
    float fYawKickb; // [esp+10h] [ebp-14h]
    float fPitchKick; // [esp+14h] [ebp-10h]
    float fPitchKicka; // [esp+14h] [ebp-10h]
    float fPitchKickb; // [esp+14h] [ebp-10h]
    int weapIndex; // [esp+18h] [ebp-Ch]
    float fPosLerp; // [esp+1Ch] [ebp-8h]
    const WeaponDef *weapDef2; // [esp+20h] [ebp-4h]

    weapIndex = BG_GetViewmodelWeaponIndex(ps);
    weapDef2 = BG_GetWeaponDef(weapIndex);
    fPosLerp = ps->fWeaponPosFrac;
    fReducePercent = 1.0f;
    if (ps->weaponRestrictKickTime > 0)
    {
        if (fPosLerp == 1.0)
            adsGunKickReducedKickPercent = weapDef2->adsGunKickReducedKickPercent;
        else
            adsGunKickReducedKickPercent = weapDef2->hipGunKickReducedKickPercent;
        fReducePercent = adsGunKickReducedKickPercent * 0.0099999998;
    }
    if (fPosLerp != 1.0)
    {
        if (weapDef2->bDualWield)
        {
            weapDefDW = BG_GetWeaponDef(weapDef2->dualWieldWeaponIndex);
            if ((ps->weaponstate == 6 || ps->weaponstate == 31) && (ps->weaponstateLeft == 6 || ps->weaponstateLeft == 31))
            {
                fPitchKick = bgs->Random()
                    * (weapDef2->fHipViewKickPitchMax
                        + weapDefDW->fHipViewKickPitchMax
                        - (weapDefDW->fHipViewKickPitchMin
                            + weapDef2->fHipViewKickPitchMin))
                    + weapDef2->fHipViewKickPitchMin
                    + weapDefDW->fHipViewKickPitchMin;
                fYawKick = bgs->Random()
                    * (weapDef2->fHipViewKickYawMax
                        + weapDefDW->fHipViewKickYawMax
                        - (weapDef2->fHipViewKickYawMin
                            + weapDefDW->fHipViewKickYawMin))
                    + weapDef2->fHipViewKickYawMin
                    + weapDefDW->fHipViewKickYawMin;
                goto LABEL_18;
            }
            if (ps->weaponstateLeft == 6 || ps->weaponstateLeft == 31)
            {
                fPitchKick = bgs->Random()
                    * (weapDefDW->fHipViewKickPitchMax - weapDefDW->fHipViewKickPitchMin)
                    + weapDefDW->fHipViewKickPitchMin;
                fYawKick = bgs->Random()
                    * (weapDefDW->fHipViewKickYawMax - weapDefDW->fHipViewKickYawMin)
                    + weapDefDW->fHipViewKickYawMin;
                goto LABEL_18;
            }
        }
        fPitchKick = bgs->Random()
            * (weapDef2->fHipViewKickPitchMax - weapDef2->fHipViewKickPitchMin)
            + weapDef2->fHipViewKickPitchMin;
        fYawKick = bgs->Random()
            * (weapDef2->fHipViewKickYawMax - weapDef2->fHipViewKickYawMin)
            + weapDef2->fHipViewKickYawMin;
        goto LABEL_18;
    }
    v4 = weapDef2->fAdsViewKickPitchMax - weapDef2->fAdsViewKickPitchMin;
    fPitchKick = bgs->Random() * v4
        + weapDef2->fAdsViewKickPitchMin;
    fYawKick = bgs->Random()
        * (weapDef2->fAdsViewKickYawMax - weapDef2->fAdsViewKickYawMin)
        + weapDef2->fAdsViewKickYawMin;
LABEL_18:
    fPitchKicka = fPitchKick * fReducePercent;
    fYawKicka = fYawKick * fReducePercent;
    if (weapDef2->weapType == WEAPTYPE_GAS && *kickAVel == 0.0)
    { 
        *kickAVel = *kickAVel - 0.001;
    }
    else
    {
        //*(_DWORD *)kickAVel = LODWORD(fPitchKicka) ^ _mask__NegFloat_;
        *kickAVel = -fPitchKicka;
    }
    kickAVel[1] = fYawKicka;
    kickAVel[2] = -0.5 * kickAVel[1];
    if (fPosLerp <= 0.0)
    {
        if (weapDef2->bDualWield)
        {
            weapDef = BG_GetWeaponDef(weapDef2->dualWieldWeaponIndex);
            if ((ps->weaponstate == 6 || ps->weaponstate == 31) && (ps->weaponstateLeft == 6 || ps->weaponstateLeft == 31))
            {
                fPitchKickb = bgs->Random()
                    * (weapDef2->fHipGunKickPitchMax
                        + weapDef->fHipGunKickPitchMax
                        - (weapDef->fHipGunKickPitchMin
                            + weapDef2->fHipGunKickPitchMin))
                    + weapDef2->fHipGunKickPitchMin
                    + weapDef->fHipGunKickPitchMin;
                fYawKickb = bgs->Random()
                    * (weapDef2->fHipGunKickYawMax
                        + weapDef->fHipGunKickYawMax
                        - (weapDef2->fHipGunKickYawMin
                            + weapDef->fHipGunKickYawMin))
                    + weapDef2->fHipGunKickYawMin
                    + weapDef->fHipGunKickYawMin;
                goto LABEL_34;
            }
            if (ps->weaponstateLeft == 6 || ps->weaponstateLeft == 31)
            {
                fPitchKickb = bgs->Random()
                    * (weapDef->fHipGunKickPitchMax - weapDef->fHipGunKickPitchMin)
                    + weapDef->fHipGunKickPitchMin;
                fYawKickb = bgs->Random()
                    * (weapDef->fHipGunKickYawMax - weapDef->fHipGunKickYawMin)
                    + weapDef->fHipGunKickYawMin;
                goto LABEL_34;
            }
        }
        fPitchKickb = bgs->Random()
            * (weapDef2->fHipGunKickPitchMax - weapDef2->fHipGunKickPitchMin)
            + weapDef2->fHipGunKickPitchMin;
        fYawKickb = bgs->Random()
            * (weapDef2->fHipGunKickYawMax - weapDef2->fHipGunKickYawMin)
            + weapDef2->fHipGunKickYawMin;
        goto LABEL_34;
    }
    fPitchKickb = bgs->Random()
        * (weapDef2->fAdsGunKickPitchMax - weapDef2->fAdsGunKickPitchMin)
        + weapDef2->fAdsGunKickPitchMin;
    fYawKickb = bgs->Random()
        * (weapDef2->fAdsGunKickYawMax - weapDef2->fAdsGunKickYawMin)
        + weapDef2->fAdsGunKickYawMin;
LABEL_34:
    *recoilSpeed = *recoilSpeed + (float)(fPitchKickb * fReducePercent);
    recoilSpeed[1] = recoilSpeed[1] + (float)(fYawKickb * fReducePercent);
}

int __cdecl BG_PlayerWeaponCountPrimaryTypes(const playerState_s *ps)
{
    int slot; // [esp+0h] [ebp-10h]
    int weapIndex; // [esp+4h] [ebp-Ch]
    int resultCount; // [esp+Ch] [ebp-4h]

    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 5819, 0, "%s", "ps") )
        __debugbreak();
    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 5825, 0, "%s", "ps") )
        __debugbreak();
    resultCount = 0;
    for ( slot = 0; slot < 15; ++slot )
    {
        weapIndex = ps->heldWeapons[slot].weapon;
        if ( weapIndex )
        {
            if ( BG_GetWeaponDef(weapIndex)->inventoryType == WEAPINVENTORY_PRIMARY )
                ++resultCount;
        }
    }
    return resultCount;
}

bool __cdecl BG_PlayerWeaponsFull_Primaries(const playerState_s *ps)
{
    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 5850, 0, "%s", "ps") )
        __debugbreak();
    return BG_PlayerWeaponCountPrimaryTypes(ps) >= 2;
}

char __cdecl BG_PlayerHasCompatibleWeapon(const playerState_s *ps, unsigned int weaponIndex)
{
    int slot; // [esp+0h] [ebp-Ch]
    int ammoIndex; // [esp+4h] [ebp-8h]
    int weapIndex; // [esp+8h] [ebp-4h]

    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 5862, 0, "%s", "ps") )
        __debugbreak();
    ammoIndex = BG_GetWeaponVariantDef(weaponIndex)->iAmmoIndex;
    for ( slot = 0; slot < 15; ++slot )
    {
        weapIndex = ps->heldWeapons[slot].weapon;
        if ( weapIndex && BG_AmmoForWeapon(weapIndex) == ammoIndex )
            return 1;
    }
    return 0;
}

bool __cdecl BG_ThrowingBackGrenade(const playerState_s *ps)
{
    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 5882, 0, "%s", "ps") )
        __debugbreak();
    return ps->throwBackGrenadeOwner != 1023;
}

static const float ofsPowZ = 5.0f;
// (aislop)
void __cdecl BG_CalcVehicleTurretWeaponPosOffset(float positionFrac, const WeaponDef *weapDef, float *outOffset)
{
    float v8 = sin(positionFrac * 3.1415927f - 1.5707964f);
    float adjustedFrac = (v8 + 1.0f) / 2.0f;

    // Default prone offsets
    float offsetX = weapDef->vProneOfs[0];
    float offsetY = weapDef->vProneOfs[1];
    float offsetZ = weapDef->vProneOfs[2];

    // Default ADS offsets
    float offsetADS_X = weapDef->vDuckedOfs[0];
    float offsetADS_Y = weapDef->vDuckedOfs[1];
    float offsetADS_Z = weapDef->vDuckedOfs[2];

    // Override with vehCameraTurretOffset if non-zero
    if (vehCameraTurretOffset->current.value != 0.0f ||
        vehCameraTurretOffset->current.vector[1] != 0.0f ||
        vehCameraTurretOffset->current.vector[2] != 0.0f)
    {
        offsetX = vehCameraTurretOffset->current.value;
        offsetY = vehCameraTurretOffset->current.vector[1];
        offsetZ = vehCameraTurretOffset->current.vector[2];
    }

    if (vehCameraTurretOffsetADS->current.value != 0.0f ||
        vehCameraTurretOffsetADS->current.vector[1] != 0.0f ||
        vehCameraTurretOffsetADS->current.vector[2] != 0.0f)
    {
        offsetADS_X = vehCameraTurretOffsetADS->current.value;
        offsetADS_Y = vehCameraTurretOffsetADS->current.vector[1];
        offsetADS_Z = vehCameraTurretOffsetADS->current.vector[2];
    }

    // Linear interpolation
    outOffset[0] = adjustedFrac * (offsetADS_X - offsetX) + offsetX;
    outOffset[1] = adjustedFrac * (offsetADS_Y - offsetY) + offsetY;
    outOffset[2] = adjustedFrac * (offsetADS_Z - offsetZ) + offsetZ;

    // Apply Z offset scaling with power
    float ofs = (offsetADS_Z - offsetZ >= 0.0f) ? (1.0f / ofsPowZ) : ofsPowZ;
    outOffset[2] = powf(offsetADS_Z - offsetZ, ofs) * adjustedFrac + offsetZ;
}

WeaponVariantDef *__cdecl BG_LoadWeaponVariantDef(const char *name)
{
    bool v2; // [esp+4h] [ebp-8h]

    v2 = fs_gameDirVar && *(_BYTE *)fs_gameDirVar->current.integer;
    if ( v2 || !useFastFile->current.enabled )
        return BG_LoadWeaponVariantDef_LoadObj((char*)name);
    else
        return BG_LoadWeaponVariantDef_FastFile(name);
}

WeaponVariantDef *__cdecl BG_LoadWeaponVariantDef_LoadObj(char *name)
{
    WeaponVariantDef *weapVariantDef; // [esp+0h] [ebp-4h]
    WeaponVariantDef *weapVariantDefa; // [esp+0h] [ebp-4h]

    if ( !*name )
        return 0;
    weapVariantDef = (WeaponVariantDef*)BG_LoadWeaponVariantDefInternal("mp", name); // KISAKTODO: bad cast
    if ( weapVariantDef )
        return weapVariantDef;
    weapVariantDefa = (WeaponVariantDef*)BG_LoadWeaponVariantDefInternal("mp", (char*)"defaultweapon_mp");
    if ( !weapVariantDefa )
        Com_Error(ERR_DROP, "BG_LoadWeaponVariantDef: Could not find default weapon");
    SetConfigString((char **)weapVariantDefa, name);
    return weapVariantDefa;
}

WeaponVariantDef *__cdecl BG_LoadWeaponVariantDef_FastFile(const char *name)
{
    if ( !*name )
        return 0;
    return DB_FindXAssetHeader(ASSET_TYPE_WEAPON, (char*)name, 1, -1).weapon;
}

void __cdecl BG_AssertOffhandIndexOrNone(unsigned int offHandIndex)
{
    const char *v1; // eax
    const char *v2; // eax

    if ( offHandIndex && BG_GetWeaponDef(offHandIndex)->offhandClass == OFFHAND_CLASS_NONE )
    {
        v1 = BG_WeaponName(offHandIndex);
        v2 = va("Weapon #%d (%s) expected to be offhand weapon or no weapon at all, but is not.", offHandIndex, v1);
        if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 5972, 0, v2) )
            __debugbreak();
    }
}

int __cdecl BG_GetFuelTankTime(const playerState_s *ps, unsigned int weaponIndex)
{
    const PlayerHeldWeapon *heldWeapon; // [esp+8h] [ebp-4h]

    heldWeapon = BG_GetHeldWeaponConst(ps, weaponIndex);
    if ( heldWeapon )
        return heldWeapon->fuelTankTime;
    else
        return 0;
}

void __cdecl BG_SetFuelTankTime(playerState_s *ps, unsigned int weaponIndex, int time)
{
    PlayerHeldWeapon *heldWeapon; // [esp+8h] [ebp-4h]

    heldWeapon = BG_GetHeldWeapon(ps, weaponIndex);
    if ( heldWeapon )
        heldWeapon->fuelTankTime = time;
}

void __cdecl BG_AddFuelTankTime(playerState_s *ps, unsigned int weaponIndex, int time)
{
    PlayerHeldWeapon *heldWeapon; // [esp+8h] [ebp-4h]

    heldWeapon = BG_GetHeldWeapon(ps, weaponIndex);
    if ( heldWeapon )
        heldWeapon->fuelTankTime += time;
}

char __cdecl BG_HoldWeapon(playerState_s *ps, unsigned int weaponIndex)
{
    const char *v2; // eax
    const char *v3; // eax
    PlayerHeldWeapon *v5; // ecx
    int slot; // [esp+14h] [ebp-8h]
    int slota; // [esp+14h] [ebp-8h]

    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons.cpp", 6046, 0, "%s", "ps") )
        __debugbreak();
    slot = BG_GetFreeHeldWeaponSlot(ps);
    if ( slot == -1 )
    {
        v2 = BG_WeaponName(weaponIndex);
        Com_PrintError(16, "Could not find empty held weapon slot when trying to hold weapon %s\n", v2);
        for ( slota = 0; slota < 15; ++slota )
        {
            v3 = BG_WeaponName(ps->heldWeapons[slota].weapon);
            Com_PrintError(16, "Holding weapon %s in slot %i \n", v3, slota);
        }
        return 0;
    }
    else
    {
        v5 = &ps->heldWeapons[slot];
        v5->weapon = 0;
        *(unsigned int *)&v5->model = 0;
        v5->options.i = 0;
        v5->fuelTankTime = 0;
        *(unsigned int *)&v5->overHeating = 0;
        *(unsigned int *)&v5->blockWeaponPickup = 0;
        v5->weapon = weaponIndex;
        BG_PlayerSetWeaponQuickReload(ps, weaponIndex, 1);
        return 1;
    }
}

int __cdecl BG_GetFreeHeldWeaponSlot(const playerState_s *ps)
{
    int slot; // [esp+0h] [ebp-4h]

    if ( !ps && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\bgame\\../bgame/bg_weapons.h", 286, 0, "%s", "ps") )
        __debugbreak();
    for ( slot = 0; slot < 15; ++slot )
    {
        if ( !ps->heldWeapons[slot].weapon )
            return slot;
    }
    return -1;
}

const char *__cdecl BG_WeaponName(unsigned int weapon)
{
    return BG_GetWeaponVariantDef(weapon)->szInternalName;
}

void __cdecl BG_StringCopy(unsigned __int8 *member, const char *keyValue)
{
    char v2; // al

    do
    {
        v2 = *keyValue;
        *member++ = *keyValue++;
    }
    while ( v2 );
}

