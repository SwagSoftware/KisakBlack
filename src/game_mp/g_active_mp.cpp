#include "g_active_mp.h"
#include <bgame/bg_misc.h>
#include <universal/com_math_anglevectors.h>
#include <game/actor_script_cmd.h>
#include <cgame_mp/cg_predict_mp.h>
#include <clientscript/cscr_vm.h>
#include "g_spawn_mp.h"
#include <clientscript/scr_const.h>
#include <glass/glass_server.h>
#include <qcommon/cm_world.h>
#include <server/sv_game.h>
#include "g_cmds_mp.h"
#include <bgame/bg_pmove.h>
#include <server_mp/sv_main_mp.h>
#include <game/g_weapon.h>
#include "g_utils_mp.h"
#include <game/g_missile.h>
#include "g_combat_mp.h"
#include <game/bullet.h>
#include <game/actor_events.h>
#include <bgame/bg_perks.h>
#include <qcommon/dobj_management.h>
#include <cgame/cg_drawtools.h>
#include <xanim/dobj_utils.h>
#include <xanim/xmodel_utils.h>
#include <cstring>
#include <clientscript/cscr_stringlist.h>
#include <bgame/bg_weapons_view.h>
#include "player_use_mp.h"
#include <qcommon/threads.h>
#include <client_mp/g_client_mp.h>
#include <server_mp/sv_bot_mp.h>
#include <server_mp/sv_init_mp.h>
#include <cgame_mp/cg_pose_mp.h>
#include <cgame_mp/cg_ents_mp.h>
#include <demo/demo_recording.h>
#include <game/turret.h>
#include <client/cl_debugdata.h>

pmove_t g_pmove[32]; // i think this is 32

float zBoost = 100.0;
float xySpeed = 100.0;
float gunnerZheight = 90.0;
float gunnerXYradius = 30.0;
float fCrouchHeightPanzer_0 = 105.0;
float fCrouchHeightT34_0 = 109.0;
float handOfs_1 = -5.0;
float handOfs_2 = -6.0;
float scale_1 = 0.15000001;
float thresh_0 = 88.0;

hudelem_s g_dummyHudCurrent;

void __cdecl P_DamageFeedback(gentity_s *player)
{
    gclient_s *client; // [esp+Ch] [ebp-40h]
    int damage; // [esp+10h] [ebp-3Ch]
    int damagea; // [esp+10h] [ebp-3Ch]
    float kick; // [esp+14h] [ebp-38h]
    float angles[3]; // [esp+18h] [ebp-34h] BYREF
    float viewaxis[3][3]; // [esp+24h] [ebp-28h] BYREF
    int DAMAGE_COUNT_DURATION; // [esp+48h] [ebp-4h]

    DAMAGE_COUNT_DURATION = 500;
    client = player->client;
    if ( !client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp", 56, 0, "%s", "client") )
    {
        __debugbreak();
    }
    if ( client->ps.pm_type < 9 )
    {
        if ( level.time - client->damageTime > 500 )
            client->ps.damageCount = 0;
        damage = client->damage_blood;
        if ( damage > 0 && client->sess.maxHealth > 0 )
        {
            damagea = 100 * damage / client->sess.maxHealth;
            if ( damagea > 127 )
                damagea = 127;
            client->ps.aimSpreadScale = (float)damagea + client->ps.aimSpreadScale;
            if ( client->ps.aimSpreadScale > 255.0 )
                client->ps.aimSpreadScale = 255.0f;
            kick = (float)damagea * bg_viewKickScale->current.value;
            if ( bg_viewKickMin->current.value <= kick )
            {
                if ( kick > bg_viewKickMax->current.value )
                    kick = bg_viewKickMax->current.value;
            }
            else
            {
                kick = bg_viewKickMin->current.value;
            }
            if ( client->damage_fromWorld )
            {
                client->v_dmg_roll = 0.0f;
                client->v_dmg_pitch = -kick;
                client->ps.damagePitch = 255;
                client->ps.damageYaw = 255;
                client->damage_fromWorld = 0;
            }
            else
            {
                vectoangles(client->damage_from, angles);
                AnglesToAxis(client->ps.viewangles, viewaxis);
                client->v_dmg_roll = (-(kick)) * (float)((float)((float)(client->damage_from[0] * viewaxis[1][0]) + (float)(client->damage_from[1] * viewaxis[1][1])) + (float)(client->damage_from[2] * viewaxis[1][2]));
                client->v_dmg_pitch = (float)((float)((float)(client->damage_from[0] * viewaxis[0][0])
                                                                                        + (float)(client->damage_from[1] * viewaxis[0][1]))
                                                                        + (float)(client->damage_from[2] * viewaxis[0][2]))
                                                        * kick;
                client->ps.damagePitch = (int)(float)((float)(angles[0] / 360.0) * 256.0);
                client->ps.damageYaw = (int)(float)((float)(angles[1] / 360.0) * 256.0);
            }
            ++client->ps.damageEvent;
            client->damageTime = level.time - 20;
            client->ps.damageCount = damagea;
            client->damage_blood = 0;
        }
    }
}

void __cdecl ClientImpacts(gentity_s *ent, pmove_t *pm)
{
    int j; // [esp+4h] [ebp-14h]
    void(__cdecl * entTouch)(gentity_s *, gentity_s *, int); // [esp+8h] [ebp-10h]
    gentity_s *other; // [esp+Ch] [ebp-Ch]
    void(__cdecl * otherTouch)(gentity_s *, gentity_s *, int); // [esp+10h] [ebp-8h]
    int i; // [esp+14h] [ebp-4h]
    int ia; // [esp+14h] [ebp-4h]

    entTouch = entityHandlers[ent->handler].touch;
    for (i = 0; i < pm->numtouch; ++i)
    {
        for (j = 0; j < i && pm->touchents[j] != pm->touchents[i]; ++j)
            ;
        if (j == i)
        {
            other = &g_entities[pm->touchents[i]];
            if (Scr_IsSystemActive(1u, SCRIPTINSTANCE_SERVER))
            {
                Scr_AddEntity(other, SCRIPTINSTANCE_SERVER);
                Scr_Notify(ent, scr_const.touch, 1u);
                Scr_AddEntity(ent, SCRIPTINSTANCE_SERVER);
                Scr_Notify(other, scr_const.touch, 1u);
            }
            otherTouch = entityHandlers[other->handler].touch;
            if (otherTouch)
                otherTouch(other, ent, 1);
            if (entTouch)
                entTouch(ent, other, 1);
        }
    }
    for (ia = 0; ia < pm->numGlassTouch; ++ia)
        GlassSv_Touch(pm->touchGlasses[ia], ent);
}

void __cdecl G_DoTouchTriggers(gentity_s *ent)
{
    gclient_s *client; // edx
    float v3; // xmm0_4
    float *origin; // [esp+1Ch] [ebp-103Ch]
    int entityList[1024]; // [esp+28h] [ebp-1030h] BYREF
    int contentmask; // [esp+1028h] [ebp-30h]
    float mins[3]; // [esp+102Ch] [ebp-2Ch] BYREF
    void(__cdecl * touch)(gentity_s *, gentity_s *, int); // [esp+1038h] [ebp-20h]
    gentity_s *hit; // [esp+103Ch] [ebp-1Ch]
    float maxs[3]; // [esp+1040h] [ebp-18h] BYREF
    void(__cdecl * v11)(gentity_s *, gentity_s *, int); // [esp+104Ch] [ebp-Ch]
    int v12; // [esp+1050h] [ebp-8h]
    int i; // [esp+1054h] [ebp-4h]

    if (!ent->client
        && !ent->actor
        && !ent->scr_vehicle
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp",
            241,
            0,
            "%s",
            "ent->client || ent->actor || ent->scr_vehicle"))
    {
        __debugbreak();
    }
    if (!ent->client
        || ent->client->ps.pm_type <= 1
        || ent->client->ps.pm_type == 6
        || ent->client->ps.pm_type == 7
        || ent->client->ps.pm_type == 8)
    {
        mins[0] = ent->r.absmin[0] - 20.0;
        mins[1] = ent->r.absmin[1] - 20.0;
        mins[2] = ent->r.absmin[2] - 20.0;
        maxs[0] = ent->r.absmax[0] + 20.0;
        maxs[1] = ent->r.absmax[1] + 20.0;
        maxs[2] = ent->r.absmax[2] + 20.0;
        if (ent->scr_vehicle)
        {
            contentmask = 8;
        LABEL_26:
            v12 = CM_AreaEntities(mins, maxs, entityList, 1024, contentmask);
            if (ent->client)
            {
                origin = ent->client->ps.origin;
                mins[0] = *origin + ent->r.mins[0];
                mins[1] = origin[1] + ent->r.mins[1];
                mins[2] = origin[2] + ent->r.mins[2];
                client = ent->client;
                maxs[0] = client->ps.origin[0] + ent->r.maxs[0];
                maxs[1] = client->ps.origin[1] + ent->r.maxs[1];
                v3 = client->ps.origin[2];
            }
            else
            {
                mins[0] = ent->r.currentOrigin[0] + ent->r.mins[0];
                mins[1] = ent->r.currentOrigin[1] + ent->r.mins[1];
                mins[2] = ent->r.currentOrigin[2] + ent->r.mins[2];
                maxs[0] = ent->r.currentOrigin[0] + ent->r.maxs[0];
                maxs[1] = ent->r.currentOrigin[1] + ent->r.maxs[1];
                v3 = ent->r.currentOrigin[2];
            }
            maxs[2] = v3 + ent->r.maxs[2];
            ExpandBoundsToWidth(mins, maxs);
            touch = entityHandlers[ent->handler].touch;
            for (i = 0; ; ++i)
            {
                if (i >= v12)
                    return;
                hit = &g_entities[entityList[i]];
                if ((hit->r.contents & 0x405C0008) == 0
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp",
                        319,
                        0,
                        "%s",
                        "hit->r.contents & MASK_TRIGGER"))
                {
                    __debugbreak();
                }
                if (hit->s.eType == 4
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp",
                        320,
                        0,
                        "%s",
                        "hit->s.eType != ET_MISSILE"))
                {
                    __debugbreak();
                }
                v11 = entityHandlers[hit->handler].touch;
                if ((v11 || touch)
                    && (!ent->client
                        || (hit->r.clientMask[(int)ent->client->ps.clientNum >> 5] & (1 << (ent->client->ps.clientNum & 0x1F))) == 0))
                {
                    if (hit->s.eType == 3)
                    {
                        if (!ent->client || !BG_PlayerTouchesItem(&ent->client->ps, &hit->s, level.time))
                            continue;
                    }
                    else if (!SV_EntityContact(mins, maxs, hit))
                    {
                        continue;
                    }
                    if (Scr_IsSystemActive(1u, SCRIPTINSTANCE_SERVER))
                    {
                        Scr_AddEntity(ent, SCRIPTINSTANCE_SERVER);
                        Scr_Notify(hit, scr_const.touch, 1u);
                        Scr_AddEntity(hit, SCRIPTINSTANCE_SERVER);
                        Scr_Notify(ent, scr_const.touch, 1u);
                    }
                    if (v11)
                        v11(hit, ent, 1);
                    if (ent->actor)
                    {
                        if (touch)
                            touch(ent, hit, 1);
                    }
                }
            }
        }
        if (!ent->sentient)
        {
            contentmask = 0x400000;
            goto LABEL_26;
        }
        if (ent->client)
        {
            contentmask = 0x40000000;
            goto LABEL_26;
        }
        switch (ent->sentient->eTeam)
        {
        case TEAM_AXIS:
            contentmask = 0x40000;
            goto LABEL_26;
        case TEAM_ALLIES:
            contentmask = 0x80000;
            goto LABEL_26;
        case TEAM_FREE:
            contentmask = 0x100000;
            goto LABEL_26;
        }
    }
}

void __cdecl SpectatorThink(gentity_s *ent, usercmd_s *ucmd)
{
    bool v2; // esi
    int jj; // [esp+14h] [ebp-2Ch]
    int ii; // [esp+18h] [ebp-28h]
    int n; // [esp+1Ch] [ebp-24h]
    int m; // [esp+20h] [ebp-20h]
    int k; // [esp+24h] [ebp-1Ch]
    int j; // [esp+28h] [ebp-18h]
    int i; // [esp+2Ch] [ebp-14h]
    gclient_s *client; // [esp+30h] [ebp-10h]
    pmove_t *pm; // [esp+34h] [ebp-Ch]
    bitarray<51> changedButton_bits; // [esp+38h] [ebp-8h]

    client = ent->client;
    for ( i = 0; i < 2; ++i )
        client->oldbutton_bits.array[i] = client->button_bits.array[i];
    for ( j = 0; j < 2; ++j )
        client->button_bits.array[j] = client->sess.cmd.button_bits.array[j];
    for ( k = 0; k < 2; ++k )
        changedButton_bits.array[k] = 0;
    for ( m = 0; m < 2; ++m )
        changedButton_bits.array[m] = client->oldbutton_bits.array[m];
    for ( n = 0; n < 2; ++n )
        changedButton_bits.array[n] = ~changedButton_bits.array[n];
    for ( ii = 0; ii < 2; ++ii )
        changedButton_bits.array[ii] &= client->button_bits.array[ii];
    for ( jj = 0; jj < 2; ++jj )
        client->button_bitsSinceLastFrame.array[jj] |= changedButton_bits.array[jj];
    pm = &g_pmove[client->ps.clientNum];
    if ( client->sess.forceSpectatorClient < 0 )
    {
        if ( G_ClientCanSpectateTeam(client, TEAM_NUM_TEAMS) )
        {
            if ( client->spectatorClient >= 0 )
            {
                v2 = client->button_bits.testBit(2u);
                if ( v2 != client->oldbutton_bits.testBit(2u) )
                    StopFollowing(ent);
            }
        }
    }
    if ( !client->button_bits.testBit(0) || client->oldbutton_bits.testBit(0) )
    {
        if ( client->button_bits.testBit(0xBu) && !client->oldbutton_bits.testBit(0xBu) )
            Cmd_FollowCycle_f(ent, -1);
    }
    else
    {
        Cmd_FollowCycle_f(ent, 1);
    }
    if ( (client->ps.otherFlags & 2) == 0 )
    {
        client->ps.pm_type = 4;
        if ( G_ClientCanSpectateTeam(client, TEAM_NUM_TEAMS) )
            client->ps.speed = 400;
        else
            client->ps.speed = 0;
        memset(pm, 0, 0x258u);
        pm->localClientNum = -1;
        pm->ps = &client->ps;
        memcpy(&pm->cmd, ucmd, sizeof(pm->cmd));
        pm->tracemask = 0x800811;
        pm->handler = 1;
        Pmove(pm);
        ent->r.currentOrigin[0] = client->ps.origin[0];
        ent->r.currentOrigin[1] = client->ps.origin[1];
        ent->r.currentOrigin[2] = client->ps.origin[2];
        SV_UnlinkEntity(ent);
    }
}

int __cdecl ClientInactivityTimer(gclient_s *client)
{
    const char *v2; // eax

    if ( (unsigned int)(client - level.clients) >= level.maxclients
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp",
                    444,
                    0,
                    "client - level.clients doesn't index level.maxclients\n\t%i not in [0, %i)",
                    client - level.clients,
                    level.maxclients) )
    {
        __debugbreak();
    }
    if ( g_inactivity->current.integer )
    {
        if ( client->sess.cmd.forwardmove
            || client->sess.cmd.rightmove
            || client->sess.cmd.button_bits.testBit(0)
            || client->sess.cmd.button_bits.testBit(0xAu) )
        {
            client->inactivityTime = level.time + 1000 * g_inactivity->current.integer;
            client->inactivityWarning = 0;
        }
        else if ( !client->sess.localClient )
        {
            if ( level.time > client->inactivityTime )
            {
                SV_GameDropClient(client - level.clients, "GAME_DROPPEDFORINACTIVITY");
                return 0;
            }
            if ( level.time > client->inactivityTime - 10000 && !client->inactivityWarning )
            {
                client->inactivityWarning = 1;
                v2 = va("%c \"GAME_INACTIVEDROPWARNING\"", 99);
                SV_GameSendServerCommand(client - level.clients, SV_CMD_CAN_IGNORE, v2);
            }
        }
    }
    else
    {
        client->inactivityTime = level.time + 60000;
        client->inactivityWarning = 0;
    }
    return 1;
}

void __cdecl ClientIntermissionThink(gentity_s *ent)
{
    int jj; // [esp+8h] [ebp-28h]
    int ii; // [esp+Ch] [ebp-24h]
    int n; // [esp+10h] [ebp-20h]
    int m; // [esp+14h] [ebp-1Ch]
    int k; // [esp+18h] [ebp-18h]
    int j; // [esp+1Ch] [ebp-14h]
    int i; // [esp+20h] [ebp-10h]
    gclient_s *client; // [esp+24h] [ebp-Ch]
    bitarray<51> changedButton_bits; // [esp+28h] [ebp-8h]

    client = ent->client;
    for ( i = 0; i < 2; ++i )
        client->oldbutton_bits.array[i] = client->button_bits.array[i];
    for ( j = 0; j < 2; ++j )
        client->button_bits.array[j] = client->sess.cmd.button_bits.array[j];
    for ( k = 0; k < 2; ++k )
        changedButton_bits.array[k] = 0;
    for ( m = 0; m < 2; ++m )
        changedButton_bits.array[m] = client->oldbutton_bits.array[m];
    for ( n = 0; n < 2; ++n )
        changedButton_bits.array[n] = ~changedButton_bits.array[n];
    for ( ii = 0; ii < 2; ++ii )
        changedButton_bits.array[ii] &= client->button_bits.array[ii];
    for ( jj = 0; jj < 2; ++jj )
        client->button_bitsSinceLastFrame.array[jj] |= changedButton_bits.array[jj];
}

void __cdecl NotifyGrenadePullback(gentity_s *ent, unsigned int weaponIndex)
{
    char *v2; // eax

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp", 507, 0, "%s", "ent") )
        __debugbreak();
    if ( !ent->client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp", 508, 0, "%s", "ent->client") )
    {
        __debugbreak();
    }
    if ( !weaponIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp",
                    509,
                    0,
                    "%s",
                    "weaponIndex != WP_NONE") )
    {
        __debugbreak();
    }
    v2 = (char *)BG_WeaponName(weaponIndex);
    Scr_AddString(v2, SCRIPTINSTANCE_SERVER);
    Scr_Notify(ent, scr_const.grenade_pullback, 1u);
}

void __cdecl HandleClientEvent(gclient_s *client, gentity_s *ent, int event, int eventParm)
{
    unsigned __int8 PlayerWeaponModel; // al
    int height; // [esp+0h] [ebp-80h]
    int iTeamFlags; // [esp+3Ch] [ebp-44h]
    float vSentientPos[3]; // [esp+40h] [ebp-40h] BYREF
    team_t eEnemyTeam; // [esp+4Ch] [ebp-34h]
    gentity_s *attacker; // [esp+50h] [ebp-30h]
    int grenadeWeaponIndex; // [esp+54h] [ebp-2Ch]
    float launchvel[3]; // [esp+58h] [ebp-28h] BYREF
    float launchspot[3]; // [esp+64h] [ebp-1Ch] BYREF
    float damage; // [esp+70h] [ebp-10h]
    float dir[3]; // [esp+74h] [ebp-Ch]

    switch ( event )
    {
        case 9:
            if ( !ent->client
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp", 711, 0, "%s", "ent->client") )
            {
                __debugbreak();
            }
            Scr_Notify(ent, scr_const.force_crouch, 0);
            return;
        case 22:
            if ( !ent->client
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp", 696, 0, "%s", "ent->client") )
            {
                __debugbreak();
            }
            Scr_Notify(ent, scr_const.reload_start, 0);
            return;
        case 23:
            if ( !ent->client
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp", 701, 0, "%s", "ent->client") )
            {
                __debugbreak();
            }
            Scr_Notify(ent, scr_const.reload, 0);
            return;
        case 28:
        case 41:
            if ( !ent->client
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp", 634, 0, "%s", "ent->client") )
            {
                __debugbreak();
            }
            NotifyGrenadePullback(ent, eventParm);
            return;
        case 29:
        case 30:
        case 47:
            if ( !g_antilag->current.enabled )
                goto LABEL_6;
            goto LABEL_3;
        case 31:
        case 32:
            if ( g_antilag->current.enabled )
LABEL_3:
                FireWeapon(ent, client->lastServerTime, eventParm);
            else
LABEL_6:
                FireWeapon(ent, level.time, eventParm);
            return;
        case 36:
            if ( g_antilag->current.enabled )
                FireWeaponMelee(ent, client->lastServerTime);
            else
                FireWeaponMelee(ent, level.time);
            return;
        case 37:
        case 191:
            return;
        case 38:
            DeployWeapon(ent);
            return;
        case 39:
            BreakDownWeapon(ent);
            return;
        case 42:
            G_UseOffHand(ent);
            return;
        case 43:
            if ( ent->client->ps.cursorHintEntIndex
                && (BG_GetWeaponDef(eventParm)->offhandClass == OFFHAND_CLASS_FRAG_GRENADE
                 || BG_GetWeaponDef(eventParm)->offhandClass == OFFHAND_CLASS_GEAR) )
            {
                AttemptLiveGrenadePickup(ent);
            }
            return;
        case 89:
            if ( ent->client->ps.grenadeTimeLeft )
            {
                grenadeWeaponIndex = ent->client->ps.offHandIndex;
                launchvel[0] = G_crandom();
                launchvel[1] = G_crandom();
                launchvel[2] = G_crandom();
                launchvel[0] = 160.0 * launchvel[0];
                launchvel[1] = 160.0 * launchvel[1];
                launchvel[2] = 160.0 * launchvel[2];
                launchspot[0] = ent->r.currentOrigin[0];
                launchspot[1] = ent->r.currentOrigin[1];
                launchspot[2] = ent->r.currentOrigin[2];
                launchspot[2] = launchspot[2] + 40.0;
                height = ent->client->ps.grenadeTimeLeft;
                PlayerWeaponModel = BG_GetPlayerWeaponModel(&ent->client->ps, grenadeWeaponIndex);
                G_FireGrenade(ent, launchspot, launchvel, grenadeWeaponIndex, PlayerWeaponModel, 1, height);
            }
            return;
        case 90:
            if ( ent->client && (ent->flags & 3) == 0 )
            {
                ent->health = 0;
                ent->client->ps.stats[0] = 0;
                if ( ent->client->ps.throwBackGrenadeOwner == 1023 )
                {
                    player_die(ent, ent, ent, 100000, 0xDu, eventParm, 0, HITLOC_NONE, 0);
                }
                else
                {
                    attacker = &g_entities[ent->client->ps.throwBackGrenadeOwner];
                    player_die(ent, attacker, attacker, 100000, 3u, eventParm, 0, HITLOC_NONE, 0);
                }
            }
            return;
        case 91:
            if ( !ent->client
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp", 706, 0, "%s", "ent->client") )
            {
                __debugbreak();
            }
            Scr_Notify(ent, scr_const.detonate, 0);
            return;
        default:
            if ( event < 142 || event > 172 )
                goto LABEL_54;
            if ( ent->s.eType == 1 )
            {
                if ( eventParm < 100 )
                    damage = (float)eventParm * 0.0099999998;
                else
                    damage = 1.1f;
                if ( damage != 0.0 )
                {
                    damage = (float)client->ps.stats[2] * damage;
                    dir[0] = 0.0f;
                    dir[1] = 0.0f;
                    dir[2] = 1.0f;
                    G_Damage(ent, 0, 0, 0, 0, (int)damage, 0, 12, 0xFFFFFFFF, HITLOC_NONE, 0, 0, 0);
                }
LABEL_54:
                if ( (event == 103 || event == 104 || event == 105 || event == 110 || event >= 111 && event <= 141)
                    && (ent->client->ps.pm_flags & 2) == 0
                    && !ent->client->sess.cmd.button_bits.testBit(0xBu)
                    && (ent->client->ps.perks[1] & 0x40) == 0
                    && ent->sentient )
                {
                    eEnemyTeam = Sentient_EnemyTeam(ent->sentient->eTeam);
                    iTeamFlags = 1 << eEnemyTeam;
                    Sentient_GetOrigin(ent->sentient, vSentientPos);
                    if ( event == 105 )
                        Actor_BroadcastPointEvent(ent, AI_EV_FOOTSTEP_LITE, iTeamFlags, vSentientPos, 0.0);
                    else
                        Actor_BroadcastPointEvent(ent, AI_EV_FOOTSTEP, iTeamFlags, vSentientPos, 0.0);
                }
            }
            return;
    }
}

void __cdecl AttemptLiveGrenadePickup(gentity_s *clientEnt)
{
    void(__cdecl * touch)(gentity_s *, gentity_s *, int); // [esp+14h] [ebp-Ch]
    gentity_s *grenadeEnt; // [esp+18h] [ebp-8h]
    int weapIdx; // [esp+1Ch] [ebp-4h]

    if (!clientEnt
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp", 539, 0, "%s", "clientEnt"))
    {
        __debugbreak();
    }
    if (!clientEnt->client
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp",
            540,
            0,
            "%s",
            "clientEnt->client"))
    {
        __debugbreak();
    }
    if (clientEnt->client->ps.cursorHintEntIndex >= 0x400u
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp",
            541,
            0,
            "%s",
            "(unsigned)clientEnt->client->ps.cursorHintEntIndex < MAX_GENTITIES"))
    {
        __debugbreak();
    }
    grenadeEnt = &g_entities[clientEnt->client->ps.cursorHintEntIndex];
    if (IsLiveGrenade(grenadeEnt))
    {
        if (clientEnt->client->ps.throwBackGrenadeTimeLeft)
        {
            touch = entityHandlers[grenadeEnt->handler].touch;
            if (touch)
            {
                if (grenadeEnt->parent.isDefined())
                    clientEnt->client->ps.throwBackGrenadeOwner = grenadeEnt->parent.entnum();
                else
                    clientEnt->client->ps.throwBackGrenadeOwner = 1022;
                if ((clientEnt->client->ps.perks[1] & 0x10) != 0)
                {
                    if (clientEnt->client->ps.throwBackGrenadeTimeLeft >= perk_grenadeTossBackTimer->current.integer)
                        clientEnt->client->ps.grenadeTimeLeft = clientEnt->client->ps.throwBackGrenadeTimeLeft;
                    else
                        clientEnt->client->ps.grenadeTimeLeft = perk_grenadeTossBackTimer->current.integer;
                }
                else
                {
                    clientEnt->client->ps.grenadeTimeLeft = clientEnt->client->ps.throwBackGrenadeTimeLeft;
                }
                weapIdx = grenadeEnt->s.un3.item % 2048;
                touch(grenadeEnt, clientEnt, 0);
                if (!clientEnt->client->ps.throwBackGrenadeTimeLeft
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp",
                        576,
                        0,
                        "%s",
                        "clientEnt->client->ps.throwBackGrenadeTimeLeft"))
                {
                    __debugbreak();
                }
                if (clientEnt->client->ps.offHandIndex != weapIdx)
                {
                    BG_AddPredictableEventToPlayerstate(0x2Bu, weapIdx, &clientEnt->client->ps);
                    clientEnt->client->ps.offHandIndex = weapIdx;
                }
            }
        }
    }
}

int __thiscall EntHandle::entnum() const
{
    if ( (unsigned int)this->number - 1 >= 0x3FF
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\game\\../game_mp/g_public_mp.h",
                    404,
                    0,
                    "number - 1 doesn't index ENTITYNUM_NONE\n\t%i not in [0, %i)",
                    this->number - 1,
                    1023) )
    {
        __debugbreak();
    }
    if ( !g_entities[this->number - 1].r.inuse
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\game\\../game_mp/g_public_mp.h",
                    405,
                    0,
                    "%s\n\t(number - 1) = %i",
                    "(g_entities[number - 1].r.inuse)",
                    this->number - 1) )
    {
        __debugbreak();
    }
    return this->number - 1;
}

bool __cdecl IsLiveGrenade(gentity_s *ent)
{
    const WeaponDef *weapDef; // [esp+0h] [ebp-4h]

    if ( ent->s.eType != 4 )
        return 0;
    weapDef = BG_GetWeaponDef(ent->s.un3.item % 2048);
    if ( !weapDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp", 524, 0, "%s", "weapDef") )
    {
        __debugbreak();
    }
    return weapDef->bThrowBack;
}

void __cdecl ClientEvents(gentity_s *ent, __int16 oldEventSequence)
{
    gclient_s *client; // [esp+0h] [ebp-10h]
    __int16 i; // [esp+Ch] [ebp-4h]

    client = ent->client;
    if ( oldEventSequence < client->ps.predictableEventSequence - 4 )
        oldEventSequence = client->ps.predictableEventSequence - 4;
    if ( oldEventSequence > client->ps.predictableEventSequence + 64 )
        oldEventSequence -= 256;
    for ( i = oldEventSequence; i < client->ps.predictableEventSequence; ++i )
        HandleClientEvent(client, ent, client->ps.predictableEvents[i & 3], client->ps.predictableEventParms[i & 3]);
}

void __cdecl G_SetLastServerTime(int clientNum, int lastServerTime)
{
    gentity_s *ent; // [esp+0h] [ebp-4h]

    ent = &g_entities[clientNum];
    if ( !ent->client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp", 817, 0, "%s", "ent->client") )
    {
        __debugbreak();
    }
    if ( level.time - lastServerTime > 1000 )
        lastServerTime = level.time - 1000;
    if ( lastServerTime >= ent->client->lastServerTime || level.time <= lastServerTime )
        ent->client->lastServerTime = lastServerTime;
}

void __cdecl G_SetClientContents(gentity_s *pEnt)
{
    if ( !pEnt->client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp", 839, 0, "%s", "pEnt->client") )
    {
        __debugbreak();
    }
    if ( (pEnt->client->flags & 3) != 0 )
    {
        pEnt->r.contents = 0;
    }
    else if ( pEnt->client->sess.sessionState == SESS_STATE_DEAD )
    {
        pEnt->r.contents = 0;
    }
    else
    {
        pEnt->r.contents = 0x2000000;
    }
}

void __cdecl ClientVehicleInteraction(gentity_s *ent)
{
    gclient_s *client; // [esp+0h] [ebp-8h]
    gentity_s *vehicle; // [esp+4h] [ebp-4h]

    client = ent->client;
    if ( !client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp", 921, 0, "%s", "client") )
    {
        __debugbreak();
    }
    if ( (client->ps.eFlags & 0x4000) != 0 && (client->ps.eFlags & 0x10000) == 0 && ent->health > 0 )
    {
        iassert(ent->r.ownerNum.isDefined());
        vehicle = ent->r.ownerNum.ent();
        if ( client->latched_button_bits.testBit(0x1Cu) && client->ps.fWeaponPosFrac == 0.0 )
        {
            VEH_SwitchClientToNextSeat(vehicle, ent);
            client->latched_button_bits.resetBit(0x1Cu);
        }
        if ( (vehicle->r.contents & 0x200000) != 0
            && (client->latched_button_bits.testBit(3u)
             || client->latched_button_bits.testBit(5u)) )
        {
            VEH_UnlinkPlayer(ent, 0, (char*)"G_VehicleFinishedAnimating");
        }
    }
}

void __cdecl ClientVehicleJumpOff(gentity_s *ent)
{
    float *velocity; // [esp+0h] [ebp-30h]
    float xyVec[3]; // [esp+1Ch] [ebp-14h] BYREF
    gclient_s *client; // [esp+28h] [ebp-8h]
    gentity_s *groundEnt; // [esp+2Ch] [ebp-4h]

    client = ent->client;
    if ( !client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp", 982, 0, "%s", "client") )
    {
        __debugbreak();
    }
    if ( client->sess.connected == CON_CONNECTED
        && client->ps.groundEntityNum != 1022
        && client->ps.groundEntityNum != 1023 )
    {
        groundEnt = &g_entities[client->ps.groundEntityNum];
        if ( groundEnt->s.eType == 14 )
        {
            client->ps.pm_flags &= 0xFFDFBE7F;
            client->ps.pm_flags |= 0x200100u;
            client->ps.pm_time = 4000;
            xyVec[0] = client->ps.origin[0] - groundEnt->r.currentOrigin[0];
            xyVec[1] = client->ps.origin[1] - groundEnt->r.currentOrigin[1];
            xyVec[2] = 0.0f;
            Vec3Normalize(xyVec);
            xyVec[0] = xySpeed * xyVec[0];
            xyVec[1] = xySpeed * xyVec[1];
            xyVec[2] = zBoost;
            velocity = client->ps.velocity;
            client->ps.velocity[0] = xyVec[0];
            velocity[1] = xyVec[1];
            velocity[2] = xyVec[2];
        }
    }
}

void __cdecl G_PlayerVehiclePositionAndBlend(gentity_s *ent, gentity_s *pTurretEnt)
{
    const char *v2; // eax
    const XModel *Model; // eax
    const char *Name; // eax
    const XModel *v5; // eax
    unsigned __int8 *v6; // eax
    char *v7; // eax
    float v8; // xmm1_4
    const char *AnimDebugName; // eax
    const char *v10; // eax
    double v11; // st7
    float *v12; // [esp+1Ch] [ebp-2DCh]
    float *origin; // [esp+24h] [ebp-2D4h]
    float *v14; // [esp+54h] [ebp-2A4h]
    float v15; // [esp+58h] [ebp-2A0h]
    float *v16; // [esp+5Ch] [ebp-29Ch]
    float *v17; // [esp+60h] [ebp-298h]
    float v18; // [esp+64h] [ebp-294h]
    float *v19; // [esp+68h] [ebp-290h]
    float fHeightRatio; // [esp+DCh] [ebp-21Ch]
    unsigned __int16 tagName; // [esp+E0h] [ebp-218h]
    unsigned __int16 tagNamea; // [esp+E0h] [ebp-218h]
    float plyrAxis[3][3]; // [esp+E4h] [ebp-214h] BYREF
    int iPrevBlend; // [esp+108h] [ebp-1F0h]
    float fPrevTransZ; // [esp+10Ch] [ebp-1ECh]
    DObj *obj; // [esp+110h] [ebp-1E8h]
    int numVertChildren; // [esp+114h] [ebp-1E4h]
    DObj *turretObj; // [esp+118h] [ebp-1E0h]
    float trans2[3]; // [esp+11Ch] [ebp-1DCh] BYREF
    float tmpAxis[3][3]; // [esp+128h] [ebp-1D0h] BYREF
    float yaw; // [esp+14Ch] [ebp-1ACh]
    float trans[3]; // [esp+150h] [ebp-1A8h] BYREF
    unsigned __int16 gunnerTags[4]; // [esp+15Ch] [ebp-19Ch]
    int iBlend; // [esp+164h] [ebp-194h]
    DObjAnimMat *tagMat; // [esp+168h] [ebp-190h]
    unsigned int heightAnim; // [esp+16Ch] [ebp-18Ch]
    float fDelta; // [esp+170h] [ebp-188h]
    float fPrevBlend; // [esp+174h] [ebp-184h]
    float rot[2]; // [esp+178h] [ebp-180h] BYREF
    float tagAxis[4][3]; // [esp+180h] [ebp-178h] BYREF
    unsigned int leafAnim1; // [esp+1B0h] [ebp-148h]
    trace_t trace; // [esp+1B4h] [ebp-144h] BYREF
    int numHorChildren; // [esp+1F0h] [ebp-108h]
    clientInfo_t *ci; // [esp+1F4h] [ebp-104h]
    float tagHeight; // [esp+1F8h] [ebp-100h]
    int gunnerNum; // [esp+1FCh] [ebp-FCh]
    int i; // [esp+200h] [ebp-F8h]
    unsigned int baseAnim; // [esp+204h] [ebp-F4h]
    float vehTagOriginMat[4][3]; // [esp+208h] [ebp-F0h] BYREF
    float vehTagOriginMatInv[4][3]; // [esp+238h] [ebp-C0h] BYREF
    int clientNum; // [esp+268h] [ebp-90h]
    lerpFrame_t *pLerpAnim; // [esp+26Ch] [ebp-8Ch]
    float fBlend; // [esp+270h] [ebp-88h]
    float axis[4][3]; // [esp+274h] [ebp-84h] BYREF
    XAnimTree_s *pAnimTree; // [esp+2A4h] [ebp-54h]
    XAnim_s *pXAnims; // [esp+2A8h] [ebp-50h]
    playerState_s *ps; // [esp+2ACh] [ebp-4Ch]
    bool isT34; // [esp+2B3h] [ebp-45h]
    unsigned int leafAnim2; // [esp+2B4h] [ebp-44h]
    float localYaw; // [esp+2B8h] [ebp-40h]
    float turretAxis[4][3]; // [esp+2BCh] [ebp-3Ch] BYREF
    float vDelta[3]; // [esp+2ECh] [ebp-Ch]

    clientNum = ent->s.clientNum;
    if ( (unsigned int)clientNum >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp",
                    1071,
                    0,
                    "clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                    clientNum,
                    32) )
    {
        __debugbreak();
    }
    ci = &level_bgs.clientinfo[clientNum];
    if ( !ci->infoValid
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp", 1073, 0, "%s", "ci->infoValid") )
    {
        __debugbreak();
    }
    ps = &ent->client->ps;
    pLerpAnim = &ci->legs;
    if ( ci->legs.animationNumber )
    {
        if ( pLerpAnim->animation )
        {
            if ( (pLerpAnim->animation->flags & 4) != 0 )
            {
                turretObj = Com_GetServerDObj(pTurretEnt->s.number);
                if ( turretObj )
                {
                    gunnerTags[0] = scr_const.tag_gunner1;
                    gunnerTags[1] = scr_const.tag_gunner2;
                    gunnerTags[2] = scr_const.tag_gunner3;
                    gunnerTags[3] = scr_const.tag_gunner4;
                    gunnerNum = ent->s.lerp.u.player.vehicleSeat - 1;
                    if ( (unsigned int)gunnerNum >= 4
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp",
                                    1101,
                                    0,
                                    "%s",
                                    "gunnerNum >= 0 && gunnerNum < VEH_MAX_GUNNERS") )
                    {
                        __debugbreak();
                    }
                    tagName = gunnerTags[gunnerNum];
                    if ( !G_DObjGetWorldTagMatrix(pTurretEnt, scr_const.tag_origin, tagAxis)
                        && !G_DObjGetWorldTagMatrix(pTurretEnt, scr_const.tag_origin, tagAxis)
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp",
                                    1107,
                                    0,
                                    "%s",
                                    "G_DObjGetWorldTagMatrix( pTurretEnt, scr_const.tag_origin, tagAxis )") )
                    {
                        __debugbreak();
                    }
                    *(_QWORD *)&vehTagOriginMat[0][0] = *(_QWORD *)&tagAxis[0][0];
                    vehTagOriginMat[0][2] = tagAxis[0][2];
                    vehTagOriginMat[1][0] = tagAxis[1][0];
                    vehTagOriginMat[1][1] = tagAxis[1][1];
                    vehTagOriginMat[1][2] = tagAxis[1][2];
                    vehTagOriginMat[2][0] = tagAxis[2][0];
                    vehTagOriginMat[2][1] = tagAxis[2][1];
                    vehTagOriginMat[2][2] = tagAxis[2][2];
                    vehTagOriginMat[3][0] = tagAxis[3][0];
                    vehTagOriginMat[3][1] = tagAxis[3][1];
                    vehTagOriginMat[3][2] = tagAxis[3][2];
                    MatrixInverseOrthogonal43(vehTagOriginMat, vehTagOriginMatInv);
                    if ( !G_DObjGetWorldTagMatrix(pTurretEnt, tagName, tagAxis)
                        && !G_DObjGetWorldTagMatrix(pTurretEnt, tagName, tagAxis)
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp",
                                    1117,
                                    0,
                                    "%s",
                                    "G_DObjGetWorldTagMatrix( pTurretEnt, tagName, tagAxis )") )
                    {
                        __debugbreak();
                    }
                    MatrixMultiply(tagAxis, vehTagOriginMatInv, tmpAxis);
                    AxisToAngles(tmpAxis, ent->r.currentAngles);
                    MatrixTransformVector43Equals(tagAxis[3], vehTagOriginMatInv[0]);
                    ent->r.currentOrigin[0] = tagAxis[3][0];
                    ent->r.currentOrigin[1] = tagAxis[3][1];
                    ent->r.currentOrigin[2] = tagAxis[3][2];
                    tagNamea = scr_const.tag_gunner_barrel1;
                    tagMat = G_DObjGetLocalTagMatrix(pTurretEnt, scr_const.tag_gunner_barrel1);
                    if ( tagMat )
                    {
                        obj = Com_GetServerDObj(ent->s.number);
                        if ( obj )
                        {
                            pAnimTree = ci->pXAnimTree;
                            pXAnims = level_bgs.animData->animScriptData.animTree.anims;
                            baseAnim = pLerpAnim->animationNumber & 0xFFFFFBFF;
                            ConvertQuatToMat(tagMat, tagAxis);
                            yaw = vectosignedyaw(tagAxis[0]);
                            localYaw = AngleNormalize180(yaw - pTurretEnt->r.currentAngles[1]);
                            numVertChildren = XAnimGetNumChildren(pXAnims, baseAnim);
                            fPrevTransZ = 0.0f;
                            fPrevBlend = 0.0f;
                            iPrevBlend = 0;
                            leafAnim2 = 0;
                            isT34 = 0;
                            Model = DObjGetModel(turretObj, 0);
                            Name = XModelGetName(Model);
                            if ( I_stristr(Name, "panzer") )
                            {
                                v17 = tagMat->trans;
                                v18 = handOfs_2;
                                v19 = tagMat->trans;
                                tagMat->trans[0] = (float)(handOfs_2 * tagAxis[0][0]) + tagMat->trans[0];
                                v17[1] = (float)(v18 * tagAxis[0][1]) + v19[1];
                                v17[2] = (float)(v18 * tagAxis[0][2]) + v19[2];
                            }
                            else
                            {
                                v5 = DObjGetModel(turretObj, 0);
                                v6 = (unsigned __int8 *)XModelGetName(v5);
                                v7 = strstr((char*)v6, "t34");
                                if ( v7 )
                                {
                                    v14 = tagMat->trans;
                                    v15 = handOfs_1;
                                    v16 = tagMat->trans;
                                    tagMat->trans[0] = (float)(handOfs_1 * tagAxis[0][0]) + tagMat->trans[0];
                                    v14[1] = (float)(v15 * tagAxis[0][1]) + v16[1];
                                    v14[2] = (float)(v15 * tagAxis[0][2]) + v16[2];
                                    isT34 = 1;
                                }
                            }
                            AnglesToAxis(vec3_origin, turretAxis);
                            memset(turretAxis[3], 0, sizeof(float[3]));
                            localYaw = 0.0f;
                            vDelta[0] = ent->r.currentOrigin[0] - 0.0;
                            vDelta[1] = ent->r.currentOrigin[1] - 0.0;
                            vDelta[2] = ent->r.currentOrigin[2] - 0.0;
                            tagHeight = (float)((float)(vDelta[0] * turretAxis[2][0]) + (float)(vDelta[1] * turretAxis[2][1]))
                                                + (float)(vDelta[2] * turretAxis[2][2]);
                            if ( ent->client->ps.leanf > 0.0 )
                            {
                                if ( isT34 )
                                    v8 = (float)((float)(1.0 - ent->client->ps.leanf) * tagMat->trans[2])
                                         + (float)(fCrouchHeightT34_0 * ent->client->ps.leanf);
                                else
                                    v8 = (float)((float)(1.0 - ent->client->ps.leanf) * tagMat->trans[2])
                                         + (float)(fCrouchHeightPanzer_0 * ent->client->ps.leanf);
                                tagMat->trans[2] = v8;
                            }
                            fDelta = tagHeight - (float)(tagMat->trans[2] - turretAxis[3][2]);
                            if ( !numVertChildren )
                            {
                                AnimDebugName = XAnimGetAnimDebugName(pXAnims, baseAnim);
                                Com_Error(ERR_DROP, "player anim '%s' has no children", AnimDebugName);
                            }
                            i = 0;
                            do
                            {
                                heightAnim = XAnimGetChildAt(pXAnims, baseAnim, numVertChildren - 1 - i);
                                numHorChildren = XAnimGetNumChildren(pXAnims, heightAnim);
                                if ( !numHorChildren )
                                {
                                    v10 = XAnimGetAnimDebugName(pXAnims, heightAnim);
                                    Com_Error(ERR_DROP, "player anim '%s' has no children", v10);
                                }
                                fBlend = (float)numHorChildren * 0.5;
                                if ( fBlend >= 0.0 )
                                {
                                    if ( fBlend >= (float)(numHorChildren - 1) )
                                        fBlend = (float)(numHorChildren - 1);
                                }
                                else
                                {
                                    fBlend = 0.0f;
                                }
                                iBlend = (int)fBlend;
                                fBlend = fBlend - (float)(int)fBlend;
                                leafAnim1 = XAnimGetChildAt(pXAnims, heightAnim, iBlend);
                                XAnimGetAbsDelta(pXAnims, leafAnim1, rot, trans, 0.0);
                                if ( fBlend != 0.0 )
                                {
                                    leafAnim2 = XAnimGetChildAt(pXAnims, heightAnim, iBlend + 1);
                                    XAnimGetAbsDelta(pXAnims, leafAnim2, rot, trans2, 0.0);
                                    Vec3Lerp(trans, trans2, fBlend, trans);
                                }
                                if ( trans[2] >= fDelta )
                                    break;
                                fPrevTransZ = trans[2];
                                iPrevBlend = iBlend;
                                fPrevBlend = fBlend;
                                ++i;
                            }
                            while ( i < numVertChildren );
                            XAnimClearTreeGoalWeightsStrict(pAnimTree, baseAnim, 0.0, -1);
                            XAnimSetGoalWeight(obj, leafAnim1, 1.0 - fBlend, 0.0, 1.0, 0, 0, 0, -1);
                            if ( fBlend != 0.0 )
                                XAnimSetGoalWeight(obj, leafAnim2, fBlend, 0.0, 1.0, 0, 0, 0, -1);
                            if ( i && i != numVertChildren )
                            {
                                if ( (float)(trans[2] - fPrevTransZ) == 0.0
                                    && !Assert_MyHandler(
                                                "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp",
                                                1245,
                                                0,
                                                "%s",
                                                "trans[2] - fPrevTransZ") )
                                {
                                    __debugbreak();
                                }
                                fHeightRatio = (float)(fDelta - fPrevTransZ) / (float)(trans[2] - fPrevTransZ);
                                XAnimSetGoalWeight(obj, heightAnim, fHeightRatio, 0.0, 1.0, 0, 0, 0, -1);
                                heightAnim = XAnimGetChildAt(pXAnims, baseAnim, numVertChildren - i);
                                XAnimSetGoalWeight(obj, heightAnim, 1.0 - fHeightRatio, 0.0, 1.0, 0, 0, 0, -1);
                                leafAnim1 = XAnimGetChildAt(pXAnims, heightAnim, iPrevBlend);
                                XAnimSetGoalWeight(obj, leafAnim1, 1.0 - fPrevBlend, 0.0, 1.0, 0, 0, 0, -1);
                                if ( fPrevBlend != 0.0 )
                                {
                                    leafAnim2 = XAnimGetChildAt(pXAnims, heightAnim, iPrevBlend + 1);
                                    XAnimSetGoalWeight(obj, leafAnim2, fPrevBlend, 0.0, 1.0, 0, 0, 0, -1);
                                }
                            }
                            else
                            {
                                XAnimSetGoalWeight(obj, heightAnim, 1.0, 0.0, 1.0, 0, 0, 0, -1);
                            }
                            XAnimCalcAbsDelta(obj, baseAnim, rot, trans);
                            VectorAngleMultiply(trans, yaw);
                            axis[3][0] = trans[0] + tagMat->trans[0];
                            axis[3][1] = trans[1] + tagMat->trans[1];
                            axis[3][2] = tagHeight + turretAxis[3][2];
                            v11 = RotationToYaw(rot);
                            yaw = v11 + yaw;
                            YawToAxis(yaw, axis);
                            AxisToAngles(axis, ent->r.currentAngles);
                            ent->r.currentOrigin[0] = axis[3][0];
                            ent->r.currentOrigin[1] = axis[3][1];
                            ent->r.currentOrigin[2] = axis[3][2];
                            AnglesToAxis(ent->r.currentAngles, plyrAxis);
                            MatrixMultiply(plyrAxis, vehTagOriginMat, tmpAxis);
                            AxisToAngles(tmpAxis, ent->r.currentAngles);
                            MatrixTransformVector43Equals(ent->r.currentOrigin, vehTagOriginMat[0]);
                            origin = ent->client->ps.origin;
                            *origin = ent->r.currentOrigin[0];
                            origin[1] = ent->r.currentOrigin[1];
                            origin[2] = ent->r.currentOrigin[2];
                            v12 = ent->client->ps.origin;
                            *v12 = ent->r.currentOrigin[0];
                            v12[1] = ent->r.currentOrigin[1];
                            v12[2] = ent->r.currentOrigin[2];
                            BG_PlayerStateToEntityState(&ent->client->ps, &ent->s, 1, 1u);
                            ent->r.mins[0] = -gunnerXYradius;
                            ent->r.mins[1] = -gunnerXYradius;
                            ent->r.mins[2] = 0.0f;
                            ent->r.maxs[0] = gunnerXYradius;
                            ent->r.maxs[1] = gunnerXYradius;
                            ent->r.maxs[2] = gunnerZheight;
                            SV_LinkEntity(ent);
                        }
                    }
                    else
                    {
                        v2 = SL_ConvertToString(tagNamea, SCRIPTINSTANCE_SERVER);
                        Com_PrintWarning(17, "WARNING: aborting player positioning on turret since '%s' does not exist\n", v2);
                    }
                }
            }
        }
    }
}

void __cdecl ClientThink_real(gentity_s *ent, usercmd_s *ucmd)
{
    int v2; // ecx
    float *origin; // [esp+1Ch] [ebp-240h]
    float *maxs; // [esp+2Ch] [ebp-230h]
    float *mins; // [esp+34h] [ebp-228h]
    float v6; // [esp+8Ch] [ebp-1D0h]
    bitarray<51> *v7; // [esp+98h] [ebp-1C4h]
    int i2; // [esp+9Ch] [ebp-1C0h]
    bitarray<51> *v9; // [esp+A0h] [ebp-1BCh]
    int i1; // [esp+A4h] [ebp-1B8h]
    bitarray<51> *p_button_bits; // [esp+A8h] [ebp-1B4h]
    int nn; // [esp+ACh] [ebp-1B0h]
    char v13; // [esp+B3h] [ebp-1A9h]
    int mm; // [esp+B4h] [ebp-1A8h]
    bitarray<51> *p_button_bitsSinceLastFrame; // [esp+B8h] [ebp-1A4h]
    int kk; // [esp+BCh] [ebp-1A0h]
    bitarray<51> *p_latched_button_bits; // [esp+C0h] [ebp-19Ch]
    int jj; // [esp+C4h] [ebp-198h]
    int ii; // [esp+C8h] [ebp-194h]
    int n; // [esp+CCh] [ebp-190h]
    int m; // [esp+D0h] [ebp-18Ch]
    int k; // [esp+D4h] [ebp-188h]
    int j; // [esp+D8h] [ebp-184h]
    int i; // [esp+DCh] [ebp-180h]
    gentity_s *vehicle; // [esp+E0h] [ebp-17Ch]
    float loc2d; // [esp+E4h] [ebp-178h]
    float loc2d_4; // [esp+E8h] [ebp-174h]
    float loc[3]; // [esp+ECh] [ebp-170h] BYREF
    int yaw; // [esp+F8h] [ebp-164h]
    gclient_s *client; // [esp+104h] [ebp-158h]
    float vAxis3[3][3]; // [esp+108h] [ebp-154h] BYREF
    weaponState_t ws; // [esp+12Ch] [ebp-130h] BYREF
    int msec; // [esp+1C0h] [ebp-9Ch]
    bitarray<51> temp_oldbits; // [esp+1C4h] [ebp-98h]
    __int16 oldEventSequence; // [esp+1CCh] [ebp-90h]
    float vAxis2[3][3]; // [esp+1D0h] [ebp-8Ch] BYREF
    float angles[3]; // [esp+1F4h] [ebp-68h] BYREF
    pmove_t *pm; // [esp+200h] [ebp-5Ch]
    float viewangles[3]; // [esp+204h] [ebp-58h] BYREF
    viewState_t vs; // [esp+210h] [ebp-4Ch] BYREF
    const WeaponDef *weapDef; // [esp+234h] [ebp-28h]
    float vAxis[3][3]; // [esp+238h] [ebp-24h] BYREF
    int savedregs; // [esp+25Ch] [ebp+0h] BYREF

    client = ent->client;
    pm = &g_pmove[client->ps.clientNum];
    if ( client->sess.connected == CON_CONNECTED )
    {
        if ( ucmd->serverTime > level.time + 200 )
            ucmd->serverTime = level.time + 200;
        if ( ucmd->serverTime < level.time - 1000 )
            ucmd->serverTime = level.time - 1000;
        msec = ucmd->serverTime - client->ps.commandTime;
        if ( msec >= 1 || client->ps.clientNum != ent - g_entities )
        {
            if ( msec > 200 )
                msec = 200;
            if ( client->sess.sessionState == SESS_STATE_INTERMISSION )
            {
                ClientIntermissionThink(ent);
            }
            else if ( client->sess.sessionState == SESS_STATE_SPECTATOR )
            {
                SpectatorThink(ent, ucmd);
            }
            else if ( ClientInactivityTimer(client) )
            {
                for ( i = 0; i < 2; ++i )
                    client->oldbutton_bits.array[i] = client->button_bits.array[i];
                if ( !client->useButtonDone )
                {
                    client->oldbutton_bits.resetBit(3u);
                    client->oldbutton_bits.resetBit(5u);
                }
                for ( j = 0; j < 2; ++j )
                    client->button_bits.array[j] = client->sess.cmd.button_bits.array[j];
                if ( !client->button_bits.testBit(3u) && !client->button_bits.testBit(5u) )
                    client->useButtonDone = 0;
                for ( k = 0; k < 2; ++k )
                    temp_oldbits.array[k] = 0;
                for ( m = 0; m < 2; ++m )
                    client->latched_button_bits.array[m] = client->button_bits.array[m];
                for ( n = 0; n < 2; ++n )
                    temp_oldbits.array[n] = client->oldbutton_bits.array[n];
                for ( ii = 0; ii < 2; ++ii )
                    temp_oldbits.array[ii] = ~temp_oldbits.array[ii];
                p_latched_button_bits = &client->latched_button_bits;
                for ( jj = 0; jj < 2; ++jj )
                    p_latched_button_bits->array[jj] &= temp_oldbits.array[jj];
                p_button_bitsSinceLastFrame = &client->button_bitsSinceLastFrame;
                for ( kk = 0; kk < 2; ++kk )
                    p_button_bitsSinceLastFrame->array[kk] |= client->latched_button_bits.array[kk];
                if ( client->latched_button_bits.testBit(0x32u) )
                    Scr_Notify(ent, scr_const.doubletap_detonate, 0);
                ClientVehicleInteraction(ent);
                ClientVehicleJumpOff(ent);
                if ( client->ps.locationSelectionInfo )
                {
                    for ( mm = 0; mm < 2; ++mm )
                    {
                        if ( client->button_bitsSinceLastFrame.array[mm] )
                        {
                            v13 = 1;
                            goto LABEL_53;
                        }
                    }
                    v13 = 0;
LABEL_53:
                    if ( v13 )
                    {
                        yaw = 2 * ucmd->selectedYaw;
                        loc2d = (float)((float)((float)ucmd->selectedLocation[0] + 128.0) / 255.0) * level.compassMapWorldSize[0];
                        loc2d_4 = (float)((float)((float)ucmd->selectedLocation[1] + 128.0) / 255.0) * level.compassMapWorldSize[1];
                        loc[0] = (float)((float)(loc2d * level.compassNorth[1]) + level.compassMapUpperLeft[0])
                                     - (float)(loc2d_4 * level.compassNorth[0]);
                        loc[1] = (float)(level.compassMapUpperLeft[1] - (float)(loc2d * level.compassNorth[0]))
                                     - (float)(loc2d_4 * level.compassNorth[1]);
                        loc[2] = 0.0f;
                        if ( client->button_bitsSinceLastFrame.testBit(0x10u) )
                        {
                            Scr_AddInt(yaw, SCRIPTINSTANCE_SERVER);
                            Scr_AddVector(loc, SCRIPTINSTANCE_SERVER);
                            Scr_Notify(ent, scr_const.confirm_location, 2u);
                        }
                    }
                    if ( client->button_bitsSinceLastFrame.testBit(0x11u) )
                        Scr_Notify(ent, scr_const.cancel_location, 0);
                    bitarray<51> mask_bits(0xCu, 8, 9, -1); // [esp+FCh] [ebp-160h] BYREF
                    //bitarray<51>::bitarray<51>(&mask_bits, 0xCu, 8, 9, -1);
                    p_button_bits = &client->button_bits;
                    for ( nn = 0; nn < 2; ++nn )
                        p_button_bits->array[nn] &= mask_bits.array[nn];
                    v9 = &client->latched_button_bits;
                    for ( i1 = 0; i1 < 2; ++i1 )
                        v9->array[i1] &= mask_bits.array[i1];
                    v7 = &client->button_bitsSinceLastFrame;
                    for ( i2 = 0; i2 < 2; ++i2 )
                        v7->array[i2] &= mask_bits.array[i2];
                }
                oldEventSequence = client->ps.predictableEventSequence;
                memset(pm, 0, 0x258u);
                pm->localClientNum = -1;
                pm->ps = &client->ps;
                memcpy(&pm->cmd, ucmd, sizeof(pm->cmd));
                memcpy(&pm->oldcmd, &client->sess.oldcmd, sizeof(pm->oldcmd));
                if ( pm->ps->pm_type < 9 )
                    pm->tracemask = 0x2818011;
                else
                    pm->tracemask = 0x810011;
                pm->handler = 1;
                vs.ps = &client->ps;
                vs.damageTime = client->damageTime;
                vs.time = level.time;
                vs.v_dmg_pitch = client->v_dmg_pitch;
                vs.v_dmg_roll = client->v_dmg_roll;
                if ( (client->ps.pm_flags & 8) != 0 )
                {
                    if ( level.time - client->ps.jumpTime >= 500 )
                        v6 = client->ps.velocity[2];
                    else
                        v6 = 0.0f;
                }
                else
                {
                    v6 = Vec2Length(client->ps.velocity);
                }
                vs.xyspeed = v6;
                vs.frametime = (float)msec * 0.001;
                vs.fLastIdleFactor = client->fLastIdleFactor;
                vs.weapIdleTime = &client->weapIdleTime;
                client->ps.speed = g_speed->current.integer;
                if ( (client->ps.perks[1] & 1) != 0 )
                    client->ps.speed = (int)(float)((float)client->ps.speed * perk_speedMultiplier->current.value);
                BG_CalculateViewMovementAngles(&vs, angles);
                viewangles[0] = client->ps.viewangles[0] + angles[0];
                viewangles[1] = client->ps.viewangles[1] + angles[1];
                viewangles[2] = client->ps.viewangles[2] + angles[2];
                weapDef = BG_GetWeaponDef(client->ps.weapon);
                G_BuildWeaponState(client, &vs, &ws);
                BG_CalculateWeaponMovement(&ws, 0, angles);
                G_SaveWeaponState(&ws, client);
                if ( level.time > client->ps.shellshockDuration + client->ps.shellshockTime )
                    client->ps.pm_flags &= ~0x10000u;
                if ( BG_IsAimDownSightWeapon(ws.ps->weapon)
                    && ws.ps->fWeaponPosFrac != 0.0
                    && weapDef->overlayReticle == WEAPOVERLAYRETICLE_NONE )
                {
                    AnglesToAxis(angles, vAxis);
                    AnglesToAxis(viewangles, vAxis2);
                    MatrixMultiply(vAxis, vAxis2, vAxis3);
                    AxisToAngles(vAxis3, viewangles);
                }
                if ( ((LODWORD(viewangles[0]) & 0x7F800000) == 0x7F800000
                     || (LODWORD(viewangles[1]) & 0x7F800000) == 0x7F800000
                     || (LODWORD(viewangles[2]) & 0x7F800000) == 0x7F800000)
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp",
                                1605,
                                0,
                                "%s",
                                "!IS_NAN((viewangles)[0]) && !IS_NAN((viewangles)[1]) && !IS_NAN((viewangles)[2])") )
                {
                    __debugbreak();
                }
                client->fGunPitch = viewangles[0];
                client->fGunYaw = viewangles[1];
                if ( Player_IsPlayerUsingTurretNearby(ent) )
                    v2 = client->ps.mantleState.flags | 0x200;
                else
                    v2 = client->ps.mantleState.flags & 0xFFFFFDFF;
                client->ps.mantleState.flags = v2;
                if ( g_debugServerAiming->current.enabled )
                    G_DrawServerAiming(client);
                if ( pm->mantleStarted
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp",
                                1624,
                                0,
                                "%s",
                                "!pm.mantleStarted") )
                {
                    __debugbreak();
                }
                Pmove(pm);
                if ( pm->mantleStarted )
                {
                    if ( (client->ps.pm_flags & 4) == 0
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp",
                                    1632,
                                    0,
                                    "%s",
                                    "client->ps.pm_flags & PMF_MANTLE") )
                    {
                        __debugbreak();
                    }
                    G_AddPlayerMantleBlockage(pm->mantleEndPos, pm->mantleDuration, pm);
                }
                if ( pm->vehAnimState )
                    G_VehicleFinishedAnimating(ent, pm->vehAnimState);
                if ( client->ps.predictableEventSequence != oldEventSequence )
                {
                    ent->eventTime = level.time;
                    ent->r.eventTime = level.time;
                }
                if ( g_smoothClients->current.enabled )
                    G_PlayerStateToEntityStateExtrapolate(&client->ps, &ent->s, client->ps.commandTime, 1);
                else
                    BG_PlayerStateToEntityState(&client->ps, &ent->s, 1, 1u);
                if ( (client->ps.eFlags & 0x4000) != 0 && client->ps.vehiclePos >= 1 && client->ps.vehiclePos <= 4 )
                {
                    iassert(ent->r.ownerNum.isDefined());

                    vehicle = ent->r.ownerNum.ent();
                    G_PlayerVehiclePositionAndBlend(ent, vehicle);
                    ent->s.lerp.pos.trBase[0] = ent->r.currentOrigin[0];
                    ent->s.lerp.pos.trBase[1] = ent->r.currentOrigin[1];
                    ent->s.lerp.pos.trBase[2] = ent->r.currentOrigin[2];
                }
                else
                {
                    ent->r.currentOrigin[0] = ent->s.lerp.pos.trBase[0];
                    ent->r.currentOrigin[1] = ent->s.lerp.pos.trBase[1];
                    ent->r.currentOrigin[2] = ent->s.lerp.pos.trBase[2];
                    mins = pm->mins;
                    ent->r.mins[0] = pm->mins[0];
                    ent->r.mins[1] = mins[1];
                    ent->r.mins[2] = mins[2];
                    maxs = pm->maxs;
                    ent->r.maxs[0] = pm->maxs[0];
                    ent->r.maxs[1] = maxs[1];
                    ent->r.maxs[2] = maxs[2];
                }
                if ( (ent->s.lerp.eFlags2 & 0x10000000) != 0 )
                {
                    ent->s.lerp.apos.trBase[0] = ent->r.currentAngles[0];
                    ent->s.lerp.apos.trBase[1] = ent->r.currentAngles[1];
                    ent->s.lerp.apos.trBase[2] = ent->r.currentAngles[2];
                }
                ClientEvents(ent, oldEventSequence);
                SV_LinkEntity(ent);
                origin = client->ps.origin;
                ent->r.currentOrigin[0] = client->ps.origin[0];
                ent->r.currentOrigin[1] = origin[1];
                ent->r.currentOrigin[2] = origin[2];
                if ( (client->ps.eFlags & 0x300) == 0
                    && ((client->ps.eFlags & 0x4000) == 0
                     || (client->ps.vehiclePos < 1 || client->ps.vehiclePos > 4) && (client->ps.eFlags2 & 0x10000000) == 0) )
                {
                    ent->r.currentAngles[0] = 0.0f;
                    ent->r.currentAngles[1] = 0.0f;
                    ent->r.currentAngles[2] = 0.0f;
                    ent->r.currentAngles[1] = client->ps.viewangles[1];
                }
                ClientImpacts(ent, pm);
                Player_WaterUpdate(ent);
                if ( client->ps.predictableEventSequence != oldEventSequence )
                    ent->eventTime = level.time;
                Player_UpdateActivate(ent);
            }
        }
    }
}

void __cdecl Player_WaterUpdate(gentity_s *ent)
{
    int waterDamage; // [esp+4h] [ebp-28h]
    gclient_s *client; // [esp+10h] [ebp-1Ch]
    float dir[3]; // [esp+14h] [ebp-18h] BYREF
    float point[3]; // [esp+20h] [ebp-Ch] BYREF

    client = ent->client;
    if ( client->ps.waterlevel < 4 )
    {
        client->outWaterTime = level.time;
    }
    else if ( level.time - client->outWaterTime > 1000 * (int)player_swimTime->current.value )
    {
        waterDamage = player_swimDamage->current.integer;
        client->outWaterTime += 1000 * (int)player_swimDamagerInterval->current.value;
        dir[0] = 0.0f;
        dir[1] = 0.0f;
        dir[2] = -1.0f;
        point[0] = ent->r.currentOrigin[0];
        point[1] = ent->r.currentOrigin[1];
        point[2] = ent->r.currentOrigin[2];
        G_Damage(ent, 0, 0, dir, point, waterDamage, 6, 19, 0, HITLOC_NONE, 0, 0, 0);
    }
}

void __cdecl G_PlayerStateToEntityStateExtrapolate(playerState_s *ps, entityState_s *s, int time, int snap)
{
    s->lerp.pos.trDelta[0] = ps->velocity[0];
    s->lerp.pos.trDelta[1] = ps->velocity[1];
    s->lerp.pos.trDelta[2] = ps->velocity[2];
    if ( ((LODWORD(s->lerp.pos.trDelta[0]) & 0x7F800000) == 0x7F800000
         || (LODWORD(s->lerp.pos.trDelta[1]) & 0x7F800000) == 0x7F800000
         || (LODWORD(s->lerp.pos.trDelta[2]) & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp",
                    862,
                    0,
                    "%s",
                    "!IS_NAN((s->lerp.pos.trDelta)[0]) && !IS_NAN((s->lerp.pos.trDelta)[1]) && !IS_NAN((s->lerp.pos.trDelta)[2])") )
    {
        __debugbreak();
    }
    s->lerp.pos.trTime = time;
    s->lerp.pos.trDuration = 50;
    BG_PlayerStateToEntityState(ps, s, snap, 1u);
    s->lerp.pos.trType = 4;
}

void __cdecl G_AddPlayerMantleBlockage(float *endPos, int duration, pmove_t *pm)
{
    gentity_s *owner; // [esp+10h] [ebp-Ch]
    gentity_s *ent; // [esp+18h] [ebp-4h]
    int savedregs; // [esp+1Ch] [ebp+0h] BYREF

    owner = &g_entities[pm->ps->clientNum];
    ent = G_Spawn();
    ent->parent.setEnt(owner);
    ent->r.ownerNum.setEnt(owner);
    ent->r.contents = 0x10000;
    ent->clipmask = 0x10000;
    ent->r.svFlags = 33;
    ent->s.eType = ET_INVISIBLE;
    ent->handler = 24;
    ent->r.mins[0] = owner->r.mins[0];
    ent->r.mins[1] = owner->r.mins[1];
    ent->r.mins[2] = owner->r.mins[2];
    ent->r.maxs[0] = owner->r.maxs[0];
    ent->r.maxs[1] = owner->r.maxs[1];
    ent->r.maxs[2] = owner->r.maxs[2];
    G_SetOrigin(ent, endPos);
    SV_LinkEntity(ent);
    ent->nextthink = g_mantleBlockTimeBuffer->current.integer + duration + level.time;
}

void __cdecl G_DrawServerAiming(const gclient_s *client)
{
    float origin[3]; // [esp+18h] [ebp-30h] BYREF
    float end[3]; // [esp+24h] [ebp-24h] BYREF
    float forward[3]; // [esp+30h] [ebp-18h] BYREF
    float viewang[3]; // [esp+3Ch] [ebp-Ch] BYREF

    viewang[0] = client->ps.viewangles[0];
    viewang[1] = client->ps.viewangles[1];
    viewang[2] = client->ps.viewangles[2];
    viewang[0] = client->fGunPitch;
    viewang[1] = client->fGunYaw;
    AngleVectors(viewang, forward, 0, 0);
    G_GetPlayerViewOrigin(&client->ps, origin);
    end[0] = (float)(1000.0 * forward[0]) + origin[0];
    end[1] = (float)(1000.0 * forward[1]) + origin[1];
    end[2] = (float)(1000.0 * forward[2]) + origin[2];
    CG_DebugSphere(end, 1.0, colorRed, 6, 0, 2);
}

void __cdecl G_BuildWeaponState(gclient_s *client, const viewState_t *vs, weaponState_t *ws)
{
    ws->frametime = vs->frametime;
    ws->ps = vs->ps;
    ws->xyspeed = vs->xyspeed;
    ws->time = vs->time;
    ws->damageTime = client->damageTime;
    ws->v_dmg_pitch = client->v_dmg_pitch;
    ws->v_dmg_roll = client->v_dmg_roll;
    ws->weapIdleTime = &client->weapIdleTime;
    ws->shellShockStart = client->ps.shellshockTime;
    ws->shellShockDuration = client->ps.shellshockDuration;
    ws->shellShockFadeTime = 3000;
    ws->fLastIdleFactor = client->fLastIdleFactor;
    ws->baseAngles[0] = client->baseAngles[0];
    ws->baseAngles[1] = client->baseAngles[1];
    ws->baseAngles[2] = client->baseAngles[2];
    ws->baseOrigin[0] = client->baseOrigin[0];
    ws->baseOrigin[1] = client->baseOrigin[1];
    ws->baseOrigin[2] = client->baseOrigin[2];
    ws->recoilAngles[0] = client->recoilAngles[0];
    ws->recoilAngles[1] = client->recoilAngles[1];
    ws->recoilAngles[2] = client->recoilAngles[2];
    ws->recoilSpeed[0] = client->recoilSpeed[0];
    ws->recoilSpeed[1] = client->recoilSpeed[1];
    ws->recoilSpeed[2] = client->recoilSpeed[2];
    ws->swayAngles[0] = client->swayAngles[0];
    ws->swayAngles[1] = client->swayAngles[1];
    ws->swayAngles[2] = client->swayAngles[2];
    ws->swayOrigin[0] = client->swayOffset[0];
    ws->swayOrigin[1] = client->swayOffset[1];
    ws->swayOrigin[2] = client->swayOffset[2];
    ws->swayViewAngles[0] = client->swayViewAngles[0];
    ws->swayViewAngles[1] = client->swayViewAngles[1];
    ws->swayViewAngles[2] = client->swayViewAngles[2];
}

void __cdecl G_SaveWeaponState(const weaponState_t *ws, gclient_s *client)
{
    client->fLastIdleFactor = ws->fLastIdleFactor;
    client->baseAngles[0] = ws->baseAngles[0];
    client->baseAngles[1] = ws->baseAngles[1];
    client->baseAngles[2] = ws->baseAngles[2];
    client->baseOrigin[0] = ws->baseOrigin[0];
    client->baseOrigin[1] = ws->baseOrigin[1];
    client->baseOrigin[2] = ws->baseOrigin[2];
    client->recoilAngles[0] = ws->recoilAngles[0];
    client->recoilAngles[1] = ws->recoilAngles[1];
    client->recoilAngles[2] = ws->recoilAngles[2];
    client->recoilSpeed[0] = ws->recoilSpeed[0];
    client->recoilSpeed[1] = ws->recoilSpeed[1];
    client->recoilSpeed[2] = ws->recoilSpeed[2];
    client->swayAngles[0] = ws->swayAngles[0];
    client->swayAngles[1] = ws->swayAngles[1];
    client->swayAngles[2] = ws->swayAngles[2];
    client->swayOffset[0] = ws->swayOrigin[0];
    client->swayOffset[1] = ws->swayOrigin[1];
    client->swayOffset[2] = ws->swayOrigin[2];
    client->swayViewAngles[0] = ws->swayViewAngles[0];
    client->swayViewAngles[1] = ws->swayViewAngles[1];
    client->swayViewAngles[2] = ws->swayViewAngles[2];
}

void __cdecl ClientThink(int clientNum)
{
    gentity_s *ent; // [esp+8h] [ebp-4h]

    iassert(Sys_IsServerThread());

    ent = &g_entities[clientNum];
    iassert(ent->client);

    iassert(bgs == 0);
    
    bgs = &level_bgs;
    memcpy(&ent->client->sess.oldcmd, &ent->client->sess.cmd, sizeof(ent->client->sess.oldcmd));
    SV_GetUsercmd(clientNum, &ent->client->sess.cmd);
    ent->client->lastCmdTime = level.time;

    if (!g_synchronousClients->current.enabled)
        ClientThink_real(ent, &ent->client->sess.cmd);

    iassert(bgs == &level_bgs);
    bgs = 0;
}

void __cdecl G_RunClient(gentity_s *ent)
{
    float *origin; // [esp+0h] [ebp-10h]
    int savedregs; // [esp+10h] [ebp+0h] BYREF

    if ( !ent->client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp", 1753, 0, "%s", "ent->client") )
    {
        __debugbreak();
    }
    if ( ent->client->sess.connected == CON_DISCONNECTED
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp",
                    1754,
                    0,
                    "%s",
                    "ent->client->sess.connected != CON_DISCONNECTED") )
    {
        __debugbreak();
    }
    if ( g_synchronousClients->current.enabled )
    {
        ent->client->sess.cmd.serverTime = level.time;
        ClientThink_real(ent, &ent->client->sess.cmd);
    }
    if ( (ent->client->flags & 1) == 0 )
    {
        if ( ent->tagInfo )
        {
            ent->client->ps.pm_type = ent->client->sess.sessionState != SESS_STATE_DEAD ? 1 : 10;
            G_SetFixedLink(ent, 2);
            G_SetOrigin(ent, ent->r.currentOrigin);
            G_SetAngle(ent, ent->r.currentAngles);
            ent->s.lerp.pos.trType = 1;
            ent->s.lerp.pos.trDuration = 0;
            ent->s.lerp.pos.trTime = 0;
            ent->s.lerp.pos.trDelta[0] = 0.0f;
            ent->s.lerp.pos.trDelta[1] = 0.0f;
            ent->s.lerp.pos.trDelta[2] = 0.0f;
            ent->s.lerp.apos.trType = 1;
            ent->s.lerp.apos.trDuration = 0;
            ent->s.lerp.apos.trTime = 0;
            ent->s.lerp.apos.trDelta[0] = 0.0f;
            ent->s.lerp.apos.trDelta[1] = 0.0f;
            ent->s.lerp.apos.trDelta[2] = 0.0f;
            SV_LinkEntity(ent);
            origin = ent->client->ps.origin;
            *origin = ent->r.currentOrigin[0];
            origin[1] = ent->r.currentOrigin[1];
            origin[2] = ent->r.currentOrigin[2];
        }
        else if ( ent->client->ps.pm_type == 1 || ent->client->ps.pm_type == 10 )
        {
            --ent->client->ps.pm_type;
        }
    }
    SV_DrawBotPath();
    SV_DrawBotThreat();
}

void __cdecl IntermissionClientEndFrame(gentity_s *ent)
{
    gclient_s *client; // [esp+0h] [ebp-4h]

    client = ent->client;
    ent->r.svFlags &= ~2u;
    ent->r.svFlags |= 1u;
    ent->takedamage = 0;
    ent->r.contents = 0;
    client->ps.otherFlags &= 0xFFFFFFE3;
    client->ps.pm_type = 5;
    client->ps.eFlags &= ~0x200000u;
    client->ps.eFlags &= ~0x40u;
    client->ps.viewmodelIndex = 0;
    ent->s.eType = ET_INVISIBLE;
}

bool __cdecl G_ClientCanSpectateTeam(gclient_s *client, team_t team)
{
    return (client->sess.noSpectate & (1 << team)) == 0;
}

bool __cdecl G_ClientCanSpectateTeamOrLocalPlayer(gclient_s *client, clientState_s *cs)
{
    return G_ClientCanSpectateTeam(client, cs->team);
}

void __cdecl SpectatorClientEndFrame(gentity_s *ent)
{
    unsigned int v2; // ecx
    int number; // [esp+0h] [ebp-2828h]
    gclient_s *client; // [esp+4h] [ebp-2824h]
    playerState_s ps; // [esp+8h] [ebp-2820h] BYREF
    int health; // [esp+2734h] [ebp-F4h] BYREF
    int otherFlags; // [esp+2738h] [ebp-F0h] BYREF
    int EarliestArchivedClientInfoTime; // [esp+273Ch] [ebp-ECh]
    clientState_s v9; // [esp+2740h] [ebp-E8h] BYREF
    int v10; // [esp+2814h] [ebp-14h]
    unsigned int v11; // [esp+2818h] [ebp-10h]
    int clientNum; // [esp+281Ch] [ebp-Ch]
    //playerState_s *ps; // [esp+2820h] [ebp-8h]
    int pArchiveTime; // [esp+2824h] [ebp-4h] BYREF
    int savedregs; // [esp+2828h] [ebp+0h] BYREF

    // LWSS: this is likely some 128bit aligned alloca
    //ps = (playerState_s *)((char *)&playerstate + ((0x80 - (((unsigned __int8)&savedregs - 32) & 0x7F)) & 0x7F));
    //if (((unsigned __int8)ps & 0x7F) != 0
    //    && !Assert_MyHandler(
    //        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp",
    //        1896,
    //        0,
    //        "%s",
    //        "( (unsigned int)ps & (128-1) ) == 0"))
    //{
    //    __debugbreak();
    //}

    v10 = 1000;
    client = ent->client;
    ent->r.svFlags &= ~2u;
    ent->r.svFlags |= 1u;
    ent->takedamage = 0;
    ent->r.contents = 0;
    client->ps.otherFlags &= ~4u;
    ent->s.eType = ET_INVISIBLE;
    client->ps.viewmodelIndex = 0;
    client->fGunPitch = 0.0f;
    client->fGunYaw = 0.0f;
    if (client->sess.forceSpectatorClient < 0)
    {
    LABEL_26:
        if (client->spectatorClient < 0 && !G_ClientCanSpectateTeam(client, TEAM_NUM_TEAMS))
            Cmd_FollowCycle_f(ent, 1);
        clientNum = client->spectatorClient;
        if (clientNum < 0)
            goto LABEL_45;
        pArchiveTime = client->sess.psOffsetTime + client->sess.archiveTime;
        if (!SV_GetArchivedClientInfo(clientNum, &pArchiveTime, &ps, &v9, 0, &health, &otherFlags))
            goto LABEL_45;
        if ((otherFlags & 4) == 0
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp",
                1997,
                0,
                "%s",
                "otherFlags & POF_PLAYER"))
        {
            __debugbreak();
        }
        if (!G_ClientCanSpectateTeamOrLocalPlayer(client, &v9))
        {
        LABEL_45:
            StopFollowing(ent);
            client->ps.otherFlags &= ~0x10u;
            if (G_ClientCanSpectateTeam(client, TEAM_ALLIES)
                || G_ClientCanSpectateTeam(client, TEAM_AXIS)
                || G_ClientCanSpectateTeam(client, TEAM_FREE))
            {
                client->ps.otherFlags |= 8u;
            }
            else
            {
                client->ps.otherFlags &= ~8u;
            }
            return;
        }
    }
    else
    {
        clientNum = client->sess.forceSpectatorClient;
        client->spectatorClient = clientNum;
        EarliestArchivedClientInfoTime = SV_GetEarliestArchivedClientInfoTime();
        while (1)
        {
            if (client->sess.archiveTime < 0)
                client->sess.archiveTime = 0;
            pArchiveTime = client->sess.archiveTime - client->sess.psOffsetTime;
            if (pArchiveTime > EarliestArchivedClientInfoTime - 1000)
            {
                client->sess.archiveTime = EarliestArchivedClientInfoTime + client->sess.psOffsetTime - 1000;
                pArchiveTime = client->sess.archiveTime - client->sess.psOffsetTime;
            }
            if (SV_GetArchivedClientInfo(clientNum, &pArchiveTime, &ps, &v9, 0, &health, &otherFlags))
            {
                if (client->sess.killCamEntity == -1)
                    ps.killCamEntity = 1023;
                else
                    ps.killCamEntity = client->sess.killCamEntity;
                ps.killCamTargetEntity = client->sess.killCamTargetEntity;
                if ((otherFlags & 4) == 0
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp",
                        1952,
                        0,
                        "%s\n\t(otherFlags) = %i",
                        "(otherFlags & (1<<2))",
                        otherFlags))
                {
                    __debugbreak();
                }
                if (G_ClientCanSpectateTeam(client, v9.team))
                    break;
            }
            if (client->sess.archiveTime <= 0)
            {
                client->sess.forceSpectatorClient = -1;
                client->sess.killCamEntity = -1;
                client->sess.killCamTargetEntity = ent->s.clientNum;
                client->spectatorClient = -1;
                goto LABEL_26;
            }
            if (client->sess.killCamEntity < 0)
                goto LABEL_25;
            number = ent->s.number;
            if (client->sess.killCamTargetEntity < 0x20u)
                number = client->sess.killCamTargetEntity;
            if (clientNum == number)
            {
            LABEL_25:
                client->sess.archiveTime -= 50;
            }
            else
            {
                clientNum = number;
                client->spectatorClient = number;
            }
        }
    }
    v11 = client->ps.eFlags & 0x100000 | ps.eFlags & 0xFFEFFFFF;
    Com_Memcpy(&client->ps, &ps, sizeof(playerState_s));
    HudElem_UpdateClient(client, ent->s.number, HUDELEM_UPDATE_CURRENT);
    if (client->sess.archiveTime && client->sess.killCamEntity >= 0)
        HudElem_ClearClient(client, HUDELEM_UPDATE_ARCHIVAL);
    client->ps.eFlags = v11;
    client->ps.otherFlags &= ~4u;
    client->ps.otherFlags |= 2u;
    if (client->sess.forceSpectatorClient >= 0 || RETURN_ZERO32())
    {
        client->ps.otherFlags &= 0xFFFFFFE7;
    }
    else
    {
        client->ps.otherFlags |= 8u;
        if (G_ClientCanSpectateTeam(client, TEAM_NUM_TEAMS))
            v2 = client->ps.otherFlags | 0x10;
        else
            v2 = client->ps.otherFlags & 0xFFFFFFEF;
        client->ps.otherFlags = v2;
    }
}

int __cdecl GetFollowPlayerState(unsigned int clientNum, playerState_s *ps)
{
    gclient_s *client; // [esp+8h] [ebp-8h]
    unsigned int index; // [esp+Ch] [ebp-4h]

    if ( clientNum >= level.maxclients
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp",
                    2072,
                    0,
                    "clientNum doesn't index level.maxclients\n\t%i not in [0, %i)",
                    clientNum,
                    level.maxclients) )
    {
        __debugbreak();
    }
    client = &level.clients[clientNum];
    if ( !client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp", 2075, 0, "%s", "client") )
    {
        __debugbreak();
    }
    if ( client != g_entities[clientNum].client
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp",
                    2076,
                    0,
                    "%s",
                    "client == g_entities[clientNum].client") )
    {
        __debugbreak();
    }
    if ( (client->ps.otherFlags & 4) != 0 )
    {
        memcpy((unsigned __int8 *)ps, (unsigned __int8 *)client, sizeof(playerState_s));
        for ( index = 0; index < 0x1F && ps->hud.current[index].type; ++index )
        {
            memset(&ps->hud.current[index], 0, sizeof(ps->hud.current[index]));
            if ( ps->hud.current[index].type )
            {
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp",
                                2091,
                                0,
                                "%s",
                                "ps->hud.current[index].type == HE_TYPE_FREE") )
                    __debugbreak();
            }
        }
        while ( index < 0x1F )
        {
            if ( memcmp(&ps->hud.current[index], &g_dummyHudCurrent, 0x70u)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp",
                            2098,
                            0,
                            "%s",
                            "!memcmp( &ps->hud.current[index], &g_dummyHudCurrent, sizeof( g_dummyHudCurrent ) )") )
            {
                __debugbreak();
            }
            ++index;
        }
        return 1;
    }
    else
    {
        memset(ps, 0, sizeof(playerState_s));
        return 0;
    }
}

int __cdecl StuckInClient(gentity_s *self)
{
    float v2; // [esp+0h] [ebp-58h]
    float integer; // [esp+4h] [ebp-54h]
    float *v4; // [esp+8h] [ebp-50h]
    float *velocity; // [esp+10h] [ebp-48h]
    float fDist; // [esp+38h] [ebp-20h]
    gentity_s *hit; // [esp+3Ch] [ebp-1Ch]
    int i; // [esp+40h] [ebp-18h]
    float selfSpeed; // [esp+44h] [ebp-14h]
    float hitSpeed; // [esp+48h] [ebp-10h]
    float vDelta[2]; // [esp+4Ch] [ebp-Ch] BYREF
    int iPushTime; // [esp+54h] [ebp-4h]

    iPushTime = 300;
    if ( (self->client->ps.otherFlags & 4) == 0 )
        return 0;
    if ( self->client->sess.sessionState )
        return 0;
    if (self->r.contents != 0x2000000 && self->r.contents != 0x4000000)
        return 0;
    hit = g_entities;
    for ( i = 0; ; ++i )
    {
        if ( i >= level.maxclients )
            return 0;
        if ( hit->r.inuse
            && (hit->client->ps.otherFlags & 4) != 0
            && hit->client->sess.sessionState == SESS_STATE_PLAYING
            && hit != self
            && hit->client
            && hit->health > 0
            && (hit->r.contents == 0x2000000 || hit->r.contents == 0x4000000)
            && hit->r.absmin[0] <= self->r.absmax[0]
            && self->r.absmin[0] <= hit->r.absmax[0]
            && hit->r.absmin[1] <= self->r.absmax[1]
            && self->r.absmin[1] <= hit->r.absmax[1]
            && hit->r.absmin[2] <= self->r.absmax[2]
            && self->r.absmin[2] <= hit->r.absmax[2] )
        {
            vDelta[0] = hit->r.currentOrigin[0] - self->r.currentOrigin[0];
            vDelta[1] = hit->r.currentOrigin[1] - self->r.currentOrigin[1];
            fDist = self->r.maxs[0] + hit->r.maxs[0];
            if ( (float)((float)(vDelta[0] * vDelta[0]) + (float)(vDelta[1] * vDelta[1])) <= (float)(fDist * fDist) )
                break;
        }
        ++hit;
    }
    vDelta[0] = hit->r.currentOrigin[0] - self->r.currentOrigin[0];
    vDelta[1] = hit->r.currentOrigin[1] - self->r.currentOrigin[1];
    vDelta[0] = G_crandom() + vDelta[0];
    vDelta[1] = G_crandom() + vDelta[1];
    Vec2Normalize(vDelta);
    if ( Vec2Length(hit->client->ps.velocity) <= 0.0 )
        integer = 0.0f;
    else
        integer = (float)g_playerCollisionEjectSpeed->current.integer;
    hitSpeed = integer;
    if ( Vec2Length(self->client->ps.velocity) <= 0.0 )
        v2 = 0.0f;
    else
        v2 = (float)g_playerCollisionEjectSpeed->current.integer;
    selfSpeed = v2;
    if ( integer < 0.000099999997 && v2 < 0.000099999997 )
    {
        hitSpeed = (float)hit->client->ps.speed;
        selfSpeed = (float)self->client->ps.speed;
    }
    velocity = hit->client->ps.velocity;
    *velocity = hitSpeed * vDelta[0];
    velocity[1] = hitSpeed * vDelta[1];
    hit->client->ps.pm_time = 300;
    hit->client->ps.pm_flags |= 0x80u;
    v4 = self->client->ps.velocity;
    v4[0] = (-(selfSpeed)) * vDelta[0];
    v4[1] = (-(selfSpeed)) * vDelta[1];
    self->client->ps.pm_time = 300;
    self->client->ps.pm_flags |= 0x80u;
    return 1;
}

const float aiMins[3] = { -25.0, -25.0, 0.0 };
const float aiMaxs[3] = { 25.0, 25.0, 70.0 };

void __cdecl CM_CheckForTraps(gentity_s *ent)
{
    float dir[3]; // [esp+20h] [ebp-10D0h] BYREF
    PhysGeomList *geoms; // [esp+2Ch] [ebp-10C4h]
    float origin[3]; // [esp+30h] [ebp-10C0h] BYREF
    DObj *obj; // [esp+3Ch] [ebp-10B4h]
    unsigned int model; // [esp+40h] [ebp-10B0h]
    float tvel[3]; // [esp+44h] [ebp-10ACh] BYREF
    //float v7; // [esp+48h] [ebp-10A8h]
    //float v8; // [esp+4Ch] [ebp-10A4h]
    float avel[3]; // [esp+50h] [ebp-10A0h] BYREF
    gentity_s *enta; // [esp+5Ch] [ebp-1094h]
    int i; // [esp+60h] [ebp-1090h]
    float v12; // [esp+64h] [ebp-108Ch]
    float v13; // [esp+68h] [ebp-1088h]
    float v14; // [esp+6Ch] [ebp-1084h]
    float mins[6]; // [esp+70h] [ebp-1080h] BYREF
    float start; // [esp+88h] [ebp-1068h] BYREF
    float v17; // [esp+8Ch] [ebp-1064h]
    float v18; // [esp+90h] [ebp-1060h]
    float end[3]; // [esp+94h] [ebp-105Ch] BYREF
    int entityList[1024]; // [esp+A0h] [ebp-1050h] BYREF
    trace_t results; // [esp+10A0h] [ebp-50h] BYREF
    float maxs[2]; // [esp+10DCh] [ebp-14h] BYREF
    float v23; // [esp+10E4h] [ebp-Ch]
    int contentmask; // [esp+10E8h] [ebp-8h]
    int v25; // [esp+10ECh] [ebp-4h]

    contentmask = 1;
    v12 = 1.0f;
    v13 = 1.0f;
    v14 = 1.0f;
    mins[3] = 55.0f;
    mins[4] = 55.0f;
    mins[5] = 0.0f;
    mins[0] = ent->r.currentOrigin[0] - 55.0;
    mins[1] = ent->r.currentOrigin[1] - 55.0;
    mins[2] = ent->r.currentOrigin[2] - 0.0;
    maxs[0] = ent->r.currentOrigin[0] + 55.0;
    maxs[1] = ent->r.currentOrigin[1] + 55.0;
    v23 = ent->r.currentOrigin[2] + 0.0;
    v23 = v23 + 70.0;
    start = ent->r.currentOrigin[0];
    v17 = ent->r.currentOrigin[1];
    v18 = ent->r.currentOrigin[2];
    end[0] = ent->r.currentOrigin[0] + 1.0;
    end[1] = ent->r.currentOrigin[1] + 1.0;
    end[2] = ent->r.currentOrigin[2] + 1.0;
    v25 = CM_AreaEntities(mins, maxs, entityList, 1024, 1);
    for ( i = 0; i < v25; ++i )
    {
        enta = &g_entities[entityList[i]];
        if ( enta->physObjId )
        {
            Phys_ObjGetVelocities(enta->physObjId, tvel, avel);
            if ( (float)((float)((float)(tvel[0] * tvel[0]) + (float)(tvel[1]* tvel[1])) + (float)(tvel[2] * tvel[2])) > (float)(thresh_0 * thresh_0))
            {
                obj = Com_GetServerDObj(enta->s.number);
                if ( obj )
                {
                    if ( !DObjHasCollmap(obj) )
                        continue;
                    geoms = DObjGetCollmap(obj);
                    model = CM_TempBrushModel(geoms);
                }
                else
                {
                    model = SV_ClipHandleForEntity(enta);
                }
                origin[0] = (float)(scale_1 * tvel[0]) + enta->r.currentOrigin[0];
                origin[1] = (float)(scale_1 * tvel[1]) + enta->r.currentOrigin[1];
                origin[2] = (float)(scale_1 * tvel[2]) + enta->r.currentOrigin[2];
                CM_TransformedBoxTraceExternal(&results, &start, end, aiMins, aiMaxs, model, -1, origin, enta->r.currentAngles);
                if ( results.startsolid )
                {
                    dir[0] = start - enta->r.currentOrigin[0];
                    dir[1] = v17 - enta->r.currentOrigin[1];
                    dir[2] = v18 - enta->r.currentOrigin[2];
                    G_Damage(ent, enta, enta, dir, enta->r.currentOrigin, 500, 0, 18, 0xFFFFFFFF, HITLOC_NONE, 0, 0, 0);
                    if ( !ent->client )
                    {
                        ent->s.lerp.pos.trType = 12;
                        ent->s.lerp.apos.trType = 12;
                    }
                }
            }
        }
    }
}

void __cdecl G_PlayerController(const gentity_s *self, int *partBits)
{
    const DObj *obj; // [esp+0h] [ebp-28h]
    clientInfo_t *ci; // [esp+4h] [ebp-24h]
    int i; // [esp+8h] [ebp-20h]
    CEntPlayerInfo player; // [esp+Ch] [ebp-1Ch] BYREF

    SV_CheckThread();
    if ( self->s.clientNum >= 0x20u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp",
                    2289,
                    0,
                    "self->s.clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                    self->s.clientNum,
                    32) )
    {
        __debugbreak();
    }
    ci = &level_bgs.clientinfo[self->s.clientNum];
    if ( !ci->infoValid
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp", 2291, 0, "%s", "ci->infoValid") )
    {
        __debugbreak();
    }

    iassert(bgs == &level_bgs);
    
    BG_Player_DoControllersSetup(&self->s, ci, level.frametime);
    obj = Com_GetServerDObj(self->s.number);
    for ( i = 0; i < 6; ++i )
    {
        player.tag[i] = -2;
        DObjGetBoneIndex(obj, *controller_names[i], &player.tag[i], -1);
    }
    player.control = &ci->control;
    BG_Player_DoControllers(&player, obj, partBits);
}

int __cdecl G_UpdateClientInfo(gentity_s *ent)
{
    unsigned int v1; // eax
    unsigned int v2; // eax
    char *tagName; // [esp+3Ch] [ebp-1Ch]
    int bChanged; // [esp+40h] [ebp-18h]
    gclient_s *client; // [esp+44h] [ebp-14h]
    char *modelName; // [esp+48h] [ebp-10h]
    char *modelNamea; // [esp+48h] [ebp-10h]
    clientInfo_t *ci; // [esp+4Ch] [ebp-Ch]
    int i; // [esp+50h] [ebp-8h]
    unsigned int clientNum; // [esp+54h] [ebp-4h]

    client = ent->client;
    if ( !client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp", 2322, 0, "%s", "client") )
    {
        __debugbreak();
    }
    if ( client->sess.connected == CON_DISCONNECTED
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp",
                    2323,
                    0,
                    "%s",
                    "client->sess.connected != CON_DISCONNECTED") )
    {
        __debugbreak();
    }
    clientNum = ent->s.clientNum;
    if ( clientNum >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp",
                    2326,
                    0,
                    "clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                    clientNum,
                    32) )
    {
        __debugbreak();
    }
    ci = &level_bgs.clientinfo[clientNum];
    if ( !ci->infoValid
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp", 2328, 0, "%s", "ci->infoValid") )
    {
        __debugbreak();
    }
    bChanged = 0;
    v1 = G_ModelName(ent->model);
    modelName = SL_ConvertToString(v1, SCRIPTINSTANCE_SERVER);
    client->sess.cs.modelindex = ent->model;
    if ( strcmp(ci->model, modelName) )
    {
        bChanged = 1;
        I_strncpyz(ci->model, modelName, 64);
    }
    for ( i = 0; i < 6; ++i )
    {
        if ( ent->attachModelNames[i] )
        {
            v2 = G_ModelName(ent->attachModelNames[i]);
            modelNamea = SL_ConvertToString(v2, SCRIPTINSTANCE_SERVER);
            client->sess.cs.attachModelIndex[i] = ent->attachModelNames[i];
            if ( strcmp(ci->attachModelNames[i], modelNamea) )
            {
                bChanged = 1;
                I_strncpyz(ci->attachModelNames[i], modelNamea, 64);
            }
            if ( !ent->attachTagNames[i]
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp",
                            2360,
                            0,
                            "%s",
                            "ent->attachTagNames[i]") )
            {
                __debugbreak();
            }
            tagName = SL_ConvertToString(ent->attachTagNames[i], SCRIPTINSTANCE_SERVER);
            client->sess.cs.attachTagIndex[i] = G_TagIndex(tagName);
            if ( strcmp(ci->attachTagNames[i], tagName) )
            {
                bChanged = 1;
                I_strncpyz(ci->attachTagNames[i], tagName, 64);
            }
        }
        else
        {
            ci->attachModelNames[i][0] = 0;
            ci->attachTagNames[i][0] = 0;
            client->sess.cs.attachModelIndex[i] = 0;
            client->sess.cs.attachTagIndex[i] = 0;
        }
    }
    if ( (client->ps.eFlags & 0x4000) == 0 || client->ps.viewlocked_entNum == 1023 )
    {
        client->sess.cs.attachedVehEntNum = 1023;
        client->sess.cs.attachedVehSeat = 0;
    }
    else if ( ent->r.ownerNum.isDefined() )
    {
        iassert(ent->r.ownerNum.isDefined());
        iassert(ent->r.ownerNum.entnum() == (unsigned)client->ps.viewlocked_entNum);

        client->sess.cs.attachedVehEntNum = ent->r.ownerNum.entnum();
        client->sess.cs.attachedVehSeat = client->ps.vehiclePos;
    }
    else
    {
        if ( (client->ps.otherFlags & 2) == 0 )
        {
            Com_PrintWarning(16, "G_UpdateClientInfo(): Veh attached, but no ownerNum\n");
            client->ps.eFlags &= ~0x4000u;
        }
        client->sess.cs.attachedVehEntNum = 1023;
        client->sess.cs.attachedVehSeat = 0;
    }
    client->sess.cs.score.ping = SV_GetClientPing(clientNum) / g_ScoresPing_Interval->current.integer;
    return bChanged;
}

void __cdecl ClientEndFrame(gentity_s *ent)
{
    int v1; // eax
    int v2; // eax
    unsigned int v3; // ecx
    const char *v4; // eax
    char *v5; // eax
    int v6; // [esp+20h] [ebp-C0h]
    bool v7; // [esp+24h] [ebp-BCh]
    bitarray<51> *v8; // [esp+28h] [ebp-B8h]
    int k; // [esp+2Ch] [ebp-B4h]
    float *playerAngles; // [esp+30h] [ebp-B0h]
    float *viewangles; // [esp+34h] [ebp-ACh]
    bitarray<51> *v12; // [esp+38h] [ebp-A8h]
    int m; // [esp+3Ch] [ebp-A4h]
    float *origin; // [esp+4Ch] [ebp-94h]
    bitarray<51> *v15; // [esp+54h] [ebp-8Ch]
    int n; // [esp+58h] [ebp-88h]
    float v17; // [esp+5Ch] [ebp-84h]
    bitarray<51> *v18; // [esp+64h] [ebp-7Ch]
    int j; // [esp+68h] [ebp-78h]
    bitarray<51> *p_button_bitsSinceLastFrame; // [esp+6Ch] [ebp-74h]
    int i; // [esp+70h] [ebp-70h]
    bitarray<51> *v22; // [esp+74h] [ebp-6Ch]
    int ii; // [esp+78h] [ebp-68h]
    int partBits[5]; // [esp+7Ch] [ebp-64h] BYREF
    gentity_s *vehicle; // [esp+90h] [ebp-50h]
    float angles[3]; // [esp+94h] [ebp-4Ch]
    const vehicle_info_t *info; // [esp+A0h] [ebp-40h]
    gentity_s *veh; // [esp+A4h] [ebp-3Ch]
    int bChanged; // [esp+A8h] [ebp-38h]
    gclient_s *client; // [esp+ACh] [ebp-34h]
    DObj *obj; // [esp+B0h] [ebp-30h]
    float vViewPos[3]; // [esp+B4h] [ebp-2Ch] BYREF
    clientInfo_t *ci; // [esp+C0h] [ebp-20h]
    int clientNum; // [esp+C4h] [ebp-1Ch]
    float spawn_angles[3]; // [esp+C8h] [ebp-18h] BYREF
    float spawn_origin[3]; // [esp+D4h] [ebp-Ch] BYREF

    client = ent->client;
    if ( !client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp", 2442, 0, "%s", "client") )
    {
        __debugbreak();
    }
    if ( client->sess.connected == CON_DISCONNECTED
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp",
                    2443,
                    0,
                    "%s",
                    "client->sess.connected != CON_DISCONNECTED") )
    {
        __debugbreak();
    }
    ent->handler = 14;
    client->ps.deltaTime = 0;
    client->ps.gravity = (int)bg_gravity->current.value;
    client->ps.moveSpeedScaleMultiplier = ent->client->sess.moveSpeedScaleMultiplier;
    if ( (client->flags & 4) != 0 )
        v1 = client->ps.pm_flags | 0x800;
    else
        v1 = client->ps.pm_flags & 0xFFFFF7FF;
    client->ps.pm_flags = v1;
    bChanged = G_UpdateClientInfo(ent);
    if ( client->sess.connected == CON_CONNECTED )
    {
        if ( Demo_IsRecording() )
            Demo_RecordPlayerSessionState(ent->s.number, client->sess.sessionState);
        if ( client->sess.sessionState == SESS_STATE_INTERMISSION )
        {
            IntermissionClientEndFrame(ent);
            p_button_bitsSinceLastFrame = &ent->client->button_bitsSinceLastFrame;
            for ( i = 0; i < 2; ++i )
                p_button_bitsSinceLastFrame->array[i] = 0;
        }
        else if ( client->sess.sessionState == SESS_STATE_SPECTATOR )
        {
            SpectatorClientEndFrame(ent);
            v18 = &ent->client->button_bitsSinceLastFrame;
            for ( j = 0; j < 2; ++j )
                v18->array[j] = 0;
        }
        else if ( client->ps.clientNum == ent->s.number )
        {
            ent->r.svFlags |= 2u;
            ent->r.svFlags &= ~1u;
            ent->takedamage = 1;
            client->ps.otherFlags |= 4u;
            client->ps.otherFlags &= 0xFFFFFFE7;
            client->ps.viewmodelIndex = client->sess.viewmodelIndex;
            client->ps.weaponIdleTime = client->weapIdleTime;
            G_SetClientContents(ent);
            client->dropWeaponTime = 0;
            if ( client->compassPingTime <= level.time )
                client->ps.eFlags &= ~0x400000u;
            if ( (client->flags & 1) != 0 )
            {
                client->ps.pm_type = 2;
            }
            else if ( (client->flags & 2) != 0 )
            {
                client->ps.pm_type = 3;
            }
            else if ( client->sess.sessionState == SESS_STATE_DEAD )
            {
                client->ps.pm_type = (ent->tagInfo != 0) + 9;
                ent->r.svFlags &= ~2u;
                if ( (client->ps.eFlags & 0x4000) == 0 || client->ps.vehicleType != 6 )
                    ent->r.svFlags |= 1u;
                ent->takedamage = 0;
            }
            else if ( client->revive )
            {
                client->ps.pm_type = 6;
            }
            else if ( client->lastStand )
            {
                client->ps.pm_type = 6;
            }
            else if ( client->ps.pm_type != 8 )
            {
                v7 = ent->tagInfo && (client->ps.eFlags & 0x4000) == 0;
                client->ps.pm_type = v7;
            }
            client->currentAimSpreadScale = client->ps.aimSpreadScale / 255.0;
            if ( (ent->client->flags & 3) == 0 )
            {
                if ( ent->tagInfo )
                {
                    client->ps.pm_type = ent->client->ps.stats[0] > 0 ? 1 : 10;
                    G_SetPlayerFixedLink(ent);
                    origin = ent->client->ps.origin;
                    *origin = ent->r.currentOrigin[0];
                    origin[1] = ent->r.currentOrigin[1];
                    origin[2] = ent->r.currentOrigin[2];
                }
                else
                {
                    ent->client->prevLinkAnglesSet = 0;
                }
            }
            Player_UpdateCursorHints(ent);
            P_DamageFeedback(ent);
            if ( level.time - client->lastCmdTime <= 1000 )
                v2 = ent->s.lerp.eFlags & 0xFFFFFF7F;
            else
                v2 = ent->s.lerp.eFlags | 0x80;
            ent->s.lerp.eFlags = v2;
            client->ps.stats[0] = ent->health;
            client->ps.satelliteTypeEnabled = level.teamHasSatellite[client->sess.cs.team];
            if ( client->hasSatellite )
                client->ps.satelliteTypeEnabled = client->hasSatellite;
            client->ps.spyplaneTypeEnabled = level.teamHasSpyplane[client->sess.cs.team];
            if ( client->hasSpyplane )
                client->ps.spyplaneTypeEnabled = client->hasSpyplane;
            ent->client->ps.loopSoundId = ent->s.loopSoundId;
            ent->client->ps.loopSoundFade = ent->s.loopSoundFade;
            if ( g_smoothClients->current.enabled )
                G_PlayerStateToEntityStateExtrapolate(&client->ps, &ent->s, client->ps.commandTime, 1);
            else
                BG_PlayerStateToEntityState(&client->ps, &ent->s, 1, 1u);
            if ( (ent->s.lerp.eFlags & 0x4000) != 0 && (ent->client->ps.eFlags2 & 0x10000000) != 0 )
            {
                veh = &g_entities[ent->client->ps.viewlocked_entNum];
                if ( !veh
                    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp", 2638, 0, "%s", "veh") )
                {
                    __debugbreak();
                }
                info = BG_GetVehicleInfo(veh->s.vehicleState.vehicleInfoIndex);
                if ( !info
                    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp", 2640, 0, "%s", "info") )
                {
                    __debugbreak();
                }
                if ( info->turretClampPlayerView
                    && (!gVehicleRelativeGunnerAngles || ent->client->ps.vehiclePos < 1 || ent->client->ps.vehiclePos > 4) )
                {
                    angles[0] = veh->r.currentAngles[0];
                    angles[1] = veh->r.currentAngles[1];
                    angles[2] = veh->r.currentAngles[2];
                    angles[0] = angles[0] + info->viewPitchOffset;
                    G_UpdateViewAngleClamp(client, veh->r.currentAngles);
                }
            }
            if ( (ent->client->ps.eFlags2 & 0x10000000) != 0 )
            {
                ent->s.lerp.apos.trBase[0] = ent->r.currentAngles[0];
                ent->s.lerp.apos.trBase[1] = ent->r.currentAngles[1];
                ent->s.lerp.apos.trBase[2] = ent->r.currentAngles[2];
            }
            if ( ent->health > 0 )
                StuckInClient(ent);
            G_GetPlayerViewOrigin(&ent->client->ps, vViewPos);
            client->ps.iCompassPlayerInfo = G_GetNonPVSPlayerInfo(ent, vViewPos, client->iLastCompassPlayerInfoEnt);
            if ( (client->ps.iCompassPlayerInfo & 0x3Fu) >= 0x20
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp",
                            2669,
                            0,
                            "COMPFR_GET_INDEX( client->ps.iCompassPlayerInfo ) doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                            client->ps.iCompassPlayerInfo & 0x3F,
                            32) )
            {
                __debugbreak();
            }
            if ( client->ps.iCompassPlayerInfo )
            {
                client->iLastCompassPlayerInfoEnt = client->ps.iCompassPlayerInfo & 0x3F;
                if ( (g_entities[client->iLastCompassPlayerInfoEnt].s.lerp.eFlags & 0x400000) != 0 )
                    v3 = 0x800000 | client->ps.eFlags;
                else
                    v3 = client->ps.eFlags & 0xFF7FFFFF;
                client->ps.eFlags = v3;
            }
            else
            {
                client->iLastCompassPlayerInfoEnt = 1023;
            }
            if ( ent->s.eType == 1 )
            {
                ent->handler = 2 * (ent->health <= 0) + 13;
                clientNum = ent->s.clientNum;
                if ( (unsigned int)clientNum >= 0x20
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp",
                                2695,
                                0,
                                "clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                                clientNum,
                                32) )
                {
                    __debugbreak();
                }
                ci = &level_bgs.clientinfo[clientNum];
                if ( !ci->infoValid
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp",
                                2697,
                                0,
                                "%s",
                                "ci->infoValid") )
                {
                    __debugbreak();
                }
                ci->lerpMoveDir = (float)ent->s.lerp.u.loopFx.period;
                ci->lerpLean = ent->s.lerp.u.turret.gunAngles[0];
                playerAngles = ci->playerAngles;
                viewangles = client->ps.viewangles;
                ci->playerAngles[0] = client->ps.viewangles[0];
                playerAngles[1] = viewangles[1];
                playerAngles[2] = viewangles[2];
                if ( bChanged )
                    G_SafeDObjFree(ent);
                obj = Com_GetServerDObj(ent->s.number);
                G_AddClientKnife(ci, &client->ps);
                BG_UpdatePlayerDObj(-1, obj, &ent->s, ci, ent->attachIgnoreCollision);
                BG_PlayerAnimation(-1, &ent->s, ci);
                if ( (client->ps.otherFlags & 4) != 0 && (client->ps.eFlags & 0x300) != 0 )
                {
                    if ( client->ps.clientNum != ent->s.number
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp",
                                    2718,
                                    0,
                                    "%s",
                                    "client->ps.clientNum == ent->s.number") )
                    {
                        __debugbreak();
                    }
                    if ( client->ps.viewlocked_entNum == 1023
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp",
                                    2719,
                                    0,
                                    "%s",
                                    "client->ps.viewlocked_entNum != ENTITYNUM_NONE") )
                    {
                        __debugbreak();
                    }
                    if ( !level.gentities[client->ps.viewlocked_entNum].r.ownerNum.isDefined()
                        || level.gentities[client->ps.viewlocked_entNum].r.ownerNum.ent() != ent )
                    {
                        if ( level.gentities[client->ps.viewlocked_entNum].r.ownerNum.isDefined() )
                        {
                            v6 = level.gentities[client->ps.viewlocked_entNum].r.ownerNum.entnum();
                            v4 = va(
                                         "viewlocked_entNum is %i, ownerNum is %i, ent->s.number is %i",
                                         client->ps.viewlocked_entNum,
                                         v6,
                                         ent->s.number);
                        }
                        else
                        {
                            v4 = va(
                                         "viewlocked_entNum is %i, ownerNum is %i, ent->s.number is %i",
                                         client->ps.viewlocked_entNum,
                                         1023,
                                         ent->s.number);
                        }
                        if ( !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp",
                                        2720,
                                        0,
                                        "%s\n\t%s",
                                        "level.gentities[client->ps.viewlocked_entNum].r.ownerNum.isDefined() && (level.gentities[client->ps."
                                        "viewlocked_entNum].r.ownerNum.ent() == ent)",
                                        v4) )
                            __debugbreak();
                    }
                    turret_think_client(&level.gentities[client->ps.viewlocked_entNum]);
                }
                if ( (client->ps.eFlags & 0x4000) != 0 && client->ps.vehiclePos >= 1 && client->ps.vehiclePos <= 4 )
                {
                    iassert(ent->r.ownerNum.isDefined());

                    vehicle = ent->r.ownerNum.ent();
                    G_PlayerVehiclePositionAndBlend(ent, vehicle);
                }
                if ( g_debugLocDamage->current.integer == 1 && SV_DObjExists(ent) )
                {
                    memset(partBits, 255, sizeof(partBits));
                    G_DObjCalcPose(ent, partBits);
                    SV_XModelDebugBoxes(ent, colorWhite, 0, 0);
                    v5 = va(
                                 "[%i] %.3f, %.3f, %.3f\n",
                                 clientNum,
                                 level_bgs.clientinfo[clientNum].legs.yawAngle,
                                 level_bgs.clientinfo[clientNum].torso.yawAngle,
                                 level_bgs.clientinfo[clientNum].playerAngles[1]);
                    CL_AddDebugStarWithText(ent->r.currentOrigin, colorYellow, colorYellow, v5, 0.25, 1);
                }
                v8 = &ent->client->button_bitsSinceLastFrame;
                for ( k = 0; k < 2; ++k )
                    v8->array[k] = 0;
            }
            else
            {
                v12 = &ent->client->button_bitsSinceLastFrame;
                for ( m = 0; m < 2; ++m )
                    v12->array[m] = 0;
            }
        }
        else
        {
            spawn_origin[0] = client->ps.origin[0];
            spawn_origin[1] = client->ps.origin[1];
            spawn_origin[2] = client->ps.origin[2];
            v17 = client->ps.viewangles[1];
            spawn_angles[0] = 0.0f;
            spawn_angles[1] = v17;
            spawn_angles[2] = 0.0f;
            ClientSpawn(ent, spawn_origin, spawn_angles);
            if ( client->ps.clientNum != ent->s.number
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp",
                            2502,
                            0,
                            "%s",
                            "client->ps.clientNum == ent->s.number") )
            {
                __debugbreak();
            }
            v15 = &ent->client->button_bitsSinceLastFrame;
            for ( n = 0; n < 2; ++n )
                v15->array[n] = 0;
        }
    }
    else
    {
        v22 = &ent->client->button_bitsSinceLastFrame;
        for ( ii = 0; ii < 2; ++ii )
            v22->array[ii] = 0;
    }
}

void __cdecl G_AddClientKnife(clientInfo_t *ci, playerState_s *ps)
{
    bool addKnife; // [esp+0h] [ebp-4h]

    addKnife = BG_IsKnifeMeleeAnim(ci, ps->torsoAnim);
    if ( !addKnife || ci->usingKnife )
    {
        if ( !addKnife && ci->usingKnife )
        {
            ci->usingKnife = 0;
            ci->dobjDirty = 1;
        }
    }
    else
    {
        ci->usingKnife = 1;
        ci->dobjDirty = 1;
    }
}

gentity_s *__cdecl G_GetPlayer(unsigned int clientNum)
{
    if ( clientNum >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp",
                    2756,
                    0,
                    "clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                    clientNum,
                    32) )
    {
        __debugbreak();
    }
    return &level.gentities[clientNum];
}

void __cdecl G_PlayerEvent(int clientNum, int event)
{
    gclient_s *client; // [esp+4h] [ebp-10h]
    float kickAVel[3]; // [esp+8h] [ebp-Ch] BYREF

    client = g_entities[clientNum].client;
    if ( !client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_active_mp.cpp", 2773, 0, "%s", "client") )
    {
        __debugbreak();
    }
    if ( event >= 29 && (event <= 32 || event == 47) )
    {
        BG_WeaponFireRecoil(&client->ps, client->recoilSpeed, kickAVel);
        //BLOPS_NULLSUB();
    }
}

