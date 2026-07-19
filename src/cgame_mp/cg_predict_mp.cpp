#include "cg_predict_mp.h"
#include <qcommon/threads.h>
#include <demo/demo_playback.h>
#include <bgame/bg_misc.h>
#include <universal/com_math_anglevectors.h>
#include <cgame/cg_draw_reticles.h>
#include <aim_assist/aim_assist.h>
#include <client_mp/cl_cgame_mp.h>
#include <client_mp/cl_input_mp.h>
#include "cg_main_mp.h"
#include "cg_vehicles_mp.h"
#include "cg_ents_mp.h"
#include <bgame/bg_pmove.h>
#include <bgame/bg_slidemove.h>
#include <clientscript/cscr_stringlist.h>

int driverClampCounts[2] =
{ 9, 8 };

int gunnerClampCounts[2] =
{ 8, 9 };

float gunnerTurretClampT34[8][2] =
{
  { 25.0, 0.0 },
  { 50.0, 90.0 },
  { 45.0, 130.0 },
  { 26.0, 180.0 },
  { 22.0, 215.0 },
  { 24.0, 242.0 },
  { 22.0, 295.0 },
  { 25.0, 360.0 }
};

float gunnerTurretClampPanzer[9][2] =
{
  { 25.0, 0.0 },
  { 24.0, 27.0 },
  { 34.0, 65.0 },
  { 41.0, 90.0 },
  { 45.0, 150.0 },
  { 40.0, 220.0 },
  { 34.0, 270.0 },
  { 23.0, 330.0 },
  { 25.0, 360.0 }
};

float driverTurretClampT34[9][2] =
{
  { 27.0, 0.0 },
  { 22.0, 45.0 },
  { 30.0, 90.0 },
  { 4.0, 155.0 },
  { 5.0, 180.0 },
  { 4.0, 205.0 },
  { 30.0, 270.0 },
  { 22.0, 315.0 },
  { 27.0, 360.0 }
};

float driverTurretClampPanzer[8][2] =
{
  { 17.0, 0.0 },
  { 13.0, 25.0 },
  { 22.0, 90.0 },
  { 8.0, 165.0 },
  { 8.0, 205.0 },
  { 22.0, 270.0 },
  { 13.0, 335.0 },
  { 17.0, 360.0 }
};

float (*driverClampArrays[2])[2] = { driverTurretClampT34, driverTurretClampPanzer };

float (*gunnerClampArrays[2])[2] = { gunnerTurretClampT34, gunnerTurretClampPanzer };


pmove_t cg_pmove[1];

int cg_itemLocalClientNum;
int cg_itemEntityCount;
centity_s *cg_itemEntities[512];

int __cdecl CG_ItemListLocalClientNum()
{
    return cg_itemLocalClientNum;
}

void __cdecl CG_ClearItemList()
{
    cg_itemEntityCount = 0;
}

void __cdecl CG_BuildItemList(int localClientNum, const snapshot_s *nextSnap)
{
    centity_s *cent; // [esp+8h] [ebp-Ch]
    int entIndex; // [esp+Ch] [ebp-8h]

    if ( !nextSnap
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_predict_mp.cpp", 76, 0, "%s", "nextSnap") )
    {
        __debugbreak();
    }
    if ( !Sys_IsMainThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_predict_mp.cpp",
                    77,
                    0,
                    "%s",
                    "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    CG_ClearItemList();
    cg_itemLocalClientNum = localClientNum;
    for ( entIndex = 0; entIndex < nextSnap->numEntities; ++entIndex )
    {
        cent = CG_GetEntity(localClientNum, nextSnap->entities[entIndex].number);
        if ( cent->nextState.eType == 3 )
            cg_itemEntities[cg_itemEntityCount++] = cent;
    }
}

void __cdecl MovieCameraMove(int localClientNum, usercmd_s *cmd)
{
    float *MapCenter; // eax
    float v3; // [esp+4h] [ebp-C4h]
    float value; // [esp+8h] [ebp-C0h]
    float *v5; // [esp+Ch] [ebp-BCh]
    float *v6; // [esp+10h] [ebp-B8h]
    float *movieCameraVelocity; // [esp+28h] [ebp-A0h]
    float *v8; // [esp+2Ch] [ebp-9Ch]
    float *v9; // [esp+30h] [ebp-98h]
    float fmove; // [esp+3Ch] [ebp-8Ch]
    float wishdir[3]; // [esp+40h] [ebp-88h] BYREF
    float max; // [esp+4Ch] [ebp-7Ch]
    float control; // [esp+50h] [ebp-78h]
    float wishvel[3]; // [esp+54h] [ebp-74h]
    cg_s *cgameGlob; // [esp+60h] [ebp-68h]
    float oldpos[3]; // [esp+64h] [ebp-64h] BYREF
    float wishspeed; // [esp+70h] [ebp-58h]
    float right[3]; // [esp+74h] [ebp-54h] BYREF
    float addspeed; // [esp+80h] [ebp-48h]
    float currentspeed; // [esp+84h] [ebp-44h]
    float total; // [esp+88h] [ebp-40h]
    float forward[3]; // [esp+8Ch] [ebp-3Ch] BYREF
    float curFriction; // [esp+98h] [ebp-30h]
    float newspeed; // [esp+9Ch] [ebp-2Ch]
    float drop; // [esp+A0h] [ebp-28h]
    float speed; // [esp+A4h] [ebp-24h]
    float smove; // [esp+A8h] [ebp-20h]
    float up[3]; // [esp+ACh] [ebp-1Ch] BYREF
    float accelspeed; // [esp+B8h] [ebp-10h]
    int i; // [esp+BCh] [ebp-Ch]
    float scale; // [esp+C0h] [ebp-8h]
    float umove; // [esp+C4h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    oldpos[0] = cgameGlob->movieCameraOrigin[0];
    oldpos[1] = cgameGlob->movieCameraOrigin[1];
    oldpos[2] = cgameGlob->movieCameraOrigin[2];
    if ( !Demo_IsPlaying() )
        goto LABEL_6;
    if ( !Demo_ShouldUpdateMovieCamera(localClientNum) )
        return;
    if ( Demo_IsClipPlaying() || Demo_IsClipPreviewRunning() )
    {
        Demo_LerpClipCameraValues(localClientNum);
    }
    else
    {
LABEL_6:
        speed = Vec3Length(cgameGlob->movieCameraVelocity);
        if ( speed >= 1.0 )
        {
            drop = 0.0f;
            curFriction = friction->current.value * 1.5;
            if ( stopspeed->current.value <= speed )
                value = speed;
            else
                value = stopspeed->current.value;
            control = value;
            drop = (float)((float)(value * curFriction) * 0.016666999) + drop;
            newspeed = speed - drop;
            if ( (float)(speed - drop) < 0.0 )
                newspeed = 0.0f;
            newspeed = newspeed / speed;
            movieCameraVelocity = cgameGlob->movieCameraVelocity;
            v8 = cgameGlob->movieCameraVelocity;
            cgameGlob->movieCameraVelocity[0] = newspeed * cgameGlob->movieCameraVelocity[0];
            movieCameraVelocity[1] = newspeed * v8[1];
            movieCameraVelocity[2] = newspeed * v8[2];
        }
        else
        {
            v9 = cgameGlob->movieCameraVelocity;
            cgameGlob->movieCameraVelocity[0] = vec3_origin[0];
            v9[1] = 0.0;
            v9[2] = 0.0;
        }
        fmove = (float)cmd->forwardmove;
        smove = (float)cmd->rightmove;
        umove = 0.0f;

        //if (bitarray<51>::testBit(&cmd->button_bits, 7u))
        if (cmd->button_bits.testBit(7))
        {
            umove = umove + 127.0;
        }

        //if (bitarray<51>::testBit(&cmd->button_bits, 6u))
        if (cmd->button_bits.testBit(6))
        {
            umove = umove - 127.0;
        }

        max = fabs(fmove);
        if ( fabs(smove) > fabs(fmove) )
            max = fabs(smove);
        if ( fabs(umove) > max )
            max = fabs(umove);
        total = sqrtf((float)((float)(fmove * fmove) + (float)(smove * smove)) + (float)(umove * umove));
        scale = (float)((float)(190.0 * max) / (float)(127.0 * total)) * 3.0;
        for ( i = 0; i < 3; ++i )
            cgameGlob->movieCameraAngles[i] = AngleNormalize180((float)cmd->angles[i] * 0.0054931641);
        AngleVectors(cgameGlob->movieCameraAngles, forward, right, up);
        for ( i = 0; i < 3; ++i )
            wishvel[i] = (float)((float)(forward[i] * fmove) + (float)(right[i] * smove)) + (float)(up[i] * umove);
        wishdir[0] = wishvel[0];
        wishdir[1] = wishvel[1];
        wishdir[2] = wishvel[2];
        wishspeed = Vec3Normalize(wishdir);
        wishspeed = wishspeed * scale;
        currentspeed = (float)((float)(cgameGlob->movieCameraVelocity[0] * wishdir[0])
                                                 + (float)(cgameGlob->movieCameraVelocity[1] * wishdir[1]))
                                 + (float)(cgameGlob->movieCameraVelocity[2] * wishdir[2]);
        addspeed = wishspeed - currentspeed;
        if ( (float)(wishspeed - currentspeed) > 0.0 )
        {
            if ( stopspeed->current.value <= wishspeed )
                v3 = wishspeed;
            else
                v3 = stopspeed->current.value;
            control = v3;
            accelspeed = 0.15000299 * v3;
            if ( (float)(0.15000299 * v3) > addspeed )
                accelspeed = addspeed;
            v5 = cgameGlob->movieCameraVelocity;
            v6 = cgameGlob->movieCameraVelocity;
            cgameGlob->movieCameraVelocity[0] = (float)(accelspeed * wishdir[0]) + cgameGlob->movieCameraVelocity[0];
            v5[1] = (float)(accelspeed * wishdir[1]) + v6[1];
            v5[2] = (float)(accelspeed * wishdir[2]) + v6[2];
        }
        MovieCameraViewTrace(localClientNum, 0x800811);
        MapCenter = (float *)CL_GetMapCenter();
        BG_ClipCameraToHeliPatch(
            cgameGlob->movieCameraOrigin,
            oldpos,
            cgameGlob->movieCameraVelocity,
            (float (*)[3])MapCenter);
    }
}

void __cdecl FollowCameraMove(cg_s *cgameGlob, centity_s *cent, usercmd_s *cmd)
{
    float right[3]; // [esp+28h] [ebp-28h] BYREF
    float forward[3]; // [esp+34h] [ebp-1Ch] BYREF
    float up[3]; // [esp+40h] [ebp-10h] BYREF
    int i; // [esp+4Ch] [ebp-4h]

    cgameGlob->movieCameraVelocity[0] = vec3_origin[0];
    cgameGlob->movieCameraVelocity[1] = 0.0;
    cgameGlob->movieCameraVelocity[2] = 0.0;
    for ( i = 0; i < 3; ++i )
        cgameGlob->movieCameraAngles[i] = AngleNormalize180((float)cmd->angles[i] * 0.0054931641);
    cgameGlob->movieCameraOrigin[0] = cent->pose.origin[0];
    cgameGlob->movieCameraOrigin[1] = cent->pose.origin[1];
    cgameGlob->movieCameraOrigin[2] = cent->pose.origin[2];
    cgameGlob->movieCameraOrigin[2] = cgameGlob->movieCameraOrigin[2] + 35.0;
    AngleVectors(cgameGlob->movieCameraAngles, forward, right, up);
    Vec3Normalize(forward);
    cgameGlob->movieCameraOrigin[0] = (float)(-70.0 * forward[0]) + cgameGlob->movieCameraOrigin[0];
    cgameGlob->movieCameraOrigin[1] = (float)(-70.0 * forward[1]) + cgameGlob->movieCameraOrigin[1];
    cgameGlob->movieCameraOrigin[2] = (float)(-70.0 * forward[2]) + cgameGlob->movieCameraOrigin[2];
}

void __cdecl CG_ClearSavedPlayerState(int localClientNum)
{
    cg_s *cgameGlob; // [esp+8h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_predict_mp.cpp", 618, 1, "%s", "cgameGlob") )
    {
        __debugbreak();
    }
    cgameGlob->oldCommandTime = 0;
    cgameGlob->oldOrigin[0] = 0.0f;
    cgameGlob->oldOrigin[1] = 0.0f;
    cgameGlob->oldOrigin[2] = 0.0f;
    cgameGlob->oldViewangles[0] = 0.0f;
    cgameGlob->oldViewangles[1] = 0.0f;
    cgameGlob->oldViewangles[2] = 0.0f;
    CG_ClearTransPlayerState(&cgameGlob->oldTransPlayerState);
}

void __cdecl CG_SavePlayerState(int localClientNum)
{
    cg_s *LocalClientGlobals; // eax

    LocalClientGlobals = CG_GetLocalClientGlobals(localClientNum);
    LocalClientGlobals->oldCommandTime = LocalClientGlobals->predictedPlayerState.commandTime;
    LocalClientGlobals->oldOrigin[0] = LocalClientGlobals->predictedPlayerState.origin[0];
    LocalClientGlobals->oldOrigin[1] = LocalClientGlobals->predictedPlayerState.origin[1];
    LocalClientGlobals->oldOrigin[2] = LocalClientGlobals->predictedPlayerState.origin[2];
    LocalClientGlobals->oldViewangles[0] = LocalClientGlobals->predictedPlayerState.viewangles[0];
    LocalClientGlobals->oldViewangles[1] = LocalClientGlobals->predictedPlayerState.viewangles[1];
    LocalClientGlobals->oldViewangles[2] = LocalClientGlobals->predictedPlayerState.viewangles[2];
    CG_ExtractTransPlayerState(&LocalClientGlobals->predictedPlayerState, &LocalClientGlobals->oldTransPlayerState);
}

void __cdecl CG_PredictPlayerState(int localClientNum)
{
    centity_s *Entity; // eax
    double v2; // st7
    float *v3; // [esp+28h] [ebp-B8h]
    float *trBase; // [esp+2Ch] [ebp-B4h]
    float v5; // [esp+34h] [ebp-ACh]
    float v6; // [esp+38h] [ebp-A8h]
    float absAxis[3][3]; // [esp+54h] [ebp-8Ch] BYREF
    centity_s *vehicle; // [esp+78h] [ebp-68h]
    float original_angles[3]; // [esp+7Ch] [ebp-64h]
    float aVelocity[3]; // [esp+88h] [ebp-58h] BYREF
    float origin[3]; // [esp+94h] [ebp-4Ch] BYREF
    float tVelocity[3]; // [esp+A0h] [ebp-40h] BYREF
    cg_s *cgameGlob; // [esp+ACh] [ebp-34h]
    float angle; // [esp+B0h] [ebp-30h]
    centity_s *playerEnt; // [esp+B4h] [ebp-2Ch]
    float size; // [esp+B8h] [ebp-28h]
    float angles[3]; // [esp+BCh] [ebp-24h] BYREF
    float fZoom; // [esp+C8h] [ebp-18h] BYREF
    const WeaponDef *weapDef; // [esp+CCh] [ebp-14h]
    float fCos; // [esp+D0h] [ebp-10h]
    playerState_s *ps; // [esp+D4h] [ebp-Ch]
    float fSin; // [esp+D8h] [ebp-8h]
    bool inVehicle; // [esp+DFh] [ebp-1h]
    int savedregs; // [esp+E0h] [ebp+0h] BYREF

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    ps = &cgameGlob->predictedPlayerState;
    cgameGlob->lastFrame.aimSpreadScale = cgameGlob->predictedPlayerState.aimSpreadScale;
    CG_PredictPlayerState_Internal(localClientNum);
    Entity = CG_GetEntity(localClientNum, ps->clientNum);
    playerEnt = Entity;
    Vec3Copy(ps->origin, Entity->pose.origin);
    BG_EvaluateTrajectory(&playerEnt->currentState.apos, cgameGlob->time, playerEnt->pose.angles);
    weapDef = BG_GetWeaponDef(ps->weapon);
    if ( (cgameGlob->nextSnap->ps.otherFlags & 4) != 0 && CG_GetWeapReticleZoom(cgameGlob, &fZoom) )
    {
        if ( weapDef->adsViewErrorMax != 0.0 && !cgameGlob->adsViewErrorDone )
        {
            cgameGlob->adsViewErrorDone = 1;
            size = flrand(weapDef->adsViewErrorMin, weapDef->adsViewErrorMax);
            v2 = Com_Random();
            angle = (v2 + v2) * 3.1415927;
            fCos = cos(angle);
            fSin = sin(angle);
            cgameGlob->offsetAngles[0] = AngleNormalize360((float)(fSin * size) + cgameGlob->offsetAngles[0]);
            cgameGlob->offsetAngles[1] = AngleNormalize360((float)(fCos * size) + cgameGlob->offsetAngles[1]);
        }
    }
    else
    {
        cgameGlob->adsViewErrorDone = 0;
    }
    cgameGlob->predictedPlayerEntity.nextState.number = ps->clientNum;
    BG_PlayerStateToEntityState(ps, &cgameGlob->predictedPlayerEntity.nextState, 0, 0);
    *((unsigned int *)&cgameGlob->predictedPlayerEntity + 201) |= 2u;
    memcpy(
        &cgameGlob->predictedPlayerEntity.currentState,
        &cgameGlob->predictedPlayerEntity.nextState.lerp,
        sizeof(cgameGlob->predictedPlayerEntity.currentState));
    CG_CopyFlagsFromSnapshotEntity(localClientNum);
    if ( (ps->eFlags2 & 0x10000000) != 0 )
    {
        v5 = ps->linkAngles[1];
        v6 = ps->linkAngles[2];
        original_angles[0] = ps->linkAngles[0];
        original_angles[1] = v5;
        original_angles[2] = v6;
        trBase = cgameGlob->predictedPlayerEntity.nextState.lerp.apos.trBase;
        cgameGlob->predictedPlayerEntity.nextState.lerp.apos.trBase[0] = original_angles[0];
        trBase[1] = original_angles[1];
        trBase[2] = original_angles[2];
        v3 = cgameGlob->predictedPlayerEntity.currentState.apos.trBase;
        cgameGlob->predictedPlayerEntity.currentState.apos.trBase[0] = original_angles[0];
        v3[1] = original_angles[1];
        v3[2] = original_angles[2];
    }
    AimAssist_BackupPlayerState(localClientNum, ps);
    CL_SetUserCmdOrigin(localClientNum, ps->origin, ps->velocity, ps->viewangles, ps->bobCycle, ps->movementDir);
    inVehicle = 0;
    memset(origin, 0, sizeof(origin));
    memset(angles, 0, sizeof(angles));
    memset(tVelocity, 0, sizeof(tVelocity));
    memset(aVelocity, 0, sizeof(aVelocity));
    if ( !com_sv_running->current.enabled && (ps->eFlags & 0x4000) != 0 )
    {
        vehicle = CG_GetEntity(localClientNum, ps->viewlocked_entNum);
        if ( vehicle->nitrousVeh )
        {
            if ( vehicle->nitrousVeh->m_phys_user_data )
            {
                Sys_EnterCriticalSection(CRITSECT_PHYSICS);
                Phys_ObjGetPosition((int)vehicle->nitrousVeh->m_phys_user_data, origin, absAxis);
                Phys_ObjGetVelocities((int)vehicle->nitrousVeh->m_phys_user_data, tVelocity, aVelocity);
                Phys_ObjGetCenterOfMass((int)vehicle->nitrousVeh->m_phys_user_data, origin);
                Sys_LeaveCriticalSection(CRITSECT_PHYSICS);
                AxisToAngles(absAxis, angles);
                inVehicle = 1;
            }
        }
    }
    CL_SetUserCmdVehicle(localClientNum, inVehicle, origin, angles, tVelocity, aVelocity);
    CL_SendCmd(localClientNum);
}

int opti = 1;
void __cdecl CG_PredictPlayerState_Internal(int localClientNum)
{
    playerState_s *v1; // ecx
    float v2; // [esp+8h] [ebp-164h]
    float value; // [esp+Ch] [ebp-160h]
    float *v5; // [esp+24h] [ebp-148h]
    float *v6; // [esp+28h] [ebp-144h]
    float *v7; // [esp+2Ch] [ebp-140h]
    float *delta_angles; // [esp+34h] [ebp-138h]
    float *v9; // [esp+38h] [ebp-134h]
    float *v10; // [esp+3Ch] [ebp-130h]
    float *predictedError; // [esp+40h] [ebp-12Ch]
    float *v12; // [esp+44h] [ebp-128h]
    float *v13; // [esp+48h] [ebp-124h]
    float *viewangles; // [esp+58h] [ebp-114h]
    float *oldViewangles; // [esp+5Ch] [ebp-110h]
    int k; // [esp+6Ch] [ebp-100h]
    int i; // [esp+74h] [ebp-F8h]
    int timeSinceStart; // [esp+78h] [ebp-F4h]
    int smoothingDuration; // [esp+7Ch] [ebp-F0h]
    float diff; // [esp+84h] [ebp-E8h]
    float stepRemaining; // [esp+88h] [ebp-E4h]
    int j; // [esp+90h] [ebp-DCh]
    float delta_diff[3]; // [esp+94h] [ebp-D8h] BYREF
    int t; // [esp+A0h] [ebp-CCh]
    float f; // [esp+A4h] [ebp-C8h]
    float adjusted[3]; // [esp+A8h] [ebp-C4h] BYREF
    float delta[3]; // [esp+B4h] [ebp-B8h] BYREF
    float len; // [esp+C0h] [ebp-ACh]
    float newAngles[3]; // [esp+C4h] [ebp-A8h] BYREF
    cg_s *cgameGlob; // [esp+D0h] [ebp-9Ch]
    int bPredictionRun; // [esp+D4h] [ebp-98h]
    usercmd_s latestCmd; // [esp+D8h] [ebp-94h] BYREF
    float oldDeltaAngles[3]; // [esp+10Ch] [ebp-60h]
    float deltaAngles[3]; // [esp+118h] [ebp-54h] BYREF
    int cmdNum; // [esp+124h] [ebp-48h]
    usercmd_s oldestCmd; // [esp+128h] [ebp-44h] BYREF
    const playerState_s *ps; // [esp+160h] [ebp-Ch]
    int current; // [esp+164h] [ebp-8h]
    centity_s *centTarget; // [esp+168h] [ebp-4h]

    for ( i = 0; i < 2; ++i )
        oldestCmd.button_bits.array[i] = 0;
    for ( k = 0; k < 2; ++k )
        latestCmd.button_bits.array[k] = 0;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob->nextSnap
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_predict_mp.cpp",
                    685,
                    0,
                    "%s",
                    "cgameGlob->nextSnap") )
    {
        __debugbreak();
    }
    ps = &cgameGlob->nextSnap->ps;
    current = CL_GetCurrentCmdNumber(localClientNum);
    if ( cgameGlob->cameraMode == 1 )
    {
        CG_InterpolatePlayerState(localClientNum, 0);
        CL_GetUserCmd(localClientNum, current, &latestCmd);
        MovieCameraMove(localClientNum, &latestCmd);
    }
    else if ( cgameGlob->cameraMode == 2 )
    {
        CG_InterpolatePlayerState(localClientNum, 0);
        centTarget = CG_GetEntity(localClientNum, cgameGlob->followCameraClient);
        if ( ((*((unsigned int *)centTarget + 201) >> 1) & 1) != 0 )
        {
            CL_GetUserCmd(localClientNum, current, &latestCmd);
            FollowCameraMove(cgameGlob, centTarget, &latestCmd);
        }
    }
    else if ( cgameGlob->demoType || (ps->otherFlags & 2) != 0 )
    {
        CG_InterpolatePlayerState(localClientNum, 0);
    }
    else if ( cg_nopredict->current.enabled || cg_synchronousClients->current.enabled )
    {
        CG_InterpolatePlayerState(localClientNum, 1);
    }
    else
    {
        cg_pmove[localClientNum].localClientNum = localClientNum;
        cg_pmove[localClientNum].ps = &cgameGlob->predictedPlayerState;
        cg_pmove[localClientNum].handler = 0;
        if (cg_pmove[localClientNum].ps->pm_type < 9)
            cg_pmove[localClientNum].tracemask = 0x2818011;
        else
            cg_pmove[localClientNum].tracemask = 0x810011;
        if (ps->pm_type == 4)
        {
            cg_pmove[localClientNum].tracemask &= 0xFDFE7FFF;
            cg_pmove[localClientNum].tracemask |= 0x800u;
        }
        cg_pmove[localClientNum].viewChange = 0.0f;
        cg_pmove[localClientNum].viewChangeTime = cgameGlob->stepViewStart;
        cmdNum = current - 127;
        if (CL_GetUserCmd(localClientNum, current - 127, &oldestCmd))
        {
            CL_GetUserCmd(localClientNum, current, &latestCmd);
            if ( opti )
            {
                if ( cgameGlob->nextSnap->serverTime != cgameGlob->lastPlayerStateOverride
                    || cgameGlob->predictedPlayerState.groundEntityNum != 1023
                    && cgameGlob->predictedPlayerState.groundEntityNum != 1022 )
                {
                    memcpy(
                        (unsigned __int8 *)&cgameGlob->predictedPlayerState,
                        (unsigned __int8 *)ps,
                        sizeof(cgameGlob->predictedPlayerState));
                    cgameGlob->lastPlayerStateOverride = cgameGlob->nextSnap->serverTime;
                }
            }
            else
            {
                memcpy(
                    (unsigned __int8 *)&cgameGlob->predictedPlayerState,
                    (unsigned __int8 *)ps,
                    sizeof(cgameGlob->predictedPlayerState));
            }
            if ( (ps->eFlags & 0x4000) != 0 && ps->vehiclePos >= 1 && ps->vehiclePos <= 4 && ps->leanf > 0.0 )
            {
                viewangles = cgameGlob->predictedPlayerState.viewangles;
                oldViewangles = cgameGlob->oldViewangles;
                cgameGlob->predictedPlayerState.viewangles[0] = cgameGlob->oldViewangles[0];
                viewangles[1] = oldViewangles[1];
                viewangles[2] = oldViewangles[2];
            }
            if ( CG_VehEntityUsingVehicle(localClientNum, ps->clientNum)
                && !CG_VehEntityUsingRemoteControlVehicle(localClientNum, ps->clientNum) )
            {
                CG_VehSeatTransformForPlayer(localClientNum, ps->clientNum, cgameGlob->predictedPlayerState.origin, newAngles);
            }
            if ( (cgameGlob->predictedPlayerState.pm_type == 1 || cgameGlob->predictedPlayerState.pm_type == 10)
                && (cgameGlob->predictedPlayerState.eFlags & 0x4000) == 0 )
            {
                CG_InterpolatePlayerState(localClientNum, 0);
            }
            CG_AdjustPositionForMover(
                localClientNum,
                cgameGlob->predictedPlayerState.origin,
                cgameGlob->predictedPlayerState.groundEntityNum,
                cgameGlob->nextSnap->serverTime,
                cgameGlob->time,
                cgameGlob->predictedPlayerState.origin,
                deltaAngles);
            cgameGlob->predictedPlayerState.delta_angles[1] = cgameGlob->predictedPlayerState.delta_angles[1] + deltaAngles[1];
            CG_InterpolatePlayerStateViewAngles(localClientNum, &cgameGlob->predictedPlayerState, &latestCmd);
            bPredictionRun = 0;
            if ( ps->commandTime >= latestCmd.serverTime )
            {
                if ( !cgameGlob->playerTeleported )
                {
                    v5 = cgameGlob->predictedPlayerState.viewangles;
                    v6 = cgameGlob->oldViewangles;
                    cgameGlob->predictedPlayerState.viewangles[0] = cgameGlob->oldViewangles[0];
                    v5[1] = v6[1];
                    v5[2] = v6[2];
                }
            }
            else
            {
                for ( cmdNum = current - 127; cmdNum <= current; ++cmdNum )
                {
                    CL_GetUserCmd(localClientNum, cmdNum, &cg_pmove[localClientNum].cmd);
                    if (cg_pmove[localClientNum].cmd.serverTime > cgameGlob->predictedPlayerState.commandTime
                        && cg_pmove[localClientNum].cmd.serverTime <= latestCmd.serverTime
                        && CL_GetUserCmd(localClientNum, cmdNum - 1, &cg_pmove[localClientNum].oldcmd))
                    {
                        if (cgameGlob->predictedPlayerState.commandTime == cgameGlob->oldCommandTime)
                        {
                            CG_AdjustPositionForMover(
                                localClientNum,
                                cgameGlob->predictedPlayerState.origin,
                                cgameGlob->predictedPlayerState.groundEntityNum,
                                cgameGlob->time,
                                cgameGlob->oldTime,
                                adjusted,
                                deltaAngles);
                            if (cg_showmiss->current.integer)
                            {
                                if (cgameGlob->oldOrigin[0] != adjusted[0]
                                    || cgameGlob->oldOrigin[1] != adjusted[1]
                                    || cgameGlob->oldOrigin[2] != adjusted[2])
                                {
                                    Com_PrintError(17, "prediction error\n");
                                }
                            }
                            delta[0] = cgameGlob->oldOrigin[0] - adjusted[0];
                            delta[1] = cgameGlob->oldOrigin[1] - adjusted[1];
                            delta[2] = cgameGlob->oldOrigin[2] - adjusted[2];
                            len = Vec3Length(delta);
                            if (len > 0.1)
                            {
                                if (cg_showmiss->current.integer)
                                    Com_Printf(17, "Prediction miss: %f\n", len);
                                if (cg_errorDecay->current.value == 0.0)
                                {
                                    predictedError = cgameGlob->predictedError;
                                    cgameGlob->predictedError[0] = 0.0f;
                                    predictedError[1] = 0.0f;
                                    predictedError[2] = 0.0f;
                                }
                                else
                                {
                                    t = cgameGlob->time - cgameGlob->predictedErrorTime;
                                    f = (float)(cg_errorDecay->current.value - (float)t) / cg_errorDecay->current.value;
                                    if (f < 0.0)
                                        f = 0.0f;
                                    if (f > 0.0 && cg_showmiss->current.integer)
                                        Com_Printf(17, "Double prediction decay: %f\n", f);
                                    v12 = cgameGlob->predictedError;
                                    v13 = cgameGlob->predictedError;
                                    cgameGlob->predictedError[0] = f * cgameGlob->predictedError[0];
                                    v12[1] = f * v13[1];
                                    v12[2] = f * v13[2];
                                }
                                v9 = cgameGlob->predictedError;
                                v10 = cgameGlob->predictedError;
                                cgameGlob->predictedError[0] = delta[0] + cgameGlob->predictedError[0];
                                v9[1] = delta[1] + v10[1];
                                v9[2] = delta[2] + v10[2];
                                cgameGlob->predictedErrorTime = cgameGlob->oldTime;
                            }
                        }
                        delta_angles = cg_pmove[localClientNum].ps->delta_angles;
                        oldDeltaAngles[0] = *delta_angles;
                        oldDeltaAngles[1] = delta_angles[1];
                        oldDeltaAngles[2] = delta_angles[2];
                        Pmove(&cg_pmove[localClientNum]);
                        if (cg_pmove[localClientNum].ps->vehiclePos >= 1 && cg_pmove[localClientNum].ps->vehiclePos <= 4)
                        {
                            v1 = cg_pmove[localClientNum].ps;
                            delta_diff[0] = v1->delta_angles[0] - oldDeltaAngles[0];
                            delta_diff[1] = v1->delta_angles[1] - oldDeltaAngles[1];
                            delta_diff[2] = v1->delta_angles[2] - oldDeltaAngles[2];
                            CL_AdjustViewAngles(localClientNum, delta_diff);
                            for (j = cmdNum; j <= current; ++j)
                                CL_AdjustUserCmdAngles(localClientNum, j, delta_diff);
                            v7 = cg_pmove[localClientNum].ps->delta_angles;
                            *v7 = oldDeltaAngles[0];
                            v7[1] = oldDeltaAngles[1];
                            v7[2] = oldDeltaAngles[2];
                        }
                        CG_TouchItemPrediction(localClientNum);
                        bPredictionRun = 1;
                    }
                }
                render_gjkcc_collision(&cg_pmove[localClientNum]);
            }
            if (cg_showmiss->current.integer > 1)
                Com_Printf(17, "[%i : %i] ", cg_pmove[localClientNum].cmd.serverTime, cgameGlob->time);
            if ( !bPredictionRun && cg_showmiss->current.integer )
                Com_Printf(17, "no prediction run\n");
            if ( (cgameGlob->predictedPlayerState.pm_flags & 0x400) != 0 )
                CL_SetStance(localClientNum, CL_STANCE_STAND);
            if ( !cgameGlob->predictedPlayerState.cursorHint )
            {
                cgameGlob->predictedPlayerState.cursorHintString = -1;
                cgameGlob->predictedPlayerState.cursorHintEntIndex = 1023;
            }
            if (cg_pmove[localClientNum].viewChange == 0.0
                || cg_pmove[localClientNum].viewChangeTime == cgameGlob->stepViewStart
                || cgameGlob->playerTeleported
                || ps->pm_type && ps->pm_type != 2 && ps->pm_type != 3)
            {
                if ( (float)(cgameGlob->time - cgameGlob->stepViewStart) > (float)(cg_viewZSmoothingTime->current.value * 1000.0) )
                    cgameGlob->stepViewChange = 0.0f;
            }
            else
            {
                diff = cg_pmove[localClientNum].viewChange;
                if ( fabs(diff) < cg_viewZSmoothingMin->current.value )
                {
                    if ( (float)(cgameGlob->time - cgameGlob->stepViewStart) > (float)(cg_viewZSmoothingTime->current.value
                                                                                                                                                     * 1000.0) )
                        cgameGlob->stepViewChange = 0.0f;
                }
                else
                {
                    stepRemaining = 0.0f;
                    if ( (float)(cg_viewZSmoothingTime->current.value * 1000.0) > (float)(cgameGlob->time
                                                                                                                                                            - cgameGlob->stepViewStart) )
                    {
                        timeSinceStart = cg_pmove[localClientNum].viewChangeTime - cgameGlob->stepViewStart;
                        smoothingDuration = (int)(float)(cg_viewZSmoothingTime->current.value * 1000.0);
                        if ( timeSinceStart >= 0 && timeSinceStart < smoothingDuration )
                            stepRemaining = (float)(1.0 - (float)((float)((float)timeSinceStart * 1.0) / (float)smoothingDuration))
                                                        * cgameGlob->stepViewChange;
                    }
                    if ( (float)(stepRemaining + diff) <= 0.0 )
                    {
                        if ( (float)(-(cg_viewZSmoothingMax->current.integer) - (float)(stepRemaining + diff)) < 0.0 )
                            v2 = stepRemaining + diff;
                        else
                            v2 = -cg_viewZSmoothingMax->current.integer;
                        cgameGlob->stepViewChange = v2;
                    }
                    else
                    {
                        if ( (float)((float)(stepRemaining + diff) - cg_viewZSmoothingMax->current.value) < 0.0 )
                            value = stepRemaining + diff;
                        else
                            value = cg_viewZSmoothingMax->current.value;
                        cgameGlob->stepViewChange = value;
                    }
                    cgameGlob->stepViewStart = cg_pmove[localClientNum].viewChangeTime;
                }
            }
        }
        else if ( cg_showmiss->current.integer )
        {
            Com_Printf(17, "exceeded PACKET_BACKUP on commands\n");
        }
    }
}

void __cdecl CG_TouchItemPrediction(int localClientNum)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-Ch]
    centity_s *cent; // [esp+4h] [ebp-8h]
    int entIndex; // [esp+8h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->predictedPlayerState.pm_type <= 1u )
    {
        for ( entIndex = 0; entIndex < cg_itemEntityCount; ++entIndex )
        {
            cent = cg_itemEntities[entIndex];
            if ( cent->nextState.eType != 3
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_predict_mp.cpp",
                            151,
                            0,
                            "%s",
                            "cent->nextState.eType == ET_ITEM") )
            {
                __debugbreak();
            }
            CG_TouchItem(cgameGlob, cent);
        }
    }
}

void __cdecl CG_TouchItem(cg_s *cgameGlob, centity_s *cent)
{
    if ( !cgameGlob
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_predict_mp.cpp", 102, 0, "%s", "cgameGlob") )
    {
        __debugbreak();
    }
    if ( !cent
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_predict_mp.cpp", 103, 0, "%s", "cent") )
    {
        __debugbreak();
    }
    if ( cg_predictItems->current.enabled
        && BG_PlayerTouchesItem(&cgameGlob->predictedPlayerState, &cent->nextState, cgameGlob->time)
        && cent->miscTime != cgameGlob->time
        && BG_PlayerHasWeapon(&cgameGlob->predictedPlayerState, cent->nextState.un3.item % 2048)
        && BG_CanItemBeGrabbed(&cent->nextState, &cgameGlob->predictedPlayerState, 1) )
    {
        if ( BG_PlayerHasRoomForEntAllAmmoTypes(&cent->nextState, &cgameGlob->predictedPlayerState) )
        {
            cent->nextState.lerp.eFlags |= 0x20u;
            cent->miscTime = cgameGlob->time;
            BG_AddPredictableEventToPlayerstate(0xCu, cent->nextState.un3.item % 2048, &cgameGlob->predictedPlayerState);
        }
    }
}

void __cdecl CG_InterpolatePlayerStateViewAngles(int localClientNum, playerState_s *out, usercmd_s *cmd)
{
    const char *v3; // eax
    float v4; // [esp+10h] [ebp-F0h]
    float v5; // [esp+20h] [ebp-E0h]
    float frameInterpolation; // [esp+28h] [ebp-D8h]
    float resultMat[3][3]; // [esp+30h] [ebp-D0h] BYREF
    float centerAngles[3]; // [esp+54h] [ebp-ACh] BYREF
    float viewYawAngles[3]; // [esp+60h] [ebp-A0h] BYREF
    float viewMat[3][3]; // [esp+6Ch] [ebp-94h] BYREF
    float vehMat[3][3]; // [esp+90h] [ebp-70h] BYREF
    float v12; // [esp+B4h] [ebp-4Ch]
    float fc; // [esp+B8h] [ebp-48h]
    int clampCount; // [esp+BCh] [ebp-44h]
    float yawOffset; // [esp+C0h] [ebp-40h]
    float pitchClamp; // [esp+C4h] [ebp-3Ch]
    float (*gunnerClamps)[2]; // [esp+C8h] [ebp-38h]
    centity_s *vehCent; // [esp+CCh] [ebp-34h]
    float vehAngles[3]; // [esp+D0h] [ebp-30h] BYREF
    float yawFrac; // [esp+DCh] [ebp-24h]
    int vehicleType; // [esp+E0h] [ebp-20h]
    unsigned __int16 vehType; // [esp+E4h] [ebp-1Ch]
    const char *vehicleTypeStr[2]; // [esp+E8h] [ebp-18h]
    cg_s *cgameGlob; // [esp+F0h] [ebp-10h]
    const snapshot_s *prevSnap; // [esp+F4h] [ebp-Ch]
    const snapshot_s *nextSnap; // [esp+F8h] [ebp-8h]
    int i; // [esp+FCh] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    prevSnap = cgameGlob->snap;
    nextSnap = cgameGlob->nextSnap;
    if ( !prevSnap
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_predict_mp.cpp", 190, 0, "%s", "prevSnap") )
    {
        __debugbreak();
    }
    if ( !nextSnap
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_predict_mp.cpp", 191, 0, "%s", "nextSnap") )
    {
        __debugbreak();
    }
    if ( CG_ShouldInterpolatePlayerStateViewClamp(localClientNum, prevSnap) )
    {
        for ( i = 0; i < 2; ++i )
        {
            v5 = prevSnap->ps.viewAngleClampBase[i];
            frameInterpolation = cgameGlob->frameInterpolation;
            out->viewAngleClampBase[i] = AngleNormalize180(nextSnap->ps.viewAngleClampBase[i] - v5) * frameInterpolation + v5;
            out->viewAngleClampRange[i] = (float)((float)(nextSnap->ps.viewAngleClampRange[i]
                                                                                                    - prevSnap->ps.viewAngleClampRange[i])
                                                                                    * cgameGlob->frameInterpolation)
                                                                    + prevSnap->ps.viewAngleClampRange[i];
        }
    }
    if ( cmd )
    {
        if ( (prevSnap->ps.eFlags & 0x4000) != 0
            && prevSnap->ps.vehiclePos == nextSnap->ps.vehiclePos
            && prevSnap->ps.vehiclePos >= 1
            && prevSnap->ps.vehiclePos <= 4 )
        {
            vehCent = CG_GetEntity(localClientNum, out->viewlocked_entNum);
            vehicleTypeStr[0] = "t34";
            vehicleTypeStr[1] = "panzer";
            vehType = CG_GetVehicleTypeString(cgameGlob->clientNum, out->viewlocked_entNum);
            if ( vehType )
            {
                for ( vehicleType = 0; vehicleType < 2; ++vehicleType )
                {
                    v3 = SL_ConvertToString(vehType, SCRIPTINSTANCE_SERVER);
                    if ( !I_stricmp(vehicleTypeStr[vehicleType], v3) )
                        break;
                }
                if ( vehicleType < 2 )
                {
                    pitchClamp = 360.0f;
                    vehAngles[0] = vehCent->pose.angles[0];
                    vehAngles[1] = vehCent->pose.angles[1];
                    vehAngles[2] = vehCent->pose.angles[2];
                    if ( prevSnap->ps.vehiclePos )
                    {
                        gunnerClamps = gunnerClampArrays[vehicleType];
                        vehAngles[1] = (float)((float)vehCent->pose.vehicle.yaw * 0.0054931641) + vehAngles[1];
                    }
                    else
                    {
                        gunnerClamps = driverClampArrays[vehicleType];
                    }
                    yawOffset = AngleNormalize360((float)((float)((float)cmd->angles[1] * 0.0054931641) + out->delta_angles[1]) - vehAngles[1]);
                    if ( prevSnap->ps.vehiclePos )
                        clampCount = gunnerClampCounts[vehicleType];
                    else
                        clampCount = driverClampCounts[vehicleType];
                    for ( i = 0; i < clampCount && gunnerClamps[i][1] <= yawOffset; ++i )
                        ;
                    v4 = (float)((float)((float)(yawOffset - gunnerClamps[i - 1][1])
                                                         / (float)(gunnerClamps[i][1] - gunnerClamps[i - 1][1]))
                                         * 3.1415927)
                         - 1.5707964;
                    fc = cos(v4);
                    v12 = sin(v4);
                    yawFrac = (float)(v12 + 1.0) / 2.0;
                    pitchClamp = (float)((float)(gunnerClamps[i][0] - gunnerClamps[i - 1][0]) * yawFrac) + gunnerClamps[i - 1][0];
                    if ( prevSnap->ps.vehiclePos )
                    {
                        out->viewAngleClampBase[0] = (float)(pitchClamp - out->viewAngleClampRange[0]) / 2.0;
                        out->viewAngleClampRange[0] = (float)(pitchClamp + out->viewAngleClampRange[0]) / 2.0;
                        out->viewAngleClampBase[0] = out->viewAngleClampBase[0] + cgameGlob->gunnerPitchOffset;
                    }
                    else
                    {
                        AnglesToAxis(vehAngles, vehMat);
                        viewYawAngles[0] = 0.0f;
                        viewYawAngles[1] = yawOffset;
                        viewYawAngles[2] = 0.0f;
                        AnglesToAxis(viewYawAngles, viewMat);
                        MatrixMultiply(viewMat, vehMat, resultMat);
                        AxisToAngles(resultMat, centerAngles);
                        out->viewAngleClampBase[0] = (float)((float)(pitchClamp - out->viewAngleClampRange[0]) / 2.0)
                                                                             + centerAngles[0];
                        out->viewAngleClampRange[0] = (float)(pitchClamp + out->viewAngleClampRange[0]) / 2.0;
                    }
                }
            }
        }
    }
}

char __cdecl CG_ShouldInterpolatePlayerStateViewClamp(int localClientNum, const snapshot_s *prevSnap)
{
    const vehicle_info_t *vehInfo; // [esp+8h] [ebp-8h]
    centity_s *cent; // [esp+Ch] [ebp-4h]

    if ( prevSnap->ps.pm_type == 1 )
        return 1;
    if ( (prevSnap->ps.eFlags & 0x300) != 0 )
        return 1;
    if ( (prevSnap->ps.eFlags & 0x4000) != 0 )
    {
        cent = CG_GetEntity(localClientNum, prevSnap->ps.viewlocked_entNum);
        if ( cent )
        {
            if ( ((*((unsigned int *)cent + 201) >> 1) & 1) != 0 )
            {
                vehInfo = CG_GetVehicleInfo(cent->nextState.vehicleState.vehicleInfoIndex);
                if ( !vehInfo
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_predict_mp.cpp",
                                171,
                                0,
                                "%s",
                                "vehInfo") )
                {
                    __debugbreak();
                }
                if ( vehInfo->turretClampPlayerView )
                    return 1;
            }
        }
    }
    return 0;
}

void __cdecl CG_InterpolatePlayerState(int localClientNum, int grabAngles)
{
    double v2; // st7
    double v3; // st7
    double v4; // st7
    float v5; // [esp+10h] [ebp-80h]
    float v6; // [esp+1Ch] [ebp-74h]
    float v7; // [esp+28h] [ebp-68h]
    int i; // [esp+38h] [ebp-58h]
    int cmdNum; // [esp+3Ch] [ebp-54h]
    usercmd_s cmd; // [esp+40h] [ebp-50h] BYREF
    playerState_s *out; // [esp+78h] [ebp-18h]
    cg_s *cgameGlob; // [esp+7Ch] [ebp-14h]
    const snapshot_s *prevSnap; // [esp+80h] [ebp-10h]
    const snapshot_s *nextSnap; // [esp+84h] [ebp-Ch]
    float f; // [esp+88h] [ebp-8h]
    int link; // [esp+8Ch] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    out = &cgameGlob->predictedPlayerState;
    prevSnap = cgameGlob->snap;
    nextSnap = cgameGlob->nextSnap;
    if ( !prevSnap
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_predict_mp.cpp", 390, 0, "%s", "prevSnap") )
    {
        __debugbreak();
    }
    if ( !nextSnap
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_predict_mp.cpp", 391, 0, "%s", "nextSnap") )
    {
        __debugbreak();
    }
    memcpy((unsigned __int8 *)out, (unsigned __int8 *)&nextSnap->ps, sizeof(playerState_s));
    if ( grabAngles )
    {
        for ( i = 0; i < 2; ++i )
            cmd.button_bits.array[i] = 0;
        cmdNum = CL_GetCurrentCmdNumber(localClientNum);
        CL_GetUserCmd(localClientNum, cmdNum, &cmd);
        PM_UpdateViewAngles(localClientNum, out, 0.0, &cmd, 0);
    }
    if ( nextSnap->serverTime > prevSnap->serverTime
        && (!Demo_IsPlaying() || prevSnap->ps.pm_type < 9 || nextSnap->ps.pm_type >= 6) )
    {
        f = cgameGlob->frameInterpolation;
        link = nextSnap->ps.bobCycle;
        if ( link < prevSnap->ps.bobCycle )
            link += 256;
        out->bobCycle = prevSnap->ps.bobCycle + (int)(float)((float)(link - prevSnap->ps.bobCycle) * f);
        out->aimSpreadScale = (float)((float)(nextSnap->ps.aimSpreadScale - prevSnap->ps.aimSpreadScale) * f)
                                                + prevSnap->ps.aimSpreadScale;
        Vec3Lerp(prevSnap->ps.origin, nextSnap->ps.origin, f, out->origin);
        Vec3Lerp(prevSnap->ps.velocity, nextSnap->ps.velocity, f, out->velocity);
        if ( !grabAngles )
        {
            for ( link = 0; link < 3; ++link )
            {
                v7 = prevSnap->ps.viewangles[link];
                v2 = AngleNormalize180(nextSnap->ps.viewangles[link] - v7);
                out->viewangles[link] = v2 * f + v7;
                v6 = prevSnap->ps.delta_angles[link];
                v3 = AngleNormalize180(nextSnap->ps.delta_angles[link] - v6);
                out->delta_angles[link] = v3 * f + v6;
                if ( prevSnap->ps.pm_type == 1 )
                {
                    v5 = prevSnap->ps.linkAngles[link];
                    v4 = AngleNormalize180(nextSnap->ps.linkAngles[link] - v5);
                    out->linkAngles[link] = v4 * f + v5;
                }
                else
                {
                    out->linkAngles[link] = nextSnap->ps.linkAngles[link];
                }
            }
        }
        if ( nextSnap->ps.viewHeightLerpTime )
            out->viewHeightCurrent = (float)((float)(nextSnap->ps.viewHeightCurrent - prevSnap->ps.viewHeightCurrent) * f)
                                                         + prevSnap->ps.viewHeightCurrent;
        else
            out->viewHeightCurrent = nextSnap->ps.viewHeightCurrent;
        out->leanf = (float)((float)(nextSnap->ps.leanf - prevSnap->ps.leanf) * f) + prevSnap->ps.leanf;
        if ( (prevSnap->ps.eFlags & 0x4000) == (nextSnap->ps.eFlags & 0x4000) )
            out->fWeaponPosFrac = (float)((float)(nextSnap->ps.fWeaponPosFrac - prevSnap->ps.fWeaponPosFrac) * f)
                                                    + prevSnap->ps.fWeaponPosFrac;
        if ( !nextSnap->ps.cursorHint )
        {
            out->cursorHintString = -1;
            out->cursorHintEntIndex = 1023;
        }
        CG_InterpolatePlayerStateViewAngles(localClientNum, out, 0);
    }
}

void __cdecl CG_CopyFlagsFromSnapshotEntity(int localClientNum)
{
    centity_s *Entity; // eax
    cg_s *cgameGlob; // [esp+8h] [ebp-8h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    Entity = CG_GetEntity(localClientNum, cgameGlob->clientNum);
    cgameGlob->predictedPlayerEntity.flagIndex = Entity->flagIndex;
    cgameGlob->predictedPlayerEntity.flagState = Entity->flagState;
}

pmove_t::pmove_t()
{
    int j; // [esp+30h] [ebp-10h]
    int i; // [esp+3Ch] [ebp-4h]

    for ( i = 0; i < 2; ++i )
        this->cmd.button_bits.array[i] = 0;
    for ( j = 0; j < 2; ++j )
        this->oldcmd.button_bits.array[j] = 0;
    //colgeom_visitor_t::colgeom_visitor_t(&this->proximity_data);
    //this->proximity_data.__vftable = (colgeom_visitor_inlined_t<200>_vtbl *)&colgeom_visitor_inlined_t<200>::`vftable';
    //colgeom_visitor_inlined_t<500>::reset(&this->proximity_data);
    this->proximity_data.reset();
    //return this;
}

pmove_t::pmove_t(const pmove_t *__that)
{
    colgeom_visitor_inlined_t<200> *p_proximity_data; // [esp+10h] [ebp-4h]

    this->ps = __that->ps;
    memcpy(&this->cmd, &__that->cmd, sizeof(this->cmd));
    memcpy(&this->oldcmd, &__that->oldcmd, 0x3Cu);
    memcpy(this->touchents, __that->touchents, 0x84u);
    memcpy(this->touchGlasses, __that->touchGlasses, 0xA8u);
    memcpy(this->pitchHistory, __that->pitchHistory, 0x81u);
    this->mantleEndPos[0] = __that->mantleEndPos[0];
    this->mantleEndPos[1] = __that->mantleEndPos[1];
    this->mantleEndPos[2] = __that->mantleEndPos[2];
    this->mantleDuration = __that->mantleDuration;
    this->viewChangeTime = __that->viewChangeTime;
    this->viewChange = __that->viewChange;
    this->vehicleAngles[0] = __that->vehicleAngles[0];
    this->vehicleAngles[1] = __that->vehicleAngles[1];
    this->vehicleAngles[2] = __that->vehicleAngles[2];
    this->vehAnimState = __that->vehAnimState;
    this->handler = __that->handler;
    this->localClientNum = __that->localClientNum;
    this->m_gjkcc_input = __that->m_gjkcc_input;
    p_proximity_data = &this->proximity_data;
    //colgeom_visitor_t::colgeom_visitor_t(&this->proximity_data, &__that->proximity_data);
    //p_proximity_data->__vftable = (colgeom_visitor_inlined_t<200>_vtbl *)&colgeom_visitor_inlined_t<200>::`vftable';
    p_proximity_data->nprims = __that->proximity_data.nprims;
    p_proximity_data->overflow = __that->proximity_data.overflow;
    memcpy(p_proximity_data->prims, __that->proximity_data.prims, sizeof(p_proximity_data->prims));
    //return this;
}