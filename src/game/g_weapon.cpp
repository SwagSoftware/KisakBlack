#include "g_weapon.h"
#include <game_mp/g_main_mp.h>
#include <server/sv_world.h>
#include <glass/glass_server.h>
#include <game_mp/g_utils_mp.h>
#include <game_mp/g_combat_mp.h>
#include <game_mp/g_trigger_mp.h>
#include <server/sv_game.h>
#include <client/cl_debugdata.h>
#include "g_missile.h"
#include <cgame/cg_drawtools.h>
#include <bgame/bg_misc.h>
#include "turret.h"
#include <game_mp/g_team_mp.h>
#include <universal/com_math_anglevectors.h>
#include <client_mp/g_client_mp.h>
#include <clientscript/cscr_vm.h>
#include <game_mp/g_spawn_mp.h>
#include <clientscript/scr_const.h>
#include "g_targets.h"
#include "bullet.h"
#include "g_debug.h"
#include <bgame/bg_weapons_ammo.h>
#include <server_mp/sv_main_mp.h>
#include <bgame/bg_weapons_def.h>
#include <flame/flame_system.h>

const float traceOffsets[9][2] =
{
  { 0.0, 0.0 },
  { 0.5, 0.0 },
  { -0.5, 0.0 },
  { 0.0, 0.5 },
  { 0.0, -0.5 },
  { 1.0, 1.0 },
  { -1.0, 1.0 },
  { 1.0, -1.0 },
  { -1.0, -1.0 }
};

float directionTweak = 0.25;
float bulletLedgeHeightRaiseAdd = 18.0;
float traceNoHitPushDist = 4.0;
float ledgeDepthNudgeReductionEnd = 1.0;
float ledgeDepthNudgeReductionStart = 8.0;
float traceEndPosAdd = 8.0;
int secondmask = 64;
int startmask = 1;
int linemask = 8415283;
float traceOffset = 12.0;
float bipodNudgeDistDefault = 8.0;


void __cdecl Weapon_SetWeaponParamsWeapon(weaponParms *wp, unsigned int weapon)
{
    unsigned int NumWeapons; // eax

    if ( !wp && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon.cpp", 41, 0, "%s", "wp") )
        __debugbreak();
    if ( weapon >= BG_GetNumWeapons() )
    {
        NumWeapons = BG_GetNumWeapons();
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon.cpp",
                        42,
                        0,
                        "weapon doesn't index BG_GetNumWeapons()\n\t%i not in [0, %i)",
                        weapon,
                        NumWeapons) )
            __debugbreak();
    }
    wp->weapVariantDef = BG_GetWeaponVariantDef(weapon);
    wp->weapDef = BG_GetWeaponDef(weapon);
    if ( !wp->weapVariantDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon.cpp", 47, 0, "%s", "wp->weapVariantDef") )
    {
        __debugbreak();
    }
    if ( !wp->weapDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon.cpp", 48, 0, "%s", "wp->weapDef") )
    {
        __debugbreak();
    }
}

void __cdecl G_AntiLagRewindClientPos(int gameTime, AntilagClientStore *antilagStore)
{
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    float *v6; // [esp+18h] [ebp-37Ch]
    float *v7; // [esp+1Ch] [ebp-378h]
    float *v8; // [esp+20h] [ebp-374h]
    float *v9; // [esp+24h] [ebp-370h]
    float *currentAngles; // [esp+44h] [ebp-350h]
    float *v11; // [esp+48h] [ebp-34Ch]
    float *currentOrigin; // [esp+4Ch] [ebp-348h]
    int client; // [esp+68h] [ebp-32Ch]
    float clientPositions[32][3]; // [esp+6Ch] [ebp-328h] BYREF
    float clientAngles[32][3]; // [esp+1ECh] [ebp-1A8h] BYREF
    bool success[32]; // [esp+36Ch] [ebp-28h] BYREF
    int snapshotTime; // [esp+390h] [ebp-4h]
    int savedregs; // [esp+394h] [ebp+0h] BYREF

    if ( !antilagStore
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon.cpp", 66, 0, "%s", "antilagStore") )
    {
        __debugbreak();
    }
    memset((unsigned __int8 *)antilagStore, 0, sizeof(AntilagClientStore));
    if ( gameTime <= 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon.cpp", 69, 0, "%s", "gameTime > 0") )
    {
        __debugbreak();
    }
    if ( level.time - gameTime > 400 )
        gameTime = level.time - 400;
    if ( level.time - gameTime > 50 )
    {
        memset(success, 0, sizeof(success));
        if ( SV_GetClientPositionsAtTime(gameTime, clientPositions, clientAngles, success) )
        {
            for ( client = 0; client < level.maxclients; ++client )
            {
                if ( success[client]
                    && level.clients[client].sess.connected == CON_CONNECTED
                    && level.clients[client].sess.sessionState == SESS_STATE_PLAYING )
                {
                    // kisaktodo: grug. fix these commented zones. they dont matter right now (make sure to get the whole file :))
                    //if ( (LODWORD(clientPositions[client][0]) & 0x7F800000) == 0x7F800000
                    //    || (LODWORD(clientPositions[client][1]) & 0x7F800000) == 0x7F800000
                    //    || (LODWORD(clientPositions[client][2]) & 0x7F800000) == 0x7F800000 )
                    //{
                    //    v2 = va(
                    //                 "client %i's antilag origin is invalid - (%f, %f, %f)",
                    //                 client,
                    //                 clientPositions[client][0],
                    //                 clientPositions[client][1],
                    //                 clientPositions[client][2]);
                    //    if ( !Assert_MyHandler(
                    //                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon.cpp",
                    //                    86,
                    //                    0,
                    //                    "%s\n\t%s",
                    //                    "!IS_NAN(clientPositions[client][0]) && !IS_NAN(clientPositions[client][1]) && !IS_NAN(clientPositions[client][2])",
                    //                    v2) )
                    //        __debugbreak();
                    //}
                    //if ( (LODWORD(clientAngles[client][0]) & 0x7F800000) == 0x7F800000
                    //    || (LODWORD(clientAngles[client][1]) & 0x7F800000) == 0x7F800000
                    //    || (LODWORD(clientAngles[client][2]) & 0x7F800000) == 0x7F800000 )
                    //{
                    //    v3 = va(
                    //                 "client %i's antilag angles are invalid - (%f, %f, %f)",
                    //                 client,
                    //                 clientAngles[client][0],
                    //                 clientAngles[client][1],
                    //                 clientAngles[client][2]);
                    //    if ( !Assert_MyHandler(
                    //                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon.cpp",
                    //                    87,
                    //                    0,
                    //                    "%s\n\t%s",
                    //                    "!IS_NAN(clientAngles[client][0]) && !IS_NAN(clientAngles[client][1]) && !IS_NAN(clientAngles[client][2])",
                    //                    v3) )
                    //        __debugbreak();
                    //}
                    snapshotTime = gameTime;
                    v11 = antilagStore->realClientPositions[client];
                    currentOrigin = g_entities[client].r.currentOrigin;
                    *v11 = *currentOrigin;
                    v11[1] = currentOrigin[1];
                    v11[2] = currentOrigin[2];
                    currentAngles = g_entities[client].r.currentAngles;
                    v11[96] = *currentAngles;
                    v11[97] = currentAngles[1];
                    //v11[98] = currentAngles[2];
                    //if ( (*(unsigned int *)v11 & 0x7F800000) == 0x7F800000
                    //    || (LODWORD(antilagStore->realClientPositions[client][1]) & 0x7F800000) == 0x7F800000
                    //    || (LODWORD(antilagStore->realClientPositions[client][2]) & 0x7F800000) == 0x7F800000 )
                    //{
                    //    v4 = va(
                    //                 "client %i's origin is invalid - (%f, %f, %f)",
                    //                 client,
                    //                 antilagStore->realClientPositions[client][0],
                    //                 antilagStore->realClientPositions[client][1],
                    //                 antilagStore->realClientPositions[client][2]);
                    //    if ( !Assert_MyHandler(
                    //                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon.cpp",
                    //                    95,
                    //                    0,
                    //                    "%s\n\t%s",
                    //                    "!IS_NAN( antilagStore->realClientPositions[client][0]) && !IS_NAN(antilagStore->realClientPositions["
                    //                    "client][1]) && !IS_NAN(antilagStore->realClientPositions[client][2])",
                    //                    v4) )
                    //        __debugbreak();
                    //}
                    //if ( (LODWORD(antilagStore->realClientAngles[client][0]) & 0x7F800000) == 0x7F800000
                    //    || (LODWORD(antilagStore->realClientAngles[client][1]) & 0x7F800000) == 0x7F800000
                    //    || (LODWORD(antilagStore->realClientAngles[client][2]) & 0x7F800000) == 0x7F800000 )
                    //{
                    //    v5 = va(
                    //                 "client %i's angles are invalid - (%f, %f, %f)",
                    //                 client,
                    //                 antilagStore->realClientAngles[client][0],
                    //                 antilagStore->realClientAngles[client][1],
                    //                 antilagStore->realClientAngles[client][2]);
                    //    if ( !Assert_MyHandler(
                    //                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon.cpp",
                    //                    96,
                    //                    0,
                    //                    "%s\n\t%s",
                    //                    "!IS_NAN( antilagStore->realClientAngles[client][0]) && !IS_NAN(antilagStore->realClientAngles[client"
                    //                    "][1]) && !IS_NAN(antilagStore->realClientAngles[client][2])",
                    //                    v5) )
                    //        __debugbreak();
                    //}
                    v8 = g_entities[client].r.currentOrigin;
                    v9 = clientPositions[client];
                    *v8 = *v9;
                    v8[1] = v9[1];
                    v8[2] = v9[2];
                    v6 = g_entities[client].r.currentAngles;
                    v7 = clientAngles[client];
                    *v6 = *v7;
                    v6[1] = v7[1];
                    v6[2] = v7[2];
                    SV_LinkEntity(&g_entities[client]);
                    antilagStore->clientMoved[client] = 1;
                }
                else
                {
                    antilagStore->clientMoved[client] = 0;
                }
            }
        }
    }
}

void __cdecl G_AntiLag_RestoreClientPos(AntilagClientStore *antilagStore)
{
    const char *v1; // eax
    const char *v2; // eax
    float *currentAngles; // [esp+18h] [ebp-2Ch]
    float *currentOrigin; // [esp+20h] [ebp-24h]
    float *v5; // [esp+24h] [ebp-20h]
    int client; // [esp+40h] [ebp-4h]
    int savedregs; // [esp+44h] [ebp+0h] BYREF

    if ( !antilagStore
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon.cpp", 124, 0, "%s", "antilagStore") )
    {
        __debugbreak();
    }
    for ( client = 0; client < level.maxclients; ++client )
    {
        if ( antilagStore->clientMoved[client] )
        {
            //if ( (LODWORD(antilagStore->realClientPositions[client][0]) & 0x7F800000) == 0x7F800000
            //    || (LODWORD(antilagStore->realClientPositions[client][1]) & 0x7F800000) == 0x7F800000
            //    || (LODWORD(antilagStore->realClientPositions[client][2]) & 0x7F800000) == 0x7F800000 )
            //{
            //    v1 = va(
            //                 "client %i's origin is invalid - (%f, %f, %f)",
            //                 client,
            //                 antilagStore->realClientPositions[client][0],
            //                 antilagStore->realClientPositions[client][1],
            //                 antilagStore->realClientPositions[client][2]);
            //    if ( !Assert_MyHandler(
            //                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon.cpp",
            //                    130,
            //                    0,
            //                    "%s\n\t%s",
            //                    "!IS_NAN(antilagStore->realClientPositions[client][0]) && !IS_NAN(antilagStore->realClientPositions[clien"
            //                    "t][1]) && !IS_NAN(antilagStore->realClientPositions[client][2])",
            //                    v1) )
            //        __debugbreak();
            //}
            //if ( (LODWORD(antilagStore->realClientAngles[client][0]) & 0x7F800000) == 0x7F800000
            //    || (LODWORD(antilagStore->realClientAngles[client][1]) & 0x7F800000) == 0x7F800000
            //    || (LODWORD(antilagStore->realClientAngles[client][2]) & 0x7F800000) == 0x7F800000 )
            //{
            //    v2 = va(
            //                 "client %i's angles are invalid - (%f, %f, %f)",
            //                 client,
            //                 antilagStore->realClientAngles[client][0],
            //                 antilagStore->realClientAngles[client][1],
            //                 antilagStore->realClientAngles[client][2]);
            //    if ( !Assert_MyHandler(
            //                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon.cpp",
            //                    131,
            //                    0,
            //                    "%s\n\t%s",
            //                    "!IS_NAN(antilagStore->realClientAngles[client][0]) && !IS_NAN(antilagStore->realClientAngles[client][1])"
            //                    " && !IS_NAN(antilagStore->realClientAngles[client][2])",
            //                    v2) )
            //        __debugbreak();
            //}
            currentOrigin = g_entities[client].r.currentOrigin;
            v5 = antilagStore->realClientPositions[client];
            *currentOrigin = *v5;
            currentOrigin[1] = v5[1];
            currentOrigin[2] = v5[2];
            currentAngles = g_entities[client].r.currentAngles;
            *currentAngles = v5[96];
            currentAngles[1] = v5[97];
            currentAngles[2] = v5[98];
            SV_LinkEntity(&g_entities[client]);
            antilagStore->clientMoved[client] = 0;
        }
    }
}

gentity_s *__cdecl Weapon_Melee(gentity_s *ent, weaponParms *wp, float range, float width, float height, int gametime)
{
    AntilagClientStore antilagClients; // [esp+Ch] [ebp-328h] BYREF
    gentity_s *traceEnt; // [esp+330h] [ebp-4h]

    G_AntiLagRewindClientPos(gametime, &antilagClients);
    traceEnt = Weapon_Melee_internal(ent, wp, range, width, height);
    G_AntiLag_RestoreClientPos(&antilagClients);
    return traceEnt;
}

gentity_s *__cdecl Weapon_Melee_internal(gentity_s *ent, weaponParms *wp, float range, float width, float height)
{
    bool IsBayonetWeapon; // al
    int v7; // eax
    int v8; // [esp-4h] [ebp-88h]
    hitLocation_t partGroup; // [esp+4h] [ebp-80h]
    unsigned int modelIndex; // [esp+8h] [ebp-7Ch]
    unsigned int partName; // [esp+Ch] [ebp-78h]
    unsigned __int16 v12; // [esp+14h] [ebp-70h]
    int damage; // [esp+1Ch] [ebp-68h]
    float hitOrigin[3]; // [esp+20h] [ebp-64h] BYREF
    gentity_s *tent; // [esp+2Ch] [ebp-58h]
    float dir[3]; // [esp+30h] [ebp-54h] BYREF
    trace_t tr; // [esp+3Ch] [ebp-48h] BYREF
    int hit_type; // [esp+78h] [ebp-Ch]
    unsigned __int16 hitEntId; // [esp+7Ch] [ebp-8h]
    gentity_s *traceEnt; // [esp+80h] [ebp-4h]

    memset(&tr, 0, 16);
    if ( !wp && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon.cpp", 251, 0, "%s", "wp") )
        __debugbreak();
    if ( !wp->weapDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon.cpp", 252, 0, "%s", "wp->weapDef") )
    {
        __debugbreak();
    }
    damage = wp->weapDef->iMeleeDamage;
    if ( ent->actor )
        damage = ai_meleeDamage->current.integer;
    if ( !Melee_Trace(ent, wp, damage, range, width, height, &tr, hitOrigin) )
        return 0;
    if ( tr.hitType == TRACE_HITTYPE_GLASS )
    {
        IsBayonetWeapon = BG_IsBayonetWeapon(ent->s.weapon);
        GlassSv_Damage(tr.hitId, GLASS_MIN_DAMAGE, IsBayonetWeapon + 7, hitOrigin, wp->forward);
        return 0;
    }
    else
    {
        hitEntId = Trace_GetEntityHitId(&tr);
        traceEnt = &g_entities[hitEntId];
        hit_type = 0;
        if ( *wp->weapDef->gunXModel )
        {
            hit_type = 1;
        }
        else if ( wp->weapDef->bHasBayonet )
        {
            hit_type = 2;
        }
        else if ( ent->actor )
        {
            hit_type = 3;
        }
        if ( ent->client && traceEnt->client )
        {
            if ( level_bgs.clientinfo[ent->client->ps.clientNum].team == TEAM_FREE
                || Dvar_GetBool("scr_hardcore")
                || level_bgs.clientinfo[traceEnt->client->ps.clientNum].team != level_bgs.clientinfo[ent->client->ps.clientNum].team )
            {
                G_AddEvent(ent, 0x2Eu, hit_type);
            }
        }
        else if ( ent->client && traceEnt->sentient )
        {
            G_AddEvent(ent, 0x2Eu, hit_type);
        }
        if ( traceEnt->client || traceEnt->actor )
            tent = G_TempEntity(hitOrigin, EV_MELEE_HIT);
        else
            tent = G_TempEntity(hitOrigin, EV_MELEE_MISS);
        AssignToSmallerType<short>(&tent->s.otherEntityNum, traceEnt->s.number);
        v12 = *wp->weapDef->gunXModel && ent->client;
        tent->s.eventParm = v12;
        if ( ent->s.eType != 1 || BG_IsUseAsMeleeWeapon(ent->s.weapon) )
        {
            tent->s.weapon = ent->s.weapon;
            tent->s.weaponModel = ent->s.weaponModel;
        }
        else
        {
            AssignToSmallerType<unsigned short>(&tent->s.weapon, ent->s.lerp.u.player.meleeWeapon);
            AssignToSmallerType<unsigned char>(&tent->s.weaponModel, ent->s.lerp.u.player.meleeWeaponModel);
        }
        dir[0] = wp->forward[0];
        dir[1] = wp->forward[1];
        dir[2] = wp->forward[2];
        dir[2] = dir[2] + directionTweak;
        if ( ent->actor )
        {
            tent->s.eventParm = 3;
            AssignToSmallerType<unsigned char>(&tent->s.un1.scale, tr.boneIndex);
        }
        if ( traceEnt->s.number == 1022 )
        {
            return 0;
        }
        else if ( traceEnt->takedamage )
        {
            partName = tr.partName;
            modelIndex = tr.modelIndex;
            partGroup = (hitLocation_t)tr.partGroup;
            v8 = BG_IsBayonetWeapon(ent->s.weapon) + 7;
            v7 = G_rand();
            G_Damage(
                traceEnt,
                ent,
                ent,
                dir,
                hitOrigin,
                damage + v7 % 5,
                0,
                v8,
                0xFFFFFFFF,
                partGroup,
                modelIndex,
                partName,
                0);
            return traceEnt;
        }
        else
        {
            return 0;
        }
    }
}

bool __cdecl Melee_Trace(
                gentity_s *ent,
                weaponParms *wp,
                int damage,
                float range,
                float width,
                float height,
                trace_t *traceResult,
                float *hitOrigin)
{
    unsigned int v9; // [esp+14h] [ebp-A4h]
    float v10; // [esp+1Ch] [ebp-9Ch]
    float v11; // [esp+24h] [ebp-94h]
    int partBits[5]; // [esp+34h] [ebp-84h] BYREF
    float endPos[3]; // [esp+48h] [ebp-70h] BYREF
    unsigned int numTraces; // [esp+54h] [ebp-64h]
    trace_t traceTest; // [esp+58h] [ebp-60h] BYREF
    float end[3]; // [esp+94h] [ebp-24h] BYREF
    float start[3]; // [esp+A0h] [ebp-18h] BYREF
    gentity_s *hitEnt; // [esp+ACh] [ebp-Ch]
    unsigned int traceIndex; // [esp+B0h] [ebp-8h]
    unsigned __int16 hitEntId; // [esp+B4h] [ebp-4h]

    memset(&traceTest, 0, 16);
    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon.cpp", 159, 0, "%s", "ent") )
        __debugbreak();
    if ( !wp && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon.cpp", 160, 0, "%s", "wp") )
        __debugbreak();
    if ( !traceResult
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon.cpp", 161, 0, "%s", "traceResult") )
    {
        __debugbreak();
    }
    if ( !hitOrigin
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon.cpp", 162, 0, "%s", "hitOrigin") )
    {
        __debugbreak();
    }
    traceResult->fraction = 1.0f;
    if ( width > 0.0 || height > 0.0 )
        v9 = 9;
    else
        v9 = 1;
    numTraces = v9;
    if ( ent->client )
    {
        start[0] = (float)(-10.0 * wp->forward[0]) + wp->muzzleTrace[0];
        start[1] = (float)(-10.0 * wp->forward[1]) + wp->muzzleTrace[1];
        start[2] = (float)(-10.0 * wp->forward[2]) + wp->muzzleTrace[2];
    }
    else
    {
        start[0] = wp->muzzleTrace[0];
        start[1] = wp->muzzleTrace[1];
        start[2] = wp->muzzleTrace[2];
    }
    for ( traceIndex = 0; ; ++traceIndex )
    {
        if ( traceIndex >= numTraces )
            return traceResult->fraction < 1.0;
        end[0] = (float)(range * wp->forward[0]) + wp->muzzleTrace[0];
        end[1] = (float)(range * wp->forward[1]) + wp->muzzleTrace[1];
        end[2] = (float)(range * wp->forward[2]) + wp->muzzleTrace[2];
        if ( numTraces > 1 )
        {
            v11 = width * (float)traceOffsets[traceIndex][0];
            end[0] = (float)(v11 * wp->right[0]) + end[0];
            end[1] = (float)(v11 * wp->right[1]) + end[1];
            end[2] = (float)(v11 * wp->right[2]) + end[2];
            v10 = height * (float)traceOffsets[traceIndex][1];
            end[0] = (float)(v10 * wp->up[0]) + end[0];
            end[1] = (float)(v10 * wp->up[1]) + end[1];
            end[2] = (float)(v10 * wp->up[2]) + end[2];
        }
        G_LocationalTraceAllowChildren(
            &traceTest,
            start,
            end,
            ent->s.number,
            0x280E893,
            bulletPriorityMap);
        Vec3Lerp(start, end, traceTest.fraction, endPos);
        if ( !traceIndex )
            G_CheckHitTriggerDamage(ent, wp->muzzleTrace, endPos, damage, 7u);
        if ( traceTest.fraction == 1.0 || (traceTest.sflags & 0x10) != 0 )
        {
            if ( melee_debug->current.enabled )
                CG_DebugLine(start, end, colorRed, 1, 200);
            continue;
        }
        hitEntId = Trace_GetEntityHitId(&traceTest);
        hitEnt = &g_entities[hitEntId];
        if ( ent->client )
        {
            if ( hitEnt->sentient )
                break;
        }
        if ( melee_debug->current.enabled )
            CG_DebugLine(start, end, colorBlue, 1, 200);
        if ( traceTest.fraction <= traceResult->fraction )
        {
            memcpy(traceResult, &traceTest, sizeof(trace_t));
            *hitOrigin = endPos[0];
            hitOrigin[1] = endPos[1];
            hitOrigin[2] = endPos[2];
        }
    }
    if ( g_debugLocHit->current.integer <= 0 )
    {
        if ( melee_debug->current.enabled )
            CG_DebugLine(start, end, colorGreen, 1, 200);
    }
    else
    {
        memset(partBits, 255, sizeof(partBits));
        G_DObjCalcPose(hitEnt, partBits);
        SV_XModelDebugBoxes(hitEnt, colorYellow, 0, g_debugLocHitTime->current.integer);
        CL_AddDebugLine(start, end, colorGreen, 0, g_debugLocHitTime->current.integer);
        CL_AddDebugStar(endPos, colorRed, g_debugLocHitTime->current.integer);
    }
    memcpy(traceResult, &traceTest, sizeof(trace_t));
    *hitOrigin = endPos[0];
    hitOrigin[1] = endPos[1];
    hitOrigin[2] = endPos[2];
    return 1;
}

gentity_s *__cdecl Weapon_Throw_Grenade(gentity_s *ent, int grenType, unsigned __int8 grenModel, weaponParms *wp)
{
    int aiFuseTime; // [esp+0h] [ebp-60h]
    float iProjectileSpeedForward; // [esp+28h] [ebp-38h]
    float iProjectileSpeedRelativeUp; // [esp+38h] [ebp-28h]
    float iProjectileSpeed; // [esp+3Ch] [ebp-24h]
    gentity_s *m; // [esp+40h] [ebp-20h]
    float fAddVel; // [esp+44h] [ebp-1Ch]
    float vTossVel[3]; // [esp+48h] [ebp-18h] BYREF
    float forwardHoriz[2]; // [esp+54h] [ebp-Ch] BYREF
    int fuseTime; // [esp+5Ch] [ebp-4h]

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon.cpp", 427, 0, "%s", "ent") )
        __debugbreak();
    if ( !wp && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon.cpp", 428, 0, "%s", "wp") )
        __debugbreak();
    iProjectileSpeed = (float)wp->weapDef->iProjectileSpeed;
    vTossVel[0] = iProjectileSpeed * wp->forward[0];
    vTossVel[1] = iProjectileSpeed * wp->forward[1];
    vTossVel[2] = iProjectileSpeed * wp->forward[2];
    iProjectileSpeedRelativeUp = (float)wp->weapDef->iProjectileSpeedRelativeUp;
    vTossVel[0] = (float)(iProjectileSpeedRelativeUp * wp->up[0]) + vTossVel[0];
    vTossVel[1] = (float)(iProjectileSpeedRelativeUp * wp->up[1]) + vTossVel[1];
    vTossVel[2] = (float)wp->weapDef->iProjectileSpeedUp
                            + (float)((float)(iProjectileSpeedRelativeUp * wp->up[2]) + vTossVel[2]);
    if ( wp->weapDef->iProjectileSpeedForward )
    {
        forwardHoriz[0] = wp->forward[0];
        forwardHoriz[1] = wp->forward[1];
        Vec2NormalizeFast(forwardHoriz);
        iProjectileSpeedForward = (float)wp->weapDef->iProjectileSpeedForward;
        vTossVel[0] = (float)(iProjectileSpeedForward * forwardHoriz[0]) + vTossVel[0];
        vTossVel[1] = (float)(iProjectileSpeedForward * forwardHoriz[1]) + vTossVel[1];
    }
    if ( ent->client )
        aiFuseTime = wp->weapDef->fuseTime;
    else
        aiFuseTime = wp->weapDef->aiFuseTime;
    fuseTime = aiFuseTime;
    m = G_FireGrenade(ent, wp->muzzleTrace, vTossVel, grenType, grenModel, 1, aiFuseTime);
    Vec3Normalize(vTossVel);
    fAddVel = (float)((float)(ent->client->ps.velocity[0] * vTossVel[0])
                                    + (float)(ent->client->ps.velocity[1] * vTossVel[1]))
                    + (float)(ent->client->ps.velocity[2] * vTossVel[2]);
    m->s.lerp.pos.trDelta[0] = (float)(fAddVel * vTossVel[0]) + m->s.lerp.pos.trDelta[0];
    m->s.lerp.pos.trDelta[1] = (float)(fAddVel * vTossVel[1]) + m->s.lerp.pos.trDelta[1];
    m->s.lerp.pos.trDelta[2] = (float)(fAddVel * vTossVel[2]) + m->s.lerp.pos.trDelta[2];
    if ( ((LODWORD(m->s.lerp.pos.trDelta[0]) & 0x7F800000) == 0x7F800000
         || (LODWORD(m->s.lerp.pos.trDelta[1]) & 0x7F800000) == 0x7F800000
         || (LODWORD(m->s.lerp.pos.trDelta[2]) & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon.cpp",
                    449,
                    0,
                    "%s",
                    "!IS_NAN((m->s.lerp.pos.trDelta)[0]) && !IS_NAN((m->s.lerp.pos.trDelta)[1]) && !IS_NAN((m->s.lerp.pos.trDelta)[2])") )
    {
        __debugbreak();
    }
    return m;
}

gentity_s *__cdecl Weapon_GrenadeLauncher_Fire(
                gentity_s *ent,
                int grenType,
                unsigned __int8 grenModel,
                weaponParms *wp)
{
    int aiFuseTime; // [esp+0h] [ebp-44h]
    float iProjectileSpeed; // [esp+28h] [ebp-1Ch]
    gentity_s *m; // [esp+2Ch] [ebp-18h]
    float fAddVel; // [esp+30h] [ebp-14h]
    float vTossVel[3]; // [esp+34h] [ebp-10h] BYREF
    int fuseTime; // [esp+40h] [ebp-4h]

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon.cpp", 462, 0, "%s", "ent") )
        __debugbreak();
    if ( !wp && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon.cpp", 463, 0, "%s", "wp") )
        __debugbreak();
    fAddVel = 0.0f;
    iProjectileSpeed = (float)wp->weapDef->iProjectileSpeed;
    vTossVel[0] = iProjectileSpeed * wp->forward[0];
    vTossVel[1] = iProjectileSpeed * wp->forward[1];
    vTossVel[2] = (float)wp->weapDef->iProjectileSpeedUp + (float)(iProjectileSpeed * wp->forward[2]);
    if ( ent->client )
        aiFuseTime = wp->weapDef->fuseTime;
    else
        aiFuseTime = wp->weapDef->aiFuseTime;
    fuseTime = aiFuseTime;
    m = G_FireGrenade(ent, wp->muzzleTrace, vTossVel, grenType, grenModel, 0, aiFuseTime);
    m->flags |= 0x20000u;
    if ( ent->client )
    {
        Vec3Normalize(vTossVel);
        fAddVel = (float)((float)(ent->client->ps.velocity[0] * vTossVel[0])
                                        + (float)(ent->client->ps.velocity[1] * vTossVel[1]))
                        + (float)(ent->client->ps.velocity[2] * vTossVel[2]);
    }
    m->s.lerp.pos.trDelta[0] = (float)(fAddVel * vTossVel[0]) + m->s.lerp.pos.trDelta[0];
    m->s.lerp.pos.trDelta[1] = (float)(fAddVel * vTossVel[1]) + m->s.lerp.pos.trDelta[1];
    m->s.lerp.pos.trDelta[2] = (float)(fAddVel * vTossVel[2]) + m->s.lerp.pos.trDelta[2];
    if ( ((LODWORD(m->s.lerp.pos.trDelta[0]) & 0x7F800000) == 0x7F800000
         || (LODWORD(m->s.lerp.pos.trDelta[1]) & 0x7F800000) == 0x7F800000
         || (LODWORD(m->s.lerp.pos.trDelta[2]) & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon.cpp",
                    547,
                    0,
                    "%s",
                    "!IS_NAN((m->s.lerp.pos.trDelta)[0]) && !IS_NAN((m->s.lerp.pos.trDelta)[1]) && !IS_NAN((m->s.lerp.pos.trDelta)[2])") )
    {
        __debugbreak();
    }
    return m;
}

gentity_s* Weapon_RocketLauncher_Fire(
    gentity_s* ent,
    unsigned int weaponIndex,
    float spread,
    weaponParms* wp,
    const float* gunVel,
    gentity_s* target,
    const float* targetOffset)
{
    float v8, v9, fAimOffset, r, u;
    float dir[3], launchpos[3];
    gentity_s* m;
    
    iassert(ent);
    iassert(wp);

    v9 = spread * 0.01745329238474369;
    v8 = tan(v9);
    fAimOffset = v8 * 16.0;
    gunrandom(&r, &u);
    r = r * fAimOffset;
    u = u * fAimOffset;
    Vec3Scale(wp->forward, 16.0, dir);
    Vec3Mad(dir, r, wp->right, dir);
    Vec3Mad(dir, u, wp->up, dir);
    Vec3Normalize(dir);
    launchpos[0] = wp->muzzleTrace[0];
    launchpos[1] = wp->muzzleTrace[1];
    launchpos[2] = wp->muzzleTrace[2];
    m = G_FireRocket(ent, weaponIndex, launchpos, dir, gunVel, target, targetOffset);
    if (ent->client && wp->weapDef->fireType)
        Vec3Mad(ent->client->ps.velocity, -64.0, wp->forward, ent->client->ps.velocity);

    return m;
}

void __cdecl gunrandom(float *x, float *y)
{
    long double v2; // st7
    float sinT; // [esp+Ch] [ebp-10h]
    float theta; // [esp+10h] [ebp-Ch]
    float r; // [esp+14h] [ebp-8h]
    float cosT; // [esp+18h] [ebp-4h]

    theta = G_random() * 360.0;
    r = G_random();
    v2 = (float)(theta * 0.017453292);
    cosT = cos(v2);
    sinT = sin(v2);
    *x = r * cosT;
    *y = r * sinT;
}

gentity_s *__cdecl Weapon_BombDrop_Fire(
                gentity_s *ent,
                unsigned int weaponIndex,
                float spread,
                weaponParms *wp,
                const float *gunVel,
                gentity_s *target,
                const float *targetOffset)
{
    long double v8; // [esp+0h] [ebp-4Ch]
    float fAimOffset; // [esp+20h] [ebp-2Ch]
    float r; // [esp+28h] [ebp-24h] BYREF
    float dir[3]; // [esp+2Ch] [ebp-20h] BYREF
    float fDist; // [esp+38h] [ebp-14h]
    float launchpos[3]; // [esp+3Ch] [ebp-10h] BYREF
    float u; // [esp+48h] [ebp-4h] BYREF

    fDist = 16.0f;
    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon.cpp", 608, 0, "%s", "ent") )
        __debugbreak();
    if ( !wp && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon.cpp", 609, 0, "%s", "wp") )
        __debugbreak();
    //__libm_sse2_tan(v8);
    //fAimOffset = (float)(spread * 0.017453292) * fDist;
    fAimOffset = (float)tan(spread * 0.017453292) * fDist;
    gunrandom(&r, &u);
    r = r * fAimOffset;
    u = u * fAimOffset;
    dir[0] = fDist * wp->forward[0];
    dir[1] = fDist * wp->forward[1];
    dir[2] = fDist * wp->forward[2];
    dir[0] = (float)(r * wp->right[0]) + dir[0];
    dir[1] = (float)(r * wp->right[1]) + dir[1];
    dir[2] = (float)(r * wp->right[2]) + dir[2];
    dir[0] = (float)(u * wp->up[0]) + dir[0];
    dir[1] = (float)(u * wp->up[1]) + dir[1];
    dir[2] = (float)(u * wp->up[2]) + dir[2];
    Vec3Normalize(dir);
    launchpos[0] = wp->muzzleTrace[0];
    launchpos[1] = wp->muzzleTrace[1];
    launchpos[2] = wp->muzzleTrace[2];
    return G_DropBomb(ent, weaponIndex, launchpos, dir, gunVel, target, targetOffset);
}

void    Weapon_Overheat_Update(gentity_s *ent)
{
    PlayerHeldWeapon *weaponState; // [esp+10h] [ebp-2Ch]
    const vehicle_info_t *info; // [esp+14h] [ebp-28h]
    int startSlot; // [esp+18h] [ebp-24h]
    gentity_s *turretEnt; // [esp+1Ch] [ebp-20h]
    gentity_s *turretEnta; // [esp+1Ch] [ebp-20h]
    int slot; // [esp+20h] [ebp-1Ch]
    int frametime; // [esp+24h] [ebp-18h]
    int turretIndex; // [esp+28h] [ebp-14h]
    unsigned int weaponIndex; // [esp+2Ch] [ebp-10h]
    unsigned int weaponIndexa; // [esp+2Ch] [ebp-10h]
    const WeaponDef *weapDef; // [esp+30h] [ebp-Ch]
    const WeaponDef *weapDefa; // [esp+30h] [ebp-Ch]
    gclient_s *ps; // [esp+34h] [ebp-8h]
    int seatIndex; // [esp+38h] [ebp-4h]

    if ( !ent->client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon.cpp", 642, 0, "%s", "ent->client") )
    {
        __debugbreak();
    }
    frametime = level.time - level.previousTime;
    if ( level.time - level.previousTime < 0 )
        frametime = 0;
    ps = ent->client;
    slot = -1;
    if ( ps->ps.weapon )
        slot = BG_GetHeldWeaponSlot(&ps->ps, ps->ps.weapon);
    if ( slot == -1 )
        slot = 0;
    startSlot = slot;
    do
    {
        weaponIndex = ps->ps.heldWeapons[slot].weapon;
        if ( weaponIndex )
        {
            weapDef = BG_GetWeaponDef(weaponIndex);
            if ( weapDef->overheatWeapon )
            {
                if ( (ps->ps.weaponstate == 6 || ps->ps.weaponstate == 31)
                    && ps->ps.weapon == weaponIndex
                    && ps->ps.waterlevel < 3 )
                {
                    BG_PlayerWeaponOverheatUpdate(
                        &ps->ps,
                        weaponIndex,
                        (float)((float)frametime * weapDef->overheatRate) / 1000.0);
                }
                if ( ps->ps.weaponstate != 6 && ps->ps.weaponstate != 31
                    || ps->ps.weapon != weaponIndex
                    || weapDef->coolWhileFiring
                    || ps->ps.waterlevel >= 3 )
                {
                    //amount.m128_f32[0] = (float)((float)frametime * weapDef->cooldownRate) / 1000.0;
                    //amount = _mm_xor_ps(amount, (__m128)_mask__NegFloat_);
                    //BG_PlayerWeaponOverheatUpdate(&ps->ps, weaponIndex, amount.m128_f32[0]);
                    BG_PlayerWeaponOverheatUpdate(&ps->ps, weaponIndex, -(frametime * weapDef->cooldownRate) / 1000.0);
                }
            }
        }
        slot = (slot + 1) % 15;
    }
    while ( slot != startSlot );
    if ( (ps->ps.eFlags & 0x300) != 0 )
    {
        turretEnt = &g_entities[ps->ps.viewlocked_entNum];
        weapDefa = BG_GetWeaponDef(turretEnt->s.weapon);
        if ( weapDefa->overheatWeapon && (turretEnt->s.lerp.eFlags & 0x40) != 0 )
        {
            turretEnt->pTurretInfo->heatVal = (float)((float)((float)frametime * weapDefa->overheatRate) / 1000.0)
                                                                            + turretEnt->pTurretInfo->heatVal;
            if ( turretEnt->pTurretInfo->heatVal >= 100.0 )
            {
                turretEnt->pTurretInfo->overheating = 1;
                turretEnt->pTurretInfo->heatVal = 100.0f;
            }
        }
    }
    else if ( (ps->ps.eFlags & 0x4000) != 0 )
    {
        turretEnta = &g_entities[ps->ps.viewlocked_entNum];
        seatIndex = ps->ps.vehiclePos;
        turretIndex = 1;
        if ( turretEnta->scr_vehicle )
        {
            info = BG_GetVehicleInfo(turretEnta->scr_vehicle->infoIdx);
            if ( seatIndex < 1 || seatIndex > 4 )
            {
                weaponIndexa = 0;
            }
            else
            {
                turretIndex = seatIndex - 1;
                weaponIndexa = *(unsigned __int16 *)&info->gunnerWeapon[3][2 * seatIndex + 62];
            }
            if ( weaponIndexa )
            {
                if ( BG_GetWeaponDef(weaponIndexa)->overheatWeapon )
                {
                    weaponState = BG_GetHeldWeapon(&ps->ps, weaponIndexa);
                    if ( weaponState )
                    {
                        weaponState->overHeating = turretEnta->scr_vehicle->gunnerTurrets[turretIndex].overheating != 0;
                        weaponState->heatPercent = turretEnta->scr_vehicle->gunnerTurrets[turretIndex].heatVal;
                    }
                }
            }
        }
    }
}

void __cdecl Weapon_Flamethrower_Update(gentity_s *ent, weaponParms *wp)
{
    int Time; // eax
    bool v3; // [esp+0h] [ebp-9Ch]
    flameWeaponConfig_t flameWeaponConfig; // [esp+1Ch] [ebp-80h] BYREF
    float angles[3]; // [esp+68h] [ebp-34h] BYREF
    float axis[3][3]; // [esp+74h] [ebp-28h] BYREF
    bool isFiring; // [esp+9Bh] [ebp-1h]

    if ( ent->s.number >= 32 )
    {
        isFiring = (ent->s.lerp.eFlags & 0x40) != 0;
    }
    else
    {
        v3 = (ent->client->ps.eFlags & 0x40) != 0 && (ent->client->ps.eFlags & 0x300) == 0 && ent->client->ps.waterlevel < 3;
        isFiring = v3;
    }
    *(_QWORD *)&axis[0][0] = *(_QWORD *)wp->forward;
    axis[0][2] = wp->forward[2];
    axis[1][0] = wp->right[0];
    axis[1][1] = wp->right[1];
    axis[1][2] = wp->right[2];
    axis[2][0] = wp->up[0];
    axis[2][1] = wp->up[1];
    axis[2][2] = wp->up[2];
    AxisToAngles(axis, angles);
    flameWeaponConfig.origin[0] = wp->muzzleTrace[0];
    flameWeaponConfig.origin[1] = wp->muzzleTrace[1];
    flameWeaponConfig.origin[2] = wp->muzzleTrace[2];
    flameWeaponConfig.angle[0] = angles[0];
    flameWeaponConfig.angle[1] = angles[1];
    flameWeaponConfig.angle[2] = angles[2];
    flameWeaponConfig.fTable = wp->weapDef->flameTableThirdPersonPtr;
    flameWeaponConfig.bFireWhileIdle = 0;
    flameWeaponConfig.bIsFiring = isFiring;
    flameWeaponConfig.burnRate = 1.0f;
    flameWeaponConfig.strength = 1.0f;
    flameWeaponConfig.thickness = 1.0f;
    flameWeaponConfig.entityOrigin[0] = ent->r.currentOrigin[0];
    flameWeaponConfig.entityOrigin[1] = ent->r.currentOrigin[1];
    flameWeaponConfig.entityOrigin[2] = ent->r.currentOrigin[2];
    flameWeaponConfig.damage = wp->weapDef->damage;
    flameWeaponConfig.damageDuration = wp->weapDef->damageDuration;
    flameWeaponConfig.damageInterval = wp->weapDef->damageInterval;
    Time = G_GetTime();
    SV_Flame_Update_Source(ent->s.number, &flameWeaponConfig, Time);
    //BLOPS_NULLSUB();
}

void __cdecl Weapon_Flamethrower_Fire(gentity_s *ent, weaponParms *wp)
{
    int Time; // eax
    bool v3; // [esp+0h] [ebp-9Ch]
    float angles[3]; // [esp+1Ch] [ebp-80h] BYREF
    float axis[3][3]; // [esp+28h] [ebp-74h] BYREF
    flameWeaponConfig_t flameWeaponConfig; // [esp+4Ch] [ebp-50h] BYREF
    bool isFiring; // [esp+9Bh] [ebp-1h]

    if ( ent->client )
    {
        v3 = (ent->client->ps.eFlags & 0x40) != 0 && (ent->client->ps.eFlags & 0x300) == 0;
        isFiring = v3;
    }
    else
    {
        isFiring = 1;
        ent->s.lerp.eFlags |= 0x40u;
    }
    *(_QWORD *)&axis[0][0] = *(_QWORD *)wp->forward;
    axis[0][2] = wp->forward[2];
    axis[1][0] = wp->right[0];
    axis[1][1] = wp->right[1];
    axis[1][2] = wp->right[2];
    axis[2][0] = wp->up[0];
    axis[2][1] = wp->up[1];
    axis[2][2] = wp->up[2];
    AxisToAngles(axis, angles);
    flameWeaponConfig.origin[0] = wp->muzzleTrace[0];
    flameWeaponConfig.origin[1] = wp->muzzleTrace[1];
    flameWeaponConfig.origin[2] = wp->muzzleTrace[2];
    flameWeaponConfig.angle[0] = angles[0];
    flameWeaponConfig.angle[1] = angles[1];
    flameWeaponConfig.angle[2] = angles[2];
    flameWeaponConfig.fTable = wp->weapDef->flameTableThirdPersonPtr;
    if ( !wp->weapDef->flameTableThirdPersonPtr
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon.cpp",
                    883,
                    0,
                    "%s\n\t%s",
                    "wp->weapDef->flameTableThirdPersonPtr",
                    "No third person flametable exists for weapon, this is required!") )
    {
        __debugbreak();
    }
    flameWeaponConfig.bFireWhileIdle = 0;
    flameWeaponConfig.bIsFiring = isFiring;
    flameWeaponConfig.burnRate = 1.0f;
    flameWeaponConfig.strength = 1.0f;
    flameWeaponConfig.thickness = 1.0f;
    flameWeaponConfig.entityOrigin[0] = ent->r.currentOrigin[0];
    flameWeaponConfig.entityOrigin[1] = ent->r.currentOrigin[1];
    flameWeaponConfig.entityOrigin[2] = ent->r.currentOrigin[2];
    flameWeaponConfig.damage = wp->weapDef->damage;
    flameWeaponConfig.damageDuration = wp->weapDef->damageDuration;
    flameWeaponConfig.damageInterval = wp->weapDef->damageInterval;
    Time = G_GetTime();
    SV_Flame_Update_Source(ent->s.number, &flameWeaponConfig, Time);
}

void __cdecl Weapon_Napalm_Flame(gentity_s *ent, trace_t *trace, int count)
{
    double v3; // st7
    double v4; // st7
    double v5; // st7
    gentity_s *v6; // eax
    int v7; // [esp-8h] [ebp-84h]
    float v8; // [esp+8h] [ebp-74h]
    int i; // [esp+10h] [ebp-6Ch]
    gentity_s *grenade; // [esp+14h] [ebp-68h]
    float new_origin[3]; // [esp+18h] [ebp-64h] BYREF
    int iWeaponIndex; // [esp+24h] [ebp-58h]
    float forward[3]; // [esp+28h] [ebp-54h] BYREF
    weaponParms wp; // [esp+34h] [ebp-48h] BYREF

    iWeaponIndex = G_GetWeaponIndexForName((char*)"napalmblob_mp");
    Weapon_SetWeaponParamsWeapon(&wp, iWeaponIndex);
    if ( wp.weapDef->weapType != WEAPTYPE_GRENADE
        && wp.weapDef->weapType != WEAPTYPE_MINE
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon.cpp",
                    942,
                    0,
                    "%s\n\t%s",
                    "wp.weapDef->weapType == WEAPTYPE_GRENADE || wp.weapDef->weapType == WEAPTYPE_MINE",
                    "Did you remember to add the napalmblob to the level csv?") )
    {
        __debugbreak();
    }
    for ( i = 0; i < count; ++i )
    {
        new_origin[0] = ent->r.currentOrigin[0];
        new_origin[1] = ent->r.currentOrigin[1];
        new_origin[2] = ent->r.currentOrigin[2];
        v3 = G_flrand(-36.0, 36.0);
        new_origin[0] = v3 + new_origin[0];
        v4 = G_flrand(-36.0, 36.0);
        new_origin[1] = v4 + new_origin[1];
        v5 = G_flrand(-36.0, 36.0);
        new_origin[2] = v5 + new_origin[2];
        AngleVectors(ent->r.currentAngles, forward, 0, 0);
        forward[0] = -forward[0];
        forward[1] = -forward[1];
        forward[2] = -forward[2];
        v8 = G_flrand((float)wp.weapDef->iProjectileSpeed / 8.0, (float)wp.weapDef->iProjectileSpeed / 4.0);
        forward[0] = v8 * forward[0];
        forward[1] = v8 * forward[1];
        forward[2] = v8 * forward[2];
        forward[2] = (float)wp.weapDef->iProjectileSpeedUp + forward[2];
        if ( ent->parent.isDefined() )
        {
            v7 = iWeaponIndex;
            //v6 = EntHandle::ent(&ent->parent);
            v6 = ent->parent.ent();
            G_FireGrenade(v6, new_origin, forward, v7, 0, 0, 0);
        }
        else
        {
            grenade = G_FireGrenade(ent, new_origin, forward, iWeaponIndex, 0, 0, 0);
            grenade->r.ownerNum.setEnt(0);
            grenade->parent.setEnt(0);
        }
    }
}

bool __cdecl LogAccuracyHit(gentity_s *target, gentity_s *attacker)
{
    if ( !target && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon.cpp", 990, 0, "%s", "target") )
        __debugbreak();
    if ( !attacker && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon.cpp", 991, 0, "%s", "attacker") )
        __debugbreak();
    if ( !target->takedamage )
        return 0;
    if ( target == attacker )
        return 0;
    if ( !target->client )
        return 0;
    if ( !attacker->client )
        return 0;
    if ( target->client->ps.pm_type < 9 )
        return !OnSameTeam(target, attacker);
    return 0;
}

void __cdecl G_CalcMuzzlePoints(const gentity_s *ent, weaponParms *wp, int shotCount)
{
    float *viewangles; // [esp+Ch] [ebp-18h]
    float spreadAngle; // [esp+10h] [ebp-14h]
    float angle; // [esp+14h] [ebp-10h]
    float viewang[3]; // [esp+18h] [ebp-Ch] BYREF

    if ( ent->client )
    {
        viewangles = ent->client->ps.viewangles;
        viewang[0] = *viewangles;
        viewang[1] = viewangles[1];
        viewang[2] = viewangles[2];
        viewang[0] = ent->client->fGunPitch;
        viewang[1] = ent->client->fGunYaw;
        if ( wp->weapDef->fireType == WEAPON_FIRETYPE_STACKED && ent->client->ps.stackFireCount > 1 )
        {
            spreadAngle = wp->weapDef->stackFireSpread;
            if ( ent->client->ps.stackFireCount == 2 )
            {
                if ( ent->client->ps.weaponShotCount == 2 )
                    angle = spreadAngle / 2.0;
                else
                    angle = (-(spreadAngle)) / 2.0;
            }
            else
            {
                angle = spreadAngle / (double)(ent->client->ps.stackFireCount - 1) * (double)(shotCount - 1) - spreadAngle / 2.0;
            }
            viewang[1] = viewang[1] + angle;
        }
        AngleVectors(viewang, wp->forward, wp->right, wp->up);
        G_GetPlayerViewOrigin(&ent->client->ps, wp->muzzleTrace);
    }
}

void __cdecl FireWeapon(gentity_s *ent, int gametime, int shotCount)
{
    char *v3; // eax
    gentity_s *target; // [esp+14h] [ebp-6Ch]
    float offset[3]; // [esp+18h] [ebp-68h] BYREF
    float fAimSpreadAmount; // [esp+24h] [ebp-5Ch]
    float minSpread; // [esp+28h] [ebp-58h] BYREF
    float maxSpread; // [esp+2Ch] [ebp-54h] BYREF
    weaponParms wp; // [esp+30h] [ebp-50h] BYREF
    float aimSpreadScale; // [esp+7Ch] [ebp-4h]
    int savedregs; // [esp+80h] [ebp+0h] BYREF

    if ( (ent->client->ps.eFlags & 0x300) == 0 || !ent->active )
    {
        v3 = (char *)BG_WeaponName(ent->s.weapon);
        Scr_AddString(v3, SCRIPTINSTANCE_SERVER);
        Scr_Notify(ent, scr_const.weapon_fired, 1u);
        Weapon_SetWeaponParamsWeapon(&wp, ent->s.weapon);
        G_CalcMuzzlePoints(ent, &wp, shotCount);
        aimSpreadScale = ent->client->currentAimSpreadScale;
        BG_GetSpreadForWeapon(&ent->client->ps, wp.weapDef, &minSpread, &maxSpread);
        if ( ent->client->ps.fWeaponPosFrac == 1.0 )
            fAimSpreadAmount = (float)((float)(maxSpread - wp.weapDef->fAdsSpread) * aimSpreadScale) + wp.weapDef->fAdsSpread;
        else
            fAimSpreadAmount = (float)((float)(maxSpread - minSpread) * aimSpreadScale) + minSpread;
        if ( wp.weapDef->weapType )
        {
            switch ( wp.weapDef->weapType )
            {
                case WEAPTYPE_GRENADE:
                case WEAPTYPE_MINE:
                    Weapon_Throw_Grenade(ent, ent->s.weapon, ent->s.weaponModel, &wp);
                    break;
                case WEAPTYPE_PROJECTILE:
                    if ( wp.weapDef->weapClass == WEAPCLASS_GRENADE )
                    {
                        Weapon_GrenadeLauncher_Fire(ent, ent->s.weapon, ent->s.weaponModel, &wp);
                    }
                    else
                    {
                        if ( (ent->client->ps.weapLockFlags & 2) != 0 )
                        {
                            target = &level.gentities[ent->client->ps.weapLockedEntnum];
                            G_TargetGetOffset(target, offset);
                        }
                        else
                        {
                            target = 0;
                            memset(offset, 0, sizeof(offset));
                        }
                        Weapon_RocketLauncher_Fire(
                            ent,
                            ent->s.weapon,
                            fAimSpreadAmount,
                            &wp,
                            vec3_origin,
                            target,
                            offset);
                    }
                    break;
                case WEAPTYPE_GAS:
                    Weapon_Flamethrower_Fire(ent, &wp);
                    break;
                default:
                    Com_Error(ERR_DROP, "Unknown weapon type %i for %s", wp.weapDef->weapType, wp.weapVariantDef->szInternalName);
                    break;
            }
        }
        else
        {
            Bullet_Fire(ent, fAimSpreadAmount, &wp, ent, gametime);
        }
    }
}

void __cdecl DeployWeapon(gentity_s *ent)
{
    double v1; // st7
    float v2; // [esp+10h] [ebp-1D4h]
    float v3; // [esp+14h] [ebp-1D0h]
    float *trBase; // [esp+18h] [ebp-1CCh]
    float *currentAngles; // [esp+1Ch] [ebp-1C8h]
    float v[2]; // [esp+48h] [ebp-19Ch] BYREF
    float v7; // [esp+50h] [ebp-194h]
    float v8; // [esp+64h] [ebp-180h]
    float v9; // [esp+68h] [ebp-17Ch]
    float v10; // [esp+7Ch] [ebp-168h]
    float *origin; // [esp+8Ch] [ebp-158h]
    float mountDir; // [esp+90h] [ebp-154h]
    float *mountPos; // [esp+94h] [ebp-150h]
    float f; // [esp+98h] [ebp-14Ch]
    float ledgeDepth; // [esp+9Ch] [ebp-148h]
    float traceHit[3]; // [esp+A0h] [ebp-144h]
    float ledgeAngles[3]; // [esp+ACh] [ebp-138h]
    int drawTime; // [esp+B8h] [ebp-12Ch]
    col_context_t context; // [esp+BCh] [ebp-128h] BYREF
    float toLedge[3]; // [esp+E4h] [ebp-100h] BYREF
    bool drawLines; // [esp+F3h] [ebp-F1h]
    gentity_s *turretEnt; // [esp+F4h] [ebp-F0h]
    float ledgeDir[3]; // [esp+F8h] [ebp-ECh] BYREF
    float traceWidth; // [esp+104h] [ebp-E0h]
    float mins[3]; // [esp+108h] [ebp-DCh] BYREF
    float start[3]; // [esp+114h] [ebp-D0h] BYREF
    float end[3]; // [esp+120h] [ebp-C4h] BYREF
    int tracemask; // [esp+12Ch] [ebp-B8h]
    float bipodNudgeDist; // [esp+130h] [ebp-B4h]
    trace_t trace; // [esp+134h] [ebp-B0h] BYREF
    float maxs[3]; // [esp+170h] [ebp-74h] BYREF
    float tracedLedgePos[3]; // [esp+17Ch] [ebp-68h] BYREF
    float up[3]; // [esp+188h] [ebp-5Ch]
    bool bBulletHit; // [esp+197h] [ebp-4Dh]
    playerState_s *ps; // [esp+198h] [ebp-4Ch]
    const WeaponDef *weapDef; // [esp+19Ch] [ebp-48h]
    float distToTurret; // [esp+1A0h] [ebp-44h]
    float color[4]; // [esp+1A4h] [ebp-40h] BYREF
    float ledgePos[3]; // [esp+1B4h] [ebp-30h] BYREF
    float toPlayer[3]; // [esp+1C0h] [ebp-24h] BYREF
    float traceStart[3]; // [esp+1CCh] [ebp-18h] BYREF
    float traceEnd[3]; // [esp+1D8h] [ebp-Ch] BYREF

    turretEnt = 0;
    memset(&trace, 0, 16);
    traceWidth = 4.0f;
    bipodNudgeDist = bipodNudgeDistDefault;
    //col_context_t::col_context_t(&context);
    drawTime = 1000;
    drawLines = cg_debugMounting->current.integer > 0;
    up[0] = 0.0f;
    up[1] = 0.0f;
    up[2] = 1.0f;
    if ( !ent->client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon.cpp", 1276, 0, "%s", "ent->client") )
    {
        __debugbreak();
    }
    ps = &ent->client->ps;
    if ( (ps->eFlags & 0x300) == 0 && !ent->client->lastStand && ps->groundEntityNum != 1023 && ps->velocity[2] >= 0.0 )
    {
        weapDef = BG_GetWeaponDef(ps->weapon);
        if ( weapDef->mountableWeapon )
        {
            mountPos = ps->mountPos;
            ledgePos[0] = ps->mountPos[0];
            ledgePos[1] = ps->mountPos[1];
            ledgePos[2] = ps->mountPos[2];
            mountDir = ps->mountDir;
            ledgeAngles[0] = 0.0f;
            ledgeAngles[1] = mountDir;
            ledgeAngles[2] = 0.0f;
            if ( drawLines )
            {
                color[0] = 0.0f;
                color[1] = 1.0f;
                color[2] = 0.5f;
                color[3] = 1.0f;
                G_DebugStar(ledgePos, color, 1000);
            }
            tracedLedgePos[0] = ledgePos[0];
            tracedLedgePos[1] = ledgePos[1];
            tracedLedgePos[2] = ledgePos[2];
            origin = ps->origin;
            toPlayer[0] = ps->origin[0] - ledgePos[0];
            toPlayer[1] = ps->origin[1] - ledgePos[1];
            toPlayer[2] = 0.0f;
            distToTurret = Vec3Normalize(toPlayer) + traceOffset + 1.0;
            traceStart[0] = 0.0f;
            traceStart[1] = 1.0f;
            traceStart[2] = 0.0f;
            YawVectors(ledgeAngles[1] + 180.0, traceStart, 0);
            traceStart[0] = (float)(distToTurret * traceStart[0]) + ledgePos[0];
            traceStart[1] = (float)(distToTurret * traceStart[1]) + ledgePos[1];
            traceStart[2] = (float)(distToTurret * traceStart[2]) + ledgePos[2];
            traceStart[2] = traceStart[2] - traceOffset;
            traceEnd[0] = 0.0f;
            traceEnd[1] = 1.0f;
            traceEnd[2] = 0.0f;
            YawVectors(ledgeAngles[1], ledgeDir, 0);
            traceEnd[0] = (float)(distToTurret * ledgeDir[0]) + ledgePos[0];
            traceEnd[1] = (float)(distToTurret * ledgeDir[1]) + ledgePos[1];
            traceEnd[2] = (float)((float)(distToTurret * ledgeDir[2]) + ledgePos[2]) - traceOffset;
            mins[0] = -0.1f;
            mins[1] = -0.1f;
            mins[2] = 0.0f;
            v10 = traceOffset * 2.0;
            maxs[0] = 0.1f;
            maxs[1] = 0.1f;
            maxs[2] = traceOffset * 2.0;
            YawVectors(ledgeAngles[1], toLedge, 0);
            bBulletHit = 1;
            traceStart[2] = traceStart[2] + traceOffset;
            traceEnd[2] = traceEnd[2] + traceOffset;
            tracemask = linemask;
            G_LocationalTraceAllowChildren(
                &trace,
                traceStart,
                traceEnd,
                ent->s.number,
                0x280E833,
                0);
            if ( trace.fraction <= 0.0 || trace.fraction >= 1.0 )
            {
                bBulletHit = 0;
                if ( drawLines )
                {
                    start[0] = traceStart[0];
                    start[1] = traceStart[1];
                    end[0] = traceEnd[0];
                    end[1] = traceEnd[1];
                    start[2] = traceStart[2] + 0.80000001;
                    end[2] = traceEnd[2] + 0.80000001;
                    color[0] = 1.0f;
                    color[1] = 0.3f;
                    color[2] = 0.6f;
                    color[3] = 1.0f;
                    CG_DebugLine(start, end, color, 0, 1000);
                }
                traceStart[2] = traceStart[2] - traceOffset;
                traceEnd[2] = traceEnd[2] - traceOffset;
                tracemask = startmask;
                G_TraceCapsule(&trace, traceStart, mins, maxs, traceEnd, ent->s.number, startmask, &context);
                if ( trace.fraction <= 0.0 || trace.fraction >= 1.0 )
                {
                    tracemask = secondmask;
                    G_TraceCapsule(&trace, traceStart, mins, maxs, traceEnd, ent->s.number, secondmask, &context);
                }
            }
            if ( trace.fraction <= 0.0 || trace.fraction >= 1.0 )
            {
                tracedLedgePos[0] = (float)(traceNoHitPushDist * toLedge[0]) + tracedLedgePos[0];
                tracedLedgePos[1] = (float)(traceNoHitPushDist * toLedge[1]) + tracedLedgePos[1];
                tracedLedgePos[2] = (float)(traceNoHitPushDist * toLedge[2]) + tracedLedgePos[2];
                if ( drawLines )
                {
                    start[0] = traceStart[0];
                    start[1] = traceStart[1];
                    start[2] = traceStart[2];
                    end[0] = traceEnd[0];
                    end[1] = traceEnd[1];
                    end[2] = traceEnd[2];
                    color[0] = 1.0f;
                    color[1] = 0.0f;
                    color[2] = 0.0f;
                    color[3] = 1.0f;
                    CG_DebugLine(start, end, color, 0, 1000);
                    CG_DebugBox(traceEnd, mins, maxs, 0.0, color, 0, 1000);
                }
            }
            else
            {
                v1 = Vec3Distance(traceStart, traceEnd);
                v9 = v1 * trace.fraction;
                tracedLedgePos[0] = (float)(v9 * toLedge[0]) + traceStart[0];
                tracedLedgePos[1] = (float)(v9 * toLedge[1]) + traceStart[1];
                tracedLedgePos[2] = ledgePos[2];
                if ( bBulletHit )
                {
                    G_LocationalTraceAllowChildren(
                        &trace,
                        traceEnd,
                        traceStart,
                        ent->s.number,
                        0x280E833,
                        0);
                    if ( trace.fraction <= 0.0 || trace.fraction >= 1.0 )
                    {
                        v8 = distToTurret + traceEndPosAdd;
                        traceEnd[0] = (float)((float)(distToTurret + traceEndPosAdd) * ledgeDir[0]) + ledgePos[0];
                        traceEnd[1] = (float)((float)(distToTurret + traceEndPosAdd) * ledgeDir[1]) + ledgePos[1];
                        traceEnd[2] = (float)((float)(distToTurret + traceEndPosAdd) * ledgeDir[2]) + ledgePos[2];
                        G_LocationalTraceAllowChildren(
                            &trace,
                            traceEnd,
                            traceStart,
                            ent->s.number,
                            0x280E833,
                            0);
                    }
                }
                else
                {
                    G_TraceCapsule(&trace, traceEnd, mins, maxs, traceStart, ent->s.number, tracemask, &context);
                }
                if ( trace.fraction > 0.0 && trace.fraction < 1.0 )
                {
                    v3 = 1.0 - trace.fraction;
                    v7 = Vec3Distance(traceStart, traceEnd) * v3;
                    traceHit[0] = (float)(v7 * toLedge[0]) + traceStart[0];
                    traceHit[1] = (float)(v7 * toLedge[1]) + traceStart[1];
                    traceHit[2] = (float)(v7 * toLedge[2]) + traceStart[2];
                    v[0] = tracedLedgePos[0] - traceHit[0];
                    v[1] = tracedLedgePos[1] - traceHit[1];
                    ledgeDepth = Vec2Length(v);
                    if ( ledgeDepthNudgeReductionStart > ledgeDepth )
                    {
                        f = (float)(ledgeDepthNudgeReductionStart - ledgeDepth)
                            / (float)(ledgeDepthNudgeReductionStart - ledgeDepthNudgeReductionEnd);
                        if ( f > 1.0 )
                            f = 1.0f;
                        bipodNudgeDist = (float)(1.0 - f) * bipodNudgeDist;
                    }
                }
                if ( drawLines )
                {
                    start[0] = traceStart[0];
                    start[1] = traceStart[1];
                    start[2] = traceStart[2];
                    end[0] = traceEnd[0];
                    end[1] = traceEnd[1];
                    end[2] = traceEnd[2];
                    color[0] = 0.0f;
                    color[1] = 1.0f;
                    color[2] = 0.0f;
                    color[3] = 1.0f;
                    CG_DebugLine(start, end, color, 0, 1000);
                    CG_DebugBox(traceEnd, mins, maxs, 0.0, color, 0, 1000);
                    if ( trace.fraction > 0.0 && trace.fraction < 1.0 )
                    {
                        color[0] = 1.0f;
                        color[1] = 0.0f;
                        color[2] = 0.0f;
                        color[3] = 1.0f;
                        start[0] = tracedLedgePos[0];
                        start[1] = tracedLedgePos[1];
                        end[0] = traceHit[0];
                        end[1] = traceHit[1];
                        start[2] = tracedLedgePos[2] + 0.5;
                        end[2] = traceHit[2] + 0.5;
                        CG_DebugLine(start, end, color, 0, 1000);
                    }
                }
            }
            traceStart[0] = tracedLedgePos[0];
            traceStart[1] = tracedLedgePos[1];
            traceStart[2] = tracedLedgePos[2];
            if ( bBulletHit )
                v2 = bulletLedgeHeightRaiseAdd;
            else
                v2 = 0.0f;
            traceStart[2] = (float)(traceOffset + v2) + traceStart[2];
            traceEnd[0] = tracedLedgePos[0];
            traceEnd[1] = tracedLedgePos[1];
            traceEnd[2] = tracedLedgePos[2] - traceOffset;
            mins[0] = -traceWidth;
            mins[1] = -traceWidth;
            mins[2] = 0.0f;
            maxs[0] = traceWidth;
            maxs[1] = traceWidth;
            maxs[2] = traceWidth * 2.0;
            tracedLedgePos[0] = (float)(bipodNudgeDist * toLedge[0]) + tracedLedgePos[0];
            tracedLedgePos[1] = (float)(bipodNudgeDist * toLedge[1]) + tracedLedgePos[1];
            tracedLedgePos[2] = (float)(bipodNudgeDist * toLedge[2]) + tracedLedgePos[2];
            if ( bBulletHit )
            {
                if ( drawLines )
                {
                    start[0] = traceStart[0];
                    start[1] = traceStart[1];
                    start[2] = traceStart[2];
                    end[0] = traceEnd[0];
                    end[1] = traceEnd[1];
                    end[2] = traceEnd[2];
                    color[0] = 1.0f;
                    color[1] = 0.5f;
                    color[2] = 0.5f;
                    color[3] = 1.0f;
                    CG_DebugLine(start, end, color, 0, 1000);
                }
                G_LocationalTraceAllowChildren(
                    &trace,
                    traceStart,
                    traceEnd,
                    ent->s.number,
                    0x280E833,
                    0);
                if ( trace.fraction <= 0.0 || trace.fraction >= 1.0 )
                {
                    traceStart[2] = traceStart[2] - bulletLedgeHeightRaiseAdd;
                    G_TraceCapsule(&trace, traceStart, mins, maxs, traceEnd, ent->s.number, 65, &context);
                }
            }
            else
            {
                G_TraceCapsule(&trace, traceStart, mins, maxs, traceEnd, ent->s.number, 65, &context);
            }
            if ( trace.fraction <= 0.0 || trace.fraction >= 1.0 )
            {
                mins[0] = -traceOffset;
                mins[1] = -traceOffset;
                mins[2] = 0.0f;
                maxs[0] = traceOffset;
                maxs[1] = traceOffset;
                maxs[2] = traceOffset * 2.0;
                tracedLedgePos[0] = (float)((-(bipodNudgeDist)) * toLedge[0]) + tracedLedgePos[0];
                tracedLedgePos[1] = (float)((-(bipodNudgeDist)) * toLedge[1]) + tracedLedgePos[1];
                tracedLedgePos[2] = (float)((-(bipodNudgeDist)) * toLedge[2]) + tracedLedgePos[2];
                G_TraceCapsule(&trace, traceStart, mins, maxs, traceEnd, ent->s.number, 65, &context);
                if ( trace.fraction > 0.0 && trace.fraction < 1.0 )
                    tracedLedgePos[2] = (float)((float)(traceEnd[2] - traceStart[2]) * trace.fraction) + traceStart[2];
            }
            else
            {
                tracedLedgePos[2] = (float)((float)(traceEnd[2] - traceStart[2]) * trace.fraction) + traceStart[2];
            }
            if ( drawLines )
            {
                start[0] = ps->origin[0];
                start[1] = ps->origin[1];
                start[2] = ps->origin[2];
                end[0] = (float)(20.0 * up[0]) + start[0];
                end[1] = (float)(20.0 * up[1]) + start[1];
                end[2] = (float)(20.0 * up[2]) + start[2];
                color[0] = 0.0f;
                color[1] = 0.0f;
                color[2] = 1.0f;
                color[3] = 1.0f;
                CG_DebugLine(start, end, color, 0, 1000);
                start[0] = ps->mountPos[0];
                start[1] = ps->mountPos[1];
                start[2] = ps->mountPos[2];
                end[0] = (float)(20.0 * up[0]) + start[0];
                end[1] = (float)(20.0 * up[1]) + start[1];
                end[2] = (float)(20.0 * up[2]) + start[2];
                color[0] = 0.0f;
                color[1] = 0.0f;
                color[2] = 1.0f;
                color[3] = 1.0f;
                CG_DebugLine(start, end, color, 0, 1000);
                color[0] = 1.0f;
                color[1] = 1.0f;
                color[2] = 0.0f;
                color[3] = 1.0f;
                G_DebugStar(tracedLedgePos, color, 1000);
            }
            if ( (float)(tracedLedgePos[2] - ent->s.lerp.pos.trBase[2]) <= 40.0 )
            {
                if ( (float)(tracedLedgePos[2] - ent->s.lerp.pos.trBase[2]) <= 20.0 )
                {
                    turretEnt = SpawnTurretInternal(scr_const.deployed_turret, tracedLedgePos, weapDef->proneMountedWeapdef);
                    if ( turretEnt && turretEnt->pTurretInfo )
                        turretEnt->pTurretInfo->stance = 2;
                }
                else
                {
                    turretEnt = SpawnTurretInternal(scr_const.deployed_turret, tracedLedgePos, weapDef->crouchMountedWeapdef);
                    if ( turretEnt && turretEnt->pTurretInfo )
                        turretEnt->pTurretInfo->stance = 1;
                }
            }
            else
            {
                turretEnt = SpawnTurretInternal(scr_const.deployed_turret, tracedLedgePos, weapDef->standMountedWeapdef);
            }
            if ( !turretEnt
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon.cpp", 1580, 0, "%s", "turretEnt") )
            {
                __debugbreak();
            }
            if ( turretEnt )
            {
                SetModelInternal(turretEnt, (char *)weapDef->mountedModel->name);
                currentAngles = turretEnt->r.currentAngles;
                turretEnt->r.currentAngles[0] = ledgeAngles[0];
                currentAngles[1] = ledgeAngles[1];
                currentAngles[2] = ledgeAngles[2];
                trBase = turretEnt->s.lerp.apos.trBase;
                turretEnt->s.lerp.apos.trBase[0] = ledgeAngles[0];
                trBase[1] = ledgeAngles[1];
                trBase[2] = ledgeAngles[2];
                turret_use(turretEnt, ent);
            }
        }
    }
}

void __cdecl BreakDownWeapon(gentity_s *ent)
{
    gentity_s *turret; // [esp+0h] [ebp-4h]

    if ( !ent->client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon.cpp", 1596, 0, "%s", "ent->client") )
    {
        __debugbreak();
    }
    if ( !ent->client->lastStand )
    {
        turret = &level.gentities[ent->client->ps.viewlocked_entNum];
        if ( turret->pTurretInfo )
            G_ClientStopUsingTurret(turret);
    }
}

void __cdecl G_UseOffHand(gentity_s *ent)
{
    unsigned __int8 PlayerWeaponModel; // al
    weaponParms wp; // [esp+Ch] [ebp-48h] BYREF

    if ( !ent->client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon.cpp", 1611, 0, "%s", "ent->client") )
    {
        __debugbreak();
    }
    if ( !ent->client->ps.offHandIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon.cpp",
                    1612,
                    0,
                    "%s",
                    "ent->client->ps.offHandIndex != WP_NONE") )
    {
        __debugbreak();
    }
    Weapon_SetWeaponParamsWeapon(&wp, ent->client->ps.offHandIndex);
    if ( wp.weapDef->weapType != WEAPTYPE_GRENADE
        && wp.weapDef->weapType != WEAPTYPE_MINE
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon.cpp",
                    1616,
                    0,
                    "%s",
                    "wp.weapDef->weapType == WEAPTYPE_GRENADE || wp.weapDef->weapType == WEAPTYPE_MINE") )
    {
        __debugbreak();
    }
    G_CalcMuzzlePoints(ent, &wp, 0);
    PlayerWeaponModel = BG_GetPlayerWeaponModel(&ent->client->ps, ent->client->ps.offHandIndex);
    Weapon_Throw_Grenade(ent, ent->client->ps.offHandIndex, PlayerWeaponModel, &wp);
}

void __cdecl FireWeaponMelee(gentity_s *ent, int gametime)
{
    int integer; // [esp+10h] [ebp-58h]
    weaponParms wp; // [esp+18h] [ebp-50h] BYREF
    int weaponIndex; // [esp+64h] [ebp-4h]

    weaponIndex = 0;
    if ( (ent->client->ps.eFlags & 0x300) == 0 || !ent->active )
    {
        if ( BG_IsUseAsMeleeWeapon(ent->s.weapon) )
            weaponIndex = ent->s.weapon;
        else
            weaponIndex = ent->s.lerp.u.player.meleeWeapon;
        Weapon_SetWeaponParamsWeapon(&wp, weaponIndex);
        G_GetPlayerViewOrigin(&ent->client->ps, wp.muzzleTrace);
        BG_GetPlayerViewDirection(&ent->client->ps, wp.forward, wp.right, wp.up);
        if ( BG_IsBayonetWeapon(weaponIndex) )
            integer = player_bayonetRange->current.integer;
        else
            integer = player_meleeRange->current.integer;
        if ( !player_meleeRange
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon.cpp", 1646, 0, "%s", "player_meleeRange") )
        {
            __debugbreak();
        }
        if ( !player_bayonetRange
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon.cpp",
                        1647,
                        0,
                        "%s",
                        "player_bayonetRange") )
        {
            __debugbreak();
        }
        if ( !player_meleeWidth
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon.cpp", 1648, 0, "%s", "player_meleeWidth") )
        {
            __debugbreak();
        }
        if ( !player_meleeHeight
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon.cpp", 1649, 0, "%s", "player_meleeHeight") )
        {
            __debugbreak();
        }
        Weapon_Melee(
            ent,
            &wp,
            *(float *)&integer,
            player_meleeWidth->current.value,
            player_meleeHeight->current.value,
            gametime);
    }
}

int __cdecl G_GivePlayerWeapon(
                playerState_s *pPS,
                int iWeaponIndex,
                unsigned __int8 altModelIndex,
                renderOptions_s weaponOptions)
{
    const WeaponVariantDef *WeaponVariantDef; // eax
    const char *v6; // eax
    int iCurrIndex; // [esp+48h] [ebp-14h]
    int newOffHandIndex; // [esp+4Ch] [ebp-10h]
    const WeaponDef *oldWeapDef; // [esp+50h] [ebp-Ch]
    const WeaponDef *weapDef; // [esp+54h] [ebp-8h]
    PlayerHeldWeapon *weaponState; // [esp+58h] [ebp-4h]
    PlayerHeldWeapon *weaponStatea; // [esp+58h] [ebp-4h]

    if ( !pPS && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon.cpp", 1663, 0, "%s", "pPS") )
        __debugbreak();
    if ( BG_PlayerHasWeapon(pPS, iWeaponIndex) )
        return 0;
    weapDef = BG_GetWeaponDef(iWeaponIndex);
    if ( weapDef->weapClass == WEAPCLASS_TURRET )
        return 0;
    if ( weapDef->weapClass == WEAPCLASS_NON_PLAYER )
        return 0;
    if ( !weapDef->gunXModel[altModelIndex] )
        return 0;
    if ( level.clientIsSpawning )
    {
        WeaponVariantDef = BG_GetWeaponVariantDef(iWeaponIndex);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon.cpp",
                        1678,
                        0,
                        "%s\n\t(BG_GetWeaponVariantDef(iWeaponIndex)->szDisplayName) = %s",
                        "(!level.clientIsSpawning)",
                        WeaponVariantDef->szDisplayName) )
            __debugbreak();
    }
    if ( BG_HoldWeapon(pPS, iWeaponIndex) )
    {
        BG_PlayerWeaponSetNeedsRechamber(pPS, iWeaponIndex, 0);
        BG_PlayerSetEverHadWeapon(pPS, iWeaponIndex, 0);
        weaponState = BG_GetHeldWeapon(pPS, iWeaponIndex);
        if ( !weaponState
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon.cpp", 1690, 0, "%s", "weaponState") )
        {
            __debugbreak();
        }
        weaponState->model = altModelIndex;
        //renderOptions_s::CopyWeaponOptions(&weaponState->options, &weaponOptions);
        weaponState->options.CopyWeaponOptions(&weaponOptions);
        if ( weapDef->weapClass == WEAPCLASS_ITEM )
        {
            return 1;
        }
        else
        {
            if ( weapDef->weapClass == WEAPCLASS_MELEE )
                pPS->meleeWeapon = iWeaponIndex;
            if ( weapDef->offhandClass )
            {
                if ( pPS->offHandIndex )
                {
                    if ( BG_WeaponAmmo(pPS, pPS->offHandIndex) <= 0 )
                    {
                        oldWeapDef = BG_GetWeaponDef(pPS->offHandIndex);
                        if ( !oldWeapDef
                            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon.cpp", 1720, 0, "%s", "oldWeapDef") )
                        {
                            __debugbreak();
                        }
                        newOffHandIndex = BG_GetFirstAvailableOffhand(pPS, oldWeapDef->offhandSlot);
                        if ( newOffHandIndex )
                            pPS->offHandIndex = newOffHandIndex;
                        else
                            pPS->offHandIndex = iWeaponIndex;
                        G_SetEquippedOffHand(pPS->clientNum, pPS->offHandIndex);
                    }
                }
                else
                {
                    pPS->offHandIndex = iWeaponIndex;
                    G_SetEquippedOffHand(pPS->clientNum, pPS->offHandIndex);
                }
                return 1;
            }
            else
            {
                for ( iCurrIndex = BG_GetWeaponVariantDef(iWeaponIndex)->altWeaponIndex;
                            iCurrIndex && !BG_PlayerHasWeapon(pPS, iCurrIndex);
                            iCurrIndex = BG_GetWeaponVariantDef(iCurrIndex)->altWeaponIndex )
                {
                    if ( !BG_HoldWeapon(pPS, iCurrIndex) )
                        goto LABEL_16;
                    BG_PlayerWeaponSetNeedsRechamber(pPS, iWeaponIndex, 0);
                    BG_PlayerSetEverHadWeapon(pPS, iWeaponIndex, 0);
                    weaponStatea = BG_GetHeldWeapon(pPS, iCurrIndex);
                    if ( !weaponStatea
                        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon.cpp", 1747, 0, "%s", "weaponState") )
                    {
                        __debugbreak();
                    }
                    weaponStatea->model = altModelIndex;
                    //renderOptions_s::CopyWeaponOptions(&weaponStatea->options, &weaponOptions);
                    weaponStatea->options.CopyWeaponOptions(&weaponOptions);
                }
                return 1;
            }
        }
    }
    else
    {
LABEL_16:
        v6 = BG_WeaponName(iWeaponIndex);
        Com_PrintWarning(
            17,
            "WARNING: Trying to give the player a weapon %s when they have no held weapon slots free.\n",
            v6);
        return 0;
    }
}

void __cdecl G_SetupWeaponDef()
{
    Com_DPrintf(17, "----------------------\n");
    Com_DPrintf(17, "Game: G_SetupWeaponDef\n");
    if ( !bg_lastParsedWeaponIndex )
    {
        Com_SetWeaponInfoMemory(1);
        ClearRegisteredItems();
        BG_ClearWeaponDef();
        G_GetWeaponIndexForName((char*)"defaultweapon_mp");
        BG_LoadWeaponTable("_mp", G_RegisterWeapon);
    }
    Com_DPrintf(17, "----------------------\n");
}

int __cdecl G_GetWeaponIndexForName(char *name)
{
    if ( level.initializing )
        return BG_GetWeaponIndexForName(name, G_RegisterWeapon);
    else
        return BG_FindWeaponIndexForName(name);
}

void __cdecl G_SelectWeaponIndex(int clientNum, int iWeaponIndex)
{
    const char *v2; // eax

    v2 = va("%c %i", 97, iWeaponIndex);
    SV_GameSendServerCommand(clientNum, SV_CMD_RELIABLE, v2);
}

void __cdecl G_SetEquippedOffHand(int clientNum, unsigned int offHandIndex)
{
    const char *v2; // eax

    BG_AssertOffhandIndexOrNone(offHandIndex);
    v2 = va("%c %i", 67, offHandIndex);
    SV_GameSendServerCommand(clientNum, SV_CMD_RELIABLE, v2);
}

unsigned int __cdecl G_PlayerTurretWeaponIdx(const playerState_s *ps)
{
    if ( (ps->eFlags & 0x300) == 0 )
        return 0;
    if ( ps->viewlocked == PLAYERVIEWLOCK_NONE )
        return 0;
    if ( ps->viewlocked_entNum == 1023
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon.cpp",
                    1812,
                    0,
                    "%s",
                    "ps->viewlocked_entNum != ENTITYNUM_NONE") )
    {
        __debugbreak();
    }
    return g_entities[ps->viewlocked_entNum].s.weapon;
}

int __cdecl G_GetPlayerWeapon(const playerState_s *ps, int localClientNum)
{
    int weapon; // [esp+0h] [ebp-4h]

    weapon = 0;
    if ( BG_UsingVehicleWeapon(ps) )
    {
        weapon = G_GetPlayerVehicleWeapon(ps);
    }
    else if ( (ps->eFlags & 0x300) != 0 )
    {
        weapon = G_PlayerTurretWeaponIdx(ps);
    }
    if ( !weapon )
        return BG_GetViewmodelWeaponIndex(ps);
    return weapon;
}

int __cdecl G_GetPlayerVehicleWeapon(const playerState_s *ps)
{
    const vehicle_info_t *info; // [esp+4h] [ebp-Ch]
    gentity_s *ent; // [esp+8h] [ebp-8h]
    int seatIndex; // [esp+Ch] [ebp-4h]

    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_weapon.cpp", 1820, 0, "%s", "ps") )
        __debugbreak();
    if ( (ps->eFlags & 0x4000) == 0 )
        return 0;
    ent = &g_entities[ps->viewlocked_entNum];
    seatIndex = ps->vehiclePos;
    if ( !ent->scr_vehicle )
        return 0;
    info = BG_GetVehicleInfo(ent->scr_vehicle->infoIdx);
    if ( seatIndex >= 1 && seatIndex <= 4 )
        return *(unsigned __int16 *)&info->gunnerWeapon[3][2 * seatIndex + 62];
    if ( seatIndex )
        return 0;
    if ( ent->s.weapon || info->driverControlledGunPos < 0 )
        return ent->s.weapon;
    return info->gunnerWeaponIndex[info->driverControlledGunPos];
}

