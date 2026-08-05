#include "cg_scr_main.h"
#include <clientscript/cscr_stringlist.h>
#include <clientscript/scr_const.h>
#include <cgame_mp/cg_scr_main_mp.h>
#include <clientscript/cscr_vm.h>
#include <bgame/bg_weapons.h>
#include <cgame_mp/cg_ents_mp.h>
#include <cgame_mp/cg_actors_mp.h>
#include <xanim/xmodel.h>
#include <cgame_mp/cg_vehicles_mp.h>
#include <bgame/bg_misctables.h>
#include <universal/surfaceflags.h>
#include <cgame_mp/cg_local_mp.h>
#include <cgame_mp/cg_main_mp.h>
#include "cg_spawn.h"
#include <clientscript/cscr_evaluate.h>
#include <client/cl_debugdata.h>
#include <universal/com_math_anglevectors.h>
#include <client_mp/cl_cgame_mp.h>
#include <ui/ui_shared.h>
#include <universal/com_shared.h>
#include <sound/snd_bank.h>
#include "cg_sound.h"
#include <sound/snd_public_async.h>
#include <sound/snd_utils.h>
#include <win32/win_shared.h>
#include <client/splitscreen.h>
#include <qcommon/dobj_management.h>
#include <xanim/dobj_utils.h>
#include <demo/demo_playback.h>
#include <qcommon/com_clients.h>
#include "cg_compass.h"
#include <gfx_d3d/r_cinematic.h>
#include <EffectsCore/fx_system.h>
#include <EffectsCore/fx_dvars.h>
#include "cg_world.h"
#include <universal/com_files.h>
#include <universal/q_parse.h>
#include <universal/com_memory.h>
#include <gfx_d3d/r_ui3d.h>
#include "cg_main.h"
#include <bgame/bg_weapons_def.h>
#include <xanim/xmodel_utils.h>
#include <bgame/bg_misc.h>
#include <gfx_d3d/r_primarylights.h>
#include "cg_camerashake.h"
#include <DynEntity/DynEntity_client.h>
#include <clientscript/cscr_memorytree.h>
#include <cgame_mp/cg_animscripted_mp.h>
#include "cg_event.h"
#include <game/g_load_utils.h>
#include <stringed/stringed_hooks.h>

cg_level_locals_t cg_level;
cscr_data_t cg_scr_data;

int g_cs_file;
char *g_cs_file_buffer;
com_parse_mark_t g_cs_file_linemark;


void NULLSUB()
{

}

void NULLSUB2(scr_entref_t ref)
{

}

const BuiltinMethodDef client_methods[] =
{
  { "print", CScr_Print, 1 },
  { "delete", CScrCmd_Delete, 0 },
  { "forcedelete", CScrCmd_ForceDelete, 0 },
  { "earthquake", CScrCmd_Earthquake, 0 },
  { "playsound", CScr_PlaySoundOnEntity, 0 },
  { "playloopsound", CScr_PlayLoopSoundOnEntity, 0 },
  { "stoploopsound", CScr_StopLoopSoundOnEntity, 0 },
  { "isplayingloopsound", CScr_IsPlayingLoopSound, 0 },
  { "makelight", CScrCmd_MakeLight, 0 },
  { "getlightcolor", CScr_GetLightColor, 0 },
  { "setlightcolor", CScr_SetLightColor, 0 },
  { "getlightintensity", CScr_GetLightIntensity, 0 },
  { "setlightintensity", CScr_SetLightIntensity, 0 },
  { "getlightradius", CScr_GetLightRadius, 0 },
  { "setlightradius", CScr_SetLightRadius, 0 },
  { "getlightfovinner", CScr_GetLightFovInner, 0 },
  { "getlightfovouter", CScr_GetLightFovOuter, 0 },
  { "setlightfovrange", CScr_SetLightFovRange, 0 },
  { "getlightexponent", CScr_GetLightExponent, 0 },
  { "setlightexponent", CScr_SetLightExponent, 0 },
  // LWSS ADD FROM BLOPS MP (LATEST)
  { "getstance", CScrCmd_GetStance, 0 },
  // LWSS END
  { "getentnum", CScrCmd_GetEntNum, 1 },
  { "getentitynumber", CScrCmd_GetEntityNumber, 0 },
  { "setmodel", CScrCmd_SetModel, 0 },
  { "attach", CScr_Attach, 0 },
  { "linkto", CScrCmd_LinkTo, 0 },
  { "unlink", CScrCmd_Unlink, 0 },
  { "istouching", CScrCmd_IsTouching, 0 },
  { "isalive", CScrCmd_IsAlive, 0 },
  { "getspeed", CScrCmd_GetSpeed, 0 },
  { "underwater", CScrCmd_UnderWater, 0 },
  { "isplayer", CScrCmd_IsPlayer, 0 },
  { "islocalplayer", CScrCmd_IsLocalPlayer, 0 },
  { "isai", CScrCmd_IsAI, 0 },
  { "hasdobj", CScr_HasDObj, 0 },
  { "setcompassicon", CScr_SetCompassIcon, 0 },
  { "launchragdoll", CScr_LaunchRagdoll, 0 },
  { "show", CScr_Show, 0 },
  { "hide", CScr_Hide, 0 },
  { "getthrottle", CScrCmd_GetThrottle, 0 },
  { "getbrake", CScrCmd_GetBrake, 0 },
  { "getmaxspeed", CScrCmd_GetMaxSpeed, 0 },
  { "getmaxreversespeed", CScrCmd_GetMaxReverseSpeed, 0 },
  { "islocalclientdriver", CScrCmd_IsLocalClientDriver, 0 },
  { "getlocalclientdriver", CScrCmd_GetLocalClientDriver, 0 },
  { "getwheelsurface", CScrCmd_GetWheelSurface, 0 },
  { "ispeelingout", CScrCmd_IsVehiclePeelingOut, 0 },
  { "iswheelsliding", CScrCmd_IsWheelSliding, 0 },
  { "iswheelcolliding", CScrCmd_IsWheelColliding, 0 },
  { "getlocalgunnerangles", CScrCmd_GetLocalGunnerAngles, 0 },
  { "setstunned", CScrCmd_SetStunned, 0 },
  { "isdriving", CScrCmd_IsDriving, 0 },
  { "rotatepitch", CScrCmd_RotatePitch, 0 },
  { "rotateyaw", CScrCmd_RotateYaw, 0 },
  { "rotateroll", CScrCmd_RotateRoll, 0 },
  { "rotateto", CScrCmd_RotateTo, 0 },
  { "moveto", CScrCmd_MoveTo, 0 },
  { "movegravity", CScrCmd_MoveGravity, 0 },
  { "movex", CScrEntCmd_MoveX, 0 },
  { "movey", CScrEntCmd_MoveY, 0 },
  { "movez", CScrEntCmd_MoveZ, 0 },
  { "rotatevelocity", CScrCmd_RotateVelocity, 0 },
  { "playrumbleonentity", NULLSUB2, 0 },
  { "playrumblelooponentity", NULLSUB2, 0 },
  { "stoprumble", NULLSUB2, 0 },
  { "startpoisoning", CScr_StartPoisoning, 0 },
  { "stoppoisoning", CScr_StopPoisoning, 0 },
  { "ispoisoned", CScr_IsPoisoned, 0 },
  { "issplitscreenhost", CScrCmd_IsSplitScreenHost, 0 },
  { "setwatchstyle", NULLSUB2, 0 },
  { "getcampos", CScrCmd_GetCamPos, 0 },
  { "getcamangles", CScrCmd_GetCamAngles, 0 },
  { "useanimtree", CScr_UseAnimTree, 0 },
  { "hasanimtree", CScr_HasAnimTree, 0 },
  { "clearanim", CScr_ClearAnim, 0 },
  { "clearanimlimited", CScr_ClearAnimLimited, 0 },
  { "setanim", CScr_SetAnim, 0 },
  { "setanimlimited", CScr_SetAnimLimited, 0 },
  { "setanimrestart", CScr_SetAnimRestart, 0 },
  { "setanimlimitedrestart", CScr_SetAnimLimitedRestart, 0 },
  { "setflaggedanim", CScr_SetFlaggedAnim, 0 },
  { "setflaggedanimlimited", CScr_SetFlaggedAnimLimited, 0 },
  { "setflaggedanimrestart", CScr_SetFlaggedAnimRestart, 0 },
  { "setflaggedanimlimitedrestart", CScr_SetFlaggedAnimLimitedRestart, 0 },
  { "setanimknob", CScr_SetAnimKnob, 0 },
  { "setanimknoblimited", CScr_SetAnimKnobLimited, 0 },
  { "setanimknobrestart", CScr_SetAnimKnobRestart, 0 },
  { "setanimknoblimitedrestart", CScr_SetAnimKnobLimitedRestart, 0 },
  { "setanimknoball", CScr_SetAnimKnobAll, 0 },
  { "setanimknoballlimited", CScr_SetAnimKnobAllLimited, 0 },
  { "setanimknoballrestart", CScr_SetAnimKnobAllRestart, 0 },
  { "setanimknoballlimitedrestart", CScr_SetAnimKnobAllLimitedRestart, 0 },
  { "setflaggedanimknob", CScr_SetFlaggedAnimKnob, 0 },
  { "setflaggedanimknoblimited", CScr_SetFlaggedAnimKnobLimited, 0 },
  { "setflaggedanimknobrestart", CScr_SetFlaggedAnimKnobRestart, 0 },
  {
    "setflaggedanimknoblimitedrestart",
    CScr_SetFlaggedAnimKnobLimitedRestart,
    0
  },
  { "setflaggedanimknoball", CScr_SetFlaggedAnimKnobAll, 0 },
  { "setflaggedanimknoballrestart", CScr_SetFlaggedAnimKnobAllRestart, 0 },
  { "getanimtime", CScr_GetAnimTime, 0 },
  { "setanimtime", CScr_SetAnimTime, 0 },
  { "getanimcurrframecount", CScr_GetAnimCurrFrameCount, 0 },
  { "animgetchildat", CScr_AnimGetChildAt, 0 },
  { "animgetnumchildren", CScr_AnimGetNumChildren, 0 },
  { "map_material", CScr_Map_Material, 0 },
  { "set_filter_pass_material", CScr_Set_Filter_Pass_Material, 0 },
  { "set_filter_pass_enabled", CScr_Set_Filter_Pass_Enabled, 0 },
  { "set_filter_pass_quads", CScr_Set_Filter_Pass_Quads, 0 },
  { "set_filter_pass_constant", CScr_Set_Filter_Pass_Constant, 0 },
  { "set_overlay_enabled", CScr_Set_Overlay_Enabled, 0 },
  { "set_overlay_material", CScr_Set_Overlay_Material, 0 },
  { "set_overlay_constant", CScr_Set_Overlay_Constant, 0 },
  { "setinfraredvisionset", CScr_SetInfraredVisionset, 0 }
};



BuiltinFunctionDef client_functions[154] =
{
  { "print", print, 1 },
  { "print3d", &CScr_Print3D, 1 },
  { "assert", &assertCmd, 1 },
  { "assertex", &assertexCmd, 1 },
  { "assertmsg", &assertmsgCmd, 1 },
  { "structinfo", &CScr_StructInfo, 1 },
  { "spawnstruct", &CScr_AddStruct, 0 },
  { "println", &println, 1 },
  { "iprintlnbold", &CScr_IPrintLnBold, 0 },
  { "line", &CScr_line, 1 },
  { "box", &CScr_box, 1 },
  { "debugstar", &CScr_debugstar, 1 },
  { "clienthassnapshot", &CScr_ClientHasSnapShot, 0 },
  { "getsystemtime", &CScr_GetSystemTime, 0 },
  { "getservertime", &CScr_GetServerTime, 0 },
  { "getarraykeys", &CScr_GetArrayKeys, 0 },
  { "splitargs", &CScr_SplitArgs, 0 },
  { "bullettrace", &CScr_BulletTrace, 0 },
  { "tracepoint", &CScr_TracePoint, 0 },
  { "getlocalclientpos", &CScr_GetLocalClientPos, 0 },
  { "getlocalclienteyepos", &CScr_GetLocalClientEyePos, 0 },
  { "getlocalclientangles", &CScr_GetLocalClientAngles, 0 },
  { "getmaxlocalclients", &CScr_GetMaxLocalClients, 0 },
  { "localclientactive", &CScr_LocalClientActive, 0 },
  { "issplitscreenhost", &CScr_IsSplitScreenHost, 0 },
  { "issplitscreen", &CScr_IsSplitScreen, 0 },
  { "aimingatfriendly", &CScr_AimingAtFriendly, 0 },
  { "randomint", &CScr_RandomInt, 0 },
  { "randomfloat", &CScr_RandomFloat, 0 },
  { "randomintrange", &CScr_RandomIntRange, 0 },
  { "randomfloatrange", &CScr_RandomFloatRange, 0 },
  { "sin", &CScr_sin, 0 },
  { "cos", &CScr_cos, 0 },
  { "tan", &CScr_tan, 0 },
  { "asin", &CScr_asin, 0 },
  { "acos", &CScr_acos, 0 },
  { "atan", &CScr_atan, 0 },
  { "int", &CScr_CastInt, 0 },
  { "float", &CScr_CastFloat, 0 },
  { "abs", &CScr_abs, 0 },
  { "min", &CScr_min, 0 },
  { "max", &CScr_max, 0 },
  { "floor", &CScr_floor, 0 },
  { "ceil", &CScr_ceil, 0 },
  { "sqrt", &CScr_sqrt, 0 },
  { "lerpfloat", &CScr_LerpFloat, 0 },
  { "lerpvector", &CScr_LerpVector, 0 },
  { "isdefined", &CScr_IsDefined, 0 },
  { "isstring", &CScr_IsString, 0 },
  { "isarray", &CScr_IsArray, 0 },
  { "vectorfromlinetopoint", &CScr_VectorFromLineToPoint, 0 },
  { "pointonsegmentnearesttopoint", &CScr_PointOnSegmentNearestToPoint, 0 },
  { "distance", &CScr_Distance, 0 },
  { "distance2d", &CScr_Distance2D, 0 },
  { "distancesquared", &CScr_DistanceSquared, 0 },
  { "length", &CScr_Length, 0 },
  { "lengthsquared", &CScr_LengthSquared, 0 },
  { "closer", &CScr_Closer, 0 },
  { "vectordot", &CScr_VectorDot, 0 },
  { "vectorcross", &CScr_VectorCross, 0 },
  { "vectornormalize", &CScr_VectorNormalize, 0 },
  { "vectortoangles", &CScr_VectorToAngles, 0 },
  { "vectorlerp", &CScr_VectorLerp, 0 },
  { "anglestoup", &CScr_AnglesToUp, 0 },
  { "anglestoright", &CScr_AnglesToRight, 0 },
  { "anglestoforward", &CScr_AnglesToForward, 0 },
  { "combineangles", &CScr_CombineAngles, 0 },
  { "angleclamp180", &CScr_ClampAngle180, 0 },
  { "absangleclamp180", &CScr_AbsAngleClamp180, 0 },
  { "getsubstr", &CScr_GetSubStr, 0 },
  { "tolower", &CScr_ToLower, 0 },
  { "strtok", &CScr_StrTok, 0 },
  { "issubstr", &CScr_IsSubStr, 0 },
  { "spawnfakeent", &CScr_SpawnFakeEnt, 0 },
  { "deletefakeent", &CScr_DeleteFakeEnt, 0 },
  { "setfakeentorg", &CScr_SetFakeEntOrg, 0 },
  { "spawn", &CScr_Spawn, 0 },
  { "spawnplane", &CScr_SpawnPlane, 0 },
  { "getent", &CScr_GetEnt, 0 },
  { "getentnum", &CScr_GetEntNum, 0 },
  { "getentarray", &CScr_GetEntArray, 0 },
  { "getlocalplayers", &CScr_GetLocalPlayers, 0 },
  { "getlocalplayer", &CScr_GetLocalPlayer, 0 },
  { "isalive", &CScr_IsAlive, 0 },
  { "getweaponmodel", &CScr_GetWeaponModel, 0 },
  { "loadfx", &CScr_LoadedFX, 0 },
  { "playfx", &CScr_PlayFX, 0 },
  { "playloopedfx", &CScr_PlayLoopedFX, 0 },
  { "stopfx", &CScr_StopFX, 0 },
  { "isfxplaying", &CScr_IsFXPlaying, 0 },
  { "bullettracer", &CScr_BulletTracer, 0 },
  { "disableimpactmarks", &CScr_DisableImpactMarks, 0 },
  { "enableimpactmarks", &CScr_EnableImpactMarks, 0 },
  { "playloopsound", &CScr_PlayLoopSound, 0 },
  { "stoploopsound", &CScr_StopLoopSound, 0 },
  { "playsound", &CScr_PlaySound, 0 },
  { "soundplaying", &CScr_SoundPlaying, 0 },
  { "deactivatereverb", &CScr_DeactivateReverb, 0 },
  { "setreverb", &CScr_SetReverb, 0 },
  { "setsoundvolume", &CScr_SetSoundVolume, 0 },
  { "setsoundpitch", &CScr_SetSoundPitch, 0 },
  { "setsoundvolumerate", &CScr_SetSoundVolumeRate, 0 },
  { "setsoundpitchrate", &CScr_SetSoundPitchRate, 0 },
  { "stopsound", &CScr_StopSound, 0 },
  { "getrealtime", &CScr_GetRealTime, 0 },
  { "getplaybacktime", &CScr_GetPlaybackTime, 0 },
  { "getknownlength", &CScr_GetKnownLength, 0 },
  { "setgroupsnapshot", &CScr_SetGroupSnapshot, 0 },
  { "setambientsnapshot", &CScr_SetAmbientSnapshot, 0 },
  { "sethealthsnapshot", &CScr_SetHealthSnapshot, 0 },
  { "setlevelfadesnapshot", &CScr_SetLevelFadeSnapshot, 0 },
  { "scalespeed", &CScr_ScaleSpeed, 0 },
  { "soundloopemitter", &CScr_SoundLoopEmitter, 0 },
  { "setsoundcontext", &CScr_SetSoundContext, 0 },
  { "soundstoploopemitter", &CScr_SoundStopLoopEmitter, 0 },
  { "soundlineemitter", &CScr_SoundLineEmitter, 0 },
  { "soundstoplineemitter", &CScr_SoundStopLineEmitter, 0 },
  { "getsoundname", &CScr_GetAliasName, 0 },
  { "getsoundcount", &CScr_GetAliasCount, 0 },
  { "issoundloop", &CScr_IsAliasLoop, 0 },
  { "stoplocalsound", &CScr_StopLocalSound, 0 },
  { "soundtimescale", &CScr_SoundTimescale, 0 },
  { "precacherumble", &CScr_PrecacheRumble, 0 },
  { "playrumbleonposition", &NULLSUB, 0 },
  { "getdvar", &CScr_GetDvar, 0 },
  { "getdvarcolorred", &CScr_GetDvarColorRed, 0 },
  { "getdvarcolorgreen", &CScr_GetDvarColorGreen, 0 },
  { "getdvarcolorblue", &CScr_GetDvarColorBlue, 0 },
  { "getdvarcoloralpha", &CScr_GetDvarColorAlpha, 0 },
  { "getdvarint", &CScr_GetDvarInt, 0 },
  { "getdvarfloat", &CScr_GetDvarFloat, 0 },
  { "getdvarvector", &CScr_GetDvarVector, 0 },
  { "getdebugdvar", &CScr_GetDebugDvar, 1 },
  { "getdebugdvarint", &CScr_GetDebugDvarInt, 1 },
  { "getdebugdvarfloat", &CScr_GetDebugDvarFloat, 1 },
  { "setdvarbool", &CScr_SetDvarBool, 0 },
  { "setdvarfloat", &CScr_SetDvarFloat, 0 },
  { "setdvar", CScr_SetDvar, 0 },
  { "setsaveddvar", &CScr_SetSavedDvar, 0 },
  { "openfile", &CScr_OpenFile, 1 },
  { "closefile", &CScr_CloseFile, 1 },
  { "fprintln", &CScr_FPrintln, 1 },
  { "fprintfields", &CScr_FPrintFields, 1 },
  { "freadln", &CScr_FReadLn, 1 },
  { "fgetarg", &CScr_FGetArg, 1 },
  { "ui3dsetwindow", &CScr_SetUI3DTextureWindow, 0 },
  { "playbink", &CScr_StopBink, 0 },
  { "stopbink", &CScr_StopBink, 0 },
  { "getbinklength", &CScr_GetBinkLength, 0 },
  { "isbinkfinished", &CScr_IsBinkFinished, 0 },
  { "isinvehicle", &CScr_IsInVehicle, 0 },
  { "isps3", &CScr_PrecacheRumble, 0 },
  { "ispc", &CScr_GetMaxLocalClients, 0 },
  { "isxenon", &CScr_PrecacheRumble, 0 }
};

const cent_field_s cent_fields[25] =
{
  { "origin", 48, { 12 }, F_VECTOR, &CScr_SetOrigin, NULL },
  { "angles", 60, { 12 }, F_VECTOR, &CScr_SetAngles, NULL },
  { "weapon", 690, { 2 }, F_STRING, &CScr_ReadOnly, &CScr_GetWeaponName },
  { "targetname", 694, { 2 }, F_STRING, NULL, NULL },
  { "species", 576, { 4 }, F_STRING, &CScr_ReadOnly, &CScr_GetSpecies },
  { "isdog", 576, { 4 }, F_INT, &CScr_ReadOnly, &CScr_GetIsDog },
  { "type", 678, { 2 }, F_STRING, &CScr_ReadOnly, &CScr_GetType },
  { "model", 678, { 2 }, F_STRING, &CScr_ReadOnly, &CScr_GetModel },
  {
    "vehicletype",
    0,
    { 64 },
    F_LSTRING,
    &CScr_ReadOnly,
    &VehicleCScr_GetVehicleInfoField
  },
  {
    "vehicleclass",
    624,
    { 1 },
    F_LSTRING,
    &CScr_ReadOnly,
    &CScr_GetVehicleClass
  },
  {
    "treadfxname",
    624,
    { 1 },
    F_LSTRING,
    &CScr_ReadOnly,
    &CScr_GetVehicleTreadFx
  },
  {
    "treadfxnamearray",
    624,
    { 1 },
    F_LSTRING,
    &CScr_ReadOnly,
    &CScr_GetVehicleTreadFxArray
  },
  {
    "exhaustfxname",
    3984,
    { 64 },
    F_LSTRING,
    &CScr_ReadOnly,
    &VehicleCScr_GetVehicleInfoField
  },
  {
    "oneexhaust",
    4048,
    { 4 },
    F_INT,
    &CScr_ReadOnly,
    &VehicleCScr_GetVehicleInfoField
  },
  {
    "rotormainidlefxname",
    4052,
    { 64 },
    F_LSTRING,
    &CScr_ReadOnly,
    &VehicleCScr_GetVehicleInfoField
  },
  {
    "rotormainstartfxname",
    4116,
    { 64 },
    F_LSTRING,
    &CScr_ReadOnly,
    &VehicleCScr_GetVehicleInfoField
  },
  {
    "rotormainrunningfxname",
    4180,
    { 64 },
    F_LSTRING,
    &CScr_ReadOnly,
    &VehicleCScr_GetVehicleInfoField
  },
  {
    "rotormainstopfxname",
    4244,
    { 64 },
    F_LSTRING,
    &CScr_ReadOnly,
    &VehicleCScr_GetVehicleInfoField
  },
  {
    "rotortailidlefxname",
    4308,
    { 64 },
    F_LSTRING,
    &CScr_ReadOnly,
    &VehicleCScr_GetVehicleInfoField
  },
  {
    "rotortailstartfxname",
    4372,
    { 64 },
    F_LSTRING,
    &CScr_ReadOnly,
    &VehicleCScr_GetVehicleInfoField
  },
  {
    "rotortailrunningfxname",
    4436,
    { 64 },
    F_LSTRING,
    &CScr_ReadOnly,
    &VehicleCScr_GetVehicleInfoField
  },
  {
    "rotortailstopfxname",
    4500,
    { 64 },
    F_LSTRING,
    &CScr_ReadOnly,
    &VehicleCScr_GetVehicleInfoField
  },
  { "enemy", 584, { 4 }, F_ENTITY, &CScr_ReadOnly, &CScr_GetEntityByIndex },
  { "team", 703, { 1 }, F_STRING, &CScr_ReadOnly, &CScr_GetTeamName },
  { NULL, 0, { 0 }, F_INT, NULL, NULL }
};



unsigned __int16 *cg_modNames[21] =
{
    &cscr_const.mod_unknown,
    &cscr_const.mod_pistol_bullet,
    &cscr_const.mod_rifle_bullet,
    &cscr_const.mod_grenade,
    &cscr_const.mod_grenade_splash,
    &cscr_const.mod_projectile,
    &cscr_const.mod_projectile_splash,
    &cscr_const.mod_melee,
    &cscr_const.mod_bayonet,
    &cscr_const.mod_head_shot,
    &cscr_const.mod_crush,
    &cscr_const.mod_telefrag,
    &cscr_const.mod_falling,
    &cscr_const.mod_suicide,
    &cscr_const.mod_trigger_hurt,
    &cscr_const.mod_explosive,
    &cscr_const.mod_impact,
    &cscr_const.mod_burned,
    &cscr_const.mod_hit_by_object,
    &cscr_const.mod_drown,
    &cscr_const.mod_gas
};


void __cdecl CScr_AddStruct()
{
    Scr_AddStruct(SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_SetClientScripts(ScriptFunctions *functions)
{
    cscr_const.none = Scr_AllocString("none", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.asphalt = Scr_AllocString("asphalt", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.bark = Scr_AllocString("bark", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.brick = Scr_AllocString("brick", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.carpet = Scr_AllocString("carpet", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.ceramic = Scr_AllocString("ceramic", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.cloth = Scr_AllocString("cloth", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.concrete = Scr_AllocString("concrete", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.cushion = Scr_AllocString("cushion", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.dirt = Scr_AllocString("dirt", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.flesh = Scr_AllocString("flesh", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.foliage = Scr_AllocString("foliage", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.fruit = Scr_AllocString("fruit", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.glass = Scr_AllocString("glass", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.grass = Scr_AllocString("grass", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.gravel = Scr_AllocString("gravel", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.ice = Scr_AllocString("ice", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.metal = Scr_AllocString("metal", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.mud = Scr_AllocString("mud", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.paintedmetal = Scr_AllocString("paintedmetal", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.paper = Scr_AllocString("paper", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.plaster = Scr_AllocString("plaster", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.plastic = Scr_AllocString("plastic", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.rock = Scr_AllocString("rock", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.rubber = Scr_AllocString("rubber", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.sand = Scr_AllocString("sand", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.snow = Scr_AllocString("snow", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.water = Scr_AllocString("water", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.wood = Scr_AllocString("wood", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.movedone = Scr_AllocString("movedone", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.rotatedone = Scr_AllocString("rotatedone", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.entityshutdown = Scr_AllocString("entityshutdown", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.front_left = Scr_AllocString("front_left", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.front_right = Scr_AllocString("front_right", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.back_left = Scr_AllocString("back_left", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.back_right = Scr_AllocString("back_right", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.middle_left = Scr_AllocString("middle_left", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.middle_right = Scr_AllocString("middle_right", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.fraction = Scr_AllocString("fraction", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.position = Scr_AllocString("position", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.entity = Scr_AllocString("entity", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.normal = Scr_AllocString("normal", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.surfacetype = Scr_AllocString("surfacetype", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.enemy = Scr_AllocString("enemy", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.weapon_fired = Scr_AllocString("weapon_fired", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.broken = Scr_AllocString("broken", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.death = Scr_AllocString("death", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.touch = Scr_AllocString("touch", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.swimming_begin = Scr_AllocString("swimming_begin", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.swimming_end = Scr_AllocString("swimming_end", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.trigger = Scr_AllocString("trigger", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.prone = Scr_AllocString("prone", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.stand = Scr_AllocString("stand", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.crouch = Scr_AllocString("crouch", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.sprint = Scr_AllocString("sprint", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.run = Scr_AllocString("run", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.walk = Scr_AllocString("walk", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.crouchrun = Scr_AllocString("crouch_run", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.crouchwalk = Scr_AllocString("crouch_walk", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.explode = Scr_AllocString("explode", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.dtp_land = Scr_AllocString("dtp_land", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.respawn = Scr_AllocString("respawn", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.breakafter = Scr_AllocString("breakafter", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.fake_fire = Scr_AllocString("fake_fire", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.hidden = Scr_AllocString("hidden", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.mod_unknown = Scr_AllocString("MOD_UNKNOWN", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.mod_pistol_bullet = Scr_AllocString("MOD_PISTOL_BULLET", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.mod_rifle_bullet = Scr_AllocString("MOD_RIFLE_BULLET", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.mod_grenade = Scr_AllocString("MOD_GRENADE", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.mod_grenade_splash = Scr_AllocString("MOD_GRENADE_SPLASH", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.mod_projectile = Scr_AllocString("MOD_PROJECTILE", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.mod_projectile_splash = Scr_AllocString("MOD_PROJECTILE_SPLASH", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.mod_melee = Scr_AllocString("MOD_MELEE", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.mod_bayonet = Scr_AllocString("MOD_BAYONET", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.mod_head_shot = Scr_AllocString("MOD_HEAD_SHOT", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.mod_crush = Scr_AllocString("MOD_CRUSH", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.mod_telefrag = Scr_AllocString("MOD_TELEFRAG", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.mod_falling = Scr_AllocString("MOD_FALLING", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.mod_suicide = Scr_AllocString("MOD_SUICIDE", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.mod_trigger_hurt = Scr_AllocString("MOD_TRIGGER_HURT", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.mod_explosive = Scr_AllocString("MOD_EXPLOSIVE", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.mod_impact = Scr_AllocString("MOD_IMPACT", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.mod_burned = Scr_AllocString("MOD_BURNED", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.mod_hit_by_object = Scr_AllocString("MOD_HIT_BY_OBJECT", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.mod_drown = Scr_AllocString("MOD_DROWN", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.mod_gas = Scr_AllocString("MOD_GAS", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.face = Scr_AllocString("face", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.face_casual = Scr_AllocString("face_casual", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.face_alert = Scr_AllocString("face_alert", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.face_cqb = Scr_AllocString("face_cqb", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.face_running = Scr_AllocString("face_running", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.face_shoot_single = Scr_AllocString("face_shoot_single", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.face_shoot_burst = Scr_AllocString("face_shoot_burst", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.face_melee = Scr_AllocString("face_melee", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.face_react = Scr_AllocString("face_react", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.face_talk = Scr_AllocString("face_talk", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.face_talk_long = Scr_AllocString("face_talk_long", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.face_pain = Scr_AllocString("face_pain", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.face_death = Scr_AllocString("face_death", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.veh_collision = Scr_AllocString("veh_collision", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.veh_inair = Scr_AllocString("veh_inair", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.veh_landed = Scr_AllocString("veh_landed", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.veh_suspension_limit_activated = Scr_AllocString(
                                                                                                "veh_suspension_limit_activated",
                                                                                                1,
                                                                                                SCRIPTINSTANCE_CLIENT);
    cscr_const.veh_engine_stutter = Scr_AllocString("veh_engine_stutter", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.liveupdate = Scr_AllocString("liveupdate", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.disconnect = Scr_AllocString("disconnect", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.freeze = Scr_AllocString("freeze", 1, SCRIPTINSTANCE_CLIENT);
    cscr_const.up = Scr_AllocString("up", 1, SCRIPTINSTANCE_CLIENT);
    CScr_SetUniqueClientScripts(functions);
    CScr_SetLevelScript(functions);
}

unsigned __int16 __cdecl CScr_GetMeansOfDeathConstString(int mod)
{
    return *cg_modNames[mod];
}

void __cdecl CScr_Notify(int localClientNum, centity_s *ent, unsigned __int16 stringValue, unsigned int paramcount)
{
    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp", 301, 0, "%s", "ent") )
        __debugbreak();
    CScr_NotifyNum(localClientNum, ent->nextState.number, 0, stringValue, paramcount);
}

void __cdecl CScr_ToLower()
{
    char v0; // al
    char tempString[1028]; // [esp+4h] [ebp-410h] BYREF
    const char *s; // [esp+40Ch] [ebp-8h]
    int i; // [esp+410h] [ebp-4h]

    s = Scr_GetString(0, SCRIPTINSTANCE_CLIENT);
    i = 0;
    while ( i < 1024 )
    {
        v0 = tolower(*s);
        tempString[i] = v0;
        if ( !v0 )
        {
            Scr_AddString(tempString, SCRIPTINSTANCE_CLIENT);
            return;
        }
        ++i;
        ++s;
    }
    Scr_Error(SCRIPTINSTANCE_CLIENT, "string too long", 0);
}

void __cdecl CScr_ReadOnly(centity_s *cent, const cent_field_s *pField)
{
    char *error; // [esp+0h] [ebp-4h]

    if ( !cent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp", 333, 0, "%s", "cent") )
        __debugbreak();
    if ( !pField && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp", 334, 0, "%s", "pField") )
        __debugbreak();
    error = va("centity field %s is read-only", pField->name);
    Scr_Error(SCRIPTINSTANCE_CLIENT, error, 0);
}

void __cdecl CScr_GetType(centity_s *cent, const cent_field_s *pField)
{
    if ( !cent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp", 343, 0, "%s", "cent") )
        __debugbreak();
    if ( !pField && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp", 344, 0, "%s", "pField") )
        __debugbreak();
    switch ( cent->nextState.eType )
    {
        case 0:
            Scr_AddString("general", SCRIPTINSTANCE_CLIENT);
            break;
        case 1:
            Scr_AddString("player", SCRIPTINSTANCE_CLIENT);
            break;
        case 2:
            Scr_AddString("player_corpse", SCRIPTINSTANCE_CLIENT);
            break;
        case 4:
            Scr_AddString("missile", SCRIPTINSTANCE_CLIENT);
            break;
        case 6:
            Scr_AddString("scriptmover", SCRIPTINSTANCE_CLIENT);
            break;
        case 0xB:
            Scr_AddString("mg42", SCRIPTINSTANCE_CLIENT);
            break;
        case 0xC:
            Scr_AddString("helicopter", SCRIPTINSTANCE_CLIENT);
            break;
        case 0xD:
            Scr_AddString("plane", SCRIPTINSTANCE_CLIENT);
            break;
        case 0xE:
            Scr_AddString("vehicle", SCRIPTINSTANCE_CLIENT);
            break;
        case 0x11:
            Scr_AddString("actor", SCRIPTINSTANCE_CLIENT);
            break;
        default:
            Scr_AddString("NA", SCRIPTINSTANCE_CLIENT);
            break;
    }
}

void __cdecl CScr_GetWeaponName(centity_s *cent, const cent_field_s *pField)
{
    char *value; // [esp+0h] [ebp-4h]

    if ( !cent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp", 398, 0, "%s", "cent") )
        __debugbreak();
    if ( !pField && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp", 399, 0, "%s", "pField") )
        __debugbreak();
    value = (char *)BG_WeaponName(cent->nextState.weapon);
    Scr_AddString(value, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_SetOrigin(centity_s *cent, const cent_field_s *pField)
{
    float org[3]; // [esp+0h] [ebp-Ch] BYREF

    if ( !cent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp", 407, 0, "%s", "cent") )
        __debugbreak();
    if ( !pField && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp", 408, 0, "%s", "pField") )
        __debugbreak();
    Scr_GetVector(0, org, SCRIPTINSTANCE_CLIENT);
    CG_SetOrigin(cent, org);
}

void __cdecl CScr_SetAngles(centity_s *cent, const cent_field_s *pField)
{
    float ang[3]; // [esp+0h] [ebp-Ch] BYREF

    if ( !cent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp", 420, 0, "%s", "cent") )
        __debugbreak();
    if ( !pField && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp", 421, 0, "%s", "pField") )
        __debugbreak();
    Scr_GetVector(0, ang, SCRIPTINSTANCE_CLIENT);
    CG_SetAngle(cent, ang);
}

void __cdecl CScr_GetModel(centity_s *cent, const cent_field_s *pField)
{
    XModel *model; // [esp+14h] [ebp-8h]
    cgs_t *cgs; // [esp+18h] [ebp-4h]

    if ( !cent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp", 433, 0, "%s", "cent") )
        __debugbreak();
    if ( !pField && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp", 434, 0, "%s", "pField") )
        __debugbreak();
    cgs = CG_GetLocalClientStaticGlobals(cent->pose.localClientNum);
    if ( !cgs && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp", 438, 0, "%s", "cgs") )
        __debugbreak();
    switch ( cent->nextState.eType )
    {
        case 6:
            if (cent->nextState.solid == 0xFFFFFF)
                goto LABEL_20;
            model = cent->destructible ? Destructible_GetDDef(cent)->model : cgs->gameModels[cent->nextState.index.brushmodel];
            if ( !model || !model->name )
                goto LABEL_20;
            Scr_AddString((char *)model->name, SCRIPTINSTANCE_CLIENT);
            break;
        case 0xC:
        case 0xE:
        case 0x10:
            Scr_AddString((char *)cgs->gameModels[cent->nextState.un3.item]->name, SCRIPTINSTANCE_CLIENT);
            break;
        case 0x11:
        case 0x13:
            Scr_AddString((char *)cgs->gameModels[cent->nextState.index.brushmodel]->name, SCRIPTINSTANCE_CLIENT);
            break;
        default:
LABEL_20:
            Scr_AddString("NA", SCRIPTINSTANCE_CLIENT);
            break;
    }
}

void __cdecl VehicleCScr_GetVehicleInfoField(centity_s *cent, const cent_field_s *pField)
{
    char *error; // [esp+0h] [ebp-8h]
    const vehicle_info_t *info; // [esp+4h] [ebp-4h]

    if ( !cent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp", 485, 0, "%s", "cent") )
        __debugbreak();
    if ( !pField && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp", 486, 0, "%s", "pField") )
        __debugbreak();
    if ( cent->nextState.eType != 14 && cent->nextState.eType != 16 && cent->nextState.eType != 12 )
    {
        error = va(".%s can only be used on vehicles.", pField->name);
        Scr_Error(SCRIPTINSTANCE_CLIENT, error, 0);
    }
    info = CG_GetVehicleInfo(cent->nextState.vehicleState.vehicleInfoIndex);
    Scr_GetGenericField((unsigned __int8 *)info, pField->type, pField->ofs, SCRIPTINSTANCE_CLIENT, 0);
}

void __cdecl CScr_GetVehicleClass(centity_s *cent, const cent_field_s *pField)
{
    const vehicle_info_t *info; // [esp+4h] [ebp-4h]

    if ( !cent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp", 498, 0, "%s", "cent") )
        __debugbreak();
    if ( !pField && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp", 499, 0, "%s", "pField") )
        __debugbreak();
    if ( cent->nextState.eType != 14 && cent->nextState.eType != 16 && cent->nextState.eType != 12 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, ".vehicleclass can only be used on vehicles.", 0);
    info = CG_GetVehicleInfo(cent->nextState.vehicleState.vehicleInfoIndex);
    Scr_AddString((char *)s_vehicleClassNames[info->type], SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_GetVehicleTreadFxArray(centity_s *cent, const cent_field_s *pField)
{
    const vehicle_info_t *info; // [esp+74h] [ebp-4h]

    if ( !cent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp", 511, 0, "%s", "cent") )
        __debugbreak();
    if ( !pField && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp", 512, 0, "%s", "pField") )
        __debugbreak();
    if ( cent->nextState.eType != 14 && cent->nextState.eType != 16 && cent->nextState.eType != 12 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, ".treadfxnamearray can only be used on vehicles.", 0);
    info = CG_GetVehicleInfo(cent->nextState.vehicleState.vehicleInfoIndex);
    Scr_MakeArray(SCRIPTINSTANCE_CLIENT);
    Scr_AddString(info->treadFx[22], SCRIPTINSTANCE_CLIENT);
    Scr_AddArrayStringIndexed(cscr_const.asphalt, SCRIPTINSTANCE_CLIENT);
    Scr_AddString(info->treadFx[1], SCRIPTINSTANCE_CLIENT);
    Scr_AddArrayStringIndexed(cscr_const.bark, SCRIPTINSTANCE_CLIENT);
    Scr_AddString(info->treadFx[2], SCRIPTINSTANCE_CLIENT);
    Scr_AddArrayStringIndexed(cscr_const.brick, SCRIPTINSTANCE_CLIENT);
    Scr_AddString(info->treadFx[3], SCRIPTINSTANCE_CLIENT);
    Scr_AddArrayStringIndexed(cscr_const.carpet, SCRIPTINSTANCE_CLIENT);
    Scr_AddString(info->treadFx[23], SCRIPTINSTANCE_CLIENT);
    Scr_AddArrayStringIndexed(cscr_const.ceramic, SCRIPTINSTANCE_CLIENT);
    Scr_AddString(info->treadFx[4], SCRIPTINSTANCE_CLIENT);
    Scr_AddArrayStringIndexed(cscr_const.cloth, SCRIPTINSTANCE_CLIENT);
    Scr_AddString(info->treadFx[5], SCRIPTINSTANCE_CLIENT);
    Scr_AddArrayStringIndexed(cscr_const.concrete, SCRIPTINSTANCE_CLIENT);
    Scr_AddString(info->treadFx[26], SCRIPTINSTANCE_CLIENT);
    Scr_AddArrayStringIndexed(cscr_const.cushion, SCRIPTINSTANCE_CLIENT);
    Scr_AddString(info->treadFx[0], SCRIPTINSTANCE_CLIENT);
    Scr_AddArrayStringIndexed(cscr_const.none, SCRIPTINSTANCE_CLIENT);
    Scr_AddString(info->treadFx[6], SCRIPTINSTANCE_CLIENT);
    Scr_AddArrayStringIndexed(cscr_const.dirt, SCRIPTINSTANCE_CLIENT);
    Scr_AddString(info->treadFx[7], SCRIPTINSTANCE_CLIENT);
    Scr_AddArrayStringIndexed(cscr_const.flesh, SCRIPTINSTANCE_CLIENT);
    Scr_AddString(info->treadFx[8], SCRIPTINSTANCE_CLIENT);
    Scr_AddArrayStringIndexed(cscr_const.foliage, SCRIPTINSTANCE_CLIENT);
    Scr_AddString(info->treadFx[27], SCRIPTINSTANCE_CLIENT);
    Scr_AddArrayStringIndexed(cscr_const.fruit, SCRIPTINSTANCE_CLIENT);
    Scr_AddString(info->treadFx[9], SCRIPTINSTANCE_CLIENT);
    Scr_AddArrayStringIndexed(cscr_const.glass, SCRIPTINSTANCE_CLIENT);
    Scr_AddString(info->treadFx[10], SCRIPTINSTANCE_CLIENT);
    Scr_AddArrayStringIndexed(cscr_const.grass, SCRIPTINSTANCE_CLIENT);
    Scr_AddString(info->treadFx[11], SCRIPTINSTANCE_CLIENT);
    Scr_AddArrayStringIndexed(cscr_const.gravel, SCRIPTINSTANCE_CLIENT);
    Scr_AddString(info->treadFx[12], SCRIPTINSTANCE_CLIENT);
    Scr_AddArrayStringIndexed(cscr_const.ice, SCRIPTINSTANCE_CLIENT);
    Scr_AddString(info->treadFx[13], SCRIPTINSTANCE_CLIENT);
    Scr_AddArrayStringIndexed(cscr_const.metal, SCRIPTINSTANCE_CLIENT);
    Scr_AddString(info->treadFx[14], SCRIPTINSTANCE_CLIENT);
    Scr_AddArrayStringIndexed(cscr_const.mud, SCRIPTINSTANCE_CLIENT);
    Scr_AddString(info->treadFx[28], SCRIPTINSTANCE_CLIENT);
    Scr_AddArrayStringIndexed(cscr_const.paintedmetal, SCRIPTINSTANCE_CLIENT);
    Scr_AddString(info->treadFx[15], SCRIPTINSTANCE_CLIENT);
    Scr_AddArrayStringIndexed(cscr_const.paper, SCRIPTINSTANCE_CLIENT);
    Scr_AddString(info->treadFx[16], SCRIPTINSTANCE_CLIENT);
    Scr_AddArrayStringIndexed(cscr_const.plaster, SCRIPTINSTANCE_CLIENT);
    Scr_AddString(info->treadFx[24], SCRIPTINSTANCE_CLIENT);
    Scr_AddArrayStringIndexed(cscr_const.plastic, SCRIPTINSTANCE_CLIENT);
    Scr_AddString(info->treadFx[17], SCRIPTINSTANCE_CLIENT);
    Scr_AddArrayStringIndexed(cscr_const.rock, SCRIPTINSTANCE_CLIENT);
    Scr_AddString(info->treadFx[25], SCRIPTINSTANCE_CLIENT);
    Scr_AddArrayStringIndexed(cscr_const.rubber, SCRIPTINSTANCE_CLIENT);
    Scr_AddString(info->treadFx[18], SCRIPTINSTANCE_CLIENT);
    Scr_AddArrayStringIndexed(cscr_const.sand, SCRIPTINSTANCE_CLIENT);
    Scr_AddString(info->treadFx[19], SCRIPTINSTANCE_CLIENT);
    Scr_AddArrayStringIndexed(cscr_const.snow, SCRIPTINSTANCE_CLIENT);
    Scr_AddString(info->treadFx[20], SCRIPTINSTANCE_CLIENT);
    Scr_AddArrayStringIndexed(cscr_const.water, SCRIPTINSTANCE_CLIENT);
    Scr_AddString(info->treadFx[21], SCRIPTINSTANCE_CLIENT);
    Scr_AddArrayStringIndexed(cscr_const.wood, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_GetVehicleTreadFx(centity_s *cent, const cent_field_s *pField)
{
    char *value; // [esp+0h] [ebp-Ch]
    const vehicle_info_t *info; // [esp+4h] [ebp-8h]
    char *surfaceType; // [esp+8h] [ebp-4h]

    if ( !cent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp", 612, 0, "%s", "cent") )
        __debugbreak();
    if ( !pField && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp", 613, 0, "%s", "pField") )
        __debugbreak();
    if ( cent->nextState.eType != 14 && cent->nextState.eType != 16 && cent->nextState.eType != 12 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, ".treadfxname can only be used on vehicles.", 0);
    info = CG_GetVehicleInfo(cent->nextState.vehicleState.vehicleInfoIndex);
    surfaceType = Scr_GetString(0, SCRIPTINSTANCE_CLIENT);
    value = (char*)info->treadFx[Com_SurfaceTypeFromName(surfaceType)];
    Scr_AddString(value, SCRIPTINSTANCE_CLIENT);
}

unsigned int __cdecl CScr_GetFakeEntity(unsigned int localClientNum)
{
    fake_centity_s *cent; // [esp+8h] [ebp-8h]
    unsigned int i; // [esp+Ch] [ebp-4h]

    if ( localClientNum >= 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                    628,
                    0,
                    "localClientNum not in [0, MAX_LOCAL_CLIENTS]\n\t%i not in [%i, %i]",
                    localClientNum,
                    0,
                    1) )
    {
        __debugbreak();
    }
    for ( i = 0; i < 0x200; ++i )
    {
        cent = &cg_fakeEntitiesArray[512 * localClientNum + i];
        if ( !cg_fakeEntitiesInuseArray[512 * localClientNum + cent - &cg_fakeEntitiesArray[512 * localClientNum]] )
        {
            CG_SetFakeEntInUse(localClientNum, cent);
            cent->cent.nextState.number = i + 1024;
            cent->cent.pose.localClientNum = localClientNum;
            *((unsigned int *)&cent->cent + 201) |= 2u;
            return i;
        }
    }
    if ( !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                    646,
                    0,
                    "Run out of fake local centities") )
        __debugbreak();
    return -1;
}

void __cdecl CG_SetFakeEntInUse(int localClientNum, fake_centity_s *ent)
{
    if ( !cg_fakeEntitiesInuseArray[512 * localClientNum + ent - &cg_fakeEntitiesArray[512 * localClientNum]] )
        ++cg_fakeEntitiesInuseCount[localClientNum];
    cg_fakeEntitiesInuseArray[512 * localClientNum + ent - &cg_fakeEntitiesArray[512 * localClientNum]] = 1;
}

void CScr_SpawnFakeEnt()
{
    unsigned int value; // [esp+0h] [ebp-10h]
    unsigned int localClientNum; // [esp+Ch] [ebp-4h]

    localClientNum = CScr_GetLocalClientNum(0);
    value = CScr_GetFakeEntity(localClientNum);
    Scr_AddInt(value, SCRIPTINSTANCE_CLIENT);
}

int __cdecl CScr_GetLocalClientNum(unsigned int index)
{
    char *error; // [esp+0h] [ebp-8h]
    int localClientNum; // [esp+4h] [ebp-4h]

    localClientNum = Scr_GetInt(index, SCRIPTINSTANCE_CLIENT);
    if ( localClientNum )
    {
        error = va("Trying to get a local client index for a client '%d' that is not a local client.", localClientNum);
        Scr_Error(SCRIPTINSTANCE_CLIENT, error, 0);
    }
    return localClientNum;
}

void CScr_DeleteFakeEnt()
{
    unsigned int localClientNum; // [esp+8h] [ebp-8h]
    unsigned int ent; // [esp+Ch] [ebp-4h]

    localClientNum = CScr_GetLocalClientNum(0);
    ent = Scr_GetInt(1u, SCRIPTINSTANCE_CLIENT);
    CScr_FreeFakeEntity(localClientNum, ent);
}

void __cdecl CScr_FreeFakeEntity(unsigned int localClientNum, unsigned int entNum)
{
    if ( localClientNum >= 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                    654,
                    0,
                    "localClientNum not in [0, MAX_LOCAL_CLIENTS]\n\t%i not in [%i, %i]",
                    localClientNum,
                    0,
                    1) )
    {
        __debugbreak();
    }
    if ( entNum > 0x200
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                    656,
                    0,
                    "entNum not in [0, MAX_FAKE_LOCAL_CENTITIES]\n\t%i not in [%i, %i]",
                    entNum,
                    0,
                    512) )
    {
        __debugbreak();
    }
    CG_Free(localClientNum, entNum + 1024);
}

void CScr_SetFakeEntOrg()
{
    unsigned int localClientNum; // [esp+14h] [ebp-18h]
    fake_centity_s *cent; // [esp+18h] [ebp-14h]
    float org[3]; // [esp+1Ch] [ebp-10h] BYREF
    int entNum; // [esp+28h] [ebp-4h]

    localClientNum = CScr_GetLocalClientNum(0);
    entNum = Scr_GetInt(1u, SCRIPTINSTANCE_CLIENT);
    if (localClientNum >= 2
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
            730,
            0,
            "localClientNum not in [0, MAX_LOCAL_CLIENTS]\n\t%i not in [%i, %i]",
            localClientNum,
            0,
            1))
    {
        __debugbreak();
    }
    if ((unsigned int)entNum > 0x200
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
            731,
            0,
            "entNum not in [0, MAX_FAKE_LOCAL_CENTITIES]\n\t%i not in [%i, %i]",
            entNum,
            0,
            512))
    {
        __debugbreak();
    }
    Scr_GetVector(2u, org, SCRIPTINSTANCE_CLIENT);
    cent = &cg_fakeEntitiesArray[512 * localClientNum + entNum];
    if (!cg_fakeEntitiesInuseArray[512 * localClientNum + cent - &cg_fakeEntitiesArray[512 * localClientNum]])
        Scr_ParamError(1u, "Fake ent is not in use!", SCRIPTINSTANCE_CLIENT);
    cent->cent.pose.origin[0] = org[0];
    cent->cent.pose.origin[1] = org[1];
    cent->cent.pose.origin[2] = org[2];
}

void __cdecl CScr_StructContents(unsigned int index)
{
    const char *CanonicalString; // eax
    unsigned int i; // [esp+0h] [ebp-100Ch]
    unsigned int names[1025]; // [esp+4h] [ebp-1008h] BYREF
    unsigned int AllVariableField; // [esp+1008h] [ebp-4h]

    AllVariableField = Scr_FindAllVariableField(SCRIPTINSTANCE_CLIENT, index, names);
    Com_Printf(cg_level.scriptPrintChannel, "Struct has %i fields :\n", AllVariableField);
    for ( i = 0; i < AllVariableField; ++i )
    {
        CanonicalString = Scr_GetCanonicalString(SCRIPTINSTANCE_CLIENT, names[i]);
        Com_Printf(cg_level.scriptPrintChannel, "%s\n", CanonicalString);
    }
}

void CScr_StructInfo()
{
    VariableUnion structId; // [esp+0h] [ebp-4h]

    structId.intValue = Scr_GetObject(0, SCRIPTINSTANCE_CLIENT);
    CScr_StructContents(structId.stringValue);
}

void CScr_IsDefined()
{
    int type; // [esp+4h] [ebp-4h]
    signed int typea; // [esp+4h] [ebp-4h]

    type = Scr_GetType(0, SCRIPTINSTANCE_CLIENT);
    if ( type == 1 )
    {
        typea = Scr_GetPointerType(0, SCRIPTINSTANCE_CLIENT);
        if ( typea < 13
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        790,
                        0,
                        "%s",
                        "type >= FIRST_OBJECT") )
        {
            __debugbreak();
        }
        if ( typea >= 21 || typea == 18 )
            Scr_AddInt(0, SCRIPTINSTANCE_CLIENT);
        else
            Scr_AddInt(1, SCRIPTINSTANCE_CLIENT);
    }
    else
    {
        if ( type >= 13
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        795,
                        0,
                        "%s",
                        "type < FIRST_OBJECT") )
        {
            __debugbreak();
        }
        Scr_AddInt(type != 0, SCRIPTINSTANCE_CLIENT);
    }
}

void CScr_IsString()
{
    bool value; // [esp+0h] [ebp-4h]

    value = Scr_GetType(0, SCRIPTINSTANCE_CLIENT) == 2;
    Scr_AddInt(value, SCRIPTINSTANCE_CLIENT);
}

void CScr_IsArray()
{
    int type; // [esp+0h] [ebp-4h]
    signed int typea; // [esp+0h] [ebp-4h]

    type = Scr_GetType(0, SCRIPTINSTANCE_CLIENT);
    if ( type == 1 )
    {
        typea = Scr_GetPointerType(0, SCRIPTINSTANCE_CLIENT);
        if ( typea < 13
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        847,
                        0,
                        "%s",
                        "type >= FIRST_OBJECT") )
        {
            __debugbreak();
        }
        Scr_AddInt(typea == 20, SCRIPTINSTANCE_CLIENT);
    }
    else
    {
        if ( type >= 13
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        852,
                        0,
                        "%s",
                        "type < FIRST_OBJECT") )
        {
            __debugbreak();
        }
        Scr_AddInt(0, SCRIPTINSTANCE_CLIENT);
    }
}

//void assertCmd()
//{
//    if ( !Scr_GetInt(0, SCRIPTINSTANCE_CLIENT) )
//        Scr_Error(SCRIPTINSTANCE_CLIENT, "assert fail", 1);
//}
//
//void assertexCmd()
//{
//    char *error; // [esp+0h] [ebp-8h]
//    char *String; // [esp+4h] [ebp-4h]
//
//    if ( !Scr_GetInt(0, SCRIPTINSTANCE_CLIENT) )
//    {
//        String = Scr_GetString(1u, SCRIPTINSTANCE_CLIENT);
//        error = va("assert fail: %s", String);
//        Scr_Error(SCRIPTINSTANCE_CLIENT, error, 1);
//    }
//}
//
//void assertmsgCmd()
//{
//    char *error; // [esp+0h] [ebp-8h]
//    char *String; // [esp+4h] [ebp-4h]
//
//    String = Scr_GetString(0, SCRIPTINSTANCE_CLIENT);
//    error = va("assert fail: %s", String);
//    Scr_Error(SCRIPTINSTANCE_CLIENT, error, 1);
//}

//void print()
//{
//    char *DebugString; // [esp+0h] [ebp-Ch]
//    int num; // [esp+4h] [ebp-8h]
//    signed int i; // [esp+8h] [ebp-4h]
//
//    num = Scr_GetNumParam(SCRIPTINSTANCE_CLIENT);
//    for (i = 0; i < num; ++i)
//    {
//        DebugString = Scr_GetDebugString(i, SCRIPTINSTANCE_CLIENT);
//        Com_Printf(cg_level.scriptPrintChannel, "%s", DebugString);
//    }
//}

//void println()
//{
//    print();
//    Com_Printf(cg_level.scriptPrintChannel, "\n");
//}

void CScr_IPrintLnBold()
{
    char *str; // [esp+0h] [ebp-4h]

    str = Scr_GetDebugString(0, SCRIPTINSTANCE_CLIENT);
    CG_BoldGameMessage(0, str, 0);
}

void CScr_Print3D()
{
    VariableUnion duration; // [esp+10h] [ebp-34h]
    float origin[3]; // [esp+14h] [ebp-30h] BYREF
    float rgb[3]; // [esp+20h] [ebp-24h] BYREF
    float scale; // [esp+2Ch] [ebp-18h]
    float color[4]; // [esp+30h] [ebp-14h] BYREF
    const char *text; // [esp+40h] [ebp-4h]

    duration.intValue = 1;
    scale = 1.0f;
    color[0] = 1.0f;
    color[1] = 1.0f;
    color[2] = 1.0f;
    color[3] = 1.0f;
    switch ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) )
    {
        case 2:
            goto $LN21_8;
        case 3:
            goto $LN3_30;
        case 4:
            goto $LN4_25;
        case 5:
            goto $LN5_23;
        case 6:
            duration.intValue = Scr_GetInt(5u, SCRIPTINSTANCE_CLIENT);
$LN5_23:
            scale = Scr_GetFloat(4u, SCRIPTINSTANCE_CLIENT);
$LN4_25:
            color[3] = Scr_GetFloat(3u, SCRIPTINSTANCE_CLIENT);
$LN3_30:
            Scr_GetVector(2u, rgb, SCRIPTINSTANCE_CLIENT);
            color[0] = rgb[0];
            color[1] = rgb[1];
            color[2] = rgb[2];
$LN21_8:
            text = Scr_GetString(1u, SCRIPTINSTANCE_CLIENT);
            Scr_GetVector(0, origin, SCRIPTINSTANCE_CLIENT);
            CL_AddDebugString(origin, color, scale, (char *)text, duration.intValue);
            break;
        default:
            Scr_Error(SCRIPTINSTANCE_CLIENT, "illegal call to print3d()", 0);
            break;
    }
}

void CScr_RandomInt()
{
    int value; // [esp+0h] [ebp-8h]
    int iMax; // [esp+4h] [ebp-4h]

    iMax = Scr_GetInt(0, SCRIPTINSTANCE_CLIENT);
    if ( iMax > 0 )
    {
        value = irand(0, iMax);
        Scr_AddInt(value, SCRIPTINSTANCE_CLIENT);
    }
    else
    {
        Com_Printf(24, "RandomInt parm: %d    ", iMax);
        Scr_Error(SCRIPTINSTANCE_CLIENT, "RandomInt parm must be positive integer.\n", 0);
    }
}

void CScr_RandomFloat()
{
    float v0; // [esp+8h] [ebp-8h]
    float fMax; // [esp+Ch] [ebp-4h]

    fMax = Scr_GetFloat(0, SCRIPTINSTANCE_CLIENT);
    v0 = flrand(0.0, fMax);
    Scr_AddFloat(v0, SCRIPTINSTANCE_CLIENT);
}

void CScr_RandomIntRange()
{
    int value; // [esp+0h] [ebp-Ch]
    int iMax; // [esp+4h] [ebp-8h]
    int iMin; // [esp+8h] [ebp-4h]

    iMin = Scr_GetInt(0, SCRIPTINSTANCE_CLIENT);
    iMax = Scr_GetInt(1u, SCRIPTINSTANCE_CLIENT);
    if ( iMax <= iMin )
    {
        Com_Printf(24, "RandomIntRange parms: %d %d ", iMin, iMax);
        Scr_Error(SCRIPTINSTANCE_CLIENT, "RandomIntRange range must be positive integer.\n", 0);
    }
    value = irand(iMin, iMax);
    Scr_AddInt(value, SCRIPTINSTANCE_CLIENT);
}

void CScr_RandomFloatRange()
{
    float v0; // [esp+10h] [ebp-Ch]
    float fMin; // [esp+14h] [ebp-8h]
    float fMax; // [esp+18h] [ebp-4h]

    fMin = Scr_GetFloat(0, SCRIPTINSTANCE_CLIENT);
    fMax = Scr_GetFloat(1u, SCRIPTINSTANCE_CLIENT);
    if ( fMin > fMax )
    {
        Com_Printf(24, "CScr_RandomFloatRange parms: %g %g ", fMin, fMax);
        Scr_Error(SCRIPTINSTANCE_CLIENT, "Scr_RandomFloatRange range must be positive float.\n", 0);
    }
    v0 = flrand(fMin, fMax);
    Scr_AddFloat(v0, SCRIPTINSTANCE_CLIENT);
}

void CScr_VectorFromLineToPoint()
{
    float segmentB[3]; // [esp+4h] [ebp-50h] BYREF
    float result[3]; // [esp+10h] [ebp-44h] BYREF
    float BA[3]; // [esp+1Ch] [ebp-38h]
    float PA[3]; // [esp+28h] [ebp-2Ch]
    float fraction; // [esp+34h] [ebp-20h]
    float segmentLengthSq; // [esp+38h] [ebp-1Ch]
    float segmentA[3]; // [esp+3Ch] [ebp-18h] BYREF
    float P[3]; // [esp+48h] [ebp-Ch] BYREF

    Scr_GetVector(0, segmentA, SCRIPTINSTANCE_CLIENT);
    Scr_GetVector(1u, segmentB, SCRIPTINSTANCE_CLIENT);
    Scr_GetVector(2u, P, SCRIPTINSTANCE_CLIENT);
    BA[0] = segmentB[0] - segmentA[0];
    BA[1] = segmentB[1] - segmentA[1];
    BA[2] = segmentB[2] - segmentA[2];
    segmentLengthSq = (float)((float)((float)(segmentB[0] - segmentA[0]) * (float)(segmentB[0] - segmentA[0]))
                                                    + (float)((float)(segmentB[1] - segmentA[1]) * (float)(segmentB[1] - segmentA[1])))
                                    + (float)((float)(segmentB[2] - segmentA[2]) * (float)(segmentB[2] - segmentA[2]));
    if ( segmentLengthSq == 0.0 )
        Scr_ParamError(0, "The two points on the line must be different from each other", SCRIPTINSTANCE_CLIENT);
    PA[0] = P[0] - segmentA[0];
    PA[1] = P[1] - segmentA[1];
    PA[2] = P[2] - segmentA[2];
    fraction = (float)((float)((float)(BA[0] * (float)(P[0] - segmentA[0])) + (float)(BA[1] * (float)(P[1] - segmentA[1])))
                                     + (float)(BA[2] * (float)(P[2] - segmentA[2])))
                     / segmentLengthSq;
    //result[0] = (float)(COERCE_FLOAT(LODWORD(fraction) ^ _mask__NegFloat_) * BA[0]) + (float)(P[0] - segmentA[0]);
    //result[1] = (float)(COERCE_FLOAT(LODWORD(fraction) ^ _mask__NegFloat_) * BA[1]) + (float)(P[1] - segmentA[1]);
    //result[2] = (float)(COERCE_FLOAT(LODWORD(fraction) ^ _mask__NegFloat_) * BA[2]) + (float)(P[2] - segmentA[2]);
    result[0] = (float)(-(fraction) * BA[0]) + (float)(P[0] - segmentA[0]);
    result[1] = (float)(-(fraction) * BA[1]) + (float)(P[1] - segmentA[1]);
    result[2] = (float)(-(fraction) * BA[2]) + (float)(P[2] - segmentA[2]);
    Scr_AddVector(result, SCRIPTINSTANCE_CLIENT);
}

void CScr_PointOnSegmentNearestToPoint()
{
    float segmentB[3]; // [esp+0h] [ebp-50h] BYREF
    float BA[3]; // [esp+Ch] [ebp-44h]
    float PA[3]; // [esp+18h] [ebp-38h]
    float fraction; // [esp+24h] [ebp-2Ch]
    float segmentLengthSq; // [esp+28h] [ebp-28h]
    float segmentA[3]; // [esp+2Ch] [ebp-24h] BYREF
    float P[3]; // [esp+38h] [ebp-18h] BYREF
    float nearPoint[3]; // [esp+44h] [ebp-Ch] BYREF

    Scr_GetVector(0, segmentA, SCRIPTINSTANCE_CLIENT);
    Scr_GetVector(1u, segmentB, SCRIPTINSTANCE_CLIENT);
    Scr_GetVector(2u, P, SCRIPTINSTANCE_CLIENT);
    BA[0] = segmentB[0] - segmentA[0];
    BA[1] = segmentB[1] - segmentA[1];
    BA[2] = segmentB[2] - segmentA[2];
    segmentLengthSq = (float)((float)((float)(segmentB[0] - segmentA[0]) * (float)(segmentB[0] - segmentA[0]))
                                                    + (float)((float)(segmentB[1] - segmentA[1]) * (float)(segmentB[1] - segmentA[1])))
                                    + (float)((float)(segmentB[2] - segmentA[2]) * (float)(segmentB[2] - segmentA[2]));
    if ( segmentLengthSq == 0.0 )
        Scr_ParamError(0, "Line segment must not have zero length", SCRIPTINSTANCE_CLIENT);
    PA[0] = P[0] - segmentA[0];
    PA[1] = P[1] - segmentA[1];
    PA[2] = P[2] - segmentA[2];
    fraction = (float)((float)((float)(BA[0] * (float)(P[0] - segmentA[0])) + (float)(BA[1] * (float)(P[1] - segmentA[1])))
                                     + (float)(BA[2] * (float)(P[2] - segmentA[2])))
                     / segmentLengthSq;
    if ( fraction >= 0.0 )
    {
        if ( fraction <= 1.0 )
        {
            nearPoint[0] = (float)(fraction * BA[0]) + segmentA[0];
            nearPoint[1] = (float)(fraction * BA[1]) + segmentA[1];
            nearPoint[2] = (float)(fraction * BA[2]) + segmentA[2];
            Scr_AddVector(nearPoint, SCRIPTINSTANCE_CLIENT);
        }
        else
        {
            Scr_AddVector(segmentB, SCRIPTINSTANCE_CLIENT);
        }
    }
    else
    {
        Scr_AddVector(segmentA, SCRIPTINSTANCE_CLIENT);
    }
}

void CScr_Distance()
{
    float value; // [esp+10h] [ebp-30h]
    float v0[3]; // [esp+24h] [ebp-1Ch] BYREF
    float v1[3]; // [esp+34h] [ebp-Ch] BYREF

    PROF_SCOPED("CScr_Distance");

    Scr_GetVector(0, v0, SCRIPTINSTANCE_CLIENT);
    Scr_GetVector(1u, v1, SCRIPTINSTANCE_CLIENT);
    value = Vec3Distance(v0, v1);
    Scr_AddFloat(value, SCRIPTINSTANCE_CLIENT);
}

void CScr_Distance2D()
{
    float value; // [esp+8h] [ebp-28h]
    float v[2]; // [esp+10h] [ebp-20h] BYREF
    float v0[3]; // [esp+18h] [ebp-18h] BYREF
    float v1[3]; // [esp+24h] [ebp-Ch] BYREF

    Scr_GetVector(0, v0, SCRIPTINSTANCE_CLIENT);
    Scr_GetVector(1u, v1, SCRIPTINSTANCE_CLIENT);
    v[0] = v1[0] - v0[0];
    v[1] = v1[1] - v0[1];
    value = Vec2Length(v);
    Scr_AddFloat(value, SCRIPTINSTANCE_CLIENT);
}

void CScr_DistanceSquared()
{
    float value; // [esp+8h] [ebp-28h]
    float v0[3]; // [esp+18h] [ebp-18h] BYREF
    float v1[3]; // [esp+24h] [ebp-Ch] BYREF

    Scr_GetVector(0, v0, SCRIPTINSTANCE_CLIENT);
    Scr_GetVector(1u, v1, SCRIPTINSTANCE_CLIENT);
    value = Vec3DistanceSq(v0, v1);
    Scr_AddFloat(value, SCRIPTINSTANCE_CLIENT);
}

void CScr_Length()
{
    float value; // [esp+8h] [ebp-14h]
    float v[3]; // [esp+10h] [ebp-Ch] BYREF

    Scr_GetVector(0, v, SCRIPTINSTANCE_CLIENT);
    value = Vec3Length(v);
    Scr_AddFloat(value, SCRIPTINSTANCE_CLIENT);
}

void CScr_LengthSquared()
{
    float v[3]; // [esp+Ch] [ebp-Ch] BYREF

    Scr_GetVector(0, v, SCRIPTINSTANCE_CLIENT);
    Scr_AddFloat((float)((float)(v[0] * v[0]) + (float)(v[1] * v[1])) + (float)(v[2] * v[2]), SCRIPTINSTANCE_CLIENT);
}

void CScr_Closer()
{
    float fDistBSqrd; // [esp+1Ch] [ebp-2Ch]
    float vB[3]; // [esp+20h] [ebp-28h] BYREF
    float fDistASqrd; // [esp+2Ch] [ebp-1Ch]
    float vRef[3]; // [esp+30h] [ebp-18h] BYREF
    float vA[3]; // [esp+3Ch] [ebp-Ch] BYREF

    Scr_GetVector(0, vRef, SCRIPTINSTANCE_CLIENT);
    Scr_GetVector(1u, vA, SCRIPTINSTANCE_CLIENT);
    Scr_GetVector(2u, vB, SCRIPTINSTANCE_CLIENT);
    fDistASqrd = Vec3DistanceSq(vA, vRef);
    fDistBSqrd = Vec3DistanceSq(vB, vRef);
    Scr_AddInt(fDistBSqrd > fDistASqrd, SCRIPTINSTANCE_CLIENT);
}

void CScr_VectorDot()
{
    float b[3]; // [esp+Ch] [ebp-18h] BYREF
    float a[3]; // [esp+18h] [ebp-Ch] BYREF

    Scr_GetVector(0, a, SCRIPTINSTANCE_CLIENT);
    Scr_GetVector(1u, b, SCRIPTINSTANCE_CLIENT);
    Scr_AddFloat((float)((float)(a[0] * b[0]) + (float)(a[1] * b[1])) + (float)(a[2] * b[2]), SCRIPTINSTANCE_CLIENT);
}

void CScr_VectorCross()
{
    float b[3]; // [esp+0h] [ebp-24h] BYREF
    float tempVec[3]; // [esp+Ch] [ebp-18h] BYREF
    float a[3]; // [esp+18h] [ebp-Ch] BYREF

    Scr_GetVector(0, a, SCRIPTINSTANCE_CLIENT);
    Scr_GetVector(1u, b, SCRIPTINSTANCE_CLIENT);
    Vec3Cross(a, b, tempVec);
    Scr_AddVector(tempVec, SCRIPTINSTANCE_CLIENT);
}

void CScr_VectorNormalize()
{
    float b[3]; // [esp+Ch] [ebp-18h] BYREF
    float a[3]; // [esp+18h] [ebp-Ch] BYREF

    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) != 1 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "wrong number of arguments to vectornormalize!", 0);
    Scr_GetVector(0, a, SCRIPTINSTANCE_CLIENT);
    b[0] = a[0];
    b[1] = a[1];
    b[2] = a[2];
    Vec3Normalize(b);
    Scr_AddVector(b, SCRIPTINSTANCE_CLIENT);
}

void CScr_VectorToAngles()
{
    float angles[3]; // [esp+0h] [ebp-18h] BYREF
    float vec[3]; // [esp+Ch] [ebp-Ch] BYREF

    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) != 1 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "wrong number of arguments to vectortoangle!", 0);
    Scr_GetVector(0, vec, SCRIPTINSTANCE_CLIENT);
    vectoangles(vec, angles);
    Scr_AddVector(angles, SCRIPTINSTANCE_CLIENT);
}

void CScr_VectorLerp()
{
    float from[3]; // [esp+8h] [ebp-28h] BYREF
    float result[3]; // [esp+14h] [ebp-1Ch] BYREF
    float fraction; // [esp+20h] [ebp-10h]
    float to[3]; // [esp+24h] [ebp-Ch] BYREF

    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) != 3 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "wrong number of arguments to vectorlerp", 0);
    Scr_GetVector(0, from, SCRIPTINSTANCE_CLIENT);
    Scr_GetVector(1u, to, SCRIPTINSTANCE_CLIENT);
    fraction = Scr_GetFloat(2u, SCRIPTINSTANCE_CLIENT);
    Vec3Lerp(from, to, fraction, result);
    Scr_AddVector(result, SCRIPTINSTANCE_CLIENT);
}

void CScr_LerpFloat()
{
    float v0; // [esp+8h] [ebp-18h]
    float v1; // [esp+Ch] [ebp-14h]
    float from; // [esp+10h] [ebp-10h]
    float time; // [esp+14h] [ebp-Ch]
    float to; // [esp+18h] [ebp-8h]

    from = Scr_GetFloat(0, SCRIPTINSTANCE_CLIENT);
    to = Scr_GetFloat(1u, SCRIPTINSTANCE_CLIENT);
    time = Scr_GetFloat(2u, SCRIPTINSTANCE_CLIENT);
    if ( (float)(time - 1.0) < 0.0 )
        v1 = time;
    else
        v1 = 1.0f;
    if ( (float)(0.0 - time) < 0.0 )
        v0 = v1;
    else
        v0 = 0.0f;
    Scr_AddFloat((float)((float)(to - from) * v0) + from, SCRIPTINSTANCE_CLIENT);
}

void CScr_LerpVector()
{
    float v0; // [esp+0h] [ebp-30h]
    float v1; // [esp+4h] [ebp-2Ch]
    float from[3]; // [esp+8h] [ebp-28h] BYREF
    float time; // [esp+14h] [ebp-1Ch]
    float to[3]; // [esp+18h] [ebp-18h] BYREF
    float retVal[3]; // [esp+24h] [ebp-Ch] BYREF

    Scr_GetVector(0, from, SCRIPTINSTANCE_CLIENT);
    Scr_GetVector(1u, to, SCRIPTINSTANCE_CLIENT);
    time = Scr_GetFloat(2u, SCRIPTINSTANCE_CLIENT);
    if ( (float)(time - 1.0) < 0.0 )
        v1 = time;
    else
        v1 = 1.0f;
    if ( (float)(0.0 - time) < 0.0 )
        v0 = v1;
    else
        v0 = 0.0f;
    time = v0;
    retVal[0] = (float)((float)(to[0] - from[0]) * v0) + from[0];
    retVal[1] = (float)((float)(to[1] - from[1]) * v0) + from[1];
    retVal[2] = (float)((float)(to[2] - from[2]) * v0) + from[2];
    Scr_AddVector(retVal, SCRIPTINSTANCE_CLIENT);
}

void CScr_AnglesToUp()
{
    float angles[3]; // [esp+0h] [ebp-18h] BYREF
    float up[3]; // [esp+Ch] [ebp-Ch] BYREF

    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) != 1 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "wrong number of arguments to anglestoup!", 0);
    Scr_GetVector(0, angles, SCRIPTINSTANCE_CLIENT);
    AngleVectors(angles, 0, 0, up);
    Scr_AddVector(up, SCRIPTINSTANCE_CLIENT);
}

void CScr_AnglesToRight()
{
    float right[3]; // [esp+0h] [ebp-18h] BYREF
    float angles[3]; // [esp+Ch] [ebp-Ch] BYREF

    Scr_GetVector(0, angles, SCRIPTINSTANCE_CLIENT);
    AngleVectors(angles, 0, right, 0);
    Scr_AddVector(right, SCRIPTINSTANCE_CLIENT);
}

void CScr_AnglesToForward()
{
    float forward[3]; // [esp+0h] [ebp-18h] BYREF
    float angles[3]; // [esp+Ch] [ebp-Ch] BYREF

    Scr_GetVector(0, angles, SCRIPTINSTANCE_CLIENT);
    AngleVectors(angles, forward, 0, 0);
    Scr_AddVector(forward, SCRIPTINSTANCE_CLIENT);
}

void CScr_CombineAngles()
{
    float anglesfinal[3]; // [esp+0h] [ebp-90h] BYREF
    float axisB[3][3]; // [esp+Ch] [ebp-84h] BYREF
    float anglesA[3]; // [esp+30h] [ebp-60h] BYREF
    float axisA[3][3]; // [esp+3Ch] [ebp-54h] BYREF
    float anglesB[3]; // [esp+60h] [ebp-30h] BYREF
    float combinedaxis[3][3]; // [esp+6Ch] [ebp-24h] BYREF

    Scr_GetVector(0, anglesA, SCRIPTINSTANCE_CLIENT);
    Scr_GetVector(1u, anglesB, SCRIPTINSTANCE_CLIENT);
    AnglesToAxis(anglesA, axisA);
    AnglesToAxis(anglesB, axisB);
    MatrixMultiply(axisB, axisA, combinedaxis);
    AxisToAngles(combinedaxis, anglesfinal);
    Scr_AddVector(anglesfinal, SCRIPTINSTANCE_CLIENT);
}

void CScr_ClampAngle180()
{
    float v0; // [esp+8h] [ebp-Ch]
    float anglea; // [esp+Ch] [ebp-8h]
    float angle; // [esp+Ch] [ebp-8h]

    anglea = Scr_GetFloat(0, SCRIPTINSTANCE_CLIENT);
    v0 = floor((float)(anglea / 360.0));
    angle = ((float)(anglea / 360.0) - v0) * 360.0;
    if ( angle <= 180.0 )
        Scr_AddFloat(angle, SCRIPTINSTANCE_CLIENT);
    else
        Scr_AddFloat(angle - 360.0, SCRIPTINSTANCE_CLIENT);
}

void CScr_AbsAngleClamp180()
{
    float v0; // [esp+8h] [ebp-Ch]
    float anglea; // [esp+Ch] [ebp-8h]
    float angle; // [esp+Ch] [ebp-8h]

    anglea = Scr_GetFloat(0, SCRIPTINSTANCE_CLIENT);
    v0 = floor((float)(anglea / 360.0));
    angle = ((float)(anglea / 360.0) - v0) * 360.0;
    if ( angle <= 180.0 )
        Scr_AddFloat(angle, SCRIPTINSTANCE_CLIENT);
    else
        Scr_AddFloat(360.0 - angle, SCRIPTINSTANCE_CLIENT);
}

int __cdecl Effect_Index(int clientNum, const char *name)
{
    char *ConfigString; // eax
    signed int i; // [esp+0h] [ebp-Ch]

    CG_GetLocalClientGlobals(clientNum);
    for ( i = 2080; i < 2276; ++i )
    {
        ConfigString = CL_GetConfigString(i);
        if ( !I_stricmp(ConfigString, name) )
            return i - 2080;
    }
    return 0;
}

void CScr_LoadedFX()
{
    int v0; // eax
    char *error; // [esp+0h] [ebp-Ch]
    char *filename; // [esp+4h] [ebp-8h]
    int id; // [esp+8h] [ebp-4h]

    filename = Scr_GetString(0, SCRIPTINSTANCE_CLIENT);
    if ( !I_strncmp(filename, "fx/", 3) )
        Scr_ParamError(0, "effect name should start after the 'fx' folder.", SCRIPTINSTANCE_CLIENT);
    v0 = RETURN_ZERO32();
    id = Effect_Index(v0, filename);
    if ( !id )
    {
        error = va(
                            "loadedfx could not find effect %s - ensure that effect is precached in the server script, and included in "
                            "appropriate CSVs.\n",
                            filename);
        Scr_Error(SCRIPTINSTANCE_CLIENT, error, 0);
    }
    Scr_AddInt(id, SCRIPTINSTANCE_CLIENT);
}

void CScr_PlayLoopedFX()
{
    unsigned int NumParam; // [esp+4h] [ebp-8Ch]
    float *origin; // [esp+8h] [ebp-88h]
    float Float; // [esp+20h] [ebp-70h]
    float pos[3]; // [esp+44h] [ebp-4Ch] BYREF
    int localClientNum; // [esp+50h] [ebp-40h]
    fake_centity_s *cent; // [esp+54h] [ebp-3Ch]
    int fxId; // [esp+58h] [ebp-38h]
    int givenAxisCount; // [esp+5Ch] [ebp-34h]
    int period; // [esp+60h] [ebp-30h]
    int entId; // [esp+64h] [ebp-2Ch]
    float axis[3][3]; // [esp+68h] [ebp-28h] BYREF
    float cullDist; // [esp+8Ch] [ebp-4h]

    cullDist = 0.0f;
    period = 0;
    if (Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) < 5 || Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) > 8)
        Scr_Error(SCRIPTINSTANCE_CLIENT, "Incorrect number of parameters", 0);
    givenAxisCount = 0;
    cullDist = 0.0f;
    localClientNum = CScr_GetLocalClientNum(0);
    entId = Scr_GetInt(1u, SCRIPTINSTANCE_CLIENT);
    fxId = Scr_GetInt(2u, SCRIPTINSTANCE_CLIENT);
    NumParam = Scr_GetNumParam(SCRIPTINSTANCE_CLIENT);
    if (NumParam != 6)
    {
        if (NumParam != 7)
        {
            if (NumParam != 8)
                goto LABEL_13;
            ++givenAxisCount;
            Scr_GetVector(7u, axis[2], SCRIPTINSTANCE_CLIENT);
            if (Vec3Normalize(axis[2]) == 0.0)
                CScr_FxParamError(localClientNum, 7, "playLoopedFx called with (0 0 0) up direction", fxId);
        }
        Scr_GetVector(6u, axis[0], SCRIPTINSTANCE_CLIENT);
        if (Vec3Normalize(axis[0]) == 0.0)
            CScr_FxParamError(localClientNum, 6, "playLoopedFx called with (0 0 0) forward direction", fxId);
        ++givenAxisCount;
    }
    cullDist = Scr_GetFloat(5u, SCRIPTINSTANCE_CLIENT);
LABEL_13:
    Scr_GetVector(4u, pos, SCRIPTINSTANCE_CLIENT);
    Float = Scr_GetFloat(3u, SCRIPTINSTANCE_CLIENT);
    period = (int)((float)(Float * 1000.0) + 9.313225746154785e-10);
    if (period <= 0)
        CScr_FxParamError(localClientNum, 3, "playLoopedFx called with repeat < 0.001 seconds", fxId);
    if ((unsigned int)entId > 0x200
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
            1884,
            0,
            "entId not in [0, MAX_FAKE_LOCAL_CENTITIES]\n\t%i not in [%i, %i]",
            entId,
            0,
            512))
    {
        __debugbreak();
    }
    cent = &cg_fakeEntitiesArray[512 * localClientNum + entId];
    if (!cg_fakeEntitiesInuseArray[512 * localClientNum + cent - &cg_fakeEntitiesArray[512 * localClientNum]]
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
            1887,
            0,
            "%s",
            "CG_FakeEntInUse(localClientNum,cent)"))
    {
        __debugbreak();
    }
    cent->cent.nextState.eType = ET_LOOP_FX;
    cent->cent.nextState.un1.scale = fxId;
    origin = cent->cent.pose.origin;
    cent->cent.pose.origin[0] = pos[0];
    origin[1] = pos[1];
    origin[2] = pos[2];
    CScr_SetFxAngles(givenAxisCount, axis, cent->cent.nextState.lerp.apos.trBase);
    cent->cent.nextState.lerp.u.turret.gunAngles[0] = cullDist;
    cent->cent.nextState.lerp.u.loopFx.period = period;
}

void __cdecl CScr_BulletTracer()
{
    float start[3]; // [esp+8h] [ebp-1Ch] BYREF
    float end[3]; // [esp+14h] [ebp-10h] BYREF
    int force; // [esp+20h] [ebp-4h]

    force = 0;
    Scr_GetVector(0, start, SCRIPTINSTANCE_CLIENT);
    Scr_GetVector(1u, end, SCRIPTINSTANCE_CLIENT);
    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) == 3 )
        force = Scr_GetInt(2u, SCRIPTINSTANCE_CLIENT) != 0;
    if ( force || (float)(cg_tracerChance->current.value * 32768.0) > (float)rand() )
        CG_SpawnTracer(0, start, end, 0, 0.0, 0.0);
}

void __cdecl CScr_AnimateUI()
{
    uiInfo_s *Info; // eax
    char *menuName; // [esp+0h] [ebp-14h]
    char *itemName; // [esp+4h] [ebp-10h]
    char *newState; // [esp+8h] [ebp-Ch]
    VariableUnion localClientNum; // [esp+Ch] [ebp-8h]
    VariableUnion milliseconds; // [esp+10h] [ebp-4h]

    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) == 5 )
    {
        localClientNum.intValue = Scr_GetInt(0, SCRIPTINSTANCE_CLIENT);
        menuName = Scr_GetString(1u, SCRIPTINSTANCE_CLIENT);
        itemName = Scr_GetString(2u, SCRIPTINSTANCE_CLIENT);
        newState = Scr_GetString(3u, SCRIPTINSTANCE_CLIENT);
        milliseconds.intValue = Scr_GetInt(4u, SCRIPTINSTANCE_CLIENT);
        Info = UI_GetInfo(localClientNum.intValue);
        UI_AnimateItem(localClientNum.intValue, &Info->uiDC, menuName, itemName, newState, milliseconds.intValue);
        UI_AnimateItem(
            localClientNum.intValue,
            &cgDC[localClientNum.intValue],
            menuName,
            itemName,
            newState,
            milliseconds.intValue);
    }
    else
    {
        Scr_Error(
            SCRIPTINSTANCE_CLIENT,
            "Incorrect number of parameters passed to AnimateUI()\n"
            "USAGE: AnimateUI( <localClientNum>, <menuName>, <itemName>, <newState>, <milliseconds> )\n",
            0);
    }
}

void __cdecl CScr_GetAnimLength()
{
    float value; // [esp+8h] [ebp-14h]
    scr_anim_s anim; // [esp+14h] [ebp-8h]
    XAnim_s *anims; // [esp+18h] [ebp-4h]

    anim = Scr_GetAnim(0, 0, SCRIPTINSTANCE_CLIENT);
    anims = Scr_GetClientAnims(anim.tree, SCRIPTINSTANCE_CLIENT);
    if ( !XAnimIsPrimitive(anims, anim.index) )
        Scr_ParamError(0, "non-primitive animation has no concept of length", SCRIPTINSTANCE_CLIENT);
    value = XAnimGetLength(anims, anim.index);
    Scr_AddFloat(value, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_ShowUI()
{
    char *menuName; // [esp+0h] [ebp-Ch]
    int localClientNum; // [esp+8h] [ebp-4h]

    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) == 3 )
    {
        localClientNum = Scr_GetInt(0, SCRIPTINSTANCE_CLIENT);
        menuName = Scr_GetString(1u, SCRIPTINSTANCE_CLIENT);
        if ( Scr_GetInt(2u, SCRIPTINSTANCE_CLIENT) )
            Menus_ShowByName(&cgDC[localClientNum], menuName);
        else
            Menus_HideByName(&cgDC[localClientNum], menuName);
    }
    else
    {
        Scr_Error(
            SCRIPTINSTANCE_CLIENT,
            "Incorrect number of parameters passed to ShowUI()\nUSAGE: ShowUI( <localClientNum>, <menuName>, <newState> )\n",
            0);
    }
}

void __cdecl CScr_GetSystemTime()
{
    qtime_s system_time; // [esp+0h] [ebp-34h] BYREF
    float time[3]; // [esp+24h] [ebp-10h] BYREF
    bool useTimeZone; // [esp+33h] [ebp-1h]

    useTimeZone = 1;
    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) == 1 )
        useTimeZone = Scr_GetInt(0, SCRIPTINSTANCE_CLIENT) == 0;
    memset(time, 0, sizeof(time));
    Com_RealTime(&system_time, useTimeZone);
    time[0] = (float)system_time.tm_hour;
    time[1] = (float)system_time.tm_min;
    time[2] = (float)system_time.tm_sec;
    Scr_AddVector(time, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_GetServerTime()
{
    cg_s *cgameGlob; // [esp+Ch] [ebp-10h]
    int localClientNum; // [esp+10h] [ebp-Ch]
    snapshot_s *nextSnap; // [esp+14h] [ebp-8h]
    int deltaTime; // [esp+18h] [ebp-4h]

    localClientNum = CScr_GetLocalClientNum(0);
    deltaTime = 0;
    if ( (unsigned int)localClientNum >= 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                    2147,
                    0,
                    "localClientNum not in [0, MAX_LOCAL_CLIENTS]\n\t%i not in [%i, %i]",
                    localClientNum,
                    0,
                    1) )
    {
        __debugbreak();
    }
    CG_GetLocalClientGlobals(localClientNum);
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    nextSnap = cgameGlob->nextSnap;
    if ( nextSnap )
        deltaTime = nextSnap->ps.deltaTime;
    Scr_AddInt(cgameGlob->time - deltaTime, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_ClientHasSnapShot()
{
    cg_s *cgameGlob; // [esp+Ch] [ebp-8h]
    int localClientNum; // [esp+10h] [ebp-4h]

    localClientNum = CScr_GetLocalClientNum(0);
    if ( (unsigned int)localClientNum >= 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                    2164,
                    0,
                    "localClientNum not in [0, MAX_LOCAL_CLIENTS]\n\t%i not in [%i, %i]",
                    localClientNum,
                    0,
                    1) )
    {
        __debugbreak();
    }
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    Scr_AddInt(cgameGlob->nextSnap != 0, SCRIPTINSTANCE_CLIENT);
}

void CScr_GetDvar()
{
    VariableUnion dvarHash; // [esp+0h] [ebp-Ch]
    char *string; // [esp+4h] [ebp-8h]
    char *dvarName; // [esp+8h] [ebp-4h]

    if ( Scr_GetType(0, SCRIPTINSTANCE_CLIENT) == 6 )
    {
        dvarHash.intValue = Scr_GetInt(0, SCRIPTINSTANCE_CLIENT);
        string = (char *)Dvar_GetVariantString(dvarHash.intValue);
    }
    else
    {
        dvarName = Scr_GetString(0, SCRIPTINSTANCE_CLIENT);
        string = (char *)Dvar_GetVariantString(dvarName);
    }
    Scr_AddString(string, SCRIPTINSTANCE_CLIENT);
}

void CScr_SetDvar()
{
    char *v0; // [esp+18h] [ebp-434h]
    char *error; // [esp+28h] [ebp-424h]
    char outString[1024]; // [esp+34h] [ebp-418h] BYREF
    const char *dvarName; // [esp+438h] [ebp-14h]
    int i; // [esp+43Ch] [ebp-10h]
    const dvar_s *dvar; // [esp+440h] [ebp-Ch]
    const char *text; // [esp+444h] [ebp-8h]
    char *pCh; // [esp+448h] [ebp-4h]

    PROF_SCOPED("SetDvar");

    dvarName = Scr_GetString(0, SCRIPTINSTANCE_CLIENT);
    text = Scr_GetString(1u, SCRIPTINSTANCE_CLIENT);
    strlen(text);
    pCh = outString;
    i = 0;
    while (i < 1023 && text[i])
    {
        *pCh = text[i];
        if (*pCh == 34)
            *pCh = 39;
        ++i;
        ++pCh;
    }
    *pCh = 0;
    if (!Dvar_IsValidName(dvarName))
    {
        error = va("Dvar %s has an invalid dvar name", dvarName);
        Scr_Error(SCRIPTINSTANCE_CLIENT, error, 0);
        return;
    }
    dvar = Dvar_FindVar(dvarName);
    if (dvar && (dvar->flags & 0x4000) == 0)
    {
        v0 = va(
            "Invalid Dvar set: %s - Internal Dvars cannot be changed by script. Use 'setsaveddvar' to alter SAVED internal dvars\n",
            dvarName);
        Scr_Error(SCRIPTINSTANCE_CLIENT, v0, 0);
    }
    if (!dvar || (dvar->flags & 0x4000) != 0)
        Dvar_SetFromStringByNameFromSource(dvarName, outString, DVAR_SOURCE_SCRIPT, 0);
}

void CScr_SetSavedDvar()
{
    char *v0; // [esp+10h] [ebp-42Ch]
    char *error; // [esp+14h] [ebp-428h]
    char outString[1028]; // [esp+1Ch] [ebp-420h] BYREF
    const char *dvarName; // [esp+424h] [ebp-18h]
    int type; // [esp+428h] [ebp-14h]
    int i; // [esp+42Ch] [ebp-10h]
    const dvar_s *dvar; // [esp+430h] [ebp-Ch]
    const char *text; // [esp+434h] [ebp-8h]
    char *pCh; // [esp+438h] [ebp-4h]

    dvarName = Scr_GetString(0, SCRIPTINSTANCE_CLIENT);
    type = Scr_GetType(1u, SCRIPTINSTANCE_CLIENT);
    if ( type == 3 )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "Cannot set saved dvar on client side with a string value", 0);
    }
    else
    {
        text = Scr_GetString(1u, SCRIPTINSTANCE_CLIENT);
        strlen(text);
        pCh = outString;
        memset(outString, 0, 0x400u);
        i = 0;
        while ( i < 0x4000 && text[i] )
        {
            *pCh = text[i];
            if ( *pCh == 34 )
                *pCh = 39;
            ++i;
            ++pCh;
        }
        if ( Dvar_IsValidName(dvarName) )
        {
            dvar = Dvar_FindVar(dvarName);
            if ( dvar )
            {
                if ( (dvar->flags & 0x1000) != 0 )
                    Dvar_SetFromStringByNameFromSource(dvarName, outString, DVAR_SOURCE_SCRIPT, 0);
                else
                    Scr_Error(SCRIPTINSTANCE_CLIENT, "SetSavedDvar can only be called on dvars with the SAVED flag set", 0);
            }
            else
            {
                v0 = va("SetSavedDvar(): The dvar \"%s\" does not exist.", dvarName);
                Scr_Error(SCRIPTINSTANCE_CLIENT, v0, 0);
            }
        }
        else
        {
            error = va("Dvar %s has an invalid dvar name", dvarName);
            Scr_Error(SCRIPTINSTANCE_CLIENT, error, 0);
        }
    }
}

void CScr_GetDvarColorRed()
{
    VariableUnion dvarHash; // [esp+8h] [ebp-Ch]
    char *dvarName; // [esp+Ch] [ebp-8h]
    float value; // [esp+10h] [ebp-4h]

    if ( Scr_GetType(0, SCRIPTINSTANCE_CLIENT) == 6 )
    {
        dvarHash.intValue = Scr_GetInt(0, SCRIPTINSTANCE_CLIENT);
        value = Dvar_GetColorRed(dvarHash.intValue);
    }
    else
    {
        dvarName = Scr_GetString(0, SCRIPTINSTANCE_CLIENT);
        value = Dvar_GetColorRed(dvarName);
    }
    Scr_AddFloat(value, SCRIPTINSTANCE_CLIENT);
}

void CScr_GetDvarColorGreen()
{
    VariableUnion dvarHash; // [esp+8h] [ebp-Ch]
    char *dvarName; // [esp+Ch] [ebp-8h]
    float value; // [esp+10h] [ebp-4h]

    if ( Scr_GetType(0, SCRIPTINSTANCE_CLIENT) == 6 )
    {
        dvarHash.intValue = Scr_GetInt(0, SCRIPTINSTANCE_CLIENT);
        value = Dvar_GetColorGreen(dvarHash.intValue);
    }
    else
    {
        dvarName = Scr_GetString(0, SCRIPTINSTANCE_CLIENT);
        value = Dvar_GetColorGreen(dvarName);
    }
    Scr_AddFloat(value, SCRIPTINSTANCE_CLIENT);
}

void CScr_GetDvarColorBlue()
{
    VariableUnion dvarHash; // [esp+8h] [ebp-Ch]
    char *dvarName; // [esp+Ch] [ebp-8h]
    float value; // [esp+10h] [ebp-4h]

    if ( Scr_GetType(0, SCRIPTINSTANCE_CLIENT) == 6 )
    {
        dvarHash.intValue = Scr_GetInt(0, SCRIPTINSTANCE_CLIENT);
        value = Dvar_GetColorBlue(dvarHash.intValue);
    }
    else
    {
        dvarName = Scr_GetString(0, SCRIPTINSTANCE_CLIENT);
        value = Dvar_GetColorBlue(dvarName);
    }
    Scr_AddFloat(value, SCRIPTINSTANCE_CLIENT);
}

void CScr_GetDvarColorAlpha()
{
    VariableUnion dvarHash; // [esp+8h] [ebp-Ch]
    char *dvarName; // [esp+Ch] [ebp-8h]
    float value; // [esp+10h] [ebp-4h]

    if ( Scr_GetType(0, SCRIPTINSTANCE_CLIENT) == 6 )
    {
        dvarHash.intValue = Scr_GetInt(0, SCRIPTINSTANCE_CLIENT);
        value = Dvar_GetColorAlpha(dvarHash.intValue);
    }
    else
    {
        dvarName = Scr_GetString(0, SCRIPTINSTANCE_CLIENT);
        value = Dvar_GetColorAlpha(dvarName);
    }
    Scr_AddFloat(value, SCRIPTINSTANCE_CLIENT);
}

void CScr_GetDvarInt()
{
    const char *VariantString; // eax
    VariableUnion dvarHash; // [esp+0h] [ebp-Ch]
    char *dvarName; // [esp+4h] [ebp-8h]
    int value; // [esp+8h] [ebp-4h]

    if ( Scr_GetType(0, SCRIPTINSTANCE_CLIENT) == 6 )
    {
        dvarHash.intValue = Scr_GetInt(0, SCRIPTINSTANCE_CLIENT);
        value = Dvar_GetInt(dvarHash.intValue);
    }
    else
    {
        dvarName = Scr_GetString(0, SCRIPTINSTANCE_CLIENT);
        VariantString = Dvar_GetVariantString(dvarName);
        value = atoi(VariantString);
    }
    Scr_AddInt(value, SCRIPTINSTANCE_CLIENT);
}

void CScr_GetDvarFloat()
{
    const char *VariantString; // eax
    VariableUnion dvarHash; // [esp+8h] [ebp-Ch]
    char *dvarName; // [esp+Ch] [ebp-8h]
    float value; // [esp+10h] [ebp-4h]

    if ( Scr_GetType(0, SCRIPTINSTANCE_CLIENT) == 6 )
    {
        dvarHash.intValue = Scr_GetInt(0, SCRIPTINSTANCE_CLIENT);
        value = Dvar_GetFloat(dvarHash.intValue);
    }
    else
    {
        dvarName = Scr_GetString(0, SCRIPTINSTANCE_CLIENT);
        VariantString = Dvar_GetVariantString(dvarName);
        value = atof(VariantString);
    }
    Scr_AddFloat(value, SCRIPTINSTANCE_CLIENT);
}

void CScr_GetDvarVector()
{
    VariableUnion dvarHash; // [esp+0h] [ebp-Ch]
    char *dvarName; // [esp+4h] [ebp-8h]
    const dvar_s *dvar; // [esp+8h] [ebp-4h]

    if ( Scr_GetType(0, SCRIPTINSTANCE_CLIENT) == 6 )
    {
        dvarHash.intValue = Scr_GetInt(0, SCRIPTINSTANCE_CLIENT);
        dvar = Dvar_FindVar(dvarHash.intValue);
    }
    else
    {
        dvarName = Scr_GetString(0, SCRIPTINSTANCE_CLIENT);
        dvar = Dvar_FindVar(dvarName);
    }
    if ( dvar )
    {
        if ( dvar->type != DVAR_TYPE_FLOAT_3 )
            Scr_Error(SCRIPTINSTANCE_CLIENT, "dvar is not a vector in but GetDvarVec has been called on it", 0);
        Scr_AddVector((float*)&dvar->current.value, SCRIPTINSTANCE_CLIENT);
    }
    else
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "Invalid Dvar passed into GetDvarVector", 0);
    }
}

void CScr_GetDebugDvar()
{
    char *value; // [esp+0h] [ebp-Ch]
    char *dvarName; // [esp+4h] [ebp-8h]
    const dvar_s *dvar; // [esp+8h] [ebp-4h]

    dvarName = Scr_GetString(0, SCRIPTINSTANCE_CLIENT);
    dvar = Dvar_FindVar(dvarName);
    if ( dvar )
    {
        value = (char *)Dvar_DisplayableValue(dvar);
        Scr_AddString(value, SCRIPTINSTANCE_CLIENT);
    }
    else
    {
        Scr_AddString((char *)"", SCRIPTINSTANCE_CLIENT);
    }
}

void CScr_GetDebugDvarInt()
{
    const char *VariantString; // eax
    int value; // [esp+0h] [ebp-8h]
    char *dvarName; // [esp+4h] [ebp-4h]

    dvarName = Scr_GetString(0, SCRIPTINSTANCE_CLIENT);
    VariantString = Dvar_GetVariantString(dvarName);
    value = atoi(VariantString);
    Scr_AddInt(value, SCRIPTINSTANCE_CLIENT);
}

void CScr_GetDebugDvarFloat()
{
    const char *VariantString; // eax
    float value; // [esp+8h] [ebp-8h]
    char *dvarName; // [esp+Ch] [ebp-4h]

    dvarName = Scr_GetString(0, SCRIPTINSTANCE_CLIENT);
    VariantString = Dvar_GetVariantString(dvarName);
    value = atof(VariantString);
    Scr_AddFloat(value, SCRIPTINSTANCE_CLIENT);
}

void CScr_SetDvarBool()
{
    char *dvarName; // [esp+0h] [ebp-8h]
    int value; // [esp+4h] [ebp-4h]

    dvarName = Scr_GetString(0, SCRIPTINSTANCE_CLIENT);
    value = Scr_GetInt(1u, SCRIPTINSTANCE_CLIENT);
    Dvar_SetBoolByName(dvarName, value != 0);
}

void CScr_SetDvarFloat()
{
    char *dvarName; // [esp+4h] [ebp-8h]
    float value; // [esp+8h] [ebp-4h]

    dvarName = Scr_GetString(0, SCRIPTINSTANCE_CLIENT);
    value = Scr_GetFloat(1u, SCRIPTINSTANCE_CLIENT);
    Dvar_SetFloatByName(dvarName, value);
}

void CScr_PlaySound()
{
    unsigned int AliasId; // eax
    float v1; // [esp+8h] [ebp-34h]
    int value; // [esp+Ch] [ebp-30h]
    float v3; // [esp+10h] [ebp-2Ch]
    float Float; // [esp+14h] [ebp-28h]
    float pos[3]; // [esp+20h] [ebp-1Ch] BYREF
    float volume; // [esp+2Ch] [ebp-10h]
    const char *pAlias; // [esp+30h] [ebp-Ch]
    int localClientNum; // [esp+34h] [ebp-8h]
    bool havePosition; // [esp+3Bh] [ebp-1h]

    volume = 1.0f;
    havePosition = 0;
    localClientNum = CScr_GetLocalClientNum(0);
    pAlias = Scr_GetString(1u, SCRIPTINSTANCE_CLIENT);
    if ( (unsigned int)Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) >= 3 )
    {
        Scr_GetVector(2u, pos, SCRIPTINSTANCE_CLIENT);
        havePosition = 1;
    }
    if ( (unsigned int)Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) >= 4 )
    {
        Float = Scr_GetFloat(3u, SCRIPTINSTANCE_CLIENT);
        if ( (float)(Float - 1.0) < 0.0 )
            v3 = Float;
        else
            v3 = 1.0f;
        if ( (float)(0.0 - Float) < 0.0 )
            v1 = v3;
        else
            v1 = 0.0f;
        volume = v1;
    }
    AliasId = SND_FindAliasId((char *)pAlias);
    value = CG_PlaySoundWithHandle(localClientNum, 1022, havePosition ? pos : 0, 0, 0, volume, AliasId);
    Scr_AddInt(value, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_PlaySoundOnEntity(scr_entref_t entref)
{
    unsigned int AliasId; // eax
    float v2; // [esp+8h] [ebp-40h]
    int value; // [esp+Ch] [ebp-3Ch]
    float v4; // [esp+10h] [ebp-38h]
    float Float; // [esp+14h] [ebp-34h]
    centity_s *pSelf; // [esp+28h] [ebp-20h]
    float pos[3]; // [esp+2Ch] [ebp-1Ch] BYREF
    float volume; // [esp+38h] [ebp-10h]
    const char *pAlias; // [esp+3Ch] [ebp-Ch]
    int localClientNum; // [esp+40h] [ebp-8h]
    fake_centity_s *pFake; // [esp+44h] [ebp-4h]

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
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        2675,
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
    volume = 1.0f;
    localClientNum = entref.client;
    pAlias = Scr_GetString(1u, SCRIPTINSTANCE_CLIENT);
    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) == 3 )
    {
        Scr_GetVector(2u, pos, SCRIPTINSTANCE_CLIENT);
    }
    else
    {
        pos[0] = pSelf->pose.origin[0];
        pos[1] = pSelf->pose.origin[1];
        pos[2] = pSelf->pose.origin[2];
    }
    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) == 4 )
    {
        Float = Scr_GetFloat(3u, SCRIPTINSTANCE_CLIENT);
        if ( (float)(Float - 1.0) < 0.0 )
            v4 = Float;
        else
            v4 = 1.0f;
        if ( (float)(0.0 - Float) < 0.0 )
            v2 = v4;
        else
            v2 = 0.0f;
        volume = v2;
    }
    AliasId = SND_FindAliasId((char *)pAlias);
    value = CG_PlaySoundWithHandle(localClientNum, entref.entnum, pos, 0, 0, volume, AliasId);
    Scr_AddInt(value, SCRIPTINSTANCE_CLIENT);
}

void CScr_PlayLoopSound()
{
    int value; // [esp+14h] [ebp-30h]
    char *error; // [esp+18h] [ebp-2Ch]
    unsigned int localClientNum; // [esp+2Ch] [ebp-18h]
    centity_s *cent; // [esp+30h] [ebp-14h]
    const char *pszSoundName; // [esp+34h] [ebp-10h]
    float fadeTime; // [esp+38h] [ebp-Ch]
    float fadeTimea; // [esp+38h] [ebp-Ch]
    unsigned int entId; // [esp+40h] [ebp-4h]

    PROF_SCOPED("CScr_PlayLoopSound");

    localClientNum = CScr_GetLocalClientNum(0);
    entId = Scr_GetInt(1u, SCRIPTINSTANCE_CLIENT);
    if (localClientNum >= 2
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
            2725,
            0,
            "localClientNum not in [0, MAX_LOCAL_CLIENTS]\n\t%i not in [%i, %i]",
            localClientNum,
            0,
            1))
    {
        __debugbreak();
    }
    if (entId > 0x200
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
            2726,
            0,
            "entId not in [0, MAX_FAKE_LOCAL_CENTITIES]\n\t%i not in [%i, %i]",
            entId,
            0,
            512))
    {
        __debugbreak();
    }
    cent = &cg_fakeEntitiesArray[512 * localClientNum + entId].cent;
    if (!cg_fakeEntitiesInuseArray[512 * localClientNum
        + &cg_fakeEntitiesArray[512 * localClientNum + entId]
        - &cg_fakeEntitiesArray[512 * localClientNum]]
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
            2731,
            0,
            "%s",
            "CG_FakeEntInUse(localClientNum,fcent)"))
    {
        __debugbreak();
    }
    pszSoundName = Scr_GetString(2u, SCRIPTINSTANCE_CLIENT);
    cent->nextState.loopSoundId = SND_FindAliasId(pszSoundName);
    fadeTime = 0.0f;
    if (Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) == 4)
    {
        fadeTimea = Scr_GetFloat(3u, SCRIPTINSTANCE_CLIENT);
        if (fadeTimea < 0.0 || fadeTimea > 32.0)
        {
            error = va("playloopsound: invalid fade value %f. it must be between 0 and 32 seconds.", fadeTimea);
            Scr_ParamError(3u, error, SCRIPTINSTANCE_CLIENT);
        }
        fadeTime = fadeTimea * 1000.0;
    }
    cent->nextState.number = entId + 1024;
    cent->nextState.loopSoundFade = (int)fadeTime;
    value = CG_PlaySoundWithHandle(
        localClientNum,
        cent->nextState.number,
        0,
        (__int16)(int)fadeTime,
        0,
        1.0,
        cent->nextState.loopSoundId);
    Scr_AddInt(value, SCRIPTINSTANCE_CLIENT);
}

void CScr_StopLoopSound()
{
    char *error; // [esp+14h] [ebp-28h]
    unsigned int localClientNum; // [esp+28h] [ebp-14h]
    centity_s *cent; // [esp+2Ch] [ebp-10h]
    float fadeTime; // [esp+30h] [ebp-Ch]
    unsigned int entId; // [esp+38h] [ebp-4h]

    PROF_SCOPED("CScr_StopLoopSound");

    localClientNum = CScr_GetLocalClientNum(0);
    entId = Scr_GetInt(1u, SCRIPTINSTANCE_CLIENT);
    if (localClientNum >= 2
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
            2760,
            0,
            "localClientNum not in [0, MAX_LOCAL_CLIENTS]\n\t%i not in [%i, %i]",
            localClientNum,
            0,
            1))
    {
        __debugbreak();
    }
    if (entId > 0x200
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
            2761,
            0,
            "entId not in [0, MAX_FAKE_LOCAL_CENTITIES]\n\t%i not in [%i, %i]",
            entId,
            0,
            512))
    {
        __debugbreak();
    }
    cent = &cg_fakeEntitiesArray[512 * localClientNum + entId].cent;
    if (!cg_fakeEntitiesInuseArray[512 * localClientNum
        + &cg_fakeEntitiesArray[512 * localClientNum + entId]
        - &cg_fakeEntitiesArray[512 * localClientNum]]
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
            2765,
            0,
            "%s",
            "CG_FakeEntInUse(localClientNum,fcent)"))
    {
        __debugbreak();
    }
    fadeTime = 0.0f;
    if (Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) == 3)
        fadeTime = Scr_GetFloat(2u, SCRIPTINSTANCE_CLIENT) * 1000.0;
    if (fadeTime < 0.0 || fadeTime > 32000.0)
    {
        error = va("stoploopsound: invalid fade value %f. it must be between 0 and 32 seconds.", fadeTime);
        Scr_ParamError(0, error, SCRIPTINSTANCE_CLIENT);
        fadeTime = 0.0f;
    }
    if (Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) == 2 || fadeTime == 0.0)
    {
        cent->nextState.loopSoundId = 0;
        cent->nextState.loopSoundFade = 0;
    }
    else
    {
        //cent->nextState.loopSoundFade = (int)COERCE_FLOAT(LODWORD(fadeTime) ^ _mask__NegFloat_);
        cent->nextState.loopSoundFade = -fadeTime;
    }
}

void __cdecl CScr_PlayLoopSoundOnEntity(scr_entref_t entref)
{
    unsigned int AliasId; // eax
    int value; // [esp+8h] [ebp-24h]
    char *error; // [esp+Ch] [ebp-20h]
    centity_s *pSelf; // [esp+1Ch] [ebp-10h]
    char *pAlias; // [esp+20h] [ebp-Ch]
    float fadeTime; // [esp+24h] [ebp-8h]
    float fadeTimea; // [esp+24h] [ebp-8h]

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        2802,
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
    pAlias = Scr_GetString(0, SCRIPTINSTANCE_CLIENT);
    pSelf->nextState.loopSoundId = SND_FindAliasId(pAlias);
    fadeTime = 0.0f;
    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) == 2 )
    {
        fadeTimea = Scr_GetFloat(1u, SCRIPTINSTANCE_CLIENT);
        if ( fadeTimea < 0.0 || fadeTimea > 32.0 )
        {
            error = va("playloopsound: invalid fade value %f. it must be between 0 and 32 seconds.", fadeTimea);
            Scr_ParamError(1u, error, SCRIPTINSTANCE_CLIENT);
        }
        fadeTime = fadeTimea * 1000.0;
    }
    pSelf->nextState.loopSoundFade = (int)fadeTime;
    AliasId = SND_FindAliasId(pAlias);
    value = CG_PlaySoundWithHandle(entref.client, entref.entnum, pSelf->pose.origin, (int)fadeTime, 0, 1.0, AliasId);
    Scr_AddInt(value, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_StopLoopSoundOnEntity(scr_entref_t entref)
{
    char *error; // [esp+8h] [ebp-1Ch]
    centity_s *pSelf; // [esp+18h] [ebp-Ch]
    float fadeTime; // [esp+1Ch] [ebp-8h]
    float fadeTimea; // [esp+1Ch] [ebp-8h]

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        2838,
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
    fadeTime = 0.0f;
    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) )
    {
        fadeTimea = Scr_GetFloat(0, SCRIPTINSTANCE_CLIENT);
        if ( fadeTimea < 0.0 || fadeTimea > 32.0 )
        {
            error = va("stoploopsound: invalid fade value %f. it must be between 0 and 32 seconds.", fadeTimea);
            Scr_ParamError(0, error, SCRIPTINSTANCE_CLIENT);
        }
        fadeTime = fadeTimea * -1000.0;
    }
    else
    {
        pSelf->nextState.loopSoundId = 0;
    }
    pSelf->nextState.loopSoundFade = (int)fadeTime;
    CG_PumpEntityLoopSound(entref.client, pSelf);
}

void __cdecl CScr_IsPlayingLoopSound(scr_entref_t entref)
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
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        2869,
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
    Scr_AddBool(pSelf->nextState.loopSoundId != 0, SCRIPTINSTANCE_CLIENT);
}

void CScr_SetSoundContext()
{
    char *v0; // eax
    char *String; // [esp-4h] [ebp-8h]

    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) == 2 )
    {
        String = Scr_GetString(1u, SCRIPTINSTANCE_CLIENT);
        v0 = Scr_GetString(0, SCRIPTINSTANCE_CLIENT);
        SND_SetContext(v0, String);
    }
    else
    {
        Scr_ParamError(0, "setAmbientSnapshot takes two parameters.", SCRIPTINSTANCE_CLIENT);
    }
}

void CScr_SoundLoopEmitter()
{
    int NumParam; // eax
    unsigned int AliasId; // eax
    char *name; // [esp+0h] [ebp-14h]
    char *error; // [esp+4h] [ebp-10h]
    float origin[3]; // [esp+8h] [ebp-Ch] BYREF

    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) == 2 )
    {
        Scr_GetVector(1u, origin, SCRIPTINSTANCE_CLIENT);
        name = Scr_GetString(0, SCRIPTINSTANCE_CLIENT);
        AliasId = SND_FindAliasId(name);
        SND_PlayLoopAt(AliasId, origin);
    }
    else
    {
        NumParam = Scr_GetNumParam(SCRIPTINSTANCE_SERVER);
        error = va("soundloopemitter has %d parameters.    There should be two.", NumParam);
        Scr_Error(SCRIPTINSTANCE_CLIENT, error, 0);
    }
}

void CScr_SoundStopLoopEmitter()
{
    int NumParam; // eax
    unsigned int AliasId; // eax
    char *name; // [esp+0h] [ebp-14h]
    char *error; // [esp+4h] [ebp-10h]
    float origin[3]; // [esp+8h] [ebp-Ch] BYREF

    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) == 2 )
    {
        Scr_GetVector(1u, origin, SCRIPTINSTANCE_CLIENT);
        name = Scr_GetString(0, SCRIPTINSTANCE_CLIENT);
        AliasId = SND_FindAliasId(name);
        SND_StopLoopAt(AliasId, origin);
    }
    else
    {
        NumParam = Scr_GetNumParam(SCRIPTINSTANCE_SERVER);
        error = va("soundstoploopemitter has %d parameters.    There should be two.", NumParam);
        Scr_Error(SCRIPTINSTANCE_CLIENT, error, 0);
    }
}

void CScr_SoundLineEmitter()
{
    int NumParam; // eax
    const char *v1; // eax
    unsigned int AliasId; // eax
    char *name; // [esp+0h] [ebp-1Ch]
    float origin1[3]; // [esp+4h] [ebp-18h] BYREF
    float origin0[3]; // [esp+10h] [ebp-Ch] BYREF

    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) == 3 )
    {
        Scr_GetVector(1u, origin0, SCRIPTINSTANCE_CLIENT);
        Scr_GetVector(2u, origin1, SCRIPTINSTANCE_CLIENT);
        name = Scr_GetString(0, SCRIPTINSTANCE_CLIENT);
        AliasId = SND_FindAliasId(name);
        SND_PlayLineAt(AliasId, origin0, origin1);
    }
    else
    {
        NumParam = Scr_GetNumParam(SCRIPTINSTANCE_SERVER);
        v1 = va("soundlineemitter has %d parameters.    There should be three.", NumParam);
        Scr_Error(v1, 0);
    }
}

void CScr_SoundStopLineEmitter()
{
    int NumParam; // eax
    const char *v1; // eax
    unsigned int AliasId; // eax
    char *name; // [esp+0h] [ebp-1Ch]
    float origin1[3]; // [esp+4h] [ebp-18h] BYREF
    float origin0[3]; // [esp+10h] [ebp-Ch] BYREF

    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) == 3 )
    {
        Scr_GetVector(1u, origin0, SCRIPTINSTANCE_CLIENT);
        Scr_GetVector(2u, origin1, SCRIPTINSTANCE_CLIENT);
        name = Scr_GetString(0, SCRIPTINSTANCE_CLIENT);
        AliasId = SND_FindAliasId(name);
        SND_StopLineAt(AliasId, origin0, origin1);
    }
    else
    {
        NumParam = Scr_GetNumParam(SCRIPTINSTANCE_SERVER);
        v1 = va("soundstoplineemitter has %d parameters.    There should be three.", NumParam);
        Scr_Error(v1, 0);
    }
}

void CScr_GetAliasName()
{
    int NumParam; // eax
    const snd_alias_list_t *v1; // eax
    char *value; // [esp+0h] [ebp-Ch]
    VariableUnion index; // [esp+4h] [ebp-8h]
    char *error; // [esp+8h] [ebp-4h]

    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) == 1 )
    {
        index.intValue = Scr_GetInt(0, SCRIPTINSTANCE_CLIENT);
        v1 = SND_AliasByIndex(index.stringValue);
        value = (char *)SND_GetAliasName(v1);
        Scr_AddString(value, SCRIPTINSTANCE_CLIENT);
    }
    else
    {
        NumParam = Scr_GetNumParam(SCRIPTINSTANCE_SERVER);
        error = va("GetSoundName has %d parameters.    There should be one.", NumParam);
        Scr_Error(SCRIPTINSTANCE_CLIENT, error, 0);
    }
}

void CScr_GetAliasCount()
{
    unsigned int value; // [esp+0h] [ebp-4h]

    value = SND_AliasCount();
    Scr_AddInt(value, SCRIPTINSTANCE_CLIENT);
}

void CScr_IsAliasLoop()
{
    int NumParam; // eax
    bool value; // [esp+0h] [ebp-Ch]
    const char *name; // [esp+4h] [ebp-8h]
    char *error; // [esp+8h] [ebp-4h]

    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) == 1 )
    {
        name = Scr_GetString(0, SCRIPTINSTANCE_CLIENT);
        value = SND_IsAliasNameLooping(name);
        Scr_AddInt(value, SCRIPTINSTANCE_CLIENT);
    }
    else
    {
        NumParam = Scr_GetNumParam(SCRIPTINSTANCE_SERVER);
        error = va("IsAliasLoop has %d parameters.    There should be one.", NumParam);
        Scr_Error(SCRIPTINSTANCE_CLIENT, error, 0);
    }
}

void CScr_StopLocalSound()
{
    cg_s *LocalClientGlobals; // eax
    unsigned int AliasId; // [esp-4h] [ebp-14h]
    char *pAlias; // [esp+8h] [ebp-8h]
    VariableUnion localClientNum; // [esp+Ch] [ebp-4h]

    localClientNum.intValue = CScr_GetLocalClientNum(0);
    pAlias = Scr_GetString(1u, SCRIPTINSTANCE_CLIENT);
    AliasId = SND_FindAliasId(pAlias);
    LocalClientGlobals = CG_GetLocalClientGlobals(localClientNum.intValue);
    CG_StopSoundAlias(localClientNum.intValue, LocalClientGlobals->nextSnap->ps.clientNum, AliasId);
}

void CScr_SoundTimescale()
{
    float value; // [esp+0h] [ebp-4h]

    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) == 1 )
    {
        value = Scr_GetFloat(0, SCRIPTINSTANCE_CLIENT);
        SND_SetScriptTimescale(value);
    }
    else
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "SoundTimescale requires 1 parameter, a float.", 0);
    }
}

void CScr_SoundPlaying()
{
    bool value; // [esp+0h] [ebp-8h]
    VariableUnion playbackId; // [esp+4h] [ebp-4h]

    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) == 1 )
    {
        playbackId.intValue = Scr_GetInt(0, SCRIPTINSTANCE_CLIENT);
        value = SND_IsPlaying(playbackId.intValue);
        Scr_AddBool(value, SCRIPTINSTANCE_CLIENT);
    }
    else
    {
        Scr_ParamError(0, "soundplaying takes one parameter.", SCRIPTINSTANCE_CLIENT);
    }
}

void CScr_SetSoundVolume()
{
    int Int; // eax
    float x; // [esp+24h] [ebp-4h]

    PROF_SCOPED("setsoundvol");

    if (Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) != 2)
    {
        Scr_ParamError(0, "setsoundvolume takes two parameters.", SCRIPTINSTANCE_CLIENT);
        return;
    }
    x = Scr_GetFloat(1u, SCRIPTINSTANCE_CLIENT);
    if ((LODWORD(x) & 0x7F800000) == 0x7F800000)
    {
        Scr_ParamError(0, "setsoundvolume cannot be NAN.", SCRIPTINSTANCE_CLIENT);
    }
    else
    {
        Int = Scr_GetInt(0, SCRIPTINSTANCE_CLIENT);
        SND_SetPlaybackAttenuation(Int, x);
    }
}

void CScr_SetSoundPitch()
{
    int Int; // eax
    float x; // [esp+24h] [ebp-4h]

    PROF_SCOPED("setsoundpitch");

    if (Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) != 2)
    {
        Scr_ParamError(0, "setsoundpitch takes two parameters.", SCRIPTINSTANCE_CLIENT);
        return;
    }
    x = Scr_GetFloat(1u, SCRIPTINSTANCE_CLIENT);
    if ((LODWORD(x) & 0x7F800000) == 0x7F800000)
    {
        Scr_ParamError(0, "setsoundpitch cannot be NAN.", SCRIPTINSTANCE_CLIENT);
    }
    else
    {
        Int = Scr_GetInt(0, SCRIPTINSTANCE_CLIENT);
        SND_SetPlaybackPitch(Int, x);
    }
}

void CScr_SetSoundVolumeRate()
{
    VariableUnion v0; // eax
    float x; // [esp+8h] [ebp-4h]

    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) == 2 )
    {
        x = Scr_GetFloat(1u, SCRIPTINSTANCE_CLIENT);
        if ( (LODWORD(x) & 0x7F800000) == 0x7F800000 )
        {
            Scr_ParamError(0, "setsoundvolumerate cannot be NAN.", SCRIPTINSTANCE_CLIENT);
        }
        else
        {
            v0.intValue = Scr_GetInt(0, SCRIPTINSTANCE_CLIENT);
            SND_SetPlaybackAttenuationRate(v0.stringValue, x);
        }
    }
    else
    {
        Scr_ParamError(0, "setsoundvolumerate takes two parameters.", SCRIPTINSTANCE_CLIENT);
    }
}

void CScr_SetSoundPitchRate()
{
    VariableUnion v0; // eax
    float x; // [esp+8h] [ebp-4h]

    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) == 2 )
    {
        x = Scr_GetFloat(1u, SCRIPTINSTANCE_CLIENT);
        if ( (LODWORD(x) & 0x7F800000) == 0x7F800000 )
        {
            Scr_ParamError(0, "setsoundpitchrate cannot be NAN.", SCRIPTINSTANCE_CLIENT);
        }
        else
        {
            v0.intValue = Scr_GetInt(0, SCRIPTINSTANCE_CLIENT);
            SND_SetPlaybackPitchRate(v0.stringValue, x);
        }
    }
    else
    {
        Scr_ParamError(0, "setsoundpitchrate takes two parameters.", SCRIPTINSTANCE_CLIENT);
    }
}

void CScr_GetRealTime()
{
    unsigned int value; // [esp+0h] [ebp-4h]

    value = Sys_Milliseconds();
    Scr_AddInt(value, SCRIPTINSTANCE_CLIENT);
}

void CScr_GetPlaybackTime()
{
    int value; // [esp+0h] [ebp-8h]
    VariableUnion playbackId; // [esp+4h] [ebp-4h]

    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) == 1 )
    {
        playbackId.intValue = Scr_GetInt(0, SCRIPTINSTANCE_CLIENT);
        value = SND_GetPlaybackTime(playbackId.intValue);
        Scr_AddInt(value, SCRIPTINSTANCE_CLIENT);
    }
    else
    {
        Scr_ParamError(0, "getplaybacktime takes a playback id.", SCRIPTINSTANCE_CLIENT);
    }
}

void CScr_GetKnownLength()
{
    VariableUnion v0; // eax
    int len; // [esp+4h] [ebp-4h] BYREF

    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) == 1 )
    {
        len = 0;
        v0.intValue = Scr_GetInt(0, SCRIPTINSTANCE_CLIENT);
        SND_GetKnownLength(v0.intValue, &len);
        Scr_AddInt(len, SCRIPTINSTANCE_CLIENT);
    }
    else
    {
        Scr_ParamError(0, "GetKnownLength takes a playback id.", SCRIPTINSTANCE_CLIENT);
    }
}

void CScr_SetGroupSnapshot()
{
    char *String; // eax

    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) == 1 )
    {
        String = Scr_GetString(0, SCRIPTINSTANCE_CLIENT);
        SND_SetSnapshot(SND_SNAPSHOT_SCRIPT, String, 0.0, 1.0);
    }
    else
    {
        Scr_ParamError(0, "setGroupSnapshot takes one parameter.", SCRIPTINSTANCE_CLIENT);
    }
}

void CScr_SetAmbientSnapshot()
{
    char *String; // eax

    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) == 1 )
    {
        String = Scr_GetString(0, SCRIPTINSTANCE_CLIENT);
        SND_SetSnapshot(SND_SNAPSHOT_AMBIENT, String, 0.0, 1.0);
    }
    else
    {
        Scr_ParamError(0, "setAmbientSnapshot takes one parameter.", SCRIPTINSTANCE_CLIENT);
    }
}

void CScr_SetHealthSnapshot()
{
    char *String; // eax
    float amount; // [esp+8h] [ebp-Ch]
    float v2; // [esp+Ch] [ebp-8h]
    float Float; // [esp+10h] [ebp-4h]

    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) == 2 )
    {
        Float = Scr_GetFloat(1u, SCRIPTINSTANCE_CLIENT);
        if ( (float)(Float - 1.0) < 0.0 )
            v2 = Float;
        else
            v2 = 1.0f;
        if ( (float)(0.0 - Float) < 0.0 )
            amount = v2;
        else
            amount = 0.0f;
        String = Scr_GetString(0, SCRIPTINSTANCE_CLIENT);
        SND_SetSnapshot(SND_SNAPSHOT_HEALTH, String, 0.0, amount);
    }
    else
    {
        Scr_ParamError(0, "setHealthSnapshot takes two parameters.", SCRIPTINSTANCE_CLIENT);
    }
}

void CScr_SetLevelFadeSnapshot()
{
    char *String; // eax
    float amount; // [esp+8h] [ebp-Ch]
    float v2; // [esp+Ch] [ebp-8h]
    float Float; // [esp+10h] [ebp-4h]

    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) == 2 )
    {
        Float = Scr_GetFloat(1u, SCRIPTINSTANCE_CLIENT);
        if ( (float)(Float - 1.0) < 0.0 )
            v2 = Float;
        else
            v2 = 1.0f;
        if ( (float)(0.0 - Float) < 0.0 )
            amount = v2;
        else
            amount = 0.0f;
        String = Scr_GetString(0, SCRIPTINSTANCE_CLIENT);
        SND_SetSnapshot(SND_SNAPSHOT_LEVELFADE, String, 0.0, amount);
    }
    else
    {
        Scr_ParamError(0, "setLevelFadeSnapshot takes two parameters.", SCRIPTINSTANCE_CLIENT);
    }
}

void __cdecl CScr_StopSound()
{
    VariableUnion v0; // eax

    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) == 1 )
    {
        v0.intValue = Scr_GetInt(0, SCRIPTINSTANCE_CLIENT);
        SND_StopPlayback(v0.stringValue);
    }
    else
    {
        Scr_ParamError(0, "stopsound takes one parameter.", SCRIPTINSTANCE_CLIENT);
    }
}

void __cdecl CScr_ScaleSpeed()
{
    float x1; // [esp+8h] [ebp-1Ch]
    float z; // [esp+Ch] [ebp-18h]
    float y1; // [esp+10h] [ebp-14h]
    float x2; // [esp+18h] [ebp-Ch]
    float y2; // [esp+1Ch] [ebp-8h]

    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) == 5 )
    {
        x1 = Scr_GetFloat(0, SCRIPTINSTANCE_CLIENT);
        x2 = Scr_GetFloat(1u, SCRIPTINSTANCE_CLIENT);
        y1 = Scr_GetFloat(2u, SCRIPTINSTANCE_CLIENT);
        y2 = Scr_GetFloat(3u, SCRIPTINSTANCE_CLIENT);
        z = Scr_GetFloat(4u, SCRIPTINSTANCE_CLIENT);
        if ( x1 > z )
            z = x1;
        if ( z > x2 )
            z = x2;
        Scr_AddFloat((float)((float)(y2 - y1) * (float)((float)(z - x1) / (float)(x2 - x1))) + y1, SCRIPTINSTANCE_CLIENT);
    }
    else
    {
        Scr_ParamError(0, "scale speed takes 5 parameters.", SCRIPTINSTANCE_CLIENT);
    }
}

void CScr_DeactivateReverb()
{
    int NumParam; // [esp+4h] [ebp-24h]
    int v1; // [esp+8h] [ebp-20h]
    float fadetime; // [esp+1Ch] [ebp-Ch]
    unsigned int priority; // [esp+20h] [ebp-8h]
    char *prio_name; // [esp+24h] [ebp-4h]

    priority = 0;
    fadetime = 0.0f;
    NumParam = Scr_GetNumParam(SCRIPTINSTANCE_CLIENT);
    if ( NumParam == 1 )
        goto LABEL_4;
    if ( NumParam == 2 )
    {
        fadetime = Scr_GetFloat(1u, SCRIPTINSTANCE_CLIENT);
LABEL_4:
        prio_name = Scr_GetString(0, SCRIPTINSTANCE_CLIENT);
        priority = 1;
        if ( I_stricmp(prio_name, "snd_enveffectsprio_level") )
        {
            if ( I_stricmp(prio_name, "snd_enveffectsprio_shellshock") )
                Scr_Error(
                    SCRIPTINSTANCE_CLIENT,
                    "priority must be 'snd_enveffectsprio_level' or 'snd_enveffectsprio_shellshock'\n",
                    0);
            else
                priority = 2;
        }
        else
        {
            priority = 1;
        }
        goto LABEL_10;
    }
    Scr_Error(SCRIPTINSTANCE_CLIENT, "Incorrect number of params.", 0);
LABEL_10:
    v1 = (int)((float)(1000.0 * fadetime) + 9.313225746154785e-10);
    if ( v1 > 0 )
        SND_DeactivateEnvironmentEffects(priority, v1);
    else
        SND_DeactivateEnvironmentEffects(priority, 0);
}

void CScr_SetReverb()
{
    int fademsec; // [esp+Ch] [ebp-34h]
    float drylevel; // [esp+28h] [ebp-18h]
    float fadetime; // [esp+2Ch] [ebp-14h]
    float wetlevel; // [esp+30h] [ebp-10h]
    const char *pszReverb; // [esp+34h] [ebp-Ch]
    const char *prio_name; // [esp+38h] [ebp-8h]
    int prio; // [esp+3Ch] [ebp-4h]

    fadetime = 0.0f;
    drylevel = 1.0f;
    wetlevel = 0.5f;
    switch ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) )
    {
        case 2u:
            goto $LN6_23;
        case 3u:
            goto $LN7_19;
        case 4u:
            goto $LN8_17;
        case 5u:
            fadetime = Scr_GetFloat(4u, SCRIPTINSTANCE_CLIENT);
$LN8_17:
            wetlevel = Scr_GetFloat(3u, SCRIPTINSTANCE_CLIENT);
$LN7_19:
            drylevel = Scr_GetFloat(2u, SCRIPTINSTANCE_CLIENT);
$LN6_23:
            pszReverb = Scr_GetString(1u, SCRIPTINSTANCE_CLIENT);
            prio_name = Scr_GetString(0, SCRIPTINSTANCE_CLIENT);
            prio = 1;
            if ( I_stricmp(prio_name, "snd_enveffectsprio_level") )
            {
                if ( I_stricmp(prio_name, "snd_enveffectsprio_shellshock") )
                    Scr_Error(
                        SCRIPTINSTANCE_CLIENT,
                        "priority must be 'snd_enveffectsprio_level' or 'snd_enveffectsprio_shellshock'\n",
                        0);
                else
                    prio = 2;
            }
            else
            {
                prio = 1;
            }
            if ( (int)((float)(1000.0 * fadetime) + 9.313225746154785e-10) > 0 )
                fademsec = (int)((float)(1000.0 * fadetime) + 9.313225746154785e-10);
            else
                fademsec = 0;
            SND_SetEnvironmentEffects(prio, pszReverb, drylevel, wetlevel, fademsec);
            break;
        default:
            Scr_Error(SCRIPTINSTANCE_CLIENT, "Incorrect number of parameters\n", 0);
            break;
    }
}

void CScr_GetArrayKeys()
{
    char *error; // [esp+0h] [ebp-Ch]
    const char *TypeName; // [esp+4h] [ebp-8h]
    VariableUnion id; // [esp+8h] [ebp-4h]

    if ( Scr_GetPointerType(0, SCRIPTINSTANCE_CLIENT) != 20 )
    {
        TypeName = Scr_GetTypeName(0, SCRIPTINSTANCE_CLIENT);
        error = va("Parameter (%s) must be an array", TypeName);
        Scr_ParamError(0, error, SCRIPTINSTANCE_CLIENT);
    }
    id.intValue = Scr_GetObject(0, SCRIPTINSTANCE_CLIENT);
    Scr_AddArrayKeys(id.stringValue, SCRIPTINSTANCE_CLIENT);
}

void CScr_GetLocalClientPos()
{
    int localClientNum; // [esp+8h] [ebp-8h]
    const playerState_s *ps; // [esp+Ch] [ebp-4h]

    localClientNum = CScr_GetLocalClientNum(0);
    if ( (unsigned int)localClientNum >= 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                    3349,
                    0,
                    "localClientNum not in [0, MAX_LOCAL_CLIENTS]\n\t%i not in [%i, %i]",
                    localClientNum,
                    0,
                    1) )
    {
        __debugbreak();
    }
    if ( !CL_IsLocalClientInGame(localClientNum)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                    3350,
                    0,
                    "%s",
                    "CL_IsLocalClientInGame( localClientNum ) == qtrue") )
    {
        __debugbreak();
    }
    ps = CG_GetPredictedPlayerState(localClientNum);
    Scr_AddVector((float*)ps->origin, SCRIPTINSTANCE_CLIENT);
}

void CScr_AimingAtFriendly()
{
    int localClientNum; // [esp+Ch] [ebp-8h]
    const playerState_s *ps; // [esp+10h] [ebp-4h]

    localClientNum = CScr_GetLocalClientNum(0);
    if ( (unsigned int)localClientNum >= 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                    3370,
                    0,
                    "localClientNum not in [0, MAX_LOCAL_CLIENTS]\n\t%i not in [%i, %i]",
                    localClientNum,
                    0,
                    1) )
    {
        __debugbreak();
    }
    if ( !CL_IsLocalClientInGame(localClientNum)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                    3371,
                    0,
                    "%s",
                    "CL_IsLocalClientInGame( localClientNum ) == qtrue") )
    {
        __debugbreak();
    }
    ps = CG_GetPredictedPlayerState(localClientNum);
    Scr_AddBool(ps->weapFlags & 8, SCRIPTINSTANCE_CLIENT);
}

void CScr_SplitArgs()
{
    char *v0; // eax
    char *v1; // eax
    char str[1028]; // [esp+0h] [ebp-418h] BYREF
    int pos; // [esp+408h] [ebp-10h]
    const char *pArg; // [esp+40Ch] [ebp-Ch]
    char *pStr; // [esp+410h] [ebp-8h]
    char *pSearch; // [esp+414h] [ebp-4h]

    pStr = str;
    pArg = Scr_GetString(0, SCRIPTINSTANCE_CLIENT);
    I_strncpyz(str, pArg, 1024);
    Scr_MakeArray(SCRIPTINSTANCE_CLIENT);
    v0 = strstr(str, " ");
    pSearch = v0;
    pos = 0;
    while ( pSearch )
    {
        pos = pSearch - pStr;
        *pSearch = 0;
        Scr_AddString(pStr, SCRIPTINSTANCE_CLIENT);
        Scr_AddArray(SCRIPTINSTANCE_CLIENT);
        pStr += pos + 1;
        v1 = strstr(pStr, " ");
        pSearch = v1;
    }
    if ( pStr )
    {
        Scr_AddString(pStr, SCRIPTINSTANCE_CLIENT);
        Scr_AddArray(SCRIPTINSTANCE_CLIENT);
    }
}

void CScr_sin()
{
    float value; // xmm0_8
    long double v1; // [esp+8h] [ebp-8h]

    //*((float *)&v1 + 1) = Scr_GetFloat(0, SCRIPTINSTANCE_CLIENT);
    //value = (float)(*((float *)&v1 + 1) * 0.017453292);
    value = (float)(Scr_GetFloat(0, SCRIPTINSTANCE_CLIENT) * 0.017453292);
    //__libm_sse2_sin(v1);
    //*(float *)&value = value;
    value = sin(value);
    Scr_AddFloat(value, SCRIPTINSTANCE_CLIENT);
}

void CScr_cos()
{
    float value; // xmm0_8
    long double v1; // [esp+8h] [ebp-8h]

    //*((float *)&v1 + 1) = Scr_GetFloat(0, SCRIPTINSTANCE_CLIENT);
    //value = (float)(*((float *)&v1 + 1) * 0.017453292);
    value = (float)(Scr_GetFloat(0, SCRIPTINSTANCE_CLIENT) * 0.017453292);
    //__libm_sse2_cos(v1);
    //*(float *)&value = value;
    value = cos(value);
    Scr_AddFloat(value, SCRIPTINSTANCE_CLIENT);
}

void CScr_tan()
{
    float v0; // [esp+14h] [ebp-Ch]
    float sinT; // [esp+18h] [ebp-8h]
    float cosT; // [esp+1Ch] [ebp-4h]

    v0 = Scr_GetFloat(0, SCRIPTINSTANCE_CLIENT) * 0.017453292;
    cosT = cos(v0);
    sinT = sin(v0);
    if ( cosT == 0.0 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "divide by 0", 0);
    Scr_AddFloat(sinT / cosT, SCRIPTINSTANCE_CLIENT);
}

void CScr_asin()
{
    long double v0; // [esp+8h] [ebp-Ch]
    float x; // [esp+10h] [ebp-4h]

    x = Scr_GetFloat(0, SCRIPTINSTANCE_CLIENT);
    if ( x < -1.0 || x > 1.0 )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, va("%g out of range", x), 0);
    }
    //__libm_sse2_asin(v0);
    Scr_AddFloat(asin(x) * 57.295776, SCRIPTINSTANCE_CLIENT);
}

void CScr_acos()
{
    long double v0; // [esp+8h] [ebp-Ch]
    float x; // [esp+10h] [ebp-4h]

    x = Scr_GetFloat(0, SCRIPTINSTANCE_CLIENT);
    if ( x < -1.0 || x > 1.0 )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, va("%g out of range", x), 0);
    }
    //__libm_sse2_acos(v0);
    Scr_AddFloat(acos(x) * 57.295776, SCRIPTINSTANCE_CLIENT);
}

void CScr_atan()
{
    double v0; // xmm0_8
    long double v1; // [esp+8h] [ebp-8h]

    //*((float *)&v1 + 1) = Scr_GetFloat(0, SCRIPTINSTANCE_CLIENT);
    //v0 = *((float *)&v1 + 1);
    //__libm_sse2_atan(v1);
    //*(float *)&v0 = v0;
    //Scr_AddFloat(*(float *)&v0 * 57.295776, SCRIPTINSTANCE_CLIENT);
    Scr_AddFloat(atan(Scr_GetFloat(0, SCRIPTINSTANCE_CLIENT)) * 57.295776, SCRIPTINSTANCE_CLIENT);
}

void CScr_abs()
{
    float Float; // [esp+Ch] [ebp-4h]

    Float = Scr_GetFloat(0, SCRIPTINSTANCE_CLIENT);
    Scr_AddFloat(fabs(Float), SCRIPTINSTANCE_CLIENT);
}

void CScr_min()
{
    float value; // [esp+8h] [ebp-Ch]
    float v1; // [esp+Ch] [ebp-8h]
    float Float; // [esp+10h] [ebp-4h]

    Float = Scr_GetFloat(1u, SCRIPTINSTANCE_CLIENT);
    v1 = Scr_GetFloat(0, SCRIPTINSTANCE_CLIENT);
    if ( (float)(Float - v1) < 0.0 )
        value = Float;
    else
        value = v1;
    Scr_AddFloat(value, SCRIPTINSTANCE_CLIENT);
}

void CScr_max()
{
    float value; // [esp+8h] [ebp-Ch]
    float v1; // [esp+Ch] [ebp-8h]
    float Float; // [esp+10h] [ebp-4h]

    Float = Scr_GetFloat(1u, SCRIPTINSTANCE_CLIENT);
    v1 = Scr_GetFloat(0, SCRIPTINSTANCE_CLIENT);
    if ( (float)(v1 - Float) < 0.0 )
        value = Float;
    else
        value = v1;
    Scr_AddFloat(value, SCRIPTINSTANCE_CLIENT);
}

void CScr_floor()
{
    float v0; // [esp+8h] [ebp-Ch]
    float Float; // [esp+10h] [ebp-4h]

    Float = Scr_GetFloat(0, SCRIPTINSTANCE_CLIENT);
    v0 = floor(Float);
    Scr_AddFloat(v0, SCRIPTINSTANCE_CLIENT);
}

void CScr_ceil()
{
    float v0; // [esp+8h] [ebp-Ch]
    float Float; // [esp+10h] [ebp-4h]

    Float = Scr_GetFloat(0, SCRIPTINSTANCE_CLIENT);
    v0 = ceil(Float);
    Scr_AddFloat(v0, SCRIPTINSTANCE_CLIENT);
}

void CScr_sqrt()
{
    float Float; // [esp+Ch] [ebp-4h]

    Float = Scr_GetFloat(0, SCRIPTINSTANCE_CLIENT);
    Scr_AddFloat(sqrtf(Float), SCRIPTINSTANCE_CLIENT);
}

void CScr_CastInt()
{
    int Type; // [esp+0h] [ebp-1Ch]
    char *error; // [esp+4h] [ebp-18h]
    const char *TypeName; // [esp+8h] [ebp-14h]
    int v3; // [esp+Ch] [ebp-10h]
    const char *nptr; // [esp+10h] [ebp-Ch]
    int Float; // [esp+14h] [ebp-8h]
    VariableUnion value; // [esp+18h] [ebp-4h]

    Type = Scr_GetType(0, SCRIPTINSTANCE_CLIENT);
    switch ( Type )
    {
        case 2:
            nptr = Scr_GetString(0, SCRIPTINSTANCE_CLIENT);
            v3 = atoi(nptr);
            Scr_AddInt(v3, SCRIPTINSTANCE_CLIENT);
            break;
        case 5:
            Float = (int)Scr_GetFloat(0, SCRIPTINSTANCE_CLIENT);
            Scr_AddInt(Float, SCRIPTINSTANCE_CLIENT);
            break;
        case 6:
            value.intValue = Scr_GetInt(0, SCRIPTINSTANCE_CLIENT);
            Scr_AddInt(value.intValue, SCRIPTINSTANCE_CLIENT);
            break;
        default:
            TypeName = Scr_GetTypeName(0, SCRIPTINSTANCE_CLIENT);
            error = va("cannot cast %s to int", TypeName);
            Scr_ParamError(0, error, SCRIPTINSTANCE_CLIENT);
            break;
    }
}

void CScr_CastFloat()
{
    int Type; // [esp+8h] [ebp-1Ch]
    char *error; // [esp+Ch] [ebp-18h]
    const char *TypeName; // [esp+10h] [ebp-14h]
    float v3; // [esp+14h] [ebp-10h]
    const char *nptr; // [esp+18h] [ebp-Ch]
    float Float; // [esp+1Ch] [ebp-8h]
    float value; // [esp+20h] [ebp-4h]

    Type = Scr_GetType(0, SCRIPTINSTANCE_CLIENT);
    switch ( Type )
    {
        case 2:
            nptr = Scr_GetString(0, SCRIPTINSTANCE_CLIENT);
            v3 = atof(nptr);
            Scr_AddFloat(v3, SCRIPTINSTANCE_CLIENT);
            break;
        case 5:
            Float = Scr_GetFloat(0, SCRIPTINSTANCE_CLIENT);
            Scr_AddFloat(Float, SCRIPTINSTANCE_CLIENT);
            break;
        case 6:
            value = (float)Scr_GetInt(0, SCRIPTINSTANCE_CLIENT);
            Scr_AddFloat(value, SCRIPTINSTANCE_CLIENT);
            break;
        default:
            TypeName = Scr_GetTypeName(0, SCRIPTINSTANCE_CLIENT);
            error = va("cannot cast %s to float", TypeName);
            Scr_ParamError(0, error, SCRIPTINSTANCE_CLIENT);
            break;
    }
}

void CScr_GetLocalClientEyePos()
{
    float origin[3]; // [esp+1Ch] [ebp-14h] BYREF
    int localClientNum; // [esp+28h] [ebp-8h]
    const playerState_s *ps; // [esp+2Ch] [ebp-4h]

    localClientNum = CScr_GetLocalClientNum(0);
    if ( (unsigned int)localClientNum >= 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                    3638,
                    0,
                    "localClientNum not in [0, MAX_LOCAL_CLIENTS]\n\t%i not in [%i, %i]",
                    localClientNum,
                    0,
                    1) )
    {
        __debugbreak();
    }
    if ( !CL_IsLocalClientInGame(localClientNum)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                    3639,
                    0,
                    "%s",
                    "CL_IsLocalClientInGame( localClientNum ) == qtrue") )
    {
        __debugbreak();
    }
    ps = CG_GetPredictedPlayerState(localClientNum);
    origin[0] = ps->origin[0];
    origin[1] = ps->origin[1];
    origin[2] = ps->origin[2];
    origin[2] = origin[2] + ps->viewHeightCurrent;
    AddLeanToPosition(origin, ps->viewangles[1], ps->leanf, 16.0, 20.0);
    Scr_AddVector(origin, SCRIPTINSTANCE_CLIENT);
}

void CScr_GetLocalClientAngles()
{
    int localClientNum; // [esp+8h] [ebp-8h]
    const playerState_s *ps; // [esp+Ch] [ebp-4h]

    localClientNum = CScr_GetLocalClientNum(0);
    if ( (unsigned int)localClientNum >= 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                    3655,
                    0,
                    "localClientNum not in [0, MAX_LOCAL_CLIENTS]\n\t%i not in [%i, %i]",
                    localClientNum,
                    0,
                    1) )
    {
        __debugbreak();
    }
    if ( !CL_IsLocalClientInGame(localClientNum)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                    3656,
                    0,
                    "%s",
                    "CL_IsLocalClientInGame( localClientNum ) == qtrue") )
    {
        __debugbreak();
    }
    ps = CG_GetPredictedPlayerState(localClientNum);
    Scr_AddVector((float*)ps->viewangles, SCRIPTINSTANCE_CLIENT);
}

void CScr_Spawn()
{
    float *v0; // [esp+10h] [ebp-28h]
    const char *entClass; // [esp+1Ch] [ebp-1Ch]
    float origin[3]; // [esp+24h] [ebp-14h] BYREF
    int localClientNum; // [esp+30h] [ebp-8h]
    centity_s *ent; // [esp+34h] [ebp-4h]

    PROF_SCOPED("spawn");

    localClientNum = CScr_GetLocalClientNum(0);
    Scr_GetVector(1u, origin, SCRIPTINSTANCE_CLIENT);
    ent = CG_Spawn(localClientNum);
    if (!ent)
    {
        return;
    }
    v0 = ent->pose.origin;
    ent->pose.origin[0] = origin[0];
    v0[1] = origin[1];
    v0[2] = origin[2];
    if (Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) >= 3)
    {
        entClass = Scr_GetString(2u, SCRIPTINSTANCE_CLIENT);
        if (!I_stricmp(entClass, "script_model") || !I_stricmp(entClass, "script_origin"))
            CG_InitScriptMover(ent);
    }
    CScr_AddEntity(ent, localClientNum);
}

void CScr_SpawnPlane()
{
    Material *v0; // eax
    float *v1; // [esp+28h] [ebp-3Ch]
    int modelIndex; // [esp+34h] [ebp-30h]
    int modelIndexa; // [esp+34h] [ebp-30h]
    float origin[3]; // [esp+38h] [ebp-2Ch] BYREF
    const char *team; // [esp+44h] [ebp-20h]
    int owner; // [esp+48h] [ebp-1Ch]
    const char *compassMaterialName; // [esp+4Ch] [ebp-18h]
    int localClientNum; // [esp+50h] [ebp-14h]
    const char *modelName; // [esp+54h] [ebp-10h]
    const char *error; // [esp+58h] [ebp-Ch]
    centity_s *ent; // [esp+5Ch] [ebp-8h]
    team_t teamEnum; // [esp+60h] [ebp-4h]

    error = "illegal call to spawnplane()\n";
    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) != 6 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, error, 0);
    localClientNum = CScr_GetLocalClientNum(0);
    Scr_GetVector(1u, origin, SCRIPTINSTANCE_CLIENT);
    modelName = Scr_GetString(2u, SCRIPTINSTANCE_CLIENT);
    team = Scr_GetString(3u, SCRIPTINSTANCE_CLIENT);
    owner = Scr_GetInt(4u, SCRIPTINSTANCE_CLIENT);
    compassMaterialName = Scr_GetString(5u, SCRIPTINSTANCE_CLIENT);
    ent = CG_Spawn(localClientNum);
    if ( ent )
    {
        v1 = ent->pose.origin;
        ent->pose.origin[0] = origin[0];
        v1[1] = origin[1];
        v1[2] = origin[2];
        CG_InitScriptMover(ent);
        if ( *modelName )
        {
            if ( *modelName == 42 )
            {
                modelIndex = atoi(modelName + 1);
                if ( modelIndex != (unsigned __int16)modelIndex
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                                3760,
                                0,
                                "%s",
                                "modelIndex == (modelNameIndex_t)modelIndex") )
                {
                    __debugbreak();
                }
                ent->nextState.index.brushmodel = modelIndex;
            }
            else
            {
                modelIndexa = CG_GetModelIndex(modelName, localClientNum);
                if ( modelIndexa < 0
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                                3767,
                                0,
                                "%s",
                                "modelIndex >= 0") )
                {
                    __debugbreak();
                }
                if ( modelIndexa != (unsigned __int16)modelIndexa
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                                3768,
                                0,
                                "%s",
                                "modelIndex == (modelNameIndex_t)modelIndex") )
                {
                    __debugbreak();
                }
                ent->nextState.index.brushmodel = modelIndexa;
                *((unsigned int *)ent + 201) |= 2u;
            }
        }
        if ( !strcmp(team, "free") )
        {
            teamEnum = TEAM_FREE;
        }
        else if ( !strcmp(team, "axis") )
        {
            teamEnum = TEAM_AXIS;
        }
        else
        {
            teamEnum = TEAM_ALLIES;
        }
        AssignToSmallerType<unsigned char>(&ent->nextState.faction.iHeadIconTeam, teamEnum | (4 * owner));
        *((unsigned int *)ent + 201) |= 0x40000u;
        v0 = Material_Register((char *)compassMaterialName, 7);
        ent->compassMaterial = v0;
        CScr_AddEntity(ent, localClientNum);
    }
}

void __cdecl CScr_IsAlive()
{
    scr_entref_t v0; // [esp+8h] [ebp-1Ch] BYREF
    scr_entref_t v1; // [esp+12h] [ebp-12h]
    scr_entref_t entref; // [esp+18h] [ebp-Ch]
    centity_s *pSelf; // [esp+20h] [ebp-4h]

    v1 = Scr_GetEntityRef(0, SCRIPTINSTANCE_CLIENT);
    entref = v1;
    if ( v1.classnum )
    {
        Scr_ParamError(0, "IsAlive param isn't an entity.", SCRIPTINSTANCE_CLIENT);
    }
    else
    {
        pSelf = CG_GetEntity(entref.client, entref.entnum);
        if ( !pSelf
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp", 3824, 0, "%s", "pSelf") )
        {
            __debugbreak();
        }
        if ( pSelf->nextState.eType != 17
            && pSelf->nextState.eType != 14
            && pSelf->nextState.eType != 16
            && pSelf->nextState.eType != 12
            && pSelf->nextState.eType != 13
            && pSelf->nextState.eType != 1 )
        {
            Scr_Error(SCRIPTINSTANCE_CLIENT, "IsAlive called on the wrong kind of entity", 0);
        }
        if ( (pSelf->nextState.eType == 17
             || pSelf->nextState.eType == 14
             || pSelf->nextState.eType == 16
             || pSelf->nextState.eType == 12
             || pSelf->nextState.eType == 13
             || pSelf->nextState.eType == 1)
            && (pSelf->nextState.lerp.eFlags & 0x40000) == 0 )
        {
            Scr_AddInt(1, SCRIPTINSTANCE_CLIENT);
        }
        else
        {
            Scr_AddInt(0, SCRIPTINSTANCE_CLIENT);
        }
    }
}

void CScr_IsSubStr()
{
    char *v0; // eax
    char *str1; // [esp+4h] [ebp-8h]
    char *str2; // [esp+8h] [ebp-4h]

    str2 = Scr_GetString(1u, SCRIPTINSTANCE_CLIENT);
    str1 = Scr_GetString(0, SCRIPTINSTANCE_CLIENT);
    v0 = strstr(str1, str2);
    Scr_AddBool(v0 != 0, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_GetSubStr()
{
    VariableUnion v0; // [esp+0h] [ebp-424h]
    int source; // [esp+4h] [ebp-420h]
    char c; // [esp+Bh] [ebp-419h]
    char tempString[1028]; // [esp+Ch] [ebp-418h] BYREF
    int start; // [esp+414h] [ebp-10h]
    int end; // [esp+418h] [ebp-Ch]
    int dest; // [esp+41Ch] [ebp-8h]
    const char *s; // [esp+420h] [ebp-4h]

    s = Scr_GetString(0, SCRIPTINSTANCE_CLIENT);
    start = Scr_GetInt(1u, SCRIPTINSTANCE_CLIENT);
    if ( (unsigned int)Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) < 3 )
        v0.intValue = 0x7FFFFFFF;
    else
        v0.intValue = Scr_GetInt(2u, SCRIPTINSTANCE_CLIENT);
    end = v0.intValue;
    source = start;
    for ( dest = 0; source < end; ++dest )
    {
        if ( dest >= 1024 )
            Scr_Error(SCRIPTINSTANCE_CLIENT, "string too long", 0);
        c = s[source];
        if ( !c )
            break;
        tempString[dest] = c;
        ++source;
    }
    tempString[dest] = 0;
    Scr_AddString(tempString, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_StrTok()
{
    int source; // [esp+10h] [ebp-42Ch]
    char c; // [esp+17h] [ebp-425h]
    unsigned int delimId; // [esp+18h] [ebp-424h]
    char tempString[1028]; // [esp+1Ch] [ebp-420h] BYREF
    const char *delim; // [esp+424h] [ebp-18h]
    int dest; // [esp+428h] [ebp-14h]
    const char *s; // [esp+42Ch] [ebp-10h]
    int i; // [esp+430h] [ebp-Ch]
    int delimLen; // [esp+434h] [ebp-8h]
    unsigned int sId; // [esp+438h] [ebp-4h]

    sId = Scr_GetConstString(0, SCRIPTINSTANCE_CLIENT);
    delimId = Scr_GetConstString(1u, SCRIPTINSTANCE_CLIENT);
    s = SL_ConvertToString(sId, SCRIPTINSTANCE_CLIENT);
    delim = SL_ConvertToString(delimId, SCRIPTINSTANCE_CLIENT);
    SL_AddRefToString(sId, SCRIPTINSTANCE_CLIENT);
    SL_AddRefToString(delimId, SCRIPTINSTANCE_CLIENT);
    delimLen = strlen(delim);
    dest = 0;
    Scr_MakeArray(SCRIPTINSTANCE_CLIENT);
    for ( source = 0; ; ++source )
    {
        c = s[source];
        if ( !c )
            break;
        for ( i = 0; i < delimLen; ++i )
        {
            if ( c == delim[i] )
            {
                if ( dest )
                {
                    tempString[dest] = 0;
                    Scr_AddString(tempString, SCRIPTINSTANCE_CLIENT);
                    Scr_AddArray(SCRIPTINSTANCE_CLIENT);
                    dest = 0;
                }
                goto LABEL_2;
            }
        }
        tempString[dest++] = c;
        if ( dest >= 1024 )
        {
            SL_RemoveRefToString(SCRIPTINSTANCE_CLIENT, sId);
            SL_RemoveRefToString(SCRIPTINSTANCE_CLIENT, delimId);
            Scr_Error(SCRIPTINSTANCE_CLIENT, "string too long", 0);
        }
LABEL_2:
        ;
    }
    if ( dest )
    {
        tempString[dest] = 0;
        Scr_AddString(tempString, SCRIPTINSTANCE_CLIENT);
        Scr_AddArray(SCRIPTINSTANCE_CLIENT);
        dest = 0;
    }
    SL_RemoveRefToString(SCRIPTINSTANCE_CLIENT, sId);
    SL_RemoveRefToString(SCRIPTINSTANCE_CLIENT, delimId);
}

void __cdecl CScr_GetSpecies(centity_s *cent, const cent_field_s *pField)
{
    if ( !cent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp", 3980, 0, "%s", "cent") )
        __debugbreak();
    if ( !pField
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp", 3981, 0, "%s", "pField") )
    {
        __debugbreak();
    }
    switch ( cent->nextState.eType )
    {
        case 1:
        case 2:
            Scr_AddString("human", SCRIPTINSTANCE_CLIENT);
            break;
        case 0x11:
        case 0x13:
            if ( cent->nextState.lerp.u.loopFx.period )
                Scr_AddString("unknown", SCRIPTINSTANCE_CLIENT);
            else
                Scr_AddString("dog", SCRIPTINSTANCE_CLIENT);
            break;
        default:
            Scr_Error(SCRIPTINSTANCE_CLIENT, ".species call on invalid ent type.", 0);
            break;
    }
}

void __cdecl CScr_GetIsDog(centity_s *cent, const cent_field_s *pField)
{
    unsigned int value; // [esp+0h] [ebp-4h]

    if ( !cent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp", 4021, 0, "%s", "cent") )
        __debugbreak();
    if ( !pField
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp", 4022, 0, "%s", "pField") )
    {
        __debugbreak();
    }
    if ( cent->nextState.eType == 17 || cent->nextState.eType == 19 )
    {
        value = Flame_GetLocalClientSourceRange();
        Scr_AddBool(value, SCRIPTINSTANCE_CLIENT);
    }
    else
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, ".isdog called on ent that is not actor or corpse.", 0);
        Scr_AddBool(0, SCRIPTINSTANCE_CLIENT);
    }
}

unsigned int __cdecl GetFootstepString(int event)
{
    unsigned int result; // eax

    switch ( event )
    {
        case 'g':
            result = cscr_const.sprint;
            break;
        case 'h':
            result = cscr_const.run;
            break;
        case 'i':
            result = cscr_const.walk;
            break;
        case 'j':
            result = cscr_const.crouchrun;
            break;
        case 'k':
            result = cscr_const.crouchwalk;
            break;
        case 'l':
            result = cscr_const.prone;
            break;
        default:
            result = cscr_const.run;
            break;
    }
    return result;
}

void __cdecl ReportBadFootsteps(int localClientNum, centity_s *cent, int surfaceType, bool isPlayerView)
{
    if ( isPlayerView && !surfaceType )
        CG_GetLocalClientGlobals(localClientNum);
}

void __cdecl CScr_PlayerFootstepEvent(
                int localClientNum,
                centity_s *cent,
                bool isPlayerView,
                int event,
                int surfaceType,
                bool quiet)
{
    unsigned int FootstepString; // [esp+0h] [ebp-Ch]
    char *value; // [esp+4h] [ebp-8h]
    unsigned __int16 t; // [esp+8h] [ebp-4h]

    ReportBadFootsteps(localClientNum, cent, surfaceType, isPlayerView);
    Scr_AddInt(quiet, SCRIPTINSTANCE_CLIENT);
    Scr_AddInt(isPlayerView, SCRIPTINSTANCE_CLIENT);
    value = (char *)Com_SurfaceTypeToName(surfaceType);
    Scr_AddString(value, SCRIPTINSTANCE_CLIENT);
    FootstepString = GetFootstepString(event);
    Scr_AddConstString(FootstepString, SCRIPTINSTANCE_CLIENT);
    CScr_AddEntity(cent, (unsigned __int16)localClientNum);
    Scr_AddInt(localClientNum, SCRIPTINSTANCE_CLIENT);
    t = Scr_ExecThread(SCRIPTINSTANCE_CLIENT, cg_scr_data.playerFootstep, 6u);
    Scr_FreeThread(t, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_PlayerJumpEvent(int localClientNum, centity_s *cent, bool isPlayerView, int surfaceType, bool quiet)
{
    char *value; // [esp+0h] [ebp-8h]
    unsigned __int16 t; // [esp+4h] [ebp-4h]

    Scr_AddInt(quiet, SCRIPTINSTANCE_CLIENT);
    Scr_AddInt(isPlayerView, SCRIPTINSTANCE_CLIENT);
    value = (char *)Com_SurfaceTypeToName(surfaceType);
    Scr_AddString(value, SCRIPTINSTANCE_CLIENT);
    CScr_AddEntity(cent, (unsigned __int16)localClientNum);
    Scr_AddInt(localClientNum, SCRIPTINSTANCE_CLIENT);
    t = Scr_ExecThread(SCRIPTINSTANCE_CLIENT, cg_scr_data.playerJump, 5u);
    Scr_FreeThread(t, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_PlayerLandEvent(
                int localClientNum,
                centity_s *cent,
                bool isPlayerView,
                int surfaceType,
                bool quiet,
                bool damagePlayer)
{
    char *value; // [esp+0h] [ebp-8h]
    unsigned __int16 t; // [esp+4h] [ebp-4h]

    ReportBadFootsteps(localClientNum, cent, surfaceType, isPlayerView);
    Scr_AddInt(damagePlayer, SCRIPTINSTANCE_CLIENT);
    Scr_AddInt(quiet, SCRIPTINSTANCE_CLIENT);
    Scr_AddInt(isPlayerView, SCRIPTINSTANCE_CLIENT);
    value = (char *)Com_SurfaceTypeToName(surfaceType);
    Scr_AddString(value, SCRIPTINSTANCE_CLIENT);
    CScr_AddEntity(cent, (unsigned __int16)localClientNum);
    Scr_AddInt(localClientNum, SCRIPTINSTANCE_CLIENT);
    t = Scr_ExecThread(SCRIPTINSTANCE_CLIENT, cg_scr_data.playerLand, 6u);
    Scr_FreeThread(t, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_PlayerFoliageEvent(int localClientNum, centity_s *cent, bool isPlayerView, bool quiet)
{
    unsigned __int16 t; // [esp+0h] [ebp-4h]

    Scr_AddInt(quiet, SCRIPTINSTANCE_CLIENT);
    Scr_AddInt(isPlayerView, SCRIPTINSTANCE_CLIENT);
    CScr_AddEntity(cent, (unsigned __int16)localClientNum);
    Scr_AddInt(localClientNum, SCRIPTINSTANCE_CLIENT);
    t = Scr_ExecThread(SCRIPTINSTANCE_CLIENT, cg_scr_data.playerFoliage, 4u);
    Scr_FreeThread(t, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScrCmd_GetThrottle(scr_entref_t entref)
{
    centity_s *pSelf; // [esp+1Ch] [ebp-8h]

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        4140,
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
    if ( pSelf->nextState.eType != 14 && pSelf->nextState.eType != 16 && pSelf->nextState.eType != 12 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "GetThrottle not called on a vehicle.", 0);
    if ( !pSelf->vehicle
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp", 4145, 0, "%s", "pSelf->vehicle") )
    {
        __debugbreak();
    }
    if ( pSelf->nitrousVeh )
        Scr_AddFloat(pSelf->nitrousVeh->m_throttle, SCRIPTINSTANCE_CLIENT);
    else
        Scr_AddFloat((float)pSelf->nextState.lerp.u.vehicle.throttle * 0.000030517578, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScrCmd_GetBrake(scr_entref_t entref)
{
    centity_s *pSelf; // [esp+1Ch] [ebp-8h]

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        4168,
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
    if ( pSelf->nextState.eType != 14 && pSelf->nextState.eType != 16 && pSelf->nextState.eType != 12 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "GetBrake not called on a vehicle.", 0);
    if ( !pSelf->vehicle
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp", 4173, 0, "%s", "pSelf->vehicle") )
    {
        __debugbreak();
    }
    if ( pSelf->nitrousVeh )
        Scr_AddFloat(pSelf->nitrousVeh->m_throttle, SCRIPTINSTANCE_CLIENT);
    else
        Scr_AddFloat((float)pSelf->nextState.lerp.u.vehicle.throttle * 0.000030517578, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScrCmd_GetMaxSpeed(scr_entref_t entref)
{
    float value; // [esp+8h] [ebp-1Ch]
    centity_s *pSelf; // [esp+18h] [ebp-Ch]

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        4198,
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
    if ( pSelf->nextState.eType != 14 && pSelf->nextState.eType != 16 && pSelf->nextState.eType != 12 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "GetMaxSpeed not called on a vehicle.", 0);
    value = CG_GetVehicleInfo(pSelf->nextState.vehicleState.vehicleInfoIndex)->maxSpeed;
    Scr_AddFloat(value, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScrCmd_GetMaxReverseSpeed(scr_entref_t entref)
{
    centity_s *pSelf; // [esp+14h] [ebp-10h]
    const vehicle_info_t *info; // [esp+1Ch] [ebp-8h]

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        4220,
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
    if ( pSelf->nextState.eType != 14 && pSelf->nextState.eType != 16 && pSelf->nextState.eType != 12 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "GetMaxReverseSpeed not called on a vehicle.", 0);
    if ( !pSelf->vehicle
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp", 4225, 0, "%s", "pSelf->vehicle") )
    {
        __debugbreak();
    }
    info = CG_GetVehicleInfo(pSelf->nextState.vehicleState.vehicleInfoIndex);
    Scr_AddFloat(info->maxSpeed * info->nitrousVehParams.m_reverse_scale, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScrCmd_IsDriving(scr_entref_t entref)
{
    const cg_s *cgameGlob; // [esp+2Ch] [ebp-10h]
    VariableUnion localClientNum; // [esp+30h] [ebp-Ch]

    PROF_SCOPED("CScrCmd_IsDriving");

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        4250,
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
    if ( entref.entnum < com_maxclients->current.integer )
    {
        localClientNum.intValue = CScr_GetLocalClientNum(0);
        cgameGlob = CG_GetLocalClientGlobals(localClientNum.intValue);
        Scr_AddInt(cgameGlob->bgs.clientinfo[entref.entnum].attachedVehEntNum != 1023, SCRIPTINSTANCE_CLIENT);

        return;
    }
    Scr_Error(SCRIPTINSTANCE_CLIENT, "IsDriving not called on a player.", 0);
    Scr_AddInt(0, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScrCmd_IsLocalClientDriver(scr_entref_t entref)
{
    centity_s *pSelf; // [esp+24h] [ebp-18h]
    const cg_s *cgameGlob; // [esp+2Ch] [ebp-10h]
    VariableUnion localClientNum; // [esp+30h] [ebp-Ch]

    PROF_SCOPED("CScrCmd_IsLocalClientDriver");

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        4283,
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
    if ( pSelf->nextState.eType != 14 && pSelf->nextState.eType != 16 && pSelf->nextState.eType != 12 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "IsLocalClientDriver not called on a vehicle.", 0);
    if ( pSelf->vehicle )
    {
        localClientNum.intValue = CScr_GetLocalClientNum(0);
        cgameGlob = CG_GetLocalClientGlobals(localClientNum.intValue);
        Scr_AddInt(
            cgameGlob->bgs.clientinfo[cgameGlob->clientNum].attachedVehEntNum == pSelf->nextState.number,
            SCRIPTINSTANCE_CLIENT);
    }
    else
    {
        Scr_AddInt(0, SCRIPTINSTANCE_CLIENT);
    }
}

void __cdecl CScrCmd_GetLocalClientDriver(scr_entref_t entref)
{
    const cg_s *cgameGlob; // [esp+1Ch] [ebp-14h]
    int i; // [esp+20h] [ebp-10h]
    centity_s *pSelf; // [esp+24h] [ebp-Ch]

    PROF_SCOPED("CScrCmd_GetLocalClientDriver");

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        4318,
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
    if ( pSelf->nextState.eType != 14 && pSelf->nextState.eType != 16 && pSelf->nextState.eType != 12 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "GetLocalClientDriver not called on a vehicle.", 0);
    if ( !pSelf->vehicle
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp", 4323, 0, "%s", "pSelf->vehicle") )
    {
        __debugbreak();
    }
    for ( i = 0; i < 1; ++i )
    {
        if ( CL_LocalClient_IsActive(i) )
        {
            cgameGlob = CG_GetLocalClientGlobals(i);
            if ( cgameGlob->bgs.clientinfo[cgameGlob->clientNum].attachedVehEntNum == pSelf->nextState.number )
            {
                Scr_AddInt(i, SCRIPTINSTANCE_CLIENT);
                return;
            }
        }
    }
}

void __cdecl CScrCmd_GetWheelSurface(scr_entref_t entref)
{
    char *value; // [esp+8h] [ebp-24h]
    centity_s *pSelf; // [esp+18h] [ebp-14h]
    unsigned int wheelName; // [esp+24h] [ebp-8h]
    int wheel; // [esp+28h] [ebp-4h]

    PROF_SCOPED("CScrCmd_GetWheelSurface");

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        4359,
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
    if ( pSelf->nextState.eType != 14 && pSelf->nextState.eType != 16 && pSelf->nextState.eType != 12 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "GetWheelSurface not called on a vehicle.", 0);
    wheelName = Scr_GetConstString(0, SCRIPTINSTANCE_CLIENT);
    wheel = 0;
    if ( wheelName == cscr_const.front_left )
    {
        wheel = 0;
    }
    else if ( wheelName == cscr_const.front_right )
    {
        wheel = 1;
    }
    else if ( wheelName == cscr_const.back_left )
    {
        wheel = 2;
    }
    else if ( wheelName == cscr_const.back_right )
    {
        wheel = 3;
    }
    else if ( wheelName == cscr_const.middle_left )
    {
        wheel = 4;
    }
    else if ( wheelName == cscr_const.middle_right )
    {
        wheel = 5;
    }
    else
    {
        Scr_Error(
            SCRIPTINSTANCE_CLIENT,
            "Bad wheel name. Valid wheel names are: [front_left, front_right, back_left, back_right, middle_left, middle_right]\n",
            0);
    }
    if ( pSelf->vehicle && pSelf->vehicle->wheelSurfType[wheel] )
    {
        value = (char *)Com_SurfaceTypeToName(pSelf->vehicle->wheelSurfType[wheel]);
        Scr_AddString(value, SCRIPTINSTANCE_CLIENT);
    }
    else
    {
        Scr_AddString("none", SCRIPTINSTANCE_CLIENT);
    }
}

void __cdecl CScrCmd_IsWheelSliding(scr_entref_t entref)
{
    centity_s *pSelf; // [esp+Ch] [ebp-10h]
    unsigned int wheelName; // [esp+14h] [ebp-8h]
    int wheel; // [esp+18h] [ebp-4h]

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        4401,
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
    if ( pSelf->nextState.eType != 14 && pSelf->nextState.eType != 16 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "IsWheelSliding not called on a wheeled vehicle.", 0);
    wheelName = Scr_GetConstString(0, SCRIPTINSTANCE_CLIENT);
    wheel = 0;
    if ( wheelName == cscr_const.front_left )
    {
        wheel = 0;
    }
    else if ( wheelName == cscr_const.front_right )
    {
        wheel = 1;
    }
    else if ( wheelName == cscr_const.back_left )
    {
        wheel = 2;
    }
    else if ( wheelName == cscr_const.back_right )
    {
        wheel = 3;
    }
    else if ( wheelName == cscr_const.middle_left )
    {
        wheel = 4;
    }
    else if ( wheelName == cscr_const.middle_right )
    {
        wheel = 5;
    }
    else
    {
        Scr_Error(
            SCRIPTINSTANCE_CLIENT,
            "Bad wheel name. Valid wheel names are: [front_left, front_right, back_left, back_right, middle_left, middle_right]\n",
            0);
    }
    if ( pSelf->nitrousVeh && pSelf->nitrousVeh->m_wheel_state[wheel].m_state == WHEEL_STATE_SKIDDING )
        Scr_AddInt(1, SCRIPTINSTANCE_CLIENT);
    else
        Scr_AddInt(0, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScrCmd_IsWheelColliding(scr_entref_t entref)
{
    rigid_body_constraint_wheel *vehicleWheel; // [esp+Ch] [ebp-14h]
    centity_s *pSelf; // [esp+10h] [ebp-10h]
    unsigned int wheelName; // [esp+18h] [ebp-8h]
    int wheel; // [esp+1Ch] [ebp-4h]

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        4449,
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
    if ( pSelf->nextState.eType != 14 && pSelf->nextState.eType != 16 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "IsWheelColliding not called on a wheeled vehicle.", 0);
    wheelName = Scr_GetConstString(0, SCRIPTINSTANCE_CLIENT);
    wheel = 0;
    if ( wheelName == cscr_const.front_left )
    {
        wheel = 0;
    }
    else if ( wheelName == cscr_const.front_right )
    {
        wheel = 1;
    }
    else if ( wheelName == cscr_const.back_left )
    {
        wheel = 2;
    }
    else if ( wheelName == cscr_const.back_right )
    {
        wheel = 3;
    }
    else if ( wheelName == cscr_const.middle_left )
    {
        wheel = 4;
    }
    else if ( wheelName == cscr_const.middle_right )
    {
        wheel = 5;
    }
    else
    {
        Scr_Error(
            SCRIPTINSTANCE_CLIENT,
            "Bad wheel name. Valid wheel names are: [front_left, front_right, back_left, back_right, middle_left, middle_right]\n",
            0);
    }
    if ( pSelf->nitrousVeh
        && (vehicleWheel = pSelf->nitrousVeh->m_wheels[wheel]) != 0
        && (vehicleWheel->m_wheel_flags & 1) != 0 )
    {
        Scr_AddInt(1, SCRIPTINSTANCE_CLIENT);
    }
    else
    {
        Scr_AddInt(0, SCRIPTINSTANCE_CLIENT);
    }
}

void __cdecl CScrCmd_GetLocalGunnerAngles(scr_entref_t entref)
{
    centity_s *pSelf; // [esp+28h] [ebp-8Ch]
    unsigned __int8 boneIndex; // [esp+2Fh] [ebp-85h] BYREF
    DObj *obj; // [esp+30h] [ebp-84h]
    int gunnerIndex; // [esp+34h] [ebp-80h]
    DObjAnimMat animMat; // [esp+38h] [ebp-7Ch] BYREF
    float tagAxis[4][3]; // [esp+58h] [ebp-5Ch] BYREF
    float tagAngles[3]; // [esp+88h] [ebp-2Ch] BYREF
    fake_centity_s *pFake; // [esp+94h] [ebp-20h]
    float localAngles[3]; // [esp+98h] [ebp-1Ch] BYREF
    unsigned __int16 *gunnerViewTags[4]; // [esp+A4h] [ebp-10h]

    memset(localAngles, 0, sizeof(localAngles));
    boneIndex = 0;
    gunnerViewTags[0] = &scr_const.tag_gunner_barrel1;
    gunnerViewTags[1] = &scr_const.tag_gunner_barrel2;
    gunnerViewTags[2] = &scr_const.tag_gunner_barrel3;
    gunnerViewTags[3] = &scr_const.tag_gunner_barrel4;
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
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        4514,
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
    if ( pSelf->nextState.eType == 14 || pSelf->nextState.eType == 16 || pSelf->nextState.eType == 12 )
    {
        if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) )
        {
            gunnerIndex = Scr_GetInt(0, SCRIPTINSTANCE_CLIENT);
            if ( gunnerIndex < 4 )
            {
                obj = Com_GetClientDObj(pSelf->nextState.number, 0);
                if ( obj )
                {
                    DObjGetBoneIndex(obj, *gunnerViewTags[gunnerIndex], &boneIndex, -1);
                    DObjGetBasePoseMatrix(obj, boneIndex, &animMat);
                    QuatToAxis(animMat.quat, tagAxis);
                    AxisToAngles(tagAxis, tagAngles);
                    localAngles[0] = AngleNormalize180((float)((float)pSelf->pose.vehicle.gunnerPitch[gunnerIndex] * 0.0054931641) - tagAngles[0]);
                    localAngles[1] = AngleNormalize180((float)((float)pSelf->pose.vehicle.gunnerYaw[gunnerIndex] * 0.0054931641) - tagAngles[1]);
                }
                Scr_AddVector(localAngles, SCRIPTINSTANCE_CLIENT);
            }
            else
            {
                Scr_Error(SCRIPTINSTANCE_CLIENT, "Invalid gunner index specified in GetLocalGunnerAngles", 0);
            }
        }
        else
        {
            Scr_Error(SCRIPTINSTANCE_CLIENT, "No gunner index specified in GetLocalGunnerAngles", 0);
        }
    }
    else
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "GetLocalGunnerAngles must be called on a vehicle", 0);
    }
}

void __cdecl CScrCmd_SetStunned(scr_entref_t entref)
{
    NitrousVehicle *nitrousVeh; // [esp+4h] [ebp-1Ch]
    VariableUnion v2; // [esp+8h] [ebp-18h]
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
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        4563,
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
    if ( pSelf->nextState.eType != 14 && pSelf->nextState.eType != 16 && pSelf->nextState.eType != 12 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "GetMaxReverseSpeed not called on a vehicle.", 0);
    if ( !pSelf->vehicle
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp", 4568, 0, "%s", "pSelf->vehicle") )
    {
        __debugbreak();
    }
    if ( pSelf->nitrousVeh )
    {
        v2.intValue = Scr_GetInt(0, SCRIPTINSTANCE_CLIENT);
        nitrousVeh = pSelf->nitrousVeh;
        if ( v2.intValue )
            nitrousVeh->m_flags |= 0x2000u;
        else
            nitrousVeh->m_flags &= ~0x2000u;
    }
}

void CScr_IsInVehicle()
{
    scr_entref_t v0; // [esp+0h] [ebp-38h] BYREF
    scr_entref_t v1; // [esp+Ah] [ebp-2Eh]
    scr_entref_t v2; // [esp+18h] [ebp-20h]
    scr_entref_t v3; // [esp+22h] [ebp-16h]
    const cg_s *cgameGlob; // [esp+28h] [ebp-10h]
    int localClientNum; // [esp+2Ch] [ebp-Ch]
    scr_entref_t vehicleEntRef; // [esp+30h] [ebp-8h]

    localClientNum = CScr_GetLocalClientNum(0);
    v1 = Scr_GetEntityRef(1, SCRIPTINSTANCE_CLIENT);
    v2 = v1;
    v3 = v1;
    vehicleEntRef = v1;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( Demo_IsMovieCamera() || cgameGlob->predictedPlayerState.viewlocked_entNum != vehicleEntRef.entnum )
        Scr_AddInt(0, SCRIPTINSTANCE_CLIENT);
    else
        Scr_AddInt(1, SCRIPTINSTANCE_CLIENT);
}

void CScr_PrecacheRumble()
{
    Scr_AddInt(0, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScrCmd_IsVehiclePeelingOut(scr_entref_t entref)
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
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        4649,
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
    if ( pSelf->nextState.eType != 14 && pSelf->nextState.eType != 16 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "IsPeelingOut not called on a wheeled vehicle.", 0);
    if ( !pSelf->vehicle
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp", 4654, 0, "%s", "pSelf->vehicle") )
    {
        __debugbreak();
    }
    if ( pSelf->nitrousVeh && pSelf->nitrousVeh->is_peeling_out() )
        Scr_AddInt(1, SCRIPTINSTANCE_CLIENT);
    else
        Scr_AddInt(0, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_line()
{
    VariableUnion duration; // [esp+4h] [ebp-3Ch]
    float rgb[3]; // [esp+8h] [ebp-38h] BYREF
    float start[3]; // [esp+14h] [ebp-2Ch] BYREF
    float end[3]; // [esp+20h] [ebp-20h] BYREF
    float color[4]; // [esp+2Ch] [ebp-14h] BYREF
    int depthTest; // [esp+3Ch] [ebp-4h]

    duration.intValue = 0;
    depthTest = 0;
    color[0] = 1.0f;
    color[1] = 1.0f;
    color[2] = 1.0f;
    color[3] = 1.0f;
    switch ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) )
    {
        case 2:
            goto $LN21_9;
        case 3:
            goto $LN3_32;
        case 4:
            goto $LN4_27;
        case 5:
            goto $LN5_25;
        case 6:
            duration.intValue = Scr_GetInt(5u, SCRIPTINSTANCE_CLIENT);
$LN5_25:
            depthTest = Scr_GetInt(4u, SCRIPTINSTANCE_CLIENT);
$LN4_27:
            color[3] = Scr_GetFloat(3u, SCRIPTINSTANCE_CLIENT);
$LN3_32:
            Scr_GetVector(2u, rgb, SCRIPTINSTANCE_CLIENT);
            color[0] = rgb[0];
            color[1] = rgb[1];
            color[2] = rgb[2];
$LN21_9:
            Scr_GetVector(1u, end, SCRIPTINSTANCE_CLIENT);
            Scr_GetVector(0, start, SCRIPTINSTANCE_CLIENT);
            break;
        default:
            Scr_Error(SCRIPTINSTANCE_CLIENT, "illegal call to line()", 0);
            break;
    }
    CL_AddDebugLine(start, end, color, depthTest, duration.intValue);
}

void __cdecl CScr_box()
{
    float pos[3]; // [esp+14h] [ebp-4Ch] BYREF
    int duration; // [esp+20h] [ebp-40h]
    float rgb[3]; // [esp+24h] [ebp-3Ch] BYREF
    float mins[3]; // [esp+30h] [ebp-30h] BYREF
    float yaw; // [esp+3Ch] [ebp-24h]
    float maxs[3]; // [esp+40h] [ebp-20h] BYREF
    float color[4]; // [esp+4Ch] [ebp-14h] BYREF
    int depthTest; // [esp+5Ch] [ebp-4h]

    duration = 0;
    depthTest = 0;
    color[0] = 1.0f;
    color[1] = 1.0f;
    color[2] = 1.0f;
    color[3] = 1.0f;
    yaw = 0.0f;
    mins[0] = -10.0f;
    mins[1] = -10.0f;
    mins[2] = -10.0f;
    maxs[0] = 10.0f;
    maxs[1] = 10.0f;
    maxs[2] = 10.0f;
    switch ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) )
    {
        case 1:
            goto $LN36_6;
        case 2:
            goto $LN34_5;
        case 3:
            goto $LN15_11;
        case 4:
            goto $LN30_7;
        case 5:
            goto $LN6_27;
        case 6:
            goto $LN7_22;
        case 7:
            goto $LN8_18;
        case 8:
            duration = Scr_GetInt(7u, SCRIPTINSTANCE_CLIENT);
$LN8_18:
            depthTest = Scr_GetInt(6u, SCRIPTINSTANCE_CLIENT);
$LN7_22:
            color[3] = Scr_GetFloat(5u, SCRIPTINSTANCE_CLIENT);
$LN6_27:
            Scr_GetVector(4u, rgb, SCRIPTINSTANCE_CLIENT);
            color[0] = rgb[0];
            color[1] = rgb[1];
            color[2] = rgb[2];
$LN30_7:
            yaw = Scr_GetFloat(3u, SCRIPTINSTANCE_CLIENT);
            if ( (yaw < 0.0 || yaw > 360.0)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                            4778,
                            0,
                            "%s",
                            "yaw >= 0.0f && yaw <= 360.0f") )
            {
                __debugbreak();
            }
$LN15_11:
            Scr_GetVector(2u, maxs, SCRIPTINSTANCE_CLIENT);
$LN34_5:
            Scr_GetVector(1u, mins, SCRIPTINSTANCE_CLIENT);
$LN36_6:
            Scr_GetVector(0, pos, SCRIPTINSTANCE_CLIENT);
            break;
        default:
            Scr_Error(SCRIPTINSTANCE_CLIENT, "illegal call to box()", 0);
            break;
    }
    CL_AddDebugBox(pos, mins, maxs, yaw, color, depthTest, duration);
}

void __cdecl CScr_debugstar()
{
    int NumParam; // [esp+0h] [ebp-30h]
    VariableUnion duration; // [esp+4h] [ebp-2Ch]
    float rgb[3]; // [esp+8h] [ebp-28h] BYREF
    float location[3]; // [esp+14h] [ebp-1Ch] BYREF
    float color[4]; // [esp+20h] [ebp-10h] BYREF

    duration.intValue = 10;
    color[0] = 1.0f;
    color[1] = 1.0f;
    color[2] = 1.0f;
    color[3] = 1.0f;
    NumParam = Scr_GetNumParam(SCRIPTINSTANCE_CLIENT);
    if ( NumParam != 1 )
    {
        if ( NumParam != 2 )
        {
            if ( NumParam != 3 )
            {
                Scr_Error(SCRIPTINSTANCE_CLIENT, "illegal call to debugstar()", 0);
                goto LABEL_8;
            }
            Scr_GetVector(2u, rgb, SCRIPTINSTANCE_CLIENT);
            color[0] = rgb[0];
            color[1] = rgb[1];
            color[2] = rgb[2];
        }
        duration.intValue = Scr_GetInt(1u, SCRIPTINSTANCE_CLIENT);
    }
    Scr_GetVector(0, location, SCRIPTINSTANCE_CLIENT);
LABEL_8:
    CL_AddDebugStar(location, color, duration.intValue);
}

void CScr_GetMaxLocalClients()
{
    Scr_AddInt(1, SCRIPTINSTANCE_CLIENT);
}

void CScr_LocalClientActive()
{
    unsigned int value; // [esp+0h] [ebp-8h]
    VariableUnion i; // [esp+4h] [ebp-4h]

    i.intValue = Scr_GetInt(0, SCRIPTINSTANCE_CLIENT);
    value = (unsigned __int8)CL_LocalClient_IsActive(i.intValue);
    Scr_AddBool(value, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_IsSplitScreenHost()
{
    bool value; // [esp+0h] [ebp-8h]
    VariableUnion localClientNum; // [esp+4h] [ebp-4h]

    localClientNum.intValue = Scr_GetInt(0, SCRIPTINSTANCE_CLIENT);
    value = Com_LocalClient_IsPrimary(localClientNum.intValue);
    Scr_AddInt(value, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_IsSplitScreen()
{
    bool value; // [esp+0h] [ebp-4h]

    value = CG_IsShowingZombieMap();
    Scr_AddInt(value, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_StopBink()
{
    Com_Printf(13, "DEBUG: Cinematic disabled\n");
}

void __cdecl CScr_GetBinkLength()
{
    float value; // [esp+10h] [ebp-4Ch]
    char movieName[64]; // [esp+14h] [ebp-48h] BYREF
    unsigned int timeInMsec; // [esp+58h] [ebp-4h] BYREF

    R_Cinematic_GetFilenameAndTimeInMsec(movieName, 64, &timeInMsec);
    value = (float)(timeInMsec / 0x3E8);
    Scr_AddFloat(value, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_IsBinkFinished()
{
    bool value; // [esp+0h] [ebp-4h]

    value = R_Cinematic_IsFinished();
    Scr_AddInt(value, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_FxParamError(int localClientNum, unsigned int paramIndex, const char *errorString, int fxId)
{
    char v4; // [esp+3h] [ebp-419h]
    char *v5; // [esp+8h] [ebp-414h]
    const char *v6; // [esp+Ch] [ebp-410h]
    char *error; // [esp+10h] [ebp-40Ch]
    char fxName[1024]; // [esp+14h] [ebp-408h] BYREF
    const char *string; // [esp+418h] [ebp-4h]

    string = 0;
    if ( !errorString
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp", 5016, 0, "%s", "errorString") )
    {
        __debugbreak();
    }
    if ( fxId )
        string = CL_GetConfigString(fxId + 2080);
    if ( *string )
    {
        v6 = string;
        v5 = fxName;
        do
        {
            v4 = *v6;
            *v5++ = *v6++;
        }
        while ( v4 );
    }
    else
    {
        strcpy(fxName, "not successfully loaded");
    }
    error = va("%s (effect = %s)\n", errorString, fxName);
    Scr_ParamError(paramIndex, error, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_SetFxAngles(unsigned int givenAxisCount, float (*axis)[3], float *angles)
{
    char *error; // [esp+14h] [ebp-24h]
    float v4; // [esp+28h] [ebp-10h]

    if ( givenAxisCount > 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                    5035,
                    0,
                    "givenAxisCount not in [0, 2]\n\t%i not in [%i, %i]",
                    givenAxisCount,
                    0,
                    2) )
    {
        __debugbreak();
    }
    if ( givenAxisCount == 1 )
    {
        vectoangles((const float *)axis, angles);
    }
    else if ( givenAxisCount == 2 )
    {
        v4 = -( (float)((float)((*axis)[0] * (*axis)[6]) + (float)((*axis)[1] * (*axis)[7])) + (float)((*axis)[2] * (*axis)[8]));
        (*axis)[6] = (float)(v4 * (*axis)[0]) + (*axis)[6];
        (*axis)[7] = (float)(v4 * (*axis)[1]) + (*axis)[7];
        (*axis)[8] = (float)(v4 * (*axis)[2]) + (*axis)[8];
        if ( Vec3Normalize(&(*axis)[6]) == 0.0 )
        {
            error = va("forward and up vectors are the same direction or exact opposite directions");
            Scr_Error(SCRIPTINSTANCE_CLIENT, error, 0);
        }
        Vec3Cross(&(*axis)[6], (const float *)axis, &(*axis)[3]);
        AxisToAngles(axis, angles);
    }
    else
    {
        *angles = 270.0f;
        angles[1] = 0.0f;
        angles[2] = 0.0f;
    }
    *angles = AngleNormalize180(*angles);
    angles[1] = AngleNormalize180(angles[1]);
    angles[2] = AngleNormalize180(angles[2]);
}

unsigned int __cdecl CScr_PlayFXInternal(int localClientNum, int fxId, float *angles, float *pos, int time)
{
    unsigned int v6; // [esp+8h] [ebp-38h]
    const FxEffectDef *fxDef; // [esp+14h] [ebp-2Ch]
    float axis[3][3]; // [esp+1Ch] [ebp-24h] BYREF

    fxDef = CG_GetLocalClientStaticGlobals(localClientNum)->fxs[fxId];
    if ( !fxDef && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp", 5067, 0, "%s", "fxDef") )
        __debugbreak();
    {
        PROF_SCOPED("CScr_PlayFXInternal");
        AnglesToAxis(angles, axis);
        v6 = FX_PlayOrientedEffect(localClientNum, fxDef, time, pos, axis);
    }
    
    return v6;
}

void __cdecl CScr_PlayFX()
{
    float Float; // [esp+10h] [ebp-7Ch]
    char *error; // [esp+2Ch] [ebp-60h]
    float pos[3]; // [esp+38h] [ebp-54h] BYREF
    int iTime; // [esp+44h] [ebp-48h]
    unsigned int effectHandle; // [esp+48h] [ebp-44h]
    int localClientNum; // [esp+4Ch] [ebp-40h]
    int numParams; // [esp+50h] [ebp-3Ch]
    float angles[3]; // [esp+54h] [ebp-38h] BYREF
    int fxId; // [esp+60h] [ebp-2Ch]
    float axis[3][3]; // [esp+64h] [ebp-28h] BYREF
    float vecLength; // [esp+88h] [ebp-4h]

    numParams = Scr_GetNumParam(SCRIPTINSTANCE_CLIENT);
    if ( numParams < 3 || numParams > 6 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "Incorrect number of parameters", 0);
    localClientNum = CScr_GetLocalClientNum(0);
    iTime = CG_GetLocalClientGlobals(localClientNum)->time;
    fxId = Scr_GetInt(1u, SCRIPTINSTANCE_CLIENT);
    if ( fxId <= 0 || fxId >= 196 )
    {
        error = va("CScr_PlayFX: invalid effect id %d", fxId);
        Scr_Error(SCRIPTINSTANCE_CLIENT, error, 0);
    }
    Scr_GetVector(2u, pos, SCRIPTINSTANCE_CLIENT);
    if ( numParams == 3 )
    {
        CScr_SetFxAngles(0, axis, angles);
        effectHandle = CScr_PlayFXInternal(localClientNum, fxId, angles, pos, iTime);
        Scr_AddInt(effectHandle, SCRIPTINSTANCE_CLIENT);
    }
    else
    {
        Scr_GetVector(3u, axis[0], SCRIPTINSTANCE_CLIENT);
        vecLength = Vec3Normalize(axis[0]);
        if ( vecLength == 0.0 )
            CScr_FxParamError(localClientNum, 3u, "playFx called with (0 0 0) forward direction", fxId);
        if ( numParams == 4 )
        {
            CScr_SetFxAngles(1u, axis, angles);
            effectHandle = CScr_PlayFXInternal(localClientNum, fxId, angles, pos, iTime);
            Scr_AddInt(effectHandle, SCRIPTINSTANCE_CLIENT);
        }
        else
        {
            if ( numParams < 5
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                            5149,
                            1,
                            "%s\n\t(numParams) = %i",
                            "(numParams >= 5)",
                            numParams) )
            {
                __debugbreak();
            }
            Scr_GetVector(4u, axis[2], SCRIPTINSTANCE_CLIENT);
            vecLength = Vec3Normalize(axis[2]);
            if ( vecLength == 0.0 )
                CScr_FxParamError(localClientNum, 4u, "playFx called with (0 0 0) up direction", fxId);
            if ( numParams > 5 )
            {
                Float = Scr_GetFloat(5u, SCRIPTINSTANCE_CLIENT);
                iTime = (int)((float)(Float * 1000.0) + 9.313225746154785e-10);
            }
            CScr_SetFxAngles(2u, axis, angles);
            effectHandle = CScr_PlayFXInternal(localClientNum, fxId, angles, pos, iTime);
            Scr_AddInt(effectHandle, SCRIPTINSTANCE_CLIENT);
        }
    }
}

void __cdecl CScr_StopFX()
{
    unsigned int fxHandle; // [esp+8h] [ebp-Ch]
    int localClientNum; // [esp+Ch] [ebp-8h]

    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) != 2 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "Incorrect number of parameters for IsFXPlaying", 0);
    localClientNum = CScr_GetLocalClientNum(0);
    if ( (unsigned int)localClientNum >= 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                    5189,
                    0,
                    "localClientNum not in [0, MAX_LOCAL_CLIENTS]\n\t%i not in [%i, %i]",
                    localClientNum,
                    0,
                    1) )
    {
        __debugbreak();
    }
    fxHandle = Scr_GetInt(1u, SCRIPTINSTANCE_CLIENT);
    FX_ThroughWithEffect(localClientNum, fxHandle, 1);
}

void __cdecl CScr_IsFXPlaying()
{
    bool isDone; // [esp+Bh] [ebp-Dh]
    unsigned int fxHandle; // [esp+Ch] [ebp-Ch]
    int localClientNum; // [esp+10h] [ebp-8h]

    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) != 2 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "Incorrect number of parameters for IsFXPlaying", 0);
    localClientNum = CScr_GetLocalClientNum(0);
    if ( (unsigned int)localClientNum >= 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                    5220,
                    0,
                    "localClientNum not in [0, MAX_LOCAL_CLIENTS]\n\t%i not in [%i, %i]",
                    localClientNum,
                    0,
                    1) )
    {
        __debugbreak();
    }
    fxHandle = Scr_GetInt(1u, SCRIPTINSTANCE_CLIENT);
    isDone = FX_IsEffectDone(localClientNum, fxHandle);
    Scr_AddBool(!isDone, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_DisableImpactMarks()
{
    Dvar_SetFloat((dvar_s *)fx_marks_range, 0.0);
}

void __cdecl CScr_EnableImpactMarks()
{
    Dvar_SetFloat((dvar_s *)fx_marks_range, 1500.0);
}

void __cdecl CScr_GetEnt()
{
    int localClientNum; // [esp+8h] [ebp-14h]
    int offset; // [esp+Ch] [ebp-10h]
    char *pName; // [esp+10h] [ebp-Ch]
    char *key; // [esp+18h] [ebp-4h]

    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) != 3 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "illegal call to getent()\n", 0);
    localClientNum = CScr_GetLocalClientNum(0);
    pName = Scr_GetString(1u, SCRIPTINSTANCE_CLIENT);
    key = Scr_GetString(2u, SCRIPTINSTANCE_CLIENT);
    offset = Scr_GetOffset(0, key, SCRIPTINSTANCE_CLIENT);
    if ( offset >= 0 )
        CScr_GetGenericEnt(localClientNum, offset, pName);
}

void __cdecl CScr_GetGenericEnt(int localClientNum, unsigned int offset, const char *pName)
{
    int ofs; // edx
    unsigned int v4; // [esp+0h] [ebp-3Ch]
    centity_s *result; // [esp+1Ch] [ebp-20h]
    unsigned int hash; // [esp+20h] [ebp-1Ch]
    unsigned int name; // [esp+28h] [ebp-14h]
    centity_s *ent; // [esp+30h] [ebp-Ch]
    int i; // [esp+34h] [ebp-8h]

    hash = Scr_GetHashCode(SCRIPTINSTANCE_CLIENT, pName, strlen(pName));
    name = SL_FindString(pName, SCRIPTINSTANCE_CLIENT);
    if ( offset >= 0x18
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                    5282,
                    0,
                    "offset doesn't index ARRAY_COUNT( cent_fields ) - 1\n\t%i not in [0, %i)",
                    offset,
                    24) )
    {
        __debugbreak();
    }
    if ( cent_fields[offset].type != F_STRING )
        Scr_ParamError(1u, "key is not internally a string", SCRIPTINSTANCE_CLIENT);
    result = 0;
    for ( i = 0; i < 1536; ++i )
    {
        ent = CG_GetEntity(localClientNum, i);
        if ( ((*((unsigned int *)ent + 201) >> 1) & 1) != 0 )
        {
            ofs = cent_fields[offset].ofs;
            if ( *(unsigned __int16 *)((char *)&ent->pose.lightingHandle + ofs) )
            {
                v4 = i >= 1024 ? name : hash;
                if ( *(__int16 *)((char *)&ent->pose.lightingHandle + ofs) == v4 )
                {
                    if ( result )
                        Scr_Error(SCRIPTINSTANCE_CLIENT, "getent used with more than one entity", 0);
                    result = ent;
                }
            }
        }
    }
    if ( result )
        CScr_AddEntity(result, localClientNum);
}

void __cdecl CScrCmd_IsAlive(scr_entref_t entref)
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
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        5373,
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
    if ( pSelf->nextState.eType != 17
        && pSelf->nextState.eType != 14
        && pSelf->nextState.eType != 16
        && pSelf->nextState.eType != 12
        && pSelf->nextState.eType != 13
        && pSelf->nextState.eType != 1 )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "IsAlive called on the wrong kind of entity", 0);
    }
    if ( pSelf->nextState.eType == 17
        || pSelf->nextState.eType == 14
        || pSelf->nextState.eType == 16
        || pSelf->nextState.eType == 12
        || pSelf->nextState.eType == 13
        || pSelf->nextState.eType == 1 )
    {
        Scr_AddInt(1, SCRIPTINSTANCE_CLIENT);
    }
    else
    {
        Scr_AddInt(0, SCRIPTINSTANCE_CLIENT);
    }
}

void __cdecl CScr_GetEntNum()
{
    int localClientNum; // [esp+10h] [ebp-Ch]
    centity_s *cent; // [esp+14h] [ebp-8h]
    VariableUnion centNum; // [esp+18h] [ebp-4h]

    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) != 2 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "CScr_GetEntNum takes 2 params.", 0);
    localClientNum = CScr_GetLocalClientNum(0);
    centNum.intValue = Scr_GetInt(1u, SCRIPTINSTANCE_CLIENT);
    cent = CG_GetEntity(localClientNum, centNum.intValue);
    if ( ((*((unsigned int *)cent + 201) >> 1) & 1) == 0 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "GetEntNum used on nonexistant ent.", 0);
    CScr_AddEntity(cent, localClientNum);
}

void __cdecl CScr_GetEntArray()
{
    int ofs; // eax
    unsigned int v1; // [esp+0h] [ebp-58h]
    unsigned int hash; // [esp+2Ch] [ebp-2Ch]
    centity_s *cent; // [esp+30h] [ebp-28h]
    centity_s *centa; // [esp+30h] [ebp-28h]
    int localClientNum; // [esp+34h] [ebp-24h]
    int offset; // [esp+3Ch] [ebp-1Ch]
    unsigned int name; // [esp+40h] [ebp-18h]
    char *pName; // [esp+44h] [ebp-14h]
    char *key; // [esp+48h] [ebp-10h]
    int i; // [esp+50h] [ebp-8h]
    int ia; // [esp+50h] [ebp-8h]

    localClientNum = CScr_GetLocalClientNum(0);
    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) == 1 )
    {
        Scr_MakeArray(SCRIPTINSTANCE_CLIENT);
        for ( i = 0; i < 1536; ++i )
        {
            cent = CG_GetEntity(localClientNum, i);
            if ( ((*((unsigned int *)cent + 201) >> 1) & 1) != 0 )
            {
                CScr_AddEntity(cent, localClientNum);
                Scr_AddArray(SCRIPTINSTANCE_CLIENT);
            }
        }
    }
    else
    {
        pName = Scr_GetString(1u, SCRIPTINSTANCE_CLIENT);
        hash = Scr_GetHashCode(SCRIPTINSTANCE_CLIENT, pName, strlen(pName));
        name = SL_FindString(pName, SCRIPTINSTANCE_CLIENT);
        key = Scr_GetString(2u, SCRIPTINSTANCE_CLIENT);
        offset = Scr_GetOffset(0, key, SCRIPTINSTANCE_CLIENT);
        if ( offset >= 0 )
        {
            if ( offset >= 24
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                            5455,
                            0,
                            "%s",
                            "offset >= 0 && offset < static_cast<int>( ARRAY_COUNT( cent_fields ) - 1 )") )
            {
                __debugbreak();
            }
            if ( cent_fields[offset].type == F_STRING )
            {
                Scr_MakeArray(SCRIPTINSTANCE_CLIENT);
                for ( ia = 0; ia < 1536; ++ia )
                {
                    centa = CG_GetEntity(localClientNum, ia);
                    if ( ((*((unsigned int *)centa + 201) >> 1) & 1) != 0 )
                    {
                        ofs = cent_fields[offset].ofs;
                        if ( *(unsigned __int16 *)((char *)&centa->pose.lightingHandle + ofs) )
                        {
                            v1 = ia >= 1024 ? name : hash;
                            if ( *(__int16 *)((char *)&centa->pose.lightingHandle + ofs) == v1 )
                            {
                                CScr_AddEntity(centa, localClientNum);
                                Scr_AddArray(SCRIPTINSTANCE_CLIENT);
                            }
                        }
                    }
                }
            }
        }
    }
}

void __cdecl CScr_GetLocalPlayers()
{
    cg_s *cGameGlob; // [esp+0h] [ebp-Ch]
    int i; // [esp+8h] [ebp-4h]

    Scr_MakeArray(SCRIPTINSTANCE_CLIENT);
    for ( i = 0; i < 1; ++i )
    {
        if ( CL_LocalClient_IsActive(i) )
        {
            cGameGlob = CG_GetLocalClientGlobals(i);
            if ( cGameGlob->nextSnap )
            {
                if ( ((*((unsigned int *)&cGameGlob->predictedPlayerEntity + 201) >> 1) & 1) != 0 )
                {
                    CScr_AddEntity(&cGameGlob->predictedPlayerEntity, i);
                    Scr_AddArray(SCRIPTINSTANCE_CLIENT);
                }
            }
        }
    }
}

void __cdecl CScr_GetLocalPlayer()
{
    centity_s *cent; // [esp+Ch] [ebp-8h]
    int localClientNum; // [esp+10h] [ebp-4h]

    localClientNum = CScr_GetLocalClientNum(0);
    if ( CL_LocalClient_IsActive(localClientNum) )
    {
        cent = &CG_GetLocalClientGlobals(localClientNum)->predictedPlayerEntity;
        CScr_AddEntity(cent, localClientNum);
    }
    else
    {
        Scr_AddUndefined(SCRIPTINSTANCE_CLIENT);
    }
}

void CScr_BulletTrace()
{
    centity_s *Entity; // eax
    unsigned intresult; // eax
    char *value; // [esp+2Ch] [ebp-DCh]
    scr_entref_t v3; // [esp+54h] [ebp-B4h] BYREF
    scr_entref_t v4; // [esp+5Eh] [ebp-AAh]
    scr_entref_t v5; // [esp+64h] [ebp-A4h]
    scr_entref_t v6; // [esp+6Eh] [ebp-9Ah]
    float vNorm[3]; // [esp+74h] [ebp-94h] BYREF
    scr_entref_t ref; // [esp+80h] [ebp-88h]
    int bIgnoreWater; // [esp+8Ch] [ebp-7Ch]
    float vEnd[3]; // [esp+90h] [ebp-78h] BYREF
    centity_s *pIgnoreEnt; // [esp+9Ch] [ebp-6Ch]
    int iClipMask; // [esp+A0h] [ebp-68h]
    trace_t trace; // [esp+A8h] [ebp-60h] BYREF
    float endpos[3]; // [esp+E4h] [ebp-24h] BYREF
    int iIgnoreEntNum; // [esp+F0h] [ebp-18h]
    int iSurfaceTypeIndex; // [esp+F4h] [ebp-14h]
    float vStart[3]; // [esp+F8h] [ebp-10h] BYREF
    unsigned __int16 hitEntId; // [esp+104h] [ebp-4h]

    pIgnoreEnt = 0;
    iIgnoreEntNum = 1023;
    iClipMask = 0x280E833;

    PROF_SCOPED("CScr_BulletTrace");

    Scr_GetVector(0, vStart, SCRIPTINSTANCE_CLIENT);
    Scr_GetVector(1u, vEnd, SCRIPTINSTANCE_CLIENT);
    if ( !Scr_GetInt(2u, SCRIPTINSTANCE_CLIENT) )
        iClipMask &= 0xFDFF7FFF;
    if ( Scr_GetType(3u, SCRIPTINSTANCE_CLIENT) == 1 && Scr_GetPointerType(3u, SCRIPTINSTANCE_CLIENT) == 19 )
    {
        v4 = Scr_GetEntityRef(3, SCRIPTINSTANCE_CLIENT);
        v5 = v4;
        v6 = v4;
        ref = v4;
        pIgnoreEnt = CG_GetEntity(v4.client, v4.entnum);
        iIgnoreEntNum = pIgnoreEnt->nextState.number;
    }
    bIgnoreWater = 0;
    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) == 5 )
        bIgnoreWater = Scr_GetInt(4u, SCRIPTINSTANCE_CLIENT);
    if ( bIgnoreWater )
        iClipMask &= ~0x20u;
    CG_LocationalTrace(&trace, vStart, vEnd, iIgnoreEntNum, iClipMask, 0, 0);
    Scr_MakeArray(SCRIPTINSTANCE_CLIENT);
    Scr_AddFloat(trace.fraction, SCRIPTINSTANCE_CLIENT);
    Scr_AddArrayStringIndexed(cscr_const.fraction, SCRIPTINSTANCE_CLIENT);
    Vec3Lerp(vStart, vEnd, trace.fraction, endpos);
    Scr_AddVector(endpos, SCRIPTINSTANCE_CLIENT);
    Scr_AddArrayStringIndexed(cscr_const.position, SCRIPTINSTANCE_CLIENT);
    hitEntId = Trace_GetEntityHitId(&trace);
    if ( hitEntId == 1023 || hitEntId == 1022 )
    {
        Scr_AddUndefined(SCRIPTINSTANCE_CLIENT);
    }
    else
    {
        Entity = CG_GetEntity(0, hitEntId);
        CScr_AddEntity(Entity, 0);
    }
    Scr_AddArrayStringIndexed(cscr_const.entity, SCRIPTINSTANCE_CLIENT);
    if ( trace.fraction >= 1.0 )
    {
        vNorm[0] = vEnd[0] - vStart[0];
        vNorm[1] = vEnd[1] - vStart[1];
        vNorm[2] = vEnd[2] - vStart[2];
        Vec3Normalize(vNorm);
        Scr_AddVector(vNorm, SCRIPTINSTANCE_CLIENT);
        Scr_AddArrayStringIndexed(cscr_const.normal, SCRIPTINSTANCE_CLIENT);
        Scr_AddConstString(cscr_const.none, SCRIPTINSTANCE_CLIENT);
    }
    else
    {
        Scr_AddVector(trace.normal.vec.v, SCRIPTINSTANCE_CLIENT);
        Scr_AddArrayStringIndexed(cscr_const.normal, SCRIPTINSTANCE_CLIENT);
        iSurfaceTypeIndex = (unsigned __int8)((int)(0x3F00000 & trace.sflags) >> 20);
        value = (char *)Com_SurfaceTypeToName(iSurfaceTypeIndex);
        Scr_AddString(value, SCRIPTINSTANCE_CLIENT);
    }
    Scr_AddArrayStringIndexed(cscr_const.surfacetype, SCRIPTINSTANCE_CLIENT);
}

void CScr_TracePoint()
{
    char *value; // [esp+30h] [ebp-54h]
    float hitp[3]; // [esp+48h] [ebp-3Ch] BYREF
    float normal[3]; // [esp+54h] [ebp-30h] BYREF
    float p1[3]; // [esp+60h] [ebp-24h] BYREF
    float fraction; // [esp+6Ch] [ebp-18h] BYREF
    int stype; // [esp+70h] [ebp-14h]
    float p0[3]; // [esp+74h] [ebp-10h] BYREF
    int mask; // [esp+80h] [ebp-4h]

    PROF_SCOPED("CScr_TracePoint");

    Scr_GetVector(0, p0, SCRIPTINSTANCE_CLIENT);
    Scr_GetVector(1u, p1, SCRIPTINSTANCE_CLIENT);
    mask = 0x280E833;
    stype = CM_TracePointDown(p0, p1, 0x280E833, 0x3F00000, hitp, &fraction, normal);
    Scr_MakeArray(SCRIPTINSTANCE_CLIENT);
    Scr_AddFloat(fraction, SCRIPTINSTANCE_CLIENT);
    Scr_AddArrayStringIndexed(cscr_const.fraction, SCRIPTINSTANCE_CLIENT);
    if ( fraction >= 1.0 )
    {
        Scr_AddVector(p1, SCRIPTINSTANCE_CLIENT);
        Scr_AddArrayStringIndexed(cscr_const.position, SCRIPTINSTANCE_CLIENT);
        normal[0] = p1[0] - p0[0];
        normal[1] = p1[1] - p0[1];
        normal[2] = p1[2] - p0[2];
        Vec3Normalize(normal);
        Scr_AddVector(normal, SCRIPTINSTANCE_CLIENT);
        Scr_AddArrayStringIndexed(cscr_const.normal, SCRIPTINSTANCE_CLIENT);
        Scr_AddConstString(cscr_const.none, SCRIPTINSTANCE_CLIENT);
    }
    else
    {
        Scr_AddVector(hitp, SCRIPTINSTANCE_CLIENT);
        Scr_AddArrayStringIndexed(cscr_const.position, SCRIPTINSTANCE_CLIENT);
        Scr_AddVector(normal, SCRIPTINSTANCE_CLIENT);
        Scr_AddArrayStringIndexed(cscr_const.normal, SCRIPTINSTANCE_CLIENT);
        value = (char *)Com_SurfaceTypeToName((stype & 0x3F00000) >> 20);
        Scr_AddString(value, SCRIPTINSTANCE_CLIENT);
    }
    Scr_AddArrayStringIndexed(cscr_const.surfacetype, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_OpenFile()
{
    char *v0; // eax
    char *v1; // eax
    int filesize; // [esp+18h] [ebp-18h]
    int tempFile; // [esp+1Ch] [ebp-14h] BYREF
    const char *fullpathname; // [esp+20h] [ebp-10h]
    const char *filename; // [esp+24h] [ebp-Ch]
    int *f; // [esp+28h] [ebp-8h]
    const char *mode; // [esp+2Ch] [ebp-4h]

    if ( (unsigned int)Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) > 1 )
    {
        filename = Scr_GetString(0, SCRIPTINSTANCE_CLIENT);
        mode = Scr_GetString(1u, SCRIPTINSTANCE_CLIENT);
        f = g_cs_file == 0 ? &g_cs_file : 0;
        if ( !f )
        {
            Com_Printf(24, "OpenFile failed.    %i files already open\n", 1);
            Scr_AddInt(-1, SCRIPTINSTANCE_CLIENT);
            return;
        }
        if ( !I_stricmp(mode, "read") )
        {
            fullpathname = va("%s/%s", "scriptdata", filename);
            filesize = FS_FOpenFileByMode((char *)fullpathname, &tempFile, FS_READ);
            if ( filesize >= 0 )
            {
                g_cs_file_buffer = Z_VirtualAlloc(filesize + 1, "CScr_OpenFile", 11);
                FS_Read((unsigned __int8 *)g_cs_file_buffer, filesize, tempFile);
                FS_FCloseFile(tempFile);
                g_cs_file_buffer[filesize] = 0;
                Com_BeginParseSession(filename);
                Com_SetCSV(1);
                g_cs_file_linemark.lines = 0;
                Scr_AddInt(0, SCRIPTINSTANCE_CLIENT);
            }
            else
            {
                Scr_AddInt(-1, SCRIPTINSTANCE_CLIENT);
            }
            return;
        }
        if ( I_stricmp(mode, "write") )
        {
            if ( strcmp(mode, "append") )
            {
                Com_Printf(24, "Valid openfile modes are 'write', 'read', and 'append'\n");
                Scr_AddInt(-1, SCRIPTINSTANCE_CLIENT);
                return;
            }
            v1 = va("%s/%s", "scriptdata", filename);
            if ( (FS_FOpenFileByMode(v1, f, FS_APPEND) & 0x80000000) != 0 )
            {
LABEL_10:
                Scr_AddInt(-1, SCRIPTINSTANCE_CLIENT);
                return;
            }
        }
        else
        {
            v0 = va("%s/%s", "scriptdata", filename);
            *f = FS_FOpenTextFileWrite(v0);
            if ( !*f )
                goto LABEL_10;
        }
        Scr_AddInt(0, SCRIPTINSTANCE_CLIENT);
    }
}

void CScr_CloseFile()
{
    int filenum; // [esp+0h] [ebp-4h]

    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) )
    {
        filenum = Scr_GetInt(0, SCRIPTINSTANCE_CLIENT);
        if ( (unsigned int)filenum >= 2 )
        {
            Com_Printf(24, "CloseFile failed, invalid file number %i\n", filenum);
            Scr_AddInt(-1, SCRIPTINSTANCE_CLIENT);
            return;
        }
        if ( g_cs_file
            && g_cs_file_buffer
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        5909,
                        0,
                        "%s",
                        "!((g_cs_file != 0) && (g_cs_file_buffer != NULL))") )
        {
            __debugbreak();
        }
        if ( g_cs_file )
        {
            FS_FCloseFile(g_cs_file);
            g_cs_file = 0;
        }
        else
        {
            if ( !g_cs_file_buffer )
            {
                Com_Printf(24, "CloseFile failed, file number %i was not open\n", filenum);
                Scr_AddInt(-1, SCRIPTINSTANCE_CLIENT);
                return;
            }
            Com_EndParseSession();
            Z_VirtualFree(g_cs_file_buffer, 11);
            g_cs_file_buffer = 0;
        }
        Scr_AddInt(1, SCRIPTINSTANCE_CLIENT);
    }
}

void CScr_FPrintln()
{
    CScr_FPrint_internal(0);
}

void __cdecl CScr_FPrint_internal(bool commaBetweenFields)
{
    int value; // [esp+10h] [ebp-10h]
    char *s; // [esp+14h] [ebp-Ch]
    unsigned int arg; // [esp+18h] [ebp-8h]
    int filenum; // [esp+1Ch] [ebp-4h]

    if ( (unsigned int)Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) > 1 )
    {
        filenum = Scr_GetInt(0, SCRIPTINSTANCE_CLIENT);
        if ( (unsigned int)filenum < 2 )
        {
            if ( g_cs_file )
            {
                for ( arg = 1; arg < Scr_GetNumParam(SCRIPTINSTANCE_CLIENT); ++arg )
                {
                    s = Scr_GetString(arg, SCRIPTINSTANCE_CLIENT);
                    FS_Write(s, strlen(s), g_cs_file);
                    if ( commaBetweenFields )
                        FS_Write(",", 1u, g_cs_file);
                }
                FS_Write("\n", 1u, g_cs_file);
                value = Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) - 1;
                Scr_AddInt(value, SCRIPTINSTANCE_CLIENT);
            }
            else
            {
                Com_Printf(24, "FPrintln failed, file number %i was not open for writing\n", filenum);
                Scr_AddInt(-1, SCRIPTINSTANCE_CLIENT);
            }
        }
        else
        {
            Com_Printf(24, "FPrintln failed, invalid file number %i\n", filenum);
            Scr_AddInt(-1, SCRIPTINSTANCE_CLIENT);
        }
    }
    else
    {
        Com_Printf(24, "fprintln requires at least 2 parameters (file, output)\n");
        Scr_AddInt(-1, SCRIPTINSTANCE_CLIENT);
    }
}

void CScr_FPrintFields()
{
    CScr_FPrint_internal(1);
}

void CScr_FReadLn()
{
    int ArgCountOnLine; // [esp+0h] [ebp-18h]
    int value; // [esp+4h] [ebp-14h]
    bool eof; // [esp+8h] [ebp-10h]
    const char *buf; // [esp+Ch] [ebp-Ch] BYREF
    const char *token; // [esp+10h] [ebp-8h]
    int filenum; // [esp+14h] [ebp-4h]

    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) )
    {
        filenum = Scr_GetInt(0, SCRIPTINSTANCE_CLIENT);
        if ( filenum )
        {
            Com_Printf(24, "freadln failed, invalid file number %i\n", filenum);
            Scr_AddInt(-1, SCRIPTINSTANCE_CLIENT);
        }
        else if ( g_cs_file_buffer )
        {
            buf = g_cs_file_buffer;
            if ( g_cs_file_linemark.lines )
            {
                Com_ParseReturnToMark(&buf, &g_cs_file_linemark);
                Com_SkipRestOfLine(&buf);
                Com_ParseSetMark(&buf, &g_cs_file_linemark);
                token = (const char *)Com_Parse(&buf);
                eof = *token == 0;
                Com_ParseReturnToMark(&buf, &g_cs_file_linemark);
                if ( eof )
                {
                    Scr_AddInt(-1, SCRIPTINSTANCE_CLIENT);
                }
                else
                {
                    ArgCountOnLine = Com_GetArgCountOnLine(&buf);
                    Scr_AddInt(ArgCountOnLine, SCRIPTINSTANCE_CLIENT);
                }
            }
            else
            {
                Com_ParseSetMark(&buf, &g_cs_file_linemark);
                value = Com_GetArgCountOnLine(&buf);
                Scr_AddInt(value, SCRIPTINSTANCE_CLIENT);
            }
        }
        else
        {
            Com_Printf(24, "freadln failed, file number %i was not open for reading\n", filenum);
            Scr_AddInt(-1, SCRIPTINSTANCE_CLIENT);
        }
    }
    else
    {
        Com_Printf(24, "freadln requires a parameter - the file to read from\n");
        Scr_AddInt(-1, SCRIPTINSTANCE_CLIENT);
    }
}

void CScr_FGetArg()
{
    const char *buf; // [esp+0h] [ebp-14h] BYREF
    int arg; // [esp+4h] [ebp-10h]
    int i; // [esp+8h] [ebp-Ch]
    const char *token; // [esp+Ch] [ebp-8h]
    int filenum; // [esp+10h] [ebp-4h]

    if ( (unsigned int)Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) > 1 )
    {
        filenum = Scr_GetInt(0, SCRIPTINSTANCE_CLIENT);
        arg = Scr_GetInt(1u, SCRIPTINSTANCE_CLIENT);
        if ( (unsigned int)filenum < 2 )
        {
            if ( arg >= 0 )
            {
                if ( g_cs_file_buffer )
                {
                    token = 0;
                    buf = g_cs_file_buffer;
                    Com_ParseReturnToMark(&buf, &g_cs_file_linemark);
                    for ( i = 0; i <= arg; ++i )
                    {
                        token = (const char *)Com_ParseOnLine(&buf);
                        if ( !*token )
                        {
                            Com_Printf(
                                24,
                                "FGetArg failed, there aren't %i arguments on this line, there are only %i arguments\n",
                                arg + 1,
                                i);
                            Scr_AddString((char *)"", SCRIPTINSTANCE_CLIENT);
                            return;
                        }
                    }
                    Scr_AddString((char *)token, SCRIPTINSTANCE_CLIENT);
                }
                else
                {
                    Com_Printf(24, "FGetArg failed, file number %i was not open for reading\n", filenum);
                    Scr_AddString((char *)"", SCRIPTINSTANCE_CLIENT);
                }
            }
            else
            {
                Com_Printf(24, "FGetArg failed, invalid argument number %i\n", arg);
                Scr_AddString((char *)"", SCRIPTINSTANCE_CLIENT);
            }
        }
        else
        {
            Com_Printf(24, "FGetArg failed, invalid file number %i\n", filenum);
            Scr_AddString((char *)"", SCRIPTINSTANCE_CLIENT);
        }
    }
    else
    {
        Com_Printf(24, "FGetArg requires at least 2 parameters (file, string)\n");
        Scr_AddString((char *)"", SCRIPTINSTANCE_CLIENT);
    }
}

void CScr_SetUI3DTextureWindow()
{
    unsigned int windowId; // [esp+10h] [ebp-14h]
    float x; // [esp+14h] [ebp-10h]
    float y; // [esp+18h] [ebp-Ch]
    float h; // [esp+1Ch] [ebp-8h]
    float w; // [esp+20h] [ebp-4h]

    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) != 5 )
        Scr_Error(
            SCRIPTINSTANCE_CLIENT,
            "Incorrect number of parameters\nUSAGE: ui3dsetwindow(<window id>, <x>, <y>, <width>, <height>)\n",
            0);
    windowId = Scr_GetInt(0, SCRIPTINSTANCE_CLIENT);
    x = Scr_GetFloat(1u, SCRIPTINSTANCE_CLIENT);
    y = Scr_GetFloat(2u, SCRIPTINSTANCE_CLIENT);
    w = Scr_GetFloat(3u, SCRIPTINSTANCE_CLIENT);
    h = Scr_GetFloat(4u, SCRIPTINSTANCE_CLIENT);
    R_UI3D_SetupTextureWindow(windowId, x, y, w, h);
}

void CScr_GetWeaponModel()
{
    char *v0; // eax
    const WeaponDef *WeaponDef; // eax
    char *Name; // [esp+0h] [ebp-18h]
    char *value; // [esp+4h] [ebp-14h]
    unsigned int weaponModel; // [esp+8h] [ebp-10h]
    unsigned int iWeaponIndex; // [esp+Ch] [ebp-Ch]
    int viewmodel; // [esp+10h] [ebp-8h]
    char *pszWeaponName; // [esp+14h] [ebp-4h]

    pszWeaponName = Scr_GetString(0, SCRIPTINSTANCE_CLIENT);
    iWeaponIndex = CG_GetWeaponIndexForName(pszWeaponName);
    weaponModel = 0;
    viewmodel = 0;
    if ( iWeaponIndex )
    {
        if ( (unsigned int)Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) >= 2 )
        {
            weaponModel = Scr_GetInt(1u, SCRIPTINSTANCE_CLIENT);
            if ( weaponModel < 0x100 )
            {
                if ( !BG_GetWeaponDef(iWeaponIndex)->worldModel[weaponModel] )
                    weaponModel = 0;
            }
            else
            {
                weaponModel = 0;
            }
        }
        if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) == 3 )
            viewmodel = Scr_GetInt(2u, SCRIPTINSTANCE_CLIENT);
        WeaponDef = BG_GetWeaponDef(iWeaponIndex);
        if ( viewmodel )
        {
            value = (char *)XModelGetName(WeaponDef->gunXModel[weaponModel]);
            Scr_AddString(value, SCRIPTINSTANCE_CLIENT);
        }
        else
        {
            Name = (char *)XModelGetName(WeaponDef->worldModel[weaponModel]);
            Scr_AddString(Name, SCRIPTINSTANCE_CLIENT);
        }
    }
    else
    {
        if ( *pszWeaponName )
        {
            if ( I_stricmp(pszWeaponName, "none") )
            {
                v0 = va("unknown weapon '%s' in getWeaponModel\n", pszWeaponName);
                Com_Printf(24, v0);
            }
        }
        Scr_AddString((char *)"", SCRIPTINSTANCE_CLIENT);
    }
}

void(__cdecl *__cdecl CScr_GetFunction(const char **pName, int *type))()
{
    unsigned int i; // [esp+18h] [ebp-4h]

    for (i = 0; i < 0x9A; ++i)
    {
        if (!strcmp(*pName, client_functions[i].actionString))
        {
            *pName = client_functions[i].actionString;
            *type = client_functions[i].type;
            return client_functions[i].actionFunc;
        }
    }
    return CScr_GetFunctionProjectSpecific(pName, type);
}

void __cdecl CScr_Print(scr_entref_t entref)
{
    print();
}

void __cdecl CScrCmd_Delete(scr_entref_t entref)
{
    bool v1; // [esp+7h] [ebp-11h]
    fake_centity_s *fcent; // [esp+14h] [ebp-4h]

    fcent = &cg_fakeEntitiesArray[512 * entref.client - 1024 + entref.entnum];
    if ( !fcent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp", 6480, 0, "%s", "fcent") )
        __debugbreak();
    if ( !cg_fakeEntitiesInuseArray[512 * entref.client + fcent - &cg_fakeEntitiesArray[512 * entref.client]]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                    6481,
                    0,
                    "%s",
                    "CG_FakeEntInUse( localClientNum, fcent )") )
    {
        __debugbreak();
    }
    CScr_NotifyNum(entref.client, entref.entnum, 0, cscr_const.death, 0);
    CScr_Notify(entref.client, &fcent->cent, cscr_const.entityshutdown, 0);
    if ( CL_LocalClient_IsFirstActive(entref.client) )
        v1 = fcent != (fake_centity_s *)-4 && ((*((unsigned int *)&fcent->cent + 201) >> 8) & 1) != 0;
    else
        v1 = 0;
    if ( v1 )
        *((unsigned int *)&fcent->cent + 201) &= ~0x100u;
    CScr_FreeEntity(&fcent->cent, entref.client);
    fcent->flags |= 4u;
}

void __cdecl CScr_FreeEntity(centity_s *cent, int clientNum)
{
    if (!cent
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\cgame\\../cgame/cg_scr_main.h", 407, 0, "%s", "cent"))
    {
        __debugbreak();
    }
    if (gScrClassMap[1]->entArrayId)
    {
        CScr_FreeEntityFields(cent);
        Scr_FreeEntityNum(cent->nextState.number + 1536 * clientNum, 0, SCRIPTINSTANCE_CLIENT);
    }
}

void __cdecl CScrCmd_ForceDelete(scr_entref_t entref)
{
    bool v1; // [esp+7h] [ebp-11h]
    fake_centity_s *fcent; // [esp+14h] [ebp-4h]

    fcent = &cg_fakeEntitiesArray[512 * entref.client - 1024 + entref.entnum];
    if ( !fcent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp", 6512, 0, "%s", "fcent") )
        __debugbreak();
    if ( !cg_fakeEntitiesInuseArray[512 * entref.client + fcent - &cg_fakeEntitiesArray[512 * entref.client]]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                    6513,
                    0,
                    "%s",
                    "CG_FakeEntInUse( localClientNum, fcent )") )
    {
        __debugbreak();
    }
    CScr_NotifyNum(entref.client, entref.entnum, 0, cscr_const.death, 0);
    CScr_Notify(entref.client, &fcent->cent, cscr_const.entityshutdown, 0);
    if ( CL_LocalClient_IsFirstActive(entref.client) )
        v1 = fcent != (fake_centity_s *)-4 && ((*((unsigned int *)&fcent->cent + 201) >> 8) & 1) != 0;
    else
        v1 = 0;
    if ( v1 )
        *((unsigned int *)&fcent->cent + 201) &= ~0x100u;
    CG_Free(entref.client, entref.entnum);
}

void __cdecl CScrCmd_SetModel(scr_entref_t entref)
{
    char *error; // [esp+0h] [ebp-18h]
    centity_s *cent; // [esp+Ch] [ebp-Ch]
    char *modelName; // [esp+10h] [ebp-8h]
    int modelIndex; // [esp+14h] [ebp-4h]
    int modelIndexa; // [esp+14h] [ebp-4h]

    cent = CG_GetEntity(entref.client, entref.entnum);
    if ( !cent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp", 6544, 0, "%s", "cent") )
        __debugbreak();
    if ( cent->nextState.eType != 6 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "SetModel can only be called on a script mover\n", 1);
    modelName = Scr_GetString(0, SCRIPTINSTANCE_CLIENT);
    if ( *modelName )
    {
        if ( *modelName == 42 )
        {
            modelIndex = atoi(modelName + 1);
            if ( modelIndex != (unsigned __int16)modelIndex
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                            6564,
                            0,
                            "%s",
                            "modelIndex == (modelNameIndex_t)modelIndex") )
            {
                __debugbreak();
            }
            cent->nextState.index.brushmodel = modelIndex;
        }
        else
        {
            modelIndexa = CG_GetModelIndex(modelName, entref.client);
            if ( modelIndexa < 0 )
            {
                error = va("model '%s' not precached\n", modelName);
                Scr_Error(SCRIPTINSTANCE_CLIENT, error, 1);
                if ( !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                                6576,
                                0,
                                "%s",
                                "modelIndex >= 0") )
                    __debugbreak();
            }
            if ( modelIndexa != (unsigned __int16)modelIndexa
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                            6577,
                            0,
                            "%s",
                            "modelIndex == (modelNameIndex_t)modelIndex") )
            {
                __debugbreak();
            }
            cent->nextState.index.brushmodel = modelIndexa;
            *((unsigned int *)cent + 201) |= 2u;
        }
        CG_ScriptMover_GetDObj(entref.client, cent);
    }
    else
    {
        cent->nextState.index.brushmodel = 0;
    }
}

void __cdecl CScr_Attach(scr_entref_t entref)
{
    char *v1; // eax
    const char *v2; // eax
    int i; // [esp+1Ch] [ebp-1Ch]
    int localClientNum; // [esp+20h] [ebp-18h]
    char *modelName; // [esp+24h] [ebp-14h]
    int attachSuccessful; // [esp+28h] [ebp-10h]
    unsigned int realTagName; // [esp+2Ch] [ebp-Ch]
    centity_s *ent; // [esp+30h] [ebp-8h]
    int modelIndex; // [esp+34h] [ebp-4h]

    if ( entref.classnum )
    {
        Scr_ParamError(0, "Attach param isn't an entity.", SCRIPTINSTANCE_CLIENT);
    }
    else
    {
        ent = CG_GetEntity(entref.client, entref.entnum);
        if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp", 6610, 0, "%s", "ent") )
            __debugbreak();
        localClientNum = ent->pose.localClientNum;
        modelName = Scr_GetString(0, SCRIPTINSTANCE_CLIENT);
        if ( (unsigned int)Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) < 2 )
            realTagName = SL_FindLowercaseString("tag_origin", SCRIPTINSTANCE_SERVER);
        else
            realTagName = CScr_GetConstServerString(1u);
        modelIndex = CG_GetModelIndex(modelName, localClientNum);
        if ( modelIndex != -1 )
        {
            attachSuccessful = 0;
            for ( i = 0; i < 2; ++i )
            {
                if ( !ent->attachModelNames[i] )
                {
                    ent->attachModelNames[i] = modelIndex;
                    ent->attachTagNames[i] = realTagName;
                    attachSuccessful = 1;
                    break;
                }
            }
            if ( attachSuccessful )
            {
                CG_DestructibleUpdateDObj(ent);
            }
            else
            {
                v1 = SL_ConvertToString(realTagName, SCRIPTINSTANCE_SERVER);
                v2 = va("failed to attach model '%s' to tag '%s'", modelName, v1);
                Scr_Error(v2, 0);
            }
        }
    }
}


unsigned int __cdecl CScr_GetConstServerString(unsigned int index)
{
    static int currentCacheIndex;
    static int stringCache[2][2];
    static unsigned int stringValue[3];

    char *v2; // eax
    char *v4; // [esp-4h] [ebp-18h]
    int i; // [esp+0h] [ebp-14h]
    char *string; // [esp+4h] [ebp-10h]
    int clientStringId; // [esp+Ch] [ebp-8h]
    unsigned int serverStringId; // [esp+10h] [ebp-4h]

    clientStringId = Scr_GetConstString(index, SCRIPTINSTANCE_CLIENT);
    for (i = 0; i < 2; ++i)
    {
        if (stringCache[i][0] == clientStringId)
        {
            v4 = SL_ConvertToString(stringValue[2 * i], SCRIPTINSTANCE_SERVER);
            v2 = SL_ConvertToString(stringCache[i][0], SCRIPTINSTANCE_CLIENT);
            if (I_stricmp(v2, v4))
            {
                if (!Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\cgame\\../cgame/cg_scr_main.h",
                    526,
                    0,
                    "%s",
                    "I_stricmp( SL_ConvertToString(stringCache[i][0], SCRIPTINSTANCE_CLIENT), SL_ConvertToString(stringCache["
                    "i][1], SCRIPTINSTANCE_SERVER) ) == 0"))
                    __debugbreak();
            }
            return stringValue[2 * i];
        }
    }
    if (stringCache[currentCacheIndex][0])
    {
        SL_RemoveRefToString(SCRIPTINSTANCE_CLIENT, stringCache[currentCacheIndex][0]);
        SL_RemoveRefToString(SCRIPTINSTANCE_SERVER, stringValue[2 * currentCacheIndex]);
        stringCache[currentCacheIndex][0] = 0;
        stringValue[2 * currentCacheIndex] = 0;
    }
    string = SL_ConvertToString(clientStringId, SCRIPTINSTANCE_CLIENT);
    serverStringId = SL_FindLowercaseString(string, SCRIPTINSTANCE_SERVER);
    if (serverStringId)
    {
        SL_AddRefToString(clientStringId, SCRIPTINSTANCE_CLIENT);
        SL_AddRefToString(serverStringId, SCRIPTINSTANCE_SERVER);
        stringCache[currentCacheIndex][0] = clientStringId;
        stringValue[2 * currentCacheIndex++] = serverStringId;
        currentCacheIndex %= 2;
    }
    return serverStringId;
}

void __cdecl CScrCmd_GetSpeed(scr_entref_t entref)
{
    double v1; // st7
    snapshot_s *nextSnap; // [esp+28h] [ebp-64h]
    float next[3]; // [esp+48h] [ebp-44h] BYREF
    float velocity_out[3]; // [esp+54h] [ebp-38h] BYREF
    unsigned int time; // [esp+60h] [ebp-2Ch]
    float current[3]; // [esp+64h] [ebp-28h] BYREF
    cg_s *LocalClientGlobals; // [esp+70h] [ebp-1Ch]
    cg_s *cgameGlob; // [esp+74h] [ebp-18h]
    int localClientNum; // [esp+78h] [ebp-14h]
    cg_s *cgg; // [esp+7Ch] [ebp-10h]
    centity_s *pSelf; // [esp+80h] [ebp-Ch]
    fake_centity_s *pFake; // [esp+84h] [ebp-8h]
    float speed; // [esp+88h] [ebp-4h]

    speed = 0.0f;
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
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        6664,
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
    if ( pSelf->nextState.eType == 14 || pSelf->nextState.eType == 16 )
    {
        if ( pSelf->nitrousVeh )
            speed = pSelf->nitrousVeh->m_forward_vel;
        else
            speed = Vec3Length(pSelf->nextState.lerp.pos.trDelta);
    }
    else if ( CG_IsLocalPlayer(entref.entnum) )
    {
        cgg = 0;
        CG_GetLocalClientGlobals(entref.client);
        for ( localClientNum = 0; localClientNum < 1; ++localClientNum )
        {
            if ( CL_LocalClient_IsActive(localClientNum) )
            {
                cgameGlob = CG_GetLocalClientGlobals(localClientNum);
                if ( cgameGlob->nextSnap )
                {
                    nextSnap = cgameGlob->nextSnap;
                    if ( (nextSnap->ps.otherFlags & 6) != 0 && entref.entnum == nextSnap->ps.clientNum )
                    {
                        cgg = cgameGlob;
                        break;
                    }
                }
            }
        }
        if ( cgg )
        {
            if ( cgg->snap )
            {
                if ( cgg->nextSnap )
                {
                    speed = Vec3Length(cgg->predictedPlayerState.velocity);
                    if ( (LODWORD(speed) & 0x7F800000) == 0x7F800000
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                                    6706,
                                    0,
                                    "%s",
                                    "!IS_NAN(speed)") )
                    {
                        __debugbreak();
                    }
                }
            }
        }
    }
    else
    {
        LocalClientGlobals = CG_GetLocalClientGlobals(entref.client);
        if ( LocalClientGlobals->snap && LocalClientGlobals->nextSnap )
        {
            BG_EvaluateTrajectory(&pSelf->currentState.pos, LocalClientGlobals->snap->serverTime, current);
            BG_EvaluateTrajectory(&pSelf->nextState.lerp.pos, LocalClientGlobals->nextSnap->serverTime, next);
            velocity_out[0] = next[0] - current[0];
            velocity_out[1] = next[1] - current[1];
            velocity_out[2] = next[2] - current[2];
            time = LocalClientGlobals->nextSnap->serverTime - LocalClientGlobals->snap->serverTime;
            if ( time )
            {
                v1 = Vec3Length(velocity_out);
                speed = v1 / (double)time * 1000.0;
            }
            if ( (LODWORD(speed) & 0x7F800000) == 0x7F800000
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                            6726,
                            0,
                            "%s",
                            "!IS_NAN(speed)") )
            {
                __debugbreak();
            }
        }
    }
    if ( (LODWORD(speed) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp", 6730, 0, "%s", "!IS_NAN(speed)") )
    {
        __debugbreak();
    }
    Scr_AddFloat(speed, SCRIPTINSTANCE_CLIENT);
}

char __cdecl CG_IsLocalPlayer(int entnumber)
{
    snapshot_s *nextSnap; // [esp+4h] [ebp-Ch]
    cg_s *cgameGlob; // [esp+8h] [ebp-8h]
    int localClientNum; // [esp+Ch] [ebp-4h]

    for ( localClientNum = 0; localClientNum < 1; ++localClientNum )
    {
        if ( CL_LocalClient_IsActive(localClientNum) )
        {
            cgameGlob = CG_GetLocalClientGlobals(localClientNum);
            if ( cgameGlob->nextSnap )
            {
                nextSnap = cgameGlob->nextSnap;
                if ( (nextSnap->ps.otherFlags & 6) != 0 && entnumber == nextSnap->ps.clientNum )
                    return 1;
            }
        }
    }
    return 0;
}

// LWSS ADD
void CScrCmd_GetStance(scr_entref_t entref)
{
    int i = Scr_GetInt(0, SCRIPTINSTANCE_CLIENT);

    if (i)
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "Trying to get a local client index for a client '%d' that is not a local client.", i);
    }

    int pm_flags = CG_GetPredictedPlayerState(i)->pm_flags;

    if (pm_flags & 1)
        Scr_AddConstString(cscr_const.prone, SCRIPTINSTANCE_CLIENT);
    if(pm_flags & 2)
        Scr_AddConstString(cscr_const.crouch, SCRIPTINSTANCE_CLIENT);

    Scr_AddConstString(cscr_const.stand, SCRIPTINSTANCE_CLIENT);
}
// LWSS END

void __cdecl CScrCmd_GetEntNum(scr_entref_t entref)
{
    Scr_AddInt(entref.entnum, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScrCmd_GetEntityNumber(scr_entref_t entref)
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
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        6762,
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
    Scr_AddInt(pSelf->nextState.number, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScrCmd_Earthquake(scr_entref_t entref)
{
    float Float; // [esp+20h] [ebp-1Ch]
    float source[3]; // [esp+24h] [ebp-18h] BYREF
    int duration; // [esp+30h] [ebp-Ch]
    float radius; // [esp+34h] [ebp-8h]
    float scale; // [esp+38h] [ebp-4h]

    scale = Scr_GetFloat(0, SCRIPTINSTANCE_CLIENT);
    Float = Scr_GetFloat(1u, SCRIPTINSTANCE_CLIENT);
    duration = (int)((float)(Float * 1000.0) + 9.313225746154785e-10);
    Scr_GetVector(2u, source, SCRIPTINSTANCE_CLIENT);
    radius = Scr_GetFloat(3u, SCRIPTINSTANCE_CLIENT);
    CG_StartShakeCamera(entref.client, scale, duration, source, radius);
}

void __cdecl CScrCmd_MakeLight(scr_entref_t entref)
{
    centity_s *pSelf; // [esp+108h] [ebp-34h]
    int primaryLightIndex; // [esp+10Ch] [ebp-30h]
    const ComPrimaryLight *light; // [esp+110h] [ebp-2Ch]
    float facingDir[3]; // [esp+114h] [ebp-28h] BYREF
    float facingAngles[3]; // [esp+120h] [ebp-1Ch] BYREF
    fake_centity_s *pFake; // [esp+12Ch] [ebp-10h]
    //float normalizedColor[3]; // [esp+130h] [ebp-Ch] BYREF
    float normalizedColor[4]; // [esp+130h] [ebp-Ch] BYREF // KISAKTODO? this is technically feeding an invalid 4th color into the [3] but I guess it dont matter

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
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        6788,
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
    primaryLightIndex = Scr_GetInt(0, SCRIPTINSTANCE_CLIENT);
    light = Com_GetPrimaryLight(primaryLightIndex);
    pSelf->nextState.index.primaryLight = primaryLightIndex;
    if ( pSelf->nextState.index.primaryLight != primaryLightIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                    6802,
                    1,
                    "pSelf->nextState.index.primaryLight == primaryLightIndex\n\t%i, %i",
                    pSelf->nextState.index.primaryLight,
                    primaryLightIndex) )
    {
        __debugbreak();
    }
    pSelf->nextState.lerp.u.primaryLight.intensity = ColorNormalize(light->color, normalizedColor);
    Byte4PackRgba(normalizedColor, pSelf->nextState.lerp.u.primaryLight.colorAndExp);
    pSelf->nextState.lerp.u.primaryLight.colorAndExp[3] = light->exponent;
    pSelf->nextState.lerp.u.primaryLight.radius = light->radius;
    pSelf->nextState.lerp.u.primaryLight.cosHalfFovOuter = light->cosHalfFovOuter;
    pSelf->nextState.lerp.u.primaryLight.cosHalfFovInner = light->cosHalfFovInner;
    pSelf->currentState.u.primaryLight.intensity = pSelf->nextState.lerp.u.primaryLight.intensity;
    Byte4PackRgba(normalizedColor, pSelf->currentState.u.primaryLight.colorAndExp);
    pSelf->currentState.u.primaryLight.colorAndExp[3] = light->exponent;
    pSelf->currentState.u.primaryLight.radius = light->radius;
    pSelf->currentState.u.primaryLight.cosHalfFovOuter = light->cosHalfFovOuter;
    pSelf->currentState.u.primaryLight.cosHalfFovInner = light->cosHalfFovInner;
    facingDir[0] = -light->dir[0];
    facingDir[1] = -light->dir[1];
    facingDir[2] = -light->dir[2];
    vectoangles(facingDir, facingAngles);
    CG_SetAngle(pSelf, facingAngles);
    CG_SetOrigin(pSelf, light->origin);
    pSelf->nextState.eType = ET_PRIMARY_LIGHT;
    pSelf->nextState.lerp.eFlags |= 0x20u;
    CG_LinkEntity(entref.client, entref.entnum);
    DynEnt_FixupLightConstraints(entref.client, entref.entnum, primaryLightIndex);
}

void __cdecl Byte4PackRgba(const float *from, unsigned __int8 *to)
{
    unsigned __int8 v2; // [esp+0h] [ebp-90h]
    unsigned __int8 v3; // [esp+4h] [ebp-8Ch]
    unsigned __int8 v4; // [esp+8h] [ebp-88h]
    unsigned __int8 v5; // [esp+Ch] [ebp-84h]
    int v6; // [esp+18h] [ebp-78h]
    int v7; // [esp+38h] [ebp-58h]
    int v8; // [esp+58h] [ebp-38h]
    int v9; // [esp+78h] [ebp-18h]

    if ( (int)((float)(255.0 * *from) + 9.313225746154785e-10) < 255 )
        v9 = (int)((float)(255.0 * *from) + 9.313225746154785e-10);
    else
        v9 = 255;
    if ( v9 > 0 )
        v5 = v9;
    else
        v5 = 0;
    *to = v5;
    if ( (int)((float)(255.0 * from[1]) + 9.313225746154785e-10) < 255 )
        v8 = (int)((float)(255.0 * from[1]) + 9.313225746154785e-10);
    else
        v8 = 255;
    if ( v8 > 0 )
        v4 = v8;
    else
        v4 = 0;
    to[1] = v4;
    if ( (int)((float)(255.0 * from[2]) + 9.313225746154785e-10) < 255 )
        v7 = (int)((float)(255.0 * from[2]) + 9.313225746154785e-10);
    else
        v7 = 255;
    if ( v7 > 0 )
        v3 = v7;
    else
        v3 = 0;
    to[2] = v3;
    if ( (int)((float)(255.0 * from[3]) + 9.313225746154785e-10) < 255 )
        v6 = (int)((float)(255.0 * from[3]) + 9.313225746154785e-10);
    else
        v6 = 255;
    if ( v6 > 0 )
        v2 = v6;
    else
        v2 = 0;
    to[3] = v2;
}

void __cdecl CScr_GetLightColor(scr_entref_t entref)
{
    centity_s *pSelf; // [esp+Ch] [ebp-18h]
    float unpackedColor[4]; // [esp+14h] [ebp-10h] BYREF

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        6844,
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
    if ( pSelf->nextState.eType != 10 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "Function can only be called on a 'light' entity.", 0);
    Byte4UnpackRgba((const unsigned __int8 *)&pSelf->nextState.lerp.u, unpackedColor);
    Scr_AddVector(unpackedColor, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_SetLightColor(scr_entref_t entref)
{
    centity_s *pSelf; // [esp+88h] [ebp-1Ch]
    unsigned __int8 exponent; // [esp+93h] [ebp-11h]
    float unpackedColor[4]; // [esp+94h] [ebp-10h] BYREF

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        6867,
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
    if ( pSelf->nextState.eType != 10 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "Function can only be called on a 'light' entity", 0);
    Scr_GetVector(0, unpackedColor, SCRIPTINSTANCE_CLIENT);
    unpackedColor[3] = 0.0f;
    exponent = pSelf->nextState.lerp.u.primaryLight.colorAndExp[3];
    Byte4PackRgba(unpackedColor, (unsigned __int8 *)&pSelf->nextState.lerp.u);
    pSelf->nextState.lerp.u.primaryLight.colorAndExp[3] = exponent;
}

void __cdecl CScr_GetLightIntensity(scr_entref_t entref)
{
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
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        6893,
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
    if ( pSelf->nextState.eType != 10 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "Function can only be called on a 'light' entity", 0);
    Scr_AddFloat(pSelf->nextState.lerp.u.turret.gunAngles[1], SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_SetLightIntensity(scr_entref_t entref)
{
    int v1; // [esp+0h] [ebp-1Ch]
    centity_s *pSelf; // [esp+10h] [ebp-Ch]
    float intensity; // [esp+14h] [ebp-8h]

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        6915,
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
    if ( pSelf->nextState.eType != 10 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "Function can only be called on a 'light' entity", 0);
    intensity = Scr_GetFloat(0, SCRIPTINSTANCE_CLIENT);
    if ( intensity < -0.001 )
        Scr_ParamError(0, "intensity must be >= 0", SCRIPTINSTANCE_CLIENT);
    if ( (float)(intensity - 0.0) < 0.0 )
        v1 = 0;
    else
        v1 = LODWORD(intensity);
    pSelf->nextState.lerp.u.loopFx.period = v1;
}

void __cdecl CScr_GetLightRadius(scr_entref_t entref)
{
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
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        6939,
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
    if ( pSelf->nextState.eType != 10 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "Function can only be called on a 'light' entity", 0);
    Scr_AddFloat(pSelf->nextState.lerp.u.turret.gunAngles[2], SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_SetLightRadius(scr_entref_t entref)
{
    int v1; // [esp+8h] [ebp-2Ch]
    float v2; // [esp+10h] [ebp-24h]
    char *error; // [esp+14h] [ebp-20h]
    centity_s *pSelf; // [esp+24h] [ebp-10h]
    float radius; // [esp+28h] [ebp-Ch]
    const ComPrimaryLight *refLight; // [esp+30h] [ebp-4h]

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        6963,
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
    if ( pSelf->nextState.eType != 10 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "Function can only be called on a 'light' entity", 0);
    refLight = Com_GetPrimaryLight(pSelf->nextState.index.brushmodel);
    if ( !refLight
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp", 6968, 0, "%s", "refLight") )
    {
        __debugbreak();
    }
    radius = Scr_GetFloat(0, SCRIPTINSTANCE_CLIENT);
    if ( radius >= -0.001 )
    {
        if ( radius > (float)(refLight->radius + 0.001) )
        {
            error = va("radius must be less than the bsp radius (%g)", refLight->radius);
            Scr_ParamError(0, error, SCRIPTINSTANCE_CLIENT);
        }
    }
    else
    {
        Scr_ParamError(0, "radius must be >= 0", SCRIPTINSTANCE_CLIENT);
    }
    if ( (float)(radius - refLight->radius) < 0.0 )
        v2 = radius;
    else
        v2 = refLight->radius;
    if ( (float)(0.0 - radius) < 0.0 )
        v1 = LODWORD(v2);
    else
        v1 = 0;
    pSelf->nextState.lerp.u.actor.team = v1;
}

void __cdecl CScr_GetLightFovInner(scr_entref_t entref)
{
    double v1; // xmm0_8
    long double v2; // [esp+8h] [ebp-1Ch]
    centity_s *pSelf; // [esp+18h] [ebp-Ch]

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        6994,
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
    if ( pSelf->nextState.eType != 10 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "Function can only be called on a 'light' entity", 0);
    //LODWORD(v2) = pSelf->nextState.lerp.u.destructibleHit.modelState3;
    //v1 = *(float *)&v2;
    //__libm_sse2_acos(v2);
    //*(float *)&v1 = v1;
    //Scr_AddFloat((float)(*(float *)&v1 * 2.0) * 57.295776, SCRIPTINSTANCE_CLIENT);
    Scr_AddFloat((float)(acos(pSelf->nextState.lerp.u.destructibleHit.modelState3) * 2.0) * 57.295776, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_GetLightFovOuter(scr_entref_t entref)
{
    double v1; // xmm0_8
    long double v2; // [esp+8h] [ebp-1Ch]
    centity_s *pSelf; // [esp+18h] [ebp-Ch]

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        7017,
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
    if ( pSelf->nextState.eType != 10 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "Function can only be called on a 'light' entity", 0);
    //LODWORD(v2) = pSelf->nextState.lerp.u.turret.ownerNum;
    //v1 = *(float *)&v2;
    //__libm_sse2_acos(v2);
    //*(float *)&v1 = v1;
    //Scr_AddFloat((float)(*(float *)&v1 * 2.0) * 57.295776, SCRIPTINSTANCE_CLIENT);
    Scr_AddFloat((float)(acos(pSelf->nextState.lerp.u.turret.ownerNum) * 2.0) * 57.295776, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_SetLightFovRange(scr_entref_t entref)
{
    const char *v1; // eax
    long double v2; // [esp+10h] [ebp-4Ch]
    long double v3; // [esp+10h] [ebp-4Ch]
    float v4; // [esp+10h] [ebp-4Ch]
    float v5; // [esp+14h] [ebp-48h]
    float v6; // [esp+18h] [ebp-44h]
    float v7; // [esp+28h] [ebp-34h]
    float v8; // [esp+30h] [ebp-2Ch]
    centity_s *pSelf; // [esp+40h] [ebp-1Ch]
    float cosHalfFovOuter; // [esp+44h] [ebp-18h]
    float fovInner; // [esp+48h] [ebp-14h]
    float fovOuter; // [esp+50h] [ebp-Ch]
    float cosHalfFovInner; // [esp+54h] [ebp-8h]
    float cosHalfFovInnera; // [esp+54h] [ebp-8h]
    const ComPrimaryLight *refLight; // [esp+58h] [ebp-4h]

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        7045,
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
    if ( pSelf->nextState.eType != 10 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "Function can only be called on a 'light' entity", 0);
    refLight = Com_GetPrimaryLight(pSelf->nextState.index.brushmodel);
    if ( !refLight
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp", 7050, 0, "%s", "refLight") )
    {
        __debugbreak();
    }
    fovOuter = Scr_GetFloat(0, SCRIPTINSTANCE_CLIENT);
    if ( fovOuter < 0.99900001 || fovOuter >= 136.00101 )
        Scr_ParamError(0, "outer fov must be in the range of 1 to 136", SCRIPTINSTANCE_CLIENT);
    
    //__libm_sse2_cos(v2);
    fovOuter = cos(fovOuter);
    cosHalfFovOuter = (float)(fovOuter * 0.017453292) * 0.5;
    if ( (float)(refLight->cosHalfFovOuter - 0.001) > cosHalfFovOuter )
        Scr_ParamError(0, "outer fov cannot be larger than the fov when the map was compiled", SCRIPTINSTANCE_CLIENT);
    if ( (float)(cosHalfFovOuter - 1.0) < 0.0 )
        v8 = (float)(fovOuter * 0.017453292) * 0.5;
    else
        v8 = 1.0f;
    if ( (float)(refLight->cosHalfFovOuter - cosHalfFovOuter) < 0.0 )
        v6 = v8;
    else
        v6 = refLight->cosHalfFovOuter;
    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) == 2 )
    {
        fovInner = Scr_GetFloat(1u, SCRIPTINSTANCE_CLIENT);
        if ( fovInner < -0.001 || fovInner >= (float)(fovOuter + 0.001) )
            Scr_ParamError(1u, "inner fov must be in the range of 0 to outer fov", SCRIPTINSTANCE_CLIENT);
        //__libm_sse2_cos(v3);
        fovInner = cos(fovInner);
        cosHalfFovInner = (float)(fovInner * 0.017453292) * 0.5;
        if ( (float)(cosHalfFovInner - 1.0) < 0.0 )
            v7 = (float)(fovInner * 0.017453292) * 0.5;
        else
            v7 = 1.0f;
        if ( (float)((float)(v6 + 0.001) - cosHalfFovInner) < 0.0 )
            v5 = v7;
        else
            v5 = v6 + 0.001;
        cosHalfFovInnera = v5;
    }
    else
    {
        if ( (float)(refLight->cosHalfFovInner - (float)(v6 + 0.001)) < 0.0 )
            v4 = refLight->cosHalfFovInner;
        else
            v4 = v6 + 0.001;
        cosHalfFovInnera = v4;
    }
    if ( v6 <= 0.0 || cosHalfFovInnera <= v6 || cosHalfFovInnera > 1.0 )
    {
        v1 = va("%g, %g", v6, cosHalfFovInnera);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        7073,
                        0,
                        "%s\n\t%s",
                        "0.0f < cosHalfFovOuter && cosHalfFovOuter < cosHalfFovInner && cosHalfFovInner <= 1.0f",
                        v1) )
            __debugbreak();
    }
    pSelf->nextState.lerp.u.primaryLight.cosHalfFovOuter = v6;
    pSelf->nextState.lerp.u.turret.heatVal = cosHalfFovInnera;
}

void __cdecl CScr_GetLightExponent(scr_entref_t entref)
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
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        7091,
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
    if ( pSelf->nextState.eType != 10 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "Function can only be called on a 'light' entity", 0);
    Scr_AddInt(pSelf->nextState.lerp.u.primaryLight.colorAndExp[3], SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_SetLightExponent(scr_entref_t entref)
{
    centity_s *pSelf; // [esp+Ch] [ebp-Ch]
    int exponent; // [esp+10h] [ebp-8h]

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        7113,
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
    if ( pSelf->nextState.eType != 10 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "Function can only be called on a 'light' entity", 0);
    exponent = Scr_GetInt(0, SCRIPTINSTANCE_CLIENT);
    if ( (unsigned int)exponent > 0x64 )
        Scr_ParamError(0, "exponent must be in the range from 0 to 100", SCRIPTINSTANCE_CLIENT);
    pSelf->nextState.lerp.u.primaryLight.colorAndExp[3] = exponent;
}

void __cdecl CScrCmdGetCommandTimes(float *pfTotalTime, float *pfAccelTime, float *pfDecelTime)
{
    float fTotalTimeRoundedUp; // [esp+0h] [ebp-8h]
    int iNumParms; // [esp+4h] [ebp-4h]

    *pfTotalTime = Scr_GetFloat(1u, SCRIPTINSTANCE_CLIENT);
    if ( *pfTotalTime <= 0.0 )
        Scr_ParamError(1u, "total time must be positive", SCRIPTINSTANCE_CLIENT);
    if ( *pfTotalTime < 0.001 )
        *pfTotalTime = 0.001f;
    iNumParms = Scr_GetNumParam(SCRIPTINSTANCE_CLIENT);
    if ( iNumParms <= 2 )
    {
        *pfAccelTime = 0.0f;
        *pfDecelTime = 0.0f;
    }
    else
    {
        *pfAccelTime = Scr_GetFloat(2u, SCRIPTINSTANCE_CLIENT);
        if ( *pfAccelTime < 0.0 )
            Scr_ParamError(2u, "accel time must be nonnegative", SCRIPTINSTANCE_CLIENT);
        if ( iNumParms <= 3 )
        {
            *pfDecelTime = 0.0f;
        }
        else
        {
            *pfDecelTime = Scr_GetFloat(3u, SCRIPTINSTANCE_CLIENT);
            if ( *pfDecelTime < 0.0 )
                Scr_ParamError(3u, "decel time must be nonnegative", SCRIPTINSTANCE_CLIENT);
        }
    }
    if ( (float)(*pfAccelTime + *pfDecelTime) > *pfTotalTime )
    {
        fTotalTimeRoundedUp = *pfTotalTime * 1.0000005;
        if ( (float)(*pfAccelTime + *pfDecelTime) <= fTotalTimeRoundedUp )
            *pfTotalTime = fTotalTimeRoundedUp;
        else
            Scr_Error(SCRIPTINSTANCE_CLIENT, "accel time plus decel time is greater than total time", 0);
    }
}

void __cdecl CScrCmd_MoveGravity(scr_entref_t entref)
{
    char *error; // [esp+1Ch] [ebp-40h]
    float velocity[3]; // [esp+38h] [ebp-24h] BYREF
    centity_s *pSelf; // [esp+44h] [ebp-18h]
    float origin[3]; // [esp+48h] [ebp-14h] BYREF
    float fTotalTime; // [esp+54h] [ebp-8h]
    fake_centity_s *pFake; // [esp+58h] [ebp-4h]

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
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        7579,
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
    Scr_GetVector(0, velocity, SCRIPTINSTANCE_CLIENT);
    if ( (LODWORD(velocity[0]) & 0x7F800000) == 0x7F800000
        || (LODWORD(velocity[1]) & 0x7F800000) == 0x7F800000
        || (LODWORD(velocity[2]) & 0x7F800000) == 0x7F800000 )
    {
        error = va(
                            "invalid velocity parameter in client movegravity command: %f %f %f",
                            velocity[0],
                            velocity[1],
                            velocity[2]);
        Scr_Error(SCRIPTINSTANCE_CLIENT, error, 0);
    }
    origin[0] = pSelf->pose.origin[0];
    origin[1] = pSelf->pose.origin[1];
    origin[2] = pSelf->pose.origin[2];
    fTotalTime = Scr_GetFloat(1u, SCRIPTINSTANCE_CLIENT);
    CScriptMover_MoveGravity(&pSelf->nextState.lerp.pos, velocity, fTotalTime, origin);
}

void __cdecl CScriptMover_MoveGravity(trajectory_t *pTr, const float *velocity, float fTotalTime, float *vCurrPos)
{
    if (pTr->trType)
        BG_EvaluateTrajectory(pTr, bgs->time, vCurrPos);
    pTr->trTime = CG_GetLocalClientGlobals(0)->time;
    pTr->trDuration = (int)(float)(fTotalTime * 10000.0);
    pTr->trBase[0] = *vCurrPos;
    pTr->trBase[1] = vCurrPos[1];
    pTr->trBase[2] = vCurrPos[2];
    if (!pTr->trDuration
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp", 7553, 0, "%s", "pTr->trDuration"))
    {
        __debugbreak();
    }
    pTr->trDelta[0] = *velocity;
    pTr->trDelta[1] = velocity[1];
    pTr->trDelta[2] = velocity[2];
    if (((LODWORD(pTr->trDelta[0]) & 0x7F800000) == 0x7F800000
        || (LODWORD(pTr->trDelta[1]) & 0x7F800000) == 0x7F800000
        || (LODWORD(pTr->trDelta[2]) & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
            7555,
            0,
            "%s",
            "!IS_NAN((pTr->trDelta)[0]) && !IS_NAN((pTr->trDelta)[1]) && !IS_NAN((pTr->trDelta)[2])"))
    {
        __debugbreak();
    }
    pTr->trType = 6;
    BG_EvaluateTrajectory(pTr, bgs->time, vCurrPos);
}

void __cdecl CScrCmd_MoveTo(scr_entref_t entref)
{
    centity_s *pSelf; // [esp+18h] [ebp-20h]
    float fTotalTime; // [esp+1Ch] [ebp-1Ch] BYREF
    float fAccelTime; // [esp+20h] [ebp-18h] BYREF
    float fDecelTime; // [esp+24h] [ebp-14h] BYREF
    fake_centity_s *pFake; // [esp+28h] [ebp-10h]
    float vPos[3]; // [esp+2Ch] [ebp-Ch] BYREF

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
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        7610,
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
    if ( pFake )
        pFake->flags |= 1u;
    Scr_GetVector(0, vPos, SCRIPTINSTANCE_CLIENT);
    CScrCmdGetCommandTimes(&fTotalTime, &fAccelTime, &fDecelTime);
    CScriptMover_Move(pSelf, vPos, fTotalTime, fAccelTime, fDecelTime);
}

void __cdecl CScriptMover_Move(
                centity_s *pEnt,
                const float *vPos,
                float fTotalTime,
                float fAccelTime,
                float fDecelTime)
{
    float origin[3]; // [esp+2Ch] [ebp-Ch] BYREF

    if ( !pEnt->cScriptMover )
        pEnt->cScriptMover = (cgScriptMover_s *)MT_Alloc(96, 22, SCRIPTINSTANCE_SERVER);
    origin[0] = pEnt->pose.origin[0];
    origin[1] = pEnt->pose.origin[1];
    origin[2] = pEnt->pose.origin[2];
    CScriptMover_SetupMove(
        &pEnt->nextState.lerp.pos,
        vPos,
        fTotalTime,
        fAccelTime,
        fDecelTime,
        origin,
        &pEnt->cScriptMover->speed,
        &pEnt->cScriptMover->midTime,
        &pEnt->cScriptMover->decelTime,
        pEnt->cScriptMover->pos1,
        pEnt->cScriptMover->pos2,
        pEnt->cScriptMover->pos3);
}

void __cdecl CScriptMover_SetupMove(
    trajectory_t *pTr,
    const float *vPos,
    float fTotalTime,
    float fAccelTime,
    float fDecelTime,
    float *vCurrPos,
    float *pfSpeed,
    float *pfMidTime,
    float *pfDecelTime,
    float *vPos1,
    float *vPos2,
    float *vPos3)
{
    float v12; // [esp+0h] [ebp-8Ch]
    float v13; // [esp+28h] [ebp-64h]
    float v14; // [esp+44h] [ebp-48h]
    float fDist; // [esp+6Ch] [ebp-20h]
    float fDelta; // [esp+70h] [ebp-1Ch]
    float fDeltaa; // [esp+70h] [ebp-1Ch]
    float vMaxSpeed[3]; // [esp+74h] [ebp-18h] BYREF
    float vMove[3]; // [esp+80h] [ebp-Ch] BYREF

    vMove[0] = *vPos - *vCurrPos;
    vMove[1] = vPos[1] - vCurrPos[1];
    vMove[2] = vPos[2] - vCurrPos[2];
    if (pTr->trType)
        BG_EvaluateTrajectory(pTr, bgs->time, vCurrPos);
    if (fAccelTime == 0.0 && fDecelTime == 0.0)
    {
        pTr->trTime =bgs->time;
        pTr->trDuration = (int)(float)(fTotalTime * 1000.0);
        *pfMidTime = fTotalTime;
        *pfDecelTime = 0.0f;
        *vPos3 = *vPos;
        vPos3[1] = vPos[1];
        vPos3[2] = vPos[2];
        pTr->trBase[0] = *vCurrPos;
        pTr->trBase[1] = vCurrPos[1];
        pTr->trBase[2] = vCurrPos[2];
        if (!pTr->trDuration
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                7353,
                0,
                "%s",
                "pTr->trDuration"))
        {
            __debugbreak();
        }
        fDelta = 1000.0 / (float)pTr->trDuration;
        pTr->trDelta[0] = fDelta * vMove[0];
        pTr->trDelta[1] = fDelta * vMove[1];
        pTr->trDelta[2] = fDelta * vMove[2];
        if (((LODWORD(pTr->trDelta[0]) & 0x7F800000) == 0x7F800000
            || (LODWORD(pTr->trDelta[1]) & 0x7F800000) == 0x7F800000
            || (LODWORD(pTr->trDelta[2]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                7356,
                0,
                "%s",
                "!IS_NAN((pTr->trDelta)[0]) && !IS_NAN((pTr->trDelta)[1]) && !IS_NAN((pTr->trDelta)[2])"))
        {
            __debugbreak();
        }
        pTr->trType = 4;
        BG_EvaluateTrajectory(pTr, bgs->time, vCurrPos);
    }
    else
    {
        *pfMidTime = (float)(fTotalTime - fAccelTime) - fDecelTime;
        *pfDecelTime = fDecelTime;
        fDist = Vec3Length(vMove);
        if ((float)((float)((float)(2.0 * fTotalTime) - fAccelTime) - fDecelTime) == 0.0
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                7368,
                0,
                "%s",
                "(2.0f * fTotalTime) - fAccelTime - fDecelTime"))
        {
            __debugbreak();
        }
        *pfSpeed = (float)(2.0 * fDist) / (float)((float)((float)(2.0 * fTotalTime) - fAccelTime) - fDecelTime);
        Vec3NormalizeTo(vMove, vMaxSpeed);
        v14 = *pfSpeed;
        vMaxSpeed[0] = *pfSpeed * vMaxSpeed[0];
        vMaxSpeed[1] = v14 * vMaxSpeed[1];
        vMaxSpeed[2] = v14 * vMaxSpeed[2];
        if (fAccelTime == 0.0)
        {
            *vPos1 = *vCurrPos;
            vPos1[1] = vCurrPos[1];
            vPos1[2] = vCurrPos[2];
            if (*pfMidTime == 0.0)
            {
                pTr->trTime = bgs->time;
                pTr->trDuration = (int)(float)(*pfDecelTime * 1000.0);
                pTr->trBase[0] = *vCurrPos;
                pTr->trBase[1] = vCurrPos[1];
                pTr->trBase[2] = vCurrPos[2];
                pTr->trDelta[0] = vMaxSpeed[0];
                pTr->trDelta[1] = vMaxSpeed[1];
                pTr->trDelta[2] = vMaxSpeed[2];
                if (((LODWORD(pTr->trDelta[0]) & 0x7F800000) == 0x7F800000
                    || (LODWORD(pTr->trDelta[1]) & 0x7F800000) == 0x7F800000
                    || (LODWORD(pTr->trDelta[2]) & 0x7F800000) == 0x7F800000)
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        7411,
                        0,
                        "%s",
                        "!IS_NAN((pTr->trDelta)[0]) && !IS_NAN((pTr->trDelta)[1]) && !IS_NAN((pTr->trDelta)[2])"))
                {
                    __debugbreak();
                }
                pTr->trType = 9;
            }
            else
            {
                pTr->trTime = bgs->time;
                pTr->trDuration = (int)(float)(*pfMidTime * 1000.0);
                pTr->trBase[0] = *vCurrPos;
                pTr->trBase[1] = vCurrPos[1];
                pTr->trBase[2] = vCurrPos[2];
                v13 = *pfMidTime;
                vMove[0] = *pfMidTime * vMaxSpeed[0];
                vMove[1] = v13 * vMaxSpeed[1];
                vMove[2] = v13 * vMaxSpeed[2];
                if (!pTr->trDuration
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        7397,
                        0,
                        "%s",
                        "pTr->trDuration"))
                {
                    __debugbreak();
                }
                fDeltaa = 1000.0 / (float)pTr->trDuration;
                pTr->trDelta[0] = fDeltaa * vMove[0];
                pTr->trDelta[1] = fDeltaa * vMove[1];
                pTr->trDelta[2] = fDeltaa * vMove[2];
                if (((LODWORD(pTr->trDelta[0]) & 0x7F800000) == 0x7F800000
                    || (LODWORD(pTr->trDelta[1]) & 0x7F800000) == 0x7F800000
                    || (LODWORD(pTr->trDelta[2]) & 0x7F800000) == 0x7F800000)
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        7400,
                        0,
                        "%s",
                        "!IS_NAN((pTr->trDelta)[0]) && !IS_NAN((pTr->trDelta)[1]) && !IS_NAN((pTr->trDelta)[2])"))
                {
                    __debugbreak();
                }
                pTr->trType = 4;
            }
        }
        else
        {
            pTr->trTime = bgs->time;
            pTr->trDuration = (int)(float)(fAccelTime * 1000.0);
            pTr->trBase[0] = *vCurrPos;
            pTr->trBase[1] = vCurrPos[1];
            pTr->trBase[2] = vCurrPos[2];
            pTr->trDelta[0] = vMaxSpeed[0];
            pTr->trDelta[1] = vMaxSpeed[1];
            pTr->trDelta[2] = vMaxSpeed[2];
            if (((LODWORD(pTr->trDelta[0]) & 0x7F800000) == 0x7F800000
                || (LODWORD(pTr->trDelta[1]) & 0x7F800000) == 0x7F800000
                || (LODWORD(pTr->trDelta[2]) & 0x7F800000) == 0x7F800000)
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                    7380,
                    0,
                    "%s",
                    "!IS_NAN((pTr->trDelta)[0]) && !IS_NAN((pTr->trDelta)[1]) && !IS_NAN((pTr->trDelta)[2])"))
            {
                __debugbreak();
            }
            pTr->trType = 8;
            BG_EvaluateTrajectory(
                pTr,
                pTr->trDuration + bgs->time,
                vPos1);
        }
        v12 = *pfMidTime;
        *vPos2 = (float)(*pfMidTime * vMaxSpeed[0]) + *vPos1;
        vPos2[1] = (float)(v12 * vMaxSpeed[1]) + vPos1[1];
        vPos2[2] = (float)(v12 * vMaxSpeed[2]) + vPos1[2];
        *vPos3 = *vPos;
        vPos3[1] = vPos[1];
        vPos3[2] = vPos[2];
        BG_EvaluateTrajectory(pTr, bgs->time, vCurrPos);
    }
}

void __cdecl CScrEnt_MoveAxis(scr_entref_t entref, int iAxis)
{
    centity_s *pSelf; // [esp+1Ch] [ebp-24h]
    float fTotalTime; // [esp+20h] [ebp-20h] BYREF
    float fAccelTime; // [esp+24h] [ebp-1Ch] BYREF
    float fDecelTime; // [esp+28h] [ebp-18h] BYREF
    fake_centity_s *pFake; // [esp+2Ch] [ebp-14h]
    float vPos[3]; // [esp+30h] [ebp-10h] BYREF
    float fMove; // [esp+3Ch] [ebp-4h]

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
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        7630,
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
    if ( pFake )
        pFake->flags |= 1u;
    fMove = Scr_GetFloat(0, SCRIPTINSTANCE_CLIENT);
    CScrCmdGetCommandTimes(&fTotalTime, &fAccelTime, &fDecelTime);
    vPos[0] = pSelf->pose.origin[0];
    vPos[1] = pSelf->pose.origin[1];
    vPos[2] = pSelf->pose.origin[2];
    vPos[iAxis] = vPos[iAxis] + fMove;
    CScriptMover_Move(pSelf, vPos, fTotalTime, fAccelTime, fDecelTime);
}

void __cdecl CScrEntCmd_MoveX(scr_entref_t entref)
{
    CScrEnt_MoveAxis(entref, 0);
}

void __cdecl CScrEntCmd_MoveY(scr_entref_t entref)
{
    CScrEnt_MoveAxis(entref, 1);
}

void __cdecl CScrEntCmd_MoveZ(scr_entref_t entref)
{
    CScrEnt_MoveAxis(entref, 2);
}

void __cdecl CScriptEnt_RotateAxis(scr_entref_t entref, int iAxis)
{
    centity_s *pSelf; // [esp+1Ch] [ebp-24h]
    float fTotalTime; // [esp+20h] [ebp-20h] BYREF
    float fAccelTime; // [esp+24h] [ebp-1Ch] BYREF
    float fDecelTime; // [esp+28h] [ebp-18h] BYREF
    fake_centity_s *pFake; // [esp+2Ch] [ebp-14h]
    float vRot[3]; // [esp+30h] [ebp-10h] BYREF
    float fMove; // [esp+3Ch] [ebp-4h]

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
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        7742,
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
    if ( pFake )
        pFake->flags |= 2u;
    fMove = Scr_GetFloat(0, SCRIPTINSTANCE_CLIENT);
    CScrCmdGetCommandTimes(&fTotalTime, &fAccelTime, &fDecelTime);
    vRot[0] = pSelf->pose.angles[0];
    vRot[1] = pSelf->pose.angles[1];
    vRot[2] = pSelf->pose.angles[2];
    vRot[iAxis] = vRot[iAxis] + fMove;
    CScriptMover_Rotate(pSelf, vRot, fTotalTime, fAccelTime, fDecelTime);
}

void __cdecl CScriptMover_Rotate(
                centity_s *cent,
                const float *vRot,
                float fTotalTime,
                float fAccelTime,
                float fDecelTime)
{
    float angles[3]; // [esp+2Ch] [ebp-Ch] BYREF

    if ( !cent->cScriptMover )
        cent->cScriptMover = (cgScriptMover_s *)MT_Alloc(96, 22, SCRIPTINSTANCE_SERVER);
    angles[0] = cent->pose.angles[0];
    angles[1] = cent->pose.angles[1];
    angles[2] = cent->pose.angles[2];
    CScriptMover_SetupMove(
        &cent->nextState.lerp.apos,
        vRot,
        fTotalTime,
        fAccelTime,
        fDecelTime,
        angles,
        &cent->cScriptMover->speed,
        &cent->cScriptMover->midTime,
        &cent->cScriptMover->decelTime,
        cent->cScriptMover->pos1,
        cent->cScriptMover->pos2,
        cent->cScriptMover->pos3);
}

void __cdecl CScrCmd_RotatePitch(scr_entref_t entref)
{
    CScriptEnt_RotateAxis(entref, 0);
}

void __cdecl CScrCmd_RotateYaw(scr_entref_t entref)
{
    CScriptEnt_RotateAxis(entref, 1);
}

void __cdecl CScrCmd_RotateRoll(scr_entref_t entref)
{
    CScriptEnt_RotateAxis(entref, 2);
}

void __cdecl CScrCmd_RotateTo(scr_entref_t entref)
{
    float v1; // [esp+Ch] [ebp-4Ch]
    centity_s *pSelf; // [esp+28h] [ebp-30h]
    float fTotalTime; // [esp+2Ch] [ebp-2Ch] BYREF
    float fAccelTime; // [esp+30h] [ebp-28h] BYREF
    float vDest[3]; // [esp+34h] [ebp-24h] BYREF
    float fDecelTime; // [esp+40h] [ebp-18h] BYREF
    fake_centity_s *pFake; // [esp+44h] [ebp-14h]
    float vRot[3]; // [esp+48h] [ebp-10h] BYREF
    int i; // [esp+54h] [ebp-4h]

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
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        7839,
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
    if ( pFake )
        pFake->flags |= 2u;
    Scr_GetVector(0, vDest, SCRIPTINSTANCE_CLIENT);
    CScrCmdGetCommandTimes(&fTotalTime, &fAccelTime, &fDecelTime);
    for ( i = 0; i < 3; ++i )
    {
        v1 = AngleNormalize180(vDest[i] - pSelf->pose.angles[i]);
        vRot[i] = pSelf->pose.angles[i] + v1;
    }
    CScriptMover_Rotate(pSelf, vRot, fTotalTime, fAccelTime, fDecelTime);
}

void __cdecl CScrCmd_RotateVelocity(scr_entref_t entref)
{
    centity_s *pSelf; // [esp+18h] [ebp-20h]
    float fTotalTime; // [esp+1Ch] [ebp-1Ch] BYREF
    float fAccelTime; // [esp+20h] [ebp-18h] BYREF
    float fDecelTime; // [esp+24h] [ebp-14h] BYREF
    fake_centity_s *pFake; // [esp+28h] [ebp-10h]
    float vSpeed[3]; // [esp+2Ch] [ebp-Ch] BYREF

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
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        7877,
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
    if ( pFake )
        pFake->flags |= 2u;
    Scr_GetVector(0, vSpeed, SCRIPTINSTANCE_CLIENT);
    CScrCmdGetCommandTimes(&fTotalTime, &fAccelTime, &fDecelTime);
    CScriptMover_RotateSpeed(pSelf, vSpeed, fTotalTime, fAccelTime, fDecelTime);
}

void __cdecl CScriptMover_RotateSpeed(
                centity_s *pEnt,
                const float *vRotSpeed,
                float fTotalTime,
                float fAccelTime,
                float fDecelTime)
{
    if ( !pEnt->cScriptMover )
        pEnt->cScriptMover = (cgScriptMover_s *)MT_Alloc(96, 22, SCRIPTINSTANCE_SERVER);
    CScriptMover_SetupMoveSpeed(
        &pEnt->nextState.lerp.apos,
        vRotSpeed,
        fTotalTime,
        fAccelTime,
        fDecelTime,
        pEnt->pose.angles,
        &pEnt->cScriptMover->aSpeed,
        &pEnt->cScriptMover->aMidTime,
        &pEnt->cScriptMover->aDecelTime,
        pEnt->cScriptMover->apos1,
        pEnt->cScriptMover->apos2,
        pEnt->cScriptMover->apos3);
}

void __cdecl CScriptMover_SetupMoveSpeed(
    trajectory_t *pTr,
    const float *vSpeed,
    float fTotalTime,
    float fAccelTime,
    float fDecelTime,
    float *vCurrPos,
    float *pfSpeed,
    float *pfMidTime,
    float *pfDecelTime,
    float *vPos1,
    float *vPos2,
    float *vPos3)
{
    float v12; // [esp+Ch] [ebp-7Ch]
    trajectory_t tr; // [esp+64h] [ebp-24h] BYREF

    if (pTr->trType)
        BG_EvaluateTrajectory(pTr, bgs->time, vCurrPos);
    if (fAccelTime == 0.0 && fDecelTime == 0.0)
    {
        pTr->trTime = bgs->time;
        pTr->trDuration = (int)(float)(fTotalTime * 1000.0);
        *pfMidTime = fTotalTime;
        *pfDecelTime = 0.0f;
        pTr->trBase[0] = *vCurrPos;
        pTr->trBase[1] = vCurrPos[1];
        pTr->trBase[2] = vCurrPos[2];
        pTr->trDelta[0] = *vSpeed;
        pTr->trDelta[1] = vSpeed[1];
        pTr->trDelta[2] = vSpeed[2];
        if (((LODWORD(pTr->trDelta[0]) & 0x7F800000) == 0x7F800000
            || (LODWORD(pTr->trDelta[1]) & 0x7F800000) == 0x7F800000
            || (LODWORD(pTr->trDelta[2]) & 0x7F800000) == 0x7F800000)
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                7445,
                0,
                "%s",
                "!IS_NAN((pTr->trDelta)[0]) && !IS_NAN((pTr->trDelta)[1]) && !IS_NAN((pTr->trDelta)[2])"))
        {
            __debugbreak();
        }
        pTr->trType = 4;
        BG_EvaluateTrajectory(pTr, bgs->time, vCurrPos);
        BG_EvaluateTrajectory(
            pTr,
            pTr->trDuration + bgs->time,
            vPos3);
    }
    else
    {
        *pfMidTime = (float)(fTotalTime - fAccelTime) - fDecelTime;
        *pfDecelTime = fDecelTime;
        *pfSpeed = Vec3Length(vSpeed);
        if (fAccelTime == 0.0)
        {
            *vPos1 = *vCurrPos;
            vPos1[1] = vCurrPos[1];
            vPos1[2] = vCurrPos[2];
            if (*pfMidTime == 0.0)
            {
                pTr->trTime = bgs->time;
                pTr->trDuration = (int)(float)(*pfDecelTime * 1000.0);
                pTr->trBase[0] = *vCurrPos;
                pTr->trBase[1] = vCurrPos[1];
                pTr->trBase[2] = vCurrPos[2];
                pTr->trDelta[0] = *vSpeed;
                pTr->trDelta[1] = vSpeed[1];
                pTr->trDelta[2] = vSpeed[2];
                if (((LODWORD(pTr->trDelta[0]) & 0x7F800000) == 0x7F800000
                    || (LODWORD(pTr->trDelta[1]) & 0x7F800000) == 0x7F800000
                    || (LODWORD(pTr->trDelta[2]) & 0x7F800000) == 0x7F800000)
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        7494,
                        0,
                        "%s",
                        "!IS_NAN((pTr->trDelta)[0]) && !IS_NAN((pTr->trDelta)[1]) && !IS_NAN((pTr->trDelta)[2])"))
                {
                    __debugbreak();
                }
                pTr->trType = 9;
            }
            else
            {
                pTr->trTime = bgs->time;
                pTr->trDuration = (int)(float)(*pfMidTime * 1000.0);
                pTr->trBase[0] = *vCurrPos;
                pTr->trBase[1] = vCurrPos[1];
                pTr->trBase[2] = vCurrPos[2];
                pTr->trDelta[0] = *vSpeed;
                pTr->trDelta[1] = vSpeed[1];
                pTr->trDelta[2] = vSpeed[2];
                if (((LODWORD(pTr->trDelta[0]) & 0x7F800000) == 0x7F800000
                    || (LODWORD(pTr->trDelta[1]) & 0x7F800000) == 0x7F800000
                    || (LODWORD(pTr->trDelta[2]) & 0x7F800000) == 0x7F800000)
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        7484,
                        0,
                        "%s",
                        "!IS_NAN((pTr->trDelta)[0]) && !IS_NAN((pTr->trDelta)[1]) && !IS_NAN((pTr->trDelta)[2])"))
                {
                    __debugbreak();
                }
                pTr->trType = 4;
            }
        }
        else
        {
            pTr->trTime = bgs->time;
            pTr->trDuration = (int)(float)(fAccelTime * 1000.0);
            pTr->trBase[0] = *vCurrPos;
            pTr->trBase[1] = vCurrPos[1];
            pTr->trBase[2] = vCurrPos[2];
            pTr->trDelta[0] = *vSpeed;
            pTr->trDelta[1] = vSpeed[1];
            pTr->trDelta[2] = vSpeed[2];
            if (((LODWORD(pTr->trDelta[0]) & 0x7F800000) == 0x7F800000
                || (LODWORD(pTr->trDelta[1]) & 0x7F800000) == 0x7F800000
                || (LODWORD(pTr->trDelta[2]) & 0x7F800000) == 0x7F800000)
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                    7467,
                    0,
                    "%s",
                    "!IS_NAN((pTr->trDelta)[0]) && !IS_NAN((pTr->trDelta)[1]) && !IS_NAN((pTr->trDelta)[2])"))
            {
                __debugbreak();
            }
            pTr->trType = 8;
            BG_EvaluateTrajectory(
                pTr,
                pTr->trDuration + bgs->time,
                vPos1);
        }
        v12 = *pfMidTime;
        *vPos2 = (float)(*pfMidTime * *vSpeed) + *vPos1;
        vPos2[1] = (float)(v12 * vSpeed[1]) + vPos1[1];
        vPos2[2] = (float)(v12 * vSpeed[2]) + vPos1[2];
        if (*pfDecelTime == 0.0)
        {
            *vPos3 = *vPos2;
            vPos3[1] = vPos2[1];
            vPos3[2] = vPos2[2];
        }
        else
        {
            tr.trType = 9;
            tr.trTime = bgs->time;
            tr.trDuration = (int)(float)(*pfDecelTime * 1000.0);
            tr.trBase[0] = *vPos2;
            tr.trBase[1] = vPos2[1];
            tr.trBase[2] = vPos2[2];
            tr.trDelta[0] = *vSpeed;
            tr.trDelta[1] = vSpeed[1];
            tr.trDelta[2] = vSpeed[2];
            if (((LODWORD(tr.trDelta[0]) & 0x7F800000) == 0x7F800000
                || (LODWORD(tr.trDelta[1]) & 0x7F800000) == 0x7F800000
                || (LODWORD(tr.trDelta[2]) & 0x7F800000) == 0x7F800000)
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                    7512,
                    0,
                    "%s",
                    "!IS_NAN((tr.trDelta)[0]) && !IS_NAN((tr.trDelta)[1]) && !IS_NAN((tr.trDelta)[2])"))
            {
                __debugbreak();
            }
            BG_EvaluateTrajectory(
                &tr,
                tr.trDuration + bgs->time,
                vPos3);
        }
        BG_EvaluateTrajectory(pTr, bgs->time, vCurrPos);
    }
}

void __cdecl CScr_StartPoisoning(scr_entref_t entref)
{
    CG_GetLocalClientGlobals(entref.client)->poisoned = 1;
}

void __cdecl CScr_StopPoisoning(scr_entref_t entref)
{
    CG_GetLocalClientGlobals(entref.client)->poisoned = 0;
}

void __cdecl CScr_IsPoisoned(scr_entref_t entref)
{
    cg_s *cg; // [esp+4h] [ebp-4h]

    cg = CG_GetLocalClientGlobals(entref.client);
    Scr_AddInt(cg->poisoned, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScrCmd_GetCamPos(scr_entref_t entref)
{
    cg_s *cgameGlob; // [esp+18h] [ebp-10h]
    int localClientNum; // [esp+1Ch] [ebp-Ch]
    centity_s *pSelf; // [esp+20h] [ebp-8h]

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        7942,
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
    if ( pSelf->nextState.eType == 1 )
    {
        if ( CG_IsLocalPlayer(entref.entnum) )
        {
            for ( localClientNum = 0; localClientNum < 1; ++localClientNum )
            {
                if ( CL_LocalClient_IsActive(localClientNum) )
                {
                    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
                    if ( cgameGlob->nextSnap )
                    {
                        if ( cgameGlob->clientNum == entref.entnum )
                        {
                            Scr_AddVector(cgameGlob->refdef.vieworg, SCRIPTINSTANCE_CLIENT);
                            return;
                        }
                    }
                }
            }
        }
    }
    else
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "GetCamPos called on an entity that isn't a player...", 0);
    }
}

void __cdecl CScrCmd_GetCamAngles(scr_entref_t entref)
{
    cg_s *cgameGlob; // [esp+18h] [ebp-10h]
    int localClientNum; // [esp+1Ch] [ebp-Ch]
    centity_s *pSelf; // [esp+20h] [ebp-8h]

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        7974,
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
    if ( pSelf->nextState.eType == 1 )
    {
        if ( CG_IsLocalPlayer(entref.entnum) )
        {
            for ( localClientNum = 0; localClientNum < 1; ++localClientNum )
            {
                if ( CL_LocalClient_IsActive(localClientNum) )
                {
                    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
                    if ( cgameGlob->nextSnap )
                    {
                        if ( cgameGlob->clientNum == entref.entnum )
                        {
                            Scr_AddVector(cgameGlob->refdefViewAngles, SCRIPTINSTANCE_CLIENT);
                            return;
                        }
                    }
                }
            }
        }
    }
    else
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "GetCamAngles called on an entity that isn't a player...", 0);
    }
}

int iVal;
void __cdecl CScr_UseAnimTree(scr_entref_t entref)
{
    centity_s *pSelf; // [esp+10h] [ebp-14h]
    DObj *obj; // [esp+14h] [ebp-10h]
    scr_animtree_t animtree; // [esp+18h] [ebp-Ch]

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        8017,
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
    if ( pSelf->nextState.eType == 1
        || pSelf->nextState.eType == 2
        || pSelf->nextState.eType == 17
        || pSelf->nextState.eType == 19 )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "UseAnimTree: Can not be called on a player or an actor.", 0);
    }
    animtree.anims = Scr_GetAnimTree(0, 0, SCRIPTINSTANCE_CLIENT).anims;
    if ( animtree.anims )
        ++iVal;
    else
        Scr_ParamError(0, "Unknown anim tree\n", SCRIPTINSTANCE_CLIENT);
    obj = Com_GetClientDObj(pSelf->nextState.number, pSelf->pose.localClientNum);
    if ( !obj )
    {
        if ( (unsigned int)Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) > 1 && Scr_GetInt(1u, SCRIPTINSTANCE_CLIENT) )
        {
            Scr_AddInt(0, SCRIPTINSTANCE_CLIENT);
            return;
        }
        Scr_Error(SCRIPTINSTANCE_CLIENT, "Cannot set an animtree into an entity that has no model.", 0);
    }
    pSelf->tree = XAnimCreateTree(animtree.anims, (void *(__cdecl *)(unsigned int))CG_AllocAnimTree);
    DObjSetTree(obj, pSelf->tree);
    Scr_AddInt(1, SCRIPTINSTANCE_CLIENT);
}

XAnimTree_s *__cdecl CScr_GetEntAnimTree(centity_s *ent)
{
    char *error; // [esp+0h] [ebp-Ch]
    const DObj *obj; // [esp+4h] [ebp-8h]

    if ( ent->tree )
        return ent->tree;
    obj = Com_GetClientDObj(ent->nextState.number, ent->pose.localClientNum);
    if ( !obj )
    {
        error = va("Client entity has no model or animtree.");
        Scr_Error(SCRIPTINSTANCE_CLIENT, error, 0);
    }
    return DObjGetTree(obj);
}

void __cdecl CScr_HasAnimTree(scr_entref_t entref)
{
    centity_s *pSelf; // [esp+Ch] [ebp-Ch]

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        8095,
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
    if ( pSelf->tree )
        Scr_AddInt(1, SCRIPTINSTANCE_CLIENT);
    else
        Scr_AddInt(0, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_ClearAnim(scr_entref_t entref)
{
    centity_s *pSelf; // [esp+1Ch] [ebp-18h]
    XAnimTree_s *tree; // [esp+20h] [ebp-14h]
    fake_centity_s *pFake; // [esp+24h] [ebp-10h]
    unsigned int anim; // [esp+2Ch] [ebp-8h]
    float blendtime; // [esp+30h] [ebp-4h]

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
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        8120,
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
    tree = CScr_GetEntAnimTree(pSelf);
    anim = Scr_GetAnim(0, tree, SCRIPTINSTANCE_CLIENT).index;
    blendtime = Scr_GetFloat(1u, SCRIPTINSTANCE_CLIENT);
    if ( !pFake && !XAnimIsClientNode(tree, anim) )
        Scr_Error(
            SCRIPTINSTANCE_CLIENT,
            "ClearAnim: Attempting to clear a non-client animation on an entity from the server.",
            0);
    XAnimClearTreeGoalWeights(tree, anim, blendtime, -1);
}

void __cdecl CScr_ClearAnimLimited(scr_entref_t entref)
{
    centity_s *pSelf; // [esp+1Ch] [ebp-18h]
    XAnimTree_s *tree; // [esp+20h] [ebp-14h]
    fake_centity_s *pFake; // [esp+24h] [ebp-10h]
    unsigned int anim; // [esp+2Ch] [ebp-8h]
    float blendtime; // [esp+30h] [ebp-4h]

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
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        8151,
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
    tree = CScr_GetEntAnimTree(pSelf);
    anim = Scr_GetAnim(0, tree, SCRIPTINSTANCE_CLIENT).index;
    blendtime = Scr_GetFloat(1u, SCRIPTINSTANCE_CLIENT);
    if ( !pFake && !XAnimIsClientNode(tree, anim) )
        Scr_Error(
            SCRIPTINSTANCE_CLIENT,
            "ClearAnimLimited: Attempting to clear a non-client animation on an entity from the server.",
            0);
    XAnimClearGoalWeight(tree, anim, blendtime, 0xFFFFu);
}

void __cdecl CScr_SetAnimKnob(scr_entref_t entref)
{
    CScr_SetAnimKnobInternal(entref, 1, 0);
}

void __cdecl CScr_SetAnimKnobInternal(scr_entref_t entref, char flags, bool bFlagged)
{
    unsigned int notifyType; // [esp+20h] [ebp-4Ch]
    float rate; // [esp+38h] [ebp-34h]
    centity_s *pSelf; // [esp+3Ch] [ebp-30h]
    VariableUnion notifyName; // [esp+40h] [ebp-2Ch]
    XAnimTree_s *tree; // [esp+44h] [ebp-28h]
    int iNumParams; // [esp+48h] [ebp-24h]
    DObj *obj; // [esp+4Ch] [ebp-20h]
    float goalWeight; // [esp+50h] [ebp-1Ch]
    float goalTime; // [esp+54h] [ebp-18h]
    int error; // [esp+58h] [ebp-14h]
    fake_centity_s *pFake; // [esp+5Ch] [ebp-10h]
    signed int iCurrParam; // [esp+60h] [ebp-Ch]
    signed int iCurrParama; // [esp+60h] [ebp-Ch]
    unsigned int anim; // [esp+68h] [ebp-4h]

    goalWeight = 1.0f;
    goalTime = 0.2f;
    rate = 1.0f;
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
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        8183,
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
    tree = CScr_GetEntAnimTree(pSelf);
    iNumParams = Scr_GetNumParam(SCRIPTINSTANCE_CLIENT);
    iCurrParam = 0;
    notifyName.intValue = 0;
    if ( bFlagged )
    {
        if ( iNumParams <= 0 )
            Scr_Error(SCRIPTINSTANCE_CLIENT, "Not enough parameters.", 0);
        notifyName.intValue = Scr_GetConstString(0, SCRIPTINSTANCE_CLIENT);
        iCurrParam = 1;
    }
    if ( iCurrParam >= iNumParams )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "Not enough parameters.", 0);
    anim = Scr_GetAnim(iCurrParam, tree, SCRIPTINSTANCE_CLIENT).index;
    iCurrParama = iCurrParam + 1;
    if ( iCurrParama >= iNumParams )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "Not enough parameters.", 0);
    if ( !pFake && !XAnimIsClientNode(tree, anim) )
        Scr_Error(
            SCRIPTINSTANCE_CLIENT,
            "SetAnimKnob*: Attempting to play a non-client animation on an entity from the server.",
            0);
    if ( iCurrParama < iNumParams )
    {
        goalWeight = Scr_GetFloat(iCurrParama, SCRIPTINSTANCE_CLIENT);
        if ( goalWeight < 0.0 )
            Scr_ParamError(iCurrParama, "must set nonnegative weight", SCRIPTINSTANCE_CLIENT);
        ++iCurrParama;
    }
    if ( iCurrParama < iNumParams )
    {
        goalTime = Scr_GetFloat(iCurrParama, SCRIPTINSTANCE_CLIENT);
        if ( goalTime < 0.0 )
            Scr_ParamError(iCurrParama, "must set nonnegative goal time", SCRIPTINSTANCE_CLIENT);
        ++iCurrParama;
    }
    else
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "Not enough parameters.", 0);
    }
    if ( iCurrParama < iNumParams )
    {
        rate = Scr_GetFloat(iCurrParama, SCRIPTINSTANCE_CLIENT);
        if ( rate < 0.0 )
            Scr_ParamError(iCurrParama, "must set nonnegative rate", SCRIPTINSTANCE_CLIENT);
        ++iCurrParama;
    }
    else
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "Not enough parameters.", 0);
    }
    if ( iCurrParama < iNumParams )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "too many parameters", 0);
    obj = Com_GetClientDObj(pSelf->nextState.number, pSelf->pose.localClientNum);
    if ( obj )
    {
        if ( (flags & 1) != 0 )
        {
            if ( goalWeight <= 0.001 )
                notifyType = 0;
            else
                notifyType = 2;
            error = XAnimSetCompleteGoalWeightKnob(
                                obj,
                                anim,
                                goalWeight,
                                goalTime,
                                rate,
                                notifyName.stringValue,
                                notifyType,
                                (flags & 2) != 0,
                                -1);
        }
        else if ( goalWeight <= 0.001 )
        {
            error = XAnimSetGoalWeightKnob(
                                obj,
                                anim,
                                goalWeight,
                                goalTime,
                                rate,
                                notifyName.stringValue,
                                0,
                                (flags & 2) != 0,
                                -1);
        }
        else
        {
            error = XAnimSetGoalWeightKnob(
                                obj,
                                anim,
                                goalWeight,
                                goalTime,
                                rate,
                                notifyName.stringValue,
                                2u,
                                (flags & 2) != 0,
                                -1);
        }
        if ( error )
            CScr_HandleAnimError(error);
    }
}

void __cdecl CScr_HandleAnimError(int error)
{
    if ( error == 1 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "root anim is not an ancestor of the anim", 0);
    if ( error != 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                    8172,
                    0,
                    "%s",
                    "error == XANIM_ERROR_BAD_NOTIFY") )
    {
        __debugbreak();
    }
    Scr_Error(SCRIPTINSTANCE_CLIENT, "cannot flag anim since it has 0 effective goal weight", 0);
}

void __cdecl CScr_SetAnimKnobLimited(scr_entref_t entref)
{
    CScr_SetAnimKnobInternal(entref, 0, 0);
}

void __cdecl CScr_SetAnimKnobRestart(scr_entref_t entref)
{
    CScr_SetAnimKnobInternal(entref, 3, 0);
}

void __cdecl CScr_SetAnimKnobLimitedRestart(scr_entref_t entref)
{
    CScr_SetAnimKnobInternal(entref, 2, 0);
}

void __cdecl CScr_SetAnimKnobAll(scr_entref_t entref)
{
    CScr_SetAnimKnobAllInternal(entref, 1, 0);
}

void __cdecl CScr_SetAnimKnobAllInternal(scr_entref_t entref, char flags, bool bFlagged)
{
    unsigned int v3; // [esp+1Ch] [ebp-5Ch]
    unsigned int notifyType; // [esp+20h] [ebp-58h]
    float rate; // [esp+40h] [ebp-38h]
    centity_s *pSelf; // [esp+44h] [ebp-34h]
    XAnimTree_s *tree; // [esp+4Ch] [ebp-2Ch]
    int iNumParams; // [esp+50h] [ebp-28h]
    DObj *obj; // [esp+54h] [ebp-24h]
    float goalWeight; // [esp+58h] [ebp-20h]
    float goalTime; // [esp+5Ch] [ebp-1Ch]
    int error; // [esp+60h] [ebp-18h]
    fake_centity_s *pFake; // [esp+64h] [ebp-14h]
    signed int iCurrParam; // [esp+68h] [ebp-10h]
    signed int iCurrParama; // [esp+68h] [ebp-10h]
    signed int iCurrParamb; // [esp+68h] [ebp-10h]
    scr_anim_s anim; // [esp+6Ch] [ebp-Ch]
    scr_anim_s rootAnim; // [esp+74h] [ebp-4h]

    goalWeight = 1.0f;
    goalTime = 0.2f;
    rate = 1.0f;
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
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        8362,
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
    tree = CScr_GetEntAnimTree(pSelf);
    iNumParams = Scr_GetNumParam(SCRIPTINSTANCE_CLIENT);
    iCurrParam = 0;
    if ( bFlagged )
    {
        if ( iNumParams <= 0 )
            Scr_Error(SCRIPTINSTANCE_CLIENT, "Not enough parameters.", 0);
        Scr_GetConstString(0, SCRIPTINSTANCE_CLIENT);
        iCurrParam = 1;
    }
    if ( iCurrParam >= iNumParams )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "Not enough parameters.", 0);
    anim = Scr_GetAnim(iCurrParam, tree, SCRIPTINSTANCE_CLIENT);
    iCurrParama = iCurrParam + 1;
    if ( iCurrParama >= iNumParams )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "Not enough parameters.", 0);
    rootAnim = Scr_GetAnim(iCurrParama, tree, SCRIPTINSTANCE_CLIENT);
    if ( rootAnim.tree != anim.tree )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "root anim is not in the same anim tree", 0);
    if ( rootAnim.index == anim.index )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "root anim is not an ancestor of the anim", 0);
    iCurrParamb = iCurrParama + 1;
    if ( !pFake && !XAnimIsClientNode(tree, anim.index) )
        Scr_Error(
            SCRIPTINSTANCE_CLIENT,
            "SetAnimKnobAll*: Attempting to play a non-client animation on an entity from the server.",
            0);
    if ( iCurrParamb < iNumParams )
    {
        goalWeight = Scr_GetFloat(iCurrParamb, SCRIPTINSTANCE_CLIENT);
        if ( goalWeight < 0.0 )
            Scr_ParamError(iCurrParamb, "must set nonnegative weight", SCRIPTINSTANCE_CLIENT);
        ++iCurrParamb;
    }
    if ( iCurrParamb < iNumParams )
    {
        goalTime = Scr_GetFloat(iCurrParamb, SCRIPTINSTANCE_CLIENT);
        if ( goalTime < 0.0 )
            Scr_ParamError(iCurrParamb, "must set nonnegative goal time", SCRIPTINSTANCE_CLIENT);
        ++iCurrParamb;
    }
    if ( iCurrParamb < iNumParams )
    {
        rate = Scr_GetFloat(iCurrParamb, SCRIPTINSTANCE_CLIENT);
        if ( rate < 0.0 )
            Scr_ParamError(iCurrParamb, "must set nonnegative rate", SCRIPTINSTANCE_CLIENT);
        ++iCurrParamb;
    }
    if ( iCurrParamb < iNumParams )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "too many parameters", 0);
    if ( !pFake )
    {
        if ( !XAnimIsClientNode(tree, rootAnim.index) )
            Scr_Error(SCRIPTINSTANCE_CLIENT, "SetAnimKnobAll*: The root index is not specified as a client animation.", 0);
        if ( !XAnimIsClientNode(tree, anim.index) )
            Scr_Error(SCRIPTINSTANCE_CLIENT, "SetAnimKnobAll*: The anim index is not specified as a client animation.", 0);
    }
    obj = Com_GetClientDObj(pSelf->nextState.number, pSelf->pose.localClientNum);
    if ( !obj )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "No model exists.", 0);
    if ( (flags & 1) != 0 )
    {
        if ( goalWeight <= 0.001 )
            notifyType = 0;
        else
            notifyType = 2;
        error = XAnimSetCompleteGoalWeightKnobAll(
                            obj,
                            anim.index,
                            rootAnim.index,
                            goalWeight,
                            goalTime,
                            rate,
                            0,
                            notifyType,
                            (flags & 2) != 0,
                            -1);
    }
    else
    {
        if ( goalWeight <= 0.001 )
            v3 = 0;
        else
            v3 = 2;
        error = XAnimSetGoalWeightKnobAll(
                            obj,
                            anim.index,
                            rootAnim.index,
                            goalWeight,
                            goalTime,
                            rate,
                            0,
                            v3,
                            (flags & 2) != 0,
                            -1);
    }
    if ( error )
        CScr_HandleAnimError(error);
}

void __cdecl CScr_SetAnimKnobAllLimited(scr_entref_t entref)
{
    CScr_SetAnimKnobAllInternal(entref, 0, 0);
}

void __cdecl CScr_SetAnimKnobAllRestart(scr_entref_t entref)
{
    CScr_SetAnimKnobAllInternal(entref, 3, 0);
}

void __cdecl CScr_SetAnimKnobAllLimitedRestart(scr_entref_t entref)
{
    CScr_SetAnimKnobAllInternal(entref, 2, 0);
}

void __cdecl CScr_SetAnim(scr_entref_t entref)
{
    CScr_SetAnimInternal(entref, 1, 0);
}

void __cdecl CScr_SetAnimInternal(scr_entref_t entref, char flags, bool bFlagged)
{
    unsigned int v3; // [esp+1Ch] [ebp-50h]
    unsigned int notifyType; // [esp+20h] [ebp-4Ch]
    float rate; // [esp+38h] [ebp-34h]
    centity_s *pSelf; // [esp+3Ch] [ebp-30h]
    VariableUnion notifyName; // [esp+40h] [ebp-2Ch]
    XAnimTree_s *tree; // [esp+44h] [ebp-28h]
    int iNumParams; // [esp+48h] [ebp-24h]
    DObj *obj; // [esp+4Ch] [ebp-20h]
    float goalWeight; // [esp+50h] [ebp-1Ch]
    float goalTime; // [esp+54h] [ebp-18h]
    int error; // [esp+58h] [ebp-14h]
    signed int iCurrParam; // [esp+60h] [ebp-Ch]
    signed int iCurrParama; // [esp+60h] [ebp-Ch]
    unsigned __int16 anim; // [esp+68h] [ebp-4h]

    goalWeight = 1.0f;
    goalTime = 0.2f;
    rate = 1.0f;
    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        8550,
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
    tree = CScr_GetEntAnimTree(pSelf);
    if ( !tree )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "SetAnim*: Entity does not have an animtree.", 0);
    iNumParams = Scr_GetNumParam(SCRIPTINSTANCE_CLIENT);
    iCurrParam = 0;
    notifyName.intValue = 0;
    if ( bFlagged )
    {
        if ( iNumParams <= 0 )
            Scr_Error(SCRIPTINSTANCE_CLIENT, "Not enough parameters.", 0);
        notifyName.intValue = Scr_GetConstString(0, SCRIPTINSTANCE_CLIENT);
        iCurrParam = 1;
    }
    if ( iCurrParam >= iNumParams )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "Not enough parameters.", 0);
    anim = Scr_GetAnim(iCurrParam, tree, SCRIPTINSTANCE_CLIENT).index;
    iCurrParama = iCurrParam + 1;
    if ( pSelf->nextState.eType == 1
        || pSelf->nextState.eType == 2
        || pSelf->nextState.eType == 17
        || pSelf->nextState.eType == 19 )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "SetAnim*: Can not be called on a player or an actor.", 0);
    }
    if ( iCurrParama < iNumParams )
    {
        goalWeight = Scr_GetFloat(iCurrParama, SCRIPTINSTANCE_CLIENT);
        if ( goalWeight < 0.0 )
            Scr_ParamError(iCurrParama, "must set nonnegative weight", SCRIPTINSTANCE_CLIENT);
        ++iCurrParama;
    }
    if ( iCurrParama < iNumParams )
    {
        goalTime = Scr_GetFloat(iCurrParama, SCRIPTINSTANCE_CLIENT);
        if ( goalTime < 0.0 )
            Scr_ParamError(iCurrParama, "must set nonnegative goal time", SCRIPTINSTANCE_CLIENT);
        ++iCurrParama;
    }
    if ( iCurrParama < iNumParams )
    {
        rate = Scr_GetFloat(iCurrParama, SCRIPTINSTANCE_CLIENT);
        if ( rate < 0.0 )
            Scr_ParamError(iCurrParama, "must set nonnegative rate", SCRIPTINSTANCE_CLIENT);
        ++iCurrParama;
    }
    if ( iCurrParama < iNumParams )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "too many parameters", 0);
    obj = Com_GetClientDObj(pSelf->nextState.number, pSelf->pose.localClientNum);
    if ( !obj )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "No model exists.", 0);
    if ( (flags & 1) != 0 )
    {
        if ( goalWeight <= 0.001 )
            notifyType = 0;
        else
            notifyType = 2;
        error = XAnimSetCompleteGoalWeight(
                            obj,
                            anim,
                            goalWeight,
                            goalTime,
                            rate,
                            notifyName.stringValue,
                            notifyType,
                            (flags & 2) != 0,
                            -1);
    }
    else
    {
        if ( goalWeight <= 0.001 )
            v3 = 0;
        else
            v3 = 2;
        error = XAnimSetGoalWeight(obj, anim, goalWeight, goalTime, rate, notifyName.stringValue, v3, (flags & 2) != 0, -1);
    }
    if ( error )
        CScr_HandleAnimError(error);
}

void __cdecl CScr_SetAnimLimited(scr_entref_t entref)
{
    CScr_SetAnimInternal(entref, 0, 0);
}

void __cdecl CScr_SetAnimRestart(scr_entref_t entref)
{
    CScr_SetAnimInternal(entref, 3, 0);
}

void __cdecl CScr_SetAnimLimitedRestart(scr_entref_t entref)
{
    CScr_SetAnimInternal(entref, 2, 0);
}

void __cdecl CScr_SetFlaggedAnim(scr_entref_t entref)
{
    CScr_SetAnimInternal(entref, 1, 1);
}

void __cdecl CScr_SetFlaggedAnimLimited(scr_entref_t entref)
{
    CScr_SetAnimInternal(entref, 0, 1);
}

void __cdecl CScr_SetFlaggedAnimRestart(scr_entref_t entref)
{
    CScr_SetAnimInternal(entref, 3, 1);
}

void __cdecl CScr_SetFlaggedAnimLimitedRestart(scr_entref_t entref)
{
    CScr_SetAnimInternal(entref, 2, 1);
}

void __cdecl CScr_SetFlaggedAnimKnob(scr_entref_t entref)
{
    CScr_SetAnimKnobInternal(entref, 1, 1);
}

void __cdecl CScr_SetFlaggedAnimKnobLimited(scr_entref_t entref)
{
    CScr_SetAnimKnobInternal(entref, 0, 1);
}

void __cdecl CScr_SetFlaggedAnimKnobRestart(scr_entref_t entref)
{
    CScr_SetAnimKnobInternal(entref, 3, 1);
}

void __cdecl CScr_SetFlaggedAnimKnobLimitedRestart(scr_entref_t entref)
{
    CScr_SetAnimKnobInternal(entref, 2, 1);
}

void __cdecl CScr_SetFlaggedAnimKnobAll(scr_entref_t entref)
{
    CScr_SetAnimKnobAllInternal(entref, 1, 1);
}

void __cdecl CScr_SetFlaggedAnimKnobAllRestart(scr_entref_t entref)
{
    CScr_SetAnimKnobAllInternal(entref, 3, 1);
}

void __cdecl CScr_AnimGetChildAt(scr_entref_t entref)
{
    centity_s *pSelf; // [esp+14h] [ebp-28h]
    XAnimTree_s *tree; // [esp+18h] [ebp-24h]
    int childIndex; // [esp+20h] [ebp-1Ch]
    scr_anim_s retAnim; // [esp+24h] [ebp-18h]
    scr_anim_s anim; // [esp+30h] [ebp-Ch]
    int numChildren; // [esp+34h] [ebp-8h]
    XAnim_s *pXAnims; // [esp+38h] [ebp-4h]

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        8957,
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
    tree = CScr_GetEntAnimTree(pSelf);
    if ( !tree )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "SetAnim*: Entity does not have an animtree.", 0);
    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) >= 3 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "Too many parameters.", 0);
    anim = Scr_GetAnim(0, tree, SCRIPTINSTANCE_CLIENT);
    if ( pSelf->nextState.eType == 1
        || pSelf->nextState.eType == 2
        || pSelf->nextState.eType == 17
        || pSelf->nextState.eType == 19 )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "SetAnim*: Can not be called on a player or an actor.", 0);
    }
    childIndex = Scr_GetInt(1u, SCRIPTINSTANCE_CLIENT);
    pXAnims = XAnimGetAnims(tree);
    numChildren = XAnimGetNumChildren(pXAnims, anim.index);
    if ( childIndex < 0 || childIndex >= numChildren )
        Scr_ParamError(childIndex, "must be a valid child index", SCRIPTINSTANCE_CLIENT);
    retAnim.tree = anim.tree;
    retAnim.index = XAnimGetChildAt(pXAnims, anim.index, childIndex);
    Scr_AddAnim(retAnim, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_AnimGetNumChildren(scr_entref_t entref)
{
    centity_s *pSelf; // [esp+14h] [ebp-20h]
    XAnimTree_s *tree; // [esp+18h] [ebp-1Ch]
    unsigned __int16 anim; // [esp+28h] [ebp-Ch]
    int numChildren; // [esp+2Ch] [ebp-8h]
    XAnim_s *pXAnims; // [esp+30h] [ebp-4h]

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        9006,
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
    tree = CScr_GetEntAnimTree(pSelf);
    if ( !tree )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "SetAnim*: Entity does not have an animtree.", 0);
    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) >= 2 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "Too many parameters.", 0);
    anim = Scr_GetAnim(0, tree, SCRIPTINSTANCE_CLIENT).index;
    if ( pSelf->nextState.eType == 1
        || pSelf->nextState.eType == 2
        || pSelf->nextState.eType == 17
        || pSelf->nextState.eType == 19 )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "SetAnim*: Can not be called on a player or an actor.", 0);
    }
    pXAnims = XAnimGetAnims(tree);
    numChildren = XAnimGetNumChildren(pXAnims, anim);
    Scr_AddInt(numChildren, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_GetAnimTime(scr_entref_t entref)
{
    const XAnim_s *Anims; // eax
    float value; // [esp+8h] [ebp-2Ch]
    centity_s *pSelf; // [esp+20h] [ebp-14h]
    XAnimTree_s *tree; // [esp+24h] [ebp-10h]
    unsigned int anim; // [esp+30h] [ebp-4h]

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        9049,
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
    tree = CScr_GetEntAnimTree(pSelf);
    anim = Scr_GetAnim(0, tree, SCRIPTINSTANCE_CLIENT).index;
    Anims = XAnimGetAnims(tree);
    if ( !XAnimHasTime(Anims, anim) )
        Scr_ParamError(0, "blended nonsynchronized animation has no concept of time", SCRIPTINSTANCE_CLIENT);
    value = XAnimGetTime(tree, anim);
    Scr_AddFloat(value, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_GetAnimCurrFrameCount(scr_entref_t entref)
{
    centity_s *pSelf; // [esp+14h] [ebp-24h]
    XAnimTree_s *tree; // [esp+1Ch] [ebp-1Ch]
    int framecount; // [esp+20h] [ebp-18h]
    float time; // [esp+28h] [ebp-10h]
    scr_anim_s anim; // [esp+30h] [ebp-8h]
    XAnim_s *anims; // [esp+34h] [ebp-4h]

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        9075,
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
    tree = CScr_GetEntAnimTree(pSelf);
    anim = Scr_GetAnim(0, 0, SCRIPTINSTANCE_CLIENT);
    anims = Scr_GetAnims(anim.tree, SCRIPTINSTANCE_CLIENT);
    if ( !XAnimIsPrimitive(anims, anim.index) )
        Scr_ParamError(0, "non-primitive animation has no concept of length", SCRIPTINSTANCE_CLIENT);
    framecount = XAnimGetFrameCount(anims, anim.index);
    time = XAnimGetTime(tree, anim.index);
    Scr_AddInt((int)(float)((float)framecount * time), SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_SetAnimTime(scr_entref_t entref)
{
    centity_s *pSelf; // [esp+1Ch] [ebp-1Ch]
    XAnimTree_s *tree; // [esp+20h] [ebp-18h]
    float time; // [esp+28h] [ebp-10h]
    unsigned __int16 anim; // [esp+30h] [ebp-8h]
    XAnim_s *anims; // [esp+34h] [ebp-4h]

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        9122,
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
    tree = CScr_GetEntAnimTree(pSelf);
    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) != 2 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "Wrong number of parameters", 0);
    anim = Scr_GetAnim(0, tree, SCRIPTINSTANCE_CLIENT).index;
    time = Scr_GetFloat(1u, SCRIPTINSTANCE_CLIENT);
    if ( time >= 0.0 )
    {
        if ( time > 1.0 )
        {
            time = 1.0f;
            Scr_ParamError(1u, "must be < 1", SCRIPTINSTANCE_CLIENT);
        }
    }
    else
    {
        time = 0.0f;
        Scr_ParamError(1u, "must be > 0", SCRIPTINSTANCE_CLIENT);
    }
    anims = XAnimGetAnims(tree);
    if ( !XAnimHasTime(anims, anim) )
        Scr_ParamError(0, "not a timed animation", SCRIPTINSTANCE_SERVER);
    if ( time == 1.0 && XAnimIsLooped(anims, anim) )
        Scr_ParamError(1u, "cannot set time 1 on looping animation", SCRIPTINSTANCE_SERVER);
    XAnimSetTime(tree, anim, time, 0xFFFFu);
}

void __cdecl CScrCmd_IsPlayer(scr_entref_t entref)
{
    centity_s *pSelf; // [esp+Ch] [ebp-Ch]

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        9171,
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
    Scr_AddInt(pSelf->nextState.eType == 1, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScrCmd_IsLocalPlayer(scr_entref_t entref)
{
    bool retVal; // [esp+Ch] [ebp-4h]

    retVal = CG_IsLocalPlayer(entref.entnum) != 0;
    Scr_AddInt(retVal, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScrCmd_IsAI(scr_entref_t entref)
{
    centity_s *pSelf; // [esp+Ch] [ebp-Ch]

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        9228,
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
    Scr_AddInt(pSelf->nextState.eType == 17, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_HasDObj(scr_entref_t entref)
{
    VariableUnion localClientNum; // [esp+Ch] [ebp-4h]

    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) != 1 )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "HasDObj: Must be passed the local client number", 0);
    localClientNum.intValue = CScr_GetLocalClientNum(0);
    if ( Com_GetClientDObj(entref.entnum, localClientNum.intValue) )
        Scr_AddInt(1, SCRIPTINSTANCE_CLIENT);
    else
        Scr_AddInt(0, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_SetCompassIcon(scr_entref_t entref)
{
    char *String; // eax
    centity_s *pSelf; // [esp+Ch] [ebp-Ch]

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
        pSelf = 0;
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        9276,
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
    String = Scr_GetString(0, SCRIPTINSTANCE_CLIENT);
    *((unsigned int *)pSelf + 201) |= 0x40000u;
    pSelf->compassMaterial = Material_Register(String, 7);
}

void __cdecl CScrCmd_IsSplitScreenHost(scr_entref_t entref)
{
    cg_s *cgameGlob; // [esp+Ch] [ebp-Ch]
    int localClientNum; // [esp+10h] [ebp-8h]
    bool bIsHost; // [esp+17h] [ebp-1h]

    bIsHost = 0;
    if ( CG_IsLocalPlayer(entref.entnum) )
    {
        for ( localClientNum = 0; localClientNum < 1; ++localClientNum )
        {
            if ( CL_LocalClient_IsActive(localClientNum) )
            {
                cgameGlob = CG_GetLocalClientGlobals(localClientNum);
                if ( cgameGlob->nextSnap )
                {
                    if ( cgameGlob->clientNum == entref.entnum && Com_LocalClient_IsPrimary(cgameGlob->localClientNum) )
                    {
                        bIsHost = 1;
                        break;
                    }
                }
            }
        }
    }
    Scr_AddBool(bIsHost, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScrCmd_IsTouching(scr_entref_t entref)
{
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    scr_entref_t v4; // [esp+38h] [ebp-BCh] BYREF
    scr_entref_t v5; // [esp+42h] [ebp-B2h]
    scr_entref_t v6; // [esp+50h] [ebp-A4h] BYREF
    scr_entref_t v7; // [esp+5Ah] [ebp-9Ah]
    scr_entref_t v8; // [esp+68h] [ebp-8Ch]
    scr_entref_t v9; // [esp+72h] [ebp-82h]
    scr_entref_t v10; // [esp+78h] [ebp-7Ch]
    scr_entref_t v11; // [esp+82h] [ebp-72h]
    DObj *dobj; // [esp+88h] [ebp-6Ch]
    scr_entref_t tempEntref; // [esp+8Ch] [ebp-68h]
    centity_s *pOther; // [esp+94h] [ebp-60h]
    float mins[3]; // [esp+98h] [ebp-5Ch] BYREF
    float absMaxs[3]; // [esp+A4h] [ebp-50h] BYREF
    scr_entref_t otherEntref; // [esp+B0h] [ebp-44h]
    float vMins[3]; // [esp+B8h] [ebp-3Ch] BYREF
    float maxs[3]; // [esp+C4h] [ebp-30h] BYREF
    centity_s *ent; // [esp+D0h] [ebp-24h]
    int bTouching; // [esp+D4h] [ebp-20h]
    centity_s *pTemp; // [esp+D8h] [ebp-1Ch]
    float vMaxs[3]; // [esp+DCh] [ebp-18h] BYREF
    float absMins[3]; // [esp+E8h] [ebp-Ch] BYREF
    int savedregs; // [esp+F4h] [ebp+0h] BYREF

    ent = CG_GetEntity(entref.client, entref.entnum);
    if (ent->nextState.solid == 0xFFFFFF
        || ((*((unsigned int *)ent + 201) >> 15) & 1) != 0 && (*((unsigned int *)ent + 201) & 0x10000) != 0 )
    {
        pTemp = ent;
        tempEntref.client = entref.client;
        tempEntref.entnum = entref.entnum;
        v7 = Scr_GetEntityRef(0, SCRIPTINSTANCE_CLIENT);
        v10 = v7;
        v11 = v7;
        otherEntref = v7;
        ent = CG_GetEntity(v7.client, v7.entnum);
        if (ent->nextState.solid == 0xFFFFFF
            || ((*((unsigned int *)ent + 201) >> 15) & 1) != 0 && (*((unsigned int *)ent + 201) & 0x10000) != 0 )
        {
            Scr_Error(SCRIPTINSTANCE_CLIENT, "istouching cannot be called on 2 brush/cylinder entities", 0);
        }
        pOther = pTemp;
        otherEntref.client = tempEntref.client;
        otherEntref.entnum = tempEntref.entnum;
    }
    else
    {
        v5 = Scr_GetEntityRef(0, SCRIPTINSTANCE_CLIENT);
        v8 = v5;
        v9 = v5;
        otherEntref = v5;
        pOther = CG_GetEntity(v5.client, v5.entnum);
    }
    if ( ent->nextState.eType == 1 || ent->nextState.eType == 17 )
    {
        mins[0] = -15.0f;
        mins[1] = -15.0f;
        mins[2] = 0.0f;
        maxs[0] = 15.0f;
        maxs[1] = 15.0f;
        maxs[2] = 70.0f;
        absMins[0] = ent->pose.origin[0] + -15.0;
        absMins[1] = ent->pose.origin[1] + -15.0;
        absMins[2] = ent->pose.origin[2] + 0.0;
        absMaxs[0] = ent->pose.origin[0] + 15.0;
        absMaxs[1] = ent->pose.origin[1] + 15.0;
        absMaxs[2] = ent->pose.origin[2] + 70.0;
    }
    else if ( ent->nextState.solid )
    {
        absMins[0] = ent->pose.absmin[0];
        absMins[1] = ent->pose.absmin[1];
        absMins[2] = ent->pose.absmin[2];
        absMaxs[0] = ent->pose.absmax[0];
        absMaxs[1] = ent->pose.absmax[1];
        absMaxs[2] = ent->pose.absmax[2];
        CG_GetEntityBModelBounds(ent, mins, maxs, 0, 0);
    }
    else
    {
        dobj = Com_GetClientDObj(ent->nextState.number, ent->pose.localClientNum);
        if ( !dobj )
        {
            Scr_AddInt(0, SCRIPTINSTANCE_CLIENT);
            return;
        }
        CG_GetEntityDobjBounds(ent, dobj, mins, maxs, absMins, absMaxs);
    }
    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp", 9420, 0, "%s", "ent") )
        __debugbreak();
    if ( maxs[0] < mins[0] )
    {
        v1 = va(
                     "entnum: %d, origin: %g %g %g",
                     ent->nextState.number,
                     ent->pose.origin[0],
                     ent->pose.origin[1],
                     ent->pose.origin[2]);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        9421,
                        0,
                        "%s\n\t%s",
                        "maxs[0] >= mins[0]",
                        v1) )
            __debugbreak();
    }
    if ( maxs[1] < mins[1] )
    {
        v2 = va(
                     "entnum: %d, origin: %g %g %g",
                     ent->nextState.number,
                     ent->pose.origin[0],
                     ent->pose.origin[1],
                     ent->pose.origin[2]);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        9422,
                        0,
                        "%s\n\t%s",
                        "maxs[1] >= mins[1]",
                        v2) )
            __debugbreak();
    }
    if ( maxs[2] < mins[2] )
    {
        v3 = va(
                     "entnum: %d, origin: %g %g %g",
                     ent->nextState.number,
                     ent->pose.origin[0],
                     ent->pose.origin[1],
                     ent->pose.origin[2]);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        9423,
                        0,
                        "%s\n\t%s",
                        "maxs[2] >= mins[2]",
                        v3) )
            __debugbreak();
    }
    vMins[0] = ent->pose.origin[0] + mins[0];
    vMins[1] = ent->pose.origin[1] + mins[1];
    vMins[2] = ent->pose.origin[2] + mins[2];
    vMaxs[0] = ent->pose.origin[0] + maxs[0];
    vMaxs[1] = ent->pose.origin[1] + maxs[1];
    vMaxs[2] = ent->pose.origin[2] + maxs[2];
    ExpandBoundsToWidth(vMins, vMaxs);
    bTouching = CG_EntityContact(vMins, vMaxs, pOther);
    Scr_AddInt(bTouching, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScrCmd_UnderWater(scr_entref_t entref)
{
    float height; // [esp+8h] [ebp-8h]
    cg_s *cg; // [esp+Ch] [ebp-4h]

    cg = CG_GetLocalClientGlobals(entref.client);
    height = CM_GetWaterHeight(cg->refdef.vieworg, 200.0, -200.0);
    if ( height <= cg->refdef.vieworg[2] )
        Scr_AddInt(0, SCRIPTINSTANCE_CLIENT);
    else
        Scr_AddInt(1, SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScrCmd_LinkTo(scr_entref_t entref)
{
    scr_entref_t v1; // [esp+14h] [ebp-38h] BYREF
    scr_entref_t v2; // [esp+1Eh] [ebp-2Eh]
    scr_entref_t v3; // [esp+2Ch] [ebp-20h]
    scr_entref_t v4; // [esp+36h] [ebp-16h]
    unsigned int tagName; // [esp+3Ch] [ebp-10h]
    scr_entref_t linkEnt; // [esp+40h] [ebp-Ch]
    centity_s *cent; // [esp+48h] [ebp-4h]

    cent = CG_GetEntity(entref.client, entref.entnum);
    v2 = Scr_GetEntityRef(0, SCRIPTINSTANCE_CLIENT);
    v3 = v2;
    v4 = v2;
    linkEnt = v2;
    tagName = CScr_GetConstServerString(1u);
    if ( tagName )
    {
        if ( linkEnt.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        9480,
                        0,
                        "%s",
                        "linkEnt.entnum < MAX_LOCAL_CENTITIES") )
        {
            __debugbreak();
        }
        if ( entref.entnum < 0x400u )
            Scr_Error(SCRIPTINSTANCE_CLIENT, "LinkTo: Only works on fake entities\n", 0);
        CG_UpdateFakeEntityLink(entref.client, cent, linkEnt.entnum, tagName);
    }
}

void __cdecl CScrCmd_Unlink(scr_entref_t entref)
{
    centity_s *cent; // [esp+8h] [ebp-4h]

    cent = CG_GetEntity(entref.client, entref.entnum);
    if ( entref.entnum < 0x400u )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "Unlink: Only works on fake entities\n", 0);
    CG_UpdateFakeEntityLink(entref.client, cent, 1023, 0);
}

void __cdecl CScr_Map_Material(scr_entref_t entref)
{
    int materialIndex; // [esp+10h] [ebp-18h]
    unsigned int numParams; // [esp+14h] [ebp-14h]
    char *materialName; // [esp+1Ch] [ebp-Ch]
    bool error; // [esp+23h] [ebp-5h]
    cg_s *cg; // [esp+24h] [ebp-4h]

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        9530,
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
    cg = CG_GetLocalClientGlobals(entref.client);
    numParams = Scr_GetNumParam(SCRIPTINSTANCE_CLIENT);
    materialIndex = Scr_GetInt(0, SCRIPTINSTANCE_CLIENT);
    materialName = Scr_GetString(1u, SCRIPTINSTANCE_CLIENT);
    error = numParams != 2;
    if ( (unsigned int)materialIndex >= 0x40 )
        error = 1;
    if ( error )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "USAGE: ent map_material( <material index 0-63 >, <material name>)\n", 0);
    else
        cg->genericMaterialMap[materialIndex] = Material_Register(materialName, 0);
}

void __cdecl CScr_Set_Filter_Pass_Material(scr_entref_t entref)
{
    int sampler1Index; // [esp+14h] [ebp-28h]
    int materialIndex; // [esp+18h] [ebp-24h]
    int filterIndex; // [esp+1Ch] [ebp-20h]
    unsigned int numParams; // [esp+20h] [ebp-1Ch]
    int targetIndex; // [esp+28h] [ebp-14h]
    bool error; // [esp+2Fh] [ebp-Dh]
    cg_s *cg; // [esp+30h] [ebp-Ch]
    int passIndex; // [esp+34h] [ebp-8h]
    int sampler0Index; // [esp+38h] [ebp-4h]

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        9554,
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
    cg = CG_GetLocalClientGlobals(entref.client);
    numParams = Scr_GetNumParam(SCRIPTINSTANCE_CLIENT);
    filterIndex = Scr_GetInt(0, SCRIPTINSTANCE_CLIENT);
    passIndex = Scr_GetInt(1u, SCRIPTINSTANCE_CLIENT);
    materialIndex = Scr_GetInt(2u, SCRIPTINSTANCE_CLIENT);
    targetIndex = Scr_GetInt(3u, SCRIPTINSTANCE_CLIENT);
    sampler0Index = Scr_GetInt(4u, SCRIPTINSTANCE_CLIENT);
    sampler1Index = Scr_GetInt(5u, SCRIPTINSTANCE_CLIENT);
    error = numParams != 6;
    if ( (unsigned int)filterIndex > 2 )
        error = 1;
    if ( (unsigned int)passIndex >= 0x10 )
        error = 1;
    if ( (unsigned int)materialIndex >= 0x40 )
        error = 1;
    if ( (unsigned int)targetIndex >= 6 )
        error = 1;
    if ( (unsigned int)sampler0Index >= 6 )
        error = 1;
    if ( (unsigned int)sampler1Index >= 6 )
        error = 1;
    if ( error )
    {
        Scr_Error(
            SCRIPTINSTANCE_CLIENT,
            "USAGE: ent set_filter_pass_material( <filterindex 0-3>, <passindex 0-15>, <materialindex 0-63>, <targetindex 0-4>,"
            " <sampler0index 0-4>, <sampler0index 0-4> )\n",
            0);
    }
    else if ( cg->genericMaterialMap[materialIndex] )
    {
        cg->genericFilterPassMaterial[filterIndex][passIndex] = cg->genericMaterialMap[materialIndex];
        cg->genericFilterPassTarget[filterIndex][passIndex] = targetIndex;
        cg->genericFilterPassSampler0[filterIndex][passIndex] = sampler0Index;
        cg->genericFilterPassSampler1[filterIndex][passIndex] = sampler1Index;
    }
    else
    {
        Scr_Error(
            SCRIPTINSTANCE_CLIENT,
            "USAGE: ent set_filter_pass_material( <filterindex 0-3>, <passindex 0-15>, <materialindex 0-63>, <targetindex 0-4>,"
            " <sampler0index 0-4>, <sampler0index 0-4> ) - no material at that index\n",
            0);
    }
}

void __cdecl CScr_Set_Filter_Pass_Enabled(scr_entref_t entref)
{
    int filterIndex; // [esp+10h] [ebp-1Ch]
    unsigned int numParams; // [esp+14h] [ebp-18h]
    bool error; // [esp+1Fh] [ebp-Dh]
    cg_s *cg; // [esp+20h] [ebp-Ch]
    int passIndex; // [esp+24h] [ebp-8h]
    int passEnable; // [esp+28h] [ebp-4h]

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        9594,
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
    cg = CG_GetLocalClientGlobals(entref.client);
    numParams = Scr_GetNumParam(SCRIPTINSTANCE_CLIENT);
    filterIndex = Scr_GetInt(0, SCRIPTINSTANCE_CLIENT);
    passIndex = Scr_GetInt(1u, SCRIPTINSTANCE_CLIENT);
    passEnable = Scr_GetInt(2u, SCRIPTINSTANCE_CLIENT);
    error = numParams != 3;
    if ( (unsigned int)filterIndex > 2 )
        error = 1;
    if ( (unsigned int)passIndex > 0x10 )
        error = 1;
    if ( error )
        Scr_Error(
            SCRIPTINSTANCE_CLIENT,
            "USAGE: ent set_filter_pass_enabled( <filterindex 0-3>, <passindex 0-16>, <enabled true/false> )\n",
            0);
    else
        cg->genericFilterPassEnabled[filterIndex][passIndex] = passEnable != 0;
}

void __cdecl CScr_Set_Filter_Pass_Quads(scr_entref_t entref)
{
    int filterIndex; // [esp+10h] [ebp-1Ch]
    unsigned int numParams; // [esp+14h] [ebp-18h]
    bool error; // [esp+1Fh] [ebp-Dh]
    cg_s *cg; // [esp+20h] [ebp-Ch]
    int passIndex; // [esp+24h] [ebp-8h]
    int passQuads; // [esp+28h] [ebp-4h]

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        9618,
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
    cg = CG_GetLocalClientGlobals(entref.client);
    numParams = Scr_GetNumParam(SCRIPTINSTANCE_CLIENT);
    filterIndex = Scr_GetInt(0, SCRIPTINSTANCE_CLIENT);
    passIndex = Scr_GetInt(1u, SCRIPTINSTANCE_CLIENT);
    passQuads = Scr_GetInt(2u, SCRIPTINSTANCE_CLIENT);
    error = numParams != 3;
    if ( (unsigned int)filterIndex > 2 )
        error = 1;
    if ( (unsigned int)passIndex > 0x10 )
        error = 1;
    if ( (unsigned int)passQuads > 0x190 )
        error = 1;
    if ( error )
        Scr_Error(
            SCRIPTINSTANCE_CLIENT,
            "USAGE: ent set_filter_pass_quads( <filterindex 0-3>, <passindex 0-15>, <passquads 0-400> )\n",
            0);
    else
        cg->genericFilterPassQuads[filterIndex][passIndex] = passQuads;
}

void __cdecl CScr_Set_Filter_Pass_Constant(scr_entref_t entref)
{
    int constantIndex; // [esp+10h] [ebp-20h]
    int filterIndex; // [esp+14h] [ebp-1Ch]
    unsigned int numParams; // [esp+18h] [ebp-18h]
    bool error; // [esp+23h] [ebp-Dh]
    cg_s *cg; // [esp+24h] [ebp-Ch]
    int passIndex; // [esp+28h] [ebp-8h]
    float value; // [esp+2Ch] [ebp-4h]

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        9643,
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
    cg = CG_GetLocalClientGlobals(entref.client);
    numParams = Scr_GetNumParam(SCRIPTINSTANCE_CLIENT);
    filterIndex = Scr_GetInt(0, SCRIPTINSTANCE_CLIENT);
    passIndex = Scr_GetInt(1u, SCRIPTINSTANCE_CLIENT);
    constantIndex = Scr_GetInt(2u, SCRIPTINSTANCE_CLIENT);
    value = Scr_GetFloat(3u, SCRIPTINSTANCE_CLIENT);
    error = numParams != 4;
    if ( (unsigned int)filterIndex > 2 )
        error = 1;
    if ( (unsigned int)passIndex >= 0x10 )
        error = 1;
    if ( (unsigned int)constantIndex >= 0x10 )
        error = 1;
    if ( error )
        Scr_Error(
            SCRIPTINSTANCE_CLIENT,
            "USAGE: ent set_filter_pass_constant( <filterindex 0-3>, <passindex 0-16>, <constantindex 0-16>, <value> )\n",
            0);
    else
        cg->genericFilterPassParam[filterIndex][passIndex][constantIndex] = value;
}

void __cdecl CScr_Set_Overlay_Material(scr_entref_t entref)
{
    int sortIndex; // [esp+14h] [ebp-1Ch]
    int materialIndex; // [esp+18h] [ebp-18h]
    unsigned int numParams; // [esp+1Ch] [ebp-14h]
    bool error; // [esp+27h] [ebp-9h]
    cg_s *cg; // [esp+28h] [ebp-8h]
    int overlayIndex; // [esp+2Ch] [ebp-4h]

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        9669,
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
    cg = CG_GetLocalClientGlobals(entref.client);
    numParams = Scr_GetNumParam(SCRIPTINSTANCE_CLIENT);
    overlayIndex = Scr_GetInt(0, SCRIPTINSTANCE_CLIENT);
    materialIndex = Scr_GetInt(1u, SCRIPTINSTANCE_CLIENT);
    sortIndex = Scr_GetInt(2u, SCRIPTINSTANCE_CLIENT);
    error = numParams != 3;
    if ( (unsigned int)overlayIndex > 2 )
        error = 1;
    if ( (unsigned int)materialIndex >= 0x40 )
        error = 1;
    if ( (unsigned int)sortIndex > 2 )
        error = 1;
    if ( error )
    {
        Scr_Error(
            SCRIPTINSTANCE_CLIENT,
            "USAGE: ent set_overlay_material( <overlayIndex 0-3>, <materialindex 0-63>, <sortindex 0-2> )\n",
            0);
    }
    else if ( cg->genericMaterialMap[materialIndex] )
    {
        cg->genericOverlayMaterial[overlayIndex] = cg->genericMaterialMap[materialIndex];
        cg->genericOverlaySortIndex[overlayIndex] = sortIndex;
    }
    else
    {
        Scr_Error(
            SCRIPTINSTANCE_CLIENT,
            "USAGE: ent set_overlay_material( <overlayIndex 0-3>, <materialindex 0-63>, <sortindex 0-2> ) - no material at that index\n",
            0);
    }
}

void __cdecl CScr_Set_Overlay_Constant(scr_entref_t entref)
{
    int constantIndex; // [esp+10h] [ebp-1Ch]
    unsigned int numParams; // [esp+14h] [ebp-18h]
    bool error; // [esp+1Fh] [ebp-Dh]
    cg_s *cg; // [esp+20h] [ebp-Ch]
    int overlayIndex; // [esp+24h] [ebp-8h]
    float value; // [esp+28h] [ebp-4h]

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        9701,
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
    cg = CG_GetLocalClientGlobals(entref.client);
    numParams = Scr_GetNumParam(SCRIPTINSTANCE_CLIENT);
    overlayIndex = Scr_GetInt(0, SCRIPTINSTANCE_CLIENT);
    constantIndex = Scr_GetInt(1u, SCRIPTINSTANCE_CLIENT);
    value = Scr_GetFloat(2u, SCRIPTINSTANCE_CLIENT);
    error = numParams != 3;
    if ( (unsigned int)overlayIndex > 2 )
        error = 1;
    if ( (unsigned int)constantIndex >= 0x10 )
        error = 1;
    if ( error )
        Scr_Error(
            SCRIPTINSTANCE_CLIENT,
            "USAGE: ent set_overlay_constant( <overlayindex 0-3>, <constantindex 0-16>, <value> )\n",
            0);
    else
        cg->genericOverlayParam[overlayIndex][constantIndex] = value;
}

void __cdecl CScr_SetInfraredVisionset(scr_entref_t entref)
{
    char v1; // [esp+3h] [ebp-25h]
    char *visionNameInfrared; // [esp+8h] [ebp-20h]
    char *String; // [esp+Ch] [ebp-1Ch]
    cg_s *cg; // [esp+1Ch] [ebp-Ch]

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        9725,
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
    if ( Scr_GetNumParam(SCRIPTINSTANCE_CLIENT) == 1 )
    {
        cg = CG_GetLocalClientGlobals(entref.client);
        String = Scr_GetString(0, SCRIPTINSTANCE_CLIENT);
        visionNameInfrared = cg->visionNameInfrared;
        do
        {
            v1 = *String;
            *visionNameInfrared++ = *String++;
        }
        while ( v1 );
        CG_VisionSetStartLerp_To(
            entref.client,
            VISIONSETMODE_INFRARED,
            VISIONSETLERP_CUSTOM_VALUE,
            cg->visionNameInfrared,
            0);
    }
    else
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "USAGE: ent SetInfraredVisionset( visionset name )\n", 0);
    }
}

void __cdecl CScr_Set_Overlay_Enabled(scr_entref_t entref)
{
    unsigned int numParams; // [esp+10h] [ebp-18h]
    bool error; // [esp+1Bh] [ebp-Dh]
    int enable; // [esp+1Ch] [ebp-Ch]
    cg_s *cg; // [esp+20h] [ebp-8h]
    int overlayIndex; // [esp+24h] [ebp-4h]

    if ( entref.classnum )
    {
        Scr_Error(SCRIPTINSTANCE_CLIENT, "not an entity", 0);
    }
    else
    {
        if ( entref.entnum >= 0x600u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        9742,
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
    cg = CG_GetLocalClientGlobals(entref.client);
    numParams = Scr_GetNumParam(SCRIPTINSTANCE_CLIENT);
    overlayIndex = Scr_GetInt(0, SCRIPTINSTANCE_CLIENT);
    enable = Scr_GetInt(1u, SCRIPTINSTANCE_CLIENT);
    error = numParams != 2;
    if ( (unsigned int)overlayIndex > 2 )
        error = 1;
    if ( error )
        Scr_Error(SCRIPTINSTANCE_CLIENT, "USAGE: ent set_overlay_enabled( <overlayindex 0-3>, <enable true/false> )\n", 0);
    else
        cg->genericOverlayEnabled[overlayIndex] = enable != 0;
}

void __cdecl CScr_LaunchRagdoll(scr_entref_t entref)
{
    centity_s *pSelf; // [esp+Ch] [ebp-FCh]
    float origin[3]; // [esp+10h] [ebp-F8h] BYREF
    fake_centity_s *pFake; // [esp+1Ch] [ebp-ECh]
    entityState_s v4; // [esp+20h] [ebp-E8h] BYREF

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
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        9764,
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
    memset(&v4, 0, sizeof(v4));
    Scr_GetVector(0, origin, SCRIPTINSTANCE_CLIENT);
    v4.lerp.pos.trBase[0] = origin[0];
    v4.lerp.pos.trBase[1] = origin[1];
    v4.lerp.pos.trBase[2] = origin[2];
    v4.lerp.pos.trType = 0;
    v4.lerp.pos.trTime = 0;
    v4.lerp.pos.trDuration = 0;
    memset(v4.lerp.pos.trDelta, 0, sizeof(v4.lerp.pos.trDelta));
    CG_PhysLaunch(entref.client, pSelf, &v4);
}

void __cdecl CScr_Show(scr_entref_t entref)
{
    centity_s *Entity; // ecx

    Entity = CG_GetEntity(entref.client, entref.entnum);
    Entity->nextState.lerp.eFlags &= ~0x20u;
}

void __cdecl CScr_Hide(scr_entref_t entref)
{
    centity_s *Entity; // ecx

    Entity = CG_GetEntity(entref.client, entref.entnum);
    Entity->nextState.lerp.eFlags |= 0x20u;
}

void (__cdecl *__cdecl CScr_GetMethod(const char **pName, int *type))(scr_entref_t)
{
    unsigned int i; // [esp+18h] [ebp-4h]

    for ( i = 0; i < ARRAY_COUNT(client_methods); ++i )
    {
        if ( !strcmp(*pName, client_methods[i].actionString) )
        {
            *pName = client_methods[i].actionString;
            *type = client_methods[i].type;
            return client_methods[i].actionFunc;
        }
    }
    return CScr_GetMethodProjectSpecific(pName, type);
}

void __cdecl CScr_LoadLevel()
{
    unsigned __int16 t; // [esp+0h] [ebp-4h]

    if ( cg_scr_data.levelscript )
    {
        t = Scr_ExecThread(SCRIPTINSTANCE_CLIENT, cg_scr_data.levelscript, 0);
        Scr_FreeThread(t, SCRIPTINSTANCE_CLIENT);
    }
}

int __cdecl CScr_SetScriptAndLabel(
                ScriptFunctions *functions,
                const char *filename,
                const char *label,
                int bEnforceExists)
{
    int func; // [esp+4h] [ebp-4h]

    if ( !cg_loadScripts || !cg_loadScripts->current.enabled )
        return 0;
    if ( functions->count >= functions->maxSize )
        Com_Error(ERR_DROP, "CODE ERROR: CScr_SetScriptAndLabel: functions->maxSize exceeded");
    func = functions->address[functions->count++];
    if ( !func )
    {
        if ( bEnforceExists )
            Com_Error(ERR_DROP, "Could not find label '%s' in script '%s'", label, filename);
    }
    return func;
}

void __cdecl CScr_SetLevelScript(ScriptFunctions *functions)
{
    char filename[64]; // [esp+0h] [ebp-48h] BYREF
    const char *mapname; // [esp+44h] [ebp-4h]

    mapname = Dvar_GetString("mapname");
    Com_sprintf(filename, 0x40u, "%s%s", "clientscripts/", mapname);
    cg_scr_data.levelscript = CScr_SetScriptAndLabel(functions, filename, "main", 0);
}

void __cdecl CG_FreeEntityDelay(centity_s *ed)
{
    unsigned __int16 hThread; // [esp+0h] [ebp-4h]

    if ( cg_scr_data.delete_ )
    {
        hThread = CScr_ExecEntThread(ed, cg_scr_data.delete_, 0);
        Scr_FreeThread(hThread, SCRIPTINSTANCE_CLIENT);
    }
}

void __cdecl CScr_FreeScripts()
{
    signed int classnum; // [esp+0h] [ebp-4h]

    for ( classnum = 0; classnum < 5; ++classnum )
        Scr_RemoveClassMap(SCRIPTINSTANCE_CLIENT, classnum);
}

unsigned int __cdecl CG_SetEntityScriptVariableInternal(const char *key, char *value)
{
    const char *v3; // eax
    int v4; // [esp+Ch] [ebp-1Ch]
    float v5; // [esp+10h] [ebp-18h]
    unsigned int index; // [esp+14h] [ebp-14h]
    int type; // [esp+18h] [ebp-10h] BYREF
    float vec[3]; // [esp+1Ch] [ebp-Ch] BYREF

    index = Scr_FindField(key, &type, SCRIPTINSTANCE_CLIENT);
    if ( !index )
        return 0;
    switch ( type )
    {
        case 2:
            Scr_AddString(value, SCRIPTINSTANCE_CLIENT);
            break;
        case 4:
            memset(vec, 0, sizeof(vec));
            sscanf(value, "%f %f %f", vec, &vec[1], &vec[2]);
            Scr_AddVector(vec, SCRIPTINSTANCE_CLIENT);
            break;
        case 5:
            v5 = atof(value);
            Scr_AddFloat(v5, SCRIPTINSTANCE_CLIENT);
            break;
        case 6:
            v4 = atoi(value);
            Scr_AddInt(v4, SCRIPTINSTANCE_CLIENT);
            break;
        default:
            v3 = va("CG_SetEntityScriptVariableInternal: bad case %d", type);
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp", 10079, 0, v3) )
                __debugbreak();
            break;
    }
    return index;
}

VariableUnion __cdecl CG_SpawnStruct(const SpawnVar *spawnVar)
{
    unsigned int index; // [esp+0h] [ebp-Ch]
    int i; // [esp+4h] [ebp-8h]
    unsigned int structId; // [esp+8h] [ebp-4h]

    if ( !spawnVar->spawnVarsValid
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                    10092,
                    0,
                    "%s",
                    "spawnVar.spawnVarsValid") )
    {
        __debugbreak();
    }
    if ( !cg_scr_data.createstruct
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                    10094,
                    0,
                    "%s",
                    "cg_scr_data.createstruct") )
    {
        __debugbreak();
    }
    Scr_AddExecThread(SCRIPTINSTANCE_CLIENT, cg_scr_data.createstruct, 0);
    structId = Scr_GetObject(0, SCRIPTINSTANCE_CLIENT);
    for ( i = 0; i < spawnVar->numSpawnVars; ++i )
    {
        index = CG_SetEntityScriptVariableInternal(spawnVar->spawnVars[i][0], spawnVar->spawnVars[i][1]);
        if ( index )
            Scr_SetStructField(structId, index, SCRIPTINSTANCE_CLIENT);
    }
    return (VariableUnion)structId;
}

void __cdecl CScr_GlassSmash(float *pos, float *dir)
{
    unsigned __int16 t; // [esp+0h] [ebp-4h]

    if ( cg_scr_data.glassSmash )
    {
        Scr_AddVector(dir, SCRIPTINSTANCE_CLIENT);
        Scr_AddVector(pos, SCRIPTINSTANCE_CLIENT);
        t = Scr_ExecThread(SCRIPTINSTANCE_CLIENT, cg_scr_data.glassSmash, 2u);
        Scr_FreeThread(t, SCRIPTINSTANCE_CLIENT);
    }
}

void __cdecl CScr_LoadStructs()
{
    int i; // [esp+0h] [ebp-14h]
    unsigned int flags; // [esp+4h] [ebp-10h]
    const char *spawnFlags; // [esp+8h] [ebp-Ch] BYREF
    unsigned __int16 hThread; // [esp+Ch] [ebp-8h]
    const char *classname; // [esp+10h] [ebp-4h] BYREF

    if ( !cg_scr_data.initstructs
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                    10165,
                    0,
                    "%s",
                    "cg_scr_data.initstructs") )
    {
        __debugbreak();
    }
    hThread = Scr_ExecThread(SCRIPTINSTANCE_CLIENT, cg_scr_data.initstructs, 0);
    Scr_FreeThread(hThread, SCRIPTINSTANCE_CLIENT);
    G_ResetEntityParsePoint();
    while ( G_ParseSpawnVars(&cg_level.spawnVar) )
    {
        G_SpawnString(&cg_level.spawnVar, "classname", "", &classname);
        if ( !I_stricmp(classname, "script_struct") )
            goto LABEL_7;
        if ( I_stricmp(classname, "node_pathnode") )
        {
            if ( I_stricmp(classname, "script_origin") )
            {
                if ( I_stricmp(classname, "script_brushmodel") && I_stricmp(classname, "script_model") )
                {
                    if ( !I_stricmp(classname, "light") )
                    {
                        G_SpawnString(&cg_level.spawnVar, "spawnflags", "", &spawnFlags);
                        flags = atoi(spawnFlags);
                        if ( !flags || (flags & 0x10) == 0 )
                        {
                            for ( i = 0; i < cg_level.spawnVar.numSpawnVars; ++i )
                            {
                                if ( !I_stricmp(cg_level.spawnVar.spawnVars[i][0], "pl#") )
                                    cg_level.spawnVar.spawnVars[i][0][2] = 0;
                            }
                            CG_SpawnStruct(&cg_level.spawnVar);
                        }
                    }
                }
                else
                {
LABEL_7:
                    CG_SpawnStruct(&cg_level.spawnVar);
                }
            }
            else
            {
                G_SpawnString(&cg_level.spawnVar, "script_label", "", &classname);
                if ( I_stricmp(classname, "random") )
                {
                    if ( !I_stricmp(classname, "looper") )
                        Com_Printf(0, "*** Sound looper warning : script_origin needs to be replaced with a script_struct\n");
                }
                else
                {
                    Com_Printf(0, "*** Sound random warning : script_origin needs to be replaced with a script_struct\n");
                }
            }
        }
        else
        {
            CG_SpawnNode();
        }
    }
}

int CG_SpawnNode()
{
    int result; // eax
    unsigned int index; // [esp+0h] [ebp-10h]
    int i; // [esp+4h] [ebp-Ch]
    int ia; // [esp+4h] [ebp-Ch]
    bool noteworthy; // [esp+Bh] [ebp-5h]
    unsigned int structId; // [esp+Ch] [ebp-4h]

    if ( !cg_level.spawnVar.spawnVarsValid
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                    10118,
                    0,
                    "%s",
                    "cg_level.spawnVar.spawnVarsValid") )
    {
        __debugbreak();
    }
    if ( !cg_scr_data.createstruct
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                    10119,
                    0,
                    "%s",
                    "cg_scr_data.createstruct") )
    {
        __debugbreak();
    }
    noteworthy = 0;
    for ( i = 0; i < cg_level.spawnVar.numSpawnVars; ++i )
    {
        if ( !I_stricmp(cg_level.spawnVar.spawnVars[i][0], "script_noteworthy") )
            noteworthy = 1;
    }
    result = noteworthy;
    if ( noteworthy )
    {
        Scr_AddExecThread(SCRIPTINSTANCE_CLIENT, cg_scr_data.createstruct, 0);
        structId = Scr_GetObject(0, SCRIPTINSTANCE_CLIENT);
        for ( ia = 0; ; ++ia )
        {
            result = ia;
            if ( ia >= cg_level.spawnVar.numSpawnVars )
                break;
            index = CG_SetEntityScriptVariableInternal(cg_level.spawnVar.spawnVars[ia][0], cg_level.spawnVar.spawnVars[ia][1]);
            if ( index )
                Scr_SetStructField(structId, index, SCRIPTINSTANCE_CLIENT);
        }
    }
    return result;
}

unsigned __int16 __cdecl CScr_ExecEntThread(centity_s *ent, int handle, unsigned int paramcount)
{
    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp", 10228, 0, "%s", "ent") )
        __debugbreak();
    return Scr_ExecEntThreadNum(
                     SCRIPTINSTANCE_CLIENT,
                     ent->nextState.number,
                     0,
                     handle,
                     paramcount,
                     ent->pose.localClientNum);
}

bool __cdecl CG_EntityContact(const float *mins, const float *maxs, const centity_s *cent)
{
    float dist; // [esp+14h] [ebp-4Ch]
    trace_t trace; // [esp+1Ch] [ebp-44h] BYREF
    float center[2]; // [esp+58h] [ebp-8h]

    if ( ((*((unsigned int *)cent + 201) >> 15) & 1) == 0 )
        return 0;
    if ( (*((unsigned int *)cent + 201) & 0x10000) != 0 )
    {
        if ( cent->pose.actor.height != 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                        10254,
                        0,
                        "%s",
                        "!cent->pose.general.mins[2]") )
        {
            __debugbreak();
        }
        if ( cent->pose.origin[2] < maxs[2] )
        {
            if ( mins[2] < (float)(cent->pose.origin[2] + cent->pose.general.maxs[2]) )
            {
                center[0] = *mins + *maxs;
                center[1] = mins[1] + maxs[1];
                center[0] = 0.5 * center[0];
                center[1] = 0.5 * center[1];
                dist = (float)(*maxs - center[0]) + cent->pose.turret.firingOffset;
                return (float)(dist * dist) > (float)((float)((float)(center[0] - cent->pose.origin[0])
                                                                                                        * (float)(center[0] - cent->pose.origin[0]))
                                                                                        + (float)((float)(center[1] - cent->pose.origin[1])
                                                                                                        * (float)(center[1] - cent->pose.origin[1])));
            }
            else
            {
                return 0;
            }
        }
        else
        {
            return 0;
        }
    }
    else if (cent->nextState.solid == 0xFFFFFF)
    {
        CM_TransformedBoxTraceExternal(
            &trace,
            vec3_origin,
            vec3_origin,
            mins,
            maxs,
            cent->nextState.index.brushmodel,
            -1,
            cent->pose.origin,
            cent->pose.angles);
        return trace.startsolid;
    }
    else
    {
        return 0;
    }
}

void __cdecl CG_DoTouchTriggers(centity_s *ent, int localClientNum)
{
    const DObj *dobj; // [esp+14h] [ebp-1878h]
    int entityList[1537]; // [esp+1Ch] [ebp-1870h] BYREF
    int contentMask; // [esp+1820h] [ebp-6Ch]
    float maxs[3]; // [esp+1824h] [ebp-68h] BYREF
    int team; // [esp+183Ch] [ebp-50h]
    float areaMins[3]; // [esp+1840h] [ebp-4Ch] BYREF
    float absMaxs[3]; // [esp+184Ch] [ebp-40h] BYREF
    cg_s *LocalClientGlobals; // [esp+1858h] [ebp-34h]
    centity_s *other; // [esp+185Ch] [ebp-30h]
    float areaMaxs[3]; // [esp+1860h] [ebp-2Ch] BYREF
    int v23; // [esp+186Ch] [ebp-20h]
    int i; // [esp+1870h] [ebp-1Ch]
    float mins[3]; // [esp+1874h] [ebp-18h] BYREF
    float absMins[3]; // [esp+1880h] [ebp-Ch] BYREF

    LocalClientGlobals = CG_GetLocalClientGlobals(localClientNum);
    team = 0;
    if ( LocalClientGlobals->frametime )
    {
        if ( ent->nextState.eType == 17 || ent->nextState.eType == 19 )
        {
            team = ent->nextState.lerp.u.actor.team & 3;
        }
        else if ( ent->nextState.eType == 1 || ent->nextState.eType == 2 )
        {
            team = LocalClientGlobals->bgs.clientinfo[ent->nextState.clientNum].team;
        }
        if ( ent->nextState.eType == 14 )
        {
            contentMask = 8;
        }
        else if ( ent->nextState.eType == 1 )
        {
            contentMask = 0x40000000;
        }
        else if ( ent->nextState.eType == 17 && team == 1 )
        {
            contentMask = 0x40000;
        }
        else if ( ent->nextState.eType == 17 && team == 2 )
        {
            contentMask = 0x80000;
        }
        else
        {
            if ( ent->nextState.eType != 17 || team )
                return;
            contentMask = 0x100000;
        }
        if ( ent->nextState.eType == 1 || ent->nextState.eType == 17 )
        {
            mins[0] = -15.0f;
            mins[1] = -15.0f;
            mins[2] = 0.0f;
            maxs[0] = 15.0f;
            maxs[1] = 15.0f;
            maxs[2] = 70.0f;
            absMins[0] = ent->pose.origin[0] + -15.0;
            absMins[1] = ent->pose.origin[1] + -15.0;
            absMins[2] = ent->pose.origin[2] + 0.0;
            absMaxs[0] = ent->pose.origin[0] + 15.0;
            absMaxs[1] = ent->pose.origin[1] + 15.0;
            absMaxs[2] = ent->pose.origin[2] + 70.0;
        }
        else
        {
            dobj = Com_GetClientDObj(ent->nextState.number, localClientNum);
            if ( !dobj )
                return;
            CG_GetEntityDobjBounds(ent, dobj, mins, maxs, absMins, absMaxs);
        }
        areaMins[0] = absMins[0] - 20.0;
        areaMins[1] = absMins[1] - 20.0;
        areaMins[2] = absMins[2] - 20.0;
        areaMaxs[0] = absMaxs[0] + 20.0;
        areaMaxs[1] = absMaxs[1] + 20.0;
        areaMaxs[2] = absMaxs[2] + 20.0;
        v23 = CG_AreaEntities(areaMins, areaMaxs, entityList, 1536, contentMask);
        areaMins[0] = ent->currentState.pos.trBase[0] + mins[0];
        areaMins[1] = ent->currentState.pos.trBase[1] + mins[1];
        areaMins[2] = ent->currentState.pos.trBase[2] + mins[2];
        areaMaxs[0] = ent->currentState.pos.trBase[0] + maxs[0];
        areaMaxs[1] = ent->currentState.pos.trBase[1] + maxs[1];
        areaMaxs[2] = ent->currentState.pos.trBase[2] + maxs[2];
        ExpandBoundsToWidth(areaMins, areaMaxs);
        for ( i = 0; i < v23; ++i )
        {
            other = CG_GetEntity(localClientNum, entityList[i]);
            if ( ((*((unsigned int *)other + 201) >> 15) & 1) != 0
                && (contentMask & other->pose.fx.triggerTime) != 0
                && CG_EntityContact(areaMins, areaMaxs, other) )
            {
                if ( Scr_IsSystemActive(1u, SCRIPTINSTANCE_CLIENT) )
                {
                    Scr_AddEntityNum(ent->nextState.number, 0, SCRIPTINSTANCE_CLIENT, localClientNum);
                    CScr_NotifyNum(localClientNum, other->nextState.number, 0, cscr_const.touch, 1u);
                    Scr_AddEntityNum(other->nextState.number, 0, SCRIPTINSTANCE_CLIENT, localClientNum);
                    CScr_NotifyNum(localClientNum, ent->nextState.number, 0, cscr_const.touch, 1u);
                }
                CG_Touch_Multi(ent, other);
            }
        }
    }
}

void __cdecl CScr_GetEntityField(unsigned int entnum, int clientNum, unsigned int offset)
{
    const char *v3; // [esp+0h] [ebp-18h]
    const char *error; // [esp+Ch] [ebp-Ch]
    const cent_field_s *f; // [esp+10h] [ebp-8h]
    centity_s *ent; // [esp+14h] [ebp-4h]

    if ( entnum >= 0x600
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                    10424,
                    0,
                    "%s",
                    "(unsigned)entnum < MAX_LOCAL_CENTITIES") )
    {
        __debugbreak();
    }
    if ( ((*((unsigned int *)&cgArray[clientNum].predictedPlayerEntity + 201) >> 1) & 1) != 0
        && cgArray[clientNum].predictedPlayerEntity.nextState.number == entnum )
    {
        ent = &cgArray[clientNum].predictedPlayerEntity;
        if ( ((*((unsigned int *)&cgArray[clientNum].predictedPlayerEntity + 201) >> 1) & 1) == 0 )
        {
            error = SEH_SafeTranslateString("EXE_ENTITY_BEFORE_SNAPSHOT");
            Scr_Error(SCRIPTINSTANCE_CLIENT, error, 1);
        }
    }
    else
    {
        ent = CG_GetEntity(clientNum, entnum);
        if ( !cgArray[clientNum].nextSnap )
        {
            v3 = SEH_SafeTranslateString("EXE_ENTITY_BEFORE_SNAPSHOT");
            Scr_Error(SCRIPTINSTANCE_CLIENT, v3, 1);
        }
    }
    if ( !Demo_IsPlaying()
        && entnum != ent->nextState.number
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                    10453,
                    0,
                    "entnum == ent->nextState.number\n\t%i, %i",
                    entnum,
                    ent->nextState.number) )
    {
        __debugbreak();
    }
    if ( offset >= 0x18
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                    10457,
                    0,
                    "%s",
                    "(unsigned)offset < ARRAY_COUNT( cent_fields ) - 1") )
    {
        __debugbreak();
    }
    f = &cent_fields[offset];
    if ( f->getter )
        f->getter(ent, f);
    else
        Scr_GetGenericField((unsigned __int8 *)ent, f->type, f->ofs, SCRIPTINSTANCE_CLIENT, 0);
}

void __cdecl CScr_GetObjectField(unsigned int classnum, unsigned int entnum, int clientNum, unsigned int offset)
{
    const char *v4; // eax

    if ( classnum )
    {
        v4 = va("bad class num %u", classnum);
        if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp", 10511, 0, v4) )
            __debugbreak();
    }
    else
    {
        CScr_GetEntityField(entnum, clientNum, offset);
    }
}

int __cdecl CScr_SetObjectField(unsigned int classnum, unsigned int entnum, int clientNum, unsigned int offset)
{
    const char *v5; // eax

    if ( !classnum )
        return CScr_SetEntityField(entnum, clientNum, offset);
    v5 = va("bad class num %u", classnum);
    if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp", 10527, 0, v5) )
        __debugbreak();
    return 1;
}

int __cdecl CScr_SetEntityField(unsigned int entnum, int clientNum, unsigned int offset)
{
    const cent_field_s *f; // [esp+8h] [ebp-8h]
    centity_s *ent; // [esp+Ch] [ebp-4h]

    if ( entnum >= 0x600
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                    10476,
                    0,
                    "%s",
                    "(unsigned)entnum < MAX_LOCAL_CENTITIES") )
    {
        __debugbreak();
    }
    ent = CG_GetEntity(clientNum, entnum);
    if ( entnum != ent->nextState.number
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                    10479,
                    0,
                    "entnum == ent->nextState.number\n\t%i, %i",
                    entnum,
                    ent->nextState.number) )
    {
        __debugbreak();
    }
    if ( offset >= 0x18
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_scr_main.cpp",
                    10483,
                    0,
                    "%s",
                    "(unsigned)offset < ARRAY_COUNT( cent_fields ) - 1") )
    {
        __debugbreak();
    }
    f = &cent_fields[offset];
    if ( f->setter )
        f->setter(ent, f);
    else
        Scr_SetGenericField((unsigned __int8 *)ent, f->type, f->ofs, SCRIPTINSTANCE_CLIENT, 0);
    return 1;
}

void __cdecl CScr_AddFieldsForEntity()
{
    const cent_field_s *f; // [esp+0h] [ebp-4h]

    for ( f = cent_fields; f->name; ++f )
        Scr_AddClassField(0, (char *)f->name, (unsigned __int16)(f - cent_fields), SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_AddFieldsForRadiant()
{
    Scr_AddFields("radiant", (char*)"txt", SCRIPTINSTANCE_CLIENT);
}

void __cdecl CScr_PostLoadScripts()
{
    signed int classnum; // [esp+0h] [ebp-4h]

    for ( classnum = 0; classnum < 5; ++classnum )
        Scr_SetClassMap(SCRIPTINSTANCE_CLIENT, classnum);
    CScr_AddFieldsForEntity();
    CScr_AddFieldsForRadiant();
}

