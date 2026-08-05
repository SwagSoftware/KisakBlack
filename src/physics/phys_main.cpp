#include "phys_main.h"
#include "phys_effects.h"
#include <tl/tl_system.h>
#include "phys_mem_new.h"
#include "phys_task_manager.h"
#include "physics_system.h"
#include <universal/surfaceflags.h>
#include <gfx_d3d/rb_debug.h>
#include "phys_render.h"
#include "phys_broad_phase.h"
#include "phys_util.h"
#include <universal/com_math_anglevectors.h>
#include "physpreset_load_obj.h"
#include "phys_assert.h"
#include <bgame/bg_slidemove.h>
#include "physics_system_internal.h"
#include <DynEntity/DynEntity_client.h>
#include <cgame/cg_main.h>
#include <xanim/xmodel_utils.h>
#include <qcommon/cm_tracebox.h>
#include <DynEntity/DynEntity_load_obj.h>
#include <cgame/cg_world.h>
#include <qcommon/dobj_management.h>
#include <xanim/dobj_utils.h>
#include <cgame_mp/cg_pose_mp.h>
#include <qcommon/cm_staticmodel.h>
#include <qcommon/cm_load.h>
#include <qcommon/cm_world.h>
#include <game_mp/g_main_mp.h>
#include <gfx_d3d/r_debug.h>
#include <client/cl_keys.h>
#include <game_mp/g_utils_mp.h>
#include <game/g_debug.h>
#include <cgame/cg_drawtools.h>
#include "xdoll.h"
#include <ragdoll/ragdoll_update.h>
#include "phys_auto_rigid_body.h"
#include <demo/demo_playback.h>
#include <client/splitscreen.h>
#include <client/client.h>
#include "phys_convex_hull.h"

// i hereby curse ye, all who enter this file. May only misery and misfortune accompany you as you read further.

const char *g_debugRenderMaskNames[10] =
{
  "All",
  "Player",
  "PlayerClip Only",
  "Vehicle",
  "AI Collision",
  "AI Sight",
  "Weapon",
  "Mantle",
  "Physics",
  NULL
};

float spin_scale = 0.80000001;
float spin_scale_0 = 0.15000001;
float range_1 = 5000.0;

minspec_mutex g_render_mutex;

void *G_PHYSICS_TOTAL_MEMORY_BUFFER;

const dvar_t *phys_gravity;
const dvar_t *phys_gravity_dir;
const dvar_t *phys_vehicleGravityMultiplier;
const dvar_t *phys_vehicleDamageFroceScale;
const dvar_t *phys_vehicleUsePredictedPosition;
const dvar_t *phys_bulletUpBias;
const dvar_t *phys_bulletSpinScale;
const dvar_t *phys_msecStep;
const dvar_t *phys_drawcontacts;
const dvar_t *phys_drawCollisionObj;
const dvar_t *phys_debugBigQueries;
const dvar_t *phys_debugCallback;
const dvar_t *phys_debugDangerousRigidBodies;
const dvar_t *phys_debugExpensivePushout;
const dvar_t *phys_drawNitrousVehicle;
const dvar_t *phys_drawNitrousVehicleEffects;
const dvar_t *phys_entityCollision;
const dvar_t *phys_vehicleWheelEntityCollision;
const dvar_t *phys_vehicleFriction;
const dvar_t *phys_ragdoll_joint_damp_scale;
const dvar_t *phys_dragLinear;
const dvar_t *phys_dragAngular;
const dvar_t *phys_userRigidBodies;
const dvar_t *phys_waterDragLinear;
const dvar_t *phys_waterDragAngular;
const dvar_t *phys_maxFloatTime;
const dvar_t *phys_buoyancyDistanceCutoff;
const dvar_t *phys_piecesSpawnDistanceCutoff;
const dvar_t *phys_floatTimeVariance;
const dvar_t *phys_buoyancyRippleFrequency;
const dvar_t *phys_buoyancyRippleVariance;
const dvar_t *phys_buoyancyFastComputation;
const dvar_t *phys_buoyancy;
const dvar_t *phys_ragdoll_buoyancy;
const dvar_t *debug_trace;
const dvar_t *g_bDebugRenderBulletMeshes;
const dvar_t *g_bDebugRenderEntityBrushes;
const dvar_t *g_bDebugRenderPatches;
const dvar_t *g_bDebugRenderBrushes;
const dvar_t *g_bDebugRenderColoredPatches;
const dvar_t *g_debugRenderMask;
const dvar_t *g_debugRenderCollisionDistance;
const dvar_t *g_bDebugRenderStaticModelsBounds;
const dvar_t *g_dumpStaticModels;
const dvar_t *g_debugRenderGjkTraceGeom;
const dvar_t *phys_player_collision_mode;
const dvar_t *phys_player_collision_adjust_height;
const dvar_t *phys_ai_collision_mode;
const dvar_t *render_player_collision;
const dvar_t *render_actor_collision;
const dvar_t *render_bpi_env_collision;
const dvar_t *enable_moving_paths;
const dvar_t *enable_new_prone_check;
const dvar_t *phys_heavyTankSwitch;
const dvar_t *phys_fluid;

struct smodel_debug_info_t // sizeof=0x8
{                                       // XREF: .data:smodel_debug_info_t * smodel_debug_infos/r
    cStaticModel_s *smodel;             // XREF: draw_static_models_bounds(void)+DE/w
    float dist2;                        // XREF: draw_static_models_bounds(void)+ED/w
};

smodel_debug_info_t smodel_debug_infos[16384];

cdl_proftimer proftimer_physics_frame_advance;
PhysGlob physGlob;

int gImpulseCacheNum;
PhysImpulse gImpulseCache[16];

float g_delta_t;
phys_free_list<PhysObjUserData>::iterator g_pop_iter;
phys_free_list<PhysObjUserData>::iterator g_pop_iter_end;
axis_aligned_sweep_and_prune *g_axis_aligned_sweep_and_prune;

bool gravityChange;
static bool physInited = false;
void __cdecl Phys_Init()
{
    phys_surface_type_info v0; // [esp+30h] [ebp-68h] BYREF
    phys_surface_type_info v1; // [esp+40h] [ebp-58h] BYREF
    phys_surface_type_info v2; // [esp+50h] [ebp-48h] BYREF
    phys_surface_type_info v3; // [esp+60h] [ebp-38h] BYREF
    phys_surface_type_info pst; // [esp+70h] [ebp-28h] BYREF
    float surfaceFriction; // [esp+80h] [ebp-18h]
    float surfaceBounce; // [esp+84h] [ebp-14h]
    int i; // [esp+88h] [ebp-10h]

    physGlob.timeLastSnapshot = 0;
    physGlob.timeLastUpdate = 0;
    Phys_InitSoundAliases();
    if ( !physInited )
    {
        physInited = 1;
        phys_gravity = _Dvar_RegisterFloat(
                                         "phys_gravity",
                                         -800.0,
                                         -3.4028235e38,
                                         3.4028235e38,
                                         0x1000u,
                                         "Physics gravity in units/sec^2.");
        phys_gravity_dir = _Dvar_RegisterVec3(
                                                 "phys_gravity_dir",
                                                 (0.0),
                                                 (0.0),
                                                 (1.0),
                                                 -1.0,
                                                 1.0,
                                                 0x1000u,
                                                 "Sets the gravity direction.");
        phys_vehicleGravityMultiplier = _Dvar_RegisterFloat(
                                                                            "phys_vehicleGravityMultiplier",
                                                                            1.0,
                                                                            0.5,
                                                                            2.0,
                                                                            0x1000u,
                                                                            "Physics gravity multiplier for vehicles.");
        phys_vehicleDamageFroceScale = _Dvar_RegisterFloat(
                                                                         "phys_vehicleDamageFroceScale",
                                                                         0.30000001,
                                                                         0.0,
                                                                         10.0,
                                                                         0x1000u,
                                                                         "Physics vehicle damamge force scale.");
        phys_vehicleUsePredictedPosition = _Dvar_RegisterBool(
                                                                                 "phys_vehicleUsePredictedPosition",
                                                                                 0,
                                                                                 0x1000u,
                                                                                 "Turning this on forces the client to use the vehicles predicted position.");
        phys_bulletUpBias = _Dvar_RegisterFloat(
                                                    "phys_bulletUpBias",
                                                    0.5,
                                                    0.0,
                                                    2.0,
                                                    0,
                                                    "Up Bias for the direction of the bullet impact.");
        phys_bulletSpinScale = _Dvar_RegisterFloat(
                                                         "phys_bulletSpinScale",
                                                         3.0,
                                                         0.0099999998,
                                                         100.0,
                                                         0,
                                                         "Scale of the effective offset from the center of mass for the bullet impacts.");
        phys_msecStep = _Dvar_RegisterInt(
                                            "phys_msecstep",
                                            0,
                                            0,
                                            1000,
                                            0,
                                            "Physics simulation update increment in milliseconds");
        phys_drawcontacts = _Dvar_RegisterBool("phys_drawcontacts", 0, 0, "Debug draw contact points");
        phys_drawCollisionObj = _Dvar_RegisterBool(
                                                            "phys_drawCollisionObj",
                                                            0,
                                                            0,
                                                            "Debug draw collision geometry for each physics object");
        phys_debugBigQueries = _Dvar_RegisterBool(
                                                         "phys_debugBigQueries",
                                                         0,
                                                         0,
                                                         "Draw debug info when phys object moves very fast therefore generating huge query boxes");
        phys_debugCallback = _Dvar_RegisterBool("phys_debugCallback", 0, 0, "Enbles the physics debug callback");
        phys_debugDangerousRigidBodies = _Dvar_RegisterBool(
                                                                             "phys_debugDangerousRigidBodies",
                                                                             1,
                                                                             0,
                                                                             "Draws debug line off the origins of dangerous rigid bodies");
        phys_debugExpensivePushout = _Dvar_RegisterBool(
                                                                     "phys_debugExpensivePushout",
                                                                     1,
                                                                     0,
                                                                     "Draw debug info when the engine resort to exensive Player/AI pushouts");
        phys_drawNitrousVehicle = _Dvar_RegisterBool(
                                                                "phys_drawNitrousVehicle",
                                                                0,
                                                                0,
                                                                "Print info about the nitrous vehicles");
        phys_drawNitrousVehicleEffects = _Dvar_RegisterBool(
                                                                             "phys_drawNitrousVehicleEffects",
                                                                             0,
                                                                             0,
                                                                             "Print info about the nitrous vehicle wheel effects");
        phys_entityCollision = _Dvar_RegisterBool("phys_entityCollision", 1, 0, "Enable to turn on entity collision.");
        phys_vehicleWheelEntityCollision = _Dvar_RegisterBool(
                                                                                 "phys_vehicleWheelEntityCollision",
                                                                                 1,
                                                                                 0x1000u,
                                                                                 "Enable to turn on vehicle wheel entity collision.");
        phys_vehicleFriction = _Dvar_RegisterFloat(
                                                         "phys_vehicleFriction",
                                                         0.1,
                                                         0.0,
                                                         2.0,
                                                         0,
                                                         "Default vehicle friction with the world and other ents.");
        phys_ragdoll_joint_damp_scale = _Dvar_RegisterFloat(
                                                                            "phys_ragdoll_joint_damp_scale",
                                                                            0.039999999,
                                                                            0.0,
                                                                            3.4028235e38,
                                                                            0,
                                                                            "Sets the phys joint damp for ragdolls.");
        phys_dragLinear = _Dvar_RegisterFloat(
                                                "phys_dragLinear",
                                                0.029999999,
                                                0.0,
                                                3.4028235e38,
                                                0,
                                                "The amount of linear drag, applied globally");
        phys_dragAngular = _Dvar_RegisterFloat(
                                                 "phys_dragAngular",
                                                 0.5,
                                                 0.0,
                                                 3.4028235e38,
                                                 0,
                                                 "The amount of angular drag, applied globally");
        phys_userRigidBodies = _Dvar_RegisterBool("phys_userRigidBodies", 1, 0, "Enables user rigid bodies.");
        phys_waterDragLinear = _Dvar_RegisterFloat(
                                                         "phys_waterDragLinear",
                                                         -1.0,
                                                         -3.4028235e38,
                                                         0.0,
                                                         0,
                                                         "The amount of linear drag for buoyant objects");
        phys_waterDragAngular = _Dvar_RegisterFloat(
                                                            "phys_waterDragAngular",
                                                            -2.0,
                                                            -3.4028235e38,
                                                            0.0,
                                                            0,
                                                            "The amount of angular drag for buoyant objects");
        phys_maxFloatTime = _Dvar_RegisterInt(
                                                    "phys_maxFloatTime",
                                                    30000,
                                                    10000,
                                                    0x7FFFFFFF,
                                                    0x1000u,
                                                    "Maximum time objects can float (in ms)");
        phys_buoyancyDistanceCutoff = _Dvar_RegisterFloat(
                                                                        "phys_buoyancyDistanceCutoff",
                                                                        2500.0,
                                                                        0.0,
                                                                        3.4028235e38,
                                                                        0x1000u,
                                                                        "Objects that are too far will sink.");
        phys_piecesSpawnDistanceCutoff = _Dvar_RegisterFloat(
                                                                             "phys_piecesSpawnDistanceCutoff",
                                                                             1000.0,
                                                                             0.0,
                                                                             3.4028235e38,
                                                                             0x1000u,
                                                                             "Determine how far we can spawn destructible pieces.");
        phys_floatTimeVariance = _Dvar_RegisterInt(
                                                             "phys_floatTimeVariance",
                                                             10000,
                                                             0,
                                                             0x7FFFFFFF,
                                                             0,
                                                             "Variance in time (in ms) objects will float for");
        phys_buoyancyRippleFrequency = _Dvar_RegisterInt(
                                                                         "phys_buoyancyRippleFrequency",
                                                                         3000,
                                                                         0,
                                                                         0x7FFFFFFF,
                                                                         0,
                                                                         "How often (in ms) floating objects will play the ripple fx");
        phys_buoyancyRippleVariance = _Dvar_RegisterInt(
                                                                        "phys_buoyancyRippleVariance",
                                                                        2000,
                                                                        0,
                                                                        0x7FFFFFFF,
                                                                        0,
                                                                        "How late (in ms) the ripple fx can be played");
        phys_buoyancyFastComputation = _Dvar_RegisterBool(
                                                                         "phys_buoyancyFastComputation",
                                                                         0,
                                                                         0,
                                                                         "Simplify the buoyancy calculations");
        phys_buoyancy = _Dvar_RegisterBool("phys_buoyancy", 0, 0x1000u, "Used to turn on/off buoyancy");
        phys_ragdoll_buoyancy = _Dvar_RegisterBool(
                                                            "phys_ragdoll_buoyancy",
                                                            0,
                                                            0x1000u,
                                                            "Used to turn on/off the buoyancy for he ragdolls");
        debug_trace = _Dvar_RegisterInt("debug_trace", 0, 0, 0x7FFFFFFF, 0, "Does a trace and draws the hit point");
        g_bDebugRenderBulletMeshes = _Dvar_RegisterBool(
                                                                     "debugRenderBulletMeshes",
                                                                     0,
                                                                     0,
                                                                     "Render bullet meshes (point to entity)");
        g_bDebugRenderEntityBrushes = _Dvar_RegisterBool(
                                                                        "debugRenderEntityBrushes",
                                                                        0,
                                                                        0,
                                                                        "Render the brushes associated with the entity you are looking at");
        g_bDebugRenderPatches = _Dvar_RegisterBool("debugRenderPatches", 0, 0, "Render patches");
        g_bDebugRenderBrushes = _Dvar_RegisterBool("debugRenderBrushes", 0, 0, "Render brushes");
        g_bDebugRenderColoredPatches = _Dvar_RegisterBool(
                                                                         "debugRenderColoredPatches",
                                                                         0,
                                                                         0,
                                                                         "Render each patch with a different color (Render Patches must be on)");
        g_debugRenderMask = _Dvar_RegisterEnum(
                                                    "debugRenderMask",
                                                    g_debugRenderMaskNames,
                                                    0,
                                                    0x80u,
                                                    "Mask for collision rendering");
        g_debugRenderCollisionDistance = _Dvar_RegisterFloat(
                                                                             "debugRenderCollisionDistance",
                                                                             800.0,
                                                                             100.0,
                                                                             2000.0,
                                                                             0x80u,
                                                                             "Distance to render debug brushes and patches");
        g_bDebugRenderStaticModelsBounds = _Dvar_RegisterBool(
                                                                                 "debugRenderStaticModelsBounds",
                                                                                 0,
                                                                                 0,
                                                                                 "Render the bounds of the nearby static models");
        g_dumpStaticModels = _Dvar_RegisterBool("dumpStaticModels", 0, 0, "Prints info about the static models");
        g_debugRenderGjkTraceGeom = _Dvar_RegisterBool("debugRenderGjkTraceGeom", 0, 0, "Render gjk trace meshes");
        phys_player_collision_mode = _Dvar_RegisterInt(
                                                                     "phys_player_collision_mode",
                                                                     1,
                                                                     0,
                                                                     1,
                                                                     0,
                                                                     "phys player collision mode");
        phys_player_collision_adjust_height = _Dvar_RegisterFloat(
                                                                                        "phys_player_collision_adjust_height",
                                                                                        0.0,
                                                                                        0.0,
                                                                                        5.0,
                                                                                        0,
                                                                                        "adjust the height of the gjk player collision capsule");
        phys_ai_collision_mode = _Dvar_RegisterInt("phys_ai_collision_mode", 1, 0, 1, 0, "phys ai collision mode");
        render_player_collision = _Dvar_RegisterInt("render_player_collision", 0, 0, 1, 0, "render player collision");
        render_actor_collision = _Dvar_RegisterInt("render_actor_collision", 0, 0, 1, 0, "render actor collision");
        render_bpi_env_collision = _Dvar_RegisterInt("render_bpi_env_collision", 0, 0, 5000, 0, "render bpi_env collision");
        enable_moving_paths = _Dvar_RegisterInt("enable_moving_paths", 1, 0, 1, 0, "enable moving paths");
        enable_new_prone_check = _Dvar_RegisterInt("enable_new_prone_check", 0, 0, 1, 0x80u, "enable new prone check");
        phys_heavyTankSwitch = _Dvar_RegisterBool(
                                                         "phys_heavyTankSwitch",
                                                         0,
                                                         0,
                                                         "Enable 'heavier' tank physics in multiplayer");
        phys_fluid = _Dvar_RegisterInt("phys_fluid", 0, 0, 5, 0, "Turn on and off the fluid simulation code");
        Phys_EffectsInit();
        if ( G_PHYSICS_TOTAL_MEMORY_BUFFER
            && _tlAssert(
                     "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_main.cpp",
                     341,
                     "G_PHYSICS_TOTAL_MEMORY_BUFFER == NULL",
                     "") )
        {
            __debugbreak();
        }
        G_PHYSICS_TOTAL_MEMORY_BUFFER = tlMemAlloc(0x380000u, 0x10u, 0);
        phys_memory_manager_init(G_PHYSICS_TOTAL_MEMORY_BUFFER, 3670016);
        phys_task_manager_init();
        phys_sys::phys_init();

        broad_phase_memory_info bpmi; // [esp+8Ch] [ebp-Ch] BYREF

        //broad_phase_memory_info::broad_phase_memory_info(&bpmi);
        bpmi.m_max_num_gjk_ci = 2048;
        bpmi.m_max_num_sap_active_pairs = 2048;
        bpmi.m_max_num_surface_types = 35;
        broad_phase_system_init(
            &bpmi,
            Phys_ShouldCollideCallback);
        phys_sys::set_collision_callback(Phys_CollisionCallback);
        phys_sys::set_max_delta_t(1.0);
        phys_sys::set_v_tol(8);
        set_bp_standard_query();
        set_debug_callback();
        pst.m_friction_coef = 0.4f;
        pst.m_bounce_coef = 0.1f;
        pst.m_solver_priority = 0;
        pst.m_no_overflow_error = 1;
        surface_type_info_database_set(31, 31, &pst);
        v3.m_friction_coef = 0.5f;
        v3.m_bounce_coef = 0.1f;
        v3.m_solver_priority = 0;
        v3.m_no_overflow_error = 1;
        surface_type_info_database_set(31, 32, &v3);
        v2.m_friction_coef = 0.5f;
        v2.m_bounce_coef = 0.1f;
        v2.m_solver_priority = 0;
        v2.m_no_overflow_error = 1;
        surface_type_info_database_set(32, 32, &v2);
        for ( i = 0; i < 31; ++i )
        {
            surfaceFriction = Com_SurfaceFrictionScale(i);
            surfaceBounce = Com_SurfaceBounceScale(i);
            v1.m_friction_coef = surfaceFriction;
            v1.m_bounce_coef = surfaceBounce;
            v1.m_solver_priority = 1;
            v1.m_no_overflow_error = 0;
            surface_type_info_database_set(31, i, &v1);
            v0.m_friction_coef = surfaceFriction;
            v0.m_bounce_coef = surfaceBounce;
            v0.m_solver_priority = 1;
            v0.m_no_overflow_error = 0;
            surface_type_info_database_set(32, i, &v0);
        }
        RB_SetDebugBrushesAndPatchesCallback(render_debug_brushes_and_patches);
    }
}

int __cdecl surface_type_info_database_get_index(int surface_type_1, int surface_type_2)
{
    int v3; // [esp+0h] [ebp-8h]

    if ((surface_type_1 < 0 || surface_type_1 >= G_BPM->m_max_num_surface_types)
        && _tlAssert(
            "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\collision\\phys_broad_phase_inline.h",
            336,
            "surface_type_1 >= 0 && surface_type_1 < G_BPM->m_max_num_surface_types",
            ""))
    {
        __debugbreak();
    }
    if ((surface_type_2 < 0 || surface_type_2 >= G_BPM->m_max_num_surface_types)
        && _tlAssert(
            "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\collision\\phys_broad_phase_inline.h",
            337,
            "surface_type_2 >= 0 && surface_type_2 < G_BPM->m_max_num_surface_types",
            ""))
    {
        __debugbreak();
    }
    if (surface_type_1 < surface_type_2)
        v3 = surface_type_1 + surface_type_2 * (surface_type_2 + 1) / 2;
    else
        v3 = surface_type_2 + surface_type_1 * (surface_type_1 + 1) / 2;
    if (v3 < 0 || v3 >= G_BPM->m_max_num_surface_type_infos)
    {
        if (_tlAssert(
            "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\collision\\phys_broad_phase_inline.h",
            339,
            "index >= 0 && index < G_BPM->m_max_num_surface_type_infos",
            ""))
        {
            __debugbreak();
        }
    }
    return v3;
}

void __cdecl surface_type_info_database_set(int surface_type_1, int surface_type_2, const phys_surface_type_info *pst)
{
    // LWSS: this is some temp debug kludge leftover
    //if ( !G_BPM->g_surface_type_info_database )
    //    break_here_27 = 1;
    if ( !G_BPM->g_surface_type_info_database
        && _tlAssert(
                 "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\collision\\phys_broad_phase_inline.h",
                 351,
                 "G_BPM->g_surface_type_info_database",
                 "") )
    {
        __debugbreak();
    }
    G_BPM->g_surface_type_info_database[surface_type_info_database_get_index(surface_type_1, surface_type_2)] = *pst;
}

broad_phase_info *__cdecl create_broad_phase_info()
{
    phys_free_list<broad_phase_info> *p_g_list_broad_phase_info; // edi
    phys_free_list<broad_phase_info>::T_internal *v1; // eax
    phys_free_list<broad_phase_info>::T_internal *v2; // esi

    p_g_list_broad_phase_info = &G_BPM->g_list_broad_phase_info;
    v1 = (phys_free_list<broad_phase_info>::T_internal *)PMM_ALLOC(0x90u, 0x10u);
    v2 = v1;
    if (v1)
    {
        v1->m_prev_T_internal = &p_g_list_broad_phase_info->m_dummy_head;
        v1->m_next_T_internal = p_g_list_broad_phase_info->m_dummy_head.m_next_T_internal;
        p_g_list_broad_phase_info->m_dummy_head.m_next_T_internal->m_prev_T_internal = v1;
        ++p_g_list_broad_phase_info->m_list_count;
        p_g_list_broad_phase_info->m_dummy_head.m_next_T_internal = v1;
        //phys_free_list<broad_phase_info>::debug_add(p_g_list_broad_phase_info, v1);
        p_g_list_broad_phase_info->debug_add(v1);
        return &v2->m_data;
    }
    else
    {
        if (_tlAssert(
            "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\phys_mem.h",
            470,
            "no_error",
            "phys_free_list error: out of memory."))
        {
            __debugbreak();
        }
        return 0;
    }
}

void __cdecl create_broad_phase_info(rigid_body *body)
{
    unsigned int v1; // eax
    unsigned int geom_id; // eax
    gjk_base_t **p_m_first_geom; // [esp+4h] [ebp-2Ch]
    gjk_base_t **v4; // [esp+10h] [ebp-20h]
    broad_phase_info *new_bpi; // [esp+18h] [ebp-18h]
    gjk_base_t *gjk_geom; // [esp+1Ch] [ebp-14h]
    broad_phase_info *bpi; // [esp+20h] [ebp-10h]
    broad_phase_group *bpg; // [esp+24h] [ebp-Ch]
    gjk_base_t *geom; // [esp+28h] [ebp-8h]
    PhysObjUserData *userData; // [esp+2Ch] [ebp-4h]

    userData = (PhysObjUserData *)body->m_userdata;
    if (userData->m_gjk_geom_list.get_geom_count() <= 1)
    {
        if (userData->m_gjk_geom_list.get_geom_count() != 1
            && _tlAssert(
                "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_main.cpp",
                429,
                "userData->m_gjk_geom_list.get_geom_count() == 1",
                ""))
        {
            __debugbreak();
        }
        p_m_first_geom = &userData->m_gjk_geom_list.m_first_geom;
        if (userData->m_gjk_geom_list.m_geom_count < 0
            && _tlAssert(
                "c:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_colgeom.h",
                1035,
                "m_geom_count >= 0",
                ""))
        {
            __debugbreak();
        }
        gjk_geom = *p_m_first_geom;
        if (!*p_m_first_geom
            && _tlAssert("C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_main.cpp", 431, "gjk_geom", ""))
        {
            __debugbreak();
        }
        new_bpi = create_broad_phase_info();
        geom_id = gjk_geom->get_geom_id();
        //broad_phase_info::set(
            new_bpi->set(
            body,
            &body->m_mat,
            &userData->cg2w,
            &userData->cg2rb,
            gjk_geom,
            geom_id,
            1,
            31,
            0,
            0x1C7u);
        userData->m_bpb = new_bpi;
        //if (rigid_body::get_flag(body, 0x20u))
        if (body->get_flag(0x20u))
            new_bpi->m_env_collision_flags &= ~1u;
    }
    else
    {
        bpg = create_broad_phase_group();
        //broad_phase_group::set(bpg);
        bpg->set();
        v4 = &userData->m_gjk_geom_list.m_first_geom;
        if (userData->m_gjk_geom_list.m_geom_count < 0
            && _tlAssert(
                "c:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_colgeom.h",
                1035,
                "m_geom_count >= 0",
                ""))
        {
            __debugbreak();
        }
        geom = *v4;
        if (!*v4 && _tlAssert("C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_main.cpp", 417, "geom", ""))
            __debugbreak();
        while (geom)
        {
            bpi = create_broad_phase_info();
            //v1 = gjk_base_t::get_geom_id(geom);
            v1 = geom->get_geom_id();
            //broad_phase_info::set(bpi, body, &body->m_mat, &userData->cg2w, &userData->cg2rb, geom, v1, 1, 31, 0, 0x1C7u);
            bpi->set(body, &body->m_mat, &userData->cg2w, &userData->cg2rb, geom, v1, 1, 31, 0, 0x1C7u);
            //broad_phase_group::add_bpi(bpg, bpi);
            bpg->add_bpi(bpi);
            geom = geom->m_next_geom;
        }
        userData->m_bpb = bpg;
    }
    environment_collision_list_add(userData->m_bpb);
    aasap_list_add(userData->m_bpb);
}

unsigned int __thiscall rigid_body::get_flag(unsigned int f)
{
    if ( (f & 0xFFFFFF00) != 0
        && _tlAssert(
                 "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\rigid_body.h",
                 60,
                 "(f & ~(FLAG_CLIENT_FLAGS_START - 1)) == 0",
                 "invalid rigid body flag") )
    {
        __debugbreak();
    }
    return f & this->m_flags;
}

void __thiscall broad_phase_group::set()
{
    this->m_list_bpi_head = 0;
    this->m_bpi_count = 0;
    this->m_flags = 0;
    this->m_rbvm = 0;
    this->m_list_wci = 0;
    this->m_sap_node = 0;
    this->m_list_bpb_next = 0;
    this->m_user_data = 0;
    this->m_flags |= 2u;
    this->m_env_collision_flags = 0;
    this->m_my_collision_type_flags = 0;
}

void __thiscall broad_phase_group::add_bpi(broad_phase_info *bpi)
{
    if ( (bpi->m_flags & 0x40) != 0
        && _tlAssert(
                 "C:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\collision\\phys_broad_phase.h",
                 92,
                 "!bpi->get_flag(broad_phase_info::FLAG_ON_BPG_LIST)",
                 "") )
    {
        __debugbreak();
    }
    environment_collision_list_remove(bpi);
    bpi->m_flags |= 0x40u;
    bpi->m_list_bpb_next = this->m_list_bpi_head;
    this->m_list_bpi_head = bpi;
    ++this->m_bpi_count;
    this->m_env_collision_flags |= bpi->m_env_collision_flags;
    this->m_my_collision_type_flags |= bpi->m_my_collision_type_flags;
}

void __cdecl environment_collision_list_add(broad_phase_base *bpb)
{
    broad_phase_memory *v1; // [esp+4h] [ebp-4h]

    iassert(bpb->is_bpi() || bpb->is_bpg());

    if ( (bpb->m_flags & 0x40) != 0
        && _tlAssert(
                 "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\collision\\phys_broad_phase_inline.h",
                 236,
                 "!bpb->get_flag(broad_phase_base::FLAG_ON_BPG_LIST)",
                 "") )
    {
        __debugbreak();
    }
    if ( (bpb->m_flags & 0x20) == 0 )
    {
        if ( bpb->m_list_bpb_next )
        {
            if ( _tlAssert(
                         "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\collision\\phys_broad_phase_inline.h",
                         239,
                         "bpb->m_list_bpb_next == NULL",
                         "") )
            {
                __debugbreak();
            }
        }
        v1 = G_BPM;
        bpb->m_list_bpb_next = G_BPM->g_list_bpb;
        v1->g_list_bpb = bpb;
        ++v1->g_list_bpb_count;
        bpb->m_flags |= 0x20u;
    }
}

void __cdecl environment_collision_list_remove(broad_phase_base *bpb)
{
    iassert(bpb->is_bpi() || bpb->is_bpg());

    if ( (bpb->m_flags & 0x40) != 0
        && _tlAssert(
                 "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\collision\\phys_broad_phase_inline.h",
                 248,
                 "!bpb->get_flag(broad_phase_base::FLAG_ON_BPG_LIST)",
                 "") )
    {
        __debugbreak();
    }
    if ( (bpb->m_flags & 0x20) != 0 )
    {
        //broad_phase_memory::list_bpb_remove(G_BPM, bpb);
        G_BPM->list_bpb_remove(bpb);
        bpb->m_list_bpb_next = 0;
        bpb->m_flags &= ~0x20u;
    }
}

void __thiscall broad_phase_memory::list_bpb_remove(broad_phase_base *bpb_to_remove)
{
    broad_phase_base *bpb; // [esp+4h] [ebp-8h]
    broad_phase_base **bpb_cur; // [esp+8h] [ebp-4h]

    bpb_cur = &this->g_list_bpb;
    for ( bpb = this->g_list_bpb; bpb != bpb_to_remove; bpb = bpb->m_list_bpb_next )
    {
        if ( !bpb )
        {
            if ( _tlAssert(
                         "c:\\projects_pc\\cod\\codsrc\\tl\\physics\\include\\collision\\phys_broad_phase_inline.h",
                         105,
                         "bpb",
                         "") )
            {
                __debugbreak();
            }
        }
        bpb_cur = &bpb->m_list_bpb_next;
    }
    *bpb_cur = bpb->m_list_bpb_next;
    --this->g_list_bpb_count;
}

void __cdecl destroy_broad_phase_info(rigid_body *body)
{
    broad_phase_info *bpi; // eax
    broad_phase_group *bpg; // eax
    broad_phase_group *v3; // eax
    PhysObjUserData *userData; // [esp+8h] [ebp-4h]

    userData = (PhysObjUserData *)body->m_userdata;
    if ( userData->m_bpb )
    {
        aasap_list_remove(userData->m_bpb);
        environment_collision_list_remove(userData->m_bpb);
        if ( userData->m_bpb->is_bpi() )
        {
            //bpi = broad_phase_base::get_bpi(userData->m_bpb);
            bpi = userData->m_bpb->get_bpi();
            destroy_broad_phase_info(bpi);
        }
        else
        {
            if ( (userData->m_bpb->m_flags & 2) == 0 )
            {
                if ( _tlAssert(
                             "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_main.cpp",
                             458,
                             "userData->m_bpb->is_bpg()",
                             "") )
                {
                    __debugbreak();
                }
            }
            //bpg = broad_phase_base::get_bpg(userData->m_bpb);
            bpg = userData->m_bpb->get_bpg();
            destroy_broad_phase_info_list(bpg->m_list_bpi_head);
            //v3 = broad_phase_base::get_bpg(userData->m_bpb);
            v3 = userData->m_bpb->get_bpg();
            destroy_broad_phase_group(v3);
        }
        userData->m_bpb = 0;
    }
}

PhysObjUserData * Phys_CreateUserBody(float *position, int id, PhysicsGeomType geomType)
{
    int j; // [esp+14h] [ebp-21Ch]
    float v7; // [esp+28h] [ebp-208h]
    float v8; // [esp+2Ch] [ebp-204h]
    float x; // [esp+94h] [ebp-19Ch]
    float y; // [esp+98h] [ebp-198h]
    float z; // [esp+9Ch] [ebp-194h]
    float max_[3]; // [esp+ACh] [ebp-184h] BYREF
    float mn_[3]; // [esp+B8h] [ebp-178h] BYREF
    float mx[3]; // [esp+C4h] [ebp-16Ch] BYREF
    float mn[3]; // [esp+D0h] [ebp-160h] BYREF
    float capsuleCenter[3]; // [esp+E4h] [ebp-14Ch] BYREF
    float rot[3][3]; // [esp+F0h] [ebp-140h] BYREF
    float trans[3]; // [esp+11Ch] [ebp-114h] BYREF
    gjk_base_t *gjk_geom; // [esp+128h] [ebp-108h]
    float rot_[3][3]; // [esp+12Ch] [ebp-104h] BYREF
    float trans_[4]; // [esp+158h] [ebp-D8h] BYREF
    user_rigid_body *body; // [esp+168h] [ebp-C8h]
    int bodyId; // [esp+16Ch] [ebp-C4h]
    PhysObjUserData *userData; // [esp+170h] [ebp-C0h]
    PhysGlob *v31; // [esp+174h] [ebp-BCh]
    phys_free_list<PhysObjUserData>::iterator i; // [esp+178h] [ebp-B8h]
    phys_free_list<PhysObjUserData>::T_internal_base *m_next_T_internal; // [esp+17Ch] [ebp-B4h]
    float centerOfMass[3]; // [esp+180h] [ebp-B0h] BYREF
    phys_mat44 dictator; // [esp+1C4h] [ebp-6Ch] BYREF
    float v36; // [esp+204h] [ebp-2Ch]
    float v37; // [esp+208h] [ebp-28h]
    float v38; // [esp+20Ch] [ebp-24h]
    phys_vec3 pos; // [esp+214h] [ebp-1Ch] BYREF
    //_UNKNOWN *v40[2]; // [esp+224h] [ebp-Ch] BYREF
    //PhysicsGeomType geomTypea; // [esp+230h] [ebp+0h]
    //
    //*(float *)v40 = a1;
    //v40[1] = (_UNKNOWN *)geomTypea;
    Phys_Vec3ToNitrousVec(position, &pos);
    
    dictator.x.x = 1.0f;
    dictator.x.y = 0.0f;
    dictator.x.z = 0.0f;
    dictator.y.x = 0.0f;
    dictator.y.y = 1.0f;
    dictator.y.z = 0.0f;
    dictator.z.x = 0.0f;
    dictator.z.y = 0.0f;
    dictator.z.z = 1.0f;
    dictator.w.x = pos.x;
    dictator.w.y = pos.y;
    dictator.w.z = pos.z;

    memset(centerOfMass, 0, 12);

    Sys_EnterCriticalSection(CRITSECT_PHYSICS);

    // aislopped loop cleanup
    for (auto *node = physGlob.objects.m_dummy_head.m_next_T_internal;
        node != &physGlob.objects.m_dummy_head;
        node = node->m_next_T_internal)
    {
        using TI = phys_free_list<PhysObjUserData>::T_internal;
        TI *ti = (TI *)node;
        userData = &ti->m_data;
        if (userData->id == id)
        {
            iassert(userData->body->is_user_rigid_body());
            ++userData->refcount;
            Sys_LeaveCriticalSection(CRITSECT_PHYSICS);
            return userData;
        }
    }

    //m_next_T_internal = physGlob.objects.m_dummy_head.m_next_T_internal;
    //for (i.m_ptr = physGlob.objects.m_dummy_head.m_next_T_internal; ; i.m_ptr = i.m_ptr->m_next_T_internal)
    //{
    //    v31 = &physGlob;
    //    if (&physGlob == (PhysGlob *)i.m_ptr)
    //        break;
    //    userData = (PhysObjUserData *)&i.m_ptr[2];
    //    if (i.m_ptr[29].m_next_T_internal == id)
    //    {
    //        iassert(userData->body->is_user_rigid_body());
    //        ++userData->refcount;
    //        Sys_LeaveCriticalSection(CRITSECT_PHYSICS);
    //        return userData;
    //    }
    //}

    body = phys_sys::create_user_rigid_body(1);
    if (body)
    {
        userData = Phys_CreateUserData(0);
        if (userData)
        {
            //LODWORD(trans_[3]) = geomType - 3;
            switch (geomType)
            {
            case PHYS_GEOM_CYLINDER:
                trans_[0] = 0.0f;
                trans_[1] = 0.0f;
                trans_[2] = 35.0f;

                rot_[0][0] = 0.0f;
                rot_[0][1] = 0.0f;
                rot_[0][2] = 1.0f;

                rot_[1][0] = 0.0f;
                rot_[1][1] = -1.0f;
                rot_[1][2] = 0.0f;

                rot_[2][0] = 1.0f;
                rot_[2][1] = 0.0f;
                rot_[2][2] = 0.0f;

                gjk_geom = create_cylinder_gjk_geom(
                    rot_,
                    trans_,
                    15.0,
                    35.0,
                    7,
                    &g_empty_collision_visitor);
                break;
            case PHYS_GEOM_CYLINDER_LARGE:
                trans[0] = 0.0f;
                trans[1] = 0.0f;
                trans[2] = 20.0f;

                rot[0][0] = 0.0f;
                rot[0][1] = 0.0f;
                rot[0][2] = 1.0f;

                rot[1][0] = 0.0f;
                rot[1][1] = -1.0f;
                rot[1][2] = 0.0f;

                rot[2][0] = 1.0f;
                rot[2][1] = 0.0f;
                rot[2][2] = 0.0f;

                gjk_geom = create_cylinder_gjk_geom(rot, trans, 35.0f, 20.0f, 7, &g_empty_collision_visitor);
                break;
            case PHYS_GEOM_CAPSULE:
                capsuleCenter[0] = 0.0f;
                capsuleCenter[1] = 0.0f;
                capsuleCenter[2] = 45.0f;
                gjk_geom = create_capsule_gjk_geom(
                    capsuleCenter,
                    15.0f,
                    5.0f,
                    2u,
                    7,
                    &g_empty_collision_visitor);
                break;
            case PHYS_GEOM_POINT:
                mn[0] = -1.0f;
                mn[1] = -1.0f;
                mn[2] = -1.0f;
                mx[0] = 1.0f;
                mx[1] = 1.0f;
                mx[2] = 1.0f;
                gjk_geom = create_aabb_gjk_geom(mn, mx, 7, &g_empty_collision_visitor);
                break;
            default:
                mn_[0] = -15.0f;
                mn_[1] = -15.0f;
                mn_[2] = 0.0f;
                max_[0] = 15.0f;
                max_[1] = 15.0f;
                max_[2] = 60.0f;
                gjk_geom = create_aabb_gjk_geom(mn_, max_, 7, &g_empty_collision_visitor);
                break;
            }
            gjk_geom->comp_aabb_loc();
            userData->cg2rb = PHYS_IDENTITY_MATRIX;
            userData->cg2w = PHYS_IDENTITY_MATRIX;
            //phys_mat44::operator=(&userData->cg2rb, &PHYS_IDENTITY_MATRIX);
            //phys_mat44::operator=(&userData->cg2w, &PHYS_IDENTITY_MATRIX);
            if ((gjk_geom->m_flags & 2) == 0
                && _tlAssert(
                    "c:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_colgeom.h",
                    83,
                    "get_flag(FLAG_AABB_LOC_VALID)",
                    ""))
            {
                __debugbreak();
            }
            x = gjk_geom->m_aabb_mx_loc.x;
            y = gjk_geom->m_aabb_mx_loc.y;
            z = gjk_geom->m_aabb_mx_loc.z;
            if ((gjk_geom->m_flags & 2) == 0
                && _tlAssert(
                    "c:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_colgeom.h",
                    82,
                    "get_flag(FLAG_AABB_LOC_VALID)",
                    ""))
            {
                __debugbreak();
            }
            userData->cg2rb.w.x = -(0.5 * (gjk_geom->m_aabb_mn_loc.x + x));
            userData->cg2rb.w.y = -(0.5 * (gjk_geom->m_aabb_mn_loc.y + y));
            userData->cg2rb.w.z = -(0.5 * (gjk_geom->m_aabb_mn_loc.z + z));

            iassert(userData->m_gjk_geom_list.get_geom_count() == 0);

            userData->m_gjk_geom_list.add_geom(gjk_geom);
            //gjk_geom_list_t::add_geom(&userData->m_gjk_geom_list, gjk_geom);
            userData->m_time_since_last_event = phys_impact_silence_window->current.value + 0.0099999998;
            userData->m_time_since_last_reeval = phys_reeval_frequency->current.value + 0.0099999998;
            userData->m_flags = 0;
            dictator.w.x = dictator.w.x - userData->cg2rb.w.x;
            dictator.w.y = dictator.w.y - userData->cg2rb.w.y;
            dictator.w.z = dictator.w.z - userData->cg2rb.w.z;
            //user_rigid_body::set(body, &dictator);
            body->set(&dictator);
            body->m_userdata = userData;
            //rigid_body::set_flag(body, 0x40u, 1);
            body->set_flag(64, 1);
            userData->body = body;
            userData->refcount = 1;
            userData->id = (int)id;
            bodyId = (int)userData;
            for (j = 0; j < 16; ++j)
            {
                if (!physGlob.userRigidBodies[j])
                {
                    physGlob.userRigidBodies[j] = userData;
                    break;
                }
            }
            Sys_LeaveCriticalSection(CRITSECT_PHYSICS);
            return (PhysObjUserData *)bodyId;
        }
        else
        {
            phys_sys::destroy(body);
            Sys_LeaveCriticalSection(CRITSECT_PHYSICS);
            return 0;
        }
    }
    else
    {
        Com_PrintWarning(20, "Maximum number of physics bodies exceeded \n");
        Sys_LeaveCriticalSection(CRITSECT_PHYSICS);
        return 0;
    }
}

PhysObjUserData *__cdecl Phys_CreateUserData(int worldIndex)
{
    PhysObjUserData *userData; // [esp+2Ch] [ebp-4h]

    //userData = phys_free_list<PhysObjUserData>::add(&physGlob.objects, 1, "phys_free_list error: out of memory.");
    userData = physGlob.objects.add(1, "phys_free_list error: out of memory.");
    if ( !userData )
        return 0;
    memset(userData, 0, sizeof(PhysObjUserData));
    //phys_link_list1<PhysObjUserData>::add(&physGlob.objects_by_world[worldIndex], userData);
    physGlob.objects_by_world[worldIndex].add(userData);
    return userData;
}

PhysObjUserData * Phys_CreateBodyFromState(
                int worldIndex,
                const BodyState *state,
                gjk_geom_list_t *gjk_geom_list,
                bool do_collision_test)
{
    //phys_vec3 v8; // [esp+28h] [ebp-36Ch] BYREF // inertia
    //float v9; // [esp+44h] [ebp-350h] BYREF // volume
    //phys_vec3 v10; // [esp+48h] [ebp-34Ch] BYREF // dim
    float mass; // [esp+6Ch] [ebp-328h]
    float m_inv_mass; // [esp+70h] [ebp-324h]
    rigid_body *body2; // [esp+74h] [ebp-320h]
    float *savedPos; // [esp+90h] [ebp-304h]
    float value; // [esp+B4h] [ebp-2E0h]
    phys_vec3 gravity_dir; // [esp+B8h] [ebp-2DCh] BYREF
    phys_mat44 rb2w; // [esp+C8h] [ebp-2CCh] BYREF
    phys_mat44 rb2m; // [esp+108h] [ebp-28Ch] BYREF
    phys_vec3 com_offset; // [esp+178h] [ebp-21Ch] BYREF
    phys_vec3 center; // [esp+188h] [ebp-20Ch]
    float v47; // [esp+1A8h] [ebp-1ECh]
    float v48; // [esp+1ACh] [ebp-1E8h]
    phys_vec3 gjk_geom_list_aabb_mn_loc; // [esp+1C8h] [ebp-1CCh] BYREF
    phys_vec3 gjk_geom_list_aabb_mx_loc; // [esp+1D8h] [ebp-1BCh] BYREF
    phys_mat44 m2w; // [esp+258h] [ebp-13Ch] BYREF
    phys_vec3 a_vel; // [esp+2B8h] [ebp-DCh] BYREF
    phys_vec3 t_vel; // [esp+2D8h] [ebp-BCh] BYREF
    int minStableContactCount; // [esp+2FCh] [ebp-98h]
    float maxAVel; // [esp+300h] [ebp-94h]
    float inertiaScale; // [esp+304h] [ebp-90h]
    phys_vec3 inertia; // [esp+308h] [ebp-8Ch] BYREF
    float volume; // [esp+324h] [ebp-70h] BYREF
    phys_vec3 dim; // [esp+328h] [ebp-6Ch] BYREF
    rigid_body *body; // [esp+340h] [ebp-54h]
    int bodyId; // [esp+344h] [ebp-50h]
    PhysObjUserData *userData; // [esp+348h] [ebp-4Ch]
    PhysGlob *v100; // [esp+34Ch] [ebp-48h]
    phys_free_list<PhysObjUserData>::iterator i; // [esp+350h] [ebp-44h]
    phys_free_list<PhysObjUserData>::T_internal_base *m_next_T_internal; // [esp+354h] [ebp-40h]
    //_UNKNOWN *v115[2]; // [esp+388h] [ebp-Ch] BYREF
    //gjk_geom_list_t *gjk_geom_lista; // [esp+394h] [ebp+0h]
    //
    //v115[0] = a1;
    //v115[1] = gjk_geom_lista;
    iassert(state);

    iassert(!IS_NAN((state->position)[0]) && !IS_NAN((state->position)[1]) && !IS_NAN((state->position)[2]));
    iassert(!IS_NAN((state->velocity)[0]) && !IS_NAN((state->velocity)[1]) && !IS_NAN((state->velocity)[2]));
    iassert(!IS_NAN((state->angVelocity)[0]) && !IS_NAN((state->angVelocity)[1]) && !IS_NAN((state->angVelocity)[2]));

    iassert(!IS_NAN((state->centerOfMassOffset)[0]) && !IS_NAN((state->centerOfMassOffset)[1]) && !IS_NAN((state->centerOfMassOffset)[2]));

    Sys_EnterCriticalSection(CRITSECT_PHYSICS);

    if (state->id != -1)
    {
        m_next_T_internal = physGlob.objects.m_dummy_head.m_next_T_internal;
        for (i.m_ptr = physGlob.objects.m_dummy_head.m_next_T_internal; ; i.m_ptr = i.m_ptr->m_next_T_internal)
        {
            v100 = &physGlob;
            if (&physGlob == (PhysGlob *)i.m_ptr)
                break;
            userData = (PhysObjUserData *)&i.m_ptr[2];
            if (i.m_ptr[29].m_next_T_internal == (phys_free_list<PhysObjUserData>::T_internal_base *)state->id)
            {
                destroy_gjk_geom(gjk_geom_list);
                ++userData->refcount;
                Sys_LeaveCriticalSection(CRITSECT_PHYSICS);
                return userData;
            }
        }
    }

    body = phys_sys::create_rigid_body(1);

    if (!body)
    {
        destroy_gjk_geom(gjk_geom_list);
        Com_PrintWarning(20, "Maximum number of physics bodies exceeded\n");
        Sys_LeaveCriticalSection(CRITSECT_PHYSICS);
        return 0;
    }

    userData = Phys_CreateUserData(worldIndex);

    if (!userData)
    {
        destroy_gjk_geom(gjk_geom_list);
        phys_sys::destroy(body);
        Com_PrintWarning(20, "Maximum number of Phys User Data exceeded\n");
        Sys_LeaveCriticalSection(CRITSECT_PHYSICS);
        return 0;
    }

    dim.x = 3.0f;
    dim.y = 3.0f;
    dim.z = 3.0f;
    nuge::calc_box_inertia(&dim, &inertia, &volume);

    inertiaScale = state->mass / volume;
    inertia.x *= inertiaScale;
    inertia.y *= inertiaScale;
    inertia.z *= inertiaScale;

    maxAVel = 16.0f;
    minStableContactCount = 3;

    if (worldIndex == 2)
        minStableContactCount = 1;

    t_vel.x = state->velocity[0];
    t_vel.y = state->velocity[1];
    t_vel.z = state->velocity[2];

    a_vel.x = state->angVelocity[0];
    a_vel.y = state->angVelocity[1];
    a_vel.z = state->angVelocity[2];

    m2w.x.x = state->rotation[0][0];
    m2w.x.y = state->rotation[0][1];
    m2w.x.z = state->rotation[0][2];

    m2w.y.x = state->rotation[1][0];
    m2w.y.y = state->rotation[1][1];
    m2w.y.z = state->rotation[1][2];

    m2w.z.x = state->rotation[2][0];
    m2w.z.y = state->rotation[2][1];
    m2w.z.z = state->rotation[2][2];

    m2w.w.x = state->position[0];
    m2w.w.y = state->position[1];
    m2w.w.z = state->position[2];

    bodyId = (int)userData;

    userData->m_gjk_geom_list.m_first_geom = gjk_geom_list->m_first_geom;
    userData->m_gjk_geom_list.m_geom_count = gjk_geom_list->m_geom_count;

    //gjk_geom_list_t::comp_aabb_loc(
    userData->m_gjk_geom_list.comp_aabb_loc(&gjk_geom_list_aabb_mn_loc, &gjk_geom_list_aabb_mx_loc);

    //phys_mat44::operator=(&userData->cg2rb, &PHYS_IDENTITY_MATRIX_43);
    userData->cg2rb = PHYS_IDENTITY_MATRIX;

    //phys_mat44::operator=(&userData->cg2w, &PHYS_IDENTITY_MATRIX_43);
    userData->cg2w = PHYS_IDENTITY_MATRIX;

    center.x = 0.5 * (float)(gjk_geom_list_aabb_mn_loc.x + gjk_geom_list_aabb_mx_loc.x);
    center.y = 0.5 * (float)(gjk_geom_list_aabb_mn_loc.y + gjk_geom_list_aabb_mx_loc.y);
    center.z = 0.5 * (float)(gjk_geom_list_aabb_mn_loc.z + gjk_geom_list_aabb_mx_loc.z);
    Phys_Vec3ToNitrousVec(state->centerOfMassOffset, &com_offset);
    center.x = center.x + com_offset.x;
    center.y = center.y + com_offset.y;
    center.z = center.z + com_offset.z;

    userData->cg2rb.w.x = -(center.x);
    userData->cg2rb.w.y = -(center.y);
    userData->cg2rb.w.z = -(center.z);
    phys_full_inverse(&rb2m, &userData->cg2rb);
    phys_full_multiply_mat(&rb2w, &m2w, &rb2m);

    iassert(Abs(t_vel) < 100000.0f);
    iassert(Abs(a_vel) < 1000.0f);

    //rigid_body::set(body, state->mass, &inertia, &rb2w, &t_vel, &a_vel, minStableContactCount);
    body->set(state->mass, &inertia, &rb2w, &t_vel, &a_vel, minStableContactCount);
    body->m_userdata = userData;
    Phys_Vec3ToNitrousVec(&phys_gravity_dir->current.value, &gravity_dir);
    value = phys_gravity->current.value;

    body->m_gravity_acc_vec.x = value * gravity_dir.x;
    body->m_gravity_acc_vec.y = value * gravity_dir.y;
    body->m_gravity_acc_vec.z = value * gravity_dir.z;

    body->m_max_avel = maxAVel;
    userData->body = body;
    savedPos = userData->savedPos;
    userData->savedPos[0] = state->position[0];
    savedPos[1] = state->position[1];
    savedPos[2] = state->position[2];
    memcpy(userData->savedRot, state->rotation, sizeof(userData->savedRot));
    userData->bounce = state->bounce;
    userData->buoyancy = state->buoyancy;
    userData->friction = state->friction;
    userData->underwater = state->underwater;
    userData->mass = state->mass;
    userData->id = state->id;
    userData->refcount = 1;
    userData->timeBuoyant = -1;
    userData->timeRipple = -1;
    userData->m_time_since_last_event = phys_impact_silence_window->current.value + 0.0099999998;
    userData->m_time_since_last_reeval = phys_reeval_frequency->current.value + 0.0099999998;
    userData->m_flags = 0;

    userData->buoyancyBoxMin[0] = state->buoyancyBoxMin[0];
    userData->buoyancyBoxMin[1] = state->buoyancyBoxMin[1];
    userData->buoyancyBoxMin[2] = state->buoyancyBoxMin[2];

    userData->buoyancyBoxMax[0] = state->buoyancyBoxMax[0];
    userData->buoyancyBoxMax[1] = state->buoyancyBoxMax[1];
    userData->buoyancyBoxMax[2] = state->buoyancyBoxMax[2];

    userData->centerOfMassOffset[0] = state->centerOfMassOffset[0];
    userData->centerOfMassOffset[1] = state->centerOfMassOffset[1];
    userData->centerOfMassOffset[2] = state->centerOfMassOffset[2];

    body2 = userData->body;
    m_inv_mass = body2->m_inv_mass;
    mass = 1.0 / m_inv_mass;

    dim.x = gjk_geom_list_aabb_mx_loc.x - gjk_geom_list_aabb_mn_loc.x;
    dim.y = gjk_geom_list_aabb_mx_loc.y - gjk_geom_list_aabb_mn_loc.y;
    dim.z = gjk_geom_list_aabb_mx_loc.z - gjk_geom_list_aabb_mn_loc.z;

    iassert(Abs(dim) < 10000.0f);

    nuge::calc_box_inertia(&dim, &inertia, &volume);
    inertia.x *= (float)(mass / volume);
    inertia.y *= (float)(mass / volume);
    inertia.z *= (float)(mass / volume);

    body2->set_inertia(&inertia);
    create_broad_phase_info(userData->body);

    //if (do_collision_test)
    //    BG_EvalVehicleName();

    Sys_LeaveCriticalSection(CRITSECT_PHYSICS);

    return (PhysObjUserData *)bodyId;
}

void    phys_full_inverse(phys_mat44 *dest, const phys_mat44 *source)
{
    phys_mat44 mat; // [esp-Ch] [ebp-8Ch] BYREF
    phys_vec3 *p_w; // [esp+40h] [ebp-40h]
    float v5; // [esp+48h] [ebp-38h]
    float v6; // [esp+4Ch] [ebp-34h]
    float v7; // [esp+54h] [ebp-2Ch]
    float v8; // [esp+58h] [ebp-28h]
    float v9; // [esp+5Ch] [ebp-24h]
    const phys_vec3 *v10; // [esp+60h] [ebp-20h]
    phys_vec3 v11; // [esp+64h] [ebp-1Ch] BYREF
    //_UNKNOWN *v12[2]; // [esp+74h] [ebp-Ch] BYREF
    //int vars0; // [esp+80h] [ebp+0h]
    //
    //v12[0] = a1;
    //v12[1] = (_UNKNOWN *)vars0;
    if (dest == source)
    {
        phys_full_inverse(&mat, source);
        //phys_mat44::operator=(dest, &mat);
        *dest = mat;
    }
    else
    {
        phys_transpose(dest, source);
        v10 = phys_multiply(&v11, dest, &source->w);
        (v9) = -(v10->x);
        (v8) = -(v10->y);
        (v7) = -(v10->z);
        v5 = v8;
        v6 = v7;
        p_w = &dest->w;
        dest->w.x = v9;
        p_w->y = v5;
        p_w->z = v6;
    }
}

void __cdecl destroy_gjk_geom(gjk_geom_list_t *gjk_geom_list)
{
    gjk_base_t *next_geom; // [esp+0h] [ebp-8h]
    gjk_base_t *geom; // [esp+4h] [ebp-4h]

    if (gjk_geom_list->m_geom_count < 0
        && _tlAssert(
            "c:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_colgeom.h",
            1035,
            "m_geom_count >= 0",
            ""))
    {
        __debugbreak();
    }

    for (geom = gjk_geom_list->m_first_geom; geom; geom = next_geom)
    {
        next_geom = geom->m_next_geom;
        destroy_gjk_geom(geom);
    }

    gjk_geom_list->m_first_geom = 0;
    gjk_geom_list->m_geom_count = 0;
}

PhysObjUserData *__cdecl Phys_ObjCreate(
                int worldIndex,
                const float *position,
                const float *quat,
                const float *velocity,
                const PhysPreset *physPreset,
                gjk_geom_list_t *gjk_geom_list,
                bool do_collision_test,
                int id)
{
    float axis[3][3]; // [esp+24h] [ebp-24h] BYREF

    nanassertvec3(position);
    nanassertvec3(quat);
    nanassertvec3(velocity);

    iassert(physInited);
    iassert(physPreset);
    iassert(gjk_geom_list);

    QuatToAxis(quat, axis);

    return Phys_ObjCreateAxis(worldIndex, position, axis, velocity, physPreset, gjk_geom_list, id, do_collision_test);
}

PhysObjUserData *__cdecl Phys_ObjCreateAxis(
                int worldIndex,
                const float *position,
                const float (*axis)[3],
                const float *velocity,
                const PhysPreset *physPreset,
                gjk_geom_list_t *gjk_geom_list,
                int id,
                bool do_collision_test)
{
    BodyState state; // [esp+24h] [ebp-90h] BYREF

    nanassertvec3(position);
    nanassertvec3(velocity);
    iassert(physInited);
    iassert(physPreset);

    AxisCopy(axis, state.rotation);

    Vec3Copy(position, state.position);
    Vec3Copy(velocity, state.velocity);

    memset(state.angVelocity, 0, sizeof(state.angVelocity));
    state.mass = physPreset->mass;
    state.bounce = physPreset->bounce;
    state.friction = physPreset->friction;
    state.timeLastAsleep = physGlob.timeLastUpdate;
    state.underwater = 0;
    state.id = id;

    if ( physPreset->canFloat )
        state.buoyancy = phys_gravity->current.value * -1.1;
    else
        state.buoyancy = phys_gravity->current.value * -0.89999998;

    Vec3Copy(physPreset->buoyancyBoxMin, state.buoyancyBoxMin);
    Vec3Copy(physPreset->buoyancyBoxMax, state.buoyancyBoxMax);
    Vec3Copy(physPreset->centerOfMassOffset, state.centerOfMassOffset);

    return Phys_CreateBodyFromState(worldIndex, &state, gjk_geom_list, do_collision_test);
}

void    Phys_ObjSetPosition(int id, float *newPosition)
{
    phys_mat44 *v3; // [esp-B8h] [ebp-DCh]
    phys_vec3 newPos; // [esp-38h] [ebp-5Ch] BYREF
    PhysObjUserData *userData; // [esp-Ch] [ebp-30h]
    phys_vec3 pos; // [esp-8h] [ebp-2Ch] OVERLAPPED BYREF
    int newPos_8; // [esp+Ch] [ebp-18h]
    int newPos_12; // [esp+10h] [ebp-14h]
    int v16; // [esp+14h] [ebp-10h]
    //_UNKNOWN *v17; // [esp+18h] [ebp-Ch]
    //int ida; // [esp+1Ch] [ebp-8h]
    //int vars0; // [esp+24h] [ebp+0h]
    //
    //v17 = a1;
    //ida = vars0;

    iassert(!IS_NAN((newPosition)[0]) && !IS_NAN((newPosition)[1]) && !IS_NAN((newPosition)[2]));
    
    Phys_Vec3ToNitrousVec(newPosition, &pos);
    userData = Phys_GetUserData(id);

    newPos.x = pos.x + userData->cg2rb.w.x;
    newPos.y = pos.y + userData->cg2rb.w.y;
    newPos.z = pos.z + userData->cg2rb.w.z;

    //phys_vec3::operator=(&UserData->body->m_mat.w, &v7);
    userData->body->m_mat.w = newPos;

    userData->body->m_last_position.x = userData->body->m_mat.w.x;
    userData->body->m_last_position.y = userData->body->m_mat.w.y;
    userData->body->m_last_position.z = userData->body->m_mat.w.z;

    iassert(!IS_NAN((userData->body->m_last_position)[0]) && !IS_NAN((userData->body->m_last_position)[1]) && !IS_NAN((userData->body->m_last_position)[2]));

    userData->savedPos[0] = *newPosition;
    userData->savedPos[1] = newPosition[1];
    userData->savedPos[2] = newPosition[2];

    if (   (fabs(userData->body->m_last_position.x)) > 100000.0
        || (fabs(userData->body->m_last_position.y)) > 100000.0
        || (fabs(userData->body->m_last_position.z)) > 100000.0)
    {
        phys_exec_debug_callback(userData->body);
    }
    if (   (fabs(userData->body->m_mat.w.x)) > 100000.0
        || (fabs(userData->body->m_mat.w.y)) > 100000.0
        //|| (v3 = &userData->body->m_mat, v3->w.z != *phys_vec3::operator[]<int>(&userData->body->m_mat.w, 2u))
        || (v3 = &userData->body->m_mat, v3->w.z != userData->body->m_mat.w.operator[](2u))
        //|| COERCE_FLOAT(*(_DWORD *)phys_vec3::operator[]<int>(&userData->body->m_mat.w, 2u) & _mask__AbsFloat_) > 100000.0)
        || (fabs(userData->body->m_mat.w.operator[](2)) > 100000.0))
    {
        phys_exec_debug_callback(userData->body);
    }
}

void __cdecl Phys_ObjSetOrientation(int id, const float *newPosition, const float *newOrientation)
{
    float *v3; // esi
    phys_mat44 *p_m_mat; // [esp+Ch] [ebp-CCh]
    phys_vec3 *p_w; // [esp+7Ch] [ebp-5Ch]
    rigid_body *body; // [esp+80h] [ebp-58h]
    float *savedPos; // [esp+84h] [ebp-54h]
    float newRotation[3][3]; // [esp+88h] [ebp-50h] BYREF
    float newCenterOfGravity[3]; // [esp+ACh] [ebp-2Ch] BYREF
    float rotatedCGOffset[3]; // [esp+B8h] [ebp-20h] BYREF
    PhysObjUserData *userData; // [esp+C4h] [ebp-14h]
    phys_mat44 *rb2w; // [esp+C8h] [ebp-10h]
    float trans[3]; // [esp+CCh] [ebp-Ch] BYREF

    userData = Phys_GetUserData(id);
    rb2w = &userData->body->m_mat;
    Phys_NitrousVecToVec3(&userData->cg2rb.w, trans);
    QuatToAxis(newOrientation, newRotation);
    AxisTransformVec3(newRotation, trans, rotatedCGOffset);
    newCenterOfGravity[0] = *newPosition - rotatedCGOffset[0];
    newCenterOfGravity[1] = newPosition[1] - rotatedCGOffset[1];
    newCenterOfGravity[2] = newPosition[2] - rotatedCGOffset[2];
    savedPos = userData->savedPos;
    userData->savedPos[0] = newCenterOfGravity[0];
    savedPos[1] = newCenterOfGravity[1];
    savedPos[2] = newCenterOfGravity[2];
    AxisCopy(newRotation, userData->savedRot);
    Phys_Vec3ToNitrousVec(newCenterOfGravity, &rb2w->w);
    Phys_AxisToNitrousMat(newRotation, rb2w);
    p_w = &rb2w->w;
    body = userData->body;
    body->m_last_position.x = rb2w->w.x;
    body->m_last_position.y = p_w->y;
    body->m_last_position.z = p_w->z;

    iassert(!IS_NAN((userData->body->m_last_position)[0]) && !IS_NAN((userData->body->m_last_position)[1]) && !IS_NAN((userData->body->m_last_position)[2]));
    
    if ( fabs(userData->body->m_last_position.x) > 100000.0
        || fabs(userData->body->m_last_position.y) > 100000.0
        || fabs(userData->body->m_last_position.z) > 100000.0 )
    {
        phys_exec_debug_callback(userData->body);
    }
    if ( fabs(userData->body->m_mat.w.x) > 100000.0
        || fabs(userData->body->m_mat.w.y) > 100000.0
        || (p_m_mat = &userData->body->m_mat,
                //v3 = phys_vec3::operator[]<int>(&userData->body->m_mat.w, 2u),
                v3 = &userData->body->m_mat.w.operator[](2u),
                //*v3 != *phys_vec3::operator[]<int>(&p_m_mat->w, 2u))
                *v3 != p_m_mat->w.operator[](2))
        //|| fabs(*(unsigned int *)phys_vec3::operator[]<int>(&userData->body->m_mat.w, 2u)) > 100000.0 )
        || fabs(userData->body->m_mat.w.operator[](2)) > 100000.0 )
    {
        phys_exec_debug_callback(userData->body);
    }
}

void    Phys_ObjSetAngularVelocity(int id, float *angularVel)
{
    rigid_body *body = Phys_GetUserData(id)->body;

    phys_vec3 angVel;
    angVel.x = angularVel[2];
    angVel.y = angularVel[0];
    angVel.z = angularVel[1];

    body->dangerous_set_a_vel(&angVel);
}

void __thiscall rigid_body::dangerous_set_a_vel(const phys_vec3 *a_vel)
{
    this->m_a_vel.x = a_vel->x;
    this->m_a_vel.y = a_vel->y;
    this->m_a_vel.z = a_vel->z;
    if ( fabs(this->m_a_vel.x) > 100000.0
        || fabs(this->m_a_vel.y) > 100000.0
        || fabs(this->m_a_vel.z) > 100000.0 )
    {
        phys_exec_debug_callback(this);
    }
}

void Phys_ObjSetVelocity(int id, float *velocity)
{
    rigid_body *body = Phys_GetUserData(id)->body;

    phys_vec3 vel;
    vel.x = velocity[0];
    vel.y = velocity[1];
    vel.z = velocity[2];

    body->dangerous_set_t_vel(&vel);
}

void __thiscall rigid_body::dangerous_set_t_vel(const phys_vec3 *t_vel)
{
    this->m_t_vel.x = t_vel->x;
    this->m_t_vel.y = t_vel->y;
    this->m_t_vel.z = t_vel->z;
    if ( fabs(this->m_t_vel.x) > 100000.0
        || fabs(this->m_t_vel.y) > 100000.0
        || fabs(this->m_t_vel.z) > 100000.0 )
    {
        phys_exec_debug_callback(this);
    }
}

void    Phys_ObjSetAngularVelocityRaw(int id, float *angularVel)
{
    phys_vec3 v3; // [esp-20h] [ebp-2Ch] BYREF
    float v4; // [esp-10h] [ebp-1Ch]
    float v5; // [esp-Ch] [ebp-18h]
    float v6; // [esp-8h] [ebp-14h]
    rigid_body *body; // [esp-4h] [ebp-10h]
    //int v8; // [esp+0h] [ebp-Ch]
    //void *v9; // [esp+4h] [ebp-8h]
    //void *retaddr; // [esp+Ch] [ebp+0h]
    //
    //v8 = a1;
    //v9 = retaddr;
    body = Phys_GetUserData(id)->body;
    v6 = *angularVel;
    v5 = angularVel[1];
    v4 = angularVel[2];
    v3.x = v6;
    v3.y = v5;
    v3.z = v4;
    ///rigid_body::dangerous_set_a_vel(body, &v3);
    body->dangerous_set_a_vel(&v3);
}

void    Phys_ObjGetPosition(int id, float *outPosition, float (*outRotation)[3])
{
    phys_mat44 m2w; // [esp-Ch] [ebp-5Ch] BYREF
    const phys_mat44 *rb2w; // [esp+3Ch] [ebp-14h]
    PhysObjUserData *UserData; // [esp+40h] [ebp-10h]
    //int v7; // [esp+44h] [ebp-Ch] BYREF
    //const phys_mat44 *rb2w; // [esp+48h] [ebp-8h]
    const phys_mat44 *retaddr; // [esp+50h] [ebp+0h]

    //v7 = a1;
    //rb2w = retaddr;
    UserData = Phys_GetUserData(id);
    rb2w = &UserData->body->m_mat;
    phys_full_multiply_mat(&m2w, rb2w, &UserData->cg2rb);
    Phys_NitrousVecToVec3(&m2w.w, outPosition);
    Phys_NitrousMat44ToVec33(&m2w, outRotation);
}

void __cdecl Phys_ObjGetVelocities(int id, float *tvel, float *avel)
{
    rigid_body *body; // [esp+0h] [ebp-4h]

    if ( !physInited
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_main.cpp", 1047, 0, "%s", "physInited") )
    {
        __debugbreak();
    }
    body = Phys_GetUserData(id)->body;
    Phys_NitrousVecToVec3(&body->m_t_vel, tvel);
    Phys_NitrousVecToVec3(&body->m_a_vel, avel);
}

void __cdecl Phys_ObjGetCenterOfMass(int id, float *outPosition)
{
    PhysObjUserData *UserData; // eax

    if ( !physInited
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_main.cpp", 1055, 0, "%s", "physInited") )
    {
        __debugbreak();
    }
    UserData = Phys_GetUserData(id);
    Phys_NitrousVecToVec3(&UserData->body->m_mat.w, outPosition);
}

void __cdecl Phys_ObjAddCollFlags(int physObjId, int collFlags)
{
    if (!physObjId
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_main.cpp",
            1063,
            0,
            "%s",
            "physObjId != PHYS_OBJ_ID_NULL"))
    {
        __debugbreak();
    }

    PhysObjUserData *userData = Phys_GetUserData(physObjId);

    if (userData->m_bpb->is_bpi())
    {
        broad_phase_info *bpi = userData->m_bpb->get_bpi();
        bpi->m_env_collision_flags |= collFlags;
    }
    else
    {
        for (broad_phase_base *i = userData->m_bpb->get_bpg()->m_list_bpi_head; i; i = i->m_list_bpb_next)
        {
            i->m_env_collision_flags |= collFlags;
        }
    }
}

void __cdecl Phys_ObjRemoveCollFlags(int physObjId, int collFlags)
{
    if (!physObjId
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_main.cpp",
            1087,
            0,
            "%s",
            "physObjId != PHYS_OBJ_ID_NULL"))
    {
        __debugbreak();
    }

    PhysObjUserData *userData = Phys_GetUserData(physObjId);

    if (userData->m_bpb->is_bpi())
    {
        broad_phase_info *bpi = userData->m_bpb->get_bpi();
        bpi->m_env_collision_flags &= ~collFlags;
    }
    else
    {
        for (broad_phase_base *i = userData->m_bpb->get_bpg()->m_list_bpi_head; i; i = i->m_list_bpb_next)
        {
            i->m_env_collision_flags &= ~collFlags;
        }
    }
}

void __cdecl fixup_wheel_constraints(rigid_body *rb)
{
    phys_free_list<rigid_body_constraint_wheel>::T_internal_base *wci; // [esp+Ch] [ebp-8h]
    phys_free_list<rigid_body_constraint_wheel> *wci_end; // [esp+10h] [ebp-4h]

    wci = g_physics_system->m_list_rbc_wheel.m_dummy_head.m_next_T_internal;
    wci_end = &g_physics_system->m_list_rbc_wheel;
    while ( wci_end != (phys_free_list<rigid_body_constraint_wheel> *)wci )
    {
        if ( wci != (phys_free_list<rigid_body_constraint_wheel>::T_internal_base *)-16
            && (rigid_body *)wci[2].m_next_T_internal == rb )
        {
            //rigid_body_constraint_wheel::set_no_collision((rigid_body_constraint_wheel *)&wci[2]);
            ((rigid_body_constraint_wheel *)&wci[2])->set_no_collision();
        }
        wci = wci->m_next_T_internal;
    }
}

void __cdecl Phys_ObjDestroy(int worldIndex, int id)
{
    int i; // [esp+Ch] [ebp-8h]
    PhysObjUserData *userData; // [esp+10h] [ebp-4h]

    Sys_EnterCriticalSection(CRITSECT_PHYSICS);
    if ( !physInited
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_main.cpp", 1126, 0, "%s", "physInited") )
    {
        __debugbreak();
    }
    userData = Phys_GetUserData(id);
    if ( --userData->refcount <= 0 )
    {
        fixup_wheel_constraints(userData->body);
        destroy_broad_phase_info(userData->body);
        if ( (userData->body->m_flags & 0x20) != 0 )
        {
            for ( i = 0; i < 16; ++i )
            {
                if ( physGlob.userRigidBodies[i] == userData )
                    physGlob.userRigidBodies[i] = 0;
            }
            phys_sys::destroy((user_rigid_body *const)userData->body);
        }
        else
        {
            phys_sys::destroy(userData->body);
        }
        userData->body = 0;
        destroy_gjk_geom(&userData->m_gjk_geom_list);
        Phys_DestroyUserData(worldIndex, userData);
        Sys_LeaveCriticalSection(CRITSECT_PHYSICS);
    }
    else
    {
        Sys_LeaveCriticalSection(CRITSECT_PHYSICS);
    }
}

void __cdecl Phys_DestroyUserData(int worldIndex, PhysObjUserData *userData)
{
    //phys_link_list1<PhysObjUserData>::remove(&physGlob.objects_by_world[worldIndex], userData);
    physGlob.objects_by_world[worldIndex].remove(userData);
    //phys_free_list<PhysObjUserData>::remove(&physGlob.objects, userData);
    physGlob.objects.remove(userData);
}

void    Phys_AddCacheImpulses()
{
    phys_vec3 v1; // [esp+14h] [ebp-5Ch] BYREF
    float v2; // [esp+28h] [ebp-48h]
    float v3; // [esp+2Ch] [ebp-44h]
    float v4; // [esp+30h] [ebp-40h]
    phys_vec3 force; // [esp+34h] [ebp-3Ch] BYREF
    phys_vec3 pos; // [esp+44h] [ebp-2Ch] BYREF
    rigid_body *body; // [esp+54h] [ebp-1Ch]
    int id; // [esp+58h] [ebp-18h]
    PhysImpulse *impulse; // [esp+5Ch] [ebp-14h]
    int i; // [esp+60h] [ebp-10h]
    //_UNKNOWN *v11; // [esp+64h] [ebp-Ch]
    //int v12; // [esp+68h] [ebp-8h]
    //int vars0; // [esp+70h] [ebp+0h]
    //
    //v11 = a1;
    //v12 = vars0;

    PROF_SCOPED("Phys_AddCacheImpulses");

    for (i = 0; i < gImpulseCacheNum; ++i)
    {
        impulse = &gImpulseCache[i];
        id = impulse->id;
        body = Phys_GetUserData(id)->body;
        if (body)
        {
            Phys_Vec3ToNitrousVec(impulse->hitp, &pos);
            Phys_Vec3ToNitrousVec(impulse->hitd, &force);
            if (Vec3Length(&force.x) >= 1000000.0
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_main.cpp",
                    1206,
                    0,
                    "%s",
                    "Abs(force) < 1000000.0f"))
            {
                __debugbreak();
            }
            v4 = 0.001 * force.x;
            v3 = 0.001 * force.y;
            v2 = 0.001 * force.z;
            v1.x = 0.001 * force.x;
            v1.y = 0.001 * force.y;
            v1.z = 0.001 * force.z;
            //rigid_body::add_force(body, &v1, &pos, 1.0);
            body->add_force(&v1, &pos, 1.0);
        }
    }

    gImpulseCacheNum = 0;
}

void __cdecl Phys_ObjAddCustomForce(
                int physObjId,
                const float *hitPos,
                const float *hitDir,
                int mod,
                PhysPreset *physPreset)
{
    float v5; // [esp+24h] [ebp-3Ch]
    float v6; // [esp+28h] [ebp-38h]
    float offset_4; // [esp+30h] [ebp-30h]
    float offset_8; // [esp+34h] [ebp-2Ch]
    float torque[3]; // [esp+38h] [ebp-28h] BYREF
    float modHitPos[3]; // [esp+44h] [ebp-1Ch] BYREF
    float scale; // [esp+50h] [ebp-10h]
    float modHitDir[3]; // [esp+54h] [ebp-Ch] BYREF
    int savedregs; // [esp+60h] [ebp+0h] BYREF

    if ( physObjId )
    {
        switch ( mod )
        {
            case 0:
                modHitDir[0] = *hitDir;
                modHitDir[1] = hitDir[1];
                modHitDir[2] = hitDir[2];
                scale = Vec3Normalize(modHitDir);
                break;
            case 4:
            case 6:
            case 15:
                scale = dynEnt_explodeForce->current.value;
                scale = scale * physPreset->explosiveForceScale;
                Phys_ObjGetCenterOfMass(physObjId, modHitPos);
                offset_4 = hitPos[1] - modHitPos[1];
                offset_8 = hitPos[2] - modHitPos[2];
                modHitPos[0] = (float)(spin_scale_0 * (float)(*hitPos - modHitPos[0])) + modHitPos[0];
                modHitPos[1] = (float)(spin_scale_0 * offset_4) + modHitPos[1];
                modHitPos[2] = (float)(spin_scale_0 * offset_8) + modHitPos[2];
                modHitDir[0] = scale * *hitDir;
                modHitDir[1] = scale * hitDir[1];
                modHitDir[2] = scale * hitDir[2];
                break;
            case 17:
                scale = 1.0f;
                Phys_ObjGetCenterOfMass(physObjId, modHitPos);
                modHitDir[0] = *hitDir;
                modHitDir[1] = hitDir[1];
                modHitDir[2] = hitDir[2];
                torque[0] = CG_flrand((-(range_1)), range_1);
                torque[1] = CG_flrand((-(range_1)), range_1);
                torque[2] = CG_flrand((-(range_1)), range_1);
                Phys_ObjAddTorque(physObjId, torque);
                break;
            default:
                scale = dynEnt_bulletForce->current.value;
                scale = scale * physPreset->bulletForceScale;
                Phys_ObjGetCenterOfMass(physObjId, modHitPos);
                v5 = hitPos[1] - modHitPos[1];
                v6 = hitPos[2] - modHitPos[2];
                modHitPos[0] = (float)(spin_scale * (float)(*hitPos - modHitPos[0])) + modHitPos[0];
                modHitPos[1] = (float)(spin_scale * v5) + modHitPos[1];
                modHitPos[2] = (float)(spin_scale * v6) + modHitPos[2];
                modHitDir[0] = scale * *hitDir;
                modHitDir[1] = scale * hitDir[1];
                modHitDir[2] = scale * hitDir[2];
                break;
        }
        if ( mod )
            Phys_ObjAddForce(physObjId, modHitPos, modHitDir, 0);
        else
            Phys_ObjBulletImpact(physObjId, hitPos, modHitDir, scale, physPreset->bulletForceScale);
    }
}

void    Phys_ObjAddForce(int id, float *worldPos, float *impulse, bool relative)
{
    phys_vec3 v7; // [esp+4h] [ebp-BCh] BYREF
    phys_vec3 v11; // [esp+24h] [ebp-9Ch] BYREF
    phys_vec3 v15; // [esp+44h] [ebp-7Ch] BYREF
    phys_vec3 v16; // [esp+54h] [ebp-6Ch] BYREF
    phys_mat44 *p_m_mat; // [esp+70h] [ebp-50h]
    phys_vec3 pos; // [esp+74h] [ebp-4Ch] BYREF
    phys_vec3 force; // [esp+94h] [ebp-2Ch] BYREF
    rigid_body *body; // [esp+B0h] [ebp-10h]

    if (id && id != -1)
    {
        body = Phys_GetUserData(id)->body;

        force.x = impulse[0];
        force.y = impulse[1];
        force.z = impulse[2];

        pos.x = worldPos[0];
        pos.y = worldPos[1];
        pos.z = worldPos[2];

        if (relative)
        {
            p_m_mat = &body->m_mat;
            phys_multiply(&v16, &body->m_mat, &pos);
            //operator+(&v15, v5, p_w);
            v15 = v16 + body->m_mat.w;
            pos.x = v15.x;
            pos.y = v15.y;
            pos.z = v15.z;
        }

        iassert(Abs(force) < 1000000.0f);

        if ((float)((float)((float)(pos.x * pos.x) + (float)(pos.y * pos.y)) + (float)(pos.z * pos.z)) == 0.0)
        {
            v11.x = 0.001 * force.x;
            v11.y = 0.001 * force.y;
            v11.z = 0.001 * force.z;
            body->add_force(&v11);
            //rigid_body::add_force(body, &v11);
        }
        else
        {
            v7.x = 0.001 * force.x;
            v7.y = 0.001 * force.y;
            v7.z = 0.001 * force.z;
            body->add_force(&v7, &pos, 1.0f);
            //rigid_body::add_force(body, &v7, &pos, 1.0);
        }
    }
}

void    Phys_ObjAddTorque(int id, float *torque)
{
    phys_vec3 torq; // [esp-2Ch] [ebp-4Ch] BYREF
    phys_vec3 _torque; // [esp-Ch] [ebp-2Ch] BYREF
    rigid_body *body; // [esp+10h] [ebp-10h]

    body = Phys_GetUserData(id)->body;
    Phys_Vec3ToNitrousVec(torque, &_torque);

    iassert(Abs(_torque) < 1000000.0f);

    torq.x = 0.001 * _torque.x;
    torq.y = 0.001 * _torque.y;
    torq.z = 0.001 * _torque.z;

    //rigid_body::add_torque(body, &v3);
    body->add_torque(&torq);
}

void __thiscall rigid_body::add_torque(const phys_vec3 *torque)
{
    this->m_torque_sum.x = this->m_torque_sum.x + torque->x;
    this->m_torque_sum.y = this->m_torque_sum.y + torque->y;
    this->m_torque_sum.z = this->m_torque_sum.z + torque->z;
    if ( fabs(this->m_torque_sum.x) > 100000.0
        || fabs(this->m_torque_sum.y) > 100000.0
        || fabs(this->m_torque_sum.z) > 100000.0 )
    {
        phys_exec_debug_callback(this);
    }
}

void __cdecl Phys_ObjBulletImpact(
                int id,
                const float *worldPosRaw,
                const float *bulletDirRaw,
                float bulletSpeed,
                float scale)
{
    float force[3]; // [esp+8h] [ebp-34h] BYREF
    rigid_body *body; // [esp+14h] [ebp-28h]
    float centerOfMass[3]; // [esp+18h] [ebp-24h] BYREF
    float bulletDir[3]; // [esp+24h] [ebp-18h] BYREF
    float worldPos[3]; // [esp+30h] [ebp-Ch] BYREF
    int savedregs; // [esp+3Ch] [ebp+0h] BYREF

    body = Phys_GetUserData(id)->body;
    Phys_NitrousVecToVec3(&body->m_mat.w, centerOfMass);
    bulletDir[0] = *bulletDirRaw;
    bulletDir[1] = bulletDirRaw[1];
    bulletDir[2] = bulletDirRaw[2];
    worldPos[0] = *worldPosRaw;
    worldPos[1] = worldPosRaw[1];
    worldPos[2] = worldPosRaw[2];
    Phys_TweakBulletImpact(worldPos, bulletDir, centerOfMass);
    force[0] = (float)(scale * bulletSpeed) * bulletDir[0];
    force[1] = (float)(scale * bulletSpeed) * bulletDir[1];
    force[2] = (float)(scale * bulletSpeed) * bulletDir[2];
    Phys_ObjAddForce(id, worldPos, force, 0);
}

void __cdecl Phys_TweakBulletImpact(float *worldPos, float *bulletDir, const float *centerOfMass)
{
    float value; // [esp+0h] [ebp-1Ch]
    float offset_4; // [esp+14h] [ebp-8h]
    float offset_8; // [esp+18h] [ebp-4h]

    if ( !phys_bulletUpBias
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_main.cpp",
                    1317,
                    0,
                    "%s",
                    "phys_bulletUpBias") )
    {
        __debugbreak();
    }
    if ( !phys_bulletSpinScale
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_main.cpp",
                    1318,
                    0,
                    "%s",
                    "phys_bulletSpinScale") )
    {
        __debugbreak();
    }
    bulletDir[2] = bulletDir[2] + phys_bulletUpBias->current.value;
    Vec3Normalize(bulletDir);
    value = phys_bulletSpinScale->current.value;
    offset_4 = value * (float)(worldPos[1] - centerOfMass[1]);
    offset_8 = value * (float)(worldPos[2] - centerOfMass[2]);
    *worldPos = *worldPos + (float)(value * (float)(*worldPos - *centerOfMass));
    worldPos[1] = worldPos[1] + offset_4;
    worldPos[2] = worldPos[2] + offset_8;
}

int __cdecl Phys_ObjGetSnapshot(int id, float *outPos, float (*outMat)[3])
{
    PhysObjUserData *userData; // [esp+Ch] [ebp-4h]

    userData = Phys_GetUserData(id);
    *outPos = userData->savedPos[0];
    outPos[1] = userData->savedPos[1];
    outPos[2] = userData->savedPos[2];
    memcpy(outMat, userData->savedRot, 0x24u);
    return physGlob.timeLastSnapshot;
}

float s_lineHeight = 8.0;
void __cdecl Phys_RenderBulletMeshInfo(
                float *pos,
                const XModel *model,
                const char *type,
                int numTriangles,
                float avgArea)
{
    bool HasCollmap; // eax
    char lineStr[260]; // [esp+0h] [ebp-108h] BYREF

    XModelRenderString(pos, (char *)model->name);
    pos[2] = pos[2] - s_lineHeight;
    if ( type )
    {
        _snprintf(lineStr, 0xFFu, "    %s, tris(%d), avgarea(%d)", type, numTriangles, (int)(float)(avgArea + 0.5));
        lineStr[255] = 0;
        XModelRenderString(pos, lineStr);
        pos[2] = pos[2] - s_lineHeight;
        _snprintf(lineStr, 0xFFu, "lods(%d) collod(%d), bones(%d)", model->numLods, model->collLod, model->numBones);
        lineStr[255] = 0;
        XModelRenderString(pos, lineStr);
        pos[2] = pos[2] - s_lineHeight;
        HasCollmap = XModelHasCollmap(model);
        _snprintf(lineStr, 0xFFu, "collmaps(%d)", HasCollmap);
        lineStr[255] = 0;
        XModelRenderString(pos, lineStr);
    }
    else
    {
        XModelRenderString(pos, (char*)"    No bullet mesh");
    }
    pos[2] = pos[2] - s_lineHeight;
}

float s_stringPosScale = 240.0f;
void __cdecl Phys_ComputeStringPosition(const float *start, const float *end, float *position)
{
    float v3; // [esp+0h] [ebp-10h]

    *position = *end - *start;
    position[1] = end[1] - start[1];
    position[2] = end[2] - start[2];
    Vec3Normalize(position);
    v3 = s_stringPosScale;
    *position = s_stringPosScale * *position;
    position[1] = v3 * position[1];
    position[2] = v3 * position[2];
    *position = *position + *start;
    position[1] = position[1] + start[1];
    position[2] = position[2] + start[2];
}

void __cdecl Phys_FindAndRenderBulletMesh(const float *start, const float *end)
{
    int baseBoneIndex; // [esp+3Ch] [ebp-154h]
    XModel *model; // [esp+40h] [ebp-150h]
    const centity_s *cent; // [esp+44h] [ebp-14Ch]
    DObjAnimMat *boneMtxList; // [esp+48h] [ebp-148h]
    int partBits[5]; // [esp+4Ch] [ebp-144h] BYREF
    float *origin; // [esp+60h] [ebp-130h]
    DObj *dobj; // [esp+64h] [ebp-12Ch]
    const float (*modelOrigin)[3]; // [esp+68h] [ebp-128h]
    const DynEntityPose *dynEntPose; // [esp+6Ch] [ebp-124h]
    pointtrace_t clip; // [esp+70h] [ebp-120h] BYREF
    float localToWorld[3][3]; // [esp+B4h] [ebp-DCh] BYREF
    trace_t resultsDyn; // [esp+D8h] [ebp-B8h] BYREF
    float viewDirNormalized[3]; // [esp+110h] [ebp-80h] BYREF
    DynEntityDrawType drawType; // [esp+11Ch] [ebp-74h] BYREF
    bool bHitDynEnt; // [esp+123h] [ebp-6Dh]
    const char *dynEntTypeStr; // [esp+124h] [ebp-6Ch]
    int entityHitId; // [esp+128h] [ebp-68h]
    int localClientNum; // [esp+12Ch] [ebp-64h]
    unsigned __int16 dynEntityHitId; // [esp+130h] [ebp-60h]
    const DynEntityDef *dynEntDef; // [esp+134h] [ebp-5Ch]
    int numTriangles; // [esp+138h] [ebp-58h]
    trace_t results; // [esp+13Ch] [ebp-54h] BYREF
    float stringPos[3]; // [esp+178h] [ebp-18h] BYREF
    int modelCount; // [esp+184h] [ebp-Ch]
    int modelIndex; // [esp+188h] [ebp-8h]
    float avgArea; // [esp+18Ch] [ebp-4h]

    avgArea = 0.0f; // Is this a bug? I dont see this initialized?

    results.staticModel = 0;
    bHitDynEnt = 0;
    //TraceExtents::TraceExtents(&clip.extents);
    memset(&resultsDyn, 0, sizeof(resultsDyn));
    resultsDyn.fraction = 1.0f;
    *(_QWORD *)clip.extents.start.vec.v = *(_QWORD *)start;
    clip.extents.start.vec.v[2] = start[2];
    *(_QWORD *)clip.extents.end.vec.v = *(_QWORD *)end;
    clip.extents.end.vec.v[2] = end[2];
    CM_CalcTraceExtents(&clip.extents);
    clip.ignoreEntParams = 0;
    clip.contentmask = 0x802010 + 3;
    clip.bLocational = 1;
    clip.priorityMap = 0;
    DynEntCl_PointTrace(&clip, &resultsDyn);
    if ( resultsDyn.hitType == TRACE_HITTYPE_DYNENT_MODEL )
    {
        dynEntityHitId = Trace_GetDynEntHitId(&resultsDyn, &drawType);
        dynEntTypeStr = "dynent";
        bHitDynEnt = 1;
    }
    else
    {
        dynEntityHitId = 0;
        drawType = DYNENT_DRAW_MODEL;
        dynEntTypeStr = "";
    }
    if ( bHitDynEnt )
    {
        dynEntDef = DynEnt_GetEntityDef(dynEntityHitId, drawType);
        dynEntPose = DynEnt_GetClientPose(dynEntityHitId, drawType);
        modelOrigin = (const float (*)[3])dynEntPose->pose.origin;
        viewDirNormalized[0] = dynEntPose->pose.origin[0] - *start;
        viewDirNormalized[1] = dynEntPose->pose.origin[1] - start[1];
        viewDirNormalized[2] = dynEntPose->pose.origin[2] - start[2];
        Vec3Normalize(viewDirNormalized);
        Phys_ComputeStringPosition(start, dynEntPose->pose.origin, stringPos);
        UnitQuatToAxis(dynEntPose->pose.quat, localToWorld);
        numTriangles = RETURN_ZERO32();
        Phys_RenderBulletMeshInfo(stringPos, dynEntDef->xModel, dynEntTypeStr, numTriangles, avgArea);
    }
    else
    {
        CG_LocationalTrace(&results, start, end, 0, 0x280E833, 0, 0);
        if ( results.hitType == TRACE_HITTYPE_ENTITY )
        {
            localClientNum = cgCollWorldLocalClientNum;
            entityHitId = Trace_GetEntityHitId(&results);
            dobj = Com_GetClientDObj(entityHitId, localClientNum);
            if ( dobj )
            {
                cent = CG_GetEntity(localClientNum, entityHitId);
                AnglesToAxis(cent->pose.angles, localToWorld);
                viewDirNormalized[0] = cent->pose.origin[0] - *start;
                viewDirNormalized[1] = cent->pose.origin[1] - start[1];
                viewDirNormalized[2] = cent->pose.origin[2] - start[2];
                Vec3Normalize(viewDirNormalized);
                Phys_ComputeStringPosition(start, cent->pose.origin, stringPos);
                modelCount = DObjGetNumModels(dobj);
                DObjGeomTracelinePartBits(dobj, clip.contentmask, partBits);
                CG_DObjCalcPose(&cent->pose, dobj, partBits);
                boneMtxList = DObjGetRotTransArray(dobj);
                baseBoneIndex = 0;
                modelIndex = 0;
                while ( modelIndex < modelCount )
                {
                    model = DObjGetModel(dobj, modelIndex);
                    if ( model->collLod < 0 )
                    {
                        Phys_RenderBulletMeshInfo(stringPos, model, 0, 0, 0.0);
                    }
                    else
                    {
                        numTriangles = RETURN_ZERO32();
                        Phys_RenderBulletMeshInfo(stringPos, model, "dobj", numTriangles, avgArea);
                    }
                    ++modelIndex;
                    boneMtxList += model->numBones;
                    baseBoneIndex += model->numBones;
                }
            }
            else if ( results.staticModel )
            {
                origin = results.staticModel->origin;
                viewDirNormalized[0] = results.staticModel->origin[0] - *start;
                viewDirNormalized[1] = results.staticModel->origin[1] - start[1];
                viewDirNormalized[2] = results.staticModel->origin[2] - start[2];
                Vec3Normalize(viewDirNormalized);
                Phys_ComputeStringPosition(start, origin, stringPos);
                MatrixInverse(results.staticModel->invScaledAxis, localToWorld);
                numTriangles = RETURN_ZERO32();
                Phys_RenderBulletMeshInfo(stringPos, results.staticModel->xmodel, "static", numTriangles, avgArea);
            }
        }
    }
}

float query_radius = 2000.0;
void    Phys_FindAndRenderEntityBrushes(const float *pos, int contentmask)
{
    void *v3; // esp
    const XModel *XModell; // eax
    const XModel *v5; // eax
    unsigned int v6; // [esp-38FCh] [ebp-3908h]
    unsigned int v7; // [esp-38FCh] [ebp-3908h]
    const chull_t *collmap_chull; // [esp-38E8h] [ebp-38F4h]
    PhysGeomList *Collmap; // [esp-38E4h] [ebp-38F0h]
    phys_mat44 v10; // [esp-38E0h] [ebp-38ECh] BYREF
    float v11[9]; // [esp-3898h] [ebp-38A4h] BYREF
    DynEntityPose *v12; // [esp-3874h] [ebp-3880h]
    const DynEntityDef *EntityDef; // [esp-3870h] [ebp-387Ch]
    unsigned __int16 *v14; // [esp-386Ch] [ebp-3878h]
    signed int n; // [esp-3868h] [ebp-3874h]
    signed int v16; // [esp-3864h] [ebp-3870h]
    _BYTE v17[8000]; // [esp-3860h] [ebp-386Ch] BYREF
    float v18; // [esp-1920h] [ebp-192Ch]
    DynEntityPose *ClientPose; // [esp-191Ch] [ebp-1928h]
    signed int m; // [esp-1918h] [ebp-1924h]
    float v21[4]; // [esp-1914h] [ebp-1920h] BYREF
    cg_s *LocalClientGlobals; // [esp-1904h] [ebp-1910h]
    unsigned __int16 v23[1000]; // [esp-1900h] [ebp-190Ch] BYREF
    DynEntityDrawType k; // [esp-1130h] [ebp-113Ch]
    const float *v25; // [esp-112Ch] [ebp-1138h]
    const float *v26; // [esp-1128h] [ebp-1134h]
    DObj *ClientDObj; // [esp-1124h] [ebp-1130h]
    phys_mat44 v28; // [esp-1120h] [ebp-112Ch] BYREF
    float v29[9]; // [esp-10D4h] [ebp-10E0h] BYREF
    centity_s *Entity; // [esp-10B0h] [ebp-10BCh]
    signed int j; // [esp-10ACh] [ebp-10B8h]
    int v32; // [esp-10A8h] [ebp-10B4h]
    bool v33; // [esp-10A4h] [ebp-10B0h]
    bool v34; // [esp-10A0h] [ebp-10ACh]
    const float *v35; // [esp-109Ch] [ebp-10A8h]
    const float *v36; // [esp-1098h] [ebp-10A4h]
    DObj *ServerDObj; // [esp-1094h] [ebp-10A0h]
    phys_mat44 v38; // [esp-1090h] [ebp-109Ch] BYREF
    float v39[9]; // [esp-1050h] [ebp-105Ch] BYREF
    gentity_s *v40; // [esp-102Ch] [ebp-1038h]
    signed int i; // [esp-1028h] [ebp-1034h]
    signed int v42; // [esp-1024h] [ebp-1030h]
    int v43[1000]; // [esp-1020h] [ebp-102Ch] BYREF
    unsigned int v44[4]; // [esp-80h] [ebp-8Ch] BYREF
    unsigned int v45[4]; // [esp-70h] [ebp-7Ch] BYREF
    float v46[4]; // [esp-60h] [ebp-6Ch] BYREF
    unsigned int v47[4]; // [esp-50h] [ebp-5Ch] BYREF
    float v48[4]; // [esp-40h] [ebp-4Ch] BYREF
    float v49[3]; // [esp-30h] [ebp-3Ch] BYREF
    float v50[3]; // [esp-24h] [ebp-30h] BYREF
    signed int v51; // [esp-18h] [ebp-24h]
    int v52; // [esp-14h] [ebp-20h]
    float v53; // [esp-10h] [ebp-1Ch]
    float v54; // [esp-Ch] [ebp-18h]
    float v55; // [esp-8h] [ebp-14h]
    //unsigned int v56[3]; // [esp+0h] [ebp-Ch] BYREF
    //_UNKNOWN *retaddr; // [esp+Ch] [ebp+0h]
    //
    //v56[0] = a1;
    //v56[1] = retaddr;
    v3 = alloca(14576);
    v53 = query_radius;
    v54 = query_radius;
    v55 = query_radius;
    v52 = 1000;
    v51 = 200;
    v50[0] = *pos - query_radius;
    v50[1] = pos[1] - query_radius;
    v50[2] = pos[2] - query_radius;
    v49[0] = *pos + query_radius;
    v49[1] = pos[1] + query_radius;
    v49[2] = pos[2] + query_radius;
    v48[0] = 0.0f;
    v48[1] = 1.0f;
    v48[2] = 1.0f;
    v48[3] = 0.25f;
    *(float *)v47 = 1.0f;
    v47[1] = 0;
    v47[2] = 0;
    *(float *)&v47[3] = 0.25f;
    v46[0] = 0.0f;
    v46[1] = 1.0f;
    v46[2] = 0.0f;
    v46[3] = 0.25f;
    *(float *)v45 = 1.0f;
    *(float *)&v45[1] = 1.0f;
    v45[2] = 0;
    *(float *)&v45[3] = 0.25f;
    *(float *)v44 = 1.0f;
    *(float *)&v44[1] = 1.0f;
    *(float *)&v44[2] = 1.0f;
    *(float *)&v44[3] = 0.25f;
    v42 = CM_AreaEntities(v50, v49, v43, 200, contentmask);
    for ( i = 0; i < v42; ++i )
    {
        v40 = &g_entities[v43[i]];
        AnglesToAxis(v40->r.currentAngles, (float (*)[3])v39);
        Phys_AxisToNitrousMat((float (*)[3])v39, &v38);
        Phys_Vec3ToNitrousVec(v40->r.currentOrigin, &v38.w);
        ServerDObj = Com_GetServerDObj(v43[i]);
        if ( ServerDObj )
        {
            if ( (v40->r.contents & 0x405C0008) != 0 )
                v36 = (const float *)v47;
            else
                v36 = v48;
            v6 = (unsigned int)v40;
            XModell = DObjGetXModel(ServerDObj, 0);
            render_xmodel_chull(XModell, v6, &v38, v36);
        }
        else if ( v40->r.bmodel )
        {
            if ( (v40->r.contents & 0x405C0008) != 0 )
                v35 = (const float *)v47;
            else
                v35 = v48;
            render_brushmodel_chull(v40->s.index.brushmodel, (unsigned int)v40, &v38, v35);
        }
    }
    v34 = Phys_GetMaskFromDVar() == 0x2820011
        && (g_bDebugRenderPatches->current.enabled || g_bDebugRenderBrushes->current.enabled);
    v33 = v34;
    if ( !v34 )
    {
        v32 = cgCollWorldLocalClientNum;
        v42 = CG_AreaEntities(v50, v49, v43, 200, contentmask);
        for ( j = 0; j < v42; ++j )
        {
            if ( v43[j] >= 1024 )
            {
                Entity = CG_GetEntity(v32, v43[j]);
                AnglesToAxis(Entity->pose.angles, (float (*)[3])v29);
                Phys_AxisToNitrousMat((float (*)[3])v29, &v28);
                Phys_Vec3ToNitrousVec(Entity->pose.origin, &v28.w);
                ClientDObj = Com_GetClientDObj(v43[j], v32);
                if ( ClientDObj )
                {
                    if ( ((*((unsigned int *)Entity + 201) >> 15) & 1) != 0 )
                        v26 = (const float *)v44;
                    else
                        v26 = (const float *)v45;
                    v7 = (unsigned int)Entity;
                    v5 = DObjGetXModel(ClientDObj, 0);
                    render_xmodel_chull(v5, v7, &v28, v26);
                }
                else if (Entity->nextState.solid == 0xFFFFFF)
                {
                    if ( ((*((unsigned int *)Entity + 201) >> 15) & 1) != 0 )
                        v25 = (const float *)v44;
                    else
                        v25 = (const float *)v45;
                    render_brushmodel_chull(Entity->nextState.index.brushmodel, (unsigned int)Entity, &v28, v25);
                }
            }
        }
        for ( k = DYNENT_DRAW_MODEL; k < DYNENT_DRAW_COUNT; ++k )
        {
            v42 = DynEntCl_AreaEntities(k, v50, v49, contentmask, 0x3E8u, v23);
            LocalClientGlobals = CG_GetLocalClientGlobals(0);
            //LODWORD(v21[3]) = LocalClientGlobals->predictedPlayerState.origin;
            v21[0] = LocalClientGlobals->predictedPlayerState.origin[0];
            v21[1] = LocalClientGlobals->predictedPlayerState.origin[1];
            v21[2] = LocalClientGlobals->predictedPlayerState.origin[2];
            for ( m = 0; m < v42; ++m )
            {
                ClientPose = DynEnt_GetClientPose(v23[m], k);
                v18 = Vec3DistanceSq(v21, ClientPose->pose.origin);
                *(_WORD *)&v17[8 * m] = v23[m];
                *(float *)&v17[8 * m + 4] = v18;
            }
            qsort(v17, v42, 8u, (int (__cdecl *)(const void *, const void *))SortSModelsByDist);
            if ( v42 >= v51 )
                v16 = v51;
            else
                v16 = v42;
            v42 = v16;
            for ( n = 0; n < v42; ++n )
            {
                v14 = (unsigned __int16 *)&v17[8 * n];
                EntityDef = DynEnt_GetEntityDef(*v14, k);
                v12 = DynEnt_GetClientPose(*v14, k);
                UnitQuatToAxis(v12->pose.quat, (float (*)[3])v11);
                Phys_AxisToNitrousMat((float (*)[3])v11, &v10);
                Phys_Vec3ToNitrousVec(v12->pose.origin, &v10.w);
                if ( k )
                {
                    render_brushmodel_chull(EntityDef->brushModel, (unsigned int)EntityDef, &v10, v46);
                }
                else
                {
                    render_xmodel_chull(EntityDef->xModel, (unsigned int)EntityDef, &v10, v48);
                    if ( XModelHasCollmap(EntityDef->xModel) )
                    {
                        Collmap = XModelGetCollmap(EntityDef->xModel, 0);
                        collmap_chull = get_collmap_chull(Collmap, (unsigned int)EntityDef);
                        render_chull(collmap_chull, &v10, v46);
                    }
                }
            }
        }
    }
}

bool prevDebugRenderEntityBrushes;
float prevDebugRenderCollisionDistance;
bool bPrevDebugRenderBrushes;
bool bPrevDebugRenderColoredPatches;
bool bPrevDebugRenderPatches;
bool bPrevDebugRenderBulletMeshes;

int prevDebugRenderMask = -1;



char __cdecl Phys_DebugRenderChanged()
{
    if ( bPrevDebugRenderBulletMeshes == g_bDebugRenderBulletMeshes->current.enabled
        && bPrevDebugRenderPatches == g_bDebugRenderPatches->current.enabled
        && bPrevDebugRenderColoredPatches == g_bDebugRenderColoredPatches->current.enabled
        && bPrevDebugRenderBrushes == g_bDebugRenderBrushes->current.enabled
        && prevDebugRenderMask == g_debugRenderMask->current.integer
        && prevDebugRenderCollisionDistance == g_debugRenderCollisionDistance->current.value
        && prevDebugRenderEntityBrushes == g_bDebugRenderEntityBrushes->current.enabled )
    {
        return 0;
    }
    bPrevDebugRenderBulletMeshes = g_bDebugRenderBulletMeshes->current.enabled;
    bPrevDebugRenderPatches = g_bDebugRenderPatches->current.enabled;
    bPrevDebugRenderColoredPatches = g_bDebugRenderColoredPatches->current.enabled;
    bPrevDebugRenderBrushes = g_bDebugRenderBrushes->current.enabled;
    prevDebugRenderMask = g_debugRenderMask->current.integer;
    prevDebugRenderCollisionDistance = g_debugRenderCollisionDistance->current.value;
    prevDebugRenderEntityBrushes = g_bDebugRenderEntityBrushes->current.enabled;
    return 1;
}

int __cdecl Phys_GetMaskFromDVar()
{
    int result; // eax
    unsigned int mask; // [esp+4h] [ebp-4h]

    mask = g_debugRenderMask->current.unsignedInt;
    if ( mask > 8
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_main.cpp",
                    2090,
                    0,
                    "%s",
                    "mask >= 0 && mask <= 8") )
    {
        __debugbreak();
    }
    switch ( mask )
    {
        case 0u:
            result = -1;
            break;
        case 1u:
            result = 0x2818011;
            break;
        case 2u:
            result = 0x10000;
            break;
        case 3u:
            result = 529;
            break;
        case 4u:
            result = 0x2820011;
            break;
        case 5u:
            result = 0x2809803;
            break;
        case 6u:
            result = 0x280E833;
            break;
        case 7u:
            result = 0x1000000;
            break;
        case 8u:
            result = 0x280EC93;
            break;
        default:
            result = -1;
            break;
    }
    return result;
}

float previous_pos[3] = { 3.4028235e38, 3.4028235e38, 3.4028235e38 };

void __cdecl Phys_RenderWorldCollMesh(const float *pos, bool bRenderStaticCollision, bool bRenderBrushes)
{
    float s_expand_h; // [esp+7Ch] [ebp-A4h]
    colgeom_debug_renderer_t colgeomDebugVisitor; // [esp+80h] [ebp-A0h] BYREF
    bool bDebugRenderChanged; // [esp+107h] [ebp-19h]
    float s_expand_v; // [esp+108h] [ebp-18h]
    float expand_vec[3]; // [esp+10Ch] [ebp-14h] BYREF
    int mask; // [esp+118h] [ebp-8h]
    bool bDoWork; // [esp+11Fh] [ebp-1h]

    bDoWork = 0;
    bDebugRenderChanged = Phys_DebugRenderChanged();
    if ( bRenderStaticCollision || bRenderBrushes )
    {
        if ( bDebugRenderChanged )
        {
            bDoWork = 1;
        }
        else if ( previous_pos[0] == 3.4028235e38 )
        {
            bDoWork = 1;
        }
        else if ( (float)((float)((float)((float)(*pos - previous_pos[0]) * (float)(*pos - previous_pos[0]))
                                                        + (float)((float)(pos[1] - previous_pos[1]) * (float)(pos[1] - previous_pos[1])))
                                        + (float)((float)(pos[2] - previous_pos[2]) * (float)(pos[2] - previous_pos[2]))) > (float)((float)(g_debugRenderCollisionDistance->current.value * 0.5) * (float)(g_debugRenderCollisionDistance->current.value * 0.5)) )
        {
            bDoWork = 1;
        }
        if ( bDoWork )
        {
            previous_pos[0] = *pos;
            previous_pos[1] = pos[1];
            previous_pos[2] = pos[2];
            DebugPatchesAndBrushesProlog();
            clear_debug_brushes_and_patches();
            //colgeom_visitor_t::colgeom_visitor_t(&colgeomDebugVisitor);
            //colgeomDebugVisitor.__vftable = (colgeom_debug_renderer_t_vtbl *)&colgeom_debug_renderer_t::`vftable';
            //colgeom_debug_renderer_t::reset(&colgeomDebugVisitor);
            colgeomDebugVisitor.reset();
            colgeomDebugVisitor.bRenderStaticCollision = bRenderStaticCollision;
            colgeomDebugVisitor.bRenderBrushes = bRenderBrushes;
            colgeomDebugVisitor.lightPos[0] = *pos;
            colgeomDebugVisitor.lightPos[1] = pos[1];
            colgeomDebugVisitor.lightPos[2] = pos[2];
            mask = Phys_GetMaskFromDVar();
            s_expand_h = g_debugRenderCollisionDistance->current.value;
            s_expand_v = 5000.0f;
            expand_vec[0] = s_expand_h;
            expand_vec[1] = s_expand_h;
            expand_vec[2] = 5000.0f;
            //colgeom_debug_renderer_t::reset(&colgeomDebugVisitor);
            colgeomDebugVisitor.reset();
            //colgeom_debug_renderer_t::update(&colgeomDebugVisitor, pos, pos, mask, expand_vec);
            colgeomDebugVisitor.update(pos, pos, mask, expand_vec);
            DebugPatchesAndBrushesEpilog();
        }
    }
    else if ( bDebugRenderChanged )
    {
        DebugPatchesAndBrushesProlog();
        clear_debug_brushes_and_patches();
        R_ClearPersistentDebugPolygons();
        DebugPatchesAndBrushesEpilog();
    }
}

void __thiscall colgeom_debug_renderer_t::reset()
{
    this->m_mn.vec.v[0] = 9.9999997e37;
    this->m_mn.vec.v[1] = 9.9999997e37;
    this->m_mn.vec.v[2] = 9.9999997e37;
    this->m_mx.vec.v[0] = -9.9999997e37;
    this->m_mx.vec.v[1] = -9.9999997e37;
    this->m_mx.vec.v[2] = -9.9999997e37;
}

void __thiscall colgeom_debug_renderer_t::visit(const CollisionAabbTree *tree)
{
    if ( this->bRenderStaticCollision )
        add_debug_patch(tree);
}

void __thiscall colgeom_debug_renderer_t::visit(const cbrush_t *brush)
{
    if ( this->bRenderBrushes )
        add_debug_brush(brush, 0);
}

void __thiscall colgeom_debug_renderer_t::update(
                const float *_mn,
                const float *_mx,
                int mask,
                const float *expand_vec)
{
    bool v5; // [esp+0h] [ebp-58h]
    float result[3]; // [esp+18h] [ebp-40h] BYREF
    float b[3]; // [esp+24h] [ebp-34h] BYREF
    float a[3]; // [esp+30h] [ebp-28h] BYREF
    bool inside; // [esp+3Fh] [ebp-19h]
    float mx[3]; // [esp+40h] [ebp-18h] BYREF
    float mn[3]; // [esp+4Ch] [ebp-Ch] BYREF

    a[0] = this->m_mn.vec.v[0] - *_mn;
    a[1] = this->m_mn.vec.v[1] - _mn[1];
    a[2] = this->m_mn.vec.v[2] - _mn[2];
    b[0] = *_mx - this->m_mx.vec.v[0];
    b[1] = _mx[1] - this->m_mx.vec.v[1];
    b[2] = _mx[2] - this->m_mx.vec.v[2];
    Vec3Max(a, b, result);
    v5 = result[0] < 0.0 && result[1] < 0.0 && result[2] < 0.0;
    inside = v5;
    if ( this->m_mask != mask || !inside )
    {
        mn[0] = *_mn - *expand_vec;
        mn[1] = _mn[1] - expand_vec[1];
        mn[2] = _mn[2] - expand_vec[2];
        mx[0] = *_mx + *expand_vec;
        mx[1] = _mx[1] + expand_vec[1];
        mx[2] = _mx[2] + expand_vec[2];
        colgeom_debug_renderer_t::reset();
        colgeom_visitor_t::intersect_box(mn, mx, mask);
    }
}

float scale_3 = 5000.0;
float velScale = 20.0;
float sc1 = 20.0;
int nboxes = 200;

int down;
int g_debug_partition;
void __cdecl debug_loop()
{
    const char *Name; // [esp+8h] [ebp-844h]
    const DObj *dobj; // [esp+B4h] [ebp-798h]
    unsigned int id; // [esp+B8h] [ebp-794h]
    float endpos[3]; // [esp+BCh] [ebp-790h] BYREF
    DynEntityDef *def; // [esp+C8h] [ebp-784h]
    float hitDir[3]; // [esp+CCh] [ebp-780h]
    float vel[3]; // [esp+D8h] [ebp-774h] BYREF
    float origin[3]; // [esp+E4h] [ebp-768h] BYREF
    float quat[4]; // [esp+F0h] [ebp-75Ch] BYREF
    float hitPos[3]; // [esp+100h] [ebp-74Ch]
    colgeom_visitor_inlined_t<200> visitor; // [esp+10Ch] [ebp-740h] BYREF
    float mins[3]; // [esp+7C8h] [ebp-84h] BYREF
    float maxs[4]; // [esp+7D4h] [ebp-78h] BYREF
    int hitNum; // [esp+7E4h] [ebp-68h] BYREF
    int mask; // [esp+7E8h] [ebp-64h]
    int passEntityNum; // [esp+7ECh] [ebp-60h]
    cg_s *cgameGlob; // [esp+7F0h] [ebp-5Ch]
    float start[3]; // [esp+7F4h] [ebp-58h] BYREF
    float end[3]; // [esp+800h] [ebp-4Ch] BYREF
    bool render_hitpoint; // [esp+80Fh] [ebp-3Dh]
    trace_t results; // [esp+810h] [ebp-3Ch] BYREF
    int savedregs; // [esp+84Ch] [ebp+0h] BYREF

    cgameGlob = CG_GetLocalClientGlobals(0);
    if ( cgameGlob && g_bDebugRenderBulletMeshes && g_bDebugRenderEntityBrushes )
    {
        start[0] = cgameGlob->refdef.vieworg[0];
        start[1] = cgameGlob->refdef.vieworg[1];
        start[2] = cgameGlob->refdef.vieworg[2];
        end[0] = (float)(scale_3 * cgameGlob->refdef.viewaxis[0][0]) + start[0];
        end[1] = (float)(scale_3 * cgameGlob->refdef.viewaxis[0][1]) + start[1];
        end[2] = (float)(scale_3 * cgameGlob->refdef.viewaxis[0][2]) + start[2];
        if ( g_bDebugRenderBulletMeshes->current.enabled )
            Phys_FindAndRenderBulletMesh(start, end);
        if ( g_bDebugRenderEntityBrushes->current.enabled )
        {
            mask = 1121773627;
            if ( g_bDebugRenderPatches->current.enabled || g_bDebugRenderBrushes->current.enabled )
                mask = Phys_GetMaskFromDVar();
            Phys_FindAndRenderEntityBrushes(start, mask);
        }
        g_debug_partition = 0;
        Phys_RenderWorldCollMesh(start, g_bDebugRenderPatches->current.enabled, g_bDebugRenderBrushes->current.enabled);
        if ( g_bDebugRenderStaticModelsBounds->current.enabled )
            draw_static_models_bounds();
        if ( g_dumpStaticModels->current.enabled )
        {
            Dvar_SetBool((dvar_s *)g_dumpStaticModels, 0);
            print_static_models_stats();
        }
        if ( debug_trace->current.integer )
        {
            passEntityNum = 0;
            memset(&results, 0, sizeof(results));
            render_hitpoint = 0;
            switch ( debug_trace->current.integer )
            {
                case 1:
                    hitNum = 0;
                    CG_SightTracePoint(&hitNum, start, end, 0x280E833, &results);
                    render_hitpoint = 1;
                    break;
                case 2:
                    G_LocationalTrace(
                        &results,
                        start,
                        end,
                        passEntityNum,
                        0x280E833,
                        bulletPriorityMap,
                        0);
                    render_hitpoint = 1;
                    g_debug_partition = results.hitPartition;
                    break;
                case 3:
                    CG_LocationalTrace(&results, start, end, passEntityNum, 0x280E833, 0, 0);
                    render_hitpoint = 1;
                    g_debug_partition = results.hitPartition;
                    break;
                case 4:
                    //colgeom_visitor_t::colgeom_visitor_t(&visitor);
                    //visitor.__vftable = (colgeom_visitor_inlined_t<200>_vtbl *)&colgeom_visitor_inlined_t<200>::`vftable';
                    //colgeom_visitor_inlined_t<500>::reset(&visitor);
                    visitor.reset();
                    mins[0] = -50.0f;
                    mins[1] = -50.0f;
                    mins[2] = -50.0f;
                    maxs[0] = 50.0f;
                    maxs[1] = 50.0f;
                    maxs[2] = 50.0f;
                    maxs[3] = NAN;
                    //colgeom_visitor_inlined_t<200>::update(&visitor, start, start, mins, maxs, -1);
                    visitor.update(start, start, mins, maxs, -1);
                    render_prims(visitor.prims, visitor.nprims);
                    //visitor.__vftable = (colgeom_visitor_inlined_t<200>_vtbl *)&visitor_base_t::`vftable';
                    break;
                case 5:
                    if ( down )
                    {
                        if ( !Key_IsDown(0, 19) )
                        {
                            if ( cm.dynEntCount[0] )
                            {
                                hitPos[0] = start[0];
                                hitPos[1] = start[1];
                                hitPos[2] = start[2];
                                hitDir[0] = 1.0f;
                                hitDir[1] = 0.0f;
                                hitDir[2] = 0.0f;
                                memset(quat, 0, 12);
                                quat[3] = 1.0f;
                                origin[0] = (float)(sc1 * cgameGlob->refdef.viewaxis[0][0]) + start[0];
                                origin[1] = (float)(sc1 * cgameGlob->refdef.viewaxis[0][1]) + start[1];
                                origin[2] = (float)(sc1 * cgameGlob->refdef.viewaxis[0][2]) + start[2];
                                vel[0] = velScale * cgameGlob->refdef.viewaxis[0][0];
                                vel[1] = velScale * cgameGlob->refdef.viewaxis[0][1];
                                vel[2] = velScale * cgameGlob->refdef.viewaxis[0][2];
                                def = &cm.dynEntDefList[0][G_irand(0, cm.dynEntCount[0] - 1)];
                                if ( def->physPreset )
                                    DynEntCl_CreateEntityModel(def->xModel, quat, origin, origin, vel, 7, def->physPreset);
                            }
                        }
                    }
                    down = Key_IsDown(0, 19);
                    break;
                default:
                    break;
            }
            if ( render_hitpoint && results.fraction < 1.0 )
            {
                Vec3Lerp(start, end, results.fraction, endpos);
                G_DebugStar(endpos, colorRed, 1);
                if ( results.hitType == TRACE_HITTYPE_ENTITY )
                {
                    id = Trace_GetEntityHitId(&results);
                    dobj = 0;
                    if ( debug_trace->current.integer == 2 )
                    {
                        dobj = Com_GetServerDObj(id);
                    }
                    else if ( debug_trace->current.integer == 3 )
                    {
                        dobj = Com_GetClientDObj(id, 0);
                    }
                    if ( dobj )
                    {
                        Name = DObjGetName(dobj);
                        Com_Printf(20, "%d, %s\n", id, Name);
                    }
                }
            }
        }
    }
}

void __cdecl print_smodel(cStaticModel_s *smodel, int refcount, const float *dims)
{
    XModel *model; // [esp+28h] [ebp-8h]
    int ntris; // [esp+2Ch] [ebp-4h]

    model = smodel->xmodel;
    //BLOPS_NULLSUB();
    ntris = 0; // LWSS: ntris is set above in the removed function (in this version, the value is undefined)
    Com_Printf(
        20,
        "%-42s\t refs( %4d ) lods( %d ) ncol_tris( %4d )     avr dims( %5.0f, %5.0f, %5.0f )\n",
        model->name,
        refcount,
        model->numLods,
        ntris,
        (float)((float)(1.0 / (float)refcount) * *dims),
        (float)((float)(1.0 / (float)refcount) * dims[1]),
        (float)((float)(1.0 / (float)refcount) * dims[2]));
}

void __cdecl print_static_models_stats()
{
    unsigned int numStaticModels; // [esp+0h] [ebp-10050h]
    cStaticModel_s *v1; // [esp+20h] [ebp-10030h]
    signed int j; // [esp+24h] [ebp-1002Ch]
    signed int i; // [esp+28h] [ebp-10028h]
    cStaticModel_s *smodel; // [esp+2Ch] [ebp-10024h]
    unsigned int base[16384]; // [esp+30h] [ebp-10020h] BYREF
    int refcount; // [esp+10030h] [ebp-20h]
    float v7; // [esp+10034h] [ebp-1Ch]
    float v8; // [esp+10038h] [ebp-18h]
    float v9; // [esp+1003Ch] [ebp-14h]
    float dims; // [esp+10040h] [ebp-10h] BYREF
    float v11; // [esp+10044h] [ebp-Ch]
    float v12; // [esp+10048h] [ebp-8h]
    unsigned int num; // [esp+1004Ch] [ebp-4h]

    Com_Printf(20, "Static models %d\n\n", cm.numStaticModels);
    if ( (int)cm.numStaticModels <= 0x4000 )
        numStaticModels = cm.numStaticModels;
    else
        numStaticModels = 0x4000;
    num = numStaticModels;
    for ( i = 0; i < (int)num; ++i )
        base[i] = (unsigned int) & cm.staticModelList[i];
    qsort(base, num, 4u, SortModelsFunc);
    refcount = 1;
    smodel = (cStaticModel_s *)base[0];
    dims = *(float *)(base[0] + 68) - *(float *)(base[0] + 56);
    v11 = *(float *)(base[0] + 72) - *(float *)(base[0] + 60);
    v12 = *(float *)(base[0] + 76) - *(float *)(base[0] + 64);
    for ( j = 1; j < (int)num; ++j )
    {
        v1 = (cStaticModel_s *)base[j];
        if ( v1->xmodel == smodel->xmodel )
        {
            ++refcount;
            v7 = smodel->absmax[0] - smodel->absmin[0];
            v8 = smodel->absmax[1] - smodel->absmin[1];
            v9 = smodel->absmax[2] - smodel->absmin[2];
            dims = v7 + dims;
            v11 = v8 + v11;
            v12 = v9 + v12;
        }
        else
        {
            print_smodel(smodel, refcount, &dims);
            smodel = v1;
            dims = v1->absmax[0] - v1->absmin[0];
            v11 = v1->absmax[1] - v1->absmin[1];
            v12 = v1->absmax[2] - v1->absmin[2];
            refcount = 1;
        }
    }
    print_smodel(smodel, refcount, &dims);
}

int __cdecl SortModelsFunc(const void *a, const void *b)
{
    if ( *(unsigned int *)(*(unsigned int *)a + 4) <= *(unsigned int *)(*(unsigned int *)b + 4) )
        return *(unsigned int *)(*(unsigned int *)a + 4) < *(unsigned int *)(*(unsigned int *)b + 4);
    else
        return -1;
}

void __cdecl draw_static_models_bounds()
{
    int v0; // [esp+10h] [ebp-4Ch]
    unsigned int numStaticModels; // [esp+14h] [ebp-48h]
    int i; // [esp+2Ch] [ebp-30h]
    float len2; // [esp+30h] [ebp-2Ch]
    cStaticModel_s *smodel; // [esp+34h] [ebp-28h]
    int j; // [esp+38h] [ebp-24h]
    cg_s *cgameGlob; // [esp+3Ch] [ebp-20h]
    float proj[3]; // [esp+40h] [ebp-1Ch] BYREF
    int nmodels; // [esp+4Ch] [ebp-10h]
    float p[3]; // [esp+50h] [ebp-Ch] BYREF

    if ((int)cm.numStaticModels <= 0x4000)
        numStaticModels = cm.numStaticModels;
    else
        numStaticModels = 0x4000;
    nmodels = numStaticModels;
    cgameGlob = CG_GetLocalClientGlobals(0);
    p[0] = cgameGlob->predictedPlayerState.origin[0];
    p[1] = cgameGlob->predictedPlayerState.origin[1];
    p[2] = cgameGlob->predictedPlayerState.origin[2];
    for (j = 0; j < nmodels; ++j)
    {
        smodel = &cm.staticModelList[j];
        Vec3Min(smodel->absmax, p, proj);
        Vec3Max(proj, smodel->absmin, proj);
        len2 = Vec3DistanceSq(p, proj);
        smodel_debug_infos[j].smodel = smodel;
        smodel_debug_infos[j].dist2 = len2;
    }
    qsort(smodel_debug_infos, nmodels, 8u, (int(__cdecl *)(const void *, const void *))SortSModelsByDist);
    for (i = 0; ; ++i)
    {
        v0 = nmodels >= nboxes ? nboxes : nmodels;
        if (i >= v0)
            break;
        CG_DebugBox(
            vec3_origin,
            smodel_debug_infos[i].smodel->absmin,
            smodel_debug_infos[i].smodel->absmax,
            0.0,
            colorWhite,
            1,
            1);
    }
}

int __cdecl SortSModelsByDist(float *a, float *b)
{
    if ( a[1] > b[1] )
        return 1;
    if ( b[1] <= a[1] )
        return 0;
    return -1;
}


int __cdecl buoyancy_worker()
{
    phys_free_list<PhysObjUserData>::T_internal_base *i; // [esp+14h] [ebp-4h]

    for ( i = g_pop_iter.m_ptr; g_pop_iter_end.m_ptr != i; i = g_pop_iter.m_ptr )
    {
        if ( (phys_free_list<PhysObjUserData>::T_internal_base *)_InterlockedCompareExchange(
                                                                                                                             (volatile unsigned __int32 *)&g_pop_iter,
                                                                                                                             (signed __int32)i->m_next_T_internal,
                                                                                                                             (signed __int32)i) == i )
            Phys_BodyGrabSnapshotNitrous((PhysObjUserData *)&i[2], g_delta_t);
    }
    return 0;
}

void __cdecl Phys_BodyGrabSnapshotNitrous(PhysObjUserData *userData, float deltaT)
{
    rigid_body *v2; // [esp+D8h] [ebp-10h] BYREF
    const phys_mat44 *rb2w; // [esp+DCh] [ebp-Ch]
    rigid_body *body; // [esp+E0h] [ebp-8h]
    float drag_scale; // [esp+E4h] [ebp-4h]
    int savedregs; // [esp+E8h] [ebp+0h] BYREF

    body = userData->body;
    v2 = body;
    _mm_prefetch((const char *)&v2, 1);
    if ( !body && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_main.cpp", 1360, 0, "%s", "body") )
        __debugbreak();
    rb2w = &userData->body->m_mat;
    phys_full_multiply_mat(&userData->cg2w, rb2w, &userData->cg2rb);
    if ( (body->m_flags & 0x20) == 0 )
    {
        drag_scale = 1.0f;
        if ( (userData->m_flags & 1) == 0 )
            drag_scale = 2.0f;
        body->m_t_drag_coef = phys_dragLinear->current.value * drag_scale;
        body->m_a_drag_coef = phys_dragAngular->current.value * drag_scale;
        userData->m_time_since_last_event = userData->m_time_since_last_event + deltaT;
        userData->m_time_since_last_reeval = userData->m_time_since_last_reeval + deltaT;
        Phys_ReEvalPriority(userData);
    }
}

//void __cdecl Nitrous_ForEachBody<void(__cdecl *)(PhysObjUserData &, float)>(void(__cdecl *func)(PhysObjUserData *, float), float t)
void __cdecl Nitrous_ForEachBody(void(__cdecl *func)(PhysObjUserData *, float), float t)
{
    PhysGlob *i; // [esp+Ch] [ebp-4h]

    for (i = (PhysGlob *)physGlob.objects.m_dummy_head.m_next_T_internal;
        &physGlob != i;
        i = (PhysGlob *)i->objects.m_dummy_head.m_next_T_internal)
    {
        ((void(__cdecl *)(PhysObjUserData **, _DWORD))func)(i->objects.m_ptr_list, LODWORD(t));
    }
}

int num_destructible_hits;
destructible_hit destructible_hits[10];
void __cdecl process_destructible_hits()
{
    float dir[3]; // [esp+Ch] [ebp-24h] BYREF
    centity_s *cent; // [esp+18h] [ebp-18h]
    destructible_hit *hit; // [esp+1Ch] [ebp-14h]
    float point[3]; // [esp+20h] [ebp-10h] BYREF
    int i; // [esp+2Ch] [ebp-4h]

    for (i = 0; i < num_destructible_hits; ++i)
    {
        hit = &destructible_hits[i];
        cent = CG_GetEntity(0, hit->entNum);
        dir[0] = 0.0f;
        dir[1] = 0.0f;
        dir[2] = 1.0f;
        point[0] = cent->pose.origin[0];
        point[1] = cent->pose.origin[1];
        point[2] = cent->pose.origin[2];
        CG_DestructibleDamage(cent, 0, dir, point, 100, 10, 0, 0, 1);
    }
    num_destructible_hits = 0;
}

void destructible_ent_aa::auto_activate(broad_phase_info *bpi_impactor)
{
    centity_s *cent; // [esp+4h] [ebp-4h]

    if (this->m_has_auto_activated
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_collision.cpp",
            233,
            0,
            "%s",
            "m_has_auto_activated == false"))
    {
        __debugbreak();
    }
    this->m_has_auto_activated = 1;
    cent = this->m_cent;
    if (!cent->destructible
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_collision.cpp",
            237,
            0,
            "%s",
            "cent->destructible"))
    {
        __debugbreak();
    }
    if (num_destructible_hits < 10)
        destructible_hits[num_destructible_hits++] = (destructible_hit)cent->nextState.number;
}

struct dynent_hit // sizeof=0x2
{
    unsigned __int16 absId;
};

dynent_hit dynent_hits[10];
int num_dynent_hits;
void __cdecl process_dynent_hits()
{
    DynEntityPose *dynEntPose; // [esp+4h] [ebp-2Ch]
    DynEntityDrawType drawType; // [esp+8h] [ebp-28h] BYREF
    float hitDir[3]; // [esp+Ch] [ebp-24h] BYREF
    dynent_hit *hit; // [esp+18h] [ebp-18h]
    float hitPos[3]; // [esp+1Ch] [ebp-14h] BYREF
    unsigned __int16 dynEntId; // [esp+28h] [ebp-8h] BYREF
    int i; // [esp+2Ch] [ebp-4h]

    for (i = 0; i < num_dynent_hits; ++i)
    {
        hit = &dynent_hits[i];
        DynEnt_GetClientIdDrawType(hit->absId, &dynEntId, &drawType);
        dynEntPose = DynEnt_GetClientPose(dynEntId, drawType);
        hitPos[0] = dynEntPose->pose.origin[0];
        hitPos[1] = dynEntPose->pose.origin[1];
        hitPos[2] = dynEntPose->pose.origin[2];
        hitDir[0] = 0.0f;
        hitDir[1] = 0.0f;
        hitDir[2] = 1.0f;
        DynEntCl_Damage(0, dynEntId, (DynEntityCollType)drawType, hitPos, hitDir, 100);
    }
    num_dynent_hits = 0;
}

void dynamic_ent_aa::auto_activate(broad_phase_info *bpi_impactor)
{
    DynEntityPose *dynEntPose; // [esp+Ch] [ebp-14h]
    DynEntityDrawType drawType; // [esp+10h] [ebp-10h]
    DynEntityClient *dynEntClient; // [esp+14h] [ebp-Ch]
    DynEntityDef *dynEntDef; // [esp+18h] [ebp-8h]
    unsigned __int16 dynEntId; // [esp+1Ch] [ebp-4h]

    if (this->m_has_auto_activated
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_collision.cpp",
            161,
            0,
            "%s",
            "m_has_auto_activated == false"))
    {
        __debugbreak();
    }
    this->m_has_auto_activated = 1;
    dynEntDef = this->m_dynEntDef;
    drawType = (DynEntityDrawType)(dynEntDef->xModel == 0);
    dynEntId = DynEnt_GetId(dynEntDef, drawType);
    dynEntClient = DynEnt_GetClientEntity(dynEntId, drawType);
    dynEntPose = DynEnt_GetClientPose(dynEntId, drawType);
    if (!dynEntClient->physObjId)
    {
        if (DynEnt_GetEntityProps(dynEntDef->type)->usePhysics)
            DynEntCl_CreatePhysObj(dynEntDef, dynEntClient, &dynEntPose->pose);
        else
            dynent_hits[num_dynent_hits++].absId = DynEnt_GetClientAbsId(dynEntId, drawType);
    }
}

void    UpdateRigidBody(float delta_t)
{
    float t; // xmm0_4
    float v3; // [esp+38h] [ebp-48h]
    float v4; // [esp+3Ch] [ebp-44h]
    float value; // [esp+54h] [ebp-2Ch]
    phys_free_list<rigid_body> *p_m_list_rigid_body; // [esp+58h] [ebp-28h]
    phys_free_list<rigid_body>::T_internal_base *m_next_T_internal; // [esp+5Ch] [ebp-24h]
    phys_free_list<rigid_body>::iterator i_end; // [esp+64h] [ebp-1Ch] BYREF
    phys_free_list<rigid_body>::iterator i; // [esp+68h] [ebp-18h]
    float v10; // [esp+6Ch] [ebp-14h]
    phys_vec3 gravity_dir; // [esp+70h] [ebp-10h] BYREF
    //float retaddr; // [esp+80h] [ebp+0h]
    //
    //gravity_dir.y = a1;
    //gravity_dir.z = retaddr;

    {
        PROF_SCOPED("update 1");

        //BLOPS_NULLSUB();
        debug_loop();
        free_chull_lists();
        Sys_EnterCriticalSection(CRITSECT_PHYSICS_UPDATE);
        if (gravityChange)
        {
            Phys_Vec3ToNitrousVec(phys_gravity_dir->current.vector, &gravity_dir);
            //m_next_T_internal = g_physics_system->m_list_rigid_body.m_dummy_head.m_next_T_internal;
            //p_m_list_rigid_body = &g_physics_system->m_list_rigid_body;
            //while ( p_m_list_rigid_body != (phys_free_list<rigid_body> *)m_next_T_internal )
            //{
            //    value = phys_gravity->current.value;
            //    v3 = value * *(float *)&i.m_ptr;
            //    v4 = value * v10;
            //    *(float *)&m_next_T_internal[18].m_prev_T_internal = value * *(float *)&i_end.m_ptr;
            //    *(float *)&m_next_T_internal[18].m_next_T_internal = v3;
            //    *(float *)&m_next_T_internal[19].m_prev_T_internal = v4;
            //    m_next_T_internal = m_next_T_internal->m_next_T_internal;
            //}

            phys_free_list<rigid_body>::T_internal_base *node = g_physics_system->m_list_rigid_body.m_dummy_head.m_next_T_internal;
            phys_free_list<rigid_body>::T_internal_base *end = &g_physics_system->m_list_rigid_body.m_dummy_head;

            while (node != end)
            {
                rigid_body *rb =
                    &static_cast<phys_free_list<rigid_body>::T_internal *>(node)->m_data;

                float g = phys_gravity->current.value;

                rb->m_t_vel.x = g * gravity_dir.x;
                rb->m_t_vel.y = g * gravity_dir.y;
                rb->m_t_vel.z = g * gravity_dir.z;

                node = node->m_next_T_internal;
            }

            gravityChange = 0;
        }
        if (delta_t >= 0.1)
            t = 0.1f;
        else
            t = delta_t;
        Phys_AddCacheImpulses();
        //Nitrous_ForEachBody<void (__cdecl *)(PhysObjUserData &,float)>(Phys_BodyGrabSnapshotNitrous, t);
        Nitrous_ForEachBody(Phys_BodyGrabSnapshotNitrous, t);
        //Nitrous_ForEachBody<void (__cdecl *)(PhysObjUserData &,float)>((void (__cdecl *)(PhysObjUserData *, float))Phys_DebugRender, t);
        Nitrous_ForEachBody((void(__cdecl *)(PhysObjUserData *, float))Phys_DebugRender, t);
        render_debug_draw_gjk_trace_geom();
        NitrousVehicle::frame_prolog_all_systems(t);
    }

    {
        PROF_SCOPED("phys_frame_advance");
        phys_sys::phys_frame_advance(t);
    }
    
    {
        PROF_SCOPED("update 2");

        NitrousVehicle::debug_render_all();
        NitrousVehicle::frame_epilog_all_systems(t);
        process_destructible_hits();
        process_dynent_hits();
    }
    
    Sys_LeaveCriticalSection(CRITSECT_PHYSICS_UPDATE);
}

void __cdecl Phys_DebugRender(PhysObjUserData *userData)
{
    int savedregs; // [esp+0h] [ebp+0h] BYREF

    if ( phys_drawCollisionObj->current.enabled )
        debug_render(userData);
}

user_rigid_body *__cdecl GetTraceResultsRigidBody(const trace_t *traceResults)
{
    const DynEntityPose *dynEntPose; // [esp+Ch] [ebp-20h]
    const DynEntityDef *dynEntDef; // [esp+10h] [ebp-1Ch]
    DynEntityClient *dynEntClient; // [esp+14h] [ebp-18h]
    user_rigid_body *rb; // [esp+18h] [ebp-14h]
    centity_s *cent; // [esp+1Ch] [ebp-10h]
    int physObjId; // [esp+20h] [ebp-Ch]
    DynEntityDrawType drawType; // [esp+24h] [ebp-8h] BYREF
    unsigned __int16 hitEntId; // [esp+28h] [ebp-4h]

    physObjId = 0;
    hitEntId = Trace_GetEntityHitId(traceResults);
    if (hitEntId != 1023 && hitEntId != 1022)
    {
        cent = CG_GetEntity(0, hitEntId);
        physObjId = cent->pose.physObjId;
        if (!physObjId)
        {
            rb = auto_rigid_body::ent_has_auto_rigid_body(cent);
            if (rb)
                return rb;
        }
    }
    hitEntId = Trace_GetDynEntHitId(traceResults, &drawType);
    if (hitEntId != 0xFFFF)
    {
        dynEntClient = DynEnt_GetClientEntity(hitEntId, drawType);
        if (!dynEntClient->physObjId)
        {
            dynEntDef = DynEnt_GetEntityDef(hitEntId, drawType);
            dynEntPose = DynEnt_GetClientPose(hitEntId, drawType);
            if (DynEnt_GetEntityProps(dynEntDef->type)->usePhysics)
            {
                if (dynEntDef->physPreset)
                    dynEntClient->physObjId = DynEntCl_CreatePhysObj(dynEntDef, dynEntClient, &dynEntPose->pose);
            }
        }
        physObjId = dynEntClient->physObjId;
    }
    if (physObjId)
        return (user_rigid_body *)Phys_GetUserData(physObjId)->body;
    else
        return (user_rigid_body *)phys_sys::get_environment_rigid_body();
}

void collide_vehicle_wheels(PhysObjUserData *userData)
{
    float v2; // [esp+2Ch] [ebp-578h] BYREF
    float v3; // [esp+30h] [ebp-574h] BYREF
    rigid_body_constraint_wheel *v5; // [esp+38h] [ebp-56Ch]
    int k; // [esp+3Ch] [ebp-568h]
    int v7; // [esp+40h] [ebp-564h]
    float m_current_front_side_fric_scale; // [esp+44h] [ebp-560h]
    float m_current_front_fwd_fric_scale; // [esp+48h] [ebp-55Ch]
    float m_current_rear_side_fric_scale; // [esp+4Ch] [ebp-558h]
    float m_current_rear_fwd_fric_scale; // [esp+50h] [ebp-554h]
    float v12; // [esp+54h] [ebp-550h]
    float v13; // [esp+58h] [ebp-54Ch]
    phys_vec3 v14; // [esp+5Ch] [ebp-548h] BYREF
    phys_vec3 *v15; // [esp+84h] [ebp-520h]
    phys_vec3 v16; // [esp+88h] [ebp-51Ch] BYREF
    phys_vec3 *p_m_ground_vel; // [esp+9Ch] [ebp-508h]
    phys_vec3 *p_m_t_vel; // [esp+A0h] [ebp-504h]
    environment_rigid_body *TraceResultsRigidBody; // [esp+A4h] [ebp-500h]
    phys_vec3 v20; // [esp+A8h] [ebp-4FCh] BYREF
    phys_vec3 v21; // [esp+B8h] [ebp-4ECh] BYREF
    int v22; // [esp+CCh] [ebp-4D8h] BYREF
    int m_entnum; // [esp+D8h] [ebp-4CCh]
    col_context_t v24; // [esp+DCh] [ebp-4C8h] BYREF
    float zero[3]; // [esp+13Ch] [ebp-468h] BYREF
    float p1[3]; // [esp+148h] [ebp-45Ch] BYREF
    float p0[4]; // [esp+154h] [ebp-450h] BYREF
    float v29; // [esp+164h] [ebp-440h]
    phys_vec3 v30; // [esp+168h] [ebp-43Ch] BYREF
    phys_vec3 v31; // [esp+178h] [ebp-42Ch] BYREF
    phys_vec3 phys_zerovec; // [esp+188h] [ebp-41Ch] BYREF
    rigid_body_constraint_wheel *v33; // [esp+19Ch] [ebp-408h]
    int j; // [esp+1A0h] [ebp-404h]
    int v35; // [esp+1A4h] [ebp-400h]
    NitrousVehicle *vehicle; // [esp+1A8h] [ebp-3FCh]
    rigid_body *rb; // [esp+438h] [ebp-16Ch]
    trace_t *i; // [esp+43Ch] [ebp-168h]
    trace_t traces[6]; // [esp+440h] [ebp-164h] BYREF
    int v40; // [esp+590h] [ebp-14h]
    //_UNKNOWN *v41[2]; // [esp+598h] [ebp-Ch] BYREF
    //int vars0; // [esp+5A4h] [ebp+0h]
    //
    //v41[0] = a1;
    //v41[1] = (_UNKNOWN *)vars0;
    
    PROF_SCOPED("phys_wheel_collision");

    v40 = 6;
    for (i = traces; --v40 >= 0; ++i)
    {
        i->normal.vec.v[0] = 0.0f;
        i->normal.vec.v[1] = 0.0f;
        i->normal.vec.v[2] = 0.0f;
        i->normal.vec.v[3] = 0.0f;
    }
    rb = userData->body;
    if (!rb || (rb->m_flags & 0x20) != 0)
    {
        return;
    }
    if (userData->vehicle)
    {
        vehicle = userData->vehicle;
        v35 = 0;
        for (j = 0; j < 6; ++j)
        {
            v33 = vehicle->m_wheels[j];
            if (v33)
            {
                memset(&phys_zerovec, 0, 12);
                //rigid_body_constraint_wheel::get_wheel_collide_segment(v33, v41, &rb->m_mat, &v30, &v31);
                v33->get_wheel_collide_segment(&rb->m_mat, &v30, &v31);
                v29 = (float)(v30.z - v31.z) * 0.30000001;
                LODWORD(p0[3]) = (DWORD)&v30.z;
                v30.z = v30.z + v29;
                Phys_NitrousVecToVec3(&v30, p0);
                Phys_NitrousVecToVec3(&v31, p1);
                Phys_NitrousVecToVec3(&phys_zerovec, zero);

                trace_t traceResults; // [esp+104h] [ebp-4A0h] BYREF
                //col_context_t::col_context_t(&v24);
                m_entnum = vehicle->m_entnum;
                CG_TraceCapsule(&traceResults, p0, zero, zero, p1, m_entnum, 529, &v24);
                if (traceResults.fraction == 1.0 || traceResults.fraction == 0.0)
                {
                    //rigid_body_constraint_wheel::set_no_collision(v33);
                    v33->set_no_collision();
                    vehicle->m_wheel_surf_types[j] = 0;
                }
                else
                {
                    ++v35;
                    Vec3Lerp(p0, p1, traceResults.fraction, (float *)&v22);
                    Phys_Vec3ToNitrousVec((float *)&v22, &v21);
                    Phys_Vec3ToNitrousVec(traceResults.normal.vec.v, &v20);
                    TraceResultsRigidBody = (environment_rigid_body*)GetTraceResultsRigidBody(&traceResults);
                    p_m_t_vel = &TraceResultsRigidBody->m_t_vel;
                    p_m_ground_vel = &vehicle->m_ground_vel;
                    vehicle->m_ground_vel.x = TraceResultsRigidBody->m_t_vel.x;
                    p_m_ground_vel->y = p_m_t_vel->y;
                    p_m_ground_vel->z = p_m_t_vel->z;
                    v15 = phys_full_inv_multiply(&v16, &TraceResultsRigidBody->m_mat, &v21);
                    v21.x = v15->x;
                    v21.y = v15->y;
                    v21.z = v15->z;
                    LODWORD(v14.z) = (DWORD)phys_inv_multiply((phys_vec3 *)&v14.w, &TraceResultsRigidBody->m_mat, &v20);
                    v20.x = *(float *)LODWORD(v14.z);
                    v20.y = *(float *)(LODWORD(v14.z) + 4);
                    v20.z = *(float *)(LODWORD(v14.z) + 8);
                    //rigid_body_constraint_wheel::set_collision(v33, TraceResultsRigidBody, &v21, &v20);
                    v33->set_collision(TraceResultsRigidBody, &v21, &v20);
                    LODWORD(v14.y) = (traceResults.sflags & 0x3F00000) >> 20;
                    vehicle->m_wheel_surf_types[j] = LODWORD(v14.y);
                    v14.x = Com_SurfaceFrictionScale(SLODWORD(v14.y));
                    v13 = 0.0f;
                    v12 = 0.0f;
                    if (j == 3 || j == 2)
                    {
                        m_current_rear_fwd_fric_scale = vehicle->m_current_rear_fwd_fric_scale;
                        v13 = v14.x * m_current_rear_fwd_fric_scale;
                        m_current_rear_side_fric_scale = vehicle->m_current_rear_side_fric_scale;
                        v12 = v14.x * m_current_rear_side_fric_scale;
                    }
                    else
                    {
                        m_current_front_fwd_fric_scale = vehicle->m_current_front_fwd_fric_scale;
                        v13 = v14.x * m_current_front_fwd_fric_scale;
                        m_current_front_side_fric_scale = vehicle->m_current_front_side_fric_scale;
                        v12 = v14.x * m_current_front_side_fric_scale;
                    }
                    if ((vehicle->m_flags & 0x80) == 0)
                    {
                        v33->m_fwd_fric_k = v13;
                        v33->m_side_fric_k = v12;
                    }
                }
            }
        }
        v7 = 3;
        if (v35 < 3 && userData->underwater > 0 && userData->vehicle->m_parameter->m_buoyancybox_min[0] != 0.0)
        {
            for (k = 0; k < 6; ++k)
            {
                v5 = vehicle->m_wheels[k];
                if (v5)
                {
                    if (!v5->m_wheel_state)
                    {
                        static const float acceleration_proportion = 0.1;


                        //rigid_body_constraint_wheel::get_wheel_state_accelerating(v5, &v2, &v3);
                        v5->get_wheel_state_accelerating(&v2, &v3);
                        //rigid_body_constraint_wheel::set_wheel_state_accelerating(v5, v2, v3 * acceleration_proportion);
                        v5->set_wheel_state_accelerating(v2, v3 *acceleration_proportion);
                    }
                }
            }
        }
    }
}

int __cdecl wheel_collision_worker(jqBatch *__);
phys_free_list<PhysObjUserData>::iterator g_wpop_iter;
phys_free_list<PhysObjUserData>::iterator g_wpop_iter_end;
jqModule wheelCollisionModule =
{
    .Name = "wheelCollision",
    .Type = JQ_WORKER_GENERIC,
    .Code = wheel_collision_worker,
};
void __cdecl prop_system_collision_process()
{
    g_wpop_iter.m_ptr = physGlob.objects.m_dummy_head.m_next_T_internal;
    //g_wpop_iter_end.m_ptr = (phys_free_list<PhysObjUserData>::T_internal_base *)&physGlob;
    g_wpop_iter_end.m_ptr = (phys_free_list<PhysObjUserData>::T_internal_base *)&physGlob;
    phys_task_manager_process(&wheelCollisionModule, 0, physGlob.objects.m_list_count);
    phys_task_manager_flush();
}

int __cdecl wheel_collision_worker(jqBatch *__)
{
    phys_free_list<PhysObjUserData>::T_internal_base *i; // [esp+10h] [ebp-4h]

    for (i = g_wpop_iter.m_ptr; g_wpop_iter_end.m_ptr != i; i = g_wpop_iter.m_ptr)
    {
        if ((phys_free_list<PhysObjUserData>::T_internal_base *)_InterlockedCompareExchange(
            (volatile unsigned __int32 *)&g_wpop_iter,
            (signed __int32)i->m_next_T_internal,
            (signed __int32)i) == i)
            collide_vehicle_wheels((PhysObjUserData *)&i[2]);
    }
    return 0;
}


void    Phys_CollisionCallback()
{
    const phys_vec3 *v1; // eax
    float v2; // [esp-2Ch] [ebp-CCh]
    float v3; // [esp-28h] [ebp-C8h]
    phys_vec3 v4; // [esp-Ch] [ebp-ACh] OVERLAPPED BYREF
    phys_vec3 v5; // [esp+4h] [ebp-9Ch] BYREF
    phys_vec3 player_origin_loc; // [esp+34h] [ebp-6Ch] BYREF
    gjk_base_t *geom; // [esp+50h] [ebp-50h]
    const phys_mat44 *geom_mat; // [esp+54h] [ebp-4Ch]
    broad_phase_info *bpi_env; // [esp+58h] [ebp-48h]
    float max_draw_dist_sq; // [esp+5Ch] [ebp-44h]
    float integer; // [esp+60h] [ebp-40h]
    phys_vec3 player_origin; // [esp+64h] [ebp-3Ch] BYREF
    centity_s *player_cent; // [esp+7Ch] [ebp-24h]
    phys_free_list<rigid_body_constraint_contact>::iterator rbc_i_end; // [esp+80h] [ebp-20h]
    phys_free_list<rigid_body_constraint_contact>::iterator rbc_i; // [esp+84h] [ebp-1Ch]
    phys_free_list<rigid_body_constraint_contact>::T_internal_base *m_next_T_internal; // [esp+88h] [ebp-18h]
    int v19; // [esp+8Ch] [ebp-14h]
    int m_alloc_count; // [esp+90h] [ebp-10h]
    //_UNKNOWN *v21[2]; // [esp+94h] [ebp-Ch] BYREF
    //int vars0; // [esp+A0h] [ebp+0h]
    //
    //v21[0] = a1;
    //v21[1] = (_UNKNOWN *)vars0;
    m_alloc_count = physGlob.objects_by_world[0].m_alloc_count;
    if (physGlob.objects_by_world[0].m_alloc_count >= 10 || (v19 = physGlob.objects_by_world[1].m_alloc_count, physGlob.objects_by_world[1].m_alloc_count >= 4))
    {
        //cdl_proftimer::start_capture(&proftimer_physics_frame_advance);
        proftimer_physics_frame_advance.start_capture();
    }
    prop_system_collision_process(); // This is used just for vehicle wheels now
    broad_phase_process();
    Phys_EffectsProcess();
    if (phys_drawcontacts->current.enabled)
    {
        m_next_T_internal = g_physics_system->m_list_rbc_contact.m_dummy_head.m_next_T_internal;
        rbc_i.m_ptr = m_next_T_internal;
        rbc_i_end.m_ptr = &g_physics_system->m_list_rbc_contact.m_dummy_head;
        while (rbc_i_end.m_ptr != rbc_i.m_ptr)
        {
            render_contact((rigid_body_constraint_contact *)&rbc_i.m_ptr[1]);
            rbc_i.m_ptr = rbc_i.m_ptr->m_next_T_internal;
        }
    }
    if (render_bpi_env_collision->current.integer > 0)
    {
        player_cent = CG_GetEntity(0, 0);
        Phys_Vec3ToNitrousVec(player_cent->pose.origin, &player_origin);
        integer = (float)render_bpi_env_collision->current.integer;
        max_draw_dist_sq = integer * integer;
        for (bpi_env = G_BPM->m_list_bpi_env; bpi_env; bpi_env = (broad_phase_info *)bpi_env->m_list_bpb_cluster_next)
        {
            geom_mat = bpi_env->m_cg_to_world_xform;
            geom = (gjk_base_t *)bpi_env->m_gjk_geom;
            //((void(__thiscall *)(gjk_base_t *, _DWORD, _DWORD))geom->comp_aabb_loc)(geom, LODWORD(v2), LODWORD(v3));
            geom->comp_aabb_loc();
            phys_full_inv_multiply(&player_origin_loc, geom_mat, &player_origin);
            //v1 = phys_max(&v5, &m_aabb_mn_loc, &player_origin_loc);
            v1 = phys_max(&v5, geom->get_aabb_mn(), &player_origin_loc);
            //phys_min(&v4, &m_aabb_mx_loc, v1);
            phys_min(&v4, geom->get_aabb_mx(), v1);
            v2 = v4.x - player_origin_loc.x;
            v3 = v4.y - player_origin_loc.y;
            if (max_draw_dist_sq >= (float)((float)((float)((float)(v4.x - player_origin_loc.x)
                * (float)(v4.x - player_origin_loc.x))
                + (float)((float)(v4.y - player_origin_loc.y)
                    * (float)(v4.y - player_origin_loc.y)))
                + (float)((float)(v4.z - player_origin_loc.z) * (float)(v4.z - player_origin_loc.z))))
                render_gjk_geom(geom, geom_mat);
        }
    }
    broad_phase_reset_buffer();
}

void __thiscall cdl_proftimer::start_capture()
{
    int i; // [esp+4h] [ebp-4h]

    if ( !this->capture )
    {
        this->capture = 1;
        this->capture_count = 0;
        this->tot = 0;
        this->avr = 0;
        for ( i = 0; i < 5; ++i )
        {
            LODWORD(this->mx[i]) = 0;
            HIDWORD(this->mx[i]) = 0;
        }
    }
}

void __thiscall cdl_proftimer::reset()
{
    unsigned __int64 tmp; // [esp+20h] [ebp-10h]
    int i; // [esp+28h] [ebp-8h]
    bool swap; // [esp+2Fh] [ebp-1h]

    this->calls = 0;
    this->avr = (this->value + 9 * this->avr) / 0xA;
    if (this->capture)
    {
        this->tot += this->value;
        this->mx[0] = this->value;
        do
        {
            swap = 0;
            for (i = 0; i < 5; ++i)
            {
                if (this->mx[i] > this->mx[i + 1])
                {
                    tmp = this->mx[i];
                    LODWORD(this->mx[i]) = this->mx[i + 1];
                    HIDWORD(this->mx[i]) = HIDWORD(this->mx[i + 1]);
                    this->mx[i + 1] = tmp;
                    swap = 1;
                }
            }
        } while (swap);
        if (++this->capture_count >= 0xB4)
            this->capture = 0;
    }
    this->value = 0;
}

bool __cdecl Phys_ShouldCollideCallback(const broad_phase_base *bpi1, const broad_phase_base *bpi2)
{
    if ( (bpi1->m_my_collision_type_flags & 0x40) != 0 )
    {
        if ( (bpi2->m_my_collision_type_flags & 4) != 0 )
            return 0;
    }
    else if ( (bpi1->m_my_collision_type_flags & 4) != 0 )
    {
        if ( (bpi2->m_my_collision_type_flags & 0x40) != 0 )
            return 0;
    }
    else if ( (bpi1->m_my_collision_type_flags & 0x400) != 0 && (bpi2->m_my_collision_type_flags & 0x400) != 0 )
    {
        return 0;
    }
    return 1;
}

void __cdecl PhysicsSystem_Update()
{
    PROF_SCOPED("PhysicsSystem_Update"); // LWSS ADD

    cg_s *cgameGlob; // [esp+0h] [ebp-Ch]
    int i; // [esp+4h] [ebp-8h]
    cg_s *cg; // [esp+8h] [ebp-4h]

    cg = CG_GetLocalClientGlobals(0);
    if ( cg && cg->snap )
    {
        for ( i = 0; i < 1; ++i )
        {
            if ( CL_LocalClient_IsActive(i) && CL_GetLocalClientConnectionState(i) >= 4 )
            {
                cgameGlob = CG_GetLocalClientGlobals(i);
                if ( cgameGlob )
                    Phys_RunToTime(cgameGlob->physicsTime);
                return;
            }
        }
    }
}

void __cdecl Phys_RunToTime(int timeNow)
{
    const char *v1; // eax
    int v2; // [esp+4h] [ebp-34h]
    int v3; // [esp+20h] [ebp-18h]
    int time_msec; // [esp+34h] [ebp-4h]
    int savedregs; // [esp+38h] [ebp+0h] BYREF

    proftimer_physics_frame_advance.stamp = tlPcGetTick().QuadPart;
    ++proftimer_physics_frame_advance.calls;

    {
        v1 = va("Physics update (obj=%d)", physGlob.objects.m_list_count);
        PROF_SCOPED_RUNTIME_NAME(v1);

        iassert(physInited);

        physGlob.timeLastSnapshot = physGlob.timeLastUpdate;
        time_msec = timeNow - physGlob.timeLastUpdate;
        if (timeNow != physGlob.timeLastUpdate)
        {
            PROF_SCOPED("auto_rigid_body");

            auto_rigid_body::update();
            if (!Demo_IsPlaying() || !Demo_IsPaused() && time_msec > 0 && time_msec < 100)
            {
                if (time_msec < 100)
                    v3 = time_msec;
                else
                    v3 = 100;
                if (v3 > 1)
                    v2 = v3;
                else
                    v2 = 1;
                time_msec = v2;
                UpdateRigidBody((float)v2 * 0.001);
            }
            physGlob.timeLastUpdate = timeNow;
        }
        proftimer_physics_frame_advance.value += tlPcGetTick().QuadPart - proftimer_physics_frame_advance.stamp;
    }

    {
        PROF_SCOPED("Ragdoll_Update/XDoll_Update");

        Ragdoll_Update(time_msec);
        XDoll_Update(time_msec);
    }
}

void __cdecl Phys_ObjGetInterpolatedState(int id, float *outPos, float *outQuat)
{
    float oldMat[3][3]; // [esp+10h] [ebp-84h] BYREF
    float newPos[3]; // [esp+34h] [ebp-60h] BYREF
    float frac; // [esp+40h] [ebp-54h]
    float newMat[3][3]; // [esp+44h] [ebp-50h] BYREF
    float oldPos[3]; // [esp+68h] [ebp-2Ch] BYREF
    float newQuat[4]; // [esp+74h] [ebp-20h] BYREF
    float oldQuat[4]; // [esp+84h] [ebp-10h] BYREF
    int savedregs; // [esp+94h] [ebp+0h] BYREF

    frac = 1.0f;
    if ( phys_msecStep->current.integer <= 0 )
    {
        Phys_ObjGetPosition(id, outPos, newMat);
        AxisToQuat(newMat, outQuat);
    }
    else
    {
        Phys_ObjGetSnapshot(id, oldPos, oldMat);
        Phys_ObjGetPosition(id, newPos, newMat);
        Vec3Lerp(oldPos, newPos, frac, outPos);
        AxisToQuat(oldMat, oldQuat);
        AxisToQuat(newMat, newQuat);
        QuatLerp(oldQuat, newQuat, frac, outQuat);
        Vec4Normalize(outQuat);
    }
}

void    Phys_SetUserBody(int id, float *position)
{
    phys_mat44 dictator; // [esp-Ch] [ebp-7Ch] BYREF
    float dictator_52; // [esp+34h] [ebp-3Ch]
    int dictator_56; // [esp+38h] [ebp-38h]
    int dictator_60; // [esp+3Ch] [ebp-34h]
    phys_vec3 pos; // [esp+44h] [ebp-2Ch] BYREF
    user_rigid_body *body; // [esp+5Ch] [ebp-14h]
    PhysObjUserData *userData; // [esp+60h] [ebp-10h]
    //_UNKNOWN *v10; // [esp+64h] [ebp-Ch]
    //int ida; // [esp+68h] [ebp-8h]
    //int vars0; // [esp+70h] [ebp+0h]
    //
    //v10 = a1;
    //ida = vars0;
    userData = Phys_GetUserData(id);
    body = (user_rigid_body *)userData->body;
    Phys_Vec3ToNitrousVec(position, &pos);
    dictator_52 = 1.0f;
    dictator_56 = 0.0f;
    dictator_60 = 0.0f;
    dictator.x.x = 1.0f;
    dictator.x.y = 0.0f;
    dictator.x.z = 0.0f;
    dictator.y.x = 0.0f;
    dictator.y.y = 1.0f;
    dictator.y.z = 0.0f;
    dictator.z.x = 0.0f;
    dictator.z.y = 0.0f;
    dictator.z.z = 1.0f;
    dictator.w.x = pos.x;
    dictator.w.y = pos.y;
    dictator.w.z = pos.z;
    dictator.w.x = pos.x - userData->cg2rb.w.x;
    dictator.w.y = pos.y - userData->cg2rb.w.y;
    dictator.w.z = pos.z - userData->cg2rb.w.z;
    //user_rigid_body::setPosition(body, &dictator);
    body->setPosition(&dictator);
}


bool __cdecl Phys_ObjIsAsleep(int id)
{
    const char *v1; // eax
    float _p1[3]; // [esp+2Ch] [ebp-1Ch] BYREF
    float _p0[3]; // [esp+38h] [ebp-10h] BYREF
    rigid_body *body; // [esp+44h] [ebp-4h]

    body = Phys_GetUserData(id)->body;
    //if ( phys_debugDangerousRigidBodies->current.enabled && rigid_body::is_dangerous(body) )
    if ( phys_debugDangerousRigidBodies->current.enabled && body->is_dangerous() )
    {
        //minspec_mutex::Lock(&g_render_mutex);
        g_render_mutex.Lock();
        Phys_NitrousVecToVec3(&body->m_last_position, _p0);
        _p1[0] = _p0[0];
        _p1[1] = _p0[1];
        _p1[2] = _p0[2] + 10000.0;
        CG_DebugLine(_p0, _p1, colorYellow, 1, 2000);
        v1 = va(
                     "The rigid body at (%6.2f, %6.2f, %6.2f) is about to freakout! Check if it was improperly placed in solid.",
                     _p0[0],
                     _p0[1],
                     _p0[2]);
        tlWarning(v1);
        //minspec_mutex::Unlock(&g_render_mutex);
        g_render_mutex.Unlock();
    }
    //return rigid_body::is_group_stable(body) || rigid_body::is_dangerous(body) || body->m_mat.w.z < -10000.0;
    return body->is_group_stable() || body->is_dangerous()|| body->m_mat.w.z < -10000.0;
}

unsigned int __thiscall rigid_body::is_dangerous()
{
    return this->m_flags & 0x80;
}

bool __cdecl Phys_ObjIsAsleepSingle(int id)
{
    const char *v1; // eax
    float _p1[3]; // [esp+2Ch] [ebp-1Ch] BYREF
    float _p0[3]; // [esp+38h] [ebp-10h] BYREF
    rigid_body *body; // [esp+44h] [ebp-4h]

    body = Phys_GetUserData(id)->body;
    //if ( phys_debugDangerousRigidBodies->current.enabled && rigid_body::is_dangerous(body) )
    if ( phys_debugDangerousRigidBodies->current.enabled && body->is_dangerous() )
    {
        //minspec_mutex::Lock(&g_render_mutex);
        g_render_mutex.Lock();
        Phys_NitrousVecToVec3(&body->m_last_position, _p0);
        _p1[0] = _p0[0];
        _p1[1] = _p0[1];
        _p1[2] = _p0[2] + 10000.0;
        CG_DebugLine(_p0, _p1, colorYellow, 1, 2000);
        v1 = va(
                     "The rigid body at (%6.2f, %6.2f, %6.2f) is about to freakout! Check if it was improperly placed in solid.",
                     _p0[0],
                     _p0[1],
                     _p0[2]);
        tlWarning(v1);
        //minspec_mutex::Unlock(&g_render_mutex);
        g_render_mutex.Unlock();
    }
    //return rigid_body::is_stable(body) || rigid_body::is_dangerous(body) || body->m_mat.w.z < -10000.0;
    return body->is_stable() || body->is_dangerous() || body->m_mat.w.z < -10000.0;
}

unsigned int __thiscall rigid_body::is_stable()
{
    return this->m_flags & 4;
}

void __cdecl Phys_SetHingeParams(rigid_body_constraint_ragdoll *id, float motorSpeed, float damp)
{
    if ( !id && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_main.cpp", 2900, 0, "%s", "id") )
        __debugbreak();
    //rigid_body_constraint_ragdoll::set_damp_k(id, damp * phys_ragdoll_joint_damp_scale->current.value);
    id->set_damp_k(damp * phys_ragdoll_joint_damp_scale->current.value);
}

rigid_body_constraint_ragdoll * Phys_CreateHinge(
                int obj1,
                int obj2,
                const float *anchor,
                float *axis,
                float motorSpeed,
                float damp,
                float lowStop,
                float highStop)
{
    phys_vec3 b2_ref_loc; // [esp+8h] [ebp-16Ch] BYREF
    float v11; // [esp+18h] [ebp-15Ch]
    int v12; // [esp+1Ch] [ebp-158h]
    int v13; // [esp+20h] [ebp-154h]
    phys_vec3 b1_ref_loc; // [esp+28h] [ebp-14Ch] BYREF
    float v15; // [esp+38h] [ebp-13Ch]
    int v16; // [esp+3Ch] [ebp-138h]
    int v17; // [esp+40h] [ebp-134h]
    phys_vec3 b2_axis_loc; // [esp+48h] [ebp-12Ch] BYREF
    const phys_vec3 *v19; // [esp+64h] [ebp-110h]
    phys_vec3 v20; // [esp+68h] [ebp-10Ch] BYREF
    phys_vec3 b1_axis_loc; // [esp+78h] [ebp-FCh] BYREF
    const phys_vec3 *v22; // [esp+94h] [ebp-E0h]
    phys_vec3 v23; // [esp+98h] [ebp-DCh] BYREF
    phys_vec3 b2_anchor_loc; // [esp+A8h] [ebp-CCh] BYREF
    phys_vec3 *v25; // [esp+C4h] [ebp-B0h]
    phys_vec3 v26; // [esp+C8h] [ebp-ACh] BYREF
    phys_vec3 b1_anchor_loc; // [esp+D8h] [ebp-9Ch] BYREF
    phys_vec3 *v28; // [esp+F4h] [ebp-80h]
    phys_vec3 v29; // [esp+F8h] [ebp-7Ch] BYREF
    phys_vec3 ref_abs; // [esp+108h] [ebp-6Ch]
    phys_vec3 axis_abs; // [esp+118h] [ebp-5Ch] BYREF
    float v32; // [esp+12Ch] [ebp-48h]
    float v33; // [esp+130h] [ebp-44h]
    float v34; // [esp+134h] [ebp-40h]
    phys_vec3 anchor_abs; // [esp+138h] [ebp-3Ch] BYREF
    float v36; // [esp+150h] [ebp-24h]
    float v37; // [esp+154h] [ebp-20h]
    float v38; // [esp+158h] [ebp-1Ch]
    rigid_body_constraint_ragdoll *joint; // [esp+15Ch] [ebp-18h]
    rigid_body *body2; // [esp+160h] [ebp-14h]
    rigid_body *body1; // [esp+164h] [ebp-10h]
    //_UNKNOWN *v42[2]; // [esp+168h] [ebp-Ch] BYREF
    //const float *anchora; // [esp+174h] [ebp+0h]
    //
    //v42[0] = a1;
    //v42[1] = anchora;
    body1 = Phys_GetUserData((int)obj1)->body;
    body2 = Phys_GetUserData(obj2)->body;
    joint = phys_sys::create_rbc_ragdoll((environment_rigid_body *)body1, (environment_rigid_body *)body2, 1);
    if (joint)
    {
        v38 = *anchor;
        v37 = anchor[1];
        v36 = anchor[2];
        anchor_abs.x = v38;
        anchor_abs.y = v37;
        anchor_abs.z = v36;
        v34 = *axis;
        v33 = axis[1];
        v32 = axis[2];
        axis_abs.x = v34;
        axis_abs.y = v33;
        axis_abs.z = v32;
        ref_abs.x = 1.0f;
        ref_abs.y = 0.0f;
        ref_abs.z = 0.0f;
        v28 = phys_full_inv_multiply(&v29, &body1->m_mat, &anchor_abs);
        *(_QWORD *)&b1_anchor_loc.x = *(_QWORD *)&v28->x;
        b1_anchor_loc.z = v28->z;
        v25 = phys_full_inv_multiply(&v26, &body2->m_mat, &anchor_abs);
        b2_anchor_loc.x = v25->x;
        b2_anchor_loc.y = v25->y;
        b2_anchor_loc.z = v25->z;
        v22 = phys_inv_multiply(&v23, &body1->m_mat, &axis_abs);
        b1_axis_loc.x = v22->x;
        b1_axis_loc.y = v22->y;
        b1_axis_loc.z = v22->z;
        v19 = phys_inv_multiply(&v20, &body2->m_mat, &axis_abs);
        b2_axis_loc.x = v19->x;
        b2_axis_loc.y = v19->y;
        b2_axis_loc.z = v19->z;
        v15 = 1.0f;
        v16 = 0.0f;
        v17 = 0.0f;
        b1_ref_loc.x = 1.0f;
        b1_ref_loc.y = 0.0f;
        b1_ref_loc.z = 0.0f;
        v11 = 1.0f;
        v12 = 0.0f;
        v13 = 0.0f;
        b2_ref_loc.x = 1.0f;
        b2_ref_loc.y = 0.0f;
        b2_ref_loc.z = 0.0f;
        //rigid_body_constraint_ragdoll::set(joint, &b1_anchor_loc, &b2_anchor_loc);
        joint->set(&b1_anchor_loc, &b2_anchor_loc);
        //rigid_body_constraint_ragdoll::set_hinge(
            joint->set_hinge(
            &b1_axis_loc,
            &b2_axis_loc,
            &b1_ref_loc,
            &b2_ref_loc,
            lowStop,
            highStop);
        //rigid_body_constraint_ragdoll::set_damp_k(joint, damp);
        joint->set_damp_k(damp);
        return joint;
    }
    else
    {
        Com_PrintWarning(20, "Physics: Out of hinge joints (%d max)\n", 256);
        return 0;
    }
}

void __cdecl Phys_SetAngularMotorParams(rigid_body_constraint_ragdoll *id, const float *motorSpeeds, float *damp)
{
    float v3; // [esp+4h] [ebp-Ch]

    if ( !id && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_main.cpp", 2946, 0, "%s", "id") )
        __debugbreak();
    if ( *damp <= damp[1] )
        v3 = damp[1];
    else
        v3 = *damp;
    //rigid_body_constraint_ragdoll::set_damp_k(id, v3 * phys_ragdoll_joint_damp_scale->current.value);
    id->set_damp_k(v3 * phys_ragdoll_joint_damp_scale->current.value);
}

rigid_body_constraint_ragdoll * Phys_CreateSwivel(
                int obj1,
                int obj2,
                const float *anchor,
                unsigned int numAxes,
                float (*axes)[3],
                const float *motorSpeeds,
                const float *motorFMaxs,
                float *lowStops,
                float *highStops)
{
    const phys_vec3 *v11; // eax
    const phys_vec3 *v12; // eax
    float v13; // xmm0_4
    float theta_max; // xmm0_4
    float z; // xmm0_4
    phys_vec3 rot_axes; // [esp+18h] [ebp-2BCh] BYREF
    phys_vec3 *p_y; // [esp+34h] [ebp-2A0h]
    phys_mat44 axes_loc; // [esp+38h] [ebp-29Ch] BYREF
    phys_vec3 b1_ud_loc; // [esp+78h] [ebp-25Ch] BYREF
    float angle_rotate; // [esp+90h] [ebp-244h]
    float angle_extents; // [esp+94h] [ebp-240h]
    phys_vec3 b1_ud_abs; // [esp+98h] [ebp-23Ch] BYREF
    float v23; // [esp+A8h] [ebp-22Ch]
    float v24; // [esp+ACh] [ebp-228h]
    float v25; // [esp+B0h] [ebp-224h]
    int i; // [esp+B4h] [ebp-220h]
    phys_vec3 v27; // [esp+B8h] [ebp-21Ch] BYREF
    float v28; // [esp+CCh] [ebp-208h]
    int v29; // [esp+D0h] [ebp-204h]
    int v30; // [esp+D4h] [ebp-200h]
    phys_vec3 v31; // [esp+D8h] [ebp-1FCh] BYREF
    int v32; // [esp+F4h] [ebp-1E0h]
    int v33; // [esp+F8h] [ebp-1DCh]
    int v34; // [esp+FCh] [ebp-1D8h]
    float max_angle; // [esp+100h] [ebp-1D4h]
    float v36; // [esp+104h] [ebp-1D0h]
    float min_angle; // [esp+108h] [ebp-1CCh]
    float v38; // [esp+10Ch] [ebp-1C8h]
    float v39; // [esp+110h] [ebp-1C4h]
    float limit; // [esp+114h] [ebp-1C0h]
    phys_vec3 b2_ref_loc; // [esp+118h] [ebp-1BCh]
    const phys_vec3 *v42; // [esp+134h] [ebp-1A0h]
    phys_vec3 v43; // [esp+138h] [ebp-19Ch] BYREF
    phys_vec3 b1_ref_loc; // [esp+148h] [ebp-18Ch]
    const phys_vec3 *v45; // [esp+164h] [ebp-170h]
    phys_vec3 v46; // [esp+168h] [ebp-16Ch] BYREF
    phys_vec3 b2_axis_loc; // [esp+178h] [ebp-15Ch] BYREF
    const phys_vec3 *v48; // [esp+194h] [ebp-140h]
    phys_vec3 v49; // [esp+198h] [ebp-13Ch] BYREF
    phys_vec3 b1_axis_loc; // [esp+1A8h] [ebp-12Ch] BYREF
    const phys_vec3 *v51; // [esp+1C4h] [ebp-110h]
    phys_vec3 v52; // [esp+1C8h] [ebp-10Ch] BYREF
    phys_vec3 b2_anchor_loc; // [esp+1D8h] [ebp-FCh] BYREF
    phys_vec3 v54; // [esp+1E8h] [ebp-ECh] BYREF
    phys_vec3 v55; // [esp+1F8h] [ebp-DCh] BYREF
    const phys_mat44 *v56; // [esp+214h] [ebp-C0h]
    phys_vec3 b1_anchor_loc; // [esp+218h] [ebp-BCh] BYREF
    phys_vec3 v58; // [esp+228h] [ebp-ACh] BYREF
    phys_vec3 v59; // [esp+238h] [ebp-9Ch] BYREF
    const phys_mat44 *p_m_mat; // [esp+254h] [ebp-80h]
    phys_vec3 ref_abs; // [esp+258h] [ebp-7Ch] BYREF
    float v62; // [esp+26Ch] [ebp-68h]
    float v63; // [esp+270h] [ebp-64h]
    float v64; // [esp+274h] [ebp-60h]
    phys_vec3 axis_abs; // [esp+278h] [ebp-5Ch] BYREF
    float v66; // [esp+28Ch] [ebp-48h]
    float v67; // [esp+290h] [ebp-44h]
    float v68; // [esp+294h] [ebp-40h]
    phys_vec3 anchor_abs; // [esp+298h] [ebp-3Ch] BYREF
    float v70; // [esp+2B0h] [ebp-24h]
    float v71; // [esp+2B4h] [ebp-20h]
    float v72; // [esp+2B8h] [ebp-1Ch]
    rigid_body_constraint_ragdoll *joint; // [esp+2BCh] [ebp-18h]
    rigid_body *body2; // [esp+2C0h] [ebp-14h]
    rigid_body *body1; // [esp+2C4h] [ebp-10h]
    //_UNKNOWN *v76[2]; // [esp+2C8h] [ebp-Ch] BYREF
    //const float *anchora; // [esp+2D4h] [ebp+0h]
    //
    //v76[0] = a1;
    //v76[1] = anchora;
    body1 = Phys_GetUserData(obj1)->body;
    body2 = Phys_GetUserData(obj2)->body;
    if ((unsigned int)numAxes >= 4
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_main.cpp",
            2958,
            0,
            "%s",
            "numAxes >= 0 && numAxes <= 3"))
    {
        __debugbreak();
    }
    joint = phys_sys::create_rbc_ragdoll((environment_rigid_body *)body1, (environment_rigid_body *)body2, 1);
    if (joint)
    {
        v72 = *anchor;
        v71 = anchor[1];
        v70 = anchor[2];
        anchor_abs.x = v72;
        anchor_abs.y = v71;
        anchor_abs.z = v70;
        v68 = (*axes)[0];
        v67 = (*axes)[1];
        v66 = (*axes)[2];
        axis_abs.x = v68;
        axis_abs.y = v67;
        axis_abs.z = v66;
        v64 = (*axes)[3];
        v63 = (*axes)[4];
        v62 = (*axes)[5];
        ref_abs.x = v64;
        ref_abs.y = v63;
        ref_abs.z = v62;
        p_m_mat = &body1->m_mat;
        v11 = &v59;
        //v11 = operator-(&v59, &anchor_abs, &body1->m_mat.w);
        v59 = anchor_abs - body1->m_mat.w;
        phys_inv_multiply(&v58, p_m_mat, v11);
        b1_anchor_loc.x = v58.x;
        b1_anchor_loc.y = v58.y;
        b1_anchor_loc.z = v58.z;
        v56 = &body2->m_mat;
        v12 = &v55;
        v55 = anchor_abs - body2->m_mat.w;
        //v12 = operator-(&v55, &anchor_abs, &body2->m_mat.w);
        phys_inv_multiply(&v54, v56, v12);
        b2_anchor_loc.x = v54.x;
        b2_anchor_loc.y = v54.y;
        b2_anchor_loc.z = v54.z;
        v51 = phys_inv_multiply(&v52, &body1->m_mat, &axis_abs);
        b1_axis_loc.x = v51->x;
        b1_axis_loc.y = v51->y;
        b1_axis_loc.z = v51->z;
        v48 = phys_inv_multiply(&v49, &body2->m_mat, &axis_abs);
        b2_axis_loc.x = v48->x;
        b2_axis_loc.y = v48->y;
        b2_axis_loc.z = v48->z;
        v45 = phys_inv_multiply(&v46, &body1->m_mat, &ref_abs);
        b1_ref_loc.x = v45->x;
        b1_ref_loc.y = v45->y;
        b1_ref_loc.z = v45->z;
        v42 = phys_inv_multiply(&v43, &body2->m_mat, &ref_abs);
        *(_QWORD *)&b2_ref_loc.x = *(_QWORD *)&v42->x;
        b2_ref_loc.z = v42->z;
        //rigid_body_constraint_ragdoll::set(joint, &b1_anchor_loc, &b2_anchor_loc);
        joint->set(&b1_anchor_loc, &b2_anchor_loc);
        limit = 1.2f;
        //LODWORD(v39) = LODWORD(FLOAT_1_2) ^ _mask__NegFloat_;
        (v39) = -(1.2f);
        //if (*lowStops <= COERCE_FLOAT(LODWORD(FLOAT_1_2) ^ _mask__NegFloat_))
        if (*lowStops <= -1.2f)
            v13 = v39;
        else
            v13 = *lowStops;
        v38 = v13;
        min_angle = v13;
        if (limit <= *highStops)
            theta_max = limit;
        else
            theta_max = *highStops;
        v36 = theta_max;
        max_angle = theta_max;
        v34 = -(b2_ref_loc.x);
        v33 = -(b2_ref_loc.y);
        v32 = -(b2_ref_loc.z);
        (v31.x) = -(b2_ref_loc.x);
        (v31.y) = -(b2_ref_loc.y);
        (v31.z) = -(b2_ref_loc.z);

        (v27.x) = -(b1_ref_loc.x);
        (v27.y) = -(b1_ref_loc.y);
        (v27.z) = -(b1_ref_loc.z);
        //rigid_body_constraint_ragdoll::set_swivel(
            joint->set_swivel(
            &b1_axis_loc,
            &b2_axis_loc,
            &v27,
            &v31,
            min_angle,
            theta_max);
        for (i = 1; i < (int)numAxes; ++i)
        {
            v25 = (*axes)[0];
            v24 = (*axes)[1];
            v23 = (*axes)[2];
            b1_ud_abs.x = v25;
            b1_ud_abs.y = v24;
            b1_ud_abs.z = v23;
            angle_extents = (float)(highStops[i] - lowStops[i]) * 0.5;
            angle_rotate = highStops[i] + lowStops[i];
            phys_inv_multiply(&b1_ud_loc, &body1->m_mat, &b1_ud_abs);
            Phys_AxisToNitrousMat(axes, &axes_loc);
            phys_inv_multiply_mat(&axes_loc, &body1->m_mat, &axes_loc);
            if (i == 1)
            {
                p_y = &axes_loc.y;
                rot_axes.x = axes_loc.y.x;
                rot_axes.y = axes_loc.y.y;
                z = axes_loc.y.z;
            }
            else
            {
                rot_axes.x = axes_loc.z.x;
                rot_axes.y = axes_loc.z.y;
                z = axes_loc.z.z;
            }
            rot_axes.z = z;
            make_rotate(&axes_loc, &rot_axes, angle_rotate, 1000.0);
            //rigid_body_constraint_ragdoll::add_joint_limit(joint, &b1_ud_loc, angle_extents);
            joint->add_joint_limit(&b1_ud_loc, angle_extents);
        }
        return joint;
    }
    else
    {
        Com_PrintWarning(20, "Physics: Out of hinge joints (%d max)\n", 256);
        return 0;
    }
}

void phys_inv_multiply_mat(phys_mat44 *dest, const phys_mat44 *left, const phys_mat44 *right)
{
    if ( dest == left )
    {
        phys_mat44 temp;
        memcpy(&temp, left, sizeof(phys_mat44));
        phys_inv_multiply_mat(dest, &temp, right);
        return;
    }
    else
    {
        phys_vec3 tmp;

        phys_inv_multiply(&tmp, left, &right->x);
        dest->x.x = tmp.x;
        dest->x.y = tmp.y;
        dest->x.z = tmp.z;

        phys_inv_multiply(&tmp, left, &right->y);
        dest->y.x = tmp.x;
        dest->y.y = tmp.y;
        dest->y.z = tmp.z;

        phys_inv_multiply(&tmp, left, &right->z);
        dest->z.x = tmp.x;
        dest->z.y = tmp.y;
        dest->z.z = tmp.z;
    }
}

void __cdecl Phys_CalcPreset(PhysPreset *physPreset, float *dims, int surfaceType)
{
    float v3; // [esp+0h] [ebp-24h]
    float mass; // [esp+10h] [ebp-14h]
    float v5; // [esp+14h] [ebp-10h]
    float size; // [esp+18h] [ebp-Ch]
    bool floats; // [esp+1Fh] [ebp-5h]
    float density; // [esp+20h] [ebp-4h]

    if ( !physPreset
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\physics\\phys_main.cpp", 3019, 0, "%s", "physPreset") )
    {
        __debugbreak();
    }
    density = Com_SurfaceDensity(surfaceType);
    size = (float)(*dims + dims[1]) + dims[2];
    floats = Com_SurfaceFloats(surfaceType);
    if ( physPreset->bounce == 0.0 )
        physPreset->bounce = 0.25f;
    if ( physPreset->friction == 0.0 )
        physPreset->friction = 1.0f;
    physPreset->bulletForceScale = 1.0f;
    physPreset->explosiveForceScale = 1.0f;
    physPreset->canFloat = floats;
    if ( surfaceType == 8 )
        physPreset->gravityScale = 0.3f;
    else
        physPreset->gravityScale = 1.0f;
    if ( physPreset->mass == 0.0 )
    {
        physPreset->mass = size * density;
        mass = physPreset->mass;
        if ( (float)(mass - 1000.0) < 0.0 )
            v5 = physPreset->mass;
        else
            v5 = 1000.0f;
        if ( (float)(1.0 - mass) < 0.0 )
            v3 = v5;
        else
            v3 = 1.0f;
        physPreset->mass = v3;
    }
    physPreset->mass = physPreset->mass * 0.001;
    physPreset->buoyancyBoxMin[0] = 0.0f;
    physPreset->buoyancyBoxMin[1] = 0.0f;
    physPreset->buoyancyBoxMin[2] = 0.0f;
    physPreset->buoyancyBoxMax[0] = 0.0f;
    physPreset->buoyancyBoxMax[1] = 0.0f;
    physPreset->buoyancyBoxMax[2] = 0.0f;
    physPreset->centerOfMassOffset[0] = 0.0f;
    physPreset->centerOfMassOffset[1] = 0.0f;
    physPreset->centerOfMassOffset[2] = 0.0f;
    physPreset->flags |= 2u;
}

int __cdecl Phys_GetCurrentTime()
{
    return physGlob.timeLastUpdate;
}

cdl_proftimer::cdl_proftimer()
{
    int i; // [esp+4h] [ebp-4h]

    this->value = 0;
    this->avr = 0;
    this->capture_count = 0;
    for ( i = 0; i < 5; ++i )
    {
        LODWORD(this->mx[i]) = 0;
        HIDWORD(this->mx[i]) = 0;
    }
    this->capture = 0;
}

PhysGlob::PhysGlob()
{
    int v2; // [esp+4h] [ebp-8h]
    phys_link_list1<PhysObjUserData> *i; // [esp+8h] [ebp-4h]

    this->objects.m_dummy_head.m_next_T_internal = &this->objects.m_dummy_head;
    this->objects.m_dummy_head.m_prev_T_internal = &this->objects.m_dummy_head;
    this->objects.m_list_count = 0;
    this->objects.m_list_count_high_water = 0;
    this->objects.m_ptr_list_count = 0;
    v2 = 3;
    for ( i = this->objects_by_world; --v2 >= 0; ++i )
    {
        i->m_first = 0;
        i->m_last = 0;
        i->m_alloc_count = 0;
    }
}