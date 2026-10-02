#include "ragdoll_update.h"
#include <cgame/cg_drawtools.h>
#include "ragdoll_controller.h"
#include <xanim/dobj_utils.h>
#include <xanim/xmodel.h>
#include <cgame_mp/cg_local_mp.h>
#include <bgame/bg_misc.h>
#include <gfx_d3d/r_dpvs.h>
#include <DynEntity/DynEntity_client.h>
#include "ragdoll_quat.h"
#include <universal/com_math_anglevectors.h>
#include <cgame_mp/cg_ents_mp.h>
#include <win32/win_common.h>
#include <physics/phys_broad_phase.h>
#include <physics/physpreset_load_obj.h>
#include <physics/phys_assert.h>
#include <qcommon/threads.h>
#include <cgame_mp/cg_main_mp.h>
#include <physics/phys_render.h>
#include <physics/physics_system.h>

#include <algorithm>

void __cdecl Ragdoll_UpdateDObjWait(RagdollBody *body, int __formal);
bool __cdecl Ragdoll_EnterDobjWait(RagdollBody *body, RagdollBodyState s1, RagdollBodyState s2);

StateEnt stateEntries[6] =
{
  { Ragdoll_EnterDead, Ragdoll_ExitDead, NULL },
  { Ragdoll_EnterDobjWait, Ragdoll_ExitDObjWait, Ragdoll_UpdateDObjWait },
  { NULL, NULL, Ragdoll_UpdateVelocityCapture },
  { Ragdoll_EnterTunnelTest, NULL, NULL },
  { NULL, NULL, Ragdoll_UpdateRunning },
  { Ragdoll_EnterIdle, Ragdoll_ExitIdle, NULL }
};

void __cdecl Ragdoll_AnimMatToMat43(const DObjAnimMat *mat, float (*out)[3])
{
    ConvertQuatToMat(mat, out);
    (*out)[9] = mat->trans[0];
    (*out)[10] = mat->trans[1];
    (*out)[11] = mat->trans[2];
}

char __cdecl Ragdoll_RebindBody(int ragdollHandle)
{
    RagdollDef *def; // [esp+0h] [ebp-1Ch]
    DObj *obj; // [esp+4h] [ebp-18h]
    RagdollBody *body; // [esp+8h] [ebp-14h]
    BoneDef *boneDef; // [esp+Ch] [ebp-10h]
    LerpBone *lerpBone; // [esp+10h] [ebp-Ch]
    int i; // [esp+14h] [ebp-8h]
    int ia; // [esp+14h] [ebp-8h]
    Bone *bone; // [esp+18h] [ebp-4h]

    body = Ragdoll_HandleBody(ragdollHandle);

    iassert(body);

    if ( body->state <= (unsigned int)BS_DOBJ_WAIT )
        return 0;
    if ( !Ragdoll_ValidateBodyObj(body) )
        return 0;
    def = Ragdoll_BodyDef(body);
    iassert(def);
    obj = Ragdoll_BodyDObj(body);
    iassert(obj);

    bone = body->bones;
    boneDef = def->boneDefs;

    for ( i = 0; i < body->numBones; ++i )
    {
        bone->parentBone = boneDef->parentBone;
        bone->animBones[1] = 0;
        bone->animBones[0] = 0;
        if ( DObjGetBoneIndex(obj, boneDef->animBoneNames[0], bone->animBones, -1) && bone->animBones[0] != 255 )
        {
            if ( boneDef->animBoneNames[1] == -1 )
            {
                bone->animBones[1] = 0;
            }
            else if ( !DObjGetBoneIndex(obj, boneDef->animBoneNames[1], &bone->animBones[1], -1) || bone->animBones[1] == 255 )
            {
                bone->animBones[1] = -1;
                bone->animBones[0] = -1;
            }
        }
        ++bone;
        ++boneDef;
    }
    lerpBone = body->lerpBones;
    for ( ia = 0; ia < body->numLerpBones; ++ia )
    {
        lerpBone->animBone = 0;
        if ( !DObjGetBoneIndex(obj, def->baseLerpBoneDefs[ia].animBoneName, &lerpBone->animBone, -1)
            || lerpBone->animBone == 255 )
        {
            return 0;
        }
        lerpBone->parentBone = def->baseLerpBoneDefs[ia].parentBoneIndex;
        ++lerpBone;
    }
    Ragdoll_SnapshotBaseLerpOffsets(body);
    return 1;
}

char __cdecl Ragdoll_ValidateBodyObj(RagdollBody *body)
{
    RagdollDef *def; // [esp+0h] [ebp-1Ch]
    DObj *obj; // [esp+4h] [ebp-18h]
    BoneDef *boneDef; // [esp+8h] [ebp-14h]
    unsigned __int8 boneIdx; // [esp+Fh] [ebp-Dh] BYREF
    int i; // [esp+10h] [ebp-Ch]
    int minBones; // [esp+14h] [ebp-8h]
    BaseLerpBoneDef *lerpBoneDef; // [esp+18h] [ebp-4h]

    iassert(body);
    def = Ragdoll_BodyDef(body);
    iassert(def);
    obj = Ragdoll_BodyDObj(body);

    if ( !obj )
        return 0;

    boneDef = def->boneDefs;
    minBones = 3;
    i = 0;
    while ( i < 3 )
    {
        boneIdx = 0;
        if ( !DObjGetBoneIndex(obj, boneDef->animBoneNames[0], &boneIdx, -1) || boneIdx == 255 )
            return 0;
        boneIdx = 0;
        if ( boneDef->animBoneNames[1] && !DObjGetBoneIndex(obj, boneDef->animBoneNames[1], &boneIdx, -1) || boneIdx == 255 )
            return 0;
        ++i;
        ++boneDef;
    }

    lerpBoneDef = def->baseLerpBoneDefs;
    i = 0;
    while ( i < body->numLerpBones )
    {
        boneIdx = 0;
        if ( !DObjGetBoneIndex(obj, lerpBoneDef->animBoneName, &boneIdx, -1) || boneIdx == 255 )
            return 0;
        ++i;
        ++lerpBoneDef;
    }
    return 1;
}

void __cdecl Ragdoll_SnapshotBaseLerpOffsets(RagdollBody *body)
{
    RagdollDef *def; // [esp+4Ch] [ebp-E8h]
    BoneOrientation *boneOrientation; // [esp+50h] [ebp-E4h]
    DObj *obj; // [esp+54h] [ebp-E0h]
    DObjAnimMat parentAnimMat; // [esp+58h] [ebp-DCh] BYREF
    DObjAnimMat boneAnimMat; // [esp+78h] [ebp-BCh] BYREF
    float boneMat[4][3]; // [esp+98h] [ebp-9Ch] BYREF
    int parentIndex; // [esp+C8h] [ebp-6Ch]
    int i; // [esp+CCh] [ebp-68h]
    float invParentMat[4][3]; // [esp+D0h] [ebp-64h] BYREF
    LerpBone *bone; // [esp+100h] [ebp-34h]
    float relMat[4][3]; // [esp+104h] [ebp-30h] BYREF

    iassert(body);
    def = Ragdoll_BodyDef(body);
    iassert(def);
    obj = Ragdoll_BodyDObj(body);
    iassert(obj);
    bone = body->lerpBones;
    boneOrientation = body->lerpBoneOffsets;
    i = 0;
    while ( i < def->numBaseLerpBones )
    {
        parentIndex = bone->parentBone;
        bcassert(parentIndex, body->numBones);
        Ragdoll_GetDObjBaseBoneMatrix(obj, bone->animBone, &boneAnimMat);
        Ragdoll_GetDObjBaseBoneMatrix(obj, body->bones[parentIndex].animBones[0], &parentAnimMat);
        ConvertQuatToInverseMat(&parentAnimMat, invParentMat);
        Ragdoll_AnimMatToMat43(&boneAnimMat, boneMat);
        MatrixMultiply43(boneMat, invParentMat, relMat);
        AxisToQuat(relMat, boneOrientation->orientation);
        boneOrientation->origin[0] = relMat[3][0];
        boneOrientation->origin[1] = relMat[3][1];
        boneOrientation->origin[2] = relMat[3][2];
        ++i;
        ++bone;
        ++boneOrientation;
    }
}

void __cdecl Ragdoll_GetDObjBaseBoneMatrix(DObj *obj, unsigned __int8 boneIndex, DObjAnimMat *outMat)
{
    iassert(obj);
    DObjGetBasePoseMatrix(obj, boneIndex, outMat);
}

bool __cdecl Ragdoll_ValidatePrecalcBoneDef(RagdollDef *def, BoneDef *bone)
{
    iassert(def);
    iassert(bone);

    return fabs(bone->mass) >= 0.000001;
}

bool CompareRagdollForExplosion(const RagdollSortStruct &a, const RagdollSortStruct &b)
{
    return b.distSq > a.distSq;
}

void __cdecl Ragdoll_ExplosionEvent(
                int localClientNum,
                bool isCylinder,
                const float *origin,
                float innerRadius,
                float outerRadius,
                const float *impulse,
                float inScale)
{
    float v11; // [esp+Ch] [ebp-1D8h]
    cg_s *cgameGlob; // [esp+64h] [ebp-180h]
    float dist2; // [esp+6Ch] [ebp-178h]
    int i; // [esp+70h] [ebp-174h]
    phys_free_list<RagdollBody>::T_internal_base *body_i; // [esp+7Ch] [ebp-168h]
    float distSqr; // [esp+80h] [ebp-164h]
    int hitCount; // [esp+84h] [ebp-160h]
    float delta[3]; // [esp+88h] [ebp-15Ch]
    //float delta; // [esp+8Ch] [ebp-158h]
    //float delta; // [esp+90h] [ebp-154h]
    //float deltaa; // [esp+88h] [ebp-15Ch]
    //float delta_4a; // [esp+8Ch] [ebp-158h]
    //float delta_8a; // [esp+90h] [ebp-154h]
    float torsoPos[3]; // [esp+94h] [ebp-150h] BYREF
    float boneScale; // [esp+A0h] [ebp-144h]
    RagdollSortStruct hitEntsSorter[32]; // [esp+A4h] [ebp-140h] BYREF
    float hitForce[3]; // [esp+1A8h] [ebp-3Ch] BYREF
    float invRange; // [esp+1B4h] [ebp-30h]
    float centerOfMass[3]; // [esp+1B8h] [ebp-2Ch] BYREF
    float ragdollOrigin[3]; // [esp+1C4h] [ebp-20h] BYREF
    int boneIdx; // [esp+1D0h] [ebp-14h]
    float innerRadiusSqr; // [esp+1D4h] [ebp-10h]
    float scale; // [esp+1D8h] [ebp-Ch]
    Bone *bone; // [esp+1DCh] [ebp-8h]
    float outerRadiusSqr; // [esp+1E0h] [ebp-4h]
    int savedregs; // [esp+1E4h] [ebp+0h] BYREF

    iassert(origin);
    iassert(innerRadius >= 0.0f);
    iassert(outerRadius >= innerRadius);
    
    if ( localClientNum == RETURN_ZERO32() && outerRadius != 0.0 )
    {
        outerRadiusSqr = outerRadius * outerRadius;
        innerRadiusSqr = innerRadius * innerRadius;
        invRange = 0.0f;
        if ( (float)(outerRadius * outerRadius) > (float)(innerRadius * innerRadius) )
            invRange = 1.0 / (float)(outerRadiusSqr - innerRadiusSqr);
        hitCount = 0;

        for (RagdollBody *body : g_ragdoll_body_pool)
        {
            iassert(Ragdoll_BodyInUse(body));
            Ragdoll_BodyRootOrigin(body, ragdollOrigin);
            hitEntsSorter[hitCount].distSq = Vec3DistanceSq(origin, ragdollOrigin);
            hitEntsSorter[hitCount].body = body;
            hitCount++;
        }

        std::sort(&hitEntsSorter[0], &hitEntsSorter[hitCount], CompareRagdollForExplosion);
        //std::_Sort<RagdollSortStruct *,int,bool (__cdecl *)(RagdollSortStruct const &,RagdollSortStruct const &)>(
        //    (MaterialMemory *)hitEntsSorter,
        //    (MaterialMemory *)&hitEntsSorter[hitCount],
        //    (8 * hitCount) >> 3,
        //    (bool (__cdecl *)(const MaterialMemory *, const MaterialMemory *))DynEntCl_CompareDynEntsForExplosion);

        for ( i = 0; i < 4 && i < hitCount; ++i )
        {
            RagdollBody *body = hitEntsSorter[i].body;

            iassert(Ragdoll_BodyInUse(body));

            if ( body->state >= BS_RUNNING )
            {
                Ragdoll_GetTorsoPosition(body, torsoPos);
                cgameGlob = CG_GetLocalClientGlobals(localClientNum);
                dist2 = Vec3DistanceSq(cgameGlob->predictedPlayerState.origin, torsoPos);
                if ( dist2 <= (float)(ragdoll_reactivation_cutoff->current.value * ragdoll_reactivation_cutoff->current.value)
                    && !R_CullPoint(localClientNum, torsoPos, -100.0) )
                {
                    if ( isCylinder )
                    {
                        delta[0] = origin[0] - torsoPos[0];
                        delta[1] = origin[1] - torsoPos[1];
                        delta[2] = 0.0f;
                    }
                    else
                    {
                        delta[0] = torsoPos[0] - origin[0];
                        delta[1] = torsoPos[1] - origin[1];
                        delta[2] = torsoPos[2] - origin[2];
                    }
                    
                    if (Vec3LengthSq(delta) < outerRadiusSqr && (!Ragdoll_BodyIdle(body) || Ragdoll_BodyNewState(body, BS_RUNNING)) )
                    {
                        body->stateMsec = 0;
                        boneScale = 1.0 / (float)body->numBones;
                        for ( boneIdx = 0; boneIdx < body->numBones; ++boneIdx )
                        {
                            scale = inScale;
                            bone = &body->bones[boneIdx];
                            if ( body->bones[boneIdx].rigidBody )
                            {
                                Phys_ObjGetCenterOfMass(bone->rigidBody, centerOfMass);
                                centerOfMass[0] = flrand(-1.0, 1.0) + centerOfMass[0];
                                centerOfMass[1] = flrand(-1.0, 1.0) + centerOfMass[1];
                                centerOfMass[2] = flrand(-1.0, 1.0) + centerOfMass[2];
                                delta[0] = centerOfMass[0] - origin[0];
                                delta[1] = centerOfMass[1] - origin[1];
                                delta[2] = centerOfMass[2] - origin[2];
                                distSqr = Vec3LengthSq(delta);

                                if ( distSqr < outerRadiusSqr )
                                {
                                    if ( distSqr > innerRadiusSqr )
                                    {
                                        iassert(outerRadiusSqr > innerRadiusSqr);
                                        scale = (float)((float)(outerRadiusSqr - distSqr) * invRange) * scale;
                                    }
                                    if ( 0.0 == impulse[0] && impulse[1] == 0.0 && impulse[2] == 0.0)
                                    {
                                        hitForce[0] = delta[0];
                                        hitForce[1] = delta[1];
                                        hitForce[2] = delta[2];
                                        if ( isCylinder )
                                            hitForce[2] = 0.0f;
                                        Vec3Normalize(hitForce);
                                    }
                                    else
                                    {
                                        hitForce[0] = impulse[0];
                                        hitForce[1] = impulse[1];
                                        hitForce[2] = impulse[2];
                                    }
                                    hitForce[2] = hitForce[2] + ragdoll_explode_upbias->current.value;
                                    Vec3Normalize(hitForce);
                                    v11 = (float)(scale * ragdoll_explode_force->current.value) * boneScale;
                                    hitForce[0] *= v11;
                                    hitForce[1] *= v11;
                                    hitForce[2] *= v11;
                                    Phys_ObjAddForce(bone->rigidBody, centerOfMass, hitForce, 0);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

bool __cdecl Ragdoll_BodyIdle(RagdollBody *body)
{
    iassert(body);
    return body->state == BS_IDLE;
}

void __cdecl Ragdoll_GetTorsoPosition(RagdollBody *body, float *center)
{
    BoneOrientation *v2; // eax

    iassert(body && Ragdoll_BodyInUse(body));

    v2 = Ragdoll_BodyBoneOrientations(body);
    center[0] = v2->origin[0];
    center[1] = v2->origin[1];
    center[2] = v2->origin[2];
}

void __cdecl Ragdoll_Launch(int localClientNum, int ragdollHandle, const float *force, hitLocation_t hitloc)
{
    float *v4; // [esp+0h] [ebp-10h]
    float v5; // [esp+4h] [ebp-Ch]
    int numBones; // [esp+8h] [ebp-8h]
    RagdollBody *body; // [esp+Ch] [ebp-4h]

    if ( localClientNum == RETURN_ZERO32() )
    {
        body = Ragdoll_HandleBody(ragdollHandle);
        iassert(body);

        if ( body->storedLaunchCount < 2 )
        {
            body->storedLaunchHitLoc[body->storedLaunchCount] = hitloc;
            numBones = body->numBones;
            if ( hitloc )
                numBones = 1;
            v4 = body->storedLaunchForce[body->storedLaunchCount];
            v5 = ragdoll_bullet_force->current.value / (float)numBones;
            v4[0] = v5 * force[0];
            v4[1] = v5 * force[1];
            v4[2] = v5 * force[2];
            ++body->storedLaunchCount;
        }
    }
}

void __cdecl Ragdoll_LaunchUpdate(RagdollBody *body)
{
    int bi; // [esp+1Ch] [ebp-28h]
    unsigned int boneName; // [esp+20h] [ebp-24h] BYREF
    RagdollDef *def; // [esp+24h] [ebp-20h]
    float hitForce[3]; // [esp+28h] [ebp-1Ch] BYREF
    float centerOfMass[3]; // [esp+34h] [ebp-10h] BYREF

    if ( !Ragdoll_BodyIdle(body) || Ragdoll_BodyNewState(body, BS_RUNNING) )
    {
        for ( int i = 0; i < body->storedLaunchCount; ++i )
        {
            hitForce[0] = body->storedLaunchForce[i][0];
            hitForce[1] = body->storedLaunchForce[i][1];
            hitForce[2] = body->storedLaunchForce[i][2];
            if ( body->storedLaunchHitLoc[i] )
            {
                MapHitLocationToRagdollBoneName(body->storedLaunchHitLoc[i], &boneName);
                def = Ragdoll_BodyDef(body);
                for ( bi = 0; bi < def->numBones; ++bi )
                {
                    if ( def->boneDefs[bi].animBoneNames[1] == boneName )
                    {
                        if ( body->bones[bi].rigidBody )
                        {
                            Phys_ObjGetCenterOfMass(body->bones[bi].rigidBody, centerOfMass);
                            centerOfMass[0] = flrand(-1.0, 1.0) + centerOfMass[0];
                            centerOfMass[1] = flrand(-1.0, 1.0) + centerOfMass[1];
                            centerOfMass[2] = flrand(-1.0, 1.0) + centerOfMass[2];
                            Phys_ObjAddForce(body->bones[bi].rigidBody, centerOfMass, hitForce, 0);
                        }
                    }
                }
            }
            else
            {
                for ( int boneIdx = 0; boneIdx < body->numBones; ++boneIdx )
                {
                    if ( body->bones[boneIdx].rigidBody )
                    {
                        Phys_ObjGetCenterOfMass(body->bones[boneIdx].rigidBody, centerOfMass);
                        centerOfMass[0] = flrand(-1.0, 1.0) + centerOfMass[0];
                        centerOfMass[1] = flrand(-1.0, 1.0) + centerOfMass[1];
                        centerOfMass[2] = flrand(-1.0, 1.0) + centerOfMass[2];
                        Phys_ObjAddForce(body->bones[boneIdx].rigidBody, centerOfMass, hitForce, 0);
                    }
                }
            }
        }
        body->storedLaunchCount = 0;
    }
}

void __cdecl Ragdoll_Attach(int localClientNum, int ragdollHandle, int attachEnt, hitLocation_t hitloc)
{
    RagdollBody *body; // [esp+0h] [ebp-4h]

    if ( localClientNum == RETURN_ZERO32() )
    {
        body = Ragdoll_HandleBody(ragdollHandle);

        iassert(body);

        if ( !body->hang_point )
        {
            body->storedAttachHitLoc = hitloc;
            body->storedAttachEnt = attachEnt;
            body->storedPhysObjId = 0;
        }
    }
}

void __cdecl Ragdoll_AttachUpdate(RagdollBody *body)
{
    if ( Ragdoll_BodyIdle(body) )
        Ragdoll_BodyNewState(body, BS_RUNNING);
}

bool __cdecl Ragdoll_EnterTunnelTest(RagdollBody *body, RagdollBodyState prevState, RagdollBodyState curState)
{
    BoneOrientation *v2; // eax
    BoneOrientation *v3; // eax
    BoneOrientation *v4; // eax
    DObj *obj; // [esp+Ch] [ebp-Ch]
    char tunnelPassed; // [esp+13h] [ebp-5h]
    const cpose_t *pose; // [esp+14h] [ebp-4h]
    int savedregs; // [esp+18h] [ebp+0h] BYREF

    obj = Ragdoll_BodyDObj(body);
    if ( obj && DObjGetRotTransArray(obj) && Ragdoll_CreateBodyPhysics(body) )
    {
        v2 = Ragdoll_BodyBoneOrientations(body);
        Ragdoll_SetCurrentPoseFromSnapshot(body, v2);
        Ragdoll_EstimateInitialVelocities(body);
        tunnelPassed = Ragdoll_TunnelTest(body);
        pose = Ragdoll_BodyPose(body);
        v3 = Ragdoll_BodyBoneOrientations(body);
        body->poseOffset[0] = pose->origin[0] - v3->origin[0];
        body->poseOffset[1] = pose->origin[1] - v3->origin[1];
        body->poseOffset[2] = pose->origin[2] - v3->origin[2];
        v4 = Ragdoll_BodyBoneOrientations(body);
        Ragdoll_SnapshotBaseLerpBones(body, v4);
        if ( tunnelPassed )
        {
            Ragdoll_BodyNewState(body, BS_RUNNING);
        }
        else
        {
            Ragdoll_PrintTunnelFail(body);
            Ragdoll_BodyNewState(body, BS_DEAD);
        }
        return 1;
    }
    else
    {
        Ragdoll_BodyNewState(body, BS_DEAD);
        return 0;
    }
}

void __cdecl Ragdoll_SnapshotBaseLerpBones(RagdollBody *body, BoneOrientation *snapshot)
{
    float v2; // [esp+8h] [ebp-1A4h]
    float v3; // [esp+60h] [ebp-14Ch]
    RagdollDef *def; // [esp+64h] [ebp-148h]
    int parentBoneNum; // [esp+68h] [ebp-144h]
    DObj *obj; // [esp+6Ch] [ebp-140h]
    BoneOrientation *baseOffsetOrientation; // [esp+70h] [ebp-13Ch]
    DObjAnimMat *parentAnimMat; // [esp+74h] [ebp-138h]
    float bodyOffset[3]; // [esp+78h] [ebp-134h] BYREF
    float lerp; // [esp+84h] [ebp-128h]
    float boneMat[4][3]; // [esp+88h] [ebp-124h] BYREF
    DObjAnimMat *boneAnimMat; // [esp+B8h] [ebp-F4h]
    BoneOrientation *parentOrientation; // [esp+BCh] [ebp-F0h]
    unsigned __int8 parentBoneIdx; // [esp+C3h] [ebp-E9h]
    int goalMsec; // [esp+C4h] [ebp-E8h]
    float invAxis[3][3]; // [esp+C8h] [ebp-E4h] BYREF
    const cpose_t *pose; // [esp+ECh] [ebp-C0h]
    LerpBone *lerpBone; // [esp+F0h] [ebp-BCh]
    unsigned __int8 boneIdx; // [esp+F7h] [ebp-B5h]
    int i; // [esp+F8h] [ebp-B4h]
    float currentOffset[3]; // [esp+FCh] [ebp-B0h] BYREF
    float newLocalRot[4]; // [esp+108h] [ebp-A4h] BYREF
    float axis[3][3]; // [esp+118h] [ebp-94h] BYREF
    float currentLocalRot[4]; // [esp+13Ch] [ebp-70h] BYREF
    float invParentMat[4][3]; // [esp+14Ch] [ebp-60h] BYREF
    float relMat[4][3]; // [esp+17Ch] [ebp-30h] BYREF
    BoneOrientation *snapshota; // [esp+1B8h] [ebp+Ch]

    def = Ragdoll_BodyDef(body);
    iassert(def);
    obj = Ragdoll_BodyDObj(body);
    if ( obj )
    {
        pose = Ragdoll_BodyPose(body);
        if ( pose )
        {
            lerpBone = body->lerpBones;
            baseOffsetOrientation = body->lerpBoneOffsets;
            snapshota = &snapshot[body->numBones];
            i = 0;
            while ( i < def->numBaseLerpBones )
            {
                goalMsec = def->baseLerpBoneDefs[i].lerpTime;
                if ( goalMsec <= 0 )
                {
                    lerp = 1.0f;
                }
                else
                {
                    lerp = (float)body->stateMsec / (float)goalMsec;
                    v3 = (float)(lerp - 1.0) < 0.0 ? lerp : 1.0f;
                    v2 = (float)(0.0 - lerp) < 0.0 ? v3 : 0.0f;
                    lerp = v2;
                }
                boneIdx = lerpBone->animBone;
                parentBoneNum = lerpBone->parentBone;
                bcassert(parentBoneNum, body->numBones);
                parentOrientation = &Ragdoll_BodyBoneOrientations(body)[parentBoneNum];
                parentBoneIdx = body->bones[parentBoneNum].animBones[0];
                boneAnimMat = Ragdoll_GetDObjLocalBoneMatrix(pose, obj, boneIdx);
                parentAnimMat = Ragdoll_GetDObjLocalBoneMatrix(pose, obj, parentBoneIdx);
                if ( !boneAnimMat || !parentAnimMat || boneAnimMat == parentAnimMat )
                    break;
                ConvertQuatToInverseMat(parentAnimMat, invParentMat);
                Ragdoll_AnimMatToMat43(boneAnimMat, boneMat);
                MatrixMultiply43(boneMat, invParentMat, relMat);
                AxisToQuat(relMat, currentLocalRot);
                currentOffset[0] = relMat[3][0];
                currentOffset[1] = relMat[3][1];
                currentOffset[2] = relMat[3][2];
                if (Vec3Length(currentOffset) <= 400.0 )
                {
                    Ragdoll_QuatLerp(currentLocalRot, baseOffsetOrientation->orientation, lerp, newLocalRot);
                    Vec3Lerp(currentOffset, baseOffsetOrientation->origin, lerp, bodyOffset);
                }
                else
                {
                    newLocalRot[0] = baseOffsetOrientation->orientation[0];
                    newLocalRot[1] = baseOffsetOrientation->orientation[1];
                    newLocalRot[2] = baseOffsetOrientation->orientation[2];
                    newLocalRot[3] = baseOffsetOrientation->orientation[3];
                    bodyOffset[0] = baseOffsetOrientation->origin[0];
                    bodyOffset[1] = baseOffsetOrientation->origin[1];
                    bodyOffset[2] = baseOffsetOrientation->origin[2];
                }
                Ragdoll_QuatMul(parentOrientation->orientation, newLocalRot, snapshota->orientation);
                QuatToAxis(parentOrientation->orientation, axis);
                AxisTranspose(axis, invAxis);
                Vec3Rotate(bodyOffset, invAxis, snapshota->origin);
                snapshota->origin[0] = parentOrientation->origin[0] + snapshota->origin[0];
                snapshota->origin[1] = parentOrientation->origin[1] + snapshota->origin[1];
                snapshota->origin[2] = parentOrientation->origin[2] + snapshota->origin[2];
                ++i;
                ++lerpBone;
                ++baseOffsetOrientation;
                ++snapshota;
            }
        }
    }
}

DObjAnimMat *__cdecl Ragdoll_GetDObjLocalBoneMatrix(const cpose_t *pose, DObj *obj, unsigned __int8 boneIndex)
{
    DObjAnimMat *mat; // [esp+4h] [ebp-4h]

    iassert(obj);
    CG_DObjCalcBone(pose, obj, boneIndex);
    mat = DObjGetRotTransArray(obj);
    if ( mat )
        return &mat[boneIndex];
    else
        return 0;
}

void __cdecl Ragdoll_SetCurrentPoseFromSnapshot(RagdollBody *body, BoneOrientation *snapshot)
{
    const float *qa; // [esp+4h] [ebp-3Ch]
    float error_sq; // [esp+Ch] [ebp-34h]
    rigid_body_constraint_ragdoll *id; // [esp+10h] [ebp-30h]
    int j; // [esp+14h] [ebp-2Ch]
    RagdollDef *def; // [esp+18h] [ebp-28h]
    float rotatedOrientation[4]; // [esp+1Ch] [ebp-24h] BYREF
    BoneDef *boneDef; // [esp+2Ch] [ebp-14h]
    int iters; // [esp+30h] [ebp-10h]
    int i; // [esp+34h] [ebp-Ch]
    Bone *bone; // [esp+38h] [ebp-8h]
    float max_error_sq; // [esp+3Ch] [ebp-4h]

    iassert(body);
    def = Ragdoll_BodyDef(body);
    iassert(def);
    bone = body->bones;
    boneDef = def->boneDefs;
    i = 0;
    while ( i < body->numBones )
    {
        qa = snapshot->orientation;
        if ( boneDef->mirror )
        {
            Ragdoll_QuatMul(qa, quatZRot, rotatedOrientation);
        }
        else
        {
            rotatedOrientation[0] = *qa;
            rotatedOrientation[1] = snapshot->orientation[1];
            rotatedOrientation[2] = snapshot->orientation[2];
            rotatedOrientation[3] = snapshot->orientation[3];
        }
        if ( bone->rigidBody )
            Phys_ObjSetOrientation(bone->rigidBody, snapshot->origin, rotatedOrientation);
        ++i;
        ++bone;
        ++boneDef;
        ++snapshot;
    }
    max_error_sq = 100.0f;
    iters = 0;
    while ( max_error_sq > 0.0000099999997 && iters < 100 )
    {
        ++iters;
        max_error_sq = 0.0f;
        for ( j = 0; j < body->numJoints; ++j )
        {
            id = (rigid_body_constraint_ragdoll *)body->joints[j].joint;
            if ( id )
            {
                //error_sq = rigid_body_constraint_ragdoll::pull_together(id, (int)&savedregs);
                error_sq = id->pull_together();
                if ( error_sq > max_error_sq )
                    max_error_sq = error_sq;
            }
        }
    }
}

char __cdecl Ragdoll_CreateBodyPhysics(RagdollBody *body)
{
    iassert(body);
    iassert(body->state > RAGDOLL_DOBJ_VALID_STATE);
    iassert(Ragdoll_BodyInUse(body));

    if ( Ragdoll_CreatePhysObjs(body) )
    {
        if ( Ragdoll_CreatePhysJoints(body) )
        {
            return 1;
        }
        else
        {
            Ragdoll_DestroyPhysObjs(body);
            Ragdoll_DestroyPhysJoints(body);
            return 0;
        }
    }
    else
    {
        Ragdoll_DestroyPhysObjs(body);
        return 0;
    }
}

char __cdecl Ragdoll_CreatePhysJoints(RagdollBody *body)
{
    RagdollDef *def; // [esp+0h] [ebp-8h]

    iassert(body);
    def = Ragdoll_BodyDef(body);
    iassert(def);

    for ( int i = 0; i < body->numJoints; ++i )
    {
        if ( !Ragdoll_CreatePhysJoint(body, &def->jointDefs[i], &body->joints[i]) )
            return 0;
    }
    return 1;
}

char __cdecl Ragdoll_CreatePhysJoint(RagdollBody *body, JointDef *jointDef, Joint *joint)
{
    JointType type; // [esp+10h] [ebp-9Ch]
    Bone *parentBone; // [esp+1Ch] [ebp-90h]
    float limitAxes[3][3]; // [esp+20h] [ebp-8Ch] BYREF
    float tAxis[3][3]; // [esp+44h] [ebp-68h] BYREF
    float anchor[3]; // [esp+68h] [ebp-44h] BYREF
    float fric[3]; // [esp+74h] [ebp-38h] BYREF
    int i; // [esp+80h] [ebp-2Ch]
    float axis[3][3]; // [esp+84h] [ebp-28h] BYREF
    Bone *bone; // [esp+A8h] [ebp-4h]

    iassert(body);
    iassert(jointDef);
    iassert(joint);
    bcassert(jointDef->bone, body->numBones);

    bone = &body->bones[jointDef->bone];
    if ( bone->parentBone == -1 )
        parentBone = 0;
    else
        parentBone = &body->bones[bone->parentBone];

    if ( !bone->rigidBody )
        return 1;

    Phys_ObjGetPosition(bone->rigidBody, anchor, tAxis);
    AxisTranspose(tAxis, axis);

    if ( !jointDef->numLimitAxes )
    {
        jointDef->limitAxes[0][0] = 0.0f;
        jointDef->limitAxes[0][1] = 0.0f;
        jointDef->limitAxes[0][2] = 1.0f;
        jointDef->axisFriction[0] = 0.0f;
        jointDef->minAngles[0] = -1.5707964;
        jointDef->maxAngles[0] = 1.5707964;
        ++jointDef->numLimitAxes;
    }

    for ( int i = 0; i < jointDef->numLimitAxes; ++i )
        Vec3Rotate(jointDef->limitAxes[i], axis, limitAxes[i]);

    fric[0] = 15.0 * jointDef->axisFriction[0];
    fric[1] = 15.0 * jointDef->axisFriction[1];
    fric[2] = 15.0 * jointDef->axisFriction[2];
    type = jointDef->type;
    if ( type == RAGDOLL_JOINT_HINGE )
    {
        joint->joint = (int)Phys_CreateHinge(
                                                    bone->rigidBody,
                                                    parentBone->rigidBody,
                                                    anchor,
                                                    limitAxes[0],
                                                    0.0,
                                                    fric[0],
                                                    jointDef->minAngles[0],
                                                    jointDef->maxAngles[0]);
        joint->joint2 = 0;
    }
    else if ( type == RAGDOLL_JOINT_SWIVEL )
    {
        joint->joint = (int)Phys_CreateSwivel(
                                                    bone->rigidBody,
                                                    parentBone->rigidBody,
                                                    anchor,
                                                    jointDef->numLimitAxes,
                                                    limitAxes,
                                                    vec3_origin,
                                                    fric,
                                                    jointDef->minAngles,
                                                    jointDef->maxAngles);
    }
    return 1;
}

char __cdecl Ragdoll_CreatePhysObjs(RagdollBody *body)
{
    broad_phase_info *v2; // eax
    broad_phase_info *bpi; // [esp+28h] [ebp-3Ch]
    PhysObjUserData *userData; // [esp+38h] [ebp-2Ch]
    PhysObjUserData *userData1; // [esp+40h] [ebp-24h]
    broad_phase_collision_pair *bpcp; // [esp+44h] [ebp-20h]
    PhysObjUserData *userData0; // [esp+48h] [ebp-1Ch]
    int i0; // [esp+4Ch] [ebp-18h]
    int i1; // [esp+50h] [ebp-14h]
    int j; // [esp+54h] [ebp-10h]
    RagdollDef *def; // [esp+58h] [ebp-Ch]
    int i; // [esp+5Ch] [ebp-8h]
    int ia; // [esp+5Ch] [ebp-8h]

    iassert(body);
    if (physGlob.objects_by_world[0].m_alloc_count / 12 >= 5)
        proftimer_physics_frame_advance.start_capture();
    def = Ragdoll_BodyDef(body);
    iassert(def);

    Sys_EnterCriticalSection(CRITSECT_PHYSICS);
    for ( i = 0; i < body->numBones; ++i )
    {
        if ( !Ragdoll_CreatePhysObj(body, &def->boneDefs[i], &body->bones[i]) )
        {
            Sys_LeaveCriticalSection(CRITSECT_PHYSICS);
            return 0;
        }
    }
    body->m_list_bpcp = 0;
    for ( j = 0; j < def->numSelfPairs; ++j )
    {
        i0 = def->selfPairDefs[j].bones[0];
        i1 = def->selfPairDefs[j].bones[1];
        if ( body->bones[i0].rigidBody && body->bones[i1].rigidBody )
        {
            userData0 = (PhysObjUserData *)body->bones[i0].rigidBody;
            userData1 = (PhysObjUserData *)body->bones[i1].rigidBody;

            iassert(userData0->m_bpb && userData0->m_bpb->is_bpi());
            iassert(userData1->m_bpb && userData1->m_bpb->is_bpi());

            bpcp = create_broad_phase_collision_pair();
            if ( bpcp )
            {
                //bpi = broad_phase_base::get_bpi(userData1->m_bpb);
                //bpcp->m_bpi1 = broad_phase_base::get_bpi(userData0->m_bpb);
                bpcp->m_bpi1 = userData0->m_bpb->get_bpi();
                bpcp->m_bpi2 = userData1->m_bpb->get_bpi();
                bpcp->m_next_bpcp = body->m_list_bpcp;
                body->m_list_bpcp = bpcp;
            }
        }
    }
    body->m_bpg = create_broad_phase_group();
    //broad_phase_group::set(body->m_bpg);
    body->m_bpg->set();
    for ( ia = 0; ia < body->numBones; ++ia )
    {
        if ( body->bones[ia].rigidBody )
        {
            userData = (PhysObjUserData *)body->bones[ia].rigidBody;
            
            iassert(userData->m_bpb && userData->m_bpb->is_bpi());

            aasap_list_remove(userData->m_bpb);
            userData->m_bpb->m_env_collision_flags &= ~0x80u;
            userData->m_bpb->m_my_collision_type_flags |= 0x200u;
            environment_collision_list_remove(userData->m_bpb);
            //v2 = broad_phase_base::get_bpi(userData->m_bpb);
            v2 = userData->m_bpb->get_bpi();
            //broad_phase_group::add_bpi(body->m_bpg, v2);
            body->m_bpg->add_bpi(v2);
            userData->m_bpb = 0;
        }
    }
    environment_collision_list_add(body->m_bpg);
    aasap_list_add(body->m_bpg);
    Sys_LeaveCriticalSection(CRITSECT_PHYSICS);
    return 1;
}

char __cdecl Ragdoll_CreatePhysObj(RagdollBody *body, BoneDef *boneDef, Bone *bone)
{
    gjk_double_sphere_t *capsule_gjk_geom; // eax
    float qRot[4]; // [esp+40h] [ebp-E4h] BYREF
    RagdollDef *def; // [esp+50h] [ebp-D4h]
    float hlength; // [esp+54h] [ebp-D0h]
    DObj *obj; // [esp+58h] [ebp-CCh]
    float b1Origin[3]; // [esp+5Ch] [ebp-C8h] BYREF
    int dir; // [esp+68h] [ebp-BCh]
    float b0Origin[3]; // [esp+6Ch] [ebp-B8h] BYREF
    PhysObjUserData *userData; // [esp+78h] [ebp-ACh]
    centity_s *cent; // [esp+7Ch] [ebp-A8h]
    float b0Quat[4]; // [esp+80h] [ebp-A4h] BYREF
    float offset; // [esp+90h] [ebp-94h]
    float invPoseAxis[3][3]; // [esp+94h] [ebp-90h] BYREF
    const cpose_t *pose; // [esp+B8h] [ebp-6Ch]
    gjk_geom_list_t gjk_geom_list; // [esp+BCh] [ebp-68h] BYREF
    PhysPreset preset; // [esp+C4h] [ebp-60h] BYREF
    int boneIdx; // [esp+120h] [ebp-4h]

    iassert(body);
    iassert(boneDef);
    iassert(bone);
    def = Ragdoll_BodyDef(body);
    iassert(def);

    obj = Ragdoll_BodyDObj(body);

    if ( !obj )
        return 0;

    iassert(def->bound);
    iassert(boneDef->animBoneNames[0]);

    bone->animBones[0] = 0;
    bone->animBones[1] = 0;

    if ( !DObjGetBoneIndex(obj, boneDef->animBoneNames[0], bone->animBones, -1) || bone->animBones[0] == 255 )
        return 1;

    if ( boneDef->animBoneNames[1] )
    {
        if ( !DObjGetBoneIndex(obj, boneDef->animBoneNames[1], &bone->animBones[1], -1) || bone->animBones[0] == 255 )
        {
            bone->animBones[0] = -1;
            bone->animBones[1] = -1;
            return 1;
        }
    }
    else
    {
        bone->animBones[1] = 0;
    }
    pose = Ragdoll_BodyPose(body);
    iassert(pose);

    Ragdoll_PoseInvAxis(pose, invPoseAxis);
    Ragdoll_GetDObjBaseBoneOriginQuat(obj, pose->origin, invPoseAxis, bone->animBones[0], b0Origin, qRot);

    if ( boneDef->mirror )
    {
        Ragdoll_QuatMul(qRot, quatZRot, b0Quat);
    }
    else
    {
        b0Quat[0] = qRot[0];
        b0Quat[1] = qRot[1];
        b0Quat[2] = qRot[2];
        b0Quat[3] = qRot[3];
    }
    if ( bone->animBones[1] )
    {
        Ragdoll_GetDObjBaseBoneOrigin(obj, pose->origin, invPoseAxis, bone->animBones[1], b1Origin);
    }
    else
    {
        b1Origin[0] = b0Origin[0];
        b1Origin[1] = b0Origin[1];
        b1Origin[2] = b0Origin[2];
    }
    boneIdx = bone - body->bones;
    preset.bounce = 0.0f;
    preset.bulletForceScale = 1.0f;
    preset.explosiveForceScale = 1.0f;
    preset.friction = boneDef->friction;
    preset.mass = boneDef->mass;
    preset.name = "ragdoll_bone";
    preset.sndAliasPrefix = 0;
    preset.canFloat = 1;
    memset(preset.centerOfMassOffset, 0, 36);
    bone->parentBone = boneDef->parentBone;
    bone->length = Vec3Distance(b0Origin, b1Origin);
    bone->center[0] = 0.0f;
    bone->center[1] = 0.0f;
    bone->center[2] = 0.0f;
    hlength = bone->length * 0.5;
    dir = boneIdx != 0 ? 0 : 2;
    bone->center[dir] = bone->length * 0.5;
    offset = 0.0f;
    if ( boneIdx == 3 )
        offset = 2.0f;
    bone->center[dir] = bone->center[dir] + offset;
    hlength = hlength + offset;
    gjk_geom_list.m_first_geom = 0;
    gjk_geom_list.m_geom_count = 0;
    capsule_gjk_geom = create_capsule_gjk_geom(
                                             bone->center,
                                             boneDef->radius,
                                             hlength,
                                             0,
                                             7,
                                             &g_empty_collision_visitor);
    //gjk_geom_list_t::add_geom(&gjk_geom_list, capsule_gjk_geom);
    gjk_geom_list.add_geom(capsule_gjk_geom);
    bone->rigidBody = (int)Phys_ObjCreate(2, b0Origin, b0Quat, vec3_origin, &preset, &gjk_geom_list, 0, -1);
    cent = CG_GetEntity(body->localClientNum, body->dobj);
    if ( bone->rigidBody )
    {
        userData = (PhysObjUserData *)bone->rigidBody;
        if ( ((*((unsigned int *)cent + 201) >> 19) & 1) != 0 )
            userData->m_flags |= 4u;
        if ( ((*((unsigned int *)cent + 201) >> 20) & 1) != 0 )
            userData->m_flags |= 0x10u;
        userData->m_flags |= 0x20u;
        return 1;
    }
    else
    {
        Com_PrintWarning(14, "Ragdoll: Failed to create rigid body\n");
        return 0;
    }
}

char __cdecl Ragdoll_GetDObjBaseBoneOrigin(
                DObj *obj,
                const float *offset,
                const float (*axis)[3],
                unsigned __int8 boneIndex,
                float *origin)
{
    DObjAnimMat mat; // [esp+0h] [ebp-20h] BYREF

    iassert(obj);
    Ragdoll_GetDObjBaseBoneMatrix(obj, boneIndex, &mat);
    Vec3Rotate(mat.trans, axis, origin);
    Vec3Add(origin, offset, origin);

    return 1;
}

char __cdecl Ragdoll_GetDObjBaseBoneOriginQuat(
                DObj *obj,
                const float *offset,
                const float (*axis)[3],
                unsigned __int8 boneIndex,
                float *origin,
                float *quat)
{
    DObjAnimMat mat; // [esp+0h] [ebp-30h] BYREF
    float orientation[4]; // [esp+20h] [ebp-10h] BYREF

    iassert(obj);
    Ragdoll_GetDObjBaseBoneMatrix(obj, boneIndex, &mat);
    Ragdoll_Mat33ToQuat(axis, orientation);
    Ragdoll_QuatMul(orientation, mat.quat, quat);
    Vec3Rotate(mat.trans, axis, origin);
    Vec3Add(origin, offset, origin);

    return 1;
}

void __cdecl Ragdoll_PoseInvAxis(const cpose_t *pose, float (*invAxis)[3])
{
    float axis[3][3]; // [esp+0h] [ebp-24h] BYREF

    iassert(pose);
    AnglesToAxis(pose->angles, axis);
    AxisTranspose(axis, invAxis);
}

void __cdecl Ragdoll_DestroyPhysJoints(RagdollBody *body)
{
    memset(body->joints, 0, sizeof(body->joints));
}

void __cdecl Ragdoll_DestroyPhysObjs(RagdollBody *body)
{
    Sys_EnterCriticalSection(CRITSECT_PHYSICS);
    iassert(body);
    if ( body->m_list_bpcp )
    {
        destroy_broad_phase_collision_pair_list(body->m_list_bpcp);
        body->m_list_bpcp = 0;
    }
    if ( body->m_bpg )
    {
        destroy_broad_phase_info_list(body->m_bpg->m_list_bpi_head);
        aasap_list_remove(body->m_bpg);
        environment_collision_list_remove(body->m_bpg);
        destroy_broad_phase_group(body->m_bpg);
        body->m_bpg = 0;
    }
    for ( int i = 0; i < body->numBones; ++i )
    {
        if ( body->bones[i].rigidBody )
            Phys_ObjDestroy(2, body->bones[i].rigidBody);
        body->bones[i].rigidBody = 0;
    }
    Sys_LeaveCriticalSection(CRITSECT_PHYSICS);
}

void __cdecl Ragdoll_EstimateInitialVelocities(RagdollBody *body)
{
    float len2; // [esp+38h] [ebp-6Ch]
    float angleOffset[3]; // [esp+3Ch] [ebp-68h] BYREF
    float posOffset[3]; // [esp+48h] [ebp-5Ch] BYREF
    BoneOrientation *prevOrientation; // [esp+54h] [ebp-50h]
    float timeScale; // [esp+58h] [ebp-4Ch]
    float prevRot[4]; // [esp+5Ch] [ebp-48h] BYREF
    float curRot[4]; // [esp+6Ch] [ebp-38h] BYREF
    BoneOrientation *curOrientation; // [esp+7Ch] [ebp-28h]
    float rotOffset[4]; // [esp+80h] [ebp-24h] BYREF
    int i; // [esp+90h] [ebp-14h]
    float angleOffsetWorld[3]; // [esp+94h] [ebp-10h] BYREF
    Bone *bone; // [esp+A0h] [ebp-4h]

    iassert(body);

    if ( body->velCaptureMsec )
    {
        timeScale = 1000.0 / (float)body->velCaptureMsec;
        curOrientation = Ragdoll_BodyBoneOrientations(body);
        prevOrientation = Ragdoll_BodyPrevBoneOrientations(body);
        bone = body->bones;
        i = 0;
        while ( i < body->numBones )
        {
            if ( bone->rigidBody )
            {
                posOffset[0] = curOrientation->origin[0] - prevOrientation->origin[0];
                posOffset[1] = curOrientation->origin[1] - prevOrientation->origin[1];
                posOffset[2] = curOrientation->origin[2] - prevOrientation->origin[2];

                if ( ((((curOrientation->orientation[0] * prevOrientation->orientation[0])
                    + (curOrientation->orientation[1] * prevOrientation->orientation[1]))
                    + (curOrientation->orientation[2] * prevOrientation->orientation[2]))
                    + (curOrientation->orientation[3] * prevOrientation->orientation[3])) >= 0.0 )
                {
                    prevRot[0] = prevOrientation->orientation[0];
                    prevRot[1] = prevOrientation->orientation[1];
                    prevRot[2] = prevOrientation->orientation[2];
                    prevRot[3] = prevOrientation->orientation[3];
                }
                else
                {
                    prevRot[0] = -prevOrientation->orientation[0];
                    prevRot[1] = -prevOrientation->orientation[1];
                    prevRot[2] = -prevOrientation->orientation[2];
                    prevRot[3] = -prevOrientation->orientation[3];
                }
                curRot[0] = curOrientation->orientation[0];
                curRot[1] = curOrientation->orientation[1];
                curRot[2] = curOrientation->orientation[2];
                curRot[3] = curOrientation->orientation[3];
                Vec4Normalize(curRot);
                Vec4Normalize(prevRot);
                Ragdoll_QuatMulInvSecond(curRot, prevRot, rotOffset);
                Vec4Normalize(rotOffset);
                Ragdoll_QuatToAxisAngle(rotOffset, angleOffset);
                Ragdoll_QuatPointRotate(angleOffset, curRot, angleOffsetWorld);
                Vec3Scale(angleOffsetWorld, (timeScale * ragdoll_rotvel_scale->current.value), angleOffset);
                Vec3Scale(posOffset, timeScale, posOffset);
                Phys_ObjSetAngularVelocityRaw(bone->rigidBody, angleOffset);
                len2 = Vec3LengthSq(posOffset);
                if ( len2 > (600.0 * 600.0) )
                {
                    Vec3Scale(posOffset, 600.0 / sqrtf(len2), posOffset);
                }
                static const float clamp = 50.0f;
                if ( posOffset[2] > clamp )
                    posOffset[2] = clamp;
                Phys_ObjSetVelocity(bone->rigidBody, posOffset);
            }
            ++i;
            ++curOrientation;
            ++prevOrientation;
            ++bone;
        }
    }
}

char    Ragdoll_TunnelTest(RagdollBody *body)
{
    rigid_body *v3; // [esp+44h] [ebp-1D0h]
    float v4; // [esp+70h] [ebp-1A4h]
    float ndif; // [esp+74h] [ebp-1A0h]
    phys_vec3 dif; // [esp+78h] [ebp-19Ch] BYREF
    float v7; // [esp+8Ch] [ebp-188h]
    float v8; // [esp+90h] [ebp-184h]
    float v9; // [esp+94h] [ebp-180h]
    phys_vec3 last_position; // [esp+98h] [ebp-17Ch]
    phys_mat44 *p_m_mat; // [esp+ACh] [ebp-168h]
    PhysObjUserData *userData; // [esp+B0h] [ebp-164h]
    float sphere_radius; // [esp+B4h] [ebp-160h]
    int v14; // [esp+B8h] [ebp-15Ch]
    int v15; // [esp+BCh] [ebp-158h]
    float v16; // [esp+C0h] [ebp-154h]
    phys_vec3 sphere_center; // [esp+C8h] [ebp-14Ch] BYREF
    const cpose_t *pose; // [esp+DCh] [ebp-138h]
    float newPos[3]; // [esp+E0h] [ebp-134h] BYREF
    float center[4]; // [esp+ECh] [ebp-128h] BYREF
    float end[3]; // [esp+FCh] [ebp-118h] BYREF
    float length; // [esp+108h] [ebp-10Ch]
    float start[3]; // [esp+10Ch] [ebp-108h] BYREF
    float boneMat[3][3]; // [esp+118h] [ebp-FCh] BYREF
    float orientation[4]; // [esp+13Ch] [ebp-D8h] BYREF
    float *v26; // [esp+14Ch] [ebp-C8h]
    int v27; // [esp+150h] [ebp-C4h]
    int i; // [esp+154h] [ebp-C0h]
    BoneOrientation *boneOrientations; // [esp+158h] [ebp-BCh]
    BoneOrientation *curOrientation; // [esp+15Ch] [ebp-B8h]
    BoneDef *boneDef; // [esp+160h] [ebp-B4h]
    Bone *bone; // [esp+164h] [ebp-B0h]
    RagdollDef *def; // [esp+168h] [ebp-ACh]
    trace_t revTrace; // [esp+16Ch] [ebp-A8h] BYREF
    col_context_t context; // [esp+1A4h] [ebp-70h] BYREF
    trace_t trace; // [esp+1CCh] [ebp-48h] BYREF

    iassert(body);
    def = Ragdoll_BodyDef(body);
    iassert(def);
    bone = body->bones;
    boneDef = def->boneDefs;
    curOrientation = Ragdoll_BodyBoneOrientations(body);
    boneOrientations = curOrientation;
    i = 0;
    while (i < body->numBones)
    {
        if (bone->rigidBody)
        {
            (v27) = boneDef->mirror;
            v26 = curOrientation->orientation;
            if (v27)
            {
                Ragdoll_QuatMul(v26, quatZRot, orientation);
            }
            else
            {
                orientation[0] = *v26;
                orientation[1] = v26[1];
                orientation[2] = v26[2];
                orientation[3] = v26[3];
            }
            QuatToAxis(orientation, boneMat);
            start[0] = curOrientation->origin[0];
            start[1] = curOrientation->origin[1];
            start[2] = curOrientation->origin[2];
            length = bone->length;
            end[0] = (float)(length * boneMat[0][0]) + curOrientation->origin[0];
            end[1] = (float)(length * boneMat[0][1]) + curOrientation->origin[1];
            end[2] = (float)(length * boneMat[0][2]) + curOrientation->origin[2];
            if (i)
            {
                center[3] = bone->length * 0.5;
                center[0] = (float)(center[3] * boneMat[0][0]) + start[0];
                center[1] = (float)(center[3] * boneMat[0][1]) + start[1];
                center[2] = (float)(center[3] * boneMat[0][2]) + start[2];
                CM_BoxTrace(
                    &trace,
                    boneOrientations->origin,
                    center,
                    vec3_origin,
                    vec3_origin,
                    0x280EC93,
                    &context);
                if (trace.startsolid)
                    return 0;
                iassert(!trace.allsolid);
                if (trace.fraction < 1.0)
                {
                    trace.fraction = trace.fraction * 0.80000001;
                    Vec3Lerp(boneOrientations->origin, center, trace.fraction, newPos);
                    Phys_ObjSetPosition(bone->rigidBody, newPos);
                }
            }
            else if (!Ragdoll_BoneTrace(&trace, &revTrace, start, end))
            {
                return 0;
            }
        }
        ++i;
        ++bone;
        ++boneDef;
        ++curOrientation;
    }
    pose = Ragdoll_BodyPose(body);
    Phys_Vec3ToNitrousVec(pose->origin, &sphere_center);
    v14 = 0.0f;
    v15 = 0.0f;
    v16 = 15.0f;
    sphere_center.x = sphere_center.x + 0.0;
    sphere_center.y = sphere_center.y + 0.0;
    sphere_center.z = sphere_center.z + 15.0;
    sphere_radius = 10.0f;
    bone = body->bones;
    i = 1;
    while (i < body->numBones)
    {
        if (bone->rigidBody)
        {
            userData = (PhysObjUserData *)bone->rigidBody;
            p_m_mat = &userData->body->m_mat;
            last_position = p_m_mat->w;
            v9 = last_position.x - sphere_center.x;
            v8 = last_position.y - sphere_center.y;
            v7 = last_position.z - sphere_center.z;
            dif.x = last_position.x - sphere_center.x;
            dif.y = last_position.y - sphere_center.y;
            dif.z = last_position.z - sphere_center.z;
            ndif = Vec3Length(&dif.x);
            if (ndif > sphere_radius)
            {
                v4 = (float)(ndif - sphere_radius) / ndif;
                last_position.x = last_position.x - (float)(v4 * dif.x);
                last_position.y = last_position.y - (float)(v4 * dif.y);
                last_position.z = last_position.z - (float)(v4 * dif.z);
            }
            v3 = userData->body;
            v3->m_last_position.x = last_position.x;
            v3->m_last_position.y = last_position.y;
            v3->m_last_position.z = last_position.z;
            if (   (fabs(userData->body->m_last_position.x)) > 100000.0
                || (fabs(userData->body->m_last_position.y)) > 100000.0
                || (fabs(userData->body->m_last_position.z)) > 100000.0)
            {
                phys_exec_debug_callback(userData->body);
            }
            //BG_EvalVehicleName();
        }
        ++i;
        ++bone;
    }
    body->m_bpg->m_flags |= 0x200u;
    return 1;
}

char __cdecl Ragdoll_BoneTrace(trace_t *trace, trace_t *revTrace, const float *start, const float *end)
{
    col_context_t context; // [esp+0h] [ebp-28h] BYREF
    ////col_context_t::col_context_t(&context);

    CM_BoxTrace(trace, start, end, vec3_origin, vec3_origin, 0x280EC93, &context);
    if ( trace->startsolid )
    {
        CM_BoxTrace(revTrace, end, start, vec3_origin, vec3_origin, 0x280EC93, &context);
        if ( revTrace->startsolid )
            return 0;
    }
    else if ( trace->fraction != 1.0 )
    {
        CM_BoxTrace(revTrace, end, start, vec3_origin, vec3_origin, 0x280EC93, &context);
        if ( revTrace->fraction != 1.0 )
            return 0;
    }
    return 1;
}

void __cdecl Ragdoll_PrintTunnelFail(RagdollBody *body)
{
    if ( ragdoll_dump_anims->current.enabled )
    {
        const cpose_t *pose = Ragdoll_BodyPose(body);
        DObj *obj = Ragdoll_BodyDObj(body);
        iassert(pose);
        iassert(obj);
        Com_Printf(
            20,
            "Ragdoll initial state in solid, using regular anim. Pos (%0.f %0.f %0.f)\n",
            pose->origin[0],
            pose->origin[1],
            pose->origin[2]);
        DObjDisplayAnim(obj, "Ragdoll anim tree: ");
    }
}

void __cdecl Ragdoll_UpdateVelocityCapture(RagdollBody *body, int __formal)
{
    BoneOrientation *snapshot; // [esp+0h] [ebp-4h]

    iassert(body);
    if ( body->stateFrames >= 1 )
    {
        body->velCaptureMsec = body->stateMsec;
        body->curOrientationBuffer ^= 1u;
        snapshot = Ragdoll_BodyBoneOrientations(body);
        Ragdoll_SnapshotAnimOrientations(body, snapshot);
        Ragdoll_BodyNewState(body, BS_TUNNEL_TEST);
    }
    else
    {
        body->curOrientationBuffer ^= 1u;
        snapshot = Ragdoll_BodyBoneOrientations(body);
        Ragdoll_SnapshotAnimOrientations(body, snapshot);
        if ( (body->flags & 4) != 0 )
        {
            body->velCaptureMsec = 1;
            body->curOrientationBuffer ^= 1u;
            snapshot = Ragdoll_BodyBoneOrientations(body);
            Ragdoll_SnapshotAnimOrientations(body, snapshot);
            Ragdoll_BodyNewState(body, BS_TUNNEL_TEST);
        }
    }
}

void __cdecl Ragdoll_SnapshotAnimOrientations(RagdollBody *body, BoneOrientation *snapshot)
{
    RagdollDef *def; // [esp+0h] [ebp-18h]
    DObj *obj; // [esp+4h] [ebp-14h]
    BoneDef *boneDef; // [esp+8h] [ebp-10h]
    const cpose_t *pose; // [esp+Ch] [ebp-Ch]
    int i; // [esp+10h] [ebp-8h]
    Bone *bone; // [esp+14h] [ebp-4h]

    iassert(body);

    def = Ragdoll_BodyDef(body);
    obj = Ragdoll_BodyDObj(body);

    if ( obj )
    {
        pose = Ragdoll_BodyPose(body);
        if ( def && obj && pose )
        {
            bone = body->bones;
            boneDef = def->boneDefs;
            i = 0;
            while ( i < body->numBones )
            {
                if ( bone->animBones[0] != 255 )
                    Ragdoll_GetDObjWorldBoneOriginQuat(
                        body->localClientNum,
                        pose,
                        obj,
                        bone->animBones[0],
                        snapshot->origin,
                        snapshot->orientation);
                ++i;
                ++bone;
                ++boneDef;
                ++snapshot;
            }
        }
        else
        {
            Ragdoll_BodyNewState(body, BS_DEAD);
        }
    }
}

char __cdecl Ragdoll_GetDObjWorldBoneOriginQuat(
                int localClientNum,
                const cpose_t *pose,
                DObj *obj,
                unsigned __int8 boneIndex,
                float *origin,
                float *quat)
{
    DObjAnimMat *mat; // [esp+4h] [ebp-4h]

    iassert(obj);
    iassert(pose);

    mat = Ragdoll_GetDObjLocalBoneMatrix(pose, obj, boneIndex);
    if ( !mat )
        return 0;

    quat[0] = mat->quat[0];
    quat[1] = mat->quat[1];
    quat[2] = mat->quat[2];
    quat[3] = mat->quat[3];

    origin[0] = mat->trans[0];
    origin[1] = mat->trans[1];
    origin[2] = mat->trans[2];

    return 1;
}

bool __cdecl Ragdoll_EnterDead(RagdollBody *body, RagdollBodyState s1, RagdollBodyState s2)
{
    int references; // [esp+0h] [ebp-8h]
    int ragdollDef; // [esp+4h] [ebp-4h]

    iassert(body);

    Ragdoll_RemoveBodyPhysics(body);

    references = body->references;
    ragdollDef = body->ragdollDef;

    memset(body, 0, sizeof(RagdollBody));

    body->references = references;
    body->ragdollDef = ragdollDef;

    return 1;
}

void __cdecl Ragdoll_RemoveBodyPhysics(RagdollBody *body)
{
    iassert(body);

    Ragdoll_DestroyPhysJoints(body);
    Ragdoll_DestroyPhysObjs(body);

    body->debug_hang_point = 0;
    body->hang_point = 0;
}

bool __cdecl Ragdoll_ExitDead(RagdollBody *body, RagdollBodyState s1, RagdollBodyState s2)
{
    RagdollDef *def; // [esp+0h] [ebp-4h]

    iassert(body);
    def = Ragdoll_BodyDef(body);
    iassert(def);

    body->numBones = def->numBones;
    body->numLerpBones = 0;
    body->numJoints = def->numJoints;

    return 1;
}

bool __cdecl Ragdoll_ExitDObjWait(RagdollBody *body, RagdollBodyState prevState, RagdollBodyState curState)
{
    RagdollDef *def; // [esp+0h] [ebp-18h]
    DObj *obj; // [esp+4h] [ebp-14h]
    BoneDef *boneDef; // [esp+8h] [ebp-10h]
    LerpBone *lerpBone; // [esp+Ch] [ebp-Ch]
    int i; // [esp+10h] [ebp-8h]
    int ia; // [esp+10h] [ebp-8h]
    Bone *bone; // [esp+14h] [ebp-4h]

    iassert(body);

    if ( curState == BS_DEAD )
        return 1;

    def = Ragdoll_BodyDef(body);
    iassert(def);
    obj = Ragdoll_BodyDObj(body);
    iassert(obj);

    bone = body->bones;
    boneDef = def->boneDefs;

    for ( i = 0; i < body->numBones; ++i )
    {
        bone->parentBone = boneDef->parentBone;
        bone->animBones[1] = 0;
        bone->animBones[0] = 0;
        if ( DObjGetBoneIndex(obj, boneDef->animBoneNames[0], bone->animBones, -1) && bone->animBones[0] != 255 )
        {
            if ( boneDef->animBoneNames[1] == -1 )
            {
                bone->animBones[1] = 0;
            }
            else if ( !DObjGetBoneIndex(obj, boneDef->animBoneNames[1], &bone->animBones[1], -1) || bone->animBones[1] == 255 )
            {
                bone->animBones[1] = -1;
                bone->animBones[0] = -1;
            }
        }
        ++bone;
        ++boneDef;
    }
    lerpBone = body->lerpBones;
    for ( ia = 0; ia < body->numLerpBones; ++ia )
    {
        lerpBone->animBone = 0;
        if ( !DObjGetBoneIndex(obj, def->baseLerpBoneDefs[ia].animBoneName, &lerpBone->animBone, -1)
            || lerpBone->animBone == 255 )
        {
            return 0;
        }
        lerpBone->parentBone = def->baseLerpBoneDefs[ia].parentBoneIndex;
        ++lerpBone;
    }
    Ragdoll_SnapshotBaseLerpOffsets(body);
    return 1;
}

bool __cdecl Ragdoll_ExitIdle(RagdollBody *body, RagdollBodyState curState, RagdollBodyState newState)
{
    iassert(body);

    if ( (unsigned int)newState <= BS_DOBJ_WAIT )
        return 1;
    if ( !Ragdoll_ValidateBodyObj(body) || !Ragdoll_BodyPoseValid(body) )
        return 0;
    if ( Ragdoll_CountPhysicsBodies() >= ragdoll_max_simulating->current.integer )
        return 0;
    if ( !Ragdoll_CreateBodyPhysics(body) )
        return 0;

    Ragdoll_SetCurrentPoseFromSnapshot(body, Ragdoll_BodyBoneOrientations(body));

    return 1;
}

bool __cdecl Ragdoll_EnterIdle(RagdollBody *body, RagdollBodyState prevState, RagdollBodyState curState)
{
    iassert(body);

    if ( Ragdoll_BodyHasPhysics(body) )
    {
        Ragdoll_SnapshotBonePositions(body, Ragdoll_BodyBoneOrientations(body));
    }

    Ragdoll_RemoveBodyPhysics(body);

    return 1;
}

void __cdecl Ragdoll_SnapshotBonePositions(RagdollBody *body, BoneOrientation *boneSnapshot)
{
    float *dest; // [esp+0h] [ebp-2Ch]
    RagdollDef *def; // [esp+8h] [ebp-24h]
    BoneOrientation *snapshot; // [esp+Ch] [ebp-20h]
    float boneRot[4]; // [esp+10h] [ebp-1Ch] BYREF
    BoneDef *boneDef; // [esp+20h] [ebp-Ch]
    int i; // [esp+24h] [ebp-8h]
    Bone *bone; // [esp+28h] [ebp-4h]

    iassert(body && boneSnapshot);

    if ( Ragdoll_BodyHasPhysics(body) )
    {
        def = Ragdoll_BodyDef(body);
        iassert(def);
        bone = body->bones;
        boneDef = def->boneDefs;
        snapshot = boneSnapshot;
        i = 0;
        while ( i < body->numBones )
        {
            if ( bone->rigidBody )
            {
                Phys_ObjGetInterpolatedState(bone->rigidBody, snapshot->origin, boneRot);
                dest = snapshot->orientation;
                if ( boneDef->mirror )
                {
                    Ragdoll_QuatMul(boneRot, quatZRot, dest);
                }
                else
                {
                    *dest = boneRot[0];
                    snapshot->orientation[1] = boneRot[1];
                    snapshot->orientation[2] = boneRot[2];
                    snapshot->orientation[3] = boneRot[3];
                }
            }
            ++i;
            ++bone;
            ++snapshot;
            ++boneDef;
        }
        Ragdoll_SnapshotBaseLerpBones(body, boneSnapshot);
    }
}

void __cdecl Ragdoll_UpdateDObjWait(RagdollBody *body, int __formal)
{
    iassert(body);

    if ( body->stateFrames <= 3 )
    {
        if ( Ragdoll_ValidateBodyObj(body) )
            Ragdoll_BodyNewState(body, BS_VELOCITY_CAPTURE);
    }
    else
    {
        if ( body->obj )
            Com_PrintWarning(20, "Ragdoll activation timed out waiting for dobj 0x%x\n", body->dobj);
        else
            Com_PrintWarning(20, "Ragdoll activation timed out waiting for dobj 0x%x\n", body->obj);
        Ragdoll_BodyNewState(body, BS_DEAD);
    }
}

bool __cdecl Ragdoll_EnterDobjWait(RagdollBody *body, RagdollBodyState s1, RagdollBodyState s2)
{
    Ragdoll_UpdateDObjWait(body, 0);
    return 1;
}

void __cdecl Ragdoll_UpdateRunning(RagdollBody *body, int msec)
{
    iassert(body);

    if ( Ragdoll_CheckIdle(body, msec) )
    {
        Ragdoll_BodyNewState(body, BS_IDLE);
    }
    else
    {
        Ragdoll_FilterBonePositions(body);
        Ragdoll_UpdateFriction(body);
        Ragdoll_LaunchUpdate(body);
        Ragdoll_AttachUpdate(body);
        Ragdoll_DebugRender(body);
    }
}

void __cdecl Ragdoll_UpdateFriction(RagdollBody *body)
{
    JointType type; // [esp+10h] [ebp-28h]
    RagdollDef *def; // [esp+18h] [ebp-20h]
    float frictionVec[3]; // [esp+1Ch] [ebp-1Ch] BYREF
    Joint *joint; // [esp+28h] [ebp-10h]
    float lerpScale; // [esp+2Ch] [ebp-Ch]
    int i; // [esp+30h] [ebp-8h]
    JointDef *jointDef; // [esp+34h] [ebp-4h]

    iassert(body);
    def = Ragdoll_BodyDef(body);
    iassert(def);

    jointDef = def->jointDefs;
    joint = body->joints;
    lerpScale = 1.0f - (float)body->stateMsec / (float)ragdoll_jointlerp_time->current.integer;
    lerpScale = lerpScale * lerpScale;
    lerpScale = lerpScale * 0.9f + 0.1f;

    static const float waterFrictionScale = 0.01f;

    if ( (body->flags & 2) != 0 )
        lerpScale = waterFrictionScale;
    i = 0;
    while ( i < def->numJoints )
    {
        type = jointDef->type;
        if ( type == RAGDOLL_JOINT_HINGE )
        {
            if ( joint->joint )
                Phys_SetHingeParams((rigid_body_constraint_ragdoll *)joint->joint, 0.0, jointDef->axisFriction[0] * lerpScale);
        }
        else if ( type == RAGDOLL_JOINT_SWIVEL )
        {
            frictionVec[0] = lerpScale * jointDef->axisFriction[0];
            frictionVec[1] = lerpScale * jointDef->axisFriction[1];
            frictionVec[2] = lerpScale * jointDef->axisFriction[2];
            if ( joint->joint )
                Phys_SetAngularMotorParams((rigid_body_constraint_ragdoll *)joint->joint, vec3_origin, frictionVec);
        }
        ++i;
        ++jointDef;
        ++joint;
    }
}

static const int hangBone = 3;
static const int renderflag = 255;
void    Ragdoll_DebugRender(RagdollBody *body)
{
    int joint2; // edx
    environment_rigid_body *envRigBody; // eax
    phys_vec3 v4; // [esp-2Ch] [ebp-6Ch] BYREF
    phys_vec3 world_pos; // [esp+0h] [ebp-40h]
    environment_rigid_body *v9; // [esp+10h] [ebp-30h]
    environment_rigid_body **v10; // [esp+14h] [ebp-2Ch]
    int rigidBody; // [esp+18h] [ebp-28h]
    rigid_body *body1; // [esp+1Ch] [ebp-24h]
    PhysObjUserData *userData; // [esp+20h] [ebp-20h]
    int id; // [esp+24h] [ebp-1Ch]
    int v15; // [esp+28h] [ebp-18h]
    int v16; // [esp+2Ch] [ebp-14h]
    int i; // [esp+30h] [ebp-10h]
    Joint joint; // [esp+34h] [ebp-Ch] BYREF

    if ( ragdoll_debug->current.integer > 0 )
    {
        if ( ragdoll_debug->current.integer == 4 )
        {
            for ( i = 0; i < body->numJoints; ++i )
            {
                joint2 = body->joints[i].joint2;
                v15 = body->joints[i].joint;
                v16 = joint2;
                Phys_JointDebugRender(v15);
            }
        }
        for ( id = 0; id < body->numBones; ++id )
        {
            if ( (renderflag & (1 << id)) != 0 )
            {
                userData = (PhysObjUserData *)body->bones[id].rigidBody;
                body1 = (rigid_body *)userData;
                debug_render(userData);
            }
        }
    }
    rigidBody = body->bones[hangBone].rigidBody;
    v10 = (environment_rigid_body **)rigidBody;
    if ( ragdoll_debug->current.integer <= 1 )
    {
        if ( body->debug_hang_point )
        {
            //rigid_body::set_flag(body->debug_hang_point->b1, 0x40u, 0);
            body->debug_hang_point->b1->set_flag(0x40, 0);
            phys_sys::destroy(body->debug_hang_point);
            body->debug_hang_point = 0;
        }
    }
    else if ( !body->debug_hang_point )
    {
        v9 = *v10;
        world_pos = v9->m_mat.w;
        world_pos.z = world_pos.z + 60.0;
        envRigBody = phys_sys::get_environment_rigid_body();
        body->debug_hang_point = phys_sys::create_rbc_point(v9, envRigBody, 0);
        memset(&v4, 0, 12);
        //rigid_body_constraint_point::set(body->debug_hang_point, &v4, &world_pos);
        body->debug_hang_point->set(&v4, &world_pos);
        //rigid_body::set_flag(v9, 0x40u, 1);
        v9->set_flag(0x40, 1);
    }
}

bool __cdecl Ragdoll_CheckIdle(RagdollBody *body, int msec)
{
    float tvel[3]; // [esp+4h] [ebp-2Ch] BYREF
    float avel[3]; // [esp+10h] [ebp-20h] BYREF
    float zvel; // [esp+1Ch] [ebp-14h]
    int i; // [esp+20h] [ebp-10h]
    int stableBoneCount; // [esp+24h] [ebp-Ch]
    int totalBoneCount; // [esp+28h] [ebp-8h]
    bool stable; // [esp+2Fh] [ebp-1h]

    if ( body->debug_hang_point )
        return 0;
    if ( body->hang_point )
        return 0;
    if ( body->rope_id )
        return 0;
    zvel = 0.0f;
    totalBoneCount = 0;
    for ( i = 0; i < body->numBones; ++i )
    {
        if ( body->bones[i].rigidBody )
        {
            ++totalBoneCount;
            Phys_ObjGetVelocities(body->bones[i].rigidBody, tvel, avel);
            zvel = zvel + tvel[2];
        }
    }
    zvel = zvel / (float)totalBoneCount;
    if ( zvel < -2500.0 )
        return 1;
    body->flags &= ~2u;
    if ( body->stateMsec > ragdoll_max_life->current.integer )
        return 1;
    stableBoneCount = 0;
    for ( i = 0; i < body->numBones; ++i )
    {
        if ( body->bones[i].rigidBody && Phys_ObjIsAsleepSingle(body->bones[i].rigidBody) )
            ++stableBoneCount;
    }
    stable = (float)stableBoneCount > (float)((float)totalBoneCount * 0.75);
    if ( (float)stableBoneCount <= (float)((float)totalBoneCount * 0.75) )
        body->stableTime = 0;
    else
        body->stableTime += msec;
    return body->stableTime > 250;
}

void __cdecl Ragdoll_FilterBonePositions(RagdollBody *body)
{
    Ragdoll_SnapshotBonePositions(body, Ragdoll_BodyBoneOrientations(body));
}

char __cdecl Ragdoll_BodyNewState(RagdollBody *body, RagdollBodyState state)
{
    RagdollBodyState prevState; // [esp+4h] [ebp-4h]

    iassert(body);
    bcassert(state, RAGDOLL_NUM_STATES);

    prevState = body->state;

    bcassert(prevState, RAGDOLL_NUM_STATES);

    if ( prevState == state )
        return 1;

    if ( stateEntries[prevState].exitFunc && !stateEntries[prevState].exitFunc(body, prevState, state) )
        return 0;

    body->state = state;
    if ( !stateEntries[state].enterFunc || stateEntries[state].enterFunc(body, prevState, state) )
    {
        body->stateFrames = 0;
        body->stateMsec = 0;
        body->stableTime = 0;
        return 1;
    }
    else
    {
        if ( body->state == state )
            body->state = prevState;
        return 0;
    }
}

void __cdecl Ragdoll_BodyUpdate(int msec, RagdollBody *body)
{
    StateEnt *entry; // [esp+0h] [ebp-8h]
    RagdollBodyState prevState; // [esp+4h] [ebp-4h]

    iassert(body);
    bcassert(body->state, RAGDOLL_NUM_STATES);

    do
    {
        prevState = body->state;
        entry = &stateEntries[prevState];
        if ( entry->updateFunc )
            entry->updateFunc(body, msec);
    }
    while ( prevState != body->state );

    ++body->stateFrames;
    body->stateMsec += msec;
}

void __cdecl Ragdoll_Update(int msec)
{
    iassert(Sys_IsMainThread());

    if ( ragdollInited && ragdoll_enable->current.enabled && cg_paused && !cg_paused->current.integer )
    {
        for (RagdollBody *body : g_ragdoll_body_pool)
        {
            iassert(Ragdoll_BodyInUse(body));
            Ragdoll_BodyUpdate(msec, body);
        }
    }
}

void __cdecl Ragdoll_RemoveConstraintsForRope(int rope_id)
{
    for (RagdollBody *body : g_ragdoll_body_pool)
    {
        iassert(Ragdoll_BodyInUse(body));
        
        if (body->rope_id - 1 == rope_id)
        {
            iassert(body->rbc_dist);
            phys_sys::destroy((rigid_body_constraint_distance *const)body->rbc_dist);
            body->rbc_dist = NULL;
            body->rope_id = 0;
            body->stateMsec = 0;
        }
    }
}

int __cdecl Ragdoll_GetRBForBone(int entnum, unsigned int boneName)
{
    for (RagdollBody *body : g_ragdoll_body_pool)
    {
        iassert(Ragdoll_BodyInUse(body));

        if (entnum == body->dobj)
        {
            RagdollDef *def = Ragdoll_BodyDef(body);

            for (int bi = 0; bi < def->numBones; bi++)
            {
                if (def->boneDefs[bi].animBoneNames[1] == boneName)
                    return body->bones[bi].rigidBody;
            }
            return 0;
        }
    }

    return 0;
}

