#include "cg_weapons.h"
#include <bgame/bg_local.h>
#include <cgame_mp/cg_local_mp.h>
#include <bgame/bg_weapons.h>
#include <bgame/bg_weapons_def.h>
#include "cg_camera.h"
#include <bgame/bg_perks.h>
#include <ui_mp/ui_gametype_variants_mp.h>
#include <cgame_mp/cg_main_mp.h>
#include <bgame/bg_mantle.h>
#include <client/splitscreen.h>
#include <client_mp/cl_cgame_mp.h>
#include <xanim/dobj_utils.h>
#include <clientscript/cscr_stringlist.h>
#include <qcommon/dobj_management.h>
#include <client_mp/cl_scrn_mp.h>
#include <stringed/stringed_hooks.h>
#include <gfx_d3d/r_stream.h>
#include <bgame/bg_misc.h>
#include <qcommon/threads.h>
#include <clientscript/scr_const.h>
#include <demo/demo_playback.h>
#include "cg_event.h"
#include <gfx_d3d/r_model.h>
#include <cgame_mp/cg_ents_mp.h>
#include <sound/snd_bank.h>
#include "cg_sound.h"
#include <sound/snd_dvar.h>
#include <cgame_mp/cg_view_mp.h>
#include <cgame_mp/cg_vehicles_mp.h>
#include <sound/snd_public_async.h>
#include <win32/win_shared.h>
#include <game_mp/g_main_mp.h>
#include "cg_drawtools.h"
#include <DynEntity/DynEntity_client.h>
#include <gfx_d3d/r_dpvs.h>
#include <server_mp/sv_init_mp.h>
#include <cgame_mp/cg_pose_mp.h>
#include <game/g_debug.h>
#include "cg_world.h"
#include <universal/com_math_anglevectors.h>
#include "cg_local.h"
#include "cg_weapon_options.h"
#include "cg_laser.h"
#include <EffectsCore/fx_dvars.h>
#include <EffectsCore/fx_update.h>
#include "cg_draw_debug.h"
#include <xanim/xanim_clientnotify.h>
#include <clientscript/cscr_vm.h>
#include "cg_scr_main.h"
#include "cg_draw_reticles.h"
#include <bgame/bg_weapons_view.h>
#include <cgame_mp/cg_servercmds_mp.h>
#include <bgame/bg_weapons_ammo.h>
#include <gfx_d3d/r_foliage.h>
#include <game/bullet.h>
#include "cg_main.h"
#include <sound/snd_utils.h>
#include "cg_localents.h"
#include <EffectsCore/fx_beam.h>
#include <EffectsCore/fx_system.h>
#include <universal/q_parse.h>
#include <bgame/bg_unlockable_items.h>
#include <client/client.h>
#include <server/sv_world.h>
#include <cgame/cg_effects_load_obj.h>

unsigned __int8 riflePriorityMap[19] = { 1u, 9u, 9u, 9u, 8u, 7u, 6u, 6u, 6u, 6u, 5u, 5u, 4u, 4u, 4u, 4u, 3u, 3u, 0u };

unsigned int snd_autosim_time;
const dvar_t *snd_autoSim;
snd_autosim g_snd_autosims[64];
snd_autosim_play g_snd_autosim_history[64];
unsigned int snd_autosim_frame;

AnimRateOffset g_animRateOffsets[66] =
{
  { -1, -1 },
  { -1, -1 },
  { -1, -1 },
  { -1, -1 },
  { 956, -1 },
  { -1, -1 },
  { 948, -1 },
  { 964, -1 },
  { 968, -1 },
  { -1, 36 },
  { 972, -1 },
  { -1, 40 },
  { 1004, -1 },
  { 1012, -1 },
  { -1, 44 },
  { -1, 48 },
  { 1020, -1 },
  { 1036, -1 },
  { 1016, -1 },
  { -1, 60 },
  { 1024, -1 },
  { 1032, -1 },
  { 1028, -1 },
  { 1040, -1 },
  { 1044, -1 },
  { 1048, -1 },
  { 1052, -1 },
  { 1056, -1 },
  { 1048, -1 },
  { 1052, -1 },
  { 1056, -1 },
  { 1060, -1 },
  { 1064, -1 },
  { 1068, -1 },
  { 1072, -1 },
  { 1076, -1 },
  { 1080, -1 },
  { 1100, -1 },
  { 1104, -1 },
  { -1, -1 },
  { 1108, -1 },
  { 1120, -1 },
  { -1, -1 },
  { -1, -1 },
  { -1, -1 },
  { 1084, -1 },
  { 1088, -1 },
  { 1092, -1 },
  { 1084, -1 },
  { 1088, -1 },
  { 1092, -1 },
  { 1096, -1 },
  { -1, -1 },
  { 1052, -1 },
  { 1084, -1 },
  { 1088, -1 },
  { 1092, -1 },
  { -1, -1 },
  { -1, -1 },
  { -1, -1 },
  { -1, -1 },
  { -1, -1 },
  { -1, 40 },
  { -1, 36 },
  { -1, -1 },
  { -1, -1 }
};



bool __cdecl CG_JavelinADS(int localClientNum)
{
    int weapIdx; // [esp+4h] [ebp-Ch]
    playerState_s *ps; // [esp+8h] [ebp-8h]

    ps = &CG_GetLocalClientGlobals(localClientNum)->predictedPlayerState;
    weapIdx = BG_GetViewmodelWeaponIndex(ps);
    if ( weapIdx <= 0 )
        return 0;
    if ( ps->fWeaponPosFrac == 1.0 )
        return BG_GetWeaponDef(weapIdx)->overlayInterface == WEAPOVERLAYINTERFACE_JAVELIN;
    return 0;
}

bool __cdecl CG_UICheckWeapLockBlink(int localClientNum, float blinkPerSec)
{
    cg_s *cgameGlob; // [esp+Ch] [ebp-8h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( (cgameGlob->predictedPlayerState.weapLockFlags & 2) != 0 )
        return 1;
    if ( (cgameGlob->predictedPlayerState.weapLockFlags & 1) == 0 )
        return 0;
    if ( blinkPerSec > 0.0 )
        return CG_GetLocalClientGlobals(localClientNum)->time / (int)(float)(1000.0 / blinkPerSec) % 2 != 0;
    return 1;
}

bool __cdecl CG_UICheckWeapLockAttackTop(int localClientNum)
{
    return (CG_GetLocalClientGlobals(localClientNum)->predictedPlayerState.weapLockFlags & 4) != 0;
}

bool __cdecl CG_UICheckWeapLockAttackDirect(int localClientNum)
{
    return (CG_GetLocalClientGlobals(localClientNum)->predictedPlayerState.weapLockFlags & 8) != 0;
}

void __cdecl CG_ResetWeaponAnimTrees(int localClientNum, const playerState_s *ps, int newPlayerstate)
{
    DObj *obj; // [esp+1Ch] [ebp-10h]
    XAnimTree_s *animTree; // [esp+20h] [ebp-Ch]
    unsigned int weapIndex; // [esp+28h] [ebp-4h]

    obj = CG_GetLocalClientViewModelInfo(localClientNum)->viewModelDObj;
    if ( obj )
    {
        animTree = DObjGetTree(obj);
        if ( !animTree
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 1288, 0, "%s", "animTree") )
        {
            __debugbreak();
        }
        XAnimClearTreeGoalWeights(animTree, 0, 0.0, -1);
        XAnimSetGoalWeight(obj, 0, 1.0, 0.0, 1.0, 0, 1u, 0, -1);
        weapIndex = BG_GetViewmodelWeaponIndex(ps);
        if ( weapIndex )
            StartWeaponAnim(localClientNum, weapIndex, obj, 1, 0.0, newPlayerstate);
    }
}

void __cdecl StartWeaponAnim(
                int localClientNum,
                unsigned int weaponNum,
                DObj *obj,
                int animIndex,
                float transitionTime,
                int newPlayerstate)
{
    float rate; // [esp+64h] [ebp-28h]
    float timeFrac; // [esp+68h] [ebp-24h]
    const cg_s *cgameGlob; // [esp+70h] [ebp-1Ch]
    const WeaponVariantDef *weapVariantDef; // [esp+74h] [ebp-18h]
    ViewModelInfo *viewModelInfo; // [esp+78h] [ebp-14h]
    XAnim_s *anims; // [esp+7Ch] [ebp-10h]
    signed int i; // [esp+80h] [ebp-Ch]
    const WeaponDef *weapDef; // [esp+84h] [ebp-8h]

    timeFrac = 0.0f;
    if ( (animIndex <= 0 || animIndex >= 64)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                    392,
                    0,
                    "%s",
                    "(animIndex > WEAP_ANIM_VIEWMODEL_START) && (animIndex < WEAP_ANIM_VIEWMODEL_END)") )
    {
        __debugbreak();
    }
    viewModelInfo = CG_GetLocalClientViewModelInfo(localClientNum);
    anims = viewModelInfo->anims;
    if ( !anims && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 396, 0, "%s", "anims") )
        __debugbreak();
    weapDef = BG_GetWeaponDef(weaponNum);
    weapVariantDef = BG_GetWeaponVariantDef(weaponNum);
    rate = GetWeaponAnimRate(localClientNum, weapVariantDef, anims, animIndex);
    if ( newPlayerstate )
        timeFrac = GetWeaponAnimTimeFrac(localClientNum, weapVariantDef, anims, animIndex);
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( (cgameGlob->predictedPlayerState.weaponstate == 10
         || cgameGlob->predictedPlayerState.weaponstate == 12
         || cgameGlob->predictedPlayerState.weaponstate == 14
         || cgameGlob->predictedPlayerState.weaponstate == 13
         || cgameGlob->predictedPlayerState.weaponstate == 11
         || cgameGlob->predictedPlayerState.weaponstate == 15
         || cgameGlob->predictedPlayerState.weaponstate == 16
         || cgameGlob->predictedPlayerState.weaponstateLeft == 10
         || cgameGlob->predictedPlayerState.weaponstateLeft == 12
         || cgameGlob->predictedPlayerState.weaponstateLeft == 14
         || cgameGlob->predictedPlayerState.weaponstateLeft == 13
         || cgameGlob->predictedPlayerState.weaponstateLeft == 11
         || cgameGlob->predictedPlayerState.weaponstateLeft == 15
         || cgameGlob->predictedPlayerState.weaponstateLeft == 16)
        && (cgameGlob->predictedPlayerState.perks[0] & 0x40000) != 0 )
    {
        if ( perk_weapReloadMultiplier->current.value == 0.0 )
            rate = 1000.0f;
        else
            rate = (float)((float)(1.0 - 1.0) * rate)
                     + (float)((float)(rate / perk_weapReloadMultiplier->current.value) * 1.0);
    }
    else if ( cgameGlob->predictedPlayerState.weaponstate == 7 && weapVariantDef->bRapidFire )
    {
        if ( 0.0 == perk_weapRateMultiplier->current.value )
            rate = 1000.0f;
        else
            rate = rate / perk_weapRateMultiplier->current.value;
    }
    else if ( (cgameGlob->predictedPlayerState.perks[0] & 0x80000) != 0
                 && BG_CanFastSwitch(weapDef, cgameGlob->predictedPlayerState.weaponstate) )
    {
        if ( perk_weapSwitchMultiplier->current.value <= 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                        438,
                        0,
                        "%s\n\t(perk_weapSwitchMultiplier->current.value) = %g",
                        "(perk_weapSwitchMultiplier->current.value > 0.f)",
                        perk_weapSwitchMultiplier->current.value) )
        {
            __debugbreak();
        }
        rate = rate / perk_weapSwitchMultiplier->current.value;
    }
    else if ( (cgameGlob->predictedPlayerState.perks[0] & 0x20000) != 0
                 && cgameGlob->predictedPlayerState.weaponstate == 19 )
    {
        if ( perk_weapMeleeMultiplier->current.value <= 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                        443,
                        0,
                        "%s\n\t(perk_weapMeleeMultiplier->current.value) = %g",
                        "(perk_weapMeleeMultiplier->current.value > 0.f)",
                        perk_weapMeleeMultiplier->current.value) )
        {
            __debugbreak();
        }
        rate = rate / perk_weapMeleeMultiplier->current.value;
    }
    else if ( cgameGlob->predictedPlayerState.weaponstate == 29
                 && (cgameGlob->predictedPlayerState.perks[1] & 0x4000) != 0 )
    {
        if ( 0.0 == perk_sprintRecoveryMultiplier->current.value )
            rate = 1000.0f;
        else
            rate = rate / perk_sprintRecoveryMultiplier->current.value;
    }
    if ( (cgameGlob->predictedPlayerState.weaponstate == 10
         || cgameGlob->predictedPlayerState.weaponstate == 12
         || cgameGlob->predictedPlayerState.weaponstate == 14
         || cgameGlob->predictedPlayerState.weaponstate == 13
         || cgameGlob->predictedPlayerState.weaponstate == 11
         || cgameGlob->predictedPlayerState.weaponstate == 15
         || cgameGlob->predictedPlayerState.weaponstate == 16
         || cgameGlob->predictedPlayerState.weaponstateLeft == 10
         || cgameGlob->predictedPlayerState.weaponstateLeft == 12
         || cgameGlob->predictedPlayerState.weaponstateLeft == 14
         || cgameGlob->predictedPlayerState.weaponstateLeft == 13
         || cgameGlob->predictedPlayerState.weaponstateLeft == 11
         || cgameGlob->predictedPlayerState.weaponstateLeft == 15
         || cgameGlob->predictedPlayerState.weaponstateLeft == 16)
        && ui_gv_reloadSpeedModifier->current.value != 0.0 )
    {
        rate = (float)(rate * ui_gv_reloadSpeedModifier->current.value)
                 - (float)((float)(rate - (float)(rate * ui_gv_reloadSpeedModifier->current.value)) * 0.5);
    }
    for ( i = 1; i < 64; ++i )
    {
        if ( animIndex == i )
        {
            XAnimSetGoalWeight(obj, i, 1.0, transitionTime, rate, 0, 1u, 1, -1);
            if ( newPlayerstate )
            {
                XAnimSetTime(viewModelInfo->tree, i, timeFrac, -1);
                if ( !viewModelInfo->tree->anims
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                                470,
                                0,
                                "%s",
                                "viewModelInfo->tree->anims") )
                {
                    __debugbreak();
                }
            }
        }
        else if ( !weapDef->bDualWield
                     || viewModelInfo->hand[0].iHandAnimIndex == i
                     || viewModelInfo->hand[1].iHandAnimIndex == i )
        {
            if ( !weapDef->bDualWield )
                XAnimSetGoalWeight(obj, i, 0.0, transitionTime, 1.0, 0, 0, 0, -1);
        }
        else
        {
            XAnimSetGoalWeight(obj, i, 0.0, transitionTime, 1.0, 0, 0, 0, -1);
        }
    }
}

double __cdecl GetWeaponAnimRate(
                int localClientNum,
                const WeaponVariantDef *weapVariantDef,
                XAnim_s *anims,
                unsigned int animIndex)
{
    int timea; // [esp+Ch] [ebp-8h]
    int time; // [esp+Ch] [ebp-8h]
    const playerState_s *ps; // [esp+10h] [ebp-4h]

    if ( !weapVariantDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 269, 0, "%s", "weapVariantDef") )
    {
        __debugbreak();
    }
    if ( !anims && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 270, 0, "%s", "anims") )
        __debugbreak();
    if ( animIndex >= 0x42
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                    271,
                    0,
                    "%s\n\t(animIndex) = %i",
                    "(((animIndex >= 0) && (animIndex < NUM_WEAP_ANIMS)))",
                    animIndex) )
    {
        __debugbreak();
    }
    ps = CG_GetPredictedPlayerState(localClientNum);
    if ( (animIndex == 52 || animIndex == 57) && Mantle_IsWeaponEquipped(ps) )
    {
        timea = Mantle_GetDuration(ps);
        return (double)XAnimGetLengthMsec(anims, animIndex) / (double)timea;
    }
    if ( g_animRateOffsets[animIndex].fromWeaponDef < 0 )
    {
        if ( g_animRateOffsets[animIndex].fromWeaponVariantDef < 0 )
            return 1.0;
        time = *(int *)((char *)&weapVariantDef->szInternalName + g_animRateOffsets[animIndex].fromWeaponVariantDef);
    }
    else
    {
        time = *(int *)((char *)&weapVariantDef->weapDef->szOverlayName + g_animRateOffsets[animIndex].fromWeaponDef);
    }
    if ( time < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                    289,
                    0,
                    "%s\n\t(time) = %i",
                    "((time >= 0))",
                    time) )
    {
        __debugbreak();
    }
    if ( time )
        return (double)XAnimGetLengthMsec(anims, animIndex) / (double)time;
    else
        return 0.0;
}

double __cdecl GetWeaponAnimTimeFrac(
                int localClientNum,
                const WeaponVariantDef *weapVariantDef,
                XAnim_s *anims,
                unsigned int animIndex)
{
    snapshot_s *nextSnap; // edx
    double result; // st7
    int totalTime; // [esp+14h] [ebp-18h]
    int totalTimea; // [esp+14h] [ebp-18h]
    cg_s *cgameGlob; // [esp+18h] [ebp-14h]
    int weaponTime; // [esp+1Ch] [ebp-10h]
    int weaponTimeLeft; // [esp+24h] [ebp-8h]

    if ( !weapVariantDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 305, 0, "%s", "weapVariantDef") )
    {
        __debugbreak();
    }
    if ( !anims && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 306, 0, "%s", "anims") )
        __debugbreak();
    if ( animIndex >= 0x42
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                    307,
                    0,
                    "%s\n\t(animIndex) = %i",
                    "(((animIndex >= 0) && (animIndex < NUM_WEAP_ANIMS)))",
                    animIndex) )
    {
        __debugbreak();
    }
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( Demo_IsPlaying() && cgameGlob->nextSnap && cgameGlob->snap )
    {
        nextSnap = cgameGlob->nextSnap;
        weaponTime = (int)(float)((float)((float)cgameGlob->snap->ps.weaponTime
                                                                        * (float)(1.0 - cgameGlob->frameInterpolation))
                                                        + (float)((float)nextSnap->ps.weaponTime * cgameGlob->frameInterpolation));
        weaponTimeLeft = (int)(float)((float)((float)cgameGlob->snap->ps.weaponTimeLeft
                                                                                * (float)(1.0 - cgameGlob->frameInterpolation))
                                                                + (float)((float)nextSnap->ps.weaponTimeLeft * cgameGlob->frameInterpolation));
    }
    else
    {
        weaponTime = cgameGlob->predictedPlayerState.weaponTime;
        weaponTimeLeft = cgameGlob->predictedPlayerState.weaponTimeLeft;
    }
    switch ( animIndex )
    {
        case 6u:
        case 9u:
        case 0xAu:
        case 0xBu:
        case 0xCu:
        case 0xDu:
        case 0xEu:
        case 0xFu:
        case 0x10u:
        case 0x11u:
        case 0x12u:
        case 0x13u:
        case 0x14u:
        case 0x15u:
        case 0x16u:
        case 0x17u:
        case 0x18u:
        case 0x2Cu:
            if ( g_animRateOffsets[animIndex].fromWeaponDef < 0 )
            {
                if ( g_animRateOffsets[animIndex].fromWeaponVariantDef < 0 )
                    return 1.0;
                totalTime = *(int *)((char *)&weapVariantDef->szInternalName + g_animRateOffsets[animIndex].fromWeaponVariantDef);
            }
            else
            {
                totalTime = *(int *)((char *)&weapVariantDef->weapDef->szOverlayName + g_animRateOffsets[animIndex].fromWeaponDef);
            }
            result = (double)(totalTime - weaponTime) / (double)totalTime;
            break;
        case 0x3Eu:
        case 0x3Fu:
            if ( g_animRateOffsets[animIndex].fromWeaponDef < 0 )
            {
                if ( g_animRateOffsets[animIndex].fromWeaponVariantDef < 0 )
                    return 1.0;
                totalTimea = *(int *)((char *)&weapVariantDef->szInternalName + g_animRateOffsets[animIndex].fromWeaponVariantDef);
            }
            else
            {
                totalTimea = *(int *)((char *)&weapVariantDef->weapDef->szOverlayName
                                                        + g_animRateOffsets[animIndex].fromWeaponDef);
            }
            result = (double)(totalTimea - weaponTimeLeft) / (double)totalTimea;
            break;
        default:
            result = 0.0;
            break;
    }
    return result;
}

int __cdecl CG_WeaponDObjHandle(int localClientNum)
{
    return localClientNum + 1536;
}

void __cdecl CG_WeaponMyChanges()
{
    ViewModelInfo *viewModelInfo; // [esp+0h] [ebp-10h]
    XAnim_s *anims; // [esp+4h] [ebp-Ch]

    if ( CL_LocalClient_IsFirstActive(0) )
    {
        if ( *CL_GetConfigString(0xC4Bu) )
        {
            viewModelInfo = CG_GetLocalClientViewModelInfo(0);
            anims = viewModelInfo->anims;
            CG_FreeWeapons(0);
            viewModelInfo->anims = anims;
            CG_RegisterItems(0);
            CG_UpdateWeaponViewmodels(0);
            CG_UpdateHandViewmodels(0);
        }
    }
}

void __cdecl CG_SetWeaponHidePartBits(
                const WeaponVariantDef *weapVariantDef,
                ViewModelInfo *viewModelInfo,
                DObj *obj,
                int modelNum)
{
    char *v4; // eax
    const char *szInternalName; // [esp-4h] [ebp-Ch]
    unsigned __int8 boneIndex; // [esp+3h] [ebp-5h] BYREF
    int tagIndex; // [esp+4h] [ebp-4h]

    for ( tagIndex = 0; tagIndex < 32 && weapVariantDef->hideTags[tagIndex]; ++tagIndex )
    {
        boneIndex = -2;
        if ( DObjGetBoneIndex(obj, weapVariantDef->hideTags[tagIndex], &boneIndex, modelNum) )
        {
            viewModelInfo->partBits[(int)boneIndex >> 5] |= 0x80000000 >> (boneIndex & 0x1F);
        }
        else
        {
            szInternalName = weapVariantDef->szInternalName;
            v4 = SL_ConvertToString(weapVariantDef->hideTags[tagIndex], SCRIPTINSTANCE_SERVER);
            Com_PrintError(14, "CG_SetWeaponHidePartBits: No such bone tag (%s) for weapon (%s)\n", v4, szInternalName);
        }
    }
}

void __cdecl CG_FreeWeapons(int localClientNum)
{
    unsigned int v1; // eax
    ViewModelInfo *viewModelInfo; // [esp+0h] [ebp-4h]

    v1 = CG_WeaponDObjHandle(localClientNum);
    Com_SafeClientDObjFree(v1, localClientNum);
    viewModelInfo = CG_GetLocalClientViewModelInfo(localClientNum);
    if ( viewModelInfo->tree
        && viewModelInfo->anims != viewModelInfo->tree->anims
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                    1471,
                    0,
                    "%s",
                    "viewModelInfo->anims == viewModelInfo->tree->anims") )
    {
        __debugbreak();
    }
    viewModelInfo->viewModelDObj = 0;
    viewModelInfo->anims = 0;
    if ( viewModelInfo->tree )
    {
        if ( viewModelInfo->tree->anims && viewModelInfo->tree->anims->size )
            XAnimFreeTree(viewModelInfo->tree, 0, SCRIPTINSTANCE_SERVER);
        viewModelInfo->tree = 0;
    }
    memset((unsigned __int8 *)cg_weaponsArray[localClientNum], 0, 0x12000u);
}

int removeMeWhenMPStopsCrashingInHere;
void __cdecl CG_RegisterWeapon(int localClientNum, unsigned int weaponNum)
{
    unsigned int NumWeapons; // eax
    weaponInfo_s *weapInfo; // [esp+4h] [ebp-10h]
    const WeaponVariantDef *weapVariantDef; // [esp+Ch] [ebp-8h]
    const WeaponDef *weapDef; // [esp+10h] [ebp-4h]

    removeMeWhenMPStopsCrashingInHere = weaponNum;
    if ( weaponNum )
    {
        if ( weaponNum >= BG_GetNumWeapons() )
        {
            NumWeapons = BG_GetNumWeapons();
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                            1510,
                            0,
                            "weaponNum doesn't index BG_GetNumWeapons()\n\t%i not in [0, %i)",
                            weaponNum,
                            NumWeapons) )
                __debugbreak();
        }
        if ( (int)(weaponNum + 7) > 2055
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                        1513,
                        0,
                        "%s",
                        "weaponNum + WEAPON_HINT_OFFSET <= LAST_WEAPON_HINT") )
        {
            __debugbreak();
        }
        weapInfo = CG_GetLocalClientWeaponInfo(localClientNum, weaponNum);
        weapVariantDef = BG_GetWeaponVariantDef(weaponNum);
        weapDef = BG_GetWeaponDef(weaponNum);
        if ( weapDef->bDualWield )
        {
            BG_GetWeaponVariantDef(weapDef->dualWieldWeaponIndex);
            BG_GetWeaponDef(weapDef->dualWieldWeaponIndex);
        }
        if ( !weapInfo->registered )
        {
            SCR_UpdateLoadScreen();
            weapInfo->handModel = 0;
            weapInfo->gogglesModel = 0;
            weapInfo->rocketModel = 0;
            *(unsigned int *)&weapInfo->weapModelIdx = 0;
            weapInfo->registered = 0;
            weapInfo->item = 0;
            weapInfo->translatedDisplayName = 0;
            weapInfo->translatedModename = 0;
            weapInfo->translatedAIOverlayDescription = 0;
            weapInfo->registered = 1;
            if ( !weapInfo->handModel )
                weapInfo->handModel = weapDef->handXModel;
            weapInfo->item = &bg_itemlist[weaponNum];
            if (weapDef->hudIcon)
                cgMedia.hintMaterials[weaponNum + 7] = weapDef->hudIcon;
            else
                cgMedia.hintMaterials[weaponNum + 7] = cgMedia.hintMaterials[3];
            weapInfo->translatedDisplayName = SEH_StringEd_GetString((char*)weapVariantDef->szDisplayName);
            if ( !weapInfo->translatedDisplayName )
            {
                if ( loc_warnings->current.enabled )
                {
                    if ( loc_warningsAsErrors->current.enabled )
                        Com_Error(
                            ERR_LOCALIZATION,
                            "Weapon %s: Could not translate display name \"%s\"",
                            weapVariantDef->szInternalName,
                            weapVariantDef->szDisplayName);
                    else
                        Com_PrintWarning(
                            17,
                            "WARNING: Weapon %s: Could not translate display name \"%s\"\n",
                            weapVariantDef->szInternalName,
                            weapVariantDef->szDisplayName);
                }
                weapInfo->translatedDisplayName = weapVariantDef->szDisplayName;
            }
            weapInfo->translatedModename = SEH_StringEd_GetString((char *)weapDef->szModeName);
            if ( !weapInfo->translatedModename )
            {
                if ( loc_warnings->current.enabled )
                {
                    if ( loc_warningsAsErrors->current.enabled )
                        Com_Error(
                            ERR_LOCALIZATION,
                            "Weapon %s: Could not translate mode name \"%s\"",
                            weapVariantDef->szInternalName,
                            weapDef->szModeName);
                    else
                        Com_PrintWarning(
                            17,
                            "WARNING: Weapon %s: Could not translate mode name \"%s\"\n",
                            weapVariantDef->szInternalName,
                            weapDef->szModeName);
                }
                weapInfo->translatedModename = weapDef->szModeName;
            }
            weapInfo->translatedAIOverlayDescription = SEH_StringEd_GetString((char *)weapDef->szOverlayName);
            if ( !weapInfo->translatedAIOverlayDescription )
            {
                if ( loc_warnings->current.enabled )
                {
                    if ( loc_warningsAsErrors->current.enabled )
                        Com_Error(
                            ERR_LOCALIZATION,
                            "Weapon %s: Could not translate AI overlay description \"%s\"",
                            weapVariantDef->szInternalName,
                            weapDef->szOverlayName);
                    else
                        Com_PrintWarning(
                            17,
                            "WARNING: Weapon %s: Could not translate AI overlay description \"%s\"\n",
                            weapVariantDef->szInternalName,
                            weapDef->szOverlayName);
                }
                weapInfo->translatedAIOverlayDescription = weapDef->szOverlayName;
            }
        }
    }
}

bool __cdecl ShouldRebuildTree(
                int localClientNum,
                const weaponInfo_s *weapInfo,
                int weaponNum,
                unsigned __int8 weaponModel,
                XModel *newHands)
{
    if ( weapInfo->weapModelIdx != weaponModel )
        return 1;
    if ( weapInfo->handModel == newHands )
        return CG_GetLocalClientGlobals(localClientNum)->prevViewmodelWeapon != weaponNum;
    return 1;
}

bool __cdecl ShouldRebuildAnims(int localClientNum, const ViewModelInfo *viewModelInfo, int weaponNum)
{
    return !viewModelInfo->anims || CG_GetLocalClientGlobals(localClientNum)->prevViewmodelWeapon != weaponNum;
}

void __cdecl CG_TouchViewModels(int localClientNum)
{
    ViewModelInfo *viewModelInfo; // [esp+0h] [ebp-4h]

    viewModelInfo = CG_GetLocalClientViewModelInfo(localClientNum);
    if ( viewModelInfo->viewModelDObj )
        R_StreamTouchDObjAndCheck(viewModelInfo->viewModelDObj, 0);
}

void __cdecl CG_UpdateWeaponViewmodels(int localClientNum)
{
    int slot; // [esp+Ch] [ebp-14h]
    unsigned __int8 weaponModel; // [esp+13h] [ebp-Dh]
    weaponInfo_s *weapInfo; // [esp+14h] [ebp-Ch]
    cg_s *cgameGlob; // [esp+18h] [ebp-8h]
    unsigned int weaponIndex; // [esp+1Ch] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    for ( slot = 0; slot < 15; ++slot )
    {
        weaponIndex = cgameGlob->nextSnap->ps.heldWeapons[slot].weapon;
        if ( weaponIndex )
        {
            weapInfo = CG_GetLocalClientWeaponInfo(localClientNum, weaponIndex);
            weaponModel = BG_GetPlayerWeaponModel(&cgameGlob->nextSnap->ps, weaponIndex);
            if ( weapInfo->weapModelIdx != weaponModel )
                ChangeViewmodelDobj(
                    localClientNum,
                    &cgameGlob->nextSnap->ps,
                    weaponIndex,
                    weaponModel,
                    weapInfo->handModel,
                    weapInfo->gogglesModel,
                    weapInfo->rocketModel,
                    1,
                    0);
        }
    }
}

void __cdecl ChangeViewmodelDobj(
    int localClientNum,
    const playerState_s *ps,
    unsigned int weaponNum,
    unsigned __int8 weaponModel,
    XModel *newHands,
    XModel *newGoggles,
    XModel *newRocket,
    bool updateClientInfo,
    bool forceRebuildTree)
{
    unsigned int NumWeapons; // eax
    const WeaponVariantDef *weapVarDef; // eax
    DObj *blendTime; // [esp+14h] [ebp-60h]
    unsigned __int16 tag_knife_attach; // [esp+1Eh] [ebp-56h]
    bool v13; // [esp+20h] [ebp-54h]
    const WeaponDef *weapDefDW; // [esp+2Ch] [ebp-48h]
    weaponInfo_s *weapInfo; // [esp+30h] [ebp-44h]
    const WeaponVariantDef *weapVariantDefDW; // [esp+34h] [ebp-40h]
    int dobjHandle; // [esp+38h] [ebp-3Ch]
    int mdlIdx; // [esp+3Ch] [ebp-38h]
    const WeaponVariantDef *weapVariantDef; // [esp+44h] [ebp-30h]
    ViewModelInfo *viewModelInfo; // [esp+48h] [ebp-2Ch]
    bool created_tree; // [esp+4Fh] [ebp-25h]
    const WeaponDef *weapDef; // [esp+50h] [ebp-24h]
    DObjModel_s dobjModels[4]; // [esp+54h] [ebp-20h] BYREF

    if (!ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 1645, 0, "%s", "ps"))
        __debugbreak();
    if (!Sys_IsMainThread()
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
            1647,
            0,
            "%s",
            "Sys_IsMainThread()"))
    {
        __debugbreak();
    }
    if (weaponNum)
    {
        if (weaponNum >= BG_GetNumWeapons())
        {
            NumWeapons = BG_GetNumWeapons();
            if (!Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                1652,
                0,
                "weaponNum doesn't index BG_GetNumWeapons()\n\t%i not in [0, %i)",
                weaponNum,
                NumWeapons))
                __debugbreak();
        }
        if (!newHands
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 1653, 0, "%s", "newHands"))
        {
            __debugbreak();
        }
        weapVariantDef = BG_GetWeaponVariantDef(weaponNum);
        weapDef = BG_GetWeaponDef(weaponNum);
        weapVariantDefDW = 0;
        if (weapDef->gunXModel[weaponModel])
        {
            CG_RegisterWeapon(localClientNum, weaponNum);
            weapInfo = CG_GetLocalClientWeaponInfo(localClientNum, weaponNum);
            viewModelInfo = CG_GetLocalClientViewModelInfo(localClientNum);
            weapInfo->handModel = newHands;
            weapInfo->weapModelIdx = weaponModel;
            if (weaponNum == BG_GetViewmodelWeaponIndex(ps))
            {
                v13 = forceRebuildTree || ShouldRebuildTree(localClientNum, weapInfo, weaponNum, weaponModel, newHands);
                weapInfo->gogglesModel = newGoggles;
                weapInfo->rocketModel = newRocket;
                dobjModels[0].boneName = 0;
                dobjModels[0].ignoreCollision = 0;
                dobjModels[0].model = weapInfo->handModel;
                if (weapDef->weapType == WEAPTYPE_MELEE)
                    tag_knife_attach = scr_const.tag_knife_attach;
                else
                    tag_knife_attach = scr_const.tag_weapon;
                dobjModels[1].boneName = tag_knife_attach;
                dobjModels[1].ignoreCollision = 0;
                dobjModels[1].model = weapDef->gunXModel[weaponModel];
                mdlIdx = 2;
                if (weapDef->bDualWield)
                {
                    weapVariantDefDW = BG_GetWeaponVariantDef(weapDef->dualWieldWeaponIndex);
                    weapDefDW = BG_GetWeaponDef(weapDef->dualWieldWeaponIndex);
                    dobjModels[2].boneName = scr_const.tag_weapon1;
                    dobjModels[2].ignoreCollision = 0;
                    dobjModels[2].model = *weapDefDW->gunXModel;
                    mdlIdx = 3;
                }
                if (weapInfo->gogglesModel)
                {
                    if (overrideNVGModelWithKnife->current.enabled)
                        dobjModels[mdlIdx].boneName = scr_const.tag_gasmask2;
                    else
                        dobjModels[mdlIdx].boneName = scr_const.tag_gasmask;
                    dobjModels[mdlIdx].model = weapInfo->gogglesModel;
                    dobjModels[mdlIdx++].ignoreCollision = 0;
                }
                if (weapInfo->rocketModel)
                {
                    dobjModels[mdlIdx].boneName = scr_const.tag_clip;
                    dobjModels[mdlIdx].ignoreCollision = 0;
                    dobjModels[mdlIdx++].model = weapInfo->rocketModel;
                }
                dobjHandle = CG_WeaponDObjHandle(localClientNum);
                if (viewModelInfo->viewModelDObj)
                    Com_SafeClientDObjFree(dobjHandle, localClientNum);
                if (v13 && viewModelInfo->tree)
                {
                    XAnimFreeTree(viewModelInfo->tree, 0, SCRIPTINSTANCE_SERVER);
                    viewModelInfo->tree = 0;
                }
                if (ShouldRebuildAnims(localClientNum, viewModelInfo, weaponNum))
                    CG_CreateWeaponViewModelXAnim(viewModelInfo, weapVariantDef);
                created_tree = 0;
                if (!viewModelInfo->tree)
                {
                    if (!viewModelInfo->anims
                        && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                            1752,
                            0,
                            "%s",
                            "viewModelInfo->anims"))
                    {
                        __debugbreak();
                    }
                    viewModelInfo->tree = XAnimCreateTree(viewModelInfo->anims, (void*(*)(unsigned int))Hunk_AllocXAnimClient);
                    if (!viewModelInfo->tree
                        && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                            1754,
                            0,
                            "%s",
                            "viewModelInfo->tree"))
                    {
                        __debugbreak();
                    }
                    created_tree = 1;
                }
                if (!viewModelInfo->tree
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                        1757,
                        0,
                        "%s",
                        "viewModelInfo->tree"))
                {
                    __debugbreak();
                }
                if (mdlIdx > 4
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                        1759,
                        0,
                        "%s",
                        "mdlIdx <= MYMODELCOUNT"))
                {
                    __debugbreak();
                }
                viewModelInfo->viewModelDObj = Com_ClientDObjCreate(
                    dobjModels,
                    mdlIdx,
                    viewModelInfo->tree,
                    dobjHandle,
                    localClientNum);
                if (created_tree)
                {
                    viewModelInfo->hand[0].iPrevAnim = -1;
                    viewModelInfo->hand[1].iPrevAnim = -1;
                    XAnimClearTreeGoalWeights(viewModelInfo->tree, 0, 0.0, -1);
                    XAnimSetGoalWeight(viewModelInfo->viewModelDObj, 0, 1.0, 0.0, 1.0, 0, 1u, 0, -1);
                    if (BG_GetAmmoInClip(ps, weaponNum))
                        XAnimSetGoalWeight(viewModelInfo->viewModelDObj, 1u, 1.0, 0.0, 1.0, 0, 1u, 0, -1);
                    else
                        XAnimSetGoalWeight(viewModelInfo->viewModelDObj, 2u, 1.0, 0.0, 1.0, 0, 1u, 0, -1);
                    if (**((_BYTE **)weapVariantDef->szXAnims + 65))
                    {
                        XAnimSetGoalWeight(viewModelInfo->viewModelDObj, 0x41u, 1.0, 0.0, 0.0, 0, 1u, 0, -1);
                        XAnimSetTime(viewModelInfo->tree, 0x41u, 1.0, -1);
                    }
                }
                viewModelInfo->partBits[0] = 0;
                viewModelInfo->partBits[1] = 0;
                viewModelInfo->partBits[2] = 0;
                viewModelInfo->partBits[3] = 0;
                viewModelInfo->partBits[4] = 0;
                if (weapDef->inventoryType == WEAPINVENTORY_ALTMODE)
                {
                    blendTime = viewModelInfo->viewModelDObj;
                    //weapVarDef = BG_GetWeaponVariantDef(*((_DWORD *)ps + 82));
                    weapVarDef = BG_GetWeaponVariantDef(ps->lastWeaponAltModeSwitch);
                    CG_SetWeaponHidePartBits(weapVarDef, viewModelInfo, blendTime, -1);
                }
                else
                {
                    CG_SetWeaponHidePartBits(weapVariantDef, viewModelInfo, viewModelInfo->viewModelDObj, -1);
                }
                if (weapDef->bDualWield)
                    CG_SetWeaponHidePartBits(weapVariantDefDW, viewModelInfo, viewModelInfo->viewModelDObj, 2);
                DObjSetHidePartBits(viewModelInfo->viewModelDObj, viewModelInfo->partBits);
                if (updateClientInfo)
                    DObjUpdateClientInfo(viewModelInfo->viewModelDObj, 0.050000001, 0);
                if (viewModelInfo
                    && viewModelInfo->tree
                    && viewModelInfo->tree->anims != viewModelInfo->anims
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                        1813,
                        0,
                        "%s",
                        "viewModelInfo->tree->anims == viewModelInfo->anims"))
                {
                    __debugbreak();
                }
            }
        }
    }
}

void __cdecl CG_CreateWeaponViewModelXAnim(ViewModelInfo *viewModelInfo, const WeaponVariantDef *weapVariantDef)
{
    int v2; // [esp+0h] [ebp-1Ch]
    int v3; // [esp+4h] [ebp-18h]
    signed int animIndex; // [esp+Ch] [ebp-10h]
    signed int animIndexa; // [esp+Ch] [ebp-10h]
    const WeaponVariantDef *DualWieldWeapVariantDef; // [esp+10h] [ebp-Ch]
    WeaponDef *weapDef; // [esp+14h] [ebp-8h]
    XAnim_s *pAnims; // [esp+18h] [ebp-4h]
    XAnim_s *pAnimsa; // [esp+18h] [ebp-4h]

    if ( !weapVariantDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 1325, 0, "%s", "weapVariantDef") )
    {
        __debugbreak();
    }
    if ( !viewModelInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 1326, 0, "%s", "viewModelInfo") )
    {
        __debugbreak();
    }
    weapDef = weapVariantDef->weapDef;
    DualWieldWeapVariantDef = 0;
    if ( weapDef->bDualWield )
    {
        DualWieldWeapVariantDef = BG_GetWeaponVariantDef(weapDef->dualWieldWeaponIndex);
    }
    else if ( !CG_NonDualWieldAnimsValid(weapVariantDef) )
    {
        return;
    }
    if ( !viewModelInfo->anims )
    {
        pAnims = XAnimCreateAnims("VIEWMODEL", 66, Hunk_AllocXAnimClient);
        if ( !pAnims
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 1344, 0, "%s", "pAnims") )
        {
            __debugbreak();
        }
        viewModelInfo->anims = pAnims;
    }
    pAnimsa = viewModelInfo->anims;
    XAnimFreeList(pAnimsa);
    XAnimBlend(pAnimsa, 0, "root", 1u, 0x41u, 0);
    for ( animIndex = 1; animIndex < 66; ++animIndex )
    {
        if ( weapDef->bDualWield && animIndex >= 58 && animIndex <= 63 )
        {
            if ( *DualWieldWeapVariantDef->szXAnims[animIndex] )
                v3 = animIndex;
            else
                v3 = 1;
            BG_CreateXAnim(pAnimsa, animIndex, DualWieldWeapVariantDef->szXAnims[v3]);
        }
        else
        {
            if ( *weapVariantDef->szXAnims[animIndex] )
                v2 = animIndex;
            else
                v2 = 1;
            BG_CreateXAnim(pAnimsa, animIndex, weapVariantDef->szXAnims[v2]);
        }
    }
    if ( **((_BYTE **)weapVariantDef->szXAnims + 64) && XAnimIsLooped(pAnimsa, 0x40u) )
        Com_Error(ERR_DROP, "CG_RegisterWeapon: ADS anim [%s] cannot be looping", *((unsigned int *)weapVariantDef->szXAnims + 64));
    if ( **((_BYTE **)weapVariantDef->szXAnims + 65) && XAnimIsLooped(pAnimsa, 0x41u) )
        Com_Error(ERR_DROP, "CG_RegisterWeapon: ADS anim [%s] cannot be looping", *((unsigned int *)weapVariantDef->szXAnims + 65));
    for ( animIndexa = 53; animIndexa <= 57; ++animIndexa )
    {
        if ( *weapVariantDef->szXAnims[animIndexa] )
        {
            if ( !XAnimHasBone(pAnimsa, animIndexa, scr_const.tag_camera) )
                Com_Error(ERR_DROP, "G_RegisterWeapon: Camera anim [%s] missing tag_camera bone", weapVariantDef->szXAnims[animIndexa]);
            if ( XAnimGetBoneCount(pAnimsa, animIndexa) > 1u )
                Com_Error(ERR_DROP, "CG_RegisterWeapon: Camera anim [%s] can only have a tag_camera bone", weapVariantDef->szXAnims[animIndexa]);
        }
    }
    if ( viewModelInfo
        && viewModelInfo->tree
        && viewModelInfo->tree->anims != viewModelInfo->anims
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                    1394,
                    0,
                    "%s",
                    "viewModelInfo->tree->anims == viewModelInfo->anims") )
    {
        __debugbreak();
    }
}

bool __cdecl CG_NonDualWieldAnimsValid(const WeaponVariantDef *weapVariantDef)
{
    return *((unsigned int *)weapVariantDef->szXAnims + 1) && **((_BYTE **)weapVariantDef->szXAnims + 1);
}

void __cdecl CG_UpdateHandViewmodels(int localClientNum)
{
    int slot; // [esp+Ch] [ebp-20h]
    unsigned __int8 weaponModel; // [esp+13h] [ebp-19h]
    weaponInfo_s *weapInfo; // [esp+14h] [ebp-18h]
    cg_s *cgameGlob; // [esp+18h] [ebp-14h]
    XModel *handModel; // [esp+1Ch] [ebp-10h]
    char *modelName; // [esp+20h] [ebp-Ch]
    unsigned int weaponIndex; // [esp+24h] [ebp-8h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->snap->ps.viewmodelIndex > 0 )
    {
        modelName = CL_GetConfigString(cgameGlob->snap->ps.viewmodelIndex + 1568);
        if ( (!modelName || !*modelName)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                        1941,
                        0,
                        "%s",
                        "modelName && modelName[0]") )
        {
            __debugbreak();
        }
        handModel = R_RegisterModel(modelName);
        if ( !handModel
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 1949, 0, "%s", "handModel") )
        {
            __debugbreak();
        }
        for ( slot = 0; slot < 15; ++slot )
        {
            weaponIndex = cgameGlob->nextSnap->ps.heldWeapons[slot].weapon;
            if ( weaponIndex )
            {
                weapInfo = CG_GetLocalClientWeaponInfo(localClientNum, weaponIndex);
                weaponModel = BG_GetPlayerWeaponModel(&cgameGlob->predictedPlayerState, weaponIndex);
                BG_GetWeaponDef(weaponIndex);
                if ( weapInfo->handModel != handModel )
                    ChangeViewmodelDobj(
                        localClientNum,
                        &cgameGlob->predictedPlayerState,
                        weaponIndex,
                        weaponModel,
                        handModel,
                        weapInfo->gogglesModel,
                        weapInfo->rocketModel,
                        1,
                        0);
            }
        }
    }
}

void __cdecl CG_RegisterItemVisuals(int localClientNum, unsigned int weapIdx)
{
    iassert(bg_itemlist[weapIdx].giType == IT_WEAPON);
    CG_RegisterWeapon(localClientNum, weapIdx);
}

void __cdecl CG_RegisterItems(int localClientNum)
{
    char v1; // dl
    char *v2; // [esp+8h] [ebp-818h]
    const char *ConfigString; // [esp+Ch] [ebp-814h]
    char items[2052]; // [esp+10h] [ebp-810h] BYREF
    int i; // [esp+818h] [ebp-8h]
    int digit; // [esp+81Ch] [ebp-4h]

    ConfigString = CL_GetConfigString(3147);
    v2 = items;
    do
    {
        v1 = *ConfigString;
        *v2++ = *ConfigString++;
    }
    while ( v1 );
    for ( i = 1; i < 2048 && items[i / 4]; ++i )
    {
        digit = items[i / 4];
        if ( digit > '9' )
            digit -= 87;
        else
            digit -= '0';
        if ( (digit & (1 << (i & 3))) != 0 )
            CG_RegisterItemVisuals(localClientNum, i);
    }
}

void __cdecl WeaponFlash(
                int localClientNum,
                int dobjHandle,
                unsigned int weaponNum,
                int bViewFlash,
                unsigned int flashTag)
{
    const FxEffectDef *viewFlashEffect; // [esp+0h] [ebp-Ch]
    const WeaponDef *weapDef; // [esp+8h] [ebp-4h]

    weapDef = BG_GetWeaponDef(weaponNum);
    if ( bViewFlash )
        viewFlashEffect = weapDef->viewFlashEffect;
    else
        viewFlashEffect = weapDef->worldFlashEffect;
    if ( viewFlashEffect )
        CG_PlayBoltedEffect(localClientNum, viewFlashEffect, dobjHandle, flashTag);
}

void __cdecl CG_HoldBreathInit(cg_s *cgameGlob)
{
    cgameGlob->holdBreathTime = -1;
    cgameGlob->holdBreathInTime = 0;
    cgameGlob->holdBreathDelay = 0;
    cgameGlob->holdBreathFrac = 0.0f;
}

void __cdecl CG_UpdateViewModelPose(const DObj *obj, int localClientNum)
{
    cg_s *cgameGlob; // [esp+8h] [ebp-4h]

    if ( obj )
    {
        DObjLock(obj);
        DObjClearSkel(obj);
        DObjUnlock(obj);
    }
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    AxisToAngles(cgameGlob->viewModelAxis, cgameGlob->viewModelPose.angles);
    cgameGlob->viewModelPose.origin[0] = cgameGlob->viewModelAxis[3][0];
    cgameGlob->viewModelPose.origin[1] = cgameGlob->viewModelAxis[3][1];
    cgameGlob->viewModelPose.origin[2] = cgameGlob->viewModelAxis[3][2];
}

void __cdecl CG_UpdateMinigunSpin(int localClientNum, cg_s *cgameGlob, centity_s *cent)
{
    unsigned int ClientWeapon; // eax
    unsigned int AliasId; // eax
    float v6; // [esp+10h] [ebp-6Ch]
    float v7; // [esp+14h] [ebp-68h]
    float v8; // [esp+18h] [ebp-64h]
    snapshot_s *nextSnap; // [esp+1Ch] [ebp-60h]
    float v10; // [esp+40h] [ebp-3Ch]
    float v11; // [esp+44h] [ebp-38h]
    DObj *obj; // [esp+50h] [ebp-2Ch]
    float sndOrigin[3]; // [esp+54h] [ebp-28h] BYREF
    centity_s *vehCent; // [esp+60h] [ebp-1Ch]
    float inc; // [esp+64h] [ebp-18h]
    float lerpRate; // [esp+68h] [ebp-14h]
    bool isSabretooth; // [esp+6Fh] [ebp-Dh]
    const WeaponDef *weaponDef; // [esp+70h] [ebp-Ch]
    clientInfo_t *ci; // [esp+74h] [ebp-8h]
    int clientNum; // [esp+78h] [ebp-4h]

    clientNum = cent->nextState.clientNum;
    ci = &cgameGlob->bgs.clientinfo[clientNum];
    ClientWeapon = CG_GetClientWeapon(clientNum, localClientNum);
    weaponDef = BG_GetWeaponDef(ClientWeapon);
    if ( !cg_paused->current.integer && !Demo_IsPaused() && !Demo_IsCompleted() && !Demo_GetClipPausedState() )
    {
        if ( weaponDef->fireType == WEAPON_FIRETYPE_MINIGUN )
        {
            isSabretooth = weaponDef->weapClass == WEAPCLASS_SPREAD;
            if ( (cent->nextState.lerp.eFlags2 & 0x400000) != 0 && !isSabretooth || (cent->nextState.lerp.eFlags & 0x40) != 0 )
            {
                if ( weaponDef->iSpinUpTime )
                    v8 = 1.0 / (float)weaponDef->iSpinUpTime;
                else
                    v8 = 1.0f;
                lerpRate = v8;
            }
            else
            {
                if ( weaponDef->iSpinDownTime )
                    v7 = 1.0 / (float)weaponDef->iSpinDownTime;
                else
                    v7 = 1.0f;
                lerpRate = -v7;
            }
            iassert(!IS_NAN(lerpRate));
            ci->lastWeaponSpinLerp = ci->weaponSpinLerp;
            v10 = (float)((float)cgameGlob->frametime * lerpRate) + ci->weaponSpinLerp;
            if ( (float)(v10 - 1.0) < 0.0 )
                v11 = (float)((float)cgameGlob->frametime * lerpRate) + ci->weaponSpinLerp;
            else
                v11 = 1.0f;
            if ( (float)(0.0 - v10) < 0.0 )
                v6 = v11;
            else
                v6 = 0.0f;
            ci->weaponSpinLerp = v6;
            iassert(!IS_NAN(ci->weaponSpinLerp));
            inc = ci->weaponSpinLerp * weaponDef->spinRate;
            if ( inc < 5.0 && ci->weaponSpinLerp > 0.0099999998 )
                inc = 5.0f;
            if ( inc > 0.0 )
                ci->weaponSpin = I_fmodulus(ci->weaponSpin + inc, 360.0);
            CG_UpdateMinigunSounds(localClientNum, cgameGlob, cent, weaponDef);
        }
        else if ( (cent->nextState.lerp.eFlags & 0x4000) == 0 || cent->nextState.lerp.u.player.vehicleSeat )
        {
            ci->lastWeaponSpinLerp = 0.0f;
            ci->weaponSpinLerp = 0.0f;
        }
        else
        {
            sndOrigin[0] = cent->pose.origin[0];
            sndOrigin[1] = cent->pose.origin[1];
            sndOrigin[2] = cent->pose.origin[2];
            vehCent = CG_GetEntity(localClientNum, ci->attachedVehEntNum);
            weaponDef = BG_GetWeaponDef(vehCent->nextState.weapon);
            obj = Com_GetClientDObj(vehCent->nextState.number, localClientNum);
            if ( weaponDef->fireType == WEAPON_FIRETYPE_MINIGUN && (cent->nextState.lerp.eFlags & 0x40) != 0 )
            {
                if ( obj )
                {
                    CG_DObjGetWorldTagPos(&vehCent->pose, obj, scr_const.tag_flash, sndOrigin);
                }
                else
                {
                    sndOrigin[0] = vehCent->pose.origin[0];
                    sndOrigin[1] = vehCent->pose.origin[1];
                    sndOrigin[2] = vehCent->pose.origin[2];
                }
                nextSnap = cgameGlob->nextSnap;
                if ( (nextSnap->ps.otherFlags & 6) != 0 && cent->nextState.number == nextSnap->ps.clientNum )
                    AliasId = SND_FindAliasId((char *)weaponDef->fireLoopSoundPlayer);
                else
                    AliasId = SND_FindAliasId((char *)weaponDef->fireLoopSound);
                CG_PlaySound(localClientNum, vehCent->nextState.number, sndOrigin, 0, 0, 1.0, AliasId);
            }
        }
    }
}

double __cdecl I_fmodulus(float f, float mod)
{
    float v3; // [esp+8h] [ebp-24h]
    float v4; // [esp+Ch] [ebp-20h]
    float v5; // [esp+14h] [ebp-18h]
    float v6; // [esp+18h] [ebp-14h]
    float v7; // [esp+1Ch] [ebp-10h]
    float v8; // [esp+20h] [ebp-Ch]

    v7 = floor(f);
    v6 = floor(mod);
    v5 = floor(mod);
    v8 = mod - v5;
    v4 = (float)((int)v7 % (int)v6);
    v3 = floor(f);
    return v8 - (f - v3) + v4;
}

void __cdecl CG_UpdateMinigunSounds(int localClientNum, cg_s *cgameGlob, centity_s *cent, const WeaponDef *weaponDef)
{
    unsigned int AliasId; // eax
    unsigned int v5; // eax
    unsigned int v6; // eax
    unsigned int v7; // eax
    unsigned int v8; // eax
    float pitch; // [esp+18h] [ebp-74h]
    bool v10; // [esp+20h] [ebp-6Ch]
    float v11; // [esp+24h] [ebp-68h]
    snapshot_s *nextSnap; // [esp+4Ch] [ebp-40h]
    unsigned int spinLoopId; // [esp+50h] [ebp-3Ch]
    DObj *vehObj; // [esp+54h] [ebp-38h]
    const vehicle_info_t *info; // [esp+58h] [ebp-34h]
    int entityNum; // [esp+64h] [ebp-28h]
    DObj *obj; // [esp+68h] [ebp-24h]
    float sndOrigin[3]; // [esp+70h] [ebp-1Ch] BYREF
    float spinScalar; // [esp+7Ch] [ebp-10h]
    centity_s *vehCent; // [esp+80h] [ebp-Ch]
    clientInfo_t *ci; // [esp+84h] [ebp-8h]
    float spinPitch; // [esp+88h] [ebp-4h]

    entityNum = cent->nextState.clientNum;
    ci = &cgameGlob->bgs.clientinfo[entityNum];
    nextSnap = cgameGlob->nextSnap;
    v10 = (nextSnap->ps.otherFlags & 6) != 0 && cent->nextState.number == nextSnap->ps.clientNum;
    spinPitch = snd_minigun_pitch_start->current.value;
    memset(sndOrigin, 0, sizeof(sndOrigin));
    obj = Com_GetClientDObj(cent->nextState.number, localClientNum);
    if ( obj )
    {
        if ( !CG_DObjGetWorldTagPos(&cent->pose, obj, scr_const.tag_flash, sndOrigin) )
        {
            sndOrigin[0] = cent->pose.origin[0];
            sndOrigin[1] = cent->pose.origin[1];
            sndOrigin[2] = cent->pose.origin[2];
        }
    }
    else
    {
        sndOrigin[0] = cent->pose.origin[0];
        sndOrigin[1] = cent->pose.origin[1];
        sndOrigin[2] = cent->pose.origin[2];
    }
    if ( !CG_KillCamEntityEnabled(localClientNum) )
    {
        if ( (cent->nextState.lerp.eFlags & 0x4000) != 0 )
        {
            vehCent = CG_GetEntity(localClientNum, ci->attachedVehEntNum);
            entityNum = vehCent->nextState.number;
            sndOrigin[0] = cent->pose.origin[0];
            sndOrigin[1] = cent->pose.origin[1];
            sndOrigin[2] = cent->pose.origin[2];
            if ( ci->attachedVehSeat >= 1 && ci->attachedVehSeat <= 4 )
            {
                vehObj = Com_GetClientDObj(vehCent->nextState.number, localClientNum);
                info = CG_GetVehicleInfo(vehCent->nextState.vehicleState.vehicleInfoIndex);
                if ( BG_GetWeaponDef(*(unsigned __int16 *)&info->gunnerWeapon[3][2 * ci->attachedVehSeat + 62])->fireType == WEAPON_FIRETYPE_MINIGUN )
                {
                    if ( vehObj )
                    {
                        CG_DObjGetWorldTagPos(&vehCent->pose, vehObj, scr_const.tag_flash, sndOrigin);
                    }
                    else
                    {
                        sndOrigin[0] = vehCent->pose.origin[0];
                        sndOrigin[1] = vehCent->pose.origin[1];
                        sndOrigin[2] = vehCent->pose.origin[2];
                    }
                }
            }
        }
        iassert(!IS_NAN(sndOrigin[0]) && !IS_NAN(sndOrigin[1]) && !IS_NAN(sndOrigin[2]));
        if ( ci->weaponSpinLerp > snd_minigun_loop_start->current.value )
        {
            if ( v10 )
                AliasId = SND_FindAliasId((char *)weaponDef->spinLoopSoundPlayer);
            else
                AliasId = SND_FindAliasId((char *)weaponDef->spinLoopSound);
            spinLoopId = CG_PlaySoundWithHandle(localClientNum, entityNum, sndOrigin, 0, 0, 1.0, AliasId);
            spinScalar = snd_minigun_pitch_end->current.value - snd_minigun_pitch_start->current.value;
            spinPitch = (float)(ci->weaponSpinLerp * spinScalar) + snd_minigun_pitch_start->current.value;
            if ( (float)(spinPitch - 2.0) < 0.0 )
                v11 = spinPitch;
            else
                v11 = 2.0f;
            if ( (float)(0.1 - spinPitch) < 0.0 )
                pitch = v11;
            else
                pitch = 0.1f;
            SND_SetPlaybackPitch(spinLoopId, pitch);
        }
        if ( (float)(ci->weaponSpinLerp - ci->lastWeaponSpinLerp) > 0.0 && ci->weaponSpinLerp < 0.2 )
        {
            if ( v10 )
                v5 = SND_FindAliasId((char *)weaponDef->startSpinSoundPlayer);
            else
                v5 = SND_FindAliasId((char *)weaponDef->startSpinSound);
            CG_PlaySound(localClientNum, entityNum, sndOrigin, 0, 0, 1.0, v5);
        }
        if ( (float)(ci->weaponSpinLerp - ci->lastWeaponSpinLerp) < 0.0 )
        {
            if ( v10 )
                v6 = SND_FindAliasId((char *)weaponDef->stopSpinSoundPlayer);
            else
                v6 = SND_FindAliasId((char *)weaponDef->stopSpinSound);
            CG_PlaySound(localClientNum, entityNum, sndOrigin, 0, 0, 1.0, v6);
        }
        if ( ci->weaponSpinLerp >= 1.0 && (cent->nextState.lerp.eFlags & 0x40) != 0 )
        {
            ci->minigunWasFiring = 1;
            if ( v10 )
                v7 = SND_FindAliasId((char *)weaponDef->fireLoopSoundPlayer);
            else
                v7 = SND_FindAliasId((char *)weaponDef->fireLoopSound);
            CG_PlaySound(localClientNum, entityNum, sndOrigin, 0, 0, 1.0, v7);
        }
        if ( ci->minigunWasFiring
            && ((float)(ci->weaponSpinLerp - ci->lastWeaponSpinLerp) < 0.0
             || ci->weaponSpinLerp >= 1.0 && (cent->nextState.lerp.eFlags & 0x40) == 0) )
        {
            ci->minigunWasFiring = 0;
            if ( v10 )
                v8 = SND_FindAliasId((char *)weaponDef->fireLoopEndSoundPlayer);
            else
                v8 = SND_FindAliasId((char *)weaponDef->fireLoopEndSound);
            CG_PlaySound(localClientNum, entityNum, sndOrigin, 0, 0, 1.0, v8);
        }
    }
}

void __cdecl CG_UpdateMinigunTags(cg_s *cgameGlob, centity_s *cent, const DObj *obj, int *partBits)
{
    if ( BG_GetWeaponDef(cent->nextState.weapon)->fireType == WEAPON_FIRETYPE_MINIGUN )
        UpdateMinigunTag(cgameGlob, cent, obj, scr_const.j_barrel, 2u, partBits);
}

void __cdecl UpdateMinigunTag(
                cg_s *cgameGlob,
                centity_s *cent,
                const DObj *obj,
                unsigned __int16 name,
                unsigned int axis,
                int *partBits)
{
    float angles[3]; // [esp+0h] [ebp-18h] BYREF
    clientInfo_t *ci; // [esp+Ch] [ebp-Ch]
    int clientNum; // [esp+10h] [ebp-8h]
    unsigned __int8 bone; // [esp+17h] [ebp-1h] BYREF

    if ( axis > 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                    2575,
                    0,
                    "%s",
                    "axis >= 0 && axis < 3") )
    {
        __debugbreak();
    }
    bone = -2;
    if ( DObjGetBoneIndex(obj, name, &bone, -1) )
    {
        clientNum = cent->nextState.clientNum;
        ci = &cgameGlob->bgs.clientinfo[clientNum];
        memset(angles, 0, sizeof(angles));
        angles[axis] = cgameGlob->bgs.clientinfo[clientNum].weaponSpin;
        DObjSetLocalTag(obj, partBits, bone, vec3_origin, angles);
    }
}

void __cdecl CG_UpdateViewModelMinigun(cg_s *cgameGlob, DObj *obj, const WeaponDef *weapDef, centity_s *cent)
{
    int partBits[5]; // [esp+0h] [ebp-14h] BYREF

    if ( weapDef->fireType == WEAPON_FIRETYPE_MINIGUN )
    {
        memset(partBits, 255, sizeof(partBits));
        CG_UpdateMinigunTags(cgameGlob, cent, obj, partBits);
    }
}

void __cdecl CG_UpdateViewModelStackCounter(
                cg_s *cgameGlob,
                const playerState_s *ps,
                DObj *obj,
                const WeaponDef *weapDef)
{
    DWORD v4; // eax
    float v5; // xmm0_4
    float v6; // xmm0_4
    signed int v7; // [esp+0h] [ebp-40h]
    float v8; // [esp+Ch] [ebp-34h]
    float desiredAngle; // [esp+10h] [ebp-30h]
    unsigned __int8 counterBone; // [esp+1Bh] [ebp-25h] BYREF
    float angles[3]; // [esp+1Ch] [ebp-24h] BYREF
    float inc; // [esp+28h] [ebp-18h]
    int partBits[5]; // [esp+2Ch] [ebp-14h] BYREF

    if (weapDef->fireType == WEAPON_FIRETYPE_STACKED && DObjGetBoneIndex(obj, scr_const.j_counter, &counterBone, -1))
    {
        DObjGetHierarchyBits(obj, counterBone, partBits);
        if ((int)(Sys_Milliseconds() - cgameGlob->counterSpinTime) >= 500)
        {
            cgameGlob->counterSpinAngle = cgameGlob->counterSpinTarget;
            if ((signed int)(ps->stackFireCount - 1) > 0)
                v7 = ps->stackFireCount - 1;
            else
                v7 = 0;
            desiredAngle = (float)v7 * 30.0;
            inc = 0.0f;
            if (desiredAngle != cgameGlob->counterSpinTarget)
            {
                cgameGlob->counterSpinTime = Sys_Milliseconds();
                cgameGlob->counterSpinTarget = desiredAngle;
            }
        }
        else
        {
            v4 = Sys_Milliseconds();
            //v5 = __libm_sse2_sin((float)((float)((float)(int)(v4 - cgameGlob->counterSpinTime - 250) / 250.0) * 2.3561945));
            v5 = sin((float)((float)((float)(int)(v4 - cgameGlob->counterSpinTime - 250) / 250.0) * 2.3561945));
            v8 = v5;
            //v6 = __libm_sse2_sin(2.356194496154785);
            v6 = sin(2.356194496154785);
            inc = (float)(cgameGlob->counterSpinTarget - cgameGlob->counterSpinAngle)
                * (float)((float)((float)(v8 / v6) + 1.0) / 2.0);
        }
        angles[0] = cgameGlob->counterSpinAngle + inc;
        angles[1] = 0.0f;
        angles[2] = 0.0f;
        DObjSetControlTagAngles(obj, partBits, counterBone, angles);
    }
}

void __cdecl BulletTrace_Start(
                int localClientNum,
                BulletFireParams *bp,
                unsigned int weaponIndex,
                const centity_s *attacker,
                float *tracerStart,
                bool drawTracer,
                bool isPlayer)
{
    BulletTraceResults br; // [esp+0h] [ebp-58h] BYREF
    bool traceHit; // [esp+57h] [ebp-1h]

    traceHit = 0;
    FireBulletPenetrate(localClientNum, bp, weaponIndex, attacker, tracerStart, drawTracer, isPlayer, &br);
}

void __cdecl FireBulletPenetrate(
                int localClientNum,
                BulletFireParams *bp,
                unsigned int weaponIndex,
                const centity_s *attacker,
                float *tracerStart,
                bool drawTracer,
                bool isPlayer,
                BulletTraceResults *br)
{
    float v8; // [esp+14h] [ebp-268h]
    float v9; // [esp+18h] [ebp-264h]
    float v10; // [esp+28h] [ebp-254h]
    bool v11; // [esp+2Ch] [ebp-250h]
    unsigned __int8 v12; // [esp+47h] [ebp-235h]
    int v13; // [esp+48h] [ebp-234h]
    int v14; // [esp+4Ch] [ebp-230h]
    unsigned int v15; // [esp+50h] [ebp-22Ch]
    int v16; // [esp+54h] [ebp-228h]
    int v17; // [esp+58h] [ebp-224h]
    float v18; // [esp+68h] [ebp-214h]
    unsigned __int8 v19; // [esp+7Bh] [ebp-201h]
    int v20; // [esp+7Ch] [ebp-200h]
    int v21; // [esp+80h] [ebp-1FCh]
    unsigned int v22; // [esp+84h] [ebp-1F8h]
    int v23; // [esp+88h] [ebp-1F4h]
    int v24; // [esp+8Ch] [ebp-1F0h]
    unsigned __int8 v25; // [esp+9Fh] [ebp-1DDh]
    __int16 cflags; // [esp+A0h] [ebp-1DCh]
    int v27; // [esp+A4h] [ebp-1D8h]
    unsigned int v28; // [esp+A8h] [ebp-1D4h]
    int targetEntityNum; // [esp+ACh] [ebp-1D0h]
    int number; // [esp+B0h] [ebp-1CCh]
    float value; // [esp+C0h] [ebp-1BCh]
    float SurfacePenetrationDepth; // [esp+D8h] [ebp-1A4h]
    float v33; // [esp+DCh] [ebp-1A0h]
    unsigned __int8 boneIndex; // [esp+137h] [ebp-145h]
    int hitContents; // [esp+138h] [ebp-144h]
    int damage; // [esp+13Ch] [ebp-140h]
    unsigned int surfType; // [esp+140h] [ebp-13Ch]
    int entityNum; // [esp+144h] [ebp-138h]
    int sourceEntityNum; // [esp+148h] [ebp-134h]
    DObj *obj; // [esp+170h] [ebp-10Ch]
    int partBits[5]; // [esp+174h] [ebp-108h] BYREF
    const centity_s *hitEnt; // [esp+188h] [ebp-F4h]
    centity_s *cent; // [esp+18Ch] [ebp-F0h]
    float wayOutThere[3]; // [esp+190h] [ebp-ECh] BYREF
    bool hit; // [esp+19Fh] [ebp-DDh]
    int actorTeam; // [esp+1A0h] [ebp-DCh]
    BulletTraceResults revBr; // [esp+1A4h] [ebp-D8h] BYREF
    float lastHitPos[3]; // [esp+1F8h] [ebp-84h] BYREF
    float depth; // [esp+204h] [ebp-78h]
    int weapType; // [esp+208h] [ebp-74h]
    int penetrateIndex; // [esp+20Ch] [ebp-70h]
    int tracerType; // [esp+210h] [ebp-6Ch]
    unsigned __int16 traceHitEntityId; // [esp+214h] [ebp-68h]
    unsigned __int8 effectFlags; // [esp+21Bh] [ebp-61h]
    cg_s *cgameGlob; // [esp+21Ch] [ebp-60h]
    int markCount; // [esp+220h] [ebp-5Ch]
    bool allSolid; // [esp+227h] [ebp-55h]
    int max_penetrations; // [esp+228h] [ebp-54h]
    BulletFireParams revBp; // [esp+22Ch] [ebp-50h] BYREF
    clientInfo_t *clientInfo; // [esp+26Ch] [ebp-10h]
    bool isPlayerOnTurret; // [esp+271h] [ebp-Bh]
    bool revTraceHit; // [esp+273h] [ebp-9h]
    float maxDepth; // [esp+274h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+278h] [ebp-4h]
    char traceHita; // [esp+2A4h] [ebp+28h]
    char traceHitb; // [esp+2A4h] [ebp+28h]

    PROF_SCOPED("FireBulletPenetrate");

    if ( !bp && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 2837, 0, "%s", "bp") )
        __debugbreak();
    if ( !attacker
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 2838, 0, "%s", "attacker") )
    {
        __debugbreak();
    }
    weapDef = BG_GetWeaponDef(weaponIndex);
    if ( !weapDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 2841, 0, "%s", "weapDef") )
    {
        __debugbreak();
    }
    traceHita = BulletTrace(localClientNum, bp, weapDef, attacker, br, 0);
    if ( g_debugBullets && g_debugBullets->current.integer >= 2 )
        CG_DebugLine(bp->start, br->hitPos, colorBlue, 1, 300);
    weapType = weapDef->weapType;
    if ( weapType )
        drawTracer = 0;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    ++cgameGlob->weaponTracerCount[attacker->nextState.number];
    tracerType = 2;
    if ( weapDef->tracerFrequency )
    {
        if ( !(cgameGlob->weaponTracerCount[attacker->nextState.number] % weapDef->tracerFrequency) )
        {
            actorTeam = 1;
            if ( attacker->nextState.eType == 1 )
            {
                if ( cgameGlob->nextSnap->ps.clientNum >= 0x20u
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                                2881,
                                0,
                                "cgameGlob->nextSnap->ps.clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                                cgameGlob->nextSnap->ps.clientNum,
                                32) )
                {
                    __debugbreak();
                }
                if ( attacker->nextState.number >= 0x20u
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                                2882,
                                0,
                                "attacker->nextState.number doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                                attacker->nextState.number,
                                32) )
                {
                    __debugbreak();
                }
                clientInfo = &cgameGlob->bgs.clientinfo[attacker->nextState.number];
                actorTeam = clientInfo->team;
            }
            clientInfo = &cgameGlob->bgs.clientinfo[cgameGlob->nextSnap->ps.clientNum];
            if ( actorTeam == clientInfo->team )
                actorTeam = 1;
            else
                actorTeam = 2;
            tracerType = actorTeam == 1;
        }
    }
    else
    {
        drawTracer = 0;
    }
    if ( !traceHita )
    {
        if ( drawTracer )
            CG_SpawnTracer(localClientNum, tracerStart, bp->end, tracerType, weapDef->tracerWidth, weapDef->tracerLength);
        hit = DynEntCl_DynEntImpactEvent(localClientNum, attacker->nextState.number, bp->start, bp->end, weapDef->damage, 0);
        if ( !isPlayer && !hit )
        {
            wayOutThere[0] = bp->dir[0];
            wayOutThere[1] = bp->dir[1];
            wayOutThere[2] = bp->dir[2];
            wayOutThere[0] = 10000.0 * wayOutThere[0];
            wayOutThere[1] = 10000.0 * wayOutThere[1];
            wayOutThere[2] = 10000.0 * wayOutThere[2];
            wayOutThere[0] = wayOutThere[0] + bp->start[0];
            wayOutThere[1] = wayOutThere[1] + bp->start[1];
            wayOutThere[2] = wayOutThere[2] + bp->start[2];
            CG_SoundWhizby(localClientNum, weapDef, bp->start, bp->dir, wayOutThere);
        }
        return;
    }
    traceHitEntityId = Trace_GetEntityHitId(&br->trace);
    if ( drawTracer )
        CG_SpawnTracer(localClientNum, tracerStart, br->hitPos, tracerType, weapDef->tracerWidth, weapDef->tracerLength);
    isPlayerOnTurret = cgameGlob->predictedPlayerState.viewlocked_entNum == attacker->nextState.number;
    if ( !isPlayer && !isPlayerOnTurret )
        CG_SoundWhizby(localClientNum, weapDef, bp->start, bp->dir, br->hitPos);
    //if ( sv_hitFXFrustumCutoff && R_CullPoint(br->hitPos, COERCE_FLOAT(sv_hitFXFrustumCutoff->current.integer ^ _mask__NegFloat_)) )
    if ( sv_hitFXFrustumCutoff && R_CullPoint(br->hitPos, -sv_hitFXFrustumCutoff->current.value) )
    {
        return;
    }
    effectFlags = 0;
    if ( !weapType || attacker && attacker->pose.eType == 14 && weapType == 2 )
    {
        if ( br->trace.partGroup == 2 )
        {
            effectFlags |= 1u;
        }
        else if ( traceHitEntityId != 1022 )
        {
            hitEnt = CG_GetEntity(localClientNum, traceHitEntityId);
            if ( hitEnt && hitEnt->nextState.eType == 1 && cg_debugLocHit->current.integer > 1 )
            {
                memset(partBits, 255, sizeof(partBits));
                obj = Com_GetClientDObj(hitEnt->nextState.number, localClientNum);
                CG_DObjCalcPose(&hitEnt->pose, obj, partBits);
                CG_XModelDebugBoxes(localClientNum, hitEnt, colorCyan, 0, 3 * cg_debugLocHitTime->current.integer);
                CG_DebugLine(bp->start, br->hitPos, colorLtGreen, 0, 3 * cg_debugLocHitTime->current.integer);
                G_DebugStar(br->hitPos, colorRedFaded, 3 * cg_debugLocHitTime->current.integer);
            }
            if ( hitEnt && hitEnt->nextState.eType == 14 && hitEnt->destructible )
                effectFlags |= RETURN_ZERO32();
        }
        if ( traceHitEntityId != 1022 )
            DynEntCl_EntityImpactEvent(&br->trace, localClientNum, attacker->nextState.number, bp->start, br->hitPos, 0);
        DynEntCl_DynEntImpactEvent(localClientNum, attacker->nextState.number, bp->start, br->hitPos, weapDef->damage, 0);
        boneIndex = br->trace.boneIndex;
        hitContents = br->trace.cflags;
        damage = weapDef->damage;
        surfType = (unsigned __int8)((int)(0x3F00000 & br->trace.sflags) >> 20);
        entityNum = traceHitEntityId;
        sourceEntityNum = attacker->nextState.number;
        if ( !sv_clientSideBullets->current.enabled || !IsEntityNotDoingClientSideBullets(localClientNum, traceHitEntityId) )
            CG_BulletHitEvent(
                localClientNum,
                sourceEntityNum,
                entityNum,
                weaponIndex,
                bp->start,
                br->hitPos,
                br->trace.normal.vec.v,
                br->trace.normal.vec.v,
                surfType,
                51,
                effectFlags,
                damage,
                hitContents,
                boneIndex);
        cent = CG_GetEntity(localClientNum, traceHitEntityId);
        if ( cent->destructible )
            CG_DestructibleDamage(
                cent,
                attacker,
                bp->dir,
                br->hitPos,
                weapDef->damage,
                bp->methodOfDeath,
                br->trace.modelIndex,
                br->trace.partName,
                1);
        if ( (unsigned __int8)((int)(0x3F00000 & br->trace.sflags) >> 20) == 20 )
            effectFlags |= 8u;
    }
    if ( weapDef->penetrateType == PENETRATE_TYPE_NONE || br->trace.startsolid )
    {
        return;
    }
    if ( weapDef->bBulletImpactExplode )
    {
        return;
    }
    max_penetrations = sv_penetrationCount->current.integer;
    markCount = 0;
    for ( penetrateIndex = 0; penetrateIndex < max_penetrations; ++penetrateIndex )
    {
        maxDepth = BG_GetSurfacePenetrationDepth(weapDef, br->depthSurfaceType);
        if ( attacker->nextState.eType == 1
            && (cgameGlob->bgs.clientinfo[attacker->nextState.clientNum].perks[0] & 0x20) != 0 )
        {
            maxDepth = maxDepth * perk_bulletPenetrationMultiplier->current.value;
        }
        if ( maxDepth <= 0.0 )
            break;
        lastHitPos[0] = br->hitPos[0];
        lastHitPos[1] = br->hitPos[1];
        lastHitPos[2] = br->hitPos[2];
        if ( !BG_AdvanceTrace(bp, br, 0.13500001) )
            break;
        traceHitb = BulletTrace(localClientNum, bp, weapDef, attacker, br, br->depthSurfaceType);
        Com_Memcpy(&revBp, bp, sizeof(BulletFireParams));
        revBp.dir[0] = -bp->dir[0];
        revBp.dir[1] = -bp->dir[1];
        revBp.dir[2] = -bp->dir[2];
        revBp.start[0] = bp->end[0];
        revBp.start[1] = bp->end[1];
        revBp.start[2] = bp->end[2];

        Vec3Mad(lastHitPos, 0.0099999998f, revBp.dir, revBp.end);

        Com_Memcpy(&revBr, br, sizeof(BulletTraceResults));

        revBr.trace.normal.vec.v[0] = -revBr.trace.normal.vec.v[0];
        revBr.trace.normal.vec.v[1] = -revBr.trace.normal.vec.v[1];
        revBr.trace.normal.vec.v[2] = -revBr.trace.normal.vec.v[2];
        if ( traceHitb )
            BG_AdvanceTrace(&revBp, &revBr, 0.0099999998);
        revTraceHit = BulletTrace(localClientNum, &revBp, weapDef, attacker, &revBr, revBr.depthSurfaceType);
        v11 = revTraceHit && revBr.trace.allsolid || br->trace.startsolid && revBr.trace.startsolid;
        allSolid = v11;
        if ( revTraceHit || allSolid )
        {
            traceHitEntityId = Trace_GetEntityHitId(&revBr.trace);
            if ( allSolid )
                v10 = Vec3Distance(revBp.start, revBp.end);
            else
                v10 = Vec3Distance(revBr.hitPos, lastHitPos);
            depth = v10;
            if ( v10 < 1.0 )
                depth = 1.0f;
            if ( revTraceHit )
            {
                if ( attacker->nextState.eType == 1
                    && (cgameGlob->bgs.clientinfo[attacker->nextState.clientNum].perks[0] & 0x20) != 0 )
                {
                    v33 = BG_GetSurfacePenetrationDepth(weapDef, revBr.depthSurfaceType)
                            * perk_bulletPenetrationMultiplier->current.value;
                    v9 = (float)(v33 - maxDepth) < 0.0 ? v33 : maxDepth;
                    maxDepth = v9;
                }
                else
                {
                    SurfacePenetrationDepth = BG_GetSurfacePenetrationDepth(weapDef, revBr.depthSurfaceType);
                    v8 = (float)(SurfacePenetrationDepth - maxDepth) < 0.0 ? SurfacePenetrationDepth : maxDepth;
                    maxDepth = v8;
                }
                if ( maxDepth <= 0.0 )
                    break;
            }
            if ( attacker->nextState.eType != 1
                || (cgameGlob->bgs.clientinfo[attacker->nextState.clientNum].perks[0] & 0x20) == 0 )
            {
                bp->damageMultiplier = bp->damageMultiplier - (float)(depth / maxDepth);
            }
            if ( bp->damageMultiplier <= 0.0 )
                break;
            if ( !allSolid && !weapType )
            {
                value = bullet_penetrationMinFxDist->current.value;
                if ( Vec3DistanceSq(br->hitPos, revBr.hitPos) > value * value )
                {
                    if ( (!traceHitb || (br->trace.sflags & 4) == 0) && markCount < 2 )
                    {
                        v25 = revBr.trace.boneIndex;
                        cflags = revBr.trace.cflags;
                        v27 = weapDef->damage;
                        v28 = (unsigned __int8)((int)(0x3F00000 & revBr.trace.sflags) >> 20);
                        targetEntityNum = traceHitEntityId;
                        number = attacker->nextState.number;
                        if ( !sv_clientSideBullets->current.enabled
                            || !IsEntityNotDoingClientSideBullets(localClientNum, traceHitEntityId) )
                        {
                            CG_BulletHitEvent(
                                localClientNum,
                                number,
                                targetEntityNum,
                                weaponIndex,
                                revBp.start,
                                revBr.hitPos,
                                bp->dir,
                                revBr.trace.normal.vec.v,
                                v28,
                                51,
                                effectFlags | 4,
                                v27,
                                cflags,
                                v25);
                        }
                        ++markCount;
                    }
                    if ( traceHitb )
                    {
                        if ( markCount < 2 )
                        {
                            v19 = br->trace.boneIndex;
                            v20 = br->trace.cflags;
                            v21 = weapDef->damage;
                            v22 = (unsigned __int8)((int)(0x3F00000 & br->trace.sflags) >> 20);
                            v23 = traceHitEntityId;
                            v24 = attacker->nextState.number;
                            if ( !sv_clientSideBullets->current.enabled
                                || !IsEntityNotDoingClientSideBullets(localClientNum, traceHitEntityId) )
                            {
                                CG_BulletHitEvent(
                                    localClientNum,
                                    v24,
                                    v23,
                                    weaponIndex,
                                    bp->start,
                                    br->hitPos,
                                    br->trace.normal.vec.v,
                                    br->trace.normal.vec.v,
                                    v22,
                                    51,
                                    effectFlags,
                                    v21,
                                    v20,
                                    v19);
                            }
                            ++markCount;
                        }
                        DynEntCl_DynEntImpactEvent(
                            localClientNum,
                            attacker->nextState.number,
                            bp->start,
                            br->hitPos,
                            weapDef->damage,
                            0);
                    }
                }
            }
        }
        else if ( traceHitb )
        {
            if ( !br->trace.allsolid )
            {
                v18 = bullet_penetrationMinFxDist->current.value;
                if ( Vec3DistanceSq(br->hitPos, lastHitPos) > v18 * v18 )
                {
                    traceHitEntityId = Trace_GetEntityHitId(&br->trace);
                    if ( !weapType )
                    {
                        if ( markCount < 2 )
                        {
                            v12 = br->trace.boneIndex;
                            v13 = br->trace.cflags;
                            v14 = weapDef->damage;
                            v15 = (unsigned __int8)((int)(0x3F00000 & br->trace.sflags) >> 20);
                            v16 = traceHitEntityId;
                            v17 = attacker->nextState.number;
                            if ( !sv_clientSideBullets->current.enabled
                                || !IsEntityNotDoingClientSideBullets(localClientNum, traceHitEntityId) )
                            {
                                CG_BulletHitEvent(
                                    localClientNum,
                                    v17,
                                    v16,
                                    weaponIndex,
                                    bp->start,
                                    br->hitPos,
                                    br->trace.normal.vec.v,
                                    br->trace.normal.vec.v,
                                    v15,
                                    51,
                                    effectFlags,
                                    v14,
                                    v13,
                                    v12);
                            }
                            ++markCount;
                        }
                        DynEntCl_DynEntImpactEvent(
                            localClientNum,
                            attacker->nextState.number,
                            bp->start,
                            br->hitPos,
                            weapDef->damage,
                            0);
                    }
                }
            }
        }
        if ( !traceHitb )
            break;
    }
}

bool __cdecl ShouldIgnoreHitEntity(int attackerNum, int hitEntNum)
{
    return hitEntNum == attackerNum;
}

bool __cdecl IsEntityNotDoingClientSideBullets(int localClientNum, int entityNum)
{
    centity_s *cent; // [esp+Ch] [ebp-4h]

    cent = CG_GetEntity(localClientNum, entityNum);
    if ( !cent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 2798, 0, "%s", "cent") )
        __debugbreak();
    return cent && (cent->nextState.eType == 1 || cent->nextState.eType == 17);
}

bool __cdecl CG_IsPlayerCrouching(clientInfo_t *ci, const centity_s *cent)
{
    return BG_IsCrouchingAnim(ci, cent->nextState.animState.state);
}

bool __cdecl CG_IsPlayerProne(clientInfo_t *ci, const centity_s *cent)
{
    return BG_IsProneAnim(ci, cent->nextState.animState.state);
}

bool __cdecl CG_IsPlayerADS(clientInfo_t *ci, const centity_s *cent)
{
    return BG_IsAds(ci, cent->nextState.animState.state);
}

void __cdecl CG_GuessSpreadForWeapon(
                int localClientNum,
                const centity_s *cent,
                const WeaponDef *weapDef,
                float *minSpread,
                float *maxSpread)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-8h]
    clientInfo_t *ci; // [esp+4h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cent->nextState.number >= 0x20u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                    3249,
                    0,
                    "cent->nextState.number doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                    cent->nextState.number,
                    32) )
    {
        __debugbreak();
    }
    ci = &cgameGlob->bgs.clientinfo[cent->nextState.number];
    if ( CG_IsPlayerProne(ci, cent) )
    {
        *minSpread = weapDef->fHipSpreadProneMin;
        *maxSpread = weapDef->hipSpreadProneMax;
    }
    else if ( CG_IsPlayerCrouching(ci, cent) )
    {
        *minSpread = weapDef->fHipSpreadDuckedMin;
        *maxSpread = weapDef->hipSpreadDuckedMax;
    }
    else
    {
        *minSpread = weapDef->fHipSpreadStandMin;
        *maxSpread = weapDef->hipSpreadStandMax;
    }
}

char __cdecl CG_GetPlayerViewOrigin(int localClientNum, const playerState_s *ps, float *origin)
{
    cg_s *LocalClientGlobals; // eax
    float v5; // [esp+10h] [ebp-C4h]
    float v6; // [esp+18h] [ebp-BCh]
    float v7; // [esp+1Ch] [ebp-B8h]
    float v8; // [esp+20h] [ebp-B4h]
    float viewHeightCurrent; // [esp+2Ch] [ebp-A8h]
    float v10; // [esp+44h] [ebp-90h]
    float v11; // [esp+4Ch] [ebp-88h]
    float v12; // [esp+50h] [ebp-84h]
    DObj *ClientDObj; // [esp+60h] [ebp-74h]
    float playerMtx[4][3]; // [esp+64h] [ebp-70h] BYREF
    DObj *obj; // [esp+94h] [ebp-40h]
    int weapon; // [esp+98h] [ebp-3Ch]
    centity_s *attachedEnt; // [esp+9Ch] [ebp-38h]
    float viewAxis[3][3]; // [esp+A0h] [ebp-34h] BYREF
    float offsetVec[3]; // [esp+C4h] [ebp-10h] BYREF
    const WeaponDef *weapDef; // [esp+D0h] [ebp-4h]

    if ( (ps->eFlags & 0x300) != 0 )
    {
        if ( ps->viewlocked == PLAYERVIEWLOCK_NONE
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 3381, 0, "%s", "ps->viewlocked") )
        {
            __debugbreak();
        }
        if ( ps->viewlocked_entNum == 1023
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                        3382,
                        0,
                        "%s",
                        "ps->viewlocked_entNum != ENTITYNUM_NONE") )
        {
            __debugbreak();
        }
        attachedEnt = CG_GetEntity(localClientNum, ps->viewlocked_entNum);
        obj = Com_GetClientDObj(attachedEnt->nextState.number, localClientNum);
        if ( !obj )
            Com_Error(ERR_DROP, "CG_GetPlayerViewOrigin: Unable to get DObj for turret entity %i", attachedEnt->nextState.number);
        if ( CG_DObjGetWorldTagPos(&attachedEnt->pose, obj, scr_const.tag_player, origin) )
        {
            weapon = CG_GetPlayerWeapon(ps, localClientNum);
            weapDef = BG_GetWeaponDef(weapon);
            if ( weapDef )
            {
                if ( weapon )
                {
                    BG_CalcVehicleTurretWeaponPosOffset(ps->fWeaponPosFrac, weapDef, offsetVec);
                    AnglesToAxis(ps->viewangles, viewAxis);
                    v12 = offsetVec[0];
                    *origin = (float)(offsetVec[0] * viewAxis[0][0]) + *origin;
                    origin[1] = (float)(v12 * viewAxis[0][1]) + origin[1];
                    origin[2] = (float)(v12 * viewAxis[0][2]) + origin[2];
                    v11 = offsetVec[1];
                    *origin = (float)(offsetVec[1] * viewAxis[1][0]) + *origin;
                    origin[1] = (float)(v11 * viewAxis[1][1]) + origin[1];
                    origin[2] = (float)(v11 * viewAxis[1][2]) + origin[2];
                    v10 = offsetVec[2];
                    *origin = (float)(offsetVec[2] * viewAxis[2][0]) + *origin;
                    origin[1] = (float)(v10 * viewAxis[2][1]) + origin[1];
                    origin[2] = (float)(v10 * viewAxis[2][2]) + origin[2];
                }
            }
            return 1;
        }
        else
        {
            Com_Error(ERR_DROP, "CG_GetPlayerViewOrigin: Couldn't find [tag_player] on turret");
            *origin = ps->origin[0];
            origin[1] = ps->origin[1];
            origin[2] = ps->origin[2];
            origin[2] = origin[2] + 8.0;
            return 0;
        }
    }
    else if ( (ps->eFlags & 0x4000) == 0 || ps->vehiclePos )
    {
        LocalClientGlobals = CG_GetLocalClientGlobals(localClientNum);
        BG_GetPlayerViewOrigin(ps, origin, LocalClientGlobals->time);
        return 1;
    }
    else
    {
        if ( ps->viewlocked_entNum == 1023
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                        3417,
                        0,
                        "%s",
                        "ps->viewlocked_entNum != ENTITYNUM_NONE") )
        {
            __debugbreak();
        }
        attachedEnt = CG_GetEntity(localClientNum, ps->viewlocked_entNum);
        ClientDObj = Com_GetClientDObj(attachedEnt->nextState.number, localClientNum);
        if ( CG_DObjGetWorldTagMatrix(&attachedEnt->pose, ClientDObj, scr_const.tag_player, playerMtx, playerMtx[3]) )
        {
            weapDef = BG_GetWeaponDef(attachedEnt->nextState.weapon);
            *origin = playerMtx[3][0];
            origin[1] = playerMtx[3][1];
            origin[2] = playerMtx[3][2];
            viewHeightCurrent = ps->viewHeightCurrent;
            *origin = (float)(viewHeightCurrent * playerMtx[2][0]) + *origin;
            origin[1] = (float)(viewHeightCurrent * playerMtx[2][1]) + origin[1];
            origin[2] = (float)(viewHeightCurrent * playerMtx[2][2]) + origin[2];
            memset(offsetVec, 0, sizeof(offsetVec));
            if ( weapDef && attachedEnt->nextState.weapon )
                BG_CalcVehicleTurretWeaponPosOffset(ps->fWeaponPosFrac, weapDef, offsetVec);
            if ( fabs(offsetVec[0]) <= 200.0 )
            {
                v7 = offsetVec[0];
                *origin = (float)(offsetVec[0] * playerMtx[0][0]) + *origin;
                origin[1] = (float)(v7 * playerMtx[0][1]) + origin[1];
                origin[2] = (float)(v7 * playerMtx[0][2]) + origin[2];
            }
            else
            {
                AnglesToAxis(ps->viewangles, viewAxis);
                v8 = offsetVec[0];
                *origin = (float)(offsetVec[0] * viewAxis[0][0]) + *origin;
                origin[1] = (float)(v8 * viewAxis[0][1]) + origin[1];
                origin[2] = (float)(v8 * viewAxis[0][2]) + origin[2];
            }
            v6 = offsetVec[1];
            *origin = (float)(offsetVec[1] * playerMtx[1][0]) + *origin;
            origin[1] = (float)(v6 * playerMtx[1][1]) + origin[1];
            origin[2] = (float)(v6 * playerMtx[1][2]) + origin[2];
            v5 = offsetVec[2];
            *origin = (float)(offsetVec[2] * playerMtx[2][0]) + *origin;
            origin[1] = (float)(v5 * playerMtx[2][1]) + origin[1];
            origin[2] = (float)(v5 * playerMtx[2][2]) + origin[2];
            return 1;
        }
        else
        {
            Com_Error(ERR_DROP, "CG_GetPlayerViewOrigin: Couldn't find [tag_player] on vehicle");
            *origin = ps->origin[0];
            origin[1] = ps->origin[1];
            origin[2] = ps->origin[2];
            origin[2] = origin[2] + 8.0;
            return 0;
        }
    }
}

cg_s *__cdecl CG_GetLocalClientCGameGlobFromEntNum(int entnumber)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-8h]
    int localClientNum; // [esp+4h] [ebp-4h]

    for ( localClientNum = 0; localClientNum < 1; ++localClientNum )
    {
        if ( CL_LocalClient_IsActive(localClientNum) )
        {
            cgameGlob = CG_GetLocalClientGlobals(localClientNum);
            if ( cgameGlob->nextSnap )
            {
                if ( cgameGlob->nextSnap->ps.clientNum == entnumber )
                    return cgameGlob;
            }
        }
    }
    return 0;
}

void __cdecl CG_AddPlayerWeapon(
                int localClientNum,
                const GfxScaledPlacement *placement,
                const playerState_s *ps,
                centity_s *cent,
                int bDrawGun)
{
    int v5; // eax
    int v6; // eax
    int v7; // eax
    unsigned int fLeanDist; // [esp+24h] [ebp-68h]
    unsigned int fLeanDista; // [esp+24h] [ebp-68h]
    unsigned int fLeanDistb; // [esp+24h] [ebp-68h]
    bool v12; // [esp+2Ch] [ebp-60h]
    const WeaponVariantDef *weapVarDef; // [esp+30h] [ebp-5Ch]
    snapshot_s *nextSnap; // [esp+34h] [ebp-58h]
    float *v15; // [esp+3Ch] [ebp-50h]
    float pos[3]; // [esp+44h] [ebp-48h] BYREF
    DObj *obj; // [esp+50h] [ebp-3Ch]
    ShaderConstantSet *constantSet; // [esp+54h] [ebp-38h]
    int textureOverrideIndex; // [esp+58h] [ebp-34h]
    bool weaponLaserOn; // [esp+5Fh] [ebp-2Dh]
    const WeaponVariantDef *primaryWeaponDef; // [esp+60h] [ebp-2Ch]
    unsigned int renderFxFlags; // [esp+64h] [ebp-28h]
    int isInGuidedMissile; // [esp+68h] [ebp-24h]
    int weaponNum; // [esp+6Ch] [ebp-20h]
    cg_s *cgameGlob; // [esp+70h] [ebp-1Ch]
    int bViewModel; // [esp+74h] [ebp-18h]
    const ViewModelInfo *viewModelInfo; // [esp+78h] [ebp-14h]
    float lightingOrigin[3]; // [esp+7Ch] [ebp-10h] BYREF
    const WeaponDef *weapDef; // [esp+88h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    bViewModel = ps != 0;
    if ( ps )
        weaponNum = BG_GetViewmodelWeaponIndex(ps);
    else
        weaponNum = cent->nextState.weapon;
    if ( weaponNum > 0
        && (cent->nextState.lerp.eFlags & 0x300) == 0
        && !BG_UsingVehicleWeapon(ps)
        && (!ps || ps->vehicleType != 3) )
    {
        weapDef = BG_GetWeaponDef(weaponNum);
        if ( bViewModel )
        {
            viewModelInfo = CG_GetLocalClientViewModelInfo(localClientNum);
            if ( !viewModelInfo->viewModelDObj
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                            3877,
                            0,
                            "%s",
                            "viewModelInfo->viewModelDObj") )
            {
                __debugbreak();
            }
            UnitQuatToAxis(placement->base.quat, cgameGlob->viewModelAxis);
            v15 = cgameGlob->viewModelAxis[3];
            cgameGlob->viewModelAxis[3][0] = placement->base.origin[0];
            v15[1] = placement->base.origin[1];
            v15[2] = placement->base.origin[2];
            if ( !weapDef
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 3882, 0, "%s", "weapDef") )
            {
                __debugbreak();
            }
            if ( weapDef->weapClass == WEAPCLASS_GAS && bViewModel )
                CG_Flame_Update_ViewModel(localClientNum, cent);
            CG_UpdateViewModelPose(viewModelInfo->viewModelDObj, localClientNum);
            CG_UpdateViewModelMinigun(cgameGlob, viewModelInfo->viewModelDObj, weapDef, cent);
            CG_UpdateViewModelStackCounter(cgameGlob, ps, viewModelInfo->viewModelDObj, weapDef);
            isInGuidedMissile = CG_IsInGuidedMissile(ps);
            if ( bDrawGun && !isInGuidedMissile )
            {
                lightingOrigin[0] = ps->origin[0];
                lightingOrigin[1] = ps->origin[1];
                lightingOrigin[2] = ps->origin[2];
                lightingOrigin[2] = lightingOrigin[2] + ps->viewHeightCurrent;
                AddLeanToPosition(lightingOrigin, ps->viewangles[1], ps->leanf, 16.0, 20.0);
                constantSet = &cent->pose.constantSet;
                if ( weapDef->inventoryType == WEAPINVENTORY_ALTMODE )
                    weapVarDef = BG_GetWeaponVariantDef(ps->lastWeaponAltModeSwitch);
                else
                    weapVarDef = BG_GetWeaponVariantDef(weaponNum);
                primaryWeaponDef = weapVarDef;
                textureOverrideIndex = CG_SetupWeaponOptionsRender(
                                                                 localClientNum,
                                                                 0,
                                                                 weapVarDef,
                                                                 cent->nextState.renderOptions,
                                                                 constantSet,
                                                                 0);
                renderFxFlags = 4194307;
                R_AddDObjToScene(
                    viewModelInfo->viewModelDObj,
                    &cgameGlob->viewModelPose,
                    0x3FFu,
                    0x400003u,
                    lightingOrigin,
                    0.0,
                    0.0,
                    0.0,
                    0.0,
                    -1,
                    textureOverrideIndex,
                    constantSet,
                    0,
                    0.0,
                    1.0);
                v12 = CG_LookingThroughNightVision(localClientNum) && weapDef->laserSightDuringNightvision;
                weaponLaserOn = v12;
                if ( cg_laserForceOn->current.enabled || weaponLaserOn )
                    CG_Laser_Add(
                        cent,
                        viewModelInfo->viewModelDObj,
                        &cgameGlob->viewModelPose,
                        cgameGlob->refdef.vieworg,
                        LASER_OWNER_PLAYER);
                cgameGlob->refdef.dof.viewModelStart = (float)((float)(weapDef->adsDofStart - ps->dofViewmodelStart)
                                                                                                         * ps->fWeaponPosFrac)
                                                                                         + ps->dofViewmodelStart;
                cgameGlob->refdef.dof.viewModelEnd = (float)((float)(weapDef->adsDofEnd - ps->dofViewmodelEnd)
                                                                                                     * ps->fWeaponPosFrac)
                                                                                     + ps->dofViewmodelEnd;
            }
            HoldBreathUpdate(localClientNum);
        }
        if ( CG_ShowMuzzleFlash(cgameGlob, cent, bViewModel) )
        {
            if ( bDrawGun )
            {
                if ( bViewModel )
                {
                    if ( weapDef->bDualWield )
                    {
                        if ( ps->weaponstate == 6
                            || ps->weaponstate == 31
                            || ps->weaponstate == 41 && ((*((unsigned int *)cent + 201) >> 2) & 1) != 0 )
                        {
                            fLeanDist = scr_const.tag_flash;
                            v5 = CG_WeaponDObjHandle(localClientNum);
                            WeaponFlash(localClientNum, v5, weaponNum, 1, fLeanDist);
                        }
                        if ( ps->weaponstateLeft == 6
                            || ps->weaponstateLeft == 31
                            || ps->weaponstateLeft == 41 && ((*((unsigned int *)cent + 201) >> 3) & 1) != 0 )
                        {
                            fLeanDista = scr_const.tag_flash1;
                            v6 = CG_WeaponDObjHandle(localClientNum);
                            WeaponFlash(localClientNum, v6, weaponNum, 1, fLeanDista);
                        }
                    }
                    else
                    {
                        fLeanDistb = scr_const.tag_flash;
                        v7 = CG_WeaponDObjHandle(localClientNum);
                        WeaponFlash(localClientNum, v7, weaponNum, 1, fLeanDistb);
                    }
                }
                else
                {
                    nextSnap = cgameGlob->nextSnap;
                    if ( (nextSnap->ps.otherFlags & 6) != 0 && cent->nextState.number == nextSnap->ps.clientNum )
                    {
                        obj = Com_GetClientDObj(cent->nextState.number, localClientNum);
                        if ( obj )
                            CG_DObjGetWorldTagPos(&cgameGlob->predictedPlayerEntity.pose, obj, scr_const.tag_flash, pos);
                    }
                    if ( weapDef->bDualWield && ((*((unsigned int *)cent + 201) >> 3) & 1) != 0 )
                        WeaponFlash(localClientNum, cent->nextState.number, weaponNum, 0, scr_const.tag_flash1);
                    if ( ((*((unsigned int *)cent + 201) >> 2) & 1) != 0 && weapDef->bUseAltTagFlash )
                    {
                        WeaponFlash(localClientNum, cent->nextState.number, weaponNum, 0, scr_const.tag_flash_alt);
                    }
                    else if ( ((*((unsigned int *)cent + 201) >> 2) & 1) != 0 )
                    {
                        WeaponFlash(localClientNum, cent->nextState.number, weaponNum, 0, scr_const.tag_flash);
                    }
                }
            }
            *((unsigned int *)cent + 201) &= ~4u;
            *((unsigned int *)cent + 201) &= ~8u;
        }
        Weapon_DrawTagAxis(localClientNum, cent->nextState.number, weaponNum, bViewModel);
        if ( !bViewModel )
            Anim_DrawTags(localClientNum, cent->nextState.number, weaponNum, &cent->pose, cent->currentState.eFlags);
    }
}

void __cdecl HoldBreathUpdate(int localClientNum)
{
    int v1; // eax
    int ClientNumForLocalClient; // eax
    int v3; // eax
    int v4; // eax
    unsigned int deltaTime; // [esp+Ch] [ebp-Ch]
    unsigned int deltaTimea; // [esp+Ch] [ebp-Ch]
    unsigned int deltaTimeb; // [esp+Ch] [ebp-Ch]
    unsigned int deltaTimec; // [esp+Ch] [ebp-Ch]
    cg_s *cgameGlob; // [esp+10h] [ebp-8h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->holdBreathDelay > 0 )
        cgameGlob->holdBreathDelay -= cgameGlob->frametime;
    if ( (cgameGlob->predictedPlayerState.weapFlags & 4) != 0 )
    {
        cgameGlob->holdBreathFrac = DiffTrack(
                                                                    1.0,
                                                                    cgameGlob->holdBreathFrac,
                                                                    player_breath_snd_lerp->current.value,
                                                                    (float)cgameGlob->frametime * 0.001);
        if ( cgameGlob->holdBreathTime >= 0 )
        {
            if ( cgameGlob->holdBreathTime > cgameGlob->holdBreathInTime )
            {
                deltaTimea = cgMedia.playerHeartBeatSound;
                ClientNumForLocalClient = CG_GetClientNumForLocalClient(localClientNum);
                CG_PlaySound(localClientNum, ClientNumForLocalClient, 0, 0, 0, 1.0, deltaTimea);
            }
        }
        else
        {
            cgameGlob->holdBreathTime = 0;
            if ( cgameGlob->holdBreathDelay > 0 )
            {
                cgameGlob->holdBreathInTime = 0;
            }
            else
            {
                deltaTime = cgMedia.playerBreathInSound;
                v1 = CG_GetClientNumForLocalClient(localClientNum);
                CG_PlaySound(localClientNum, v1, 0, 0, 0, 1.0, deltaTime);
                cgameGlob->holdBreathInTime = 1000;
                cgameGlob->holdBreathDelay = (int)(float)(player_breath_snd_delay->current.value * 1000.0);
            }
        }
        cgameGlob->holdBreathTime += cgameGlob->frametime;
    }
    else
    {
        if ( cgameGlob->holdBreathTime >= 0 )
        {
            cgameGlob->holdBreathTime += cgameGlob->frametime;
            if ( cgameGlob->holdBreathTime <= (int)(float)(player_breath_hold_time->current.value * 1000.0) )
            {
                if ( cgameGlob->holdBreathDelay <= 0 )
                {
                    deltaTimec = cgMedia.playerBreathOutSound;
                    v4 = CG_GetClientNumForLocalClient(localClientNum);
                    CG_PlaySound(localClientNum, v4, 0, 0, 0, 1.0, deltaTimec);
                    cgameGlob->holdBreathDelay = (int)(float)(player_breath_snd_delay->current.value * 1000.0);
                }
            }
            else
            {
                deltaTimeb = cgMedia.playerBreathGaspSound;
                v3 = CG_GetClientNumForLocalClient(localClientNum);
                CG_PlaySound(localClientNum, v3, 0, 0, 0, 1.0, deltaTimeb);
            }
        }
        cgameGlob->holdBreathTime = -1;
        cgameGlob->holdBreathInTime = 0;
        cgameGlob->holdBreathFrac = 0.0f;
    }
    HoldBreathSoundLerp(localClientNum, cgameGlob->holdBreathFrac);
}

void __cdecl HoldBreathSoundLerp(int localClientNum, float lerp)
{
    if ( lerp == 0.0 )
        SND_SetSnapshot(SND_SNAPSHOT_BREATH, "default", 0.0, 1.0);
    else
        SND_SetSnapshot(SND_SNAPSHOT_BREATH, "hold_breath", 0.0, 1.0);
}

bool __cdecl CG_ShowMuzzleFlash(cg_s *cgameGlob, centity_s *cent, int bViewModel)
{
    bool v4; // [esp+0h] [ebp-8h]
    snapshot_s *nextSnap; // [esp+4h] [ebp-4h]

    if ( !cgameGlob
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 3799, 0, "%s", "cgameGlob") )
    {
        __debugbreak();
    }
    if ( !cent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 3800, 0, "%s", "cent") )
        __debugbreak();
    if ( ((*((unsigned int *)cent + 201) >> 2) & 1) == 0 && ((*((unsigned int *)cent + 201) >> 3) & 1) == 0 )
        return 0;
    if ( bViewModel )
        return 1;
    nextSnap = cgameGlob->nextSnap;
    v4 = (nextSnap->ps.otherFlags & 6) != 0 && cent->nextState.number == nextSnap->ps.clientNum;
    return !v4 || cgameGlob->renderingThirdPerson == TP_FOR_MODEL;
}

void __cdecl Weapon_DrawAxisOrigin(orientation_t orient, float colorScale)
{
    float green[4]; // [esp+Ch] [ebp-6Ch] BYREF
    float arrow[3]; // [esp+1Ch] [ebp-5Ch]
    float blue[4]; // [esp+28h] [ebp-50h] BYREF
    float right[3]; // [esp+38h] [ebp-40h] BYREF
    float red[4]; // [esp+44h] [ebp-34h] BYREF
    float end[3]; // [esp+54h] [ebp-24h] BYREF
    float frontEnd[3]; // [esp+60h] [ebp-18h] BYREF
    float left[4]; // [esp+6Ch] [ebp-Ch] BYREF

    red[0] = colorScale * 1.0;
    red[1] = colorScale * 0.0;
    red[2] = colorScale * 0.0;
    red[3] = colorScale * 1.0;
    green[0] = colorScale * 0.0;
    green[1] = colorScale * 1.0;
    green[2] = colorScale * 0.0;
    green[3] = colorScale * 1.0;
    blue[0] = colorScale * 0.0;
    blue[1] = colorScale * 0.0;
    blue[2] = colorScale * 1.0;
    blue[3] = colorScale * 1.0;
    frontEnd[0] = (float)(5.0 * orient.axis[0][0]) + orient.origin[0];
    frontEnd[1] = (float)(5.0 * orient.axis[0][1]) + orient.origin[1];
    frontEnd[2] = (float)(5.0 * orient.axis[0][2]) + orient.origin[2];
    CG_DebugLine(orient.origin, frontEnd, red, 0, 1);
    right[0] = (float)(5.0 * orient.axis[1][0]) + orient.origin[0];
    right[1] = (float)(5.0 * orient.axis[1][1]) + orient.origin[1];
    right[2] = (float)(5.0 * orient.axis[1][2]) + orient.origin[2];
    CG_DebugLine(orient.origin, right, green, 0, 1);
    end[0] = (float)(5.0 * orient.axis[2][0]) + orient.origin[0];
    end[1] = (float)(5.0 * orient.axis[2][1]) + orient.origin[1];
    end[2] = (float)(5.0 * orient.axis[2][2]) + orient.origin[2];
    CG_DebugLine(orient.origin, end, blue, 0, 1);
    left[3] = -1.0 * orient.axis[2][0];
    left[0] = (float)(5.0 * (float)(-1.0 * orient.axis[1][0])) + orient.origin[0];
    left[1] = (float)(5.0 * (float)(-1.0 * orient.axis[1][1])) + orient.origin[1];
    left[2] = (float)(5.0 * (float)(-1.0 * orient.axis[1][2])) + orient.origin[2];
    arrow[0] = (float)(2.5 * right[0]) + frontEnd[0];
    arrow[1] = (float)(2.5 * right[1]) + frontEnd[1];
    arrow[2] = (float)(2.5 * right[2]) + frontEnd[2];
    CG_DebugLine(frontEnd, right, red, 0, 1);
    arrow[0] = (float)(2.5 * left[0]) + frontEnd[0];
    arrow[1] = (float)(2.5 * left[1]) + frontEnd[1];
    arrow[2] = (float)(2.5 * left[2]) + frontEnd[2];
    CG_DebugLine(frontEnd, left, red, 0, 1);
}

void __cdecl Weapon_DrawTagAxis(int localClientNum, int entityNum, unsigned int weaponNum, int bViewModel)
{
    orientation_t v4; // [esp-30h] [ebp-10Ch] BYREF
    int v5; // [esp+Ch] [ebp-D0h]
    orientation_t v6; // [esp+10h] [ebp-CCh] BYREF
    orientation_t v7; // [esp+40h] [ebp-9Ch] BYREF
    orientation_t v8; // [esp+70h] [ebp-6Ch] BYREF
    orientation_t orient; // [esp+A0h] [ebp-3Ch] BYREF
    int dobjHandle; // [esp+D0h] [ebp-Ch]
    unsigned __int8 boneIndex; // [esp+D7h] [ebp-5h] BYREF
    const WeaponDef *weapDef; // [esp+D8h] [ebp-4h]

    if ( fx_draw_weapon_tags->current.enabled )
    {
        if ( bViewModel )
            v5 = CG_WeaponDObjHandle(localClientNum);
        else
            v5 = entityNum;
        dobjHandle = v5;
        boneIndex = -2;
        if ( CG_GetBoneIndex(localClientNum, v5, scr_const.tag_brass, &boneIndex)
            && FX_GetBoneOrientation(localClientNum, dobjHandle, boneIndex, &orient) )
        {
            memcpy(&v4, &orient, sizeof(v4));
            Weapon_DrawAxisOrigin(v4, 1.0);
        }
        if ( CG_GetBoneIndex(localClientNum, dobjHandle, scr_const.tag_flash, &boneIndex)
            && FX_GetBoneOrientation(localClientNum, dobjHandle, boneIndex, &v8) )
        {
            memcpy(&v4, &v8, sizeof(v4));
            Weapon_DrawAxisOrigin(v4, 1.0);
        }
        weapDef = BG_GetWeaponDef(weaponNum);
        if ( weapDef->bDualWield )
        {
            if ( CG_GetBoneIndex(localClientNum, dobjHandle, scr_const.tag_brass1, &boneIndex)
                && FX_GetBoneOrientation(localClientNum, dobjHandle, boneIndex, &v7) )
            {
                memcpy(&v4, &v7, sizeof(v4));
                Weapon_DrawAxisOrigin(v4, 1.0);
            }
            if ( CG_GetBoneIndex(localClientNum, dobjHandle, scr_const.tag_flash1, &boneIndex) )
            {
                if ( FX_GetBoneOrientation(localClientNum, dobjHandle, boneIndex, &v6) )
                {
                    memcpy(&v4, &v6, sizeof(v4));
                    Weapon_DrawAxisOrigin(v4, 1.0);
                }
            }
        }
    }
}

void __cdecl Anim_DrawTags(int localClientNum, int entityNum, int weaponNum, cpose_t *pose, char flags)
{
    orientation_t v5; // [esp-20h] [ebp-1ACh] BYREF
    float *origin; // [esp+1Ch] [ebp-170h]
    float standOffset[3]; // [esp+20h] [ebp-16Ch]
    float proneOffset[3]; // [esp+2Ch] [ebp-160h]
    float crouchOffset[3]; // [esp+38h] [ebp-154h]
    orientation_t v10; // [esp+44h] [ebp-148h] BYREF
    orientation_t v11; // [esp+74h] [ebp-118h] BYREF
    orientation_t v12; // [esp+A4h] [ebp-E8h] BYREF
    orientation_t v13; // [esp+D4h] [ebp-B8h] BYREF
    orientation_t v14; // [esp+104h] [ebp-88h] BYREF
    int dobjHandle; // [esp+134h] [ebp-58h]
    unsigned __int8 boneIndex; // [esp+13Bh] [ebp-51h] BYREF
    float originOffset[3]; // [esp+13Ch] [ebp-50h]
    cg_s *cgameGlob; // [esp+148h] [ebp-44h]
    float colorScale; // [esp+14Ch] [ebp-40h]
    orientation_t orient; // [esp+150h] [ebp-3Ch] BYREF
    float fireOrigin[3]; // [esp+180h] [ebp-Ch]

    if ( cg_drawAnimAttachTags->current.enabled )
    {
        dobjHandle = entityNum;
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        boneIndex = -2;
        colorScale = 1.0f;
        if ( CG_GetBoneIndex(localClientNum, entityNum, scr_const.tag_weapon_left, &boneIndex)
            && FX_GetBoneOrientation(localClientNum, dobjHandle, boneIndex, &v14) )
        {
            memcpy(&v5, &v14, sizeof(v5));
            Weapon_DrawAxisOrigin(v5, colorScale);
            CG_DebugSphere(v14.origin, 1.0, g_TagColors[0], 10, 0, 1);
        }
        if ( CG_GetBoneIndex(localClientNum, dobjHandle, scr_const.tag_weapon_right, &boneIndex)
            && FX_GetBoneOrientation(localClientNum, dobjHandle, boneIndex, &v13) )
        {
            memcpy(&v5, &v13, sizeof(v5));
            Weapon_DrawAxisOrigin(v5, colorScale);
            CG_DebugSphere(v13.origin, 1.0, g_TagColors[1], 10, 0, 1);
        }
        if ( CG_GetBoneIndex(localClientNum, dobjHandle, scr_const.tag_inhand, &boneIndex)
            && FX_GetBoneOrientation(localClientNum, dobjHandle, boneIndex, &v12) )
        {
            memcpy(&v5, &v12, sizeof(v5));
            Weapon_DrawAxisOrigin(v5, colorScale);
            CG_DebugSphere(v12.origin, 1.0, g_TagColors[2], 10, 0, 1);
        }
        if ( CG_GetBoneIndex(localClientNum, dobjHandle, scr_const.tag_stowed_back, &boneIndex)
            && FX_GetBoneOrientation(localClientNum, dobjHandle, boneIndex, &v11) )
        {
            memcpy(&v5, &v11, sizeof(v5));
            Weapon_DrawAxisOrigin(v5, colorScale);
            CG_DebugSphere(v11.origin, 1.0, g_TagColors[3], 10, 0, 1);
        }
        if ( CG_GetBoneIndex(localClientNum, dobjHandle, scr_const.tag_weapon_chest, &boneIndex)
            && FX_GetBoneOrientation(localClientNum, dobjHandle, boneIndex, &v10) )
        {
            memcpy(&v5, &v10, sizeof(v5));
            Weapon_DrawAxisOrigin(v5, colorScale);
            CG_DebugSphere(v10.origin, 1.0, g_TagColors[4], 10, 0, 1);
        }
        if ( (flags & 4) != 0 )
        {
            crouchOffset[0] = 0.0f;
            crouchOffset[1] = 0.0f;
            crouchOffset[2] = 39.0f;
            originOffset[0] = 0.0f;
            originOffset[1] = 0.0f;
            originOffset[2] = 39.0f;
        }
        else if ( (flags & 8) != 0 )
        {
            proneOffset[0] = 0.0f;
            proneOffset[1] = 0.0f;
            proneOffset[2] = 11.0f;
            originOffset[0] = 0.0f;
            originOffset[1] = 0.0f;
            originOffset[2] = 11.0f;
        }
        else
        {
            standOffset[0] = 0.0f;
            standOffset[1] = 0.0f;
            standOffset[2] = 60.0f;
            originOffset[0] = 0.0f;
            originOffset[1] = 0.0f;
            originOffset[2] = 60.0f;
        }
        origin = pose->origin;
        fireOrigin[0] = pose->origin[0] + originOffset[0];
        fireOrigin[1] = pose->origin[1] + originOffset[1];
        fireOrigin[2] = pose->origin[2] + originOffset[2];
        AnglesToAxis(pose->angles, orient.axis);
        orient.origin[0] = fireOrigin[0];
        orient.origin[1] = fireOrigin[1];
        orient.origin[2] = fireOrigin[2];
        memcpy(&v5, &orient, sizeof(v5));
        Weapon_DrawAxisOrigin(v5, colorScale);
        CG_DebugSphere(orient.origin, 1.0, g_TagColors[5], 10, 0, 0);
    }
}

void __cdecl CG_UpdateViewWeaponAnim(int localClientNum, int newPlayerstate)
{
    unsigned __int8 weaponModel; // [esp+17h] [ebp-625h]
    weaponInfo_s *weapInfo; // [esp+18h] [ebp-624h]
    cg_s *cgameGlob; // [esp+20h] [ebp-61Ch]
    ViewModelInfo *viewModelInfo; // [esp+630h] [ebp-Ch]
    int weaponIndex; // [esp+634h] [ebp-8h]
    const playerState_s *ps; // [esp+638h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    ps = &cgameGlob->predictedPlayerState;
    if ( cgameGlob->predictedPlayerState.pm_type < 9 )
    {
        weaponIndex = BG_GetViewmodelWeaponIndex(ps);
        if ( weaponIndex > 0 )
        {
            weaponModel = BG_GetPlayerWeaponModel(ps, weaponIndex);
            CG_RegisterWeapon(localClientNum, weaponIndex);
            viewModelInfo = CG_GetLocalClientViewModelInfo(localClientNum);
            weapInfo = CG_GetLocalClientWeaponInfo(localClientNum, weaponIndex);
            if ( NeedToRebuildViewmodel(localClientNum, weaponIndex, weaponModel, viewModelInfo, weapInfo) )
                ChangeViewmodelDobj(
                    localClientNum,
                    ps,
                    weaponIndex,
                    weaponModel,
                    weapInfo->handModel,
                    weapInfo->gogglesModel,
                    weapInfo->rocketModel,
                    0,
                    0);
            if ( !viewModelInfo->viewModelDObj
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                            4567,
                            0,
                            "%s",
                            "viewModelInfo->viewModelDObj") )
            {
                __debugbreak();
            }
            WeaponRunXModelAnims(localClientNum, ps, viewModelInfo, newPlayerstate);
            if ( !Sys_IsMainThread()
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                            4570,
                            0,
                            "%s",
                            "Sys_IsMainThread()") )
            {
                __debugbreak();
            }

            XAnimClientNotifyList veryLargeNameOfNotifyListToMinimizeContact; // [esp+24h] [ebp-618h] BYREF
            //XAnimClientNotifyList::XAnimClientNotifyList(&veryLargeNameOfNotifyListToMinimizeContact);
            DObjSetClientNotifies(&veryLargeNameOfNotifyListToMinimizeContact);
            DObjUpdateClientInfo(viewModelInfo->viewModelDObj, (float)cgameGlob->frametime * 0.001, 3);
            ProcessWeaponNoteTracks(localClientNum, ps, 1);
            DObjClearClientNotifies();
            //XAnimClientNotifyList::~XAnimClientNotifyList(&veryLargeNameOfNotifyListToMinimizeContact);
        }
    }
    else
    {
        CG_ResetWeaponAnimTrees(localClientNum, ps, newPlayerstate);
        if ( Demo_IsPlaying() )
            cgameGlob->prevViewmodelWeapon = 0;
    }
}

void __cdecl WeaponRunXModelAnims(
                int localClientNum,
                const playerState_s *ps,
                ViewModelInfo *viewModelInfo,
                int newPlayerstate)
{
    bool v4; // [esp+24h] [ebp-40h]
    bool v5; // [esp+28h] [ebp-3Ch]
    bool v6; // [esp+2Ch] [ebp-38h]
    int playedAnim; // [esp+38h] [ebp-2Ch]
    int playedAnima; // [esp+38h] [ebp-2Ch]
    DObj *obj; // [esp+3Ch] [ebp-28h]
    float transitionTime; // [esp+40h] [ebp-24h]
    float transitionTimea; // [esp+40h] [ebp-24h]
    cg_s *cgameGlob; // [esp+48h] [ebp-1Ch]
    const WeaponVariantDef *weapVariantDef; // [esp+50h] [ebp-14h]
    int weaponIndex; // [esp+54h] [ebp-10h]
    signed int i; // [esp+58h] [ebp-Ch]
    signed int ia; // [esp+58h] [ebp-Ch]
    XAnimTree_s *pAnimTree; // [esp+5Ch] [ebp-8h]
    const WeaponDef *weapDef; // [esp+60h] [ebp-4h]

    CG_GetLocalClientGlobals(localClientNum);
    obj = viewModelInfo->viewModelDObj;
    if ( !viewModelInfo->viewModelDObj
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 718, 0, "%s", "obj") )
    {
        __debugbreak();
    }
    pAnimTree = DObjGetTree(obj);
    weaponIndex = BG_GetViewmodelWeaponIndex(ps);
    if ( weaponIndex <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                    724,
                    0,
                    "%s",
                    "weaponIndex > WP_NONE") )
    {
        __debugbreak();
    }
    weapVariantDef = BG_GetWeaponVariantDef(weaponIndex);
    weapDef = BG_GetWeaponDef(weaponIndex);
    if ( weapDef->aimDownSight )
    {
        v6 = ps->weaponstate == 10 && ps->weaponTime - weapDef->iPositionReloadTransTime > 0;
        v5 = (ps->pm_flags & 0x10) != 0 && (ps->weapFlags & 2) == 0;
        v4 = !v6 && v5;
        PlayADSAnim(ps->fWeaponPosFrac, weaponIndex, obj, 65 - v4);
    }
    else if ( **((_BYTE **)weapVariantDef->szXAnims + 65) )
    {
        PlayADSAnim(0.0, weaponIndex, obj, 65);
    }
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    playedAnim = 1;
    if ( weapDef->bDualWield
        && (viewModelInfo->hand[1].iPrevAnim != ps->weapAnimLeft
         || weaponIndex != cgameGlob->prevViewmodelWeapon
         || newPlayerstate) )
    {
        transitionTime = 0.0f;
        switch ( ps->weapAnimLeft & 0xFFFFFBFF )
        {
            case 0u:
            case 1u:
                for ( i = 58; i <= 63; ++i )
                {
                    if ( !XAnimHasFinished(pAnimTree, i)
                        && (viewModelInfo->hand[0].iHandAnimIndex != i
                         || !CheckForValidAnimIDDW(viewModelInfo->hand[0].iHandAnimIndex)) )
                    {
                        viewModelInfo->hand[1].iPrevAnim = -1;
                        if ( (ps->weapAnimLeft & 1) == 0 )
                        {
                            playedAnim = 0;
                            break;
                        }
                        if ( !newPlayerstate )
                            transitionTime = 0.5f;
                    }
                }
                if ( playedAnim )
                {
                    viewModelInfo->hand[1].iHandAnimIndex = 60;
                    StartWeaponAnim(localClientNum, weaponIndex, obj, 60, transitionTime, newPlayerstate);
                }
                break;
            case 2u:
                viewModelInfo->hand[1].iHandAnimIndex = 58;
                StartWeaponAnim(localClientNum, weaponIndex, obj, 58, 0.0, newPlayerstate);
                break;
            case 4u:
                viewModelInfo->hand[1].iHandAnimIndex = 59;
                StartWeaponAnim(localClientNum, weaponIndex, obj, 59, 0.0, newPlayerstate);
                break;
            case 0xFu:
                viewModelInfo->hand[1].iHandAnimIndex = 63;
                StartWeaponAnim(localClientNum, weaponIndex, obj, 63, 0.0, newPlayerstate);
                break;
            case 0x10u:
                viewModelInfo->hand[1].iHandAnimIndex = 62;
                StartWeaponAnim(localClientNum, weaponIndex, obj, 62, 0.0, newPlayerstate);
                break;
            default:
                viewModelInfo->hand[1].iHandAnimIndex = 66;
                break;
        }
        if ( weapDef->bDualWield || !playedAnim )
        {
            if ( playedAnim )
                viewModelInfo->hand[1].iPrevAnim = ps->weapAnimLeft;
        }
        else
        {
            viewModelInfo->hand[1].iPrevAnim = 66;
        }
    }
    playedAnima = 1;
    if ( ps->weapAnim != viewModelInfo->hand[0].iPrevAnim
        || weaponIndex != cgameGlob->prevViewmodelWeapon
        || newPlayerstate )
    {
        transitionTimea = 0.0f;
        switch ( ps->weapAnim & 0xFFFFFBFF )
        {
            case 0u:
            case 1u:
                for ( ia = 1; ia < 64; ++ia )
                {
                    if ( !XAnimHasFinished(pAnimTree, ia) && viewModelInfo->hand[1].iHandAnimIndex != ia )
                    {
                        viewModelInfo->hand[0].iPrevAnim = -1;
                        if ( (ps->weapAnim & 1) == 0 )
                        {
                            cgameGlob->prevViewmodelWeapon = weaponIndex;
                            playedAnima = 0;
                            break;
                        }
                        if ( !newPlayerstate )
                            transitionTimea = 0.5f;
                        if ( (ps->perks[0] & 0x80000) != 0 && (ps->weapAnim & 1) != 0 )
                            transitionTimea = transitionTimea * perk_weapSwitchMultiplier->current.value;
                    }
                }
                if ( playedAnima )
                    goto LABEL_68;
                break;
            case 2u:
                viewModelInfo->hand[0].iHandAnimIndex = 3;
                StartWeaponAnim(localClientNum, weaponIndex, obj, 3, 0.0, newPlayerstate);
                break;
            case 3u:
                viewModelInfo->hand[1].iHandAnimIndex = 58;
                StartWeaponAnim(localClientNum, weaponIndex, obj, 58, 0.0, newPlayerstate);
                break;
            case 4u:
                viewModelInfo->hand[0].iHandAnimIndex = 5;
                StartWeaponAnim(localClientNum, weaponIndex, obj, 5, 0.0, newPlayerstate);
                break;
            case 5u:
                viewModelInfo->hand[1].iHandAnimIndex = 59;
                StartWeaponAnim(localClientNum, weaponIndex, obj, 59, 0.0, newPlayerstate);
                break;
            case 6u:
                viewModelInfo->hand[0].iHandAnimIndex = 6;
                StartWeaponAnim(localClientNum, weaponIndex, obj, 6, 0.0, newPlayerstate);
                break;
            case 7u:
                viewModelInfo->hand[0].iHandAnimIndex = 42;
                StartWeaponAnim(localClientNum, weaponIndex, obj, 42, 0.0, newPlayerstate);
                break;
            case 8u:
                viewModelInfo->hand[0].iHandAnimIndex = 43;
                StartWeaponAnim(localClientNum, weaponIndex, obj, 43, 0.0, newPlayerstate);
                break;
            case 9u:
                viewModelInfo->hand[0].iHandAnimIndex = 44;
                StartWeaponAnim(localClientNum, weaponIndex, obj, 44, 0.0, newPlayerstate);
                break;
            case 0xAu:
                viewModelInfo->hand[0].iHandAnimIndex = 7;
                StartWeaponAnim(localClientNum, weaponIndex, obj, 7, 0.0, newPlayerstate);
                break;
            case 0xBu:
                viewModelInfo->hand[0].iHandAnimIndex = 8;
                StartWeaponAnim(localClientNum, weaponIndex, obj, 8, 0.0, newPlayerstate);
                break;
            case 0xCu:
                viewModelInfo->hand[0].iHandAnimIndex = 18;
                StartWeaponAnim(localClientNum, weaponIndex, obj, 18, 0.0, newPlayerstate);
                break;
            case 0xDu:
                viewModelInfo->hand[0].iHandAnimIndex = 16;
                StartWeaponAnim(localClientNum, weaponIndex, obj, 16, 0.0, newPlayerstate);
                break;
            case 0xEu:
                viewModelInfo->hand[0].iHandAnimIndex = 17;
                StartWeaponAnim(localClientNum, weaponIndex, obj, 17, 0.0, newPlayerstate);
                break;
            case 0xFu:
                viewModelInfo->hand[0].iHandAnimIndex = 9;
                StartWeaponAnim(localClientNum, weaponIndex, obj, 9, 0.0, newPlayerstate);
                break;
            case 0x10u:
                viewModelInfo->hand[0].iHandAnimIndex = 11;
                StartWeaponAnim(localClientNum, weaponIndex, obj, 11, 0.0, newPlayerstate);
                break;
            case 0x11u:
                viewModelInfo->hand[0].iHandAnimIndex = 12;
                StartWeaponAnim(localClientNum, weaponIndex, obj, 12, 0.0, newPlayerstate);
                break;
            case 0x12u:
                viewModelInfo->hand[0].iHandAnimIndex = 13;
                StartWeaponAnim(localClientNum, weaponIndex, obj, 13, 0.0, newPlayerstate);
                break;
            case 0x13u:
                viewModelInfo->hand[0].iHandAnimIndex = 14;
                StartWeaponAnim(localClientNum, weaponIndex, obj, 14, 0.0, newPlayerstate);
                break;
            case 0x14u:
                viewModelInfo->hand[0].iHandAnimIndex = 15;
                StartWeaponAnim(localClientNum, weaponIndex, obj, 15, 0.0, newPlayerstate);
                break;
            case 0x15u:
                viewModelInfo->hand[0].iHandAnimIndex = 20;
                StartWeaponAnim(localClientNum, weaponIndex, obj, 20, 0.0, newPlayerstate);
                break;
            case 0x16u:
                viewModelInfo->hand[0].iHandAnimIndex = 19;
                StartWeaponAnim(localClientNum, weaponIndex, obj, 19, 0.0, newPlayerstate);
                break;
            case 0x17u:
                viewModelInfo->hand[0].iHandAnimIndex = 22;
                StartWeaponAnim(localClientNum, weaponIndex, obj, 22, 0.0, newPlayerstate);
                break;
            case 0x18u:
                viewModelInfo->hand[0].iHandAnimIndex = 21;
                StartWeaponAnim(localClientNum, weaponIndex, obj, 21, 0.0, newPlayerstate);
                break;
            case 0x19u:
                viewModelInfo->hand[0].iHandAnimIndex = 24;
                StartWeaponAnim(localClientNum, weaponIndex, obj, 24, 0.0, newPlayerstate);
                break;
            case 0x1Au:
                viewModelInfo->hand[0].iHandAnimIndex = 23;
                StartWeaponAnim(localClientNum, weaponIndex, obj, 23, 0.0, newPlayerstate);
                break;
            case 0x1Bu:
                if ( **((_BYTE **)weapVariantDef->szXAnims + 25) )
                {
                    viewModelInfo->hand[0].iHandAnimIndex = 25;
                    StartWeaponAnim(localClientNum, weaponIndex, obj, 25, 0.0, newPlayerstate);
                }
                break;
            case 0x1Cu:
                if ( **((_BYTE **)weapVariantDef->szXAnims + 25) )
                {
                    viewModelInfo->hand[0].iHandAnimIndex = 26;
                    StartWeaponAnim(localClientNum, weaponIndex, obj, 26, 0.0, newPlayerstate);
                }
                else
                {
                    PlayIdleAnim(localClientNum, ps, viewModelInfo, obj, weaponIndex, 0.0, newPlayerstate);
                }
                if ( **((_BYTE **)weapVariantDef->szXAnims + 53) )
                    StartCameraAnim(localClientNum, weaponIndex, obj, 53, 0.0);
                break;
            case 0x1Du:
                if ( **((_BYTE **)weapVariantDef->szXAnims + 27) )
                {
                    viewModelInfo->hand[0].iHandAnimIndex = 27;
                    StartWeaponAnim(localClientNum, weaponIndex, obj, 27, 0.0, newPlayerstate);
                }
                if ( **((_BYTE **)weapVariantDef->szXAnims + 53) )
                    StopCameraAnim(obj);
                break;
            case 0x1Eu:
                if ( **((_BYTE **)weapVariantDef->szXAnims + 28) )
                {
                    viewModelInfo->hand[0].iHandAnimIndex = 28;
                    StartWeaponAnim(localClientNum, weaponIndex, obj, 28, 0.0, newPlayerstate);
                }
                else if ( **((_BYTE **)weapVariantDef->szXAnims + 25) )
                {
                    viewModelInfo->hand[0].iHandAnimIndex = 25;
                    StartWeaponAnim(localClientNum, weaponIndex, obj, 25, 0.0, newPlayerstate);
                }
                break;
            case 0x1Fu:
                if ( **((_BYTE **)weapVariantDef->szXAnims + 28) )
                {
                    viewModelInfo->hand[0].iHandAnimIndex = 29;
                    StartWeaponAnim(localClientNum, weaponIndex, obj, 29, 0.0, newPlayerstate);
                }
                else if ( **((_BYTE **)weapVariantDef->szXAnims + 25) )
                {
                    viewModelInfo->hand[0].iHandAnimIndex = 26;
                    StartWeaponAnim(localClientNum, weaponIndex, obj, 26, 0.0, newPlayerstate);
                }
                else
                {
                    PlayIdleAnim(localClientNum, ps, viewModelInfo, obj, weaponIndex, 0.0, newPlayerstate);
                }
                if ( **((_BYTE **)weapVariantDef->szXAnims + 53) )
                    StartCameraAnim(localClientNum, weaponIndex, obj, 53, 0.0);
                break;
            case 0x20u:
                if ( **((_BYTE **)weapVariantDef->szXAnims + 30) )
                {
                    viewModelInfo->hand[0].iHandAnimIndex = 30;
                    StartWeaponAnim(localClientNum, weaponIndex, obj, 30, 0.0, newPlayerstate);
                }
                else if ( **((_BYTE **)weapVariantDef->szXAnims + 27) )
                {
                    viewModelInfo->hand[0].iHandAnimIndex = 27;
                    StartWeaponAnim(localClientNum, weaponIndex, obj, 27, 0.0, newPlayerstate);
                }
                if ( **((_BYTE **)weapVariantDef->szXAnims + 53) )
                    StopCameraAnim(obj);
                break;
            case 0x24u:
                viewModelInfo->hand[0].iHandAnimIndex = 34;
                StartWeaponAnim(localClientNum, weaponIndex, obj, 34, 0.0, newPlayerstate);
                break;
            case 0x25u:
                viewModelInfo->hand[0].iHandAnimIndex = 35;
                StartWeaponAnim(localClientNum, weaponIndex, obj, 35, 0.0, newPlayerstate);
                break;
            case 0x26u:
                viewModelInfo->hand[0].iHandAnimIndex = 36;
                StartWeaponAnim(localClientNum, weaponIndex, obj, 36, 0.0, newPlayerstate);
                break;
            case 0x27u:
                viewModelInfo->hand[0].iHandAnimIndex = 4;
                StartWeaponAnim(localClientNum, weaponIndex, obj, 4, 0.0, newPlayerstate);
                break;
            case 0x28u:
                viewModelInfo->hand[0].iHandAnimIndex = 39;
                StartWeaponAnim(localClientNum, weaponIndex, obj, 39, 0.0, newPlayerstate);
                break;
            case 0x29u:
                viewModelInfo->hand[0].iHandAnimIndex = 40;
                StartWeaponAnim(localClientNum, weaponIndex, obj, 40, 0.0, newPlayerstate);
                break;
            case 0x2Au:
                viewModelInfo->hand[0].iHandAnimIndex = 41;
                StartWeaponAnim(localClientNum, weaponIndex, obj, 41, 0.0, newPlayerstate);
                break;
            case 0x2Bu:
                viewModelInfo->hand[0].iHandAnimIndex = 37;
                StartWeaponAnim(localClientNum, weaponIndex, obj, 37, 0.0, newPlayerstate);
                break;
            case 0x2Cu:
                viewModelInfo->hand[0].iHandAnimIndex = 38;
                StartWeaponAnim(localClientNum, weaponIndex, obj, 38, 0.0, newPlayerstate);
                break;
            case 0x2Du:
                if ( **((_BYTE **)weapVariantDef->szXAnims + 45) )
                {
                    viewModelInfo->hand[0].iHandAnimIndex = 45;
                    StartWeaponAnim(localClientNum, weaponIndex, obj, 45, 0.0, newPlayerstate);
                }
                else
                {
                    PlayIdleAnim(localClientNum, ps, viewModelInfo, obj, weaponIndex, 0.0, newPlayerstate);
                }
                if ( **((_BYTE **)weapVariantDef->szXAnims + 54) )
                    StartCameraAnim(localClientNum, weaponIndex, obj, 54, 0.0);
                break;
            case 0x2Eu:
                if ( **((_BYTE **)weapVariantDef->szXAnims + 46) )
                {
                    viewModelInfo->hand[0].iHandAnimIndex = 46;
                    StartWeaponAnim(localClientNum, weaponIndex, obj, 46, 0.0, newPlayerstate);
                }
                else
                {
                    PlayIdleAnim(localClientNum, ps, viewModelInfo, obj, weaponIndex, 0.0, newPlayerstate);
                }
                if ( **((_BYTE **)weapVariantDef->szXAnims + 55) )
                    StartCameraAnim(localClientNum, weaponIndex, obj, 55, 0.0);
                break;
            case 0x2Fu:
                if ( **((_BYTE **)weapVariantDef->szXAnims + 47) )
                {
                    viewModelInfo->hand[0].iHandAnimIndex = 47;
                    StartWeaponAnim(localClientNum, weaponIndex, obj, 47, 0.0, newPlayerstate);
                }
                else
                {
                    PlayIdleAnim(localClientNum, ps, viewModelInfo, obj, weaponIndex, 0.0, newPlayerstate);
                }
                if ( **((_BYTE **)weapVariantDef->szXAnims + 56) )
                    StartCameraAnim(localClientNum, weaponIndex, obj, 56, 0.0);
                break;
            case 0x30u:
                if ( **((_BYTE **)weapVariantDef->szXAnims + 48) )
                {
                    viewModelInfo->hand[0].iHandAnimIndex = 48;
                    StartWeaponAnim(localClientNum, weaponIndex, obj, 48, 0.0, newPlayerstate);
                }
                else if ( **((_BYTE **)weapVariantDef->szXAnims + 45) )
                {
                    viewModelInfo->hand[0].iHandAnimIndex = 45;
                    StartWeaponAnim(localClientNum, weaponIndex, obj, 45, 0.0, newPlayerstate);
                }
                else
                {
                    PlayIdleAnim(localClientNum, ps, viewModelInfo, obj, weaponIndex, 0.0, newPlayerstate);
                }
                if ( **((_BYTE **)weapVariantDef->szXAnims + 54) )
                    StartCameraAnim(localClientNum, weaponIndex, obj, 54, 0.0);
                break;
            case 0x31u:
                if ( **((_BYTE **)weapVariantDef->szXAnims + 49) )
                {
                    viewModelInfo->hand[0].iHandAnimIndex = 49;
                    StartWeaponAnim(localClientNum, weaponIndex, obj, 49, 0.0, newPlayerstate);
                }
                else if ( **((_BYTE **)weapVariantDef->szXAnims + 46) )
                {
                    viewModelInfo->hand[0].iHandAnimIndex = 46;
                    StartWeaponAnim(localClientNum, weaponIndex, obj, 46, 0.0, newPlayerstate);
                }
                else
                {
                    PlayIdleAnim(localClientNum, ps, viewModelInfo, obj, weaponIndex, 0.0, newPlayerstate);
                }
                if ( **((_BYTE **)weapVariantDef->szXAnims + 55) )
                    StartCameraAnim(localClientNum, weaponIndex, obj, 55, 0.0);
                break;
            case 0x32u:
                if ( **((_BYTE **)weapVariantDef->szXAnims + 50) )
                {
                    viewModelInfo->hand[0].iHandAnimIndex = 50;
                    StartWeaponAnim(localClientNum, weaponIndex, obj, 50, 0.0, newPlayerstate);
                }
                else if ( **((_BYTE **)weapVariantDef->szXAnims + 47) )
                {
                    viewModelInfo->hand[0].iHandAnimIndex = 47;
                    StartWeaponAnim(localClientNum, weaponIndex, obj, 47, 0.0, newPlayerstate);
                }
                else
                {
                    PlayIdleAnim(localClientNum, ps, viewModelInfo, obj, weaponIndex, 0.0, newPlayerstate);
                }
                if ( **((_BYTE **)weapVariantDef->szXAnims + 56) )
                    StartCameraAnim(localClientNum, weaponIndex, obj, 56, 0.0);
                break;
            case 0x33u:
                if ( **((_BYTE **)weapVariantDef->szXAnims + 51) )
                {
                    viewModelInfo->hand[0].iHandAnimIndex = 51;
                    StartWeaponAnim(localClientNum, weaponIndex, obj, 51, 0.0, newPlayerstate);
                }
                else
                {
LABEL_68:
                    PlayIdleAnim(localClientNum, ps, viewModelInfo, obj, weaponIndex, transitionTimea, newPlayerstate);
                }
                break;
            case 0x34u:
                if ( **((_BYTE **)weapVariantDef->szXAnims + 52) && Mantle_DoAnim(ps) )
                {
                    viewModelInfo->hand[0].iHandAnimIndex = 52;
                    StartWeaponAnim(localClientNum, weaponIndex, obj, 52, 0.0, newPlayerstate);
                }
                else
                {
                    PlayIdleAnim(localClientNum, ps, viewModelInfo, obj, weaponIndex, 0.0, newPlayerstate);
                }
                if ( **((_BYTE **)weapVariantDef->szXAnims + 57) && Mantle_DoAnim(ps) )
                    StartCameraAnim(localClientNum, weaponIndex, obj, 57, 0.0);
                break;
            default:
                PlayIdleAnim(localClientNum, ps, viewModelInfo, obj, weaponIndex, 0.0, newPlayerstate);
                Com_Printf(19, "WeaponRunXModelAnims: Unknown weapon animation %i\n", ps->weapAnim & 0xFFFFFBFF);
                break;
        }
        if ( playedAnima )
        {
            viewModelInfo->hand[0].iPrevAnim = ps->weapAnim;
            cgameGlob->prevViewmodelWeapon = weaponIndex;
        }
    }
    if ( weapDef->bDualWield )
        EnsureValidLeftAnim(localClientNum, obj, viewModelInfo, pAnimTree, weaponIndex, newPlayerstate);
}

void __cdecl StartCameraAnim(
                int localClientNum,
                unsigned int weaponNum,
                DObj *obj,
                int animIndex,
                float transitionTime)
{
    float rate; // [esp+1Ch] [ebp-14h]
    const WeaponVariantDef *weapVariantDef; // [esp+20h] [ebp-10h]
    ViewModelInfo *viewModelInfo; // [esp+24h] [ebp-Ch]
    XAnim_s *anims; // [esp+28h] [ebp-8h]
    signed int i; // [esp+2Ch] [ebp-4h]

    if ( (animIndex < 53 || animIndex > 57)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                    490,
                    0,
                    "%s",
                    "(animIndex >= WEAP_ANIM_CAMERA_START) && (animIndex <= WEAP_ANIM_CAMERA_END)") )
    {
        __debugbreak();
    }
    viewModelInfo = CG_GetLocalClientViewModelInfo(localClientNum);
    anims = viewModelInfo->anims;
    if ( !anims && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 494, 0, "%s", "anims") )
        __debugbreak();
    if ( viewModelInfo->tree->anims != viewModelInfo->anims
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                    495,
                    0,
                    "%s",
                    "viewModelInfo->tree->anims == viewModelInfo->anims") )
    {
        __debugbreak();
    }
    weapVariantDef = BG_GetWeaponVariantDef(weaponNum);
    rate = GetWeaponAnimRate(localClientNum, weapVariantDef, anims, animIndex);
    for ( i = 53; i <= 57; ++i )
    {
        if ( animIndex == i )
            XAnimSetGoalWeight(obj, i, 1.0, transitionTime, rate, 0, 1u, 1, -1);
        else
            XAnimSetGoalWeight(obj, i, 0.0, transitionTime, 1.0, 0, 0, 0, -1);
    }
}

void __cdecl StopCameraAnim(DObj *obj)
{
    signed int i; // [esp+1Ch] [ebp-4h]

    for ( i = 53; i <= 57; ++i )
        XAnimSetGoalWeight(obj, i, 0.0, 0.0, 1.0, 0, 0, 0, -1);
}

void __cdecl PlayIdleAnim(
                int localClientNum,
                const playerState_s *ps,
                ViewModelInfo *viewModelInfo,
                DObj *obj,
                unsigned int weaponIndex,
                float transitionTime,
                int newPlayerstate)
{
    if ( BG_GetAmmoInClip(ps, weaponIndex) )
    {
        viewModelInfo->hand[0].iHandAnimIndex = 1;
        StartWeaponAnim(localClientNum, weaponIndex, obj, 1, transitionTime, newPlayerstate);
    }
    else
    {
        viewModelInfo->hand[0].iHandAnimIndex = 2;
        StartWeaponAnim(localClientNum, weaponIndex, obj, 2, transitionTime, newPlayerstate);
    }
}

void __cdecl PlayADSAnim(float weaponPosFrac, int weaponNum, DObj *obj, int animIndex)
{
    XAnimTree_s *Tree; // eax
    XAnimTree_s *v5; // eax

    if ( animIndex != 64
        && animIndex != 65
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                    541,
                    0,
                    "%s",
                    "(animIndex == WEAP_ANIM_ADS_UP) || (animIndex == WEAP_ANIM_ADS_DOWN)") )
    {
        __debugbreak();
    }
    if ( animIndex == 64 )
    {
        XAnimSetGoalWeight(obj, 0x40u, 1.0, 0.1, 0.0, 0, 1u, 0, -1);
        XAnimSetGoalWeight(obj, 0x41u, 0.0, 0.1, 0.0, 0, 0, 0, -1);
    }
    else
    {
        XAnimSetGoalWeight(obj, 0x40u, 0.0, 0.1, 0.0, 0, 0, 0, -1);
        XAnimSetGoalWeight(obj, 0x41u, 1.0, 0.1, 0.0, 0, 1u, 0, -1);
    }
    Tree = DObjGetTree(obj);
    XAnimSetTime(Tree, 0x40u, weaponPosFrac, 0xFFFFu);
    v5 = DObjGetTree(obj);
    XAnimSetTime(v5, 0x41u, 1.0 - weaponPosFrac, 0xFFFFu);
}

int __cdecl CheckForValidAnimIDDW(int animID)
{
    int result; // eax

    switch ( animID )
    {
        case 1:
        case 2:
        case 3:
        case 5:
        case 9:
        case 11:
            result = 1;
            break;
        default:
            result = 0;
            break;
    }
    return result;
}

void __cdecl EnsureValidLeftAnim(
                int localClientNum,
                DObj *obj,
                ViewModelInfo *viewModelInfo,
                XAnimTree_s *pAnimTree,
                unsigned int weaponIndex,
                int newPlayerstate)
{
    signed int i; // [esp+8h] [ebp-8h]
    int isLeftAnimPlaying; // [esp+Ch] [ebp-4h]

    isLeftAnimPlaying = 0;
    if ( CheckForValidAnimIDDW(viewModelInfo->hand[0].iHandAnimIndex) )
    {
        for ( i = 58; i <= 63; ++i )
        {
            if ( XAnimGetWeight(pAnimTree, i) != 0.0 )
                isLeftAnimPlaying = 1;
        }
        if ( !isLeftAnimPlaying )
        {
            viewModelInfo->hand[1].iHandAnimIndex = 60;
            StartWeaponAnim(localClientNum, weaponIndex, obj, 60, 0.0, newPlayerstate);
        }
    }
}

void __cdecl ProcessWeaponNoteTracks(int localClientNum, const playerState_s *predictedPlayerState, bool isViewArms)
{
    unsigned int NumWeapons; // eax
    const char *NotifyStringName; // eax
    const char *v5; // eax
    const char *v6; // eax
    char *value; // [esp+0h] [ebp-20h]
    unsigned __int16 t; // [esp+4h] [ebp-1Ch]
    XAnimClientNotify *notify; // [esp+8h] [ebp-18h]
    int i; // [esp+Ch] [ebp-14h]
    int noteListSize; // [esp+10h] [ebp-10h]
    unsigned int weapIndex; // [esp+14h] [ebp-Ch]
    const WeaponDef *weapDef; // [esp+18h] [ebp-8h]
    XAnimClientNotifyList *pNotifyList; // [esp+1Ch] [ebp-4h]

    weapIndex = BG_GetViewmodelWeaponIndex(predictedPlayerState);
    if ( weapIndex )
    {
        if ( weapIndex >= BG_GetNumWeapons() )
        {
            NumWeapons = BG_GetNumWeapons();
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                            2024,
                            0,
                            "weapIndex doesn't index BG_GetNumWeapons()\n\t%i not in [0, %i)",
                            weapIndex,
                            NumWeapons) )
                __debugbreak();
        }
        weapDef = BG_GetWeaponDef(weapIndex);
        pNotifyList = (XAnimClientNotifyList *)DObjGetClientNotifies();
        if ( pNotifyList )
        {
            noteListSize = pNotifyList->m_numNotifies;
            for ( i = 0; i < noteListSize; ++i )
            {
                //notify = (XAnimClientNotify *)((char *)XAnimClientNotifyList::GetNotifyList(pNotifyList) + 24 * i);
                notify = (XAnimClientNotify *)((char *)pNotifyList->GetNotifyList() + 24 * i);
                //NotifyStringName = XAnimClientNotify::GetNotifyStringName(notify);
                NotifyStringName = notify->GetNotifyStringName();
                if ( I_stricmp(NotifyStringName, "end") )
                {
                    //value = (char *)XAnimClientNotify::GetNotifyStringName(notify);
                    value = (char*)notify->GetNotifyStringName();
                    Scr_AddString(value, SCRIPTINSTANCE_CLIENT);
                    Scr_AddInt(localClientNum, SCRIPTINSTANCE_CLIENT);
                    Scr_AddString("notetrack", SCRIPTINSTANCE_CLIENT);
                    t = Scr_ExecThread(SCRIPTINSTANCE_CLIENT, cg_scr_data.levelnotify, 3u);
                    Scr_FreeThread(t, SCRIPTINSTANCE_CLIENT);
                    //v5 = XAnimClientNotify::GetNotifyStringName(notify);
                    v5 = notify->GetNotifyStringName();
                    PlayNoteMappedSoundAliases(localClientNum, v5, weapDef);
                    //v6 = XAnimClientNotify::GetNotifyStringName(notify);
                    v6 = notify->GetNotifyStringName();
                    CG_PlayClientSoundNoteTracks(
                        localClientNum,
                        predictedPlayerState->clientNum,
                        (float*)predictedPlayerState->origin,
                        v6,
                        isViewArms);
                }
            }
        }
    }
}

void __cdecl PlayNoteMappedSoundAliases(int localClientNum, const char *noteName, const WeaponDef *weapDef)
{
    int ClientNumForLocalClient; // eax
    unsigned int AliasId; // [esp+4h] [ebp-10h]
    char *soundName; // [esp+8h] [ebp-Ch]
    int mapIdx; // [esp+Ch] [ebp-8h]
    unsigned int noteNameSL; // [esp+10h] [ebp-4h]

    if ( *weapDef->notetrackSoundMapKeys )
    {
        noteNameSL = SL_FindLowercaseString(noteName, SCRIPTINSTANCE_SERVER);
        if ( noteNameSL )
        {
            for ( mapIdx = 0; mapIdx < 20 && weapDef->notetrackSoundMapKeys[mapIdx]; ++mapIdx )
            {
                if ( weapDef->notetrackSoundMapValues[mapIdx] && weapDef->notetrackSoundMapKeys[mapIdx] == noteNameSL )
                {
                    soundName = SL_ConvertToString(weapDef->notetrackSoundMapValues[mapIdx], SCRIPTINSTANCE_SERVER);
                    if ( soundName )
                    {
                        AliasId = SND_FindAliasId(soundName);
                        ClientNumForLocalClient = CG_GetClientNumForLocalClient(localClientNum);
                        CG_PlaySound(localClientNum, ClientNumForLocalClient, 0, 0, 0, 1.0, AliasId);
                    }
                }
            }
        }
    }
}

bool __cdecl NeedToRebuildViewmodel(
                int localClientNum,
                unsigned int weaponIndex,
                unsigned __int8 weaponModel,
                const ViewModelInfo *viewModelInfo,
                weaponInfo_s *weapInfo)
{
    if ( !viewModelInfo->viewModelDObj )
        return 1;
    if ( weaponIndex == CG_GetLocalClientGlobals(localClientNum)->prevViewmodelWeapon )
        return UpdateViewmodelAttachments(localClientNum, weaponIndex, weaponModel, weapInfo);
    return 1;
}

bool __cdecl UpdateViewmodelAttachments(
                int localClientNum,
                unsigned int weaponNum,
                unsigned __int8 weaponModel,
                weaponInfo_s *weapInfo)
{
    const WeaponDef *weapDef; // [esp+0h] [ebp-10h]
    bool changed; // [esp+7h] [ebp-9h]
    XModel *newRocket; // [esp+Ch] [ebp-4h]

    if ( !weapInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 4470, 0, "%s", "weapInfo") )
    {
        __debugbreak();
    }
    newRocket = 0;
    weapDef = BG_GetWeaponDef(weaponNum);
    changed = 0;
    if ( weapInfo->gogglesModel )
    {
        changed = 1;
        weapInfo->gogglesModel = 0;
    }
    if ( ViewmodelRocketShouldBeAttached(localClientNum, weaponNum) )
        newRocket = weapDef->rocketModel;
    if ( newRocket != weapInfo->rocketModel )
    {
        changed = 1;
        weapInfo->rocketModel = newRocket;
    }
    return changed;
}

bool __cdecl ViewmodelRocketShouldBeAttached(int localClientNum, unsigned int weaponIndex)
{
    cg_s *cgameGlob; // [esp+Ch] [ebp-14h]
    const WeaponVariantDef *weapVariantDef; // [esp+10h] [ebp-10h]
    const WeaponDef *weapDef; // [esp+1Ch] [ebp-4h]

    weapVariantDef = BG_GetWeaponVariantDef(weaponIndex);
    weapDef = BG_GetWeaponDef(weaponIndex);
    if ( !weapDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 1831, 0, "%s", "weapDef") )
    {
        __debugbreak();
    }
    if ( !weapDef->rocketModel )
        return 0;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( BG_GetAmmoInClip(&cgameGlob->predictedPlayerState, cgameGlob->predictedPlayerState.weapon) )
        return 1;
    return (cgameGlob->predictedPlayerState.weaponstate == 10
             || cgameGlob->predictedPlayerState.weaponstate == 12
             || cgameGlob->predictedPlayerState.weaponstate == 14
             || cgameGlob->predictedPlayerState.weaponstate == 13
             || cgameGlob->predictedPlayerState.weaponstate == 11
             || cgameGlob->predictedPlayerState.weaponstate == 15
             || cgameGlob->predictedPlayerState.weaponstate == 16)
            && weapVariantDef->iReloadTime - cgameGlob->predictedPlayerState.weaponTime > weapDef->reloadShowRocketTime;
}

void __cdecl CG_AddViewWeapon(int localClientNum)
{
    weaponState_t ws; // [esp+0h] [ebp-D0h] BYREF
    cg_s *cgameGlob; // [esp+90h] [ebp-40h]
    int drawgun; // [esp+94h] [ebp-3Ch]
    float angles[3]; // [esp+98h] [ebp-38h] BYREF
    GfxScaledPlacement placement; // [esp+A4h] [ebp-2Ch] BYREF
    int weaponIndex; // [esp+C4h] [ebp-Ch]
    float fZoom; // [esp+C8h] [ebp-8h] BYREF
    playerState_s *ps; // [esp+CCh] [ebp-4h]

    drawgun = 1;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    ps = &cgameGlob->predictedPlayerState;
    cgameGlob->refdef.dof.viewModelStart = 0.0f;
    cgameGlob->refdef.dof.viewModelEnd = 0.0f;
    CG_TouchViewModels(localClientNum);
    if ( ps->pm_type != 4
        && ps->pm_type != 5
        && (ps->eFlags & 0x300) == 0
        && cgameGlob->renderingThirdPerson != TP_FOR_MODEL
        && !CG_RenderPlayerFromMissilePOV(localClientNum)
        && player_topDownCamMode->current.integer <= 0
        && !BG_UsingVehicleWeapon(ps) )
    {
        if ( cgameGlob->cubemapShot || !cg_drawGun->current.enabled || CG_GetWeapReticleZoom(cgameGlob, &fZoom) )
            drawgun = 0;
        weaponIndex = BG_GetViewmodelWeaponIndex(ps);
        if ( weaponIndex <= 0 )
        {
            cgameGlob->gunPitch = cgameGlob->refdefViewAngles[0];
            cgameGlob->gunYaw = cgameGlob->refdefViewAngles[1];
            cgameGlob->gunXOfs = 0.0f;
            cgameGlob->gunYOfs = 0.0f;
            cgameGlob->gunZOfs = 0.0f;
        }
        else
        {
            CalculateWeaponPostion_PositionToADS(cgameGlob, ps);
            CG_BuildWeaponState(cgameGlob, &ws);
            BG_CalculateWeaponMovement(&ws, placement.base.origin, angles);
            CG_CalculateWeaponMovement_ClientSpecific(cgameGlob, &placement, angles);
            CG_SaveWeaponState(&ws, cgameGlob);
            CG_AddPlayerWeapon(localClientNum, &placement, ps, &cgameGlob->predictedPlayerEntity, drawgun);
        }
    }
}

void __cdecl CalculateWeaponPostion_PositionToADS(cg_s *cgameGlob, playerState_s *ps)
{
    int weapIndex; // [esp+4h] [ebp-4h]

    weapIndex = BG_GetViewmodelWeaponIndex(ps);
    if ( BG_IsAimDownSightWeapon(weapIndex) )
    {
        if ( ps->fWeaponPosFrac == 1.0 )
        {
            cgameGlob->playerEntity.bPositionToADS = 0;
        }
        else if ( ps->fWeaponPosFrac == 0.0 )
        {
            cgameGlob->playerEntity.bPositionToADS = 1;
        }
        cgameGlob->playerEntity.fLastWeaponPosFrac = ps->fWeaponPosFrac;
    }
}

void __cdecl CG_CalculateWeaponMovement_ClientSpecific(cg_s *cgameGlob, GfxScaledPlacement *placement, float *angles)
{
    if ( !placement
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 4704, 0, "%s", "placement") )
    {
        __debugbreak();
    }
    CG_CalculateWeaponMovement_LocalToWorld(cgameGlob, placement->base.origin);
    CG_CalculateWeaponMovement_Landing(cgameGlob, placement->base.origin);
    CG_CalculateWeaponMovement_SaveOrigin(cgameGlob, placement->base.origin);
    CG_CalculateWeaponMovement_Debug(cgameGlob, placement->base.origin);
    CG_CalculateWeaponMovement_SaveAngles(cgameGlob, angles, placement);
    placement->scale = 1.0f;
}

void __cdecl CG_CalculateWeaponMovement_LocalToWorld(const cg_s *cgameGlob, float *origin)
{
    float originCopy[3]; // [esp+0h] [ebp-Ch] BYREF

    originCopy[0] = *origin;
    originCopy[1] = origin[1];
    originCopy[2] = origin[2];
    MatrixTransformVector43(originCopy, cgameGlob->viewModelAxis, origin);
}

void __cdecl CG_CalculateWeaponMovement_Landing(const cg_s *cgameGlob, float *origin)
{
    int delta; // [esp+0h] [ebp-4h]

    delta = cgameGlob->time - cgameGlob->landTime;
    if ( delta >= 150 )
    {
        if ( delta < 450 )
            origin[2] = (float)((float)((float)(cgameGlob->landChange * 0.25) * (float)(450 - delta)) / 300.0) + origin[2];
    }
    else
    {
        origin[2] = (float)((float)((float)(cgameGlob->landChange * 0.25) * (float)delta) / 150.0) + origin[2];
    }
}

void __cdecl CG_CalculateWeaponMovement_SaveOrigin(cg_s *cgameGlob, float *origin)
{
    float fPosLerp; // [esp+0h] [ebp-8h]
    int weapIndex; // [esp+4h] [ebp-4h]

    weapIndex = BG_GetViewmodelWeaponIndex(&cgameGlob->predictedPlayerState);
    if ( !BG_IsAimDownSightWeapon(weapIndex)
        || (fPosLerp = cgameGlob->predictedPlayerState.fWeaponPosFrac, fPosLerp == 0.0) )
    {
        cgameGlob->gunXOfs = 0.0f;
        cgameGlob->gunYOfs = 0.0f;
        cgameGlob->gunZOfs = 0.0f;
    }
    else
    {
        cgameGlob->gunXOfs = (float)(*origin - cgameGlob->refdef.vieworg[0]) * fPosLerp;
        cgameGlob->gunYOfs = (float)(origin[1] - cgameGlob->refdef.vieworg[1]) * fPosLerp;
        cgameGlob->gunZOfs = (float)(origin[2] - cgameGlob->refdef.vieworg[2]) * fPosLerp;
    }
}

void __cdecl CG_CalculateWeaponMovement_Debug(const cg_s *cgameGlob, float *origin)
{
    float v2; // [esp+0h] [ebp-34h]
    float v3; // [esp+8h] [ebp-2Ch]
    float v4; // [esp+10h] [ebp-24h]
    float v5; // [esp+18h] [ebp-1Ch]
    float v6; // [esp+1Ch] [ebp-18h]
    float v7; // [esp+20h] [ebp-14h]
    float fovcomp_y; // [esp+24h] [ebp-10h]
    float fovcomp_z; // [esp+28h] [ebp-Ch]
    float fovCoeff; // [esp+2Ch] [ebp-8h]

    v6 = (float)(cg_fov->current.value - cg_fov_default->current.value)
         * (float)(1.0 / (float)(cg_fovCompMax->current.value - cg_fov_default->current.value));
    if ( (float)(v6 - 1.0) < 0.0 )
        v7 = (float)(cg_fov->current.value - cg_fov_default->current.value)
             * (float)(1.0 / (float)(cg_fovCompMax->current.value - cg_fov_default->current.value));
    else
        v7 = 1.0f;
    if ( (float)(0.0 - v6) < 0.0 )
        v2 = v7;
    else
        v2 = 0.0f;
    fovCoeff = (float)(1.0 - cgameGlob->predictedPlayerState.fWeaponPosFrac) * v2;
    fovcomp_y = cg_gun_fovcomp_y->current.value * fovCoeff;
    fovcomp_z = cg_gun_fovcomp_z->current.value * fovCoeff;
    v5 = cg_gun_x->current.value + (float)(cg_gun_fovcomp_x->current.value * fovCoeff);
    *origin = (float)(v5 * cgameGlob->viewModelAxis[0][0]) + *origin;
    origin[1] = (float)(v5 * cgameGlob->viewModelAxis[0][1]) + origin[1];
    origin[2] = (float)(v5 * cgameGlob->viewModelAxis[0][2]) + origin[2];
    v4 = cg_gun_y->current.value + fovcomp_y;
    *origin = (float)(v4 * cgameGlob->viewModelAxis[1][0]) + *origin;
    origin[1] = (float)(v4 * cgameGlob->viewModelAxis[1][1]) + origin[1];
    origin[2] = (float)(v4 * cgameGlob->viewModelAxis[1][2]) + origin[2];
    v3 = cg_gun_z->current.value + fovcomp_z;
    *origin = (float)(v3 * cgameGlob->viewModelAxis[2][0]) + *origin;
    origin[1] = (float)(v3 * cgameGlob->viewModelAxis[2][1]) + origin[1];
    origin[2] = (float)(v3 * cgameGlob->viewModelAxis[2][2]) + origin[2];
}

void __cdecl CG_CalculateWeaponMovement_SaveAngles(cg_s *cgameGlob, float *angles, GfxScaledPlacement *placement)
{
    float vAxis2[3][3]; // [esp+Ch] [ebp-78h] BYREF
    int weaponIndex; // [esp+30h] [ebp-54h]
    playerState_s *ps; // [esp+34h] [ebp-50h]
    const WeaponDef *weapDef; // [esp+38h] [ebp-4Ch]
    float axis[3][3]; // [esp+3Ch] [ebp-48h] BYREF
    float vAxis[3][3]; // [esp+60h] [ebp-24h] BYREF

    ps = &cgameGlob->predictedPlayerState;
    cgameGlob->baseGunAngles[0] = cgameGlob->refdefViewAngles[0];
    cgameGlob->baseGunAngles[1] = cgameGlob->refdefViewAngles[1];
    cgameGlob->baseGunAngles[2] = cgameGlob->refdefViewAngles[2];
    AnglesToAxis(angles, vAxis);
    AnglesToAxis(cgameGlob->baseGunAngles, vAxis2);
    MatrixMultiply(vAxis, vAxis2, axis);
    AxisToQuat(axis, placement->base.quat);
    weaponIndex = BG_GetViewmodelWeaponIndex(ps);
    weapDef = BG_GetWeaponDef(weaponIndex);
    if ( !BG_IsAimDownSightWeapon(ps->weapon) || ps->fWeaponPosFrac == 0.0 || weapDef->overlayReticle )
    {
        cgameGlob->gunPitch = cgameGlob->baseGunAngles[0];
        cgameGlob->gunYaw = cgameGlob->baseGunAngles[1];
    }
    else
    {
        UnitQuatToAngles(placement->base.quat, angles);
        cgameGlob->gunPitch = AngleNormalize360(*angles);
        cgameGlob->gunYaw = AngleNormalize360(angles[1]);
    }
}

void __cdecl CG_BuildWeaponState(cg_s *cgameGlob, weaponState_t *ws)
{
    int v2; // [esp+0h] [ebp-50h]
    const shellshock_parms_t *ssParms; // [esp+48h] [ebp-8h]

    ws->ps = &cgameGlob->predictedPlayerState;
    ws->xyspeed = cgameGlob->xyspeed;
    if ( cgameGlob->resetWeaponStateFlag )
    {
        ws->frametime = 1.0f;
        cgameGlob->swayViewAngles[0] = cgameGlob->predictedPlayerState.viewangles[0];
        cgameGlob->swayViewAngles[1] = cgameGlob->predictedPlayerState.viewangles[1];
        cgameGlob->swayViewAngles[2] = cgameGlob->predictedPlayerState.viewangles[2];
        cgameGlob->resetWeaponStateFlag = 0;
    }
    else
    {
        ws->frametime = (float)cgameGlob->frametime * 0.001;
    }
    ws->weapIdleTime = &cgameGlob->weapIdleTime;
    ws->v_dmg_pitch = cgameGlob->v_dmg_pitch;
    ws->v_dmg_roll = cgameGlob->v_dmg_roll;
    ws->shellShockStart = cgameGlob->shellshock.startTime;
    ws->shellShockDuration = cgameGlob->shellshock.duration;
    ssParms = cgameGlob->shellshock.parms;
    if ( ssParms )
        ws->shellShockFadeTime = ssParms->view.fadeTime;
    else
        ws->shellShockFadeTime = 3000;
    ws->time = cgameGlob->time - cgameGlob->predictedPlayerState.deltaTime;
    if ( cgameGlob->damageTime )
        v2 = cgameGlob->damageTime - cgameGlob->predictedPlayerState.deltaTime;
    else
        v2 = 0;
    ws->damageTime = v2;
    ws->recoilAngles[0] = cgameGlob->recoilAngles[0];
    ws->recoilAngles[1] = cgameGlob->recoilAngles[1];
    ws->recoilAngles[2] = cgameGlob->recoilAngles[2];
    ws->recoilSpeed[0] = cgameGlob->recoilSpeed[0];
    ws->recoilSpeed[1] = cgameGlob->recoilSpeed[1];
    ws->recoilSpeed[2] = cgameGlob->recoilSpeed[2];
    ws->swayAngles[0] = cgameGlob->swayAngles[0];
    ws->swayAngles[1] = cgameGlob->swayAngles[1];
    ws->swayAngles[2] = cgameGlob->swayAngles[2];
    ws->swayOrigin[0] = cgameGlob->swayOffset[0];
    ws->swayOrigin[1] = cgameGlob->swayOffset[1];
    ws->swayOrigin[2] = cgameGlob->swayOffset[2];
    ws->swayViewAngles[0] = cgameGlob->swayViewAngles[0];
    ws->swayViewAngles[1] = cgameGlob->swayViewAngles[1];
    ws->swayViewAngles[2] = cgameGlob->swayViewAngles[2];
    ws->fLastIdleFactor = cgameGlob->playerEntity.fLastIdleFactor;
    ws->baseAngles[0] = cgameGlob->playerEntity.baseMoveAngles[0];
    ws->baseAngles[1] = cgameGlob->playerEntity.baseMoveAngles[1];
    ws->baseAngles[2] = cgameGlob->playerEntity.baseMoveAngles[2];
    ws->baseOrigin[0] = cgameGlob->playerEntity.baseMoveOrigin[0];
    ws->baseOrigin[1] = cgameGlob->playerEntity.baseMoveOrigin[1];
    ws->baseOrigin[2] = cgameGlob->playerEntity.baseMoveOrigin[2];
}

void __cdecl CG_SaveWeaponState(const weaponState_t *ws, cg_s *cgameGlob)
{
    cgameGlob->recoilAngles[0] = ws->recoilAngles[0];
    cgameGlob->recoilAngles[1] = ws->recoilAngles[1];
    cgameGlob->recoilAngles[2] = ws->recoilAngles[2];
    cgameGlob->recoilSpeed[0] = ws->recoilSpeed[0];
    cgameGlob->recoilSpeed[1] = ws->recoilSpeed[1];
    cgameGlob->recoilSpeed[2] = ws->recoilSpeed[2];
    cgameGlob->swayAngles[0] = ws->swayAngles[0];
    cgameGlob->swayAngles[1] = ws->swayAngles[1];
    cgameGlob->swayAngles[2] = ws->swayAngles[2];
    cgameGlob->swayOffset[0] = ws->swayOrigin[0];
    cgameGlob->swayOffset[1] = ws->swayOrigin[1];
    cgameGlob->swayOffset[2] = ws->swayOrigin[2];
    cgameGlob->swayViewAngles[0] = ws->swayViewAngles[0];
    cgameGlob->swayViewAngles[1] = ws->swayViewAngles[1];
    cgameGlob->swayViewAngles[2] = ws->swayViewAngles[2];
    cgameGlob->playerEntity.fLastIdleFactor = ws->fLastIdleFactor;
    cgameGlob->playerEntity.baseMoveAngles[0] = ws->baseAngles[0];
    cgameGlob->playerEntity.baseMoveAngles[1] = ws->baseAngles[1];
    cgameGlob->playerEntity.baseMoveAngles[2] = ws->baseAngles[2];
    cgameGlob->playerEntity.baseMoveOrigin[0] = ws->baseOrigin[0];
    cgameGlob->playerEntity.baseMoveOrigin[1] = ws->baseOrigin[1];
    cgameGlob->playerEntity.baseMoveOrigin[2] = ws->baseOrigin[2];
}

void __cdecl CG_NextWeapon_f()
{
    cg_s *cgameGlob; // [esp+0h] [ebp-8h]

    cgameGlob = CG_GetLocalClientGlobals(0);
    if ( cgameGlob->nextSnap )
    {
        if ( WeaponCycleAllowed(cgameGlob) )
        {
            cgameGlob->weaponSelectTime = cgameGlob->time;
            CG_MenuShowNotify(0, 1);
            CycleWeapPrimary(0, 1, 0);
        }
    }
}

bool __cdecl WeaponCycleAllowed(cg_s *cgameGlob)
{
    if ( !cgameGlob
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 4998, 0, "%s", "cgameGlob") )
    {
        __debugbreak();
    }
    if ( Demo_IsPlaying() )
        return 0;
    if ( (cgameGlob->predictedPlayerState.pm_flags & 0xC00) != 0 || cgameGlob->predictedPlayerState.weaponstate == 35 )
        return 0;
    if ( (cgameGlob->predictedPlayerState.weapFlags & 0x80) != 0 )
        return 0;
    if ( (cgameGlob->predictedPlayerState.weapFlags & 0x200) != 0 )
        return 0;
    if ( cgameGlob->time - cgameGlob->weaponSelectTime < cg_weaponCycleDelay->current.integer )
        return 0;
    if ( cgameGlob->predictedPlayerState.waterlevel >= 3 )
        return 0;
    if ( (cgameGlob->predictedPlayerState.pm_flags & 4) != 0 )
        return 0;
    if ( (cgameGlob->predictedPlayerState.pm_flags & 0x400000) != 0 )
        return 0;
    if ( (cgameGlob->nextSnap->ps.otherFlags & 4) == 0 )
        return 0;
    if ( cgameGlob->weaponForcedSelect
        && (cgameGlob->predictedPlayerState.weaponstate == 3
         || cgameGlob->predictedPlayerState.weaponstate == 4
         || cgameGlob->predictedPlayerState.weaponstate == 5) )
    {
        return 0;
    }
    return CG_CancelOffhand(cgameGlob) == 0;
}

char __cdecl CG_CancelOffhand(cg_s *cgameGlob)
{
    if ( !cgameGlob
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 4978, 0, "%s", "cgameGlob") )
    {
        __debugbreak();
    }
    if ( cgameGlob->predictedPlayerState.weaponstate < 20
        || cgameGlob->predictedPlayerState.weaponstate > 24
        || !BG_GetWeaponDef(cgameGlob->predictedPlayerState.offHandIndex)->offhandHoldIsCancelable )
    {
        return 0;
    }
    //bitarray<51>::setBit(&cgameGlob->extraButton_bits, 0x31u);
    cgameGlob->extraButton_bits.setBit(49);
    return 1;
}

void __cdecl CG_PrevWeapon_f()
{
    cg_s *cgameGlob; // [esp+0h] [ebp-8h]

    cgameGlob = CG_GetLocalClientGlobals(0);
    if ( cgameGlob->nextSnap )
    {
        if ( WeaponCycleAllowed(cgameGlob) )
        {
            cgameGlob->weaponSelectTime = cgameGlob->time;
            CycleWeapPrimary(0, 0, 0);
        }
    }
}

void __cdecl CG_OutOfAmmoChange(int localClientNum)
{
    unsigned int weaponIndex; // [esp+Ch] [ebp-14h]
    unsigned int weapon; // [esp+10h] [ebp-10h]
    cg_s *cgameGlob; // [esp+14h] [ebp-Ch]
    playerState_s *ps; // [esp+18h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+1Ch] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->nextSnap )
    {
        ps = &cgameGlob->predictedPlayerState;
        if ( cgameGlob->predictedPlayerState.pm_type < 9 )
        {
            if ( !cgameGlob->predictedPlayerState.weapon
                && cgameGlob->weaponLatestPrimaryIdx
                && BG_PlayerHasWeapon(ps, cgameGlob->weaponLatestPrimaryIdx) )
            {
                CG_SelectWeaponIndex(localClientNum, cgameGlob->weaponLatestPrimaryIdx);
                return;
            }
            if ( (cgameGlob->predictedPlayerState.weapFlags & 0x200) == 0 )
            {
                weapDef = BG_GetWeaponDef(cgameGlob->predictedPlayerState.weapon);
                if ( weapDef->inventoryType != WEAPINVENTORY_ALTMODE )
                    cgameGlob->weaponForcedSelect = 1;
                if ( !weapDef->cancelAutoHolsterWhenEmpty )
                {
                    if ( weapDef->inventoryType != WEAPINVENTORY_ALTMODE )
                        goto LABEL_20;
                    if ( !VerifyPlayerAltModeWeapon(localClientNum, cgameGlob->predictedPlayerState.weapon) )
                        return;
                    weapon = CG_GetAltWeapon(cgameGlob, cgameGlob->predictedPlayerState.weapon);
                    if ( BG_WeaponAmmo(ps, weapon) )
                    {
                        CG_SelectWeaponIndex(localClientNum, weapon);
                    }
                    else
                    {
LABEL_20:
                        if ( !CycleWeapPrimary(localClientNum, 1, 1) && weapDef->inventoryType == WEAPINVENTORY_ALTMODE )
                        {
                            weaponIndex = CG_GetAltWeapon(cgameGlob, cgameGlob->predictedPlayerState.weapon);
                            CG_SelectWeaponIndex(localClientNum, weaponIndex);
                        }
                    }
                }
            }
        }
    }
}

char __cdecl VerifyPlayerAltModeWeapon(int localClientNum, unsigned int weaponIndex)
{
    unsigned int AltWeapon; // eax
    const char *v3; // eax
    const char *szAltWeaponName; // [esp-4h] [ebp-20h]
    cg_s *cgameGlob; // [esp+10h] [ebp-Ch]
    const WeaponDef *weapDef; // [esp+18h] [ebp-4h]

    weapDef = BG_GetWeaponDef(weaponIndex);
    if ( !weapDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 5098, 0, "%s", "weapDef") )
    {
        __debugbreak();
    }
    if ( weapDef->inventoryType != WEAPINVENTORY_ALTMODE
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                    5099,
                    0,
                    "%s",
                    "weapDef->inventoryType == WEAPINVENTORY_ALTMODE") )
    {
        __debugbreak();
    }
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    AltWeapon = CG_GetAltWeapon(cgameGlob, weaponIndex);
    if ( BG_PlayerHasWeapon(&cgameGlob->predictedPlayerState, AltWeapon) )
        return 1;
    szAltWeaponName = BG_GetWeaponVariantDef(weaponIndex)->szAltWeaponName;
    v3 = BG_WeaponName(weaponIndex);
    Com_PrintError(
        14,
        "Player is holding alt-mode weapon \"%s\", but does not posses it's original, \"%s\".\n",
        v3,
        szAltWeaponName);
    CG_SelectWeaponIndex(localClientNum, 0);
    return 0;
}

char __cdecl CycleWeapPrimary(int localClientNum, int cycleForward, int bIgnoreEmpty)
{
    unsigned int AltWeapon; // eax
    int startSlot; // [esp+28h] [ebp-1Ch]
    cg_s *cgameGlob; // [esp+2Ch] [ebp-18h]
    int slot; // [esp+30h] [ebp-14h]
    int slota; // [esp+30h] [ebp-14h]
    int weaponIndex; // [esp+38h] [ebp-Ch]
    int weaponIndexa; // [esp+38h] [ebp-Ch]
    playerState_s *ps; // [esp+3Ch] [ebp-8h]
    const WeaponDef *weapDef; // [esp+40h] [ebp-4h]
    const WeaponDef *weapDefa; // [esp+40h] [ebp-4h]
    const WeaponDef *weapDefb; // [esp+40h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob->nextSnap )
        return 0;
    if ( (cgameGlob->nextSnap->ps.otherFlags & 4) == 0 )
        return 0;
    ps = &cgameGlob->predictedPlayerState;
    if ( cgameGlob->predictedPlayerState.pm_type >= 9 )
        return 0;
    if ( (cgameGlob->predictedPlayerState.eFlags2 & 0x10000000) != 0 )
        return 0;
    weaponIndex = cgameGlob->weaponSelect;
    weapDef = BG_GetWeaponDef(weaponIndex);
    if ( BG_GetNumWeapons() < 2 )
        return 0;
    if ( weapDef->inventoryType == WEAPINVENTORY_ALTMODE )
    {
        if ( VerifyPlayerAltModeWeapon(localClientNum, weaponIndex) )
        {
            AltWeapon = CG_GetAltWeapon(cgameGlob, weaponIndex);
            CG_SelectWeaponIndex(localClientNum, AltWeapon);
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else if ( weapDef->inventoryType
                 && cgameGlob->weaponLatestPrimaryIdx
                 && BG_PlayerHasWeapon(ps, cgameGlob->weaponLatestPrimaryIdx) )
    {
        CG_SelectWeaponIndex(localClientNum, cgameGlob->weaponLatestPrimaryIdx);
        if ( BG_GetWeaponDef(cgameGlob->weaponLatestPrimaryIdx)->bIsCarriedKillstreakWeapon )
            cgameGlob->weaponLatestPrimaryIdx = 0;
        return 1;
    }
    else if ( BG_GetWeaponDef(cgameGlob->predictedPlayerState.weapon)->bIsCarriedKillstreakWeapon
                 && cgameGlob->weaponLatestPrimaryIdx
                 && cgameGlob->weaponLatestPrimaryIdx != cgameGlob->predictedPlayerState.weapon
                 && BG_PlayerHasWeapon(ps, cgameGlob->weaponLatestPrimaryIdx) )
    {
        CG_SelectWeaponIndex(localClientNum, cgameGlob->weaponLatestPrimaryIdx);
        return 1;
    }
    else
    {
        slot = -1;
        if ( weaponIndex )
            slot = BG_GetHeldWeaponSlot(ps, weaponIndex);
        if ( slot == -1 )
            slot = 0;
        startSlot = slot;
        while ( 1 )
        {
            slot = (2 * (cycleForward != 0) - 1 + slot) % 15;
            if ( slot < 0 )
                slot = 14;
            if ( slot == startSlot )
                break;
            weaponIndexa = ps->heldWeapons[slot].weapon;
            if ( weaponIndexa )
            {
                weapDefa = BG_GetWeaponDef(weaponIndexa);
                if ( (!bIgnoreEmpty
                     || BG_WeaponAmmo(ps, weaponIndexa)
                     || weapDefa->unlimitedAmmo
                     || CG_CheckIfDualWieldEmpty(ps, weaponIndexa))
                    && weapDefa->inventoryType != WEAPINVENTORY_ALTMODE
                    && weapDefa->inventoryType != WEAPINVENTORY_ITEM
                    && weapDefa->inventoryType != WEAPINVENTORY_OFFHAND
                    && weapDefa->inventoryType != WEAPINVENTORY_MELEE
                    && !weapDefa->bIsCarriedKillstreakWeapon )
                {
LABEL_44:
                    CG_SelectWeaponIndex(localClientNum, weaponIndexa);
                    return 1;
                }
            }
        }
        if ( zombiemode->current.enabled )
        {
            weapDefb = BG_GetWeaponDef(cgameGlob->weaponSelect);
            if ( weapDefb->inventoryType )
            {
                if ( weapDefb->inventoryType != WEAPINVENTORY_MELEE )
                {
                    for ( slota = 0; slota < 15; ++slota )
                    {
                        weaponIndexa = ps->heldWeapons[slota].weapon;
                        if ( weaponIndexa && BG_GetWeaponDef(weaponIndexa)->inventoryType == WEAPINVENTORY_MELEE )
                            goto LABEL_44;
                    }
                }
            }
        }
        if ( !BG_PlayerHasWeapon(ps, cgameGlob->weaponSelect) )
            CG_SelectWeaponIndex(localClientNum, 0);
        return 0;
    }
}

unsigned int __cdecl CG_GetLastWeaponForAlt(const cg_s *cgameGlob, const playerState_s *ps, unsigned int weapIndex)
{
    const char *v4; // eax
    const char *v5; // eax
    int heldWeaponIndex; // [esp+4h] [ebp-4h]

    if ( !weapIndex )
        return 0;
    heldWeaponIndex = BG_GetHeldWeaponSlot(ps, weapIndex);
    if ( heldWeaponIndex == -1 )
    {
        v4 = BG_WeaponName(weapIndex);
        v5 = va("Not holding alt weapon %s, but trying to get the primary weapon to switch back to.\n", v4);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                        5374,
                        0,
                        "%s\n\t%s",
                        "heldWeaponIndex != INVALID_HELD_WEAPON_SLOT",
                        v5) )
            __debugbreak();
    }
    if ( heldWeaponIndex == -1 )
        return 0;
    else
        return cgameGlob->lastWeaponForAlt[heldWeaponIndex];
}

unsigned int __cdecl CG_GetAltWeapon(const cg_s *cgameGlob, unsigned int weapIndex)
{
    const WeaponVariantDef *weapVarDef; // eax
    const WeaponVariantDef *weapVariantDef; // [esp+0h] [ebp-Ch]
    const WeaponDef *weapDef; // [esp+4h] [ebp-8h]
    unsigned int lastWeaponForAlt; // [esp+8h] [ebp-4h]

    weapVariantDef = BG_GetWeaponVariantDef(weapIndex);
    weapDef = BG_GetWeaponDef(weapIndex);
    if ( weapDef->inventoryType != WEAPINVENTORY_ALTMODE )
        return weapVariantDef->altWeaponIndex;
    if ( Demo_IsPlaying() )
    {
        if ( weapIndex != cgameGlob->predictedPlayerState.weapon
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                        5392,
                        0,
                        "%s",
                        "weapIndex == cgameGlob->predictedPlayerState.weapon") )
        {
            __debugbreak();
        }
        return cgameGlob->predictedPlayerState.lastWeaponAltModeSwitch;
    }
    else if ( (cgameGlob->predictedPlayerState.otherFlags & 2) != 0 || cgameGlob->inKillCam )
    {
        if ( weapIndex != cgameGlob->predictedPlayerState.weapon
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                        5399,
                        0,
                        "%s",
                        "weapIndex == cgameGlob->predictedPlayerState.weapon") )
        {
            __debugbreak();
        }
        return cgameGlob->predictedPlayerState.lastWeaponAltModeSwitch;
    }
    else
    {
        lastWeaponForAlt = CG_GetLastWeaponForAlt(cgameGlob, &cgameGlob->predictedPlayerState, weapIndex);
        weapVarDef = BG_GetWeaponVariantDef(lastWeaponForAlt);
        if ( BG_GetWeaponDef(weapVarDef->altWeaponIndex) != weapDef
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                        5405,
                        0,
                        "%s",
                        "BG_GetWeaponDef( BG_GetWeaponVariantDef( lastWeaponForAlt )->altWeaponIndex ) == weapDef") )
        {
            __debugbreak();
        }
        return lastWeaponForAlt;
    }
}

unsigned int __cdecl CG_AltWeaponToggleIndex(int localClientNum, const cg_s *cgameGlob, unsigned int *primaryWeapon)
{
    unsigned int altWeaponIndex; // [esp+0h] [ebp-14h]
    const WeaponVariantDef *weapVariantDef; // [esp+4h] [ebp-10h]
    const playerState_s *ps; // [esp+8h] [ebp-Ch]
    unsigned int newPrimaryIdx; // [esp+10h] [ebp-4h]

    if ( !cgameGlob
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 5420, 0, "%s", "cgameGlob") )
    {
        __debugbreak();
    }
    ps = &cgameGlob->predictedPlayerState;
    weapVariantDef = BG_GetWeaponVariantDef(cgameGlob->weaponSelect);
    if ( BG_GetWeaponDef(cgameGlob->weaponSelect)->inventoryType == WEAPINVENTORY_ALTMODE )
        altWeaponIndex = CG_GetAltWeapon(cgameGlob, cgameGlob->weaponSelect);
    else
        altWeaponIndex = weapVariantDef->altWeaponIndex;
    if ( altWeaponIndex )
        return altWeaponIndex;
    newPrimaryIdx = NextWeapInCycle(localClientNum, ps, WEAPINVENTORY_PRIMARY, cgameGlob->weaponSelect, 1, 1, 1);
    if ( !newPrimaryIdx )
    {
        newPrimaryIdx = NextWeapInCycle(localClientNum, ps, WEAPINVENTORY_PRIMARY, cgameGlob->weaponSelect, 1, 0, 1);
        if ( !newPrimaryIdx )
            return 0;
    }
    if ( primaryWeapon )
        *primaryWeapon = newPrimaryIdx;
    return BG_GetWeaponVariantDef(newPrimaryIdx)->altWeaponIndex;
}

unsigned int __cdecl NextWeapInCycle(
                int localClientNum,
                const playerState_s *ps,
                weapInventoryType_t type,
                unsigned int startWeaponIndex,
                bool cycleForward,
                bool skipEmpties,
                bool skipHaveNoAlts)
{
    int startSlot; // [esp+4h] [ebp-18h]
    int slot; // [esp+8h] [ebp-14h]
    const WeaponVariantDef *weapVariantDef; // [esp+10h] [ebp-Ch]
    unsigned int weaponIndex; // [esp+14h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+18h] [ebp-4h]

    slot = -1;
    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 5183, 0, "%s", "ps") )
        __debugbreak();
    if ( BG_GetNumWeapons() < 2 )
        return 0;
    if ( startWeaponIndex )
        slot = BG_GetHeldWeaponSlot(ps, startWeaponIndex);
    if ( slot == -1 )
        slot = 0;
    startSlot = slot;
    while ( 1 )
    {
        slot = ((cycleForward ? 1 : -1) + slot) % 15;
        if ( slot < 0 )
            slot = 14;
        if ( slot == startSlot )
            break;
        weaponIndex = ps->heldWeapons[slot].weapon;
        if ( weaponIndex )
        {
            weapVariantDef = BG_GetWeaponVariantDef(weaponIndex);
            weapDef = BG_GetWeaponDef(weaponIndex);
            if ( weapDef->inventoryType == type
                && (!skipEmpties || BG_WeaponAmmo(ps, weaponIndex))
                && (!skipHaveNoAlts || weapVariantDef->altWeaponIndex)
                && !weapDef->bIsCarriedKillstreakWeapon )
            {
                return weaponIndex;
            }
        }
    }
    return 0;
}

void __cdecl CG_ActionSlotDown_f()
{
    bool v0; // [esp+0h] [ebp-50h]
    ActionSlotType v1; // [esp+4h] [ebp-4Ch]
    cg_s *cgameGlob; // [esp+30h] [ebp-20h]
    int slot; // [esp+34h] [ebp-1Ch] BYREF
    int weapon; // [esp+38h] [ebp-18h]
    int localClientNum; // [esp+3Ch] [ebp-14h]
    bool didSomething; // [esp+43h] [ebp-Dh]
    playerState_s *ps; // [esp+44h] [ebp-Ch]
    const WeaponDef *weapDef; // [esp+48h] [ebp-8h]
    bool new_weapon_has_no_ammo; // [esp+4Fh] [ebp-1h]

    slot = 0;
    localClientNum = 0;
    cgameGlob = CG_GetLocalClientGlobals(0);
    ps = &cgameGlob->predictedPlayerState;
    if ( ActionSlotUsageAllowed(cgameGlob) && cgameGlob->time - cgameGlob->lastActionSlotTime >= 250 && ActionParms(&slot) )
    {
        switch ( slot )
        {
            case 0:
            {
                //bitarray<51>::setBit(&cgameGlob->extraButton_bits, 0x2Du);
                cgameGlob->extraButton_bits.setBit(45);
                break;
            }
            case 1:
            {
                //bitarray<51>::setBit(&cgameGlob->extraButton_bits, 0x2Eu);
                cgameGlob->extraButton_bits.setBit(46);
                break;
            }
            case 2:
            {
                //bitarray<51>::setBit(&cgameGlob->extraButton_bits, 0x2Fu);
                cgameGlob->extraButton_bits.setBit(47);
                break;
            }
            case 3:
            {
                //bitarray<51>::setBit(&cgameGlob->extraButton_bits, 0x30u);
                cgameGlob->extraButton_bits.setBit(48);
                break;
            }
            default:
                break;
        }
        if ( (ps->weapFlags & 0x80) == 0 && (ps->eFlags2 & 0x10000000) == 0 )
        {
            didSomething = 0;
            v1 = ps->actionSlotType[slot];
            switch ( v1 )
            {
                case ACTIONSLOTTYPE_SPECIFYWEAPON:
                    weapon = ps->actionSlotParam[slot].specifyWeapon.index;
                    if ( weapon == cgameGlob->weaponSelect || CG_CheckPuttingAwayKillstreakWeapon(ps->weapon, weapon) )
                    {
                        if ( weapon == cgameGlob->weaponLatestPrimaryIdx
                            || !BG_PlayerHasWeapon(ps, cgameGlob->weaponLatestPrimaryIdx) )
                        {
                            if ( (ps->eFlags & 0x4000) == 0 )
                                didSomething = CycleWeapPrimary(localClientNum, 1, 0);
                        }
                        else
                        {
                            CG_SelectWeaponIndex(localClientNum, cgameGlob->weaponLatestPrimaryIdx);
                            didSomething = 1;
                        }
                    }
                    else if ( BG_PlayerHasWeapon(ps, weapon)
                                 && ps->weapon != weapon
                                 && (ps->weaponstate < 20 || ps->weaponstate > 25) )
                    {
                        weapDef = BG_GetWeaponDef(weapon);
                        v0 = !BG_GetAmmoInClip(ps, weapon) && !BG_GetAmmoNotInClip(ps, weapon) && !weapDef->unlimitedAmmo;
                        new_weapon_has_no_ammo = v0;
                        if ( weapDef->offhandSlot == OFFHAND_SLOT_EQUIPMENT && new_weapon_has_no_ammo && !weapDef->hasDetonator )
                        {
                            if ( cgameGlob->extraCamEntity == 1023
                                && cgameGlob->invalidCmdHintType != INVALID_CMD_NO_AMMO_EQUIPMENT
                                && !weapDef->hasDetonator )
                            {
                                cgameGlob->invalidCmdHintType = INVALID_CMD_NO_AMMO_EQUIPMENT;
                                cgameGlob->invalidCmdHintTime = cgameGlob->time;
                            }
                        }
                        else
                        {
                            didSomething = 1;
                            CG_SelectWeaponIndex(localClientNum, weapon);
                        }
                    }
                    break;
                case ACTIONSLOTTYPE_ALTWEAPONTOGGLE:
                    didSomething = ToggleWeaponAltMode(localClientNum);
                    break;
                case ACTIONSLOTTYPE_NIGHTVISION:
                    //bitarray<51>::setBit(&cgameGlob->extraButton_bits, 0x12u);
                    cgameGlob->extraButton_bits.setBit(18);
                    didSomething = 1;
                    break;
            }
            cgameGlob->ammoFadeTime = cgameGlob->time;
            if ( didSomething )
            {
                cgameGlob->lastActionSlotTime = cgameGlob->time;
                if ((ps->eFlags & 0x300) != 0 && ps->actionSlotType[slot] == ACTIONSLOTTYPE_SPECIFYWEAPON)
                {
                    //bitarray<51>::setBit(&cgameGlob->extraButton_bits, 5u);
                    cgameGlob->extraButton_bits.setBit(5);
                }
            }
        }
    }
}

char __cdecl ToggleWeaponAltMode(int localClientNum)
{
    bool v2; // [esp+3h] [ebp-25h]
    const WeaponDef *newWeaponDef; // [esp+10h] [ebp-18h]
    unsigned int primaryWeapIdx; // [esp+18h] [ebp-10h] BYREF
    cg_s *cgameGlob; // [esp+1Ch] [ebp-Ch]
    unsigned int weapIdx; // [esp+20h] [ebp-8h]
    playerState_s *ps; // [esp+24h] [ebp-4h]

    primaryWeapIdx = 0;
    if ( Demo_IsPlaying() )
        return 0;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    ps = &cgameGlob->predictedPlayerState;
    if ( (cgameGlob->predictedPlayerState.weaponstate == 1
         || ps->weaponstate == 2
         || ps->weaponstate == 3
         || ps->weaponstate == 4
         || ps->weaponstate == 5)
        && ps->weaponstate != 1 )
    {
        return 0;
    }
    weapIdx = CG_AltWeaponToggleIndex(localClientNum, cgameGlob, &primaryWeapIdx);
    if ( !weapIdx )
        return 0;
    if ( primaryWeapIdx )
        CG_SelectWeaponIndex(localClientNum, primaryWeapIdx);
    newWeaponDef = BG_GetWeaponDef(weapIdx);
    v2 = !BG_GetAmmoInClip(ps, weapIdx) && !BG_GetAmmoNotInClip(ps, weapIdx) && !newWeaponDef->unlimitedAmmo;
    if ( v2 && newWeaponDef->inventoryType == WEAPINVENTORY_ALTMODE )
    {
        if ( cgameGlob->invalidCmdHintType != INVALID_CMD_NO_AMMO_BULLETS )
        {
            cgameGlob->invalidCmdHintType = INVALID_CMD_NO_AMMO_BULLETS;
            cgameGlob->invalidCmdHintTime = cgameGlob->time;
        }
        return 0;
    }
    else
    {
        CG_SelectWeaponIndex(localClientNum, weapIdx);
        return 1;
    }
}

bool __cdecl ActionSlotUsageAllowed(cg_s *cgameGlob)
{
    int i; // [esp+4h] [ebp-40h]
    usercmd_s cmd; // [esp+8h] [ebp-3Ch] BYREF
    int newCmdIndex; // [esp+40h] [ebp-4h]

    if ( !cgameGlob
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 5503, 0, "%s", "cgameGlob") )
    {
        __debugbreak();
    }
    if ( Demo_IsPlaying() )
        return 0;
    if ( !cgameGlob->nextSnap )
        return 0;
    if ( cgameGlob->predictedPlayerState.weaponstate == 3
        || cgameGlob->predictedPlayerState.weaponstate == 4
        || cgameGlob->predictedPlayerState.weaponstate == 5 )
    {
        return 0;
    }
    if ( (cgameGlob->predictedPlayerState.pm_flags & 0xC00) != 0 || cgameGlob->predictedPlayerState.weaponstate == 35 )
        return 0;
    if ( (cgameGlob->nextSnap->ps.otherFlags & 4) == 0 )
        return 0;
    if ( (cgameGlob->predictedPlayerState.pm_flags & 0x400000) != 0 )
        return 0;
    if ( cgameGlob->predictedPlayerState.pm_type >= 9 )
        return 0;
    if ( cgameGlob->predictedPlayerState.pm_type == 6 || cgameGlob->predictedPlayerState.pm_type == 7 )
        return 0;
    for ( i = 0; i < 2; ++i )
        cmd.button_bits.array[i] = 0;
    newCmdIndex = CL_GetCurrentCmdNumber(cgameGlob->localClientNum);
    if ( newCmdIndex <= 1 )
        return 1;
    CL_GetUserCmd(cgameGlob->localClientNum, newCmdIndex, &cmd);

    //if ( bitarray<51>::testBit(&cmd.button_bits, 4u) )
    if ( cmd.button_bits.testBit(4) )
        return 0;

    //return !bitarray<51>::testBit(&cmd.button_bits, 5u);
    return !cmd.button_bits.testBit(5);
}

char __cdecl ActionParms(int *slotResult)
{
    const char *v2; // eax
    int slot; // [esp+10h] [ebp-4h]

    if ( Cmd_Argc() >= 2 )
    {
        v2 = Cmd_Argv(1);
        slot = atoi(v2);
        if ( slot >= 1 && slot <= 4 )
        {
            *slotResult = slot - 1;
            return 1;
        }
        else
        {
            Com_Printf(0, "+/-actionslot; number given is out of range.    Was %i, expected 1 thru %i.\n", slot, 4);
            return 0;
        }
    }
    else
    {
        Com_Printf(0, "USAGE: +/-actionslot <number>\n");
        return 0;
    }
}

bool __cdecl CG_CheckPuttingAwayKillstreakWeapon(unsigned int oldWeapon, unsigned int newWeapon)
{
    const WeaponDef *oldWeapDef; // [esp+0h] [ebp-8h]
    const WeaponDef *newWeapDef; // [esp+4h] [ebp-4h]

    oldWeapDef = BG_GetWeaponDef(oldWeapon);
    newWeapDef = BG_GetWeaponDef(newWeapon);
    return oldWeapDef->bIsCarriedKillstreakWeapon && newWeapDef->weapClass == WEAPCLASS_KILLSTREAK_ALT_STORED_WEAPON;
}

void __cdecl CG_ActionSlotUp_f()
{
    cg_s *cgameGlob; // [esp+8h] [ebp-Ch]
    int slot; // [esp+Ch] [ebp-8h] BYREF
    int localClientNum; // [esp+10h] [ebp-4h]

    slot = 0;
    localClientNum = 0;
    cgameGlob = CG_GetLocalClientGlobals(0);
    if ( ActionSlotUsageAllowed(cgameGlob) )
        ActionParms(&slot);
}

void __cdecl CG_EjectWeaponBrass(int localClientNum, const entityState_s *ent, int event, unsigned int weapon)
{
    int number; // [esp+0h] [ebp-2Ch]
    const FxEffectDef *viewShellEjectEffect; // [esp+4h] [ebp-28h]
    const FxEffectDef *viewLastShotEjectEffect; // [esp+8h] [ebp-24h]
    bool v7; // [esp+Ch] [ebp-20h]
    snapshot_s *nextSnap; // [esp+10h] [ebp-1Ch]
    unsigned int tagName; // [esp+14h] [ebp-18h]
    const FxEffectDef *fxDef; // [esp+24h] [ebp-8h]
    const WeaponDef *weaponDef; // [esp+28h] [ebp-4h]

    if ( cg_brass->current.enabled && ent->eType < 21 && weapon )
    {
        if ( weapon < BG_GetNumWeapons() )
        {
            nextSnap = CG_GetLocalClientGlobals(localClientNum)->nextSnap;
            v7 = (nextSnap->ps.otherFlags & 6) != 0 && ent->number == nextSnap->ps.clientNum;
            weaponDef = BG_GetWeaponDef(weapon);
            if ( weaponDef->viewLastShotEjectEffect && weaponDef->worldLastShotEjectEffect && (event == 30 || event == 32) )
            {
                if ( v7 )
                    viewLastShotEjectEffect = weaponDef->viewLastShotEjectEffect;
                else
                    viewLastShotEjectEffect = weaponDef->worldLastShotEjectEffect;
                fxDef = viewLastShotEjectEffect;
            }
            else
            {
                if ( v7 )
                    viewShellEjectEffect = weaponDef->viewShellEjectEffect;
                else
                    viewShellEjectEffect = weaponDef->worldShellEjectEffect;
                fxDef = viewShellEjectEffect;
            }
            tagName = scr_const.tag_brass;
            switch ( event )
            {
                case 174:
                    tagName = scr_const.tag_gunner_brass1;
                    break;
                case 178:
                    tagName = scr_const.tag_gunner_brass1a;
                    break;
                case 175:
                    tagName = scr_const.tag_gunner_brass2;
                    break;
                case 179:
                    tagName = scr_const.tag_gunner_brass2a;
                    break;
                case 48:
                    tagName = scr_const.tag_brass_2;
                    break;
            }
            if ( event == 31 || event == 32 )
                tagName = scr_const.tag_brass1;
            if ( fxDef )
            {
                if ( v7 )
                    number = CG_WeaponDObjHandle(localClientNum);
                else
                    number = ent->number;
                CG_PlayBoltedEffect(localClientNum, fxDef, number, tagName);
            }
        }
        else
        {
            Com_Error(ERR_DROP, "CG_EjectWeaponBrass: ent->weapon >= BG_GetNumWeapons()");
        }
    }
}

float force = 50000.0f;
void __cdecl CG_VehicleJolt(centity_s *cent, float *origin, float *dir)
{
    float v3; // [esp+8h] [ebp-Ch]
    int savedregs; // [esp+14h] [ebp+0h] BYREF

    if ( cent->nitrousVeh
        && cent->nitrousVeh->m_phys_user_data
        && BG_GetWeaponDef(cent->nextState.weapon)->weapType == WEAPTYPE_PROJECTILE )
    {
        v3 = -force;
        *dir = (-(force)) * *dir;
        dir[1] = v3 * dir[1];
        dir[2] = v3 * dir[2];
        if ( !cent->nitrousVeh->m_phys_user_data
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                        5885,
                        0,
                        "%s\n\t%s",
                        "cent->nitrousVeh->get_phys_user_data()",
                        "CG_VehicleJolt:    Vehicle did not have valid physics user data.") )
        {
            __debugbreak();
        }
        Phys_ObjAddForce((int)cent->nitrousVeh->m_phys_user_data, origin, dir, 0);
    }
}

bool __cdecl CG_FireWeapon_GenerateMark(centity_s *cent, const playerState_s *ps, const WeaponDef *weaponDef)
{
    return sv_clientSideBullets->current.enabled && weaponDef->weapType == WEAPTYPE_BULLET;
}

bool __cdecl CG_FireWeapon_LocalPlayerUsingVehicleWeapon(centity_s *cent, const playerState_s *ps, int event)
{
    if ( (cent->nextState.eType == 14 || cent->nextState.eType == 12)
        && (ps->eFlags & 0x4000) != 0
        && ps->viewlocked_entNum == cent->nextState.number )
    {
        if ( event < 174 || event > 181 )
        {
            if ( event == 29 || event == 173 && !ps->vehiclePos )
                return 1;
        }
        else if ( ps->vehiclePos - 1 == (event - 174) % 4 || event == 175 && !ps->vehiclePos )
        {
            return 1;
        }
    }
    return cent->nextState.eType == 50 && (ps->eFlags & 0x4000) != 0 && cent->nextState.eventParm == ps->viewlocked_entNum;
}

bool __cdecl CG_FireWeapon_LocalPlayerUsingTurretWeapon(centity_s *cent, const playerState_s *ps)
{
    return cent->nextState.eType == 11 && (ps->eFlags & 0x300) != 0 && ps->viewlocked_entNum == cent->nextState.number;
}

void __cdecl CG_FireWeapon_VehicleFire(
                int localClientNum,
                centity_s *cent,
                const playerState_s *ps,
                int event,
                unsigned __int16 tagName)
{
    centity_s *playerCent; // [esp+Ch] [ebp-4h]

    if ( cent->nextState.eType == 14 )
    {
        if ( event == 29 || event == 173 )
            CScr_NotifyNum(localClientNum, cent->nextState.number, 0, cscr_const.weapon_fired, 0);
        if ( CL_LocalClient_IsFirstActive(localClientNum) )
        {
            if ( event == 29 || event == 173 )
                CG_VehicleFire(cent, tagName);
            if ( event == 173 && cent->vehicle )
                cent->vehicle->fireTime = CG_GetLocalClientGlobals(localClientNum)->time;
        }
        playerCent = CG_GetEntity(localClientNum, ps->clientNum);
        if ( playerCent->nextState.eType == 1 )
            CG_CompassAddWeaponPingInfo(localClientNum, playerCent, playerCent->pose.origin, 200);
    }
}

void __cdecl CG_VehicleFire(centity_s *cent, unsigned __int16 tagName)
{
    float mtx[4][3]; // [esp+0h] [ebp-34h] BYREF
    DObj *obj; // [esp+30h] [ebp-4h]

    if ( cent->nitrousVeh )
    {
        if ( cent->nitrousVeh->m_vehicle_info->type == 2 )
        {
            obj = Com_GetClientDObj(cent->nextState.number, cent->pose.localClientNum);
            if ( obj )
            {
                if ( CG_DObjGetWorldTagMatrix(&cent->pose, obj, tagName, mtx, mtx[3]) )
                    CG_VehicleJolt(cent, mtx[3], mtx[0]);
            }
        }
    }
}

void __cdecl CG_FireWeapon_OriginAndDirection(
                int localClientNum,
                centity_s *cent,
                unsigned __int16 tagName,
                bool isPlayer,
                float *origin,
                float *direction)
{
    float angles[3]; // [esp+0h] [ebp-44h] BYREF
    DObj *obj; // [esp+Ch] [ebp-38h]
    cg_s *cgameGlob; // [esp+10h] [ebp-34h]
    const ViewModelInfo *viewModelInfo; // [esp+14h] [ebp-30h]
    const cpose_t *pose; // [esp+18h] [ebp-2Ch]
    const entityState_s *ent; // [esp+1Ch] [ebp-28h]
    float orientation[3][3]; // [esp+20h] [ebp-24h] BYREF

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    ent = &cent->nextState;
    viewModelInfo = CG_GetLocalClientViewModelInfo(localClientNum);
    if ( !isPlayer || player_topDownCamMode->current.integer )
    {
        obj = Com_GetClientDObj(ent->number, localClientNum);
        pose = &cent->pose;
    }
    else
    {
        obj = viewModelInfo->viewModelDObj;
        pose = &cgameGlob->viewModelPose;
    }
    if ( !obj || !CG_DObjGetWorldTagMatrix(pose, obj, tagName, orientation, origin) )
    {
        BG_EvaluateTrajectory(&ent->lerp.pos, cgameGlob->time, origin);
        BG_EvaluateTrajectory(&ent->lerp.apos, cgameGlob->time, angles);
        AnglesToAxis(angles, orientation);
    }
    *(_QWORD *)direction = *(_QWORD *)&orientation[0][0];
    direction[2] = orientation[0][2];
}

void __cdecl CG_FireWeapon(
                int localClientNum,
                centity_s *cent,
                int event,
                unsigned __int16 tagName,
                unsigned int weapon,
                const playerState_s *ps,
                bool leftGun)
{
    int v7; // edx
    bool v8; // [esp+0h] [ebp-B4h]
    bool v9; // [esp+4h] [ebp-B0h]
    snapshot_s *nextSnap; // [esp+20h] [ebp-94h]
    SndEntHandle shooter; // [esp+34h] [ebp-80h]
    float entMatrix[3][3]; // [esp+44h] [ebp-70h] BYREF
    float fireOrigin[3]; // [esp+68h] [ebp-4Ch] BYREF
    bool shouldPing; // [esp+76h] [ebp-3Eh]
    bool firstPersonFlash; // [esp+77h] [ebp-3Dh]
    float origin[3]; // [esp+78h] [ebp-3Ch] BYREF
    cg_s *cgameGlob; // [esp+84h] [ebp-30h]
    bool isPlayer; // [esp+8Bh] [ebp-29h]
    const WeaponDef *weaponDef; // [esp+8Ch] [ebp-28h]
    bool isLocalPlayer; // [esp+92h] [ebp-22h]
    bool localPlayerUsingVehicleWeapon; // [esp+93h] [ebp-21h]
    const ViewModelInfo *viewModelInfo; // [esp+94h] [ebp-20h]
    const entityState_s *ent; // [esp+9Ch] [ebp-18h]
    float direction[3]; // [esp+A0h] [ebp-14h] BYREF
    const WeaponVariantDef *weaponVariantDef; // [esp+ACh] [ebp-8h]
    bool playerUsingTurret; // [esp+B3h] [ebp-1h]

    PROF_SCOPED("CG_FireWeapon");

    ent = &cent->nextState;
    if ( !weapon )
        weapon = ent->weapon;
    if ( !weapon )
    {
        return;
    }
    if ( weapon < BG_GetNumWeapons() )
    {
        viewModelInfo = CG_GetLocalClientViewModelInfo((int)localClientNum);
        weaponVariantDef = BG_GetWeaponVariantDef(weapon);
        weaponDef = BG_GetWeaponDef(weapon);
        if ( !weaponDef
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 6101, 0, "%s", "weaponDef") )
        {
            __debugbreak();
        }
        if ( leftGun )
            v7 = *((unsigned int *)cent + 201) | 8;
        else
            v7 = *((unsigned int *)cent + 201) | 4;
        *((unsigned int *)cent + 201) = v7;
        cgameGlob = CG_GetLocalClientGlobals((int)localClientNum);
        isLocalPlayer = 0;
        nextSnap = cgameGlob->nextSnap;
        v9 = (nextSnap->ps.otherFlags & 6) != 0 && ent->number == nextSnap->ps.clientNum;
        isPlayer = v9;
        if ( ent->eType == 1 )
            isLocalPlayer = CG_IsLocalPlayer(ent->number);
        localPlayerUsingVehicleWeapon = CG_FireWeapon_LocalPlayerUsingVehicleWeapon(cent, ps, event);
        playerUsingTurret = CG_FireWeapon_LocalPlayerUsingTurretWeapon(cent, ps);
        if ( localPlayerUsingVehicleWeapon || playerUsingTurret )
        {
            isLocalPlayer = 1;
            isPlayer = 1;
        }
        if ( Demo_IsMovieCamera() )
        {
            isLocalPlayer = 0;
            isPlayer = 0;
        }
        if ( CG_FireWeapon_GenerateMark(cent, ps, weaponDef) )
            DrawBulletImpacts((int)localClientNum, cent, tagName, ps, weapon, event, isPlayer || isLocalPlayer);
        if ( isPlayer && cent->pose.eType != 12 )
        {
            CG_UpdateViewModelPose(viewModelInfo->viewModelDObj, (int)localClientNum);
            BG_WeaponFireRecoil(&cgameGlob->predictedPlayerState, cgameGlob->recoilSpeed, cgameGlob->kickAVel);
        }
        CG_FireWeapon_VehicleFire((int)localClientNum, cent, ps, event, tagName);
        //BLOPS_NULLSUB();
        if ( ent->eType == 11 || weaponDef->fireType == WEAPON_FIRETYPE_MINIGUN && cent->pose.eType == 12 )
        {
            firstPersonFlash = localPlayerUsingVehicleWeapon || playerUsingTurret;
            WeaponFlash((int)localClientNum, ent->number, weapon, localPlayerUsingVehicleWeapon || playerUsingTurret, tagName);
        }
        CG_FireWeapon_OriginAndDirection((int)localClientNum, cent, tagName, isPlayer, origin, direction);
        shooter.field = CG_SndEntHandle((int)localClientNum, ent->number).field;
        CG_WeaponFireSingle(
            (int)localClientNum,
            shooter,
            weapon,
            isPlayer,
            isLocalPlayer,
            event == 30,
            origin,
            direction,
            tagName,
            weaponVariantDef->bRapidFire);
        if ( cent->nextState.eType == 1 )
        {
            shouldPing = !weaponVariantDef->bSilenced;
            v8 = shouldPing && weaponDef->weapType != WEAPTYPE_GRENADE && weaponDef->weapType != WEAPTYPE_MINE;
            shouldPing = v8;
            if ( v8 )
            {
                CG_CompassAddWeaponPingInfo((int)localClientNum, cent, origin, 200);
                //BLOPS_NULLSUB();
            }
        }
        if ( !BG_GetWeaponDef(weapon)->bBoltAction )
            CG_EjectWeaponBrass((int)localClientNum, ent, event, weapon);
        if ( isPlayer )
            TakeClipOnlyWeaponIfEmpty((int)localClientNum, &cgameGlob->predictedPlayerState);
        if ( CG_IsCharacter(isPlayer, cent->pose.eType) )
        {
            fireOrigin[0] = cent->pose.origin[0] + 0.0;
            fireOrigin[1] = cent->pose.origin[1] + 0.0;
            fireOrigin[2] = cent->pose.origin[2] + 60.0;
            AnglesToAxis(cent->pose.angles, entMatrix);
            R_FoliageNofityWeaponFiring(
                (int)localClientNum,
                (unsigned int)cent,
                fireOrigin,
                entMatrix[0],
                weaponDef->weapClass == WEAPCLASS_GAS);
        }
    }
    else
    {
        Com_Error(ERR_DROP, "CG_FireWeapon: weapon >= BG_GetNumWeapons()");
    }
}

void __cdecl DrawBulletImpacts(
                int localClientNum,
                const centity_s *ent,
                unsigned __int16 boneName,
                const playerState_s *ps,
                unsigned int weapon,
                int event,
                bool isPlayer)
{
    float v7; // [esp+20h] [ebp-1FCh]
    float v8; // [esp+24h] [ebp-1F8h]
    bool v9; // [esp+2Ch] [ebp-1F0h]
    float v10; // [esp+58h] [ebp-1C4h]
    float v11; // [esp+60h] [ebp-1BCh]
    float v12; // [esp+6Ch] [ebp-1B0h]
    float v13; // [esp+70h] [ebp-1ACh]
    float v14; // [esp+A4h] [ebp-178h]
    float v15; // [esp+ACh] [ebp-170h]
    snapshot_s *nextSnap; // [esp+E0h] [ebp-13Ch]
    const vehicle_info_t *info; // [esp+E4h] [ebp-138h]
    float forward[3]; // [esp+E8h] [ebp-134h] BYREF
    float up[3]; // [esp+F4h] [ebp-128h] BYREF
    cg_s *otherLocalClientGlob; // [esp+100h] [ebp-11Ch]
    float velocity[3]; // [esp+104h] [ebp-118h] BYREF
    unsigned __int8 boneIndex; // [esp+113h] [ebp-109h] BYREF
    int weaponNum; // [esp+114h] [ebp-108h]
    int ads; // [esp+118h] [ebp-104h]
    int shotCount; // [esp+11Ch] [ebp-100h]
    float origin[3]; // [esp+120h] [ebp-FCh] BYREF
    float range; // [esp+12Ch] [ebp-F0h]
    float dist; // [esp+130h] [ebp-ECh]
    cg_s *cgameGlob; // [esp+134h] [ebp-E8h]
    int shot; // [esp+138h] [ebp-E4h]
    int dobjNumber; // [esp+13Ch] [ebp-E0h]
    const WeaponDef *weaponDef; // [esp+140h] [ebp-DCh]
    orientation_t orient; // [esp+144h] [ebp-D8h] BYREF
    bool drawTracers; // [esp+177h] [ebp-A5h]
    float minSpread; // [esp+178h] [ebp-A4h] BYREF
    float maxSpread; // [esp+17Ch] [ebp-A0h] BYREF
    float viewang[3]; // [esp+180h] [ebp-9Ch] BYREF
    float tracerStart[3]; // [esp+18Ch] [ebp-90h] BYREF
    clientInfo_t *ci; // [esp+198h] [ebp-84h]
    int secondBarrel; // [esp+19Ch] [ebp-80h]
    BulletFireParams v41; // [esp+1A4h] [ebp-78h] BYREF
    float aimSpreadScale; // [esp+1E4h] [ebp-38h]
    float aimSpreadAmount; // [esp+1E8h] [ebp-34h]
    orientation_t gunOrient; // [esp+1ECh] [ebp-30h] BYREF

    PROF_SCOPED("DrawBulletImpacts");

    weaponDef = BG_GetWeaponDef(weapon);
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    secondBarrel = 0;
    drawTracers = 1;
    nextSnap = cgameGlob->nextSnap;
    v9 = (nextSnap->ps.otherFlags & 6) != 0 && ent->nextState.number == nextSnap->ps.clientNum;
    if ( v9 && !Demo_IsThirdPersonCamera() && !Demo_IsMovieCamera() )
    {
        weaponNum = BG_GetViewmodelWeaponIndex(ps);
        dobjNumber = CG_WeaponDObjHandle(localClientNum);
        aimSpreadScale = cgameGlob->lastFrame.aimSpreadScale / 255.0;
        ads = ps->fWeaponPosFrac == 1.0;
        BG_GetSpreadForWeapon(ps, weaponDef, &minSpread, &maxSpread);
        if ( !CG_GetPlayerViewOrigin(localClientNum, ps, origin) )
        {
            return;
        }
        viewang[0] = cgameGlob->gunPitch;
        viewang[1] = cgameGlob->gunYaw;
        viewang[2] = 0.0f;
        if ( zombietron->current.enabled )
        {
            dobjNumber = cgameGlob->nextSnap->ps.clientNum;
            boneIndex = 0;
            if ( !CG_GetBoneIndex(localClientNum, dobjNumber, boneName, &boneIndex) )
            {
                return;
            }
            if ( !FX_GetBoneOrientation(localClientNum, dobjNumber, boneIndex, &gunOrient) )
            {
                return;
            }
            tracerStart[0] = gunOrient.origin[0];
            tracerStart[1] = gunOrient.origin[1];
            tracerStart[2] = gunOrient.origin[2];
            AngleVectors(cgameGlob->predictedPlayerState.viewangles, orient.axis[0], orient.axis[1], orient.axis[2]);
        }
        else if ( cgameGlob->renderingThirdPerson == TP_FOR_MODEL )
        {
            ci = &cgameGlob->bgs.clientinfo[ent->nextState.number];
            tracerStart[0] = origin[0];
            tracerStart[1] = origin[1];
            tracerStart[2] = origin[2];
            AngleVectors(ci->playerAngles, orient.axis[0], orient.axis[1], orient.axis[2]);
            tracerStart[0] = (float)(30.0 * orient.axis[0][0]) + tracerStart[0];
            tracerStart[1] = (float)(30.0 * orient.axis[0][1]) + tracerStart[1];
            tracerStart[2] = (float)(30.0 * orient.axis[0][2]) + tracerStart[2];
        }
        else
        {
            boneIndex = 0;
            if ( !CG_GetBoneIndex(localClientNum, dobjNumber, boneName, &boneIndex) )
            {
                return;
            }
            if ( !FX_GetBoneOrientation(localClientNum, dobjNumber, boneIndex, &gunOrient) )
            {
                return;
            }
            tracerStart[0] = gunOrient.origin[0];
            tracerStart[1] = gunOrient.origin[1];
            tracerStart[2] = gunOrient.origin[2];
            AngleVectors(viewang, orient.axis[0], orient.axis[1], orient.axis[2]);
        }
        goto LABEL_94;
    }
    switch ( ent->nextState.eType )
    {
        case 1:
            CG_GuessSpreadForWeapon(localClientNum, ent, weaponDef, &minSpread, &maxSpread);
            ci = &cgameGlob->bgs.clientinfo[ent->nextState.number];
            ads = CG_IsPlayerADS(ci, ent);
            velocity[0] = ent->nextState.lerp.pos.trBase[0] - ent->currentState.pos.trBase[0];
            velocity[1] = ent->nextState.lerp.pos.trBase[1] - ent->currentState.pos.trBase[1];
            velocity[2] = ent->nextState.lerp.pos.trBase[2] - ent->currentState.pos.trBase[2];
            if ( cgameGlob->nextSnap->serverTime != cgameGlob->snap->serverTime )
            {
                v15 = 1000.0 / (float)(cgameGlob->nextSnap->serverTime - cgameGlob->snap->serverTime);
                velocity[0] = v15 * velocity[0];
                velocity[1] = v15 * velocity[1];
                velocity[2] = v15 * velocity[2];
            }
            v14 = Vec3Length(velocity);
            if ( (float)(v14 - 255.0) < 0.0 )
                v8 = v14;
            else
                v8 = 255.0f;
            dist = v8;
            aimSpreadScale = v8 / 255.0;
            dobjNumber = ent->nextState.number;
            boneIndex = 0;
            if ( !CG_GetBoneIndex(localClientNum, dobjNumber, boneName, &boneIndex) )
            {
                return;
            }
            if ( !FX_GetBoneOrientation(localClientNum, dobjNumber, boneIndex, &orient) )
            {
                return;
            }
            CG_CalcEyePoint(localClientNum, ent->nextState.number, origin);
            tracerStart[0] = orient.origin[0];
            tracerStart[1] = orient.origin[1];
            tracerStart[2] = orient.origin[2];
            otherLocalClientGlob = CG_GetLocalClientCGameGlobFromEntNum(ent->nextState.number);
            if ( otherLocalClientGlob && zombietron->current.enabled )
                AngleVectors(
                    otherLocalClientGlob->predictedPlayerState.viewangles,
                    orient.axis[0],
                    orient.axis[1],
                    orient.axis[2]);
            else
                AngleVectors(ci->playerAngles, orient.axis[0], orient.axis[1], orient.axis[2]);
LABEL_94:
            if ( ads )
                aimSpreadAmount = (float)((float)(maxSpread - weaponDef->fAdsSpread) * aimSpreadScale) + weaponDef->fAdsSpread;
            else
                aimSpreadAmount = (float)((float)(maxSpread - minSpread) * aimSpreadScale) + minSpread;
            memset((unsigned __int8 *)&v41, 0, sizeof(v41));
            v41.weaponEntIndex = 1022;
            v41.ignoreEntIndex = ent->nextState.number;
            if ( vehicle_selfCollision->current.enabled && ent->nextState.eType == 14 )
                v41.ignoreEntIndex = 1022;
            v41.damageMultiplier = 1.0f;
            v41.methodOfDeath = weaponDef->bRifleBullet + 1;
            if ( weaponDef->weapClass == WEAPCLASS_SPREAD )
            {
                shotCount = weaponDef->shotCount;
                range = weaponDef->fMinDamageRange;
            }
            else
            {
                shotCount = 1;
                range = sv_bullet_range->current.value;
            }
            v41.origStart[0] = origin[0];
            v41.origStart[1] = origin[1];
            v41.origStart[2] = origin[2];
            for ( shot = 0; shot < shotCount; ++shot )
            {
                *(_QWORD *)v41.dir = *(_QWORD *)&orient.axis[0][0];
                v41.dir[2] = orient.axis[0][2];
                v41.start[0] = origin[0];
                v41.start[1] = origin[1];
                v41.start[2] = origin[2];
                CG_BulletEndpos(
                    secondBarrel + shot + ps->commandTime,
                    aimSpreadAmount,
                    v41.start,
                    v41.end,
                    v41.dir,
                    orient.axis[0],
                    orient.axis[1],
                    orient.axis[2],
                    range);
                v41.damageMultiplier = 1.0f;
                BulletTrace_Start(localClientNum, &v41, weapon, ent, tracerStart, drawTracers, isPlayer);
            }
            //if ( GetCurrentThreadId() == g_DXDeviceThread )
                //D3DPERF_EndEvent();
            return;
        case 0xB:
            minSpread = weaponDef->playerSpread;
            maxSpread = weaponDef->playerSpread;
            aimSpreadScale = 1.0f;
            ads = 0;
            dobjNumber = ent->nextState.number;
            boneIndex = 0;
            if ( !CG_GetBoneIndex(localClientNum, dobjNumber, boneName, &boneIndex) )
            {
                return;
            }
            if ( !FX_GetBoneOrientation(localClientNum, dobjNumber, boneIndex, &orient) )
            {
                return;
            }
LABEL_52:
            origin[0] = orient.origin[0];
            origin[1] = orient.origin[1];
            origin[2] = orient.origin[2];
            tracerStart[0] = orient.origin[0];
            tracerStart[1] = orient.origin[1];
            tracerStart[2] = orient.origin[2];
            goto LABEL_94;
        case 0xE:
        case 0xC:
            if ( isPlayer )
            {
                minSpread = weaponDef->playerSpread;
                maxSpread = weaponDef->playerSpread;
            }
            else
            {
                minSpread = weaponDef->aiSpread;
                maxSpread = weaponDef->aiSpread;
            }
            if ( event >= 178 && event <= 181 )
                secondBarrel = 1;
            ads = 0;
            aimSpreadScale = 0.0f;
            dobjNumber = ent->nextState.number;
            boneIndex = 0;
            if ( !CG_GetBoneIndex(localClientNum, dobjNumber, boneName, &boneIndex) )
            {
                return;
            }
            if ( !FX_GetBoneOrientation(localClientNum, dobjNumber, boneIndex, &orient) )
            {
                return;
            }
            origin[0] = orient.origin[0];
            origin[1] = orient.origin[1];
            origin[2] = orient.origin[2];
            if ( isPlayer )
            {
                AngleVectors(ent->pose.angles, forward, 0, up);
                info = CG_GetVehicleInfo(ent->nextState.vehicleState.vehicleInfoIndex);
                v13 = info->tracerOffset[0];
                tracerStart[0] = (float)(v13 * forward[0]) + orient.origin[0];
                tracerStart[1] = (float)(v13 * forward[1]) + orient.origin[1];
                tracerStart[2] = (float)(v13 * forward[2]) + orient.origin[2];
                v12 = info->tracerOffset[1];
                tracerStart[0] = (float)(v12 * up[0]) + tracerStart[0];
                tracerStart[1] = (float)(v12 * up[1]) + tracerStart[1];
                tracerStart[2] = (float)(v12 * up[2]) + tracerStart[2];
            }
            else
            {
                tracerStart[0] = orient.origin[0];
                tracerStart[1] = orient.origin[1];
                tracerStart[2] = orient.origin[2];
            }
            goto LABEL_94;
        case 0x11:
            maxSpread = 0.0f;
            minSpread = 0.0f;
            ads = 0;
            velocity[0] = ent->nextState.lerp.pos.trBase[0] - ent->currentState.pos.trBase[0];
            velocity[1] = ent->nextState.lerp.pos.trBase[1] - ent->currentState.pos.trBase[1];
            velocity[2] = ent->nextState.lerp.pos.trBase[2] - ent->currentState.pos.trBase[2];
            if ( cgameGlob->nextSnap->serverTime != cgameGlob->snap->serverTime )
            {
                v11 = 1000.0 / (float)(cgameGlob->nextSnap->serverTime - cgameGlob->snap->serverTime);
                velocity[0] = v11 * velocity[0];
                velocity[1] = v11 * velocity[1];
                velocity[2] = v11 * velocity[2];
            }
            v10 = Vec3Length(velocity);
            if ( (float)(v10 - 255.0) < 0.0 )
                v7 = v10;
            else
                v7 = 255.0f;
            dist = v7;
            aimSpreadScale = v7 / 255.0;
            dobjNumber = ent->nextState.number;
            boneIndex = 0;
            if ( !CG_GetBoneIndex(localClientNum, dobjNumber, boneName, &boneIndex) )
            {
                return;
            }
            if ( !FX_GetBoneOrientation(localClientNum, dobjNumber, boneIndex, &orient) )
            {
                return;
            }
            goto LABEL_52;
        case 0x32:
            minSpread = 0.0f;
            maxSpread = 0.0f;
            aimSpreadScale = 0.0f;
            ads = 0;
            if ( weaponDef->weapClass == WEAPCLASS_SPREAD )
            {
                minSpread = weaponDef->fHipSpreadStandMin;
                maxSpread = weaponDef->fHipSpreadStandMin;
            }
            orient.origin[0] = ent->pose.origin[0];
            orient.origin[1] = ent->pose.origin[1];
            orient.origin[2] = ent->pose.origin[2];
            AngleVectors(ent->nextState.lerp.apos.trBase, orient.axis[0], orient.axis[1], orient.axis[2]);
            origin[0] = orient.origin[0];
            origin[1] = orient.origin[1];
            origin[2] = orient.origin[2];
            tracerStart[0] = orient.origin[0];
            tracerStart[1] = orient.origin[1];
            tracerStart[2] = orient.origin[2];
            goto LABEL_94;
        case 6:
            minSpread = 0.0f;
            maxSpread = 0.0f;
            aimSpreadScale = 0.0f;
            ads = 0;
            if ( weaponDef->weapClass == WEAPCLASS_SPREAD )
            {
                minSpread = weaponDef->fHipSpreadStandMin;
                maxSpread = weaponDef->fHipSpreadStandMin;
            }
            orient.origin[0] = ent->pose.origin[0];
            orient.origin[1] = ent->pose.origin[1];
            orient.origin[2] = ent->pose.origin[2];
            AngleVectors(ent->pose.angles, orient.axis[0], orient.axis[1], orient.axis[2]);
            origin[0] = orient.origin[0];
            origin[1] = orient.origin[1];
            origin[2] = orient.origin[2];
            tracerStart[0] = orient.origin[0];
            tracerStart[1] = orient.origin[1];
            tracerStart[2] = orient.origin[2];
            goto LABEL_94;
    }
    Com_PrintError(14, "Unknown eType %i in CG_DrawBulletImpacts()\n", ent->nextState.eType);
    //if ( GetCurrentThreadId() == g_DXDeviceThread )
        //D3DPERF_EndEvent();
}

void __cdecl CG_BulletEndpos(
                int randSeed,
                float spread,
                const float *start,
                float *end,
                float *dir,
                const float *forwardDir,
                const float *rightDir,
                const float *upDir,
                float maxRange)
{
    long double v9; // [esp+0h] [ebp-54h]
    float v10; // [esp+24h] [ebp-30h]
    float v11; // [esp+28h] [ebp-2Ch]
    float right; // [esp+48h] [ebp-Ch] BYREF
    float aimOffset; // [esp+4Ch] [ebp-8h]
    float up; // [esp+50h] [ebp-4h] BYREF

    if ( (LODWORD(spread) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 3334, 0, "%s", "!IS_NAN(spread)") )
    {
        __debugbreak();
    }
    if ( !end && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 3336, 0, "%s", "end") )
        __debugbreak();
    //__libm_sse2_tan(v9);
    //aimOffset = (float)(spread * 0.017453292) * maxRange;

    aimOffset = tanf(spread * 0.017453292f) * maxRange; // convert spread to radians and tan

    if ( (LODWORD(aimOffset) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                    3340,
                    0,
                    "%s",
                    "!IS_NAN(aimOffset)") )
    {
        __debugbreak();
    }
    RandomBulletDir(randSeed, &right, &up);
    right = right * aimOffset;
    up = up * aimOffset;
    if ( (LODWORD(right) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 3347, 0, "%s", "!IS_NAN(right)") )
    {
        __debugbreak();
    }
    if ( (LODWORD(up) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 3348, 0, "%s", "!IS_NAN(up)") )
    {
        __debugbreak();
    }
    *end = (float)(maxRange * *forwardDir) + *start;
    end[1] = (float)(maxRange * forwardDir[1]) + start[1];
    end[2] = (float)(maxRange * forwardDir[2]) + start[2];

    nanassertvec3(end);

    v11 = right;
    *end = (float)(right * *rightDir) + *end;
    end[1] = (float)(v11 * rightDir[1]) + end[1];
    end[2] = (float)(v11 * rightDir[2]) + end[2];
    v10 = up;
    *end = (float)(up * *upDir) + *end;
    end[1] = (float)(v10 * upDir[1]) + end[1];
    end[2] = (float)(v10 * upDir[2]) + end[2];

    nanassertvec3(end);

    if ( dir )
    {
        *dir = *end - *start;
        dir[1] = end[1] - start[1];
        dir[2] = end[2] - start[2];
        Vec3Normalize(dir);

        nanassertvec3(dir);

    }
}

void __cdecl RandomBulletDir(int randSeed, float *x, float *y)
{
    long double v3; // st7
    float sinT; // [esp+Ch] [ebp-10h]
    float theta; // [esp+10h] [ebp-Ch]
    float r; // [esp+14h] [ebp-8h]
    float cosT; // [esp+18h] [ebp-4h]

    if ( !x && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 3311, 0, "%s", "x") )
        __debugbreak();
    if ( !y && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 3312, 0, "%s", "y") )
        __debugbreak();
    theta = G_GoodRandomFloat(&randSeed) * 360.0;
    r = G_GoodRandomFloat(&randSeed);
    v3 = (float)(theta * 0.017453292);
    cosT = cos(v3);
    sinT = sin(v3);
    *x = r * cosT;
    *y = r * sinT;
}

void __cdecl TakeClipOnlyWeaponIfEmpty(int localClientNum, playerState_s *ps)
{
    if ( BG_WeaponIsClipOnly(ps->weapon)
        && !BG_GetAmmoInClip(ps, ps->weapon)
        && !BG_GetAmmoNotInClip(ps, ps->weapon)
        && !BG_GetWeaponDef(ps->weapon)->hasDetonator
        && BG_GetWeaponDef(ps->weapon)->offhandSlot != OFFHAND_SLOT_EQUIPMENT
        && BG_GetWeaponDef(ps->weapon)->offhandSlot != OFFHAND_SLOT_SPECIFIC_USE )
    {
        BG_TakePlayerWeapon(ps, ps->weapon);
        CG_OutOfAmmoChange(localClientNum);
    }
}

bool __cdecl CG_IsCharacter(bool isPlayer, int eType)
{
    return isPlayer || eType == 1;
}

void __cdecl CG_FakeFireWeapon(
                int localClientNum,
                centity_s *cent,
                unsigned int weapon,
                float *origin,
                unsigned int count)
{
    SndEntHandle v5; // eax
    float angles[3]; // [esp+Ch] [ebp-1Ch] BYREF
    const entityState_s *ent; // [esp+18h] [ebp-10h]
    float direction[3]; // [esp+1Ch] [ebp-Ch] BYREF

    ent = &cent->nextState;
    if ( !weapon )
        weapon = ent->weapon;
    if ( weapon )
    {
        if ( weapon < BG_GetNumWeapons() )
        {
            if ( !BG_GetWeaponDef(weapon)
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 6284, 0, "%s", "weaponDef") )
            {
                __debugbreak();
            }
            CG_GetLocalClientGlobals(localClientNum);
            angles[0] = 0.0f;
            angles[2] = 0.0f;
            angles[1] = (float)irand(0, 360);
            AngleVectors(angles, direction, 0, 0);
            v5.field = CG_SndEntHandle(localClientNum, ent->number).field;
            CG_WeaponFireFake(localClientNum, v5, weapon, origin, direction, count);
        }
        else
        {
            Com_Error(ERR_DROP, "CG_FakeFireWeapon: weapon >= BG_GetNumWeapons()");
        }
    }
}

void __cdecl CG_SndAutoSimReset()
{
    if ( !snd_autoSim )
        snd_autoSim = _Dvar_RegisterBool("snd_autoSim", 1, 0x80u, "turn on client side simulation of automatic gun sounds");
    memset((unsigned __int8 *)g_snd_autosims, 0, sizeof(g_snd_autosims));
    memset((unsigned __int8 *)g_snd_autosim_history, 0, sizeof(g_snd_autosim_history));
    snd_autosim_time = Sys_Milliseconds();
    snd_autosim_frame = 5;
}

void __cdecl CG_SndKillAutoSimEnt(SndEntHandle ent)
{
    unsigned int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < 0x40; ++i )
    {
        if ( g_snd_autosims[i].shot.shooter.handle == ent.handle )
            g_snd_autosims[i].used = 0;
    }
}

void __cdecl CG_SndUpdateAutoSim()
{
    DWORD v1; // eax
    double v2; // xmm0_8
    long double v3; // [esp+0h] [ebp-ACh]
    unsigned int n; // [esp+5Ch] [ebp-50h]
    float da[3]; // [esp+60h] [ebp-4Ch] BYREF
    float db[3]; // [esp+6Ch] [ebp-40h] BYREF
    float maskerDistance; // [esp+78h] [ebp-34h]
    float maskedDistance; // [esp+7Ch] [ebp-30h]
    snd_autosim_play *masked; // [esp+80h] [ebp-2Ch]
    snd_autosim_play *masker; // [esp+84h] [ebp-28h]
    int j; // [esp+88h] [ebp-24h]
    unsigned int m; // [esp+8Ch] [ebp-20h]
    unsigned int k; // [esp+90h] [ebp-1Ch]
    centity_s *cent; // [esp+94h] [ebp-18h]
    snd_autosim *sim; // [esp+98h] [ebp-14h]
    const WeaponDef *weapDef; // [esp+9Ch] [ebp-10h]
    unsigned int i; // [esp+A0h] [ebp-Ch]
    unsigned int dt; // [esp+A4h] [ebp-8h]
    cg_s *cgameGlob; // [esp+A8h] [ebp-4h]

    v1 = Sys_Milliseconds();
    dt = v1 - snd_autosim_time;
    snd_autosim_time = v1;
    ++snd_autosim_frame;
    if (!cl_paused->current.integer)
    {
        for (i = 0; i < 0x40; ++i)
        {
            sim = &g_snd_autosims[i];
            if (sim->used)
            {
                sim->lastPing += dt;
                cent = CG_GetEntity(sim->shot.localClientNum, sim->shot.shooter.handle & 0xFFF);
                weapDef = BG_GetWeaponDef(sim->shot.weapon);
                if (sim->lastPing <= 0xC8)
                {
                    if (cent->nextState.eType != 1 || (cent->nextState.lerp.eFlags & 0x40) != 0 || sim->shot.isLastShotInClip)
                    {
                        if (weapDef->fireType == WEAPON_FIRETYPE_BURSTFIRE3 && sim->shotCount >= 3)
                        {
                            if (sim->shotCount >= 4)
                            {
                                sim->used = 0;
                                sim->lastShot = 0;
                                sim->shotCount = 0;
                            }
                        }
                        else if (sim->shot.fakeFire && sim->shotCount >= sim->shot.burstCount)
                        {
                            sim->used = 0;
                        }
                        else
                        {
                            if (!sim->isNew)
                                sim->lastShot += dt;
                            if (sim->lastShot >= sim->fireTime || sim->isNew)
                            {
                                sim->lastShot = 0;
                                for (k = 0; k < 0x40; ++k)
                                {
                                    if (!g_snd_autosim_history[k].frame)
                                    {
                                        g_snd_autosim_history[k].frame = snd_autosim_frame;
                                        memcpy(&g_snd_autosim_history[k].shot, sim, sizeof(g_snd_autosim_history[k].shot));
                                        break;
                                    }
                                }
                                sim->isNew = 0;
                                ++sim->shotCount;
                                if (sim->shot.fakeFire)
                                    sim->lastPing = 0;
                            }
                        }
                    }
                    else
                    {
                        sim->used = 0;
                    }
                }
                else
                {
                    sim->used = 0;
                }
            }
        }
        cgameGlob = CG_GetLocalClientGlobals(0);
        if (snd_autosim_window->current.enabled)
        {
            for (m = 0; m < 0x40; ++m)
            {
                if (g_snd_autosim_history[m].frame)
                {
                    for (j = 0; (unsigned int)j < 0x40; ++j)
                    {
                        if (g_snd_autosim_history[j].frame)
                        {
                            if (m != j)
                            {
                                masker = &g_snd_autosim_history[m];
                                masked = &g_snd_autosim_history[j];
                                if (masker->shot.weapon == masked->shot.weapon)
                                {
                                    maskerDistance = Vec3Distance(cgameGlob->refdef.vieworg, masker->shot.origin);
                                    maskedDistance = Vec3Distance(cgameGlob->refdef.vieworg, masked->shot.origin);
                                    if (maskedDistance > maskerDistance)
                                    {
                                        da[0] = masker->shot.origin[0] - cgameGlob->refdef.vieworg[0];
                                        da[1] = masker->shot.origin[1] - cgameGlob->refdef.vieworg[1];
                                        da[2] = masker->shot.origin[2] - cgameGlob->refdef.vieworg[2];
                                        db[0] = masked->shot.origin[0] - cgameGlob->refdef.vieworg[0];
                                        db[1] = masked->shot.origin[1] - cgameGlob->refdef.vieworg[1];
                                        db[2] = masked->shot.origin[2] - cgameGlob->refdef.vieworg[2];
                                        Vec3Normalize(da);
                                        Vec3Normalize(db);
                                        v2 = (float)((float)((float)(da[0] * db[0]) + (float)(da[1] * db[1])) + (float)(da[2] * db[2]));
                                        //__libm_sse2_acos(v3);
                                        //*(float *)&v2 = v2;
                                        if (acos(v2) < 0.52359879)
                                        {
                                            g_snd_autosim_history[j].frame = 0;
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        for (n = 0; n < 0x40; ++n)
        {
            if (g_snd_autosim_history[n].frame == snd_autosim_frame - 1)
            {
                CG_SndWeaponFire(&g_snd_autosim_history[n].shot);
                g_snd_autosim_history[n].frame = 0;
            }
        }
    }
}

void __cdecl CG_SndWeaponFire(snd_weapon_shot *shot)
{
    int firesound; // [esp+2Ch] [ebp-Ch]
    const WeaponDef *weaponDef; // [esp+34h] [ebp-4h]

    PROF_SCOPED("CG_SndWeaponFire");

    weaponDef = BG_GetWeaponDef(shot->weapon);
    if ( !weaponDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 6363, 0, "%s", "weaponDef") )
    {
        __debugbreak();
    }
    if ( shot->shooterIsLocalPlayer )
    {
        if ( shot->isLastShotInClip && weaponDef->fireLastSoundPlayer && *weaponDef->fireLastSoundPlayer )
            firesound = SND_FindAliasId((char *)weaponDef->fireLastSoundPlayer);
        else
            firesound = SND_FindAliasId((char *)weaponDef->fireSoundPlayer);
    }
    else if ( shot->isLastShotInClip && weaponDef->fireLastSound && *weaponDef->fireLastSound )
    {
        firesound = SND_FindAliasId((char *)weaponDef->fireLastSound);
    }
    else
    {
        firesound = SND_FindAliasId((char *)weaponDef->fireSound);
    }
    if ( (!shot->shooterIsLocalPlayer || shot->isTurret) && !shot->fakeFire )
        WeaponFlash(
            shot->localClientNum,
            shot->shooter.handle & 0xFFF,
            shot->weapon,
            shot->shooterIsLocalPlayer,
            shot->tagName);
    if ( shot->fakeFire )
        CG_SndWeaponFakeFire(shot, weaponDef);
    if ( !firesound )
    {
        return;
    }
    if ( CG_ShouldPlaySoundOnLocalClient(shot->localClientNum, shot->shooter.handle & 0xFFF, shot->origin, firesound) )
    {
        SND_Play(firesound, 0, 1.0, shot->shooter, shot->origin, shot->direction, 0);
    }
}

float range_0 = 30.0f;
void __cdecl CG_SndWeaponFakeFire(snd_weapon_shot *shot, const WeaponDef *weaponDef)
{
    float v3; // [esp+14h] [ebp-80h]
    float v4; // [esp+18h] [ebp-7Ch]
    float v5; // [esp+40h] [ebp-54h]
    float v6; // [esp+44h] [ebp-50h]
    float v7; // [esp+48h] [ebp-4Ch]
    centity_s *shot_ent; // [esp+54h] [ebp-40h]
    cg_s *cgameGlob; // [esp+68h] [ebp-2Ch]
    unsigned int shooter_index; // [esp+74h] [ebp-20h]
    float shotDirection[3]; // [esp+7Ch] [ebp-18h] BYREF
    float endPosition[3]; // [esp+88h] [ebp-Ch] BYREF

    cgameGlob = CG_GetLocalClientGlobals(shot->localClientNum);
    shot_ent = CG_GetEntity(shot->localClientNum, shot->shooter.handle & 0xFFF);
    shooter_index = shot->shooter.handle & 0xFFF;
    v5 = CG_flrand((-(range_0)), range_0);
    v6 = CG_flrand((-(range_0)), range_0);
    v7 = CG_flrand((-(range_0)), range_0);
    endPosition[0] = shot->origin[0] + v5;
    endPosition[1] = shot->origin[1] + v6;
    endPosition[2] = shot->origin[2] + v7;
    shotDirection[0] = cgameGlob->refdef.vieworg[0] - endPosition[0];
    shotDirection[1] = cgameGlob->refdef.vieworg[1] - endPosition[1];
    shotDirection[2] = cgameGlob->refdef.vieworg[2] - endPosition[2];
    Vec3NormalizeFast(shotDirection);
    if ( ((*((unsigned int *)shot_ent + 201) >> 1) & 1) != 0 )
    {
        shooter_index = shot_ent->nextState.otherEntityNum;
        CG_CompassAddFakeFirePingInfo(
            shot->localClientNum,
            shooter_index,
            shot->shooter.handle & 0xFFF,
            shot->origin,
            shotDirection,
            50);
    }
    CG_SndWeaponFakeFireNotify(shot->localClientNum, shot->shooter.handle & 0xFFF, shot_ent, shot->origin);
    v3 = cgameGlob->refdef.vieworg[0] - shot->origin[0];
    v4 = cgameGlob->refdef.vieworg[1] - shot->origin[1];
    if ( (float)((float)(v3 * v3) + (float)(v4 * v4)) <= 1000000.0
        && (cgameGlob->bgs.clientinfo[shooter_index].team != cgameGlob->bgs.clientinfo[cgameGlob->nextSnap->ps.clientNum].team
         || cgameGlob->bgs.clientinfo[cgameGlob->nextSnap->ps.clientNum].team == TEAM_FREE
         && shooter_index != cgameGlob->nextSnap->ps.clientNum)
        && (float)(cg_fakefireWizbyChance->current.value * 32768.0) > (float)rand() )
    {
        CG_SoundWhizby(shot->localClientNum, weaponDef, shot->origin, shotDirection, endPosition);
    }
}

void __cdecl CG_SndWeaponFakeFireNotify(int localClientNum, unsigned int entNum, centity_s *shot_ent, float *origin)
{
    unsigned __int16 t; // [esp+4h] [ebp-4h]

    if ( ((*((unsigned int *)shot_ent + 201) >> 1) & 1) != 0 && shot_ent->nextState.eType < 21 )
    {
        CScr_NotifyNum(localClientNum, entNum, 0, cscr_const.fake_fire, 0);
    }
    else
    {
        Scr_AddVector(origin, SCRIPTINSTANCE_CLIENT);
        Scr_AddConstString(cscr_const.fake_fire, SCRIPTINSTANCE_CLIENT);
        t = Scr_ExecThread(SCRIPTINSTANCE_CLIENT, cg_scr_data.levelnotify, 2u);
        Scr_FreeThread(t, SCRIPTINSTANCE_CLIENT);
    }
}

void __cdecl CG_WeaponFireSingle(
                int localClientNum,
                SndEntHandle shooter,
                unsigned int weapon,
                bool shooterIsPlayer,
                bool shooterIsLocalPlayer,
                bool isLastShotInClip,
                const float *origin,
                const float *direction,
                unsigned int tagName,
                bool doubleTap)
{
    snd_weapon_shot shot; // [esp+28h] [ebp-3Ch] BYREF
    const WeaponDef *weaponDef; // [esp+5Ch] [ebp-8h]
    bool killcam; // [esp+60h] [ebp-4h]
    bool simulate; // [esp+61h] [ebp-3h]
    bool isAuto; // [esp+63h] [ebp-1h]

    PROF_SCOPED("CG_WeaponFireSingle");

    nanassertvec3(origin);
    nanassertvec3(direction);

    weaponDef = BG_GetWeaponDef(weapon);
    if ( weaponDef->fireType == WEAPON_FIRETYPE_MINIGUN )
    {
        return;
    }
    else
    {
        shot.localClientNum = localClientNum;
        shot.shooter = shooter;
        shot.weapon = weapon;
        shot.shooterIsPlayer = shooterIsPlayer;
        shot.shooterIsLocalPlayer = shooterIsLocalPlayer;
        shot.isLastShotInClip = isLastShotInClip;
        shot.isTurret = weaponDef->weapClass == WEAPCLASS_TURRET;
        shot.doubleTap = doubleTap;
        shot.origin[0] = *origin;
        shot.origin[1] = origin[1];
        shot.origin[2] = origin[2];
        shot.direction[0] = *direction;
        shot.direction[1] = direction[1];
        shot.direction[2] = direction[2];
        shot.tagName = tagName;
        shot.fakeFire = 0;
        isAuto = weaponDef->fireType != WEAPON_FIRETYPE_SINGLESHOT;
        simulate = snd_autoSim->current.enabled;
        killcam = (shooter.handle & 0x400000) != 0;
        if ( !isAuto )
            simulate = 0;
        if ( !shot.isTurret && shooterIsLocalPlayer && !killcam )
            simulate = 0;
        if ( Demo_IsPlaying() && isAuto )
            simulate = 1;
        if ( simulate )
            CG_SndPingAutoSim(&shot);
        else
            CG_SndWeaponFire(&shot);
    }
}

void __cdecl CG_SndPingAutoSim(snd_weapon_shot *shot)
{
    unsigned __int16 aliasHash; // [esp+28h] [ebp-20h]
    unsigned int fireTime; // [esp+2Ch] [ebp-1Ch]
    const WeaponDef *weaponDef; // [esp+30h] [ebp-18h]
    unsigned int i; // [esp+38h] [ebp-10h]
    snd_autosim *found_sim; // [esp+3Ch] [ebp-Ch]
    snd_autosim *free_sim; // [esp+44h] [ebp-4h]

    PROF_SCOPED("CG_SndPingAutoSim");

    free_sim = 0;
    found_sim = 0;
    for ( i = 0; i < 0x40; ++i )
    {
        if ( g_snd_autosims[i].used )
        {
            if ( g_snd_autosims[i].shot.shooter.handle == shot->shooter.handle
                && g_snd_autosims[i].shot.weapon == shot->weapon
                && g_snd_autosims[i].shot.tagName == shot->tagName )
            {
                found_sim = &g_snd_autosims[i];
                break;
            }
        }
        else if ( !free_sim )
        {
            free_sim = &g_snd_autosims[i];
        }
    }
    if ( found_sim )
    {
        if ( !found_sim->used
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 6462, 0, "%s", "found_sim->used") )
        {
            __debugbreak();
        }
        found_sim->lastPing = 0;
        if ( found_sim->shot.fakeFire )
            found_sim->shotCount = 0;
        memcpy(found_sim, shot, 0x34u);
    }
    else if ( free_sim )
    {
        weaponDef = BG_GetWeaponDef(shot->weapon);
        if ( !weaponDef
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 6474, 0, "%s", "weaponDef") )
        {
            __debugbreak();
        }
        fireTime = weaponDef->iFireTime;
        if ( shot->doubleTap )
            fireTime = (__int64)((double)fireTime * perk_weapRateMultiplier->current.value);
        if ( free_sim->used
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 6485, 0, "%s", "!free_sim->used") )
        {
            __debugbreak();
        }
        memcpy(free_sim, shot, 0x34u);
        free_sim->lastPing = 0;
        free_sim->lastShot = fireTime;
        free_sim->fireTime = fireTime;
        free_sim->used = 1;
        free_sim->isNew = 1;
        free_sim->shotCount = 0;
        aliasHash = SND_HashName(weaponDef->fireSound);
        free_sim->fireTime += (int)(randomhash((aliasHash << 16) + (free_sim->shot.shooter.handle & 0xFFFu))
                                                            * snd_auto_random->current.value);
        free_sim->lastShot = free_sim->fireTime;
    }
    else
    {
        Com_Printf(9, "WARNING: ran out of automatic sims???\n");
        CG_SndWeaponFire(shot);
    }
}

void __cdecl CG_WeaponFireFake(
                int localClientNum,
                SndEntHandle shooter,
                unsigned int weapon,
                const float *origin,
                const float *direction,
                unsigned int count)
{
    snd_weapon_shot shot; // [esp+18h] [ebp-38h] BYREF
    const WeaponDef *weaponDef; // [esp+4Ch] [ebp-4h]

    nanassertvec3(origin);
    nanassertvec3(direction);

    weaponDef = BG_GetWeaponDef(weapon);
    shot.localClientNum = localClientNum;
    shot.shooter = shooter;
    shot.weapon = weapon;
    shot.shooterIsPlayer = 0;
    shot.shooterIsLocalPlayer = 0;
    shot.isLastShotInClip = 0;
    shot.isTurret = weaponDef->weapClass == WEAPCLASS_TURRET;
    shot.doubleTap = 0;
    shot.origin[0] = *origin;
    shot.origin[1] = origin[1];
    shot.origin[2] = origin[2];
    shot.direction[0] = *direction;
    shot.direction[1] = direction[1];
    shot.direction[2] = direction[2];
    shot.tagName = 0;
    shot.fakeFire = 1;
    shot.burstCount = count;
    CG_SndPingAutoSim(&shot);
}

void __cdecl CG_SpawnTracer(
                int localClientNum,
                const float *pstart,
                const float *pend,
                int type,
                float width,
                float length)
{
    int v6; // [esp+4h] [ebp-68h]
    float *trDelta; // [esp+20h] [ebp-4Ch]
    float value; // [esp+24h] [ebp-48h]
    float *trBase; // [esp+28h] [ebp-44h]
    float dir[3]; // [esp+38h] [ebp-34h] BYREF
    float dist; // [esp+44h] [ebp-28h]
    const cg_s *cgameGlob; // [esp+48h] [ebp-24h]
    float start[3]; // [esp+4Ch] [ebp-20h]
    float end[3]; // [esp+58h] [ebp-14h]
    int startTime; // [esp+64h] [ebp-8h]
    localEntity_s *le; // [esp+68h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->frametime )
        v6 = rand() % cgameGlob->frametime / 2;
    else
        v6 = 0;
    startTime = cgameGlob->time - v6;
    start[0] = *pstart;
    start[1] = pstart[1];
    start[2] = pstart[2];
    end[0] = *pend;
    end[1] = pend[1];
    end[2] = pend[2];
    dir[0] = end[0] - start[0];
    dir[1] = end[1] - start[1];
    dir[2] = end[2] - start[2];
    dist = Vec3Normalize(dir);
    le = CG_AllocLocalEntity(localClientNum);
    if ( type == 1 )
    {
        le->leType = LE_MOVING_TRACER_GREEN;
    }
    else if ( type == 2 )
    {
        le->leType = LE_MOVING_BULLET;
    }
    else
    {
        le->leType = LE_MOVING_TRACER_RED;
    }
    le->tracerClipDist = dist;
    le->tracerLength = length;
    le->tracerWidth = width;
    le->endTime = startTime + (int)(float)((float)(1000.0 * dist) / cg_tracerSpeed->current.value);
    le->pos.trType = 3;
    le->pos.trTime = startTime;
    trBase = le->pos.trBase;
    le->pos.trBase[0] = start[0];
    trBase[1] = start[1];
    trBase[2] = start[2];
    trDelta = le->pos.trDelta;
    value = cg_tracerSpeed->current.value;
    le->pos.trDelta[0] = value * dir[0];
    trDelta[1] = value * dir[1];
    trDelta[2] = value * dir[2];
    if ( ((LODWORD(le->pos.trBase[0]) & 0x7F800000) == 0x7F800000
         || (LODWORD(le->pos.trBase[1]) & 0x7F800000) == 0x7F800000
         || (LODWORD(le->pos.trBase[2]) & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                    6833,
                    0,
                    "%s",
                    "!IS_NAN((le->pos.trBase)[0]) && !IS_NAN((le->pos.trBase)[1]) && !IS_NAN((le->pos.trBase)[2])") )
    {
        __debugbreak();
    }
    if ( ((LODWORD(le->pos.trDelta[0]) & 0x7F800000) == 0x7F800000
         || (LODWORD(le->pos.trDelta[1]) & 0x7F800000) == 0x7F800000
         || (LODWORD(le->pos.trDelta[2]) & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                    6834,
                    0,
                    "%s",
                    "!IS_NAN((le->pos.trDelta)[0]) && !IS_NAN((le->pos.trDelta)[1]) && !IS_NAN((le->pos.trDelta)[2])") )
    {
        __debugbreak();
    }
}

void __cdecl CG_DrawTracer(const float *start, const float *finish, const refdef_s *refdef, localEntity_s *le)
{
    leType_t leType; // [esp+0h] [ebp-70h]
    float finishWidth; // [esp+14h] [ebp-5Ch] BYREF
    FxBeam beam; // [esp+18h] [ebp-58h] BYREF
    float startWidth; // [esp+68h] [ebp-8h] BYREF
    float tracerWidth; // [esp+6Ch] [ebp-4h]

    memset((unsigned __int8 *)&beam, 0, sizeof(beam));
    if ( le->tracerWidth == 0.0 )
        tracerWidth = cg_tracerWidth->current.value;
    else
        tracerWidth = le->tracerWidth;
    leType = le->leType;
    if ( leType == LE_MOVING_TRACER_GREEN )
    {
        startWidth = tracerWidth;
        finishWidth = tracerWidth;
        beam.material = cgMedia.greenTracerMaterial;
    }
    else if ( leType == LE_MOVING_BULLET )
    {
        startWidth = cg_bulletWidth->current.value;
        finishWidth = cg_bulletWidth->current.value;
        beam.material = cgMedia.bulletMaterial;
    }
    else
    {
        startWidth = tracerWidth;
        finishWidth = tracerWidth;
        beam.material = cgMedia.redTracerMaterial;
    }
    ScaleTracer(start, finish, refdef->vieworg, &startWidth, &finishWidth);
    beam.begin[0] = *start;
    beam.begin[1] = start[1];
    beam.begin[2] = start[2];
    beam.end[0] = *finish;
    beam.end[1] = finish[1];
    beam.end[2] = finish[2];
    beam.beginColor.packed = -1;
    beam.endColor.packed = -1;
    beam.beginRadius = startWidth;
    beam.endRadius = finishWidth;
    beam.segmentVerts = 0;
    beam.segmentCount = (int)(Vec3Distance(start, finish) * 8.0 / cg_tracerScrewDist->current.value);
    LODWORD(beam.wiggleDist) = cg_tracerScrewRadius->current.integer;
    beam.flags = 1;
    FX_Beam_Add(&beam);
}

void __cdecl ScaleTracer(
                const float *start,
                const float *finish,
                const float *viewOrg,
                float *startWidth,
                float *finishWidth)
{
    float startDist; // [esp+44h] [ebp-14h]
    float finishDist; // [esp+48h] [ebp-10h]
    float tracerScale; // [esp+4Ch] [ebp-Ch]
    int tracerScaleDistRange; // [esp+50h] [ebp-8h]
    float tracerScaleMinDist; // [esp+54h] [ebp-4h]

    if ( !startWidth
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 6864, 0, "%s", "startWidth") )
    {
        __debugbreak();
    }
    if ( !finishWidth
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 6865, 0, "%s", "finishWidth") )
    {
        __debugbreak();
    }
    if ( !cg_tracerScale
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 6866, 0, "%s", "cg_tracerScale") )
    {
        __debugbreak();
    }
    if ( !cg_tracerScaleMinDist
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                    6867,
                    0,
                    "%s",
                    "cg_tracerScaleMinDist") )
    {
        __debugbreak();
    }
    if ( !cg_tracerScaleDistRange
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                    6868,
                    0,
                    "%s",
                    "cg_tracerScaleDistRange") )
    {
        __debugbreak();
    }
    tracerScale = cg_tracerScale->current.value;
    tracerScaleMinDist = cg_tracerScaleMinDist->current.value;
    tracerScaleDistRange = cg_tracerScaleDistRange->current.integer;
    if ( tracerScale != 1.0 )
    {
        startDist = Vec3Distance(start, viewOrg);
        finishDist = Vec3Distance(finish, viewOrg);
        if ( tracerScaleMinDist != 0.0 )
        {
            startDist = startDist - tracerScaleMinDist;
            finishDist = finishDist - tracerScaleMinDist;
        }
        if ( startDist > 0.0 )
            *startWidth = CalcTracerFinalScale(*(float *)&tracerScaleDistRange, startDist, tracerScale) * *startWidth;
        if ( finishDist > 0.0 )
            *finishWidth = CalcTracerFinalScale(*(float *)&tracerScaleDistRange, finishDist, tracerScale) * *finishWidth;
    }
}

double __cdecl CalcTracerFinalScale(float tracerScaleDistRange, float dist, float tracerScale)
{
    float v4; // [esp+0h] [ebp-Ch]

    if ( tracerScaleDistRange <= 0.0 )
    {
        return tracerScale;
    }
    else
    {
        v4 = (float)(dist / tracerScaleDistRange) * tracerScale;
        if ( tracerScale <= 1.0
            && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\universal\\com_math.h", 730, 0, "%s", "min < max") )
        {
            __debugbreak();
        }
        if ( v4 >= 1.0 )
        {
            if ( v4 <= tracerScale )
                return (float)((float)(dist / tracerScaleDistRange) * tracerScale);
            else
                return tracerScale;
        }
        else
        {
            return 1.0f;
        }
    }
}

cg_s *__cdecl CG_GetLocalClientGlobalsForEnt(int localClientNum, int entityNum)
{
    snapshot_s *nextSnap; // [esp+4h] [ebp-8h]
    cg_s *cgameGlob; // [esp+8h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    nextSnap = cgameGlob->nextSnap;
    if ( (nextSnap->ps.otherFlags & 6) != 0 && entityNum == nextSnap->ps.clientNum )
        return cgameGlob;
    else
        return 0;
}

void __cdecl CG_GetViewDirection(int localClientNum, int entityNum, float *forward, float *right, float *up)
{
    const cg_s *cgameGlob; // [esp+8h] [ebp-10h]
    const clientInfo_t *ci; // [esp+10h] [ebp-8h]
    unsigned int clientNum; // [esp+14h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobalsForEnt(localClientNum, entityNum);
    if ( cgameGlob )
    {
        BG_GetPlayerViewDirection(&cgameGlob->predictedPlayerState, forward, right, up);
    }
    else
    {
        clientNum = CG_GetEntity(localClientNum, entityNum)->nextState.clientNum;
        if ( clientNum >= 0x20
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                        6996,
                        0,
                        "clientNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                        clientNum,
                        32) )
        {
            __debugbreak();
        }
        ci = &CG_GetLocalClientGlobals(localClientNum)->bgs.clientinfo[clientNum];
        if ( !ci->infoValid
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 6999, 0, "%s", "ci->infoValid") )
        {
            __debugbreak();
        }
        AngleVectors(ci->playerAngles, forward, right, up);
    }
}

void __cdecl CG_CalcEyePoint(int localClientNum, int entityNum, float *eyePos)
{
    float v3; // xmm0_4
    const cg_s *cgameGlob; // [esp+10h] [ebp-8h]
    centity_s *cent; // [esp+14h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobalsForEnt(localClientNum, entityNum);
    if ( !cgameGlob || cgameGlob->renderingThirdPerson == TP_FOR_MODEL )
    {
        cent = CG_GetEntity(localClientNum, entityNum);
        *eyePos = cent->pose.origin[0];
        eyePos[1] = cent->pose.origin[1];
        eyePos[2] = cent->pose.origin[2];
        if ( entityNum < com_maxclients->current.integer )
        {
            if ( (cent->nextState.lerp.eFlags & 8) != 0 )
            {
                eyePos[2] = eyePos[2] + 11.0;
            }
            else
            {
                if ( (cent->nextState.lerp.eFlags & 4) != 0 )
                    v3 = eyePos[2] + 40.0;
                else
                    v3 = eyePos[2] + 60.0;
                eyePos[2] = v3;
            }
        }
    }
    else
    {
        *eyePos = cgameGlob->refdef.vieworg[0];
        eyePos[1] = cgameGlob->refdef.vieworg[1];
        eyePos[2] = cgameGlob->refdef.vieworg[2];
    }
}

void __cdecl CG_RandomEffectAxis(float *forward, float *left, float *up)
{
    float degrees; // [esp+0h] [ebp-30h]
    float dot; // [esp+10h] [ebp-20h]
    float point[3]; // [esp+24h] [ebp-Ch] BYREF

    iassert(forward);
    iassert(left);
    iassert(up);

    point[0] = -forward[2];
    point[1] = forward[0]; // v5
    point[2] = -forward[1]; // v6

    //dot = -((float)((float)(forward[0] * forward[0]) + (float)(v5 * forward[1])) + (float)(v6 * forward[2]));

    dot = -Vec3Dot(forward, forward);

    point[0] += (dot * forward[0]);
    point[1] += (dot * forward[1]);
    point[2] += (dot * forward[2]);

    degrees = Com_Random() * 360.0;

    RotatePointAroundVector(left, forward, point, degrees);
    Vec3Normalize(left);
    Vec3Cross(forward, left, up);
}

void __cdecl CG_ImpactEffectForWeapon(
                unsigned int weaponIndex,
                unsigned int surfType,
                char impactFlags,
                const FxEffectDef **outFx,
                unsigned int *outSnd)
{
    int AliasId; // eax
    unsigned int v6; // [esp+0h] [ebp-34h]
    unsigned int v7; // [esp+8h] [ebp-2Ch]
    unsigned int v8; // [esp+Ch] [ebp-28h]
    unsigned int v9; // [esp+10h] [ebp-24h]
    unsigned int v10; // [esp+14h] [ebp-20h]
    unsigned int v11; // [esp+18h] [ebp-1Ch]
    int fleshType; // [esp+20h] [ebp-14h]
    int fxType; // [esp+28h] [ebp-Ch]
    int impactType; // [esp+2Ch] [ebp-8h]
    const WeaponDef *weaponDef; // [esp+30h] [ebp-4h]

    weaponDef = BG_GetWeaponDef(weaponIndex);
    if (!weaponDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 7163, 0, "%s", "weaponDef"))
    {
        __debugbreak();
    }
    if (surfType >= 0x1F
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
            7164,
            0,
            "surfType doesn't index SURF_TYPECOUNT\n\t%i not in [0, %i)",
            surfType,
            31))
    {
        __debugbreak();
    }
    fxType = -1;
    *outSnd = 0;
    impactType = weaponDef->impactType;
    if ((impactFlags & 8) != 0)
        impactType = 1;
    switch (impactType)
    {
    case 1:
        if ((impactFlags & 4) != 0)
        {
            fxType = 1;
        }
        else if ((impactFlags & 8) != 0)
        {
            fxType = 2;
        }
        else
        {
            fxType = 0;
        }
        if ((impactFlags & 4) != 0)
            v11 = cgMedia.bulletExitSmallSound[surfType];
        else
            v11 = cgMedia.bulletHitSmallSound[surfType];
        *outSnd = v11;
        break;
    case 2:
        fxType = ((impactFlags & 4) != 0) + 3;
        if ((impactFlags & 4) != 0)
            v10 = cgMedia.bulletExitLargeSound[surfType];
        else
            v10 = cgMedia.bulletHitLargeSound[surfType];
        *outSnd = v10;
        break;
    case 3:
        fxType = ((impactFlags & 4) != 0) + 7;
        if ((impactFlags & 4) != 0)
            v9 = cgMedia.bulletExitAPSound[surfType];
        else
            v9 = cgMedia.bulletHitAPSound[surfType];
        *outSnd = v9;
        break;
    case 4:
        fxType = ((impactFlags & 4) != 0) + 9;
        if ((impactFlags & 4) != 0)
            v8 = cgMedia.bulletExitXTremeSound[surfType];
        else
            v8 = cgMedia.bulletHitXTremeSound[surfType];
        *outSnd = v8;
        break;
    case 5:
        fxType = ((impactFlags & 4) != 0) + 5;
        if ((impactFlags & 4) != 0)
            v7 = cgMedia.shotgunExitSound[surfType];
        else
            v7 = cgMedia.shotgunHitSound[surfType];
        *outSnd = v7;
        break;
    case 6:
        fxType = 11;
        if (weaponDef->bounceSound)
            AliasId = SND_FindAliasId((char *)weaponDef->bounceSound[surfType]);
        else
            AliasId = SND_FindAliasId((char*)0);
        *outSnd = AliasId;
        break;
    case 7:
        fxType = 12;
        *outSnd = cgMedia.grenadeExplodeSound[surfType];
        break;
    case 8:
        fxType = 12;
        *outSnd = cgMedia.rifleGrenadeSound[surfType];
        break;
    case 9:
        fxType = 13;
        *outSnd = cgMedia.rocketExplodeSound[surfType];
        break;
    case 10:
        fxType = 14;
        *outSnd = cgMedia.rocketExplodeSound[surfType];
        break;
    case 11:
        fxType = 15;
        *outSnd = SND_FindAliasId((char *)weaponDef->projDudSound);
        break;
    case 12:
        fxType = 16;
        if (weaponDef->mortarShellSound)
            *outSnd = SND_FindAliasId((char *)weaponDef->mortarShellSound);
        else
            *outSnd = cgMedia.mortarShellExplodeSound[surfType];
        break;
    case 13:
        fxType = 17;
        if (weaponDef->tankShellSound)
            *outSnd = SND_FindAliasId((char *)weaponDef->tankShellSound);
        else
            *outSnd = cgMedia.tankShellExplodeSound[surfType];
        break;
    case 14:
        if ((impactFlags & 4) != 0)
        {
            fxType = 19;
        }
        else if ((impactFlags & 8) != 0)
        {
            fxType = 2;
        }
        else
        {
            fxType = 19;
        }
        if ((impactFlags & 4) != 0)
            v6 = cgMedia.boltExitSound[surfType];
        else
            v6 = cgMedia.boltHitSound[surfType];
        *outSnd = v6;
        break;
    case 15:
        fxType = 20;
        *outSnd = cgMedia.bladeHitSound[surfType];
        break;
    default:
        break;
    }
    if (fxType >= 0)
    {
        if ((impactFlags & 0x10) != 0)
        {
            if (cgMedia.weaponImpactsTankArmorSound[impactType])
                *outSnd = cgMedia.weaponImpactsTankArmorSound[impactType];
        }
        else if ((impactFlags & 0x20) != 0 && cgMedia.weaponImpactsTankTreadSound[impactType])
        {
            *outSnd = cgMedia.weaponImpactsTankTreadSound[impactType];
        }
        if (surfType == 7)
        {
            if ((impactFlags & 2) != 0)
                fleshType = (impactFlags & 1) != 0 ? 3 : 1;
            else
                fleshType = (impactFlags & 1) != 0 ? 2 : 0;
            *outFx = cgMedia.fx->table[fxType].flesh[fleshType];
        }
        else
        {
            *outFx = cgMedia.fx->table[fxType].nonflesh[surfType];
        }
    }
    else
    {
        *outFx = 0;
    }
}

void __cdecl CG_BulletHitEvent(
                int localClientNum,
                int sourceEntityNum,
                int targetEntityNum,
                int weaponIndex,
                const float *startPos,
                const float *position,
                const float *normal,
                const float *seeThruDecalNormal,
                unsigned int surfType,
                int event,
                unsigned __int8 eventParam,
                int damage,
                __int16 hitContents,
                unsigned __int8 boneIndex)
{
    float v[2]; // [esp+30h] [ebp-1Ch] BYREF
    float v15; // [esp+38h] [ebp-14h]
    const cg_s *cgameGlob; // [esp+3Ch] [ebp-10h]
    float exitDir[3]; // [esp+40h] [ebp-Ch] BYREF

    if ( sv_clientSideBullets->current.enabled
        && IsEntityNotDoingClientSideBullets(localClientNum, targetEntityNum)
        && (eventParam & 4) == 0 )
    {
        CG_BulletHitEvent_GenExitDir(localClientNum, sourceEntityNum, position, exitDir);
        CG_BulletHitEvent_Internal(
            localClientNum,
            sourceEntityNum,
            targetEntityNum,
            weaponIndex,
            startPos,
            position,
            exitDir,
            seeThruDecalNormal,
            surfType,
            event,
            eventParam | 4,
            damage,
            hitContents,
            boneIndex);
    }
    if ( surfType == 20
        && (cgameGlob = CG_GetLocalClientGlobals(localClientNum), cgameGlob->predictedPlayerState.waterlevel >= 5) )
    {
        v[0] = *position - *startPos;
        v[1] = position[1] - startPos[1];
        v15 = position[2] - startPos[2];
        Vec3Normalize(v);
        if ( v15 < 0.0 )
            CG_BulletHitEvent_Internal(
                localClientNum,
                sourceEntityNum,
                targetEntityNum,
                weaponIndex,
                startPos,
                position,
                v,
                seeThruDecalNormal,
                0x14u,
                event,
                eventParam | 4,
                damage,
                hitContents,
                boneIndex);
    }
    else
    {
        CG_BulletHitEvent_Internal(
            localClientNum,
            sourceEntityNum,
            targetEntityNum,
            weaponIndex,
            startPos,
            position,
            normal,
            seeThruDecalNormal,
            surfType,
            event,
            eventParam,
            damage,
            hitContents,
            boneIndex);
    }
}

void __cdecl CG_BulletHitEvent_Internal(
                int localClientNum,
                int sourceEntityNum,
                int targetEntityNum,
                int weaponIndex,
                const float *startPos,
                const float *position,
                const float *normal,
                const float *seeThruDecalNormal,
                unsigned int surfType,
                int event,
                unsigned __int8 eventParam,
                int damage,
                __int16 hitContents,
                unsigned __int8 boneIndex)
{
    snapshot_s *nextSnap; // [esp+14h] [ebp-6Ch]
    centity_s *cent; // [esp+34h] [ebp-4Ch]
    const WeaponDef *weapDef; // [esp+3Ch] [ebp-44h]
    const centity_s *attacker; // [esp+40h] [ebp-40h]
    unsigned int hitSound; // [esp+48h] [ebp-38h] BYREF
    cg_s *cgameGlob; // [esp+4Ch] [ebp-34h]
    int time; // [esp+50h] [ebp-30h]
    const FxEffectDef *fx; // [esp+54h] [ebp-2Ch] BYREF
    centity_s *attackee; // [esp+58h] [ebp-28h]
    float axis[3][3]; // [esp+5Ch] [ebp-24h] BYREF

    if ( sourceEntityNum < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                    7327,
                    0,
                    "%s",
                    "sourceEntityNum >= 0") )
    {
        __debugbreak();
    }
    if ( sourceEntityNum == 1023
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                    7328,
                    0,
                    "%s",
                    "sourceEntityNum != ENTITYNUM_NONE") )
    {
        __debugbreak();
    }
    if ( !position
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 7329, 0, "%s", "position") )
    {
        __debugbreak();
    }
    if ( !normal && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 7330, 0, "%s", "normal") )
        __debugbreak();
    if ( surfType >= 0x1F
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                    7331,
                    0,
                    "surfType doesn't index SURF_TYPECOUNT\n\t%i not in [0, %i)",
                    surfType,
                    31) )
    {
        __debugbreak();
    }
    if ( damage < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                    7332,
                    0,
                    "%s\n\t(damage) = %i",
                    "(damage >= 0)",
                    damage) )
    {
        __debugbreak();
    }
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    attacker = CG_GetEntity(localClientNum, sourceEntityNum);
    attackee = CG_GetEntity(localClientNum, targetEntityNum);
    fx = 0;
    hitSound = 0;
    if ( (hitContents & 0x800) == 0 )
    {
        weapDef = BG_GetWeaponDef(weaponIndex);
        if ( ((*((unsigned int *)attackee + 201) >> 1) & 1) != 0 && attackee->nextState.eType == 17
            || attackee->nextState.eType == 1 )
        {
            Scr_AddConstString(cscr_const.face_pain, SCRIPTINSTANCE_CLIENT);
            CScr_Notify(localClientNum, attackee, cscr_const.face, 1u);
        }
        if ( !attacker || !weapDef || attacker->pose.eType != 14 || weapDef->weapType != WEAPTYPE_PROJECTILE )
            CG_ImpactEffectForWeapon(weaponIndex, surfType, eventParam, &fx, &hitSound);
    }
    if ( fx && surfType == 7 )
    {
        cent = CG_GetEntity(localClientNum, targetEntityNum);
        if ( ((*((unsigned int *)cent + 201) >> 6) & 1) != 0 )
        {
            fx = 0;
        }
        else if ( ((*((unsigned int *)cent + 201) >> 7) & 1) != 0 )
        {
            fx = cgMedia.fxDtpArmSlide2;
        }
    }
    if ( fx && (!cg_blood->current.enabled || !CG_IsMature()) && surfType == 7 )
        fx = cgMedia.fxDogNoBlood;
    if ( hitSound )
        CG_PlaySound(localClientNum, 1022, (float*)position, 0, 0, 1.0, hitSound);
    time = CG_GetLocalClientGlobals(localClientNum)->time;
    if ( fx )
    {
        if ( (fx->flags & 2) == 0 )
            seeThruDecalNormal = normal;
        if ( *seeThruDecalNormal != 0.0 || seeThruDecalNormal[1] != 0.0 || seeThruDecalNormal[2] != 0.0 )
        {
            *(_QWORD *)&axis[0][0] = *(_QWORD *)seeThruDecalNormal;
            axis[0][2] = seeThruDecalNormal[2];
            CG_RandomEffectAxis(axis[0], axis[1], axis[2]);
            if ( cg_marks_ents_player_only->current.enabled )
            {
                nextSnap = cgameGlob->nextSnap;
                if ( (nextSnap->ps.otherFlags & 6) == 0 || sourceEntityNum != nextSnap->ps.clientNum )
                    targetEntityNum = 1023;
            }
            if ( ((*((unsigned int *)attackee + 201) >> 1) & 1) != 0 && boneIndex != 254 && surfType == 7 )
            {
                FX_PlayBoltedEffect(localClientNum, fx, cgameGlob->time, targetEntityNum, boneIndex, position, axis);
            }
            else if ( ((*((unsigned int *)attackee + 201) >> 1) & 1) != 0
                         && (attackee->nextState.eType == 1 || attackee->nextState.eType == 2)
                         && boneIndex != 254
                         && surfType == 13 )
            {
                if ( BG_GetWeaponDef(weaponIndex)->impactType == IMPACT_TYPE_BULLET_SMALL )
                    fx = cgMedia.fxPlayerSliding;
                else
                    fx = cgMedia.fxPuff;
                (axis[0][0]) = -(axis[0][0]);
                (axis[0][1]) = -(axis[0][1]);
                (axis[0][2]) = -(axis[0][2]);
                FX_PlayOrientedEffectWithMarkEntity(localClientNum, fx, cgameGlob->time, position, axis, targetEntityNum);
            }
            else
            {
                FX_PlayOrientedEffectWithMarkEntity(localClientNum, fx, cgameGlob->time, position, axis, targetEntityNum);
            }
        }
    }
    if ( !attacker->vehicle )
        BulletTrajectoryEffects(
            localClientNum,
            sourceEntityNum,
            weaponIndex,
            startPos,
            position,
            surfType,
            scr_const.tag_flash,
            eventParam,
            damage,
            0);
}

void __cdecl BulletTrajectoryEffects(
                int localClientNum,
                int sourceEntityNum,
                unsigned int weaponIndex,
                const float *startPos,
                const float *position,
                int surfType,
                unsigned int flashTag,
                unsigned __int8 impactFlags,
                int damage,
                float *tracerStart)
{
    float muzzle[3]; // [esp+4h] [ebp-Ch] BYREF

    if ( sourceEntityNum < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                    7106,
                    0,
                    "%s",
                    "sourceEntityNum >= 0") )
    {
        __debugbreak();
    }
    if ( damage < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                    7107,
                    0,
                    "%s\n\t(damage) = %i",
                    "(damage >= 0)",
                    damage) )
    {
        __debugbreak();
    }
    BG_GetWeaponDef(weaponIndex);
    if ( tracerStart )
    {
        muzzle[0] = *tracerStart;
        muzzle[1] = tracerStart[1];
        muzzle[2] = tracerStart[2];
    }
    else
    {
        CalcMuzzlePoint(localClientNum, sourceEntityNum, muzzle, flashTag);
    }
    if ( (impactFlags & 4) == 0 && !sv_clientSideBullets->current.enabled )
        DynEntCl_DynEntImpactEvent(localClientNum, sourceEntityNum, startPos, position, damage, 0);
}

void __cdecl CalcMuzzlePoint(int localClientNum, int entityNum, float *muzzle, unsigned int flashTag)
{
    char *v4; // eax
    float v5; // xmm0_4
    snapshot_s *nextSnap; // [esp+14h] [ebp-10h]
    DObj *obj; // [esp+18h] [ebp-Ch]
    const cg_s *cgameGlob; // [esp+1Ch] [ebp-8h]
    centity_s *cent; // [esp+20h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    nextSnap = cgameGlob->nextSnap;
    if ( (nextSnap->ps.otherFlags & 6) != 0 && entityNum == nextSnap->ps.clientNum )
    {
        *muzzle = cgameGlob->refdef.vieworg[0];
        muzzle[1] = cgameGlob->refdef.vieworg[1];
        muzzle[2] = cgameGlob->refdef.vieworg[2];
    }
    else
    {
        cent = CG_GetEntity(localClientNum, entityNum);
        obj = Com_GetClientDObj(cent->nextState.number, 0);
        if ( !obj || !CG_DObjGetWorldTagPos(&cent->pose, obj, flashTag, muzzle) )
        {
            *muzzle = cent->pose.origin[0];
            muzzle[1] = cent->pose.origin[1];
            muzzle[2] = cent->pose.origin[2];
            if ( entityNum < com_maxclients->current.integer )
            {
                v4 = SL_ConvertToString(flashTag, SCRIPTINSTANCE_SERVER);
                Com_DPrintf(17, "No %s in CalcMuzzlePoint on entity %d.\n", v4, entityNum);
                if ( (cent->nextState.lerp.eFlags & 8) != 0 )
                {
                    muzzle[2] = muzzle[2] + 11.0;
                }
                else
                {
                    if ( (cent->nextState.lerp.eFlags & 4) != 0 )
                        v5 = muzzle[2] + 40.0;
                    else
                        v5 = muzzle[2] + 60.0;
                    muzzle[2] = v5;
                }
            }
        }
    }
}

void __cdecl CG_BulletHitEvent_GenExitDir(
                int localClientNum,
                int sourceEntityNum,
                const float *position,
                float *outExitDir)
{
    float muzzle[3]; // [esp+Ch] [ebp-Ch] BYREF

    CalcMuzzlePoint(localClientNum, sourceEntityNum, muzzle, scr_const.tag_flash);
    *outExitDir = *position - muzzle[0];
    outExitDir[1] = position[1] - muzzle[1];
    outExitDir[2] = position[2] - muzzle[2];
    Vec3Normalize(outExitDir);
}

void __cdecl CG_BulletHitClientEvent(
                int localClientNum,
                int sourceEntityNum,
                unsigned int weaponNum,
                int hitEntityNum,
                const float *startPos,
                float *position,
                unsigned int surfType,
                int event,
                int damage)
{
    const char *v9; // eax
    unsigned int hitSound; // [esp+Ch] [ebp-4h]

    if ( sourceEntityNum < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                    7546,
                    0,
                    "%s",
                    "sourceEntityNum >= 0") )
    {
        __debugbreak();
    }
    if ( sourceEntityNum == 1023
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                    7547,
                    0,
                    "%s",
                    "sourceEntityNum != ENTITYNUM_NONE") )
    {
        __debugbreak();
    }
    if ( !position
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 7548, 0, "%s", "position") )
    {
        __debugbreak();
    }
    if ( surfType > 0x1E
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                    7549,
                    0,
                    "%s",
                    "surfType >= 0 && surfType < SURF_TYPECOUNT") )
    {
        __debugbreak();
    }
    if ( damage < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                    7550,
                    0,
                    "%s\n\t(damage) = %i",
                    "(damage >= 0)",
                    damage) )
    {
        __debugbreak();
    }
    if ( surfType == 7 )
        surfType = 29;
    if ( event == 52 )
    {
        hitSound = cgMedia.bulletHitSmallSound[surfType];
LABEL_26:
        CG_PlaySound(localClientNum, 1022, position, 0, 0, 1.0, hitSound);
        BulletTrajectoryEffects(
            localClientNum,
            sourceEntityNum,
            weaponNum,
            startPos,
            position,
            surfType,
            scr_const.tag_flash,
            0,
            damage,
            0);
        return;
    }
    if ( event == 53 )
    {
        hitSound = cgMedia.bulletHitLargeSound[surfType];
        goto LABEL_26;
    }
    v9 = va("CG_BulletHitClientEvent: Unknown event [%d]\n", event);
    if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 7575, 0, v9) )
        __debugbreak();
}

void __cdecl CG_DogBloodEvent(int localClientNum, unsigned int entnum, float *position, int boneIndex)
{
    cg_s *LocalClientGlobals; // eax
    cg_s *v5; // eax

    if ( cg_blood->current.enabled && CG_IsMature() )
    {
        LocalClientGlobals = CG_GetLocalClientGlobals(localClientNum);
        FX_PlayBoltedEffect(localClientNum, cgMedia.fxBodyArmorLarge, LocalClientGlobals->time, entnum, boneIndex);
    }
    else
    {
        v5 = CG_GetLocalClientGlobals(localClientNum);
        FX_PlayBoltedEffect(localClientNum, cgMedia.fxDtpArmSlide1, v5->time, entnum, boneIndex);
    }
}

void __cdecl CG_MeleeBloodEvent(int localClientNum, const centity_s *cent, int eventParm)
{
    bool isPlayer; // [esp+0h] [ebp-1Ch]
    snapshot_s *nextSnap; // [esp+4h] [ebp-18h]
    int dobjHandle; // [esp+Ch] [ebp-10h]

    if ( !cent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 7606, 0, "%s", "cent") )
        __debugbreak();
    nextSnap = CG_GetLocalClientGlobals(localClientNum)->nextSnap;
    isPlayer = (nextSnap->ps.otherFlags & 6) != 0 && cent->nextState.number == nextSnap->ps.clientNum;
    if ( !isPlayer
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 7611, 0, "%s", "isPlayer") )
    {
        __debugbreak();
    }
    dobjHandle = CG_WeaponDObjHandle(localClientNum);
    if ( BG_IsUseAsMeleeWeapon(cent->nextState.weapon) )
        CG_GetLocalClientWeaponInfo(localClientNum, cent->nextState.lerp.u.player.primaryWeapon);
    else
        CG_GetLocalClientWeaponInfo(localClientNum, cent->nextState.lerp.u.player.meleeWeapon);
    if ( eventParm == 1 )
    {
        if ( cg_blood->current.enabled && CG_IsMature() )
            CG_PlayBoltedEffect(localClientNum, cgMedia.fxNonFatalHero, dobjHandle, scr_const.tag_knife_fx);
        else
            CG_PlayBoltedEffect(localClientNum, cgMedia.fxBodyArmorSmall, dobjHandle, scr_const.tag_knife_fx);
    }
    else if ( eventParm == 2 )
    {
        if ( cg_blood->current.enabled && CG_IsMature() )
            CG_PlayBoltedEffect(localClientNum, cgMedia.fxNonFatalHero, dobjHandle, scr_const.tag_bayonet);
        else
            CG_PlayBoltedEffect(localClientNum, cgMedia.fxBodyArmorSmall, dobjHandle, scr_const.tag_bayonet);
    }
}

void __cdecl CG_SetBaseWeaponForStats(const WeaponVariantDef *weapVariantDef)
{
    int itemIndex; // [esp+0h] [ebp-4h]

    if ( weapVariantDef->weapDef )
    {
        if ( weapVariantDef->weapDef->parentWeaponName && *weapVariantDef->weapDef->parentWeaponName )
        {
            itemIndex = BG_UnlockablesGetItemIndexFromRef(weapVariantDef->weapDef->parentWeaponName);
            if ( itemIndex == -1 )
                Com_Error(
                    ERR_DROP,
                    "Parent weapon '%s' not found for '%s'",
                    weapVariantDef->weapDef->parentWeaponName,
                    weapVariantDef->szInternalName);
            else
                weapVariantDef->weapDef->itemIndex = itemIndex;
        }
        else if ( !weapVariantDef->weapDef->itemIndex )
        {
            Com_Printf(16, "weapon %s not found in weapon table\n", weapVariantDef->szInternalName);
        }
    }
}

void __cdecl CG_SetupWeaponConfigString(int configStringIndex)
{
    int WeaponIndexForName; // eax
    const WeaponVariantDef *weapVarDef; // eax
    unsigned int currentIndex; // [esp+0h] [ebp-81Ch]
    int variantCount; // [esp+4h] [ebp-818h]
    const WeaponVariantDef *weapVariantDef; // [esp+8h] [ebp-814h]
    char weaponbuf[2048]; // [esp+Ch] [ebp-810h] BYREF
    unsigned int weaponIndex; // [esp+810h] [ebp-Ch]
    const char *token; // [esp+814h] [ebp-8h]
    const char *weaponConfigString; // [esp+818h] [ebp-4h] BYREF

    // LWSS ADD - sv.configstrings is not set at this point (FOR CLIENTS CONNECTING TO A DEDICATED)
    // So I'm calling CL_GetConfigString() instead for clients - this IS set in CL_ParseGameState()
    if (IsDedicatedServer())
    {
        SV_GetConfigstring(configStringIndex + 2836, weaponbuf, 2048);
        weaponConfigString = weaponbuf;
    }
    else
    {
        weaponConfigString = CL_GetConfigString(configStringIndex + 2836);
    }
    // LWSS END

    if ( weaponbuf && *weaponConfigString )
    {
        token = (const char *)Com_Parse(&weaponConfigString);
        if ( !token || !*token )
            Com_Error(ERR_DROP, "CG_SetupWeaponConfigString: Invalid weapon config string index");
        weaponIndex = atoi(token);
        token = (const char *)Com_Parse(&weaponConfigString);
        if ( !token || !*token )
            Com_Error(ERR_DROP, "CG_SetupWeaponConfigString: Invalid weapon config string name");
        WeaponIndexForName = CG_GetWeaponIndexForName((char *)token);
        if ( WeaponIndexForName == weaponIndex )
        {
            weapVariantDef = BG_GetWeaponVariantDef(weaponIndex);
            CG_SetBaseWeaponForStats(weapVariantDef);
            variantCount = 0;
            token = (const char *)Com_Parse(&weaponConfigString);
            if ( token )
            {
                if ( *token )
                {
                    variantCount = atoi(token);
                    if ( variantCount < 0 )
                        Com_Error(ERR_DROP, "CG_SetupWeaponConfigString: Invalid weapon config variant count");
                }
            }
            if ( weapVariantDef->iVariantCount == variantCount )
            {
                for ( currentIndex = weaponIndex; currentIndex < variantCount + weaponIndex; ++currentIndex )
                {
                    weapVarDef = BG_GetWeaponVariantDef(currentIndex);
                    CG_SetBaseWeaponForStats(weapVarDef);
                }
                return;
            }
            Com_Error(ERR_DROP, "Weapon variant count mismatch for '%s'", weapVariantDef->szInternalName);
        }
        Com_Error(ERR_DROP, "Weapon index mismatch for '%s'", token);
    }
}

void __cdecl CG_SetupWeaponDef()
{
    unsigned int configStringIndex; // [esp+0h] [ebp-4h]

    CG_GetWeaponIndexForName((char*)"defaultweapon_mp");
    if ( !G_ExitAfterToolComplete() )
    {
        BG_LoadWeaponTable("_mp", 0);
        for ( configStringIndex = 0; configStringIndex < 0x100; ++configStringIndex )
            CG_SetupWeaponConfigString(configStringIndex);
    }
}

unsigned int __cdecl ValidLatestPrimaryWeapIdx(const cg_s *cgameGlob, unsigned int weaponIndex)
{
    const WeaponDef *weapDef; // [esp+0h] [ebp-4h]

    if ( !weaponIndex )
        return 0;
    weapDef = BG_GetWeaponDef(weaponIndex);
    if ( weapDef->bIsCarriedKillstreakWeapon )
        return 0;
    if ( weapDef->inventoryType == WEAPINVENTORY_PRIMARY )
        return weaponIndex;
    if ( weapDef->inventoryType == WEAPINVENTORY_ALTMODE )
        weaponIndex = CG_GetAltWeapon(cgameGlob, weaponIndex);
    if ( BG_GetWeaponDef(weaponIndex)->inventoryType )
        return 0;
    else
        return weaponIndex;
}

char __cdecl CG_SetLastWeaponForAlt(
                cg_s *cgameGlob,
                const playerState_s *ps,
                unsigned int altWeapon,
                unsigned int weapon)
{
    const char *v5; // eax
    const char *v6; // eax
    int heldWeaponIndex; // [esp+4h] [ebp-4h]

    if ( !altWeapon )
        return 0;
    heldWeaponIndex = BG_GetHeldWeaponSlot(ps, altWeapon);
    if ( heldWeaponIndex == -1 )
    {
        v5 = BG_WeaponName(altWeapon);
        v6 = va("Not holding alt weapon %s, but trying to set the last weapon to switch back to.\n", v5);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                        7798,
                        0,
                        "%s\n\t%s",
                        "heldWeaponIndex != INVALID_HELD_WEAPON_SLOT",
                        v6) )
            __debugbreak();
    }
    if ( heldWeaponIndex == -1 )
        return 0;
    cgameGlob->lastWeaponForAlt[heldWeaponIndex] = weapon;
    AssignToSmallerType<unsigned short>(&cgameGlob->lastWeaponForAlt[heldWeaponIndex], weapon);
    return 1;
}

void __cdecl CG_SelectWeaponIndex(int localClientNum, const playerState_s *ps, unsigned int weaponIndex)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-18h]
    bool altSwitch; // [esp+4h] [ebp-14h]
    const WeaponDef *newWeapDef; // [esp+8h] [ebp-10h]
    const WeaponDef *oldWeapDef; // [esp+Ch] [ebp-Ch]
    unsigned int validLatest; // [esp+10h] [ebp-8h]
    const WeaponDef *lastValidWeapDef; // [esp+14h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    cgameGlob->weaponSelectTime = cgameGlob->time;
    newWeapDef = BG_GetWeaponDef(weaponIndex);
    oldWeapDef = BG_GetWeaponDef(ps->weapon);
    lastValidWeapDef = BG_GetWeaponDef(cgameGlob->weaponLatestPrimaryIdx);
    if ( cgameGlob->weaponSelect == weaponIndex && newWeapDef->bIsCarriedKillstreakWeapon )
        weaponIndex = cgameGlob->weaponLatestPrimaryIdx;
    if ( cgameGlob->weaponSelect != weaponIndex )
    {
        if ( newWeapDef->blocksProne
            && newWeapDef->bIsCarriedKillstreakWeapon
            && (cgameGlob->predictedPlayerState.pm_flags & 1) != 0 )
        {
            if ( cgameGlob->invalidCmdHintType != INVALID_CMD_CANT_PLANT_EQUIPMENT )
            {
                cgameGlob->invalidCmdHintType = INVALID_CMD_CANT_PLANT_EQUIPMENT;
                cgameGlob->invalidCmdHintTime = cgameGlob->time;
            }
        }
        else if ( (ps->weapFlags & 0x100000) == 0
                     && CG_UpdateLastWeaponForAlt(cgameGlob, ps, cgameGlob->weaponSelect, weaponIndex)
                     && (newWeapDef->offhandClass == OFFHAND_CLASS_NONE || newWeapDef->offhandClass == OFFHAND_CLASS_GEAR) )
        {
            altSwitch = BG_IsAltSwitch(&cgameGlob->predictedPlayerState, cgameGlob->weaponSelect, weaponIndex);
            if ( lastValidWeapDef->bIsCarriedKillstreakWeapon && newWeapDef->offhandClass == OFFHAND_CLASS_GEAR )
            {
                cgameGlob->weaponLatestPrimaryIdx = 0;
            }
            else if ( lastValidWeapDef->bIsCarriedKillstreakWeapon )
            {
                if ( weaponIndex != cgameGlob->weaponLatestPrimaryIdx )
                    weaponIndex = cgameGlob->weaponLatestPrimaryIdx;
                cgameGlob->weaponLatestPrimaryIdx = 0;
            }
            validLatest = ValidLatestPrimaryWeapIdx(cgameGlob, weaponIndex);
            if ( validLatest )
                cgameGlob->weaponLatestPrimaryIdx = validLatest;
            if ( (newWeapDef->offhandSlot == OFFHAND_SLOT_SPECIFIC_USE || newWeapDef->bCanUseInVehicle)
                && oldWeapDef->bIsCarriedKillstreakWeapon )
            {
                cgameGlob->weaponLatestPrimaryIdx = ps->weapon;
            }
            cgameGlob->weaponSelect = weaponIndex;
            CG_MenuShowNotify(localClientNum, 1);
            if ( !altSwitch )
                CL_SetADS(localClientNum, 0);
        }
    }
}

char __cdecl CG_UpdateLastWeaponForAlt(
                cg_s *cgameGlob,
                const playerState_s *ps,
                unsigned int oldweapon,
                unsigned int weapon)
{
    const char *v5; // eax
    const char *v6; // [esp-4h] [ebp-18h]
    const WeaponVariantDef *weapVariantDef; // [esp+0h] [ebp-14h]
    const WeaponVariantDef *oldWeapVariantDef; // [esp+8h] [ebp-Ch]
    const WeaponDef *oldWeapDef; // [esp+Ch] [ebp-8h]

    if ( weapon >= 0x800
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                    7811,
                    0,
                    "weapon doesn't index MAX_WEAPONS\n\t%i not in [0, %i)",
                    weapon,
                    2048) )
    {
        __debugbreak();
    }
    if ( oldweapon >= 0x800
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                    7812,
                    0,
                    "oldweapon doesn't index MAX_WEAPONS\n\t%i not in [0, %i)",
                    oldweapon,
                    2048) )
    {
        __debugbreak();
    }
    if ( !weapon )
        return 1;
    if ( BG_GetWeaponDef(weapon)->inventoryType != WEAPINVENTORY_ALTMODE )
        return 1;
    oldWeapVariantDef = BG_GetWeaponVariantDef(oldweapon);
    oldWeapDef = BG_GetWeaponDef(oldweapon);
    if ( oldWeapVariantDef->altWeaponIndex == weapon && oldWeapDef->inventoryType == WEAPINVENTORY_PRIMARY )
        return CG_SetLastWeaponForAlt(cgameGlob, ps, weapon, oldweapon);
    if ( CG_GetLastWeaponForAlt(cgameGlob, ps, weapon) )
        return 1;
    weapVariantDef = BG_GetWeaponVariantDef(weapon);
    if ( weapVariantDef->altWeaponIndex )
        return CG_SetLastWeaponForAlt(cgameGlob, ps, weapon, weapVariantDef->altWeaponIndex);
    v6 = BG_WeaponName(weapon);
    v5 = BG_WeaponName(oldweapon);
    Com_PrintError(
        1,
        "You can't switch directly to an alternate fire mode without first using a base weapon with that alt-fire mode.(old '%s' new '%s')\n",
        v5,
        v6);
    return 0;
}

void __cdecl CG_SelectWeaponIndex(int localClientNum, unsigned int weaponIndex)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    CG_SelectWeaponIndex(localClientNum, &cgameGlob->predictedPlayerState, weaponIndex);
}

void __cdecl CG_DisplayViewmodelAnim(int localClientNum)
{
    const cg_s *cgameGlob; // [esp+0h] [ebp-Ch]
    ViewModelInfo *viewModelInfo; // [esp+4h] [ebp-8h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( (int)BG_GetViewmodelWeaponIndex(&cgameGlob->predictedPlayerState) > 0 )
    {
        viewModelInfo = CG_GetLocalClientViewModelInfo(localClientNum);
        DObjDisplayAnim(viewModelInfo->viewModelDObj, "");
    }
}

bool __cdecl CG_ScopeIsOverlayed(int localClientNum)
{
    cg_s *cgameGlob; // [esp+4h] [ebp-8h]
    float zoom; // [esp+8h] [ebp-4h] BYREF

    if ( CL_GetLocalClientConnectionState(localClientNum) < 10 )
        return 0;
    if ( CG_PlayerUsingScopedTurret(localClientNum) )
        return 1;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    return CG_GetWeapReticleZoom(cgameGlob, &zoom);
}

unsigned int __cdecl CG_PlayerTurretWeaponIdx(int localClientNum)
{
    cg_s *cgameGlob; // [esp+8h] [ebp-Ch]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( (cgameGlob->predictedPlayerState.eFlags & 0x300) == 0 )
        return 0;
    if ( cgameGlob->predictedPlayerState.viewlocked == PLAYERVIEWLOCK_NONE )
        return 0;
    if ( cgameGlob->predictedPlayerState.viewlocked_entNum == 1023
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                    8036,
                    0,
                    "%s",
                    "ps->viewlocked_entNum != ENTITYNUM_NONE") )
    {
        __debugbreak();
    }
    return CG_GetEntity(localClientNum, cgameGlob->predictedPlayerState.viewlocked_entNum)->nextState.weapon;
}

unsigned int __cdecl CG_GetPlayerMeleeWeapon(const playerState_s *ps)
{
    if ( BG_IsUseAsMeleeWeapon(ps->weapon) )
        return ps->weapon;
    else
        return ps->meleeWeapon;
}

int __cdecl CG_GetPlayerWeapon(const playerState_s *ps, int localClientNum)
{
    unsigned int weapon; // [esp+0h] [ebp-4h]

    weapon = 0;
    if ( BG_UsingVehicleWeapon(ps) )
    {
        weapon = CG_GetPlayerVehicleWeapon(ps, localClientNum);
    }
    else if ( (ps->eFlags & 0x300) != 0 )
    {
        weapon = CG_PlayerTurretWeaponIdx(localClientNum);
    }
    if ( !weapon )
        return BG_GetViewmodelWeaponIndex(ps);
    return weapon;
}

int __cdecl CG_GetPlayerVehicleWeapon(const playerState_s *ps, int localClientNum)
{
    bool v3; // [esp+0h] [ebp-20h]
    const vehicle_info_t *info; // [esp+14h] [ebp-Ch]
    centity_s *cent; // [esp+18h] [ebp-8h]
    int seatIndex; // [esp+1Ch] [ebp-4h]

    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 8046, 0, "%s", "ps") )
        __debugbreak();
    if ( (ps->eFlags & 0x4000) == 0 )
        return 0;
    cent = CG_GetEntity(localClientNum, ps->viewlocked_entNum);
    info = CG_GetVehicleInfo(cent->nextState.vehicleState.vehicleInfoIndex);
    seatIndex = ps->vehiclePos;
    if ( seatIndex >= 1 && seatIndex <= 4 )
        return *(unsigned __int16 *)&info->gunnerWeapon[3][2 * seatIndex + 62];
    if ( seatIndex )
        return 0;
    v3 = !cent->nextState.weapon || CG_IsVehicleMayhemGameType();
    if ( v3 && info->driverControlledGunPos >= 0 )
        return info->gunnerWeaponIndex[info->driverControlledGunPos];
    else
        return cent->nextState.weapon;
}

unsigned int __cdecl CG_GetClientWeapon(unsigned int clientNum, int localClientNum)
{
    const vehicle_info_t *vehInfo; // [esp+8h] [ebp-18h]
    const centity_s *veh; // [esp+Ch] [ebp-14h]
    int gunnerIndex; // [esp+10h] [ebp-10h]
    const cg_s *cgameGlob; // [esp+14h] [ebp-Ch]
    int weapon; // [esp+18h] [ebp-8h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    weapon = 0;
    if ( clientNum >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp",
                    8109,
                    0,
                    "%s",
                    "clientNum >= 0 && clientNum < MAX_CLIENTS") )
    {
        __debugbreak();
    }
    if ( !cgameGlob
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 8110, 0, "%s", "cgameGlob") )
    {
        __debugbreak();
    }
    if ( !cgameGlob->bgs.clientinfo[clientNum].infoValid
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 8111, 0, "%s", "ci->infoValid") )
    {
        __debugbreak();
    }
    if ( cgameGlob->bgs.clientinfo[clientNum].attachedVehEntNum == 1023 )
        return cgameGlob->bgs.clientinfo[clientNum].iDObjWeapon;
    if ( cgameGlob->bgs.clientinfo[clientNum].attachedVehSeat >= 1
        && cgameGlob->bgs.clientinfo[clientNum].attachedVehSeat <= 4 )
    {
        gunnerIndex = cgameGlob->bgs.clientinfo[clientNum].attachedVehSeat - 1;
        veh = CG_GetEntity(localClientNum, cgameGlob->bgs.clientinfo[clientNum].attachedVehEntNum);
        if ( veh )
        {
            if ( ((*((unsigned int *)veh + 201) >> 1) & 1) != 0 )
            {
                vehInfo = CG_GetVehicleInfo(veh->nextState.vehicleState.vehicleInfoIndex);
                if ( !vehInfo
                    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 8123, 0, "%s", "vehInfo") )
                {
                    __debugbreak();
                }
                return vehInfo->gunnerWeaponIndex[gunnerIndex];
            }
        }
    }
    return weapon;
}

bool __cdecl CG_PlayerUsingScopedTurret(int localClientNum)
{
    unsigned int weapIdxTurret; // [esp+4h] [ebp-4h]

    weapIdxTurret = CG_PlayerTurretWeaponIdx(localClientNum);
    return weapIdxTurret && BG_GetWeaponVariantDef(weapIdxTurret)->overlayMaterial != 0;
}

void __cdecl CG_ApplyWeaponTurnRateCap(int localClientNum)
{
    float yawRate; // [esp+3Ch] [ebp-14h]
    float pitchRate; // [esp+40h] [ebp-10h]
    cg_s *cgameGlob; // [esp+44h] [ebp-Ch]
    int weapon; // [esp+48h] [ebp-8h]
    const WeaponDef *weaponDef; // [esp+4Ch] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->cameraMode != 1 )
    {
        weapon = CG_GetPlayerWeapon(&cgameGlob->predictedPlayerState, localClientNum);
        weaponDef = BG_GetWeaponDef(weapon);
        if ( weaponDef )
        {
            if ( weaponDef->maxTurnSpeed[1] != 0.0 && weaponDef->maxTurnSpeed[0] != 0.0 )
            {
                pitchRate = weaponDef->maxTurnSpeed[0];
                yawRate = weaponDef->maxTurnSpeed[1];
                if ( CG_PlayerUsingHandbrakeInVehicle(localClientNum) )
                    yawRate = CG_GetPlayerVehicleHandbrakeTurnSpeedValue(localClientNum) + yawRate;
                CL_CapTurnRate(localClientNum, pitchRate, yawRate);
            }
        }
    }
}

bool __cdecl CG_PlayerUsingHandbrakeInVehicle(int localClientNum)
{
    cg_s *cgameGlob; // [esp+Ch] [ebp-Ch]
    centity_s *vehicle; // [esp+10h] [ebp-8h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( (cgameGlob->predictedPlayerState.eFlags & 0x4000) == 0 )
        return 0;
    vehicle = CG_GetEntity(localClientNum, cgameGlob->predictedPlayerState.viewlocked_entNum);
    return vehicle->nitrousVeh && vehicle->nitrousVeh->m_hand_brake > 0.001;
}

double __cdecl CG_GetPlayerVehicleHandbrakeTurnSpeedValue(int localClientNum)
{
    cg_s *cgameGlob; // [esp+10h] [ebp-Ch]
    centity_s *vehicle; // [esp+14h] [ebp-8h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( (cgameGlob->predictedPlayerState.eFlags & 0x4000) == 0 )
        return 0.0;
    vehicle = CG_GetEntity(localClientNum, cgameGlob->predictedPlayerState.viewlocked_entNum);
    if ( vehicle->nitrousVeh )
        return CG_GetVehicleInfo(vehicle->nextState.vehicleState.vehicleInfoIndex)->thirdPersonCameraHandbrakeTurnRateInc
                 * vehicle->nitrousVeh->m_hand_brake;
    else
        return 0.0;
}

static float offset = 5.0f;
void __cdecl CG_MolotovFloat(int localClientNum, const centity_s *cent, const entityState_s *es)
{
    XModel *model; // [esp+4h] [ebp-38h]
    float tagOrigin[3]; // [esp+Ch] [ebp-30h] BYREF
    float tagMat[3][3]; // [esp+18h] [ebp-24h] BYREF

    if ( !localClientNum )
    {
        if ( !cent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 8255, 0, "%s", "cent") )
            __debugbreak();
        model = BG_GetWeaponDef(cent->nextState.weapon)->projectileModel;
        if ( !model && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 8260, 0, "%s", "model") )
            __debugbreak();
        if ( model )
        {
            AnglesToAxis(cent->pose.angles, tagMat);
            tagOrigin[0] = cent->pose.origin[0];
            tagOrigin[1] = cent->pose.origin[1];
            tagOrigin[2] = cent->pose.origin[2] + offset;
            CG_DestructibleSpawnDynEnt(cent, model, tagOrigin, tagMat, es->lerp.pos.trBase, es->lerp.pos.trDelta, 17);
        }
    }
}

Material *__cdecl CG_GetWeaponOverlay(int localClientNum, unsigned int weaponIndex)
{
    const WeaponVariantDef *variantDef; // [esp+0h] [ebp-8h]

    variantDef = BG_GetWeaponVariantDef(weaponIndex);
    if ( !variantDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_weapons.cpp", 8280, 0, "%s", "variantDef") )
    {
        __debugbreak();
    }
    if ( CG_UsingLowResViewPort(localClientNum) && variantDef->overlayMaterialLowRes )
        return variantDef->overlayMaterialLowRes;
    else
        return variantDef->overlayMaterial;
}

bool __cdecl CG_IsInGuidedMissile(const playerState_s *ps)
{
    if ( (ps->eFlags2 & 0x40000) == 0
        && (BG_GetWeaponDef(ps->weapon)->guidedMissileType != MISSILE_GUIDANCE_TVGUIDED || ps->fWeaponPosFrac < 0.5) )
    {
        return 0;
    }
    if ( ps->fWeaponPosFrac >= 0.94999999 )
        return 1;
    return ps->viewlocked_entNum != 1023;
}

bool __cdecl CG_IsInGuidedMissileStatic(const playerState_s *ps)
{
    return (ps->eFlags2 & 0x40000) != 0 && ps->viewlocked_entNum == 1023 && !BG_GetAmmoInClip(ps, ps->weapon);
}
