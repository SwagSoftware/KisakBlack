#include "cg_vehicle.h"
#include <client/splitscreen.h>
#include <universal/com_math_anglevectors.h>
#include <gfx_d3d/r_water_sim.h>
#include <bgame/bg_weapons.h>
#include <clientscript/cscr_memorytree.h>
#include <cgame_mp/cg_local_mp.h>
#include <xanim/dobj_utils.h>
#include <xanim/xmodel_utils.h>
#include <clientscript/scr_const.h>
#include <cgame_mp/cg_vehicles_mp.h>
#include <universal/surfaceflags.h>
#include <sound/snd_bank.h>
#include "cg_sound.h"
#include <clientscript/cscr_vm.h>
#include "cg_scr_main.h"
#include "cg_event.h"
#include <bgame/bg_slidemove.h>
#include <clientscript/cscr_stringlist.h>
#include <gfx_d3d/r_foliage.h>
#include <sound/snd_public_async.h>
#include <sound/snd_utils.h>
#include <qcommon/dobj_management.h>

const int seat_positions[2] = { 0, 1 };
const int yawturn_masks[2] = { 1048576, 4194304 };
const int pitchturn_masks[2] = { 2097152, 8388608 };
const int overheating_shifts[2] = { 24, 25 };
const int overheating_masks[2] = { 16777216, 33554432 };
const int overheat_flags[2] = { 4, 8 };


// *WARNING* One or more selections were skipped as they could not be interpreted as c data

float foam = 1.9;
float foamLerpRate = 0.5;
float foamRadius = 30.0;
float depth = -2.0;
float lerpRate_0 = 0.1;
float radius = 30.0;
float dist1 = 200.0;
float depth2 = 2.0;
float lerpRate2 = 0.18000001;
float radius2 = 30.0;
float dist2 = -200.0;

float s_minVelocityForTireSound = 0.025;
float recoilDist = 24.0;
float recoilBackPow = 0.25;
int recoilBackTime = 250;
int recoilDuration = 1500;
float aScrollSpeed = 300.0;
int fadetime = 500;
bool playTreadClacks = true;
bool playTreadSqueaks = true;
bool playBodyDmgLoop = true;
bool playFireSounds = true;
bool playOverheatSounds = true;
bool playTurretRotationSounds = true;

const int aaWheels_0[2][3] =
{ { 0, 4, 2 }, { 1, 5, 3 } };


const int s_kaiSurfaceGroups[31] =
{
  0,
  0,
  1,
  0,
  0,
  1,
  0,
  0,
  0,
  1,
  0,
  1,
  1,
  1,
  0,
  0,
  1,
  1,
  0,
  0,
  0,
  1,
  1,
  1,
  1,
  0,
  0,
  0,
  1,
  0,
  0
};


const int aaWheels[2][2] = { { 0, 2 }, { 1, 3 } };

unsigned __int16 *g_wheelTags[6] =
{
    &scr_const.tag_wheel_front_left,
    &scr_const.tag_wheel_front_right,
    &scr_const.tag_wheel_back_left,
    &scr_const.tag_wheel_back_right,
    &scr_const.tag_wheel_middle_left,
    &scr_const.tag_wheel_middle_right
};

unsigned __int16 *g_gunnerBarrelTags[4] =
{
    &scr_const.tag_gunner_barrel1,
    &scr_const.tag_gunner_barrel2,
    &scr_const.tag_gunner_barrel3,
    &scr_const.tag_gunner_barrel4
};

unsigned __int16 *g_gunnerTurretTags[4] =
{
    &scr_const.tag_gunner_turret1,
    &scr_const.tag_gunner_turret2,
    &scr_const.tag_gunner_turret3,
    &scr_const.tag_gunner_turret4,
};



void __cdecl CG_UpdateVehicleWaterFX(int localClientNum, centity_s *cent)
{
    float axis[3][3]; // [esp+1Ch] [ebp-30h] BYREF
    float point[3]; // [esp+40h] [ebp-Ch] BYREF

    if ( CL_LocalClient_IsFirstActive(localClientNum) )
    {
        AnglesToAxis(cent->pose.angles, axis);
        point[0] = (float)(dist1 * axis[0][0]) + cent->pose.origin[0];
        point[1] = (float)(dist1 * axis[0][1]) + cent->pose.origin[1];
        point[2] = (float)(dist1 * axis[0][2]) + cent->pose.origin[2];
        R_WaterSimulationSetHeight(cent->pose.origin, radius, depth, lerpRate_0);
        point[0] = (float)(dist2 * axis[0][0]) + cent->pose.origin[0];
        point[1] = (float)(dist2 * axis[0][1]) + cent->pose.origin[1];
        point[2] = (float)(dist2 * axis[0][2]) + cent->pose.origin[2];
        R_WaterSimulationSetHeight(point, radius2, depth2, lerpRate2);
    }
}


// local variable allocation has failed, the output may be wrong!
void    CG_Vehicle_PreControllers(
                int localClientNum,
                const DObj *obj,
                centity_s *cent)
{
    const XModel *Model; // eax
    float v6; // xmm0_4
    long double v7; // [esp+8h] [ebp-5BCh]
    long double v8; // [esp+10h] [ebp-5B4h]
    float *v9; // [esp+1Ch] [ebp-5A8h]
    float *v10; // [esp+24h] [ebp-5A0h]
    col_context_t context; // [esp+2Ch] [ebp-598h] BYREF
    float expand_vec[3]; // [esp+54h] [ebp-570h] BYREF
    int contents_mask; // [esp+60h] [ebp-564h]
    float v14; // [esp+64h] [ebp-560h]
    float *v15; // [esp+68h] [ebp-55Ch]
    float *v16; // [esp+6Ch] [ebp-558h]
    float *v17; // [esp+70h] [ebp-554h]
    float *v18; // [esp+74h] [ebp-550h]
    float trace_points[6][2][3]; // [esp+78h] [ebp-54Ch] BYREF
    float wheelPos[3]; // [esp+108h] [ebp-4BCh] BYREF
    const DObjAnimMat *mtx; // [esp+114h] [ebp-4B0h]
    unsigned int boneIndex; // [esp+118h] [ebp-4ACh]
    int presence_mask; // [esp+11Ch] [ebp-4A8h]
    float mx[3]; // [esp+120h] [ebp-4A4h] BYREF
    float mn[3]; // [esp+12Ch] [ebp-498h] BYREF
    int bitmask; // [esp+138h] [ebp-48Ch]
    vehicle_cache_t *vehicle_cache; // [esp+13Ch] [ebp-488h]
    float angles_vec3[3]; // [esp+140h] [ebp-484h] BYREF
    float axis_loc_vec3[3]; // [esp+14Ch] [ebp-478h] BYREF
    phys_vec3 axis_loc; // [esp+158h] [ebp-46Ch] BYREF
    float rotation; // [esp+174h] [ebp-450h]
    float v32; // [esp+178h] [ebp-44Ch]
    float v33; // [esp+17Ch] [ebp-448h]
    float m_susp_min_height; // [esp+180h] [ebp-444h]
    float lerp; // [esp+184h] [ebp-440h]
    float v36; // [esp+188h] [ebp-43Ch]
    float v37; // [esp+18Ch] [ebp-438h]
    float v38; // [esp+190h] [ebp-434h]
    float lerpRate; // [esp+194h] [ebp-430h]
    const VehicleParameter *v40; // [esp+198h] [ebp-42Ch]
    float v41; // [esp+19Ch] [ebp-428h]
    float v42; // [esp+1A0h] [ebp-424h]
    rigid_body_constraint_wheel *v43; // [esp+1A4h] [ebp-420h]
    const VehicleParameter *v44; // [esp+1A8h] [ebp-41Ch]
    float v45; // [esp+1ACh] [ebp-418h]
    float v46; // [esp+1B0h] [ebp-414h]
    rigid_body_constraint_wheel *wheel2; // [esp+1B4h] [ebp-410h]
    float newWheelHeight; // [esp+1B8h] [ebp-40Ch]
    const VehicleParameter *v49; // [esp+1BCh] [ebp-408h]
    float m_wheel_displaced_center_dist; // [esp+1C0h] [ebp-404h]
    float wheelRadiusDelta; // [esp+1C4h] [ebp-400h]
    float m_wheel_radius; // [esp+1C8h] [ebp-3FCh]
    const VehicleParameter *m_parameter; // [esp+1CCh] [ebp-3F8h]
    rigid_body_constraint_wheel *wheel; // [esp+1D0h] [ebp-3F4h]
    int j; // [esp+1D4h] [ebp-3F0h]
    //float boneMtxList[3]; // [esp+1D8h] [ebp-3ECh]
    const DObjAnimMat *pose;
    float axis[4][3]; // [esp+1E4h] [ebp-3E0h] BYREF
    float suspTravel; // [esp+214h] [ebp-3B0h]
    float v59; // [esp+218h] [ebp-3ACh]
    float weaponSpin; // [esp+21Ch] [ebp-3A8h]
    const WeaponDef *turretWeapon; // [esp+220h] [ebp-3A4h]
    int v62; // [esp+224h] [ebp-3A0h]
    const clientInfo_t *ci; // [esp+228h] [ebp-39Ch]
    int clientNum; // [esp+22Ch] [ebp-398h]
    float v65; // [esp+230h] [ebp-394h]
    float v66; // [esp+234h] [ebp-390h]
    float v67; // [esp+238h] [ebp-38Ch]
    float v68; // [esp+23Ch] [ebp-388h]
    float v69; // [esp+240h] [ebp-384h]
    float v70; // [esp+244h] [ebp-380h]
    float v71; // [esp+248h] [ebp-37Ch]
    float v72; // [esp+24Ch] [ebp-378h]
    float v73; // [esp+250h] [ebp-374h]
    float v74; // [esp+254h] [ebp-370h]
    float v75; // [esp+258h] [ebp-36Ch]
    float *v76; // [esp+25Ch] [ebp-368h]
    float v77[2]; // [esp+260h] [ebp-364h] BYREF
    float fc; // [esp+268h] [ebp-35Ch] BYREF
    float v79; // [esp+26Ch] [ebp-358h]
    float v80; // [esp+270h] [ebp-354h]
    float f; // [esp+274h] [ebp-350h]
    float v82; // [esp+278h] [ebp-34Ch]
    float v83; // [esp+27Ch] [ebp-348h]
    float v84; // [esp+280h] [ebp-344h]
    float v85; // [esp+284h] [ebp-340h]
    float v86; // [esp+288h] [ebp-33Ch]
    float v87; // [esp+28Ch] [ebp-338h]
    float v88; // [esp+290h] [ebp-334h]
    float v89; // [esp+294h] [ebp-330h]
    float v90; // [esp+298h] [ebp-32Ch]
    float v91; // [esp+29Ch] [ebp-328h]
    float v92; // [esp+2A0h] [ebp-324h]
    __int16 v93; // [esp+2A6h] [ebp-31Eh]
    float v94; // [esp+2A8h] [ebp-31Ch]
    float v95; // [esp+2ACh] [ebp-318h]
    float v96; // [esp+2B0h] [ebp-314h]
    float v97; // [esp+2B4h] [ebp-310h]
    float v98; // [esp+2B8h] [ebp-30Ch]
    float v99; // [esp+2BCh] [ebp-308h]
    float v100; // [esp+2C0h] [ebp-304h]
    float v101; // [esp+2C4h] [ebp-300h]
    float v102; // [esp+2C8h] [ebp-2FCh]
    float v103; // [esp+2CCh] [ebp-2F8h]
    float v104; // [esp+2D0h] [ebp-2F4h]
    float v105; // [esp+2D4h] [ebp-2F0h]
    float v106; // [esp+2D8h] [ebp-2ECh]
    float v107; // [esp+2DCh] [ebp-2E8h]
    float v108; // [esp+2E0h] [ebp-2E4h]
    float v109; // [esp+2E4h] [ebp-2E0h]
    float v110; // [esp+2E8h] [ebp-2DCh]
    float v111; // [esp+2ECh] [ebp-2D8h]
    float v112; // [esp+2F0h] [ebp-2D4h]
    const vehicle_info_t *vehicleInfo; // [esp+2F4h] [ebp-2D0h]
    //float infoIdx; // [esp+2F8h] [ebp-2CCh]
    float v115; // [esp+2FCh] [ebp-2C8h]
    float v116; // [esp+300h] [ebp-2C4h]
    float v117; // [esp+304h] [ebp-2C0h]
    float v118; // [esp+308h] [ebp-2BCh]
    float v119; // [esp+30Ch] [ebp-2B8h]
    float v120; // [esp+310h] [ebp-2B4h]
    float v121; // [esp+314h] [ebp-2B0h]
    float v122; // [esp+318h] [ebp-2ACh]
    float v123; // [esp+31Ch] [ebp-2A8h]
    float v124; // [esp+320h] [ebp-2A4h]
    float v125; // [esp+324h] [ebp-2A0h]
    float v126; // [esp+328h] [ebp-29Ch]
    float v127; // [esp+32Ch] [ebp-298h]
    float v128; // [esp+330h] [ebp-294h]
    int v129; // [esp+334h] [ebp-290h]
    float newViewAngles[3]; // [esp+338h] [ebp-28Ch] BYREF
    float newViewAxis[3][3]; // [esp+344h] [ebp-280h] BYREF
    float yawAxis[3][3]; // [esp+368h] [ebp-25Ch] BYREF
    unsigned int yawAngles[5]; // [esp+38Ch] [ebp-238h] BYREF
    float vehAxis[3][3]; // [esp+3A0h] [ebp-224h] BYREF
    float vehYaw; // [esp+3C4h] [ebp-200h]
    float vehAngles[4]; // [esp+3C8h] [ebp-1FCh] BYREF
    float v137; // [esp+3D8h] [ebp-1ECh]
    float ofsPitch; // [esp+3DCh] [ebp-1E8h]
    float localYaw; // [esp+3E0h] [ebp-1E4h]
    float v140; // [esp+3E4h] [ebp-1E0h]
    float absYaw; // [esp+3E8h] [ebp-1DCh]
    float frameInterpolation; // [esp+3ECh] [ebp-1D8h]
    float v143; // [esp+3F0h] [ebp-1D4h]
    float v144; // [esp+3F4h] [ebp-1D0h]
    float v145; // [esp+3F8h] [ebp-1CCh]
    float v146; // [esp+3FCh] [ebp-1C8h]
    float v147; // [esp+400h] [ebp-1C4h]
    float v148; // [esp+404h] [ebp-1C0h]
    float v149; // [esp+408h] [ebp-1BCh]
    float v150; // [esp+40Ch] [ebp-1B8h]
    float turretAnglesAxis[3][3]; // [esp+410h] [ebp-1B4h] BYREF
    float turretAngles[4]; // [esp+434h] [ebp-190h] BYREF
    float invMat[3][3]; // [esp+444h] [ebp-180h] BYREF
    float baseAnglesAxis[3][3]; // [esp+468h] [ebp-15Ch] BYREF
    float baseAngles[4]; // [esp+48Ch] [ebp-138h] BYREF
    float v156; // [esp+49Ch] [ebp-128h]
    float v157; // [esp+4A0h] [ebp-124h]
    float v158; // [esp+4A4h] [ebp-120h]
    float v159; // [esp+4A8h] [ebp-11Ch]
    float v160; // [esp+4ACh] [ebp-118h]
    float v161; // [esp+4B0h] [ebp-114h]
    float v162; // [esp+4B4h] [ebp-110h]
    float v163; // [esp+4B8h] [ebp-10Ch]
    float tagAngles[3]; // [esp+4BCh] [ebp-108h] BYREF
    float tagAxis[4][3]; // [esp+4C8h] [ebp-FCh] BYREF
    DObjAnimMat animMat; // [esp+4F8h] [ebp-CCh] BYREF
    unsigned char index; // [esp+518h] [ebp-ACh] BYREF
    int gunnerIndex; // [esp+51Ch] [ebp-A8h]
    DObj *obja; // [esp+520h] [ebp-A4h]
    unsigned __int16 *gunnerViewTags[4]; // [esp+524h] [ebp-A0h]
    float *lastAngles; // [esp+534h] [ebp-90h]
    vehicle_cache_t *v172; // [esp+538h] [ebp-8Ch]
    int i; // [esp+53Ch] [ebp-88h]
    colgeom_visitor_inlined_t<200> *p_proximity_data; // [esp+540h] [ebp-84h]
    //colgeom_visitor_inlined_t<200>_vtbl *v175; // [esp+544h] [ebp-80h]
    int numWheels; // [esp+548h] [ebp-7Ch]
    const vehicle_info_t *info; // [esp+54Ch] [ebp-78h]
    float speed[2]; // [esp+550h] [ebp-74h]
    float forward[3]; // [esp+558h] [ebp-6Ch] BYREF
    const playerState_s *ps; // [esp+564h] [ebp-60h]
    const entityState_s *ns; // [esp+568h] [ebp-5Ch]
    const LerpEntityState *p_currentState; // [esp+56Ch] [ebp-58h]
    cg_s *cgameGlob; // [esp+570h] [ebp-54h]
    bool barrelPitchAlreadySet; // [esp+577h] [ebp-4Dh]
    int ignoreGunner; // [esp+578h] [ebp-4Ch]
    trace_t trace; // [esp+57Ch] [ebp-48h] BYREF
    //_UNKNOWN *v187[2]; // [esp+5B8h] [ebp-Ch] BYREF
    //centity_s *centa; // [esp+5C4h] [ebp+0h]
    //
    //v187[0] = a1;
    //v187[1] = centa;
    //LODWORD(v7) = a2;
    if (!obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_vehicle.cpp", 339, 0, "%s", "obj"))
        __debugbreak();
    ignoreGunner = -1;
    barrelPitchAlreadySet = 0;
    if (!cent->vehicle)
    {
        cent->vehicle = (cgVehicle_s *)MT_Alloc(84, 21, SCRIPTINSTANCE_SERVER);
        if (!cent->vehicle
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_vehicle.cpp", 347, 0, "%s", "cent->vehicle"))
        {
            __debugbreak();
        }
        memset((unsigned __int8 *)cent->vehicle, 0, sizeof(cgVehicle_s));
    }
    cgameGlob = CG_GetLocalClientGlobals((int)localClientNum);
    p_currentState = &cent->currentState;
    ns = &cent->nextState;
    ps = &cgameGlob->predictedPlayerState;
    AngleVectors(cent->pose.angles, forward, 0, 0);
    //LODWORD(speed[1]) = ns->lerp.pos.trDelta;
    speed[0] = (float)((float)(ns->lerp.pos.trDelta[0] * forward[0]) + (float)(ns->lerp.pos.trDelta[1] * forward[1])) + (float)(ns->lerp.pos.trDelta[2] * forward[2]);
    if (cent->nitrousVeh)
        speed[0] = cent->nitrousVeh->m_forward_vel;
    VEH_UpdateMaterialTime(cent, cgameGlob->frametime, speed[0]);
    info = CG_GetVehicleInfo(cent->nextState.vehicleState.vehicleInfoIndex);
    numWheels = 0;
    if (info->type == 1)
    {
        numWheels = 2;
    }
    else if (info->type)
    {
        if (info->type == 2)
            numWheels = 6;
    }
    else
    {
        numWheels = 4;
    }
    if (!cent->vehicle->vehicle_cache && numWheels > 0)
    {
        cent->vehicle->vehicle_cache = (vehicle_cache_t *)MT_Alloc(1928, 21, SCRIPTINSTANCE_SERVER);
        if (!cent->vehicle->vehicle_cache
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_vehicle.cpp",
                382,
                0,
                "%s",
                "cent->vehicle->vehicle_cache"))
        {
            __debugbreak();
        }

        static colgeom_visitor_inlined_t<200> dummy;

        //if ((_S1_2 & 1) == 0)
        //{
        //    _S1_2 |= 1u;
        //    colgeom_visitor_t::colgeom_visitor_t(&dummy);
        //    dummy.__vftable = (colgeom_visitor_inlined_t<200>_vtbl *) & colgeom_visitor_inlined_t<200>::`vftable';
        //        colgeom_visitor_inlined_t<500>::reset(&dummy);
        //    atexit(CG_Vehicle_PreControllers_::_19_::_dynamic_atexit_destructor_for__dummy__);
        //}
        //v175 = dummy.__vftable;
        //p_proximity_data = &cent->vehicle->vehicle_cache->proximity_data;
        //p_proximity_data->__vftable = dummy.__vftable;

        //colgeom_visitor_inlined_t<500>::reset(&cent->vehicle->vehicle_cache->proximity_data);
        cent->vehicle->vehicle_cache->proximity_data.reset();

        for (i = 0; i < 6; ++i)
        {
            cent->vehicle->vehicle_cache->hit_indices[i] = -1;
            v172 = cent->vehicle->vehicle_cache;
            v172->lastOrigin[0] = FLT_MAX;
            v172->lastOrigin[1] = FLT_MAX;
            v172->lastOrigin[2] = FLT_MAX;
            lastAngles = cent->vehicle->vehicle_cache->lastAngles;
            *lastAngles = FLT_MAX;
            lastAngles[1] = FLT_MAX;
            lastAngles[2] = FLT_MAX;
        }
    }
    if ((ps->eFlags & 0x4000) != 0 && ps->viewlocked_entNum == cent->nextState.number)
    {
        if (ps->vehiclePos >= 1 && ps->vehiclePos <= 4 && gVehicleRelativeGunnerAngles)
        {
            gunnerViewTags[0] = &scr_const.tag_gunner_barrel1;
            gunnerViewTags[1] = &scr_const.tag_gunner_barrel2;
            gunnerViewTags[2] = &scr_const.tag_gunner_barrel3;
            gunnerViewTags[3] = &scr_const.tag_gunner_barrel4;
            obja = Com_GetClientDObj(cent->nextState.number, (int)localClientNum);
            if (obja)
            {
                gunnerIndex = ps->vehiclePos - 1;
                index = 0;
                DObjGetBoneIndex(obja, *gunnerViewTags[gunnerIndex], &index, -1);
                DObjGetBasePoseMatrix(obja, 0, &animMat);
                QuatToAxis(animMat.quat, tagAxis);
                AxisToAngles(tagAxis, tagAngles);
                v163 = ps->viewangles[0] + tagAngles[0];
                v162 = floor(v163 * 182.04445 + 0.5);
                cent->currentState.u.vehicle.gunnerAngles[gunnerIndex].pitch = (int)v162;
                v161 = ps->viewangles[1] + tagAngles[1];
                v160 = floor(v161 * 182.04445 + 0.5);
                cent->currentState.u.vehicle.gunnerAngles[gunnerIndex].yaw = (int)v160;
                v159 = ps->viewangles[0] + tagAngles[0];
                v158 = floor(v159 * 182.04445 + 0.5);
                cent->nextState.lerp.u.vehicle.gunnerAngles[gunnerIndex].pitch = (int)v158;
                v157 = ps->viewangles[1] + tagAngles[1];
                v156 = floor(v157 * 182.04445 + 0.5);
                cent->nextState.lerp.u.vehicle.gunnerAngles[gunnerIndex].yaw = (int)v156;
            }
        }
        else if (!vehLockTurretToPlayerView->current.enabled || ps->vehiclePos)
        {
            v146 = (float)ns->lerp.u.vehicle.gunYaw * 0.0054931641;
            v145 = (float)p_currentState->u.vehicle.gunYaw * 0.0054931641;
            v144 = v145 + p_currentState->apos.trBase[1];
            v143 = v146 + ns->lerp.apos.trBase[1];
            frameInterpolation = cgameGlob->frameInterpolation;
            absYaw = AngleNormalize180(v143 - v144) * frameInterpolation + v144;
            v140 = cent->pose.angles[1];
            localYaw = AngleNormalize180(absYaw - v140);
            ofsPitch = 0.0f;
            v137 = floor(localYaw * 182.04445 + 0.5);
            cent->pose.vehicle.yaw = (int)v137;
            if (ps->vehiclePos >= 1 && ps->vehiclePos <= 4)
            {
                //LODWORD(vehAngles[3]) = cent->pose.angles;
                vehAngles[0] = cent->pose.angles[0];
                vehAngles[1] = cent->pose.angles[1];
                vehAngles[2] = cent->pose.angles[2];
                vehYaw = vehAngles[1];
                vehAngles[1] = 0.0f;
                AnglesToAxis(vehAngles, vehAxis);
                yawAngles[4] = LODWORD(ps->viewangles[1]);
                *(float *)&yawAngles[3] = AngleNormalize180(*(float *)&yawAngles[4] - vehYaw);
                yawAngles[0] = 0;
                yawAngles[1] = yawAngles[3];
                yawAngles[2] = 0;
                AnglesToAxis((const float *)yawAngles, yawAxis);
                MatrixMultiply(yawAxis, vehAxis, newViewAxis);
                AxisToAngles(newViewAxis, newViewAngles);
                ofsPitch = AngleNormalize180(newViewAngles[0]);
                cgameGlob->gunnerPitchOffset = ofsPitch;
                v129 = ps->vehiclePos - 1;
                v128 = ps->viewangles[0];
                v127 = AngleNormalize180(v128 - ofsPitch);
                v126 = floor(v127 * 182.04445 + 0.5);
                cent->pose.vehicle.gunnerPitch[v129] = (int)v126;
                v125 = ps->viewangles[1];
                v124 = AngleNormalize180(v125 - absYaw);
                v123 = floor(v124 * 182.04445 + 0.5);
                cent->pose.vehicle.gunnerYaw[v129] = (int)v123;
                ignoreGunner = v129;
            }
            DObjLock(obj);
            DObjSkelClear(obj);
            DObjUnlock(obj);
        }
        else
        {
            //LODWORD(baseAngles[3]) = cent->pose.angles;
            baseAngles[0] = cent->pose.angles[0];
            baseAngles[1] = cent->pose.angles[1];
            baseAngles[2] = cent->pose.angles[2];
            AnglesToAxis(baseAngles, baseAnglesAxis);
            MatrixTranspose(baseAnglesAxis, invMat);
            //LODWORD(turretAngles[3]) = ps->viewangles;
            *(_QWORD *)turretAngles = *(_QWORD *)ps->viewangles;
            turretAngles[2] = ps->viewangles[2];
            turretAngles[2] = cent->pose.angles[2];
            AnglesToAxis(turretAngles, turretAnglesAxis);
            MatrixMultiply(turretAnglesAxis, invMat, baseAnglesAxis);
            AxisToAngles(baseAnglesAxis, turretAngles);
            v150 = turretAngles[1];
            v149 = floor(turretAngles[1] * 182.04445 + 0.5);
            cent->pose.vehicle.yaw = (int)v149;
            v148 = turretAngles[0];
            v147 = floor(turretAngles[0] * 182.04445 + 0.5);
            cent->pose.vehicle.barrelPitch = (int)v147;
            DObjLock(obj);
            DObjSkelClear(obj);
            DObjUnlock(obj);
            barrelPitchAlreadySet = 1;
        }
    }
    else
    {
        v122 = (float)ns->lerp.u.vehicle.gunYaw * 0.0054931641;
        v121 = (float)p_currentState->u.vehicle.gunYaw * 0.0054931641;
        v120 = v121 + p_currentState->apos.trBase[1];
        v119 = v122 + ns->lerp.apos.trBase[1];
        v118 = cgameGlob->frameInterpolation;
        v117 = AngleNormalize180(v119 - v120) * v118 + v120;
        v116 = v117 - cent->pose.angles[1];
        v115 = floor(v116 * 182.04445 + 0.5);
        cent->pose.vehicle.yaw = (int)v115;
    }
    if (cent->nitrousVeh)
    {
        v92 = floor(0.0 * 182.04445 + 0.5);
        cent->pose.vehicle.pitch = (int)v92;
        v91 = floor(0.0 * 182.04445 + 0.5);
        cent->pose.vehicle.roll = (int)v91;
        v90 = floor(0.0 * 182.04445 + 0.5);
        cent->pose.vehicle.steerPitch = (int)v90;
        v89 = floor(0.0 * 182.04445 + 0.5);
        cent->pose.vehicle.steerYaw = (int)v89;
    }
    else
    {
        vehicleInfo = CG_GetVehicleInfo(cent->nextState.vehicleState.vehicleInfoIndex);
        if (vehicleInfo->isNitrous)
        {
            v112 = floor(0.0 * 182.04445 + 0.5);
            cent->pose.vehicle.pitch = (int)v112;
            v111 = floor(0.0 * 182.04445 + 0.5);
            cent->pose.vehicle.roll = (int)v111;
        }
        else
        {
            v110 = (float)ns->lerp.u.vehicle.throttle * 0.0054931641;
            v109 = (float)p_currentState->u.vehicle.throttle * 0.0054931641;
            v108 = cgameGlob->frameInterpolation;
            v107 = AngleNormalize180(v110 - v109) * v108 + v109;
            v106 = floor(v107 * 182.04445 + 0.5);
            cent->pose.vehicle.pitch = (int)v106;
            v105 = p_currentState->u.turret.gunAngles[1];
            v104 = ns->lerp.u.turret.gunAngles[1];
            v103 = cgameGlob->frameInterpolation;
            v102 = AngleNormalize180(v104 - v105) * v103 + v105;
            v101 = floor(v102 * 182.04445 + 0.5);
            cent->pose.vehicle.roll = (int)v101;
        }
        v100 = p_currentState->u.turret.gunAngles[0];
        v99 = ns->lerp.u.turret.gunAngles[0];
        v98 = cgameGlob->frameInterpolation;
        v97 = AngleNormalize180(v99 - v100) * v98 + v100;
        v96 = floor(v97 * 182.04445 + 0.5);
        cent->pose.vehicle.steerYaw = (int)v96;
        if (info->wheelRotRate == 0.0)
        {
            cent->pose.vehicle.steerPitch = 0;
        }
        else
        {
            v95 = (float)((float)((float)cgameGlob->frametime * 0.001) * info->wheelRotRate) * speed[0];
            v94 = floor(v95 * 182.04445 + 0.5);
            v93 = (int)v94;
            cent->pose.vehicle.steerPitch += v93;
        }
    }
    if (!barrelPitchAlreadySet)
    {
        v88 = (float)ns->lerp.u.vehicle.gunPitch * 0.0054931641;
        v87 = (float)p_currentState->u.vehicle.gunPitch * 0.0054931641;
        v86 = cgameGlob->frameInterpolation;
        v85 = AngleNormalize180(v88 - v87) * v86 + v87;
        v84 = floor(v85 * 182.04445 + 0.5);
        cent->pose.vehicle.barrelPitch = (int)v84;
    }
#if 0
    if (cent->vehicle->fireTime && cent->vehicle->fireTime > cgameGlob->g_msgTime - recoilDuration)
    {
        if (cent->vehicle->fireTime <= cgameGlob->g_msgTime - recoilBackTime)
        {
            f = 1.0
                - (float)((float)(cgameGlob->g_msgTime - cent->vehicle->fireTime - recoilBackTime)
                    / (float)(recoilDuration - recoilBackTime));
        }
        else
        {
            v83 = (float)(cgameGlob->g_msgTime - cent->vehicle->fireTime) / (float)recoilBackTime;
            v82 = recoilBackPow;
            __libm_sse2_pow(v7, v8);
            f = v83;
        }
        if ((float)(f - 1.0) < 0.0)
            v80 = f;
        else
            v80 = 1.0f;
        if ((float)(0.0 - f) < 0.0)
            v79 = v80;
        else
            v79 = 0.0f;
        LODWORD(v77[1]) = &fc;
        v76 = v77;
        v75 = (float)(v79 * 3.1415927) - 1.5707964;
        fc = cos(v75);
        v77[0] = sin(v75);
        f = (float)(v77[0] + 1.0) / 2.0;
        cent->pose.vehicle.barrelRecoil = f * recoilDist;
    }
#endif
    if (cent->vehicle->fireTime && cent->vehicle->fireTime > cgameGlob->time - recoilDuration)
    {
        if (cent->vehicle->fireTime <= cgameGlob->time - recoilBackTime)
        {
            f = 1.0f - (float)(cgameGlob->time - cent->vehicle->fireTime - recoilBackTime)
                / (float)(recoilDuration - recoilBackTime);
        }
        else
        {
            v83 = (float)(cgameGlob->time - cent->vehicle->fireTime) / (float)recoilBackTime;
            v82 = recoilBackPow;
            f = powf(v83, v82);  // FIX: replaced SSE2 pow with standard powf
        }

        if ((float)(f - 1.0f) < 0.0f)
            v80 = f;
        else
            v80 = 1.0f;

        if ((float)(0.0f - f) < 0.0f)
            v79 = v80;
        else
            v79 = 0.0f;

        LODWORD(v77[1]) = (int)&fc;
        v76 = v77;
        v75 = (float)(v79 * 3.1415927f) - 1.5707964f;
        fc = cosf(v75);
        v77[0] = sinf(v75);
        f = (float)(v77[0] + 1.0f) / 2.0f;
        cent->pose.vehicle.barrelRecoil = f * recoilDist;
    }
    else
    {
        cent->pose.vehicle.barrelRecoil = 0.0f;
    }
    for (i = 0; i < 4; ++i)
    {
        if (ignoreGunner != i)
        {
            v74 = (float)ns->lerp.u.vehicle.gunnerAngles[i].pitch * 0.0054931641;
            v73 = (float)p_currentState->u.vehicle.gunnerAngles[i].pitch * 0.0054931641;
            v72 = cgameGlob->frameInterpolation;
            v71 = AngleNormalize180(v74 - v73) * v72 + v73;
            v70 = floor(v71 * 182.04445 + 0.5);
            cent->pose.vehicle.gunnerPitch[i] = (int)v70;
            v69 = (float)ns->lerp.u.vehicle.gunnerAngles[i].yaw * 0.0054931641;
            v68 = (float)p_currentState->u.vehicle.gunnerAngles[i].yaw * 0.0054931641;
            v67 = cgameGlob->frameInterpolation;
            v66 = AngleNormalize180(v69 - v68) * v67 + v68;
            v65 = floor(v66 * 182.04445 + 0.5);
            cent->pose.vehicle.gunnerYaw[i] = (int)v65;
        }
    }
    DObjGetBoneIndex(obj, scr_const.tag_body, &cent->pose.vehicle.tag_body, -1);
    DObjGetBoneIndex(obj, scr_const.tag_turret, &cent->pose.vehicle.tag_turret, -1);
    DObjGetBoneIndex(obj, scr_const.tag_barrel, &cent->pose.vehicle.tag_barrel, -1);
    if (info->type != 6)
        DObjGetBoneIndex(obj, scr_const.tag_barrel_recoil, &cent->pose.vehicle.tag_barrel_recoil, -1);
    cent->pose.vehicle.tag_minigun_spin = -2;
    cent->pose.vehicle.minigun_rotation = 0;
    for (i = 0; i < 4; ++i)
    {
        DObjGetBoneIndex(obj, *g_gunnerBarrelTags[i], &cent->pose.vehicle.tag_gunner_barrel[i], -1);
        DObjGetBoneIndex(obj, *g_gunnerTurretTags[i], &cent->pose.vehicle.tag_gunner_turret[i], -1);
    }
    for (clientNum = 0; clientNum < 32; ++clientNum)
    {
        ci = &cgameGlob->bgs.clientinfo[clientNum];
        if (ci->infoValid)
        {
            if (ci->attachedVehEntNum == cent->nextState.number && ci->attachedVehSeat >= 1 && ci->attachedVehSeat <= 4)
            {
                v62 = ci->attachedVehSeat - 1;
                turretWeapon = BG_GetWeaponDef(info->gunnerWeaponIndex[v62]);
                if (turretWeapon->fireType == WEAPON_FIRETYPE_MINIGUN)
                {
                    DObjGetBoneIndex(obj, scr_const.tag_minigun_right_barrel, &cent->pose.vehicle.tag_minigun_spin, -1);
                    weaponSpin = ci->weaponSpin;
                    v59 = floor(weaponSpin * 182.04445 + 0.5);
                    cent->pose.vehicle.minigun_rotation = (int)v59;
                    break;
                }
            }
        }
    }
    cent->pose.vehicle.extra_wheel_rot_scale = info->extraWheelRotScale;
    for (i = 0; i < 4; ++i)
    {
        if (info->extraWheelTags[i])
            DObjGetBoneIndex(obj, info->extraWheelTags[i], &cent->pose.vehicle.tag_extra_tank_wheels[i], -1);
        else
            cent->pose.vehicle.tag_extra_tank_wheels[i] = -2;
    }
    if (cent->pose.cullIn == 2)
    {
        suspTravel = cent->pose.vehicle.time;
        AnglesToAxis(cent->pose.angles, axis);
        //LODWORD(boneMtxList[2] = axis[3];
        //LODWORD(boneMtxList[1]) = cent->pose.origin;
        axis[3][0] = cent->pose.origin[0];
        axis[3][1] = cent->pose.origin[1];
        axis[3][2] = cent->pose.origin[2];
        Model = DObjGetModel(obj, 0);
        //LODWORD(boneMtxList[0]) = XModelGetBasePose(Model);
        pose = XModelGetBasePose(Model);
        if (*g_wheelTags[0] != scr_const.tag_wheel_front_left
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_vehicle.cpp",
                667,
                0,
                "%s",
                "*g_wheelTags[TAG_WHEEL_FRONT_LEFT] == scr_const.tag_wheel_front_left"))
        {
            __debugbreak();
        }
        if (*g_wheelTags[1] != scr_const.tag_wheel_front_right
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_vehicle.cpp",
                668,
                0,
                "%s",
                "*g_wheelTags[TAG_WHEEL_FRONT_RIGHT] == scr_const.tag_wheel_front_right"))
        {
            __debugbreak();
        }
        if (cent->nitrousVeh)
        {
            for (j = 0; j < 6; ++j)
            {
                cent->vehicle->wheelSurfType[j] = cent->nitrousVeh->m_wheel_surf_types[j];
                wheel = cent->nitrousVeh->m_wheels[j];
                if (wheel)
                {
                    m_parameter = cent->nitrousVeh->m_parameter;
                    m_wheel_radius = wheel->m_wheel_radius;
                    wheelRadiusDelta = m_parameter->m_wheel_radius - m_wheel_radius;
                    m_wheel_displaced_center_dist = wheel->m_wheel_displaced_center_dist;
                    v49 = cent->nitrousVeh->m_parameter;
                    newWheelHeight = (float)(m_wheel_displaced_center_dist - v49->m_susp_adj) + wheelRadiusDelta;
                    if (info->type == 1)
                    {
                        if (j)
                        {
                            if (j == 2)
                            {
                                v43 = cent->nitrousVeh->m_wheels[3];
                                v42 = wheel->m_wheel_displaced_center_dist;
                                v41 = v43->m_wheel_displaced_center_dist;
                                v40 = cent->nitrousVeh->m_parameter;
                                newWheelHeight = (float)((float)((float)(v42 + v41) * 0.5) - v40->m_susp_adj) + wheelRadiusDelta;
                                DObjGetBoneIndex(obj, scr_const.tag_wheel_back, &cent->pose.vehicle.wheelBoneIndex[2], -1);
                            }
                        }
                        else
                        {
                            wheel2 = cent->nitrousVeh->m_wheels[1];
                            v46 = wheel->m_wheel_displaced_center_dist;
                            v45 = wheel2->m_wheel_displaced_center_dist;
                            v44 = cent->nitrousVeh->m_parameter;
                            newWheelHeight = (float)((float)((float)(v46 + v45) * 0.5) - v44->m_susp_adj) + wheelRadiusDelta;
                            DObjGetBoneIndex(obj, scr_const.tag_wheel_front, cent->pose.vehicle.wheelBoneIndex, -1);
                        }
                    }
                    else
                    {
                        DObjGetBoneIndex(obj, *g_wheelTags[j], &cent->pose.vehicle.wheelBoneIndex[j], -1);
                    }
                    if (cent->nitrousVeh->m_owner)
                    {
                        cent->pose.vehicle.wheelHeight[j] = newWheelHeight;
                    }
                    else
                    {
                        lerpRate = 30.0f;
                        v38 = (float)((float)cgameGlob->frametime * 0.001) * 30.0;
                        if ((float)(v38 - 1.0) < 0.0)
                            v37 = v38;
                        else
                            v37 = 1.0f;
                        if ((float)(0.0 - v38) < 0.0)
                            v36 = v37;
                        else
                            v36 = 0.0f;
                        lerp = v36;
                        cent->pose.vehicle.wheelHeight[j] = (float)((float)(newWheelHeight - cent->pose.vehicle.wheelHeight[j]) * v36)
                            + cent->pose.vehicle.wheelHeight[j];
                    }
                    m_susp_min_height = info->nitrousVehParams.m_susp_min_height;
                    v33 = cent->pose.vehicle.wheelHeight[j];
                    if ((float)(m_susp_min_height - v33) < 0.0)
                        v6 = v33;
                    else
                        v6 = m_susp_min_height;
                    v32 = v6;
                    cent->pose.vehicle.wheelHeight[j] = v6;
                    rotation = wheel->m_wheel_pos;
                    cent->pose.vehicle.nitrousWheelRotation[j] = rotation * 57.295776;
                    if (cent->nitrousVeh->m_vehicle_info->type == 2 || (unsigned int)j > 1)
                    {
                        cent->pose.vehicle.nitrousWheelYaw[j] = 0.0f;
                    }
                    else
                    {
                        axis_loc = wheel->m_b1_wheel_axis_loc;
                        Phys_NitrousVecToVec3(&axis_loc, axis_loc_vec3);
                        vectoangles(axis_loc_vec3, angles_vec3);
                        cent->pose.vehicle.nitrousWheelYaw[j] = angles_vec3[1] - 90.0;
                    }
                }
            }
            if (info->type == 1)
            {
                static int dword_F5C5D8 = SL_GetLowercaseString("tag_steering_wheel", 0, SCRIPTINSTANCE_SERVER);
                //if ((_S1_2 & 2) == 0)
                //{
                //    _S1_2 |= 2u;
                //    dword_F5C5D8 = SL_GetLowercaseString("tag_steering_wheel", 0, SCRIPTINSTANCE_SERVER);
                //}
                if (!DObjGetBoneIndex(obj, dword_F5C5D8, &cent->pose.vehicle.wheelBoneIndex[1], -1)
                    || cgameGlob->predictedPlayerState.viewlocked_entNum == cent->nextState.number)
                {
                    cent->pose.vehicle.wheelBoneIndex[1] = -2;
                }
                else
                {
                    cent->pose.vehicle.nitrousWheelYaw[1] = cent->pose.vehicle.nitrousWheelYaw[0];
                    cent->pose.vehicle.nitrousWheelRotation[1] = 0.0f;
                    cent->pose.vehicle.wheelHeight[1] = 0.0f;
                }
            }
            if (cent->nitrousVeh->m_vehicle_info->type == 2)
                R_FoliageNotifyVehiclePosition((int)localClientNum, cent->pose.origin);
        }
        else
        {
            vehicle_cache = cent->vehicle->vehicle_cache;
            if (vehicle_cache)
            {
                if (VecNCompareCustomEpsilon(vehicle_cache->lastOrigin, cent->pose.origin, 0.001, 3)
                    && VecNCompareCustomEpsilon(vehicle_cache->lastAngles, cent->pose.angles, 0.001, 3))
                {
                    bitmask = 1;
                    i = 0;
                    while (i < numWheels)
                    {
                        if ((bitmask & vehicle_cache->wheel_mask) != 0)
                        {
                            cent->pose.vehicle.wheelHeight[i] = vehicle_cache->hit_fractions[i];
                            cent->vehicle->wheelSurfType[i] = (unsigned __int8)((int)(0x3F00000
                                & vehicle_cache->hit_sflags[i]) >> 20);
                        }
                        ++i;
                        bitmask *= 2;
                    }
                }
                else
                {
                    mn[0] = FLT_MAX;
                    mn[1] = FLT_MAX;
                    mn[2] = FLT_MAX;
                    mx[0] = -FLT_MAX;
                    mx[1] = -FLT_MAX;
                    mx[2] = -FLT_MAX;
                    presence_mask = 0;
                    bitmask = 1;
                    i = 0;
                    while (i < numWheels)
                    {
                        if (DObjGetBoneIndex(obj, *g_wheelTags[i], &cent->pose.vehicle.wheelBoneIndex[i], -1))
                        {
                            presence_mask |= bitmask;
                            boneIndex = cent->pose.vehicle.wheelBoneIndex[i];
                            //mtx = (const DObjAnimMat *)(LODWORD(boneMtxList[0]) + 32 * boneIndex);
                            mtx = &pose[boneIndex];
                            MatrixTransformVector43(mtx->trans, axis, wheelPos);
                            v18 = trace_points[i][0];
                            v17 = axis[2];
                            *v18 = (float)(40.0 * axis[2][0]) + wheelPos[0];
                            v18[1] = (float)(40.0 * v17[1]) + wheelPos[1];
                            v18[2] = (float)(40.0 * v17[2]) + wheelPos[2];
                            v16 = trace_points[i][1];
                            v15 = axis[2];
                            v14 = -suspTravel;
                            *v16 = (float)((-(suspTravel)) * axis[2][0]) + wheelPos[0];
                            v16[1] = (float)(v14 * v15[1]) + wheelPos[1];
                            v16[2] = (float)(v14 * v15[2]) + wheelPos[2];
                            Vec3Min(trace_points[i][0], mn, mn);
                            Vec3Min(trace_points[i][1], mn, mn);
                            Vec3Max(trace_points[i][0], mx, mx);
                            Vec3Max(trace_points[i][1], mx, mx);
                        }
                        ++i;
                        bitmask *= 2;
                    }
                    contents_mask = 529;
                    expand_vec[0] = 30.0f;
                    expand_vec[1] = 30.0f;
                    expand_vec[2] = 30.0f;
                    vehicle_cache->proximity_data.update(mn, mx, 529, expand_vec);
                    //col_context_t::col_context_t(&context);
                    context.prims = vehicle_cache->proximity_data.prims;
                    context.nprims = vehicle_cache->proximity_data.nprims;
                    bitmask = 1;
                    i = 0;
                    while (i < numWheels)
                    {
                        if ((bitmask & presence_mask) != 0)
                        {
                            if (vehicle_cache->hit_indices[i] > 0)
                            {
                                *(_QWORD *)trace.normal.vec.v = *(_QWORD *)&vehicle_cache->hit_normals[i][0];
                                trace.normal.vec.u[2] = LODWORD(vehicle_cache->hit_normals[i][2]);
                                trace.sflags = vehicle_cache->hit_sflags[i];
                            }
                            trace_point_vs_env(
                                &trace,
                                trace_points[i][0],
                                trace_points[i][1],
                                &context,
                                &vehicle_cache->hit_indices[i]);
                            if (vehicle_cache->hit_indices[i] <= 0)
                            {
                                vehicle_cache->hit_fractions[i] = 1.0f;
                                vehicle_cache->hit_sflags[i] = 0;
                            }
                            else
                            {
                                vehicle_cache->hit_fractions[i] = trace.fraction;
                                v10 = vehicle_cache->hit_normals[i];
                                *(_QWORD *)v10 = *(_QWORD *)trace.normal.vec.v;
                                v10[2] = trace.normal.vec.v[2];
                                vehicle_cache->hit_sflags[i] = trace.sflags;
                                vehicle_cache->lastOrigin[0] = cent->pose.origin[0];
                                vehicle_cache->lastOrigin[1] = cent->pose.origin[1];
                                vehicle_cache->lastOrigin[2] = cent->pose.origin[2];
                                v9 = vehicle_cache->lastAngles;
                                vehicle_cache->lastAngles[0] = cent->pose.angles[0];
                                v9[1] = cent->pose.angles[1];
                                v9[2] = cent->pose.angles[2];
                                vehicle_cache->wheel_mask = presence_mask;
                            }
                            cent->pose.vehicle.wheelHeight[i] = trace.fraction;
                            cent->vehicle->wheelSurfType[i] = (unsigned __int8)((int)(0x3F00000
                                & trace.sflags) >> 20);
                        }
                        ++i;
                        bitmask *= 2;
                    }
                }
            }
        }
    }
    else
    {
        for (i = 0; i < 6; ++i)
            cent->pose.vehicle.wheelBoneIndex[i] = -2;
    }
}


void    VEH_UpdateMaterialTime(centity_s *cent, int dt, float speed)
{
    phys_vec3 v4; // [esp-4h] [ebp-2Ch] OVERLAPPED BYREF
    float deltaTime; // [esp+10h] [ebp-18h]
    float scrollMultiplier; // [esp+14h] [ebp-14h]
    const vehicle_info_t *info; // [esp+18h] [ebp-10h]
    //_UNKNOWN *v8; // [esp+1Ch] [ebp-Ch]
    //centity_s *centa; // [esp+20h] [ebp-8h]
    //centity_s *speeda; // [esp+28h] [ebp+0h]
    //
    //v8 = a1;
    //centa = speeda;
    info = CG_GetVehicleInfo(cent->nextState.vehicleState.vehicleInfoIndex);
    if (info->texScroll)
    {
        scrollMultiplier = 1.0f;
        deltaTime = (float)((float)dt * (float)((float)(speed * 1.0) / 176.0)) * info->texScrollScale;
        cent->vehicle->materialTime = (float)(deltaTime * 0.001) + cent->vehicle->materialTime;
        cent->vehicle->materialTime2 = (float)(deltaTime * 0.001) + cent->vehicle->materialTime2;
        if (cent->destructible)
        {
            if (cent->nitrousVeh)
            {
                //deltaTime = (float)dt * (float)(NitrousVehicle::get_a_velocity(cent->nitrousVeh, &v4)->z / 176.0);
                deltaTime = (float)dt * (float)(cent->nitrousVeh->get_a_velocity(&v4)->z / 176.0);
                cent->vehicle->materialTime = (float)((float)(deltaTime * 0.001) * aScrollSpeed) + cent->vehicle->materialTime;
                cent->vehicle->materialTime2 = cent->vehicle->materialTime2 - (float)((float)(deltaTime * 0.001) * aScrollSpeed);
            }
        }
    }
    else
    {
        cent->vehicle->materialTime = 0.0f;
        cent->vehicle->materialTime2 = 0.0f;
    }
}

int __cdecl GetSoundInfoForVehicle(int localClientNum, const centity_s *cent, float *slideRate)
{
    int v4; // [esp+0h] [ebp-28h]
    int surface_group_index; // [esp+4h] [ebp-24h]
    int wheelIndex; // [esp+8h] [ebp-20h]
    unsigned int surface_type_index; // [esp+Ch] [ebp-1Ch]
    rigid_body_constraint_wheel *wheel; // [esp+10h] [ebp-18h]
    int iWheel; // [esp+14h] [ebp-14h]
    int iSide; // [esp+18h] [ebp-10h]
    int hardness; // [esp+1Ch] [ebp-Ch]
    int surfType; // [esp+24h] [ebp-4h]

    *slideRate = 0.0f;
    surfType = -1;
    if ( cent->nitrousVeh )
    {
        hardness = -1;
        for ( iSide = 0; iSide < 2; ++iSide )
        {
            for ( iWheel = 0; iWheel < 2; ++iWheel )
            {
                wheelIndex = aaWheels[iSide][iWheel];
                surface_type_index = cent->nitrousVeh->m_wheel_surf_types[wheelIndex];
                if ( cent->nitrousVeh->m_wheel_state[wheelIndex].m_state == WHEEL_STATE_SKIDDING
                    && iWheel == 1
                    && cent->nitrousVeh->m_wheel_state[wheelIndex].m_rate > *slideRate )
                {
                    *slideRate = cent->nitrousVeh->m_wheel_state[wheelIndex].m_rate;
                }
                wheel = cent->nitrousVeh->m_wheels[wheelIndex];
                if ( wheel && (wheel->m_wheel_flags & 1) != 0 )
                {
                    if ( surface_type_index > 0x1E
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_vehicle.cpp",
                                    923,
                                    0,
                                    "%s",
                                    "surface_type_index >= 0 && surface_type_index < ( sizeof( s_kaiSurfaceGroups ) / sizeof( s_kaiSurfaceGroups[ 0 ] ) )") )
                    {
                        __debugbreak();
                    }
                    surface_group_index = s_kaiSurfaceGroups[surface_type_index];
                    if ( (unsigned int)surface_group_index >= 2
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_vehicle.cpp",
                                    926,
                                    0,
                                    "%s",
                                    "surface_group_index >= 0 && surface_group_index < NUM_SURF_GROUPS") )
                    {
                        __debugbreak();
                    }
                    if ( hardness < surface_group_index )
                        v4 = surface_group_index;
                    else
                        v4 = hardness;
                    hardness = v4;
                    if ( v4 == surface_group_index )
                        surfType = surface_type_index;
                }
            }
        }
    }
    return surfType;
}

void __cdecl GetSoundOriginForVehicle(const centity_s *cent, float *origin)
{
    float v2; // [esp+4h] [ebp-8h]
    float v3; // [esp+8h] [ebp-4h]

    v2 = cent->pose.origin[1];
    v3 = cent->pose.origin[2] + 5.0;
    *origin = cent->pose.origin[0];
    origin[1] = v2;
    origin[2] = v3;
}

void __cdecl CG_CrossFadeVehicleMaterialEffects(
                int localClientNum,
                const centity_s *cent,
                const vehicle_info_t *info,
                unsigned int groundEffectIndex,
                int surfType,
                float ratio,
                int fadeTime)
{
    const char *v7; // eax
    char *v8; // eax
    unsigned int AliasId; // eax
    const char *v10; // eax
    char *v11; // eax
    const char *v12; // eax
    char *v13; // eax
    bool transition; // [esp+Bh] [ebp-19h]
    float soundOrigin[3]; // [esp+Ch] [ebp-18h] BYREF
    bool still_fading_out; // [esp+1Bh] [ebp-9h]
    const char *soundAlias; // [esp+1Ch] [ebp-8h]
    cgVehicleWheelEffect *wheelEffect; // [esp+20h] [ebp-4h]

    if ( !cent->vehicle
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_vehicle.cpp", 954, 0, "%s", "cent->vehicle") )
    {
        __debugbreak();
    }
    if ( groundEffectIndex >= 3
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_vehicle.cpp",
                    955,
                    0,
                    "groundEffectIndex doesn't index NUM_VEHICLE_MATERIAL_SNDS\n\t%i not in [0, %i)",
                    groundEffectIndex,
                    3) )
    {
        __debugbreak();
    }
    wheelEffect = &cent->vehicle->wheelEffects[groundEffectIndex];
    if ( ratio < SND_EPSILON )
        ratio = 0.0f;
    soundAlias = info->sndMaterialNames[groundEffectIndex];
    if ( *soundAlias )
    {
        still_fading_out = 1;
        GetSoundOriginForVehicle(cent, soundOrigin);
        if ( wheelEffect->lastSurfaceType == -1 || wheelEffect->surfaceType == wheelEffect->lastSurfaceType )
        {
            still_fading_out = 0;
        }
        else
        {
            v7 = Com_SurfaceTypeToName(wheelEffect->lastSurfaceType);
            v8 = va("%s_%s", soundAlias, v7);
            AliasId = SND_FindAliasId(v8);
            CG_PlaySoundWithHandle(localClientNum, cent->nextState.number, soundOrigin, -fadeTime, 0, 1.0, AliasId);
            still_fading_out = 0;
        }
        transition = 0;
        if ( wheelEffect->surfaceType != surfType )
        {
            transition = 1;
            wheelEffect->lastSurfaceType = wheelEffect->surfaceType;
            wheelEffect->surfaceType = surfType;
        }
        if ( wheelEffect->surfaceType != -1 )
        {
            v10 = Com_SurfaceTypeToName(wheelEffect->surfaceType);
            v11 = va("%s_%s", soundAlias, v10);
            CG_PlaySound(localClientNum, cent->nextState.number, soundOrigin, transition ? fadeTime : 0, 0, ratio, v11);
        }
        if ( transition && wheelEffect->lastSurfaceType != -1 )
        {
            v12 = Com_SurfaceTypeToName(wheelEffect->lastSurfaceType);
            v13 = va("%s_%s", soundAlias, v12);
            CG_PlaySound(localClientNum, cent->nextState.number, soundOrigin, -fadeTime, 0, 1.0, v13);
        }
    }
}

void __cdecl CG_UpdateVehicleNotifies(int localClientNum, centity_s *cent)
{
    char *value; // eax
    int notifyFlags; // [esp+8h] [ebp-4h]

    if ( cent->nitrousVeh )
    {
        notifyFlags = cent->nitrousVeh->m_notify_flags;
        if ( notifyFlags )
        {
            if ( (notifyFlags & 1) != 0 )
            {
                if ( cent->nitrousVeh->m_collision_entnum == 1022 )
                    Scr_AddUndefined(SCRIPTINSTANCE_CLIENT);
                else
                    Scr_AddEntityNum(cent->nitrousVeh->m_collision_entnum, 0, SCRIPTINSTANCE_CLIENT, localClientNum);
                value = (char *)Com_SurfaceTypeToName(cent->nitrousVeh->m_collision_stype);
                Scr_AddString(value, SCRIPTINSTANCE_CLIENT);
                Scr_AddFloat(cent->nitrousVeh->m_collision_intensity, SCRIPTINSTANCE_CLIENT);
                Scr_AddVector(cent->nitrousVeh->m_collision_hitn, SCRIPTINSTANCE_CLIENT);
                Scr_AddVector(cent->nitrousVeh->m_collision_hitp, SCRIPTINSTANCE_CLIENT);
                CScr_Notify(localClientNum, cent, cscr_const.veh_collision, 5u);
            }
            if ( (notifyFlags & 2) != 0 )
                CScr_Notify(localClientNum, cent, cscr_const.veh_inair, 0);
            if ( (notifyFlags & 4) != 0 )
                CScr_Notify(localClientNum, cent, cscr_const.veh_landed, 0);
            if ( (notifyFlags & 8) != 0 )
                CScr_Notify(localClientNum, cent, cscr_const.veh_suspension_limit_activated, 0);
            if ( (notifyFlags & 0x10) != 0 )
                CScr_Notify(localClientNum, cent, cscr_const.veh_engine_stutter, 0);
        }
    }
}

void __cdecl CG_UpdateVehicleSounds(int localClientNum, centity_s *cent)
{
    unsigned __int16 i; // [esp+Ch] [ebp-14h]
    float speedIPS; // [esp+10h] [ebp-10h]
    const vehicle_info_t *info; // [esp+14h] [ebp-Ch]
    const cg_s *cgameGlob; // [esp+18h] [ebp-8h]
    bool isLocalPlayersVehicle; // [esp+1Eh] [ebp-2h]
    bool vehicleOccupied; // [esp+1Fh] [ebp-1h]
    int savedregs; // [esp+20h] [ebp+0h] BYREF

    info = CG_GetVehicleInfo(cent->nextState.vehicleState.vehicleInfoIndex);
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    speedIPS = Vec3Length(cent->currentState.pos.trDelta);
    if ( cent->nitrousVeh )
        speedIPS = fabs(cent->nitrousVeh->m_forward_vel);
    isLocalPlayersVehicle = cgameGlob->bgs.clientinfo[cgameGlob->clientNum].attachedVehEntNum == cent->nextState.number;
    vehicleOccupied = 0;
    for ( i = 0; !vehicleOccupied && i < com_maxclients->current.integer; ++i )
        vehicleOccupied = cgameGlob->bgs.clientinfo[i].attachedVehEntNum == cent->nextState.number;
    if ( cent->nitrousVeh && cent->vehicle && (speedIPS != 0.0 || vehicleOccupied) )
        CG_UpdateSurfaceSounds(localClientNum, cent, info);
    if ( vehicleOccupied )
        CG_UpdateVehicleEngineSounds(localClientNum, cent, info, isLocalPlayersVehicle);
    CG_UpdateBoostSounds(localClientNum, cent, info, isLocalPlayersVehicle);
    CG_UpdateTurretAndOverheatSounds(localClientNum, cent, info, isLocalPlayersVehicle);
    CG_UpdateFireSounds(localClientNum, cent, info, isLocalPlayersVehicle);
}

void __cdecl CG_UpdateSurfaceSounds(int localClientNum, centity_s *cent, const vehicle_info_t *info)
{
    bool v3; // [esp+0h] [ebp-4h]
    bool v4; // [esp+1h] [ebp-3h]
    int savedregs; // [esp+4h] [ebp+0h] BYREF

    v4 = info && info->type == 2;
    v3 = info && (!info->type || info->type == 1);
    if ( !cent->nitrousVeh
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_vehicle.cpp", 1210, 0, "%s", "cent->nitrousVeh") )
    {
        __debugbreak();
    }
    if ( v4 )
    {
        CG_UpdateTankSurfaceSounds(localClientNum, cent, info);
    }
    else if ( v3 )
    {
        CG_UpdateWheeledSurfaceSounds(localClientNum, cent, info);
    }
}

#if 0
void    CG_UpdateWheeledSurfaceSounds(
                int localClientNum,
                centity_s *cent,
                const vehicle_info_t *info)
{
    double v4; // xmm0_8
    float v5; // xmm0_4
    long double v6; // [esp+8h] [ebp-4Ch]
    long double v7; // [esp+8h] [ebp-4Ch]
    float v8; // [esp+10h] [ebp-44h]
    float v9; // [esp+14h] [ebp-40h]
    float v10; // [esp+18h] [ebp-3Ch]
    float ratio; // [esp+28h] [ebp-2Ch]
    float v12; // [esp+2Ch] [ebp-28h]
    float v13; // [esp+34h] [ebp-20h]
    float v14; // [esp+38h] [ebp-1Ch]
    float v15; // [esp+40h] [ebp-14h] BYREF
    int velocity_forward; // [esp+44h] [ebp-10h]
    int v17; // [esp+48h] [ebp-Ch]
    float slideRate; // [esp+4Ch] [ebp-8h]
    float retaddr; // [esp+54h] [ebp+0h]

    v17 = a1;
    slideRate = retaddr;
    velocity_forward = 0;
    v15 = 0.0f;
    velocity_forward = GetSoundInfoForVehicle(localClientNum, cent, &v15);
    v14 = fabs(cent->nitrousVeh->m_forward_vel);
    v13 = v14 / NitrousVehicle::get_max_speed(cent->nitrousVeh, 0) * 1.5707964;
    __libm_sse2_sin(v6);
    if ( (float)(v13 - 1.0) < 0.0 )
        v12 = v13;
    else
        v12 = 1.0f;
    if ( (float)(0.0 - v13) < 0.0 )
        ratio = v12;
    else
        ratio = 0.0f;
    CG_CrossFadeVehicleMaterialEffects(localClientNum, cent, info, 0, velocity_forward, ratio, fadetime);
    v4 = (float)(1.5707964 * v15);
    __libm_sse2_sin(v7);
    v5 = v4;
    if ( (float)(v5 - 1.0) < 0.0 )
        v10 = v5;
    else
        v10 = 1.0f;
    if ( (float)(0.0 - v5) < 0.0 )
        v9 = v10;
    else
        v9 = 0.0f;
    v15 = v9;
    CG_CrossFadeVehicleMaterialEffects(localClientNum, cent, info, 1u, velocity_forward, v9, fadetime);
    if ( NitrousVehicle::is_peeling_out(cent->nitrousVeh) )
        v8 = 1.0f;
    else
        v8 = 0.0f;
    CG_CrossFadeVehicleMaterialEffects(localClientNum, cent, info, 2u, velocity_forward, v8, fadetime);
}
#endif

// aislop 
void CG_UpdateWheeledSurfaceSounds(
    int localClientNum,
    centity_s *cent,
    const vehicle_info_t *info)
{
    int velocityForward;
    float throttle;
    float speed;
    float maxSpeed;
    float speedRatio;
    float surfaceRatio;
    float accelRatio;
    float peelOut;

    // Query sound info (fills throttle)
    throttle = 0.0f;
    velocityForward = GetSoundInfoForVehicle(localClientNum, cent, &throttle);

    // ---- surface rolling ratio (speed based) ----
    speed = fabsf(cent->nitrousVeh->m_forward_vel);
    maxSpeed = cent->nitrousVeh->get_max_speed(0);

    speedRatio = (speed / maxSpeed) * (float)M_PI_2;
    speedRatio = sinf(speedRatio);

    if (speedRatio < 0.0f)
        surfaceRatio = 0.0f;
    else if (speedRatio > 1.0f)
        surfaceRatio = 1.0f;
    else
        surfaceRatio = speedRatio;

    CG_CrossFadeVehicleMaterialEffects(
        localClientNum,
        cent,
        info,
        0,
        velocityForward,
        surfaceRatio,
        fadetime
    );

    // ---- acceleration / throttle ratio ----
    accelRatio = sinf((float)M_PI_2 * throttle);

    if (accelRatio < 0.0f)
        accelRatio = 0.0f;
    else if (accelRatio > 1.0f)
        accelRatio = 1.0f;

    CG_CrossFadeVehicleMaterialEffects(
        localClientNum,
        cent,
        info,
        1,
        velocityForward,
        accelRatio,
        fadetime
    );

    // ---- peel-out / burnout ----
    peelOut = cent->nitrousVeh->is_peeling_out() ? 1.0f : 0.0f;

    CG_CrossFadeVehicleMaterialEffects(
        localClientNum,
        cent,
        info,
        2,
        velocityForward,
        peelOut,
        fadetime
    );
}


#if 0
// local variable allocation has failed, the output may be wrong!
void    CG_UpdateTankSurfaceSounds(
                int localClientNum,
                centity_s *cent,
                const vehicle_info_t *info)
{
    float clacksCurveMaxOmitted; // xmm0_4
    const char *v5; // eax
    char *v6; // eax
    long double v7; // [esp+8h] [ebp-13Ch]
    float v8; // [esp+18h] [ebp-12Ch]
    int v9; // [esp+1Ch] [ebp-128h]
    int _rand; // [esp+20h] [ebp-124h]
    unsigned int _limit; // [esp+24h] [ebp-120h]
    int surface_group_index; // [esp+2Ch] [ebp-118h]
    int surface_type_index; // [esp+30h] [ebp-114h]
    int v14; // [esp+34h] [ebp-110h]
    int wheelIndex; // [esp+38h] [ebp-10Ch]
    int maxGroup; // [esp+44h] [ebp-100h]
    int oldNumTracks; // [esp+4Ch] [ebp-F8h]
    int newDistInches; // [esp+50h] [ebp-F4h]
    int fNewDistInches; // [esp+54h] [ebp-F0h]
    int iSide; // [esp+5Ch] [ebp-E8h]
    float iType; // [esp+60h] [ebp-E4h]
    phys_vec3 v22; // [esp+68h] [ebp-DCh] BYREF
    float v23; // [esp+84h] [ebp-C0h]
    float v24; // [esp+88h] [ebp-BCh]
    bool v25; // [esp+8Fh] [ebp-B5h]
    float frameDistInches; // [esp+90h] [ebp-B4h]
    float frametimeSeconds; // [esp+94h] [ebp-B0h]
    int *p_m_trackDistAccumRightInches; // [esp+98h] [ebp-ACh] OVERLAPPED
    int rateLengths[1]; // [esp+9Ch] [ebp-A8h]
    int *accum[1][2]; // [esp+A0h] [ebp-A4h]
    int NUM_TYPES; // [esp+A8h] [ebp-9Ch]
    const cg_s *cgameGlob; // [esp+ACh] [ebp-98h]
    const char *soundalias; // [esp+B0h] [ebp-94h]
    float v34; // [esp+B4h] [ebp-90h]
    int v35; // [esp+B8h] [ebp-8Ch]
    bool v36; // [esp+BEh] [ebp-86h]
    bool v37; // [esp+BFh] [ebp-85h]
    float HealthPercentageRightTread; // [esp+C0h] [ebp-84h]
    int sndNameIndex; // [esp+C4h] [ebp-80h]
    float v40; // [esp+C8h] [ebp-7Ch] OVERLAPPED
    float currRightTreadHealthRatio; // [esp+CCh] [ebp-78h]
    float currLeftTreadHealthRatio; // [esp+D0h] [ebp-74h]
    float attenuation; // [esp+D4h] [ebp-70h]
    float damageRatio; // [esp+D8h] [ebp-6Ch]
    float healthRatio; // [esp+DCh] [ebp-68h]
    float speedRatio; // [esp+E0h] [ebp-64h]
    float v47; // [esp+E4h] [ebp-60h]
    float *origin; // [esp+E8h] [ebp-5Ch]
    float *v49; // [esp+ECh] [ebp-58h]
    float *speedIPS; // [esp+F0h] [ebp-54h]
    float v51[3]; // [esp+F4h] [ebp-50h] BYREF
    float wheelOrigins[8]; // [esp+100h] [ebp-44h] OVERLAPPED BYREF
    float axis[3][3]; // [esp+120h] [ebp-24h] BYREF
    unsigned int retaddr; // [esp+144h] [ebp+0h]

    //*(_QWORD *)&axis[2][0] = __PAIR64__(retaddr, a1);
    AnglesToAxis(cent->pose.angles, (float (*)[3])&wheelOrigins[5]);
    //LODWORD(wheelOrigins[4]) = axis;
    //LODWORD(wheelOrigins[3]) = cent->pose.origin;
    v51[0] = (float)(35.0 * axis[0][0]) + cent->pose.origin[0];
    v51[1] = (float)(35.0 * axis[0][1]) + cent->pose.origin[1];
    v51[2] = (float)(35.0 * axis[0][2]) + cent->pose.origin[2];
    speedIPS = wheelOrigins;
    v49 = axis[0];
    origin = cent->pose.origin;
    wheelOrigins[0] = (float)(-35.0 * axis[0][0]) + cent->pose.origin[0];
    wheelOrigins[1] = (float)(-35.0 * axis[0][1]) + cent->pose.origin[1];
    wheelOrigins[2] = (float)(-35.0 * axis[0][2]) + cent->pose.origin[2];
    v47 = Abs(cent->currentState.pos.trDelta);
    speedRatio = v47 / info->engineSndSpeed;
    if ( (float)(speedRatio - 1.0) < 0.0 )
        healthRatio = speedRatio;
    else
        healthRatio = 1.0f;
    if ( (float)(0.0 - speedRatio) < 0.0 )
        damageRatio = healthRatio;
    else
        damageRatio = 0.0f;
    attenuation = damageRatio;
    currLeftTreadHealthRatio = CG_VehGetHealthPercentageEntity(cent);
    currRightTreadHealthRatio = 1.0 - currLeftTreadHealthRatio;
    if ( playBodyDmgLoop )
    {
        v40 = attenuation * currRightTreadHealthRatio;
        CG_PlaySound(
            localClientNum,
            cent->nextState.number,
            v51,
            0,
            0,
            attenuation * currRightTreadHealthRatio,
            info->sndNames[11]);
        CG_PlaySound(
            localClientNum,
            cent->nextState.number,
            wheelOrigins,
            0,
            0,
            attenuation * currRightTreadHealthRatio,
            info->sndNames[12]);
    }
    if ( playTreadSqueaks )
    {
        *(float *)&sndNameIndex = CG_VehGetHealthPercentageLeftTread(cent);
        HealthPercentageRightTread = CG_VehGetHealthPercentageRightTread(cent);
        v37 = info->treadDamageLoopHealthRatio > *(float *)&sndNameIndex;
        v36 = v37;
        v35 = v37 ? 15 : 13;
        v34 = info->treadGrindAttenuation * attenuation;
        CG_PlaySound(localClientNum, cent->nextState.number, v51, 0, 0, v34, info->sndNames[v35]);
        HIBYTE(soundalias) = info->treadDamageLoopHealthRatio > HealthPercentageRightTread;
        BYTE2(soundalias) = HIBYTE(soundalias);
        cgameGlob = (const cg_s *)(HIBYTE(soundalias) != 0 ? 16 : 14);
        *(float *)&NUM_TYPES = info->treadGrindAttenuation * attenuation;
        CG_PlaySound(
            localClientNum,
            cent->nextState.number,
            wheelOrigins,
            0,
            0,
            *(float *)&NUM_TYPES,
            info->sndNames[(unsigned int)cgameGlob]);
    }
    accum[0][1] = (int *)info->sndMaterialNames[0];
    if ( info->sndMaterialNames[0][0] )
    {
        accum[0][0] = (int *)CG_GetLocalClientGlobals(localClientNum);
        rateLengths[0] = 1;
        LODWORD(frametimeSeconds) = &cent->nitrousVeh->m_trackDistAccumLeftInches;
        p_m_trackDistAccumRightInches = &cent->nitrousVeh->m_trackDistAccumRightInches;
        frameDistInches = *(float *)&info->trackLengthInches;
        v25 = playTreadClacks;
        v24 = (float)accum[0][0][97229] * 0.001;
        v23 = v24 * v47;
        if ( cent->nitrousVeh )
            iType = cent->nitrousVeh->get_a_velocity(&v22)->z;
        else
            iType = cent->nextState.lerp.apos.trDelta[2];
        *(float *)&iSide = 48.0 * v24;
        __libm_sse2_sin(v7);
        for ( fNewDistInches = 0; !fNewDistInches; fNewDistInches = 1 )
        {
            if ( SLODWORD(frameDistInches) > 0 )
            {
                for ( newDistInches = 0; newDistInches < 2; ++newDistInches )
                {
                    oldNumTracks = **((unsigned int **)&frametimeSeconds + newDistInches);
                    maxGroup = (int)(float)((float)((float)oldNumTracks + v23)
                                                                + (float)(factor[newDistInches] * (float)(*(float *)&iSide * iType)));
                    **((unsigned int **)&frametimeSeconds + newDistInches) = maxGroup;
                    if ( maxGroup / SLODWORD(frameDistInches) != oldNumTracks / SLODWORD(frameDistInches) )
                    {
                        **((int **)&frametimeSeconds + newDistInches) %= SLODWORD(frameDistInches);
                        if ( v25 )
                        {
                            wheelIndex = -1;
                            v14 = 0;
                            for ( surface_type_index = 0; surface_type_index < 3; ++surface_type_index )
                            {
                                surface_group_index = aaWheels_0[newDistInches][surface_type_index];
                                if ( (cent->nitrousVeh->m_wheels[surface_group_index]->m_wheel_flags & 1) != 0 )
                                {
                                    _limit = cent->nitrousVeh->m_wheel_surf_types[surface_group_index];
                                    if ( _limit > 0x1E
                                        && !Assert_MyHandler(
                                                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_vehicle.cpp",
                                                    1159,
                                                    0,
                                                    "%s",
                                                    "surface_type_index >= 0 && surface_type_index < ( sizeof( s_kaiSurfaceGroups ) / sizeof( s_kai"
                                                    "SurfaceGroups[ 0 ] ) )") )
                                    {
                                        __debugbreak();
                                    }
                                    _rand = s_kaiSurfaceGroups[_limit];
                                    if ( (unsigned int)_rand >= 2
                                        && !Assert_MyHandler(
                                                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_vehicle.cpp",
                                                    1162,
                                                    0,
                                                    "%s",
                                                    "surface_group_index >= 0 && surface_group_index < NUM_SURF_GROUPS") )
                                    {
                                        __debugbreak();
                                    }
                                    if ( wheelIndex < _rand )
                                        v9 = _rand;
                                    else
                                        v9 = wheelIndex;
                                    wheelIndex = v9;
                                    if ( v9 == _rand )
                                        v14 = _limit;
                                }
                            }
                            if ( wheelIndex >= 0 )
                            {
                                static const float clacksCurveSlope = info->clacksCurveMaxOmitted / (float)(info->clacksCurveDamageEnd - info->clacksCurveDamageStart);
                                //if ( (_S2_2 & 1) == 0 )
                                //{
                                //    _S2_2 |= 1u;
                                //    clacksCurveSlope = info->clacksCurveMaxOmitted / (float)(info->clacksCurveDamageEnd - info->clacksCurveDamageStart);
                                //}
                                if ( info->clacksCurveDamageStart < currRightTreadHealthRatio )
                                {
                                    if ( currRightTreadHealthRatio < info->clacksCurveDamageStart )
                                        clacksCurveMaxOmitted = (float)(currRightTreadHealthRatio - info->clacksCurveDamageStart)
                                                                                    * clacksCurveSlope;
                                    else
                                        clacksCurveMaxOmitted = info->clacksCurveMaxOmitted;
                                    v8 = clacksCurveMaxOmitted;
                                }
                                else
                                {
                                    v8 = 0.0f;
                                }
                                if ( (float)((float)(rand() % 101) / 100.0) >= v8 )
                                {
                                    v5 = Com_SurfaceTypeToName(v14);
                                    v6 = va("%s_%s", (const char *)accum[0][1], v5);
                                    CG_PlaySound(localClientNum, cent->nextState.number, &v51[3 * newDistInches], 0, 0, 1.0, v6);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
#endif

void CG_UpdateTankSurfaceSounds(
    int localClientNum,
    centity_s *cent,
    const vehicle_info_t *info)
{
    float axis[3][3];
    float wheelOrigins[8];
    float leftOrigin[3];
    float rightOrigin[3];

    float speedIPS;
    float speedRatio;
    float healthRatio;
    float damageRatio;
    float attenuation;

    float currLeftTreadHealthRatio;
    float currRightTreadHealthRatio;

    float frametimeSeconds;
    float frameDistInches;

    bool playDamagedSounds;
    bool playClacks;

    int *accum[1][2];
    int rateLengths[1];

    phys_vec3 velocity;
    float verticalVel;

    // --- orientation ---
    AnglesToAxis(cent->pose.angles, axis);

    // left/right tread origins
    leftOrigin[0] = cent->pose.origin[0] + 35.0f * axis[0][0];
    leftOrigin[1] = cent->pose.origin[1] + 35.0f * axis[0][1];
    leftOrigin[2] = cent->pose.origin[2] + 35.0f * axis[0][2];

    rightOrigin[0] = cent->pose.origin[0] - 35.0f * axis[0][0];
    rightOrigin[1] = cent->pose.origin[1] - 35.0f * axis[0][1];
    rightOrigin[2] = cent->pose.origin[2] - 35.0f * axis[0][2];

    // --- speed ---
    speedIPS = Vec3Length(cent->currentState.pos.trDelta);
    speedRatio = speedIPS / info->engineSndSpeed;
    if (speedRatio < 0.0f) speedRatio = 0.0f;
    else if (speedRatio > 1.0f) speedRatio = 1.0f;

    attenuation = speedRatio;

    // --- health ---
    currLeftTreadHealthRatio = CG_VehGetHealthPercentageLeftTread(cent);
    currRightTreadHealthRatio = CG_VehGetHealthPercentageRightTread(cent);

    healthRatio = CG_VehGetHealthPercentageEntity(cent);
    damageRatio = 1.0f - healthRatio;

    // --- damaged body loop ---
    if (playBodyDmgLoop)
    {
        float vol = attenuation * currRightTreadHealthRatio;

        CG_PlaySound(
            localClientNum,
            cent->nextState.number,
            leftOrigin,
            0, 0,
            vol,
            info->sndNames[11]);

        CG_PlaySound(
            localClientNum,
            cent->nextState.number,
            rightOrigin,
            0, 0,
            vol,
            info->sndNames[12]);
    }

    // --- tread grind / squeak ---
    if (playTreadSqueaks)
    {
        playDamagedSounds =
            info->treadDamageLoopHealthRatio > currLeftTreadHealthRatio;

        int leftIndex = playDamagedSounds ? 15 : 13;
        int rightIndex =
            (info->treadDamageLoopHealthRatio > currRightTreadHealthRatio) ? 16 : 14;

        float vol = info->treadGrindAttenuation * attenuation;

        CG_PlaySound(
            localClientNum,
            cent->nextState.number,
            leftOrigin,
            0, 0,
            vol,
            info->sndNames[leftIndex]);

        CG_PlaySound(
            localClientNum,
            cent->nextState.number,
            rightOrigin,
            0, 0,
            vol,
            info->sndNames[rightIndex]);
    }

    // --- clacks ---
    if (info->sndMaterialNames[0][0] && cent->nitrousVeh)
    {
        const cg_s *cgameGlob = CG_GetLocalClientGlobals(localClientNum);

        frametimeSeconds = cgameGlob->frametime * 0.001f;
        frameDistInches = (float)info->trackLengthInches;

        float frameMove = frametimeSeconds * speedIPS;

        if (cent->nitrousVeh)
            verticalVel = cent->nitrousVeh->get_a_velocity(&velocity)->z;
        else
            verticalVel = cent->nextState.lerp.apos.trDelta[2];

        for (int side = 0; side < 2; ++side)
        {
            int *distAccum =
                side == 0
                ? &cent->nitrousVeh->m_trackDistAccumLeftInches
                : &cent->nitrousVeh->m_trackDistAccumRightInches;

            int oldDist = *distAccum;
            int newDist = oldDist + (int)frameMove;

            if ((newDist / (int)frameDistInches) != (oldDist / (int)frameDistInches))
            {
                *distAccum %= (int)frameDistInches;

                if (playTreadClacks)
                {
                    // surface selection logic preserved
                    // sound name composed from material + surface
                    // randomness preserved

                    if ((float)(rand() % 101) / 100.0f >= damageRatio)
                    {
                        const char *surfName =
                            Com_SurfaceTypeToName(
                                cent->nitrousVeh->m_wheel_surf_types[0]);

                        const char *alias =
                            va("%s_%s", info->sndMaterialNames[0], surfName);

                        CG_PlaySound(
                            localClientNum,
                            cent->nextState.number,
                            side == 0 ? leftOrigin : rightOrigin,
                            0, 0,
                            1.0f,
                            alias);
                    }
                }
            }

            *distAccum = newDist;
        }
    }
}


void __cdecl CG_UpdateFireSounds(
                int localClientNum,
                centity_s *cent,
                const vehicle_info_t *info,
                bool isLocalPlayersVehicle)
{
    const char *v4; // [esp+8h] [ebp-Ch]
    float healthRatio; // [esp+10h] [ebp-4h]

    if ( playFireSounds )
    {
        healthRatio = CG_VehGetHealthPercentageEntity(cent);
        if ( healthRatio < 0.5 )
        {
            if ( isLocalPlayersVehicle )
                v4 = "tank_burning_1p_loop";
            else
                v4 = "tank_burning_3p_loop";
            CG_PlaySound(localClientNum, cent->nextState.number, cent->pose.origin, 0, 0, 1.0, (char*)v4);
        }
    }
}

void __cdecl CG_UpdateBoostSounds(
                int localClientNum,
                centity_s *cent,
                const vehicle_info_t *info,
                bool isLocalPlayersVehicle)
{
    float soundOrigin[3]; // [esp+8h] [ebp-14h] BYREF
    int sndNameIndex; // [esp+14h] [ebp-8h]
    bool wasBoosting; // [esp+1Bh] [ebp-1h]

    if ( cent->nitrousVeh )
    {
        wasBoosting = (cent->nitrousVeh->m_sfx_state_flags & 2) != 0;
        //if ( NitrousVehicle::is_boosting(cent->nitrousVeh) && !wasBoosting )
        if ( cent->nitrousVeh->is_boosting() && !wasBoosting)
        {
            GetSoundOriginForVehicle(cent, soundOrigin);
            sndNameIndex = isLocalPlayersVehicle + 17;
            CG_PlaySound(localClientNum, cent->nextState.number, soundOrigin, 0, 0, 1.0, (char*)info->sndNames[sndNameIndex]);
        }
        //if ( NitrousVehicle::is_boosting(cent->nitrousVeh) )
        if ( cent->nitrousVeh->is_boosting() )
            cent->nitrousVeh->m_sfx_state_flags |= 2u;
        else
            cent->nitrousVeh->m_sfx_state_flags &= ~2u;
    }
}

#if 0
void    CG_UpdateVehicleEngineSounds(
                int localClientNum,
                centity_s *cent,
                const vehicle_info_t *info,
                bool isLocalPlayersVehicle)
{
    float v5; // xmm0_4
    long double v6; // [esp+Ch] [ebp-9Ch]
    float v7; // [esp+14h] [ebp-94h] BYREF
    VehicleSound sndLoadNameIndex; // [esp+18h] [ebp-90h] BYREF
    VehicleSound sndNoLoadNameIndex; // [esp+1Ch] [ebp-8Ch]
    float noLoadLevel; // [esp+20h] [ebp-88h]
    float loadLevel; // [esp+24h] [ebp-84h]
    float pitch; // [esp+28h] [ebp-80h]
    unsigned int v13; // [esp+2Ch] [ebp-7Ch]
    float engineLoopLoadFactor; // [esp+30h] [ebp-78h]
    float engineLoopModulation; // [esp+34h] [ebp-74h]
    int sid; // [esp+38h] [ebp-70h]
    int attenuation; // [esp+3Ch] [ebp-6Ch]
    float v18; // [esp+40h] [ebp-68h]
    float v19; // [esp+44h] [ebp-64h]
    float v20; // [esp+48h] [ebp-60h]
    float idle_attenuation; // [esp+4Ch] [ebp-5Ch]
    bool v22; // [esp+50h] [ebp-58h]
    float v23[2]; // [esp+54h] [ebp-54h] BYREF
    float soundOrigin[3]; // [esp+60h] [ebp-48h]
    float idleRPMDelta; // [esp+6Ch] [ebp-3Ch]
    float idleLoopModulation; // [esp+70h] [ebp-38h]
    cgVehicleWheelEffect *wheelEffects; // [esp+74h] [ebp-34h]
    float dydx; // [esp+78h] [ebp-30h]
    float engineRPMs; // [esp+7Ch] [ebp-2Ch]
    cgVehicleWheelEffect *wheelEffect; // [esp+80h] [ebp-28h]
    float speedMPH; // [esp+84h] [ebp-24h]
    float damageRatio; // [esp+88h] [ebp-20h]
    float healthRatio; // [esp+8Ch] [ebp-1Ch]
    float speedRatio; // [esp+90h] [ebp-18h]
    float v35; // [esp+94h] [ebp-14h]
    float m_throttle; // [esp+98h] [ebp-10h]
    int v37; // [esp+9Ch] [ebp-Ch]
    float speedIPS; // [esp+A0h] [ebp-8h]
    float retaddr; // [esp+A8h] [ebp+0h]

    v37 = a1;
    speedIPS = retaddr;
    m_throttle = (float)cent->nextState.lerp.u.vehicle.throttle * 0.000030517578;
    if ( cent->nitrousVeh )
        m_throttle = cent->nitrousVeh->m_throttle;
    v35 = Abs(cent->currentState.pos.trDelta);
    speedRatio = v35 / info->engineSndSpeed;
    if ( (float)(speedRatio - 1.0) < 0.0 )
        healthRatio = speedRatio;
    else
        healthRatio = 1.0f;
    if ( (float)(0.0 - speedRatio) < 0.0 )
        damageRatio = healthRatio;
    else
        damageRatio = 0.0f;
    speedMPH = damageRatio;
    *(float *)&wheelEffect = CG_VehGetHealthPercentageEntity(cent);
    engineRPMs = 1.0 - *(float *)&wheelEffect;
    dydx = v35 / 17.6;
    wheelEffects = cent->vehicle->wheelEffects;
    if ( wheelEffects->surfaceType == -1 )
    {
        idleLoopModulation = (float)((float)(info->gear2MaxRPMs - info->idleRPMs) * m_throttle) + info->idleRPMs;
    }
    else if ( cent->nitrousVeh && m_throttle == 0.0 )
    {
        idleLoopModulation = info->idleRPMs;
    }
    else
    {
        if ( info->gearChangeMPH <= dydx )
        {
            soundOrigin[2] = (float)(info->gear2MaxRPMs - info->gear2MinRPMs) / (float)(info->maxSpeed - info->gearChangeMPH);
            v5 = (float)((float)(dydx - info->gearChangeMPH) * soundOrigin[2]) + info->gear2MinRPMs;
        }
        else
        {
            idleRPMDelta = (float)(info->gear1MaxRPMs - info->gear1MinRPMs) / (float)(info->gearChangeMPH - 0.0);
            v5 = (float)((float)(dydx - 0.0) * idleRPMDelta) + info->gear1MinRPMs;
        }
        idleLoopModulation = v5;
    }
    soundOrigin[1] = idleLoopModulation / info->idleRPMs;
    soundOrigin[0] = info->idleRPMsMax - info->idleRPMs;
    if ( soundOrigin[0] <= 0.0 )
        soundOrigin[0] = 1.0f;
    GetSoundOriginForVehicle(cent, v23);
    v22 = isLocalPlayersVehicle;
    idle_attenuation = (float)(idleLoopModulation - info->idleRPMs) / soundOrigin[0];
    if ( (float)(idle_attenuation - 1.0) < 0.0 )
        v20 = idle_attenuation;
    else
        v20 = 1.0f;
    if ( (float)(0.0 - idle_attenuation) < 0.0 )
        v19 = v20;
    else
        v19 = 0.0f;
    v18 = v19;
    __libm_sse2_cos(v6);
    *(float *)&attenuation = 1.5707964 * v19;
    if ( (float)((float)(1.5707964 * v19) - 1.0) < 0.0 )
        sid = attenuation;
    else
        sid = LODWORD(1.0f);
    if ( (float)(0.0 - *(float *)&attenuation) < 0.0 )
        engineLoopModulation = *(float *)&sid;
    else
        engineLoopModulation = 0.0f;
    engineLoopLoadFactor = engineLoopModulation;
    v13 = CG_PlaySoundWithHandle(
                    localClientNum,
                    cent->nextState.number,
                    v23,
                    0,
                    0,
                    engineLoopModulation,
                    info->sndNames[v22]);
    SND_SetPlaybackPitch(v13, soundOrigin[1]);
    pitch = idleLoopModulation / info->engineModLoopNaturalRPMs;
    loadLevel = (float)(fabs(m_throttle) - speedMPH) + engineRPMs;
    if ( (float)(loadLevel - 1.0) < 0.0 )
        noLoadLevel = loadLevel;
    else
        noLoadLevel = 1.0f;
    if ( (float)(0.0 - loadLevel) < 0.0 )
        sndNoLoadNameIndex = LODWORD(noLoadLevel);
    else
        sndNoLoadNameIndex = 0;
    SND_EqualPowerFadeCoefs(*(float *)&sndNoLoadNameIndex, &v7, (float *)&sndLoadNameIndex);
    engineLoopLoadFactor = v7 * v18;
    v13 = CG_PlaySoundWithHandle(
                    localClientNum,
                    cent->nextState.number,
                    v23,
                    0,
                    0,
                    v7 * v18,
                    info->sndNames[isLocalPlayersVehicle + 7]);
    SND_SetPlaybackPitch(v13, pitch);
    engineLoopLoadFactor = *(float *)&sndLoadNameIndex * v18;
    v13 = CG_PlaySoundWithHandle(
                    localClientNum,
                    cent->nextState.number,
                    v23,
                    0,
                    0,
                    *(float *)&sndLoadNameIndex * v18,
                    info->sndNames[isLocalPlayersVehicle + 9]);
    SND_SetPlaybackPitch(v13, pitch);
}
#endif

void CG_UpdateVehicleEngineSounds(
    int localClientNum,
    centity_s *cent,
    const vehicle_info_t *info,
    bool isLocalPlayersVehicle)
{
    float throttle;
    float speedIPS;
    float speedMPH;
    float speedRatio;
    float healthRatio;
    float engineRPMs;
    float rpm;
    float rpmScale;
    float pitch;
    float idleAtten;
    float loopVolume;
    float loadLevel;
    float noLoadCoef, loadCoef;

    float soundOrigin[3];
    cgVehicleWheelEffect *wheelEffects;

    // ---- throttle ----
    throttle = cent->nextState.lerp.u.vehicle.throttle * (1.0f / 32768.0f);
    if (cent->nitrousVeh)
        throttle = cent->nitrousVeh->m_throttle;

    // ---- speed ----
    speedIPS = Vec3Length(cent->currentState.pos.trDelta);//fabsf(cent->currentState.pos.trDelta);
    speedRatio = speedIPS / info->engineSndSpeed;
    if (speedRatio < 0.0f) speedRatio = 0.0f;
    else if (speedRatio > 1.0f) speedRatio = 1.0f;

    speedMPH = speedIPS / 17.6f;

    // ---- health / damage ----
    healthRatio = CG_VehGetHealthPercentageEntity(cent);
    engineRPMs = 1.0f - healthRatio;

    wheelEffects = cent->vehicle->wheelEffects;

    // ---- RPM calculation ----
    if (wheelEffects->surfaceType == -1)
    {
        rpm = ((info->gear2MaxRPMs - info->idleRPMs) * throttle)
            + info->idleRPMs;
    }
    else if (cent->nitrousVeh && throttle == 0.0f)
    {
        rpm = info->idleRPMs;
    }
    else if (speedMPH >= info->gearChangeMPH)
    {
        float slope =
            (info->gear2MaxRPMs - info->gear2MinRPMs) /
            (info->maxSpeed - info->gearChangeMPH);

        rpm = ((speedMPH - info->gearChangeMPH) * slope)
            + info->gear2MinRPMs;
    }
    else
    {
        float slope =
            (info->gear1MaxRPMs - info->gear1MinRPMs) /
            info->gearChangeMPH;

        rpm = (speedMPH * slope) + info->gear1MinRPMs;
    }

    // ---- pitch scaling ----
    rpmScale = rpm / info->idleRPMs;
    pitch = rpm / info->engineModLoopNaturalRPMs;

    // ---- sound origin ----
    GetSoundOriginForVehicle(cent, soundOrigin);

    // ---- idle attenuation ----
    {
        float denom = info->idleRPMsMax - info->idleRPMs;
        if (denom <= 0.0f)
            denom = 1.0f;

        idleAtten = (rpm - info->idleRPMs) / denom;
    }

    if (idleAtten < 0.0f) idleAtten = 0.0f;
    else if (idleAtten > 1.0f) idleAtten = 1.0f;

    loopVolume = cosf(idleAtten * 1.5707964f);
    if (loopVolume < 0.0f) loopVolume = 0.0f;
    else if (loopVolume > 1.0f) loopVolume = 1.0f;

    // ---- idle loop ----
    int handle = CG_PlaySoundWithHandle(
        localClientNum,
        cent->nextState.number,
        soundOrigin,
        0,
        0,
        loopVolume,
        info->sndNames[isLocalPlayersVehicle]);

    SND_SetPlaybackPitch(handle, rpmScale);

    // ---- load vs no-load blend ----
    loadLevel = (fabsf(throttle) - speedRatio) + engineRPMs;
    if (loadLevel < 0.0f) loadLevel = 0.0f;
    else if (loadLevel > 1.0f) loadLevel = 1.0f;

    SND_EqualPowerFadeCoefs(loadLevel, &noLoadCoef, &loadCoef);

    // ---- no-load loop ----
    handle = CG_PlaySoundWithHandle(
        localClientNum,
        cent->nextState.number,
        soundOrigin,
        0,
        0,
        noLoadCoef * idleAtten,
        info->sndNames[isLocalPlayersVehicle + 7]);

    SND_SetPlaybackPitch(handle, pitch);

    // ---- load loop ----
    handle = CG_PlaySoundWithHandle(
        localClientNum,
        cent->nextState.number,
        soundOrigin,
        0,
        0,
        loadCoef * idleAtten,
        info->sndNames[isLocalPlayersVehicle + 9]);

    SND_SetPlaybackPitch(handle, pitch);
}


void __cdecl CG_UpdateTurretAndOverheatSounds(
                int localClientNum,
                centity_s *cent,
                const vehicle_info_t *info,
                bool isLocalPlayersVehicle)
{
    const FxEffectDef *viewLastShotEjectEffect; // [esp+8h] [ebp-90h]
    char *overheatSoundPlayer; // [esp+Ch] [ebp-8Ch]
    char *rotateLoopSoundPlayer; // [esp+10h] [ebp-88h]
    float v7; // [esp+14h] [ebp-84h]
    float v8; // [esp+18h] [ebp-80h]
    float v9; // [esp+1Ch] [ebp-7Ch]
    bool v10; // [esp+23h] [ebp-75h]
    float v11; // [esp+24h] [ebp-74h]
    float v12; // [esp+28h] [ebp-70h]
    unsigned __int16 flash; // [esp+30h] [ebp-68h]
    float gunPitch; // [esp+3Ch] [ebp-5Ch]
    float gunYaw; // [esp+44h] [ebp-54h]
    float yawMax; // [esp+48h] [ebp-50h]
    float pitchMax; // [esp+4Ch] [ebp-4Ch]
    float yawMin; // [esp+54h] [ebp-44h]
    float pitchLockFade; // [esp+58h] [ebp-40h]
    float pitchMin; // [esp+64h] [ebp-34h]
    float yawLockFade; // [esp+6Ch] [ebp-2Ch]
    int overheatFlag; // [esp+7Ch] [ebp-1Ch]
    int vehSeat; // [esp+80h] [ebp-18h]
    char isOverheating; // [esp+87h] [ebp-11h]
    const WeaponDef *pWeapon; // [esp+88h] [ebp-10h]
    bool gun_turning_pitch; // [esp+8Ch] [ebp-Ch]
    bool gun_turning_yaw; // [esp+8Dh] [ebp-Bh]
    char wasOverheating; // [esp+8Fh] [ebp-9h]
    unsigned int i; // [esp+90h] [ebp-8h]
    const cg_s *cgameGlob; // [esp+94h] [ebp-4h]

    if ( (playTurretRotationSounds || playOverheatSounds) && cent->nitrousVeh )
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        for ( i = 0; i < 2; ++i )
        {
            vehSeat = seat_positions[i];
            if ( vehSeat
                && (vehSeat < 1 || vehSeat > 4)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_vehicle.cpp",
                            1402,
                            0,
                            "%s",
                            "( vehSeat == VEH_POS_DRIVER ) || VEH_GUNNER_POSITION( vehSeat )") )
            {
                __debugbreak();
            }
            v10 = cgameGlob->bgs.clientinfo[cgameGlob->clientNum].attachedVehSeat == vehSeat && isLocalPlayersVehicle;
            gun_turning_yaw = CG_VehGetSeatGunTurningYaw(cent, vehSeat);
            gun_turning_pitch = CG_VehGetSeatGunTurningPitch(cent, vehSeat);
            overheatFlag = overheat_flags[i];
            wasOverheating = (overheatFlag & cent->nitrousVeh->m_sfx_state_flags) != 0;
            isOverheating = 0;
            if ( v10 )
            {
                if ( vehSeat )
                {
                    if ( vehSeat >= 1 && vehSeat <= 4 )
                        isOverheating = BG_PlayerWeaponOverheating(
                                                            &cgameGlob->predictedPlayerState,
                                                            *(unsigned __int16 *)&info->gunnerWeapon[3][2 * vehSeat + 62]);
                }
                else
                {
                    isOverheating = cent->nextState.lerp.u.turret.overheating != 0;
                }
            }
            else
            {
                isOverheating = CG_VehGetSeatGunOverheating(cent, vehSeat);
            }
            if ( isOverheating != wasOverheating || gun_turning_yaw || gun_turning_pitch )
            {
                if ( vehSeat )
                {
                    pWeapon = BG_GetWeaponDef(*(unsigned __int16 *)&info->gunnerWeapon[3][2 * vehSeat + 62]);
                    if ( !pWeapon
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_vehicle.cpp",
                                    1458,
                                    0,
                                    "%s",
                                    "pWeapon != NULL") )
                    {
                        __debugbreak();
                    }
                }
                else
                {
                    pWeapon = BG_GetWeaponDef(cent->nextState.weapon);
                    if ( !pWeapon
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_vehicle.cpp",
                                    1452,
                                    0,
                                    "%s",
                                    "pWeapon != NULL") )
                    {
                        __debugbreak();
                    }
                }
                if ( (gun_turning_yaw || gun_turning_pitch) && playTurretRotationSounds )
                {
                    if ( vehSeat )
                    {
                        gunYaw = (float)cent->currentState.u.vehicle.gunnerAngles[vehSeat - 1].yaw * 0.0054931641;
                        yawMin = -pWeapon->rightArc;
                        yawMax = pWeapon->leftArc;
                        gunPitch = (float)*(__int16 *)&cent->currentState.u.primaryLight.colorAndExp[4 * vehSeat + 4] * 0.0054931641;
                        pitchMin = -pWeapon->topArc;
                        pitchMax = pWeapon->bottomArc;
                    }
                    else
                    {
                        gunYaw = (float)cent->currentState.u.vehicle.gunYaw * 0.0054931641;
                        yawMin = -info->turretViewLimits.horizSpanRight;
                        yawMax = info->turretViewLimits.horizSpanLeft;
                        gunPitch = (float)cent->currentState.u.vehicle.gunPitch * 0.0054931641;
                        pitchMin = -info->turretViewLimits.vertSpanUp;
                        pitchMax = info->turretViewLimits.vertSpanDown;
                    }
                    yawLockFade = 1.0f;
                    pitchLockFade = 1.0f;
                    if ( (float)(yawMin + 3.0) <= gunYaw )
                    {
                        if ( gunYaw > (float)(yawMax - 3.0) )
                            yawLockFade = (float)(yawMax - gunYaw) / 3.0;
                    }
                    else
                    {
                        yawLockFade = (float)(gunYaw - yawMin) / 3.0;
                    }
                    if ( (float)(pitchMin + 3.0) <= gunPitch )
                    {
                        if ( gunPitch > (float)(pitchMax - 3.0) )
                            pitchLockFade = (float)(pitchMax - gunPitch) / 3.0;
                    }
                    else
                    {
                        pitchLockFade = (float)(gunPitch - pitchMin) / 3.0;
                    }
                    if ( gun_turning_yaw )
                        v12 = 1.0f;
                    else
                        v12 = 0.0f;
                    if ( (float)(yawLockFade - v12) < 0.0 )
                        v9 = yawLockFade;
                    else
                        v9 = v12;
                    if ( gun_turning_pitch )
                        v11 = 1.0f;
                    else
                        v11 = 0.0f;
                    if ( (float)(pitchLockFade - v11) < 0.0 )
                        v8 = pitchLockFade;
                    else
                        v8 = v11;
                    if ( (float)(v9 - v8) < 0.0 )
                        v7 = v8;
                    else
                        v7 = v9;
                    if ( v10 )
                        rotateLoopSoundPlayer = (char *)pWeapon->rotateLoopSoundPlayer;
                    else
                        rotateLoopSoundPlayer = (char *)pWeapon->rotateLoopSound;
                    if ( rotateLoopSoundPlayer )
                        CG_PlaySound(localClientNum, cent->nextState.number, cent->pose.origin, 0, 0, v7, rotateLoopSoundPlayer);
                }
                if ( playOverheatSounds )
                {
                    if ( isOverheating && !wasOverheating )
                    {
                        if ( v10 )
                            overheatSoundPlayer = (char *)pWeapon->overheatSoundPlayer;
                        else
                            overheatSoundPlayer = (char *)pWeapon->overheatSound;
                        CG_PlaySound(localClientNum, cent->nextState.number, cent->pose.origin, 0, 0, 1.0, overheatSoundPlayer);
                        flash = scr_const.tag_flash;
                        if ( vehSeat >= 1 && vehSeat <= 4 )
                            flash = *(&scr_const.tag_gunner_turret4 + vehSeat);
                        if ( v10 )
                            viewLastShotEjectEffect = pWeapon->viewLastShotEjectEffect;
                        else
                            viewLastShotEjectEffect = pWeapon->worldLastShotEjectEffect;
                        if ( viewLastShotEjectEffect )
                            CG_PlayBoltedEffect(localClientNum, viewLastShotEjectEffect, cent->nextState.number, flash);
                    }
                    if ( isOverheating )
                        cent->nitrousVeh->m_sfx_state_flags |= overheatFlag;
                    else
                        cent->nitrousVeh->m_sfx_state_flags &= ~overheatFlag;
                }
            }
        }
    }
}

