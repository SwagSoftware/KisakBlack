#include "bullet.h"
#include <cgame/cg_world.h>
#include <bgame/bg_misc.h>
#include "game_public.h"
#include "g_weapon.h"
#include <server_mp/sv_init_mp.h>
#include <game_mp/g_main_mp.h>
#include <cgame/cg_drawtools.h>
#include <game_mp/g_team_mp.h>
#include <server/sv_world.h>
#include <game_mp/g_trigger_mp.h>
#include <DynEntity/DynEntity_server.h>
#include <game_mp/g_combat_mp.h>
#include <client/cl_debugdata.h>
#include <game_mp/g_utils_mp.h>
#include <server/sv_game.h>
#include <glass/glass_server.h>
#include "actor_events.h"
#include <bgame/bg_perks.h>
#include <cgame/cg_weapons.h>
#include <cgame_mp/cg_local_mp.h>

unsigned __int8 scr_playerdamage_boneindex = 254u;

char __cdecl BulletTrace(
                int localClientNum,
                const BulletFireParams *bp,
                const WeaponDef *weapDef,
                const centity_s *attacker,
                BulletTraceResults *br,
                unsigned int lastSurfaceType)
{
    centity_s *Entity; // [esp+Ch] [ebp-2Ch]
    unsigned __int16 hitEntId; // [esp+34h] [ebp-4h]

    PROF_SCOPED("BulletTrace");

    if ( !bp && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 2684, 0, "%s", "bp") )
        __debugbreak();
    if ( !weapDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 2685, 0, "%s", "weapDef") )
    {
        __debugbreak();
    }
    if ( !attacker
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 2686, 0, "%s", "attacker") )
    {
        __debugbreak();
    }
    if ( !br && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 2687, 0, "%s", "br") )
        __debugbreak();
    if ( lastSurfaceType >= 0x1F
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                    2688,
                    0,
                    "lastSurfaceType doesn't index SURF_TYPECOUNT\n\t%i not in [0, %i)",
                    lastSurfaceType,
                    31) )
    {
        __debugbreak();
    }
    Com_Memset(br, 0, sizeof(BulletTraceResults));
    CG_LocationalTrace(
        &br->trace,
        bp->start,
        bp->end,
        bp->ignoreEntIndex,
        0x280E833,
        attacker->nextState.eType == 1,
        0);
    if ( br->trace.hitType )
    {
        hitEntId = Trace_GetEntityHitId(&br->trace);
        if ( hitEntId == 1022 )
            Entity = 0;
        else
            Entity = CG_GetEntity(localClientNum, hitEntId);
        Vec3Lerp(bp->start, bp->end, br->trace.fraction, br->hitPos);
        if ( Entity )
        {
            if ( !br->trace.sflags )
            {
                switch ( Entity->nextState.eType )
                {
                    case 1:
                    case 2:
                    case 0x11:
                    case 0x13:
                        br->trace.sflags = 0x700000;
                        break;
                    case 6:
                        if ( Entity->nextState.surfType == 7 )
                            br->trace.sflags = 0x700000;
                        break;
                    default:
                        break;
                }
            }
            br->ignoreHitEnt = ShouldIgnoreHitEntity(attacker->nextState.number, hitEntId);
        }
        br->depthSurfaceType = (unsigned __int8)((int)(0x3F00000
                                                                                                 & br->trace.sflags) >> 20);
        if ( (br->trace.sflags & 0x100) != 0 )
        {
            br->depthSurfaceType = 0;
        }
        else if ( !br->depthSurfaceType && lastSurfaceType )
        {
            br->depthSurfaceType = lastSurfaceType;
        }
        return 1;
    }
    else
    {
        return 0;
    }
}

double __cdecl G_GoodRandomFloat(int *idum)
{
    int j; // [esp+14h] [ebp-88h]
    int iv[32]; // [esp+1Ch] [ebp-80h]

    *idum = -*idum;
    for ( j = 39; j >= 0; --j )
    {
        *idum = 16807 * (*idum % 127773) - 2836 * (*idum / 127773);
        if ( *idum < 0 )
            *idum += 0x7FFFFFFF;
        if ( j < 32 )
            iv[j] = *idum;
    }
    *idum = 16807 * (*idum % 127773) - 2836 * (*idum / 127773);
    if ( *idum < 0 )
        *idum += 0x7FFFFFFF;
    if ( 0.99999988 - (double)iv[iv[0] / 0x4000000] * 4.656612875245797e-10 < 0.0 )
        return (float)0.99999988;
    else
        return (float)((double)iv[iv[0] / 0x4000000] * 4.656612875245797e-10);
}

void __cdecl Bullet_Endpos(int randSeed, float spread, float *end, float *dir, const weaponParms *wp, float maxRange)
{
    long double v6; // [esp+0h] [ebp-94h]
    float v7; // [esp+2Ch] [ebp-68h]
    float v8; // [esp+34h] [ebp-60h]
    float right; // [esp+88h] [ebp-Ch] BYREF
    float aimOffset; // [esp+8Ch] [ebp-8h]
    float up; // [esp+90h] [ebp-4h] BYREF

    if ( (LODWORD(spread) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp", 111, 0, "%s", "!IS_NAN(spread)") )
    {
        __debugbreak();
    }
    if ( !end && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp", 113, 0, "%s", "end") )
        __debugbreak();
    if ( !wp && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp", 114, 0, "%s", "wp") )
        __debugbreak();
    //__libm_sse2_tan(v6);
    //aimOffset = (float)(spread * 0.017453292) * maxRange;
    aimOffset = (float)(tan(DEG2RAD(spread))) * maxRange;
    if ( (LODWORD(aimOffset) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp", 118, 0, "%s", "!IS_NAN(aimOffset)") )
    {
        __debugbreak();
    }
    Bullet_RandomDir(randSeed, &right, &up);
    right = right * aimOffset;
    up = up * aimOffset;
    if ( (LODWORD(right) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp", 125, 0, "%s", "!IS_NAN(right)") )
    {
        __debugbreak();
    }
    if ( (LODWORD(up) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp", 126, 0, "%s", "!IS_NAN(up)") )
    {
        __debugbreak();
    }
    nanassertvec3(wp->muzzleTrace);
    nanassertvec3(wp->forward);
    nanassertvec3(wp->right);
    nanassertvec3(wp->up);

    *end = (float)(maxRange * wp->forward[0]) + wp->muzzleTrace[0];
    end[1] = (float)(maxRange * wp->forward[1]) + wp->muzzleTrace[1];
    end[2] = (float)(maxRange * wp->forward[2]) + wp->muzzleTrace[2];

    nanassertvec3(end);

    v8 = right;
    *end = (float)(right * wp->right[0]) + *end;
    end[1] = (float)(v8 * wp->right[1]) + end[1];
    end[2] = (float)(v8 * wp->right[2]) + end[2];
    v7 = up;
    *end = (float)(up * wp->up[0]) + *end;
    end[1] = (float)(v7 * wp->up[1]) + end[1];
    end[2] = (float)(v7 * wp->up[2]) + end[2];

    nanassertvec3(end);

    if ( dir )
    {
        *dir = *end - wp->muzzleTrace[0];
        dir[1] = end[1] - wp->muzzleTrace[1];
        dir[2] = end[2] - wp->muzzleTrace[2];
        Vec3Normalize(dir);

        nanassertvec3(dir);
    }
}

void __cdecl Bullet_RandomDir(int time, float *x, float *y)
{
    long double v3; // st7
    float sinT; // [esp+Ch] [ebp-10h]
    float theta; // [esp+10h] [ebp-Ch]
    float r; // [esp+14h] [ebp-8h]
    float cosT; // [esp+18h] [ebp-4h]

    if ( !x && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp", 88, 0, "%s", "x") )
        __debugbreak();
    if ( !y && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp", 89, 0, "%s", "y") )
        __debugbreak();
    theta = G_GoodRandomFloat(&time) * 360.0;
    r = G_GoodRandomFloat(&time);
    v3 = (float)(DEG2RAD(theta));
    cosT = cos(v3);
    sinT = sin(v3);
    *x = r * cosT;
    *y = r * sinT;
}

bool __cdecl BG_WeaponBulletFire_ExplodeOnImpact(const WeaponDef *weapDef)
{
    if ( !weapDef && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp", 870, 0, "%s", "weapDef") )
        __debugbreak();
    if ( weapDef->weapType
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp",
                    871,
                    0,
                    "%s",
                    "weapDef->weapType == WEAPTYPE_BULLET") )
    {
        __debugbreak();
    }
    return weapDef->bBulletImpactExplode || bg_forceExplosiveBullets->current.enabled;
}

bool __cdecl BG_WeaponBulletFire_ShouldPenetrate(const WeaponDef *weapDef)
{
    if ( !weapDef && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp", 881, 0, "%s", "weapDef") )
        __debugbreak();
    if ( weapDef->weapType
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp",
                    882,
                    0,
                    "%s",
                    "weapDef->weapType == WEAPTYPE_BULLET") )
    {
        __debugbreak();
    }
    return weapDef->penetrateType && !BG_WeaponBulletFire_ExplodeOnImpact(weapDef);
}

void __cdecl Bullet_Fire(
                gentity_s *attacker,
                float spread,
                const weaponParms *wp,
                const gentity_s *weaponEnt,
                int gameTime)
{
    int number; // [esp+14h] [ebp-384h]
    int shotCount; // [esp+20h] [ebp-378h]
    float range; // [esp+24h] [ebp-374h]
    AntilagClientStore antilagClients; // [esp+28h] [ebp-370h] BYREF
    BulletFireParams v9; // [esp+350h] [ebp-48h] BYREF
    int shotIndex; // [esp+394h] [ebp-4h]

    if ( !attacker && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp", 903, 0, "%s", "attacker") )
        __debugbreak();
    if ( !wp && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp", 904, 0, "%s", "wp") )
        __debugbreak();
    if ( !wp->weapDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp", 905, 0, "%s", "wp->weapDef") )
    {
        __debugbreak();
    }
    if ( wp->weapDef->weapType
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp",
                    906,
                    0,
                    "%s",
                    "wp->weapDef->weapType == WEAPTYPE_BULLET") )
    {
        __debugbreak();
    }
    G_AntiLagRewindClientPos(gameTime, &antilagClients);
    if ( wp->weapDef->weapClass == WEAPCLASS_SPREAD )
    {
        shotCount = wp->weapDef->shotCount;
        range = wp->weapDef->fMinDamageRange;
    }
    else
    {
        shotCount = 1;
        range = sv_bullet_range->current.value;
    }
    for ( shotIndex = 0; shotIndex < shotCount; ++shotIndex )
    {
        if ( weaponEnt )
            number = weaponEnt->s.number;
        else
            number = 1022;
        v9.weaponEntIndex = number;
        v9.ignoreEntIndex = number;
        if ( vehicle_selfCollision->current.enabled && weaponEnt && weaponEnt->scr_vehicle )
            v9.ignoreEntIndex = 1022;
        v9.damageMultiplier = 1.0f;
        v9.methodOfDeath = wp->weapDef->bRifleBullet + 1;
        v9.origStart[0] = wp->muzzleTrace[0];
        v9.origStart[1] = wp->muzzleTrace[1];
        v9.origStart[2] = wp->muzzleTrace[2];
        v9.start[0] = wp->muzzleTrace[0];
        v9.start[1] = wp->muzzleTrace[1];
        v9.start[2] = wp->muzzleTrace[2];
        Bullet_Endpos(shotIndex + gameTime, spread, v9.end, v9.dir, wp, range);
        if ( g_debugBullets->current.integer == 1 )
            CG_DebugLine(v9.start, v9.end, colorYellow, 1, 1000);
        if ( BG_WeaponBulletFire_ShouldPenetrate(wp->weapDef) )
            Bullet_FirePenetrate(&v9, wp->weapVariantDef, attacker, gameTime);
        else
            Bullet_FireExtended(&v9, wp->weapVariantDef, attacker, gameTime);
    }
    G_AntiLag_RestoreClientPos(&antilagClients);
}

void __cdecl Bullet_FireExtended(
                BulletFireParams *bp,
                const WeaponVariantDef *weapVariantDef,
                gentity_s *attacker,
                int gameTime)
{
    int extIndex; // [esp+4h] [ebp-60h]
    int impactFlags; // [esp+8h] [ebp-5Ch] BYREF
    BulletTraceResults br; // [esp+Ch] [ebp-58h] BYREF
    const WeaponDef *weapDef; // [esp+60h] [ebp-4h]

    if ( !bp && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp", 660, 0, "%s", "bp") )
        __debugbreak();
    if ( !weapVariantDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp", 661, 0, "%s", "weapVariantDef") )
    {
        __debugbreak();
    }
    if ( !attacker && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp", 662, 0, "%s", "attacker") )
        __debugbreak();
    weapDef = weapVariantDef->weapDef;
    for ( extIndex = 0; extIndex < 12 && Bullet_Trace(bp, weapVariantDef, attacker, &br, 0); ++extIndex )
    {
        Bullet_Process(bp, &br, weapVariantDef, attacker, 0, gameTime, &impactFlags, 1);
        if ( (br.trace.cflags & 0x10) != 0 )
        {
            if ( !BG_AdvanceTrace(bp, &br, 0.13500001) )
                return;
        }
        else
        {
            if ( !br.hitEnt || !br.hitEnt->takedamage )
                return;
            if ( br.ignoreHitEnt )
            {
                BG_AdvanceTrace(bp, &br, 0.0);
            }
            else
            {
                if ( !weapDef->bRifleBullet
                    || !br.hitEnt->sentient
                    || !Dvar_GetInt("scr_team_fftype") && OnSameTeam(br.hitEnt, attacker) )
                {
                    return;
                }
                bp->damageMultiplier = bp->damageMultiplier * 0.5;
                BG_AdvanceTrace(bp, &br, 0.0);
            }
        }
    }
}

char __cdecl Bullet_Trace(
                const BulletFireParams *bp,
                const WeaponVariantDef *weapVariantDef,
                gentity_s *attacker,
                BulletTraceResults *br,
                unsigned int lastSurfaceType)
{
    gentity_s *v7; // [esp+Ch] [ebp-14h]
    WeaponDef *weapDef; // [esp+18h] [ebp-8h]
    unsigned __int16 hitEntId; // [esp+1Ch] [ebp-4h]

    if ( !bp && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp", 433, 0, "%s", "bp") )
        __debugbreak();
    if ( !weapVariantDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp", 434, 0, "%s", "weapVariantDef") )
    {
        __debugbreak();
    }
    if ( !attacker && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp", 435, 0, "%s", "attacker") )
        __debugbreak();
    if ( !br && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp", 436, 0, "%s", "br") )
        __debugbreak();
    if ( lastSurfaceType >= 0x1F
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp",
                    437,
                    0,
                    "lastSurfaceType doesn't index SURF_TYPECOUNT\n\t%i not in [0, %i)",
                    lastSurfaceType,
                    31) )
    {
        __debugbreak();
    }
    weapDef = weapVariantDef->weapDef;
    Com_Memset(br, 0, sizeof(BulletTraceResults));
    if ( weapDef->bRifleBullet )
        G_LocationalTraceAllowChildren(&br->trace, bp->start, bp->end, bp->ignoreEntIndex, 0x280E833, riflePriorityMap);
    else
        G_LocationalTraceAllowChildren(&br->trace, bp->start, bp->end, bp->ignoreEntIndex, 0x280E833, bulletPriorityMap);
    if ( br->trace.hitType == TRACE_HITTYPE_NONE && (br->trace.sflags & 4) == 0 )
        return 0;
    hitEntId = Trace_GetEntityHitId(&br->trace);
    if ( hitEntId == 1022 )
        v7 = 0;
    else
        v7 = &g_entities[hitEntId];
    br->hitEnt = v7;
    if ( (br->trace.sflags & 4) != 0 )
        br->hitEnt = &g_entities[hitEntId];
    Vec3Lerp(bp->start, bp->end, br->trace.fraction, br->hitPos);
    if ( br->hitEnt )
    {
        if ( (br->hitEnt->s.eType == 1 || br->hitEnt->s.eType == 2 || br->hitEnt->s.eType == 17 || br->hitEnt->s.eType == 19)
            && !br->trace.sflags )
        {
            br->trace.sflags = 0x700000;
        }
        br->ignoreHitEnt = Bullet_IgnoreHitEntity(bp, br, attacker);
    }
    br->depthSurfaceType = (unsigned __int8)((int)(0x3F00000
                                                                                             & br->trace.sflags) >> 20);
    if ( (br->trace.sflags & 0x100) != 0 )
    {
        br->depthSurfaceType = 0;
    }
    else if ( !br->depthSurfaceType )
    {
        if ( lastSurfaceType )
            br->depthSurfaceType = lastSurfaceType;
    }
    return 1;
}

bool __cdecl Bullet_IgnoreHitEntity(const BulletFireParams *bp, const BulletTraceResults *br, gentity_s *attacker)
{
    if ( !bp && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp", 382, 0, "%s", "bp") )
        __debugbreak();
    if ( !br && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp", 383, 0, "%s", "br") )
        __debugbreak();
    if ( !br->hitEnt
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp", 384, 0, "%s", "br->hitEnt") )
    {
        __debugbreak();
    }
    if ( !attacker && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp", 385, 0, "%s", "attacker") )
        __debugbreak();
    return br->hitEnt == attacker;
}

void __cdecl Bullet_Process(
                BulletFireParams *bp,
                BulletTraceResults *br,
                const WeaponVariantDef *weapVariantDef,
                gentity_s *attacker,
                int dFlags,
                int gameTime,
                int *outImpactFlags,
                bool processFx)
{
    double v8; // xmm0_8
    float v9; // xmm0_4
    unsigned int WeaponIndex; // eax
    hitLocation_t v11; // [esp+10h] [ebp-70h]
    unsigned int modelIndex; // [esp+14h] [ebp-6Ch]
    unsigned int partName; // [esp+18h] [ebp-68h]
    int v14; // [esp+1Ch] [ebp-64h]
    long double v15; // [esp+20h] [ebp-60h]
    bool v16; // [esp+20h] [ebp-60h]
    float v17; // [esp+24h] [ebp-5Ch]
    int partBits[5]; // [esp+30h] [ebp-50h] BYREF
    bool targetIsAlive; // [esp+46h] [ebp-3Ah]
    bool targetWasAlive; // [esp+47h] [ebp-39h]
    gentity_s *useEnt; // [esp+48h] [ebp-38h]
    hitLocation_t hitLoc; // [esp+4Ch] [ebp-34h]
    int weapon; // [esp+50h] [ebp-30h]
    float coneCos; // [esp+54h] [ebp-2Ch]
    int damage; // [esp+58h] [ebp-28h]
    gentity_s *inflictorEnt; // [esp+5Ch] [ebp-24h]
    DynEntityDrawType drawType; // [esp+60h] [ebp-20h] BYREF
    gentity_s *bulletEffectTempEnt; // [esp+64h] [ebp-1Ch] BYREF
    const WeaponDef *weapDef; // [esp+68h] [ebp-18h]
    float color[4]; // [esp+6Ch] [ebp-14h] BYREF
    unsigned __int16 hitEntId; // [esp+7Ch] [ebp-4h]

    if ( !bp && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp", 494, 0, "%s", "bp") )
        __debugbreak();
    if ( !br && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp", 495, 0, "%s", "br") )
        __debugbreak();
    if ( br->trace.hitType == TRACE_HITTYPE_NONE
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp",
                    496,
                    0,
                    "%s",
                    "br->trace.hitType != TRACE_HITTYPE_NONE") )
    {
        __debugbreak();
    }
    if ( !weapVariantDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp", 497, 0, "%s", "weapVariantDef") )
    {
        __debugbreak();
    }
    if ( !attacker && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp", 498, 0, "%s", "attacker") )
        __debugbreak();
    weapDef = weapVariantDef->weapDef;
    *outImpactFlags = 0;
    if ( attacker->sentient )
        Bullet_NofifyActor(bp, attacker, bp->start, br->hitPos, br->hitEnt, weapVariantDef);
    color[0] = 1.0f;
    color[1] = 0.0f;
    color[2] = 0.0f;
    color[3] = 1.0f;
    if ( g_debugBullets->current.integer == 2 )
    {
        CG_DebugLine(bp->start, br->hitPos, color, 1, 1000);
    }
    else if ( g_debugBullets->current.integer == 1 )
    {
        CG_DebugLine(bp->start, br->hitPos, color, 1, 100);
    }
    damage = Bullet_GetDamage(bp, br, weapDef, attacker);
    G_CheckHitTriggerDamage(attacker, bp->start, br->hitPos, damage, bp->methodOfDeath);
    bulletEffectTempEnt = 0;
    if ( processFx )
        Bullet_ImpactEffect(bp, br, br->trace.normal.vec.v, weapVariantDef, attacker, 0, &bulletEffectTempEnt);
    hitEntId = Trace_GetDynEntHitId(&br->trace, &drawType);
    if ( hitEntId != 0xFFFF )
        DynEntSv_Damage(hitEntId, drawType, br->hitPos, bp->dir, damage, 0);
    //BLOPS_NULLSUB();
    if ( weapDef->bBulletImpactExplode && weapDef->iExplosionInnerDamage )
    {
        v8 = (float)(DEG2RAD(weapDef->damageConeAngle));
        //__libm_sse2_cos(v15);
        //v9 = v8;
        v9 = cos(v8);
        if ( (float)(weapDef->damageConeAngle - 180.0) < 0.0 )
            v17 = v9;
        else
            v17 = -1.0f;
        coneCos = v17;
        weapon = BG_GetWeaponIndex(weapVariantDef);
        inflictorEnt = attacker;
        if ( bp->weaponEntIndex != 1022 || bp->weaponEntIndex != 1023 )
            inflictorEnt = &g_entities[bp->weaponEntIndex];
        G_RadiusDamage(
            br->hitPos,
            inflictorEnt,
            attacker,
            (float)weapDef->iExplosionInnerDamage,
            (float)weapDef->iExplosionOuterDamage,
            (float)weapDef->iExplosionRadius,
            coneCos,
            bp->dir,
            0,
            6,
            weapon);
    }
    if ( br->hitEnt && br->hitEnt->takedamage && !br->ignoreHitEnt )
    {
        if ( weapDef->armorPiercing )
            dFlags |= 2u;
        if ( !br->hitEnt->r.inuse
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp", 558, 0, "%s", "br->hitEnt->r.inuse") )
        {
            __debugbreak();
        }
        targetWasAlive = br->hitEnt->health > 0;
        hitLoc = (hitLocation_t)br->trace.partGroup;
        if ( hitLoc == HITLOC_HEAD || hitLoc == HITLOC_HELMET )
            *outImpactFlags |= 1u;
        if ( g_debugLocDamage->current.integer > 0 )
            CL_AddDebugStar(br->hitPos, colorRed, 100);
        if ( g_debugLocHit->current.integer > 0 )
        {
            memset(partBits, 255, sizeof(partBits));
            G_DObjCalcPose(br->hitEnt, partBits);
            SV_XModelDebugBoxes(br->hitEnt, colorYellow, 0, g_debugLocHitTime->current.integer);
            CL_AddDebugLine(bp->start, br->hitPos, colorGreen, 0, g_debugLocHitTime->current.integer);
            CL_AddDebugStar(br->hitPos, colorRed, g_debugLocHitTime->current.integer);
        }
        scr_playerdamage_boneindex = br->trace.boneIndex;
        useEnt = attacker;
        if ( (attacker->s.lerp.eFlags & 0x4300) != 0 && attacker->client && attacker->client->ps.viewlocked_entNum != 1023 )
            useEnt = &g_entities[attacker->client->ps.viewlocked_entNum];
        if ( !useEnt && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp", 591, 0, "%s", "useEnt") )
            __debugbreak();
        v14 = level.time - gameTime;
        partName = br->trace.partName;
        modelIndex = br->trace.modelIndex;
        v11 = hitLoc;
        WeaponIndex = BG_GetWeaponIndex(weapVariantDef);
        G_Damage(
            br->hitEnt,
            useEnt,
            attacker,
            bp->dir,
            br->hitPos,
            damage,
            dFlags,
            bp->methodOfDeath,
            WeaponIndex,
            v11,
            modelIndex,
            partName,
            v14);
        scr_playerdamage_boneindex = -2;
        v16 = br->hitEnt->r.inuse && br->hitEnt->health > 0;
        targetIsAlive = v16;
    }
    if ( br->trace.hitType == TRACE_HITTYPE_GLASS )
        GlassSv_Damage(br->trace.hitId, damage, bp->methodOfDeath, br->hitPos, bp->dir);
    if ( bulletEffectTempEnt )
        Bullet_ImpactEffect_SetAdditionalFlags(bulletEffectTempEnt, *outImpactFlags);
}

int __cdecl Bullet_GetDamage(
                const BulletFireParams *bp,
                const BulletTraceResults *br,
                const WeaponDef *weapDef,
                gentity_s *attacker)
{
    int damage; // [esp+18h] [ebp-18h]
    float lerpAmount; // [esp+1Ch] [ebp-14h]
    int baseDamage; // [esp+20h] [ebp-10h]
    float range; // [esp+24h] [ebp-Ch]
    float dist; // [esp+28h] [ebp-8h]
    int minDamage; // [esp+2Ch] [ebp-4h]

    if ( !bp && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp", 162, 0, "%s", "bp") )
        __debugbreak();
    if ( !br && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp", 163, 0, "%s", "br") )
        __debugbreak();
    if ( br->trace.hitType == TRACE_HITTYPE_NONE
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp",
                    164,
                    0,
                    "%s",
                    "br->trace.hitType != TRACE_HITTYPE_NONE") )
    {
        __debugbreak();
    }
    if ( !weapDef && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp", 165, 0, "%s", "weapDef") )
        __debugbreak();
    if ( !attacker && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp", 166, 0, "%s", "attacker") )
        __debugbreak();
    baseDamage = weapDef->damage;
    minDamage = weapDef->minDamage;
    damage = baseDamage;
    range = weapDef->fMinDamageRange - weapDef->fMaxDamageRange;
    if ( baseDamage != minDamage && range != 0.0 )
    {
        dist = Vec3Distance(bp->origStart, br->hitPos);
        if ( weapDef->fMaxDamageRange <= dist )
        {
            if ( weapDef->fMinDamageRange <= dist )
            {
                damage = minDamage;
            }
            else
            {
                lerpAmount = (float)(dist - weapDef->fMaxDamageRange) / range;
                if ( (lerpAmount < 0.0 || lerpAmount > 1.0)
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp",
                                213,
                                0,
                                "%s\n\t(lerpAmount) = %g",
                                "((lerpAmount >= 0.0) && (lerpAmount <= 1.0))",
                                lerpAmount) )
                {
                    __debugbreak();
                }
                damage = (int)(float)((float)((float)baseDamage * (float)(1.0 - lerpAmount))
                                                        + (float)((float)minDamage * lerpAmount));
            }
        }
        else
        {
            damage = baseDamage;
        }
    }
    return (int)(float)((float)damage * bp->damageMultiplier);
}

void __cdecl Bullet_NofifyActor(
                const BulletFireParams *bp,
                gentity_s *attacker,
                float *start,
                const float *end,
                gentity_s *hitEnt,
                const WeaponVariantDef *weapVariantDef)
{
    int v6; // [esp+4h] [ebp-3Ch]
    char enemyTeam; // [esp+24h] [ebp-1Ch]
    int teamFlags; // [esp+2Ch] [ebp-14h]
    float lineStart[3]; // [esp+30h] [ebp-10h] BYREF
    ai_event_t eType; // [esp+3Ch] [ebp-4h]

    if ( !bp && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp", 241, 0, "%s", "bp") )
        __debugbreak();
    if ( !attacker && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp", 242, 0, "%s", "attacker") )
        __debugbreak();
    if ( !attacker->sentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp", 243, 0, "%s", "attacker->sentient") )
    {
        __debugbreak();
    }
    if ( !start && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp", 244, 0, "%s", "start") )
        __debugbreak();
    if ( !end && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp", 245, 0, "%s", "end") )
        __debugbreak();
    if ( !weapVariantDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp", 246, 0, "%s", "weapVariantDef") )
    {
        __debugbreak();
    }
    enemyTeam = Sentient_EnemyTeam(attacker->sentient->eTeam);
    if ( bp->damageMultiplier == 1.0 )
    {
        eType = (ai_event_t)(weapVariantDef->bSilenced + 11);
        Actor_BroadcastPointEvent(attacker, eType, 1 << enemyTeam, start, 0.0);
    }
    lineStart[0] = *start - *end;
    lineStart[1] = start[1] - end[1];
    lineStart[2] = start[2] - end[2];
    Vec3Normalize(lineStart);
    lineStart[0] = (float)(200.0 * lineStart[0]) + *start;
    lineStart[1] = (float)(200.0 * lineStart[1]) + start[1];
    lineStart[2] = (float)(200.0 * lineStart[2]) + start[2];
    if ( ai_friendlySuppression->current.enabled )
    {
        teamFlags = 6;
    }
    else
    {
        if ( attacker->client )
            v6 = (1 << attacker->sentient->eTeam) | (1 << enemyTeam);
        else
            v6 = 1 << enemyTeam;
        teamFlags = v6;
    }
    Actor_BroadcastLineEvent(attacker, hitEnt, AI_EV_BULLET, teamFlags, lineStart, end, 0.0);
}

team_t __cdecl Sentient_EnemyTeam(team_t eTeam)
{
    team_t EnemyTeam[4]; // [esp+0h] [ebp-10h]

    EnemyTeam[0] = TEAM_FREE;
    EnemyTeam[1] = TEAM_ALLIES;
    EnemyTeam[2] = TEAM_AXIS;
    EnemyTeam[3] = TEAM_FREE;
    if ( (unsigned int)eTeam >= TEAM_NUM_TEAMS
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\game_mp\\../game/sentient.h",
                    184,
                    0,
                    "%s",
                    "eTeam >= 0 && eTeam < TEAM_NUM_TEAMS") )
    {
        __debugbreak();
    }
    return EnemyTeam[eTeam];
}

void __cdecl Bullet_ImpactEffect(
                const BulletFireParams *bp,
                const BulletTraceResults *br,
                const float *normal,
                const WeaponVariantDef *weapVariantDef,
                gentity_s *attacker,
                int impactEffectFlags,
                gentity_s **outTempEnt)
{
    int WeaponIndex; // eax
    bool v8; // [esp+4h] [ebp-2Ch]
    gentity_s *tempEnt; // [esp+18h] [ebp-18h]
    bool createEffect; // [esp+2Bh] [ebp-5h]

    if ( !bp && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp", 281, 0, "%s", "bp") )
        __debugbreak();
    if ( !br && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp", 282, 0, "%s", "br") )
        __debugbreak();
    if ( br->trace.hitType == TRACE_HITTYPE_NONE
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp",
                    283,
                    0,
                    "%s",
                    "br->trace.hitType != TRACE_HITTYPE_NONE") )
    {
        __debugbreak();
    }
    if ( !weapVariantDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp", 284, 0, "%s", "weapVariantDef") )
    {
        __debugbreak();
    }
    if ( !attacker && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp", 285, 0, "%s", "attacker") )
        __debugbreak();
    if ( !outTempEnt
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp", 286, 0, "%s", "outTempEnt") )
    {
        __debugbreak();
    }
    if ( br->ignoreHitEnt )
    {
        *outTempEnt = 0;
    }
    else
    {
        v8 = (br->trace.sflags & 4) == 0 && br->trace.fraction < 1.0;
        createEffect = v8;
        if ( br->hitEnt )
            createEffect = br->hitEnt->sentient == 0 && v8;
        if ( createEffect )
        {
            if ( sv_clientSideBullets->current.enabled )
            {
                tempEnt = 0;
            }
            else
            {
                tempEnt = G_TempEntity(br->hitPos, EV_BULLET_HIT);
                WeaponIndex = BG_GetWeaponIndex(weapVariantDef);
                AssignToSmallerType<unsigned short>(&tempEnt->s.weapon, WeaponIndex);
                tempEnt->s.eventParm = DirToByte(normal);
                //tempEnt->s.un1 = impactEffectFlags;
                tempEnt->s.un1.scale = impactEffectFlags;
                tempEnt->s.surfType = (int)(0x3F00000 & br->trace.sflags) >> 20;
                tempEnt->s.index.brushmodel = LOBYTE(br->trace.boneIndex);
                tempEnt->s.lerp.u.turret.gunAngles[0] = bp->start[0];
                tempEnt->s.lerp.u.turret.gunAngles[1] = bp->start[1];
                tempEnt->s.lerp.u.turret.gunAngles[2] = bp->start[2];
                tempEnt->s.otherEntityNum = bp->weaponEntIndex;
                if ( br->hitEnt )
                    AssignToSmallerType<short>(&tempEnt->s.groundEntityNum, br->hitEnt->s.number);
                else
                    AssignToSmallerType<short>(&tempEnt->s.groundEntityNum, 1023);
                if ( tempEnt->s.otherEntityNum != bp->weaponEntIndex
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp",
                                364,
                                0,
                                "%s",
                                "tempEnt->s.otherEntityNum == bp->weaponEntIndex") )
                {
                    __debugbreak();
                }
            }
            *outTempEnt = tempEnt;
        }
        else
        {
            *outTempEnt = 0;
        }
    }
}

void __cdecl Bullet_ImpactEffect_SetAdditionalFlags(gentity_s *tempEnt, char impactEffectFlags)
{
    if ( !tempEnt && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp", 373, 0, "%s", "tempEnt") )
        __debugbreak();
    tempEnt->s.un1.scale |= impactEffectFlags;
}

void __cdecl Bullet_FirePenetrate(
                BulletFireParams *bp,
                const WeaponVariantDef *weapVariantDef,
                gentity_s *attacker,
                int gameTime)
{
    float v4; // [esp+Ch] [ebp-1ACh]
    float v5; // [esp+10h] [ebp-1A8h]
    float v6; // [esp+20h] [ebp-198h]
    bool v7; // [esp+24h] [ebp-194h]
    float v8; // [esp+34h] [ebp-184h]
    float value; // [esp+44h] [ebp-174h]
    float SurfacePenetrationDepth; // [esp+54h] [ebp-164h]
    float v11; // [esp+58h] [ebp-160h]
    gentity_s *bulletEffectTempEnt; // [esp+94h] [ebp-124h] BYREF
    BulletTraceResults revBr; // [esp+98h] [ebp-120h] BYREF
    float lastHitPos[3]; // [esp+ECh] [ebp-CCh] BYREF
    float depth; // [esp+F8h] [ebp-C0h]
    bool processFx; // [esp+FFh] [ebp-B9h]
    int penetrateIndex; // [esp+100h] [ebp-B8h]
    bool allSolid; // [esp+107h] [ebp-B1h]
    int max_penetrations; // [esp+108h] [ebp-B0h]
    int impactFlags; // [esp+10Ch] [ebp-ACh] BYREF
    BulletFireParams revBp; // [esp+110h] [ebp-A8h] BYREF
    bool revTraceHit; // [esp+157h] [ebp-61h]
    BulletTraceResults br; // [esp+158h] [ebp-60h] BYREF
    float maxDepth; // [esp+1ACh] [ebp-Ch]
    bool traceHit; // [esp+1B3h] [ebp-5h]
    const WeaponDef *weapDef; // [esp+1B4h] [ebp-4h]

    if ( !bp && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp", 735, 0, "%s", "bp") )
        __debugbreak();
    if ( !weapVariantDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp", 736, 0, "%s", "weapVariantDef") )
    {
        __debugbreak();
    }
    if ( !attacker && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp", 737, 0, "%s", "attacker") )
        __debugbreak();
    weapDef = weapVariantDef->weapDef;
    if ( weapDef->penetrateType == PENETRATE_TYPE_NONE
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\bullet.cpp",
                    740,
                    0,
                    "%s",
                    "weapDef->penetrateType != PENETRATE_TYPE_NONE") )
    {
        __debugbreak();
    }
    if ( Bullet_Trace(bp, weapVariantDef, attacker, &br, 0) )
    {
        Bullet_Process(bp, &br, weapVariantDef, attacker, 0, gameTime, &impactFlags, 1);
        if ( !weapDef->bBulletImpactExplode )
        {
            max_penetrations = sv_penetrationCount->current.integer;
            for ( penetrateIndex = 0; penetrateIndex < max_penetrations; ++penetrateIndex )
            {
                maxDepth = BG_GetSurfacePenetrationDepth(weapDef, br.depthSurfaceType);
                if ( (br.trace.sflags & 4) != 0 )
                    maxDepth = 100.0f;
                if ( attacker->client && (attacker->client->ps.perks[0] & 0x20) != 0 )
                    maxDepth = maxDepth * perk_bulletPenetrationMultiplier->current.value;
                if ( maxDepth <= 0.0 )
                    break;
                lastHitPos[0] = br.hitPos[0];
                lastHitPos[1] = br.hitPos[1];
                lastHitPos[2] = br.hitPos[2];
                if ( !BG_AdvanceTrace(bp, &br, 0.13500001) )
                    break;
                traceHit = Bullet_Trace(bp, weapVariantDef, attacker, &br, br.depthSurfaceType);
                Com_Memcpy(&revBp, bp, sizeof(BulletFireParams));
                revBp.dir[0] = -bp->dir[0];
                revBp.dir[1] = -bp->dir[1];
                revBp.dir[2] = -bp->dir[2];
                revBp.start[0] = bp->end[0];
                revBp.start[1] = bp->end[1];
                revBp.start[2] = bp->end[2];
                revBp.end[0] = (float)(0.0099999998 * revBp.dir[0]) + lastHitPos[0];
                revBp.end[1] = (float)(0.0099999998 * revBp.dir[1]) + lastHitPos[1];
                revBp.end[2] = (float)(0.0099999998 * revBp.dir[2]) + lastHitPos[2];
                Com_Memcpy(&revBr, &br, sizeof(BulletTraceResults));
                //revBr.trace.normal.vec.u[0] ^= _mask__NegFloat_;
                //revBr.trace.normal.vec.u[1] ^= _mask__NegFloat_;
                //revBr.trace.normal.vec.u[2] ^= _mask__NegFloat_;
                revBr.trace.normal.vec.v[0] = -revBr.trace.normal.vec.v[0];
                revBr.trace.normal.vec.v[1] = -revBr.trace.normal.vec.v[1];
                revBr.trace.normal.vec.v[2] = -revBr.trace.normal.vec.v[2];
                if ( traceHit )
                    BG_AdvanceTrace(&revBp, &revBr, 0.0099999998);
                revTraceHit = Bullet_Trace(&revBp, weapVariantDef, attacker, &revBr, revBr.depthSurfaceType);
                v7 = revTraceHit && revBr.trace.allsolid || br.trace.startsolid && revBr.trace.startsolid;
                allSolid = v7;
                if ( (revBr.trace.sflags & 4) != 0 )
                    revBr.hitEnt = &g_entities[revBr.trace.hitId];
                if ( revTraceHit || allSolid )
                {
                    if ( allSolid )
                        v6 = Vec3Distance(revBp.start, revBp.end);
                    else
                        v6 = Vec3Distance(revBr.hitPos, lastHitPos);
                    depth = v6;
                    if ( v6 < 1.0 )
                        depth = 1.0f;
                    if ( revTraceHit )
                    {
                        if ( attacker->client && (attacker->client->ps.perks[0] & 0x20) != 0 )
                        {
                            v11 = BG_GetSurfacePenetrationDepth(weapDef, revBr.depthSurfaceType)
                                    * perk_bulletPenetrationMultiplier->current.value;
                            if ( (float)(v11 - maxDepth) < 0.0 )
                                v5 = v11;
                            else
                                v5 = maxDepth;
                            maxDepth = v5;
                        }
                        else
                        {
                            SurfacePenetrationDepth = BG_GetSurfacePenetrationDepth(weapDef, revBr.depthSurfaceType);
                            if ( (float)(SurfacePenetrationDepth - maxDepth) < 0.0 )
                                v4 = SurfacePenetrationDepth;
                            else
                                v4 = maxDepth;
                            maxDepth = v4;
                        }
                        if ( (revBr.trace.sflags & 4) != 0 )
                            maxDepth = 100.0f;
                        if ( maxDepth <= 0.0 )
                            return;
                    }
                    bp->damageMultiplier = bp->damageMultiplier - (float)(depth / maxDepth);
                    if ( bp->damageMultiplier <= 0.0 )
                        return;
                    if ( !allSolid )
                    {
                        value = bullet_penetrationMinFxDist->current.value;
                        processFx = Vec3DistanceSq(br.hitPos, revBr.hitPos) > value * value;
                        if ( processFx && (!traceHit || (br.trace.sflags & 4) == 0) )
                            Bullet_ImpactEffect(
                                &revBp,
                                &revBr,
                                bp->dir,
                                weapVariantDef,
                                attacker,
                                impactFlags | 4,
                                &bulletEffectTempEnt);
                        if ( traceHit )
                            Bullet_Process(bp, &br, weapVariantDef, attacker, 8, gameTime, &impactFlags, processFx);
                    }
                }
                else if ( traceHit )
                {
                    v8 = bullet_penetrationMinFxDist->current.value;
                    processFx = Vec3DistanceSq(br.hitPos, lastHitPos) > v8 * v8;
                    Bullet_Process(bp, &br, weapVariantDef, attacker, 8, gameTime, &impactFlags, processFx);
                }
                if ( !traceHit )
                    return;
            }
        }
    }
}

