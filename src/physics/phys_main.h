#pragma once
#include <universal/dvar.h>

#include "phys_collision.h"
#include "phys_colgeom.h"
#include "phys_gjk_collision_detection.h"
#include "physpreset_load_obj.h"
#include <gfx_d3d/r_workercmds.h>

enum PhysicsGeomType : __int32
{                                       // XREF: BoneDef/r
                                        // ?Phys_CreateUserBody@@YAHQAMHW4PhysicsGeomType@@@Z/r
    PHYS_GEOM_NONE           = 0x0,
    PHYS_GEOM_BOX            = 0x1,
    PHYS_GEOM_BRUSH          = 0x2,
    PHYS_GEOM_CYLINDER       = 0x3,
    PHYS_GEOM_CYLINDER_LARGE = 0x4,
    PHYS_GEOM_CAPSULE        = 0x5,
    PHYS_GEOM_POINT          = 0x6,
    PHYS_GEOM_COUNT          = 0x7,
};
inline PhysicsGeomType &operator++(PhysicsGeomType &t)
{
    t = static_cast<PhysicsGeomType>((static_cast<int>(t) + 1));
    return t;
}
inline PhysicsGeomType operator++(PhysicsGeomType &t, int)
{
    PhysicsGeomType old = t;
    t = static_cast<PhysicsGeomType>((static_cast<int>(t) + 1));
    return old;
}

enum Phys_SurfaceType : __int32
{                                       // XREF: Phys_GetSoundAliasIndex/r
    EFFECT_SURF_TYPE_HARD    = 0x0,     // XREF: .data:remap_table/s
    EFFECT_SURF_TYPE_SOFT    = 0x1,     // XREF: .data:remap_table/s
    EFFECT_SURF_TYPE_METAL   = 0x2,     // XREF: .data:remap_table/s
    EFFECT_SURF_TYPE_GLASS   = 0x3,     // XREF: .data:remap_table/s
    EFFECT_SURF_TYPE_WATER   = 0x4,     // XREF: .data:remap_table/s
    EFFECT_SURF_TYPE_WOOD    = 0x5,     // XREF: .data:remap_table/s
    EFFECT_SURF_TYPE_FOLIAGE = 0x6,     // XREF: .data:remap_table/s
    EFFECT_SURF_TYPE_FLESH   = 0x7,     // XREF: .data:remap_table/s
    EFFECT_SURF_TYPE_COUNT   = 0x8,
};
inline Phys_SurfaceType &operator++(Phys_SurfaceType &t)
{
    t = static_cast<Phys_SurfaceType>((static_cast<int>(t) + 1));
    return t;
}
inline Phys_SurfaceType operator++(Phys_SurfaceType &t, int)
{
    Phys_SurfaceType old = t;
    t = static_cast<Phys_SurfaceType>((static_cast<int>(t) + 1));
    return old;
}

struct BodyState // sizeof=0x88
{                                       // XREF: Phys_ObjCreateAxis/r
    float position[3];                  // XREF: Phys_ObjCreateNitrousVehicle(float const * const,float const * const,float const * const,float const * const,PhysPreset const *,gjk_geom_list_t *)+1A5/w
    float rotation[3][3];               // XREF: Phys_ObjCreateNitrousVehicle(float const * const,float const * const,float const * const,float const * const,PhysPreset const *,gjk_geom_list_t *)+18B/o
    float velocity[3];                  // XREF: Phys_ObjCreateNitrousVehicle(float const * const,float const * const,float const * const,float const * const,PhysPreset const *,gjk_geom_list_t *)+1D4/w
    float angVelocity[3];               // XREF: Phys_ObjCreateNitrousVehicle(float const * const,float const * const,float const * const,float const * const,PhysPreset const *,gjk_geom_list_t *)+1FB/w
    float centerOfMassOffset[3];        // XREF: Phys_ObjCreateNitrousVehicle(float const * const,float const * const,float const * const,float const * const,PhysPreset const *,gjk_geom_list_t *)+222/w
    float buoyancyBoxMin[3];            // XREF: Phys_ObjCreateNitrousVehicle(float const * const,float const * const,float const * const,float const * const,PhysPreset const *,gjk_geom_list_t *)+249/w
    float buoyancyBoxMax[3];            // XREF: Phys_ObjCreateNitrousVehicle(float const * const,float const * const,float const * const,float const * const,PhysPreset const *,gjk_geom_list_t *)+270/w
    float mass;                         // XREF: Phys_ObjCreateNitrousVehicle(float const * const,float const * const,float const * const,float const * const,PhysPreset const *,gjk_geom_list_t *)+297/w
    float friction;                     // XREF: Phys_ObjCreateNitrousVehicle(float const * const,float const * const,float const * const,float const * const,PhysPreset const *,gjk_geom_list_t *)+2B1/w
    float bounce;                       // XREF: Phys_ObjCreateNitrousVehicle(float const * const,float const * const,float const * const,float const * const,PhysPreset const *,gjk_geom_list_t *)+2A4/w
    int timeLastAsleep;                 // XREF: Phys_ObjCreateNitrousVehicle(float const * const,float const * const,float const * const,float const * const,PhysPreset const *,gjk_geom_list_t *)+2BC/w
    int id;                             // XREF: Phys_ObjCreateNitrousVehicle(float const * const,float const * const,float const * const,float const * const,PhysPreset const *,gjk_geom_list_t *)+304/w
    int buoyancy;                       // XREF: Phys_ObjCreateNitrousVehicle(float const * const,float const * const,float const * const,float const * const,PhysPreset const *,gjk_geom_list_t *)+2DE/w
    int underwater;                     // XREF: Phys_ObjCreateNitrousVehicle(float const * const,float const * const,float const * const,float const * const,PhysPreset const *,gjk_geom_list_t *):loc_81FDAD/w
};

struct PhysImpulse // sizeof=0x1C
{                                       // XREF: .data:PhysImpulse * gImpulseCache/r
    int id;
    float hitp[3];
    float hitd[3];
};

struct PhysObjUserData // sizeof=0x130
{                                       // XREF: phys_free_list<PhysObjUserData>::T_internal/r
    rigid_body *body;
    struct NitrousVehicle *vehicle;
    gjk_geom_list_t m_gjk_geom_list;
    PhysObjUserData *m_next_link;
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
    phys_mat44 cg2rb;
    phys_mat44 cg2w;
    broad_phase_base *m_bpb;
    float savedPos[3];
    float savedRot[3][3];
    float friction;
    float bounce;
    int id;
    int refcount;
    int underwater;
    int buoyancy;
    int timeBuoyant;
    int timeRipple;
    unsigned __int16 trDuration;
    // padding byte
    // padding byte
    float m_time_since_last_event;
    float m_time_since_last_reeval;
    int m_flags;
    float mass;
    float buoyancyBoxMin[3];
    float buoyancyBoxMax[3];
    float centerOfMassOffset[3];
    // padding byte
    // padding byte
    // padding byte
    // padding byte

    // phys_link_list1 accessors
    void set_next_link(PhysObjUserData *const p) { m_next_link = p; }
    PhysObjUserData *get_next_link() const { return m_next_link; }
};

static_assert(sizeof(phys_surface_type_info) == 16);


struct PhysGlob // sizeof=0x480
{                                       // XREF: .data:PhysGlob physGlob/r
    PhysGlob();

    phys_free_list<PhysObjUserData> objects;
    phys_link_list1<PhysObjUserData> objects_by_world[3];
    PhysObjUserData *userRigidBodies[16];
    int timeLastSnapshot;               // XREF: Phys_Init(void)+6/w
    int timeLastUpdate;                 // XREF: Phys_ObjCreateNitrousVehicle(float const * const,float const * const,float const * const,float const * const,PhysPreset const *,gjk_geom_list_t *)+2B6/r
};

struct cdl_proftimer // sizeof=0x60
{                                       // XREF: .data:cdl_proftimer proftimer_physics_frame_advance/r
                                        // .data:cdl_proftimer cl_flame_proftimer/r ...
    unsigned __int64 stamp;             // XREF: Phys_RunToTime(int)+B/w
                                        // Phys_RunToTime(int)+10/w ...
    unsigned __int64 value;             // XREF: Phys_RunToTime(int)+1A4/r
                                        // Phys_RunToTime(int)+1AA/r ...
    unsigned __int64 avr;               // XREF: CG_DrawFPS(int,ScreenPlacement const *,float,meminfo_t *)+CE9/r
                                        // CG_DrawFPS(int,ScreenPlacement const *,float,meminfo_t *)+CEF/r ...
    unsigned __int64 tot;
    unsigned __int64 capture_count;
    unsigned __int64 calls;             // XREF: CG_DrawFPS(int,ScreenPlacement const *,float,meminfo_t *)+D9B/r
                                        // CG_DrawFPS(int,ScreenPlacement const *,float,meminfo_t *)+DA2/r ...
    bool capture;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    unsigned __int64 mx[5];


    cdl_proftimer();

    void start_capture();

    void reset();
};

struct cbrush_t;
struct CollisionAabbTree;
struct cLeafBrushNode_s;
struct XModel;

void __cdecl Phys_Init();
void __cdecl create_broad_phase_info(rigid_body *body);
void __cdecl destroy_broad_phase_info(rigid_body *body);
PhysObjUserData * Phys_CreateUserBody(
                float *position,
                int id,
                PhysicsGeomType geomType);
PhysObjUserData *__cdecl Phys_CreateUserData(int worldIndex);
PhysObjUserData * Phys_CreateBodyFromState(
                int worldIndex,
                const BodyState *state,
                gjk_geom_list_t *gjk_geom_list,
                bool do_collision_test);
PhysObjUserData *__cdecl Phys_ObjCreate(
                int worldIndex,
                const float *position,
                const float *quat,
                const float *velocity,
                const PhysPreset *physPreset,
                gjk_geom_list_t *gjk_geom_list,
                bool do_collision_test,
                int id);
PhysObjUserData *__cdecl Phys_ObjCreateAxis(
                int worldIndex,
                const float *position,
                const float (*axis)[3],
                const float *velocity,
                const PhysPreset *physPreset,
                gjk_geom_list_t *gjk_geom_list,
                int id,
                bool do_collision_test);
void    Phys_ObjSetPosition(int id, float *newPosition);
void __cdecl Phys_ObjSetOrientation(int id, const float *newPosition, const float *newOrientation);
void    Phys_ObjSetAngularVelocity(int id, float *angularVel);
void    Phys_ObjSetVelocity(int id, float *velocity);
void    Phys_ObjSetAngularVelocityRaw(int id, float *angularVel);
void    Phys_ObjGetPosition(int id, float *outPosition, float (*outRotation)[3]);
void __cdecl Phys_ObjGetVelocities(int id, float *tvel, float *avel);
void __cdecl Phys_ObjGetCenterOfMass(int id, float *outPosition);
void __cdecl Phys_ObjAddCollFlags(int physObjId, int collFlags);
void __cdecl Phys_ObjRemoveCollFlags(int physObjId, int collFlags);
void __cdecl fixup_wheel_constraints(rigid_body *rb);
void __cdecl Phys_ObjDestroy(int worldIndex, int id);
void __cdecl Phys_DestroyUserData(int worldIndex, PhysObjUserData *userData);
void    Phys_AddCacheImpulses();
void __cdecl Phys_ObjAddCustomForce(
                int physObjId,
                const float *hitPos,
                const float *hitDir,
                int mod,
                PhysPreset *physPreset);
void    Phys_ObjAddForce(int id, float *worldPos, float *impulse, bool relative);
void    Phys_ObjAddTorque(int id, float *torque);
void __cdecl Phys_ObjBulletImpact(
                int id,
                const float *worldPosRaw,
                const float *bulletDirRaw,
                float bulletSpeed,
                float scale);
void __cdecl Phys_TweakBulletImpact(float *worldPos, float *bulletDir, const float *centerOfMass);
int __cdecl Phys_ObjGetSnapshot(int id, float *outPos, float (*outMat)[3]);
void __cdecl Phys_RenderBulletMeshInfo(
                float *pos,
                const XModel *model,
                const char *type,
                int numTriangles,
                float avgArea);
void __cdecl Phys_ComputeStringPosition(const float *start, const float *end, float *position);
void __cdecl Phys_FindAndRenderBulletMesh(const float *start, const float *end);

void Phys_FindAndRenderEntityBrushes(const float *pos, int contentmask);
char __cdecl Phys_DebugRenderChanged();
int __cdecl Phys_GetMaskFromDVar();
void __cdecl Phys_RenderWorldCollMesh(const float *pos, bool bRenderStaticCollision, bool bRenderBrushes);

void __cdecl debug_loop();
void __cdecl print_smodel(struct cStaticModel_s *smodel, int refcount, const float *dims);
void __cdecl print_static_models_stats();
int __cdecl SortModelsFunc(const void *a, const void *b);
void __cdecl draw_static_models_bounds();
int __cdecl SortSModelsByDist(float *a, float *b);
int __cdecl buoyancy_worker();
void __cdecl Phys_BodyGrabSnapshotNitrous(PhysObjUserData *userData, float deltaT);
void    UpdateRigidBody(float delta_t);
void __cdecl Phys_DebugRender(PhysObjUserData *userData);
void    Phys_CollisionCallback();
bool __cdecl Phys_ShouldCollideCallback(const broad_phase_base *bpi1, const broad_phase_base *bpi2);
void __cdecl PhysicsSystem_Update();
void __cdecl Phys_RunToTime(int timeNow);
void __cdecl Phys_ObjGetInterpolatedState(int id, float *outPos, float *outQuat);
void    Phys_SetUserBody(int id, float *position);
bool __cdecl Phys_ObjIsAsleep(int id);
bool __cdecl Phys_ObjIsAsleepSingle(int id);

void __cdecl Phys_SetHingeParams(rigid_body_constraint_ragdoll *id, float motorSpeed, float damp);
rigid_body_constraint_ragdoll * Phys_CreateHinge(
                int obj1,
                int obj2,
                const float *anchor,
                float *axis,
                float motorSpeed,
                float damp,
                float lowStop,
                float highStop);
void __cdecl Phys_SetAngularMotorParams(rigid_body_constraint_ragdoll *id, const float *motorSpeeds, float *damp);
rigid_body_constraint_ragdoll * Phys_CreateSwivel(
                int obj1,
                int obj2,
                const float *anchor,
                unsigned int numAxes,
                float (*axes)[3],
                const float *motorSpeeds,
                const float *motorFMaxs,
                float *lowStops,
                float *highStops);
void __cdecl Phys_CalcPreset(PhysPreset *physPreset, float *dims, int surfaceType);
int __cdecl Phys_GetCurrentTime();

extern const dvar_t *phys_gravity;
extern const dvar_t *phys_gravity_dir;
extern const dvar_t *phys_vehicleGravityMultiplier;
extern const dvar_t *phys_vehicleDamageFroceScale;
extern const dvar_t *phys_vehicleUsePredictedPosition;
extern const dvar_t *phys_bulletUpBias;
extern const dvar_t *phys_bulletSpinScale;
extern const dvar_t *phys_msecStep;
extern const dvar_t *phys_drawcontacts;
extern const dvar_t *phys_drawCollisionObj;
extern const dvar_t *phys_debugBigQueries;
extern const dvar_t *phys_debugCallback;
extern const dvar_t *phys_debugDangerousRigidBodies;
extern const dvar_t *phys_debugExpensivePushout;
extern const dvar_t *phys_drawNitrousVehicle;
extern const dvar_t *phys_drawNitrousVehicleEffects;
extern const dvar_t *phys_entityCollision;
extern const dvar_t *phys_vehicleWheelEntityCollision;
extern const dvar_t *phys_vehicleFriction;
extern const dvar_t *phys_ragdoll_joint_damp_scale;
extern const dvar_t *phys_dragLinear;
extern const dvar_t *phys_dragAngular;
extern const dvar_t *phys_userRigidBodies;
extern const dvar_t *phys_waterDragLinear;
extern const dvar_t *phys_waterDragAngular;
extern const dvar_t *phys_maxFloatTime;
extern const dvar_t *phys_buoyancyDistanceCutoff;
extern const dvar_t *phys_piecesSpawnDistanceCutoff;
extern const dvar_t *phys_floatTimeVariance;
extern const dvar_t *phys_buoyancyRippleFrequency;
extern const dvar_t *phys_buoyancyRippleVariance;
extern const dvar_t *phys_buoyancyFastComputation;
extern const dvar_t *phys_buoyancy;
extern const dvar_t *phys_ragdoll_buoyancy;
extern const dvar_t *debug_trace;
extern const dvar_t *g_bDebugRenderBulletMeshes;
extern const dvar_t *g_bDebugRenderEntityBrushes;
extern const dvar_t *g_bDebugRenderPatches;
extern const dvar_t *g_bDebugRenderBrushes;
extern const dvar_t *g_bDebugRenderColoredPatches;
extern const dvar_t *g_debugRenderMask;
extern const dvar_t *g_debugRenderCollisionDistance;
extern const dvar_t *g_bDebugRenderStaticModelsBounds;
extern const dvar_t *g_dumpStaticModels;
extern const dvar_t *g_debugRenderGjkTraceGeom;
extern const dvar_t *phys_player_collision_mode;
extern const dvar_t *phys_player_collision_adjust_height;
extern const dvar_t *phys_ai_collision_mode;
extern const dvar_t *render_player_collision;
extern const dvar_t *render_actor_collision;
extern const dvar_t *render_bpi_env_collision;
extern const dvar_t *enable_moving_paths;
extern const dvar_t *enable_new_prone_check;
extern const dvar_t *phys_heavyTankSwitch;
extern const dvar_t *phys_fluid;

extern cdl_proftimer proftimer_physics_frame_advance;
extern PhysGlob physGlob;
extern minspec_mutex g_render_mutex;

extern int g_debug_partition;
