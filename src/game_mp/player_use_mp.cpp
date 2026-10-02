#include "player_use_mp.h"
#include <clientscript/cscr_vm.h>
#include "g_spawn_mp.h"
#include <clientscript/scr_const.h>
#include <game/turret.h>
#include <client_mp/g_client_mp.h>
#include <bgame/bg_misc.h>
#include <qcommon/cm_world.h>
#include <server/sv_game.h>
#include "g_team_mp.h"
#include <bgame/bg_perks.h>

#include <algorithm>
#include "g_utils_mp.h"
#include <game/actor_script_cmd.h>
#include <bgame/bg_weapons_def.h>


bool __cdecl compare_use(const useList_t &a, const useList_t &b)
{
    return b.score > a.score;
}

static const float turretScanRadiusSq = 16384.0;
char __cdecl Player_IsPlayerUsingTurretNearby(gentity_s *ent)
{
    gentity_s *clEnt; // [esp+Ch] [ebp-8h]
    int i; // [esp+10h] [ebp-4h]

    i = 0;
    clEnt = g_entities;
    while ( i < com_maxclients->current.integer )
    {
        if ( clEnt->client
            && clEnt->client->sess.connected == CON_CONNECTED
            && ((clEnt->client->ps.eFlags & 0x300) != 0
             || (clEnt->client->ps.weapFlags & 0x800) != 0
             || clEnt->client->ps.weaponstate == 35)
            && Vec3DistanceSq(ent->client->ps.origin, clEnt->client->ps.origin) <= turretScanRadiusSq )
        {
            return 1;
        }
        ++i;
        ++clEnt;
    }
    return 0;
}

void __cdecl Player_UpdateActivate(gentity_s *ent)
{
    char useSucceeded; // [esp+3h] [ebp-1h]

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\player_use_mp.cpp", 259, 0, "%s", "ent") )
        __debugbreak();
    if ( !ent->client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\player_use_mp.cpp", 260, 0, "%s", "ent->client") )
    {
        __debugbreak();
    }
    ent->client->ps.weapFlags &= ~1u;
    ent->client->ps.weapFlags &= ~0x10000u;
    useSucceeded = 0;
    if ( ent->client->button_bits.testBit(0x32u) )
    {
        Player_ActivateDoubleTapCmd(ent);
    }
    else if ( (ent->client->useHoldEntity.isDefined() || ent->client->ps.mountAvailable)
                 &&  ent->client->oldbutton_bits.testBit(5u)
                 && !ent->client->button_bits.testBit(5u) )
    {
        ent->client->ps.weapFlags |= 1u;
        ent->client->ps.weapFlags |= 0x10000u;
    }
    else
    {
        if (   ent->client->latched_button_bits.testBit(3u)
            || ent->client->latched_button_bits.testBit(5u)
            || ent->client->latched_button_bits.testBit(4u) )
        {
            useSucceeded = Player_ActivateCmd(ent);
        }
        if ( ent->client->useHoldEntity.isDefined() || useSucceeded || ent->client->ps.mountAvailable )
        {
            if ( ent->client->button_bits.testBit(3u) || ent->client->button_bits.testBit(5u) )
                Player_ActivateHoldCmd(ent);
            ent->client->useButtonDone = 1;
        }
        else if ( ent->client->latched_button_bits.testBit(5u) )
        {
            ent->client->ps.weapFlags |= 1u;
            ent->client->ps.weapFlags |= 0x10000u;
        }
    }
}

char __cdecl Player_ActivateCmd(gentity_s *ent)
{
    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\player_use_mp.cpp", 81, 0, "%s", "ent") )
        __debugbreak();
    if ( !ent->r.inuse
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\player_use_mp.cpp", 82, 0, "%s", "ent->r.inuse") )
    {
        __debugbreak();
    }
    if ( !ent->client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\player_use_mp.cpp", 83, 0, "%s", "ent->client") )
    {
        __debugbreak();
    }
    if ( !Scr_IsSystemActive(1u, SCRIPTINSTANCE_SERVER) )
        return 0;
    ent->client->useHoldEntity.setEnt(0);
    if ( ent->active )
    {
        if ( (ent->client->ps.eFlags & 0x300) != 0 )
            ent->active = 2;
        else
            ent->active = 0;
        return 1;
    }
    else
    {
        if ( (ent->client->ps.eFlags & 0x14000) != 0 )
            ent->client->useButtonDone = 1;
        if ( (ent->client->ps.pm_flags & 4) != 0 )
        {
            return 1;
        }
        else if ( (ent->client->ps.pm_flags & 0x8000) != 0 )
        {
            return 1;
        }
        else if ( (ent->client->ps.pm_flags & 0x400000) != 0 )
        {
            return 1;
        }
        else if ( ent->client->ps.weaponstate < 20 || ent->client->ps.weaponstate > 25 )
        {
            if ( ent->client->ps.cursorHint || ent->client->ps.mountAvailable )
            {
                if ( ent->client->ps.cursorHintEntIndex != 1023 || ent->client->ps.mountAvailable )
                {
                    if ( ent->client->ps.cursorHintEntIndex != 1023 )
                    {
                        if ( !g_entities[ent->client->ps.cursorHintEntIndex].r.inuse
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\player_use_mp.cpp",
                                        126,
                                        0,
                                        "%s",
                                        "g_entities[ ent->client->ps.cursorHintEntIndex ].r.inuse") )
                        {
                            __debugbreak();
                        }
                        ent->client->useHoldEntity.setEnt(&g_entities[ent->client->ps.cursorHintEntIndex]);
                    }
                    ent->client->useHoldTime = level.time;
                    return 1;
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
        }
        else
        {
            return 1;
        }
    }
}

void __cdecl Player_ActivateHoldCmd(gentity_s *ent)
{
    int v1; // eax
    gentity_s *useEnt; // [esp+0h] [ebp-4h]

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\player_use_mp.cpp", 170, 0, "%s", "ent") )
        __debugbreak();
    if ( !ent->client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\player_use_mp.cpp", 171, 0, "%s", "ent->client") )
    {
        __debugbreak();
    }
    if ( Scr_IsSystemActive(1u, SCRIPTINSTANCE_SERVER)
        && (ent->client->useHoldEntity.isDefined() || ent->client->ps.mountAvailable)
        && level.time - ent->client->lastSpawnTime >= g_useholdspawndelay->current.integer
        && (!ent->client->button_bits.testBit(5u)
         || level.time - ent->client->useHoldTime >= g_useholdtime->current.integer) )
    {
        if ( ent->client->ps.mountAvailable )
        {
            if ( Mount_CanPlayerDeployTurret(&ent->client->ps) && !Player_IsPlayerUsingTurretNearby(ent) )
            {
                ent->client->ps.weapFlags |= 0x800u;
                BG_AddPredictableEventToPlayerstate(0x25u, 0, &ent->client->ps);
            }
        }
        else if ( ent->client->ps.weaponstate != 35
                     && ent->client->ps.weaponstate != 36
                     && ent->client->ps.weaponstate != 37
                     && (ent->client->ps.eFlags & 0x300) == 0
                     && (ent->client->ps.pm_flags & 4) == 0
                     && ent->client->ps.weaponstate != 1
                     && ent->client->ps.weaponstate != 2
                     && ent->client->ps.weaponstate != 3
                     && ent->client->ps.weaponstate != 4
                     && ent->client->ps.weaponstate != 5 )
        {
            useEnt = ent->client->useHoldEntity.ent();
            if ( useEnt->s.number != ent->client->useHoldEntity.entnum() )
            {
                v1 = ent->client->useHoldEntity.entnum();
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\player_use_mp.cpp",
                                205,
                                0,
                                "useEnt->s.number == (int)ent->client->useHoldEntity.entnum()\n\t%i, %i",
                                useEnt->s.number,
                                v1) )
                    __debugbreak();
            }
            if ( !useEnt->r.inuse
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\player_use_mp.cpp",
                            206,
                            0,
                            "%s\n\t(useEnt->s.number) = %i",
                            "(useEnt->r.inuse)",
                            useEnt->s.number) )
            {
                __debugbreak();
            }
            Player_UseEntity(ent, useEnt);
        }
    }
}

void __cdecl Player_UseEntity(gentity_s *playerEnt, gentity_s *useEnt)
{
    void(__cdecl * touch)(gentity_s *, gentity_s *, int); // [esp+0h] [ebp-8h]
    void(__cdecl * use)(gentity_s *, gentity_s *, gentity_s *); // [esp+4h] [ebp-4h]

    if (!playerEnt
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\player_use_mp.cpp", 40, 0, "%s", "playerEnt"))
    {
        __debugbreak();
    }
    if (!playerEnt->client
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\player_use_mp.cpp",
            41,
            0,
            "%s",
            "playerEnt->client"))
    {
        __debugbreak();
    }
    if (!useEnt
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\player_use_mp.cpp", 42, 0, "%s", "useEnt"))
    {
        __debugbreak();
    }
    if (!useEnt->r.inuse
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\player_use_mp.cpp",
            44,
            0,
            "%s",
            "useEnt->r.inuse"))
    {
        __debugbreak();
    }
    if (useEnt->s.eType == 3)
    {
        Scr_AddEntity(playerEnt, SCRIPTINSTANCE_SERVER);
        Scr_Notify(useEnt, scr_const.touch, 1u);
        useEnt->active = 1;
        touch = entityHandlers[useEnt->handler].touch;
        if (touch)
            touch(useEnt, playerEnt, 0);
    }
    else if (useEnt->s.eType != 11 || G_IsTurretUsable(useEnt, playerEnt))
    {
        Scr_AddEntity(playerEnt, SCRIPTINSTANCE_SERVER);
        Scr_Notify(useEnt, scr_const.trigger, 1u);
        use = entityHandlers[useEnt->handler].use;
        if (use)
            use(useEnt, playerEnt, playerEnt);
    }
    playerEnt->client->useHoldEntity.setEnt(0);
}

void __cdecl Player_ActivateDoubleTapCmd(gentity_s *ent)
{
    int v1; // eax
    gentity_s *useEnt; // [esp+0h] [ebp-4h]

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\player_use_mp.cpp", 220, 0, "%s", "ent") )
        __debugbreak();
    if ( !ent->client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\player_use_mp.cpp", 221, 0, "%s", "ent->client") )
    {
        __debugbreak();
    }
    if ( Scr_IsSystemActive(1u, SCRIPTINSTANCE_SERVER)
        && ent->client->useHoldEntity.isDefined()
        && level.time - ent->client->lastSpawnTime >= g_useholdspawndelay->current.integer
        && ent->client->ps.weaponstate != 35
        && ent->client->ps.weaponstate != 36
        && ent->client->ps.weaponstate != 37
        && (ent->client->ps.eFlags & 0x300) == 0
        && (ent->client->ps.pm_flags & 4) == 0 )
    {
        useEnt = ent->client->useHoldEntity.ent();
        if ( useEnt->s.number != ent->client->useHoldEntity.entnum() )
        {
            v1 = ent->client->useHoldEntity.entnum();
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\player_use_mp.cpp",
                            244,
                            0,
                            "useEnt->s.number == (int)ent->client->useHoldEntity.entnum()\n\t%i, %i",
                            useEnt->s.number,
                            v1) )
                __debugbreak();
        }
        if ( !useEnt->r.inuse
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\player_use_mp.cpp",
                        245,
                        0,
                        "%s\n\t(useEnt->s.number) = %i",
                        "(useEnt->r.inuse)",
                        useEnt->s.number) )
        {
            __debugbreak();
        }
        Scr_AddEntity(ent, SCRIPTINSTANCE_SERVER);
        Scr_Notify(useEnt, scr_const.trigger_use_doubletap, 1u);
    }
}

int Player_GetUseList(gentity_s *ent, useList_t *useList, int prevHintEntIndex);

void __cdecl Player_UpdateCursorHints(gentity_s *ent)
{
    int v2; // [esp+14h] [ebp-202Ch]
    int IsVehicleUsable; // [esp+18h] [ebp-2028h]
    int scale; // [esp+1Ch] [ebp-2024h]
    int ItemCursorHint; // [esp+20h] [ebp-2020h]
    int item; // [esp+2Ch] [ebp-2014h]
    useList_t useList[1024]; // [esp+30h] [ebp-2010h] BYREF
    int v8; // [esp+2030h] [ebp-10h]
    int v9; // [esp+2034h] [ebp-Ch]
    playerState_s *ps; // [esp+2038h] [ebp-8h]
    gentity_s *self; // [esp+203Ch] [ebp-4h]

    self = 0;
    if ( !ent->client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\player_use_mp.cpp", 726, 0, "%s", "ent->client") )
    {
        __debugbreak();
    }
    ps = &ent->client->ps;
    ps->cursorHint = 0;
    if ( !BG_ThrowingBackGrenade(ps) )
        ps->throwBackGrenadeTimeLeft = 0;
    if ( ent->health > 0 )
    {
        if ( ent->active )
        {
            if ( (ps->eFlags & 0x300) != 0 )
                Player_SetTurretDropHint(ent);
            return;
        }
        if ( (ent->client->ps.pm_flags & 4) == 0
            && (ent->client->ps.pm_flags & 0x8000) == 0
            && (ent->client->ps.pm_flags & 0x400000) == 0
            && (ent->client->ps.pm_flags & 8) == 0
            && (ps->eFlags2 & 0x10000000) == 0
            && (ps->weapFlags & 0x100000) == 0
            && (ps->weaponstate < 20 || ps->weaponstate > 24) )
        {
            if ( !Player_IsPlayerUsingTurretNearby(ent)
                && (ent->client->ps.mountAvailable || ps->weaponstate == 35 || ps->weaponstate == 36 || ps->weaponstate == 37) )
            {
                ps->cursorHintEntIndex = 1023;
                return;
            }
            if ( ps->pm_type == 6 )
            {
                ps->cursorHintEntIndex = 1023;
                return;
            }
            if ( (ps->weapFlags & 0x400000) == 0
                && ((ps->locationSelectionInfo & 0xF) == 0xF || !ps->locationSelectionInfo)
                && (ps->eFlags & 0x4000) == 0
                && (ent->client->flags & 4) == 0
                && (ent->client->flags & 8) == 0 )
            {
                v8 = Player_GetUseList(ent, useList, ps->cursorHintEntIndex);
                if ( v8 )
                {
                    item = 0;
                    scale = -1;
                    v9 = 0;
                    while ( 1 )
                    {
                        if ( v9 >= v8 )
                            return;
                        self = useList[v9].ent;
                        if ( !self
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\player_use_mp.cpp",
                                        841,
                                        0,
                                        "%s",
                                        "traceEnt") )
                        {
                            __debugbreak();
                        }
                        if ( !self->r.inuse
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\player_use_mp.cpp",
                                        842,
                                        0,
                                        "%s",
                                        "traceEnt->r.inuse") )
                        {
                            __debugbreak();
                        }
                        switch ( self->s.eType )
                        {
                            case 0:
                                if ( self->classname != scr_const.trigger_use
                                    && self->classname != scr_const.trigger_use_touch
                                    && self->classname != scr_const.trigger_radius
                                    && self->classname != scr_const.trigger_radius_use )
                                {
                                    goto LABEL_85;
                                }
                                if ( self->team && self->team != ent->client->sess.cs.team
                                    || self->trigger.perk != 52 && !BG_HasPerk(ent->client->ps.perks, self->trigger.perk)
                                    || self->item[1].ammoCount != 1023 && self->item[1].ammoCount != ent->client->ps.clientNum
                                    || (self->r.clientMask[(int)ps->clientNum >> 5] & (1 << (ps->clientNum & 0x1F))) != 0 )
                                {
                                    goto LABEL_39;
                                }
                                item = self->s.un3.item;
                                if ( item && self->s.un1.scale != 255 )
                                    scale = self->s.un1.scale;
                                goto LABEL_85;
                            case 3:
                                if ( (0x800000 & self->s.lerp.eFlags2) != 0 )
                                    goto LABEL_39;
                                ItemCursorHint = Player_GetItemCursorHint(ent->client, self);
                                if ( !ItemCursorHint )
                                    goto LABEL_39;
                                item = ItemCursorHint;
                                goto LABEL_85;
                            case 4:
                                item = self->s.un3.item % 2048 + 7;
                                ps->throwBackGrenadeTimeLeft = self->nextthink - level.time;
                                goto LABEL_85;
                            case 6:
                                item = self->s.un3.item;
                                if ( item && self->s.un1.scale != 255 )
                                    scale = self->s.un1.scale;
                                if ( !self->s.un3.item )
                                    goto LABEL_85;
                                v2 = (self->s.eventParm & 0xC000) >> 14;
                                if ( !v2 || v2 == ent->client->sess.cs.team )
                                {
                                    if ( (self->s.eventParm & 0x3FFF) != 0 && BG_HasPerk(ps->perks, (self->s.eventParm & 0x3FFF) >> 8) )
                                        scale = (unsigned __int8)self->s.eventParm;
LABEL_85:
                                    if ( item )
                                    {
                                        ps->cursorHintEntIndex = self->s.number;
                                        self->flags |= 0x200000u;
                                        ps->cursorHint = item;
                                        ps->cursorHintString = scale;
                                    }
                                    else
                                    {
                                        ps->cursorHintEntIndex = 1023;
                                    }
                                    if ( ps->cursorHintEntIndex != 1023
                                        && !g_entities[ps->cursorHintEntIndex].r.inuse
                                        && !Assert_MyHandler(
                                                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\player_use_mp.cpp",
                                                    961,
                                                    1,
                                                    "%s\n\t(ps->cursorHintEntIndex) = %i",
                                                    "(ps->cursorHintEntIndex == ((1<<10)-1) || g_entities[ps->cursorHintEntIndex].r.inuse)",
                                                    ps->cursorHintEntIndex) )
                                    {
                                        __debugbreak();
                                    }
                                    return;
                                }
                                item = 0;
                                scale = -1;
LABEL_39:
                                ++v9;
                                break;
                            case 0xB:
                                if ( !G_IsTurretUsable(self, ent) )
                                    goto LABEL_39;
                                item = self->s.weapon + 7;
                                if ( *BG_GetWeaponDef(self->s.weapon)->szUseHintString )
                                    scale = BG_GetWeaponDef(self->s.weapon)->iUseHintStringIndex;
                                goto LABEL_85;
                            case 0xE:
                                IsVehicleUsable = G_IsVehicleUsable(self, ent);
                                if ( IsVehicleUsable == -1 )
                                    goto LABEL_39;
                                item = 1;
                                scale = IsVehicleUsable;
                                if ( !ent->client->disallowVehicleUsage )
                                    goto LABEL_85;
                                ps->cursorHintEntIndex = 1023;
                                ps->cursorHint = 1;
                                ps->cursorHintString = IsVehicleUsable;
                                goto LABEL_39;
                            default:
                                goto LABEL_39;
                        }
                    }
                }
            }
        }
    }
}

int Player_GetUseList(gentity_s *ent, useList_t *useList, int prevHintEntIndex)
{
    int eType; // [esp+0h] [ebp-13A4h]
    int integer; // [esp+10h] [ebp-1394h]
    float v7; // [esp+14h] [ebp-1390h]
    float value; // [esp+18h] [ebp-138Ch]
    float v9; // [esp+280h] [ebp-1124h]
    float v10; // [esp+284h] [ebp-1120h]
    float v11[3]; // [esp+2A0h] [ebp-1104h] BYREF
    BOOL v12; // [esp+2ACh] [ebp-10F8h]
    float v13[3]; // [esp+2B0h] [ebp-10F4h] BYREF
    float v14; // [esp+2BCh] [ebp-10E8h]
    BOOL v15; // [esp+2C0h] [ebp-10E4h]
    float v16; // [esp+2C4h] [ebp-10E0h]
    gentity_s *ent1; // [esp+2C8h] [ebp-10DCh]
    float v18; // [esp+2CCh] [ebp-10D8h]
    float v[3]; // [esp+2D0h] [ebp-10D4h] BYREF
    //float v20; // [esp+2D4h] [ebp-10D0h]
    //float v21; // [esp+2D8h] [ebp-10CCh]
    int entityList[1025]; // [esp+2DCh] [ebp-10C8h] BYREF
    int hitNum; // [esp+12E0h] [ebp-C4h] BYREF
    float origin[3]; // [esp+130Ch] [ebp-98h] BYREF
    //float v26; // [esp+1310h] [ebp-94h]
    //float v27; // [esp+1314h] [ebp-90h]
    float v28[3]; // [esp+1318h] [ebp-8Ch] BYREF
    float v31; // [esp+1324h] [ebp-80h]
    float mins[3]; // [esp+1328h] [ebp-7Ch] BYREF
    float v33; // [esp+1334h] [ebp-70h]
    float v34; // [esp+1338h] [ebp-6Ch]
    float v35; // [esp+133Ch] [ebp-68h]
    float v36; // [esp+1340h] [ebp-64h]
    int v37; // [esp+1344h] [ebp-60h]
    float v38; // [esp+1348h] [ebp-5Ch]
    gentity_s *gEnt; // [esp+134Ch] [ebp-58h]
    float forward[3]; // [esp+1350h] [ebp-54h] BYREF
    float v41; // [esp+135Ch] [ebp-48h]
    int v42; // [esp+1360h] [ebp-44h]
    int v43; // [esp+1364h] [ebp-40h]
    float maxs[3]; // [esp+1368h] [ebp-3Ch] BYREF
    float pos[3]; // [esp+1374h] [ebp-30h] BYREF
    //float v46; // [esp+1378h] [ebp-2Ch]
    //float v47; // [esp+137Ch] [ebp-28h]
    int v48; // [esp+1380h] [ebp-24h]
    int v49; // [esp+1384h] [ebp-20h]
    int i; // [esp+1388h] [ebp-1Ch]
    playerState_s *ps; // [esp+138Ch] [ebp-18h]
    team_t team; // [esp+1390h] [ebp-14h]
    float v53; // [esp+1394h] [ebp-10h]
    float v54[3]; // [esp+1398h] [ebp-Ch] BYREF
    gentity_s *ent2; // [esp+13ACh] [ebp+8h]

    v31 = 256.0f;
    v53 = 0.75999999f;
    v48 = 0;
    if ((float)(192.0 - player_throwbackOuterRadius->current.value) < 0.0)
        value = player_throwbackOuterRadius->current.value;
    else
        value = 192.0f;
    v34 = value;
    if ((float)(value - player_MGUseRadius->current.value) < 0.0)
        v7 = player_MGUseRadius->current.value;
    else
        v7 = v34;
    v34 = v7;
    if ((float)(v7 - player_useRadius->current.value) < 0.0)
        integer = player_useRadius->current.integer;
    else
        integer = LODWORD(v34);
    v34 = *(float *)&integer;
    v35 = *(float *)&integer;
    v36 = 96.0f;
    ps = &ent->client->ps;
    v37 = 0;
    G_GetPlayerViewOrigin(ps, origin);
    BG_GetPlayerViewDirection(ps, forward, 0, 0);
    v54[0] = ps->origin[0] + -15.0;
    v54[1] = ps->origin[1] + -15.0;
    v54[2] = ps->origin[2] + 0.0;
    v28[0] = ps->origin[0] + 15.0;
    v28[1] = ps->origin[1] + 15.0;
    v28[2] = ps->origin[2] + 70.0;
    mins[0] = origin[0] - v34;
    mins[1] = origin[1] - v35;
    mins[2] = origin[2] - v36;
    maxs[0] = origin[0] + v34;
    maxs[1] = origin[1] + v35;
    maxs[2] = origin[2] + v36;
    v49 = CM_AreaEntities(mins, maxs, entityList, 1024, 0x200000);
    v42 = 0;
    for (i = 0; i < v49; ++i)
    {
        gEnt = &g_entities[entityList[i]];
        if (ent != gEnt && (gEnt->s.eType == 3 || (gEnt->r.contents & 0x200000) != 0))
        {
            if (gEnt->classname == scr_const.trigger_use_touch)
            {
                if (gEnt->r.absmin[0] <= v28[0]
                    && v54[0] <= gEnt->r.absmax[0]
                    && gEnt->r.absmin[1] <= v28[1]
                    && v54[1] <= gEnt->r.absmax[1]
                    && gEnt->r.absmin[2] <= v28[2]
                    && v54[2] <= gEnt->r.absmax[2])
                {
                    if (SV_EntityContact(v54, v28, gEnt))
                    {
                        useList[v42].score = -256.0f;
                        useList[v42++].ent = gEnt;
                    }
                }
            }
            else
            {
                if (gEnt->s.eType != 4)
                    goto LABEL_42;
                if (ps->waterlevel < 3 && BG_GetWeaponDef(gEnt->s.weapon)->bThrowBack && !gEnt->s.lerp.u.turret.ownerNum)
                {
                    if (gEnt->parent.isDefined())
                    {
                        //ent1 = EntHandle::ent(&gEnt->parent);
                        ent1 = gEnt->parent.ent();
                        v15 = OnSameTeam(ent1, ent);
                        if ((!v15
                            || ent->client->sess.cs.team == TEAM_FREE
                            || ent1->s.clientNum == ent->s.clientNum
                            || BG_GetFriendlyFireStatus())
                            && !gEnt->tagInfo)
                        {
                            v16 = gEnt->r.currentOrigin[2] - ent->r.currentOrigin[2];
                            if (prevHintEntIndex == gEnt->s.number)
                                goto LABEL_41;
                            v14 = player_throwbackInnerRadius->current.value;
                            if ((ps->perks[1] & 0x10) != 0)
                                v14 = perk_throwbackInnerRadius->current.value;
                            v9 = ent->r.currentOrigin[0] - gEnt->r.currentOrigin[0];
                            v10 = ent->r.currentOrigin[1] - gEnt->r.currentOrigin[1];
                            if ((float)((float)(v9 * v9) + (float)(v10 * v10)) <= (float)(v14 * v14))
                            {
                            LABEL_41:
                                if ((float)((float)((float)(gEnt->s.lerp.pos.trDelta[0] * gEnt->s.lerp.pos.trDelta[0])
                                    + (float)(gEnt->s.lerp.pos.trDelta[1] * gEnt->s.lerp.pos.trDelta[1]))
                                    + (float)(gEnt->s.lerp.pos.trDelta[2] * gEnt->s.lerp.pos.trDelta[2])) <= (float)(bg_maxGrenadeIndicatorSpeed->current.value * bg_maxGrenadeIndicatorSpeed->current.value))
                                {
                                LABEL_42:
                                    eType = gEnt->s.eType;
                                    switch (eType)
                                    {
                                    case 4:
                                        v38 = player_throwbackOuterRadius->current.value;
                                        break;
                                    case 11:
                                        v38 = player_MGUseRadius->current.value;
                                        break;
                                    case 14:
                                        v38 = vehicle_useRadius->current.value;
                                        break;
                                    default:
                                        v38 = player_useRadius->current.value;
                                        break;
                                    }
                                    pos[0] = gEnt->r.absmin[0] + gEnt->r.absmax[0];
                                    pos[1] = gEnt->r.absmin[1] + gEnt->r.absmax[1];
                                    pos[2] = gEnt->r.absmin[2] + gEnt->r.absmax[2];
                                    pos[0] *= 0.5;
                                    pos[1] *= 0.5;
                                    pos[2] *= 0.5;
                                    v[0] = pos[0] - origin[0];
                                    v[1] = pos[1] - origin[1];
                                    v[2] = pos[2] - origin[2];
                                    v33 = Vec3Normalize(v);
                                    if (v33 <= v38)
                                    {
                                        if ((v18 = (float)((float)(v[0] * forward[0]) + (float)(v[1] * forward[1])) + (float)(v[2] * forward[2]),
                                            gEnt->classname != scr_const.trigger_use)
                                            && gEnt->classname != scr_const.trigger_radius
                                            && gEnt->classname != scr_const.trigger_radius_use
                                            || !gEnt->trigger.requireLookAt
                                            || v53 <= v18)
                                        {
                                            if (gEnt->classname != scr_const.trigger_radius
                                                && gEnt->classname != scr_const.trigger_radius_use)
                                            {
                                                goto LABEL_61;
                                            }
                                            team = level_bgs.clientinfo[ent->client->ps.clientNum].team;
                                            if (gEnt->team != 255 && gEnt->team == team)
                                            {
                                                v12 = 0;
                                                v11[0] = gEnt->r.currentOrigin[0] + gEnt->r.mins[0];
                                                v11[1] = gEnt->r.currentOrigin[1] + gEnt->r.mins[1];
                                                v11[2] = gEnt->r.currentOrigin[2] + gEnt->r.mins[2];
                                                v13[0] = gEnt->r.currentOrigin[0] + gEnt->r.maxs[0];
                                                v13[1] = gEnt->r.currentOrigin[1] + gEnt->r.maxs[1];
                                                v13[2] = gEnt->r.currentOrigin[2] + gEnt->r.maxs[2];
                                                ExpandBoundsToWidth(v11, v13);
                                                v12 = SV_EntityContact(v11, v13, ent);
                                                if (v12)
                                                {
                                                LABEL_61:
                                                    v41 = 1.0 - (float)((float)(v18 + 1.0) * 0.5);
                                                    useList[v42].score = v41 * v31;
                                                    if ((gEnt->flags & 0x200) != 0)
                                                    {
                                                        useList[v42].score = useList[v42].score - (float)(v31 * 0.55000001);
                                                    }
                                                    else
                                                    {
                                                        if (gEnt->s.eType == 4)
                                                            useList[v42].score = useList[v42].score - (float)(v31 * 2.0);
                                                        if (gEnt->s.eType == 14)
                                                            useList[v42].score = useList[v42].score - (float)(v31 * 2.0);
                                                        if (gEnt->classname == scr_const.trigger_radius
                                                            || gEnt->classname == scr_const.trigger_radius_use && gEnt->s.un3.item > 0)
                                                        {
                                                            useList[v42].score = useList[v42].score - (float)(v31 * 1.5);
                                                        }
                                                        if (gEnt->classname == scr_const.trigger_use || gEnt->classname == scr_const.trigger_radius)
                                                            useList[v42].score = useList[v42].score - v31;
                                                        if (gEnt->s.eType == 6)
                                                            useList[v42].score = useList[v42].score - (float)(v31 * 0.75);
                                                    }
                                                    if (gEnt->s.eType == 11)
                                                    {
                                                        if (G_IsTurretUsable(gEnt, ent))
                                                            useList[v42].score = useList[v42].score - (float)(v31 * 0.5);
                                                        else
                                                            useList[v42].score = useList[v42].score + 10000.0;
                                                    }
                                                    if (gEnt->s.eType == 3
                                                        && (!BG_CanItemBeGrabbed(&gEnt->s, &ent->client->ps, 0)
                                                            || BG_PlayerWeaponBlockPickupWeapon(&ent->client->ps, ent->client->ps.weapon)))
                                                    {
                                                        useList[v42].score = useList[v42].score + 10000.0;
                                                        ++v48;
                                                    }
                                                    useList[v42].ent = gEnt;
                                                    useList[v42].score = useList[v42].score + v33;
                                                    ++v42;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    //std::_Sort<RagdollSortStruct *, int, bool(__cdecl *)(RagdollSortStruct const &, RagdollSortStruct const &)>(
    //    (MaterialMemory *)useList,
    //    (MaterialMemory *)&useList[v42],
    //    (8 * v42) >> 3,
    //    (bool(__cdecl *)(const MaterialMemory *, const MaterialMemory *))compare_use);

    std::sort(useList, useList + v42, compare_use);

    v42 -= v48;
    v43 = 0;
    col_context_t context(17); // [esp+12E4h] [ebp-C0h] BYREF

    //col_context_t::col_context_t(&context, 17);
    context.passEntityNum0 = ps->clientNum;
    context.passEntityNum1 = ps->clientNum;
    hitNum = -1;
    for (i = 0; i < v42; ++i)
    {
        ent2 = useList[i].ent;
        if (ent2->classname != scr_const.trigger_use_touch && ent2->classname != scr_const.trigger_radius)
        {
            pos[0] = ent2->r.absmin[0] + ent2->r.absmax[0];
            pos[1] = ent2->r.absmin[1] + ent2->r.absmax[1];
            pos[2] = ent2->r.absmin[2] + ent2->r.absmax[2];
            pos[0] *= 0.5;
            pos[1] *= 0.5;
            pos[2] *= 0.5;
            if (ent2->s.eType == 14)
                pos[2] += (float)(ent2->r.maxs[2] * 0.5);
            if (ent2->s.eType == 11)
                G_DObjGetWorldTagPos(ent2, scr_const.tag_aim, pos);
            if (ent2->classname != scr_const.trigger_radius_use || ent2->s.otherEntityNum == 1023)
                context.passEntityNum1 = ent2->s.number;
            else
                context.passEntityNum1 = ent2->s.otherEntityNum;
            if (!SV_SightTracePoint(&hitNum, origin, pos, &context))
            {
                useList[i].score = useList[i].score + 10000.0;
                ++v43;
            }
        }
    }
    //std::_Sort<RagdollSortStruct *, int, bool(__cdecl *)(RagdollSortStruct const &, RagdollSortStruct const &)>(
    //    (MaterialMemory *)useList,
    //    (MaterialMemory *)&useList[v42],
    //    (8 * v42) >> 3,
    //    (bool(__cdecl *)(const MaterialMemory *, const MaterialMemory *))compare_use);

    std::sort(useList, useList + v42, compare_use);
    return v42 - v43;
}

bool __cdecl BG_PlayerWeaponBlockPickupWeapon(const playerState_s *ps, unsigned int weaponIndex)
{
    const PlayerHeldWeapon *heldWeapon; // [esp+8h] [ebp-4h]

    if ( !ps && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\bgame\\../bgame/bg_weapons.h", 394, 0, "%s", "ps") )
        __debugbreak();
    heldWeapon = BG_GetHeldWeaponConst(ps, weaponIndex);
    return heldWeapon && heldWeapon->blockWeaponPickup;
}

int __cdecl Player_GetItemCursorHint(const gclient_s *client, const gentity_s *traceEnt)
{
    const WeaponDef *weapDefItem; // [esp+Ch] [ebp-14h]
    const WeaponDef *weapDefPlayer; // [esp+14h] [ebp-Ch]
    int index; // [esp+18h] [ebp-8h]

    if ( !traceEnt
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\player_use_mp.cpp", 654, 0, "%s", "traceEnt") )
    {
        __debugbreak();
    }
    if ( !client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\player_use_mp.cpp", 655, 0, "%s", "client") )
    {
        __debugbreak();
    }
    index = traceEnt->s.un3.item;
    if ( (unsigned int)index >= 0x800
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\player_use_mp.cpp",
                    659,
                    0,
                    "%s\n\t(index) = %i",
                    "((0 <= index) && (index < (( 1 << 11 ) )))",
                    index) )
    {
        __debugbreak();
    }
    //if ( bg_itemlist[index] != IT_WEAPON
    //    && !Assert_MyHandler(
    //                "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\player_use_mp.cpp",
    //                662,
    //                0,
    //                "%s",
    //                "item->giType == IT_WEAPON") )
    //{
    //    __debugbreak();
    //}
    weapDefItem = BG_GetWeaponDef(index % 2048);
    weapDefPlayer = BG_GetWeaponDef(client->ps.weapon);
    if ( BG_PlayerHasWeapon(&client->ps, index % 2048) )
        return 0;
    if ( weapDefPlayer->inventoryType == WEAPINVENTORY_PRIMARY
        || weapDefPlayer->inventoryType == WEAPINVENTORY_ALTMODE
        || weapDefItem->inventoryType && weapDefItem->inventoryType != WEAPINVENTORY_ALTMODE
        || BG_PlayerWeaponCountPrimaryTypes(&client->ps) < 2 )
    {
        return index % 2048 + 7;
    }
    return 0;
}

void __cdecl Player_SetTurretDropHint(gentity_s *ent)
{
    gentity_s *turret; // [esp+0h] [ebp-8h]
    gclient_s *ps; // [esp+4h] [ebp-4h]

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\player_use_mp.cpp", 689, 0, "%s", "ent") )
        __debugbreak();
    if ( !ent->client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\player_use_mp.cpp", 690, 0, "%s", "ent->client") )
    {
        __debugbreak();
    }
    if ( !ent->active
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\player_use_mp.cpp", 691, 0, "%s", "ent->active") )
    {
        __debugbreak();
    }
    ps = ent->client;
    if ( (ps->ps.eFlags & 0x300) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\player_use_mp.cpp",
                    695,
                    0,
                    "%s",
                    "ps->eFlags & EF_TURRET_ACTIVE") )
    {
        __debugbreak();
    }
    if ( ps->ps.viewlocked_entNum == 1023
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\player_use_mp.cpp",
                    696,
                    0,
                    "%s",
                    "ps->viewlocked_entNum != ENTITYNUM_NONE") )
    {
        __debugbreak();
    }
    turret = &level.gentities[ps->ps.viewlocked_entNum];
    if ( turret->s.eType != 11
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\player_use_mp.cpp",
                    699,
                    0,
                    "%s",
                    "turret->s.eType == ET_MG42") )
    {
        __debugbreak();
    }
    if ( *BG_GetWeaponDef(turret->s.weapon)->dropHintString )
    {
        ps->ps.cursorHintEntIndex = 1023;
        ps->ps.cursorHint = turret->s.weapon + 7;
        ps->ps.cursorHintString = BG_GetWeaponDef(turret->s.weapon)->dropHintStringIndex;
    }
}

