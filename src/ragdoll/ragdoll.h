#pragma once
#include <xanim/dobj.h>
#include <bgame/bg_local.h>
#include <physics/phys_collision.h>
#include <physics/phys_main.h>
#include <qcommon/common.h>

#define RAGDOLL_MAX_DEFS 2

#define RAGDOLL_DOBJ_VALID_STATE 2

enum RagdollBodyState : __int32
{                                       // XREF: RagdollBody/r
    BS_DEAD             = 0x0,
    BS_DOBJ_WAIT        = 0x1,
    BS_VELOCITY_CAPTURE = 0x2,
    BS_TUNNEL_TEST      = 0x3,
    BS_RUNNING          = 0x4,
    BS_IDLE             = 0x5,
    RAGDOLL_NUM_STATES  = 0x6,
};

enum JointType : __int32
{                                       // XREF: JointDef/r
    RAGDOLL_JOINT_NONE   = 0x0,
    RAGDOLL_JOINT_HINGE  = 0x1,
    RAGDOLL_JOINT_SWIVEL = 0x2,
};
inline JointType &operator++(JointType &t)
{
    t = static_cast<JointType>((static_cast<int>(t) + 1));
    return t;
}
inline JointType operator++(JointType &t, int)
{
    JointType old = t;
    t = static_cast<JointType>((static_cast<int>(t) + 1));
    return old;
}

struct BaseLerpBoneDef // sizeof=0x20
{                                       // XREF: RagdollDef/r
    char animBoneTextName[20];
    unsigned int animBoneName;
    unsigned int parentBoneIndex;
    int lerpTime;
};

struct BoneDef // sizeof=0x50
{                                       // XREF: RagdollDef/r
    char animBoneTextNames[2][20];
    unsigned int animBoneNames[2];
    float radius;
    bool mirror;
    // padding byte
    // padding byte
    // padding byte
    float percent;
    float mass;
    float friction;
    float buoyancy;
    int parentBone;
    PhysicsGeomType geomType;
};

struct SelfPairDef // sizeof=0x2
{                                       // XREF: RagdollDef/r
    unsigned __int8 bones[2];
};

struct JointDef // sizeof=0x54
{                                       // XREF: RagdollDef/r
    unsigned __int8 bone;
    // padding byte
    // padding byte
    // padding byte
    JointType type;
    int numLimitAxes;
    float limitAxes[3][3];
    float minAngles[3];
    float maxAngles[3];
    float axisFriction[3];
};

struct __declspec(align(4)) RagdollDef // sizeof=0xF08
{
    bool inUse;
    bool bound;
    // padding byte
    // padding byte
    int numBones;
    BoneDef boneDefs[14];
    int numBaseLerpBones;
    BaseLerpBoneDef baseLerpBoneDefs[9];
    int numJoints;
    JointDef jointDefs[28];
    int numSelfPairs;
    SelfPairDef selfPairDefs[33];
    // padding byte
    // padding byte
};

struct Bone // sizeof=0x1C
{                                       // XREF: RagdollBody/r
    int parentBone;
    unsigned __int8 animBones[2];
    // padding byte
    // padding byte
    int rigidBody;
    float length;
    float center[3];
};

struct BoneOrientation // sizeof=0x20
{                                       // XREF: RagdollBody/r
    float origin[3];
    int boneFlags;
    float orientation[4];
};

struct __declspec(align(4)) LerpBone // sizeof=0x8
{                                       // XREF: RagdollBody/r
    int parentBone;
    unsigned __int8 animBone;
    // padding byte
    // padding byte
    // padding byte
};

struct Joint // sizeof=0x8
{                                       // XREF: RagdollBody/r
                                        // Ragdoll_DebugRender/r
    int joint;
    int joint2;                         // XREF: Ragdoll_DebugRender+10/w
};

struct RagdollBody // sizeof=0xA28
{                                       // XREF: phys_free_list<RagdollBody>::T_internal/r
    int references;
    int ragdollDef;
    int dobj;
    DObj *obj;
    const cpose_t *pose;
    float poseOffset[3];
    int localClientNum;
    RagdollBodyState state;
    int stateMsec;
    int stateFrames;
    int velCaptureMsec;
    int stableTime;
    int numBones;
    Bone bones[14];
    int numLerpBones;
    LerpBone lerpBones[9];
    int numJoints;
    Joint joints[28];
    int curOrientationBuffer;
    BoneOrientation boneOrientations[2][23];
    BoneOrientation lerpBoneOffsets[9];
    int flags;
    int storedLaunchCount;
    hitLocation_t storedLaunchHitLoc[2];
    float storedLaunchForce[2][3];
    hitLocation_t storedAttachHitLoc;
    int storedAttachEnt;
    int storedPhysObjId;
    rigid_body_constraint_point *debug_hang_point;
    rigid_body_constraint_point *hang_point;
    int start_hang_time;
    broad_phase_group *m_bpg;
    broad_phase_collision_pair *m_list_bpcp;
    int rope_id;
    void *rbc_dist;
};

void __cdecl Ragdoll_DebugDraw();
bool __cdecl Ragdoll_BodyInUse(RagdollBody *body);
RagdollDef *__cdecl Ragdoll_BodyDef(RagdollBody *body);
DObj *__cdecl Ragdoll_BodyDObj(RagdollBody *body);
const cpose_t *__cdecl Ragdoll_BodyPose(RagdollBody *body);
void __cdecl Ragdoll_BodyRootOrigin(RagdollBody *body, float *origin);
bool __cdecl Ragdoll_BodyPoseValid(RagdollBody *body);
void __cdecl Ragdoll_GetRootOrigin(int ragdollHandle, float *origin);
int __cdecl Ragdoll_CountPhysicsBodies();
bool __cdecl Ragdoll_BodyHasPhysics(RagdollBody *body);
const RagdollBody *__cdecl Ragdoll_CreateRagdollForDObj(
                int localClientNum,
                int ragdollDef,
                int dobj,
                bool reset,
                bool share);
RagdollBody *__cdecl Ragdoll_GetUnusedBody();
void __cdecl Ragdoll_InitBody(RagdollBody *body);
RagdollBody *__cdecl Ragdoll_ReferenceDObjBody(int dobj);
char __cdecl Ragdoll_BindDef(unsigned int ragdollDef);
bool __cdecl Ragdoll_ValidateDef(unsigned int ragdollDef);
void __cdecl Ragdoll_Remove(int ragdoll);
void __cdecl Ragdoll_FreeBody(int ragdollBody);
void __cdecl Ragdoll_InitDvars();
void __cdecl Ragdoll_Register();
void __cdecl Ragdoll_Init();
void __cdecl Ragdoll_Shutdown();

inline RagdollBody *__cdecl Ragdoll_HandleBody(int handle)
{
    return (RagdollBody *)handle;
}

extern const dvar_t *ragdoll_enable;
extern const dvar_t *ragdoll_debug;
extern const dvar_t *ragdoll_fps;
extern const dvar_t *ragdoll_max_life;
extern const dvar_t *ragdoll_max_simulating;
extern const dvar_t *ragdoll_explode_force;
extern const dvar_t *ragdoll_explode_upbias;
extern const dvar_t *ragdoll_bullet_force;
extern const dvar_t *ragdoll_bullet_upbias;
extern const dvar_t *ragdoll_baselerp_time;
extern const dvar_t *ragdoll_jointlerp_time;
extern const dvar_t *ragdoll_rotvel_scale;
extern const dvar_t *ragdoll_jitter_scale;
extern const dvar_t *ragdoll_self_collision_scale;
extern const dvar_t *ragdoll_dump_anims;

extern phys_free_list<RagdollBody> g_ragdoll_body_pool;
extern RagdollDef ragdollDefs[RAGDOLL_MAX_DEFS];

extern bool ragdollInited;