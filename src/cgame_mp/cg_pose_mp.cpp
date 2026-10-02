#include "cg_pose_mp.h"
#include <xanim/dobj_utils.h>
#include <ragdoll/ragdoll_controller.h>
#include "cg_local_mp.h"
#include <physics/xdoll.h>
#include <universal/com_math_anglevectors.h>
#include <cgame/cg_drawtools.h>
#include <client_mp/cl_pose_mp.h>
#include <xanim/dobj_skel.h>

float recoilVec[3] = { -1.0, 0.0, 0.0 };
float ysScale = 1.0f;
float pcScale = 1.0f;
float ycFwdScale = 1.0f;
float pcFwdScale = -1.3f;



void __cdecl BG_Player_DoControllers(const CEntPlayerInfo *player, const DObj *obj, int *partBits)
{
    int v3; // [esp+0h] [ebp-14h]
    bool v4; // [esp+4h] [ebp-10h]
    clientControllers_t *control; // [esp+10h] [ebp-4h]

    control = player->control;
    v4 = player->control->tag_origin_angles[0] == 0.0
        && player->control->tag_origin_angles[1] == 0.0
        && player->control->tag_origin_angles[2] == 0.0;
    if ( !v4
        || (control->tag_origin_offset[0] != 0.0
         || control->tag_origin_offset[1] != 0.0
         || control->tag_origin_offset[2] != 0.0
            ? (v3 = 0)
            : (v3 = 1),
                !v3) )
    {
        DObjSetLocalTag(obj, partBits, 0, control->tag_origin_offset, control->tag_origin_angles);
    }
}

void __cdecl CG_Destructible_DoControllers(const cpose_t *pose, const DObj *obj, int *partBits)
{
    float controlAngles[3]; // [esp+8h] [ebp-28h] BYREF
    unsigned __int8 boneIndex; // [esp+17h] [ebp-19h] BYREF
    Destructible_BonePose *bonepose; // [esp+18h] [ebp-18h]
    int i; // [esp+1Ch] [ebp-14h]
    DestructiblePose *dpose; // [esp+20h] [ebp-10h]
    float noTrans[3]; // [esp+24h] [ebp-Ch] BYREF

    memset(noTrans, 0, sizeof(noTrans));
    if ( pose->destructiblePose )
    {
        dpose = CG_GetDestructiblePose(pose->destructiblePose);
        for ( i = 0; i < 32; ++i )
        {
            bonepose = &dpose->bones[i];
            if ( !bonepose->boneid )
                break;
            boneIndex = 0;
            if ( DObjGetBoneIndex(obj, bonepose->boneid, &boneIndex, -1) )
            {
                if ( DObjSetRotTransIndex(obj, partBits, boneIndex) )
                {
                    controlAngles[0] = bonepose->angles_offset[0] + bonepose->angles_original[0];
                    controlAngles[1] = bonepose->angles_offset[1] + bonepose->angles_original[1];
                    controlAngles[2] = bonepose->angles_offset[2] + bonepose->angles_original[2];
                    DObjSetLocalTagInternal(obj, noTrans, controlAngles, boneIndex);
                }
            }
        }
    }
}

void __cdecl CG_Actor_DoControllers(const cpose_t *pose, const DObj *obj, int *partBits)
{
    float *v3; // eax
    int nextWaterHeightCheck; // [esp+0h] [ebp-20h]
    float v5; // [esp+4h] [ebp-1Ch]
    float offset[3]; // [esp+10h] [ebp-10h] BYREF
    DObjAnimMat *mat; // [esp+1Ch] [ebp-4h]

    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_pose_mp.cpp", 520, 0, "%s", "obj") )
        __debugbreak();
    mat = DObjGetRotTransArray(obj);
    if ( mat )
    {
        if ( DObjSetRotTransIndex(obj, partBits, 0) )
        {
            //v5 = pose->turret.$251974A72D8ACF7EC8C19B3B5F3F224B::angles.yaw * 0.0087266462;
            v5 = DEG2RAD(pose->actor.pitch) * 0.5;
            mat->quat[0] = 0.0f;
            mat->quat[2] = 0.0f;
            v3 = &mat->quat[1];
            mat->quat[3] = cos(v5);
            *v3 = sin(v5);
            nextWaterHeightCheck = pose->player.nextWaterHeightCheck;
            offset[0] = 0.0f;
            offset[1] = 0.0f;
            LODWORD(offset[2]) = nextWaterHeightCheck;
            DObjSetTrans(mat, offset);
        }
    }
}

void __cdecl CG_DoControllers(const cpose_t *pose, const DObj *obj, int *partBits)
{
    unsigned __int16 EntNum; // ax
    Destructible *v4; // [esp+0h] [ebp-38h]
    int xdoll_handle; // [esp+10h] [ebp-28h]
    int pi; // [esp+14h] [ebp-24h]
    int count; // [esp+18h] [ebp-20h]
    centity_s *cent; // [esp+1Ch] [ebp-1Ch]
    int setPartBits[5]; // [esp+24h] [ebp-14h] BYREF
    int savedregs; // [esp+38h] [ebp+0h] BYREF

    DObjGetSetBones(obj, setPartBits);
    switch ( pose->eType )
    {
        case 1u:
            CG_Player_DoControllers(pose, obj, partBits);
            break;
        case 0xBu:
            goto $LN10_13;
        case 0xCu:
        case 0xEu:
        case 0x10u:
            CG_Vehicle_DoControllers(pose, obj, partBits);
            break;
        case 0x11u:
        case 0x13u:
            CG_Actor_DoControllers(pose, obj, partBits);
$LN10_13:
            CG_mg42_DoControllers(pose, obj, partBits);
            break;
        default:
            break;
    }
    if ( pose->destructiblePose )
        CG_Destructible_DoControllers(pose, obj, partBits);
    CG_DoBaseOriginController(pose, obj, setPartBits);
    if ( pose->isRagdoll && (pose->ragdollHandle > 0 || pose->killcamRagdollHandle > 0) )
        Ragdoll_DoControllers(pose, obj, partBits);
    if ( pose->eType != 21 )
    {
        EntNum = DObjGetEntNum(obj);
        cent = CG_GetEntity(pose->localClientNum, EntNum - 1);
        v4 = cent ? cent->destructible : 0;
        if ( v4 && (unsigned __int8)Destructible_XDollActive(v4) )
        {
            count = Destructible_GetPieceCount(v4);
            for ( pi = 0; pi < count; ++pi )
            {
                xdoll_handle = Destructible_GetXDollHandle(v4, pi);
                if ( xdoll_handle )
                    XDoll_DoControllers(xdoll_handle, partBits, pose->localClientNum);
            }
        }
    }
}

void __cdecl CG_Player_DoControllers(const cpose_t *pose, const DObj *obj, int *partBits)
{
    cg_s *cgameGlob; // [esp+8h] [ebp-8h]
    centity_s *ent; // [esp+Ch] [ebp-4h]

    if ( pose->fx.triggerTime )
    {
        cgameGlob = CG_GetLocalClientGlobals(pose->localClientNum);
        ent = CG_GetEntity(pose->localClientNum, obj->entnum - 1);
        CG_UpdateMinigunTags(cgameGlob, ent, obj, partBits);
        BG_Player_DoControllers(&pose->player, obj, partBits);
    }
}

#if 0
void    CG_Vehicle_DoControllers(const cpose_t *pose, const DObj *obj, int *partBits)
{
    unsigned __int16 EntNum; // ax
    int v5; // xmm0_4
    float v6[3]; // [esp-Ch] [ebp-2A8h] BYREF
    float angles[3]; // [esp+0h] [ebp-29Ch]
    int ChildBones; // [esp+Ch] [ebp-290h]
    unsigned __int8 v9[4]; // [esp+10h] [ebp-28Ch] BYREF
    float v10; // [esp+14h] [ebp-288h]
    float v11[3]; // [esp+18h] [ebp-284h] BYREF
    float v12; // [esp+24h] [ebp-278h]
    float v13[3]; // [esp+28h] [ebp-274h] BYREF
    float v14; // [esp+34h] [ebp-268h]
    int k; // [esp+38h] [ebp-264h]
    int n; // [esp+40h] [ebp-25Ch]
    int m; // [esp+44h] [ebp-258h]
    DObjAnimMat *skel; // [esp+48h] [ebp-254h]
    unsigned __int8 v20[4]; // [esp+4Ch] [ebp-250h] BYREF
    int j; // [esp+50h] [ebp-24Ch] BYREF
    int numChildren; // [esp+54h] [ebp-248h]
    unsigned __int8 children[4]; // [esp+58h] [ebp-244h]
    float offset[4]; // [esp+5Ch] [ebp-240h]
    float v25; // [esp+6Ch] [ebp-230h]
    float v26; // [esp+70h] [ebp-22Ch]
    int v27; // [esp+74h] [ebp-228h]
    int v28; // [esp+78h] [ebp-224h]
    int v29; // [esp+7Ch] [ebp-220h]
    float v30; // [esp+80h] [ebp-21Ch]
    float dist; // [esp+84h] [ebp-218h]
    float4 v32; // [esp+88h] [ebp-214h]
    float4 partPos; // [esp+98h] [ebp-204h]
    float4 trans; // [esp+A8h] [ebp-1F4h]
    float *v35; // [esp+B8h] [ebp-1E4h]
    const DObjAnimMat *mtx; // [esp+BCh] [ebp-1E0h]
    unsigned int boneIndex; // [esp+C0h] [ebp-1DCh]
    centity_s *cent; // [esp+C4h] [ebp-1D8h]
    const DObjAnimMat *boneMtxList; // [esp+C8h] [ebp-1D4h]
    const DObjAnimMat *remote_boneMtxList; // [esp+CCh] [ebp-1D0h]
    unsigned int boneCount; // [esp+D0h] [ebp-1CCh]
    XModel *model; // [esp+D4h] [ebp-1C8h]
    vector4 invAxis; // [esp+D8h] [ebp-1C4h]
    float4 axisW; // [esp+118h] [ebp-184h]
    float *v45; // [esp+128h] [ebp-174h]
    float4 axisZ; // [esp+12Ch] [ebp-170h]
    float *v47; // [esp+13Ch] [ebp-160h]
    vector4 axis; // [esp+140h] [ebp-15Ch]
    unsigned int v49[3]; // [esp+184h] [ebp-118h]
    float tempAxis[4][3]; // [esp+190h] [ebp-10Ch] BYREF
    float suspTravel; // [esp+1C0h] [ebp-DCh]
    float minigunAngles[3]; // [esp+1C4h] [ebp-D8h] BYREF
    float barrelOffset[5]; // [esp+1D0h] [ebp-CCh] BYREF
    float gunnerTurretAngles[4][3]; // [esp+1E4h] [ebp-B8h] BYREF
    unsigned int *v55; // [esp+214h] [ebp-88h]
    float gunnerBarrelAngles[4][3]; // [esp+218h] [ebp-84h] BYREF
    int i; // [esp+248h] [ebp-54h]
    float steerYaw; // [esp+24Ch] [ebp-50h]
    float steerAnglesPitch[3]; // [esp+250h] [ebp-4Ch] BYREF
    float steerAnglesYaw[3]; // [esp+25Ch] [ebp-40h] BYREF
    float bodyAngles[3]; // [esp+268h] [ebp-34h] BYREF
    float barrelAngles[3]; // [esp+274h] [ebp-28h] BYREF
    float turretAngles[4]; // [esp+280h] [ebp-1Ch] BYREF
    _UNKNOWN *v64; // [esp+290h] [ebp-Ch]
    const cpose_t *posea; // [esp+294h] [ebp-8h]
    int *partBitsa; // [esp+29Ch] [ebp+0h]

    v64 = a1;
    posea = (const cpose_t *)partBitsa;
    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_pose_mp.cpp", 89, 0, "%s", "obj") )
        __debugbreak();
    memset(turretAngles, 0, 12);
    memset(barrelAngles, 0, sizeof(barrelAngles));
    memset(bodyAngles, 0, sizeof(bodyAngles));
    memset(steerAnglesYaw, 0, sizeof(steerAnglesYaw));
    memset(steerAnglesPitch, 0, sizeof(steerAnglesPitch));
    bodyAngles[0] = (float)pose->vehicle.pitch * 0.0054931641;
    bodyAngles[2] = (float)pose->vehicle.roll * 0.0054931641;
    barrelAngles[0] = (float)pose->vehicle.barrelPitch * 0.0054931641;
    turretAngles[1] = (float)pose->vehicle.yaw * 0.0054931641;
    steerYaw = (float)pose->vehicle.steerYaw * 0.0054931641;
    steerAnglesPitch[0] = (float)pose->vehicle.steerPitch * 0.0054931641;
    for ( i = 0; i < 4; ++i )
    {
        v55 = gunnerBarrelAngles[i];
        *v55 = 0;
        v55[1] = 0;
        v55[2] = 0;
        LODWORD(barrelOffset[4]) = gunnerTurretAngles[i];
        *(unsigned int *)LODWORD(barrelOffset[4]) = 0;
        *(unsigned int *)(LODWORD(barrelOffset[4]) + 4) = 0;
        *(unsigned int *)(LODWORD(barrelOffset[4]) + 8) = 0;
        gunnerBarrelAngles[i][0] = (float)pose->vehicle.gunnerPitch[i] * 0.0054931641;
        gunnerTurretAngles[i][1] = (float)pose->vehicle.gunnerYaw[i] * 0.0054931641;
    }
    DObjSetLocalTag(obj, partBits, pose->vehicle.tag_body, vec3_origin, bodyAngles);
    DObjSetLocalTag(obj, partBits, pose->vehicle.tag_turret, vec3_origin, turretAngles);
    DObjSetLocalTag(obj, partBits, pose->vehicle.tag_barrel, vec3_origin, barrelAngles);
    if ( pose->vehicle.barrelRecoil > 0.0 )
    {
        barrelOffset[3] = pose->vehicle.barrelRecoil;
        barrelOffset[0] = barrelOffset[3] * recoilVec[0];
        barrelOffset[1] = barrelOffset[3] * *(float *)&dword_E03414;
        barrelOffset[2] = barrelOffset[3] * *(float *)&dword_E03418;
        DObjSetLocalTag(obj, partBits, pose->vehicle.tag_barrel_recoil, barrelOffset, vec3_origin);
    }
    for ( i = 0; i < 4; ++i )
    {
        DObjSetLocalTag(obj, partBits, pose->vehicle.tag_gunner_turret[i], vec3_origin, gunnerTurretAngles[i]);
        DObjSetLocalTag(obj, partBits, pose->vehicle.tag_gunner_barrel[i], vec3_origin, gunnerBarrelAngles[i]);
    }
    if ( pose->vehicle.tag_minigun_spin != 254 )
    {
        minigunAngles[0] = 0.0f;
        minigunAngles[1] = 0.0f;
        minigunAngles[2] = (float)pose->vehicle.minigun_rotation * 0.0054931641;
        DObjSetLocalTag(obj, partBits, pose->vehicle.tag_minigun_spin, vec3_origin, minigunAngles);
    }
    suspTravel = pose->vehicle.g_msgTime;
    AnglesToAxis(pose->angles, tempAxis);
    v49[2] = tempAxis[3];
    v49[1] = pose->origin;
    tempAxis[3][0] = pose->origin[0];
    tempAxis[3][1] = pose->origin[1];
    tempAxis[3][2] = pose->origin[2];
    v49[0] = tempAxis;
    *(_QWORD *)axis.x.v = *(_QWORD *)&tempAxis[0][0];
    *(_QWORD *)&axis.x.unitVec[2].packed = __PAIR64__(0, LODWORD(tempAxis[0][2]));
    *(_QWORD *)axis.y.v = *(_QWORD *)&tempAxis[1][0];
    *(_QWORD *)&axis.y.unitVec[2].packed = __PAIR64__(0, LODWORD(tempAxis[1][2]));
    *(_QWORD *)axis.z.v = *(_QWORD *)&tempAxis[2][0];
    *(_QWORD *)&axis.z.unitVec[2].packed = __PAIR64__(0, LODWORD(tempAxis[2][2]));
    *(_QWORD *)axis.w.v = *(_QWORD *)&tempAxis[3][0];
    *(_QWORD *)&axis.w.unitVec[2].packed = __PAIR64__(LODWORD(1.0f), LODWORD(tempAxis[3][2]));
    v47 = tempAxis[2];
    *(_QWORD *)axisZ.v = *(_QWORD *)&tempAxis[2][0];
    *(_QWORD *)&axisZ.unitVec[2].packed = __PAIR64__(0, LODWORD(tempAxis[2][2]));
    v45 = tempAxis[3];
    *(_QWORD *)axisW.v = *(_QWORD *)&tempAxis[3][0];
    *(_QWORD *)&axisW.unitVec[2].packed = __PAIR64__(0, LODWORD(tempAxis[3][2]));
    *(_QWORD *)invAxis.x.v = __PAIR64__(LODWORD(tempAxis[1][0]), LODWORD(tempAxis[0][0]));
    *(_QWORD *)&invAxis.x.unitVec[2].packed = __PAIR64__(LODWORD(1.0f), LODWORD(tempAxis[2][0]));
    *(_QWORD *)invAxis.y.v = __PAIR64__(LODWORD(tempAxis[1][1]), LODWORD(tempAxis[0][1]));
    *(_QWORD *)&invAxis.y.unitVec[2].packed = __PAIR64__(LODWORD(1.0f), LODWORD(tempAxis[2][1]));
    *(_QWORD *)invAxis.z.v = __PAIR64__(LODWORD(tempAxis[1][2]), LODWORD(tempAxis[0][2]));
    *(_QWORD *)&invAxis.z.unitVec[2].packed = __PAIR64__(LODWORD(1.0f), LODWORD(tempAxis[2][2]));
    model = DObjGetModel(obj, 0);
    boneCount = XModelNumBones((const cpose_t *)model);
    remote_boneMtxList = XModelGetBasePose(model);
    boneMtxList = remote_boneMtxList;
    EntNum = DObjGetEntNum(obj);
    cent = CG_GetEntity(pose->localClientNum, EntNum - 1);
    if ( cent->nitrousVeh )
    {
        if ( DObjGetRotTransArray(obj) )
        {
            for ( k = 0; k < 6; ++k )
            {
                boneIndex = pose->vehicle.wheelBoneIndex[k];
                if ( boneIndex < 0xFE && DObjSetRotTransIndex(obj, partBits, boneIndex) )
                {
                    v14 = pose->vehicle.wheelHeight[k];
                    v13[0] = 0.0f;
                    v13[1] = 0.0f;
                    v13[2] = v14;
                    v12 = pose->vehicle.nitrousWheelYaw[k];
                    v11[0] = 0.0f;
                    v11[1] = v12;
                    v11[2] = 0.0f;
                    DObjSetLocalTagInternal(obj, v13, v11, boneIndex);
                    v10 = pose->vehicle.nitrousWheelRotation[k];
                    v11[0] = v10;
                    v11[1] = 0.0f;
                    v11[2] = 0.0f;
                    ChildBones = DObjGetChildBones(obj, boneIndex, v9, 4);
                    for ( angles[2] = 0.0; SLODWORD(angles[2]) < ChildBones; ++LODWORD(angles[2]) )
                    {
                        if ( DObjSetRotTransIndex(obj, partBits, v9[LODWORD(angles[2])]) )
                            DObjSetLocalTagInternal(obj, vec3_origin, v11, v9[LODWORD(angles[2])]);
                    }
                }
            }
            for ( angles[1] = 0.0; SLODWORD(angles[1]) < 4; ++LODWORD(angles[1]) )
            {
                boneIndex = pose->vehicle.tag_extra_tank_wheels[LODWORD(angles[1])];
                if ( boneIndex < 0xFE )
                {
                    angles[0] = pose->vehicle.nitrousWheelRotation[LODWORD(angles[1])] * pose->vehicle.extra_wheel_rot_scale;
                    v6[0] = angles[0];
                    v6[1] = 0.0f;
                    v6[2] = 0.0f;
                    if ( DObjSetRotTransIndex(obj, partBits, boneIndex) )
                        DObjSetLocalTagInternal(obj, vec3_origin, v6, boneIndex);
                }
            }
        }
    }
    else
    {
        for ( i = 0; i < 6; ++i )
        {
            boneIndex = pose->vehicle.wheelBoneIndex[i];
            if ( boneIndex < 0xFE && DObjSetRotTransIndex(obj, partBits, boneIndex) )
            {
                mtx = &boneMtxList[boneIndex];
                v35 = (float*)mtx->trans;
                *(_QWORD *)trans.v = *(_QWORD *)mtx->trans;
                trans.u[2] = LODWORD(mtx->trans[2]);
                trans.u[3] = 0;
                v32.v[0] = (float)((float)((float)(trans.v[0] * axis.x.v[0]) + (float)(trans.v[1] * axis.y.v[0]))
                                                 + (float)(trans.v[2] * axis.z.v[0]))
                                 + axis.w.v[0];
                v32.v[1] = (float)((float)((float)(trans.v[0] * axis.x.v[1]) + (float)(trans.v[1] * axis.y.v[1]))
                                                 + (float)(trans.v[2] * axis.z.v[1]))
                                 + axis.w.v[1];
                v32.v[2] = (float)((float)((float)(trans.v[0] * axis.x.v[2]) + (float)(trans.v[1] * axis.y.v[2]))
                                                 + (float)(trans.v[2] * axis.z.v[2]))
                                 + axis.w.v[2];
                v32.v[3] = (float)((float)((float)(trans.v[0] * axis.x.v[3]) + (float)(trans.v[1] * axis.y.v[3]))
                                                 + (float)(trans.v[2] * axis.z.v[3]))
                                 + axis.w.v[3];
                partPos = v32;
                dist = (float)(suspTravel + 40.0) * pose->vehicle.wheelHeight[i];
                v30 = 40.0 - suspTravel;
                v5 = (float)(dist - (float)(40.0 - suspTravel)) < 0.0 ? LODWORD(v30) : LODWORD(dist);
                v29 = v5;
                v28 = v5;
                partPos.v[0] = (float)(40.0 * axisZ.v[0]) + partPos.v[0];
                partPos.v[1] = (float)(40.0 * axisZ.v[1]) + partPos.v[1];
                partPos.v[2] = (float)(40.0 * axisZ.v[2]) + partPos.v[2];
                v27 = -v5;
                partPos.v[0] = (float)((-v5) * axisZ.v[0]) + partPos.v[0];
                partPos.v[1] = (float)((-v5) * axisZ.v[1]) + partPos.v[1];
                partPos.v[2] = (float)((-v5) * axisZ.v[2]) + partPos.v[2];
                partPos.v[0] = partPos.v[0] - axisW.v[0];
                partPos.v[1] = partPos.v[1] - axisW.v[1];
                partPos.v[2] = partPos.v[2] - axisW.v[2];
                offset[2] = (float)((float)(partPos.v[0] * invAxis.x.v[0]) + (float)(partPos.v[1] * invAxis.y.v[0]))
                                    + (float)(partPos.v[2] * invAxis.z.v[0]);
                offset[3] = (float)((float)(partPos.v[0] * invAxis.x.v[1]) + (float)(partPos.v[1] * invAxis.y.v[1]))
                                    + (float)(partPos.v[2] * invAxis.z.v[1]);
                v25 = (float)((float)(partPos.v[0] * invAxis.x.v[2]) + (float)(partPos.v[1] * invAxis.y.v[2]))
                        + (float)(partPos.v[2] * invAxis.z.v[2]);
                v26 = (float)((float)(partPos.v[0] * invAxis.x.v[3]) + (float)(partPos.v[1] * invAxis.y.v[3]))
                        + (float)(partPos.v[2] * invAxis.z.v[3]);
                partPos.v[0] = offset[2] - trans.v[0];
                partPos.v[1] = offset[3] - trans.v[1];
                partPos.v[2] = v25 - trans.v[2];
                partPos.v[3] = v26 - trans.v[3];
                *(float *)&j = offset[2] - trans.v[0];
                *(float *)&numChildren = offset[3] - trans.v[1];
                *(float *)children = v25 - trans.v[2];
                offset[0] = v26 - trans.v[3];
                steerAnglesYaw[1] = (unsigned int)i > 1 ? 0.0f : steerYaw;
                DObjSetLocalTagInternal(obj, (const float *)&j, steerAnglesYaw, boneIndex);
                if ( steerAnglesPitch[0] != 0.0 )
                {
                    skel = (DObjAnimMat *)DObjGetChildBones(obj, boneIndex, v20, 4);
                    for ( m = 0; m < (int)skel; ++m )
                    {
                        if ( DObjSetRotTransIndex(obj, partBits, v20[m]) )
                            DObjSetLocalTagInternal(obj, vec3_origin, steerAnglesPitch, v20[m]);
                    }
                }
            }
        }
        steerAnglesPitch[0] = steerAnglesPitch[0] * pose->vehicle.extra_wheel_rot_scale;
        for ( n = 0; n < 4; ++n )
        {
            boneIndex = pose->vehicle.tag_extra_tank_wheels[n];
            if ( boneIndex < 0xFE && DObjSetRotTransIndex(obj, partBits, boneIndex) )
                DObjSetLocalTagInternal(obj, vec3_origin, steerAnglesPitch, boneIndex);
        }
    }
}
#endif
void CG_Vehicle_DoControllers(const cpose_t *pose, const DObj *obj, int *partBits)
{
    iassert(obj);

    float bodyAngles[3] = { 0 };
    float turretAngles[4] = { 0 };
    float barrelAngles[3] = { 0 };
    float steerYaw = 0.0f;
    float steerAnglesPitch[3] = { 0 };
    float steerAnglesYaw[3] = { 0 };
    float minigunAngles[3] = { 0 };
    float barrelOffset[5] = { 0 };
    float gunnerTurretAngles[12] = { 0 };
    float gunnerBarrelAngles[4][3] = { 0 };

    // Main vehicle angles
    steerAnglesYaw[2] = pose->vehicle.pitch * 0.0054931641f;
    bodyAngles[1] = pose->vehicle.roll * 0.0054931641f;
    bodyAngles[2] = pose->vehicle.barrelPitch * 0.0054931641f;
    turretAngles[0] = pose->vehicle.yaw * 0.0054931641f;
    steerYaw = pose->vehicle.steerPitch * 0.0054931641f;

    // Set main tags
    DObjSetLocalTag(obj, partBits, pose->vehicle.tag_body, vec3_origin, &steerAnglesYaw[2]);
    DObjSetLocalTag(obj, partBits, pose->vehicle.tag_turret, vec3_origin, &barrelAngles[2]);
    DObjSetLocalTag(obj, partBits, pose->vehicle.tag_barrel, vec3_origin, &bodyAngles[2]);

    // Barrel recoil
    if (pose->vehicle.barrelRecoil > 0.0f) {
        barrelOffset[2] = pose->vehicle.barrelRecoil;
        minigunAngles[2] = barrelOffset[2] * recoilVec[0];
        barrelOffset[0] = barrelOffset[2] * recoilVec[1];
        barrelOffset[1] = barrelOffset[2] * recoilVec[2];
        DObjSetLocalTag(obj, partBits, pose->vehicle.tag_barrel_recoil, &minigunAngles[2], vec3_origin);
    }

    // Minigun spin
    if (pose->vehicle.tag_minigun_spin != 254) {
        float spin = pose->vehicle.minigun_rotation * 0.0054931641f;
        minigunAngles[1] = spin;
        DObjSetLocalTag(obj, partBits, pose->vehicle.tag_minigun_spin, vec3_origin, &spin);
    }

    // Gunner wheels / turrets
    for (int i = 0; i < 4; ++i) {
        int boneIndex = pose->vehicle.tag_gunner_turret[i];
        gunnerTurretAngles[i * 3] = pose->vehicle.gunnerYaw[i] * 0.0054931641f;
        gunnerTurretAngles[i * 3 + 2] = pose->vehicle.gunnerPitch[i] * 0.0054931641f;

        if (DObjSetRotTransIndex(obj, partBits, boneIndex)) {
            DObjSetLocalTagInternal(obj, vec3_origin, &gunnerTurretAngles[i * 3], boneIndex);
        }
    }

    // Wheels
    for (int k = 0; k < 6; ++k) {
        int wheelBone = pose->vehicle.wheelBoneIndex[k];
        float wheelHeight = pose->vehicle.wheelHeight[k];
        float wheelRotation = pose->vehicle.nitrousWheelRotation[k];

        if (DObjSetRotTransIndex(obj, partBits, wheelBone)) {
            float wheelPos[3] = { 0, 0, wheelHeight };
            DObjSetLocalTagInternal(obj, wheelPos, &wheelRotation, wheelBone);
        }
    }

    // Extra tank wheels
    for (int m = 0; m < 4; ++m) {
        int wheelBone = pose->vehicle.tag_extra_tank_wheels[m];
        float angle = pose->vehicle.nitrousWheelRotation[m] * pose->vehicle.extra_wheel_rot_scale;
        float wheelAngles[3] = { 0, 0, angle };

        if (DObjSetRotTransIndex(obj, partBits, wheelBone)) {
            DObjSetLocalTagInternal(obj, vec3_origin, &wheelAngles[2], wheelBone);
        }
    }
}



void __cdecl CG_mg42_DoControllers(const cpose_t *pose, const DObj *obj, int *partBits)
{
    long double v3; // st7
    long double v4; // st7
    float firingOffset; // [esp+4h] [ebp-B8h]
    float ofsVec[3]; // [esp+48h] [ebp-74h] BYREF
    float axis[3][3]; // [esp+54h] [ebp-68h] BYREF
    float ofs[3]; // [esp+78h] [ebp-44h]
    float ofsDist; // [esp+84h] [ebp-38h]
    float ys; // [esp+88h] [ebp-34h]
    float yc; // [esp+8Ch] [ebp-30h]
    float turretYaw; // [esp+90h] [ebp-2Ch]
    float totOfs[3]; // [esp+94h] [ebp-28h] BYREF
    float angles[3]; // [esp+A0h] [ebp-1Ch] BYREF
    float pc; // [esp+ACh] [ebp-10h]
    float turretPitch; // [esp+B0h] [ebp-Ch]
    const float *viewAngles; // [esp+B4h] [ebp-8h]
    float ps; // [esp+B8h] [ebp-4h]

    memset(angles, 0, sizeof(angles));
    if ( pose->turret.playerUsing )
    {
        viewAngles = pose->turret.viewAngles;
        turretPitch = AngleNormalize180(*viewAngles - pose->angles[0]);
        turretYaw = AngleNormalize180(viewAngles[1] - pose->angles[1]);
    }
    else
    {
        turretPitch = pose->turret.angles.pitch;
        turretYaw = pose->turret.angles.yaw;
    }
    angles[2] = 0.0f;
    ofsDist = pose->player.waterHeight;
    memset(totOfs, 0, sizeof(totOfs));
    if ( pose->turret.tag_aim_pivot == 255 )
    {
        angles[1] = turretYaw;
        angles[0] = turretPitch;
        v3 = (float)(DEG2RAD(turretPitch));
        pc = cos(v3);
        ps = sin(v3);
        v4 = (float)(DEG2RAD(turretYaw));
        yc = cos(v4);
        ys = sin(v4);
        ofs[0] = (float)((float)((float)(yc - 1.0) * ofsDist) * ycFwdScale)
                     + (float)((float)((float)((float)(1.0 - pc) * ofsDist) * pcFwdScale) * (float)(yc - 1.0));
        ofs[1] = (float)((float)(ys * ofsDist) * ysScale)
                     + (float)((float)((float)((float)(pc - 1.0) * ofsDist) * pcScale) * ys);
        ofs[2] = 0.0f;
        totOfs[0] = (float)(totOfs[0] + (float)(pc * ofsDist)) + ofs[0];
        totOfs[1] = (float)(totOfs[1] + 0.0) + ofs[1];
        totOfs[2] = (float)(totOfs[2] + (float)((-(ps)) * ofsDist)) + 0.0;
        if ( pose->turret.firingOffset != 0.0 )
        {
            AnglesToAxis(angles, axis);
            firingOffset = pose->turret.firingOffset;
            totOfs[0] = totOfs[0] + (float)(firingOffset * axis[0][0]);
            totOfs[1] = totOfs[1] + (float)(firingOffset * axis[0][1]);
            totOfs[2] = totOfs[2] + (float)(firingOffset * axis[0][2]);
            (ofsVec[0]) = -(firingOffset * axis[0][0]);
            (ofsVec[1]) = -(firingOffset * axis[0][1]);
            (ofsVec[2]) = -(firingOffset * axis[0][2]);
            DObjSetLocalTag(obj, partBits, pose->turret.tag_weapon, ofsVec, vec3_origin);
        }
        DObjSetLocalTag(obj, partBits, pose->turret.tag_aim, totOfs, angles);
        DObjSetLocalTag(obj, partBits, pose->turret.tag_aim_animated, totOfs, angles);
    }
    else
    {
        angles[0] = turretPitch;
        DObjSetLocalTag(obj, partBits, pose->turret.tag_aim_pivot, vec3_origin, angles);
        angles[0] = 0.0f;
        angles[1] = turretYaw;
        DObjSetLocalTag(obj, partBits, pose->turret.tag_aim, totOfs, angles);
        DObjSetLocalTag(obj, partBits, pose->turret.tag_aim_animated, totOfs, angles);
    }
    angles[0] = pose->turret.barrelPitch;
    angles[1] = 0.0f;
    DObjSetControlTagAngles(obj, partBits, pose->turret.tag_flash, angles);
}

void __cdecl CG_DoBaseOriginController(const cpose_t *pose, const DObj *obj, int *setPartBits)
{
    float v3; // [esp+78h] [ebp-80h]
    float baseQuat[4]; // [esp+84h] [ebp-74h] BYREF
    float origin[3]; // [esp+94h] [ebp-64h] BYREF
    int partIndex; // [esp+A0h] [ebp-58h]
    DObjAnimMat animMat; // [esp+A4h] [ebp-54h] BYREF
    int rootBoneCount; // [esp+C4h] [ebp-34h]
    unsigned int maxHighIndex; // [esp+C8h] [ebp-30h]
    DObjAnimMat *mat; // [esp+CCh] [ebp-2Ch]
    unsigned int highIndex; // [esp+D0h] [ebp-28h]
    int partBits[9]; // [esp+D4h] [ebp-24h] BYREF

    rootBoneCount = DObjGetRootBoneCount(obj);
    if ( !rootBoneCount
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_pose_mp.cpp", 448, 0, "%s", "rootBoneCount") )
    {
        __debugbreak();
    }
    maxHighIndex = --rootBoneCount >> 5;
    for ( highIndex = 0; highIndex < maxHighIndex; ++highIndex )
    {
        if ( setPartBits[highIndex] != -1 )
            goto notSet;
    }
    if ( ((0xFFFFFFFF >> ((rootBoneCount & 0x1F) + 1)) | setPartBits[maxHighIndex]) == 0xFFFFFFFF )
        return;
notSet:
    mat = DObjGetRotTransArray(obj);
    if ( mat )
    {
        AnglesToQuat(pose->angles, baseQuat);
        memset(partBits, 0, 16);
        memset(&partBits[5], 0, 16);
        partBits[4] = 0x80000000;
        partIndex = 0;
        while ( partIndex <= rootBoneCount )
        {
            highIndex = partIndex >> 5;
            if ( (setPartBits[partIndex >> 5] & partBits[4]) == 0 )
            {
                if ( DObjSetRotTransIndex(obj, &partBits[4 - highIndex], partIndex) )
                {
                    mat->quat[0] = baseQuat[0];
                    mat->quat[1] = baseQuat[1];
                    mat->quat[2] = baseQuat[2];
                    mat->quat[3] = baseQuat[3];
                    origin[0] = pose->origin[0];
                    origin[1] = pose->origin[1];
                    origin[2] = pose->origin[2];
                }
                else
                {
                    animMat.quat[0] = baseQuat[0];
                    animMat.quat[1] = baseQuat[1];
                    animMat.quat[2] = baseQuat[2];
                    animMat.quat[3] = baseQuat[3];
                    DObjSetTrans(&animMat, pose->origin);
                    v3 = Vec4LengthSq(animMat.quat);
                    if ( v3 == 0.0 )
                    {
                        animMat.quat[3] = 1.0f;
                        animMat.transWeight = 2.0f;
                    }
                    else
                    {
                        animMat.transWeight = 2.0 / v3;
                    }
                    QuatMultiplyEquals(baseQuat, mat->quat);
                    MatrixTransformVectorQuatTrans(mat->trans, &animMat, origin);
                }
                DObjSetTrans(mat, origin);
            }
            ++partIndex;
            partBits[4] = (partBits[4] << 31) | ((unsigned int)partBits[4] >> 1);
            ++mat;
        }
    }
}

void __cdecl MatrixTransformVectorQuatTrans(const float *in, const DObjAnimMat *mat, float *out)
{
    float axis[3][3]; // [esp+48h] [ebp-24h] BYREF

    ConvertQuatToMat(mat, axis);
    *out = (float)((float)((float)(*in * axis[0][0]) + (float)(in[1] * axis[1][0])) + (float)(in[2] * axis[2][0]))
             + mat->trans[0];
    out[1] = (float)((float)((float)(*in * axis[0][1]) + (float)(in[1] * axis[1][1])) + (float)(in[2] * axis[2][1]))
                 + mat->trans[1];
    out[2] = (float)((float)((float)(*in * axis[0][2]) + (float)(in[1] * axis[1][2])) + (float)(in[2] * axis[2][2]))
                 + mat->trans[2];
}

void __cdecl QuatMultiplyEquals(const float *in, float *inout)
{
    float temp_4; // [esp+4h] [ebp-Ch]
    float temp_8; // [esp+8h] [ebp-8h]
    float temp_12; // [esp+Ch] [ebp-4h]

    temp_4 = (float)((float)((float)(inout[1] * in[3]) - (float)(inout[2] * *in)) + (float)(inout[3] * in[1]))
                 + (float)(*inout * in[2]);
    temp_8 = (float)((float)((float)(inout[2] * in[3]) + (float)(inout[1] * *in)) - (float)(*inout * in[1]))
                 + (float)(inout[3] * in[2]);
    temp_12 = (float)((float)((float)(inout[3] * in[3]) - (float)(*inout * *in)) - (float)(inout[1] * in[1]))
                    - (float)(inout[2] * in[2]);
    *inout = (float)((float)((float)(*inout * in[3]) + (float)(inout[3] * *in)) + (float)(inout[2] * in[1]))
                 - (float)(inout[1] * in[2]);
    inout[1] = temp_4;
    inout[2] = temp_8;
    inout[3] = temp_12;
}

DObjAnimMat *__cdecl CG_DObjCalcPose(const cpose_t *remote_pose, DObj *obj, int *partBits)
{
    DObjAnimMat *boneMatrix; // [esp+0h] [ebp-8h] BYREF
    const cpose_t *pose; // [esp+4h] [ebp-4h]

    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_pose_mp.cpp", 641, 0, "%s", "obj") )
        __debugbreak();
    if ( !remote_pose
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_pose_mp.cpp", 642, 0, "%s", "remote_pose") )
    {
        __debugbreak();
    }
    DObjLock(obj);
    if ( !CL_DObjCreateSkelForBones(obj, partBits, &boneMatrix) )
    {
        pose = remote_pose;
        DObjCompleteHierarchyBits(obj, partBits);
        CG_DoControllers(pose, obj, partBits);
        DObjCalcSkel(obj, partBits);
    }
    DObjUnlock(obj);
    return boneMatrix;
}

