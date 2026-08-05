#include "nitrous_vehicle.h"
#include <xanim/dobj.h>
#include <bgame/bg_public.h>
#include <game/g_scr_vehicle.h>
#include <qcommon/dobj_management.h>
#include <xanim/dobj_utils.h>
#include <physics/phys_assert.h>
#include <client_mp/cl_main_mp.h>
#include <cgame_mp/cg_actors_mp.h>
#include <cgame_mp/cg_ents_mp.h>
#include <cgame_mp/cg_vehicles_mp.h>
#include <physics/physpreset_load_obj.h>
#include <physics/phys_broad_phase.h>
#include <physics/physics_system.h>
#include <bgame/bg_slidemove.h>
#include <physics/phys_util.h>
#include <clientscript/cscr_stringlist.h>
#include <ragdoll/ragdoll_update.h>
#include <clientscript/scr_const.h>
#include <game_mp/g_utils_mp.h>
#include "nitrous_vehicle_constraint.h"
#include "nitrous_vehicle_effects.h"
#include <sound/snd_dvar.h>
#include <server/sv_world.h>
#include <physics/phys_render.h>
#include <game/g_debug.h>
#include <game_mp/g_main_mp.h>
#include <server_mp/sv_main_mp.h>
#include <universal/com_math_anglevectors.h>
#include <DynEntity/DynEntity_client.h>
#include <client/cl_debugdata.h>
#include <cgame/cg_drawtools.h>
#include <xanim/xanim_clientnotify.h>
#include <cgame_mp/cg_local_mp.h>
#include <bgame/bg_misc.h>

float S_HEAVY_TILT_FAKEY = 0.40000001;
float S_HEAVY_TANK_ACCEL = 15.0;
float S_HEAVY_TILT_FAKEY_0 = 0.40000001;
float S_HEAVY_TANK_ACCEL_0 = 15.0;
float S_T34_DAMP = 3.0;
float wheelSeperation = 8.5;
float tempRadiusPercent = 0.5;
float avel_threshold = 0.30000001;
float vel_threshold = 10.0;
int scale_max_speed = 1;
float percent_to_give = 1.5;
float low_end_speed = 300.0;
float climbHeavyAccelAdd = 120.0;
float pitchFakeyZone = 0.1;
float fwd_fric_scale = 1.0;
float side_fric_scale = 1.0;
float hand_brake_friction_time = 0.30000001;
bool render_wheel_cir = true;
float sphere_size = 5.0;
int duration = 100;
float acceptablePosErrorBase = 10.0;
float acceptableAVelError = 0.1;
float acceptableVelError = 3.0;
float timeCap = 0.80000001;
float errorAReductionRate = 2.0;
float errorReductionRate = 4.0;

phys_vec3 gWheelAxisLoc = { 0.0f, 1.0f, 0.0f };

// *WARNING* One or more selections were skipped as they could not be interpreted as c data

float mass_z_offset = 20.0f;

VehicleParameter g_default_params;
phys_free_list<NitrousVehicle> g_rb_vehicle_list;


VehicleParameter::VehicleParameter()
{
    float *m_buoyancybox_max; // ecx

    this->m_speed_max = 1100.0f;
    this->m_accel_max = 300.0f;
    this->m_reverse_scale = 0.8f;
    this->m_steer_angle_max = 0.6f;
    this->m_steer_speed = 5.0f;
    this->m_wheel_radius = 15.0f;
    this->m_susp_spring_k = 20.0f;
    this->m_susp_damp_k = 1.0f;
    this->m_susp_adj = 10.0f;
    this->m_susp_hard_limit = 23.0f;
    this->m_susp_min_height = -100.0f;
    this->m_tire_fric_fwd = 2.5f;
    this->m_tire_fric_side = 2.3f;
    this->m_tire_fric_brake = 2.3f;
    this->m_tire_fric_hand_brake = 0.0f;
    this->m_body_mass = 1.0f;
    this->m_roll_stability = 10.0f;
    this->m_pitch_roll_resistance = 50.0f;
    this->m_yaw_resistance = 0.0f;
    this->m_upright_strength = 50.0f;
    this->m_tilt_fakey = 0.25f;
    this->m_peel_out_max_speed = 150.0f;
    this->m_tire_damp_coast = 10.0f;
    this->m_tire_damp_brake = 100.0f;
    this->m_tire_damp_hand = 10000.0f;
    this->m_traction_type = TRACTION_TYPE_ALL_WD;
    this->m_water_ebrake_power = 1.5f;
    this->m_tire_fric_side_max = 0.0f;
    this->m_bbox_min[0] = 0.0f;
    this->m_bbox_min[1] = 0.0f;
    this->m_bbox_min[2] = 0.0f;
    this->m_bbox_max[0] = 0.0f;
    this->m_bbox_max[1] = 0.0f;
    this->m_bbox_max[2] = 0.0f;
    this->m_mass_center_offset[0] = 0.0f;
    this->m_mass_center_offset[1] = 0.0f;
    this->m_mass_center_offset[2] = 0.0f;
    this->m_buoyancybox_min[0] = 0.0f;
    this->m_buoyancybox_min[1] = 0.0f;
    this->m_buoyancybox_min[2] = 0.0f;
    this->m_buoyancybox_max[0] = 0.0f;
    m_buoyancybox_max = this->m_buoyancybox_max;
    m_buoyancybox_max[1] = 0.0f;
    m_buoyancybox_max[2] = 0.0f;
    this->m_mass_center_offset[2] = mass_z_offset;
}

NitrousVehicle::NitrousVehicle()
{
    int k; // [esp+18h] [ebp-10h]
    int v3; // [esp+1Ch] [ebp-Ch]
    phys_mat44 *j; // [esp+20h] [ebp-8h]
    int i; // [esp+24h] [ebp-4h]

    v3 = 6;
    for ( j = this->m_wheel_orig_relpo; --v3 >= 0; ++j )
        ;
    this->m_parameter = 0;
    this->m_script_brake = 0.0f;
    this->m_hand_brake_friction_time = 0.0f;
    this->m_owner = 0;
    this->m_entnum = 0;
    this->m_vehicle_info = 0;
    this->m_phys_user_data = 0;
    this->m_xmodel = 0;
    this->m_orientation_constraint = 0;
    this->m_vpc = 0;
    this->m_notify_flags = 0;
    this->m_server_notify_flags = 0;
    for ( k = 0; k < 2; ++k )
        this->mVehicleController.m_cmd.button_bits.array[k] = 0;
    this->m_fake_rpm = 0.0f;
    this->m_trackDistAccumLeftInches = 0;
    this->m_trackDistAccumRightInches = 0;
    this->m_sfx_state_flags = 0;
    this->m_flags = 0;
    this->refcount = 0;
    this->m_state_flags = 0;
    this->m_flags |= 1u;
    this->m_flags &= ~2u;
    for ( i = 0; i < 6; ++i )
        this->m_wheels[i] = 0;
}

void NitrousVehicle::init(gentity_s *owner, const VehicleParameter *parameter)
{
    DObj *ServerDObj; // eax
    float axis[9]; // [esp+28h] [ebp-44h] BYREF
    phys_vec3 *p_m_ground_vel; // [esp+4Ch] [ebp-20h]
    float *m_networkErrorAngles; // [esp+50h] [ebp-1Ch]
    float *m_networkErrorOrigin; // [esp+54h] [ebp-18h]
    float *m_angles; // [esp+58h] [ebp-14h]
    float *currentAngles; // [esp+5Ch] [ebp-10h]
    float *m_origin; // [esp+60h] [ebp-Ch]
    float *currentOrigin; // [esp+64h] [ebp-8h]
    int i; // [esp+68h] [ebp-4h]
    int savedregs; // [esp+6Ch] [ebp+0h] BYREF

    if ( !parameter )
        parameter = &g_default_params;
    //NitrousVehicle::pause_physics(this, 0);
    this->pause_physics(false);
    this->m_owner = owner;
    this->m_entnum = owner->s.number;
    this->m_vehicle_info = BG_GetVehicleInfo(owner->scr_vehicle->infoIdx);
    ServerDObj = Com_GetServerDObj(this->m_entnum);
    this->m_xmodel = DObjGetModel(ServerDObj, 0);
    this->m_parameter = (VehicleParameter*)parameter;
    this->m_hand_brake = 0.0f;
    this->m_brake = 0.0f;
    //NitrousVehicle::set_throttle(this, 0.0);
    this->set_throttle(0.0f);
    this->m_steer_factor = 0.0f;
    this->m_forward_vel = 0.0f;
    this->m_script_brake = 0.0f;
    this->m_stuck_time = 0.0f;
    m_origin = this->m_origin;
    currentOrigin = owner->r.currentOrigin;
    this->m_origin[0] = owner->r.currentOrigin[0];
    m_origin[1] = currentOrigin[1];
    m_origin[2] = currentOrigin[2];
    m_angles = this->m_angles;
    currentAngles = owner->r.currentAngles;
    this->m_angles[0] = owner->r.currentAngles[0];
    m_angles[1] = currentAngles[1];
    m_angles[2] = currentAngles[2];
    m_networkErrorOrigin = this->m_networkErrorOrigin;
    this->m_networkErrorOrigin[0] = 0.0f;
    m_networkErrorOrigin[1] = 0.0f;
    m_networkErrorOrigin[2] = 0.0f;
    m_networkErrorAngles = this->m_networkErrorAngles;
    this->m_networkErrorAngles[0] = 0.0f;
    m_networkErrorAngles[1] = 0.0f;
    m_networkErrorAngles[2] = 0.0f;
    p_m_ground_vel = &this->m_ground_vel;
    this->m_ground_vel.x = 0.0f;
    p_m_ground_vel->y = 0.0f;
    p_m_ground_vel->z = 0.0f;
    this->m_lastErrorReductionTime = 0;
    this->m_speed_scale = 1.0f;
    this->m_drivepath_scale = 1.0f;
    this->m_boost_start_time = 0;
    this->m_boost_time_pool = this->m_vehicle_info->boostDuration;
    this->m_throttle_held_down = 0;
    this->m_throttle_time_started = 0;
    this->m_flags &= ~0x100u;
    this->m_flags &= ~0x80u;
    this->m_flags &= ~4u;
    AnglesToAxis(this->m_angles, (float (*)[3])axis);
    Phys_AxisToNitrousMat((float (*)[3])axis, &this->m_prev_rb_mat);
    Phys_Vec3ToNitrousVec(this->m_origin, &this->m_prev_rb_mat.w);
    if ( fabs(this->m_prev_rb_mat.w.x) > 100000.0
        || fabs(this->m_prev_rb_mat.w.y) > 100000.0
        || fabs(this->m_prev_rb_mat.w.z) > 100000.0 )
    {
        phys_exec_debug_callback(0);
    }
    //NitrousVehicle::_setup_wheels(this, (DObj *)&savedregs, 0);
    this->_setup_wheels(0);
    for ( i = 0; i < 4; ++i )
    {
        this->m_wheel_state[i].m_state = WHEEL_STATE_AIRBORN;
        this->m_wheel_state[i].m_last_state = WHEEL_STATE_AIRBORN;
        this->m_wheel_state[i].m_state_time = 0.0f;
        this->m_wheel_state[i].m_rate = 0.0f;
    }
}

void NitrousVehicle::set_throttle(float throttle)
{
    if ( (throttle < -1.0 || throttle > 1.0)
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\vehicle\\nitrous_vehicle.h",
                    453,
                    0,
                    "%s\n\t%s",
                    "throttle>= -1.0f && throttle<= 1.0f",
                    "Out of range") )
    {
        __debugbreak();
    }
    this->m_throttle = throttle;
}

void NitrousVehicle::init(
                int localClientNum,
                centity_s *owner,
                const VehicleParameter *parameter)
{
    float axis[9]; // [esp+1Ch] [ebp-4Ch] BYREF
    phys_vec3 *p_m_ground_vel; // [esp+40h] [ebp-28h]
    float *m_networkErrorAngles; // [esp+44h] [ebp-24h]
    float *m_networkErrorOrigin; // [esp+48h] [ebp-20h]
    float *m_angles; // [esp+4Ch] [ebp-1Ch]
    float *angles; // [esp+50h] [ebp-18h]
    float *m_origin; // [esp+54h] [ebp-14h]
    float *origin; // [esp+58h] [ebp-10h]
    int i; // [esp+5Ch] [ebp-Ch]
    const DObj *obj; // [esp+60h] [ebp-8h]
    const cgs_t *cgs; // [esp+64h] [ebp-4h]
    int savedregs; // [esp+68h] [ebp+0h] BYREF

    if ( this->refcount <= 1 )
    {
        if ( !parameter )
            parameter = &g_default_params;
        cgs = CG_GetLocalClientStaticGlobals(localClientNum);
        obj = CG_PreProcess_GetDObj(
                        localClientNum,
                        owner->nextState.number,
                        owner->nextState.eType,
                        cgs->gameModels[owner->nextState.un3.item],
                        0);
        if ( !obj
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\vehicle\\nitrous_vehicle.cpp", 188, 0, "%s", "obj") )
        {
            __debugbreak();
        }
        this->m_xmodel = DObjGetModel(obj, 0);
        this->m_owner = 0;
        this->m_entnum = owner->nextState.number;
        this->m_vehicle_info = CG_GetVehicleInfo(owner->nextState.vehicleState.vehicleInfoIndex);
        this->m_parameter = (VehicleParameter*)parameter;
        this->m_hand_brake = 0.0f;
        this->m_brake = 0.0f;
        //NitrousVehicle::set_throttle(this, 0.0);
        this->set_throttle(0.0f);
        this->m_steer_factor = 0.0f;
        this->m_forward_vel = 0.0f;
        this->m_script_brake = 0.0f;
        this->m_stuck_time = 0.0f;
        m_origin = this->m_origin;
        origin = owner->pose.origin;
        this->m_origin[0] = owner->pose.origin[0];
        m_origin[1] = origin[1];
        m_origin[2] = origin[2];
        m_angles = this->m_angles;
        angles = owner->pose.angles;
        this->m_angles[0] = owner->pose.angles[0];
        m_angles[1] = angles[1];
        m_angles[2] = angles[2];
        m_networkErrorOrigin = this->m_networkErrorOrigin;
        this->m_networkErrorOrigin[0] = 0.0f;
        m_networkErrorOrigin[1] = 0.0f;
        m_networkErrorOrigin[2] = 0.0f;
        m_networkErrorAngles = this->m_networkErrorAngles;
        this->m_networkErrorAngles[0] = 0.0f;
        m_networkErrorAngles[1] = 0.0f;
        m_networkErrorAngles[2] = 0.0f;
        p_m_ground_vel = &this->m_ground_vel;
        this->m_ground_vel.x = 0.0f;
        p_m_ground_vel->y = 0.0f;
        p_m_ground_vel->z = 0.0f;
        this->m_speed_scale = 1.0f;
        this->m_drivepath_scale = 1.0f;
        this->m_boost_start_time = 0;
        this->m_boost_time_pool = this->m_vehicle_info->boostDuration;
        this->m_throttle_held_down = 0;
        AnglesToAxis(this->m_angles, (float (*)[3])axis);
        Phys_AxisToNitrousMat((float (*)[3])axis, &this->m_prev_rb_mat);
        Phys_Vec3ToNitrousVec(this->m_origin, &this->m_prev_rb_mat.w);
        if ( fabs(this->m_prev_rb_mat.w.x) > 100000.0
            || fabs(this->m_prev_rb_mat.w.y) > 100000.0
            || fabs(this->m_prev_rb_mat.w.z) > 100000.0 )
        {
            phys_exec_debug_callback(0);
        }
        //NitrousVehicle::_setup_wheels(this, (DObj *)&savedregs, localClientNum);
        this->_setup_wheels(localClientNum);
        for ( i = 0; i < 4; ++i )
        {
            this->m_wheel_state[i].m_state = WHEEL_STATE_AIRBORN;
            this->m_wheel_state[i].m_last_state = WHEEL_STATE_AIRBORN;
            this->m_wheel_state[i].m_state_time = 0.0f;
            this->m_wheel_state[i].m_rate = 0.0f;
        }
    }
}

PhysObjUserData *__cdecl Phys_ObjCreateNitrousVehicle(
                const float *position,
                const float *angles,
                const float *velocity,
                const float *translation,
                const PhysPreset *physPreset,
                gjk_geom_list_t *gjk_geom_list)
{
    double v6; // xmm0_8
    BodyState state; // [esp+18h] [ebp-B0h] BYREF
    float axis[3][3]; // [esp+A4h] [ebp-24h] BYREF
    int savedregs; // [esp+C8h] [ebp+0h] BYREF

    nanassertvec3(position);
    nanassertvec3(velocity);

    if ( !physPreset
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\vehicle\\nitrous_vehicle.cpp", 235, 0, "%s", "physPreset") )
    {
        __debugbreak();
    }
    AnglesToAxis(angles, axis);
    AxisCopy(axis, state.rotation);
    state.position[0] = *position;
    state.position[1] = position[1];
    state.position[2] = position[2];
    state.velocity[0] = *velocity;
    state.velocity[1] = velocity[1];
    state.velocity[2] = velocity[2];
    memset(state.angVelocity, 0, 48);
    state.mass = physPreset->mass;
    state.bounce = physPreset->bounce;
    state.friction = physPreset->friction;
    state.timeLastAsleep = physGlob.timeLastUpdate;
    if ( physPreset->canFloat )
        v6 = phys_gravity->current.value * -1.1;
    else
        v6 = phys_gravity->current.value * -0.9;
    state.buoyancy = (int)v6;
    state.underwater = 0;
    state.id = -1;
    //return Phys_CreateBodyFromState((int)&savedregs, 0, &state, gjk_geom_list, 1);
    return Phys_CreateBodyFromState(0, &state, gjk_geom_list, 1);
}

void    NitrousVehicle::unpause_physics()
{
    PhysObjUserData *v2; // eax
    phys_vec3 *p_m_gravity_acc_vec; // [esp-54h] [ebp-104h]
    float v4; // [esp-48h] [ebp-F8h]
    float v5; // [esp-44h] [ebp-F4h]
    float v6; // [esp-30h] [ebp-E0h]
    float value; // [esp-10h] [ebp-C0h]
    float v8[3]; // [esp-Ch] [ebp-BCh] BYREF
    phys_vec3 gravity_dir; // [esp+0h] [ebp-B0h]
    broad_phase_info *i; // [esp+10h] [ebp-A0h]
    broad_phase_group *bpg; // [esp+14h] [ebp-9Ch]
    int flags; // [esp+18h] [ebp-98h]
    unsigned int m_env_collision_flags; // [esp+1Ch] [ebp-94h]
    broad_phase_info *bpi; // [esp+20h] [ebp-90h]
    gjk_geom_list_t gjk_geom_list; // [esp+2Ch] [ebp-84h] BYREF
    float vel[3]; // [esp+34h] [ebp-7Ch] BYREF
    const VehicleParameter *m_parameter; // [esp+40h] [ebp-70h]
    PhysPreset physPreset; // [esp+44h] [ebp-6Ch] BYREF
    int m_flags; // [esp+9Ch] [ebp-14h]
    NitrousVehicle *thisa; // [esp+A0h] [ebp-10h]
    //_UNKNOWN *v22[2]; // [esp+A4h] [ebp-Ch] BYREF
    //int vars0; // [esp+B0h] [ebp+0h]
    //
    //v22[0] = a2;
    //v22[1] = (_UNKNOWN *)vars0;
    thisa = this;
    if ((this->m_flags & 1) != 0)
    {
        thisa->m_flags &= ~1u;
        m_flags = thisa->m_flags;
        physPreset.bounce = 0.3f;
        physPreset.bulletForceScale = 1.0f;
        physPreset.explosiveForceScale = 1.0f;
        if (thisa->m_vehicle_info->type == 4)
            physPreset.friction = 1.0f;
        else
            physPreset.friction = 0.4f;
        m_parameter = thisa->m_parameter;
        physPreset.mass = m_parameter->m_body_mass;
        physPreset.canFloat = thisa->m_parameter->m_buoyancybox_min[0] != 0.0;
        memset(vel, 0, sizeof(vel));
        gjk_geom_list.m_first_geom = 0;
        gjk_geom_list.m_geom_count = 0;
        create_gjk_geom_collision_visitor collision_visitor; // [esp+24h] [ebp-8Ch] BYREF
        //collision_visitor.__vftable = (create_gjk_geom_collision_visitor_vtbl *)&create_gjk_geom_collision_visitor::`vftable';
        collision_visitor.gjk_geom_list = &gjk_geom_list;
        create_xmodel_gjk_geom(
            thisa->m_xmodel,
            &collision_visitor,
            -1,
            0x280EC93u,
            1,
            0,
            0);
        v2 = Phys_ObjCreateNitrousVehicle(
            thisa->m_origin,
            thisa->m_angles,
            vel,
            thisa->m_parameter->m_mass_center_offset,
            &physPreset,
            &gjk_geom_list);
        thisa->m_phys_user_data = v2;
        thisa->m_phys_user_data->vehicle = thisa;
        if ( thisa->m_phys_user_data->m_bpb->is_bpi() )
        {
            //bpi = broad_phase_base::get_bpi(thisa->m_phys_user_data->m_bpb);
            bpi = thisa->m_phys_user_data->m_bpb->get_bpi();
            m_env_collision_flags = bpi->m_env_collision_flags;
            flags = (m_env_collision_flags | (8 * (thisa->m_vehicle_info->remoteControl != 0) + 8)) & 0xFFFFFF7F;
            bpi->m_env_collision_flags = flags;
        }
        else
        {
            //bpg = broad_phase_base::get_bpg(thisa->m_phys_user_data->m_bpb);
            bpg = thisa->m_phys_user_data->m_bpb->get_bpg();
            for (i = bpg->m_list_bpi_head; i; i = (broad_phase_info *)i->m_list_bpb_next)
            {
                i->m_env_collision_flags = i->m_env_collision_flags | (8 * (thisa->m_vehicle_info->remoteControl != 0) + 8) & 0xFFFFFF7F;
            }
        }
        Phys_Vec3ToNitrousVec(&phys_gravity_dir->current.value, (phys_vec3 *)v8);
        value = phys_gravity->current.value;
        v6 = phys_vehicleGravityMultiplier->current.value;
        v4 = v6 * (float)(value * v8[1]);
        v5 = v6 * (float)(value * v8[2]);
        p_m_gravity_acc_vec = &thisa->m_phys_user_data->body->m_gravity_acc_vec;
        p_m_gravity_acc_vec->x = v6 * (float)(value * v8[0]);
        p_m_gravity_acc_vec->y = v4;
        p_m_gravity_acc_vec->z = v5;
        NitrousVehicle::update_parms(thisa->m_parameter, 1);
        thisa->m_flags |= 2u;
        //NitrousVehicleController::Init(&thisa->mVehicleController);
        this->mVehicleController.Init();
    }
}

void __thiscall NitrousVehicleController::Init()
{
    this->m_stuck_time = 0.0f;
    this->m_stopped_time = 0.0f;
    memset(&this->m_cmd, 0, sizeof(this->m_cmd));
    this->m_cmd_local_client_num = 0;
    this->m_cmd_valid = 1;
    this->m_stop_at_goal = 0;
}

void __thiscall NitrousVehicle::pause_physics(bool shutdown)
{
    if ( (this->m_flags & 1) == 0 )
    {
        Sys_EnterCriticalSection(CRITSECT_PHYSICS_UPDATE);
        if ( !shutdown )
        {
            //phys_mat44::operator=(&this->m_prev_rb_mat, &this->m_phys_user_data->body->m_mat);
            this->m_prev_rb_mat = this->m_phys_user_data->body->m_mat;
            NitrousVehicle::remove_wheels();
            if ( this->m_orientation_constraint )
                phys_sys::destroy(this->m_orientation_constraint);
            this->m_orientation_constraint = 0;
        }
        NitrousVehicle::cleanup_path();
        this->m_flags |= 1u;
        Phys_ObjDestroy(0, (int)this->m_phys_user_data);
        this->m_phys_user_data = 0;
        Sys_LeaveCriticalSection(CRITSECT_PHYSICS_UPDATE);
    }
}

void __thiscall NitrousVehicle::_update_pause()
{
    bool v1; // [esp+0h] [ebp-18h]
    int savedregs; // [esp+18h] [ebp+0h] BYREF

    this->m_flags |= 0x40u;
    v1 = (this->m_flags & 4) != 0 || (this->m_flags & 0x80) != 0 || (this->m_flags & 0x100) != 0;
    if (this->m_phys_user_data)
    {
        //rigid_body::set_flag(this->m_phys_user_data->body, 0x40u, v1 | 1);
        this->m_phys_user_data->body->set_flag(0x40u, v1 | 1);
    }
    if ((this->m_flags & 1) != 0)
    {
        //NitrousVehicle::unpause_physics(this, (int)&savedregs);
        NitrousVehicle::unpause_physics();
    }
}

void __thiscall rigid_body::set_flag(unsigned int f, int b)
{
    if ( (f & 0xFFFFFF00) != 0
        && _tlAssert(
                 "C:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\rigid_body.h",
                 59,
                 "(f & ~(FLAG_CLIENT_FLAGS_START - 1)) == 0",
                 "invalid rigid body flag") )
    {
        __debugbreak();
    }
    if ( b )
        this->m_flags |= f;
    else
        this->m_flags &= ~f;
}

unsigned int __thiscall rigid_body::is_group_stable()
{
    return this->m_flags & 8;
}

void NitrousVehicle::update_parms(
                const VehicleParameter *params,
                bool initialization)
{
    const VehicleParameter *m_parameter; // eax
    double maximum_acceleration; // st7
    float fwd_fric_k; // [esp+8h] [ebp-32Ch]
    float suspension_stiffness_k; // [esp+10h] [ebp-324h]
    float suspension_damp_k; // [esp+14h] [ebp-320h]
    float hard_limit_dist; // [esp+18h] [ebp-31Ch]
    float roll_stability_factor; // [esp+1Ch] [ebp-318h]
    float v11; // [esp+40h] [ebp-2F4h]
    float v12; // [esp+ACh] [ebp-288h]
    float v13; // [esp+B0h] [ebp-284h]
    rigid_body_constraint_wheel *v14; // [esp+E4h] [ebp-250h]
    rigid_body_constraint_wheel *v15; // [esp+100h] [ebp-234h]
    float front_back_length; // [esp+104h] [ebp-230h]
    phys_vec3 front_steer_pt; // [esp+108h] [ebp-22Ch] BYREF
    float v18; // [esp+118h] [ebp-21Ch]
    int v19; // [esp+11Ch] [ebp-218h]
    int v20; // [esp+120h] [ebp-214h]
    unsigned int v21; // [esp+12Ch] [ebp-208h]
    unsigned int v22; // [esp+130h] [ebp-204h]
    unsigned int v24; // [esp+138h] [ebp-1FCh]
    BOOL v25; // [esp+13Ch] [ebp-1F8h]
    TractionType m_traction_type; // [esp+140h] [ebp-1F4h]
    unsigned int v27; // [esp+144h] [ebp-1F0h]
    unsigned int v28; // [esp+148h] [ebp-1ECh]
    unsigned int v30; // [esp+150h] [ebp-1E4h]
    unsigned int v31; // [esp+154h] [ebp-1E0h]
    unsigned int m_wheel_flags; // [esp+15Ch] [ebp-1D8h]
    float side_fric_max; // [esp+160h] [ebp-1D4h]
    float wheel_radius; // [esp+164h] [ebp-1D0h]
    phys_vec3 susp_dir; // [esp+168h] [ebp-1CCh] BYREF
    float susp_damp_k; // [esp+17Ch] [ebp-1B8h]
    float susp_spring_k; // [esp+180h] [ebp-1B4h]
    float mass; // [esp+184h] [ebp-1B0h]
    const VehicleParameter *v40; // [esp+188h] [ebp-1ACh]
    float v41; // [esp+18Ch] [ebp-1A8h]
    float v42; // [esp+190h] [ebp-1A4h]
    float com_spring_factor; // [esp+194h] [ebp-1A0h]
    float dist_to_com; // [esp+198h] [ebp-19Ch]
    float v45; // [esp+19Ch] [ebp-198h]
    float v46; // [esp+1A0h] [ebp-194h]
    float v47; // [esp+1A4h] [ebp-190h]
    phys_vec3 wheel_center_loc; // [esp+1A8h] [ebp-18Ch] BYREF
    float v49; // [esp+1B8h] [ebp-17Ch]
    float v50; // [esp+1BCh] [ebp-178h]
    float v51; // [esp+1C0h] [ebp-174h]
    float v52; // [esp+1C8h] [ebp-16Ch]
    float v53; // [esp+1CCh] [ebp-168h]
    float v54; // [esp+1D0h] [ebp-164h]
    phys_vec3 *v55; // [esp+1D4h] [ebp-160h]
    phys_vec3 translation; // [esp+1D8h] [ebp-15Ch]
    rigid_body_constraint_wheel *rbc_wheel; // [esp+1F0h] [ebp-144h]
    int hasWheel; // [esp+1F4h] [ebp-140h]
    BOOL v59; // [esp+1F8h] [ebp-13Ch]
    float y; // [esp+1FCh] [ebp-138h]
    int i; // [esp+200h] [ebp-134h]
    float wheel_to_wheel_length; // [esp+204h] [ebp-130h]
    float v63[7]; // [esp+208h] [ebp-12Ch] BYREF
    float v64; // [esp+224h] [ebp-110h]
    float v65; // [esp+228h] [ebp-10Ch]
    float v66; // [esp+22Ch] [ebp-108h]
    phys_vec3 *v67; // [esp+230h] [ebp-104h]
    phys_vec3 *p_w; // [esp+234h] [ebp-100h]
    const VehicleParameter *v69; // [esp+238h] [ebp-FCh]
    float maxAvel; // [esp+23Ch] [ebp-F8h]
    rigid_body *rb; // [esp+240h] [ebp-F4h]
    phys_mat44 *p_cg2rb; // [esp+244h] [ebp-F0h]
    phys_vec3 v73; // [esp+248h] [ebp-ECh] BYREF
    int v74; // [esp+25Ch] [ebp-D8h]
    int v75; // [esp+260h] [ebp-D4h]
    int v76; // [esp+264h] [ebp-D0h]
    phys_vec3 _com; // [esp+268h] [ebp-CCh] BYREF
    float *m_mass_center_offset; // [esp+27Ch] [ebp-B8h]
    float v79; // [esp+280h] [ebp-B4h]
    const VehicleParameter *v80; // [esp+284h] [ebp-B0h]
    phys_vec3 phys_vec3_unit_inertia; // [esp+288h] [ebp-ACh] BYREF
    float volume; // [esp+2A4h] [ebp-90h] BYREF
    phys_vec3 phys_dim; // [esp+2A8h] [ebp-8Ch] BYREF
    float dim[3]; // [esp+2C0h] [ebp-74h] BYREF
    float com[4]; // [esp+2CCh] [ebp-68h] BYREF
    float *m_bbox_min; // [esp+2DCh] [ebp-58h]
    float vmax[3]; // [esp+2E0h] [ebp-54h] BYREF
    float vmin[3]; // [esp+2ECh] [ebp-48h] BYREF
    phys_vec3 gjk_geom_list_aabb_mn_loc; // [esp+2F8h] [ebp-3Ch] BYREF
    phys_vec3 gjk_geom_list_aabb_mx_loc; // [esp+308h] [ebp-2Ch] BYREF
    NitrousVehicle *thisa; // [esp+324h] [ebp-10h]
    //_UNKNOWN *v92[2]; // [esp+328h] [ebp-Ch] BYREF
    //int vars0; // [esp+334h] [ebp+0h]
    //
    //v92[0] = a2;
    //v92[1] = (_UNKNOWN *)vars0;
    thisa = this;
    if (params)
        thisa->m_parameter = (VehicleParameter*)params;
    if (phys_heavyTankSwitch->current.enabled)
    {
        if (thisa->m_parameter->m_susp_damp_k == S_T34_DAMP)
        {
            thisa->m_parameter->m_accel_max = S_HEAVY_TANK_ACCEL_0;
            thisa->m_parameter->m_tilt_fakey = S_HEAVY_TILT_FAKEY_0;
        }
        else
        {
            thisa->m_parameter->m_accel_max = S_HEAVY_TANK_ACCEL;
            thisa->m_parameter->m_tilt_fakey = S_HEAVY_TILT_FAKEY;
        }
    }
    if ((thisa->m_flags & 1) != 0)
    {
        NitrousVehicle::unpause_physics();
    }
    else
    {
        if (thisa->m_parameter->m_bbox_min[0] == 0.0)
        {
            this->m_phys_user_data->m_gjk_geom_list.comp_aabb_loc(&gjk_geom_list_aabb_mn_loc, &gjk_geom_list_aabb_mx_loc);
            //gjk_geom_list_t::comp_aabb_loc(
            //    &thisa->m_phys_user_data->m_gjk_geom_list,
            //    (int)v92,
            //    &gjk_geom_list_aabb_mn_loc,
            //    &gjk_geom_list_aabb_mx_loc);
            Phys_NitrousVecToVec3(&gjk_geom_list_aabb_mn_loc, vmin);
            Phys_NitrousVecToVec3(&gjk_geom_list_aabb_mx_loc, vmax);
        }
        else
        {
            m_bbox_min = thisa->m_parameter->m_bbox_min;
            vmin[0] = *m_bbox_min;
            vmin[1] = m_bbox_min[1];
            vmin[2] = m_bbox_min[2];
            m_parameter = thisa->m_parameter;
            //LODWORD(com[3]) = m_parameter->m_bbox_max;
            vmax[0] = m_parameter->m_bbox_max[0];
            vmax[1] = m_parameter->m_bbox_max[1];
            vmax[2] = m_parameter->m_bbox_max[2];
        }
        com[0] = 0.5 * (float)(vmax[0] + vmin[0]);
        com[1] = 0.5 * (float)(vmax[1] + vmin[1]);
        com[2] = 0.5 * (float)(vmax[2] + vmin[2]);
        dim[0] = vmax[0] - vmin[0];
        dim[1] = vmax[1] - vmin[1];
        dim[2] = vmax[2] - vmin[2];
        Phys_Vec3ToNitrousVec(dim, &phys_dim);
        nuge::calc_box_inertia(&phys_dim, &phys_vec3_unit_inertia, &volume);
        v80 = thisa->m_parameter;
        v79 = v80->m_body_mass / volume;
        phys_vec3_unit_inertia.x = phys_vec3_unit_inertia.x * v79;
        phys_vec3_unit_inertia.y = phys_vec3_unit_inertia.y * v79;
        phys_vec3_unit_inertia.z = phys_vec3_unit_inertia.z * v79;
        m_mass_center_offset = thisa->m_parameter->m_mass_center_offset;
        com[0] = com[0] + *m_mass_center_offset;
        com[1] = com[1] + m_mass_center_offset[1];
        com[2] = com[2] + m_mass_center_offset[2];
        Phys_Vec3ToNitrousVec(com, &_com);
        v76 = -(_com.x);
        v75 = -(_com.y);
        v74 = -(_com.z);
        (v73.x) = -(_com.x);
        (v73.y) = -(_com.y);
        (v73.z) = -(_com.z);
        p_cg2rb = &thisa->m_phys_user_data->cg2rb;
        //phys_vec3::operator=(&p_cg2rb->w, &v73);
        p_cg2rb->w = v73;
        rb = thisa->m_phys_user_data->body;
        if (thisa->m_vehicle_info->type == 2)
        {
            maxAvel = thisa->m_vehicle_info->rotRate * 0.017453292;
            rb->m_max_avel = maxAvel;
        }
        else
        {
            rb->m_max_avel = 10.0f;
        }
        v69 = thisa->m_parameter;
        //rigid_body::set_mass(rb, v69->m_body_mass);
        rb->set_mass(v69->m_body_mass);
        //rigid_body::set_inertia(rb, &phys_vec3_unit_inertia);
        rb->set_inertia(&phys_vec3_unit_inertia);
        p_w = &thisa->m_wheel_orig_relpo[2].w;
        v67 = &thisa->m_wheel_orig_relpo[0].w;
        v66 = thisa->m_wheel_orig_relpo[0].w.x - thisa->m_wheel_orig_relpo[2].w.x;
        v65 = thisa->m_wheel_orig_relpo[0].w.y - thisa->m_wheel_orig_relpo[2].w.y;
        v64 = thisa->m_wheel_orig_relpo[0].w.z - thisa->m_wheel_orig_relpo[2].w.z;
        v63[0] = v66;
        v63[1] = v65;
        v63[2] = v64;
        wheel_to_wheel_length = Vec3Length(v63);
        for (i = 0; i < 6; ++i)
        {
            y = thisa->m_wheel_orig_relpo[i].w.y;
            v59 = y != 0.0;
            hasWheel = v59;
            if (y == 0.0)
            {
                thisa->m_wheels[i] = 0;
            }
            else
            {
                rbc_wheel = 0;
                if (initialization)
                {
                    rbc_wheel = phys_sys::create_rbc_wheel((environment_rigid_body *)rb, 0, 0);
                    if (!rbc_wheel
                        && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\vehicle\\nitrous_vehicle.cpp",
                            497,
                            0,
                            "%s\n\t%s",
                            "rbc_wheel",
                            "Ran out of wheel constraints."))
                    {
                        __debugbreak();
                    }
                    thisa->m_wheels[i] = rbc_wheel;
                }
                else
                {
                    rbc_wheel = thisa->m_wheels[i];
                }
                translation = thisa->m_phys_user_data->cg2rb.w;
                v55 = &thisa->m_wheel_orig_relpo[i].w;
                v54 = v55->x + translation.x;
                v53 = thisa->m_wheel_orig_relpo[i].w.y + translation.y;
                v52 = thisa->m_wheel_orig_relpo[i].w.z + translation.z;
                v49 = v54;
                v50 = v53;
                v51 = v52;
                wheel_center_loc.x = v54;
                wheel_center_loc.y = v53;
                wheel_center_loc.z = v52;
                v47 = v52;
                v46 = (float)(v52 - thisa->m_parameter->m_susp_adj) + thisa->m_parameter->m_wheel_radius;
                wheel_center_loc.z = v46;
                v45 = v54;
                //LODWORD(dist_to_com) = LODWORD(v54) & _mask__AbsFloat_;
                (dist_to_com) = fabs(v54);
                //com_spring_factor = COERCE_FLOAT(LODWORD(v54) & _mask__AbsFloat_) / wheel_to_wheel_length;
                com_spring_factor = (fabs(v54)) / wheel_to_wheel_length;
                if ((float)(com_spring_factor - 1.0) < 0.0)
                    v42 = com_spring_factor;
                else
                    v42 = 1.0f;
                if ((float)(0.0 - com_spring_factor) < 0.0)
                    v41 = v42;
                else
                    v41 = 0.0f;
                com_spring_factor = (float)(1.0 - v41) * 2.0;
                v40 = thisa->m_parameter;
                mass = v40->m_body_mass;
                susp_spring_k = (float)((float)(thisa->m_parameter->m_susp_spring_k
                    * phys_vehicleGravityMultiplier->current.value)
                    * com_spring_factor)
                    * mass;
                susp_damp_k = (float)((float)(thisa->m_parameter->m_susp_damp_k * phys_vehicleGravityMultiplier->current.value)
                    * com_spring_factor)
                    * mass;
                susp_dir.x = 0.0f;
                susp_dir.y = 0.0f;
                susp_dir.z = -1.0f;
                wheel_radius = thisa->m_parameter->m_wheel_radius;
                if (thisa->m_parameter->m_tire_fric_side_max == 0.0)
                    side_fric_max = FLT_MAX;
                else
                    side_fric_max = thisa->m_parameter->m_tire_fric_side_max;
                //rigid_body_constraint_wheel::set(
                    rbc_wheel->set(
                    &wheel_center_loc,
                    &susp_dir,
                    &gWheelAxisLoc,
                    wheel_radius,
                    thisa->m_parameter->m_tire_fric_fwd / phys_vehicleGravityMultiplier->current.value,
                    thisa->m_parameter->m_tire_fric_side / phys_vehicleGravityMultiplier->current.value,
                    susp_spring_k,
                    susp_damp_k,
                    thisa->m_parameter->m_susp_hard_limit,
                    thisa->m_parameter->m_roll_stability,
                    thisa->m_parameter->m_roll_stability,
                    side_fric_max);
                if (thisa->m_vehicle_info->type == 2 || thisa->m_vehicle_info->isFourWheelSteering)
                {
                    rbc_wheel->m_wheel_flags |= 8u;
                    m_wheel_flags = rbc_wheel->m_wheel_flags;
                }
                else
                {
                    if ((unsigned int)i < 2 || i == 4 || i == 5)
                        rbc_wheel->m_wheel_flags |= 8u;
                    else
                        rbc_wheel->m_wheel_flags &= ~8u;
                    v31 = rbc_wheel->m_wheel_flags;
                }
                rbc_wheel->m_wheel_flags &= ~0x80u;
                v30 = rbc_wheel->m_wheel_flags;
                if (i == 2 || i == 3)
                    rbc_wheel->m_wheel_flags |= 0x20u;
                else
                    rbc_wheel->m_wheel_flags &= ~0x20u;
                v28 = rbc_wheel->m_wheel_flags;
                rbc_wheel->m_wheel_flags |= 0x40u;
                v27 = rbc_wheel->m_wheel_flags;
                m_traction_type = thisa->m_parameter->m_traction_type;
                if (m_traction_type)
                {
                    if (m_traction_type == TRACTION_TYPE_BACK)
                    {
                        if (i == 2 || i == 3)
                            rbc_wheel->m_wheel_flags |= 0x10u;
                        else
                            rbc_wheel->m_wheel_flags &= ~0x10u;
                        v22 = rbc_wheel->m_wheel_flags;
                    }
                    else if (m_traction_type == TRACTION_TYPE_ALL_WD)
                    {
                        rbc_wheel->m_wheel_flags |= 0x10u;
                        v21 = rbc_wheel->m_wheel_flags;
                    }
                }
                else
                {
                    v25 = (unsigned int)i < 2;
                    if ((unsigned int)i >= 2)
                        rbc_wheel->m_wheel_flags &= ~0x10u;
                    else
                        rbc_wheel->m_wheel_flags |= 0x10u;
                    v24 = rbc_wheel->m_wheel_flags;
                }
            }
        }
        if (thisa->m_vehicle_info->type == 4)
        {
            v18 = 50.0f;
            v19 = 0.0f;
            v20 = 0.0f;
            front_steer_pt.x = 50.0f;
            front_steer_pt.y = 0.0f;
            front_steer_pt.z = 0.0f;
            front_back_length = 100.0f;
        }
        else
        {
            v15 = thisa->m_wheels[1];
            v14 = thisa->m_wheels[0];
            v12 = v14->m_b1_wheel_center_loc.y + v15->m_b1_wheel_center_loc.y;
            v13 = v14->m_b1_wheel_center_loc.z + v15->m_b1_wheel_center_loc.z;
            front_steer_pt.x = (float)(v14->m_b1_wheel_center_loc.x + v15->m_b1_wheel_center_loc.x) * 0.5;
            front_steer_pt.y = v12 * 0.5;
            front_steer_pt.z = v13 * 0.5;
            front_back_length = front_steer_pt.x
                - (float)((float)(thisa->m_wheels[2]->m_b1_wheel_center_loc.x
                    + thisa->m_wheels[3]->m_b1_wheel_center_loc.x)
                    * 0.5);
        }
        maximum_acceleration = NitrousVehicle::get_maximum_acceleration();
        v11 = maximum_acceleration * thisa->m_parameter->m_body_mass;
        roll_stability_factor = thisa->m_parameter->m_steer_speed;
        hard_limit_dist = thisa->m_parameter->m_steer_angle_max;
        suspension_damp_k = thisa->m_parameter->m_wheel_radius;
        suspension_stiffness_k = thisa->m_parameter->m_tire_damp_coast * thisa->m_parameter->m_body_mass;
        fwd_fric_k = NitrousVehicle::get_max_speed(1);
        NitrousVehicle::set(
            thisa->m_parameter->m_tire_damp_hand * thisa->m_parameter->m_body_mass,
            thisa->m_parameter->m_tire_damp_brake * thisa->m_parameter->m_body_mass,
            fwd_fric_k,
            v11,
            suspension_stiffness_k,
            suspension_damp_k,
            hard_limit_dist,
            roll_stability_factor,
            &front_steer_pt,
            front_back_length);
        if (thisa->m_orientation_constraint)
        {
            thisa->m_orientation_constraint->m_torque_resistance_pitch_roll = thisa->m_parameter->m_pitch_roll_resistance
                * thisa->m_parameter->m_body_mass;
            thisa->m_orientation_constraint->m_torque_resistance_yaw = thisa->m_parameter->m_yaw_resistance
                * thisa->m_parameter->m_body_mass;
            thisa->m_orientation_constraint->m_upright_strength = thisa->m_parameter->m_upright_strength
                * thisa->m_parameter->m_body_mass;
            NitrousVehicle::_update_orientation_constraint();
        }
    }
}

// local variable allocation has failed, the output may be wrong!
void NitrousVehicle::_setup_wheels(int localClientNum)
{
    char *v3; // eax
    phys_vec3 *v4; // [esp+24h] [ebp-220h]
    int i; // [esp+40h] [ebp-204h]
    phys_vec3 *v6; // [esp+44h] [ebp-200h]
    phys_vec3 *v7; // [esp+64h] [ebp-1E0h]
    phys_vec3 *v8; // [esp+84h] [ebp-1C0h]
    float v9; // [esp+8Ch] [ebp-1B8h]
    phys_vec3 *v10; // [esp+C4h] [ebp-180h]
    float v11; // [esp+C8h] [ebp-17Ch]
    float v12; // [esp+CCh] [ebp-178h]
    float v13; // [esp+D0h] [ebp-174h]
    float y; // [esp+10Ch] [ebp-138h]
    phys_vec3 *v15; // [esp+124h] [ebp-120h]
    float v16; // [esp+12Ch] [ebp-118h]
    phys_vec3 *p_w; // [esp+164h] [ebp-E0h]
    float v18; // [esp+168h] [ebp-DCh]
    float v19; // [esp+16Ch] [ebp-D8h]
    float v20; // [esp+170h] [ebp-D4h]
    float pos_4; // [esp+1ACh] [ebp-98h]
    float mat34[4][3]; // [esp+1D0h] [ebp-74h] BYREF
    DObjAnimMat mat; // [esp+200h] [ebp-44h] BYREF
    unsigned int backWheel; // [esp+220h] [ebp-24h]
    unsigned int frontWheel; // [esp+224h] [ebp-20h]
    gentity_s *m_owner; // [esp+228h] [ebp-1Ch]
    DObj *obj; // [esp+22Ch] [ebp-18h]
    unsigned __int8 index; // [esp+233h] [ebp-11h] BYREF
    NitrousVehicle *thisa; // [esp+234h] [ebp-10h]
    //_UNKNOWN *v30; // [esp+238h] [ebp-Ch]
    //int localClientNuma; // [esp+23Ch] [ebp-8h]
    //int v32; // [esp+240h] [ebp-4h] BYREF
    //int vars0; // [esp+244h] [ebp+0h]
    //
    //v30 = a2;
    //localClientNuma = vars0;
    thisa = this;
    index = 0;
    obj = 0;
    if (!this->m_xmodel)
    {
        v3 = va(
            "NitrousVehicle:  Trying to setup wheels on vehicle with invalid model.  Vehicle %s at (%g %g %g).\n",
            thisa->m_vehicle_info->name,
            thisa->m_origin[0],
            thisa->m_origin[1],
            thisa->m_origin[2]);
        if (!Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\vehicle\\nitrous_vehicle.cpp",
            631,
            0,
            "%s\n\t%s",
            "m_xmodel",
            v3))
            __debugbreak();
    }
    if (thisa->m_owner)
    {
        m_owner = thisa->m_owner;
        obj = Com_GetServerDObj(m_owner->s.number);
    }
    else
    {
        obj = Com_GetClientDObj(thisa->m_entnum, localClientNum);
    }
    if (thisa->m_vehicle_info->type == 1)
    {
        frontWheel = SL_FindLowercaseString("tag_wheel_front", SCRIPTINSTANCE_SERVER);
        backWheel = SL_FindLowercaseString("tag_wheel_back", SCRIPTINSTANCE_SERVER);
        if (DObjGetBoneIndex(obj, frontWheel, &index, -1))
        {
            DObjGetBasePoseMatrix(obj, index, &mat);
            Ragdoll_AnimMatToMat43(&mat, mat34);

            iassert(!IS_NAN((mat34[3])[0]) && !IS_NAN((mat34[3])[1]) && !IS_NAN((mat34[3])[2]));

            Phys_Mat4ToNitrousMat(mat34, thisa->m_wheel_orig_relpo);
            //phys_mat44::operator=(&thisa->m_wheel_orig_relpo[1], thisa->m_wheel_orig_relpo);
            thisa->m_wheel_orig_relpo[1] = thisa->m_wheel_orig_relpo[0];
            pos_4 = thisa->m_wheel_orig_relpo[0].w.y;
            v18 = thisa->m_wheel_orig_relpo[0].w.x + 0.0;
            //v19 = pos_4 + COERCE_FLOAT(LODWORD(wheelSeperation) ^ _mask__NegFloat_);
            v19 = pos_4 + (-(wheelSeperation));
            v20 = thisa->m_wheel_orig_relpo[0].w.z + 0.0;
            p_w = &thisa->m_wheel_orig_relpo[0].w;
            thisa->m_wheel_orig_relpo[0].w.x = v18;
            p_w->y = v19;
            p_w->z = v20;
            v16 = pos_4 + wheelSeperation;
            v15 = &thisa->m_wheel_orig_relpo[1].w;
            thisa->m_wheel_orig_relpo[1].w.x = v18;
            v15->y = v16;
            v15->z = v20;
        }
        else if (!Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\vehicle\\nitrous_vehicle.cpp",
            662,
            0,
            "Failed to find tag_wheel_front for motorcycle."))
        {
            __debugbreak();
        }
        if (DObjGetBoneIndex(obj, backWheel, &index, -1))
        {
            DObjGetBasePoseMatrix(obj, index, &mat);
            Ragdoll_AnimMatToMat43(&mat, mat34);

            iassert(!IS_NAN((mat34[3])[0]) && !IS_NAN((mat34[3])[1]) && !IS_NAN((mat34[3])[2]));
            
            Phys_Mat4ToNitrousMat(mat34, &thisa->m_wheel_orig_relpo[2]);
            //phys_mat44::operator=(&thisa->m_wheel_orig_relpo[3], &thisa->m_wheel_orig_relpo[2]);
            thisa->m_wheel_orig_relpo[3] = thisa->m_wheel_orig_relpo[2];
            y = thisa->m_wheel_orig_relpo[2].w.y;
            v11 = thisa->m_wheel_orig_relpo[2].w.x + 0.0;
            //v12 = y + COERCE_FLOAT(LODWORD(wheelSeperation) ^ _mask__NegFloat_);
            v12 = y + (-(wheelSeperation));
            v13 = thisa->m_wheel_orig_relpo[2].w.z + 0.0;
            v10 = &thisa->m_wheel_orig_relpo[2].w;
            thisa->m_wheel_orig_relpo[2].w.x = v11;
            v10->y = v12;
            v10->z = v13;
            v9 = y + wheelSeperation;
            v8 = &thisa->m_wheel_orig_relpo[3].w;
            thisa->m_wheel_orig_relpo[3].w.x = v11;
            v8->y = v9;
            v8->z = v13;
        }
        else if (!Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\vehicle\\nitrous_vehicle.cpp",
            678,
            0,
            "Failed to find tag_wheel_back for motorcycle."))
        {
            __debugbreak();
        }
        v7 = &thisa->m_wheel_orig_relpo[4].w;
        thisa->m_wheel_orig_relpo[4].w.x = 0.0f;
        v7->y = 0.0f;
        v7->z = 0.0f;
        v6 = &thisa->m_wheel_orig_relpo[5].w;
        thisa->m_wheel_orig_relpo[5].w.x = 0.0f;
        v6->y = 0.0f;
        v6->z = 0.0f;
    }
    else
    {
        for (i = 0; i < 6; ++i)
        {
            index = 0;
            if (DObjGetBoneIndex(obj, *s_wheelTags[i], &index, -1))
            {
                DObjGetBasePoseMatrix(obj, index, &mat);
                Ragdoll_AnimMatToMat43(&mat, mat34);
                Phys_Mat4ToNitrousMat(mat34, &thisa->m_wheel_orig_relpo[i]);
            }
            else
            {
                v4 = &thisa->m_wheel_orig_relpo[i].w;
                v4->x = 0.0f;
                v4->y = 0.0f;
                v4->z = 0.0f;
            }
            thisa->m_wheel_damage[i] = 0;
        }
    }
}

void __cdecl Phys_Mat4ToNitrousMat(float (*inMat)[3], phys_mat44 *outMat)
{
    Phys_Vec3ToNitrousVec((float *)inMat, &outMat->x);
    Phys_Vec3ToNitrousVec(&(*inMat)[3], &outMat->y);
    Phys_Vec3ToNitrousVec(&(*inMat)[6], &outMat->z);
    Phys_Vec3ToNitrousVec(&(*inMat)[9], &outMat->w);
}

double __thiscall NitrousVehicle::_calc_initial_susp_spring_k(rigid_body_constraint_wheel *wheel_constraint)
{
    float v3; // [esp-8h] [ebp-40h]
    float v4; // [esp-4h] [ebp-3Ch]
    float susp_spring_k; // [esp+0h] [ebp-38h]

    susp_spring_k = fabs(wheel_constraint->m_b1_wheel_center_loc.x)
                                / this->m_steer_front_back_length;
    if ( (float)(susp_spring_k - 1.0) < 0.0 )
        v4 = fabs(wheel_constraint->m_b1_wheel_center_loc.x)
             / this->m_steer_front_back_length;
    else
        v4 = 1.0f;
    if ( (float)(0.0 - susp_spring_k) < 0.0 )
        v3 = v4;
    else
        v3 = 0.0f;
    return (float)((float)((float)(this->m_parameter->m_susp_spring_k * phys_vehicleGravityMultiplier->current.value)
                                             * (float)((float)(1.0 - v3) * 2.0))
                             * this->m_parameter->m_body_mass);
}

bool __thiscall NitrousVehicle::is_peeling_out()
{
    return this->m_throttle > 0.89999998 && this->m_parameter->m_peel_out_max_speed > this->m_forward_vel;
}

unsigned int s_tankWheelJnts[6];
void __thiscall NitrousVehicle::destructible_damage(const char *notifyLabel, int damageStage)
{
    int j; // [esp+10h] [ebp-10h]
    int i; // [esp+14h] [ebp-Ch]
    int max; // [esp+18h] [ebp-8h]
    int min; // [esp+1Ch] [ebp-4h]

    min = 0;
    max = -1;
    if (notifyLabel)
    {
        if (!s_tankWheelJnts[0])
        {
            s_tankWheelJnts[0] = scr_const.tag_wheel_front_left;
            s_tankWheelJnts[1] = scr_const.tag_wheel_middle_left;
            s_tankWheelJnts[2] = scr_const.tag_wheel_back_left;
            s_tankWheelJnts[3] = scr_const.tag_wheel_front_right;
            s_tankWheelJnts[4] = scr_const.tag_wheel_middle_right;
            s_tankWheelJnts[5] = scr_const.tag_wheel_back_right;
        }
        if (I_strcmp(notifyLabel, "left_tread_destroyed"))
        {
            if (!I_strcmp(notifyLabel, "right_tread_destroyed"))
            {
                min = 3;
                max = 6;
                this->m_speed_scale = 0.0f;
            }
        }
        else
        {
            min = 0;
            max = 3;
            this->m_speed_scale = 0.0f;
        }
        if (this->m_speed_scale < 0.0)
            this->m_speed_scale = 1.0f;
        for (i = min; i < max; ++i)
        {
            for (j = 0; j < 6; ++j)
            {
                if (s_tankWheelJnts[i] == *s_wheelTags[j])
                    this->m_wheels[j]->m_wheel_radius = this->m_parameter->m_wheel_radius * tempRadiusPercent;
            }
        }
    }
}

void __thiscall NitrousVehicle::damage(unsigned int boneName, int damageStage)
{
    ;
}

void __thiscall NitrousVehicle::damage(int damage, const float *point, const float *dir, int mod)
{
    gentity_s *m_owner; // edx
    gentity_s *tempent; // [esp+34h] [ebp-18h]
    float force[3]; // [esp+38h] [ebp-14h] BYREF
    float forcescale; // [esp+44h] [ebp-8h]
    bool doForce; // [esp+4Bh] [ebp-1h]

    doForce = mod >= 3 && (mod <= 6 || mod == 15);
    if ( this->m_phys_user_data && doForce )
    {
        if ( damage <= 250 )
        {
            if ( damage < 50 )
                damage = 50;
        }
        else
        {
            damage = 250;
        }
        force[0] = *dir;
        force[1] = dir[1];
        force[2] = dir[2];
        Vec3NormalizeFast(force);
        forcescale = phys_vehicleDamageFroceScale->current.value;
        force[0] = (float)((float)damage * forcescale) * force[0];
        force[1] = (float)((float)damage * forcescale) * force[1];
        force[2] = (float)((float)damage * forcescale) * force[2];
        if ( this->m_owner )
        {
            tempent = G_TempEntity(force, EV_PHYS_LAUNCH);
            AssignToSmallerType<short>(&tempent->s.otherEntityNum, this->m_owner->s.number);
            tempent->s.lerp.pos.trDelta[0] = force[0];
            tempent->s.lerp.pos.trDelta[1] = force[1];
            tempent->s.lerp.pos.trDelta[2] = force[2];
            m_owner = this->m_owner;
            tempent->s.lerp.pos.trDelta[0] = *point - m_owner->r.currentOrigin[0];
            tempent->s.lerp.pos.trDelta[1] = point[1] - m_owner->r.currentOrigin[1];
            tempent->s.lerp.pos.trDelta[2] = point[2] - m_owner->r.currentOrigin[2];
        }
    }
}

phys_vec3 *__thiscall NitrousVehicle::get_velocity(phys_vec3 *result)
{
    rigid_body *body; // ecx
    float v5; // [esp+10h] [ebp-8h]
    float v6; // [esp+14h] [ebp-4h]

    if ( this->m_phys_user_data )
    {
        body = this->m_phys_user_data->body;
        v5 = body->m_t_vel.y - this->m_ground_vel.y;
        v6 = body->m_t_vel.z - this->m_ground_vel.z;
        result->x = body->m_t_vel.x - this->m_ground_vel.x;
        result->y = v5;
        result->z = v6;
    }
    else
    {
        result->x = 0.0f;
        result->y = 0.0f;
        result->z = 0.0f;
    }
    return result;
}

phys_vec3 *__thiscall NitrousVehicle::get_a_velocity(phys_vec3 *result)
{
    if ( this->m_phys_user_data )
    {
        *result = this->m_phys_user_data->body->m_a_vel;
    }
    else
    {
        result->x = 0.0f;
        result->y = 0.0f;
        result->z = 0.0f;
    }
    return result;
}

void __thiscall NitrousVehicle::_update_orientation_constraint()
{
    bool v1; // [esp+2h] [ebp-36h]
    bool v2; // [esp+3h] [ebp-35h]
    rigid_body_constraint_wheel *rbc_wheel; // [esp+2Ch] [ebp-Ch]
    int i; // [esp+30h] [ebp-8h]

    if ( !this->m_orientation_constraint
        && _tlAssert(
                 "C:\\projects_pc\\cod\\codsrc\\src\\vehicle\\nitrous_vehicle.cpp",
                 880,
                 "m_orientation_constraint",
                 "") )
    {
        __debugbreak();
    }
    if ( !this->m_phys_user_data
        && _tlAssert(
                 "C:\\projects_pc\\cod\\codsrc\\src\\vehicle\\nitrous_vehicle.cpp",
                 881,
                 "m_phys_user_data",
                 "") )
    {
        __debugbreak();
    }
    if ( !this->m_phys_user_data->body
        && _tlAssert(
                 "C:\\projects_pc\\cod\\codsrc\\src\\vehicle\\nitrous_vehicle.cpp",
                 882,
                 "m_phys_user_data->body",
                 "") )
    {
        __debugbreak();
    }
    v2 = ((this->m_flags & 4) != 0 || (this->m_flags & 0x40) != 0 || (this->m_flags & 0x100) != 0)
        && (this->m_flags & 0x200) == 0
        || (this->m_flags & 0x80) != 0;
    v1 = !this->m_num_colliding_wheels && !this->m_phys_user_data->body->m_contact_count;
    this->m_orientation_constraint->m_active = v2;
    this->m_orientation_constraint->m_no_orientation_correction = v1;
    if ( (this->m_flags & 0x1000) != 0 )
    {
        this->m_orientation_constraint->m_active = 0;
        this->m_orientation_constraint->m_torque_resistance_pitch_roll = 0.0f;
        this->m_orientation_constraint->m_torque_resistance_yaw = 0.0f;
        this->m_orientation_constraint->m_upright_strength = 0.0f;
        this->m_orientation_constraint->m_no_orientation_correction = 1;
        for ( i = 0; i < 6; ++i )
        {
            rbc_wheel = this->m_wheels[i];
            if ( rbc_wheel )
                rbc_wheel->m_roll_stability_factor = 0.0f;
        }
    }
}

double __thiscall NitrousVehicle::_get_stuck_accel_factor(float delta_t)
{
    float v4; // [esp+8h] [ebp-1Ch]
    float accel_scale; // [esp+1Ch] [ebp-8h]
    float abs_steer_factor; // [esp+20h] [ebp-4h]

    if ( !this->m_phys_user_data )
        return 1.0;
    if ( this->m_throttle <= 0.2 )
    {
        this->m_stuck_time = 0.0f;
    }
    else if ( this->m_phys_user_data )
    {
        abs_steer_factor = fabs(this->m_steer_factor);
        if ( (float)(vel_threshold * vel_threshold) <= (float)((float)((float)(this->m_phys_user_data->body->m_t_vel.x
                                                                                                                                                 * this->m_phys_user_data->body->m_t_vel.x)
                                                                                                                                 + (float)(this->m_phys_user_data->body->m_t_vel.y
                                                                                                                                                 * this->m_phys_user_data->body->m_t_vel.y))
                                                                                                                 + (float)(this->m_phys_user_data->body->m_t_vel.z
                                                                                                                                 * this->m_phys_user_data->body->m_t_vel.z))
            || abs_steer_factor >= 0.30000001 )
        {
            if ( (float)(avel_threshold * avel_threshold) <= (float)((float)((float)(this->m_phys_user_data->body->m_a_vel.x
                                                                                                                                                         * this->m_phys_user_data->body->m_a_vel.x)
                                                                                                                                         + (float)(this->m_phys_user_data->body->m_a_vel.y
                                                                                                                                                         * this->m_phys_user_data->body->m_a_vel.y))
                                                                                                                         + (float)(this->m_phys_user_data->body->m_a_vel.z
                                                                                                                                         * this->m_phys_user_data->body->m_a_vel.z))
                || abs_steer_factor <= 0.5 )
            {
                this->m_stuck_time = 0.0f;
            }
            else
            {
                this->m_stuck_time = this->m_stuck_time + delta_t;
            }
        }
        else
        {
            this->m_stuck_time = this->m_stuck_time + delta_t;
        }
    }
    if ( this->m_stuck_time <= 0.40000001 )
        return 1.0;
    accel_scale = (float)((float)(this->m_stuck_time - 0.40000001) * 30.0) + 1.0;
    if ( (float)(accel_scale - 40.0) < 0.0 )
        v4 = (float)((float)(this->m_stuck_time - 0.40000001) * 30.0) + 1.0;
    else
        v4 = 40.0f;
    if ( (float)(1.0 - accel_scale) < 0.0 )
        return v4;
    else
        return 1.0f;
}

void __thiscall NitrousVehicle::_update_boost(float delta_t)
{
    float boostDuration; // [esp+0h] [ebp-40h]
    float m_boost_time_pool; // [esp+4h] [ebp-3Ch]
    bool v4; // [esp+Ch] [ebp-34h]
    bool v5; // [esp+10h] [ebp-30h]
    bool tryingToBoost; // [esp+3Ah] [ebp-6h]
    bool wasBoosting; // [esp+3Dh] [ebp-3h]

    //tryingToBoost = bitarray<51>::testBit(&this->mVehicleController.m_cmd.button_bits, 0x2Bu);
    tryingToBoost = this->mVehicleController.m_cmd.button_bits.testBit(0x2Bu);
    wasBoosting = (this->m_flags & 0x800) != 0;
    if ( (this->m_flags & 0x800) != 0 )
        this->m_boost_time_pool = this->m_boost_time_pool - delta_t;
    if ( this->m_boost_start_time )
        v5 = this->mVehicleController.m_cmd.serverTime < this->m_boost_start_time
                                                                                                     + (int)(float)(this->m_vehicle_info->boostInterval * 1000.0);
    else
        v5 = 0;
    if ( this->m_boost_start_time )
        v4 = this->mVehicleController.m_cmd.serverTime < this->m_boost_start_time
                                                                                                     + (int)(float)(this->m_vehicle_info->boostContinuousRegenCooldown
                                                                                                                                * 1000.0);
    else
        v4 = 1;
    if ( this->m_vehicle_info->boostMode )
    {
        if ( this->m_vehicle_info->boostMode != 1 )
            goto LABEL_37;
        if ( !tryingToBoost || this->m_boost_time_pool <= 0.0 )
        {
            if ( !tryingToBoost && !v4 && this->m_vehicle_info->boostDuration > this->m_boost_time_pool )
            {
                this->m_boost_time_pool = (float)(this->m_vehicle_info->boostContinuousRegenAmount * delta_t)
                                                                + this->m_boost_time_pool;
                if ( (float)(this->m_vehicle_info->boostDuration - this->m_boost_time_pool) < 0.0 )
                    boostDuration = this->m_vehicle_info->boostDuration;
                else
                    boostDuration = this->m_boost_time_pool;
                this->m_boost_time_pool = boostDuration;
            }
            goto LABEL_37;
        }
LABEL_19:
        NitrousVehicle::set_boosting(1);
        return;
    }
    if ( this->m_vehicle_info->boostInterval > this->m_vehicle_info->boostDuration )
    {
        if ( wasBoosting && this->m_boost_time_pool > 0.0 )
        {
            NitrousVehicle::set_boosting(wasBoosting);
            return;
        }
        if ( !tryingToBoost || this->m_boost_time_pool <= 0.0 || v5 )
        {
            if ( v5 && this->m_vehicle_info->boostDuration > this->m_boost_time_pool )
            {
                this->m_boost_time_pool = (float)((float)(this->m_vehicle_info->boostDuration
                                                                                                / (float)(this->m_vehicle_info->boostInterval
                                                                                                                - this->m_vehicle_info->boostDuration))
                                                                                * delta_t)
                                                                + this->m_boost_time_pool;
                if ( (float)(this->m_vehicle_info->boostDuration - this->m_boost_time_pool) < 0.0 )
                    m_boost_time_pool = this->m_vehicle_info->boostDuration;
                else
                    m_boost_time_pool = this->m_boost_time_pool;
                this->m_boost_time_pool = m_boost_time_pool;
            }
            goto LABEL_37;
        }
        goto LABEL_19;
    }
    Com_PrintWarning(15, "boostDuration duration must be less than boostInterval\n");
LABEL_37:
    NitrousVehicle::set_boosting(0);
}

void NitrousVehicle::_update_prolog(float delta_t)
{
    const VehicleParameter *m_parameter; // esi
    const VehicleParameter *v4; // esi
    double Value; // st7
    float v6; // xmm0_4
    rigid_body *body; // [esp+58h] [ebp-E8h]
    float v8; // [esp+5Ch] [ebp-E4h]
    float maximum_acceleration_factor; // [esp+88h] [ebp-B8h]
    float fraction; // [esp+9Ch] [ebp-A4h]
    float v11; // [esp+A4h] [ebp-9Ch]
    float v12; // [esp+ACh] [ebp-94h]
    float speed; // [esp+B0h] [ebp-90h]
    float v14; // [esp+B4h] [ebp-8Ch]
    float v15; // [esp+C0h] [ebp-80h]
    float v16; // [esp+C4h] [ebp-7Ch]
    float v17; // [esp+C8h] [ebp-78h]
    float v18; // [esp+D0h] [ebp-70h]
    float v19; // [esp+D4h] [ebp-6Ch]
    float v20; // [esp+DCh] [ebp-64h]
    float v21; // [esp+E0h] [ebp-60h]
    float z; // [esp+E8h] [ebp-58h]
    int v23; // [esp+100h] [ebp-40h]
    rigid_body_constraint_wheel *accel_factor; // [esp+104h] [ebp-3Ch]
    int j; // [esp+108h] [ebp-38h]
    int v26; // [esp+10Ch] [ebp-34h]
    float rbc_wheela; // [esp+110h] [ebp-30h]
    float rbc_wheel; // [esp+110h] [ebp-30h]
    int i; // [esp+114h] [ebp-2Ch]
    float ib; // [esp+114h] [ebp-2Ch]
    int ia; // [esp+114h] [ebp-2Ch]
    //unsigned int v33[2]; // [esp+134h] [ebp-Ch] BYREF
    //_UNKNOWN *retaddr; // [esp+140h] [ebp+0h]
    //
    //v33[0] = a2;
    //v33[1] = retaddr;
    NitrousVehicle::_update_pause();
    this->m_notify_flags = 0;
    //NitrousVehicleController::UpdateControls(&this->mVehicleController, COERCE_FLOAT(v33), this, delta_t);
    this->mVehicleController.UpdateControls(this, delta_t);
    NitrousVehicle::_update_boost(delta_t);
    if ( this->m_throttle <= 0.0 )
        this->m_state_flags &= ~4u;
    else
        this->m_state_flags |= 4u;
    if ( this->m_hand_brake <= 0.0 )
        this->m_state_flags &= ~1u;
    else
        this->m_state_flags |= 1u;
    m_parameter = this->m_parameter;
    *(float *)&i = NitrousVehicle::get_maximum_acceleration() * m_parameter->m_body_mass;
    rbc_wheela = NitrousVehicle::get_max_speed(1);
    rbc_wheel = rbc_wheela * this->m_speed_scale;
    if ( this->m_vehicle_info->type == 2 )
    {
        v26 = 0;
        for ( j = 0; j < 6; ++j )
        {
            accel_factor = this->m_wheels[j];
            if ( accel_factor )
            {
                if ( (accel_factor->m_wheel_flags & 1) != 0 )
                    ++v26;
            }
        }
        if ( v26 <= 1 )
            v23 = 2;
        else
            v23 = v26;
        ib = *(float *)&i * (float)(fabs(this->m_throttle) * (float)(6.0 / (float)v23));
        *(float *)&ia = NitrousVehicle::_get_stuck_accel_factor(delta_t) * ib;
    }
    else
    {
        *(float *)&ia = fabs(this->m_throttle) * *(float *)&i;
        if ( this->m_throttle < 0.0 )
        {
            *(float *)&ia = *(float *)&ia * this->m_parameter->m_reverse_scale;
            rbc_wheel = rbc_wheel * this->m_parameter->m_reverse_scale;
        }
    }
    if ( this->m_phys_user_data )
    {
        z = this->m_phys_user_data->body->m_mat.x.z;
        if ( phys_heavyTankSwitch->current.enabled )
        {
            if ( z <= 0.0 )
            {
                if ( (float)((float)(z + pitchFakeyZone) - 0.0) < 0.0 )
                    v19 = z + pitchFakeyZone;
                else
                    v19 = 0.0f;
                if ( (float)(-1.0 - (float)(z + pitchFakeyZone)) < 0.0 )
                    v18 = v19;
                else
                    v18 = -1.0f;
                z = v18;
            }
            else
            {
                if ( (float)((float)(z - pitchFakeyZone) - 1.0) < 0.0 )
                    v21 = z - pitchFakeyZone;
                else
                    v21 = 1.0f;
                if ( (float)(0.0 - (float)(z - pitchFakeyZone)) < 0.0 )
                    v20 = v21;
                else
                    v20 = 0.0f;
                z = v20;
            }
        }
        if ( (float)((float)(z / 0.40000001) - 1.0) < 0.0 )
            v17 = z / 0.40000001;
        else
            v17 = 1.0f;
        if ( (float)(-1.0 - (float)(z / 0.40000001)) < 0.0 )
            v16 = v17;
        else
            v16 = -1.0f;
        v15 = (float)((float)(1.0 - fabs(this->m_steer_factor)) * this->m_throttle)
                * v16;
        if ( (float)(v15 - 1.0) < 0.0 )
            v14 = (float)((float)(1.0 - fabs(this->m_steer_factor)) * this->m_throttle)
                    * v16;
        else
            v14 = 1.0f;
        if ( (float)(0.0 - v15) < 0.0 )
            speed = v14;
        else
            speed = 0.0f;
        v4 = this->m_parameter;
        v12 = NitrousVehicle::get_maximum_acceleration() * v4->m_body_mass;
        if ( phys_heavyTankSwitch->current.enabled )
            *(float *)&ia = (float)(speed * climbHeavyAccelAdd) + *(float *)&ia;
        *(float *)&ia = (float)(speed * v12) + *(float *)&ia;
    }
    if ( (this->m_flags & 1) == 0 )
    {
        v11 = fabs(this->m_forward_vel);
        if ( this->m_vehicle_info->accelGraph )
        {
            if ( (float)((float)(v11 / rbc_wheel) - 1.0) < 0.0 )
                fraction = v11 / rbc_wheel;
            else
                fraction = 1.0f;
            if ( (float)(0.0 - (float)(v11 / rbc_wheel)) < 0.0 )
                Value = GraphFloat_GetValue(this->m_vehicle_info->accelGraph, fraction);
            else
                Value = GraphFloat_GetValue(this->m_vehicle_info->accelGraph, 0.0);
            *(float *)&ia = Value * *(float *)&ia;
        }
        else if ( low_end_speed > v11 )
        {
            *(float *)&ia = (float)((float)(*(float *)&ia * (float)(1.0 - (float)(v11 / low_end_speed))) * percent_to_give)
                                        + *(float *)&ia;
        }
    }
    if ( scale_max_speed )
        rbc_wheel = rbc_wheel * fabs(this->m_throttle);
    if ( *(float *)&ia <= NitrousVehicle::get_maximum_acceleration_factor() )
        maximum_acceleration_factor = NitrousVehicle::get_maximum_acceleration_factor();
    else
        maximum_acceleration_factor = *(float *)&ia;
    this->m_acceleration_factor = maximum_acceleration_factor;
    this->m_desired_speed_factor = rbc_wheel;
    if ( this->m_hand_brake <= 0.0 && this->m_throttle <= 0.0 )
        this->m_state_flags |= 0x10u;
    else
        this->m_state_flags &= ~0x10u;
    if ( this->m_brake <= 0.0 )
        this->m_state_flags &= ~2u;
    else
        this->m_state_flags |= 2u;
    this->m_state_flags &= ~8u;
    if ( this->m_parameter->m_traction_type == TRACTION_TYPE_ALL_WD )
    {
        if ( this->m_hand_brake <= 0.0 )
            v6 = *(float *)&ia;
        else
            v6 = *(float *)&ia * 2.0;
        if ( v6 <= NitrousVehicle::get_maximum_acceleration_factor() )
            v8 = NitrousVehicle::get_maximum_acceleration_factor();
        else
            v8 = v6;
        this->m_acceleration_factor = v8;
    }
    if ( this->m_phys_user_data )
    {
        body = this->m_phys_user_data->body;
        this->m_forward_vel = (float)((float)((float)(body->m_t_vel.x - this->m_ground_vel.x) * body->m_mat.x.x)
                                                                + (float)((float)(body->m_t_vel.y - this->m_ground_vel.y) * body->m_mat.x.y))
                                                + (float)((float)(body->m_t_vel.z - this->m_ground_vel.z) * body->m_mat.x.z);
        if ( this->m_throttle >= 0.0 )
        {
            if ( this->m_throttle > 0.0
                && this->m_parameter->m_peel_out_max_speed > 5.0
                //&& COERCE_FLOAT(LODWORD(this->m_parameter->m_peel_out_max_speed) ^ _mask__NegFloat_) > this->m_forward_vel )
                && (-(this->m_parameter->m_peel_out_max_speed)) > this->m_forward_vel )
            {
                this->m_state_flags |= 2u;
            }
        }
        else if ( this->m_forward_vel <= 10.0 || this->m_num_colliding_wheels <= 1 )
        {
            this->m_state_flags |= 8u;
        }
        else
        {
            this->m_state_flags |= 2u;
        }
    }
    else
    {
        this->m_forward_vel = 0.0f;
    }
    if ( this->m_script_brake > 0.0 )
        this->m_state_flags |= 2u;
    if ( (this->m_flags & 1) == 0 )
    {
        NitrousVehicle::update_braking_and_acceleration(delta_t);
        NitrousVehicle::update_steering(delta_t);
        NitrousVehicle::_update_fakey_stuff(delta_t);
        NitrousVehicle::_update_friction(delta_t);
        NitrousVehicle::_update_orientation_constraint();
        if ( (this->m_flags & 0x80) != 0 )
        {
            if ( !this->m_vpc )
                NitrousVehicle::start_path(0);
            if ( this->m_vpc )
                path_constraint_update(this->m_vpc, this->m_owner);
        }
    }
}

void __thiscall NitrousVehicle::_update_epilog(float delta_t)
{
    int savedregs; // [esp+8h] [ebp+0h] BYREF

    NitrousVehicle::_update_wheel_effect_states(delta_t);
}

void __thiscall NitrousVehicle::_update_friction(float delta_t)
{
    float v2; // [esp+0h] [ebp-28h]
    float v4; // [esp+10h] [ebp-18h]
    float fric_scaler; // [esp+1Ch] [ebp-Ch]
    float fric_scalera; // [esp+1Ch] [ebp-Ch]
    float up; // [esp+24h] [ebp-4h]

    if ( (this->m_flags & 0x80) == 0 )
    {
        this->m_hand_brake_friction_time = this->m_hand_brake_friction_time - delta_t;
        if ( this->m_hand_brake > 0.1 && this->m_parameter->m_tire_fric_hand_brake > 0.0 )
            this->m_hand_brake_friction_time = hand_brake_friction_time;
        up = this->m_phys_user_data->body->m_mat.z.z;
        fric_scaler = 1.0f;
        if ( this->m_vehicle_info->max_fric_tilt_angle > up && this->m_vehicle_info->max_fric_tilt_angle > 0.0 )
        {
            fric_scalera = (float)(this->m_vehicle_info->max_fric_tilt / this->m_vehicle_info->max_fric_tilt_angle) * up;
            if ( (float)(fric_scalera - 1.0) < 0.0 )
                v4 = (float)(this->m_vehicle_info->max_fric_tilt / this->m_vehicle_info->max_fric_tilt_angle) * up;
            else
                v4 = 1.0f;
            if ( (float)(0.15000001 - fric_scalera) < 0.0 )
                v2 = v4;
            else
                v2 = 0.15000001;
            fric_scaler = v2;
        }
        if ( NitrousVehicle::is_boosting() && this->m_vehicle_info->boostFrictionMultiplier > 0.0 )
            fric_scaler = fric_scaler * this->m_vehicle_info->boostFrictionMultiplier;
        if ( (this->m_flags & 0x80) != 0 )
        {
            this->m_current_front_side_fric_scale = 0.0f;
            this->m_current_front_fwd_fric_scale = 0.0f;
        }
        else if ( this->m_brake <= 0.1 )
        {
            if ( this->m_hand_brake_friction_time <= 0.0 )
            {
                this->m_current_front_side_fric_scale = (float)(this->m_parameter->m_tire_fric_side * fric_scaler)
                                                                                            * side_fric_scale;
                this->m_current_front_fwd_fric_scale = (float)(this->m_parameter->m_tire_fric_fwd * fric_scaler)
                                                                                         * fwd_fric_scale;
            }
            else
            {
                this->m_current_front_side_fric_scale = this->m_parameter->m_tire_fric_hand_brake * fric_scaler;
                if ( fabs(this->m_throttle) <= 0.1 )
                    this->m_current_front_fwd_fric_scale = this->m_current_front_side_fric_scale;
                else
                    this->m_current_front_fwd_fric_scale = this->m_parameter->m_tire_fric_fwd * fric_scaler;
            }
        }
        else
        {
            this->m_current_front_side_fric_scale = this->m_parameter->m_tire_fric_brake * fric_scaler;
            this->m_current_front_fwd_fric_scale = this->m_current_front_side_fric_scale;
        }
        this->m_current_rear_side_fric_scale = this->m_current_front_side_fric_scale;
        this->m_current_rear_fwd_fric_scale = this->m_current_front_fwd_fric_scale;
    }
}

void __thiscall NitrousVehicle::_update_fakey_stuff(float delta_t)
{
    environment_rigid_body *env_rigid_body; // eax
    float v4; // [esp+Ch] [ebp-7Ch]
    rigid_body_constraint_wheel *v5; // [esp+48h] [ebp-40h]
    int j; // [esp+4Ch] [ebp-3Ch]
    rigid_body_constraint_wheel *rbc_wheel; // [esp+50h] [ebp-38h]
    int i; // [esp+54h] [ebp-34h]
    float rpm_lerp_rate; // [esp+58h] [ebp-30h]
    environment_rigid_body *rb; // [esp+5Ch] [ebp-2Ch]
    float spring_delta; // [esp+60h] [ebp-28h]
    float spring_k; // [esp+64h] [ebp-24h]
    int num_colliding_wheels; // [esp+6Ch] [ebp-1Ch]
    bool high_rpm; // [esp+73h] [ebp-15h]
    float rpm_target; // [esp+7Ch] [ebp-Ch]
    bool peeling_out; // [esp+83h] [ebp-5h]
    rigid_body_constraint_wheel *wheel; // [esp+84h] [ebp-4h]
    rigid_body_constraint_wheel *wheela; // [esp+84h] [ebp-4h]
    rigid_body_constraint_wheel *wheelb; // [esp+84h] [ebp-4h]
    rigid_body_constraint_wheel *wheelc; // [esp+84h] [ebp-4h]

    rb = (environment_rigid_body *)this->m_phys_user_data->body;
    num_colliding_wheels = 0;
    spring_delta = 0.0f;
    if ( this->m_throttle <= 0.5 )
    {
        if (this->m_throttle < -0.5)
        {
            //spring_delta = (float)(COERCE_FLOAT(LODWORD(this->m_parameter->m_susp_spring_k) ^ _mask__NegFloat_) * this->m_parameter->m_tilt_fakey) * this->m_parameter->m_body_mass;
            spring_delta = (float)((-(this->m_parameter->m_susp_spring_k)) * this->m_parameter->m_tilt_fakey) * this->m_parameter->m_body_mass;
        }
    }
    else
    {
        spring_delta = (float)(this->m_parameter->m_susp_spring_k * this->m_parameter->m_tilt_fakey)
                                 * this->m_parameter->m_body_mass;
    }
    spring_k = 30.0f;
    wheel = this->m_wheels[1];
    if ( wheel )
    {
        spring_k = NitrousVehicle::_calc_initial_susp_spring_k(wheel);
        wheel->m_suspension_stiffness_k = spring_k + spring_delta;
        if ( (wheel->m_wheel_flags & 1) != 0 )
            num_colliding_wheels = 1;
    }
    wheela = this->m_wheels[0];
    if ( wheela )
    {
        wheela->m_suspension_stiffness_k = spring_k + spring_delta;
        if ( (wheela->m_wheel_flags & 1) != 0 )
            ++num_colliding_wheels;
    }
    wheelb = this->m_wheels[3];
    if ( wheelb )
    {
        spring_k = NitrousVehicle::_calc_initial_susp_spring_k(wheelb);
        if ( (wheelb->m_wheel_flags & 1) != 0 )
            ++num_colliding_wheels;
        wheelb->m_suspension_stiffness_k = spring_k - spring_delta;
    }
    wheelc = this->m_wheels[2];
    if ( wheelc )
    {
        wheelc->m_suspension_stiffness_k = spring_k - spring_delta;
        if ( (wheelc->m_wheel_flags & 1) != 0 )
            ++num_colliding_wheels;
    }
    if ( !this->m_num_colliding_wheels || num_colliding_wheels )
    {
        if ( !this->m_num_colliding_wheels && num_colliding_wheels )
        {
            this->m_notify_flags |= 4u;
            this->m_server_notify_flags |= 4u;
        }
    }
    else
    {
        this->m_notify_flags |= 2u;
        this->m_server_notify_flags |= 2u;
    }
    for ( i = 0; i < 6; ++i )
    {
        rbc_wheel = this->m_wheels[i];
        if ( rbc_wheel && (rbc_wheel->m_wheel_flags & 2) != 0 )
        {
            this->m_notify_flags |= 8u;
            this->m_server_notify_flags |= 8u;
            break;
        }
    }
    this->m_num_colliding_wheels = num_colliding_wheels;
    if ( this->mVehicleController.m_stun.m_changed_this_frame )
    {
        this->m_notify_flags |= 0x10u;
        this->m_server_notify_flags |= 0x10u;
    }
    if ( !this->m_orientation_constraint )
    {
        env_rigid_body = phys_sys::get_environment_rigid_body();
        this->m_orientation_constraint = phys_sys::create_rbc_custom_orientation(rb, env_rigid_body, 0);
        this->m_orientation_constraint->m_torque_resistance_pitch_roll = this->m_parameter->m_pitch_roll_resistance
                                                                                                                                     * this->m_parameter->m_body_mass;
        this->m_orientation_constraint->m_torque_resistance_yaw = this->m_parameter->m_yaw_resistance
                                                                                                                        * this->m_parameter->m_body_mass;
        this->m_orientation_constraint->m_upright_strength = this->m_parameter->m_upright_strength
                                                                                                             * this->m_parameter->m_body_mass;
    }
    peeling_out = NitrousVehicle::is_peeling_out();
    high_rpm = 0;
    if ( this->m_vehicle_info->type != 2 && this->m_vehicle_info->type != 4 )
    {
        for ( j = 0; j < 6; ++j )
        {
            v5 = this->m_wheels[j];
            if ( v5 && !v5->m_wheel_state && (peeling_out || this->m_throttle > 0.69999999 && (v5->m_wheel_flags & 1) == 0) )
            {
                high_rpm = 1;
                v5->m_wheel_pos = (float)(40.0 * delta_t) + v5->m_wheel_pos;
            }
        }
    }
    if ( peeling_out && this->m_throttle > 0.69999999 )
        high_rpm = 1;
    v4 = this->m_forward_vel / NitrousVehicle::get_max_speed(1);
    rpm_target = fabs(this->m_throttle) * fabs(v4);
    if ( this->m_vehicle_info->type == 4 )
    {
        if ( (float)(this->m_throttle * 0.80000001) > rpm_target )
            rpm_target = this->m_throttle * 0.80000001;
        if ( (this->m_flags & 0x400) == 0 )
            rpm_target = 1.0f;
    }
    if ( high_rpm && rpm_target < 0.80000001 )
        rpm_target = 0.8f;
    rpm_lerp_rate = 1.5f;
    if ( snd_boat_using_lerp_rpm && snd_boat_using_lerp_rpm->current.enabled )
        rpm_lerp_rate = snd_boat_lerp_rpm->current.value;
    this->m_fake_rpm = (float)((float)(delta_t * (float)(rpm_target - this->m_fake_rpm)) * rpm_lerp_rate) + this->m_fake_rpm;
}

void __thiscall NitrousVehicle::start_path(int attach_mode)
{
    float axis[9]; // [esp+30h] [ebp-3Ch] BYREF
    float *currentOrigin; // [esp+54h] [ebp-18h]
    float origin[3]; // [esp+58h] [ebp-14h] BYREF
    rigid_body_constraint_wheel *rbc_wheel; // [esp+64h] [ebp-8h]
    int i; // [esp+68h] [ebp-4h]
    int savedregs; // [esp+6Ch] [ebp+0h] BYREF

    Sys_EnterCriticalSection(CRITSECT_PHYSICS_UPDATE);
    Sys_EnterCriticalSection(CRITSECT_PHYSICS);
    this->m_flags &= ~4u;
    this->m_flags &= ~0x100u;
    this->m_flags &= ~0x80u;
    if ( attach_mode )
    {
        this->m_flags |= 0x100u;
        NitrousVehicle::unpause_physics();
    }
    else
    {
        this->m_flags |= 0x80u;
        if ( (this->m_flags & 1) != 0 )
        {
            currentOrigin = this->m_owner->r.currentOrigin;
            origin[0] = *currentOrigin;
            origin[1] = currentOrigin[1];
            origin[2] = currentOrigin[2];
            origin[2] = origin[2] + 40.0;
            AnglesToAxis(this->m_owner->r.currentAngles, (float (*)[3])axis);
            Phys_AxisToNitrousMat((float (*)[3])axis, &this->m_prev_rb_mat);
            Phys_Vec3ToNitrousVec(origin, &this->m_prev_rb_mat.w);
            if ( fabs(this->m_prev_rb_mat.w.x) > 100000.0
                || fabs(this->m_prev_rb_mat.w.y) > 100000.0
                || fabs(this->m_prev_rb_mat.w.z) > 100000.0 )
            {
                phys_exec_debug_callback(0);
            }
            SV_LinkEntity(this->m_owner);
        }
        else if ( !this->m_vpc )
        {
            this->m_vpc = path_constraint_create(this->m_owner);
            for ( i = 0; i < 6; ++i )
            {
                rbc_wheel = this->m_wheels[i];
                if ( rbc_wheel )
                {
                    rbc_wheel->m_fwd_fric_k = 0.01f;
                    rbc_wheel->m_side_fric_k = 0.01f;
                }
            }
        }
    }
    Sys_LeaveCriticalSection(CRITSECT_PHYSICS);
    Sys_LeaveCriticalSection(CRITSECT_PHYSICS_UPDATE);
}

void __thiscall NitrousVehicle::end_path()
{
    Sys_EnterCriticalSection(CRITSECT_PHYSICS_UPDATE);
    Sys_EnterCriticalSection(CRITSECT_PHYSICS);
    this->m_flags &= ~0x100u;
    this->m_flags &= ~0x80u;
    NitrousVehicle::cleanup_path();
    Sys_LeaveCriticalSection(CRITSECT_PHYSICS);
    Sys_LeaveCriticalSection(CRITSECT_PHYSICS_UPDATE);
}

void __thiscall NitrousVehicle::cleanup_path()
{
    if ( this->m_vpc )
    {
        path_constraint_destroy(this->m_vpc);
        this->m_vpc = 0;
    }
}

void __cdecl NitrousVehicle::frame_prolog_all_systems(float delta_t)
{
#if 0
    phys_free_list<NitrousVehicle>::T_internal_base *i; // [esp+14h] [ebp-8h]
    int savedregs; // [esp+1Ch] [ebp+0h] BYREF

    //PIXBeginNamedEvent(-1, "NitrousVehicle::frame_prolog");
    for ( i = g_rb_vehicle_list.m_dummy_head.m_next_T_internal; &g_rb_vehicle_list != (phys_free_list<NitrousVehicle> *)i; i = i->m_next_T_internal )
    {
        //NitrousVehicle::_update_prolog((NitrousVehicle *)&i[2], (int)&savedregs, delta_t);
        ((NitrousVehicle *)&i[2])->_update_prolog(delta_t);
    }
    //if ( g_DXDeviceThread == GetCurrentThreadId() )
        //D3DPERF_EndEvent();
#endif

    using List = phys_free_list<NitrousVehicle>;
    using Node = List::T_internal;
    using NodeBase = List::T_internal_base;

    NodeBase *head = &g_rb_vehicle_list.m_dummy_head;

    PROF_SCOPED("NitrousVehicle::frame_prolog");

    for (NodeBase *it = head->m_next_T_internal;
        it != head;
        it = it->m_next_T_internal)
    {
        Node *node = static_cast<Node *>(it);
        NitrousVehicle *vehicle = &node->m_data;

        vehicle->_update_prolog(delta_t);
    }
}

void __cdecl NitrousVehicle::frame_epilog_all_systems(float delta_t)
{
#if 0
    phys_free_list<NitrousVehicle>::T_internal_base *i; // [esp+14h] [ebp-8h]

    //PIXBeginNamedEvent(-1, "NitrousVehicle::frame_epilog");
    for ( i = g_rb_vehicle_list.m_dummy_head.m_next_T_internal; &g_rb_vehicle_list != (phys_free_list<NitrousVehicle> *)i; i = i->m_next_T_internal )
    {
        //NitrousVehicle::_update_epilog((NitrousVehicle *)&i[2], delta_t);
        ((NitrousVehicle *)&i[2])->_update_epilog(delta_t);
    }
    //if ( g_DXDeviceThread == GetCurrentThreadId() )
        //D3DPERF_EndEvent();
#endif

    using List = phys_free_list<NitrousVehicle>;
    using Node = List::T_internal;
    using NodeBase = List::T_internal_base;

    NodeBase *head = &g_rb_vehicle_list.m_dummy_head;

    PROF_SCOPED("NitrousVehicle::frame_epilog");

    for (NodeBase *it = head->m_next_T_internal;
        it != head;
        it = it->m_next_T_internal)
    {
        Node *node = static_cast<Node *>(it);
        NitrousVehicle *vehicle = &node->m_data;

        vehicle->_update_epilog(delta_t);
    }
}

NitrousVehicle *__cdecl NitrousVehicle::add_vehicle(int id)
{
    phys_free_list<NitrousVehicle> *i; // [esp+24h] [ebp-8h]
    NitrousVehicle *vehicle; // [esp+28h] [ebp-4h]

    if (id != -1)
    {
        for (i = (phys_free_list<NitrousVehicle> *)g_rb_vehicle_list.m_dummy_head.m_next_T_internal;
            &g_rb_vehicle_list != i;
            i = (phys_free_list<NitrousVehicle> *)i->m_dummy_head.m_next_T_internal)
        {
            if (i->m_ptr_list[244] == (NitrousVehicle *)id)
            {
                ++i->m_ptr_list[245];
                return (NitrousVehicle*)i->m_ptr_list;
            }
        }
    }
    //vehicle = phys_free_list<NitrousVehicle>::add(&g_rb_vehicle_list, 1, "phys_free_list error: out of memory.");
    vehicle = g_rb_vehicle_list.add(1, "phys_free_list error: out of memory.");
    if ((!vehicle || vehicle->refcount)
        && _tlAssert(
            "C:\\projects_pc\\cod\\codsrc\\src\\vehicle\\nitrous_vehicle.cpp",
            1553,
            "vehicle && vehicle->refcount == 0",
            "Failed to add a new NitrousVehicle."))
    {
        __debugbreak();
    }
    if ((vehicle->m_flags & 1) == 0
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\vehicle\\nitrous_vehicle.cpp",
            1554,
            0,
            "%s",
            "vehicle->get_flag(FLAG_PAUSED)"))
    {
        __debugbreak();
    }
    vehicle->id = id;
    vehicle->refcount = 1;
    return vehicle;
}

void __cdecl NitrousVehicle::reinit_parms()
{
    using List = phys_free_list<NitrousVehicle>;
    using Node = List::T_internal;
    using NodeBase = List::T_internal_base;

    NodeBase *head = &g_rb_vehicle_list.m_dummy_head;

    for (NodeBase *it = head->m_next_T_internal;
        it != head;
        it = it->m_next_T_internal)
    {
        Node *node = static_cast<Node *>(it);
        NitrousVehicle *vehicle = &node->m_data;

        vehicle->update_parms(vehicle->m_parameter, 0);
    }

    //phys_free_list<NitrousVehicle>::T_internal_base *i; // [esp+10h] [ebp-4h]
    //int savedregs; // [esp+14h] [ebp+0h] BYREF
    //
    //for ( i = g_rb_vehicle_list.m_dummy_head.m_next_T_internal;
    //            &g_rb_vehicle_list != (phys_free_list<NitrousVehicle> *)i;
    //            i = i->m_next_T_internal )
    //{
    //    NitrousVehicle::update_parms(
    //        (NitrousVehicle *)&i[2],
    //        (int)&savedregs,
    //        (const VehicleParameter *)i[64].m_prev_T_internal,
    //        0);
    //}
}

void __cdecl NitrousVehicle::reset_vehicle_physics()
{
    using List = phys_free_list<NitrousVehicle>;
    using Node = List::T_internal;
    using NodeBase = List::T_internal_base;

    NodeBase *head = &g_rb_vehicle_list.m_dummy_head;

    for (NodeBase *it = head->m_next_T_internal;
        it != head;
        it = it->m_next_T_internal)
    {
        Node *node = static_cast<Node *>(it);
        NitrousVehicle *vehicle = &node->m_data;

        vehicle->pause_physics(false);
        vehicle->unpause_physics();
    }

    //phys_free_list<NitrousVehicle>::T_internal_base *i; // [esp+Ch] [ebp-4h]
    //int savedregs; // [esp+10h] [ebp+0h] BYREF
    //
    //for ( i = g_rb_vehicle_list.m_dummy_head.m_next_T_internal;
    //            &g_rb_vehicle_list != (phys_free_list<NitrousVehicle> *)i;
    //            i = i->m_next_T_internal )
    //{
    //    NitrousVehicle::pause_physics((NitrousVehicle *)&i[2], 0);
    //    NitrousVehicle::unpause_physics((NitrousVehicle *)&i[2], (int)&savedregs);
    //}
}

void __cdecl NitrousVehicle::remove_vehicle(NitrousVehicle *const v)
{
    if ( !--v->refcount )
    {
        //NitrousVehicle::pause_physics(v, 1);
        v->pause_physics(true);
        //phys_free_list<NitrousVehicle>::remove(&g_rb_vehicle_list, v);
        g_rb_vehicle_list.remove(v);
    }
}

void __cdecl NitrousVehicle::debug_render_all()
{
    using List = phys_free_list<NitrousVehicle>;
    using Node = List::T_internal;
    using NodeBase = List::T_internal_base;

    NodeBase *head = &g_rb_vehicle_list.m_dummy_head;

    for (NodeBase *it = head->m_next_T_internal;
        it != head;
        it = it->m_next_T_internal)
    {
        Node *node = static_cast<Node *>(it);
        NitrousVehicle *vehicle = &node->m_data;

        vehicle->debug_render();
    }

    //phys_free_list<NitrousVehicle>::T_internal_base *i; // [esp+8h] [ebp-4h]
    //int savedregs; // [esp+Ch] [ebp+0h] BYREF
    //
    //for ( i = g_rb_vehicle_list.m_dummy_head.m_next_T_internal;
    //            &g_rb_vehicle_list != (phys_free_list<NitrousVehicle> *)i;
    //            i = i->m_next_T_internal )
    //{
    //    NitrousVehicle::debug_render((NitrousVehicle *)&i[2], (int)&savedregs);
    //}
}

void    NitrousVehicle::debug_render()
{
    //phys_vec3 *v2; // eax
    const phys_vec3 *v3; // eax
    const phys_vec3 *v4; // eax
    rigid_body *body; // ecx
    phys_vec3 *scale_4; // [esp+10h] [ebp-640h]
    phys_vec3 *scale_4a; // [esp+10h] [ebp-640h]
    char buffer[508]; // [esp+1Ch] [ebp-634h] BYREF
    float textPos[7]; // [esp+218h] [ebp-438h] BYREF
    float v10; // [esp+234h] [ebp-41Ch]
    float v11; // [esp+238h] [ebp-418h]
    float v12; // [esp+23Ch] [ebp-414h]
    phys_mat44 *v13; // [esp+240h] [ebp-410h]
    float v14; // [esp+244h] [ebp-40Ch]
    float v15; // [esp+248h] [ebp-408h]
    float v16; // [esp+24Ch] [ebp-404h]
    float v17; // [esp+254h] [ebp-3FCh]
    float v18; // [esp+258h] [ebp-3F8h]
    float v19; // [esp+25Ch] [ebp-3F4h]
    phys_mat44 *v20; // [esp+260h] [ebp-3F0h]
    float v21; // [esp+264h] [ebp-3ECh]
    float z; // [esp+268h] [ebp-3E8h]
    float arm_col[4]; // [esp+26Ch] [ebp-3E4h] BYREF
    float _pos[3]; // [esp+27Ch] [ebp-3D4h] BYREF
    float _dir[3]; // [esp+288h] [ebp-3C8h] BYREF
    phys_vec3 dir; // [esp+294h] [ebp-3BCh] BYREF
    phys_vec3 m_b1_wheel_axis_loc; // [esp+2A4h] [ebp-3ACh] BYREF
    phys_vec3 v28; // [esp+2B4h] [ebp-39Ch] BYREF
    phys_vec3 v29; // [esp+2C4h] [ebp-38Ch] BYREF
    phys_mat44 *v30; // [esp+2E0h] [ebp-370h]
    phys_vec3 limit_pos; // [esp+2E4h] [ebp-36Ch] BYREF
    float v32; // [esp+2F8h] [ebp-358h]
    float v33; // [esp+2FCh] [ebp-354h]
    float v34; // [esp+300h] [ebp-350h]
    phys_vec3 m_b1_wheel_center_loc; // [esp+304h] [ebp-34Ch]
    float v36; // [esp+314h] [ebp-33Ch]
    float v37; // [esp+318h] [ebp-338h]
    float v38; // [esp+31Ch] [ebp-334h]
    float v39; // [esp+324h] [ebp-32Ch]
    float v40; // [esp+328h] [ebp-328h]
    float v41; // [esp+32Ch] [ebp-324h]
    float v42; // [esp+330h] [ebp-320h]
    phys_vec3 v43; // [esp+334h] [ebp-31Ch]
    float m_hard_limit_dist; // [esp+350h] [ebp-300h]
    phys_vec3 hit_pos_displaced; // [esp+354h] [ebp-2FCh] BYREF
    float v46; // [esp+368h] [ebp-2E8h]
    float v47; // [esp+36Ch] [ebp-2E4h]
    float v48; // [esp+370h] [ebp-2E0h]
    float v49; // [esp+374h] [ebp-2DCh]
    float v50; // [esp+378h] [ebp-2D8h]
    float v51; // [esp+37Ch] [ebp-2D4h]
    float v52; // [esp+384h] [ebp-2CCh]
    float v53; // [esp+388h] [ebp-2C8h]
    float v54; // [esp+38Ch] [ebp-2C4h]
    const phys_vec3 *v55; // [esp+390h] [ebp-2C0h]
    phys_vec3 v56; // [esp+394h] [ebp-2BCh] BYREF
    phys_vec3 m_b1_suspension_dir_loc; // [esp+3A4h] [ebp-2ACh] BYREF
    float m_roll_stability_factor; // [esp+3C0h] [ebp-290h]
    phys_vec3 hit_pos; // [esp+3C4h] [ebp-28Ch] BYREF
    phys_vec3 v60; // [esp+3D4h] [ebp-27Ch] BYREF
    phys_vec3 v61; // [esp+3E4h] [ebp-26Ch] BYREF
    phys_mat44 *v62; // [esp+400h] [ebp-250h]
    phys_vec3 wheel_pos; // [esp+404h] [ebp-24Ch] BYREF
    phys_vec3 v64; // [esp+414h] [ebp-23Ch] BYREF
    float radius; // [esp+430h] [ebp-220h]
    rigid_body *b1; // [esp+434h] [ebp-21Ch]
    rigid_body_constraint_wheel *rbc_wheel; // [esp+438h] [ebp-218h]
    float col[4]; // [esp+43Ch] [ebp-214h] BYREF
    int i; // [esp+44Ch] [ebp-204h]
    float alpha; // [esp+450h] [ebp-200h]
    phys_vec3 v71; // [esp+454h] [ebp-1FCh] BYREF
    float v72; // [esp+468h] [ebp-1E8h]
    float v73; // [esp+46Ch] [ebp-1E4h]
    float v74; // [esp+470h] [ebp-1E0h]
    phys_mat44 v75; // [esp+474h] [ebp-1DCh] BYREF
    phys_vec3 v76; // [esp+4B4h] [ebp-19Ch] BYREF
    float v77; // [esp+4C8h] [ebp-188h]
    float v78; // [esp+4CCh] [ebp-184h]
    float v79; // [esp+4D0h] [ebp-180h]
    float v80; // [esp+4D4h] [ebp-17Ch]
    float v81; // [esp+4D8h] [ebp-178h]
    float v82; // [esp+4DCh] [ebp-174h]
    float v83; // [esp+4E8h] [ebp-168h]
    float v84; // [esp+4ECh] [ebp-164h]
    float v85; // [esp+4F0h] [ebp-160h]
    float v86; // [esp+4F4h] [ebp-15Ch]
    float v87; // [esp+4F8h] [ebp-158h]
    float v88; // [esp+4FCh] [ebp-154h]
    float v89; // [esp+508h] [ebp-148h]
    float v90; // [esp+50Ch] [ebp-144h]
    float v91; // [esp+510h] [ebp-140h]
    phys_vec3 v92; // [esp+514h] [ebp-13Ch] BYREF
    float v93; // [esp+524h] [ebp-12Ch] BYREF
    phys_vec3 v94; // [esp+528h] [ebp-128h] BYREF
    float v95; // [esp+538h] [ebp-118h]
    float v96; // [esp+53Ch] [ebp-114h]
    float v97; // [esp+548h] [ebp-108h]
    float v98; // [esp+54Ch] [ebp-104h]
    float v99; // [esp+550h] [ebp-100h]
    phys_mat44 transform; // [esp+554h] [ebp-FCh] BYREF
    phys_vec3 *p_w; // [esp+5A0h] [ebp-B0h]
    phys_vec3 com; // [esp+5A4h] [ebp-ACh] BYREF
    float v103; // [esp+5B8h] [ebp-98h]
    float v104; // [esp+5BCh] [ebp-94h]
    float v105; // [esp+5C0h] [ebp-90h]
    float v106; // [esp+5C4h] [ebp-8Ch]
    float v107; // [esp+5C8h] [ebp-88h]
    float v108; // [esp+5CCh] [ebp-84h]
    float v109; // [esp+5D8h] [ebp-78h]
    float v110; // [esp+5DCh] [ebp-74h]
    float v111; // [esp+5E0h] [ebp-70h]
    phys_vec3 dim; // [esp+5E4h] [ebp-6Ch]
    float v113; // [esp+5F8h] [ebp-58h]
    float v114; // [esp+5FCh] [ebp-54h]
    float v115; // [esp+600h] [ebp-50h]
    phys_vec3 vmax; // [esp+604h] [ebp-4Ch] BYREF
    phys_vec3 vmin; // [esp+614h] [ebp-3Ch] BYREF
    phys_vec3 pos; // [esp+624h] [ebp-2Ch] BYREF
    phys_mat44 *p_m_mat; // [esp+638h] [ebp-18h]
    NitrousVehicle *thisa; // [esp+63Ch] [ebp-14h]
    //_UNKNOWN *v121[2]; // [esp+644h] [ebp-Ch] BYREF
    //int vars0; // [esp+650h] [ebp+0h]
    //
    //v121[0] = a2;
    //v121[1] = (_UNKNOWN *)vars0;
    thisa = this;
    if ((phys_drawNitrousVehicle->current.enabled || (thisa->m_flags & 8) != 0) && thisa->m_phys_user_data)
    {
        p_m_mat = &thisa->m_phys_user_data->body->m_mat;
        pos = p_m_mat->w;
        Phys_DebugBox(&thisa->m_phys_user_data->body->m_mat, colorYellow, 1.0);
        if (thisa->m_parameter->m_bbox_min[0] != 0.0)
        {
            Phys_Vec3ToNitrousVec(thisa->m_parameter->m_bbox_min, &vmin);
            Phys_Vec3ToNitrousVec(thisa->m_parameter->m_bbox_max, &vmax);
            v115 = vmax.x - vmin.x;
            v114 = vmax.y - vmin.y;
            v113 = vmax.z - vmin.z;
            dim.x = vmax.x - vmin.x;
            dim.y = vmax.y - vmin.y;
            dim.z = vmax.z - vmin.z;
            v111 = vmax.x + vmin.x;
            v110 = vmax.y + vmin.y;
            v109 = vmax.z + vmin.z;
            v106 = vmax.x + vmin.x;
            v107 = vmax.y + vmin.y;
            v108 = vmax.z + vmin.z;
            v105 = 0.5 * (float)(vmax.x + vmin.x);
            v104 = 0.5 * (float)(vmax.y + vmin.y);
            v103 = 0.5 * (float)(vmax.z + vmin.z);
            com.x = v105;
            com.y = v104;
            com.z = v103;
            p_w = &thisa->m_phys_user_data->cg2rb.w;
            com.x = v105 + p_w->x;
            com.y = v104 + p_w->y;
            com.z = v103 + p_w->z;
            //phys_mat44::operator=(&transform, &PHYS_IDENTITY_MATRIX_34);
            transform = PHYS_IDENTITY_MATRIX;
            //phys_vec3::operator=(&transform.w, &com);
            transform.w = com;
            phys_full_multiply_mat(&transform, &transform, &thisa->m_phys_user_data->body->m_mat);
            v99 = 0.5 * dim.x;
            v98 = 0.5 * dim.y;
            v97 = 0.5 * dim.z;
            v94.w = 0.5 * dim.x;
            v95 = 0.5 * dim.y;
            v96 = 0.5 * dim.z;
            Phys_DebugBox(&transform, (phys_vec3 *)&v94.w, colorGreen);
        }
        if (thisa->m_parameter->m_buoyancybox_min[0] != 0.0)
        {
            Phys_Vec3ToNitrousVec(thisa->m_parameter->m_buoyancybox_min, (phys_vec3 *)&v93);
            Phys_Vec3ToNitrousVec(thisa->m_parameter->m_buoyancybox_max, &v92);
            v91 = v92.x - v93;
            v90 = v92.y - v94.x;
            v89 = v92.z - v94.y;
            v86 = v92.x - v93;
            v87 = v92.y - v94.x;
            v88 = v92.z - v94.y;
            v85 = v92.x + v93;
            v84 = v92.y + v94.x;
            v83 = v92.z + v94.y;
            v80 = v92.x + v93;
            v81 = v92.y + v94.x;
            v82 = v92.z + v94.y;
            v79 = 0.5 * (float)(v92.x + v93);
            v78 = 0.5 * (float)(v92.y + v94.x);
            v77 = 0.5 * (float)(v92.z + v94.y);
            v76.x = v79;
            v76.y = v78;
            v76.z = v77;
            //phys_mat44::operator=(&v75, &PHYS_IDENTITY_MATRIX_34);
            v75 = PHYS_IDENTITY_MATRIX;
            //phys_vec3::operator=(&v75.w, &v76);
            v75.w = v76;
            phys_full_multiply_mat(&v75, &v75, &thisa->m_phys_user_data->body->m_mat);
            v74 = 0.5 * v86;
            v73 = 0.5 * v87;
            v72 = 0.5 * v88;
            v71.x = 0.5 * v86;
            v71.y = 0.5 * v87;
            v71.z = 0.5 * v88;
            Phys_DebugBox(&v75, &v71, colorBlue);
        }
        alpha = 0.18f;
        for (i = 0; i < 6; ++i)
        {
            col[0] = 0.0f;
            col[1] = 1.0f;
            col[2] = 0.0f;
            col[3] = alpha;
            rbc_wheel = thisa->m_wheels[i];
            if (rbc_wheel)
            {
                b1 = rbc_wheel->b1;
                radius = rbc_wheel->m_wheel_radius;
                //v2 = operator*(&v64, rbc_wheel->m_wheel_displaced_center_dist, &rbc_wheel->m_b1_suspension_dir_loc);
                v64 = rbc_wheel->m_b1_suspension_dir_loc * rbc_wheel->m_wheel_displaced_center_dist;
                //operator-(&wheel_pos, &rbc_wheel->m_b1_wheel_center_loc, v2);
                wheel_pos = rbc_wheel->m_b1_wheel_center_loc - v64;
                v62 = &b1->m_mat;
                scale_4 = &b1->m_mat.w;
                v3 = phys_multiply(&v61, &b1->m_mat, &wheel_pos);
                //operator+(&v60, v3, scale_4);
                v60 = *v3 + *scale_4;
                wheel_pos.x = v60.x;
                wheel_pos.y = v60.y;
                wheel_pos.z = v60.z;
                hit_pos = rbc_wheel->m_b2_hitp_loc;
                m_roll_stability_factor = rbc_wheel->m_roll_stability_factor;
                m_b1_suspension_dir_loc = rbc_wheel->m_b1_suspension_dir_loc;
                v55 = phys_multiply(&v56, &b1->m_mat, &m_b1_suspension_dir_loc);
                v54 = m_roll_stability_factor * v55->x;
                v53 = m_roll_stability_factor * v55->y;
                v52 = m_roll_stability_factor * v55->z;
                v49 = v54;
                v50 = v53;
                v51 = v52;
                v48 = hit_pos.x - v54;
                v47 = hit_pos.y - v53;
                v46 = hit_pos.z - v52;
                hit_pos_displaced.x = hit_pos.x - v54;
                hit_pos_displaced.y = hit_pos.y - v53;
                hit_pos_displaced.z = hit_pos.z - v52;
                if ((rbc_wheel->m_wheel_flags & 1) != 0)
                {
                    col[0] = 1.0f;
                    col[1] = 1.0f;
                    col[2] = 0.0f;
                    col[3] = alpha;
                }
                if ((rbc_wheel->m_wheel_flags & 4) != 0)
                {
                    col[0] = 1.0f;
                    col[1] = 0.0f;
                    col[2] = 0.0f;
                    col[3] = alpha;
                }
                if ((rbc_wheel->m_wheel_flags & 2) != 0)
                {
                    m_hard_limit_dist = rbc_wheel->m_hard_limit_dist;
                    v43 = rbc_wheel->m_b1_suspension_dir_loc;
                    v42 = m_hard_limit_dist + radius;
                    v41 = (float)(m_hard_limit_dist + radius) * v43.x;
                    v40 = (float)(m_hard_limit_dist + radius) * v43.y;
                    v39 = (float)(m_hard_limit_dist + radius) * v43.z;
                    v36 = v41;
                    v37 = v40;
                    v38 = v39;
                    m_b1_wheel_center_loc = rbc_wheel->m_b1_wheel_center_loc;
                    v34 = m_b1_wheel_center_loc.x - v41;
                    v33 = m_b1_wheel_center_loc.y - v40;
                    v32 = m_b1_wheel_center_loc.z - v39;
                    limit_pos.x = m_b1_wheel_center_loc.x - v41;
                    limit_pos.y = m_b1_wheel_center_loc.y - v40;
                    limit_pos.z = m_b1_wheel_center_loc.z - v39;
                    v30 = &b1->m_mat;
                    scale_4a = &b1->m_mat.w;
                    v4 = phys_multiply(&v29, &b1->m_mat, &limit_pos);
                    //operator+(&v28, v4, scale_4a);
                    v28 = *v4 + *scale_4a;
                    limit_pos.x = v28.x;
                    limit_pos.y = v28.y;
                    limit_pos.z = v28.z;
                    if ((rbc_wheel->m_wheel_flags & 2) != 0)
                        Phys_DebugPoint(&limit_pos, sphere_size, colorRed);
                    else
                        Phys_DebugPoint(&limit_pos, sphere_size, colorGreen);
                }
                Phys_DebugPoint(&hit_pos, sphere_size, colorRed);
                Phys_DebugPoint(&hit_pos_displaced, sphere_size, colorBlue);
                Phys_DebugPoint(&wheel_pos, radius, col);
                if (thisa->m_vehicle_info->type == 2 || render_wheel_cir)
                {
                    m_b1_wheel_axis_loc = rbc_wheel->m_b1_wheel_axis_loc;
                    phys_multiply(&dir, &b1->m_mat, &m_b1_wheel_axis_loc);
                    Phys_NitrousVecToVec3(&dir, _dir);
                    Phys_NitrousVecToVec3(&wheel_pos, _pos);
                    G_DebugCircleEx(_pos, radius, _dir, colorWhite, 0, 0);
                }
                arm_col[0] = 0.0f;
                arm_col[1] = 1.0f;
                arm_col[2] = 1.0f;
                arm_col[3] = 1.0f;
                z = pos.z;
                v21 = hit_pos_displaced.z;
                if (hit_pos_displaced.z > (float)(pos.z - 5.0))
                {
                    arm_col[0] = 1.0f;
                    arm_col[1] = 0.0f;
                    arm_col[2] = 0.0f;
                    arm_col[3] = 1.0f;
                }
                render_line(&pos, &hit_pos_displaced, arm_col, 0, 0);
            }
        }
        v20 = &thisa->m_phys_user_data->body->m_mat;
        v19 = 130.0 * v20->x.x;
        v18 = 130.0 * v20->x.y;
        v17 = 130.0 * v20->x.z;
        v14 = v19;
        v15 = v18;
        v16 = v17;
        pos.x = pos.x + v19;
        pos.y = pos.y + v18;
        pos.z = pos.z + v17;
        if ((thisa->m_flags & 0x40) != 0)
        {
            body = thisa->m_phys_user_data->body;
            v13 = &body->m_mat;
            v12 = 100.0 * body->m_mat.x.x;
            v11 = 100.0 * body->m_mat.x.y;
            v10 = 100.0 * body->m_mat.x.z;
            textPos[3] = v12;
            textPos[4] = v11;
            textPos[5] = v10;
            pos.x = pos.x + v12;
            pos.y = pos.y + v11;
            pos.z = pos.z + v10;
        }
        Phys_NitrousVecToVec3(&pos, textPos);
        textPos[2] = textPos[2] + 50.0;
        sprintf(buffer, "Speed %.1f %.0f", (float)(thisa->m_forward_vel / 17.6), thisa->m_forward_vel);
        CL_AddDebugString(textPos, colorWhite, 1.0, buffer, 1);
        textPos[2] = textPos[2] - 20.0;
        sprintf(buffer, "Rpm %.2f", thisa->m_fake_rpm);
        CL_AddDebugString(textPos, colorWhite, 1.0, buffer, 1);
        if (snd_boat_using_lerp_rpm && snd_boat_using_lerp_rpm->current.enabled)
        {
            textPos[2] = textPos[2] - 20.0;
            sprintf(buffer, "boat rpm %.2f", snd_boat_current_rpm->current.value);
            CL_AddDebugString(textPos, colorWhite, 1.0, buffer, 1);
        }
    }
    NitrousVehicle::debug_render_effects();
}

// local variable allocation has failed, the output may be wrong!
void    phys_full_multiply_mat(phys_mat44 *dest, const phys_mat44 *left, const phys_mat44 *right)
{
    phys_mat44 v4; // [esp-Ch] [ebp-CCh] BYREF
    phys_vec3 *temp_60; // [esp+3Ch] [ebp-84h]
    const phys_vec3 *v6; // [esp+40h] [ebp-80h]
    phys_vec3 v7; // [esp+44h] [ebp-7Ch] BYREF
    phys_vec3 *p_z; // [esp+5Ch] [ebp-64h]
    const phys_vec3 *v9; // [esp+60h] [ebp-60h]
    phys_vec3 v10; // [esp+64h] [ebp-5Ch] BYREF
    phys_vec3 *p_y; // [esp+7Ch] [ebp-44h]
    const phys_vec3 *v12; // [esp+80h] [ebp-40h]
    phys_vec3 v13; // [esp+84h] [ebp-3Ch] BYREF
    const phys_vec3 *v14; // [esp+A0h] [ebp-20h]
    phys_vec3 v15; // [esp+A4h] [ebp-1Ch] BYREF
    //_UNKNOWN *v16[2]; // [esp+B4h] [ebp-Ch] BYREF
    //const phys_mat44 *righta; // [esp+C0h] [ebp+0h]
    //
    //v16[0] = a1;
    //v16[1] = righta;

    assert_mat44_initialized(*left);
    assert_mat44_initialized(*right);

    if (dest == left)
    {
        memcpy(&v4, left, sizeof(v4));
        phys_full_multiply_mat(dest, &v4, right);
    }
    else
    {
        v14 = phys_multiply(&v15, left, &right->x);
        dest->x.x = v14->x;
        dest->x.y = v14->y;
        dest->x.z = v14->z;
        v12 = phys_multiply(&v13, left, &right->y);
        p_y = &dest->y;
        dest->y.x = v12->x;
        p_y->y = v12->y;
        p_y->z = v12->z;
        v9 = phys_multiply(&v10, left, &right->z);
        p_z = &dest->z;
        dest->z.x = v9->x;
        p_z->y = v9->y;
        p_z->z = v9->z;
        v6 = phys_full_multiply(&v7, left, &right->w);
        temp_60 = &dest->w;
        dest->w.x = v6->x;
        temp_60->y = v6->y;
        temp_60->z = v6->z;
    }
}

void __thiscall NitrousVehicle::remove_wheels()
{
    int i; // [esp+4h] [ebp-4h]

    for ( i = 0; i < 6; ++i )
    {
        if ( this->m_wheels[i] )
            this->m_wheels[i] = 0;
    }
}

void __thiscall NitrousVehicle::set(
                float power_braking_factor,
                float braking_factor,
                float desired_speed_factor,
                float acceleration_factor,
                float coasting_factor,
                float reference_wheel_radius,
                float steer_max_angle,
                float steer_speed,
                phys_vec3 *steer_front_pt_loc,
                float steer_front_back_length)
{
    this->m_power_braking_factor = power_braking_factor;
    this->m_braking_factor = braking_factor;
    this->m_desired_speed_factor = desired_speed_factor;
    this->m_acceleration_factor = acceleration_factor;
    this->m_coasting_factor = coasting_factor;
    this->m_reference_wheel_radius = reference_wheel_radius;
    this->m_steer_max_angle = steer_max_angle;
    this->m_steer_speed = steer_speed;
    this->m_steer_front_pt_loc.x = steer_front_pt_loc->x;
    this->m_steer_front_pt_loc.y = steer_front_pt_loc->y;
    this->m_steer_front_pt_loc.z = steer_front_pt_loc->z;
    this->m_steer_front_back_length = steer_front_back_length;
    this->m_steer_factor = 0.0f;
    this->m_steer_current_angle = 0.0f;
    this->m_state_flags = 0;
    this->m_forward_vel = 0.0f;
}

// local variable allocation has failed, the output may be wrong!
void NitrousVehicle::update_steering(float delta_t)
{
    float v3; // xmm0_4
    float v4; // xmm0_4
    int m; // [esp-Ch] [ebp-144h]
    float v6; // [esp-8h] [ebp-140h]
    float v7; // [esp-4h] [ebp-13Ch]
    phys_vec3 *p_m_b1_wheel_axis_loc; // [esp+0h] [ebp-138h]
    float v9; // [esp+14h] [ebp-124h]
    float nwheel_axis; // [esp+18h] [ebp-120h]
    phys_vec3 wheel_axis; // [esp+1Ch] [ebp-11Ch] BYREF
    float v12; // [esp+30h] [ebp-108h]
    float v13; // [esp+34h] [ebp-104h]
    float v14; // [esp+38h] [ebp-100h]
    float x; // [esp+3Ch] [ebp-FCh]
    float y; // [esp+40h] [ebp-F8h]
    float z; // [esp+44h] [ebp-F4h]
    float w; // [esp+48h] [ebp-F0h]
    rigid_body_constraint_wheel *v19; // [esp+54h] [ebp-E4h]
    int k; // [esp+58h] [ebp-E0h]
    float wheel_ratios[6]; // [esp+5Ch] [ebp-DCh] BYREF
    float total_ratio; // [esp+74h] [ebp-C4h]
    int num_accel_wheels; // [esp+78h] [ebp-C0h]
    phys_vec3 turning_center; // [esp+7Ch] [ebp-BCh]
    float v25; // [esp+8Ch] [ebp-ACh]
    float v26; // [esp+90h] [ebp-A8h]
    float v27; // [esp+94h] [ebp-A4h]
    phys_vec3 *p_m_steer_front_pt_loc; // [esp+98h] [ebp-A0h]
    float v29; // [esp+9Ch] [ebp-9Ch]
    float v30; // [esp+A0h] [ebp-98h]
    int v31; // [esp+A4h] [ebp-94h]
    float v32; // [esp+ACh] [ebp-8Ch]
    float v33; // [esp+B0h] [ebp-88h]
    float v34; // [esp+B4h] [ebp-84h]
    float turning_center_radius; // [esp+B8h] [ebp-80h]
    phys_vec3 turning_center_radius_vec; // [esp+BCh] [ebp-7Ch] BYREF
    float v37; // [esp+CCh] [ebp-6Ch]
    float v38; // [esp+D0h] [ebp-68h]
    int v39; // [esp+D4h] [ebp-64h]
    float v40; // [esp+E4h] [ebp-54h]
    float width; // [esp+E8h] [ebp-50h]
    float v42; // [esp+ECh] [ebp-4Ch]
    float v43; // [esp+F0h] [ebp-48h]
    float m_hand_brake; // [esp+F4h] [ebp-44h]
    int j; // [esp+F8h] [ebp-40h]
    bool all_wheel_steering; // [esp+FFh] [ebp-39h]
    rigid_body_constraint_wheel *v47; // [esp+100h] [ebp-38h]
    rigid_body_constraint_wheel *v48; // [esp+104h] [ebp-34h]
    phys_vec3 *v49; // [esp+108h] [ebp-30h]
    rigid_body_constraint_wheel *v50; // [esp+10Ch] [ebp-2Ch]
    int i; // [esp+110h] [ebp-28h]
    float m_steer_current_angle; // [esp+114h] [ebp-24h]
    float angle_delta; // [esp+118h] [ebp-20h]
    float max_delta_angle; // [esp+11Ch] [ebp-1Ch]
    float new_steer_angle; // [esp+120h] [ebp-18h]
    float steer; // [esp+124h] [ebp-14h]
    NitrousVehicle *thisa; // [esp+128h] [ebp-10h]
    //_UNKNOWN *v58; // [esp+12Ch] [ebp-Ch]
    //float delta_ta; // [esp+130h] [ebp-8h]
    //int vars0; // [esp+138h] [ebp+0h]
    //
    //v58 = a2;
    //LODWORD(delta_ta) = vars0;
    thisa = this;
    steer = this->m_steer_factor;
    if (steer <= 1.0)
    {
        if (steer < -1.0)
            steer = -1.0f;
    }
    else
    {
        steer = 1.0f;
    }
    new_steer_angle = thisa->m_steer_max_angle * steer;
    max_delta_angle = thisa->m_steer_speed * delta_t;
    angle_delta = new_steer_angle - thisa->m_steer_current_angle;
    if (angle_delta <= max_delta_angle)
    {
        if ((-(max_delta_angle)) > angle_delta)
            (angle_delta) = -(max_delta_angle);
    }
    else
    {
        angle_delta = max_delta_angle;
    }
    thisa->m_steer_current_angle = thisa->m_steer_current_angle + angle_delta;
    m_steer_current_angle = thisa->m_steer_current_angle;
    if ((fabs(m_steer_current_angle)) > 0.0049999999)
    {
        all_wheel_steering = 1;
        for (j = 0; j < 6; ++j)
        {
            if (thisa->m_wheels[j])
            {
                if ((thisa->m_wheels[j]->m_wheel_flags & 8) == 0
                    || (m_hand_brake = thisa->m_hand_brake, m_hand_brake > 0.1) && (thisa->m_wheels[j]->m_wheel_flags & 0x80) != 0)
                {
                    all_wheel_steering = 0;
                    break;
                }
            }
        }
        if (all_wheel_steering)
        {
            v43 = thisa->m_steer_current_angle;
            //v3 = __libm_sse2_tan(v43);
            v3 = tan(v43);
            v42 = v3;
            width = (float)(thisa->m_steer_front_back_length * 0.5) / v3;
            v40 = (-(thisa->m_steer_front_back_length)) * 0.5;
            v37 = v40;
            v38 = width;
            v39 = 0.0f;
            turning_center_radius_vec.x = v40;
        }
        else
        {
            v34 = thisa->m_steer_current_angle;
            //v4 = __libm_sse2_tan(v34);
            v4 = tan(v34);
            v33 = v4;
            width = thisa->m_steer_front_back_length / v4;
            (v32) = -(thisa->m_steer_front_back_length);
            v29 = v32;
            v30 = width;
            v31 = 0.0f;
            turning_center_radius_vec.x = v32;
        }
        turning_center_radius_vec.y = width;
        turning_center_radius_vec.z = 0.0f;
        turning_center_radius = Vec3Length(&turning_center_radius_vec.x);
        p_m_steer_front_pt_loc = &thisa->m_steer_front_pt_loc;
        v27 = thisa->m_steer_front_pt_loc.x + turning_center_radius_vec.x;
        v26 = thisa->m_steer_front_pt_loc.y + turning_center_radius_vec.y;
        v25 = thisa->m_steer_front_pt_loc.z + turning_center_radius_vec.z;
        turning_center.x = v27;
        turning_center.y = v26;
        turning_center.z = v25;
        num_accel_wheels = 0;
        total_ratio = 0.0f;
        memset(wheel_ratios, 0, sizeof(wheel_ratios));
        for (k = 0; k < 6; ++k)
        {
            if (thisa->m_wheels[k])
            {
                v19 = thisa->m_wheels[k];
                x = v19->m_b1_wheel_center_loc.x;
                y = v19->m_b1_wheel_center_loc.y;
                z = v19->m_b1_wheel_center_loc.z;
                w = v19->m_b1_wheel_center_loc.w;
                v14 = turning_center.x - x;
                v13 = turning_center.y - y;
                v12 = turning_center.z - z;
                wheel_axis.x = turning_center.x - x;
                wheel_axis.y = turning_center.y - y;
                wheel_axis.z = 0.0f;
                nwheel_axis = Vec3Length(&wheel_axis.x);
                if ((thisa->m_wheels[k]->m_wheel_flags & 8) != 0)
                {
                    if (thisa->m_steer_current_angle < 0.0)
                        v9 = -1.0f;
                    else
                        v9 = 1.0f;
                    if (nwheel_axis <= 0.001)
                    {
                        wheel_axis.x = gWheelAxisLoc.x;
                        wheel_axis.y = gWheelAxisLoc.y;
                        wheel_axis.z = gWheelAxisLoc.z;
                    }
                    else
                    {
                        wheel_axis.x = wheel_axis.x * (float)(v9 / nwheel_axis);
                        wheel_axis.y = wheel_axis.y * (float)(v9 / nwheel_axis);
                        wheel_axis.z = wheel_axis.z * (float)(v9 / nwheel_axis);
                    }
                    if (thisa->m_hand_brake > 0.1 && (thisa->m_wheels[k]->m_wheel_flags & 0x80) != 0)
                    {
                        wheel_axis.x = gWheelAxisLoc.x;
                        wheel_axis.y = gWheelAxisLoc.y;
                        wheel_axis.z = gWheelAxisLoc.z;
                    }
                    p_m_b1_wheel_axis_loc = &thisa->m_wheels[k]->m_b1_wheel_axis_loc;
                    p_m_b1_wheel_axis_loc->x = wheel_axis.x;
                    p_m_b1_wheel_axis_loc->y = wheel_axis.y;
                    p_m_b1_wheel_axis_loc->z = wheel_axis.z;
                }
                v7 = nwheel_axis / turning_center_radius;
                if ((thisa->m_wheels[k]->m_wheel_flags & 0x10) != 0)
                {
                    ++num_accel_wheels;
                    wheel_ratios[k] = v7;
                    total_ratio = total_ratio + v7;
                }
            }
        }
        v6 = (float)num_accel_wheels / total_ratio;
        for (m = 0; m < 6; ++m)
        {
            if (thisa->m_wheels[m])
            {
                thisa->m_wheels[m]->m_turning_radius_ratio_max_speed = wheel_ratios[m];
                thisa->m_wheels[m]->m_turning_radius_ratio_accel = wheel_ratios[m] * v6;
            }
        }
    }
    else
    {
        for (i = 0; i < 6; ++i)
        {
            if (thisa->m_wheels[i])
            {
                v50 = thisa->m_wheels[i];
                v49 = &v50->m_b1_wheel_axis_loc;
                v50->m_b1_wheel_axis_loc.x = gWheelAxisLoc.x;
                v49->y = gWheelAxisLoc.y;
                v49->z = gWheelAxisLoc.z;
                v48 = thisa->m_wheels[i];
                v48->m_turning_radius_ratio_max_speed = 1.0f;
                v47 = thisa->m_wheels[i];
                v47->m_turning_radius_ratio_accel = 1.0f;
            }
        }
    }
}

void __thiscall NitrousVehicle::update_braking_and_acceleration(float __formal)
{
    rigid_body_constraint_wheel *rbc_wheel; // [esp+Ch] [ebp-8h]
    int i; // [esp+10h] [ebp-4h]

    for ( i = 0; i < 6; ++i )
    {
        rbc_wheel = this->m_wheels[i];
        if ( rbc_wheel )
        {
            if ( (this->m_state_flags & 1) != 0 && (rbc_wheel->m_wheel_flags & 0x20) != 0 )
            {
                rbc_wheel->set_wheel_state_braking(this->m_power_braking_factor);
                //rigid_body_constraint_wheel::set_wheel_state_braking(rbc_wheel, this->m_power_braking_factor);
            }
            else if ( (this->m_state_flags & 2) != 0 && (rbc_wheel->m_wheel_flags & 0x40) != 0 )
            {
                rbc_wheel->set_wheel_state_braking(this->m_braking_factor);
                //rigid_body_constraint_wheel::set_wheel_state_braking(rbc_wheel, this->m_braking_factor);
            }
            else if ( (this->m_state_flags & 4) != 0 && (rbc_wheel->m_wheel_flags & 0x10) != 0 )
            {
                rbc_wheel->set_wheel_state_accelerating(this->m_desired_speed_factor / this->m_reference_wheel_radius, this->m_acceleration_factor);
                //rigid_body_constraint_wheel::set_wheel_state_accelerating(
                //    rbc_wheel,
                //    this->m_desired_speed_factor / this->m_reference_wheel_radius,
                //    this->m_acceleration_factor);
            }
            else if ( (this->m_state_flags & 8) != 0 && (rbc_wheel->m_wheel_flags & 0x10) != 0 )
            {
                rbc_wheel->set_wheel_state_accelerating(-this->m_desired_speed_factor / this->m_reference_wheel_radius, this->m_acceleration_factor);
                //rigid_body_constraint_wheel::set_wheel_state_accelerating(
                //    rbc_wheel,
                //    COERCE_FLOAT(LODWORD(this->m_desired_speed_factor) ^ _mask__NegFloat_) / this->m_reference_wheel_radius,
                //    this->m_acceleration_factor);
            }
            else if ( (this->m_state_flags & 0x10) != 0 )
            {
                rbc_wheel->set_wheel_state_braking(this->m_coasting_factor);
                //rigid_body_constraint_wheel::set_wheel_state_braking(rbc_wheel, this->m_coasting_factor);
            }
            else
            {
                rbc_wheel->set_wheel_state_braking(0.0f);
                //rigid_body_constraint_wheel::set_wheel_state_braking(rbc_wheel, 0.0);
            }
        }
    }
}

phys_vec3 *__cdecl phys_cross(phys_vec3 *result, const phys_vec3 *a, const phys_vec3 *b)
{
    float v4; // [esp+4h] [ebp-8h]
    float v5; // [esp+8h] [ebp-4h]

    v4 = (float)(a->z * b->x) - (float)(a->x * b->z);
    v5 = (float)(a->x * b->y) - (float)(a->y * b->x);
    result->x = (float)(a->y * b->z) - (float)(a->z * b->y);
    result->y = v4;
    result->z = v5;
    return result;
}

void __thiscall NitrousVehicle::set_boosting(bool boosting)
{
    if ( boosting != ((this->m_flags & 0x800) != 0) )
    {
        if ( boosting )
            this->m_flags |= 0x800u;
        else
            this->m_flags &= ~0x800u;
        if ( boosting )
            this->m_boost_start_time = this->mVehicleController.m_cmd.serverTime;
    }
}

bool __thiscall NitrousVehicle::is_boosting()
{
    return (this->m_flags & 0x800) != 0;
}

double __thiscall NitrousVehicle::get_maximum_acceleration_factor()
{
    return 2.0;
}

double __thiscall NitrousVehicle::get_maximum_acceleration()
{
    float maximum_acceleration; // [esp+4h] [ebp-4h]

    maximum_acceleration = this->m_parameter->m_accel_max;
    if ( NitrousVehicle::is_boosting() )
        return (float)(maximum_acceleration * this->m_vehicle_info->boostAccelMultiplier);
    return maximum_acceleration;
}

double __thiscall NitrousVehicle::get_max_speed(bool include_perks)
{
    float max_speed; // [esp+4h] [ebp-4h]

    max_speed = this->m_parameter->m_speed_max;
    if ( NitrousVehicle::is_boosting() )
        return (float)(max_speed * this->m_vehicle_info->boostSpeedIncrease);
    return max_speed;
}

void __thiscall NitrousVehicle::update_control_from_network(LerpEntityStateVehicle *vehicleState)
{
    float v2; // [esp+4h] [ebp-10h]
    float steerYaw; // [esp+10h] [ebp-4h]

    if ( this->m_phys_user_data )
    {
        NitrousVehicle::set_throttle((float)vehicleState->throttle * 0.000030517578);
        if ( (float)(vehicleState->steerYaw - 1.0) < 0.0 )
            steerYaw = vehicleState->steerYaw;
        else
            steerYaw = 1.0f;
        if ( (float)(-1.0 - vehicleState->steerYaw) < 0.0 )
            v2 = steerYaw;
        else
            v2 = -1.0f;
        this->m_steer_factor = v2;
        this->mVehicleController.m_cmd_valid = 0;
    }
}

// local variable allocation has failed, the output may be wrong!
void NitrousVehicle::update_from_network(
                trajectory_t pos,
                trajectory_t apos,
                LerpEntityStateVehicle *vehicleState,
                float timeDifference)
{
    const phys_vec3 *v6; // eax
    double v7; // st7
    float currentAnglesError[3]; // [esp+10h] [ebp-294h] BYREF
    float predictedAngles[3]; // [esp+1Ch] [ebp-288h] BYREF
    float predictedPhysAxis[3][3]; // [esp+28h] [ebp-27Ch] BYREF
    float localPhysicsAngles[3]; // [esp+4Ch] [ebp-258h] BYREF
    phys_mat44 rv; // [esp+58h] [ebp-24Ch] BYREF
    float currentOriginError[3]; // [esp+98h] [ebp-20Ch] BYREF
    float predictedPos[3]; // [esp+A4h] [ebp-200h] BYREF
    float z; // [esp+B0h] [ebp-1F4h]
    rigid_body *v16; // [esp+B4h] [ebp-1F0h]
    rigid_body *v17; // [esp+B8h] [ebp-1ECh]
    rigid_body *v18; // [esp+BCh] [ebp-1E8h]
    float y; // [esp+C0h] [ebp-1E4h]
    rigid_body *v20; // [esp+C4h] [ebp-1E0h]
    rigid_body *v21; // [esp+C8h] [ebp-1DCh]
    rigid_body *v22; // [esp+CCh] [ebp-1D8h]
    float x; // [esp+D0h] [ebp-1D4h]
    rigid_body *v24; // [esp+D4h] [ebp-1D0h]
    rigid_body *v25; // [esp+D8h] [ebp-1CCh]
    rigid_body *v26; // [esp+DCh] [ebp-1C8h]
    rigid_body *body; // [esp+E0h] [ebp-1C4h]
    bool v28; // [esp+E7h] [ebp-1BDh]
    //int v29; // [esp+E8h] [ebp-1BCh] BYREF
    phys_vec3 v30; // [esp+ECh] [ebp-1B8h] BYREF
    float acceptablePosError; // [esp+114h] [ebp-190h]
    phys_vec3 networkPredictedRBPosition; // [esp+118h] [ebp-18Ch] BYREF
    float v33; // [esp+128h] [ebp-17Ch]
    float v34; // [esp+12Ch] [ebp-178h]
    float v35; // [esp+130h] [ebp-174h]
    float v36; // [esp+13Ch] [ebp-168h]
    float v37; // [esp+140h] [ebp-164h]
    float v38; // [esp+144h] [ebp-160h]
    float v39; // [esp+148h] [ebp-15Ch]
    float v40; // [esp+14Ch] [ebp-158h]
    float v41; // [esp+150h] [ebp-154h]
    float v42; // [esp+158h] [ebp-14Ch]
    float v43; // [esp+15Ch] [ebp-148h]
    float v44; // [esp+160h] [ebp-144h]
    int v45; // [esp+164h] [ebp-140h]
    phys_vec3 networkVel; // [esp+168h] [ebp-13Ch] BYREF
    float localPhysAxis[3][3]; // [esp+180h] [ebp-124h] BYREF
    phys_vec3 *i; // [esp+1A4h] [ebp-100h]
    phys_vec3 phys_axis[3]; // [esp+1A8h] [ebp-FCh] BYREF
    int v50; // [esp+1E4h] [ebp-C0h]
    phys_vec3 position; // [esp+1E8h] [ebp-BCh] BYREF
    float networkAngles[3]; // [esp+204h] [ebp-A0h]
    float networkRBPos[3]; // [esp+210h] [ebp-94h] BYREF
    float translation[3]; // [esp+21Ch] [ebp-88h] BYREF
    float networkAxisTranspose[3][3]; // [esp+228h] [ebp-7Ch] BYREF
    float localTrans[3]; // [esp+24Ch] [ebp-58h] BYREF
    float networkAxis[3][3]; // [esp+258h] [ebp-4Ch] BYREF
    float localPos[3]; // [esp+27Ch] [ebp-28h] BYREF
    phys_vec3 *aVelocity; // [esp+288h] [ebp-1Ch]
    phys_vec3 *velocity; // [esp+28Ch] [ebp-18h]
    phys_mat44 *mat; // [esp+290h] [ebp-14h]
    NitrousVehicle *thisa; // [esp+294h] [ebp-10h]
    //_UNKNOWN *v63[3]; // [esp+298h] [ebp-Ch] BYREF
    //void *vars0; // [esp+2A4h] [ebp+0h]
    //
    //v63[0] = a2;
    //v63[1] = vars0;
    thisa = this;
    if (this->m_phys_user_data)
    {
        NitrousVehicle::update_control_from_network(vehicleState);
        mat = &thisa->m_phys_user_data->body->m_mat;
        velocity = &thisa->m_phys_user_data->body->m_t_vel;
        aVelocity = &thisa->m_phys_user_data->body->m_a_vel;
        Phys_NitrousVecToVec3(&mat->w, localPos);
        AnglesToAxis(apos.trBase, networkAxis);
        Phys_NitrousVecToVec3(&thisa->m_phys_user_data->cg2rb.w, localTrans);
        MatrixTranspose(networkAxis, networkAxisTranspose);
        MatrixVecMultiply(networkAxisTranspose, localTrans, translation);
        networkRBPos[0] = pos.trBase[0] - translation[0];
        networkRBPos[1] = pos.trBase[1] - translation[1];
        networkRBPos[2] = pos.trBase[2] - translation[2];
        networkAngles[0] = apos.trBase[0];
        networkAngles[1] = apos.trBase[1];
        networkAngles[2] = apos.trBase[2];
        Phys_Vec3ToNitrousVec(networkRBPos, &position);
        v50 = 3;
        for (i = phys_axis; --v50 >= 0; ++i)
            ;
        Phys_Vec3ToNitrousVec(networkAxis[0], phys_axis);
        Phys_Vec3ToNitrousVec(networkAxis[1], &phys_axis[1]);
        Phys_Vec3ToNitrousVec(networkAxis[2], &phys_axis[2]);
        Phys_NitrousMat44ToVec33(mat, localPhysAxis);
        Phys_Vec3ToNitrousVec(pos.trDelta, &networkVel);
        v45 = fabs(timeDifference);
        if (timeCap > (fabs(timeDifference)))
        {
            v44 = timeDifference * networkVel.x;
            v43 = timeDifference * networkVel.y;
            v42 = timeDifference * networkVel.z;
            v39 = timeDifference * networkVel.x;
            v40 = timeDifference * networkVel.y;
            v41 = timeDifference * networkVel.z;
            v38 = position.x + (float)(timeDifference * networkVel.x);
            v37 = position.y + (float)(timeDifference * networkVel.y);
            v36 = position.z + (float)(timeDifference * networkVel.z);
            v33 = v38;
            v34 = v37;
            v35 = v36;
            networkPredictedRBPosition.x = v38;
            networkPredictedRBPosition.y = v37;
            networkPredictedRBPosition.z = v36;
            if (!CompareVec3ToPhysVec(pos.trDelta, velocity, acceptableVelError))
                Phys_Vec3ToNitrousVec(pos.trDelta, velocity);
            if (!CompareVec3ToPhysVec(apos.trDelta, aVelocity, acceptableAVelError))
                Phys_Vec3ToNitrousVec(apos.trDelta, aVelocity);
            acceptablePosError = acceptablePosErrorBase;
            phys_vec3 tmp;
            //v6 = operator-((phys_vec3 *)&v30.w, &networkPredictedRBPosition, &mat->w);
            tmp = networkPredictedRBPosition - mat->w;
            //phys_vec3::operator=((phys_vec3 *)&v29, v6);
            //v7 = AbsSquared((const phys_vec3 *)&v29);
            v7 = AbsSquared(&tmp);
            v28 = acceptablePosError * acceptablePosError > v7;
            if (acceptablePosError * acceptablePosError <= v7)
            {
                Phys_Vec3ToNitrousVec(pos.trDelta, velocity);
                Phys_Vec3ToNitrousVec(apos.trDelta, aVelocity);
                //phys_vec3::operator=(&mat->x, phys_axis);
                mat->x = phys_axis[0];
                mat->y = phys_axis[1];
                mat->z = phys_axis[2];
                mat->w = networkPredictedRBPosition;
                //phys_vec3::operator=(&mat->y, &phys_axis[1]);
                //phys_vec3::operator=(&mat->z, &phys_axis[2]);
                //phys_vec3::operator=(&mat->w, &networkPredictedRBPosition);
                body = thisa->m_phys_user_data->body;
                *(_QWORD *)&body->m_last_position.x = *(_QWORD *)&position.x;
                body->m_last_position.z = position.z;
                v26 = thisa->m_phys_user_data->body;
                v25 = thisa->m_phys_user_data->body;
                if (v26->m_last_position.x != v25->m_last_position.x)
                    goto LABEL_19;
                v24 = thisa->m_phys_user_data->body;
                x = v24->m_last_position.x;
                if ((fabs(x)) > 100000.0)
                    goto LABEL_19;
                v22 = thisa->m_phys_user_data->body;
                v21 = thisa->m_phys_user_data->body;
                if (v22->m_last_position.y != v21->m_last_position.y
                    || (v20 = thisa->m_phys_user_data->body,
                        y = v20->m_last_position.y,
                        (fabs(y)) > 100000.0)
                    || (v18 = thisa->m_phys_user_data->body,
                        v17 = thisa->m_phys_user_data->body,
                        v18->m_last_position.z != v17->m_last_position.z)
                    || (v16 = thisa->m_phys_user_data->body,
                        z = v16->m_last_position.z,
                        (fabs(z)) > 100000.0))
                {
                LABEL_19:
                    phys_exec_debug_callback(thisa->m_phys_user_data->body);
                }
                Phys_NitrousVecToVec3(&networkPredictedRBPosition, predictedPos);
                Vec3Sub(localPos, predictedPos, currentOriginError);
                Vec3Add(currentOriginError, thisa->m_networkErrorOrigin, thisa->m_networkErrorOrigin);
                //XAnimClientNotifyList::GetNotifyList((XAnimClientNotifyList *)&rv);
                make_rotate(&rv, aVelocity, timeDifference, 1000.0);
                phys_multiply_mat(mat, &rv, mat);
                AxisToAngles(localPhysAxis, localPhysicsAngles);
                Phys_NitrousMat44ToVec33(mat, predictedPhysAxis);
                AxisToAngles(predictedPhysAxis, predictedAngles);
                AnglesSubtract(localPhysicsAngles, predictedAngles, currentAnglesError);
                Vec3Add(currentAnglesError, thisa->m_networkErrorAngles, thisa->m_networkErrorAngles);
                if (phys_drawNitrousVehicle->current.enabled)
                {
                    G_DebugStar(networkRBPos, colorGreen, duration);
                    G_DebugStar(predictedPos, colorYellow, duration);
                    G_DebugStar(localPos, colorRed, duration);
                    CG_DebugLine(networkRBPos, predictedPos, colorWhite, 1, duration);
                    CG_DebugLine(predictedPos, localPos, colorWhite, 1, duration);
                }
            }
        }
    }
}

double __cdecl AbsSquared(const phys_vec3 *a)
{
    return a->x * a->x + a->y * a->y + a->z * a->z;
}

// local variable allocation has failed, the output may be wrong!
void    phys_multiply_mat(phys_mat44 *dest, const phys_mat44 *left, const phys_mat44 *right)
{
    phys_mat44 v4; // [esp-Ch] [ebp-ACh] BYREF
    phys_vec3 *temp_60; // [esp+3Ch] [ebp-64h]
    const phys_vec3 *v6; // [esp+40h] [ebp-60h]
    phys_vec3 v7; // [esp+44h] [ebp-5Ch] BYREF
    phys_vec3 *p_y; // [esp+5Ch] [ebp-44h]
    const phys_vec3 *v9; // [esp+60h] [ebp-40h]
    phys_vec3 v10; // [esp+64h] [ebp-3Ch] BYREF
    const phys_vec3 *v11; // [esp+80h] [ebp-20h]
    phys_vec3 v12; // [esp+84h] [ebp-1Ch] BYREF
    //_UNKNOWN *v13[2]; // [esp+94h] [ebp-Ch] BYREF
    //const phys_mat44 *righta; // [esp+A0h] [ebp+0h]
    //
    //v13[0] = a1;
    //v13[1] = righta;

    assert_mat44_initialized(*left);
    //assert_mat44_initialized(*right);

    if (dest == left)
    {
        memcpy(&v4, left, sizeof(v4));
        phys_multiply_mat(dest, &v4, right);
    }
    else
    {
        v11 = phys_multiply(&v12, left, &right->x);
        dest->x.x = v11->x;
        dest->x.y = v11->y;
        dest->x.z = v11->z;
        v9 = phys_multiply(&v10, left, &right->y);
        p_y = &dest->y;
        dest->y.x = v9->x;
        p_y->y = v9->y;
        p_y->z = v9->z;
        v6 = phys_multiply(&v7, left, &right->z);
        temp_60 = &dest->z;
        dest->z.x = v6->x;
        temp_60->y = v6->y;
        temp_60->z = v6->z;
    }
}

// defined in phys_render
//void    make_rotate(
//                phys_mat44 *mat,
//                const phys_vec3 *v,
//                float theta_factor,
//                float max_rotation_radians)
//{
//    float v5; // xmm0_4
//    float ca; // xmm0_4
//    float x; // [esp+4h] [ebp-50h]
//    phys_vec3 v8; // [esp+18h] [ebp-3Ch] BYREF
//    float v9; // [esp+34h] [ebp-20h]
//    float v10; // [esp+38h] [ebp-1Ch]
//    float v11; // [esp+3Ch] [ebp-18h]
//    float v12; // [esp+40h] [ebp-14h]
//    float v13; // [esp+44h] [ebp-10h]
//    //int v14; // [esp+48h] [ebp-Ch]
//    //void *v15; // [esp+4Ch] [ebp-8h]
//    //void *retaddr; // [esp+54h] [ebp+0h]
//    //
//    //v14 = a1;
//    //v15 = retaddr;
//    v13 = Abs(&v->x);
//    if (v13 >= 0.0000099999997)
//    {
//        v12 = 1.0 / v13;
//        v11 = (float)(1.0 / v13) * v->x;
//        v10 = (float)(1.0 / v13) * v->y;
//        v9 = (float)(1.0 / v13) * v->z;
//        v8.x = v11;
//        v8.y = v10;
//        v8.z = v9;
//        v13 = v13 * theta_factor;
//        if (v13 > max_rotation_radians)
//            v13 = max_rotation_radians;
//        //v5 = __libm_sse2_sin(v13);
//        v5 = sin(v13);
//        x = v5;
//        //ca = __libm_sse2_cos(v13);
//        ca = cos(v13);
//        make_rotate(mat, &v8, ca, x);
//    }
//    else
//    {
//        //phys_mat44::operator=(mat, &PHYS_IDENTITY_MATRIX_34);
//        *mat = PHYS_IDENTITY_MATRIX;
//    }
//}

// defined in phys_render
//void __cdecl make_rotate(phys_mat44 *m, const phys_vec3 *u, float ca, float sa)
//{
//    float yy; // [esp+1Ch] [ebp-28h]
//    float xsa; // [esp+20h] [ebp-24h]
//    float xy; // [esp+24h] [ebp-20h]
//    float zsa; // [esp+28h] [ebp-1Ch]
//    float zz; // [esp+2Ch] [ebp-18h]
//    float ysa; // [esp+34h] [ebp-10h]
//    float xz; // [esp+38h] [ebp-Ch]
//    float yz; // [esp+3Ch] [ebp-8h]
//
//    xy = (float)(u->x * u->y) * (float)(1.0 - ca);
//    xz = (float)(u->x * u->z) * (float)(1.0 - ca);
//    yy = (float)(u->y * u->y) * (float)(1.0 - ca);
//    yz = (float)(u->y * u->z) * (float)(1.0 - ca);
//    zz = (float)(u->z * u->z) * (float)(1.0 - ca);
//    xsa = u->x * sa;
//    ysa = u->y * sa;
//    zsa = u->z * sa;
//    m->x.x = (float)((float)(u->x * u->x) * (float)(1.0 - ca)) + ca;
//    m->y.x = xy - zsa;
//    m->z.x = xz + ysa;
//    m->w.x = 0.0f;
//    //*phys_vec3::operator[]<int>(&m->x, 1u) = xy + zsa;
//    m->x[1] = xy + zsa;
//    //*phys_vec3::operator[]<int>(&m->y, 1u) = yy + ca;
//    m->y[1] = yy + ca;
//    //*phys_vec3::operator[]<int>(&m->z, 1u) = yz - xsa;
//    m->z[1] = yz - xsa;
//    //*phys_vec3::operator[]<int>(&m->w, 1u) = 0.0f;
//    m->w[1] = 0.0f;
//    //*phys_vec3::operator[]<int>(&m->x, 2u) = xz - ysa;
//    m->x[2] = xz - ysa;
//    //*phys_vec3::operator[]<int>(&m->y, 2u) = yz + xsa;
//    m->y[2] = yz + xsa;
//    //*phys_vec3::operator[]<int>(&m->z, 2u) = zz + ca;
//    m->z[2] = zz + ca;
//    //*phys_vec3::operator[]<int>(&m->w, 2u) = 0.0f;
//    m->w[2] = 0.0f;
//    //phys_mat44::fix_w_column(m);
//    m->fix_w_column();
//}

void __cdecl Phys_NitrousMat44ToVec33(const phys_mat44 *inMat, float (*outAxis)[3])
{
    assert_mat44_initialized(*inMat);

    (*outAxis)[0] = inMat->x.x;
    (*outAxis)[1] = inMat->x.y;
    (*outAxis)[2] = inMat->x.z;
    (*outAxis)[3] = inMat->y.x;
    (*outAxis)[4] = inMat->y.y;
    (*outAxis)[5] = inMat->y.z;
    (*outAxis)[6] = inMat->z.x;
    (*outAxis)[7] = inMat->z.y;
    (*outAxis)[8] = inMat->z.z;
}

bool __cdecl CompareVec3ToPhysVec(const float *a, const phys_vec3 *b, float acceptableError)
{
    float diff_4; // [esp+10h] [ebp-8h]
    float diff_8; // [esp+14h] [ebp-4h]

    diff_4 = b->y - a[1];
    diff_8 = b->z - a[2];
    return (float)(acceptableError * acceptableError) > (float)((float)((float)((float)(b->x - *a) * (float)(b->x - *a))
                                                                                                                                        + (float)(diff_4 * diff_4))
                                                                                                                        + (float)(diff_8 * diff_8));
}

void __thiscall NitrousVehicle::GetEntityPose(
                float *origin,
                float *angles,
                int curTime,
                bool isDriver)
{
    float v5; // [esp+0h] [ebp-90h]
    float v6; // [esp+4h] [ebp-8Ch]
    float v8; // [esp+3Ch] [ebp-54h]
    float v9; // [esp+50h] [ebp-40h]
    float dt; // [esp+54h] [ebp-3Ch]
    float reduction[3]; // [esp+58h] [ebp-38h] BYREF
    float scale; // [esp+64h] [ebp-2Ch]
    float rbPosition[3]; // [esp+68h] [ebp-28h] BYREF
    float rbAngles[3]; // [esp+74h] [ebp-1Ch] BYREF
    float rbRotation[4]; // [esp+80h] [ebp-10h] BYREF

    Sys_EnterCriticalSection(CRITSECT_PHYSICS_UPDATE);
    if ( this->m_phys_user_data )
    {
        if ( isDriver )
        {
            Phys_ObjGetInterpolatedState((int)this->m_phys_user_data, rbPosition, rbRotation);
            UnitQuatToAngles(rbRotation, rbAngles);
            *origin = rbPosition[0];
            origin[1] = rbPosition[1];
            origin[2] = rbPosition[2];
            *angles = rbAngles[0];
            angles[1] = rbAngles[1];
            angles[2] = rbAngles[2];
        }
        else
        {
            if ( curTime > this->m_lastErrorReductionTime )
            {
                dt = (float)(curTime - this->m_lastErrorReductionTime) * 0.001;
                if ( (float)((float)(errorReductionRate * dt) - 1.0) < 0.0 )
                    v9 = errorReductionRate * dt;
                else
                    v9 = 1.0f;
                if ( (float)(0.0 - (float)(errorReductionRate * dt)) < 0.0 )
                    v6 = v9;
                else
                    v6 = 0.0f;
                scale = v6;
                reduction[0] = v6 * this->m_networkErrorOrigin[0];
                reduction[1] = v6 * this->m_networkErrorOrigin[1];
                reduction[2] = v6 * this->m_networkErrorOrigin[2];
                this->m_networkErrorOrigin[0] = this->m_networkErrorOrigin[0] - reduction[0];
                this->m_networkErrorOrigin[1] = this->m_networkErrorOrigin[1] - reduction[1];
                this->m_networkErrorOrigin[2] = this->m_networkErrorOrigin[2] - reduction[2];
                if ( (float)((float)(errorAReductionRate * dt) - 1.0) < 0.0 )
                    v8 = errorAReductionRate * dt;
                else
                    v8 = 1.0f;
                if ( (float)(0.0 - (float)(errorAReductionRate * dt)) < 0.0 )
                    v5 = v8;
                else
                    v5 = 0.0f;
                scale = v5;
                reduction[0] = v5 * this->m_networkErrorAngles[0];
                reduction[1] = v5 * this->m_networkErrorAngles[1];
                reduction[2] = v5 * this->m_networkErrorAngles[2];
                AnglesSubtract(this->m_networkErrorAngles, reduction, this->m_networkErrorAngles);
            }
            this->m_lastErrorReductionTime = curTime;
            Phys_ObjGetInterpolatedState((int)this->m_phys_user_data, rbPosition, rbRotation);
            UnitQuatToAngles(rbRotation, rbAngles);
            rbPosition[0] = rbPosition[0] + this->m_networkErrorOrigin[0];
            rbPosition[1] = rbPosition[1] + this->m_networkErrorOrigin[1];
            rbPosition[2] = rbPosition[2] + this->m_networkErrorOrigin[2];
            rbAngles[0] = rbAngles[0] + this->m_networkErrorAngles[0];
            rbAngles[1] = rbAngles[1] + this->m_networkErrorAngles[1];
            rbAngles[2] = rbAngles[2] + this->m_networkErrorAngles[2];
            *origin = rbPosition[0];
            origin[1] = rbPosition[1];
            origin[2] = rbPosition[2];
            *angles = rbAngles[0];
            angles[1] = rbAngles[1];
            angles[2] = rbAngles[2];
        }
        Sys_LeaveCriticalSection(CRITSECT_PHYSICS_UPDATE);

        nanassertvec3(origin);
        nanassertvec3(angles);

    }
    else
    {
        Sys_LeaveCriticalSection(CRITSECT_PHYSICS_UPDATE);
    }
}

int duration_0 = 100;
void __cdecl G_SetVehDriverInputs(int localClientNum, int vehEntNum, usercmd_s *cmd)
{
    float *inVector; // [esp+8h] [ebp-D4h]
    float axis[9]; // [esp+10h] [ebp-CCh] BYREF
    float *m_networkErrorAngles; // [esp+34h] [ebp-A8h]
    float *v6; // [esp+38h] [ebp-A4h]
    float *m_networkErrorOrigin; // [esp+60h] [ebp-7Ch]
    float *v8; // [esp+64h] [ebp-78h]
    float *origin; // [esp+68h] [ebp-74h]
    NitrousVehicle *nitrousVehicle; // [esp+6Ch] [ebp-70h]
    NitrousVehicleController *p_mVehicleController; // [esp+70h] [ebp-6Ch]
    phys_vec3 *velocity; // [esp+74h] [ebp-68h]
    phys_vec3 *aVelocity; // [esp+78h] [ebp-64h]
    float localPos[3]; // [esp+7Ch] [ebp-60h] BYREF
    float currentOriginError[3]; // [esp+88h] [ebp-54h]
    phys_mat44 *mat; // [esp+94h] [ebp-48h]
    float localPhysAxis[3][3]; // [esp+98h] [ebp-44h] BYREF
    float localAngles[3]; // [esp+BCh] [ebp-20h] BYREF
    float currentAnglesError[3]; // [esp+C8h] [ebp-14h] BYREF
    gentity_s *ent; // [esp+D4h] [ebp-8h]
    gentity_s *self; // [esp+D8h] [ebp-4h]

    self = &g_entities[vehEntNum];
    if ( self->scr_vehicle )
    {
        if ( self->scr_vehicle->nitrousVehicle )
        {
            nitrousVehicle = self->scr_vehicle->nitrousVehicle;
            p_mVehicleController = &nitrousVehicle->mVehicleController;
            memcpy(&nitrousVehicle->mVehicleController.m_cmd, cmd, sizeof(nitrousVehicle->mVehicleController.m_cmd));
            nitrousVehicle->mVehicleController.m_cmd_local_client_num = localClientNum;
            p_mVehicleController->m_cmd_valid = 1;
            //if ( EntHandle::isDefined(&self->r.ownerNum) )
            if ( self->r.ownerNum.isDefined() )
            {
                //ent = EntHandle::ent(&self->r.ownerNum);
                ent = self->r.ownerNum.ent();
                if ( (ent->s.lerp.eFlags & 0x4000) == 0
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\vehicle\\nitrous_vehicle.cpp",
                                2385,
                                0,
                                "%s",
                                "ent->s.lerp.eFlags & EF_VEHICLE_ACTIVE") )
                {
                    __debugbreak();
                }
                if ( ent->s.number >= 32
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\vehicle\\nitrous_vehicle.cpp",
                                2386,
                                0,
                                "%s",
                                "ent->s.number < MAX_CLIENTS") )
                {
                    __debugbreak();
                }
                if ( svs.clients[ent->s.clientNum].header.vehicle.inVehicle )
                {
                    Sys_EnterCriticalSection(CRITSECT_PHYSICS);
                    mat = &self->scr_vehicle->nitrousVehicle->m_phys_user_data->body->m_mat;
                    velocity = &self->scr_vehicle->nitrousVehicle->m_phys_user_data->body->m_t_vel;
                    aVelocity = &self->scr_vehicle->nitrousVehicle->m_phys_user_data->body->m_a_vel;
                    Phys_NitrousVecToVec3(&mat->w, localPos);
                    origin = svs.clients[ent->s.clientNum].header.vehicle.origin;
                    currentOriginError[0] = localPos[0] - *origin;
                    currentOriginError[1] = localPos[1] - origin[1];
                    currentOriginError[2] = localPos[2] - origin[2];
                    m_networkErrorOrigin = self->scr_vehicle->nitrousVehicle->m_networkErrorOrigin;
                    v8 = self->scr_vehicle->nitrousVehicle->m_networkErrorOrigin;
                    *m_networkErrorOrigin = currentOriginError[0] + *v8;
                    m_networkErrorOrigin[1] = currentOriginError[1] + v8[1];
                    m_networkErrorOrigin[2] = currentOriginError[2] + v8[2];
                    Phys_NitrousMat44ToVec33(mat, localPhysAxis);
                    AxisToAngles(localPhysAxis, localAngles);
                    AnglesSubtract(localAngles, svs.clients[ent->s.clientNum].header.vehicle.angles, currentAnglesError);
                    m_networkErrorAngles = self->scr_vehicle->nitrousVehicle->m_networkErrorAngles;
                    v6 = self->scr_vehicle->nitrousVehicle->m_networkErrorAngles;
                    *m_networkErrorAngles = currentAnglesError[0] + *v6;
                    m_networkErrorAngles[1] = currentAnglesError[1] + v6[1];
                    m_networkErrorAngles[2] = currentAnglesError[2] + v6[2];
                    Phys_Vec3ToNitrousVec(svs.clients[ent->s.clientNum].header.vehicle.tVel, velocity);
                    Phys_Vec3ToNitrousVec(svs.clients[ent->s.clientNum].header.vehicle.aVel, aVelocity);
                    inVector = svs.clients[ent->s.clientNum].header.vehicle.origin;
                    AnglesToAxis(svs.clients[ent->s.clientNum].header.vehicle.angles, (float (*)[3])axis);
                    Phys_AxisToNitrousMat((float (*)[3])axis, mat);
                    Phys_Vec3ToNitrousVec(inVector, &mat->w);
                    if ( phys_drawNitrousVehicle->current.enabled )
                    {
                        G_DebugStar(svs.clients[ent->s.clientNum].header.vehicle.origin, colorGreen, duration_0);
                        G_DebugStar(localPos, colorRed, duration_0);
                    }
                    Sys_LeaveCriticalSection(CRITSECT_PHYSICS);
                }
            }
        }
    }
}

void __cdecl G_ClearVehicleInputs()
{
    phys_free_list<NitrousVehicle>::T_internal_base *i; // [esp+24h] [ebp-8h]

    if ( g_rb_vehicle_list.m_list_count > 0 )
    {
        PROF_SCOPED("G_ClearVehicleInputs");

        Sys_EnterCriticalSection(CRITSECT_PHYSICS_UPDATE);
        Sys_EnterCriticalSection(CRITSECT_PHYSICS);
        for ( i = g_rb_vehicle_list.m_dummy_head.m_next_T_internal;
                    &g_rb_vehicle_list != (phys_free_list<NitrousVehicle> *)i;
                    i = i->m_next_T_internal )
        {
            if ( i[70].m_next_T_internal
                && i[70].m_next_T_internal[43].m_prev_T_internal
                && VEH_GetSeatOccupantEntNum((gentity_s *)i[70].m_next_T_internal, 0) == 1023 )
            {
                memset(&i[100], 0, 0x34u);
            }
        }
        Sys_LeaveCriticalSection(CRITSECT_PHYSICS);
        Sys_LeaveCriticalSection(CRITSECT_PHYSICS_UPDATE);
    }
}

void __cdecl CG_SetVehDriverInputs(int localClientNum, int vehEntNum, usercmd_s *cmd)
{
    NitrousVehicle *nitrousVeh; // [esp+8h] [ebp-14h]
    centity_s *self; // [esp+18h] [ebp-4h]

    self = CG_GetEntity(localClientNum, vehEntNum);
    if ( self->nitrousVeh )
    {
        nitrousVeh = self->nitrousVeh;
        memcpy(&nitrousVeh->mVehicleController.m_cmd, cmd, sizeof(nitrousVeh->mVehicleController.m_cmd));
        nitrousVeh->mVehicleController.m_cmd_local_client_num = localClientNum;
        nitrousVeh->mVehicleController.m_cmd_valid = 1;
    }
}

void __cdecl Vehicle_Launch(int localClientNum, centity_s *cent, float *hitp, const float *force, bool relative)
{
    float value; // [esp+4h] [ebp-14h]
    float scaledForce[3]; // [esp+Ch] [ebp-Ch] BYREF
    int savedregs; // [esp+18h] [ebp+0h] BYREF

    if ( !localClientNum && cent->nitrousVeh )
    {
        if ( cent->nitrousVeh->m_phys_user_data )
        {
            value = dynEnt_bulletForce->current.value;
            scaledForce[0] = value * *force;
            scaledForce[1] = value * force[1];
            scaledForce[2] = value * force[2];
            Phys_ObjAddForce((int)cent->nitrousVeh->m_phys_user_data, hitp, scaledForce, relative);
        }
    }
}

//NitrousVehicle *__thiscall phys_free_list<NitrousVehicle>::add(
//                phys_free_list<NitrousVehicle> *this,
//                int no_error,
//                const char *error_msg)
//{
//    phys_free_list<NitrousVehicle>::T_internal *ptr; // [esp+20h] [ebp-4h]
//
//    ptr = (phys_free_list<NitrousVehicle>::T_internal *)PMM_ALLOC(0x460u, 0x10u);
//    if ( ptr )
//    {
//        NitrousVehicle::NitrousVehicle(&ptr->m_data);
//        ptr->m_prev_T_internal = &this->m_dummy_head;
//        ptr->m_next_T_internal = this->m_dummy_head.m_next_T_internal;
//        this->m_dummy_head.m_next_T_internal->m_prev_T_internal = ptr;
//        this->m_dummy_head.m_next_T_internal = ptr;
//        ++this->m_list_count;
//        phys_free_list<NitrousVehicle>::debug_add(this, ptr);
//        return &ptr->m_data;
//    }
//    else
//    {
//        if ( !no_error )
//        {
//            if ( _tlAssert("c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h", 470, "no_error", error_msg) )
//                __debugbreak();
//        }
//        return 0;
//    }
//}
//
//void __thiscall phys_free_list<NitrousVehicle>::remove(phys_free_list<NitrousVehicle> *this, NitrousVehicle *data_)
//{
//    if ( data_ )
//    {
//        PMM_VALIDATE((char *)&data_[-1].m_steer_front_back_length, 0x460u, 0x10u);
//        phys_free_list<NitrousVehicle>::remove(
//            this,
//            (phys_free_list<NitrousVehicle>::T_internal *)&data_[-1].m_steer_front_back_length);
//    }
//}
//
//void __thiscall phys_free_list<NitrousVehicle>::debug_add(
//                phys_free_list<NitrousVehicle> *this,
//                phys_free_list<NitrousVehicle>::T_internal *T_i)
//{
//    int m_list_count; // [esp+0h] [ebp-10h]
//
//    if ( this->m_list_count_high_water <= this->m_list_count )
//        m_list_count = this->m_list_count;
//    else
//        m_list_count = this->m_list_count_high_water;
//    this->m_list_count_high_water = m_list_count;
//    if ( this->m_ptr_list_count >= 256 )
//    {
//        T_i->m_ptr_list_index = -1;
//    }
//    else
//    {
//        T_i->m_ptr_list_index = this->m_ptr_list_count;
//        this->m_ptr_list[this->m_ptr_list_count++] = &T_i->m_data;
//    }
//}
//
//void __thiscall phys_free_list<NitrousVehicle>::remove(
//                phys_free_list<NitrousVehicle> *this,
//                phys_free_list<NitrousVehicle>::T_internal *data)
//{
//    phys_free_list<NitrousVehicle>::T_internal_base *next; // [esp+14h] [ebp-8h]
//    phys_free_list<NitrousVehicle>::T_internal_base *prev; // [esp+18h] [ebp-4h]
//
//    if ( !data
//        && _tlAssert("c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h", 477, "data", "") )
//    {
//        __debugbreak();
//    }
//    --this->m_list_count;
//    phys_free_list<NitrousVehicle>::debug_remove(this, data);
//    next = data->m_next_T_internal;
//    prev = data->m_prev_T_internal;
//    prev->m_next_T_internal = next;
//    next->m_prev_T_internal = prev;
//    PMM_FREE((unsigned __int8 *)data, 0x460u, 0x10u);
//}
//
//void __thiscall phys_free_list<NitrousVehicle>::debug_remove(
//                phys_free_list<NitrousVehicle> *this,
//                phys_free_list<NitrousVehicle>::T_internal *T_i)
//{
//    if ( T_i->m_ptr_list_index != -1 )
//    {
//        if ( T_i->m_ptr_list_index >= 0x100u
//            && _tlAssert(
//                     "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h",
//                     421,
//                     "T_i->m_ptr_list_index >= 0 && T_i->m_ptr_list_index < PTR_LIST_SIZE",
//                     "") )
//        {
//            __debugbreak();
//        }
//        if ( this->m_ptr_list[T_i->m_ptr_list_index] != &T_i->m_data )
//        {
//            if ( _tlAssert(
//                         "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h",
//                         422,
//                         "m_ptr_list[T_i->m_ptr_list_index] == &T_i->m_data",
//                         "") )
//            {
//                __debugbreak();
//            }
//        }
//        this->m_ptr_list[--this->m_ptr_list_count][1].m_wheel_state[0].m_state = T_i->m_ptr_list_index;
//        this->m_ptr_list[T_i->m_ptr_list_index] = this->m_ptr_list[this->m_ptr_list_count];
//    }
//}

// defined in phys_render
//void    make_rotate(phys_mat44 *mat, const phys_vec3 *v1, const phys_vec3 *v2)
//{
//    float v4; // [esp+8h] [ebp-2Ch]
//    float v5; // [esp+Ch] [ebp-28h]
//    float len; // [esp+14h] [ebp-20h]
//    float lena; // [esp+14h] [ebp-20h]
//    float co_; // [esp+18h] [ebp-1Ch] BYREF
//    float v9; // [esp+1Ch] [ebp-18h]
//    float si_; // [esp+20h] [ebp-14h]
//    phys_vec3 ud; // [esp+24h] [ebp-10h]
//    float retaddr; // [esp+34h] [ebp+0h]
//
//    ud.y = a1;
//    ud.z = retaddr;
//    phys_cross((phys_vec3 *)&co_, v1, v2);
//    len = (float)((float)(co_ * co_) + (float)(v9 * v9)) + (float)(si_ * si_);
//    if ( len >= 0.0000099999997 )
//    {
//        lena = sqrtf(len);
//        co_ = co_ * (float)(1.0 / lena);
//        v9 = v9 * (float)(1.0 / lena);
//        si_ = si_ * (float)(1.0 / lena);
//        v5 = (float)((float)(v1->x * v2->x) + (float)(v1->y * v2->y)) + (float)(v1->z * v2->z);
//        v4 = sqrtf((float)(v5 * v5) + (float)(lena * lena));
//        make_rotate(mat, (const phys_vec3 *)&co_, v5 / v4, lena / v4);
//    }
//    else
//    {
//        phys_mat44::operator=(mat, &PHYS_IDENTITY_MATRIX_44);
//    }
//}

