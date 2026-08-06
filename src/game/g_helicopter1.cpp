#include "g_helicopter1.h"
#include <universal/com_math_anglevectors.h>
#include <clientscript/cscr_vm.h>
#include <game_mp/g_spawn_mp.h>
#include <clientscript/scr_const.h>
#include <bgame/bg_misc.h>
#include <game_mp/g_utils_mp.h>

const dvar_t *vehHelicopterMaxAccelVertical;
const dvar_t *vehHelicopterLookaheadTime;
const dvar_t *vehHelicopterHoverSpeedThreshold;
const dvar_t *vehHelicopterRightStickDeadzone;
const dvar_t *vehHelicopterStrafeDeadzone;
const dvar_t *vehHelicopterScaleMovement;
const dvar_t *vehHelicopterYawAltitudeControls;
const dvar_t *vehHelicopterSoftCollisions;
const dvar_t *vehHelicopterDecelerationFwd;
const dvar_t *vehHelicopterDecelerationSide;
const dvar_t *vehHelicopterInvertUpDown;
const dvar_t *vehHelicopterYawOnLeftStick;
const dvar_t *vehHelicopterTiltFromControllerAxes;
const dvar_t *vehHelicopterTiltFromFwdAndYaw_VelAtMaxTilt;
const dvar_t *vehHelicopterTiltFromViewangles;
const dvar_t *vehHelicopterJitterJerkyness;
const dvar_t *vehHelicopterHeadSwayDontSwayTheTurret;
const dvar_t *vehHelicopterTiltMomentum;
const dvar_t *vehHelicopterAlwaysFaceCamera;
const dvar_t *vehHelicopterAlwaysFaceCameraRate;
const dvar_t *vehHelicopterPathTransitionTime;
const dvar_t *vehHelicopterFreeLook;
const dvar_t *vehHelicopterDefaultPitch;
const dvar_t *vehHelicopterRotDecel;
const dvar_t *vehHelicopterTiltFromFwdAndYaw;
const dvar_t *vehHelicopterMaxSpeedVertical;
const dvar_t *vehHelicopterboundsOn;
const dvar_t *vehHelicopterboundMapUpperLeftX;
const dvar_t *vehHelicopterboundMapUpperLeftY;
const dvar_t *vehHelicopterboundMapLowerRightX;
const dvar_t *vehHelicopterboundMapLowerRightY;
const dvar_t *vehHelicopterboundMapHeight;

const char *g_helicopterYawAltitudeControlsNames[4] =
{
  "Altitude and yaw are both controlled by the thumbstick",
  "Altitude: thumbstick         Yaw: bumper buttons",
  "Altitude: bumper buttons     Yaw: thumbstick",
  NULL
};

float stopInputRange = 15.0;
float inputFallOffRange = 50.0;
float lookAheadTime = 1.0;
float vel_landing_scale = 0.5;
float max_height_offset = 2000.0;
float frame_inc = 50.0;
float velInfluence = 12.0;
float controllerInfleunce = 10.0;
float viewDiffInfleunce = 80.0;
float aircraftRotLag = 1.0;



void __cdecl Helicopter_RegisterDvars()
{
    vehHelicopterMaxAccelVertical = _Dvar_RegisterFloat(
                                                                        "vehHelicopterMaxAccelVertical",
                                                                        60.0,
                                                                        0.0099999998,
                                                                        3.4028235e38,
                                                                        0x5080u,
                                                                        "Maximum vertical acceleration of the player helicopter (in MPH per second)");
    vehHelicopterLookaheadTime = _Dvar_RegisterFloat(
                                                                 "vehHelicopterLookaheadTime",
                                                                 1.0,
                                                                 0.0099999998,
                                                                 3.4028235e38,
                                                                 0x80u,
                                                                 "How far ahead (in seconds) the player helicopter looks ahead, to avoid hard collisions."
                                                                 "    (Like driving down the highway, you should keep 2 seconds distance between you and th"
                                                                 "e vehicle in front of you)");
    vehHelicopterHoverSpeedThreshold = _Dvar_RegisterFloat(
                                                                             "vehHelicopterHoverSpeedThreshold",
                                                                             400.0,
                                                                             0.0099999998,
                                                                             3.4028235e38,
                                                                             0x80u,
                                                                             "The speed below which the player helicopter begins to jitter the tilt, for hovering");
    vehHelicopterRightStickDeadzone = _Dvar_RegisterFloat(
                                                                            "vehHelicopterRightStickDeadzone",
                                                                            0.30000001,
                                                                            0.0099999998,
                                                                            1.0,
                                                                            0x80u,
                                                                            "Dead-zone for the axes of the right thumbstick.    This helps to better control the "
                                                                            "two axes separately.");
    vehHelicopterStrafeDeadzone = _Dvar_RegisterFloat(
                                                                    "vehHelicopterStrafeDeadzone",
                                                                    0.30000001,
                                                                    0.0099999998,
                                                                    1.0,
                                                                    0x80u,
                                                                    "Dead-zone so that you can fly straight forward easily without accidentally strafing (a"
                                                                    "nd thus rolling).");
    vehHelicopterScaleMovement = _Dvar_RegisterBool(
                                                                 "vehHelicopterScaleMovement",
                                                                 1,
                                                                 0x80u,
                                                                 "Scales down the smaller of the left stick axes.");
    vehHelicopterYawAltitudeControls = _Dvar_RegisterEnum(
                                                                             "vehHelicopterYawAltitudeControls",
                                                                             g_helicopterYawAltitudeControlsNames,
                                                                             2,
                                                                             0,
                                                                             "Determines how to control yaw and altitude");
    vehHelicopterSoftCollisions = _Dvar_RegisterBool(
                                                                    "vehHelicopterSoftCollisions",
                                                                    0,
                                                                    0x80u,
                                                                    "Player helicopters have soft collisions (slow down before they collide).");
    vehHelicopterDecelerationFwd = _Dvar_RegisterFloat(
                                                                     "vehHelicopterDecelerationFwd",
                                                                     0.5,
                                                                     0.0,
                                                                     3.4028235e38,
                                                                     0x80u,
                                                                     "Set the deceleration of the player helicopter (as a fraction of acceleration) in the "
                                                                     "direction the chopper is facing.    So 1.0 makes it equal to the acceleration.");
    vehHelicopterDecelerationSide = _Dvar_RegisterFloat(
                                                                        "vehHelicopterDecelerationSide",
                                                                        1.0,
                                                                        0.0,
                                                                        3.4028235e38,
                                                                        0x80u,
                                                                        "Set the side-to-side deceleration of the player helicopter (as a fraction of acceler"
                                                                        "ation).    So 1.0 makes it equal to the acceleration.");
    vehHelicopterInvertUpDown = _Dvar_RegisterBool(
                                                                "vehHelicopterInvertUpDown",
                                                                0,
                                                                0x80u,
                                                                "Invert the altitude control on the player helicopter.");
    vehHelicopterYawOnLeftStick = _Dvar_RegisterFloat(
                                                                    "vehHelicopterYawOnLeftStick",
                                                                    5.0,
                                                                    0.0,
                                                                    3.4028235e38,
                                                                    0x80u,
                                                                    "The yaw speed created by the left stick when pushing the stick diagonally (e.g., movin"
                                                                    "g forward and strafing slightly).");
    vehHelicopterTiltFromControllerAxes = _Dvar_RegisterFloat(
                                                                                    "vehHelicopterTiltFromControllerAxes",
                                                                                    0.0,
                                                                                    0.0,
                                                                                    3.4028235e38,
                                                                                    0x80u,
                                                                                    "The amount of tilt caused by the desired velocity (i.e., the amount of control"
                                                                                    "ler stick deflection)");
    vehHelicopterTiltFromFwdAndYaw_VelAtMaxTilt = _Dvar_RegisterFloat(
                                                                                                    "vehHelicopterTiltFromFwdAndYaw_VelAtMaxTilt",
                                                                                                    1.0,
                                                                                                    0.0,
                                                                                                    3.4028235e38,
                                                                                                    0x4000u,
                                                                                                    "The forward speed (as a fraction of top speed) at which the tilt due t"
                                                                                                    "o yaw reaches is maximum value.");
    vehHelicopterTiltFromViewangles = _Dvar_RegisterFloat(
                                                                            "vehHelicopterTiltFromViewangles",
                                                                            3.0,
                                                                            0.1,
                                                                            20.0,
                                                                            0x80u,
                                                                            "The amount of tilt caused by the current velocity");
    vehHelicopterJitterJerkyness = _Dvar_RegisterFloat(
                                                                     "vehHelicopterJitterJerkyness",
                                                                     0.30000001,
                                                                     0.000099999997,
                                                                     3.4028235e38,
                                                                     0,
                                                                     "Specifies how jerky the tilt jitter should be");
    vehHelicopterHeadSwayDontSwayTheTurret = _Dvar_RegisterBool(
                                                                                         "vehHelicopterHeadSwayDontSwayTheTurret",
                                                                                         0,
                                                                                         0,
                                                                                         "If set, the turret will not fire through the crosshairs, but straight ahead"
                                                                                         " of the vehicle, when the player is not freelooking.");
    vehHelicopterTiltMomentum = _Dvar_RegisterFloat(
                                                                "vehHelicopterTiltMomentum",
                                                                0.40000001,
                                                                0.000099999997,
                                                                3.4028235e38,
                                                                0x80u,
                                                                "The amount of rotational momentum the helicopter has with regards to tilting.");
    vehHelicopterAlwaysFaceCamera = _Dvar_RegisterBool(
                                                                        "vehHelicopterAlwaysFaceCamera",
                                                                        1,
                                                                        0,
                                                                        "Setting this will have the helicopter always face the same direction as the camera.");
    vehHelicopterAlwaysFaceCameraRate = _Dvar_RegisterFloat(
                                                                                "vehHelicopterAlwaysFaceCameraRate",
                                                                                2.0,
                                                                                0.1,
                                                                                3.4028235e38,
                                                                                0x4000u,
                                                                                "How quickly the helicopter catches up to the camera.");
    vehHelicopterPathTransitionTime = _Dvar_RegisterFloat(
                                                                            "vehHelicopterPathTransitionTime",
                                                                            1.3,
                                                                            0.1,
                                                                            10.0,
                                                                            0x80u,
                                                                            "How quickly the helicopter transitions from driving path to locked path.");
    vehHelicopterFreeLook = _Dvar_RegisterBool("vehHelicopterFreeLook", 0, 0x80u, "Allow free look with left trigger.");
    vehHelicopterDefaultPitch = _Dvar_RegisterFloat(
                                                                "vehHelicopterDefaultPitch",
                                                                0.0,
                                                                -50.0,
                                                                50.0,
                                                                0x5000u,
                                                                "Set the default pitch of the helicopter.");
    vehHelicopterRotDecel = _Dvar_RegisterFloat(
                                                        "vehHelicopterRotDecel",
                                                        1.3,
                                                        0.0,
                                                        50.0,
                                                        0x4000u,
                                                        "Set the rotational deceleration or dampening for the helicopter.");
    vehHelicopterTiltFromFwdAndYaw = _Dvar_RegisterFloat(
                                                                         "vehHelicopterTiltFromFwdAndYaw",
                                                                         0.0,
                                                                         0.0,
                                                                         3.4028235e38,
                                                                         0x4000u,
                                                                         "The amount of roll caused by yawing while moving forward.");
    vehHelicopterMaxSpeedVertical = _Dvar_RegisterFloat(
                                                                        "vehHelicopterMaxSpeedVertical",
                                                                        65.0,
                                                                        0.0099999998,
                                                                        3.4028235e38,
                                                                        0x80u,
                                                                        "Maximum vertical speed of the player helicopter (in MPH)");
    vehHelicopterboundsOn = _Dvar_RegisterBool("vehHelicopterboundsOn", 0, 0x80u, "Heli bounds on/off.");
    vehHelicopterboundMapUpperLeftX = _Dvar_RegisterFloat(
                                                                            "vehHelicopterboundMapUpperLeftX",
                                                                            -1000.0,
                                                                            -3.4028235e38,
                                                                            3.4028235e38,
                                                                            0x80u,
                                                                            "Helicopter Bounding box.");
    vehHelicopterboundMapUpperLeftY = _Dvar_RegisterFloat(
                                                                            "vehHelicopterboundMapUpperLeftY",
                                                                            -1000.0,
                                                                            -3.4028235e38,
                                                                            3.4028235e38,
                                                                            0x80u,
                                                                            "Helicopter Bounding box.");
    vehHelicopterboundMapLowerRightX = _Dvar_RegisterFloat(
                                                                             "vehHelicopterboundMapLowerRightX",
                                                                             1000.0,
                                                                             -3.4028235e38,
                                                                             3.4028235e38,
                                                                             0x80u,
                                                                             "Helicopter Bounding box.");
    vehHelicopterboundMapLowerRightY = _Dvar_RegisterFloat(
                                                                             "vehHelicopterboundMapLowerRightY",
                                                                             1000.0,
                                                                             -3.4028235e38,
                                                                             3.4028235e38,
                                                                             0x80u,
                                                                             "Helicopter Bounding box.");
    vehHelicopterboundMapHeight = _Dvar_RegisterFloat(
                                                                    "vehHelicopterboundMapHeight",
                                                                    500.0,
                                                                    -3.4028235e38,
                                                                    3.4028235e38,
                                                                    0x80u,
                                                                    "Helicopter Bounding height.");
}

void __cdecl HELI_CancelAIMove(gentity_s *ent)
{
    float maxBodyRoll; // [esp+0h] [ebp-74h]
    float maxBodyPitch; // [esp+4h] [ebp-70h]
    float v3; // [esp+10h] [ebp-64h]
    const vehicle_info_t *info; // [esp+20h] [ebp-54h]
    scr_vehicle_s *veh; // [esp+24h] [ebp-50h]
    float tilt[3]; // [esp+28h] [ebp-4Ch] BYREF
    float maxRoll; // [esp+34h] [ebp-40h]
    float bodyMat[4][3]; // [esp+38h] [ebp-3Ch] BYREF
    float yawAngles[3]; // [esp+68h] [ebp-Ch] BYREF

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_helicopter.cpp", 463, 0, "%s", "ent") )
        __debugbreak();
    if ( !ent->scr_vehicle
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_helicopter.cpp", 464, 0, "%s", "ent->scr_vehicle") )
    {
        __debugbreak();
    }
    veh = ent->scr_vehicle;
    info = BG_GetVehicleInfo(veh->infoIdx);
    yawAngles[0] = 0.0f;
    yawAngles[1] = veh->phys.angles[1];
    yawAngles[2] = 0.0f;
    AngleVectors(yawAngles, bodyMat[0], bodyMat[1], bodyMat[2]);
    memset(bodyMat[3], 0, sizeof(float[3]));
    if ( info->maxBodyPitch <= 0.0 )
        maxBodyPitch = 1.0f;
    else
        maxBodyPitch = info->maxBodyPitch;
    if ( info->maxBodyRoll <= 0.0 )
        maxBodyRoll = 1.0f;
    else
        maxBodyRoll = info->maxBodyRoll;
    maxRoll = maxBodyRoll;
    v3 = veh->phys.angles[2] / maxBodyRoll;
    tilt[0] = veh->phys.angles[0] / maxBodyPitch;
    tilt[1] = v3;
    tilt[2] = 0.0f;
    MatrixTransformVector(tilt, bodyMat, veh->phys.worldTilt);
    veh->phys.worldTiltVel[0] = 0.0f;
    veh->phys.worldTiltVel[1] = 0.0f;
    veh->phys.worldTiltVel[2] = 0.0f;
}

void __cdecl VEH_CheckForPredictedCrash(gentity_s *ent)
{
    float value; // [esp+4h] [ebp-7Ch]
    col_context_t context; // [esp+8h] [ebp-78h] BYREF
    vehicle_physic_t *phys; // [esp+30h] [ebp-50h]
    scr_vehicle_s *veh; // [esp+34h] [ebp-4Ch]
    trace_t trace; // [esp+38h] [ebp-48h] BYREF
    float targetPos[3]; // [esp+74h] [ebp-Ch] BYREF

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_helicopter.cpp", 570, 0, "%s", "ent") )
        __debugbreak();
    if ( !ent->scr_vehicle
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_helicopter.cpp", 571, 0, "%s", "ent->scr_vehicle") )
    {
        __debugbreak();
    }
    veh = ent->scr_vehicle;
    phys = &veh->phys;
    if ( vehHelicopterLookaheadTime->current.value != 0.0
        && (phys->vel[0] != 0.0 || phys->vel[1] != 0.0 || phys->vel[2] != 0.0) )
    {
        value = vehHelicopterLookaheadTime->current.value;
        targetPos[0] = (float)(value * phys->vel[0]) + phys->origin[0];
        targetPos[1] = (float)(value * phys->vel[1]) + phys->origin[1];
        targetPos[2] = (float)(value * phys->vel[2]) + phys->origin[2];
        //col_context_t::col_context_t(&context);
        G_TraceCapsule(&trace, phys->origin, phys->mins, phys->maxs, targetPos, ent->s.number, ent->clipmask, &context);
        if ( trace.fraction < 1.0 )
        {
            Scr_AddVector(trace.normal.vec.v, SCRIPTINSTANCE_SERVER);
            Scr_AddVector(phys->vel, SCRIPTINSTANCE_SERVER);
            Scr_Notify(ent, scr_const.veh_predictedcollision, 2u);
        }
    }
}

void __cdecl VEH_CheckForCrash(gentity_s *ent, float *newPosition)
{
    col_context_t context; // [esp+0h] [ebp-6Ch] BYREF
    vehicle_physic_t *phys; // [esp+28h] [ebp-44h]
    scr_vehicle_s *veh; // [esp+2Ch] [ebp-40h]
    trace_t trace; // [esp+30h] [ebp-3Ch] BYREF

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_helicopter.cpp", 601, 0, "%s", "ent") )
        __debugbreak();
    if ( !ent->scr_vehicle
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_helicopter.cpp", 602, 0, "%s", "ent->scr_vehicle") )
    {
        __debugbreak();
    }
    veh = ent->scr_vehicle;
    phys = &veh->phys;
    if ( veh->phys.vel[0] != 0.0 || phys->vel[1] != 0.0 || phys->vel[2] != 0.0 )
    {
        //col_context_t::col_context_t(&context);
        G_TraceCapsule(&trace, phys->origin, phys->mins, phys->maxs, newPosition, ent->s.number, ent->clipmask, &context);
        if ( trace.fraction < 1.0 )
        {
            Scr_AddVector(trace.normal.vec.v, SCRIPTINSTANCE_SERVER);
            Scr_AddVector(phys->vel, SCRIPTINSTANCE_SERVER);
            Scr_Notify(ent, scr_const.veh_collision, 2u);
        }
    }
}

char __cdecl VEH_HandleHeliLockHeight(gentity_s *ent)
{
    scr_vehicle_s *veh; // [esp+28h] [ebp-20h]
    float deltaHeight; // [esp+2Ch] [ebp-1Ch]
    float testPoint[3]; // [esp+34h] [ebp-14h] BYREF
    float moveStrength; // [esp+40h] [ebp-8h]
    float absDeltaHeight; // [esp+44h] [ebp-4h]

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_helicopter.cpp", 627, 0, "%s", "ent") )
        __debugbreak();
    if ( !ent->scr_vehicle
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_helicopter.cpp", 628, 0, "%s", "ent->scr_vehicle") )
    {
        __debugbreak();
    }
    veh = ent->scr_vehicle;
    if ( (veh->flags & 0x200) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_helicopter.cpp",
                    633,
                    0,
                    "%s",
                    "veh->flags & VEHFLAG_LOCK_HELI_HEIGHT") )
    {
        __debugbreak();
    }
    testPoint[0] = (float)(lookAheadTime * veh->phys.vel[0]) + veh->phys.origin[0];
    testPoint[1] = (float)(lookAheadTime * veh->phys.vel[1]) + veh->phys.origin[1];
    testPoint[2] = (float)(lookAheadTime * veh->phys.vel[2]) + veh->phys.origin[2];
    testPoint[2] = testPoint[2] + veh->phys.mins[2];
    testPoint[2] = testPoint[2] - veh->heliHeightLockOffset;
    if ( !CM_GetHeliHeight(testPoint, 200.0, &veh->phys.heliLockHeight) )
        CM_GetHeliHeight(testPoint, 3000.0, &veh->phys.heliLockHeight);
    deltaHeight = testPoint[2] - veh->phys.heliLockHeight;
    moveStrength = 1.0f;
    absDeltaHeight = fabs(deltaHeight);
    if ( inputFallOffRange > fabs(deltaHeight) )
    {
        if ( stopInputRange <= absDeltaHeight )
            moveStrength = absDeltaHeight / inputFallOffRange;
        else
            moveStrength = 0.0f;
    }
    if ( (moveStrength < 0.0 || moveStrength > 1.0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_helicopter.cpp",
                    660,
                    0,
                    "moveStrength not in [0.0f, 1.0f]\n\t%g not in [%g, %g]",
                    moveStrength,
                    0.0,
                    1.0) )
    {
        __debugbreak();
    }
    if ( deltaHeight >= 0.0 )
    {
        if ( veh->phys.vel[2] > 4.0 )
            moveStrength = 1.0f;
        return (int)(float)(-127.0 * moveStrength);
    }
    else
    {
        if ( veh->phys.vel[2] < -4.0 )
            moveStrength = 1.0f;
        return (int)(float)(127.0 * moveStrength);
    }
}

void __cdecl VEH_UpdateClientChopper(gentity_s *ent)
{
    gclient_s *client; // eax
    double v2; // st7
    bool v3; // eax
    double v4; // st7
    float v6; // [esp+10h] [ebp-190h]
    float v7; // [esp+14h] [ebp-18Ch]
    float v8; // [esp+18h] [ebp-188h]
    float v9; // [esp+1Ch] [ebp-184h]
    float v10; // [esp+20h] [ebp-180h]
    float v11; // [esp+24h] [ebp-17Ch]
    int integer; // [esp+28h] [ebp-178h]
    float *v13; // [esp+30h] [ebp-170h]
    float v14; // [esp+34h] [ebp-16Ch]
    float *v15; // [esp+38h] [ebp-168h]
    float *vel; // [esp+50h] [ebp-150h]
    float *v17; // [esp+54h] [ebp-14Ch]
    float v18; // [esp+6Ch] [ebp-134h]
    float heliHeightLockOffset; // [esp+7Ch] [ebp-124h]
    float v20; // [esp+80h] [ebp-120h]
    float v21; // [esp+88h] [ebp-118h]
    float v22; // [esp+8Ch] [ebp-114h]
    float v23; // [esp+90h] [ebp-110h]
    float v24; // [esp+9Ch] [ebp-104h]
    float v25; // [esp+A4h] [ebp-FCh]
    float v26; // [esp+A8h] [ebp-F8h]
    float moveVec[2]; // [esp+BCh] [ebp-E4h] BYREF
    float yaw; // [esp+C4h] [ebp-DCh]
    float len; // [esp+C8h] [ebp-D8h]
    float temp[3]; // [esp+CCh] [ebp-D4h] BYREF
    float deltaYaw; // [esp+D8h] [ebp-C8h]
    float velRatio; // [esp+DCh] [ebp-C4h]
    float fraction; // [esp+E0h] [ebp-C0h]
    float controllerMove; // [esp+E4h] [ebp-BCh]
    float viewDiffMove; // [esp+E8h] [ebp-B8h]
    float diff; // [esp+ECh] [ebp-B4h]
    float scale; // [esp+F0h] [ebp-B0h]
    vehicle_physic_t *phys; // [esp+F4h] [ebp-ACh]
    const vehicle_info_t *info; // [esp+F8h] [ebp-A8h]
    scr_vehicle_s *veh; // [esp+FCh] [ebp-A4h]
    char move[7]; // [esp+100h] [ebp-A0h] BYREF
    bool bumped; // [esp+107h] [ebp-99h]
    usercmd_s *usercmd; // [esp+108h] [ebp-98h]
    float rotAccel[3]; // [esp+10Ch] [ebp-94h] BYREF
    float startPos[3]; // [esp+118h] [ebp-88h]
    float viewangles[3]; // [esp+124h] [ebp-7Ch] BYREF
    float collision[3]; // [esp+130h] [ebp-70h] BYREF
    float bodyAccel[3]; // [esp+13Ch] [ebp-64h] BYREF
    float worldAccel[3]; // [esp+148h] [ebp-58h] BYREF
    float axis[4][3]; // [esp+154h] [ebp-4Ch] BYREF
    float yawAngles[3]; // [esp+184h] [ebp-1Ch] BYREF
    float startVel[3]; // [esp+190h] [ebp-10h] BYREF
    gentity_s *player; // [esp+19Ch] [ebp-4h]

    move[0] = 0;
    move[1] = 0;
    move[2] = 0;
    move[3] = 0;
    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_helicopter.cpp", 701, 0, "%s", "ent") )
        __debugbreak();
    if ( !ent->scr_vehicle
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_helicopter.cpp", 702, 0, "%s", "ent->scr_vehicle") )
    {
        __debugbreak();
    }
    memset(viewangles, 0, sizeof(viewangles));
    veh = ent->scr_vehicle;
    phys = &veh->phys;
    info = BG_GetVehicleInfo(veh->infoIdx);
    player = VEH_GetSeatOccupantEntity(ent, 0);
    if ( player && player->client && !player->client->ps.vehiclePos )
    {
        player->client->linkAnglesFrac = 0.0f;
        usercmd = &player->client->sess.cmd;
        if ( (player->client->ps.pm_flags & 0xC00) == 0 )
        {
            move[0] = usercmd->forwardmove;
            move[1] = usercmd->rightmove;
            //if ( bitarray<51>::testBit(&usercmd->button_bits, 0xBu) && vehHelicopterFreeLook->current.enabled )
            if ( usercmd->button_bits.testBit(0xBu) && vehHelicopterFreeLook->current.enabled )
            {
                player->client->linkAnglesMinClamp[1] = -info->turretViewLimits.horizSpanRight;
                player->client->linkAnglesMaxClamp[1] = info->turretViewLimits.horizSpanLeft;
                player->client->linkAnglesMinClamp[0] = -info->turretViewLimits.vertSpanUp;
                player->client->linkAnglesMaxClamp[0] = info->turretViewLimits.vertSpanDown;
            }
            else
            {
                integer = vehHelicopterYawAltitudeControls->current.integer;
                if ( integer )
                {
                    if ( integer == 1 )
                    {
                        move[2] = usercmd->pitchmove;
                        //if ( bitarray<51>::testBit(&usercmd->button_bits, 0x24u) )
                        if ( usercmd->button_bits.testBit(0x24u) )
                            move[3] = 127;
                        //if ( bitarray<51>::testBit(&usercmd->button_bits, 0x25u) )
                        if ( usercmd->button_bits.testBit(0x25u) )
                            move[3] -= 127;
                    }
                    else if ( integer == 2 )
                    {
                        move[3] = usercmd->yawmove;
                        //if ( bitarray<51>::testBit(&usercmd->button_bits, 0x24u) )
                        if ( usercmd->button_bits.testBit(0x24u) )
                            move[2] = 127;
                        //if ( bitarray<51>::testBit(&usercmd->button_bits, 0x25u) )
                        if ( usercmd->button_bits.testBit(0x25u) )
                            move[2] -= 127;
                    }
                }
                else
                {
                    move[2] = usercmd->pitchmove;
                    move[3] = usercmd->yawmove;
                }
                if ( vehHelicopterInvertUpDown->current.enabled )
                    move[2] = -move[2];
            }
            if ( vehHelicopterAlwaysFaceCamera->current.enabled )
            {
                scale = vehHelicopterAlwaysFaceCameraRate->current.value;
                diff = AngleNormalize180(player->client->ps.viewangles[1] - veh->phys.angles[1]);
                if ( info->steerGraph )
                {
                    v25 = (float)(diff / info->turretViewLimits.horizSpanLeft) * scale;
                    if ( (float)(v25 - 1.0) < 0.0 )
                        v26 = (float)(diff / info->turretViewLimits.horizSpanLeft) * scale;
                    else
                        v26 = 1.0f;
                    if ( (float)(-1.0 - v25) < 0.0 )
                        v11 = v26;
                    else
                        v11 = -1.0f;
                    fraction = v11;
                    fraction = GraphFloat_GetValue(info->steerGraph, fabs(v11));
                    if ( diff < 0.0 )
                        fraction = fraction * -1.0;
                    if ( (float)((float)(fraction * 128.0) - viewDiffInfleunce) < 0.0 )
                        v24 = fraction * 128.0;
                    else
                        v24 = viewDiffInfleunce;
                    if ( (float)((-(viewDiffInfleunce)) - (float)(fraction * 128.0)) < 0.0 )
                        v10 = v24;
                    else
                        v10 = -viewDiffInfleunce;
                    viewDiffMove = v10;
                    controllerMove = 0.0f;
                    if ( (player->client->ps.pm_flags & 0xC00) == 0 )
                        controllerMove = (float)((float)usercmd->yawmove / 128.0) * controllerInfleunce;
                    move[3] = (int)(float)(viewDiffMove + controllerMove);
                }
                else
                {
                    velRatio = phys->rotVel[1] * velInfluence;
                    v22 = (float)(diff * scale) - velRatio;
                    if ( (float)(v22 - 180.0) < 0.0 )
                        v23 = (float)(diff * scale) - velRatio;
                    else
                        v23 = 180.0f;
                    if ( (float)(-180.0 - v22) < 0.0 )
                        v9 = v23;
                    else
                        v9 = -180.0f;
                    diff = v9;
                    if ( (float)(v9 - 127.0) < 0.0 )
                        v21 = diff;
                    else
                        v21 = 127.0f;
                    if ( (float)(-127.0 - diff) < 0.0 )
                        v8 = v21;
                    else
                        v8 = -127.0f;
                    move[3] = (int)v8;
                }
                //if ( bitarray<51>::testBit(&usercmd->button_bits, 0xBu) && vehHelicopterFreeLook->current.enabled )
                if ( usercmd->button_bits.testBit(0xBu) && vehHelicopterFreeLook->current.enabled )
                {
                    move[3] = 0;
                }
                else
                {
                    client = player->client;
                    viewangles[0] = client->ps.viewangles[0];
                    viewangles[1] = client->ps.viewangles[1];
                    viewangles[2] = client->ps.viewangles[2];
                }
            }
        }
        if ( (veh->flags & 0x200) != 0 )
        {
            //if ( bitarray<51>::testBit(&usercmd->button_bits, 0x24u) )
            if ( usercmd->button_bits.testBit(0x24u) )
            {
                veh->heliHeightLockOffset = veh->heliHeightLockOffset + frame_inc;
            }
            //else if ( bitarray<51>::testBit(&usercmd->button_bits, 0x25u) )
            else if ( usercmd->button_bits.testBit(0x25u) )
            {
                veh->heliHeightLockOffset = veh->heliHeightLockOffset - frame_inc;
            }
            heliHeightLockOffset = veh->heliHeightLockOffset;
            if ( (float)(heliHeightLockOffset - max_height_offset) < 0.0 )
                v20 = veh->heliHeightLockOffset;
            else
                v20 = max_height_offset;
            if ( (float)(0.0 - heliHeightLockOffset) < 0.0 )
                v7 = v20;
            else
                v7 = 0.0f;
            veh->heliHeightLockOffset = v7;
            move[2] = VEH_HandleHeliLockHeight(ent);
        }
        if ( player_topDownCamMode->current.integer >= 2 )
        {
            moveVec[0] = (float)usercmd->forwardmove;
            //LODWORD(moveVec[1]) = COERCE_UNSIGNED_INT((float)usercmd->rightmove) ^ _mask__NegFloat_;
            (moveVec[1]) = -((float)usercmd->rightmove);
            len = Vec2Normalize(moveVec);
            if ( (float)(len - 127.0) < 0.0 )
                v18 = len;
            else
                v18 = 127.0f;
            if ( (float)(-127.0 - len) < 0.0 )
                v6 = v18;
            else
                v6 = -127.0f;
            len = v6;
            yaw = vectoyaw(moveVec) + player_topDownCamAngles->current.vector[1];
            deltaYaw = -AngleDelta(yaw, phys->angles[1]);
            YawVectors(deltaYaw, temp, 0);
            move[0] = (int)(float)(temp[0] * len);
            move[1] = (int)(float)(temp[1] * len);
        }
    }
    HELI_CalcAccel(ent, move, bodyAccel, rotAccel, viewangles);
    phys->rotVel[1] = (float)(rotAccel[1] * 0.05f) + phys->rotVel[1];
    if ( vehHelicopterAlwaysFaceCamera->current.enabled )
        phys->rotVel[1] = phys->rotVel[1]
                                        - (float)((float)(phys->rotVel[1] * 0.05f) * vehHelicopterRotDecel->current.value);
    v2 = AngleNormalize180((float)(phys->rotVel[1] * 0.05f) + phys->prevAngles[1]);
    phys->angles[1] = v2;
    phys->angles[0] = rotAccel[0];
    phys->angles[2] = rotAccel[2];
    if ( ((LODWORD(phys->angles[0]) & 0x7F800000) == 0x7F800000
         || (LODWORD(phys->angles[1]) & 0x7F800000) == 0x7F800000
         || (LODWORD(phys->angles[2]) & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_helicopter.cpp",
                    875,
                    0,
                    "%s",
                    "!IS_NAN((phys->angles)[0]) && !IS_NAN((phys->angles)[1]) && !IS_NAN((phys->angles)[2])") )
    {
        __debugbreak();
    }
    yawAngles[0] = 0.0f;
    yawAngles[1] = phys->angles[1];
    yawAngles[2] = 0.0f;
    AngleVectors(yawAngles, axis[0], axis[1], axis[2]);
    memset(axis[3], 0, sizeof(float[3]));
    MatrixTransformVector(bodyAccel, axis, worldAccel);
    if ( vehHelicopterSoftCollisions->current.enabled )
        HELI_SoftenCollisions(ent, worldAccel);
    vel = phys->vel;
    v17 = phys->vel;
    phys->vel[0] = (float)(0.05f * worldAccel[0]) + phys->vel[0];
    vel[1] = (float)(0.05f * worldAccel[1]) + v17[1];
    vel[2] = (float)(0.05f * worldAccel[2]) + v17[2];
    if ( phys->vel[0] != 0.0 || phys->vel[1] != 0.0 || phys->vel[2] != 0.0 )
    {
        startVel[0] = phys->vel[0];
        startVel[1] = phys->vel[1];
        startVel[2] = phys->vel[2];
        startPos[0] = phys->origin[0];
        startPos[1] = phys->origin[1];
        startPos[2] = phys->origin[2];
        VEH_ClearGround();
        v3 = VEH_SlideMove(ent, 0);
        bumped = v3;
        if ( v3 )
        {
            collision[0] = (float)(0.05f * startVel[0]) + startPos[0];
            collision[1] = (float)(0.05f * startVel[1]) + startPos[1];
            collision[2] = (float)(0.05f * startVel[2]) + startPos[2];
            collision[0] = phys->origin[0] - collision[0];
            collision[1] = phys->origin[1] - collision[1];
            collision[2] = phys->origin[2] - collision[2];
            if ( Vec3Normalize(collision) > 0.0 )
            {
                Scr_AddVector(collision, SCRIPTINSTANCE_SERVER);
                Scr_AddVector(startVel, SCRIPTINSTANCE_SERVER);
                Scr_Notify(ent, scr_const.veh_collision, 2u);
                if ( collision[2] > 0.94999999 )
                {
                    v13 = phys->vel;
                    v14 = vel_landing_scale;
                    v15 = phys->vel;
                    phys->vel[0] = vel_landing_scale * phys->vel[0];
                    v13[1] = v14 * v15[1];
                    v13[2] = v14 * v15[2];
                }
            }
        }
    }
    VEH_CheckForPredictedCrash(ent);
    MatrixTransposeTransformVector43(phys->vel, axis, phys->bodyVel);
    VEH_UpdateVelocityWithRotation(ent);
    v4 = Vec3Length(phys->vel);
    veh->speed = v4;
    if ( veh->speed < 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_helicopter.cpp",
                    928,
                    0,
                    "%s",
                    "veh->speed >= 0.0f") )
    {
        __debugbreak();
    }
    if ( info->isDrivable && vehHelicopterboundsOn->current.enabled )
    {
        phys->origin[2] = vehHelicopterboundMapHeight->current.value;
        if ( vehHelicopterboundMapUpperLeftX->current.value > phys->origin[0] )
            phys->origin[0] = vehHelicopterboundMapUpperLeftX->current.value;
        if ( phys->origin[0] > vehHelicopterboundMapLowerRightX->current.value )
            phys->origin[0] = vehHelicopterboundMapLowerRightX->current.value;
        if ( vehHelicopterboundMapUpperLeftY->current.value > phys->origin[1] )
            phys->origin[1] = vehHelicopterboundMapUpperLeftY->current.value;
        if ( phys->origin[1] > vehHelicopterboundMapLowerRightY->current.value )
            phys->origin[1] = vehHelicopterboundMapLowerRightY->current.value;
    }
}

void __cdecl HELI_CalcAccel(gentity_s *ent, char *move, float *bodyAccel, float *rotAccel, float *viewangles)
{
    float v5; // [esp+10h] [ebp-240h]
    float v6; // [esp+14h] [ebp-23Ch]
    float v7; // [esp+18h] [ebp-238h]
    float tgt; // [esp+1Ch] [ebp-234h]
    float v9; // [esp+20h] [ebp-230h]
    float v10; // [esp+24h] [ebp-22Ch]
    float v11; // [esp+28h] [ebp-228h]
    float v12; // [esp+2Ch] [ebp-224h]
    float v13; // [esp+6Ch] [ebp-1E4h]
    float v14; // [esp+74h] [ebp-1DCh]
    float v15; // [esp+80h] [ebp-1D0h]
    float v16; // [esp+88h] [ebp-1C8h]
    float accel; // [esp+94h] [ebp-1BCh]
    float rotRate; // [esp+A0h] [ebp-1B0h]
    float v19; // [esp+A4h] [ebp-1ACh]
    float v20; // [esp+ACh] [ebp-1A4h]
    float v21; // [esp+B0h] [ebp-1A0h]
    float v22; // [esp+BCh] [ebp-194h]
    float v23; // [esp+C0h] [ebp-190h]
    float v24; // [esp+CCh] [ebp-184h]
    float v25; // [esp+D0h] [ebp-180h]
    float v26; // [esp+DCh] [ebp-174h]
    int integer; // [esp+E4h] [ebp-16Ch]
    float v28; // [esp+108h] [ebp-148h]
    float v29; // [esp+10Ch] [ebp-144h]
    float frac; // [esp+128h] [ebp-128h]
    float velScale; // [esp+12Ch] [ebp-124h]
    float velScalea; // [esp+12Ch] [ebp-124h]
    float speedParallel; // [esp+134h] [ebp-11Ch]
    vehicle_physic_t *phys; // [esp+138h] [ebp-118h]
    const vehicle_info_t *info; // [esp+13Ch] [ebp-114h]
    float tgtSpeed; // [esp+140h] [ebp-110h]
    float velParallel[3]; // [esp+144h] [ebp-10Ch]
    float deltaTilt[2]; // [esp+150h] [ebp-100h]
    scr_vehicle_s *veh; // [esp+158h] [ebp-F8h]
    float tgtDir[3]; // [esp+15Ch] [ebp-F4h] BYREF
    float targetTilt[3]; // [esp+168h] [ebp-E8h] BYREF
    float tgtVel[3]; // [esp+174h] [ebp-DCh] BYREF
    float controllerFrac[4]; // [esp+180h] [ebp-D0h] BYREF
    float oldTiltVel[2]; // [esp+190h] [ebp-C0h]
    float speedSq; // [esp+198h] [ebp-B8h]
    float tiltAccel[2]; // [esp+19Ch] [ebp-B4h]
    float tgtYawVel; // [esp+1A4h] [ebp-ACh]
    float track[4]; // [esp+1A8h] [ebp-A8h]
    float decel[2]; // [esp+1B8h] [ebp-98h]
    float maxSpeed[3]; // [esp+1C0h] [ebp-90h]
    float nextState; // [esp+1CCh] [ebp-84h]
    float currentMaxSpeed; // [esp+1D0h] [ebp-80h]
    float bodyMat[4][3]; // [esp+1D4h] [ebp-7Ch] BYREF
    float newAccel[2]; // [esp+204h] [ebp-4Ch]
    int axis; // [esp+20Ch] [ebp-44h]
    float yawAngles[3]; // [esp+210h] [ebp-40h] BYREF
    float viewAnglesTargetPitch; // [esp+21Ch] [ebp-34h]
    float maxAccel[3]; // [esp+220h] [ebp-30h]
    float deltaFracToApply; // [esp+22Ch] [ebp-24h]
    float worldTargetTilt[3]; // [esp+230h] [ebp-20h] BYREF
    float velOrthogonal[3]; // [esp+23Ch] [ebp-14h]
    float newDecel[2]; // [esp+248h] [ebp-8h]

    veh = ent->scr_vehicle;
    phys = &veh->phys;
    info = BG_GetVehicleInfo(veh->infoIdx);
    if ( veh->joltTime <= 0.0 )
    {
        if ( veh->maxSpeedOverride <= 0.0 )
            currentMaxSpeed = info->maxSpeed;
        else
            currentMaxSpeed = veh->maxSpeedOverride;
        v29 = 17.6 * vehHelicopterMaxSpeedVertical->current.value;
        maxSpeed[0] = currentMaxSpeed;
        maxSpeed[1] = currentMaxSpeed;
        maxSpeed[2] = v29;
        v28 = 17.6 * vehHelicopterMaxAccelVertical->current.value;
        maxAccel[0] = info->accel;
        maxAccel[1] = maxAccel[0];
        maxAccel[2] = v28;
    }
    else
    {
        maxSpeed[0] = veh->joltSpeed;
        maxSpeed[1] = maxSpeed[0];
        maxSpeed[2] = maxSpeed[0];
        maxAccel[0] = veh->joltDecel;
        maxAccel[1] = maxAccel[0];
        maxAccel[2] = maxAccel[0];
    }
    HELI_CmdScale(move, controllerFrac);
    for ( axis = 0; axis < 3; ++axis )
    {
        if ( maxSpeed[axis] == 0.0 )
            track[axis] = 1.0f;
        else
            track[axis] = maxAccel[axis] / maxSpeed[axis];
    }
    tgtVel[0] = maxSpeed[0] * controllerFrac[0];
    tgtVel[1] = maxSpeed[1] * controllerFrac[1];
    tgtVel[2] = maxSpeed[2] * controllerFrac[2];
    tgtSpeed = Vec3NormalizeTo(tgtVel, tgtDir);
    speedParallel = (float)((float)(phys->bodyVel[0] * tgtDir[0]) + (float)(phys->bodyVel[1] * tgtDir[1]))
                                + (float)(phys->bodyVel[2] * tgtDir[2]);
    velOrthogonal[0] = (float)((-(speedParallel)) * tgtDir[0]) + phys->bodyVel[0];
    velOrthogonal[1] = (float)((-(speedParallel)) * tgtDir[1]) + phys->bodyVel[1];
    velOrthogonal[2] = (float)((-(speedParallel)) * tgtDir[2]) + phys->bodyVel[2];
    velParallel[0] = speedParallel * tgtDir[0];
    velParallel[1] = speedParallel * tgtDir[1];
    velParallel[2] = speedParallel * tgtDir[2];
    integer = vehHelicopterDecelerationSide->current.integer;
    decel[0] = vehHelicopterDecelerationFwd->current.value;
    LODWORD(decel[1]) = integer;
    newAccel[0] = 0.0f;
    newAccel[1] = 0.0f;
    newDecel[0] = 0.0f;
    newDecel[1] = 0.0f;
    for ( axis = 0; axis < 2; ++axis )
    {
        nextState = DiffTrack(0.0, velOrthogonal[axis], decel[axis] * track[axis], 0.05f);
        newDecel[axis] = (float)(nextState - velOrthogonal[axis]) / 0.05f;
        if ( tgtSpeed <= speedParallel )
        {
            nextState = DiffTrack(tgtVel[axis], velParallel[axis], decel[axis] * track[axis], 0.05f);
            newDecel[axis] = (float)((float)(nextState - velParallel[axis]) / 0.05f) + newDecel[axis];
        }
        else
        {
            nextState = DiffTrack(tgtVel[axis], velParallel[axis], track[axis], 0.05f);
            newAccel[axis] = (float)((float)(nextState - velParallel[axis]) / 0.05f) + newAccel[axis];
        }
        bodyAccel[axis] = newDecel[axis] + newAccel[axis];
        v25 = bodyAccel[axis];
        if ( (float)(v25 - maxAccel[axis]) < 0.0 )
            v26 = bodyAccel[axis];
        else
            v26 = maxAccel[axis];
        if ( (float)((-(maxAccel[axis])) - v25) < 0.0 )
            v12 = v26;
        else
            v12 = -maxAccel[axis];
        bodyAccel[axis] = v12;
        v23 = newDecel[axis];
        if ( (float)(v23 - maxAccel[axis]) < 0.0 )
            v24 = newDecel[axis];
        else
            v24 = maxAccel[axis];
        if ( (float)((-(maxAccel[axis])) - v23) < 0.0 )
            v11 = v24;
        else
            v11 = -maxAccel[axis];
        newDecel[axis] = v11;
        v21 = newAccel[axis];
        if ( (float)(v21 - maxAccel[axis]) < 0.0 )
            v22 = newAccel[axis];
        else
            v22 = maxAccel[axis];
        if ( (float)((-(maxAccel[axis])) - v21) < 0.0 )
            v10 = v22;
        else
            v10 = -maxAccel[axis];
        newAccel[axis] = v10;
    }
    nextState = DiffTrack(tgtVel[2], phys->bodyVel[2], track[2], 0.05f);
    bodyAccel[2] = (float)(nextState - phys->bodyVel[2]) / 0.05f;
    v19 = bodyAccel[2];
    if ( (float)(v19 - maxAccel[2]) < 0.0 )
        v20 = bodyAccel[2];
    else
        v20 = maxAccel[2];
    if ( (float)((-(maxAccel[2])) - v19) < 0.0 )
        v9 = v20;
    else
        v9 = -maxAccel[2];
    bodyAccel[2] = v9;
    if ( info->rotRate <= 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_helicopter.cpp",
                    317,
                    0,
                    "%s",
                    "info->rotRate > 0.0f") )
    {
        __debugbreak();
    }
    track[3] = info->rotAccel / info->rotRate;
    tgtYawVel = info->rotRate * controllerFrac[3];
    tgtYawVel = tgtYawVel
                        - (float)((float)(controllerFrac[0] * controllerFrac[1]) * vehHelicopterYawOnLeftStick->current.value);
    if ( (float)(tgtYawVel - info->rotRate) < 0.0 )
        rotRate = tgtYawVel;
    else
        rotRate = info->rotRate;
    if ( (float)((-(info->rotRate)) - tgtYawVel) < 0.0 )
        tgt = rotRate;
    else
        tgt = -info->rotRate;
    tgtYawVel = tgt;
    nextState = DiffTrack(tgt, phys->rotVel[1], track[3], 0.05f);
    rotAccel[1] = (float)(nextState - phys->rotVel[1]) / 0.05f;
    v16 = rotAccel[1];
    if ( (float)(v16 - info->accel) < 0.0 )
        accel = rotAccel[1];
    else
        accel = info->accel;
    if ( (float)((-(info->accel)) - v16) < 0.0 )
        v7 = accel;
    else
        v7 = -info->accel;
    rotAccel[1] = v7;
    for ( axis = 0; axis < 2; ++axis )
    {
        if ( maxSpeed[axis] <= 0.0 )
        {
            if ( info->maxSpeed <= 0.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_helicopter.cpp",
                            335,
                            0,
                            "%s",
                            "info->maxSpeed > 0.0f") )
            {
                __debugbreak();
            }
            maxSpeed[axis] = info->maxSpeed;
        }
        if ( maxAccel[axis] <= 0.0 )
        {
            if ( info->accel <= 0.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_helicopter.cpp",
                            340,
                            0,
                            "%s",
                            "info->accel > 0.0f") )
            {
                __debugbreak();
            }
            maxAccel[axis] = info->accel;
        }
    }
    yawAngles[0] = 0.0f;
    yawAngles[1] = phys->angles[1];
    yawAngles[2] = 0.0f;
    AngleVectors(yawAngles, bodyMat[0], bodyMat[1], bodyMat[2]);
    memset(bodyMat[3], 0, sizeof(float[3]));
    targetTilt[0] = (float)(phys->bodyVel[0] / maxSpeed[0]) * info->tiltFromVelocity[0];
    targetTilt[0] = (float)((float)(tgtVel[0] / maxSpeed[0]) * vehHelicopterTiltFromControllerAxes->current.value)
                                + targetTilt[0];
    targetTilt[0] = (float)((float)(newAccel[0] / maxAccel[0]) * info->tiltFromAcceleration[0]) + targetTilt[0];
    targetTilt[0] = (float)((float)(newDecel[0] / maxAccel[0]) * info->tiltFromDeceleration[0]) + targetTilt[0];
    if ( (float)(targetTilt[0] - 1.0) < 0.0 )
        v15 = targetTilt[0];
    else
        v15 = 1.0f;
    if ( (float)(-1.0 - targetTilt[0]) < 0.0 )
        v6 = v15;
    else
        v6 = -1.0f;
    targetTilt[0] = v6;
    viewAnglesTargetPitch = AngleNormalize180(*viewangles - info->viewPitchOffset)
                                                * vehHelicopterTiltFromViewangles->current.value
                                                / 90.0;
    deltaFracToApply = 0.65f;
    if ( player_topDownCamMode->current.integer > 0 )
        deltaFracToApply = 0.3f;
    targetTilt[0] = (float)((float)(viewAnglesTargetPitch - targetTilt[0]) * deltaFracToApply) + targetTilt[0];
    targetTilt[1] = (float)(phys->bodyVel[1] / maxSpeed[1]) * info->tiltFromVelocity[1];
    targetTilt[1] = (float)((float)(tgtVel[1] / maxSpeed[1]) * vehHelicopterTiltFromControllerAxes->current.value)
                                + targetTilt[1];
    targetTilt[1] = (float)((float)(newAccel[1] / maxAccel[1]) * info->tiltFromAcceleration[1]) + targetTilt[1];
    targetTilt[1] = (float)((float)(newDecel[1] / maxAccel[1]) * info->tiltFromDeceleration[1]) + targetTilt[1];
    if ( phys->bodyVel[0] > 0.0 && fabs(phys->rotVel[1]) > 0.0 )
    {
        if ( maxSpeed[0] <= 0.0
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_helicopter.cpp", 379, 0, "%s", "maxSpeed[0] > 0") )
        {
            __debugbreak();
        }
        velScale = phys->bodyVel[0] / maxSpeed[0];
        if ( vehHelicopterTiltFromFwdAndYaw_VelAtMaxTilt->current.value <= velScale )
        {
            velScalea = 1.0f;
        }
        else
        {
            if ( vehHelicopterTiltFromFwdAndYaw_VelAtMaxTilt->current.value <= 0.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_helicopter.cpp",
                            383,
                            1,
                            "%s",
                            "vehHelicopterTiltFromFwdAndYaw_VelAtMaxTilt->current.value > 0") )
            {
                __debugbreak();
            }
            velScalea = velScale / vehHelicopterTiltFromFwdAndYaw_VelAtMaxTilt->current.value;
        }
        if ( info->rotRate <= 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_helicopter.cpp",
                        389,
                        0,
                        "%s",
                        "info->rotRate > 0.0f") )
        {
            __debugbreak();
        }
        targetTilt[1] = (float)((float)(vehHelicopterTiltFromFwdAndYaw->current.value * velScalea)
                                                    * (float)((-(phys->rotVel[1])) / info->rotRate))
                                    + targetTilt[1];
    }
    if ( (float)(targetTilt[1] - 1.0) < 0.0 )
        v14 = targetTilt[1];
    else
        v14 = 1.0f;
    if ( (float)(-1.0 - targetTilt[1]) < 0.0 )
        v5 = v14;
    else
        v5 = -1.0f;
    targetTilt[1] = v5;
    targetTilt[2] = 0.0f;
    MatrixTransformVector(targetTilt, bodyMat, worldTargetTilt);
    deltaTilt[0] = worldTargetTilt[0] - phys->worldTilt[0];
    deltaTilt[1] = worldTargetTilt[1] - phys->worldTilt[1];
    if ( vehHelicopterTiltMomentum->current.value == 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_helicopter.cpp",
                    401,
                    0,
                    "%s",
                    "vehHelicopterTiltMomentum->current.value") )
    {
        __debugbreak();
    }
    tiltAccel[0] = (float)(info->tiltSpeed[0] / vehHelicopterTiltMomentum->current.value) * deltaTilt[0];
    tiltAccel[1] = (float)(info->tiltSpeed[1] / vehHelicopterTiltMomentum->current.value) * deltaTilt[1];
    v13 = -1.0 / vehHelicopterTiltMomentum->current.value;
    tiltAccel[0] = (float)(v13 * phys->worldTiltVel[0]) + tiltAccel[0];
    tiltAccel[1] = (float)(v13 * phys->worldTiltVel[1]) + tiltAccel[1];
    oldTiltVel[0] = phys->worldTiltVel[0];
    oldTiltVel[1] = phys->worldTiltVel[1];
    phys->worldTiltVel[0] = (float)(0.05f * tiltAccel[0]) + phys->worldTiltVel[0];
    phys->worldTiltVel[1] = (float)(0.05f * tiltAccel[1]) + phys->worldTiltVel[1];
    oldTiltVel[0] = (float)(oldTiltVel[0] + phys->worldTiltVel[0]) * 0.5;
    oldTiltVel[1] = (float)(oldTiltVel[1] + phys->worldTiltVel[1]) * 0.5;
    phys->worldTilt[0] = (float)(0.05f * oldTiltVel[0]) + phys->worldTilt[0];
    phys->worldTilt[1] = (float)(0.05f * oldTiltVel[1]) + phys->worldTilt[1];
    MatrixTransposeTransformVector43(phys->worldTilt, bodyMat, targetTilt);
    *rotAccel = targetTilt[0] * info->maxBodyPitch;
    rotAccel[2] = targetTilt[1] * info->maxBodyRoll;
    speedSq = (float)(phys->bodyVel[0] * phys->bodyVel[0]) + (float)(phys->bodyVel[1] * phys->bodyVel[1]);
    if ( (float)(vehHelicopterHoverSpeedThreshold->current.value * vehHelicopterHoverSpeedThreshold->current.value) > speedSq )
    {
        if ( vehHelicopterHoverSpeedThreshold->current.value <= 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_helicopter.cpp",
                        438,
                        0,
                        "%s",
                        "vehHelicopterHoverSpeedThreshold->current.value > 0") )
        {
            __debugbreak();
        }
        frac = (float)(vehHelicopterHoverSpeedThreshold->current.value - sqrtf(speedSq))
                 / vehHelicopterHoverSpeedThreshold->current.value;
        HELI_UpdateJitter(&veh->jitter);
        *rotAccel = (float)(frac * veh->jitter.jitterPos[0]) + *rotAccel;
        rotAccel[2] = (float)(frac * veh->jitter.jitterPos[2]) + rotAccel[2];
    }
    nanassertvec3(bodyAccel);
    nanassertvec3(rotAccel);
}

void __cdecl HELI_CmdScale(char *move, float *outFracs)
{
    int max; // [esp+24h] [ebp-18h]
    float total; // [esp+28h] [ebp-14h]
    float absAxis; // [esp+2Ch] [ebp-10h]
    float absAxis_4; // [esp+30h] [ebp-Ch]
    int axis; // [esp+38h] [ebp-4h]

    if ( !move && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_helicopter.cpp", 112, 0, "%s", "move") )
        __debugbreak();
    if ( !outFracs
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_helicopter.cpp", 113, 0, "%s", "outFracs") )
    {
        __debugbreak();
    }
    for ( axis = 0; axis < 4; ++axis )
        outFracs[axis] = (float)move[axis] / 127.0;
    if ( *move || move[1] )
    {
        total = sqrtf((float)(move[1] * move[1] + *move * *move));
        max = abs8(*move);
        if ( abs8(move[1]) > max )
            max = abs8(move[1]);
        if ( max )
        {
            *outFracs = (float)((float)max / total) * *outFracs;
            outFracs[1] = (float)((float)max / total) * outFracs[1];
        }
        if ( vehHelicopterStrafeDeadzone->current.value > fabs((unsigned int)outFracs[1]) )
            outFracs[1] = 0.0f;
        if ( vehHelicopterScaleMovement->current.enabled )
        {
            (absAxis) =   fabs(outFracs[0]);
            (absAxis_4) = fabs(outFracs[1]);
            if ( absAxis > 1.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_helicopter.cpp",
                            142,
                            0,
                            "%s",
                            "absAxis[0] <= 1.0f") )
            {
                __debugbreak();
            }
            if ( absAxis_4 > 1.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_helicopter.cpp",
                            143,
                            0,
                            "%s",
                            "absAxis[1] <= 1.0f") )
            {
                __debugbreak();
            }
            if ( absAxis <= absAxis_4 )
                *outFracs = *outFracs * (float)(1.0 - (float)(absAxis_4 - absAxis));
            else
                outFracs[1] = outFracs[1] * (float)(1.0 - (float)(absAxis - absAxis_4));
        }
    }
    if ( vehHelicopterRightStickDeadzone->current.value > fabs((unsigned int)outFracs[2]) )
        outFracs[2] = 0.0f;
    if ( !vehHelicopterAlwaysFaceCamera->current.enabled
        && vehHelicopterRightStickDeadzone->current.value > fabs((unsigned int)outFracs[3]) )
    {
        outFracs[3] = 0.0f;
    }
}

void __cdecl HELI_UpdateJitter(VehicleJitter *jitter)
{
    double v1; // st7
    double v2; // st7
    float v3; // [esp+20h] [ebp-24h]
    int jitterDelay; // [esp+30h] [ebp-14h]
    float newOffset[3]; // [esp+34h] [ebp-10h]
    int i; // [esp+40h] [ebp-4h]

    if ( jitter->jitterPeriodMin || jitter->jitterPeriodMax )
    {
        if ( level.time > jitter->jitterEndTime )
        {
            if ( (float)jitter->jitterPeriodMin < 0.05f )
                jitter->jitterPeriodMin = 50;
            jitterDelay = G_irand(jitter->jitterPeriodMin, jitter->jitterPeriodMax);
            jitter->jitterEndTime = jitterDelay + level.time;
            for ( i = 0; i < 3; ++i )
            {
                if ( jitter->jitterOffsetRange[i] != 0.0 )
                {
                    v1 = G_flrand((-(jitter->jitterOffsetRange[i])), jitter->jitterOffsetRange[i]);
                    newOffset[i] = v1;
                }
            }
            jitter->jitterDeltaAccel[0] = newOffset[0] - jitter->jitterAccel[0];
            jitter->jitterDeltaAccel[1] = newOffset[1] - jitter->jitterAccel[1];
            jitter->jitterDeltaAccel[2] = newOffset[2] - jitter->jitterAccel[2];
            if ( jitterDelay <= 0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_helicopter.cpp",
                            190,
                            0,
                            "%s",
                            "jitterDelay > 0") )
            {
                __debugbreak();
            }
            v3 = 50.000004 / (float)jitterDelay;
            jitter->jitterDeltaAccel[0] = v3 * jitter->jitterDeltaAccel[0];
            jitter->jitterDeltaAccel[1] = v3 * jitter->jitterDeltaAccel[1];
            jitter->jitterDeltaAccel[2] = v3 * jitter->jitterDeltaAccel[2];
        }
        jitter->jitterAccel[0] = jitter->jitterAccel[0] + jitter->jitterDeltaAccel[0];
        jitter->jitterAccel[1] = jitter->jitterAccel[1] + jitter->jitterDeltaAccel[1];
        jitter->jitterAccel[2] = jitter->jitterAccel[2] + jitter->jitterDeltaAccel[2];
        for ( i = 0; i < 3; ++i )
        {
            v2 = DiffTrack(
                         jitter->jitterAccel[i],
                         jitter->jitterPos[i],
                         vehHelicopterJitterJerkyness->current.value,
                         0.05f);
            jitter->jitterPos[i] = v2;
        }
    }
}

void __cdecl HELI_SoftenCollisions(gentity_s *ent, float *worldAccel)
{
    float v2; // [esp+4h] [ebp-7Ch]
    float v3; // [esp+18h] [ebp-68h]
    bool clipped; // [esp+24h] [ebp-5Ch]
    float errorAccel; // [esp+2Ch] [ebp-54h]
    scr_vehicle_s *veh; // [esp+30h] [ebp-50h]
    float clippedPos[3]; // [esp+34h] [ebp-4Ch] BYREF
    float velChange[3]; // [esp+40h] [ebp-40h]
    float error[3]; // [esp+4Ch] [ebp-34h]
    float wishVel[3]; // [esp+58h] [ebp-28h]
    float targetPos[3]; // [esp+64h] [ebp-1Ch]
    float errorMagSqr; // [esp+70h] [ebp-10h]
    float oldVel[3]; // [esp+74h] [ebp-Ch]

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_helicopter.cpp", 522, 0, "%s", "ent") )
        __debugbreak();
    if ( !ent->scr_vehicle
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_helicopter.cpp", 523, 0, "%s", "ent->scr_vehicle") )
    {
        __debugbreak();
    }
    veh = ent->scr_vehicle;
    if ( veh->phys.vel[0] != 0.0 || veh->phys.vel[1] != 0.0 || veh->phys.vel[2] != 0.0 )
    {
        oldVel[0] = veh->phys.vel[0];
        oldVel[1] = veh->phys.vel[1];
        oldVel[2] = veh->phys.vel[2];
        v3 = vehHelicopterLookaheadTime->current.value / 0.05f;
        veh->phys.vel[0] = v3 * veh->phys.vel[0];
        veh->phys.vel[1] = v3 * veh->phys.vel[1];
        veh->phys.vel[2] = v3 * veh->phys.vel[2];
        targetPos[0] = (float)(0.05f * veh->phys.vel[0]) + veh->phys.origin[0];
        targetPos[1] = (float)(0.05f * veh->phys.vel[1]) + veh->phys.origin[1];
        targetPos[2] = (float)(0.05f * veh->phys.vel[2]) + veh->phys.origin[2];
        clipped = VEH_TestSlideMove(ent, clippedPos);
        veh->phys.vel[0] = oldVel[0];
        veh->phys.vel[1] = oldVel[1];
        veh->phys.vel[2] = oldVel[2];
        if ( clipped )
        {
            error[0] = targetPos[0] - clippedPos[0];
            error[1] = targetPos[1] - clippedPos[1];
            error[2] = targetPos[2] - clippedPos[2];
            errorMagSqr = (float)((float)((float)(targetPos[0] - clippedPos[0]) * (float)(targetPos[0] - clippedPos[0]))
                                                    + (float)((float)(targetPos[1] - clippedPos[1]) * (float)(targetPos[1] - clippedPos[1])))
                                    + (float)((float)(targetPos[2] - clippedPos[2]) * (float)(targetPos[2] - clippedPos[2]));
            if ( errorMagSqr >= 1.0 )
            {
                errorAccel = (float)((float)((float)(*worldAccel * error[0]) + (float)(worldAccel[1] * error[1]))
                                                     + (float)(worldAccel[2] * error[2]))
                                     / errorMagSqr;
                if ( errorAccel > 0.0 )
                {
                    worldAccel[0] = (float)((-(errorAccel)) * error[0]) + *worldAccel;
                    worldAccel[1] = (float)((-(errorAccel)) * error[1]) + worldAccel[1];
                    worldAccel[2] = (float)((-(errorAccel)) * error[2]) + worldAccel[2];
                }
                wishVel[0] = clippedPos[0] - veh->phys.origin[0];
                wishVel[1] = clippedPos[1] - veh->phys.origin[1];
                wishVel[2] = clippedPos[2] - veh->phys.origin[2];
                if ( vehHelicopterLookaheadTime->current.value <= 0.0
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_helicopter.cpp",
                                556,
                                0,
                                "%s",
                                "vehHelicopterLookaheadTime->current.value > 0") )
                {
                    __debugbreak();
                }
                v2 = 1.0 / vehHelicopterLookaheadTime->current.value;
                wishVel[0] = v2 * wishVel[0];
                wishVel[1] = v2 * wishVel[1];
                wishVel[2] = v2 * wishVel[2];
                velChange[0] = wishVel[0] - veh->phys.vel[0];
                velChange[1] = wishVel[1] - veh->phys.vel[1];
                velChange[2] = wishVel[2] - veh->phys.vel[2];
                *worldAccel = (float)(20.0 * velChange[0]) + *worldAccel;
                worldAccel[1] = (float)(20.0 * velChange[1]) + worldAccel[1];
                worldAccel[2] = (float)(20.0 * velChange[2]) + worldAccel[2];
            }
        }
    }
}

bool __cdecl VEH_TestSlideMove(gentity_s *ent, float *outPos)
{
    bool result; // eax
    scr_vehicle_s *veh; // [esp+10h] [ebp-1Ch]
    float startOrigin; // [esp+14h] [ebp-18h]
    float startOrigin_4; // [esp+18h] [ebp-14h]
    float startOrigin_8; // [esp+1Ch] [ebp-10h]
    float startVel; // [esp+20h] [ebp-Ch]
    float startVel_4; // [esp+24h] [ebp-8h]
    float startVel_8; // [esp+28h] [ebp-4h]

    veh = ent->scr_vehicle;
    startOrigin = veh->phys.origin[0];
    startOrigin_4 = veh->phys.origin[1];
    startOrigin_8 = veh->phys.origin[2];
    startVel = veh->phys.vel[0];
    startVel_4 = veh->phys.vel[1];
    startVel_8 = veh->phys.vel[2];
    result = VEH_SlideMove(ent, 0);
    *outPos = veh->phys.origin[0];
    outPos[1] = veh->phys.origin[1];
    outPos[2] = veh->phys.origin[2];
    veh->phys.origin[0] = startOrigin;
    veh->phys.origin[1] = startOrigin_4;
    veh->phys.origin[2] = startOrigin_8;
    veh->phys.vel[0] = startVel;
    veh->phys.vel[1] = startVel_4;
    veh->phys.vel[2] = startVel_8;
    return result;
}

double __cdecl Vehicle_GetFakeLift(float *planeForward, float *planeUp)
{
    float dotOffUp; // [esp+Ch] [ebp-4h]

    dotOffUp = (float)((float)(*planeUp * 0.0) + (float)(planeUp[1] * 0.0)) + (float)(planeUp[2] * 1.0);
    if ( dotOffUp < 0.0 )
        dotOffUp = 0.0f;
    return -vehPlaneFakeLiftForce->current.value * (1.0 - dotOffUp);
}

double __cdecl Vehicle_AdjustRollForInputDeadZone(float rollPercent)
{
    if ( (vehPlaneRollDeadZone->current.value < 0.0 || vehPlaneRollDeadZone->current.value >= 128.0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_helicopter.cpp",
                    981,
                    0,
                    "%s",
                    "vehPlaneRollDeadZone->current.value >= 0.0f && vehPlaneRollDeadZone->current.value < 128.0f") )
    {
        __debugbreak();
    }
    if ( vehPlaneRollDeadZone->current.value > rollPercent && rollPercent > -(vehPlaneRollDeadZone->current.value) )
    {
        return 0.0;
    }
    if ( rollPercent > vehPlaneRollDeadZone->current.value )
        return (rollPercent - vehPlaneRollDeadZone->current.value) / (1.0 - vehPlaneRollDeadZone->current.value);
    if ( (-vehPlaneRollDeadZone->current.value) <= rollPercent )
        return 0.0;
    return (rollPercent + vehPlaneRollDeadZone->current.value) / (1.0 - vehPlaneRollDeadZone->current.value);
}

void __cdecl Vehicle_AddRotate(vehicle_physic_t *phys, float fRoll, float fPitch, float fYaw)
{
    long double v4; // st7
    long double v5; // st7
    long double v6; // st7
    int n; // [esp+38h] [ebp-8Ch]
    int m; // [esp+3Ch] [ebp-88h]
    int k; // [esp+40h] [ebp-84h]
    int j; // [esp+44h] [ebp-80h]
    int i; // [esp+48h] [ebp-7Ch]
    float axisOut[3][3]; // [esp+4Ch] [ebp-78h] BYREF
    float desiredAngle[3]; // [esp+70h] [ebp-54h] BYREF
    float axisIn[3][3]; // [esp+7Ch] [ebp-48h] BYREF
    float actualAng[3]; // [esp+A0h] [ebp-24h]
    float fCos; // [esp+ACh] [ebp-18h]
    int iAxis; // [esp+B0h] [ebp-14h]
    float actualAngScaled[3]; // [esp+B4h] [ebp-10h]
    float fSin; // [esp+C0h] [ebp-4h]

    for ( i = 0; i < 3; ++i )
    {
        for ( j = 0; j < 3; ++j )
            axisOut[i][j] = 0.0f;
    }
    AnglesToAxis(phys->angles, axisIn);
    v4 = (float)(fRoll * 0.017453292);
    fCos = cos(v4);
    fSin = sin(v4);
    iAxis = 0;
    for ( k = 0; k < 3; ++k )
    {
        axisOut[iAxis][k] = axisOut[iAxis][k] + axisIn[iAxis][k];
        axisOut[(iAxis + 1) % 3][k] = (float)((float)(axisIn[(iAxis + 1) % 3][k] * fCos)
                                                                                + (float)(axisIn[(iAxis + 2) % 3][k] * fSin))
                                                                + axisOut[(iAxis + 1) % 3][k];
        axisOut[(iAxis + 2) % 3][k] = (float)((float)(axisIn[(iAxis + 2) % 3][k] * fCos)
                                                                                - (float)(axisIn[(iAxis + 1) % 3][k] * fSin))
                                                                + axisOut[(iAxis + 2) % 3][k];
    }
    iAxis = 1;
    v5 = (float)(fPitch * 0.017453292);
    fCos = cos(v5);
    fSin = sin(v5);
    for ( m = 0; m < 3; ++m )
    {
        axisOut[iAxis][m] = axisOut[iAxis][m] + axisIn[iAxis][m];
        axisOut[(iAxis + 1) % 3][m] = (float)((float)(axisIn[(iAxis + 1) % 3][m] * fCos)
                                                                                + (float)(axisIn[(iAxis + 2) % 3][m] * fSin))
                                                                + axisOut[(iAxis + 1) % 3][m];
        axisOut[(iAxis + 2) % 3][m] = (float)((float)(axisIn[(iAxis + 2) % 3][m] * fCos)
                                                                                - (float)(axisIn[(iAxis + 1) % 3][m] * fSin))
                                                                + axisOut[(iAxis + 2) % 3][m];
    }
    iAxis = 2;
    v6 = (float)(fYaw * 0.017453292);
    fCos = cos(v6);
    fSin = sin(v6);
    for ( n = 0; n < 3; ++n )
    {
        axisOut[iAxis][n] = axisOut[iAxis][n] + axisIn[iAxis][n];
        axisOut[(iAxis + 1) % 3][n] = (float)((float)(axisIn[(iAxis + 1) % 3][n] * fCos)
                                                                                + (float)(axisIn[(iAxis + 2) % 3][n] * fSin))
                                                                + axisOut[(iAxis + 1) % 3][n];
        axisOut[(iAxis + 2) % 3][n] = (float)((float)(axisIn[(iAxis + 2) % 3][n] * fCos)
                                                                                - (float)(axisIn[(iAxis + 1) % 3][n] * fSin))
                                                                + axisOut[(iAxis + 2) % 3][n];
    }
    AxisToAngles(axisOut, desiredAngle);
    actualAng[0] = desiredAngle[0] - phys->angles[0];
    actualAng[1] = desiredAngle[1] - phys->angles[1];
    actualAng[2] = desiredAngle[2] - phys->angles[2];
    actualAngScaled[0] = aircraftRotLag * actualAng[0];
    actualAngScaled[1] = aircraftRotLag * actualAng[1];
    actualAngScaled[2] = aircraftRotLag * actualAng[2];
    actualAng[0] = phys->angles[0] + (float)(aircraftRotLag * actualAng[0]);
    actualAng[1] = phys->angles[1] + (float)(aircraftRotLag * actualAng[1]);
    actualAng[2] = phys->angles[2] + (float)(aircraftRotLag * actualAng[2]);
    phys->angles[0] = actualAng[0];
    phys->angles[1] = actualAng[1];
    phys->angles[2] = actualAng[2];
}

void __cdecl VEH_UpdateClientPlane(gentity_s *ent)
{
    double v1; // st7
    int integer; // [esp+10h] [ebp-128h]
    float actualPitch; // [esp+24h] [ebp-114h]
    float actualPitcha; // [esp+24h] [ebp-114h]
    float target_axis[3][3]; // [esp+28h] [ebp-110h] BYREF
    float gravityVec[3]; // [esp+4Ch] [ebp-ECh]
    float end[3]; // [esp+58h] [ebp-E0h] BYREF
    float desiredPitch; // [esp+64h] [ebp-D4h]
    float actualYaw; // [esp+68h] [ebp-D0h]
    float zerovec[3]; // [esp+6Ch] [ebp-CCh] BYREF
    float desiredRoll; // [esp+78h] [ebp-C0h]
    float desiredYaw; // [esp+7Ch] [ebp-BCh]
    float up[3]; // [esp+80h] [ebp-B8h]
    float dotOffUp; // [esp+8Ch] [ebp-ACh]
    float upVec[3]; // [esp+90h] [ebp-A8h]
    float actualRoll; // [esp+9Ch] [ebp-9Ch]
    float desiredDelta; // [esp+A0h] [ebp-98h]
    float speedStick; // [esp+A4h] [ebp-94h]
    vehicle_physic_t *phys; // [esp+A8h] [ebp-90h]
    const vehicle_info_t *info; // [esp+ACh] [ebp-8Ch]
    float yawStick; // [esp+B0h] [ebp-88h]
    scr_vehicle_s *veh; // [esp+B4h] [ebp-84h]
    char move[4]; // [esp+B8h] [ebp-80h]
    usercmd_s *usercmd; // [esp+BCh] [ebp-7Ch]
    float pitchStick; // [esp+C0h] [ebp-78h]
    trace_t trace; // [esp+C4h] [ebp-74h] BYREF
    float axis[4][3]; // [esp+100h] [ebp-38h] BYREF
    gentity_s *player; // [esp+130h] [ebp-8h]
    float rollStick; // [esp+134h] [ebp-4h]

    move[0] = 0;
    move[1] = 0;
    move[2] = 0;
    move[3] = 0;
    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_helicopter.cpp", 1081, 0, "%s", "ent") )
        __debugbreak();
    if ( !ent->scr_vehicle
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_helicopter.cpp", 1082, 0, "%s", "ent->scr_vehicle") )
    {
        __debugbreak();
    }
    veh = ent->scr_vehicle;
    phys = &veh->phys;
    info = BG_GetVehicleInfo(veh->infoIdx);
    rollStick = 0.0f;
    pitchStick = 0.0f;
    yawStick = 0.0f;
    speedStick = 0.0f;
    //if ( EntHandle::isDefined(&ent->r.ownerNum) )
    if ( ent->r.ownerNum.isDefined() )
    {
        //player = EntHandle::ent(&ent->r.ownerNum);
        player = ent->r.ownerNum.ent();
        if ( !player->client
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_helicopter.cpp", 1097, 0, "%s", "player->client") )
        {
            __debugbreak();
        }
        player->client->linkAnglesFrac = 0.0f;
        usercmd = &player->client->sess.cmd;
        if ( (player->client->ps.pm_flags & 0xC00) == 0 )
        {
            move[0] = usercmd->forwardmove;
            move[1] = usercmd->rightmove;
            //if ( bitarray<51>::testBit(&usercmd->button_bits, 0xBu) )
            if ( usercmd->button_bits.testBit(0xBu) )
            {
                player->client->linkAnglesFrac = 1.0f;
                player->client->linkAnglesMinClamp[1] = -info->turretViewLimits.horizSpanRight;
                player->client->linkAnglesMaxClamp[1] = info->turretViewLimits.horizSpanLeft;
                player->client->linkAnglesMinClamp[0] = -info->turretViewLimits.vertSpanUp;
                player->client->linkAnglesMaxClamp[0] = info->turretViewLimits.vertSpanDown;
            }
            else
            {
                integer = vehHelicopterYawAltitudeControls->current.integer;
                if ( integer )
                {
                    if ( integer == 1 )
                    {
                        move[2] = usercmd->pitchmove;
                        //if ( bitarray<51>::testBit(&usercmd->button_bits, 0xFu) )
                        if ( usercmd->button_bits.testBit(0xFu) )
                            move[3] = 127;
                        //if ( bitarray<51>::testBit(&usercmd->button_bits, 0xEu) )
                        if ( usercmd->button_bits.testBit(0xEu) )
                            move[3] -= 127;
                    }
                    else if ( integer == 2 )
                    {
                        move[3] = usercmd->yawmove;
                        //if ( bitarray<51>::testBit(&usercmd->button_bits, 0xEu) )
                        if ( usercmd->button_bits.testBit(0xEu) )
                            move[2] = 127;
                        //if ( bitarray<51>::testBit(&usercmd->button_bits, 0xFu) )
                        if ( usercmd->button_bits.testBit(0xFu) )
                            move[2] -= 127;
                    }
                }
                else
                {
                    //if ( bitarray<51>::testBit(&usercmd->button_bits, 0xFu) )
                    if ( usercmd->button_bits.testBit(0xFu) )
                        rollStick = rollStick - 1.0;
                    //if ( bitarray<51>::testBit(&usercmd->button_bits, 0xEu) )
                    if ( usercmd->button_bits.testBit(0xEu) )
                        rollStick = 1.0f;
                    //yawStick = COERCE_FLOAT(COERCE_UNSIGNED_INT((float)usercmd->rightmove) ^ _mask__NegFloat_) / 128.0;
                    yawStick = (-((float)usercmd->rightmove)) / 128.0;
                    pitchStick = (float)usercmd->forwardmove / 128.0;
                    //speedStick = COERCE_FLOAT(COERCE_UNSIGNED_INT((float)usercmd->pitchmove) ^ _mask__NegFloat_) / 128.0;
                    speedStick = (-((float)usercmd->pitchmove)) / 128.0;
                    rollStick = Vehicle_AdjustRollForInputDeadZone((float)usercmd->rightmove / 128.0);
                }
                if ( vehHelicopterInvertUpDown->current.enabled )
                    move[2] = -move[2];
            }
        }
        AnglesToAxis(phys->angles, target_axis);
        desiredRoll = (float)(info->maxBodyRoll * 0.05f) * rollStick;
        desiredPitch = (float)(info->maxBodyPitch * 0.05f) * pitchStick;
        desiredYaw = (float)(info->rotRate * 0.05f) * yawStick;
        actualRoll = DiffTrack(desiredRoll, phys->rotVel[2], vehPlaneRollAccel->current.value, 0.05f);
        actualYaw = DiffTrack(desiredYaw, phys->rotVel[1], info->rotRate, 0.05f);
        actualYaw = actualYaw - (float)(actualRoll * vehPlaneYawFromRollScale->current.value);
        up[0] = 0.0f;
        up[1] = 0.0f;
        up[2] = 1.0f;
        dotOffUp = (float)((float)(target_axis[2][0] * 0.0) + (float)(target_axis[2][1] * 0.0))
                         + (float)(target_axis[2][2] * 1.0);
        if ( dotOffUp < 0.0 )
            dotOffUp = 0.0f;
        actualPitch = DiffTrack(desiredPitch, phys->rotVel[0], vehPlanePitchAccel->current.value, 0.05f);
        actualPitcha = Vehicle_GetFakeLift(target_axis[0], target_axis[2]) + actualPitch;
        Vehicle_AddRotate(phys, actualRoll, actualPitcha, actualYaw);
        desiredDelta = (float)(0.05f * speedStick) * info->accel;
        phys->bodyVel[0] = phys->bodyVel[0] + desiredDelta;
        if ( vehPlaneLowSpeed->current.value > phys->bodyVel[0] )
            phys->bodyVel[0] = vehPlaneLowSpeed->current.value;
        if ( phys->bodyVel[0] > info->maxSpeed )
            phys->bodyVel[0] = info->maxSpeed;
        AnglesToAxis(phys->angles, axis);
        phys->vel[0] = target_axis[0][0] * phys->bodyVel[0];
        phys->vel[1] = target_axis[0][1] * phys->bodyVel[0];
        phys->vel[2] = target_axis[0][2] * phys->bodyVel[0];
        end[0] = (float)(0.5 * phys->vel[0]) + phys->origin[0];
        end[1] = (float)(0.5 * phys->vel[1]) + phys->origin[1];
        end[2] = (float)(0.5 * phys->vel[2]) + phys->origin[2];
        end[0] = (float)(0.05f * phys->vel[0]) + phys->origin[0];
        end[1] = (float)(0.05f * phys->vel[1]) + phys->origin[1];
        end[2] = (float)(0.05f * phys->vel[2]) + phys->origin[2];
        gravityVec[0] = 0.0f;
        gravityVec[1] = 0.0f;
        gravityVec[2] = -1.0f;
        gravityVec[0] = 0.0 * vehPlaneGravityForce->current.value;
        gravityVec[1] = 0.0 * vehPlaneGravityForce->current.value;
        gravityVec[2] = -1.0 * vehPlaneGravityForce->current.value;
        end[0] = (float)(0.05f * gravityVec[0]) + end[0];
        end[1] = (float)(0.05f * gravityVec[1]) + end[1];
        end[2] = (float)(0.05f * gravityVec[2]) + end[2];
        upVec[0] = target_axis[2][0] * vehPlaneLiftForce->current.value;
        upVec[1] = target_axis[2][1] * vehPlaneLiftForce->current.value;
        upVec[2] = target_axis[2][2] * vehPlaneLiftForce->current.value;
        end[0] = (float)(0.05f * upVec[0]) + end[0];
        end[1] = (float)(0.05f * upVec[1]) + end[1];
        end[2] = (float)(0.05f * upVec[2]) + end[2];
        memset(zerovec, 0, sizeof(zerovec));
        VEH_CheckForCrash(ent, end);
        phys->origin[0] = end[0];
        phys->origin[1] = end[1];
        phys->origin[2] = end[2];
        v1 = Vec3Length(phys->bodyVel);
        veh->speed = v1;
        if ( veh->speed < 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_helicopter.cpp",
                        1301,
                        0,
                        "%s",
                        "veh->speed >= 0.0f") )
        {
            __debugbreak();
        }
    }
}

