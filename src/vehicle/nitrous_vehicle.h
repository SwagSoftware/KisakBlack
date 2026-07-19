#pragma once
#include <physics/phys_local.h>
#include <game/teams.h>
#include <gfx_d3d/r_material.h>
#include <qcommon/graph.h>
#include <physics/rigid_body.h>
#include "nitrous_vehicle_controller.h"

enum WheelEffectState : __int32
{                                       // XREF: WheelState/r
    WHEEL_STATE_ROLLING  = 0x0,
    WHEEL_STATE_AIRBORN  = 0x1,
    WHEEL_STATE_SKIDDING = 0x2,
    WHEEL_STATE_MAX      = 0x3,
};

enum TractionType : __int32
{                                       // XREF: VehicleParameter/r
    TRACTION_TYPE_FRONT  = 0x0,
    TRACTION_TYPE_BACK   = 0x1,
    TRACTION_TYPE_ALL_WD = 0x2,
    NUM_TRACTION_TYPES   = 0x3,
};

struct WheelState // sizeof=0x10
{                                       // XREF: NitrousVehicle/r
    WheelEffectState m_state;
    WheelEffectState m_last_state;
    float m_state_time;
    float m_rate;
};

struct view_limits_t // sizeof=0x10
{                                       // XREF: vehicle_info_t/r
                                        // vehicle_info_t/r
    float horizSpanLeft;
    float horizSpanRight;
    float vertSpanUp;
    float vertSpanDown;
};

struct VehicleParameter // sizeof=0x124
{                                       // XREF: .data:VehicleParameter g_default_params/r
                                        // vehicle_info_t/r ...
    VehicleParameter();

    float m_speed_max;
    float m_accel_max;
    float m_reverse_scale;
    float m_steer_angle_max;
    float m_steer_speed;
    float m_wheel_radius;
    float m_susp_spring_k;
    float m_susp_damp_k;
    float m_susp_adj;
    float m_susp_hard_limit;
    float m_susp_min_height;
    float m_tire_fric_fwd;
    float m_tire_fric_side;
    float m_tire_fric_brake;
    float m_tire_fric_hand_brake;
    float m_body_mass;
    float m_roll_stability;
    float m_pitch_roll_resistance;
    float m_yaw_resistance;
    float m_upright_strength;
    float m_tilt_fakey;
    float m_peel_out_max_speed;
    float m_tire_damp_coast;
    float m_tire_damp_brake;
    float m_tire_damp_hand;
    float m_auto_hand_brake_min_speed;
    TractionType m_traction_type;
    char m_name[64];
    float m_bbox_min[3];
    float m_bbox_max[3];
    float m_mass_center_offset[3];
    float m_buoyancybox_min[3];
    float m_buoyancybox_max[3];
    float m_water_speed_max;
    float m_water_accel_max;
    float m_water_turn_accel;
    float m_water_turn_speed_max;
    float m_water_ebrake_power;
    float m_boat_motor_offset[3];
    float m_boat_speed_rise;
    float m_boat_speed_tilt;
    float m_boat_side_fric_scale;
    float m_boat_forward_fric_scale;
    float m_boat_vertical_fric_scale;
    float m_jump_force;
    float m_tire_fric_side_max;
};

struct vehicle_info_t // sizeof=0x1DD8
{                                       // XREF: .data:vehicle_info_t * bg_vehicleInfos/r
    char name[64];
    __int16 type;                       // XREF: G_SpawnHelicopter(gentity_s *,gentity_s *,char const *,char const *)+70/w
    // padding byte
    // padding byte
    int remoteControl;
    int steerWheels;
    int texScroll;
    int quadBarrel;
    int bulletDamage;
    int armorPiercingDamage;
    int grenadeDamage;
    int projectileDamage;
    int projectileSplashDamage;
    int heavyExplosiveDamage;
    __int16 cameraMode;
    // padding byte
    // padding byte
    int autoRecenterOnAccel;
    int thirdPersonDriver;
    float thirdPersonCameraRange;
    float thirdPersonCameraMinPitchClamp;
    float thirdPersonCameraMaxPitchClamp;
    float thirdPersonCameraHeight[2];
    float thirdPersonCameraPitch[2];
    int cameraAlwaysAutoCenter;
    float cameraAutoCenterLerpRate;
    float cameraAutoCenterMaxLerpRate;
    float thirdPersonCameraSpringDistance;
    float thirdPersonCameraSpringTime;
    float thirdPersonCameraHandbrakeTurnRateInc;
    float cameraFOV;
    int killcamCollision;
    float killcamDist;
    float killcamZDist;
    float killcamMinDist;
    float killcamZTargetOffset;
    float killcamFOV;
    float killcamNearBlur;
    float killcamNearBlurStart;
    float killcamNearBlurEnd;
    float killcamFarBlur;
    float killcamFarBlurStart;
    float killcamFarBlurEnd;
    int isDrivable;                     // XREF: VEH_UpdateDebug(gentity_s *)+310/w
    int numberOfSeats;
    int numberOfGunners;
    int seatSwitchOrder[11];
    int driverControlledGunPos;
    float entryPointRadius[5];
    float texScrollScale;
    float wheelRotRate;
    float extraWheelRotScale;
    float maxSpeed;
    float accel;
    float rotRate;
    float rotAccel;
    float maxBodyPitch;
    float maxBodyRoll;
    float collisionDamage;
    float collisionSpeed;
    float suspensionTravel;
    __int16 boostMode;
    // padding byte
    // padding byte
    float boostSpeedIncrease;
    float boostAccelMultiplier;
    float boostInterval;
    float boostDuration;
    float boostFrictionMultiplier;
    float boostContinuousRegenCooldown;
    float boostContinuousRegenAmount;
    float heliCollisionScalar;
    float viewPitchOffset;
    float viewInfluence;
    float tiltFromAcceleration[2];
    float tiltFromDeceleration[2];
    float tiltFromVelocity[2];
    float tiltSpeed[2];
    char turretWeapon[64];
    view_limits_t turretViewLimits;
    float turretRotRate;
    int turretClampPlayerView;
    char turretTag1[64];
    char turretTag2[64];
    char turretTag3[64];
    char turretTag4[64];
    char gunnerWeapon[4][64];
    unsigned __int16 gunnerWeaponIndex[4];
                                        // XREF: VEH_InitVehicle(gentity_s *,scr_vehicle_s *,int)+4CF/o
                                        // VEH_UpdateGunnerWeapon+3A6/o ...
    float gunnerRotRate;
    float gunnerRestAngles[4][2];       // XREF: G_UpdateVehicleTags(gentity_s *)+31A/w
                                        // G_UpdateVehicleTags(gentity_s *)+33B/w
    view_limits_t passengerViewLimits[6];
    char sndNames[19][64];
    unsigned int sndIndices[19];
    char sndMaterialNames[3][64];
    float engineSndSpeed;
    float engineModLoopNaturalRPMs;
    float idleRPMs;
    float idleRPMsMax;
    float gear1MinRPMs;
    float gear1MaxRPMs;
    float gearChangeMPH;
    float gear2MinRPMs;
    float gear2MaxRPMs;
    int trackLengthInches;
    float clacksCurveDamageStart;
    float clacksCurveDamageEnd;
    float clacksCurveMaxOmitted;
    float treadGrindAttenuation;
    float treadDamageLoopHealthRatio;
    float skidSpeedMin;
    float skidSpeedMax;
    int animType;
    char animSet[64];
    float mantleAngles[4];
    int extraWheelTags[4];
    char attachmentModels[4][64];
    char attachmentTags[4][64];
    char deathAttachmentModels[4][64];
    char deathAttachmentTags[4][64];
    float tracerOffset[2];
    char model[64];
    char deathModel[64];
    float modelSwapDelay;
    char exhaustFx[64];
    int oneExhaust;
    char rotorMainIdleFx[64];
    char rotorMainStartFx[64];
    char rotorMainRunningFx[64];
    char rotorMainStopFx[64];
    char rotorTailIdleFx[64];
    char rotorTailStartFx[64];          // XREF: PM_GroundSurfaceType(playerState_s *,pml_t *)+47/o
                                        // PM_FootstepEvent(pmove_t *,pml_t *,int,int,int)+30B/o ...
    char rotorTailRunningFx[64];
    char rotorTailStopFx[64];
    char treadFx[31][64];
    char deathFxName[64];
    char deathFxTag[64];
    char deathFxSound[64];
    float radiusDamageMin;
    float radiusDamageMax;
    float radiusDamageRadius;
    char shootShock[64];
    char shootRumble[64];
    float deathQuakeScale;
    float deathQuakeDuration;
    float deathQuakeRadius;
    char secTurretType[64];
    char secTurretTag[64];
    char secTurretModel[64];
    int secTurretAiControlled;
    float frontArmorRegen;
    char rumbleType[64];
    float rumbleScale;
    float rumbleDuration;
    float rumbleRadius;
    float rumbleBaseTime;
    float rumbleAdditionalTime;
    int healthDefault;
    int healthMin;
    int healthMax;
    team_t eTeam;
    int addToCompass;
    int addToCompassEnemy;
    char compassIcon[64];
    Material *compassIconMaterial;
    int gasButton;
    int boostButton;
    int reverseBrakeButton;
    int handBrakeButton;
    int attackButton;
    int attackSecondaryButton;
    int moveUpButton;
    int moveDownButton;
    int switchSeatButton;
    int specialAbilityButton;
    int firePickupButton;
    int swapPickupButton;
    int dropDeployableButton;
    GraphFloat *steerGraph;
    GraphFloat *accelGraph;
    int isNitrous;
    int isFourWheelSteering;
    float max_fric_tilt_angle;
    float max_fric_tilt;
    int noDirectionalDamage;
    VehicleParameter nitrousVehParams;
};

struct rigid_body_constraint_wheel;
struct XModel;
struct rigid_body_constraint_custom_orientation;
struct rigid_body_constraint_custom_path;

struct alignas(16) NitrousVehicle // sizeof=0x440
{                                       // XREF: phys_free_list<NitrousVehicle>::T_internal/r
    NitrousVehicle();

    static NitrousVehicle *__cdecl add_vehicle(int id);
    static void remove_vehicle(NitrousVehicle *const v);

    void init(struct gentity_s *owner, const struct VehicleParameter *parameter);
    void init(
        int localClientNum,
        struct centity_s *owner,
        const VehicleParameter *parameter);

    void update_script_target(float *goal_position);

    void set_throttle(float throttle);

    void set_hand_brake(float braking);
    void set_brake(float braking);

    void unpause_physics();
    void pause_physics(bool shutdown);
    void _update_pause();

    void update_parms(const VehicleParameter *params, bool initialization);

    double _calc_initial_susp_spring_k(rigid_body_constraint_wheel *wheel_constraint);
    bool is_peeling_out();

    void destructible_damage(const char *notifyLabel, int damageStage);
    void damage(unsigned int boneName, int damageStage);
    void damage(int damage, const float *point, const float *dir, int mod);
    phys_vec3 *get_velocity(phys_vec3 *result);
    phys_vec3 *get_a_velocity(phys_vec3 *result);
    void _update_orientation_constraint();
    double _get_stuck_accel_factor(float delta_t);
    void _update_boost(float delta_t);
    void _update_prolog(float delta_t);
    void _update_epilog(float delta_t);
    void _update_friction(float delta_t);
    void _update_fakey_stuff(float delta_t);
    void start_path(int attach_mode);
    void end_path();
    void cleanup_path();
    static void __cdecl frame_prolog_all_systems(float delta_t);
    static void __cdecl frame_epilog_all_systems(float delta_t);
    static void __cdecl reinit_parms();
    static void __cdecl reset_vehicle_physics();
    static void __cdecl debug_render_all();
    void debug_render();
    void remove_wheels();
    void set(
        float power_braking_factor,
        float braking_factor,
        float desired_speed_factor,
        float acceleration_factor,
        float coasting_factor,
        float reference_wheel_radius,
        float steer_max_angle,
        float steer_speed,
        phys_vec3 *steer_front_pt_loc,
        float steer_front_back_length);

    void _setup_wheels(int localClientNum);
    void update_steering(float delta_t);
    void update_braking_and_acceleration(float __formal);
    void set_boosting(bool boosting);
    bool is_boosting();
    double get_maximum_acceleration_factor();
    double get_maximum_acceleration();
    double get_max_speed(bool include_perks);
    void update_control_from_network(LerpEntityStateVehicle *vehicleState);
    void update_from_network(
        trajectory_t pos,
        trajectory_t apos,
        LerpEntityStateVehicle *vehicleState,
        float timeDifference);
    void GetEntityPose(
        float *origin,
        float *angles,
        int curTime,
        bool isDriver);


    // nitrous_vehicle_effects
    void _update_wheel_effect_states(float delta_t);
    double debug_render_wheel_effects(int wheel);
    void debug_render_effects();

    WheelState m_wheel_state[4];
    phys_mat44 m_wheel_orig_relpo[6];
    int m_wheel_damage[6];
    int m_wheel_surf_types[6];
    VehicleParameter *m_parameter;
    float m_throttle;
    float m_brake;
    float m_hand_brake;
    float m_script_brake;
    float m_steer_factor;
    float m_forward_vel;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    phys_vec3 m_ground_vel;
    float m_hand_brake_friction_time;
    struct gentity_s *m_owner;
    int m_entnum;
    const vehicle_info_t *m_vehicle_info;
    struct PhysObjUserData *m_phys_user_data;
    XModel *m_xmodel;
    float m_origin[3];
    float m_angles[3];
    struct rigid_body_constraint_custom_orientation *m_orientation_constraint;
    rigid_body_constraint_custom_path *m_vpc;
    int m_flags;
    int m_notify_flags;
    int m_server_notify_flags;
    float m_collision_hitp[3];
    float m_collision_hitn[3];
    float m_collision_intensity;
    int m_collision_stype;
    int m_collision_entnum;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    phys_mat44 m_prev_rb_mat;
    NitrousVehicleController mVehicleController;
    float m_fake_rpm;
    int m_num_colliding_wheels;
    float m_current_front_side_fric_scale;
    float m_current_rear_side_fric_scale;
    float m_current_front_fwd_fric_scale;
    float m_current_rear_fwd_fric_scale;
    float m_stuck_time;
    int m_lastNetworkTime;
    int m_lastErrorReductionTime;
    float m_networkErrorOrigin[3];
    float m_networkErrorAngles[3];
    int m_trackDistAccumLeftInches;
    int m_trackDistAccumRightInches;
    int m_sfx_state_flags;
    float m_speed_scale;
    float m_drivepath_scale;
    int m_boost_start_time;
    float m_boost_time_pool;
    int m_throttle_held_down;
    bool m_throttle_time_started;
    // padding byte
    // padding byte
    // padding byte
    int id;
    int refcount;
    rigid_body_constraint_wheel *m_wheels[6];
    float m_desired_speed_factor;
    float m_acceleration_factor;
    float m_power_braking_factor;
    float m_braking_factor;
    float m_coasting_factor;
    float m_reference_wheel_radius;
    float m_steer_current_angle;
    float m_steer_max_angle;
    float m_steer_speed;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    phys_vec3 m_steer_front_pt_loc;
    float m_steer_front_back_length;
    unsigned int m_state_flags;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};
static_assert(sizeof(NitrousVehicle) == 0x440);

struct PhysObjUserData *__cdecl Phys_ObjCreateNitrousVehicle(
                const float *position,
                const float *angles,
                const float *velocity,
                const float *translation,
                const PhysPreset *physPreset,
                struct gjk_geom_list_t *gjk_geom_list);


double __cdecl AbsSquared(const phys_vec3 *a);

bool __cdecl CompareVec3ToPhysVec(const float *a, const phys_vec3 *b, float acceptableError);

void __cdecl G_SetVehDriverInputs(int localClientNum, int vehEntNum, usercmd_s *cmd);
void __cdecl G_ClearVehicleInputs();
void __cdecl CG_SetVehDriverInputs(int localClientNum, int vehEntNum, usercmd_s *cmd);
void __cdecl Vehicle_Launch(int localClientNum, centity_s *cent, float *hitp, const float *force, bool relative);


