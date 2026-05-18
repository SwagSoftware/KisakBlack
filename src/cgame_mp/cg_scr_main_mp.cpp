#include "cg_scr_main_mp.h"
#include <clientscript/cscr_variable.h>
#include <clientscript/cscr_vm.h>
#include <cgame/cg_camerashake.h>
#include <EffectsCore/fx_system.h>
#include <cgame/cg_scr_main.h>
#include <qcommon/dobj_management.h>
#include <clientscript/cscr_stringlist.h>
#include <cgame/cg_event.h>
#include <demo/demo_playback.h>
#include "cg_main_mp.h"
#include "cg_ents_mp.h"
#include <bgame/bg_perks.h>
#include "cg_vehicles_mp.h"
#include <bgame/bg_dog.h>
#include <bgame/bg_mantle.h>
#include <bgame/bg_weapons_ammo.h>
#include <client/splitscreen.h>
#include <cgame/cg_compass.h>
#include <xanim/dobj_utils.h>
#include <xanim/xmodel.h>
#include <client_mp/cl_cgame_mp.h>
#include <clientscript/scr_const.h>
#include "cg_animscripted_mp.h"
#include <universal/surfaceflags.h>
#include <gfx_d3d/r_fog.h>
#include "cg_servercmds_mp.h"

GfxFog cg_clientVolFog;

unsigned __int16 *footTags[4] =
{
    &scr_const.j_palm_ri,
    &scr_const.j_palm_le,
    &scr_const.j_ball_ri,
    &scr_const.j_palm_ri
};

BuiltinFunctionDef client_project_functions[] =
{
  { "getgridfrompos", &CScr_GetGridFromPos, 0 },
  { "compassscale", &CScr_CompassScale, 0 },
  { "resetcompassscale", &CScr_ResetCompassScale, 0 },
  { "isdemoplaying", &CScr_IsDemoPlaying, 0 },
  { "isspectating", &CScr_IsSpectating, 0 },
  { "getlocalplayerteam", &CScr_GetLocalPlayerTeam, 0 },
  { "playfxontag", &CScr_PlayFXOnTag, 0 },
  { "playviewmodelfx", &CScr_PlayViewmodelFX, 0 },
  { "spawnfx", &CScr_SpawnFX, 0 },
  { "deletefx", &CScr_DeleteFX, 0 },
  { "getanimlength", &CScr_GetAnimLength, 0 },
  { "animateui", &CScr_AnimateUI, 0 },
  { "showui", &CScr_ShowUI, 0 },
  { "getcurrentweapon", &CScr_GetCurrentWeapon, 0 },
  { "getcurrentweaponincludingmelee", &CScr_GetCurrentWeaponIncludingMelee, 0 },
  { "hasweapon", &CScr_HasWeapon, 0 },
  { "gettotalammo", &CScr_GetTotalAmmo, 0 },
  { "setlocalradarenabled", &CScr_SetLocalRadarEnabled, 0 },
  { "setlocalradarposition", &CScr_SetLocalRadarPosition, 0 },
  { "setextracamentity", &CScr_SetExtraCamEntity, 0 },
  { "setextracamactive", &CScr_SetExtraCamActive, 0 },
  { "getextracamstatic", &CScr_GetExtraCamStatic, 0 },
  { "setextracamstatic", &CScr_SetExtraCamStatic, 0 },
  { "setextracamorigin", &CScr_SetExtraCamOrigin, 0 },
  { "setextracamangles", &CScr_SetExtraCamAngles, 0 },
  { "iscameraspiketoggled", &CScr_IsCameraSpikeToggled, 0 },
  // NEW FUNCS FROM BLOPS MP RETAIL (LATEST)
  { "setclientvolumetricfog", &CScr_SetClientVolumetricFog, 0 },
  { "switchtoservervolumetricfog", &CScr_SwitchToServerVolumetricFog, 0 },
  { "switchtoclientvolumetricfog", &CScr_SwitchToClientVolumetricFog, 0 },
  { "isinhelicopter", &CScr_IsInHelicopter, 0 },
};

// LWSS: Looks congruent to retail blops MP
const BuiltinMethodDef client_project_methods[29] =
{
  { "gettagorigin", &CScr_GetTagOrigin, 0 },
  { "gettagangles", &CScr_GetTagAngles, 0 },
  { "getinkillcam", &CScr_GetInKillcam, 0 },
  { "getowner", &CScrCmd_GetOwner, 0 },
  { "getanimstate", &CScr_GetAnimState, 0 },
  { "getanimstatecategory", &CScr_GetAnimStateCategory, 0 },
  { "getvehiclehealth", &CScr_GetVehicleHealth, 0 },
  { "getlefttreadhealth", &CScr_GetLeftTreadHealth, 0 },
  { "getrighttreadhealth", &CScr_GetRightTreadHealth, 0 },
  { "gethelidamagestate", &CScr_GetHeliDamageState, 0 },
  { "isburning", &CScrCmd_IsBurning, 0 },
  { "hasperk", &CPlayerCmd_HasPerk, 0 },
  { "getstance", &CScr_GetStance, 0 },
  { "shellshock", &CScrCmd_ShellShock, 0 },
  { "earthquake", &CScrCmd_Earthquake, 0 },
  { "setenemyglobalscrambler", &CScr_SetEnemyGlobalScrambler, 0 },
  { "setenemyscrambleramount", &CScr_SetEnemyScramblerAmount, 0 },
  { "getenemyscrambleramount", &CScr_GetEnemyScramblerAmount, 0 },
  { "isscrambled", &CScr_IsScrambled, 0 },
  { "setfriendlyscrambleramount", &CScr_SetFriendlyScramblerAmount, 0 },
  { "getfriendlyscrambleramount", &CScr_GetFriendlyScramblerAmount, 0 },
  { "addfriendlyscrambler", &CScr_AddFriendlyScrambler, 0 },
  { "clearnearestenemyscrambler", &CScr_ClearNearestEnemyScrambler, 0 },
  { "setnearestenemyscrambler", &CScr_SetNearestEnemyScrambler, 0 },
  { "removefriendlyscrambler", &CScr_RemoveFriendlyScrambler, 0 },
  { "removeallfriendlyscramblers", &CScr_RemoveAllFriendlyScramblers, 0 },
  { "hastacticalmaskoverlay", CScr_HasTacticalMaskOverlay, 0 },
  { "setflagasaway", &CScr_SetFlagAsAway, 0 },
  { "getparententity", &CScr_GetParentEntity, 0 }
};

cached_tag_mat_t cg_cachedTagMat;
cscr_mp_data_t cg_scr_mp_data;

//void __cdecl CScrCmd_Earthquake(scr_entref_t entref)
//{
//    float Float; // [esp+20h] [ebp-1Ch]
//    float source[3]; // [esp+24h] [ebp-18h] BYREF
//    int duration; // [esp+30h] [ebp-Ch]
//    float radius; // [esp+34h] [ebp-8h]
//    float scale; // [esp+38h] [ebp-4h]
//
//    scale = Scr_GetFloat(0, SCRIPTINSTANCE_CLIENT);
//    Float = Scr_GetFloat(1u, SCRIPTINSTANCE_CLIENT);
//    duration = (int)((float)(Float * 1000.0) + 9.313225746154785e-10);
//    Scr_GetVector(2u, source, SCRIPTINSTANCE_CLIENT);
//    radius = Scr_GetFloat(3u, SCRIPTINSTANCE_CLIENT);
//    CG_StartShakeCamera(entref.client, scale, duration, source, radius);
//}

unsigned int __cdecl CScr_SpawnFXInternal(int localClientNum, int fxId, float (*axis)[3], float *pos, int time)
{
    const FxEffectDef *fxDef; // [esp+0h] [ebp-8h]

    fxDef = CG_GetLocalClientStaticGlobals(localClientNum)->fxs[fxId];
    if ( !fxDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scr_main_mp.cpp", 91, 0, "%s", "fxDef") )
    {
        __debugbreak();
    }
    return FX_SpawnOrientedEffect(localClientNum, fxDef, time, pos, axis, 0x3FFu);
}

void CScr_DeleteFX()
{
    int localClientNum; // [esp+8h] [ebp-Ch]
    int intFxPtr; // [esp+Ch] [ebp-8h]

    localClientNum = CScr_GetLocalClientNum(0);
    intFxPtr = Scr_GetInt(1u, SCRIPTINSTANCE_CLIENT);
    if ( (unsigned int)localClientNum >= 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scr_main_mp.cpp",
                    118,
                    0,
                    "localClientNum not in [0, MAX_LOCAL_CLIENTS]\n\t%i not in [%i, %i]",
                    localClientNum,
                    0,
                    1) )
    {
        __debugbreak();
    }
    FX_ThroughWithEffect(localClientNum, intFxPtr, 1);
}

void CScr_SpawnFX()
{
    unsigned int v0; // [esp+0h] [ebp-94h]
    unsigned int v1; // [esp+10h] [ebp-84h]
    unsigned int value; // [esp+20h] [ebp-74h]
    float Float; // [esp+34h] [ebp-60h]
    char *error; // [esp+38h] [ebp-5Ch]
    float pos[3]; // [esp+44h] [ebp-50h] BYREF
    int iTime; // [esp+50h] [ebp-44h]
    int localClientNum; // [esp+54h] [ebp-40h]
    int numParams; // [esp+58h] [ebp-3Ch]
    float angles[3]; // [esp+5Ch] [ebp-38h] BYREF
    int fxId; // [esp+68h] [ebp-2Ch]
    float axis[3][3]; // [esp+6Ch] [ebp-28h] BYREF
    float vecLength; // [esp+90h] [ebp-4h]

    numParams = Scr_GetNumParam(SCRIPTINSTANCE_CLIENT);
    if ( numParams < 4 || numParams > 6 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "Incorrect number of parameters", 0);
    localClientNum = CScr_GetLocalClientNum(0);
    if ( (unsigned int)localClientNum >= 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scr_main_mp.cpp",
                    161,
                    0,
                    "localClientNum not in [0, MAX_LOCAL_CLIENTS]\n\t%i not in [%i, %i]",
                    localClientNum,
                    0,
                    1) )
    {
        __debugbreak();
    }
    fxId = Scr_GetInt(1u, SCRIPTINSTANCE_CLIENT);
    if ( fxId <= 0 || fxId >= 196 )
    {
        error = va("CScr_PlayFX: invalid effect id %d", fxId);
        Scr_Error(SCRIPTINSTANCE_CLIENT, error, 0);
    }
    Scr_GetVector(2u, pos, SCRIPTINSTANCE_CLIENT);
    Float = Scr_GetFloat(3u, SCRIPTINSTANCE_CLIENT);
    iTime = (int)((float)(Float * 1000.0) + 9.313225746154785e-10);
    if ( numParams == 4 )
    {
        CScr_SetFxAngles(0, axis, angles);
        value = CScr_SpawnFXInternal(localClientNum, fxId, axis, pos, iTime);
        Scr_AddInt(value, SCRIPTINSTANCE_CLIENT);
    }
    else
    {
        if ( numParams != 5
            && numParams != 6
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scr_main_mp.cpp",
                        179,
                        1,
                        "%s\n\t(numParams) = %i",
                        "(numParams == 5 || numParams == 6)",
                        numParams) )
        {
            __debugbreak();
        }
        Scr_GetVector(4u, axis[0], SCRIPTINSTANCE_CLIENT);
        vecLength = Vec3Normalize(axis[0]);
        if ( vecLength == 0.0 )
            CScr_FxParamError(localClientNum, 4u, "playFx called with (0 0 0) forward direction", fxId);
        if ( numParams == 5 )
        {
            CScr_SetFxAngles(1u, axis, angles);
            v1 = CScr_SpawnFXInternal(localClientNum, fxId, axis, pos, iTime);
            Scr_AddInt(v1, SCRIPTINSTANCE_CLIENT);
        }
        else
        {
            if ( numParams != 6
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scr_main_mp.cpp",
                            193,
                            1,
                            "%s\n\t(numParams) = %i",
                            "(numParams == 6)",
                            numParams) )
            {
                __debugbreak();
            }
            Scr_GetVector(5u, axis[2], SCRIPTINSTANCE_CLIENT);
            vecLength = Vec3Normalize(axis[2]);
            if ( vecLength == 0.0 )
                CScr_FxParamError(localClientNum, 5u, "playFx called with (0 0 0) up direction", fxId);
            CScr_SetFxAngles(2u, axis, angles);
            v0 = CScr_SpawnFXInternal(localClientNum, fxId, axis, pos, iTime);
            Scr_AddInt(v0, SCRIPTINSTANCE_CLIENT);
        }
    }
}

void CScr_PlayFXOnTag()
{
    char *v0; // [esp+0h] [ebp-60h]
    char *error; // [esp+4h] [ebp-5Ch]
    scr_entref_t v2; // [esp+10h] [ebp-50h] BYREF
    scr_entref_t v3; // [esp+1Ah] [ebp-46h]
    scr_entref_t v4; // [esp+28h] [ebp-38h]
    scr_entref_t v5; // [esp+32h] [ebp-2Eh]
    scr_entref_t entref; // [esp+38h] [ebp-28h]
    unsigned int tagName; // [esp+40h] [ebp-20h]
    unsigned int effectHandle; // [esp+44h] [ebp-1Ch]
    int localClientNum; // [esp+48h] [ebp-18h]
    const FxEffectDef *fxDef; // [esp+4Ch] [ebp-14h]
    int numParams; // [esp+50h] [ebp-10h]
    const char *name; // [esp+54h] [ebp-Ch]
    cgs_t *cgs; // [esp+58h] [ebp-8h]
    int fxId; // [esp+5Ch] [ebp-4h]

    numParams = Scr_GetNumParam(SCRIPTINSTANCE_CLIENT);
    if ( numParams != 4 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "Incorrect number of parameters for playfxontag", 0);
    localClientNum = CScr_GetLocalClientNum(0);
    fxId = Scr_GetInt(1u, SCRIPTINSTANCE_CLIENT);
    v3 = Scr_GetEntityRef(2, SCRIPTINSTANCE_CLIENT);
    v4 = v3;
    v5 = v3;
    entref = v3;
    if ( !Com_GetClientDObj(v3.entnum, localClientNum) )
    {
        error = va(
                            "CScr_PlayFX: invalid entity for local client %i, either not in the snapshot for the client or dobj does not exist yet",
                            localClientNum);
        Scr_Error(SCRIPTINSTANCE_CLIENT, error, 0);
    }
    if ( fxId <= 0 || fxId >= 196 )
    {
        v0 = va("CScr_PlayFX: invalid effect id %d", fxId);
        Scr_Error(SCRIPTINSTANCE_CLIENT, v0, 0);
    }
    cgs = CG_GetLocalClientStaticGlobals(localClientNum);
    fxDef = cgs->fxs[fxId];
    if ( !fxDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scr_main_mp.cpp", 251, 0, "%s", "fxDef") )
    {
        __debugbreak();
    }
    tagName = Scr_GetConstLowercaseString(3u, SCRIPTINSTANCE_CLIENT);
    name = SL_ConvertToString(tagName, SCRIPTINSTANCE_CLIENT);
    tagName = SL_FindLowercaseString(name, SCRIPTINSTANCE_SERVER);
    effectHandle = CG_PlayBoltedEffect(localClientNum, fxDef, entref.entnum, tagName);
    Scr_AddInt(effectHandle, SCRIPTINSTANCE_CLIENT);
}

void CScr_PlayViewmodelFX()
{
    cg_s *LocalClientGlobals; // eax
    unsigned int v1; // eax
    const char *v2; // eax
    int v3; // eax
    unsigned int v4; // [esp-8h] [ebp-48h]
    char *v5; // [esp-4h] [ebp-44h]
    unsigned int v6; // [esp-4h] [ebp-44h]
    char *error; // [esp+0h] [ebp-40h]
    unsigned int handle; // [esp+20h] [ebp-20h]
    unsigned __int8 boneIndex; // [esp+27h] [ebp-19h] BYREF
    int weaponNum; // [esp+28h] [ebp-18h]
    int nparams; // [esp+2Ch] [ebp-14h]
    int localClientNum; // [esp+30h] [ebp-10h]
    const cgs_t *cgs; // [esp+34h] [ebp-Ch]
    int fxId; // [esp+38h] [ebp-8h]
    int realTagName; // [esp+3Ch] [ebp-4h]

    nparams = Scr_GetNumParam(SCRIPTINSTANCE_CLIENT);
    if ( nparams != 3 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "PlayViewmodelFX() called with wrong params.\n", 0);
    localClientNum = CScr_GetLocalClientNum(0);
    if ( (unsigned int)localClientNum >= 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scr_main_mp.cpp",
                    275,
                    0,
                    "localClientNum not in [0, MAX_LOCAL_CLIENTS]\n\t%i not in [%i, %i]",
                    localClientNum,
                    0,
                    1) )
    {
        __debugbreak();
    }
    fxId = Scr_GetInt(1u, SCRIPTINSTANCE_CLIENT);
    if ( (unsigned int)fxId > 0xC4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scr_main_mp.cpp",
                    278,
                    0,
                    "fxId not in [0, MAX_EFFECT_NAMES]\n\t%i not in [%i, %i]",
                    fxId,
                    0,
                    196) )
    {
        __debugbreak();
    }
    realTagName = CScr_GetConstServerString(2u);
    if ( !realTagName )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "PlayViewmodelFX(): unable to find viewmodel tag.\n", 0);
    cgs = CG_GetLocalClientStaticGlobals(localClientNum);
    if ( !cgs && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scr_main_mp.cpp", 287, 0, "%s", "cgs") )
        __debugbreak();
    LocalClientGlobals = CG_GetLocalClientGlobals(localClientNum);
    weaponNum = BG_GetViewmodelWeaponIndex(&LocalClientGlobals->predictedPlayerState);
    boneIndex = -2;
    v4 = realTagName;
    v1 = CG_WeaponDObjHandle(localClientNum);
    if ( !CG_GetBoneIndex(localClientNum, v1, v4, &boneIndex) )
    {
        v5 = SL_ConvertToString(realTagName, SCRIPTINSTANCE_SERVER);
        v2 = BG_WeaponName(weaponNum);
        error = va("PlayViewmodelFX(): viewmodel weapon '%s', does not have bone '%s'", v2, v5);
        Scr_Error(SCRIPTINSTANCE_CLIENT, error, 0);
    }
    v6 = realTagName;
    v3 = CG_WeaponDObjHandle(localClientNum);
    handle = CG_PlayBoltedEffect(localClientNum, cgs->fxs[fxId], v3, v6);
    Scr_AddInt(handle, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_IsDemoPlaying()
{
    if ( Demo_IsPlaying() )
        Scr_AddInt(1, SCRIPTINSTANCE_CLIENT);
    else
        Scr_AddInt(0, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_IsSpectating()
{
    cg_s *cgameGlob; // [esp+8h] [ebp-Ch]
    VariableUnion localClientNum; // [esp+Ch] [ebp-8h]

    localClientNum.intValue = CScr_GetLocalClientNum(0);
    cgameGlob = CG_GetLocalClientGlobals(localClientNum.intValue);
    if ( !cgameGlob
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scr_main_mp.cpp", 343, 0, "%s", "cgameGlob") )
    {
        __debugbreak();
    }
    if ( cgameGlob == (cg_s *)-263324
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scr_main_mp.cpp", 346, 0, "%s", "ps") )
    {
        __debugbreak();
    }
    if ( Demo_IsPlaying() )
    {
        if ( Demo_IsMovieCamera() || Demo_IsThirdPersonCamera() )
        {
LABEL_17:
            Scr_AddInt(1, SCRIPTINSTANCE_CLIENT);
            return;
        }
    }
    else if ( ((unsigned int)Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) <= 1
                    || !Scr_GetInt(1u, SCRIPTINSTANCE_CLIENT)
                    || cgameGlob->renderingThirdPerson
                    || (cgameGlob->predictedPlayerState.otherFlags & 2) == 0)
                 && (cgameGlob->predictedPlayerState.otherFlags & 0x1A) != 0 )
    {
        goto LABEL_17;
    }
    Scr_AddInt(0, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScrCmd_IsBurning(scr_entref_t entref)
{
    centity_s *pSelf; // [esp+Ch] [ebp-Ch]
    unsigned int isBurning; // [esp+14h] [ebp-4h]

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scr_main_mp.cpp",
                        389,
                        0,
                        "%s",
                        "entref.entnum < MAX_LOCAL_CENTITIES") )
        {
            __debugbreak();
        }
        if ( CG_GetClientNumForLocalClient(entref.client) == entref.entnum )
            pSelf = &CG_GetLocalClientGlobals(entref.client)->predictedPlayerEntity;
        else
            pSelf = CG_GetEntity(entref.client, entref.entnum);
        if ( entref.entnum >= 0x400u )
            CG_GetFakeEntity(entref.client, entref.entnum);
    }
    isBurning = 0;
    if ( (pSelf->currentState.eFlags2 & 0x200000) != 0 || ((*((unsigned int *)pSelf + 201) >> 5) & 1) != 0 )
        isBurning = 1;
    Scr_AddBool(isBurning, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CPlayerCmd_HasPerk(scr_entref_t entref)
{
    unsigned int value; // [esp+0h] [ebp-18h]
    char *error; // [esp+8h] [ebp-10h]
    char *perkName; // [esp+Ch] [ebp-Ch]
    cg_s *cGameGlob; // [esp+10h] [ebp-8h]
    unsigned int perkIndex; // [esp+14h] [ebp-4h]

    if ( entref.entnum >= 0x20u )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "hasperk() can only be called on players", 0);
    perkName = Scr_GetString(0, SCRIPTINSTANCE_CLIENT);
    perkIndex = BG_GetPerkIndexForName(perkName);
    if ( perkIndex == 52 )
    {
        error = va("Unknown perk: %s\n", perkName);
        Scr_Error(SCRIPTINSTANCE_CLIENT, error, 0);
    }
    cGameGlob = CG_GetLocalClientGlobals(0);
    value = BG_HasPerk(cGameGlob->bgs.clientinfo[entref.entnum].perks, perkIndex);
    Scr_AddBool(value, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_GetVehicleHealth(scr_entref_t entref)
{
    float value; // [esp+8h] [ebp-18h]
    centity_s *pSelf; // [esp+18h] [ebp-8h]

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scr_main_mp.cpp",
                        445,
                        0,
                        "%s",
                        "entref.entnum < MAX_LOCAL_CENTITIES") )
        {
            __debugbreak();
        }
        if ( CG_GetClientNumForLocalClient(entref.client) == entref.entnum )
            pSelf = &CG_GetLocalClientGlobals(entref.client)->predictedPlayerEntity;
        else
            pSelf = CG_GetEntity(entref.client, entref.entnum);
        if ( entref.entnum >= 0x400u )
            CG_GetFakeEntity(entref.client, entref.entnum);
    }
    if ( pSelf->nextState.eType != 14 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "GetVehicleHealth() can only be called on vehicles", 0);
    value = CG_VehGetHealthPercentageEntity(pSelf);
    Scr_AddFloat(value, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_GetLeftTreadHealth(scr_entref_t entref)
{
    float value; // [esp+8h] [ebp-18h]
    centity_s *pSelf; // [esp+18h] [ebp-8h]

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scr_main_mp.cpp",
                        470,
                        0,
                        "%s",
                        "entref.entnum < MAX_LOCAL_CENTITIES") )
        {
            __debugbreak();
        }
        if ( CG_GetClientNumForLocalClient(entref.client) == entref.entnum )
            pSelf = &CG_GetLocalClientGlobals(entref.client)->predictedPlayerEntity;
        else
            pSelf = CG_GetEntity(entref.client, entref.entnum);
        if ( entref.entnum >= 0x400u )
            CG_GetFakeEntity(entref.client, entref.entnum);
    }
    if ( pSelf->nextState.eType != 14 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "GetLeftTreadHealth() can only be called on vehicles", 0);
    value = CG_VehGetHealthPercentageLeftTread(pSelf);
    Scr_AddFloat(value, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_GetHeliDamageState(scr_entref_t entref)
{
    centity_s *pSelf; // [esp+10h] [ebp-8h]

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scr_main_mp.cpp",
                        495,
                        0,
                        "%s",
                        "entref.entnum < MAX_LOCAL_CENTITIES") )
        {
            __debugbreak();
        }
        if ( CG_GetClientNumForLocalClient(entref.client) == entref.entnum )
            pSelf = &CG_GetLocalClientGlobals(entref.client)->predictedPlayerEntity;
        else
            pSelf = CG_GetEntity(entref.client, entref.entnum);
        if ( entref.entnum >= 0x400u )
            CG_GetFakeEntity(entref.client, entref.entnum);
    }
    if ( pSelf->nextState.eType != 12 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "GetHeliDamageState() can only be called on helicopters", 0);
    Scr_AddInt(pSelf->nextState.un1.scale, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_GetRightTreadHealth(scr_entref_t entref)
{
    float value; // [esp+8h] [ebp-18h]
    centity_s *pSelf; // [esp+18h] [ebp-8h]

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scr_main_mp.cpp",
                        520,
                        0,
                        "%s",
                        "entref.entnum < MAX_LOCAL_CENTITIES") )
        {
            __debugbreak();
        }
        if ( CG_GetClientNumForLocalClient(entref.client) == entref.entnum )
            pSelf = &CG_GetLocalClientGlobals(entref.client)->predictedPlayerEntity;
        else
            pSelf = CG_GetEntity(entref.client, entref.entnum);
        if ( entref.entnum >= 0x400u )
            CG_GetFakeEntity(entref.client, entref.entnum);
    }
    if ( pSelf->nextState.eType != 14 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "GetRightTreadHealth() can only be called on vehicles", 0);
    value = CG_VehGetHealthPercentageRightTread(pSelf);
    Scr_AddFloat(value, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_GetInKillcam(scr_entref_t entref)
{
    cg_s *cGameGlob; // [esp+1Ch] [ebp-Ch]
    VariableUnion localClientNum; // [esp+20h] [ebp-8h]

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scr_main_mp.cpp",
                        546,
                        0,
                        "%s",
                        "entref.entnum < MAX_LOCAL_CENTITIES") )
        {
            __debugbreak();
        }
        if ( CG_GetClientNumForLocalClient(entref.client) == entref.entnum )
            CG_GetLocalClientGlobals(entref.client);
        else
            CG_GetEntity(entref.client, entref.entnum);
        if ( entref.entnum >= 0x400u )
            CG_GetFakeEntity(entref.client, entref.entnum);
    }
    localClientNum.intValue = CScr_GetLocalClientNum(0);
    cGameGlob = CG_GetLocalClientGlobals(localClientNum.intValue);
    Scr_AddInt(cGameGlob->inKillCam, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_GetAnimState(scr_entref_t entref)
{
    char *value; // [esp+0h] [ebp-18h]
    centity_s *pSelf; // [esp+10h] [ebp-8h]

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scr_main_mp.cpp",
                        569,
                        0,
                        "%s",
                        "entref.entnum < MAX_LOCAL_CENTITIES") )
        {
            __debugbreak();
        }
        if ( CG_GetClientNumForLocalClient(entref.client) == entref.entnum )
            pSelf = &CG_GetLocalClientGlobals(entref.client)->predictedPlayerEntity;
        else
            pSelf = CG_GetEntity(entref.client, entref.entnum);
        if ( entref.entnum >= 0x400u )
            CG_GetFakeEntity(entref.client, entref.entnum);
    }
    if ( pSelf->nextState.eType != 17 && pSelf->nextState.eType != 19 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "GetAnimState() can only be called on actors", 0);
    value = BG_Actor_GetAnimStateName(pSelf->nextState.animState.state);
    Scr_AddString(value, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_GetAnimStateCategory(scr_entref_t entref)
{
    char *value; // [esp+0h] [ebp-18h]
    centity_s *pSelf; // [esp+10h] [ebp-8h]

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scr_main_mp.cpp",
                        593,
                        0,
                        "%s",
                        "entref.entnum < MAX_LOCAL_CENTITIES") )
        {
            __debugbreak();
        }
        if ( CG_GetClientNumForLocalClient(entref.client) == entref.entnum )
            pSelf = &CG_GetLocalClientGlobals(entref.client)->predictedPlayerEntity;
        else
            pSelf = CG_GetEntity(entref.client, entref.entnum);
        if ( entref.entnum >= 0x400u )
            CG_GetFakeEntity(entref.client, entref.entnum);
    }
    if ( pSelf->nextState.eType != 17 && pSelf->nextState.eType != 19 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "GetAnimStateCategory() can only be called on actors", 0);
    value = BG_Actor_GetAnimStateCategoryName(pSelf->nextState.animState.state);
    Scr_AddString(value, SCRIPTINSTANCE_CLIENT);
}

void CScr_GetTotalAmmo()
{
    char *weaponName; // [esp+10h] [ebp-1Ch]
    int localClientNum; // [esp+18h] [ebp-14h]
    int ammoStock; // [esp+1Ch] [ebp-10h]
    int roundsInClip; // [esp+20h] [ebp-Ch]
    unsigned int weapIdx; // [esp+24h] [ebp-8h]
    const playerState_s *ps; // [esp+28h] [ebp-4h]

    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) == 2 )
    {
        localClientNum = CScr_GetLocalClientNum(0);
        if ( !CG_GetLocalClientGlobals(localClientNum)
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scr_main_mp.cpp", 625, 0, "%s", "cgameGlob") )
        {
            __debugbreak();
        }
        weaponName = Scr_GetString(1u, SCRIPTINSTANCE_CLIENT);
        if ( !weaponName
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scr_main_mp.cpp",
                        627,
                        0,
                        "%s",
                        "weaponName") )
        {
            __debugbreak();
        }
        ps = CG_GetPredictedPlayerState(localClientNum);
        weapIdx = BG_FindWeaponIndexForName(weaponName);
        roundsInClip = BG_GetAmmoInClip(ps, weapIdx);
        ammoStock = BG_GetTotalAmmoReserve(ps, weapIdx);
        Scr_AddInt(ammoStock + roundsInClip, SCRIPTINSTANCE_CLIENT);
    }
    else
    {
        Scr_Error(
            SCRIPTINSTANCE_CLIENT,
            "Incorrect number of parameters passed to GetTotalAmmo()\nUSAGE: GetTotalAmmo( <localClientNum>, <weaponName> )\n",
            0);
    }
}

void CScr_GetCurrentWeapon()
{
    const cg_s *cgameGlob; // [esp+Ch] [ebp-Ch]
    VariableUnion localClientNum; // [esp+10h] [ebp-8h]
    const WeaponVariantDef *varDef; // [esp+14h] [ebp-4h]

    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) == 1 )
    {
        localClientNum.intValue = CScr_GetLocalClientNum(0);
        cgameGlob = CG_GetLocalClientGlobals(localClientNum.intValue);
        if ( !cgameGlob
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scr_main_mp.cpp", 661, 0, "%s", "cgameGlob") )
        {
            __debugbreak();
        }
        varDef = BG_GetWeaponVariantDef(cgameGlob->predictedPlayerState.weapon);
        if ( !varDef
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scr_main_mp.cpp", 665, 0, "%s", "varDef") )
        {
            __debugbreak();
        }
        if ( !varDef->szInternalName
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scr_main_mp.cpp",
                        666,
                        0,
                        "%s",
                        "varDef->szInternalName") )
        {
            __debugbreak();
        }
        Scr_AddString((char *)varDef->szInternalName, SCRIPTINSTANCE_CLIENT);
    }
    else
    {
        Scr_Error(
            SCRIPTINSTANCE_CLIENT,
            "Incorrect number of parameters passed to GetCurrentWeapon()\nUSAGE: GetCurrentWeapon( <localClientNum> )\n",
            0);
    }
}

void CScr_GetCurrentWeaponIncludingMelee()
{
    const cg_s *cgameGlob; // [esp+Ch] [ebp-14h]
    unsigned __int16 weapon; // [esp+10h] [ebp-10h]
    VariableUnion localClientNum; // [esp+14h] [ebp-Ch]
    const WeaponVariantDef *varDef; // [esp+18h] [ebp-8h]

    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) == 1 )
    {
        localClientNum.intValue = CScr_GetLocalClientNum(0);
        cgameGlob = CG_GetLocalClientGlobals(localClientNum.intValue);
        if ( !cgameGlob
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scr_main_mp.cpp", 698, 0, "%s", "cgameGlob") )
        {
            __debugbreak();
        }
        if ( cgameGlob->predictedPlayerState.weaponstate == 17
            || cgameGlob->predictedPlayerState.weaponstate == 18
            || cgameGlob->predictedPlayerState.weaponstate == 19 )
        {
            weapon = cgameGlob->predictedPlayerState.meleeWeapon;
        }
        else
        {
            weapon = cgameGlob->predictedPlayerState.weapon;
        }
        varDef = BG_GetWeaponVariantDef(weapon);
        if ( !varDef
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scr_main_mp.cpp", 708, 0, "%s", "varDef") )
        {
            __debugbreak();
        }
        if ( !varDef->szInternalName
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scr_main_mp.cpp",
                        709,
                        0,
                        "%s",
                        "varDef->szInternalName") )
        {
            __debugbreak();
        }
        Scr_AddString((char *)varDef->szInternalName, SCRIPTINSTANCE_CLIENT);
    }
    else
    {
        Scr_Error(
            SCRIPTINSTANCE_CLIENT,
            "Incorrect number of parameters passed to GetCurrentWeapon()\nUSAGE: GetCurrentWeapon( <localClientNum> )\n",
            0);
    }
}

void CScr_HasWeapon()
{
    char *weaponName; // [esp+14h] [ebp-10h]
    const cg_s *cgameGlob; // [esp+18h] [ebp-Ch]
    VariableUnion localClientNum; // [esp+1Ch] [ebp-8h]
    unsigned int weapIdx; // [esp+20h] [ebp-4h]

    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) == 2 )
    {
        localClientNum.intValue = CScr_GetLocalClientNum(0);
        cgameGlob = CG_GetLocalClientGlobals(localClientNum.intValue);
        weaponName = Scr_GetString(1u, SCRIPTINSTANCE_CLIENT);
        if ( !weaponName
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scr_main_mp.cpp",
                        739,
                        0,
                        "%s",
                        "weaponName") )
        {
            __debugbreak();
        }
        if ( !cgameGlob
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scr_main_mp.cpp", 740, 0, "%s", "cgameGlob") )
        {
            __debugbreak();
        }
        weapIdx = BG_FindWeaponIndexForName(weaponName);
        if ( weapIdx && BG_PlayerHasWeapon(&cgameGlob->predictedPlayerState, weapIdx) )
            Scr_AddBool(1u, SCRIPTINSTANCE_CLIENT);
        else
            Scr_AddBool(0, SCRIPTINSTANCE_CLIENT);
    }
    else
    {
        Scr_Error(
            SCRIPTINSTANCE_CLIENT,
            "Incorrect number of parameters passed to HasWeapon()\nUSAGE: HasWeapon( <localClientNum>, <weaponName> )\n",
            0);
    }
}

void CScr_SetLocalRadarEnabled()
{
    cg_s *cgameGlob; // [esp+Ch] [ebp-8h]
    VariableUnion localClientNum; // [esp+10h] [ebp-4h]

    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) == 2 )
    {
        localClientNum.intValue = CScr_GetLocalClientNum(0);
        cgameGlob = CG_GetLocalClientGlobals(localClientNum.intValue);
        if ( Scr_GetInt(1u, SCRIPTINSTANCE_CLIENT) )
            cgameGlob->hasLocalRadar = 1;
        else
            cgameGlob->hasLocalRadar = 0;
    }
    else
    {
        Scr_Error(
            SCRIPTINSTANCE_CLIENT,
            "Incorrect number of parameters passed to SetLocalRadarEnabled()\n"
            "USAGE: SetLocalRadarEnabled( <localClientNum>, <enabled> )\n",
            0);
    }
}

void CScr_SetLocalRadarPosition()
{
    float origin[3]; // [esp+Ch] [ebp-14h] BYREF
    cg_s *cgameGlob; // [esp+18h] [ebp-8h]
    int localClientNum; // [esp+1Ch] [ebp-4h]

    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) == 2 )
    {
        localClientNum = CScr_GetLocalClientNum(0);
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        Scr_GetVector(1u, origin, SCRIPTINSTANCE_CLIENT);
        *(_QWORD *)cgameGlob->localRadarPos = *(_QWORD *)origin;
    }
    else
    {
        Scr_Error(
            SCRIPTINSTANCE_CLIENT,
            "Incorrect number of parameters passed to SetLocalRadarPosition()\n"
            "USAGE: SetLocalRadarPosition( <localClientNum>, <position> )\n",
            0);
    }
}

void CScr_SetExtraCamEntity()
{
    scr_entref_t v0; // [esp+0h] [ebp-38h] BYREF
    int v1; // [esp+Ah] [ebp-2Eh]
    cg_s *cgameGlob; // [esp+28h] [ebp-10h]
    int localClientNum; // [esp+2Ch] [ebp-Ch]

    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) == 2 )
    {
        localClientNum = CScr_GetLocalClientNum(0);
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        if ( Scr_GetType(1u, SCRIPTINSTANCE_CLIENT) )
        {
            //v1 = *(unsigned int *)&Scr_GetEntityRef(&v0, 1u, SCRIPTINSTANCE_CLIENT)->entnum;
            v1 = Scr_GetEntityRef(1u, SCRIPTINSTANCE_CLIENT).entnum;
            cgameGlob->extraCamEntity = (unsigned __int16)v1;
        }
        else
        {
            cgameGlob->extraCamEntity = 1023;
        }
    }
    else
    {
        Scr_Error(
            SCRIPTINSTANCE_CLIENT,
            "Incorrect number of parameters passed to SetExtraCamEntity()\nUSAGE: SetExtraCam( <localClientNum>, <entity> )\n",
            0);
    }
}

void CScr_SetExtraCamActive()
{
    cg_s *cgameGlob; // [esp+8h] [ebp-Ch]
    VariableUnion localClientNum; // [esp+Ch] [ebp-8h]

    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) == 2 )
    {
        if ( CL_LocalClient_GetActiveCount() <= 1 )
        {
            localClientNum.intValue = CScr_GetLocalClientNum(0);
            cgameGlob = CG_GetLocalClientGlobals(localClientNum.intValue);
            if ( Scr_GetInt(1u, SCRIPTINSTANCE_CLIENT) )
                cgameGlob->extraCamActive = 1;
            else
                cgameGlob->extraCamActive = 0;
        }
    }
    else
    {
        Scr_Error(
            SCRIPTINSTANCE_CLIENT,
            "Incorrect number of parameters passed to SetExtraCamActive()\nUSAGE: SetExtraCam( <localClientNum>, <active> )\n",
            0);
    }
}

void CScr_GetExtraCamStatic()
{
    VariableUnion localClientNum; // [esp+Ch] [ebp-4h]

    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) == 1 )
    {
        localClientNum.intValue = CScr_GetLocalClientNum(0);
        if ( CG_GetLocalClientGlobals(localClientNum.intValue)->extraCamStatic )
            Scr_AddInt(1, SCRIPTINSTANCE_CLIENT);
        else
            Scr_AddInt(0, SCRIPTINSTANCE_CLIENT);
    }
    else
    {
        Scr_Error(
            SCRIPTINSTANCE_CLIENT,
            "Incorrect number of parameters passed to GetExtraCamStatic()\nUSAGE: GetExtraCamStatic( <localClientNum> )\n",
            0);
    }
}

void CScr_SetExtraCamStatic()
{
    cg_s *cgameGlob; // [esp+8h] [ebp-Ch]
    VariableUnion localClientNum; // [esp+Ch] [ebp-8h]

    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) == 2 )
    {
        if ( CL_LocalClient_GetActiveCount() <= 1 )
        {
            localClientNum.intValue = CScr_GetLocalClientNum(0);
            cgameGlob = CG_GetLocalClientGlobals(localClientNum.intValue);
            if ( Scr_GetInt(1u, SCRIPTINSTANCE_CLIENT) )
                cgameGlob->extraCamStatic = 1;
            else
                cgameGlob->extraCamStatic = 0;
        }
    }
    else
    {
        Scr_Error(
            SCRIPTINSTANCE_CLIENT,
            "Incorrect number of parameters passed to SetExtraCamStatic()\n"
            "USAGE: SetExtraCamStatic( <localClientNum>, <active> )\n",
            0);
    }
}

void CScr_SetExtraCamOrigin()
{
    float *extraCamOrigin; // [esp+0h] [ebp-20h]
    float origin[3]; // [esp+Ch] [ebp-14h] BYREF
    cg_s *cgameGlob; // [esp+18h] [ebp-8h]
    int localClientNum; // [esp+1Ch] [ebp-4h]

    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) == 2 )
    {
        localClientNum = CScr_GetLocalClientNum(0);
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        Scr_GetVector(1u, origin, SCRIPTINSTANCE_CLIENT);
        extraCamOrigin = cgameGlob->extraCamOrigin;
        cgameGlob->extraCamOrigin[0] = origin[0];
        extraCamOrigin[1] = origin[1];
        extraCamOrigin[2] = origin[2];
    }
    else
    {
        Scr_Error(
            SCRIPTINSTANCE_CLIENT,
            "Incorrect number of parameters passed to SetExtraCamOrigin()\n"
            "USAGE: SetExtraCamOrigin( <localClientNum>, <origin> )\n",
            0);
    }
}

void CScr_SetExtraCamAngles()
{
    cg_s *cgameGlob; // [esp+Ch] [ebp-14h]
    VariableUnion localClientNum; // [esp+10h] [ebp-10h]
    float angles[3]; // [esp+14h] [ebp-Ch] BYREF

    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) == 2 )
    {
        localClientNum.intValue = CScr_GetLocalClientNum(0);
        cgameGlob = CG_GetLocalClientGlobals(localClientNum.intValue);
        Scr_GetVector(1u, angles, SCRIPTINSTANCE_CLIENT);
        cgameGlob->extraCamAngles[0] = angles[0];
        cgameGlob->extraCamAngles[1] = angles[1];
        cgameGlob->extraCamAngles[2] = angles[2];
    }
    else
    {
        Scr_Error(
            SCRIPTINSTANCE_CLIENT,
            "Incorrect number of parameters passed to SetExtraCamAngles()\n"
            "USAGE: SetExtraCamAngles( <localClientNum>, <angles> )\n",
            0);
    }
}

void CScr_IsCameraSpikeToggled()
{
    VariableUnion localClientNum; // [esp+Ch] [ebp-4h]

    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) == 1 )
    {
        localClientNum.intValue = CScr_GetLocalClientNum(0);
        if ( (CG_GetLocalClientGlobals(localClientNum.intValue)->predictedPlayerState.weapFlags & 0x200000) != 0 )
            Scr_AddInt(1, SCRIPTINSTANCE_CLIENT);
        else
            Scr_AddInt(0, SCRIPTINSTANCE_CLIENT);
    }
    else
    {
        Scr_Error(
            SCRIPTINSTANCE_CLIENT,
            "Incorrect number of parameters passed to IsCameraSpikeToggled\nUSAGE: IsCameraSpikeToggled( <localClientNum> )\n",
            0);
    }
}

void CScr_SetClientVolumetricFog()
{
    if (Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) != 18 && Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) != 8)
    {
        Scr_Error(
            SCRIPTINSTANCE_CLIENT,
            "Incorrect number of parameters\n"
            "USAGE: setClientVolFog(<startDist>, <halfwayDist>, <halfwayHeight>, <baseHeight>, <red>, <green>, <blue>, <transit"
            "ion time>, <sun red>, <sun blue>, <sun green>, <sun dir X>, <sun dir Y>, <sun dir Z>, <sun start angle>, <sun end "
            "angle>, <max fog opacity>)\n",
            0);
    }

    float startDist = Scr_GetFloat(0, SCRIPTINSTANCE_CLIENT);
    if (startDist < 0.0)
        Scr_Error(SCRIPTINSTANCE_CLIENT, "setClientVolFog: startDist must be greater or equal to 0", 0);

    float halfwayDist = Scr_GetFloat(1u, SCRIPTINSTANCE_CLIENT);
    if (halfwayDist <= 0.0)
        Scr_Error(SCRIPTINSTANCE_CLIENT, "setClientVolFog: halfwayDist must be greater than 0", 0);

    float halfwayHeight = Scr_GetFloat(2u, SCRIPTINSTANCE_CLIENT);
    if (halfwayHeight < 0.0)
        Scr_Error(SCRIPTINSTANCE_CLIENT, "setClientVolFog: halfwayHeight must be greater or equal to 0", 0);

    float baseHeight = Scr_GetFloat(3u, SCRIPTINSTANCE_CLIENT);
    float density = 1.0 / halfwayDist;

    float heightDensity;
    if (halfwayHeight < 1.0)
        heightDensity = 0.0;
    else
        heightDensity = 1.0 / halfwayHeight;

    float red = Scr_GetFloat(4u, SCRIPTINSTANCE_CLIENT);
    float green = Scr_GetFloat(5u, SCRIPTINSTANCE_CLIENT);
    float blue = Scr_GetFloat(6u, SCRIPTINSTANCE_CLIENT);
    int numParams = Scr_GetNumParam(SCRIPTINSTANCE_CLIENT);

    cg_clientVolFog.fogStart = startDist;
    cg_clientVolFog.color[0] = red;
    cg_clientVolFog.color[1] = green;
    cg_clientVolFog.color[2] = blue;
    cg_clientVolFog.density = density;
    cg_clientVolFog.heightDensity = heightDensity;
    cg_clientVolFog.baseHeight = baseHeight;

    if (numParams == 8)
    {
        cg_clientVolFog.color[3] = 1.0;
        cg_clientVolFog.sunFogColor[0] = 0.0;
        cg_clientVolFog.sunFogColor[1] = 0.0;
        cg_clientVolFog.sunFogColor[2] = 0.0;
        cg_clientVolFog.sunFogDir[0] = 0.0;
        cg_clientVolFog.sunFogDir[1] = 0.0;
        cg_clientVolFog.sunFogDir[2] = 0.0;
        cg_clientVolFog.sunFogStartAng = 0.0;
        cg_clientVolFog.sunFogEndAng = 0.0;
        cg_clientVolFog.sunFogColor[3] = 1.0;
    }
    else
    {
        cg_clientVolFog.color[3] = Scr_GetFloat(7u, SCRIPTINSTANCE_CLIENT);
        cg_clientVolFog.sunFogColor[0] = Scr_GetFloat(8u, SCRIPTINSTANCE_CLIENT);
        cg_clientVolFog.sunFogColor[1] = Scr_GetFloat(9u, SCRIPTINSTANCE_CLIENT);
        cg_clientVolFog.sunFogColor[2] = Scr_GetFloat(0xAu, SCRIPTINSTANCE_CLIENT);
        cg_clientVolFog.sunFogDir[0] = Scr_GetFloat(0xBu, SCRIPTINSTANCE_CLIENT);
        cg_clientVolFog.sunFogDir[1] = Scr_GetFloat(0xCu, SCRIPTINSTANCE_CLIENT);
        cg_clientVolFog.sunFogDir[2] = Scr_GetFloat(0xDu, SCRIPTINSTANCE_CLIENT);
        cg_clientVolFog.sunFogStartAng = Scr_GetFloat(0xEu, SCRIPTINSTANCE_CLIENT);
        cg_clientVolFog.sunFogEndAng = Scr_GetFloat(0xFu, SCRIPTINSTANCE_CLIENT);
        cg_clientVolFog.sunFogColor[3] = Scr_GetFloat(0x11u, SCRIPTINSTANCE_CLIENT);
    }
}

void CScr_SwitchToServerVolumetricFog()
{
    if (Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) != 1)
    {
        Scr_Error(
            SCRIPTINSTANCE_CLIENT,
            "Incorrect number of parameters\nUSAGE: SwitchToServerVolumetricFog(localClientNum)\n",
            0);
    }

    int clientIndex = Scr_GetInt(0, SCRIPTINSTANCE_CLIENT);
    if (clientIndex)
    {
        char* errorBuf = va("Trying to get a local client index for a client '%d' that is not a local client.", clientIndex);
        Scr_Error(SCRIPTINSTANCE_CLIENT, errorBuf, 0);
    }

    cg_s* cgameGlob = CG_GetLocalClientGlobals(clientIndex);
    R_SetFogFromServer(
        clientIndex,
        cg_serverVolFog.fogStart,
        cg_serverVolFog.color[0],
        cg_serverVolFog.color[1],
        cg_serverVolFog.color[2],
        cg_serverVolFog.density,
        cg_serverVolFog.heightDensity,
        cg_serverVolFog.baseHeight,
        cg_serverVolFog.color[3],
        cg_serverVolFog.sunFogColor[0],
        cg_serverVolFog.sunFogColor[1],
        cg_serverVolFog.sunFogColor[2],
        cg_serverVolFog.sunFogDir[0],
        cg_serverVolFog.sunFogDir[1],
        cg_serverVolFog.sunFogDir[2],
        cg_serverVolFog.sunFogStartAng,
        cg_serverVolFog.sunFogEndAng,
        cg_serverVolFog.sunFogColor[3]);

    R_SwitchFog(clientIndex, 1u, cgameGlob->time, 0);
}

void CScr_SwitchToClientVolumetricFog()
{
    if (Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) != 1)
    {
        Scr_Error(
            SCRIPTINSTANCE_CLIENT,
            "Incorrect number of parameters\nUSAGE: SwitchToClientVolumetricFog(localClientNum)\n",
            0);
    }
        
    int clientIndex = Scr_GetInt(0, SCRIPTINSTANCE_CLIENT);
    if (clientIndex)
    {
        char* errorBuf = va("Trying to get a local client index for a client '%d' that is not a local client.", clientIndex);
        Scr_Error(SCRIPTINSTANCE_CLIENT, errorBuf, 0);
    }

    cg_s* cgameGlob = CG_GetLocalClientGlobals(clientIndex);
    R_SetFogFromServer(
        clientIndex,
        cg_clientVolFog.fogStart,
        cg_clientVolFog.color[0],
        cg_clientVolFog.color[1],
        cg_clientVolFog.color[2],
        cg_clientVolFog.density,
        cg_clientVolFog.heightDensity,
        cg_clientVolFog.baseHeight,
        cg_clientVolFog.color[3],
        cg_clientVolFog.sunFogColor[0],
        cg_clientVolFog.sunFogColor[1],
        cg_clientVolFog.sunFogColor[2],
        cg_clientVolFog.sunFogDir[0],
        cg_clientVolFog.sunFogDir[1],
        cg_clientVolFog.sunFogDir[2],
        cg_clientVolFog.sunFogStartAng,
        cg_clientVolFog.sunFogEndAng,
        cg_clientVolFog.sunFogColor[3]);

    R_SwitchFog(clientIndex, 1u, cgameGlob->time, 0);
}

void CScr_IsInHelicopter()
{
    if (Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) != 1)
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "Incorrect number of parameters\nUSAGE: IsInHelicopter(localClientNum)\n", 0);
    }
    
    int clientIndex = Scr_GetInt(0, SCRIPTINSTANCE_CLIENT);
    if (clientIndex)
    {
        char* errorBuf = va("Trying to get a local client index for a client '%d' that is not a local client.", clientIndex);
        Scr_Error(SCRIPTINSTANCE_CLIENT, errorBuf, 0);
    }

    cg_s* cgameGlob = CG_GetLocalClientGlobals(clientIndex);
    
    int isInHelicopter = 0;
    if(cgameGlob->cameraData.lastCamMode == CAM_VEHICLE)
    {
        centity_s* vehicle = CG_GetEntity(clientIndex, cgameGlob->predictedPlayerState.viewlocked_entNum);
        if (vehicle)
        {
            isInHelicopter = CG_GetVehicleInfo(vehicle->nextState.vehicleState.vehicleInfoIndex)->type == 6;
        }
    }
    else
    {
        isInHelicopter = cgameGlob->cameraData.lastCamMode == CAM_VEHICLE_GUNNER;
    }

    Scr_AddInt(isInHelicopter, SCRIPTINSTANCE_CLIENT);
}

void CScr_GetGridFromPos()
{
    VariableUnion v0; // eax
    float pos[3]; // [esp+8h] [ebp-14h] BYREF
    char gridName[4]; // [esp+14h] [ebp-8h] BYREF
    int argc; // [esp+18h] [ebp-4h]

    argc = Scr_GetNumParam(SCRIPTINSTANCE_CLIENT);
    if ( argc != 2 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "CGScr_GetGridByPos( <clientNum> <pos> ) takes 2 parameters", 0);
    Scr_GetVector(1u, pos, SCRIPTINSTANCE_CLIENT);
    v0.intValue = CScr_GetLocalClientNum(0);
    CG_GetGridFromPos(v0.intValue, pos, gridName);
    Scr_AddString(gridName, SCRIPTINSTANCE_CLIENT);
}

void CScr_CompassScale()
{
    VariableUnion duration; // [esp+0h] [ebp-Ch]
    VariableUnion difference; // [esp+4h] [ebp-8h]

    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) != 2 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "CScr_CompassScale( <diff> <dur> ) takes 2 parameters", 0);
    difference.intValue = Scr_GetInt(0, SCRIPTINSTANCE_CLIENT);
    duration.intValue = Scr_GetInt(1u, SCRIPTINSTANCE_CLIENT);
    Dvar_SetInt((dvar_s *)compassScaleDiff, difference.intValue);
    Dvar_SetInt((dvar_s *)compassScaleDuration, duration.intValue);
}

void CScr_ResetCompassScale()
{
    VariableUnion duration; // [esp+0h] [ebp-8h]

    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) != 1 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "CScr_ResetCompassScale( <dur> ) takes 1 parameter", 0);
    duration.intValue = Scr_GetInt(0, SCRIPTINSTANCE_CLIENT);
    Dvar_SetBool((dvar_s *)compassScaleReset, 1);
    Dvar_SetInt((dvar_s *)compassScaleDuration, duration.intValue);
}

void __cdecl CScr_GetLocalPlayerTeam()
{
    cg_s *cGameGlob; // [esp+8h] [ebp-Ch]
    int localClientNum; // [esp+10h] [ebp-4h]

    localClientNum = CScr_GetLocalClientNum(0);
    if ( CL_LocalClient_IsActive(localClientNum) )
    {
        cGameGlob = CG_GetLocalClientGlobals(localClientNum);
        CScr_AddTeamName(cGameGlob->bgs.clientinfo[cGameGlob->clientNum].team);
    }
    else
    {
        Scr_AddUndefined(SCRIPTINSTANCE_CLIENT);
    }
}

void __cdecl CScr_AddTeamName(team_t team)
{
    switch ( team )
    {
        case TEAM_FREE:
            Scr_AddString("free", SCRIPTINSTANCE_CLIENT);
            break;
        case TEAM_AXIS:
            Scr_AddString("axis", SCRIPTINSTANCE_CLIENT);
            break;
        case TEAM_ALLIES:
            Scr_AddString("allies", SCRIPTINSTANCE_CLIENT);
            break;
        case TEAM_SPECTATOR:
            Scr_AddString("spectator", SCRIPTINSTANCE_CLIENT);
            break;
        default:
            return;
    }
}

void(__cdecl *__cdecl CScr_GetFunctionProjectSpecific(const char **pName, int *type))()
{
    unsigned int i; // [esp+18h] [ebp-4h]

    for (i = 0; i < ARRAY_COUNT(client_project_functions); ++i)
    {
        if (!strcmp(*pName, client_project_functions[i].actionString))
        {
            *pName = client_project_functions[i].actionString;
            *type = client_project_functions[i].type;
            return client_project_functions[i].actionFunc;
        }
    }
    return 0;
}

void __cdecl CScrCmd_GetOwner(scr_entref_t entref)
{
    centity_s *Entity; // eax
    int intValue; // [esp-4h] [ebp-30h]
    centity_s *pSelf; // [esp+1Ch] [ebp-10h]

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scr_main_mp.cpp",
                        1227,
                        0,
                        "%s",
                        "entref.entnum < MAX_LOCAL_CENTITIES") )
        {
            __debugbreak();
        }
        if ( CG_GetClientNumForLocalClient(entref.client) == entref.entnum )
            pSelf = &CG_GetLocalClientGlobals(entref.client)->predictedPlayerEntity;
        else
            pSelf = CG_GetEntity(entref.client, entref.entnum);
        if ( entref.entnum >= 0x400u )
            CG_GetFakeEntity(entref.client, entref.entnum);
    }
    intValue = CScr_GetLocalClientNum(0);
    Entity = CG_GetEntity(intValue, (int)pSelf->nextState.faction.iHeadIconTeam >> 2);
    CScr_AddEntity(Entity, intValue);
}

void __cdecl CScr_GetTagOrigin(scr_entref_t entref)
{
    VariableUnion v1; // eax
    unsigned int tagName; // [esp+Ch] [ebp-10h]
    centity_s *pSelf; // [esp+10h] [ebp-Ch]
    char *name; // [esp+14h] [ebp-8h]

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scr_main_mp.cpp",
                        1288,
                        0,
                        "%s",
                        "entref.entnum < MAX_LOCAL_CENTITIES") )
        {
            __debugbreak();
        }
        if ( CG_GetClientNumForLocalClient(entref.client) == entref.entnum )
            pSelf = &CG_GetLocalClientGlobals(entref.client)->predictedPlayerEntity;
        else
            pSelf = CG_GetEntity(entref.client, entref.entnum);
        if ( entref.entnum >= 0x400u )
            CG_GetFakeEntity(entref.client, entref.entnum);
    }
    v1.intValue = Scr_GetConstLowercaseString(0, SCRIPTINSTANCE_CLIENT);
    name = SL_ConvertToString(v1.stringValue, SCRIPTINSTANCE_CLIENT);
    tagName = SL_FindLowercaseString(name, SCRIPTINSTANCE_SERVER);
    CScr_UpdateTagInternal(pSelf, tagName, &cg_cachedTagMat);
    Scr_AddVector(cg_cachedTagMat.tagMat[3], SCRIPTINSTANCE_CLIENT);
}

int __cdecl CScr_UpdateTagInternal(centity_s *ent, unsigned int tagName, cached_tag_mat_t *cachedTag)
{
    char *v4; // eax
    cg_s *LocalClientGlobals; // eax
    const char *name; // [esp-4h] [ebp-18h]
    char *error; // [esp+0h] [ebp-14h]
    DObj *obj; // [esp+Ch] [ebp-8h]
    cg_s *cgameGlob; // [esp+10h] [ebp-4h]

    if ( !ent
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scr_main_mp.cpp", 1236, 0, "%s", "ent") )
    {
        __debugbreak();
    }
    cgameGlob = CG_GetLocalClientGlobals(ent->pose.localClientNum);
    if ( ent->nextState.number != cachedTag->entnum || cgameGlob->time != cachedTag->time || tagName != cachedTag->name )
    {
        obj = Com_GetClientDObj(ent->nextState.number, ent->pose.localClientNum);
        if ( !obj )
        {
            Scr_ObjectError("entity has no model defined", SCRIPTINSTANCE_SERVER);
            return 0;
        }
        if ( !CG_DObjGetWorldTagMatrix(&ent->pose, obj, tagName, cachedTag->tagMat, cachedTag->tagMat[3]) )
        {
            name = DObjGetModel(obj, 0)->name;
            v4 = SL_ConvertToString(tagName, SCRIPTINSTANCE_CLIENT);
            error = va("tag '%s' does not exist in model '%s' (or any attached submodels)", v4, name);
            Scr_Error(SCRIPTINSTANCE_CLIENT, error, 0);
            return 0;
        }
        LocalClientGlobals = CG_GetLocalClientGlobals(ent->pose.localClientNum);
        cachedTag->entnum = ent->nextState.number;
        cachedTag->time = LocalClientGlobals->time;
        cachedTag->name = tagName;
    }
    return 1;
}

void __cdecl CScr_GetTagAngles(scr_entref_t entref)
{
    VariableUnion v1; // eax
    unsigned int tagName; // [esp+Ch] [ebp-1Ch]
    centity_s *pSelf; // [esp+10h] [ebp-18h]
    char *name; // [esp+14h] [ebp-14h]
    float angles[3]; // [esp+18h] [ebp-10h] BYREF
    fake_centity_s *pFake; // [esp+24h] [ebp-4h]

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
        pSelf = 0;
        pFake = 0;
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scr_main_mp.cpp",
                        1314,
                        0,
                        "%s",
                        "entref.entnum < MAX_LOCAL_CENTITIES") )
        {
            __debugbreak();
        }
        if ( CG_GetClientNumForLocalClient(entref.client) == entref.entnum )
            pSelf = &CG_GetLocalClientGlobals(entref.client)->predictedPlayerEntity;
        else
            pSelf = CG_GetEntity(entref.client, entref.entnum);
        if ( entref.entnum < 0x400u )
            pFake = 0;
        else
            pFake = CG_GetFakeEntity(entref.client, entref.entnum);
    }
    v1.intValue = Scr_GetConstLowercaseString(0, SCRIPTINSTANCE_CLIENT);
    name = SL_ConvertToString(v1.stringValue, SCRIPTINSTANCE_CLIENT);
    tagName = SL_FindLowercaseString(name, SCRIPTINSTANCE_SERVER);
    CScr_UpdateTagInternal(pSelf, tagName, &cg_cachedTagMat);
    AxisToAngles(cg_cachedTagMat.tagMat, angles);
    Scr_AddVector(angles, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScrCmd_ShellShock(scr_entref_t entref)
{
    shellshock_parms_t *ShellshockParms; // eax
    int time; // [esp+0h] [ebp-50h]
    char *v3; // [esp+8h] [ebp-48h]
    char *error; // [esp+Ch] [ebp-44h]
    float Float; // [esp+20h] [ebp-30h]
    int duration; // [esp+34h] [ebp-1Ch]
    char *shock; // [esp+38h] [ebp-18h]
    cg_s *cgameGlob; // [esp+3Ch] [ebp-14h]
    int localClientNum; // [esp+44h] [ebp-Ch]
    char *configString; // [esp+48h] [ebp-8h]
    signed int id; // [esp+4Ch] [ebp-4h]

    CG_GetEntity(entref.client, entref.entnum);
    if ( (unsigned int)Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) < 3 )
        Scr_Error(
            SCRIPTINSTANCE_CLIENT,
            "USAGE: <player> shellshock( <local client number>, <shellshockname>, <duration>)\n",
            0);
    localClientNum = CScr_GetLocalClientNum(0);
    shock = Scr_GetString(1u, SCRIPTINSTANCE_CLIENT);
    Float = Scr_GetFloat(2u, SCRIPTINSTANCE_CLIENT);
    duration = (int)((float)(1000.0 * Float) + 9.313225746154785e-10);
    if ( (unsigned int)duration > 0xEA60 )
    {
        error = va("duration %g should be >= 0 and <= 60", (float)((float)duration * 0.001));
        Scr_ParamError(1u, error, SCRIPTINSTANCE_CLIENT);
    }
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    for ( id = 1; id < 16; ++id )
    {
        configString = CL_GetConfigString(id + 2532);
        if ( !I_stricmp(configString, shock) )
        {
            time = cgameGlob->time;
            ShellshockParms = BG_GetShellshockParms(id);
            CG_StartShellShock(cgameGlob, ShellshockParms, time, duration);
            return;
        }
    }
    v3 = va(
                 "shellshock '%s' was not precached.    You need to precache it in the server script before calling it on the client.\n",
                 shock);
    Scr_Error(SCRIPTINSTANCE_CLIENT, v3, 0);
}

void __cdecl CScr_SetEnemyGlobalScrambler(scr_entref_t entref)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-8h]

    cgameGlob = CG_GetLocalClientGlobals(entref.client);
    cgameGlob->globalScramblerActive = Scr_GetInt(0, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_SetEnemyScramblerAmount(scr_entref_t entref)
{
    float alphaAmount; // [esp+0h] [ebp-8h]
    cg_s *cgameGlob; // [esp+4h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(entref.client);
    alphaAmount = Scr_GetFloat(0, SCRIPTINSTANCE_CLIENT);
    if ( alphaAmount < 0.0 || alphaAmount > 1.0 )
        Scr_Error("float value must be between 0 and 1", 0);
    cgameGlob->scramblerEnemyAlpha = alphaAmount;
}

void __cdecl CScr_SetFriendlyScramblerAmount(scr_entref_t entref)
{
    float alphaAmount; // [esp+0h] [ebp-8h]
    cg_s *cgameGlob; // [esp+4h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(entref.client);
    alphaAmount = Scr_GetFloat(0, SCRIPTINSTANCE_CLIENT);
    if ( alphaAmount < 0.0 || alphaAmount > 1.0 )
        Scr_Error("float value must be between 0 and 1", 0);
    cgameGlob->scramblerFriendlyAlpha = alphaAmount;
}

void __cdecl CScr_GetFriendlyScramblerAmount(scr_entref_t entref)
{
    float value; // [esp+8h] [ebp-8h]

    value = CG_GetLocalClientGlobals(entref.client)->scramblerFriendlyAlpha;
    Scr_AddFloat(value, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_GetEnemyScramblerAmount(scr_entref_t entref)
{
    float value; // [esp+8h] [ebp-8h]

    value = CG_GetLocalClientGlobals(entref.client)->scramblerEnemyAlpha;
    Scr_AddFloat(value, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_IsScrambled(scr_entref_t entref)
{
    unsigned int IsScrambled; // [esp+4h] [ebp-4h]

    IsScrambled = CG_GetLocalClientGlobals(entref.client)->scramblerEnemyAlpha > 0.01;
    Scr_AddBool(IsScrambled, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_SetNearestEnemyScrambler(scr_entref_t entref)
{
    scr_entref_t v1; // [esp+8h] [ebp-2Ch] BYREF
    scr_entref_t v2; // [esp+12h] [ebp-22h]
    scr_entref_t v3; // [esp+18h] [ebp-1Ch]
    scr_entref_t v4; // [esp+22h] [ebp-12h]
    centity_s *cent; // [esp+28h] [ebp-Ch]
    scr_entref_t scramblerEntity; // [esp+2Ch] [ebp-8h]

    //v2 = *Scr_GetEntityRef(&v1, 0, SCRIPTINSTANCE_CLIENT);
    v2 = Scr_GetEntityRef(0, SCRIPTINSTANCE_CLIENT);
    v3 = v2;
    v4 = v2;
    scramblerEntity = v2;
    cent = CG_GetEntity(v2.client, v2.entnum);
    CG_AddEnemyScrambler(entref.client, cent);
}

void __cdecl CScr_ClearNearestEnemyScrambler(scr_entref_t entref)
{
    CG_ClearNearestEnemyScrambler(entref.client);
}

void __cdecl CScr_AddFriendlyScrambler(scr_entref_t entref)
{
    VariableUnion scramblerHandle; // [esp+Ch] [ebp-Ch]
    float scramblerY; // [esp+10h] [ebp-8h]
    float scramblerX; // [esp+14h] [ebp-4h]

    scramblerX = Scr_GetFloat(0, SCRIPTINSTANCE_CLIENT);
    scramblerY = Scr_GetFloat(1u, SCRIPTINSTANCE_CLIENT);
    scramblerHandle.intValue = Scr_GetInt(2u, SCRIPTINSTANCE_CLIENT);
    CG_AddFriendlyScrambler(entref.client, scramblerX, scramblerY, scramblerHandle.intValue);
}

void __cdecl CScr_RemoveFriendlyScrambler(scr_entref_t entref)
{
    VariableUnion scramblerHandle; // [esp+0h] [ebp-4h]

    scramblerHandle.intValue = Scr_GetInt(0, SCRIPTINSTANCE_CLIENT);
    CG_RemoveFriendlyScrambler(entref.client, scramblerHandle.intValue);
}

void __cdecl CScr_RemoveAllFriendlyScramblers(scr_entref_t entref)
{
    CG_RemoveAllFriendlyScramblers(entref.client);
}

void __cdecl CScr_HasTacticalMaskOverlay(scr_entref_t entref)
{
    if ( CG_IsShowingZombieMap() )
        Scr_AddInt(1, SCRIPTINSTANCE_CLIENT);
    else
        Scr_AddInt(0, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_GetStance(scr_entref_t entref)
{
    centity_s *pSelf; // [esp+Ch] [ebp-8h]

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scr_main_mp.cpp",
                        1663,
                        0,
                        "%s",
                        "entref.entnum < MAX_LOCAL_CENTITIES") )
        {
            __debugbreak();
        }
        if ( CG_GetClientNumForLocalClient(entref.client) == entref.entnum )
            pSelf = &CG_GetLocalClientGlobals(entref.client)->predictedPlayerEntity;
        else
            pSelf = CG_GetEntity(entref.client, entref.entnum);
        if ( entref.entnum >= 0x400u )
            CG_GetFakeEntity(entref.client, entref.entnum);
    }
    if ( ((*((unsigned int *)pSelf + 201) >> 1) & 1) == 0 )
        Scr_Error("GetStance can not be used on a player that is not in the snapshot.", 0);
    if ( pSelf->nextState.eType == 1 || pSelf->nextState.eType == 5 )
    {
        if ( (pSelf->nextState.lerp.eFlags & 8) != 0 )
        {
            Scr_AddConstString(cscr_const.prone, SCRIPTINSTANCE_SERVER);
        }
        else if ( (pSelf->nextState.lerp.eFlags & 4) != 0 )
        {
            Scr_AddConstString(cscr_const.crouch, SCRIPTINSTANCE_SERVER);
        }
        else
        {
            Scr_AddConstString(cscr_const.stand, SCRIPTINSTANCE_SERVER);
        }
    }
    else
    {
        Scr_Error("GetStance is only defined for players.", 0);
    }
}

void __cdecl CScr_SetFlagAsAway(scr_entref_t entref)
{
    centity_s *pSelf; // [esp+14h] [ebp-18h]
    team_t team; // [esp+18h] [ebp-14h]
    cg_s *cgameGlob; // [esp+1Ch] [ebp-10h]
    VariableUnion localClientNum; // [esp+20h] [ebp-Ch]
    int away; // [esp+28h] [ebp-4h]

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scr_main_mp.cpp",
                        1702,
                        0,
                        "%s",
                        "entref.entnum < MAX_LOCAL_CENTITIES") )
        {
            __debugbreak();
        }
        if ( CG_GetClientNumForLocalClient(entref.client) == entref.entnum )
            pSelf = &CG_GetLocalClientGlobals(entref.client)->predictedPlayerEntity;
        else
            pSelf = CG_GetEntity(entref.client, entref.entnum);
        if ( entref.entnum >= 0x400u )
            CG_GetFakeEntity(entref.client, entref.entnum);
    }
    if ( ((*((unsigned int *)pSelf + 201) >> 1) & 1) != 0 )
    {
        if ( (unsigned int)Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) >= 2 )
        {
            localClientNum.intValue = CScr_GetLocalClientNum(0);
            away = Scr_GetInt(1u, SCRIPTINSTANCE_CLIENT);
            cgameGlob = CG_GetLocalClientGlobals(localClientNum.intValue);
            team = (team_t)(pSelf->nextState.faction.iHeadIconTeam & 3);
            if ( team == TEAM_ALLIES )
            {
                cgameGlob->alliesFlagAway = away != 0;
            }
            else if ( team == TEAM_AXIS )
            {
                cgameGlob->axisFlagAway = away != 0;
            }
        }
        else
        {
            Scr_Error(SCRIPTINSTANCE_CLIENT, "Not enough parameters supplied to SetFlagAsAway", 0);
        }
    }
    else
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "SetFlagAsAway must be called on a valid entity", 0);
    }
}

void __cdecl CScr_GetParentEntity(scr_entref_t entref)
{
    centity_s *cent; // [esp+8h] [ebp-8h]
    centity_s *parent; // [esp+Ch] [ebp-4h]

    cent = CG_GetEntity(entref.client, entref.entnum);
    parent = CG_EntGetLinkToParent(entref.client, cent);
    if ( parent )
        CScr_AddEntity(parent, entref.client);
    else
        Scr_AddUndefined(SCRIPTINSTANCE_CLIENT);
}

void (__cdecl *__cdecl CScr_GetMethodProjectSpecific(const char **pName, int *type))(scr_entref_t)
{
    unsigned int i; // [esp+18h] [ebp-4h]

    for ( i = 0; i < 0x1D; ++i )
    {
        if ( !strcmp(*pName, client_project_methods[i].actionString) )
        {
            *pName = client_project_methods[i].actionString;
            *type = client_project_methods[i].type;
            return client_project_methods[i].actionFunc;
        }
    }
    return 0;
}

void __cdecl CScr_SetUniqueClientScripts(ScriptFunctions *functions)
{
    cg_scr_data.delete_ = CScr_SetScriptAndLabel(functions, "codescripts/delete", "main", 1);
    cg_scr_data.initstructs = CScr_SetScriptAndLabel(functions, "codescripts/struct", "initstructs", 1);
    cg_scr_data.createstruct = CScr_SetScriptAndLabel(functions, "codescripts/struct", "createstruct", 1);
    cg_scr_data.findstruct = CScr_SetScriptAndLabel(functions, "codescripts/struct", "findstruct", 1);
    cg_scr_data.clientsysstatechange = CScr_SetScriptAndLabel(functions, "clientscripts/mp/_callbacks", "statechange", 1);
    cg_scr_data.maprestart = CScr_SetScriptAndLabel(functions, "clientscripts/mp/_callbacks", "maprestart", 1);
    cg_scr_data.localclientconnect = CScr_SetScriptAndLabel(
                                                                         functions,
                                                                         "clientscripts/mp/_callbacks",
                                                                         "localclientconnect",
                                                                         1);
    cg_scr_data.localclientdisconnect = CScr_SetScriptAndLabel(
                                                                                functions,
                                                                                "clientscripts/mp/_callbacks",
                                                                                "localclientdisconnect",
                                                                                1);
    cg_scr_data.entityspawned = CScr_SetScriptAndLabel(functions, "clientscripts/mp/_callbacks", "entityspawned", 1);
    cg_scr_data.playerspawned = CScr_SetScriptAndLabel(functions, "clientscripts/mp/_callbacks", "playerspawned", 1);
    cg_scr_data.clientFlagCB = CScr_SetScriptAndLabel(functions, "clientscripts/mp/_callbacks", "client_flag_callback", 1);
    cg_scr_data.clientFlagAsValCB = CScr_SetScriptAndLabel(
                                                                        functions,
                                                                        "clientscripts/mp/_callbacks",
                                                                        "client_flagasval_callback",
                                                                        1);
    cg_scr_data.destructible_callback = CScr_SetScriptAndLabel(
                                                                                functions,
                                                                                "clientscripts/_destructible",
                                                                                "CodeCallback_DestructibleEvent",
                                                                                0);
    cg_scr_data.corpse_callback = CScr_SetScriptAndLabel(
                                                                    functions,
                                                                    "clientscripts/mp/_callbacks",
                                                                    "CodeCallback_CreatingCorpse",
                                                                    1);
    cg_scr_data.playerFootstep = CScr_SetScriptAndLabel(
                                                                 functions,
                                                                 "clientscripts/mp/_callbacks",
                                                                 "CodeCallback_PlayerFootstep",
                                                                 1);
    cg_scr_data.playerJump = CScr_SetScriptAndLabel(
                                                         functions,
                                                         "clientscripts/mp/_callbacks",
                                                         "CodeCallback_PlayerJump",
                                                         1);
    cg_scr_data.playerLand = CScr_SetScriptAndLabel(
                                                         functions,
                                                         "clientscripts/mp/_callbacks",
                                                         "CodeCallback_PlayerLand",
                                                         1);
    cg_scr_data.playerFoliage = CScr_SetScriptAndLabel(
                                                                functions,
                                                                "clientscripts/mp/_callbacks",
                                                                "CodeCallback_PlayerFoliage",
                                                                1);
    cg_scr_mp_data.dogstep = CScr_SetScriptAndLabel(functions, "clientscripts/mp/_dogs", "playDogstep", 1);
    cg_scr_data.levelnotify = CScr_SetScriptAndLabel(functions, "clientscripts/mp/_callbacks", "level_notify", 1);
    cg_scr_data.dogSoundNotify = CScr_SetScriptAndLabel(functions, "clientscripts/mp/_dogs", "soundNotify", 1);
    cg_scr_mp_data.airsupport = CScr_SetScriptAndLabel(functions, "clientscripts/mp/_callbacks", "airsupport", 1);
    cg_scr_mp_data.demo_jump = CScr_SetScriptAndLabel(functions, "clientscripts/mp/_callbacks", "demo_jump", 1);
    cg_scr_mp_data.demo_playerSwitch = CScr_SetScriptAndLabel(
                                                                             functions,
                                                                             "clientscripts/mp/_callbacks",
                                                                             "demo_player_switch",
                                                                             1);
    cg_scr_data.entityshutdownCB = CScr_SetScriptAndLabel(
                                                                     functions,
                                                                     "clientscripts/mp/_callbacks",
                                                                     "entityshutdown_callback",
                                                                     1);
    cg_scr_data.glassSmash = CScr_SetScriptAndLabel(functions, "clientscripts/mp/_callbacks", "glass_smash", 0);
}

void __cdecl CG_SendSwimNotify(int localClientNum, unsigned int clientNum, int start)
{
    unsigned __int16 swimming_begin; // [esp+2h] [ebp-2h]

    if ( start )
        swimming_begin = cscr_const.swimming_begin;
    else
        swimming_begin = cscr_const.swimming_end;
    CScr_NotifyNum(localClientNum, clientNum, 0, swimming_begin, 0);
}

void __cdecl CScr_GetEntityByIndex(centity_s *cent, const cent_field_s *pField)
{
    centity_s *Entity; // eax
    unsigned int index; // [esp+10h] [ebp-4h]

    if ( !cent
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scr_main_mp.cpp", 1867, 0, "%s", "cent") )
    {
        __debugbreak();
    }
    if ( !pField
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scr_main_mp.cpp", 1868, 0, "%s", "pField") )
    {
        __debugbreak();
    }
    index = GetField((const int *)((char *)cent + pField->ofs), pField->size[0]);
    if ( index < 0x3FE )
    {
        Entity = CG_GetEntity(cent->pose.localClientNum, index);
        CScr_AddEntity(Entity, cent->pose.localClientNum);
    }
}

int __cdecl GetField(const int *i, int size)
{
    switch ( size )
    {
        case 1:
            return *(char *)i;
        case 2:
            return *(__int16 *)i;
        case 4:
            return *i;
    }
    if ( !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scr_main_mp.cpp",
                    1861,
                    0,
                    "unknown field size") )
        __debugbreak();
    return 0;
}

void __cdecl CScr_GetTeamName(centity_s *cent, const cent_field_s *pField)
{
    team_t team; // [esp+0h] [ebp-4h]

    if ( !cent
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scr_main_mp.cpp", 1902, 0, "%s", "cent") )
    {
        __debugbreak();
    }
    if ( !pField
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scr_main_mp.cpp", 1903, 0, "%s", "pField") )
    {
        __debugbreak();
    }
    team = GetTeam(cent);
    CScr_AddTeamName(team);
}

team_t __cdecl GetTeam(centity_s *cent)
{
    cg_s *cgameGlob; // [esp+0h] [ebp-8h]
    unsigned int localClientNum; // [esp+4h] [ebp-4h]

    if ( !cent
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scr_main_mp.cpp", 1882, 0, "%s", "cent") )
    {
        __debugbreak();
    }
    if ( cent->nextState.eType != 1 )
        return (team_t)(cent->nextState.faction.iHeadIconTeam & 3);
    localClientNum = RETURN_ZERO32();
    if ( localClientNum >= 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scr_main_mp.cpp",
                    1888,
                    0,
                    "localClientNum not in [0, MAX_LOCAL_CLIENTS]\n\t%i not in [%i, %i]",
                    localClientNum,
                    0,
                    1) )
    {
        __debugbreak();
    }
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->bgs.clientinfo[cent->nextState.clientNum].infoValid )
        return cgameGlob->bgs.clientinfo[cent->nextState.clientNum].team;
    else
        return TEAM_BAD;
}

unsigned __int16 __cdecl CScr_GetFootTag(eFoot foot)
{
    if ( (unsigned int)foot >= FOOTSTEP_COUNT
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_scr_main_mp.cpp",
                    1923,
                    0,
                    "foot doesn't index FOOTSTEP_COUNT\n\t%i not in [0, %i)",
                    foot,
                    4) )
    {
        __debugbreak();
    }
    return *footTags[foot];
}

float footprintGroundTraceUp = 15.0f;
float footprintGroundTraceDown = 15.0f;

void __cdecl CScr_PlayDogstepSound(int localClientNum, centity_s *cent, eFoot foot)
{
    unsigned __int16 FootTag; // ax
    char *value; // [esp+0h] [ebp-68h]
    unsigned __int16 t; // [esp+14h] [ebp-54h]
    DObj *obj; // [esp+18h] [ebp-50h]
    float start[3]; // [esp+1Ch] [ebp-4Ch] BYREF
    float end[3]; // [esp+28h] [ebp-40h] BYREF
    int surfType; // [esp+34h] [ebp-34h]
    float footMatrix[4][3]; // [esp+38h] [ebp-30h] BYREF

    if ( cg_scr_mp_data.dogstep )
    {
        obj = Com_GetClientDObj(cent->nextState.number, localClientNum);
        if ( !obj
            || (FootTag = CScr_GetFootTag(foot),
                    !CG_DObjGetWorldTagMatrix(&cent->pose, obj, FootTag, footMatrix, footMatrix[3])) )
        {
            footMatrix[3][0] = cent->pose.origin[0];
            footMatrix[3][1] = cent->pose.origin[1];
            footMatrix[3][2] = cent->pose.origin[2];
        }
        *(_QWORD *)start = *(_QWORD *)&footMatrix[3][0];
        start[2] = footMatrix[3][2] + footprintGroundTraceUp;
        *(_QWORD *)end = *(_QWORD *)&footMatrix[3][0];
        end[2] = footMatrix[3][2] - footprintGroundTraceDown;
        surfType = (CM_TracePointDown(start, end, 2065, 0x3F00000, footMatrix[3], 0, 0) & 0x3F00000) >> 20;
        if ( (cent->currentState.eFlags2 & 0x200000) != 0 || ((*((unsigned int *)cent + 201) >> 5) & 1) != 0 )
            Scr_AddInt(1, SCRIPTINSTANCE_CLIENT);
        else
            Scr_AddInt(0, SCRIPTINSTANCE_CLIENT);
        value = (char *)Com_SurfaceTypeToName(surfType);
        Scr_AddString(value, SCRIPTINSTANCE_CLIENT);
        Scr_AddVector(footMatrix[3], SCRIPTINSTANCE_CLIENT);
        CScr_AddEntity(cent, localClientNum);
        Scr_AddInt(localClientNum, SCRIPTINSTANCE_CLIENT);
        t = Scr_ExecThread(SCRIPTINSTANCE_CLIENT, cg_scr_mp_data.dogstep, 5u);
        Scr_FreeThread(t, SCRIPTINSTANCE_CLIENT);
    }
}

