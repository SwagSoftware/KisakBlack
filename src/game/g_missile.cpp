#include "g_missile.h"
#include <server_mp/sv_main_mp.h>
#include <server/sv_world.h>
#include <game_mp/g_main_mp.h>
#include <clientscript/scr_const.h>
#include <game_mp/g_utils_mp.h>
#include <bgame/bg_misc.h>
#include <universal/surfaceflags.h>
#include <game_mp/g_combat_mp.h>
#include "actor_script_cmd.h"
#include <game_mp/g_spawn_mp.h>
#include <clientscript/cscr_vm.h>
#include <game_mp/g_trigger_mp.h>
#include "g_debug.h"
#include <cstring>
#include <glass/glass_server.h>
#include "actor_events.h"
#include "g_weapon.h"
#include <DynEntity/DynEntity_client.h>
#include <universal/com_math_anglevectors.h>
#include <clientscript/cscr_stringlist.h>
#include <server/sv_game.h>
#include <cgame/cg_drawtools.h>
#include <client_mp/g_client_mp.h>
#include <xanim/xmodel_utils.h>
#include <qcommon/dobj_management.h>
#include "turret.h"
#include "g_targets.h"
#include <bgame/bg_weapons_def.h>

static float rollRadius = 5.0f;
float rollFrac_0 = 0.5f;
float grenadeRollThreshold = 30.0f;
const float up[3] = { 0.0, 0.0, 1.0 };



struct //$60CB2C8C864AAD4BAC72BAB25E42491F // sizeof=0x380
{                                       // XREF: .data:attrGlob/r
    AttractorRepulsor_t attractors[32]; // XREF: Missile_FreeAttractorRefs(gentity_s *)+22/r
} attrGlob;

const dvar_t *missileHellfireMaxSlope;
const dvar_t *missileHellfireUpAccel;

const dvar_t *missileJavClimbHeightDirect;
const dvar_t *missileJavClimbHeightTop;
const dvar_t *missileJavClimbAngleDirect;
const dvar_t *missileJavClimbAngleTop;
const dvar_t *missileJavClimbCeilingDirect;
const dvar_t *missileJavClimbCeilingTop;
const dvar_t *missileJavTurnRateDirect;
const dvar_t *missileJavTurnRateTop;
const dvar_t *missileJavAccelClimb;
const dvar_t *missileJavAccelDescend;
const dvar_t *missileJavSpeedLimitClimb;
const dvar_t *missileJavSpeedLimitDescend;
const dvar_t *missileJavTurnDecel;
const dvar_t *missileJavClimbToOwner;

const dvar_t *missileWaterMaxDepth;

const dvar_t *missileTVGuidedStickDeadzone;
const dvar_t *missileTVGuidedTurnRate;
const dvar_t *missileTVGuidedBoost;
const dvar_t *missileTVGuidedBoostSpeedUp;
const dvar_t *missileTVGuidedBoostSpeedDown;
const dvar_t *missileTVGuidedMaxRollAngle;
const dvar_t *missileTVGuidedTurningRollAccel;
const dvar_t *missileTVGuidedFlatteningRollAccel;
const dvar_t *missileTVGuidedMPSpecific;

const dvar_t *missilePlantableSize;
const dvar_t *missileMolotovBlobNum;
const dvar_t *missileMolotovBlobTime;

const dvar_t *grenadeRestThreshold;
const dvar_t *grenadeRollingEnabled;
const dvar_t *grenadeFrictionLow;
const dvar_t *grenadeFrictionHigh;
const dvar_t *grenadeFrictionThresh;   // (registered as "grenadeFrictionMaxThresh")
const dvar_t *grenadeBumpFreq;
const dvar_t *grenadeBumpMag;
const dvar_t *grenadeBumpMax;
const dvar_t *grenadeWobbleFreq;
const dvar_t *grenadeWobbleFwdMag;
const dvar_t *grenadeWobbleSideMag;
const dvar_t *grenadeWobbleSideDamp;
const dvar_t *grenadeCurveMax;
const dvar_t *grenadeBounceRestitutionMax;

const dvar_t *missileDebugDraw;
const dvar_t *missileDebugText;
const dvar_t *missileDebugAttractors;


void __cdecl G_RegisterMissileDvars()
{
    missileHellfireMaxSlope = _Dvar_RegisterFloat(
                                                            "missileHellfireMaxSlope",
                                                            0.30000001,
                                                            0.0,
                                                            3.4028235e38,
                                                            0x80u,
                                                            "This limits how steeply the hellfire missile can turn upward when climbing");
    missileHellfireUpAccel = _Dvar_RegisterFloat(
                                                         "missileHellfireUpAccel",
                                                         20.0,
                                                         0.1,
                                                         3.4028235e38,
                                                         0x80u,
                                                         "The rate at which the hellfire missile curves upward");
    missileJavClimbHeightDirect = _Dvar_RegisterFloat(
                                                                    "missileJavClimbHeightDirect",
                                                                    10000.0,
                                                                    0.0,
                                                                    3.4028235e38,
                                                                    0x80u,
                                                                    "In direct-fire mode, how far above the target the rocket will aim for when climbing.");
    missileJavClimbHeightTop = _Dvar_RegisterFloat(
                                                             "missileJavClimbHeightTop",
                                                             15000.0,
                                                             0.0,
                                                             3.4028235e38,
                                                             0x80u,
                                                             "In top-fire mode, how far above the target the rocket will aim for when climbing.");
    missileJavClimbAngleDirect = _Dvar_RegisterFloat(
                                                                 "missileJavClimbAngleDirect",
                                                                 85.0,
                                                                 0.0,
                                                                 3.4028235e38,
                                                                 0x80u,
                                                                 "In direct-fire mode, the minimum angle between the rocket and target until the rocket s"
                                                                 "tops climbing.    Smaller angles make for higher climbs.");
    missileJavClimbAngleTop = _Dvar_RegisterFloat(
                                                            "missileJavClimbAngleTop",
                                                            50.0,
                                                            0.0,
                                                            3.4028235e38,
                                                            0x80u,
                                                            "In top-fire mode, the minimum angle between the rocket and target until the rocket stops c"
                                                            "limbing.    Smaller angles make for higher climbs.");
    missileJavClimbCeilingDirect = _Dvar_RegisterFloat(
                                                                     "missileJavClimbCeilingDirect",
                                                                     0.0,
                                                                     0.0,
                                                                     3.4028235e38,
                                                                     0x80u,
                                                                     "In direct-fire mode, how high the missile needs to reach before it descends.");
    missileJavClimbCeilingTop = _Dvar_RegisterFloat(
                                                                "missileJavClimbCeilingTop",
                                                                3000.0,
                                                                0.0,
                                                                3.4028235e38,
                                                                0x80u,
                                                                "In top-fire mode, how high the missile needs to reach before it descends.");
    missileJavTurnRateDirect = _Dvar_RegisterFloat(
                                                             "missileJavTurnRateDirect",
                                                             60.0,
                                                             0.0,
                                                             3.4028235e38,
                                                             0x80u,
                                                             "In direct-fire mode, how sharp the rocket can turn, in angles/sec.");
    missileJavTurnRateTop = _Dvar_RegisterFloat(
                                                        "missileJavTurnRateTop",
                                                        100.0,
                                                        0.0,
                                                        3.4028235e38,
                                                        0x80u,
                                                        "In top-fire mode, how sharp the rocket can turn, in angles/sec.");
    missileJavAccelClimb = _Dvar_RegisterFloat(
                                                     "missileJavAccelClimb",
                                                     300.0,
                                                     0.0,
                                                     3.4028235e38,
                                                     0x80u,
                                                     "Rocket acceleration when climbing.");
    missileJavAccelDescend = _Dvar_RegisterFloat(
                                                         "missileJavAccelDescend",
                                                         3000.0,
                                                         0.0,
                                                         3.4028235e38,
                                                         0x80u,
                                                         "Rocket acceleration when descending towards target.");
    missileJavSpeedLimitClimb = _Dvar_RegisterFloat(
                                                                "missileJavSpeedLimitClimb",
                                                                1000.0,
                                                                0.0,
                                                                3.4028235e38,
                                                                0x80u,
                                                                "Rocket's speed limit when climbing.");
    missileJavSpeedLimitDescend = _Dvar_RegisterFloat(
                                                                    "missileJavSpeedLimitDescend",
                                                                    6000.0,
                                                                    0.0,
                                                                    3.4028235e38,
                                                                    0x80u,
                                                                    "Rocket's speed limit when descending towards target.");
    missileJavTurnDecel = _Dvar_RegisterFloat("missileJavTurnDecel", 0.05f, 0.0, 1.0, 0x80u, "");
    missileJavClimbToOwner = _Dvar_RegisterFloat(
                                                         "missileJavClimbToOwner",
                                                         700.0,
                                                         0.0,
                                                         3.4028235e38,
                                                         0x80u,
                                                         "");
    missileWaterMaxDepth = _Dvar_RegisterFloat(
                                                     "missileWaterMaxDepth",
                                                     60.0,
                                                     0.0,
                                                     3.4028235e38,
                                                     0x80u,
                                                     "If a missile explodes deeper under water than this, they explosion effect/sound will not play.");
    missileTVGuidedStickDeadzone = _Dvar_RegisterInt(
                                                                     "missileTVGuidedStickDeadzone",
                                                                     15,
                                                                     -128,
                                                                     127,
                                                                     0x80u,
                                                                     "defines the dead zone for stick movement (stick values range from -128..127");
    missileTVGuidedTurnRate = _Dvar_RegisterFloat(
                                                            "missileTVGuidedTurnRate",
                                                            3.0,
                                                            0.0,
                                                            3.4028235e38,
                                                            0x80u,
                                                            "max turn rate of a tv guided missile (multiplied by the stick deflection [0-1])");
    missilePlantableSize = _Dvar_RegisterFloat(
                                                     "missilePlantableSize",
                                                     5.0,
                                                     0.0,
                                                     3.4028235e38,
                                                     0x80u,
                                                     "Size of the bounding box used for plantable equipment collision.");
    missileMolotovBlobNum = _Dvar_RegisterInt(
                                                        "missileMolotovBlobNum",
                                                        5,
                                                        1,
                                                        10,
                                                        0x80u,
                                                        "The number of fire blobs a molotov should spawn on impact, these count against the server entities.");
    missileMolotovBlobTime = _Dvar_RegisterFloat(
                                                         "missileMolotovBlobTime",
                                                         2.0,
                                                         0.1,
                                                         10.0,
                                                         0x80u,
                                                         "The life time of a molotov fire blob in seconds.");
    missileTVGuidedBoost = _Dvar_RegisterFloat(
                                                     "missileTVGuidedBoost",
                                                     2.0,
                                                     0.1,
                                                     10.0,
                                                     0x80u,
                                                     "The multiplier that the TV guided missile recieves while boosting.");
    missileTVGuidedBoostSpeedUp = _Dvar_RegisterInt(
                                                                    "missileTVGuidedBoostSpeedUp",
                                                                    1000,
                                                                    0,
                                                                    10000,
                                                                    0x80u,
                                                                    "Speed increase factor to get to full boost.");
    missileTVGuidedBoostSpeedDown = _Dvar_RegisterInt(
                                                                        "missileTVGuidedBoostSpeedDown",
                                                                        -1000,
                                                                        -10000,
                                                                        0,
                                                                        0x80u,
                                                                        "Speed decrease factor to return from full boost.");
    missileTVGuidedMaxRollAngle = _Dvar_RegisterFloat(
                                                                    "missileTVGuidedMaxRollAngle",
                                                                    35.0,
                                                                    0.0,
                                                                    3.4028235e38,
                                                                    0x80u,
                                                                    "Max roll the missile will have when turning.");
    missileTVGuidedTurningRollAccel = _Dvar_RegisterFloat(
                                                                            "missileTVGuidedTurningRollAccel",
                                                                            1.5,
                                                                            0.0,
                                                                            3.4028235e38,
                                                                            0x80u,
                                                                            "Roll acceleration when the missile is turning.");
    missileTVGuidedFlatteningRollAccel = _Dvar_RegisterFloat(
                                                                                 "missileTVGuidedFlatteningRollAccel",
                                                                                 2.0,
                                                                                 0.0,
                                                                                 3.4028235e38,
                                                                                 0x80u,
                                                                                 "Roll acceleration when the missile is flattening out after a turn.");
    missileTVGuidedMPSpecific = _Dvar_RegisterBool(
                                                                "missileTVGuidedMPSpecific",
                                                                1,
                                                                0x80u,
                                                                "MP specific Guided Missile functionality");
    grenadeRestThreshold = _Dvar_RegisterFloat(
                                                     "grenadeRestThreshold",
                                                     20.0,
                                                     0.0,
                                                     3.4028235e38,
                                                     0x80u,
                                                     "The speed threshold below which grenades will come to rest");
    grenadeRollingEnabled = _Dvar_RegisterBool(
                                                        "grenadeRollingEnabled",
                                                        1,
                                                        0x1080u,
                                                        "Enables the new \"rolling\" grenade behavior");
    grenadeFrictionLow = _Dvar_RegisterFloat(
                                                 "grenadeFrictionLow",
                                                 0.0099999998,
                                                 0.0,
                                                 1.0,
                                                 0x80u,
                                                 "The amount of friction (0 to 1) for slower/rolling grenades");
    grenadeFrictionHigh = _Dvar_RegisterFloat(
                                                    "grenadeFrictionHigh",
                                                    0.40000001,
                                                    0.0,
                                                    1.0,
                                                    0x80u,
                                                    "The amount of friction (0 to 1) for fast-moving grenades");
    grenadeFrictionThresh = _Dvar_RegisterFloat(
                                                        "grenadeFrictionMaxThresh",
                                                        100.0,
                                                        0.0,
                                                        3.4028235e38,
                                                        0x80u,
                                                        "The speed threshold that determines whether to use grenadeFrictionLow/High");
    grenadeBumpFreq = _Dvar_RegisterFloat(
                                            "grenadeBumpFreq",
                                            0.30000001,
                                            0.0,
                                            3.4028235e38,
                                            0x80u,
                                            "How likely (per server frame) a bump will occur");
    grenadeBumpMag = _Dvar_RegisterFloat(
                                         "grenadeBumpMag",
                                         0.40000001,
                                         0.0,
                                         3.4028235e38,
                                         0x80u,
                                         "Size of bumps (as a fraction of the grenade's current speed)");
    grenadeBumpMax = _Dvar_RegisterFloat(
                                         "grenadeBumpMax",
                                         100.0,
                                         0.0,
                                         3.4028235e38,
                                         0x80u,
                                         "Maximum upward speed of a bump (inches/sec)");
    grenadeWobbleFreq = _Dvar_RegisterFloat(
                                                "grenadeWobbleFreq",
                                                0.079999998,
                                                0.0,
                                                3.4028235e38,
                                                0x80u,
                                                "Wobble cycles per inch of rolling distance (approx)");
    grenadeWobbleFwdMag = _Dvar_RegisterFloat(
                                                    "grenadeWobbleFwdMag",
                                                    10.0,
                                                    0.0,
                                                    3.4028235e38,
                                                    0x80u,
                                                    "The forward rolling speed will oscillate +/- this amount");
    grenadeWobbleSideMag = _Dvar_RegisterFloat(
                                                     "grenadeWobbleSideMag",
                                                     1.0,
                                                     0.0,
                                                     3.4028235e38,
                                                     0x80u,
                                                     "The distance to wobble left and right");
    grenadeWobbleSideDamp = _Dvar_RegisterFloat(
                                                        "grenadeWobbleSideDamp",
                                                        0.0099999998,
                                                        0.0,
                                                        3.4028235e38,
                                                        0x80u,
                                                        "The rate at which the amount of side-to-side wobbling decreases as overall grenade speed increases");
    grenadeCurveMax = _Dvar_RegisterFloat(
                                            "grenadeCurveMax",
                                            4.0,
                                            0.0,
                                            3.4028235e38,
                                            0x80u,
                                            "Largest rolling curvature (will be random between +/- this value)");
    grenadeBounceRestitutionMax = _Dvar_RegisterFloat(
                                                                    "grenadeBounceRestitutionMax",
                                                                    0.30000001,
                                                                    0.0,
                                                                    3.4028235e38,
                                                                    0x80u,
                                                                    "Cap to keep code from increasing bounce restitution too high.");
}

void __cdecl G_RegisterMissileDebugDvars()
{
    missileDebugDraw = _Dvar_RegisterBool("missileDebugDraw", 0, 0, "Draw guided missile trajectories.");
    missileDebugText = _Dvar_RegisterBool("missileDebugText", 0, 0, "Print debug missile info to console.");
    missileDebugAttractors = _Dvar_RegisterBool(
                                                         "missileDebugAttractors",
                                                         0,
                                                         0,
                                                         "Draw the attractors and repulsors.    Attractors are green, and repulsors are yellow.");
}

bool __cdecl collide_missle_entity_filter(int entnum)
{
    return *(__int16 *)((char *)&sv.gentities->s.eType + entnum * sv.gentitySize) != 4;
}

void __cdecl G_MissileTrace(
                trace_t *results,
                const float *start,
                const float *end,
                int passEntityNum,
                int contentmask,
                unsigned int weapon)
{
    float plantableSize; // [esp+Ch] [ebp-54h]
    col_context_t context; // [esp+10h] [ebp-50h] BYREF
    float mins[3]; // [esp+38h] [ebp-28h] BYREF
    float maxs[3]; // [esp+44h] [ebp-1Ch] BYREF
    float dir[3]; // [esp+50h] [ebp-10h] BYREF
    const WeaponDef *weapDef; // [esp+5Ch] [ebp-4h]

    weapDef = BG_GetWeaponDef(weapon);
    if ( !weapDef && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 420, 0, "%s", "weapDef") )
        __debugbreak();
    if ( weapDef->plantable )
    {
        plantableSize = missilePlantableSize->current.value;
        //col_context_t::col_context_t(&context);
        mins[0] = -plantableSize;
        mins[1] = -plantableSize;
        mins[2] = 0.0f;
        maxs[0] = plantableSize;
        maxs[1] = plantableSize;
        maxs[2] = plantableSize * 2.0;
        G_TraceCapsule(results, start, mins, maxs, end, passEntityNum, contentmask, &context);
    }
    else
    {
        G_LocationalTrace(
            results,
            start,
            end,
            passEntityNum,
            contentmask,
            bulletPriorityMap,
            (int (__cdecl *)(int, col_context_t *))collide_missle_entity_filter);
    }
    if ( results->startsolid )
    {
        results->fraction = 0.0f;
        dir[0] = *start - *end;
        dir[1] = start[1] - end[1];
        dir[2] = start[2] - end[2];
        Vec3NormalizeTo(dir, results->normal.vec.v);
    }
}

void __cdecl G_TimedObjectThink(gentity_s *ent)
{
    ent->s.lerp.eFlags &= ~0x10u;
}

// local variable allocation has failed, the output may be wrong!
void    G_ExplodeMissile(gentity_s *ent)
{
    unsigned __int8 v3; // al
    unsigned __int8 v4; // al
    unsigned __int8 v5; // al
    unsigned __int8 v6; // al
    unsigned __int8 v7; // al
    long double v8; // [esp+20h] [ebp-1ACh]
    gentity_s *v9; // [esp+24h] [ebp-1A8h]
    gentity_s *v10; // [esp+28h] [ebp-1A4h]
    gentity_s *v11; // [esp+2Ch] [ebp-1A0h]
    gentity_s *v12; // [esp+30h] [ebp-19Ch]
    float v13[3]; // [esp+34h] [ebp-198h] BYREF
    int splashMethodOfDeath; // [esp+40h] [ebp-18Ch]
    float forwardDir[3]; // [esp+44h] [ebp-188h] BYREF
    float coneCos; // [esp+50h] [ebp-17Ch]
    float v17; // [esp+54h] [ebp-178h]
    float v18; // [esp+58h] [ebp-174h]
    float v19; // [esp+5Ch] [ebp-170h]
    gentity_s *v20; // [esp+60h] [ebp-16Ch]
    int *v21; // [esp+64h] [ebp-168h]
    float endpos[4]; // [esp+68h] [ebp-164h] BYREF
    float impact_normal[4]; // [esp+78h] [ebp-154h] BYREF
    gentity_s *groundEnt; // [esp+C0h] [ebp-10Ch]
    col_context_t explosionPos; // [esp+C4h] [ebp-108h] BYREF
    const float *normal; // [esp+104h] [ebp-C8h]
    gentity_s *eventEnt; // [esp+108h] [ebp-C4h]
    unsigned __int8 surfType; // [esp+10Fh] [ebp-BDh]
    float waterSurfacePos[3]; // [esp+110h] [ebp-BCh] BYREF
    float waterNormal[3]; // [esp+11Ch] [ebp-B0h] BYREF
    float end[4]; // [esp+150h] [ebp-7Ch] BYREF
    bool inWater; // [esp+163h] [ebp-69h]
    BOOL v38; // [esp+164h] [ebp-68h]
    float waterHeight; // [esp+168h] [ebp-64h]
    bool doEvent; // [esp+16Fh] [ebp-5Dh]
    float origin[3]; // [esp+170h] [ebp-5Ch] BYREF
    const WeaponDef *weapDef; // [esp+17Ch] [ebp-50h]
    gentity_s *other; // [esp+180h] [ebp-4Ch]
    trace_t trace; // [esp+184h] [ebp-48h] BYREF
    //_UNKNOWN *v45[2]; // [esp+1C0h] [ebp-Ch] BYREF
    //int v46; // [esp+1C8h] [ebp-4h] BYREF
    //int vars0; // [esp+1CCh] [ebp+0h]
    //
    //v45[0] = a1;
    //v45[1] = (_UNKNOWN *)vars0;
    other = ent;
    iassert(ent);
    iassert(ent->s.weapon);
    weapDef = BG_GetWeaponDef(ent->s.weapon);
    iassert(weapDef);

    if (weapDef->guidedMissileType == MISSILE_GUIDANCE_TVGUIDED)
        G_UnlinkPlayerToRocket(ent);

    if (weapDef->bExplodeOnGround && ent->s.groundEntityNum == 1023 || ForcedDud(ent))
    {
        if (level.time - ent->item[0].index <= 60000)
        {
            ent->nextthink = 50;
        }
        else
        {
            Scr_Notify(ent, scr_const.death, 0);
            G_FreeEntity(ent);
        }
    }
    else
    {
        BG_EvaluateTrajectory(&ent->s.lerp.pos, level.time, origin);
        origin[0] = (float)(int)origin[0];
        origin[1] = (float)(int)origin[1];
        origin[2] = (float)(int)origin[2];
        if (BG_ValidateOrigin(origin, 16, 17, svs.mapCenter))
        {
            G_SetOrigin(ent, origin);
            doEvent = 1;
            waterHeight = CM_GetWaterHeight(ent->r.currentOrigin, 1000.0, -200.0);
            v38 = waterHeight >= ent->r.currentOrigin[2];
            inWater = v38;
            if (!v38)
                inWater = SV_PointContents(ent->r.currentOrigin, -1, 32) != 0;
            if (inWater)
            {
                end[0] = ent->r.currentOrigin[0];
                end[1] = ent->r.currentOrigin[1];
                end[2] = ent->r.currentOrigin[2];
                end[2] = end[2] + missileWaterMaxDepth->current.value;

                col_context_t context; // [esp+128h] [ebp-A4h] BYREF
                //col_context_t::col_context_t(&context);
                G_TraceCapsule(&trace, end, vec3_origin, vec3_origin, ent->r.currentOrigin, ent->s.number, 32, &context);
                if (trace.startsolid || trace.fraction >= 1.0)
                {
                    doEvent = 0;
                }
                else
                {
                    waterNormal[0] = trace.normal.vec.v[0];
                    waterNormal[1] = trace.normal.vec.v[1];
                    waterNormal[2] = trace.normal.vec.v[2];
                    Vec3Lerp(end, ent->r.currentOrigin, trace.fraction, waterSurfacePos);
                }
            }
            ent->s.lerp.eFlags |= 0x20u;
            if (inWater)
                surfType = 20;
            else
                surfType = ent->s.surfType;
            Scr_AddString(Com_SurfaceTypeToName(surfType), SCRIPTINSTANCE_SERVER);
            Scr_AddVector(ent->r.currentOrigin, SCRIPTINSTANCE_SERVER);
            Scr_Notify(ent, scr_const.explode, 2u);
            eventEnt = 0;
            if (doEvent)
            {
                eventEnt = G_Spawn();
                eventEnt->s.eType = ET_GENERAL;
                eventEnt->s.lerp.eFlags |= 0x20u;
                eventEnt->s.weapon = ent->s.weapon;
                eventEnt->s.weaponModel = ent->s.weaponModel;
                eventEnt->r.contents = ent->r.contents;
                G_BroadcastEntity(eventEnt);
                if (inWater)
                {
                    G_SetOrigin(eventEnt, waterSurfacePos);
                    eventEnt->s.surfType = 20;
                    normal = waterNormal;
                }
                else
                {
                    G_SetOrigin(eventEnt, ent->r.currentOrigin);
                    if ((weapDef->stickiness == WEAPSTICKINESS_ALL || weapDef->stickiness == WEAPSTICKINESS_ALL_NO_SENTIENTS)
                        && ent->s.groundEntityNum != 1023)
                    {
                        end[0] = (float)(-16.0 * ent->mover.midTime) + ent->r.currentOrigin[0];
                        end[1] = (float)(-16.0 * ent->mover.aMidTime) + ent->r.currentOrigin[1];
                        end[2] = (float)(-16.0 * ent->trigger.exposureLerpToLighter) + ent->r.currentOrigin[2];
                    }
                    else
                    {
                        end[0] = ent->r.currentOrigin[0];
                        end[1] = ent->r.currentOrigin[1];
                        end[2] = ent->r.currentOrigin[2];
                        end[2] = end[2] - 16.0;
                    }

                    col_context_t context; // [esp+128h] [ebp-A4h] BYREF

                    //col_context_t::col_context_t((col_context_t *)&explosionPos.ignoreEntParams);
                    G_TraceCapsule(
                        &trace,
                        ent->r.currentOrigin,
                        vec3_origin,
                        vec3_origin,
                        end,
                        ent->s.number,
                        2065,
                        &context);
                        //(col_context_t *)&explosionPos.ignoreEntParams);
                    if (weapDef->projExplosionEffectForceNormalUp)
                        normal = up;
                    else
                        normal = (const float *)&trace;
                    eventEnt->s.surfType = (int)(0x3F00000 & trace.sflags) >> 20;
                }
                if (weapDef->projExplosion && weapDef->projExplosion != WEAPPROJEXP_HEAVY)
                {
                    switch (weapDef->projExplosion)
                    {
                    case WEAPPROJEXP_ROCKET:
                        v4 = DirToByte(normal);
                        G_AddEvent(eventEnt, 0x3Au, v4);
                        break;
                    case WEAPPROJEXP_FLASHBANG:
                        v5 = DirToByte(normal);
                        G_AddEvent(eventEnt, 0x3Cu, v5);
                        eventEnt->s.lerp.u.actor.actorNum = level.time;
                        break;
                    case WEAPPROJEXP_FIRE:
                        v6 = DirToByte(normal);
                        G_AddEvent(eventEnt, 0x42u, v6);
                        break;
                    default:
                        v7 = DirToByte(normal);
                        G_AddEvent(eventEnt, 0x3Du, v7);
                        eventEnt->s.lerp.u.actor.actorNum = level.time;
                        break;
                    }
                }
                else
                {
                    if (weapDef->explosionTag && G_DObjGetWorldTagPos(ent, weapDef->explosionTag, (float *)&explosionPos.mask))
                        G_SetOrigin(eventEnt, (const float *)&explosionPos.mask);
                    v3 = DirToByte(normal);
                    G_AddEvent(eventEnt, 0x39u, v3);
                }
                if ((weapDef->projExplosion == WEAPPROJEXP_SMOKE || weapDef->projExplosion == WEAPPROJEXP_FLASHBANG)
                    && weapDef->projExplosionEffect)
                {
                    eventEnt->s.lerp.pos.trBase[0] = (float)(int)eventEnt->s.lerp.pos.trBase[0];
                    eventEnt->s.lerp.pos.trBase[1] = (float)(int)eventEnt->s.lerp.pos.trBase[1];
                    eventEnt->s.lerp.pos.trBase[2] = (float)(int)eventEnt->s.lerp.pos.trBase[2];
                    G_SetOrigin(eventEnt, eventEnt->s.lerp.pos.trBase);
                    eventEnt->s.lerp.eFlags |= 0x4000u;
                    eventEnt->s.lerp.u.actor.actorNum = level.time;
                    eventEnt->s.lerp.u.loopFx.period = ent->missile.grenade.effectIndex;
                    eventEnt->s.time2 = level.time + 61000;
                    eventEnt->s.lerp.eFlags |= 0x10u;
                    eventEnt->handler = 11;
                    eventEnt->nextthink = level.time + 1;
                    Com_Printf(
                        15,
                        "Sending smoke grenade that starts at %i and is at ( %f, %f, %f )\n",
                        level.time,
                        eventEnt->s.lerp.pos.trBase[0],
                        eventEnt->s.lerp.pos.trBase[1],
                        eventEnt->s.lerp.pos.trBase[2]);
                }
                else
                {
                    G_FreeEntityAfterEvent(eventEnt);
                }
            }
            if (ent->s.groundEntityNum != 1023
                && ent->s.groundEntityNum != 1022
                && (weapDef->stickiness == WEAPSTICKINESS_ALL || weapDef->stickiness == WEAPSTICKINESS_ALL_NO_SENTIENTS))
            {
                groundEnt = &g_entities[ent->s.groundEntityNum];
                if (groundEnt->scr_vehicle)
                {
                    trace_t tr; // [esp+88h] [ebp-144h] BYREF
                    //LODWORD(impact_normal[3]) = &ent->mover.midTime;
                    //LODWORD(impact_normal[0]) = ent->item[1].clipAmmoCount ^ _mask__NegFloat_;
                    (impact_normal[0]) = -ent->item[1].clipAmmoCount;
                    //LODWORD(impact_normal[1]) = ent->item[1].index ^ _mask__NegFloat_;
                    (impact_normal[1]) = -ent->item[1].index;
                    //LODWORD(impact_normal[2]) = LODWORD(ent->trigger.exposureLerpToLighter) ^ _mask__NegFloat_;
                    (impact_normal[2]) = -(ent->trigger.exposureLerpToLighter);
                    //LODWORD(endpos[3]) = ent->r.currentOrigin;
                    endpos[0] = ent->r.currentOrigin[0];
                    endpos[1] = ent->r.currentOrigin[1];
                    endpos[2] = ent->r.currentOrigin[2];
                    v21 = &ent->item[1].clipAmmoCount;
                    endpos[0] = (float)(-1.635 * ent->mover.midTime) + endpos[0];
                    endpos[1] = (float)(-1.635 * ent->mover.aMidTime) + endpos[1];
                    endpos[2] = (float)(-1.635 * ent->trigger.exposureLerpToLighter) + endpos[2];
                    G_MissileTrace(
                        &tr,
                        ent->r.currentOrigin,
                        endpos,
                        ent->s.number,
                        ent->clipmask,
                        ent->s.weapon);
                    Vec3Lerp(ent->r.currentOrigin, endpos, tr.fraction, endpos);
                    //if (EntHandle::isDefined(&ent->parent))
                    if (ent->parent.isDefined())
                    {
                        //v20 = EntHandle::ent(&ent->parent);
                        v20 = ent->parent.ent();
                    }
                    else
                        v20 = 0;
                    G_Damage(
                        groundEnt,
                        ent,
                        v20,
                        impact_normal,
                        endpos,
                        weapDef->iExplosionInnerDamage,
                        0,
                        entityHandlers[ent->handler].methodOfDeath,
                        ent->s.weapon,
                        HITLOC_NONE,
                        tr.modelIndex,
                        tr.partName,
                        0);
                    other = groundEnt;
                }
            }
            if (weapDef->iExplosionInnerDamage)
            {
                v19 = weapDef->damageConeAngle * 0.017453292;
                //__libm_sse2_cos(v8);
                v18 = cos(v19);
                if ((float)(weapDef->damageConeAngle - 180.0) < 0.0)
                    v17 = v18;
                else
                    v17 = -1.0f;
                coneCos = v17;
                AngleVectors(ent->r.currentAngles, forwardDir, 0, 0);
                splashMethodOfDeath = GetSplashMethodOfDeath(ent);
                if (weapDef->explosionTag)
                {
                    if (G_DObjGetWorldTagPos(ent, weapDef->explosionTag, v13))
                    {
                        //if (EntHandle::isDefined(&ent->parent))
                        if (ent->parent.isDefined())
                        {
                            //v12 = EntHandle::ent(&ent->parent);
                            v12 = ent->parent.ent();
                        }
                        else
                        {
                            v12 = 0;
                        }
                        G_RadiusDamage(
                            v13,
                            ent,
                            v12,
                            (float)weapDef->iExplosionInnerDamage,
                            (float)weapDef->iExplosionOuterDamage,
                            (float)weapDef->iExplosionRadius,
                            coneCos,
                            forwardDir,
                            other,
                            splashMethodOfDeath,
                            ent->s.weapon);
                    }
                    else
                    {
                        //if (EntHandle::isDefined(&ent->parent))
                        if (ent->parent.isDefined())
                        {
                            //v11 = EntHandle::ent(&ent->parent);
                            v11 = ent->parent.ent();
                        }
                        else
                        {
                            v11 = 0;
                        }
                        G_RadiusDamage(
                            ent->r.currentOrigin,
                            ent,
                            v11,
                            (float)weapDef->iExplosionInnerDamage,
                            (float)weapDef->iExplosionOuterDamage,
                            (float)weapDef->iExplosionRadius,
                            coneCos,
                            forwardDir,
                            other,
                            splashMethodOfDeath,
                            ent->s.weapon);
                    }
                }
                else
                {
                    //if (EntHandle::isDefined(&ent->parent))
                    if (ent->parent.isDefined())
                    {
                        //v10 = EntHandle::ent(&ent->parent);
                        v10 = ent->parent.ent();
                    }
                    else
                    {
                        v10 = 0;
                    }
                    G_RadiusDamage(
                        ent->r.currentOrigin,
                        ent,
                        v10,
                        (float)weapDef->iExplosionInnerDamage,
                        (float)weapDef->iExplosionOuterDamage,
                        (float)weapDef->iExplosionRadius,
                        coneCos,
                        forwardDir,
                        other,
                        splashMethodOfDeath,
                        ent->s.weapon);
                }
            }
            if (weapDef->projExplosion == WEAPPROJEXP_FLASHBANG)
            {
                //if (EntHandle::isDefined(&ent->parent))
                if (ent->parent.isDefined())
                {
                    //v9 = EntHandle::ent(&ent->parent);
                    v9 = ent->parent.ent();
                }
                else
                {
                    v9 = 0;
                }
                G_FlashbangBlast(
                    ent->r.currentOrigin,
                    (float)weapDef->iExplosionRadius,
                    (float)weapDef->iExplosionRadiusMin,
                    v9,
                    ent->missile.team);
            }
            if (doEvent)
            {
                if (!eventEnt
                    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 2013, 1, "%s", "eventEnt"))
                {
                    __debugbreak();
                }
                SV_LinkEntity(eventEnt);
            }
            Scr_Notify(ent, scr_const.death, 0);
            G_FreeEntity(ent);
        }
        else
        {
            if (weapDef->guidedMissileType == MISSILE_GUIDANCE_TVGUIDED)
                G_UnlinkPlayerToRocket(ent);
            Scr_Notify(ent, scr_const.death, 0);
            G_FreeEntity(ent);
        }
    }
}

int __cdecl ForcedDud(gentity_s *ent)
{
    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 329, 0, "%s", "ent") )
        __debugbreak();
    return ent->s.lerp.u.missile.forcedDud;
}

int __cdecl GetSplashMethodOfDeath(gentity_s *ent)
{
    const WeaponDef *weapDef; // [esp+4h] [ebp-4h]

    if (!ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 801, 0, "%s", "ent"))
        __debugbreak();
    if (!ent->s.weapon
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 802, 0, "%s", "ent->s.weapon"))
    {
        __debugbreak();
    }
    weapDef = BG_GetWeaponDef(ent->s.weapon);
    if (!weapDef && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 805, 0, "%s", "weapDef"))
        __debugbreak();
    if (entityHandlers[ent->handler].splashMethodOfDeath == 4 && weapDef->projExplosion == WEAPPROJEXP_HEAVY)
        return 15;
    else
        return entityHandlers[ent->handler].splashMethodOfDeath;
}

void __cdecl G_UnlinkPlayerToRocket(gentity_s *ent)
{
    gclient_s *client; // [esp+0h] [ebp-4h]

    //if ( EntHandle::isDefined(&ent->parent) )
    if ( ent->parent.isDefined() )
    {
        //client = EntHandle::ent(&ent->parent)->client;
        client = ent->parent.ent()->client;
        if ( !client && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 858, 0, "%s", "client") )
            __debugbreak();
        if ( (client->ps.eFlags & 0x300) == 0 )
            client->ps.viewlocked_entNum = 1023;
        client->ps.weapFlags &= ~0x200u;
    }
}

void __cdecl Missile_InitAttractors()
{
    memset(&attrGlob, 0, sizeof(attrGlob));
}

void __cdecl Missile_FreeAttractorRefs(gentity_s *ent)
{
    AttractorRepulsor_t *v1; // ecx
    unsigned int attractorIndex; // [esp+0h] [ebp-4h]

    for ( attractorIndex = 0; attractorIndex < 0x20; ++attractorIndex )
    {
        if ( attrGlob.attractors[attractorIndex].inUse && attrGlob.attractors[attractorIndex].entnum == ent->s.number )
        {
            v1 = &attrGlob.attractors[attractorIndex];
            *(unsigned int *)&v1->inUse = 0;
            v1->entnum = 0;
            v1->origin[0] = 0.0;
            v1->origin[1] = 0.0;
            v1->origin[2] = 0.0;
            v1->strength = 0.0;
            v1->maxDist = 0.0;
        }
    }
}

void __cdecl Scr_MissileCreateAttractorEnt()
{
    unsigned int attractorIndex; // [esp+0h] [ebp-8h]
    gentity_s *ent; // [esp+4h] [ebp-4h]

    attractorIndex = Missile_GetFreeAttractor();
    attrGlob.attractors[attractorIndex].isAttractor = 1;
    ent = Scr_GetEntity(0);
    attrGlob.attractors[attractorIndex].strength = Scr_GetFloat(1u, SCRIPTINSTANCE_SERVER);
    attrGlob.attractors[attractorIndex].maxDist = Scr_GetFloat(2u, SCRIPTINSTANCE_SERVER);
    if ( attrGlob.attractors[attractorIndex].maxDist <= 0.0 )
        Scr_ParamError(2u, "maxDist must be greater than zero", SCRIPTINSTANCE_SERVER);
    attrGlob.attractors[attractorIndex].entnum = ent->s.number;
    ent->flags |= 0x800000;
    attrGlob.attractors[attractorIndex].inUse = 1;
    Scr_AddInt(attractorIndex, SCRIPTINSTANCE_SERVER);
}

unsigned int __cdecl Missile_GetFreeAttractor()
{
    const char *v0; // eax
    unsigned int attractorIndex; // [esp+0h] [ebp-4h]

    for ( attractorIndex = 0; attractorIndex < 0x20 && attrGlob.attractors[attractorIndex].inUse; ++attractorIndex )
        ;
    if ( attractorIndex == 32 )
    {
        v0 = va("Ran out of attractor/repulsors.    Max allowed: %i", 32);
        Scr_Error(v0, 0);
    }
    return attractorIndex;
}

void __cdecl Scr_MissileCreateAttractorOrigin()
{
    unsigned int attractorIndex; // [esp+0h] [ebp-4h]

    attractorIndex = Missile_GetFreeAttractor();
    attrGlob.attractors[attractorIndex].isAttractor = 1;
    attrGlob.attractors[attractorIndex].entnum = 1023;
    Scr_GetVector(0, attrGlob.attractors[attractorIndex].origin, SCRIPTINSTANCE_SERVER);
    attrGlob.attractors[attractorIndex].strength = Scr_GetFloat(1u, SCRIPTINSTANCE_SERVER);
    attrGlob.attractors[attractorIndex].maxDist = Scr_GetFloat(2u, SCRIPTINSTANCE_SERVER);
    if ( attrGlob.attractors[attractorIndex].maxDist <= 0.0 )
        Scr_ParamError(2u, "maxDist must be greater than zero", SCRIPTINSTANCE_SERVER);
    attrGlob.attractors[attractorIndex].inUse = 1;
    Scr_AddInt(attractorIndex, SCRIPTINSTANCE_SERVER);
}

void __cdecl Scr_MissileCreateRepulsorEnt()
{
    unsigned int attractorIndex; // [esp+0h] [ebp-4h]

    attractorIndex = Missile_GetFreeAttractor();
    attrGlob.attractors[attractorIndex].isAttractor = 0;
    attrGlob.attractors[attractorIndex].entnum = LOWORD(Scr_GetEntity(0)->s.number);
    attrGlob.attractors[attractorIndex].strength = Scr_GetFloat(1u, SCRIPTINSTANCE_SERVER);
    attrGlob.attractors[attractorIndex].maxDist = Scr_GetFloat(2u, SCRIPTINSTANCE_SERVER);
    if ( attrGlob.attractors[attractorIndex].maxDist <= 0.0 )
        Scr_ParamError(2u, "maxDist must be greater than zero", SCRIPTINSTANCE_SERVER);
    attrGlob.attractors[attractorIndex].inUse = 1;
    Scr_AddInt(attractorIndex, SCRIPTINSTANCE_SERVER);
}

void __cdecl Scr_MissileCreateRepulsorOrigin()
{
    unsigned int attractorIndex; // [esp+0h] [ebp-4h]

    attractorIndex = Missile_GetFreeAttractor();
    attrGlob.attractors[attractorIndex].isAttractor = 0;
    attrGlob.attractors[attractorIndex].entnum = 1023;
    Scr_GetVector(0, attrGlob.attractors[attractorIndex].origin, SCRIPTINSTANCE_SERVER);
    attrGlob.attractors[attractorIndex].strength = Scr_GetFloat(1u, SCRIPTINSTANCE_SERVER);
    attrGlob.attractors[attractorIndex].maxDist = Scr_GetFloat(2u, SCRIPTINSTANCE_SERVER);
    if ( attrGlob.attractors[attractorIndex].maxDist <= 0.0 )
        Scr_ParamError(2u, "maxDist must be greater than zero", SCRIPTINSTANCE_SERVER);
    attrGlob.attractors[attractorIndex].inUse = 1;
    Scr_AddInt(attractorIndex, SCRIPTINSTANCE_SERVER);
}

void __cdecl Scr_MissileDeleteAttractor()
{
    AttractorRepulsor_t *v0; // ecx
    unsigned int attractorIndex; // [esp+0h] [ebp-4h]

    attractorIndex = Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
    if ( attractorIndex >= 0x20 )
        Scr_ParamError(0, "Invalid attractor or repulsor", SCRIPTINSTANCE_SERVER);
    v0 = &attrGlob.attractors[attractorIndex];
    *(unsigned int *)&v0->inUse = 0;
    v0->entnum = 0;
    v0->origin[0] = 0.0;
    v0->origin[1] = 0.0;
    v0->origin[2] = 0.0;
    v0->strength = 0.0;
    v0->maxDist = 0.0;
}

void __cdecl G_MakeMissilePickupItem(gentity_s *ent)
{
    const gitem_s *item; // [esp+8h] [ebp-8h]
    int itemIndex; // [esp+Ch] [ebp-4h]

    ent->r.mins[0] = -1.0f;
    ent->r.mins[1] = -1.0f;
    ent->r.mins[2] = -1.0f;
    ent->r.maxs[0] = 1.0f;
    ent->r.maxs[1] = 1.0f;
    ent->r.maxs[2] = 1.0f;
    ent->r.contents |= 0x200000u;
    item = BG_FindItemForWeapon(ent->s.weapon);
    if ( !item && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 3389, 0, "%s", "item") )
        __debugbreak();
    itemIndex = ((char *)item - (char *)bg_itemlist) >> 2;
    ent->s.un3.item = (unsigned __int16)itemIndex;
    if ( ent->s.un3.item >= 0x800u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp",
                    3394,
                    0,
                    "ent->s.un3.item doesn't index MAX_ITEMS\n\t%i not in [0, %i)",
                    ent->s.un3.item,
                    2048) )
    {
        __debugbreak();
    }
    if ( ent->s.un3.item != itemIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp",
                    3395,
                    0,
                    "%s",
                    "ent->s.un3.item == itemIndex") )
    {
        __debugbreak();
    }
    if ( item->giType != IT_WEAPON
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp",
                    3396,
                    0,
                    "%s",
                    "item->giType == IT_WEAPON") )
    {
        __debugbreak();
    }
    ent->s.clientNum = 32;
}

void __cdecl G_RunMissileInternal(gentity_s *ent)
{
    const float *v1; // [esp+1Ch] [ebp-1C4h]
    float circleDir3[3]; // [esp+A8h] [ebp-138h] BYREF
    float center; // [esp+B4h] [ebp-12Ch] BYREF
    float v4; // [esp+B8h] [ebp-128h]
    float v5; // [esp+BCh] [ebp-124h]
    float radius; // [esp+C0h] [ebp-120h]
    float circleDir2[3]; // [esp+C4h] [ebp-11Ch] BYREF
    float circleDir1[3]; // [esp+D0h] [ebp-110h] BYREF
    const float *color; // [esp+DCh] [ebp-104h]
    unsigned int attractorIndex; // [esp+E0h] [ebp-100h]
    float tracePosSmall[3]; // [esp+E4h] [ebp-FCh] BYREF
    float tracePosLarge[3]; // [esp+F0h] [ebp-F0h] BYREF
    float travel_distance; // [esp+FCh] [ebp-E4h]
    float delta; // [esp+100h] [ebp-E0h]
    float originOffset[3]; // [esp+104h] [ebp-DCh]
    float traceStart[3]; // [esp+110h] [ebp-D0h] BYREF
    int clipMask; // [esp+11Ch] [ebp-C4h]
    float velocity[3]; // [esp+120h] [ebp-C0h] BYREF
    float speed; // [esp+12Ch] [ebp-B4h]
    float vOldOrigin[3]; // [esp+130h] [ebp-B0h] BYREF
    float dir[3]; // [esp+13Ch] [ebp-A4h] BYREF
    float origin[3]; // [esp+148h] [ebp-98h] BYREF
    gentity_s *owner; // [esp+154h] [ebp-8Ch]
    float endpos[3]; // [esp+158h] [ebp-88h] BYREF
    trace_t tr; // [esp+164h] [ebp-7Ch] BYREF
    trace_t trDown; // [esp+19Ch] [ebp-44h] BYREF
    int passEntNum; // [esp+1D8h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+1DCh] [ebp-4h]
    int savedregs; // [esp+1E0h] [ebp+0h] BYREF

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 3531, 0, "%s", "ent") )
        __debugbreak();
    if ( ent->s.eType != 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp",
                    3532,
                    0,
                    "%s",
                    "ent->s.eType == ET_MISSILE") )
    {
        __debugbreak();
    }
    weapDef = BG_GetWeaponDef(ent->s.weapon);
    if ( !weapDef && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 3535, 0, "%s", "weapDef") )
        __debugbreak();
    passEntNum = 1023;
    //if ( EntHandle::isDefined(&ent->r.ownerNum) )
    if ( ent->r.ownerNum.isDefined() )
    {
        //owner = EntHandle::ent(&ent->r.ownerNum);
        owner = ent->r.ownerNum.ent();
        if (owner->tagInfo && owner->client && (owner->client->ps.eFlags & 0x4000) != 0)
        {
            passEntNum = owner->tagInfo->parent->s.number;
        }
        else
        {
            //passEntNum = EntHandle::entnum(&ent->r.ownerNum);
            passEntNum = ent->r.ownerNum.entnum();
        }
    }
    RunMissile_BroadcastActorEvents(ent);
    if ( weapDef->stickiness != WEAPSTICKINESS_ALL_NO_SENTIENTS
        || ent->s.lerp.pos.trType
        || ent->s.groundEntityNum == 1022 )
    {
        if ( weapDef->isRollingGrenade && ent->s.lerp.pos.trType == 1 )
        {
            ent->s.lerp.pos.trType = G_GetGrenadeTrType(ent);
            ent->s.lerp.pos.trBase[0] = ent->r.currentOrigin[0];
            ent->s.lerp.pos.trBase[1] = ent->r.currentOrigin[1];
            ent->s.lerp.pos.trBase[2] = ent->r.currentOrigin[2];
        }
    }
    else
    {
        origin[0] = ent->r.currentOrigin[0];
        origin[1] = ent->r.currentOrigin[1];
        origin[2] = ent->r.currentOrigin[2];
        origin[0] = (float)(-1.635 * ent->mover.midTime) + origin[0];
        origin[1] = (float)(-1.635 * ent->mover.aMidTime) + origin[1];
        origin[2] = (float)(-1.635 * ent->trigger.exposureLerpToLighter) + origin[2];
        G_MissileTrace(&tr, ent->r.currentOrigin, origin, passEntNum, ent->clipmask, ent->s.weapon);
        if ( tr.fraction == 1.0 )
        {
            ent->s.lerp.pos.trType = 6;
            ent->s.lerp.pos.trTime = level.time;
            ent->s.lerp.pos.trDuration = 0;
            ent->s.lerp.pos.trBase[0] = ent->r.currentOrigin[0];
            ent->s.lerp.pos.trBase[1] = ent->r.currentOrigin[1];
            ent->s.lerp.pos.trBase[2] = ent->r.currentOrigin[2];
            ent->s.lerp.pos.trDelta[0] = 0.0f;
            ent->s.lerp.pos.trDelta[1] = 0.0f;
            ent->s.lerp.pos.trDelta[2] = 0.0f;
        }
    }
    vOldOrigin[0] = ent->r.currentOrigin[0];
    vOldOrigin[1] = ent->r.currentOrigin[1];
    vOldOrigin[2] = ent->r.currentOrigin[2];
    MissileTrajectory(ent, origin);
    if ( weapDef->guidedMissileType == MISSILE_GUIDANCE_TVGUIDED && CheckForMissileClientControlledDetonation(ent) )
        G_ExplodeMissile(ent);
    if ( ent->s.eType == 4 )
    {
        dir[0] = origin[0] - ent->r.currentOrigin[0];
        dir[1] = origin[1] - ent->r.currentOrigin[1];
        dir[2] = origin[2] - ent->r.currentOrigin[2];
        speed = Vec3Normalize(dir);
        if ( speed < 0.001 )
        {
            G_RunThink(ent);
            return;
        }
        if ( weapDef->guidedMissileType != MISSILE_GUIDANCE_TVGUIDED || UpdateGuidedMissileFuelTime(ent, 0) )
        {
            if ( weapDef->weapType == WEAPTYPE_PROJECTILE && weapDef->projExplosion == WEAPPROJEXP_BOLT )
            {
                BG_EvaluateTrajectoryDelta(&ent->s.lerp.pos, level.time, velocity);
                vectoangles(velocity, ent->r.currentAngles);
                G_SetAngle(ent, ent->r.currentAngles);
            }
            if ( speed <= 20.0 || SV_PointContents(ent->r.currentOrigin, -1, 32) )
            {
                G_MissileTrace(&tr, ent->r.currentOrigin, origin, passEntNum, ent->clipmask, ent->s.weapon);
            }
            else
            {
                clipMask = ent->clipmask | 0x20;
                G_MissileTrace(&tr, ent->r.currentOrigin, origin, passEntNum, clipMask, ent->s.weapon);
            }
            if ((tr.sflags & 0x3F00000) == 0x1400000)
            {
                Vec3Lerp(ent->r.currentOrigin, origin, tr.fraction, endpos);
                RunMissile_CreateWaterSplash(ent, endpos, &tr);
                G_MissileTrace(&tr, ent->r.currentOrigin, origin, passEntNum, ent->clipmask, ent->s.weapon);
            }
            if ((tr.sflags & 0x3F00000) == 0x900000 )
                Missile_PenetrateGlass(&tr, ent, ent->r.currentOrigin, origin, weapDef->damage, 0);
            Vec3Lerp(ent->r.currentOrigin, origin, tr.fraction, endpos);
            DrawMissileDebug(ent->r.currentOrigin, endpos);
            ent->r.currentOrigin[0] = endpos[0];
            ent->r.currentOrigin[1] = endpos[1];
            ent->r.currentOrigin[2] = endpos[2];
            if ((ent->s.lerp.eFlags & 0x1000000) != 0)
            {
                if ( weapDef->stickiness == WEAPSTICKINESS_ALL
                    || (weapDef->stickiness == WEAPSTICKINESS_GROUND || weapDef->stickiness == WEAPSTICKINESS_GROUND_WITH_YAW)
                    && tr.normal.vec.v[2] > 0.69999999 )
                {
                    if ( tr.fraction < 1.0 )
                    {
                        traceStart[0] = (float)(0.13500001 * tr.normal.vec.v[0]) + ent->r.currentOrigin[0];
                        traceStart[1] = (float)(0.13500001 * tr.normal.vec.v[1]) + ent->r.currentOrigin[1];
                        traceStart[2] = (float)(0.13500001 * tr.normal.vec.v[2]) + ent->r.currentOrigin[2];
                        origin[0] = (float)(-1.5 * tr.normal.vec.v[0]) + ent->r.currentOrigin[0];
                        origin[1] = (float)(-1.5 * tr.normal.vec.v[1]) + ent->r.currentOrigin[1];
                        origin[2] = (float)(-1.5 * tr.normal.vec.v[2]) + ent->r.currentOrigin[2];
                        G_MissileTrace(&trDown, traceStart, origin, passEntNum, ent->clipmask, ent->s.weapon);
                        if ( trDown.fraction != 1.0 )
                        {
                            memcpy(&tr, &trDown, sizeof(tr));
                            Vec3Lerp(traceStart, origin, tr.fraction, endpos);
                            originOffset[0] = endpos[0] - origin[0];
                            originOffset[1] = endpos[1] - origin[1];
                            originOffset[2] = endpos[2] - origin[2];
                            ent->s.lerp.pos.trBase[0] = ent->s.lerp.pos.trBase[0] + (float)(endpos[0] - origin[0]);
                            ent->s.lerp.pos.trBase[1] = ent->s.lerp.pos.trBase[1] + originOffset[1];
                            ent->s.lerp.pos.trBase[2] = ent->s.lerp.pos.trBase[2] + originOffset[2];
                            ent->r.currentOrigin[0] = endpos[0] + originOffset[0];
                            ent->r.currentOrigin[1] = endpos[1] + originOffset[1];
                            ent->r.currentOrigin[2] = endpos[2] + originOffset[2];
                        }
                    }
                }
                else if ( tr.fraction == 1.0 || tr.fraction < 1.0 && tr.normal.vec.v[2] > 0.69999999 )
                {
                    traceStart[0] = ent->r.currentOrigin[0];
                    traceStart[1] = ent->r.currentOrigin[1];
                    traceStart[2] = ent->r.currentOrigin[2];
                    origin[0] = ent->r.currentOrigin[0];
                    origin[1] = ent->r.currentOrigin[1];
                    origin[2] = ent->r.currentOrigin[2];
                    traceStart[2] = traceStart[2] + 0.13500001;
                    origin[2] = origin[2] - 1.5;
                    G_MissileTrace(&trDown, traceStart, origin, passEntNum, ent->clipmask, ent->s.weapon);
                    if ( trDown.fraction != 1.0 )
                    {
                        memcpy(&tr, &trDown, sizeof(tr));
                        Vec3Lerp(traceStart, origin, tr.fraction, endpos);
                        ent->s.lerp.pos.trBase[2] = (float)((float)(endpos[2] + 1.5) - ent->r.currentOrigin[2])
                                                                            + ent->s.lerp.pos.trBase[2];
                        ent->r.currentOrigin[0] = endpos[0];
                        ent->r.currentOrigin[1] = endpos[1];
                        ent->r.currentOrigin[2] = endpos[2];
                        ent->r.currentOrigin[2] = ent->r.currentOrigin[2] + 1.5;
                    }
                }
            }
            SV_LinkEntity(ent);
            if ( weapDef->iProjectileActivateDist > 0 )
            {
                delta = Vec3Distance(vOldOrigin, endpos);
                travel_distance = ent->mover.aSpeed + delta;
                ent->mover.aSpeed = travel_distance;
            }
            if (entityHandlers[ent->handler].methodOfDeath == 3)
                G_GrenadeTouchTriggerDamage(
                    ent,
                    vOldOrigin,
                    ent->r.currentOrigin,
                    weapDef->iExplosionInnerDamage,
                    entityHandlers[ent->handler].methodOfDeath);
            if ( tr.fraction == 1.0 )
            {
                if (Vec3Length(ent->s.lerp.pos.trDelta) != 0.0 )
                {
                    ent->s.groundEntityNum = 1023;
                    if ( weapDef->weapType == WEAPTYPE_PROJECTILE
                        && (ent->flags & 0x20000) == 0
                        && (weapDef->guidedMissileType == MISSILE_GUIDANCE_NONE
                         || weapDef->guidedMissileType == MISSILE_GUIDANCE_SIDEWINDER && weapDef->destabilizeDistance > 0) )
                    {
                        RunMissile_Destabilize(ent);
                    }
                }
                goto LABEL_90;
            }
            if ( weapDef->weapType == WEAPTYPE_BOMB )
            {
                Vec3Lerp(ent->r.currentOrigin, origin, 1.0, tracePosSmall);
                G_MissileTrace(&tr, tracePosSmall, ent->r.currentOrigin, passEntNum, ent->clipmask, ent->s.weapon);
                if ( (tr.sflags & 4) != 0 )
                {
                    Vec3Lerp(origin, ent->r.currentOrigin, tr.fraction, tracePosSmall);
                    ent->r.currentOrigin[0] = tracePosSmall[0];
                    ent->r.currentOrigin[1] = tracePosSmall[1];
                    ent->r.currentOrigin[2] = tracePosSmall[2];
                }
                else
                {
                    Vec3Lerp(ent->r.currentOrigin, origin, 2.0, tracePosLarge);
                    G_MissileTrace(&tr, tracePosLarge, ent->r.currentOrigin, passEntNum, ent->clipmask, ent->s.weapon);
                    if ( (tr.sflags & 4) != 0 )
                    {
                        Vec3Lerp(origin, ent->r.currentOrigin, tr.fraction, tracePosLarge);
                        ent->r.currentOrigin[0] = tracePosLarge[0];
                        ent->r.currentOrigin[1] = tracePosLarge[1];
                        ent->r.currentOrigin[2] = tracePosLarge[2];
                    }
                    else
                    {
                        MissileImpact(ent, &tr, dir, endpos);
                    }
                }
                return;
            }
            if ( (tr.sflags & 4) != 0 )
            {
                ent->r.currentOrigin[0] = origin[0];
                ent->r.currentOrigin[1] = origin[1];
                ent->r.currentOrigin[2] = origin[2];
                return;
            }
            if ( (tr.cflags & 0x800) != 0 )
            {
                ent->r.currentOrigin[0] = origin[0];
                ent->r.currentOrigin[1] = origin[1];
                ent->r.currentOrigin[2] = origin[2];
                return;
            }
            MissileImpact(ent, &tr, dir, endpos);
            if ( ent->s.eType == 4 )
            {
                if ( weapDef->isRollingGrenade && tr.normal.vec.v[2] > 0.69999999 && ent->s.lerp.pos.trType )
                {
                    if ( ent->s.lerp.pos.trType != 6
                        && ent->s.lerp.pos.trType != 7
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp",
                                    3795,
                                    0,
                                    "%s\n\t(ent->s.lerp.pos.trType) = %i",
                                    "(ent->s.lerp.pos.trType == TR_GRAVITY || ent->s.lerp.pos.trType == TR_LOW_GRAVITY)",
                                    ent->s.lerp.pos.trType) )
                    {
                        __debugbreak();
                    }
                    ent->s.lerp.pos.trType = 1;
                    ent->s.lerp.pos.trBase[0] = ent->r.currentOrigin[0];
                    ent->s.lerp.pos.trBase[1] = ent->r.currentOrigin[1];
                    ent->s.lerp.pos.trBase[2] = ent->r.currentOrigin[2];
                }
LABEL_90:
                if ( missileDebugAttractors->current.enabled )
                {
                    for ( attractorIndex = 0; attractorIndex < 0x20; ++attractorIndex )
                    {
                        circleDir1[0] = 1.0f;
                        circleDir1[1] = 0.0f;
                        circleDir1[2] = 0.0f;
                        circleDir2[0] = 0.0f;
                        circleDir2[1] = 1.0f;
                        circleDir2[2] = 0.0f;
                        circleDir3[0] = 0.0f;
                        circleDir3[1] = 0.0f;
                        circleDir3[2] = 1.0f;
                        radius = 10.0f;
                        if ( attrGlob.attractors[attractorIndex].inUse )
                        {
                            if ( attrGlob.attractors[attractorIndex].entnum == 1023 )
                            {
                                center = attrGlob.attractors[attractorIndex].origin[0];
                                v4 = attrGlob.attractors[attractorIndex].origin[1];
                                v5 = attrGlob.attractors[attractorIndex].origin[2];
                            }
                            else
                            {
                                if ( attrGlob.attractors[attractorIndex].entnum >= 1024
                                    && !Assert_MyHandler(
                                                "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp",
                                                3831,
                                                0,
                                                "%s",
                                                "attrGlob.attractors[attractorIndex].entnum < MAX_GENTITIES") )
                                {
                                    __debugbreak();
                                }
                                ent = &g_entities[attrGlob.attractors[attractorIndex].entnum];
                                center = ent->r.currentOrigin[0];
                                v4 = ent->r.currentOrigin[1];
                                v5 = ent->r.currentOrigin[2];
                            }
                            if ( attrGlob.attractors[attractorIndex].isAttractor )
                                v1 = colorGreen;
                            else
                                v1 = colorOrange;
                            color = v1;
                            G_DebugCircleEx(&center, radius, circleDir1, v1, 0, 1);
                            G_DebugCircleEx(&center, radius, circleDir2, color, 0, 1);
                            G_DebugCircleEx(&center, radius, circleDir3, color, 0, 1);
                        }
                    }
                }
                G_RunThink(ent);
                if ( ent->r.inuse && !BG_ValidateOrigin(origin, 16, 17, svs.mapCenter) )
                {
                    if ( weapDef->guidedMissileType == MISSILE_GUIDANCE_TVGUIDED )
                        G_UnlinkPlayerToRocket(ent);
                    Scr_Notify(ent, scr_const.death, 0);
                    G_FreeEntity(ent);
                }
            }
        }
    }
}

// local variable allocation has failed, the output may be wrong!
void    MissileImpact(gentity_s *ent, trace_t *trace, float *dir, float *endpos)
{
    gentity_s *v5; // eax
    unsigned __int8 v6; // al
    gentity_s *v7; // eax
    unsigned __int8 v8; // al
    unsigned __int8 v9; // al
    unsigned __int8 v10; // al
    unsigned __int8 v11; // al
    unsigned __int8 v12; // al
    unsigned __int8 v13; // al
    unsigned __int8 v14; // al
    unsigned __int8 v15; // al
    unsigned __int8 v16; // al
    unsigned __int8 v17; // al
    unsigned __int8 v18; // al
    unsigned int WeaponIndexForName; // eax
    gentity_s *v20; // eax
    gentity_s *v21; // eax
    double v22; // xmm0_8
    float v23; // xmm0_4
    gentity_s *v24; // eax
    gentity_s *v25; // eax
    float fInnerDamage; // [esp+0h] [ebp-18Ch]
    float fOuterDamage; // [esp+4h] [ebp-188h]
    float radius; // [esp+8h] [ebp-184h]
    int radius_max; // [esp+10h] [ebp-17Ch]
    float *radius_min; // [esp+14h] [ebp-178h]
    gentity_s *radius_mina; // [esp+14h] [ebp-178h]
    unsigned __int16 z_up; // [esp+18h] [ebp-174h]
    int fRadiusSqrd; // [esp+1Ch] [ebp-170h]
    unsigned int fRadiusSqrda; // [esp+1Ch] [ebp-170h]
    long double v35; // [esp+20h] [ebp-16Ch]
    gentity_s *v36; // [esp+28h] [ebp-164h]
    float coneAngleCos; // [esp+30h] [ebp-15Ch]
    int splashMethodOfDeath; // [esp+3Ch] [ebp-150h]
    gentity_s *attacker; // [esp+40h] [ebp-14Ch]
    unsigned __int8 v40; // [esp+47h] [ebp-145h]
    int entnum; // [esp+50h] [ebp-13Ch]
    unsigned int boneName; // [esp+54h] [ebp-138h] BYREF
    bool isExplodingCrossbowBolt; // [esp+5Bh] [ebp-131h]
    BOOL v44; // [esp+5Ch] [ebp-130h]
    const char *v45; // [esp+60h] [ebp-12Ch]
    const char *v46; // [esp+68h] [ebp-124h]
    const char *szSpawnedGrenadeWeaponName; // [esp+6Ch] [ebp-120h]
    gentity_s *owner; // [esp+70h] [ebp-11Ch]
    bool hit_vehicle; // [esp+77h] [ebp-115h]
    float javNormal[3]; // [esp+78h] [ebp-114h] BYREF
    const float *normal; // [esp+84h] [ebp-108h]
    float depth; // [esp+88h] [ebp-104h]
    bool waterExplodeAllowed; // [esp+8Fh] [ebp-FDh]
    int nomarks; // [esp+90h] [ebp-FCh]
    BOOL v55; // [esp+94h] [ebp-F8h]
    gentity_s *v56; // [esp+98h] [ebp-F4h]
    gentity_s *v57; // [esp+9Ch] [ebp-F0h]
    int dFlags; // [esp+A0h] [ebp-ECh]
    float speed; // [esp+A4h] [ebp-E8h]
    float velocity[3]; // [esp+A8h] [ebp-E4h] BYREF
    gentity_s *v61; // [esp+B4h] [ebp-D8h]
    float waterSurfacePos[4]; // [esp+B8h] [ebp-D4h] BYREF
    float waterNormal[3]; // [esp+C8h] [ebp-C4h] BYREF
    bool inWater; // [esp+D7h] [ebp-B5h]
    BOOL v65; // [esp+D8h] [ebp-B4h]
    float waterHeight; // [esp+DCh] [ebp-B0h]
    float preBounceVelocity[4]; // [esp+E0h] [ebp-ACh] BYREF
    gentity_s *v68; // [esp+F0h] [ebp-9Ch]
    float v69[3]; // [esp+F4h] [ebp-98h] BYREF
    //float v70; // [esp+F8h] [ebp-94h]
    //float v71; // [esp+FCh] [ebp-90h]
    hitLocation_t hitLocation; // [esp+100h] [ebp-8Ch]
    hitLocation_t partGroup; // [esp+104h] [ebp-88h]
    int methodOfDeath; // [esp+108h] [ebp-84h]
    bool dud; // [esp+10Fh] [ebp-7Dh]
    gentity_s *eventEnt; // [esp+110h] [ebp-7Ch]
    float direction_vector[3]; // [esp+114h] [ebp-78h] BYREF
    int damage; // [esp+120h] [ebp-6Ch]
    int explosionType; // [esp+124h] [ebp-68h]
    int explodeOnImpact; // [esp+128h] [ebp-64h]
    const WeaponDef *weapDef; // [esp+12Ch] [ebp-60h]
    const WeaponVariantDef *weapVariantDef; // [esp+130h] [ebp-5Ch]
    gentity_s *other; // [esp+134h] [ebp-58h]
    int hitEntId; // [esp+138h] [ebp-54h]
    bool impactDamageDealt; // [esp+13Fh] [ebp-4Dh]
    trace_t waterTrace; // [esp+140h] [ebp-4Ch] BYREF
    int hitClient; // [esp+178h] [ebp-14h]
    //_UNKNOWN *v88[2]; // [esp+180h] [ebp-Ch] BYREF
    //trace_t *tracea; // [esp+188h] [ebp-4h] BYREF
    //float *dira; // [esp+18Ch] [ebp+0h]
    //
    //v88[0] = a1;
    //v88[1] = dira;
    hitClient = 0;
    impactDamageDealt = 0;
    if (!ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 1187, 0, "%s", "ent"))
        __debugbreak();
    if (ent->s.eType != 4
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp",
            1188,
            0,
            "%s",
            "ent->s.eType == ET_MISSILE"))
    {
        __debugbreak();
    }
    if (!ent->s.weapon
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 1189, 0, "%s", "ent->s.weapon"))
    {
        __debugbreak();
    }
    hitEntId = Trace_GetEntityHitId(trace);
    other = &g_entities[(unsigned __int16)hitEntId];
    weapVariantDef = BG_GetWeaponVariantDef(ent->s.weapon);
    weapDef = BG_GetWeaponDef(ent->s.weapon);
    if (!weapVariantDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 1196, 0, "%s", "weapVariantDef"))
    {
        __debugbreak();
    }
    if (!weapDef && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 1197, 0, "%s", "weapDef"))
        __debugbreak();
    explodeOnImpact = weapDef->bProjImpactExplode;
    explosionType = weapDef->projExplosion;
    damage = weapDef->damage;
    if (trace->hitType == TRACE_HITTYPE_GLASS)
    {
        BG_EvaluateTrajectoryDelta(&ent->s.lerp.pos, level.time, direction_vector);
        if ((float)((float)((float)(direction_vector[0] * direction_vector[0])
            + (float)(direction_vector[1] * direction_vector[1]))
            + (float)(direction_vector[2] * direction_vector[2])) >= 0.001)
        {
            Vec3Normalize(direction_vector);
        }
        else
        {
            direction_vector[0] = 0.0f;
            direction_vector[1] = 0.0f;
            direction_vector[2] = 1.0f;
        }
        GlassSv_Damage(trace->hitId, damage, 16, ent->r.currentOrigin, direction_vector);
    }
    else
    {
        ent->s.surfType = (trace->sflags & 0x3F00000) >> 20;
        eventEnt = 0;
        dud = isDud(ent, weapDef);
        if (dud)
        {
            explosionType = 4;
            explodeOnImpact = 0;
            ent->mover.aSpeed = -1.0e10;
            methodOfDeath = 16;
        }
        else if (explodeOnImpact)
        {
            methodOfDeath = entityHandlers[ent->handler].methodOfDeath;
        }
        else
        {
            methodOfDeath = 16;
        }
        if (methodOfDeath == 16 || explosionType == 9)
            partGroup = (hitLocation_t)trace->partGroup;
        else
            partGroup = HITLOC_NONE;
        hitLocation = partGroup;
        //if (methodOfDeath != 7 && EntHandle::isDefined(&ent->r.ownerNum))
        if (methodOfDeath != 7 && ent->r.ownerNum.isDefined())
        {
            //radius_min = EntHandle::ent(&ent->r.ownerNum)->s.lerp.pos.trBase;
            radius_min = ent->r.ownerNum.ent()->s.lerp.pos.trBase;
            //v5 = EntHandle::ent(&ent->r.ownerNum);
            v5 = ent->r.ownerNum.ent();
            Actor_BroadcastLineEvent(v5, 0, AI_EV_PROJECTILE_IMPACT, 0, radius_min, endpos, 0.0);
        }
        if (other->takedamage && damage > 0)
        {
            BG_EvaluateTrajectoryDelta(&ent->s.lerp.pos, level.time, v69);
            if ((float)((float)((float)(v69[0] * v69[0]) + (float)(v69[1] * v69[1])) + (float)(v69[2] * v69[2])) < 0.001)
            {
                v69[0] = 0.0f;
                v69[1] = 0.0f;
                v69[2] = 1.0f;
            }
            //if (EntHandle::isDefined(&ent->r.ownerNum))
            if (ent->r.ownerNum.isDefined())
            {
                //v68 = EntHandle::ent(&ent->r.ownerNum);
                v68 = ent->r.ownerNum.ent();
            }
            else
                v68 = 0;
            G_Damage(
                other,
                ent,
                v68,
                v69,
                ent->r.currentOrigin,
                damage,
                0,
                methodOfDeath,
                ent->s.weapon,
                hitLocation,
                trace->modelIndex,
                trace->partName,
                0);
            impactDamageDealt = 1;
        }
        if (weapDef->guidedMissileType == MISSILE_GUIDANCE_TVGUIDED)
            G_UnlinkPlayerToRocket(ent);
        if ((ent->s.lerp.eFlags & 0x1000000) == 0 || explodeOnImpact || CheckCrumpleMissile(ent, trace))
        {
            waterHeight = CM_GetWaterHeight(ent->r.currentOrigin, 1000.0, -200.0);
            v65 = waterHeight >= ent->r.currentOrigin[2];
            inWater = v65;
            if (v65)
            {
                waterNormal[0] = 0.0f;
                waterNormal[1] = 0.0f;
                waterNormal[2] = 1.0f;
                //LODWORD(waterSurfacePos[3]) = ent->r.currentOrigin;
                waterSurfacePos[0] = ent->r.currentOrigin[0];
                waterSurfacePos[1] = ent->r.currentOrigin[1];
                waterSurfacePos[2] = waterHeight;
            }
            else
            {
                inWater = SV_PointContents(ent->r.currentOrigin, -1, 32) != 0;
            }
            if (explosionType == 7 && inWater)
                damage = 0;
            if (!other->takedamage)
                goto LABEL_98;
            if (damage)
            {
                //if (EntHandle::isDefined(&ent->r.ownerNum))
                if (ent->r.ownerNum.isDefined())
                {
                    //v61 = EntHandle::ent(&ent->r.ownerNum);
                    v61 = ent->r.ownerNum.ent();
                }
                else
                    v61 = &g_entities[1023];

                if (LogAccuracyHit(other, v61))
                    hitClient = 1;
                BG_EvaluateTrajectoryDelta(&ent->s.lerp.pos, level.time, velocity);
                speed = Vec3Length(velocity);
                if (speed == 0.0)
                    velocity[2] = 1.0f;
                if (weapDef->weapType != WEAPTYPE_GRENADE && weapDef->weapType != WEAPTYPE_MINE
                    || explosionType == 7
                    || speed > g_minGrenadeDamageSpeed->current.value)
                {
                    dFlags = 0;
                    if (weapDef->armorPiercing)
                        dFlags |= 2u;
                    if (!impactDamageDealt)
                    {
                        //if (EntHandle::isDefined(&ent->r.ownerNum))
                        if (ent->r.ownerNum.isDefined())
                        {
                            //v57 = EntHandle::ent(&ent->r.ownerNum);
                            v57 = ent->r.ownerNum.ent();
                        }
                        else
                        {
                            v57 = 0;
                        }
                        G_Damage(
                            other,
                            ent,
                            v57,
                            velocity,
                            ent->r.currentOrigin,
                            damage,
                            dFlags,
                            methodOfDeath,
                            ent->s.weapon,
                            hitLocation,
                            trace->modelIndex,
                            trace->partName,
                            0);
                    }
                }
            }
            if (explodeOnImpact)
                goto LABEL_98;
            if (other->client && !trace->sflags)
                trace->sflags = 0x700000;
            if (CheckCrumpleMissile(ent, trace) || !isBounceProjectile(ent))
            {
            LABEL_98:
                if (damage)
                {
                    //if (EntHandle::isDefined(&ent->r.ownerNum))
                    if (ent->r.ownerNum.isDefined())
                    {
                        //v56 = EntHandle::ent(&ent->r.ownerNum);
                        v56 = ent->r.ownerNum.ent();
                    }
                    else
                        v56 = &g_entities[1022];
                    G_CheckHitTriggerDamage(v56, ent->r.currentOrigin, endpos, damage, methodOfDeath);
                }
                v55 = hitClient || trace->partName;
                nomarks = v55;
                waterExplodeAllowed = 1;
                if (inWater && explosionType != 7)
                {
                    depth = waterHeight - endpos[2];
                    if (depth <= -1.0 || missileWaterMaxDepth->current.value <= depth)
                    {
                        waterExplodeAllowed = 0;
                    }
                    else
                    {
                        waterNormal[0] = 0.0f;
                        waterNormal[1] = 0.0f;
                        waterNormal[2] = 1.0f;
                        waterSurfacePos[0] = *endpos;
                        waterSurfacePos[1] = endpos[1];
                        waterSurfacePos[2] = waterHeight;
                    }
                }
                if (inWater && waterExplodeAllowed)
                {
                    normal = waterNormal;
                }
                else if (ent->s.eType == 4
                    && weapDef->guidedMissileType == MISSILE_GUIDANCE_JAVELIN
                    && ent->missile.missile.flightMode == MISSILEFLIGHTMODE_TOP
                    //&& EntHandle::isDefined(&ent->missileTargetEnt)
                    && ent->missileTargetEnt.isDefined()
                    //&& (v7 = EntHandle::ent(&ent->missileTargetEnt), v7 == other))
                    && (v7 = ent->missileTargetEnt.ent(), v7 == other))
                {
                    normal = up;
                }
                else if (weapDef->projExplosionEffectForceNormalUp)
                {
                    normal = up;
                }
                else
                {
                    normal = (const float *)trace;
                }
                if (!inWater || waterExplodeAllowed || explosionType == 7)
                {
                    if (explosionType && explosionType != 6)
                    {
                        switch (explosionType)
                        {
                        case 1:
                            v10 = DirToByte(normal);
                            G_AddEvent(ent, (nomarks != 0) + 58, v10);
                            break;
                        case 3:
                            v11 = DirToByte(normal);
                            G_AddEvent(ent, (nomarks != 0) + 61, v11);
                            break;
                        case 2:
                            v12 = DirToByte(normal);
                            G_AddEvent(ent, 0x3Cu, v12);
                            break;
                        case 4:
                            if (JavelinProjectile(ent, weapDef))
                            {
                                Vec3NormalizeTo(ent->s.lerp.pos.trDelta, javNormal);
                                (javNormal[0]) = -(javNormal[0]);
                                (javNormal[1]) = -(javNormal[1]);
                                (javNormal[2]) = -(javNormal[2]);
                                v13 = DirToByte(javNormal);
                                G_AddEvent(ent, 0x41u, v13);
                            }
                            else
                            {
                                v14 = DirToByte(normal);
                                G_AddEvent(ent, 0x41u, v14);
                            }
                            break;
                        case 7:
                            if (inWater)
                            {
                                explodeOnImpact = 0;
                                waterExplodeAllowed = 0;
                                v16 = DirToByte(normal);
                                G_AddEvent(ent, 0x44u, v16);
                            }
                            else
                            {
                                hit_vehicle = other->scr_vehicle != 0;
                                //if (!hit_vehicle && EntHandle::isDefined(&other->parent))
                                if (!hit_vehicle && other->parent.isDefined())
                                {
                                    //hit_vehicle = EntHandle::ent(&other->parent)->scr_vehicle != 0;
                                    hit_vehicle = other->parent.ent()->scr_vehicle != 0;
                                }

                                if (!hit_vehicle)
                                    Weapon_Napalm_Flame(ent, trace, missileMolotovBlobNum->current.integer);

                                //if (EntHandle::isDefined(&ent->r.ownerNum))
                                if (ent->r.ownerNum.isDefined())
                                {
                                    //owner = EntHandle::ent(&ent->parent);
                                    owner = ent->parent.ent();
                                    AssignToSmallerType<short>(&ent->s.attackerEntityNum, owner->s.number);
                                }
                                else
                                {
                                    ent->s.attackerEntityNum = -1;
                                }
                                v15 = DirToByte(normal);
                                G_AddEvent(ent, 0x42u, v15);
                            }
                            break;
                        case 8:
                            eventEnt = G_Spawn();
                            eventEnt->s.eType = ET_GENERAL;
                            eventEnt->s.lerp.eFlags |= 0x20u;
                            eventEnt->s.weapon = ent->s.weapon;
                            eventEnt->s.weaponModel = ent->s.weaponModel;
                            eventEnt->r.contents = ent->r.contents;
                            G_SetOrigin(eventEnt, endpos);
                            G_BroadcastEntity(eventEnt);
                            v17 = DirToByte(normal);
                            G_AddEvent(eventEnt, 0x42u, v17);
                            eventEnt->s.lerp.pos.trBase[0] = (float)(int)eventEnt->s.lerp.pos.trBase[0];
                            eventEnt->s.lerp.pos.trBase[1] = (float)(int)eventEnt->s.lerp.pos.trBase[1];
                            eventEnt->s.lerp.pos.trBase[2] = (float)(int)eventEnt->s.lerp.pos.trBase[2];
                            G_SetOrigin(eventEnt, eventEnt->s.lerp.pos.trBase);
                            eventEnt->s.lerp.eFlags |= 0x4000u;
                            eventEnt->s.lerp.u.actor.actorNum = level.time;
                            eventEnt->s.time2 = level.time + (int)(float)(missileMolotovBlobTime->current.value * 1000.0);
                            eventEnt->s.lerp.eFlags |= 0x10u;
                            eventEnt->handler = 11;
                            eventEnt->nextthink = level.time + 1;
                            eventEnt->s.attackerEntityNum = ent->s.attackerEntityNum;
                            break;
                        case 9:
                            szSpawnedGrenadeWeaponName = weapDef->szSpawnedGrenadeWeaponName;
                            v46 = szSpawnedGrenadeWeaponName + 1;
                            szSpawnedGrenadeWeaponName += strlen(szSpawnedGrenadeWeaponName) + 1;
                            v45 = (const char *)(szSpawnedGrenadeWeaponName - v46);
                            v44 = szSpawnedGrenadeWeaponName != v46 && (other->s.number || other->client);
                            isExplodingCrossbowBolt = v44;
                            boneName = 0;
                            entnum = (unsigned __int16)hitEntId;
                            if (other->client)
                            {
                                MapHitLocationToRagdollBoneName(hitLocation, &boneName);
                                if (other->client->ps.pm_type == 9
                                    && *(int *)&g_scr_data.playerCorpseInfo[1504 * other->client->ps.corpseIndex + 4] >= 0)
                                {
                                    entnum = *(int*)&g_scr_data.playerCorpseInfo[1504 * other->client->ps.corpseIndex + 4];
                                }
                            }
                            else if (other->actor)
                            {
                                MapHitLocationToRagdollBoneName(hitLocation, &boneName);
                            }
                            else if (other->scr_vehicle)
                            {
                                boneName = trace->partName;
                            }
                            if (boneName)
                                AssignToSmallerType<unsigned short>((unsigned __int16 *)&ent->s.index, boneName);
                            AssignToSmallerType<short>(&ent->s.groundEntityNum, entnum);
                            v18 = DirToByte(normal);
                            G_AddEvent(ent, 0xC1u, v18);
                            if (isExplodingCrossbowBolt)
                            {
                                WeaponIndexForName = G_GetWeaponIndexForName((char *)weapDef->szSpawnedGrenadeWeaponName);
                                fRadiusSqrd = BG_GetWeaponDef(WeaponIndexForName)->fuseTime;
                                radius_max = G_GetWeaponIndexForName((char *)weapDef->szSpawnedGrenadeWeaponName);
                                //v20 = EntHandle::ent(&ent->parent);
                                v20 = ent->parent.ent();
                                v21 = G_FireGrenade(v20, endpos, dir, radius_max, 0, 0, fRadiusSqrd);
                                eventEnt = v21;
                                v21->mover.midTime = trace->normal.vec.v[0];
                                v21->mover.aMidTime = trace->normal.vec.v[1];
                                v21->trigger.exposureLerpToLighter = trace->normal.vec.v[2];
                                AttachMissileToEntity(
                                    eventEnt,
                                    entnum,
                                    boneName,
                                    hitLocation,
                                    ent->s.lerp.apos.trDelta);
                                AttachBoltGrenade(eventEnt, other, trace, hitLocation);
                            }
                            break;
                        }
                    }
                    else
                    {
                        v9 = DirToByte(normal);
                        G_AddEvent(ent, 0x39u, v9);
                    }
                }
                else
                {
                    v8 = DirToByte(normal);
                    G_AddEvent(ent, 0x40u, v8);
                }
                if (inWater)
                    v40 = 20;
                else
                    v40 = (trace->sflags & 0x3F00000) >> 20;
                ent->s.surfType = v40;
                attacker = 0;

                //if (EntHandle::isDefined(&ent->r.ownerNum))
                if (ent->r.ownerNum.isDefined())
                {
                    //attacker = EntHandle::ent(&ent->r.ownerNum);
                    attacker = ent->r.ownerNum.ent();
                }
                if (weapDef->bDieOnRespawn || weapDef->bRetrievable)
                {
                    createRetrieveableProjectile(ent, other, weapDef, trace, hitLocation, dir, endpos);
                }
                else if (attacker)
                {
                    Scr_AddEntity(attacker, SCRIPTINSTANCE_SERVER);
                    Scr_Notify(ent, scr_const.death, 1u);
                }
                else
                {
                    Scr_Notify(ent, scr_const.death, 0);
                }
                ent->s.lerp.eFlags ^= 2u;
                ent->s.lerp.eFlags |= 0x20u;
                if (inWater && waterExplodeAllowed)
                    G_SetOrigin(ent, waterSurfacePos);
                else
                    G_SetOrigin(ent, endpos);
                if (explodeOnImpact)
                {
                    if (weapDef->iExplosionInnerDamage)
                    {
                        splashMethodOfDeath = GetSplashMethodOfDeath(ent);
                        //if (EntHandle::isDefined(&ent->parent))
                        if (ent->parent.isDefined())
                        {
                            v22 = (float)(weapDef->damageConeAngle * 0.017453292);
                            //__libm_sse2_cos(v35);
                            v23 = cos(v22);
                            if ((float)(weapDef->damageConeAngle - 180.0) < 0.0)
                                coneAngleCos = v23;
                            else
                                coneAngleCos = -1.0f;
                            if (other && other->destructible)
                                other = 0;
                            fRadiusSqrda = ent->s.weapon;
                            radius_mina = other;
                            radius = (float)weapDef->iExplosionRadius;
                            fOuterDamage = (float)weapDef->iExplosionOuterDamage;
                            fInnerDamage = (float)weapDef->iExplosionInnerDamage;
                            //v24 = EntHandle::ent(&ent->parent);
                            v24 = ent->parent.ent();
                            G_RadiusDamage(
                                endpos,
                                ent,
                                v24,
                                fInnerDamage,
                                fOuterDamage,
                                radius,
                                coneAngleCos,
                                dir,
                                radius_mina,
                                splashMethodOfDeath,
                                fRadiusSqrda);
                            Scr_AddEntity(ent, SCRIPTINSTANCE_SERVER);
                            Scr_AddInt(weapDef->iExplosionRadius, SCRIPTINSTANCE_SERVER);
                            Scr_AddVector(endpos, SCRIPTINSTANCE_SERVER);
                            Scr_AddString((char *)weapVariantDef->szInternalName, SCRIPTINSTANCE_SERVER);
                            z_up = scr_const.projectile_impact;
                            //v25 = EntHandle::ent(&ent->parent);
                            v25 = ent->parent.ent();
                            Scr_Notify(v25, z_up, 4u);
                        }
                    }
                }
                if (explosionType == 2)
                {
                    //if (EntHandle::isDefined(&ent->parent))
                    if (ent->parent.isDefined())
                    {
                        //v36 = EntHandle::ent(&ent->parent);
                        v36 = ent->parent.ent();
                    }
                    else
                        v36 = 0;
                    G_FlashbangBlast(
                        endpos,
                        (float)weapDef->iExplosionRadius,
                        (float)weapDef->iExplosionRadiusMin,
                        v36,
                        ent->missile.team);
                }
                if (explosionType == 8)
                    G_FreeEntity(ent);
                else
                    G_FreeEntityAfterEvent(ent);
                if (explosionType == 8)
                {
                    if (eventEnt)
                        SV_LinkEntity(eventEnt);
                }
                else
                {
                    SV_LinkEntity(ent);
                }
            }
            else if (BounceMissile(ent, trace) && !trace->startsolid)
            {
                G_AddEvent(ent, 0x38u, (trace->sflags & 0x3F00000) >> 20);
                ent->s.surfType = (trace->sflags & 0x3F00000) >> 20;
            }
        }
        else
        {
            //LODWORD(preBounceVelocity[3]) = ent->s.lerp.pos.trDelta;
            preBounceVelocity[0] = ent->s.lerp.pos.trDelta[0];
            preBounceVelocity[1] = ent->s.lerp.pos.trDelta[1];
            preBounceVelocity[2] = ent->s.lerp.pos.trDelta[2];
            if (BounceMissile(ent, trace) && !trace->startsolid)
            {
                if (weapDef->stickiness == WEAPSTICKINESS_ALL || weapDef->stickiness == WEAPSTICKINESS_FLESH)
                {
                    if (StickMissile(ent, other, weapDef, trace, hitLocation, preBounceVelocity))
                    {
                        GScr_AddEntity(ent);
                        Scr_Notify(other, scr_const.grenade_stuck, 1u);
                        Scr_AddVector(trace->normal.vec.v, SCRIPTINSTANCE_SERVER);
                        Scr_AddVector(endpos, SCRIPTINSTANCE_SERVER);
                        Scr_Notify(ent, scr_const.stationary, 2u);
                    }
                    else
                    {
                        Scr_AddVector(trace->normal.vec.v, SCRIPTINSTANCE_SERVER);
                        Scr_AddVector(endpos, SCRIPTINSTANCE_SERVER);
                        Scr_Notify(ent, scr_const.grenade_bounce, 2u);
                    }
                }
                else
                {
                    Scr_AddVector(trace->normal.vec.v, SCRIPTINSTANCE_SERVER);
                    Scr_AddVector(endpos, SCRIPTINSTANCE_SERVER);
                    Scr_Notify(ent, scr_const.grenade_bounce, 2u);
                }
                if (other->sentient && !trace->sflags)
                    trace->sflags = 0x700000;
                G_AddEvent(ent, 0x38u, (trace->sflags & 0x3F00000) >> 20);
                ent->s.surfType = (trace->sflags & 0x3F00000) >> 20;
            }
            if (dud && !ent->s.lerp.pos.trType)
            {
                v6 = DirToByte(trace->normal.vec.v);
                G_AddEvent(ent, 0x3Fu, v6);
                Scr_Notify(ent, scr_const.death, 0);
                G_FreeEntityAfterEvent(ent);
            }
        }
    }
}

bool __cdecl CheckCrumpleMissile(gentity_s *ent, trace_t *trace)
{
    float velocity[3]; // [esp+Ch] [ebp-20h] BYREF
    int hitTime; // [esp+18h] [ebp-14h]
    float MIN_CRUMPLE_SPEED; // [esp+1Ch] [ebp-10h]
    float cos45; // [esp+20h] [ebp-Ch]
    float speed; // [esp+24h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+28h] [ebp-4h]

    if ( !ent->s.weapon
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 342, 0, "%s", "ent->s.weapon") )
    {
        __debugbreak();
    }
    weapDef = BG_GetWeaponDef(ent->s.weapon);
    if ( !weapDef && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 344, 0, "%s", "weapDef") )
        __debugbreak();
    if ( ForcedDud(ent) )
        return 0;
    if ( weapDef->weapType != WEAPTYPE_PROJECTILE )
        return 0;
    if ( trace->sflags == 0x700000 )
        return 1;
    if ( weapDef->bNoCrumpleMissile )
        return 0;
    hitTime = level.previousTime + (int)(float)((float)(level.time - level.previousTime) * trace->fraction);
    BG_EvaluateTrajectoryDelta(&ent->s.lerp.pos, hitTime, velocity);
    speed = Vec3Length(velocity);
    MIN_CRUMPLE_SPEED = 500.0f;
    if ( speed < 500.0 )
        return 0;
    velocity[0] = (float)(-1.0 / speed) * velocity[0];
    velocity[1] = (float)(-1.0 / speed) * velocity[1];
    velocity[2] = (float)(-1.0 / speed) * velocity[2];
    cos45 = 0.70700002f;
    return (float)((float)((float)(velocity[0] * trace->normal.vec.v[0]) + (float)(velocity[1] * trace->normal.vec.v[1])) + (float)(velocity[2] * trace->normal.vec.v[2])) > 0.70700002;
}

// local variable allocation has failed, the output may be wrong!
bool    BounceMissile(gentity_s *ent, trace_t *trace)
{
    float v3; // xmm0_4
    long double v5; // [esp+20h] [ebp-1CCh]
    long double v6; // [esp+20h] [ebp-1CCh]
    int v7; // [esp+38h] [ebp-1B4h]
    float traceEnd[4]; // [esp+3Ch] [ebp-1B0h] BYREF
    float vDelta[3]; // [esp+4Ch] [ebp-1A0h]
    gentity_s *groundEnt; // [esp+58h] [ebp-194h]
    int *p_clipAmmoCount; // [esp+5Ch] [ebp-190h]
    float vAngles[3]; // [esp+60h] [ebp-18Ch] BYREF
    float v13; // [esp+6Ch] [ebp-180h]
    float v14; // [esp+70h] [ebp-17Ch]
    float v15; // [esp+74h] [ebp-178h]
    float v16; // [esp+78h] [ebp-174h]
    float v17; // [esp+7Ch] [ebp-170h]
    float v18; // [esp+80h] [ebp-16Ch]
    float targetAngles[3]; // [esp+84h] [ebp-168h] BYREF
    float revolutions; // [esp+90h] [ebp-15Ch]
    float linearSpeed; // [esp+94h] [ebp-158h]
    float *v22; // [esp+98h] [ebp-154h]
    float v23; // [esp+9Ch] [ebp-150h]
    float *v24; // [esp+A0h] [ebp-14Ch]
    int v25; // [esp+A4h] [ebp-148h]
    float wobbleNewPos[4]; // [esp+A8h] [ebp-144h] BYREF
    float v27; // [esp+B8h] [ebp-134h]
    float wobbleSideScale; // [esp+BCh] [ebp-130h]
    float v29; // [esp+C0h] [ebp-12Ch]
    float v30; // [esp+C4h] [ebp-128h]
    float side[3]; // [esp+C8h] [ebp-124h] BYREF
    float *v32; // [esp+D4h] [ebp-118h]
    float v33; // [esp+D8h] [ebp-114h]
    float *v34; // [esp+DCh] [ebp-110h]
    float wobbleDelta; // [esp+E0h] [ebp-10Ch]
    float v36; // [esp+E4h] [ebp-108h]
    float v37; // [esp+E8h] [ebp-104h]
    float oldCycle; // [esp+ECh] [ebp-100h]
    float wobbleFreq; // [esp+F0h] [ebp-FCh]
    float *v41; // [esp+12Ch] [ebp-C0h]
    float *v42; // [esp+130h] [ebp-BCh]
    float mag; // [esp+134h] [ebp-B8h]
    float v44; // [esp+138h] [ebp-B4h]
    float v45; // [esp+13Ch] [ebp-B0h]
    float value; // [esp+140h] [ebp-ACh]
    float slideSpeed; // [esp+144h] [ebp-A8h] BYREF
    float slideDir[3]; // [esp+148h] [ebp-A4h] BYREF
    bool isDud; // [esp+156h] [ebp-96h]
    bool doRolling; // [esp+157h] [ebp-95h]
    BOOL v51; // [esp+158h] [ebp-94h]
    bool mayStop; // [esp+15Fh] [ebp-8Dh]
    float v53; // [esp+160h] [ebp-8Ch]
    float v54; // [esp+164h] [ebp-88h]
    float v55; // [esp+168h] [ebp-84h]
    float v56; // [esp+16Ch] [ebp-80h]
    float *v57; // [esp+170h] [ebp-7Ch]
    float v58; // [esp+174h] [ebp-78h]
    float *trDelta; // [esp+178h] [ebp-74h]
    int shouldRoll; // [esp+17Ch] [ebp-70h]
    BOOL v61; // [esp+180h] [ebp-6Ch]
    float dot; // [esp+184h] [ebp-68h]
    float velocity[3]; // [esp+188h] [ebp-64h] BYREF
    int hitTime; // [esp+194h] [ebp-58h]
    int surfType; // [esp+198h] [ebp-54h]
    int contents; // [esp+19Ch] [ebp-50h]
    const WeaponDef *weapDef; // [esp+1A0h] [ebp-4Ch]
    trace_t tempTrace; // [esp+1A4h] [ebp-48h] BYREF
    //_UNKNOWN *v69; // [esp+1E0h] [ebp-Ch]
    //gentity_s *enta; // [esp+1E4h] [ebp-8h]
    //trace_t *tracea; // [esp+1E8h] [ebp-4h] BYREF
    //int vars0; // [esp+1ECh] [ebp+0h]
    //
    //v69 = a1;
    //enta = (gentity_s *)vars0;
    if (!ent->s.weapon
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 529, 0, "%s", "ent->s.weapon"))
    {
        __debugbreak();
    }
    weapDef = BG_GetWeaponDef(ent->s.weapon);
    if (!weapDef && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 531, 0, "%s", "weapDef"))
        __debugbreak();
    contents = SV_PointContents(ent->r.currentOrigin, -1, 32);
    surfType = (unsigned __int8)((int)(0x3F00000 & trace->sflags) >> 20);
    hitTime = level.previousTime + (int)(float)((float)(level.time - level.previousTime) * trace->fraction);
    BG_EvaluateTrajectoryDelta(&ent->s.lerp.pos, hitTime, velocity);
    dot = (float)((float)(velocity[0] * trace->normal.vec.v[0]) + (float)(velocity[1] * trace->normal.vec.v[1]))
        + (float)(velocity[2] * trace->normal.vec.v[2]);
    v61 = grenadeRollingEnabled->current.enabled && weapDef->isRollingGrenade;
    shouldRoll = v61;
    if (v61)
    {
        trDelta = ent->s.lerp.pos.trDelta;
        (v58) = -(dot);
        ent->s.lerp.pos.trDelta[0] = (float)((-(dot)) * trace->normal.vec.v[0])
            + velocity[0];
        trDelta[1] = (float)(v58 * trace->normal.vec.v[1]) + velocity[1];
        trDelta[2] = (float)(v58 * trace->normal.vec.v[2]) + velocity[2];
    }
    else
    {
        v57 = ent->s.lerp.pos.trDelta;
        v56 = -2.0 * dot;
        ent->s.lerp.pos.trDelta[0] = (float)((float)(-2.0 * dot) * trace->normal.vec.v[0]) + velocity[0];
        v57[1] = (float)(v56 * trace->normal.vec.v[1]) + velocity[1];
        v57[2] = (float)(v56 * trace->normal.vec.v[2]) + velocity[2];
    }
    v55 = ent->s.lerp.pos.trDelta[0];
    if ((LODWORD(v55) & 0x7F800000) == 0x7F800000
        || (v54 = ent->s.lerp.pos.trDelta[1], (LODWORD(v54) & 0x7F800000) == 0x7F800000)
        || (v53 = ent->s.lerp.pos.trDelta[2], (LODWORD(v53) & 0x7F800000) == 0x7F800000))
    {
        if (!Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp",
            546,
            0,
            "%s",
            "!IS_NAN((ent->s.lerp.pos.trDelta)[0]) && !IS_NAN((ent->s.lerp.pos.trDelta)[1]) && !IS_NAN((ent->s.lerp.pos.trDelta)[2])"))
            __debugbreak();
    }
    if (weapDef->stickiness == WEAPSTICKINESS_ALL)
    {
        mayStop = 1;
        if (weapDef->stickiness == WEAPSTICKINESS_ALL_NO_SENTIENTS
            && (Trace_GetEntityHitId(trace) < 0x20u || g_entities[Trace_GetEntityHitId(trace)].s.eType == 17))
        {
            mayStop = 0;
        }
    }
    else
    {
        v51 = weapDef->stickiness == WEAPSTICKINESS_NONE || Trace_GetEntityHitId(trace) >= 0x20u;
        mayStop = v51;
    }
    if (weapDef->stickiness != WEAPSTICKINESS_GROUND_WITH_YAW && g_entities[Trace_GetEntityHitId(trace)].s.eType == 4)
        mayStop = 0;
    if (mayStop
        && (weapDef->stickiness == WEAPSTICKINESS_ALL
            || weapDef->stickiness == WEAPSTICKINESS_ALL_NO_SENTIENTS
            || trace->normal.vec.v[2] > 0.69999999))
    {
        ent->s.groundEntityNum = Trace_GetEntityHitId(trace);
        g_entities[ent->s.groundEntityNum].flags |= 0x100000u;
    }
    doRolling = 0;
    if ((ent->s.lerp.eFlags & 0x1000000) == 0)
        goto LABEL_103;
    isDud = ent->s.lerp.u.turret.ownerNum != 0;
    doRolling = GrenadeBounceVelocity(
        velocity,
        dot,
        trace->normal.vec.v,
        surfType,
        weapDef,
        &ent->s.lerp.pos,
        &slideSpeed,
        slideDir,
        isDud);
    if (doRolling)
    {
        if (grenadeBumpFreq->current.value > G_random())
        {
            value = grenadeBumpMax->current.value;
            v45 = slideSpeed * grenadeBumpMag->current.value;
            v3 = (float)(v45 - value) < 0.0 ? v45 : value;
            v44 = v3;
            mag = v3;
            if (v3 > (float)(bg_gravity->current.value * 0.05f))
            {
                v42 = ent->s.lerp.pos.trDelta;
                v41 = ent->s.lerp.pos.trDelta;
                ent->s.lerp.pos.trDelta[0] = (float)(mag * trace->normal.vec.v[0]) + ent->s.lerp.pos.trDelta[0];
                v42[1] = (float)(mag * trace->normal.vec.v[1]) + v41[1];
                v42[2] = (float)(mag * trace->normal.vec.v[2]) + v41[2];
            }
        }
        if (slideDir[2] < 25.0 && trace->normal.vec.v[2] > 0.69999999)
        {
            trace_t sideTrace; // [esp+F4h] [ebp-F8h] BYREF
            wobbleFreq = slideSpeed * grenadeWobbleFreq->current.value;
            oldCycle = ent->mover.apos2[2];
            for (ent->mover.apos2[2] = (float)((float)((float)(wobbleFreq * 0.05f) * 2.0) * 3.1415901)
                + ent->mover.apos2[2];
                ent->mover.apos2[2] > 6.2831802;
                ent->mover.apos2[2] = ent->mover.apos2[2] - 6.2831802)
            {
                ;
            }
            v37 = ent->mover.apos2[2];
            
            //__libm_sse2_sin(v5);
            v36 = sin(v37);
            //__libm_sse2_sin(v6);
            wobbleDelta = sin(v37) - oldCycle; // KISAKTODO: sin() 2x seems weird?

            if (slideSpeed > 0.0099999998)
            {
                v34 = ent->s.lerp.pos.trDelta;
                v33 = (float)((float)(wobbleDelta * grenadeWobbleFwdMag->current.value) + slideSpeed) / slideSpeed;
                v32 = ent->s.lerp.pos.trDelta;
                ent->s.lerp.pos.trDelta[0] = v33 * ent->s.lerp.pos.trDelta[0];
                v34[1] = v33 * v32[1];
                v34[2] = v33 * v32[2];
            }
            Vec3Cross(slideDir, trace->normal.vec.v, side);
            v30 = 1.0 - (float)(slideSpeed * grenadeWobbleSideDamp->current.value);
            if ((float)(0.0 - v30) < 0.0)
                v29 = v30;
            else
                v29 = 0.0f;
            wobbleSideScale = v29;
            if (v29 > 0.0)
            {
                v27 = (float)(wobbleDelta * grenadeWobbleSideMag->current.value) * wobbleSideScale;
                //LODWORD(wobbleNewPos[3]) = ent->r.currentOrigin;
                wobbleNewPos[0] = (float)(v27 * side[0]) + ent->r.currentOrigin[0];
                wobbleNewPos[1] = (float)(v27 * side[1]) + ent->r.currentOrigin[1];
                wobbleNewPos[2] = (float)(v27 * side[2]) + ent->r.currentOrigin[2];

                //if (EntHandle::isDefined(&ent->r.ownerNum))
                if (ent->r.ownerNum.isDefined())
                {
                    //v25 = EntHandle::entnum(&ent->r.ownerNum);
                    v25 = ent->r.ownerNum.entnum();
                }
                else
                {
                    v25 = 1023;
                }
                G_MissileTrace(
                    &sideTrace,
                    ent->r.currentOrigin,
                    wobbleNewPos,
                    v25,
                    ent->clipmask,
                    ent->s.weapon);
                Vec3Lerp(ent->r.currentOrigin, wobbleNewPos, sideTrace.fraction, ent->r.currentOrigin);
            }
            if (ent->mover.apos3[0] == 0.0)
                ent->mover.apos3[0] = G_random() * 2.0 - 1.0;
            v24 = ent->s.lerp.pos.trDelta;
            v23 = ent->mover.apos3[0] * grenadeCurveMax->current.value;
            v22 = ent->s.lerp.pos.trDelta;
            ent->s.lerp.pos.trDelta[0] = (float)(v23 * side[0]) + ent->s.lerp.pos.trDelta[0];
            v24[1] = (float)(v23 * side[1]) + v22[1];
            v24[2] = (float)(v23 * side[2]) + v22[2];
        }
        linearSpeed = Vec3Length(ent->s.lerp.pos.trDelta);
        revolutions = linearSpeed / (float)((float)(3.1415901 * rollRadius) * 2.0);
        vectoangles(ent->s.lerp.pos.trDelta, targetAngles);
        ent->s.lerp.apos.trBase[1] = targetAngles[1];
        ent->s.lerp.apos.trBase[2] = 90.0f;
        ent->s.lerp.apos.trDelta[0] = 360.0 * revolutions;
        ent->s.lerp.apos.trDelta[1] = 0.0f;
        ent->s.lerp.apos.trDelta[2] = 0.0f;
    }
    v18 = ent->r.currentOrigin[0];
    if ((LODWORD(v18) & 0x7F800000) == 0x7F800000
        || (v17 = ent->r.currentOrigin[1], (LODWORD(v17) & 0x7F800000) == 0x7F800000)
        || (v16 = ent->r.currentOrigin[2], (LODWORD(v16) & 0x7F800000) == 0x7F800000))
    {
        if (!Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp",
            654,
            0,
            "%s",
            "!IS_NAN((ent->r.currentOrigin)[0]) && !IS_NAN((ent->r.currentOrigin)[1]) && !IS_NAN((ent->r.currentOrigin)[2])"))
            __debugbreak();
    }
    v15 = ent->s.lerp.pos.trDelta[0];
    if ((LODWORD(v15) & 0x7F800000) == 0x7F800000
        || (v14 = ent->s.lerp.pos.trDelta[1], (LODWORD(v14) & 0x7F800000) == 0x7F800000)
        || (v13 = ent->s.lerp.pos.trDelta[2], (LODWORD(v13) & 0x7F800000) == 0x7F800000))
    {
        if (!Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp",
            655,
            0,
            "%s",
            "!IS_NAN((ent->s.lerp.pos.trDelta)[0]) && !IS_NAN((ent->s.lerp.pos.trDelta)[1]) && !IS_NAN((ent->s.lerp.pos.trDelta)[2])"))
            __debugbreak();
    }
    if (!mayStop
        || weapDef->stickiness != WEAPSTICKINESS_ALL
        && weapDef->stickiness != WEAPSTICKINESS_ALL_NO_SENTIENTS
        && (trace->normal.vec.v[2] <= 0.69999999
            || weapDef->stickiness != WEAPSTICKINESS_GROUND
            && weapDef->stickiness != WEAPSTICKINESS_GROUND_WITH_YAW
            && grenadeRestThreshold->current.value <= Vec3Length(ent->s.lerp.pos.trDelta)))
    {
    LABEL_103:
        vDelta[0] = 0.1 * trace->normal.vec.v[0];
        vDelta[1] = 0.1 * trace->normal.vec.v[1];
        vDelta[2] = 0.1 * trace->normal.vec.v[2];
        if (vDelta[2] > 0.0)
            vDelta[2] = 0.0f;
        //LODWORD(traceEnd[3]) = ent->r.currentOrigin;
        traceEnd[0] = ent->r.currentOrigin[0] + vDelta[0];
        traceEnd[1] = ent->r.currentOrigin[1] + vDelta[1];
        traceEnd[2] = ent->r.currentOrigin[2] + vDelta[2];

        //if (EntHandle::isDefined(&ent->r.ownerNum))
        if (ent->r.ownerNum.isDefined())
        {
            //v7 = EntHandle::entnum(&ent->r.ownerNum);
            v7 = ent->r.ownerNum.entnum();
            G_MissileTrace(&tempTrace, ent->r.currentOrigin, traceEnd, v7, ent->clipmask, ent->s.weapon);
        }
        else
        {
            G_MissileTrace(&tempTrace, ent->r.currentOrigin, traceEnd, 1023, ent->clipmask, ent->s.weapon);
        }
        Vec3Lerp(ent->r.currentOrigin, traceEnd, tempTrace.fraction, ent->r.currentOrigin);
        ent->s.lerp.pos.trBase[0] = ent->r.currentOrigin[0];
        ent->s.lerp.pos.trBase[1] = ent->r.currentOrigin[1];
        ent->s.lerp.pos.trBase[2] = ent->r.currentOrigin[2];
        ent->s.lerp.pos.trTime = level.time;
        if (doRolling
            || (float)((float)((float)(ent->s.lerp.pos.trDelta[0] * trace->normal.vec.v[0])
                + (float)(ent->s.lerp.pos.trDelta[1] * trace->normal.vec.v[1]))
                + (float)(ent->s.lerp.pos.trDelta[2] * trace->normal.vec.v[2])) <= 25.0)
        {
            goto LABEL_117;
        }
        if (weapDef->rotateType)
        {
            if (weapDef->rotateType == WEAPROTATE_BLADE_ROTATE)
            {
                MissileLandAngles(ent, trace, vAngles, 0, 1);
            LABEL_116:
                ent->s.lerp.apos.trBase[0] = vAngles[0];
                ent->s.lerp.apos.trBase[1] = vAngles[1];
                ent->s.lerp.apos.trBase[2] = vAngles[2];
                ent->s.lerp.apos.trTime = level.time;
            LABEL_117:
                if (contents)
                    return 0;
                velocity[0] = ent->s.lerp.pos.trDelta[0] - velocity[0];
                velocity[1] = ent->s.lerp.pos.trDelta[1] - velocity[1];
                velocity[2] = ent->s.lerp.pos.trDelta[2] - velocity[2];
                dot = Vec3Length(velocity);
                return dot > 100.0;
            }
            if (weapDef->rotateType == WEAPROTATE_CYLINDER_ROTATE)
            {
                MissileLandAngles(ent, trace, vAngles, 1, 0);
                goto LABEL_116;
            }
        }
        MissileLandAngles(ent, trace, vAngles, 0, 0);
        goto LABEL_116;
    }
    G_SetOrigin(ent, ent->r.currentOrigin);
    if (weapDef->stickiness == WEAPSTICKINESS_GROUND_WITH_YAW || weapDef->stickiness == WEAPSTICKINESS_ALL_NO_SENTIENTS)
    {
        MissileLandAnglesFlatMaintainingDirection(ent, trace, vAngles);
    }
    else if (weapDef->stickiness == WEAPSTICKINESS_GROUND)
    {
        MissileLandAnglesFlat(ent, trace, vAngles);
    }
    else
    {
        MissileLandAngles(ent, trace, vAngles, 1, 0);
    }
    G_SetAngle(ent, vAngles);
    p_clipAmmoCount = &ent->item[1].clipAmmoCount;
    ent->mover.midTime = trace->normal.vec.v[0];
    p_clipAmmoCount[1] = trace->normal.vec.u[1];
    p_clipAmmoCount[2] = trace->normal.vec.u[2];
    if (!weapDef->timedDetonation)
        ent->nextthink = 0;
    CheckGrenadeDanger(ent);
    if (ent->s.groundEntityNum != 1023 && ent->s.groundEntityNum != 1022)
    {
        if (ent->s.groundEntityNum >= 1024
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp",
                686,
                0,
                "%s",
                "ent->s.groundEntityNum < MAX_GENTITIES"))
        {
            __debugbreak();
        }
        if (ent->s.groundEntityNum < 1024)
        {
            groundEnt = &g_entities[ent->s.groundEntityNum];
            if (!groundEnt
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 690, 0, "%s", "groundEnt"))
            {
                __debugbreak();
            }
            if (groundEnt)
            {
                if (groundEnt->s.eType == 6)
                {
                    ent->flags |= 0x1000u;
                    G_EntLinkTo(ent, groundEnt, 0);
                }
            }
        }
    }
    Scr_Notify(ent, scr_const.stationary, 0);
    return 1;
}

void __cdecl MissileLandAngles(gentity_s *ent, trace_t *trace, float *vAngles, int bForceAlign, int bKeepSpeed)
{
    double v5; // st7
    double v6; // st7
    char v7; // al
    int hitTime; // [esp+1Ch] [ebp-28h]
    float fSurfacePitch; // [esp+20h] [ebp-24h]
    float fAbsAngDelta; // [esp+24h] [ebp-20h]
    const WeaponDef *weapDef; // [esp+30h] [ebp-14h]
    float fAngleDelta; // [esp+40h] [ebp-4h]

    weapDef = BG_GetWeaponDef(ent->s.weapon);
    hitTime = level.previousTime + (int)(float)((float)(level.time - level.previousTime) * trace->fraction);
    BG_EvaluateTrajectory(&ent->s.lerp.apos, hitTime, vAngles);
    if ( !grenadeRollingEnabled->current.enabled || !weapDef->bKeepRolling || ent->s.lerp.u.turret.ownerNum )
    {
        if ( trace->normal.vec.v[2] <= 0.1 )
        {
            if ( !bForceAlign && !bKeepSpeed )
            {
                v7 = G_rand();
                ent->s.lerp.apos.trDelta[0] = AngleNormalize360((float)((v7 & 0x7F) - 63) + ent->s.lerp.apos.trDelta[0]);
            }
        }
        else
        {
            fSurfacePitch = PitchForYawOnNormal(vAngles[1], trace->normal.vec.v);
            fAngleDelta = AngleNormalize180(fSurfacePitch - *vAngles);
            fAbsAngDelta = fabs(fAngleDelta);
            if ( !bForceAlign || bKeepSpeed )
            {
                ent->s.lerp.apos.trBase[0] = *vAngles;
                ent->s.lerp.apos.trBase[1] = vAngles[1];
                ent->s.lerp.apos.trBase[2] = vAngles[2];
                ent->s.lerp.apos.trTime = hitTime;
                if ( fAbsAngDelta >= 80.0 )
                {
                    if ( !bKeepSpeed )
                        ent->s.lerp.apos.trDelta[0] = (G_random() * 0.30000001 + 0.85000002) * ent->s.lerp.apos.trDelta[0];
                }
                else if ( bKeepSpeed )
                {
                    ent->s.lerp.apos.trDelta[0] = ent->s.lerp.apos.trDelta[0] * -1.0;
                }
                else
                {
                    ent->s.lerp.apos.trDelta[0] = (G_random() * 0.30000001 + 0.85000002) * ent->s.lerp.apos.trDelta[0] * -1.0;
                }
            }
            *vAngles = AngleNormalize180(*vAngles);
            if ( bForceAlign || fAbsAngDelta < 45.0 )
            {
                if ( fabs(*vAngles) <= 90.0 )
                    v5 = AngleNormalize360(fSurfacePitch);
                else
                    v5 = AngleNormalize360(fSurfacePitch + 180.0);
                *vAngles = v5;
            }
            else
            {
                if ( fAbsAngDelta >= 80.0 )
                    v6 = AngleNormalize360(*vAngles);
                else
                    v6 = AngleNormalize360((float)(fAngleDelta * 0.25) + *vAngles);
                *vAngles = v6;
            }
        }
        if ( bForceAlign && weapDef->rotateType == WEAPROTATE_CYLINDER_ROTATE )
            vAngles[2] = 90.0f;
        if ( bForceAlign && !bKeepSpeed && weapDef->rotateType == WEAPROTATE_BLADE_ROTATE )
            vAngles[2] = 90.0f;
    }
}

void __cdecl MissileLandAnglesFlat(gentity_s *ent, trace_t *trace, float *angles)
{
    float normalUpComponent; // [esp+Ch] [ebp-20h]
    float right[3]; // [esp+10h] [ebp-1Ch] BYREF
    float normalRightComponent; // [esp+1Ch] [ebp-10h]
    float up[3]; // [esp+20h] [ebp-Ch] BYREF

    iassert(ent);
    iassert(trace);

    BG_EvaluateTrajectory(
        &ent->s.lerp.apos,
        level.previousTime + (int)((double)(level.time - level.previousTime) * trace->fraction),
        angles);

    NearestPitchAndYawOnPlane(angles, trace->normal.vec.v, angles);

    iassert(angles[ROLL] == 0.f);

    AngleVectors(angles, 0, right, up);
    normalRightComponent = Vec3Dot(right, trace->normal.vec.v);
    normalUpComponent = Vec3Dot(up, trace->normal.vec.v);
    angles[2] = atan2(normalRightComponent, normalUpComponent) * 180.0f / 3.141592741012573f;
}

void __cdecl MissileLandAnglesFlatMaintainingDirection(gentity_s *ent, trace_t *trace, float *angles)
{
    float normalUpComponent; // [esp+14h] [ebp-20h]
    float right[3]; // [esp+18h] [ebp-1Ch] BYREF
    float normalRightComponent; // [esp+24h] [ebp-10h]
    float up[3]; // [esp+28h] [ebp-Ch] BYREF

    iassert(ent);
    iassert(trace);

    BG_EvaluateTrajectory(
        &ent->s.lerp.apos,
        level.previousTime + (int)((double)(level.time - level.previousTime) * trace->fraction),
        angles);
    angles[0] = PitchForYawOnNormal(angles[1], trace->normal.vec.v);
    angles[2] = 0.0f;
    AngleVectors(angles, 0, right, up);
    normalRightComponent = Vec3Dot(right, trace->normal.vec.v);
    normalUpComponent = Vec3Dot(up, trace->normal.vec.v);
    angles[2] = atan2(normalRightComponent, normalUpComponent) * 180.0f / 3.141592741012573f;
}

void __cdecl CheckGrenadeDanger(gentity_s *grenadeEnt)
{
    char *v1; // eax
    gentity_s *v2; // eax
    float damageRadiusSquared; // [esp+Ch] [ebp-10h]
    gentity_s *ent; // [esp+10h] [ebp-Ch]
    int i; // [esp+14h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+18h] [ebp-4h]

    if ( !grenadeEnt
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 382, 0, "%s", "grenadeEnt") )
    {
        __debugbreak();
    }
    if ( !grenadeEnt->s.weapon
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp",
                    383,
                    0,
                    "%s",
                    "grenadeEnt->s.weapon != WP_NONE") )
    {
        __debugbreak();
    }
    weapDef = BG_GetWeaponDef(grenadeEnt->s.weapon);
    if ( !weapDef && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 385, 0, "%s", "weapDef") )
        __debugbreak();
    damageRadiusSquared = (float)weapDef->iExplosionRadius * (float)weapDef->iExplosionRadius;
    ent = g_entities;
    for ( i = 0; i < level.maxclients; ++i )
    {
        if ( ent->r.inuse )
        {
            if ( !ent->client
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 393, 0, "%s", "ent->client") )
            {
                __debugbreak();
            }
            if ( G_WithinDamageRadius(grenadeEnt->r.currentOrigin, damageRadiusSquared, ent) )
            {
                v1 = (char *)BG_WeaponName(grenadeEnt->s.weapon);
                Scr_AddString(v1, SCRIPTINSTANCE_SERVER);
                //if ( EntHandle::isDefined(&grenadeEnt->parent) )
                if ( grenadeEnt->parent.isDefined() )
                {
                    //v2 = EntHandle::ent(&grenadeEnt->parent);
                    v2 = grenadeEnt->parent.ent();
                    Scr_AddEntity(v2, SCRIPTINSTANCE_SERVER);
                }
                else
                {
                    Scr_AddUndefined(SCRIPTINSTANCE_SERVER);
                }
                Scr_AddEntity(grenadeEnt, SCRIPTINSTANCE_SERVER);
                Scr_Notify(ent, scr_const.grenadedanger, 3u);
            }
        }
        ++ent;
    }
}

char __cdecl GrenadeBounceVelocity(
                const float *preBounceVelocity,
                float dot,
                const float *normal,
                int surfType,
                const WeaponDef *weapDef,
                trajectory_t *pos,
                float *rollSlideSpeed,
                float *rollSlideDir,
                bool isDud)
{
    float v10; // [esp+0h] [ebp-88h]
    float value; // [esp+4h] [ebp-84h]
    bool v12; // [esp+8h] [ebp-80h]
    float v13; // [esp+1Ch] [ebp-6Ch]
    float v14; // [esp+24h] [ebp-64h]
    float v15; // [esp+44h] [ebp-44h]
    float slideDir[3]; // [esp+5Ch] [ebp-2Ch] BYREF
    float slideSpeed; // [esp+68h] [ebp-20h]
    float v18; // [esp+6Ch] [ebp-1Ch]
    float frictionScale; // [esp+70h] [ebp-18h]
    float tr_n; // [esp+74h] [ebp-14h]
    float per; // [esp+78h] [ebp-10h]
    float par; // [esp+7Ch] [ebp-Ch]
    float bounceFactor; // [esp+80h] [ebp-8h]
    bool shouldRoll; // [esp+87h] [ebp-1h]

    bounceFactor = Vec3Length(preBounceVelocity);
    if ( bounceFactor <= 0.0 || dot > 0.0 )
        goto LABEL_30;
    v12 = grenadeRollingEnabled->current.enabled && weapDef->isRollingGrenade;
    shouldRoll = v12;
    if ( !v12 || isDud )
    {
        par = weapDef->parallelBounce[surfType];
        per = weapDef->perpendicularBounce[surfType];
        if ( weapDef->bKeepRolling && !isDud && grenadeRollThreshold > Vec3Length(pos->trDelta) )
            par = (float)(par * rollFrac_0) + par;
        tr_n = (float)((float)(pos->trDelta[0] * *normal) + (float)(pos->trDelta[1] * normal[1]))
                 + (float)(pos->trDelta[2] * normal[2]);
        v15 = (float)(per - par) * tr_n;
        pos->trDelta[0] = (float)(v15 * *normal) + (float)(par * pos->trDelta[0]);
        pos->trDelta[1] = (float)(v15 * normal[1]) + (float)(par * pos->trDelta[1]);
        pos->trDelta[2] = (float)(v15 * normal[2]) + (float)(par * pos->trDelta[2]);
        goto LABEL_30;
    }
    slideSpeed = Vec3NormalizeTo(pos->trDelta, slideDir);
    if ( slideSpeed <= grenadeFrictionThresh->current.value )
        value = grenadeFrictionLow->current.value;
    else
        value = grenadeFrictionHigh->current.value;
    frictionScale = 1.0 - value;
    pos->trDelta[0] = (float)(1.0 - value) * pos->trDelta[0];
    pos->trDelta[1] = frictionScale * pos->trDelta[1];
    pos->trDelta[2] = frictionScale * pos->trDelta[2];
    if ( fabs(dot) >= 25.0 )
    {
        v18 = 1.0 - (float)((float)(1.0 - weapDef->perpendicularBounce[surfType]) * normal[2]);
        if ( (float)(grenadeBounceRestitutionMax->current.value - weapDef->perpendicularBounce[surfType]) < 0.0 )
            v14 = weapDef->perpendicularBounce[surfType];
        else
            v14 = grenadeBounceRestitutionMax->current.value;
        if ( (float)(v14 - v18) < 0.0 )
            v10 = v14;
        else
            v10 = v18;
        v18 = v10;
        v13 = (-(v10)) * dot;
        pos->trDelta[0] = (float)(v13 * *normal) + pos->trDelta[0];
        pos->trDelta[1] = (float)(v13 * normal[1]) + pos->trDelta[1];
        pos->trDelta[2] = (float)(v13 * normal[2]) + pos->trDelta[2];
LABEL_30:
        if ( ((LODWORD(pos->trDelta[0]) & 0x7F800000) == 0x7F800000
             || (LODWORD(pos->trDelta[1]) & 0x7F800000) == 0x7F800000
             || (LODWORD(pos->trDelta[2]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp",
                        511,
                        0,
                        "%s",
                        "!IS_NAN((pos->trDelta)[0]) && !IS_NAN((pos->trDelta)[1]) && !IS_NAN((pos->trDelta)[2])") )
        {
            __debugbreak();
        }
        return 0;
    }
    if ( rollSlideSpeed )
        *rollSlideSpeed = slideSpeed;
    if ( rollSlideDir )
    {
        *rollSlideDir = slideDir[0];
        rollSlideDir[1] = slideDir[1];
        rollSlideDir[2] = slideDir[2];
    }
    return 1;
}

bool __cdecl JavelinProjectile(gentity_s *ent, const WeaponDef *weapDef)
{
    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 770, 0, "%s", "ent") )
        __debugbreak();
    if ( !weapDef && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 771, 0, "%s", "weapDef") )
        __debugbreak();
    return ent->s.eType == 4 && weapDef->guidedMissileType == MISSILE_GUIDANCE_JAVELIN;
}

bool __cdecl isDud(gentity_s *ent, const WeaponDef *weapDef)
{
    return ForcedDud(ent) || GrenadeDud(ent, weapDef) || JavelinDud(ent, weapDef);
}

bool __cdecl GrenadeDud(gentity_s *ent, const WeaponDef *weapDef)
{
    bool dud; // [esp+7h] [ebp-1h]

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 751, 0, "%s", "ent") )
        __debugbreak();
    if ( !weapDef && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 752, 0, "%s", "weapDef") )
        __debugbreak();
    dud = 0;
    if ( weapDef->iProjectileActivateDist > 0 )
        return (float)weapDef->iProjectileActivateDist > ent->mover.aSpeed;
    return dud;
}

bool __cdecl JavelinDud(gentity_s *ent, const WeaponDef *weapDef)
{
    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 784, 0, "%s", "ent") )
        __debugbreak();
    if ( !weapDef && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 785, 0, "%s", "weapDef") )
        __debugbreak();
    return JavelinProjectile(ent, weapDef) && ent->missile.missile.stage == MISSILESTAGE_SOFTLAUNCH;
}

void __cdecl AttachBoltGrenade(gentity_s *ent, gentity_s *hitEnt, trace_t *trace, hitLocation_t hitLocation)
{
    unsigned int LowercaseString; // eax
    unsigned int tagName; // [esp+0h] [ebp-Ch] BYREF
    unsigned int boneName; // [esp+4h] [ebp-8h] BYREF
    int ownerIndex; // [esp+8h] [ebp-4h]

    ent->flags |= 0x1000u;
    ent->s.lerp.pos.trType = 0;
    ownerIndex = 0;

    //if ( EntHandle::isDefined(&ent->parent) && EntHandle::ent(&ent->parent)->client )
    if (ent->parent.isDefined() && ent->parent.ent()->client)
    {
        //ownerIndex = EntHandle::ent(&ent->parent)->client - level.clients;
        ownerIndex = ent->parent.ent()->client - level.clients;
    }

    if ( (unsigned int)ownerIndex >= level.maxclients
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp",
                    887,
                    0,
                    "ownerIndex doesn't index level.maxclients\n\t%i not in [0, %i)",
                    ownerIndex,
                    level.maxclients) )
    {
        __debugbreak();
    }
    AssignToSmallerType<unsigned char>(&ent->s.faction.iHeadIconTeam, ent->missile.team | (4 * ownerIndex));
    if ( hitEnt->client )
    {
        boneName = 0;
        MapHitLocationToRagdollBoneName(hitLocation, &boneName);
        G_EntLinkTo(ent, hitEnt, boneName);
        Scr_AddEntity(hitEnt, SCRIPTINSTANCE_SERVER);
        Scr_Notify(ent, scr_const.stuck_to_player, 1u);
    }
    else if ( ent->actor )
    {
        tagName = 0;
        MapHitLocationToRagdollBoneName(hitLocation, &tagName);
        if ( !G_EntLinkTo(ent, hitEnt, tagName) )
        {
            LowercaseString = SL_FindLowercaseString("j_knee_ri", SCRIPTINSTANCE_SERVER);
            G_EntLinkTo(ent, hitEnt, LowercaseString);
        }
    }
    else if ( hitEnt->scr_vehicle )
    {
        if ( !G_EntLinkTo(ent, hitEnt, trace->partName) )
            G_EntLinkTo(ent, hitEnt, 0);
    }
    else
    {
        G_EntLinkTo(ent, hitEnt, 0);
    }
    GScr_AddEntity(ent);
    Scr_Notify(hitEnt, scr_const.grenade_stuck, 1u);
    Scr_AddVector(trace->normal.vec.v, SCRIPTINSTANCE_SERVER);
    Scr_AddVector(ent->r.currentOrigin, SCRIPTINSTANCE_SERVER);
    Scr_Notify(ent, scr_const.stationary, 2u);
}

void __cdecl AttachMissileToEntity(
                gentity_s *missile,
                unsigned int entnum,
                unsigned int boneName,
                int hitLocation,
                float *velocity)
{
    float AttachedMissileOffset; // [esp+0h] [ebp-44h]
    const WeaponDef *weapDef; // [esp+1Ch] [ebp-28h]
    float dir[3]; // [esp+20h] [ebp-24h] BYREF
    float bonePos[3]; // [esp+2Ch] [ebp-18h] BYREF
    float offset[3]; // [esp+38h] [ebp-Ch] BYREF

    if ( !missile && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 974, 0, "%s", "missile") )
        __debugbreak();
    if ( entnum >= 0x400
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp",
                    975,
                    0,
                    "%s",
                    "entnum >= 0 && entnum < MAX_GENTITIES") )
    {
        __debugbreak();
    }
    if ( entnum <= 0x3FE && boneName )
    {
        weapDef = BG_GetWeaponDef(missile->s.weapon);
        if ( !weapDef
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 982, 0, "%s", "weapDef") )
        {
            __debugbreak();
        }
        if ( weapDef && weapDef->rotateType == WEAPROTATE_BLADE_ROTATE )
        {
            G_DObjGetWorldTagPos(&g_entities[entnum], boneName, bonePos);
            dir[0] = *velocity;
            dir[1] = velocity[1];
            dir[2] = 0.0f;
            Vec3Normalize(dir);
            missile->r.currentAngles[0] = 0.0f;
            missile->r.currentAngles[1] = 0.0f;
            missile->r.currentAngles[2] = 0.0f;
            missile->r.currentAngles[1] = vectoyaw(dir);
            missile->s.lerp.apos.trBase[0] = missile->r.currentAngles[0];
            missile->s.lerp.apos.trBase[1] = missile->r.currentAngles[1];
            missile->s.lerp.apos.trBase[2] = missile->r.currentAngles[2];
            if ( hitLocation )
            {
                //LODWORD(dir[0]) ^= _mask__NegFloat_;
                //LODWORD(dir[1]) ^= _mask__NegFloat_;
                //LODWORD(dir[2]) ^= _mask__NegFloat_;
                dir[0] = -dir[0];
                dir[1] = -dir[1];
                dir[2] = -dir[2];
                AttachedMissileOffset = GetAttachedMissileOffset(hitLocation);
                offset[0] = (float)(AttachedMissileOffset * dir[0]) + bonePos[0];
                offset[1] = (float)(AttachedMissileOffset * dir[1]) + bonePos[1];
                offset[2] = (float)(AttachedMissileOffset * dir[2]) + bonePos[2];
                G_SetOrigin(missile, offset);
            }
        }
    }
}

double __cdecl GetAttachedMissileOffset(int hitlocation)
{
    double result; // st7

    switch ( hitlocation )
    {
        case 1:
        case 2:
        case 3:
            result = 5.0;
            break;
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
        case 17:
            result = 2.0;
            break;
        default:
            result = 7.5;
            break;
    }
    return result;
}

int __cdecl StickMissile(
                gentity_s *ent,
                gentity_s *other,
                const WeaponDef *weapDef,
                trace_t *trace,
                hitLocation_t hitLocation,
                float *velocity)
{
    unsigned int LowercaseString; // eax
    unsigned int v8; // eax
    unsigned int tagName; // [esp+0h] [ebp-18h] BYREF
    team_t eTeam; // [esp+4h] [ebp-14h]
    unsigned int boneName; // [esp+8h] [ebp-10h] BYREF
    int team; // [esp+Ch] [ebp-Ch]
    int entnum; // [esp+10h] [ebp-8h]
    int isStuck; // [esp+14h] [ebp-4h]

    isStuck = 0;
    if ( other->client )
    {
        team = other->client->sess.cs.team;
        if ( team == ent->missile.team && !weapDef->timedDetonation && team )
            return isStuck;
        entnum = other->s.number;
        if ( other->client->ps.pm_type == 9
            && (int)g_scr_data.actorXAnimTrees[376 * other->client->ps.corpseIndex - 1495] >= 0 )
        {
            entnum = (int)g_scr_data.actorXAnimTrees[376 * other->client->ps.corpseIndex - 1495];
        }
        ent->flags |= 0x1000u;
        boneName = 0;
        MapHitLocationToRagdollBoneName(hitLocation, &boneName);
        AttachMissileToEntity(ent, entnum, boneName, hitLocation, velocity);
        if ( other->client->ps.pm_type == 9 )
            G_EntLinkTo(ent, &g_entities[entnum], boneName);
        else
            G_EntLinkTo(ent, other, boneName);
        ent->s.lerp.pos.trType = 0;
        Scr_AddEntity(other, SCRIPTINSTANCE_SERVER);
        Scr_Notify(ent, scr_const.stuck_to_player, 1u);
        return 1;
    }
    else if ( other->actor )
    {
        eTeam = other->actor->sentient->eTeam;
        if ( eTeam == ent->missile.team && !weapDef->timedDetonation && eTeam )
            return isStuck;
        ent->flags |= 0x1000u;
        tagName = 0;
        MapHitLocationToRagdollBoneName(hitLocation, &tagName);
        AttachMissileToEntity(ent, other->s.number, tagName, hitLocation, velocity);
        if ( !G_EntLinkTo(ent, other, tagName) )
        {
            LowercaseString = SL_FindLowercaseString("j_knee_ri", SCRIPTINSTANCE_SERVER);
            AttachMissileToEntity(ent, other->s.number, LowercaseString, 12, velocity);
            v8 = SL_FindLowercaseString("j_knee_ri", SCRIPTINSTANCE_SERVER);
            G_EntLinkTo(ent, other, v8);
        }
        ent->s.lerp.pos.trType = 0;
        return 1;
    }
    else if ( other->scr_vehicle && weapDef->stickiness != WEAPSTICKINESS_FLESH )
    {
        ent->flags |= 0x1000u;
        if ( !G_EntLinkTo(ent, other, trace->partName) )
            G_EntLinkTo(ent, other, 0);
        ent->s.lerp.pos.trType = 0;
        return 1;
    }
    return isStuck;
}

bool __cdecl isBounceProjectile(gentity_s *ent)
{
    const WeaponDef *weapDef; // [esp+0h] [ebp-4h]

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 1095, 0, "%s", "ent") )
        __debugbreak();
    if ( ent->s.eType != 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp",
                    1096,
                    0,
                    "%s",
                    "ent->s.eType == ET_MISSILE") )
    {
        __debugbreak();
    }
    if ( !ent->s.weapon
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 1097, 0, "%s", "ent->s.weapon") )
    {
        __debugbreak();
    }
    weapDef = BG_GetWeaponDef(ent->s.weapon);
    return (ent->s.lerp.eFlags & 0x1000000) != 0
            || weapDef->stickiness != WEAPSTICKINESS_ALL && weapDef->stickiness != WEAPSTICKINESS_FLESH;
}

void __cdecl createRetrieveableProjectile(
                gentity_s *ent,
                gentity_s *other,
                const WeaponDef *weapDef,
                trace_t *trace,
                hitLocation_t hitLocation,
                float *dir,
                float *endpos)
{
    gentity_s *v7; // eax
    unsigned int boneName; // [esp+4h] [ebp-14h] BYREF
    float angles[3]; // [esp+8h] [ebp-10h] BYREF
    gentity_s *target_ent; // [esp+14h] [ebp-4h]

    target_ent = 0;
    boneName = 0;
    if ( weapDef->stickiness )
    {
        MapHitLocationToRagdollBoneName(hitLocation, &boneName);
        if ( other->client )
        {
            target_ent = other;
            if (other->client->ps.pm_type == 9)
                target_ent = (gentity_s *)((char *)sv.gentities + *(_DWORD *)&g_scr_data.playerCorpseInfo[1504 * other->client->ps.corpseIndex + 4] * sv.gentitySize);
        }
        else if ( other->actor )
        {
            target_ent = other;
            if ( SV_DObjGetBoneIndex(other, boneName) < 0 )
                boneName = SL_FindLowercaseString("j_knee_ri", SCRIPTINSTANCE_SERVER);
        }
        else if ( other && other->s.number != 1022 )
        {
            target_ent = other;
        }
    }
    if ( boneName )
        Scr_AddConstString(boneName, SCRIPTINSTANCE_SERVER);
    else
        Scr_AddString((char *)"", SCRIPTINSTANCE_SERVER);
    if ( target_ent && target_ent->r.inuse )
        Scr_AddEntity(target_ent, SCRIPTINSTANCE_SERVER);
    else
        Scr_AddUndefined(SCRIPTINSTANCE_SERVER);
    //if ( EntHandle::isDefined(&ent->r.ownerNum) )
    if ( ent->r.ownerNum.isDefined() )
    {
        //v7 = EntHandle::ent(&ent->r.ownerNum);
        v7 = ent->r.ownerNum.ent();
        Scr_AddEntity(v7, SCRIPTINSTANCE_SERVER);
    }
    else
    {
        Scr_AddUndefined(SCRIPTINSTANCE_SERVER);
    }
    vectoangles(dir, angles);
    Scr_AddVector(angles, SCRIPTINSTANCE_SERVER);
    Scr_AddVector(trace->normal.vec.v, SCRIPTINSTANCE_SERVER);
    Scr_AddVector(endpos, SCRIPTINSTANCE_SERVER);
    Scr_Notify(ent, scr_const.stationary, 6u);
}

void __cdecl Missile_PenetrateGlass(
                trace_t *results,
                gentity_s *ent,
                const float *start,
                const float *end,
                int damage,
                bool predicted)
{
    int passEntityNum; // [esp+0h] [ebp-28h]
    gentity_s *v7; // [esp+4h] [ebp-24h]
    int contents; // [esp+8h] [ebp-20h]
    float vel[3]; // [esp+Ch] [ebp-1Ch] BYREF
    gentity_s *owner; // [esp+18h] [ebp-10h]
    gentity_s *hitEnt; // [esp+1Ch] [ebp-Ch]
    hitLocation_t hitLoc; // [esp+20h] [ebp-8h]
    unsigned __int16 hitEntId; // [esp+24h] [ebp-4h]

    if ( !results && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 2032, 0, "%s", "results") )
        __debugbreak();
    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 2033, 0, "%s", "ent") )
        __debugbreak();
    if ( !start && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 2034, 0, "%s", "start") )
        __debugbreak();
    if ( !end && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 2035, 0, "%s", "end") )
        __debugbreak();
    hitEntId = Trace_GetEntityHitId(results);
    if ( hitEntId < 0x3FEu )
    {
        hitEnt = &g_entities[hitEntId];
        if ( hitEnt->takedamage )
        {
            if ( damage )
            {
                BG_EvaluateTrajectoryDelta(&ent->s.lerp.pos, level.time, vel);
                if ( (float)((float)((float)(vel[0] * vel[0]) + (float)(vel[1] * vel[1])) + (float)(vel[2] * vel[2])) >= 1.0 )
                {
                    if ( !predicted )
                    {
                        //if ( EntHandle::isDefined(&ent->r.ownerNum) )
                        if (ent->r.ownerNum.isDefined())
                        {
                            //v7 = EntHandle::ent(&ent->r.ownerNum);
                            v7 = ent->r.ownerNum.ent();
                        }
                        else
                        {
                            v7 = 0;
                        }
                        owner = v7;
                        hitLoc = (hitLocation_t)results->partGroup;
                        if ( hitEnt->destructible && damage < 1000 )
                            damage = 1000;
                        G_Damage(
                            hitEnt,
                            ent,
                            owner,
                            vel,
                            ent->r.currentOrigin,
                            damage,
                            0,
                            16,
                            ent->s.weapon,
                            hitLoc,
                            results->modelIndex,
                            results->partName,
                            0);
                    }
                    contents = hitEnt->r.contents;
                    hitEnt->r.contents = 0;
                    //if ( EntHandle::isDefined(&ent->r.ownerNum) )
                    if ( ent->r.ownerNum.isDefined() )
                    {
                        //passEntityNum = EntHandle::entnum(&ent->r.ownerNum);
                        passEntityNum = ent->r.ownerNum.entnum();
                        G_MissileTrace(results, start, end, passEntityNum, ent->clipmask, ent->s.weapon);
                    }
                    else
                    {
                        G_MissileTrace(results, start, end, 1023, ent->clipmask, ent->s.weapon);
                    }
                    hitEnt->r.contents = contents;
                }
            }
        }
    }
}

void __cdecl DrawMissileDebug(float *start, float *end)
{
    float color[4]; // [esp+0h] [ebp-10h] BYREF

    color[0] = 1.0f;
    color[1] = 0.0f;
    color[2] = 0.0f;
    color[3] = 1.0f;
    if ( g_debugBullets->current.integer >= 5 || missileDebugDraw->current.enabled )
        CG_DebugLine(start, end, color, 1, 500);
}

void __cdecl RunMissile_Destabilize(gentity_s *missile)
{
    double v1; // st7
    double v2; // st7
    float iProjectileSpeed; // [esp+20h] [ebp-40h]
    float perturbationMax; // [esp+30h] [ebp-30h]
    const WeaponDef *weaponDef; // [esp+34h] [ebp-2Ch]
    float newAngleAccel[3]; // [esp+38h] [ebp-28h]
    float direction[3]; // [esp+44h] [ebp-1Ch] BYREF
    int axis; // [esp+50h] [ebp-10h]
    float newAPos[3]; // [esp+54h] [ebp-Ch] BYREF
    int savedregs; // [esp+60h] [ebp+0h] BYREF

    if ( !missile && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 2428, 0, "%s", "missile") )
        __debugbreak();
    if ( missile->s.eType != 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp",
                    2429,
                    0,
                    "%s",
                    "missile->s.eType == ET_MISSILE") )
    {
        __debugbreak();
    }
    if ( (missile->flags & 0x20000) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp",
                    2430,
                    0,
                    "%s",
                    "!(missile->flags & FL_STABLE_MISSILES)") )
    {
        __debugbreak();
    }
    weaponDef = BG_GetWeaponDef(missile->s.weapon);
    if ( missile->s.lerp.pos.trTime + (int)missile->mover.aDecelTime >= level.time )
    {
        if ( (missile->flags & 0x10000) == 0 )
            return;
    }
    else
    {
        perturbationMax = RunMissile_GetPerturbation(weaponDef->destabilizationCurvatureMax);
        if ( (missile->flags & 0x10000) != 0 )
        {
            for ( axis = 0; axis < 3; ++axis )
            {
                v1 = G_flrand(0.0, 1.0);
                newAngleAccel[axis] = v1 * perturbationMax;
            }
            if (missile->mover.pos3[0] < 0.0)
            {
                //LODWORD(newAngleAccel[0]) ^= _mask__NegFloat_;
                (newAngleAccel[0]) = -(newAngleAccel[0]);
            }
            if (missile->mover.pos2[2] > 0.0)
            {
                //LODWORD(newAngleAccel[1]) ^= _mask__NegFloat_;
                (newAngleAccel[1]) = -(newAngleAccel[1]);
            }
        }
        else
        {
            for ( axis = 0; axis < 3; ++axis )
            {
                v2 = G_flrand(-1.0, 1.0);
                newAngleAccel[axis] = v2 * perturbationMax;
            }
        }
        *(spawner_ent_t *)((char *)&missile->spawner + 44) = *(spawner_ent_t *)newAngleAccel;
        missile->mover.pos3[1] = newAngleAccel[2];
        missile->s.lerp.pos.trTime = level.time;
        missile->mover.aDecelTime = weaponDef->destabilizationRateTime * 1000.0;
        missile->flags |= 0x10000u;
    }
    newAPos[0] = (float)(0.05f * missile->mover.pos2[2]) + missile->s.lerp.apos.trBase[0];
    newAPos[1] = (float)(0.05f * missile->mover.pos3[0]) + missile->s.lerp.apos.trBase[1];
    newAPos[2] = (float)(0.05f * missile->mover.pos3[1]) + missile->s.lerp.apos.trBase[2];
    G_SetAngle(missile, newAPos);
    AngleVectors(newAPos, direction, 0, 0);
    iProjectileSpeed = (float)weaponDef->iProjectileSpeed;
    missile->s.lerp.pos.trDelta[0] = iProjectileSpeed * direction[0];
    missile->s.lerp.pos.trDelta[1] = iProjectileSpeed * direction[1];
    missile->s.lerp.pos.trDelta[2] = iProjectileSpeed * direction[2];
    if ( ((LODWORD(missile->s.lerp.pos.trDelta[0]) & 0x7F800000) == 0x7F800000
         || (LODWORD(missile->s.lerp.pos.trDelta[1]) & 0x7F800000) == 0x7F800000
         || (LODWORD(missile->s.lerp.pos.trDelta[2]) & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp",
                    2472,
                    0,
                    "%s",
                    "!IS_NAN((missile->s.lerp.pos.trDelta)[0]) && !IS_NAN((missile->s.lerp.pos.trDelta)[1]) && !IS_NAN((missile->s."
                    "lerp.pos.trDelta)[2])") )
    {
        __debugbreak();
    }
    missile->s.lerp.pos.trBase[0] = missile->r.currentOrigin[0];
    missile->s.lerp.pos.trBase[1] = missile->r.currentOrigin[1];
    missile->s.lerp.pos.trBase[2] = missile->r.currentOrigin[2];
    missile->s.lerp.pos.trType = 1;
    missile->s.lerp.apos.trType = 1;
    Missile_ApplyAttractorsRepulsors(missile);
}

double __cdecl RunMissile_GetPerturbation(float destabilizationCurvatureMax)
{
    if ( (destabilizationCurvatureMax >= 1000000000.0 || destabilizationCurvatureMax < 0.0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp",
                    2089,
                    0,
                    "%s\n\t(destabilizationCurvatureMax) = %g",
                    "(destabilizationCurvatureMax < 1000000000.0f && destabilizationCurvatureMax >= 0.0f)",
                    destabilizationCurvatureMax) )
    {
        __debugbreak();
    }
    return destabilizationCurvatureMax;
}

void    Missile_ApplyAttractorsRepulsors(gentity_s *missile)
{
    double v2; // xmm0_8
    float v3; // xmm0_4
    float v4; // xmm0_4
    float v5; // xmm0_4
    long double v6; // [esp-24h] [ebp-ACh]
    float iProjectileSpeed; // [esp-24h] [ebp-ACh]
    float angleToAttractor; // [esp+8h] [ebp-80h]
    float angleToAttractora; // [esp+8h] [ebp-80h]
    float force; // [esp+Ch] [ebp-7Ch]
    float totalDist; // [esp+10h] [ebp-78h]
    float perpDist; // [esp+14h] [ebp-74h] BYREF
    float perpDir[3]; // [esp+18h] [ebp-70h] BYREF
    float perpDelta[3]; // [esp+24h] [ebp-64h]
    float forwardDist; // [esp+30h] [ebp-58h]
    float delta[5]; // [esp+34h] [ebp-54h] BYREF
    gentity_s *ent; // [esp+48h] [ebp-40h]
    float attractorOrigin[4]; // [esp+4Ch] [ebp-3Ch]
    unsigned int attractorIndex; // [esp+5Ch] [ebp-2Ch]
    float forceVector[3]; // [esp+60h] [ebp-28h] BYREF
    float forwardDir[3]; // [esp+6Ch] [ebp-1Ch] BYREF
    const WeaponDef *weaponDef; // [esp+78h] [ebp-10h]
    //_UNKNOWN *v24; // [esp+7Ch] [ebp-Ch]
    //gentity_s *missilea; // [esp+80h] [ebp-8h]
    //int vars0; // [esp+88h] [ebp+0h]
    //
    //v24 = a1;
    //missilea = (gentity_s *)vars0;
    weaponDef = BG_GetWeaponDef(missile->s.weapon);
    if (Vec3NormalizeTo(missile->s.lerp.pos.trDelta, forwardDir) < 0.0000099999997)
        return;
    memset(forceVector, 0, sizeof(forceVector));
    for (attractorIndex = 0; attractorIndex < 0x20; ++attractorIndex)
    {
        if (attrGlob.attractors[attractorIndex].inUse)
        {
            if (attrGlob.attractors[attractorIndex].entnum == 1023)
            {
                LODWORD(attractorOrigin[3]) = 28 * attractorIndex + 65863520;
                attractorOrigin[0] = attrGlob.attractors[attractorIndex].origin[0];
                attractorOrigin[1] = attrGlob.attractors[attractorIndex].origin[1];
                attractorOrigin[2] = attrGlob.attractors[attractorIndex].origin[2];
            }
            else
            {
                if (attrGlob.attractors[attractorIndex].entnum >= 1024
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp",
                        2355,
                        0,
                        "%s",
                        "attrGlob.attractors[attractorIndex].entnum < MAX_GENTITIES"))
                {
                    __debugbreak();
                }
                ent = &g_entities[attrGlob.attractors[attractorIndex].entnum];
                //LODWORD(delta[4]) = ent->r.currentOrigin;
                attractorOrigin[0] = ent->r.currentOrigin[0];
                attractorOrigin[1] = ent->r.currentOrigin[1];
                attractorOrigin[2] = ent->r.currentOrigin[2];
            }
            ///LODWORD(delta[3]) = missile->s.lerp.pos.trBase;
            delta[0] = attractorOrigin[0] - missile->s.lerp.pos.trBase[0];
            delta[1] = attractorOrigin[1] - missile->s.lerp.pos.trBase[1];
            delta[2] = attractorOrigin[2] - missile->s.lerp.pos.trBase[2];
            forwardDist = (float)((float)(delta[0] * forwardDir[0]) + (float)(delta[1] * forwardDir[1])) + (float)(delta[2] * forwardDir[2]);
            if (forwardDist > 0.0)
            {
                (perpDelta[2]) = -(forwardDist);
                perpDir[2] =   (float)((-(forwardDist)) * forwardDir[0]) + delta[0];
                perpDelta[0] = (float)((-(forwardDist)) * forwardDir[1]) + delta[1];
                perpDelta[1] = (float)((-(forwardDist)) * forwardDir[2]) + delta[2];
                totalDist = Vec3NormalizeTo(&perpDir[2], &perpDist);
                if (totalDist < 0.0000099999997)
                {
                    if (attrGlob.attractors[attractorIndex].isAttractor)
                        continue;
                    perpDist = 0.0f;
                    perpDir[0] = 0.0f;
                    perpDir[1] = -1.0f;
                }
                if (!attrGlob.attractors[attractorIndex].isAttractor && perpDir[1] > 0.0)
                {
                    perpDist = 0.0f;
                    perpDir[0] = 0.0f;
                    perpDir[1] = -1.0f;
                    totalDist = 0.0f;
                }
                force = Vec3Length(delta);
                if (force <= attrGlob.attractors[attractorIndex].maxDist)
                {
                    if (attrGlob.attractors[attractorIndex].maxDist <= 0.0
                        && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp",
                            2387,
                            0,
                            "%s",
                            "attrGlob.attractors[attractorIndex].maxDist > 0"))
                    {
                        __debugbreak();
                    }
                    angleToAttractor = (float)(1.0 - (float)(force / attrGlob.attractors[attractorIndex].maxDist))
                        * attrGlob.attractors[attractorIndex].strength;
                    v2 = (float)((fabs(totalDist)) / forwardDist);
                    //__libm_sse2_atan(v6);
                    v3 = atan(v2);
                    if (attrGlob.attractors[attractorIndex].isAttractor)
                        v4 = v3 * 0.63662028;
                    else
                        v4 = (float)(v3 * 0.63662028) - 1.0;
                    angleToAttractora = angleToAttractor * v4;
                    if (attrGlob.attractors[attractorIndex].isAttractor)
                    {
                        if ((float)(angleToAttractora
                            - (float)((float)((float)((float)weaponDef->iProjectileSpeed * totalDist) / forwardDist) * 20.0)) < 0.0)
                            v5 = angleToAttractora;
                        else
                            v5 = (float)((float)((float)weaponDef->iProjectileSpeed * totalDist) / forwardDist) * 20.0;
                        angleToAttractora = v5;
                    }
                    forceVector[0] = (float)(angleToAttractora * perpDist) + forceVector[0];
                    forceVector[1] = (float)(angleToAttractora * perpDir[0]) + forceVector[1];
                    forceVector[2] = (float)(angleToAttractora * perpDir[1]) + forceVector[2];
                }
            }
        }
    }
    if (forceVector[0] != 0.0 || forceVector[1] != 0.0 || forceVector[2] != 0.0)
    {
        missile->s.lerp.pos.trDelta[0] = (float)(0.05f * forceVector[0]) + missile->s.lerp.pos.trDelta[0];
        missile->s.lerp.pos.trDelta[1] = (float)(0.05f * forceVector[1]) + missile->s.lerp.pos.trDelta[1];
        missile->s.lerp.pos.trDelta[2] = (float)(0.05f * forceVector[2]) + missile->s.lerp.pos.trDelta[2];
        Vec3NormalizeTo(missile->s.lerp.pos.trDelta, forwardDir);
        iProjectileSpeed = (float)weaponDef->iProjectileSpeed;
        missile->s.lerp.pos.trDelta[0] = iProjectileSpeed * forwardDir[0];
        missile->s.lerp.pos.trDelta[1] = iProjectileSpeed * forwardDir[1];
        missile->s.lerp.pos.trDelta[2] = iProjectileSpeed * forwardDir[2];
        vectoangles(forwardDir, missile->s.lerp.apos.trBase);
    }
}

void __cdecl RunMissile_CreateWaterSplash(const gentity_s *missile, const float *hitPos, const trace_t *trace)
{
    gentity_s *tent; // [esp+Ch] [ebp-10h]
    float reflect[3]; // [esp+10h] [ebp-Ch] BYREF

    if ( !missile && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 2486, 0, "%s", "missile") )
        __debugbreak();
    if ( !trace && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 2487, 0, "%s", "trace") )
        __debugbreak();
    Vec3NormalizeTo(missile->s.lerp.pos.trDelta, reflect);
    if ( reflect[2] < 0.0 )
        reflect[2] = reflect[2] * -1.0;
    tent = G_TempEntity(hitPos, EV_GRENADE_BOUNCE);
    tent->s.eventParm = DirToByte(trace->normal.vec.v);
    tent->s.un1.scale = 0;
    tent->s.surfType = (int)(0x3F00000 & trace->sflags) >> 20;
    AssignToSmallerType<short>(&tent->s.otherEntityNum, missile->s.number);
    tent->s.weapon = missile->s.weapon;
}

void __cdecl RunMissile_BroadcastActorEvents(gentity_s *missile)
{
    int methodOfDeath; // [esp+4h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+8h] [ebp-4h]

    if ( !missile && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 2508, 0, "%s", "missile") )
        __debugbreak();
    methodOfDeath = entityHandlers[missile->handler].methodOfDeath;
    weapDef = BG_GetWeaponDef(missile->s.weapon);
    if ( !weapDef && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 2512, 0, "%s", "weapDef") )
        __debugbreak();
    if ( methodOfDeath == 3 )
    {
        if ( weapDef->offhandClass )
        {
            if ( level.time - missile->item[0].ammoCount >= 250 )
            {
                Actor_BroadcastPointEvent(missile, AI_EV_GRENADE_PING, -1, missile->r.currentOrigin, 0.0);
                missile->item[0].ammoCount = level.time;
            }
        }
    }
    else
    {
        Actor_BroadcastPointEvent(missile, AI_EV_PROJECTILE_PING, -1, missile->r.currentOrigin, 0.0);
    }
}

void __cdecl MissileTrajectory(gentity_s *ent, float *result)
{
    const float *MapCenter; // eax
    float v3; // [esp+48h] [ebp-48h]
    float v4; // [esp+54h] [ebp-3Ch]
    float dir[3]; // [esp+60h] [ebp-30h] BYREF
    bool keepRoll; // [esp+6Fh] [ebp-21h]
    float roll; // [esp+70h] [ebp-20h]
    bool validOrigin; // [esp+77h] [ebp-19h]
    float dbgStart[3]; // [esp+78h] [ebp-18h] BYREF
    float forwardSpeed; // [esp+84h] [ebp-Ch]
    const WeaponDef *weapDef; // [esp+88h] [ebp-8h]
    float accel; // [esp+8Ch] [ebp-4h]

    validOrigin = 1;
    keepRoll = 0;
    roll = 0.0f;
    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 3279, 0, "%s", "ent") )
        __debugbreak();
    if ( !result && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 3280, 0, "%s", "result") )
        __debugbreak();
    weapDef = BG_GetWeaponDef(ent->s.weapon);
    if ( !weapDef && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 3283, 0, "%s", "weapDef") )
        __debugbreak();
    MapCenter = (const float *)SV_GetMapCenter();
    validOrigin = BG_ValidateOrigin(ent->r.currentOrigin, 16, 17, MapCenter);
    if ( validOrigin )
    {
        if ( level.time <= ent->s.lerp.u.actor.actorNum
            || ent->s.lerp.pos.trType == 3
            || ent->handler != 12
            || weapDef->weapType == WEAPTYPE_BOMB
            || weapDef->guidedMissileType == MISSILE_GUIDANCE_BALLISTIC )
        {
            BG_EvaluateTrajectory(&ent->s.lerp.pos, level.time, result);
        }
        else
        {
            if ( weapDef->timeToAccelerate > 0.0 )
            {
                AngleVectors(ent->r.currentAngles, dir, 0, 0);
                forwardSpeed = (float)((float)(dir[0] * ent->s.lerp.pos.trDelta[0])
                                                         + (float)(dir[1] * ent->s.lerp.pos.trDelta[1]))
                                         + (float)(dir[2] * ent->s.lerp.pos.trDelta[2]);
                if ( forwardSpeed < (float)weapDef->iProjectileSpeed )
                {
                    accel = (float)weapDef->iProjectileSpeed / weapDef->timeToAccelerate;
                    v3 = accel * 0.05f;
                    ent->s.lerp.pos.trDelta[0] = (float)((float)(accel * 0.05f) * dir[0]) + ent->s.lerp.pos.trDelta[0];
                    ent->s.lerp.pos.trDelta[1] = (float)(v3 * dir[1]) + ent->s.lerp.pos.trDelta[1];
                    ent->s.lerp.pos.trDelta[2] = (float)(v3 * dir[2]) + ent->s.lerp.pos.trDelta[2];
                }
                else
                {
                    v4 = (float)weapDef->iProjectileSpeed - forwardSpeed;
                    ent->s.lerp.pos.trDelta[0] = (float)(v4 * dir[0]) + ent->s.lerp.pos.trDelta[0];
                    ent->s.lerp.pos.trDelta[1] = (float)(v4 * dir[1]) + ent->s.lerp.pos.trDelta[1];
                    ent->s.lerp.pos.trDelta[2] = (float)(v4 * dir[2]) + ent->s.lerp.pos.trDelta[2];
                }
            }
            if ( weapDef->projectileCurvature > 0.0 )
            {
                ent->s.lerp.pos.trDelta[0] = (float)(0.05f * ent->mover.pos2[2]) + ent->s.lerp.pos.trDelta[0];
                ent->s.lerp.pos.trDelta[1] = (float)(0.05f * ent->mover.pos3[0]) + ent->s.lerp.pos.trDelta[1];
                ent->s.lerp.pos.trDelta[2] = (float)(0.05f * ent->mover.pos3[1]) + ent->s.lerp.pos.trDelta[2];
            }
            if ( missileDebugDraw->current.enabled )
            {
                dbgStart[0] = ent->r.currentOrigin[0];
                dbgStart[1] = ent->r.currentOrigin[1];
                dbgStart[2] = ent->r.currentOrigin[2];
            }
            GuidedMissileSteering(ent);
            if ( weapDef->guidedMissileType == MISSILE_GUIDANCE_TVGUIDED )
            {
                ent->s.lerp.pos.trBase[0] = (float)(0.05f * ent->s.lerp.pos.trDelta[0]) + ent->s.lerp.pos.trBase[0];
                ent->s.lerp.pos.trBase[1] = (float)(0.05f * ent->s.lerp.pos.trDelta[1]) + ent->s.lerp.pos.trBase[1];
                ent->s.lerp.pos.trBase[2] = (float)(0.05f * ent->s.lerp.pos.trDelta[2]) + ent->s.lerp.pos.trBase[2];
                roll = ent->s.lerp.apos.trBase[2];
                keepRoll = 1;
                vectoangles(ent->s.lerp.pos.trDelta, ent->s.lerp.apos.trBase);
                ent->s.lerp.apos.trBase[2] = roll;
                G_SetAngle(ent, ent->s.lerp.apos.trBase);
                ent->s.lerp.apos.trType = 1;
                MissileTrajectoryClientControlled(ent, result);
            }
            if ( weapDef->guidedMissileType != MISSILE_GUIDANCE_JAVELIN || ent->missile.missile.stage )
            {
                ent->s.lerp.pos.trBase[0] = (float)(0.05f * ent->s.lerp.pos.trDelta[0]) + ent->s.lerp.pos.trBase[0];
                ent->s.lerp.pos.trBase[1] = (float)(0.05f * ent->s.lerp.pos.trDelta[1]) + ent->s.lerp.pos.trBase[1];
                ent->s.lerp.pos.trBase[2] = (float)(0.05f * ent->s.lerp.pos.trDelta[2]) + ent->s.lerp.pos.trBase[2];
                *result = ent->s.lerp.pos.trBase[0];
                result[1] = ent->s.lerp.pos.trBase[1];
                result[2] = ent->s.lerp.pos.trBase[2];
            }
            else
            {
                BG_EvaluateTrajectory(&ent->s.lerp.pos, level.time, result);
            }
            if ( missileDebugDraw->current.enabled )
            {
                if ( weapDef->guidedMissileType == MISSILE_GUIDANCE_JAVELIN && ent->missile.missile.stage == MISSILESTAGE_ASCENT )
                    CG_DebugLine(dbgStart, result, colorOrange, 0, 200);
                else
                    CG_DebugLine(dbgStart, result, colorRed, 0, 200);
            }
            if ( ent->s.lerp.apos.trType == 1 && MissileIsReadyForSteering(ent) )
            {
                vectoangles(ent->s.lerp.pos.trDelta, ent->r.currentAngles);
                if ( keepRoll )
                    ent->r.currentAngles[2] = roll;
                G_SetAngle(ent, ent->r.currentAngles);
                ent->s.lerp.apos.trType = 1;
            }
        }
    }
    else
    {
        Com_Printf(
            15,
            "Removing missile at position    ( %f, %f, %f )\n",
            ent->r.currentOrigin[0],
            ent->r.currentOrigin[1],
            ent->r.currentOrigin[2]);
        Scr_Notify(ent, scr_const.death, 0);
        G_FreeEntity(ent);
    }
}

bool __cdecl MissileIsReadyForSteering(gentity_s *ent)
{
    return (float)(BG_GetWeaponDef(ent->s.weapon)->timeToAccelerate
                             - (float)((float)(level.time - ent->s.lerp.pos.trTime) * 0.001)) <= 0.0;
}

void __cdecl MissileTrajectoryClientControlled(gentity_s *ent, float *result)
{
    float dirOrig[3]; // [esp+10h] [ebp-20h] BYREF
    gentity_s *owner; // [esp+1Ch] [ebp-14h]
    float targetSpeed; // [esp+20h] [ebp-10h]
    float speed; // [esp+24h] [ebp-Ch]
    const WeaponDef *weapDef; // [esp+28h] [ebp-8h]
    usercmd_s *cmd; // [esp+2Ch] [ebp-4h]

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 3057, 0, "%s", "ent") )
        __debugbreak();
    if ( !result && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 3058, 0, "%s", "result") )
        __debugbreak();
    ent->clipmask &= ~0x800u;
    //owner = EntHandle::ent(&ent->parent);
    owner = ent->parent.ent();
    if ( !owner->client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 3066, 0, "%s", "owner->client") )
    {
        __debugbreak();
    }
    weapDef = BG_GetWeaponDef(ent->s.weapon);
    cmd = &owner->client->sess.cmd;
    AngleVectors(ent->r.currentAngles, dirOrig, 0, 0);
    speed = (float)((float)(dirOrig[0] * ent->s.lerp.pos.trDelta[0]) + (float)(dirOrig[1] * ent->s.lerp.pos.trDelta[1]))
                + (float)(dirOrig[2] * ent->s.lerp.pos.trDelta[2]);
    //if ( bitarray<51>::testBit(&cmd->button_bits, 0xFu) && (ent->missile.flags & 1) != 0 )
    if ( cmd->button_bits.testBit(0xFu) && (ent->missile.flags & 1) != 0 )
    {
        targetSpeed = (float)weapDef->iProjectileSpeed
                                + (float)((float)weapDef->iProjectileSpeed * missileTVGuidedBoost->current.value);
        ent->missile.flags |= 4u;
        if ( targetSpeed > speed )
            speed = (float)missileTVGuidedBoostSpeedUp->current.integer * 0.05f;
        if ( speed >= targetSpeed )
            speed = targetSpeed - speed;
    }
    else
    {
        //if ( !bitarray<51>::testBit(&cmd->button_bits, 0xFu) && (ent->missile.flags & 1) == 0 )
        if ( !cmd->button_bits.testBit(0xFu) && (ent->missile.flags & 1) == 0 )
            ent->missile.flags |= 1u;
        targetSpeed = (float)weapDef->iProjectileSpeed;
        ent->missile.flags &= ~4u;
        if ( speed < targetSpeed )
            speed = targetSpeed - speed;
        else
            speed = (float)missileTVGuidedBoostSpeedDown->current.integer * 0.05f;
    }
    ent->s.lerp.pos.trDelta[0] = (float)(speed * dirOrig[0]) + ent->s.lerp.pos.trDelta[0];
    ent->s.lerp.pos.trDelta[1] = (float)(speed * dirOrig[1]) + ent->s.lerp.pos.trDelta[1];
    ent->s.lerp.pos.trDelta[2] = (float)(speed * dirOrig[2]) + ent->s.lerp.pos.trDelta[2];
    *result = ent->s.lerp.pos.trBase[0];
    result[1] = ent->s.lerp.pos.trBase[1];
    result[2] = ent->s.lerp.pos.trBase[2];
}

void __cdecl GuidedMissileSteering(gentity_s *ent)
{
    float v1; // [esp+14h] [ebp-124h]
    float value; // [esp+18h] [ebp-120h]
    gentity_s *v3; // [esp+54h] [ebp-E4h]
    float v5; // [esp+94h] [ebp-A4h]
    float v6; // [esp+98h] [ebp-A0h]
    float v7; // [esp+9Ch] [ebp-9Ch]
    float origin[3]; // [esp+A0h] [ebp-98h] BYREF
    float deltaRoll; // [esp+ACh] [ebp-8Ch]
    gentity_s *owner; // [esp+B0h] [ebp-88h]
    float baseAngle[3]; // [esp+B4h] [ebp-84h] BYREF
    usercmd_s *usercmd; // [esp+C0h] [ebp-78h]
    float desiredAngle; // [esp+C4h] [ebp-74h]
    float farPoint[3]; // [esp+C8h] [ebp-70h]
    bool usingGamepad; // [esp+D7h] [ebp-61h]
    float rollAccel; // [esp+D8h] [ebp-60h]
    float nearPoint[3]; // [esp+DCh] [ebp-5Ch]
    float currentHorzSpeed; // [esp+E8h] [ebp-50h]
    float currentRight[2]; // [esp+ECh] [ebp-4Ch] BYREF
    float toTarget[3]; // [esp+F4h] [ebp-44h]
    float currentHorzDir[2]; // [esp+100h] [ebp-38h] BYREF
    float steer[3]; // [esp+108h] [ebp-30h] BYREF
    float targetPos[3]; // [esp+114h] [ebp-24h] BYREF
    bool tvGuided; // [esp+123h] [ebp-15h]
    const WeaponDef *weapDef; // [esp+124h] [ebp-14h]
    float maxAccel; // [esp+128h] [ebp-10h]
    float toTargetRelative[3]; // [esp+12Ch] [ebp-Ch] BYREF

    weapDef = BG_GetWeaponDef(ent->s.weapon);
    if ( weapDef->guidedMissileType
        && (weapDef->guidedMissileType == MISSILE_GUIDANCE_WIREGUIDED
         || weapDef->guidedMissileType == MISSILE_GUIDANCE_TVGUIDED
         //|| EntHandle::isDefined(&ent->missileTargetEnt))
         || ent->missileTargetEnt.isDefined())
        && (weapDef->guidedMissileType == MISSILE_GUIDANCE_WIREGUIDED
         || weapDef->guidedMissileType == MISSILE_GUIDANCE_TVGUIDED
         || IsMissileLockedOn(ent))
        && MissileIsReadyForSteering(ent) )
    {
        if ( weapDef->guidedMissileType == MISSILE_GUIDANCE_JAVELIN )
        {
            JavelinSteering(ent, weapDef);
        }
        else
        {
            currentHorzSpeed = Vec2NormalizeTo(ent->s.lerp.pos.trDelta, currentHorzDir);
            currentRight[0] = currentHorzDir[1];
            currentRight[1] = -currentHorzDir[0];
            tvGuided = 0;
            maxAccel = 0.0f;
            if ( weapDef->guidedMissileType == MISSILE_GUIDANCE_TVGUIDED )
            {
                iassert(ent->r.ownerNum.isDefined());

                //owner = EntHandle::ent(&ent->r.ownerNum);
                owner = ent->r.ownerNum.ent();
                usercmd = &owner->client->sess.cmd;
                usingGamepad = 0;
                desiredAngle = 0.0f;
                deltaRoll = 0.0f;
                rollAccel = 0.0f;
                baseAngle[0] = ent->r.currentAngles[0];
                baseAngle[1] = ent->r.currentAngles[1];
                baseAngle[2] = ent->r.currentAngles[2];
                if ( usercmd->pitchmove > missileTVGuidedStickDeadzone->current.integer
                    || usercmd->pitchmove < -missileTVGuidedStickDeadzone->current.integer )
                {
                    baseAngle[0] = (float)((float)((float)usercmd->pitchmove / 126.0) * missileTVGuidedTurnRate->current.value)
                                             + baseAngle[0];
                    usingGamepad = 1;
                }
                if ( usercmd->yawmove <= missileTVGuidedStickDeadzone->current.integer
                    && usercmd->yawmove >= -missileTVGuidedStickDeadzone->current.integer )
                {
                    rollAccel = missileTVGuidedFlatteningRollAccel->current.value;
                }
                else
                {
                    baseAngle[1] = (float)((float)((float)usercmd->yawmove / 126.0) * missileTVGuidedTurnRate->current.value)
                                             + baseAngle[1];
                    usingGamepad = 1;
                    maxAccel = (float)((float)((float)usercmd->yawmove / 126.0) * weapDef->maxSteeringAccel) * -1.0;
                    tvGuided = 1;
                    desiredAngle = (float)((float)usercmd->yawmove / 126.0) * -(missileTVGuidedMaxRollAngle->current.value);
                    rollAccel = missileTVGuidedTurningRollAccel->current.value;
                }
                if ( ent->s.lerp.apos.trBase[2] <= desiredAngle )
                {
                    if ( desiredAngle > ent->s.lerp.apos.trBase[2] )
                    {
                        deltaRoll = ent->s.lerp.apos.trBase[2] - desiredAngle;
                        ent->s.lerp.apos.trBase[2] = (float)((float)((float)(deltaRoll * 0.05f) * -1.0) * rollAccel)
                                                                             + ent->s.lerp.apos.trBase[2];
                    }
                }
                else
                {
                    deltaRoll = desiredAngle - ent->s.lerp.apos.trBase[2];
                    ent->s.lerp.apos.trBase[2] = (float)((float)(deltaRoll * 0.05f) * rollAccel)
                                                                         + ent->s.lerp.apos.trBase[2];
                }
                if ( !usingGamepad )
                {
                    baseAngle[0] = (float)usercmd->pitchmove + baseAngle[0];
                    baseAngle[1] = (float)usercmd->yawmove + baseAngle[1];
                    ent->s.lerp.apos.trBase[2] = (float)usercmd->yawmove + ent->s.lerp.apos.trBase[2];
                    if ( (float)(missileTVGuidedMaxRollAngle->current.value - ent->r.currentAngles[2]) < 0.0 )
                        value = missileTVGuidedMaxRollAngle->current.value;
                    else
                        value = ent->r.currentAngles[2];
                    ent->s.lerp.apos.trBase[2] = value;
                    if ( (float)(ent->r.currentAngles[2] - -(missileTVGuidedMaxRollAngle->current.value)) < 0.0 )
                        (v1) = -missileTVGuidedMaxRollAngle->current.value;
                    else
                        v1 = ent->r.currentAngles[2];
                    ent->s.lerp.apos.trBase[2] = v1;
                }
                AngleVectors(baseAngle, targetPos, 0, 0);
                farPoint[0] = 300.0f;
                farPoint[1] = 300.0f;
                farPoint[2] = 300.0f;
                targetPos[0] = targetPos[0] * 300.0;
                targetPos[1] = targetPos[1] * 300.0;
                targetPos[2] = targetPos[2] * 300.0;
                nearPoint[0] = ent->r.currentOrigin[0];
                nearPoint[1] = ent->r.currentOrigin[1];
                nearPoint[2] = ent->r.currentOrigin[2];
                targetPos[0] = targetPos[0] + nearPoint[0];
                targetPos[1] = targetPos[1] + nearPoint[1];
                targetPos[2] = targetPos[2] + nearPoint[2];
            }
            else if ( weapDef->guidedMissileType == MISSILE_GUIDANCE_WIREGUIDED )
            {
                iassert(ent->r.ownerNum.isDefined());

                //v3 = EntHandle::ent(&ent->r.ownerNum);
                v3 = ent->r.ownerNum.ent();
                AngleVectors(v3->s.lerp.apos.trBase, targetPos, 0, 0);
                v5 = 30000.0f;
                v6 = 30000.0f;
                v7 = 30000.0f;
                targetPos[0] = targetPos[0] * 30000.0;
                targetPos[1] = targetPos[1] * 30000.0;
                targetPos[2] = targetPos[2] * 30000.0;
                G_GetPlayerViewOrigin(&v3->client->ps, origin);
                targetPos[0] = targetPos[0] + origin[0];
                targetPos[1] = targetPos[1] + origin[1];
                targetPos[2] = targetPos[2] + origin[2];

                trace_t tr; // [esp+58h] [ebp-E0h] BYREF
                G_LocationalTraceAllowChildren(&tr, origin, targetPos, v3->s.number, 0x280E833, 0);
                if ( tr.fraction < 1.0 )
                    Vec3Lerp(origin, targetPos, tr.fraction, targetPos);
            }
            else
            {
                GetTargetPosition(ent, targetPos);
            }
            if ( missileDebugDraw->current.enabled )
            {
                CG_DebugLine(ent->s.lerp.pos.trBase, targetPos, colorGreen, 0, 200);
                G_DebugCircle(targetPos, 10.0, colorOrange, 0, 0, 500);
            }
            toTarget[0] = targetPos[0] - ent->s.lerp.pos.trBase[0];
            toTarget[1] = targetPos[1] - ent->s.lerp.pos.trBase[1];
            toTarget[2] = targetPos[2] - ent->s.lerp.pos.trBase[2];
            toTargetRelative[0] = (float)(toTarget[0] * currentHorzDir[0]) + (float)(toTarget[1] * currentHorzDir[1]);
            toTargetRelative[1] = (float)(toTarget[0] * currentRight[0]) + (float)(toTarget[1] * currentRight[1]);
            toTargetRelative[2] = toTarget[2];
            memset(steer, 0, sizeof(steer));
            if ( weapDef->guidedMissileType == MISSILE_GUIDANCE_TVGUIDED )
                MissileTVGuidedHorzSteerToTarget(ent, currentRight, toTargetRelative, steer, maxAccel);
            else
                MissileHorzSteerToTarget(ent, currentRight, toTargetRelative, currentHorzSpeed, steer, tvGuided);
            MissileVerticalSteering(ent, toTargetRelative, currentHorzSpeed, steer);
            ent->s.lerp.pos.trDelta[0] = (float)(0.05f * steer[0]) + ent->s.lerp.pos.trDelta[0];
            ent->s.lerp.pos.trDelta[1] = (float)(0.05f * steer[1]) + ent->s.lerp.pos.trDelta[1];
            ent->s.lerp.pos.trDelta[2] = (float)(0.05f * steer[2]) + ent->s.lerp.pos.trDelta[2];
        }
    }
}

char __cdecl IsMissileLockedOn(gentity_s *ent)
{
    //if ( !EntHandle::isDefined(&ent->missileTargetEnt) )
    if ( !ent->missileTargetEnt.isDefined() )
        return 0;

    //if ( !EntHandle::ent(&ent->missileTargetEnt)->r.inuse
    if ( !ent->missileTargetEnt.ent()->r.inuse
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 2541, 0, "%s", "target->r.inuse") )
    {
        __debugbreak();
    }
    return 1;
}

void __cdecl GetTargetPosition(gentity_s *ent, float *result)
{
    float v2; // [esp+4h] [ebp-3Ch]
    float v3; // [esp+Ch] [ebp-34h]
    float v4; // [esp+10h] [ebp-30h]
    gentity_s *target; // [esp+18h] [ebp-28h]
    float axis[3][3]; // [esp+1Ch] [ebp-24h] BYREF

    iassert(ent);
    iassert(ent->s.eType == ET_MISSILE);
    iassert(ent->missileTargetEnt.isDefined());
    
    //target = EntHandle::ent(&ent->missileTargetEnt);
    target = ent->missileTargetEnt.ent();
    AnglesToAxis(target->r.currentAngles, axis);
    *result = target->r.currentOrigin[0];
    result[1] = target->r.currentOrigin[1];
    result[2] = target->r.currentOrigin[2];
    v4 = ent->mover.pos3[2];
    *result = (float)(v4 * axis[0][0]) + *result;
    result[1] = (float)(v4 * axis[0][1]) + result[1];
    result[2] = (float)(v4 * axis[0][2]) + result[2];
    v3 = ent->mover.apos1[0];
    *result = (float)(v3 * axis[1][0]) + *result;
    result[1] = (float)(v3 * axis[1][1]) + result[1];
    result[2] = (float)(v3 * axis[1][2]) + result[2];
    v2 = ent->mover.apos1[1];
    *result = (float)(v2 * axis[2][0]) + *result;
    result[1] = (float)(v2 * axis[2][1]) + result[1];
    result[2] = (float)(v2 * axis[2][2]) + result[2];
}

void __cdecl MissileTVGuidedHorzSteerToTarget(
                gentity_s *ent,
                const float *currentRight,
                const float *toTargetRelative,
                float *steer,
                float accel)
{
    *steer = accel * *currentRight;
    steer[1] = accel * currentRight[1];
}

void __cdecl MissileHorzSteerToTarget(
                gentity_s *ent,
                const float *currentRight,
                const float *toTargetRelative,
                float currentHorzSpeed,
                float *steer,
                bool tvGuided)
{
    float v6; // xmm0_4
    gentity_s *v7; // eax
    float v8; // [esp+0h] [ebp-48h]
    float v9; // [esp+14h] [ebp-34h]
    float maxSteeringAccel; // [esp+18h] [ebp-30h]
    float v11; // [esp+24h] [ebp-24h]
    float dist; // [esp+28h] [ebp-20h]
    float targetPos[3]; // [esp+2Ch] [ebp-1Ch] BYREF
    float radius; // [esp+38h] [ebp-10h]
    float tightestRadius; // [esp+3Ch] [ebp-Ch]
    const WeaponDef *weapDef; // [esp+40h] [ebp-8h]
    float accel; // [esp+44h] [ebp-4h]
    int savedregs; // [esp+48h] [ebp+0h] BYREF

    weapDef = BG_GetWeaponDef(ent->s.weapon);
    if ( toTargetRelative[1] == 0.0 )
        radius = FLT_MAX;
    else
        radius = (float)((float)(*toTargetRelative * *toTargetRelative) + (float)(toTargetRelative[1] * toTargetRelative[1]))
                     / (float)(2.0 * toTargetRelative[1]);
    if ( weapDef->maxSteeringAccel <= 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp",
                    2590,
                    0,
                    "%s",
                    "weapDef->maxSteeringAccel > 0") )
    {
        __debugbreak();
    }
    tightestRadius = (float)((float)weapDef->iProjectileSpeed * (float)weapDef->iProjectileSpeed)
                                 / weapDef->maxSteeringAccel;
    if ( *toTargetRelative <= 0.0 || tvGuided )
    {
        if ( (float)(tightestRadius + 60.0) <= fabs(radius) )
        {
            if ( toTargetRelative[1] <= 0.0 )
            {
                v9 = -weapDef->maxSteeringAccel;
                *steer = v9 * *currentRight;
                v6 = v9 * currentRight[1];
            }
            else
            {
                maxSteeringAccel = weapDef->maxSteeringAccel;
                *steer = maxSteeringAccel * *currentRight;
                v6 = maxSteeringAccel * currentRight[1];
            }
            steer[1] = v6;
            //if ( EntHandle::isDefined(&ent->missileTargetEnt) )
            if ( ent->missileTargetEnt.isDefined() )
            {
                //v7 = EntHandle::ent(&ent->missileTargetEnt);
                v7 = ent->missileTargetEnt.ent();
                if ( BG_GetWeaponDef(v7->s.weapon)->guidedMissileType == MISSILE_GUIDANCE_TVGUIDED )
                {
                    GetTargetPosition(ent, targetPos);
                    dist = Vec3Distance(ent->r.currentOrigin, targetPos);
                    if ( (float)weapDef->iExplosionRadius > dist )
                        G_ExplodeMissile(ent);
                }
            }
        }
    }
    else if ( toTargetRelative[1] != 0.0 )
    {
        if ( tightestRadius > fabs(radius) )
        {
            if ( radius <= 0.0 )
                radius = -tightestRadius;
            else
                radius = tightestRadius;
        }
        if ( radius == 0.0
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 2606, 0, "%s", "radius != 0") )
        {
            __debugbreak();
        }
        accel = (float)((float)(2.0 * currentHorzSpeed) * currentHorzSpeed) / radius;
        if ( (float)(accel - weapDef->maxSteeringAccel) < 0.0 )
            v11 = accel;
        else
            v11 = weapDef->maxSteeringAccel;
        if ( (float)((-(weapDef->maxSteeringAccel)) - accel) < 0.0 )
            v8 = v11;
        else
            v8 = -weapDef->maxSteeringAccel;
        accel = v8;
        *steer = v8 * *currentRight;
        steer[1] = accel * currentRight[1];
    }
}

void __cdecl MissileVerticalSteering(
                gentity_s *ent,
                const float *toTargetRelative,
                float currentHorzSpeed,
                float *steer)
{
    int v4; // [esp+10h] [ebp-3Ch]
    float v5; // [esp+18h] [ebp-34h]
    int integer; // [esp+24h] [ebp-28h]
    float toTargetHorzRelativeDir[2]; // [esp+34h] [ebp-18h] BYREF
    float horzDistToTarg; // [esp+3Ch] [ebp-10h]
    float minTimeToTarg; // [esp+40h] [ebp-Ch]
    float maxVertSpeedAtMaxAccel; // [esp+44h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+48h] [ebp-4h]

    weapDef = BG_GetWeaponDef(ent->s.weapon);
    horzDistToTarg = Vec2NormalizeTo(toTargetRelative, toTargetHorzRelativeDir);
    steer[2] = 0.0f;
    if ( horzDistToTarg != 0.0 )
    {
        switch ( weapDef->guidedMissileType )
        {
            case MISSILE_GUIDANCE_SIDEWINDER:
            case MISSILE_GUIDANCE_WIREGUIDED:
            case MISSILE_GUIDANCE_TVGUIDED:
                goto $LN10_39;
            case MISSILE_GUIDANCE_HELLFIRE:
                if ( ent->s.lerp.pos.trDuration )
                {
$LN10_39:
                    MissileVerticalSteerToTarget(
                        ent,
                        toTargetHorzRelativeDir,
                        horzDistToTarg,
                        toTargetRelative[2],
                        currentHorzSpeed,
                        steer);
                }
                else
                {
                    if ( *toTargetRelative <= 0.0 || currentHorzSpeed == 0.0 )
                    {
                        if ( !weapDef->iProjectileSpeed
                            && !Assert_MyHandler(
                                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp",
                                        2691,
                                        0,
                                        "%s",
                                        "weapDef->iProjectileSpeed != 0") )
                        {
                            __debugbreak();
                        }
                        minTimeToTarg = horzDistToTarg / (float)weapDef->iProjectileSpeed;
                    }
                    else
                    {
                        minTimeToTarg = *toTargetRelative / (float)weapDef->iProjectileSpeed;
                    }
                    if ( minTimeToTarg <= 0.0
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp",
                                    2695,
                                    0,
                                    "%s",
                                    "minTimeToTarg > 0") )
                    {
                        __debugbreak();
                    }
                    maxVertSpeedAtMaxAccel = toTargetRelative[2] / minTimeToTarg;
                    maxVertSpeedAtMaxAccel = (float)((float)(0.5 * weapDef->maxSteeringAccel) * minTimeToTarg)
                                                                 + maxVertSpeedAtMaxAccel;
                    maxVertSpeedAtMaxAccel = maxVertSpeedAtMaxAccel * 0.89999998;
                    if ( maxVertSpeedAtMaxAccel > (float)(currentHorzSpeed * missileHellfireMaxSlope->current.value) )
                        maxVertSpeedAtMaxAccel = currentHorzSpeed * missileHellfireMaxSlope->current.value;
                    if ( maxVertSpeedAtMaxAccel <= ent->s.lerp.pos.trDelta[2] )
                    {
                        ent->s.lerp.pos.trDuration = 1;
                    }
                    else
                    {
                        steer[2] = (float)(maxVertSpeedAtMaxAccel - ent->s.lerp.pos.trDelta[2]) * 20.0;
                        v5 = steer[2];
                        if ( (float)(v5 - missileHellfireUpAccel->current.value) < 0.0 )
                            integer = *((unsigned int *)steer + 2);
                        else
                            integer = missileHellfireUpAccel->current.integer;
                        if ((float)((-(weapDef->maxSteeringAccel)) - v5) < 0.0)
                        {
                            v4 = integer;
                        }
                        else
                        {
                            //v4 = LODWORD(weapDef->maxSteeringAccel) ^ _mask__NegFloat_;
                            v4 = -(weapDef->maxSteeringAccel);
                        }
                        *((unsigned int *)steer + 2) = v4;
                    }
                }
                break;
            default:
                return;
        }
    }
}

void __cdecl MissileVerticalSteerToTarget(
                gentity_s *ent,
                const float *toTargetHorzRelDir,
                float horzDistToTarg,
                float vertDistToTarg,
                float currentHorzSpeed,
                float *steer)
{
    float v6; // [esp+0h] [ebp-20h]
    float v7; // [esp+4h] [ebp-1Ch]
    float maxSteeringAccel; // [esp+10h] [ebp-10h]
    const WeaponDef *weapDef; // [esp+1Ch] [ebp-4h]

    if ( *toTargetHorzRelDir >= 0.0 )
    {
        weapDef = BG_GetWeaponDef(ent->s.weapon);
        steer[2] = (float)((float)((float)(fabs(*toTargetHorzRelDir * currentHorzSpeed)
                                                                         * vertDistToTarg)
                                                         / horzDistToTarg)
                                         - ent->s.lerp.pos.trDelta[2])
                         * 20.0;
        v7 = steer[2];
        if ( (float)(v7 - weapDef->maxSteeringAccel) < 0.0 )
            maxSteeringAccel = steer[2];
        else
            maxSteeringAccel = weapDef->maxSteeringAccel;
        if ( (float)((-(weapDef->maxSteeringAccel)) - v7) < 0.0 )
            v6 = maxSteeringAccel;
        else
            v6 = -weapDef->maxSteeringAccel;
        steer[2] = v6;
    }
}

void __cdecl JavelinSteering(gentity_s *ent, const WeaponDef *weapDef)
{
    const char *v2; // [esp+20h] [ebp-64h]
    float v[6]; // [esp+3Ch] [ebp-48h] BYREF
    float *trBase; // [esp+54h] [ebp-30h]
    float *currentOrigin; // [esp+58h] [ebp-2Ch]
    float limit; // [esp+5Ch] [ebp-28h]
    float distance2D; // [esp+60h] [ebp-24h]
    float height; // [esp+64h] [ebp-20h]
    float distance3D; // [esp+68h] [ebp-1Ch]
    float toTarget[3]; // [esp+6Ch] [ebp-18h] BYREF
    float targetPos[3]; // [esp+78h] [ebp-Ch] BYREF

    iassert(ent);
    iassert(ent->missileTargetEnt.isDefined());
    
    if ( ent->missile.missile.stage == MISSILESTAGE_SOFTLAUNCH )
    {
        if ( level.time - ent->s.lerp.u.actor.actorNum < weapDef->projIgnitionDelay )
        {
            if ( missileDebugText->current.enabled )
                Com_Printf(15, "Javelin: softlaunch\n");
            return;
        }
        ent->missile.missile.stage = MISSILESTAGE_ASCENT;
        ent->s.lerp.pos.trType = 1;
        trBase = ent->s.lerp.pos.trBase;
        currentOrigin = ent->r.currentOrigin;
        ent->s.lerp.pos.trBase[0] = ent->r.currentOrigin[0];
        trBase[1] = currentOrigin[1];
        trBase[2] = currentOrigin[2];
    }
    GetTargetPosition(ent, targetPos);
    if ( ent->missile.missile.stage == MISSILESTAGE_ASCENT )
    {
        if ( JavelinClimbEnd(ent, targetPos) )
            ent->missile.missile.stage = MISSILESTAGE_DESCENT;
        else
            JavelinClimbOffset(ent, targetPos);
    }
    //LODWORD(v[5]) = ent->s.lerp.pos.trBase;
    toTarget[0] = targetPos[0] - ent->s.lerp.pos.trBase[0];
    toTarget[1] = targetPos[1] - ent->s.lerp.pos.trBase[1];
    toTarget[2] = targetPos[2] - ent->s.lerp.pos.trBase[2];
    Vec3Normalize(toTarget);
    JavelinRotateVelocity(ent, ent->s.lerp.pos.trDelta, toTarget, ent->s.lerp.pos.trDelta);
    if ( missileDebugText->current.enabled )
    {
        height = (float)(ent->s.lerp.pos.trBase[2] - targetPos[2]) - ent->mover.apos1[1];
        limit = JavelinClimbCeiling(ent);
        v[0] = targetPos[0] - ent->s.lerp.pos.trBase[0];
        v[1] = targetPos[1] - ent->s.lerp.pos.trBase[1];
        distance2D = Vec2Length(v);
        distance3D = Vec3Distance(ent->s.lerp.pos.trBase, targetPos);
        if ( ent->missile.missile.stage == MISSILESTAGE_ASCENT )
            v2 = "A";
        else
            v2 = "D";
        Com_Printf(15, "Jav:%s h:%.0f/%.0f dist 2d:%.0f 3d:%.0f\n", v2, height, limit, distance2D, distance3D);
    }
}

void __cdecl JavelinClimbOffset(gentity_s *ent, float *targetPos)
{
    gentity_s *v2; // [esp+0h] [ebp-2Ch]
    float value; // [esp+4h] [ebp-28h]
    gentity_s *target; // [esp+20h] [ebp-Ch]
    float ownerDir[2]; // [esp+24h] [ebp-8h] BYREF

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 2801, 0, "%s", "ent") )
        __debugbreak();
    if ( ent->s.eType != 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp",
                    2802,
                    0,
                    "%s",
                    "ent->s.eType == ET_MISSILE") )
    {
        __debugbreak();
    }
    if ( ent->missile.missile.flightMode )
    {
        if ( ent->missile.missile.flightMode != MISSILEFLIGHTMODE_DIRECT
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp",
                        2810,
                        0,
                        "%s",
                        "ent->missile.missile.flightMode == MISSILEFLIGHTMODE_DIRECT") )
        {
            __debugbreak();
        }
        targetPos[2] = targetPos[2] + missileJavClimbHeightDirect->current.value;


        //if ( EntHandle::isDefined(&ent->r.ownerNum) )
        if (ent->r.ownerNum.isDefined())
        {
            //v2 = EntHandle::ent(&ent->r.ownerNum);
            v2 = ent->r.ownerNum.ent();
        }
        else
        {
            v2 = &g_entities[1023];
        }

        iassert(ent->missileTargetEnt.isDefined());
        
        //target = EntHandle::ent(&ent->missileTargetEnt);
        target = ent->missileTargetEnt.ent();
        ownerDir[0] = v2->s.lerp.pos.trBase[0] - target->s.lerp.pos.trBase[0];
        ownerDir[1] = v2->s.lerp.pos.trBase[1] - target->s.lerp.pos.trBase[1];
        Vec2Normalize(ownerDir);
        value = missileJavClimbToOwner->current.value;
        ownerDir[0] = value * ownerDir[0];
        ownerDir[1] = value * ownerDir[1];
        *targetPos = *targetPos + ownerDir[0];
        targetPos[1] = targetPos[1] + ownerDir[1];
    }
    else
    {
        targetPos[2] = targetPos[2] + missileJavClimbHeightTop->current.value;
    }
}

void __cdecl JavelinRotateVelocity(gentity_s *ent, const float *currentVel, const float *targetDir, float *resultVel)
{
    float currentDir[3]; // [esp+Ch] [ebp-20h] BYREF
    float len; // [esp+18h] [ebp-14h]
    float resultDir[3]; // [esp+1Ch] [ebp-10h] BYREF
    float turnDiff; // [esp+28h] [ebp-4h]

    len = Vec3NormalizeTo(currentVel, currentDir);
    turnDiff = JavelinRotateDir(ent, currentDir, targetDir, resultDir);
    if ( turnDiff < 30.0 )
    {
        if ( ent->missile.missile.stage == MISSILESTAGE_ASCENT || currentVel[2] > 0.0 )
        {
            len = (float)(missileJavAccelClimb->current.value * 0.05f) + len;
            if ( len > missileJavSpeedLimitClimb->current.value )
                len = missileJavSpeedLimitClimb->current.value;
        }
        else
        {
            if ( ent->missile.missile.stage != MISSILESTAGE_DESCENT
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp",
                            2932,
                            0,
                            "%s",
                            "ent->missile.missile.stage == MISSILESTAGE_DESCENT") )
            {
                __debugbreak();
            }
            len = (float)(missileJavAccelDescend->current.value * 0.05f) + len;
            if ( len > missileJavSpeedLimitDescend->current.value )
                len = missileJavSpeedLimitDescend->current.value;
        }
    }
    len = (float)(1.0 - (float)((float)(turnDiff / 180.0) * missileJavTurnDecel->current.value)) * len;
    *resultVel = len * resultDir[0];
    resultVel[1] = len * resultDir[1];
    resultVel[2] = len * resultDir[2];
}

double __cdecl JavelinRotateDir(gentity_s *ent, const float *currentDir, const float *targetDir, float *resultDir)
{
    float end[3]; // [esp+34h] [ebp-4Ch] BYREF
    float maxDPS; // [esp+40h] [ebp-40h]
    float targetQuat[4]; // [esp+44h] [ebp-3Ch] BYREF
    float frac; // [esp+54h] [ebp-2Ch]
    float resultQuat[4]; // [esp+58h] [ebp-28h] BYREF
    float targetDPS; // [esp+68h] [ebp-18h]
    float currentQuat[4]; // [esp+6Ch] [ebp-14h] BYREF
    float dot; // [esp+7Ch] [ebp-4h]

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 2858, 0, "%s", "ent") )
        __debugbreak();
    maxDPS = JavelinMaxDPS(ent);
    dot = (float)(1.0
                            - (float)((float)((float)((float)((float)(*targetDir * *currentDir) + (float)(targetDir[1] * currentDir[1]))
                                                                            + (float)(targetDir[2] * currentDir[2]))
                                                            + 1.0)
                                            * 0.5))
            * 180.0;
    if ( dot > 0.1 )
    {
        targetDPS = dot / 0.05f;
        if ( maxDPS <= (float)(dot / 0.05f) )
        {
            frac = maxDPS / targetDPS;
            if ( missileDebugText->current.enabled )
                Com_Printf(15, "dot:%.2f frac:%.2f =%.0f/%.0f ", dot, frac, maxDPS, targetDPS);
            VecToQuat(currentDir, currentQuat);
            VecToQuat(targetDir, targetQuat);
            QuatSlerp(currentQuat, targetQuat, frac, resultQuat);
            Vec4Normalize(resultQuat);
            UnitQuatToForward(resultQuat, resultDir);
            if ( missileDebugDraw->current.enabled )
            {
                end[0] = (float)(100.0 * *currentDir) + ent->s.lerp.pos.trBase[0];
                end[1] = (float)(100.0 * currentDir[1]) + ent->s.lerp.pos.trBase[1];
                end[2] = (float)(100.0 * currentDir[2]) + ent->s.lerp.pos.trBase[2];
                CG_DebugLine(ent->s.lerp.pos.trBase, end, colorBlue, 0, 200);
                end[0] = (float)(100.0 * *targetDir) + ent->s.lerp.pos.trBase[0];
                end[1] = (float)(100.0 * targetDir[1]) + ent->s.lerp.pos.trBase[1];
                end[2] = (float)(100.0 * targetDir[2]) + ent->s.lerp.pos.trBase[2];
                CG_DebugLine(ent->s.lerp.pos.trBase, end, colorLtGrey, 0, 200);
                end[0] = (float)(100.0 * *resultDir) + ent->s.lerp.pos.trBase[0];
                end[1] = (float)(100.0 * resultDir[1]) + ent->s.lerp.pos.trBase[1];
                end[2] = (float)(100.0 * resultDir[2]) + ent->s.lerp.pos.trBase[2];
                CG_DebugLine(ent->s.lerp.pos.trBase, end, colorGreen, 0, 200);
            }
            return dot;
        }
        else
        {
            *resultDir = *targetDir;
            resultDir[1] = targetDir[1];
            resultDir[2] = targetDir[2];
            if ( missileDebugText->current.enabled )
                Com_Printf(15, "dot:%.2f (%.0f > %.0f) ", dot, maxDPS, targetDPS);
            return 0.0;
        }
    }
    else
    {
        *resultDir = *targetDir;
        resultDir[1] = targetDir[1];
        resultDir[2] = targetDir[2];
        if ( missileDebugText->current.enabled )
            Com_Printf(15, "dot:%.2f ", dot);
        return 0.0;
    }
}

double __cdecl JavelinMaxDPS(gentity_s *ent)
{
    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 2826, 0, "%s", "ent") )
        __debugbreak();
    if ( ent->s.eType != 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp",
                    2827,
                    0,
                    "%s",
                    "ent->s.eType == ET_MISSILE") )
    {
        __debugbreak();
    }
    if ( ent->missile.missile.flightMode == MISSILEFLIGHTMODE_TOP )
        return missileJavTurnRateTop->current.value;
    if ( ent->missile.missile.flightMode != MISSILEFLIGHTMODE_DIRECT
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp",
                    2831,
                    0,
                    "%s",
                    "ent->missile.missile.flightMode == MISSILEFLIGHTMODE_DIRECT") )
    {
        __debugbreak();
    }
    return missileJavTurnRateDirect->current.value;
}

void __cdecl VecToQuat(const float *vec, float *quat)
{
    float angles[3]; // [esp+0h] [ebp-30h] BYREF
    float axis[3][3]; // [esp+Ch] [ebp-24h] BYREF

    vectoangles(vec, angles);
    AnglesToAxis(angles, axis);
    AxisToQuat(axis, quat);
}

double __cdecl JavelinClimbCeiling(gentity_s *ent)
{
    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 2947, 0, "%s", "ent") )
        __debugbreak();
    if ( ent->s.eType != 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp",
                    2948,
                    0,
                    "%s",
                    "ent->s.eType == ET_MISSILE") )
    {
        __debugbreak();
    }
    if ( ent->missile.missile.flightMode == MISSILEFLIGHTMODE_DIRECT )
        return missileJavClimbCeilingDirect->current.value;
    if ( ent->missile.missile.flightMode )
    {
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp",
                        2956,
                        0,
                        "%s",
                        "ent->missile.missile.flightMode == MISSILEFLIGHTMODE_TOP") )
            __debugbreak();
    }
    return missileJavClimbCeilingTop->current.value;
}

bool __cdecl JavelinClimbEnd(gentity_s *ent, const float *targetPos)
{
    int savedregs; // [esp+0h] [ebp+0h] BYREF

    if ( !JavelinClimbIsAboveCeiling(ent, targetPos) )
        return 0;
    if ( JavelinClimbExceededAngle(ent, targetPos) )
        return 1;
    return JavelinClimbWithinDistance(ent, targetPos) != 0;
}

char JavelinClimbExceededAngle(gentity_s *ent, const float *targetPos)
{
    double v3; // xmm0_8
    long double v5; // [esp-10h] [ebp-3Ch]
    float toTarget[3]; // [esp+Ch] [ebp-20h] BYREF
    float value; // [esp+18h] [ebp-14h]
    float currentHorzDir[2]; // [esp+1Ch] [ebp-10h]
    //float limit; // [esp+24h] [ebp-8h]
    //float retaddr; // [esp+2Ch] [ebp+0h]

    //currentHorzDir[1] = a1;
    //limit = retaddr;
    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 2743, 0, "%s", "ent") )
        __debugbreak();
    if ( ent->s.eType != 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp",
                    2744,
                    0,
                    "%s",
                    "ent->s.eType == ET_MISSILE") )
    {
        __debugbreak();
    }
    currentHorzDir[0] = ent->mover.apos2[0];
    if ( LODWORD(currentHorzDir[0]) )
    {
        if ( ent->missile.missile.flightMode != MISSILEFLIGHTMODE_DIRECT
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp",
                        2752,
                        0,
                        "%s",
                        "ent->missile.missile.flightMode == MISSILEFLIGHTMODE_DIRECT") )
        {
            __debugbreak();
        }
        value = missileJavClimbAngleDirect->current.value;
    }
    else
    {
        value = missileJavClimbAngleTop->current.value;
    }
    Vec2NormalizeTo(ent->s.lerp.pos.trDelta, &toTarget[1]);
    v3 = (float)((float)((float)((float)(*targetPos - ent->s.lerp.pos.trBase[0]) * toTarget[1])
                                         + (float)((float)(targetPos[1] - ent->s.lerp.pos.trBase[1]) * toTarget[2]))
                         / (float)(targetPos[2] - ent->s.lerp.pos.trBase[2]));
    //__libm_sse2_atan(v5);
    v3 = atan(v3);
    if ( value <= fabs(*(float *)&v3 * 57.295776) )
        return 0;
    if ( missileDebugText->current.enabled )
        Com_Printf(15, "Javelin: *** Exceeded climb angle ***\n");
    return 1;
}

char __cdecl JavelinClimbWithinDistance(gentity_s *ent, const float *targetPos)
{
    float v[2]; // [esp+8h] [ebp-Ch] BYREF
    float distance; // [esp+10h] [ebp-4h]

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 2779, 0, "%s", "ent") )
        __debugbreak();
    if ( ent->s.eType != 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp",
                    2780,
                    0,
                    "%s",
                    "ent->s.eType == ET_MISSILE") )
    {
        __debugbreak();
    }
    v[0] = *targetPos - ent->s.lerp.pos.trBase[0];
    v[1] = targetPos[1] - ent->s.lerp.pos.trBase[1];
    distance = Vec2Length(v);
    if ( distance >= 400.0 )
        return 0;
    if ( missileDebugText->current.enabled )
        Com_Printf(15, "Javelin: *** Exceeded climb distance ***\n");
    return 1;
}

bool __cdecl JavelinClimbIsAboveCeiling(gentity_s *ent, const float *targetPos)
{
    float limit; // [esp+0h] [ebp-8h]
    float height; // [esp+4h] [ebp-4h]

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 2967, 0, "%s", "ent") )
        __debugbreak();
    if ( ent->s.eType != 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp",
                    2968,
                    0,
                    "%s",
                    "ent->s.eType == ET_MISSILE") )
    {
        __debugbreak();
    }
    height = (float)(ent->s.lerp.pos.trBase[2] - targetPos[2]) - ent->mover.apos1[1];
    limit = JavelinClimbCeiling(ent);
    return height > limit;
}

bool __cdecl CheckForMissileClientControlledDetonation(gentity_s *ent)
{
    gentity_s *owner; // [esp+0h] [ebp-Ch]
    const WeaponDef *weapDef; // [esp+4h] [ebp-8h]
    usercmd_s *cmd; // [esp+8h] [ebp-4h]

    iassert(ent);
    iassert(ent->s.eType == ET_MISSILE);
    
    //owner = EntHandle::ent(&ent->parent);
    owner = ent->parent.ent();

    iassert(owner->client);
    weapDef = BG_GetWeaponDef(ent->s.weapon);
    cmd = &owner->client->sess.cmd;
    if ( weapDef->guidedMissileType != MISSILE_GUIDANCE_TVGUIDED )
        return 0;

    //if ( !bitarray<51>::testBit(&owner->client->sess.cmd.button_bits, 0) && (ent->missile.flags & 2) == 0 )
    if ( !owner->client->sess.cmd.button_bits.testBit(0) && (ent->missile.flags & 2) == 0 )
        ent->missile.flags |= 2u;

    //return bitarray<51>::testBit(&cmd->button_bits, 0) && (ent->missile.flags & 2) != 0;
    return cmd->button_bits.testBit(0) && (ent->missile.flags & 2) != 0;
}

char __cdecl UpdateGuidedMissileFuelTime(gentity_s *ent, bool initTimer)
{
    gentity_s *owner; // [esp+0h] [ebp-Ch]
    const WeaponDef *weapDef; // [esp+4h] [ebp-8h]
    playerState_s *ps; // [esp+8h] [ebp-4h]
    int savedregs; // [esp+Ch] [ebp+0h] BYREF

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 3442, 0, "%s", "ent") )
        __debugbreak();
    if ( ent->s.eType != 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp",
                    3443,
                    0,
                    "%s",
                    "ent->s.eType == ET_MISSILE") )
    {
        __debugbreak();
    }
    //owner = EntHandle::ent(&ent->parent);
    owner = ent->parent.ent();
    if ( !owner->client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 3446, 0, "%s", "owner->client") )
    {
        __debugbreak();
    }
    ps = &owner->client->ps;
    weapDef = BG_GetWeaponDef(ent->s.weapon);
    if ( !ps->weapon )
        return 1;
    if ( initTimer )
        ent->missile.timeAlive = level.time;
    if ( (ent->missile.flags & 4) != 0 )
        BG_AddFuelTankTime(ps, ps->weapon, 2 * (level.time - ent->missile.timeAlive));
    else
        BG_AddFuelTankTime(ps, ps->weapon, level.time - ent->missile.timeAlive);
    if ( (float)BG_GetFuelTankTime(ps, ps->weapon) <= (float)(weapDef->projLifetime * 1000.0) )
    {
        ent->missile.timeAlive = level.time;
        return 1;
    }
    else
    {
        BG_SetFuelTankTime(ps, ps->weapon, (int)(float)(weapDef->projLifetime * 1000.0));
        G_ExplodeMissile(ent);
        return 0;
    }
}

unsigned __int8 __cdecl G_GetGrenadeTrType(gentity_s *grenade)
{
    //if ( EntHandle::isDefined(&grenade->parent) && EntHandle::ent(&grenade->parent)->s.eType == 17 )
    if ( grenade->parent.isDefined() && grenade->parent.ent()->s.eType == 17)
        return 7;
    else
        return 6;
}

void __cdecl G_RunMissile(gentity_s *ent)
{
    bool doAntilag; // [esp+3h] [ebp-329h]
    AntilagClientStore antilagClients; // [esp+4h] [ebp-328h] BYREF
    const WeaponDef *weapDef; // [esp+328h] [ebp-4h]

    weapDef = BG_GetWeaponDef(ent->s.weapon);
    if ( !weapDef && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 3867, 0, "%s", "weapDef") )
        __debugbreak();
    doAntilag = weapDef->bUseAntiLagRewind;
    if ( doAntilag )
        G_AntiLagRewindClientPos(ent->missile.antilagTimeOffset + level.time, &antilagClients);
    G_RunMissileInternal(ent);
    if ( doAntilag )
        G_AntiLag_RestoreClientPos(&antilagClients);
}

int __cdecl G_PredictMissile(gentity_s *ent, int duration, float *vLandPos, int allowBounce, int *timeAtRest)
{
    int v6; // [esp+10h] [ebp-3B8h]
    int v7; // [esp+14h] [ebp-3B4h]
    int passEntityNum; // [esp+18h] [ebp-3B0h]
    trajectory_t pos; // [esp+38h] [ebp-390h] BYREF
    float origin[3]; // [esp+5Ch] [ebp-36Ch] BYREF
    gentity_s backupEnt; // [esp+68h] [ebp-360h] BYREF
    int time; // [esp+360h] [ebp-68h]
    float endpos[3]; // [esp+364h] [ebp-64h] BYREF
    trace_t tr; // [esp+370h] [ebp-58h] BYREF
    float org[3]; // [esp+3ACh] [ebp-1Ch] BYREF
    const WeaponDef *weapDef; // [esp+3B8h] [ebp-10h]
    float traceStart[3]; // [esp+3BCh] [ebp-Ch] BYREF

    memcpy(&pos, &ent->s.lerp.pos, sizeof(pos));
    BG_EvaluateTrajectory(&pos, level.time - 50, org);
    memcpy(&backupEnt, ent, sizeof(backupEnt));
    *timeAtRest = ent->nextthink;
    weapDef = BG_GetWeaponDef(ent->s.weapon);
    if ( !weapDef && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 3978, 0, "%s", "weapDef") )
        __debugbreak();
    *vLandPos = 0.0f;
    vLandPos[1] = 0.0f;
    vLandPos[2] = 0.0f;
    for ( time = level.time; time < duration + level.time; time += 50 )
    {
        BG_EvaluateTrajectory(&pos, time, origin);
        //if ( EntHandle::isDefined(&ent->r.ownerNum) )
        if ( ent->r.ownerNum.isDefined() )
        {
            //passEntityNum = EntHandle::entnum(&ent->r.ownerNum);
            passEntityNum = ent->r.ownerNum.entnum();
            G_MissileTrace(&tr, org, origin, passEntityNum, ent->clipmask, ent->s.weapon);
        }
        else
        {
            G_MissileTrace(&tr, org, origin, 1023, ent->clipmask, ent->s.weapon);
        }
        if ( tr.startsolid )
        {
            if ( time != level.time )
                goto LABEL_36;
            org[0] = origin[0];
            org[1] = origin[1];
            org[2] = origin[2];
        }
        else
        {
            if ((tr.sflags & 0x3F00000) == 0x900000)
                Missile_PenetrateGlass(&tr, ent, org, origin, weapDef->damage, 1);
            Vec3Lerp(org, origin, tr.fraction, endpos);
            DrawMissilePredictDebug(org, endpos);
            org[0] = endpos[0];
            org[1] = endpos[1];
            org[2] = endpos[2];
            if ( weapDef->stickiness == WEAPSTICKINESS_ALL || weapDef->stickiness == WEAPSTICKINESS_ALL_NO_SENTIENTS )
            {
                if ( tr.fraction < 1.0 )
                {
                    traceStart[0] = (float)(0.13500001 * tr.normal.vec.v[0]) + org[0];
                    traceStart[1] = (float)(0.13500001 * tr.normal.vec.v[1]) + org[1];
                    traceStart[2] = (float)(0.13500001 * tr.normal.vec.v[2]) + org[2];
                    origin[0] = (float)(-1.5 * tr.normal.vec.v[0]) + org[0];
                    origin[1] = (float)(-1.5 * tr.normal.vec.v[1]) + org[1];
                    origin[2] = (float)(-1.5 * tr.normal.vec.v[2]) + org[2];
                    //if ( EntHandle::isDefined(&ent->r.ownerNum) )
                    if ( ent->r.ownerNum.isDefined() )
                    {
                        //v7 = EntHandle::entnum(&ent->r.ownerNum);
                        v7 = ent->r.ownerNum.entnum();
                        G_MissileTrace(&tr, traceStart, origin, v7, ent->clipmask, ent->s.weapon);
                    }
                    else
                    {
                        G_MissileTrace(&tr, traceStart, origin, 1023, ent->clipmask, ent->s.weapon);
                    }
                    Vec3Lerp(traceStart, origin, tr.fraction, endpos);
                    if ( tr.fraction != 1.0 && Trace_GetEntityHitId(&tr) == 1022 )
                    {
                        org[0] = endpos[0] + (float)(endpos[0] - origin[0]);
                        org[1] = endpos[1] + (float)(endpos[1] - origin[1]);
                        org[2] = endpos[2] + (float)(endpos[2] - origin[2]);
                    }
                }
            }
            else if ( tr.fraction == 1.0 || tr.fraction < 1.0 && tr.normal.vec.v[2] > 0.69999999 )
            {
                traceStart[0] = org[0];
                traceStart[1] = org[1];
                origin[0] = org[0];
                origin[1] = org[1];
                traceStart[2] = org[2] + 0.13500001;
                origin[2] = org[2] - 1.5;
                //if ( EntHandle::isDefined(&ent->r.ownerNum) )
                if ( ent->r.ownerNum.isDefined() )
                {
                    //v6 = EntHandle::entnum(&ent->r.ownerNum);
                    v6 = ent->r.ownerNum.entnum();
                    G_MissileTrace(&tr, traceStart, origin, v6, ent->clipmask, ent->s.weapon);
                }
                else
                {
                    G_MissileTrace(&tr, traceStart, origin, 1023, ent->clipmask, ent->s.weapon);
                }
                Vec3Lerp(traceStart, origin, tr.fraction, endpos);
                if ( tr.fraction != 1.0 )
                {
                    pos.trBase[2] = (float)((float)(endpos[2] + 1.5) - org[2]) + pos.trBase[2];
                    org[0] = endpos[0];
                    org[1] = endpos[1];
                    org[2] = endpos[2] + 1.5;
                }
            }
            if ( tr.fraction != 1.0 )
            {
                if ( (tr.sflags & 4) != 0 )
                {
LABEL_36:
                    memcpy(ent, &backupEnt, sizeof(gentity_s));
                    return 0;
                }
                if ( !allowBounce
                    || (ent->s.lerp.eFlags & 0x1000000) == 0
                    || (PredictBounceMissile(ent, &pos, &tr, time, time + (int)(float)(50.0 * tr.fraction) - 50, org, endpos),
                            pos.trTime = time,
                            !pos.trType) )
                {
                    *timeAtRest = time;
                    break;
                }
            }
        }
    }
    *vLandPos = org[0];
    vLandPos[1] = org[1];
    vLandPos[2] = org[2];

    nanassertvec3(vLandPos);

    memcpy(ent, &backupEnt, sizeof(gentity_s));
    if (allowBounce && (ent->s.lerp.eFlags & 0x1000000) != 0)
        return ent->nextthink;
    else
        return time;
}

void __cdecl DrawMissilePredictDebug(float *start, float *end)
{
    float color[4]; // [esp+0h] [ebp-10h] BYREF

    color[0] = 0.5f;
    color[1] = 0.0f;
    color[2] = 1.0f;
    color[3] = 1.0f;
    if ( g_debugBullets->current.integer >= 5 )
        CG_DebugLine(start, end, color, 1, 1000);
}

void __cdecl PredictBounceMissile(
                gentity_s *ent,
                trajectory_t *pos,
                trace_t *trace,
                int time,
                int velocityTime,
                float *origin,
                float *endpos)
{
    float v7; // [esp+3Ch] [ebp-30h]
    float velocity[3]; // [esp+40h] [ebp-2Ch] BYREF
    int hitTime; // [esp+4Ch] [ebp-20h]
    int surfType; // [esp+50h] [ebp-1Ch]
    float bounceFactor; // [esp+54h] [ebp-18h]
    const WeaponDef *weapDef; // [esp+58h] [ebp-14h]
    float dot; // [esp+5Ch] [ebp-10h]
    float vDelta[3]; // [esp+60h] [ebp-Ch]

    if ( !ent->s.weapon
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 3905, 0, "%s", "ent->s.weapon") )
    {
        __debugbreak();
    }
    weapDef = BG_GetWeaponDef(ent->s.weapon);
    if ( !weapDef && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 3907, 0, "%s", "weapDef") )
        __debugbreak();
    surfType = (unsigned __int8)((int)(0x3F00000 & trace->sflags) >> 20);
    hitTime = velocityTime;
    BG_EvaluateTrajectoryDelta(pos, velocityTime, velocity);
    dot = (float)((float)(velocity[0] * trace->normal.vec.v[0]) + (float)(velocity[1] * trace->normal.vec.v[1]))
            + (float)(velocity[2] * trace->normal.vec.v[2]);
    v7 = -2.0 * dot;
    pos->trDelta[0] = (float)((float)(-2.0 * dot) * trace->normal.vec.v[0]) + velocity[0];
    pos->trDelta[1] = (float)(v7 * trace->normal.vec.v[1]) + velocity[1];
    pos->trDelta[2] = (float)(v7 * trace->normal.vec.v[2]) + velocity[2];
    if ( ((LODWORD(pos->trDelta[0]) & 0x7F800000) == 0x7F800000
         || (LODWORD(pos->trDelta[1]) & 0x7F800000) == 0x7F800000
         || (LODWORD(pos->trDelta[2]) & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp",
                    3916,
                    0,
                    "%s",
                    "!IS_NAN((pos->trDelta)[0]) && !IS_NAN((pos->trDelta)[1]) && !IS_NAN((pos->trDelta)[2])") )
    {
        __debugbreak();
    }
    if ((ent->s.lerp.eFlags & 0x1000000) == 0)
        goto LABEL_27;
    bounceFactor = Vec3Length(velocity);
    if ( bounceFactor > 0.0 && dot <= 0.0 )
    {
        dot = dot / (-(bounceFactor));
        bounceFactor = (float)((float)(weapDef->perpendicularBounce[surfType] - weapDef->parallelBounce[surfType]) * dot)
                                 + weapDef->parallelBounce[surfType];
        pos->trDelta[0] = bounceFactor * pos->trDelta[0];
        pos->trDelta[1] = bounceFactor * pos->trDelta[1];
        pos->trDelta[2] = bounceFactor * pos->trDelta[2];
        if ( ((LODWORD(pos->trDelta[0]) & 0x7F800000) == 0x7F800000
             || (LODWORD(pos->trDelta[1]) & 0x7F800000) == 0x7F800000
             || (LODWORD(pos->trDelta[2]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp",
                        3928,
                        0,
                        "%s",
                        "!IS_NAN((pos->trDelta)[0]) && !IS_NAN((pos->trDelta)[1]) && !IS_NAN((pos->trDelta)[2])") )
        {
            __debugbreak();
        }
    }
    if ( weapDef->stickiness == WEAPSTICKINESS_ALL
        || weapDef->stickiness == WEAPSTICKINESS_ALL_NO_SENTIENTS
        || trace->normal.vec.v[2] > 0.69999999
        && (weapDef->stickiness == WEAPSTICKINESS_GROUND
         || weapDef->stickiness == WEAPSTICKINESS_GROUND_WITH_YAW
         || Vec3Length(pos->trDelta) < 20.0) )
    {
        pos->trBase[0] = *endpos;
        pos->trBase[1] = endpos[1];
        pos->trBase[2] = endpos[2];
        pos->trType = 0;
        pos->trTime = 0;
        pos->trDuration = 0;
        pos->trDelta[0] = 0.0f;
        pos->trDelta[1] = 0.0f;
        pos->trDelta[2] = 0.0f;
    }
    else
    {
LABEL_27:
        vDelta[0] = 0.1 * trace->normal.vec.v[0];
        vDelta[1] = 0.1 * trace->normal.vec.v[1];
        vDelta[2] = 0.1 * trace->normal.vec.v[2];
        if ( vDelta[2] > 0.0 )
            vDelta[2] = 0.0f;
        pos->trBase[0] = *origin + vDelta[0];
        pos->trBase[1] = origin[1] + vDelta[1];
        pos->trBase[2] = origin[2] + vDelta[2];
        pos->trTime = time;
    }
}

void __cdecl G_InitGrenadeEntity(gentity_s *parent, gentity_s *grenade)
{
    gentity_s *ent; // [esp+0h] [ebp-14h]
    int fusetime; // [esp+Ch] [ebp-8h]
    const WeaponDef *weapDef; // [esp+10h] [ebp-4h]

    if ( !parent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 4104, 0, "%s", "parent") )
        __debugbreak();
    if ( !grenade && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 4105, 0, "%s", "grenade") )
        __debugbreak();
    if ( !grenade->s.weapon
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 4107, 0, "%s", "grenade->s.weapon") )
    {
        __debugbreak();
    }
    weapDef = BG_GetWeaponDef(grenade->s.weapon);
    if ( !weapDef && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 4109, 0, "%s", "weapDef") )
        __debugbreak();
    grenade->s.eType = ET_MISSILE;
    grenade->s.lerp.eFlags = 0x1000000;
    grenade->s.lerp.u.actor.actorNum = level.time;
    fusetime = weapDef->aiFuseTime;
    if ( parent && parent->client )
        fusetime = weapDef->fuseTime;
    grenade->s.lerp.u.actor.team = fusetime;
    grenade->r.contents = 256;
    grenade->r.contents |= 0x2000u;
    grenade->r.mins[0] = -1.5f;
    grenade->r.mins[1] = -1.5f;
    grenade->r.mins[2] = -1.5f;
    grenade->r.maxs[0] = 1.5f;
    grenade->r.maxs[1] = 1.5f;
    grenade->r.maxs[2] = 1.5f;
    if ( parent && parent->client )
        grenade->missile.antilagTimeOffset = parent->client->lastServerTime - level.time;
    else
        grenade->missile.antilagTimeOffset = 0;
    if ( !parent->client
        || parent->client->ps.throwBackGrenadeOwner == 1023
        || (Scr_AddEntity(grenade, SCRIPTINSTANCE_SERVER),
                Scr_AddEntity(&g_entities[parent->client->ps.throwBackGrenadeOwner], SCRIPTINSTANCE_SERVER),
                Scr_Notify(parent, scr_const.grenade_throwback, 2u),
                parent->client->ps.grenadeTimeLeft >= 0) )
    {
        //EntHandle::setEnt(&grenade->r.ownerNum, parent);
        grenade->r.ownerNum.setEnt(parent);
        //EntHandle::setEnt(&grenade->parent, parent);
        grenade->parent.setEnt(parent);
        grenade->s.lerp.u.loopFx.period = parent->s.clientNum;
    }
    else
    {
        //EntHandle::setEnt(&grenade->r.ownerNum, &g_entities[parent->client->ps.throwBackGrenadeOwner]);
        grenade->r.ownerNum.setEnt(&g_entities[parent->client->ps.throwBackGrenadeOwner]);
        //if ( EntHandle::isDefined(&grenade->r.ownerNum) )
        if ( grenade->r.ownerNum.isDefined() )
        {
            //ent = EntHandle::ent(&grenade->r.ownerNum);
            ent = grenade->r.ownerNum.ent();
            //EntHandle::setEnt(&grenade->parent, ent);
            grenade->parent.setEnt(ent);
        }
        else
        {
            //EntHandle::setEnt(&grenade->parent, 0);
            grenade->parent.setEnt(0);
        }
    }
    grenade->clipmask = 0x280E893;
    if ( weapDef->plantable )
        grenade->clipmask |= 0x2818011u;
    if ( weapDef->stickiness == WEAPSTICKINESS_GROUND_WITH_YAW )
        grenade->clipmask &= ~0x2000000u;
    grenade->handler = 10;
    G_BroadcastEntity(grenade);
    grenade->r.svFlags = 4;
    if ( weapDef->bThrowBack )
        G_MakeMissilePickupItem(grenade);
    if ( parent->client )
        grenade->missile.team = parent->client->sess.cs.team;
    else
        grenade->missile.team = TEAM_FREE;
}

void __cdecl G_InitGrenadeMovement(
                gentity_s *grenade,
                const float *start,
                const float *dir,
                int rotate,
                WeapRotateType rotateType)
{
    float v5; // [esp+Ch] [ebp-34h]
    float v6; // [esp+14h] [ebp-2Ch]

    if ( !grenade && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 4195, 0, "%s", "grenade") )
        __debugbreak();
    grenade->item[1].ammoCount = 0;
    grenade->s.lerp.pos.trType = G_GetGrenadeTrType(grenade);
    grenade->s.lerp.pos.trTime = level.time;
    grenade->r.currentOrigin[0] = *start;
    grenade->r.currentOrigin[1] = start[1];
    grenade->r.currentOrigin[2] = start[2];
    grenade->s.lerp.pos.trBase[0] = *start;
    grenade->s.lerp.pos.trBase[1] = start[1];
    grenade->s.lerp.pos.trBase[2] = start[2];
    grenade->s.lerp.pos.trDelta[0] = *dir;
    grenade->s.lerp.pos.trDelta[1] = dir[1];
    grenade->s.lerp.pos.trDelta[2] = dir[2];
    if ( ((LODWORD(grenade->s.lerp.pos.trDelta[0]) & 0x7F800000) == 0x7F800000
         || (LODWORD(grenade->s.lerp.pos.trDelta[1]) & 0x7F800000) == 0x7F800000
         || (LODWORD(grenade->s.lerp.pos.trDelta[2]) & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp",
                    4205,
                    0,
                    "%s",
                    "!IS_NAN((grenade->s.lerp.pos.trDelta)[0]) && !IS_NAN((grenade->s.lerp.pos.trDelta)[1]) && !IS_NAN((grenade->s."
                    "lerp.pos.trDelta)[2])") )
    {
        __debugbreak();
    }
    vectoangles(dir, grenade->r.currentAngles);
    grenade->s.lerp.pos.trDelta[0] = (float)(int)grenade->s.lerp.pos.trDelta[0];
    grenade->s.lerp.pos.trDelta[1] = (float)(int)grenade->s.lerp.pos.trDelta[1];
    grenade->s.lerp.pos.trDelta[2] = (float)(int)grenade->s.lerp.pos.trDelta[2];
    if ( rotate )
    {
        grenade->s.lerp.apos.trType = 3;
        grenade->s.lerp.apos.trTime = level.time;
        grenade->s.lerp.apos.trBase[0] = grenade->r.currentAngles[0];
        grenade->s.lerp.apos.trBase[1] = grenade->r.currentAngles[1];
        grenade->s.lerp.apos.trBase[2] = grenade->r.currentAngles[2];
        grenade->s.lerp.apos.trBase[0] = AngleNormalize360(grenade->s.lerp.apos.trBase[0] - 120.0);
        if ( rotateType == WEAPROTATE_BLADE_ROTATE )
        {
            grenade->s.lerp.apos.trDelta[0] = 1500.0f;
            grenade->s.lerp.apos.trDelta[1] = 0.0f;
            grenade->s.lerp.apos.trDelta[2] = 0.0f;
        }
        else
        {
            v6 = G_flrand(340.0, 800.0);
            grenade->s.lerp.apos.trDelta[0] = (double)(2 * G_irand(0, 2) - 1) * v6;
            grenade->s.lerp.apos.trDelta[1] = 0.0f;
            v5 = G_flrand(180.0, 540.0);
            grenade->s.lerp.apos.trDelta[2] = (double)(2 * G_irand(0, 2) - 1) * v5;
            grenade->s.lerp.pos.trDelta[0] = (float)(int)grenade->s.lerp.pos.trDelta[0];
            grenade->s.lerp.pos.trDelta[1] = (float)(int)grenade->s.lerp.pos.trDelta[1];
            grenade->s.lerp.pos.trDelta[2] = (float)(int)grenade->s.lerp.pos.trDelta[2];
        }
        grenade->r.currentAngles[0] = grenade->s.lerp.apos.trBase[0];
        grenade->r.currentAngles[1] = grenade->s.lerp.apos.trBase[1];
        grenade->r.currentAngles[2] = grenade->s.lerp.apos.trBase[2];
    }
    else
    {
        G_SetAngle(grenade, grenade->r.currentAngles);
    }
}

gentity_s *__cdecl G_FireGrenade(
                gentity_s *parent,
                float *start,
                float *dir,
                int grenadeWPID,
                unsigned __int8 grenModel,
                int rotate,
                int time)
{
    char *Name; // eax
    char *v8; // eax
    float speed; // [esp+0h] [ebp-18h]
    gentity_s *grenade; // [esp+Ch] [ebp-Ch]
    DObj *obj; // [esp+10h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+14h] [ebp-4h]

    if ( !parent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 4297, 0, "%s", "parent") )
        __debugbreak();
    weapDef = BG_GetWeaponDef(grenadeWPID);
    if ( !weapDef && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 4300, 0, "%s", "weapDef") )
        __debugbreak();
    grenade = G_Spawn();
    Scr_SetString(&grenade->classname, scr_const.grenade, SCRIPTINSTANCE_SERVER);
    AssignToSmallerType<unsigned short>(&grenade->s.weapon, grenadeWPID);
    grenade->s.weaponModel = grenModel;
    grenade->s.lerp.u.turret.ownerNum = 0;
    grenade->missile.grenade.effectIndex = 55;
    if ( parent->client )
        grenade->missile.grenade.effectIndex = parent->client->ps.smokeColorIndex;
    if ( parent->client )
        grenade->s.attackerEntityNum = parent->s.clientNum;
    else
        grenade->s.attackerEntityNum = -1;
    G_InitGrenadeEntity(parent, grenade);
    if ( rotate && weapDef->rotate )
        G_InitGrenadeMovement(grenade, start, dir, 1, weapDef->rotateType);
    else
        G_InitGrenadeMovement(grenade, start, dir, 0, weapDef->rotateType);
    if ( parent->client )
    {
        speed = Vec3Length(dir);
        grenade->s.lerp.u.actor.actorNum += CalcMissileNoDrawTime(speed);
    }
    InitGrenadeTimer(parent, grenade, weapDef, time);
    if ( grenade->model
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 4333, 0, "%s", "!grenade->model") )
    {
        __debugbreak();
    }
    if ( weapDef->projectileModel )
    {
        Name = (char *)XModelGetName(weapDef->projectileModel);
        G_SetModel(grenade, Name);
    }
    G_DObjUpdate(grenade);
    obj = Com_GetServerDObj(grenade->s.number);
    if ( obj )
    {
        grenade->r.contents |= DObjGetContents(obj);
        DObjCalcBounds(obj, grenade->r.mins, grenade->r.maxs);
    }
    v8 = (char *)BG_WeaponName(grenadeWPID);
    Scr_AddString(v8, SCRIPTINSTANCE_SERVER);
    Scr_AddEntity(grenade, SCRIPTINSTANCE_SERVER);
    if ( weapDef->iProjectileActivateDist )
        Scr_Notify(parent, scr_const.grenade_launcher_fire, 2u);
    else
        Scr_Notify(parent, scr_const.grenade_fire, 2u);
    return grenade;
}

int __cdecl CalcMissileNoDrawTime(float speed)
{
    int v3; // [esp+4h] [ebp-8h]

    if ( (int)(float)((float)((float)(-35.0 * speed) / 600.0) + 85.0) < 50 )
        v3 = (int)(float)((float)((float)(-35.0 * speed) / 600.0) + 85.0);
    else
        v3 = 50;
    if ( v3 > 20 )
        return v3;
    else
        return 20;
}

void __cdecl InitGrenadeTimer(const gentity_s *parent, gentity_s *grenade, const WeaponDef *weapDef, int time)
{
    if ( !parent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 4249, 0, "%s", "parent") )
        __debugbreak();
    if ( !grenade && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 4250, 0, "%s", "grenade") )
        __debugbreak();
    if ( !weapDef && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 4251, 0, "%s", "weapDef") )
        __debugbreak();
    if ( !weapDef->iProjectileActivateDist || time > 0 )
    {
        if ( !parent->client || weapDef->timedDetonation )
        {
            if ( parent->client && parent->client->ps.grenadeTimeLeft )
            {
                grenade->nextthink = parent->client->ps.grenadeTimeLeft + level.time;
                parent->client->ps.grenadeTimeLeft = 0;
            }
            else
            {
                grenade->nextthink = time + level.time;
            }
        }
        else
        {
            parent->client->ps.grenadeTimeLeft = 0;
        }
    }
    if ( grenade->handler != 10
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp",
                    4274,
                    0,
                    "%s",
                    "grenade->handler == ENT_HANDLER_GRENADE") )
    {
        __debugbreak();
    }
    grenade->item[0].index = level.time;
    if ( !grenade->nextthink )
        grenade->nextthink = level.time + 30000;
    if ( grenade->nextthink > level.time + 60000 )
        grenade->nextthink = level.time + 60000;
}


float MYJAVELINOFFSET_RIGHT = 10.0f;
float MYJAVELINOFFSET = 0.3f;
float MYJAVELINOFFSET_RIGHT_0 = 10.0f;
float MYJAVELINOFFSET_0 = 0.3f;


gentity_s *__cdecl G_FireRocket(
                gentity_s *parent,
                unsigned int weaponIndex,
                float *start,
                float *dir,
                const float *gunVel,
                gentity_s *target,
                const float *targetOffset)
{
    int v7; // eax
    long double v8; // st7
    char *Name; // eax
    int v10; // edx
    int Contents; // eax
    char *v12; // eax
    //$0D33AF6AB483EB176B99DAC6E021D6CF *v14; // [esp+10h] [ebp-B0h]
    //$0D33AF6AB483EB176B99DAC6E021D6CF *v15; // [esp+18h] [ebp-A8h]
    //$0D33AF6AB483EB176B99DAC6E021D6CF *v16; // [esp+1Ch] [ebp-A4h]
    float v17; // [esp+20h] [ebp-A0h]
    float *currentOrigin; // [esp+30h] [ebp-90h]
    float *v19; // [esp+40h] [ebp-80h]
    float *v20; // [esp+44h] [ebp-7Ch]
    float *trBase; // [esp+48h] [ebp-78h]
    float *trDelta; // [esp+4Ch] [ebp-74h]
    float iProjectileSpeed; // [esp+50h] [ebp-70h]
    float *v24; // [esp+54h] [ebp-6Ch]
    float *v25; // [esp+58h] [ebp-68h]
    float *v26; // [esp+5Ch] [ebp-64h]
    float v27; // [esp+60h] [ebp-60h]
    float sinT; // [esp+7Ch] [ebp-44h]
    float theta; // [esp+80h] [ebp-40h]
    float r; // [esp+84h] [ebp-3Ch]
    float v31[3]; // [esp+88h] [ebp-38h] BYREF
    float cosT; // [esp+94h] [ebp-2Ch]
    float up[3]; // [esp+98h] [ebp-28h] BYREF
    float right[3]; // [esp+A4h] [ebp-1Ch] BYREF
    DObj *obj; // [esp+B0h] [ebp-10h]
    int ownerIndex; // [esp+B4h] [ebp-Ch]
    gentity_s *bolt; // [esp+B8h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+BCh] [ebp-4h]
    int savedregs; // [esp+C0h] [ebp+0h] BYREF

    if ( !parent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 4395, 0, "%s", "parent") )
        __debugbreak();
    weapDef = BG_GetWeaponDef(weaponIndex);
    if ( !weapDef && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 4398, 0, "%s", "weapDef") )
        __debugbreak();
    Vec3Normalize(dir);
    if ( weapDef->guidedMissileType == MISSILE_GUIDANCE_JAVELIN )
    {
        dir[2] = dir[2] + MYJAVELINOFFSET;
        Vec3Normalize(dir);
        AngleVectors(parent->s.lerp.apos.trBase, 0, right, 0);
        v27 = MYJAVELINOFFSET_RIGHT;
        *start = (float)(MYJAVELINOFFSET_RIGHT * right[0]) + *start;
        start[1] = (float)(v27 * right[1]) + start[1];
        start[2] = (float)(v27 * right[2]) + start[2];
    }
    if ( weapDef->guidedMissileType == MISSILE_GUIDANCE_TVGUIDED )
    {
        *start = (float)(13.0 * *dir) + *start;
        start[1] = (float)(13.0 * dir[1]) + start[1];
        start[2] = (float)(13.0 * dir[2]) + start[2];
    }
    bolt = G_Spawn();
    Scr_SetString(&bolt->classname, scr_const.rocket, SCRIPTINSTANCE_SERVER);
    bolt->s.eType = ET_MISSILE;
    AssignToSmallerType<unsigned short>(&bolt->s.weapon, weaponIndex);
    bolt->s.weaponModel = 0;
    bolt->s.lerp.u.actor.actorNum = level.time;
    if ( parent->client )
    {
        v7 = CalcMissileNoDrawTime((float)weapDef->iProjectileSpeed);
        bolt->s.lerp.u.actor.actorNum += v7;
    }
    if ( parent->pTurretInfo && (parent->pTurretInfo->flags & 0x20000) != 0 )
    {
        bolt->r.ownerNum.setEnt(&g_entities[parent->s.otherEntityNum]);
        bolt->parent.setEnt(&g_entities[parent->s.otherEntityNum]);
    }
    else
    {
        bolt->r.ownerNum.setEnt(parent);
        bolt->parent.setEnt(parent);
    }
    bolt->clipmask = 0x280E893;
    bolt->handler = 12;
    InitRocketTimer(bolt, weapDef);
    bolt->item[1].ammoCount = 0;
    bolt->missileTargetEnt.setEnt(target);
    if ( targetOffset )
    {
        v26 = &bolt->mover.pos3[2];
        bolt->mover.pos3[2] = *targetOffset;
        v26[1] = targetOffset[1];
        v26[2] = targetOffset[2];
    }
    else
    {
        v25 = &bolt->mover.pos3[2];
        bolt->mover.pos3[2] = 0.0f;
        v25[1] = 0.0f;
        v25[2] = 0.0f;
    }
    if ( parent->client )
        bolt->missile.team = parent->client->sess.cs.team;
    else
        bolt->missile.team = (team_t)parent->team;
    if ( weapDef->bForceBounce )
        bolt->s.lerp.eFlags = 0x1000000;
    ownerIndex = 0;
    if ( parent->client )
        ownerIndex = parent->client - level.clients;
    if ( (unsigned int)ownerIndex >= level.maxclients
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp",
                    4474,
                    0,
                    "ownerIndex doesn't index level.maxclients\n\t%i not in [0, %i)",
                    ownerIndex,
                    level.maxclients) )
    {
        __debugbreak();
    }
    AssignToSmallerType<unsigned char>(&bolt->s.faction.iHeadIconTeam, bolt->missile.team | (4 * ownerIndex));
    G_BroadcastEntity(bolt);
    if ( weapDef->guidedMissileType == MISSILE_GUIDANCE_TVGUIDED )
    {
        bolt->r.contents |= 0x2100u;
        bolt->r.svFlags |= 4u;
        bolt->takedamage = 1;
        bolt->health = 10000;
    }
    bolt->s.lerp.pos.trType = 3;
    if ( weapDef->timeToAccelerate <= 0.0 )
    {
        trDelta = bolt->s.lerp.pos.trDelta;
        iProjectileSpeed = (float)weapDef->iProjectileSpeed;
        bolt->s.lerp.pos.trDelta[0] = iProjectileSpeed * *dir;
        trDelta[1] = iProjectileSpeed * dir[1];
        trDelta[2] = iProjectileSpeed * dir[2];
        bolt->s.lerp.pos.trDelta[2] = (float)weapDef->iProjectileSpeedUp + bolt->s.lerp.pos.trDelta[2];
    }
    else
    {
        bolt->s.lerp.pos.trType = 1;
        v24 = bolt->s.lerp.pos.trDelta;
        bolt->s.lerp.pos.trDelta[0] = 0.0f;
        v24[1] = 0.0f;
        v24[2] = 0.0f;
    }
    bolt->s.lerp.pos.trTime = level.time;
    trBase = bolt->s.lerp.pos.trBase;
    bolt->s.lerp.pos.trBase[0] = *start;
    trBase[1] = start[1];
    trBase[2] = start[2];
    v19 = bolt->s.lerp.pos.trDelta;
    v20 = bolt->s.lerp.pos.trDelta;
    bolt->s.lerp.pos.trDelta[0] = bolt->s.lerp.pos.trDelta[0] + *gunVel;
    v19[1] = v20[1] + gunVel[1];
    v19[2] = v20[2] + gunVel[2];
    if ( ((LODWORD(bolt->s.lerp.pos.trDelta[0]) & 0x7F800000) == 0x7F800000
         || (LODWORD(bolt->s.lerp.pos.trDelta[1]) & 0x7F800000) == 0x7F800000
         || (LODWORD(bolt->s.lerp.pos.trDelta[2]) & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp",
                    4507,
                    0,
                    "%s",
                    "!IS_NAN((bolt->s.lerp.pos.trDelta)[0]) && !IS_NAN((bolt->s.lerp.pos.trDelta)[1]) && !IS_NAN((bolt->s.lerp.pos.trDelta)[2])") )
    {
        __debugbreak();
    }
    bolt->s.lerp.pos.trDelta[0] = (float)(int)bolt->s.lerp.pos.trDelta[0];
    bolt->s.lerp.pos.trDelta[1] = (float)(int)bolt->s.lerp.pos.trDelta[1];
    bolt->s.lerp.pos.trDelta[2] = (float)(int)bolt->s.lerp.pos.trDelta[2];
    currentOrigin = bolt->r.currentOrigin;
    bolt->r.currentOrigin[0] = *start;
    currentOrigin[1] = start[1];
    currentOrigin[2] = start[2];
    vectoangles(dir, bolt->r.currentAngles);
    G_SetAngle(bolt, bolt->r.currentAngles);
    if ( weapDef->projectileCurvature > 0.0 )
    {
        bolt->s.lerp.pos.trType = TR_INTERPOLATE;
        AngleVectors(bolt->r.currentAngles, 0, v31, up);
        theta = G_random() * 360.0;
        r = G_random() * weapDef->projectileCurvature;
        v8 = (float)(theta * 0.017453292);
        cosT = cos(v8);
        sinT = sin(v8);
        //v16 = &bolt->missile.44;
        v17 = r * cosT;
        bolt->mover.pos2[2] = (float)(r * cosT) * v31[0];
        bolt->missile.missile.curvature[1] = v17 * v31[1];
        bolt->missile.missile.curvature[2] = v17 * v31[2];
        //v14 = &bolt->missile.44;
        //v15 = &bolt->missile.44;
        bolt->mover.pos2[2] = (float)((float)(r * sinT) * up[0]) + bolt->mover.pos2[2];
        bolt->missile.missile.curvature[1] = (float)((float)(r * sinT) * up[1]) + bolt->missile.missile.curvature[1];
        bolt->missile.missile.curvature[2] = (float)((float)(r * sinT) * up[2]) + bolt->missile.missile.curvature[2];
        if ( ((bolt->missile.grenade.effectIndex & 0x7F800000) == 0x7F800000
             || (LODWORD(bolt->mover.pos3[0]) & 0x7F800000) == 0x7F800000
             || (LODWORD(bolt->mover.pos3[1]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp",
                        4532,
                        0,
                        "%s",
                        "!IS_NAN((bolt->missile.missile.curvature)[0]) && !IS_NAN((bolt->missile.missile.curvature)[1]) && !IS_NAN((b"
                        "olt->missile.missile.curvature)[2])") )
        {
            __debugbreak();
        }
    }
    if ( weapDef->guidedMissileType )
    {
        bolt->s.lerp.pos.trType = 1;
        bolt->s.lerp.apos.trType = 1;
        bolt->s.lerp.pos.trDuration = 0;
        if ( weapDef->guidedMissileType == MISSILE_GUIDANCE_JAVELIN )
        {
            bolt->s.lerp.pos.trType = 6;
            bolt->s.lerp.pos.trTime = level.time;
            bolt->missile.missile.stage = MISSILESTAGE_SOFTLAUNCH;
            bolt->missile.missile.flightMode = (MissileFlightMode)!G_TargetAttackProfileTop(target);
        }
        else if ( weapDef->guidedMissileType == MISSILE_GUIDANCE_BALLISTIC )
        {
            bolt->s.lerp.pos.trType = 6;
            bolt->s.lerp.pos.trTime = level.time;
        }
    }
    if ( weapDef->guidedMissileType == MISSILE_GUIDANCE_TVGUIDED )
    {
        if ( bolt->model
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 4566, 0, "%s", "!bolt->model") )
        {
            __debugbreak();
        }
        if ( weapDef->projectileModel )
        {
            Name = (char *)XModelGetName(weapDef->projectileModel);
            G_SetModel(bolt, Name);
        }
    }
    if ( !weapDef->iProjectileSpeed
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp",
                    4572,
                    0,
                    "%s",
                    "weapDef->iProjectileSpeed") )
    {
        __debugbreak();
    }
    bolt->mover.aDecelTime = (float)((float)weapDef->destabilizeDistance / (float)weapDef->iProjectileSpeed) * 1000.0;
    bolt->s.lerp.u.turret.ownerNum = 0;
    if ( weapDef->destabilizationRateTime == 0.0 )
        v10 = bolt->flags | 0x20000;
    else
        v10 = bolt->flags | parent->flags & 0x20000;
    bolt->flags = v10;
    G_DObjUpdate(bolt);
    obj = Com_GetServerDObj(bolt->s.number);
    if ( obj )
    {
        Contents = DObjGetContents(obj);
        bolt->r.contents |= Contents;
        DObjCalcBounds(obj, bolt->r.mins, bolt->r.maxs);
    }
    SV_LinkEntity(bolt);
    if ( weapDef->guidedMissileType == MISSILE_GUIDANCE_TVGUIDED )
        G_LinkPlayerToRocket(bolt, parent);
    if ( target )
        Scr_AddEntity(target, SCRIPTINSTANCE_SERVER);
    else
        Scr_AddUndefined(SCRIPTINSTANCE_SERVER);
    v12 = (char *)BG_WeaponName(weaponIndex);
    Scr_AddString(v12, SCRIPTINSTANCE_SERVER);
    Scr_AddEntity(bolt, SCRIPTINSTANCE_SERVER);
    Scr_Notify(parent, scr_const.missile_fire, 3u);
    return bolt;
}

void __cdecl G_LinkPlayerToRocket(gentity_s *ent, gentity_s *player)
{
    gentity_s *turret; // [esp+8h] [ebp-8h]
    gclient_s *client; // [esp+Ch] [ebp-4h]

    client = player->client;
    if ( !client && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 827, 0, "%s", "client") )
        __debugbreak();
    if ( (client->ps.eFlags & 0x300) != 0 )
    {
        turret = &g_entities[client->ps.viewlocked_entNum];
        if ( turret->s.eType == 11 )
            G_ClientStopUsingTurret(turret);
    }
    AssignToSmallerType<short>(&client->ps.viewlocked_entNum, ent->s.number);
    client->ps.weapFlags |= 0x200u;
    client->ps.weapFlags &= ~0x100000u;
    client->ps.linkAngles[0] = client->ps.viewangles[0];
    client->ps.linkAngles[1] = client->ps.viewangles[1];
    client->ps.linkAngles[2] = client->ps.viewangles[2];
}

void __cdecl InitRocketTimer(gentity_s *bolt, const WeaponDef *weapDef)
{
    gclient_s *ps; // [esp+0h] [ebp-8h]

    if ( !bolt && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 4367, 0, "%s", "bolt") )
        __debugbreak();
    if ( !weapDef && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 4368, 0, "%s", "weapDef") )
        __debugbreak();
    bolt->nextthink = level.time + (int)(float)(weapDef->projLifetime * 1000.0);
    if ( weapDef->guidedMissileType == MISSILE_GUIDANCE_TVGUIDED )
    {
        iassert(bolt->parent.isDefined());
        iassert(bolt->parent.ent()->client);

        ps = bolt->parent.ent()->client;
        UpdateGuidedMissileFuelTime(bolt, 1);
        BG_SetFuelTankTime(&ps->ps, ps->ps.weapon, 0);
    }
    if ( bolt->nextthink > level.time + 60000 )
        bolt->nextthink = level.time + 60000;
}

gentity_s *__cdecl G_DropBomb(
                gentity_s *parent,
                unsigned int weaponIndex,
                float *start,
                float *dir,
                const float *gunVel,
                gentity_s *target,
                const float *targetOffset)
{
    int v7; // eax
    long double v8; // st7
    int v9; // ecx
    //$0D33AF6AB483EB176B99DAC6E021D6CF *v11; // [esp+10h] [ebp-A4h]
    //$0D33AF6AB483EB176B99DAC6E021D6CF *v12; // [esp+18h] [ebp-9Ch]
    //$0D33AF6AB483EB176B99DAC6E021D6CF *v13; // [esp+1Ch] [ebp-98h]
    float v14; // [esp+20h] [ebp-94h]
    float *currentOrigin; // [esp+30h] [ebp-84h]
    float *v16; // [esp+40h] [ebp-74h]
    float *trBase; // [esp+44h] [ebp-70h]
    float *trDelta; // [esp+48h] [ebp-6Ch]
    float iProjectileSpeed; // [esp+4Ch] [ebp-68h]
    float *v20; // [esp+50h] [ebp-64h]
    float *v21; // [esp+54h] [ebp-60h]
    float *v22; // [esp+58h] [ebp-5Ch]
    float v23; // [esp+5Ch] [ebp-58h]
    float sinT; // [esp+78h] [ebp-3Ch]
    float theta; // [esp+7Ch] [ebp-38h]
    float r; // [esp+80h] [ebp-34h]
    float v27[3]; // [esp+84h] [ebp-30h] BYREF
    float cosT; // [esp+90h] [ebp-24h]
    float up[3]; // [esp+94h] [ebp-20h] BYREF
    float right[3]; // [esp+A0h] [ebp-14h] BYREF
    gentity_s *bolt; // [esp+ACh] [ebp-8h]
    const WeaponDef *weapDef; // [esp+B0h] [ebp-4h]
    int savedregs; // [esp+B4h] [ebp+0h] BYREF

    if ( !parent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 4621, 0, "%s", "parent") )
        __debugbreak();
    weapDef = BG_GetWeaponDef(weaponIndex);
    if ( !weapDef && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp", 4624, 0, "%s", "weapDef") )
        __debugbreak();
    Vec3Normalize(dir);
    if ( weapDef->guidedMissileType == MISSILE_GUIDANCE_JAVELIN )
    {
        dir[2] = dir[2] + MYJAVELINOFFSET_0;
        Vec3Normalize(dir);
        AngleVectors(parent->s.lerp.apos.trBase, 0, right, 0);
        v23 = MYJAVELINOFFSET_RIGHT_0;
        *start = (float)(MYJAVELINOFFSET_RIGHT_0 * right[0]) + *start;
        start[1] = (float)(v23 * right[1]) + start[1];
        start[2] = (float)(v23 * right[2]) + start[2];
    }
    bolt = G_Spawn();
    Scr_SetString(&bolt->classname, scr_const.rocket, SCRIPTINSTANCE_SERVER);
    bolt->s.eType = ET_MISSILE;
    AssignToSmallerType<unsigned short>(&bolt->s.weapon, weaponIndex);
    bolt->s.lerp.u.actor.actorNum = level.time;
    if ( parent->client )
    {
        v7 = CalcMissileNoDrawTime((float)weapDef->iProjectileSpeed);
        bolt->s.lerp.u.actor.actorNum += v7;
    }
    bolt->r.ownerNum.setEnt(parent);
    bolt->parent.setEnt(parent);
    bolt->clipmask = 0x280E893;
    bolt->handler = 12;
    InitRocketTimer(bolt, weapDef);
    if ( targetOffset )
    {
        v22 = &bolt->mover.pos3[2];
        bolt->mover.pos3[2] = *targetOffset;
        v22[1] = targetOffset[1];
        v22[2] = targetOffset[2];
    }
    else
    {
        v21 = &bolt->mover.pos3[2];
        bolt->mover.pos3[2] = 0.0f;
        v21[1] = 0.0f;
        v21[2] = 0.0f;
    }
    if ( parent->client )
        bolt->missile.team = parent->client->sess.cs.team;
    else
        bolt->missile.team = TEAM_FREE;
    G_BroadcastEntity(bolt);
    bolt->s.lerp.pos.trType = 6;
    if ( weapDef->timeToAccelerate <= 0.0 )
    {
        trDelta = bolt->s.lerp.pos.trDelta;
        iProjectileSpeed = (float)weapDef->iProjectileSpeed;
        bolt->s.lerp.pos.trDelta[0] = iProjectileSpeed * *dir;
        trDelta[1] = iProjectileSpeed * dir[1];
        trDelta[2] = iProjectileSpeed * dir[2];
    }
    else
    {
        bolt->s.lerp.pos.trType = 1;
        v20 = bolt->s.lerp.pos.trDelta;
        bolt->s.lerp.pos.trDelta[0] = 0.0f;
        v20[1] = 0.0f;
        v20[2] = 0.0f;
    }
    bolt->s.lerp.pos.trTime = level.time;
    trBase = bolt->s.lerp.pos.trBase;
    bolt->s.lerp.pos.trBase[0] = *start;
    trBase[1] = start[1];
    trBase[2] = start[2];
    v16 = bolt->s.lerp.pos.trDelta;
    bolt->s.lerp.pos.trDelta[0] = *gunVel;
    v16[1] = gunVel[1];
    v16[2] = gunVel[2];
    if ( ((LODWORD(bolt->s.lerp.pos.trDelta[0]) & 0x7F800000) == 0x7F800000
         || (LODWORD(bolt->s.lerp.pos.trDelta[1]) & 0x7F800000) == 0x7F800000
         || (LODWORD(bolt->s.lerp.pos.trDelta[2]) & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp",
                    4691,
                    0,
                    "%s",
                    "!IS_NAN((bolt->s.lerp.pos.trDelta)[0]) && !IS_NAN((bolt->s.lerp.pos.trDelta)[1]) && !IS_NAN((bolt->s.lerp.pos.trDelta)[2])") )
    {
        __debugbreak();
    }
    bolt->s.lerp.pos.trDelta[0] = (float)(int)bolt->s.lerp.pos.trDelta[0];
    bolt->s.lerp.pos.trDelta[1] = (float)(int)bolt->s.lerp.pos.trDelta[1];
    bolt->s.lerp.pos.trDelta[2] = (float)(int)bolt->s.lerp.pos.trDelta[2];
    currentOrigin = bolt->r.currentOrigin;
    bolt->r.currentOrigin[0] = *start;
    currentOrigin[1] = start[1];
    currentOrigin[2] = start[2];
    vectoangles(dir, bolt->r.currentAngles);
    G_SetAngle(bolt, bolt->r.currentAngles);
    if ( weapDef->projectileCurvature > 0.0 )
    {
        bolt->s.lerp.pos.trType = 1;
        AngleVectors(bolt->r.currentAngles, 0, v27, up);
        theta = G_random() * 360.0;
        r = G_random() * weapDef->projectileCurvature;
        v8 = (float)(theta * 0.017453292);
        cosT = cos(v8);
        sinT = sin(v8);
        //v13 = &bolt->missile.44;
        v14 = r * cosT;
        bolt->mover.pos2[2] = (float)(r * cosT) * v27[0];
        bolt->missile.missile.curvature[1] = v14 * v27[1];
        bolt->missile.missile.curvature[2] = v14 * v27[2];
        //v11 = &bolt->missile.44;
        //v12 = &bolt->missile.44;
        bolt->mover.pos2[2] = (float)((float)(r * sinT) * up[0]) + bolt->mover.pos2[2];
        bolt->missile.missile.curvature[1] = (float)((float)(r * sinT) * up[1]) + bolt->missile.missile.curvature[1];
        bolt->missile.missile.curvature[2] = (float)((float)(r * sinT) * up[2]) + bolt->missile.missile.curvature[2];
        if ( ((bolt->missile.grenade.effectIndex & 0x7F800000) == 0x7F800000
             || (LODWORD(bolt->mover.pos3[0]) & 0x7F800000) == 0x7F800000
             || (LODWORD(bolt->mover.pos3[1]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp",
                        4716,
                        0,
                        "%s",
                        "!IS_NAN((bolt->missile.missile.curvature)[0]) && !IS_NAN((bolt->missile.missile.curvature)[1]) && !IS_NAN((b"
                        "olt->missile.missile.curvature)[2])") )
        {
            __debugbreak();
        }
    }
    if ( weapDef->guidedMissileType )
    {
        if ( weapDef->guidedMissileType != MISSILE_GUIDANCE_BALLISTIC )
        {
            bolt->s.lerp.pos.trType = 1;
            bolt->s.lerp.apos.trType = 1;
            bolt->s.lerp.pos.trDuration = 0;
            if ( weapDef->guidedMissileType == MISSILE_GUIDANCE_JAVELIN )
            {
                bolt->s.lerp.pos.trType = 6;
                bolt->s.lerp.pos.trTime = level.time;
                bolt->missile.missile.stage = MISSILESTAGE_SOFTLAUNCH;
                bolt->missile.missile.flightMode = (MissileFlightMode)!G_TargetAttackProfileTop(target);
            }
        }
    }
    if ( weapDef->guidedMissileType == MISSILE_GUIDANCE_BALLISTIC )
    {
        bolt->s.lerp.apos.trType = 3;
        bolt->s.lerp.apos.trTime = level.time;
        bolt->s.lerp.apos.trDelta[2] = 75.0f;
    }
    if ( !weapDef->iProjectileSpeed
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_missile.cpp",
                    4751,
                    0,
                    "%s",
                    "weapDef->iProjectileSpeed") )
    {
        __debugbreak();
    }
    bolt->mover.aDecelTime = (float)((float)weapDef->destabilizeDistance / (float)weapDef->iProjectileSpeed) * 1000.0;
    bolt->s.lerp.u.turret.ownerNum = 0;
    if ( weapDef->destabilizationRateTime == 0.0 )
        v9 = bolt->flags | 0x20000;
    else
        v9 = bolt->flags | parent->flags & 0x20000;
    bolt->flags = v9;
    SV_LinkEntity(bolt);
    return bolt;
}

