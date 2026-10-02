#include "aim_target.h"

#include <cstring>
#include <xanim/dobj.h>
#include <qcommon/dobj_management.h>
#include <bgame/bg_local.h>
#include <cgame_mp/cg_ents_mp.h>
#include <clientscript/cscr_stringlist.h>
#include <qcommon/common.h>
#include <qcommon/cmd.h>
#include <universal/dvar.h>
#include <cgame_mp/cg_local_mp.h>
#include <demo/demo_common.h>
#include <gfx_d3d/r_dpvs.h>
#include <cgame_mp/cg_players_mp.h>
#include <universal/com_math.h>
#include <clientscript/scr_const.h>
#include <universal/com_math_anglevectors.h>
#include <bgame/bg_animconditions.h>
#include <bgame/bg_misc.h>
#include <cgame_mp/cg_main_mp.h>
#include <qcommon/cm_load.h>
#include <qcommon/cm_trace.h>
#include <qcommon/cm_test.h>
#include <EffectsCore/fx_system.h>
#include <cgame/cg_world.h>

static const float dog_length = 20.0f;

AimTargetGlob atGlobArray[1];

const dvar_s *aim_target_sentient_radius;
const dvar_s *aim_target_sentient_half_height;
const dvar_s *aim_target_frustum_expand_on_screen;
const dvar_s *aim_target_frustum_expand_fast_updates;
const dvar_s *aim_target_frustum_min_distance;
const dvar_s *aim_target_aim_tag_fast_update_interval;
const dvar_s *aim_target_aim_tag_slow_update_interval;

AimTargetGlob *__cdecl AimTarget_GetGlobArray(int localClientNum)
{
    return &atGlobArray[localClientNum];
}

void __cdecl AimTarget_Init(int localClientNum)
{
    unsigned __int8 *GlobArray; // eax

    GlobArray = (unsigned __int8 *)AimTarget_GetGlobArray(localClientNum);
    memset(GlobArray, 0, 0xC04u);
    AimTarget_RegisterDvars();
    Cbuf_InsertText(0, "exec devgui_aimassist\n");
}

void AimTarget_RegisterDvars()
{
    aim_target_sentient_radius = _Dvar_RegisterFloat(
                                                                 "aim_target_sentient_radius",
                                                                 10.0,
                                                                 0.0,
                                                                 128.0,
                                                                 0x80u,
                                                                 "The radius used to calculate target bounds for a sentient(actor or player)");
    aim_target_sentient_half_height = _Dvar_RegisterFloat(
                                                                            "aim_target_sentient_half_height",
                                                                            32.0,
                                                                            0.0,
                                                                            128.0,
                                                                            0x80u,
                                                                            "The radius used to calculate frustum target center for a sentient(actor or player)");
    aim_target_frustum_expand_on_screen = _Dvar_RegisterFloat(
                                                                                    "aim_target_frustum_expand_on_screen",
                                                                                    -30.0,
                                                                                    -1000.0,
                                                                                    1000.0,
                                                                                    0x80u,
                                                                                    "The frustum expansion to determine if a target is on screen.    Positive is inwards.");
    aim_target_frustum_expand_fast_updates = _Dvar_RegisterFloat(
                                                                                         "aim_target_frustum_expand_fast_updates",
                                                                                         40.0,
                                                                                         -1000.0,
                                                                                         1000.0,
                                                                                         0x80u,
                                                                                         "The frustum expansion to determine if a target is in the fast interval tag "
                                                                                         "update zone.    Positive is inwards.");
    aim_target_frustum_min_distance = _Dvar_RegisterFloat(
                                                                            "aim_target_frustum_min_distance",
                                                                            256.0,
                                                                            0.0,
                                                                            10000.0,
                                                                            0x80u,
                                                                            "If target is inside this distance frustum culling is not applied.");
    aim_target_aim_tag_fast_update_interval = _Dvar_RegisterInt(
                                                                                            "aim_target_aim_tag_fast_update_interval",
                                                                                            59,
                                                                                            1,
                                                                                            10000,
                                                                                            0x80u,
                                                                                            "How often the aim target tag is updated instead of pulled from cache insid"
                                                                                            "e of the \"fast\" zone.");
    aim_target_aim_tag_slow_update_interval = _Dvar_RegisterInt(
                         "aim_target_aim_tag_slow_update_interval",
                         131,
                         1,
                         10000,
                         0x80u,
                         "How often the aim target tag is updated instead of pulled from cache inside of the \"fast\" zone.");
}

void __cdecl expandMins(float *mins, float *point)
{
    if ( *mins > *point )
        *mins = *point;
    if ( mins[1] > point[1] )
        mins[1] = point[1];
    if ( mins[2] > point[2] )
        mins[2] = point[2];
}

void __cdecl expandMaxs(float *maxs, float *point)
{
    if ( *point > *maxs )
        *maxs = *point;
    if ( point[1] > maxs[1] )
        maxs[1] = point[1];
    if ( point[2] > maxs[2] )
        maxs[2] = point[2];
}

void __cdecl AimTarget_ProcessEntityInternal(int localClientNum, const centity_s *ent)
{
    const cg_s *cgameGlob; // [esp+3Ch] [ebp-38h]
    AimTarget target; // [esp+40h] [ebp-34h] BYREF
    unsigned int visBone; // [esp+70h] [ebp-4h]

    PROF_SCOPED("AimTarget_ProcessEntity");

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_target.cpp", 902, 0, "%s", "ent") )
        __debugbreak();
    if ( ((*((unsigned int *)ent + 201) >> 1) & 1) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_target.cpp",
                    903,
                    0,
                    "%s",
                    "ent->nextValid") )
    {
        __debugbreak();
    }
    if ( ent->nextState.number == cgameGlob->predictedPlayerState.clientNum
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_target.cpp",
                    905,
                    0,
                    "%s",
                    "ent->nextState.number != cgameGlob->predictedPlayerState.clientNum") )
    {
        __debugbreak();
    }
    if ( Demo_IsPlaying() )
    {
            return;
    }
    if ( !AimTarget_PlayerInValidState(&cgameGlob->predictedPlayerState) )
    {
            return;
    }
    if ( ent->nextState.eType == 1 || ent->nextState.eType == 17 )
    {
        visBone = scr_const.aim_vis_bone;
    }
    else
    {
        if ( (ent->nextState.lerp.eFlags & 0x800) == 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_target.cpp",
                        922,
                        0,
                        "%s",
                        "ent->nextState.lerp.eFlags & EF_AIM_ASSIST") )
        {
            __debugbreak();
        }
        visBone = 0;
    }
    if ( !AimTarget_IsTargetValid(cgameGlob, ent) )
    {
            return;
    }
    if ((ent->nextState.lerp.eFlags2 & 0x1000000) != 0) 
    {
            return;
    }
    if ( ent->pose.eType == 17 && (ent->nextState.lerp.eFlags & 0x40000) != 0 )
    {
            return;
    }
    else if ( player_topDownCamMode->current.integer || AimTarget_IsTargetVisible(localClientNum, ent, visBone) )
    {
        AimTarget_CreateTarget(localClientNum, ent, &target);
        return;
    }
}

bool __cdecl AimTarget_IsTargetValid(const cg_s *cgameGlob, const centity_s *targetEnt)
{
    const clientInfo_t *playerInfo; // [esp+24h] [ebp-40h]
    team_t playerInfoTeam; // [esp+2Ch] [ebp-38h]
    const clientInfo_t *localTargetInfo; // [esp+30h] [ebp-34h]
    float targetDir; // [esp+34h] [ebp-30h]
    float targetDir_4; // [esp+38h] [ebp-2Ch]
    float targetDir_8; // [esp+3Ch] [ebp-28h]
    float top[3]; // [esp+40h] [ebp-24h] BYREF
    float radius; // [esp+4Ch] [ebp-18h]
    const playerState_s *predictedPlayerState; // [esp+50h] [ebp-14h]
    float playerDir[3]; // [esp+54h] [ebp-10h]
    float dot; // [esp+60h] [ebp-4h]

    if ( !cgameGlob
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_target.cpp", 582, 0, "%s", "cgameGlob") )
    {
        __debugbreak();
    }
    if ( !targetEnt
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_target.cpp", 583, 0, "%s", "targetEnt") )
    {
        __debugbreak();
    }
    if ( ((*((unsigned int *)targetEnt + 201) >> 1) & 1) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_target.cpp",
                    584,
                    0,
                    "%s",
                    "targetEnt->nextValid") )
    {
        __debugbreak();
    }
    if ( targetEnt->nextState.number == cgameGlob->predictedPlayerState.clientNum
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_target.cpp",
                    586,
                    0,
                    "%s",
                    "targetEnt->nextState.number != cgameGlob->predictedPlayerState.clientNum") )
    {
        __debugbreak();
    }
    predictedPlayerState = &cgameGlob->predictedPlayerState;
    if ( targetEnt->nextState.eType == 1 )
    {
        if ( (targetEnt->nextState.lerp.eFlags & 0x40000) != 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_target.cpp",
                        596,
                        0,
                        "%s",
                        "!(targetEnt->nextState.lerp.eFlags & EF_DEAD)") )
        {
            __debugbreak();
        }
        if ( targetEnt->nextState.clientNum >= 0x20u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_target.cpp",
                        598,
                        0,
                        "targetEnt->nextState.clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                        targetEnt->nextState.clientNum,
                        32) )
        {
            __debugbreak();
        }
        localTargetInfo = &cgameGlob->bgs.clientinfo[targetEnt->nextState.clientNum];
        if ( predictedPlayerState->clientNum >= 0x20u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_target.cpp",
                        604,
                        0,
                        "predictedPlayerState->clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                        predictedPlayerState->clientNum,
                        32) )
        {
            __debugbreak();
        }
        playerInfo = &cgameGlob->bgs.clientinfo[predictedPlayerState->clientNum];
        if ( !localTargetInfo->infoValid || !localTargetInfo->model[0] )
            return 0;
        if ( !Com_GetClientDObj(targetEnt->nextState.number, targetEnt->pose.localClientNum)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_target.cpp",
                        613,
                        0,
                        "%s",
                        "Com_GetClientDObj( targetEnt->nextState.number, targetEnt->pose.localClientNum )") )
        {
            __debugbreak();
        }
        playerInfoTeam = playerInfo->team;
        if ( localTargetInfo->team == playerInfoTeam && playerInfoTeam )
            return 0;
    }
    else if ( targetEnt->nextState.eType == 17 )
    {
        if ( predictedPlayerState->clientNum >= 0x20u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_target.cpp",
                        627,
                        0,
                        "predictedPlayerState->clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                        predictedPlayerState->clientNum,
                        32) )
        {
            __debugbreak();
        }
        if ( targetEnt->nextState.lerp.u.actor.team == cgameGlob->bgs.clientinfo[predictedPlayerState->clientNum].team )
            return 0;
    }
    else if ( (targetEnt->nextState.lerp.eFlags & 0x800) == 0 )
    {
        return 0;
    }
    targetDir = targetEnt->pose.origin[0] - predictedPlayerState->origin[0];
    targetDir_4 = targetEnt->pose.origin[1] - predictedPlayerState->origin[1];
    targetDir_8 = targetEnt->pose.origin[2] - predictedPlayerState->origin[2];
    if ( player_topDownCamMode->current.integer > 0 )
        return 1;
    *(_QWORD *)playerDir = *(_QWORD *)&cgameGlob->refdef.viewaxis[0][0];
    playerDir[2] = cgameGlob->refdef.viewaxis[0][2];
    radius = AimTarget_GetTargetRadius(targetEnt);
    dot = (float)((float)((float)(playerDir[0] * targetDir) + (float)(playerDir[1] * targetDir_4))
                            + (float)(playerDir[2] * targetDir_8))
            + radius;
    if ( dot < 0.0 )
        return 0;
    if ( aim_target_frustum_min_distance->current.value > Vec3DistanceSq(
                                                                                                                    predictedPlayerState->origin,
                                                                                                                    targetEnt->pose.origin) )
        return 1;
    if ( !CG_PlayerInViewFrustum(targetEnt->pose.localClientNum, targetEnt) )
        return 0;
    AimTarget_GetTargetTop(targetEnt, top);
    return R_CullPoint(cgameGlob->localClientNum, top, aim_target_frustum_expand_on_screen->current.value) == 0;
}

double __cdecl AimTarget_GetTargetRadius(const centity_s *targetEnt)
{
    float mins[3]; // [esp+0h] [ebp-1Ch] BYREF
    float maxs[3]; // [esp+10h] [ebp-Ch] BYREF

    if ( !targetEnt
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_target.cpp", 519, 0, "%s", "targetEnt") )
    {
        __debugbreak();
    }
    if ( targetEnt->nextState.eType == 1 || targetEnt->nextState.eType == 17 )
        return aim_target_sentient_radius->current.value;
    AimTarget_GetTargetBounds(targetEnt, mins, maxs);
    return (float)RadiusFromBounds(mins, maxs);
}

void __cdecl AimTarget_GetTagPos_0(const centity_s *ent, unsigned int tagName, float *pos)
{
    char *v3; // eax
    char *v4; // eax
    DObj *dobj; // [esp+0h] [ebp-Ch]
    bool target_last_frame; // [esp+7h] [ebp-5h]
    int update_interval; // [esp+8h] [ebp-4h]

    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_target.cpp", 314, 0, "%s", "ent") )
        __debugbreak();
    if ( !pos && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_target.cpp", 315, 0, "%s", "pos") )
        __debugbreak();
    if ( ent->nextState.eType == 1 )
    {
        target_last_frame = AimAssist_IsPrevTargetEntity(ent->pose.localClientNum, ent->nextState.number);
        update_interval = AimTarget_GetTagUpdateInterval(ent);
        if ( !CachedTag_GetTagPos(ent, tagName, pos, update_interval, target_last_frame) )
        {
            v3 = SL_ConvertToString(tagName, SCRIPTINSTANCE_SERVER);
            Com_Error(ERR_DROP, "AimTarget_GetTagPos: Cannot find tag [%s] on entity\n", v3);
        }
    }
    else
    {
        dobj = Com_GetClientDObj(ent->nextState.number, ent->pose.localClientNum);
        if ( !dobj
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_target.cpp", 333, 0, "%s", "dobj") )
        {
            __debugbreak();
        }
        if ( !CG_DObjGetWorldTagPos(&ent->pose, dobj, tagName, pos) )
        {
            v4 = SL_ConvertToString(tagName, SCRIPTINSTANCE_SERVER);
            Com_Error(ERR_DROP, "AimTarget_GetTagPos: Cannot find tag [%s] on entity\n", v4);
        }
    }
}

void __cdecl AimTarget_GetTargetBounds(const centity_s *targetEnt, float *mins, float *maxs)
{
    float v3; // [esp+0h] [ebp-5Ch]
    float v4; // [esp+4h] [ebp-58h]
    float value; // [esp+8h] [ebp-54h]
    const DObj *dobj; // [esp+Ch] [ebp-50h]
    float head; // [esp+14h] [ebp-48h]
    float head_4; // [esp+18h] [ebp-44h]
    float head_8; // [esp+1Ch] [ebp-40h]
    float right[3]; // [esp+20h] [ebp-3Ch] BYREF
    float forward[3]; // [esp+2Ch] [ebp-30h] BYREF
    float left[3]; // [esp+38h] [ebp-24h] BYREF
    float tail[3]; // [esp+44h] [ebp-18h] BYREF
    float highBonePos[3]; // [esp+50h] [ebp-Ch] BYREF

    if ( !targetEnt
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_target.cpp", 427, 0, "%s", "targetEnt") )
    {
        __debugbreak();
    }
    if ( !mins && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_target.cpp", 428, 0, "%s", "mins") )
        __debugbreak();
    if ( !maxs && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_target.cpp", 429, 0, "%s", "maxs") )
        __debugbreak();
    if ( targetEnt->nextState.eType == 17 )
    {
        AimTarget_GetTagPos_0(targetEnt, scr_const.aim_highest_bone, highBonePos);
        AngleVectors(targetEnt->pose.angles, forward, right, 0);
        value = aim_target_sentient_radius->current.value;
        right[0] = value * right[0];
        right[1] = value * right[1];
        right[2] = value * right[2];
        left[0] = -1.0 * right[0];
        left[1] = -1.0 * right[1];
        left[2] = -1.0 * right[2];
        v4 = aim_target_sentient_radius->current.value;
        head = v4 * forward[0];
        head_4 = v4 * forward[1];
        head_8 = v4 * forward[2];
        v3 = (float)(v4 + dog_length) * -1.0;
        tail[0] = v3 * forward[0];
        tail[1] = v3 * forward[1];
        tail[2] = v3 * forward[2];
        *mins = v4 * forward[0];
        mins[1] = head_4;
        mins[2] = head_8;
        expandMins(mins, tail);
        expandMins(mins, right);
        expandMins(mins, left);
        *maxs = head;
        maxs[1] = head_4;
        maxs[2] = head_8;
        expandMaxs(maxs, tail);
        expandMaxs(maxs, right);
        expandMaxs(maxs, left);
        maxs[2] = (float)(highBonePos[2] - targetEnt->pose.origin[2]) + maxs[2];
    }
    else if ( targetEnt->nextState.eType == 1 || targetEnt->nextState.eType == 17 )
    {
        AimTarget_GetTagPos_0(targetEnt, scr_const.aim_highest_bone, highBonePos);
        //*(unsigned int *)mins = aim_target_sentient_radius->current.integer ^ _mask__NegFloat_;
        //*((unsigned int *)mins + 1) = aim_target_sentient_radius->current.integer ^ _mask__NegFloat_;
        mins[0] = -aim_target_sentient_radius->current.value;
        mins[1] = -aim_target_sentient_radius->current.value;
        mins[2] = 0.0f;

        maxs[0] = aim_target_sentient_radius->current.value;
        maxs[1] = aim_target_sentient_radius->current.value;
        maxs[2] = highBonePos[2] - targetEnt->pose.origin[2];
    }
    else if (targetEnt->nextState.solid == 0xFFFFFF)
    {
        CM_ModelBounds(targetEnt->nextState.index.brushmodel, mins, maxs);
    }
    else
    {
        dobj = Com_GetClientDObj(targetEnt->nextState.number, targetEnt->pose.localClientNum);
        if ( dobj )
        {
            DObjGetBounds(dobj, mins, maxs);
        }
        else
        {
            *mins = -1.0f;
            mins[1] = -1.0f;
            mins[2] = -1.0f;
            *maxs = 1.0f;
            maxs[1] = 1.0f;
            maxs[2] = 1.0f;
        }
    }
}

int __cdecl AimTarget_GetTagUpdateInterval(const centity_s *ent)
{
    const cg_s *cgameGlob; // [esp+14h] [ebp-10h]
    float center[3]; // [esp+18h] [ebp-Ch] BYREF

    cgameGlob = CG_GetLocalClientGlobals(ent->pose.localClientNum);
    center[0] = ent->pose.origin[0];
    center[1] = ent->pose.origin[1];
    center[2] = ent->pose.origin[2] + aim_target_sentient_half_height->current.value;
    if ( (double)aim_target_frustum_min_distance->current.value > Vec3DistanceSq(
                                                                                                                                    cgameGlob->predictedPlayerState.origin,
                                                                                                                                    ent->pose.origin) )
        return aim_target_aim_tag_fast_update_interval->current.integer;
    if ( R_CullPoint(ent->pose.localClientNum, center, aim_target_frustum_expand_fast_updates->current.value) )
        return aim_target_aim_tag_slow_update_interval->current.integer;
    return aim_target_aim_tag_fast_update_interval->current.integer;
}

void __cdecl AimTarget_GetTargetTop(const centity_s *targetEnt, float *top)
{
    float mins[3]; // [esp+4h] [ebp-18h] BYREF
    float maxs[3]; // [esp+10h] [ebp-Ch] BYREF

    if ( !targetEnt
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_target.cpp", 558, 0, "%s", "targetEnt") )
    {
        __debugbreak();
    }
    if ( !top && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_target.cpp", 559, 0, "%s", "top") )
        __debugbreak();
    AimTarget_GetTargetBounds(targetEnt, mins, maxs);
    *top = mins[0] + maxs[0];
    top[1] = mins[1] + maxs[1];
    top[2] = mins[2] + maxs[2];
    *top = 0.5 * *top;
    top[1] = 0.5 * top[1];
    top[2] = 0.5 * top[2];
    top[2] = maxs[2];
    *top = targetEnt->pose.origin[0] + *top;
    top[1] = targetEnt->pose.origin[1] + top[1];
    top[2] = targetEnt->pose.origin[2] + top[2];
}

bool __cdecl AimTarget_IsTargetVisible(int localClientNum, const centity_s *targetEnt, unsigned int visBone)
{
    col_context_t context; // [esp+8h] [ebp-98h] BYREF
    float endPos[3]; // [esp+30h] [ebp-70h] BYREF
    const cg_s *cgameGlob; // [esp+3Ch] [ebp-64h]
    const float (*localViewOrg)[3]; // [esp+40h] [ebp-60h]
    const playerState_s *predictedPlayerState; // [esp+44h] [ebp-5Ch]
    trace_t trace; // [esp+4Ch] [ebp-54h] BYREF
    float playerEyePos[3]; // [esp+88h] [ebp-18h] BYREF
    float targetEyePos[3]; // [esp+94h] [ebp-Ch] BYREF

    if ( !targetEnt
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_target.cpp", 736, 0, "%s", "targetEnt") )
    {
        __debugbreak();
    }
    if ( visBone )
        AimTarget_GetTagPos_0(targetEnt, visBone, targetEyePos);
    else
        AimTarget_GetTargetCenter(targetEnt, targetEyePos);
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    localViewOrg = (const float (*)[3])cgameGlob->refdef.vieworg;
    playerEyePos[0] = cgameGlob->refdef.vieworg[0];
    playerEyePos[1] = cgameGlob->refdef.vieworg[1];
    playerEyePos[2] = cgameGlob->refdef.vieworg[2];
    predictedPlayerState = &cgameGlob->predictedPlayerState;
    ////col_context_t::col_context_t(&context);
    CG_TraceCapsule(
        &trace,
        playerEyePos,
        vec3_origin,
        vec3_origin,
        targetEyePos,
        predictedPlayerState->clientNum,
        8400899,
        &context);
    if ( trace.fraction != 1.0 && Trace_GetEntityHitId(&trace) != targetEnt->nextState.number )
    {
        if (targetEnt->nextState.solid != 0xFFFFFF)
            return 0;
        Vec3Lerp(playerEyePos, targetEyePos, trace.fraction, endPos);
        if ( !CM_TransformedPointContents(
                        endPos,
                        targetEnt->nextState.index.brushmodel,
                        targetEnt->pose.origin,
                        targetEnt->pose.angles) )
            return 0;
    }
    return FX_ClientVisibilityTest(localClientNum, playerEyePos, targetEyePos);
}

void __cdecl AimTarget_GetTargetCenter(const centity_s *targetEnt, float *center)
{
    float mins[3]; // [esp+4h] [ebp-18h] BYREF
    float maxs[3]; // [esp+10h] [ebp-Ch] BYREF

    if ( !targetEnt
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_target.cpp", 539, 0, "%s", "targetEnt") )
    {
        __debugbreak();
    }
    if ( !center
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_target.cpp", 540, 0, "%s", "center") )
    {
        __debugbreak();
    }
    AimTarget_GetTargetBounds(targetEnt, mins, maxs);
    *center = mins[0] + maxs[0];
    center[1] = mins[1] + maxs[1];
    center[2] = mins[2] + maxs[2];
    *center = 0.5 * *center;
    center[1] = 0.5 * center[1];
    center[2] = 0.5 * center[2];
    *center = targetEnt->pose.origin[0] + *center;
    center[1] = targetEnt->pose.origin[1] + center[1];
    center[2] = targetEnt->pose.origin[2] + center[2];
}

void __cdecl AimTarget_CreateTarget(int localClientNum, const centity_s *targetEnt, AimTarget *target)
{
    float v3; // [esp+8h] [ebp-4Ch]
    int snapServerTime; // [esp+20h] [ebp-34h]
    const cg_s *cgameGlob; // [esp+24h] [ebp-30h]
    int nextSnapServerTime; // [esp+30h] [ebp-24h]
    float currentPos[3]; // [esp+38h] [ebp-1Ch] BYREF
    float nextPos[3]; // [esp+44h] [ebp-10h] BYREF
    float deltaTime; // [esp+50h] [ebp-4h]

    if ( !targetEnt
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_target.cpp", 805, 0, "%s", "targetEnt") )
    {
        __debugbreak();
    }
    if ( !target
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_target.cpp", 806, 0, "%s", "target") )
    {
        __debugbreak();
    }
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    target->entIndex = targetEnt->nextState.number;
    target->worldDistSqr = Vec3DistanceSq(cgameGlob->predictedPlayerState.origin, targetEnt->pose.origin);
    AimTarget_GetTargetBounds(targetEnt, target->mins, target->maxs);
    snapServerTime = cgameGlob->snap->serverTime;
    nextSnapServerTime = cgameGlob->nextSnap->serverTime;
    deltaTime = (float)(nextSnapServerTime - snapServerTime) * 0.001;
    if ( deltaTime <= 0.0 )
    {
        target->velocity[0] = 0.0f;
        target->velocity[1] = 0.0f;
        target->velocity[2] = 0.0f;
        target->heightChange = 0.0f;
    }
    else
    {
        BG_EvaluateTrajectory(&targetEnt->currentState.pos, snapServerTime, currentPos);
        BG_EvaluateTrajectory(&targetEnt->nextState.lerp.pos, nextSnapServerTime, nextPos);
        target->velocity[0] = nextPos[0] - currentPos[0];
        target->velocity[1] = nextPos[1] - currentPos[1];
        target->velocity[2] = nextPos[2] - currentPos[2];
        v3 = 1.0 / deltaTime;
        target->velocity[0] = (float)(1.0 / deltaTime) * target->velocity[0];
        target->velocity[1] = v3 * target->velocity[1];
        target->velocity[2] = v3 * target->velocity[2];
    }
    AimTarget_AddTargetToList(localClientNum, target);
}

void __cdecl AimTarget_AddTargetToList(int localClientNum, const AimTarget *target)
{
    int targetIndex; // [esp+8h] [ebp-14h]
    int low; // [esp+Ch] [ebp-10h]
    AimTargetGlob *atGlob; // [esp+14h] [ebp-8h]
    int high; // [esp+18h] [ebp-4h]

    if ( !target
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_target.cpp", 239, 0, "%s", "target") )
    {
        __debugbreak();
    }
    atGlob = AimTarget_GetGlobArray(localClientNum);
    for ( targetIndex = 0; targetIndex < atGlob->targetCount; ++targetIndex )
    {
        if ( target->entIndex == atGlob->targets[targetIndex].entIndex
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_target.cpp",
                        251,
                        0,
                        "%s",
                        "target->entIndex != atGlob->targets[targetIndex].entIndex") )
        {
            __debugbreak();
        }
    }
    low = 0;
    high = atGlob->targetCount;
    while ( low < high )
    {
        if ( AimTarget_CompareTargets(target, &atGlob->targets[(high + low) / 2]) <= 0 )
            low = (high + low) / 2 + 1;
        else
            high = (high + low) / 2;
    }
    if ( low < 64 )
    {
        if ( atGlob->targetCount == 64 )
            --atGlob->targetCount;
        memmove(
            (unsigned __int8 *)&atGlob->targets[low + 1],
            (unsigned __int8 *)&atGlob->targets[low],
            48 * (atGlob->targetCount - low));
        memcpy(&atGlob->targets[low], target, sizeof(atGlob->targets[low]));
        ++atGlob->targetCount;
    }
}

int __cdecl AimTarget_CompareTargets(const AimTarget *targetA, const AimTarget *targetB)
{
    if ( !targetA
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_target.cpp", 216, 0, "%s", "targetA") )
    {
        __debugbreak();
    }
    if ( !targetB
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_target.cpp", 217, 0, "%s", "targetB") )
    {
        __debugbreak();
    }
    if ( targetB->worldDistSqr > targetA->worldDistSqr )
        return 1;
    if ( targetA->worldDistSqr <= targetB->worldDistSqr )
        return 0;
    return -1;
}

bool __cdecl AimTarget_PlayerInValidState(const playerState_s *ps)
{
    bool result; // al

    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_target.cpp", 865, 0, "%s", "ps") )
        __debugbreak();
    if ( (ps->otherFlags & 2) != 0 )
        return 0;
    switch ( ps->pm_type )
    {
        case 2:
        case 3:
        case 4:
        case 5:
        case 9:
        case 0xA:
            result = 0;
            break;
        default:
            result = 1;
            break;
    }
    return result;
}

void __cdecl AimTarget_UpdateClientTargets(int localClientNum)
{
    int eType; // [esp+0h] [ebp-30h]
    cg_s *cgameGlob; // [esp+18h] [ebp-18h]
    const centity_s *cent; // [esp+1Ch] [ebp-14h]
    int snapEntIndex; // [esp+2Ch] [ebp-4h]

    if ( localClientNum
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_target.cpp",
                    1078,
                    0,
                    "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                    localClientNum,
                    1) )
    {
        __debugbreak();
    }
    cgameGlob = CG_GetLocalClientGlobals((int)localClientNum);
    if ( !cgameGlob->nextSnap
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_target.cpp",
                    1082,
                    0,
                    "%s",
                    "cgameGlob->nextSnap") )
    {
        __debugbreak();
    }
    AimTarget_ClearTargetList((int)localClientNum);
    //BLOPS_NULLSUB();
    for ( snapEntIndex = 0; snapEntIndex < cgameGlob->nextSnap->numEntities; ++snapEntIndex )
    {
        cent = CG_GetEntity((int)localClientNum, cgameGlob->nextSnap->entities[snapEntIndex].number);
        if ( (cent->nextState.lerp.eFlags & 0x20) == 0
            && ((*((unsigned int *)cent + 201) >> 1) & 1) != 0
            && cgameGlob->predictedPlayerState.clientNum != cent->nextState.number )
        {
            eType = cent->nextState.eType;
            if ( eType == 1 )
            {
                if ( (cent->nextState.lerp.eFlags & 0x40000) != 0 )
                    continue;
LABEL_23:
                AimTarget_ProcessEntity((int)localClientNum, cent);
                continue;
            }
            if ( eType == 6 )
            {
                if ( (cent->nextState.lerp.eFlags & 0x800) == 0 )
                    continue;
                goto LABEL_23;
            }
            if ( eType == 17 && (cent->nextState.lerp.eFlags & 0x40000) == 0 )
                goto LABEL_23;
        }
    }

    {
        PROF_SCOPED("aim assist epilog");
        //BLOPS_NULLSUB();
    }
}

void __cdecl AimTarget_ClearTargetList(int localClientNum)
{
    AimTarget_GetGlobArray(localClientNum)->targetCount = 0;
}

void __cdecl AimTarget_ProcessEntity(int localClientNum, const centity_s *ent)
{
    AimTarget_ProcessEntityInternal(localClientNum, ent);
}

void __cdecl AimTarget_GetClientTargetList(int localClientNum, AimTargetGlob **targetList, int *targetCount)
{
    AimTargetGlob *GlobArray; // eax

    if ( !targetList
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_target.cpp", 1145, 0, "%s", "targetList") )
    {
        __debugbreak();
    }
    if ( !targetCount
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_target.cpp", 1146, 0, "%s", "targetCount") )
    {
        __debugbreak();
    }
    GlobArray = AimTarget_GetGlobArray(localClientNum);
    *targetList = GlobArray;
    *targetCount = GlobArray->targetCount;
}

