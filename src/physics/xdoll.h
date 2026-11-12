#pragma once
#include "phys_collision.h"
#include "physconstraints_load_obj.h"

struct XDollBone // sizeof=0x10
{                                       // XREF: XDollBody/r
    unsigned __int8 index;
    unsigned __int8 flags;
    // padding byte
    // padding byte
    unsigned int name_hash;
    int rigidBody;
    struct PhysPreset *physPreset;
};

struct XDollBoneOrientation // sizeof=0x20
{                                       // XREF: XDollBody/r
    float origin[3];
    int boneFlags;
    float orientation[4];
};

struct XDoll_ConstraintInfo // sizeof=0xC
{                                       // XREF: XDollBody/r
    int type;
    int timer;
    //$F8FBF74742F563EDABF51D7175B13032 ___u2;
    union //$F8FBF74742F563EDABF51D7175B13032 // sizeof=0x4
    {                                       // XREF: XDoll_ConstraintInfo/r
        rigid_body_constraint *constraint;
        rigid_body_constraint_ragdoll *ragdoll_constraint;
        rigid_body_constraint_point *point_constraint;
        rigid_body_constraint_hinge *hinge_constraint;
        rigid_body_constraint_angular_actuator *actuator_constraint;
    };
};

struct XDollBody // sizeof=0x118
{
    int references;
    int entity_index;
    int client_index;
    int piece_index;
    unsigned int attach_bone_hash;
    struct PhysConstraints *constraintsDef;
    int numBones;
    XDollBone bones[4];
    XDollBoneOrientation boneOrientations[4];
    XDoll_ConstraintInfo constraintInfos[4];
    int userBody;
    unsigned int userBodyBoneHash;
    int stableTime;
};

bool __cdecl XDoll_IsXDollConstraint(ConstraintType type);
void __cdecl XDoll_SetCollides(XDollBone *bone, bool collides);
void __cdecl XDoll_SetCollides(XDollBody *body, bool collides);
void __cdecl XDoll_SetCollides(int xdoll_handle, bool collides);
int __cdecl XDoll_GetStableTime(int xdoll_handle);
void __cdecl XDoll_UpdateStability(XDollBody *body, int msec);
void __cdecl XDoll_Update(int msec);
void __cdecl XDoll_SnapshotBonePositions(XDollBody *body, XDollBoneOrientation *boneSnapshot);
void __cdecl XDoll_UpdateTimers(XDollBody *body, int msec);
void __cdecl XDoll_DestroyConstraint(XDoll_ConstraintInfo *cinfo);
void __cdecl XDoll_DestroyXDoll(int xdoll_handle);
void __cdecl XDoll_DestroyXDoll(XDollBody *body);
int __cdecl XDoll_CreateXDollForBoneLaunch(
                int localClientNum,
                int entity_index,
                int piece_index,
                unsigned int bone_name,
                PhysPreset *physPreset);
int __cdecl XDoll_GetUnusedBody();
void __cdecl XDoll_InitBody(XDollBody *body);
char __cdecl XDoll_CreatePhysObj(
                XDollBody *body,
                XDollBone *bone,
                XModel *model,
                const cpose_t *pose,
                PhysPreset *physPreset,
                bool collide);
int __cdecl XDoll_CreateXDollForConstraints(
                int localClientNum,
                int entity_index,
                int piece_index,
                int health,
                PhysConstraints *constraints,
                PhysPreset *physPreset);
char __cdecl XDoll_Activate(const PhysConstraints *constraintsDef, int health);
void __cdecl XDoll_UpdateHealth(int xdoll_handle, int health);
void XDoll_CreateConstraint(
                XDollBody *body,
                const PhysConstraint *constraint,
                XDoll_ConstraintInfo *cinfo);
rigid_body_constraint_hinge * XDoll_CreateHingeConstraint(
                environment_rigid_body *rb1,
                environment_rigid_body *rb2,
                const phys_vec3 *b1_anchor_loc,
                const phys_vec3 *b1_hinge_axis_loc,
                const phys_vec3 *b1_ref_axis_loc,
                float damp,
                float lowStop,
                float highStop);
rigid_body_constraint_point * XDoll_CreatePointConstraint(
                environment_rigid_body *rb1,
                environment_rigid_body *rb2,
                const phys_vec3 *b1_anchor_loc);
rigid_body_constraint_angular_actuator * XDoll_CreateActuatorConstraint(
                environment_rigid_body *rb1,
                environment_rigid_body *rb2,
                float power);
environment_rigid_body *__cdecl XDoll_GetRigidBody(XDollBody *body, const char *bone_name);
void __cdecl XDoll_ApplyForce(int xdoll_handle, const float *hitp, const float *hitd, int mod);
XDollBody *__cdecl XDoll_HandleBody(int xdollHandle);
DObj *__cdecl XDoll_BodyDObj(XDollBody *body, int localClientNum);
XDollBoneOrientation *__cdecl XDoll_BodyBoneOrientations(XDollBody *body);
void __cdecl XDoll_DoControllers(int xdoll_handle, int *partBits, int localClientNum);
