#include "cg_view_mp.h"
#include "universal/q_shared.h"
#include "cg_local_mp.h"
#include "cg_main_mp.h"
#include <EffectsCore/fx_system.h>
#include <EffectsCore/fx_load_obj.h>
#include <universal/com_math_anglevectors.h>
#include "cg_vehicles_mp.h"
#include <cgame/cg_camerashake.h>
#include <bgame/bg_misc.h>
#include <EffectsCore/fx_update.h>
#include "cg_predict_mp.h"
#include <gfx_d3d/r_dpvs.h>
#include <client_mp/cl_scrn_mp.h>
#include <bgame/bg_weapons_view.h>
#include <demo/demo_playback.h>
#include <gfx_d3d/r_adszscale.h>
#include <qcommon/dobj_management.h>
#include <clientscript/scr_const.h>
#include "cg_ents_mp.h"
#include <game/statindex.h>
#include <cgame/cg_drawtools.h>
#include <gfx_d3d/r_dvars.h>
#include <client/splitscreen.h>
#include <cgame/cg_draw_reticles.h>
#include <qcommon/threads.h>
#include <cgame/cg_world.h>
#include <EffectsCore/fx_marks.h>
#include <gfx_d3d/r_rope_render.h>
#include "cg_draw_net_mp.h"
#include <cgame/cg_main.h>
#include "cg_snapshot_mp.h"
#include <bgame/bg_wind.h>
#include <cgame/cg_draw_indicators.h>
#include <cgame/cg_bolt.h>
#include <cgame/cg_compass.h>
#include <aim_assist/aim_target.h>
#include <client/cl_keys.h>
#include <client_mp/cl_input_mp.h>
#include "cg_scr_main_mp.h"
#include <xanim/xanim_clientnotify.h>
#include <cgame/cg_event.h>
#include <cgame/cg_scr_main.h>
#include <clientscript/cscr_vm.h>
#include <sound/snd_public_async.h>
#include <bgame/bg_fire.h>
#include "cg_draw_mp.h"
#include <ik/ik.h>
#include <gfx_d3d/r_extracam.h>
#include <client/client.h>
#include <gfx_d3d/r_scene.h>
#include <gfx_d3d/r_workercmds_common.h>
#include <gfx_d3d/r_rendercmds.h>

ClientViewParams clientViewParamsArray[1][1][1] = { { { { 0.0, 0.0, 1.0, 1.0, VIEWPORT_LARGE } } } };

static const float cg_shallowWaterLevel = 30.0f;

static const float CAM_MINS[3] = { -4.0, -4.0, -4.0 };
static const float CAM_MAXS[3] = { 4.0, 4.0, 4.0 };

float devSavedAngles[3];
float devSavedOrigin[3];


const dvar_t *cg_heliKillCamFov;
const dvar_t *cg_heliKillCamNearBlur;
const dvar_t *cg_heliKillCamFarBlur;
const dvar_t *cg_heliKillCamFarBlurStart;
const dvar_t *cg_heliKillCamFarBlurDist;
const dvar_t *cg_heliKillCamNearBlurStart;
const dvar_t *cg_heliKillCamNearBlurEnd;

const dvar_t *cg_scriptedKillCamFov;
const dvar_t *cg_scriptedKillCamCloseXYDist;
const dvar_t *cg_scriptedKillCamCloseZDist;
const dvar_t *cg_scriptedKillCamNearBlur;
const dvar_t *cg_scriptedKillCamFarBlur;
const dvar_t *cg_scriptedKillCamFarBlurStart;
const dvar_t *cg_scriptedKillCamFarBlurDist;
const dvar_t *cg_scriptedKillCamNearBlurStart;
const dvar_t *cg_scriptedKillCamNearBlurEnd;

const dvar_t *cg_destructibleKillCamFov;
const dvar_t *cg_destructibleKillCamCloseXYDist;
const dvar_t *cg_destructibleKillCamCloseZDist;
const dvar_t *cg_destructibleKillCamNearBlur;
const dvar_t *cg_destructibleKillCamFarBlur;
const dvar_t *cg_destructibleKillCamFarBlurStart;
const dvar_t *cg_destructibleKillCamFarBlurDist;
const dvar_t *cg_destructibleKillCamNearBlurStart;
const dvar_t *cg_destructibleKillCamNearBlurEnd;
const dvar_t *cg_destructibleKillCamZIncrease;
const dvar_t *cg_destructibleKillCamRegularHeight;

const dvar_t *cg_explosiveKillCamUpDist;
const dvar_t *cg_explosiveKillCamBackDist;
const dvar_t *cg_explosiveKillCamWallOutDist;
const dvar_t *cg_explosiveKillCamWallSideDist;
const dvar_t *cg_explosiveKillCamGroundUpDist;
const dvar_t *cg_explosiveKillCamGroundBackDist;
const dvar_t *cg_explosiveKillCamStopDist;
const dvar_t *cg_explosiveKillCamStopDecelDist;

const dvar_t *cg_rocketKillCamUpDist;
const dvar_t *cg_rocketKillCamBackDist;

const dvar_t *cg_dogKillCamFov;
const dvar_t *cg_dogKillCamForwardDist;
const dvar_t *cg_dogKillCamUpDist;
const dvar_t *cg_dogKillCamSideDist;
const dvar_t *cg_dogKillCamDistFromEyes;
const dvar_t *cg_dogKillMinDistFromTarget;

const dvar_t *cg_artilleryKillCamFov;
const dvar_t *cg_artilleryKillCamUpDist;
const dvar_t *cg_artilleryKillCamBackDist;
const dvar_t *cg_artilleryKillCamWallOutDist;
const dvar_t *cg_artilleryKillCamWallSideDist;
const dvar_t *cg_artilleryKillCamGroundUpDist;
const dvar_t *cg_artilleryKillCamGroundBackDist;

const dvar_t *cg_turretKillCamFov;
const dvar_t *cg_turretKillCamCloseXYDist;
const dvar_t *cg_turretKillCamCloseZDist;
const dvar_t *cg_turretKillCamNearBlur;
const dvar_t *cg_turretKillCamFarBlur;
const dvar_t *cg_turretKillCamFarBlurStart;
const dvar_t *cg_turretKillCamFarBlurDist;
const dvar_t *cg_turretKillCamNearBlurStart;
const dvar_t *cg_turretKillCamNearBlurEnd;
const dvar_t *cg_turretKillCamHeightIncrease;
const dvar_t *cg_turretKillCamBackOffset;
const dvar_t *cg_turretKillCamSideOffset;
const dvar_t *cg_turretKillCamDistanceIncrease;

const dvar_t *cg_infraredBlurTime;


TestEffect s_testEffect[1];

void __cdecl CG_DrawWaterTrail(int localClientNum, const float *pos, float waterHeight)
{
    int waterEffect; // [esp+0h] [ebp-38h]
    cg_s *cgameGlob; // [esp+4h] [ebp-34h]
    float fxPos[3]; // [esp+8h] [ebp-30h] BYREF
    float axis[3][3]; // [esp+14h] [ebp-24h] BYREF

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    axis[0][0] = 0.0f;
    axis[0][1] = 0.0f;
    axis[0][2] = 1.0f;
    axis[1][0] = 0.0f;
    axis[1][1] = -1.0f;
    axis[1][2] = 0.0f;
    axis[2][0] = 1.0f;
    axis[2][1] = 0.0f;
    axis[2][2] = 0.0f;
    fxPos[0] = pos[0];
    fxPos[1] = pos[1];
    fxPos[2] = waterHeight;

    if ((float)(waterHeight - pos[2]) < cg_shallowWaterLevel)
        waterEffect = 6;
    else
        waterEffect = 5;

    FX_PlayOrientedEffect(localClientNum, cgMedia.physicsWaterEffects[waterEffect], cgameGlob->time, fxPos, axis);
}

int __cdecl CG_GetNextRippleTime(centity_s *cent)
{
    if ( cent->nextState.eType == 1 )
        return cent->pose.player.nextRippleTime;
    if ( cent->nextState.eType == 17 )
        return cent->pose.actor.nextRippleTime;
    return 0x7FFFFFFF;
}

void __cdecl CG_SetNextRippleTime(centity_s *cent, int time, float speed, bool in_water)
{
    int delta; // [esp+0h] [ebp-8h]
    int nextRippleTime; // [esp+4h] [ebp-4h]

    delta = irand(
                        cg_waterTrailRippleFrequency->current.integer,
                        cg_waterTrailRippleVariance->current.integer + cg_waterTrailRippleFrequency->current.integer);
    if ( speed >= 5.0 )
    {
        if ( in_water )
            delta = (int)(float)((float)delta * 0.5);
    }
    else
    {
        delta *= 3;
    }
    nextRippleTime = delta + time;
    if ( cent->nextState.eType == 1 )
    {
        cent->pose.player.nextRippleTime = nextRippleTime;
    }
    else if ( cent->nextState.eType == 17 )
    {
        cent->pose.actor.nextRippleTime = nextRippleTime;
    }
}

int __cdecl CG_GetNextWaterCheck(centity_s *cent)
{
    if ( cent->nextState.eType == 1 )
        return cent->pose.player.nextWaterHeightCheck;
    if ( cent->nextState.eType == 17 )
        return cent->pose.actor.nextWaterHeightCheck;
    return 0x7FFFFFFF;
}

void __cdecl CG_SetNextWaterCheck(centity_s *cent, int time, bool in_water)
{
    int v3; // [esp+0h] [ebp-14h]
    int delta; // [esp+4h] [ebp-10h]
    int nextWaterCheck; // [esp+Ch] [ebp-8h]

    delta = irand(0, 200) + 200;
    if ( in_water )
        v3 = delta;
    else
        v3 = 2 * delta;
    nextWaterCheck = v3 + time;
    if ( cent->nextState.eType == 1 )
    {
        cent->pose.player.nextWaterHeightCheck = nextWaterCheck;
    }
    else if ( cent->nextState.eType == 17 )
    {
        cent->pose.actor.nextWaterHeightCheck = nextWaterCheck;
    }
}

void __cdecl CG_SetCentWaterHeight(centity_s *cent, float height)
{
    if ( cent->nextState.eType == 1 )
    {
        cent->pose.player.waterHeight = height;
    }
    else if ( cent->nextState.eType == 17 )
    {
        cent->pose.turret.firingOffset = height;
    }
}

double __cdecl CG_GetCentWaterHeight(centity_s *cent)
{
    if ( cent->nextState.eType == 1 )
        return cent->pose.player.waterHeight;
    if ( cent->nextState.eType == 17 )
        return cent->pose.turret.firingOffset;
    return -20000.0;
}

void __cdecl CG_DrawWaterTrail(int localClientNum, centity_s *cent)
{
    bool in_water; // [esp+1Bh] [ebp-21h]
    float speed; // [esp+1Ch] [ebp-20h]
    float v4; // [esp+20h] [ebp-1Ch]
    float velocity[3]; // [esp+24h] [ebp-18h] BYREF
    cg_s *cgameGlob; // [esp+30h] [ebp-Ch]
    float waterHeight; // [esp+34h] [ebp-8h]
    int nextRippleTime; // [esp+38h] [ebp-4h]

    if ( !cent->pose.ragdollHandle
        && !cent->pose.killcamRagdollHandle
        && (cent->nextState.eType != 1 || (cent->currentState.eFlags & 0x4000) == 0) )
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        if ( CG_GetNextWaterCheck(cent) < cgameGlob->time )
        {
            v4 = CM_GetWaterHeight(cent->pose.origin, 70.0, -5.0);
            CG_SetCentWaterHeight(cent, v4);
            CG_SetNextWaterCheck(cent, cgameGlob->time, v4 >= cent->pose.origin[2]);
        }
        waterHeight = CG_GetCentWaterHeight(cent);
        velocity[0] = cent->nextState.lerp.pos.trBase[0] - cent->currentState.pos.trBase[0];
        velocity[1] = cent->nextState.lerp.pos.trBase[1] - cent->currentState.pos.trBase[1];
        velocity[2] = cent->nextState.lerp.pos.trBase[2] - cent->currentState.pos.trBase[2];
        nextRippleTime = CG_GetNextRippleTime(cent);
        if ( nextRippleTime < cgameGlob->time )
        {
            in_water = 0;
            if ( waterHeight >= cent->pose.origin[2] )
            {
                in_water = 1;
                CG_DrawWaterTrail(localClientNum, cent->pose.origin, waterHeight);
            }
            speed = Vec3Length(velocity);
            CG_SetNextRippleTime(cent, cgameGlob->time, speed, in_water);
        }
    }
}

void __cdecl CG_DrawWaterTrail(int localClientNum)
{
    bool in_water; // [esp+17h] [ebp-15h]
    float speed; // [esp+18h] [ebp-14h]
    cg_s *cgameGlob; // [esp+20h] [ebp-Ch]
    centity_s *cent; // [esp+24h] [ebp-8h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->renderingThirdPerson != TP_FOR_MODEL && cgameGlob->predictedPlayerState.pm_type != 1 )
    {
        cent = CG_GetEntity(localClientNum, cgameGlob->predictedPlayerState.clientNum);
        if ( cgameGlob->predictedPlayerState.waterlevel )
        {
            if ( (cent->currentState.eFlags & 0x4000) == 0 )
            {
                if ( cent->pose.player.nextWaterHeightCheck < cgameGlob->time )
                {
                    cent->pose.player.waterHeight = CM_GetWaterHeight(
                                                                                        cgameGlob->predictedPlayerState.origin,
                                                                                        cgameGlob->predictedPlayerState.viewHeightCurrent + 10.0,
                                                                                        -5.0);
                    CG_SetNextWaterCheck(cent, cgameGlob->time, 1);
                }
                if ( CG_GetNextRippleTime(cent) < cgameGlob->time )
                {
                    in_water = 0;
                    if ( cent->pose.player.waterHeight >= cgameGlob->predictedPlayerState.origin[2] )
                    {
                        CG_DrawWaterTrail(localClientNum, cgameGlob->predictedPlayerState.origin, cent->pose.player.waterHeight);
                        in_water = 1;
                    }
                    speed = Vec3Length(cgameGlob->predictedPlayerState.velocity);
                    CG_SetNextRippleTime(cent, cgameGlob->time, speed, in_water);
                }
            }
        }
        else
        {
            cent->pose.player.waterHeight = -1000000.0;
        }
    }
}

void __cdecl CG_FxSetTestPosition()
{
    const cg_s *cgameGlob; // [esp+28h] [ebp-8h]

    cgameGlob = CG_GetLocalClientGlobals(0);
    if ( cgameGlob->nextSnap )
    {
        s_testEffect[0].pos[0] = (float)(100.0 * cgameGlob->refdef.viewaxis[0][0]) + cgameGlob->refdef.vieworg[0];
        s_testEffect[0].pos[1] = (float)(100.0 * cgameGlob->refdef.viewaxis[0][1]) + cgameGlob->refdef.vieworg[1];
        s_testEffect[0].pos[2] = (float)(100.0 * cgameGlob->refdef.viewaxis[0][2]) + cgameGlob->refdef.vieworg[2];
        Com_Printf(
            21,
            "\n\nFX Testing position set to: (%f, %f, %f)\n\n",
            s_testEffect[0].pos[0],
            s_testEffect[0].pos[1],
            s_testEffect[0].pos[2]);
    }
}

void __cdecl CG_FxTest()
{
    const char *v0; // eax
    const char *v1; // eax

    if ( CG_GetLocalClientGlobals(0)->nextSnap )
    {
        if ( Cmd_Argc() < 2 )
            Com_Printf(21, "Must supply filename from base path.    Optional restart time.\n");
        v0 = Cmd_Argv(1);
        I_strncpyz(s_testEffect[0].name, v0, 64);
        if ( I_strncmp(s_testEffect[0].name, "fx/", 3) )
        {
            Com_Printf(21, "Spawning Fx %s\n", s_testEffect[0].name);
            CG_PlayTestFx(0);
            if ( Cmd_Argc() == 3 )
            {
                v1 = Cmd_Argv(2);
                s_testEffect[0].respawnTime = (int)(atof(v1) * 1000.0);
            }
            else
            {
                s_testEffect[0].respawnTime = 0;
            }
        }
        else
        {
            Com_PrintError(1, "Fx path [%s] must not include \"fx/\" \n", s_testEffect[0].name);
        }
    }
}

void __cdecl CG_PlayTestFx(int localClientNum)
{
    TestEffect *testEffect; // [esp+8h] [ebp-30h]
    const FxEffectDef *fxDef; // [esp+Ch] [ebp-2Ch]
    int time; // [esp+10h] [ebp-28h]
    float axis[3][3]; // [esp+14h] [ebp-24h] BYREF

    testEffect = &s_testEffect[localClientNum];
    fxDef = FX_Register(testEffect->name);
    //axis[0][0] = 0.0f;
    //*(_QWORD *)&axis[0][1] = __PAIR64__(LODWORD(1.0f), 0);
    //*(_QWORD *)&axis[1][0] = __PAIR64__(0, LODWORD(1.0f));
    //axis[1][2] = 0.0f;
    //*(_QWORD *)&axis[2][0] = __PAIR64__(LODWORD(1.0f), 0);
    //axis[2][2] = 0.0f;

    axis[0][0] = 0.0f;
    axis[0][1] = 0.0f;
    axis[0][2] = 1.0f;

    axis[1][0] = 1.0f;
    axis[1][1] = 0.0f;
    axis[1][2] = 0.0f;

    axis[2][0] = 0.0f;
    axis[2][1] = 1.0f;
    axis[2][2] = 0.0f;

    time = CG_GetLocalClientGlobals(localClientNum)->time;
    FX_PlayOrientedEffect(localClientNum, fxDef, time, testEffect->pos, axis);
    testEffect->time = time;
}

void __cdecl CG_ViewRegisterDvars()
{
    cg_heliKillCamFov = _Dvar_RegisterFloat(
                                                "cg_heliKillCamFov",
                                                15.0,
                                                0.1,
                                                160.0,
                                                0x80u,
                                                "Helicopter kill camera field of view.");
    cg_heliKillCamNearBlur = _Dvar_RegisterFloat(
                                                         "cg_heliKillCamNearBlur",
                                                         4.0,
                                                         4.0,
                                                         10.0,
                                                         0x80u,
                                                         "Sets the radius of the gaussian blur used by depth of field, in pixels at 640x480");
    cg_heliKillCamFarBlur = _Dvar_RegisterFloat(
                                                        "cg_heliKillCamFarBlur",
                                                        2.0,
                                                        0.0,
                                                        10.0,
                                                        0x80u,
                                                        "Sets the radius of the gaussian blur used by depth of field, in pixels at 640x480");
    cg_heliKillCamFarBlurStart = _Dvar_RegisterFloat(
                                                                 "cg_heliKillCamFarBlurStart",
                                                                 100.0,
                                                                 0.0,
                                                                 3.4028235e38,
                                                                 0x80u,
                                                                 "Helicopter kill camera distance above the helicopter.");
    cg_heliKillCamFarBlurDist = _Dvar_RegisterFloat(
                                                                "cg_heliKillCamFarBlurDist",
                                                                300.0,
                                                                0.0,
                                                                3.4028235e38,
                                                                0x80u,
                                                                "Helicopter kill camera distance above the helicopter.");
    cg_heliKillCamNearBlurStart = _Dvar_RegisterFloat(
                                                                    "cg_heliKillCamNearBlurStart",
                                                                    0.0,
                                                                    0.0,
                                                                    3.4028235e38,
                                                                    0x80u,
                                                                    "Helicopter kill camera distance above the helicopter.");
    cg_heliKillCamNearBlurEnd = _Dvar_RegisterFloat(
                                                                "cg_heliKillCamNearBlurEnd",
                                                                100.0,
                                                                0.0,
                                                                10000.0,
                                                                0x80u,
                                                                "Helicopter kill camera distance above the helicopter.");
    cg_scriptedKillCamFov = _Dvar_RegisterFloat(
                                                        "cg_scriptedKillCamFov",
                                                        50.0,
                                                        0.1,
                                                        160.0,
                                                        0x80u,
                                                        "Scripted kill camera field of view.");
    cg_scriptedKillCamCloseXYDist = _Dvar_RegisterFloat(
                                                                        "cg_scriptedKillCamCloseXYDist",
                                                                        24.0,
                                                                        0.0,
                                                                        3.4028235e38,
                                                                        0x80u,
                                                                        "Scripted kill camera closest distance in front of the bomb.");
    cg_scriptedKillCamCloseZDist = _Dvar_RegisterFloat(
                                                                     "cg_scriptedKillCamCloseZDist",
                                                                     24.0,
                                                                     0.0,
                                                                     3.4028235e38,
                                                                     0x80u,
                                                                     "Scripted kill camera closest distance above the target.");
    cg_scriptedKillCamNearBlur = _Dvar_RegisterFloat(
                                                                 "cg_scriptedKillCamNearBlur",
                                                                 4.0,
                                                                 4.0,
                                                                 10.0,
                                                                 0x80u,
                                                                 "Sets the radius of the gaussian blur used by depth of field, in pixels at 640x480");
    cg_scriptedKillCamFarBlur = _Dvar_RegisterFloat(
                                                                "cg_scriptedKillCamFarBlur",
                                                                2.0,
                                                                0.0,
                                                                10.0,
                                                                0x80u,
                                                                "Sets the radius of the gaussian blur used by depth of field, in pixels at 640x480");
    cg_scriptedKillCamFarBlurStart = _Dvar_RegisterFloat(
                                                                         "cg_scriptedKillCamFarBlurStart",
                                                                         100.0,
                                                                         0.0,
                                                                         3.4028235e38,
                                                                         0x80u,
                                                                         "Scripted kill camera distance above the airplane.");
    cg_scriptedKillCamFarBlurDist = _Dvar_RegisterFloat(
                                                                        "cg_scriptedKillCamFarBlurDist",
                                                                        300.0,
                                                                        0.0,
                                                                        3.4028235e38,
                                                                        0x80u,
                                                                        "Scripted kill camera distance above the airplane.");
    cg_scriptedKillCamNearBlurStart = _Dvar_RegisterFloat(
                                                                            "cg_scriptedKillCamNearBlurStart",
                                                                            0.0,
                                                                            0.0,
                                                                            3.4028235e38,
                                                                            0x80u,
                                                                            "Scripted kill camera distance above the airplane.");
    cg_scriptedKillCamNearBlurEnd = _Dvar_RegisterFloat(
                                                                        "cg_scriptedKillCamNearBlurEnd",
                                                                        100.0,
                                                                        0.0,
                                                                        10000.0,
                                                                        0x80u,
                                                                        "Scripted kill camera distance above the airplane.");
    cg_destructibleKillCamFov = _Dvar_RegisterFloat(
                                                                "cg_destructibleKillCamFov",
                                                                50.0,
                                                                0.1,
                                                                160.0,
                                                                0x80u,
                                                                "Destructible kill camera field of view.");
    cg_destructibleKillCamCloseXYDist = _Dvar_RegisterFloat(
                                                                                "cg_destructibleKillCamCloseXYDist",
                                                                                24.0,
                                                                                0.0,
                                                                                3.4028235e38,
                                                                                0x80u,
                                                                                "Destructible kill camera closest distance in front of the bomb.");
    cg_destructibleKillCamCloseZDist = _Dvar_RegisterFloat(
                                                                             "cg_destructibleKillCamCloseZDist",
                                                                             24.0,
                                                                             0.0,
                                                                             3.4028235e38,
                                                                             0x80u,
                                                                             "Destructible kill camera closest distance above the target.");
    cg_destructibleKillCamNearBlur = _Dvar_RegisterFloat(
                                                                         "cg_destructibleKillCamNearBlur",
                                                                         4.0,
                                                                         4.0,
                                                                         10.0,
                                                                         0x80u,
                                                                         "Sets the radius of the gaussian blur used by depth of field, in pixels at 640x480");
    cg_destructibleKillCamFarBlur = _Dvar_RegisterFloat(
                                                                        "cg_destructibleKillCamFarBlur",
                                                                        2.0,
                                                                        0.0,
                                                                        10.0,
                                                                        0x80u,
                                                                        "Sets the radius of the gaussian blur used by depth of field, in pixels at 640x480");
    cg_destructibleKillCamFarBlurStart = _Dvar_RegisterFloat(
                                                                                 "cg_destructibleKillCamFarBlurStart",
                                                                                 100.0,
                                                                                 0.0,
                                                                                 3.4028235e38,
                                                                                 0x80u,
                                                                                 "Destructible kill camera distance above the airplane.");
    cg_destructibleKillCamFarBlurDist = _Dvar_RegisterFloat(
                                                                                "cg_destructibleKillCamFarBlurDist",
                                                                                300.0,
                                                                                0.0,
                                                                                3.4028235e38,
                                                                                0x80u,
                                                                                "Destructible kill camera distance above the airplane.");
    cg_destructibleKillCamNearBlurStart = _Dvar_RegisterFloat(
                                                                                    "cg_destructibleKillCamNearBlurStart",
                                                                                    0.0,
                                                                                    0.0,
                                                                                    3.4028235e38,
                                                                                    0x80u,
                                                                                    "Destructible kill camera distance above the airplane.");
    cg_destructibleKillCamNearBlurEnd = _Dvar_RegisterFloat(
                                                                                "cg_destructibleKillCamNearBlurEnd",
                                                                                100.0,
                                                                                0.0,
                                                                                10000.0,
                                                                                0x80u,
                                                                                "Destructible kill camera distance above the airplane.");
    cg_destructibleKillCamZIncrease = _Dvar_RegisterFloat(
                                                                            "cg_destructibleKillCamZIncrease",
                                                                            15.0,
                                                                            0.0,
                                                                            10000.0,
                                                                            0x80u,
                                                                            "Height above origin for the destructibles");
    cg_destructibleKillCamRegularHeight = _Dvar_RegisterFloat(
                                                                                    "cg_destructibleKillCamRegularHeight",
                                                                                    90.0,
                                                                                    0.0,
                                                                                    10000.0,
                                                                                    0x80u,
                                                                                    "If a destructible is above this height, then just move the camera to above it");
    cg_explosiveKillCamUpDist = _Dvar_RegisterFloat(
                                                                "cg_explosiveKillCamUpDist",
                                                                10.0,
                                                                -3.4028235e38,
                                                                3.4028235e38,
                                                                0x80u,
                                                                "Explosive kill camera: distance of camera vertically from explosive.");
    cg_explosiveKillCamBackDist = _Dvar_RegisterFloat(
                                                                    "cg_explosiveKillCamBackDist",
                                                                    150.0,
                                                                    -3.4028235e38,
                                                                    3.4028235e38,
                                                                    0x80u,
                                                                    "Explosive kill camera: distance of camera backwards from explosive.");
    cg_explosiveKillCamWallOutDist = _Dvar_RegisterFloat(
                                                                         "cg_explosiveKillCamWallOutDist",
                                                                         10.0,
                                                                         -3.4028235e38,
                                                                         3.4028235e38,
                                                                         0x80u,
                                                                         "Explosive kill camera when stuck to wall: distance of camera out from wall.");
    cg_explosiveKillCamWallSideDist = _Dvar_RegisterFloat(
                                                                            "cg_explosiveKillCamWallSideDist",
                                                                            30.0,
                                                                            -3.4028235e38,
                                                                            3.4028235e38,
                                                                            0x80u,
                                                                            "Explosive kill camera when stuck to wall: distance of camera along wall from explosive.");
    cg_explosiveKillCamGroundUpDist = _Dvar_RegisterFloat(
                                                                            "cg_explosiveKillCamGroundUpDist",
                                                                            15.0,
                                                                            -3.4028235e38,
                                                                            3.4028235e38,
                                                                            0x80u,
                                                                            "Explosive kill camera when stuck to ground: distance of camera vertically from explosive.");
    cg_explosiveKillCamGroundBackDist = _Dvar_RegisterFloat(
                                                                                "cg_explosiveKillCamGroundBackDist",
                                                                                30.0,
                                                                                -3.4028235e38,
                                                                                3.4028235e38,
                                                                                0x80u,
                                                                                "Explosive kill camera when stuck to ground: distance of camera backwards from explosive.");
    cg_rocketKillCamUpDist = _Dvar_RegisterFloat(
                                                         "cg_rocketKillCamUpDist",
                                                         10.0,
                                                         -3.4028235e38,
                                                         3.4028235e38,
                                                         0x80u,
                                                         "Rocket kill camera: distance of camera vertically from rocket.");
    cg_rocketKillCamBackDist = _Dvar_RegisterFloat(
                                                             "cg_rocketKillCamBackDist",
                                                             70.0,
                                                             -3.4028235e38,
                                                             3.4028235e38,
                                                             0x80u,
                                                             "Rocket kill camera: distance of camera backwards from rocket.");
    cg_explosiveKillCamStopDist = _Dvar_RegisterFloat(
                                                                    "cg_explosiveKillCamStopDist",
                                                                    200.0,
                                                                    -3.4028235e38,
                                                                    3.4028235e38,
                                                                    0x80u,
                                                                    "Rocket and Grenade Launcher kill camera: distance from player to begin coming to rest");
    cg_explosiveKillCamStopDecelDist = _Dvar_RegisterFloat(
                                                                             "cg_explosiveKillCamStopDecelDist",
                                                                             100.0,
                                                                             -3.4028235e38,
                                                                             3.4028235e38,
                                                                             0x80u,
                                                                             "Rocket and Grenade Launcher kill camera: distance over which to decelerate when coming to rest");
    cg_dogKillCamFov = _Dvar_RegisterFloat("cg_dogKillCamFov", 90.0, 0.1, 160.0, 0x80u, "Dog kill camera field of view.");
    cg_dogKillCamForwardDist = _Dvar_RegisterFloat(
                                                             "cg_dogKillCamForwardDist",
                                                             -40.0,
                                                             -3.4028235e38,
                                                             3.4028235e38,
                                                             0x80u,
                                                             "Dog kill camera: distance of target camera vertically from dog.");
    cg_dogKillCamUpDist = _Dvar_RegisterFloat(
                                                    "cg_dogKillCamUpDist",
                                                    30.0,
                                                    -3.4028235e38,
                                                    3.4028235e38,
                                                    0x80u,
                                                    "Dog kill camera: distance of camera target vertically from dog.");
    cg_dogKillCamSideDist = _Dvar_RegisterFloat(
                                                        "cg_dogKillCamSideDist",
                                                        0.0,
                                                        -3.4028235e38,
                                                        3.4028235e38,
                                                        0x80u,
                                                        "Dog kill camera: distance of camera target vertically from dog.");
    cg_dogKillCamDistFromEyes = _Dvar_RegisterFloat(
                                                                "cg_dogKillCamDistFromEyes",
                                                                20.0,
                                                                -3.4028235e38,
                                                                3.4028235e38,
                                                                0x80u,
                                                                "Dog kill camera: distance of camera past the delta from player target to dog eye");
    cg_dogKillMinDistFromTarget = _Dvar_RegisterFloat(
                                                                    "cg_dogKillMinDistFromTarget",
                                                                    100.0,
                                                                    0.0,
                                                                    3.4028235e38,
                                                                    0x80u,
                                                                    "Dog kill camera: minimum distance that the camera will approach the target");
    cg_artilleryKillCamFov = _Dvar_RegisterFloat(
                                                         "cg_artilleryKillCamFov",
                                                         30.0,
                                                         0.1,
                                                         160.0,
                                                         0x80u,
                                                         "Artillery kill camera field of view.");
    cg_artilleryKillCamUpDist = _Dvar_RegisterFloat(
                                                                "cg_artilleryKillCamUpDist",
                                                                3.0,
                                                                -3.4028235e38,
                                                                3.4028235e38,
                                                                0x80u,
                                                                "artillery kill camera: distance of camera vertically from artillery.");
    cg_artilleryKillCamBackDist = _Dvar_RegisterFloat(
                                                                    "cg_artilleryKillCamBackDist",
                                                                    30.0,
                                                                    -3.4028235e38,
                                                                    3.4028235e38,
                                                                    0x80u,
                                                                    "artillery kill camera: distance of camera backwards from artillery.");
    cg_artilleryKillCamWallOutDist = _Dvar_RegisterFloat(
                                                                         "cg_artilleryKillCamWallOutDist",
                                                                         10.0,
                                                                         -3.4028235e38,
                                                                         3.4028235e38,
                                                                         0x80u,
                                                                         "artillery kill camera when stuck to wall: distance of camera out from wall.");
    cg_artilleryKillCamWallSideDist = _Dvar_RegisterFloat(
                                                                            "cg_artilleryKillCamWallSideDist",
                                                                            30.0,
                                                                            -3.4028235e38,
                                                                            3.4028235e38,
                                                                            0x80u,
                                                                            "artillery kill camera when stuck to wall: distance of camera along wall from artillery.");
    cg_artilleryKillCamGroundUpDist = _Dvar_RegisterFloat(
                                                                            "cg_artilleryKillCamGroundUpDist",
                                                                            15.0,
                                                                            -3.4028235e38,
                                                                            3.4028235e38,
                                                                            0x80u,
                                                                            "artillery kill camera when stuck to ground: distance of camera vertically from artillery.");
    cg_artilleryKillCamGroundBackDist = _Dvar_RegisterFloat(
                                                                                "cg_artilleryKillCamGroundBackDist",
                                                                                30.0,
                                                                                -3.4028235e38,
                                                                                3.4028235e38,
                                                                                0x80u,
                                                                                "artillery kill camera when stuck to ground: distance of camera backwards from artillery.");
    cg_turretKillCamFov = _Dvar_RegisterFloat(
                                                    "cg_turretKillCamFov",
                                                    100.0,
                                                    0.1,
                                                    160.0,
                                                    0x80u,
                                                    "Airstrike kill camera field of view.");
    cg_turretKillCamCloseXYDist = _Dvar_RegisterFloat(
                                                                    "cg_turretKillCamCloseXYDist",
                                                                    24.0,
                                                                    0.0,
                                                                    3.4028235e38,
                                                                    0x80u,
                                                                    "Airstrike kill camera closest distance in front of the bomb.");
    cg_turretKillCamCloseZDist = _Dvar_RegisterFloat(
                                                                 "cg_turretKillCamCloseZDist",
                                                                 24.0,
                                                                 0.0,
                                                                 3.4028235e38,
                                                                 0x80u,
                                                                 "Airstrike kill camera closest distance above the target.");
    cg_turretKillCamNearBlur = _Dvar_RegisterFloat(
                                                             "cg_turretKillCamNearBlur",
                                                             4.0,
                                                             4.0,
                                                             10.0,
                                                             0x80u,
                                                             "Sets the radius of the gaussian blur used by depth of field, in pixels at 640x480");
    cg_turretKillCamFarBlur = _Dvar_RegisterFloat(
                                                            "cg_turretKillCamFarBlur",
                                                            2.0,
                                                            0.0,
                                                            10.0,
                                                            0x80u,
                                                            "Sets the radius of the gaussian blur used by depth of field, in pixels at 640x480");
    cg_turretKillCamFarBlurStart = _Dvar_RegisterFloat(
                                                                     "cg_turretKillCamFarBlurStart",
                                                                     100.0,
                                                                     0.0,
                                                                     3.4028235e38,
                                                                     0x80u,
                                                                     "Airstrike kill camera distance above the airplane.");
    cg_turretKillCamFarBlurDist = _Dvar_RegisterFloat(
                                                                    "cg_turretKillCamFarBlurDist",
                                                                    300.0,
                                                                    0.0,
                                                                    3.4028235e38,
                                                                    0x80u,
                                                                    "Airstrike kill camera distance above the airplane.");
    cg_turretKillCamNearBlurStart = _Dvar_RegisterFloat(
                                                                        "cg_turretKillCamNearBlurStart",
                                                                        0.0,
                                                                        0.0,
                                                                        3.4028235e38,
                                                                        0x80u,
                                                                        "Airstrike kill camera distance above the airplane.");
    cg_turretKillCamNearBlurEnd = _Dvar_RegisterFloat(
                                                                    "cg_turretKillCamNearBlurEnd",
                                                                    25.0,
                                                                    0.0,
                                                                    10000.0,
                                                                    0x80u,
                                                                    "Airstrike kill camera distance above the airplane.");
    cg_turretKillCamHeightIncrease = _Dvar_RegisterFloat(
                                                                         "cg_turretKillCamHeightIncrease",
                                                                         60.0,
                                                                         0.0,
                                                                         10000.0,
                                                                         0x80u,
                                                                         "Airstrike kill camera distance above the airplane.");
    cg_turretKillCamBackOffset = _Dvar_RegisterFloat(
                                                                 "cg_turretKillCamBackOffset",
                                                                 0.0,
                                                                 -100.0,
                                                                 100.0,
                                                                 0x80u,
                                                                 "Move the camera to the Back of the turret by this much.");
    cg_turretKillCamSideOffset = _Dvar_RegisterFloat(
                                                                 "cg_turretKillCamSideOffset",
                                                                 0.0,
                                                                 -100.0,
                                                                 100.0,
                                                                 0x80u,
                                                                 "Move the camera to the side of the turret by this much.");
    cg_turretKillCamDistanceIncrease = _Dvar_RegisterFloat(
                                                                             "cg_turretKillCamDistanceIncrease",
                                                                             100.0,
                                                                             -100.0,
                                                                             100.0,
                                                                             0x80u,
                                                                             "Airstrike kill camera distance above the airplane.");
    cg_infraredBlurTime = _Dvar_RegisterFloat(
                                                    "cg_infraredBlurTime",
                                                    250.0,
                                                    0.0,
                                                    10000.0,
                                                    0x80u,
                                                    "Time [in millisecs] that the infrared blur lasts.");
}

void __cdecl CG_UpdateTurretKillCam(int localClientNum)
{
    long double v1; // st7
    float *refdefViewAngles; // [esp+8h] [ebp-7Ch]
    float *vieworg; // [esp+1Ch] [ebp-68h]
    float v4; // [esp+34h] [ebp-50h]
    float delta[3]; // [esp+44h] [ebp-40h] BYREF
    float lookAt[3]; // [esp+50h] [ebp-34h] BYREF
    float origin[3]; // [esp+5Ch] [ebp-28h] BYREF
    cg_s *cgameGlob; // [esp+68h] [ebp-1Ch]
    float sine; // [esp+6Ch] [ebp-18h]
    float cosine; // [esp+70h] [ebp-14h]
    float distance; // [esp+74h] [ebp-10h]
    float angles[3]; // [esp+78h] [ebp-Ch] BYREF

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob->inKillCam
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                    1541,
                    0,
                    "%s",
                    "cgameGlob->inKillCam") )
    {
        __debugbreak();
    }
    if ( cgameGlob->killCamEntityType == KC_NO_ENTITY
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                    1542,
                    0,
                    "%s",
                    "cgameGlob->killCamEntityType != KC_NO_ENTITY") )
    {
        __debugbreak();
    }
    CG_GetPlayerKillCamEntityLookAt(localClientNum, lookAt);
    CG_GetTurretEntityOrgAngles(localClientNum, origin, angles);
    v1 = (float)(angles[1] * 0.017453292);
    cosine = cos(v1);
    sine = sin(v1);
    origin[0] = (float)(cosine * cg_turretKillCamBackOffset->current.value) + origin[0];
    origin[1] = (float)(sine * cg_turretKillCamBackOffset->current.value) + origin[1];
    v4 = (float)(angles[1] + 90.0) * 0.017453292;
    cosine = cos(v4);
    sine = sin(v4);
    origin[0] = (float)(cosine * cg_turretKillCamSideOffset->current.value) + origin[0];
    origin[1] = (float)(sine * cg_turretKillCamSideOffset->current.value) + origin[1];
    origin[2] = origin[2] + cg_turretKillCamHeightIncrease->current.value;
    distance = Vec3Normalize(delta);
    vieworg = cgameGlob->refdef.vieworg;
    cgameGlob->refdef.vieworg[0] = origin[0];
    vieworg[1] = origin[1];
    vieworg[2] = origin[2];
    delta[0] = lookAt[0] - cgameGlob->refdef.vieworg[0];
    delta[1] = lookAt[1] - cgameGlob->refdef.vieworg[1];
    delta[2] = lookAt[2] - cgameGlob->refdef.vieworg[2];
    distance = Vec3Normalize(delta);
    CG_UpdateTurretKillCam(distance, &cgameGlob->refdef.dof);
    AnglesToAxis(angles, cgameGlob->refdef.viewaxis);
    AxisToAngles(cgameGlob->refdef.viewaxis, cgameGlob->refdefViewAngles);
    refdefViewAngles = cgameGlob->refdefViewAngles;
    cgameGlob->refdefViewAngles[0] = angles[0];
    refdefViewAngles[1] = angles[1];
    refdefViewAngles[2] = angles[2];
    CG_CalcFov(localClientNum, cg_turretKillCamFov->current.value);
}

void __cdecl CG_GetPlayerKillCamEntityLookAt(int localClientNum, float *lookAtPos)
{
    cg_s *cgameGlob; // [esp+4h] [ebp-4h]

    if ( !lookAtPos
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp", 1374, 0, "%s", "lookAtPos") )
    {
        __debugbreak();
    }
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob->inKillCam
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                    1377,
                    0,
                    "%s",
                    "cgameGlob->inKillCam") )
    {
        __debugbreak();
    }
    if ( cgameGlob->killCamEntityType == KC_NO_ENTITY
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                    1378,
                    0,
                    "%s",
                    "cgameGlob->killCamEntityType != KC_NO_ENTITY") )
    {
        __debugbreak();
    }
    *lookAtPos = cgameGlob->killCamLookAt[0];
    lookAtPos[1] = cgameGlob->killCamLookAt[1];
    lookAtPos[2] = cgameGlob->killCamLookAt[2];
}

void __cdecl CG_UpdateTurretKillCam(float distance, GfxDepthOfField *dof)
{
    dof->nearBlur = cg_turretKillCamNearBlur->current.value;
    dof->farBlur = cg_turretKillCamFarBlur->current.value;
    dof->nearStart = cg_turretKillCamNearBlurStart->current.value;
    dof->nearEnd = cg_turretKillCamNearBlurEnd->current.value;
    dof->farStart = distance + cg_turretKillCamFarBlurStart->current.value;
    dof->farEnd = (float)(distance + cg_turretKillCamFarBlurStart->current.value)
                            + cg_turretKillCamFarBlurDist->current.value;
}

void __cdecl CG_GetTurretEntityOrgAngles(int localClientNum, float *origin, float *angles)
{
    cg_s *cgameGlob; // [esp+18h] [ebp-8h]
    centity_s *cent; // [esp+1Ch] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob->inKillCam
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                    1505,
                    0,
                    "%s",
                    "cgameGlob->inKillCam") )
    {
        __debugbreak();
    }
    if ( cgameGlob->killCamEntityType == KC_NO_ENTITY
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                    1506,
                    0,
                    "%s",
                    "cgameGlob->killCamEntityType != KC_NO_ENTITY") )
    {
        __debugbreak();
    }
    if ( !origin
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp", 1508, 0, "%s", "origin") )
    {
        __debugbreak();
    }
    if ( !angles
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp", 1509, 0, "%s", "angles") )
    {
        __debugbreak();
    }
    if ( cgameGlob->killCamEntity == 1023
        || (cent = CG_GetEntity(localClientNum, cgameGlob->killCamEntity), ((*((unsigned int *)cent + 201) >> 1) & 1) == 0)
        || cent == (centity_s *)-108 )
    {
        *origin = cgameGlob->killCamLastEntityOrg[0];
        origin[1] = cgameGlob->killCamLastEntityOrg[1];
        origin[2] = cgameGlob->killCamLastEntityOrg[2];
        *angles = cgameGlob->killCamLastEntityAngles[0];
        angles[1] = cgameGlob->killCamLastEntityAngles[1];
        angles[2] = cgameGlob->killCamLastEntityAngles[2];
    }
    else
    {
        *origin = cent->pose.origin[0];
        origin[1] = cent->pose.origin[1];
        origin[2] = cent->pose.origin[2];
        *angles = cent->pose.angles[0];
        angles[1] = cent->pose.angles[1];
        angles[2] = cent->pose.angles[2];
        *angles = *angles + cent->pose.turret.angles.pitch;
        angles[1] = angles[1] + cent->pose.turret.angles.yaw;
    }
}

const float up[3] = { 0.0, 0.0, 1.0 };

void __cdecl CG_UpdateVehicleKillCam(int localClientNum)
{
    float v1; // [esp+8h] [ebp-98h]
    float *v2; // [esp+Ch] [ebp-94h]
    float *v3; // [esp+10h] [ebp-90h]
    float killcamDist; // [esp+2Ch] [ebp-74h]
    float forward[3]; // [esp+40h] [ebp-60h] BYREF
    float heliAngles[3]; // [esp+4Ch] [ebp-54h] BYREF
    float delta[3]; // [esp+58h] [ebp-48h] BYREF
    float lookAt[3]; // [esp+64h] [ebp-3Ch] BYREF
    const vehicle_info_t *info; // [esp+70h] [ebp-30h]
    float origin[3]; // [esp+74h] [ebp-2Ch] BYREF
    cg_s *cgameGlob; // [esp+80h] [ebp-20h]
    float right[3]; // [esp+84h] [ebp-1Ch] BYREF
    float distance; // [esp+90h] [ebp-10h]
    float heliOrigin[3]; // [esp+94h] [ebp-Ch] BYREF

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob->inKillCam
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                    1624,
                    0,
                    "%s",
                    "cgameGlob->inKillCam") )
    {
        __debugbreak();
    }
    if ( cgameGlob->killCamEntityType == KC_NO_ENTITY
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                    1625,
                    0,
                    "%s",
                    "cgameGlob->killCamEntityType != KC_NO_ENTITY") )
    {
        __debugbreak();
    }
    info = CG_GetKillCamVehicleInfo(localClientNum);
    if ( !info && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp", 1628, 0, "%s", "info") )
        __debugbreak();
    CG_GetKillCamEntityOrgAngles(localClientNum, heliOrigin, heliAngles);
    CG_GetPlayerKillCamEntityLookAt(localClientNum, lookAt);
    lookAt[2] = lookAt[2] + info->killcamZTargetOffset;
    origin[0] = heliOrigin[0];
    origin[1] = heliOrigin[1];
    origin[2] = heliOrigin[2];
    origin[2] = heliOrigin[2] + info->killcamZDist;
    delta[0] = lookAt[0] - heliOrigin[0];
    delta[1] = lookAt[1] - heliOrigin[1];
    delta[2] = lookAt[2] - origin[2];
    distance = Vec3Normalize(delta);
    if ( distance <= 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                    1639,
                    0,
                    "%s",
                    "distance > 0.0f") )
    {
        __debugbreak();
    }
    if ( info->killcamMinDist > distance )
    {
        AngleVectors(cgameGlob->killCamOldViewAngles, forward, 0, 0);
        killcamDist = info->killcamDist;
        origin[0] = (float)(killcamDist * forward[0]) + cgameGlob->killCamOldViewOrg[0];
        origin[1] = (float)(killcamDist * forward[1]) + cgameGlob->killCamOldViewOrg[1];
        origin[2] = (float)(killcamDist * forward[2]) + cgameGlob->killCamOldViewOrg[2];
        delta[0] = lookAt[0] - origin[0];
        delta[1] = lookAt[1] - origin[1];
        delta[2] = lookAt[2] - origin[2];
        distance = Vec3Normalize(delta);
        if ( distance <= 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                        1648,
                        0,
                        "%s",
                        "distance > 0.0f") )
        {
            __debugbreak();
        }
    }
    Vec3Cross(up, delta, right);
    Vec3Normalize(right);
    v3 = cgameGlob->refdef.viewaxis[1];
    cgameGlob->refdef.viewaxis[1][0] = right[0];
    v3[1] = right[1];
    v3[2] = right[2];
    v2 = cgameGlob->refdef.viewaxis[0];
    cgameGlob->refdef.viewaxis[0][0] = delta[0];
    v2[1] = delta[1];
    v2[2] = delta[2];
    Vec3Cross(cgameGlob->refdef.viewaxis[0], cgameGlob->refdef.viewaxis[1], cgameGlob->refdef.viewaxis[2]);
    Vec3Normalize(cgameGlob->refdef.viewaxis[2]);
    v1 = -info->killcamDist;
    origin[0] = (float)(v1 * delta[0]) + origin[0];
    origin[1] = (float)(v1 * delta[1]) + origin[1];
    origin[2] = (float)(v1 * delta[2]) + origin[2];
    Vec3Copy(origin, cgameGlob->refdef.vieworg);
    CG_UpdateVehicleKillCamDof(info, distance, &cgameGlob->refdef.dof);
    AxisToAngles(cgameGlob->refdef.viewaxis, cgameGlob->refdefViewAngles);
    CG_CalcFov(localClientNum, info->killcamFOV);
}

void __cdecl CG_GetKillCamEntityOrgAngles(int localClientNum, float *origin, float *angles)
{
    cg_s *cgameGlob; // [esp+8h] [ebp-4h]

    if ( !origin
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp", 1387, 0, "%s", "origin") )
    {
        __debugbreak();
    }
    if ( !angles
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp", 1388, 0, "%s", "angles") )
    {
        __debugbreak();
    }
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob->inKillCam
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                    1391,
                    0,
                    "%s",
                    "cgameGlob->inKillCam") )
    {
        __debugbreak();
    }
    if ( cgameGlob->killCamEntityType == KC_NO_ENTITY
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                    1392,
                    0,
                    "%s",
                    "cgameGlob->killCamEntityType != KC_NO_ENTITY") )
    {
        __debugbreak();
    }
    *origin = cgameGlob->killCamLastEntityOrg[0];
    origin[1] = cgameGlob->killCamLastEntityOrg[1];
    origin[2] = cgameGlob->killCamLastEntityOrg[2];
    *angles = cgameGlob->killCamLastEntityAngles[0];
    angles[1] = cgameGlob->killCamLastEntityAngles[1];
    angles[2] = cgameGlob->killCamLastEntityAngles[2];
}

const vehicle_info_t *__cdecl CG_GetKillCamVehicleInfo(int localClientNum)
{
    const vehicle_info_t *info; // [esp+8h] [ebp-10h]
    cg_s *cgameGlob; // [esp+Ch] [ebp-Ch]
    centity_s *cent; // [esp+10h] [ebp-8h]
    __int16 vehicleInfoIndex; // [esp+14h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob->inKillCam
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                    1579,
                    0,
                    "%s",
                    "cgameGlob->inKillCam") )
    {
        __debugbreak();
    }
    if ( cgameGlob->killCamEntityType == KC_NO_ENTITY
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                    1580,
                    0,
                    "%s",
                    "cgameGlob->killCamEntityType != KC_NO_ENTITY") )
    {
        __debugbreak();
    }
    vehicleInfoIndex = cgameGlob->killCamLastVehicleInfoIndex;
    if ( cgameGlob->killCamEntity != 1023 )
    {
        cent = CG_GetEntity(localClientNum, cgameGlob->killCamEntity);
        if ( ((*((unsigned int *)cent + 201) >> 1) & 1) != 0 )
            vehicleInfoIndex = cent->nextState.vehicleState.vehicleInfoIndex;
    }
    info = CG_GetVehicleInfo(vehicleInfoIndex);
    if ( !info && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp", 1592, 0, "%s", "info") )
        __debugbreak();
    cgameGlob->killCamLastVehicleInfoIndex = vehicleInfoIndex;
    return info;
}

void __cdecl CG_UpdateVehicleKillCamDof(const vehicle_info_t *info, float distance, GfxDepthOfField *dof)
{
    if ( !info && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp", 1601, 0, "%s", "info") )
        __debugbreak();
    dof->nearBlur = info->killcamNearBlur;
    dof->farBlur = info->killcamFarBlur;
    dof->nearStart = info->killcamNearBlurStart;
    dof->nearEnd = (float)(info->killcamDist + distance) - info->killcamNearBlurEnd;
    dof->farStart = (float)(info->killcamDist + distance) + info->killcamFarBlurStart;
    dof->farEnd = (float)((float)(info->killcamDist + distance) + info->killcamFarBlurStart) + info->killcamFarBlurEnd;
}

void __cdecl CG_UpdateScriptedKillCam(int localClientNum)
{
    long double v1; // [esp+8h] [ebp-68h]
    long double v2; // [esp+8h] [ebp-68h]
    long double v3; // [esp+8h] [ebp-68h]
    float *vieworg; // [esp+8h] [ebp-68h]
    float *refdefViewAngles; // [esp+18h] [ebp-58h]
    float phase; // [esp+28h] [ebp-48h]
    float sx; // [esp+30h] [ebp-40h]
    float delta[3]; // [esp+34h] [ebp-3Ch] BYREF
    float lookAt[3]; // [esp+40h] [ebp-30h] BYREF
    float scriptOrigin[3]; // [esp+4Ch] [ebp-24h] BYREF
    cg_s *cgameGlob; // [esp+58h] [ebp-18h]
    float distance; // [esp+5Ch] [ebp-14h]
    centity_s *scriptedEnt; // [esp+60h] [ebp-10h]
    float scriptAngles[3]; // [esp+64h] [ebp-Ch] BYREF

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob->inKillCam
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                    1758,
                    0,
                    "%s",
                    "cgameGlob->inKillCam") )
    {
        __debugbreak();
    }
    if ( cgameGlob->killCamEntityType == KC_NO_ENTITY
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                    1759,
                    0,
                    "%s",
                    "cgameGlob->killCamEntityType != KC_NO_ENTITY") )
    {
        __debugbreak();
    }
    CG_GetPlayerKillCamEntityLookAt(localClientNum, lookAt);
    CG_GetKillCamEntityOrgAngles(localClientNum, scriptOrigin, scriptAngles);
    scriptedEnt = CG_GetEntity(localClientNum, cgameGlob->killCamEntity);
    if ( scriptAngles[0] == 0.0 && scriptAngles[1] == 0.0 && scriptAngles[2] == 0.0 )
    {
        distance = LookAtAxisAndDistance(scriptOrigin, lookAt, cgameGlob->refdef.viewaxis);
        AxisToAngles(cgameGlob->refdef.viewaxis, cgameGlob->refdefViewAngles);
    }
    else
    {
        refdefViewAngles = cgameGlob->refdefViewAngles;
        cgameGlob->refdefViewAngles[0] = scriptAngles[0];
        refdefViewAngles[1] = scriptAngles[1];
        refdefViewAngles[2] = scriptAngles[2];
        //sx = (float)cgameGlob->time / 600.0;
        //phase = Com_CRandom() * 3.1415927;
        //__libm_sse2_sin(v1);
        //cgameGlob->refdefViewAngles[0] = cgameGlob->refdefViewAngles[0]
        //                                                             + (float)((float)((float)((float)(25.132742 * sx) + phase) * 1.8) * 0.0020000001);
        //__libm_sse2_sin(v2);
        //cgameGlob->refdefViewAngles[1] = cgameGlob->refdefViewAngles[1]
        //                                                             + (float)((float)((float)((float)(47.12389 * sx) + phase) * 1.6) * 0.0020000001);
        //__libm_sse2_sin(v3);
        //cgameGlob->refdefViewAngles[2] = cgameGlob->refdefViewAngles[2]
        //                                                             + (float)((float)((float)((float)(37.699112 * sx) + phase) * 1.0) * 0.0020000001);
        //AnglesToAxis(cgameGlob->refdefViewAngles, cgameGlob->refdef.viewaxis);
        sx = (float)cgameGlob->time * (1.0f / 600.0f);
        phase = Com_CRandom() * 3.1415927f;

        cgameGlob->refdefViewAngles[0] += sinf((25.132742f * sx) + phase) * 1.8f * 0.0020000001f;

        cgameGlob->refdefViewAngles[1] += sinf((47.12389f * sx) + phase) * 1.6f * 0.0020000001f;

        cgameGlob->refdefViewAngles[2] += sinf((37.699112f * sx) + phase) * 1.0f * 0.0020000001f;
    }
    distance = Vec3Normalize(delta);
    CG_CalcFov(localClientNum, cg_scriptedKillCamFov->current.value);
    CG_UpdateScriptedKillCamDof(distance, &cgameGlob->refdef.dof);
    CG_PerturbCamera(cgameGlob);
    vieworg = cgameGlob->refdef.vieworg;
    cgameGlob->refdef.vieworg[0] = scriptOrigin[0];
    vieworg[1] = scriptOrigin[1];
    vieworg[2] = scriptOrigin[2];
}

double __cdecl LookAtAxisAndDistance(const float *origin, const float *lookAt, float (*axis)[3])
{
    double v3; // st7
    const char *v4; // eax
    float v6; // [esp+3Ch] [ebp-14h]
    float distance; // [esp+4Ch] [ebp-4h]

    (*axis)[0] = *lookAt - *origin;
    (*axis)[1] = lookAt[1] - origin[1];
    (*axis)[2] = lookAt[2] - origin[2];
    distance = Vec3Normalize((float *)axis);
    v6 = (*axis)[0];
    ((*axis)[3]) = -((*axis)[1]);
    (*axis)[4] = v6;
    (*axis)[5] = 0.0f;
    if ( (*axis)[3] == 0.0 && (*axis)[4] == 0.0 )
        (*axis)[3] = 1.0f;
    Vec3Normalize(&(*axis)[3]);
    Vec3Cross((const float *)axis, &(*axis)[3], &(*axis)[6]);
    if ( !Vec3IsNormalized(&(*axis)[6]) )
    {
        v3 = Vec3Length(&(*axis)[6]);
        v4 = va("(%g %g %g) len %g", (*axis)[6], (*axis)[7], (*axis)[8], v3);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                        1409,
                        0,
                        "%s\n\t%s",
                        "Vec3IsNormalized( axis[2] )",
                        v4) )
            __debugbreak();
    }
    return distance;
}

void __cdecl CG_UpdateScriptedKillCamDof(float distance, GfxDepthOfField *dof)
{
    dof->nearBlur = cg_scriptedKillCamNearBlur->current.value;
    dof->farBlur = cg_scriptedKillCamFarBlur->current.value;
    dof->nearStart = cg_scriptedKillCamNearBlurStart->current.value;
    dof->nearEnd = cg_scriptedKillCamNearBlurEnd->current.value;
    dof->farStart = distance + cg_scriptedKillCamFarBlurStart->current.value;
    dof->farEnd = (float)(distance + cg_scriptedKillCamFarBlurStart->current.value)
                            + cg_scriptedKillCamFarBlurDist->current.value;
}

void __cdecl CG_UpdateDestructibleKillCam(int localClientNum)
{
    double v1; // st7
    const char *v2; // eax
    float *vieworg; // [esp+20h] [ebp-88h]
    float *v4; // [esp+44h] [ebp-64h]
    float v5; // [esp+4Ch] [ebp-5Ch]
    float *v6; // [esp+50h] [ebp-58h]
    float delta[3]; // [esp+68h] [ebp-40h] BYREF
    float lookAt[3]; // [esp+74h] [ebp-34h] BYREF
    float heightIncrease; // [esp+80h] [ebp-28h]
    float destructibleOrigin[3]; // [esp+84h] [ebp-24h] BYREF
    cg_s *cgameGlob; // [esp+90h] [ebp-18h]
    float distance; // [esp+94h] [ebp-14h]
    float destructibleAngles[3]; // [esp+98h] [ebp-10h] BYREF
    centity_s *destructibleEnt; // [esp+A4h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob->inKillCam
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                    1837,
                    0,
                    "%s",
                    "cgameGlob->inKillCam") )
    {
        __debugbreak();
    }
    if ( cgameGlob->killCamEntityType == KC_NO_ENTITY
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                    1838,
                    0,
                    "%s",
                    "cgameGlob->killCamEntityType != KC_NO_ENTITY") )
    {
        __debugbreak();
    }
    CG_GetPlayerKillCamEntityLookAt(localClientNum, lookAt);
    CG_GetKillCamEntityOrgAngles(localClientNum, destructibleOrigin, destructibleAngles);
    destructibleEnt = CG_GetEntity(localClientNum, cgameGlob->killCamEntity);
    heightIncrease = (float)(destructibleEnt->pose.absmax[2] - destructibleEnt->pose.absmin[2])
                                 + cg_destructibleKillCamZIncrease->current.value;
    destructibleOrigin[2] = destructibleOrigin[2] + heightIncrease;
    delta[0] = lookAt[0] - destructibleOrigin[0];
    delta[1] = lookAt[1] - destructibleOrigin[1];
    delta[2] = lookAt[2] - destructibleOrigin[2];
    Vec3Normalize(delta);
    v6 = cgameGlob->refdef.viewaxis[0];
    cgameGlob->refdef.viewaxis[0][0] = delta[0];
    v6[1] = delta[1];
    v6[2] = delta[2];
    v4 = cgameGlob->refdef.viewaxis[1];
    v5 = cgameGlob->refdef.viewaxis[0][0];
    cgameGlob->refdef.viewaxis[1][0] = -cgameGlob->refdef.viewaxis[0][1];
    v4[1] = v5;
    v4[2] = 0.0f;
    if ( cgameGlob->refdef.viewaxis[1][0] == 0.0 && cgameGlob->refdef.viewaxis[1][1] == 0.0 )
        cgameGlob->refdef.viewaxis[1][0] = 1.0f;
    Vec3Normalize(cgameGlob->refdef.viewaxis[1]);
    Vec3Cross(cgameGlob->refdef.viewaxis[0], cgameGlob->refdef.viewaxis[1], cgameGlob->refdef.viewaxis[2]);
    if ( !Vec3IsNormalized(cgameGlob->refdef.viewaxis[2]) )
    {
        v1 = Vec3Length(cgameGlob->refdef.viewaxis[2]);
        v2 = va(
                     "(%g %g %g) len %g",
                     cgameGlob->refdef.viewaxis[2][0],
                     cgameGlob->refdef.viewaxis[2][1],
                     cgameGlob->refdef.viewaxis[2][2],
                     v1);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                        1858,
                        0,
                        "%s\n\t%s",
                        "Vec3IsNormalized( cgameGlob->refdef.viewaxis[2] )",
                        v2) )
            __debugbreak();
    }
    AxisToAngles(cgameGlob->refdef.viewaxis, cgameGlob->refdefViewAngles);
    distance = Vec3Normalize(delta);
    CG_CalcFov(localClientNum, cg_destructibleKillCamFov->current.value);
    CG_UpdateDestructibleKillCamDof(distance, &cgameGlob->refdef.dof);
    vieworg = cgameGlob->refdef.vieworg;
    cgameGlob->refdef.vieworg[0] = destructibleOrigin[0];
    vieworg[1] = destructibleOrigin[1];
    vieworg[2] = destructibleOrigin[2];
}

void __cdecl CG_UpdateDestructibleKillCamDof(float distance, GfxDepthOfField *dof)
{
    dof->nearBlur = cg_destructibleKillCamNearBlur->current.value;
    dof->farBlur = cg_destructibleKillCamFarBlur->current.value;
    dof->nearStart = cg_destructibleKillCamNearBlurStart->current.value;
    dof->nearEnd = cg_destructibleKillCamNearBlurEnd->current.value;
    dof->farStart = distance + cg_destructibleKillCamFarBlurStart->current.value;
    dof->farEnd = (float)(distance + cg_destructibleKillCamFarBlurStart->current.value)
                            + cg_destructibleKillCamFarBlurDist->current.value;
}

void __cdecl CG_UpdateArtilleryKillCam(int localClientNum)
{
    double v1; // st7
    const char *v2; // eax
    double v3; // st7
    const char *v4; // eax
    shellshock_parms_t *ShellshockParms; // eax
    float *v6; // [esp+34h] [ebp-184h]
    float v7; // [esp+3Ch] [ebp-17Ch]
    float *v8; // [esp+4Ch] [ebp-16Ch]
    float *v9; // [esp+50h] [ebp-168h]
    float v10; // [esp+54h] [ebp-164h]
    float *vieworg; // [esp+58h] [ebp-160h]
    float v12; // [esp+5Ch] [ebp-15Ch]
    float v13; // [esp+60h] [ebp-158h]
    float v14; // [esp+64h] [ebp-154h]
    float v15; // [esp+68h] [ebp-150h]
    float value; // [esp+6Ch] [ebp-14Ch]
    float v17; // [esp+70h] [ebp-148h]
    float v18; // [esp+78h] [ebp-140h]
    float isect[3]; // [esp+B0h] [ebp-108h] BYREF
    float v20[3]; // [esp+BCh] [ebp-FCh] BYREF
    float deltaProjectedToWall[3]; // [esp+C8h] [ebp-F0h] BYREF
    float wallNormal[3]; // [esp+D4h] [ebp-E4h] BYREF
    float bombOrigin[3]; // [esp+E0h] [ebp-D8h] BYREF
    col_context_t context; // [esp+ECh] [ebp-CCh] BYREF
    float delta[3]; // [esp+114h] [ebp-A4h] BYREF
    float lookAt[3]; // [esp+120h] [ebp-98h] BYREF
    const float *cam_maxs; // [esp+12Ch] [ebp-8Ch]
    cg_s *cgameGlob; // [esp+130h] [ebp-88h]
    float bombAngles[3]; // [esp+134h] [ebp-84h] BYREF
    float distance; // [esp+140h] [ebp-78h]
    float left[3]; // [esp+144h] [ebp-74h] BYREF
    trace_t trace; // [esp+150h] [ebp-68h] BYREF
    const float *cam_mins; // [esp+18Ch] [ebp-2Ch]
    int mask; // [esp+190h] [ebp-28h]
    float up[3]; // [esp+194h] [ebp-24h] BYREF
    float campos[3]; // [esp+1A0h] [ebp-18h] BYREF
    float originalBombOrigin[3]; // [esp+1ACh] [ebp-Ch]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob->inKillCam
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                    2217,
                    0,
                    "%s",
                    "cgameGlob->inKillCam") )
    {
        __debugbreak();
    }
    if ( cgameGlob->killCamEntityType == KC_NO_ENTITY
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                    2218,
                    0,
                    "%s",
                    "cgameGlob->killCamEntityType != KC_NO_ENTITY") )
    {
        __debugbreak();
    }
    CG_GetPlayerKillCamEntityLookAt(localClientNum, lookAt);
    CG_GetKillCamEntityOrgAngles(localClientNum, bombOrigin, bombAngles);
    delta[0] = lookAt[0] - bombOrigin[0];
    delta[1] = lookAt[1] - bombOrigin[1];
    delta[2] = lookAt[2] - bombOrigin[2];
    Vec3Normalize(delta);
    left[0] = -delta[1];
    left[1] = delta[0];
    left[2] = 0.0f;
    if ( (-(delta[1])) == 0.0 && left[1] == 0.0 )
        left[0] = 1.0f;
    Vec3Normalize(left);
    Vec3Cross(delta, left, up);
    if ( !Vec3IsNormalized(up) )
    {
        v1 = Vec3Length(up);
        v2 = va("(%g %g %g) len %g", up[0], up[1], up[2], v1);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                        2232,
                        0,
                        "%s\n\t%s",
                        "Vec3IsNormalized( up )",
                        v2) )
            __debugbreak();
    }
    if ( cgameGlob->killCamEntityRestState == KC_ENT_STUCK_WALL )
    {
        AngleVectors(bombAngles, 0, 0, wallNormal);
        (v18) = -((float)((float)(delta[0] * wallNormal[0]) + (float)(delta[1] * wallNormal[1])) + (float)(delta[2] * wallNormal[2]));
        deltaProjectedToWall[0] = (float)(v18 * wallNormal[0]) + delta[0];
        deltaProjectedToWall[1] = (float)(v18 * wallNormal[1]) + delta[1];
        deltaProjectedToWall[2] = (float)(v18 * wallNormal[2]) + delta[2];
        v17 = -(cg_artilleryKillCamWallSideDist->current.value / Vec3Length(deltaProjectedToWall));
        campos[0] = (float)(v17 * deltaProjectedToWall[0]) + bombOrigin[0];
        campos[1] = (float)(v17 * deltaProjectedToWall[1]) + bombOrigin[1];
        campos[2] = (float)(v17 * deltaProjectedToWall[2]) + bombOrigin[2];
        value = cg_artilleryKillCamWallOutDist->current.value;
        campos[0] = (float)(value * wallNormal[0]) + campos[0];
        campos[1] = (float)(value * wallNormal[1]) + campos[1];
        campos[2] = (float)(value * wallNormal[2]) + campos[2];
    }
    else if ( cgameGlob->killCamEntityRestState == KC_ENT_STUCK_GROUND )
    {
        v15 = cg_artilleryKillCamGroundUpDist->current.value;
        campos[0] = (float)(v15 * up[0]) + bombOrigin[0];
        campos[1] = (float)(v15 * up[1]) + bombOrigin[1];
        campos[2] = (float)(v15 * up[2]) + bombOrigin[2];
        (v14) = -cg_artilleryKillCamGroundBackDist->current.value;
        campos[0] = (float)(v14 * delta[0]) + campos[0];
        campos[1] = (float)(v14 * delta[1]) + campos[1];
        campos[2] = (float)(v14 * delta[2]) + campos[2];
    }
    else
    {
        v13 = cg_artilleryKillCamUpDist->current.value;
        campos[0] = (float)(v13 * up[0]) + bombOrigin[0];
        campos[1] = (float)(v13 * up[1]) + bombOrigin[1];
        campos[2] = (float)(v13 * up[2]) + bombOrigin[2];
        (v12) = -cg_artilleryKillCamBackDist->current.value;
        campos[0] = (float)(v12 * delta[0]) + campos[0];
        campos[1] = (float)(v12 * delta[1]) + campos[1];
        campos[2] = (float)(v12 * delta[2]) + campos[2];
    }
    cam_mins = CAM_MINS;
    cam_maxs = CAM_MAXS;
    originalBombOrigin[0] = bombOrigin[0];
    originalBombOrigin[1] = bombOrigin[1];
    originalBombOrigin[2] = bombOrigin[2];
    mask = 17;
    //col_context_t::col_context_t(&context);
    CG_TraceCapsule(
        &trace,
        bombOrigin,
        cam_mins,
        cam_maxs,
        campos,
        cgameGlob->predictedPlayerState.clientNum,
        mask,
        &context);
    if ( trace.startsolid )
    {
        if ( cgameGlob->killCamEntityRestState == KC_ENT_AT_REST || cgameGlob->killCamEntityRestState == KC_ENT_STUCK_GROUND )
        {
            bombOrigin[2] = bombOrigin[2] + 4.0999999;
            CG_TraceCapsule(
                &trace,
                bombOrigin,
                cam_mins,
                cam_maxs,
                campos,
                cgameGlob->predictedPlayerState.clientNum,
                mask,
                &context);
        }
        else if ( cgameGlob->killCamEntityRestState == KC_ENT_STUCK_WALL )
        {
            AngleVectors(bombAngles, 0, 0, v20);
            bombOrigin[0] = (float)(4.0999999 * v20[0]) + bombOrigin[0];
            bombOrigin[1] = (float)(4.0999999 * v20[1]) + bombOrigin[1];
            bombOrigin[2] = (float)(4.0999999 * v20[2]) + bombOrigin[2];
            CG_TraceCapsule(
                &trace,
                bombOrigin,
                cam_mins,
                cam_maxs,
                campos,
                cgameGlob->predictedPlayerState.clientNum,
                mask,
                &context);
        }
        else
        {
            if ( cgameGlob->killCamEntityRestState
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                            2285,
                            0,
                            "cgameGlob->killCamEntityRestState == KC_ENT_MOVING\n\t%i, %i",
                            cgameGlob->killCamEntityRestState,
                            0) )
            {
                __debugbreak();
            }
            CG_TraceCapsule(
                &trace,
                lookAt,
                cam_mins,
                cam_maxs,
                bombOrigin,
                cgameGlob->predictedPlayerState.clientNum,
                mask,
                &context);
            Vec3Lerp(lookAt, bombOrigin, trace.fraction, bombOrigin);
            CG_TraceCapsule(
                &trace,
                bombOrigin,
                cam_mins,
                cam_maxs,
                campos,
                cgameGlob->predictedPlayerState.clientNum,
                mask,
                &context);
        }
    }
    if ( trace.startsolid )
    {
        cam_mins = vec3_origin;
        cam_maxs = vec3_origin;
        bombOrigin[0] = originalBombOrigin[0];
        bombOrigin[1] = originalBombOrigin[1];
        bombOrigin[2] = originalBombOrigin[2];
        CG_TraceCapsule(
            &trace,
            bombOrigin,
            vec3_origin,
            vec3_origin,
            campos,
            cgameGlob->predictedPlayerState.clientNum,
            mask,
            &context);
    }
    if ( trace.startsolid )
    {
        vieworg = cgameGlob->refdef.vieworg;
        cgameGlob->refdef.vieworg[0] = bombOrigin[0];
        vieworg[1] = bombOrigin[1];
        vieworg[2] = bombOrigin[2];
    }
    else
    {
        if ( trace.fraction < 1.0 )
        {
            Vec3Lerp(bombOrigin, campos, trace.fraction, isect);
            (v10) = ((float)((float)((float)(campos[0] - isect[0]) * trace.normal.vec.v[0]) + (float)((float)(campos[1] - isect[1]) * trace.normal.vec.v[1])) + (float)((float)(campos[2] - isect[2]) * trace.normal.vec.v[2]));
            campos[0] = isect[0] + (float)((float)(v10 * trace.normal.vec.v[0]) + (float)(campos[0] - isect[0]));
            campos[1] = isect[1] + (float)((float)(v10 * trace.normal.vec.v[1]) + (float)(campos[1] - isect[1]));
            campos[2] = isect[2] + (float)((float)(v10 * trace.normal.vec.v[2]) + (float)(campos[2] - isect[2]));
            CG_TraceCapsule(
                &trace,
                isect,
                cam_mins,
                cam_maxs,
                campos,
                cgameGlob->predictedPlayerState.clientNum,
                mask,
                &context);
        }
        Vec3Lerp(bombOrigin, campos, trace.fraction, cgameGlob->refdef.vieworg);
    }
    v8 = cgameGlob->refdef.viewaxis[0];
    v9 = cgameGlob->refdef.vieworg;
    cgameGlob->refdef.viewaxis[0][0] = lookAt[0] - cgameGlob->refdef.vieworg[0];
    v8[1] = lookAt[1] - v9[1];
    v8[2] = lookAt[2] - v9[2];
    distance = Vec3Normalize(cgameGlob->refdef.viewaxis[0]);
    v6 = cgameGlob->refdef.viewaxis[1];
    v7 = cgameGlob->refdef.viewaxis[0][0];
    cgameGlob->refdef.viewaxis[1][0] = -cgameGlob->refdef.viewaxis[0][1];
    v6[1] = v7;
    v6[2] = 0.0f;
    if ( cgameGlob->refdef.viewaxis[1][0] == 0.0 && cgameGlob->refdef.viewaxis[1][1] == 0.0 )
        cgameGlob->refdef.viewaxis[1][0] = 1.0f;
    Vec3Normalize(cgameGlob->refdef.viewaxis[1]);
    Vec3Cross(cgameGlob->refdef.viewaxis[0], cgameGlob->refdef.viewaxis[1], cgameGlob->refdef.viewaxis[2]);
    if ( !Vec3IsNormalized(cgameGlob->refdef.viewaxis[2]) )
    {
        v3 = Vec3Length(cgameGlob->refdef.viewaxis[2]);
        v4 = va(
                     "(%g %g %g) len %g",
                     cgameGlob->refdef.viewaxis[2][0],
                     cgameGlob->refdef.viewaxis[2][1],
                     cgameGlob->refdef.viewaxis[2][2],
                     v3);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                        2332,
                        0,
                        "%s\n\t%s",
                        "Vec3IsNormalized( cgameGlob->refdef.viewaxis[2] )",
                        v4) )
            __debugbreak();
    }
    CG_UpdateScriptedKillCamDof(distance, &cgameGlob->refdef.dof);
    AxisToAngles(cgameGlob->refdef.viewaxis, cgameGlob->refdefViewAngles);
    CG_ShakeCamera(localClientNum);
    CG_PerturbCamera(cgameGlob);
    ShellshockParms = BG_GetShellshockParms(0);
    CG_StartShellShock(cgameGlob, ShellshockParms, 0, 0);
    CG_CalcFov(localClientNum, cg_artilleryKillCamFov->current.value);
}


float last_origin[3];
void __cdecl CG_UpdateDogKillCam(int localClientNum)
{
    double v1; // st7
    const char *v2; // eax
    float *vieworg; // [esp+20h] [ebp-110h]
    float *v4; // [esp+38h] [ebp-F8h]
    float v5; // [esp+40h] [ebp-F0h]
    float *v6; // [esp+44h] [ebp-ECh]
    float v7; // [esp+54h] [ebp-DCh]
    float v8; // [esp+7Ch] [ebp-B4h]
    float v9; // [esp+84h] [ebp-ACh]
    float value; // [esp+88h] [ebp-A8h]
    float dogAngles[3]; // [esp+8Ch] [ebp-A4h] BYREF
    float dogEyesOrigin[3]; // [esp+98h] [ebp-98h]
    float delta[3]; // [esp+A4h] [ebp-8Ch] BYREF
    float lookAt[3]; // [esp+B0h] [ebp-80h] BYREF
    cg_s *cgameGlob; // [esp+BCh] [ebp-74h]
    float distance; // [esp+C0h] [ebp-70h]
    float dogEyesAxis[3][3]; // [esp+C4h] [ebp-6Ch] BYREF
    trace_t trace; // [esp+E8h] [ebp-48h] BYREF
    float dogOrigin[3]; // [esp+124h] [ebp-Ch] BYREF

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob->inKillCam
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                    2413,
                    0,
                    "%s",
                    "cgameGlob->inKillCam") )
    {
        __debugbreak();
    }
    if ( cgameGlob->killCamEntityType == KC_NO_ENTITY
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                    2414,
                    0,
                    "%s",
                    "cgameGlob->killCamEntityType != KC_NO_ENTITY") )
    {
        __debugbreak();
    }
    CG_GetPlayerKillCamEntityLookAt(localClientNum, lookAt);
    CG_GetKillCamEntityOrgAngles(localClientNum, dogOrigin, dogAngles);
    dogEyesOrigin[0] = dogOrigin[0];
    dogEyesOrigin[1] = dogOrigin[1];
    dogEyesOrigin[2] = dogOrigin[2];
    AnglesToAxis(dogAngles, dogEyesAxis);
    value = cg_dogKillCamForwardDist->current.value;
    dogEyesOrigin[0] = (float)(value * dogEyesAxis[0][0]) + dogEyesOrigin[0];
    dogEyesOrigin[1] = (float)(value * dogEyesAxis[0][1]) + dogEyesOrigin[1];
    dogEyesOrigin[2] = (float)(value * dogEyesAxis[0][2]) + dogEyesOrigin[2];
    v9 = cg_dogKillCamSideDist->current.value;
    dogEyesOrigin[0] = (float)(v9 * dogEyesAxis[1][0]) + dogEyesOrigin[0];
    dogEyesOrigin[1] = (float)(v9 * dogEyesAxis[1][1]) + dogEyesOrigin[1];
    dogEyesOrigin[2] = (float)(v9 * dogEyesAxis[1][2]) + dogEyesOrigin[2];
    v8 = cg_dogKillCamUpDist->current.value;
    dogEyesOrigin[0] = (float)(v8 * dogEyesAxis[2][0]) + dogEyesOrigin[0];
    dogEyesOrigin[1] = (float)(v8 * dogEyesAxis[2][1]) + dogEyesOrigin[1];
    dogEyesOrigin[2] = (float)(v8 * dogEyesAxis[2][2]) + dogEyesOrigin[2];
    delta[0] = lookAt[0] - dogEyesOrigin[0];
    delta[1] = lookAt[1] - dogEyesOrigin[1];
    delta[2] = lookAt[2] - dogEyesOrigin[2];
    distance = Vec3Normalize(delta);
    distance = distance + cg_dogKillCamDistFromEyes->current.value;
    dogEyesOrigin[0] = (float)((float)(-1.0 * distance) * delta[0]) + lookAt[0];
    dogEyesOrigin[1] = (float)((float)(-1.0 * distance) * delta[1]) + lookAt[1];
    dogEyesOrigin[2] = (float)((float)(-1.0 * distance) * delta[2]) + lookAt[2];
    if ( cg_dogKillMinDistFromTarget->current.value > distance )
    {
        delta[0] = lookAt[0] - cgameGlob->killCamOldViewOrg[0];
        delta[1] = lookAt[1] - cgameGlob->killCamOldViewOrg[1];
        delta[2] = lookAt[2] - cgameGlob->killCamOldViewOrg[2];
        Vec3Normalize(delta);
        v7 = -1.0 * cg_dogKillMinDistFromTarget->current.value;
        dogEyesOrigin[0] = (float)(v7 * delta[0]) + lookAt[0];
        dogEyesOrigin[1] = (float)(v7 * delta[1]) + lookAt[1];
        dogEyesOrigin[2] = (float)(v7 * delta[2]) + lookAt[2];
        delta[0] = lookAt[0] - dogEyesOrigin[0];
        delta[1] = lookAt[1] - dogEyesOrigin[1];
        delta[2] = lookAt[2] - dogEyesOrigin[2];
        distance = Vec3Normalize(delta);
    }
    v6 = cgameGlob->refdef.viewaxis[0];
    cgameGlob->refdef.viewaxis[0][0] = delta[0];
    v6[1] = delta[1];
    v6[2] = delta[2];
    v4 = cgameGlob->refdef.viewaxis[1];
    v5 = cgameGlob->refdef.viewaxis[0][0];
    cgameGlob->refdef.viewaxis[1][0] = -cgameGlob->refdef.viewaxis[0][1];
    v4[1] = v5;
    v4[2] = 0.0f;
    if ( cgameGlob->refdef.viewaxis[1][0] == 0.0 && cgameGlob->refdef.viewaxis[1][1] == 0.0 )
        cgameGlob->refdef.viewaxis[1][0] = 1.0f;
    Vec3Normalize(cgameGlob->refdef.viewaxis[1]);
    Vec3Cross(cgameGlob->refdef.viewaxis[0], cgameGlob->refdef.viewaxis[1], cgameGlob->refdef.viewaxis[2]);
    if ( !Vec3IsNormalized(cgameGlob->refdef.viewaxis[2]) )
    {
        v1 = Vec3Length(cgameGlob->refdef.viewaxis[2]);
        v2 = va(
                     "(%g %g %g) len %g",
                     cgameGlob->refdef.viewaxis[2][0],
                     cgameGlob->refdef.viewaxis[2][1],
                     cgameGlob->refdef.viewaxis[2][2],
                     v1);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                        2462,
                        0,
                        "%s\n\t%s",
                        "Vec3IsNormalized( cgameGlob->refdef.viewaxis[2] )",
                        v2) )
            __debugbreak();
    }
    vieworg = cgameGlob->refdef.vieworg;
    cgameGlob->refdef.vieworg[0] = dogEyesOrigin[0];
    vieworg[1] = dogEyesOrigin[1];
    vieworg[2] = dogEyesOrigin[2];
    last_origin[0] = dogEyesOrigin[0];
    last_origin[1] = dogEyesOrigin[1];
    last_origin[2] = dogEyesOrigin[2];
    Vec3Sub(lookAt, cgameGlob->refdef.vieworg, delta);
    distance = Vec3Normalize(delta);
    CG_UpdateScriptedKillCamDof(distance, &cgameGlob->refdef.dof);
    AxisToAngles(cgameGlob->refdef.viewaxis, cgameGlob->refdefViewAngles);
    CG_CalcFov(localClientNum, cg_dogKillCamFov->current.value);
}

bool __cdecl CG_KillCamEntityEnabled(int localClientNum)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    return cgameGlob->inKillCam && cgameGlob->killCamEntityType != KC_NO_ENTITY;
}

void __cdecl CG_InitView(int localClientNum)
{
    float zfar; // [esp+0h] [ebp-44h]
    FxCameraUpdate fxcam; // [esp+4h] [ebp-40h] BYREF
    cg_s *cgameGlob; // [esp+40h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    CG_ClearSavedPlayerState(localClientNum);
    CG_UpdateThirdPerson(localClientNum, 0);
    CG_UpdateViewOffset(localClientNum);
    CG_PredictPlayerState(localClientNum);
    CG_UpdateViewWeaponAnim(localClientNum, 0);
    CG_CalcViewValues(localClientNum);
    LerpKillCamView(localClientNum);
    FX_SetNextUpdateTime(localClientNum, cgameGlob->time);
    zfar = R_GetFarPlaneDist();
    FX_GetCameraUpdateFromRefdefAndZFar(&fxcam, &cgameGlob->refdef, zfar);
    FX_SetNextUpdateCamera(localClientNum, &fxcam);
}

void __cdecl LerpKillCamView(int localClientNum)
{
    double v1; // st7
    double v2; // st7
    double v3; // st7
    float *killCamOldViewOrg; // [esp+8h] [ebp-100h]
    float *vieworg; // [esp+Ch] [ebp-FCh]
    float *killCamOldViewAngles; // [esp+10h] [ebp-F8h]
    float *refdefViewAngles; // [esp+14h] [ebp-F4h]
    float v8; // [esp+24h] [ebp-E4h]
    float v9; // [esp+30h] [ebp-D8h]
    float v10; // [esp+3Ch] [ebp-CCh]
    float v11; // [esp+54h] [ebp-B4h]
    float anglesDiscard[3]; // [esp+68h] [ebp-A0h] BYREF
    col_context_t context; // [esp+74h] [ebp-94h] BYREF
    const float *cam_maxs; // [esp+9Ch] [ebp-6Ch]
    float entityVel[3]; // [esp+A0h] [ebp-68h] BYREF
    centity_s *cent; // [esp+ACh] [ebp-5Ch]
    int timeRemaining; // [esp+B0h] [ebp-58h]
    float lerp; // [esp+B4h] [ebp-54h]
    trace_t trace; // [esp+B8h] [ebp-50h] BYREF
    const float *cam_mins; // [esp+F4h] [ebp-14h]
    float predictedEndPos[3]; // [esp+F8h] [ebp-10h] BYREF
    cg_s *cgameGlob; // [esp+104h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->inKillCam )
    {
        if ( cgameGlob->killCamLerpEndTime > cgameGlob->time )
        {
            if ( cgameGlob->killCamLerpEndTime <= cgameGlob->oldTime
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                            2606,
                            0,
                            "cgameGlob->killCamLerpEndTime > cgameGlob->oldTime\n\t%i, %i",
                            cgameGlob->killCamLerpEndTime,
                            cgameGlob->oldTime) )
            {
                __debugbreak();
            }
            lerp = (float)cgameGlob->frametime / (float)(cgameGlob->killCamLerpEndTime - cgameGlob->oldTime);
            memset(entityVel, 0, sizeof(entityVel));
            if ( cgameGlob->killCamEntity != 1023 && cgameGlob->killCamFirstFrameRan )
            {
                cent = CG_GetEntity(localClientNum, cgameGlob->killCamEntity);
                if ( ((*((unsigned int *)cent + 201) >> 1) & 1) != 0 && cgameGlob->frametime > 0 )
                {
                    entityVel[0] = cent->pose.origin[0] - cgameGlob->killCamPrevBombOrigin[0];
                    entityVel[1] = cent->pose.origin[1] - cgameGlob->killCamPrevBombOrigin[1];
                    entityVel[2] = cent->pose.origin[2] - cgameGlob->killCamPrevBombOrigin[2];
                    v11 = 1.0 / (float)cgameGlob->frametime;
                    entityVel[0] = v11 * entityVel[0];
                    entityVel[1] = v11 * entityVel[1];
                    entityVel[2] = v11 * entityVel[2];
                }
            }
            timeRemaining = cgameGlob->killCamLerpEndTime - cgameGlob->time;
            predictedEndPos[0] = (float)((float)timeRemaining * entityVel[0]) + cgameGlob->refdef.vieworg[0];
            predictedEndPos[1] = (float)((float)timeRemaining * entityVel[1]) + cgameGlob->refdef.vieworg[1];
            predictedEndPos[2] = (float)((float)timeRemaining * entityVel[2]) + cgameGlob->refdef.vieworg[2];
            cam_mins = CAM_MINS;
            cam_maxs = CAM_MAXS;
            //col_context_t::col_context_t(&context);
            CG_TraceCapsule(
                &trace,
                cgameGlob->refdef.vieworg,
                cam_mins,
                cam_maxs,
                predictedEndPos,
                cgameGlob->predictedPlayerState.clientNum,
                2065,
                &context);
            if ( trace.startsolid )
            {
                predictedEndPos[0] = cgameGlob->refdef.vieworg[0];
                predictedEndPos[1] = cgameGlob->refdef.vieworg[1];
                predictedEndPos[2] = cgameGlob->refdef.vieworg[2];
            }
            else
            {
                Vec3Lerp(cgameGlob->refdef.vieworg, predictedEndPos, trace.fraction, predictedEndPos);
            }
            Vec3Lerp(cgameGlob->killCamOldViewOrg, predictedEndPos, lerp, cgameGlob->refdef.vieworg);
            v10 = cgameGlob->killCamOldViewAngles[0];
            v1 = AngleNormalize180(cgameGlob->refdefViewAngles[0] - v10);
            cgameGlob->refdefViewAngles[0] = v1 * lerp + v10;
            v9 = cgameGlob->killCamOldViewAngles[1];
            v2 = AngleNormalize180(cgameGlob->refdefViewAngles[1] - v9);
            cgameGlob->refdefViewAngles[1] = v2 * lerp + v9;
            v8 = cgameGlob->killCamOldViewAngles[2];
            v3 = AngleNormalize180(cgameGlob->refdefViewAngles[2] - v8);
            cgameGlob->refdefViewAngles[2] = v3 * lerp + v8;
            AnglesToAxis(cgameGlob->refdefViewAngles, cgameGlob->refdef.viewaxis);
            cgameGlob->renderingThirdPerson = TP_FOR_MODEL;
        }
        if ( cgameGlob->killCamEntityType )
        {
            CG_GetKillCamEntityOrgAngles(localClientNum, cgameGlob->killCamPrevBombOrigin, anglesDiscard);
            if ( ((LODWORD(cgameGlob->killCamPrevBombOrigin[0]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(cgameGlob->killCamPrevBombOrigin[1]) & 0x7F800000) == 0x7F800000
                 || (LODWORD(cgameGlob->killCamPrevBombOrigin[2]) & 0x7F800000) == 0x7F800000)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                            2653,
                            0,
                            "%s",
                            "!IS_NAN((cgameGlob->killCamPrevBombOrigin)[0]) && !IS_NAN((cgameGlob->killCamPrevBombOrigin)[1]) && !IS_NA"
                            "N((cgameGlob->killCamPrevBombOrigin)[2])") )
            {
                __debugbreak();
            }
            cgameGlob->killCamFirstFrameRan = 1;
        }
    }
    killCamOldViewAngles = cgameGlob->killCamOldViewAngles;
    refdefViewAngles = cgameGlob->refdefViewAngles;
    cgameGlob->killCamOldViewAngles[0] = cgameGlob->refdefViewAngles[0];
    killCamOldViewAngles[1] = refdefViewAngles[1];
    killCamOldViewAngles[2] = refdefViewAngles[2];
    killCamOldViewOrg = cgameGlob->killCamOldViewOrg;
    vieworg = cgameGlob->refdef.vieworg;
    cgameGlob->killCamOldViewOrg[0] = cgameGlob->refdef.vieworg[0];
    killCamOldViewOrg[1] = vieworg[1];
    killCamOldViewOrg[2] = vieworg[2];
}

void __cdecl CG_CalcViewValues(int localClientNum)
{
    float v1; // [esp+4h] [ebp-8Ch]
    float *v2; // [esp+8h] [ebp-88h]
    float *lastViewAngles; // [esp+10h] [ebp-80h]
    float *v4; // [esp+14h] [ebp-7Ch]
    float *v5; // [esp+18h] [ebp-78h]
    float *predictedError; // [esp+1Ch] [ebp-74h]
    float *v7; // [esp+20h] [ebp-70h]
    float *v8; // [esp+24h] [ebp-6Ch]
    float *v9; // [esp+28h] [ebp-68h]
    float *v10; // [esp+2Ch] [ebp-64h]
    float *v11; // [esp+30h] [ebp-60h]
    float *v12; // [esp+34h] [ebp-5Ch]
    float *v13; // [esp+38h] [ebp-58h]
    float v14; // [esp+3Ch] [ebp-54h]
    float *v15; // [esp+48h] [ebp-48h]
    float *v16; // [esp+4Ch] [ebp-44h]
    float *v17; // [esp+50h] [ebp-40h]
    float *v18; // [esp+54h] [ebp-3Ch]
    float *refdefViewAngles; // [esp+58h] [ebp-38h]
    float *viewangles; // [esp+5Ch] [ebp-34h]
    float *vieworg; // [esp+60h] [ebp-30h]
    float *origin; // [esp+64h] [ebp-2Ch]
    float angles[3]; // [esp+6Ch] [ebp-24h] BYREF
    int t; // [esp+78h] [ebp-18h]
    float f; // [esp+7Ch] [ebp-14h]
    CameraMode camMode; // [esp+80h] [ebp-10h]
    cg_s *cgameGlob; // [esp+84h] [ebp-Ch]
    float uiBlurRadius; // [esp+88h] [ebp-8h]
    playerState_s *ps; // [esp+8Ch] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    cgameGlob->refdef.zNear = 0.0f;
    cgameGlob->refdef.time = cgameGlob->time;
    cgameGlob->refdef.localClientNum = localClientNum;
    cgameGlob->refdef.sunVisibility = 1.0f;
    cgameGlob->refdef.noLodCullOut = 0;
    uiBlurRadius = CL_GetMenuBlurRadius(localClientNum);
    cgameGlob->refdef.blurRadius = sqrtf(
        (float)(cgDC[localClientNum].blurRadiusOut * cgDC[localClientNum].blurRadiusOut)
        + (float)(uiBlurRadius * uiBlurRadius));
    camMode = CG_UpdateCameraMode(localClientNum);
    CG_CalcFov(localClientNum, -1.0);
    CG_VisionSetApplyToRefdef(localClientNum, 0);
    if (cgameGlob->cubemapShot)
    {
        CG_CalcFov(localClientNum, -1.0);
        CG_CalcCubemapViewValues(cgameGlob);
    }
    else
    {
        CG_CalcVrect(localClientNum);
        ps = &cgameGlob->predictedPlayerState;
        if (cgameGlob->predictedPlayerState.pm_type == 5)
        {
            vieworg = cgameGlob->refdef.vieworg;
            origin = ps->origin;
            cgameGlob->refdef.vieworg[0] = ps->origin[0];
            vieworg[1] = origin[1];
            vieworg[2] = origin[2];
            refdefViewAngles = cgameGlob->refdefViewAngles;
            viewangles = ps->viewangles;
            cgameGlob->refdefViewAngles[0] = ps->viewangles[0];
            refdefViewAngles[1] = viewangles[1];
            refdefViewAngles[2] = viewangles[2];
            AnglesToAxis(cgameGlob->refdefViewAngles, cgameGlob->refdef.viewaxis);
        }
        else if (Demo_IsPlaying() && ps->pm_type == 4 && !Demo_IsMovieCamera() && ps->stats[4] < 2)
        {
            v17 = cgameGlob->refdef.vieworg;
            v18 = ps->origin;
            cgameGlob->refdef.vieworg[0] = ps->origin[0];
            v17[1] = v18[1];
            v17[2] = v18[2];
            v15 = cgameGlob->refdefViewAngles;
            v16 = ps->viewangles;
            cgameGlob->refdefViewAngles[0] = ps->viewangles[0];
            v15[1] = v16[1];
            v15[2] = v16[2];
            AnglesToAxis(cgameGlob->refdefViewAngles, cgameGlob->refdef.viewaxis);
        }
        else if (CG_KillCamEntityEnabled(localClientNum))
        {
            CG_UpdateKillCamEntity(cgameGlob->killCamEntityType, localClientNum);
            CG_VisionSetApplyToRefdef(localClientNum, 0);
        }
        else
        {
            cgameGlob->fBobCycle = BG_GetBobCycle(ps);
            if ((ps->pm_flags & 8) != 0)
            {
                if (cgameGlob->time - ps->jumpTime >= 500)
                    v14 = ps->velocity[2];
                else
                    v14 = 0.0f;
            }
            else
            {
                v14 = Vec2Length(ps->velocity);
            }
            cgameGlob->xyspeed = v14;
            v12 = cgameGlob->refdef.vieworg;
            v13 = ps->origin;
            cgameGlob->refdef.vieworg[0] = ps->origin[0];
            v12[1] = v13[1];
            v12[2] = v13[2];
            if (!cgameGlob->playerTeleported
                && (!cgameGlob->nextSnap->ps.pm_type
                    || cgameGlob->nextSnap->ps.pm_type == 2
                    || cgameGlob->nextSnap->ps.pm_type == 3)
                && cgameGlob->renderingThirdPerson == TP_OFF)
            {
                CG_SmoothCameraZ(cgameGlob);
            }
            v10 = cgameGlob->refdefViewAngles;
            v11 = ps->viewangles;
            cgameGlob->refdefViewAngles[0] = ps->viewangles[0];
            v10[1] = v11[1];
            v10[2] = v11[2];
            switch (camMode)
            {
            case CAM_VEHICLE:
                CG_CalcVehicleViewValues(localClientNum);
                v8 = cgameGlob->refdefViewAngles;
                v9 = ps->viewangles;
                cgameGlob->refdefViewAngles[0] = ps->viewangles[0];
                v8[1] = v9[1];
                v8[2] = v9[2];
                break;
            case CAM_VEHICLE_THIRDPERSON:
                CG_Calc3rdPersonVehicleViewValues(localClientNum);
                break;
            case CAM_MISSILE:
                CG_CalcMissileViewValues(localClientNum);
                break;
            case CAM_EXTRACAM:
                CG_CalcExtraCamViewValues(localClientNum);
                break;
            }
            if (camMode != CAM_VEHICLE && camMode != CAM_VEHICLE_THIRDPERSON && cg_errorDecay->current.value > 0.0)
            {
                t = cgameGlob->time - cgameGlob->predictedErrorTime;
                f = (float)(cg_errorDecay->current.value - (float)t) / cg_errorDecay->current.value;
                if (f <= 0.0 || f >= 1.0)
                {
                    cgameGlob->predictedErrorTime = 0;
                }
                else
                {
                    v5 = cgameGlob->refdef.vieworg;
                    predictedError = cgameGlob->predictedError;
                    v7 = cgameGlob->refdef.vieworg;
                    cgameGlob->refdef.vieworg[0] = (float)(f * cgameGlob->predictedError[0]) + cgameGlob->refdef.vieworg[0];
                    v5[1] = (float)(f * predictedError[1]) + v7[1];
                    v5[2] = (float)(f * predictedError[2]) + v7[2];
                }
            }
            switch (camMode)
            {
            case CAM_TURRET:
                CG_CalcTurretViewValues(localClientNum);
                break;
            case CAM_VEHICLE:
            case CAM_VEHICLE_THIRDPERSON:
                CG_OffsetVehicleView(localClientNum, camMode);
                break;
            case CAM_VEHICLE_GUNNER:
                cgameGlob->refdef.noLodCullOut = 1;
                CG_OffsetVehicleGunner(localClientNum, cgameGlob);
                break;
            default:
                if (camMode != CAM_EXTRACAM && camMode != CAM_MISSILE)
                {
                    if (camMode == CAM_RADIANT)
                    {
                        CG_RadiantCamCalcView(localClientNum);
                    }
                    else if (cgameGlob->renderingThirdPerson != TP_FOR_MODEL)
                    {
                        CG_OffsetFirstPersonView(cgameGlob);
                    }
                }
                break;
            }
            CG_ShakeCamera(localClientNum);
            CG_CalcFov(localClientNum, -1.0);
            CG_UpdateCameraTween(localClientNum);
            if (camMode == CAM_MISSILE)
            {
                CG_CalcMissileAngleValues(localClientNum);
                lastViewAngles = cgameGlob->cameraData.lastViewAngles;
                v4 = cgameGlob->refdefViewAngles;
                cgameGlob->cameraData.lastViewAngles[0] = cgameGlob->refdefViewAngles[0];
                lastViewAngles[1] = v4[1];
                lastViewAngles[2] = v4[2];
            }
            AnglesToAxis(cgameGlob->refdefViewAngles, cgameGlob->refdef.viewaxis);
            CG_ApplyViewAnimation(localClientNum);
            if (cgameGlob->renderingThirdPerson == TP_FOR_MODEL && CG_ShouldRenderThirdPerson(camMode))
            {
                CG_OffsetThirdPersonView(localClientNum);
            }
            else if (camMode == CAM_VEHICLE && (ps->otherFlags & 2) != 0 && cgameGlob->renderingThirdPerson == TP_FOR_MODEL)
            {
                CG_OffsetChaseCamView(localClientNum, camMode);
            }
            CG_PerturbCamera(cgameGlob);
            if ((float)((float)(cg_adsZScaleMax->current.value - (float)(ps->fWeaponPosFrac * ps->fWeaponPosFrac)) - 1.0) < 0.0)
                v1 = cg_adsZScaleMax->current.value - (float)(ps->fWeaponPosFrac * ps->fWeaponPosFrac);
            else
                v1 = 1.0f;
            R_SetADSZScale(localClientNum, v1);
            if (cg_thirdPerson->current.integer && cg_thirdPersonMode->current.integer == 2)
            {
                AnglesToAxis(devSavedAngles, cgameGlob->refdef.viewaxis);
                v2 = cgameGlob->refdef.vieworg;
                cgameGlob->refdef.vieworg[0] = devSavedOrigin[0];
                v2[1] = devSavedOrigin[1];
                v2[2] = devSavedOrigin[2];
            }
            else
            {
                AxisToAngles(cgameGlob->refdef.viewaxis, angles);
                CG_DevSaveCamera(angles, cgameGlob->refdef.vieworg);
            }
            CG_ExtraCamDebug_SaveView(localClientNum);
        }
    }
}

void __cdecl CG_DevSaveCamera(float *angles, float *origin)
{
    devSavedAngles[0] = *angles;
    devSavedAngles[1] = angles[1];
    devSavedAngles[2] = angles[2];
    devSavedOrigin[0] = *origin;
    devSavedOrigin[1] = origin[1];
    devSavedOrigin[2] = origin[2];
}

#if 0
void    CG_OffsetChaseCamView(int localClientNum, CameraMode camMode)
{
    long double v3; // [esp+4h] [ebp-9Ch]
    long double v4; // [esp+Ch] [ebp-94h] BYREF
    float right[3]; // [esp+1Ch] [ebp-84h] BYREF
    float up[3]; // [esp+28h] [ebp-78h]
    float v7; // [esp+34h] [ebp-6Ch]
    float *vieworg; // [esp+38h] [ebp-68h]
    float focusDist; // [esp+3Ch] [ebp-64h] BYREF
    float v10; // [esp+40h] [ebp-60h]
    float v11; // [esp+44h] [ebp-5Ch]
    float view[3]; // [esp+48h] [ebp-58h]
    float v13; // [esp+54h] [ebp-4Ch]
    float focusPoint[3]; // [esp+58h] [ebp-48h] BYREF
    float v15; // [esp+64h] [ebp-3Ch]
    float forward[3]; // [esp+68h] [ebp-38h] BYREF
    float viewAngles[4]; // [esp+74h] [ebp-2Ch] BYREF
    float focusAngles[3]; // [esp+84h] [ebp-1Ch]
    cg_s *LocalClientGlobals; // [esp+90h] [ebp-10h]
    clientActive_t *v20; // [esp+94h] [ebp-Ch]
    void *v21; // [esp+98h] [ebp-8h]
    void *retaddr; // [esp+A0h] [ebp+0h]

    v20 = a1;
    v21 = retaddr;
    LocalClientGlobals = CG_GetLocalClientGlobals(localClientNum);
    if ( !Demo_IsThirdPersonCamera()
        && (LocalClientGlobals->predictedPlayerState.otherFlags & 2) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                    644,
                    0,
                    "%s",
                    "Demo_IsThirdPersonCamera() || cgameGlob->predictedPlayerState.otherFlags & POF_FOLLOW") )
    {
        __debugbreak();
    }
    if ( cg_thirdPersonMode->current.integer == 2 )
    {
        AnglesToAxis(devSavedAngles, LocalClientGlobals->refdef.viewaxis);
        LODWORD(focusAngles[2]) = LocalClientGlobals->refdef.vieworg;
        LocalClientGlobals->refdef.vieworg[0] = devSavedOrigin[0];
        *(float *)(LODWORD(focusAngles[2]) + 4) = devSavedOrigin[1];
        *(float *)(LODWORD(focusAngles[2]) + 8) = devSavedOrigin[2];
    }
    else
    {
        if ( Demo_IsClipPlaying() || Demo_IsClipPreviewRunning() )
            Demo_LerpClipCameraValues(localClientNum);
        LODWORD(focusAngles[1]) = CL_GetLocalClientGlobals(localClientNum);
        LODWORD(focusAngles[0]) = LODWORD(focusAngles[1]) + 10248;
        viewAngles[1] = *(float *)(LODWORD(focusAngles[1]) + 10248);
        viewAngles[2] = *(float *)(LODWORD(focusAngles[1]) + 10252);
        viewAngles[3] = *(float *)(LODWORD(focusAngles[1]) + 10256);
        LODWORD(viewAngles[0]) = LODWORD(focusAngles[1]) + 10248;
        forward[0] = *(float *)(LODWORD(focusAngles[1]) + 10248);
        forward[1] = *(float *)(LODWORD(focusAngles[1]) + 10252);
        forward[2] = *(float *)(LODWORD(focusAngles[1]) + 10256);
        if ( cg_thirdPersonMode->current.integer == 1 )
        {
            forward[1] = forward[1] + LocalClientGlobals->refdefViewAngles[1];
            viewAngles[2] = viewAngles[2] + LocalClientGlobals->refdefViewAngles[1];
        }
        viewAngles[2] = AngleNormalize180(viewAngles[2]);
        forward[1] = AngleNormalize180(forward[1]);
        AngleVectors(&viewAngles[1], &focusPoint[1], 0, 0);
        LODWORD(focusPoint[0]) = LocalClientGlobals->refdef.vieworg;
        view[1] = (float)(512.0 * focusPoint[1]) + LocalClientGlobals->refdef.vieworg[0];
        view[2] = (float)(512.0 * focusPoint[2]) + LocalClientGlobals->refdef.vieworg[1];
        v13 = (float)(512.0 * v15) + LocalClientGlobals->refdef.vieworg[2];
        LODWORD(view[0]) = LocalClientGlobals->refdef.vieworg;
        focusDist = LocalClientGlobals->refdef.vieworg[0];
        v10 = LocalClientGlobals->refdef.vieworg[1];
        v11 = LocalClientGlobals->refdef.vieworg[2];
        vieworg = LocalClientGlobals->refdef.vieworg;
        view[1] = view[1] - LocalClientGlobals->refdef.vieworg[0];
        view[2] = view[2] - LocalClientGlobals->refdef.vieworg[1];
        v13 = v13 - LocalClientGlobals->refdef.vieworg[2];
        v7 = (float)(view[1] * view[1]) + (float)(view[2] * view[2]);
        up[2] = sqrtf(v7);
        if ( (float)(up[2] - 1.0) < 0.0 )
            up[1] = 1.0f;
        else
            up[1] = up[2];
        up[2] = up[1];
        up[0] = v13;
        __libm_sse2_atan2(v3, v4);
        viewAngles[1] = v13 * -57.295776;
        viewAngles[2] = viewAngles[2] - cg_thirdPersonAngle->current.value;
        AngleVectors(&viewAngles[1], &focusPoint[1], (float *)&v4 + 1, right);
        LODWORD(v4) = cg_thirdPersonRange->current.integer;
        if ( camMode == CAM_VEHICLE && LocalClientGlobals->renderingThirdPerson == TP_FOR_MODEL )
            *(float *)&v4 = 512.0f;
        focusDist = (float)(COERCE_FLOAT(LODWORD(v4) ^ _mask__NegFloat_) * focusPoint[1]) + focusDist;
        v10 = (float)(COERCE_FLOAT(LODWORD(v4) ^ _mask__NegFloat_) * focusPoint[2]) + v10;
        v11 = (float)(COERCE_FLOAT(LODWORD(v4) ^ _mask__NegFloat_) * v15) + v11;
        ThirdPersonViewTrace(
            LocalClientGlobals,
            LocalClientGlobals->refdef.vieworg,
            &focusDist,
            0x2818011,
            LocalClientGlobals->refdef.vieworg,
            1,
            1);
        AnglesToAxis(forward, LocalClientGlobals->refdef.viewaxis);
    }
}
#endif

// aislop used after fixing stack in IDA
void CG_OffsetChaseCamView(int localClientNum, CameraMode camMode)
{
    cg_s *cgameGlob;
    clientActive_t *cl;
    float viewAngles[3];
    float focusAngles[3];
    float forward[3], right[3], up[3];
    float focusPoint[3];
    float view[3];
    float focusDist, horizDist;
    float thirdPersonRange;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    iassert(Demo_IsThirdPersonCamera() || cgameGlob->predictedPlayerState.otherFlags & POF_FOLLOW);

    if (cg_thirdPersonMode->current.integer == 2)
    {
        AnglesToAxis(devSavedAngles, cgameGlob->refdef.viewaxis);
        Vec3Copy(devSavedOrigin, cgameGlob->refdef.vieworg);
        return;
    }

    if (Demo_IsClipPlaying() || Demo_IsClipPreviewRunning())
        Demo_LerpClipCameraValues(localClientNum);

    cl = CL_GetLocalClientGlobals(localClientNum);

    Vec3Copy(cl->viewangles, viewAngles);
    Vec3Copy(cl->viewangles, focusAngles);

    if (cg_thirdPersonMode->current.integer == 1)
    {
        viewAngles[YAW] += cgameGlob->refdefViewAngles[YAW];
        focusAngles[YAW] += cgameGlob->refdefViewAngles[YAW];
    }

    viewAngles[YAW] = AngleNormalize180(viewAngles[YAW]);
    focusAngles[YAW] = AngleNormalize180(focusAngles[YAW]);

    AngleVectors(viewAngles, forward, right, up);

    // focus point 512 units forward
    Vec3Mad(cgameGlob->refdef.vieworg, 512.0f, forward, focusPoint);

    Vec3Copy(cgameGlob->refdef.vieworg, view);

    // delta from view to focus
    focusPoint[0] -= view[0];
    focusPoint[1] -= view[1];
    focusPoint[2] -= view[2];

    horizDist = sqrtf(focusPoint[0] * focusPoint[0] + focusPoint[1] * focusPoint[1]);
    if (horizDist < 1.0f)
        horizDist = 1.0f;

    viewAngles[PITCH] = -atan2f(focusPoint[2], horizDist) * 57.295776f;

    focusAngles[PITCH] -= cg_thirdPersonAngle->current.value;

    AngleVectors(viewAngles, forward, right, up);

    thirdPersonRange = cg_thirdPersonRange->current.value;

    if (camMode == CAM_VEHICLE && cgameGlob->renderingThirdPerson == TP_FOR_MODEL)
        thirdPersonRange = 512.0f;

    Vec3Mad(view, -thirdPersonRange, forward, view);

    ThirdPersonViewTrace(
        cgameGlob,
        cgameGlob->refdef.vieworg,
        view,
        0x2818011,
        cgameGlob->refdef.vieworg,
        true,
        true);

    AnglesToAxis(viewAngles, cgameGlob->refdef.viewaxis);
}


// this function is actually in jedi academy, albeit quite different
#if 0
void    CG_OffsetThirdPersonView(int localClientNum)
{
    float v2; // xmm0_4
    double v3; // xmm0_8
    long double v4; // [esp-14h] [ebp-BCh]
    long double v5; // [esp-Ch] [ebp-B4h]
    float focusDist; // [esp+0h] [ebp-A8h]
    __int64 v7; // [esp+4h] [ebp-A4h] BYREF
    float v8; // [esp+Ch] [ebp-9Ch]
    float view[3]; // [esp+10h] [ebp-98h]
    float v10; // [esp+1Ch] [ebp-8Ch]
    float focusPoint[3]; // [esp+20h] [ebp-88h] BYREF
    float v12; // [esp+2Ch] [ebp-7Ch]
    float forward[3]; // [esp+30h] [ebp-78h] BYREF
    float focusAngles[4]; // [esp+3Ch] [ebp-6Ch] BYREF
    float viewAngles[3]; // [esp+4Ch] [ebp-5Ch]
    float *v16; // [esp+58h] [ebp-50h]
    int v17; // [esp+5Ch] [ebp-4Ch]
    float corpseCent[3]; // [esp+60h] [ebp-48h] OVERLAPPED BYREF
    float tagMtx[4][3]; // [esp+6Ch] [ebp-3Ch] BYREF
    DObj *obj[2]; // [esp+9Ch] [ebp-Ch] BYREF
    DObj *retaddr; // [esp+A8h] [ebp+0h]

    obj[0] = a1;
    obj[1] = retaddr;
    LODWORD(tagMtx[3][2]) = CG_GetLocalClientGlobals(localClientNum);
    if ( cg_thirdPersonMode->current.integer == 2 )
    {
        AnglesToAxis(devSavedAngles, (float (*)[3])(LODWORD(tagMtx[3][2]) + 274156));
        LODWORD(tagMtx[3][1]) = LODWORD(tagMtx[3][2]) + 274140;
        *(float *)(LODWORD(tagMtx[3][2]) + 274140) = devSavedOrigin[0];
        *(_QWORD *)(LODWORD(tagMtx[3][1]) + 4) = *(_QWORD *)&devSavedOrigin[1];
        return;
    }
    if ( (*(unsigned int *)(LODWORD(tagMtx[3][2]) + 263548) & 0x300) == 0 )
    {
        LODWORD(tagMtx[3][0]) = Com_GetClientDObj(*(unsigned __int8 *)(LODWORD(tagMtx[3][2]) + 263628), localClientNum);
        if ( !LODWORD(tagMtx[3][0])
            || !CG_DObjGetWorldTagMatrix(
                        (const cpose_t *)(LODWORD(tagMtx[3][2]) + 273220),
                        (DObj *)LODWORD(tagMtx[3][0]),
                        scr_const.j_head,
                        (float (*)[3])corpseCent,
                        tagMtx[2]) )
        {
            return;
        }
        v17 = LODWORD(tagMtx[3][2]) + 274140;
        v16 = tagMtx[2];
        *(float *)(LODWORD(tagMtx[3][2]) + 274140) = tagMtx[2][0];
        *(float *)(v17 + 4) = v16[1];
        *(float *)(v17 + 8) = v16[2];
    }
    if ( *(int *)(LODWORD(tagMtx[3][2]) + 263328) >= 9 && *(int *)(LODWORD(tagMtx[3][2]) + 263540) >= 0 )
    {
        if ( *(unsigned int *)(LODWORD(tagMtx[3][2]) + 263540) >= 4u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                        743,
                        0,
                        "cgameGlob->predictedPlayerState.corpseIndex doesn't index MAX_CLIENT_CORPSES\n\t%i not in [0, %i)",
                        *(unsigned int *)(LODWORD(tagMtx[3][2]) + 263540),
                        4) )
        {
            __debugbreak();
        }
        if ( *(int *)(LODWORD(tagMtx[3][2]) + 263540) < 4 )
        {
            LODWORD(viewAngles[2]) = CG_GetEntity(localClientNum, *(unsigned int *)(LODWORD(tagMtx[3][2]) + 263540) + 32);
            if ( ((*(unsigned int *)(LODWORD(viewAngles[2]) + 804) >> 1) & 1) != 0 )
            {
                LODWORD(viewAngles[1]) = Com_GetClientDObj(*(unsigned int *)(LODWORD(viewAngles[2]) + 488), localClientNum);
                if ( LODWORD(viewAngles[1]) )
                    CG_DObjGetWorldTagPos(
                        (const cpose_t *)LODWORD(viewAngles[2]),
                        (DObj *)LODWORD(viewAngles[1]),
                        scr_const.j_mainroot,
                        (float *)(LODWORD(tagMtx[3][2]) + 274140));
            }
        }
    }
    if ( (*(unsigned int *)(LODWORD(tagMtx[3][2]) + 263344) & 2) != 0 || Demo_IsThirdPersonCamera() )
    {
        CG_OffsetChaseCamView((clientActive_t *)obj, localClientNum, CAM_NORMAL);
    }
    else
    {
        LODWORD(viewAngles[0]) = LODWORD(tagMtx[3][2]) + 373712;
        *(_QWORD *)&focusAngles[1] = *(_QWORD *)(LODWORD(tagMtx[3][2]) + 373712);
        focusAngles[3] = *(float *)(LODWORD(tagMtx[3][2]) + 373720);
        LODWORD(focusAngles[0]) = LODWORD(tagMtx[3][2]) + 373712;
        *(_QWORD *)forward = *(_QWORD *)(LODWORD(tagMtx[3][2]) + 373712);
        forward[2] = *(float *)(LODWORD(tagMtx[3][2]) + 373720);
        if ( cg_thirdPerson->current.integer == 2 )
        {
            forward[0] = 0.0f;
            focusAngles[1] = 0.0f;
        }
        if ( *(int *)(LODWORD(tagMtx[3][2]) + 263328) >= 9 )
        {
            forward[1] = (float)*(int *)(LODWORD(tagMtx[3][2]) + 263780);
            focusAngles[2] = (float)*(int *)(LODWORD(tagMtx[3][2]) + 263780);
        }
        if ( forward[0] > 45.0 )
            forward[0] = 45.0f;
        AngleVectors(forward, &focusPoint[1], 0, 0);
        LODWORD(focusPoint[0]) = LODWORD(tagMtx[3][2]) + 274140;
        view[1] = (float)(512.0 * focusPoint[1]) + *(float *)(LODWORD(tagMtx[3][2]) + 274140);
        view[2] = (float)(512.0 * focusPoint[2]) + *(float *)(LODWORD(tagMtx[3][2]) + 274144);
        v10 = (float)(512.0 * v12) + *(float *)(LODWORD(tagMtx[3][2]) + 274148);
        LODWORD(view[0]) = LODWORD(tagMtx[3][2]) + 274140;
        v7 = *(_QWORD *)(LODWORD(tagMtx[3][2]) + 274140);
        v8 = *(float *)(LODWORD(tagMtx[3][2]) + 274148) + 8.0;
        v2 = focusAngles[1] * 0.5;
        focusAngles[1] = v2;
        focusAngles[2] = focusAngles[2] - cg_thirdPersonAngle->current.value;
        AngleVectors(&focusAngles[1], &focusPoint[1], 0, 0);
        LODWORD(focusDist) = cg_thirdPersonRange->current.integer ^ _mask__NegFloat_;
        *(float *)&v7 = (float)(focusDist * focusPoint[1]) + *(float *)&v7;
        *((float *)&v7 + 1) = (float)(focusDist * focusPoint[2]) + *((float *)&v7 + 1);
        v8 = (float)(focusDist * v12) + v8;
        ThirdPersonViewTrace(
            (cg_s *)LODWORD(tagMtx[3][2]),
            (const float *)(LODWORD(tagMtx[3][2]) + 274140),
            (const float *)&v7,
            (int)&loc_800811,
            (float *)(LODWORD(tagMtx[3][2]) + 274140),
            1,
            1);
        view[1] = view[1] - *(float *)(LODWORD(tagMtx[3][2]) + 274140);
        view[2] = view[2] - *(float *)(LODWORD(tagMtx[3][2]) + 274144);
        v10 = v10 - *(float *)(LODWORD(tagMtx[3][2]) + 274148);
        *((float *)&v5 + 1) = (float)(view[1] * view[1]) + (float)(view[2] * view[2]);
        LODWORD(v5) = sqrtf(*((float *)&v5 + 1));
        if ( *(float *)&v5 < 1.0 )
            *(float *)&v5 = 1.0f;
        *((float *)&v4 + 1) = v10;
        v3 = v10;
        __libm_sse2_atan2(v4, v5);
        *(float *)&v3 = v3;
        focusAngles[1] = *(float *)&v3 * -57.295776;
        AnglesToAxis(&focusAngles[1], (float (*)[3])(LODWORD(tagMtx[3][2]) + 274156));
    }
}
#endif

void CG_OffsetThirdPersonView(int localClientNum)
{
    cg_s *cgameGlob;
    float forward[3];
    float focusPoint[3];
    float focusDir[3];
    float view[3];
    float camTarget[3];
    float focusAngles[3];
    float viewAngles[3];
    float corpseCent[3];
    float tagMtx[3][3];
    float focusDist;
    float dist;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    if (cg_thirdPersonMode->current.integer == 2)
    {
        AnglesToAxis(devSavedAngles, cgameGlob->refdef.viewaxis);
        Vec3Copy(devSavedOrigin, cgameGlob->refdef.vieworg);
        return;
    }

    if (!(cgameGlob->predictedPlayerState.eFlags & 0x300))
    {
        DObj *obj = Com_GetClientDObj(cgameGlob->predictedPlayerState.clientNum, localClientNum);
        if (!obj ||
            !CG_DObjGetWorldTagMatrix(
                &cgameGlob->predictedPlayerEntity.pose,
                obj,
                scr_const.j_head,
                tagMtx,
                corpseCent))
        {
            return;
        }

        Vec3Copy(corpseCent, cgameGlob->refdef.vieworg);
    }

    if (cgameGlob->predictedPlayerState.pm_type >= 9 &&
        cgameGlob->predictedPlayerState.corpseIndex >= 0)
    {
        if (cgameGlob->predictedPlayerState.corpseIndex >= 4)
        {
            if (!Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                743,
                0,
                "cgameGlob->predictedPlayerState.corpseIndex doesn't index MAX_CLIENT_CORPSES\n\t%i not in [0, %i)",
                cgameGlob->predictedPlayerState.corpseIndex,
                4))
            {
                __debugbreak();
            }
        }

        if (cgameGlob->predictedPlayerState.corpseIndex < 4)
        {
            centity_s *cent =
                CG_GetEntity(localClientNum,
                    cgameGlob->predictedPlayerState.corpseIndex + 32);

            if (((*((_DWORD *)cent + 201) >> 1) & 1) != 0)
            {
                DObj *obj = Com_GetClientDObj(cent->nextState.number, localClientNum);
                if (obj)
                {
                    CG_DObjGetWorldTagPos(
                        &cent->pose,
                        obj,
                        scr_const.j_mainroot,
                        cgameGlob->refdef.vieworg);
                }
            }
        }
    }

    if ((cgameGlob->predictedPlayerState.otherFlags & 2) ||
        Demo_IsThirdPersonCamera())
    {
        CG_OffsetChaseCamView(localClientNum, CAM_NORMAL);
        return;
    }

    Vec3Copy(cgameGlob->refdefViewAngles, focusAngles);
    Vec3Copy(cgameGlob->refdefViewAngles, viewAngles);

    if (cg_thirdPerson->current.integer == 2)
    {
        viewAngles[PITCH] = 0.0f;
        focusAngles[PITCH] = 0.0f;
    }

    if (cgameGlob->predictedPlayerState.pm_type >= 9)
    {
        viewAngles[YAW] = (float)cgameGlob->predictedPlayerState.stats[STAT_DEAD_YAW];
        focusAngles[YAW] = (float)cgameGlob->predictedPlayerState.stats[STAT_DEAD_YAW];
    }

    if (viewAngles[PITCH] > 45.0f)
        viewAngles[PITCH] = 45.0f;

    AngleVectors(viewAngles, focusDir, NULL, NULL);

    view[0] = (float)(512.0f * focusDir[0]) + cgameGlob->refdef.vieworg[0];
    view[1] = (float)(512.0f * focusDir[1]) + cgameGlob->refdef.vieworg[1];
    view[2] = (float)(512.0f * focusDir[2]) + cgameGlob->refdef.vieworg[2];

    camTarget[0] = cgameGlob->refdef.vieworg[0];
    camTarget[1] = cgameGlob->refdef.vieworg[1];
    camTarget[2] = cgameGlob->refdef.vieworg[2] + 8.0f;

    focusAngles[PITCH] *= 0.5f;
    focusAngles[YAW] -= cg_thirdPersonAngle->current.value;

    AngleVectors(focusAngles, focusDir, NULL, NULL);

    focusDist = -cg_thirdPersonRange->current.value;

    camTarget[0] = (float)(focusDist * focusDir[0]) + camTarget[0];
    camTarget[1] = (float)(focusDist * focusDir[1]) + camTarget[1];
    camTarget[2] = (float)(focusDist * focusDir[2]) + camTarget[2];

    ThirdPersonViewTrace(
        cgameGlob,
        cgameGlob->refdef.vieworg,
        camTarget,
        0x800811,
        cgameGlob->refdef.vieworg,
        true,
        true);

    view[0] = view[0] - cgameGlob->refdef.vieworg[0];
    view[1] = view[1] - cgameGlob->refdef.vieworg[1];
    view[2] = view[2] - cgameGlob->refdef.vieworg[2];

    dist = sqrtf((float)(view[0] * view[0]) + (float)(view[1] * view[1]));
    if (dist < 1.0f)
        dist = 1.0f;

    focusAngles[PITCH] = -atan2f(view[2], dist) * 57.295776f;

    AnglesToAxis(focusAngles, cgameGlob->refdef.viewaxis);
}


void __cdecl CG_CalcVrect(int localClientNum)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-8h]
    const cgs_t *cgs; // [esp+4h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    cgs = CG_GetLocalClientStaticGlobals(localClientNum);
    cgameGlob->refdef.x = cgs->viewX;
    cgameGlob->refdef.y = cgs->viewY;
    cgameGlob->refdef.width = cgs->viewWidth;
    cgameGlob->refdef.height = cgs->viewHeight;
    cgameGlob->refdef.useScissorViewport = 0;
}

void __cdecl CG_SmoothCameraZ(cg_s *cgameGlob)
{
    int timeSinceStart; // [esp+4h] [ebp-10h]
    int smoothingDuration; // [esp+Ch] [ebp-8h]
    float lerp; // [esp+10h] [ebp-4h]

    if ( cgameGlob->stepViewChange != 0.0 && cgameGlob->time - cgameGlob->stepViewStart >= 0 )
    {
        timeSinceStart = cgameGlob->time - cgameGlob->stepViewStart;
        smoothingDuration = (int)(float)(cg_viewZSmoothingTime->current.value * 1000.0);
        if ( timeSinceStart < smoothingDuration )
        {
            if ( timeSinceStart >= 0 )
                lerp = (float)((float)timeSinceStart * 1.0) / (float)smoothingDuration;
            else
                lerp = 0.0f;
        }
        else
        {
            lerp = 1.0f;
        }
        cgameGlob->refdef.vieworg[2] = cgameGlob->refdef.vieworg[2]
                                                                 - (float)((float)(1.0 - lerp) * cgameGlob->stepViewChange);
    }
}

void __cdecl CG_OffsetFirstPersonView(cg_s *cgameGlob)
{
    int v1; // [esp+10h] [ebp-5Ch]
    float deltaa; // [esp+20h] [ebp-4Ch]
    float delta; // [esp+20h] [ebp-4Ch]
    float vRight[3]; // [esp+28h] [ebp-44h] BYREF
    float angles[3]; // [esp+34h] [ebp-38h] BYREF
    float f; // [esp+40h] [ebp-2Ch]
    float deltaB; // [esp+44h] [ebp-28h]
    viewState_t vs; // [esp+48h] [ebp-24h] BYREF

    if ( cgameGlob->nextSnap->ps.pm_type >= 9
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                    1091,
                    0,
                    "%s",
                    "cgameGlob->nextSnap->ps.pm_type < PM_DEAD") )
    {
        __debugbreak();
    }
    if ( cgameGlob->nextSnap->ps.pm_type != 5 && (cgameGlob->predictedPlayerState.eFlags & 0x300) == 0 )
    {
        vs.ps = &cgameGlob->predictedPlayerState;
        if ( cgameGlob->damageTime )
            v1 = cgameGlob->damageTime - vs.ps->deltaTime;
        else
            v1 = 0;
        vs.damageTime = v1;
        vs.time = cgameGlob->time - vs.ps->deltaTime;
        vs.v_dmg_pitch = cgameGlob->v_dmg_pitch;
        vs.v_dmg_roll = cgameGlob->v_dmg_roll;
        vs.xyspeed = cgameGlob->xyspeed;
        vs.frametime = (float)cgameGlob->frametime * 0.001;
        vs.fLastIdleFactor = cgameGlob->playerEntity.fLastIdleFactor;
        vs.weapIdleTime = &cgameGlob->weapIdleTime;
        BG_CalculateViewMovementAngles(&vs, angles);
        cgameGlob->refdefViewAngles[0] = cgameGlob->refdefViewAngles[0] + angles[0];
        cgameGlob->refdefViewAngles[1] = cgameGlob->refdefViewAngles[1] + angles[1];
        cgameGlob->refdefViewAngles[2] = cgameGlob->refdefViewAngles[2] + angles[2];
        cgameGlob->refdef.vieworg[2] = cgameGlob->refdef.vieworg[2] + cgameGlob->predictedPlayerState.viewHeightCurrent;
        deltaa = BG_CalculateViewMovement_VerticalBobFactor(
                             &cgameGlob->predictedPlayerState,
                             cgameGlob->fBobCycle,
                             cgameGlob->xyspeed);
        cgameGlob->refdef.vieworg[2] = cgameGlob->refdef.vieworg[2] + deltaa;
        deltaB = BG_CalculateViewMovement_HorizontalBobFactor(
                             &cgameGlob->predictedPlayerState,
                             cgameGlob->fBobCycle,
                             cgameGlob->xyspeed);
        AngleVectors(cgameGlob->refdefViewAngles, 0, vRight, 0);
        cgameGlob->refdef.vieworg[0] = (float)(deltaB * vRight[0]) + cgameGlob->refdef.vieworg[0];
        cgameGlob->refdef.vieworg[1] = (float)(deltaB * vRight[1]) + cgameGlob->refdef.vieworg[1];
        cgameGlob->refdef.vieworg[2] = (float)(deltaB * vRight[2]) + cgameGlob->refdef.vieworg[2];
        delta = (float)(cgameGlob->time - cgameGlob->landTime);
        if ( delta <= 0.0 || delta >= 150.0 )
        {
            if ( delta > 0.0 && delta < 450.0 )
            {
                f = 1.0 - (float)((float)(delta - 150.0) / 300.0);
                cgameGlob->refdef.vieworg[2] = (float)(cgameGlob->landChange * f) + cgameGlob->refdef.vieworg[2];
            }
        }
        else
        {
            f = delta / 150.0;
            cgameGlob->refdef.vieworg[2] = (float)(cgameGlob->landChange * (float)(delta / 150.0))
                                                                     + cgameGlob->refdef.vieworg[2];
        }
        AddLeanToPosition(
            cgameGlob->refdef.vieworg,
            cgameGlob->refdefViewAngles[1],
            cgameGlob->predictedPlayerState.leanf,
            16.0,
            20.0);
        if ( (float)(cgameGlob->predictedPlayerState.origin[2] + 8.0) > cgameGlob->refdef.vieworg[2] )
            cgameGlob->refdef.vieworg[2] = cgameGlob->predictedPlayerState.origin[2] + 8.0;
    }
}

void __cdecl CG_CalcCubemapViewValues(cg_s *cgameGlob)
{
    cgameGlob->refdef.x = 0;
    cgameGlob->refdef.y = 0;
    cgameGlob->refdef.width = cgameGlob->cubemapSize + 2;
    cgameGlob->refdef.height = cgameGlob->cubemapSize + 2;
    cgameGlob->refdef.tanHalfFovX = (float)(cgameGlob->cubemapSize + 2) / (float)cgameGlob->cubemapSize;
    cgameGlob->refdef.tanHalfFovY = cgameGlob->refdef.tanHalfFovX;
    cgameGlob->refdef.vieworg[0] = cgameGlob->predictedPlayerState.origin[0];
    cgameGlob->refdef.vieworg[1] = cgameGlob->predictedPlayerState.origin[1];
    cgameGlob->refdef.vieworg[2] = cgameGlob->predictedPlayerState.origin[2];
    cgameGlob->refdef.vieworg[2] = cgameGlob->refdef.vieworg[2] + cgameGlob->predictedPlayerState.viewHeightCurrent;
    switch ( cgameGlob->cubemapShot )
    {
        case CUBEMAPSHOT_RIGHT:
            cgameGlob->refdef.viewaxis[0][0] = 1.0f;
            cgameGlob->refdef.viewaxis[0][1] = 0.0f;
            cgameGlob->refdef.viewaxis[0][2] = 0.0f;
            cgameGlob->refdef.viewaxis[1][0] = 0.0f;
            cgameGlob->refdef.viewaxis[1][1] = 1.0f;
            cgameGlob->refdef.viewaxis[1][2] = 0.0f;
            cgameGlob->refdef.viewaxis[2][0] = 0.0f;
            cgameGlob->refdef.viewaxis[2][1] = 0.0f;
            cgameGlob->refdef.viewaxis[2][2] = 1.0f;
            break;
        case CUBEMAPSHOT_LEFT:
            cgameGlob->refdef.viewaxis[0][0] = -1.0f;
            cgameGlob->refdef.viewaxis[0][1] = 0.0f;
            cgameGlob->refdef.viewaxis[0][2] = 0.0f;
            cgameGlob->refdef.viewaxis[1][0] = 0.0f;
            cgameGlob->refdef.viewaxis[1][1] = -1.0f;
            cgameGlob->refdef.viewaxis[1][2] = 0.0f;
            cgameGlob->refdef.viewaxis[2][0] = 0.0f;
            cgameGlob->refdef.viewaxis[2][1] = 0.0f;
            cgameGlob->refdef.viewaxis[2][2] = 1.0f;
            break;
        case CUBEMAPSHOT_BACK:
            cgameGlob->refdef.viewaxis[0][0] = 0.0f;
            cgameGlob->refdef.viewaxis[0][1] = 1.0f;
            cgameGlob->refdef.viewaxis[0][2] = 0.0f;
            cgameGlob->refdef.viewaxis[1][0] = -1.0f;
            cgameGlob->refdef.viewaxis[1][1] = 0.0f;
            cgameGlob->refdef.viewaxis[1][2] = 0.0f;
            cgameGlob->refdef.viewaxis[2][0] = 0.0f;
            cgameGlob->refdef.viewaxis[2][1] = 0.0f;
            cgameGlob->refdef.viewaxis[2][2] = 1.0f;
            break;
        case CUBEMAPSHOT_FRONT:
            cgameGlob->refdef.viewaxis[0][0] = 0.0f;
            cgameGlob->refdef.viewaxis[0][1] = -1.0f;
            cgameGlob->refdef.viewaxis[0][2] = 0.0f;
            cgameGlob->refdef.viewaxis[1][0] = 1.0f;
            cgameGlob->refdef.viewaxis[1][1] = 0.0f;
            cgameGlob->refdef.viewaxis[1][2] = 0.0f;
            cgameGlob->refdef.viewaxis[2][0] = 0.0f;
            cgameGlob->refdef.viewaxis[2][1] = 0.0f;
            cgameGlob->refdef.viewaxis[2][2] = 1.0f;
            break;
        case CUBEMAPSHOT_UP:
            cgameGlob->refdef.viewaxis[0][0] = 0.0f;
            cgameGlob->refdef.viewaxis[0][1] = 0.0f;
            cgameGlob->refdef.viewaxis[0][2] = 1.0f;
            cgameGlob->refdef.viewaxis[1][0] = 0.0f;
            cgameGlob->refdef.viewaxis[1][1] = 1.0f;
            cgameGlob->refdef.viewaxis[1][2] = 0.0f;
            cgameGlob->refdef.viewaxis[2][0] = -1.0f;
            cgameGlob->refdef.viewaxis[2][1] = 0.0f;
            cgameGlob->refdef.viewaxis[2][2] = 0.0f;
            break;
        case CUBEMAPSHOT_DOWN:
            cgameGlob->refdef.viewaxis[0][0] = 0.0f;
            cgameGlob->refdef.viewaxis[0][1] = 0.0f;
            cgameGlob->refdef.viewaxis[0][2] = -1.0f;
            cgameGlob->refdef.viewaxis[1][0] = 0.0f;
            cgameGlob->refdef.viewaxis[1][1] = 1.0f;
            cgameGlob->refdef.viewaxis[1][2] = 0.0f;
            cgameGlob->refdef.viewaxis[2][0] = 1.0f;
            cgameGlob->refdef.viewaxis[2][1] = 0.0f;
            cgameGlob->refdef.viewaxis[2][2] = 0.0f;
            break;
        default:
            return;
    }
}

int swayTime[1];
float swayViewAngles_1[1][3];
float swayOffset_1[1][3];
float swayAngles_1[1][3];
float swayScale = 2.0f;

void __cdecl CG_CalcTurretViewValues(int localClientNum)
{
    float v1; // [esp+10h] [ebp-6Ch]
    float v2; // [esp+20h] [ebp-5Ch]
    float v3; // [esp+2Ch] [ebp-50h]
    float *v4; // [esp+34h] [ebp-48h]
    DObj *obj; // [esp+44h] [ebp-38h]
    cg_s *cgameGlob; // [esp+48h] [ebp-34h]
    const centity_s *cent; // [esp+4Ch] [ebp-30h]
    const WeaponDef *weapDef; // [esp+54h] [ebp-28h]
    float vAxis[3][3]; // [esp+58h] [ebp-24h] BYREF

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( (cgameGlob->predictedPlayerState.eFlags & 0x300) != 0 )
    {
        if ( cgameGlob->predictedPlayerState.viewlocked == PLAYERVIEWLOCK_NONE
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                        1223,
                        0,
                        "%s",
                        "ps->viewlocked") )
        {
            __debugbreak();
        }
        if ( cgameGlob->predictedPlayerState.viewlocked_entNum == 1023
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                        1224,
                        0,
                        "%s",
                        "ps->viewlocked_entNum != ENTITYNUM_NONE") )
        {
            __debugbreak();
        }
        cent = CG_GetEntity(localClientNum, cgameGlob->predictedPlayerState.viewlocked_entNum);
        obj = Com_GetClientDObj(cent->nextState.number, localClientNum);
        if ( obj )
        {
            if ( !CG_DObjGetWorldTagPos(&cent->pose, obj, scr_const.tag_player, cgameGlob->refdef.vieworg) )
                Com_Error(ERR_DROP, "Turret has no bone: tag_player");
            weapDef = BG_GetWeaponDef(cent->nextState.weapon);
            if ( cgameGlob->renderingThirdPerson == TP_OFF )
            {
                cgameGlob->refdef.vieworg[0] = cgameGlob->refdef.vieworg[0] + weapDef->vProneOfs[0];
                cgameGlob->refdef.vieworg[1] = cgameGlob->refdef.vieworg[1] + weapDef->vProneOfs[1];
                cgameGlob->refdef.vieworg[2] = cgameGlob->refdef.vieworg[2] + weapDef->vProneOfs[2];
            }
            if ( cgameGlob->predictedPlayerState.viewlocked == PLAYERVIEWLOCK_WEAPONJITTER
                && cgameGlob->renderingThirdPerson == TP_OFF )
            {
                cgameGlob->refdefViewAngles[0] = Com_CRandom() * weapDef->vertViewJitter + cgameGlob->refdefViewAngles[0];
                cgameGlob->refdefViewAngles[1] = Com_CRandom() * weapDef->horizViewJitter + cgameGlob->refdefViewAngles[1];
            }
            if ( swayTime[localClientNum] > cgameGlob->time || swayTime[localClientNum] < cgameGlob->time - 300 )
            {
                v4 = swayViewAngles_1[localClientNum];
                *v4 = cgameGlob->predictedPlayerState.viewangles[0];
                v4[1] = cgameGlob->predictedPlayerState.viewangles[1];
                v4[2] = cgameGlob->predictedPlayerState.viewangles[2];
                swayTime[localClientNum] = cgameGlob->time;
            }
            CG_CalculateGunnerOffset_Sway(
                cgameGlob->predictedPlayerState.viewangles,
                cent->nextState.weapon,
                swayViewAngles_1[localClientNum],
                swayOffset_1[localClientNum],
                swayAngles_1[localClientNum],
                swayScale,
                cgameGlob->time - swayTime[localClientNum]);
            swayTime[localClientNum] = cgameGlob->time;
            AnglesToAxis(cgameGlob->refdefViewAngles, vAxis);
            v3 = swayOffset_1[localClientNum][0];
            cgameGlob->refdef.vieworg[0] = (float)(v3 * vAxis[0][0]) + cgameGlob->refdef.vieworg[0];
            cgameGlob->refdef.vieworg[1] = (float)(v3 * vAxis[0][1]) + cgameGlob->refdef.vieworg[1];
            cgameGlob->refdef.vieworg[2] = (float)(v3 * vAxis[0][2]) + cgameGlob->refdef.vieworg[2];
            v2 = swayOffset_1[localClientNum][1];
            cgameGlob->refdef.vieworg[0] = (float)(v2 * vAxis[1][0]) + cgameGlob->refdef.vieworg[0];
            cgameGlob->refdef.vieworg[1] = (float)(v2 * vAxis[1][1]) + cgameGlob->refdef.vieworg[1];
            cgameGlob->refdef.vieworg[2] = (float)(v2 * vAxis[1][2]) + cgameGlob->refdef.vieworg[2];
            (v1) = -(swayOffset_1[localClientNum][2]);
            cgameGlob->refdef.vieworg[0] = (float)(v1 * vAxis[2][0]) + cgameGlob->refdef.vieworg[0];
            cgameGlob->refdef.vieworg[1] = (float)(v1 * vAxis[2][1]) + cgameGlob->refdef.vieworg[1];
            cgameGlob->refdef.vieworg[2] = (float)(v1 * vAxis[2][2]) + cgameGlob->refdef.vieworg[2];
        }
    }
}

void __cdecl CG_UpdateKillCamEntity(KillCamEntityType killCamEntityType, int localClientNum)
{
    switch ( killCamEntityType )
    {
        case KC_HELICOPTER:
            CG_UpdateHelicopterKillCam(killCamEntityType, localClientNum);
            break;
        case KC_DESTRUCTIBLE:
            CG_UpdateDestructibleKillCam(localClientNum);
            break;
        case KC_SCRIPTED:
            CG_UpdateScriptedKillCam(localClientNum);
            break;
        case KC_EXPLOSIVE:
        case KC_FAST_EXPLOSIVE:
        case KC_ROCKET:
            CG_UpdateExplosiveKillCam(localClientNum, killCamEntityType);
            break;
        case KC_DOG:
            CG_UpdateDogKillCam(localClientNum);
            break;
        case KC_ARTILLERY:
            CG_UpdateArtilleryKillCam(localClientNum);
            break;
        case KC_VEHICLE:
            CG_UpdateVehicleKillCam(localClientNum);
            break;
        case KC_TURRET:
            CG_UpdateTurretKillCam(localClientNum);
            break;
        default:
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp", 2723, 0, "unreachable") )
                __debugbreak();
            break;
    }
}

void __cdecl CG_UpdateHelicopterKillCam(KillCamEntityType killCamEntityType, int localClientNum)
{
    double v2; // st7
    const char *v3; // eax
    float *vieworg; // [esp+20h] [ebp-90h]
    float heliAngles[3]; // [esp+48h] [ebp-68h] BYREF
    float delta[3]; // [esp+54h] [ebp-5Ch] BYREF
    float lookAt[3]; // [esp+60h] [ebp-50h] BYREF
    float origin[3]; // [esp+6Ch] [ebp-44h] BYREF
    cg_s *cgameGlob; // [esp+78h] [ebp-38h]
    float right[3]; // [esp+7Ch] [ebp-34h] BYREF
    float backdist; // [esp+88h] [ebp-28h]
    float distance; // [esp+8Ch] [ebp-24h]
    float up[3]; // [esp+90h] [ebp-20h] BYREF
    float zdist; // [esp+9Ch] [ebp-14h]
    float updist; // [esp+A0h] [ebp-10h]
    float heliOrigin[3]; // [esp+A4h] [ebp-Ch] BYREF

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob->inKillCam
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                    1698,
                    0,
                    "%s",
                    "cgameGlob->inKillCam") )
    {
        __debugbreak();
    }
    if ( cgameGlob->killCamEntityType == KC_NO_ENTITY
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                    1699,
                    0,
                    "%s",
                    "cgameGlob->killCamEntityType != KC_NO_ENTITY") )
    {
        __debugbreak();
    }
    CG_GetKillCamEntityOrgAngles(localClientNum, heliOrigin, heliAngles);
    CG_GetPlayerKillCamEntityLookAt(localClientNum, lookAt);
    lookAt[2] = lookAt[2] - 100.0;
    zdist = cgameGlob->killCamHelicopterOffset[0];
    backdist = cgameGlob->killCamHelicopterOffset[1];
    updist = cgameGlob->killCamHelicopterOffset[2];
    origin[0] = heliOrigin[0];
    origin[1] = heliOrigin[1];
    origin[2] = heliOrigin[2] + zdist;
    delta[0] = lookAt[0] - heliOrigin[0];
    delta[1] = lookAt[1] - heliOrigin[1];
    delta[2] = lookAt[2] - (float)(heliOrigin[2] + zdist);
    Vec3Normalize(delta);
    Vec3Cross(orIdentity.axis[2], delta, right);
    Vec3Normalize(right);
    Vec3Cross(delta, right, up);
    if ( !Vec3IsNormalized(up) )
    {
        v2 = Vec3Length(up);
        v3 = va("(%g %g %g) len %g", up[0], up[1], up[2], v2);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                        1718,
                        0,
                        "%s\n\t%s",
                        "Vec3IsNormalized( up )",
                        v3) )
            __debugbreak();
    }
    origin[0] = (float)((-(backdist)) * delta[0]) + origin[0];
    origin[1] = (float)((-(backdist)) * delta[1]) + origin[1];
    origin[2] = (float)((-(backdist)) * delta[2]) + origin[2];
    origin[0] = (float)(updist * up[0]) + origin[0];
    origin[1] = (float)(updist * up[1]) + origin[1];
    origin[2] = (float)(updist * up[2]) + origin[2];
    vieworg = cgameGlob->refdef.vieworg;
    cgameGlob->refdef.vieworg[0] = origin[0];
    vieworg[1] = origin[1];
    vieworg[2] = origin[2];
    distance = LookAtAxisAndDistance(origin, lookAt, cgameGlob->refdef.viewaxis);
    if ( distance <= 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                    1725,
                    0,
                    "%s",
                    "distance > 0.0f") )
    {
        __debugbreak();
    }
    CG_UpdateHelicopterKillCamDof(distance, &cgameGlob->refdef.dof);
    AxisToAngles(cgameGlob->refdef.viewaxis, cgameGlob->refdefViewAngles);
    CG_CalcFov(localClientNum, cg_heliKillCamFov->current.value);
}

void __cdecl CG_UpdateHelicopterKillCamDof(float distance, GfxDepthOfField *dof)
{
    dof->nearBlur = cg_heliKillCamNearBlur->current.value;
    dof->farBlur = cg_heliKillCamFarBlur->current.value;
    dof->nearStart = cg_heliKillCamNearBlurStart->current.value;
    dof->nearEnd = distance - cg_heliKillCamNearBlurEnd->current.value;
    dof->farStart = distance + cg_heliKillCamFarBlurStart->current.value;
    dof->farEnd = (float)(distance + cg_heliKillCamFarBlurStart->current.value) + cg_heliKillCamFarBlurDist->current.value;
}

void __cdecl CG_UpdateExplosiveKillCam(int localClientNum, KillCamEntityType killCamEntityType)
{
    double v2; // st7
    const char *v3; // eax
    double v4; // st7
    const char *v5; // eax
    bool v6; // [esp+24h] [ebp-1F8h]
    float *killCamStoppedVel; // [esp+28h] [ebp-1F4h]
    float *v8; // [esp+50h] [ebp-1CCh]
    float v9; // [esp+54h] [ebp-1C8h]
    float *v10; // [esp+58h] [ebp-1C4h]
    float *v11; // [esp+5Ch] [ebp-1C0h]
    float *killCamPrevBombOrigin; // [esp+60h] [ebp-1BCh]
    float *killCamStoppedPos; // [esp+64h] [ebp-1B8h]
    float v14; // [esp+80h] [ebp-19Ch]
    float v15; // [esp+90h] [ebp-18Ch]
    float v16; // [esp+94h] [ebp-188h]
    float v17; // [esp+98h] [ebp-184h]
    float value; // [esp+9Ch] [ebp-180h]
    float v19; // [esp+D0h] [ebp-14Ch]
    float v20; // [esp+DCh] [ebp-140h]
    col_context_t context; // [esp+E4h] [ebp-138h] BYREF
    float dir[3]; // [esp+10Ch] [ebp-110h] BYREF
    float bombEndPos[3]; // [esp+118h] [ebp-104h] BYREF
    float desiredStopDist; // [esp+124h] [ebp-F8h]
    float speed; // [esp+160h] [ebp-BCh]
    float time; // [esp+164h] [ebp-B8h]
    float decelFactor[3]; // [esp+168h] [ebp-B4h]
    float bombOrigin[3]; // [esp+174h] [ebp-A8h] BYREF
    float delta[3]; // [esp+180h] [ebp-9Ch] BYREF
    float lookAt[3]; // [esp+18Ch] [ebp-90h] BYREF
    cg_s *cgameGlob; // [esp+198h] [ebp-84h]
    float bombAngles[3]; // [esp+19Ch] [ebp-80h] BYREF
    float distance; // [esp+1A8h] [ebp-74h]
    float left[3]; // [esp+1ACh] [ebp-70h] BYREF
    trace_t trace; // [esp+1B8h] [ebp-64h] BYREF
    float up[3]; // [esp+1F4h] [ebp-28h] BYREF
    float realBombOrigin[3]; // [esp+200h] [ebp-1Ch] BYREF
    bool shouldStop; // [esp+20Fh] [ebp-Dh]
    float campos[3]; // [esp+210h] [ebp-Ch] BYREF

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob->inKillCam
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                    2058,
                    0,
                    "%s",
                    "cgameGlob->inKillCam") )
    {
        __debugbreak();
    }
    if ( killCamEntityType == KC_NO_ENTITY
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                    2059,
                    0,
                    "%s",
                    "killCamEntityType != KC_NO_ENTITY") )
    {
        __debugbreak();
    }
    CG_GetPlayerKillCamEntityLookAt(localClientNum, lookAt);
    CG_GetKillCamEntityOrgAngles(localClientNum, realBombOrigin, bombAngles);
    if ( cgameGlob->killCamStoppedTime && cgameGlob->time > cgameGlob->killCamStoppedTime )
    {
        time = (float)(cgameGlob->time - cgameGlob->killCamStoppedTime);
        if ( time <= 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                        2073,
                        0,
                        "%s\n\t(time) = %g",
                        "(time > 0.0f)",
                        time) )
        {
            __debugbreak();
        }
        if ( time < cgameGlob->killCamStoppedDecelTime )
        {
            if ( cgameGlob->killCamStoppedDecelTime <= 0.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                            2081,
                            0,
                            "%s\n\t(cgameGlob->killCamStoppedDecelTime) = %g",
                            "(cgameGlob->killCamStoppedDecelTime > 0.0f)",
                            cgameGlob->killCamStoppedDecelTime) )
            {
                __debugbreak();
            }
            v19 = -0.5 / cgameGlob->killCamStoppedDecelTime;
            decelFactor[0] = v19 * cgameGlob->killCamStoppedVel[0];
            decelFactor[1] = v19 * cgameGlob->killCamStoppedVel[1];
            decelFactor[2] = v19 * cgameGlob->killCamStoppedVel[2];
            bombOrigin[0] = (float)((float)((float)(time * decelFactor[0]) + cgameGlob->killCamStoppedVel[0]) * time)
                                        + cgameGlob->killCamStoppedPos[0];
            bombOrigin[1] = (float)((float)((float)(time * decelFactor[1]) + cgameGlob->killCamStoppedVel[1]) * time)
                                        + cgameGlob->killCamStoppedPos[1];
            bombOrigin[2] = (float)((float)((float)(time * decelFactor[2]) + cgameGlob->killCamStoppedVel[2]) * time)
                                        + cgameGlob->killCamStoppedPos[2];
        }
        else
        {
            v20 = 0.5 * cgameGlob->killCamStoppedDecelTime;
            bombOrigin[0] = (float)(v20 * cgameGlob->killCamStoppedVel[0]) + cgameGlob->killCamStoppedPos[0];
            bombOrigin[1] = (float)(v20 * cgameGlob->killCamStoppedVel[1]) + cgameGlob->killCamStoppedPos[1];
            bombOrigin[2] = (float)(v20 * cgameGlob->killCamStoppedVel[2]) + cgameGlob->killCamStoppedPos[2];
        }
    }
    else
    {
        bombOrigin[0] = realBombOrigin[0];
        bombOrigin[1] = realBombOrigin[1];
        bombOrigin[2] = realBombOrigin[2];
    }
    delta[0] = lookAt[0] - bombOrigin[0];
    delta[1] = lookAt[1] - bombOrigin[1];
    delta[2] = lookAt[2] - bombOrigin[2];
    v6 = (float)(lookAt[0] - bombOrigin[0]) != 0.0 || delta[1] != 0.0 || delta[2] != 0.0;
    if ( !v6
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                    2095,
                    0,
                    "%s",
                    "Vec3NotZero( delta )") )
    {
        __debugbreak();
    }
    if ( delta[2] > 0.0 )
        delta[2] = 0.0f;
    Vec3Normalize(delta);
    if ( !Vec3IsNormalized(delta) )
    {
        v2 = Vec3Length(delta);
        v3 = va("(%g %g %g) len %g", delta[0], delta[1], delta[2], v2);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                        2101,
                        0,
                        "%s\n\t%s",
                        "Vec3IsNormalized( delta )",
                        v3) )
            __debugbreak();
    }
    left[0] = -delta[1];
    left[1] = delta[0];
    left[2] = 0.0f;
    if ( (-(delta[1])) == 0.0 && left[1] == 0.0 )
        left[0] = 1.0f;
    Vec3Normalize(left);
    Vec3Cross(delta, left, up);
    if ( !Vec3IsNormalized(up) )
    {
        v4 = Vec3Length(up);
        v5 = va("(%g %g %g) len %g", up[0], up[1], up[2], v4);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                        2108,
                        0,
                        "%s\n\t%s",
                        "Vec3IsNormalized( up )",
                        v5) )
            __debugbreak();
    }
    if ( cgameGlob->killCamEntityRestState == KC_ENT_STUCK_GROUND )
    {
        value = cg_explosiveKillCamGroundUpDist->current.value;
        campos[0] = (float)(value * up[0]) + bombOrigin[0];
        campos[1] = (float)(value * up[1]) + bombOrigin[1];
        campos[2] = (float)(value * up[2]) + bombOrigin[2];
        (v17) = -cg_explosiveKillCamGroundBackDist->current.value;
        campos[0] = (float)(v17 * delta[0]) + campos[0];
        campos[1] = (float)(v17 * delta[1]) + campos[1];
        campos[2] = (float)(v17 * delta[2]) + campos[2];
    }
    else
    {
        v16 = cg_explosiveKillCamUpDist->current.value;
        campos[0] = (float)(v16 * up[0]) + bombOrigin[0];
        campos[1] = (float)(v16 * up[1]) + bombOrigin[1];
        campos[2] = (float)(v16 * up[2]) + bombOrigin[2];
        (v15) = -cg_explosiveKillCamBackDist->current.value;
        campos[0] = (float)(v15 * delta[0]) + campos[0];
        campos[1] = (float)(v15 * delta[1]) + campos[1];
        campos[2] = (float)(v15 * delta[2]) + campos[2];
    }
    CG_KillcamCameraTrace(
        bombOrigin,
        bombAngles,
        campos,
        cgameGlob->predictedPlayerState.clientNum,
        cgameGlob->killCamEntityRestState,
        cgameGlob->refdef.vieworg);
    distance = LookAtBothPoints(lookAt, realBombOrigin, cgameGlob->refdef.vieworg, cgameGlob->refdef.viewaxis);
    if ( killCamEntityType == KC_ROCKET || killCamEntityType == KC_FAST_EXPLOSIVE )
    {
        v14 = cg_explosiveKillCamStopDist->current.value;
        shouldStop = v14 * v14 > Vec3DistanceSq(lookAt, realBombOrigin);
        if ( shouldStop && !cgameGlob->killCamStoppedTime && cgameGlob->killCamFirstFrameRan )
        {
            trace_t results; // [esp+128h] [ebp-F4h] BYREF
            nanassertvec3(cgameGlob->killCamPrevBombOrigin);
            nanassertvec3(realBombOrigin);
            
            cgameGlob->killCamStoppedTime = cgameGlob->time;
            killCamStoppedPos = cgameGlob->killCamStoppedPos;
            cgameGlob->killCamStoppedPos[0] = realBombOrigin[0];
            killCamStoppedPos[1] = realBombOrigin[1];
            killCamStoppedPos[2] = realBombOrigin[2];
            if ( cgameGlob->frametime <= 0 )
            {
                killCamStoppedVel = cgameGlob->killCamStoppedVel;
                cgameGlob->killCamStoppedVel[0] = 0.0f;
                killCamStoppedVel[1] = 0.0f;
                killCamStoppedVel[2] = 0.0f;
                memset(dir, 0, sizeof(dir));
                speed = 0.0f;
            }
            else
            {
                v11 = cgameGlob->killCamStoppedVel;
                killCamPrevBombOrigin = cgameGlob->killCamPrevBombOrigin;
                cgameGlob->killCamStoppedVel[0] = realBombOrigin[0] - cgameGlob->killCamPrevBombOrigin[0];
                v11[1] = realBombOrigin[1] - killCamPrevBombOrigin[1];
                v11[2] = realBombOrigin[2] - killCamPrevBombOrigin[2];
                v8 = cgameGlob->killCamStoppedVel;
                v9 = 1.0 / (float)cgameGlob->frametime;
                v10 = cgameGlob->killCamStoppedVel;
                cgameGlob->killCamStoppedVel[0] = v9 * cgameGlob->killCamStoppedVel[0];
                v8[1] = v9 * v10[1];
                v8[2] = v9 * v10[2];
                if ( ((LODWORD(cgameGlob->killCamStoppedVel[0]) & 0x7F800000) == 0x7F800000
                     || (LODWORD(cgameGlob->killCamStoppedVel[1]) & 0x7F800000) == 0x7F800000
                     || (LODWORD(cgameGlob->killCamStoppedVel[2]) & 0x7F800000) == 0x7F800000)
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                                2159,
                                0,
                                "%s",
                                "!IS_NAN((cgameGlob->killCamStoppedVel)[0]) && !IS_NAN((cgameGlob->killCamStoppedVel)[1]) && !IS_NAN((cga"
                                "meGlob->killCamStoppedVel)[2])") )
                {
                    __debugbreak();
                }
                speed = Vec3NormalizeTo(cgameGlob->killCamStoppedVel, dir);
                if ( ((LODWORD(dir[0]) & 0x7F800000) == 0x7F800000
                     || (LODWORD(dir[1]) & 0x7F800000) == 0x7F800000
                     || (LODWORD(dir[2]) & 0x7F800000) == 0x7F800000)
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                                2162,
                                0,
                                "%s",
                                "!IS_NAN((dir)[0]) && !IS_NAN((dir)[1]) && !IS_NAN((dir)[2])") )
                {
                    __debugbreak();
                }
            }
            desiredStopDist = cg_explosiveKillCamStopDecelDist->current.value;
            if ( speed > 0.0 )
            {
                bombEndPos[0] = (float)(desiredStopDist * dir[0]) + realBombOrigin[0];
                bombEndPos[1] = (float)(desiredStopDist * dir[1]) + realBombOrigin[1];
                bombEndPos[2] = (float)(desiredStopDist * dir[2]) + realBombOrigin[2];
                //col_context_t::col_context_t(&context);
                CG_TraceCapsule(
                    &results,
                    realBombOrigin,
                    vec3_origin,
                    vec3_origin,
                    bombEndPos,
                    cgameGlob->predictedPlayerState.clientNum,
                    2065,
                    &context);
                desiredStopDist = desiredStopDist * results.fraction;
                cgameGlob->killCamStoppedDecelTime = (float)(2.0 * desiredStopDist) / speed;
            }
            else
            {
                cgameGlob->killCamStoppedDecelTime = 0.0f;
            }
        }
    }
    CG_UpdateScriptedKillCamDof(distance, &cgameGlob->refdef.dof);
    AxisToAngles(cgameGlob->refdef.viewaxis, cgameGlob->refdefViewAngles);
    CG_ShakeCamera(localClientNum);
    CG_PerturbCamera(cgameGlob);
    CG_CalcFov(localClientNum, cg_scriptedKillCamFov->current.value);
}

double __cdecl LookAtBothPoints(const float *point1, const float *point2, const float *campos, float (*lookaxis)[3])
{
    double v4; // st7
    const char *v5; // eax
    float v7; // [esp+30h] [ebp-74h]
    float axisOfRotation[3]; // [esp+68h] [ebp-3Ch] BYREF
    float lerp; // [esp+74h] [ebp-30h]
    float lookDir[3]; // [esp+78h] [ebp-2Ch] BYREF
    float point1dir[3]; // [esp+84h] [ebp-20h] BYREF
    float distance; // [esp+90h] [ebp-14h]
    float point2dir[3]; // [esp+94h] [ebp-10h] BYREF
    float dot; // [esp+A0h] [ebp-4h]

    point1dir[0] = *point1 - *campos;
    point1dir[1] = point1[1] - campos[1];
    point1dir[2] = point1[2] - campos[2];
    distance = Vec3Normalize(point1dir);
    point2dir[0] = *point2 - *campos;
    point2dir[1] = point2[1] - campos[1];
    point2dir[2] = point2[2] - campos[2];
    Vec3Normalize(point2dir);
    lookDir[0] = (float)(point1dir[0] + point2dir[0]) * 0.5;
    lookDir[1] = (float)(point1dir[1] + point2dir[1]) * 0.5;
    lookDir[2] = (float)(point1dir[2] + point2dir[2]) * 0.5;
    Vec3Normalize(lookDir);
    dot = (float)((float)(lookDir[0] * point1dir[0]) + (float)(lookDir[1] * point1dir[1]))
            + (float)(lookDir[2] * point1dir[2]);
    if ( dot < 0.98480779 )
    {
        if ( dot >= 0.86602539 )
        {
            Vec3Cross(point1dir, point2dir, axisOfRotation);
            Vec3Normalize(axisOfRotation);
            RotatePointAroundVector(lookDir, axisOfRotation, point1dir, 10.0);
            lerp = (float)(dot - 0.98480779) / (float)(0.86602539 - 0.98480779);
            Vec3Lerp(lookDir, point1dir, lerp, lookDir);
        }
        else
        {
            lookDir[0] = point1dir[0];
            lookDir[1] = point1dir[1];
            lookDir[2] = point1dir[2];
        }
    }
    if ( lookDir[2] < -0.94999999 )
    {
        if ( lookDir[2] >= -0.98000002 )
        {
            Vec3Lerp(point1dir, lookDir, (float)(lookDir[2] - -0.98000002) / 0.030000031, lookDir);
        }
        else
        {
            lookDir[0] = point1dir[0];
            lookDir[1] = point1dir[1];
            lookDir[2] = point1dir[2];
        }
    }
    Vec3NormalizeTo(lookDir, (float *)lookaxis);
    v7 = (*lookaxis)[0];
    ((*lookaxis)[3]) = -((*lookaxis)[1]);
    (*lookaxis)[4] = v7;
    (*lookaxis)[5] = 0.0f;
    if ( (*lookaxis)[3] == 0.0 && (*lookaxis)[4] == 0.0 )
        (*lookaxis)[3] = 1.0f;
    Vec3Normalize(&(*lookaxis)[3]);
    Vec3Cross((const float *)lookaxis, &(*lookaxis)[3], &(*lookaxis)[6]);
    if ( !Vec3IsNormalized(&(*lookaxis)[6]) )
    {
        v4 = Vec3Length(&(*lookaxis)[6]);
        v5 = va("(%g %g %g) len %g", (*lookaxis)[6], (*lookaxis)[7], (*lookaxis)[8], v4);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                        1481,
                        0,
                        "%s\n\t%s",
                        "Vec3IsNormalized( lookaxis[2] )",
                        v5) )
            __debugbreak();
    }
    return distance;
}

const float THIN_MAXS[3] = { 0.0, 0.0, 0.0 };

void __cdecl CG_KillcamCameraTrace(
                const float *originalBombOrigin,
                const float *bombAngles,
                const float *desiredCamPos,
                int passEntIndex,
                KillCamEntityRestState killCamEntityRestState,
                float *campos)
{
    float v6; // [esp+8h] [ebp-B8h]
    float cameraDir; // [esp+10h] [ebp-B0h]
    float cameraDir_4; // [esp+14h] [ebp-ACh]
    float cameraDir_4a; // [esp+14h] [ebp-ACh]
    float cameraDir_8; // [esp+18h] [ebp-A8h]
    float cameraDir_8a; // [esp+18h] [ebp-A8h]
    float isect[3]; // [esp+1Ch] [ebp-A4h] BYREF
    int traceIndex; // [esp+28h] [ebp-98h]
    const float *thin_maxs; // [esp+2Ch] [ebp-94h]
    float bombOrigin[3]; // [esp+30h] [ebp-90h] BYREF
    col_context_t context; // [esp+3Ch] [ebp-84h] BYREF
    const float *cam_maxs; // [esp+64h] [ebp-5Ch]
    int tryCount; // [esp+68h] [ebp-58h]
    trace_t trace; // [esp+6Ch] [ebp-54h] BYREF
    const float *cam_mins; // [esp+A8h] [ebp-18h]
    bool thinTrace; // [esp+AEh] [ebp-12h]
    bool debugCampos; // [esp+AFh] [ebp-11h]
    float prevBombOrigin[3]; // [esp+B0h] [ebp-10h] BYREF
    const float *thin_mins; // [esp+BCh] [ebp-4h]

    cam_mins = CAM_MINS;
    cam_maxs = CAM_MAXS;
    thin_mins = THIN_MAXS;
    thin_maxs = THIN_MAXS;
    thinTrace = 0;
    debugCampos = Dvar_GetBool("cg_killcamdebug");
    bombOrigin[0] = *originalBombOrigin;
    bombOrigin[1] = originalBombOrigin[1];
    bombOrigin[2] = originalBombOrigin[2];
    *campos = *desiredCamPos;
    campos[1] = desiredCamPos[1];
    campos[2] = desiredCamPos[2];
    //col_context_t::col_context_t(&context);
    CG_TraceCapsule(&trace, bombOrigin, cam_mins, cam_maxs, campos, passEntIndex, 2065, &context);
    for ( tryCount = 0; tryCount < 3; ++tryCount )
    {
        if ( !trace.startsolid )
            break;
        if ( debugCampos )
        {
            prevBombOrigin[0] = bombOrigin[0];
            prevBombOrigin[1] = bombOrigin[1];
            prevBombOrigin[2] = bombOrigin[2];
            DebugTraceLine(bombOrigin, campos, trace.fraction, colorDkGrey);
        }
        CG_TraceCapsule(&trace, bombOrigin, thin_mins, thin_maxs, campos, passEntIndex, 2065, &context);
        if ( trace.fraction == 1.0 )
        {
            bombOrigin[2] = (float)(5.0 + 1.0) + bombOrigin[2];
        }
        else
        {
            bombOrigin[0] = (float)((float)(5.0 + 1.0) * trace.normal.vec.v[0]) + bombOrigin[0];
            bombOrigin[1] = (float)((float)(5.0 + 1.0) * trace.normal.vec.v[1]) + bombOrigin[1];
            bombOrigin[2] = (float)((float)(5.0 + 1.0) * trace.normal.vec.v[2]) + bombOrigin[2];
        }
        CG_TraceCapsule(&trace, bombOrigin, cam_mins, cam_maxs, campos, passEntIndex, 2065, &context);
        if ( debugCampos )
            DebugTraceLine(prevBombOrigin, bombOrigin, 1.0, colorLtCyan);
    }
    if ( debugCampos )
        DebugTraceLine(bombOrigin, campos, trace.fraction, colorWhite);
    if ( trace.startsolid )
    {
        *campos = bombOrigin[0];
        campos[1] = bombOrigin[1];
        campos[2] = bombOrigin[2];
    }
    else if ( trace.fraction < 1.0 )
    {
        Vec3Lerp(bombOrigin, campos, trace.fraction, isect);
        for ( traceIndex = 0; traceIndex < 3; ++traceIndex )
        {
            cameraDir = *campos - isect[0];
            cameraDir_4 = campos[1] - isect[1];
            cameraDir_8 = campos[2] - isect[2];
            v6 = (-((float)((float)(cameraDir * trace.normal.vec.v[0]) + (float)(cameraDir_4 * trace.normal.vec.v[1])) + (float)(cameraDir_8 * trace.normal.vec.v[2]))) + 0.1;
            cameraDir_4a = (float)(v6 * trace.normal.vec.v[1]) + cameraDir_4;
            cameraDir_8a = (float)(v6 * trace.normal.vec.v[2]) + cameraDir_8;
            *campos = isect[0] + (float)((float)(v6 * trace.normal.vec.v[0]) + cameraDir);
            campos[1] = isect[1] + cameraDir_4a;
            campos[2] = isect[2] + cameraDir_8a;
            CG_TraceCapsule(&trace, isect, cam_mins, cam_maxs, campos, passEntIndex, 2065, &context);
            if ( thinTrace )
                RewindTrace(&trace, isect, campos, 5.0);
            if ( debugCampos )
                DebugTraceLine(isect, campos, trace.fraction, colorMdGrey);
            if ( trace.fraction == 1.0 )
                break;
            Vec3Lerp(isect, campos, trace.fraction, isect);
        }
        Vec3Lerp(isect, campos, trace.fraction, campos);
    }
}

void __cdecl RewindTrace(trace_t *trace, const float *start, const float *end, float amount)
{
    float diff; // [esp+0h] [ebp-10h]
    float diff_4; // [esp+4h] [ebp-Ch]
    float diff_8; // [esp+8h] [ebp-8h]
    float dot; // [esp+Ch] [ebp-4h]

    if ( trace->fraction != 1.0 )
    {
        diff = *end - *start;
        diff_4 = end[1] - start[1];
        diff_8 = end[2] - start[2];
        dot = (float)((float)(diff * trace->normal.vec.v[0]) + (float)(diff_4 * trace->normal.vec.v[1]))
                + (float)(diff_8 * trace->normal.vec.v[2]);
        if ( dot < 0.0 )
        {
            trace->fraction = (float)((float)(dot * amount)
                                                            / (float)((float)((float)(diff * diff) + (float)(diff_4 * diff_4))
                                                                            + (float)(diff_8 * diff_8)))
                                            + trace->fraction;
            if ( trace->fraction < 0.0 )
                trace->fraction = 0.0f;
        }
    }
}

void __cdecl DebugTraceLine(float *start, float *end, float frac, const float *color)
{
    float isect[3]; // [esp+8h] [ebp-Ch] BYREF

    if ( frac >= 1.0 )
    {
        CG_DebugLine(start, end, color, 0, 3000);
    }
    else
    {
        Vec3Lerp(start, end, frac, isect);
        CG_DebugLine(start, isect, color, 0, 3000);
    }
}

bool __cdecl CG_ShouldRenderThirdPerson(CameraMode camMode)
{
    switch ( camMode )
    {
        case CAM_VEHICLE:
            return 0;
        case CAM_VEHICLE_THIRDPERSON:
            return 0;
        case CAM_VEHICLE_GUNNER:
            return 0;
        case CAM_EXTRACAM:
            return 0;
        case CAM_MISSILE:
            return 0;
    }
    return camMode != CAM_RADIANT;
}

void __cdecl CG_UpdateThirdPerson(int localClientNum, bool forExtraCam)
{
    centity_s *cent; // [esp+Ch] [ebp-8h]
    cg_s *cgameGlob; // [esp+10h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    cgameGlob->renderingThirdPerson = (thirdPersonType)(cg_thirdPerson->current.integer || cgameGlob->nextSnap->ps.pm_type >= 9);
    if ( cgameGlob->nextSnap->ps.pm_type == 4 && (cgameGlob->nextSnap->ps.otherFlags & 2) == 0 )
        cgameGlob->renderingThirdPerson = TP_OFF;
    if ( (cgameGlob->nextSnap->ps.eFlags & 0x10000) != 0 )
        cgameGlob->renderingThirdPerson = TP_FOR_MODEL;
    if ( (cgameGlob->nextSnap->ps.eFlags & 0x4000) != 0 && (cgameGlob->nextSnap->ps.otherFlags & 2) == 0 )
    {
        cent = CG_GetEntity(localClientNum, cgameGlob->nextSnap->ps.viewlocked_entNum);
        if ( ((*((unsigned int *)cent + 201) >> 1) & 1) != 0 )
        {
            if ( CG_GetVehicleInfo(cent->nextState.vehicleState.vehicleInfoIndex)->remoteControl )
                cgameGlob->renderingThirdPerson = TP_FOR_MODEL;
        }
    }
    if ( CG_KillCamEntityEnabled(localClientNum) )
        cgameGlob->renderingThirdPerson = TP_FOR_MODEL;
    if ( (cgameGlob->nextSnap->ps.eFlags2 & 0x10000000) != 0 && (cgameGlob->nextSnap->ps.otherFlags & 2) == 0 )
        cgameGlob->renderingThirdPerson = TP_FOR_MODEL;
    if ( (unsigned __int8)CG_ScrCamActive(localClientNum) )
        cgameGlob->renderingThirdPerson = TP_FOR_MODEL;
    if ( Demo_IsThirdPersonCamera() || Demo_IsMovieCamera() )
        cgameGlob->renderingThirdPerson = TP_FOR_MODEL;
    if ( cgameGlob->cameraMode == 1 || cgameGlob->cameraMode == 2 )
        cgameGlob->renderingThirdPerson = TP_FOR_MODEL;
    if ( cgameGlob->radiantCamInUse && cgameGlob->cameraData.lastCamMode == CAM_RADIANT )
        cgameGlob->renderingThirdPerson = TP_FOR_MODEL;
    if ( forExtraCam )
        cgameGlob->renderingThirdPerson = TP_FOR_MODEL;
    if ( !r_enablePlayerShadow->current.enabled
        || cgameGlob->renderingThirdPerson
        || (cgameGlob->nextSnap->ps.eFlags & 0x4000) != 0 )
    {
        R_SetEnablePlayerShadowFlag(0);
    }
    else
    {
        cgameGlob->renderingThirdPerson = TP_FOR_SHADOW;
        R_SetEnablePlayerShadowFlag(1);
    }
}

const ClientViewParams *__cdecl CG_GetLocalClientViewParams(int localClientNum)
{
    int activeClientCountArrayIndex; // [esp+0h] [ebp-8h]
    int activeClientIndex; // [esp+4h] [ebp-4h]

    activeClientIndex = CL_LocalActiveIndexFromClientNum(localClientNum);
    activeClientCountArrayIndex = CL_LocalClient_GetActiveCount();
    return CG_GetLocalClientViewParams(localClientNum, activeClientIndex, activeClientCountArrayIndex);
}

const ClientViewParams *__cdecl CG_GetLocalClientViewParams(
                int localClientNum,
                int activeClientIndex,
                int activeClientCountArrayIndex)
{
    if ( activeClientIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                    3373,
                    0,
                    "activeClientIndex doesn't index ARRAY_COUNT( clientViewParamsArray[current_display_mode][0] )\n"
                    "\t%i not in [0, %i)",
                    activeClientIndex,
                    1) )
    {
        __debugbreak();
    }
    if ( activeClientCountArrayIndex != 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                    3374,
                    0,
                    "activeClientCountArrayIndex-1 doesn't index ARRAY_COUNT( clientViewParamsArray[current_display_mode] )\n"
                    "\t%i not in [0, %i)",
                    activeClientCountArrayIndex - 1,
                    1) )
    {
        __debugbreak();
    }
    if ( activeClientIndex > activeClientCountArrayIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                    3375,
                    0,
                    "activeClientIndex <= activeClientCountArrayIndex\n\t%i, %i",
                    activeClientIndex,
                    activeClientCountArrayIndex) )
    {
        __debugbreak();
    }
    return clientViewParamsArray[activeClientCountArrayIndex - 1][activeClientIndex];
}

void __cdecl GetCeilingHeight(cg_s *cgameGlob)
{
    col_context_t context; // [esp+Ch] [ebp-74h] BYREF
    trace_t trace; // [esp+34h] [ebp-4Ch] BYREF
    float endPos[3]; // [esp+70h] [ebp-10h] BYREF
    float TEST_HEIGHT; // [esp+7Ch] [ebp-4h]

    if ( ++cgameGlob->heightToCeilingTS >= 50 )
    {
        TEST_HEIGHT = 1024.0f;
        endPos[0] = cgameGlob->predictedPlayerState.origin[0];
        endPos[1] = cgameGlob->predictedPlayerState.origin[1];
        endPos[2] = cgameGlob->predictedPlayerState.origin[2];
        endPos[2] = endPos[2] + 1024.0;
        //col_context_t::col_context_t(&context);
        CG_TraceCapsule(&trace, cgameGlob->predictedPlayerState.origin, playerMins, playerMaxs, endPos, 1023, 1, &context);
        if ( trace.fraction < 1.0 )
        {
            Vec3Lerp(cgameGlob->predictedPlayerState.origin, endPos, trace.fraction, endPos);
            cgameGlob->heightToCeiling = endPos[2] - cgameGlob->predictedPlayerState.origin[2];
        }
        else
        {
            cgameGlob->heightToCeiling = FLT_MAX;
        }
    }
}

void __cdecl CG_UpdateViewOffset(int localClientNum)
{
    cg_s *cgameGlob; // [esp+8h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( CG_KillCamEntityEnabled(localClientNum) )
    {
        CG_UpdateKillCamEntityViewOffset(localClientNum);
    }
    else if ( CG_VehEntityUsingRemoteControlVehicle(localClientNum, cgameGlob->snap->ps.clientNum) )
    {
        CG_UpdateRemoteControlEntityViewOffset(localClientNum);
    }
    else
    {
        Vec3Lerp(
            cgameGlob->snap->ps.origin,
            cgameGlob->nextSnap->ps.origin,
            cgameGlob->frameInterpolation,
            cgameGlob->refdef.vieworg);
        cgameGlob->refdef.vieworg[2] = cgameGlob->refdef.vieworg[2] + cgameGlob->nextSnap->ps.viewHeightCurrent;
    }
    CL_ResetSkeletonCache(localClientNum);
}

void __cdecl CG_UpdateKillCamEntityViewOffset(int localClientNum)
{
    cg_s *cgameGlob; // [esp+8h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob->inKillCam
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                    1364,
                    0,
                    "%s",
                    "cgameGlob->inKillCam") )
    {
        __debugbreak();
    }
    if ( cgameGlob->killCamEntityType == KC_NO_ENTITY
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                    1365,
                    0,
                    "%s",
                    "cgameGlob->killCamEntityType != KC_NO_ENTITY") )
    {
        __debugbreak();
    }
    cgameGlob->refdef.vieworg[0] = cgameGlob->killCamLastEntityOrg[0];
    cgameGlob->refdef.vieworg[1] = cgameGlob->killCamLastEntityOrg[1];
    cgameGlob->refdef.vieworg[2] = cgameGlob->killCamLastEntityOrg[2];
}

void __cdecl CG_UpdateRemoteControlEntityViewOffset(int localClientNum)
{
    cg_s *cgameGlob; // [esp+18h] [ebp-8h]
    centity_s *vehicle; // [esp+1Ch] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    vehicle = CG_GetEntity(localClientNum, cgameGlob->nextSnap->ps.viewlocked_entNum);
    if ( ((*((unsigned int *)vehicle + 201) >> 1) & 1) != 0 )
    {
        Vec3Lerp(
            vehicle->currentState.pos.trBase,
            vehicle->nextState.lerp.pos.trBase,
            cgameGlob->frameInterpolation,
            cgameGlob->refdef.vieworg);
    }
    else
    {
        cgameGlob->refdef.vieworg[0] = cgameGlob->cameraData.lastViewOrg[0];
        cgameGlob->refdef.vieworg[1] = cgameGlob->cameraData.lastViewOrg[1];
        cgameGlob->refdef.vieworg[2] = cgameGlob->cameraData.lastViewOrg[2];
    }
}

void __cdecl CG_SetupFlameFX(int localClientNum, int enable)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( enable )
    {
        cgameGlob->refdef.flameFx.enabled = 1;
        cgameGlob->refdef.flameFx.duration = 0;
        cgameGlob->refdef.flameFx.distortionMagnitude = r_flameFX_magnitude->current.value;
        cgameGlob->refdef.flameFx.distortionScale[0] = r_flameFX_distortionScaleFactor->current.value;
        cgameGlob->refdef.flameFx.distortionScale[1] = r_flameFX_distortionScaleFactor->current.vector[1];
        cgameGlob->refdef.flameFx.frameRate = (float)r_flameFX_FPS->current.integer;
    }
    else if ( !cgameGlob->refdef.flameFx.duration )
    {
        cgameGlob->refdef.flameFx.startMSec = cgameGlob->time;
        cgameGlob->refdef.flameFx.currentTime = cgameGlob->time;
        cgameGlob->refdef.flameFx.duration = (int)(float)(r_flameFX_fadeDuration->current.value * 1000.0);
    }
}

void __cdecl CG_SetupElectrifiedFX(int localClientNum, int enable)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( enable )
    {
        cgameGlob->refdef.electrifiedFx.enabled = 1;
        cgameGlob->refdef.electrifiedFx.duration = 0;
        cgameGlob->refdef.electrifiedFx.distortionMagnitude = r_flameFX_magnitude->current.value;
        cgameGlob->refdef.electrifiedFx.distortionScale[0] = r_flameFX_distortionScaleFactor->current.value;
        cgameGlob->refdef.electrifiedFx.distortionScale[1] = r_flameFX_distortionScaleFactor->current.vector[1];
        cgameGlob->refdef.electrifiedFx.frameRate = (float)r_flameFX_FPS->current.integer;
    }
    else if ( !cgameGlob->refdef.electrifiedFx.duration )
    {
        cgameGlob->refdef.electrifiedFx.startMSec = cgameGlob->time;
        cgameGlob->refdef.electrifiedFx.currentTime = cgameGlob->time;
        cgameGlob->refdef.electrifiedFx.duration = (int)(float)(r_flameFX_fadeDuration->current.value * 1000.0);
    }
}

bool __cdecl CG_IsInfrared(int localClientNum)
{
    const char *Name; // eax
    const char *v2; // eax
    bool result; // al
    const WeaponVariantDef *weapVariantDef; // [esp+0h] [ebp-10h]
    int weapIndex; // [esp+4h] [ebp-Ch]
    cg_s *cgameGlob; // [esp+8h] [ebp-8h]
    float fZoom; // [esp+Ch] [ebp-4h] BYREF

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    result = 0;
    if ( cgameGlob->renderingThirdPerson == TP_OFF )
    {
        if ( CG_GetWeapReticleZoom(cgameGlob, &fZoom) )
        {
            if ( fZoom >= 0.99000001 )
            {
                weapIndex = BG_GetViewmodelWeaponIndex(&cgameGlob->predictedPlayerState);
                weapVariantDef = BG_GetWeaponVariantDef(weapIndex);
                if ( weapVariantDef->overlayMaterial )
                {
                    Name = Material_GetName(weapVariantDef->overlayMaterial);
                    v2 = strstr(Name, "_ir");
                    if ( v2 )
                        return 1;
                }
            }
        }
    }
    return result;
}

bool __cdecl CG_IsTvguided(int localClientNum, bool onlyADS)
{
    const char *Name; // eax
    const char *v4; // eax
    unsigned int weaponIndex; // [esp+0h] [ebp-18h]
    const WeaponDef *weapDef; // [esp+4h] [ebp-14h]
    const WeaponVariantDef *weapVariantDef; // [esp+8h] [ebp-10h]
    int weapIndex; // [esp+Ch] [ebp-Ch]
    cg_s *cgameGlob; // [esp+10h] [ebp-8h]
    float fZoom; // [esp+14h] [ebp-4h] BYREF

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( Demo_IsMovieCamera() )
        return 0;
    if ( cgameGlob->renderingThirdPerson == TP_OFF )
    {
        if ( CG_GetWeapReticleZoom(cgameGlob, &fZoom) )
        {
            weapIndex = BG_GetViewmodelWeaponIndex(&cgameGlob->predictedPlayerState);
            weapVariantDef = BG_GetWeaponVariantDef(weapIndex);
            if ( weapVariantDef->overlayMaterial )
            {
                Name = Material_GetName(weapVariantDef->overlayMaterial);
                v4 = strstr(Name, "tow_");
                if ( v4 )
                    return 1;
            }
        }
    }
    if ( onlyADS )
        return 0;
    weaponIndex = BG_GetViewmodelWeaponIndex(&cgameGlob->predictedPlayerState);
    weapDef = BG_GetWeaponDef(weaponIndex);
    return (cgameGlob->predictedPlayerState.eFlags2 & 0x10000000) != 0
            && weapDef->guidedMissileType == MISSILE_GUIDANCE_TVGUIDED
            && !CG_KillCamEntityEnabled(localClientNum)
            && cgameGlob->predictedPlayerState.viewlocked_entNum != 1023;
}

int CG_DrawActiveFrame(
    int localClientNum,
    int serverTime,
    DemoType demoType,
    CubemapShot cubemapShot,
    int cubemapSize,
    int renderScreen)
{
    shellshock_parms_t *ShellshockParms; // eax
    int shellShockDuration;
    int shellShockTime;
    float zfar; // [esp+4h] [ebp-924h]
    float *kickAngles; // [esp+50h] [ebp-8D8h]
    float *kickAVel; // [esp+54h] [ebp-8D4h]
    shellshock_parms_t parms; // [esp+58h] [ebp-8D0h] BYREF
    int bMoreTriggered; // [esp+268h] [ebp-6C0h]
    float radius; // [esp+26Ch] [ebp-6BCh]
    centity_s *playerCEnt; // [esp+270h] [ebp-6B8h]
    DObj *ClientDObj; // [esp+274h] [ebp-6B4h]
    int playerEntNum; // [esp+278h] [ebp-6B0h]
    int k; // [esp+88Ch] [ebp-9Ch]
    int j; // [esp+890h] [ebp-98h]
    int i; // [esp+894h] [ebp-94h]
    float xy[2]; // [esp+898h] [ebp-90h]
    DObj *obj; // [esp+8A0h] [ebp-88h]
    FxCameraUpdate fxcam; // [esp+8A4h] [ebp-84h] BYREF
    cg_s *cgameGlob; // [esp+8E0h] [ebp-48h]
    FxCmd fxUpdateCmd; // [esp+8E4h] [ebp-44h] BYREF
    centity_s *cent; // [esp+918h] [ebp-10h]
    int hadSavedScreen; // [esp+91Ch] [ebp-Ch]
    int delayedEnt; // [esp+920h] [ebp-8h]
    const cgs_t *cgs; // [esp+924h] [ebp-4h]
    int savedregs; // [esp+928h] [ebp+0h] BYREF

    if (!Sys_IsMainThread()
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
            4825,
            0,
            "%s",
            "Sys_IsMainThread()"))
    {
        __debugbreak();
    }
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    R_ClearScene(localClientNum);
    FX_BeginUpdate(localClientNum);
    R_Rope_ClearAll();
    if (cgameGlob->clearMarks)
    {
        FX_ClearMarks(localClientNum);
        cgameGlob->clearMarks = 0;
    }
    hadSavedScreen = cgameGlob->shellshock.hasSavedScreen;
    if (!Sys_IsMainThread()
        && !Assert_MyHandler(
            "c:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\../cgame/cg_colltree.h",
            86,
            0,
            "%s",
            "Sys_IsMainThread()"))
    {
        __debugbreak();
    }
    cgCollWorldLocalClientNum = localClientNum;
    cgameGlob->oldTime = cgameGlob->time;
    cgameGlob->time = serverTime;
    cgameGlob->bgs.time = cgameGlob->time;
    cgameGlob->physicsTime = serverTime;
    cgameGlob->demoType = demoType;
    cgameGlob->cubemapShot = cubemapShot;
    cgameGlob->cubemapSize = cubemapSize;
    cgameGlob->renderScreen = renderScreen;
    CG_ClearSaveScreenFx(localClientNum);
    cgameGlob->frametime = cgameGlob->time - cgameGlob->oldTime;
    if (cgameGlob->frametime < 0)
    {
        if (!Demo_IsPlaying())
            FX_RewindTo(localClientNum, cgameGlob->time);
        cgameGlob->frametime = 0;
        cgameGlob->oldTime = cgameGlob->time;
    }
    CG_AddLagometerFrameInfo(cgameGlob);
    cgameGlob->bgs.frametime = cgameGlob->frametime;
    CG_UpdateUIDeviceContexts(localClientNum, cgameGlob->time);

    iassert(bgs == 0);

    if (cgameGlob->isLoading)
        return 0;

    bgs = &cgameGlob->bgs;

    CG_SavePlayerState(localClientNum);
    CG_ProcessSnapshots(localClientNum);

    if (!cgameGlob->renderScreen)
    {
        iassert(bgs == &cgameGlob->bgs);

        bgs = NULL;
        return 0;
    }
    if (!cgameGlob->nextSnap || (cgameGlob->nextSnap->snapFlags & 2) != 0)
    {
        iassert(bgs == &cgameGlob->bgs);

        bgs = NULL;
        return 0;
    }

    if (CL_IsServerLoadingMap())
    {
        iassert(bgs == &cgameGlob->bgs);

        bgs = NULL;
        return 0;
    }

    CL_SetWaitingOnServerToLoadMap(localClientNum, 0);

    iassert(bgs == &cgameGlob->bgs);
    iassert(cgameGlob->snap);
    iassert(cgameGlob->nextSnap);

    CG_UpdateTimeScale(localClientNum);
    CG_UpdateIKTiming(localClientNum);
    CG_VisionSetsUpdate(localClientNum);
    CG_UpdateViewOffset(localClientNum);
    if (CG_ExtraCamIsActive(localClientNum))
    {
        CG_ExtraCam_GetViewOrigin(localClientNum, cgameGlob->refdef.extraCamPos);
        cgameGlob->refdef.extraCamPosValid = 1;
    }
    else
    {
        cgameGlob->refdef.extraCamPosValid = 0;
    }
    cgameGlob->refdef.time = cgameGlob->time;
    if ((unsigned __int8)CG_ScrCamActive(localClientNum))
        CG_ScrCamCalcView(localClientNum);
    if (cgameGlob->cameraMode == 1 || cgameGlob->cameraMode == 2)
        CG_MovieCamCalcView(localClientNum);
    if ((cgameGlob->predictedPlayerState.eFlags2 & 0x10000000) != 0
        && (cgameGlob->predictedPlayerState.eFlags & 0x4000) == 0)
    {
        CG_CalcMissileViewValues(localClientNum);
    }
    R_SetLodOrigin(&cgameGlob->refdef);
    FX_SetNextUpdateTime(localClientNum, cgameGlob->time);
    FX_FillUpdateCmd(localClientNum, &fxUpdateCmd);
    R_UpdateNonDependentEffects(&fxUpdateCmd);
    CG_UpdateWind(cgameGlob->time);
    cgs = CG_GetLocalClientStaticGlobals(localClientNum);
    if (cgameGlob->snap->ps.shellshockIndex)
    {
        shellShockTime = cgameGlob->snap->ps.shellshockTime;
        shellShockDuration = cgameGlob->snap->ps.shellshockDuration;
        ShellshockParms = BG_GetShellshockParms(cgameGlob->snap->ps.shellshockIndex);
    }
    else
    {
        shellShockTime = cgameGlob->testShock.time;
        shellShockDuration = cgameGlob->testShock.duration;
        ShellshockParms = BG_GetShellshockParms(0);
    }
    CG_StartShellShock(cgameGlob, ShellshockParms, shellShockTime, shellShockDuration);
    CG_UpdateShellShock(
        localClientNum,
        cgameGlob->shellshock.parms,
        cgameGlob->shellshock.startTime,
        cgameGlob->shellshock.duration);
    CG_UpdateThirdPerson(localClientNum, 0);
    CG_ClearHudGrenades();
    CG_UpdateEntInfo(localClientNum);
    CG_ProcessRadiantCmds();
    CG_UpdateBolt(localClientNum);
    CG_UpdateEnemyScramblerAlpha(localClientNum);
    CG_ProcessDestructibleEvents();
    //BLOPS_NULLSUB();
    delayedEnt = CG_AddPacketEntities(localClientNum);
    AimTarget_UpdateClientTargets(localClientNum);
    if (!cgameGlob->predictedPlayerState.locationSelectionInfo
        || (cgameGlob->predictedPlayerState.otherFlags & 0x1A) != 0
        || Demo_IsPlaying())
    {
        if (Key_IsCatcherActive(localClientNum, 8))
            Key_RemoveCatcher(localClientNum, -9);
        cgameGlob->selectedLocation[0] = 0.5f;
        cgameGlob->selectedLocation[1] = 0.5f;
        for (j = 0; j < 3; ++j)
        {
            cgameGlob->selectedLocations[j][0] = -1.0f;
            cgameGlob->selectedLocations[j][1] = -1.0f;
        }
        cgameGlob->lastTimeSelected = 0;
    }
    else if (Key_IsCatcherActive(localClientNum, 8) || Key_IsCatcherActive(localClientNum, -1))
    {
        if (!Key_IsCatcherActive(localClientNum, 8) && UI_GetActiveMenu(localClientNum) == UIMENU_SCOREBOARD)
            UI_CloseAllMenus(localClientNum);
    }
    else
    {
        xy[0] = 0.5f;
        xy[1] = 0.5f;
        Key_AddCatcher(localClientNum, 8);
        cgameGlob->selectedLocation[0] = 0.5f;
        cgameGlob->selectedLocation[1] = xy[1];
        for (i = 0; i < 3; ++i)
        {
            cgameGlob->selectedLocations[i][0] = -1.0f;
            cgameGlob->selectedLocations[i][1] = -1.0f;
        }
        cgameGlob->lastTimeSelected = 0;
    }
    if ((cgameGlob->nextSnap->ps.otherFlags & 4) == 0
        || cgameGlob->cameraMode == 1
        || Demo_IsThirdPersonCamera()
        || Demo_IsMovieCamera())
    {
        kickAVel = cgameGlob->kickAVel;
        cgameGlob->kickAVel[0] = 0.0f;
        kickAVel[1] = 0.0f;
        kickAVel[2] = 0.0f;
        kickAngles = cgameGlob->kickAngles;
        cgameGlob->kickAngles[0] = 0.0f;
        kickAngles[1] = 0.0f;
        kickAngles[2] = 0.0f;
    }
    else
    {
        CG_KickAngles(cgameGlob);
    }
    CL_SyncGpu();
    Rope_Update(localClientNum, cgameGlob->time);
    CG_ApplyWeaponTurnRateCap(localClientNum);
    CL_Input(localClientNum);
    {
        PROF_SCOPED("player state");
        CG_PredictPlayerState(localClientNum);
        CG_TransitionPlayerState(localClientNum, &cgameGlob->predictedPlayerState, &cgameGlob->oldTransPlayerState);
        if (cg_pmove[localClientNum].ps && cg_pmove[localClientNum].ps->waterlevel >= 3)
        {
            if (!cgameGlob->swimming)
            {
                cgameGlob->swimming = 1;
                CG_SendSwimNotify(localClientNum, cgameGlob->clientNum, 1);
            }
        }
        else if (cgameGlob->swimming)
        {
            cgameGlob->swimming = 0;
            CG_SendSwimNotify(localClientNum, cgameGlob->clientNum, 0);
        }
        if ((cgameGlob->nextSnap->ps.otherFlags & 6) != 0)
        {
            obj = Com_GetClientDObj(cgameGlob->nextSnap->ps.clientNum, localClientNum);
            if (obj)
            {
                if (!Sys_IsMainThread()
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                        5162,
                        0,
                        "%s",
                        "Sys_IsMainThread()"))
                {
                    __debugbreak();
                }
                XAnimClientNotifyList veryLargeNameOfNotifyListToMinimizeContact; // [esp+280h] [ebp-6A8h] BYREF

                //XAnimClientNotifyList::XAnimClientNotifyList(&veryLargeNameOfNotifyListToMinimizeContact);
                DObjSetClientNotifies(&veryLargeNameOfNotifyListToMinimizeContact);
                CG_DObjUpdateInfo(cgameGlob, obj, 2);
                if ((cgameGlob->nextSnap->ps.otherFlags & 2) != 0)
                    CG_ProcessFakeEntClientNoteTracks(localClientNum, cgameGlob->nextSnap->ps.clientNum);
                DObjClearClientNotifies();
                //XAnimClientNotifyList::~XAnimClientNotifyList(&veryLargeNameOfNotifyListToMinimizeContact);
            }
            CG_CalcEntityLerpPositions(localClientNum, &cgameGlob->predictedPlayerEntity);
            for (k = 0; k < 6; ++k)
                cgameGlob->predictedPlayerEntity.pose.player.tag[k] = -2;
            if (Demo_IsPlaying())
            {
                playerCEnt = &cgameGlob->predictedPlayerEntity;
                playerEntNum = cgameGlob->predictedPlayerEntity.nextState.number;
                ClientDObj = Com_GetClientDObj(playerEntNum, localClientNum);
                if (ClientDObj)
                {
                    radius = DObjGetRadius(ClientDObj) + 16.0;
                    R_LinkDObjEntity(localClientNum, playerEntNum, playerCEnt->pose.origin, radius);
                }
            }
            CG_ProcessEntity(localClientNum, &cgameGlob->predictedPlayerEntity);
        }
        if (delayedEnt != 1023)
        {
            cent = CG_GetEntity(localClientNum, delayedEnt);
            CG_ProcessEntity(localClientNum, cent);
            CG_CheckEntEvents(localClientNum, cent);
        }
    }

    {
        PROF_SCOPED("view anim");
        CG_UpdateViewWeaponAnim(localClientNum, 0);
    }

    gScrExecuteTime[1] = 0;

    if (CL_LocalClient_IsFirstActive(localClientNum) && cgameGlob->snap)
    {
        PROF_SCOPED("pending triggers");
        iassert(cg_level.currentTriggerListSize == 0);

        Com_Memcpy(cg_level.currentTriggerList, cg_level.pendingTriggerList, 12 * cg_level.pendingTriggerListSize);
        cg_level.currentTriggerListSize = cg_level.pendingTriggerListSize;
        cg_level.pendingTriggerListSize = 0;
        if (cg_level.currentTriggerListSize)
        {
            do
            {
                bMoreTriggered = CG_NotifyTriggers();
                Scr_RunCurrentThreads(SCRIPTINSTANCE_CLIENT);
            } while (bMoreTriggered);
            if (cg_level.currentTriggerListSize
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                    5234,
                    0,
                    "%s",
                    "cg_level.currentTriggerListSize == 0"))
            {
                __debugbreak();
            }
        }
        Scr_IncTime(SCRIPTINSTANCE_CLIENT);
    }

    {
        PROF_SCOPED("view values");
        UpdateKillCamEntityCache(localClientNum);
        CG_CalcViewValues(localClientNum);
        LerpKillCamView(localClientNum);
        if ((unsigned __int8)CG_ScrCamActive(localClientNum))
            CG_ScrCamCalcView(localClientNum);
        if (cgameGlob->cameraMode == 1 || cgameGlob->cameraMode == 2)
            CG_MovieCamCalcView(localClientNum);
        CG_DrawWaterTrail(localClientNum);
    }

    {
        PROF_SCOPED("player entity");
        if (r_lockPvs->current.enabled)
        {
            R_PvsLock_GetViewOrigin(fxcam.vieworg);
            R_PvsLock_GetViewAxis(fxcam.viewaxis);
            fxcam.tanHalfFovX = cgameGlob->refdef.tanHalfFovX;
            fxcam.tanHalfFovY = cgameGlob->refdef.tanHalfFovY;
            fxcam.zfar = R_GetFarPlaneDist();
        }
        else
        {
            zfar = R_GetFarPlaneDist();
            FX_GetCameraUpdateFromRefdefAndZFar(&fxcam, &cgameGlob->refdef, zfar);
        }
        FX_SetNextUpdateCamera(localClientNum, &fxcam);
        R_UpdateSpotLightEffect(&fxUpdateCmd);
        SND_SetListener(
            localClientNum,
            cgameGlob->nextSnap->ps.clientNum,
            cgameGlob->bgs.clientinfo[cgameGlob->clientNum].team,
            cgameGlob->refdef.vieworg,
            cgameGlob->refdef.viewaxis);
        CG_AddViewWeapon(localClientNum);
        CG_UpdateTestFX(localClientNum);
        CG_UpdateFire(localClientNum);
        CG_Flame_Update_Source(localClientNum);
        GetCeilingHeight(cgameGlob);
        if (CL_LocalClient_IsFirstActive(localClientNum))
            DumpAnims(localClientNum);
        FX_MarkUpdateClientTime(localClientNum, cgameGlob->frametime);
    }

    {
        PROF_SCOPED("remaining fx");
        R_UpdateRemainingEffects(&fxUpdateCmd);
        Rope_Render(localClientNum);
    }
    {
        PROF_SCOPED("aim assist");
        AimAssist_UpdateScreenTargets(
            localClientNum,
            cgameGlob->refdef.vieworg,
            cgameGlob->refdefViewAngles,
            cgameGlob->refdef.tanHalfFovX,
            cgameGlob->refdef.tanHalfFovY);
    }

    //BLOPS_NULLSUB();
    CG_UpdateSceneDepthOfField(localClientNum);
    CG_UpdateWaterSheetingFX(cgameGlob);
    CG_UpdateFlameFX(cgameGlob);
    CG_UpdatePoisonFX(cgameGlob);
    CG_UpdateElectrifiedFX(cgameGlob);
    CG_UpdateGenericFilter(cgameGlob);

    {
        PROF_SCOPED("draw 2D");
        R_AddCmdProjectionSet2D();
        DrawShellshockBlend(localClientNum);
        if (CG_IsInfrared(localClientNum))
        {
            if (hadSavedScreen)
                cgameGlob->shellshock.hasSavedScreen = 1;
            parms.screenBlend.blurredEffectTime = (int)cg_infraredBlurTime->current.value;
            parms.screenBlend.blurredFadeTime = 0;
            CG_DrawShellShockSavedScreenBlendBlurred(localClientNum, &parms, cgameGlob->time, 100000);
        }
        CG_CompassIncreaseRadarTime(localClientNum);
        CG_Draw2D(localClientNum);
    }
    
    CG_ValidateWeaponSelect(cgameGlob);
    CG_DrawActive(localClientNum);
    CG_CheckBattleChatter();
    iassert(bgs == &cgameGlob->bgs);
    bgs = NULL;
    return 1;
}

void __cdecl CG_UpdateTestFX(int localClientNum)
{
    if ( s_testEffect[localClientNum].respawnTime >= 1
        && CG_GetLocalClientGlobals(localClientNum)->time > s_testEffect[localClientNum].respawnTime
                                                                                                            + s_testEffect[localClientNum].time )
    {
        CG_PlayTestFx(localClientNum);
    }
}

void __cdecl CG_KickAngles(cg_s *cgameGlob)
{
    float v1; // [esp+0h] [ebp-40h]
    float v2; // [esp+4h] [ebp-3Ch]
    int v3; // [esp+8h] [ebp-38h]
    const WeaponVariantDef *weapVariantDefDW; // [esp+10h] [ebp-30h]
    float kickChange; // [esp+14h] [ebp-2Ch]
    int t; // [esp+18h] [ebp-28h]
    const WeaponVariantDef *weapVariantDef; // [esp+20h] [ebp-20h]
    float idealCenterSpeed; // [esp+24h] [ebp-1Ch]
    int weapIndex; // [esp+28h] [ebp-18h]
    int i; // [esp+2Ch] [ebp-14h]
    const WeaponDef *weapDef; // [esp+30h] [ebp-10h]
    float ft; // [esp+3Ch] [ebp-4h]

    weapIndex = BG_GetViewmodelWeaponIndex(&cgameGlob->predictedPlayerState);
    weapVariantDef = BG_GetWeaponVariantDef(weapIndex);
    weapDef = BG_GetWeaponDef(weapIndex);
    for ( t = cgameGlob->frametime; t > 0; t -= 5 )
    {
        if ( t <= 5 )
            v3 = t;
        else
            v3 = 5;
        ft = (float)v3 * 0.001;
        for ( i = 0; i < 3; ++i )
        {
            if ( cgameGlob->kickAVel[i] != 0.0 || cgameGlob->kickAngles[i] != 0.0 )
            {
                if ( cgameGlob->kickAngles[i] != 0.0 )
                {
                    v2 = cgameGlob->kickAngles[i] <= 0.0 ? 1.0f : -1.0f;
                    if ( v2 != 0.0 )
                    {
                        if ( weapIndex )
                        {
                            if ( cgameGlob->predictedPlayerState.fWeaponPosFrac <= 0.5 )
                            {
                                if ( weapDef->bDualWield )
                                {
                                    weapVariantDefDW = BG_GetWeaponVariantDef(weapDef->dualWieldWeaponIndex);
                                    if ( (cgameGlob->predictedPlayerState.weaponstate == 6
                                         || cgameGlob->predictedPlayerState.weaponstate == 31)
                                        && (cgameGlob->predictedPlayerState.weaponstateLeft == 6
                                         || cgameGlob->predictedPlayerState.weaponstateLeft == 31) )
                                    {
                                        idealCenterSpeed = (float)(weapVariantDef->fHipViewKickCenterSpeed
                                                                                         + weapVariantDefDW->fHipViewKickCenterSpeed)
                                                                         * v2;
                                    }
                                    else if ( cgameGlob->predictedPlayerState.weaponstateLeft == 6
                                                 || cgameGlob->predictedPlayerState.weaponstateLeft == 31 )
                                    {
                                        idealCenterSpeed = v2 * weapVariantDefDW->fHipViewKickCenterSpeed;
                                    }
                                    else
                                    {
                                        idealCenterSpeed = v2 * weapVariantDef->fHipViewKickCenterSpeed;
                                    }
                                }
                                else
                                {
                                    idealCenterSpeed = v2 * weapVariantDef->fHipViewKickCenterSpeed;
                                }
                            }
                            else
                            {
                                idealCenterSpeed = v2 * weapVariantDef->fAdsViewKickCenterSpeed;
                            }
                        }
                        else
                        {
                            idealCenterSpeed = v2 * 2400.0;
                        }
                        cgameGlob->kickAVel[i] = (float)(idealCenterSpeed * ft) + cgameGlob->kickAVel[i];
                    }
                }
                kickChange = cgameGlob->kickAVel[i] * ft;
                if ( (float)(cgameGlob->kickAngles[i] * kickChange) < 0.0 )
                    kickChange = kickChange * 0.059999999;
                if ( (float)((float)(cgameGlob->kickAngles[i] + kickChange) * cgameGlob->kickAngles[i]) < 0.0 )
                {
                    cgameGlob->kickAngles[i] = 0.0f;
                    cgameGlob->kickAVel[i] = 0.0f;
                }
                else
                {
                    cgameGlob->kickAngles[i] = cgameGlob->kickAngles[i] + kickChange;
                    if ( cgameGlob->kickAngles[i] == 0.0 )
                    {
                        cgameGlob->kickAVel[i] = 0.0f;
                    }
                    else if ( fabs(cgameGlob->kickAngles[i]) > 10.0 )
                    {
                        if ( cgameGlob->kickAngles[i] <= 0.0 )
                            v1 = -10.0f;
                        else
                            v1 = 10.0f;
                        cgameGlob->kickAngles[i] = v1;
                        cgameGlob->kickAVel[i] = 0.0f;
                    }
                }
            }
        }
    }
}

void __cdecl UpdateKillCamEntityCache(int localClientNum)
{
    if ( CG_GetLocalClientGlobals(localClientNum)->inKillCam )
    {
        CG_CacheKillCamEntityPose(localClientNum);
        CG_CacheKillCamLookAtEntityOrigin(localClientNum);
    }
}

void __cdecl CG_CacheKillCamEntityPose(int localClientNum)
{
    cg_s *cgameGlob; // [esp+18h] [ebp-8h]
    centity_s *cent; // [esp+1Ch] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp", 2524, 0, "%s", "cgameGlob") )
    {
        __debugbreak();
    }
    if ( !cgameGlob->inKillCam
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                    2525,
                    0,
                    "%s",
                    "cgameGlob->inKillCam") )
    {
        __debugbreak();
    }
    if ( cgameGlob->killCamLastEntityNum != 1023 )
    {
        if ( cgameGlob->killCamEntityType == KC_NO_ENTITY
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                        2531,
                        0,
                        "%s",
                        "cgameGlob->killCamEntityType != KC_NO_ENTITY") )
        {
            __debugbreak();
        }
        cent = CG_GetEntity(localClientNum, cgameGlob->killCamLastEntityNum);
        if ( ((*((unsigned int *)cent + 201) >> 1) & 1) == 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                        2534,
                        0,
                        "%s",
                        "cent->nextValid") )
        {
            __debugbreak();
        }
        cgameGlob->killCamLastEntityOrg[0] = cent->pose.origin[0];
        cgameGlob->killCamLastEntityOrg[1] = cent->pose.origin[1];
        cgameGlob->killCamLastEntityOrg[2] = cent->pose.origin[2];
        cgameGlob->killCamLastEntityAngles[0] = cent->pose.angles[0];
        cgameGlob->killCamLastEntityAngles[1] = cent->pose.angles[1];
        cgameGlob->killCamLastEntityAngles[2] = cent->pose.angles[2];
    }
}

void __cdecl CG_CacheKillCamLookAtEntityOrigin(int localClientNum)
{
    float *origin; // [esp+8h] [ebp-28h]
    DObj *obj; // [esp+14h] [ebp-1Ch]
    float heightOffset; // [esp+18h] [ebp-18h]
    cg_s *cgameGlob; // [esp+1Ch] [ebp-14h]
    float vPos[3]; // [esp+20h] [ebp-10h] BYREF
    centity_s *centTarget; // [esp+2Ch] [ebp-4h]

    heightOffset = 50.0f;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp", 2551, 0, "%s", "cgameGlob") )
    {
        __debugbreak();
    }
    if ( !cgameGlob->inKillCam
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                    2552,
                    0,
                    "%s",
                    "cgameGlob->inKillCam") )
    {
        __debugbreak();
    }
    if ( cgameGlob->killCamLookAtEntity != 1023 )
    {
        centTarget = CG_GetEntity(localClientNum, cgameGlob->killCamLookAtEntity);
        if ( ((*((unsigned int *)centTarget + 201) >> 1) & 1) != 0 )
        {
            if ( centTarget->pose.eType != 1
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                            2562,
                            0,
                            "%s",
                            "centTarget->pose.eType == ET_PLAYER") )
            {
                __debugbreak();
            }
            origin = centTarget->pose.origin;
            cgameGlob->killCamLookAt[0] = centTarget->pose.origin[0];
            cgameGlob->killCamLookAt[1] = origin[1];
            cgameGlob->killCamLookAt[2] = origin[2];
            obj = Com_GetClientDObj(cgameGlob->killCamLookAtEntity, localClientNum);
            if ( obj )
            {
                if ( CG_DObjGetWorldTagPos(&centTarget->pose, obj, scr_const.j_head, vPos) )
                    heightOffset = fabs(vPos[2] - centTarget->pose.origin[2])
                                             * 0.5;
            }
            cgameGlob->killCamLookAt[2] = cgameGlob->killCamLookAt[2] + heightOffset;
        }
    }
}

void __cdecl CG_UpdateEntInfo(int localClientNum)
{
    centity_s *cent; // [esp+624h] [ebp-62Ch]
    fake_centity_s *fakeEnt; // [esp+628h] [ebp-628h]
    DObj *obj; // [esp+C40h] [ebp-10h]
    cg_s *cgameGlob; // [esp+C44h] [ebp-Ch]
    int num; // [esp+C48h] [ebp-8h]
    int entnum; // [esp+C4Ch] [ebp-4h]

    PROF_SCOPED("update ent info");

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    for ( num = 0; num < cgameGlob->nextSnap->numEntities; ++num )
    {
        entnum = cgameGlob->nextSnap->entities[num].number;
        if ( !Demo_IsPlaying() || entnum != cgameGlob->clientNum )
        {
            obj = Com_GetClientDObj(entnum, localClientNum);
            if ( obj )
            {
                if ( !Sys_IsMainThread()
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                                3200,
                                0,
                                "%s",
                                "Sys_IsMainThread()") )
                {
                    __debugbreak();
                }
                XAnimClientNotifyList veryLargeNameOfNotifyListToMinimizeContact; // [esp+630h] [ebp-620h] BYREF

                //XAnimClientNotifyList::XAnimClientNotifyList(&veryLargeNameOfNotifyListToMinimizeContact);
                DObjSetClientNotifies(&veryLargeNameOfNotifyListToMinimizeContact);
                CG_DObjUpdateInfo(cgameGlob, obj, 2);
                CG_ProcessFakeEntClientNoteTracks(localClientNum, entnum);
                DObjClearClientNotifies();
                //XAnimClientNotifyList::~XAnimClientNotifyList(&veryLargeNameOfNotifyListToMinimizeContact);
            }
        }
    }
    for ( num = 0; num < 512; ++num )
    {
        fakeEnt = &cg_fakeEntitiesArray[512 * localClientNum + num];
        if ( cg_fakeEntitiesInuseArray[512 * localClientNum + fakeEnt - &cg_fakeEntitiesArray[512 * localClientNum]] )
        {
            _mm_prefetch((const char *)&cg_fakeEntitiesArray[512 * localClientNum + num].cent.nextState.eType, 1);
            cent = &fakeEnt->cent;
            obj = Com_GetClientDObj(fakeEnt->cent.nextState.number, localClientNum);
            if ( obj )
            {
                if ( !Sys_IsMainThread()
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                                3224,
                                0,
                                "%s",
                                "Sys_IsMainThread()") )
                {
                    __debugbreak();
                }
                XAnimClientNotifyList pNotifyList; // [esp+18h] [ebp-C38h] BYREF

                //XAnimClientNotifyList::XAnimClientNotifyList(&pNotifyList);
                DObjSetClientNotifies(&pNotifyList);
                CG_DObjUpdateInfo(cgameGlob, obj, 2);
                CG_ProcessFakeEntClientNoteTracks(localClientNum, cent->nextState.number);
                DObjClearClientNotifies();
                //XAnimClientNotifyList::~XAnimClientNotifyList(&pNotifyList);
            }
        }
    }
}

void __cdecl CG_UpdateIKTiming(int localClientNum)
{
    cg_s *cgameGlob; // [esp+8h] [ebp-4h]

    PROF_SCOPED("update ik timing");
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    IK_UpdateTimeAll(cgameGlob->time, localClientNum, 0);
}

void __cdecl DumpAnims(int localClientNum)
{
    const char *v1; // eax
    cg_s *cgameGlob; // [esp+0h] [ebp-8h]
    const DObj *obj; // [esp+4h] [ebp-4h]

    if ( (cg_dumpAnims->current.integer < -1 || cg_dumpAnims->current.integer >= 1536)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                    3490,
                    0,
                    "%s\n\t(cg_dumpAnims->current.integer) = %i",
                    "(cg_dumpAnims->current.integer >= -1 && cg_dumpAnims->current.integer < ((1<<10) + 512))",
                    cg_dumpAnims->current.integer) )
    {
        __debugbreak();
    }
    if ( cg_dumpAnims->current.integer >= 0 && !cg_paused->current.integer )
    {
        if ( cg_dumpAnims->current.integer )
        {
            cgameGlob = CG_GetLocalClientGlobals(localClientNum);
            obj = Com_GetClientDObj(cg_dumpAnims->current.unsignedInt, localClientNum);
            if ( obj )
            {
                v1 = va("client %i %i:\n", localClientNum, cgameGlob->time);
                DObjDisplayAnim(obj, v1);
            }
        }
        else
        {
            CG_DisplayViewmodelAnim(localClientNum);
        }
    }
}

void __cdecl DrawShellshockBlend(int localClientNum)
{
    cg_s *cgameGlob; // [esp+4h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cg_drawShellshock->current.enabled
        && !Demo_IsThirdPersonCamera()
        && !Demo_IsMovieCamera()
        && cgameGlob->cameraMode != 1
        && !CG_KillCamEntityEnabled(localClientNum) )
    {
        if ( cgameGlob->shellshock.parms->screenBlend.type )
        {
            if ( cgameGlob->shellshock.parms->screenBlend.type != SHELLSHOCK_VIEWTYPE_FLASHED
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                            3541,
                            0,
                            "%s",
                            "cgameGlob->shellshock.parms->screenBlend.type == SHELLSHOCK_VIEWTYPE_FLASHED") )
            {
                __debugbreak();
            }
            CG_DrawShellShockSavedScreenBlendFlashed(
                localClientNum,
                cgameGlob->shellshock.parms,
                cgameGlob->shellshock.startTime,
                cgameGlob->shellshock.duration);
        }
        else
        {
            CG_DrawShellShockSavedScreenBlendBlurred(
                localClientNum,
                cgameGlob->shellshock.parms,
                cgameGlob->shellshock.startTime,
                cgameGlob->shellshock.duration);
        }
    }
}

void __cdecl CG_UpdateTimeScale(int localClientNum)
{
    float lerp; // [esp+4h] [ebp-Ch]
    cg_s *cgameGlob; // [esp+Ch] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->timeScaleTimeStart && cgameGlob->time > cgameGlob->timeScaleTimeStart )
    {
        if ( cgameGlob->time <= cgameGlob->timeScaleTimeEnd )
        {
            lerp = (float)(cgameGlob->time - cgameGlob->timeScaleTimeStart)
                     / (float)(cgameGlob->timeScaleTimeEnd - cgameGlob->timeScaleTimeStart);
            Com_SetTimeScale((float)((float)(1.0 - lerp) * cgameGlob->timeScaleStart) + (float)(cgameGlob->timeScaleEnd * lerp));
        }
        else
        {
            Com_SetTimeScale(cgameGlob->timeScaleEnd);
            cgameGlob->timeScaleTimeStart = 0;
        }
    }
}

void __cdecl CG_UpdateSceneDepthOfField(int localClientNum)
{
    cg_s *cgameGlob; // [esp+8h] [ebp-8h]
    playerState_s *ps; // [esp+Ch] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    ps = &cgameGlob->snap->ps;
    if ( Demo_IsMovieCamera() || Demo_IsThirdPersonCamera() )
    {
        cgameGlob->refdef.dof.nearStart = 0.0f;
        cgameGlob->refdef.dof.nearEnd = 0.0f;
        cgameGlob->refdef.dof.farStart = 5000.0f;
        cgameGlob->refdef.dof.farEnd = 5000.0f;
        cgameGlob->refdef.dof.nearBlur = 6.0f;
        cgameGlob->refdef.dof.farBlur = 0.0f;
        if ( cgameGlob->refdef.dof.nearBlur < 4.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                        3822,
                        0,
                        "%s\n\t(cgameGlob->refdef.dof.nearBlur) = %g",
                        "(cgameGlob->refdef.dof.nearBlur >= 4.0f)",
                        cgameGlob->refdef.dof.nearBlur) )
        {
            __debugbreak();
        }
    }
    else if ( !CG_KillCamEntityEnabled(localClientNum) && cgameGlob->cameraMode != 1 )
    {
        if ( !CG_IsShowingZombieMap()
            && ps->dofNearStart == 0.0
            && ps->dofNearEnd == 0.0
            && ps->dofFarStart == 0.0
            && ps->dofFarEnd == 0.0 )
        {
            CG_UpdateAdsDof(localClientNum, &cgameGlob->refdef.dof);
            if ( cgameGlob->refdef.dof.nearBlur < 4.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                            3839,
                            0,
                            "%s\n\t(cgameGlob->refdef.dof.nearBlur) = %g",
                            "(cgameGlob->refdef.dof.nearBlur >= 4.0f)",
                            cgameGlob->refdef.dof.nearBlur) )
            {
                __debugbreak();
            }
        }
        else
        {
            cgameGlob->refdef.dof.nearStart = ps->dofNearStart;
            cgameGlob->refdef.dof.nearEnd = ps->dofNearEnd;
            cgameGlob->refdef.dof.farStart = ps->dofFarStart;
            cgameGlob->refdef.dof.farEnd = ps->dofFarEnd;
            cgameGlob->refdef.dof.nearBlur = ps->dofNearBlur;
            cgameGlob->refdef.dof.farBlur = ps->dofFarBlur;
        }
    }
}

void __cdecl CG_UpdateAdsDof(int localClientNum, GfxDepthOfField *dof)
{
    int ScreenTargetEntity; // eax
    float dt; // [esp+5Ch] [ebp-A4h]
    col_context_t context; // [esp+60h] [ebp-A0h] BYREF
    int targetCount; // [esp+88h] [ebp-78h]
    cg_s *cgameGlob; // [esp+8Ch] [ebp-74h]
    float nearEnd; // [esp+90h] [ebp-70h]
    centity_s *cent; // [esp+94h] [ebp-6Ch]
    float nearBlur; // [esp+98h] [ebp-68h]
    float farStart; // [esp+9Ch] [ebp-64h]
    int targetIndex; // [esp+A0h] [ebp-60h]
    trace_t trace; // [esp+A4h] [ebp-5Ch] BYREF
    float targetDist; // [esp+E0h] [ebp-20h]
    float traceDist; // [esp+E4h] [ebp-1Ch]
    float farBlur; // [esp+E8h] [ebp-18h]
    playerState_s *ps; // [esp+ECh] [ebp-14h]
    float traceEnd[3]; // [esp+F0h] [ebp-10h] BYREF
    float farEnd; // [esp+FCh] [ebp-4h]

    if ( !dof && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp", 3702, 0, "%s", "dof") )
        __debugbreak();
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    ps = &cgameGlob->predictedPlayerState;
    if ( ShouldDoDefaultAdsDof(localClientNum) )
    {
        dof->nearStart = 0.0f;
        dof->nearEnd = 0.0f;
        dof->farStart = 5000.0f;
        dof->farEnd = 5000.0f;
        dof->nearBlur = 6.0f;
        dof->farBlur = 0.0f;
    }
    else
    {
        nearEnd = 10000.0f;
        farStart = -1.0f;
        targetCount = AimAssist_GetScreenTargetCount(localClientNum);
        for ( targetIndex = 0; targetIndex < targetCount; ++targetIndex )
        {
            ScreenTargetEntity = AimAssist_GetScreenTargetEntity(localClientNum, targetIndex);
            cent = CG_GetEntity(localClientNum, ScreenTargetEntity);
            targetDist = Vec3Distance(cent->pose.origin, ps->origin);
            if ( nearEnd > (float)(targetDist - 30.0) )
                nearEnd = targetDist - 30.0;
            if ( (float)(targetDist + 30.0) > farStart )
                farStart = targetDist + 30.0;
        }
        if ( nearEnd <= farStart )
        {
            if ( nearEnd >= 50.0 )
            {
                if ( nearEnd > 512.0 )
                    nearEnd = 512.0f;
            }
            else
            {
                nearEnd = 50.0f;
            }
            if ( farStart <= 2500.0 )
            {
                if ( farStart < 1000.0 )
                    farStart = 1000.0f;
            }
            else
            {
                farStart = 2500.0f;
            }
        }
        else
        {
            nearEnd = 256.0f;
            farStart = 2500.0f;
        }
        traceEnd[0] = (float)(8192.0 * cgameGlob->refdef.viewaxis[0][0]) + cgameGlob->refdef.vieworg[0];
        traceEnd[1] = (float)(8192.0 * cgameGlob->refdef.viewaxis[0][1]) + cgameGlob->refdef.vieworg[1];
        traceEnd[2] = (float)(8192.0 * cgameGlob->refdef.viewaxis[0][2]) + cgameGlob->refdef.vieworg[2];
        //col_context_t::col_context_t(&context);
        CG_TraceCapsule(
            &trace,
            cgameGlob->refdef.vieworg,
            vec3_origin,
            vec3_origin,
            traceEnd,
            ps->clientNum,
            0x806C31,
            &context);
        Vec3Lerp(cgameGlob->refdef.vieworg, traceEnd, trace.fraction, traceEnd);
        traceDist = Vec3Distance(cgameGlob->refdef.vieworg, traceEnd);
        if ( nearEnd > traceDist )
            nearEnd = traceDist - 30.0;
        if ( nearEnd < 1.0 )
            nearEnd = 1.0f;
        if ( traceDist > farStart )
            farStart = traceDist;
        farEnd = farStart * 4.0;
        nearBlur = 6.0f;
        farBlur = 0.0f;
        dt = (float)cgameGlob->frametime * 0.001;
        if ( ps->fWeaponPosFrac != 1.0 && (ps->pm_type < 9 || (ps->eFlags2 & 0x10000000) != 0) )
        {
            dof->nearStart = (float)((float)(1.0 - ps->fWeaponPosFrac) * 0.0) + (float)(1.0 * ps->fWeaponPosFrac);
            dof->nearEnd = (float)((float)(1.0 - ps->fWeaponPosFrac) * 0.0) + (float)(nearEnd * ps->fWeaponPosFrac);
            dof->farStart = (float)((float)(1.0 - ps->fWeaponPosFrac) * 5000.0) + (float)(farStart * ps->fWeaponPosFrac);
            dof->farEnd = (float)((float)(1.0 - ps->fWeaponPosFrac) * 5000.0) + (float)(farEnd * ps->fWeaponPosFrac);
            dof->nearBlur = (float)((float)(1.0 - ps->fWeaponPosFrac) * 6.0) + (float)(nearBlur * ps->fWeaponPosFrac);
            dof->farBlur = (float)((float)(1.0 - ps->fWeaponPosFrac) * 0.0) + (float)(farBlur * ps->fWeaponPosFrac);
        }
        else
        {
            dof->nearStart = CG_UpdateAdsDofValue(dof->nearStart, 1.0, 50.0, dt);
            dof->nearEnd = CG_UpdateAdsDofValue(dof->nearEnd, nearEnd, 50.0, dt);
            dof->farStart = CG_UpdateAdsDofValue(dof->farStart, farStart, 400.0, dt);
            dof->farEnd = CG_UpdateAdsDofValue(dof->farEnd, farEnd, 400.0, dt);
            dof->nearBlur = CG_UpdateAdsDofValue(dof->nearBlur, nearBlur, 0.1, dt);
            dof->farBlur = CG_UpdateAdsDofValue(dof->farBlur, farBlur, 0.1, dt);
        }
    }
}

double __cdecl CG_UpdateAdsDofValue(float currentValue, float targetValue, float maxChange, float dt)
{
    float changeVal; // [esp+0h] [ebp-4h]
    float changeVala; // [esp+0h] [ebp-4h]
    float maxChangea; // [esp+14h] [ebp+10h]

    maxChangea = (float)(maxChange / 0.050000001) * dt;
    if ( currentValue <= targetValue )
    {
        if ( targetValue > currentValue )
        {
            changeVala = (float)(targetValue - currentValue) * 0.5;
            if ( changeVala <= maxChangea )
            {
                if ( changeVala < 1.0 )
                    changeVala = 1.0f;
            }
            else
            {
                changeVala = maxChangea;
            }
            if ( (float)(currentValue + changeVala) <= targetValue )
                return (float)(currentValue + changeVala);
            else
                return targetValue;
        }
    }
    else
    {
        changeVal = (float)(currentValue - targetValue) * 0.5;
        if ( changeVal <= maxChangea )
        {
            if ( changeVal < 1.0 )
                changeVal = 1.0f;
        }
        else
        {
            changeVal = maxChangea;
        }
        if ( targetValue <= (float)(currentValue - changeVal) )
            return (float)(currentValue - changeVal);
        else
            return targetValue;
    }
    return currentValue;
}

bool __cdecl ShouldDoDefaultAdsDof(int localClientNum)
{
    cg_s *LocalClientGlobals; // eax

    if ( Demo_IsMovieCamera() )
        return 1;
    if ( Demo_IsThirdPersonCamera() )
        return 1;
    LocalClientGlobals = CG_GetLocalClientGlobals(localClientNum);
    if ( LocalClientGlobals->renderingThirdPerson == TP_FOR_MODEL
        && (LocalClientGlobals->predictedPlayerState.otherFlags & 0x1A) != 0 )
    {
        return 1;
    }
    if ( LocalClientGlobals->predictedPlayerState.pm_type >= 9 )
        return (LocalClientGlobals->predictedPlayerState.eFlags2 & 0x10000000) != 0;
    if ( LocalClientGlobals->predictedPlayerState.fWeaponPosFrac == 0.0 )
        return 1;
    return (LocalClientGlobals->predictedPlayerState.eFlags & 0x4000) != 0
            || (LocalClientGlobals->predictedPlayerState.eFlags2 & 0x10000000) != 0;
}

void __cdecl CG_UpdatePoisonFX(cg_s *cgameGlob)
{
    float curAmountDeltaPerSec; // [esp+10h] [ebp-Ch]
    GfxPoison *fx; // [esp+14h] [ebp-8h]

    fx = &cgameGlob->refdef.poisonFx;
    if ( !cgameGlob->predictedPlayerState.poisoned && !cgameGlob->poisoned
        || (cgameGlob->predictedPlayerState.perks[0] & 0x1000000) != 0
        || (cgameGlob->predictedPlayerState.eFlags2 & 0x10000000) != 0
        || cgameGlob->inKillCam && cgameGlob->killCamEntityType )
    {
        fx->curAmountTarget = 0.0f;
    }
    else
    {
        fx->curAmountTarget = 1.0f;
    }
    if ( cgameGlob->cameraMode == 1 )
        fx->curAmountTarget = 0.0f;
    if ( Demo_IsThirdPersonCamera() || Demo_IsMovieCamera() )
    {
        fx->curAmountTarget = 0.0f;
        cgameGlob->refdef.poisonFx.curAmount = 0.0f;
    }
    if ( cgameGlob->refdef.poisonFx.curAmount != fx->curAmountTarget )
    {
        curAmountDeltaPerSec = 0.66666669 * (float)((float)cgameGlob->frametime / 1000.0);
        if ( cgameGlob->refdef.poisonFx.curAmountTarget <= cgameGlob->refdef.poisonFx.curAmount )
        {
            cgameGlob->refdef.poisonFx.curAmount = cgameGlob->refdef.poisonFx.curAmount - curAmountDeltaPerSec;
            if ( cgameGlob->refdef.poisonFx.curAmountTarget > cgameGlob->refdef.poisonFx.curAmount )
                cgameGlob->refdef.poisonFx.curAmount = cgameGlob->refdef.poisonFx.curAmountTarget;
        }
        else
        {
            cgameGlob->refdef.poisonFx.curAmount = cgameGlob->refdef.poisonFx.curAmount + curAmountDeltaPerSec;
            if ( cgameGlob->refdef.poisonFx.curAmount > cgameGlob->refdef.poisonFx.curAmountTarget )
                cgameGlob->refdef.poisonFx.curAmount = cgameGlob->refdef.poisonFx.curAmountTarget;
        }
    }
}

void __cdecl CG_UpdateGenericFilter(cg_s *cgameGlob)
{
    int iParam; // [esp+4h] [ebp-10h]
    int iPass; // [esp+8h] [ebp-Ch]
    int iFilter; // [esp+Ch] [ebp-8h]
    GfxGenericFilter *filter; // [esp+10h] [ebp-4h]

    filter = &cgameGlob->refdef.genericFilter;
    if ( !Demo_IsMovieCamera() )
    {
        for ( iFilter = 0; iFilter < 3; ++iFilter )
        {
            for ( iPass = 0; iPass < 16; ++iPass )
            {
                filter->passEnabled[iFilter][iPass] = cgameGlob->genericFilterPassEnabled[iFilter][iPass];
                if ( filter->passEnabled[iFilter][iPass] )
                {
                    filter->passMaterial[iFilter][iPass] = cgameGlob->genericFilterPassMaterial[iFilter][iPass];
                    filter->passTarget[iFilter][iPass] = cgameGlob->genericFilterPassTarget[iFilter][iPass];
                    filter->passSampler0[iFilter][iPass] = cgameGlob->genericFilterPassSampler0[iFilter][iPass];
                    filter->passSampler1[iFilter][iPass] = cgameGlob->genericFilterPassSampler1[iFilter][iPass];
                    filter->passQuads[iFilter][iPass] = cgameGlob->genericFilterPassQuads[iFilter][iPass];
                    filter->passFlareOcclusion[iFilter][iPass] = cgameGlob->genericFilterPassFlareOcclusion[iFilter][iPass];
                    for ( iParam = 0; iParam < 16; ++iParam )
                        filter->passParam[iFilter][iPass][iParam] = cgameGlob->genericFilterPassParam[iFilter][iPass][iParam];
                }
            }
        }
    }
}

void __cdecl CG_UpdateWaterSheetingFX(cg_s *cgameGlob)
{
    if ( cgameGlob->snap )
    {
        if ( cg_watersheeting->current.enabled && cgameGlob->predictedPlayerState.waterlevel >= 5 )
        {
            cgameGlob->refdef.waterSheetingFx.enabled = 1;
            cgameGlob->refdef.waterSheetingFx.duration = 0;
            cgameGlob->refdef.waterSheetingFx.blurRadius = r_waterSheetingFX_radius->current.value;
            cgameGlob->refdef.waterSheetingFx.distortionMagnitude = 0.01f;
            cgameGlob->refdef.waterSheetingFx.distortionScale[0] = 0.0f;
            cgameGlob->refdef.waterSheetingFx.distortionScale[1] = 0.0f;
            cgameGlob->refdef.waterSheetingFx.startMSec = cgameGlob->time;
        }
        else
        {
            cgameGlob->refdef.waterSheetingFx.blurRadius = r_waterSheetingFX_radius->current.value;
            cgameGlob->refdef.waterSheetingFx.distortionMagnitude = r_waterSheetingFX_magnitude->current.value;
            cgameGlob->refdef.waterSheetingFx.distortionScale[0] = r_waterSheetingFX_distortionScaleFactor->current.value;
            cgameGlob->refdef.waterSheetingFx.distortionScale[1] = r_waterSheetingFX_distortionScaleFactor->current.vector[1];
            cgameGlob->refdef.waterSheetingFx.duration = 3000;
        }
        if ( cgameGlob->refdef.waterSheetingFx.enabled && cgameGlob->refdef.waterSheetingFx.duration > 0 )
        {
            cgameGlob->refdef.waterSheetingFx.currentTime = cgameGlob->time;
            if ( cgameGlob->refdef.waterSheetingFx.currentTime - cgameGlob->refdef.waterSheetingFx.startMSec > cgameGlob->refdef.waterSheetingFx.duration )
                cgameGlob->refdef.waterSheetingFx.enabled = 0;
        }
    }
}

void __cdecl CG_UpdateFlameFX(cg_s *cgameGlob)
{
    if ( Demo_IsThirdPersonCamera() || Demo_IsMovieCamera() )
    {
        cgameGlob->refdef.flameFx.enabled = 0;
    }
    else if ( cgameGlob->cameraMode == 1 )
    {
        cgameGlob->refdef.flameFx.enabled = 0;
    }
    else if ( cgameGlob->refdef.flameFx.enabled && cgameGlob->refdef.flameFx.duration > 0 )
    {
        cgameGlob->refdef.flameFx.currentTime = cgameGlob->time;
        if ( cgameGlob->refdef.flameFx.currentTime - cgameGlob->refdef.flameFx.startMSec > cgameGlob->refdef.flameFx.duration )
            cgameGlob->refdef.flameFx.enabled = 0;
    }
}

void __cdecl CG_UpdateElectrifiedFX(cg_s *cgameGlob)
{
    if ( cgameGlob->cameraMode == 1 )
    {
        cgameGlob->refdef.electrifiedFx.enabled = 0;
    }
    else if ( Demo_IsThirdPersonCamera() || Demo_IsMovieCamera() )
    {
        cgameGlob->refdef.electrifiedFx.enabled = 0;
    }
    else if ( cgameGlob->refdef.electrifiedFx.enabled && cgameGlob->refdef.electrifiedFx.duration > 0 )
    {
        cgameGlob->refdef.electrifiedFx.currentTime = cgameGlob->time;
        if ( cgameGlob->refdef.electrifiedFx.currentTime - cgameGlob->refdef.electrifiedFx.startMSec > cgameGlob->refdef.electrifiedFx.duration )
            cgameGlob->refdef.electrifiedFx.enabled = 0;
    }
}

void __cdecl CG_ValidateWeaponSelect(cg_s *cgameGlob)
{
    unsigned int NumWeapons; // eax
    bool invalid; // [esp+Fh] [ebp-1h]

    invalid = 0;
    if ( cgameGlob->weaponSelect )
    {
        if ( cgameGlob->weaponSelect >= BG_GetNumWeapons() )
        {
            NumWeapons = BG_GetNumWeapons();
            Com_PrintWarning(
                17,
                "WARNING: Invalid weaponSelect setting %i (out of range 0 - %i)\n",
                cgameGlob->weaponSelect,
                NumWeapons - 1);
            invalid = 1;
        }
        if ( !invalid )
            invalid = !BG_PlayerHasWeapon(&cgameGlob->predictedPlayerState, cgameGlob->weaponSelect);
        if ( invalid )
            CG_PickValidWeaponSelect(cgameGlob);
    }
}

void __cdecl CG_PickValidWeaponSelect(cg_s *cgameGlob)
{
    int slot; // [esp+0h] [ebp-8h]
    unsigned int weapIdx; // [esp+4h] [ebp-4h]

    cgameGlob->weaponSelect = 0;
    for ( slot = 0; slot < 15; ++slot )
    {
        weapIdx = cgameGlob->predictedPlayerState.heldWeapons[slot].weapon;
        if ( weapIdx )
        {
            cgameGlob->weaponSelect = weapIdx;
            return;
        }
    }
}

int __cdecl CG_DrawExtraCamFrame(
                int localClientNum,
                int serverTime,
                DemoType demoType,
                CubemapShot cubemapShot,
                int cubemapSize,
                int renderScreen)
{
    float zfar; // [esp+0h] [ebp-470h]
    char *Name; // [esp+2Ch] [ebp-444h]
    int i; // [esp+48h] [ebp-428h]
    unsigned __int8 drawPlayer3rdPerson; // [esp+4Fh] [ebp-421h]
    FxCameraUpdate fxcam; // [esp+50h] [ebp-420h] BYREF
    cg_s *cgameGlob; // [esp+8Ch] [ebp-3E4h]
    FxCmd fxUpdateCmd; // [esp+90h] [ebp-3E0h] BYREF
    ExtraCamClientStateRestore clientRestore(localClientNum); // [esp+C8h] [ebp-3A8h] BYREF

    Name = va("CG_DrawExtraCamFrame c=%d", localClientNum);

    PROF_SCOPED("CG_DrawExtraCamFrame");
    ZoneTextF("c=%d", localClientNum);

    //ExtraCamClientStateRestore::ExtraCamClientStateRestore(&clientRestore, localClientNum);
    if ( !Sys_IsMainThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_view_mp.cpp",
                    5424,
                    0,
                    "%s",
                    "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    R_ClearScene_For_ExtraCam(localClientNum);
    if ( !Sys_IsMainThread()
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\../cgame/cg_colltree.h",
                    86,
                    0,
                    "%s",
                    "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    cgCollWorldLocalClientNum = localClientNum;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    cgameGlob->demoType = demoType;
    cgameGlob->cubemapShot = cubemapShot;
    cgameGlob->cubemapSize = cubemapSize;
    cgameGlob->renderScreen = renderScreen;

    iassert(bgs == 0);

    
    if ( cgameGlob->isLoading )
    {
        //ExtraCamClientStateRestore::~ExtraCamClientStateRestore(&clientRestore);
        return 0;
    }
    else
    {
        bgs = &cgameGlob->bgs;
        if ( cgameGlob->renderScreen )
        {
            if ( cgameGlob->nextSnap && (cgameGlob->nextSnap->snapFlags & 2) == 0 )
            {
                if ( CL_IsServerLoadingMap() )
                {
                    iassert(bgs == &cgameGlob->bgs);
                    bgs = NULL;
                    //ExtraCamClientStateRestore::~ExtraCamClientStateRestore(&clientRestore);
                    return 0;
                }
                else
                {
                    iassert(bgs == &cgameGlob->bgs);
                    iassert(cgameGlob->snap);
                    iassert(cgameGlob->nextSnap);
                    
                    FX_FillUpdateCmd(localClientNum, &fxUpdateCmd);
                    fxUpdateCmd.updateCameraType = 1;
                    drawPlayer3rdPerson = Flame_GetLocalClientSourceRange();
                    if ( drawPlayer3rdPerson )
                        CG_UpdateThirdPerson(localClientNum, 1);
                    CL_SyncGpu();
                    if ( drawPlayer3rdPerson && (cgameGlob->nextSnap->ps.otherFlags & 6) != 0 )
                    {
                        CG_CalcEntityLerpPositions(localClientNum, &cgameGlob->predictedPlayerEntity);
                        for ( i = 0; i < 6; ++i )
                            cgameGlob->predictedPlayerEntity.pose.player.tag[i] = -2;
                        CG_ProcessEntity(localClientNum, &cgameGlob->predictedPlayerEntity);
                    }
                    CG_CalcViewValues_ExtraCam(localClientNum);
                    zfar = R_GetFarPlaneDist();
                    FX_GetCameraUpdateFromRefdefAndZFar(&fxcam, &cgameGlob->refdef, zfar);
                    FX_SetExtraCamera(localClientNum, &fxcam);
                    FX_SetNextUpdateCamera(localClientNum, &fxcam);
                    R_DrawEffects(&fxUpdateCmd);
                    CG_DrawMissileCam(localClientNum);
                    iassert(bgs == &cgameGlob->bgs);
                    bgs = NULL;
                    //ExtraCamClientStateRestore::~ExtraCamClientStateRestore(&clientRestore);
                    return 1;
                }
            }
            else
            {
                iassert(bgs == &cgameGlob->bgs);
                bgs = NULL;
                //ExtraCamClientStateRestore::~ExtraCamClientStateRestore(&clientRestore);
                return 0;
            }
        }
        else
        {
            iassert(bgs == &cgameGlob->bgs);
            bgs = NULL;
            //ExtraCamClientStateRestore::~ExtraCamClientStateRestore(&clientRestore);
            return 0;
        }
    }
}

void __cdecl CG_CalcViewValues_ExtraCam(int localClientNum)
{
    CameraMode camMode; // [esp+20h] [ebp-Ch]
    cg_s *cgameGlob; // [esp+24h] [ebp-8h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    cgameGlob->refdef.sunVisibility = 1.0f;
    camMode = CG_UpdateExtraCamMode(localClientNum);
    CG_CalcFov_ExtraCam(localClientNum);
    CG_VisionSetApplyToRefdef(localClientNum, 1);
    if ( cgameGlob->cubemapShot )
    {
        CG_CalcFov_ExtraCam(localClientNum);
        CG_CalcCubemapViewValues(cgameGlob);
    }
    else
    {
        CG_CalcVrect_ExtraCam(localClientNum);
        if ( cgameGlob->predictedPlayerState.pm_type == 5 )
        {
            cgameGlob->refdef.vieworg[0] = cgameGlob->predictedPlayerState.origin[0];
            cgameGlob->refdef.vieworg[1] = cgameGlob->predictedPlayerState.origin[1];
            cgameGlob->refdef.vieworg[2] = cgameGlob->predictedPlayerState.origin[2];
            cgameGlob->refdefViewAngles[0] = cgameGlob->predictedPlayerState.viewangles[0];
            cgameGlob->refdefViewAngles[1] = cgameGlob->predictedPlayerState.viewangles[1];
            cgameGlob->refdefViewAngles[2] = cgameGlob->predictedPlayerState.viewangles[2];
            AnglesToAxis(cgameGlob->refdefViewAngles, cgameGlob->refdef.viewaxis);
        }
        else if ( CG_KillCamEntityEnabled(localClientNum) )
        {
            CG_UpdateKillCamEntity(cgameGlob->killCamEntityType, localClientNum);
            CG_VisionSetApplyToRefdef(localClientNum, 1);
        }
        else
        {
            cgameGlob->refdef.vieworg[0] = cgameGlob->predictedPlayerState.origin[0];
            cgameGlob->refdef.vieworg[1] = cgameGlob->predictedPlayerState.origin[1];
            cgameGlob->refdef.vieworg[2] = cgameGlob->predictedPlayerState.origin[2];
            cgameGlob->refdefViewAngles[0] = cgameGlob->predictedPlayerState.viewangles[0];
            cgameGlob->refdefViewAngles[1] = cgameGlob->predictedPlayerState.viewangles[1];
            cgameGlob->refdefViewAngles[2] = cgameGlob->predictedPlayerState.viewangles[2];
            if ( camMode == CAM_MISSILE )
            {
                CG_CalcMissileViewValues(localClientNum);
            }
            else if ( camMode == CAM_EXTRACAM )
            {
                CG_CalcExtraCamViewValues(localClientNum);
            }
            if ( camMode != CAM_EXTRACAM && camMode != CAM_MISSILE )
                cgameGlob->refdef.vieworg[2] = cgameGlob->refdef.vieworg[2] + cgameGlob->predictedPlayerState.viewHeightCurrent;
            CG_CalcFov_ExtraCam(localClientNum);
            if ( camMode == CAM_MISSILE )
                CG_CalcMissileAngleValues(localClientNum);
            AnglesToAxis(cgameGlob->refdefViewAngles, cgameGlob->refdef.viewaxis);
            CG_ExtraCamDebug_SetView(localClientNum);
        }
    }
}

void __cdecl CG_CalcVrect_ExtraCam(int localClientNum)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-8h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    CG_GetLocalClientStaticGlobals(localClientNum);
    cgameGlob->refdef.x = 0;
    cgameGlob->refdef.y = 0;
    cgameGlob->refdef.width = R_ExtraCam_RenderTargetWidth();
    cgameGlob->refdef.height = R_ExtraCam_RenderTargetHeight();
    cgameGlob->refdef.useScissorViewport = 0;
}

void __cdecl CG_ClearSaveScreenFx(int localClientNum)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    memset((unsigned __int8 *)&cgameGlob->refdef.saveScreenFx, 0, sizeof(cgameGlob->refdef.saveScreenFx));
}

void __cdecl CG_SaveScreen(int localClientNum)
{
    cg_s *cgameGlob; // [esp+4h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    cgameGlob->refdef.saveScreenFx.saveScreenParam.mode = 1;
    cgameGlob->refdef.saveScreenFx.saveScreenParam.screenTimerId = localClientNum;
}

void __cdecl CG_SaveScreenSection(float viewX, float viewY, float viewWidth, float viewHeight, int screenTimerId)
{
    cg_s *cgameGlob; // [esp+4h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(screenTimerId);
    cgameGlob->refdef.saveScreenFx.saveScreenParam.mode = 2;
    cgameGlob->refdef.saveScreenFx.saveScreenParam.s0 = viewX;
    cgameGlob->refdef.saveScreenFx.saveScreenParam.t0 = viewY;
    cgameGlob->refdef.saveScreenFx.saveScreenParam.ds = viewWidth;
    cgameGlob->refdef.saveScreenFx.saveScreenParam.dt = viewHeight;
    cgameGlob->refdef.saveScreenFx.saveScreenParam.screenTimerId = screenTimerId;
}

void __cdecl CG_BlendSavedScreenShockBlurred(
                int fadeMsec,
                float viewX,
                float viewY,
                float viewWidth,
                float viewHeight,
                int screenTimerId)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-8h]

    if ( fadeMsec > 0 )
    {
        cgameGlob = CG_GetLocalClientGlobals(screenTimerId);
        cgameGlob->refdef.saveScreenFx.blendBlurredParam.enabled = 1;
        cgameGlob->refdef.saveScreenFx.blendBlurredParam.fadeMsec = fadeMsec;
        cgameGlob->refdef.saveScreenFx.blendBlurredParam.s0 = viewX;
        cgameGlob->refdef.saveScreenFx.blendBlurredParam.t0 = viewY;
        cgameGlob->refdef.saveScreenFx.blendBlurredParam.ds = viewWidth;
        cgameGlob->refdef.saveScreenFx.blendBlurredParam.dt = viewHeight;
        cgameGlob->refdef.saveScreenFx.blendBlurredParam.screenTimerId = screenTimerId;
    }
}

void __cdecl CG_BlendSavedScreenShockFlashed(
                int localClientNum,
                float intensityWhiteout,
                float intensityScreengrab,
                float viewX,
                float viewY,
                float viewWidth,
                float viewHeight)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-8h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    cgameGlob->refdef.saveScreenFx.blendFlashedParam.enabled = 1;
    cgameGlob->refdef.saveScreenFx.blendFlashedParam.intensityWhiteout = intensityWhiteout;
    cgameGlob->refdef.saveScreenFx.blendFlashedParam.intensityScreengrab = intensityScreengrab;
    cgameGlob->refdef.saveScreenFx.blendFlashedParam.s0 = viewX;
    cgameGlob->refdef.saveScreenFx.blendFlashedParam.t0 = viewY;
    cgameGlob->refdef.saveScreenFx.blendFlashedParam.ds = viewWidth;
    cgameGlob->refdef.saveScreenFx.blendFlashedParam.dt = viewHeight;
}

void __cdecl CG_ResetFullscreenFX(int localClientNum)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob )
    {
        cgameGlob->healthOverlayPulseTime = 0;
        cgameGlob->healthOverlayHurt = 0;
        cgameGlob->healthOverlayToAlpha = 0.0f;
        cgameGlob->healthOverlayPulsePhase = 0;
        cgameGlob->refdef.poisonFx.curAmount = 0.0f;
        cgameGlob->refdef.poisonFx.curAmountTarget = 0.0f;
        if ( cgameGlob->visionNameNaked[0] )
            CG_VisionSetStartLerp_To(
                cgameGlob->localClientNum,
                VISIONSETMODE_NAKED,
                VISIONSETLERP_TO_SMOOTH,
                cgameGlob->visionNameNaked,
                0);
        CG_ClearBurn(localClientNum);
        cgameGlob->refdef.flameFx.enabled = 0;
    }
}

float __cdecl CG_GetViewZoomScale()
{
    return cgArray[0].refdef.tanHalfFovY * 1.5890048f;
}