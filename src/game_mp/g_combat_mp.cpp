#include "g_combat_mp.h"
#include "g_main_mp.h"
#include <clientscript/cscr_stringlist.h>
#include <universal/com_loadutils.h>
#include <clientscript/cscr_vm.h>
#include "actor_mp.h"
#include <server_mp/sv_init_mp.h>
#include <qcommon/dobj_management.h>
#include <game/turret.h>
#include "g_misc_mp.h"
#include <clientscript/scr_const.h>
#include "g_spawn_mp.h"
#include "g_active_mp.h"
#include <server/sv_world.h>
#include "g_utils_mp.h"
#include <bgame/bg_misc.h>
#include <game/g_missile.h>
#include <bgame/bg_perks.h>
#include <game/actor_script_cmd.h>
#include <client_mp/g_client_mp.h>
#include <cgame/cg_drawtools.h>
#include <qcommon/cm_world.h>
#include <universal/com_math_anglevectors.h>
#include <game/g_weapon.h>
#include <DynEntity/DynEntity_server.h>
#include <glass/glass_server.h>

unsigned char bulletPriorityMap[20] =
{
  1u,
  3u,
  3u,
  3u,
  3u,
  3u,
  3u,
  3u,
  3u,
  3u,
  3u,
  3u,
  3u,
  3u,
  3u,
  3u,
  3u,
  3u,
  0u,
  0u
};

const char *g_HitLocNames[19] =
{
  "none",
  "helmet",
  "head",
  "neck",
  "torso_upper",
  "torso_lower",
  "right_arm_upper",
  "left_arm_upper",
  "right_arm_lower",
  "left_arm_lower",
  "right_hand",
  "left_hand",
  "right_leg_upper",
  "left_leg_upper",
  "right_leg_lower",
  "left_leg_lower",
  "right_foot",
  "left_foot",
  "gun"
};

unsigned __int16 g_HitLocConstNames[19];
float g_fHitLocDamageMult[19];

void __cdecl G_ParseHitLocDmgTable()
{
    unsigned __int16 v0; // ax
    char *pszBuffer; // [esp+4h] [ebp-40F4h]
    cspField_t pFieldList[19]; // [esp+8h] [ebp-40F0h] BYREF
    char loadBuffer[16384]; // [esp+F0h] [ebp-4008h] BYREF
    int i; // [esp+40F4h] [ebp-4h]

    for (i = 0; i < 19; ++i)
    {
        g_fHitLocDamageMult[i] = 1.0f;
        pFieldList[i].szName = g_HitLocNames[i];
        pFieldList[i].iOffset = 4 * i;
        pFieldList[i].iFieldType = 7;
        v0 = Scr_AllocString((char *)g_HitLocNames[i], 1, SCRIPTINSTANCE_SERVER);
        g_HitLocConstNames[i] = v0;
    }
    g_fHitLocDamageMult[18] = 0.0f;
    pszBuffer = Com_LoadInfoString((char*)"info/mp_lochit_dmgtable", "hitloc damage table", "LOCDMGTABLE", loadBuffer);
    if (!ParseConfigStringToStruct(
        (unsigned __int8 *)g_fHitLocDamageMult,
        pFieldList,
        19,
        pszBuffer,
        0,
        0,
        BG_StringCopy))
        Com_Error(ERR_DROP, "Error parsing hitloc damage table %s", "info/mp_lochit_dmgtable");
}

void __cdecl LookAtKiller(gentity_s *self, gentity_s *inflictor, gentity_s *attacker)
{
    float dir[3]; // [esp+10h] [ebp-18h] BYREF

    if ( attacker && attacker != self )
    {
        dir[0] = attacker->r.currentOrigin[0] - self->r.currentOrigin[0];
        dir[1] = attacker->r.currentOrigin[1] - self->r.currentOrigin[1];
        dir[2] = attacker->r.currentOrigin[2] - self->r.currentOrigin[2];
LABEL_11:
        self->client->ps.stats[1] = (int)vectoyaw(dir);
        vectoyaw(dir);
        return;
    }
    if ( inflictor && inflictor != self )
    {
        dir[0] = inflictor->r.currentOrigin[0] - self->r.currentOrigin[0];
        dir[1] = inflictor->r.currentOrigin[1] - self->r.currentOrigin[1];
        dir[2] = inflictor->r.currentOrigin[2] - self->r.currentOrigin[2];
        goto LABEL_11;
    }
    if ( !self->client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_combat_mp.cpp", 199, 0, "%s", "self->client") )
    {
        __debugbreak();
    }
    self->client->ps.stats[1] = (int)self->r.currentAngles[1];
}

int __cdecl G_MeansOfDeathFromScriptParam(unsigned int scrParam)
{
    char *v2; // eax
    const char *v3; // eax
    unsigned __int16 modName; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    modName = (unsigned __int16)Scr_GetConstString(scrParam, SCRIPTINSTANCE_SERVER);
    for ( i = 0; i < 21; ++i )
    {
        if ( *modNames[i] == modName )
            return i;
    }
    v2 = SL_ConvertToString(modName, SCRIPTINSTANCE_SERVER);
    v3 = va("Unknown means of death \"%s\"\n", v2);
    Scr_ParamError(scrParam, v3, SCRIPTINSTANCE_SERVER);
    return 0;
}

void __cdecl player_die(
                gentity_s *self,
                gentity_s *inflictor,
                gentity_s *attacker,
                int damage,
                int meansOfDeath,
                const int iWeapon,
                const float *vDir,
                const hitLocation_t hitLoc,
                int psTimeOffset)
{
    float *viewangles; // [esp+4h] [ebp-2Ch]
    float *velocity; // [esp+Ch] [ebp-24h]
    gentity_s *turret; // [esp+14h] [ebp-1Ch]
    const char *weaponName; // [esp+18h] [ebp-18h]
    int deathAnimDuration; // [esp+1Ch] [ebp-14h]
    int i; // [esp+24h] [ebp-Ch]
    const WeaponDef *weapDef; // [esp+28h] [ebp-8h]
    unsigned int isFlared; // [esp+2Ch] [ebp-4h]
    int savedregs; // [esp+30h] [ebp+0h] BYREF

    SV_CheckThread();
    if ( !self->client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_combat_mp.cpp", 348, 0, "%s", "self->client") )
    {
        __debugbreak();
    }
    if ( (self->client->ps.pm_flags & 0x20000) != 0 )
    {
        velocity = self->client->ps.velocity;
        *velocity = 0.0f;
        velocity[1] = 0.0f;
    }
    weaponName = "none";
    weapDef = BG_GetWeaponDef(iWeapon);
    if ( weapDef )
        weaponName = (char *)BG_WeaponName(iWeapon);
    if ( Com_GetServerDObj(self->client->ps.clientNum)
        && (self->client->ps.pm_type < 2u
         || self->client->ps.pm_type == 6
         || self->client->ps.pm_type == 7
         || self->client->ps.pm_type == 8)
        && (self->client->ps.otherFlags & 2) == 0 )
    {
        iassert(bgs == &level_bgs);

        if ( (self->client->ps.eFlags & 0x300) != 0 )
        {
            if ( self->client->ps.viewlocked_entNum == 1023
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_combat_mp.cpp",
                            390,
                            0,
                            "%s",
                            "self->client->ps.viewlocked_entNum != ENTITYNUM_NONE") )
            {
                __debugbreak();
            }
            if ( level.gentities[self->client->ps.viewlocked_entNum].s.eType != 11
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_combat_mp.cpp",
                            391,
                            0,
                            "%s",
                            "level.gentities[self->client->ps.viewlocked_entNum].s.eType == ET_MG42") )
            {
                __debugbreak();
            }
            turret = &level.gentities[self->client->ps.viewlocked_entNum];
            if ( turret->pTurretInfo )
                G_ClientStopUsingTurret(turret);
        }
        if ( attacker->s.eType == 11 && attacker->r.ownerNum.isDefined() )
            attacker = attacker->r.ownerNum.ent();
        ScrNotify_FaceEvent(self, scr_const.death);
        isFlared = self->client->ps.visionSetLerpRatio > 0.001;
        Scr_AddBool(self->client->ps.poisoned, SCRIPTINSTANCE_SERVER);
        Scr_AddBool(isFlared, SCRIPTINSTANCE_SERVER);
        Scr_AddEntity(attacker, SCRIPTINSTANCE_SERVER);
        Scr_Notify(self, scr_const.flared_or_tabuned_death, 3u);
        Scr_AddString(weaponName, SCRIPTINSTANCE_SERVER);
        Scr_AddBool(meansOfDeath == 9, SCRIPTINSTANCE_SERVER);
        Scr_AddEntity(attacker, SCRIPTINSTANCE_SERVER);
        Scr_Notify(self, scr_const.death, 3u);
        switch ( meansOfDeath )
        {
            case 0x11u:
                self->client->ps.dmgType = 1;
                break;
            case 3u:
            case 4u:
            case 5u:
            case 6u:
            case 0xFu:
                self->client->ps.dmgType = 4;
                break;
            case 7u:
            case 8u:
                self->client->ps.dmgType = 2;
                break;
            case 9u:
                self->client->ps.dmgType = 3;
                break;
            case 0xDu:
                if ( weapDef->weapType == WEAPTYPE_GRENADE )
                    self->client->ps.dmgType = 4;
                else
                    self->client->ps.dmgType = 5;
                break;
            default:
                self->client->ps.dmgType = 0;
                break;
        }
        if ( self->client->ps.pm_type >= 2u
            && self->client->ps.pm_type != 6
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_combat_mp.cpp",
                        455,
                        1,
                        "%s\n\t(self->client->ps.pm_type) = %i",
                        "(self->client->ps.pm_type == PM_NORMAL_LINKED || self->client->ps.pm_type == PM_NORMAL || self->client->ps.p"
                        "m_type == PM_LASTSTAND)",
                        self->client->ps.pm_type) )
        {
            __debugbreak();
        }
        deathAnimDuration = BG_AnimScriptEvent(&g_pmove[self->client->ps.clientNum], ANIM_ET_DEATH, 0, 1);
        self->client->ps.pm_type = 10 - (self->client->ps.pm_type != 1);
        self->client->ps.stats[0] = 0;
        Scr_PlayerKilled(
            self,
            inflictor,
            attacker,
            damage,
            meansOfDeath,
            iWeapon,
            (float*)vDir,
            hitLoc,
            psTimeOffset,
            deathAnimDuration);
        DeathGrenadeDrop(self);
        for ( i = 0; i < level.maxclients; ++i )
            ;
        self->takedamage = 1;
        self->r.contents = 0x4000000;
        if ( (self->client->ps.eFlags2 & 0x10000000) == 0 )
        {
            self->r.currentAngles[2] = 0.0f;
            LookAtKiller(self, inflictor, attacker);
            viewangles = self->client->ps.viewangles;
            *viewangles = self->r.currentAngles[0];
            viewangles[1] = self->r.currentAngles[1];
            viewangles[2] = self->r.currentAngles[2];
            self->s.loopSoundId = 0;
            self->s.loopSoundFade = 0;
            Sentient_Dissociate(self->sentient);
            if ( (self->client->ps.eFlags & 0x4000) != 0 )
                VEH_UnlinkPlayer(self, 0, (char*)"player_die");
            SV_UnlinkEntity(self);
            self->r.maxs[2] = 30.0f;
            if ( self->r.maxs[2] < self->r.mins[2]
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_combat_mp.cpp",
                            509,
                            0,
                            "%s",
                            "self->r.maxs[2] >= self->r.mins[2]") )
            {
                __debugbreak();
            }
            SV_LinkEntity(self);
            self->health = 0;
            self->handler = 15;
            iassert(bgs == &level_bgs);
        }
    }
}

void __cdecl DeathGrenadeDrop(gentity_s *self)
{
    unsigned __int8 PlayerWeaponModel; // al
    unsigned __int8 v2; // al
    gentity_s *v3; // edx
    int grenadeTimeLeft; // [esp-4h] [ebp-5Ch]
    int fuseTime; // [esp-4h] [ebp-5Ch]
    const WeaponDef *weapDef2; // [esp+34h] [ebp-24h]
    unsigned int grenadeWeaponIndex; // [esp+38h] [ebp-20h]
    int grenadeWeaponIndexa; // [esp+38h] [ebp-20h]
    float launchvel[3]; // [esp+3Ch] [ebp-1Ch] BYREF
    float launchspot[3]; // [esp+48h] [ebp-10h] BYREF
    const WeaponDef *weapDef; // [esp+54h] [ebp-4h]

    if (self->client->ps.grenadeTimeLeft && (self->client->ps.eFlags & 0x4000) == 0)
    {
        if ((self->client->ps.weapFlags & 2) != 0)
            grenadeWeaponIndex = self->client->ps.offHandIndex;
        else
            grenadeWeaponIndex = self->client->ps.weapon;
        launchvel[0] = G_crandom();
        launchvel[1] = G_crandom();
        launchvel[2] = G_crandom();
        launchvel[0] = 160.0 * launchvel[0];
        launchvel[1] = 160.0 * launchvel[1];
        launchvel[2] = 160.0 * launchvel[2];
        launchspot[0] = self->r.currentOrigin[0];
        launchspot[1] = self->r.currentOrigin[1];
        launchspot[2] = self->r.currentOrigin[2];
        weapDef = BG_GetWeaponDef(grenadeWeaponIndex);
        if (weapDef->stickiness != WEAPSTICKINESS_ALL)
            launchspot[2] = launchspot[2] + 40.0;
        if (weapDef->offhandSlot != OFFHAND_SLOT_EQUIPMENT)
        {
            grenadeTimeLeft = self->client->ps.grenadeTimeLeft;
            PlayerWeaponModel = BG_GetPlayerWeaponModel(&self->client->ps, grenadeWeaponIndex);
            G_FireGrenade(self, launchspot, launchvel, grenadeWeaponIndex, PlayerWeaponModel, 1, grenadeTimeLeft);
        }
    }
    if ((self->client->ps.weapFlags & 0x40000) != 0
        && (self->client->ps.perks[0] & 0x4000000) != 0
        && ((self->client->ps.eFlags & 0x4000) == 0 || self->client->ps.vehicleType == 6))
    {
        grenadeWeaponIndexa = BG_FindWeaponIndexForName(perk_grenadeDeath->current.string);
        if (grenadeWeaponIndexa)
        {
            weapDef2 = BG_GetWeaponDef(grenadeWeaponIndexa);
            launchvel[0] = G_crandom();
            launchvel[1] = G_crandom();
            launchvel[2] = G_crandom();
            launchvel[0] = 160.0 * launchvel[0];
            launchvel[1] = 160.0 * launchvel[1];
            launchvel[2] = 160.0 * launchvel[2];
            launchspot[0] = self->r.currentOrigin[0];
            launchspot[1] = self->r.currentOrigin[1];
            launchspot[2] = self->r.currentOrigin[2];
            launchspot[2] = launchspot[2] + 40.0;
            fuseTime = weapDef2->fuseTime;
            v2 = BG_GetPlayerWeaponModel(&self->client->ps, grenadeWeaponIndexa);
            v3 = G_FireGrenade(self, launchspot, launchvel, grenadeWeaponIndexa, v2, 1, fuseTime);
            v3->flags |= 0x8000u;
        }
        else
        {
            Com_PrintWarning(14, "Unknown perk_grenadeDeath grenade: %s\n", perk_grenadeDeath->current.string);
        }
    }
}

double __cdecl G_GetWeaponHitLocationMultiplier(hitLocation_t hitLoc, unsigned int weapon)
{
    const WeaponDef *weapDef; // [esp+0h] [ebp-4h]

    if ( (unsigned int)hitLoc > HITLOC_GUN
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_combat_mp.cpp",
                    524,
                    0,
                    "%s",
                    "(hitLoc >= HITLOC_NONE) && (hitLoc < HITLOC_NUM)") )
    {
        __debugbreak();
    }
    if ( !weapon )
        return g_fHitLocDamageMult[hitLoc];
    weapDef = BG_GetWeaponDef(weapon);
    if ( weapDef
        && (weapDef->weapType == WEAPTYPE_BULLET
         || weapDef->weapType == WEAPTYPE_PROJECTILE
         || weapDef->weapType == WEAPTYPE_GRENADE) )
    {
        return weapDef->locationDamageMultipliers[hitLoc];
    }
    else
    {
        return g_fHitLocDamageMult[hitLoc];
    }
}

void __cdecl G_DamageClient(
                gentity_s *targ,
                gentity_s *inflictor,
                gentity_s *attacker,
                float *dir,
                float *point,
                int damage,
                int dflags,
                unsigned int mod,
                unsigned int weapon,
                hitLocation_t hitLoc,
                int timeOffset)
{
    unsigned int NumWeapons; // eax
    float v12; // [esp+0h] [ebp-4h]

    if ( targ->takedamage
        && damage > 0
        && (targ->client->flags & 3) == 0
        && targ->client->sess.connected == CON_CONNECTED
        && targ->client->ps.pm_type != 9
        && (targ->client->ps.otherFlags & 1) == 0 )
    {
        if ( weapon == -1 )
        {
            if ( inflictor )
            {
                weapon = G_GetWeaponIndexForEntity(inflictor);
            }
            else if ( attacker )
            {
                weapon = G_GetWeaponIndexForEntity(attacker);
            }
            else
            {
                weapon = 0;
            }
        }
        if ( weapon >= BG_GetNumWeapons() )
        {
            NumWeapons = BG_GetNumWeapons();
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_combat_mp.cpp",
                            618,
                            0,
                            "weapon doesn't index BG_GetNumWeapons()\n\t%i not in [0, %i)",
                            weapon,
                            NumWeapons) )
                __debugbreak();
        }
        if ( (unsigned int)hitLoc > HITLOC_GUN
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_combat_mp.cpp",
                        621,
                        0,
                        "%s",
                        "(hitLoc >= HITLOC_NONE) && (hitLoc < HITLOC_NUM)") )
        {
            __debugbreak();
        }
        if ( mod != 7 )
        {
            v12 = (float)damage;
            damage = (int)(G_GetWeaponHitLocationMultiplier(hitLoc, weapon) * v12);
        }
        if ( damage <= 0 )
            damage = 1;
        Scr_PlayerDamage(targ, inflictor, attacker, damage, dflags, mod, weapon, point, dir, hitLoc, timeOffset);
    }
}

unsigned int __cdecl G_GetWeaponIndexForEntity(const gentity_s *ent)
{
    gclient_s *client; // [esp+8h] [ebp-4h]

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_combat_mp.cpp", 544, 0, "%s", "ent") )
        __debugbreak();
    client = ent->client;
    if ( !client )
        return ent->s.weapon;
    if ( (client->ps.eFlags & 0x300) == 0 )
        return BG_GetViewmodelWeaponIndex(&client->ps);
    if ( client->ps.viewlocked == PLAYERVIEWLOCK_NONE
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_combat_mp.cpp", 558, 0, "%s", "ps->viewlocked") )
    {
        __debugbreak();
    }
    if ( client->ps.viewlocked_entNum == 1023
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_combat_mp.cpp",
                    559,
                    0,
                    "%s",
                    "ps->viewlocked_entNum != ENTITYNUM_NONE") )
    {
        __debugbreak();
    }
    return g_entities[client->ps.viewlocked_entNum].s.weapon;
}

void __cdecl G_DamageActor(
                gentity_s *targ,
                gentity_s *inflictor,
                gentity_s *attacker,
                float *dir,
                float *point,
                int damage,
                int dflags,
                unsigned int mod,
                unsigned int weapon,
                hitLocation_t hitLoc,
                int timeOffset)
{
    unsigned int NumWeapons; // eax
    float v12; // [esp+0h] [ebp-4h]

    if ( targ->takedamage && damage > 0 && targ->actor->Physics.bIsAlive )
    {
        if ( weapon == -1 )
        {
            if ( inflictor )
            {
                weapon = G_GetWeaponIndexForEntity(inflictor);
            }
            else if ( attacker )
            {
                weapon = G_GetWeaponIndexForEntity(attacker);
            }
            else
            {
                weapon = 0;
            }
        }
        if ( weapon >= BG_GetNumWeapons() )
        {
            NumWeapons = BG_GetNumWeapons();
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_combat_mp.cpp",
                            678,
                            0,
                            "weapon doesn't index BG_GetNumWeapons()\n\t%i not in [0, %i)",
                            weapon,
                            NumWeapons) )
                __debugbreak();
        }
        if ( (unsigned int)hitLoc > HITLOC_GUN
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_combat_mp.cpp",
                        681,
                        0,
                        "%s",
                        "(hitLoc >= HITLOC_NONE) && (hitLoc < HITLOC_NUM)") )
        {
            __debugbreak();
        }
        if ( mod != 7 )
        {
            v12 = (float)damage;
            damage = (int)(G_GetWeaponHitLocationMultiplier(hitLoc, weapon) * v12);
        }
        if ( damage <= 0 )
            damage = 1;
        Scr_ActorDamage(targ, inflictor, attacker, damage, dflags, mod, weapon, point, dir, hitLoc, timeOffset);
    }
}

void __cdecl G_DamageVehicle(
                gentity_s *targ,
                gentity_s *inflictor,
                gentity_s *attacker,
                float *dir,
                float *point,
                int damage,
                int dflags,
                int mod,
                unsigned int weapon,
                hitLocation_t hitLoc,
                int timeOffset,
                unsigned int modelIndex,
                unsigned int partName)
{
    unsigned int NumWeapons; // eax
    int damageFromUnderneath; // [esp+0h] [ebp-4h] BYREF
    int savedregs; // [esp+4h] [ebp+0h] BYREF

    damageFromUnderneath = 0;
    if ( targ->takedamage && damage > 0 && targ->health > 0 )
    {
        if ( weapon == -1 )
        {
            if ( inflictor )
            {
                weapon = G_GetWeaponIndexForEntity(inflictor);
            }
            else if ( attacker )
            {
                weapon = G_GetWeaponIndexForEntity(attacker);
            }
            else
            {
                weapon = 0;
            }
        }
        if ( weapon >= BG_GetNumWeapons() )
        {
            NumWeapons = BG_GetNumWeapons();
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_combat_mp.cpp",
                            725,
                            0,
                            "weapon doesn't index BG_GetNumWeapons()\n\t%i not in [0, %i)",
                            weapon,
                            NumWeapons) )
                __debugbreak();
        }
        Scr_Vehicle_DamageScale(
            targ,
            attacker,
            inflictor,
            point,
            mod,
            weapon,
            &damageFromUnderneath);
        Scr_VehicleDamage(
            targ,
            inflictor,
            attacker,
            damage,
            dflags,
            mod,
            weapon,
            point,
            dir,
            hitLoc,
            timeOffset,
            damageFromUnderneath,
            modelIndex,
            partName);
    }
}

void __cdecl G_DamageNotify(
                unsigned __int16 notify,
                gentity_s *targ,
                gentity_s *attacker,
                float *dir,
                float *point,
                int damage,
                int mod,
                int dFlags,
                unsigned int modelIndex,
                unsigned int partName,
                char *weaponName)
{
    unsigned int modelName; // [esp+0h] [ebp-4h]

    Scr_AddInt(dFlags, SCRIPTINSTANCE_SERVER);
    Scr_AddString(weaponName, SCRIPTINSTANCE_SERVER);
    if ( partName )
        Scr_AddConstString(partName, SCRIPTINSTANCE_SERVER);
    else
        Scr_AddString((char *)"", SCRIPTINSTANCE_SERVER);
    if ( !modelIndex || targ->destructible )
    {
        Scr_AddString((char *)"", SCRIPTINSTANCE_SERVER);
        Scr_AddString((char *)"", SCRIPTINSTANCE_SERVER);
    }
    else
    {
        if ( !targ->attachModelNames[modelIndex + 18]
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_combat_mp.cpp",
                        756,
                        0,
                        "%s",
                        "targ->attachTagNames[modelIndex - 1]") )
        {
            __debugbreak();
        }
        modelName = SV_GetConfigstringConst(*((unsigned __int16 *)&targ->tagChildren + modelIndex + 1) + 1568);
        if ( !modelName
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_combat_mp.cpp", 759, 1, "%s", "modelName") )
        {
            __debugbreak();
        }
        Scr_AddConstString(targ->attachModelNames[modelIndex + 18], SCRIPTINSTANCE_SERVER);
        Scr_AddConstString(modelName, SCRIPTINSTANCE_SERVER);
    }
    Scr_AddConstString(*modNames[mod], SCRIPTINSTANCE_SERVER);
    if ( point )
        Scr_AddVector(point, SCRIPTINSTANCE_SERVER);
    else
        Scr_AddVector((float *)vec3_origin, SCRIPTINSTANCE_SERVER);
    if ( dir )
        Scr_AddVector(dir, SCRIPTINSTANCE_SERVER);
    else
        Scr_AddVector((float *)vec3_origin, SCRIPTINSTANCE_SERVER);
    Scr_AddEntity(attacker, SCRIPTINSTANCE_SERVER);
    Scr_AddInt(damage, SCRIPTINSTANCE_SERVER);
    Scr_Notify(targ, notify, 0xAu);
}

void __cdecl G_Damage(
                gentity_s *targ,
                gentity_s *inflictor,
                gentity_s *attacker,
                const float *dir,
                const float *point,
                int damage,
                int dFlags,
                int mod,
                unsigned int weapon,
                hitLocation_t hitLoc,
                unsigned int modelIndex,
                unsigned int partName,
                int timeOffset)
{
    unsigned int NumWeapons; // eax
    const char *v14; // eax
    float localdir[3]; // [esp+Ch] [ebp-14h] BYREF
    void (__cdecl *die)(gentity_s *, gentity_s *, gentity_s *, int, int, const int, const float *, const hitLocation_t, int); // [esp+18h] [ebp-8h]
    void (__cdecl *pain)(gentity_s *, gentity_s *, int, const float *, const int, const float *, const hitLocation_t, const int); // [esp+1Ch] [ebp-4h]

    if ( targ->client )
    {
        G_DamageClient(targ, inflictor, attacker, (float*)dir, (float *)point, damage, dFlags, mod, weapon, hitLoc, timeOffset);
    }
    else if ( targ->actor )
    {
        G_DamageActor(targ, inflictor, attacker, (float *)dir, (float *)point, damage, dFlags, mod, weapon, hitLoc, timeOffset);
    }
    else if ( targ->scr_vehicle )
    {
        G_DamageVehicle(
            targ,
            inflictor,
            attacker,
            (float*)dir,
            (float*)point,
            damage,
            dFlags,
            mod,
            weapon,
            hitLoc,
            timeOffset,
            modelIndex,
            partName);
    }
    else if ( targ->takedamage )
    {
        if ( !inflictor )
            inflictor = &g_entities[1022];
        if ( !attacker )
            attacker = &g_entities[1022];
        if ( weapon == -1 )
            weapon = G_GetWeaponIndexForEntity(inflictor);
        if ( weapon >= BG_GetNumWeapons() )
        {
            NumWeapons = BG_GetNumWeapons();
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_combat_mp.cpp",
                            849,
                            0,
                            "weapon doesn't index BG_GetNumWeapons()\n\t%i not in [0, %i)",
                            weapon,
                            NumWeapons) )
                __debugbreak();
        }
        if ( !targ->scr_vehicle || !G_IsVehicleImmune(targ, mod, dFlags, weapon) )
        {
            if ( !targ->r.inuse
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_combat_mp.cpp",
                            857,
                            0,
                            "%s\n\t(targ->s.number) = %i",
                            "(targ->r.inuse)",
                            targ->s.number) )
            {
                __debugbreak();
            }
            if ( !attacker->r.inuse
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_combat_mp.cpp",
                            858,
                            0,
                            "%s\n\t(attacker->s.number) = %i",
                            "(attacker->r.inuse)",
                            attacker->s.number) )
            {
                __debugbreak();
            }
            Vec3NormalizeTo(dir, localdir);
            if ( (targ->flags & 1) == 0 )
            {
                if ( damage < 1 )
                    damage = 1;
                if ( (targ->flags & 2) != 0 && targ->health - damage <= 0 )
                    damage = targ->health - 1;
                if ( g_debugDamage->current.enabled )
                    Com_Printf(15, "target:%i health:%i damage:%i\n", targ->s.number, targ->health, damage);
                if ( damage && (dFlags & 1) == 0 && targ->destructible )
                {
                    if ( (dFlags & 0x10) != 0 )
                        DestructibleDamage(targ, attacker, dir, point, damage, mod, modelIndex, partName);
                    else
                        damage = DestructibleDamage(targ, attacker, dir, point, damage, mod, modelIndex, partName);
                }
                if ( targ->classname != scr_const.trigger_damage )
                    targ->health -= damage;
                v14 = BG_WeaponName(weapon);
                G_DamageNotify(scr_const.damage, targ, attacker, (float *)dir, (float *)point, damage, mod, dFlags, modelIndex, partName, (char*)v14);
                if ( targ->health > 0 )
                {
                    pain = entityHandlers[targ->handler].pain;
                    if ( pain )
                        pain(targ, attacker, damage, point, mod, localdir, hitLoc, weapon);
                }
                else
                {
                    if ( targ->health < -999 )
                        targ->health = -999;
                    ScrNotify_FaceEvent(targ, scr_const.death);
                    Scr_AddEntity(attacker, SCRIPTINSTANCE_SERVER);
                    Scr_Notify(targ, scr_const.death, 1u);
                    die = entityHandlers[targ->handler].die;
                    if ( die )
                        die(targ, inflictor, attacker, damage, mod, weapon, localdir, hitLoc, timeOffset);
                }
            }
        }
    }
}

double __cdecl CanDamage(
                gentity_s *targ,
                gentity_s *inflictor,
                const float *centerPos,
                float coneAngleCos,
                float *coneDirection,
                int contentMask)
{
    float v7[3]; // [esp+E4h] [ebp-134h] BYREF
    float v10[3]; // [esp+F0h] [ebp-128h] BYREF
    char success_1; // [esp+FFh] [ebp-119h]
    float *color_; // [esp+100h] [ebp-118h]
    float centerToCorner[3]; // [esp+104h] [ebp-114h]
    DObj *obj; // [esp+110h] [ebp-108h]
    float radiusUp; // [esp+114h] [ebp-104h]
    float absMaxs[3]; // [esp+118h] [ebp-100h] BYREF
    float v1[3]; // [esp+124h] [ebp-F4h] BYREF
    float v0[3]; // [esp+130h] [ebp-E8h] BYREF
    float up[3]; // [esp+13Ch] [ebp-DCh] BYREF
    float radiusRight; // [esp+148h] [ebp-D0h]
    float absMins[3]; // [esp+14Ch] [ebp-CCh] BYREF
    float v[3]; // [esp+158h] [ebp-C0h] BYREF
    float dir[3]; // [esp+164h] [ebp-B4h] BYREF
    bool success; // [esp+173h] [ebp-A5h]
    const float *color; // [esp+174h] [ebp-A4h]
    float halfWidth; // [esp+178h] [ebp-A0h]
    float right[3]; // [esp+17Ch] [ebp-9Ch]
    float forward[3]; // [esp+188h] [ebp-90h] BYREF
    float eyeOrigin[3]; // [esp+194h] [ebp-84h] BYREF
    float halfHeight; // [esp+1A0h] [ebp-78h]
    int hits; // [esp+1A4h] [ebp-74h]
    int hitnum; // [esp+1A8h] [ebp-70h] BYREF
    int inflictorNum; // [esp+1D4h] [ebp-44h]
    float dest[5][3]; // [esp+1D8h] [ebp-40h] BYREF
    int i; // [esp+214h] [ebp-4h]

    if ( inflictor )
    {
        if ( inflictor->s.eType == 4
            && inflictor->tagInfo
            && inflictor->tagInfo->parent
            && inflictor->tagInfo->parent == targ )
        {
            return 1.0;
        }
        inflictorNum = inflictor->s.number;
    }
    else
    {
        inflictorNum = 1023;
    }

    //col_context_t::col_context_t(&context, contentMask);
    col_context_t context(contentMask); // [esp+1ACh] [ebp-6Ch] BYREF

    context.init_locational(targ->s.number, inflictorNum);
    //col_context_t::init_locational(&context, targ->s.number, inflictorNum);

    hitnum = -1;
    if ( targ->client )
    {
        halfWidth = 15.0f;
        G_GetPlayerBodyViewOrigin(&targ->client->ps, eyeOrigin);

        halfHeight = (float)(eyeOrigin[2] - targ->r.currentOrigin[2]) * 0.5;

        forward[0] = *centerPos - targ->r.currentOrigin[0];
        forward[1] = centerPos[1] - targ->r.currentOrigin[1];
        forward[2] = 0.0f;

        Vec3Normalize(forward);
        right[0] = -forward[1];
        right[1] = forward[0];
        right[2] = forward[2];
        dest[0][0] = eyeOrigin[0] + targ->r.currentOrigin[0];
        dest[0][1] = eyeOrigin[1] + targ->r.currentOrigin[1];
        dest[0][2] = eyeOrigin[2] + targ->r.currentOrigin[2];

        dest[0][0] = 0.5 * dest[0][0];
        dest[0][1] = 0.5 * dest[0][1];
        dest[0][2] = 0.5 * dest[0][2];

        dest[1][0] = (float)(15.0 * (-(forward[1]))) + dest[0][0];
        dest[1][1] = (float)(15.0 * forward[0]) + dest[0][1];
        dest[1][2] = (float)(15.0 * forward[2]) + dest[0][2];

        dest[1][2] = dest[1][2] + halfHeight;

        dest[2][0] = dest[1][0];
        dest[2][1] = dest[1][1];

        dest[2][2] = (float)(15.0 * forward[2]) + dest[0][2];
        dest[2][2] = dest[2][2] - halfHeight;

        dest[3][0] = (float)((-(15.0f)) * (-(forward[1]))) + dest[0][0];
        dest[3][1] = (float)((-(15.0f)) * forward[0]) + dest[0][1];
        dest[3][2] = (float)((-(15.0f)) * forward[2]) + dest[0][2];
        dest[3][2] = dest[3][2] + halfHeight;

        dest[4][0] = dest[3][0];
        dest[4][1] = dest[3][1];
        dest[4][2] = (float)((-(15.0f)) * forward[2]) + dest[0][2];
        dest[4][2] = dest[4][2] - halfHeight;
        if ( radius_damage_debug->current.enabled )
        {
            for ( i = 0; i < 5; ++i )
            {
                success = 1;
                color = colorWhite;
                if ( coneAngleCos != -1.0 )
                {
                    if ( coneDirection )
                    {
                        dir[0] = dest[i][0] - *centerPos;
                        dir[1] = dest[i][1] - centerPos[1];
                        dir[2] = dest[i][2] - centerPos[2];
                        Vec3Normalize(dir);
                        if ( coneAngleCos > (float)((float)((float)(dir[0] * *coneDirection) + (float)(dir[1] * coneDirection[1]))
                                                                            + (float)(dir[2] * coneDirection[2])) )
                        {
                            success = 0;
                            color = colorOrange;
                        }
                    }
                }
                if ( success && !SV_SightTracePoint(&hitnum, centerPos, dest[i], &context) )
                    color = colorRed;
                CG_DebugLine(centerPos, dest[i], color, 1, 300);
            }
        }
        hits = 0;
        for ( i = 0; i < 5; ++i )
        {
            if ( coneAngleCos != -1.0 )
            {
                if ( coneDirection )
                {
                    v[0] = dest[i][0] - *centerPos;
                    v[1] = dest[i][1] - centerPos[1];
                    v[2] = dest[i][2] - centerPos[2];
                    Vec3Normalize(v);
                    if ( coneAngleCos > (float)((float)((float)(v[0] * *coneDirection) + (float)(v[1] * coneDirection[1]))
                                                                        + (float)(v[2] * coneDirection[2])) )
                        continue;
                }
            }
            if ( SV_SightTracePoint(&hitnum, centerPos, dest[i], &context) )
                ++hits;
        }
        if ( hits )
        {
            if ( hits <= 3 )
                return (double)hits / 3.0;
            else
                return 1.0;
        }
        else
        {
            return 0.0;
        }
    }
    else
    {
        if ( targ->classname == scr_const.script_model && targ->model )
        {
            obj = Com_GetServerDObj(targ->s.number);
            DObjPhysicsGetBounds(obj, absMins, absMaxs);
            absMins[0] = targ->r.currentOrigin[0] + absMins[0];
            absMins[1] = targ->r.currentOrigin[1] + absMins[1];
            absMins[2] = targ->r.currentOrigin[2] + absMins[2];
            absMaxs[0] = targ->r.currentOrigin[0] + absMaxs[0];
            absMaxs[1] = targ->r.currentOrigin[1] + absMaxs[1];
            absMaxs[2] = targ->r.currentOrigin[2] + absMaxs[2];
        }
        else
        {
            absMins[0] = targ->r.absmin[0];
            absMins[1] = targ->r.absmin[1];
            absMins[2] = targ->r.absmin[2];
            absMaxs[0] = targ->r.absmax[0];
            absMaxs[1] = targ->r.absmax[1];
            absMaxs[2] = targ->r.absmax[2];
        }
        dest[0][0] = 0.5 * (float)(absMins[0] + absMaxs[0]);
        dest[0][1] = 0.5 * (float)(absMins[1] + absMaxs[1]);
        dest[0][2] = 0.5 * (float)(absMins[2] + absMaxs[2]);// G_EntityCentroidWithBounds
        v0[0] = *centerPos - dest[0][0];
        v0[1] = centerPos[1] - dest[0][1];
        v0[2] = centerPos[2] - dest[0][2];
        Vec3Normalize(v0);
        if ( (float)((float)(v0[0] * v0[0]) + (float)(v0[1] * v0[1])) < 0.001 )
        {
            v0[0] = 1.0f;
            v0[1] = 0.0f;
            v0[2] = 0.0f;
        }
        v1[0] = -v0[1];
        v1[1] = v0[0];
        v1[2] = 0.0f;
        Vec3Normalize(v1);
        Vec3Cross(v0, v1, up);
        centerToCorner[0] = absMaxs[0] - dest[0][0];
        centerToCorner[1] = absMaxs[1] - dest[0][1];
        centerToCorner[2] = absMaxs[2] - dest[0][2];
        radiusRight = fabs(v1[1] * (float)(absMaxs[1] - dest[0][1]))
                                + fabs(v1[0] * (float)(absMaxs[0] - dest[0][0]));
        radiusUp = (float)(fabs(up[0] * (float)(absMaxs[0] - dest[0][0]))
                                         + fabs(up[1] * (float)(absMaxs[1] - dest[0][1])))
                         + fabs(up[2] * (float)(absMaxs[2] - dest[0][2]));
        v1[0] = radiusRight * v1[0];
        v1[1] = radiusRight * v1[1];
        v1[2] = radiusRight * v1[2];
        up[0] = radiusUp * up[0];
        up[1] = radiusUp * up[1];
        up[2] = radiusUp * up[2];
        dest[1][0] = (float)(dest[0][0] + v1[0]) + up[0];
        dest[1][1] = (float)(dest[0][1] + v1[1]) + up[1];
        dest[1][2] = (float)(dest[0][2] + v1[2]) + up[2];
        dest[2][0] = (float)(-1.0 * v1[0]) + dest[0][0];
        dest[2][1] = (float)(-1.0 * v1[1]) + dest[0][1];
        dest[2][2] = (float)(-1.0 * v1[2]) + dest[0][2];
        dest[2][0] = dest[2][0] + up[0];
        dest[2][1] = dest[2][1] + up[1];
        dest[2][2] = dest[2][2] + up[2];
        dest[3][0] = (float)(-1.0 * up[0]) + (float)(dest[0][0] + v1[0]);
        dest[3][1] = (float)(-1.0 * up[1]) + (float)(dest[0][1] + v1[1]);
        dest[3][2] = (float)(-1.0 * up[2]) + (float)(dest[0][2] + v1[2]);
        dest[4][0] = (float)(-1.0 * v1[0]) + dest[0][0];
        dest[4][1] = (float)(-1.0 * v1[1]) + dest[0][1];
        dest[4][2] = (float)(-1.0 * v1[2]) + dest[0][2];
        dest[4][0] = (float)(-1.0 * up[0]) + dest[4][0];
        dest[4][1] = (float)(-1.0 * up[1]) + dest[4][1];
        dest[4][2] = (float)(-1.0 * up[2]) + dest[4][2];
        if ( radius_damage_debug->current.enabled )
        {
            for ( i = 0; i < 5; ++i )
            {
                success_1 = 1;
                color_ = (float *)colorWhite;
                if ( coneAngleCos != -1.0 )
                {
                    if ( coneDirection )
                    {
                        v10[0] = dest[i][0] - *centerPos;
                        v10[1] = dest[i][1] - centerPos[1];
                        v10[2] = dest[i][2] - centerPos[2];
                        Vec3Normalize(v10);
                        if ( coneAngleCos > (float)((float)((float)(v10[0] * *coneDirection) + (float)(v10[1] * coneDirection[1]))
                                                                            + (float)(v10[2] * coneDirection[2])) )
                        {
                            success_1 = 0;
                            color_ = (float *)colorOrange;
                        }
                    }
                }
                if ( success_1 && !SV_SightTracePoint(&hitnum, centerPos, dest[i], &context) )
                    color_ = (float *)colorRed;
                CG_DebugLine(centerPos, dest[i], color_, 1, 300);
            }
        }
        for ( i = 0; i < 5; ++i )
        {
            if ( coneAngleCos != -1.0 )
            {
                if ( coneDirection )
                {
                    v7[0] = dest[i][0] - *centerPos;
                    v7[1] = dest[i][1] - centerPos[1];
                    v7[2] = dest[i][2] - centerPos[2];
                    Vec3Normalize(v7);
                    if ( coneAngleCos > (float)((float)((float)(v7[0] * *coneDirection) + (float)(v7[1] * coneDirection[1]))
                                                                        + (float)(v7[2] * coneDirection[2])) )
                        continue;
                }
            }
            if ( SV_SightTracePoint(&hitnum, centerPos, dest[i], &context) )
                return 1.0;
        }
        return 0.0;
    }
}

void __cdecl G_FlashbangBlast(float *origin, float radius_max, float radius_min, gentity_s *attacker, team_t team)
{
    int i; // [esp+ACh] [ebp-100Ch]
    int entList[1024]; // [esp+B0h] [ebp-1008h] BYREF
    gentity_s *ent; // [esp+10B0h] [ebp-8h]
    int entListCount; // [esp+10B4h] [ebp-4h] BYREF

    if ( radius_min < 1.0 )
        radius_min = 1.0f;
    if ( radius_min > radius_max )
        radius_max = radius_min;
    GetEntListForRadius(origin, radius_max, radius_min, entList, &entListCount);
    for ( i = 0; i < entListCount; ++i )
    {
        ent = &g_entities[entList[i]];
        if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_combat_mp.cpp", 1358, 0, "%s", "ent") )
            __debugbreak();
        FlashbangBlastEnt(ent, origin, radius_max, radius_min, attacker, team);
    }
}

void __cdecl GetEntListForRadius(
                const float *origin,
                float radius_max,
                float radius_min,
                int *entList,
                int *entListCount)
{
    float mins[3]; // [esp+0h] [ebp-20h] BYREF
    float boxradius; // [esp+Ch] [ebp-14h]
    float maxs[3]; // [esp+10h] [ebp-10h] BYREF
    int i; // [esp+1Ch] [ebp-4h]

    boxradius = 1.4142135 * radius_max;
    for ( i = 0; i < 3; ++i )
    {
        mins[i] = origin[i] - boxradius;
        maxs[i] = origin[i] + boxradius;
    }
    *entListCount = CM_AreaEntities(mins, maxs, entList, 1024, -1);
}

void __cdecl FlashbangBlastEnt(
                gentity_s *ent,
                const float *blastOrigin,
                float radius_max,
                float radius_min,
                gentity_s *attacker,
                team_t team)
{
    float percent_distance; // [esp+48h] [ebp-5Ch]
    float percent_angle; // [esp+4Ch] [ebp-58h]
    float forward[3]; // [esp+50h] [ebp-54h] BYREF
    float toBlast[3]; // [esp+5Ch] [ebp-48h] BYREF
    float dist; // [esp+90h] [ebp-14h]
    int hitNum; // [esp+94h] [ebp-10h] BYREF
    float playerEyes[3]; // [esp+98h] [ebp-Ch] BYREF

    hitNum = -1;
    if ( CanEntityBeFlashbanged(ent) )
    {
        if ( ent->takedamage )
        {
            dist = EntDistToPoint(blastOrigin, ent);
            if ( dist <= radius_max )
            {
                GetFlashbangViewPos(ent, playerEyes);
                //col_context_t::col_context_t(&context, 8419363);
                col_context_t context(0x807823); // [esp+68h] [ebp-3Ch] BYREF

                context.passEntityNum0 = ent->s.number;
                SV_SightTracePoint(&hitNum, playerEyes, blastOrigin, &context);
                if ( hitNum == -1 || !hitNum )
                {
                    if ( radius_min < dist )
                        percent_distance = 1.0 - (float)((float)(dist - radius_min) / (float)(radius_max - radius_min));
                    else
                        percent_distance = 1.0f;
                    GetFlashbangViewDirection(ent, forward);
                    toBlast[0] = *blastOrigin - playerEyes[0];
                    toBlast[1] = blastOrigin[1] - playerEyes[1];
                    toBlast[2] = blastOrigin[2] - playerEyes[2];
                    Vec3NormalizeFast(toBlast);
                    percent_angle = (float)((float)((float)((float)(forward[0] * toBlast[0]) + (float)(forward[1] * toBlast[1]))
                                                                                + (float)(forward[2] * toBlast[2]))
                                                                + 1.0)
                                                * 0.5;
                    AddScrTeamName(team);
                    if ( attacker )
                        Scr_AddEntity(attacker, SCRIPTINSTANCE_SERVER);
                    else
                        Scr_AddUndefined(SCRIPTINSTANCE_SERVER);
                    Scr_AddFloat(percent_angle, SCRIPTINSTANCE_SERVER);
                    Scr_AddFloat(percent_distance, SCRIPTINSTANCE_SERVER);
                    Scr_Notify(ent, scr_const.flashbang, 4u);
                }
            }
        }
    }
}

double __cdecl EntDistToPoint(const float *origin, gentity_s *ent)
{
    unsigned int i; // [esp+10h] [ebp-10h]
    float v[3]; // [esp+14h] [ebp-Ch] BYREF

    if ( ent->r.bmodel )
    {
        for ( i = 0; i < 3; ++i )
        {
            if ( ent->r.absmin[i] <= origin[i] )
            {
                if ( origin[i] <= ent->r.absmax[i] )
                    v[i] = 0.0f;
                else
                    v[i] = origin[i] - ent->r.absmax[i];
            }
            else
            {
                v[i] = ent->r.absmin[i] - origin[i];
            }
        }
        return Vec3Length(v);
    }
    else
    {
        v[0] = ent->r.currentOrigin[0] - *origin;
        v[1] = ent->r.currentOrigin[1] - origin[1];
        v[2] = ent->r.currentOrigin[2] - origin[2];
        return Vec3Length(v);
    }
}

void __cdecl AddScrTeamName(team_t team)
{
    switch ( team )
    {
        case TEAM_FREE:
            Scr_AddConstString(scr_const.free, SCRIPTINSTANCE_SERVER);
            break;
        case TEAM_AXIS:
            Scr_AddConstString(scr_const.axis, SCRIPTINSTANCE_SERVER);
            break;
        case TEAM_ALLIES:
            Scr_AddConstString(scr_const.allies, SCRIPTINSTANCE_SERVER);
            break;
        case TEAM_SPECTATOR:
            Scr_AddConstString(scr_const.spectator, SCRIPTINSTANCE_SERVER);
            break;
        default:
            Com_PrintWarning(15, "AddScrTeamName(): Unhandled team name %i.\n", team);
            Scr_AddUndefined(SCRIPTINSTANCE_SERVER);
            break;
    }
}

bool __cdecl CanEntityBeFlashbanged(gentity_s *ent)
{
    if ( ent->client )
        return 1;
    return ent->s.eType == 14
            && G_IsVehicleOccupied(ent)
            && G_IsVehicleRemoteControl(ent->s.vehicleState.vehicleInfoIndex);
}

void __cdecl GetFlashbangViewPos(gentity_s *ent, float *origin)
{
    gclient_s *client; // edx

    if ( ent->client )
    {
        client = ent->client;
        *origin = client->ps.origin[0];
        origin[1] = client->ps.origin[1];
        origin[2] = client->ps.origin[2];
        origin[2] = origin[2] + ent->client->ps.viewHeightCurrent;
    }
    else
    {
        *origin = ent->r.currentOrigin[0];
        origin[1] = ent->r.currentOrigin[1];
        origin[2] = ent->r.currentOrigin[2];
    }
}

void __cdecl GetFlashbangViewDirection(gentity_s *ent, float *dir)
{
    gentity_s *driver; // [esp+0h] [ebp-4h]

    if ( ent->client )
    {
        AngleVectors(ent->client->ps.viewangles, dir, 0, 0);
    }
    else if ( ent->s.eType == 14 && (driver = VEH_GetSeatOccupantEntity(ent, 0)) != 0 && driver->client )
    {
        AngleVectors(driver->client->ps.viewangles, dir, 0, 0);
    }
    else
    {
        AngleVectors(ent->r.currentAngles, dir, 0, 0);
    }
}

bool __cdecl G_WithinDamageRadius(const float *damageOrigin, float radiusSquared, gentity_s *ent)
{
    float distSqrd; // [esp+4h] [ebp-4h]

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_combat_mp.cpp", 1405, 0, "%s", "ent") )
        __debugbreak();
    distSqrd = G_GetRadiusDamageDistanceSquared(damageOrigin, ent);
    return radiusSquared > distSqrd;
}

double __cdecl G_GetRadiusDamageDistanceSquared(const float *damageOrigin, gentity_s *ent)
{
    int i; // [esp+8h] [ebp-10h]
    float v[3]; // [esp+Ch] [ebp-Ch]

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_combat_mp.cpp", 1373, 0, "%s", "ent") )
        __debugbreak();
    if ( ent->r.bmodel )
    {
        for ( i = 0; i < 3; ++i )
        {
            if ( ent->r.absmin[i] <= damageOrigin[i] )
            {
                if ( damageOrigin[i] <= ent->r.absmax[i] )
                    v[i] = 0.0f;
                else
                    v[i] = damageOrigin[i] - ent->r.absmax[i];
            }
            else
            {
                v[i] = ent->r.absmin[i] - damageOrigin[i];
            }
        }
    }
    else
    {
        v[0] = ent->r.currentOrigin[0] - *damageOrigin;
        v[1] = ent->r.currentOrigin[1] - damageOrigin[1];
        v[2] = ent->r.currentOrigin[2] - damageOrigin[2];
    }
    return v[2] * v[2] + v[1] * v[1] + v[0] * v[0];
}

int __cdecl G_RadiusDamage(
                float *origin,
                gentity_s *inflictor,
                gentity_s *attacker,
                float fInnerDamage,
                float fOuterDamage,
                float radius,
                float coneAngleCos,
                float *coneDirection,
                gentity_s *ignore,
                int mod,
                unsigned int weapon)
{
    int j; // [esp+2Ch] [ebp-1054h]
    float damage; // [esp+30h] [ebp-1050h]
    float dir[3]; // [esp+34h] [ebp-104Ch] BYREF
    //float v15; // [esp+3Ch] [ebp-1044h]
    float v16; // [esp+40h] [ebp-1040h]
    float mins[3]; // [esp+44h] [ebp-103Ch] BYREF
    float v18; // [esp+50h] [ebp-1030h]
    float v19; // [esp+54h] [ebp-102Ch]
    float RadiusDamageDistanceSquared; // [esp+58h] [ebp-1028h]
    int v21; // [esp+5Ch] [ebp-1024h]
    float maxs[3]; // [esp+60h] [ebp-1020h] BYREF
    float v23; // [esp+6Ch] [ebp-1014h]
    int entityList[1024]; // [esp+70h] [ebp-1010h] BYREF
    gentity_s *ent; // [esp+1070h] [ebp-10h]
    int dflags; // [esp+1074h] [ebp-Ch]
    int i; // [esp+1078h] [ebp-8h]
    int v28; // [esp+107Ch] [ebp-4h]

    v28 = 0;
    dflags = 5;
    if ( !attacker )
        return 0;
    if ( radius < 1.0 )
        radius = 1.0f;
    v18 = radius * radius;
    v19 = 1.4142135 * radius;
    for ( i = 0; i < 3; ++i )
    {
        mins[i] = origin[i] - v19;
        maxs[i] = origin[i] + v19;
    }
    v21 = CM_AreaEntities(mins, maxs, entityList, 1024, -1);
    for ( j = 0; j < v21; ++j )
    {
        ent = &g_entities[entityList[j]];
        if ( ent != ignore && ent->takedamage && (!ent->client || !level.bPlayerIgnoreRadiusDamage) )
        {
            RadiusDamageDistanceSquared = G_GetRadiusDamageDistanceSquared(origin, ent);
            if ( RadiusDamageDistanceSquared < v18 )
            {
                v16 = sqrtf(RadiusDamageDistanceSquared);
                damage = (float)((float)(fInnerDamage - fOuterDamage) * (float)(1.0 - (float)(v16 / radius))) + fOuterDamage;
                if ( ent->scr_vehicle )
                {
                    Scr_VehicleRadiusDamage(
                        ent,
                        inflictor,
                        attacker,
                        (int)damage,
                        fInnerDamage,
                        fOuterDamage,
                        dflags,
                        mod,
                        weapon,
                        origin,
                        radius,
                        coneAngleCos,
                        coneDirection,
                        0);
                }
                else
                {
                    v23 = CanDamage(ent, inflictor, origin, coneAngleCos, coneDirection, 8396819);
                    if ( v23 > 0.0 )
                    {
                        if ( LogAccuracyHit(ent, attacker) )
                            v28 = 1;
                        dir[0] = ent->r.currentOrigin[0] - *origin;
                        dir[1] = ent->r.currentOrigin[1] - origin[1];
                        dir[2] = ent->r.currentOrigin[2] - origin[2];
                        dir[2] += 24.0;
                        if ( ent->destructible )
                            damage = DestructibleRadiusDamage(ent, origin, fInnerDamage, fOuterDamage, radius, mod, attacker);
                        G_Damage(
                            ent,
                            inflictor,
                            attacker,
                            dir,
                            origin,
                            (int)(float)(damage * v23),
                            dflags,
                            mod,
                            weapon,
                            HITLOC_NONE,
                            0,
                            0,
                            0);
                    }
                }
            }
        }
    }
    DynEntSv_RadiusDamage(origin, radius, coneAngleCos, coneDirection, fInnerDamage, fOuterDamage);
    GlassSv_RadiusDamage(origin, radius, coneAngleCos, coneDirection, fInnerDamage, fOuterDamage, mod);
    return v28;
}

unsigned __int16 __cdecl G_GetHitLocationString(hitLocation_t hitLoc)
{
    if ( (unsigned int)hitLoc >= HITLOC_NUM
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_combat_mp.cpp",
                    1525,
                    0,
                    "%s",
                    "(unsigned)hitLoc < HITLOC_NUM") )
    {
        __debugbreak();
    }
    return g_HitLocConstNames[hitLoc];
}

int __cdecl G_GetHitLocationIndexFromString(unsigned __int16 sString)
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < 19; ++i )
    {
        if ( g_HitLocConstNames[i] == sString )
            return i;
    }
    return 0;
}

