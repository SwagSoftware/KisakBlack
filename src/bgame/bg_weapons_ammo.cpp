#include "bg_weapons_ammo.h"
#include "bg_weapons.h"
#include <string.h>
#include "bg_weapons_def.h"
#include <qcommon/common.h>
#include "bg_weapons_load_obj.h"
#include "bg_local.h"
#include "bg_mantle.h"
#include "bg_misc.h"
#include "bg_pmove.h"
#include <flame/flame_system.h>

unsigned int bg_numWeapClips;
unsigned int bg_numAmmoTypes;
unsigned int bg_numSharedAmmoCaps;

WeaponVariantDef *bg_weapClips[2048];
WeaponVariantDef *bg_weapAmmoTypes[2048];
WeaponDef *bg_sharedAmmoCaps[2048];


void __cdecl BG_SetupWeaponDefAmmoIndexes(unsigned int weapIndex, WeaponDef *weapDef, WeaponVariantDef *weapVarDef)
{
    const WeaponVariantDef *dwWeaponVarDef; // [esp+14h] [ebp-Ch]
    unsigned int dwIndex; // [esp+18h] [ebp-8h]
    unsigned int index; // [esp+1Ch] [ebp-4h]

    if ( !weapDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_ammo.cpp", 40, 0, "%s", "weapDef") )
    {
        __debugbreak();
    }
    if ( weapDef->sharedAmmo )
    {
        for ( index = 0; index < bg_numAmmoTypes; ++index )
        {
            if ( !strcmp(bg_weapAmmoTypes[index]->szAmmoName, weapVarDef->szAmmoName) )
            {
                weapVarDef->iAmmoIndex = index;
                return;
            }
        }
    }
    if ( weapVarDef->iAmmoIndex < 0
        || weapVarDef->iAmmoIndex >= (signed int)bg_numAmmoTypes
        || bg_weapAmmoTypes[weapVarDef->iAmmoIndex] != weapVarDef )
    {
        if ( weapDef->bDualWield
            && (dwIndex = BG_GetWeaponIndexForName((char *)weapDef->szDualWieldWeaponName)) != 0
            && (dwWeaponVarDef = BG_GetWeaponVariantDef(dwIndex), dwWeaponVarDef->iAmmoIndex >= 0)
            && dwWeaponVarDef->iAmmoIndex < (signed int)bg_numAmmoTypes
            && bg_weapAmmoTypes[dwWeaponVarDef->iAmmoIndex] == dwWeaponVarDef )
        {
            weapVarDef->iAmmoIndex = dwWeaponVarDef->iAmmoIndex;
        }
        else
        {
            if ( bg_numAmmoTypes >= 0x800
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_ammo.cpp",
                            78,
                            0,
                            "bg_numAmmoTypes doesn't index ARRAY_COUNT( bg_weapAmmoTypes )\n\t%i not in [0, %i)",
                            bg_numAmmoTypes,
                            2048) )
            {
                __debugbreak();
            }
            bg_weapAmmoTypes[bg_numAmmoTypes] = weapVarDef;
            weapVarDef->iAmmoIndex = bg_numAmmoTypes++;
        }
    }
}

void __cdecl BG_SetupWeaponDefSharedAmmoIndexes(unsigned int weapIndex, WeaponDef *weapDef)
{
    const char *v2; // eax
    const char *v3; // eax
    int v4; // [esp-Ch] [ebp-18h]
    const char *v5; // [esp-8h] [ebp-14h]
    const char *szSharedAmmoCapName; // [esp-4h] [ebp-10h]
    int iSharedAmmoCap; // [esp-4h] [ebp-10h]
    unsigned int otherWeapIndex; // [esp+0h] [ebp-Ch]
    const WeaponDef *otherWeapDef; // [esp+4h] [ebp-8h]
    unsigned int index; // [esp+8h] [ebp-4h]

    if ( !weapDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_ammo.cpp", 87, 0, "%s", "weapDef") )
    {
        __debugbreak();
    }
    weapDef->iSharedAmmoCapIndex = -1;
    if ( *weapDef->szSharedAmmoCapName )
    {
        szSharedAmmoCapName = weapDef->szSharedAmmoCapName;
        v2 = BG_WeaponName(weapIndex);
        Com_DPrintf(17, "%s: %s\n", v2, szSharedAmmoCapName);
        for ( index = 0; ; ++index )
        {
            if ( index >= bg_numSharedAmmoCaps )
            {
                bg_sharedAmmoCaps[index] = weapDef;
                weapDef->iSharedAmmoCapIndex = index;
                ++bg_numSharedAmmoCaps;
                return;
            }
            if ( !I_stricmp(bg_sharedAmmoCaps[index]->szSharedAmmoCapName, weapDef->szSharedAmmoCapName) )
                break;
        }
        weapDef->iSharedAmmoCapIndex = index;
        if ( bg_sharedAmmoCaps[index]->iSharedAmmoCap != weapDef->iSharedAmmoCap && index )
        {
            for ( otherWeapIndex = 1; otherWeapIndex < weapIndex; ++otherWeapIndex )
            {
                otherWeapDef = BG_GetWeaponDef(otherWeapIndex);
                if ( !I_stricmp(bg_sharedAmmoCaps[index]->szSharedAmmoCapName, otherWeapDef->szSharedAmmoCapName)
                    && otherWeapDef->iSharedAmmoCap == bg_sharedAmmoCaps[index]->iSharedAmmoCap )
                {
                    iSharedAmmoCap = otherWeapDef->iSharedAmmoCap;
                    v5 = BG_WeaponName(otherWeapIndex);
                    v4 = weapDef->iSharedAmmoCap;
                    v3 = BG_WeaponName(weapIndex);
                    Com_Error(ERR_DROP, "Shared ammo cap mismatch for \"%s\" shared ammo cap: %s set to %i but \"%s\" already set it to %i", weapDef->szSharedAmmoCapName, v3, v4, v5, iSharedAmmoCap);
                }
            }
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_ammo.cpp", 121, 0, "unreachable") )
                __debugbreak();
        }
    }
}

void __cdecl BG_SetupWeaponDefClipIndexes(WeaponDef *weapDef, WeaponVariantDef *weapVarDef)
{
    unsigned int index; // [esp+14h] [ebp-4h]

    if ( !weapDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_ammo.cpp", 135, 0, "%s", "weapDef") )
    {
        __debugbreak();
    }
    if ( weapDef->unlimitedAmmo )
    {
        weapVarDef->iClipIndex = -1;
    }
    else
    {
        if ( weapDef->sharedAmmo )
        {
            for ( index = 0; index < bg_numWeapClips; ++index )
            {
                if ( !strcmp(bg_weapClips[index]->szClipName, weapVarDef->szClipName) )
                {
                    weapVarDef->iClipIndex = index;
                    return;
                }
            }
        }
        if ( weapVarDef->iClipIndex < 0
            || weapVarDef->iClipIndex >= (signed int)bg_numWeapClips
            || bg_weapClips[weapVarDef->iClipIndex] != weapVarDef )
        {
            if ( bg_numWeapClips >= 0x800
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_ammo.cpp",
                            161,
                            0,
                            "bg_numWeapClips doesn't index ARRAY_COUNT( bg_weapClips )\n\t%i not in [0, %i)",
                            bg_numWeapClips,
                            2048) )
            {
                __debugbreak();
            }
            bg_weapClips[bg_numWeapClips] = weapVarDef;
            weapVarDef->iClipIndex = bg_numWeapClips++;
        }
    }
}

void __cdecl BG_ClearWeaponDefAmmo()
{
    bg_weapAmmoTypes[0] = BG_LoadDefaultWeaponVariantDef();
    bg_numAmmoTypes = 1;
    bg_sharedAmmoCaps[0] = bg_weapAmmoTypes[0]->weapDef;
    bg_numSharedAmmoCaps = 1;
    bg_weapClips[0] = bg_weapAmmoTypes[0];
    bg_numWeapClips = 1;
}

int __cdecl BG_GetSharedAmmoCapSize(unsigned int capIndex)
{
    if ( capIndex >= bg_numSharedAmmoCaps
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_ammo.cpp",
                    195,
                    0,
                    "capIndex doesn't index bg_numSharedAmmoCaps\n\t%i not in [0, %i)",
                    capIndex,
                    bg_numSharedAmmoCaps) )
    {
        __debugbreak();
    }
    return bg_sharedAmmoCaps[capIndex]->iSharedAmmoCap;
}

AmmoClip *__cdecl BG_GetAmmoClip(playerState_s *ps, int clipIndex)
{
    int slot; // [esp+0h] [ebp-4h]

    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_ammo.cpp", 258, 0, "%s", "ps") )
        __debugbreak();
    for ( slot = 0; slot < 15; ++slot )
    {
        if ( ps->ammoInClip[slot].clipIndex == clipIndex )
            return &ps->ammoInClip[slot];
    }
    return 0;
}

void __cdecl BG_AddAmmoToClip(playerState_s *ps, int clipIndex, int amount)
{
    const char *v3; // eax
    AmmoClip *clip; // [esp+0h] [ebp-4h]

    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_ammo.cpp", 335, 0, "%s", "ps") )
        __debugbreak();
    clip = BG_AddAmmoClip(ps, clipIndex);
    if ( !clip )
    {
        v3 = va("Tried to add ammo for clipIndex %i but there are no free available clips", clipIndex);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_ammo.cpp",
                        338,
                        0,
                        "%s\n\t%s",
                        "clip",
                        v3) )
            __debugbreak();
    }
    if ( amount + clip->count <= 0 )
        clip->count = 0;
    else
        clip->count += amount;
}

AmmoClip *__cdecl BG_AddAmmoClip(playerState_s *ps, int clipIndex)
{
    AmmoClip *clip; // [esp+0h] [ebp-4h]

    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_ammo.cpp", 319, 0, "%s", "ps") )
        __debugbreak();
    clip = BG_GetAmmoClip(ps, clipIndex);
    if ( !clip )
    {
        clip = BG_GetFreeAmmoClip(ps);
        clip->clipIndex = clipIndex;
    }
    return clip;
}

AmmoClip *__cdecl BG_GetFreeAmmoClip(playerState_s *ps)
{
    AmmoClip *result; // eax
    const char *v2; // eax
    bool matched; // [esp+3h] [ebp-Dh]
    int weaponSlot; // [esp+8h] [ebp-8h]
    int slot; // [esp+Ch] [ebp-4h]
    int slota; // [esp+Ch] [ebp-4h]

    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_ammo.cpp", 273, 0, "%s", "ps") )
        __debugbreak();
    for ( slot = 0; slot < 15; ++slot )
    {
        if ( !ps->ammoInClip[slot].clipIndex )
        {
            result = &ps->ammoInClip[slot];
            result->clipIndex = 0;
            result->count = 0;
            return result;
        }
    }
    for ( slota = 0; slota < 15; ++slota )
    {
        if ( !ps->ammoInClip[slota].clipIndex
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_ammo.cpp",
                        294,
                        0,
                        "%s",
                        "clip->clipIndex != 0") )
        {
            __debugbreak();
        }
        matched = 0;
        for ( weaponSlot = 0; weaponSlot < 15; ++weaponSlot )
        {
            if ( BG_GetWeaponVariantDef(ps->heldWeapons[weaponSlot].weapon)->iClipIndex == ps->ammoInClip[slota].clipIndex )
            {
                v2 = BG_WeaponName(ps->heldWeapons[weaponSlot].weapon);
                Com_PrintError(17, "%i ammo for %s\n", ps->ammoInClip[slota].count, v2);
                matched = 1;
                break;
            }
        }
        if ( !matched )
            Com_PrintError(17, "found ammo clip %i that we do not have a weapon for\n", slota);
    }
    return 0;
}

void __cdecl BG_SetAmmoInClip(playerState_s *ps, int clipIndex, int amount)
{
    const char *v3; // eax
    AmmoClip *clip; // [esp+0h] [ebp-4h]

    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_ammo.cpp", 348, 0, "%s", "ps") )
        __debugbreak();
    clip = BG_AddAmmoClip(ps, clipIndex);
    if ( !clip )
    {
        v3 = va("Tried to set ammo for clipIndex %i but there are no free available clips", clipIndex);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_ammo.cpp",
                        351,
                        0,
                        "%s\n\t%s",
                        "clip",
                        v3) )
            __debugbreak();
    }
    if ( amount <= 0 )
        clip->count = 0;
    else
        clip->count = amount;
}

AmmoPool *__cdecl BG_GetAmmoPool(playerState_s *ps, int ammoIndex)
{
    int slot; // [esp+0h] [ebp-4h]

    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_ammo.cpp", 361, 0, "%s", "ps") )
        __debugbreak();
    for ( slot = 0; slot < 15; ++slot )
    {
        if ( ps->ammoNotInClip[slot].ammoIndex == ammoIndex )
            return &ps->ammoNotInClip[slot];
    }
    return 0;
}

void __cdecl BG_AddAmmoToPool(playerState_s *ps, unsigned int weaponIndex, int amount)
{
    int v3; // eax
    AmmoPool *pool; // [esp+0h] [ebp-4h]

    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_ammo.cpp", 439, 0, "%s", "ps") )
        __debugbreak();
    v3 = BG_AmmoForWeapon(weaponIndex);
    pool = BG_GetAmmoPool(ps, v3);
    if ( !pool )
    {
        pool = BG_GetFreeAmmoPool(ps);
        pool->ammoIndex = BG_AmmoForWeapon(weaponIndex);
        pool->count = 0;
    }
    pool->count += amount;
    if ( pool->count < 0 )
        pool->count = 0;
}

AmmoPool *__cdecl BG_GetFreeAmmoPool(playerState_s *ps)
{
    AmmoPool *result; // eax
    const char *v2; // eax
    bool matched; // [esp+3h] [ebp-Dh]
    int weaponSlot; // [esp+8h] [ebp-8h]
    int slot; // [esp+Ch] [ebp-4h]
    int slota; // [esp+Ch] [ebp-4h]

    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_ammo.cpp", 393, 0, "%s", "ps") )
        __debugbreak();
    for ( slot = 0; slot < 15; ++slot )
    {
        if ( !ps->ammoNotInClip[slot].ammoIndex )
        {
            result = &ps->ammoNotInClip[slot];
            result->ammoIndex = 0;
            result->count = 0;
            return result;
        }
    }
    for ( slota = 0; slota < 15; ++slota )
    {
        if ( !ps->ammoNotInClip[slota].ammoIndex
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_ammo.cpp",
                        414,
                        0,
                        "%s",
                        "pool->ammoIndex != 0") )
        {
            __debugbreak();
        }
        matched = 0;
        for ( weaponSlot = 0; weaponSlot < 15; ++weaponSlot )
        {
            if ( BG_GetWeaponVariantDef(ps->heldWeapons[weaponSlot].weapon)->iAmmoIndex == ps->ammoNotInClip[slota].ammoIndex )
            {
                v2 = BG_WeaponName(ps->heldWeapons[weaponSlot].weapon);
                Com_PrintError(16, "%i ammo for %s\n", ps->ammoNotInClip[slota].count, v2);
                matched = 1;
                break;
            }
        }
        if ( !matched )
            Com_PrintError(16, "found ammo pool %i that we do not have a weapon for\n", slota);
    }
    return ps->ammoNotInClip;
}

void __cdecl BG_SetAmmoInPool(playerState_s *ps, unsigned int weaponIndex, int amount)
{
    int v3; // eax
    AmmoPool *pool; // [esp+0h] [ebp-4h]

    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_ammo.cpp", 455, 0, "%s", "ps") )
        __debugbreak();
    v3 = BG_AmmoForWeapon(weaponIndex);
    pool = BG_GetAmmoPool(ps, v3);
    if ( !pool )
    {
        pool = BG_GetFreeAmmoPool(ps);
        pool->ammoIndex = BG_AmmoForWeapon(weaponIndex);
        pool->count = 0;
    }
    pool->count = amount;
    if ( pool->count < 0 )
        pool->count = 0;
}

int __cdecl BG_GetAmmoPlayerMax(const playerState_s *ps, unsigned int weaponIndex, unsigned int weaponIndexToSkip)
{
    const WeaponDef *thisWeapDef; // [esp+38h] [ebp-18h]
    const WeaponVariantDef *weapVarDef; // [esp+3Ch] [ebp-14h]
    int slot; // [esp+40h] [ebp-10h]
    int total; // [esp+44h] [ebp-Ch]
    unsigned int thisWeapIdx; // [esp+48h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+4Ch] [ebp-4h]

    weapDef = BG_GetWeaponDef(weaponIndex);
    weapVarDef = BG_GetWeaponVariantDef(weaponIndex);
    if ( weapDef->iSharedAmmoCapIndex >= 0 )
        return BG_GetSharedAmmoCapSize(weapDef->iSharedAmmoCapIndex);
    if ( BG_WeaponIsClipOnly(weaponIndex) )
        return BG_GetClipSize(weaponIndex);
    total = 0;
    for ( slot = 0; slot < 15; ++slot )
    {
        thisWeapIdx = ps->heldWeapons[slot].weapon;
        if ( thisWeapIdx != weaponIndexToSkip )
        {
            thisWeapDef = BG_GetWeaponDef(thisWeapIdx);
            if ( BG_GetWeaponVariantDef(thisWeapIdx)->iAmmoIndex == weapVarDef->iAmmoIndex
                && (thisWeapDef->inventoryType != WEAPINVENTORY_ALTMODE
                 || BG_GetWeaponVariantDef(thisWeapIdx)->altWeaponIndex != thisWeapIdx) )
            {
                if ( thisWeapDef->iSharedAmmoCapIndex >= 0 )
                    return BG_GetSharedAmmoCapSize(thisWeapDef->iSharedAmmoCapIndex);
                total += BG_GetMaxAmmo(thisWeapIdx);
            }
        }
    }
    return total;
}

int __cdecl BG_GetMaxAmmo(unsigned int weaponIndex)
{
    const WeaponDef *weapDef; // [esp+18h] [ebp-4h]

    weapDef = BG_GetWeaponDef(weaponIndex);
    if ( weapDef->ammoCountClipRelative )
        return weapDef->iMaxAmmo * BG_GetClipSize(weaponIndex);
    else
        return weapDef->iMaxAmmo;
}

char __cdecl ValueInArray(const int *weaponArray, int value)
{
    int slot; // [esp+0h] [ebp-4h]

    for ( slot = 0; slot < 15; ++slot )
    {
        if ( weaponArray[slot] == value )
            return 1;
    }
    return 0;
}

void __cdecl AddValueToArray(int *weaponArray, int value)
{
    int slot; // [esp+0h] [ebp-4h]

    for ( slot = 0; slot < 15; ++slot )
    {
        if ( !weaponArray[slot] )
        {
            weaponArray[slot] = value;
            return;
        }
    }
    if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_ammo.cpp", 536, 0, "%s", "0") )
        __debugbreak();
}

int __cdecl BG_GetMaxPickupableAmmo(const playerState_s *ps, unsigned int weaponIndex)
{
    int ClipSize; // esi
    int v4; // eax
    int v5; // eax
    int v6; // eax
    int v7; // eax
    int maxAmmo; // [esp+34h] [ebp-90h]
    int ammo; // [esp+38h] [ebp-8Ch]
    int slot; // [esp+3Ch] [ebp-88h]
    int clipCounted[15]; // [esp+40h] [ebp-84h] BYREF
    const WeaponDef *curWeapDef; // [esp+7Ch] [ebp-48h]
    unsigned int currWeap; // [esp+80h] [ebp-44h]
    const WeaponDef *weapDef; // [esp+84h] [ebp-40h]
    int ammoCounted[15]; // [esp+88h] [ebp-3Ch] BYREF

    memset(ammoCounted, 0, sizeof(ammoCounted));
    memset(clipCounted, 0, sizeof(clipCounted));
    weapDef = BG_GetWeaponDef(weaponIndex);
    if ( weapDef->iSharedAmmoCapIndex >= 0 )
    {
        ammo = BG_GetSharedAmmoCapSize(weapDef->iSharedAmmoCapIndex);
        for ( slot = 0; slot < 15; ++slot )
        {
            currWeap = ps->heldWeapons[slot].weapon;
            if ( currWeap )
            {
                curWeapDef = BG_GetWeaponDef(currWeap);
                if ( curWeapDef->iSharedAmmoCapIndex == weapDef->iSharedAmmoCapIndex )
                {
                    if ( BG_WeaponIsClipOnly(currWeap) )
                    {
                        v4 = BG_ClipForWeapon(currWeap);
                        if ( !ValueInArray(clipCounted, v4) )
                        {
                            v5 = BG_ClipForWeapon(currWeap);
                            AddValueToArray(clipCounted, v5);
                            ammo -= BG_GetAmmoInClip(ps, currWeap);
                        }
                    }
                    else
                    {
                        v6 = BG_AmmoForWeapon(currWeap);
                        if ( !ValueInArray(ammoCounted, v6) )
                        {
                            v7 = BG_AmmoForWeapon(currWeap);
                            AddValueToArray(ammoCounted, v7);
                            ammo -= BG_GetAmmoNotInClip(ps, currWeap);
                        }
                    }
                }
            }
        }
        return ammo;
    }
    else if ( BG_WeaponIsClipOnly(weaponIndex) )
    {
        ClipSize = BG_GetClipSize(weaponIndex);
        return ClipSize - BG_GetAmmoInClip(ps, weaponIndex);
    }
    else
    {
        maxAmmo = BG_GetAmmoPlayerMax(ps, weaponIndex, 0);
        return maxAmmo - BG_GetAmmoNotInClip(ps, weaponIndex);
    }
}

int __cdecl BG_GetTotalAmmoReserve(const playerState_s *ps, unsigned int weaponIndex)
{
    int v2; // eax
    int v3; // eax
    int v4; // eax
    int v5; // eax
    int slot; // [esp+18h] [ebp-8Ch]
    int ammo; // [esp+1Ch] [ebp-88h]
    int clipCounted[15]; // [esp+20h] [ebp-84h] BYREF
    const WeaponDef *curWeapDef; // [esp+5Ch] [ebp-48h]
    unsigned int currWeap; // [esp+60h] [ebp-44h]
    const WeaponDef *weapDef; // [esp+64h] [ebp-40h]
    int ammoCounted[15]; // [esp+68h] [ebp-3Ch] BYREF

    ammo = 0;
    memset(ammoCounted, 0, sizeof(ammoCounted));
    memset(clipCounted, 0, sizeof(clipCounted));
    weapDef = BG_GetWeaponDef(weaponIndex);
    if ( weapDef->iSharedAmmoCapIndex < 0 )
    {
        if ( BG_WeaponIsClipOnly(weaponIndex) )
            return BG_GetAmmoInClip(ps, weaponIndex);
        else
            return BG_GetAmmoNotInClip(ps, weaponIndex);
    }
    else
    {
        for ( slot = 0; slot < 15; ++slot )
        {
            currWeap = ps->heldWeapons[slot].weapon;
            if ( currWeap )
            {
                curWeapDef = BG_GetWeaponDef(currWeap);
                if ( curWeapDef->iSharedAmmoCapIndex == weapDef->iSharedAmmoCapIndex )
                {
                    if ( BG_WeaponIsClipOnly(currWeap) )
                    {
                        v2 = BG_ClipForWeapon(currWeap);
                        if ( !ValueInArray(clipCounted, v2) )
                        {
                            v3 = BG_ClipForWeapon(currWeap);
                            AddValueToArray(clipCounted, v3);
                            ammo += BG_GetAmmoInClip(ps, currWeap);
                        }
                    }
                    else
                    {
                        v4 = BG_AmmoForWeapon(currWeap);
                        if ( !ValueInArray(ammoCounted, v4) )
                        {
                            v5 = BG_AmmoForWeapon(currWeap);
                            AddValueToArray(ammoCounted, v5);
                            ammo += BG_GetAmmoNotInClip(ps, currWeap);
                        }
                    }
                }
            }
        }
    }
    return ammo;
}

int __cdecl BG_ClipForWeapon(unsigned int weapon)
{
    return BG_GetWeaponVariantDef(weapon)->iClipIndex;
}

int __cdecl BG_AmmoForWeapon(unsigned int weapon)
{
    return BG_GetWeaponVariantDef(weapon)->iAmmoIndex;
}

bool __cdecl BG_WeaponIsClipOnly(unsigned int weapon)
{
    return BG_GetWeaponDef(weapon)->bClipOnly;
}

int __cdecl BG_WeaponAmmo(const playerState_s *ps, unsigned int weapon)
{
    int AmmoNotInClip; // esi

    AmmoNotInClip = BG_GetAmmoNotInClip(ps, weapon);
    return AmmoNotInClip + BG_GetAmmoInClip(ps, weapon);
}

void __cdecl PM_ReloadClip(playerState_s *ps)
{
    const WeaponVariantDef *weapVarDef; // [esp+24h] [ebp-20h]
    int ammoReserve; // [esp+28h] [ebp-1Ch]
    int ammoAdd; // [esp+2Ch] [ebp-18h]
    int clipSize; // [esp+30h] [ebp-14h]
    int ammoInClip; // [esp+34h] [ebp-10h]
    unsigned int weaponIndex; // [esp+38h] [ebp-Ch]
    const WeaponDef *weapDef; // [esp+3Ch] [ebp-8h]
    int *weaponState; // [esp+40h] [ebp-4h]

    weaponIndex = PM_GetWeaponIndexForHand(ps);
    weapDef = BG_GetWeaponDef(weaponIndex);
    weapVarDef = BG_GetWeaponVariantDef(weaponIndex);
    weaponState = BG_GetWeaponState(ps, ps->bRunLeftGun);
    if ( *weaponState != 12 && *weaponState != 13 || weapDef->iReloadStartAdd )
    {
        ammoReserve = BG_GetAmmoNotInClip(ps, weaponIndex);
        ammoInClip = BG_GetAmmoInClip(ps, weaponIndex);
        if ( weapDef->fuelTankWeapon )
        {
            ammoInClip = 0;
            BG_SetAmmoInClip(ps, weapVarDef->iClipIndex, 0);
        }
        clipSize = BG_GetClipSize(weaponIndex);
        ammoAdd = clipSize - ammoInClip;
        if ( clipSize - ammoInClip > ammoReserve )
            ammoAdd = ammoReserve;
        if ( *weaponState == 12 || *weaponState == 13 )
        {
            if ( weapDef->iReloadStartAdd < clipSize && ammoAdd > weapDef->iReloadStartAdd )
                ammoAdd = weapDef->iReloadStartAdd;
        }
        else if ( weapDef->iReloadAmmoAdd && weapDef->iReloadAmmoAdd < clipSize && ammoAdd > weapDef->iReloadAmmoAdd )
        {
            ammoAdd = weapDef->iReloadAmmoAdd;
        }
        if ( ammoAdd )
        {
            BG_AddAmmoToPool(ps, weaponIndex, -ammoAdd);
            BG_AddAmmoToClip(ps, weapVarDef->iClipIndex, ammoAdd);
            if ( weapDef->fuelTankWeapon )
                BG_SetFuelTankTime(ps, ps->weapon, 0);
            PM_AddEvent(ps, 0x17u);
        }
    }
}

void __cdecl PM_WeaponUseAmmo(playerState_s *ps, unsigned int wp, int amount)
{
    const WeaponVariantDef *WeaponVariantDef; // eax
    int v4; // [esp+0h] [ebp-10h]
    int AmmoInClip; // [esp+4h] [ebp-Ch]

    if ( !player_sustainAmmo->current.enabled || !Flame_GetLocalClientSourceRange() )
    {
        AmmoInClip = BG_GetAmmoInClip(ps, wp);
        if ( AmmoInClip < amount )
            v4 = AmmoInClip;
        else
            v4 = amount;
        WeaponVariantDef = BG_GetWeaponVariantDef(wp);
        BG_AddAmmoToClip(ps, WeaponVariantDef->iClipIndex, -v4);
    }
}

int __cdecl PM_WeaponAmmoAvailable(playerState_s *ps)
{
    int weapon; // [esp+8h] [ebp-4h]

    if ( !ps->bRunLeftGun )
        return BG_GetAmmoInClip(ps, ps->weapon);
    weapon = BG_GetWeaponDef(ps->weapon)->dualWieldWeaponIndex;
    return BG_GetAmmoInClip(ps, weapon);
}

bool __cdecl PM_WeaponClipEmpty(playerState_s *ps)
{
    unsigned int weapon; // [esp+8h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+Ch] [ebp-4h]

    weapDef = BG_GetWeaponDef(ps->weapon);
    if ( weapDef->unlimitedAmmo )
        return 0;
    weapon = ps->weapon;
    if ( ps->bRunLeftGun )
        weapon = weapDef->dualWieldWeaponIndex;
    return BG_GetAmmoInClip(ps, weapon) == 0;
}

