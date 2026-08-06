#include "aim_assist.h"

#include <cstring>
#include <qcommon/graph.h>
#include <qcommon/cmd.h>
#include <universal/dvar.h>
#include <client/screen_placement.h>
#include <bgame/bg_local.h>
#include <bgame/bg_weapons.h>
#include <cgame/cg_weapons.h>
#include <bgame/bg_weapons_def.h>
#include <bgame/bg_weapons_ammo.h>
#include <cgame_mp/cg_local_mp.h>
#include <demo/demo_common.h>
#include <universal/com_math_anglevectors.h>
#include "aim_target.h"
#include <universal/com_math.h>
#include <clientscript/scr_const.h>
#include <xanim/dobj.h>
#include <qcommon/dobj_management.h>
#include <cgame_mp/cg_ents_mp.h>
#include <clientscript/cscr_stringlist.h>
#include <qcommon/common.h>
#include <cgame_mp/cg_main_mp.h>
#include <cmath>
#include <xanim/xanim.h>
#include <client_mp/cl_cgame_mp.h>

AimAssistGlobals aaGlobArray[1];
GraphFloat aaInputGraph[4];

const dvar_s *aim_aimAssistRangeScale;
const dvar_s *aim_autoAimRangeScale;
const dvar_s *aim_input_graph_enabled;
const dvar_s *aim_input_graph_debug;
const dvar_s *aim_input_graph_index;
const dvar_s *aim_turnrate_pitch;
const dvar_s *aim_turnrate_pitch_ads;
const dvar_s *aim_turnrate_yaw;
const dvar_s *aim_turnrate_yaw_ads;
const dvar_s *aim_accel_turnrate_enabled;
const dvar_s *aim_accel_turnrate_debug;
const dvar_s *aim_accel_turnrate_lerp;
const dvar_s *aim_slowdown_enabled;
const dvar_s *aim_slowdown_debug;
const dvar_s *aim_slowdown_region_width;
const dvar_s *aim_slowdown_region_height;
const dvar_s *aim_slowdown_region_extended_width;
const dvar_s *aim_slowdown_region_extended_height;
const dvar_s *aim_slowdown_pitch_scale;
const dvar_s *aim_slowdown_pitch_scale_ads;
const dvar_s *aim_slowdown_yaw_scale;
const dvar_s *aim_slowdown_yaw_scale_ads;
const dvar_s *aim_autoaim_lerp;
const dvar_s *aim_autoaim_enabled;
const dvar_s *aim_autoaim_debug;
const dvar_s *aim_autoaim_region_width;
const dvar_s *aim_autoaim_region_height;
const dvar_s *aim_automelee_enabled;
const dvar_s *aim_automelee_debug;
const dvar_s *aim_automelee_lerp;
const dvar_s *aim_automelee_region_width;
const dvar_s *aim_automelee_region_height;
const dvar_s *aim_automelee_range;
const dvar_s *aim_autobayonet_range;
const dvar_s *aim_view_sensitivity_override;
const dvar_s *aim_lockon_enabled;
const dvar_s *aim_lockon_debug;
const dvar_s *aim_lockon_deflection;
const dvar_s *aim_lockon_strength;
const dvar_s *aim_lockon_pitch_strength;
const dvar_s *aim_lockon_region_width;
const dvar_s *aim_lockon_region_height;
const dvar_s *aim_scale_view_axis;
const dvar_s *aim_assist_script_disable;
const dvar_s *aim_assist_min_target_distance;


void __cdecl AimAssist_Init(int localClientNum)
{
    char graphName[128]; // [esp+4h] [ebp-88h] BYREF
    int graphIndex; // [esp+88h] [ebp-4h]

    memset(&aaGlobArray[localClientNum], 0, sizeof(AimAssistGlobals));
    AimAssist_RegisterDvars();
    for ( graphIndex = 0; graphIndex < 4; ++graphIndex )
    {
        Com_sprintf(graphName, 0x80u, "aim_assist/view_input_%d.graph", graphIndex);
        GraphFloat_Load(&aaInputGraph[graphIndex], graphName, 1.0);
    }
    Cbuf_InsertText(0, "exec devgui_aimassist\n");
    for ( graphIndex = 0; graphIndex < 4; ++graphIndex )
    {
        Com_sprintf(graphName, 0x80u, "Aim Assist/Input Graph:1/Graph %d", graphIndex);
        GraphFloat_CreateDevGui(&aaInputGraph[graphIndex], graphName);
    }
}

void AimAssist_RegisterDvars()
{
    const dvar_s *result; // eax

    aim_aimAssistRangeScale = _Dvar_RegisterFloat(
                                                            "aim_aimAssistRangeScale",
                                                            1.0,
                                                            0.0,
                                                            2.0,
                                                            0x1080u,
                                                            "Scales the weapon's aim assist range");
    aim_autoAimRangeScale = _Dvar_RegisterFloat(
                                                        "aim_autoAimRangeScale",
                                                        1.0,
                                                        0.0,
                                                        2.0,
                                                        0x1080u,
                                                        "Scales the weapon's auto aim range");
    aim_input_graph_enabled = _Dvar_RegisterBool(
                                                            "aim_input_graph_enabled",
                                                            1,
                                                            0x1080u,
                                                            "Use graph for adjusting view input");
    aim_input_graph_debug = _Dvar_RegisterBool("aim_input_graph_debug", 0, 0x1080u, "Debug the view input graphs");
    aim_input_graph_index = _Dvar_RegisterInt("aim_input_graph_index", 3, 0, 3, 0x1080u, "Which input graph to use");
    aim_turnrate_pitch = _Dvar_RegisterFloat(
                                                 "aim_turnrate_pitch",
                                                 90.0,
                                                 0.0,
                                                 1080.0,
                                                 0x1080u,
                                                 "The vertical turn rate for aim assist when firing from the hip");
    aim_turnrate_pitch_ads = _Dvar_RegisterFloat(
                                                         "aim_turnrate_pitch_ads",
                                                         55.0,
                                                         0.0,
                                                         1080.0,
                                                         0x1080u,
                                                         "The turn rate up and down for aim assist when aiming down the sight");
    aim_turnrate_yaw = _Dvar_RegisterFloat(
                                             "aim_turnrate_yaw",
                                             260.0,
                                             0.0,
                                             1080.0,
                                             0x1080u,
                                             "The horizontal turn rate for aim assist when firing from the hip");
    aim_turnrate_yaw_ads = _Dvar_RegisterFloat(
                                                     "aim_turnrate_yaw_ads",
                                                     90.0,
                                                     0.0,
                                                     1080.0,
                                                     0x1080u,
                                                     "The horizontal turn rate for aim assist when aiming down the sight");
    aim_accel_turnrate_enabled = _Dvar_RegisterBool(
                                                                 "aim_accel_turnrate_enabled",
                                                                 1,
                                                                 0x1080u,
                                                                 "Enable/disable acceleration of the turnrates");
    aim_accel_turnrate_debug = _Dvar_RegisterBool(
                                                             "aim_accel_turnrate_debug",
                                                             0,
                                                             0x1080u,
                                                             "Turn on debugging info for the acceleration");
    aim_accel_turnrate_lerp = _Dvar_RegisterFloat(
                                                            "aim_accel_turnrate_lerp",
                                                            1200.0,
                                                            0.0,
                                                            4000.0,
                                                            0x1080u,
                                                            "The acceleration of the turnrates");
    aim_slowdown_enabled = _Dvar_RegisterBool(
                                                     "aim_slowdown_enabled",
                                                     1,
                                                     0x1080u,
                                                     "Slowdown the turn rate when the cross hair passes over a target");
    aim_slowdown_debug = _Dvar_RegisterBool("aim_slowdown_debug", 0, 0x1080u, "Turn on debugging info for aim slowdown");
    aim_slowdown_region_width = _Dvar_RegisterFloat(
                                                                "aim_slowdown_region_width",
                                                                90.0,
                                                                0.0,
                                                                640.0,
                                                                0x1080u,
                                                                "The screen width of the aim slowdown region");
    aim_slowdown_region_height = _Dvar_RegisterFloat(
                                                                 "aim_slowdown_region_height",
                                                                 90.0,
                                                                 0.0,
                                                                 480.0,
                                                                 0x1080u,
                                                                 "The screen height of the aim assist slowdown region");
    aim_slowdown_region_extended_width = _Dvar_RegisterFloat(
                                                                                 "aim_slowdown_region_extended_width",
                                                                                 90.0,
                                                                                 0.0,
                                                                                 640.0,
                                                                                 0x1080u,
                                                                                 "The screen width of the extended aim slowdown region");
    aim_slowdown_region_extended_height = _Dvar_RegisterFloat(
                                                                                    "aim_slowdown_region_extended_height",
                                                                                    90.0,
                                                                                    0.0,
                                                                                    480.0,
                                                                                    0x1080u,
                                                                                    "The screen height of the extended aim assist slowdown region");
    aim_slowdown_pitch_scale = _Dvar_RegisterFloat(
                                                             "aim_slowdown_pitch_scale",
                                                             0.40000001,
                                                             0.0,
                                                             1.0,
                                                             0x1080u,
                                                             "The vertical aim assist slowdown ratio from the hip");
    aim_slowdown_pitch_scale_ads = _Dvar_RegisterFloat(
                                                                     "aim_slowdown_pitch_scale_ads",
                                                                     0.5,
                                                                     0.0,
                                                                     1.0,
                                                                     0x1080u,
                                                                     "The vertical aim assist slowdown ratio when aiming down the sight");
    aim_slowdown_yaw_scale = _Dvar_RegisterFloat(
                                                         "aim_slowdown_yaw_scale",
                                                         0.40000001,
                                                         0.0,
                                                         1.0,
                                                         0x1080u,
                                                         "The horizontal aim assist slowdown ratio from the hip");
    aim_slowdown_yaw_scale_ads = _Dvar_RegisterFloat(
                                                                 "aim_slowdown_yaw_scale_ads",
                                                                 0.5,
                                                                 0.0,
                                                                 1.0,
                                                                 0x1080u,
                                                                 "The horizontal aim assist slowdown ratio when aiming down the sight");
    aim_autoaim_lerp = _Dvar_RegisterFloat(
                                             "aim_autoaim_lerp",
                                             40.0,
                                             0.0,
                                             100.0,
                                             0x1080u,
                                             "The rate in degrees per second that the auto aim will converge to its target");
    aim_autoaim_enabled = _Dvar_RegisterBool("aim_autoaim_enabled", 0, 0x1080u, "Turn on auto aim");
    aim_autoaim_debug = _Dvar_RegisterBool("aim_autoaim_debug", 0, 0x1080u, "Turn on auto aim debugging");
    aim_autoaim_region_width = _Dvar_RegisterFloat(
                                                             "aim_autoaim_region_width",
                                                             160.0,
                                                             0.0,
                                                             640.0,
                                                             0x1080u,
                                                             "The width of the auto aim region in virtual screen coordinates (0 - 640)");
    aim_autoaim_region_height = _Dvar_RegisterFloat(
                                                                "aim_autoaim_region_height",
                                                                120.0,
                                                                0.0,
                                                                480.0,
                                                                0x1080u,
                                                                "The height of the auto aim region in virtual screen coordinates (0 - 480)");
    aim_automelee_enabled = _Dvar_RegisterBool("aim_automelee_enabled", 1, 0x1080u, "Turn on auto melee");
    aim_automelee_debug = _Dvar_RegisterBool("aim_automelee_debug", 0, 0x5080u, "Turn on auto melee debugging");
    aim_automelee_lerp = _Dvar_RegisterFloat(
                                                 "aim_automelee_lerp",
                                                 40.0,
                                                 0.0,
                                                 100.0,
                                                 0x1080u,
                                                 "The rate in degrees per second that the auto melee will converge to its target");
    aim_automelee_region_width = _Dvar_RegisterFloat(
                                                                 "aim_automelee_region_width",
                                                                 320.0,
                                                                 0.0,
                                                                 640.0,
                                                                 0x1080u,
                                                                 "The width of the auto melee region in virtual screen coordinates (0 - 640)");
    aim_automelee_region_height = _Dvar_RegisterFloat(
                                                                    "aim_automelee_region_height",
                                                                    240.0,
                                                                    0.0,
                                                                    480.0,
                                                                    0x1080u,
                                                                    "The height of the auto melee region in virtual screen coordinates (0 - 480)");
    aim_automelee_range = _Dvar_RegisterFloat(
                                                    "aim_automelee_range",
                                                    128.0,
                                                    0.0,
                                                    255.0,
                                                    0x1080u,
                                                    "The range of the auto melee");
    aim_autobayonet_range = _Dvar_RegisterFloat(
                                                        "aim_autobayonet_range",
                                                        200.0,
                                                        0.0,
                                                        255.0,
                                                        0x1080u,
                                                        "The range of the auto bayonet");
    aim_view_sensitivity_override = _Dvar_RegisterFloat(
                                                                        "aim_view_sensitivity_override",
                                                                        -1.0,
                                                                        -1.0,
                                                                        5.0,
                                                                        0x1000u,
                                                                        "override value for view sensitivity");
    aim_lockon_enabled = _Dvar_RegisterBool(
                                                 "aim_lockon_enabled",
                                                 0,
                                                 0x1080u,
                                                 "Aim lock on helps the player to stay on target");
    aim_lockon_debug = _Dvar_RegisterBool("aim_lockon_debug", 0, 0x1080u, "Turn on debugging info for aim lock on");
    aim_lockon_deflection = _Dvar_RegisterFloat(
                                                        "aim_lockon_deflection",
                                                        0.05f,
                                                        0.0,
                                                        1.0,
                                                        0x1080u,
                                                        "The amount of stick deflection for the lockon to activate");
    aim_lockon_strength = _Dvar_RegisterFloat(
                                                    "aim_lockon_strength",
                                                    0.60000002,
                                                    0.0,
                                                    1.0,
                                                    0x1080u,
                                                    "The amount of aim assistance given by the target lock on (yaw)");
    aim_lockon_pitch_strength = _Dvar_RegisterFloat(
                                                                "aim_lockon_pitch_strength",
                                                                0.60000002,
                                                                0.0,
                                                                1.0,
                                                                0x1000u,
                                                                "The amount of aim assistance given by the target lock on (pitch)");
    aim_lockon_region_width = _Dvar_RegisterFloat(
                                                            "aim_lockon_region_width",
                                                            90.0,
                                                            0.0,
                                                            640.0,
                                                            0x1080u,
                                                            "The width of the auto aim region in virtual screen coordinates(0-640)");
    aim_lockon_region_height = _Dvar_RegisterFloat(
                                                             "aim_lockon_region_height",
                                                             90.0,
                                                             0.0,
                                                             480.0,
                                                             0x1080u,
                                                             "The height of the auto aim region in virtual screen coordinates(0-480)");
    aim_scale_view_axis = _Dvar_RegisterBool(
                                                    "aim_scale_view_axis",
                                                    1,
                                                    0x1080u,
                                                    "Scale the influence of each input axis so that the major axis has more influence on the control");
    aim_assist_script_disable = _Dvar_RegisterBool(
                                                                "aim_assist_script_disable",
                                                                0,
                                                                0x4000u,
                                                                "Override aim assist from script: set to true to disable code aim assist (i.e. for meatshield)");
    aim_assist_min_target_distance = _Dvar_RegisterFloat(
            "aim_assist_min_target_distance",
            10000.0,
            0.0,
            10000.0,
            0x5000u,
            "Aim assist will not work on targets beyond this distance");
}

void __cdecl AimAssist_Setup(int localClientNum, const playerState_s *ps)
{
    AimAssistGlobals *aaGlob; // [esp+0h] [ebp-4h]

    if ( localClientNum
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp",
                    501,
                    0,
                    "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                    localClientNum,
                    1) )
    {
        __debugbreak();
    }
    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 502, 0, "%s", "ps") )
        __debugbreak();
    aaGlob = &aaGlobArray[localClientNum];
    memset(aaGlob, 0, sizeof(AimAssistGlobals));
    aaGlob->initialized = 1;
    aaGlob->fovTurnRateScale = 1.0f;
    aaGlob->fovScaleInv = 1.0f;
    aaGlob->screenWidth = scrPlaceView[localClientNum].realViewportSize[0];
    aaGlob->screenHeight = scrPlaceView[localClientNum].realViewportSize[1];
    aaGlob->autoAimTargetEnt = 1023;
    aaGlob->autoMeleeTargetEnt = 1023;
    aaGlob->lockOnTargetEnt = 1023;
    aaGlob->aimSlowdownTargetEnt = 1023;
    aaGlob->autoAimActive = 0;
    aaGlob->autoAimHasRealTarget = 0;
    aaGlob->autoAimJustGotTarget = 0;
    aaGlob->aimSlowdownActive = 0;
    AimAssist_BackupPlayerState(localClientNum, ps);
}

void __cdecl AimAssist_BackupPlayerState(int localClientNum, const playerState_s *ps)
{
    unsigned int NumWeapons; // eax
    AimAssistGlobals *aaGlob; // [esp+1Ch] [ebp-4h]

    if ( localClientNum
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp",
                    585,
                    0,
                    "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                    localClientNum,
                    1) )
    {
        __debugbreak();
    }
    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 586, 0, "%s", "ps") )
        __debugbreak();
    aaGlob = &aaGlobArray[localClientNum];
    aaGlob->ps.velocity[0] = ps->velocity[0];
    aaGlob->ps.velocity[1] = ps->velocity[1];
    aaGlob->ps.velocity[2] = ps->velocity[2];
    aaGlob->ps.viewangles[0] = ps->viewangles[0];
    aaGlob->ps.viewangles[1] = ps->viewangles[1];
    aaGlob->ps.viewangles[2] = ps->viewangles[2];
    aaGlob->ps.origin[0] = ps->origin[0];
    aaGlob->ps.origin[1] = ps->origin[1];
    aaGlob->ps.origin[2] = ps->origin[2];
    aaGlob->ps.eFlags = ps->eFlags;
    aaGlob->ps.linkFlags = ps->linkFlags;
    aaGlob->ps.pm_flags = ps->pm_flags;
    aaGlob->ps.weapFlags = ps->weapFlags;
    aaGlob->ps.weaponstate = ps->weaponstate;
    aaGlob->ps.fWeaponPosFrac = ps->fWeaponPosFrac;
    aaGlob->ps.viewHeightCurrent = ps->viewHeightCurrent;
    if ( (ps->eFlags & 0x4300) != 0 )
    {
        if ( ps->viewlocked_entNum == 1023
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp",
                        605,
                        0,
                        "%s",
                        "ps->viewlocked_entNum != ENTITYNUM_NONE") )
        {
            __debugbreak();
        }
        if ( (unsigned int)ps->viewlocked_entNum >= 0x400
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp",
                        606,
                        0,
                        "%s\n\t(ps->viewlocked_entNum) = %i",
                        "((ps->viewlocked_entNum >= 0) && (ps->viewlocked_entNum < (1<<10)))",
                        ps->viewlocked_entNum) )
        {
            __debugbreak();
        }
        aaGlob->ps.weapIndex = CG_GetEntity(localClientNum, ps->viewlocked_entNum)->nextState.weapon;
    }
    else
    {
        aaGlob->ps.weapIndex = BG_GetViewmodelWeaponIndex(ps);
    }
    aaGlob->ps.meleeWeapIndex = CG_GetPlayerMeleeWeapon(ps);
    if ( aaGlob->ps.weapIndex >= BG_GetNumWeapons() )
    {
        NumWeapons = BG_GetNumWeapons();
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp",
                        616,
                        0,
                        "aaGlob->ps.weapIndex doesn't index BG_GetNumWeapons()\n\t%i not in [0, %i)",
                        aaGlob->ps.weapIndex,
                        NumWeapons) )
            __debugbreak();
    }
    aaGlob->ps.hasAmmo = BG_WeaponAmmo(ps, aaGlob->ps.weapIndex) > 0 || ps->throwBackGrenadeTimeLeft > 0;
    aaGlob->ps.isDualWielding = BG_GetWeaponDef(aaGlob->ps.weapIndex)->bDualWield;
    aaGlob->ps.targetAssistDisabled = AimAssist_PlayerDisabledAutoAim();
}

centity_s *__cdecl CG_GetEntity(int localClientNum, int entityIndex)
{
    if ( localClientNum
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\../cgame_mp/cg_local_mp.h",
                    1894,
                    0,
                    "%s\n\t(localClientNum) = %i",
                    "(localClientNum == 0)",
                    localClientNum) )
    {
        __debugbreak();
    }
    if ( (unsigned int)entityIndex >= 0x600
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\../cgame_mp/cg_local_mp.h",
                    1898,
                    0,
                    "entityIndex doesn't index MAX_LOCAL_CENTITIES\n\t%i not in [0, %i)",
                    entityIndex,
                    1536) )
    {
        __debugbreak();
    }
    if ( entityIndex < 1024 )
        return &cg_entitiesArray[localClientNum][entityIndex];
    else
        return &cg_fakeEntitiesArray[512 * localClientNum - 1024 + entityIndex].cent;
}

char __cdecl AimAssist_PlayerDisabledAutoAim()
{
    Demo_IsPlaying();
    return 1;
}

void __cdecl AimAssist_UpdateScreenTargets(
                int localClientNum,
                const float *viewOrg,
                const float *viewAngles,
                float tanHalfFovX,
                float tanHalfFovY)
{
    float *v5; // [esp+2Ch] [ebp-B4h]
    float *viewOrigin; // [esp+30h] [ebp-B0h]
    AimScreenTarget screenTarget; // [esp+34h] [ebp-ACh] BYREF
    int targetCount; // [esp+68h] [ebp-78h] BYREF
    const AimTarget *target; // [esp+6Ch] [ebp-74h]
    const AimTarget *targetList; // [esp+70h] [ebp-70h] BYREF
    centity_s *cent; // [esp+74h] [ebp-6Ch]
    float entMtx[4][3]; // [esp+78h] [ebp-68h] BYREF
    int targetIndex; // [esp+A8h] [ebp-38h]
    float bounds[2][3]; // [esp+ACh] [ebp-34h] BYREF
    float clipBounds[2][3]; // [esp+C4h] [ebp-1Ch] BYREF
    AimAssistGlobals *aaGlob; // [esp+DCh] [ebp-4h]
    int savedregs; // [esp+E0h] [ebp+0h] BYREF

    aaGlob = &aaGlobArray[localClientNum];
    if ( aaGlob->initialized )
    {
        viewOrigin = aaGlob->viewOrigin;
        aaGlob->viewOrigin[0] = *viewOrg;
        viewOrigin[1] = viewOrg[1];
        viewOrigin[2] = viewOrg[2];
        v5 = aaGlob->viewAngles;
        aaGlob->viewAngles[0] = *viewAngles;
        v5[1] = viewAngles[1];
        v5[2] = viewAngles[2];
        AnglesToAxis(viewAngles, aaGlob->viewAxis);
        AimAssist_FovScale(aaGlob, tanHalfFovY);
        AimAssist_CreateScreenMatrix(aaGlob, tanHalfFovX, tanHalfFovY);
        aaGlob->screenTargetCount = 0;
        AimTarget_GetClientTargetList(localClientNum, (AimTargetGlob **)&targetList, &targetCount);
        for ( targetIndex = 0; targetIndex < targetCount; ++targetIndex )
        {
            target = &targetList[targetIndex];
            cent = CG_GetEntity(localClientNum, target->entIndex);
            if ( ((*((unsigned int *)cent + 201) >> 1) & 1) != 0
                && target->worldDistSqr <= (float)(aim_assist_min_target_distance->current.value
                                                                                 * aim_assist_min_target_distance->current.value) )
            {
                *(_QWORD *)&bounds[0][0] = *(_QWORD *)target->mins;
                bounds[0][2] = target->mins[2];
                bounds[1][0] = target->maxs[0];
                bounds[1][1] = target->maxs[1];
                bounds[1][2] = target->maxs[2];
                AnglesToAxis(cent->pose.angles, entMtx);
                entMtx[3][0] = cent->pose.origin[0];
                entMtx[3][1] = cent->pose.origin[1];
                entMtx[3][2] = cent->pose.origin[2];
                if ( AimAssist_ConvertToClipBounds(aaGlob, bounds, entMtx, clipBounds) )
                {
                    screenTarget.entIndex = target->entIndex;
                    *(_QWORD *)screenTarget.clipMins = *(_QWORD *)&clipBounds[0][0];
                    *(_QWORD *)screenTarget.clipMaxs = *(_QWORD *)&clipBounds[1][0];
                    screenTarget.velocity[0] = target->velocity[0];
                    screenTarget.velocity[1] = target->velocity[1];
                    screenTarget.velocity[2] = target->velocity[2];
                    if ( AimAssist_CalcAimPos(localClientNum, cent, target, screenTarget.aimPos) )
                    {
                        screenTarget.distSqr = target->worldDistSqr;
                        screenTarget.crosshairDistSqr = AimAssist_GetCrosshairDistSqr(clipBounds[0], clipBounds[1]);
                        AimAssist_AddToTargetList(aaGlob, &screenTarget);
                    }
                }
            }
        }
    }
}

void __cdecl AimAssist_FovScale(AimAssistGlobals *aaGlob, float tanHalfFovY)
{
        float v2; // [esp+8h] [ebp-Ch]
        float v3; // [esp+Ch] [ebp-8h]
        float tanHalfBaseFovY; // [esp+10h] [ebp-4h]

        iassert(aaGlob);
        aaGlob->fovTurnRateScale = tanHalfFovY / (float)0.47780272;
        v3 = cg_fov->current.value * 0.01745329238474369 * 0.5;
        v2 = tan(v3);
        tanHalfBaseFovY = v2 * 0.75;
        iassert(tanHalfBaseFovY != 0.0f);
        aaGlob->fovScaleInv = tanHalfBaseFovY / tanHalfFovY;
}

void    AimAssist_CreateScreenMatrix(
                AimAssistGlobals *aaGlob,
                float tanHalfFovX,
                float tanHalfFovY)
{
        float viewMtx[4][4]; // [esp+Ch] [ebp-C0h] BYREF
        float projMtx[4][4]; // [esp+4Ch] [ebp-80h] BYREF
        float screenMtx[4][4]; // [esp+8Ch] [ebp-40h] BYREF

        iassert(aaGlob);

        //MatrixForViewer(viewMtx, aaGlob->viewOrigin, aaGlob->viewAxis);
        MatrixForViewer(aaGlob->viewOrigin, aaGlob->viewAxis, viewMtx);
        //InfinitePerspectiveMatrix(projMtx, tanHalfFovX, tanHalfFovY, 1.0);
        InfinitePerspectiveMatrix(tanHalfFovX, tanHalfFovY, 1.0f, projMtx);
        MatrixMultiply44(viewMtx, projMtx, screenMtx);
        MatrixTranspose44(screenMtx, aaGlob->screenMtx);
        MatrixInverse44(aaGlob->screenMtx, aaGlob->invScreenMtx);
}

char __cdecl AimAssist_ConvertToClipBounds(
                const AimAssistGlobals *aaGlob,
                const float (*bounds)[3],
                const float (*mtx)[3],
                float (*clipBounds)[3])
{
    float v5; // [esp+0h] [ebp-70h]
    float v6; // [esp+4h] [ebp-6Ch]
    float v7; // [esp+8h] [ebp-68h]
    float v8; // [esp+Ch] [ebp-64h]
    float v9; // [esp+10h] [ebp-60h]
    float v10; // [esp+14h] [ebp-5Ch]
    float v11; // [esp+18h] [ebp-58h]
    float v12; // [esp+1Ch] [ebp-54h]
    float v13; // [esp+20h] [ebp-50h]
    float v14; // [esp+24h] [ebp-4Ch]
    float v15; // [esp+28h] [ebp-48h]
    float v16; // [esp+2Ch] [ebp-44h]
    float v17; // [esp+30h] [ebp-40h]
    float v18; // [esp+34h] [ebp-3Ch]
    float v19; // [esp+38h] [ebp-38h]
    float v20; // [esp+3Ch] [ebp-34h]
    float v21; // [esp+44h] [ebp-2Ch]
    float worldCorner[3]; // [esp+48h] [ebp-28h] BYREF
    float clipCorner[3]; // [esp+54h] [ebp-1Ch] BYREF
    int ptIndex; // [esp+60h] [ebp-10h]
    float worldCornerRotated[3]; // [esp+64h] [ebp-Ch] BYREF

    if ( !bounds
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 722, 0, "%s", "bounds") )
    {
        __debugbreak();
    }
    if ( !mtx && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 723, 0, "%s", "mtx") )
        __debugbreak();
    if ( !clipBounds
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 724, 0, "%s", "clipBounds") )
    {
        __debugbreak();
    }
    ClearBounds((float *)clipBounds, &(*clipBounds)[3]);
    for ( ptIndex = 0; ptIndex < 8; ++ptIndex )
    {
        worldCorner[0] = (*bounds)[3 * (ptIndex & 1)];
        worldCorner[1] = (*bounds)[3 * ((ptIndex >> 1) & 1) + 1];
        worldCorner[2] = (*bounds)[3 * ((ptIndex >> 2) & 1) + 2];
        MatrixTransformVector43(worldCorner, mtx, worldCornerRotated);
        if ( AimAssist_XfmWorldPointToClipSpace(aaGlob, worldCornerRotated, clipCorner) )
            AddPointToBounds(clipCorner, (float *)clipBounds, &(*clipBounds)[3]);
    }
    if ( (*clipBounds)[0] >= (*clipBounds)[3]
        || (*clipBounds)[1] >= (*clipBounds)[4]
        || (*clipBounds)[2] >= (*clipBounds)[5] )
    {
        return 0;
    }
    if ( (*clipBounds)[0] > 1.0 || (*clipBounds)[1] > 1.0 || (*clipBounds)[2] > 1.0 )
        return 0;
    if ( (*clipBounds)[3] < -1.0 || (*clipBounds)[4] < -1.0 || (*clipBounds)[5] < 0.0 )
        return 0;
    if ( (float)((*clipBounds)[0] - 1.0) < 0.0 )
        v21 = (*clipBounds)[0];
    else
        v21 = 1.0f;
    if ( (float)(-1.0 - (*clipBounds)[0]) < 0.0 )
        v10 = v21;
    else
        v10 = -1.0f;
    (*clipBounds)[0] = v10;
    v19 = (*clipBounds)[1];
    if ( (float)(v19 - 1.0) < 0.0 )
        v20 = (*clipBounds)[1];
    else
        v20 = 1.0f;
    if ( (float)(-1.0 - v19) < 0.0 )
        v9 = v20;
    else
        v9 = -1.0f;
    (*clipBounds)[1] = v9;
    v17 = (*clipBounds)[2];
    if ( (float)(v17 - 1.0) < 0.0 )
        v18 = (*clipBounds)[2];
    else
        v18 = 1.0f;
    if ( (float)(0.0 - v17) < 0.0 )
        v8 = v18;
    else
        v8 = 0.0f;
    (*clipBounds)[2] = v8;
    v15 = (*clipBounds)[3];
    if ( (float)(v15 - 1.0) < 0.0 )
        v16 = (*clipBounds)[3];
    else
        v16 = 1.0f;
    if ( (float)(-1.0 - v15) < 0.0 )
        v7 = v16;
    else
        v7 = -1.0f;
    (*clipBounds)[3] = v7;
    v13 = (*clipBounds)[4];
    if ( (float)(v13 - 1.0) < 0.0 )
        v14 = (*clipBounds)[4];
    else
        v14 = 1.0f;
    if ( (float)(-1.0 - v13) < 0.0 )
        v6 = v14;
    else
        v6 = -1.0f;
    (*clipBounds)[4] = v6;
    v11 = (*clipBounds)[5];
    if ( (float)(v11 - 1.0) < 0.0 )
        v12 = (*clipBounds)[5];
    else
        v12 = 1.0f;
    if ( (float)(0.0 - v11) < 0.0 )
        v5 = v12;
    else
        v5 = 0.0f;
    (*clipBounds)[5] = v5;
    return 1;
}

char __cdecl AimAssist_XfmWorldPointToClipSpace(const AimAssistGlobals *aaGlob, float *in, float *out)
{
    float clip_4; // [esp+14h] [ebp-1Ch]
    float clip_8; // [esp+18h] [ebp-18h]
    float clip_12; // [esp+1Ch] [ebp-14h]
    float xyzw; // [esp+20h] [ebp-10h]
    float xyzw_4; // [esp+24h] [ebp-Ch]
    float xyzw_8; // [esp+28h] [ebp-8h]

    if ( !aaGlob
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 689, 0, "%s", "aaGlob") )
    {
        __debugbreak();
    }
    if ( !in && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 690, 0, "%s", "in") )
        __debugbreak();
    if ( !out && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 691, 0, "%s", "out") )
        __debugbreak();
    xyzw = *in;
    xyzw_4 = in[1];
    xyzw_8 = in[2];
    clip_12 = (float)((float)((float)(aaGlob->screenMtx[3][0] * *in) + (float)(aaGlob->screenMtx[3][1] * xyzw_4))
                                    + (float)(aaGlob->screenMtx[3][2] * xyzw_8))
                    + (float)(aaGlob->screenMtx[3][3] * 1.0);
    if ( clip_12 <= 0.0 )
        return 0;
    clip_4 = (float)((float)((float)(aaGlob->screenMtx[1][0] * xyzw) + (float)(aaGlob->screenMtx[1][1] * xyzw_4))
                                 + (float)(aaGlob->screenMtx[1][2] * xyzw_8))
                 + (float)(aaGlob->screenMtx[1][3] * 1.0);
    clip_8 = (float)((float)((float)(aaGlob->screenMtx[2][0] * xyzw) + (float)(aaGlob->screenMtx[2][1] * xyzw_4))
                                 + (float)(aaGlob->screenMtx[2][2] * xyzw_8))
                 + (float)(aaGlob->screenMtx[2][3] * 1.0);
    *out = (float)((float)((float)((float)(aaGlob->screenMtx[0][0] * xyzw) + (float)(aaGlob->screenMtx[0][1] * xyzw_4))
                                             + (float)(aaGlob->screenMtx[0][2] * xyzw_8))
                             + (float)(aaGlob->screenMtx[0][3] * 1.0))
             / clip_12;
    out[1] = (float)(clip_4 / clip_12) * -1.0;
    out[2] = clip_8 / clip_12;
    return 1;
}

double __cdecl AimAssist_GetCrosshairDistSqr(const float *clipMins, const float *clipMaxs)
{
    float center_4; // [esp+8h] [ebp-4h]

    if ( !clipMins
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 766, 0, "%s", "clipMins") )
    {
        __debugbreak();
    }
    if ( !clipMaxs
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 767, 0, "%s", "clipMaxs") )
    {
        __debugbreak();
    }
    center_4 = 0.5 * (float)(clipMins[1] + clipMaxs[1]);
    return (float)((float)((float)(0.5 * (float)(*clipMins + *clipMaxs)) * (float)(0.5 * (float)(*clipMins + *clipMaxs)))
                             + (float)(center_4 * center_4));
}

void __cdecl AimAssist_AddToTargetList(AimAssistGlobals *aaGlob, const AimScreenTarget *screenTarget)
{
    int low; // [esp+8h] [ebp-Ch]
    int high; // [esp+10h] [ebp-4h]

    if ( !aaGlob
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 883, 0, "%s", "aaGlob") )
    {
        __debugbreak();
    }
    if ( !screenTarget
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 884, 0, "%s", "screenTarget") )
    {
        __debugbreak();
    }
    low = 0;
    high = aaGlob->screenTargetCount;
    while ( low < high )
    {
        if ( AimAssist_CompareTargets(screenTarget, &aaGlob->screenTargets[(high + low) / 2]) <= 0 )
            low = (high + low) / 2 + 1;
        else
            high = (high + low) / 2;
    }
    if ( low < 64 )
    {
        if ( aaGlob->screenTargetCount == 64 )
            --aaGlob->screenTargetCount;
        memmove(
            (unsigned __int8 *)&aaGlob->screenTargets[low + 1],
            (unsigned __int8 *)&aaGlob->screenTargets[low],
            52 * (aaGlob->screenTargetCount - low));
        memcpy(&aaGlob->screenTargets[low], screenTarget, sizeof(aaGlob->screenTargets[low]));
        ++aaGlob->screenTargetCount;
    }
}

int __cdecl AimAssist_CompareTargets(const AimScreenTarget *screenTargetA, const AimScreenTarget *screenTargetB)
{
    if ( !screenTargetA
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 860, 0, "%s", "screenTargetA") )
    {
        __debugbreak();
    }
    if ( !screenTargetB
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 861, 0, "%s", "screenTargetB") )
    {
        __debugbreak();
    }
    if ( screenTargetB->crosshairDistSqr > screenTargetA->crosshairDistSqr )
        return 1;
    if ( screenTargetA->crosshairDistSqr <= screenTargetB->crosshairDistSqr )
        return 0;
    return -1;
}

int __cdecl AimAssist_CalcAimPos(
                int localClientNum,
                const centity_s *targetEnt,
                const AimTarget *target,
                float *aimPos)
{
    float center_4; // [esp+10h] [ebp-8h]
    float center_8; // [esp+14h] [ebp-4h]

    if ( !targetEnt
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 941, 0, "%s", "targetEnt") )
    {
        __debugbreak();
    }
    if ( !aimPos
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 942, 0, "%s", "aimPos") )
    {
        __debugbreak();
    }
    if ( targetEnt->nextState.eType == 1 )
        return AimTarget_GetTagPos(localClientNum, targetEnt, scr_const.aim_bone, aimPos);
    if ( !target )
        return 0;
    center_4 = (float)(target->mins[1] + target->maxs[1]) * 0.5;
    center_8 = (float)(target->mins[2] + target->maxs[2]) * 0.5;
    *aimPos = targetEnt->pose.origin[0] + (float)((float)(target->mins[0] + target->maxs[0]) * 0.5);
    aimPos[1] = targetEnt->pose.origin[1] + center_4;
    aimPos[2] = targetEnt->pose.origin[2] + center_8;
    return 1;
}

int __cdecl AimTarget_GetTagPos(int localClientNum, const centity_s *cent, unsigned int tagName, float *pos)
{
    char *v5; // eax
    DObj *dobj; // [esp+0h] [ebp-4h]

    if ( !cent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 920, 0, "%s", "cent") )
        __debugbreak();
    if ( !pos && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 921, 0, "%s", "pos") )
        __debugbreak();
    dobj = Com_GetClientDObj(cent->nextState.number, localClientNum);
    if ( !dobj )
        return 0;
    if ( !CG_DObjGetWorldTagPos(&cent->pose, dobj, tagName, pos) )
    {
        v5 = SL_ConvertToString(tagName, SCRIPTINSTANCE_SERVER);
        Com_Error(ERR_DROP, "AimTarget_GetTagPos: Cannot find tag [%s] on entity\n", v5);
    }
    return 1;
}

int __cdecl AimAssist_GetScreenTargetCount(int localClientNum)
{
    return aaGlobArray[localClientNum].screenTargetCount;
}

int __cdecl AimAssist_GetScreenTargetEntity(int localClientNum, unsigned int targetIndex)
{
    const AimAssistGlobals *aaGlob; // [esp+0h] [ebp-4h]

    aaGlob = &aaGlobArray[localClientNum];
    if ( targetIndex >= aaGlob->screenTargetCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp",
                    1067,
                    0,
                    "targetIndex doesn't index aaGlob->screenTargetCount\n\t%i not in [0, %i)",
                    targetIndex,
                    aaGlob->screenTargetCount) )
    {
        __debugbreak();
    }
    return aaGlob->screenTargets[targetIndex].entIndex;
}

void __cdecl AimAssist_ClearEntityReference(int localClientNum, int entIndex)
{
    AimAssistGlobals *aaGlob; // [esp+0h] [ebp-4h]

    aaGlob = &aaGlobArray[localClientNum];
    if ( aaGlob->autoAimTargetEnt == entIndex )
        aaGlob->autoAimTargetEnt = 1023;
    if ( aaGlob->autoMeleeTargetEnt == entIndex )
        aaGlob->autoMeleeTargetEnt = 1023;
    if ( aaGlob->lockOnTargetEnt == entIndex )
        aaGlob->lockOnTargetEnt = 1023;
    if ( aaGlob->aimSlowdownTargetEnt == entIndex )
        aaGlob->aimSlowdownTargetEnt = 1023;
}

bool __cdecl AimAssist_IsPrevTargetEntity(int localClientNum, int entIndex)
{
    AimAssistGlobals *aaGlob; // [esp+0h] [ebp-4h]

    aaGlob = &aaGlobArray[localClientNum];
    if ( aaGlob->aimSlowdownTargetEnt == entIndex )
        return 1;
    if ( aaGlob->autoMeleeTargetEnt == entIndex )
        return 1;
    if ( aaGlob->lockOnTargetEnt == entIndex )
        return 1;
    return aaGlob->autoAimTargetEnt == entIndex;
}

void __cdecl AimAssist_UpdateGamePadInput(const AimInput *input, AimOutput *output)
{
    int i; // [esp+4h] [ebp-8h]
    AimAssistGlobals *aaGlob; // [esp+8h] [ebp-4h]

    if ( !input
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 2330, 0, "%s", "input") )
    {
        __debugbreak();
    }
    if ( !output
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 2331, 0, "%s", "output") )
    {
        __debugbreak();
    }
    output->pitch = input->pitch;
    output->yaw = input->yaw;
    aaGlob = &aaGlobArray[input->localClientNum];
    if ( aaGlob->initialized )
    {
        AimAssist_UpdateTweakables(input->localClientNum);
        AimAssist_UpdateAdsLerp(input);
        AimAssist_ApplyTurnRates(input, output);
        if ( aaGlob->ps.weaponstate < 47 || aaGlob->ps.weaponstate > 49 )
        {
            AimAssist_ApplyAutoAim(input, output);
            AimAssist_ApplyAutoMelee(input, output);
            AimAssist_ApplyLockOn(input, output);
        }
        for ( i = 0; i < 2; ++i )
            aaGlob->prev_button_bits.array[i] = input->button_bits.array[i];
    }
}

void __cdecl AimAssist_UpdateTweakables(int localClientNum)
{
        float v1; // [esp+4h] [ebp-5Ch]
        float v2; // [esp+4h] [ebp-5Ch]
        float v3; // [esp+Ch] [ebp-54h]
        float v4; // [esp+Ch] [ebp-54h]
        float v5; // [esp+14h] [ebp-4Ch]
        float v6; // [esp+14h] [ebp-4Ch]
        float v7; // [esp+1Ch] [ebp-44h]
        float v8; // [esp+1Ch] [ebp-44h]
        float v9; // [esp+24h] [ebp-3Ch]
        float v10; // [esp+24h] [ebp-3Ch]
        float v11; // [esp+2Ch] [ebp-34h]
        float v12; // [esp+2Ch] [ebp-34h]
        float overrideAutoaimWidthValue; // [esp+34h] [ebp-2Ch]
        float v14; // [esp+34h] [ebp-2Ch]
        float v15; // [esp+3Ch] [ebp-24h]
        float v16; // [esp+3Ch] [ebp-24h]
        float v17; // [esp+44h] [ebp-1Ch]
        float v18; // [esp+44h] [ebp-1Ch]
        float v19; // [esp+4Ch] [ebp-14h]
        float v20; // [esp+4Ch] [ebp-14h]
        float value; // [esp+54h] [ebp-Ch]
        float v22; // [esp+54h] [ebp-Ch]
        AimAssistGlobals *aaGlob; // [esp+58h] [ebp-8h]
        AimTweakables *tweaks; // [esp+5Ch] [ebp-4h]

        if (localClientNum
                && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp",
                        311,
                        0,
                        "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                        localClientNum,
                        1))
        {
                __debugbreak();
        }
        aaGlob = &aaGlobArray[localClientNum];
        tweaks = &aaGlob->tweakables;
        if (aaGlob->aimSlowdownActive)
        {
                value = aim_slowdown_region_extended_width->current.value;
                v22 = ScrPlace_HiResGetScale() * value;
                tweaks->slowdownRegionWidth = (float)(v22 * scrPlaceView[localClientNum].scaleVirtualToReal[0])
                        / aaGlobArray[localClientNum].screenWidth;
                v19 = aim_slowdown_region_extended_height->current.value;
                v20 = ScrPlace_HiResGetScale() * v19;
                aaGlobArray[localClientNum].tweakables.slowdownRegionHeight = (float)(v20
                        * scrPlaceView[localClientNum].scaleVirtualToReal[1])
                        / aaGlobArray[localClientNum].screenHeight;
        }
        else
        {
                v17 = aim_slowdown_region_width->current.value;
                v18 = ScrPlace_HiResGetScale() * v17;
                tweaks->slowdownRegionWidth = (float)(v18 * scrPlaceView[localClientNum].scaleVirtualToReal[0])
                        / aaGlobArray[localClientNum].screenWidth;
                v15 = aim_slowdown_region_height->current.value;
                v16 = ScrPlace_HiResGetScale() * v15;
                aaGlobArray[localClientNum].tweakables.slowdownRegionHeight = (float)(v16
                        * scrPlaceView[localClientNum].scaleVirtualToReal[1])
                        / aaGlobArray[localClientNum].screenHeight;
        }
        if (aaGlob->overrideSnapWidthAndLerp)
        {
                overrideAutoaimWidthValue = aaGlob->overrideAutoaimWidthValue;
                v14 = ScrPlace_HiResGetScale() * overrideAutoaimWidthValue;
                aaGlobArray[localClientNum].tweakables.autoAimRegionWidth = (float)((float)(v14
                        * scrPlaceView[localClientNum].scaleVirtualToReal[0])
                        / aaGlobArray[localClientNum].screenWidth)
                        * aaGlob->fovScaleInv;
        }
        else
        {
                v11 = aim_autoaim_region_width->current.value;
                v12 = ScrPlace_HiResGetScale() * v11;
                aaGlobArray[localClientNum].tweakables.autoAimRegionWidth = (float)((float)(v12
                        * scrPlaceView[localClientNum].scaleVirtualToReal[0])
                        / aaGlobArray[localClientNum].screenWidth)
                        * aaGlob->fovScaleInv;
        }
        v9 = aim_autoaim_region_height->current.value;
        v10 = ScrPlace_HiResGetScale() * v9;
        aaGlobArray[localClientNum].tweakables.autoAimRegionHeight = (float)((float)(v10
                * scrPlaceView[localClientNum].scaleVirtualToReal[1])
                / aaGlobArray[localClientNum].screenHeight)
                * aaGlob->fovScaleInv;
        v7 = aim_automelee_region_width->current.value;
        v8 = ScrPlace_HiResGetScale() * v7;
        aaGlobArray[localClientNum].tweakables.autoMeleeRegionWidth = (float)((float)(v8
                * scrPlaceView[localClientNum].scaleVirtualToReal[0])
                / aaGlobArray[localClientNum].screenWidth)
                * aaGlob->fovScaleInv;
        v5 = aim_automelee_region_height->current.value;
        v6 = ScrPlace_HiResGetScale() * v5;
        aaGlobArray[localClientNum].tweakables.autoMeleeRegionHeight = (float)((float)(v6
                * scrPlaceView[localClientNum].scaleVirtualToReal[1])
                / aaGlobArray[localClientNum].screenHeight)
                * aaGlob->fovScaleInv;
        v3 = aim_lockon_region_width->current.value;
        v4 = ScrPlace_HiResGetScale() * v3;
        aaGlobArray[localClientNum].tweakables.lockOnRegionWidth = (float)(v4
                * scrPlaceView[localClientNum].scaleVirtualToReal[0])
                / aaGlobArray[localClientNum].screenWidth;
        v1 = aim_lockon_region_height->current.value;
        v2 = ScrPlace_HiResGetScale() * v1;
        aaGlobArray[localClientNum].tweakables.lockOnRegionHeight = (float)(v2
                * scrPlaceView[localClientNum].scaleVirtualToReal[1])
                / aaGlobArray[localClientNum].screenHeight;
}

void __cdecl AimAssist_UpdateAdsLerp(const AimInput *input)
{
    AimAssistGlobals *aaGlob; // [esp+0h] [ebp-4h]

    if ( !input
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 347, 0, "%s", "input") )
    {
        __debugbreak();
    }
    aaGlob = &aaGlobArray[input->localClientNum];
    if ( Demo_IsPlaying() )
    {
        aaGlob->adsLerp = 0.0f;
    }
    else
    {
        aaGlob->adsLerp = aaGlob->ps.fWeaponPosFrac;
        if ( (aaGlob->ps.eFlags & 0x300) != 0 )
        {
            if ( input->button_bits.testBit(11) )
                aaGlob->adsLerp = 1.0f;
        }
    }
}

void __cdecl AimAssist_ApplyTurnRates(const AimInput *input, AimOutput *output)
{
    double v2; // st7
    double v3; // st7
    float v4; // [esp+10h] [ebp-44h]
    float v5; // [esp+14h] [ebp-40h]
    float adjustedYawAxis; // [esp+20h] [ebp-34h] BYREF
    float pitchTurnRate; // [esp+24h] [ebp-30h]
    float adjustedPitchAxis; // [esp+28h] [ebp-2Ch] BYREF
    float yawDelta; // [esp+2Ch] [ebp-28h]
    float pitchSign; // [esp+30h] [ebp-24h]
    float slowdownYawScale; // [esp+34h] [ebp-20h] BYREF
    float sensitivity; // [esp+38h] [ebp-1Ch]
    float yawTurnRate; // [esp+3Ch] [ebp-18h]
    float pitchDelta; // [esp+40h] [ebp-14h]
    float slowdownPitchScale; // [esp+44h] [ebp-10h] BYREF
    AimAssistGlobals *aaGlob; // [esp+48h] [ebp-Ch]
    float yawSign; // [esp+4Ch] [ebp-8h]
    float accel; // [esp+50h] [ebp-4h]

    slowdownPitchScale = 0.0f;
    slowdownYawScale = 0.0f;
    if ( !input
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 1429, 0, "%s", "input") )
    {
        __debugbreak();
    }
    if ( !output
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 1430, 0, "%s", "output") )
    {
        __debugbreak();
    }
    aaGlob = &aaGlobArray[input->localClientNum];
    if ( aaGlob->autoMeleeState == AMS_TARGETING )
    {
        adjustedPitchAxis = 0.0f;
        adjustedYawAxis = 0.0f;
        slowdownPitchScale = 1.0f;
        slowdownYawScale = 1.0f;
    }
    else
    {
        AimAssist_CalcAdjustedAxis(input, &adjustedPitchAxis, &adjustedYawAxis);
        AimAssist_CalcSlowdown(input, &slowdownPitchScale, &slowdownYawScale);
    }
    sensitivity = AimAssist_GetProfileSensitivity();
    pitchTurnRate = AimAssist_LerpDvars(aim_turnrate_pitch, aim_turnrate_pitch_ads, aaGlob->adsLerp);
    pitchTurnRate = (float)((float)(slowdownPitchScale * aaGlob->fovTurnRateScale) * sensitivity) * pitchTurnRate;
    yawTurnRate = AimAssist_LerpDvars(aim_turnrate_yaw, aim_turnrate_yaw_ads, aaGlob->adsLerp);
    yawTurnRate = (float)((float)(slowdownYawScale * aaGlob->fovTurnRateScale) * sensitivity) * yawTurnRate;
    if ( input->pitchMax != 0.0 && pitchTurnRate > input->pitchMax )
        pitchTurnRate = input->pitchMax;
    if ( input->yawMax != 0.0 && yawTurnRate > input->yawMax )
        yawTurnRate = input->yawMax;
    if ( adjustedPitchAxis >= 0.0 )
        v5 = 1.0f;
    else
        v5 = -1.0f;
    pitchSign = v5;
    if ( adjustedYawAxis >= 0.0 )
        v4 = 1.0f;
    else
        v4 = -1.0f;
    yawSign = v4;
    //pitchDelta = fabs(adjustedPitchAxis) * pitchTurnRate;
    pitchDelta = fabs(adjustedPitchAxis) * pitchTurnRate;
    //yawDelta = fabs(adjustedYawAxis) * yawTurnRate;
    yawDelta = fabs(adjustedYawAxis) * yawTurnRate;
    if ( !aim_accel_turnrate_enabled->current.enabled || aim_assist_script_disable->current.enabled )
    {
        aaGlob->pitchDelta = pitchDelta;
        aaGlob->yawDelta = yawDelta;
    }
    else
    {
        accel = aim_accel_turnrate_lerp->current.value * sensitivity;
        if ( pitchDelta <= aaGlob->pitchDelta )
        {
            aaGlob->pitchDelta = pitchDelta;
        }
        else
        {
            v2 = LinearTrack(pitchDelta, aaGlob->pitchDelta, accel, input->deltaTime);
            aaGlob->pitchDelta = v2;
        }
        if ( yawDelta <= aaGlob->yawDelta )
        {
            aaGlob->yawDelta = yawDelta;
        }
        else
        {
            v3 = LinearTrack(yawDelta, aaGlob->yawDelta, accel, input->deltaTime);
            aaGlob->yawDelta = v3;
        }
        if ( aim_accel_turnrate_debug->current.enabled )
        {
            Com_Printf(17, "Target Pit = %f, Current Pit = %f\n", pitchDelta, aaGlob->pitchDelta);
            Com_Printf(17, "Target Yaw = %f, Current Yaw = %f\n", yawDelta, aaGlob->yawDelta);
        }
    }
    output->pitch = (float)((float)(aaGlob->pitchDelta * input->deltaTime) * pitchSign) + output->pitch;
    output->yaw = (float)((float)(aaGlob->yawDelta * input->deltaTime) * yawSign) + output->yaw;
}

double __cdecl AimAssist_LerpDvars(const dvar_s *from, const dvar_s *to, float frac)
{
    if ( !from && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 277, 0, "%s", "from") )
        __debugbreak();
    if ( !to && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 278, 0, "%s", "to") )
        __debugbreak();
    if ( (frac < 0.0 || frac > 1.0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp",
                    279,
                    0,
                    "%s",
                    "(frac >= 0.0f) && (frac <= 1.0f)") )
    {
        __debugbreak();
    }
    return (to->current.value - from->current.value) * frac + from->current.value;
}

void __cdecl AimAssist_CalcAdjustedAxis(const AimInput *input, float *pitchAxis, float *yawAxis)
{
    float fraction; // [esp+10h] [ebp-28h]
    float v4; // [esp+1Ch] [ebp-1Ch]
    float deflection; // [esp+28h] [ebp-10h]
    float absPitchAxis; // [esp+2Ch] [ebp-Ch]
    float graphScale; // [esp+30h] [ebp-8h]
    float absYawAxis; // [esp+34h] [ebp-4h]

    if ( !input
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 1206, 0, "%s", "input") )
    {
        __debugbreak();
    }
    if ( !pitchAxis
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 1207, 0, "%s", "pitchAxis") )
    {
        __debugbreak();
    }
    if ( !yawAxis
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 1208, 0, "%s", "yawAxis") )
    {
        __debugbreak();
    }
    if ( aim_input_graph_enabled->current.enabled )
    {
        deflection = sqrtf((float)(input->pitchAxis * input->pitchAxis) + (float)(input->yawAxis * input->yawAxis));
        if ( (float)(deflection - 1.0) < 0.0 )
            v4 = deflection;
        else
            v4 = 1.0f;
        if ( (float)(0.0 - deflection) < 0.0 )
            fraction = v4;
        else
            fraction = 0.0f;
        if ( aim_input_graph_index->current.integer >= 4u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp",
                        1215,
                        0,
                        "aim_input_graph_index->current.integer doesn't index AIM_INPUT_GRAPH_COUNT\n\t%i not in [0, %i)",
                        aim_input_graph_index->current.integer,
                        4) )
        {
            __debugbreak();
        }
        graphScale = GraphFloat_GetValue(&aaInputGraph[aim_input_graph_index->current.integer], fraction);
        *pitchAxis = input->pitchAxis * graphScale;
        *yawAxis = input->yawAxis * graphScale;
        if ( aim_input_graph_debug->current.enabled )
        {
            Com_Printf(17, "Input Pit = %f, Output Pit = %f\n", input->pitchAxis, *pitchAxis);
            Com_Printf(17, "Input Yaw = %f, Output Yaw = %f\n", input->yawAxis, *yawAxis);
        }
    }
    else
    {
        *pitchAxis = input->pitchAxis;
        *yawAxis = input->yawAxis;
    }
    if ( aim_scale_view_axis->current.enabled )
    {
        //LODWORD(absPitchAxis) = *(unsigned int *)pitchAxis & _mask__AbsFloat_;
        absPitchAxis = fabs(*pitchAxis);
        //LODWORD(absYawAxis) = *(unsigned int *)yawAxis & _mask__AbsFloat_;
        absYawAxis = fabs(*yawAxis);
        if ( absPitchAxis > 1.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp",
                        1239,
                        0,
                        "%s",
                        "absPitchAxis <= 1.0f") )
        {
            __debugbreak();
        }
        if ( absYawAxis > 1.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp",
                        1240,
                        0,
                        "%s",
                        "absYawAxis <= 1.0f") )
        {
            __debugbreak();
        }
        if ( absPitchAxis <= absYawAxis )
            *pitchAxis = (float)(1.0 - (float)(absYawAxis - absPitchAxis)) * *pitchAxis;
        else
            *yawAxis = (float)(1.0 - (float)(absPitchAxis - absYawAxis)) * *yawAxis;
    }
}

void __cdecl AimAssist_CalcSlowdown(const AimInput *input, float *pitchScale, float *yawScale)
{
    const AimScreenTarget *screenTarget; // [esp+Ch] [ebp-10h]
    float aimAssistRange; // [esp+10h] [ebp-Ch]
    AimAssistGlobals *aaGlob; // [esp+14h] [ebp-8h]

    if ( !input
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 1334, 0, "%s", "input") )
    {
        __debugbreak();
    }
    if ( !pitchScale
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 1335, 0, "%s", "pitchScale") )
    {
        __debugbreak();
    }
    if ( !yawScale
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 1336, 0, "%s", "yawScale") )
    {
        __debugbreak();
    }
    aaGlob = &aaGlobArray[input->localClientNum];
    *pitchScale = 1.0f;
    *yawScale = 1.0f;
    aaGlob->aimSlowdownActive = 0;
    aaGlob->aimSlowdownTargetEnt = 1023;
    if ( AimAssist_IsSlowdownActive(&aaGlob->ps) && (aaGlob->ps.weaponstate < 47 || aaGlob->ps.weaponstate > 49) )
    {
        if ( !aaGlob->ps.weapIndex
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp",
                        1350,
                        0,
                        "%s",
                        "aaGlob->ps.weapIndex != WP_NONE") )
        {
            __debugbreak();
        }
        aimAssistRange = AimAssist_GetAimAssistRange(aaGlob->ps.weapIndex, aaGlob->adsLerp);
        screenTarget = AimAssist_GetBestTarget(
                                         aaGlob,
                                         aimAssistRange,
                                         aaGlob->tweakables.slowdownRegionWidth,
                                         aaGlob->tweakables.slowdownRegionHeight);
        if ( screenTarget )
        {
            *pitchScale = AimAssist_LerpDvars(aim_slowdown_pitch_scale, aim_slowdown_pitch_scale_ads, aaGlob->adsLerp);
            *yawScale = AimAssist_LerpDvars(aim_slowdown_yaw_scale, aim_slowdown_yaw_scale_ads, aaGlob->adsLerp);
            aaGlob->aimSlowdownActive = 1;
            aaGlob->aimSlowdownTargetEnt = screenTarget->entIndex;
        }
        if ( AimAssist_IsPlayerUsingOffhand(&aaGlob->ps) )
            *pitchScale = 1.0f;
        if ( aim_slowdown_debug->current.enabled )
        {
            Com_Printf(17, "Target slowdown pitch scale = %f\n", *pitchScale);
            Com_Printf(17, "Target slowdown yaw scale = %f\n", *yawScale);
        }
    }
}

double __cdecl AimAssist_GetAimAssistRange(unsigned int weapIndex, float adsLerp)
{
    const WeaponVariantDef *weapVariantDef; // [esp+1Ch] [ebp-8h]

    if ( !weapIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp",
                    827,
                    0,
                    "%s",
                    "weapIndex != WP_NONE") )
    {
        __debugbreak();
    }
    if ( (adsLerp < 0.0 || adsLerp > 1.0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp",
                    828,
                    0,
                    "adsLerp not in [0.0f, 1.0f]\n\t%g not in [%g, %g]",
                    adsLerp,
                    0.0,
                    1.0) )
    {
        __debugbreak();
    }
    weapVariantDef = BG_GetWeaponVariantDef(weapIndex);
    return (float)((float)((float)((float)(1.0 - adsLerp) * weapVariantDef->weapDef->aimAssistRange)
                                             + (float)(weapVariantDef->fAimAssistRangeAds * adsLerp))
                             * aim_aimAssistRangeScale->current.value);
}

static bool __cdecl AimAssist_DoBoundsIntersectCenterBox(
    const float *clipMins,
    const float *clipMaxs,
    float clipHalfWidth,
    float clipHalfHeight)
{
    iassert(clipMins);
    iassert(clipMaxs);

    return (clipHalfWidth >= (double)*clipMins && *clipMaxs >= -clipHalfWidth)
        && (clipHalfHeight >= (double)clipMins[1] && clipMaxs[1] >= -clipHalfHeight);
}

const AimScreenTarget *__cdecl AimAssist_GetBestTarget(
                const AimAssistGlobals *aaGlob,
                float range,
                float regionWidth,
                float regionHeight)
{
    int targetIndex; // [esp+Ch] [ebp-4h]

    if ( !aaGlob
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 1153, 0, "%s", "aaGlob") )
    {
        __debugbreak();
    }
    if ( range < 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp",
                    1154,
                    0,
                    "%s\n\t(range) = %g",
                    "((range >= 0.0f))",
                    range) )
    {
        __debugbreak();
    }
    for ( targetIndex = 0; targetIndex < aaGlob->screenTargetCount; ++targetIndex )
    {
        if ( aaGlob->screenTargets[targetIndex].distSqr <= (float)(range * range)
            && AimAssist_DoBoundsIntersectCenterBox(
                     aaGlob->screenTargets[targetIndex].clipMins,
                     aaGlob->screenTargets[targetIndex].clipMaxs,
                     regionWidth,
                     regionHeight) )
        {
            return &aaGlob->screenTargets[targetIndex];
        }
    }
    return 0;
}

bool __cdecl AimAssist_IsPlayerUsingOffhand(const AimAssistPlayerState *ps)
{
    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 1253, 0, "%s", "ps") )
        __debugbreak();
    if ( (ps->weapFlags & 2) == 0 )
        return 0;
    if ( ps->weapIndex )
        return BG_GetWeaponDef(ps->weapIndex)->impactType != IMPACT_TYPE_BLADE;
    return 0;
}

bool __cdecl AimAssist_IsSlowdownActive(const AimAssistPlayerState *ps)
{
    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 1276, 0, "%s", "ps") )
        __debugbreak();
    if ( aim_assist_script_disable->current.enabled )
        return 0;
    if ( !aim_slowdown_enabled->current.enabled )
        return 0;
    if ( ps->targetAssistDisabled )
        return 0;
    if ( !ps->weapIndex )
        return 0;
    if ( BG_GetWeaponDef(ps->weapIndex)->requireLockonToFire )
        return 0;
    if ( BG_GetWeaponDef(ps->weapIndex)->guidedMissileType == MISSILE_GUIDANCE_TVGUIDED )
        return 0;
    if ( (ps->pm_flags & 0x10000) != 0 )
        return 0;
    if ( (ps->eFlags & 0x300) != 0 )
        return 1;
    return ps->hasAmmo;
}

double __cdecl AimAssist_GetProfileSensitivity()
{
    float sensitivity; // [esp+0h] [ebp-4h]

    if ( aim_view_sensitivity_override->current.value > 0.0 )
        sensitivity = aim_view_sensitivity_override->current.value;
    else
        sensitivity = Dvar_GetFloat("input_viewSensitivity");
    if ( sensitivity < 0.001 )
        return 1.0f;
    return sensitivity;
}

void __cdecl AimAssist_ApplyAutoAim(const AimInput *input, AimOutput *output)
{
    const AimScreenTarget *screenTarget; // [esp+20h] [ebp-28h]
    float yawDelta; // [esp+24h] [ebp-24h]
    float newPitch; // [esp+28h] [ebp-20h]
    bool autoAimEnabled; // [esp+2Fh] [ebp-19h]
    float pitchDelta; // [esp+30h] [ebp-18h]
    AimAssistGlobals *aaGlob; // [esp+34h] [ebp-14h]
    float newYaw; // [esp+38h] [ebp-10h]
    float autoAimRange; // [esp+44h] [ebp-4h]

    if ( !input
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 1646, 0, "%s", "input") )
    {
        __debugbreak();
    }
    if ( !output
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 1647, 0, "%s", "output") )
    {
        __debugbreak();
    }
    aaGlob = &aaGlobArray[input->localClientNum];
    autoAimEnabled = AimAssist_PlayerDisabledAutoAim() == 0;
    BG_GetWeaponDef(aaGlob->ps.weapIndex);
    if ( aim_autoaim_enabled->current.enabled
        && !aim_assist_script_disable->current.enabled
        && input->button_bits.testBit(11)
        && autoAimEnabled
        && aaGlob->ps.weapIndex )
    {
        if ( !aaGlob->autoAimPressed && input->button_bits.testBit(11) )
        {
            if ( aaGlob->ps.weaponstate != 10
                && aaGlob->ps.weaponstate != 12
                && aaGlob->ps.weaponstate != 14
                && aaGlob->ps.weaponstate != 13
                && aaGlob->ps.weaponstate != 11
                && aaGlob->ps.weaponstate != 15
                && aaGlob->ps.weaponstate != 16 )
            {
                if ( BG_IsAimDownSightWeapon(aaGlob->ps.weapIndex) )
                {
                    autoAimRange = AimAssist_GetAutoAimRange(aaGlob->ps.weapIndex);
                    screenTarget = AimAssist_GetBestTarget(
                                                     aaGlob,
                                                     autoAimRange,
                                                     aaGlob->tweakables.autoAimRegionWidth,
                                                     aaGlob->tweakables.autoAimRegionHeight);
                    if ( screenTarget )
                    {
                        AimAssist_SetAutoAimTarget(aaGlob, screenTarget);
                        aaGlob->autoAimJustGotTarget = 1;
                    }
                }
            }
            aaGlob->autoAimPressed = 1;
        }
        if ( aaGlob->autoAimActive )
        {
            if ( (aaGlob->ps.eFlags & 0x300) == 0
                && aaGlob->adsLerp < 1.0
                && !AimAssist_UpdateAutoAimTarget(aaGlob)
                && !aaGlob->autoAimJustGotTarget )
            {
                goto LABEL_37;
            }
            newPitch = DiffTrackAngle(
                                     aaGlob->autoAimPitchTarget,
                                     aaGlob->autoAimPitch,
                                     aim_autoaim_lerp->current.value,
                                     input->deltaTime);
            newYaw = aaGlob->overrideSnapWidthAndLerp
                         ? DiffTrackAngle(
                                 aaGlob->autoAimYawTarget,
                                 aaGlob->autoAimYaw,
                                 aaGlob->overrideAutoaimLerpValue,
                                 input->deltaTime)
                         : DiffTrackAngle(
                                 aaGlob->autoAimYawTarget,
                                 aaGlob->autoAimYaw,
                                 aim_autoaim_lerp->current.value,
                                 input->deltaTime);
            pitchDelta = AngleNormalize180(newPitch - aaGlob->autoAimPitch);
            yawDelta = AngleNormalize180(newYaw - aaGlob->autoAimYaw);
            aaGlob->autoAimPitch = newPitch;
            aaGlob->autoAimYaw = newYaw;
            output->pitch = output->pitch + pitchDelta;
            output->yaw = output->yaw + yawDelta;
            //if ( fabs(pitchDelta) < 0.001
            //    && fabs(yawDelta) <= 0.001
            //    && aaGlob->ps.fWeaponPosFrac == 0.0 )
            if ( fabs(pitchDelta) < 0.001f
                && fabs(yawDelta) <= 0.001f
                && aaGlob->ps.fWeaponPosFrac == 0.0 )
            {
LABEL_37:
                AimAssist_ClearAutoAimTarget(aaGlob);
            }
        }
    }
    else
    {
        AimAssist_ClearAutoAimTarget(aaGlob);
        aaGlob->autoAimPressed = 0;
    }
}

double __cdecl AimAssist_GetAutoAimRange(unsigned int weapIndex)
{
    if ( !weapIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp",
                    845,
                    0,
                    "%s",
                    "weapIndex != WP_NONE") )
    {
        __debugbreak();
    }
    return (float)(BG_GetWeaponDef(weapIndex)->autoAimRange * aim_autoAimRangeScale->current.value);
}

void __cdecl AimAssist_ClearAutoAimTarget(AimAssistGlobals *aaGlob)
{
    if ( !aaGlob
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 1531, 0, "%s", "aaGlob") )
    {
        __debugbreak();
    }
    aaGlob->autoAimTargetEnt = 1023;
    aaGlob->autoAimActive = 0;
    aaGlob->autoAimPitch = 0.0f;
    aaGlob->autoAimPitchTarget = 0.0f;
    aaGlob->autoAimYaw = 0.0f;
    aaGlob->autoAimYawTarget = 0.0f;
    aaGlob->autoAimJustGotTarget = 0;
    aaGlob->autoAimHasRealTarget = 0;
}

char __cdecl AimAssist_UpdateAutoAimTarget(AimAssistGlobals *aaGlob)
{
    float targetDir[3]; // [esp+8h] [ebp-1Ch] BYREF
    const AimScreenTarget *screenTarget; // [esp+14h] [ebp-10h]
    float targetAngles[3]; // [esp+18h] [ebp-Ch] BYREF

    if ( !aaGlob
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 1556, 0, "%s", "aaGlob") )
    {
        __debugbreak();
    }
    if ( !aaGlob->autoAimActive
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp",
                    1557,
                    0,
                    "%s",
                    "aaGlob->autoAimActive") )
    {
        __debugbreak();
    }
    screenTarget = AimAssist_GetTargetFromEntity(aaGlob, aaGlob->autoAimTargetEnt);
    if ( !screenTarget )
        return 0;
    targetDir[0] = screenTarget->aimPos[0] - aaGlob->viewOrigin[0];
    targetDir[1] = screenTarget->aimPos[1] - aaGlob->viewOrigin[1];
    targetDir[2] = screenTarget->aimPos[2] - aaGlob->viewOrigin[2];
    vectoangles(targetDir, targetAngles);
    if ( aaGlob->overrideSnapWidthAndLerp )
    {
        aaGlob->autoAimPitch = aaGlob->viewAngles[0];
        aaGlob->autoAimYaw = aaGlob->viewAngles[1];
    }
    aaGlob->autoAimPitchTarget = targetAngles[0];
    aaGlob->autoAimYawTarget = targetAngles[1];
    return 1;
}

const AimScreenTarget *__cdecl AimAssist_GetTargetFromEntity(const AimAssistGlobals *aaGlob, int entIndex)
{
    int targetIndex; // [esp+4h] [ebp-4h]

    if ( !aaGlob
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 1128, 0, "%s", "aaGlob") )
    {
        __debugbreak();
    }
    if ( entIndex == 1023 )
        return 0;
    for ( targetIndex = 0; targetIndex < aaGlob->screenTargetCount; ++targetIndex )
    {
        if ( aaGlob->screenTargets[targetIndex].entIndex == entIndex )
            return &aaGlob->screenTargets[targetIndex];
    }
    return 0;
}

void __cdecl AimAssist_SetAutoAimTarget(AimAssistGlobals *aaGlob, const AimScreenTarget *screenTarget)
{
    if ( !aaGlob
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 1584, 0, "%s", "aaGlob") )
    {
        __debugbreak();
    }
    if ( !screenTarget
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 1585, 0, "%s", "screenTarget") )
    {
        __debugbreak();
    }
    AimAssist_ClearAutoAimTarget(aaGlob);
    aaGlob->autoAimTargetEnt = screenTarget->entIndex;
    aaGlob->autoAimActive = 1;
    aaGlob->autoAimHasRealTarget = 1;
    aaGlob->autoAimPitch = aaGlob->viewAngles[0];
    aaGlob->autoAimYaw = aaGlob->viewAngles[1];
    AimAssist_UpdateAutoAimTarget(aaGlob);
}

void __cdecl AimAssist_ApplyLockOn(const AimInput *input, AimOutput *output)
{
    float arcLength; // [esp+4Ch] [ebp-30h]
    const AimScreenTarget *screenTarget; // [esp+50h] [ebp-2Ch]
    float pitchTurnRate; // [esp+54h] [ebp-28h]
    int prevTargetEnt; // [esp+58h] [ebp-24h]
    float aimAssistRange; // [esp+64h] [ebp-18h]
    AimAssistGlobals *aaGlob; // [esp+70h] [ebp-Ch]

    if ( !input
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 1739, 0, "%s", "input") )
    {
        __debugbreak();
    }
    if ( !output
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 1740, 0, "%s", "output") )
    {
        __debugbreak();
    }
    aaGlob = &aaGlobArray[input->localClientNum];
    prevTargetEnt = aaGlob->lockOnTargetEnt;
    aaGlob->lockOnTargetEnt = 1023;
    if ( aim_lockon_enabled->current.enabled
        && !AimAssist_IsPlayerUsingOffhand(&aaGlob->ps)
        && !aim_assist_script_disable->current.enabled
        && !aaGlob->ps.targetAssistDisabled
        && !aaGlob->autoAimActive
        && aaGlob->autoMeleeState != AMS_TARGETING
        && BG_GetWeaponDef(aaGlob->ps.weapIndex)->guidedMissileType != MISSILE_GUIDANCE_TVGUIDED
        && !BG_GetWeaponDef(aaGlob->ps.weapIndex)->requireLockonToFire
        && (aim_lockon_deflection->current.value <= fabs(input->pitchAxis)
         || aim_lockon_deflection->current.value <= fabs(input->yawAxis)
         || aim_lockon_deflection->current.value <= fabs(input->rightAxis))
        && !CG_GetLocalClientGlobals(input->localClientNum)->cameraMode )
    {
        if ( aaGlob->ps.weapIndex )
        {
            aimAssistRange = AimAssist_GetAimAssistRange(aaGlob->ps.weapIndex, aaGlob->adsLerp);
            screenTarget = AimAssist_GetPrevOrBestTarget(
                                             aaGlob,
                                             aimAssistRange,
                                             aaGlob->tweakables.lockOnRegionWidth,
                                             aaGlob->tweakables.lockOnRegionHeight,
                                             prevTargetEnt);
        }
        else
        {
            screenTarget = 0;
        }
        if ( screenTarget && screenTarget->distSqr > 0.0 )
        {
            aaGlob->lockOnTargetEnt = screenTarget->entIndex;
            arcLength = sqrtf(screenTarget->distSqr) * 3.1415927;
            if ( arcLength <= 0.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp",
                            1789,
                            0,
                            "%s",
                            "arcLength > 0.0f") )
            {
                __debugbreak();
            }
            pitchTurnRate = (float)((float)((float)((float)((float)((float)(screenTarget->velocity[0] * aaGlob->viewAxis[2][0])
                                                                                                                        + (float)(screenTarget->velocity[1] * aaGlob->viewAxis[2][1]))
                                                                                                        + (float)(screenTarget->velocity[2] * aaGlob->viewAxis[2][2]))
                                                                                        - (float)((float)((float)(aaGlob->ps.velocity[0] * aaGlob->viewAxis[2][0])
                                                                                                                        + (float)(aaGlob->ps.velocity[1] * aaGlob->viewAxis[2][1]))
                                                                                                        + (float)(aaGlob->ps.velocity[2] * aaGlob->viewAxis[2][2])))
                                                                        / arcLength)
                                                        * 180.0)
                                        * aim_lockon_pitch_strength->current.value;
            output->yaw = (float)((float)((float)((float)((float)((float)((float)((float)(screenTarget->velocity[0]
                                                                                                                                                                    * aaGlob->viewAxis[1][0])
                                                                                                                                                    + (float)(screenTarget->velocity[1]
                                                                                                                                                                    * aaGlob->viewAxis[1][1]))
                                                                                                                                    + (float)(screenTarget->velocity[2]
                                                                                                                                                    * aaGlob->viewAxis[1][2]))
                                                                                                                    - (float)((float)((float)(aaGlob->ps.velocity[0]
                                                                                                                                                                    * aaGlob->viewAxis[1][0])
                                                                                                                                                    + (float)(aaGlob->ps.velocity[1]
                                                                                                                                                                    * aaGlob->viewAxis[1][1]))
                                                                                                                                    + (float)(aaGlob->ps.velocity[2]
                                                                                                                                                    * aaGlob->viewAxis[1][2])))
                                                                                                    / arcLength)
                                                                                    * 180.0)
                                                                    * aim_lockon_strength->current.value)
                                                    * input->deltaTime)
                                    + output->yaw;
            output->pitch = output->pitch - (float)(pitchTurnRate * input->deltaTime);
        }
    }
}

const AimScreenTarget *__cdecl AimAssist_GetPrevOrBestTarget(
                const AimAssistGlobals *aaGlob,
                float range,
                float regionWidth,
                float regionHeight,
                int prevTargetEnt)
{
    const AimScreenTarget *screenTarget; // [esp+Ch] [ebp-4h]

    if ( !aaGlob
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 1181, 0, "%s", "aaGlob") )
    {
        __debugbreak();
    }
    if ( range < 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp",
                    1182,
                    0,
                    "%s\n\t(range) = %g",
                    "((range >= 0.0f))",
                    range) )
    {
        __debugbreak();
    }
    screenTarget = AimAssist_GetTargetFromEntity(aaGlob, prevTargetEnt);
    if ( screenTarget
        && (float)(range * range) > screenTarget->distSqr
        && AimAssist_DoBoundsIntersectCenterBox(screenTarget->clipMins, screenTarget->clipMaxs, regionWidth, regionHeight) )
    {
        return screenTarget;
    }
    else
    {
        return AimAssist_GetBestTarget(aaGlob, range, regionWidth, regionHeight);
    }
}

static const float dist_offset = 32.0f;
void __cdecl AimAssist_ApplyAutoMelee(const AimInput *input, AimOutput *output)
{
    int integer; // [esp+10h] [ebp-5Ch]
    float targetDir[2]; // [esp+38h] [ebp-34h] BYREF
    float dist; // [esp+40h] [ebp-2Ch]
    const AimScreenTarget *screenTarget; // [esp+44h] [ebp-28h]
    float range; // [esp+48h] [ebp-24h]
    bool meleeing; // [esp+4Fh] [ebp-1Dh]
    float yawDelta; // [esp+50h] [ebp-1Ch]
    float newPitch; // [esp+54h] [ebp-18h]
    float pitchDelta; // [esp+58h] [ebp-14h]
    bool isBayonet; // [esp+5Fh] [ebp-Dh]
    AimAssistGlobals *aaGlob; // [esp+60h] [ebp-Ch]
    float newYaw; // [esp+64h] [ebp-8h]
    const AimTweakables *tweaks; // [esp+68h] [ebp-4h]

    if ( !input
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 1982, 0, "%s", "input") )
    {
        __debugbreak();
    }
    if ( !output
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 1983, 0, "%s", "output") )
    {
        __debugbreak();
    }
    output->meleeChargeYaw = 0.0f;
    output->meleeChargeDist = 0;
    aaGlob = &aaGlobArray[input->localClientNum];
    tweaks = &aaGlob->tweakables;
    isBayonet = BG_IsBayonetWeapon(aaGlob->ps.weapIndex);
    if ( !aim_automelee_enabled->current.enabled || !aaGlob->ps.weapIndex )
        goto LABEL_24;
    //v3 = bitarray<51>::testBit(&input->button_bits, 2u) && !bitarray<51>::testBit(&aaGlob->prev_button_bits, 2u);
    meleeing = input->button_bits.testBit(2) && !aaGlob->prev_button_bits.testBit(2);
    if ( BG_GetWeaponDef(aaGlob->ps.weapIndex)->meleeChargeRange == 0.0 )
    {
        if ( isBayonet )
            integer = aim_autobayonet_range->current.integer;
        else
            integer = aim_automelee_range->current.integer;
        LODWORD(range) = integer;
    }
    else
    {
        range = BG_GetWeaponDef(aaGlob->ps.weapIndex)->meleeChargeRange;
    }
    if ( aaGlob->autoMeleeState != AMS_TARGETING && meleeing )
    {
        screenTarget = AimAssist_GetTargetFromEntity(aaGlob, aaGlob->autoMeleeTargetEnt);
        if ( !screenTarget )
        {
            screenTarget = AimAssist_GetBestTarget(aaGlob, range, tweaks->autoMeleeRegionWidth, tweaks->autoMeleeRegionHeight);
            if ( !screenTarget )
            {
LABEL_24:
                AimAssist_ClearAutoMeleeTarget(aaGlob);
                return;
            }
        }
        AimAssist_SetAutoMeleeTarget(aaGlob, screenTarget);
        if ( (aaGlob->ps.pm_flags & 1) == 0 && screenTarget->distSqr > 0.0 )
        {
            dist = sqrtf(screenTarget->distSqr) - dist_offset;
            if ( dist > 0.0 )
            {
                if ( dist > 255.0 )
                    dist = 255.0f;
                targetDir[0] = screenTarget->aimPos[0] - aaGlob->viewOrigin[0];
                targetDir[1] = screenTarget->aimPos[1] - aaGlob->viewOrigin[1];
                output->meleeChargeYaw = vectoyaw(targetDir);
                output->meleeChargeDist = (int)dist;
            }
        }
    }
    if ( aaGlob->autoMeleeState == AMS_TARGET_AQUIRED && aaGlob->ps.weaponstate == 17 )
        aaGlob->autoMeleeState = AMS_TARGETING;
    if ( aaGlob->autoMeleeState == AMS_TARGETING )
    {
        if ( aaGlob->ps.weaponstate != 18
            && (aaGlob->ps.weaponstate == 17 || aaGlob->ps.weaponstate == 18 || aaGlob->ps.weaponstate == 19)
            && AimAssist_UpdateAutoMeleeTarget(aaGlob, input->localClientNum) )
        {
            newPitch = DiffTrackAngle(
                                     aaGlob->autoMeleePitchTarget,
                                     aaGlob->autoMeleePitch,
                                     aim_automelee_lerp->current.value,
                                     input->deltaTime);
            newYaw = DiffTrackAngle(
                                 aaGlob->autoMeleeYawTarget,
                                 aaGlob->autoMeleeYaw,
                                 aim_automelee_lerp->current.value,
                                 input->deltaTime);
            pitchDelta = AngleNormalize180(newPitch - aaGlob->autoMeleePitch);
            yawDelta = AngleNormalize180(newYaw - aaGlob->autoMeleeYaw);
            aaGlob->autoMeleePitch = newPitch;
            aaGlob->autoMeleeYaw = newYaw;
            output->pitch = output->pitch + pitchDelta;
            output->yaw = output->yaw + yawDelta;
        }
        else
        {
            AimAssist_ClearAutoMeleeTarget(aaGlob);
        }
    }
}

void __cdecl AimAssist_ClearAutoMeleeTarget(AimAssistGlobals *aaGlob)
{
    if ( !aaGlob
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 1813, 0, "%s", "aaGlob") )
    {
        __debugbreak();
    }
    aaGlob->autoMeleeState = AMS_NOT_ACTIVE;
    aaGlob->autoMeleeTargetEnt = 1023;
    aaGlob->autoMeleePitch = 0.0f;
    aaGlob->autoMeleePitchTarget = 0.0f;
    aaGlob->autoMeleeYaw = 0.0f;
    aaGlob->autoMeleeYawTarget = 0.0f;
}

char __cdecl AimAssist_UpdateAutoMeleeTarget(AimAssistGlobals *aaGlob, int localClientNum)
{
    centity_s *cent; // [esp+10h] [ebp-2Ch]
    float targetDir[3]; // [esp+14h] [ebp-28h] BYREF
    const AimScreenTarget *screenTarget; // [esp+20h] [ebp-1Ch]
    float aimPos[3]; // [esp+24h] [ebp-18h] BYREF
    float targetAngles[3]; // [esp+30h] [ebp-Ch] BYREF

    if ( !aaGlob
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 1846, 0, "%s", "aaGlob") )
    {
        __debugbreak();
    }
    if ( aaGlob->autoMeleeState == AMS_NOT_ACTIVE
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp",
                    1847,
                    0,
                    "%s",
                    "aaGlob->autoMeleeState != AMS_NOT_ACTIVE") )
    {
        __debugbreak();
    }
    screenTarget = AimAssist_GetTargetFromEntity(aaGlob, aaGlob->autoMeleeTargetEnt);
    if ( !screenTarget && aaGlob->autoMeleeTargetEnt == 1023 )
        return 0;
    if ( screenTarget )
    {
        aimPos[0] = screenTarget->aimPos[0];
        aimPos[1] = screenTarget->aimPos[1];
        aimPos[2] = screenTarget->aimPos[2];
    }
    else
    {
        cent = CG_GetEntity(localClientNum, aaGlob->autoMeleeTargetEnt);
        if ( ((*((unsigned int *)cent + 201) >> 1) & 1) == 0 )
            return 0;
        if ( !AimAssist_CalcAimPos(localClientNum, cent, 0, aimPos) )
            return 0;
    }
    targetDir[0] = aimPos[0] - aaGlob->viewOrigin[0];
    targetDir[1] = aimPos[1] - aaGlob->viewOrigin[1];
    targetDir[2] = aimPos[2] - aaGlob->viewOrigin[2];
    vectoangles(targetDir, targetAngles);
    aaGlob->autoMeleePitchTarget = targetAngles[0];
    aaGlob->autoMeleeYawTarget = targetAngles[1];
    return 1;
}

void __cdecl AimAssist_SetAutoMeleeTarget(AimAssistGlobals *aaGlob, const AimScreenTarget *screenTarget)
{
    if ( !aaGlob
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 1884, 0, "%s", "aaGlob") )
    {
        __debugbreak();
    }
    if ( !screenTarget
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 1885, 0, "%s", "screenTarget") )
    {
        __debugbreak();
    }
    aaGlob->autoMeleeTargetEnt = screenTarget->entIndex;
    aaGlob->autoMeleeState = AMS_TARGET_AQUIRED;
    aaGlob->autoMeleePitch = aaGlob->viewAngles[0];
    aaGlob->autoMeleePitchTarget = 0.0f;
    aaGlob->autoMeleeYaw = aaGlob->viewAngles[1];
    aaGlob->autoMeleeYawTarget = 0.0f;
}

void __cdecl AimAssist_UpdateMouseInput(const AimInput *input, AimOutput *output)
{
    int i; // [esp+4h] [ebp-8h]
    AimAssistGlobals *aaGlob; // [esp+8h] [ebp-4h]

    if ( !input
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 2378, 0, "%s", "input") )
    {
        __debugbreak();
    }
    if ( !output
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 2379, 0, "%s", "output") )
    {
        __debugbreak();
    }
    output->pitch = input->pitch;
    output->yaw = input->yaw;
    aaGlob = &aaGlobArray[input->localClientNum];
    if ( aaGlob->initialized )
    {
        AimAssist_UpdateTweakables(input->localClientNum);
        AimAssist_UpdateAdsLerp(input);
        AimAssist_ApplyAutoMelee(input, output);
        for ( i = 0; i < 2; ++i )
            aaGlob->prev_button_bits.array[i] = input->button_bits.array[i];
    }
}

void __cdecl AimAssist_DrawDebugOverlay(int localClientNum)
{
    float green[4]; // [esp+10h] [ebp-2Ch] BYREF
    float red[4]; // [esp+20h] [ebp-1Ch] BYREF
    const AimAssistGlobals *aaGlob; // [esp+30h] [ebp-Ch]
    const playerState_s *ps; // [esp+34h] [ebp-8h]
    const AimTweakables *tweaks; // [esp+38h] [ebp-4h]

    red[0] = 1.0f;
    red[1] = 0.0f;
    red[2] = 0.0f;
    red[3] = 0.25f;
    green[0] = 0.0f;
    green[1] = 1.0f;
    green[2] = 0.0f;
    green[3] = 0.25f;
    aaGlob = &aaGlobArray[localClientNum];
    if ( aaGlob->initialized )
    {
        tweaks = &aaGlob->tweakables;
        ps = &CG_GetLocalClientGlobals(localClientNum)->predictedPlayerState;
        if ( aim_slowdown_debug->current.enabled )
        {
            AimAssist_DrawTargets(localClientNum, ps, red);
            AimAssist_DrawCenterBox(aaGlob, tweaks->slowdownRegionWidth, tweaks->slowdownRegionHeight, green);
        }
        if ( aim_autoaim_debug->current.enabled )
        {
            AimAssist_DrawTargets(localClientNum, ps, red);
            AimAssist_DrawCenterBox(aaGlob, tweaks->autoAimRegionWidth, tweaks->autoAimRegionHeight, green);
        }
        if ( aim_automelee_debug->current.enabled )
        {
            AimAssist_DrawTargets(localClientNum, ps, red);
            AimAssist_DrawCenterBox(aaGlob, tweaks->autoMeleeRegionWidth, tweaks->autoMeleeRegionHeight, green);
        }
        if ( aim_lockon_debug->current.enabled )
        {
            AimAssist_DrawTargets(localClientNum, ps, red);
            AimAssist_DrawCenterBox(aaGlob, tweaks->lockOnRegionWidth, tweaks->lockOnRegionHeight, green);
        }
    }
}

void __cdecl AimAssist_DrawCenterBox(
                const AimAssistGlobals *aaGlob,
                float clipHalfWidth,
                float clipHalfHeight,
                const float *color)
{
    if ( !aaGlob
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 2420, 0, "%s", "aaGlob") )
    {
        __debugbreak();
    }
    if ( !color
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 2421, 0, "%s", "color") )
    {
        __debugbreak();
    }
    CL_DrawStretchPicPhysical(
        (float)((float)(aaGlob->screenWidth - (float)(clipHalfWidth * aaGlob->screenWidth)) * 0.5) + 0.0,
        (float)((float)(aaGlob->screenHeight - (float)(clipHalfHeight * aaGlob->screenHeight)) * 0.5) + 0.0,
        clipHalfWidth * aaGlob->screenWidth,
        clipHalfHeight * aaGlob->screenHeight,
        0.0,
        0.0,
        1.0,
        1.0,
        color,
        cgMedia.whiteMaterial);
}

void __cdecl AimAssist_DrawTargets(int localClientNum, const playerState_s *ps, const float *color)
{
    char *v3; // eax
    char *v4; // eax
    char *v5; // eax
    double v6; // st7
    char *v7; // eax
    float value; // [esp+28h] [ebp-4Ch]
    float aimAssistRange; // [esp+4Ch] [ebp-28h]
    char *msg; // [esp+50h] [ebp-24h]
    int targetIndex; // [esp+54h] [ebp-20h]
    const AimAssistGlobals *aaGlob; // [esp+5Ch] [ebp-18h]
    float x; // [esp+60h] [ebp-14h]
    float y; // [esp+68h] [ebp-Ch]
    float ya; // [esp+68h] [ebp-Ch]
    float yb; // [esp+68h] [ebp-Ch]
    float yc; // [esp+68h] [ebp-Ch]
    float autoAimRange; // [esp+70h] [ebp-4h]

    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 2458, 0, "%s", "ps") )
        __debugbreak();
    if ( !color
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\aim_assist\\aim_assist.cpp", 2459, 0, "%s", "color") )
    {
        __debugbreak();
    }
    aaGlob = &aaGlobArray[localClientNum];
    if ( aaGlob->ps.weapIndex )
    {
        if ( BG_IsBayonetWeapon(aaGlob->ps.weapIndex) )
            value = aim_autobayonet_range->current.value;
        else
            value = aim_automelee_range->current.value;
        aimAssistRange = AimAssist_GetAimAssistRange(aaGlob->ps.weapIndex, aaGlob->adsLerp);
        autoAimRange = AimAssist_GetAutoAimRange(aaGlob->ps.weapIndex);
        for ( targetIndex = 0; targetIndex < aaGlob->screenTargetCount; ++targetIndex )
        {
            if ( (!aim_autoaim_debug->current.enabled
                 || aaGlob->screenTargets[targetIndex].distSqr <= (float)(autoAimRange * autoAimRange))
                && (!aim_automelee_debug->current.enabled || aaGlob->screenTargets[targetIndex].distSqr <= (float)(value * value))
                && (!aim_slowdown_debug->current.enabled
                 || aaGlob->screenTargets[targetIndex].distSqr <= (float)(aimAssistRange * aimAssistRange))
                && (!aim_lockon_debug->current.enabled
                 || aaGlob->screenTargets[targetIndex].distSqr <= (float)(aimAssistRange * aimAssistRange)) )
            {
                x = (float)(aaGlob->screenTargets[targetIndex].clipMins[0] + 1.0) * (float)(aaGlob->screenWidth * 0.5);
                y = (float)(aaGlob->screenTargets[targetIndex].clipMins[1] + 1.0) * (float)(aaGlob->screenHeight * 0.5);
                CL_DrawStretchPicPhysical(
                    x,
                    y,
                    (float)((float)(aaGlob->screenTargets[targetIndex].clipMaxs[0] + 1.0) * (float)(aaGlob->screenWidth * 0.5))
                - x,
                    (float)((float)(aaGlob->screenTargets[targetIndex].clipMaxs[1] + 1.0) * (float)(aaGlob->screenHeight * 0.5))
                - y,
                    0.0,
                    0.0,
                    1.0,
                    1.0,
                    color,
                    cgMedia.whiteMaterial);
                v3 = va("Pri: %i", targetIndex);
                CL_DrawText(
                    &scrPlaceView[localClientNum],
                    v3,
                    0x7FFFFFFF,
                    cgMedia.smallDevFont,
                    x,
                    y,
                    5,
                    5,
                    1.0,
                    1.0,
                    colorYellow,
                    0);
                ya = y - 10.0;
                v4 = va("Ent: %i", aaGlob->screenTargets[targetIndex].entIndex);
                CL_DrawText(
                    &scrPlaceView[localClientNum],
                    v4,
                    0x7FFFFFFF,
                    cgMedia.smallDevFont,
                    x,
                    ya,
                    5,
                    5,
                    1.0,
                    1.0,
                    colorYellow,
                    0);
                yb = ya - 10.0;
                v5 = va("Dist: %.2f", sqrtf(aaGlob->screenTargets[targetIndex].distSqr));
                CL_DrawText(
                    &scrPlaceView[localClientNum],
                    v5,
                    0x7FFFFFFF,
                    cgMedia.smallDevFont,
                    x,
                    yb,
                    5,
                    5,
                    1.0,
                    1.0,
                    colorWhite,
                    0);
                yc = yb - 10.0;
                v6 = Vec3Length(aaGlob->screenTargets[targetIndex].velocity);
                v7 = va("Speed: %.2f", v6);
                CL_DrawText(
                    &scrPlaceView[localClientNum],
                    v7,
                    0x7FFFFFFF,
                    cgMedia.smallDevFont,
                    x,
                    yc,
                    5,
                    5,
                    1.0,
                    1.0,
                    colorWhite,
                    0);
                msg = va("XHairDist: %.4f", sqrtf(aaGlob->screenTargets[targetIndex].crosshairDistSqr));
                CL_DrawText(
                    &scrPlaceView[localClientNum],
                    msg,
                    0x7FFFFFFF,
                    cgMedia.smallDevFont,
                    x,
                    yc - 10.0,
                    5,
                    5,
                    1.0,
                    1.0,
                    colorWhite,
                    0);
            }
        }
    }
}

