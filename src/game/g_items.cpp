#include "g_items.h"
#include <bgame/bg_weapons_ammo.h>
#include <bgame/bg_misc.h>
#include "g_weapon.h"
#include <game_mp/g_main_mp.h>
#include <game_mp/g_utils_mp.h>
#include <server/sv_game.h>
#include <game_mp/g_spawn_mp.h>
#include <clientscript/scr_const.h>
#include <server/sv_world.h>
#include <server_mp/sv_main_mp.h>
#include <clientscript/cscr_vm.h>
#include <universal/com_math_anglevectors.h>
#include <xanim/xmodel_utils.h>
#include <qcommon/dobj_management.h>
#include <xanim/dobj_utils.h>
#include "g_load_utils.h"
#include <clientscript/cscr_stringlist.h>
#include <server_mp/sv_init_mp.h>
#include <bgame/bg_weapons_def.h>
#include <cgame/cg_weapons.h>

int itemRegistered[2048];

void __cdecl Fill_Clip(playerState_s *ps, unsigned int weapon)
{
    const WeaponVariantDef *weapVarDef; // [esp+28h] [ebp-14h]
    int ammomove; // [esp+2Ch] [ebp-10h]
    int iAmmoIndex; // [esp+34h] [ebp-8h]
    int inclip; // [esp+38h] [ebp-4h]

    iAmmoIndex = BG_AmmoForWeapon(weapon);
    if ( weapon && weapon < BG_GetNumWeapons() )
    {
        inclip = BG_GetAmmoInClip(ps, weapon);
        ammomove = BG_GetClipSize(weapon) - inclip;
        if ( ammomove > BG_GetAmmoNotInClipForAmmoIndex(ps, iAmmoIndex) )
            ammomove = BG_GetAmmoNotInClipForAmmoIndex(ps, iAmmoIndex);
        if ( ammomove )
        {
            weapVarDef = BG_GetWeaponVariantDef(weapon);
            BG_AddAmmoToPool(ps, weapon, -ammomove);
            BG_AddAmmoToClip(ps, weapVarDef->iClipIndex, ammomove);
        }
    }
}

int __cdecl Add_Ammo(gentity_s *ent, unsigned int weaponIndex, unsigned __int8 weaponModel, int count, int fillClip)
{
    int AmmoInClip; // esi
    int ClipSize; // eax
    const WeaponVariantDef *weapVarDef2; // eax
    int v10; // esi
    const WeaponVariantDef *weapVarDef; // [esp+5Ch] [ebp-1Ch]
    int oldClip; // [esp+60h] [ebp-18h]
    bool clipOnly; // [esp+67h] [ebp-11h]
    int maxWeaponAmmo; // [esp+68h] [ebp-10h]
    int oldAmmo; // [esp+6Ch] [ebp-Ch]
    gclient_s *ps; // [esp+70h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+74h] [ebp-4h]
    int counta; // [esp+8Ch] [ebp+14h]

    if (!ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp", 77, 0, "%s", "ent"))
        __debugbreak();
    if (!ent->client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp", 78, 0, "%s", "ent->client"))
    {
        __debugbreak();
    }
    ps = ent->client;
    weapDef = BG_GetWeaponDef(weaponIndex);
    weapVarDef = BG_GetWeaponVariantDef(weaponIndex);
    if (!BG_PlayerHasWeapon(&ps->ps, weaponIndex) && weapDef->offhandClass != OFFHAND_CLASS_FRAG_GRENADE)
        return 0;
    clipOnly = 0;
    oldAmmo = BG_GetAmmoNotInClip(&ps->ps, weaponIndex);
    oldClip = BG_GetAmmoInClip(&ps->ps, weaponIndex);
    maxWeaponAmmo = BG_GetAmmoPlayerMax(&ps->ps, weaponIndex, 0);
    BG_AddAmmoToPool(&ps->ps, weaponIndex, count);
    if (BG_WeaponIsClipOnly(weaponIndex))
    {
        G_GivePlayerWeapon(&ps->ps, weaponIndex, weaponModel, 0);
        clipOnly = 1;
    }
    if (fillClip || clipOnly)
        Fill_Clip(&ps->ps, weaponIndex);
    if (clipOnly)
    {
        BG_SetAmmoInPool(&ps->ps, weaponIndex, 0);
    }
    else if (BG_GetAmmoNotInClip(&ps->ps, weaponIndex) > maxWeaponAmmo)
    {
        BG_SetAmmoInPool(&ps->ps, weaponIndex, maxWeaponAmmo);
    }
    AmmoInClip = BG_GetAmmoInClip(&ps->ps, weaponIndex);
    if (AmmoInClip > BG_GetClipSize(weaponIndex))
    {
        ClipSize = BG_GetClipSize(weaponIndex);
        BG_SetAmmoInClip(&ps->ps, weapVarDef->iClipIndex, ClipSize);
    }
    if (BG_GetWeaponDef(weaponIndex)->iSharedAmmoCapIndex >= 0)
    {
        counta = BG_GetMaxPickupableAmmo(&ps->ps, weaponIndex);
        if (counta < 0)
        {
            if (BG_WeaponIsClipOnly(weaponIndex))
            {
                weapVarDef2 = BG_GetWeaponVariantDef(weaponIndex);
                BG_AddAmmoToClip(&ps->ps, weapVarDef2->iClipIndex, counta);
                if (BG_GetAmmoInClip(&ps->ps, weaponIndex) <= 0)
                {
                    BG_TakePlayerWeapon(&ps->ps, weaponIndex);
                    return 0;
                }
            }
            else
            {
                BG_AddAmmoToPool(&ps->ps, weaponIndex, counta);
            }
        }
    }
    v10 = BG_GetAmmoNotInClip(&ps->ps, weaponIndex) - oldAmmo;
    return v10 + BG_GetAmmoInClip(&ps->ps, weaponIndex) - oldClip;
}

void __cdecl Touch_Item_Auto(gentity_s *ent, gentity_s *other, int bTouched)
{
    ent->active = 1;
    Touch_Item(ent, other, bTouched);
}

void __cdecl Touch_Item(gentity_s *ent, gentity_s *other, int touched)
{
    int Guid; // eax
    int number; // [esp-Ch] [ebp-68h]
    const char *v5; // [esp-4h] [ebp-60h]
    gitem_s *item; // [esp+0h] [ebp-5Ch]
    char cleanname[68]; // [esp+4h] [ebp-58h] BYREF
    int pickupEvent; // [esp+50h] [ebp-Ch] BYREF
    int pickedUp; // [esp+54h] [ebp-8h]
    int weapIndex; // [esp+58h] [ebp-4h]

    if ( ent->active )
    {
        pickupEvent = 0;
        ent->active = 0;
        if ( other->client )
        {
            if ( other->health >= 1 && !level.clientIsSpawning )
            {
                weapIndex = ent->s.un3.item % 2048;
                item = (gitem_s *)&bg_itemlist[ent->s.un3.item];
                if ( BG_CanItemBeGrabbed(&ent->s, &other->client->ps, touched) )
                {
                    I_strncpyz(cleanname, other->client->sess.cs.name, 64);
                    I_CleanStr(cleanname);
                    v5 = BG_WeaponName(weapIndex);
                    number = other->s.number;
                    Guid = SV_GetGuid(other->s.number);
                    G_LogPrintf("Weapon;%d;%d;%s;%s\n", Guid, number, cleanname, v5);
                    if ( !item && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp", 595, 0, "%s", "item") )
                        __debugbreak();
                    if ( item->giType != IT_WEAPON
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp",
                                    596,
                                    0,
                                    "%s",
                                    "item->giType == IT_WEAPON") )
                    {
                        __debugbreak();
                    }
                    pickedUp = WeaponPickup(ent, other, &pickupEvent, touched);
                    if ( pickupEvent )
                    {
                        if ( other->client->sess.predictItemPickup )
                            G_AddPredictableEvent(other, pickupEvent, weapIndex);
                        else
                            G_AddEvent(other, pickupEvent, weapIndex);
                    }
                    if ( pickedUp )
                    {
                        if ( (0x800000 & ent->s.lerp.eFlags2) != 0 )
                        {
                            Scr_AddEntity(other, SCRIPTINSTANCE_SERVER);
                            Scr_Notify(ent, scr_const.scavenger, 1u);
                        }
                        if ( ent->s.eType == 4 )
                            Scr_Notify(ent, scr_const.death, 0);
                        G_FreeEntity(ent);
                    }
                }
                else
                {
                    PrintMessage_CannotGrabItem(ent, other, touched, item, weapIndex);
                }
            }
        }
    }
}

int __cdecl WeaponPickup(gentity_s *weaponEnt, gentity_s *player, int *pickupEvent, int touched)
{
    unsigned int weapIdx; // [esp+0h] [ebp-Ch]
    const WeaponDef *weapDef; // [esp+4h] [ebp-8h]

    if ( !weaponEnt
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp", 508, 0, "%s", "weaponEnt") )
    {
        __debugbreak();
    }
    if ( !player && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp", 509, 0, "%s", "player") )
        __debugbreak();
    if ( !player->client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp", 510, 0, "%s", "player->client") )
    {
        __debugbreak();
    }
    weapIdx = weaponEnt->s.un3.item % 2048;
    weapDef = BG_GetWeaponDef(weapIdx);
    if ( !BG_PlayerCanPickUpWeaponType(weapDef, &player->client->ps) )
        return 0;
    if ( BG_GetWeaponDef(player->client->ps.weapon)->bIsCarriedKillstreakWeapon )
        return 0;
    if ( touched )
        return WeaponPickup_Touch(weaponEnt, player, weapIdx, pickupEvent);
    return WeaponPickup_Grab(weaponEnt, player, weapIdx, pickupEvent);
}

int __cdecl WeaponPickup_Grab(gentity_s *weaponEnt, gentity_s *player, int weapIdx, int *pickupEvent)
{
    unsigned __int8 weaponModel; // [esp+3h] [ebp-Dh]
    int gotWeapon; // [esp+4h] [ebp-Ch]
    gentity_s *droppedEnt; // [esp+8h] [ebp-8h] BYREF
    const WeaponDef *weapDef; // [esp+Ch] [ebp-4h]

    if ( !weaponEnt
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp", 429, 0, "%s", "weaponEnt") )
    {
        __debugbreak();
    }
    if ( !player && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp", 430, 0, "%s", "player") )
        __debugbreak();
    if ( !player->client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp", 431, 0, "%s", "player->client") )
    {
        __debugbreak();
    }
    droppedEnt = 0;
    weaponModel = weaponEnt->s.un3.item / 2048;
    weapDef = BG_GetWeaponDef(weapIdx);
    if ( weaponEnt->s.eType == 4 )
    {
        *pickupEvent = 12;
        WeaponPickup_Notifies(weaponEnt, 0, player, weapDef);
        return 1;
    }
    else
    {
        if ( !weaponEnt->r.inuse
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp", 444, 0, "%s", "weaponEnt->r.inuse") )
        {
            __debugbreak();
        }
        weaponEnt->flags |= 0x2000000u;
        gotWeapon = WeaponPickup_AddWeapon(weaponEnt, player, weapIdx, weaponModel, &droppedEnt);
        if ( !weaponEnt->r.inuse
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp", 449, 0, "%s", "weaponEnt->r.inuse") )
        {
            __debugbreak();
        }
        weaponEnt->flags &= ~0x2000000u;
        if ( gotWeapon )
        {
            *pickupEvent = 11;
            WeaponPickup_AddAmmoForNewWeapon(weaponEnt, player);
            if ( droppedEnt )
                WeaponPickup_LeechFromWeaponEnt(droppedEnt, player, 0, 0, 1);
            WeaponPickup_Notifies(weaponEnt, droppedEnt, player, weapDef);
            if ( weapDef->weapType != WEAPTYPE_GRENADE && weapDef->weapType != WEAPTYPE_MINE
                || weapDef->offhandClass == OFFHAND_CLASS_NONE )
            {
                G_SelectWeaponIndex(player - g_entities, weapIdx);
            }
            return 1;
        }
        else
        {
            return 0;
        }
    }
}

int __cdecl WeaponPickup_AddWeapon(
                gentity_s *ent,
                gentity_s *other,
                int weapon,
                unsigned __int8 weaponModel,
                gentity_s **pDroppedWeapon)
{
    const char *v6; // eax
    unsigned __int8 PlayerWeaponModel; // al
    int v8; // [esp+8h] [ebp-A8h]
    int passEntityNum; // [esp+Ch] [ebp-A4h]
    col_context_t context; // [esp+2Ch] [ebp-84h] BYREF
    gentity_s *droppedEnt; // [esp+54h] [ebp-5Ch]
    trace_t trace; // [esp+58h] [ebp-58h] BYREF
    int mask; // [esp+94h] [ebp-1Ch]
    float up[3]; // [esp+98h] [ebp-18h] BYREF
    playerState_s *ps; // [esp+A4h] [ebp-Ch]
    const WeaponDef *weapDef; // [esp+A8h] [ebp-8h]
    int playerWeapIdx; // [esp+ACh] [ebp-4h]
    int savedregs; // [esp+B0h] [ebp+0h] BYREF

    //col_context_t::col_context_t(&context);
    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp", 192, 0, "%s", "ent") )
        __debugbreak();
    if ( !other && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp", 193, 0, "%s", "other") )
        __debugbreak();
    droppedEnt = 0;
    ps = &other->client->ps;
    weapDef = BG_GetWeaponDef(weapon);
    if ( weapDef->inventoryType )
        goto LABEL_8;
    if ( ps->weapon && !BG_PlayerHasWeapon(ps, ps->weapon) )
        return 0;
    if ( !BG_PlayerWeaponsFull_Primaries(ps) )
    {
LABEL_8:
        G_GivePlayerWeapon(ps, weapon, weaponModel, ent->s.renderOptions);
        return 1;
    }
    playerWeapIdx = CurrentPrimaryWeapon(ps);
    if ( playerWeapIdx )
    {
        if ( !BG_PlayerHasWeapon(ps, playerWeapIdx)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp",
                        223,
                        0,
                        "%s",
                        "BG_PlayerHasWeapon( ps, playerWeapIdx )") )
        {
            __debugbreak();
        }
        PlayerWeaponModel = BG_GetPlayerWeaponModel(ps, playerWeapIdx);
        droppedEnt = Drop_Weapon(other, playerWeapIdx, PlayerWeaponModel, 0);
        if ( droppedEnt )
        {
            droppedEnt->spawnflags = ent->spawnflags & 0xFFFFFFFE;
            droppedEnt->s.groundEntityNum = ent->s.groundEntityNum;
            if ( (droppedEnt->flags & 0x1000) != 0 && ent->tagInfo && ent->tagInfo->parent )
            {
                G_SetOrigin(droppedEnt, ent->r.currentOrigin);
                G_SetAngle(droppedEnt, ent->r.currentAngles);
                G_EntLinkTo(droppedEnt, ent->tagInfo->parent, ent->tagInfo->name);
            }
            else
            {
                up[0] = ent->r.currentOrigin[0];
                up[1] = ent->r.currentOrigin[1];
                up[2] = ent->r.currentOrigin[2];
                up[2] = up[2] + 2.0;
                mask = G_ItemClipMask(ent);
                //if ( EntHandle::isDefined(&droppedEnt->r.ownerNum) )
                if ( droppedEnt->r.ownerNum.isDefined() )
                {
                    //passEntityNum = EntHandle::entnum(&droppedEnt->r.ownerNum);
                    passEntityNum = droppedEnt->r.ownerNum.entnum();
                    G_TraceCapsule(
                        &trace,
                        ent->r.currentOrigin,
                        droppedEnt->r.mins,
                        droppedEnt->r.maxs,
                        up,
                        passEntityNum,
                        mask,
                        &context);
                }
                else
                {
                    G_TraceCapsule(&trace, ent->r.currentOrigin, droppedEnt->r.mins, droppedEnt->r.maxs, up, 1023, mask, &context);
                }
                if ( trace.allsolid )
                {
                    G_SetOrigin(droppedEnt, ent->r.currentOrigin);
                }
                else
                {
                    Vec3Lerp(ent->r.currentOrigin, up, trace.fraction, up);
                    //if ( EntHandle::isDefined(&droppedEnt->r.ownerNum) )
                    if ( droppedEnt->r.ownerNum.isDefined() )
                    {
                        //v8 = EntHandle::entnum(&droppedEnt->r.ownerNum);
                        v8 = droppedEnt->r.ownerNum.entnum();
                        G_TraceCapsule(&trace, up, droppedEnt->r.mins, droppedEnt->r.maxs, ent->r.currentOrigin, v8, mask, &context);
                    }
                    else
                    {
                        G_TraceCapsule(
                            &trace,
                            up,
                            droppedEnt->r.mins,
                            droppedEnt->r.maxs,
                            ent->r.currentOrigin,
                            1023,
                            mask,
                            &context);
                    }
                    Vec3Lerp(up, ent->r.currentOrigin, trace.fraction, up);
                    G_SetOrigin(droppedEnt, up);
                }
                G_SetAngle(droppedEnt, ent->r.currentAngles);
            }
            SV_LinkEntity(droppedEnt);
        }
        *pDroppedWeapon = droppedEnt;
        G_GivePlayerWeapon(ps, weapon, weaponModel, ent->s.renderOptions);
        return 1;
    }
    else
    {
        v6 = va("%c \"GAME_CANT_GET_PRIMARY_WEAP_MESSAGE\"", 102);
        SV_GameSendServerCommand(other - g_entities, SV_CMD_CAN_IGNORE, v6);
        return 0;
    }
}

int __cdecl CurrentPrimaryWeapon(playerState_s *ps)
{
    int weapIdx; // [esp+Ch] [ebp-8h]
    const WeaponDef *weapDef; // [esp+10h] [ebp-4h]

    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp", 147, 0, "%s", "ps") )
        __debugbreak();
    if ( !ps->weapon )
        return 0;
    weapIdx = ps->weapon;
    weapDef = BG_GetWeaponDef(weapIdx);
    if ( weapDef->inventoryType == WEAPINVENTORY_ALTMODE )
    {
        weapIdx = ps->lastWeaponAltModeSwitch;
        weapDef = BG_GetWeaponDef(weapIdx);
    }
    if ( !BG_PlayerHasWeapon(ps, weapIdx) )
        return 0;
    if ( weapDef->inventoryType )
        return 0;
    return weapIdx;
}

int __cdecl G_ItemClipMask(gentity_s *ent)
{
    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp", 175, 0, "%s", "ent") )
        __debugbreak();
    if ( ent->clipmask )
        return ent->clipmask;
    else
        return 1169;
}

bool __cdecl WeaponPickup_LeechFromWeaponEnt(
                gentity_s *weaponEnt,
                gentity_s *player,
                int haveExactWeapon,
                int *pickupEvent,
                bool suppressNotifies)
{
    signed int v5; // ecx
    unsigned __int8 weaponModel; // [esp+3h] [ebp-1Dh]
    int ammoTaken; // [esp+4h] [ebp-1Ch]
    unsigned int weapIdx; // [esp+8h] [ebp-18h]
    int ammoAvailable; // [esp+10h] [ebp-10h]
    item_ent_t *itemEnt; // [esp+14h] [ebp-Ch]
    int i; // [esp+18h] [ebp-8h]
    bool ammoRemains; // [esp+1Dh] [ebp-3h]
    bool removedAnyAmmo; // [esp+1Eh] [ebp-2h]
    bool grenadeWeapon; // [esp+1Fh] [ebp-1h]

    if ( !weaponEnt
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp", 285, 0, "%s", "weaponEnt") )
    {
        __debugbreak();
    }
    if ( !player && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp", 286, 0, "%s", "player") )
        __debugbreak();
    removedAnyAmmo = 0;
    ammoRemains = 0;
    grenadeWeapon = 0;
    for ( i = 0; i < 2; ++i )
    {
        v5 = weaponEnt->item[i].index % 2048;
        weapIdx = v5;
        if ( v5 > 0 )
        {
            weaponModel = weaponEnt->item[i].index / 2048;
            itemEnt = &weaponEnt->item[i];
            if ( BG_GetWeaponDef(v5)->weapType == WEAPTYPE_GRENADE )
                grenadeWeapon = 1;
            ammoAvailable = itemEnt->ammoCount;
            if ( haveExactWeapon )
                ammoAvailable += weaponEnt->item[i].clipAmmoCount;
            ammoTaken = Add_Ammo(player, weapIdx, weaponModel, ammoAvailable, 0);
            if ( ammoTaken )
            {
                removedAnyAmmo = 1;
                if ( !suppressNotifies && pickupPrints->current.enabled )
                    PrintPlayerPickupMessage(player, weapIdx);
                itemEnt->ammoCount -= ammoTaken;
                if ( itemEnt->ammoCount < 0 )
                {
                    weaponEnt->item[i].clipAmmoCount += weaponEnt->item[i].ammoCount;
                    itemEnt->ammoCount = 0;
                    if ( weaponEnt->item[i].clipAmmoCount < 0 )
                        weaponEnt->item[i].clipAmmoCount = 0;
                }
            }
            if ( itemEnt->ammoCount > 0 || weaponEnt->item[i].clipAmmoCount > 0 )
                ammoRemains = 1;
        }
    }
    if ( !suppressNotifies && removedAnyAmmo )
    {
        if ( !pickupEvent
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp", 339, 0, "%s", "pickupEvent") )
        {
            __debugbreak();
        }
        *pickupEvent = 12;
    }
    if ( grenadeWeapon && !ammoRemains )
        return 1;
    return haveExactWeapon && removedAnyAmmo;
}

void __cdecl PrintPlayerPickupMessage(gentity_s *player, unsigned int weapIdx)
{
    const char *v2; // eax
    const WeaponVariantDef *weapVariantDef; // [esp+0h] [ebp-4h]

    if ( !player && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp", 269, 0, "%s", "player") )
        __debugbreak();
    weapVariantDef = BG_GetWeaponVariantDef(weapIdx);
    if ( !weapVariantDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp", 271, 0, "%s", "weapVariantDef") )
    {
        __debugbreak();
    }
    if ( BG_WeaponIsClipOnly(weapIdx) )
        v2 = va("%c GAME_PICKUP_CLIPONLY_AMMO %s", 102, weapVariantDef->szDisplayName);
    else
        v2 = va("%c GAME_PICKUP_AMMO %s", 102, weapVariantDef->szDisplayName);
    SV_GameSendServerCommand(player - g_entities, SV_CMD_CAN_IGNORE, v2);
}

void __cdecl WeaponPickup_AddAmmoForNewWeapon(gentity_s *weaponEnt, gentity_s *player)
{
    int i; // [esp+18h] [ebp-18h]
    unsigned __int8 weaponModel; // [esp+1Fh] [ebp-11h]
    const WeaponVariantDef *weapVarDef; // [esp+20h] [ebp-10h]
    signed int weapon; // [esp+28h] [ebp-8h]
    int clipAmmo; // [esp+2Ch] [ebp-4h]

    for ( i = 0; i < 2; ++i )
    {
        weapon = weaponEnt->item[i].index % 2048;
        weaponModel = weaponEnt->item[i].index / 2048;
        if ( weapon > 0 )
        {
            weapVarDef = BG_GetWeaponVariantDef(weapon);
            clipAmmo = weaponEnt->item[i].clipAmmoCount;
            if ( clipAmmo >= 0 )
            {
                if ( clipAmmo > BG_GetClipSize(weapon)
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp",
                                379,
                                0,
                                "%s",
                                "clipAmmo <= BG_GetClipSize( weapon )") )
                {
                    __debugbreak();
                }
                BG_SetAmmoInClip(&player->client->ps, weapVarDef->iClipIndex, clipAmmo);
            }
            Add_Ammo(player, weapon, weaponModel, weaponEnt->item[i].ammoCount, clipAmmo == -1);
        }
    }
}

void __cdecl WeaponPickup_Notifies(
                gentity_s *thisItem,
                gentity_s *newDroppedItem,
                gentity_s *player,
                const WeaponDef *weapDef)
{
    if ( !thisItem && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp", 392, 0, "%s", "thisItem") )
        __debugbreak();
    if ( !player && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp", 393, 0, "%s", "player") )
        __debugbreak();
    if ( !weapDef && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp", 394, 0, "%s", "weapDef") )
        __debugbreak();
    if ( newDroppedItem )
        Scr_AddEntity(newDroppedItem, SCRIPTINSTANCE_SERVER);
    else
        Scr_AddUndefined(SCRIPTINSTANCE_SERVER);
    Scr_AddEntity(player, SCRIPTINSTANCE_SERVER);
    Scr_Notify(thisItem, scr_const.trigger, 2u);
}

bool __cdecl WeaponPickup_Touch(gentity_s *weaponEnt, gentity_s *player, unsigned int weapIdx, int *pickupEvent)
{
    bool haveExactWeapon; // [esp+24h] [ebp-Ch]
    bool removeWeaponFromWorld; // [esp+2Bh] [ebp-5h]
    const WeaponDef *weapDef; // [esp+2Ch] [ebp-4h]

    if ( !weaponEnt
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp", 474, 0, "%s", "weaponEnt") )
    {
        __debugbreak();
    }
    if ( !player && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp", 475, 0, "%s", "player") )
        __debugbreak();
    if ( !player->client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp", 476, 0, "%s", "player->client") )
    {
        __debugbreak();
    }
    if ( !pickupEvent
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp", 477, 0, "%s", "pickupEvent") )
    {
        __debugbreak();
    }
    if ( (0x800000 & weaponEnt->s.lerp.eFlags2) != 0 && (player->client->ps.perks[1] & 0x200) != 0 )
        return 1;
    weapDef = BG_GetWeaponDef(weapIdx);
    haveExactWeapon = BG_PlayerHasWeapon(&player->client->ps, weapIdx);
    if ( !haveExactWeapon
        && !BG_PlayerHasCompatibleWeapon(&player->client->ps, weapIdx)
        && weapDef->offhandClass != OFFHAND_CLASS_FRAG_GRENADE )
    {
        return 0;
    }
    removeWeaponFromWorld = WeaponPickup_LeechFromWeaponEnt(weaponEnt, player, haveExactWeapon, pickupEvent, 0);
    if ( *pickupEvent )
        WeaponPickup_Notifies(weaponEnt, 0, player, weapDef);
    return removeWeaponFromWorld;
}

void __cdecl PrintMessage_CannotGrabItem(
                gentity_s *ent,
                gentity_s *player,
                int touched,
                gitem_s *item,
                unsigned int weapIndex)
{
    const WeaponVariantDef *WeaponVariantDef; // eax
    const char *v6; // eax

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp", 539, 0, "%s", "ent") )
        __debugbreak();
    if ( !player && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp", 540, 0, "%s", "player") )
        __debugbreak();
    if ( !player->client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp", 541, 0, "%s", "player->client") )
    {
        __debugbreak();
    }
    if ( !item && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp", 542, 0, "%s", "item") )
        __debugbreak();
    if ( !touched && ent->s.clientNum != player->s.number )
    {
        if ( item->giType != IT_WEAPON
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp",
                        549,
                        0,
                        "%s",
                        "item->giType == IT_WEAPON") )
        {
            __debugbreak();
        }
        if ( BG_PlayerHasWeapon(&player->client->ps, weapIndex) )
        {
            WeaponVariantDef = BG_GetWeaponVariantDef(weapIndex);
            v6 = va("%c GAME_PICKUP_CANTCARRYMOREAMMO %s", 102, WeaponVariantDef->szDisplayName);
        }
        else
        {
            v6 = va("%c \"GAME_CANT_GET_PRIMARY_WEAP_MESSAGE\"", 102);
        }
        SV_GameSendServerCommand(player - g_entities, SV_CMD_CAN_IGNORE, v6);
    }
}

void __cdecl DroppedItemClearOwner(gentity_s *pSelf)
{
    pSelf->s.clientNum = 32;
}

void __cdecl G_GetItemClassname(const gitem_s *item, unsigned __int16 *out)
{
    const char *v2; // eax
    char classname[260]; // [esp+4h] [ebp-108h] BYREF

    v2 = BG_WeaponName((((char *)item - (char *)bg_itemlist) >> 2) % 2048);
    Com_sprintf(classname, 0x100u, "weapon_%s", v2);
    G_SetConstString(out, classname);
}

gentity_s *__cdecl Drop_Item(gentity_s *ent, const gitem_s *item, float angle, int novelocity)
{
    float value; // [esp+4h] [ebp-2Ch]
    float velocity[3]; // [esp+Ch] [ebp-24h] BYREF
    float angles[3]; // [esp+18h] [ebp-18h] BYREF
    float vPos[3]; // [esp+24h] [ebp-Ch] BYREF

    angles[0] = ent->r.currentAngles[0];
    angles[1] = ent->r.currentAngles[1] + angle;
    angles[0] = 0.0f;
    angles[2] = 0.0f;
    if ( novelocity )
    {
        memset(velocity, 0, sizeof(velocity));
    }
    else
    {
        AngleVectors(angles, velocity, 0, 0);
        value = g_dropForwardSpeed->current.value;
        velocity[0] = value * velocity[0];
        velocity[1] = value * velocity[1];
        velocity[2] = value * velocity[2];
        velocity[2] = G_crandom() * g_dropUpSpeedRand->current.value + g_dropUpSpeedBase->current.value + velocity[2];
        velocity[0] = G_crandom() * g_dropHorzSpeedRand->current.value + velocity[0];
        velocity[1] = G_crandom() * g_dropHorzSpeedRand->current.value + velocity[1];
    }
    vPos[0] = ent->r.currentOrigin[0];
    vPos[1] = ent->r.currentOrigin[1];
    vPos[2] = ent->r.currentOrigin[2];
    vPos[2] = (float)((float)(ent->r.maxs[2] - ent->r.mins[2]) * 0.5) + vPos[2];
    return LaunchItem(item, vPos, angles, velocity, ent->s.number);
}

gentity_s *__cdecl LaunchItem(const gitem_s *item, float *origin, float *angles, float *velocity, int ownerNum)
{
    char *Name; // eax
    float *trDelta; // [esp+Ch] [ebp-4Ch]
    float *maxs; // [esp+10h] [ebp-48h]
    float *mins; // [esp+14h] [ebp-44h]
    unsigned __int8 boneIndex; // [esp+33h] [ebp-25h] BYREF
    DObj *dobj; // [esp+34h] [ebp-24h]
    int tagIndex; // [esp+38h] [ebp-20h]
    gentity_s *dropped; // [esp+3Ch] [ebp-1Ch]
    int dropIdx; // [esp+40h] [ebp-18h]
    const WeaponVariantDef *weapVariantDef; // [esp+44h] [ebp-14h]
    int itemIndex; // [esp+48h] [ebp-10h]
    int weapIndex; // [esp+4Ch] [ebp-Ch]
    unsigned __int8 weapModel; // [esp+53h] [ebp-5h]
    const WeaponDef *weapDef; // [esp+54h] [ebp-4h]
    int savedregs; // [esp+58h] [ebp+0h] BYREF

    if ( !item && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp", 733, 0, "%s", "item") )
        __debugbreak();

    nanassertvec3(origin);
    nanassertvec3(velocity);

    itemIndex = ((char *)item - (char *)bg_itemlist) >> 2;
    dropped = G_Spawn();
    dropIdx = GetFreeDropCueIdx();
    //EntHandle::setEnt((EntHandle *)(4 * dropIdx + 65348584), dropped);
    //EntHandle::setEnt(&level.droppedWeaponCue[dropIdx], dropped);
    level.droppedWeaponCue[dropIdx].setEnt(dropped);
    dropped->s.eType = ET_ITEM;
    dropped->s.un3.item = (unsigned __int16)itemIndex;
    if ( dropped->s.un3.item >= 0x800u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp",
                    746,
                    0,
                    "dropped->s.un3.item doesn't index MAX_ITEMS\n\t%i not in [0, %i)",
                    dropped->s.un3.item,
                    2048) )
    {
        __debugbreak();
    }
    G_GetItemClassname(item, &dropped->classname);
    dropped->item[0].index = itemIndex;
    if ( item->giType != IT_WEAPON
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp",
                    751,
                    0,
                    "%s",
                    "item->giType == IT_WEAPON") )
    {
        __debugbreak();
    }
    mins = dropped->r.mins;
    dropped->r.mins[0] = -1.0f;
    mins[1] = -1.0f;
    mins[2] = -1.0f;
    maxs = dropped->r.maxs;
    dropped->r.maxs[0] = 1.0f;
    maxs[1] = 1.0f;
    maxs[2] = 1.0f;
    dropped->r.contents = 1079771400;
    dropped->r.contents |= 0x200000u;
    if ( ownerNum < level.maxclients )
        AssignToSmallerType<unsigned char>(&dropped->s.clientNum, ownerNum);
    weapIndex = itemIndex % 2048;
    if ( itemIndex % 2048 >= (int)BG_GetNumWeapons() )
        Com_Error(ERR_DROP, "LaunchItem() - Bad item WEAPIDX %i on entity", weapIndex);
    weapModel = itemIndex / 2048;
    weapDef = BG_GetWeaponDef(weapIndex);
    if ( !weapDef && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp", 776, 0, "%s", "weapDef") )
        __debugbreak();
    if ( weapDef->bUseDroppedModelAsStowed
        && *((unsigned int *)weapDef->worldModel + 1)
        && weapDef->weapClass != WEAPCLASS_GRENADE
        && weapDef->weapClass != WEAPCLASS_KILLSTREAK_ALT_STORED_WEAPON )
    {
        weapModel = 1;
    }
    if ( weapDef->worldModel[weapModel] )
    {
        Name = (char *)XModelGetName(weapDef->worldModel[weapModel]);
        G_SetModel(dropped, Name);
    }
    G_DObjUpdate(dropped);
    if ( weapDef->worldModel[weapModel] )
    {
        dobj = Com_GetServerDObj(dropped->s.number);
        weapVariantDef = BG_GetWeaponVariantDef(weapIndex);
        if ( !weapVariantDef
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp", 794, 0, "%s", "weapVariantDef") )
        {
            __debugbreak();
        }
        for ( tagIndex = 0; tagIndex < 32 && weapVariantDef->hideTags[tagIndex]; ++tagIndex )
        {
            boneIndex = -2;
            if ( DObjGetBoneIndex(dobj, weapVariantDef->hideTags[tagIndex], &boneIndex, -1) )
                dropped->s.partBits[(int)boneIndex >> 5] |= 0x80000000 >> (boneIndex & 0x1F);
        }
        DObjSetHidePartBits(dobj, dropped->s.partBits);
    }
    dropped->handler = 19;
    G_SetOrigin(dropped, origin);
    dropped->s.lerp.pos.trType = 6;
    G_SetAngle(dropped, angles);
    dropped->s.lerp.pos.trTime = level.time;
    trDelta = dropped->s.lerp.pos.trDelta;
    dropped->s.lerp.pos.trDelta[0] = *velocity;
    trDelta[1] = velocity[1];
    trDelta[2] = velocity[2];
    if ( ((LODWORD(dropped->s.lerp.pos.trDelta[0]) & 0x7F800000) == 0x7F800000
         || (LODWORD(dropped->s.lerp.pos.trDelta[1]) & 0x7F800000) == 0x7F800000
         || (LODWORD(dropped->s.lerp.pos.trDelta[2]) & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp",
                    860,
                    0,
                    "%s",
                    "!IS_NAN((dropped->s.lerp.pos.trDelta)[0]) && !IS_NAN((dropped->s.lerp.pos.trDelta)[1]) && !IS_NAN((dropped->s."
                    "lerp.pos.trDelta)[2])") )
    {
        __debugbreak();
    }
    dropped->flags = 16;
    dropped->nextthink = level.time + 1000;
    SV_LinkEntity(dropped);
    return dropped;
}

int __cdecl GetFreeDropCueIdx()
{
    int iBest; // [esp+14h] [ebp-24h]
    int j; // [esp+18h] [ebp-20h]
    int maxDroppedWeapon; // [esp+1Ch] [ebp-1Ch]
    float fDistSqrd; // [esp+20h] [ebp-18h]
    float fBestDistSqrd; // [esp+24h] [ebp-14h]
    float fClientDistSqrd; // [esp+2Ch] [ebp-Ch]
    gentity_s *ent; // [esp+30h] [ebp-8h]
    gentity_s *enta; // [esp+30h] [ebp-8h]
    int i; // [esp+34h] [ebp-4h]

    maxDroppedWeapon = g_maxDroppedWeapons->current.integer;
    if ( (maxDroppedWeapon < 1 || maxDroppedWeapon > 32)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp",
                    643,
                    0,
                    "%s\n\t(maxDroppedWeapon) = %i",
                    "(maxDroppedWeapon >= 1 && maxDroppedWeapon <= 32)",
                    maxDroppedWeapon) )
    {
        __debugbreak();
    }
    iBest = -1;
    fBestDistSqrd = -1.0f;
    for ( i = 0; i < maxDroppedWeapon; ++i )
    {
        //if ( !EntHandle::isDefined((EntHandle *)(4 * i + 65348584)) )
        if (!level.droppedWeaponCue[i].isDefined())
            return i;
        //ent = EntHandle::ent(&level.droppedWeaponCue[i]);
        ent = level.droppedWeaponCue[i].ent();
        if ((ent->flags & 0x2000000) == 0)
        {
            //if ( bg_itemlist[ent->s.un3.item] != 1
            //    && !Assert_MyHandler(
            //                "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp",
            //                659,
            //                0,
            //                "%s",
            //                "bg_itemlist[ ent->s.un3.item ].giType == IT_WEAPON") )
            //{
            //    __debugbreak();
            //}
            if ( !BG_GetWeaponDef(ent->item[0].index % 2048)->avoidDropCleanup )
            {
                fDistSqrd = FLT_MAX;
                for ( j = 0; j < level.maxclients; ++j )
                {
                    if ( level.clients[j].sess.connected == CON_CONNECTED
                        && level.clients[j].sess.sessionState == SESS_STATE_PLAYING )
                    {
                        fClientDistSqrd = Vec3DistanceSq(g_entities[j].r.currentOrigin, ent->r.currentOrigin);
                        if ( fDistSqrd > fClientDistSqrd )
                            fDistSqrd = fClientDistSqrd;
                    }
                }
                if ( fDistSqrd > fBestDistSqrd )
                {
                    fBestDistSqrd = fDistSqrd;
                    iBest = i;
                }
            }
        }
    }
    if ( iBest == -1 )
    {
        Com_PrintWarning(
            16,
            "Could not find a suitable weapon entity to free out of %i possible.    Using index zero.\n",
            maxDroppedWeapon);
        iBest = 0;
    }
    //enta = EntHandle::ent(&level.droppedWeaponCue[iBest]);
    enta = level.droppedWeaponCue[iBest].ent();
    if (!enta && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp", 693, 0, "%s", "ent"))
        __debugbreak();
    G_FreeEntity(enta);
    //EntHandle::setEnt(&level.droppedWeaponCue[iBest], 0);
    level.droppedWeaponCue[iBest].setEnt(0);
    return iBest;
}

gentity_s *__cdecl ThrowClientWeapon(gentity_s *ent, const gitem_s *item, float heightIncrease, float velocityIncrease)
{
    float v5; // [esp+Ch] [ebp-30h]
    float velocity[3]; // [esp+10h] [ebp-2Ch] BYREF
    float sine; // [esp+1Ch] [ebp-20h]
    float cosine; // [esp+20h] [ebp-1Ch]
    float angles[3]; // [esp+24h] [ebp-18h] BYREF
    float vPos[3]; // [esp+30h] [ebp-Ch] BYREF

    angles[0] = 0.0f;
    angles[2] = 0.0f;
    if ( ent->client )
    {
        angles[1] = ent->client->fGunYaw;
        AngleVectors(angles, velocity, 0, 0);
        v5 = ent->client->fGunYaw * 0.017453292;
        cosine = cos(v5);
        sine = sin(v5);
        velocity[0] = (float)(velocityIncrease * cosine) + velocity[0];
        velocity[1] = (float)(velocityIncrease * sine) + velocity[1];
        velocity[2] = velocity[2] + heightIncrease;
    }
    else
    {
        angles[1] = 0.0f;
        memset(velocity, 0, sizeof(velocity));
    }
    vPos[0] = ent->r.currentOrigin[0];
    vPos[1] = ent->r.currentOrigin[1];
    vPos[2] = ent->r.currentOrigin[2];
    vPos[2] = (float)((float)(ent->r.maxs[2] - ent->r.mins[2]) * 0.5) + vPos[2];
    return LaunchItem(item, vPos, angles, velocity, ent->s.number);
}

bool __cdecl PlayerHasAnyAmmoToTransferToWeapon(gentity_s *player, unsigned int transferWeapon)
{
    if ( !player->client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp", 962, 0, "%s", "player->client") )
    {
        __debugbreak();
    }
    return BG_GetAmmoInClip(&player->client->ps, transferWeapon) > 0
            || GetNonClipAmmoToTransferToWeaponEntity(player, transferWeapon) > 0;
}

int __cdecl GetNonClipAmmoToTransferToWeaponEntity(gentity_s *player, unsigned int transferWeapon)
{
    int maxAmmoAfterDrop; // [esp+8h] [ebp-8h]
    int ammoDrop; // [esp+Ch] [ebp-4h]

    BG_AmmoForWeapon(transferWeapon);
    maxAmmoAfterDrop = BG_GetAmmoPlayerMax(&player->client->ps, transferWeapon, transferWeapon);
    ammoDrop = BG_GetAmmoNotInClip(&player->client->ps, transferWeapon) - maxAmmoAfterDrop;
    if ( ammoDrop < 0 )
        return 0;
    return ammoDrop;
}

gentity_s *__cdecl ThrowDownWeapon(
                gentity_s *ent,
                int weapIdx,
                unsigned __int8 weaponModel,
                unsigned int tag,
                float throwHeight,
                float throwDistance)
{
    const char *v6; // eax
    int v9; // eax
    float *currentOrigin; // [esp+8h] [ebp-C8h]
    float *trBase; // [esp+Ch] [ebp-C4h]
    col_context_t context; // [esp+24h] [ebp-ACh] BYREF
    float vCenter[3]; // [esp+88h] [ebp-48h] BYREF
    float tagMat[4][3]; // [esp+94h] [ebp-3Ch] BYREF
    unsigned int altWeapIdx; // [esp+C4h] [ebp-Ch]
    gentity_s *dropEnt; // [esp+C8h] [ebp-8h]
    const gitem_s *weapItem; // [esp+CCh] [ebp-4h]

    if ( BG_GetWeaponDef(weapIdx)->inventoryType == WEAPINVENTORY_ALTMODE )
    {
        altWeapIdx = BG_GetWeaponVariantDef(weapIdx)->altWeaponIndex;
        if ( !altWeapIdx )
        {
            v6 = BG_WeaponName(weapIdx);
            Com_PrintError(
                15,
                "Drop_Weapon(): Trying to drop alt-type weapon, \"%s\", but it has no corresponding altWeapon set.\n",
                v6);
            return 0;
        }
        weapIdx = altWeapIdx;
    }

    weapItem = (const gitem_s *)&bg_itemlist[2048 * weaponModel + weapIdx];
    iassert(weapItem->giType == IT_WEAPON);
    
    if ( (!ent->client || BG_PlayerHasWeapon(&ent->client->ps, weapIdx)
                                         && PlayerHasAnyAmmoToTransferToWeapon(ent, weapIdx))
        && (!ent->client || !BG_GetWeaponDef(weapIdx)->bClipOnly || BG_GetAmmoInClip(&ent->client->ps, weapIdx)) )
    {
        if ( throwHeight == 0.0 && throwDistance == 0.0 )
            dropEnt = Drop_Item(ent, weapItem, 0.0, 0);
        else
            dropEnt = ThrowClientWeapon(ent, weapItem, throwHeight, throwDistance);
        //renderOptions_s::CopyWeaponOptions(&dropEnt->s.renderOptions, &ent->s.renderOptions);
        dropEnt->s.renderOptions.CopyWeaponOptions(&ent->s.renderOptions);
        if ( ent->client )
        {
            TransferPlayerAmmoToWeaponEntity(ent, dropEnt, weapIdx);
            BG_TakePlayerWeapon(&ent->client->ps, weapIdx);
        }
        else
        {
            TransferRandomAmmoToWeaponEntity(dropEnt, weapIdx);
        }
        if ( tag )
        {
            if ( G_DObjGetWorldTagMatrix(ent, tag, tagMat) )
            {
                trace_t trace; // [esp+4Ch] [ebp-84h] BYREF
                vCenter[0] = (float)(ent->r.mins[0] + ent->r.maxs[0]) * 0.5;
                vCenter[1] = (float)(ent->r.mins[1] + ent->r.maxs[1]) * 0.5;
                vCenter[2] = (float)(ent->r.mins[2] + ent->r.maxs[2]) * 0.5;
                vCenter[0] = vCenter[0] + ent->r.currentOrigin[0];
                vCenter[1] = vCenter[1] + ent->r.currentOrigin[1];
                vCenter[2] = vCenter[2] + ent->r.currentOrigin[2];
                //col_context_t::col_context_t(&context);
                v9 = G_ItemClipMask(dropEnt);
                G_TraceCapsule(&trace, vCenter, dropEnt->r.mins, dropEnt->r.maxs, tagMat[3], ent->s.number, v9, &context);
                Vec3Lerp(vCenter, tagMat[3], trace.fraction, dropEnt->s.lerp.pos.trBase);
                currentOrigin = dropEnt->r.currentOrigin;
                trBase = dropEnt->s.lerp.pos.trBase;
                dropEnt->r.currentOrigin[0] = dropEnt->s.lerp.pos.trBase[0];
                currentOrigin[1] = trBase[1];
                currentOrigin[2] = trBase[2];
                dropEnt->s.lerp.pos.trTime = level.time;
            }
            if ( dropEnt->s.lerp.apos.trType != 10 )
                G_SetAngle(dropEnt, ent->r.currentAngles);
        }
        return dropEnt;
    }
    else
    {
        BG_TakePlayerWeapon(&ent->client->ps, weapIdx);
        return 0;
    }
}

int __cdecl TransferPlayerAmmoToWeaponEntity(gentity_s *player, gentity_s *weaponEnt, int transferWeapon)
{
    int NonClipAmmoToTransferToWeaponEntity; // eax
    int i; // [esp+Ch] [ebp-Ch]
    int total; // [esp+10h] [ebp-8h]

    total = 0;
    for ( i = 0; i < 2; ++i )
    {
        if ( transferWeapon > 0 )
        {
            NonClipAmmoToTransferToWeaponEntity = GetNonClipAmmoToTransferToWeaponEntity(player, transferWeapon);
            weaponEnt->item[i].index = transferWeapon;
            weaponEnt->item[i].ammoCount = NonClipAmmoToTransferToWeaponEntity;
            weaponEnt->item[i].clipAmmoCount = BG_GetAmmoInClip(&player->client->ps, transferWeapon);
            total += weaponEnt->item[i].clipAmmoCount + weaponEnt->item[i].ammoCount;
            if ( BG_GetWeaponDef(transferWeapon)->bDualWield )
                transferWeapon = BG_GetWeaponDef(transferWeapon)->dualWieldWeaponIndex;
            else
                transferWeapon = BG_GetWeaponVariantDef(transferWeapon)->altWeaponIndex;
        }
    }
    return total;
}

int __cdecl TransferRandomAmmoToWeaponEntity(gentity_s *weaponEnt, int transferWeapon)
{
    int v3; // [esp+0h] [ebp-B0h]
    int v4; // [esp+4h] [ebp-ACh]
    int iDropClipAmmoMax; // [esp+Ch] [ebp-A4h]
    int iDropClipAmmoMin; // [esp+14h] [ebp-9Ch]
    int clipSize; // [esp+84h] [ebp-2Ch]
    int i; // [esp+88h] [ebp-28h]
    int iDropAmmo; // [esp+90h] [ebp-20h]
    int iDropAmmoa; // [esp+90h] [ebp-20h]
    int iMax; // [esp+98h] [ebp-18h]
    int total; // [esp+9Ch] [ebp-14h]
    int iMin; // [esp+A0h] [ebp-10h]
    int iDropClip; // [esp+A8h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+ACh] [ebp-4h]

    total = 0;
    for ( i = 0; i < 2; ++i )
    {
        if ( transferWeapon > 0 )
        {
            weapDef = BG_GetWeaponDef(transferWeapon);
            if ( weapDef->iSharedAmmoCapIndex >= 0 && i )
                return total;
            iMax = BG_GetDropMaxAmmo(transferWeapon);
            iMin = BG_GetDropMinAmmo(transferWeapon);
            if ( iMax < iMin )
            {
                iMax = iMin;
                iMin = BG_GetDropMaxAmmo(transferWeapon);
            }
            if ( iMax >= 0 )
            {
                if ( iMax < iMin
                    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp", 1050, 0, "%s", "iMax >= iMin") )
                {
                    __debugbreak();
                }
                iDropAmmoa = iMin + G_rand() % (iMax - iMin + 1);
                if ( iDropAmmoa > 0 )
                {
                    clipSize = BG_GetClipSize(transferWeapon);
                    if ( clipSize < 0
                        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp", 1061, 0, "%s", "clipSize >= 0") )
                    {
                        __debugbreak();
                    }
                    if ( clipSize == 1 )
                    {
                        iDropClip = 1;
                    }
                    else
                    {
                        if ( weapDef->iDropClipAmmoMin < clipSize )
                            iDropClipAmmoMin = weapDef->iDropClipAmmoMin;
                        else
                            iDropClipAmmoMin = clipSize;
                        if ( iDropClipAmmoMin > 0 )
                            v4 = iDropClipAmmoMin;
                        else
                            v4 = 0;
                        if ( weapDef->iDropClipAmmoMax < clipSize )
                            iDropClipAmmoMax = weapDef->iDropClipAmmoMax;
                        else
                            iDropClipAmmoMax = clipSize;
                        if ( iDropClipAmmoMax > 0 )
                            v3 = iDropClipAmmoMax;
                        else
                            v3 = 0;
                        iDropClip = v4 + G_rand() % (v3 - v4 + 1);
                    }
                    if ( iDropClip < iDropAmmoa )
                    {
                        iDropAmmo = iDropAmmoa - iDropClip;
                    }
                    else
                    {
                        iDropClip = iDropAmmoa;
                        iDropAmmo = 0;
                    }
                }
                else
                {
                    iDropAmmo = 0;
                    iDropClip = 0;
                }
            }
            else
            {
                iDropAmmo = 0;
                iDropClip = 0;
            }
            weaponEnt->item[i].index = transferWeapon;
            weaponEnt->item[i].ammoCount = iDropAmmo;
            weaponEnt->item[i].clipAmmoCount = iDropClip;
            if ( BG_GetWeaponDef(transferWeapon)->bDualWield )
                transferWeapon = BG_GetWeaponDef(transferWeapon)->dualWieldWeaponIndex;
            else
                transferWeapon = BG_GetWeaponVariantDef(transferWeapon)->altWeaponIndex;
            total += weaponEnt->item[i].clipAmmoCount + weaponEnt->item[i].ammoCount;
        }
    }
    return total;
}

int __cdecl BG_GetDropMinAmmo(unsigned int weaponIndex)
{
    int v2; // esi
    const WeaponDef *weapDef; // [esp+1Ch] [ebp-4h]

    weapDef = BG_GetWeaponDef(weaponIndex);
    if ( !weapDef->ammoCountClipRelative )
        return weapDef->iDropAmmoMin;
    v2 = weapDef->iDropAmmoMin - 1;
    return BG_GetClipSize(weaponIndex) * v2 + 1;
}

int __cdecl BG_GetDropMaxAmmo(unsigned int weaponIndex)
{
    const WeaponDef *weapDef; // [esp+18h] [ebp-4h]

    weapDef = BG_GetWeaponDef(weaponIndex);
    if ( weapDef->ammoCountClipRelative )
        return weapDef->iDropAmmoMax * BG_GetClipSize(weaponIndex);
    else
        return weapDef->iDropAmmoMax;
}

gentity_s *__cdecl Drop_Weapon(gentity_s *ent, int weapIdx, unsigned __int8 weaponModel, unsigned int tag)
{
    return ThrowDownWeapon(ent, weapIdx, weaponModel, tag, 0.0, 0.0);
}

void __cdecl FinishSpawningItem(gentity_s *ent)
{
    char *v2; // eax
    const char *v3; // [esp+4h] [ebp-100h]
    float v4; // [esp+10h] [ebp-F4h]
    float v5; // [esp+14h] [ebp-F0h]
    float v6; // [esp+20h] [ebp-E4h]
    float v7; // [esp+24h] [ebp-E0h]
    float vAngles[3]; // [esp+2Ch] [ebp-D8h] BYREF
    float vAxis[3][3]; // [esp+38h] [ebp-CCh] BYREF
    gitem_s *item; // [esp+5Ch] [ebp-A8h]
    col_context_t context; // [esp+60h] [ebp-A4h] BYREF
    float mins[3]; // [esp+88h] [ebp-7Ch] BYREF
    float start[3]; // [esp+94h] [ebp-70h] BYREF
    float maxs[3]; // [esp+A0h] [ebp-64h] BYREF
    float endpos[3]; // [esp+ACh] [ebp-58h] BYREF
    trace_t tr; // [esp+B8h] [ebp-4Ch] BYREF
    float dest[3]; // [esp+F4h] [ebp-10h] BYREF
    int clipMask; // [esp+100h] [ebp-4h]
    int savedregs; // [esp+104h] [ebp+0h] BYREF

    //col_context_t::col_context_t(&context);
    ent->handler = 21;
    if ((ent->spawnflags & 1) != 0)
    {
        G_SetOrigin(ent, ent->r.currentOrigin);
    }
    else
    {
        item = (gitem_s *)&bg_itemlist[ent->s.un3.item];
        if (item->giType != IT_WEAPON
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp",
                1317,
                0,
                "%s",
                "item->giType == IT_WEAPON"))
        {
            __debugbreak();
        }
        mins[0] = -1.0f;
        mins[1] = -1.0f;
        mins[2] = -1.0f;
        maxs[0] = 1.0f;
        maxs[1] = 1.0f;
        maxs[2] = 1.0f;
        clipMask = G_ItemClipMask(ent);
        start[0] = ent->r.currentOrigin[0];
        start[1] = ent->r.currentOrigin[1];
        start[2] = ent->r.currentOrigin[2];
        v6 = ent->r.currentOrigin[1];
        v7 = ent->r.currentOrigin[2] - 4096.0;
        dest[0] = ent->r.currentOrigin[0];
        dest[1] = v6;
        dest[2] = v7;
        G_TraceCapsule(&tr, start, mins, maxs, dest, ent->s.number, clipMask, &context);
        if (tr.startsolid)
        {
            start[0] = ent->r.currentOrigin[0];
            start[1] = ent->r.currentOrigin[1];
            start[2] = ent->r.currentOrigin[2];
            start[2] = start[2] - 15.0;
            v4 = ent->r.currentOrigin[1];
            v5 = ent->r.currentOrigin[2] - 4096.0;
            dest[0] = ent->r.currentOrigin[0];
            dest[1] = v4;
            dest[2] = v5;
            G_TraceCapsule(&tr, start, mins, maxs, dest, ent->s.number, clipMask, &context);
        }
        if (tr.startsolid)
        {
            v3 = vtos(ent->r.currentOrigin);
            v2 = SL_ConvertToString(ent->classname, SCRIPTINSTANCE_SERVER);
            Com_Printf(15, "FinishSpawningItem: %s startsolid at %s\n", v2, v3);
            G_FreeEntity(ent);
            return;
        }
        ent->s.groundEntityNum = Trace_GetEntityHitId(&tr);
        g_entities[ent->s.groundEntityNum].flags |= 0x100000u;
        Vec3Lerp(start, dest, tr.fraction, endpos);
        G_SetOrigin(ent, endpos);
        if (tr.fraction < 1.0)
        {
            *(_QWORD *)&vAxis[2][0] = *(_QWORD *)tr.normal.vec.v;
            LODWORD(vAxis[2][2]) = tr.normal.vec.u[2];
            AngleVectors(ent->r.currentAngles, vAxis[0], 0, 0);
            Vec3Cross(vAxis[2], vAxis[0], vAxis[1]);
            Vec3Cross(vAxis[1], vAxis[2], vAxis[0]);
            AxisToAngles(vAxis, vAngles);
            //if (bg_itemlist[ent->s.un3.item] != 1
            //    && !Assert_MyHandler(
            //        "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp",
            //        1362,
            //        0,
            //        "%s",
            //        "bg_itemlist[ent->s.un3.item].giType == IT_WEAPON"))
            //{
            //    __debugbreak();
            //}
            vAngles[2] = vAngles[2] + 90.0;
            G_SetAngle(ent, vAngles);
        }
    }
    SV_LinkEntity(ent);
}

void __cdecl ClearRegisteredItems()
{
    memset(itemRegistered, 0, sizeof(itemRegistered));
    itemRegistered[0] = 1;
}

void __cdecl SaveRegisteredWeapons()
{
    char *v0; // eax
    int lastWrittenIndex; // [esp+0h] [ebp-418h]
    int lastWrittenVariantCount; // [esp+4h] [ebp-414h]
    const WeaponVariantDef *weapVariantDef; // [esp+40Ch] [ebp-Ch]
    unsigned int weapIndex; // [esp+410h] [ebp-8h]
    int count; // [esp+414h] [ebp-4h]

    count = 0;
    lastWrittenIndex = 0;
    lastWrittenVariantCount = 0;
    level.registerWeapons = 0;
    for ( weapIndex = 1; weapIndex < BG_GetNumWeapons(); ++weapIndex )
    {
        weapVariantDef = BG_GetWeaponVariantDef(weapIndex);
        if ( weapVariantDef->iVariantCount >= 0 )
        {
            if ( count >= 256 )
            {
                Com_Error(ERR_DROP, "SaveRegisteredWeapons: Too many weapon configstrings.    Max is %i.\n", 256);
                return;
            }
            if ( weapIndex != lastWrittenIndex + lastWrittenVariantCount + 1
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp",
                            1412,
                            0,
                            "%s",
                            "static_cast<int>(weapIndex) == (lastWrittenIndex + lastWrittenVariantCount + 1 )") )
            {
                __debugbreak();
            }
            lastWrittenIndex = weapIndex;
            lastWrittenVariantCount = weapVariantDef->iVariantCount;
            if ( lastWrittenVariantCount )
                v0 = va("%i %s %i", weapIndex, weapVariantDef->szInternalName, weapVariantDef->iVariantCount);
            else
                v0 = va("%i %s", weapIndex, weapVariantDef->szInternalName);
            SV_SetConfigstring(count + 2836, v0);
            ++count;
        }
    }
    CG_SetupWeaponDef();
}

void __cdecl SaveRegisteredItems()
{
    char string[2048]; // [esp+0h] [ebp-818h] BYREF
    int bits; // [esp+804h] [ebp-14h]
    int weapIdx; // [esp+808h] [ebp-10h]
    int n; // [esp+80Ch] [ebp-Ch]
    int digit; // [esp+810h] [ebp-8h]
    int last_non_zero_char; // [esp+814h] [ebp-4h]

    level.bRegisterItems = 0;
    last_non_zero_char = 0;
    n = 0;
    digit = 0;
    bits = 0;
    for ( weapIdx = 0; weapIdx < 2048; ++weapIdx )
    {
        if ( itemRegistered[weapIdx] )
            digit += 1 << bits;
        if ( ++bits == 4 )
        {
            string[n++] = digit + (digit >= 10 ? 87 : 48);
            if ( digit )
                last_non_zero_char = n;
            digit = 0;
            bits = 0;
        }
    }
    if ( bits )
    {
        string[n++] = digit + (digit >= 10 ? 87 : 48);
        if ( digit )
            last_non_zero_char = n;
    }
    string[last_non_zero_char] = 0;
    SV_SetConfigstring(3147, string);
}

void __cdecl G_RegisterWeapon(unsigned int weapIndex)
{
    char *Name; // eax
    char *v2; // eax
    char *v3; // eax
    char *v4; // eax
    unsigned int modelindex; // [esp+0h] [ebp-Ch]
    WeaponDef *weapDef; // [esp+8h] [ebp-4h]

    if ( itemRegistered[weapIndex]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp",
                    1530,
                    0,
                    "%s",
                    "!itemRegistered[weapIndex]") )
    {
        __debugbreak();
    }
    itemRegistered[weapIndex] = 1;
    level.bRegisterItems = 1;
    level.registerWeapons = 1;
    weapDef = bg_weaponVariantDefs[weapIndex]->weapDef;
    if ( *weapDef->szUseHintString
        && !G_GetHintStringIndex(&weapDef->iUseHintStringIndex, (char *)weapDef->szUseHintString) )
    {
        Com_Error(ERR_DROP, "Too many different hintstring values on weapons. Max allowed is %i different strings.", 96);
    }
    if ( *weapDef->dropHintString && !G_GetHintStringIndex(&weapDef->dropHintStringIndex, (char *)weapDef->dropHintString) )
        Com_Error(ERR_DROP, "Too many different hintstring values on weapons. Max allowed is %i different strings.", 96);
    if ( *weapDef->worldModel )
    {
        Name = (char *)XModelGetName(*(const XModel **)weapDef->worldModel);
        modelindex = G_ModelIndex(Name);
        if ( XModelBad(*(const XModel **)weapDef->worldModel) )
            G_OverrideModel(modelindex, (char*)"defaultweapon");
    }
    if ( weapDef->projectileModel )
    {
        v2 = (char *)XModelGetName(weapDef->projectileModel);
        G_ModelIndex(v2);
    }
    if ( *((unsigned int *)weapDef->worldModel + 1) )
    {
        v3 = (char *)XModelGetName(*((const XModel **)weapDef->worldModel + 1));
        G_ModelIndex(v3);
    }
    if ( weapDef->additionalMeleeModel )
    {
        v4 = (char *)XModelGetName(weapDef->additionalMeleeModel);
        G_ModelIndex(v4);
    }
}

int __cdecl IsItemRegistered(unsigned int iItemIndex)
{
    if ( iItemIndex >= 0x800
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp",
                    1585,
                    0,
                    "%s",
                    "(iItemIndex >= 0) && (iItemIndex < MAX_WEAPONS)") )
    {
        __debugbreak();
    }
    return itemRegistered[iItemIndex];
}

void __cdecl G_SpawnItem(gentity_s *ent, const gitem_s *item)
{
    char *Name; // eax
    unsigned __int8 boneIndex; // [esp+Bh] [ebp-19h] BYREF
    DObj *dobj; // [esp+Ch] [ebp-18h]
    int tagIndex; // [esp+10h] [ebp-14h]
    const WeaponVariantDef *weapVariantDef; // [esp+14h] [ebp-10h]
    int weapIndex; // [esp+18h] [ebp-Ch]
    unsigned __int8 weapModel; // [esp+1Fh] [ebp-5h]
    const WeaponDef *weapDef; // [esp+20h] [ebp-4h]
    int savedregs; // [esp+24h] [ebp+0h] BYREF

    ent->item[0].index = ((char *)item - (char *)bg_itemlist) >> 2;
    weapIndex = ent->item[0].index % 2048;
    if ( weapIndex >= (int)BG_GetNumWeapons() )
        Com_Error(ERR_DROP, "G_SpawnItem() - Bad item WEAPIDX %i on entity", weapIndex);
    weapModel = ent->item[0].index / 2048;
    weapDef = BG_GetWeaponDef(weapIndex);
    if ( !weapDef && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp", 1618, 0, "%s", "weapDef") )
        __debugbreak();
    if ( weapDef->worldModel[weapModel] )
    {
        Name = (char *)XModelGetName(weapDef->worldModel[weapModel]);
        G_SetModel(ent, Name);
    }
    if ( item->giType != IT_WEAPON
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp",
                    1622,
                    0,
                    "%s",
                    "item->giType == IT_WEAPON") )
    {
        __debugbreak();
    }
    ent->r.mins[0] = -1.0f;
    ent->r.mins[1] = -1.0f;
    ent->r.mins[2] = -1.0f;
    ent->r.maxs[0] = 1.0f;
    ent->r.maxs[1] = 1.0f;
    ent->r.maxs[2] = 1.0f;
    ent->r.contents = 1079771400;
    ent->r.contents |= 0x200000u;
    ent->s.eType = ET_ITEM;
    ent->s.un3.item = *((unsigned __int16 *)&ent->spawner + 4);
    if ( ent->s.un3.item >= 0x800u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp",
                    1632,
                    0,
                    "ent->s.un3.item doesn't index MAX_ITEMS\n\t%i not in [0, %i)",
                    ent->s.un3.item,
                    2048) )
    {
        __debugbreak();
    }
    if ( ent->count )
        TransferSpecifiedAmmoToWeaponEntity(ent, weapIndex);
    else
        TransferRandomAmmoToWeaponEntity(ent, weapIndex);
    G_DObjUpdate(ent);
    if ( weapDef->worldModel[weapModel] )
    {
        dobj = Com_GetServerDObj(ent->s.number);
        weapVariantDef = BG_GetWeaponVariantDef(weapIndex);
        if ( !weapVariantDef
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp", 1652, 0, "%s", "weapVariantDef") )
        {
            __debugbreak();
        }
        for ( tagIndex = 0; tagIndex < 32 && weapVariantDef->hideTags[tagIndex]; ++tagIndex )
        {
            boneIndex = -2;
            if ( DObjGetBoneIndex(dobj, weapVariantDef->hideTags[tagIndex], &boneIndex, -1) )
                ent->s.partBits[(int)boneIndex >> 5] |= 0x80000000 >> (boneIndex & 0x1F);
        }
        DObjSetHidePartBits(dobj, ent->s.partBits);
    }
    ent->s.clientNum = 32;
    ent->flags |= 0x1000u;
    if ( level.spawnVar.spawnVarsValid )
    {
        G_SetAngle(ent, ent->r.currentAngles);
        ent->nextthink = level.time + 100;
        ent->handler = 20;
    }
    else
    {
        ent->handler = 21;
        if ( (ent->spawnflags & 1) == 0 )
        {
            ent->s.groundEntityNum = 1023;
            if ( item->giType != IT_WEAPON
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp",
                            1693,
                            0,
                            "%s",
                            "item->giType == IT_WEAPON") )
            {
                __debugbreak();
            }
            ent->r.currentAngles[2] = ent->r.currentAngles[2] + 90.0;
        }
        G_SetAngle(ent, ent->r.currentAngles);
        G_SetOrigin(ent, ent->r.currentOrigin);
        SV_LinkEntity(ent);
    }
}

int __cdecl TransferSpecifiedAmmoToWeaponEntity(gentity_s *weaponEnt, int transferWeapon)
{
    int count; // [esp+0h] [ebp-58h]
    int clipSize; // [esp+38h] [ebp-20h]
    int i; // [esp+3Ch] [ebp-1Ch]
    int iDropAmmo; // [esp+40h] [ebp-18h]
    int total; // [esp+44h] [ebp-14h]
    int iMaxAmmo; // [esp+48h] [ebp-10h]
    int iDropClip; // [esp+50h] [ebp-8h]

    total = 0;
    for ( i = 0; i < 2; ++i )
    {
        if ( transferWeapon > 0 )
        {
            if ( BG_GetWeaponDef(transferWeapon)->iSharedAmmoCapIndex >= 0 && i )
                return total;
            iMaxAmmo = BG_GetMaxAmmo(transferWeapon);
            if ( weaponEnt->count <= iMaxAmmo )
                count = weaponEnt->count;
            else
                count = iMaxAmmo;
            if ( count > 0 )
            {
                clipSize = BG_GetClipSize(transferWeapon);
                if ( clipSize < 0
                    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp", 1137, 0, "%s", "clipSize >= 0") )
                {
                    __debugbreak();
                }
                if ( clipSize == 1 )
                    iDropClip = 1;
                else
                    iDropClip = G_rand() % (clipSize + 1);
                if ( iDropClip < count )
                {
                    iDropAmmo = count - iDropClip;
                }
                else
                {
                    iDropClip = count;
                    iDropAmmo = 0;
                }
            }
            else
            {
                iDropAmmo = 0;
                iDropClip = 0;
            }
            weaponEnt->item[i].index = transferWeapon;
            weaponEnt->item[i].ammoCount = iDropAmmo;
            weaponEnt->item[i].clipAmmoCount = iDropClip;
            transferWeapon = BG_GetWeaponVariantDef(transferWeapon)->altWeaponIndex;
            total += weaponEnt->item[i].clipAmmoCount + weaponEnt->item[i].ammoCount;
        }
    }
    return total;
}

void __cdecl G_RunItem(gentity_s *ent)
{
    int v1; // [esp+8h] [ebp-FCh]
    int passEntityNum; // [esp+Ch] [ebp-F8h]
    col_context_t context; // [esp+74h] [ebp-90h] BYREF
    float delta[3]; // [esp+9Ch] [ebp-68h]
    float origin[3]; // [esp+A8h] [ebp-5Ch] BYREF
    float endpos[3]; // [esp+B4h] [ebp-50h] BYREF
    trace_t tr; // [esp+C0h] [ebp-44h] BYREF
    int mask; // [esp+FCh] [ebp-8h]
    float dot; // [esp+100h] [ebp-4h]
    int savedregs; // [esp+104h] [ebp+0h] BYREF

    if ( ent->s.eType == 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp",
                    1743,
                    0,
                    "%s",
                    "ent->s.eType != ET_PLAYER_CORPSE") )
    {
        __debugbreak();
    }
    if ( ent->s.groundEntityNum == 1023 || level.gentities[ent->s.groundEntityNum].s.lerp.pos.trType )
    {
        if ( ent->s.lerp.pos.trType == 6 || ent->s.lerp.pos.trType == 10 || ent->s.lerp.pos.trType == 13 )
        {
            if ( ent->s.lerp.pos.trType == 10 )
                return;
        }
        else if ( (ent->spawnflags & 1) == 0 )
        {
            ent->s.lerp.pos.trType = 6;
            ent->s.lerp.pos.trTime = level.time;
            ent->s.lerp.pos.trBase[0] = ent->r.currentOrigin[0];
            ent->s.lerp.pos.trBase[1] = ent->r.currentOrigin[1];
            ent->s.lerp.pos.trBase[2] = ent->r.currentOrigin[2];
            ent->s.lerp.pos.trDelta[0] = 0.0f;
            ent->s.lerp.pos.trDelta[1] = 0.0f;
            ent->s.lerp.pos.trDelta[2] = 0.0f;
        }
    }
    if ( ((LODWORD(ent->s.lerp.pos.trBase[0]) & 0x7F800000) == 0x7F800000
         || (LODWORD(ent->s.lerp.pos.trBase[1]) & 0x7F800000) == 0x7F800000
         || (LODWORD(ent->s.lerp.pos.trBase[2]) & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp",
                    1768,
                    0,
                    "%s",
                    "!IS_NAN((ent->s.lerp.pos.trBase)[0]) && !IS_NAN((ent->s.lerp.pos.trBase)[1]) && !IS_NAN((ent->s.lerp.pos.trBase)[2])") )
    {
        __debugbreak();
    }
    if ( ((LODWORD(ent->s.lerp.pos.trDelta[0]) & 0x7F800000) == 0x7F800000
         || (LODWORD(ent->s.lerp.pos.trDelta[1]) & 0x7F800000) == 0x7F800000
         || (LODWORD(ent->s.lerp.pos.trDelta[2]) & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp",
                    1769,
                    0,
                    "%s",
                    "!IS_NAN((ent->s.lerp.pos.trDelta)[0]) && !IS_NAN((ent->s.lerp.pos.trDelta)[1]) && !IS_NAN((ent->s.lerp.pos.trDelta)[2])") )
    {
        __debugbreak();
    }
    if ( ent->s.lerp.pos.trType && !ent->tagInfo )
    {
        BG_EvaluateTrajectory(&ent->s.lerp.pos, level.time + 50, origin);
        if ( ((LODWORD(origin[0]) & 0x7F800000) == 0x7F800000
             || (LODWORD(origin[1]) & 0x7F800000) == 0x7F800000
             || (LODWORD(origin[2]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp",
                        1781,
                        0,
                        "%s",
                        "!IS_NAN((origin)[0]) && !IS_NAN((origin)[1]) && !IS_NAN((origin)[2])") )
        {
            __debugbreak();
        }
        mask = G_ItemClipMask(ent);
        if ( (mask & ent->r.contents) != 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp",
                        1787,
                        0,
                        "%s",
                        "!( ent->r.contents & mask )") )
        {
            __debugbreak();
        }
        if ( Vec3DistanceSq(ent->r.currentOrigin, origin) < 0.1 )
            origin[2] = origin[2] - 1.0;
        //col_context_t::col_context_t(&context);
        //if ( EntHandle::isDefined(&ent->r.ownerNum) )
        if ( ent->r.ownerNum.isDefined() )
        {
            //passEntityNum = EntHandle::entnum(&ent->r.ownerNum);
            passEntityNum = ent->r.ownerNum.entnum();
            G_TraceCapsule(&tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, passEntityNum, mask, &context);
        }
        else
        {
            G_TraceCapsule(&tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, 1023, mask, &context);
        }
        if ( tr.fraction >= 1.0 )
        {
            ent->r.currentOrigin[0] = origin[0];
            ent->r.currentOrigin[1] = origin[1];
            ent->r.currentOrigin[2] = origin[2];
        }
        else
        {
            Vec3Lerp(ent->r.currentOrigin, origin, tr.fraction, endpos);
            if ( !tr.startsolid && tr.fraction < 0.0099999998 && tr.normal.vec.v[2] < 0.5 )
            {
                delta[0] = origin[0] - ent->r.currentOrigin[0];
                delta[1] = origin[1] - ent->r.currentOrigin[1];
                delta[2] = origin[2] - ent->r.currentOrigin[2];
                dot = 1.0
                        - (float)((float)((float)(delta[0] * tr.normal.vec.v[0]) + (float)(delta[1] * tr.normal.vec.v[1]))
                                        + (float)(delta[2] * tr.normal.vec.v[2]));
                origin[0] = (float)(dot * tr.normal.vec.v[0]) + origin[0];
                origin[1] = (float)(dot * tr.normal.vec.v[1]) + origin[1];
                origin[2] = (float)(dot * tr.normal.vec.v[2]) + origin[2];
                //if ( EntHandle::isDefined(&ent->r.ownerNum) )
                if ( ent->r.ownerNum.isDefined() )
                {
                    //v1 = EntHandle::entnum(&ent->r.ownerNum);
                    v1 = ent->r.ownerNum.entnum();
                    G_TraceCapsule(&tr, endpos, ent->r.mins, ent->r.maxs, origin, v1, mask, &context);
                }
                else
                {
                    G_TraceCapsule(&tr, endpos, ent->r.mins, ent->r.maxs, origin, 1023, mask, &context);
                }
                Vec3Lerp(endpos, origin, tr.fraction, endpos);
            }
            ent->s.lerp.pos.trType = 4;
            ent->s.lerp.pos.trTime = level.time;
            ent->s.lerp.pos.trDuration = 50;
            ent->s.lerp.pos.trBase[0] = ent->r.currentOrigin[0];
            ent->s.lerp.pos.trBase[1] = ent->r.currentOrigin[1];
            ent->s.lerp.pos.trBase[2] = ent->r.currentOrigin[2];
            ent->s.lerp.pos.trDelta[0] = endpos[0] - ent->r.currentOrigin[0];
            ent->s.lerp.pos.trDelta[1] = endpos[1] - ent->r.currentOrigin[1];
            ent->s.lerp.pos.trDelta[2] = endpos[2] - ent->r.currentOrigin[2];
            ent->s.lerp.pos.trDelta[0] = 20.0 * ent->s.lerp.pos.trDelta[0];
            ent->s.lerp.pos.trDelta[1] = 20.0 * ent->s.lerp.pos.trDelta[1];
            ent->s.lerp.pos.trDelta[2] = 20.0 * ent->s.lerp.pos.trDelta[2];
            ent->r.currentOrigin[0] = endpos[0];
            ent->r.currentOrigin[1] = endpos[1];
            ent->r.currentOrigin[2] = endpos[2];
        }
        SV_LinkEntity(ent);
        G_RunThink(ent);
        if ( ent->r.inuse && tr.fraction < 0.0099999998 )
        {
            if ( tr.normal.vec.v[2] <= 0.0 || SV_PointContents(ent->r.currentOrigin, -1, 0x80000000) )
            {
                G_FreeEntity(ent);
            }
            else
            {
                G_OrientItemToGround(ent, &tr);
                G_SetOrigin(ent, endpos);
                ent->s.groundEntityNum = Trace_GetEntityHitId(&tr);
                g_entities[ent->s.groundEntityNum].flags |= 0x100000u;
                SV_LinkEntity(ent);
            }
        }
    }
    else
    {
        G_RunThink(ent);
    }
}

void __cdecl G_OrientItemToGround(gentity_s *ent, trace_t *trace)
{
    float vAngles[3]; // [esp+4h] [ebp-30h] BYREF
    float vAxis[3][3]; // [esp+10h] [ebp-24h] BYREF

    vAxis[2][0] = trace->normal.vec.v[0];
    vAxis[2][1] = trace->normal.vec.v[1];
    vAxis[2][2] = trace->normal.vec.v[2];
    AngleVectors(ent->r.currentAngles, vAxis[0], 0, 0);
    Vec3Cross(vAxis[2], vAxis[0], vAxis[1]);
    Vec3Cross(vAxis[1], vAxis[2], vAxis[0]);
    AxisToAngles(vAxis, vAngles);
    if ( ent->s.un3.item )
    {
        //if ( bg_itemlist[ent->s.un3.item] != 1
        //    && !Assert_MyHandler(
        //                "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_items.cpp",
        //                1721,
        //                0,
        //                "%s",
        //                "bg_itemlist[ent->s.un3.item].giType == IT_WEAPON") )
        //{
        //    __debugbreak();
        //}
        vAngles[2] = vAngles[2] + 90.0;
    }
    G_SetAngle(ent, vAngles);
}

