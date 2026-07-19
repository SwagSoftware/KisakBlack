#include "cg_playerstate.h"
#include <cgame_mp/cg_local_mp.h>
#include <cgame_mp/cg_predict_mp.h>
#include "cg_camerashake.h"
#include <client_mp/cl_input_mp.h>
#include <client_mp/cl_cgame_mp.h>
#include "offhandweapons.h"
#include <cgame_mp/cg_view_mp.h>
#include "cg_main.h"
#include <cgame_mp/cg_main_mp.h>
#include <clientscript/cscr_vm.h>
#include <clientscript/scr_const.h>
#include <bgame/bg_misc.h>
#include <bgame/bg_perks.h>
#include <universal/com_math_anglevectors.h>
#include <cgame_mp/cg_servercmds_mp.h>
#include "cg_event.h"

void __cdecl CG_Respawn(int localClientNum, int spectate)
{
    cg_s *cgameGlob; // [esp+24h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( spectate )
        CG_SavePlayerState(localClientNum);
    else
        CG_ClearSavedPlayerState(localClientNum);
    if ( !cgameGlob->snap
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_playerstate.cpp",
                    127,
                    0,
                    "%s",
                    "cgameGlob->snap") )
    {
        __debugbreak();
    }
    memcpy(
        (unsigned __int8 *)&cgameGlob->predictedPlayerState,
        (unsigned __int8 *)&cgameGlob->snap->ps,
        sizeof(cgameGlob->predictedPlayerState));
    cgameGlob->weaponSelectTime = cgameGlob->time;
    cgameGlob->equippedOffHand = cgameGlob->predictedPlayerState.offHandIndex;
    cgameGlob->prevViewmodelWeapon = 0;
    cgameGlob->cursorHintIcon = 0;
    cgameGlob->cursorHintTime = 0;
    cgameGlob->proneBlockedEndTime = 0;
    cgameGlob->recoilAngles[0] = 0.0f;
    cgameGlob->recoilAngles[1] = 0.0f;
    cgameGlob->recoilAngles[2] = 0.0f;
    cgameGlob->recoilSpeed[0] = 0.0f;
    cgameGlob->recoilSpeed[1] = 0.0f;
    cgameGlob->recoilSpeed[2] = 0.0f;
    cgameGlob->swayViewAngles[0] = 0.0f;
    cgameGlob->swayViewAngles[1] = 0.0f;
    cgameGlob->swayViewAngles[2] = 0.0f;
    cgameGlob->swayAngles[0] = 0.0f;
    cgameGlob->swayAngles[1] = 0.0f;
    cgameGlob->swayAngles[2] = 0.0f;
    cgameGlob->swayOffset[0] = 0.0f;
    cgameGlob->swayOffset[1] = 0.0f;
    cgameGlob->swayOffset[2] = 0.0f;
    cgameGlob->kickAngles[0] = 0.0f;
    cgameGlob->kickAngles[1] = 0.0f;
    cgameGlob->kickAngles[2] = 0.0f;
    cgameGlob->kickAVel[0] = 0.0f;
    cgameGlob->kickAVel[1] = 0.0f;
    cgameGlob->kickAVel[2] = 0.0f;
    cgameGlob->xyspeed = 0.0f;
    memset((unsigned __int8 *)&cgameGlob->playerEntity, 0, sizeof(cgameGlob->playerEntity));
    cgameGlob->damageTime = 0;
    cgameGlob->v_dmg_pitch = 0.0f;
    cgameGlob->v_dmg_roll = 0.0f;
    memset((unsigned __int8 *)cgameGlob->viewDamage, 0, sizeof(cgameGlob->viewDamage));
    CG_ClearCameraShakes(localClientNum);
    cgameGlob->predictedError[0] = 0.0f;
    cgameGlob->predictedError[1] = 0.0f;
    cgameGlob->predictedError[2] = 0.0f;
    cgameGlob->adsViewErrorDone = 0;
    CL_SetStance(localClientNum, CL_STANCE_STAND);
    cgameGlob->warCurrentMomentumProgress = cgameGlob->warDesiredMomentumProgress;
    CL_SetADS(localClientNum, 0);
    CG_SetEquippedOffHand(localClientNum, cgameGlob->predictedPlayerState.offHandIndex);
    cgameGlob->playerEntity.fLastIdleFactor = 0.0f;
    cgameGlob->weapIdleTime = 0;
    cgameGlob->heightToCeiling = FLT_MAX;
    CG_HoldBreathInit(cgameGlob);
    CG_ResetFullscreenFX(localClientNum);
    if ( (cgameGlob->snap->ps.otherFlags & 0xA) != 0 )
        CG_SetThirdPerson(cg_spectateThirdPerson->current.color[0]);
    else
        CG_SetThirdPerson(0);
    CScr_NotifyNum(localClientNum, cgameGlob->predictedPlayerState.clientNum, 0, cscr_const.respawn, 0);
}

void __cdecl CG_ResetSpectatorViewAngles(int localClientNum)
{
    float angles[3]; // [esp+18h] [ebp-Ch] BYREF

    memset(angles, 0, sizeof(angles));
    CL_SetViewAngles(localClientNum, angles);
    Com_Printf(0, "CL_SetViewAngles() - SPECCY - (%3.0f, %3.0f, %3.0f)\n", angles[0], angles[1], angles[2]);
}

void __cdecl CG_TransitionPlayerState(int localClientNum, playerState_s *ps, const transPlayerState_t *ops)
{
    if ( ps->damageEvent != ops->damageEvent && ps->damageCount )
        CG_DamageFeedback(localClientNum, ps->damageYaw, ps->damagePitch, ps->damageCount);
    CG_CheckPlayerstateEvents(localClientNum, ps, ops);
}

void __cdecl CG_DamageFeedback(int localClientNum, int yawByte, int pitchByte, int damage)
{
    double v4; // st7
    float angle; // [esp+0h] [ebp-58h]
    float kick; // [esp+20h] [ebp-38h]
    float dir[3]; // [esp+24h] [ebp-34h] BYREF
    int slot; // [esp+30h] [ebp-28h]
    cg_s *cgameGlob; // [esp+34h] [ebp-24h]
    float yaw; // [esp+38h] [ebp-20h]
    float forwardFrac; // [esp+3Ch] [ebp-1Ch]
    float sideFrac; // [esp+40h] [ebp-18h]
    float angles[3]; // [esp+44h] [ebp-14h] BYREF
    float pitch; // [esp+50h] [ebp-8h]
    int i; // [esp+54h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    kick = (float)damage * bg_viewKickScale->current.value;
    if ( (cgameGlob->predictedPlayerState.perks[0] & 0x10) != 0 )
        kick = kick * perk_damageKickReduction->current.value;
    if ( bg_viewKickMin->current.value <= kick )
    {
        if ( kick > bg_viewKickMax->current.value )
            kick = bg_viewKickMax->current.value;
    }
    else
    {
        kick = bg_viewKickMin->current.value;
    }
    if ( yawByte == 255 && pitchByte == 255 )
    {
        cgameGlob->v_dmg_roll = 0.0f;
        cgameGlob->v_dmg_pitch = -kick;
    }
    else
    {
        pitch = (float)((float)pitchByte / 255.0) * 360.0;
        yaw = (float)((float)yawByte / 255.0) * 360.0;
        angles[0] = pitch;
        angles[1] = yaw;
        angles[2] = 0.0f;
        AngleVectors(angles, dir, 0, 0);
        sideFrac = (float)((float)(dir[0] * cgameGlob->refdef.viewaxis[1][0])
                                         + (float)(dir[1] * cgameGlob->refdef.viewaxis[1][1]))
                         + (float)(dir[2] * cgameGlob->refdef.viewaxis[1][2]);
        forwardFrac = (float)((float)(dir[0] * cgameGlob->refdef.viewaxis[0][0])
                                                + (float)(dir[1] * cgameGlob->refdef.viewaxis[0][1]))
                                + (float)(dir[2] * cgameGlob->refdef.viewaxis[0][2]);
        if ( yawByte || pitchByte )
        {
            //cgameGlob->v_dmg_roll = COERCE_FLOAT(LODWORD(kick) ^ _mask__NegFloat_) * sideFrac;
            cgameGlob->v_dmg_roll = -kick * sideFrac;
            cgameGlob->v_dmg_pitch = kick * forwardFrac;
        }
        slot = 0;
        for ( i = 1; i < 8; ++i )
        {
            if ( cgameGlob->viewDamage[i].time < cgameGlob->viewDamage[slot].time )
                slot = i;
        }
        cgameGlob->viewDamage[slot].time = cgameGlob->snap->serverTime;
        cgameGlob->viewDamage[slot].duration = cg_hudDamageIconTime->current.integer;
        angle = (Com_Random() - 0.5) * 20.0 + yaw;
        v4 = AngleNormalize360(angle);
        cgameGlob->viewDamage[slot].yaw = v4;
    }
    cgameGlob->v_dmg_time = cgameGlob->time + 500;
    cgameGlob->damageTime = cgameGlob->snap->serverTime;
    CG_MenuShowNotify(localClientNum, 0);
}

void __cdecl CG_CheckPlayerstateEvents(int localClientNum, playerState_s *ps, const transPlayerState_t *ops)
{
    __int16 v3; // [esp+6h] [ebp-22h]
    __int16 unpredictableEventSequence; // [esp+8h] [ebp-20h]
    __int16 predictableEventSequence; // [esp+Ah] [ebp-1Eh]
    int eventIndex; // [esp+Ch] [ebp-1Ch]
    __int16 prevEvtSeq; // [esp+10h] [ebp-18h]
    int event; // [esp+14h] [ebp-14h]
    int eventa; // [esp+14h] [ebp-14h]
    int eventb; // [esp+14h] [ebp-14h]
    centity_s *cent; // [esp+1Ch] [ebp-Ch]
    int i; // [esp+20h] [ebp-8h]
    int ia; // [esp+20h] [ebp-8h]

    cent = &CG_GetLocalClientGlobals(localClientNum)->predictedPlayerEntity;
    predictableEventSequence = ops->predictableEventSequence;
    if ( predictableEventSequence <= ps->predictableEventSequence + 64 )
        prevEvtSeq = ops->predictableEventSequence;
    else
        prevEvtSeq = predictableEventSequence - 256;
    for ( i = ps->predictableEventSequence - 4; i < ps->predictableEventSequence; ++i )
    {
        eventIndex = i & 3;
        if ( i < prevEvtSeq )
        {
            if ( i > prevEvtSeq - 4 && ps->predictableEvents[eventIndex] != ops->predictableEvents[eventIndex] )
            {
                eventa = ps->predictableEvents[eventIndex];
                cent->nextState.eventParm = ps->predictableEventParms[eventIndex];
                CG_EntityEvent(localClientNum, cent, eventa);
                if ( Dvar_GetBool("showevents") )
                    Com_Printf(
                        17,
                        "processed unanticipated predicted event svt %5d -> %5d/%5d: num = %20s parm %d, index %d\n",
                        ps->commandTime,
                        ps->predictableEventSequence,
                        ops->predictableEventSequence,
                        eventnames[eventa],
                        cent->nextState.eventParm,
                        eventIndex);
            }
        }
        else
        {
            event = ps->predictableEvents[eventIndex];
            cent->nextState.eventParm = ps->predictableEventParms[eventIndex];
            CG_EntityEvent(localClientNum, cent, event);
            if ( Dvar_GetBool("showevents") )
                Com_Printf(
                    17,
                    "processed predicted event svt %5d -> %5d/%5d: num = %20s parm %d, index %d\n",
                    ps->commandTime,
                    ps->predictableEventSequence,
                    ops->predictableEventSequence,
                    eventnames[event],
                    cent->nextState.eventParm,
                    eventIndex);
        }
    }
    unpredictableEventSequence = ops->unpredictableEventSequence;
    if ( unpredictableEventSequence <= ps->unpredictableEventSequence + 64 )
        v3 = ops->unpredictableEventSequence;
    else
        v3 = unpredictableEventSequence - 256;
    for ( ia = v3; ia < ps->unpredictableEventSequence; ++ia )
    {
        eventb = ps->unpredictableEvents[ia & 3];
        cent->nextState.eventParm = ps->unpredictableEventParms[ia & 3];
        CG_EntityEvent(localClientNum, cent, eventb);
        if ( Dvar_GetBool("showevents") )
            Com_Printf(
                17,
                "processed unpredicted event svt %5d -> %5d/%5d: num = %20s parm %d, index %d\n",
                ps->commandTime,
                ps->unpredictableEventSequence,
                ops->unpredictableEventSequence,
                eventnames[eventb],
                cent->nextState.eventParm,
                ia & 3);
    }
}

void __cdecl CG_ClearTransPlayerState(transPlayerState_t *transPs)
{
    transPs->otherFlags = 0;
    transPs->damageEvent = 0;
    transPs->predictableEventSequence = 0;
    transPs->predictableEvents[0] = 0;
    transPs->predictableEvents[1] = 0;
    transPs->predictableEvents[2] = 0;
    transPs->predictableEvents[3] = 0;
    transPs->unpredictableEventSequence = 0;
    transPs->unpredictableEvents[0] = 0;
    transPs->unpredictableEvents[1] = 0;
    transPs->unpredictableEvents[2] = 0;
    transPs->unpredictableEvents[3] = 0;
}

void __cdecl CG_ExtractTransPlayerState(const playerState_s *ps, transPlayerState_t *transPs)
{
    transPs->otherFlags = ps->otherFlags;
    transPs->damageEvent = ps->damageEvent;
    transPs->predictableEventSequence = ps->predictableEventSequence;
    transPs->predictableEvents[0] = ps->predictableEvents[0];
    transPs->predictableEvents[1] = ps->predictableEvents[1];
    transPs->predictableEvents[2] = ps->predictableEvents[2];
    transPs->predictableEvents[3] = ps->predictableEvents[3];
    transPs->unpredictableEventSequence = ps->unpredictableEventSequence;
    transPs->unpredictableEvents[0] = ps->unpredictableEvents[0];
    transPs->unpredictableEvents[1] = ps->unpredictableEvents[1];
    transPs->unpredictableEvents[2] = ps->unpredictableEvents[2];
    transPs->unpredictableEvents[3] = ps->unpredictableEvents[3];
}

