#include "g_scr_vehicle.h"
#include <universal/q_shared.h>
#include <clientscript/cscr_vm.h>
#include <clientscript/scr_const.h>
#include <game_mp/g_main_mp.h>
#include "g_weapon.h"
#include <game_mp/g_utils_mp.h>
#include <server_mp/sv_main_mp.h>
#include <bgame/bg_misc.h>
#include <clientscript/cscr_stringlist.h>
#include <server/sv_world.h>
#include <server/sv_game.h>
#include <universal/com_math_anglevectors.h>
#include <qcommon/dobj_management.h>
#include <xanim/dobj_utils.h>
#include <xanim/xmodel_utils.h>
#include "bullet.h"
#include <client/cl_debugdata.h>
#include <game_mp/g_misc_mp.h>
#include <client_mp/g_client_mp.h>
#include <game_mp/g_spawn_mp.h>
#include <universal/com_loadutils.h>
#include <bgame/bg_misctables.h>
#include <server_mp/sv_init_mp.h>
#include <universal/CurveManager.h>
#include <physics/phys_assert.h>
#include <cgame/cg_drawtools.h>
#include "g_debug.h"
#include <game_mp/g_active_mp.h>
#include "actor_script_cmd.h"
#include <qcommon/cm_world.h>
#include <glass/glass_server.h>
#include <universal/surfaceflags.h>
#include "g_helicopter1.h"
#include "g_mover.h"
#include "g_targets.h"
#include <game_mp/g_combat_mp.h>
#include "g_load_utils.h"
#include <cgame/cg_scr_main.h>
#include <sound/snd_bank.h>


struct VehiclePhysicsBackup // sizeof=0x204
{                                       // XREF: .data:s_backup/r
    vehicle_pathpos_t pathPos;
    vehicle_physic_t phys;              // XREF: VEH_BackupPosition+F3/o
};
VehiclePhysicsBackup s_backup;

void __cdecl METHODS_NULLSUB(scr_entref_t entref)
{

}

// Looks congruent to retail blops mp latest
const BuiltinMethodDef s_methods_0[89] =
{
  { "attachpath", CMD_VEH_AttachPath, 0 },
  { "vehgetmodel", CMD_VEH_GetModel, 0 },
  { "getattachpos", CMD_VEH_GetAttachPos, 0 },
  { "startpath", CMD_VEH_StartPath, 0 },
  { "drivepath", CMD_VEH_DrivePath, 0 },
  { "setdrivepathphysicsscale", CMD_VEH_SetDrivePathPhysicsScale, 0 },
  { "setswitchnode", CMD_VEH_SetSwitchNode, 0 },
  { "setwaitnode", CMD_VEH_SetWaitNode, 0 },
  { "setwaitspeed", CMD_VEH_SetWaitSpeed, 0 },
  { "setspeed", CMD_VEH_SetSpeed, 0 },
  { "setspeedimmediate", CMD_VEH_SetSpeedImmediate, 0 },
  { "getspeed", CMD_VEH_GetSpeed, 0 },
  { "getspeedmph", CMD_VEH_GetSpeedMPH, 0 },
  { "getgoalspeedmph", CMD_VEH_GetGoalSpeedMPH, 0 },
  { "setbrake", CMD_VEH_SetBrake, 0 },
  { "setacceleration", CMD_VEH_SetAcceleration, 0 },
  { "setdeceleration", CMD_VEH_SetDeceleration, 0 },
  { "resumespeed", CMD_VEH_ResumeSpeed, 0 },
  { "setyawspeed", CMD_VEH_SetYawSpeed, 0 },
  { "setmaxpitchroll", CMD_VEH_SetMaxPitchRoll, 0 },
  { "setturningability", CMD_VEH_SetTurningAbility, 0 },
  { "setjitterparams", CMD_VEH_SetJitterParams, 0 },
  { "setairresistance", CMD_VEH_SetAirResitance, 0 },
  { "getsteerfactor", CMD_VEH_GetSteerFactor, 0 },
  { "getthrottle", CMD_VEH_GetThrottle, 0 },
  { "sethoverparams", CMD_VEH_SetHoverParams, 0 },
  { "joltbody", CMD_VEH_JoltBody, 0 },
  { "freevehicle", CMD_VEH_FreeVehicle, 0 },
  { "getwheelsurface", CMD_VEH_GetWheelSurface, 0 },
  { "getvehicleowner", CMD_VEH_GetVehicleOwner, 0 },
  { "startenginesound", METHODS_NULLSUB, 0 },
  { "stopenginesound", METHODS_NULLSUB, 0 },
  { "isvehicleusable", CMD_VEH_IsVehicleUsable, 0 },
  { "makevehicleusable", CMD_VEH_MakeVehicleUsable, 0 },
  { "makevehicleunusable", CMD_VEH_MakeVehicleUnusable, 0 },
  { "usevehicle", CMD_VEH_UseVehicle, 0 },
  { "setviewclamp", CMD_VEH_SetViewClamp, 0 },
  { "resetviewclamp", CMD_VEH_ResetViewClamp, 0 },
  { "setvehiclelookattext", CMD_VEH_SetVehicleLookatText, 0 },
  { "setneargoalnotifydist", CMD_VEH_NearGoalNotifyDist, 0 },
  { "setvehgoalpos", CMD_VEH_SetGoalPos, 0 },
  { "clearvehgoalpos", CMD_VEH_ClearGoalPos, 0 },
  { "setplanegoalpos", CMD_VEH_SetPlaneGoalPos, 0 },
  { "setplanebarrelroll", CMD_VEH_SetPlaneBarrelRoll, 0 },
  { "setgoalyaw", CMD_VEH_SetGoalYaw, 0 },
  { "cleargoalyaw", CMD_VEH_ClearGoalYaw, 0 },
  { "settargetyaw", CMD_VEH_SetTargetYaw, 0 },
  { "cleartargetyaw", CMD_VEH_ClearTargetYaw, 0 },
  { "setlookatent", CMD_VEH_SetLookAtEnt, 0 },
  { "clearlookatent", CMD_VEH_ClearLookAtEnt, 0 },
  { "returnplayercontrol", CMD_VEH_ReturnPlayerControl, 0 },
  { "setturrettargetvec", CMD_VEH_SetTurretTargetVec, 0 },
  { "setturrettargetent", CMD_VEH_SetTurretTargetEnt, 0 },
  { "clearturrettarget", CMD_VEH_ClearTurretTarget, 0 },
  { "setgunnertargetvec", CMD_VEH_SetGunnerTargetVec, 0 },
  { "getgunnertargetvec", CMD_VEH_GetGunnerTargetVec, 0 },
  { "setgunnertargetent", CMD_VEH_SetGunnerTargetEnt, 0 },
  { "cleargunnertarget", CMD_VEH_ClearGunnerTarget, 0 },
  { "setgunnerturretontargetrange", CMD_VEH_SetGunnerTurretOnTargetRange, 0 },
  { "getgunnertargetent", CMD_VEH_GetGunnerTargetEnt, 0 },
  { "getgunneranimpitch", CMD_VEH_GetGunnerAnimPitch, 0 },
  { "getgunneranimyaw", CMD_VEH_GetGunnerAnimYaw, 0 },
  { "setvehweapon", CMD_VEH_SetWeapon, 0 },
  { "fireweapon", CMD_VEH_FireWeapon, 0 },
  { "firegunnerweapon", CMD_VEH_FireGunnerWeapon, 0 },
  { "stopfireweapon", CMD_VEH_StopFireWeapon, 0 },
  { "isturretready", CMD_VEH_IsTurretReady, 0 },
  { "vehforcematerialspeed", CMD_VEH_ForceMaterialSpeed, 0 },
  { "setvehmaxspeed", CMD_VEH_SetMaxSpeed, 0 },
  { "getvehoccupants", CMD_VEH_GetOccupants, 0 },
  { "getseatoccupant", CMD_VEH_GetSeatOccupant, 0 },
  { "getoccupantseat", CMD_VEH_GetOccupantSeat, 0 },
  { "setseatoccupied", CMD_VEH_SetSeatOccupied, 0 },
  { "seatgetweapon", CMD_VEH_SeatGetWeapon, 0 },
  { "getseatfiringorigin", CMD_VEH_GetSeatFiringOrigin, 0 },
  { "getseatfiringangles", CMD_VEH_GetSeatFiringAngles, 0 },
  { "isgunnerfiring", CMD_VEH_IsGunnerFiring, 0 },
  { "disablegunnerfiring", CMD_VEH_DisableGunnerFiring, 0 },
  { "finishvehicledamage", CMD_VEH_finishVehicleDamage, 0 },
  { "finishvehicleradiusdamage", CMD_VEH_finishVehicleRadiusDamage, 0 },
  { "isvehicleimmunetodamage", CMD_VEH_IsVehicleImmuneToDamage, 0 },
  { "setdefaultpitch", CMD_VEH_SetDefaultPitch, 0 },
  { "cleardefaultpitch", CMD_VEH_ClearDefaultPitch, 0 },
  { "getangularvelocity", CMD_VEH_GetAngularVelocity, 0 },
  { "setangularvelocity", CMD_VEH_SetAngularVelocity, 0 },
  { "setvehvelocity", CMD_VEH_SetVehVelocity, 0 },
  { "gettreadhealth", CMD_VEH_GetTreadHealth, 0 },
  { "getboost", CMD_VEH_GetBoost, 0 },
  { "isboosting", CMD_VEH_IsBoosting, 0 }
};

unsigned __int16 *s_wheelTags[6] =
{
    &scr_const.tag_wheel_front_left,
    &scr_const.tag_wheel_front_right,
    &scr_const.tag_wheel_back_left,
    &scr_const.tag_wheel_back_right,
    &scr_const.tag_wheel_middle_left,
    &scr_const.tag_wheel_middle_right,
};

unsigned __int16 *s_flashTags[5] =
{
    &scr_const.tag_flash,
    &scr_const.tag_flash_11,
    &scr_const.tag_flash_2,
    &scr_const.tag_flash_22,
    &scr_const.tag_flash_3,
};

unsigned __int16 *s_gunnerBarrelTags[4] =
{
    &scr_const.tag_gunner_barrel1,
    &scr_const.tag_gunner_barrel2,
    &scr_const.tag_gunner_barrel3,
    &scr_const.tag_gunner_barrel4,
};

unsigned __int16 *s_gunnerTurretTags[4] =
{
    &scr_const.tag_gunner_turret1,
    &scr_const.tag_gunner_turret2,
    &scr_const.tag_gunner_turret3,
    &scr_const.tag_gunner_turret4,
};

unsigned __int16 *s_gunnerFlashTags[8] =
{
    &scr_const.tag_flash_gunner[0],
    &scr_const.tag_flash_gunner[1],
    &scr_const.tag_flash_gunner[2],
    &scr_const.tag_flash_gunner[3],
    &scr_const.tag_flash_gunner[4],
    &scr_const.tag_flash_gunner[5],
    &scr_const.tag_flash_gunner[6],
    &scr_const.tag_flash_gunner[7],
};

// just the above at offset 4-8
//unsigned __int16 *s_gunnerFlashTags2[4] =
//{
//    &scr_const.tag_flash_gunner
//};

const unsigned __int16 *s_seatTags[11] =
{
    &scr_const.tag_driver,
    &scr_const.tag_gunner1,
    &scr_const.tag_gunner2,
    &scr_const.tag_gunner3,
    &scr_const.tag_gunner4,
    &scr_const.tag_passenger1,
    &scr_const.tag_passenger2,
    &scr_const.tag_passenger3,
    &scr_const.tag_passenger4,
    &scr_const.tag_passenger5,
    &scr_const.tag_passenger6,
};

const unsigned __int16 *s_entryPointTags[5] =
{
    &scr_const.tag_enter_left,
    &scr_const.tag_enter_right,
    &scr_const.tag_enter_back,
    &scr_const.tag_enter_gunner3,
    &scr_const.tag_enter_gunner4,
};

const unsigned __int16 *s_entryPointOldTags[5] =
{
    &scr_const.tag_enter_driver,
    &scr_const.tag_enter_gunner1,
    &scr_const.tag_enter_gunner2,
    &scr_const.tag_enter_gunner3,
    &scr_const.tag_enter_gunner4,
};


// *WARNING* One or more selections were skipped as they could not be interpreted as c data


float exitPointOffsetDown = 30.0;
float exitPointOffsetUp = 10.0;



const float s_correctSolidDeltas[26][3] =
{
  { 0.0, 0.0, 1.0 },
  { -1.0, 0.0, 1.0 },
  { 0.0, -1.0, 1.0 },
  { 1.0, 0.0, 1.0 },
  { 0.0, 1.0, 1.0 },
  { -1.0, 0.0, 0.0 },
  { 0.0, -1.0, 0.0 },
  { 1.0, 0.0, 0.0 },
  { 0.0, 1.0, 0.0 },
  { 0.0, 0.0, -1.0 },
  { -1.0, 0.0, -1.0 },
  { 0.0, -1.0, -1.0 },
  { 1.0, 0.0, -1.0 },
  { 0.0, 1.0, -1.0 },
  { -1.0, -1.0, 1.0 },
  { 1.0, -1.0, 1.0 },
  { 1.0, 1.0, 1.0 },
  { -1.0, 1.0, 1.0 },
  { -1.0, -1.0, 0.0 },
  { 1.0, -1.0, 0.0 },
  { 1.0, 1.0, 0.0 },
  { -1.0, 1.0, 0.0 },
  { -1.0, -1.0, -1.0 },
  { 1.0, -1.0, -1.0 },
  { 1.0, 1.0, -1.0 },
  { -1.0, 1.0, -1.0 }
};


scr_vehicle_s s_vehicles[16];
int bg_numVehicleInfos;
vehicle_info_t bg_vehicleInfos[32];

float s_rolls[1];
float s_pitches[1];

struct VehicleLocalPhysics // sizeof=0x40
{                                       // XREF: .data:s_phys/r
    trace_t groundTrace;                // XREF: VEH_SlideMove(gentity_s *,int)+131/r
    int hasGround;                      // XREF: VEH_ClearGround(gentity_s *)+3/w
    int onGround;                       // XREF: VEH_UpdateClient+36D/r
} s_phys;

const vehicle_info_t *__cdecl BG_GetVehicleInfo(int index)
{
    if ( index < 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 529, 0, "%s", "index >= 0") )
    {
        __debugbreak();
    }
    if ( index >= 32
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                    530,
                    0,
                    "%s",
                    "index < MAX_VEHICLE_FILES") )
    {
        __debugbreak();
    }
    return &bg_vehicleInfos[index];
}

gentity_s *__cdecl GScr_GetVehicle(scr_entref_t entref)
{
    if ( !entref.classnum )
        return VEH_GetVehicle(entref.entnum);
    Scr_ObjectError("not an entity", SCRIPTINSTANCE_SERVER);
    return 0;
}

gentity_s *__cdecl VEH_GetVehicle(int entNum)
{
    const char *v1; // eax
    const char *v2; // eax

    if ( entNum >= 1024
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                    589,
                    0,
                    "%s",
                    "entNum < MAX_GENTITIES") )
    {
        __debugbreak();
    }
    if ( g_entities[entNum].classname != scr_const.script_vehicle )
    {
        v1 = va("entity %i is not a script_vehicle\n", entNum);
        Scr_Error(v1, 0);
    }
    if ( !g_entities[entNum].scr_vehicle )
    {
        v2 = va("entity %i doesn't have a script_vehicle\n", entNum);
        Scr_Error(v2, 0);
    }
    return &g_entities[entNum];
}

int __cdecl VEH_IsSeatPresent(gentity_s *ent, int seatIdx, const vehicle_info_t *info)
{
    if ( !info )
        info = BG_GetVehicleInfo(ent->scr_vehicle->infoIdx);
    if ( seatIdx < 1 )
        return info->isDrivable;
    if ( seatIdx < info->numberOfGunners + 1 )
        return info->driverControlledGunPos + 1 != seatIdx;
    return seatIdx >= 5 && seatIdx < info->numberOfSeats - info->numberOfGunners - info->isDrivable + 5;
}

int __cdecl VEH_GetSeatWeaponIndex(gentity_s *const vehicle, int seatIndex)
{
    const vehicle_info_t *info; // [esp+8h] [ebp-4h]

    info = BG_GetVehicleInfo(vehicle->scr_vehicle->infoIdx);
    if ( !VEH_IsSeatPresent(0, seatIndex, info) )
        return 0;
    if ( !seatIndex )
        return G_GetWeaponIndexForName((char*)info->turretWeapon);
    if ( seatIndex - 1 >= info->numberOfGunners )
        return 0;
    else
        return G_GetWeaponIndexForName((char *)&info->turretTag4[64 * seatIndex]);
}

int __cdecl VEH_GetSeatFiringOriginAngles(gentity_s *const vehEnt, int seatIndex, float *origin, float *angles)
{
    int v5; // [esp+0h] [ebp-60h]
    bool v6; // [esp+4h] [ebp-5Ch]
    const VehicleTurret *p_turret; // [esp+8h] [ebp-58h]
    scr_vehicle_s *veh; // [esp+14h] [ebp-4Ch]
    float flashMatrix[4][3]; // [esp+18h] [ebp-48h] BYREF
    float forward[3]; // [esp+48h] [ebp-18h] BYREF
    int flash; // [esp+54h] [ebp-Ch]
    const VehicleTurret *pTurret; // [esp+58h] [ebp-8h]
    int fireBarrel; // [esp+5Ch] [ebp-4h]

    if ( !VEH_GetSeatWeaponIndex(vehEnt, seatIndex) )
        return 0;
    veh = vehEnt->scr_vehicle;
    if ( seatIndex )
        p_turret = (const VehicleTurret *)&veh->jitter.jitterPos[8 * seatIndex + 2];
    else
        p_turret = &veh->turret;
    pTurret = p_turret;
    v6 = (p_turret->flags & 2) != 0 && !pTurret->fireBarrel;
    fireBarrel = v6;
    if ( seatIndex )
    {
        if ( fireBarrel )
            v5 = veh->boneIndex.flash[4 * seatIndex + 4];
        else
            v5 = veh->boneIndex.flash[4 * seatIndex + 3];
        flash = v5;
    }
    else
    {
        flash = veh->boneIndex.flash[fireBarrel];
    }
    if ( flash < 0 )
        return 0;
    G_DObjGetWorldBoneIndexMatrix(vehEnt, flash, flashMatrix);
    //*(_QWORD *)forward = *(_QWORD *)&flashMatrix[0][0];
    forward[0] = flashMatrix[0][0];
    forward[1] = flashMatrix[0][1];
    forward[2] = flashMatrix[0][2];
    vectoangles(forward, angles);
    *origin = flashMatrix[3][0];
    origin[1] = flashMatrix[3][1];
    origin[2] = flashMatrix[3][2];
    return 1;
}

VehicleSeat *__cdecl VEH_GetSeat(scr_vehicle_s *veh, int seatIdx)
{
    if ( !veh && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 731, 0, "%s", "veh") )
        __debugbreak();
    return &veh->seats[seatIdx];
}

VehicleSeat *__cdecl VEH_GetSeat(gentity_s *vehEnt, int seatIdx)
{
    scr_vehicle_s *veh; // [esp+4h] [ebp-4h]

    veh = GetVehicleLogObjectError(vehEnt);
    if ( veh )
        return VEH_GetSeat(veh, seatIdx);
    else
        return 0;
}

scr_vehicle_s *__cdecl GetVehicleLogObjectError(gentity_s *ent)
{
    scr_vehicle_s *veh; // [esp+0h] [ebp-4h]

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 717, 0, "%s", "ent") )
        __debugbreak();
    veh = ent->scr_vehicle;
    if ( !veh )
        Scr_ObjectError("VEH_GetSeat(): Entity not a vehicle", SCRIPTINSTANCE_SERVER);
    return veh;
}

int __cdecl VEH_GetSeatOccupantEntNum(scr_vehicle_s *veh, int seatIdx)
{
    const VehicleSeat *seat; // [esp+0h] [ebp-4h]

    seat = VEH_GetSeat(veh, seatIdx);
    if ( !seat )
        return 1023;
    if ( seat->_occupantEntNum >= 0x400u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                    750,
                    0,
                    "%s",
                    "seat->_occupantEntNum == ENTITYNUM_NONE || ( seat->_occupantEntNum >= 0 && seat->_occupantEntNum < MAX_GENTITIES )") )
    {
        __debugbreak();
    }
    return seat->_occupantEntNum;
}

int __cdecl VEH_GetSeatOccupantEntNum(gentity_s *vehEnt, int seatIdx)
{
    scr_vehicle_s *veh; // [esp+4h] [ebp-4h]

    veh = GetVehicleLogObjectError(vehEnt);
    if ( veh )
        return VEH_GetSeatOccupantEntNum(veh, seatIdx);
    else
        return 1023;
}

int __cdecl VEH_SetSeatOccupantEntNum(scr_vehicle_s *veh, int seatIdx, unsigned int occEntNum)
{
    int oldOccEntNum; // [esp+0h] [ebp-8h]
    VehicleSeat *seat; // [esp+4h] [ebp-4h]

    if ( occEntNum >= 0x400
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                    765,
                    0,
                    "%s",
                    "occEntNum == ENTITYNUM_NONE || ( occEntNum >= 0 && occEntNum < MAX_GENTITIES )") )
    {
        __debugbreak();
    }
    seat = VEH_GetSeat(veh, seatIdx);
    if ( seat )
    {
        oldOccEntNum = seat->_occupantEntNum;
        seat->_occupantEntNum = occEntNum;
        return oldOccEntNum;
    }
    else
    {
        Scr_ObjectError("VEH_SetSeatOccupantEntNum(): Invalid vehicle/seat specification", SCRIPTINSTANCE_SERVER);
        return 1023;
    }
}

gentity_s *__cdecl VEH_GetSeatOccupantEntity(scr_vehicle_s *veh, int seatIdx)
{
    int occEntNum; // [esp+4h] [ebp-4h]

    occEntNum = VEH_GetSeatOccupantEntNum(veh, seatIdx);
    if ( occEntNum == 1023 || occEntNum == 1022 )
        return 0;
    else
        return &g_entities[occEntNum];
}

gentity_s *__cdecl VEH_GetSeatOccupantEntity(gentity_s *vehEnt, int seatIdx)
{
    scr_vehicle_s *veh; // [esp+4h] [ebp-4h]

    veh = GetVehicleLogObjectError(vehEnt);
    if ( veh )
        return VEH_GetSeatOccupantEntity(veh, seatIdx);
    else
        return 0;
}

void __cdecl VEH_SetPosition(gentity_s *ent, const float *origin, float *vel, const float *angles)
{
    const char *v4; // eax
    int v6; // [esp+20h] [ebp-54h]
    int v7; // [esp+24h] [ebp-50h]
    int v8; // [esp+28h] [ebp-4Ch]
    bool v9; // [esp+2Ch] [ebp-48h]
    char *v10; // [esp+30h] [ebp-44h]
    float snapped_vela; // [esp+50h] [ebp-24h]
    float snapped_vel; // [esp+50h] [ebp-24h]
    float snapped_vel_4a; // [esp+54h] [ebp-20h]
    float snapped_vel_4; // [esp+54h] [ebp-20h]
    float snapped_vel_8a; // [esp+58h] [ebp-1Ch]
    float snapped_vel_8; // [esp+58h] [ebp-1Ch]
    float old_vel; // [esp+5Ch] [ebp-18h]
    float old_vel_4; // [esp+60h] [ebp-14h]
    float old_vel_8; // [esp+64h] [ebp-10h]
    float snapped_origin[3]; // [esp+68h] [ebp-Ch] BYREF
    int savedregs; // [esp+74h] [ebp+0h] BYREF

    snapped_origin[0] = *origin;
    snapped_origin[1] = origin[1];
    snapped_origin[2] = origin[2];
    snapped_vela = *vel;
    snapped_vel_4a = vel[1];
    snapped_vel_8a = vel[2];
    old_vel = ent->s.lerp.pos.trDelta[0];
    old_vel_4 = ent->s.lerp.pos.trDelta[1];
    old_vel_8 = ent->s.lerp.pos.trDelta[2];
    snapped_origin[0] = (float)(int)snapped_origin[0];
    snapped_origin[1] = (float)(int)snapped_origin[1];
    snapped_origin[2] = (float)(int)snapped_origin[2];
    snapped_vel = (float)(int)snapped_vela;
    snapped_vel_4 = (float)(int)snapped_vel_4a;
    snapped_vel_8 = (float)(int)snapped_vel_8a;
    if ( !BG_ValidateOriginValue(snapped_origin[2], 17, svs.mapCenter[2]) )
    {
        if ( ent->targetname )
            v10 = SL_ConvertToString(ent->targetname, SCRIPTINSTANCE_SERVER);
        else
            v10 = (char*)"<undefined>";
        v4 = va("Vehicle %s has fallen out of the world at (%0.2f, %0.2f, %0.2f)\n", v10, *origin, origin[1], origin[2]);
        Scr_Error(v4, 0);
    }
    v9 = ent->r.currentOrigin[0] == snapped_origin[0]
        && ent->r.currentOrigin[1] == snapped_origin[1]
        && ent->r.currentOrigin[2] == snapped_origin[2];
    if ( v9
        && (ent->s.lerp.pos.trBase[0] != snapped_origin[0]
         || ent->s.lerp.pos.trBase[1] != snapped_origin[1]
         || ent->s.lerp.pos.trBase[2] != snapped_origin[2]
            ? (v8 = 0)
            : (v8 = 1),
                v8
         && (ent->r.currentAngles[0] != *angles
            || ent->r.currentAngles[1] != angles[1]
            || ent->r.currentAngles[2] != angles[2]
             ? (v7 = 0)
             : (v7 = 1),
                 v7
            && (ent->s.lerp.apos.trBase[0] != *angles
             || ent->s.lerp.apos.trBase[1] != angles[1]
             || ent->s.lerp.apos.trBase[2] != angles[2]
                ? (v6 = 0)
                : (v6 = 1),
                    v6))) )
    {
        if ( old_vel != snapped_vel || old_vel_4 != snapped_vel_4 || old_vel_8 != snapped_vel_8 )
        {
            ent->s.lerp.pos.trDelta[0] = snapped_vel;
            ent->s.lerp.pos.trDelta[1] = snapped_vel_4;
            ent->s.lerp.pos.trDelta[2] = snapped_vel_8;
        }
    }
    else
    {
        G_SetOrigin(ent, snapped_origin);
        G_SetAngle(ent, angles);
        ent->s.lerp.pos.trType = 1;
        ent->s.lerp.apos.trType = 1;
        ent->s.lerp.pos.trDelta[0] = snapped_vel;
        ent->s.lerp.pos.trDelta[1] = snapped_vel_4;
        ent->s.lerp.pos.trDelta[2] = snapped_vel_8;
        SV_LinkEntity(ent);
    }
}

void __cdecl VEH_InitEntity(gentity_s *ent, scr_vehicle_s *veh, int infoIdx)
{
    int WeaponIndexForName; // eax
    float v4; // [esp+8h] [ebp-20h]
    float v5; // [esp+Ch] [ebp-1Ch]
    float v6; // [esp+10h] [ebp-18h]
    float v7; // [esp+14h] [ebp-14h]
    float v8; // [esp+18h] [ebp-10h]
    int i; // [esp+1Ch] [ebp-Ch]
    const vehicle_info_t *info; // [esp+20h] [ebp-8h]
    int savedregs; // [esp+28h] [ebp+0h] BYREF

    info = BG_GetVehicleInfo(infoIdx);
    ent->handler = 25;
    ent->r.svFlags = 4;
    ent->r.contents = 8462464;
    if ( (ent->spawnflags & 1) != 0 )
        ent->r.contents |= 0x200000u;
    ent->s.eType = ET_VEHICLE;
    if ( (ent->s.lerp.eFlags & 0x20000) != 0 )
        ent->s.lerp.eFlags |= 0x20000u;
    ent->s.lerp.pos.trType = 1;
    ent->s.lerp.apos.trType = 1;
    ent->s.time2 = 0;
    ent->s.loopSoundId = 0;
    ent->s.loopSoundFade = 0;
    if ( !g_connectpaths->current.integer )
    {
        WeaponIndexForName = G_GetWeaponIndexForName((char*)info->turretWeapon);
        AssignToSmallerType<unsigned short>(&ent->s.weapon, WeaponIndexForName);
    }
    ent->s.weaponModel = 0;
    ent->s.lerp.u.actor.actorNum = 0;
    ent->s.lerp.u.vehicle.drawOnCompass = 0;
    v8 = floor(0.0 * 182.04445 + 0.5);
    ent->s.lerp.u.vehicle.throttle = (int)v8;
    ent->s.lerp.u.loopFx.period = 0;
    v7 = floor(0.0 * 182.04445 + 0.5);
    ent->s.lerp.u.vehicle.gunPitch = (int)v7;
    v6 = floor(0.0 * 182.04445 + 0.5);
    ent->s.lerp.u.vehicle.gunYaw = (int)v6;
    for ( i = 0; i < 4; ++i )
    {
        v5 = floor(0.0 * 182.04445 + 0.5);
        ent->s.lerp.u.vehicle.gunnerAngles[i].pitch = (int)v5;
        v4 = floor(0.0 * 182.04445 + 0.5);
        ent->s.lerp.u.vehicle.gunnerAngles[i].yaw = (int)v4;
    }
    ent->s.vehicleState.vehicleInfoIndex = infoIdx;
    ent->scr_vehicle = veh;
    ent->nextthink = level.time + 50;
    ent->takedamage = 1;
    ent->active = 1;
    ent->clipmask = 8454673;
    ent->flags |= 0x800u;
    SV_DObjGetBounds(ent, ent->r.mins, ent->r.maxs);
    SV_LinkEntity(ent);
}

void __cdecl VEH_InitVehicle(gentity_s *ent, scr_vehicle_s *veh, int infoIdx)
{
    int WeaponIndexForName; // eax
    int k; // [esp+Ch] [ebp-Ch]
    const vehicle_info_t *info; // [esp+10h] [ebp-8h]
    int seatIndex; // [esp+14h] [ebp-4h]

    info = BG_GetVehicleInfo(infoIdx);
    veh->infoIdx = infoIdx;
    veh->entNum = ent->s.number;
    veh->moveState = VEH_MOVESTATE_STOP;
    veh->flags = 0;
    veh->waitNode = -1;
    veh->waitSpeed = -1.0f;
    veh->turret.fireTime = 0;
    veh->turret.fireBarrel = 0;
    veh->turret.turretState = VEH_TURRET_STOPPED;
    veh->turret.barrelOffset = 0.0f;
    veh->turret.flags = 0;
    veh->turret.heatVal = 0.0f;
    veh->turret.overheating = 0;
    veh->turretRotScale = 1.0f;
    Com_Memset((unsigned int *)&veh->jitter, 0, 60);
    veh->lookAtText0 = 0;
    veh->lookAtText1 = 0;
    veh->manualMode = 0;
    veh->manualTime = 0.0f;
    veh->speed = 0.0f;
    veh->maxSpeedOverride = 0.0f;
    veh->maxDragSpeed = 60.0 * 17.6;
    veh->turningAbility = 0.5f;
    veh->pathTransitionTime = 0.0f;
    veh->hasTarget = 0;
    veh->targetEnt = 1023;

    iassert(!veh->lookAtEnt.isDefined());

    veh->targetOrigin[0] = 0.0f;
    veh->targetOrigin[1] = 0.0f;
    veh->targetOrigin[2] = 0.0f;
    veh->targetOffset[0] = 0.0f;
    veh->targetOffset[1] = 0.0f;
    veh->targetOffset[2] = 0.0f;
    veh->joltDir[0] = 0.0f;
    veh->joltDir[1] = 0.0f;
    veh->joltTime = 0.0f;
    veh->joltWave = 0.0f;
    veh->joltSpeed = 0.0f;
    veh->joltDecel = 0.0f;
    veh->turretHitNum = 0;
    veh->hover.hoverRadius = 30.0f;
    veh->hover.hoverSpeed = 10.0 * 17.6;
    veh->hover.hoverAccel = 5.0 * 17.6;
    veh->hover.useHoverAccelForAngles = 0;
    veh->yawSlowDown = 0;
    veh->yawOverShoot = 0.1f;
    veh->prevGoalYaw = -1.0f;
    veh->nearGoalNotifyDist = 0.0f;
    veh->modelSwapDelay = info->modelSwapDelay;
    veh->oneExhaust = info->oneExhaust;
    veh->deathQuakeScale = info->deathQuakeScale;
    veh->deathQuakeDuration = info->deathQuakeDuration;
    veh->deathQuakeRadius = info->deathQuakeRadius;
    veh->secTurretAiControlled = info->secTurretAiControlled;
    veh->frontArmorRegen = info->frontArmorRegen;
    veh->addToCompass = info->addToCompass;
    veh->m_bSpecialAbilityEventDown = 0;
    veh->m_bFirePickupEventDown = 0;
    veh->m_bSwapPickupEventDown = 0;
    veh->m_bDropDeployableEventDown = 0;
    for ( seatIndex = 0; seatIndex < 11; ++seatIndex )
        VEH_InitSeat(veh, seatIndex);
    for ( k = 0; k < 4; ++k )
    {
        if ( !g_connectpaths->current.integer && !info->gunnerWeaponIndex[k] )
        {
            WeaponIndexForName = G_GetWeaponIndexForName((char*)info->gunnerWeapon[k]);
            AssignToSmallerType<unsigned short>(&bg_vehicleInfos[infoIdx].gunnerWeaponIndex[k], WeaponIndexForName);
        }
        veh->gunnerTargets[k].targetEnt = 1023;
        veh->gunnerTurrets[k].heatVal = 0.0f;
        veh->gunnerTurrets[k].overheating = 0;
    }
    VEH_SetPosition(ent, ent->r.currentOrigin, (float *)vec3_origin, ent->r.currentAngles);
    VEH_InitPhysics(ent, infoIdx);
    VEH_InitFromInfo(veh, info);
}

void __cdecl VEH_InitSeat(scr_vehicle_s *veh, int seatIdx)
{
    VehicleSeat *seat; // [esp+0h] [ebp-4h]

    seat = VEH_GetSeat(veh, seatIdx);
    if ( seat )
        seat->_occupantEntNum = 1023;
}

void __cdecl VEH_InitFromInfo(scr_vehicle_s *veh, const vehicle_info_t *info)
{
    veh->team = info->eTeam;
    veh->phys.maxPitchAngle = info->maxBodyPitch;
    veh->phys.maxRollAngle = info->maxBodyRoll;
    veh->phys.maxAngleVel[1] = info->rotRate;
    veh->phys.maxAngleVel[0] = info->rotRate * 0.5;
    veh->phys.maxAngleVel[2] = info->rotRate * 0.5;
    veh->phys.yawAccel = info->rotAccel;
    veh->phys.yawDecel = info->rotAccel * 0.64999998;
    veh->manualSpeed = info->maxSpeed;
    veh->manualAccel = info->accel;
    veh->manualDecel = info->accel;
}

void __cdecl VEH_InitPhysics(gentity_s *ent, int infoIdx)
{
    vehicle_physic_t *phys; // [esp+38h] [ebp-10h]
    const vehicle_info_t *info; // [esp+3Ch] [ebp-Ch]
    scr_vehicle_s *veh; // [esp+40h] [ebp-8h]
    int i; // [esp+44h] [ebp-4h]

    veh = ent->scr_vehicle;
    phys = &veh->phys;
    veh->phys.origin[0] = ent->r.currentOrigin[0];
    veh->phys.origin[1] = ent->r.currentOrigin[1];
    veh->phys.origin[2] = ent->r.currentOrigin[2];
    veh->phys.prevOrigin[0] = ent->r.currentOrigin[0];
    veh->phys.prevOrigin[1] = ent->r.currentOrigin[1];
    veh->phys.prevOrigin[2] = ent->r.currentOrigin[2];
    veh->phys.angles[0] = ent->r.currentAngles[0];
    veh->phys.angles[1] = ent->r.currentAngles[1];
    veh->phys.angles[2] = ent->r.currentAngles[2];
    veh->phys.prevAngles[0] = ent->r.currentAngles[0];
    veh->phys.prevAngles[1] = ent->r.currentAngles[1];
    veh->phys.prevAngles[2] = ent->r.currentAngles[2];
    veh->phys.mins[0] = 0.0f;
    veh->phys.mins[1] = 0.0f;
    veh->phys.mins[2] = 0.0f;
    veh->phys.maxs[0] = 0.0f;
    veh->phys.maxs[1] = 0.0f;
    veh->phys.maxs[2] = 0.0f;
    veh->phys.vel[0] = 0.0f;
    veh->phys.vel[1] = 0.0f;
    veh->phys.vel[2] = 0.0f;
    veh->phys.bodyVel[0] = 0.0f;
    veh->phys.bodyVel[1] = 0.0f;
    veh->phys.bodyVel[2] = 0.0f;
    veh->phys.rotVel[0] = 0.0f;
    veh->phys.rotVel[1] = 0.0f;
    veh->phys.rotVel[2] = 0.0f;
    for ( i = 0; i < 6; ++i )
    {
        phys->wheelZVel[i] = 0.0f;
        phys->wheelZPos[i] = 0.0f;
        phys->wheelSurfType[i] = 0;
    }
    veh->phys.worldTilt[0] = 0.0f;
    veh->phys.worldTilt[1] = 0.0f;
    veh->phys.worldTilt[2] = 0.0f;
    veh->phys.worldTiltVel[0] = 0.0f;
    veh->phys.worldTiltVel[1] = 0.0f;
    veh->phys.worldTiltVel[2] = 0.0f;
    info = BG_GetVehicleInfo(infoIdx);
    if ( info->isNitrous )
    {
        Sys_EnterCriticalSection(CRITSECT_PHYSICS_UPDATE);
        Sys_EnterCriticalSection(CRITSECT_PHYSICS);
        if ( veh->nitrousVehicle
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                        1259,
                        0,
                        "%s",
                        "!veh->nitrousVehicle") )
        {
            __debugbreak();
        }
        veh->nitrousVehicle = (NitrousVehicle *)NitrousVehicle::add_vehicle(ent->s.number);

        if (veh->nitrousVehicle)
        {
            //NitrousVehicle::init(veh->nitrousVehicle, ent, &info->nitrousVehParams);
            veh->nitrousVehicle->init(ent, &info->nitrousVehParams);
        }
        Sys_LeaveCriticalSection(CRITSECT_PHYSICS);
        Sys_LeaveCriticalSection(CRITSECT_PHYSICS_UPDATE);
    }
}

void __cdecl VEH_JoltBody(gentity_s *ent, const float *dir, float intensity, float speedFrac, float decel)
{
    float v5; // [esp+0h] [ebp-48h]
    float v6; // [esp+14h] [ebp-34h]
    const vehicle_info_t *info; // [esp+1Ch] [ebp-2Ch]
    scr_vehicle_s *veh; // [esp+20h] [ebp-28h]
    float axis[3][3]; // [esp+24h] [ebp-24h] BYREF

    veh = ent->scr_vehicle;
    info = BG_GetVehicleInfo(veh->infoIdx);
    if ( !info->isNitrous && info->type != 6 )
    {
        if ( (float)(intensity - 1.0) < 0.0 )
            v6 = intensity;
        else
            v6 = 1.0f;
        if ( (float)(0.0 - intensity) < 0.0 )
            v5 = v6;
        else
            v5 = 0.0f;
        AnglesToAxis(veh->phys.angles, axis);
        veh->joltDir[0] = (float)((float)(*dir * axis[0][0]) + (float)(dir[1] * axis[0][1])) + (float)(dir[2] * axis[0][2]);
        //LODWORD(veh->joltDir[1]) = COERCE_UNSIGNED_INT((float)((float)(*dir * axis[1][0]) + (float)(dir[1] * axis[1][1])) + (float)(dir[2] * axis[1][2])) ^ _mask__NegFloat_;
        (veh->joltDir[1]) = -((float)((float)(*dir * axis[1][0]) + (float)(dir[1] * axis[1][1])) + (float)(dir[2] * axis[1][2]));
        veh->joltTime = 1.3f;
        veh->joltWave = 0.0f;
        Vec2Normalize(veh->joltDir);
        veh->joltDir[0] = (float)(info->maxBodyPitch * v5) * veh->joltDir[0];
        veh->joltDir[1] = (float)(info->maxBodyRoll * v5) * veh->joltDir[1];
        veh->joltSpeed = veh->speed * speedFrac;
        veh->joltDecel = decel;
    }
}

void __cdecl VEH_CalcAccel(gentity_s *ent, char *move, float *bodyAccel, float *rotAccel)
{
    float v4; // [esp+4h] [ebp-CCh]
    float v5; // [esp+8h] [ebp-C8h]
    float v6; // [esp+Ch] [ebp-C4h]
    float v7; // [esp+10h] [ebp-C0h]
    float v8; // [esp+14h] [ebp-BCh]
    float v9; // [esp+18h] [ebp-B8h]
    float v10; // [esp+1Ch] [ebp-B4h]
    float v11; // [esp+20h] [ebp-B0h]
    float v12; // [esp+2Ch] [ebp-A4h]
    float v13; // [esp+3Ch] [ebp-94h]
    float v14; // [esp+40h] [ebp-90h]
    float v15; // [esp+4Ch] [ebp-84h]
    float rotRate; // [esp+58h] [ebp-78h]
    float v17; // [esp+64h] [ebp-6Ch]
    float v18; // [esp+74h] [ebp-5Ch]
    float v19; // [esp+7Ch] [ebp-54h]
    float v20; // [esp+80h] [ebp-50h]
    float v21; // [esp+88h] [ebp-48h]
    gentity_s *player; // [esp+94h] [ebp-3Ch]
    float xAccel; // [esp+98h] [ebp-38h]
    const vehicle_info_t *info; // [esp+A0h] [ebp-30h]
    scr_vehicle_s *veh; // [esp+A4h] [ebp-2Ch]
    float tgtVel[3]; // [esp+A8h] [ebp-28h]
    float maxSpeed; // [esp+B4h] [ebp-1Ch]
    float yAccel; // [esp+B8h] [ebp-18h]
    int i; // [esp+BCh] [ebp-14h]
    float tgtRotVel[3]; // [esp+C0h] [ebp-10h]
    float accel; // [esp+CCh] [ebp-4h]

    veh = ent->scr_vehicle;
    info = BG_GetVehicleInfo(veh->infoIdx);
    if ( veh->joltTime <= 0.0 )
    {
        if ( veh->maxSpeedOverride <= 0.0 )
            maxSpeed = info->maxSpeed;
        else
            maxSpeed = veh->maxSpeedOverride;
        accel = info->accel;
    }
    else
    {
        maxSpeed = veh->joltSpeed;
        accel = veh->joltDecel;
    }
    xAccel = fabs((float)*move) / 127.0;
    yAccel = fabs((float)move[1]) / 127.0;
    if ( *move <= 0 )
    {
        if ( *move >= 0 )
            tgtVel[0] = 0.0f;
        else
            tgtVel[0] = (float)(veh->phys.bodyVel[0] - (float)(accel * 0.05f)) * xAccel;
    }
    else
    {
        tgtVel[0] = (float)((float)(accel * 0.05f) + veh->phys.bodyVel[0]) * xAccel;
    }
    tgtVel[1] = 0.0f;
    tgtVel[2] = veh->phys.bodyVel[2];
    for ( i = 0; i < 3; ++i )
    {
        v20 = tgtVel[i];
        if ( (float)(v20 - maxSpeed) < 0.0 )
            v21 = tgtVel[i];
        else
            v21 = maxSpeed;
        if ( (float)((-(maxSpeed)) - v20) < 0.0 )
            v10 = v21;
        else
            v10 = -maxSpeed;
        tgtVel[i] = v10;
        bodyAccel[i] = (float)(tgtVel[i] - veh->phys.bodyVel[i]) / 0.05f;
        v18 = bodyAccel[i];
        if ( (float)(v18 - accel) < 0.0 )
            v19 = bodyAccel[i];
        else
            v19 = accel;
        if ( (float)((-(accel)) - v18) < 0.0 )
            v9 = v19;
        else
            v9 = -accel;
        bodyAccel[i] = v9;
    }
    if ( move[2] <= 0 )
    {
        if ( move[1] >= 0 )
        {
            if ( move[1] <= 0 )
                tgtRotVel[1] = 0.0f;
            else
                tgtRotVel[1] = (float)(veh->phys.rotVel[1] - (float)(info->rotAccel * 0.05f)) * yAccel;
        }
        else
        {
            tgtRotVel[1] = (float)((float)(info->rotAccel * 0.05f) + veh->phys.rotVel[1]) * yAccel;
        }
        if ( (float)(tgtRotVel[1] - info->rotRate) < 0.0 )
            rotRate = tgtRotVel[1];
        else
            rotRate = info->rotRate;
        if ( (float)((-(info->rotRate)) - tgtRotVel[1]) < 0.0 )
            v7 = rotRate;
        else
            v7 = -info->rotRate;
        tgtRotVel[1] = v7;
        rotAccel[1] = (float)(v7 - veh->phys.rotVel[1]) / 0.05f;
        v14 = rotAccel[1];
        if ( (float)(v14 - info->rotAccel) < 0.0 )
            v15 = rotAccel[1];
        else
            v15 = info->rotAccel;
        if ( (float)((-(info->rotAccel)) - v14) < 0.0 )
            v6 = v15;
        else
            v6 = -info->rotAccel;
        rotAccel[1] = v6;
    }
    else
    {
        iassert(ent->r.ownerNum.isDefined());

        //player = EntHandle::ent(&ent->r.ownerNum);
        player = ent->r.ownerNum.ent();

        iassert(player->client);

        tgtRotVel[1] = AngleNormalize180(player->client->ps.viewangles[1] - veh->phys.prevAngles[1]) / 0.05f;
        if ( (float)(tgtRotVel[1] - info->rotRate) < 0.0 )
            v17 = tgtRotVel[1];
        else
            v17 = info->rotRate;
        if ( (float)((-(info->rotRate)) - tgtRotVel[1]) < 0.0 )
            v8 = v17;
        else
            v8 = -info->rotRate;
        tgtRotVel[1] = v8;
        rotAccel[1] = (float)(v8 - veh->phys.rotVel[1]) / 0.05f;
    }
    tgtRotVel[0] = 0.0f;
    *rotAccel = (float)(0.0 - veh->phys.rotVel[0]) / 0.05f;
    if ( (float)(*rotAccel - info->rotAccel) < 0.0 )
        v13 = *rotAccel;
    else
        v13 = info->rotAccel;
    if ( (float)((-(info->rotAccel)) - *rotAccel) < 0.0 )
        v5 = v13;
    else
        v5 = -info->rotAccel;
    *rotAccel = v5;
    tgtRotVel[2] = 0.0f;
    rotAccel[2] = (float)(0.0 - veh->phys.rotVel[2]) / 0.05f;
    v11 = rotAccel[2];
    if ( (float)(v11 - info->rotAccel) < 0.0 )
        v12 = rotAccel[2];
    else
        v12 = info->rotAccel;
    if ( (float)((-(info->rotAccel)) - v11) < 0.0 )
        v4 = v12;
    else
        v4 = -info->rotAccel;
    rotAccel[2] = v4;
}

int __cdecl VEH_CorrectAllSolid(gentity_s *ent, trace_t *trace)
{
    col_context_t context; // [esp+14h] [ebp-40h] BYREF
    vehicle_physic_t *phys; // [esp+3Ch] [ebp-18h]
    scr_vehicle_s *veh; // [esp+40h] [ebp-14h]
    unsigned int i; // [esp+44h] [ebp-10h]
    float point[3]; // [esp+48h] [ebp-Ch] BYREF

    veh = ent->scr_vehicle;
    phys = &veh->phys;
    //col_context_t::col_context_t(&context);
    for ( i = 0; i < 0x1A; ++i )
    {
        point[0] = phys->origin[0] + (float)s_correctSolidDeltas[i][0];
        point[1] = phys->origin[1] + (float)s_correctSolidDeltas[i][1];
        point[2] = phys->origin[2] + (float)s_correctSolidDeltas[i][2];
        G_TraceCapsule(trace, point, phys->mins, phys->maxs, point, ent->s.number, ent->clipmask, &context);
        if ( !trace->startsolid )
        {
            phys->origin[0] = point[0];
            phys->origin[1] = point[1];
            phys->origin[2] = point[2];
            point[0] = phys->origin[0];
            point[1] = phys->origin[1];
            point[2] = phys->origin[2] - 1.0;
            G_TraceCapsule(trace, phys->origin, phys->mins, phys->maxs, point, ent->s.number, ent->clipmask, &context);
            memcpy(&s_phys, trace, 0x38u);
            Vec3Lerp(phys->origin, point, trace->fraction, phys->origin);
            return 1;
        }
    }
    return 0;
}

void __cdecl VEH_ClearGround()
{
    s_phys.hasGround = 0;
}

bool __cdecl VEH_SlideMove(gentity_s *ent, int gravity)
{
    float *v3; // [esp+8h] [ebp-158h]
    float *v4; // [esp+Ch] [ebp-154h]
    float *v5; // [esp+10h] [ebp-150h]
    float *v6; // [esp+38h] [ebp-128h]
    float *v7; // [esp+3Ch] [ebp-124h]
    float *v8; // [esp+40h] [ebp-120h]
    float *vel; // [esp+48h] [ebp-118h]
    float timeLeft; // [esp+60h] [ebp-100h]
    int j; // [esp+64h] [ebp-FCh]
    col_context_t context; // [esp+68h] [ebp-F8h] BYREF
    vehicle_physic_t *phys; // [esp+90h] [ebp-D0h]
    float dir[3]; // [esp+94h] [ebp-CCh] BYREF
    int bumpCount; // [esp+A0h] [ebp-C0h]
    scr_vehicle_s *veh; // [esp+A4h] [ebp-BCh]
    int k; // [esp+A8h] [ebp-B8h]
    float planes[5][3]; // [esp+ACh] [ebp-B4h] BYREF
    float clipVel[3]; // [esp+E8h] [ebp-78h] BYREF
    float end[3]; // [esp+F4h] [ebp-6Ch] BYREF
    float endVel[3]; // [esp+100h] [ebp-60h] BYREF
    int numPlanes; // [esp+10Ch] [ebp-54h]
    trace_t trace; // [esp+110h] [ebp-50h] BYREF
    float endClipVel[3]; // [esp+14Ch] [ebp-14h] BYREF
    int i; // [esp+158h] [ebp-8h]
    float dot; // [esp+15Ch] [ebp-4h]

    veh = ent->scr_vehicle;
    phys = &veh->phys;
    timeLeft = 0.05f;
    endVel[0] = veh->phys.vel[0];
    endVel[1] = veh->phys.vel[1];
    endVel[2] = veh->phys.vel[2];
    if ( gravity )
    {
        endVel[2] = endVel[2] - 40.0;
        phys->vel[2] = (float)(phys->vel[2] + endVel[2]) * 0.5;
        if ( s_phys.hasGround )
            VEH_ClipVelocity(phys->vel, s_phys.groundTrace.normal.vec.v, phys->vel);
    }
    if ( s_phys.hasGround )
    {
        numPlanes = 1;
        //*(_QWORD *)&planes[0][0] = *(_QWORD *)s_phys.groundTrace.normal.vec.v;
        planes[0][0] = s_phys.groundTrace.normal.vec.v[0];
        planes[0][1] = s_phys.groundTrace.normal.vec.v[1];
        planes[0][2] = s_phys.groundTrace.normal.vec.v[2];
        //LODWORD(planes[0][2]) = s_phys.groundTrace.normal.vec.u[2];
    }
    else
    {
        numPlanes = 0;
    }
    Vec3NormalizeTo(phys->vel, planes[numPlanes++]);
    //col_context_t::col_context_t(&context);
    for ( bumpCount = 0; bumpCount < 4; ++bumpCount )
    {
        end[0] = (float)(timeLeft * phys->vel[0]) + phys->origin[0];
        end[1] = (float)(timeLeft * phys->vel[1]) + phys->origin[1];
        end[2] = (float)(timeLeft * phys->vel[2]) + phys->origin[2];
        G_TraceCapsule(&trace, phys->origin, phys->mins, phys->maxs, end, ent->s.number, ent->clipmask, &context);
        if ( trace.startsolid )
        {
            phys->vel[2] = 0.0f;
            return 1;
        }
        if ( trace.fraction > 0.0 )
            Vec3Lerp(phys->origin, end, trace.fraction, phys->origin);
        if ( trace.fraction == 1.0 )
            break;
        timeLeft = timeLeft - (float)(timeLeft * trace.fraction);
        if ( numPlanes >= 5 )
        {
            vel = phys->vel;
            phys->vel[0] = 0.0f;
            vel[1] = 0.0f;
            vel[2] = 0.0f;
            return 1;
        }
        for ( i = 0; i < numPlanes; ++i )
        {
            if ( (float)((float)((float)(trace.normal.vec.v[0] * planes[i][0]) + (float)(trace.normal.vec.v[1] * planes[i][1]))
                                 + (float)(trace.normal.vec.v[2] * planes[i][2])) > 0.99000001 )
            {
                v7 = phys->vel;
                v8 = phys->vel;
                phys->vel[0] = trace.normal.vec.v[0] + phys->vel[0];
                v7[1] = trace.normal.vec.v[1] + v8[1];
                v7[2] = trace.normal.vec.v[2] + v8[2];
                break;
            }
        }
        if ( i >= numPlanes )
        {
            v6 = planes[numPlanes];
            //*(_QWORD *)v6 = *(_QWORD *)trace.normal.vec.v;
            v6[0] = trace.normal.vec.v[0];
            v6[1] = trace.normal.vec.v[1];
            v6[2] = trace.normal.vec.v[2];
            ++numPlanes;
            for ( i = 0; i < numPlanes; ++i )
            {
                if ( (float)((float)((float)(phys->vel[0] * planes[i][0]) + (float)(phys->vel[1] * planes[i][1]))
                                     + (float)(phys->vel[2] * planes[i][2])) < 0.1 )
                {
                    VEH_ClipVelocity(phys->vel, planes[i], clipVel);
                    VEH_ClipVelocity(endVel, planes[i], endClipVel);
                    for ( j = 0; j < numPlanes; ++j )
                    {
                        if ( j != i
                            && (float)((float)((float)(clipVel[0] * planes[j][0]) + (float)(clipVel[1] * planes[j][1]))
                                             + (float)(clipVel[2] * planes[j][2])) < 0.1 )
                        {
                            VEH_ClipVelocity(clipVel, planes[j], clipVel);
                            VEH_ClipVelocity(endClipVel, planes[j], endClipVel);
                            if ( (float)((float)((float)(clipVel[0] * planes[i][0]) + (float)(clipVel[1] * planes[i][1]))
                                                 + (float)(clipVel[2] * planes[i][2])) < 0.0 )
                            {
                                Vec3Cross(planes[i], planes[j], dir);
                                Vec3Normalize(dir);
                                dot = (float)((float)(dir[0] * phys->vel[0]) + (float)(dir[1] * phys->vel[1]))
                                        + (float)(dir[2] * phys->vel[2]);
                                clipVel[0] = dot * dir[0];
                                clipVel[1] = dot * dir[1];
                                clipVel[2] = dot * dir[2];
                                dot = (float)((float)(dir[0] * endVel[0]) + (float)(dir[1] * endVel[1])) + (float)(dir[2] * endVel[2]);
                                endClipVel[0] = dot * dir[0];
                                endClipVel[1] = dot * dir[1];
                                endClipVel[2] = dot * dir[2];
                                for ( k = 0; k < numPlanes; ++k )
                                {
                                    if ( k != i
                                        && k != j
                                        && (float)((float)((float)(clipVel[0] * planes[k][0]) + (float)(clipVel[1] * planes[k][1]))
                                                         + (float)(clipVel[2] * planes[k][2])) < 0.1 )
                                    {
                                        v5 = phys->vel;
                                        phys->vel[0] = 0.0f;
                                        v5[1] = 0.0f;
                                        v5[2] = 0.0f;
                                        return 1;
                                    }
                                }
                            }
                        }
                    }
                    v4 = phys->vel;
                    phys->vel[0] = clipVel[0];
                    v4[1] = clipVel[1];
                    v4[2] = clipVel[2];
                    endVel[0] = endClipVel[0];
                    endVel[1] = endClipVel[1];
                    endVel[2] = endClipVel[2];
                    break;
                }
            }
        }
    }
    if ( gravity )
    {
        v3 = phys->vel;
        phys->vel[0] = endVel[0];
        v3[1] = endVel[1];
        v3[2] = endVel[2];
    }
    return bumpCount != 0;
}

void __cdecl VEH_ClipVelocity(float *in, float *normal, float *out)
{
    int i; // [esp+4h] [ebp-8h]
    float backoff; // [esp+8h] [ebp-4h]
    float backoffa; // [esp+8h] [ebp-4h]

    if ( normal[2] < 0.69999999 || (float)((float)(*in * *in) + (float)(in[1] * in[1])) < (float)(in[2] * in[2]) )
    {
        backoff = (float)((float)(*in * *normal) + (float)(in[1] * normal[1])) + (float)(in[2] * normal[2]);
        if ( backoff >= 0.0 )
            backoffa = backoff / 1.01;
        else
            backoffa = backoff * 1.01;
        for ( i = 0; i < 3; ++i )
            out[i] = in[i] - (float)(normal[i] * backoffa);
    }
    else
    {
        //out[2] = (float)(COERCE_FLOAT(*(unsigned int *)in ^ _mask__NegFloat_) * *normal) - (float)(in[1] * normal[1]);
        out[2] = (float)(-(*in) * *normal) - (float)(in[1] * normal[1]);
        *out = *in * normal[2];
        out[1] = in[1] * normal[2];
    }
}

void __cdecl VEH_AirMove(gentity_s *ent, int gravity)
{
    if ( s_phys.hasGround )
        VEH_ClipVelocity(ent->scr_vehicle->phys.vel, s_phys.groundTrace.normal.vec.v, ent->scr_vehicle->phys.vel);
    VEH_StepSlideMove(ent, gravity);
}

void __cdecl VEH_StepSlideMove(gentity_s *ent, int gravity)
{
    float *vel; // [esp+8h] [ebp-B0h]
    col_context_t context; // [esp+10h] [ebp-A8h] BYREF
    vehicle_physic_t *phys; // [esp+38h] [ebp-80h]
    scr_vehicle_s *veh; // [esp+3Ch] [ebp-7Ch]
    float startOrigin[3]; // [esp+40h] [ebp-78h] BYREF
    float endpos[3]; // [esp+4Ch] [ebp-6Ch] BYREF
    trace_t trace; // [esp+58h] [ebp-60h] BYREF
    float up[3]; // [esp+94h] [ebp-24h] BYREF
    float down[3]; // [esp+A0h] [ebp-18h] BYREF
    float startVel[3]; // [esp+ACh] [ebp-Ch]

    veh = ent->scr_vehicle;
    phys = &veh->phys;
    //col_context_t::col_context_t(&context);
    startOrigin[0] = phys->origin[0];
    startOrigin[1] = phys->origin[1];
    startOrigin[2] = phys->origin[2];
    startVel[0] = phys->vel[0];
    startVel[1] = phys->vel[1];
    startVel[2] = phys->vel[2];
    if ( VEH_SlideMove(ent, gravity) )
    {
        down[0] = startOrigin[0];
        down[1] = startOrigin[1];
        down[2] = startOrigin[2] - 18.0;
        G_TraceCapsule(&trace, startOrigin, phys->mins, phys->maxs, down, ent->s.number, ent->clipmask, &context);
        if ( phys->vel[2] <= 0.0 || trace.fraction != 1.0 && trace.normal.vec.v[2] >= 0.69999999 )
        {
            up[0] = startOrigin[0];
            up[1] = startOrigin[1];
            up[2] = startOrigin[2] + 18.0;
            G_TraceCapsule(&trace, startOrigin, phys->mins, phys->maxs, up, ent->s.number, ent->clipmask, &context);
            if ( !trace.startsolid )
            {
                Vec3Lerp(startOrigin, up, trace.fraction, endpos);
                phys->origin[0] = endpos[0];
                phys->origin[1] = endpos[1];
                phys->origin[2] = endpos[2];
                vel = phys->vel;
                phys->vel[0] = startVel[0];
                vel[1] = startVel[1];
                vel[2] = startVel[2];
                VEH_SlideMove(ent, gravity);
                down[0] = phys->origin[0];
                down[1] = phys->origin[1];
                down[2] = phys->origin[2];
                down[2] = (float)(startOrigin[2] - endpos[2]) + down[2];
                G_TraceCapsule(&trace, phys->origin, phys->mins, phys->maxs, down, ent->s.number, ent->clipmask, &context);
                if ( !trace.startsolid )
                    Vec3Lerp(phys->origin, down, trace.fraction, phys->origin);
                if ( trace.fraction < 1.0 )
                    VEH_ClipVelocity(phys->vel, trace.normal.vec.v, phys->vel);
            }
        }
    }
}

bool __cdecl VEH_ClampPointToBoundingBox(gentity_s *vehicle, const float *point, float *out)
{
    float v4; // [esp+0h] [ebp-118h]
    float v5; // [esp+8h] [ebp-110h]
    float v6; // [esp+Ch] [ebp-10Ch]
    float v7; // [esp+10h] [ebp-108h]
    float v8; // [esp+1Ch] [ebp-FCh]
    float v9; // [esp+24h] [ebp-F4h]
    float v10; // [esp+28h] [ebp-F0h]
    float v11; // [esp+2Ch] [ebp-ECh]
    float v12; // [esp+30h] [ebp-E8h]
    float v13; // [esp+34h] [ebp-E4h]
    float distanceFromMin; // [esp+50h] [ebp-C8h]
    float distanceFromMax; // [esp+54h] [ebp-C4h]
    int i; // [esp+60h] [ebp-B8h]
    bool isMax; // [esp+67h] [ebp-B1h]
    float smallestDistance; // [esp+68h] [ebp-B0h]
    int index; // [esp+6Ch] [ebp-ACh]
    float vehicleInvQuat[4]; // [esp+70h] [ebp-A8h] BYREF
    float bbMins[3]; // [esp+80h] [ebp-98h] BYREF
    float vehicleAxis[3][3]; // [esp+8Ch] [ebp-8Ch] BYREF
    bool wasOutside; // [esp+B3h] [ebp-65h]
    float bbMaxs[3]; // [esp+B4h] [ebp-64h] BYREF
    float playerOrigin[3]; // [esp+C0h] [ebp-58h]
    float vehicleInvAxis[3][3]; // [esp+CCh] [ebp-4Ch] BYREF
    float localPlayerOrigin[3]; // [esp+F0h] [ebp-28h]
    float vehicleQuat[4]; // [esp+FCh] [ebp-1Ch] BYREF
    float clampedLocalOrigin[3]; // [esp+10Ch] [ebp-Ch]

    wasOutside = 1;
    playerOrigin[0] = *point;
    playerOrigin[1] = point[1];
    playerOrigin[2] = point[2];
    VEH_GetBoundingBoxFromCollmap(vehicle, bbMins, bbMaxs);
    playerOrigin[0] = playerOrigin[0] - vehicle->r.currentOrigin[0];
    playerOrigin[1] = playerOrigin[1] - vehicle->r.currentOrigin[1];
    playerOrigin[2] = playerOrigin[2] - vehicle->r.currentOrigin[2];
    AnglesToQuat(vehicle->r.currentAngles, vehicleQuat);
    vehicleInvQuat[0] = -vehicleQuat[0];
    vehicleInvQuat[1] = -vehicleQuat[1];
    vehicleInvQuat[2] = -vehicleQuat[2];
    vehicleInvQuat[3] = vehicleQuat[3];
    QuatToAxis(vehicleInvQuat, vehicleInvAxis);
    localPlayerOrigin[0] = (float)(playerOrigin[1] * vehicleInvAxis[1][0])
                                             + (float)(playerOrigin[0] * vehicleInvAxis[0][0]);
    localPlayerOrigin[1] = (float)(playerOrigin[1] * vehicleInvAxis[1][1])
                                             + (float)(playerOrigin[0] * vehicleInvAxis[0][1]);
    localPlayerOrigin[2] = (float)(playerOrigin[1] * vehicleInvAxis[1][2])
                                             + (float)(playerOrigin[0] * vehicleInvAxis[0][2]);
    localPlayerOrigin[0] = (float)(playerOrigin[2] * vehicleInvAxis[2][0]) + localPlayerOrigin[0];
    localPlayerOrigin[1] = (float)(playerOrigin[2] * vehicleInvAxis[2][1]) + localPlayerOrigin[1];
    localPlayerOrigin[2] = (float)(playerOrigin[2] * vehicleInvAxis[2][2]) + localPlayerOrigin[2];
    if ( (float)(localPlayerOrigin[0] - bbMaxs[0]) < 0.0 )
        v13 = localPlayerOrigin[0];
    else
        v13 = bbMaxs[0];
    if ( (float)(bbMins[0] - localPlayerOrigin[0]) < 0.0 )
        v7 = v13;
    else
        v7 = bbMins[0];
    clampedLocalOrigin[0] = v7;
    if ( (float)(localPlayerOrigin[1] - bbMaxs[1]) < 0.0 )
        v12 = localPlayerOrigin[1];
    else
        v12 = bbMaxs[1];
    if ( (float)(bbMins[1] - localPlayerOrigin[1]) < 0.0 )
        v6 = v12;
    else
        v6 = bbMins[1];
    clampedLocalOrigin[1] = v6;
    if ( (float)(localPlayerOrigin[2] - bbMaxs[2]) < 0.0 )
        v11 = localPlayerOrigin[2];
    else
        v11 = bbMaxs[2];
    if ( (float)(bbMins[2] - localPlayerOrigin[2]) < 0.0 )
        v5 = v11;
    else
        v5 = bbMins[2];
    clampedLocalOrigin[2] = v5;
    if ( clampedLocalOrigin[0] == localPlayerOrigin[0] && clampedLocalOrigin[1] == localPlayerOrigin[1] )
    {
        wasOutside = 0;
        smallestDistance = FLT_MAX;
        index = 0;
        isMax = 1;
        for ( i = 0; i < 2; ++i )
        {
            distanceFromMax = bbMaxs[i] - clampedLocalOrigin[i];
            distanceFromMin = clampedLocalOrigin[i] - bbMins[i];
            if ( distanceFromMin <= distanceFromMax )
                v4 = clampedLocalOrigin[i] - bbMins[i];
            else
                v4 = bbMaxs[i] - clampedLocalOrigin[i];
            if ( smallestDistance > v4 )
            {
                smallestDistance = v4;
                index = i;
                isMax = distanceFromMin > distanceFromMax;
            }
        }
        if ( isMax )
            clampedLocalOrigin[index] = bbMaxs[index];
        else
            clampedLocalOrigin[index] = bbMins[index];
    }
    QuatToAxis(vehicleQuat, vehicleAxis);
    v10 = clampedLocalOrigin[0];
    *out = clampedLocalOrigin[0] * vehicleAxis[0][0];
    out[1] = v10 * vehicleAxis[0][1];
    out[2] = v10 * vehicleAxis[0][2];
    v9 = clampedLocalOrigin[1];
    *out = (float)(clampedLocalOrigin[1] * vehicleAxis[1][0]) + *out;
    out[1] = (float)(v9 * vehicleAxis[1][1]) + out[1];
    out[2] = (float)(v9 * vehicleAxis[1][2]) + out[2];
    v8 = clampedLocalOrigin[2];
    *out = (float)(clampedLocalOrigin[2] * vehicleAxis[2][0]) + *out;
    out[1] = (float)(v8 * vehicleAxis[2][1]) + out[1];
    out[2] = (float)(v8 * vehicleAxis[2][2]) + out[2];
    *out = *out + vehicle->r.currentOrigin[0];
    out[1] = out[1] + vehicle->r.currentOrigin[1];
    out[2] = out[2] + vehicle->r.currentOrigin[2];
    return wasOutside;
}

void __cdecl VEH_GetBoundingBoxFromCollmap(gentity_s *vehicle, float *bbMins, float *bbMaxs)
{
    BrushWrapper *v3; // edx
    float v4; // [esp+0h] [ebp-44h]
    float v5; // [esp+4h] [ebp-40h]
    float *maxs; // [esp+8h] [ebp-3Ch]
    float *mins; // [esp+Ch] [ebp-38h]
    BrushWrapper *brush; // [esp+14h] [ebp-30h]
    int i; // [esp+18h] [ebp-2Ch]
    float brushMins[3]; // [esp+1Ch] [ebp-28h]
    float brushMaxs[3]; // [esp+28h] [ebp-1Ch]
    unsigned int geomIndex; // [esp+34h] [ebp-10h]
    DObj *obj; // [esp+38h] [ebp-Ch]
    XModel *baseModel; // [esp+3Ch] [ebp-8h]
    const PhysGeomList *collmap; // [esp+40h] [ebp-4h]

    obj = Com_GetServerDObj(vehicle->s.number);
    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 2270, 0, "%s", "obj") )
        __debugbreak();
    if ( DObjGetNumModels(obj) <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                    2272,
                    0,
                    "%s",
                    "DObjGetNumModels( obj ) > 0") )
    {
        __debugbreak();
    }
    baseModel = DObjGetModel(obj, 0);
    *bbMins = FLT_MAX;
    bbMins[1] = FLT_MAX;
    bbMins[2] = FLT_MAX;
    *bbMaxs = -FLT_MAX;
    bbMaxs[1] = -FLT_MAX;
    bbMaxs[2] = -FLT_MAX;
    if ( XModelHasCollmap(baseModel) )
    {
        collmap = XModelGetCollmap(baseModel, 0);
        if ( !collmap
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                        2279,
                        0,
                        "%s\n\t(collmap) = %p",
                        "(collmap != 0)",
                        0) )
        {
            __debugbreak();
        }
        for ( geomIndex = 0; geomIndex < collmap->count; ++geomIndex )
        {
            if ( !collmap->geoms
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                            2283,
                            0,
                            "%s",
                            "collmap->geoms") )
            {
                __debugbreak();
            }
            if ( collmap->geoms[geomIndex].brush )
            {
                brush = collmap->geoms[geomIndex].brush;
                brushMins[0] = brush->mins[0];
                brushMins[1] = brush->mins[1];
                brushMins[2] = brush->mins[2];
                v3 = collmap->geoms[geomIndex].brush;
                brushMaxs[0] = v3->maxs[0];
                brushMaxs[1] = v3->maxs[1];
                brushMaxs[2] = v3->maxs[2];
                for ( i = 0; i < 3; ++i )
                {
                    if ( bbMins[i] <= brushMins[i] )
                        v5 = bbMins[i];
                    else
                        v5 = brushMins[i];
                    bbMins[i] = v5;
                    if ( brushMaxs[i] <= bbMaxs[i] )
                        v4 = bbMaxs[i];
                    else
                        v4 = brushMaxs[i];
                    bbMaxs[i] = v4;
                }
            }
        }
    }
    else
    {
        mins = baseModel->mins;
        *bbMins = baseModel->mins[0];
        bbMins[1] = mins[1];
        bbMins[2] = mins[2];
        maxs = baseModel->maxs;
        *bbMaxs = baseModel->maxs[0];
        bbMaxs[1] = maxs[1];
        bbMaxs[2] = maxs[2];
    }
}

void __cdecl VEH_DebugAim(gentity_s *ent, const float *color, int duration)
{
    int boneIndex; // [esp+20h] [ebp-C8h]
    float angles[3]; // [esp+24h] [ebp-C4h] BYREF
    weaponParms wp; // [esp+30h] [ebp-B8h] BYREF
    BulletFireParams v6; // [esp+78h] [ebp-70h] BYREF
    float flashMtx[4][3]; // [esp+B8h] [ebp-30h] BYREF

    Weapon_SetWeaponParamsWeapon(&wp, ent->s.weapon);
    boneIndex = ent->scr_vehicle->boneIndex.flash[0];
    if ( boneIndex < 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 2782, 1, "%s", "boneIndex >= 0") )
    {
        __debugbreak();
    }
    G_DObjGetWorldBoneIndexMatrix(ent, boneIndex, flashMtx);
    //*(_QWORD *)wp.gunForward = *(_QWORD *)&flashMtx[0][0];
    wp.gunForward[0] = flashMtx[0][0];
    wp.gunForward[1] = flashMtx[0][1];
    wp.gunForward[2] = flashMtx[0][2];
    //*(_QWORD *)wp.muzzleTrace = *(_QWORD *)&flashMtx[3][0];
    wp.muzzleTrace[0] = flashMtx[3][0];
    wp.muzzleTrace[1] = flashMtx[3][1];
    wp.muzzleTrace[2] = flashMtx[3][2];
    //*(_QWORD *)wp.forward = *(_QWORD *)&flashMtx[0][0];
    //*(_QWORD *)wp.forward = *(_QWORD *)&flashMtx[0][0];
    wp.forward[0] = flashMtx[0][0];
    wp.forward[1] = flashMtx[0][1];
    wp.forward[2] = flashMtx[0][2];
    AxisToAngles(flashMtx, angles);
    //*(_QWORD *)v6.origStart = *(_QWORD *)wp.muzzleTrace;
    v6.origStart[0] = wp.muzzleTrace[0];
    v6.origStart[1] = wp.muzzleTrace[1];
    v6.origStart[2] = wp.muzzleTrace[2];
    //*(_QWORD *)v6.start = *(_QWORD *)wp.muzzleTrace;
    v6.start[0] = wp.muzzleTrace[0];
    v6.start[1] = wp.muzzleTrace[1];
    v6.start[2] = wp.muzzleTrace[2];
    //*(_QWORD *)wp.right = *(_QWORD *)&flashMtx[1][0];
    wp.right[0] = flashMtx[1][0];
    wp.right[1] = flashMtx[1][1];
    wp.right[2] = flashMtx[1][2];
    //*(_QWORD *)wp.up = *(_QWORD *)&flashMtx[2][0];
    wp.up[0] = flashMtx[2][0];
    wp.up[1] = flashMtx[2][1];
    wp.up[2] = flashMtx[2][2];
    Bullet_Endpos(level.time, wp.weapDef->aiSpread, v6.end, v6.dir, &wp, 8192.0);
    CL_AddDebugLine(v6.start, v6.end, color, 1, duration);
}

static float minSpeedToRotate = 50.0f;
void __cdecl VEH_UpdateVelocityWithRotation(gentity_s *ent)
{
    float forwardSpeedToRotate; // [esp+20h] [ebp-38h]
    float speedFracToRotate; // [esp+24h] [ebp-34h]
    scr_vehicle_s *veh; // [esp+2Ch] [ebp-2Ch]
    float right[3]; // [esp+30h] [ebp-28h] BYREF
    float forward[3]; // [esp+3Ch] [ebp-1Ch] BYREF
    float forwardVelToRotate[3]; // [esp+48h] [ebp-10h]
    float forwardSpeed; // [esp+54h] [ebp-4h]

    veh = ent->scr_vehicle;
    YawVectors(veh->phys.prevAngles[1], forward, right);
    forwardSpeed = (float)((float)(veh->phys.vel[0] * forward[0]) + (float)(veh->phys.vel[1] * forward[1]))
                             + (float)(veh->phys.vel[2] * forward[2]);
    speedFracToRotate = veh->turningAbility;
    if ( forwardSpeed > minSpeedToRotate )
    {
        forwardSpeedToRotate = forwardSpeed * speedFracToRotate;
        forwardVelToRotate[0] = (float)(forwardSpeed * speedFracToRotate) * forward[0];
        forwardVelToRotate[1] = (float)(forwardSpeed * speedFracToRotate) * forward[1];
        forwardVelToRotate[2] = (float)(forwardSpeed * speedFracToRotate) * forward[2];
        veh->phys.vel[0] = veh->phys.vel[0] - forwardVelToRotate[0];
        veh->phys.vel[1] = veh->phys.vel[1] - forwardVelToRotate[1];
        veh->phys.vel[2] = veh->phys.vel[2] - forwardVelToRotate[2];
        YawVectors(veh->phys.angles[1], forward, right);
        forwardVelToRotate[0] = forwardSpeedToRotate * forward[0];
        forwardVelToRotate[1] = forwardSpeedToRotate * forward[1];
        forwardVelToRotate[2] = forwardSpeedToRotate * forward[2];
        veh->phys.vel[0] = veh->phys.vel[0] + (float)(forwardSpeedToRotate * forward[0]);
        veh->phys.vel[1] = veh->phys.vel[1] + forwardVelToRotate[1];
        veh->phys.vel[2] = veh->phys.vel[2] + forwardVelToRotate[2];
    }
}

void __cdecl VEH_GetNewSpeedAndAccel(
                scr_vehicle_s *veh,
                float dt,
                int hovering,
                float speedScale,
                float *newSpeed,
                float *accelMax)
{
    const vehicle_info_t *info; // [esp+10h] [ebp-10h]
    float decel; // [esp+14h] [ebp-Ch]
    float speed; // [esp+18h] [ebp-8h]
    float speeda; // [esp+18h] [ebp-8h]
    float accel; // [esp+1Ch] [ebp-4h]

    if ( !veh && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 5090, 0, "%s", "veh") )
        __debugbreak();
    if ( hovering )
    {
        speed = veh->hover.hoverSpeed;
        accel = veh->hover.hoverAccel;
        decel = accel * 0.5;
    }
    else if ( veh->manualMode == 1 )
    {
        speed = veh->manualSpeed;
        accel = veh->manualAccel;
        decel = veh->manualDecel;
    }
    else
    {
        if ( (veh->flags & 0x100) != 0 )
        {
            info = BG_GetVehicleInfo(veh->infoIdx);
            speed = veh->pathPos.speed;
        }
        else
        {
            info = BG_GetVehicleInfo(veh->infoIdx);
            speed = info->maxSpeed;
        }
        accel = info->accel;
        decel = accel * 0.5;
    }
    speeda = speed * speedScale;
    if ( veh->stopping )
    {
        *newSpeed = VEH_AccelerateSpeed(veh->speed, 0.0, decel, dt);
        *accelMax = decel;
    }
    else
    {
        if ( speeda < 0.0
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 5128, 0, "%s", "speed >= 0.0f") )
        {
            __debugbreak();
        }
        *newSpeed = VEH_AccelerateSpeed(veh->speed, speeda, accel, dt);
        *accelMax = accel;
    }
}

double __cdecl VEH_AccelerateSpeed(float speed, float tgtSpeed, float accel, float dt)
{
    float speeda; // [esp+8h] [ebp+8h]

    if ( tgtSpeed <= speed )
    {
        speeda = speed - (float)(accel * dt);
        if ( tgtSpeed > speeda )
            return tgtSpeed;
    }
    else
    {
        speeda = (float)(accel * dt) + speed;
        if ( speeda > tgtSpeed )
            return tgtSpeed;
    }
    return speeda;
}

void __cdecl VEH_SetLinkAngleClamps(
                const vehicle_info_t *info,
                gentity_s *player,
                int seatIndex,
                float *playerAngles,
                float *vehicleAngles)
{
    float zero_vec[3]; // [esp+1Ch] [ebp-18h] BYREF
    int gunnerIndex; // [esp+28h] [ebp-Ch]
    const WeaponDef *weapDef; // [esp+2Ch] [ebp-8h]
    gclient_s *client; // [esp+30h] [ebp-4h]

    if ( !player->client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 5986, 0, "%s", "player->client") )
    {
        __debugbreak();
    }
    client = player->client;
    client->linkAnglesMinClamp[1] = -180.0f;
    client->linkAnglesMaxClamp[1] = 180.0f;
    client->linkAnglesMinClamp[0] = -180.0f;
    client->linkAnglesMaxClamp[0] = 180.0f;
    G_UpdateViewAngleClamp(player->client, playerAngles);
    if ( seatIndex < 1 || seatIndex > 4 )
    {
        if ( seatIndex )
        {
            if ( (seatIndex < 5 || seatIndex > 10)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                            6038,
                            0,
                            "%s",
                            "VEH_PASSENGER_POSITION(seatIndex)") )
            {
                __debugbreak();
            }
            client->linkAnglesMaxClamp[0] = *(float *)&info->gunnerWeapon[3][16 * seatIndex + 40];
            //LODWORD(client->linkAnglesMinClamp[0]) = *(unsigned int *)&info->gunnerWeapon[3][16 * seatIndex + 36] ^ _mask__NegFloat_;
            (client->linkAnglesMinClamp[0]) = -(*(float*)&info->gunnerWeapon[3][16 * seatIndex + 36]);
            client->linkAnglesMaxClamp[1] = *(float *)&info->gunnerWeapon[3][16 * seatIndex + 28];
            //LODWORD(client->linkAnglesMinClamp[1]) = *(unsigned int *)&info->gunnerWeapon[3][16 * seatIndex + 32] ^ _mask__NegFloat_;
            (client->linkAnglesMinClamp[1]) = -(*(float *)&info->gunnerWeapon[3][16 * seatIndex + 32]);
            client->ps.linkFlags |= 2u;
            G_UpdateViewAngleClamp(client, vehicleAngles);
        }
        else if ( info->turretClampPlayerView )
        {
            client->linkAnglesMaxClamp[0] = info->turretViewLimits.vertSpanDown;
            client->linkAnglesMinClamp[0] = -info->turretViewLimits.vertSpanUp;
            client->linkAnglesMaxClamp[1] = info->turretViewLimits.horizSpanLeft;
            client->linkAnglesMinClamp[1] = -info->turretViewLimits.horizSpanRight;
            G_UpdateViewAngleClamp(client, vehicleAngles);
        }
    }
    else
    {
        gunnerIndex = seatIndex - 1;
        weapDef = BG_GetWeaponDef(*(unsigned __int16 *)&info->gunnerWeapon[3][2 * seatIndex + 62]);
        if ( gVehicleRelativeGunnerAngles )
        {
            memset(zero_vec, 0, sizeof(zero_vec));
            SetClientViewAngle(player, zero_vec);
            client->ps.viewAngleClampBase[0] = (float)(weapDef->bottomArc - weapDef->topArc) / 2.0;
            client->ps.viewAngleClampBase[1] = (float)(weapDef->leftArc - weapDef->rightArc) / 2.0;
            client->ps.viewAngleClampRange[0] = (float)(fabs(weapDef->topArc)
                                                                                                + fabs(weapDef->bottomArc))
                                                                                / 2.0;
            client->ps.viewAngleClampRange[1] = (float)(fabs(weapDef->leftArc)
                                                                                                + fabs(weapDef->rightArc))
                                                                                / 2.0;
        }
        else
        {
            client->linkAnglesMaxClamp[0] = info->gunnerRestAngles[gunnerIndex][0] + weapDef->bottomArc;
            client->linkAnglesMinClamp[0] = info->gunnerRestAngles[gunnerIndex][0] - weapDef->topArc;
            client->linkAnglesMaxClamp[1] = info->gunnerRestAngles[gunnerIndex][1] + weapDef->leftArc;
            client->linkAnglesMinClamp[1] = info->gunnerRestAngles[gunnerIndex][1] - weapDef->rightArc;
            G_UpdateViewAngleClamp(client, vehicleAngles);
        }
    }
}

void __cdecl VEH_SetPlayerVehicle(gentity_s *ent, bool enable)
{
    broad_phase_info *bpi; // eax
    broad_phase_info *v3; // eax
    broad_phase_info *j; // [esp+24h] [ebp-28h]
    broad_phase_info *i; // [esp+38h] [ebp-14h]
    PhysObjUserData *phys_user_data; // [esp+48h] [ebp-4h]

    if ( (!ent || !ent->scr_vehicle || !ent->scr_vehicle->nitrousVehicle)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                    6054,
                    0,
                    "%s",
                    "ent && ent->scr_vehicle && ent->scr_vehicle->nitrousVehicle") )
    {
        __debugbreak();
    }
    Sys_EnterCriticalSection(CRITSECT_PHYSICS);
    phys_user_data = ent->scr_vehicle->nitrousVehicle->m_phys_user_data;
    if ( phys_user_data )
    {
        if ( enable )
        {
            if ( phys_user_data->m_bpb->is_bpi() )
            {
                //bpi = broad_phase_base::get_bpi(phys_user_data->m_bpb);
                bpi = phys_user_data->m_bpb->get_bpi();
                bpi->m_env_collision_flags |= 0x20u;
                bpi->m_my_collision_type_flags |= 0x80u;
            }
            else
            {
                for ( i = phys_user_data->m_bpb->get_bpg()->m_list_bpi_head;
                            i;
                            i = (broad_phase_info *)i->m_list_bpb_next )
                {
                    i->m_env_collision_flags |= 0x20u;
                    i->m_my_collision_type_flags |= 0x80u;
                }
            }
        }
        else if ( phys_user_data->m_bpb->is_bpi() )
        {
            //v3 = broad_phase_base::get_bpi(phys_user_data->m_bpb);
            v3 = phys_user_data->m_bpb->get_bpi();
            v3->m_env_collision_flags &= ~0x20u;
            v3->m_my_collision_type_flags &= ~0x80u;
        }
        else
        {
            //for ( j = broad_phase_base::get_bpg(phys_user_data->m_bpb)->m_list_bpi_head;
            for ( j = phys_user_data->m_bpb->get_bpg()->m_list_bpi_head;
                        j;
                        j = (broad_phase_info *)j->m_list_bpb_next )
            {
                j->m_env_collision_flags &= ~0x20u;
                j->m_my_collision_type_flags &= ~0x80u;
            }
        }
    }
    Sys_LeaveCriticalSection(CRITSECT_PHYSICS);
}

void __cdecl VEH_LinkPlayer(gentity_s *ent, gentity_s *player, int seatIndex, bool changingSeats)
{
    char *v4; // eax
    char *v5; // eax
    char *v6; // eax
    char *v7; // eax
    int occupant; // [esp+14h] [ebp-7Ch]
    gclient_s *client; // [esp+18h] [ebp-78h]
    float playerAngles[3]; // [esp+1Ch] [ebp-74h] BYREF
    const vehicle_info_t *info; // [esp+28h] [ebp-68h]
    scr_vehicle_s *veh; // [esp+2Ch] [ebp-64h]
    float playerMtx[4][3]; // [esp+30h] [ebp-60h] BYREF
    float barrelMtx[4][3]; // [esp+60h] [ebp-30h] BYREF

    veh = ent->scr_vehicle;
    client = player->client;
    if ( !client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 6126, 0, "%s", "client") )
    {
        __debugbreak();
    }
    info = BG_GetVehicleInfo(veh->infoIdx);
    if ( player->client->lastStand && !info->remoteControl )
    {
        Com_PrintWarning(1, "Attempted to link a player in second chance to a vehicle!\n");
        return;
    }
    if ( VEH_GetSeatOccupantEntity(veh, seatIndex) != player )
    {
        if ( (client->ps.eFlags & 0x10000) == 0 )
        {
            if ( (client->ps.eFlags & 0x4000) != 0 )
                Com_Error(ERR_DROP, "VEH_LinkPlayer: Player is already using a vehicle");
            if ( player->r.ownerNum.isDefined() )
                Com_Error(ERR_DROP, "VEH_LinkPlayer: Player already has an owner");
        }
        if ( seatIndex < 0 )
            Com_Error(ERR_DROP, "VEH_LinkPlayer: No valid seat to attach player too");
        if ( veh->boneIndex.seats[seatIndex] < 0 )
        {
            v4 = SL_ConvertToString(*s_seatTags[seatIndex], SCRIPTINSTANCE_SERVER);
            Com_Error(ERR_DROP, "VEH_LinkPlayer: Trying to use vehicle without a bone [%s]", v4);
        }
        occupant = VEH_GetSeatOccupantEntNum(veh, seatIndex);
        if ( occupant != 1023 && occupant != player->s.number )
        {
            v5 = SL_ConvertToString(*s_seatTags[seatIndex], SCRIPTINSTANCE_SERVER);
            Com_Error(ERR_DROP, "VEH_LinkPlayer: Trying to get in seat [%s] that is already occupied", v5);
        }
        G_DObjGetWorldBoneIndexMatrix(ent, veh->boneIndex.seats[seatIndex], playerMtx);
        if ( seatIndex )
        {
            if ( seatIndex <= 0 || seatIndex > 4 )
                goto LABEL_28;
        }
        else if ( veh->boneIndex.barrel >= 0 )
        {
            G_DObjGetWorldBoneIndexMatrix(ent, veh->boneIndex.barrel, barrelMtx);
            AxisToAngles(barrelMtx, playerAngles);
            goto LABEL_29;
        }
        if ( veh->boneIndex.flash[4 * seatIndex + 2] < 0 )
        {
LABEL_28:
            AxisToAngles(playerMtx, playerAngles);
            goto LABEL_29;
        }
        G_DObjGetWorldBoneIndexMatrix(ent, veh->boneIndex.flash[4 * seatIndex + 2], barrelMtx);
        AxisToAngles(barrelMtx, playerAngles);
LABEL_29:
        if ( info->remoteControl )
        {
            client->ps.linkAngles[0] = client->ps.viewangles[0];
            client->ps.linkAngles[1] = client->ps.viewangles[1];
            client->ps.linkAngles[2] = client->ps.viewangles[2];
        }
        playerAngles[2] = 0.0f;
        playerAngles[0] = AngleNormalize180(playerAngles[0]);
        SetClientViewAngle(player, playerAngles);
        client->ps.linkFlags &= ~1u;
        client->linkAnglesLocked = 0;
        if ( !info->remoteControl && !G_EntLinkToWithOffset(player, ent, *s_seatTags[seatIndex], vec3_origin, vec3_origin) )
        {
            v6 = SL_ConvertToString(*s_seatTags[seatIndex], SCRIPTINSTANCE_SERVER);
            Com_PrintWarning(15, "WARNING: vehicle missing bone '%s'\n", v6);
            if ( !G_EntLinkToWithOffset(player, ent, scr_const.tag_origin, vec3_origin, vec3_origin) )
            {
                v7 = SL_ConvertToString(*s_seatTags[seatIndex], SCRIPTINSTANCE_SERVER);
                Com_Error(ERR_DROP, "VEH_LinkPlayer: Cannot link to vehicle bone [%s]", v7);
            }
        }
        if ( info->type == 6 && seatIndex >= 5 && seatIndex <= 10 )
            client->linkAnglesFrac = 1.0f;
        if ( veh->targetEnt == player->s.number )
        {
            veh->hasTarget = 0;
            veh->targetEnt = 1023;
            //EntHandle::setEnt(&veh->lookAtEnt, 0);
            veh->lookAtEnt.setEnt(0);
        }
        VEH_SetSeatOccupantEntNum(veh, seatIndex, player->s.number);
        if ( !seatIndex )
            veh->flags |= 1u;
        ent->s.lerp.eFlags |= 0x4000u;
        player->s.lerp.u.player.vehicleType = info->type;
        player->s.lerp.u.player.vehicleSeat = seatIndex;
        if (!seatIndex)
        {
            ent->r.ownerNum.setEnt(player);
        }
        //EntHandle::setEnt(&player->r.ownerNum, ent);
        player->r.ownerNum.setEnt(ent);
        player->s.otherEntityNum = ent->s.number;
        client->ps.eFlags |= 0x4000u;
        if ( info->remoteControl )
        {
            ent->s.lerp.eFlags2 |= 0x10000000u;
            client->ps.eFlags2 |= 0x10000000u;
        }
        client->ps.viewlocked_entNum = ent->s.number;
        client->ps.vehicleType = info->type;
        client->ps.vehiclePos = seatIndex;
        Com_Printf(
            14,
            "VEH_LinkPlayer: Player %i %s is linking. Seat %i.\n",
            player->s.number,
            player->client->sess.cs.name,
            seatIndex);
        VEH_SetLinkAngleClamps(info, player, seatIndex, playerAngles, veh->phys.angles);
        if ( ent->scr_vehicle->nitrousVehicle && player->client )
            VEH_SetPlayerVehicle(ent, 1);
        if ( info->remoteControl )
            player->r.currentAngles[1] = client->ps.linkAngles[1];
        if ( !changingSeats )
        {
            Scr_AddInt(seatIndex, SCRIPTINSTANCE_SERVER);
            Scr_AddEntity(ent, SCRIPTINSTANCE_SERVER);
            Scr_Notify(player, scr_const.enter_vehicle, 2u);
            Scr_AddInt(seatIndex, SCRIPTINSTANCE_SERVER);
            Scr_AddEntity(player, SCRIPTINSTANCE_SERVER);
            Scr_Notify(ent, scr_const.enter_vehicle, 2u);
        }
    }
}

double    Scr_Vehicle_DamageScale(
                gentity_s *pSelf,
                gentity_s *pAttacker,
                gentity_s *pInflictor,
                const float *point,
                int mod,
                int weapon,
                int *damageFromUnderneath)
{
    int v9; // xmm0_4
    int integer; // xmm0_4
    long double v11; // [esp-14h] [ebp-9Ch]
    float v12; // [esp-8h] [ebp-90h]
    float pointAdjusted; // [esp+8h] [ebp-80h]
    float pointAdjusted_4; // [esp+Ch] [ebp-7Ch]
    float pointAdjusted_8; // [esp+10h] [ebp-78h]
    float height; // [esp+14h] [ebp-74h]
    int a; // [esp+28h] [ebp-60h]
    int bestAxis; // [esp+2Ch] [ebp-5Ch]
    float bestDot; // [esp+30h] [ebp-58h]
    float vdir[3]; // [esp+34h] [ebp-54h] BYREF
    float axis[3][3]; // [esp+40h] [ebp-48h] BYREF
    float scale; // [esp+64h] [ebp-24h]
    int radius_damage; // [esp+68h] [ebp-20h]
    int v24; // [esp+6Ch] [ebp-1Ch]
    float back_angle_limits; // [esp+70h] [ebp-18h]
    vehicle_physic_t *phys; // [esp+74h] [ebp-14h]
    scr_vehicle_s *veh; // [esp+78h] [ebp-10h]
    //_UNKNOWN *v28; // [esp+7Ch] [ebp-Ch]
    //gentity_s *pSelfa; // [esp+80h] [ebp-8h]
    //gentity_s *pInflictora; // [esp+88h] [ebp+0h]
    //
    //v28 = a1;
    //pSelfa = pInflictora;
    veh = pSelf->scr_vehicle;
    phys = &veh->phys;
    //__libm_sse2_cos(v11);
    //back_angle_limits = 0.3490658700466156;
    back_angle_limits = cos(0.3490658700466156);
    if (veh->nitrousVehicle && veh->nitrousVehicle->m_vehicle_info->noDirectionalDamage)
        return 1.0;
    if (mod == 17)
        return 0.0;
    v24 = mod;
    radius_damage = mod == 4 || v24 == 6;
    scale = 1.0f;
    AnglesToAxis(phys->angles, axis);
    if (point)
    {
        vdir[0] = *point - phys->origin[0];
        vdir[1] = point[1] - phys->origin[1];
        vdir[2] = point[2] - phys->origin[2];
    }
    else
    {
        vdir[0] = 1.0f;
        vdir[1] = 0.0f;
        vdir[2] = 0.0f;
    }
    vdir[2] = 0.0f;
    Vec3Normalize(vdir);
    bestDot = 0.0f;
    bestAxis = -1;
    for (a = 0; a < 2; ++a)
    {
        if (bestAxis < 0
            || (fabs((float)((float)(vdir[0] * axis[a][0]) + (float)(vdir[1] * axis[a][1])) + (float)(vdir[2] * axis[a][2]))) > (fabs(bestDot)))
        {
            bestDot = (float)((float)(vdir[0] * axis[a][0]) + (float)(vdir[1] * axis[a][1])) + (float)(vdir[2] * axis[a][2]);
            bestAxis = a;
        }
    }
    if ((unsigned int)bestAxis >= 2
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
            6371,
            0,
            "%s",
            "bestAxis == 0 || bestAxis == 1"))
    {
        __debugbreak();
    }
    if (radius_damage)
    {
        height = 1.0f;
        pointAdjusted_8 = 1.0f;
        pointAdjusted_4 = pSelf->r.maxs[1] * 0.80000001;
        pointAdjusted = pSelf->r.maxs[2] * 0.5;
        v12 = Vec3DistanceSq(point, phys->origin);
        if (mod == 4)
        {
            if ((float)(phys->origin[2] + pointAdjusted) <= (float)(point[2] - 10.0)
                || (float)(pointAdjusted_4 * pointAdjusted_4) <= v12)
            {
                return scale * 1.0;
            }
            else
            {
                *damageFromUnderneath = 1;
                return scale * vehicle_damage_zone_under->current.value * 1.0;
            }
        }
        else
        {
            if (bestAxis)
            {
                if (bestAxis == 1)
                {
                    height = vehicle_damage_zone_side->current.value;
                    if ((float)((float)((float)(vdir[0] * axis[0][0]) + (float)(vdir[1] * axis[0][1]))
                        + (float)(vdir[2] * axis[0][2])) >= 0.0)
                        integer = vehicle_damage_zone_front->current.integer;
                    else
                        integer = vehicle_damage_zone_rear->current.integer;
                    pointAdjusted_8 = *(float *)&integer;
                }
            }
            else
            {
                if (bestDot >= 0.0)
                    v9 = vehicle_damage_zone_front->current.integer;
                else
                    v9 = vehicle_damage_zone_rear->current.integer;
                height = *(float *)&v9;
                pointAdjusted_8 = vehicle_damage_zone_side->current.value;
            }
            return (height * (fabs(bestDot))
                + (1.0 - (fabs(bestDot))) * pointAdjusted_8)
                * scale
                * 0.69999999;
        }
    }
    else if (bestAxis)
    {
        if (bestAxis == 1)
            return scale * vehicle_damage_zone_side->current.value;
        else
            return 0.0;
    }
    else if (bestDot >= 0.0)
    {
        return scale * vehicle_damage_zone_front->current.value;
    }
    else if (back_angle_limits <= (-(bestDot)))
    {
        return scale * vehicle_damage_zone_rear->current.value;
    }
    else
    {
        return scale * vehicle_damage_zone_side->current.value;
    }
}

void __cdecl VEH_UnlinkPlayer(gentity_s *player, bool changingSeats, char *error_msg)
{
    gentity_s *v3; // eax
    gentity_s *v4; // eax
    float playerAngles[3]; // [esp+14h] [ebp-84h] BYREF
    float barrelMtx[4][3]; // [esp+20h] [ebp-78h] BYREF
    int gunnerIndex; // [esp+50h] [ebp-48h]
    gentity_s *occupant; // [esp+54h] [ebp-44h]
    gclient_s *client; // [esp+58h] [ebp-40h]
    vehicle_physic_t *phys; // [esp+5Ch] [ebp-3Ch]
    const vehicle_info_t *info; // [esp+60h] [ebp-38h]
    bool stillOccupied; // [esp+67h] [ebp-31h]
    scr_vehicle_s *veh; // [esp+68h] [ebp-30h]
    float origin[3]; // [esp+6Ch] [ebp-2Ch] BYREF
    float angles[3]; // [esp+78h] [ebp-20h] BYREF
    int playerSeatIndex; // [esp+84h] [ebp-14h]
    gentity_s *ent; // [esp+88h] [ebp-10h]
    int i; // [esp+8Ch] [ebp-Ch]
    int seatIndex; // [esp+90h] [ebp-8h]
    int exitPoint; // [esp+94h] [ebp-4h]

    seatIndex = 0;
    client = player->client;
    if ( !client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 6554, 0, "%s", "client") )
    {
        __debugbreak();
    }
    if ( (client->ps.eFlags & 0x300) == 0 )
    {
        if ( (client->ps.eFlags & 0x4000) == 0 )
            Com_Error(ERR_DROP, "VEH_UnlinkPlayer: Player is not using a vehicle or turret attached to vehicle");
        //if ( !EntHandle::isDefined(&player->r.ownerNum) )
        if ( !player->r.ownerNum.isDefined() )
            Com_Error(
                ERR_DROP,
                "VEH_UnlinkPlayer: Player %i %s doesn',27h,'t has an owner. Seat %i. %s",
                player->s.number,
                player->client->sess.cs.name,
                client->ps.vehiclePos,
                error_msg);
        //ent = EntHandle::ent(&player->r.ownerNum);
        ent = player->r.ownerNum.ent();
        veh = ent->scr_vehicle;
        if ( !veh && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 6575, 0, "%s", "veh") )
            __debugbreak();
        phys = &veh->phys;
        info = BG_GetVehicleInfo(veh->infoIdx);
        playerSeatIndex = G_GetVehicleSeatPlayerOccupies(ent, player);
        if ( info->type != 3 && playerSeatIndex < 0 )
            Com_Error(ERR_DROP, "VEH_UnlinkPlayer: Player is not in a vehicle seat");
        if ( !playerSeatIndex )
        {
            veh->flags &= ~1u;
            //EntHandle::setEnt(&ent->r.ownerNum, 0);
            ent->r.ownerNum.setEnt(0);
        }
        if ( info->remoteControl )
        {
            origin[0] = player->r.currentOrigin[0];
            origin[1] = player->r.currentOrigin[1];
            origin[2] = player->r.currentOrigin[2];
        }
        else
        {
            origin[0] = ent->r.currentOrigin[0];
            origin[1] = ent->r.currentOrigin[1];
            origin[2] = ent->r.currentOrigin[2];
            origin[2] = origin[2] + phys->maxs[2];
            exitPoint = G_GetEntryPointForSeat(ent, playerSeatIndex);
            if ( !VEH_ExitPosOkay(ent, player, exitPoint, origin) )
            {
                for ( i = 0; i < 5 && (i == exitPoint || !VEH_ExitPosOkay(ent, player, i, origin)); ++i )
                    ;
                if ( i == 5 )
                {
                    origin[0] = ent->r.currentOrigin[0];
                    origin[1] = ent->r.currentOrigin[1];
                    origin[2] = ent->r.currentOrigin[2];
                    origin[2] = origin[2] + phys->maxs[2];
                }
            }
        }
        if ( playerSeatIndex >= 0 )
            VEH_SetSeatOccupantEntNum(veh, playerSeatIndex, 0x3FFu);
        stillOccupied = 0;
        //v3 = EntHandle::ent(&player->r.ownerNum);
        v3 = player->r.ownerNum.ent();
        if ( v3 == ent )
        {
            for ( seatIndex = 0; seatIndex < 11 && !stillOccupied; ++seatIndex )
            {
                occupant = VEH_GetSeatOccupantEntity(veh, seatIndex);
                if ( occupant )
                {
                    //v4 = EntHandle::ent(&occupant->r.ownerNum);
                    v4 = occupant->r.ownerNum.ent();
                    if ( v4 != ent
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                                    6638,
                                    0,
                                    "%s",
                                    "occupant->r.ownerNum.ent() == ent") )
                    {
                        __debugbreak();
                    }
                    stillOccupied = 1;
                }
            }
        }
        if ( !stillOccupied )
            ent->s.lerp.eFlags &= ~0x4000u;
        G_EntUnlink(player);
        if ( info->remoteControl )
        {
            angles[0] = player->r.currentAngles[0];
            angles[1] = player->r.currentAngles[1];
            angles[2] = player->r.currentAngles[2];
        }
        else
        {
            angles[0] = client->ps.viewangles[0];
            angles[1] = client->ps.viewangles[1];
            angles[2] = client->ps.viewangles[2];
        }
        angles[2] = 0.0f;
        TeleportPlayer(player, origin, angles);
        client->ps.eFlags &= 0xFFFEBFFF;
        if (!error_msg)
        {
            //MEMORY[0] = 0; // cool random null deref dude
            char *err = NULL;
            *err = 0;
        }
        Com_Printf(
            14,
            "VEH_UnlinkPlayer: Player %i %s is unlinking. Seat %i. %s\n",
            player->s.number,
            player->client->sess.cs.name,
            playerSeatIndex,
            error_msg);
        player->r.ownerNum.setEnt(0);
        client->ps.viewlocked_entNum = 1023;
        client->ps.eFlags &= ~0x4000u;
        player->s.lerp.eFlags &= ~0x4000u;
        client->ps.eFlags2 &= ~0x10000000u;
        player->s.lerp.eFlags2 &= ~0x10000000u;
        if ( !error_msg || I_stricmp("ClientDisconnect", error_msg) )
        {
            if ( playerSeatIndex >= 0 )
            {
                seatIndex = client->ps.vehiclePos;
                if ( playerSeatIndex != seatIndex
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                                6712,
                                0,
                                "%s",
                                "playerSeatIndex == seatIndex") )
                {
                    __debugbreak();
                }
                if ( seatIndex >= 1 && seatIndex <= 4 )
                {
                    gunnerIndex = seatIndex - 1;
                    if ( veh->boneIndex.flash[4 * seatIndex + 2] >= 0 )
                    {
                        G_DObjGetWorldBoneIndexMatrix(ent, veh->boneIndex.gunnerTags[gunnerIndex].barrel, barrelMtx);
                        AxisToAngles(barrelMtx, playerAngles);
                        playerAngles[2] = 0.0f;
                        SetClientViewAngle(player, playerAngles);
                    }
                }
            }
            if ( veh->nitrousVehicle )
                VEH_SetPlayerVehicle(ent, 0);
            if ( !changingSeats )
            {
                if ( info->remoteControl )
                    SetClientViewAngle(player, client->ps.linkAngles);
                Scr_AddInt(seatIndex, SCRIPTINSTANCE_SERVER);
                Scr_AddEntity(ent, SCRIPTINSTANCE_SERVER);
                Scr_Notify(player, scr_const.exit_vehicle, 2u);
                Scr_AddInt(seatIndex, SCRIPTINSTANCE_SERVER);
                Scr_AddEntity(player, SCRIPTINSTANCE_SERVER);
                Scr_Notify(ent, scr_const.exit_vehicle, 2u);
            }
        }
    }
}

char __cdecl VEH_ExitPosOkay(gentity_s *vehEnt, gentity_s *player, int exitIndex, float *origin)
{
    float v5; // [esp+18h] [ebp-D8h]
    float pushOutVec[3]; // [esp+20h] [ebp-D0h] BYREF
    col_context_t context; // [esp+2Ch] [ebp-C4h] BYREF
    float exitMat[4][3]; // [esp+54h] [ebp-9Ch] BYREF
    float vEnd[3]; // [esp+84h] [ebp-6Ch] BYREF
    trace_t traceResults; // [esp+90h] [ebp-60h] BYREF
    float clampedExitPoint[3]; // [esp+CCh] [ebp-24h] BYREF
    float safeVehicleOrigin[3]; // [esp+D8h] [ebp-18h] BYREF
    float exitPoint[3]; // [esp+E4h] [ebp-Ch] BYREF

    //col_context_t::col_context_t(&context);
    if ( vehEnt->scr_vehicle->boneIndex.entryPoints[exitIndex] < 0 )
        return 0;
    G_DObjGetWorldBoneIndexMatrix(vehEnt, vehEnt->scr_vehicle->boneIndex.entryPoints[exitIndex], exitMat);
    //*(_QWORD *)exitPoint = *(_QWORD *)&exitMat[3][0];
    exitPoint[0] = exitMat[3][0];
    exitPoint[1] = exitMat[3][1];
    exitPoint[2] = exitMat[3][2];
    if ( vehicle_riding->current.enabled )
    {
        *origin = exitPoint[0];
        origin[1] = exitPoint[1];
        origin[2] = exitPoint[2];
        vEnd[0] = exitPoint[0];
        vEnd[1] = exitPoint[1];
        vEnd[2] = exitPoint[2];
        v5 = exitPointOffsetUp;
        *origin = (float)(exitPointOffsetUp * exitMat[2][0]) + *origin;
        origin[1] = (float)(v5 * exitMat[2][1]) + origin[1];
        origin[2] = (float)(v5 * exitMat[2][2]) + origin[2];
        vEnd[0] = *origin;
        vEnd[1] = origin[1];
        vEnd[2] = origin[2] - (float)(exitPointOffsetDown + exitPointOffsetUp);
        G_TraceCapsule(
            &traceResults,
            origin,
            player->r.mins,
            player->r.maxs,
            vEnd,
            player->s.number,
            player->clipmask,
            &context);
        Vec3Lerp(origin, vEnd, traceResults.fraction, origin);
        return 1;
    }
    else
    {
        exitPoint[2] = exitPoint[2] + exitPointOffsetDown;
        if ( !VEH_ClampPointToBoundingBox(vehEnt, exitPoint, clampedExitPoint) )
        {
            pushOutVec[0] = clampedExitPoint[0] - exitPoint[0];
            pushOutVec[1] = clampedExitPoint[1] - exitPoint[1];
            pushOutVec[2] = 0.0f;
            Vec3NormalizeFast(pushOutVec);
            exitPoint[0] = (float)(100.0 * pushOutVec[0]) + exitPoint[0];
            exitPoint[1] = (float)(100.0 * pushOutVec[1]) + exitPoint[1];
            exitPoint[2] = (float)(100.0 * pushOutVec[2]) + exitPoint[2];
        }
        safeVehicleOrigin[0] = vehEnt->r.currentOrigin[0];
        safeVehicleOrigin[1] = vehEnt->r.currentOrigin[1];
        safeVehicleOrigin[2] = vehEnt->r.currentOrigin[2];
        safeVehicleOrigin[2] = safeVehicleOrigin[2] + 30.0;
        G_TraceCapsule(
            &traceResults,
            safeVehicleOrigin,
            player->r.mins,
            player->r.maxs,
            exitPoint,
            vehEnt->s.number,
            player->clipmask,
            &context);
        if ( traceResults.fraction >= 1.0 )
        {
            vEnd[0] = exitPoint[0];
            vEnd[1] = exitPoint[1];
            vEnd[2] = exitPoint[2] - exitPointOffsetDown;
            G_TraceCapsule(
                &traceResults,
                exitPoint,
                player->r.mins,
                player->r.maxs,
                vEnd,
                vehEnt->s.number,
                player->clipmask,
                &context);
            Vec3Lerp(exitPoint, vEnd, traceResults.fraction, origin);
            return 1;
        }
        else
        {
            return 0;
        }
    }
}

void __cdecl G_VehicleFinishedAnimating(gentity_s *player, pmoveVehAnimState_t vehAnimState)
{
    gentity_s *vehicle; // [esp+0h] [ebp-4h]

    //vehicle = EntHandle::ent(&player->r.ownerNum);
    vehicle = player->r.ownerNum.ent();
    if ( vehAnimState == PMOVE_VEH_ANIM_STATE_COMPLETE )
    {
        if ( player->client->ps.vehicleAnimStage < 8 )
        {
            VEH_LinkPlayer(vehicle, player, player->client->ps.vehiclePos, 0);
            player->client->ps.eFlags &= ~0x10000u;
            player->client->ps.vehicleAnimStage = 0;
            player->client->ps.vehicleAnimBoneIndex = 0;
        }
        else
        {
            if ( (player->client->ps.eFlags & 0x4000) == 0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                            6763,
                            0,
                            "%s",
                            "player->client->ps.eFlags & EF_VEHICLE_ACTIVE") )
            {
                __debugbreak();
            }
            if ( !vehicle
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 6764, 0, "%s", "vehicle") )
            {
                __debugbreak();
            }
            VEH_UnlinkPlayer(player, 0, (char*)"G_VehicleFinishedAnimating");
        }
    }
}

int __cdecl VEH_GetNextSeat(const vehicle_info_t *info, int currentSeat)
{
    int i; // [esp+0h] [ebp-4h]
    int ia; // [esp+0h] [ebp-4h]

    for ( i = 0; ; ++i )
    {
        if ( i >= 11 )
            return -1;
        if ( info->seatSwitchOrder[i] == currentSeat )
            break;
    }
    if ( i != 10 && info->seatSwitchOrder[i + 1] >= 0 )
        return info->seatSwitchOrder[i + 1];
    for ( ia = i - 1; ia >= 0; --ia )
    {
        if ( info->seatSwitchOrder[ia] < 0 )
            return info->seatSwitchOrder[ia + 1];
    }
    return info->seatSwitchOrder[0];
}

int __cdecl VEH_NextAvailableSeat(gentity_s *player)
{
    const vehicle_info_t *info; // [esp+4h] [ebp-10h]
    gentity_s *vehicle; // [esp+8h] [ebp-Ch]
    int playerSeatIndex; // [esp+Ch] [ebp-8h]
    int seatIndex; // [esp+10h] [ebp-4h]

    if ( !player
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 6851, 0, "%s", "player") )
    {
        __debugbreak();
    }
    if ( !player->client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 6852, 0, "%s", "player->client") )
    {
        __debugbreak();
    }
    if ( player->client->ps.viewlocked_entNum != 1023 )
    {
        vehicle = &g_entities[player->client->ps.viewlocked_entNum];
        if ( !vehicle
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 6857, 0, "%s", "vehicle") )
        {
            __debugbreak();
        }
        if ( vehicle->scr_vehicle )
        {
            playerSeatIndex = G_GetVehicleSeatPlayerOccupies(vehicle, player);
            info = BG_GetVehicleInfo(vehicle->scr_vehicle->infoIdx);
            seatIndex = VEH_GetNextSeat(info, playerSeatIndex);
            while ( seatIndex >= 0 && seatIndex != playerSeatIndex )
            {
                if ( VEH_IsSeatPresent(vehicle, seatIndex, 0) )
                {
                    if ( !G_IsVehicleSeatOccupied(vehicle, seatIndex) )
                        return seatIndex;
                    seatIndex = VEH_GetNextSeat(info, seatIndex);
                }
            }
        }
    }
    return -1;
}

void __cdecl VEH_SwitchClientToNextSeat(gentity_s *ent, gentity_s *player)
{
    int nextSeat; // [esp+4h] [ebp-4h]

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 6889, 0, "%s", "ent") )
        __debugbreak();
    if ( !player
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 6890, 0, "%s", "player") )
    {
        __debugbreak();
    }
    if ( !player->client
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 6891, 0, "%s", "player->client") )
    {
        __debugbreak();
    }
    if ( vehicle_switch_seat_delay->current.value <= (float)((float)(level.time - player->client->switchSeatTime) / 1000.0) )
    {
        nextSeat = VEH_NextAvailableSeat(player);
        if ( nextSeat >= 0 )
        {
            player->client->switchSeatTime = level.time;
            VEH_SwitchClientToSeat(ent, player, nextSeat);
        }
    }
}

void __cdecl VEH_SwitchClientToSeat(gentity_s *ent, gentity_s *player, int seatIndex)
{
    int oldSeatIndex; // [esp+0h] [ebp-4h]

    oldSeatIndex = G_GetVehicleSeatPlayerOccupies(ent, player);
    if ( (player->client->ps.eFlags & 0x4000) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                    6787,
                    0,
                    "%s",
                    "player->client->ps.eFlags & EF_VEHICLE_ACTIVE") )
    {
        __debugbreak();
    }
    //if ( !EntHandle::isDefined(&player->r.ownerNum)
    if ( !player->r.ownerNum.isDefined()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                    6788,
                    0,
                    "%s",
                    "player->r.ownerNum.isDefined()") )
    {
        __debugbreak();
    }
    VEH_UnlinkPlayer(player, 1, (char*)"VEH_SwitchClientToSeat");
    VEH_LinkPlayer(ent, player, seatIndex, 1);
    Scr_AddInt(seatIndex, SCRIPTINSTANCE_SERVER);
    Scr_AddInt(oldSeatIndex, SCRIPTINSTANCE_SERVER);
    Scr_AddEntity(ent, SCRIPTINSTANCE_SERVER);
    Scr_Notify(player, scr_const.change_seat, 3u);
    Scr_AddInt(seatIndex, SCRIPTINSTANCE_SERVER);
    Scr_AddInt(oldSeatIndex, SCRIPTINSTANCE_SERVER);
    Scr_AddEntity(player, SCRIPTINSTANCE_SERVER);
    Scr_Notify(ent, scr_const.change_seat, 3u);
    if ( !seatIndex )
        Scr_Notify(player, scr_const.vehicle_driver, 0);
}

void __cdecl G_ParseScrVehicleInfo()
{
    bg_numVehicleInfos = 0;
}

void __cdecl VEH_RinitVehiclesUsingInfo(int infoIdx)
{
    int i; // [esp+8h] [ebp-Ch]
    const vehicle_info_t *info; // [esp+Ch] [ebp-8h]
    scr_vehicle_s *veh; // [esp+10h] [ebp-4h]

    info = BG_GetVehicleInfo(infoIdx);
    for ( i = 0; i < 16; ++i )
    {
        veh = &s_vehicles[i];
        if ( veh->entNum != 1023 && veh->infoIdx == infoIdx )
        {
            VEH_InitFromInfo(veh, info);
            SV_SendServerCommand(
                0,
                SV_CMD_RELIABLE,
                "%c %d %d",
                61,
                veh->entNum,
                g_entities[veh->entNum].s.lerp.eFlags2 & 0x2000000);
        }
    }
}

char __cdecl G_VehicleUpdateField(const char *vehicleInfoName, char *keyValue)
{
    const vehicle_info_t *info; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    if ( !vehicleInfoName || !*vehicleInfoName )
        return 0;
    for ( i = 0; i < bg_numVehicleInfos; ++i )
    {
        info = BG_GetVehicleInfo(i);
        if ( !I_stricmp(vehicleInfoName, info->name) )
        {
            ParseConfigStringToStruct(
                (unsigned __int8 *)info,
                s_vehicleFields,
                s_numVehicleFields,
                keyValue,
                25,
                (int (__cdecl *)(unsigned __int8 *, const char *, const int, const int))VEH_ParseSpecificField,
                BG_StringCopy);
            NitrousVehicle::reinit_parms();
            VEH_RinitVehiclesUsingInfo(i);
        }
    }
    return 1;
}

#if 0
void __cdecl G_ReloadScrVehicleInfo()
{
    double v0; // xmm0_8
    long double v1; // [esp+0h] [ebp-4058h]
    char string[64]; // [esp+8h] [ebp-4050h] BYREF
    char *pszBuffer; // [esp+48h] [ebp-4010h]
    unsigned __int8 *pStruct; // [esp+4Ch] [ebp-400Ch]
    char loadBuffer[16384]; // [esp+50h] [ebp-4008h] BYREF
    int index; // [esp+4054h] [ebp-4h]

    for ( index = 0; index < bg_numVehicleInfos; ++index )
    {
        pStruct = (unsigned __int8 *)BG_GetVehicleInfo(index);
        sprintf(string, "vehicles/%s", (const char *)pStruct);
        pszBuffer = Com_LoadInfoString(string, "vehicle file", "VEHICLEFILE", loadBuffer);
        if ( ParseConfigStringToStruct(
                     pStruct,
                     s_vehicleFields,
                     s_numVehicleFields,
                     pszBuffer,
                     25,
                     (int (__cdecl *)(unsigned __int8 *, const char *, const int, const int))VEH_ParseSpecificField,
                     BG_StringCopy) )
        {
            HIDWORD(v1) = 0;
            while ( SHIDWORD(v1) < 4 )
            {
                *(float *)&v1 = (float)(*(float *)&pStruct[4 * HIDWORD(v1) + 2788] * 0.5) * 0.017453292;
                v0 = *(float *)&v1;
                __libm_sse2_cos(v1);
                *(float *)&v0 = v0;
                *(unsigned int *)&pStruct[4 * HIDWORD(v1)++ + 2788] = LODWORD(v0);
            }
        }
    }
    NitrousVehicle::reinit_parms();
}
#endif

// aislop
static constexpr float DEG2RAD = 0.017453292f;

void G_ReloadScrVehicleInfo()
{
    char path[64];
    char loadBuffer[16384];

    for (int index = 0; index < bg_numVehicleInfos; ++index)
    {
        uint8_t *vehicle = (uint8_t *)(BG_GetVehicleInfo(index));
        if (!vehicle)
            continue;

        // First field in struct appears to be vehicle name string
        const char *vehicleName = reinterpret_cast<const char *>(vehicle);

        snprintf(path, sizeof(path), "vehicles/%s", vehicleName);

        char *buffer = Com_LoadInfoString(
            path,
            "vehicle file",
            "VEHICLEFILE",
            loadBuffer
        );

        if (!buffer)
            continue;

        bool parsed = ParseConfigStringToStruct(
            vehicle,
            s_vehicleFields,
            s_numVehicleFields,
            buffer,
            25,
            VEH_ParseSpecificField,
            BG_StringCopy
        );

        if (!parsed)
            continue;

        // Offset 2788 appears to be start of 4 angle values
        float *angles = reinterpret_cast<float *>(vehicle + 2788);

        for (int i = 0; i < 4; ++i)
        {
            // Original logic:
            // value = (value * 0.5) * DEG2RAD
            angles[i] = (angles[i] * 0.5f) * DEG2RAD;

            // The old code called cos() but discarded result.
            // If that was intentional, it would be:
            // angles[i] = std::cos(angles[i]);
        }
    }

    NitrousVehicle::reinit_parms();
}


void __cdecl G_InitScrVehicles()
{
    __int16 i; // [esp+0h] [ebp-4h]
    __int16 ia; // [esp+0h] [ebp-4h]

    for ( i = 0; i < 16; ++i )
    {
        G_VehInitPathPos(&s_vehicles[i].pathPos);
        s_vehicles[i].entNum = 1023;
    }
    for ( ia = 0; ia < 1; ++ia )
    {
        s_pitches[ia] = 0.0f;
        s_rolls[ia] = 0.0f;
    }
    level.vehicles = s_vehicles;
}

void __cdecl G_SetupScrVehicles()
{
    gentity_s *ent; // [esp+4h] [ebp-8h]
    __int16 i; // [esp+8h] [ebp-4h]

    for ( i = 0; i < 16; ++i )
    {
        if ( s_vehicles[i].entNum != 1023 )
        {
            ent = VEH_GetVehicle(s_vehicles[i].entNum);
            VEH_SetupCollmap(ent);
        }
    }
}

void __cdecl VEH_SetupCollmap(gentity_s *ent)
{
    unsigned int v1; // eax
    char *v2; // eax
    unsigned int v3; // eax
    char *v4; // eax
    gentity_s *cmEnt; // [esp+0h] [ebp-4h]

    ent->r.contents = 8462464;
    if ( (ent->spawnflags & 1) != 0 )
        ent->r.contents |= 0x200000u;
    v1 = G_ModelName(ent->model);
    v2 = SL_ConvertToString(v1, SCRIPTINSTANCE_SERVER);
    cmEnt = VEH_GetCollMap(v2);
    if ( cmEnt )
    {
        if ( cmEnt->s.index.brushmodel )
        {
            ent->s.index.brushmodel = cmEnt->s.index.brushmodel;
            SV_SetBrushModel(ent);
            ent->r.contents = 8462464;
            if ( (ent->spawnflags & 1) != 0 )
                ent->r.contents |= 0x200000u;
        }
        else
        {
            v3 = G_ModelName(ent->model);
            v4 = SL_ConvertToString(v3, SCRIPTINSTANCE_SERVER);
            Com_PrintWarning(15, "WARNING: Cannot use empty vehicle collmap for [%s]\n", v4);
        }
    }
}

gentity_s *__cdecl VEH_GetCollMap(const char *modelname)
{
    char *targetname; // [esp+0h] [ebp-Ch]
    gentity_s *ent; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    for ( i = 0; i < level.num_entities; ++i )
    {
        ent = &g_entities[i];
        if ( ent->r.inuse && ent->classname == scr_const.script_vehicle_collmap )
        {
            targetname = SL_ConvertToString(ent->targetname, SCRIPTINSTANCE_SERVER);
            if ( Com_IsLegacyXModelName(targetname) )
                targetname += 7;
            if ( !I_stricmp(targetname, modelname) )
                return &g_entities[i];
        }
    }
    return 0;
}

void __cdecl G_FreeScrVehicles()
{
    __int16 i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < 16; ++i )
        G_VehFreePathPos(&s_vehicles[i].pathPos);
}

void __cdecl G_SpawnVehicle(gentity_s *ent, char *typeName, int load)
{
    const char *v3; // eax
    unsigned int weapIdx; // [esp+2Ch] [ebp-14h]
    const vehicle_info_t *info; // [esp+30h] [ebp-10h]
    scr_vehicle_s *veh; // [esp+34h] [ebp-Ch]
    int infoIdx; // [esp+38h] [ebp-8h] BYREF
    int i; // [esp+3Ch] [ebp-4h]

    veh = 0;
    for ( i = 0; i < 16; ++i )
    {
        veh = &s_vehicles[i];
        if ( veh->entNum == 1023 )
            break;
    }
    if ( i == 16 )
        Com_Error(ERR_DROP, "Hit max vehicle count [%d]", 16);
    infoIdx = VEH_GetVehicleInfoFromName(typeName);
    if ( infoIdx < 0 )
        Com_Error(ERR_DROP, "Can't find info for script vehicle [%s]", typeName);
    VEH_InitModelAndValidateTags(ent, &infoIdx);
    if ( !level.initializing )
    {
        info = BG_GetVehicleInfo(infoIdx);
        weapIdx = G_GetWeaponIndexForName((char*)info->turretWeapon);
        if ( !IsItemRegistered(weapIdx) )
        {
            v3 = va("vehicle '%s' not precached", info->name);
            Scr_Error(v3, 0);
        }
    }
    memset(veh, 0, sizeof(scr_vehicle_s));

    //if ( (_S2_6 & 1) == 0 )
    //{
    //    _S2_6 |= 1u;
    //    colgeom_visitor_t::colgeom_visitor_t(&dummy_0);
    //    dummy_0.__vftable = (colgeom_visitor_inlined_t<200>_vtbl *)&colgeom_visitor_inlined_t<200>::`vftable';
    //    colgeom_visitor_inlined_t<500>::reset(&dummy_0);
    //    atexit(G_SpawnVehicle_::_16_::_dynamic_atexit_destructor_for__dummy__);
    //}

    static colgeom_visitor_inlined_t<200> dummy_0;

    //veh->vehicle_cache.proximity_data.__vftable = dummy_0.__vftable;
    veh->vehicle_cache.proximity_data = dummy_0;

    //colgeom_visitor_inlined_t<500>::reset(&veh->vehicle_cache.proximity_data);
    veh->vehicle_cache.proximity_data.reset();
    for ( i = 0; i < 6; ++i )
    {
        veh->vehicle_cache.hit_indices[i] = -1;
        veh->vehicle_cache.lastOrigin[0] = FLT_MAX;
        veh->vehicle_cache.lastOrigin[1] = FLT_MAX;
        veh->vehicle_cache.lastOrigin[2] = FLT_MAX;
        veh->vehicle_cache.lastAngles[0] = FLT_MAX;
        veh->vehicle_cache.lastAngles[1] = FLT_MAX;
        veh->vehicle_cache.lastAngles[2] = FLT_MAX;
    }
    veh->targetEnt = 1023;
    iassert(!veh->lookAtEnt.isDefined());

    VEH_InitEntity(ent, veh, infoIdx);
    VEH_InitVehicle(ent, veh, infoIdx);
    if ( !load )
        VEH_SetupCollmap(ent);
    G_UpdateVehicleTags(ent);
    if ( (ent->spawnflags & 2) != 0 )
    {
        G_FreeVehicle(ent);
        ent->s.eType = ET_VEHICLE_CORPSE;
    }
}

int __cdecl VEH_GetVehicleInfoFromName(char *name)
{
    const vehicle_info_t *info; // [esp+0h] [ebp-8h]
    const vehicle_info_t *infoa; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]
    int ia; // [esp+4h] [ebp-4h]
    int ib; // [esp+4h] [ebp-4h]
    int ic; // [esp+4h] [ebp-4h]

    if ( !name || !*name )
        return -1;
    for ( i = 0; i < bg_numVehicleInfos; ++i )
    {
        info = BG_GetVehicleInfo(i);
        if ( !I_stricmp(name, info->name) )
            return i;
    }
    ia = G_LoadVehicle(name);
    if ( ia >= 0 )
        return ia;
    Com_PrintWarning(15, "WARNING: couldn't find vehicle info for '%s', attempting to use 'defaultvehicle'.\n", name);
    for ( ib = 0; ib < bg_numVehicleInfos; ++ib )
    {
        infoa = BG_GetVehicleInfo(ib);
        if ( !I_stricmp("defaultvehicle_mp", infoa->name) )
            return ib;
    }
    ic = G_LoadVehicle((char*)"defaultvehicle_mp");
    if ( ic >= 0 )
        return ic;
    Com_Error(ERR_DROP, "Cannot find vehicle info for 'defaultvehicle'. This is a default vehicle that you should have.");
    return -1;
}

#if 0
int __cdecl G_LoadVehicle(char *name)
{
    double v2; // xmm0_8
    int AliasId; // eax
    long double v4; // [esp+0h] [ebp-4070h]
    unsigned __int8 *v5; // [esp+8h] [ebp-4068h]
    char *v6; // [esp+Ch] [ebp-4064h]
    int i; // [esp+14h] [ebp-405Ch]
    char string[68]; // [esp+18h] [ebp-4058h] BYREF
    int j; // [esp+5Ch] [ebp-4014h]
    char *pszBuffer; // [esp+60h] [ebp-4010h]
    unsigned __int8 *dst; // [esp+64h] [ebp-400Ch]
    char loadBuffer[16384]; // [esp+68h] [ebp-4008h] BYREF
    int index; // [esp+406Ch] [ebp-4h]

    if ( !name && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 831, 0, "%s", "name") )
        __debugbreak();
    if ( level.initializing )
    {
        sprintf(string, "vehicles/%s", name);
        pszBuffer = Com_LoadInfoString(string, "vehicle file", "VEHICLEFILE", loadBuffer);
        index = bg_numVehicleInfos;
        dst = (unsigned __int8 *)BG_GetVehicleInfo(bg_numVehicleInfos);
        memset(dst, 0, 0x1DD8u);
        v6 = name;
        v5 = dst;
        HIDWORD(v4) = dst;
        do
        {
            BYTE3(v4) = *v6;
            *v5++ = *v6++;
        }
        while ( BYTE3(v4) );
        if ( ParseConfigStringToStruct(
                     dst,
                     s_vehicleFields,
                     s_numVehicleFields,
                     pszBuffer,
                     25,
                     (int (__cdecl *)(unsigned __int8 *, const char *, const int, const int))VEH_ParseSpecificField,
                     BG_StringCopy) )
        {
            for ( i = 0; i < 4; ++i )
            {
                v2 = (float)((float)(*(float *)&dst[4 * i + 2788] * 0.5) * 0.017453292);
                __libm_sse2_cos(v4);
                *(float *)&v2 = v2;
                *(unsigned int *)&dst[4 * i + 2788] = LODWORD(v2);
            }
            for ( j = 0; j < 19; ++j )
            {
                if ( dst[64 * j + 1168] )
                {
                    AliasId = SND_FindAliasId((char *)&dst[64 * j + 1168]);
                    *(unsigned int *)&dst[4 * j + 2384] = AliasId;
                }
                else
                {
                    *(unsigned int *)&dst[4 * j + 2384] = 0;
                }
            }
            if ( (unsigned int)bg_numVehicleInfos >= 0x20
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                            875,
                            0,
                            "%s\n\t(bg_numVehicleInfos) = %i",
                            "(bg_numVehicleInfos >= 0 && bg_numVehicleInfos < 32)",
                            bg_numVehicleInfos) )
            {
                __debugbreak();
            }
            SV_SetConfigstring(bg_numVehicleInfos + 3148, name);
            ++bg_numVehicleInfos;
            return index;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        Com_PrintError(15, "ERROR: Trying to load vehicle %s inside the game. The vehicle needs to be precached.\n", name);
        return -1;
    }
}
#else // aislop
int __cdecl G_LoadVehicle(char *name)
{
    int index;
    int i, j;
    int aliasId;
    char path[68];
    char loadBuffer[16384];
    char *buffer;
    unsigned char *vehicle;
    float *angles;
    char *soundName;
    int *aliasSlot;

    if (!name)
    {
        iassert(!"G_LoadVehicle: name is NULL");
        return -1;
    }

    if (!level.initializing)
    {
        Com_PrintError(
            15,
            "ERROR: Trying to load vehicle %s inside the game. The vehicle needs to be precached.\n",
            name
        );
        return -1;
    }

    if (bg_numVehicleInfos >= 32)
    {
        iassert(!"bg_numVehicleInfos >= 32");
        return -1;
    }

    sprintf(path, "vehicles/%s", name);

    buffer = Com_LoadInfoString(
        path,
        "vehicle file",
        "VEHICLEFILE",
        loadBuffer
    );

    if (!buffer)
        return -1;

    index = bg_numVehicleInfos;

    vehicle = (unsigned char *)BG_GetVehicleInfo(index);

    memset(vehicle, 0, 0x1DD8);

    /* first field is vehicle name string */
    strcpy((char *)vehicle, name);

    if (!ParseConfigStringToStruct(
        vehicle,
        s_vehicleFields,
        s_numVehicleFields,
        buffer,
        25,
        (int(__cdecl *)(unsigned char *, const char *, const int, const int))VEH_ParseSpecificField,
        BG_StringCopy))
    {
        return -1;
    }

    /* ---- Fixup 1: convert 4 angle values ---- */
    angles = (float *)(vehicle + 2788);

    for (i = 0; i < 4; ++i)
    {
        /* original logic: (value * 0.5) * DEG2RAD */
        angles[i] = (angles[i] * 0.5f) * 0.017453292f;
    }

    /* ---- Fixup 2: resolve 19 sound aliases ---- */
    for (j = 0; j < 19; ++j)
    {
        soundName = (char *)(vehicle + 1168 + (64 * j));
        aliasSlot = (int *)(vehicle + 2384 + (4 * j));

        if (soundName[0])
        {
            aliasId = SND_FindAliasId(soundName);
            *aliasSlot = aliasId;
        }
        else
        {
            *aliasSlot = 0;
        }
    }

    SV_SetConfigstring(index + 3148, name);

    ++bg_numVehicleInfos;

    return index;
}
#endif


void __cdecl VEH_InitModelAndValidateTags(gentity_s *ent, int *infoIdx)
{
    const char *v2; // eax
    unsigned int v3; // eax
    char *v4; // eax
    int defaultInfoIdx; // [esp+18h] [ebp-8h]
    bool isDefault; // [esp+1Fh] [ebp-1h]

    defaultInfoIdx = VEH_GetVehicleInfoFromName((char*)"defaultvehicle_mp");
    isDefault = 0;
    if ( *infoIdx == defaultInfoIdx )
    {
        isDefault = 1;
        G_SetModel(ent, (char *)"defaultvehicle");
        *infoIdx = defaultInfoIdx;
    }
    else if ( ent->model && G_XModelBad(ent->model) )
    {
        isDefault = 1;
        G_OverrideModel(ent->model, (char *)"defaultvehicle");
        if ( G_XModelBad(ent->model) )
        {
            v2 = va(
                         "Tried to set default vehicle model but it does not exist Entity:%d Origin %.1f %.1f %.1f",
                         ent->s.number,
                         ent->r.currentOrigin[0],
                         ent->r.currentOrigin[1],
                         ent->r.currentOrigin[2]);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                            1116,
                            0,
                            "%s\n\t%s",
                            "!G_XModelBad( ent->model )",
                            v2) )
                __debugbreak();
        }
        *infoIdx = defaultInfoIdx;
    }
    ent->s.eType = ET_VEHICLE;
    G_DObjUpdate(ent);
    if ( !VEH_DObjHasRequiredTags(ent, *infoIdx) )
    {
        if ( isDefault )
            Com_Error(ERR_DROP, "ERROR: default vehicle is missing a required tag!");
        v3 = G_ModelName(ent->model);
        v4 = SL_ConvertToString(v3, SCRIPTINSTANCE_SERVER);
        Com_PrintWarning(
            15,
            "WARNING: vehicle '%s' is missing a required tag! switching to default vehicle model and info.\n",
            v4);
        G_SetModel(ent, (char *)"defaultvehicle");
        *infoIdx = defaultInfoIdx;
        G_DObjUpdate(ent);
        if ( !VEH_DObjHasRequiredTags(ent, *infoIdx) )
            Com_Error(ERR_DROP, "ERROR: default vehicle is missing a required tag!");
    }
}

char __cdecl VEH_DObjHasRequiredTags(gentity_s *ent, int infoIdx)
{
    unsigned int v2; // eax
    char *v3; // eax
    unsigned int v5; // eax
    char *v6; // eax
    char *v7; // [esp-4h] [ebp-10h]
    char *v8; // [esp-4h] [ebp-10h]
    const vehicle_info_t *info; // [esp+0h] [ebp-Ch]
    int numWheels; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]
    int ia; // [esp+8h] [ebp-4h]

    info = BG_GetVehicleInfo(infoIdx);
    if ( !info->type || info->type == 2 )
    {
        numWheels = info->type != 0 ? 6 : 4;
        for ( i = 0; i < numWheels; ++i )
        {
            if ( SV_DObjGetBoneIndex(ent, *s_wheelTags[i]) < 0 )
            {
                v7 = SL_ConvertToString(*s_wheelTags[i], SCRIPTINSTANCE_SERVER);
                v2 = G_ModelName(ent->model);
                v3 = SL_ConvertToString(v2, SCRIPTINSTANCE_SERVER);
                Com_PrintWarning(15, "WARNING: vehicle model '%s' is missing '%s'\n", v3, v7);
                return 0;
            }
        }
    }
    if ( !info->type || info->type == 2 || info->type == 4 || info->type == 1 )
    {
        for ( ia = 0; ia < 11; ++ia )
        {
            if ( VEH_IsSeatPresent(ent, ia, info) )
            {
                if ( SV_DObjGetBoneIndex(ent, *s_seatTags[ia]) < 0 )
                {
                    v8 = SL_ConvertToString(*s_seatTags[ia], SCRIPTINSTANCE_SERVER);
                    v5 = G_ModelName(ent->model);
                    v6 = SL_ConvertToString(v5, SCRIPTINSTANCE_SERVER);
                    Com_PrintWarning(15, "WARNING: vehicle model '%s' is missing '%s'\n", v6, v8);
                }
            }
        }
    }
    return 1;
}

void __cdecl G_FreeVehicle(gentity_s *ent)
{
    if ( ent->scr_vehicle->entNum == 1023
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                    7132,
                    0,
                    "%s",
                    "ent->scr_vehicle->entNum != ENTITYNUM_NONE") )
    {
        __debugbreak();
    }
    ent->health = 0;
    //BLOPS_NULLSUB();
    if ( ent->scr_vehicle->nitrousVehicle )
    {
        NitrousVehicle::remove_vehicle(ent->scr_vehicle->nitrousVehicle);
        ent->scr_vehicle->nitrousVehicle = 0;
    }
    Scr_SetString(&ent->scr_vehicle->lookAtText0, 0, SCRIPTINSTANCE_SERVER);
    Scr_SetString(&ent->scr_vehicle->lookAtText1, 0, SCRIPTINSTANCE_SERVER);
    //EntHandle::setEnt(&ent->scr_vehicle->lookAtEnt, 0);
    ent->scr_vehicle->lookAtEnt.setEnt(0);
    ent->handler = 27;
    ent->nextthink = 0;
    ent->takedamage = 0;
    ent->active = 0;
    ent->s.lerp.eFlags = 0;
    ent->s.lerp.pos.trType = 0;
    ent->s.lerp.apos.trType = 0;
    if ( ent->scr_vehicle->moveState == VEH_MOVESTATE_PLANE_ONCURVE )
    {
        cCurveManager::FreeCurve(ent->scr_vehicle->phys.curveID);
        ent->scr_vehicle->phys.curveID = -1;
        ent->scr_vehicle->phys.curveLength = 0.0f;
        ent->scr_vehicle->phys.curveTime = 0.0f;
        ent->scr_vehicle->moveState = VEH_MOVESTATE_PLANE_FREE;
    }
    if ( ent->scr_vehicle->pathPos.customPath )
        VP_FreeCustomPath(ent->scr_vehicle->pathPos.customPath);
    ent->scr_vehicle->entNum = 1023;
    ent->scr_vehicle = 0;
}

void __cdecl G_HideVehicle(gentity_s *ent)
{
    int occupantNum; // [esp+0h] [ebp-8h]
    int seatIndex; // [esp+4h] [ebp-4h]

    ent->nextthink = 0;
    ent->takedamage = 0;
    ent->active = 0;
    //EntHandle::setEnt(&ent->scr_vehicle->lookAtEnt, 0);
    ent->scr_vehicle->lookAtEnt.setEnt(0);
    ent->r.contents = 0;
    if ( ent->scr_vehicle->nitrousVehicle )
    {
        NitrousVehicle::remove_vehicle(ent->scr_vehicle->nitrousVehicle);
        ent->scr_vehicle->nitrousVehicle = 0;
    }
    for ( seatIndex = 0; seatIndex < 11; ++seatIndex )
    {
        if ( VEH_IsSeatPresent(ent, seatIndex, 0) && G_IsVehicleSeatOccupied(ent, seatIndex) )
        {
            occupantNum = VEH_GetSeatOccupantEntNum(ent, seatIndex);
            if ( (unsigned int)occupantNum < 0x20 )
                ent->r.clientMask[occupantNum >> 5] &= ~(1 << (occupantNum & 0x1F));
        }
    }
}

void __cdecl G_MakeVehicleUsable(gentity_s *ent)
{
    int i; // [esp+0h] [ebp-8h]
    bool hasUseTag; // [esp+7h] [ebp-1h]

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 7241, 0, "%s", "ent") )
        __debugbreak();
    ent->spawnflags |= 1u;
    ent->r.contents |= 0x200000u;
    if ( (ent->r.contents & 0x200000) != 0 )
    {
        hasUseTag = 0;
        for ( i = 0; i < 5; ++i )
        {
            if ( ent->scr_vehicle->boneIndex.entryPoints[i] >= 0 )
                hasUseTag = 1;
        }
        if ( !hasUseTag )
            ent->scr_vehicle->boneIndex.entryPoints[0] = SV_DObjGetBoneIndex(ent, scr_const.tag_origin);
        if ( ent->scr_vehicle->boneIndex.seats[0] < 0 )
            ent->scr_vehicle->boneIndex.seats[0] = SV_DObjGetBoneIndex(ent, scr_const.tag_origin);
    }
}

void __cdecl G_UpdateVehicleTags(gentity_s *ent)
{
    int BoneIndex; // eax
    int v2; // eax
    int v3; // eax
    int v4; // eax
    int v5; // eax
    int v6; // eax
    int v7; // eax
    int v8; // eax
    float v9; // [esp+8h] [ebp-6Ch]
    float v10; // [esp+Ch] [ebp-68h]
    const DObj *obj; // [esp+18h] [ebp-5Ch]
    DObjAnimMat animMat; // [esp+1Ch] [ebp-58h] BYREF
    float tagAxis[3][3]; // [esp+3Ch] [ebp-38h] BYREF
    float tagAngles[3]; // [esp+60h] [ebp-14h] BYREF
    scr_vehicle_s *veh; // [esp+6Ch] [ebp-8h]
    int i; // [esp+70h] [ebp-4h]

    if (!ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 7281, 0, "%s", "ent"))
        __debugbreak();
    if (!ent->scr_vehicle
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
            7282,
            0,
            "%s",
            "ent->scr_vehicle"))
    {
        __debugbreak();
    }
    veh = ent->scr_vehicle;
    veh->boneIndex.player = SV_DObjGetBoneIndex(ent, scr_const.tag_player);
    veh->boneIndex.popout = SV_DObjGetBoneIndex(ent, scr_const.tag_popout);
    veh->boneIndex.body = SV_DObjGetBoneIndex(ent, scr_const.tag_body);
    veh->boneIndex.turret = SV_DObjGetBoneIndex(ent, scr_const.tag_turret);
    veh->boneIndex.barrel = SV_DObjGetBoneIndex(ent, scr_const.tag_barrel);
    veh->boneIndex.turret_base = SV_DObjGetBoneIndex(ent, scr_const.tag_turret_base);
    for (i = 0; i < 5; ++i)
        veh->boneIndex.flash[i] = SV_DObjGetBoneIndex(ent, *s_flashTags[i]);
    for (i = 0; i < 4; ++i)
    {
        BoneIndex = SV_DObjGetBoneIndex(ent, *s_gunnerBarrelTags[i]);
        veh->boneIndex.gunnerTags[i].barrel = BoneIndex;
        v2 = SV_DObjGetBoneIndex(ent, *s_gunnerTurretTags[i]);
        veh->boneIndex.gunnerTags[i].turret = v2;
        v3 = SV_DObjGetBoneIndex(ent, *s_gunnerFlashTags[i]);
        veh->boneIndex.gunnerTags[i].flash = v3;
        v4 = SV_DObjGetBoneIndex(ent, *s_gunnerFlashTags[i + 4]);
        veh->boneIndex.gunnerTags[i].flash2 = v4;
        if (veh->boneIndex.gunnerTags[i].flash2 < 0)
            veh->gunnerTurrets[i].flags &= ~2u;
        else
            veh->gunnerTurrets[i].flags |= 2u;
        if (veh->boneIndex.gunnerTags[i].barrel != 254 && veh->boneIndex.gunnerTags[i].barrel > 0)
        {
            obj = Com_GetServerDObj(ent->s.number);
            DObjGetBasePoseMatrix((DObj*)obj, veh->boneIndex.gunnerTags[i].barrel, &animMat);
            QuatToAxis(animMat.quat, tagAxis);
            AxisToAngles(tagAxis, tagAngles);
            bg_vehicleInfos[veh->infoIdx].gunnerRestAngles[i][1] = tagAngles[1];
            bg_vehicleInfos[veh->infoIdx].gunnerRestAngles[i][0] = tagAngles[0];
            v10 = floor(tagAngles[1] * 182.04445 + 0.5);
            ent->s.lerp.u.vehicle.gunnerAngles[i].yaw = (int)v10;
            v9 = floor(tagAngles[0] * 182.04445 + 0.5);
            ent->s.lerp.u.vehicle.gunnerAngles[i].pitch = (int)v9;
        }
    }
    for (i = 0; i < 6; ++i)
    {
        v5 = SV_DObjGetBoneIndex(ent, *s_wheelTags[i]);
        veh->boneIndex.wheel[i] = v5;
    }
    for (i = 0; i < 11; ++i)
    {
        if (s_seatTags[i])
        {
            v6 = SV_DObjGetBoneIndex(ent, *s_seatTags[i]);
            veh->boneIndex.seats[i] = v6;
        }
    }
    for (i = 0; i < 5; ++i)
    {
        v7 = SV_DObjGetBoneIndex(ent, *s_entryPointTags[i]);
        veh->boneIndex.entryPoints[i] = v7;
        if (veh->boneIndex.entryPoints[i] < 0 && s_entryPointOldTags[i])
        {
            v8 = SV_DObjGetBoneIndex(ent, *s_entryPointOldTags[i]);
            veh->boneIndex.entryPoints[i] = v8;
        }
    }
    if ((ent->r.contents & 0x200000) != 0)
        G_MakeVehicleUsable(ent);
}

bool __cdecl G_IsVehicleRemoteControl(int index)
{
    if ( index < 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 7391, 0, "%s", "index >= 0") )
    {
        __debugbreak();
    }
    if ( index >= bg_numVehicleInfos
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                    7392,
                    0,
                    "%s",
                    "index < bg_numVehicleInfos") )
    {
        __debugbreak();
    }
    return BG_GetVehicleInfo(index)->remoteControl != 0;
}

void __cdecl GScr_PrecacheVehicle()
{
    const char *v0; // eax
    const vehicle_info_t *info; // [esp+0h] [ebp-Ch]
    int infoIdx; // [esp+4h] [ebp-8h]
    char *vehInfo; // [esp+8h] [ebp-4h]

    vehInfo = Scr_GetString(0, SCRIPTINSTANCE_SERVER);
    if ( !level.initializing )
        Scr_Error("precacheVehicle must be called before any wait statements in the level script\n", 0);
    infoIdx = VEH_GetVehicleInfoFromName(vehInfo);
    if ( infoIdx < 0 )
    {
        v0 = va("Cannot find vehicle info for [%s]\n", vehInfo);
        Scr_Error(v0, 0);
    }
    info = BG_GetVehicleInfo(infoIdx);
    G_GetWeaponIndexForName((char*)info->turretWeapon);
}

void __cdecl GScr_GetVehicleTreadFXArray()
{
    const char *v0; // eax
    int i; // [esp+0h] [ebp-10h]
    const vehicle_info_t *info; // [esp+4h] [ebp-Ch]
    int infoIdx; // [esp+8h] [ebp-8h]
    char *vehInfo; // [esp+Ch] [ebp-4h]

    vehInfo = Scr_GetString(0, SCRIPTINSTANCE_SERVER);
    infoIdx = VEH_GetVehicleInfoFromName(vehInfo);
    if ( infoIdx < 0 )
    {
        v0 = va("Cannot find vehicle info for [%s]\n", vehInfo);
        Scr_Error(v0, 0);
    }
    info = BG_GetVehicleInfo(infoIdx);
    Scr_MakeArray(SCRIPTINSTANCE_SERVER);
    for ( i = 0; i < 31; ++i )
    {
        Scr_AddString(info->treadFx[i], SCRIPTINSTANCE_SERVER);
        Scr_AddArray(SCRIPTINSTANCE_SERVER);
    }
}

int __cdecl G_IsVehicleOccupied(gentity_s *ent)
{
    int seatIndex; // [esp+0h] [ebp-4h]

    if ( !ent->scr_vehicle )
        return 0;
    for ( seatIndex = 0; seatIndex < 11; ++seatIndex )
    {
        if ( VEH_IsSeatPresent(ent, seatIndex, 0) && G_IsVehicleSeatOccupied(ent, seatIndex) )
            return 1;
    }
    return 0;
}

team_t __cdecl G_GetVehicleOccupantsTeam(gentity_s *vehEnt)
{
    const gentity_s *pOccupant; // [esp+0h] [ebp-Ch]
    int seatIndex; // [esp+4h] [ebp-8h]
    scr_vehicle_s *veh; // [esp+8h] [ebp-4h]

    veh = vehEnt->scr_vehicle;
    if ( veh )
    {
        for ( seatIndex = 0; seatIndex < 11; ++seatIndex )
        {
            pOccupant = VEH_GetSeatOccupantEntity(veh, seatIndex);
            if ( pOccupant && pOccupant->client )
                return pOccupant->client->sess.cs.team;
        }
    }
    return TEAM_BAD;
}

bool __cdecl G_IsVehicleSeatOccupied(gentity_s *ent, int seatIndex)
{
    scr_vehicle_s *veh; // [esp+4h] [ebp-8h]
    const VehicleSeat *seat; // [esp+8h] [ebp-4h]

    veh = ent->scr_vehicle;
    if ( !veh )
        return 0;
    if ( !VEH_IsSeatPresent(ent, seatIndex, 0) )
        Com_Error(ERR_DROP, "G_IsVehicleSeatOccupied() called on invalid seat position");
    seat = VEH_GetSeat(veh, seatIndex);
    return seat && seat->_occupantEntNum != 1023;
}

int __cdecl G_GetVehicleSeatPlayerOccupies(gentity_s *ent, gentity_s *player)
{
    scr_vehicle_s *veh; // [esp+0h] [ebp-8h]
    int seatIndex; // [esp+4h] [ebp-4h]

    veh = ent->scr_vehicle;
    if ( !veh )
        return -1;
    for ( seatIndex = 0; seatIndex < 11; ++seatIndex )
    {
        if ( VEH_IsSeatPresent(ent, seatIndex, 0) && VEH_GetSeatOccupantEntNum(veh, seatIndex) == player->s.number )
            return seatIndex;
    }
    return -1;
}

int __cdecl G_GetVehicleSeatToEnter(gentity_s *ent, gentity_s *player, bool scriptUse, int *entryPoint)
{
    bool foundEntryPoint; // [esp+Fh] [ebp-25h]
    const vehicle_info_t *info; // [esp+10h] [ebp-24h]
    scr_vehicle_s *veh; // [esp+14h] [ebp-20h]
    float bestDist; // [esp+18h] [ebp-1Ch]
    float dist; // [esp+1Ch] [ebp-18h]
    int i; // [esp+20h] [ebp-14h]
    float entryPointPosition[3]; // [esp+24h] [ebp-10h] BYREF
    int seatIndex; // [esp+30h] [ebp-4h]

    if ( player->r.currentOrigin[2] > (float)(ent->r.currentOrigin[2] + ent->r.maxs[2]) )
        return -1;
    veh = ent->scr_vehicle;
    bestDist = 40000.0f;
    seatIndex = -1;
    if ( !veh )
        return -1;
    info = BG_GetVehicleInfo(veh->infoIdx);
    if ( !vehLocationalVehicleSeatEntry->current.enabled || scriptUse )
    {
        for ( seatIndex = 0;
                    seatIndex < 11 && (!VEH_IsSeatPresent(ent, seatIndex, 0) || G_IsVehicleSeatOccupied(ent, seatIndex));
                    ++seatIndex )
        {
            ;
        }
        if ( seatIndex >= 11 )
            return -1;
    }
    if ( scriptUse )
        return seatIndex;
    foundEntryPoint = 0;
    for ( i = 0; i < 5; ++i )
    {
        if ( veh->boneIndex.entryPoints[i] >= 0 )
        {
            G_DObjGetWorldBoneIndexPos(ent, veh->boneIndex.entryPoints[i], entryPointPosition);
            dist = (float)((float)(player->r.currentOrigin[0] - entryPointPosition[0])
                                     * (float)(player->r.currentOrigin[0] - entryPointPosition[0]))
                     + (float)((float)(player->r.currentOrigin[1] - entryPointPosition[1])
                                     * (float)(player->r.currentOrigin[1] - entryPointPosition[1]));
            if ( bestDist > dist && (float)(info->entryPointRadius[i] * info->entryPointRadius[i]) > dist )
            {
                if ( vehLocationalVehicleSeatEntry->current.enabled )
                {
                    if ( !VEH_IsSeatPresent(ent, i, 0) || G_IsVehicleSeatOccupied(ent, i) )
                        continue;
                    seatIndex = i;
                    bestDist = dist;
                }
                if ( entryPoint )
                    *entryPoint = i;
                foundEntryPoint = 1;
            }
        }
    }
    if ( foundEntryPoint )
        return seatIndex;
    else
        return -1;
}

int __cdecl G_GetEntryPointForSeat(gentity_s *ent, int seat)
{
    scr_vehicle_s *veh; // [esp+0h] [ebp-Ch]
    int entryPoint; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    entryPoint = 0;
    veh = ent->scr_vehicle;
    if ( seat <= 5 )
        entryPoint = seat;
    if ( veh->boneIndex.entryPoints[entryPoint] >= 0 )
        return entryPoint;
    for ( i = 0; i < 5; ++i )
    {
        if ( veh->boneIndex.entryPoints[i] >= 0 )
            return i;
    }
    return entryPoint;
}

int __cdecl G_GetPlayerVehicleMantlePoint(gentity_s *ent, gentity_s *player)
{
    float v3; // [esp+0h] [ebp-68h]
    float v4; // [esp+4h] [ebp-64h]
    float v5; // [esp+8h] [ebp-60h]
    float v6; // [esp+Ch] [ebp-5Ch]
    float vecToPlayer[3]; // [esp+28h] [ebp-40h] BYREF
    const vehicle_info_t *info; // [esp+34h] [ebp-34h]
    scr_vehicle_s *veh; // [esp+38h] [ebp-30h]
    float axis[3][3]; // [esp+3Ch] [ebp-2Ch] BYREF
    float forwardDot; // [esp+60h] [ebp-8h]
    float leftDot; // [esp+64h] [ebp-4h]

    veh = ent->scr_vehicle;
    info = BG_GetVehicleInfo(veh->infoIdx);
    AnglesToAxis(ent->r.currentAngles, axis);
    vecToPlayer[0] = player->r.currentOrigin[0] - ent->r.currentOrigin[0];
    vecToPlayer[1] = player->r.currentOrigin[1] - ent->r.currentOrigin[1];
    vecToPlayer[2] = player->r.currentOrigin[2] - ent->r.currentOrigin[2];
    Vec3Normalize(vecToPlayer);
    forwardDot = (float)((float)(vecToPlayer[0] * axis[0][0]) + (float)(vecToPlayer[1] * axis[0][1]))
                         + (float)(vecToPlayer[2] * axis[0][2]);
    leftDot = (float)((float)(vecToPlayer[0] * axis[1][0]) + (float)(vecToPlayer[1] * axis[1][1]))
                    + (float)(vecToPlayer[2] * axis[1][2]);
    if ( (float)(forwardDot - 1.0) < 0.0 )
        v6 = forwardDot;
    else
        v6 = 1.0f;
    if ( (float)(-1.0 - forwardDot) < 0.0 )
        v4 = v6;
    else
        v4 = -1.0f;
    forwardDot = v4;
    if ( (float)(leftDot - 1.0) < 0.0 )
        v5 = leftDot;
    else
        v5 = 1.0f;
    if ( (float)(-1.0 - leftDot) < 0.0 )
        v3 = v5;
    else
        v3 = -1.0f;
    leftDot = v3;
    if ( forwardDot > info->mantleAngles[0] )
        return 0;
    if ( (-(forwardDot)) > info->mantleAngles[1] )
        return 1;
    if ( (-(leftDot)) > info->mantleAngles[3] )
        return 3;
    if ( leftDot <= info->mantleAngles[2] )
        return 4;
    return 2;
}

int __cdecl G_IsVehicleUsable(gentity_s *ent, gentity_s *player)
{
    const vehicle_info_t *info; // [esp+0h] [ebp-24h]
    const WeaponDef *weapDef; // [esp+8h] [ebp-1Ch]
    gentity_s *occupant; // [esp+Ch] [ebp-18h]
    int seat_index; // [esp+10h] [ebp-14h]
    gclient_s *client; // [esp+18h] [ebp-Ch]
    int hintString; // [esp+1Ch] [ebp-8h] BYREF
    int seatIndex; // [esp+20h] [ebp-4h]

    client = player->client;
    hintString = -1;
    if ( !client )
        return hintString;
    if ( (client->ps.eFlags & 0x4000) != 0 )
        return hintString;
    //if ( EntHandle::isDefined(&player->r.ownerNum) )
    if ( player->r.ownerNum.isDefined() )
        return hintString;
    if ( ent->health <= 0 )
        return hintString;
    if ( (ent->r.contents & 0x200000) == 0 )
        return hintString;
    if ( (ent->scr_vehicle->flags & 8) == 0 || player->team && ent->team == player->team )
    {
        if ( VEH_ShouldEjectOccupants(ent) )
            return hintString;
        for ( seat_index = 0; seat_index < 11; ++seat_index )
        {
            occupant = VEH_GetSeatOccupantEntity(ent->scr_vehicle, seat_index);
            if ( occupant
                && occupant->client
                && (occupant->client->sess.cs.team == TEAM_FREE || occupant->client->sess.cs.team != client->sess.cs.team) )
            {
                return hintString;
            }
        }
        seatIndex = G_GetVehicleSeatToEnter(ent, player, 0, 0);
        if ( seatIndex == -1 )
            return hintString;
        if ( client->disallowVehicleUsage )
        {
            G_GetHintStringIndex(&hintString, (char*)"GAME_CANNOTUSEVEHICLE");
            return hintString;
        }
        if ( seatIndex )
        {
            if ( seatIndex >= 1 && seatIndex <= 4 )
            {
                info = BG_GetVehicleInfo(ent->scr_vehicle->infoIdx);
                weapDef = BG_GetWeaponDef(*(unsigned __int16 *)&info->gunnerWeapon[3][2 * seatIndex + 62]);
                if ( *weapDef->szUseHintString )
                    return weapDef->iUseHintStringIndex;
                G_GetHintStringIndex(&hintString, (char *)"PLATFORM_USEVEHICLEGUNNER");
                return hintString;
            }
            if ( seatIndex >= 5 && seatIndex <= 10 )
            {
                G_GetHintStringIndex(&hintString, (char *)"PLATFORM_RIDEVEHICLE");
                return hintString;
            }
        }
        else if ( *BG_GetWeaponDef(ent->s.weapon)->szUseHintString )
        {
            return BG_GetWeaponDef(ent->s.weapon)->iUseHintStringIndex;
        }
        G_GetHintStringIndex(&hintString, (char *)"PLATFORM_USEVEHICLE");
        return hintString;
    }
    if ( G_GetPlayerVehicleMantlePoint(ent, player) != 4 )
        G_GetHintStringIndex(&hintString, (char *)"PLATFORM_MANTLEVEHICLE");
    return hintString;
}

bool __cdecl VEH_ShouldEjectOccupants(gentity_s *ent)
{
    float *p_x; // eax
    bool result; // al
    phys_vec3 *aVelocity; // [esp+1Ch] [ebp-Ch]
    NitrousVehicle *nitrousVeh; // [esp+24h] [ebp-4h]

    result = 0;
    if ( ent->scr_vehicle->nitrousVehicle )
    {
        nitrousVeh = ent->scr_vehicle->nitrousVehicle;
        if ( nitrousVeh->m_phys_user_data )
        {
            if ( nitrousVeh->m_phys_user_data->body )
            {
                if ( nitrousVeh->m_phys_user_data->body->m_mat.z.z < 0.2 )
                {
                    p_x = &nitrousVeh->m_phys_user_data->body->m_t_vel.x;
                    if ( (float)((float)((float)(*p_x * *p_x)
                                                         + (float)(nitrousVeh->m_phys_user_data->body->m_t_vel.y
                                                                         * nitrousVeh->m_phys_user_data->body->m_t_vel.y))
                                         + (float)(p_x[2] * p_x[2])) < 100.0 )
                    {
                        aVelocity = &nitrousVeh->m_phys_user_data->body->m_a_vel;
                        if ( (float)((float)((float)(aVelocity->x * aVelocity->x) + (float)(aVelocity->y * aVelocity->y))
                                             + (float)(aVelocity->z * aVelocity->z)) < 1.0 )
                            return 1;
                    }
                }
            }
        }
    }
    return result;
}

bool __cdecl G_IsVehicleImmune(gentity_s *ent, int mod, char damageFlags, unsigned int weapon)
{
    bool result; // eax
    gclient_s *client; // [esp+4h] [ebp-10h]
    const vehicle_info_t *info; // [esp+8h] [ebp-Ch]

    info = BG_GetVehicleInfo(ent->scr_vehicle->infoIdx);
    //if ( !EntHandle::isDefined(&ent->r.ownerNum) || !EntHandle::ent(&ent->r.ownerNum)->client )
    if ( !ent->r.ownerNum.isDefined() || !ent->r.ownerNum.ent()->client)
        goto LABEL_7;
    //client = EntHandle::ent(&ent->r.ownerNum)->client;
    client = ent->r.ownerNum.ent()->client;
    if ( (client->ps.otherFlags & 1) != 0 )
        return 1;
    if ( (client->flags & 3) != 0 )
        return 1;
LABEL_7:
    switch ( mod )
    {
        case 1:
        case 2:
            if ( info->bulletDamage )
                result = 0;
            else
                result = (damageFlags & 2) == 0 || !info->armorPiercingDamage;
            break;
        case 3:
        case 4:
            if ( BG_GetWeaponDef(weapon)->projExplosion == WEAPPROJEXP_HEAVY )
                result = info->heavyExplosiveDamage == 0;
            else
                result = ((damageFlags & 2) == 0 || !info->armorPiercingDamage) && info->grenadeDamage == 0;
            break;
        case 5:
        case 16:
            result = ((damageFlags & 2) == 0 || !info->armorPiercingDamage) && info->projectileDamage == 0;
            break;
        case 6:
            result = info->projectileSplashDamage == 0;
            break;
        case 15:
            result = 0;
            break;
        case 17:
            result = info->remoteControl == 0;
            break;
        default:
            result = 1;
            break;
    }
    return result;
}

bool __cdecl G_IsPlayerDrivingVehicle(const gentity_s *player)
{
    gclient_s *client; // [esp+0h] [ebp-10h]
    const scr_vehicle_s *veh; // [esp+8h] [ebp-8h]

    if ( !player
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 8004, 0, "%s", "player") )
    {
        __debugbreak();
    }
    client = player->client;
    if ( !client )
        return 0;
    if ( (client->ps.eFlags & 0x4000) == 0 )
        return 0;
    //if ( !EntHandle::isDefined(&player->r.ownerNum) )
    if ( !player->r.ownerNum.isDefined() )
        return 0;
    veh = player->r.ownerNum.ent()->scr_vehicle;
    if ( !veh )
        return 0;
    if ( (veh->flags & 0x82) != 0 )
        return 0;
    return BG_GetVehicleInfo(veh->infoIdx)->type != 5;
}

// local variable allocation has failed, the output may be wrong!
void    VEH_Teleport(gentity_s *pSelf, float *origin, float *angles, float *vel)
{
    float *v5; // [esp-24h] [ebp-130h]
    float *v6; // [esp-20h] [ebp-12Ch]
    float *v7; // [esp-1Ch] [ebp-128h]
    float v8; // [esp-18h] [ebp-124h]
    float v9; // [esp-14h] [ebp-120h]
    phys_vec3 *p_m_a_vel; // [esp-Ch] [ebp-118h]
    phys_vec3 *p_m_t_vel; // [esp-4h] [ebp-110h]
    rigid_body *body; // [esp+5Ch] [ebp-B0h]
    phys_vec3 physNewOrigin; // [esp+70h] [ebp-9Ch] BYREF
    float v14; // [esp+80h] [ebp-8Ch]
    float v15; // [esp+84h] [ebp-88h]
    float v16; // [esp+88h] [ebp-84h]
    phys_vec3 *i; // [esp+8Ch] [ebp-80h]
    phys_vec3 phys_axis[3]; // [esp+90h] [ebp-7Ch] BYREF
    int v19; // [esp+C4h] [ebp-48h]
    float newAxis[3][3]; // [esp+C8h] [ebp-44h] BYREF
    phys_mat44 *mat; // [esp+ECh] [ebp-20h]
    PhysObjUserData *m_phys_user_data; // [esp+F0h] [ebp-1Ch]
    NitrousVehicle *nitrousVeh; // [esp+F4h] [ebp-18h]
    vehicle_physic_t *phys; // [esp+FCh] [ebp-10h]
    //_UNKNOWN *v26; // [esp+100h] [ebp-Ch]
    //gentity_s *pSelfa; // [esp+104h] [ebp-8h]
    //float *origina; // [esp+108h] [ebp-4h] BYREF
    //float *anglesa; // [esp+10Ch] [ebp+0h]

    //v26 = a1;
    //pSelfa = (gentity_s *)anglesa;

    iassert(pSelf->scr_vehicle);
    
    phys = &pSelf->scr_vehicle->phys;
    Sys_EnterCriticalSection(CRITSECT_PHYSICS_UPDATE);
    Sys_EnterCriticalSection(CRITSECT_PHYSICS);
    if (pSelf->scr_vehicle->nitrousVehicle)
    {
        if (pSelf->scr_vehicle->nitrousVehicle->m_phys_user_data)
        {
            nitrousVeh = pSelf->scr_vehicle->nitrousVehicle;
            m_phys_user_data = nitrousVeh->m_phys_user_data;
            mat = &m_phys_user_data->body->m_mat;
            AnglesToAxis(angles, newAxis);
            v19 = 3;
            for (i = phys_axis; --v19 >= 0; ++i)
                ;
            Phys_Vec3ToNitrousVec(newAxis[0], phys_axis);
            Phys_Vec3ToNitrousVec(newAxis[1], &phys_axis[1]);
            Phys_Vec3ToNitrousVec(newAxis[2], &phys_axis[2]);
            mat->x = phys_axis[0];
            mat->y = phys_axis[1];
            mat->z = phys_axis[2];
            //phys_vec3::operator=(&mat->x, phys_axis);
            //phys_vec3::operator=(&mat->y, &phys_axis[1]);
            //phys_vec3::operator=(&mat->z, &phys_axis[2]);
            v16 = *origin;
            v15 = origin[1];
            v14 = origin[2];
            physNewOrigin.x = v16;
            physNewOrigin.y = v15;
            physNewOrigin.z = v14;
            //phys_vec3::operator=(&mat->w, &physNewOrigin);
            mat->w = physNewOrigin;
            if (   (fabs(mat->w.x)) > 100000.0
                || (fabs(mat->w.y)) > 100000.0
                || (fabs(mat->w.z)) > 100000.0)
            {
                phys_exec_debug_callback(0);
            }
            body = nitrousVeh->m_phys_user_data->body;
            body->m_last_position.x = physNewOrigin.x;
            body->m_last_position.y = physNewOrigin.y;
            body->m_last_position.z = physNewOrigin.z;
            if (   (fabs(nitrousVeh->m_phys_user_data->body->m_last_position.x)) > 100000.0
                || (fabs(nitrousVeh->m_phys_user_data->body->m_last_position.y)) > 100000.0
                || (fabs(nitrousVeh->m_phys_user_data->body->m_last_position.z)) > 100000.0)
            {
                phys_exec_debug_callback(nitrousVeh->m_phys_user_data->body);
            }
            p_m_t_vel = &nitrousVeh->m_phys_user_data->body->m_t_vel;
            p_m_a_vel = &nitrousVeh->m_phys_user_data->body->m_a_vel;
            if (vel)
            {
                v9 = vel[1];
                v8 = vel[2];
                p_m_t_vel->x = *vel;
                p_m_t_vel->y = v9;
                p_m_t_vel->z = v8;
            }
            else
            {
                p_m_t_vel->x = 0.0f;
                p_m_t_vel->y = 0.0f;
                p_m_t_vel->z = 0.0f;
            }
            p_m_a_vel->x = 0.0f;
            p_m_a_vel->y = 0.0f;
            p_m_a_vel->z = 0.0f;
        }
    }
    Sys_LeaveCriticalSection(CRITSECT_PHYSICS);
    Sys_LeaveCriticalSection(CRITSECT_PHYSICS_UPDATE);
    phys->origin[0] = *origin;
    phys->origin[1] = origin[1];
    phys->origin[2] = origin[2];
    v7 = phys->angles;
    phys->angles[0] = *angles;
    v7[1] = angles[1];
    v7[2] = angles[2];
    if (vel)
    {
        v6 = phys->vel;
        phys->vel[0] = *vel;
        v6[1] = vel[1];
        v6[2] = vel[2];
    }
    else
    {
        v5 = phys->vel;
        phys->vel[0] = 0.0f;
        v5[1] = 0.0f;
        v5[2] = 0.0f;
    }
    VEH_SetPosition(pSelf, phys->origin, phys->vel, phys->angles);
}

void __cdecl VEH_UpdateNOClip(gentity_s *pSelf)
{
    float newOrigin[3]; // [esp+4h] [ebp-10h] BYREF
    gentity_s *player; // [esp+10h] [ebp-4h]
    int savedregs; // [esp+14h] [ebp+0h] BYREF

    player = VEH_GetSeatOccupantEntity(pSelf, 0);
    if ( player && player->client && player->client->ps.pm_type == 2 )
    {
        if ( pSelf->scr_vehicle->nitrousVehicle )
        {
            newOrigin[0] = player->r.currentOrigin[0];
            newOrigin[1] = player->r.currentOrigin[1];
            newOrigin[2] = player->r.currentOrigin[2] + 70.0;
            VEH_Teleport(pSelf, newOrigin, player->client->ps.viewangles, 0);
        }
    }
}

void __cdecl VEH_UpdateDebug(gentity_s *pSelf)
{
    char *v1; // eax
    float mtx[4][3]; // [esp+38h] [ebp-8Ch] BYREF
    DObj *ServerDObj; // [esp+68h] [ebp-5Ch]
    float end[3]; // [esp+6Ch] [ebp-58h] BYREF
    char *text; // [esp+78h] [ebp-4Ch]
    int flash; // [esp+7Ch] [ebp-48h]
    int i; // [esp+80h] [ebp-44h]
    DObj *obj; // [esp+84h] [ebp-40h]
    const char *tag_name; // [esp+88h] [ebp-3Ch]
    int boneIndex; // [esp+8Ch] [ebp-38h]
    vehicle_physic_t *phys; // [esp+90h] [ebp-34h]
    const vehicle_info_t *info; // [esp+94h] [ebp-30h]
    float org[3]; // [esp+98h] [ebp-2Ch] BYREF
    float entryPointPosition[3]; // [esp+A4h] [ebp-20h] BYREF
    float color[4]; // [esp+B0h] [ebp-14h] BYREF
    int seatIndex; // [esp+C0h] [ebp-4h]

    if ( g_vehicleDebug->current.enabled )
    {
        phys = &pSelf->scr_vehicle->phys;
        info = BG_GetVehicleInfo(pSelf->scr_vehicle->infoIdx);
        color[0] = 1.0f;
        color[1] = 1.0f;
        color[2] = 0.0f;
        color[3] = 1.0f;
        CG_DebugBox(phys->origin, phys->mins, phys->maxs, phys->angles[1], color, 1, 1);
        org[0] = phys->origin[0];
        org[1] = phys->origin[1];
        org[2] = phys->origin[2];
        org[2] = org[2] + 30.0;
        v1 = va(
                     "Ent %d: origin %g %g %g, angles %g %g %g",
                     pSelf->s.number,
                     pSelf->r.currentOrigin[0],
                     pSelf->r.currentOrigin[1],
                     pSelf->r.currentOrigin[2],
                     pSelf->r.currentAngles[0],
                     pSelf->r.currentAngles[1],
                     pSelf->r.currentAngles[2]);
        CL_AddDebugString(org, colorWhite, 0.69999999, v1, 0);
        color[0] = 0.0f;
        color[1] = 1.0f;
        color[2] = 0.0f;
        color[3] = 1.0f;
        for ( seatIndex = 0; seatIndex < 5; ++seatIndex )
        {
            boneIndex = pSelf->scr_vehicle->boneIndex.entryPoints[seatIndex];
            if ( boneIndex >= 0 )
            {
                G_DObjGetWorldBoneIndexPos(pSelf, boneIndex, entryPointPosition);
                G_DebugCircle(entryPointPosition, info->entryPointRadius[seatIndex], color, 1, 1, 0);
                obj = Com_GetServerDObj(pSelf->s.number);
                if ( !obj
                    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 8434, 0, "%s", "obj") )
                {
                    __debugbreak();
                }
                tag_name = DObjGetBoneName(obj, boneIndex);
                CL_AddDebugString(entryPointPosition, colorWhite, 0.69999999, (char *)tag_name, 0);
            }
        }
        if ( pSelf->scr_vehicle->pathPos.customPath )
            VP_DrawCustomPath(pSelf->scr_vehicle->pathPos.customPath);
        pSelf->spawnflags |= 1u;
        pSelf->r.contents |= 0x200000u;
        bg_vehicleInfos[pSelf->scr_vehicle->infoIdx].isDrivable = 1;
        if ( pSelf->scr_vehicle->boneIndex.entryPoints[0] < 0 )
            pSelf->scr_vehicle->boneIndex.entryPoints[0] = 0;
        if ( pSelf->scr_vehicle->boneIndex.seats[0] < 0 )
            pSelf->scr_vehicle->boneIndex.seats[0] = SV_DObjGetBoneIndex(pSelf, scr_const.tag_origin);
        for ( i = 0; i < 4; ++i )
        {
            flash = pSelf->scr_vehicle->boneIndex.gunnerTags[i].flash;
            if ( flash >= 0 )
            {
                G_DObjGetWorldBoneIndexMatrix(pSelf, flash, mtx);
                end[0] = (float)(10.0 * mtx[0][0]) + mtx[3][0];
                end[1] = (float)(10.0 * mtx[0][1]) + mtx[3][1];
                end[2] = (float)(10.0 * mtx[0][2]) + mtx[3][2];
                G_DebugLine(mtx[3], end, color, 1);
                G_DebugStar(mtx[3], color, 0);
                ServerDObj = Com_GetServerDObj(pSelf->s.number);
                if ( !ServerDObj
                    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 8525, 0, "%s", "obj") )
                {
                    __debugbreak();
                }
                text = DObjGetBoneName(ServerDObj, flash);
                CL_AddDebugString(entryPointPosition, colorWhite, 0.69999999, text, 0);
            }
        }
    }
    VEH_UpdateNOClip(pSelf);
}

void __cdecl Scr_Vehicle_Controller(const gentity_s *pSelf, int *partBits)
{
    float v2; // [esp+4h] [ebp-3Ch]
    float v3; // [esp+Ch] [ebp-34h]
    float barrelAngles[3]; // [esp+10h] [ebp-30h] BYREF
    DObj *obj; // [esp+1Ch] [ebp-24h]
    scr_vehicle_s *veh; // [esp+20h] [ebp-20h]
    float bodyAngles[3]; // [esp+24h] [ebp-1Ch] BYREF
    float turretAngles[3]; // [esp+30h] [ebp-10h] BYREF
    int i; // [esp+3Ch] [ebp-4h]

    if ( !pSelf && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 8561, 0, "%s", "pSelf") )
        __debugbreak();
    if ( !pSelf->scr_vehicle
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                    8562,
                    0,
                    "%s",
                    "pSelf->scr_vehicle") )
    {
        __debugbreak();
    }
    veh = pSelf->scr_vehicle;
    obj = Com_GetServerDObj(pSelf->s.number);
    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 8566, 0, "%s", "obj") )
        __debugbreak();
    if ( veh->nitrousVehicle )
    {
        memset(bodyAngles, 0, sizeof(bodyAngles));
    }
    else
    {
        bodyAngles[0] = (float)pSelf->s.lerp.u.vehicle.throttle * 0.0054931641;
        bodyAngles[1] = 0.0f;
        bodyAngles[2] = pSelf->s.lerp.u.turret.gunAngles[1];
    }
    if ( veh->boneIndex.body >= 0 )
        DObjSetLocalBoneIndex(obj, partBits, veh->boneIndex.body, vec3_origin, bodyAngles);
    v3 = (float)pSelf->s.lerp.u.vehicle.gunYaw * 0.0054931641;
    turretAngles[0] = 0.0f;
    turretAngles[1] = v3;
    turretAngles[2] = 0.0f;
    barrelAngles[0] = (float)pSelf->s.lerp.u.vehicle.gunPitch * 0.0054931641;
    barrelAngles[1] = 0.0f;
    barrelAngles[2] = 0.0f;
    if ( veh->boneIndex.turret >= 0 )
        DObjSetLocalBoneIndex(obj, partBits, veh->boneIndex.turret, vec3_origin, turretAngles);
    if ( veh->boneIndex.barrel >= 0 )
        DObjSetLocalBoneIndex(obj, partBits, veh->boneIndex.barrel, vec3_origin, barrelAngles);
    for ( i = 0; i < 4; ++i )
    {
        v2 = (float)pSelf->s.lerp.u.vehicle.gunnerAngles[i].yaw * 0.0054931641;
        turretAngles[0] = 0.0f;
        turretAngles[1] = v2;
        turretAngles[2] = 0.0f;
        barrelAngles[0] = (float)pSelf->s.lerp.u.vehicle.gunnerAngles[i].pitch * 0.0054931641;
        barrelAngles[1] = 0.0f;
        barrelAngles[2] = 0.0f;
        if ( veh->boneIndex.gunnerTags[i].turret >= 0 )
            DObjSetLocalBoneIndex(obj, partBits, veh->boneIndex.gunnerTags[i].turret, vec3_origin, turretAngles);
        if ( veh->boneIndex.gunnerTags[i].barrel >= 0 )
            DObjSetLocalBoneIndex(obj, partBits, veh->boneIndex.gunnerTags[i].barrel, vec3_origin, barrelAngles);
    }
}

void __cdecl Scr_Vehicle_Init(gentity_s *pSelf)
{
    double v1; // st7
    float *prevAngles; // [esp+0h] [ebp-78h]
    float *angles; // [esp+4h] [ebp-74h]
    float *prevOrigin; // [esp+8h] [ebp-70h]
    float *maxs; // [esp+1Ch] [ebp-5Ch]
    float *mins; // [esp+20h] [ebp-58h]
    float *phys_maxs; // [esp+28h] [ebp-50h]
    float *phys_mins; // [esp+2Ch] [ebp-4Ch]
    float diff; // [esp+34h] [ebp-44h]
    float pos[3]; // [esp+38h] [ebp-40h] BYREF
    vehicle_physic_t *phys; // [esp+44h] [ebp-34h]
    const vehicle_info_t *info; // [esp+48h] [ebp-30h]
    DObjAnimMat *mtx; // [esp+4Ch] [ebp-2Ch]
    scr_vehicle_s *veh; // [esp+50h] [ebp-28h]
    float origin[3]; // [esp+54h] [ebp-24h] BYREF
    float maxHeight; // [esp+60h] [ebp-18h]
    float radius; // [esp+64h] [ebp-14h]
    int wheelIndex; // [esp+68h] [ebp-10h]
    float wheelRadius; // [esp+6Ch] [ebp-Ch]
    float minHeight; // [esp+70h] [ebp-8h]
    int wheelCount; // [esp+74h] [ebp-4h]

    veh = pSelf->scr_vehicle;
    phys = &veh->phys;
    info = BG_GetVehicleInfo(veh->infoIdx);
    if ( info->type && info->type != 2 )
    {
        if ( info->type == 6 )
        {
            radius = 0.0f;
            minHeight = 0.0f;
            maxHeight = 0.0f;
            for ( wheelIndex = 0; wheelIndex < 6; ++wheelIndex )
            {
                if ( veh->boneIndex.wheel[wheelIndex] >= 0 )
                {
                    mtx = G_DObjGetLocalBoneIndexMatrix(pSelf, veh->boneIndex.wheel[wheelIndex]);
                    wheelRadius = (float)(mtx->trans[0] * mtx->trans[0]) + (float)(mtx->trans[1] * mtx->trans[1]);
                    if ( minHeight > mtx->trans[2] )
                        minHeight = mtx->trans[2];
                    if ( mtx->trans[2] > maxHeight )
                        maxHeight = mtx->trans[2];
                    if ( wheelRadius > radius )
                        radius = wheelRadius;
                }
            }
            if ( radius > 0.0 )
            {
                radius = (float)(sqrtf(radius) + 1.0) * info->heliCollisionScalar;
                diff = maxHeight - minHeight;
                if ( radius > (float)(maxHeight - minHeight) )
                {
                    maxHeight = (float)(0.5 * (float)(radius - diff)) + maxHeight;
                    minHeight = minHeight - (float)(0.5 * (float)(radius - diff));
                }
                mins = phys->mins;
                phys->mins[0] = -radius;
                //*((unsigned int *)mins + 1) = LODWORD(radius) ^ _mask__NegFloat_;
                mins[1] = -radius;
                mins[2] = minHeight;
                maxs = phys->maxs;
                phys->maxs[0] = radius;
                maxs[1] = radius;
                maxs[2] = maxHeight;
            }
        }
    }
    else
    {
        wheelCount = info->type != 0 ? 6 : 4;
        radius = 0.0f;
        for ( wheelIndex = 0; wheelIndex < wheelCount; ++wheelIndex )
        {
            VEH_GetWheelOrigin(pSelf, wheelIndex, origin, 0);
            wheelRadius = (float)((float)(origin[0] * origin[0]) + (float)(origin[1] * origin[1]))
                                    + (float)(origin[2] * origin[2]);
            if ( wheelRadius > radius )
                radius = wheelRadius;
        }
        if ( radius <= 0.0
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 8641, 0, "%s", "radius > 0.0f") )
        {
            __debugbreak();
        }
        radius = sqrtf(radius) + 1.0;
        phys_mins = phys->mins;
        phys->mins[0] = -radius;
        //*((unsigned int *)phys_mins + 1) = LODWORD(radius) ^ _mask__NegFloat_;
        phys_mins[1] = -radius;
        phys_mins[2] = 0.0f;
        phys_maxs = phys->maxs;
        phys->maxs[0] = radius;
        phys_maxs[1] = radius;
        phys_maxs[2] = radius * 2.0;
    }
    for ( wheelIndex = 0; wheelIndex < 6; ++wheelIndex )
    {
        if ( veh->boneIndex.wheel[wheelIndex] >= 0 )
        {
            G_DObjGetWorldBoneIndexPos(pSelf, veh->boneIndex.wheel[wheelIndex], pos);
            phys->wheelZPos[wheelIndex] = pos[2];
        }
    }
    if ( veh->boneIndex.flash[0] >= 0 && veh->boneIndex.barrel >= 0 )
    {
        G_DObjGetWorldBoneIndexPos(pSelf, veh->boneIndex.flash[0], origin);
        G_DObjGetWorldBoneIndexPos(pSelf, veh->boneIndex.barrel, pos);
        v1 = Vec3Distance(pos, origin);
        veh->turret.barrelOffset = v1;
    }

    if ( !info->type || info->type == 2 )
        VEH_GroundPlant(pSelf, phys, 0);

    VEH_SetPosition(pSelf, phys->origin, phys->vel, phys->angles);

    prevOrigin = phys->prevOrigin;
    phys->prevOrigin[0] = phys->origin[0];
    prevOrigin[1] = phys->origin[1];
    prevOrigin[2] = phys->origin[2];

    prevAngles = phys->prevAngles;
    angles = phys->angles;
    phys->prevAngles[0] = phys->angles[0];
    prevAngles[1] = angles[1];
    prevAngles[2] = angles[2];

    VEH_TouchEntities(pSelf);
    G_DoTouchTriggers(pSelf);
    if ( pSelf->handler != 25
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                    8723,
                    0,
                    "%s",
                    "pSelf->handler == ENT_HANDLER_VEHICLE_INIT") )
    {
        __debugbreak();
    }
    pSelf->handler = 26;
    pSelf->nextthink = level.time + 50;
}

void __cdecl VEH_GetWheelOrigin(gentity_s *ent, int idx, float *origin, float *quat)
{
    char *v4; // eax
    char *v5; // [esp-4h] [ebp-38h]
    const char *v6; // [esp+0h] [ebp-34h]
    float v7; // [esp+4h] [ebp-30h]
    const vehicle_info_t *info; // [esp+18h] [ebp-1Ch]
    const DObj *obj; // [esp+1Ch] [ebp-18h]
    const char *name; // [esp+20h] [ebp-14h]
    DObjAnimMat *mtx; // [esp+28h] [ebp-Ch]
    scr_vehicle_s *veh; // [esp+2Ch] [ebp-8h]

    veh = ent->scr_vehicle;
    if ( veh->boneIndex.wheel[idx] < 0 )
    {
        info = BG_GetVehicleInfo(veh->infoIdx);
        obj = Com_GetServerDObj(ent->s.number);
        name = 0;
        if ( obj )
            name = DObjGetModel(obj, 0)->name;
        if ( name )
            v6 = name;
        else
            v6 = "UNKNOWN";
        v5 = SL_ConvertToString(ent->targetname, SCRIPTINSTANCE_SERVER);
        v4 = SL_ConvertToString(*s_wheelTags[idx], SCRIPTINSTANCE_SERVER);
        Com_Error(ERR_DROP, "Script vehicle [%s] with model [%s] needs [%s] with targetname [%s]", info, v6, v4, v5);
    }
    mtx = G_DObjGetLocalBoneIndexMatrix(ent, veh->boneIndex.wheel[idx]);
    if ( !mtx && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 975, 0, "%s", "mtx") )
        __debugbreak();
    *origin = mtx->trans[0];
    origin[1] = mtx->trans[1];
    origin[2] = mtx->trans[2];
    if ( quat )
    {
        *quat = mtx->quat[0];
        quat[1] = mtx->quat[1];
        quat[2] = mtx->quat[2];
        quat[3] = mtx->quat[3];
    }
    if ( (veh->flags & 0x80) == 0
        && (float)((float)((float)(*origin * *origin) + (float)(origin[1] * origin[1])) + (float)(origin[2] * origin[2])) > (float)(veh->phys.maxs[0] * veh->phys.maxs[0]) )
    {
        Vec3Normalize(origin);
        v7 = veh->phys.maxs[0] - 2.0;
        *origin = v7 * *origin;
        origin[1] = v7 * origin[1];
        origin[2] = v7 * origin[2];
    }
}

void __cdecl VEH_GroundPlant(gentity_s *ent, vehicle_physic_t *phys, int gravity)
{
    float v3; // [esp+28h] [ebp-318h]
    float v4; // [esp+2Ch] [ebp-314h]
    float *v5; // [esp+30h] [ebp-310h]
    float *v6; // [esp+34h] [ebp-30Ch]
    float v7; // [esp+40h] [ebp-300h]
    float v8; // [esp+44h] [ebp-2FCh]
    float v9; // [esp+48h] [ebp-2F8h]
    float v10; // [esp+4Ch] [ebp-2F4h]
    float *v11; // [esp+A8h] [ebp-298h]
    float *v12; // [esp+B0h] [ebp-290h]
    float *v13; // [esp+B4h] [ebp-28Ch]
    float v14; // [esp+C4h] [ebp-27Ch]
    float proj[4][3]; // [esp+C8h] [ebp-278h] BYREF
    float wheelMatrix[4][3]; // [esp+F8h] [ebp-248h] BYREF
    int contents; // [esp+128h] [ebp-218h]
    col_context_t context; // [esp+12Ch] [ebp-214h] BYREF
    float pt1[3]; // [esp+154h] [ebp-1ECh]
    float plane[4]; // [esp+160h] [ebp-1E0h] BYREF
    const vehicle_info_t *info; // [esp+170h] [ebp-1D0h]
    scr_vehicle_s *veh; // [esp+174h] [ebp-1CCh]
    int numWheels; // [esp+178h] [ebp-1C8h]
    float right[3]; // [esp+17Ch] [ebp-1C4h] BYREF
    int contents_mask; // [esp+188h] [ebp-1B8h]
    float mx[3]; // [esp+18Ch] [ebp-1B4h] BYREF
    float trace_points[37]; // [esp+198h] [ebp-1A8h] BYREF
    float forward[3]; // [esp+22Ch] [ebp-114h] BYREF
    float angles[3]; // [esp+238h] [ebp-108h] BYREF
    float expand_vec[3]; // [esp+244h] [ebp-FCh] BYREF
    trace_t trace; // [esp+250h] [ebp-F0h] BYREF
    float hitPos[3]; // [esp+28Ch] [ebp-B4h] BYREF
    float temp[3]; // [esp+298h] [ebp-A8h] BYREF
    float pt2[3]; // [esp+2A4h] [ebp-9Ch]
    float wheelPos[6][3]; // [esp+2B0h] [ebp-90h] BYREF
    int i; // [esp+2F8h] [ebp-48h]
    float mn[3]; // [esp+2FCh] [ebp-44h] BYREF
    vehicle_cache_t *vehicle_cache; // [esp+308h] [ebp-38h]
    float axis[4][3]; // [esp+30Ch] [ebp-34h] BYREF
    float dot; // [esp+33Ch] [ebp-4h]

    //col_context_t::col_context_t(&context);
    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 1984, 0, "%s", "ent") )
        __debugbreak();
    if ( !ent->scr_vehicle
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                    1985,
                    0,
                    "%s",
                    "ent->scr_vehicle") )
    {
        __debugbreak();
    }
    if ( !phys && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 1986, 0, "%s", "phys") )
        __debugbreak();
    veh = ent->scr_vehicle;
    info = BG_GetVehicleInfo(veh->infoIdx);
    if ( info->type
        && info->type != 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                    1991,
                    0,
                    "%s",
                    "(info->type == VEH_WHEELS_4) || (info->type == VEH_TANK)") )
    {
        __debugbreak();
    }
    if ( info->type )
        numWheels = 6;
    else
        numWheels = 4;
    contents = 529;
    if ( (veh->flags & 1) != 0 )
        contents |= 0x10000u;
    v14 = phys->prevOrigin[2];
    //*(_QWORD *)&axis[3][0] = *(_QWORD *)phys->origin;
    axis[3][0] = phys->origin[0];
    axis[3][1] = phys->origin[1];
    axis[3][2] = v14;
    AnglesToAxis(phys->angles, axis);
    mn[0] = FLT_MAX;
    mn[1] = FLT_MAX;
    mn[2] = FLT_MAX;
    mx[0] = -FLT_MAX;
    mx[1] = -FLT_MAX;
    mx[2] = -FLT_MAX;
    for ( i = 0; i < numWheels; ++i )
    {
        VEH_GetWheelOrigin(ent, i, temp, 0);
        MatrixTransformVector43(temp, axis, hitPos);
        v13 = &trace_points[6 * i];
        *v13 = hitPos[0];
        v13[1] = hitPos[1];
        v13[2] = hitPos[2];
        v12 = &trace_points[6 * i + 3];
        *v12 = hitPos[0];
        v12[1] = hitPos[1];
        v12[2] = hitPos[2];
        trace_points[6 * i + 2] = trace_points[6 * i + 2] + 64.0;
        trace_points[6 * i + 5] = trace_points[6 * i + 5] - 256.0;
        Vec3Min(&trace_points[6 * i], mn, mn);
        Vec3Min(&trace_points[6 * i + 3], mn, mn);
        Vec3Max(&trace_points[6 * i], mx, mx);
        Vec3Max(&trace_points[6 * i + 3], mx, mx);
        if ( g_vehicleDebug->current.enabled )
        {
            VEH_DebugBox(hitPos, 4.0, 1.0, 0.0, 0.0);
            VEH_DebugLine(&trace_points[6 * i], &trace_points[6 * i + 3], 0.0, 0.0, 1.0);
        }
    }
    vehicle_cache = &veh->vehicle_cache;
    contents_mask = contents;
    expand_vec[0] = 30.0f;
    expand_vec[1] = 30.0f;
    expand_vec[2] = 30.0f;
    veh->vehicle_cache.proximity_data.update(mn, mx, contents, expand_vec);
    context.prims = vehicle_cache->proximity_data.prims;
    context.nprims = vehicle_cache->proximity_data.nprims;
    for ( i = 0; i < numWheels; ++i )
    {
        if ( vehicle_cache->hit_indices[i] > 0 )
        {
            //*(_QWORD *)trace.normal.vec.v = *(_QWORD *)&vehicle_cache->hit_normals[i][0];
            trace.normal.vec.v[0] = vehicle_cache->hit_normals[i][0];
            trace.normal.vec.v[1] = vehicle_cache->hit_normals[i][1];
            trace.normal.vec.v[2] = vehicle_cache->hit_normals[i][2];
            trace.sflags = vehicle_cache->hit_sflags[i];
        }
        trace_point_vs_env(&trace, &trace_points[6 * i], &trace_points[6 * i + 3], &context, &vehicle_cache->hit_indices[i]);
        if ( vehicle_cache->hit_indices[i] > 0 )
        {
            v11 = vehicle_cache->hit_normals[i];
            //*(_QWORD *)v11 = *(_QWORD *)trace.normal.vec.v;
            v11[0] = trace.normal.vec.v[0];
            v11[1] = trace.normal.vec.v[1];
            v11[2] = trace.normal.vec.v[2];
            vehicle_cache->hit_sflags[i] = trace.sflags;
        }
        if ( trace.fraction >= 1.0 )
        {
            //*(_QWORD *)hitPos = *(_QWORD *)&trace_points[6 * i + 3];
            hitPos[0] = trace_points[6 * i + 3];
            hitPos[1] = trace_points[6 * i + 4];
            hitPos[2] = trace_points[6 * i + 5];
            phys->wheelSurfType[i] = 0;
        }
        else
        {
            Vec3Lerp(&trace_points[6 * i], &trace_points[6 * i + 3], trace.fraction, hitPos);
            phys->wheelSurfType[i] = (unsigned __int8)((int)(0x3F00000
                                                                                                         & trace.sflags) >> 20);
        }
        if ( gravity )
        {
            phys->wheelZVel[i] = phys->wheelZVel[i] - 40.0;
            phys->wheelZPos[i] = (float)(phys->wheelZVel[i] * 0.05f) + phys->wheelZPos[i];
            if ( hitPos[2] > phys->wheelZPos[i] )
            {
                phys->wheelZPos[i] = hitPos[2];
                phys->wheelZVel[i] = 0.0f;
                memcpy(wheelMatrix, axis, 0x24u);
                //*(_QWORD *)&wheelMatrix[3][0] = *(_QWORD *)hitPos;
                wheelMatrix[3][0] = hitPos[0];
                wheelMatrix[3][1] = hitPos[1];
                wheelMatrix[3][2] = hitPos[2];
            }
        }
        else
        {
            phys->wheelZPos[i] = hitPos[2];
            phys->wheelZVel[i] = 0.0f;
        }
        wheelPos[i][0] = hitPos[0];
        wheelPos[i][1] = hitPos[1];
        wheelPos[i][2] = phys->wheelZPos[i];
        if ( g_vehicleDebug->current.enabled )
            VEH_DebugBox(wheelPos[i], 4.0, 0.0, 1.0, 0.0);
    }
    pt1[0] = 0.5 * (float)(wheelPos[1][0] + wheelPos[3][0]);
    pt1[1] = 0.5 * (float)(wheelPos[1][1] + wheelPos[3][1]);
    pt1[2] = 0.5 * (float)(wheelPos[1][2] + wheelPos[3][2]);
    pt2[0] = 0.5 * (float)(wheelPos[0][0] + wheelPos[2][0]);
    pt2[1] = 0.5 * (float)(wheelPos[0][1] + wheelPos[2][1]);
    pt2[2] = 0.5 * (float)(wheelPos[0][2] + wheelPos[2][2]);
    right[0] = pt1[0] - pt2[0];
    right[1] = pt1[1] - pt2[1];
    right[2] = pt1[2] - pt2[2];
    Vec3Normalize(right);
    pt1[0] = 0.5 * (float)(wheelPos[0][0] + wheelPos[1][0]);
    pt1[1] = 0.5 * (float)(wheelPos[0][1] + wheelPos[1][1]);
    pt1[2] = 0.5 * (float)(wheelPos[0][2] + wheelPos[1][2]);
    pt2[0] = 0.5 * (float)(wheelPos[2][0] + wheelPos[3][0]);
    pt2[1] = 0.5 * (float)(wheelPos[2][1] + wheelPos[3][1]);
    pt2[2] = 0.5 * (float)(wheelPos[2][2] + wheelPos[3][2]);
    forward[0] = pt1[0] - pt2[0];
    forward[1] = pt1[1] - pt2[1];
    forward[2] = pt1[2] - pt2[2];
    Vec3Normalize(forward);
    Vec3Cross(right, forward, plane);
    plane[3] = (float)((float)(wheelPos[0][0] * plane[0]) + (float)(wheelPos[0][1] * plane[1]))
                     + (float)(wheelPos[0][2] * plane[2]);
    for ( i = 1; i < numWheels; ++i )
    {
        dot = (float)((float)((float)(plane[0] * wheelPos[i][0]) + (float)(plane[1] * wheelPos[i][1]))
                                + (float)(plane[2] * wheelPos[i][2]))
                - plane[3];
        if ( dot > info->suspensionTravel )
            plane[3] = (float)((float)((float)(wheelPos[i][0] * plane[0]) + (float)(wheelPos[i][1] * plane[1]))
                                             + (float)(wheelPos[i][2] * plane[2]))
                             - info->suspensionTravel;
    }
    Vec3Cross(plane, axis[0], axis[1]);
    Vec3Normalize(axis[1]);
    Vec3Cross(axis[1], plane, axis[0]);
    Vec3Normalize(axis[0]);
    AxisToAngles(axis, angles);
    phys->angles[0] = DiffTrackAngle(angles[0], phys->prevAngles[0], 6.0, 0.05f);
    phys->angles[2] = DiffTrackAngle(angles[2], phys->prevAngles[2], 6.0, 0.05f);
    v9 = phys->angles[0];
    if ( (float)(v9 - 60.0) < 0.0 )
        v10 = phys->angles[0];
    else
        v10 = 60.0f;
    if ( (float)(-60.0 - v9) < 0.0 )
        v4 = v10;
    else
        v4 = -60.0f;
    phys->angles[0] = v4;
    v7 = phys->angles[2];
    if ( (float)(v7 - 60.0) < 0.0 )
        v8 = phys->angles[2];
    else
        v8 = 60.0f;
    if ( (float)(-60.0 - v7) < 0.0 )
        v3 = v8;
    else
        v3 = -60.0f;
    phys->angles[2] = v3;
    if ( ((veh->flags & 0x80) != 0 || (veh->flags & 1) == 0) && plane[2] != 0.0 )
        phys->origin[2] = (-((float)((float)(phys->origin[0] * plane[0]) + (float)(phys->origin[1] * plane[1])) - plane[3])) / plane[2];
    AnglesSubtract(phys->angles, phys->prevAngles, phys->rotVel);
    phys->rotVel[0] = 20.0 * phys->rotVel[0];
    phys->rotVel[1] = 20.0 * phys->rotVel[1];
    phys->rotVel[2] = 20.0 * phys->rotVel[2];
    if ( g_vehicleDebug->current.enabled )
    {
        for ( i = 0; i < 4; ++i )
        {
            v5 = proj[i];
            v6 = wheelPos[i];
            *v5 = *v6;
            v5[1] = v6[1];
            v5[2] = v6[2];
            proj[i][2] = (-((float)((float)(proj[i][0] * plane[0]) + (float)(proj[i][1] * plane[1])) - plane[3])) / plane[2];
        }
        VEH_DebugLine(proj[0], proj[1], 1.0, 1.0, 0.0);
        VEH_DebugLine(proj[1], proj[3], 1.0, 1.0, 0.0);
        VEH_DebugLine(proj[3], proj[2], 1.0, 1.0, 0.0);
        VEH_DebugLine(proj[2], proj[0], 1.0, 1.0, 0.0);
    }
}

void __cdecl VEH_DebugLine(const float *start, const float *end, float r, float g, float b)
{
    float color[4]; // [esp+0h] [ebp-10h] BYREF

    color[0] = r;
    color[1] = g;
    color[2] = b;
    color[3] = 1.0f;
    G_DebugLine(start, end, color, 1);
}

void __cdecl VEH_DebugBox(float *pos, float width, float r, float g, float b)
{
    float mins[3]; // [esp+10h] [ebp-28h] BYREF
    float maxs[3]; // [esp+1Ch] [ebp-1Ch] BYREF
    float color[4]; // [esp+28h] [ebp-10h] BYREF

    color[0] = r;
    color[1] = g;
    color[2] = b;
    color[3] = 1.0f;
    //LODWORD(mins[0]) = COERCE_UNSIGNED_INT(width * 0.5) ^ _mask__NegFloat_;
    mins[0] = -(width * 0.5f);
    mins[1] = mins[0];
    mins[2] = mins[0];
    maxs[0] = width * 0.5;
    maxs[1] = width * 0.5;
    maxs[2] = width * 0.5;
    CG_DebugBox(pos, mins, maxs, 0.0, color, 1, 0);
}

static float predictTime = 0.15f;
void __cdecl VEH_TouchEntities(gentity_s *ent)
{
    int contentmask; // [esp+30h] [ebp-1088h]
    int var1084; // [esp+34h] [ebp-1084h]
    float v4; // [esp+40h] [ebp-1078h] BYREF
    float v5; // [esp+44h] [ebp-1074h]
    float v6; // [esp+48h] [ebp-1070h]
    DObj *obj; // [esp+4Ch] [ebp-106Ch]
    scr_vehicle_s *scr_vehicle; // [esp+50h] [ebp-1068h]
    float mins; // [esp+54h] [ebp-1064h] BYREF
    float v10; // [esp+58h] [ebp-1060h]
    float v11; // [esp+5Ch] [ebp-105Ch]
    void(__cdecl * touch)(gentity_s *, gentity_s *, int); // [esp+60h] [ebp-1058h]
    gentity_s *enta; // [esp+64h] [ebp-1054h]
    float out[3]; // [esp+68h] [ebp-1050h] BYREF
    float v3[3]; // [esp+74h] [ebp-1044h] BYREF
    float maxs; // [esp+80h] [ebp-1038h] BYREF
    float v17; // [esp+84h] [ebp-1034h]
    float v18; // [esp+88h] [ebp-1030h]
    void(__cdecl * v19)(gentity_s *, gentity_s *, int); // [esp+8Ch] [ebp-102Ch]
    int entityList[1024]; // [esp+90h] [ebp-1028h] BYREF
    int i; // [esp+1090h] [ebp-28h]
    float v22; // [esp+1094h] [ebp-24h] BYREF
    float v23; // [esp+1098h] [ebp-20h]
    float v24; // [esp+109Ch] [ebp-1Ch]
    float v25; // [esp+10A0h] [ebp-18h]
    float v26; // [esp+10A4h] [ebp-14h]
    float v27; // [esp+10A8h] [ebp-10h]
    float offset[3]; // [esp+10ACh] [ebp-Ch] BYREF

    if (!ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 2649, 0, "%s", "ent"))
        __debugbreak();
    if (!ent->scr_vehicle
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
            2650,
            0,
            "%s",
            "ent->scr_vehicle"))
    {
        __debugbreak();
    }
    scr_vehicle = ent->scr_vehicle;
    BG_GetVehicleInfo(scr_vehicle->infoIdx);
    touch = entityHandlers[ent->handler].touch;
    offset[0] = scr_vehicle->phys.origin[0] - scr_vehicle->phys.prevOrigin[0];
    offset[1] = scr_vehicle->phys.origin[1] - scr_vehicle->phys.prevOrigin[1];
    offset[2] = scr_vehicle->phys.origin[2] - scr_vehicle->phys.prevOrigin[2];
    AnglesSubtract(scr_vehicle->phys.angles, scr_vehicle->phys.prevAngles, v3);
    Vec3NormalizeTo(scr_vehicle->phys.vel, out);
    maxs = ent->r.absmax[0];
    v17 = ent->r.absmax[1];
    v18 = ent->r.absmax[2];
    mins = ent->r.absmin[0];
    v10 = ent->r.absmin[1];
    v11 = ent->r.absmin[2];
    ExtendBounds(&mins, &maxs, offset);
    v25 = predictTime * scr_vehicle->phys.vel[0];
    v26 = predictTime * scr_vehicle->phys.vel[1];
    v27 = predictTime * scr_vehicle->phys.vel[2];
    mins = mins + v25;
    v10 = v10 + v26;
    v11 = v11 + v27;
    maxs = maxs + v25;
    v17 = v17 + v26;
    v18 = v18 + v27;
    contentmask = 0x280E091;
    if (vehicle_riding->current.enabled)
        contentmask = 0x80E091;
    var1084 = CM_AreaEntities(&mins, &maxs, entityList, 1024, contentmask);
    for (i = 0; i < var1084; ++i)
    {
        enta = &g_entities[entityList[i]];
        v19 = entityHandlers[enta->handler].touch;
        if (enta->s.number != ent->s.number
            && (enta->s.eType == 1 || enta->s.eType == 6 || enta->s.eType == 4 || enta->s.eType == 17 || enta->s.eType == 14)
            && enta->s.groundEntityNum != ent->s.number)
        {
            if (enta->classname == scr_const.script_model)
            {
                if (!enta->model)
                    continue;
                obj = Com_GetServerDObj(enta->s.number);
                DObjPhysicsGetBounds(obj, &v22, &v4);
                v22 = enta->r.currentOrigin[0] + v22;
                v23 = enta->r.currentOrigin[1] + v23;
                v24 = enta->r.currentOrigin[2] + v24;
                v4 = enta->r.currentOrigin[0] + v4;
                v5 = enta->r.currentOrigin[1] + v5;
                v6 = enta->r.currentOrigin[2] + v6;
            }
            else
            {
                v22 = enta->r.absmin[0];
                v23 = enta->r.absmin[1];
                v24 = enta->r.absmin[2];
                v4 = enta->r.absmax[0];
                v5 = enta->r.absmax[1];
                v6 = enta->r.absmax[2];
            }
            ExpandBoundsToWidth(&v22, &v4);
            v22 = v22 - v25;
            v23 = v23 - v26;
            v24 = v24 - v27;
            v4 = v4 - v25;
            v5 = v5 - v26;
            v6 = v6 - v27;
            if (SV_EntityContact(&v22, &v4, ent))
            {
                if (Scr_IsSystemActive(1u, SCRIPTINSTANCE_SERVER))
                {
                    Scr_AddEntity(ent, SCRIPTINSTANCE_SERVER);
                    Scr_Notify(enta, scr_const.touch, 1u);
                    Scr_AddEntity(enta, SCRIPTINSTANCE_SERVER);
                    Scr_Notify(ent, scr_const.touch, 1u);
                }
                if (v19)
                    v19(enta, ent, 1);
                if (touch)
                    touch(ent, enta, 1);
                if (enta->s.eType == 1 || enta->actor && enta->actor->Physics.bIsAlive)
                    VEH_PushEntity(ent, enta);
            }
        }
    }
    GlassSv_PredictTouch(ent);
}

void __cdecl VEH_PushEntity(gentity_s *ent, gentity_s *target)
{
    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 2550, 0, "%s", "ent") )
        __debugbreak();
    if ( !ent->scr_vehicle
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                    2551,
                    0,
                    "%s",
                    "ent->scr_vehicle") )
    {
        __debugbreak();
    }
    if ( !target
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 2552, 0, "%s", "target") )
    {
        __debugbreak();
    }
    if ( !target->tagInfo && AttachedStickyMissile(ent, target) )
        PushAttachedStickyMissile(ent, target);
}

bool __cdecl AttachedStickyMissile(gentity_s *vehicle, gentity_s *missile)
{
    const WeaponDef *weapDef; // [esp+0h] [ebp-4h]

    if ( !vehicle
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 2209, 0, "%s", "vehicle") )
    {
        __debugbreak();
    }
    if ( !missile
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 2210, 0, "%s", "missile") )
    {
        __debugbreak();
    }
    if ( missile->s.groundEntityNum != vehicle->s.number )
        return 0;
    if ( missile->s.eType != 4 )
        return 0;
    weapDef = BG_GetWeaponDef(missile->s.weapon);
    if ( !weapDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 2218, 0, "%s", "weapDef") )
    {
        __debugbreak();
    }
    return weapDef->stickiness == WEAPSTICKINESS_ALL || weapDef->stickiness == WEAPSTICKINESS_ALL_NO_SENTIENTS;
}

void __cdecl PushAttachedStickyMissile(gentity_s *vehicle, gentity_s *missile)
{
    scr_vehicle_s *scr_vehicle; // edx
    float newVehMat[3][3]; // [esp+18h] [ebp-F4h] BYREF
    vehicle_physic_t *phys; // [esp+3Ch] [ebp-D0h]
    float origin[3]; // [esp+40h] [ebp-CCh] BYREF
    float oldMissileMat[3][3]; // [esp+4Ch] [ebp-C0h] BYREF
    float newMissileMat[3][3]; // [esp+70h] [ebp-9Ch] BYREF
    float oldVehMatInv[3][3]; // [esp+94h] [ebp-78h] BYREF
    float deltaMat[3][3]; // [esp+B8h] [ebp-54h] BYREF
    float oldVehMat[3][3]; // [esp+DCh] [ebp-30h] BYREF
    float relativeOrig[3]; // [esp+100h] [ebp-Ch] BYREF

    if ( !vehicle
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 2237, 0, "%s", "vehicle") )
    {
        __debugbreak();
    }
    if ( !vehicle->scr_vehicle
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                    2238,
                    0,
                    "%s",
                    "vehicle->scr_vehicle") )
    {
        __debugbreak();
    }
    if ( !missile
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 2239, 0, "%s", "missile") )
    {
        __debugbreak();
    }
    scr_vehicle = vehicle->scr_vehicle;
    phys = &scr_vehicle->phys;
    if ( scr_vehicle == (scr_vehicle_s *)-220
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 2242, 0, "%s", "phys") )
    {
        __debugbreak();
    }
    relativeOrig[0] = missile->r.currentOrigin[0] - phys->prevOrigin[0];
    relativeOrig[1] = missile->r.currentOrigin[1] - phys->prevOrigin[1];
    relativeOrig[2] = missile->r.currentOrigin[2] - phys->prevOrigin[2];
    AnglesToAxis(phys->angles, newVehMat);
    AnglesToAxis(phys->prevAngles, oldVehMat);
    MatrixTranspose(oldVehMat, oldVehMatInv);
    MatrixMultiply(oldVehMatInv, newVehMat, deltaMat);
    MatrixTransformVector(relativeOrig, deltaMat, origin);
    origin[0] = origin[0] + phys->origin[0];
    origin[1] = origin[1] + phys->origin[1];
    origin[2] = origin[2] + phys->origin[2];
    missile->r.currentOrigin[0] = origin[0];
    missile->r.currentOrigin[1] = origin[1];
    missile->r.currentOrigin[2] = origin[2];
    missile->s.lerp.pos.trBase[0] = origin[0];
    missile->s.lerp.pos.trBase[1] = origin[1];
    missile->s.lerp.pos.trBase[2] = origin[2];
    AnglesToAxis(missile->r.currentAngles, oldMissileMat);
    MatrixMultiply(oldMissileMat, deltaMat, newMissileMat);
    AxisToAngles(newMissileMat, missile->r.currentAngles);
    missile->s.lerp.apos.trBase[0] = missile->r.currentAngles[0];
    missile->s.lerp.apos.trBase[1] = missile->r.currentAngles[1];
    missile->s.lerp.apos.trBase[2] = missile->r.currentAngles[2];
}

void __cdecl VEH_UpdateHealth(gentity_s *pSelf)
{
    bool maingun_turningpitch; // [esp+5h] [ebp-1Bh]
    bool maingun_turningyaw; // [esp+6h] [ebp-1Ah]
    bool maingun_overheating; // [esp+7h] [ebp-19h]
    float vehTreadLeftHealthPercentage; // [esp+8h] [ebp-18h]
    bool gunner1_turningpitch; // [esp+Ch] [ebp-14h]
    bool gunner1_turningyaw; // [esp+Dh] [ebp-13h]
    bool driver_seat_occupied; // [esp+Eh] [ebp-12h]
    int seat_occupancy_flags; // [esp+10h] [ebp-10h]
    float vehTreadRightHealthPercentage; // [esp+14h] [ebp-Ch]
    float vehHealthPercentage; // [esp+18h] [ebp-8h]
    bool gunner1_overheating; // [esp+1Fh] [ebp-1h]

    if ( !pSelf->maxHealth || pSelf->health > pSelf->maxHealth )
        pSelf->maxHealth = pSelf->health;
    driver_seat_occupied = VEH_GetSeatOccupantEntity(pSelf, 0) != 0;
    seat_occupancy_flags = (VEH_GetSeatOccupantEntity(pSelf, 1) != 0 ? 2 : 0) | driver_seat_occupied;
    vehHealthPercentage = (float)pSelf->health / (float)pSelf->maxHealth;
    vehTreadLeftHealthPercentage = GetPieceHealthPercentage(
                                                                     pSelf->destructible,
                                                                     scr_const.left_tread,
                                                                     vehHealthPercentage);
    vehTreadRightHealthPercentage = GetPieceHealthPercentage(
                                                                        pSelf->destructible,
                                                                        scr_const.right_tread,
                                                                        vehHealthPercentage);
    maingun_turningyaw = PlayersSeatGunTurning(pSelf, 0, 1);
    maingun_turningpitch = PlayersSeatGunTurning(pSelf, 0, 0);
    gunner1_turningyaw = PlayersSeatGunTurning(pSelf, 1, 1);
    gunner1_turningpitch = PlayersSeatGunTurning(pSelf, 1, 0);
    maingun_overheating = pSelf->scr_vehicle->turret.overheating != 0;
    gunner1_overheating = pSelf->scr_vehicle->gunnerTurrets[0].overheating != 0;
    G_VehSetClientSideHealthPercentageEntity(pSelf, vehHealthPercentage);
    G_VehSetClientSideHealthPercentageLeftTread(pSelf, vehTreadLeftHealthPercentage);
    G_VehSetClientSideHealthPercentageRightTread(pSelf, vehTreadRightHealthPercentage);
    G_VehSetClientSideSeatOccupancyFlags(pSelf, seat_occupancy_flags);
    G_VehSetClientSideGunTurningYaw(pSelf, 0, maingun_turningyaw);
    G_VehSetClientSideGunTurningYaw(pSelf, 1u, gunner1_turningyaw);
    G_VehSetClientSideGunTurningPitch(pSelf, 0, maingun_turningpitch);
    G_VehSetClientSideGunTurningPitch(pSelf, 1u, gunner1_turningpitch);
    G_VehSetClientSideGunOverheating(pSelf, 0, maingun_overheating);
    G_VehSetClientSideGunOverheating(pSelf, 1u, gunner1_overheating);
}

double __cdecl GetPieceHealthPercentage(
                Destructible *dest,
                unsigned __int16 destructibleLabelString,
                float defaultPercentage)
{
    int iPieceHealth; // [esp+0h] [ebp-18h] BYREF
    int pieceIndex; // [esp+4h] [ebp-14h]
    float pieceMaxHealth; // [esp+8h] [ebp-10h]
    DestructibleDef *ddef; // [esp+10h] [ebp-8h]
    float pieceHealth; // [esp+14h] [ebp-4h]

    if ( !dest )
        return defaultPercentage;
    ddef = Destructible_GetDDef(dest);
    pieceIndex = Destructible_GetPieceIndexForLabel(ddef, destructibleLabelString);
    if ( !GetPieceHealth(dest, destructibleLabelString, &iPieceHealth) )
        return defaultPercentage;
    pieceHealth = (float)iPieceHealth;
    pieceMaxHealth = (float)Destructible_GetPieceMaxHealth(ddef, pieceIndex);
    return (float)(pieceHealth / pieceMaxHealth);
}

char __cdecl GetPieceHealth(Destructible *dest, unsigned __int16 destructibleLabelString, int *pieceHealth)
{
    int pieceIndex; // [esp+0h] [ebp-8h]
    DestructibleDef *ddef; // [esp+4h] [ebp-4h]

    if ( !dest )
        return 0;
    ddef = Destructible_GetDDef(dest);
    pieceIndex = Destructible_GetPieceIndexForLabel(ddef, destructibleLabelString);
    if ( pieceIndex < 0 )
        return 0;
    *pieceHealth = Destructible_GetPieceHealth(dest, pieceIndex);
    return 1;
}

bool __cdecl PlayersSeatGunTurning(gentity_s *const pVehEnt, int seatIndex, int angle)
{
    const gentity_s *pOccupant; // [esp+Ch] [ebp-4h]

    pOccupant = VEH_GetSeatOccupantEntity(pVehEnt, seatIndex);
    return pOccupant
            && pOccupant->client
            && (int)abs(pOccupant->client->sess.cmd.angles[angle] - pOccupant->client->sess.oldcmd.angles[angle]) >= 5;
}

void __cdecl VEH_UpdateNitrousPosition(gentity_s *pSelf)
{
    char *v1; // eax
    __int64 X; // [esp+0h] [ebp-ECh]
    float v3; // [esp+8h] [ebp-E4h]
    float *origin; // [esp+24h] [ebp-C8h]
    float *vel; // [esp+30h] [ebp-BCh]
    float *v6; // [esp+34h] [ebp-B8h]
    NitrousVehicle *nitrousVeh; // [esp+4Ch] [ebp-A0h]
    float absPos[3]; // [esp+50h] [ebp-9Ch] BYREF
    vehicle_physic_t *phys; // [esp+5Ch] [ebp-90h]
    const vehicle_info_t *info; // [esp+60h] [ebp-8Ch]
    scr_vehicle_s *veh; // [esp+68h] [ebp-84h]
    float zero[3]; // [esp+6Ch] [ebp-80h] BYREF
    float hitp[3]; // [esp+78h] [ebp-74h] BYREF
    gentity_s *driver; // [esp+84h] [ebp-68h]
    float tvel[3]; // [esp+88h] [ebp-64h] BYREF
    float intensity; // [esp+94h] [ebp-58h]
    int hit_stype; // [esp+98h] [ebp-54h]
    float angles[3]; // [esp+9Ch] [ebp-50h] BYREF
    float absAxis[3][3]; // [esp+A8h] [ebp-44h] BYREF
    float hitn[3]; // [esp+CCh] [ebp-20h] BYREF
    int hit_entnum; // [esp+D8h] [ebp-14h]
    float avel[3]; // [esp+DCh] [ebp-10h] BYREF
    int notifyFlags; // [esp+E8h] [ebp-4h]
    int savedregs; // [esp+ECh] [ebp+0h] BYREF

    PROF_SCOPED("VEH_UpdateNitrousPosition");

    if ( !pSelf && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 8896, 0, "%s", "pSelf") )
        __debugbreak();
    if ( !pSelf->scr_vehicle
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                    8897,
                    0,
                    "%s",
                    "pSelf->scr_vehicle") )
    {
        __debugbreak();
    }
    veh = pSelf->scr_vehicle;
    phys = &veh->phys;
    info = BG_GetVehicleInfo(veh->infoIdx);
    nitrousVeh = veh->nitrousVehicle;
    memset(zero, 0, sizeof(zero));
    Sys_EnterCriticalSection(CRITSECT_PHYSICS);
    Phys_ObjGetPosition((int)nitrousVeh->m_phys_user_data, absPos, absAxis);
    Phys_ObjGetVelocities((int)nitrousVeh->m_phys_user_data, tvel, avel);
    notifyFlags = nitrousVeh->m_server_notify_flags;
    nitrousVeh->m_server_notify_flags = 0;
    intensity = nitrousVeh->m_collision_intensity;
    hit_stype = nitrousVeh->m_collision_stype;
    hit_entnum = nitrousVeh->m_collision_entnum;
    hitp[0] = nitrousVeh->m_collision_hitp[0];
    hitp[1] = nitrousVeh->m_collision_hitp[1];
    hitp[2] = nitrousVeh->m_collision_hitp[2];
    hitn[0] = nitrousVeh->m_collision_hitn[0];
    hitn[1] = nitrousVeh->m_collision_hitn[1];
    hitn[2] = nitrousVeh->m_collision_hitn[2];
    driver = VEH_GetSeatOccupantEntity(veh, 0);
    if ( driver && driver->client )
        VEH_SetPlayerVehicle(pSelf, 1);
    Sys_LeaveCriticalSection(CRITSECT_PHYSICS);
    AxisToAngles(absAxis, angles);
    if ( (nitrousVeh->m_flags & 0x100) == 0 && (nitrousVeh->m_flags & 0x80) == 0 )
        veh->speed = nitrousVeh->m_forward_vel;
    if ( (nitrousVeh->m_flags & 0x80) == 0 )
    {
        phys->origin[0] = absPos[0];
        phys->origin[1] = absPos[1];
        phys->origin[2] = absPos[2];
        v6 = phys->angles;
        phys->angles[0] = angles[0];
        v6[1] = angles[1];
        v6[2] = angles[2];
        vel = phys->vel;
        phys->vel[0] = tvel[0];
        vel[1] = tvel[1];
        vel[2] = tvel[2];
    }
    if ( sv_clientSideVehicles->current.enabled )
    {
        v3 = floor(nitrousVeh->m_throttle * 32768.0 + 0.5);
        pSelf->s.lerp.u.vehicle.throttle = (int)v3;
        //pSelf->s.lerp.u.actor.actorNum = LODWORD(nitrousVeh->m_steer_factor);
        pSelf->s.lerp.u.actor.actorNum = int(nitrousVeh->m_steer_factor); // KISAKTODO: union abuse
        if ( (veh->flags & 0x100) != 0 && (nitrousVeh->m_flags & 0x80) == 0 )
        {
            origin = veh->pathPos.origin;
            veh->pathPos.origin[0] = absPos[0];
            origin[1] = absPos[1];
        }
    }
    else if ( info->type && info->type != 1 )
    {
        pSelf->s.lerp.u.actor.actorNum = 0;
    }
    else
    {
        pSelf->s.lerp.u.turret.gunAngles[0] = (float)(info->nitrousVehParams.m_steer_angle_max * 57.295776)
                                                                                * nitrousVeh->m_steer_factor;
    }
    VEH_SetPosition(pSelf, absPos, tvel, angles);
    pSelf->s.lerp.pos.trDelta[0] = tvel[0];
    pSelf->s.lerp.pos.trDelta[1] = tvel[1];
    pSelf->s.lerp.pos.trDelta[2] = tvel[2];
    pSelf->s.lerp.pos.trDelta[0] = (float)(int)pSelf->s.lerp.pos.trDelta[0];
    pSelf->s.lerp.pos.trDelta[1] = (float)(int)pSelf->s.lerp.pos.trDelta[1];
    pSelf->s.lerp.pos.trDelta[2] = (float)(int)pSelf->s.lerp.pos.trDelta[2];
    if ( (float)((float)((float)(avel[0] * avel[0]) + (float)(avel[1] * avel[1])) + (float)(avel[2] * avel[2])) >= 0.001 )
    {
        pSelf->s.lerp.apos.trDelta[0] = avel[0];
        pSelf->s.lerp.apos.trDelta[1] = avel[1];
        pSelf->s.lerp.apos.trDelta[2] = avel[2];
    }
    else
    {
        pSelf->s.lerp.apos.trDelta[0] = 0.0f;
        pSelf->s.lerp.apos.trDelta[1] = 0.0f;
        pSelf->s.lerp.apos.trDelta[2] = 0.0f;
    }
    pSelf->s.lerp.pos.trTime = Phys_GetCurrentTime();
    if ( notifyFlags )
    {
        if ( (notifyFlags & 1) != 0 )
        {
            if ( hit_entnum == 1022 )
                Scr_AddUndefined(SCRIPTINSTANCE_SERVER);
            else
                Scr_AddEntityNum(hit_entnum, 0, SCRIPTINSTANCE_SERVER, 0);
            X = (unsigned int)hit_stype;
            v1 = (char *)Com_SurfaceTypeToName(hit_stype);
            Scr_AddString(v1, SCRIPTINSTANCE_SERVER);
            Scr_AddFloat(intensity, SCRIPTINSTANCE_SERVER);
            Scr_AddVector(hitn, SCRIPTINSTANCE_SERVER);
            Scr_AddVector(hitp, SCRIPTINSTANCE_SERVER);
            Scr_Notify(pSelf, scr_const.veh_collision, 5u);
        }
        if ( (notifyFlags & 2) != 0 )
            Scr_Notify(pSelf, scr_const.veh_inair, 0);
        if ( (notifyFlags & 4) != 0 )
            Scr_Notify(pSelf, scr_const.veh_landed, 0);
        if ( (notifyFlags & 8) != 0 )
            Scr_Notify(pSelf, scr_const.veh_suspension_limit_activated, 0);
        if ( (notifyFlags & 0x10) != 0 )
            Scr_Notify(pSelf, scr_const.veh_engine_stutter, 0);
    }
}

void __cdecl Scr_Vehicle_Think(gentity_s *pSelf)
{
    int v1; // eax
    NitrousVehicle *nitrousVeh; // [esp+Ch] [ebp-18h]
    const vehicle_info_t *info; // [esp+14h] [ebp-10h]
    scr_vehicle_s *veh; // [esp+1Ch] [ebp-8h]
    float defaultPitch; // [esp+20h] [ebp-4h]
    int savedregs; // [esp+24h] [ebp+0h] BYREF

    PROF_SCOPED("Scr_Vehicle_think");

    if ( !pSelf && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 9059, 0, "%s", "pSelf") )
        __debugbreak();
    if ( !pSelf->scr_vehicle
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                    9060,
                    0,
                    "%s",
                    "pSelf->scr_vehicle") )
    {
        __debugbreak();
    }
    veh = pSelf->scr_vehicle;
    info = BG_GetVehicleInfo(veh->infoIdx);
    if ( info->isNitrous && !veh->nitrousVehicle )
    {
        Sys_EnterCriticalSection(CRITSECT_PHYSICS_UPDATE);
        Sys_EnterCriticalSection(CRITSECT_PHYSICS);
        veh->nitrousVehicle = (NitrousVehicle *)NitrousVehicle::add_vehicle(pSelf->s.number);
        if (veh->nitrousVehicle)
        {
            //NitrousVehicle::init(veh->nitrousVehicle, pSelf, &info->nitrousVehParams);
            veh->nitrousVehicle->init(pSelf, &info->nitrousVehParams);
        }
        Sys_LeaveCriticalSection(CRITSECT_PHYSICS);
        Sys_LeaveCriticalSection(CRITSECT_PHYSICS_UPDATE);
    }
    VEH_BackupPosition(pSelf);
    memset(&s_phys, 0, sizeof(s_phys));
    //if ( EntHandle::isDefined(&pSelf->r.ownerNum) && g_entities[EntHandle::entnum(&pSelf->r.ownerNum)].health <= 0 )
    if ( pSelf->r.ownerNum.isDefined() && g_entities[pSelf->r.ownerNum.entnum()].health <= 0)
    {
        //v1 = EntHandle::entnum(&pSelf->r.ownerNum);
        v1 = pSelf->r.ownerNum.entnum();
        G_EntUnlink(&g_entities[v1]);
    }
    if ( (veh->flags & 0x40) == 0 )
        pSelf->s.lerp.eFlags &= ~0x40u;
    if ( info->type != 5 )
    {
        if ( (veh->flags & 0x80) != 0 )
        {
            VEH_UpdatePath(pSelf);
        }
        else if ( (veh->flags & 2) != 0 )
        {
            VEH_UpdateAIMove(pSelf);
        }
        else if ( info->type == 3 )
        {
            VEH_UpdateClientPlane(pSelf);
        }
        else if ( info->type == 6 )
        {
            VEH_UpdateClientChopper(pSelf);
        }
        else
        {
            VEH_UpdateClient(pSelf);
        }
        if ( info->type == 6 && (veh->flags & 0x100) != 0 )
            VEH_UpdateChopperPathDrive(pSelf);
    }
    defaultPitch = 0.0f;
    if ( info->type == 6 )
    {
        if ( veh->hasDefaultPitch )
            defaultPitch = veh->defaultPitch;
        else
            defaultPitch = vehHelicopterDefaultPitch->current.value;
        veh->phys.angles[0] = veh->phys.angles[0] + defaultPitch;
    }
    nitrousVeh = pSelf->scr_vehicle->nitrousVehicle;
    if ( nitrousVeh && nitrousVeh->m_phys_user_data )
        VEH_UpdateNitrousPosition(pSelf);
    else
        VEH_SetPosition(pSelf, veh->phys.origin, veh->phys.vel, veh->phys.angles);
    if ( (veh->flags & 2) != 0 )
        VEH_MoveTrace(pSelf);
    VEH_UpdateAvoidance(pSelf);
    VEH_TouchEntities(pSelf);
    G_DoTouchTriggers(pSelf);
    veh->turret.flags &= ~1u;
    VEH_UpdateDriverWeapons(pSelf);
    VEH_UpdateAim(pSelf);
    VEH_UpdateGunnerAimAll(pSelf);
    VEH_UpdateGunnerWeaponsAll(pSelf, 50);
    VEH_UpdateBody(pSelf);
    if ( !veh->nitrousVehicle )
        VEH_UpdateSteering(pSelf);
    //BLOPS_NULLSUB();
    VEH_UpdateHealth(pSelf);
    pSelf->nextthink = level.time + 50;
    VEH_UpdateDebug(pSelf);
    if ( vehicle_riding->current.enabled )
        G_MoverTeam_New(pSelf);
    if ( info->type == 6 )
        veh->phys.angles[0] = veh->phys.angles[0] - defaultPitch;
}

void __cdecl VEH_MoveTrace(gentity_s *ent)
{
    col_context_t context; // [esp+1Ch] [ebp-84h] BYREF
    vehicle_physic_t *phys; // [esp+44h] [ebp-5Ch]
    float size[3]; // [esp+48h] [ebp-58h]
    int dimIndex; // [esp+54h] [ebp-4Ch]
    trace_t trace; // [esp+58h] [ebp-48h] BYREF
    float maxs[3]; // [esp+94h] [ebp-Ch] BYREF

    if ( Scr_IsSystemActive(1u, SCRIPTINSTANCE_SERVER) )
    {
        phys = &ent->scr_vehicle->phys;
        if ( Vec3DistanceSq(phys->prevOrigin, phys->origin) != 0.0 )
        {
            maxs[0] = ent->r.maxs[0];
            maxs[1] = ent->r.maxs[1];
            maxs[2] = ent->r.maxs[2];
            size[0] = maxs[0] - ent->r.mins[0];
            size[1] = maxs[1] - ent->r.mins[1];
            size[2] = maxs[2] - ent->r.mins[2];
            dimIndex = size[0] < size[1];
            if ( size[dimIndex] > size[2] )
                maxs[2] = (float)(size[dimIndex] - size[2]) + maxs[2];
            //col_context_t::col_context_t(&context);
            G_TraceCapsule(&trace, phys->prevOrigin, ent->r.mins, maxs, phys->origin, ent->s.number, 8321, &context);
            if ( trace.fraction < 1.0 && Trace_GetEntityHitId(&trace) == 1022 )
            {
                Scr_AddVector(trace.normal.vec.v, SCRIPTINSTANCE_SERVER);
                Scr_Notify(ent, scr_const.script_vehicle_collision, 1u);
            }
        }
    }
}

void __cdecl VEH_BackupPosition(gentity_s *ent)
{
    scr_vehicle_s *veh; // [esp+18h] [ebp-4h]

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 2191, 0, "%s", "ent") )
        __debugbreak();
    if ( !ent->scr_vehicle
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                    2192,
                    0,
                    "%s",
                    "ent->scr_vehicle") )
    {
        __debugbreak();
    }
    veh = ent->scr_vehicle;
    veh->phys.prevOrigin[0] = veh->phys.origin[0];
    veh->phys.prevOrigin[1] = veh->phys.origin[1];
    veh->phys.prevOrigin[2] = veh->phys.origin[2];
    veh->phys.prevAngles[0] = veh->phys.angles[0];
    veh->phys.prevAngles[1] = veh->phys.angles[1];
    veh->phys.prevAngles[2] = veh->phys.angles[2];
    memcpy(&s_backup.phys, &veh->phys, sizeof(s_backup.phys));
}

void __cdecl VEH_UpdateAim(gentity_s *ent)
{
    float v1; // [esp+10h] [ebp-18Ch]
    float v2; // [esp+14h] [ebp-188h]
    gentity_s *v3; // [esp+18h] [ebp-184h]
    vehicle_physic_t *phys; // [esp+84h] [ebp-118h]
    const vehicle_info_t *info; // [esp+88h] [ebp-114h]
    float mtx[3][3]; // [esp+8Ch] [ebp-110h] BYREF
    float decompressedGunYaw; // [esp+B0h] [ebp-ECh]
    scr_vehicle_s *veh; // [esp+B4h] [ebp-E8h]
    float tgtDir[3]; // [esp+B8h] [ebp-E4h] BYREF
    float tgtAngles[3]; // [esp+C4h] [ebp-D8h] BYREF
    float invMtx[3][3]; // [esp+D0h] [ebp-CCh] BYREF
    gentity_s *tgtEnt; // [esp+F4h] [ebp-A8h]
    float decompressedGunPitch; // [esp+F8h] [ebp-A4h]
    float deltaAngles[3]; // [esp+FCh] [ebp-A0h] BYREF
    float angles[3]; // [esp+108h] [ebp-94h] BYREF
    float stopAngles[3]; // [esp+114h] [ebp-88h]
    float turretBaseMat[4][3]; // [esp+120h] [ebp-7Ch] BYREF
    float barrelPos[3]; // [esp+150h] [ebp-4Ch] BYREF
    float prevAngles[3]; // [esp+15Ch] [ebp-40h] BYREF
    float axis[3][3]; // [esp+168h] [ebp-34h] BYREF
    gentity_s *player; // [esp+18Ch] [ebp-10h]
    float tgtPos[3]; // [esp+190h] [ebp-Ch] BYREF

    veh = ent->scr_vehicle;
    phys = &veh->phys;
    info = BG_GetVehicleInfo(veh->infoIdx);
    tgtEnt = 0;
    if ( veh->hasTarget && ent->health > 0 )
    {
        if ( veh->targetEnt == 1023 )
        {
            tgtEnt = 0;
            tgtPos[0] = veh->targetOrigin[0];
            tgtPos[1] = veh->targetOrigin[1];
            tgtPos[2] = veh->targetOrigin[2];
        }
        else
        {
            tgtEnt = &g_entities[veh->targetEnt];
            tgtPos[0] = tgtEnt->r.currentOrigin[0] + veh->targetOffset[0];
            tgtPos[1] = tgtEnt->r.currentOrigin[1] + veh->targetOffset[1];
            tgtPos[2] = tgtEnt->r.currentOrigin[2] + veh->targetOffset[2];
        }
        if ( veh->boneIndex.barrel >= 0 )
        {
            G_DObjGetWorldBoneIndexPos(ent, veh->boneIndex.barrel, barrelPos);
            if ( veh->boneIndex.turret_base == -1 )
            {
                angles[0] = phys->angles[0];
                angles[1] = phys->angles[1];
                angles[2] = phys->angles[2];
            }
            else
            {
                G_DObjGetWorldBoneIndexMatrix(ent, veh->boneIndex.turret_base, turretBaseMat);
                AxisToAngles(turretBaseMat, angles);
            }
            //if ( EntHandle::isDefined(&ent->r.ownerNum) )
            if (ent->r.ownerNum.isDefined())
            {
                //v3 = EntHandle::ent(&ent->r.ownerNum);
                v3 = ent->r.ownerNum.ent();
            }
            else
                v3 = 0;
            player = v3;
            if ( v3 && !player->client->ps.vehiclePos && veh->targetEnt == 1023 )
            {
                tgtDir[0] = veh->targetOrigin[0] - barrelPos[0];
                tgtDir[1] = veh->targetOrigin[1] - barrelPos[1];
                tgtDir[2] = veh->targetOrigin[2] - barrelPos[2];
                Vec3Normalize(tgtDir);
                vectoangles(tgtDir, tgtAngles);
                if ( g_vehicleDebug->current.enabled )
                    CL_AddDebugLine(veh->targetOrigin, barrelPos, colorRed, 1, 1);
                if ( info->type != 6 )
                    tgtAngles[1] = player->client->ps.viewangles[1];
                tgtAngles[2] = 0.0f;
            }
            else
            {
                if ( g_vehicleDebug->current.enabled )
                    CL_AddDebugLine(tgtPos, barrelPos, colorRed, 1, 1);
                tgtDir[0] = tgtPos[0] - barrelPos[0];
                tgtDir[1] = tgtPos[1] - barrelPos[1];
                tgtDir[2] = tgtPos[2] - barrelPos[2];
                Vec3Normalize(tgtDir);
                vectoangles(tgtDir, tgtAngles);
            }
            AnglesToAxis(tgtAngles, axis);
            AnglesToAxis(angles, mtx);
            MatrixTranspose(mtx, invMtx);
            MatrixMultiply(axis, invMtx, mtx);
            AxisToAngles(mtx, tgtAngles);
            decompressedGunYaw = (float)ent->s.lerp.u.vehicle.gunYaw * 0.0054931641;
            decompressedGunPitch = (float)ent->s.lerp.u.vehicle.gunPitch * 0.0054931641;
            decompressedGunYaw = (float)(decompressedGunYaw - phys->angles[1]) + veh->phys.prevAngles[1];
            prevAngles[0] = decompressedGunPitch;
            prevAngles[1] = decompressedGunYaw;
            prevAngles[2] = 0.0f;
            AnglesSubtract(tgtAngles, prevAngles, deltaAngles);
            (deltaAngles[0]) = fabs(deltaAngles[0]);
            (deltaAngles[1]) = fabs(deltaAngles[1]);
            decompressedGunPitch = LinearTrackAngle(
                                                             tgtAngles[0],
                                                             prevAngles[0],
                                                             info->turretRotRate * veh->turretRotScale,
                                                             0.05f);
            decompressedGunYaw = LinearTrackAngle(
                                                         tgtAngles[1],
                                                         prevAngles[1],
                                                         info->turretRotRate * veh->turretRotScale,
                                                         0.05f);
            if ( vehLockTurretToPlayerView->current.enabled && player && !player->client->ps.vehiclePos )
                decompressedGunYaw = AngleNormalize180(tgtAngles[1]);
            stopAngles[0] = decompressedGunPitch;
            stopAngles[1] = decompressedGunYaw;
            stopAngles[2] = 0.0f;
            stopAngles[0] = AngleNormalize180(decompressedGunPitch - decompressedGunPitch);
            stopAngles[1] = AngleNormalize180(stopAngles[1] - decompressedGunYaw);
            v2 = floor(decompressedGunYaw * 182.04445 + 0.5);
            ent->s.lerp.u.vehicle.gunYaw = (int)v2;
            v1 = floor(decompressedGunPitch * 182.04445 + 0.5);
            ent->s.lerp.u.vehicle.gunPitch = (int)v1;
            if ( deltaAngles[0] >= 2.0 && stopAngles[0] == 0.0 || deltaAngles[1] >= 2.0 && stopAngles[1] == 0.0 )
            {
                veh->turret.turretState = VEH_TURRET_MOVING;
            }
            else if ( veh->turret.turretState == VEH_TURRET_MOVING )
            {
                veh->turret.turretState = VEH_TURRET_STOPPING;
            }
            else if ( veh->turret.turretState == VEH_TURRET_STOPPING )
            {
                Scr_Notify(ent, scr_const.turret_rotate_stopped, 0);
                veh->turret.turretState = VEH_TURRET_STOPPED;
            }
            if ( deltaAngles[0] >= 1.0 || deltaAngles[1] >= 1.0 )
            {
                Scr_Notify(ent, scr_const.turret_not_on_target, 0);
                Scr_Notify(ent, scr_const.turret_no_vis, 0);
            }
            else
            {
                if ( g_vehicleDebug->current.enabled )
                    VEH_DebugAim(ent, colorBlue, 40);
                Scr_Notify(ent, scr_const.turret_on_target, 0);
                if ( tgtEnt )
                {
                    //col_context_t::col_context_t(&context, 2049);
                    col_context_t context(0x801); // [esp+5Ch] [ebp-140h] BYREF

                    context.passEntityNum0 = ent->s.number;
                    context.passEntityNum1 = tgtEnt->s.number;
                    SV_SightTracePoint(&veh->turretHitNum, barrelPos, tgtPos, &context);
                    if ( veh->turretHitNum )
                        Scr_Notify(ent, scr_const.turret_no_vis, 0);
                    else
                        Scr_Notify(ent, scr_const.turret_on_vistarget, 0);
                }
                else
                {
                    Scr_Notify(ent, scr_const.turret_no_vis, 0);
                }
            }
            if ( g_vehicleDebug->current.enabled )
                VEH_DebugAim(ent, colorYellow, 1);
        }
    }
    else if ( veh->turret.turretState == VEH_TURRET_MOVING )
    {
        veh->turret.turretState = VEH_TURRET_STOPPING;
    }
    else if ( veh->turret.turretState == VEH_TURRET_STOPPING )
    {
        veh->turret.turretState = VEH_TURRET_STOPPED;
    }
}

void __cdecl VEH_UpdateGunnerAimAll(gentity_s *ent)
{
    const vehicle_info_t *info; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    for ( i = 0; i < 4; ++i )
    {
        info = BG_GetVehicleInfo(ent->scr_vehicle->infoIdx);
        if ( VEH_IsSeatPresent(ent, i + 1, 0) || info->driverControlledGunPos == i )
            VEH_UpdateGunnerAim(ent, i);
    }
}

void __cdecl VEH_UpdateGunnerAim(gentity_s *ent, int gunnerIndex)
{
    float v2; // [esp+10h] [ebp-178h]
    float v3; // [esp+14h] [ebp-174h]
    float v4; // [esp+18h] [ebp-170h]
    float v5; // [esp+1Ch] [ebp-16Ch]
    float v6; // [esp+20h] [ebp-168h]
    float v7; // [esp+24h] [ebp-164h]
    float leftArc; // [esp+44h] [ebp-144h]
    float bottomArc; // [esp+58h] [ebp-130h]
    const vehicle_info_t *info; // [esp+98h] [ebp-F0h]
    float mtx[3][3]; // [esp+9Ch] [ebp-ECh] BYREF
    scr_vehicle_s *veh; // [esp+C0h] [ebp-C8h]
    float tgtDir[3]; // [esp+C4h] [ebp-C4h] BYREF
    float tgtAngles[3]; // [esp+D0h] [ebp-B8h] BYREF
    float invMtx[3][3]; // [esp+DCh] [ebp-ACh] BYREF
    float yaw; // [esp+100h] [ebp-88h]
    gentity_s *tgtEnt; // [esp+104h] [ebp-84h]
    float turretOnTargetRange; // [esp+108h] [ebp-80h]
    float deltaAngles[3]; // [esp+10Ch] [ebp-7Ch] BYREF
    float angles[3]; // [esp+118h] [ebp-70h] BYREF
    float stopAngles[3]; // [esp+124h] [ebp-64h]
    bool worldTargetAngles; // [esp+133h] [ebp-55h]
    float pitch; // [esp+134h] [ebp-54h]
    const WeaponDef *weapDef; // [esp+138h] [ebp-50h]
    float barrelPos[3]; // [esp+13Ch] [ebp-4Ch] BYREF
    float prevAngles[3]; // [esp+148h] [ebp-40h] BYREF
    float axis[3][3]; // [esp+154h] [ebp-34h] BYREF
    gentity_s *player; // [esp+178h] [ebp-10h]
    float tgtPos[3]; // [esp+17Ch] [ebp-Ch] BYREF

    if ( !g_connectpaths->current.integer )
    {
        veh = ent->scr_vehicle;
        info = BG_GetVehicleInfo(veh->infoIdx);
        tgtEnt = 0;
        weapDef = BG_GetWeaponDef(info->gunnerWeaponIndex[gunnerIndex]);
        if ( ent->health > 0 )
        {
            pitch = (float)ent->s.lerp.u.vehicle.gunnerAngles[gunnerIndex].pitch * 0.0054931641;
            yaw = (float)ent->s.lerp.u.vehicle.gunnerAngles[gunnerIndex].yaw * 0.0054931641;
            prevAngles[0] = pitch;
            prevAngles[1] = yaw;
            prevAngles[2] = 0.0f;
            if ( veh->gunnerTargets[gunnerIndex].targetEnt == 1023 )
            {
                tgtEnt = 0;
                tgtPos[0] = veh->gunnerTargets[gunnerIndex].targetOrigin[0];
                tgtPos[1] = veh->gunnerTargets[gunnerIndex].targetOrigin[1];
                tgtPos[2] = veh->gunnerTargets[gunnerIndex].targetOrigin[2];
            }
            else
            {
                tgtEnt = &g_entities[veh->gunnerTargets[gunnerIndex].targetEnt];
                tgtPos[0] = tgtEnt->r.currentOrigin[0] + veh->gunnerTargets[gunnerIndex].targetOffset[0];
                tgtPos[1] = tgtEnt->r.currentOrigin[1] + veh->gunnerTargets[gunnerIndex].targetOffset[1];
                tgtPos[2] = tgtEnt->r.currentOrigin[2] + veh->gunnerTargets[gunnerIndex].targetOffset[2];
            }
            if ( veh->boneIndex.gunnerTags[gunnerIndex].barrel >= 0 )
            {
                worldTargetAngles = 1;
                player = VEH_GetSeatOccupantEntity(veh, gunnerIndex + 1);
                if ( !player && info->driverControlledGunPos == gunnerIndex )
                    player = VEH_GetSeatOccupantEntity(veh, 0);
                if ( player && player->client && player->client->ps.leanf == 0.0 && !veh->gunnerTargets[gunnerIndex].valid )
                {
                    if ( !player->client
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                                    3063,
                                    0,
                                    "%s",
                                    "player->client") )
                    {
                        __debugbreak();
                    }
                    if ( (player->client->ps.eFlags & 0x10000) != 0 )
                    {
                        tgtAngles[1] = info->gunnerRestAngles[gunnerIndex][1];
                        tgtAngles[0] = info->gunnerRestAngles[gunnerIndex][0];
                        tgtAngles[2] = 0.0f;
                        worldTargetAngles = 0;
                    }
                    else
                    {
                        tgtAngles[0] = player->client->ps.viewangles[0];
                        tgtAngles[1] = player->client->ps.viewangles[1];
                        tgtAngles[2] = 0.0f;
                        worldTargetAngles = 1;
                    }
                }
                else if ( veh->gunnerTargets[gunnerIndex].valid )
                {
                    G_DObjGetWorldBoneIndexPos(ent, veh->boneIndex.gunnerTags[gunnerIndex].barrel, barrelPos);

                    iassert(!IS_NAN((barrelPos)[0]) && !IS_NAN((barrelPos)[1]) && !IS_NAN((barrelPos)[2]));

                    tgtDir[0] = tgtPos[0] - barrelPos[0];
                    tgtDir[1] = tgtPos[1] - barrelPos[1];
                    tgtDir[2] = tgtPos[2] - barrelPos[2];
                    Vec3Normalize(tgtDir);
                    vectoangles(tgtDir, tgtAngles);
                    worldTargetAngles = 1;
                    if ( g_vehicleDebug->current.enabled )
                        CL_AddDebugLine(tgtPos, barrelPos, colorRed, 1, 1);
                }
                else
                {
                    tgtAngles[0] = prevAngles[0];
                    tgtAngles[1] = prevAngles[1];
                    tgtAngles[2] = prevAngles[2];
                    worldTargetAngles = 0;
                }
                if ( worldTargetAngles )
                {
                    angles[0] = veh->phys.angles[0];
                    angles[1] = veh->phys.angles[1];
                    angles[2] = veh->phys.angles[2];
                    AnglesToAxis(angles, mtx);
                    MatrixTranspose(mtx, invMtx);
                    AnglesToAxis(tgtAngles, axis);
                    MatrixMultiply(axis, invMtx, mtx);
                    AxisToAngles(mtx, tgtAngles);
                }
                AnglesSubtract(tgtAngles, prevAngles, deltaAngles);
                (deltaAngles[0]) = fabs(deltaAngles[0]);
                (deltaAngles[1]) = fabs(deltaAngles[1]);
                if ( player && player->client && !veh->gunnerTargets[gunnerIndex].valid )
                {
                    pitch = AngleNormalize180(tgtAngles[0]);
                    yaw = tgtAngles[1];
                }
                else
                {
                    pitch = LinearTrackAngle(tgtAngles[0], prevAngles[0], info->gunnerRotRate, 0.05f);
                    yaw = LinearTrackAngle(tgtAngles[1], prevAngles[1], info->gunnerRotRate, 0.05f);
                }
                stopAngles[0] = pitch;
                stopAngles[1] = yaw;
                stopAngles[2] = 0.0f;
                if ( gunnerIndex == info->driverControlledGunPos )
                {
                    pitch = AngleNormalize180(pitch - info->gunnerRestAngles[gunnerIndex][0]);
                    if ( (float)(pitch - weapDef->bottomArc) < 0.0 )
                        bottomArc = pitch;
                    else
                        bottomArc = weapDef->bottomArc;
                    if ( (float)((-(weapDef->topArc)) - pitch) < 0.0 )
                        v7 = bottomArc;
                    else
                        v7 = -weapDef->topArc;
                    pitch = v7;
                    pitch = AngleNormalize180(v7 + info->gunnerRestAngles[gunnerIndex][0]);
                    yaw = AngleNormalize180(yaw - info->gunnerRestAngles[gunnerIndex][1]);
                    if ( (float)(yaw - weapDef->leftArc) < 0.0 )
                        leftArc = yaw;
                    else
                        leftArc = weapDef->leftArc;
                    if ( (float)((-(weapDef->rightArc)) - yaw) < 0.0 )
                        v6 = leftArc;
                    else
                        v6 = -weapDef->rightArc;
                    yaw = v6;
                    yaw = AngleNormalize180(v6 + info->gunnerRestAngles[gunnerIndex][1]);
                }
                if ( player && player->client && gVehicleRelativeGunnerAngles && info->driverControlledGunPos != gunnerIndex )
                {
                    v5 = floor((float)(player->client->ps.viewangles[0] + info->gunnerRestAngles[gunnerIndex][0]) * 182.04445 + 0.5);
                    ent->s.lerp.u.vehicle.gunnerAngles[gunnerIndex].pitch = (int)v5;
                    v4 = floor((float)(player->client->ps.viewangles[1] + info->gunnerRestAngles[gunnerIndex][1]) * 182.04445 + 0.5);
                    ent->s.lerp.u.vehicle.gunnerAngles[gunnerIndex].yaw = (int)v4;
                }
                else
                {
                    v3 = floor(pitch * 182.04445 + 0.5);
                    ent->s.lerp.u.vehicle.gunnerAngles[gunnerIndex].pitch = (int)v3;
                    v2 = floor(yaw * 182.04445 + 0.5);
                    ent->s.lerp.u.vehicle.gunnerAngles[gunnerIndex].yaw = (int)v2;
                }
                stopAngles[0] = AngleNormalize180(stopAngles[0] - pitch);
                stopAngles[1] = AngleNormalize180(stopAngles[1] - yaw);
                if ( deltaAngles[0] >= 0.05f && stopAngles[0] == 0.0
                    || deltaAngles[1] >= 0.05f && stopAngles[1] == 0.0 )
                {
                    veh->gunnerTurrets[gunnerIndex].turretState = VEH_TURRET_MOVING;
                }
                else if ( veh->gunnerTurrets[gunnerIndex].turretState == VEH_TURRET_MOVING )
                {
                    veh->gunnerTurrets[gunnerIndex].turretState = VEH_TURRET_STOPPING;
                }
                else if ( veh->gunnerTurrets[gunnerIndex].turretState == VEH_TURRET_STOPPING )
                {
                    Scr_AddInt(gunnerIndex, SCRIPTINSTANCE_SERVER);
                    Scr_Notify(ent, scr_const.turret_rotate_stopped, 1u);
                    veh->gunnerTurrets[gunnerIndex].turretState = VEH_TURRET_STOPPED;
                }
                turretOnTargetRange = veh->gunnerTurrets[gunnerIndex].turretOnTargetRange;
                if ( turretOnTargetRange != 0.0 && turretOnTargetRange > deltaAngles[0] && turretOnTargetRange > deltaAngles[1] )
                {
                    Scr_AddInt(gunnerIndex, SCRIPTINSTANCE_SERVER);
                    Scr_Notify(ent, scr_const.gunner_turret_on_target, 1u);
                }
            }
        }
        else if ( veh->gunnerTurrets[gunnerIndex].turretState == VEH_TURRET_MOVING )
        {
            veh->gunnerTurrets[gunnerIndex].turretState = VEH_TURRET_STOPPING;
        }
        else if ( veh->gunnerTurrets[gunnerIndex].turretState == VEH_TURRET_STOPPING )
        {
            veh->gunnerTurrets[gunnerIndex].turretState = VEH_TURRET_STOPPED;
        }
    }
}

void __cdecl VEH_UpdateGunnerWeaponsAll(gentity_s *ent, int msec)
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < 4; ++i )
        VEH_UpdateGunnerWeapon(ent, i, msec);
}

void __cdecl VEH_UpdateGunnerWeapon(gentity_s *ent, int gunnerIndex, int msec)
{
    int WeaponIndexForName; // eax
    int frametime; // [esp+8h] [ebp-18h]
    const vehicle_info_t *info; // [esp+Ch] [ebp-14h]
    scr_vehicle_s *veh; // [esp+10h] [ebp-10h]
    const WeaponDef *weapDef; // [esp+14h] [ebp-Ch]
    gentity_s *player; // [esp+18h] [ebp-8h]
    unsigned int checkButton; // [esp+1Ch] [ebp-4h]

    veh = ent->scr_vehicle;
    info = BG_GetVehicleInfo(veh->infoIdx);
    checkButton = 0;
    if ( (float)ent->health > 0.0 && info->gunnerWeaponIndex[gunnerIndex] )
    {
        veh->gunnerTurrets[gunnerIndex].fireTime -= msec;
        if ( veh->gunnerTurrets[gunnerIndex].fireTime < 0 )
            veh->gunnerTurrets[gunnerIndex].fireTime = 0;
        player = VEH_GetSeatOccupantEntity(veh, gunnerIndex + 1);
        if ( !player && info->driverControlledGunPos == gunnerIndex )
        {
            checkButton = 35;
            player = VEH_GetSeatOccupantEntity(veh, 0);
        }
        weapDef = BG_GetWeaponDef(info->gunnerWeaponIndex[gunnerIndex]);
        if ( weapDef->overheatWeapon )
        {
            frametime = level.time - level.previousTime;
            if ( !player
                //|| !bitarray<51>::testBit(&player->client->button_bits, checkButton)
                || !player->client->button_bits.testBit(checkButton)
                || weapDef->coolWhileFiring
                || veh->gunnerTurrets[gunnerIndex].overheating == 1
                || player->client->ps.leanf > 0.0 )
            {
                veh->gunnerTurrets[gunnerIndex].heatVal = veh->gunnerTurrets[gunnerIndex].heatVal
                                                                                                - (float)((float)((float)frametime * weapDef->cooldownRate) / 1000.0);
                if ( veh->gunnerTurrets[gunnerIndex].heatVal < 0.0 )
                    veh->gunnerTurrets[gunnerIndex].heatVal = 0.0f;
                if ( weapDef->overheatEndVal >= veh->gunnerTurrets[gunnerIndex].heatVal )
                    veh->gunnerTurrets[gunnerIndex].overheating = 0;
            }
            if ( player )
            {
                //if ( bitarray<51>::testBit(&player->client->button_bits, checkButton) )
                if ( player->client->button_bits.testBit(checkButton) )
                {
                    if ( !veh->gunnerTurrets[gunnerIndex].overheating && player->client->ps.leanf == 0.0 )
                    {
                        veh->gunnerTurrets[gunnerIndex].heatVal = (float)((float)((float)frametime * weapDef->overheatRate) / 1000.0)
                                                                                                        + veh->gunnerTurrets[gunnerIndex].heatVal;
                        if ( veh->gunnerTurrets[gunnerIndex].heatVal >= 100.0 )
                        {
                            veh->gunnerTurrets[gunnerIndex].overheating = 1;
                            veh->gunnerTurrets[gunnerIndex].heatVal = 100.0f;
                        }
                    }
                }
            }
        }
        if ( player )
        {
            if ( !player->client
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                            3567,
                            0,
                            "%s",
                            "player->client") )
            {
                __debugbreak();
            }
            if ( (player->client->ps.eFlags & 0x10000) == 0 )
            {
                if ( !info->gunnerWeaponIndex[gunnerIndex] )
                {
                    WeaponIndexForName = G_GetWeaponIndexForName((char*)info->gunnerWeapon[gunnerIndex]);
                    AssignToSmallerType<unsigned short>(
                        &bg_vehicleInfos[veh->infoIdx].gunnerWeaponIndex[gunnerIndex],
                        WeaponIndexForName);
                    if ( !info->gunnerWeaponIndex[gunnerIndex] )
                        Com_Error(ERR_DROP, "Invalid gunner weapon specified for [%s] gunner [%d][%s]", info, gunnerIndex, info->gunnerWeapon[gunnerIndex]);
                }
                //if ( bitarray<51>::testBit(&player->client->button_bits, checkButton) )
                if ( player->client->button_bits.testBit(checkButton) )
                {
                    if ( !veh->gunnerTurrets[gunnerIndex].fireTime && !veh->gunnerTurrets[gunnerIndex].overheating )
                    {
                        VEH_FireGunnerWeapon(ent, gunnerIndex, player);
                        if ( (veh->gunnerTurrets[gunnerIndex].flags & 2) != 0 && (veh->gunnerTurrets[gunnerIndex].flags & 4) == 0 )
                            VEH_FireGunnerWeapon(ent, gunnerIndex, player);
                    }
                }
            }
        }
    }
}

gentity_s *__cdecl VEH_FireGunnerWeapon(gentity_s *ent, int gunnerIndex, gentity_s *attacker)
{
    int WeaponIndexForName; // eax
    const char *v5; // eax
    char *v6; // eax
    const char *v7; // eax
    char *v8; // eax
    const char *v9; // eax
    const vehicle_info_t *speedFrac; // [esp+Ch] [ebp-130h]
    const vehicle_info_t *speedFraca; // [esp+Ch] [ebp-130h]
    float playerSpread; // [esp+14h] [ebp-128h]
    gentity_s *v13; // [esp+18h] [ebp-124h]
    float vel[3]; // [esp+2Ch] [ebp-110h] BYREF
    gentity_s *target; // [esp+38h] [ebp-104h]
    float offset[3]; // [esp+3Ch] [ebp-100h] BYREF
    gentity_s *driver; // [esp+48h] [ebp-F4h]
    DObjTrace_s trace; // [esp+4Ch] [ebp-F0h] BYREF
    float spread; // [esp+70h] [ebp-CCh]
    float joltDir[3]; // [esp+74h] [ebp-C8h] BYREF
    const vehicle_info_t *info; // [esp+80h] [ebp-BCh]
    scr_vehicle_s *veh; // [esp+84h] [ebp-B8h]
    int event; // [esp+88h] [ebp-B4h]
    int flash; // [esp+8Ch] [ebp-B0h]
    gentity_s *proj; // [esp+90h] [ebp-ACh]
    weaponParms wp; // [esp+94h] [ebp-A8h] BYREF
    float barrelMtx[4][3]; // [esp+DCh] [ebp-60h] BYREF
    float flashMtx[4][3]; // [esp+10Ch] [ebp-30h] BYREF
    int savedregs; // [esp+13Ch] [ebp+0h] BYREF

    veh = ent->scr_vehicle;
    info = BG_GetVehicleInfo(veh->infoIdx);
    if ( !info->gunnerWeaponIndex[gunnerIndex] )
    {
        WeaponIndexForName = G_GetWeaponIndexForName((char*)info->gunnerWeapon[gunnerIndex]);
        AssignToSmallerType<unsigned short>(
            &bg_vehicleInfos[veh->infoIdx].gunnerWeaponIndex[gunnerIndex],
            WeaponIndexForName);
        if ( !info->gunnerWeaponIndex[gunnerIndex] )
            Com_Error(ERR_DROP, "Invalid gunner weapon specified for [%s] gunner [%d][%s]", info, gunnerIndex, info->gunnerWeapon[gunnerIndex]);
    }
    if ( (veh->gunnerTurrets[gunnerIndex].flags & 8) != 0 )
        return 0;
    proj = 0;
    Weapon_SetWeaponParamsWeapon(&wp, info->gunnerWeaponIndex[gunnerIndex]);
    if ( wp.weapDef->fireType != WEAPON_FIRETYPE_MINIGUN )
        goto LABEL_13;
    if ( !attacker || !attacker->client )
        return proj;
    if ( attacker->client->ps.weaponSpinLerp < 1.0 )
        return proj;
LABEL_13:
    if ( info->type != 3 && info->type != 6 )
    {
        if ( veh->boneIndex.gunnerTags[gunnerIndex].flash < 0 )
        {
            v5 = va("Missing tag_flash_gunner%i for [%s]\n", gunnerIndex + 1, info->name);
            Scr_Error(v5, 0);
        }
        G_DObjGetWorldBoneIndexMatrix(ent, veh->boneIndex.gunnerTags[gunnerIndex].flash, barrelMtx);
        if ( veh->joltTime == 0.0 || veh->joltTime < 0.64999998 )
        {
            joltDir[0] = -barrelMtx[0][0];
            joltDir[1] = -barrelMtx[0][1];
            joltDir[2] = -barrelMtx[0][2];
            VEH_JoltBody(ent, joltDir, 0.2, 0.0, 0.0);
        }
    }
    if ( vehicle_selfCollision->current.enabled && (veh->gunnerTurrets[gunnerIndex].flags & 2) != 0 )
    {
        if ( veh->gunnerTurrets[gunnerIndex].fireBarrel )
            veh->gunnerTurrets[gunnerIndex].fireBarrel = 0;
        else
            veh->gunnerTurrets[gunnerIndex].fireBarrel = 1;
    }
    if ( veh->gunnerTurrets[gunnerIndex].fireBarrel )
    {
        event = gunnerIndex + 178;
        flash = veh->boneIndex.gunnerTags[gunnerIndex].flash2;
        if ( flash < 0 )
        {
            speedFrac = info;
            v6 = SL_ConvertToString(*s_gunnerFlashTags[gunnerIndex + 4], SCRIPTINSTANCE_SERVER);
            v7 = va("Missing %s for vehicle %s\n", v6, speedFrac->name);
            Scr_Error(v7, 0);
        }
    }
    else
    {
        event = gunnerIndex + 174;
        flash = veh->boneIndex.gunnerTags[gunnerIndex].flash;
        if ( flash < 0 )
        {
            speedFraca = info;
            v8 = SL_ConvertToString(*s_gunnerFlashTags[gunnerIndex], SCRIPTINSTANCE_SERVER);
            v9 = va("Missing %s for vehicle %s\n", v8, speedFraca->name);
            Scr_Error(v9, 0);
        }
    }
    G_DObjGetWorldBoneIndexMatrix(ent, flash, flashMtx);
    if ( !vehicle_selfCollision->current.enabled )
    {
        G_TraceBulletPathForVehTurret(ent, &trace, gunnerIndex);
        if ( trace.fraction != 1.0 )
            return 0;
    }
    //*(_QWORD *)wp.gunForward = *(_QWORD *)&flashMtx[0][0];
    wp.gunForward[0] = flashMtx[0][0];
    wp.gunForward[1] = flashMtx[0][1];
    wp.gunForward[2] = flashMtx[0][2];
    //*(_QWORD *)wp.forward = *(_QWORD *)&flashMtx[0][0];
    wp.forward[0] = flashMtx[0][0];
    wp.forward[1] = flashMtx[0][1];
    wp.forward[2] = flashMtx[0][2];
    //*(_QWORD *)wp.right = *(_QWORD *)&flashMtx[1][0];
    wp.right[0] = flashMtx[1][0];
    wp.right[1] = flashMtx[1][1];
    wp.right[2] = flashMtx[1][2];
    //*(_QWORD *)wp.up = *(_QWORD *)&flashMtx[2][0];
    wp.up[0] = flashMtx[2][0];
    wp.up[1] = flashMtx[2][1];
    wp.up[2] = flashMtx[2][2];
    //*(_QWORD *)wp.muzzleTrace = *(_QWORD *)&flashMtx[3][0];
    wp.muzzleTrace[0] = flashMtx[3][0];
    wp.muzzleTrace[1] = flashMtx[3][1];
    wp.muzzleTrace[2] = flashMtx[3][2];
    if ( !attacker )
    {
        attacker = VEH_GetSeatOccupantEntity(veh, gunnerIndex + 1);
        if ( !attacker && !G_IsVehicleSeatOccupied(ent, gunnerIndex + 1) )
        {
            driver = VEH_GetSeatOccupantEntity(veh, 0);
            if ( driver )
                v13 = driver;
            else
                v13 = ent;
            attacker = v13;
        }
        if ( !attacker )
            attacker = ent;
    }
    if ( attacker->client )
    {
        if ( attacker->client->ps.vehiclePos )
        {
            G_GetPlayerViewOrigin(&attacker->client->ps, wp.muzzleTrace);
            if ( attacker->client->ps.vehiclePos >= 1
                && attacker->client->ps.vehiclePos <= 4
                && attacker->client->ps.leanf > 0.0 )
            {
                return 0;
            }
        }
    }
    if ( attacker && attacker->client )
        playerSpread = wp.weapDef->playerSpread;
    else
        playerSpread = wp.weapDef->aiSpread;
    spread = playerSpread;
    if ( wp.weapDef->weapType )
    {
        switch ( wp.weapDef->weapType )
        {
            case WEAPTYPE_PROJECTILE:
                if ( info->type == 6 )
                {
                    memset(vel, 0, sizeof(vel));
                }
                else
                {
                    vel[0] = veh->phys.vel[0];
                    vel[1] = veh->phys.vel[1];
                    vel[2] = veh->phys.vel[2];
                }
                if ( attacker && attacker->client && (attacker->client->ps.weapLockFlags & 2) != 0 )
                {
                    target = &level.gentities[attacker->client->ps.weapLockedEntnum];
                    G_TargetGetOffset(target, offset);
                }
                else
                {
                    target = 0;
                    memset(offset, 0, sizeof(offset));
                }
                proj = Weapon_RocketLauncher_Fire(
                                 attacker,
                                 info->gunnerWeaponIndex[gunnerIndex],
                                 spread,
                                 &wp,
                                 vel,
                                 target,
                                 offset);
                break;
            case WEAPTYPE_GAS:
                if ( attacker->client )
                    attacker->client->ps.eFlags |= 0x40u;
                veh->flags |= 0x40u;
                ent->s.lerp.eFlags |= 0x40u;
                Weapon_Flamethrower_Fire(attacker, &wp);
                if ( !attacker
                    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 3483, 0, "%s", "attacker") )
                {
                    __debugbreak();
                }
                break;
            case WEAPTYPE_GRENADE:
                proj = Weapon_GrenadeLauncher_Fire(attacker, info->gunnerWeaponIndex[gunnerIndex], 0, &wp);
                break;
        }
    }
    else
    {
        Bullet_Fire(attacker, spread, &wp, ent, veh->gunnerTurrets[gunnerIndex].fireBarrel + level.time);
    }
    G_AddEvent(ent, event, attacker->s.number);
    veh->gunnerTurrets[gunnerIndex].fireTime = wp.weapDef->iFireTime;
    return proj;
}

void __cdecl VEH_UpdateBody(gentity_s *ent)
{
    float v1; // xmm0_8
    //long double v2; // [esp+8h] [ebp-18h]
    float v3; // [esp+8h] [ebp-18h]
    scr_vehicle_s *veh; // [esp+18h] [ebp-8h]
    float intensity; // [esp+1Ch] [ebp-4h]

    veh = ent->scr_vehicle;
    if ( !veh->nitrousVehicle && veh->joltTime > 0.0 )
    {
        v1 = (float)(veh->joltWave * 0.017453292);
        //__libm_sse2_sin(v2);
        //*(float *)&v1 = v1;
        v1 = sin(v1);
        intensity = (float)(veh->joltTime / 1.3) * *(float *)&v1;
        v3 = floor((float)(intensity * veh->joltDir[0]) * 182.04445 + 0.5);
        ent->s.lerp.u.vehicle.throttle = (int)v3;
        ent->s.lerp.u.turret.gunAngles[1] = intensity * veh->joltDir[1];
        veh->joltTime = veh->joltTime - 0.05f;
        veh->joltWave = veh->joltWave + 36.0;
    }
}

void __cdecl VEH_UpdateSteering(gentity_s *ent)
{
    float v1; // [esp+4h] [ebp-44h]
    float v2; // [esp+8h] [ebp-40h]
    float v3; // [esp+10h] [ebp-38h]
    vehicle_physic_t *phys; // [esp+28h] [ebp-20h]
    scr_vehicle_s *veh; // [esp+30h] [ebp-18h]
    float forward[3]; // [esp+34h] [ebp-14h] BYREF
    bool isReverse; // [esp+43h] [ebp-5h]
    float deltaYaw; // [esp+44h] [ebp-4h]

    veh = ent->scr_vehicle;
    if (BG_GetVehicleInfo(veh->infoIdx)->steerWheels)
    {
        isReverse = 0;
        phys = &veh->phys;
        if ((float)((float)((float)(phys->vel[0] * phys->vel[0]) + (float)(phys->vel[1] * phys->vel[1]))
            + (float)(phys->vel[2] * phys->vel[2])) != 0.0)
        {
            AngleVectors(veh->phys.angles, forward, 0, 0);
            isReverse = (float)((float)((float)(forward[0] * veh->phys.vel[0]) + (float)(forward[1] * veh->phys.vel[1]))
                + (float)(forward[2] * veh->phys.vel[2])) < 0.0;
        }
        deltaYaw = AngleNormalize180(veh->phys.angles[1] - veh->phys.prevAngles[1]);
        if (isReverse)
            v2 = -10.0f;
        else
            v2 = 10.0f;
        deltaYaw = deltaYaw * v2;
        if ((float)(deltaYaw - 45.0) < 0.0)
            v3 = deltaYaw;
        else
            v3 = 45.0f;
        if ((float)(-45.0 - deltaYaw) < 0.0)
            v1 = v3;
        else
            v1 = -45.0f;
        ent->s.lerp.u.turret.gunAngles[0] = v1;
    }
    else
    {
        ent->s.lerp.u.actor.actorNum = 0;// *(_DWORD *)&FLOAT_0_0;
    }
}

void __cdecl VEH_UpdateDriverWeapons(gentity_s *ent)
{
    int v1; // eax
    char *v2; // eax
    float *targetOffset; // [esp+Ch] [ebp-118h]
    float *v4; // [esp+10h] [ebp-114h]
    float *v5; // [esp+14h] [ebp-110h]
    float *v6; // [esp+18h] [ebp-10Ch]
    float *targetOrigin; // [esp+20h] [ebp-104h]
    int hitnum; // [esp+28h] [ebp-FCh] BYREF
    float angles[3]; // [esp+54h] [ebp-D0h] BYREF
    const WeaponDef *gunnerWeapDef; // [esp+60h] [ebp-C4h]
    int driverControlledGunPos; // [esp+64h] [ebp-C0h]
    int gunnerIndex; // [esp+68h] [ebp-BCh]
    bool wasAlreadyInReloadTime; // [esp+6Eh] [ebp-B6h]
    bool nowInReloadTime; // [esp+6Fh] [ebp-B5h]
    gclient_s *client; // [esp+70h] [ebp-B4h]
    const vehicle_info_t *info; // [esp+74h] [ebp-B0h]
    scr_vehicle_s *veh; // [esp+78h] [ebp-ACh]
    float flashPos[3]; // [esp+7Ch] [ebp-A8h] BYREF
    float right[3]; // [esp+88h] [ebp-9Ch] BYREF
    float start[3]; // [esp+94h] [ebp-90h] BYREF
    float end[3]; // [esp+A0h] [ebp-84h] BYREF
    float forward[3]; // [esp+ACh] [ebp-78h] BYREF
    trace_t trace; // [esp+B8h] [ebp-6Ch] BYREF
    const WeaponVariantDef *weapVariantDef; // [esp+F4h] [ebp-30h]
    float offsetVec[3]; // [esp+F8h] [ebp-2Ch] BYREF
    float up[3]; // [esp+104h] [ebp-20h] BYREF
    float barrelPos[3]; // [esp+110h] [ebp-14h] BYREF
    const WeaponDef *weapDef; // [esp+11Ch] [ebp-8h]
    gentity_s *player; // [esp+120h] [ebp-4h]

    veh = ent->scr_vehicle;
    player = 0;
    client = 0;
    info = BG_GetVehicleInfo(veh->infoIdx);
    weapVariantDef = 0;
    weapDef = 0;
    VEH_UpdateDriverActions(ent);
    if ( ent->s.weapon )
    {
        weapVariantDef = BG_GetWeaponVariantDef(ent->s.weapon);
        weapDef = BG_GetWeaponDef(ent->s.weapon);
        if ( veh->turret.fireTime > 0 )
        {
            wasAlreadyInReloadTime = veh->turret.fireTime <= weapVariantDef->iReloadTime;
            veh->turret.fireTime -= 50;
            nowInReloadTime = veh->turret.fireTime <= weapVariantDef->iReloadTime;
            if ( !wasAlreadyInReloadTime && nowInReloadTime )
                G_AddEvent(ent, 0x14u, 0);
            if ( veh->turret.fireTime <= 0 )
                G_AddEvent(ent, 0x15u, 0);
        }
    }
    //if ( EntHandle::isDefined(&ent->r.ownerNum) )
    if ( ent->r.ownerNum.isDefined() )
    {
        //player = EntHandle::ent(&ent->r.ownerNum);
        player = ent->r.ownerNum.ent();
        client = player->client;
        if ( !client
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 3820, 0, "%s", "client") )
        {
            __debugbreak();
        }
        if ( (player->client->ps.eFlags & 0x10000) == 0 && !player->client->ps.vehiclePos )
        {
            if ( veh->turret.fireTime <= 0
                && ent->s.weapon
                //&& bitarray<51>::testBit(&client->sess.cmd.button_bits, 0x22u)
                && client->sess.cmd.button_bits.testBit(0x22u)
                && (client->ps.pm_flags & 0xC00) == 0 )
            {
                Scr_AddEntity(player, SCRIPTINSTANCE_SERVER);
                Scr_Notify(ent, scr_const.turret_fire, 1u);
            }
            if ( ent->s.weapon )
                veh->hasTarget = 1;
            G_GetPlayerViewOrigin(&client->ps, start);
            if ( vehHelicopterHeadSwayDontSwayTheTurret->current.enabled && info->type == 6 )
                AngleVectors(ent->r.currentAngles, forward, right, up);
            else
                G_GetPlayerViewDirection(player, forward, right, up);
            memset(offsetVec, 0, sizeof(offsetVec));
            if ( !weapDef && info->driverControlledGunPos >= 0 )
            {
                gunnerIndex = info->driverControlledGunPos;
                weapDef = BG_GetWeaponDef(info->gunnerWeaponIndex[gunnerIndex]);
                if ( weapDef )
                    BG_CalcVehicleTurretWeaponPosOffset(player->client->ps.fWeaponPosFrac, weapDef, offsetVec);
            }
            if ( info->type != 6 )
            {
                start[0] = (float)((float)((-(offsetVec[0])) + 300.0) * forward[0]) + start[0];
                start[1] = (float)((float)((-(offsetVec[0])) + 300.0) * forward[1]) + start[1];
                start[2] = (float)((float)((-(offsetVec[0])) + 300.0) * forward[2]) + start[2];
            }
            end[0] = (float)(10240.0 * forward[0]) + start[0];
            end[1] = (float)(10240.0 * forward[1]) + start[1];
            end[2] = (float)(10240.0 * forward[2]) + start[2];
            targetOrigin = veh->targetOrigin;
            veh->targetOrigin[0] = end[0];
            targetOrigin[1] = end[1];
            targetOrigin[2] = end[2];
            //v1 = EntHandle::entnum(&ent->r.ownerNum);
            v1 = ent->r.ownerNum.entnum();
            G_LocationalTrace(&trace, start, end, v1, 0x280E893, 0, 0);
            if ( trace.fraction < 1.0 )
                Vec3Lerp(start, end, trace.fraction, veh->targetOrigin);
            if ( info->driverControlledGunPos >= 0 )
            {
                driverControlledGunPos = info->driverControlledGunPos;
                if ( veh->gunnerTargets[driverControlledGunPos].targetEnt == 1023 )
                {
                    gunnerWeapDef = BG_GetWeaponDef(info->gunnerWeaponIndex[driverControlledGunPos]);
                    if ( gunnerWeapDef->weapType == WEAPTYPE_GAS )
                    {
                        angles[0] = client->ps.viewangles[0];
                        angles[1] = client->ps.viewangles[1];
                        angles[2] = client->ps.viewangles[2];
                        angles[0] = angles[0] - 6.0;
                        AngleVectors(angles, forward, 0, 0);
                        v6 = veh->gunnerTargets[driverControlledGunPos].targetOrigin;
                        *v6 = (float)(5000.0 * forward[0]) + start[0];
                        v6[1] = (float)(5000.0 * forward[1]) + start[1];
                        v6[2] = (float)(5000.0 * forward[2]) + start[2];
                    }
                    else
                    {
                        v4 = veh->gunnerTargets[driverControlledGunPos].targetOrigin;
                        v5 = veh->targetOrigin;
                        *v4 = veh->targetOrigin[0];
                        v4[1] = v5[1];
                        v4[2] = v5[2];
                    }
                    targetOffset = veh->gunnerTargets[driverControlledGunPos].targetOffset;
                    *targetOffset = 0.0f;
                    targetOffset[1] = 0.0f;
                    targetOffset[2] = 0.0f;
                    veh->gunnerTargets[driverControlledGunPos].valid = 1;
                }
            }
            if ( ent->s.weapon && info->type == 2 )
            {
                if ( veh->boneIndex.flash[0] < 0 )
                {
                    v2 = SL_ConvertToString(*s_flashTags[0], SCRIPTINSTANCE_SERVER);
                    Com_Error(ERR_DROP, "Player vehicle has no %s", v2);
                }
                if ( veh->boneIndex.barrel < 0 && info->type != 3 )
                    Com_Error(ERR_DROP, "Player vehicle has no tag_barrel");
                G_DObjGetWorldBoneIndexPos(ent, veh->boneIndex.flash[0], flashPos);
                if ( info->type != 3 )
                    G_DObjGetWorldBoneIndexPos(ent, veh->boneIndex.barrel, barrelPos);
                hitnum = -1;

                col_context_t context(ent->clipmask); // [esp+2Ch] [ebp-F8h] BYREF
                context.passEntityNum0 = ent->s.number;
                if ( info->type != 3 && !SV_SightTracePoint(&hitnum, barrelPos, flashPos, &context) )
                    veh->turret.flags |= 1u;
            }
        }
    }
}

void __cdecl VEH_UpdateDriverActions(gentity_s *ent)
{
    scr_vehicle_s *veh; // [esp+0h] [ebp-8h]
    gentity_s *player; // [esp+4h] [ebp-4h]

    //if ( EntHandle::isDefined(&ent->r.ownerNum) )
    if ( ent->r.ownerNum.isDefined() )
    {
        //player = EntHandle::ent(&ent->r.ownerNum);
        player = ent->r.ownerNum.ent();
        if ( player )
        {
            if ( player->client )
            {
                veh = ent->scr_vehicle;
                if ( veh )
                {
                    //if ( bitarray<51>::testBit(&player->client->button_bits, 0x27u) )
                    if ( player->client->button_bits.testBit(0x27u) )
                    {
                        Scr_AddEntity(ent, SCRIPTINSTANCE_SERVER);
                        Scr_Notify(ent, scr_const.veh_usespecialability, 1u);
                        veh->m_bSpecialAbilityEventDown = 1;
                    }
                    else if ( veh->m_bSpecialAbilityEventDown )
                    {
                        Scr_AddEntity(ent, SCRIPTINSTANCE_SERVER);
                        Scr_Notify(ent, scr_const.veh_usespecialabilityend, 1u);
                        veh->m_bSpecialAbilityEventDown = 0;
                    }
                    if ( player->client->button_bits.testBit(0x28u) )
                    {
                        Scr_AddEntity(ent, SCRIPTINSTANCE_SERVER);
                        Scr_Notify(ent, scr_const.veh_firepickup, 1u);
                        veh->m_bFirePickupEventDown = 1;
                    }
                    else if ( veh->m_bFirePickupEventDown )
                    {
                        Scr_AddEntity(ent, SCRIPTINSTANCE_SERVER);
                        Scr_Notify(ent, scr_const.veh_firepickupend, 1u);
                        veh->m_bFirePickupEventDown = 0;
                    }
                    if ( player->client->button_bits.testBit(0x29u) )
                    {
                        Scr_AddEntity(ent, SCRIPTINSTANCE_SERVER);
                        Scr_Notify(ent, scr_const.veh_swappickup, 1u);
                        veh->m_bSwapPickupEventDown = 1;
                    }
                    else if ( veh->m_bSwapPickupEventDown )
                    {
                        Scr_AddEntity(ent, SCRIPTINSTANCE_SERVER);
                        Scr_Notify(ent, scr_const.veh_swappickupend, 1u);
                        veh->m_bSwapPickupEventDown = 0;
                    }
                    if ( player->client->button_bits.testBit(0x2Au) )
                    {
                        Scr_AddEntity(ent, SCRIPTINSTANCE_SERVER);
                        Scr_Notify(ent, scr_const.veh_dropdeployable, 1u);
                        veh->m_bDropDeployableEventDown = 1;
                    }
                    else if ( veh->m_bDropDeployableEventDown )
                    {
                        Scr_AddEntity(ent, SCRIPTINSTANCE_SERVER);
                        Scr_Notify(ent, scr_const.veh_dropdeployableend, 1u);
                        veh->m_bDropDeployableEventDown = 0;
                    }
                    if ( player->client->button_bits.testBit(0x17u) )
                    {
                        Scr_AddEntity(ent, SCRIPTINSTANCE_SERVER);
                        Scr_Notify(ent, scr_const.veh_handbreak, 1u);
                    }
                    if ( player->client->button_bits.testBit(1u) )
                    {
                        Scr_AddEntity(ent, SCRIPTINSTANCE_SERVER);
                        Scr_Notify(ent, scr_const.veh_boost, 1u);
                    }
                    else if ( ent->scr_vehicle
                                 && ent->scr_vehicle->nitrousVehicle
                                 && ent->scr_vehicle->nitrousVehicle->m_vehicle_info->boostDuration > ent->scr_vehicle->nitrousVehicle->m_boost_time_pool )
                    {
                        Scr_AddEntity(ent, SCRIPTINSTANCE_SERVER);
                        Scr_Notify(ent, scr_const.veh_boost_regen, 1u);
                    }
                }
            }
        }
    }
}

void __cdecl VEH_UpdateClient(gentity_s *ent)
{
    gentity_s *player; // [esp+28h] [ebp-68h]
    const vehicle_info_t *info; // [esp+30h] [ebp-60h]
    scr_vehicle_s *veh; // [esp+34h] [ebp-5Ch]
    char move[4]; // [esp+38h] [ebp-58h] BYREF
    float rotAccel[3]; // [esp+3Ch] [ebp-54h] BYREF
    float bodyAccel[3]; // [esp+48h] [ebp-48h] BYREF
    float worldAccel[3]; // [esp+54h] [ebp-3Ch] BYREF
    float axis[4][3]; // [esp+60h] [ebp-30h] BYREF

    move[0] = 0;
    move[1] = 0;
    move[2] = 0;
    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 4060, 0, "%s", "ent") )
        __debugbreak();
    if ( !ent->scr_vehicle
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                    4061,
                    0,
                    "%s",
                    "ent->scr_vehicle") )
    {
        __debugbreak();
    }
    veh = ent->scr_vehicle;
    info = BG_GetVehicleInfo(veh->infoIdx);
    if ( !veh->nitrousVehicle )
    {
        player = VEH_GetSeatOccupantEntity(veh, 0);
        if ( player )
        {
            if ( !player->client
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                            4073,
                            0,
                            "%s",
                            "player->client") )
            {
                __debugbreak();
            }
            if ( (player->client->ps.pm_flags & 0xC00) == 0 )
            {
                move[0] = player->client->sess.cmd.forwardmove;
                move[1] = player->client->sess.cmd.rightmove;
                move[2] = player->client->sess.cmd.button_bits.testBit(0xBu) ? 0x7F : 0;
            }
        }
        VEH_CalcAccel(ent, move, bodyAccel, rotAccel);
        veh->phys.rotVel[0] = (float)(0.05f * rotAccel[0]) + veh->phys.rotVel[0];
        veh->phys.rotVel[1] = (float)(0.05f * rotAccel[1]) + veh->phys.rotVel[1];
        veh->phys.rotVel[2] = (float)(0.05f * rotAccel[2]) + veh->phys.rotVel[2];
        veh->phys.angles[1] = AngleNormalize180((float)(veh->phys.rotVel[1] * 0.05f) + veh->phys.prevAngles[1]);
        veh->phys.angles[0] = 0.0f;
        veh->phys.angles[2] = 0.0f;
        AnglesToAxis(veh->phys.angles, axis);
        memset(axis[3], 0, sizeof(float[3]));
        MatrixTransformVector(bodyAccel, axis, worldAccel);
        veh->phys.vel[0] = (float)(0.05f * worldAccel[0]) + veh->phys.vel[0];
        veh->phys.vel[1] = (float)(0.05f * worldAccel[1]) + veh->phys.vel[1];
        veh->phys.vel[2] = (float)(0.05f * worldAccel[2]) + veh->phys.vel[2];
        if ( veh->phys.vel[0] != 0.0 || veh->phys.vel[1] != 0.0 || veh->phys.vel[2] != 0.0 )
        {
            VEH_GroundTrace(ent);
            if ( s_phys.onGround )
                VEH_GroundMove(ent);
            else
                VEH_AirMove(ent, 1);
        }
        if ( !info->type || info->type == 2 )
            VEH_GroundPlant(ent, &veh->phys, 1);
        MatrixTransposeTransformVector43(veh->phys.vel, axis, veh->phys.bodyVel);
        veh->speed = fabs(veh->phys.bodyVel[0]);
        if ( veh->speed < 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                        4136,
                        0,
                        "%s",
                        "veh->speed >= 0.0f") )
        {
            __debugbreak();
        }
    }
    if ( VEH_ShouldEjectOccupants(ent) )
    {
        Scr_AddEntity(ent, SCRIPTINSTANCE_SERVER);
        Scr_Notify(ent, scr_const.veh_ejectoccupants, 1u);
    }
}

void __cdecl VEH_GroundTrace(gentity_s *ent)
{
    col_context_t context; // [esp+Ch] [ebp-84h] BYREF
    vehicle_physic_t *phys; // [esp+34h] [ebp-5Ch]
    scr_vehicle_s *veh; // [esp+38h] [ebp-58h]
    float start[3]; // [esp+3Ch] [ebp-54h] BYREF
    trace_t trace; // [esp+48h] [ebp-48h] BYREF
    float point[3]; // [esp+84h] [ebp-Ch] BYREF

    veh = ent->scr_vehicle;
    phys = &veh->phys;
    //col_context_t::col_context_t(&context);
    start[0] = phys->origin[0];
    start[1] = phys->origin[1];
    start[2] = phys->origin[2] + 0.25;
    point[0] = phys->origin[0];
    point[1] = phys->origin[1];
    point[2] = phys->origin[2] - 0.25;
    G_TraceCapsule(&trace, start, phys->mins, phys->maxs, point, ent->s.number, ent->clipmask, &context);
    memcpy(&s_phys, &trace, 0x38u);
    s_phys.hasGround = 0;
    s_phys.onGround = 0;
    if ( (!trace.allsolid || VEH_CorrectAllSolid(ent, &trace))
        && trace.fraction != 1.0
        && (phys->vel[2] <= 0.0
         || (float)((float)((float)(phys->vel[0] * trace.normal.vec.v[0]) + (float)(phys->vel[1] * trace.normal.vec.v[1]))
                            + (float)(phys->vel[2] * trace.normal.vec.v[2])) <= 10.0) )
    {
        s_phys.hasGround = 1;
        if ( trace.normal.vec.v[2] >= 0.69999999 )
            s_phys.onGround = 1;
    }
}

void __cdecl VEH_GroundMove(gentity_s *ent)
{
    float vel; // [esp+24h] [ebp-14h]
    scr_vehicle_s *veh; // [esp+28h] [ebp-10h]
    float oldvel; // [esp+2Ch] [ebp-Ch]
    float oldvel_4; // [esp+30h] [ebp-8h]
    float oldvel_8; // [esp+34h] [ebp-4h]

    veh = ent->scr_vehicle;
    vel = Vec3Length(veh->phys.vel);
    oldvel = veh->phys.vel[0];
    oldvel_4 = veh->phys.vel[1];
    oldvel_8 = veh->phys.vel[2];
    VEH_ClipVelocity(veh->phys.vel, s_phys.groundTrace.normal.vec.v, veh->phys.vel);
    if ( (float)((float)((float)(veh->phys.vel[0] * oldvel) + (float)(veh->phys.vel[1] * oldvel_4))
                         + (float)(veh->phys.vel[2] * oldvel_8)) > 0.0 )
    {
        Vec3Normalize(veh->phys.vel);
        veh->phys.vel[0] = vel * veh->phys.vel[0];
        veh->phys.vel[1] = vel * veh->phys.vel[1];
        veh->phys.vel[2] = vel * veh->phys.vel[2];
    }
    if ( veh->phys.vel[0] != 0.0 || veh->phys.vel[1] != 0.0 )
        VEH_StepSlideMove(ent, 0);
}

static float avoidRadius = 200.0f;
void __cdecl VEH_UpdateAvoidance(gentity_s *ent)
{
    scr_vehicle_s *scr_vehicle; // ecx
    scr_vehicle_s *v2; // eax
    float *lookPos; // edx
    float v4; // [esp+20h] [ebp-50h]
    float howClose; // [esp+30h] [ebp-40h]
    float otherEntPos; // [esp+40h] [ebp-30h]
    float otherEntPos_4; // [esp+44h] [ebp-2Ch]
    float otherEntPos_8; // [esp+48h] [ebp-28h]
    int i; // [esp+4Ch] [ebp-24h]
    float goalPos[3]; // [esp+50h] [ebp-20h] BYREF
    scr_vehicle_s *otherVeh; // [esp+5Ch] [ebp-14h]
    float closestPt[3]; // [esp+60h] [ebp-10h] BYREF
    gentity_s *otherEnt; // [esp+6Ch] [ebp-4h]
    int savedregs; // [esp+70h] [ebp+0h] BYREF

    if ( (ent->scr_vehicle->flags & 0x102) != 0 && (ent->scr_vehicle->flags & 0x400) != 0 )
    {
        if ( (ent->scr_vehicle->flags & 0x100) != 0 )
        {
            scr_vehicle = ent->scr_vehicle;
            goalPos[0] = scr_vehicle->pathPos.lookPos[0];
            goalPos[1] = scr_vehicle->pathPos.lookPos[1];
            goalPos[2] = scr_vehicle->pathPos.lookPos[2];
        }
        else
        {
            v2 = ent->scr_vehicle;
            goalPos[0] = v2->goalPosition[0];
            goalPos[1] = v2->goalPosition[1];
            goalPos[2] = v2->goalPosition[2];
        }
        goalPos[2] = ent->r.currentOrigin[2];
        for ( i = 0; i < 16; ++i )
        {
            otherVeh = &s_vehicles[i];
            if ( otherVeh->entNum != 1023 && otherVeh->entNum != ent->s.number )
            {
                otherEnt = &g_entities[otherVeh->entNum];
                if ( ent->r.absmin[2] <= otherEnt->r.absmax[2] && otherEnt->r.absmin[2] <= ent->r.absmax[2] )
                {
                    otherEntPos = otherEnt->r.currentOrigin[0];
                    otherEntPos_4 = otherEnt->r.currentOrigin[1];
                    otherEntPos_8 = goalPos[2];
                    GetClosestPointOnSegment(goalPos, ent->r.currentOrigin, otherEnt->r.currentOrigin, closestPt);
                    howClose = (float)((float)((float)(closestPt[0] - otherEntPos) * (float)(closestPt[0] - otherEntPos))
                                                     + (float)((float)(closestPt[1] - otherEntPos_4) * (float)(closestPt[1] - otherEntPos_4)))
                                     + (float)((float)(closestPt[2] - otherEntPos_8) * (float)(closestPt[2] - otherEntPos_8));
                    if ( (float)(avoidRadius * avoidRadius) > howClose )
                    {
                        v4 = 1.0 / sqrtf(howClose);
                        goalPos[0] = (float)(avoidRadius * (float)(v4 * (float)(closestPt[0] - otherEntPos)))
                                             + otherEnt->r.currentOrigin[0];
                        goalPos[1] = (float)(avoidRadius * (float)(v4 * (float)(closestPt[1] - otherEntPos_4)))
                                             + otherEnt->r.currentOrigin[1];
                        goalPos[2] = (float)(avoidRadius * (float)(v4 * (float)(closestPt[2] - otherEntPos_8)))
                                             + otherEnt->r.currentOrigin[2];
                        if ( g_vehicleDebug->current.enabled )
                            G_DebugLine(goalPos, otherEnt->r.currentOrigin, colorWhite, 0);
                    }
                }
            }
        }
        if ( (ent->scr_vehicle->flags & 0x100) != 0 )
        {
            lookPos = ent->scr_vehicle->pathPos.lookPos;
            *lookPos = goalPos[0];
            lookPos[1] = goalPos[1];
            lookPos[2] = goalPos[2];
        }
        else if ( ent->scr_vehicle->nitrousVehicle && (ent->scr_vehicle->flags & 2) != 0 )
        {
            //NitrousVehicle::update_script_target(ent->scr_vehicle->nitrousVehicle, goalPos);
            ent->scr_vehicle->nitrousVehicle->update_script_target(goalPos);
        }
    }
}

void NitrousVehicle::update_script_target(float *goal_position)
{
    NitrousVehicleController *p_mVehicleController; // eax
    unsigned int v4[3]; // [esp-Ch] [ebp-2Ch] BYREF
    NitrousVehicle *v5; // [esp+10h] [ebp-10h]
    //int v6; // [esp+14h] [ebp-Ch]
    //void *v7; // [esp+18h] [ebp-8h]
    //void *retaddr; // [esp+20h] [ebp+0h]
    //
    //v6 = a2;
    //v7 = retaddr;
    v5 = this;
    Phys_Vec3ToNitrousVec(goal_position, (phys_vec3 *)v4);
    p_mVehicleController = &v5->mVehicleController;
    LODWORD(v5->mVehicleController.m_script_goal_position.x) = v4[0];
    LODWORD(p_mVehicleController->m_script_goal_position.y) = v4[1];
    LODWORD(p_mVehicleController->m_script_goal_position.z) = v4[2];
}

void __cdecl GetClosestPointOnSegment(float *pt1, float *pt2, float *testPt, float *out)
{
    float fraction; // [esp+Ch] [ebp-14h]
    float segmentLengthSq; // [esp+10h] [ebp-10h]
    float line; // [esp+14h] [ebp-Ch]
    float line_4; // [esp+18h] [ebp-8h]
    float line_8; // [esp+1Ch] [ebp-4h]

    line = *pt2 - *pt1;
    line_4 = pt2[1] - pt1[1];
    line_8 = pt2[2] - pt1[2];
    segmentLengthSq = (float)((float)(line * line) + (float)(line_4 * line_4)) + (float)(line_8 * line_8);
    if ( segmentLengthSq == 0.0
        || (fraction = (float)((float)((float)(line * (float)(*testPt - *pt1))
                                                                 + (float)(line_4 * (float)(testPt[1] - pt1[1])))
                                                 + (float)(line_8 * (float)(testPt[2] - pt1[2])))
                                 / segmentLengthSq,
                fraction < 0.0) )
    {
        *out = *pt1;
        out[1] = pt1[1];
        out[2] = pt1[2];
    }
    else if ( fraction <= 1.0 )
    {
        *out = (float)(fraction * line) + *pt1;
        out[1] = (float)(fraction * line_4) + pt1[1];
        out[2] = (float)(fraction * line_8) + pt1[2];
    }
    else
    {
        *out = *pt2;
        out[1] = pt2[1];
        out[2] = pt2[2];
    }
}

// local variable allocation has failed, the output may be wrong!
void    VEH_UpdatePath(gentity_s *ent)
{
    float speed; // xmm0_4
    double v3; // st7
    double v4; // st7
    double v5; // st7
    double v6; // st7
    double v7; // st7
    double v8; // st7
    double v9; // st7
    phys_vec3 goalPosition; // [esp+34h] [ebp-17Ch] BYREF
    float goalSpeed; // [esp+50h] [ebp-160h]
    int v12; // [esp+54h] [ebp-15Ch]
    float *v13; // [esp+58h] [ebp-158h]
    float *rotVel; // [esp+5Ch] [ebp-154h]
    float *bodyVel; // [esp+60h] [ebp-150h]
    float *v16; // [esp+64h] [ebp-14Ch]
    float *v17; // [esp+68h] [ebp-148h]
    float *prevOrigin; // [esp+6Ch] [ebp-144h]
    float *vel; // [esp+70h] [ebp-140h]
    float v20; // [esp+74h] [ebp-13Ch]
    float v21; // [esp+78h] [ebp-138h]
    float v22; // [esp+7Ch] [ebp-134h]
    float v23; // [esp+80h] [ebp-130h]
    float v24; // [esp+84h] [ebp-12Ch]
    float v25; // [esp+88h] [ebp-128h]
    float manualTime; // [esp+8Ch] [ebp-124h]
    float v27; // [esp+90h] [ebp-120h]
    float v28; // [esp+94h] [ebp-11Ch]
    float nodeIdx; // [esp+98h] [ebp-118h]
    int lastNode; // [esp+9Ch] [ebp-114h]
    float accel; // [esp+A0h] [ebp-110h] BYREF
    float tgtSpeed; // [esp+A4h] [ebp-10Ch]
    float v33; // [esp+A8h] [ebp-108h]
    bool waitNodeHit; // [esp+AFh] [ebp-101h]
    float prevSpeed; // [esp+B0h] [ebp-100h]
    vehicle_pathpos_t nextVpp; // [esp+B4h] [ebp-FCh] BYREF
    const vehicle_info_t *info; // [esp+198h] [ebp-18h]
    vehicle_physic_t *phys; // [esp+19Ch] [ebp-14h]
    scr_vehicle_s *veh; // [esp+1A0h] [ebp-10h]
    //_UNKNOWN *v40; // [esp+1A4h] [ebp-Ch]
    //gentity_s *enta; // [esp+1A8h] [ebp-8h]
    //int v42; // [esp+1ACh] [ebp-4h] BYREF
    //int vars0; // [esp+1B0h] [ebp+0h]
    //
    //v40 = a1;
    //enta = (gentity_s *)vars0;
    if (!ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 4262, 0, "%s", "ent"))
        __debugbreak();
    if (!ent->scr_vehicle
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
            4263,
            0,
            "%s",
            "ent->scr_vehicle"))
    {
        __debugbreak();
    }
    veh = ent->scr_vehicle;
    phys = &veh->phys;
    info = BG_GetVehicleInfo(veh->infoIdx);
    memcpy(&nextVpp, veh, sizeof(nextVpp));
    prevSpeed = veh->speed;
    waitNodeHit = 0;
    if (nextVpp.nodeIdx >= 0)
    {
        if (veh->pathPos.speed < 0.0
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                4276,
                0,
                "%s",
                "veh->pathPos.speed >= 0.0f"))
        {
            __debugbreak();
        }
        if (veh->manualSpeed < 0.0
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                4277,
                0,
                "%s",
                "veh->manualSpeed >= 0.0f"))
        {
            __debugbreak();
        }
        if (veh->manualMode)
        {
            if (veh->manualMode == 2)
                speed = veh->pathPos.speed;
            else
                speed = veh->manualSpeed;
            v33 = speed;
            tgtSpeed = speed;
            v3 = VEH_AccelerateSpeed(veh->speed, speed, veh->manualAccel, 0.05f);
            veh->speed = v3;
            if (veh->manualMode == 2 && veh->speed == tgtSpeed)
                veh->manualMode = 0;
        }
        else if ((veh->flags & 0x100) != 0)
        {
            VEH_GetNewSpeedAndAccel(veh, 0.05f, 0, 1.0, &veh->speed, &accel);
        }
        else
        {
            veh->speed = veh->pathPos.speed;
        }
        if (veh->pathPos.speed <= 0.0)
            veh->manualTime = 0.0f;
        else
            veh->manualTime = (float)(veh->speed / veh->pathPos.speed) + veh->manualTime;
        lastNode = nextVpp.nodeIdx;
        while (veh->manualTime >= 1.0)
        {
            if (G_VehUpdatePathPos(&nextVpp, veh->waitNode))
                waitNodeHit = 1;
            veh->manualTime = veh->manualTime - 1.0;
            if (lastNode != nextVpp.nodeIdx)
            {
                LOWORD(nodeIdx) = nextVpp.nodeIdx;
                if (nextVpp.customPath)
                    LOWORD(nodeIdx) = nextVpp.customPath->pathOrder[nextVpp.nodeIdx];
                Scr_AddEntityNum(SLOWORD(nodeIdx), 3u, SCRIPTINSTANCE_SERVER, 0);
                Scr_Notify(ent, scr_const.reached_node, 1u);
            }
        }
        memcpy(veh, &nextVpp, 0xDCu);
        if (veh->manualTime > 0.0)
        {
            if (veh->manualTime >= 1.0
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                    4328,
                    0,
                    "%s",
                    "veh->manualTime < 1.0f"))
            {
                __debugbreak();
            }
            G_VehUpdatePathPos(&nextVpp, -1);
        }
        if (*(_BYTE *)g_vehicleDrawPath->current.integer && *(_BYTE *)g_vehicleDrawPath->current.integer == 49)
            VP_DrawPath(&nextVpp);
        if (veh->pathPos.endOfPath && (veh->flags & 0x100) == 0)
            veh->speed = 0.0f;
        phys->origin[0] = (float)((float)(nextVpp.origin[0] - veh->pathPos.origin[0]) * veh->manualTime)
            + veh->pathPos.origin[0];
        phys->origin[1] = (float)((float)(nextVpp.origin[1] - veh->pathPos.origin[1]) * veh->manualTime)
            + veh->pathPos.origin[1];
        phys->origin[2] = (float)((float)(nextVpp.origin[2] - veh->pathPos.origin[2]) * veh->manualTime)
            + veh->pathPos.origin[2];
        v28 = veh->pathPos.angles[0];
        v27 = nextVpp.angles[0];
        manualTime = veh->manualTime;
        v4 = AngleNormalize180(nextVpp.angles[0] - v28);
        phys->angles[0] = v4 * manualTime + v28;
        v25 = veh->pathPos.angles[1];
        v24 = nextVpp.angles[1];
        v23 = veh->manualTime;
        v5 = AngleNormalize180(nextVpp.angles[1] - v25);
        phys->angles[1] = v5 * v23 + v25;
        v22 = veh->pathPos.angles[2];
        v21 = nextVpp.angles[2];
        v20 = veh->manualTime;
        v6 = AngleNormalize180(nextVpp.angles[2] - v22);
        phys->angles[2] = v6 * v20 + v22;
        v7 = DiffTrackAngle(phys->angles[0], phys->prevAngles[0], 6.0, 0.05f);
        phys->angles[0] = v7;
        v8 = DiffTrackAngle(phys->angles[1], phys->prevAngles[1], 4.0, 0.05f);
        phys->angles[1] = v8;
        v9 = DiffTrackAngle(phys->angles[2], phys->prevAngles[2], 6.0, 0.05f);
        phys->angles[2] = v9;
        if ((veh->flags & 0x100) != 0)
        {
            veh->pathPos.lookPos[0] = (float)((float)(nextVpp.lookPos[0] - veh->pathPos.lookPos[0]) * veh->manualTime)
                + veh->pathPos.lookPos[0];
            veh->pathPos.lookPos[1] = (float)((float)(nextVpp.lookPos[1] - veh->pathPos.lookPos[1]) * veh->manualTime)
                + veh->pathPos.lookPos[1];
            veh->pathPos.lookPos[2] = (float)((float)(nextVpp.lookPos[2] - veh->pathPos.lookPos[2]) * veh->manualTime)
                + veh->pathPos.lookPos[2];
        }
        if ((!info->type || info->type == 2) && (veh->flags & 0x100) == 0)
            VEH_GroundPlant(ent, phys, 1);
        if (g_vehicleDebug->current.enabled)
            VEH_DebugBox(veh->pathPos.lookPos, 8.0, 0.0, 1.0, 1.0);
        if (!veh->nitrousVehicle || (veh->nitrousVehicle->m_flags & 0x80) != 0)
        {
            vel = phys->vel;
            prevOrigin = phys->prevOrigin;
            phys->vel[0] = phys->origin[0] - phys->prevOrigin[0];
            vel[1] = phys->origin[1] - prevOrigin[1];
            vel[2] = phys->origin[2] - prevOrigin[2];
            v17 = phys->vel;
            v16 = phys->vel;
            phys->vel[0] = 20.0 * phys->vel[0];
            v17[1] = 20.0 * v16[1];
            v17[2] = 20.0 * v16[2];
            bodyVel = phys->bodyVel;
            phys->bodyVel[0] = 0.0f;
            bodyVel[1] = 0.0f;
            bodyVel[2] = 0.0f;
            phys->bodyVel[0] = veh->speed;
            AnglesSubtract(phys->angles, phys->prevAngles, phys->rotVel);
            rotVel = phys->rotVel;
            v13 = phys->rotVel;
            phys->rotVel[0] = 20.0 * phys->rotVel[0];
            rotVel[1] = 20.0 * v13[1];
            rotVel[2] = 20.0 * v13[2];
        }
        if (waitNodeHit && veh->waitNode > -1)
            Scr_Notify(ent, scr_const.reached_wait_node, 0);
        if (nextVpp.endOfPath)
        {
            Scr_Notify(ent, scr_const.reached_end_node, 0);
            LOWORD(v12) = veh->pathPos.nodeIdx;
            if (nextVpp.customPath)
                LOWORD(v12) = nextVpp.customPath->pathOrder[veh->pathPos.nodeIdx];
            Scr_AddEntityNum((__int16)v12, 3u, SCRIPTINSTANCE_SERVER, 0);
            Scr_Notify(ent, scr_const.reached_node, 1u);
            if ((veh->flags & 0x100) != 0)
            {
                if (veh->nitrousVehicle)
                {
                    //NitrousVehicle::end_path(veh->nitrousVehicle);
                    veh->nitrousVehicle->end_path();
                }
                if (veh->pathPos.customPath)
                {
                    VP_FreeCustomPath(veh->pathPos.customPath);
                    veh->pathPos.customPath = 0;
                    if (veh->nitrousVehicle)
                    {
                        //goalSpeed = NitrousVehicle::get_max_speed(veh->nitrousVehicle, 1);
                        goalSpeed = veh->nitrousVehicle->get_max_speed(true);
                        if (veh->manualMode == 1)
                            goalSpeed = veh->manualSpeed;
                        Phys_Vec3ToNitrousVec(veh->goalPosition, &goalPosition);
                        //NitrousVehicleController::SetScriptTarget(
                            veh->nitrousVehicle->mVehicleController.SetScriptTarget(
                            veh->nitrousVehicle,
                            &goalPosition,
                            veh->hover.hoverRadius,
                            goalSpeed,
                            veh->stopAtGoal != 0);
                    }
                    veh->flags &= 0xFFFFFE7F;
                    veh->flags |= 2u;
                    veh->moveState = VEH_MOVESTATE_MOVE;
                    veh->stopping = 0;
                }
            }
        }
        if (veh->waitSpeed >= 0.0
            && (veh->waitSpeed >= prevSpeed && veh->speed >= veh->waitSpeed
                || prevSpeed >= veh->waitSpeed && veh->waitSpeed >= veh->speed))
        {
            Scr_Notify(ent, scr_const.reached_wait_speed, 0);
        }
    }
}

void __cdecl VEH_UpdateAIMove(gentity_s *ent)
{
    scr_vehicle_s *veh; // [esp+4h] [ebp-4h]

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 5877, 0, "%s", "ent") )
        __debugbreak();
    if ( !ent->scr_vehicle
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                    5878,
                    0,
                    "%s",
                    "ent->scr_vehicle") )
    {
        __debugbreak();
    }
    veh = ent->scr_vehicle;
    switch ( veh->moveState )
    {
        case VEH_MOVESTATE_MOVE:
            VEH_UpdateMoveToGoal(ent, veh->goalPosition);
            break;
        case VEH_MOVESTATE_HOVER:
            VEH_UpdateHover(ent);
            break;
        case VEH_MOVESTATE_PLANE_ONCURVE:
            VEH_UpdatePlaneOnCurve(ent);
            break;
        case VEH_MOVESTATE_PLANE_FREE:
            VEH_UpdatePlaneFree(ent);
            break;
        default:
            return;
    }
}

void __cdecl VEH_UpdateMoveToGoal(gentity_s *ent, const float *goalPos)
{
    double v2; // st7
    float *vel; // [esp+14h] [ebp-150h]
    float v4; // [esp+18h] [ebp-14Ch]
    float *accel; // [esp+20h] [ebp-144h]
    float v6; // [esp+24h] [ebp-140h]
    float *v7; // [esp+28h] [ebp-13Ch]
    float *v8; // [esp+30h] [ebp-134h]
    float speed; // [esp+34h] [ebp-130h]
    float *v10; // [esp+3Ch] [ebp-128h]
    float v11; // [esp+48h] [ebp-11Ch]
    float alignedVel_4; // [esp+80h] [ebp-E4h]
    float alignedVel_8; // [esp+84h] [ebp-E0h]
    float axis[3][3]; // [esp+88h] [ebp-DCh] BYREF
    float dotVec[2]; // [esp+ACh] [ebp-B8h] BYREF
    float dt; // [esp+B4h] [ebp-B0h]
    float v17; // [esp+B8h] [ebp-ACh] BYREF
    float v18; // [esp+BCh] [ebp-A8h]
    float v19; // [esp+C0h] [ebp-A4h]
    float newVerticalSpeed; // [esp+C4h] [ebp-A0h] BYREF
    float startMoveAngle; // [esp+C8h] [ebp-9Ch]
    float desiredVel[3]; // [esp+CCh] [ebp-98h]
    float newSpeed; // [esp+D8h] [ebp-8Ch] BYREF
    float accelMax; // [esp+DCh] [ebp-88h] BYREF
    float accelMaxDt; // [esp+E0h] [ebp-84h]
    float prevVel[3]; // [esp+E4h] [ebp-80h]
    float accelMagnitude; // [esp+F0h] [ebp-74h]
    float averageVel[3]; // [esp+F4h] [ebp-70h]
    float accelVec[3]; // [esp+100h] [ebp-64h] BYREF
    float speedScale; // [esp+10Ch] [ebp-58h]
    float testPoint[3]; // [esp+110h] [ebp-54h] BYREF
    float lookAheadTime; // [esp+11Ch] [ebp-48h]
    float desiredDir[3]; // [esp+120h] [ebp-44h] BYREF
    vehicle_physic_t *phys; // [esp+12Ch] [ebp-38h]
    const vehicle_info_t *info; // [esp+130h] [ebp-34h]
    scr_vehicle_s *veh; // [esp+134h] [ebp-30h]
    float vecToGoal[3]; // [esp+138h] [ebp-2Ch] BYREF
    float distToGoal; // [esp+144h] [ebp-20h]
    int hovering; // [esp+148h] [ebp-1Ch]
    bool hasGoalhanged; // [esp+14Fh] [ebp-15h]
    float desiredYaw; // [esp+150h] [ebp-14h]
    float realGoalPos[3]; // [esp+154h] [ebp-10h] BYREF
    float angleDiff; // [esp+160h] [ebp-4h]

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 5248, 0, "%s", "ent") )
        __debugbreak();
    if ( !ent->scr_vehicle
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                    5249,
                    0,
                    "%s",
                    "ent->scr_vehicle") )
    {
        __debugbreak();
    }
    veh = ent->scr_vehicle;
    phys = &veh->phys;
    hovering = VEH_IsHovering(veh);
    info = BG_GetVehicleInfo(veh->infoIdx);
    realGoalPos[0] = *goalPos;
    realGoalPos[1] = goalPos[1];
    realGoalPos[2] = goalPos[2];
    if ( (veh->flags & 0x200) != 0 )
    {
        lookAheadTime = 1.0f;
        testPoint[0] = (float)(1.0 * veh->phys.vel[0]) + veh->phys.origin[0];
        testPoint[1] = (float)(1.0 * veh->phys.vel[1]) + veh->phys.origin[1];
        testPoint[2] = (float)(1.0 * veh->phys.vel[2]) + veh->phys.origin[2];
        testPoint[2] = testPoint[2] + veh->phys.mins[2];
        if ( !CM_GetHeliHeight(testPoint, 200.0, &veh->phys.heliLockHeight) )
            CM_GetHeliHeight(testPoint, 3000.0, &veh->phys.heliLockHeight);
        realGoalPos[2] = veh->phys.heliLockHeight - veh->phys.mins[2];
    }
    vecToGoal[0] = realGoalPos[0] - phys->origin[0];
    vecToGoal[1] = realGoalPos[1] - phys->origin[1];
    vecToGoal[2] = realGoalPos[2] - phys->origin[2];
    if ( g_vehicleDebug->current.enabled )
        VEH_DebugLine(phys->origin, realGoalPos, 0.5, 1.0, 0.5);
    if ( info->type != 6 )
        vecToGoal[2] = 0.0f;
    distToGoal = Vec3Length(vecToGoal);
    desiredDir[0] = (float)(1.0 / distToGoal) * vecToGoal[0];
    desiredDir[1] = (float)(1.0 / distToGoal) * vecToGoal[1];
    desiredDir[2] = (float)(1.0 / distToGoal) * vecToGoal[2];
    desiredYaw = VEH_UpdateMove_GetDesiredYaw(veh, desiredDir);
    hasGoalhanged = VEH_CheckIfGoalYawChanged(ent, desiredYaw);
    if ( distToGoal <= 0.0 )
    {
        vel = phys->vel;
        phys->vel[0] = 0.0f;
        vel[1] = 0.0f;
        vel[2] = 0.0f;
        if ( info->type == 2 )
            VEH_GroundPlant(ent, phys, 1);
    }
    else
    {
        dt = 0.05f;
        startMoveAngle = 30.0f;
        if ( info->type == 2 )
            VEH_UpdateYawAngularVel(ent, desiredYaw);
        prevVel[0] = phys->vel[0];
        prevVel[1] = phys->vel[1];
        prevVel[2] = phys->vel[2];
        if ( veh->stopAtGoal )
            dt = VEH_UpdateMove_CheckStop(veh, distToGoal);
        if ( (veh->flags & 0x100) != 0 || info->type != 2 )
            startMoveAngle = 90.0f;
        angleDiff = AngleNormalize180(desiredYaw - phys->angles[1]);
        if ( startMoveAngle <= fabs(angleDiff) )
            speedScale = 0.0f;
        else
            speedScale = 1.0 - (float)(angleDiff / startMoveAngle);
        VEH_GetNewSpeedAndAccel(veh, dt, hovering, speedScale, &newSpeed, &accelMax);
        VEH_GetNewSpeedAndAccel(veh, dt, hovering, 0.5, &newVerticalSpeed, &accelMax);
        if ( info->type == 6 && newVerticalSpeed > (float)(vehHelicopterMaxSpeedVertical->current.value * 17.6) )
            newVerticalSpeed = vehHelicopterMaxSpeedVertical->current.value * 17.6;
        v17 = (float)(1.0 / distToGoal) * vecToGoal[0];
        v18 = (float)(1.0 / distToGoal) * vecToGoal[1];
        v19 = (float)(1.0 / distToGoal) * vecToGoal[2];
        desiredVel[0] = newSpeed * v17;
        desiredVel[1] = newSpeed * v18;
        desiredVel[2] = v19 * newVerticalSpeed;
        accelVec[0] = (float)(newSpeed * v17) - prevVel[0];
        accelVec[1] = (float)(newSpeed * v18) - prevVel[1];
        accelVec[2] = (float)(v19 * newVerticalSpeed) - prevVel[2];
        if ( !veh->stopping
            && veh->manualSpeed >= veh->speed
            && (float)((float)(prevVel[0] * desiredVel[0]) + (float)(prevVel[1] * desiredVel[1])) > 0.0
            && (float)((float)(prevVel[0] * accelVec[0]) + (float)(prevVel[1] * accelVec[1])) < 0.0 )
        {
            dotVec[0] = prevVel[0];
            dotVec[1] = prevVel[1];
            Vec2Normalize(dotVec);
            v11 = (float)(dotVec[0] * accelVec[0]) + (float)(dotVec[1] * accelVec[1]);
            dotVec[0] = v11 * dotVec[0];
            dotVec[1] = v11 * dotVec[1];
            accelVec[0] = accelVec[0] - dotVec[0];
            accelVec[1] = accelVec[1] - dotVec[1];
        }
        accelMaxDt = accelMax * dt;
        accelMagnitude = Vec2Length(accelVec);
        if ( accelMagnitude > accelMaxDt )
        {
            if ( accelMagnitude == 0.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                            5405,
                            0,
                            "%s",
                            "accelMagnitude") )
            {
                __debugbreak();
            }
            accelVec[0] = (float)(accelMaxDt / accelMagnitude) * accelVec[0];
            accelVec[1] = (float)(accelMaxDt / accelMagnitude) * accelVec[1];
        }
        accelMaxDt = accelMaxDt * 0.5;
        if ( accelVec[2] <= accelMaxDt )
        {
            if ( (-(accelMaxDt)) > accelVec[2] )
                accelVec[2] = -accelMaxDt;
        }
        else
        {
            accelVec[2] = accelMaxDt;
        }
        if ( !hovering )
        {
            VEH_CheckHorizontalVelocityToGoal(veh, vecToGoal, accelMax, accelVec);
            if ( vecToGoal[2] != 0.0 )
                VEH_CheckVerticalVelocityToGoal(veh, vecToGoal[2], accelVec);
        }
        v10 = phys->vel;
        phys->vel[0] = prevVel[0] + accelVec[0];
        v10[1] = prevVel[1] + accelVec[1];
        v10[2] = prevVel[2] + accelVec[2];
        v2 = Vec3Length(phys->vel);
        veh->speed = v2;
        if ( info->type == 2 )
        {
            AnglesToAxis(ent->r.currentAngles, axis);
            speed = veh->speed;
            alignedVel_4 = speed * axis[0][1];
            alignedVel_8 = speed * axis[0][2];
            v8 = phys->vel;
            phys->vel[0] = speed * axis[0][0];
            v8[1] = alignedVel_4;
            v8[2] = alignedVel_8;
        }
        Vec3Lerp(phys->accel, accelVec, 0.5, phys->accel);
        accelMagnitude = Vec3Length(phys->accel);
        if ( accelMagnitude > accelMaxDt && !veh->stopAtGoal )
        {
            accel = phys->accel;
            v6 = accelMaxDt / accelMagnitude;
            v7 = phys->accel;
            phys->accel[0] = (float)(accelMaxDt / accelMagnitude) * phys->accel[0];
            accel[1] = v6 * v7[1];
            accel[2] = v6 * v7[2];
        }
        averageVel[0] = prevVel[0] + phys->vel[0];
        averageVel[1] = prevVel[1] + phys->vel[1];
        averageVel[2] = prevVel[2] + phys->vel[2];
        averageVel[0] = 0.5 * averageVel[0];
        averageVel[1] = 0.5 * averageVel[1];
        averageVel[2] = 0.5 * averageVel[2];
        if ( dt < 0.05f )
        {
            v4 = 0.05f - dt;
            phys->origin[0] = (float)((float)(0.05f - dt) * prevVel[0]) + phys->origin[0];
            phys->origin[1] = (float)(v4 * prevVel[1]) + phys->origin[1];
            phys->origin[2] = (float)(v4 * prevVel[2]) + phys->origin[2];
        }
        phys->origin[0] = (float)(dt * averageVel[0]) + phys->origin[0];
        phys->origin[1] = (float)(dt * averageVel[1]) + phys->origin[1];
        phys->origin[2] = (float)(dt * averageVel[2]) + phys->origin[2];
        if ( info->type != 2 )
            VEH_UpdateMoveOrientation(ent, &v17);
        if ( info->type == 6 )
            VEH_UpdateVelocityWithRotation(ent);
        if ( info->type == 2 )
            VEH_GroundPlant(ent, phys, 1);
        if ( BG_GetVehicleInfo(veh->infoIdx)->engineSndSpeed == 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                        5472,
                        0,
                        "%s",
                        "BG_GetVehicleInfo(veh->infoIdx)->engineSndSpeed") )
        {
            __debugbreak();
        }
        if ( !hovering )
        {
            VEH_UpdateMove_CheckNearGoal(ent, distToGoal);
            VEH_UpdateMove_CheckGoalReached(ent, distToGoal);
        }
    }
}

bool __cdecl VEH_IsHovering(scr_vehicle_s *veh)
{
    if ( !veh && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 4438, 0, "%s", "veh") )
        __debugbreak();
    return veh->moveState == VEH_MOVESTATE_HOVER;
}

double __cdecl VEH_UpdateMove_GetDesiredYaw(scr_vehicle_s *veh, float *desiredDir)
{
    float v3; // [esp+4h] [ebp-34h]
    float stopAngle; // [esp+10h] [ebp-28h]
    float timeToTurn; // [esp+14h] [ebp-24h]
    float timeToStop; // [esp+18h] [ebp-20h]
    float angleDiff; // [esp+1Ch] [ebp-1Ch]
    float vec[3]; // [esp+20h] [ebp-18h] BYREF
    gentity_s *lookAtEnt; // [esp+2Ch] [ebp-Ch]
    vehicle_physic_t *phys; // [esp+30h] [ebp-8h]
    float desiredYaw; // [esp+34h] [ebp-4h]

    phys = &veh->phys;
    //if ( EntHandle::isDefined(&veh->lookAtEnt) )
    if ( veh->lookAtEnt.isDefined() )
    {
        //lookAtEnt = EntHandle::ent(&veh->lookAtEnt);
        lookAtEnt = veh->lookAtEnt.ent();
        vec[0] = lookAtEnt->r.currentOrigin[0] - phys->origin[0];
        vec[1] = lookAtEnt->r.currentOrigin[1] - phys->origin[1];
        vec[2] = lookAtEnt->r.currentOrigin[2] - phys->origin[2];
        return (float)vectoyaw(vec);
    }
    else
    {
        if ( veh->hasGoalYaw && (veh->stopping || VEH_IsHovering(veh)) )
        {
            desiredYaw = veh->goalYaw;
            if ( phys->maxAngleVel[1] == 0.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                            4553,
                            0,
                            "%s",
                            "phys->maxAngleVel[YAW]") )
            {
                __debugbreak();
            }
            if ( veh->manualDecel == 0.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                            4554,
                            0,
                            "%s",
                            "veh->manualDecel") )
            {
                __debugbreak();
            }
            timeToStop = veh->speed / veh->manualDecel;
            timeToTurn = (float)(phys->maxAngleVel[1] / phys->yawAccel) * 2.0;
            stopAngle = (float)(phys->maxAngleVel[1] * 0.5) * timeToTurn;
            v3 = AngleNormalize180(desiredYaw - phys->angles[1]);
            angleDiff = fabs(v3);
            if ( timeToStop > timeToTurn && angleDiff > stopAngle )
                timeToTurn = (float)((float)(angleDiff - stopAngle) / phys->maxAngleVel[1]) + timeToTurn;
            if ( timeToTurn >= timeToStop )
                return desiredYaw;
        }
        if ( veh->hasTargetYaw )
        {
            return veh->targetYaw;
        }
        else if ( VEH_IsHovering(veh) )
        {
            return phys->angles[1];
        }
        else
        {
            return (float)vectoyaw(desiredDir);
        }
    }
}

void __cdecl VEH_UpdateMoveOrientation(gentity_s *ent, float *desiredDir)
{
    float timeToGoal; // [esp+34h] [ebp-50h]
    float stoppingTime; // [esp+38h] [ebp-4Ch]
    scr_vehicle_s *veh; // [esp+44h] [ebp-40h]
    float angularAccel; // [esp+48h] [ebp-3Ch]
    float accelFraction; // [esp+4Ch] [ebp-38h]
    float angle; // [esp+50h] [ebp-34h]
    float horizontalAccel; // [esp+54h] [ebp-30h]
    float bodyDir; // [esp+58h] [ebp-2Ch]
    float bodyDir_4; // [esp+5Ch] [ebp-28h]
    float desiredYaw; // [esp+60h] [ebp-24h]
    float accelVec[3]; // [esp+64h] [ebp-20h] BYREF
    float accelFactor; // [esp+70h] [ebp-14h]
    float perpDir[2]; // [esp+74h] [ebp-10h]
    float dot; // [esp+7Ch] [ebp-8h]
    float stoppingFactor; // [esp+80h] [ebp-4h]

    veh = ent->scr_vehicle;
    desiredYaw = VEH_UpdateMove_GetDesiredYaw(veh, desiredDir);
    VEH_UpdateYawAndNotify(ent, desiredYaw);
    accelVec[0] = veh->phys.accel[0];
    accelVec[1] = veh->phys.accel[1];
    accelVec[2] = veh->phys.accel[2];
    VEH_AddFakeDrag(veh->phys.vel, veh->maxDragSpeed, accelVec);
    horizontalAccel = Vec2Length(accelVec) / 0.05f;
    Vec3Normalize(accelVec);
    angle = veh->phys.angles[1] * 0.017453292;
    bodyDir = cos(angle);
    bodyDir_4 = sin(angle);
    accelFraction = VEH_CalcAccelFraction(horizontalAccel, veh->infoIdx);
    stoppingFactor = 1.0f;
    if ( veh->stopping && horizontalAccel > 0.0 )
    {
        if ( horizontalAccel == 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                        4806,
                        0,
                        "%s",
                        "horizontalAccel") )
        {
            __debugbreak();
        }
        timeToGoal = Vec2Length(veh->phys.vel) / horizontalAccel;
        stoppingTime = VEH_CalcStoppingTime(horizontalAccel, accelFraction);
        if ( stoppingTime == 0.0
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 4810, 0, "%s", "stoppingTime") )
        {
            __debugbreak();
        }
        if ( stoppingTime > timeToGoal )
            stoppingFactor = timeToGoal / stoppingTime;
    }
    dot = (float)(bodyDir * accelVec[0]) + (float)(bodyDir_4 * accelVec[1]);
    dot = dot * stoppingFactor;
    accelFactor = VEH_CalcAngleForAccel(accelFraction);
    angularAccel = VEH_GetAccelForAngles(veh);
    VEH_UpdateAngleAndAngularVel(
        0,
        (float)(veh->phys.maxPitchAngle * dot) * accelFactor,
        angularAccel,
        angularAccel * 0.40000001,
        0.0,
        &veh->phys);
    perpDir[0] = bodyDir_4;
    perpDir[1] = -bodyDir;
    VEH_UpdateAngleAndAngularVel(
        2,
        (float)(veh->phys.maxRollAngle * (float)((float)((float)(bodyDir_4 * accelVec[0]) + (float)((-(bodyDir)) * accelVec[1])) * stoppingFactor))
    * accelFactor,
        angularAccel,
        angularAccel * 0.40000001,
        0.0,
        &veh->phys);
}

void __cdecl VEH_UpdateAngleAndAngularVel(
                int index,
                float desiredAngle,
                float accel,
                float decel,
                float overShoot,
                vehicle_physic_t *phys)
{
    float absCurAngleVel; // [esp+30h] [ebp-10h]
    float effectiveAccel; // [esp+34h] [ebp-Ch]
    float targetAngleVel; // [esp+38h] [ebp-8h]
    float angleDiff; // [esp+3Ch] [ebp-4h]

    angleDiff = AngleNormalize180(desiredAngle - phys->angles[index]);
    if ( (float)(0.0099999998 * 0.0099999998) <= (float)(angleDiff * angleDiff)
        || (float)(0.05f * 0.05f) <= (float)(phys->rotVel[index] * phys->rotVel[index]) )
    {
        absCurAngleVel = fabs(phys->rotVel[index]);
        targetAngleVel = phys->maxAngleVel[index];
        if ( (float)(angleDiff * phys->rotVel[index]) < 0.0 )
        {
            effectiveAccel = accel;
        }
        else
        {
            if ( decel == 0.0
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 4470, 0, "%s", "decel") )
            {
                __debugbreak();
            }
            if ( (overShoot < 0.0 || overShoot >= 1.0)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                            4474,
                            0,
                            "%s",
                            "overShoot >= 0.f && overShoot < 1.f") )
            {
                __debugbreak();
            }
            if ( (float)((float)(1.0 - overShoot) * (float)((float)(absCurAngleVel * 0.5) * (float)(absCurAngleVel / decel))) < fabs(angleDiff) )
            {
                effectiveAccel = accel;
            }
            else
            {
                targetAngleVel = 0.0f;
                effectiveAccel = decel;
            }
        }
        if (angleDiff < 0.0)
        {
            //LODWORD(targetAngleVel) ^= _mask__NegFloat_;
            targetAngleVel = -targetAngleVel;
        }
        if ( (float)(effectiveAccel * 0.05f) <= absCurAngleVel
            || (float)(absCurAngleVel * 0.05f) <= fabs(angleDiff) )
        {
            phys->rotVel[index] = VEH_AccelerateSpeed(phys->rotVel[index], targetAngleVel, effectiveAccel, 0.05f);
            phys->angles[index] = (float)(phys->rotVel[index] * 0.05f) + phys->angles[index];
            phys->angles[index] = AngleNormalize180(phys->angles[index]);
        }
        else
        {
            phys->angles[index] = desiredAngle;
            phys->rotVel[index] = 0.0f;
        }
    }
    else
    {
        phys->rotVel[index] = 0.0f;
        phys->angles[index] = desiredAngle;
    }
}

double __cdecl VEH_CalcAccelFraction(float accel, int infoIdx)
{
    float v3; // [esp+0h] [ebp-14h]
    float v4; // [esp+4h] [ebp-10h]
    float minAccel; // [esp+8h] [ebp-Ch]
    float maxAccel; // [esp+10h] [ebp-4h]

    minAccel = 0.0 * 17.6;
    maxAccel = BG_GetVehicleInfo(infoIdx)->accel;
    if ( maxAccel <= (float)(0.0 * 17.6)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                    4593,
                    0,
                    "%s",
                    "maxAccel > minAccel") )
    {
        __debugbreak();
    }
    if ( (float)(accel - maxAccel) < 0.0 )
        v4 = accel;
    else
        v4 = maxAccel;
    if ( (float)(minAccel - accel) < 0.0 )
        v3 = v4;
    else
        v3 = 0.0 * 17.6;
    return (v3 - minAccel) / (maxAccel - minAccel);
}

double __cdecl VEH_CalcAngleForAccel(float accelFraction)
{
    return 1.0 * accelFraction + (1.0 - accelFraction) * 0.1;
}

double __cdecl VEH_CalcStoppingTime(float accel, float accelFraction)
{
    return 3.5 * (float)(1.0 - accelFraction) + (1.0 - (float)(1.0 - accelFraction)) * 2.5;
}

void __cdecl VEH_UpdateYawAndNotify(gentity_s *ent, float desiredYaw)
{
    bool v2; // [esp+14h] [ebp-30h]
    float finalYawDiff; // [esp+18h] [ebp-2Ch]
    float yawDecel; // [esp+20h] [ebp-24h]
    scr_vehicle_s *veh; // [esp+28h] [ebp-1Ch]
    float initialYawDiff; // [esp+2Ch] [ebp-18h]
    float overshoot; // [esp+30h] [ebp-14h]
    float yawAccel; // [esp+34h] [ebp-10h]
    float initalVel; // [esp+38h] [ebp-Ch]

    initialYawDiff = 0.0f;
    veh = ent->scr_vehicle;
    if ( veh->hasGoalYaw )
    {
        initialYawDiff = AngleDelta(veh->phys.angles[1], veh->goalYaw);
    }
    else if ( veh->hasTargetYaw )
    {
        initialYawDiff = AngleDelta(veh->phys.angles[1], veh->targetYaw);
    }
    v2 = (veh->hasGoalYaw || veh->hasTargetYaw) && fabs(initialYawDiff) >= 1.5;
    initalVel = veh->phys.rotVel[1];
    yawAccel = veh->phys.yawAccel;
    yawDecel = veh->phys.yawDecel;
    if ( desiredYaw != veh->prevGoalYaw )
    {
        veh->yawSlowDown = 0;
        veh->prevGoalYaw = desiredYaw;
    }
    if ( veh->yawSlowDown )
    {
        yawAccel = yawAccel * 0.2;
        yawDecel = yawDecel * 0.2;
    }
    if ( veh->hasGoalYaw || veh->hasTargetYaw )
        overshoot = veh->yawOverShoot;
    else
        overshoot = 0.0f;
    VEH_UpdateAngleAndAngularVel(1, desiredYaw, yawAccel, yawDecel, overshoot, &veh->phys);
    if ( veh->hasGoalYaw && (float)(veh->phys.rotVel[1] * initalVel) < 0.0 )
        veh->yawSlowDown = 1;
    if ( v2 )
    {
        if ( veh->hasGoalYaw )
            finalYawDiff = AngleDelta(veh->phys.angles[1], veh->goalYaw);
        else
            finalYawDiff = AngleDelta(veh->phys.angles[1], veh->targetYaw);
        if ( (float)(finalYawDiff * initialYawDiff) < 0.0 || fabs(finalYawDiff) <= 1.5 )
            Scr_Notify(ent, scr_const.goal_yaw, 0);
    }
}

double __cdecl VEH_GetAccelForAngles(scr_vehicle_s *veh)
{
    if ( !veh && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 4690, 0, "%s", "veh") )
        __debugbreak();
    return veh->phys.yawAccel;
}

void __cdecl VEH_AddFakeDrag(const float *velocity, float maxDragSpeed, float *accelVec)
{
    float v3; // [esp+0h] [ebp-2Ch]
    float v4; // [esp+4h] [ebp-28h]
    float horizontalVel; // [esp+20h] [ebp-Ch]
    float velocityVec[2]; // [esp+24h] [ebp-8h] BYREF

    velocityVec[0] = *velocity;
    velocityVec[1] = velocity[1];
    horizontalVel = Vec2Length(velocityVec);
    if ( (float)(maxDragSpeed - horizontalVel) < 0.0 )
        v3 = maxDragSpeed;
    else
        v3 = horizontalVel;
    if ( maxDragSpeed == 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 4715, 0, "%s", "maxDragSpeed") )
    {
        __debugbreak();
    }
    Vec2Normalize(velocityVec);
    v4 = 5.0 * (float)((float)(v3 / maxDragSpeed) * (float)(v3 / maxDragSpeed));
    velocityVec[0] = v4 * velocityVec[0];
    velocityVec[1] = v4 * velocityVec[1];
    *accelVec = *accelVec + velocityVec[0];
    accelVec[1] = accelVec[1] + velocityVec[1];
}

void __cdecl VEH_CheckHorizontalVelocityToGoal(
                scr_vehicle_s *veh,
                const float *vecToGoal,
                float accelMax,
                float *accelVec)
{
    float turningAbility; // [esp+0h] [ebp-94h]
    float *v5; // [esp+4h] [ebp-90h]
    float *v6; // [esp+8h] [ebp-8Ch]
    float *vel; // [esp+24h] [ebp-70h]
    float breakVec; // [esp+40h] [ebp-54h]
    float breakVec_4; // [esp+44h] [ebp-50h]
    float breakingAccel; // [esp+48h] [ebp-4Ch]
    float radius; // [esp+54h] [ebp-40h]
    float oldSpeed; // [esp+58h] [ebp-3Ch]
    float requiredDecel; // [esp+5Ch] [ebp-38h]
    float actualDecel; // [esp+64h] [ebp-30h]
    float newSpeed; // [esp+68h] [ebp-2Ch]
    float newVel[3]; // [esp+6Ch] [ebp-28h] BYREF
    vehicle_physic_t *phys; // [esp+78h] [ebp-1Ch]
    float horizontalDist; // [esp+7Ch] [ebp-18h]
    float horizontalSpeed; // [esp+80h] [ebp-14h]
    float perpDir[2]; // [esp+84h] [ebp-10h]
    float radiusVec[2]; // [esp+8Ch] [ebp-8h]

    phys = &veh->phys;
    horizontalDist = Vec2Length(vecToGoal);
    if ( horizontalDist >= 1.0 )
    {
        horizontalSpeed = Vec2Length(phys->vel);
        if ( veh->stopAtGoal
            && (newVel[0] = phys->vel[0] + *accelVec,
                    newVel[1] = phys->vel[1] + accelVec[1],
                    newSpeed = Vec2Length(newVel),
                    newSpeed > 0.0) )
        {
            oldSpeed = Vec2Length(phys->vel);
            requiredDecel = (float)((float)(horizontalSpeed * horizontalSpeed) / (float)(2.0 * horizontalDist)) * 0.05f;
            if ( requiredDecel > fabs(newSpeed - oldSpeed) )
            {
                newVel[0] = (float)((float)(oldSpeed - requiredDecel) / newSpeed) * newVel[0];
                newVel[1] = (float)((float)(oldSpeed - requiredDecel) / newSpeed) * newVel[1];
                vel = phys->vel;
                *accelVec = newVel[0] - phys->vel[0];
                accelVec[1] = newVel[1] - vel[1];
                actualDecel = Vec3Length(accelVec);
                if ( actualDecel > accelMax )
                {
                    *accelVec = (float)(accelMax / actualDecel) * *accelVec;
                    accelVec[1] = (float)(accelMax / actualDecel) * accelVec[1];
                    accelVec[2] = (float)(accelMax / actualDecel) * accelVec[2];
                }
            }
        }
        else
        {
            perpDir[0] = phys->vel[1];
            perpDir[1] = -phys->vel[0];
            if ( horizontalDist == 0.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                            4884,
                            0,
                            "%s",
                            "horizontalDist") )
            {
                __debugbreak();
            }
            radiusVec[0] = fabs((float)(perpDir[0] * *vecToGoal) + (float)(perpDir[1] * vecToGoal[1]));
            if ( radiusVec[0] > horizontalDist )
            {
                radiusVec[1] = (float)(phys->vel[0] * *vecToGoal) + (float)(phys->vel[1] * vecToGoal[1]);
                radiusVec[0] = (float)(1.0 / horizontalSpeed) * radiusVec[0];
                radiusVec[1] = (float)(1.0 / horizontalSpeed) * radiusVec[1];
                radius = (float)((float)(radiusVec[0] * radiusVec[0]) + (float)(radiusVec[1] * radiusVec[1]))
                             / (float)(2.0 * radiusVec[0]);
                if ( radius <= 0.0
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                                4896,
                                0,
                                "%s",
                                "radius > 0.f") )
                {
                    __debugbreak();
                }
                if ( radius > 1.0 && (float)(horizontalSpeed * horizontalSpeed) > (float)(accelMax * radius) )
                {
                    if ( radius == 0.0
                        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 4906, 0, "%s", "radius") )
                    {
                        __debugbreak();
                    }
                    breakingAccel = (float)(horizontalSpeed * horizontalSpeed) / radius;
                    if ( breakingAccel > horizontalSpeed )
                        breakingAccel = horizontalSpeed;
                    if ( veh->stopAtGoal )
                        turningAbility = 1.0f;
                    else
                        turningAbility = veh->turningAbility;
                    breakVec = (float)((float)((float)(  (-(turningAbility)) * breakingAccel) / horizontalSpeed) * 0.05) * phys->vel[0];
                    breakVec_4 = (float)((float)((float)((-(turningAbility)) * breakingAccel) / horizontalSpeed) * 0.05) * phys->vel[1];
                    v5 = phys->vel;
                    v6 = phys->vel;
                    phys->vel[0] = phys->vel[0] + breakVec;
                    v5[1] = v6[1] + breakVec_4;
                    *accelVec = *accelVec + breakVec;
                    accelVec[1] = accelVec[1] + breakVec_4;
                }
            }
        }
    }
}

void __cdecl VEH_CheckVerticalVelocityToGoal(scr_vehicle_s *veh, float verticalDist, float *accelVec)
{
    float v3; // [esp+0h] [ebp-2Ch]
    float v4; // [esp+8h] [ebp-24h]
    float accelerationCap; // [esp+10h] [ebp-1Ch]
    float breakingAccel; // [esp+14h] [ebp-18h]
    float desiredStoppingTime; // [esp+18h] [ebp-14h]
    float verticalSpeed; // [esp+28h] [ebp-4h]

    verticalSpeed = veh->phys.vel[2];
    if ( fabs((unsigned int)accelVec[2]) >= 0.001
        && fabs(verticalSpeed) >= 0.001
        && (float)(verticalSpeed * accelVec[2]) < 0.0
        && (float)(verticalDist * verticalSpeed) > 0.0 )
    {
        desiredStoppingTime = verticalDist / (float)(verticalSpeed * 0.5);
        //if ( (float)((float)(verticalSpeed * 0.05f) / COERCE_FLOAT(*((unsigned int *)accelVec + 2) ^ _mask__NegFloat_)) > desiredStoppingTime )
        if ( (float)((float)(verticalSpeed * 0.05f) / -accelVec[2]) > desiredStoppingTime)
        {
            if ( desiredStoppingTime == 0.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                            4947,
                            0,
                            "%s",
                            "desiredStoppingTime") )
            {
                __debugbreak();
            }
            breakingAccel = (float)((-(verticalSpeed)) * 0.05f) / desiredStoppingTime;
            if ( (float)(breakingAccel * breakingAccel) > (float)(accelVec[2] * accelVec[2]) )
            {
                accelerationCap = (float)(veh->manualAccel * 0.05f) * 3.0;
                if ( (float)(breakingAccel - accelerationCap) < 0.0 )
                    v4 = (float)((-(verticalSpeed)) * 0.05f) / desiredStoppingTime;
                else
                    v4 = (float)(veh->manualAccel * 0.05f) * 3.0;
                if ( (float)((-(accelerationCap)) - breakingAccel) < 0.0 )
                    v3 = v4;
                else
                    v3 = -accelerationCap;
                veh->phys.vel[2] = (float)(v3 - accelVec[2]) + veh->phys.vel[2];
                accelVec[2] = v3;
            }
        }
    }
}

int __cdecl VEH_UpdateMove_CheckGoalReached(gentity_s *ent, float distToGoal)
{
    bool v3; // [esp+0h] [ebp-20h]
    bool v4; // [esp+4h] [ebp-1Ch]
    bool goalReached; // [esp+17h] [ebp-9h]
    const vehicle_info_t *info; // [esp+18h] [ebp-8h]
    scr_vehicle_s *veh; // [esp+1Ch] [ebp-4h]

    veh = ent->scr_vehicle;
    info = BG_GetVehicleInfo(veh->infoIdx);
    if ( veh->stopAtGoal )
    {
        if ( veh->hover.hoverRadius == 0.0 )
        {
            v3 = (veh->stopping || distToGoal == 0.0) && veh->speed == 0.0;
            goalReached = v3;
        }
        else
        {
            v4 = veh->hover.hoverRadius >= distToGoal && (float)(2.0 * 17.6) > veh->speed;
            goalReached = v4;
        }
        if ( goalReached )
        {
            if ( info->type == 6 )
                veh->moveState = VEH_MOVESTATE_HOVER;
            else
                veh->moveState = VEH_MOVESTATE_STOP;
            if ( veh->hover.hoverRadius == 0.0 )
            {
                veh->phys.accel[0] = 0.0f;
                veh->phys.accel[1] = 0.0f;
                veh->phys.accel[2] = 0.0f;
                veh->phys.vel[0] = 0.0f;
                veh->phys.vel[1] = 0.0f;
                veh->phys.vel[2] = 0.0f;
            }
LABEL_20:
            Scr_Notify(ent, scr_const.goal, 0);
            return 1;
        }
    }
    else if ( (float)(veh->speed * 0.05f) >= distToGoal )
    {
        if ( info->type != 6 && !veh->nitrousVehicle )
            veh->moveState = VEH_MOVESTATE_STOP;
        goto LABEL_20;
    }
    return 0;
}

double __cdecl VEH_UpdateMove_CheckStop(scr_vehicle_s *veh, float distToGoal)
{
    float v3; // [esp+10h] [ebp-1Ch]
    float v4; // [esp+14h] [ebp-18h]
    float dt; // [esp+18h] [ebp-14h]
    float dta; // [esp+18h] [ebp-14h]
    float newSpeed; // [esp+20h] [ebp-Ch]
    float stopDist; // [esp+28h] [ebp-4h]

    dt = 0.05f;
    newSpeed = VEH_AccelerateSpeed(veh->speed, veh->manualSpeed, veh->manualAccel, 0.05f);
    if ( veh->manualDecel == 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                    5043,
                    0,
                    "%s",
                    "veh->manualDecel") )
    {
        __debugbreak();
    }
    stopDist = (float)(newSpeed * 0.5) * (float)(newSpeed / veh->manualDecel);
    if ( stopDist < (float)(distToGoal - (float)(newSpeed * 0.05f)) || veh->speed <= 0.0 )
    {
        veh->stopping = 0;
    }
    else
    {
        if ( distToGoal > stopDist )
        {
            if ( veh->speed == 0.0
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 5055, 0, "%s", "veh->speed") )
            {
                __debugbreak();
            }
            dta = 0.05f - (float)((float)(distToGoal - stopDist) / veh->speed);
            if ( (float)(dta - 0.05f) < 0.0 )
                v4 = 0.05f - (float)((float)(distToGoal - stopDist) / veh->speed);
            else
                v4 = 0.05f;
            if ( (float)(0.0 - dta) < 0.0 )
                v3 = v4;
            else
                v3 = 0.0f;
            dt = v3;
        }
        veh->stopping = 1;
    }
    return dt;
}

void __cdecl VEH_UpdateMove_CheckNearGoal(gentity_s *ent, float distToGoal)
{
    scr_vehicle_s *veh; // [esp+0h] [ebp-4h]

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 5073, 0, "%s", "ent") )
        __debugbreak();
    veh = ent->scr_vehicle;
    if ( veh->nearGoalNotifyDist != 0.0 && veh->nearGoalNotifyDist > distToGoal )
        Scr_Notify(ent, scr_const.near_goal, 0);
}

void __cdecl VEH_UpdateYawAngularVel(gentity_s *ent, float desiredYaw)
{
    float yawDecel; // [esp+24h] [ebp-24h]
    scr_vehicle_s *veh; // [esp+30h] [ebp-18h]
    float overshoot; // [esp+38h] [ebp-10h]
    float yawAccel; // [esp+3Ch] [ebp-Ch]

    veh = ent->scr_vehicle;
    AngleNormalize180(desiredYaw - veh->phys.angles[1]);
    yawAccel = veh->phys.yawAccel;
    yawDecel = veh->phys.yawDecel;
    if ( desiredYaw != veh->prevGoalYaw )
    {
        veh->yawSlowDown = 0;
        veh->prevGoalYaw = desiredYaw;
    }
    if ( veh->yawSlowDown )
    {
        yawAccel = yawAccel * 0.2;
        yawDecel = yawDecel * 0.2;
    }
    if ( veh->hasGoalYaw || veh->hasTargetYaw )
        overshoot = veh->yawOverShoot;
    else
        overshoot = 0.0f;
    if ( BG_GetVehicleInfo(veh->infoIdx)->type == 2 )
        overshoot = 0.0f;
    VEH_UpdateAngleAndAngularVel(1, desiredYaw, yawAccel, yawDecel, overshoot, &veh->phys);
}

bool __cdecl VEH_CheckIfGoalYawChanged(gentity_s *ent, float desiredYaw)
{
    scr_vehicle_s *veh; // [esp+8h] [ebp-4h]

    veh = ent->scr_vehicle;
    return BG_GetVehicleInfo(veh->infoIdx)->type == 2
            && fabs(desiredYaw - veh->phys.angles[1]) > 1.0;
}

void __cdecl VEH_UpdateHover(gentity_s *ent)
{
    double v1; // st7
    scr_vehicle_s *veh; // [esp+14h] [ebp-18h]
    float hoverPos[3]; // [esp+1Ch] [ebp-10h] BYREF
    float newHoverDist; // [esp+28h] [ebp-4h]

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 5493, 0, "%s", "ent") )
        __debugbreak();
    if ( !ent->scr_vehicle
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                    5494,
                    0,
                    "%s",
                    "ent->scr_vehicle") )
    {
        __debugbreak();
    }
    veh = ent->scr_vehicle;
    hoverPos[0] = veh->goalPosition[0] + veh->hover.hoverGoalPos[0];
    hoverPos[1] = veh->goalPosition[1] + veh->hover.hoverGoalPos[1];
    hoverPos[2] = veh->goalPosition[2] + veh->hover.hoverGoalPos[2];
    VEH_UpdateMoveToGoal(ent, hoverPos);
    newHoverDist = veh->hover.hoverRadius * 0.25;
    v1 = Vec3DistanceSq(veh->phys.origin, hoverPos);
    if ( newHoverDist * newHoverDist > v1 )
        VEH_SetHoverGoal(ent);
}

void __cdecl VEH_SetHoverGoal(gentity_s *ent)
{
    int i; // [esp+14h] [ebp-14h]
    scr_vehicle_s *veh; // [esp+18h] [ebp-10h]
    float randomOffset[3]; // [esp+1Ch] [ebp-Ch]

    veh = ent->scr_vehicle;
    if ( veh->hover.hoverRadius == 0.0 )
    {
        veh->hover.hoverGoalPos[0] = 0.0f;
        veh->hover.hoverGoalPos[1] = 0.0f;
        veh->hover.hoverGoalPos[2] = 0.0f;
    }
    else
    {
        for ( i = 0; i < 3; ++i )
            randomOffset[i] = G_flrand((-(veh->hover.hoverRadius)), veh->hover.hoverRadius);
        veh->hover.hoverGoalPos[0] = (float)(-0.5 * veh->hover.hoverGoalPos[0]) + randomOffset[0];
        veh->hover.hoverGoalPos[1] = (float)(-0.5 * veh->hover.hoverGoalPos[1]) + randomOffset[1];
        veh->hover.hoverGoalPos[2] = (float)(-0.5 * veh->hover.hoverGoalPos[2]) + randomOffset[2];
    }
}

void __cdecl VEH_UpdatePlaneOnCurve(gentity_s *ent)
{
    double v1; // st7
    float *vel; // [esp+14h] [ebp-8Ch]
    float speed; // [esp+18h] [ebp-88h]
    float *prevAngles; // [esp+30h] [ebp-70h]
    float *v5; // [esp+34h] [ebp-6Ch]
    float yawDiff; // [esp+3Ch] [ebp-64h]
    float pos[3]; // [esp+40h] [ebp-60h] BYREF
    float frameDist; // [esp+4Ch] [ebp-54h]
    float diff[3]; // [esp+50h] [ebp-50h] BYREF
    bool curveDone; // [esp+5Fh] [ebp-41h]
    vehicle_physic_t *phys; // [esp+60h] [ebp-40h]
    scr_vehicle_s *veh; // [esp+64h] [ebp-3Ch]
    float timeThisFrame; // [esp+68h] [ebp-38h]
    float newSpeed; // [esp+6Ch] [ebp-34h] BYREF
    float accelMax; // [esp+70h] [ebp-30h] BYREF
    float curveTime; // [esp+74h] [ebp-2Ch]
    float angles[3]; // [esp+78h] [ebp-28h] BYREF
    float forwardPos[3]; // [esp+84h] [ebp-1Ch] BYREF
    float length; // [esp+90h] [ebp-10h]
    float prevPos[3]; // [esp+94h] [ebp-Ch]

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 5731, 0, "%s", "ent") )
        __debugbreak();
    if ( !ent->scr_vehicle
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                    5732,
                    0,
                    "%s",
                    "ent->scr_vehicle") )
    {
        __debugbreak();
    }
    length = 0.0f;
    veh = ent->scr_vehicle;
    phys = &veh->phys;
    VEH_GetNewSpeedAndAccel(veh, 0.05f, 0, 1.0, &newSpeed, &accelMax);
    frameDist = newSpeed * 0.05f;
    curveTime = veh->phys.curveTime;
    prevPos[0] = veh->phys.origin[0];
    prevPos[1] = veh->phys.origin[1];
    prevPos[2] = veh->phys.origin[2];
    curveDone = 0;
    while ( frameDist > length && !curveDone )
    {
        curveTime = curveTime + phys->timeStep;
        if ( curveTime > 1.0 )
        {
            curveDone = 1;
            curveTime = 1.0f;
        }
        cCurveManager::GetPos(phys->curveID, curveTime, pos);
        diff[0] = pos[0] - prevPos[0];
        diff[1] = pos[1] - prevPos[1];
        diff[2] = pos[2] - prevPos[2];
        v1 = Vec3Length(diff);
        length = v1 + length;
        prevPos[0] = pos[0];
        prevPos[1] = pos[1];
        prevPos[2] = pos[2];
    }
    timeThisFrame = curveTime - phys->curveTime;
    phys->curveTime = (float)((float)(frameDist / length) * timeThisFrame) + phys->curveTime;
    if ( (float)(phys->curveTime + phys->timeStep) > 1.0 )
    {
        curveDone = 1;
        phys->curveTime = 1.0 - phys->timeStep;
    }
    cCurveManager::GetPos(phys->curveID, phys->curveTime, pos);
    cCurveManager::GetPos(phys->curveID, phys->curveTime + phys->timeStep, forwardPos);
    phys->origin[0] = pos[0];
    phys->origin[1] = pos[1];
    phys->origin[2] = pos[2];
    prevAngles = phys->prevAngles;
    v5 = phys->angles;
    phys->prevAngles[0] = phys->angles[0];
    prevAngles[1] = v5[1];
    prevAngles[2] = v5[2];
    diff[0] = forwardPos[0] - pos[0];
    diff[1] = forwardPos[1] - pos[1];
    diff[2] = forwardPos[2] - pos[2];
    Vec3Normalize(diff);
    vectoangles(diff, angles);
    phys->angles[0] = angles[0];
    phys->angles[1] = angles[1];
    if ( veh->hasGoalRoll == 0.0 )
    {
        if ( phys->curveTime > 0.2 )
        {
            yawDiff = phys->prevAngles[1] - phys->angles[1];
            if ( yawDiff <= 0.0 )
            {
                if ( yawDiff < 0.0 && (-(phys->maxRollAngle)) <= phys->angles[2] && fabs((float)(2.0 * yawDiff) - phys->prevAngles[2]) > 2.0 )
                {
                    phys->angles[2] = phys->angles[2] - 1.0;
                }
            }
            else if ( phys->angles[2] <= phys->maxRollAngle
                         && fabs((float)(2.0 * yawDiff) - phys->prevAngles[2]) > 2.0 )
            {
                phys->angles[2] = phys->angles[2] + 1.0;
            }
        }
    }
    else
    {
        VEH_UpdatePlaneRoll(ent);
    }
    veh->speed = newSpeed;
    speed = veh->speed;
    diff[0] = speed * diff[0];
    diff[1] = speed * diff[1];
    diff[2] = speed * diff[2];
    vel = phys->vel;
    phys->vel[0] = diff[0];
    vel[1] = diff[1];
    vel[2] = diff[2];
    if ( g_vehicleDebug->current.enabled )
    {
        cCurveManager::SetCurveDraw(phys->curveID, 0);
        VEH_DebugPlaneOnCurve(ent);
    }
    if ( (float)(phys->curveTime + phys->timeStep) >= 1.0 || curveDone )
    {
        if ( phys->curveTime >= 1.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                        5841,
                        0,
                        "%s",
                        "phys->curveTime < 1.0f") )
        {
            __debugbreak();
        }
        veh->moveState = VEH_MOVESTATE_PLANE_FREE;
        cCurveManager::FreeCurve(phys->curveID);
        phys->curveTime = 0.0f;
        phys->curveID = -1;
        Scr_Notify(ent, scr_const.curve_end, 0);
    }
}

void __cdecl VEH_DebugPlaneOnCurve(gentity_s *ent)
{
    scr_vehicle_s *veh; // [esp+4h] [ebp-4Ch]
    float left1[3]; // [esp+8h] [ebp-48h] BYREF
    float left2[3]; // [esp+14h] [ebp-3Ch] BYREF
    float right2[3]; // [esp+20h] [ebp-30h] BYREF
    float color[4]; // [esp+2Ch] [ebp-24h] BYREF
    float x; // [esp+3Ch] [ebp-14h]
    float y; // [esp+40h] [ebp-10h]
    float right1[3]; // [esp+44h] [ebp-Ch] BYREF

    color[0] = 1.0f;
    color[1] = 0.0f;
    color[2] = 0.0f;
    color[3] = 1.0f;
    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 5550, 0, "%s", "ent") )
        __debugbreak();
    if ( !ent->scr_vehicle
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                    5551,
                    0,
                    "%s",
                    "ent->scr_vehicle") )
    {
        __debugbreak();
    }
    veh = ent->scr_vehicle;
    x = veh->phys.origin[0];
    y = veh->phys.origin[1];
    left1[0] = x + 1000.0;
    left1[1] = y + 1000.0;
    left1[2] = 0.0f;
    right1[0] = x - 1000.0;
    right1[1] = y - 1000.0;
    right1[2] = 0.0f;
    left2[0] = x + 1000.0;
    left2[1] = y - 1000.0;
    left2[2] = 0.0f;
    right2[0] = x - 1000.0;
    right2[1] = y + 1000.0;
    right2[2] = 0.0f;
    CG_DebugLine(left1, left2, color, 0, 5);
    CG_DebugLine(right1, right2, color, 0, 5);
    CG_DebugLine(left1, right2, color, 0, 5);
    CG_DebugLine(left2, right1, color, 0, 5);
}

void __cdecl VEH_UpdatePlaneRoll(gentity_s *ent)
{
    float v2; // xmm0_4
    scr_vehicle_s *veh; // [esp+10h] [ebp-8h]

    if (!ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 5699, 0, "%s", "ent"))
        __debugbreak();
    if (!ent->scr_vehicle
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
            5700,
            0,
            "%s",
            "ent->scr_vehicle"))
    {
        __debugbreak();
    }
    veh = ent->scr_vehicle;
    if (BG_GetVehicleInfo(veh->infoIdx)->type != 3)
        Com_Error(ERR_DROP, "Vehicle type has to be plane to set Roll. ");
    if ((veh->flags & 2) == 0
        || veh->moveState != VEH_MOVESTATE_PLANE_ONCURVE && veh->moveState != VEH_MOVESTATE_PLANE_FREE)
    {
        Com_Error(ERR_DROP, "Can not set roll on non moving plane. ");
    }
    veh->currentRollTime = veh->currentRollTime + 0.05f;
    //v2 = __libm_sse2_sin((float)((float)((float)(90.0 * veh->currentRollTime) / veh->goalRollTime) * 0.017453292));
    v2 = sin((float)((float)((float)(90.0 * veh->currentRollTime) / veh->goalRollTime) * 0.017453292));
    veh->phys.angles[2] = veh->goalRoll * v2;
    veh->phys.angles[2] = AngleNormalize360(veh->phys.angles[2]);
    if (veh->currentRollTime >= veh->goalRollTime)
        veh->hasGoalRoll = 0.0f;
}

void __cdecl VEH_UpdatePlaneFree(gentity_s *ent)
{
    vehicle_physic_t *phys; // [esp+10h] [ebp-18h]
    float dir[3]; // [esp+14h] [ebp-14h] BYREF
    scr_vehicle_s *veh; // [esp+20h] [ebp-8h]
    float dist; // [esp+24h] [ebp-4h]

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 5855, 0, "%s", "ent") )
        __debugbreak();
    if ( !ent->scr_vehicle
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                    5856,
                    0,
                    "%s",
                    "ent->scr_vehicle") )
    {
        __debugbreak();
    }
    veh = ent->scr_vehicle;
    phys = &veh->phys;
    dir[0] = veh->phys.vel[0];
    dir[1] = veh->phys.vel[1];
    dir[2] = veh->phys.vel[2];
    Vec3Normalize(dir);
    dist = veh->speed * 0.05f;
    dir[0] = dist * dir[0];
    dir[1] = dist * dir[1];
    dir[2] = dist * dir[2];
    phys->origin[0] = phys->origin[0] + dir[0];
    phys->origin[1] = phys->origin[1] + dir[1];
    phys->origin[2] = phys->origin[2] + dir[2];
}

void __cdecl VEH_UpdateChopperPathDrive(gentity_s *ent)
{
    float v1; // [esp+28h] [ebp-58h]
    float v2; // [esp+34h] [ebp-4Ch]
    float v3; // [esp+40h] [ebp-40h]
    float transitionTimeFrac; // [esp+5Ch] [ebp-24h]
    scr_vehicle_s *veh; // [esp+64h] [ebp-1Ch]
    float pathPosAngles; // [esp+68h] [ebp-18h]
    float pathPosAngles_4; // [esp+6Ch] [ebp-14h]
    float pathPosAngles_8; // [esp+70h] [ebp-10h]
    float pathPosOrigin; // [esp+74h] [ebp-Ch]
    float pathPosOrigin_4; // [esp+78h] [ebp-8h]
    float pathPosOrigin_8; // [esp+7Ch] [ebp-4h]

    veh = ent->scr_vehicle;
    if ( veh->pathTransitionTime > 0.0 )
    {
        pathPosOrigin = veh->phys.origin[0];
        pathPosOrigin_4 = veh->phys.origin[1];
        pathPosOrigin_8 = veh->phys.origin[2];
        pathPosAngles = veh->phys.angles[0];
        pathPosAngles_4 = veh->phys.angles[1];
        pathPosAngles_8 = veh->phys.angles[2];
    }
    memcpy(&veh->phys, &s_backup.phys, sizeof(veh->phys));
    veh->targetYaw = veh->pathPos.angles[1];
    veh->hasTargetYaw = 1;
    VEH_UpdateMoveToGoal(ent, veh->pathPos.lookPos);
    if ( g_vehicleDebug->current.enabled )
        VEH_DebugBox(veh->pathPos.lookPos, 18.0, 0.60000002, 1.0, 0.60000002);
    veh->pathPos.origin[0] = veh->phys.origin[0];
    veh->pathPos.origin[1] = veh->phys.origin[1];
    if ( veh->pathTransitionTime > 0.0 )
    {
        veh->pathTransitionTime = veh->pathTransitionTime - 0.05f;
        transitionTimeFrac = 1.0 - (float)(veh->pathTransitionTime / vehHelicopterPathTransitionTime->current.value);
        if ( transitionTimeFrac >= 1.0 )
        {
            veh->flags &= ~0x100u;
            veh->hasTargetYaw = 0;
            veh->phys.origin[0] = pathPosOrigin;
            veh->phys.origin[1] = pathPosOrigin_4;
            veh->phys.origin[2] = pathPosOrigin_8;
            veh->phys.angles[0] = pathPosAngles;
            veh->phys.angles[1] = pathPosAngles_4;
            veh->phys.angles[2] = pathPosAngles_8;
        }
        else
        {
            veh->phys.origin[0] = (float)((float)(pathPosOrigin - veh->phys.origin[0]) * transitionTimeFrac)
                                                    + veh->phys.origin[0];
            veh->phys.origin[1] = (float)((float)(pathPosOrigin_4 - veh->phys.origin[1]) * transitionTimeFrac)
                                                    + veh->phys.origin[1];
            veh->phys.origin[2] = (float)((float)(pathPosOrigin_8 - veh->phys.origin[2]) * transitionTimeFrac)
                                                    + veh->phys.origin[2];
            v3 = veh->phys.angles[0];
            veh->phys.angles[0] = AngleNormalize180(pathPosAngles - v3) * transitionTimeFrac + v3;
            v2 = veh->phys.angles[1];
            veh->phys.angles[1] = AngleNormalize180(pathPosAngles_4 - v2) * transitionTimeFrac + v2;
            v1 = veh->phys.angles[2];
            veh->phys.angles[2] = AngleNormalize180(pathPosAngles_8 - v1) * transitionTimeFrac + v1;
        }
    }
    if ( veh->pathPos.endOfPath && (veh->flags & 0x100) != 0 )
    {
        veh->flags &= ~0x80u;
        veh->flags &= ~0x100u;
        veh->flags |= 2u;
        if ( veh->pathPos.speed > 1.0 )
            veh->moveState = VEH_MOVESTATE_HOVER;
        else
            veh->moveState = VEH_MOVESTATE_STOP;
        veh->stopAtGoal = 1;
        veh->goalPosition[0] = veh->pathPos.lookPos[0];
        veh->goalPosition[1] = veh->pathPos.lookPos[1];
        veh->goalPosition[2] = veh->pathPos.lookPos[2];
        veh->hasTargetYaw = 0;
    }
}

void __cdecl Scr_Vehicle_Pain(
                gentity_s *pSelf,
                gentity_s *pAttacker,
                int damage,
                const float *point,
                int mod,
                const float *dir,
                hitLocation_t hitLoc,
                int weaponIdx)
{
    const WeaponDef *weapDef; // [esp+Ch] [ebp-4h]

    weapDef = BG_GetWeaponDef(weaponIdx);
    if ( weapDef->weapType == WEAPTYPE_PROJECTILE
        || weapDef->weapType == WEAPTYPE_GRENADE
        || weapDef->weapType == WEAPTYPE_MINE )
    {
        VEH_JoltBody(pSelf, dir, 1.0, 0.0, 0.0);
    }
    if (pSelf->scr_vehicle->nitrousVehicle)
    {
        //NitrousVehicle::damage(pSelf->scr_vehicle->nitrousVehicle, damage, point, dir, mod);
        pSelf->scr_vehicle->nitrousVehicle->damage(damage, point, dir, mod);
    }
}

void __cdecl Scr_Vehicle_Touch(gentity_s *pSelf, gentity_s *pOther, int __formal)
{
    gentity_s *v2; // [esp+0h] [ebp-68h]
    int damage; // [esp+2Ch] [ebp-3Ch]
    float moveLen; // [esp+34h] [ebp-34h]
    const vehicle_info_t *info; // [esp+3Ch] [ebp-2Ch]
    float hitDir[2]; // [esp+40h] [ebp-28h] BYREF
    scr_vehicle_s *veh; // [esp+48h] [ebp-20h]
    gentity_s *driver; // [esp+4Ch] [ebp-1Ch]
    float damageScale; // [esp+50h] [ebp-18h]
    float speed; // [esp+54h] [ebp-14h]
    float dot; // [esp+58h] [ebp-10h]
    float moveDir[3]; // [esp+5Ch] [ebp-Ch] BYREF

    if ( !pSelf && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 9248, 0, "%s", "pSelf") )
        __debugbreak();
    if ( !pSelf->scr_vehicle
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                    9249,
                    0,
                    "%s",
                    "pSelf->scr_vehicle") )
    {
        __debugbreak();
    }
    if ( !pOther
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 9250, 0, "%s", "pOther") )
    {
        __debugbreak();
    }
    veh = pSelf->scr_vehicle;
    info = BG_GetVehicleInfo(veh->infoIdx);
    if ( (pOther->s.eType == 1 || pOther->s.eType == 17 || pOther->s.eType == 6 || pOther->s.eType == 14)
        && pOther->takedamage
        && !pOther->tagInfo
        && (pOther->s.eType != 1 || !vehicle_riding->current.enabled)
        && pOther->s.groundEntityNum != pSelf->s.number
        && !info->remoteControl )
    {
        driver = VEH_GetSeatOccupantEntity(veh, 0);
        if ( !driver || !pOther->client )
            goto LABEL_28;
        if ( !driver->client
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                        9282,
                        0,
                        "%s",
                        "driver->client") )
        {
            __debugbreak();
        }
        if ( driver->client->sess.cs.team != pOther->client->sess.cs.team || driver->client->sess.cs.team == TEAM_FREE )
        {
LABEL_28:
            if ( driver )
                v2 = driver;
            else
                v2 = pSelf;
            if ( info->collisionDamage > 0.0 )
            {
                moveLen = Vec3NormalizeTo(veh->phys.vel, moveDir);
                if ( moveLen >= 0.001 && pOther->s.eType != 6 )
                {
                    hitDir[0] = pOther->r.currentOrigin[0] - pSelf->r.currentOrigin[0];
                    hitDir[1] = pOther->r.currentOrigin[1] - pSelf->r.currentOrigin[1];
                    Vec2Normalize(hitDir);
                    if ( pOther->client && (pOther->client->ps.pm_flags & 1) != 0 )
                    {
                        G_Damage(
                            pOther,
                            pSelf,
                            v2,
                            moveDir,
                            pOther->r.currentOrigin,
                            999999,
                            0,
                            10,
                            0xFFFFFFFF,
                            HITLOC_NONE,
                            0,
                            0,
                            0);
                    }
                    else
                    {
                        dot = (float)(moveDir[0] * hitDir[0]) + (float)(moveDir[1] * hitDir[1]);
                        if ( dot >= 0.80000001 )
                        {
                            if ( info->collisionSpeed == 0.0
                                && !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                                            9336,
                                            0,
                                            "%s",
                                            "info->collisionSpeed") )
                            {
                                __debugbreak();
                            }

                            //if ( (_S3_0 & 1) == 0 )
                            //{
                            //    _S3_0 |= 1u;
                            //    minDamageIPS = 5.0 * 17.6;
                            //}
                            static float minDamageIPS = 5.0 * 17.6;

                            speed = fabs(veh->speed);
                            if ( minDamageIPS <= speed )
                            {
                                damageScale = (float)(speed - minDamageIPS) / info->collisionSpeed;
                                if ( damageScale > 1.0 )
                                    damageScale = 1.0f;
                                damageScale = (float)((float)(dot - 0.80000001) / 0.19999999) * damageScale;
                                damage = (int)(float)(damageScale * info->collisionDamage);
                                if ( damage > 0 )
                                    G_Damage(
                                        pOther,
                                        pSelf,
                                        v2,
                                        moveDir,
                                        pOther->r.currentOrigin,
                                        damage,
                                        0,
                                        10,
                                        0xFFFFFFFF,
                                        HITLOC_NONE,
                                        0,
                                        0,
                                        0);
                            }
                        }
                    }
                }
            }
        }
    }
}

unsigned __int16 __cdecl G_GetVehicleTypeString(int clientNum, int entityNum)
{
    const vehicle_info_t *info; // [esp+0h] [ebp-10h]
    scr_vehicle_s *veh; // [esp+4h] [ebp-Ch]
    unsigned __int16 string; // [esp+8h] [ebp-8h]

    veh = g_entities[entityNum].scr_vehicle;
    if ( !veh )
        return 0;
    info = BG_GetVehicleInfo(veh->infoIdx);
    string = SL_FindString(info->animSet, SCRIPTINSTANCE_SERVER);
    if ( !string )
        return SL_GetString_(SCRIPTINSTANCE_SERVER, info->animSet, 0, 22);
    return string;
}

void __cdecl Scr_Vehicle_Use(gentity_s *pEnt, gentity_s *pOther, gentity_s *__formal)
{
    team_t vehicle_team; // [esp+0h] [ebp-Ch]
    int entryPoint; // [esp+4h] [ebp-8h] BYREF
    int seatIndex; // [esp+8h] [ebp-4h]

    if ( pOther->client )
    {
        if ( (pEnt->scr_vehicle->flags & 8) == 0 && (pOther->client->ps.eFlags & 0x4000) == 0 )
        {
            vehicle_team = G_GetVehicleOccupantsTeam(pEnt);
            if ( vehicle_team == TEAM_FREE || vehicle_team == pOther->client->sess.cs.team )
            {
                entryPoint = -1;
                seatIndex = G_GetVehicleSeatToEnter(pEnt, pOther, 0, &entryPoint);
                if ( seatIndex != -1 )
                {
                    VEH_LinkPlayer(pEnt, pOther, seatIndex, 0);
                    if ( !seatIndex )
                        Scr_Notify(pOther, scr_const.vehicle_driver, 0);
                }
            }
        }
    }
}

void __cdecl Scr_Vehicle_Die(
    gentity_s *pSelf,
    gentity_s *pInflictor,
    gentity_s *pAttacker,
    int damage,
    int mod,
    const int weapon,
    const float *dir,
    const hitLocation_t hitLoc,
    int timeOffset)
{
    gentity_s *v9; // eax
    int v10; // eax
    char *v11; // eax
    unsigned __int16 speedFrac; // [esp+4h] [ebp-20h]
    gentity_s *occupant; // [esp+14h] [ebp-10h]
    int seatIndex; // [esp+18h] [ebp-Ch]
    gentity_s *vehicleOccupant; // [esp+1Ch] [ebp-8h]
    const WeaponDef *weapDef; // [esp+20h] [ebp-4h]

    vehicleOccupant = 0;
    //if ( EntHandle::isDefined(&pSelf->r.ownerNum) )
    if ( pSelf->r.ownerNum.isDefined() )
    {
        //vehicleOccupant = EntHandle::ent(&pSelf->r.ownerNum);
        vehicleOccupant = pSelf->r.ownerNum.ent();
        Scr_AddBool(1u, SCRIPTINSTANCE_SERVER);
        speedFrac = scr_const.vehicle_death;
        //v9 = EntHandle::ent(&pSelf->r.ownerNum);
        v9 = pSelf->r.ownerNum.ent();
        Scr_Notify(v9, speedFrac, 1u);
        //v10 = EntHandle::entnum(&pSelf->r.ownerNum);
        v10 = pSelf->r.ownerNum.entnum();
        VEH_UnlinkPlayer(&g_entities[v10], 0, (char*)"Scr_Vehicle_Die A");
        if ( vehicleOccupant->health > 0 )
        {
            if ( damage > 100000 )
                player_die(vehicleOccupant, pInflictor, pAttacker, damage, mod, weapon, dir, HITLOC_NONE, timeOffset);
            else
                player_die(vehicleOccupant, pInflictor, pAttacker, 100000, mod, weapon, dir, HITLOC_NONE, timeOffset);
        }
    }
    for ( seatIndex = 0; seatIndex < 11; ++seatIndex )
    {
        occupant = VEH_GetSeatOccupantEntity(pSelf->scr_vehicle, seatIndex);
        if ( occupant )
        {
            Scr_AddBool(1u, SCRIPTINSTANCE_SERVER);
            Scr_Notify(occupant, scr_const.vehicle_death, 1u);
            VEH_UnlinkPlayer(occupant, 0, (char *)"Scr_Vehicle_Die B");
            vehicleOccupant = occupant;
            if ( occupant->health > 0 )
            {
                if ( damage > 100000 )
                    player_die(occupant, pInflictor, pAttacker, damage, mod, weapon, dir, HITLOC_NONE, timeOffset);
                else
                    player_die(occupant, pInflictor, pAttacker, 100000, mod, weapon, dir, HITLOC_NONE, timeOffset);
            }
        }
    }
    if ( pAttacker )
    {
        if ( weapon )
        {
            if ( weapon != -1 )
            {
                weapDef = BG_GetWeaponDef(weapon);
                if ( weapDef->weapType == WEAPTYPE_PROJECTILE
                    || weapDef->weapType == WEAPTYPE_GRENADE
                    || weapDef->weapType == WEAPTYPE_MINE )
                {
                    VEH_JoltBody(pSelf, dir, 1.0, 0.0, 0.0);
                }
            }
        }
    }
    if ( vehicleOccupant && pAttacker && pAttacker->client )
    {
        Scr_AddEntity(vehicleOccupant, SCRIPTINSTANCE_SERVER);
        if ( (pAttacker->client->ps.eFlags & 0x4000) != 0 )
        {
            Scr_AddString("tankGun", SCRIPTINSTANCE_SERVER);
        }
        else if ( !weapon || weapon == -1 )
        {
            Scr_AddString("none", SCRIPTINSTANCE_SERVER);
        }
        else
        {
            v11 = (char *)BG_WeaponName(weapon);
            Scr_AddString(v11, SCRIPTINSTANCE_SERVER);
        }
        Scr_Notify(pAttacker, scr_const.destroyed_vehicle, 2u);
    }
    if (pSelf->scr_vehicle->nitrousVehicle)
    {
        //NitrousVehicle::damage(pSelf->scr_vehicle->nitrousVehicle, damage, pSelf->r.currentOrigin, dir, mod);
        pSelf->scr_vehicle->nitrousVehicle->damage(damage, pSelf->r.currentOrigin, dir, mod);
    }
    pSelf->flags |= 0x40000u;
}

void __cdecl SP_script_vehicle(gentity_s *pSelf, SpawnVar *spawnVar)
{
    const char *typeName; // [esp+0h] [ebp-4h] BYREF

    if ( spawnVar )
    {
        G_SpawnString(spawnVar, "vehicletype", 0, &typeName);
        G_SpawnVehicle(pSelf, (char *)typeName, 1);
    }
}

void __cdecl SP_script_vehicle_collmap(gentity_s *pSelf, SpawnVar *v)
{
    pSelf->r.contents = 0;
    pSelf->s.eType = ET_VEHICLE_COLLMAP;
}

void (__cdecl *__cdecl ScriptVehicle_GetMethod(const char **pName))(scr_entref_t)
{
    unsigned int i; // [esp+18h] [ebp-4h]

    for ( i = 0; i < ARRAY_COUNT(s_methods_0); ++i )
    {
        if ( !strcmp(*pName, s_methods_0[i].actionString) )
        {
            *pName = s_methods_0[i].actionString;
            return s_methods_0[i].actionFunc;
        }
    }
    return 0;
}

void __cdecl CMD_VEH_AttachPath(scr_entref_t entref)
{
    const vehicle_info_t *info; // [esp+28h] [ebp-10h]
    unsigned __int16 nodeIdx; // [esp+2Ch] [ebp-Ch]
    scr_vehicle_s *veh; // [esp+30h] [ebp-8h]
    gentity_s *ent; // [esp+34h] [ebp-4h]

    ent = GScr_GetVehicle(entref);
    veh = ent->scr_vehicle;
    info = BG_GetVehicleInfo(veh->infoIdx);
    nodeIdx = GScr_GetVehicleNodeIndex(0);
    G_VehSetUpPathPos(&veh->pathPos, nodeIdx);
    veh->flags &= ~0x100u;
    veh->phys.origin[0] = veh->pathPos.origin[0];
    veh->phys.origin[1] = veh->pathPos.origin[1];
    veh->phys.origin[2] = veh->pathPos.origin[2];
    veh->phys.angles[0] = veh->pathPos.angles[0];
    veh->phys.angles[1] = veh->pathPos.angles[1];
    veh->phys.angles[2] = veh->pathPos.angles[2];
    VEH_SetPosition(ent, veh->phys.origin, veh->phys.vel, veh->phys.angles);
    veh->phys.prevOrigin[0] = veh->phys.origin[0];
    veh->phys.prevOrigin[1] = veh->phys.origin[1];
    veh->phys.prevOrigin[2] = veh->phys.origin[2];
    veh->phys.prevAngles[0] = veh->phys.angles[0];
    veh->phys.prevAngles[1] = veh->phys.angles[1];
    veh->phys.prevAngles[2] = veh->phys.angles[2];
    VEH_ResetWheels(ent, &veh->phys);
    if ( !info->type || info->type == 2 )
        VEH_GroundPlant(ent, &veh->phys, 0);
    VEH_SetPosition(ent, veh->phys.origin, veh->phys.vel, veh->phys.angles);
    veh->phys.prevOrigin[0] = veh->phys.origin[0];
    veh->phys.prevOrigin[1] = veh->phys.origin[1];
    veh->phys.prevOrigin[2] = veh->phys.origin[2];
    veh->phys.prevAngles[0] = veh->phys.angles[0];
    veh->phys.prevAngles[1] = veh->phys.angles[1];
    veh->phys.prevAngles[2] = veh->phys.angles[2];
}

void __cdecl VEH_ResetWheels(gentity_s *ent, vehicle_physic_t *phys)
{
    scr_vehicle_s *veh; // [esp+0h] [ebp-14h]
    int wheelIndex; // [esp+4h] [ebp-10h]
    float wheelPos[3]; // [esp+8h] [ebp-Ch] BYREF

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 1004, 0, "%s", "ent") )
        __debugbreak();
    if ( !ent->scr_vehicle
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                    1005,
                    0,
                    "%s",
                    "ent->scr_vehicle") )
    {
        __debugbreak();
    }
    if ( !phys && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 1006, 0, "%s", "phys") )
        __debugbreak();
    veh = ent->scr_vehicle;
    for ( wheelIndex = 0; wheelIndex < 6; ++wheelIndex )
    {
        if ( veh->boneIndex.wheel[wheelIndex] >= 0 )
        {
            G_DObjGetWorldBoneIndexPos(ent, veh->boneIndex.wheel[wheelIndex], wheelPos);
            phys->wheelZPos[wheelIndex] = wheelPos[2];
        }
    }
}

void __cdecl CMD_VEH_GetModel(scr_entref_t entref)
{
    gentity_s *ent; // [esp+4h] [ebp-4h]

    ent = GScr_GetVehicle(entref);
    if ( ent && ent->scr_vehicle && ent->scr_vehicle->nitrousVehicle && ent->scr_vehicle->nitrousVehicle->m_xmodel )
        Scr_AddString((char *)ent->scr_vehicle->nitrousVehicle->m_xmodel->name, SCRIPTINSTANCE_SERVER);
    else
        Scr_Error("Incorrect usage of GetModelName()", 0);
}

void __cdecl CMD_VEH_GetAttachPos(scr_entref_t entref)
{
    const vehicle_info_t *info; // [esp+0h] [ebp-21Ch]
    vehicle_physic_t phys; // [esp+4h] [ebp-218h] BYREF
    __int16 nodeIdx; // [esp+12Ch] [ebp-F0h]
    const scr_vehicle_s *veh; // [esp+130h] [ebp-ECh]
    vehicle_pathpos_t pathPos; // [esp+134h] [ebp-E8h] BYREF
    gentity_s *ent; // [esp+218h] [ebp-4h]

    ent = GScr_GetVehicle(entref);
    veh = ent->scr_vehicle;
    info = BG_GetVehicleInfo(veh->infoIdx);
    nodeIdx = GScr_GetVehicleNodeIndex(0);
    G_VehSetUpPathPos(&pathPos, nodeIdx);
    Com_Memcpy(&phys, &veh->phys, 296);
    phys.origin[0] = pathPos.origin[0];
    phys.origin[1] = pathPos.origin[1];
    phys.origin[2] = pathPos.origin[2];
    phys.angles[0] = pathPos.angles[0];
    phys.angles[1] = pathPos.angles[1];
    phys.angles[2] = pathPos.angles[2];
    VEH_ResetWheels(ent, &phys);
    if ( !info->type || info->type == 2 )
        VEH_GroundPlant(ent, &phys, 0);
    Scr_MakeArray(SCRIPTINSTANCE_SERVER);
    Scr_AddVector(phys.origin, SCRIPTINSTANCE_SERVER);
    Scr_AddArray(SCRIPTINSTANCE_SERVER);
    Scr_AddVector(phys.angles, SCRIPTINSTANCE_SERVER);
    Scr_AddArray(SCRIPTINSTANCE_SERVER);
}

void __cdecl CMD_VEH_StartPath(scr_entref_t entref)
{
    const vehicle_info_t *info; // [esp+0h] [ebp-Ch]
    scr_vehicle_s *veh; // [esp+4h] [ebp-8h]

    veh = GScr_GetVehicle(entref)->scr_vehicle;
    info = BG_GetVehicleInfo(veh->infoIdx);
    if ( veh->pathPos.nodeIdx < 0 )
        Scr_Error("Can't start path on a vehicle that hasn't been attached", 0);
    if ( info->isNitrous && !veh->nitrousVehicle )
        Scr_Error("Can't start physics vehicles on path until the physics is setup", 0);
    if (veh->nitrousVehicle)
    {
        //NitrousVehicle::start_path(veh->nitrousVehicle, 0);
        veh->nitrousVehicle->start_path(0);
    }
    veh->flags |= 0x80u;
    if ( (veh->pathPos.flags & 0x100) != 0 )
        veh->pathTransitionTime = vehHelicopterPathTransitionTime->current.value;
}

void __cdecl CMD_VEH_DrivePath(scr_entref_t entref)
{
    const vehicle_info_t *info; // [esp+0h] [ebp-10h]
    unsigned __int16 nodeIdx; // [esp+4h] [ebp-Ch]
    scr_vehicle_s *veh; // [esp+8h] [ebp-8h]

    veh = GScr_GetVehicle(entref)->scr_vehicle;
    info = BG_GetVehicleInfo(veh->infoIdx);
    if ( Scr_GetNumParam(SCRIPTINSTANCE_SERVER) == 1 )
    {
        nodeIdx = GScr_GetVehicleNodeIndex(0);
        G_VehSetUpPathPos(&veh->pathPos, nodeIdx);
    }
    veh->pathPos.flags |= 0x100u;
    if ( info->isNitrous && !veh->nitrousVehicle )
        Scr_Error("Can't start physics vehicles on path until the physics is setup", 0);
    if ( veh->pathPos.nodeIdx < 0 )
        Scr_Error("Invalid start node to DrivePath", 0);
    if ( info->type != 6 && !veh->nitrousVehicle )
        Scr_Error("DrivePath can only be called on helicopters and physics vehicles", 0);
    if (veh->nitrousVehicle)
    {
        //NitrousVehicle::start_path(veh->nitrousVehicle, 1);
        veh->nitrousVehicle->start_path(1);
    }
    veh->flags |= 0x180u;
}

void __cdecl CMD_VEH_SetDrivePathPhysicsScale(scr_entref_t entref)
{
    const vehicle_info_t *info; // [esp+0h] [ebp-10h]
    scr_vehicle_s *veh; // [esp+4h] [ebp-Ch]
    float scale; // [esp+Ch] [ebp-4h]

    veh = GScr_GetVehicle(entref)->scr_vehicle;
    info = BG_GetVehicleInfo(veh->infoIdx);
    scale = 3.0f;
    if ( Scr_GetNumParam(SCRIPTINSTANCE_SERVER) == 1 )
        scale = Scr_GetFloat(0, SCRIPTINSTANCE_SERVER);
    if ( info->isNitrous && !veh->nitrousVehicle )
        Scr_Error("Can't SetDrivePathPhysicsScale until the physics is setup", 0);
    if ( info->type != 6 && !veh->nitrousVehicle )
        Scr_Error("SetDrivePathPhysicsScale can only be called on helicopters and physics vehicles", 0);
    if ( veh->nitrousVehicle )
        veh->nitrousVehicle->m_drivepath_scale = scale;
}

void __cdecl CMD_VEH_SetSwitchNode(scr_entref_t entref)
{
    unsigned __int16 srcNode; // [esp+0h] [ebp-10h]
    unsigned __int16 dstNode; // [esp+4h] [ebp-Ch]
    scr_vehicle_s *veh; // [esp+8h] [ebp-8h]

    veh = GScr_GetVehicle(entref)->scr_vehicle;
    srcNode = GScr_GetVehicleNodeIndex(0);
    dstNode = GScr_GetVehicleNodeIndex(1u);
    G_VehSetSwitchNode(&veh->pathPos, srcNode, dstNode);
}

void __cdecl CMD_VEH_SetWaitNode(scr_entref_t entref)
{
    scr_vehicle_s *veh; // [esp+0h] [ebp-8h]

    veh = GScr_GetVehicle(entref)->scr_vehicle;
    veh->waitNode = GScr_GetVehicleNodeIndex(0);
}

void __cdecl CMD_VEH_SetWaitSpeed(scr_entref_t entref)
{
    scr_vehicle_s *veh; // [esp+0h] [ebp-8h]

    veh = GScr_GetVehicle(entref)->scr_vehicle;
    veh->waitSpeed = Scr_GetFloat(0, SCRIPTINSTANCE_SERVER) * 17.6;
    if ( veh->waitSpeed < 0.0 )
        Scr_ParamError(0, "Cannot have a negative wait speed on a vehicle", SCRIPTINSTANCE_SERVER);
}

void __cdecl CMD_VEH_SetSpeed(scr_entref_t entref)
{
    gentity_s *ent; // [esp+8h] [ebp-4h]

    ent = GScr_GetVehicle(entref);
    CMD_VEH_Script_SetSpeed(ent);
}

void __cdecl CMD_VEH_Script_SetSpeed(gentity_s *ent)
{
    const vehicle_info_t *info; // [esp+8h] [ebp-8h]
    scr_vehicle_s *veh; // [esp+Ch] [ebp-4h]

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 9887, 0, "%s", "ent") )
        __debugbreak();
    veh = ent->scr_vehicle;
    info = BG_GetVehicleInfo(veh->infoIdx);
    if ( !veh && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 9892, 0, "%s", "veh") )
        __debugbreak();
    if ( !info && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 9893, 0, "%s", "info") )
        __debugbreak();
    veh->manualMode = 1;
    veh->manualSpeed = Scr_GetFloat(0, SCRIPTINSTANCE_SERVER) * 17.6;
    if ( veh->nitrousVehicle )
        veh->nitrousVehicle->mVehicleController.m_script_goal_speed = veh->manualSpeed;
    if ( veh->manualSpeed < 0.0 )
    {
        Scr_ParamError(0, "Cannot set negative speed on vehicle", SCRIPTINSTANCE_SERVER);
        veh->manualSpeed = 0.0f;
    }
    if ( (unsigned int)Scr_GetNumParam(SCRIPTINSTANCE_SERVER) > 1 )
        veh->manualAccel = Scr_GetFloat(1u, SCRIPTINSTANCE_SERVER) * 17.6;
    if ( info->type == 6 && veh->manualSpeed > veh->speed && veh->manualAccel > veh->manualSpeed )
    {
        Com_PrintWarning(15, "WARNING: capping acceleration to speed / sec for vehicle '%d'\n", ent->s.number);
        veh->manualAccel = veh->manualSpeed;
    }
    if ( (unsigned int)Scr_GetNumParam(SCRIPTINSTANCE_SERVER) <= 2 )
        veh->manualDecel = veh->manualAccel * 0.5;
    else
        veh->manualDecel = Scr_GetFloat(2u, SCRIPTINSTANCE_SERVER) * 17.6;
    if ( veh->manualAccel <= 0.0 || veh->manualDecel <= 0.0 )
    {
        Scr_ParamError(1u, "Acceleration/deceleration must be > 0", SCRIPTINSTANCE_SERVER);
        veh->manualAccel = 1.0 * 17.6;
        veh->manualDecel = 1.0 * 17.6;
    }
}

void __cdecl CMD_VEH_SetSpeedImmediate(scr_entref_t entref)
{
    int v2; // [esp+4h] [ebp-38h]
    float speed; // [esp+Ch] [ebp-30h]
    scr_vehicle_s *veh; // [esp+28h] [ebp-14h]
    float vecToGoal[3]; // [esp+2Ch] [ebp-10h] BYREF
    gentity_s *ent; // [esp+38h] [ebp-4h]

    ent = GScr_GetVehicle(entref);
    veh = ent->scr_vehicle;
    CMD_VEH_Script_SetSpeed(ent);
    memset(vecToGoal, 0, sizeof(vecToGoal));
    if ( veh->speed > 0.0
        || (veh->goalPosition[0] != 0.0 || veh->goalPosition[1] != 0.0 || veh->goalPosition[2] != 0.0 ? (v2 = 0) : (v2 = 1),
                !v2) )
    {
        vecToGoal[0] = veh->goalPosition[0] - ent->r.currentOrigin[0];
        vecToGoal[1] = veh->goalPosition[1] - ent->r.currentOrigin[1];
        vecToGoal[2] = veh->goalPosition[2] - ent->r.currentOrigin[2];
        Vec3Normalize(vecToGoal);
    }
    if ( vecToGoal[0] == 0.0 && vecToGoal[1] == 0.0 && vecToGoal[2] == 0.0 )
        AngleVectors(ent->r.currentAngles, vecToGoal, 0, 0);
    veh->speed = veh->manualSpeed;
    veh->phys.bodyVel[0] = veh->speed;
    speed = veh->speed;
    veh->phys.vel[0] = speed * vecToGoal[0];
    veh->phys.vel[1] = speed * vecToGoal[1];
    veh->phys.vel[2] = speed * vecToGoal[2];
}

void __cdecl CMD_VEH_GetSpeed(scr_entref_t entref)
{
    scr_vehicle_s *value; // [esp+0h] [ebp-10h]

    value = GScr_GetVehicle(entref)->scr_vehicle;
    Scr_AddFloat(value->speed, SCRIPTINSTANCE_SERVER);
}

void __cdecl CMD_VEH_GetSpeedMPH(scr_entref_t entref)
{
    gentity_s *ent; // [esp+Ch] [ebp-4h]

    ent = GScr_GetVehicle(entref);
    Scr_AddFloat(ent->scr_vehicle->speed / 17.6, SCRIPTINSTANCE_SERVER);
}

void __cdecl CMD_VEH_GetGoalSpeedMPH(scr_entref_t entref)
{
    gentity_s *ent; // [esp+Ch] [ebp-4h]

    ent = GScr_GetVehicle(entref);
    Scr_AddFloat(ent->scr_vehicle->manualSpeed / 17.6, SCRIPTINSTANCE_SERVER);
}

void __cdecl CMD_VEH_SetBrake(scr_entref_t entref)
{
    scr_vehicle_s *veh; // [esp+4h] [ebp-Ch]
    float brake; // [esp+8h] [ebp-8h]

    veh = GScr_GetVehicle(entref)->scr_vehicle;
    brake = Scr_GetFloat(0, SCRIPTINSTANCE_SERVER);
    if ( veh->nitrousVehicle )
        veh->nitrousVehicle->m_script_brake = brake;
    else
        Scr_Error("SetBrake can only be called on nitrous physics vehicles", 0);
}

void __cdecl CMD_VEH_SetAcceleration(scr_entref_t entref)
{
    scr_vehicle_s *veh; // [esp+0h] [ebp-8h]
    gentity_s *ent; // [esp+4h] [ebp-4h]

    ent = GScr_GetVehicle(entref);
    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 10103, 0, "%s", "ent") )
        __debugbreak();
    veh = ent->scr_vehicle;
    if ( !veh && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 10106, 0, "%s", "veh") )
        __debugbreak();
    veh->manualAccel = Scr_GetFloat(0, SCRIPTINSTANCE_SERVER) * 17.6;
}

void __cdecl CMD_VEH_SetDeceleration(scr_entref_t entref)
{
    scr_vehicle_s *veh; // [esp+0h] [ebp-8h]
    gentity_s *ent; // [esp+4h] [ebp-4h]

    ent = GScr_GetVehicle(entref);
    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 10126, 0, "%s", "ent") )
        __debugbreak();
    veh = ent->scr_vehicle;
    if ( !veh && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 10129, 0, "%s", "veh") )
        __debugbreak();
    veh->manualDecel = Scr_GetFloat(0, SCRIPTINSTANCE_SERVER) * 17.6;
}

void __cdecl CMD_VEH_ResumeSpeed(scr_entref_t entref)
{
    float goalSpeed; // [esp+8h] [ebp-Ch]
    scr_vehicle_s *veh; // [esp+Ch] [ebp-8h]

    veh = GScr_GetVehicle(entref)->scr_vehicle;
    veh->manualMode = 2;
    veh->manualAccel = Scr_GetFloat(0, SCRIPTINSTANCE_SERVER) * 17.6;
    if ( veh->nitrousVehicle )
    {
        //goalSpeed = NitrousVehicle::get_max_speed(veh->nitrousVehicle, 1);
        goalSpeed = veh->nitrousVehicle->get_max_speed(true);
        veh->nitrousVehicle->mVehicleController.m_script_goal_speed = goalSpeed;
        veh->nitrousVehicle->m_speed_scale = 1.0f;
    }
    if ( veh->manualAccel < 0.0 )
        Scr_ParamError(0, "Cannot set negative acceleration on vehicle", SCRIPTINSTANCE_SERVER);
}

void __cdecl CMD_VEH_SetYawSpeed(scr_entref_t entref)
{
    scr_vehicle_s *veh; // [esp+0h] [ebp-8h]

    veh = GScr_GetVehicle(entref)->scr_vehicle;
    veh->phys.maxAngleVel[1] = Scr_GetFloat(0, SCRIPTINSTANCE_SERVER);
    veh->phys.yawAccel = Scr_GetFloat(1u, SCRIPTINSTANCE_SERVER);
    if ( (unsigned int)Scr_GetNumParam(SCRIPTINSTANCE_SERVER) <= 2 )
        veh->phys.yawDecel = veh->phys.yawAccel;
    else
        veh->phys.yawDecel = Scr_GetFloat(2u, SCRIPTINSTANCE_SERVER);
    if ( (unsigned int)Scr_GetNumParam(SCRIPTINSTANCE_SERVER) > 3 )
    {
        veh->yawOverShoot = Scr_GetFloat(3u, SCRIPTINSTANCE_SERVER);
        if ( veh->yawOverShoot < 0.0 || veh->yawOverShoot > 1.0 )
            Scr_ParamError(3u, "Overshoot must be in 0 to 1 range", SCRIPTINSTANCE_SERVER);
    }
    if ( veh->phys.maxAngleVel[1] < 0.0 )
        Scr_ParamError(0, "Cannot set negative yaw speed on vehicle", SCRIPTINSTANCE_SERVER);
    if ( veh->phys.yawAccel < 0.0 )
        Scr_ParamError(1u, "Cannot set negative yaw acceleration on vehicle", SCRIPTINSTANCE_SERVER);
}

void __cdecl CMD_VEH_SetMaxPitchRoll(scr_entref_t entref)
{
    const vehicle_info_t *info; // [esp+0h] [ebp-Ch]
    scr_vehicle_s *veh; // [esp+4h] [ebp-8h]

    veh = GScr_GetVehicle(entref)->scr_vehicle;
    info = BG_GetVehicleInfo(veh->infoIdx);
    if ( info->type != 6 && info->type != 3 )
        Scr_Error("Max Pitch Roll only valid for helicopters and planes.", 0);
    veh->phys.maxPitchAngle = Scr_GetFloat(0, SCRIPTINSTANCE_SERVER);
    veh->phys.maxRollAngle = Scr_GetFloat(1u, SCRIPTINSTANCE_SERVER);
    if ( veh->phys.maxPitchAngle < 0.0 )
        Scr_ParamError(0, "Cannot set negative max pitch", SCRIPTINSTANCE_SERVER);
    if ( veh->phys.maxRollAngle < 0.0 )
        Scr_ParamError(1u, "Cannot set negative max roll", SCRIPTINSTANCE_SERVER);
}

void __cdecl CMD_VEH_SetAirResitance(scr_entref_t entref)
{
    scr_vehicle_s *veh; // [esp+0h] [ebp-8h]

    veh = GScr_GetVehicle(entref)->scr_vehicle;
    veh->maxDragSpeed = Scr_GetFloat(0, SCRIPTINSTANCE_SERVER) * 17.6;
}

void __cdecl CMD_VEH_GetSteerFactor(scr_entref_t entref)
{
    float value; // [esp+8h] [ebp-Ch]

    value = GScr_GetVehicle(entref)->scr_vehicle->nitrousVehicle->m_steer_factor;
    Scr_AddFloat(value, SCRIPTINSTANCE_SERVER);
}

void __cdecl CMD_VEH_GetThrottle(scr_entref_t entref)
{
    float value; // [esp+8h] [ebp-Ch]

    value = GScr_GetVehicle(entref)->scr_vehicle->nitrousVehicle->m_throttle;
    Scr_AddFloat(value, SCRIPTINSTANCE_SERVER);
}

void __cdecl CMD_VEH_SetTurningAbility(scr_entref_t entref)
{
    scr_vehicle_s *veh; // [esp+Ch] [ebp-8h]

    veh = GScr_GetVehicle(entref)->scr_vehicle;
    veh->turningAbility = Scr_GetFloat(0, SCRIPTINSTANCE_SERVER);
}

void __cdecl CMD_VEH_SetJitterParams(scr_entref_t entref)
{
    float v1; // [esp+0h] [ebp-1Ch]
    float Float; // [esp+4h] [ebp-18h]
    scr_vehicle_s *veh; // [esp+8h] [ebp-14h]

    veh = GScr_GetVehicle(entref)->scr_vehicle;
    Scr_GetVector(0, veh->jitter.jitterOffsetRange, SCRIPTINSTANCE_SERVER);
    if ( (unsigned int)Scr_GetNumParam(SCRIPTINSTANCE_SERVER) <= 1 )
        Float = 0.5f;
    else
        Float = Scr_GetFloat(1u, SCRIPTINSTANCE_SERVER);
    veh->jitter.jitterPeriodMin = (int)(float)(Float * 1000.0);
    if ( (unsigned int)Scr_GetNumParam(SCRIPTINSTANCE_SERVER) <= 2 )
        v1 = 1.0f;
    else
        v1 = Scr_GetFloat(2u, SCRIPTINSTANCE_SERVER);
    veh->jitter.jitterPeriodMax = (int)(float)(v1 * 1000.0);
    veh->jitter.jitterEndTime = 0;
}

void __cdecl CMD_VEH_SetHoverParams(scr_entref_t entref)
{
    scr_vehicle_s *veh; // [esp+0h] [ebp-8h]

    veh = GScr_GetVehicle(entref)->scr_vehicle;
    veh->hover.hoverRadius = Scr_GetFloat(0, SCRIPTINSTANCE_SERVER);
    if ( (unsigned int)Scr_GetNumParam(SCRIPTINSTANCE_SERVER) > 1 )
    {
        veh->hover.hoverSpeed = Scr_GetFloat(1u, SCRIPTINSTANCE_SERVER);
        if ( (unsigned int)Scr_GetNumParam(SCRIPTINSTANCE_SERVER) > 2 )
            veh->hover.hoverAccel = Scr_GetFloat(2u, SCRIPTINSTANCE_SERVER);
    }
}

void __cdecl CMD_VEH_JoltBody(scr_entref_t entref)
{
    float joltDir[3]; // [esp+1Ch] [ebp-2Ch] BYREF
    float intensity; // [esp+28h] [ebp-20h]
    int numParams; // [esp+2Ch] [ebp-1Ch]
    float speedFrac; // [esp+30h] [ebp-18h]
    float decel; // [esp+34h] [ebp-14h]
    gentity_s *ent; // [esp+38h] [ebp-10h]
    float joltPos[3]; // [esp+3Ch] [ebp-Ch] BYREF

    ent = GScr_GetVehicle(entref);
    numParams = Scr_GetNumParam(SCRIPTINSTANCE_SERVER);
    Scr_GetVector(0, joltPos, SCRIPTINSTANCE_SERVER);
    intensity = Scr_GetFloat(1u, SCRIPTINSTANCE_SERVER);
    if ( numParams <= 2 )
    {
        speedFrac = 0.0f;
        decel = 0.0f;
    }
    else
    {
        speedFrac = Scr_GetFloat(2u, SCRIPTINSTANCE_SERVER);
        if ( speedFrac < 0.0 || speedFrac > 1.0 )
            Scr_ParamError(2u, "Speed fraction must be between [0,1]", SCRIPTINSTANCE_SERVER);
        decel = Scr_GetFloat(3u, SCRIPTINSTANCE_SERVER) * 17.6;
        if ( decel < 0.0 )
            Scr_ParamError(3u, "Deceleration can't be negative", SCRIPTINSTANCE_SERVER);
    }
    joltDir[0] = ent->r.currentOrigin[0] - joltPos[0];
    joltDir[1] = ent->r.currentOrigin[1] - joltPos[1];
    joltDir[2] = ent->r.currentOrigin[2] - joltPos[2];
    Vec3Normalize(joltDir);
    VEH_JoltBody(ent, joltDir, intensity, speedFrac, decel);
}

void __cdecl CMD_VEH_FreeVehicle(scr_entref_t entref)
{
    const char *v1; // eax
    scr_vehicle_s *veh; // [esp+0h] [ebp-8h]
    gentity_s *ent; // [esp+4h] [ebp-4h]

    ent = GScr_GetVehicle(entref);
    veh = ent->scr_vehicle;
    if ( (veh->flags & 1) != 0 )
    {
        v1 = va("Can't free vehicle that a player is using");
        Scr_Error(v1, 0);
    }
    if ( !veh->nitrousVehicle )
    {
        G_FreeVehicle(ent);
        ent->s.eType = ET_VEHICLE_CORPSE;
        Scr_SetString(&ent->classname, scr_const.script_vehicle_corpse, SCRIPTINSTANCE_SERVER);
    }
    ent->flags |= 0x40000u;
    Scr_Notify(ent, scr_const.death, 0);
}

void __cdecl CMD_VEH_GetWheelSurface(scr_entref_t entref)
{
    const char *v1; // eax
    char *v2; // eax
    const vehicle_info_t *info; // [esp+0h] [ebp-14h]
    scr_vehicle_s *veh; // [esp+4h] [ebp-10h]
    unsigned int wheelName; // [esp+8h] [ebp-Ch]
    int wheel; // [esp+10h] [ebp-4h]

    veh = GScr_GetVehicle(entref)->scr_vehicle;
    info = BG_GetVehicleInfo(veh->infoIdx);
    wheelName = Scr_GetConstString(0, SCRIPTINSTANCE_SERVER);
    wheel = 0;
    if ( info->type != 2 && info->type && info->type != 1 )
    {
        v1 = va("Vehicle type [%s] has no wheels\n", info->name);
        Scr_Error(v1, 0);
    }
    if ( wheelName == scr_const.front_left )
    {
        wheel = 0;
    }
    else if ( wheelName == scr_const.front_right )
    {
        wheel = 1;
    }
    else if ( wheelName == scr_const.back_left )
    {
        wheel = 2;
    }
    else if ( wheelName == scr_const.back_right )
    {
        wheel = 3;
    }
    else if ( wheelName == scr_const.middle_left )
    {
        wheel = 4;
    }
    else if ( wheelName == scr_const.middle_right )
    {
        wheel = 5;
    }
    else
    {
        Scr_ParamError(
            0,
            "Valid wheel names are: [front_left, front_right, back_left, back_right, middle_left, middle_right]\n",
            SCRIPTINSTANCE_SERVER);
    }
    if ( !info->type && wheel > 3 )
        Scr_ParamError(0, "Vehicle has no middle wheels\n", SCRIPTINSTANCE_SERVER);
    if ( veh->phys.wheelSurfType[wheel] )
    {
        v2 = (char *)Com_SurfaceTypeToName(veh->phys.wheelSurfType[wheel]);
        Scr_AddString(v2, SCRIPTINSTANCE_SERVER);
    }
    else
    {
        Scr_AddString("none", SCRIPTINSTANCE_SERVER);
    }
}

void __cdecl CMD_VEH_GetVehicleOwner(scr_entref_t entref)
{
    gentity_s *v1; // eax
    gentity_s *ent; // [esp+8h] [ebp-4h]

    ent = GScr_GetVehicle(entref);

    //if ( EntHandle::isDefined(&ent->r.ownerNum) )
    if ( ent->r.ownerNum.isDefined() )
    {
        //v1 = EntHandle::ent(&ent->r.ownerNum);
        v1 = ent->r.ownerNum.ent();
        Scr_AddEntity(v1, SCRIPTINSTANCE_SERVER);
    }
}

void __cdecl CMD_VEH_IsVehicleUsable(scr_entref_t entref)
{
    if ( (GScr_GetVehicle(entref)->spawnflags & 1) != 0 )
        Scr_AddInt(1, SCRIPTINSTANCE_SERVER);
    else
        Scr_AddInt(0, SCRIPTINSTANCE_SERVER);
}

void __cdecl CMD_VEH_MakeVehicleUsable(scr_entref_t entref)
{
    gentity_s *ent; // [esp+8h] [ebp-4h]

    ent = GScr_GetVehicle(entref);
    G_MakeVehicleUsable(ent);
}

void __cdecl CMD_VEH_MakeVehicleUnusable(scr_entref_t entref)
{
    gentity_s *ent; // [esp+8h] [ebp-4h]

    ent = GScr_GetVehicle(entref);
    G_MakeVehicleUsable(ent);
    ent->spawnflags &= ~1u;
    ent->r.contents &= ~0x200000u;
}

void __cdecl CMD_VEH_UseVehicle(scr_entref_t entref)
{
    gentity_s *user; // [esp+0h] [ebp-Ch]
    gentity_s *ent; // [esp+4h] [ebp-8h]
    int seatIndex; // [esp+8h] [ebp-4h]

    ent = GScr_GetVehicle(entref);
    if ( Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                    10628,
                    0,
                    "%s",
                    "Scr_GetNumParam() == 2") )
    {
        __debugbreak();
    }
    user = Scr_GetEntity(0);
    seatIndex = Scr_GetInt(1u, SCRIPTINSTANCE_SERVER);
    if ( user->client )
    {
        if ( (user->client->ps.eFlags & 0x4000) != 0 )
            VEH_UnlinkPlayer(user, 0, (char*)"CMD_VEH_UseVehicle");
        VEH_LinkPlayer(ent, user, seatIndex, 0);
    }
}

void __cdecl CMD_VEH_SetViewClamp(scr_entref_t entref)
{
    const char *v0; // eax
    float v1; // [esp+0h] [ebp-38h]
    float v2; // [esp+4h] [ebp-34h]
    float v3; // [esp+8h] [ebp-30h]
    float v4; // [esp+Ch] [ebp-2Ch]
    float v5; // [esp+10h] [ebp-28h]
    float v6; // [esp+14h] [ebp-24h]
    float v7; // [esp+18h] [ebp-20h]
    float v8; // [esp+1Ch] [ebp-1Ch]
    float v9; // [esp+20h] [ebp-18h]
    float v10; // [esp+24h] [ebp-14h]
    float Float; // [esp+28h] [ebp-10h]
    float v12; // [esp+2Ch] [ebp-Ch]
    int numParam; // [esp+30h] [ebp-8h]
    gentity_s *user; // [esp+34h] [ebp-4h]

    numParam = Scr_GetNumParam(SCRIPTINSTANCE_SERVER);
    if ( numParam < 3 || numParam > 5 )
    {
        v0 = va(
                     "SetViewClamp:    Does not have manditory parameters SetViewClamp( <user entity>, <right arc>, <left arc>, [<top"
                     " arc>], [<bottom arc>] )\n");
        Scr_Error(v0, 0);
    }
    user = Scr_GetEntity(0);
    if ( user->client )
    {
        if ( numParam <= 1 )
            Float = user->client->linkAnglesMinClamp[1];
        else
            Float = Scr_GetFloat(1u, SCRIPTINSTANCE_SERVER);
        if ( (float)(Float - 180.0) < 0.0 )
            v12 = Float;
        else
            v12 = 180.0f;
        if ( (float)(0.0 - Float) < 0.0 )
            v4 = v12;
        else
            v4 = 0.0f;
        user->client->linkAnglesMinClamp[1] = -v4;
        if ( numParam <= 2 )
            v9 = user->client->linkAnglesMaxClamp[1];
        else
            v9 = Scr_GetFloat(2u, SCRIPTINSTANCE_SERVER);
        if ( (float)(v9 - 180.0) < 0.0 )
            v10 = v9;
        else
            v10 = 180.0f;
        if ( (float)(0.0 - v9) < 0.0 )
            v3 = v10;
        else
            v3 = 0.0f;
        user->client->linkAnglesMaxClamp[1] = v3;
        if ( numParam <= 3 )
            v7 = user->client->linkAnglesMinClamp[0];
        else
            v7 = Scr_GetFloat(3u, SCRIPTINSTANCE_SERVER);
        if ( (float)(v7 - 180.0) < 0.0 )
            v8 = v7;
        else
            v8 = 180.0f;
        if ( (float)(0.0 - v7) < 0.0 )
            v2 = v8;
        else
            v2 = 0.0f;
        user->client->linkAnglesMinClamp[0] = -v2;
        if ( numParam <= 4 )
            v5 = user->client->linkAnglesMaxClamp[0];
        else
            v5 = Scr_GetFloat(4u, SCRIPTINSTANCE_SERVER);
        if ( (float)(v5 - 180.0) < 0.0 )
            v6 = v5;
        else
            v6 = 180.0f;
        if ( (float)(0.0 - v5) < 0.0 )
            v1 = v6;
        else
            v1 = 0.0f;
        user->client->linkAnglesMaxClamp[0] = v1;
    }
}

void __cdecl CMD_VEH_ResetViewClamp(scr_entref_t entref)
{
    const char *v1; // eax
    const vehicle_info_t *info; // [esp+0h] [ebp-14h]
    scr_vehicle_s *veh; // [esp+4h] [ebp-10h]
    gentity_s *user; // [esp+8h] [ebp-Ch]
    gentity_s *ent; // [esp+Ch] [ebp-8h]
    int seatIndex; // [esp+10h] [ebp-4h]

    ent = GScr_GetVehicle(entref);
    veh = ent->scr_vehicle;
    info = BG_GetVehicleInfo(veh->infoIdx);
    if ( Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 1 )
    {
        v1 = va(
                     "SetViewClamp:    Does not have manditory parameters SetViewClamp( <user entity>, <right arc>, <left arc>, [<top"
                     " arc>], [<bottom arc>] )\n");
        Scr_Error(v1, 0);
    }
    user = Scr_GetEntity(0);
    if ( user->client )
    {
        seatIndex = G_GetVehicleSeatPlayerOccupies(ent, user);
        if ( seatIndex < 0 )
            Scr_Error("ResetViewClamp: Player not on vehicle.", 0);
        VEH_SetLinkAngleClamps(info, user, seatIndex, user->client->ps.viewangles, veh->phys.angles);
    }
}

void __cdecl CMD_VEH_SetVehicleLookatText(scr_entref_t entref)
{
    scr_vehicle_s *veh; // [esp+0h] [ebp-10h]
    VariableUnion string1; // [esp+4h] [ebp-Ch]
    VariableUnion string0; // [esp+8h] [ebp-8h]

    veh = GScr_GetVehicle(entref)->scr_vehicle;
    string0.intValue = Scr_GetConstString(0, SCRIPTINSTANCE_SERVER);
    Scr_SetString(&veh->lookAtText0, string0.stringValue, SCRIPTINSTANCE_SERVER);
    if ( (unsigned int)Scr_GetNumParam(SCRIPTINSTANCE_SERVER) > 1 )
    {
        string1.intValue = Scr_GetConstIString(1u, SCRIPTINSTANCE_SERVER);
        Scr_SetString(&veh->lookAtText1, string1.stringValue, SCRIPTINSTANCE_SERVER);
    }
}

void __cdecl CMD_VEH_NearGoalNotifyDist(scr_entref_t entref)
{
    scr_vehicle_s *veh; // [esp+0h] [ebp-8h]

    veh = GScr_GetVehicle(entref)->scr_vehicle;
    veh->nearGoalNotifyDist = Scr_GetFloat(0, SCRIPTINSTANCE_SERVER);
}

void    CMD_VEH_SetGoalPos(scr_entref_t entref)
{
    char *v2; // eax
    const vehicle_info_t *info; // [esp+18h] [ebp-40h]
    int usePath; // [esp+1Ch] [ebp-3Ch]
    phys_vec3 goalPosition; // [esp+2Ch] [ebp-2Ch] BYREF
    float goalSpeed; // [esp+40h] [ebp-18h]
    scr_vehicle_s *veh; // [esp+44h] [ebp-14h]
    gentity_s *ent; // [esp+48h] [ebp-10h]
    //_UNKNOWN *v9; // [esp+4Ch] [ebp-Ch]
    //scr_entref_t entrefa; // [esp+50h] [ebp-8h] BYREF
    //int arg0; // [esp+58h] [ebp+0h]
    //
    //v9 = a1;
    //*(_DWORD *)&entrefa.entnum = arg0;

    ent = GScr_GetVehicle(entref);
    veh = ent->scr_vehicle;
    if (!veh->nitrousVehicle)
        veh->manualMode = 1;
    Scr_GetVector(0, veh->goalPosition, SCRIPTINSTANCE_SERVER);
    veh->stopAtGoal = (unsigned int)Scr_GetNumParam(SCRIPTINSTANCE_SERVER) > 1
        && Scr_GetInt(1u, SCRIPTINSTANCE_SERVER) != 0;
    if (veh->nitrousVehicle)
    {
        goalSpeed = veh->nitrousVehicle->get_max_speed(true);
        if (veh->manualMode == 1)
            goalSpeed = veh->manualSpeed;
        Phys_Vec3ToNitrousVec(veh->goalPosition, &goalPosition);
        //NitrousVehicleController::SetScriptTarget(
            veh->nitrousVehicle->mVehicleController.SetScriptTarget(
            veh->nitrousVehicle,
            &goalPosition,
            veh->hover.hoverRadius,
            goalSpeed,
            veh->stopAtGoal != 0);
    }
    else if (veh->manualSpeed == 0.0 || veh->manualAccel == 0.0 || veh->manualDecel == 0.0)
    {
        Scr_Error("Speed and acceleration must not be zero before setting goal pos", 0);
    }
    veh->flags &= 0xFFFFFE7F;
    veh->flags |= 2u;
    veh->moveState = VEH_MOVESTATE_MOVE;
    veh->stopping = 0;
    usePath = 0;
    if ((unsigned int)Scr_GetNumParam(SCRIPTINSTANCE_SERVER) > 2)
        usePath = Scr_GetInt(2u, SCRIPTINSTANCE_SERVER);
    if (usePath)
    {
        if (VP_FindPath(veh->phys.origin, veh->goalPosition, &veh->pathPos))
        {
            info = BG_GetVehicleInfo(veh->infoIdx);
            veh->pathPos.flags |= 0x100u;
            if (info->type != 6 && !veh->nitrousVehicle)
            {
                v2 = va("SetVehGoalPos with 'usePath' can only be called on helicopters and physics vehicles");
                Scr_Error(v2, 0);
            }
            if (veh->nitrousVehicle)
            {
                //NitrousVehicle::start_path(veh->nitrousVehicle, 1);
                veh->nitrousVehicle->start_path(1);
            }
            veh->flags |= 0x180u;
            Scr_AddBool(1u, SCRIPTINSTANCE_SERVER);
        }
        else
        {
            veh->moveState = VEH_MOVESTATE_STOP;
            veh->flags &= 0xFFFFFE7D;
            Scr_AddBool(0, SCRIPTINSTANCE_SERVER);
        }
    }
    else
    {
        Scr_AddBool(1u, SCRIPTINSTANCE_SERVER);
    }
}

void __cdecl CMD_VEH_ClearGoalPos(scr_entref_t entref)
{
    scr_vehicle_s *veh; // [esp+8h] [ebp-8h]

    veh = GScr_GetVehicle(entref)->scr_vehicle;
    if ( veh->nitrousVehicle )
    {
        veh->nitrousVehicle->m_flags &= ~4u;
        //NitrousVehicle::end_path(veh->nitrousVehicle);
        veh->nitrousVehicle->end_path();
        if ( (veh->flags & 0x100) != 0 && veh->pathPos.customPath )
        {
            VP_FreeCustomPath(veh->pathPos.customPath);
            veh->pathPos.customPath = 0;
        }
    }
    else
    {
        veh->moveState = VEH_MOVESTATE_STOP;
    }
    veh->flags &= 0xFFFFFE7F;
}

void __cdecl CMD_VEH_SetPlaneGoalPos(scr_entref_t entref)
{
    double Float; // st7
    const char *v2; // eax
    float *goalPosition; // [esp+0h] [ebp-B8h]
    float *v4; // [esp+4h] [ebp-B4h]
    signed int i; // [esp+14h] [ebp-A4h]
    float goals[10][3]; // [esp+18h] [ebp-A0h] BYREF
    scr_vehicle_s *veh; // [esp+94h] [ebp-24h]
    float goalAngles[3]; // [esp+98h] [ebp-20h] BYREF
    float travelTime; // [esp+A4h] [ebp-14h]
    float avgSpeed; // [esp+A8h] [ebp-10h]
    int numGoals; // [esp+ACh] [ebp-Ch]
    float speedChange; // [esp+B0h] [ebp-8h]
    gentity_s *ent; // [esp+B4h] [ebp-4h]

    ent = GScr_GetVehicle(entref);
    veh = ent->scr_vehicle;
    numGoals = 0;
    if ( !veh && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 11096, 0, "%s", "veh") )
        __debugbreak();
    for ( i = 0; i < 10 && Scr_GetType(i, SCRIPTINSTANCE_SERVER) == 4; ++i )
    {
        Scr_GetVector(i, goals[i], SCRIPTINSTANCE_SERVER);
        ++numGoals;
    }
    goalPosition = veh->goalPosition;
    v4 = goals[numGoals - 1];
    veh->goalPosition[0] = *v4;
    goalPosition[1] = v4[1];
    goalPosition[2] = v4[2];
    memset(goalAngles, 0, sizeof(goalAngles));
    veh->hasGoalYaw = 0;
    if ( (unsigned int)Scr_GetNumParam(SCRIPTINSTANCE_SERVER) > 1
        && (Scr_GetType(numGoals, SCRIPTINSTANCE_SERVER) == 5 || Scr_GetType(numGoals, SCRIPTINSTANCE_SERVER) == 6) )
    {
        if ( Scr_GetType(numGoals, SCRIPTINSTANCE_SERVER) != 5 )
            Scr_GetType(numGoals, SCRIPTINSTANCE_SERVER);
        Float = Scr_GetFloat(numGoals, SCRIPTINSTANCE_SERVER);
        veh->manualSpeed = Float * 17.6;
    }
    else
    {
        veh->manualSpeed = veh->speed;
        if ( veh->speed <= 0.0 )
        {
            v2 = va("entity %i is at rest ( speed of 0 ), new speed is needed to make plane move.", ent->s.number);
            Scr_Error(v2, 0);
        }
    }
    if ( veh->moveState == VEH_MOVESTATE_PLANE_ONCURVE )
    {
        cCurveManager::FreeCurve(veh->phys.curveID);
        veh->phys.curveID = -1;
        veh->phys.curveLength = 0.0f;
        veh->phys.curveTime = 0.0f;
    }
    VEH_GenerateCurveForPlane(ent, goals, numGoals, goalAngles);
    veh->manualMode = 1;
    avgSpeed = (float)(veh->manualSpeed + veh->speed) / 2.0;
    travelTime = veh->phys.curveLength / avgSpeed;
    speedChange = veh->manualSpeed - veh->speed;
    if ( speedChange < 0.0 )
        veh->manualDecel = speedChange / travelTime;
    else
        veh->manualAccel = speedChange / travelTime;
    veh->phys.timeStep = 40.0 / veh->phys.curveLength;
    veh->flags |= 2u;
    veh->moveState = VEH_MOVESTATE_PLANE_ONCURVE;
    veh->stopping = 0;
}

static float tweaker = 0.89999998;
void __cdecl VEH_GenerateCurveForPlane(gentity_s *ent, float (*goals)[3], int numGoals, const float *goalAngles)
{
    int v4; // edx
    double v5; // st7
    float manualSpeed; // [esp+34h] [ebp-ACh]
    int i; // [esp+68h] [ebp-78h]
    float distNodes[3]; // [esp+6Ch] [ebp-74h] BYREF
    vehicle_physic_t *phys; // [esp+78h] [ebp-68h]
    scr_vehicle_s *veh; // [esp+7Ch] [ebp-64h]
    float distBetweenGoals; // [esp+80h] [ebp-60h]
    float vecToGoal[3]; // [esp+84h] [ebp-5Ch] BYREF
    float nodes[5][3]; // [esp+90h] [ebp-50h] BYREF
    float distToGoal; // [esp+CCh] [ebp-14h]
    float distance[3]; // [esp+D0h] [ebp-10h] BYREF
    float timeStep; // [esp+DCh] [ebp-4h]

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 5599, 0, "%s", "ent") )
        __debugbreak();
    if ( !ent->scr_vehicle
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                    5600,
                    0,
                    "%s",
                    "ent->scr_vehicle") )
    {
        __debugbreak();
    }
    veh = ent->scr_vehicle;
    phys = &veh->phys;
    vecToGoal[0] = (*goals)[0] - veh->phys.origin[0];
    vecToGoal[1] = (*goals)[1] - veh->phys.origin[1];
    vecToGoal[2] = (*goals)[2] - veh->phys.origin[2];
    distToGoal = Vec3Length(vecToGoal);
    *(_QWORD *)&nodes[0][0] = *(_QWORD *)veh->phys.origin;
    nodes[0][2] = veh->phys.origin[2];
    AngleVectors(veh->phys.angles, nodes[1], 0, 0);
    nodes[1][0] = (float)(distToGoal * 0.05f) * nodes[1][0];
    nodes[1][1] = (float)(distToGoal * 0.05f) * nodes[1][1];
    nodes[1][2] = (float)(distToGoal * 0.05f) * nodes[1][2];
    nodes[1][0] = veh->phys.origin[0] + nodes[1][0];
    nodes[1][1] = veh->phys.origin[1] + nodes[1][1];
    nodes[1][2] = veh->phys.origin[2] + nodes[1][2];
    AngleVectors(veh->phys.angles, nodes[2], 0, 0);
    nodes[2][0] = (float)(distToGoal * 0.1) * nodes[2][0];
    nodes[2][1] = (float)(distToGoal * 0.1) * nodes[2][1];
    nodes[2][2] = (float)(distToGoal * 0.1) * nodes[2][2];
    nodes[2][0] = veh->phys.origin[0] + nodes[2][0];
    nodes[2][1] = veh->phys.origin[1] + nodes[2][1];
    nodes[2][2] = veh->phys.origin[2] + nodes[2][2];
    if ( veh->hasGoalYaw )
    {
        AngleVectors(goalAngles, nodes[4], 0, 0);
        manualSpeed = veh->manualSpeed;
        nodes[4][0] = manualSpeed * nodes[4][0];
        nodes[4][1] = manualSpeed * nodes[4][1];
        nodes[4][2] = manualSpeed * nodes[4][2];
        nodes[4][0] = veh->goalPosition[0] + nodes[4][0];
        nodes[4][1] = veh->goalPosition[1] + nodes[4][1];
        nodes[4][2] = veh->goalPosition[2] + nodes[4][2];
    }
    phys->curveID = cCurveManager::GetFreeCurve();
    timeStep = Dvar_GetFloat("g_vehiclePlaneCurveTime");
    if ( veh->hasGoalYaw )
        phys->curveStep = timeStep + 1.2;
    else
        phys->curveStep = timeStep + 0.2;
    cCurveManager::AddNodeToCurve(phys->curveID, nodes[0], 0.0);
    cCurveManager::AddNodeToCurve(phys->curveID, nodes[1], 0.1);
    cCurveManager::AddNodeToCurve(phys->curveID, nodes[2], 0.1);
    distance[0] = (*goals)[0] - nodes[2][0];
    distance[1] = (*goals)[1] - nodes[2][1];
    distance[2] = (*goals)[2] - nodes[2][2];
    distBetweenGoals = Vec3Length(distance);
    for ( i = 0; i < numGoals; ++i )
    {
        timeStep = (float)(distBetweenGoals / veh->manualSpeed) * tweaker;
        cCurveManager::AddNodeToCurve(phys->curveID, &(*goals)[3 * i], timeStep);
        v4 = 3 * (i + 1);
        distance[0] = (*goals)[v4] - (*goals)[3 * i];
        distance[1] = (*goals)[v4 + 1] - (*goals)[3 * i + 1];
        distance[2] = (*goals)[v4 + 2] - (*goals)[3 * i + 2];
        distBetweenGoals = Vec3Length(distance);
    }
    if ( veh->hasGoalYaw )
        cCurveManager::AddNodeToCurve(phys->curveID, nodes[4], 1.0);
    cCurveManager::BuildCurve(phys->curveID);
    phys->curveLength = cCurveManager::GetCurveLength(phys->curveID);
    if ( veh->hasGoalYaw )
    {
        distNodes[0] = nodes[4][0] - nodes[3][0];
        distNodes[1] = nodes[4][1] - nodes[3][1];
        distNodes[2] = nodes[4][2] - nodes[3][2];
        v5 = Vec3Length(distNodes);
        phys->curveLength = phys->curveLength - v5;
    }
    if ( g_vehicleDebug->current.enabled )
    {
        cCurveManager::SetCurveDraw(phys->curveID, 0);
        VEH_DebugPlaneOnCurve(ent);
    }
}

void __cdecl CMD_VEH_SetPlaneBarrelRoll(scr_entref_t entref)
{
    float v1; // xmm0_4
    VariableUnion *veh; // [esp+4h] [ebp-Ch]
    float yawDiff; // [esp+8h] [ebp-8h]

    veh = (VariableUnion *)GScr_GetVehicle(entref)->scr_vehicle;
    Scr_GetNumParam(SCRIPTINSTANCE_SERVER);
    veh[297].intValue = Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
    veh[293].floatValue = 1.0f;
    if ( (unsigned int)Scr_GetNumParam(SCRIPTINSTANCE_SERVER) <= 1 )
        veh[295].floatValue = 2.0f;
    else
        veh[295].floatValue = Scr_GetFloat(1u, SCRIPTINSTANCE_SERVER);
    veh[296].intValue = 0;
    yawDiff = veh[65].floatValue - veh[62].floatValue;
    if ( yawDiff >= 0.0 )
    {
        if ( yawDiff <= 0.0 )
        {
            if ( G_irand(0, 1) )
                v1 = (float)veh[297].intValue * -360.0;
            else
                v1 = (float)veh[297].intValue * 360.0;
            veh[294].floatValue = v1;
        }
        else
        {
            veh[294].floatValue = (float)veh[297].intValue * 360.0;
        }
    }
    else
    {
        veh[294].floatValue = (float)veh[297].intValue * -360.0;
    }
}

void __cdecl CMD_VEH_SetGoalYaw(scr_entref_t entref)
{
    scr_vehicle_s *veh; // [esp+0h] [ebp-8h]

    veh = GScr_GetVehicle(entref)->scr_vehicle;
    veh->goalYaw = Scr_GetFloat(0, SCRIPTINSTANCE_SERVER);
    veh->hasGoalYaw = 1;
}

void __cdecl CMD_VEH_ClearGoalYaw(scr_entref_t entref)
{
    scr_vehicle_s *scr_vehicle; // edx

    scr_vehicle = GScr_GetVehicle(entref)->scr_vehicle;
    scr_vehicle->goalYaw = 0.0f;
    scr_vehicle->hasGoalYaw = 0;
}

void __cdecl CMD_VEH_SetTargetYaw(scr_entref_t entref)
{
    scr_vehicle_s *veh; // [esp+0h] [ebp-8h]

    veh = GScr_GetVehicle(entref)->scr_vehicle;
    veh->targetYaw = Scr_GetFloat(0, SCRIPTINSTANCE_SERVER);
    veh->hasTargetYaw = 1;
}

void __cdecl CMD_VEH_ClearTargetYaw(scr_entref_t entref)
{
    scr_vehicle_s *scr_vehicle; // edx

    scr_vehicle = GScr_GetVehicle(entref)->scr_vehicle;
    scr_vehicle->targetYaw = 0.0f;
    scr_vehicle->hasTargetYaw = 0;
}

void __cdecl CMD_VEH_SetDefaultPitch(scr_entref_t entref)
{
    scr_vehicle_s *veh; // [esp+0h] [ebp-8h]

    veh = GScr_GetVehicle(entref)->scr_vehicle;
    veh->defaultPitch = Scr_GetFloat(0, SCRIPTINSTANCE_SERVER);
    veh->hasDefaultPitch = 1;
}

void __cdecl CMD_VEH_ClearDefaultPitch(scr_entref_t entref)
{
    scr_vehicle_s *scr_vehicle; // edx

    scr_vehicle = GScr_GetVehicle(entref)->scr_vehicle;
    scr_vehicle->defaultPitch = 0.0f;
    scr_vehicle->hasDefaultPitch = 0;
}

void __cdecl CMD_VEH_GetAngularVelocity(scr_entref_t entref)
{
    scr_vehicle_s *veh; // [esp+4h] [ebp-8h]

    veh = GScr_GetVehicle(entref)->scr_vehicle;
    Scr_AddVector(veh->phys.rotVel, SCRIPTINSTANCE_SERVER);
}

void __cdecl CMD_VEH_SetAngularVelocity(scr_entref_t entref)
{
    scr_vehicle_s *veh; // [esp+4h] [ebp-8h]

    veh = GScr_GetVehicle(entref)->scr_vehicle;
    Scr_GetVector(0, veh->phys.rotVel, SCRIPTINSTANCE_SERVER);
}

void __cdecl CMD_VEH_SetVehVelocity(scr_entref_t entref)
{
    const char *v1; // eax
    scr_vehicle_s *veh; // [esp+8h] [ebp-8h]

    veh = GScr_GetVehicle(entref)->scr_vehicle;
    if ( BG_GetVehicleInfo(veh->infoIdx)->type != 6 )
    {
        v1 = va("SetVelocity called only be called on a helicopter\n");
        Scr_Error(v1, 0);
    }
    Scr_GetVector(0, veh->phys.vel, SCRIPTINSTANCE_SERVER);
}

void __cdecl CMD_VEH_SetTurretTargetVec(scr_entref_t entref)
{
    const char *v1; // eax
    const char *v2; // eax
    scr_vehicle_s *veh; // [esp+8h] [ebp-14h]
    gentity_s *ent; // [esp+Ch] [ebp-10h]
    float tgtPos[3]; // [esp+10h] [ebp-Ch] BYREF

    ent = GScr_GetVehicle(entref);
    veh = ent->scr_vehicle;
    if ( (veh->flags & 0x10) == 0 )
    {
        if ( (veh->flags & 1) != 0 )
        {
            v1 = va("Can't set target position on player's vehicle");
            Scr_Error(v1, 0);
        }
        if ( (float)ent->health <= 0.0 )
        {
            v2 = va("Vehicle must have health to control the turret");
            Scr_Error(v2, 0);
        }
        veh->hasTarget = 1;
        veh->targetEnt = 1023;
        Scr_GetVector(0, tgtPos, SCRIPTINSTANCE_SERVER);
        veh->targetOrigin[0] = tgtPos[0];
        veh->targetOrigin[1] = tgtPos[1];
        veh->targetOrigin[2] = tgtPos[2];
        veh->targetOffset[0] = 0.0f;
        veh->targetOffset[1] = 0.0f;
        veh->targetOffset[2] = 0.0f;
    }
}

void __cdecl CMD_VEH_SetTurretTargetEnt(scr_entref_t entref)
{
    const char *v1; // eax
    int number; // [esp+0h] [ebp-18h]
    scr_vehicle_s *veh; // [esp+Ch] [ebp-Ch]
    gentity_s *tgtEnt; // [esp+10h] [ebp-8h]
    gentity_s *ent; // [esp+14h] [ebp-4h]

    ent = GScr_GetVehicle(entref);
    veh = ent->scr_vehicle;
    if ( (veh->flags & 0x10) == 0 )
    {
        if ( (float)ent->health <= 0.0 )
        {
            v1 = va("Vehicle must have health to control the turret");
            Scr_Error(v1, 0);
        }
        tgtEnt = Scr_GetEntity(0);
        veh->hasTarget = 1;
        if ( tgtEnt )
            number = tgtEnt->s.number;
        else
            number = 1023;
        veh->targetEnt = number;
        if ( tgtEnt )
            tgtEnt->flags |= 0x80000u;
        if ( (unsigned int)Scr_GetNumParam(SCRIPTINSTANCE_SERVER) <= 1 )
        {
            veh->targetOffset[0] = 0.0f;
            veh->targetOffset[1] = 0.0f;
            veh->targetOffset[2] = 0.0f;
        }
        else
        {
            Scr_GetVector(1u, veh->targetOffset, SCRIPTINSTANCE_SERVER);
        }
        veh->targetOrigin[0] = 0.0f;
        veh->targetOrigin[1] = 0.0f;
        veh->targetOrigin[2] = 0.0f;
    }
}

void __cdecl CMD_VEH_ClearTurretTarget(scr_entref_t entref)
{
    scr_vehicle_s *veh; // [esp+8h] [ebp-8h]

    veh = GScr_GetVehicle(entref)->scr_vehicle;
    if ( (veh->flags & 0x10) == 0 )
    {
        if ( (veh->flags & 1) == 0 )
            veh->hasTarget = 0;
        veh->targetEnt = 1023;
        veh->targetOrigin[0] = 0.0f;
        veh->targetOrigin[1] = 0.0f;
        veh->targetOrigin[2] = 0.0f;
        veh->targetOffset[0] = 0.0f;
        veh->targetOffset[1] = 0.0f;
        veh->targetOffset[2] = 0.0f;
    }
}

void __cdecl CMD_VEH_SetGunnerTargetVec(scr_entref_t entref)
{
    unsigned int *targetOffset; // [esp+0h] [ebp-20h]
    float *targetOrigin; // [esp+4h] [ebp-1Ch]
    scr_vehicle_s *veh; // [esp+8h] [ebp-18h]
    unsigned int gunnerIndex; // [esp+Ch] [ebp-14h]
    float tgtPos[3]; // [esp+14h] [ebp-Ch] BYREF

    veh = GScr_GetVehicle(entref)->scr_vehicle;
    gunnerIndex = 0;
    Scr_GetVector(0, tgtPos, SCRIPTINSTANCE_SERVER);
    if ( (unsigned int)Scr_GetNumParam(SCRIPTINSTANCE_SERVER) > 1 )
        gunnerIndex = Scr_GetInt(1u, SCRIPTINSTANCE_SERVER);
    if ( gunnerIndex >= 4 )
        Scr_Error("Invalid gunner index set in SetGunnerTargetVec", 0);
    targetOrigin = veh->gunnerTargets[gunnerIndex].targetOrigin;
    *targetOrigin = tgtPos[0];
    targetOrigin[1] = tgtPos[1];
    targetOrigin[2] = tgtPos[2];
    targetOffset = (unsigned int *)veh->gunnerTargets[gunnerIndex].targetOffset;
    *targetOffset = 0;
    targetOffset[1] = 0;
    targetOffset[2] = 0;
    veh->gunnerTargets[gunnerIndex].valid = 1;
    veh->gunnerTargets[gunnerIndex].targetEnt = 1023;
}

void __cdecl CMD_VEH_GetGunnerTargetVec(scr_entref_t entref)
{
    scr_vehicle_s *veh; // [esp+0h] [ebp-Ch]
    int gunnerIndex; // [esp+4h] [ebp-8h]

    veh = GScr_GetVehicle(entref)->scr_vehicle;
    gunnerIndex = Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
    if ( (unsigned int)gunnerIndex >= 4 )
        Scr_Error("Invalid gunner index set in GetGunnerTargetVec", 0);
    Scr_AddVector(veh->gunnerTargets[gunnerIndex].targetOrigin, SCRIPTINSTANCE_SERVER);
}

void __cdecl CMD_VEH_SetGunnerTargetEnt(scr_entref_t entref)
{
    int targetOrigin; // edx
    int number; // [esp+0h] [ebp-28h]
    float *v3; // [esp+8h] [ebp-20h]
    scr_vehicle_s *veh; // [esp+Ch] [ebp-1Ch]
    unsigned int gunnerIndex; // [esp+10h] [ebp-18h]
    gentity_s *tgtEnt; // [esp+14h] [ebp-14h]
    float targetOffset[3]; // [esp+18h] [ebp-10h] BYREF
    gentity_s *ent; // [esp+24h] [ebp-4h]

    ent = GScr_GetVehicle(entref);
    veh = ent->scr_vehicle;
    gunnerIndex = 0;
    tgtEnt = Scr_GetEntity(0);
    if ( (unsigned int)Scr_GetNumParam(SCRIPTINSTANCE_SERVER) <= 1 )
        memset(targetOffset, 0, sizeof(targetOffset));
    else
        Scr_GetVector(1u, targetOffset, SCRIPTINSTANCE_SERVER);
    if ( (unsigned int)Scr_GetNumParam(SCRIPTINSTANCE_SERVER) > 2 )
        gunnerIndex = Scr_GetInt(2u, SCRIPTINSTANCE_SERVER);
    if ( gunnerIndex >= 4 )
        Scr_Error("Invalid gunner index set in SetGunnerTargetEnt", 0);
    veh->gunnerTargets[gunnerIndex].valid = 1;
    if ( tgtEnt )
        number = tgtEnt->s.number;
    else
        number = 1023;
    veh->gunnerTargets[gunnerIndex].targetEnt = number;
    v3 = veh->gunnerTargets[gunnerIndex].targetOffset;
    *v3 = targetOffset[0];
    v3[1] = targetOffset[1];
    v3[2] = targetOffset[2];
    veh->gunnerTargets[gunnerIndex].targetOrigin[0] = 0.0f;
    targetOrigin = (int)veh->gunnerTargets[gunnerIndex].targetOrigin;
    *(unsigned int *)(targetOrigin + 4) = 0;
    *(unsigned int *)(targetOrigin + 8) = 0;
}

void __cdecl CMD_VEH_ClearGunnerTarget(scr_entref_t entref)
{
    unsigned int *targetOrigin; // [esp+0h] [ebp-14h]
    unsigned int *targetOffset; // [esp+4h] [ebp-10h]
    scr_vehicle_s *veh; // [esp+8h] [ebp-Ch]
    unsigned int gunnerIndex; // [esp+Ch] [ebp-8h]

    veh = GScr_GetVehicle(entref)->scr_vehicle;
    gunnerIndex = 0;
    if ( Scr_GetNumParam(SCRIPTINSTANCE_SERVER) )
        gunnerIndex = Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
    if ( gunnerIndex >= 4 )
        Scr_Error("Invalid gunner index set in ClearGunnerTarget", 0);
    veh->gunnerTargets[gunnerIndex].valid = 0;
    veh->gunnerTargets[gunnerIndex].targetEnt = 1023;
    targetOffset = (unsigned int *)veh->gunnerTargets[gunnerIndex].targetOffset;
    *targetOffset = 0;
    targetOffset[1] = 0;
    targetOffset[2] = 0;
    targetOrigin = (unsigned int *)veh->gunnerTargets[gunnerIndex].targetOrigin;
    *targetOrigin = 0;
    targetOrigin[1] = 0;
    targetOrigin[2] = 0;
}

void __cdecl CMD_VEH_SetGunnerTurretOnTargetRange(scr_entref_t entref)
{
    scr_vehicle_s *veh; // [esp+0h] [ebp-10h]
    float range; // [esp+4h] [ebp-Ch]
    int gunnerIndex; // [esp+8h] [ebp-8h]

    veh = GScr_GetVehicle(entref)->scr_vehicle;
    range = 0.0f;
    gunnerIndex = Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
    if ( (unsigned int)Scr_GetNumParam(SCRIPTINSTANCE_SERVER) > 1 )
        range = Scr_GetFloat(1u, SCRIPTINSTANCE_SERVER);
    if ( (unsigned int)gunnerIndex >= 4 )
        Scr_Error("Invalid gunner index set in SetGunnerTurretOnTargetRange", 0);
    veh->gunnerTurrets[gunnerIndex].turretOnTargetRange = range;
}

void __cdecl CMD_VEH_GetGunnerTargetEnt(scr_entref_t entref)
{
    gentity_s *tgtEnt; // [esp+0h] [ebp-10h]
    scr_vehicle_s *veh; // [esp+4h] [ebp-Ch]
    int gunnerIndex; // [esp+8h] [ebp-8h]

    veh = GScr_GetVehicle(entref)->scr_vehicle;
    gunnerIndex = Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
    if ( (unsigned int)gunnerIndex >= 4 )
        Scr_Error("Invalid gunner index set in GetGunnerTargetEnt", 0);
    if ( veh->gunnerTargets[gunnerIndex].targetEnt == 1023
        || (tgtEnt = &g_entities[veh->gunnerTargets[gunnerIndex].targetEnt], !tgtEnt->r.inuse) )
    {
        Scr_AddUndefined(SCRIPTINSTANCE_SERVER);
    }
    else
    {
        Scr_AddEntity(tgtEnt, SCRIPTINSTANCE_SERVER);
    }
}

void __cdecl CMD_VEH_GetGunnerAnimPitch(scr_entref_t entref)
{
    const vehicle_info_t *info; // [esp+Ch] [ebp-14h]
    int gunnerIndex; // [esp+14h] [ebp-Ch]
    float pitch; // [esp+18h] [ebp-8h]
    gentity_s *ent; // [esp+1Ch] [ebp-4h]

    ent = GScr_GetVehicle(entref);
    info = BG_GetVehicleInfo(ent->scr_vehicle->infoIdx);
    gunnerIndex = Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
    if ( (unsigned int)gunnerIndex >= 4 )
        Scr_Error("Invalid gunner index set in GetGunnerAnimPitch", 0);
    pitch = AngleNormalize180(
                        (float)((float)ent->s.lerp.u.vehicle.gunnerAngles[gunnerIndex].pitch * 0.0054931641)
                    - info->gunnerRestAngles[gunnerIndex][0]);
    Scr_AddFloat(pitch, SCRIPTINSTANCE_SERVER);
}

void __cdecl CMD_VEH_GetGunnerAnimYaw(scr_entref_t entref)
{
    const vehicle_info_t *info; // [esp+Ch] [ebp-14h]
    int gunnerIndex; // [esp+14h] [ebp-Ch]
    float yaw; // [esp+18h] [ebp-8h]
    gentity_s *ent; // [esp+1Ch] [ebp-4h]

    ent = GScr_GetVehicle(entref);
    info = BG_GetVehicleInfo(ent->scr_vehicle->infoIdx);
    gunnerIndex = Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
    if ( (unsigned int)gunnerIndex >= 4 )
        Scr_Error("Invalid gunner index set in GetGunnerAnimYaw", 0);
    yaw = AngleNormalize180(
                    (float)((float)ent->s.lerp.u.vehicle.gunnerAngles[gunnerIndex].yaw * 0.0054931641)
                - info->gunnerRestAngles[gunnerIndex][1]);
    Scr_AddFloat(yaw, SCRIPTINSTANCE_SERVER);
}

void __cdecl CMD_VEH_SetLookAtEnt(scr_entref_t entref)
{
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    scr_vehicle_s *veh; // [esp+0h] [ebp-Ch]
    gentity_s *tgtEnt; // [esp+4h] [ebp-8h]
    gentity_s *ent; // [esp+8h] [ebp-4h]

    ent = GScr_GetVehicle(entref);
    veh = ent->scr_vehicle;
    if ( (veh->flags & 1) != 0 )
    {
        v1 = va("Can't set target on player's vehicle");
        Scr_Error(v1, 0);
    }
    if ( (float)ent->health <= 0.0 )
    {
        v2 = va("Vehicle must have health to control");
        Scr_Error(v2, 0);
    }
    tgtEnt = Scr_GetEntity(0);
    if ( !tgtEnt )
    {
        v3 = va("Invalid entity");
        Scr_Error(v3, 0);
    }
    //EntHandle::setEnt(&veh->lookAtEnt, tgtEnt);
    veh->lookAtEnt.setEnt(tgtEnt);
}

void __cdecl CMD_VEH_ClearLookAtEnt(scr_entref_t entref)
{
    gentity_s *ent; // [esp+4h] [ebp-4h]

    ent = GScr_GetVehicle(entref);
    ent->scr_vehicle->lookAtEnt.setEnt(0);
}

void __cdecl CMD_VEH_ReturnPlayerControl(scr_entref_t entref)
{
    gentity_s *ent; // [esp+8h] [ebp-4h]

    ent = GScr_GetVehicle(entref);
    if ( ent->r.ownerNum.isDefined() )
        VEH_CancelAIMove(ent);
    if ( ent->scr_vehicle )
        ent->scr_vehicle->flags &= 0xFFFFFE7F;
}

void __cdecl VEH_CancelAIMove(gentity_s *ent)
{
    const vehicle_info_t *info; // [esp+0h] [ebp-8h]
    scr_vehicle_s *veh; // [esp+4h] [ebp-4h]

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 4969, 0, "%s", "ent") )
        __debugbreak();
    if ( !ent->scr_vehicle
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                    4970,
                    0,
                    "%s",
                    "ent->scr_vehicle") )
    {
        __debugbreak();
    }
    veh = ent->scr_vehicle;
    info = BG_GetVehicleInfo(veh->infoIdx);
    veh->flags &= ~2u;
    if ( info->type == 6 )
        HELI_CancelAIMove(ent);
}

void __cdecl CMD_VEH_SetWeapon(scr_entref_t entref)
{
    char *String; // eax
    int WeaponIndexForName; // eax
    gentity_s *ent; // [esp+8h] [ebp-4h]

    ent = GScr_GetVehicle(entref);
    String = Scr_GetString(0, SCRIPTINSTANCE_SERVER);
    WeaponIndexForName = BG_FindWeaponIndexForName(String);
    AssignToSmallerType<unsigned short>(&ent->s.weapon, WeaponIndexForName);
    ent->s.weaponModel = 0;
}

void __cdecl CMD_VEH_FireWeapon(scr_entref_t entref)
{
    const char *v1; // eax
    char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    char *v5; // eax
    const char *v6; // eax
    unsigned int v7; // eax
    gentity_s *v8; // [esp+14h] [ebp-184h]
    gentity_s *Entity; // [esp+18h] [ebp-180h]
    gentity_s *v10; // [esp+1Ch] [ebp-17Ch]
    float v11; // [esp+20h] [ebp-178h]
    float v12; // [esp+24h] [ebp-174h]
    float v13; // [esp+2Ch] [ebp-16Ch]
    float v14; // [esp+44h] [ebp-154h]
    float aimPadding; // [esp+50h] [ebp-148h]
    float spread; // [esp+68h] [ebp-130h]
    gentity_s *driver; // [esp+6Ch] [ebp-12Ch]
    float curiosity[3]; // [esp+70h] [ebp-128h] BYREF
    gentity_s *attacker; // [esp+7Ch] [ebp-11Ch]
    float joltDir[3]; // [esp+80h] [ebp-118h] BYREF
    int numBarrels; // [esp+8Ch] [ebp-10Ch]
    int boneIndex; // [esp+90h] [ebp-108h]
    unsigned __int16 boneName; // [esp+94h] [ebp-104h]
    const vehicle_info_t *info; // [esp+98h] [ebp-100h]
    scr_vehicle_s *veh; // [esp+9Ch] [ebp-FCh]
    float diffAngles[3]; // [esp+A0h] [ebp-F8h] BYREF
    int event; // [esp+ACh] [ebp-ECh]
    gentity_s *target; // [esp+B0h] [ebp-E8h]
    int startBarrel; // [esp+B4h] [ebp-E4h]
    gentity_s *proj; // [esp+B8h] [ebp-E0h]
    float gunAngles[3]; // [esp+BCh] [ebp-DCh] BYREF
    weaponParms wp; // [esp+C8h] [ebp-D0h] BYREF
    float targetOffset[3]; // [esp+110h] [ebp-88h] BYREF
    gentity_s *ent; // [esp+11Ch] [ebp-7Ch]
    float bulletAngles[3]; // [esp+120h] [ebp-78h] BYREF
    int i; // [esp+12Ch] [ebp-6Ch]
    float barrelMtx[4][3]; // [esp+130h] [ebp-68h] BYREF
    int barrel; // [esp+160h] [ebp-38h]
    gentity_s *player; // [esp+164h] [ebp-34h]
    float flashMtx[4][3]; // [esp+168h] [ebp-30h] BYREF
    int savedregs; // [esp+198h] [ebp+0h] BYREF

    ent = GScr_GetVehicle(entref);
    veh = ent->scr_vehicle;
    info = BG_GetVehicleInfo(veh->infoIdx);
    player = 0;
    boneIndex = 0;
    if ( (float)ent->health <= 0.0 )
    {
        v1 = va("Vehicle must have health to control the turret");
        Scr_Error(v1, 0);
    }
    if ( !ent->s.weapon )
    {
        v2 = SL_ConvertToString(ent->targetname, SCRIPTINSTANCE_SERVER);
        v3 = va("Invalid weapon specified for [%s]\n", v2);
        Scr_Error(v3, 0);
    }
    Weapon_SetWeaponParamsWeapon(&wp, ent->s.weapon);
    if ( wp.weapDef->weapType
        && wp.weapDef->weapType != WEAPTYPE_PROJECTILE
        && wp.weapDef->weapType != WEAPTYPE_BOMB
        && wp.weapDef->weapType != WEAPTYPE_GAS
        && wp.weapDef->weapType != WEAPTYPE_GRENADE )
    {
        v4 = va("Vehicles only support bullet, bomb, gas, grenade and projectile weapons\n");
        Scr_Error(v4, 0);
    }
    if ( info->type != 3 && info->type != 6 )
    {
        if ( veh->boneIndex.barrel < 0 )
        {
            v5 = SL_ConvertToString(ent->targetname, SCRIPTINSTANCE_SERVER);
            v6 = va("No tag_barrel for [%s]\n", v5);
            Scr_Error(v6, 0);
        }
        G_DObjGetWorldBoneIndexMatrix(ent, veh->boneIndex.barrel, barrelMtx);
        joltDir[0] = -barrelMtx[0][0];
        joltDir[1] = -barrelMtx[0][1];
        joltDir[2] = -barrelMtx[0][2];
        VEH_JoltBody(ent, joltDir, 1.0, 0.0, 0.0);
    }
    if ( Scr_GetNumParam(SCRIPTINSTANCE_SERVER) )
        boneName = (unsigned __int16)Scr_GetConstLowercaseString(0, SCRIPTINSTANCE_SERVER);
    else
        boneName = 0;
    if ( !Scr_GetNumParam(SCRIPTINSTANCE_SERVER) && info->quadBarrel )
    {
        numBarrels = 2;
        startBarrel = veh->turret.fireBarrel != 0 ? 2 : 0;
        event = (veh->turret.fireBarrel != 0) + 49;
    }
    else
    {
        numBarrels = 1;
        startBarrel = 0;
        event = 29;
    }
    for ( i = 0; i < numBarrels; ++i )
    {
        barrel = i + startBarrel;
        boneIndex = VEH_GetTagBoneIndex(ent, i + startBarrel);
        if ( boneIndex < 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                        12004,
                        1,
                        "%s",
                        "boneIndex >= 0") )
        {
            __debugbreak();
        }
        G_DObjGetWorldBoneIndexMatrix(ent, boneIndex, flashMtx);
        //if ( EntHandle::isDefined(&ent->r.ownerNum)
        if ( ent->r.ownerNum.isDefined()
            && numBarrels == 1
            && (veh->turret.flags & 1) == 0
            && veh->targetEnt == 1023
            && wp.weapDef->weapType )
        {
            //player = EntHandle::ent(&ent->r.ownerNum);
            player = ent->r.ownerNum.ent();
            if ( !player->client
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                            12015,
                            0,
                            "%s",
                            "player->client") )
            {
                __debugbreak();
            }
            wp.gunForward[0] = veh->targetOrigin[0] - flashMtx[3][0];
            wp.gunForward[1] = veh->targetOrigin[1] - flashMtx[3][1];
            wp.gunForward[2] = veh->targetOrigin[2] - flashMtx[3][2];
            Vec3Normalize(wp.gunForward);
            vectoangles(flashMtx[0], gunAngles);
            vectoangles(wp.gunForward, bulletAngles);
            AnglesSubtract(gunAngles, bulletAngles, diffAngles);
            if ( (float)(diffAngles[0] - wp.weapDef->aimPadding) < 0.0 )
                aimPadding = diffAngles[0];
            else
                aimPadding = wp.weapDef->aimPadding;
            if ( (float)((-(wp.weapDef->aimPadding)) - diffAngles[0]) < 0.0 )
                v12 = aimPadding;
            else
                v12 = -wp.weapDef->aimPadding;
            diffAngles[0] = v12;
            if ( (float)(diffAngles[1] - wp.weapDef->aimPadding) < 0.0 )
                v14 = diffAngles[1];
            else
                v14 = wp.weapDef->aimPadding;
            if ( (float)((-(wp.weapDef->aimPadding)) - diffAngles[1]) < 0.0 )
                v11 = v14;
            else
                v11 = -wp.weapDef->aimPadding;
            diffAngles[1] = v11;
            diffAngles[2] = 0.0f;
            AnglesSubtract(gunAngles, diffAngles, gunAngles);
            AngleVectors(gunAngles, wp.gunForward, 0, 0);
        }
        else
        {
            *(_QWORD *)wp.gunForward = *(_QWORD *)&flashMtx[0][0];
            wp.gunForward[2] = flashMtx[0][2];
        }
        vectoangles(wp.gunForward, curiosity);
        wp.forward[0] = wp.gunForward[0];
        wp.forward[1] = wp.gunForward[1];
        wp.forward[2] = wp.gunForward[2];
        *(_QWORD *)wp.right = *(_QWORD *)&flashMtx[1][0];
        wp.right[2] = flashMtx[1][2];
        *(_QWORD *)wp.up = *(_QWORD *)&flashMtx[2][0];
        wp.up[2] = flashMtx[2][2];
        if ( (veh->turret.flags & 1) != 0 )
        {
            v13 = -veh->turret.barrelOffset;
            wp.muzzleTrace[0] = (float)(v13 * wp.gunForward[0]) + flashMtx[3][0];
            wp.muzzleTrace[1] = (float)(v13 * wp.gunForward[1]) + flashMtx[3][1];
            wp.muzzleTrace[2] = (float)(v13 * wp.gunForward[2]) + flashMtx[3][2];
        }
        else
        {
            *(_QWORD *)wp.muzzleTrace = *(_QWORD *)&flashMtx[3][0];
            wp.muzzleTrace[2] = flashMtx[3][2];
        }
        driver = VEH_GetSeatOccupantEntity(ent, 0);
        if ( driver )
            v10 = driver;
        else
            v10 = ent;
        attacker = v10;
        if ( v10 && attacker->client )
            spread = wp.weapDef->playerSpread;
        else
            spread = wp.weapDef->aiSpread;
        if ( wp.weapDef->weapType == WEAPTYPE_BULLET )
        {
            Bullet_Fire(attacker, spread, &wp, ent, level.time);
            continue;
        }
        if ( wp.weapDef->weapType == WEAPTYPE_BOMB )
        {
            if ( (unsigned int)Scr_GetNumParam(SCRIPTINSTANCE_SERVER) <= 1 )
                Entity = 0;
            else
                Entity = Scr_GetEntity(1u);
            target = Entity;
            if ( (unsigned int)Scr_GetNumParam(SCRIPTINSTANCE_SERVER) <= 2 )
                memset(targetOffset, 0, sizeof(targetOffset));
            else
                Scr_GetVector(2u, targetOffset, SCRIPTINSTANCE_SERVER);
            proj = Weapon_BombDrop_Fire(
                             attacker,
                             ent->s.weapon,
                             wp.weapDef->fAdsSpread,
                             &wp,
                             veh->phys.vel,
                             target,
                             targetOffset);
            if ( !proj
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 12085, 0, "%s", "proj") )
            {
                __debugbreak();
            }
LABEL_87:
            Scr_AddEntity(proj, SCRIPTINSTANCE_SERVER);
            continue;
        }
        if ( wp.weapDef->weapType != WEAPTYPE_GAS )
        {
            if ( (unsigned int)Scr_GetNumParam(SCRIPTINSTANCE_SERVER) <= 1 )
                v8 = 0;
            else
                v8 = Scr_GetEntity(1u);
            target = v8;
            if ( (unsigned int)Scr_GetNumParam(SCRIPTINSTANCE_SERVER) <= 2 )
                memset(targetOffset, 0, sizeof(targetOffset));
            else
                Scr_GetVector(2u, targetOffset, SCRIPTINSTANCE_SERVER);
            proj = Weapon_RocketLauncher_Fire(
                             attacker,
                             ent->s.weapon,
                             wp.weapDef->fAdsSpread,
                             &wp,
                             veh->phys.vel,
                             target,
                             targetOffset);
            if ( !proj
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 12105, 0, "%s", "proj") )
            {
                __debugbreak();
            }
            goto LABEL_87;
        }
        veh->flags |= 0x40u;
        ent->s.lerp.eFlags |= 0x40u;
        if ( attacker->client )
            attacker->client->ps.eFlags |= 0x40u;
        Weapon_Flamethrower_Fire(attacker, &wp);
    }
    if ( info->type == 6 )
    {
        G_AddEvent(ent, 0xADu, 0x7FEu);
    }
    else
    {
        //v7 = EntHandle::entnum(&ent->r.ownerNum);
        v7 = ent->r.ownerNum.entnum();
        G_AddEvent(ent, 0xADu, v7);
    }
    Scr_Notify(ent, scr_const.weapon_fired, 0);
    veh->turret.fireTime = wp.weapDef->iFireTime;
    veh->turret.fireBarrel = veh->turret.fireBarrel == 0;
}

int __cdecl VEH_GetTagBoneIndex(gentity_s *ent, int barrel)
{
    VariableUnion v2; // eax
    char *v3; // eax
    const char *v4; // eax
    char *v5; // eax
    const char *v6; // eax
    char *v8; // [esp-8h] [ebp-14h]
    char *boneName; // [esp+0h] [ebp-Ch]
    int boneIndex; // [esp+4h] [ebp-8h]
    scr_vehicle_s *veh; // [esp+8h] [ebp-4h]

    boneIndex = -1;
    veh = ent->scr_vehicle;
    if ( Scr_GetNumParam(SCRIPTINSTANCE_SERVER) )
    {
        boneName = Scr_GetString(0, SCRIPTINSTANCE_SERVER);
        if ( *boneName )
        {
            v2.intValue = Scr_GetConstLowercaseString(0, SCRIPTINSTANCE_SERVER);
            boneIndex = SV_DObjGetBoneIndex(ent, v2.stringValue);
            if ( boneIndex < 0 )
            {
                v3 = SL_ConvertToString(ent->targetname, SCRIPTINSTANCE_SERVER);
                v4 = va("No tag %s for [%s]\n", boneName, v3);
                Scr_Error(v4, 0);
            }
        }
        else
        {
            boneIndex = -1;
        }
    }
    if ( !Scr_GetNumParam(SCRIPTINSTANCE_SERVER) || boneIndex < 0 )
    {
        if ( veh->boneIndex.flash[barrel] < 0 )
        {
            v8 = SL_ConvertToString(ent->targetname, SCRIPTINSTANCE_SERVER);
            v5 = SL_ConvertToString(*s_flashTags[barrel], SCRIPTINSTANCE_SERVER);
            v6 = va("No %s for [%s]\n", v5, v8);
            Scr_Error(v6, 0);
        }
        else
        {
            return veh->boneIndex.flash[barrel];
        }
    }
    return boneIndex;
}

void __cdecl CMD_VEH_FireGunnerWeapon(scr_entref_t entref)
{
    VariableUnion gunnerIndex; // [esp+0h] [ebp-Ch]
    gentity_s *proj; // [esp+4h] [ebp-8h]
    gentity_s *ent; // [esp+8h] [ebp-4h]

    ent = GScr_GetVehicle(entref);
    gunnerIndex.intValue = 0;
    if ( Scr_GetNumParam(SCRIPTINSTANCE_SERVER) )
        gunnerIndex.intValue = Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
    if ( gunnerIndex.intValue >= 4u )
        Scr_Error("Invalid gunner index set in FireGunnerWeapon", 0);
    proj = VEH_FireGunnerWeapon(ent, gunnerIndex.intValue, 0);
    if ( proj )
        Scr_AddEntity(proj, SCRIPTINSTANCE_SERVER);
}

void __cdecl CMD_VEH_StopFireWeapon(scr_entref_t entref)
{
    gentity_s *ent; // [esp+8h] [ebp-4h]

    ent = GScr_GetVehicle(entref);
    ent->s.lerp.eFlags &= ~0x40u;
    ent->scr_vehicle->flags &= ~0x40u;
}

void __cdecl CMD_VEH_IsTurretReady(scr_entref_t entref)
{
    const char *v1; // eax
    scr_vehicle_s *veh; // [esp+0h] [ebp-8h]

    veh = GScr_GetVehicle(entref)->scr_vehicle;
    if ( (veh->flags & 1) == 0 )
    {
        v1 = va("Must be called on a player controlled vehicle");
        Scr_Error(v1, 0);
    }
    if ( veh->turret.fireTime > 0 )
        Scr_AddInt(0, SCRIPTINSTANCE_SERVER);
    else
        Scr_AddInt(1, SCRIPTINSTANCE_SERVER);
}

void __cdecl CMD_VEH_ForceMaterialSpeed(scr_entref_t entref)
{
    scr_vehicle_s *veh; // [esp+0h] [ebp-10h]
    float speed; // [esp+Ch] [ebp-4h]

    veh = GScr_GetVehicle(entref)->scr_vehicle;
    if ( !veh && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 12231, 0, "%s", "veh") )
        __debugbreak();
    if ( Scr_GetInt(0, SCRIPTINSTANCE_SERVER) )
    {
        speed = Scr_GetFloat(1u, SCRIPTINSTANCE_SERVER);
        veh->flags |= 4u;
        veh->forcedMaterialSpeed = speed;
    }
    else
    {
        veh->flags &= ~4u;
    }
}

void __cdecl CMD_VEH_SetMaxSpeed(scr_entref_t entref)
{
    float max_speed_scale; // [esp+8h] [ebp-10h]
    scr_vehicle_s *veh; // [esp+Ch] [ebp-Ch]
    float speed; // [esp+14h] [ebp-4h]

    veh = GScr_GetVehicle(entref)->scr_vehicle;
    if ( !veh && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp", 12266, 0, "%s", "veh") )
        __debugbreak();
    speed = Scr_GetFloat(0, SCRIPTINSTANCE_SERVER);
    veh->maxSpeedOverride = speed * 17.6;
    if ( veh->nitrousVehicle )
    {
        if ( onlinegame->current.enabled )
            Scr_Error("Can not use SetVehMaxSpeed in online games for physics vehicles.", 0);
        if ( speed <= 0.0 )
        {
            veh->nitrousVehicle->m_speed_scale = 1.0f;
        }
        else
        {
            //max_speed_scale = veh->maxSpeedOverride / NitrousVehicle::get_max_speed(veh->nitrousVehicle, 1);
            max_speed_scale = veh->maxSpeedOverride / veh->nitrousVehicle->get_max_speed(true);
            veh->nitrousVehicle->m_speed_scale = max_speed_scale;
        }
    }
}

void __cdecl CMD_VEH_GetOccupants(scr_entref_t entref)
{
    gentity_s *occupant; // [esp+0h] [ebp-Ch]
    gentity_s *ent; // [esp+4h] [ebp-8h]
    int seatIndex; // [esp+8h] [ebp-4h]

    ent = GScr_GetVehicle(entref);
    if ( !ent->scr_vehicle
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                    12308,
                    0,
                    "%s",
                    "ent->scr_vehicle") )
    {
        __debugbreak();
    }
    Scr_MakeArray(SCRIPTINSTANCE_SERVER);
    for ( seatIndex = 0; seatIndex < 11; ++seatIndex )
    {
        occupant = VEH_GetSeatOccupantEntity(ent, seatIndex);
        if ( occupant )
        {
            Scr_AddEntity(occupant, SCRIPTINSTANCE_SERVER);
            Scr_AddArray(SCRIPTINSTANCE_SERVER);
        }
    }
}

void __cdecl CMD_VEH_GetSeatOccupant(scr_entref_t entref)
{
    gentity_s *occupant; // [esp+0h] [ebp-Ch]
    gentity_s *vehicle; // [esp+4h] [ebp-8h]
    int seatIndex; // [esp+8h] [ebp-4h]

    vehicle = GScr_GetVehicle(entref);
    seatIndex = Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
    occupant = VEH_GetSeatOccupantEntity(vehicle, seatIndex);
    if ( occupant )
        Scr_AddEntity(occupant, SCRIPTINSTANCE_SERVER);
}

void __cdecl CMD_VEH_GetOccupantSeat(scr_entref_t entref)
{
    gentity_s *vehicle; // [esp+0h] [ebp-Ch]
    gentity_s *player; // [esp+4h] [ebp-8h]
    int seatIndex; // [esp+8h] [ebp-4h]

    vehicle = GScr_GetVehicle(entref);
    player = Scr_GetEntity(0);
    seatIndex = G_GetVehicleSeatPlayerOccupies(vehicle, player);
    if ( seatIndex >= 0 )
        Scr_AddInt(seatIndex, SCRIPTINSTANCE_SERVER);
}

void __cdecl CMD_VEH_SetSeatOccupied(scr_entref_t entref)
{
    int occupied; // [esp+0h] [ebp-10h]
    gentity_s *vehicle; // [esp+4h] [ebp-Ch]
    int seatIndex; // [esp+8h] [ebp-8h]
    VehicleSeat *seat; // [esp+Ch] [ebp-4h]

    vehicle = GScr_GetVehicle(entref);
    seatIndex = Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
    occupied = 1;
    if ( (unsigned int)Scr_GetNumParam(SCRIPTINSTANCE_SERVER) > 1 )
        occupied = Scr_GetInt(1u, SCRIPTINSTANCE_SERVER);
    seat = VEH_GetSeat(vehicle, seatIndex);
    if ( !seat )
        Scr_Error("Invalid seat specified for vehicle in SetSeatOccupied.", 0);
    if ( occupied )
    {
        if ( seat->_occupantEntNum != 1023 )
            Scr_Error("SetSeatOccupied already has an occupant so you can't call set occupied.", 0);
        seat->_occupantEntNum = 1022;
    }
    else if ( seat->_occupantEntNum == 1022 )
    {
        seat->_occupantEntNum = 1023;
    }
    else
    {
        Scr_Error("SetSeatOccupied is either occupied by a player or not set occupied by the script.", 0);
    }
}

void __cdecl CMD_VEH_SeatGetWeapon(scr_entref_t entref)
{
    char *v1; // eax
    unsigned int weapon; // [esp+0h] [ebp-Ch]
    gentity_s *vehicle; // [esp+4h] [ebp-8h]
    int seatIndex; // [esp+8h] [ebp-4h]

    vehicle = GScr_GetVehicle(entref);
    seatIndex = Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
    weapon = VEH_GetSeatWeaponIndex(vehicle, seatIndex);
    if ( weapon )
    {
        v1 = (char *)BG_WeaponName(weapon);
        Scr_AddString(v1, SCRIPTINSTANCE_SERVER);
    }
    else
    {
        Scr_AddUndefined(SCRIPTINSTANCE_SERVER);
    }
}

void __cdecl CMD_VEH_GetSeatFiringOrigin(scr_entref_t entref)
{
    const char *v1; // eax
    float origin[3]; // [esp+0h] [ebp-20h] BYREF
    gentity_s *vehicle; // [esp+Ch] [ebp-14h]
    float angles[3]; // [esp+10h] [ebp-10h] BYREF
    int seatIndex; // [esp+1Ch] [ebp-4h]

    vehicle = GScr_GetVehicle(entref);
    seatIndex = Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
    if ( VEH_GetSeatFiringOriginAngles(vehicle, seatIndex, origin, angles) )
    {
        Scr_AddVector(origin, SCRIPTINSTANCE_SERVER);
    }
    else
    {
        v1 = va("Can't get firing origin for a seat that has no weapon or no flash tag (seat %i).", seatIndex);
        Scr_Error(v1, 0);
    }
}

void __cdecl CMD_VEH_GetSeatFiringAngles(scr_entref_t entref)
{
    const char *v1; // eax
    float origin[3]; // [esp+0h] [ebp-20h] BYREF
    gentity_s *vehicle; // [esp+Ch] [ebp-14h]
    float angles[3]; // [esp+10h] [ebp-10h] BYREF
    int seatIndex; // [esp+1Ch] [ebp-4h]

    vehicle = GScr_GetVehicle(entref);
    seatIndex = Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
    if ( VEH_GetSeatFiringOriginAngles(vehicle, seatIndex, origin, angles) )
    {
        Scr_AddVector(angles, SCRIPTINSTANCE_SERVER);
    }
    else
    {
        v1 = va("Can't get firing angles for a seat that has no weapon or no flash tag (seat %i).", seatIndex);
        Scr_Error(v1, 0);
    }
}

void __cdecl CMD_VEH_IsGunnerFiring(scr_entref_t entref)
{
    gentity_s *vehicle; // [esp+4h] [ebp-8h]
    bool firing; // [esp+Bh] [ebp-1h]

    vehicle = GScr_GetVehicle(entref);
    firing = vehicle->scr_vehicle->gunnerTurrets[Scr_GetInt(0, SCRIPTINSTANCE_SERVER)].fireTime > 0;
    Scr_AddBool(firing, SCRIPTINSTANCE_SERVER);
}

void __cdecl CMD_VEH_DisableGunnerFiring(scr_entref_t entref)
{
    unsigned int v1; // eax
    int gunnerIndex; // [esp+0h] [ebp-Ch]
    gentity_s *vehicle; // [esp+4h] [ebp-8h]

    vehicle = GScr_GetVehicle(entref);
    gunnerIndex = Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
    if ( Scr_GetInt(1u, SCRIPTINSTANCE_SERVER) )
        v1 = vehicle->scr_vehicle->gunnerTurrets[gunnerIndex].flags | 8;
    else
        v1 = vehicle->scr_vehicle->gunnerTurrets[gunnerIndex].flags & 0xFFFFFFF7;
    vehicle->scr_vehicle->gunnerTurrets[gunnerIndex].flags = v1;
}

void __cdecl CMD_VEH_GetTreadHealth(scr_entref_t entref)
{
    unsigned __int16 right_tread; // [esp+2h] [ebp-12h]
    int treadHealth; // [esp+8h] [ebp-Ch] BYREF
    int leftRightIndex; // [esp+Ch] [ebp-8h]
    gentity_s *vehicle; // [esp+10h] [ebp-4h]

    vehicle = GScr_GetVehicle(entref);
    if ( vehicle->destructible
        && ((leftRightIndex = Scr_GetInt(0, SCRIPTINSTANCE_SERVER)) != 0
            ? (right_tread = scr_const.right_tread)
            : (right_tread = scr_const.left_tread),
                GetPieceHealth(vehicle->destructible, right_tread, &treadHealth)) )
    {
        Scr_AddInt(treadHealth, SCRIPTINSTANCE_SERVER);
    }
    else
    {
        Scr_AddInt(vehicle->health, SCRIPTINSTANCE_SERVER);
    }
}

void __cdecl CMD_VEH_finishVehicleDamage(scr_entref_t entref)
{
    unsigned __int16 floatValue; // ax
    float v2; // [esp+0h] [ebp-90h]
    gentity_s *occupant; // [esp+10h] [ebp-80h]
    int seatIndex; // [esp+14h] [ebp-7Ch]
    gentity_s *attacker; // [esp+18h] [ebp-78h]
    int damage; // [esp+1Ch] [ebp-74h]
    int damagea; // [esp+1Ch] [ebp-74h]
    meansOfDeath_t mod; // [esp+20h] [ebp-70h]
    char *weaponName; // [esp+24h] [ebp-6Ch]
    float *dir; // [esp+28h] [ebp-68h]
    float localdir[3]; // [esp+2Ch] [ebp-64h] BYREF
    float vDir[3]; // [esp+38h] [ebp-58h] BYREF
    float vPoint[3]; // [esp+44h] [ebp-4Ch] BYREF
    gentity_s *tempBulletHitEntity; // [esp+50h] [ebp-40h]
    int iWeapon; // [esp+54h] [ebp-3Ch]
    int psTimeOffset; // [esp+58h] [ebp-38h]
    gentity_s *self; // [esp+5Ch] [ebp-34h]
    int dflags; // [esp+60h] [ebp-30h]
    int isBulletWeapon; // [esp+64h] [ebp-2Ch]
    int damageTeamates; // [esp+68h] [ebp-28h]
    int damageFromUnderneath; // [esp+6Ch] [ebp-24h] BYREF
    gentity_s *inflictor; // [esp+70h] [ebp-20h]
    hitLocation_t hitLoc; // [esp+74h] [ebp-1Ch]
    void (__cdecl *die)(gentity_s *, gentity_s *, gentity_s *, int, int, const int, const float *, const hitLocation_t, int); // [esp+78h] [ebp-18h]
    void (__cdecl *pain)(gentity_s *, gentity_s *, int, const float *, const int, const float *, const hitLocation_t, const int); // [esp+7Ch] [ebp-14h]
    int originalDamage; // [esp+80h] [ebp-10h]
    unsigned int modelIndex; // [esp+84h] [ebp-Ch]
    unsigned int partName; // [esp+88h] [ebp-8h]
    const float *point; // [esp+8Ch] [ebp-4h]
    int savedregs; // [esp+90h] [ebp+0h] BYREF

    self = GScr_GetVehicle(entref);
    inflictor = &g_entities[1022];
    attacker = &g_entities[1022];
    dir = 0;
    point = 0;
    isBulletWeapon = 0;
    tempBulletHitEntity = 0;
    damage = Scr_GetInt(2u, SCRIPTINSTANCE_SERVER);
    if ( damage > 0 )
    {
        if ( Scr_GetType(0, SCRIPTINSTANCE_SERVER) && Scr_GetPointerType(0, SCRIPTINSTANCE_SERVER) == 19 )
            inflictor = Scr_GetEntity(0);
        if ( Scr_GetType(1u, SCRIPTINSTANCE_SERVER) && Scr_GetPointerType(1u, SCRIPTINSTANCE_SERVER) == 19 )
            attacker = Scr_GetEntity(1u);
        dflags = Scr_GetInt(3u, SCRIPTINSTANCE_SERVER);
        mod = (meansOfDeath_t)G_MeansOfDeathFromScriptParam(4u);
        weaponName = Scr_GetString(5u, SCRIPTINSTANCE_SERVER);
        iWeapon = G_GetWeaponIndexForName(weaponName);
        if ( Scr_GetType(6u, SCRIPTINSTANCE_SERVER) )
        {
            Scr_GetVector(6u, vPoint, SCRIPTINSTANCE_SERVER);
            point = vPoint;
        }
        if ( Scr_GetType(7u, SCRIPTINSTANCE_SERVER) )
        {
            Scr_GetVector(7u, vDir, SCRIPTINSTANCE_SERVER);
            dir = vDir;
        }
        floatValue = (unsigned __int16)Scr_GetConstString(8u, SCRIPTINSTANCE_SERVER);
        hitLoc = (hitLocation_t)G_GetHitLocationIndexFromString(floatValue);
        psTimeOffset = Scr_GetInt(9u, SCRIPTINSTANCE_SERVER);
        damageFromUnderneath = Scr_GetInt(0xAu, SCRIPTINSTANCE_SERVER);
        modelIndex = Scr_GetInt(0xBu, SCRIPTINSTANCE_SERVER);
        partName = Scr_GetInt(0xCu, SCRIPTINSTANCE_SERVER);
        damageTeamates = Scr_GetInt(0xDu, SCRIPTINSTANCE_SERVER);
        if ( dir )
            Vec3NormalizeTo(dir, localdir);
        else
            memset(localdir, 0, sizeof(localdir));
        originalDamage = damage;
        v2 = (float)damage;
        damagea = (int)(Scr_Vehicle_DamageScale(
                                            self,
                                            attacker,
                                            inflictor,
                                            point,
                                            mod,
                                            iWeapon,
                                            &damageFromUnderneath)
                                    * v2);
        if ( damagea && (dflags & 1) == 0 && self->destructible )
            damagea = DestructibleDamage(self, attacker, dir, point, damagea, mod, modelIndex, partName);
        if ( self->health > 0 )
        {
            if ( g_debugDamage->current.enabled )
                Com_Printf(15, "target:%i health:%i damage:%i\n", self->s.number, self->health, damagea);
            self->health -= damagea;
            if ( damageTeamates && self->health <= 0 )
            {
                for ( seatIndex = 0; seatIndex < 11; ++seatIndex )
                {
                    occupant = VEH_GetSeatOccupantEntity(self->scr_vehicle, seatIndex);
                    if ( occupant != attacker )
                    {
                        if ( occupant )
                            VEH_UnlinkPlayer(occupant, 0, (char*)"Ejecting teammates");
                    }
                }
            }
            if ( mod == MOD_PISTOL_BULLET || mod == MOD_RIFLE_BULLET )
                isBulletWeapon = 1;
            G_DamageNotify(
                scr_const.damage,
                self,
                attacker,
                dir,
                (float *)point,
                damagea,
                mod,
                dflags,
                modelIndex,
                partName,
                weaponName);
            if ( self->health > 0 )
            {
                pain = entityHandlers[self->handler].pain;
                if ( pain )
                    pain(self, attacker, originalDamage, point, mod, localdir, hitLoc, iWeapon);
            }
            else
            {
                if ( self->health < -999 )
                    self->health = -999;
                Scr_AddString(weaponName, SCRIPTINSTANCE_SERVER);
                Scr_AddBool(damageFromUnderneath, SCRIPTINSTANCE_SERVER);
                Scr_AddEntity(attacker, SCRIPTINSTANCE_SERVER);
                Scr_Notify(self, scr_const.death, 3u);
                die = entityHandlers[self->handler].die;
                if ( die )
                    die(self, inflictor, attacker, damagea, mod, iWeapon, localdir, hitLoc, psTimeOffset);
            }
        }
    }
}

void __cdecl CMD_VEH_finishVehicleRadiusDamage(scr_entref_t entref)
{
    gentity_s *attacker; // [esp+24h] [ebp-68h]
    int damage; // [esp+28h] [ebp-64h]
    meansOfDeath_t mod; // [esp+2Ch] [ebp-60h]
    char *weaponName; // [esp+30h] [ebp-5Ch]
    float *coneDirection; // [esp+34h] [ebp-58h]
    float localdir[3]; // [esp+38h] [ebp-54h] BYREF
    float vDir[3]; // [esp+44h] [ebp-48h] BYREF
    float vPoint[3]; // [esp+50h] [ebp-3Ch] BYREF
    gentity_s *tempBulletHitEntity; // [esp+5Ch] [ebp-30h]
    float radius; // [esp+60h] [ebp-2Ch]
    int iWeapon; // [esp+64h] [ebp-28h]
    int psTimeOffset; // [esp+68h] [ebp-24h]
    float fOuterDamage; // [esp+6Ch] [ebp-20h]
    gentity_s *self; // [esp+70h] [ebp-1Ch]
    int dflags; // [esp+74h] [ebp-18h]
    float fConeAngleCos; // [esp+78h] [ebp-14h]
    float damageScale; // [esp+7Ch] [ebp-10h]
    gentity_s *inflictor; // [esp+80h] [ebp-Ch]
    const float *point; // [esp+84h] [ebp-8h]
    float fInnerDamage; // [esp+88h] [ebp-4h]

    self = GScr_GetVehicle(entref);
    inflictor = &g_entities[1022];
    attacker = &g_entities[1022];
    coneDirection = 0;
    point = 0;
    tempBulletHitEntity = 0;
    damage = Scr_GetInt(2u, SCRIPTINSTANCE_SERVER);
    if ( damage > 0 )
    {
        if ( Scr_GetType(0, SCRIPTINSTANCE_SERVER) && Scr_GetPointerType(0, SCRIPTINSTANCE_SERVER) == 19 )
            inflictor = Scr_GetEntity(0);
        if ( Scr_GetType(1u, SCRIPTINSTANCE_SERVER) && Scr_GetPointerType(1u, SCRIPTINSTANCE_SERVER) == 19 )
            attacker = Scr_GetEntity(1u);
        fInnerDamage = Scr_GetFloat(3u, SCRIPTINSTANCE_SERVER);
        fOuterDamage = Scr_GetFloat(4u, SCRIPTINSTANCE_SERVER);
        dflags = Scr_GetInt(5u, SCRIPTINSTANCE_SERVER);
        mod = (meansOfDeath_t)G_MeansOfDeathFromScriptParam(6u);
        weaponName = Scr_GetString(7u, SCRIPTINSTANCE_SERVER);
        iWeapon = G_GetWeaponIndexForName(weaponName);
        if ( Scr_GetType(8u, SCRIPTINSTANCE_SERVER) )
        {
            Scr_GetVector(8u, vPoint, SCRIPTINSTANCE_SERVER);
            point = vPoint;
        }
        radius = Scr_GetFloat(9u, SCRIPTINSTANCE_SERVER);
        fConeAngleCos = Scr_GetFloat(0xAu, SCRIPTINSTANCE_SERVER);
        if ( Scr_GetType(0xBu, SCRIPTINSTANCE_SERVER) )
        {
            Scr_GetVector(0xBu, vDir, SCRIPTINSTANCE_SERVER);
            Vec3Normalize(vDir);
            coneDirection = vDir;
        }
        psTimeOffset = Scr_GetInt(0xCu, SCRIPTINSTANCE_SERVER);
        damageScale = CanDamage(self, inflictor, vPoint, fConeAngleCos, coneDirection, 8396819);
        if ( damageScale > 0.0 )
        {
            localdir[0] = self->r.currentOrigin[0] - vPoint[0];
            localdir[1] = self->r.currentOrigin[1] - vPoint[1];
            localdir[2] = self->r.currentOrigin[2] - vPoint[2];
            if ( self->destructible )
                damage = (int)DestructibleRadiusDamage(self, vPoint, fInnerDamage, fOuterDamage, radius, mod, attacker);
            G_Damage(
                self,
                inflictor,
                attacker,
                localdir,
                vPoint,
                (int)(float)((float)damage * damageScale),
                dflags,
                mod,
                iWeapon,
                HITLOC_NONE,
                0,
                0,
                0);
        }
    }
}

void __cdecl CMD_VEH_IsVehicleImmuneToDamage(scr_entref_t entref)
{
    char *String; // eax
    meansOfDeath_t mod; // [esp+0h] [ebp-10h]
    unsigned int iWeapon; // [esp+4h] [ebp-Ch]
    gentity_s *vehicle; // [esp+8h] [ebp-8h]
    char dflags; // [esp+Ch] [ebp-4h]

    vehicle = GScr_GetVehicle(entref);
    dflags = (unsigned __int8)Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
    mod = (meansOfDeath_t)G_MeansOfDeathFromScriptParam(1u);
    String = Scr_GetString(2u, SCRIPTINSTANCE_SERVER);
    iWeapon = G_GetWeaponIndexForName(String);
    if ( G_IsVehicleImmune(vehicle, mod, dflags, iWeapon) )
        Scr_AddBool(1u, SCRIPTINSTANCE_SERVER);
    else
        Scr_AddBool(0, SCRIPTINSTANCE_SERVER);
}

void __cdecl CMD_VEH_IsBoosting(scr_entref_t entref)
{
    bool isBoosting; // [esp+7h] [ebp-9h]
    scr_vehicle_s *veh; // [esp+8h] [ebp-8h]

    isBoosting = 0;
    veh = GScr_GetVehicle(entref)->scr_vehicle;
    if ( veh )
        isBoosting = veh->nitrousVehicle != 0;
    Scr_AddBool(isBoosting, SCRIPTINSTANCE_SERVER);
}

void __cdecl CMD_VEH_GetBoost(scr_entref_t entref)
{
    NitrousVehicle *nitVeh; // [esp+8h] [ebp-10h]
    scr_vehicle_s *veh; // [esp+Ch] [ebp-Ch]
    float boost; // [esp+10h] [ebp-8h]
    gentity_s *self; // [esp+14h] [ebp-4h]

    boost = 0.0f;
    self = GScr_GetVehicle(entref);
    if ( self )
    {
        veh = self->scr_vehicle;
        if ( veh )
        {
            nitVeh = veh->nitrousVehicle;
            if ( nitVeh )
                boost = nitVeh->m_boost_time_pool / nitVeh->m_vehicle_info->boostDuration;
        }
    }
    Scr_AddFloat(boost, SCRIPTINSTANCE_SERVER);
}

void __cdecl G_VehSetClientSideHealthPercentageEntity(gentity_s *const ent, float percentage)
{
    VehSetClientSideHealth(ent, percentage, 63, 0);
}

void __cdecl VehSetClientSideHealth(gentity_s *const ent, float percentage, int mask, char shift)
{
    float v4; // [esp+0h] [ebp-Ch]
    float v5; // [esp+4h] [ebp-8h]

    if ( (float)(percentage - 1.0) < 0.0 )
        v5 = percentage;
    else
        v5 = 1.0f;
    if ( (float)(0.0 - percentage) < 0.0 )
        v4 = v5;
    else
        v4 = 0.0f;
    VehSetClientSideTime2Val(ent, (int)(float)(v4 * 63.0), mask, shift);
}

void __cdecl VehSetClientSideTime2Val(gentity_s *const ent, int val, int mask, char shift)
{
    ent->s.time2 = mask & (val << shift) | ent->s.time2 & ~mask;
}

void __cdecl G_VehSetClientSideHealthPercentageLeftTread(gentity_s *const ent, float percentage)
{
    VehSetClientSideHealth(ent, percentage, 4032, 6);
}

void __cdecl G_VehSetClientSideHealthPercentageRightTread(gentity_s *const ent, float percentage)
{
    VehSetClientSideHealth(ent, percentage, 258048, 12);
}

void __cdecl G_VehSetClientSideSeatOccupancyFlags(gentity_s *const ent, int occupancy_flags)
{
    VehSetClientSideTime2Val(ent, occupancy_flags, 786432, 18);
}

const int yawturn_shifts[2] = { 20, 22 };
const int yawturn_masks_0[2] = { 1048576, 4194304 };
const int pitchturn_shifts[2] = { 21, 23 };
const int pitchturn_masks_0[2] = { 2097152, 8388608 };
const int overheating_shifts_0[2] = { 24, 25 };
const int overheating_masks_0[2] = { 16777216, 33554432 };

void __cdecl G_VehSetClientSideGunTurningYaw(gentity_s *const ent, unsigned int seatIndex, bool turning)
{
    if ( seatIndex >= 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                    13024,
                    0,
                    "%s",
                    "seatIndex >= 0 && seatIndex < ( sizeof( _yawturn_shifts ) / sizeof ( _yawturn_shifts[ 0 ] ) )") )
    {
        __debugbreak();
    }
    VehSetClientSideGunTurnRate(ent, (float)turning, yawturn_masks_0[seatIndex], yawturn_shifts[seatIndex]);
}

void __cdecl VehSetClientSideGunTurnRate(gentity_s *const ent, float percentage, int mask, char shift)
{
    if ( percentage == 0.0 )
        VehSetClientSideTime2Val(ent, 0, mask, shift);
    else
        VehSetClientSideTime2Val(ent, 1, mask, shift);
}

void __cdecl G_VehSetClientSideGunTurningPitch(gentity_s *const ent, unsigned int seatIndex, bool turning)
{
    if ( seatIndex >= 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                    13030,
                    0,
                    "%s",
                    "seatIndex >= 0 && seatIndex < ( sizeof( _pitchturn_shifts ) / sizeof ( _pitchturn_shifts[ 0 ] ) )") )
    {
        __debugbreak();
    }
    VehSetClientSideGunTurnRate(ent, (float)turning, pitchturn_masks_0[seatIndex], pitchturn_shifts[seatIndex]);
}

void __cdecl G_VehSetClientSideGunOverheating(gentity_s *const ent, unsigned int seatIndex, bool overheating)
{
    if ( seatIndex >= 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_scr_vehicle.cpp",
                    13036,
                    0,
                    "%s",
                    "seatIndex >= 0 && seatIndex < ( sizeof( _overheating_shifts ) / sizeof ( _overheating_shifts[ 0 ] ) )") )
    {
        __debugbreak();
    }
    VehSetClientSideTime2Val(ent, overheating, overheating_masks_0[seatIndex], overheating_shifts_0[seatIndex]);
}

